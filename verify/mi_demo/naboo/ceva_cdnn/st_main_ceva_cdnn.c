#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>

#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>

#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>

#include "mi_sys.h"
#include "st_hdmi.h"
#include "st_common.h"
#include "st_disp.h"
#include "st_vpe.h"
#include "st_vdisp.h"
#include "st_vif.h"
#include "mi_venc.h"
#include "st_fb.h"
#include "mi_rgn.h"
#include "i2c.h"
#include "st_ceva_cdnn.h"


//#define SUPPORT_CDNN

#ifdef SUPPORT_CDNN
// include cdnn header
#include "CDNNCommonInterface.h"
#include "mi_ive.h"
#include "mem_teller.h"
#include "yolo.h"
#include "dummy_data.h"         // rect result

#endif

pthread_t pt;
static MI_BOOL _bThreadRunning = FALSE;

static MI_BOOL g_subExit = FALSE;
static MI_BOOL g_bExit = FALSE;
static MI_U32 g_u32SubCaseIndex = 0;

static MI_U32 g_u32CaseIndex = 0;
static MI_U32 g_u32LastSubCaseIndex = 0;
static MI_U32 g_u32CurSubCaseIndex = 0;
static MI_S32 g_s32AdWorkMode = 0;

#define MAX_VIF_DEV_NUM 4
#define MAX_VIF_CHN_NUM 16

#define SUPPORT_VIDEO_ENCODE

#define SUPPORT_UVC

#define SUPPORT_VIDEO_PREVIEW

#ifdef SUPPORT_UVC
#include "st_uvc.h"
#endif

#if 0
#define DEBUG_ON    1
#if DEBUG_ON
#define DEBUG_PRINT     printf
#else
#define DEBUG_PRINT
#endif
#endif
//#define DUMP_OUTPUTPORTBUF_TO_FILE
//#define FAKE_CDNN_FRAMEWORK

typedef struct
{
    MI_VENC_CHN vencChn;
    MI_U32 u32MainWidth;
    MI_U32 u32MainHeight;
    MI_VENC_ModType_e eType;
    int vencFd;
} VENC_Attr_t;

typedef struct
{
    pthread_t ptGetEs;
    pthread_t ptFillYuv;
    VENC_Attr_t stVencAttr[MAX_VIF_CHN_NUM];
    MI_U32 u32ChnNum;
    MI_BOOL bRunFlag;
} Venc_Args_t;

Venc_Args_t g_stVencArgs[MAX_VIF_CHN_NUM];

typedef struct ST_ChnInfo_s
{
    MI_S32 s32VideoFormat; //720P 1080P ...
    MI_S32 s32VideoType; //CVI TVI AHD CVBS ...
    MI_S32 s32VideoLost;
    MI_U8 u8EnChannel;
    MI_U8 u8ViDev;
    MI_U8 u8ViChn;
    MI_U8 u8ViPort; //main or sub
} ST_ChnInfo_t;

typedef struct ST_TestInfo_s
{
    ST_ChnInfo_t stChnInfo[MAX_VIF_CHN_NUM];
} ST_TestInfo_t;

#ifdef SUPPORT_CDNN
#define FRAME_WIDTH         608
#define FRAME_HEIGHT        352
#define BOX_MAX_NUM         32

#define SRC_NAME_0  "src_img_0"
#define SRC_NAME_1  "src_img_1"
#define DST_NAME_0  "dst_img_0"

typedef void* CDNN_HANDLE;

typedef struct
{
    MI_S32 s32Fd;
    MI_SYS_ChnPort_t stSinkChnPort;

}ST_SinkHandle_t;

typedef struct
{
    MI_U16 u16LeftTopX;
    MI_U16 u16LeftTopY;
    MI_U16 u16RightBottomX;
    MI_U16 u16RightBottomY;
}ST_DisplayRect_t;

typedef struct
{
    CDNN_HANDLE hCdnnHandle;
    MI_IVE_HANDLE hIveHandle;
    mem_teller stTeller;
    MI_IVE_SrcImage_t stSrcImage;
    MI_IVE_DstImage_t stDstImage;
    BBox aBox[BOX_MAX_NUM];
    ST_DisplayRect_t stDisplayRect[BOX_MAX_NUM];
}ST_CDNN_Algo_t;

typedef struct
{
    MI_BOOL bUsrGet;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    pthread_mutex_t mtxChn;
    MI_U16 u16PreviewWidth;
    MI_U16 u16PreviewHeight;
    MI_U16 u16CdnnWidth;
    MI_U16 u16CdnnHeight;
}ST_CDNN_FrameInfo_t;

const char _gszDeviceName[] = "/dev/ceva_linkdrv_xm6_0";        // fixed
const char _gszImagePath[] = "image.bin";                       // dsp_bootup_load
float _gafAnchors[18] = {9.5,9.5, 9.5,9.5, 9.5,9.5, 9.5,9.5, 9.5,9.5, 9.5,9.5, 9.5,9.5, 9.5,9.5, 9.5,9.5};
YoloOutputParam _gYoloParam = {
        .img_height = FRAME_HEIGHT,
        .img_width = FRAME_WIDTH,
        .height = FRAME_HEIGHT/32,
        .width = FRAME_WIDTH/32,
        .n_box = 9,
        .n_class = 1,
        .prob_thresh = 0.4,
        .nms_thresh = 0.5,
        .anchors = _gafAnchors
    };

ST_CDNN_Algo_t _gstCdnnAlgo;
ST_SinkHandle_t _gastCvSinkHandle[MI_VPE_MAX_CHANNEL_NUM];
struct pollfd _gaFdSet[MI_VPE_MAX_CHANNEL_NUM];
ST_CDNN_FrameInfo_t _gastFrameInfo[MI_VPE_MAX_CHANNEL_NUM];

MI_U32 _gu32ActiveFdCnt = 0;

pthread_mutex_t _gmtxFdSet;
MI_BOOL _gbFdWorkExit = FALSE;
pthread_t _gpthreadFd;

pthread_t _gpGetBufThread;
MI_BOOL _gbGetBufThreadExit = FALSE;
MI_BOOL _gbGetBufThreadPause = TRUE;

pthread_t _gpCalcThread;
MI_BOOL _gbCalcThreadExit = FALSE;
MI_BOOL _gbCalcThreadPause = TRUE;


#endif

//Config logic chn trans to phy chn
ST_VifChnConfig_t stVifChnCfg[VIF_MAX_CHN_NUM] = {
    {0, 0, 0}, {0, 1, 0}, {0, 2, 0}, {0, 3, 0}, //16main
    {0, 4, 0}, {0, 5, 0}, {0, 6, 0}, {0, 7, 0},
    {0, 8, 0}, {0, 9, 0}, {0, 10, 0}, {0, 11, 0},
    {0, 12, 0}, {0, 13, 0}, {0, 14, 0}, {0, 15, 0},
    {0, 0, 1}, {0, 1, 1}, {0, 2, 1}, {0, 3, 1}, //16sub
    {0, 4, 1}, {0, 5, 1}, {0, 6, 1}, {0, 7, 1},
    {0, 8, 1}, {0, 9, 1}, {0, 10, 1}, {0, 11, 1},
    {0, 12, 1}, {0, 13, 1}, {0, 14, 1}, {0, 15, 1},
};

