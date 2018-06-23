#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_disp_datatype.h"
#include "mi_vpe.h"
#include "mi_disp.h"
#include "mi_hdmi.h"

#include "../mi_disp_test.h"
#include "../../vpe/mi_vpe_test.h"

#define MAX_TEST_CHANNEL (16)
#define VPE_OUTPORT 0
#if (VPE_OUTPORT == 3)
MI_SYS_PixelFormat_e ePixel = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
#else
MI_SYS_PixelFormat_e ePixel = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
#endif

static test_vpe_Config stTest002[MAX_TEST_CHANNEL] = {
    {
        .inputFile  = TEST_VPE_CHNN_FILE420(002, 0, 1920x1080),
        .stSrcWin   = {0, 0, 1920, 1080},
        .stCropWin   = {0, 0, 1920, 1080},
        .stOutPort  = {
            [VPE_OUTPORT] = {
                .bEnable    = TRUE,
            },
        },
    },
    {
        .inputFile = TEST_VPE_CHNN_FILE420(002, 1, 1280x720),
            .stSrcWin   = {0, 0, 1280, 720},
            .stCropWin   = {0, 0, 1280, 720},
        .stOutPort  = {
            [VPE_OUTPORT] = {
                .bEnable    = TRUE,
            },
        },
    },
    {
        .inputFile = TEST_VPE_CHNN_FILE420(002, 2, 640x360),
        .stSrcWin   = {0, 0, 640, 360},
        .stCropWin  = {0, 0, 640, 360},
        .stOutPort  = {
            [VPE_OUTPORT] = {
                .bEnable    = TRUE,
            },
        },
    },
    {
        .inputFile = TEST_VPE_CHNN_FILE420(002, 3, 640x360),
        .stSrcWin   = {0, 0, 640, 360},
        .stCropWin  = {0, 0, 640, 360},
        .stOutPort  = {
            [VPE_OUTPORT] = {
                .bEnable    = TRUE,
            },
        },
    }
};

typedef struct test_disp_PortWin_s{
    MI_U16 u16X;
    MI_U16 u16Y;
    MI_U16 u16Width;
    MI_U16 u16Height;
}test_disp_PortWin_t;

test_disp_PortWin_t stDispPort1add5[]=
{
    {0, 0, 1280, 720},
    {1280, 0, 640, 360},
    {1280, 360, 640, 360},
    {0, 720, 640, 360},
    {640, 720, 640, 360},
    {1280, 720, 640, 360},

};

test_disp_PortWin_t stDispPort1add2x2[]=
{
    {0, 0, 960, 1080},
    {960, 0, 480, 540},
    {1440, 0, 480, 540},
    {960, 540, 480, 540},
    {1440, 540, 480, 540},
};

test_disp_PortWin_t stDispPort2x2[]=
{
    {0, 0, 960, 540},
    {960, 0, 960, 540},
    {0, 540, 960, 540},
    {960, 540, 960, 540},
};

test_disp_PortWin_t stDispPort1[]=
{
    {0, 0, 1920, 1080},
};

test_disp_PortWin_t stDispPort1add2[]=
{
    {0, 0, 640, 1080},
    {640, 0, 640, 1080},
    {1280, 0, 640, 1080},
};

test_disp_PortWin_t stDispPort4x4[]=
{
    {0, 0, 480, 270},
    {480, 0, 480, 270},
    {960, 0, 480, 270},
    {1440, 0, 480, 270},
    {0, 270, 480, 270},
    {480, 270, 480, 270},
    {960, 270, 480, 270},
    {1440, 270, 480, 270},
    {0, 540, 480, 270},
    {480, 540, 480, 270},
    {960, 540, 480, 270},
    {1440, 540, 480, 270},
    {0, 810, 480, 270},
    {480, 810, 480, 270},
    {960, 810, 480, 270},
    {1440, 810, 480, 270}
};


int bind(MI_U8 SrcChnl,MI_U8 SrcPort,MI_U8 u8DevId, MI_U8 DstChn,MI_U8 DstPort, MI_BOOL Flag)
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;

    stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = SrcChnl;
    stSrcChnPort.u32PortId = SrcPort;

    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId = u8DevId;
    stDstChnPort.u32ChnId = DstChn;
    stDstChnPort.u32PortId = DstPort;

    u32SrcFrmrate = 30;
    u32DstFrmrate = 30;

    if(Flag == TRUE)
    {
        ExecFunc(MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate), MI_SUCCESS);
    }
    else if(Flag == FALSE)
    {
        ExecFunc(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort), MI_SUCCESS);
    }
    return TRUE;
}