ST_CaseDesc_t g_stVifCaseDesc[] =
{
    {
        .stDesc =
        {
            .u32CaseIndex = 0,
            .szDesc = "MIPI0-1x1080P Video Capture(VPE)",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 8,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 1280x1024 timing",
                .eDispoutTiming = E_ST_TIMING_1280x1024_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1024x768 timing",
                .eDispoutTiming = E_ST_TIMING_1024x768_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1600x1200 timing",
                .eDispoutTiming = E_ST_TIMING_1600x1200_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "change to 1440x900 timing",
                .eDispoutTiming = E_ST_TIMING_1440x900_60,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_CHN,
                .uChnArg =
                {
                    .stVifChnArg =
                    {
                        .u32Chn = 0,
                        .u16CapWidth = 3840,
                        .u16CapHeight = 2160,
                        .s32FrmRate = E_MI_VIF_FRAMERATE_FULL,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                    }
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 1,
            .szDesc = "MIPI0-1x1080P Video Capture H264(VENC)",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 8,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 1280x1024 timing",
                .eDispoutTiming = E_ST_TIMING_1280x1024_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1024x768 timing",
                .eDispoutTiming = E_ST_TIMING_1024x768_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1600x1200 timing",
                .eDispoutTiming = E_ST_TIMING_1600x1200_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "change to 1440x900 timing",
                .eDispoutTiming = E_ST_TIMING_1440x900_60,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_CHN,
                .uChnArg =
                {
                    .stVifChnArg =
                    {
                        .u32Chn = 0,
                        .u16CapWidth = 3840,
                        .u16CapHeight = 2160,
                        .s32FrmRate = E_MI_VIF_FRAMERATE_FULL,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                    }
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 2,
            .szDesc = "exit",
            .u32WndNum = 0,
        }
    }
};

#ifdef SUPPORT_CDNN
int dsp_boot_up(const char *image_name, void *working_buffer)
{
#ifdef FAKE_CDNN_FRAMEWORK
    return 0;
#else
    struct dsp_image image;
    int img_handle, dsp_handle, size, ret = 0;

    // check file size
    struct stat st;
    if (stat(image_name, &st))  // samba方式使用会异�?    {
        printf("stat fail : %s, %s\n", image_name, strerror(errno));
        return -1;
    }

    if (st.st_size == 0)
    {
        printf("image size is 0\n");
        return -1;
    }

    // allocate temporary buffer for image
    image.size = st.st_size;
    image.data = malloc(image.size);
    if (image.data == NULL)
    {
        printf("can't allocate buffer for image\n");
        return -1;
    }

    ///////////////////////////////////////////////////////////
    // read image

    img_handle = open(image_name, O_RDONLY);
    if (img_handle <= 0)
    {
        printf("Can't open image %s (%d: %s)\n", image_name, errno, strerror(errno));
        ret = -1;
        goto RETURN_0;
    }

    size = read(img_handle, image.data, image.size);
    if (size != image.size)
    {
        printf("only read %d but size is %d\n", size, image.size);
        ret = -1;
        goto RETURN_1;
    }

    ///////////////////////////////////////////////////////////
    // boot up DSP

    dsp_handle = open(_gszDeviceName, O_RDWR);
    if (dsp_handle < 0)
    {
        printf("can't open device %s\n", _gszDeviceName);
        ret = -1;
        goto RETURN_1;
    }

    ret = ioctl(dsp_handle, IOC_CEVADRV_BOOT_UP, &image);
    if (ret != 0)
    {
        printf("DSP does not boot up\n");
        ret = -1;
        goto RETURN_2;
    }

    printf("DSP boot up\n");

RETURN_2:
    close(dsp_handle);

RETURN_1:
    close(img_handle);

RETURN_0:
    free(image.data);

    return ret;
#endif
}

void* cdnn_create(const char *model_name)
{
    int status = 0;
    void *cdnn_handle = NULL;

    cdnnBringUpInfo_st cdnnBringUpInfo = { 0 };
    cdnnTargetInfo_st cdnnTargetInfo;

    cdnnTargetInfo.eDeviceType = E_CDNN_DEVICE_USER_XM6;
    cdnnBringUpInfo.NumberOfTargets = 1;
    cdnnBringUpInfo.pTargetsInformation = &cdnnTargetInfo;
    status = CDNNCreate(&cdnn_handle, &cdnnBringUpInfo);
    if (status)
    {
        printf("Failed to call CDNNCreate(), ret is %d\n", status);
        return NULL;
    }

    // we do not create a real network so far because driver is not ready

    return cdnn_handle;
}

void cdnn_release(void *cdnn_handle)
{
}

int ive_buffer_create(mem_teller *teller, MI_IVE_SrcImage_t *src, MI_IVE_DstImage_t *dst)
{
    int ret = 0;

    memset(src, 0, sizeof(MI_IVE_Image_t));
    src->eType = E_MI_IVE_IMAGE_TYPE_YUV420SP;
    src->u16Width  = FRAME_WIDTH;
    src->u16Height = FRAME_HEIGHT;
    src->azu16Stride[0] = FRAME_WIDTH;
    src->azu16Stride[1] = FRAME_WIDTH;
    src->azu16Stride[2] = FRAME_WIDTH;

    ret = mem_teller_alloc(teller, &src->aphyPhyAddr[0], FRAME_WIDTH*FRAME_HEIGHT, SRC_NAME_0);
    if (ret != 0) goto RETURN_0;

    ret = mem_teller_mmap(teller, src->aphyPhyAddr[0], &src->apu8VirAddr[0], FRAME_WIDTH*FRAME_HEIGHT);
    if (ret != 0) goto RETURN_1;

    ret = mem_teller_alloc(teller, &src->aphyPhyAddr[1], FRAME_WIDTH*FRAME_HEIGHT/2, SRC_NAME_1);
    if (ret != 0) goto RETURN_2;

    ret = mem_teller_mmap(teller, src->aphyPhyAddr[1], &src->apu8VirAddr[1], FRAME_WIDTH*FRAME_HEIGHT/2);
    if (ret != 0) goto RETURN_3;

    memset(dst, 0, sizeof(MI_IVE_Image_t));
    dst->eType = E_MI_IVE_IMAGE_TYPE_U8C3_PACKAGE;
    dst->u16Width  = FRAME_WIDTH;
    dst->u16Height = FRAME_HEIGHT;
    dst->azu16Stride[0] = FRAME_WIDTH;
    dst->azu16Stride[1] = FRAME_WIDTH;
    dst->azu16Stride[2] = FRAME_WIDTH;

    ret = mem_teller_alloc(teller, &dst->aphyPhyAddr[0], FRAME_WIDTH*FRAME_HEIGHT*3, DST_NAME_0);
    if (ret != 0) goto RETURN_4;

    ret = mem_teller_mmap(teller, dst->aphyPhyAddr[0], &dst->apu8VirAddr[0], FRAME_WIDTH*FRAME_HEIGHT*3);
    if (ret != 0) goto RETURN_5;

    return ret;

RETURN_5:
    mem_teller_free(teller, dst->aphyPhyAddr[0], DST_NAME_0);

RETURN_4:
    mem_teller_unmmap(teller, src->apu8VirAddr[1], FRAME_WIDTH*FRAME_HEIGHT/2);

RETURN_3:
    mem_teller_free(teller, src->aphyPhyAddr[1], SRC_NAME_1);

RETURN_2:
    mem_teller_unmmap(teller, src->apu8VirAddr[0], FRAME_WIDTH*FRAME_HEIGHT);

RETURN_1:
    mem_teller_free(teller, src->aphyPhyAddr[0], SRC_NAME_0);

RETURN_0:
    return ret;
}

void ive_buffer_release(mem_teller *teller, MI_IVE_SrcImage_t *src, MI_IVE_DstImage_t *dst)
{
    mem_teller_unmmap(teller, dst->apu8VirAddr[0], FRAME_WIDTH*FRAME_HEIGHT*3);
    mem_teller_free(teller, dst->aphyPhyAddr[0], DST_NAME_0);
    mem_teller_unmmap(teller, src->apu8VirAddr[1], FRAME_WIDTH*FRAME_HEIGHT/2);
    mem_teller_free(teller, src->aphyPhyAddr[1], SRC_NAME_1);
    mem_teller_unmmap(teller, src->apu8VirAddr[0], FRAME_WIDTH*FRAME_HEIGHT);
    mem_teller_free(teller, src->aphyPhyAddr[0], SRC_NAME_0);
}





#endif

void ST_VifUsage(void)
{
    ST_CaseDesc_t *pstCaseDesc = g_stVifCaseDesc;
    MI_U32 u32Size = ARRAY_SIZE(g_stVifCaseDesc);
    MI_U32 i = 0;

    for (i = 0; i < u32Size; i ++)
    {
        printf("%d)\t %s\n", pstCaseDesc[i].stDesc.u32CaseIndex + 1, pstCaseDesc[i].stDesc.szDesc);
    }
}

void ST_CaseSubUsage(void)
{
    ST_CaseDesc_t *pstCaseDesc = g_stVifCaseDesc;
    MI_U32 u32CaseSize = ARRAY_SIZE(g_stVifCaseDesc);
    MI_U32 u32CaseIndex = g_u32CaseIndex;
    MI_U32 i = 0;

    if (u32CaseIndex < 0 || u32CaseIndex > u32CaseSize)
    {
        return;
    }

    for (i = 0; i < pstCaseDesc[u32CaseIndex].u32SubCaseNum; i ++)
    {
        printf("\t%d) %s\n", pstCaseDesc[u32CaseIndex].stSubDesc[i].u32CaseIndex + 1,
            pstCaseDesc[u32CaseIndex].stSubDesc[i].szDesc);
    }
}

#ifdef SUPPORT_CDNN
void _ST_CEVA_GetChnPortFormFd(MI_S32 s32Fd, MI_SYS_ChnPort_t *pstChnPort)
{
    MI_S32 i = 0;

    for (i = 0; i < MI_VPE_MAX_CHANNEL_NUM; i++)
    {
        if (s32Fd == _gastCvSinkHandle[i].s32Fd)
        {
            *pstChnPort = _gastCvSinkHandle[i].stSinkChnPort;
            break;
        }
    }

//    DEBUG_PRINT("ChnPort info: devId=%d chnId=%d portId=%d\n", pstChnPort->u32DevId, pstChnPort->u32ChnId,
//                 pstChnPort->u32PortId);
}

void *_ST_CEVA_CdnnGetBufThread(void * args)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_S32 s32TimeOutMs = 20;
    void *pFrameDataBuf[2];
    MI_U32 u32BufSize = 0;
    MI_S32 s32Ret;
    MI_S32 i, S32TotalBox;

#ifdef DUMP_OUTPUTPORTBUF_TO_FILE
    FILE *paFile[MI_VPE_MAX_CHANNEL_NUM] = NULL;
    char szaFilePath[MI_VPE_MAX_CHANNEL_NUM][16];
    MI_S32 s32SampleForTest = 200;

    for (i = 0; i < MI_VPE_MAX_CHANNEL_NUM; i++)
    {
        memset(szaFilePath[i], 0, 16);
        sprintf(szaFilePath[i], "ceva_sub.yuv_%d", i);

        paFile[i] = fopen(szaFilePath[i], "ab+");
        if (!paFile[i])
        {
            DEBUG_PRINT("Ceva_Thread: Create ceva_sub %d file error\n", i);
        }
    }
#endif

    DEBUG_PRINT("_ST_CEVA_CdnnGetBufThread\n");

    while (1)
    {
#ifdef DUMP_OUTPUTPORTBUF_TO_FILE
        if (s32SampleForTest-- <= 0)
        {
            _gbGetBufThreadExit = TRUE;
        }
#endif

        if(_gbGetBufThreadExit)
        {
            DEBUG_PRINT("Ceva_Thread: Ceva work stop\n");
#ifdef DUMP_OUTPUTPORTBUF_TO_FILE
            for (i = 0; i < MI_VPE_MAX_CHANNEL_NUM; i++)
            {
                if (paFile[i])
                {
                    fclose(paFile[i]);
                    paFile[i] = NULL;
                }
            }
#endif
            break;
        }

        if (_gbGetBufThreadPause)
        {
            usleep(20000);
            continue;
        }

        // recv date

        pthread_mutex_lock(&_gmtxFdSet);
        s32Ret = -1;
        if (_gu32ActiveFdCnt > 0)
        {
//            DEBUG_PRINT("active fd cnt=%d\n", _gu32ActiveFdCnt);

            //s32Ret = poll(_gaFdSet, _gu32ActiveFdCnt, s32TimeOutMs);
            s32Ret = poll(_gaFdSet, MI_VPE_MAX_CHANNEL_NUM, s32TimeOutMs);

            if (s32Ret > 0)
            {
                for (i = 0; i < _gu32ActiveFdCnt; i++)
                {
                    if (_gaFdSet[i].revents & POLLIN)
                    {
                        _ST_CEVA_GetChnPortFormFd(_gaFdSet[i].fd, &stChnPort);
                        break;
                    }
                }
            }
        }
        DEBUG_PRINT("getBufThread: recv data. fd=%d portId=%u s32Ret=%d\n", _gaFdSet[i-1].fd, stChnPort.u32PortId, s32Ret);
        pthread_mutex_unlock(&_gmtxFdSet);

        if (s32Ret > 0)
        {
            // get data
            pthread_mutex_lock(&_gastFrameInfo[stChnPort.u32ChnId].mtxChn);

            if (_gastFrameInfo[stChnPort.u32ChnId].bUsrGet)
            {
                MI_SYS_ChnOutputPortPutBuf(hHandle);
                _gastFrameInfo[stChnPort.u32ChnId].bUsrGet = FALSE;
            }

            s32Ret = MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &hHandle);

            DEBUG_PRINT("getBufThread: MI_SYS_ChnOutputPortGetBuf ret=%d\n", s32Ret);

            if (MI_SUCCESS ==  s32Ret)
            {
                _gastFrameInfo[stChnPort.u32ChnId].stBufInfo = stBufInfo;
                _gastFrameInfo[stChnPort.u32ChnId].hHandle = hHandle;
                _gastFrameInfo[stChnPort.u32ChnId].bUsrGet = TRUE;
            }
            pthread_mutex_unlock(&_gastFrameInfo[stChnPort.u32ChnId].mtxChn);

            // dump buf
#ifdef DUMP_OUTPUTPORTBUF_TO_FILE
            if (E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == stBufInfo.stFrameData.ePixelFormat)
            {
                u32BufSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u16Width;
                MI_SYS_Mmap(stBufInfo.stFrameData.phyAddr[0], u32BufSize, &pFrameDataBuf[0], FALSE);
                MI_SYS_Mmap(stBufInfo.stFrameData.phyAddr[1], u32BufSize, &pFrameDataBuf[1], FALSE);

                DEBUG_PRINT("chn=%d, hgt=%d, stride0=%d, size=%d fp=%p vir0=%p phy0=%llx vir1=%p phy1=%llx\n"           ,
                            stChnPort.u32ChnId, stBufInfo.stFrameData.u16Height, stBufInfo.stFrameData.u32Stride[0],
                            u32BufSize, paFile[stChnPort.u32ChnId], stBufInfo.stFrameData.pVirAddr[0],
                            stBufInfo.stFrameData.phyAddr[0], stBufInfo.stFrameData.pVirAddr[1], stBufInfo.stFrameData.phyAddr[1]);

                if (paFile[stChnPort.u32ChnId] && u32BufSize && pFrameDataBuf[0] && pFrameDataBuf[1])
                {
                    fwrite(pFrameDataBuf[0], u32BufSize, 1, paFile[stChnPort.u32ChnId]);
                    fwrite(pFrameDataBuf[1], u32BufSize/2, 1, paFile[stChnPort.u32ChnId]);
                }
            }
            else if (E_MI_SYS_PIXEL_FRAME_YUV422_YUYV == stBufInfo.stFrameData.ePixelFormat)   // 422
            {
                u32BufSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                s32Ret = MI_SYS_Mmap(stBufInfo.stFrameData.phyAddr[0], u32BufSize, &pFrameDataBuf[0], FALSE);
                DEBUG_PRINT("chn=%d, hgt=%d, stride0=%d, size=%d fp=%p vir0=%p phy0=%lld pWriteBuf0=%p ret=%d\n",
                            stChnPort.u32ChnId, stBufInfo.stFrameData.u16Height, stBufInfo.stFrameData.u32Stride[0],
                            u32BufSize, paFile[stChnPort.u32ChnId], stBufInfo.stFrameData.pVirAddr[0],
                            stBufInfo.stFrameData.phyAddr[0], pFrameDataBuf[0], s32Ret);

                if (paFile[stChnPort.u32ChnId] && u32BufSize && pFrameDataBuf[0] && (MI_SUCCESS==s32Ret))
                {
                    s32Ret = fwrite(pFrameDataBuf[0], u32BufSize, 1, paFile[stChnPort.u32ChnId]);
                    // DEBUG_PRINT("write %d bytes\n", s32Ret);
                }
            }
            else
            {
                DEBUG_PRINT("Ceva_Thread: Frame format not support\n");
            }
#endif
        }

        usleep(200000);
    }

#ifdef DUMP_OUTPUTPORTBUF_TO_FILE
    for (i = 0; i < MI_VPE_MAX_CHANNEL_NUM; i++)
    {
        if (paFile[i])
        {
            fclose(paFile[i]);
            paFile[i] = NULL;
        }
    }

    DEBUG_PRINT("Close dump file\n");
#endif

    return NULL;
}


void *_ST_CEVA_CdnnCalcThread(void * args)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_S32 S32TotalBox;
    MI_U32 i = 0;
    MI_U32 u32ActiveCnt = 0;
    MI_BOOL bDataValid = TRUE;
    MI_S32 s32CurFd = 0;
    MI_S32 s32FdIndex = 0;
    MI_S32 s32CurChn = 0;

    DEBUG_PRINT("_ST_CEVA_CdnnCalcThread\n");

    while (1)
    {
        if (_gbCalcThreadExit)
        {
            break;
        }

        if (_gbCalcThreadPause)
        {
            usleep(20000);
            continue;
        }

        pthread_mutex_lock(&_gmtxFdSet);
        u32ActiveCnt = _gu32ActiveFdCnt;

        if (_gu32ActiveFdCnt > 0)
        {
            if (s32FdIndex >= _gu32ActiveFdCnt)
                s32FdIndex = 0;

            s32CurFd = _gaFdSet[s32FdIndex].fd;
//             DEBUG_PRINT("CalcThread: curFd=%d\n", s32CurFd);
        }
        else
        {
            usleep(10000);
            pthread_mutex_unlock(&_gmtxFdSet);
            continue;
        }

        pthread_mutex_unlock(&_gmtxFdSet);

        _ST_CEVA_GetChnPortFormFd(s32CurFd, &stChnPort);
        s32CurChn = stChnPort.u32ChnId;
//        DEBUG_PRINT("CalcThread: curChn=%d\n", s32CurChn);

        pthread_mutex_lock(&_gastFrameInfo[s32CurChn].mtxChn);
        if (_gastFrameInfo[s32CurChn].bUsrGet)
        {
            DEBUG_PRINT("CalcThread: get buff curChn=%d\n", s32CurChn);
            bDataValid = TRUE;

            // 1. transfer YUV420/YUV422 to YUV444 by IVE
            if (E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == _gastFrameInfo[s32CurChn].stBufInfo.stFrameData.ePixelFormat)
            {
                // 1. convert YUYV to 444
            }
            else if (E_MI_SYS_PIXEL_FRAME_YUV422_YUYV == _gastFrameInfo[s32CurChn].stBufInfo.stFrameData.ePixelFormat)   // 422
            {
                // 1. convert YUYV to 444
            }
            else
            {
                DEBUG_PRINT("Ceva_Thread: Frame format not support\n");
                bDataValid = FALSE;
            }

            if (bDataValid)
            {
                // 2. calculate to generator a result

                // 3. parse result to x-y of cdnn port
                S32TotalBox = GetBBox((float *)cdnn_result, _gYoloParam, _gstCdnnAlgo.aBox, BOX_MAX_NUM);

                if (S32TotalBox > BOX_MAX_NUM)
                {
                    DEBUG_PRINT("detect %d but only %d gotten\n", S32TotalBox, BOX_MAX_NUM);
                    S32TotalBox = BOX_MAX_NUM;
                }

                // 4. convert x-y of cdnn port to preview port's
                memset(_gstCdnnAlgo.stDisplayRect, 0, sizeof(_gstCdnnAlgo.stDisplayRect));

                for (i=0; i<S32TotalBox; i++)
                {
                    DEBUG_PRINT("prob = %.2f%%, top-left = (%4d, %4d), bottom-right = (%4d, %4d)\n",
                                _gstCdnnAlgo.aBox[i].prob*100, _gstCdnnAlgo.aBox[i].x_min, _gstCdnnAlgo.aBox[i].y_min,
                                _gstCdnnAlgo.aBox[i].x_max, _gstCdnnAlgo.aBox[i].y_max);

                    _gstCdnnAlgo.stDisplayRect[i].u16LeftTopX = _gstCdnnAlgo.aBox[i].x_min * _gastFrameInfo[s32CurChn].u16PreviewWidth
                                                                      / _gastFrameInfo[s32CurChn].u16CdnnWidth;
                    _gstCdnnAlgo.stDisplayRect[i].u16LeftTopY = _gstCdnnAlgo.aBox[i].y_min * _gastFrameInfo[s32CurChn].u16PreviewHeight
                                                                      / _gastFrameInfo[s32CurChn].u16CdnnHeight;
                    _gstCdnnAlgo.stDisplayRect[i].u16RightBottomX = _gstCdnnAlgo.aBox[i].x_max * _gastFrameInfo[s32CurChn].u16PreviewWidth
                                                                           / _gastFrameInfo[s32CurChn].u16CdnnWidth;
                    _gstCdnnAlgo.stDisplayRect[i].u16RightBottomY = _gstCdnnAlgo.aBox[i].y_max * _gastFrameInfo[s32CurChn].u16PreviewHeight
                                                                           / _gastFrameInfo[s32CurChn].u16CdnnHeight;

                    DEBUG_PRINT("%2d: top-left = (%4d, %4d), bottom-right = (%4d, %4d)\n", i, _gstCdnnAlgo.stDisplayRect[i].u16LeftTopX
                                , _gstCdnnAlgo.stDisplayRect[i].u16LeftTopY, _gstCdnnAlgo.stDisplayRect[i].u16RightBottomX,
                                _gstCdnnAlgo.stDisplayRect[i].u16RightBottomY);
                }

                // 5. create rgn to draw rect


            }

            MI_SYS_ChnOutputPortPutBuf(_gastFrameInfo[s32CurChn].hHandle);
            _gastFrameInfo[s32CurChn].bUsrGet = FALSE;
        }

        pthread_mutex_unlock(&_gastFrameInfo[s32CurChn].mtxChn);

        s32FdIndex++;
    }

    return NULL;
}

// initial fd list
MI_S32 ST_CEVA_CDNN_Init()
{
    MI_S32 i = 0;

    _gbGetBufThreadExit = FALSE;
    _gbCalcThreadExit = FALSE;
    _gbGetBufThreadPause = TRUE;
    _gbCalcThreadPause = TRUE;

    memset(&_gstCdnnAlgo, 0 ,sizeof(ST_CDNN_Algo_t));
    memset(_gastCvSinkHandle, 0, sizeof(_gastCvSinkHandle));
    memset(_gastFrameInfo, 0, sizeof(_gastFrameInfo));
    memset(_gaFdSet, 0, sizeof(_gaFdSet));

    pthread_mutex_init(&_gmtxFdSet, NULL);

    for (i = 0; i < MI_VPE_MAX_CHANNEL_NUM; i++)
    {
        pthread_mutex_init(&_gastFrameInfo[i].mtxChn, NULL);
        _gastFrameInfo[i].bUsrGet = FALSE;
        _gastFrameInfo[i].hHandle = 0;
        memset(&_gastFrameInfo[i].stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
        _gastFrameInfo[i].u16PreviewWidth = 0;
        _gastFrameInfo[i].u16PreviewHeight = 0;
        _gastFrameInfo[i].u16CdnnWidth = 0;
        _gastFrameInfo[i].u16CdnnHeight = 0;

        _gaFdSet[i].events = POLLIN|POLLERR;
    }

    // 1. ive init
    // 1.1 create buf handle
    DEBUG_PRINT("1.1 create buf handle\n");
    if (mem_teller_create(&_gstCdnnAlgo.stTeller) != 0)
    {
        DEBUG_PRINT("can't create memory teller\n");
        goto INITIAL_FAILED;
    }

    // 1.2 create frame buffer
    DEBUG_PRINT("1.2 create frame buffer\n");
    if (ive_buffer_create(&_gstCdnnAlgo.stTeller, &_gstCdnnAlgo.stSrcImage, &_gstCdnnAlgo.stDstImage) != 0)
    {
        printf("can't create ive buffers\n");
        goto DESTROY_BUFHANDLE;
    }

    // 1.3 create ive handle
    DEBUG_PRINT("1.3 create ive handle\n");
    if (MI_IVE_Create(_gstCdnnAlgo.hIveHandle) != 0)
    {
        printf("Could not create IVE handle %d\n", _gstCdnnAlgo.hIveHandle);
        goto DESTROY_IVE_BUF;
    }

    // 2. boot up dsp
    DEBUG_PRINT("2. boot up dsp\n");
    if (dsp_boot_up("image.bin", NULL) != 0)
    {
        printf("can't boot up dsp\n");
        goto DESTROY_IVE_HANDLE;
    }

    // 3. cdnn init
    /* not ready yet...
    DEBUG_PRINT("3. cdnn init\n");
    cdnn_handle = cdnn_create(....);
    if (cdnn_handle == NULL)
    {
        printf("can't create dsp\n");
        goto DESTROY_IVE_HANDLE;
    }
    */
#if 1
    // 4. create cdnn workThread
    DEBUG_PRINT("4.1 create cdnn getBufThread\n");
    pthread_create(&_gpGetBufThread, NULL, _ST_CEVA_CdnnGetBufThread, NULL);  // save to buf list
    if (!_gpGetBufThread)
    {
        goto DESTROY_CDNN_HANDLE;
    }

    DEBUG_PRINT("4.2 create cdnn calcThread\n");
    pthread_create(&_gpCalcThread, NULL, _ST_CEVA_CdnnCalcThread, NULL);    // proc thread
    if (!_gpCalcThread)
    {
        goto DESTROY_GETBUF_THREAD;
    }

    return 0;

DESTROY_GETBUF_THREAD:
    pthread_join(_gpGetBufThread, NULL);
#endif

DESTROY_CDNN_HANDLE:
    cdnn_release(_gstCdnnAlgo.hCdnnHandle);

DESTROY_IVE_HANDLE:
    MI_IVE_Destroy(_gstCdnnAlgo.hIveHandle);

DESTROY_IVE_BUF:
    ive_buffer_release(&_gstCdnnAlgo.stTeller, &_gstCdnnAlgo.stSrcImage, &_gstCdnnAlgo.stDstImage);

DESTROY_BUFHANDLE:
    mem_teller_release(&_gstCdnnAlgo.stTeller);

INITIAL_FAILED:
    return -1;
}

// deinit fd list
MI_S32 ST_CEVA_CDNN_Deinit()
{
    MI_S32 i = 0;

    if (_gpGetBufThread)
    {
        _gbGetBufThreadExit = TRUE;
        pthread_join(_gpGetBufThread, NULL);
    }

    if (_gpCalcThread)
    {
        _gbCalcThreadExit = TRUE;
        pthread_join(_gpCalcThread, NULL);
    }

    _gpGetBufThread = NULL;
    _gpCalcThread = NULL;

    // release cdnn
    cdnn_release(_gstCdnnAlgo.hCdnnHandle);

    // destroy ive handle
    MI_IVE_Destroy(_gstCdnnAlgo.hIveHandle);

    // free ive buf
    ive_buffer_release(&_gstCdnnAlgo.stTeller, &_gstCdnnAlgo.stSrcImage, &_gstCdnnAlgo.stDstImage);

    // release buf handle
    mem_teller_release(&_gstCdnnAlgo.stTeller);

    // param destroy
    for (i = 0; i < MI_VPE_MAX_CHANNEL_NUM; i++)
    {
        pthread_mutex_destroy(&_gastFrameInfo[i].mtxChn);
        _gastFrameInfo[i].bUsrGet = FALSE;
        _gastFrameInfo[i].hHandle = 0;
        memset(&_gastFrameInfo[i].stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
        _gastFrameInfo[i].u16PreviewWidth = 0;
        _gastFrameInfo[i].u16PreviewHeight = 0;
        _gastFrameInfo[i].u16CdnnWidth = 0;
        _gastFrameInfo[i].u16CdnnHeight = 0;
    }

    pthread_mutex_destroy(&_gmtxFdSet);

    memset(&_gstCdnnAlgo, 0 ,sizeof(ST_CDNN_Algo_t));
    memset(_gastCvSinkHandle, 0, sizeof(_gastCvSinkHandle));
    memset(_gaFdSet, 0, sizeof(_gaFdSet));

    DEBUG_PRINT("ST_CEVA_CDNN_DeInit\n");

    return 0;
}

MI_S32 ST_CEVA_CDNN_Start()
{
    if (_gpGetBufThread)
    {
        _gbGetBufThreadPause = FALSE;
    }

    if (_gpCalcThread)
    {
        _gbCalcThreadPause = FALSE;
    }

    return 0;
}

MI_S32 ST_CEVA_CDNN_Stop()
{
    if (_gpGetBufThread)
    {
        _gbGetBufThreadPause = TRUE;
    }

    if (_gpCalcThread)
    {
        _gbCalcThreadPause = TRUE;
    }

    return 0;
}

// regitster chn, add port to listen fd list
MI_S32 ST_CEVA_RegisterSevr(MI_SYS_ChnPort_t stCdnnPort)
{
    MI_S32 s32Fd = 0;
    MI_S32 i = 0;
    MI_S32 s32Ret;

    MI_SYS_SetChnOutputPortDepth(&stCdnnPort, 3, 5);

    s32Ret = MI_SYS_GetFd(&stCdnnPort, &s32Fd);
    if (MI_SUCCESS != s32Ret)
    {
        DEBUG_PRINT("get fd error, devId=%d, chn=%d, port=%d\n", stCdnnPort.u32DevId,          stCdnnPort.u32ChnId,
                    stCdnnPort.u32PortId);
        return -1;
    }

    DEBUG_PRINT("register port: chnId=%d portId=%d fd=%d\n", stCdnnPort.u32ChnId, stCdnnPort.u32PortId, s32Fd);

    _gastCvSinkHandle[stCdnnPort.u32ChnId].s32Fd = s32Fd;
    _gastCvSinkHandle[stCdnnPort.u32ChnId].stSinkChnPort = stCdnnPort;

    pthread_mutex_lock(&_gmtxFdSet);
    for (i = 0; i < MI_VPE_MAX_CHANNEL_NUM; i++)
    {
        if (_gaFdSet[i].fd == s32Fd)
        {
            DEBUG_PRINT("Port has been registered, devId=%d, chnId=%d, portId=%d\n", stCdnnPort.u32DevId,
                         stCdnnPort.u32ChnId, stCdnnPort.u32PortId);
            pthread_mutex_unlock(&_gmtxFdSet);
            return MI_SUCCESS;
        }
        else if (_gaFdSet[i].fd <= 0)
        {
            _gaFdSet[i].fd = s32Fd;
            _gaFdSet[i].events = POLLIN|POLLERR;
            break;
        }
    }

    _gu32ActiveFdCnt++;
    pthread_mutex_unlock(&_gmtxFdSet);

    DEBUG_PRINT("Dump fdset:");
    for (i = 0; i < MI_VPE_MAX_CHANNEL_NUM; i++)
    {
        DEBUG_PRINT("%d ", _gaFdSet[i].fd);
    }
    DEBUG_PRINT("\n");

    return 0;
}


// unregister chn, remove port from fd list and resort fd list
MI_S32 ST_CEVA_UnRegisterSevr(MI_SYS_ChnPort_t stCdnnPort)
{
    MI_S32 s32Fd = 0;
    MI_S32 i = 0;
    MI_S32 s32Ret;

    s32Ret = MI_SYS_GetFd(&stCdnnPort, &s32Fd);
    if (MI_SUCCESS != s32Ret)
    {
        DEBUG_PRINT("get fd error, devId=%d, chn=%d, port=%d\n", stCdnnPort.u32DevId,          stCdnnPort.u32ChnId,
                    stCdnnPort.u32PortId);
        return s32Ret;
    }

    pthread_mutex_lock(&_gmtxFdSet);
    for (i = 0; i < _gu32ActiveFdCnt; i++)
    {
        if (_gaFdSet[i].fd = s32Fd)
        {
            _gaFdSet[i].fd = 0;
            i--;
            break;
        }
    }

    for(;i < _gu32ActiveFdCnt; i++)
    {
        if (i == _gu32ActiveFdCnt - 1)
            _gaFdSet[i].fd = 0;
        else
            _gaFdSet[i].fd = _gaFdSet[i+1].fd;
    }

    _gu32ActiveFdCnt--;
    pthread_mutex_unlock(&_gmtxFdSet);

    return 0;
}

#endif

#ifdef SUPPORT_UVC
MI_S32 UVC_Init(void *uvc)
{
    return MI_SUCCESS;
}

MI_S32 UVC_Deinit(void *uvc)
{
    return MI_SUCCESS;
}

FILE *g_pUVCFile = NULL;
MI_S32 _UVC_FillBuffer_Encoded(ST_UVC_Device_t *uvc, MI_U32 *length, void *buf)
{
#if 1
    MI_SYS_ChnPort_t stVencChnInputPort;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufInfo_t stBufInfo;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 len = 0;
    ST_CaseDesc_t *pstCaseDesc = g_stVifCaseDesc;
    MI_U32 u32DevId = 0;

    stVencChnInputPort.eModId = E_MI_MODULE_ID_VENC;
    stVencChnInputPort.u32DevId = 2;

    s32Ret = MI_VENC_GetChnDevid(0, &u32DevId);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s %d, MI_VENC_GetChnDevid %d error, %X\n", __func__, __LINE__, 0, s32Ret);
    }
    stVencChnInputPort.u32DevId = u32DevId;
    stVencChnInputPort.u32ChnId = 0;
    stVencChnInputPort.u32PortId = 0;

    hHandle = MI_HANDLE_NULL;
    memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));

    if(MI_SUCCESS == (s32Ret = MI_SYS_ChnOutputPortGetBuf(&stVencChnInputPort, &stBufInfo, &hHandle)))
    {
        if(hHandle == NULL)
        {
            printf("%s %d NULL output port buffer handle.\n", __func__, __LINE__);
        }
        else if(stBufInfo.stRawData.pVirAddr == NULL)
        {
            printf("%s %d unable to read buffer. VA==0\n", __func__, __LINE__);
        }
        else if(stBufInfo.stRawData.u32ContentSize >= 200 * 1024)  //MAX_OUTPUT_ES_SIZE in KO
        {
            printf("%s %d unable to read buffer. buffer overflow\n", __func__, __LINE__);
        }


        // len = write(fd, stBufInfo.stRawData.pVirAddr, stBufInfo.stRawData.u32ContentSize);
        printf("send buf length : %d\n", stBufInfo.stRawData.u32ContentSize);
        *length = stBufInfo.stRawData.u32ContentSize;
        memcpy(buf, stBufInfo.stRawData.pVirAddr, stBufInfo.stRawData.u32ContentSize);

        //printf("%s %d, chn:%d write frame len=%d, real len=%d\n", __func__, __LINE__, stVencChnInputPort.u32ChnId,
        //    len, stBufInfo.stRawData.u32ContentSize);

        MI_SYS_ChnOutputPortPutBuf(hHandle);
    }