static MI_BOOL test_disp_SendStrem(MI_U16 u16ChannelNum, MI_U32 count)
{

    MI_U16 i=0, j=0;
    struct timeval stTime;
    MI_SYS_ChnPort_t stVpeChnInputPort0;
    int src_fd =0;
    int frame_size = 0;
    int width  = 0;
    int height = 0;
    int y_size = 0;
    int uv_size = 0;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    do{
        MI_SYS_BufConf_t stBufConf;
        MI_S32 s32Ret;
        if (u16ChannelNum <= 0)
        {
            break;
        }

        for (i = 0; i < u16ChannelNum; i++)
        {
            src_fd = stTest002[i].src_fd;
            if (stTest002[i].count >= count)
            {
                u16ChannelNum--;
                continue;
            }
            for (j = 0; j < 4; j++)
            {
                if (stTest002[i].stOutPort[j].bEnable == TRUE)
                {
                    if (1)//(stTest002[i].product == 0)
                    {
                        stTest002[i].product = 1;
                        memset(&stVpeChnInputPort0, 0, sizeof(stVpeChnInputPort0));
                        stVpeChnInputPort0.eModId = E_MI_MODULE_ID_VPE;
                        stVpeChnInputPort0.u32DevId = 0;
                        stVpeChnInputPort0.u32ChnId = i;
                        stVpeChnInputPort0.u32PortId = 0;

                        memset(&stBufConf ,  0 , sizeof(stBufConf));
                        MI_VPE_TEST_DBG("%s()@line%d: Start get chnn %d input buffer.\n", __func__, __LINE__, i);
                        gettimeofday(&stTime, NULL);
                        stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
                        stBufConf.u64TargetPts = stTime.tv_sec*1000000 + stTime.tv_usec;
                        stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                        stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
                        stBufConf.stFrameCfg.u16Width = stTest002[i].stSrcWin.u16Width;
                        stBufConf.stFrameCfg.u16Height = stTest002[i].stSrcWin.u16Height;
                        if(MI_SUCCESS  == MI_SYS_ChnInputPortGetBuf(&stVpeChnInputPort0,&stBufConf,&stBufInfo,&hHandle, 0))
                        {
                            // Start user put int buffer
                            width   = stBufInfo.stFrameData.u16Width;
                            height  = stBufInfo.stFrameData.u16Height;
                            y_size  = width*height;
                            //width   = stBufInfo.stFrameData.u32Stride[1];
                            uv_size  = width*height/2;

                            //test_vpe_ShowFrameInfo("Input : ", &stBufInfo.stFrameData);
                            if (1 == test_vpe_GetOneFrameYUV420(src_fd, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.pVirAddr[1], y_size, uv_size))
                            {
                                stTest002[i].src_offset += y_size + uv_size;

                                //printf("%s()@line%d: Start put input buffer.\n", __func__, __LINE__);
                                s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle,&stBufInfo, FALSE);
                                stTest002[i].count++;
                            }
                            else
                            {
                                s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle,&stBufInfo, TRUE);
                                stTest002[i].src_offset = 0;
                                stTest002[i].src_count = 0;
                                test_vpe_FdRewind(src_fd);
                                continue;
                            }
                        }
                    }
                }
            }
        }
    }while(1);

    return TRUE;
}