#endif
}

MI_S32 UVC_FillBuffer(void *uvc_dev,ST_UVC_BufInfo_t *bufInfo)
{
    ST_UVC_Device_t *uvc = (ST_UVC_Device_t*)uvc_dev;
    unsigned int *length = &(bufInfo->length);
    void *buf  = bufInfo->b.buf;

    switch(uvc->stream_param.fcc)
    {
        case V4L2_PIX_FMT_H264:
        {
            _UVC_FillBuffer_Encoded(uvc, length, buf);
        }
        break;

        case V4L2_PIX_FMT_MJPEG:
        {
            _UVC_FillBuffer_Encoded(uvc,length,buf);
        }
        break;

        default:
        {
            _UVC_FillBuffer_Encoded(uvc,length,buf);
            //printf("%s %d, not support this type, %d\n", __func__, __LINE__, uvc->stream_param.fcc);
        }
        break;
    }

    return MI_SUCCESS;
}

MI_S32 UVC_StartCapture(void *uvc,Stream_Params_t format)
{
    return MI_SUCCESS;
}

MI_S32 UVC_StopCapture(void *uvc)
{
    return MI_SUCCESS;
}

void ST_UVCInit()
{
    ST_UVC_Setting_t pstSet={4,UVC_MEMORY_MMAP,USB_ISOC_MODE};
    ST_UVC_OPS_t fops = { UVC_Init ,
                          UVC_Deinit,
                          .u=UVC_FillBuffer,
                          UVC_StartCapture,
                          UVC_StopCapture};

    ST_UVC_ChnAttr_t pstAttr ={pstSet,fops};
    ST_UVC_Init("/dev/video0");
    ST_UVC_CreateDev(&pstAttr);
    ST_UVC_StartDev();
}
#endif

void *ST_VencGetEsBufferProc(void *args)
{
    Venc_Args_t *pArgs = (Venc_Args_t *)args;

    MI_SYS_ChnPort_t stVencChnInputPort;
    char szFileName[128];
    int fd = -1;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufInfo_t stBufInfo;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 len = 0;

    stVencChnInputPort.eModId = E_MI_MODULE_ID_VENC;
    if (E_MI_VENC_MODTYPE_H264E == pArgs->stVencAttr[0].eType)
    {
        stVencChnInputPort.u32DevId = 2;
    }
    else if (E_MI_VENC_MODTYPE_H265E == pArgs->stVencAttr[0].eType)
    {
        stVencChnInputPort.u32DevId = 0;
    }
    else if (E_MI_VENC_MODTYPE_JPEGE == pArgs->stVencAttr[0].eType)
    {
        stVencChnInputPort.u32DevId = 4;
    }
    stVencChnInputPort.u32ChnId = pArgs->stVencAttr[0].vencChn;
    stVencChnInputPort.u32PortId = 0;

    memset(szFileName, 0, sizeof(szFileName));
    snprintf(szFileName, sizeof(szFileName) - 1, "venc_dev%d_chn%d_port%d_%dx%d_%s.es",
            stVencChnInputPort.u32DevId, stVencChnInputPort.u32ChnId, stVencChnInputPort.u32PortId,
            pArgs->stVencAttr[0].u32MainWidth, pArgs->stVencAttr[0].u32MainHeight,
            (pArgs->stVencAttr[0].eType == E_MI_VENC_MODTYPE_H264E) ? "h264" : "h265");

    fd = open(szFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd <= 0)
    {
        printf("%s %d create %s error\n", __func__, __LINE__, szFileName);
        return NULL;
    }

    printf("%s %d create %s success\n", __func__, __LINE__, szFileName);

    while (1)
    {
        hHandle = MI_HANDLE_NULL;
        memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));

        if(MI_SUCCESS == (s32Ret = MI_SYS_ChnOutputPortGetBuf(&stVencChnInputPort, &stBufInfo, &hHandle)))
        {
            if(hHandle == NULL)
            {
                printf("%s %d NULL output port buffer handle.\n", __func__, __LINE__);
            }
            else if(stBufInfo.stRawData.pVirAddr == NULL)
            {
                printf("%s %d unable to read buffer. VA==0\n", __func__, __LINE__);
            }
            else if(stBufInfo.stRawData.u32ContentSize >= 200 * 1024)  //MAX_OUTPUT_ES_SIZE in KO
            {
                printf("%s %d unable to read buffer. buffer overflow\n", __func__, __LINE__);
            }

            len = write(fd, stBufInfo.stRawData.pVirAddr, stBufInfo.stRawData.u32ContentSize);

            //printf("%s %d, chn:%d write frame len=%d, real len=%d\n", __func__, __LINE__, stVencChnInputPort.u32ChnId,
            //    len, stBufInfo.stRawData.u32ContentSize);

            MI_SYS_ChnOutputPortPutBuf(hHandle);
        }
        else
        {
             //printf("%s %d, MI_SYS_ChnOutputPortGetBuf error, %X\n", __func__, __LINE__, s32Ret);
            usleep(10 * 1000);//sleep 1 ms
        }
    }

    close(fd);
}

void *st_GetOutputDataThread(void * args)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_S32 s32Ret = MI_SUCCESS, s32VoChannel = 0;
    MI_S32 s32TimeOutMs = 20;
    MI_S32 s32ReadCnt = 0;
    FILE *fp = NULL;

    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_VPE;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = s32VoChannel;
    stChnPort.u32PortId = 2;
    printf("..st_GetOutputDataThread.s32VoChannel(%d)...\n", s32VoChannel);

    s32ReadCnt = 0;

    MI_SYS_SetChnOutputPortDepth(&stChnPort, 1, 3); //Default queue frame depth--->20
    fp = fopen("vpe_2.yuv","wb");
    while (!_bThreadRunning)
    {
        if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &hHandle))
        {
            if (E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == stBufInfo.stFrameData.ePixelFormat)
            {
                int size = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u16Width;

                if (0 == (s32ReadCnt++ % 30))
                {
                    if (fp)
                    {
                        fwrite(stBufInfo.stFrameData.pVirAddr[0], size, 1, fp);
                        fwrite(stBufInfo.stFrameData.pVirAddr[1], size/2, 1, fp);
                    }
                    printf("\t\t\t\t\t vif(%d) get buf cnt (%d)...w(%d)...h(%d)..\n", s32VoChannel, s32ReadCnt, stBufInfo.stFrameData.u16Width, stBufInfo.stFrameData.u16Height);
                }
            }
            else
            {
                int size = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];

                if (0 == (s32ReadCnt++ % 30))
                {
                    if (fp)
                    {
                        fwrite(stBufInfo.stFrameData.pVirAddr[0], size, 1, fp);
                    }
                    printf("\t\t\t\t\t vif(%d) get buf cnt (%d)...w(%d)...h(%d)..\n", s32VoChannel, s32ReadCnt, stBufInfo.stFrameData.u16Width, stBufInfo.stFrameData.u16Height);
                }
            }
            MI_SYS_ChnOutputPortPutBuf(hHandle);
            if(stBufInfo.bEndOfStream)
                break;
        }
        usleep(10*1000);
    }
    printf("\n\n");
    usleep(3000000);

    return NULL;
}

int ST_SplitWindow()
{
    ST_CaseDesc_t *pstCaseDesc = g_stVifCaseDesc;
    MI_U32 u32CaseSize = ARRAY_SIZE(g_stVifCaseDesc);
    MI_U32 u32CaseIndex = g_u32CaseIndex;
    MI_U32 u32CurSubCaseIndex = g_u32CurSubCaseIndex;
    MI_U32 u32LastSubCaseIndex = g_u32LastSubCaseIndex;
    MI_U32 u32CurWndNum = 0;
    MI_U32 u32LastWndNum = 0;
    MI_U32 i = 0;
    MI_U32 u32Square = 1;
    MI_U16 u16DispWidth = 0, u16DispHeight = 0;
    MI_S32 s32HdmiTiming = 0, s32DispTiming = 0;
    ST_Sys_BindInfo_t stBindInfo;
    MI_VPE_CHANNEL vpeChn = 0;
    MI_VPE_PortMode_t stVpeMode;
    MI_SYS_WindowRect_t stVpeRect;

    if (u32CurSubCaseIndex < 0 || u32LastSubCaseIndex < 0)
    {
        printf("error index\n");
        return 0;
    }

    /*
    VDEC->DIVP->VDISP->DISP

    (1) unbind VDEC DIVP
    (2) VDISP disable input port
    (3) divp set output port attr
    (4) vdisp set input port attr
    (5) bind vdec divp
    (6) enable vdisp input port

    VDEC->DIVP->VPE->VDISP->DISP
    (1) unbind vdec divp
    (2) unbind divp vpe
    (3) vdisp disable input port
    (4) divp set chn attr
    (5) vpe set port mode
    (6) vdisp set input port attr
    (7) bind vdec divp
    (8) bind divp vpe
    (9) enable vdisp input port
    */

    u32CurWndNum = pstCaseDesc[u32CaseIndex].stSubDesc[u32CurSubCaseIndex].u32WndNum;
    u32LastWndNum = pstCaseDesc[u32CaseIndex].u32ShowWndNum;

    if (u32CurWndNum == u32LastWndNum)
    {
        printf("same wnd num, skip\n");
        return 0;
    }
    else
    {
        printf("split window from %d to %d\n", u32LastWndNum, u32CurWndNum);
    }

    STCHECKRESULT(ST_GetTimingInfo(pstCaseDesc[u32CaseIndex].eDispoutTiming,
                &s32HdmiTiming, &s32DispTiming, &u16DispWidth, &u16DispHeight));

    printf("%s %d, u16DispWidth:%d,u16DispHeight:%d\n", __func__, __LINE__, u16DispWidth,
        u16DispHeight);

    // 1, unbind VDEC to DIVP
    for (i = 0; i < u32LastWndNum; i++)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = i;
        stBindInfo.stSrcChnPort.u32PortId = 0;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = i; //only equal zero
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate = 0;
        stBindInfo.u32DstFrmrate = 0;

        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }

    // 2, unbind DIVP to VPE
    for (i = 0; i < u32LastWndNum; i++)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = i;
        stBindInfo.stSrcChnPort.u32PortId = 0;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = i;
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;

        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }

    // 3, disable vdisp input port
    MI_VDISP_DEV vdispDev = MI_VDISP_DEV_0;
    MI_VDISP_PORT vdispPort = 0;
    MI_VDISP_InputPortAttr_t stInputPortAttr;

#if 1
    // stop divp and vpe
    for (i = 0; i < u32LastWndNum; i++)
    {
        STCHECKRESULT(MI_DIVP_StopChn(i));
    }

    for (i = 0; i < u32LastWndNum; i++)
    {
        vpeChn = i;
        STCHECKRESULT(MI_VPE_DisablePort(vpeChn, 0));
    }
#endif

    for (i = 0; i < u32LastWndNum; i++)
    {
        vdispPort = i;
        STCHECKRESULT(MI_VDISP_DisableInputPort(vdispDev, vdispPort));
    }

    if (u32CurWndNum <= 1)
    {
        u32Square = 1;
    }
    else if (u32CurWndNum <= 4)
    {
        u32Square = 2;
    }
    else if (u32CurWndNum <= 9)
    {
        u32Square = 3;
    }
    else if (u32CurWndNum <= 16)
    {
        u32Square = 4;
    }

    // 4, divp set output port chn attr


    // 5, set vpe port mode
    for (i = 0; i < u32CurWndNum; i++)
    {
        vpeChn = i;

        memset(&stVpeMode, 0, sizeof(MI_VPE_PortMode_t));
        ExecFunc(MI_VPE_GetPortMode(vpeChn, 0, &stVpeMode), MI_VPE_OK);
        stVpeMode.u16Width = ALIGN_BACK(u16DispWidth / u32Square, 2);
        stVpeMode.u16Height = ALIGN_BACK(u16DispHeight / u32Square, 2);
        ExecFunc(MI_VPE_SetPortMode(vpeChn, 0, &stVpeMode), MI_VPE_OK);
    }

    // 6, set vdisp input port attribute
    for (i = 0; i < u32CurWndNum; i++)
    {
        vdispPort = i;

        memset(&stInputPortAttr, 0, sizeof(MI_VDISP_InputPortAttr_t));

        STCHECKRESULT(MI_VDISP_GetInputPortAttr(vdispDev, vdispPort, &stInputPortAttr));

        stInputPortAttr.u32OutX = ALIGN_BACK((u16DispWidth / u32Square) * (i % u32Square), 2);
        stInputPortAttr.u32OutY = ALIGN_BACK((u16DispHeight / u32Square) * (i / u32Square), 2);
        stInputPortAttr.u32OutWidth = ALIGN_BACK(u16DispWidth / u32Square, 2);
        stInputPortAttr.u32OutHeight = ALIGN_BACK(u16DispHeight / u32Square, 2);

        printf("%s %d, u32OutWidth:%d,u32OutHeight:%d,u32Square:%d\n", __func__, __LINE__, stInputPortAttr.u32OutWidth,
             stInputPortAttr.u32OutHeight, u32Square);
        STCHECKRESULT(MI_VDISP_SetInputPortAttr(vdispDev, vdispPort, &stInputPortAttr));
    }

    // start divp and vpe
    for (i = 0; i < u32CurWndNum; i++)
    {
        STCHECKRESULT(MI_DIVP_StartChn(i));
    }

    for (i = 0; i < u32CurWndNum; i++)
    {
        vpeChn = i;
        STCHECKRESULT(MI_VPE_EnablePort(vpeChn, 0));
    }

    // 9, enable vdisp input port
    for (i = 0; i < u32CurWndNum; i++)
    {
        vdispPort = i;
        STCHECKRESULT(MI_VDISP_EnableInputPort(vdispDev, vdispPort));
    }

    // 7, bind VDEC to DIVP
    for (i = 0; i < u32CurWndNum; i++)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = i;
        stBindInfo.stSrcChnPort.u32PortId = 0;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = i;
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate = 0;
        stBindInfo.u32DstFrmrate = 0;

        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }

    // 8, bind DIVP to VPE
    for (i = 0; i < u32CurWndNum; i++)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = i;
        stBindInfo.stSrcChnPort.u32PortId = 0;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = i; //only equal zero
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;

        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }

    pstCaseDesc[u32CaseIndex].u32ShowWndNum =
        pstCaseDesc[u32CaseIndex].stSubDesc[u32CurSubCaseIndex].u32WndNum;
    g_u32LastSubCaseIndex = g_u32CurSubCaseIndex;

    return 0;
}

MI_S32 ST_SubExit()
{
    MI_U32 u32WndNum = 0, u32ShowWndNum;
    MI_S32 i = 0;
    ST_Sys_BindInfo_t stBindInfo;
    ST_CaseDesc_t *pstCaseDesc = g_stVifCaseDesc;
    MI_U32 u32CaseSize = ARRAY_SIZE(g_stVifCaseDesc);
    u32WndNum = pstCaseDesc[g_u32CaseIndex].stDesc.u32WndNum;
    u32ShowWndNum = pstCaseDesc[g_u32CaseIndex].u32ShowWndNum;

    /************************************************
    step1:  unbind VIF to VPE
    *************************************************/
    for (i = 0; i < u32WndNum; i++)
    {
        STCHECKRESULT(ST_Vif_StopPort(stVifChnCfg[i].u8ViChn, stVifChnCfg[i].u8ViPort));
    }
    for (i = 0; i < u32ShowWndNum; i++)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
        stBindInfo.stSrcChnPort.u32DevId = stVifChnCfg[i].u8ViDev;
        stBindInfo.stSrcChnPort.u32ChnId = stVifChnCfg[i].u8ViChn;
        stBindInfo.stSrcChnPort.u32PortId = stVifChnCfg[i].u8ViPort;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = i;
        stBindInfo.stDstChnPort.u32PortId = 0;

        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }

    /************************************************
    step4:  unbind VPE to VDISP
    *************************************************/
    for (i = 0; i < u32ShowWndNum; i++)
    {
        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = i;
        stBindInfo.stSrcChnPort.u32PortId = 0;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = i;

        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }

    /************************************************
    step5:  destroy vif  vpe vdisp disp
    *************************************************/
    for (i = 0; i < u32ShowWndNum; i++)
    {
        STCHECKRESULT(ST_Vpe_StopPort(i, 0));
#ifdef SUPPORT_CDNN
        STCHECKRESULT(ST_Vpe_StopPort(i, SUB_VENC_PORT));  // for CDNN
#endif
        STCHECKRESULT(ST_Vpe_StopChannel(i));
        STCHECKRESULT(ST_Vpe_DestroyChannel(i));
    }

#ifdef SUPPORT_CDNN
    ST_CEVA_CDNN_Stop();
    ST_CEVA_CDNN_Deinit();
#endif

    ST_CEVA_CDNN_Stop();
    ST_CEVA_CDNN_Deinit();

    STCHECKRESULT(ST_Vif_DisableDev(0));//0 1 2 3?

    STCHECKRESULT(ST_Disp_DeInit(ST_DISP_DEV0, 0 , u32ShowWndNum)); //disp input port 0
    STCHECKRESULT(ST_Hdmi_DeInit(E_MI_HDMI_ID_0));
    STCHECKRESULT(ST_Fb_DeInit());
    STCHECKRESULT(ST_Sys_Exit());

    return MI_SUCCESS;
}