static MI_BOOL test_Disp_Init(MI_U16 DispDev, MI_U16 DispLayer, MI_U16 u16PortNum, MI_U16 layerwidth, MI_U16 layerHeight, test_disp_PortWin_t *pstDispPort)
{
    MI_U8 i=0;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_U16 u16LayerDispWidth =0, u16LayerDispHeight =0;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    memset(&stPubAttr, 0, sizeof(stPubAttr));

    if(0 == DispDev)
    {
        u16LayerDispWidth = 1920;
        u16LayerDispHeight = 1080;
        stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_PAL;
        stPubAttr.eIntfType = E_MI_DISP_INTF_CVBS;
        ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
        stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    }
    else if(1 == DispDev)
    {
        u16LayerDispWidth = 1920;
        u16LayerDispHeight = 1080;
        stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
    }

    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

    stLayerAttr.stVidLayerSize.u16Width = layerwidth;
    stLayerAttr.stVidLayerSize.u16Height= layerHeight;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = u16LayerDispWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = u16LayerDispHeight;
    stLayerAttr.ePixFormat = ePixel;
    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    for(i = 0; i<u16PortNum; i++)
    {
        MI_DISP_InputPortAttr_t stInputPortAttr;
        MI_DISP_INPUTPORT LayerInputPort = i;
        memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));

        ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);

        stInputPortAttr.stDispWin.u16X = pstDispPort[i].u16X;
        stInputPortAttr.stDispWin.u16Y = pstDispPort[i].u16Y;
        stInputPortAttr.stDispWin.u16Width = pstDispPort[i].u16Width;
        stInputPortAttr.stDispWin.u16Height =pstDispPort[i].u16Height;

        ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
        printf("Dev: %d layer: %d Port[%d]: {%d, %d, %d, %d}.\n", DispDev, DispLayer, LayerInputPort,
            stInputPortAttr.stDispWin.u16X, stInputPortAttr.stDispWin.u16Y, stInputPortAttr.stDispWin.u16Width, stInputPortAttr.stDispWin.u16Height);
        ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_SUCCESS);
        MI_DISP_SetInputPortSyncMode (DispLayer, LayerInputPort, E_MI_DISP_SYNC_MODE_FREE_RUN);
    }

    return TRUE;
}

static MI_BOOL test_Vpe_Disp_DeInit(MI_U8 DispDev, MI_U8 DispLayer, MI_U16 u16PortNum)
{
    MI_U16 u16PortId =0;
    int src_fd;

    for (u16PortId = 0; u16PortId < u16PortNum; u16PortId++)
    {
        src_fd = stTest002[u16PortId].src_fd;
        bind(u16PortId,VPE_OUTPORT,DispDev,0,u16PortId,FALSE);
        test_vpe_CloseFd(src_fd);
        test_vpe_DestroyChannel(u16PortId, VPE_OUTPORT);
    }

    MI_DISP_SetVideoLayerAttrBegin(DispLayer);
    for(u16PortId =0; u16PortId <u16PortNum; u16PortId++)
    {
        ExecFunc(MI_DISP_DisableInputPort(DispLayer, u16PortId), MI_SUCCESS);
    }
    ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    MI_DISP_SetVideoLayerAttrEnd(DispLayer);
    ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);


    return TRUE;
}

static MI_BOOL test_disp_WindowSwitch(MI_U8 DispDev, MI_U8 DispLayer, MI_U16 ChannelNum, char *pbaseDir, test_disp_PortWin_t *pstDispPort)
{
    MI_VPE_PortMode_t stVpeMode;
    MI_U8 VpeChannel =0;
    MI_U8 VpePort =0;
    char src_file[256];
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;
    MI_SYS_ChnPort_t stVpeChnOutputPort0;
    memset(&stVpeChnOutputPort0, 0, sizeof(stVpeChnOutputPort0));
    memset(&stVpeMode, 0, sizeof(stVpeMode));

    for (VpeChannel = 0; VpeChannel < ChannelNum; VpeChannel++)
    {
         stTest002[VpeChannel].inputFile = stTest002[VpeChannel%4].inputFile;
         stTest002[VpeChannel].stSrcWin = stTest002[VpeChannel%4].stSrcWin;
         stTest002[VpeChannel].stOutPort[VPE_OUTPORT] = stTest002[VpeChannel%4].stOutPort[VPE_OUTPORT];

        sprintf(src_file, "%s/%s", pbaseDir, stTest002[VpeChannel].inputFile);
        ExecFunc(test_vpe_OpenSourceFile(src_file, &stTest002[VpeChannel].src_fd), TRUE);

        stTest002[VpeChannel].count = 0;
        stTest002[VpeChannel].src_offset = 0;
        stTest002[VpeChannel].src_count  = 0;
        stTest002[VpeChannel].product = 0;
    }

    //Create VPE
    for (VpeChannel = 0; VpeChannel < ChannelNum; VpeChannel++)
    {
        for(VpePort = 0; VpePort < 4; VpePort++)
        {
            if (stTest002[VpeChannel].stOutPort[VpePort].bEnable == TRUE)
            {
                stTest002[VpeChannel].stOutPort[VpePort].stPortWin.u16Width = pstDispPort[VpeChannel].u16Width;
                stTest002[VpeChannel].stOutPort[VpePort].stPortWin.u16Height = pstDispPort[VpeChannel].u16Height;
                test_vpe_CreatChannel(VpeChannel, VpePort, &stTest002[VpeChannel].stCropWin, &stTest002[VpeChannel].stOutPort[VpePort].stPortWin,E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420,ePixel);

                // set vpe port buffer depth
                stVpeChnOutputPort0.eModId = E_MI_MODULE_ID_VPE;
                stVpeChnOutputPort0.u32DevId = 0;
                stVpeChnOutputPort0.u32ChnId = VpeChannel;
                stVpeChnOutputPort0.u32PortId = VpePort;
                MI_SYS_SetChnOutputPortDepth(&stVpeChnOutputPort0, 0, 3);

                stTest002[VpeChannel].count = 0;
                stTest002[VpeChannel].src_offset = 0;
                stTest002[VpeChannel].src_count  = 0;
                stTest002[VpeChannel].product = 0;
                break;
            }
        }
    }

    //Create Disp
    test_Disp_Init(DispDev, DispLayer, ChannelNum, 1920, 1080, pstDispPort);

    for(VpeChannel = 0; VpeChannel<ChannelNum; VpeChannel++)
        bind(VpeChannel,VPE_OUTPORT,DispDev,0,VpeChannel,TRUE);

    return TRUE;
}