int ST_ChangeDisplayTiming()
{
    ST_CaseDesc_t *pstCaseDesc = g_stVifCaseDesc;
    MI_U32 u32CaseSize = ARRAY_SIZE(g_stVifCaseDesc);
    MI_U32 u32CaseIndex = g_u32CaseIndex;
    MI_U32 u32CurSubCaseIndex = g_u32CurSubCaseIndex;
    MI_U32 u32LastSubCaseIndex = g_u32LastSubCaseIndex;
    ST_DispoutTiming_e eLastDispoutTiming = E_ST_TIMING_MAX;
    ST_DispoutTiming_e eCurDispoutTiming = E_ST_TIMING_MAX;
    MI_S32 s32LastHdmiTiming, s32CurHdmiTiming;
    MI_S32 s32LastDispTiming, s32CurDispTiming;
    MI_U16 u16LastDispWidth = 0, u16LastDispHeight = 0;
    MI_U16 u16CurDispWidth = 0, u16CurDispHeight = 0;
    MI_U32 u32CurWndNum = 0;
    MI_U32 u32TotalWnd = 0;
    MI_U32 i = 0;
    MI_U32 u32Square = 0;
    ST_Sys_BindInfo_t stBindInfo;

    if (u32CurSubCaseIndex < 0 || u32LastSubCaseIndex < 0)
    {
        printf("error index\n");
        return 0;
    }

    eCurDispoutTiming = pstCaseDesc[u32CaseIndex].stSubDesc[u32CurSubCaseIndex].eDispoutTiming;
    eLastDispoutTiming = pstCaseDesc[u32CaseIndex].eDispoutTiming;

    if (eCurDispoutTiming == eLastDispoutTiming)
    {
        printf("the same timing, skip\n");
        return 0;
    }

    u32CurWndNum = pstCaseDesc[u32CaseIndex].u32ShowWndNum;

    STCHECKRESULT(ST_GetTimingInfo(eCurDispoutTiming,
                &s32CurHdmiTiming, &s32CurDispTiming, &u16CurDispWidth, &u16CurDispHeight));

    STCHECKRESULT(ST_GetTimingInfo(eLastDispoutTiming,
                &s32LastHdmiTiming, &s32LastDispTiming, &u16LastDispWidth, &u16LastDispHeight));

    printf("change from %dx%d to %dx%d\n", u16LastDispWidth, u16LastDispHeight, u16CurDispWidth,
                u16CurDispHeight);

    /*
    (1), stop HDMI
    (2), stop VDISP
    (3), stop DISP
    (4), set vpe port mode
    (5), set vdisp input port chn attr
    (6), start disp
    (7), start vdisp
    (8), star HDMI
    */

    if (u32CurWndNum <= 1)
    {
        u32Square = 1;
    }
    else if (u32CurWndNum <= 4)
    {
        u32Square = 2;
    }
    else if (u32CurWndNum <= 9)
    {
        u32Square = 3;
    }
    else if (u32CurWndNum <= 16)
    {
        u32Square = 4;
    }

    // stop hdmi
    ExecFunc(MI_HDMI_Stop(E_MI_HDMI_ID_0), MI_SUCCESS);

    // stop vdisp
    MI_VDISP_DEV vdispDev = MI_VDISP_DEV_0;
    MI_S32 s32FrmRate = 30;
    MI_S32 s32OutputPort = 0;
    MI_VDISP_PORT vdispPort = 0;

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDISP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));

    STCHECKRESULT(ST_Vdisp_StopDevice(vdispDev));

    // stop disp
    STCHECKRESULT(ST_Disp_DeInit(ST_DISP_DEV0, 0, DISP_MAX_CHN));

    // set vpe port mode
    MI_VPE_CHANNEL vpeChn = 0;
    MI_VPE_PortMode_t stVpeMode;
    for (i = 0; i < u32CurWndNum; i++)
    {
        vpeChn = i;

        memset(&stVpeMode, 0, sizeof(MI_VPE_PortMode_t));
        ExecFunc(MI_VPE_GetPortMode(vpeChn, 0, &stVpeMode), MI_VPE_OK);
        stVpeMode.u16Width = ALIGN_BACK(u16CurDispWidth / u32Square, 2);
        stVpeMode.u16Height = ALIGN_BACK(u16CurDispHeight / u32Square, 2);
        ExecFunc(MI_VPE_SetPortMode(vpeChn, 0, &stVpeMode), MI_VPE_OK);
    }
    // start vdisp
    for (i = 0; i < u32CurWndNum; i++)
    {
        STCHECKRESULT(MI_VDISP_DisableInputPort(vdispDev, vdispPort));
    }
    ST_Rect_t stdispRect, stRect;
    stdispRect.u16PicW = u16CurDispWidth;
    stdispRect.u16PicH = u16CurDispHeight;
    STCHECKRESULT(ST_Vdisp_SetOutputPortAttr(vdispDev, s32OutputPort, &stdispRect, s32FrmRate, 1));

    // set vdisp input port chn attr
    MI_VDISP_InputPortAttr_t stInputPortAttr;
    for (i = 0; i < u32CurWndNum; i++)
    {
        vdispPort = i;

        memset(&stInputPortAttr, 0, sizeof(MI_VDISP_InputPortAttr_t));

        STCHECKRESULT(MI_VDISP_GetInputPortAttr(vdispDev, vdispPort, &stInputPortAttr));

        stInputPortAttr.u32OutX = ALIGN_BACK((u16CurDispWidth / u32Square) * (i % u32Square), 2);
        stInputPortAttr.u32OutY = ALIGN_BACK((u16CurDispHeight / u32Square) * (i / u32Square), 2);
        stInputPortAttr.u32OutWidth = ALIGN_BACK(u16CurDispWidth / u32Square, 2);
        stInputPortAttr.u32OutHeight = ALIGN_BACK(u16CurDispHeight / u32Square, 2);

        // printf("%s %d, u32OutWidth:%d,u32OutHeight:%d,u32Square:%d\n", __func__, __LINE__, stInputPortAttr.u32OutWidth,
        //     stInputPortAttr.u32OutHeight, u32Square);
        STCHECKRESULT(MI_VDISP_SetInputPortAttr(vdispDev, vdispPort, &stInputPortAttr));
    }
    for (i = 0; i < u32CurWndNum; i++)
    {
        STCHECKRESULT(MI_VDISP_EnableInputPort(vdispDev, vdispPort));
    }

    STCHECKRESULT(ST_Vdisp_StartDevice(vdispDev));

    // start disp
    STCHECKRESULT(ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, s32CurDispTiming));
    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDISP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

    // start HDMI
    STCHECKRESULT(ST_Hdmi_Start(E_MI_HDMI_ID_0, s32CurHdmiTiming));

    pstCaseDesc[u32CaseIndex].eDispoutTiming = eCurDispoutTiming;

    g_u32LastSubCaseIndex = g_u32CurSubCaseIndex;
}

void ST_WaitSubCmd(void)
{
    ST_CaseDesc_t *pstCaseDesc = g_stVifCaseDesc;
    char szCmd[16];
    MI_U32 index = 0;
    MI_U32 u32CaseIndex = g_u32CaseIndex;
    MI_U32 u32SubCaseSize = pstCaseDesc[u32CaseIndex].u32SubCaseNum;

    while (!g_subExit)
    {
        ST_CaseSubUsage();

        fgets((szCmd), (sizeof(szCmd) - 1), stdin);

        index = atoi(szCmd);

        if (index <= 0 || index > u32SubCaseSize)
        {
            continue;
        }

        g_u32CurSubCaseIndex = index - 1;

        if (pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].eDispoutTiming > 0)
        {
            ST_ChangeDisplayTiming(); //change timing
        }
        else
        {
            if (0 == (strncmp(pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "exit", 4)))
            {
                ST_SubExit();
                return;
            }
            else if (0 == (strncmp(pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "zoom", 4)))
            {
            }
            else
            {
                if (pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].u32WndNum > 0)
                {
                    ST_SplitWindow(); //switch screen
                }
            }
        }
    }
}

MI_S32 ST_VifToDisp(MI_S32 s32CaseIndex)
{
    MI_VIF_DEV VifDevId = 0;
    MI_U16 u16DispLayerW = 1920;
    MI_U16 u16DispLayerH = 1080;
    ST_VPE_ChannelInfo_t stVpeChannelInfo;
    ST_VIF_PortInfo_t stVifPortInfoInfo;
    ST_Rect_t stdispRect = {0, 0, 1920, 1080};

#ifdef SUPPORT_VIDEO_PREVIEW
    ST_VPE_PortInfo_t stVpePortInfo;        // vpe->disp    port3
#endif
#ifdef SUPPORT_VIDEO_ENCODE
    ST_VPE_PortInfo_t stVpeEncodePortInfo;  // vpe->venc    port0
#endif
#ifdef SUPPORT_CDNN
    ST_VPE_PortInfo_t stVpeCdnnPortInfo;    // vpe->cdnn    port1
    MI_SYS_ChnPort_t stPreviewPort;
    MI_SYS_ChnPort_t stCdnnPort;
#endif

    ST_CEVA_CDNN_PortInfo_t stCvPortInfo;        // ceva virtual port
    ST_CEVA_CDNN_ResolutionMap_t stCvMap;

    ST_Rect_t stRect;
    ST_Sys_BindInfo_t stBindInfo;
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;
    MI_HDMI_TimingType_e eHdmiTiming = E_MI_HDMI_TIMING_1080_60P;
    MI_DISP_OutputTiming_e eDispoutTiming = E_MI_DISP_OUTPUT_1080P60;
    MI_S32 s32CapChnNum = 0, s32DispChnNum = 0, i = 0;
    MI_S32 s32DevNum = 0;
    ST_CaseDesc_t *pstCaseDesc = g_stVifCaseDesc;
    MI_U32 u32ArraySize = ARRAY_SIZE(g_stVifCaseDesc);
    MI_U32 u32Square = 0;
    MI_U32 u32SubCaseSize = pstCaseDesc[s32CaseIndex].u32SubCaseNum;
    MI_U32 u32WndNum = pstCaseDesc[s32CaseIndex].stDesc.u32WndNum;
    ST_Rect_t stDispWndRect[16] = {0};
    MI_S32 s32AdId[4];
    MI_S32 s32AdWorkMode = 0;

    s32DispChnNum = pstCaseDesc[s32CaseIndex].stDesc.u32WndNum;
    s32CapChnNum = s32DispChnNum;
    STCHECKRESULT(ST_GetTimingInfo(pstCaseDesc[s32CaseIndex].eDispoutTiming, (MI_S32 *)&eHdmiTiming,
        (MI_S32 *)&eDispoutTiming, (MI_U16*)&stdispRect.u16PicW, (MI_U16*)&stdispRect.u16PicH));
    for (i = 0; i < 4; i++)
    {
        s32AdId[i] = 0;
    }
    if (u32WndNum <= 1)
    {
        u32Square = 1;
    }
    else if (u32WndNum <= 4)
    {
        u32Square = 2;
    }
    else if (u32WndNum <= 9)
    {
        u32Square = 3;
    }
    else if (u32WndNum <= 16)
    {
        u32Square = 4;
    }

    if (6 == u32WndNum) //irrgular split
    {
        MI_U16 u16DivW = ALIGN_BACK(stdispRect.u16PicW / 3, 2);
        MI_U16 u16DivH = ALIGN_BACK(stdispRect.u16PicH / 3, 2);
        ST_Rect_t stRectSplit[] =
        {
            {0, 0, 2, 2},
            {2, 0, 1, 1},
            {2, 1, 1, 1},
            {0, 2, 1, 1},
            {1, 2, 1, 1},
            {2, 2, 1, 1},
        };//3x3 split div

        for (i = 0; i < u32WndNum; i++)
        {
            stDispWndRect[i].s32X = stRectSplit[i].s32X * u16DivW;
            stDispWndRect[i].s32Y = stRectSplit[i].s32Y * u16DivH;
            stDispWndRect[i].u16PicW = stRectSplit[i].u16PicW * u16DivW;
            stDispWndRect[i].u16PicH = stRectSplit[i].u16PicH * u16DivH;
        }
    }
    else if (8 == u32WndNum) //irrgular split
    {
        MI_U16 u16DivW = ALIGN_BACK(stdispRect.u16PicW / 4, 2);
        MI_U16 u16DivH = ALIGN_BACK(stdispRect.u16PicH / 4, 2);
        ST_Rect_t stRectSplit[] =
        {
            {0, 0, 3, 3},
            {3, 0, 1, 1},
            {3, 1, 1, 1},
            {3, 2, 1, 1},
            {0, 3, 1, 1},
            {1, 3, 1, 1},
            {2, 3, 1, 1},
            {3, 3, 1, 1},
        };//4x4 split div

        for (i = 0; i < u32WndNum; i++)
        {
            stDispWndRect[i].s32X = stRectSplit[i].s32X * u16DivW;
            stDispWndRect[i].s32Y = stRectSplit[i].s32Y * u16DivH;
            stDispWndRect[i].u16PicW = stRectSplit[i].u16PicW * u16DivW;
            stDispWndRect[i].u16PicH = stRectSplit[i].u16PicH * u16DivH;
        }
    }
    else
    {
        for (i = 0; i < u32WndNum; i++)
        {
            stDispWndRect[i].s32X    = ALIGN_BACK((stdispRect.u16PicW / u32Square) * (i % u32Square), 2);
            stDispWndRect[i].s32Y    = ALIGN_BACK((stdispRect.u16PicH / u32Square) * (i / u32Square), 2);
            stDispWndRect[i].u16PicW = ALIGN_BACK((stdispRect.u16PicW / u32Square), 2);
            stDispWndRect[i].u16PicH = ALIGN_BACK((stdispRect.u16PicH / u32Square), 2);
        }
    }
    /************************************************
    Step1:  init SYS
    *************************************************/
    printf("s32DispChnNum=%d, s32CapChnNum=%d\n", s32DispChnNum, s32CapChnNum);
    printf("Step 1: int SYS....................\n\n\n");
    STCHECKRESULT(ST_Sys_Init());

    // ExecFunc(vif_i2c_init(), 0);
    /************************************************
    Step2:  init HDMI
    *************************************************/
    printf("Step 2: init HDMI....................\n\n\n");
    // STCHECKRESULT(ST_Hdmi_Init());

    STCHECKRESULT(ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, eDispoutTiming)); //Dispout timing

    /************************************************
    Step3:  init VIF
    *************************************************/
    printf("Step 3: init VIF....................\n\n]n");
    s32DevNum = s32CapChnNum / 4;

    /*
        case 0: AD_A CHN0 1*FHD
        case 1: AD_B CHN0 1*FHD
        case 2: AD_C CHN0 1*FHD
        case 3: AD_D CHN0 1*FHD
        case 4: AD_A-AD_D CHN0/CHN4/CHN8/CHN12
        case 5: AD_A 4*D1 CHN0/CHN4/CHN8/CHN12
    */
    switch (s32CaseIndex)
    {
        case 0:
            s32AdId[0] = 1;
            stVifChnCfg[0].u8ViDev = 0;
            stVifChnCfg[0].u8ViChn = 0;
            stVifChnCfg[1].u8ViDev = 0;
            stVifChnCfg[1].u8ViChn = 0;
            s32AdWorkMode = SAMPLE_VI_MODE_MIPI_1_1080P_VPE;
            break;
        case 1:
            s32AdId[0] = 1;
            stVifChnCfg[0].u8ViDev = 0;
            stVifChnCfg[0].u8ViChn = 0;
            stVifChnCfg[1].u8ViDev = 0;
            stVifChnCfg[1].u8ViChn = 0;
            s32AdWorkMode = SAMPLE_VI_MODE_MIPI_1_1080P_VENC;
            break;
        case 2:
            s32AdId[0] = 1;
            stVifChnCfg[0].u8ViDev = 0;
            stVifChnCfg[0].u8ViChn = 0;
            stVifChnCfg[1].u8ViDev = 0;
            stVifChnCfg[1].u8ViChn = 0;
            s32AdWorkMode = SAMPLE_VI_MODE_MIPI_1_1080P_VENC;
            break;
        default:
            ST_DBG("Unkown test case(%d)!\n", s32CaseIndex);
            return 0;
    }

    g_s32AdWorkMode = s32AdWorkMode;
    printf("work mode is %d\n", g_s32AdWorkMode);

    MI_SYS_ChnPort_t stChnPort;
    for (i = 0; i < MAX_VIF_DEV_NUM; i++) //init vif device
    {
        if (s32AdId[i])
        {
            ST_DBG("ST_Vif_CreateDev....DEV(%d)...s32AdWorkMode(%d)...\n", i, s32AdWorkMode);
            STCHECKRESULT(ST_Vif_CreateDev(i, s32AdWorkMode));
            printf("vif createDev dev_%d\n", i);
        }
    }

    for (i = 0; i < s32CapChnNum; i++) //init vif channel
    {
        memset(&stVifPortInfoInfo, 0x0, sizeof(ST_VIF_PortInfo_t));
        stVifPortInfoInfo.u32RectX = 0;
        stVifPortInfoInfo.u32RectY = 0;
        stVifPortInfoInfo.u32RectWidth = pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth;
        stVifPortInfoInfo.u32RectHeight = pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight;
        stVifPortInfoInfo.u32DestWidth = pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth;
        stVifPortInfoInfo.u32DestHeight = pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight;
        stVifPortInfoInfo.ePixFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
        STCHECKRESULT(ST_Vif_CreatePort(stVifChnCfg[i].u8ViChn, 0, &stVifPortInfoInfo));
        printf("vif createPort chn_%d, port_0, x_0, y_0, w_%d, h_%d, dst_w_%d, dst_h_%d\n", stVifChnCfg[i].u8ViChn
               , stVifPortInfoInfo.u32RectWidth, stVifPortInfoInfo.u32RectHeight, stVifPortInfoInfo.u32DestWidth
               , stVifPortInfoInfo.u32DestHeight);

        memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
        stChnPort.eModId = E_MI_MODULE_ID_VIF;
        stChnPort.u32DevId = 0;
        stChnPort.u32ChnId = stVifChnCfg[i].u8ViChn;
        stChnPort.u32PortId = 0;
        MI_SYS_SetChnOutputPortDepth(&stChnPort, 1, 3);
        printf("vif set outputDepth port_0 depth 1 3\n");

        ST_DBG("============vif channel(%d) x(%d)-y(%d)-w(%d)-h(%d)..\n", i, stVifPortInfoInfo.u32RectX, stVifPortInfoInfo.u32RectY,
            stVifPortInfoInfo.u32RectWidth, stVifPortInfoInfo.u32RectHeight);
    }

    /************************************************
    Step CEVA:  init CEVA   (single handle)
    *************************************************/
#ifdef SUPPORT_CDNN
    DEBUG_PRINT("Step ceva: init ceva\n\n\n");
    ST_CEVA_CDNN_Init();
#endif

    printf("Step ceva: init ceva\n\n\n");
    ST_CEVA_CDNN_Init();

    /************************************************
    Step4:  init VPE
    *************************************************/
    printf("Step 4: init VPE....................\n\n\n");
    MI_VPE_RunningMode_e eRunningMode = ST_Vpe_GetRunModeByWorkMode(s32AdWorkMode);

    for (i = 0; i < s32CapChnNum; i++)
    {
        if (eRunningMode == E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE)
        {
            stVpeChannelInfo.u16VpeMaxW = 1920;
            stVpeChannelInfo.u16VpeMaxH = 1080; // max res support to FHD
        }
        else if (eRunningMode == E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
        {
            stVpeChannelInfo.u16VpeMaxW = 3840;
            stVpeChannelInfo.u16VpeMaxH = 2160;
        }
        stVpeChannelInfo.u32X = 0;
        stVpeChannelInfo.u32Y = 0;
        stVpeChannelInfo.u16VpeCropW = pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth;
        stVpeChannelInfo.u16VpeCropH = pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight;
        stVpeChannelInfo.eRunningMode = eRunningMode;
        stVpeChannelInfo.eFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
        STCHECKRESULT(ST_Vpe_CreateChannel(i, &stVpeChannelInfo));
        printf("vpe createChn chn_%d MaxW %d MaxH %d\n", i, stVpeChannelInfo.u16VpeMaxW, stVpeChannelInfo.u16VpeMaxH);
        //STCHECKRESULT(ST_Vpe_StartChannel(i));


#ifdef SUPPORT_VIDEO_PREVIEW
        memset(&stVpePortInfo, 0, sizeof(stVpePortInfo));
        stVpePortInfo.DepVpeChannel = i;
        stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        stVpePortInfo.u16OutputWidth = stDispWndRect[i].u16PicW;
        stVpePortInfo.u16OutputHeight = stDispWndRect[i].u16PicH;
        ST_DBG("vpe chn:%d, u16OutputWidth:%d,u16OutputHeight:%d\n", i, stVpePortInfo.u16OutputWidth,
            stVpePortInfo.u16OutputHeight);
        STCHECKRESULT(ST_Vpe_CreatePort(3, &stVpePortInfo)); //default support port0 --->>> vdisp
        printf("vpe createPort chn_%d, port_%d fmt_%d w_%d, h_%d\n", i, 3, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV
               , stVpePortInfo.u16OutputWidth, stVpePortInfo.u16OutputHeight);
#endif

#ifdef SUPPORT_VIDEO_ENCODE
        stVpeEncodePortInfo.DepVpeChannel = i;
        stVpeEncodePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        if (eRunningMode == E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
        {
            if (s32AdWorkMode == SAMPLE_VI_MODE_MIPI_1_1080P_VPE)
            {
                stVpeEncodePortInfo.u16OutputWidth = 640;
                stVpeEncodePortInfo.u16OutputHeight = 480;
            }
            else
            {
                stVpeEncodePortInfo.u16OutputWidth = ALIGN_N(stDispWndRect[i].u16PicW, 32);
                stVpeEncodePortInfo.u16OutputHeight = ALIGN_N(stDispWndRect[i].u16PicH, 8);
            }
        }
        else
        {
            stVpeEncodePortInfo.u16OutputWidth = ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
            stVpeEncodePortInfo.u16OutputHeight = ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 8);
        }
        STCHECKRESULT(ST_Vpe_CreatePort(0, &stVpeEncodePortInfo)); //default support port2 --->>> venc

        ST_DBG("============vpe channel(%d) x(%d)-y(%d)-w(%d)-h(%d).outw(%d)-outh(%d).\n", i, stVpeChannelInfo.u32X, stVpeChannelInfo.u32Y,
            stVpeChannelInfo.u16VpeCropW, stVpeChannelInfo.u16VpeCropH, stVpeEncodePortInfo.u16OutputWidth, stVpeEncodePortInfo.u16OutputHeight);

#endif

#ifdef SUPPORT_CDNN
        memset(&stVpeCdnnPortInfo, 0, sizeof(stVpeCdnnPortInfo));
        stVpeCdnnPortInfo.DepVpeChannel = i;
        stVpeCdnnPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        stVpeCdnnPortInfo.u16OutputWidth = FRAME_WIDTH;  //1920;
        stVpeCdnnPortInfo.u16OutputHeight = FRAME_HEIGHT; //1080;
        ST_DBG("vpe chn:%d, u16OutputWidth:%d,u16OutputHeight:%d\n", i, stVpeCdnnPortInfo.u16OutputWidth,
            stVpeCdnnPortInfo.u16OutputHeight);
        STCHECKRESULT(ST_Vpe_CreatePort(1, &stVpeCdnnPortInfo)); //default support port1 --->>> vdisp, used for CDNN
        DEBUG_PRINT("vpe createPort chn_%d, port_%d fmt_%d w_%d, h_%d\n", i, 1, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV
               , stVpeCdnnPortInfo.u16OutputWidth, stVpeCdnnPortInfo.u16OutputHeight);

        stPreviewPort.eModId = E_MI_MODULE_ID_VPE;
        stPreviewPort.u32DevId = 0;
        stPreviewPort.u32ChnId = i;
        stPreviewPort.u32PortId = 3;

        stCdnnPort.eModId = E_MI_MODULE_ID_VPE;
        stCdnnPort.u32DevId = 0;
        stCdnnPort.u32ChnId = i;
        stCdnnPort.u32PortId = 1;

        _gastFrameInfo[i].u16PreviewWidth = stVpePortInfo.u16OutputWidth;
        _gastFrameInfo[i].u16PreviewHeight = stVpePortInfo.u16OutputHeight;
        _gastFrameInfo[i].u16CdnnWidth = stVpeCdnnPortInfo.u16OutputWidth;
        _gastFrameInfo[i].u16CdnnHeight = stVpeCdnnPortInfo.u16OutputHeight;

        ST_CEVA_RegisterSevr(stCdnnPort);
#endif
        memset(&stCvPortInfo, 0, sizeof(ST_CEVA_CDNN_PortInfo_t));
        memset(&stCvMap, 0, sizeof(ST_CEVA_CDNN_ResolutionMap_t));
        stCvPortInfo.stPortInfo.DepVpeChannel = i;
        stCvPortInfo.stPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        stCvPortInfo.stPortInfo.u16OutputWidth = 608;  //1920;
        stCvPortInfo.stPortInfo.u16OutputHeight = 352; //1080;
        stCvPortInfo.stChnPort.eModId = E_MI_MODULE_ID_VPE;
        stCvPortInfo.stChnPort.u32DevId = 0;
        stCvPortInfo.stChnPort.u32ChnId = i;
        stCvPortInfo.stChnPort.u32PortId = 1;
        stCvMap.u16SrcWidth = stVpePortInfo.u16OutputWidth;
        stCvMap.u16SrcHeight = stVpePortInfo.u16OutputHeight;
        stCvMap.u16DstWidth = stCvPortInfo.stPortInfo.u16OutputWidth;
        stCvMap.u16DstHeight = stCvPortInfo.stPortInfo.u16OutputHeight;
        STCHECKRESULT(ST_Vpe_CreatePort(1, &stCvPortInfo.stPortInfo));
        printf("ST_CEVA_CDNN_RegisterChn\n");
        ST_CEVA_CDNN_RegisterChn(&stCvPortInfo, &stCvMap);

        STCHECKRESULT(ST_Vpe_StartChannel(i));
        printf("vpe startChn chn_%d\n", i);
    }

    /************************************************
    Step6:  init DISP
    *************************************************/
    // STCHECKRESULT(ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, eDispoutTiming)); //Dispout timing

#ifdef SUPPORT_VIDEO_PREVIEW
    ST_DispChnInfo_t stDispChnInfo;
    stDispChnInfo.InputPortNum = s32DispChnNum;
    for (i = 0; i < s32DispChnNum; i++)
    {
        stDispChnInfo.stInputPortAttr[i].u32Port = i;
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16X = stDispWndRect[i].s32X;
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Y = stDispWndRect[i].s32Y;
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Width = stDispWndRect[i].u16PicW;
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Height = stDispWndRect[i].u16PicH;
        ST_DBG("===========disp channel(%d) x(%d)-y(%d)-w(%d)-h(%d)...\n", i,
            stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16X,
            stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Y,
            stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Width,
            stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Height);
    }
    STCHECKRESULT(ST_Disp_ChnInit(0, &stDispChnInfo));
#endif

    // must init after disp
    // ST_Fb_Init();
    // ST_FB_Show(FALSE);

    /************************************************
    Step7:  Bind VIF->VPE
    *************************************************/
    printf("Step 7: bind vif&vpe....................\n\n\n");
    for (i = 0; i < s32CapChnNum; i++)
    {
        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
        stBindInfo.stSrcChnPort.u32DevId = 0; //VIF dev == 0
        stBindInfo.stSrcChnPort.u32ChnId = stVifChnCfg[i].u8ViChn;
        stBindInfo.stSrcChnPort.u32PortId = 0; //Main stream
        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = i;
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
        printf("sysBind vif_%d_%d_%d vpe_%d_%d_%d srcRate_%d dstRate_%d\n", 0, stVifChnCfg[i].u8ViChn, 0
                    , 0, i, 0, 30, 30);
    }

#ifdef SUPPORT_VIDEO_ENCODE //Create Video encode Channel
// #if 0
    // main+sub venc
    MI_VENC_CHN VencChn = 0;
    MI_S32 s32Ret = 0;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_SYS_ChnPort_t stVencChnOutputPort;
    MI_U32 u32DevId = 0;

    ST_DBG("total chn num:%d\n", s32CapChnNum);

    if (s32AdWorkMode == SAMPLE_VI_MODE_MIPI_1_1080P_VENC)
    {
        for (i = 0; i < s32CapChnNum; i ++)
        {
            VencChn = i;

            memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
            memset(&stVencChnOutputPort, 0, sizeof(MI_SYS_ChnPort_t));

            printf("%s %d, chn:%d,eType:%d, eRunningMode:%d\n", __func__, __LINE__, VencChn,
                pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVencChnArg.eType, eRunningMode);
            if (E_MI_VENC_MODTYPE_H264E == pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.eType)
            {
                stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
                stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2; // not support B frame
                if (eRunningMode == E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
                {
                    stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = 640;
                        // ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                    stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = 480;
                        // ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 8);
                    stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = 640;
                        // ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                    stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = 480;
                        // ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 8);
                }
                else
                {
                    stChnAttr.stVeAttr.stAttrH264e.u32PicWidth =
                        ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                    stChnAttr.stVeAttr.stAttrH264e.u32PicHeight =
                        ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 32);
                    stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth =
                        ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                    stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight =
                        ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 32);
                }

                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum =
                        pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.s32FrmRate;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;

                stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 25;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 25;
            }
            else if (E_MI_VENC_MODTYPE_H265E == pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.eType)
            {
                stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
                stChnAttr.stVeAttr.stAttrH265e.u32PicWidth =
                    ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                stChnAttr.stVeAttr.stAttrH265e.u32PicHeight =
                    ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 32);
                stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth =
                    ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight =
                    ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 32);
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum =
                    pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.s32FrmRate;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32Gop = 30;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32IQp = 25;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32PQp = 25;
            }
            else if (E_MI_VENC_MODTYPE_JPEGE == pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.eType)
            {
                stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;

                if (eRunningMode == E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
                {
                    stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = 640;
                        // ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                    stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = 480;
                        // ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 32);
                    stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = 640;
                        // ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                    stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = 480;
                        // ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 32);
                }
                else
                {
                stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth =
                    ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight =
                    ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 32);
                stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth =
                    ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapWidth, 32);
                stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight =
                    ALIGN_N(pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.u16CapHeight, 32);
                }
            }
            s32Ret = MI_VENC_CreateChn(VencChn, &stChnAttr);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s %d, MI_VENC_CreateChn %d error, %X\n", __func__, __LINE__, VencChn, s32Ret);
            }
            if (E_MI_VENC_MODTYPE_JPEGE == pstCaseDesc[s32CaseIndex].stCaseArgs[i].uChnArg.stVifChnArg.eType)
            {
                MI_VENC_ParamJpeg_t stParamJpeg;

                memset(&stParamJpeg, 0, sizeof(stParamJpeg));
                s32Ret = MI_VENC_GetJpegParam(VencChn, &stParamJpeg);
                if(s32Ret != MI_SUCCESS)
                {
                    return s32Ret;
                }
                printf("Get Qf:%d\n", stParamJpeg.u32Qfactor);

                stParamJpeg.u32Qfactor = 100;
                s32Ret = MI_VENC_SetJpegParam(VencChn, &stParamJpeg);
                if(s32Ret != MI_SUCCESS)
                {
                    return s32Ret;
                }
            }

            s32Ret = MI_VENC_GetChnDevid(VencChn, &u32DevId);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s %d, MI_VENC_GetChnDevid %d error, %X\n", __func__, __LINE__, VencChn, s32Ret);
            }
            stVencChnOutputPort.u32DevId = u32DevId;
            stVencChnOutputPort.eModId = E_MI_MODULE_ID_VENC;
            stVencChnOutputPort.u32ChnId = VencChn;
            stVencChnOutputPort.u32PortId = 0;
            //This was set to (5, 10) and might be too big for kernel
            s32Ret = MI_SYS_SetChnOutputPortDepth(&stVencChnOutputPort, 2, 5);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s %d, MI_SYS_SetChnOutputPortDepth %d error, %X\n", __func__, __LINE__, VencChn, s32Ret);
            }

            s32Ret = MI_VENC_StartRecvPic(VencChn);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s %d, MI_VENC_StartRecvPic %d error, %X\n", __func__, __LINE__, VencChn, s32Ret);
            }
        }
    }