//int test_vpe_TestCase002_main(int argc, const char *argv[])
int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U32 i = 0;
    int j = 0;
    MI_U32 u32ChannelNum = 0;
    MI_U32 u32ChannelNum_tmp = 0;
    MI_U8 DispDev=0;
    MI_U8 DispLayer=0;
    MI_SYS_ChnPort_t stVpeChnInputPort0;
    MI_SYS_ChnPort_t stVpeChnOutputPort0;
    MI_U32 count = 0;
    struct timeval stTime;
    char src_file[256];

    char pbaseDir[40];
    int src_fd;
    MI_SYS_FrameData_t framedata;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    int frame_size = 0;
    int width  = 0;
    int height = 0;
    int y_size = 0;
    int uv_size = 0;
    struct timeval stTv;
    memset(&framedata, 0, sizeof(framedata));
    memset(&stTime, 0, sizeof(stTime));

    if (argc < 6)
    {
        printf("%s <test_dir> <count> <ChannelNum> <DispDev> <DispLayer>.\n", argv[0]);
        printf("%s.\n", VPE_TEST_002_DESC);
        for (i = 0; i < 4; i++)
        {
            printf("Channel[%d][%d][%d][%d]:\n", i, 4+i, 8+i, 12+i);
            printf("InputFile: %s.\n", stTest002[i].inputFile);
        }
        return 0;
    }

    snprintf(pbaseDir, (sizeof(pbaseDir)-1), "%s", argv[1]);
    count = atoi(argv[2]);
    u32ChannelNum = atoi(argv[3]);
    DispDev = atoi(argv[4]);
    DispLayer = atoi(argv[5]);

    printf("%s %s %d %d %d %d.\n", argv[0], pbaseDir, count,u32ChannelNum,DispDev, DispLayer);

    for (i = 0; i < u32ChannelNum; i++)
    {
         stTest002[i].inputFile = stTest002[i%4].inputFile;
         stTest002[i].stSrcWin = stTest002[i%4].stSrcWin;
         stTest002[i].stOutPort[VPE_OUTPORT] = stTest002[i%4].stOutPort[VPE_OUTPORT];
        sprintf(src_file, "%s/%s", pbaseDir, stTest002[i].inputFile);
        ExecFunc(test_vpe_OpenSourceFile(src_file, &stTest002[i].src_fd), TRUE);

        stTest002[i].count = 0;
        stTest002[i].src_offset = 0;
        stTest002[i].src_count  = 0;
        stTest002[i].product = 0;
    }

    // init MI_SYS
    ExecFunc(test_vpe_SysEnvInit(), TRUE);

    // Create VPE channel
    MI_U8 u8Canvs = 0;
    MI_U32 layerwidth = 0;
    MI_U32 layerHeight = 0;
    MI_U32 u16VpeOutportWidth = 0,u16VpeOutportHeight = 0;
    if(u32ChannelNum <= 1)
        u8Canvs = 1;
    else if(u32ChannelNum <= 4)
        u8Canvs = 2;
    else if(u32ChannelNum <= 9)
        u8Canvs = 3;
    else if(u32ChannelNum <= 16)
        u8Canvs = 4;


    MI_DISP_PubAttr_t stPubAttr0,stPubAttr1;
    memset(&stPubAttr1, 0, sizeof(stPubAttr1));
    memset(&stPubAttr0, 0, sizeof(stPubAttr0));

    if(0 == DispDev)
    {
        stPubAttr0.eIntfSync = E_MI_DISP_OUTPUT_PAL;
        stPubAttr0.eIntfType = E_MI_DISP_INTF_CVBS;
        ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr0), MI_DISP_SUCCESS);
        layerwidth = 1920;
        layerHeight = 1080;
        stPubAttr1.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
        stPubAttr1.eIntfType = E_MI_DISP_INTF_HDMI;
    }
    else if(1 == DispDev)
    {
        printf("Dev %d\n", DispDev);
        layerwidth = 1920;
        layerHeight = 1080;
        stPubAttr1.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
        stPubAttr1.eIntfType = E_MI_DISP_INTF_VGA;
    }

    u16VpeOutportWidth = (layerwidth / u8Canvs)/2*2;
    u16VpeOutportHeight = (layerHeight / u8Canvs)/2*2;

    for (i = 0; i < u32ChannelNum; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (stTest002[i].stOutPort[j].bEnable == TRUE)
            {
                stTest002[i].stOutPort[j].stPortWin.u16Width = u16VpeOutportWidth;
                stTest002[i].stOutPort[j].stPortWin.u16Height = u16VpeOutportHeight;
                test_vpe_CreatChannel(i, j, &stTest002[i].stCropWin, &stTest002[i].stOutPort[j].stPortWin,E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420,ePixel);

                // set vpe port buffer depth
                stVpeChnOutputPort0.eModId = E_MI_MODULE_ID_VPE;
                stVpeChnOutputPort0.u32DevId = 0;
                stVpeChnOutputPort0.u32ChnId = i;
                stVpeChnOutputPort0.u32PortId = j;
                MI_SYS_SetChnOutputPortDepth(&stVpeChnOutputPort0, 0, 3);
                break;
            }
        }
    }

       ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr1), MI_DISP_SUCCESS);
       ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);
       MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;

        if(DispDev == 0)
        {
            mi_disp_hdmiInit();
            MI_HDMI_Attr_t stHdmiAttr;
            MI_HDMI_GetAttr(eHdmi,&stHdmiAttr);
            stHdmiAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
            MI_HDMI_SetAttr(eHdmi,&stHdmiAttr);
        }

       MI_U32 u32Toleration = 100000;
       MI_DISP_CompressAttr_t stCompressAttr;
       MI_DISP_VideoLayerAttr_t stLayerAttr, stLayerAttrBackup;
       memset(&stLayerAttr, 0, sizeof(stLayerAttr));

       stLayerAttr.stVidLayerSize.u16Width = layerwidth;
       stLayerAttr.stVidLayerSize.u16Height= layerHeight;
       stLayerAttr.stVidLayerDispWin.u16X = 0;
       stLayerAttr.stVidLayerDispWin.u16Y = 0;
       stLayerAttr.stVidLayerDispWin.u16Width = layerwidth;
       stLayerAttr.stVidLayerDispWin.u16Height = layerHeight;
       stLayerAttr.ePixFormat = ePixel;
       ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
       ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
       ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
       ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    {
        MI_U8 i=0;

        for(i = 0; i<u32ChannelNum;i++)
        {
            MI_DISP_InputPortAttr_t stInputPortAttr;
             MI_DISP_INPUTPORT LayerInputPort = i;
            memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));

            ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);

            stInputPortAttr.stDispWin.u16X = (i%u8Canvs)*u16VpeOutportWidth;
            stInputPortAttr.stDispWin.u16Y = (i/u8Canvs)*u16VpeOutportHeight;
            stInputPortAttr.stDispWin.u16Width = u16VpeOutportWidth;
            stInputPortAttr.stDispWin.u16Height =u16VpeOutportHeight;

            ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
            ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
            MI_VPE_TEST_INFO("Dev: %d layer: %d Port[%d]: {%d, %d, %d, %d}.\n", DispDev, DispLayer, LayerInputPort,
                stInputPortAttr.stDispWin.u16X, stInputPortAttr.stDispWin.u16Y, stInputPortAttr.stDispWin.u16Width, stInputPortAttr.stDispWin.u16Height);
            ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_SUCCESS);
            MI_DISP_SetInputPortSyncMode (DispLayer, LayerInputPort, E_MI_DISP_SYNC_MODE_FREE_RUN);
        }

        for(i = 0; i<u32ChannelNum;i++)
            bind(i,VPE_OUTPORT,DispDev,0,i,TRUE);
    }

    test_disp_SendStrem(u32ChannelNum, count);

    /*******switch to 1 add 5 channel*********/
    MI_HDMI_SetAvMute(eHdmi, TRUE);
    test_Vpe_Disp_DeInit(DispDev, DispLayer, u32ChannelNum);
    u32ChannelNum = sizeof(stDispPort1add5)/sizeof(stDispPort1add5[0]);
    test_disp_WindowSwitch(DispDev, DispLayer,u32ChannelNum, pbaseDir, stDispPort1add5);
    MI_HDMI_SetAvMute(eHdmi,FALSE);
    test_disp_SendStrem(u32ChannelNum, count);


    /*******switch to 1 add 2x2 channel*********/
    MI_HDMI_SetAvMute(eHdmi, TRUE);
    test_Vpe_Disp_DeInit(DispDev, DispLayer, u32ChannelNum);
    u32ChannelNum = sizeof(stDispPort1add2x2)/sizeof(stDispPort1add2x2[0]);
    test_disp_WindowSwitch(DispDev, DispLayer,u32ChannelNum, pbaseDir, stDispPort1add2x2);
    MI_HDMI_SetAvMute(eHdmi,FALSE);
    test_disp_SendStrem(u32ChannelNum, count);

    /*******switch to 2x2 channel*********/
    MI_HDMI_SetAvMute(eHdmi, TRUE);
    test_Vpe_Disp_DeInit(DispDev, DispLayer, u32ChannelNum);
    u32ChannelNum = sizeof(stDispPort2x2)/sizeof(stDispPort2x2[0]);
    test_disp_WindowSwitch(DispDev, DispLayer,u32ChannelNum, pbaseDir, stDispPort2x2);
    MI_HDMI_SetAvMute(eHdmi,FALSE);
    test_disp_SendStrem(u32ChannelNum, count);

    /*******switch to 1 channel*********/
    MI_HDMI_SetAvMute(eHdmi, TRUE);
    test_Vpe_Disp_DeInit(DispDev, DispLayer, u32ChannelNum);
    u32ChannelNum = sizeof(stDispPort1)/sizeof(stDispPort1[0]);
    test_disp_WindowSwitch(DispDev, DispLayer,u32ChannelNum, pbaseDir, stDispPort1);
    MI_HDMI_SetAvMute(eHdmi,FALSE);
    test_disp_SendStrem(u32ChannelNum, count);


    /*******switch to 1+2 channel*********/
    MI_HDMI_SetAvMute(eHdmi, TRUE);
    test_Vpe_Disp_DeInit(DispDev, DispLayer, u32ChannelNum);
    u32ChannelNum = sizeof(stDispPort1add2)/sizeof(stDispPort1add2[0]);
    test_disp_WindowSwitch(DispDev, DispLayer,u32ChannelNum, pbaseDir, stDispPort1add2);
    MI_HDMI_SetAvMute(eHdmi,FALSE);
    test_disp_SendStrem(u32ChannelNum, count);


    /*******switch to 4x4 channel*********/
    MI_HDMI_SetAvMute(eHdmi, TRUE);
    test_Vpe_Disp_DeInit(DispDev, DispLayer, u32ChannelNum);
    u32ChannelNum = sizeof(stDispPort4x4)/sizeof(stDispPort4x4[0]);
    test_disp_WindowSwitch(DispDev, DispLayer,u32ChannelNum, pbaseDir, stDispPort4x4);
    MI_HDMI_SetAvMute(eHdmi,FALSE);
    test_disp_SendStrem(u32ChannelNum, count);

    test_Vpe_Disp_DeInit(DispDev, DispLayer, u32ChannelNum);
    ExecFunc(test_vpe_SysEnvDeinit(), TRUE);

    MI_VPE_TEST_DBG("%s()@line %d pass exit\n", __func__, __LINE__);
    return 0;
}