#endif

    /************************************************
    Step8:  Bind VPE->VDISP
    *************************************************/
    printf("Step 8: bind vpe&disp....................\n\n\n");
#ifdef SUPPORT_VIDEO_PREVIEW
    for (i = 0; i < s32DispChnNum; i++)
    {
        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = i;
        stBindInfo.stSrcChnPort.u32PortId = 3;
        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = i;
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
        printf("sysBind vpe_%d_%d_%d disp_%d_%d_%d srcRate_%d dstRate_%d\n", 0, i, 3
                    , 0, 0, i, 30, 30);
    }
#endif

#ifdef SUPPORT_VIDEO_ENCODE
// #if 0
    if (s32AdWorkMode == SAMPLE_VI_MODE_MIPI_1_1080P_VENC)
    {
        for (i = 0; i < s32DispChnNum; i++)
        {
            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;
            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;

            s32Ret = MI_VENC_GetChnDevid(VencChn, &u32DevId);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s %d, MI_VENC_GetChnDevid %d error, %X\n", __func__, __LINE__, VencChn, s32Ret);
            }
            stBindInfo.stDstChnPort.u32DevId = u32DevId;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate = 30;
            stBindInfo.u32DstFrmrate = 30;
            STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
        }
    }
#endif

    for (i = 0; i < s32CapChnNum; i++) //init vif channel
    {
        ST_NOP("=================ST_Vif_StartPort...i(%d)..vichn(%d)..===========\n", i, stVifChnCfg[i].u8ViChn);
        STCHECKRESULT(ST_Vif_StartPort(stVifChnCfg[i].u8ViChn, 0));
		printf("vif startPort chn_%d port_0\n", stVifChnCfg[i].u8ViChn);
    }
//#ifdef SUPPORT_VIDEO_ENCODE

#ifdef SUPPORT_UVC
    ST_UVCInit();
#endif

#ifdef SUPPORT_VIDEO_PREVIEW
    STCHECKRESULT(ST_Hdmi_Init());
    STCHECKRESULT(ST_Hdmi_Start(eHdmi, eHdmiTiming)); //Hdmi timing
#endif

    g_u32LastSubCaseIndex = pstCaseDesc[s32CaseIndex].u32SubCaseNum - 1;
    g_u32CurSubCaseIndex = pstCaseDesc[s32CaseIndex].u32SubCaseNum - 1;
    pstCaseDesc[s32CaseIndex].u32ShowWndNum = pstCaseDesc[s32CaseIndex].stDesc.u32WndNum;
    //pthread_create(&pt, NULL, st_GetOutputDataThread, NULL);
#ifdef SUPPORT_CDNN
    ST_CEVA_CDNN_Start();
#endif

    printf("ST_CEVA_CDNN_Start\n");
    ST_CEVA_CDNN_Start();

    return MI_SUCCESS;
}

void ST_DealCase(int argc, char **argv)
{
    MI_U32 u32Index = 0;
    MI_U32 u32SubIndex = 0;
    ST_CaseDesc_t *pstCaseDesc = g_stVifCaseDesc;

    if (argc != 3)
    {
        return;
    }

    u32Index = atoi(argv[1]);
    u32SubIndex = atoi(argv[2]);

    if (u32Index <= 0 || u32Index > ARRAY_SIZE(g_stVifCaseDesc))//case num
    {
        printf("case index range (%d~%d)\n", 1, ARRAY_SIZE(g_stVifCaseDesc));
        return;
    }
    g_u32CaseIndex = u32Index - 1;//real array index

    if (u32SubIndex <= 0 || u32SubIndex > pstCaseDesc[g_u32CaseIndex].u32SubCaseNum)
    {
        printf("sub case index range (%d~%d)\n", 1, pstCaseDesc[g_u32CaseIndex].u32SubCaseNum);
        return;
    }

    g_u32LastSubCaseIndex = pstCaseDesc[g_u32CaseIndex].u32SubCaseNum - 1;
    pstCaseDesc[g_u32CaseIndex].u32ShowWndNum = pstCaseDesc[g_u32CaseIndex].stDesc.u32WndNum;

    printf("case index %d, sub case %d-%d\n", g_u32CaseIndex, g_u32CurSubCaseIndex, g_u32LastSubCaseIndex);

    ST_VifToDisp(g_u32CaseIndex);

    g_u32CurSubCaseIndex = u32SubIndex - 1;//select subIndex

    if (pstCaseDesc[g_u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].eDispoutTiming > 0)
    {
        ST_ChangeDisplayTiming(); //change timing
    }
    else
    {
        if (0 == (strncmp(pstCaseDesc[g_u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "exit", 4)))
        {
            ST_SubExit();
            return;
        }
        else if (0 == (strncmp(pstCaseDesc[g_u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "zoom", 4)))
        {
        }
        else
        {
            if (pstCaseDesc[g_u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].u32WndNum > 0)
            {
                ST_SplitWindow(); //switch screen
            }
        }
    }

    ST_WaitSubCmd();
}

MI_S32 test_main(int argc, char **argv)
{
    MI_S32 a = 0, b = 1;
    MI_S32 sum = 0;

    while (1)
    {
        sum = a + b;
    }

    return 0;
}

MI_S32 main(int argc, char **argv)
{
    char szCmd[16];
    MI_U32 u32Index = 0;

    //return test_main(argc, argv);

    struct rlimit limit;
    limit.rlim_cur = RLIM_INFINITY;
    limit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &limit);
    signal(SIGCHLD, SIG_IGN);

    ST_DealCase(argc, argv);
    while (!g_bExit)
    {
        ST_VifUsage();//case usage
        fgets((szCmd), (sizeof(szCmd) - 1), stdin);

        u32Index = atoi(szCmd);

        if (u32Index <= 0 || u32Index > ARRAY_SIZE(g_stVifCaseDesc))
        {
            continue;
        }
        g_u32CaseIndex = u32Index - 1;
        if (0 == (strncmp(g_stVifCaseDesc[g_u32CaseIndex].stDesc.szDesc, "exit", 4)))
        {
            return MI_SUCCESS;
        }
        ST_VifToDisp(g_u32CaseIndex);
        ST_WaitSubCmd();
    }

    return 0;
}
