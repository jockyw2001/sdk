#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <poll.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include "mi_sys.h"
#include "mi_vpe.h"
#include "mi_venc.h"
#include "mi_uac.h"
#include "mi_aio.h"
#include "mi_aio_datatype.h"
#include "st_uvc.h"
#include "st_common.h"
#include "st_hdmi.h"
#include "st_disp.h"
#include "st_vpe.h"
#include "st_vif.h"
#include "st_fb.h"
#include "st_warp.h"
#include "st_sd.h"
#include "list.h"
#include "mi_rgn.h"
#include "tem.h"
#define SUPPORT_VIDEO_ENCODE
#define SUPPORT_UVC
#define UVC_SUPPORT_MMAP
//#define UVC_SUPPORT_LL
#define SUPPORT_WARP
#ifdef UVC_SUPPORT_USERPTR
#define UVC_MEMORY_MODE UVC_MEMORY_USERPTR
//yuv420 orz...
#else
#define UVC_MEMORY_MODE UVC_MEMORY_MMAP
#endif
//#define SUPPORT_WRITE_FILE
#define ENABLE_PUTES_TO_UVC 0
#define RD_OR_WR 1
#define ENABLE_DUMPCIF_PORT1 0
typedef enum
{
    E_UVC_TIMMING_4K2K_JPG,
    E_UVC_TIMMING_2560X1440P_JPG,
    E_UVC_TIMMING_1920X1080P_JPG,
    E_UVC_TIMMING_1280X720P_JPG,
    E_UVC_TIMMING_640X480P_JPG,
    E_UVC_TIMMING_320X240P_JPG,
    E_UVC_TIMMING_4K2K_H264,
    E_UVC_TIMMING_2560X1440P_H264,
    E_UVC_TIMMING_1920X1080P_H264,
    E_UVC_TIMMING_1280X720P_H264,
    E_UVC_TIMMING_640X480P_H264,
    E_UVC_TIMMING_320X240P_H264,
    E_UVC_TIMMING_4K2K_H265,
    E_UVC_TIMMING_2560X1440P_H265,
    E_UVC_TIMMING_1920X1080P_H265,
    E_UVC_TIMMING_1280X720P_H265,
    E_UVC_TIMMING_640X480P_H265,
    E_UVC_TIMMING_320X240P_H265,
    E_UVC_TIMMING_1920X1080_NV12,
    E_UVC_TIMMING_1280X720_NV12,
    E_UVC_TIMMING_640X480_NV12,
    E_UVC_TIMMING_320X240_NV12,
    E_UVC_TIMMING_1920X1080_YUV422_YUYV,
    E_UVC_TIMMING_1280X720_YUV422_YUYV,
    E_UVC_TIMMING_640X480_YUV422_YUYV,
    E_UVC_TIMMING_320X240_YUV422_YUYV,
}UVC_FormatTimming_e;
static MI_SYS_ChnPort_t gstChnPort;
static struct pollfd gpfd[1] =
{
    {0, POLLIN | POLLERR},
};
typedef struct ST_ModuleState_s
{
    MI_BOOL bEnableVpe;
    MI_BOOL bEnableWarp;
    MI_BOOL bEnableCevaVx;
    MI_BOOL bEnableSd;
    MI_BOOL bEnableVenc;
}ST_ModuleState_t;

extern MI_U32 Mif_Syscfg_GetTime0();
static MI_U32 curtime[5];
static MI_U32 u32BufSize = 0;;
static pthread_mutex_t _gTime = PTHREAD_MUTEX_INITIALIZER;
static MI_S32 _gs32Nv12ToMjpg = 0;
static MI_SYS_ChnPort_t _gstVencSrcPort;
static ST_ModuleState_t _gstModState;
static MI_S32 _gs32UseUac = 0;
static pthread_t _gUacSendFrameThread;
static bool _gUacSendFrameWorkExit = false;

#ifdef SUPPORT_WRITE_FILE
static void * ST_DoWaitData(ST_TEM_BUFFER stBuffer, pthread_mutex_t *pMutex)
{
    MI_SYS_ChnPort_t *pstChnPort = NULL;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE stBufHandle;
    MI_U32 u32WriteSize = 0;
    int rval = 0;
    int fd= 0;
    int write_len= 0;
    void *pData = NULL;
    pthread_mutex_lock(&_gTime);
    curtime[0] =  Mif_Syscfg_GetTime0();
    pstChnPort = &gstChnPort;
    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
    rval = poll(gpfd, 1, 200);
    if(rval < 0)
    {
        printf("poll error!\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    if(rval == 0)
    {
        printf("get fd time out!\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    if((gpfd[0].revents & POLLIN) != POLLIN)
    {
        printf("error !\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    curtime[1] =  Mif_Syscfg_GetTime0();
    fd = *((int *)stBuffer.pTemBuffer);
    if(MI_SUCCESS != MI_SYS_ChnOutputPortGetBuf(pstChnPort, &stBufInfo, &stBufHandle))
    {
        printf("GetBuf fail\n");
        pthread_mutex_unlock(&_gTime);
        return NULL;
    }
    curtime[2] =  Mif_Syscfg_GetTime0();
    switch(stBufInfo.eBufType)
    {
    case E_MI_SYS_BUFDATA_RAW:
        u32WriteSize = stBufInfo.stRawData.u32ContentSize;
        write_len = write(fd, stBufInfo.stRawData.pVirAddr, u32WriteSize);
        ASSERT(write_len == u32WriteSize);
        u32BufSize = u32WriteSize;
        break;
    case E_MI_SYS_BUFDATA_FRAME:
        if (stBufInfo.stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
        {
            u32WriteSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            write_len = write(fd, stBufInfo.stFrameData.pVirAddr[0], u32WriteSize);
            ASSERT(write_len == u32WriteSize);
            u32BufSize = u32WriteSize;
            u32WriteSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[1] / 2;
            write_len = write(fd, stBufInfo.stFrameData.pVirAddr[0], u32WriteSize);
            ASSERT(write_len == u32WriteSize);
            u32BufSize += u32WriteSize;
        }
        else if(stBufInfo.stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
        {
            u32WriteSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            write_len = write(fd, stBufInfo.stFrameData.pVirAddr[0], u32WriteSize);
            ASSERT(write_len == u32WriteSize);
            u32BufSize = u32WriteSize;
        } else
          ASSERT(0);
        break;
    default:
        ASSERT(0);
    }
    curtime[3] =  Mif_Syscfg_GetTime0();
    MI_SYS_ChnOutputPortPutBuf(stBufHandle);
    curtime[4] =  Mif_Syscfg_GetTime0();
    pthread_mutex_unlock(&_gTime);
    return NULL;
}
static void ST_TemDestroy(void)
{
    int fd;
    TemGetBuffer("Wait data", &fd);
    TemClose("Wait data");
    close(fd);
}
static void ST_TemCreate(void)
{
    ST_TEM_ATTR stAttr;
    pthread_attr_t m_SigMonThreadAttr;
    MI_U32 u32DevId = 0;
    MI_S32 s32Ret = 0;
    ST_TEM_USER_DATA stUserData;
    int fd = 0;
    fd = open("./venc_data", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd < 0)
    {
        perror("open");
        return;
    }
    PTH_RET_CHK(pthread_attr_init(&m_SigMonThreadAttr));
    memset(&stAttr, 0, sizeof(ST_TEM_ATTR));
    stAttr.fpThreadDoSignal = NULL;
    stAttr.fpThreadWaitTimeOut = ST_DoWaitData;
    stAttr.thread_attr = m_SigMonThreadAttr;
    stAttr.u32ThreadTimeoutMs = 20;
    stAttr.bSignalResetTimer = 0;
    stAttr.stTemBuf.pTemBuffer = (void *)&fd;
    stAttr.stTemBuf.u32TemBufferSize = sizeof(int);
    TemOpen("Wait data", stAttr);
    TemStartMonitor("Wait data");
}
#endif
#if (ENABLE_DUMPCIF_PORT1 == 1)
static struct pollfd gpfd1[1] =
{
    {0, POLLIN | POLLERR},
};
static void * DC_DoWaitData(ST_TEM_BUFFER stBuffer, pthread_mutex_t *pMutex)
{
    int fd= 0;
    int rval = 0;
    MI_SYS_ChnPort_t stVpeChnOutputPort0;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufInfo_t stBufInfo;
    pthread_mutex_lock(&_gTime);
    rval = poll(gpfd1, 1, 200);
    if(rval < 0)
    {
        printf("poll error!\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    if(rval == 0)
    {
        //printf("get fd time out!\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    if((gpfd1[0].revents & POLLIN) != POLLIN)
    {
        printf("error !\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&hHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
    memset(&stVpeChnOutputPort0,0,sizeof(MI_SYS_ChnPort_t));
    stVpeChnOutputPort0.eModId = E_MI_MODULE_ID_VPE;
    stVpeChnOutputPort0.u32DevId = 0;
    stVpeChnOutputPort0.u32ChnId = 0;
    stVpeChnOutputPort0.u32PortId = 1;
    fd = *((int *)stBuffer.pTemBuffer);
    if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stVpeChnOutputPort0 , &stBufInfo,&hHandle))
    {
    	// put frame
    	//printf("putframe\n");
    	test_vpe_PutOneFrame(fd,  stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.u32Stride[0], 352, 288);
    	test_vpe_PutOneFrame(fd,  stBufInfo.stFrameData.pVirAddr[1], stBufInfo.stFrameData.u32Stride[1], 352, 288/2);
    	MI_SYS_ChnOutputPortPutBuf(hHandle);
    }
    pthread_mutex_unlock(&_gTime);
    return NULL;
}
static void DC_TemDestroy(void)
{
    int fd;
    TemGetBuffer("DC Wait data", &fd);
    TemClose("DC Wait data");
    close(fd);
}
static void DC_TemCreate(void)
{
    ST_TEM_ATTR stAttr;
    pthread_attr_t m_SigMonThreadAttr;
    MI_U32 u32DevId = 0;
    MI_S32 s32Ret = 0;
    ST_TEM_USER_DATA stUserData;
    char szFn[64];
    sprintf(szFn, "/mnt/yuv/out_CIF.yuv");
    int fd = open(szFn, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd < 0)
    {
        perror("open");
        return;
    }
    PTH_RET_CHK(pthread_attr_init(&m_SigMonThreadAttr));
    memset(&stAttr, 0, sizeof(ST_TEM_ATTR));
    stAttr.fpThreadDoSignal = NULL;
    stAttr.fpThreadWaitTimeOut = DC_DoWaitData;
    stAttr.thread_attr = m_SigMonThreadAttr;
    stAttr.u32ThreadTimeoutMs = 33;
    stAttr.bSignalResetTimer = 0;
    stAttr.stTemBuf.pTemBuffer = (void *)&fd;
    stAttr.stTemBuf.u32TemBufferSize = sizeof(int);
    TemOpen("DC Wait data", stAttr);
    TemStartMonitor("DC Wait data");
}
#endif
static int wd_fd;
static int wd_fd_size;
static int rd_fd;
static int rd_fd_size;
static MI_U32 ST_DoGetData_mmap(MI_SYS_ChnPort_t * pstChnPort, void *pData,bool *is_tail)
{
#if (ENABLE_PUTES_TO_UVC == 1)
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE stBufHandle;
    MI_U32 u32RetSize = 0;
    MI_U16 i = 0;
    MI_U8 *u8CopyData = pData;
    int rval = 0;
    ASSERT(pstChnPort);
    ASSERT(pData);
#if (RD_OR_WR == 0)
    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
    rval = poll(gpfd, 1, 200);
    if(rval < 0)
    {
        printf("poll error!\n");
        return 0;
    }
    if(rval == 0)
    {
        printf("get fd time out!\n");
        return 0;
    }
    if((gpfd[0].revents & POLLIN) != POLLIN)
    {
        printf("error !\n");
        return 0;
    }
    if (MI_SUCCESS != MI_SYS_ChnOutputPortGetBuf(pstChnPort, &stBufInfo, &stBufHandle))
    {
        printf("Get buffer error!\n");
        return 0;
    }
    switch(stBufInfo.eBufType)
    {
        case E_MI_SYS_BUFDATA_RAW:
        {
            u32RetSize = stBufInfo.stRawData.u32ContentSize;
            memcpy(u8CopyData, stBufInfo.stRawData.pVirAddr, u32RetSize);
            write(wd_fd, stBufInfo.stRawData.pVirAddr, u32RetSize);
            write(wd_fd_size, &u32RetSize, sizeof(MI_U32));
        }
        break;
        case E_MI_SYS_BUFDATA_FRAME:
        {
            if (stBufInfo.stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
            {
                u32RetSize = stBufInfo.stFrameData.u16Height * (stBufInfo.stFrameData.u32Stride[0] +  stBufInfo.stFrameData.u32Stride[1] / 2);
                memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0]);
                write(wd_fd, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0]);

                u8CopyData += stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[1], stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[1] /2);
                write(wd_fd, stBufInfo.stFrameData.pVirAddr[1], stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[1] /2);
                write(wd_fd_size, &u32RetSize, sizeof(MI_U32));
            } else if(stBufInfo.stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
            {
                u32RetSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0], u32RetSize);
                write(wd_fd, stBufInfo.stRawData.pVirAddr, u32RetSize);
                write(wd_fd_size, &u32RetSize, sizeof(MI_U32));
            }else
                ASSERT(0);
        }
        break;
        default:
            ASSERT(0);
    }
    MI_SYS_ChnOutputPortPutBuf(stBufHandle);
#else
    if (0 == read(rd_fd_size, &u32RetSize, 4))
    {
        lseek(rd_fd_size, 0, SEEK_SET);
        lseek(rd_fd, 0, SEEK_SET);
        read(rd_fd_size, &u32RetSize, 4);
    }
    printf("Get size is %x\n", u32RetSize);
    read(rd_fd, pData, u32RetSize);
#endif
#else
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE stBufHandle;
    MI_U32 u32RetSize = 0;
    MI_U16 i = 0;
    MI_U8 *u8CopyData = pData;
    int rval = 0;
    ASSERT(pstChnPort);
    ASSERT(pData);
    pthread_mutex_lock(&_gTime);
    curtime[0] =  Mif_Syscfg_GetTime0();
    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
    rval = poll(gpfd, 1, 200);
    if(rval < 0)
    {
        printf("poll error!\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    if(rval == 0)
    {
        printf("get fd time out!\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    if((gpfd[0].revents & POLLIN) != POLLIN)
    {
        printf("error !\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    curtime[1] =  Mif_Syscfg_GetTime0();
    if (MI_SUCCESS != MI_SYS_ChnOutputPortGetBuf(pstChnPort, &stBufInfo, &stBufHandle))
    {
        printf("Get buffer error!\n");
        pthread_mutex_unlock(&_gTime);
        return 0;
    }
    curtime[2] =  Mif_Syscfg_GetTime0();
    switch(stBufInfo.eBufType)
    {
        case E_MI_SYS_BUFDATA_RAW:
            u32RetSize = stBufInfo.stRawData.u32ContentSize;
            memcpy(u8CopyData, stBufInfo.stRawData.pVirAddr, u32RetSize);
            break;
        case E_MI_SYS_BUFDATA_FRAME:
            if (stBufInfo.stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
            {
                u32RetSize = stBufInfo.stFrameData.u16Height * (stBufInfo.stFrameData.u32Stride[0] +  stBufInfo.stFrameData.u32Stride[1] / 2);
                memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0]);
                u8CopyData += stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[1], stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[1] /2);
            }
            else if(stBufInfo.stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
            {
                u32RetSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0], u32RetSize);
            }
            else
                ASSERT(0);
            break;
        default:
            ASSERT(0);
    }
    curtime[3] =  Mif_Syscfg_GetTime0();
    MI_SYS_ChnOutputPortPutBuf(stBufHandle);
    curtime[4] =  Mif_Syscfg_GetTime0();
#ifdef UVC_SUPPORT_LL
    if((unsigned int)(stBufInfo.u64SidebandMsg) ==2)
        *is_tail = 1;
    else
        *is_tail = 0;
#else
    *is_tail = 1;
#endif
    u32BufSize = u32RetSize;
    pthread_mutex_unlock(&_gTime);
#endif
    return u32RetSize;
}
static MI_S32 UVC_EnableVpe(ST_VPE_PortInfo_t *pVpePortInfo)
{
    ASSERT(pVpePortInfo);
    pVpePortInfo->DepVpeChannel = 0;
    pVpePortInfo->eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    STCHECKRESULT(ST_Vpe_CreatePort(MAIN_VENC_PORT, pVpePortInfo)); //default support port2 --->>> venc
#if (ENABLE_DUMPCIF_PORT1 == 1)
    ST_VPE_PortInfo_t stPortInfo;
    MI_SYS_ChnPort_t stChnPort;
    memset(&stPortInfo,0,sizeof(ST_VPE_PortInfo_t));
    memset(&stChnPort,0,sizeof(MI_SYS_ChnPort_t));
    stPortInfo.u16OutputWidth = 352;
    stPortInfo.u16OutputHeight = 288;
    stPortInfo.DepVpeChannel = 0;
    stPortInfo.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    STCHECKRESULT(ST_Vpe_CreatePort(1, &stPortInfo)); //default support port2 --->>> venc
    stChnPort.eModId = E_MI_MODULE_ID_VPE;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = stPortInfo.DepVpeChannel;
    stChnPort.u32PortId = 1;
    ExecFunc(MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 5), 0);
    if(MI_SUCCESS != MI_SYS_GetFd(&stChnPort, (MI_S32 *)&gpfd1[0].fd))
    {
        printf("MI_SYS_GetFd fail\n");
    }
#endif
    _gstModState.bEnableVpe = TRUE;

    return MI_SUCCESS;
}
static MI_S32 UVC_DisableVpe(void)
{
    STCHECKRESULT(ST_Vpe_StopPort(0, MAIN_VENC_PORT)); //default support port2 --->>> venc
    _gstModState.bEnableVpe = FALSE;

    return MI_SUCCESS;
}
static MI_S32 UVC_EnableWarp(ST_Warp_Timming_e eWarpTimming)
{
    MI_SYS_ChnPort_t stChnPort;
    STCHECKRESULT(ST_Warp_Init(eWarpTimming));
    /************************************************
    Step6:  init Warp
    *************************************************/
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_WARP;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;
    stChnPort.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 4);
    STCHECKRESULT(ST_Warp_CreateChannel(0)); //default support port2 --->>> venc
    printf("####FUNC %s\n", __FUNCTION__);
    _gstModState.bEnableWarp = TRUE;

    return MI_SUCCESS;
}
static MI_S32 UVC_DisableWarp(void)
{
    STCHECKRESULT(ST_Warp_DestroyChannel(0)); //default support port2 --->>> venc
    STCHECKRESULT(ST_Warp_Exit());
    printf("####FUNC %s\n", __FUNCTION__);
    _gstModState.bEnableWarp = FALSE;

    return MI_SUCCESS;
}
static MI_S32 UVC_EnableSd(ST_SD_PortInfo_t *pstPortInfo)
{
    MI_SYS_ChnPort_t stChnPort;
    ST_SD_ChannelInfo_t stSDChannelInfo;

    stSDChannelInfo.u32X =0;
    stSDChannelInfo.u32Y =0;
    stSDChannelInfo.u16SDCropW = 1920;
    stSDChannelInfo.u16SDCropH = 1080;
    stSDChannelInfo.u16SDMaxW = 1920;
    stSDChannelInfo.u16SDMaxH = 1080;
    STCHECKRESULT(ST_SD_CreateChannel(0, &stSDChannelInfo));

    ASSERT(pstPortInfo);
    pstPortInfo->DepSDChannel =0;
    pstPortInfo->eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    pstPortInfo->ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    STCHECKRESULT(ST_SD_CreatePort(pstPortInfo));

    _gstModState.bEnableSd = TRUE;

    return MI_SUCCESS;
}
static MI_S32 UVC_DisableSd(void)
{
    STCHECKRESULT(ST_SD_StopPort(0, 0));
    STCHECKRESULT(ST_SD_DestroyChannel(0));
    _gstModState.bEnableSd = FALSE;

    return MI_SUCCESS;
}
static MI_S32 UVC_EnableVenc(MI_VENC_ChnAttr_t *pstChnAttr)
{
    MI_U32 u32DevId = 0;
    MI_S32 s32Ret = 0;
    MI_SYS_ChnPort_t stChnPort;
    MI_VENC_ParamJpeg_t stJpegPara;
    ASSERT(pstChnAttr);
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    s32Ret = MI_VENC_CreateChn(0, pstChnAttr);
    if(pstChnAttr->stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        memset(&stJpegPara, 0, sizeof(MI_VENC_ParamJpeg_t));
        MI_VENC_GetJpegParam(0, &stJpegPara);
        stJpegPara.u32Qfactor = 30;
        MI_VENC_SetJpegParam(0, &stJpegPara);
    }
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s %d, MI_VENC_CreateChn %d error, %X\n", __func__, __LINE__, 0, s32Ret);
    }
    s32Ret = MI_VENC_GetChnDevid(0, &u32DevId);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s %d, MI_VENC_GetChnDevid %d error, %X\n", __func__, __LINE__, 0, s32Ret);
    }
    ST_DBG("u32DevId:%d\n", u32DevId);
    stChnPort.u32DevId = u32DevId;
    stChnPort.eModId = E_MI_MODULE_ID_VENC;
    stChnPort.u32ChnId = 0;
    stChnPort.u32PortId = 0;
    //This was set to (5, 10) and might be too big for kernel
    s32Ret = MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 5);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s %d, MI_SYS_SetChnOutputPortDepth %d error, %X\n", __func__, __LINE__, 0, s32Ret);
    }
    s32Ret = MI_VENC_StartRecvPic(0);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s %d, MI_VENC_StartRecvPic %d error, %X\n", __func__, __LINE__, 0, s32Ret);
    }

    printf("####FUNC %s\n", __FUNCTION__);
    _gstModState.bEnableVenc = TRUE;

    return MI_SUCCESS;
}
static MI_S32 UVC_DisableVenc(void)
{
    MI_S32 s32Ret = 0;
    s32Ret = MI_VENC_StopRecvPic(0);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s %d, MI_VENC_StartRecvPic %d error, %X\n", __func__, __LINE__, 0, s32Ret);
    }
    s32Ret = MI_VENC_DestroyChn(0);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s %d, MI_VENC_CreateChn %d error, %X\n", __func__, __LINE__, 0, s32Ret);
    }
    printf("####FUNC %s\n", __FUNCTION__);
    _gstModState.bEnableVenc = FALSE;

    return MI_SUCCESS;
}

static MI_S32 _UVC_IsSupportNv12ToMjpg(void)
{
    MI_S32 s32Choose = 0;
    printf("Which flow do you want ?\n 0: YUY2 to MJPG, 1: NV12 to MJPG\n");
    scanf("%d", &s32Choose);
    printf("You select support %s to MJPG\n", s32Choose?"YUY2":"NV12");

    return s32Choose;
}

static MI_S32 _UVC_IsSupportAudio(void)
{
    MI_S32 s32Choose = 0;
    printf("Support audio with UAC ?\n 0: NO, 1: YES\n");
    scanf("%d", &s32Choose);
    if ((s32Choose != 0) && (s32Choose != 1))
    {
        printf("You select choose %d, set to default 0\n", s32Choose);
        s32Choose = 0;
    }
    printf("You select %s audio\n", s32Choose ? "support" : "not support");

    return s32Choose;
}

static void UVC_EnableWindow(UVC_FormatTimming_e eFormatTimming)
{
    ST_SD_PortInfo_t stSdPortInfo;
    ST_VPE_PortInfo_t stVpePortInfo;
    ST_Sys_BindInfo_t stBindInfo;
    MI_S32 s32Ret = 0;
    MI_U32 u32FrameRate = 30;
    MI_BOOL bByframe = FALSE;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VPE_PortMode_t stVpeMode;
    MI_SD_OuputPortAttr_t stSdOutAttr;

    printf("####FUNC %s\n", __FUNCTION__);
    printf("Timming! %d\n", eFormatTimming);
    memset(&stVpeMode, 0, sizeof(MI_VPE_PortMode_t));
    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    memset(&stSdOutAttr, 0, sizeof(MI_SD_OuputPortAttr_t));
    memset(&_gstVencSrcPort, 0, sizeof(MI_SYS_ChnPort_t));

    _gstVencSrcPort.eModId = E_MI_MODULE_ID_WARP;
    _gstVencSrcPort.u32DevId = 0;
    _gstVencSrcPort.u32ChnId = 0;
    _gstVencSrcPort.u32PortId = 0;

#ifdef UVC_SUPPORT_LL
    bByframe = FALSE;
#else
    bByframe = TRUE;
#endif
    switch(eFormatTimming)
    {
        case E_UVC_TIMMING_4K2K_JPG:
        {
            stVpePortInfo.u16OutputWidth = 3840;
            stVpePortInfo.u16OutputHeight = 2160;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_3840_2160_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = 3840;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = 2160;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = 3840;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = 2160;
            stChnAttr.stVeAttr.stAttrJpeg.bByFrame = bByframe;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_2560X1440P_JPG:
        {
            stVpePortInfo.u16OutputWidth = 2560;
            stVpePortInfo.u16OutputHeight = 1440;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_2560_1440_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = 2560;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = 1440;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = 2560;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = 1440;
            stChnAttr.stVeAttr.stAttrJpeg.bByFrame = bByframe;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_1920X1080P_JPG:
        {
            stVpePortInfo.u16OutputWidth = 1920;
            stVpePortInfo.u16OutputHeight = 1080;
            if (!_gs32Nv12ToMjpg)
            {
                stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
                UVC_EnableVpe(&stVpePortInfo);
                UVC_EnableWarp(E_WARP_1920_1080_NV16);
                stSdPortInfo.u16OutputWidth = stVpePortInfo.u16OutputWidth;
                stSdPortInfo.u16OutputHeight = stVpePortInfo.u16OutputHeight;
                stSdPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
                UVC_EnableSd(&stSdPortInfo);
                _gstVencSrcPort.eModId = E_MI_MODULE_ID_SD;
            } else {
                stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                UVC_EnableVpe(&stVpePortInfo);
                UVC_EnableWarp(E_WARP_1920_1080_NV12);
            }
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = 1920;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = 1080;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = 1920;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = 1080;
            stChnAttr.stVeAttr.stAttrJpeg.bByFrame = bByframe;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_1280X720P_JPG:
        {
            stVpePortInfo.u16OutputWidth = 1280;
            stVpePortInfo.u16OutputHeight = 720;
            if (!_gs32Nv12ToMjpg)
            {
                stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
                UVC_EnableVpe(&stVpePortInfo);
                UVC_EnableWarp(E_WARP_1280_720_NV16);
                stSdPortInfo.u16OutputWidth = stVpePortInfo.u16OutputWidth;
                stSdPortInfo.u16OutputHeight = stVpePortInfo.u16OutputHeight;
                stSdPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
                UVC_EnableSd(&stSdPortInfo);
                _gstVencSrcPort.eModId = E_MI_MODULE_ID_SD;
            } else {
                stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                UVC_EnableVpe(&stVpePortInfo);
                UVC_EnableWarp(E_WARP_1280_720_NV12);
            }
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = 1280;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = 720;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = 1280;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = 720;
            stChnAttr.stVeAttr.stAttrJpeg.bByFrame = bByframe;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_640X480P_JPG:
        {
            stVpePortInfo.u16OutputWidth = 640;
            stVpePortInfo.u16OutputHeight = 480;
            if (!_gs32Nv12ToMjpg)
            {
                stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
                UVC_EnableVpe(&stVpePortInfo);
                UVC_EnableWarp(E_WARP_640_480_NV16);
                stSdPortInfo.u16OutputWidth = stVpePortInfo.u16OutputWidth;
                stSdPortInfo.u16OutputHeight = stVpePortInfo.u16OutputHeight;
                stSdPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
                UVC_EnableSd(&stSdPortInfo);
                _gstVencSrcPort.eModId = E_MI_MODULE_ID_SD;
            } else {
                stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                UVC_EnableVpe(&stVpePortInfo);
                UVC_EnableWarp(E_WARP_640_480_NV12);
            }
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = 640;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = 480;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = 640;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = 480;
            stChnAttr.stVeAttr.stAttrJpeg.bByFrame = bByframe;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_320X240P_JPG:
        {
            stVpePortInfo.u16OutputWidth = 320;
            stVpePortInfo.u16OutputHeight = 240;
            if (!_gs32Nv12ToMjpg)
            {
                stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
                UVC_EnableVpe(&stVpePortInfo);
                UVC_EnableWarp(E_WARP_320_240_NV16);
                stSdPortInfo.u16OutputWidth = stVpePortInfo.u16OutputWidth;
                stSdPortInfo.u16OutputHeight = stVpePortInfo.u16OutputHeight;
                stSdPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
                UVC_EnableSd(&stSdPortInfo);
                _gstVencSrcPort.eModId = E_MI_MODULE_ID_SD;
            } else {
                stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                UVC_EnableVpe(&stVpePortInfo);
                UVC_EnableWarp(E_WARP_320_240_NV12);
            }
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = 320;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = 240;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = 320;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = 240;
            stChnAttr.stVeAttr.stAttrJpeg.bByFrame = bByframe;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_4K2K_H264:
        {
            stVpePortInfo.u16OutputWidth = 3840;
            stVpePortInfo.u16OutputHeight = 2160;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_3840_2160_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = 3840;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = 2160;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = 3840;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = 2160;
            stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 36;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 36;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_2560X1440P_H264:
        {
            stVpePortInfo.u16OutputWidth = 2560;
            stVpePortInfo.u16OutputHeight = 1440;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_2560_1440_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = 2560;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = 1440;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = 2560;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = 1440;
            stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 36;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 36;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_1920X1080P_H264:
        {
            stVpePortInfo.u16OutputWidth = 1920;
            stVpePortInfo.u16OutputHeight = 1080;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_1920_1080_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = 1920;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = 1080;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = 1920;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = 1080;
            stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 36;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 36;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_1280X720P_H264:
        {
            stVpePortInfo.u16OutputWidth = 1280;
            stVpePortInfo.u16OutputHeight = 720;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_1280_720_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = 1280;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = 720;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = 1280;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = 720;
            stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 36;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 36;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_640X480P_H264:
        {
            stVpePortInfo.u16OutputWidth = 640;
            stVpePortInfo.u16OutputHeight = 480;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_640_480_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = 640;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = 480;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = 640;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = 480;
            stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 36;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 36;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_320X240P_H264:
        {
            stVpePortInfo.u16OutputWidth = 320;
            stVpePortInfo.u16OutputHeight = 240;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_320_240_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = 320;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = 240;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = 320;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = 240;
            stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 36;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 36;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_4K2K_H265:
        {
            stVpePortInfo.u16OutputWidth = 3840;
            stVpePortInfo.u16OutputHeight = 2160;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_3840_2160_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 3840;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 2160;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 3840;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 2160;
            stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum= 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate=1024*1024;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp=45;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp=25;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_2560X1440P_H265:
        {
            stVpePortInfo.u16OutputWidth = 2560;
            stVpePortInfo.u16OutputHeight = 1440;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_2560_1440_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 2560;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 1440;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 2560;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 1440;
            stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum= 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate=1024*1024;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp=45;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp=25;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_1920X1080P_H265:
        {
            stVpePortInfo.u16OutputWidth = 1920;
            stVpePortInfo.u16OutputHeight = 1080;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_1920_1080_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 1920;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 1080;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 1920;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 1080;
            stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum= 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate=1024*1024;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp=45;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp=25;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_1280X720P_H265:
        {
            stVpePortInfo.u16OutputWidth = 1280;
            stVpePortInfo.u16OutputHeight = 720;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_1280_720_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 1280;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 720;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 1280;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 720;
            stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum= 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate=1024*1024;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp=45;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp=25;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_640X480P_H265:
        {
            stVpePortInfo.u16OutputWidth = 640;
            stVpePortInfo.u16OutputHeight = 480;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_640_480_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 640;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 480;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 640;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 480;
            stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum= 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate=1024*1024;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp=45;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp=25;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_320X240P_H265:
        {
            stVpePortInfo.u16OutputWidth = 320;
            stVpePortInfo.u16OutputHeight = 240;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_320_240_NV12);
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2; // not support B frame
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 320;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 240;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 320;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 240;
            stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum= 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate=1024*1024;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp=45;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp=25;
            UVC_EnableVenc(&stChnAttr);
        }
        break;
        case E_UVC_TIMMING_1920X1080_NV12:
        {
            stVpePortInfo.u16OutputWidth = 1920;
            stVpePortInfo.u16OutputHeight = 1080;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_1920_1080_NV12);
        }
        break;
        case E_UVC_TIMMING_1280X720_NV12:
        {
            stVpePortInfo.u16OutputWidth = 1280;
            stVpePortInfo.u16OutputHeight = 720;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_1280_720_NV12);
        }
        break;
        case E_UVC_TIMMING_640X480_NV12:
        {
            stVpePortInfo.u16OutputWidth = 640;
            stVpePortInfo.u16OutputHeight = 480;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_640_480_NV12);
        }
        break;
        case E_UVC_TIMMING_320X240_NV12:
        {
            stVpePortInfo.u16OutputWidth = 320;
            stVpePortInfo.u16OutputHeight = 240;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_320_240_NV12);
        }
        break;
        case E_UVC_TIMMING_320X240_YUV422_YUYV:
        {
            stVpePortInfo.u16OutputWidth = 320;
            stVpePortInfo.u16OutputHeight = 240;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_320_240_NV16);
            stSdPortInfo.u16OutputWidth = stVpePortInfo.u16OutputWidth;
            stSdPortInfo.u16OutputHeight = stVpePortInfo.u16OutputHeight;
            stSdPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            UVC_EnableSd(&stSdPortInfo);
        }
        break;
        case E_UVC_TIMMING_640X480_YUV422_YUYV:
        {
            stVpePortInfo.u16OutputWidth = 640;
            stVpePortInfo.u16OutputHeight = 480;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_640_480_NV16);
            stSdPortInfo.u16OutputWidth = stVpePortInfo.u16OutputWidth;
            stSdPortInfo.u16OutputHeight = stVpePortInfo.u16OutputHeight;
            stSdPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            UVC_EnableSd(&stSdPortInfo);
        }
        break;
        case E_UVC_TIMMING_1280X720_YUV422_YUYV:
        {
            stVpePortInfo.u16OutputWidth = 1280;
            stVpePortInfo.u16OutputHeight = 720;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_1280_720_NV16);
            stSdPortInfo.u16OutputWidth = stVpePortInfo.u16OutputWidth;
            stSdPortInfo.u16OutputHeight = stVpePortInfo.u16OutputHeight;
            stSdPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            UVC_EnableSd(&stSdPortInfo);
        }
        break;
        case E_UVC_TIMMING_1920X1080_YUV422_YUYV:
        {
            stVpePortInfo.u16OutputWidth = 1920;
            stVpePortInfo.u16OutputHeight = 1080;
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
            UVC_EnableVpe(&stVpePortInfo);
            UVC_EnableWarp(E_WARP_1920_1080_NV16);
            stSdPortInfo.u16OutputWidth = stVpePortInfo.u16OutputWidth;
            stSdPortInfo.u16OutputHeight = stVpePortInfo.u16OutputHeight;
            stSdPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            UVC_EnableSd(&stSdPortInfo);
        }
        break;
        default:
            printf("Format not support!\n");
            return;
    }
}
static void UVC_DisableWindow(void)
{
    printf("####FUNC %s\n", __FUNCTION__);
    if (_gstModState.bEnableVenc)
    {
        UVC_DisableVenc();
    }
    if (_gstModState.bEnableSd)
    {
        UVC_DisableSd();
    }
    if(_gstModState.bEnableWarp)
    {
        UVC_DisableWarp();
    }
    if(_gstModState.bEnableVpe)
    {
        UVC_DisableVpe();
    }
}
static MI_S32 UVC_EnableBindRelation(void)
{
    ST_Sys_BindInfo_t stBindInfo;
    MI_U32 u32DevId = 0;
    MI_S32 s32Ret = 0;

    ASSERT(_gstModState.bEnableVpe);
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    if(_gstModState.bEnableWarp)
    {
        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = 0;
        stBindInfo.stSrcChnPort.u32PortId = MAIN_VENC_PORT;
        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_WARP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = 0;
#ifdef UVC_SUPPORT_LL
        stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = TRUE;
        stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 19;
#endif
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }
    if (_gstModState.bEnableCevaVx)
    {
    }
    else if (_gstModState.bEnableSd)
    {
        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_WARP;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = 0;
        stBindInfo.stSrcChnPort.u32PortId = 0;
        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_SD;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = 0;
#ifdef UVC_SUPPORT_LL
        stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = TRUE;
        stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 3;
#endif
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }
    if (_gstModState.bEnableVenc)
    {
        s32Ret = MI_VENC_GetChnDevid(0, &u32DevId);
        if (MI_SUCCESS != s32Ret)
        {
            printf("%s %d, MI_VENC_GetChnDevid %d error, %X\n", __func__, __LINE__, 0, s32Ret);
        }
        stBindInfo.stSrcChnPort = _gstVencSrcPort;
        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
        stBindInfo.stDstChnPort.u32DevId = u32DevId;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = 0;
#ifdef UVC_SUPPORT_LL
        stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = TRUE;
        stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 3;
#endif
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }

    return MI_SUCCESS;
}
static MI_S32 UVC_DisableBindRelation(void)
{
    ST_Sys_BindInfo_t stBindInfo;
    MI_U32 u32DevId = 0;
    MI_S32 s32Ret = 0;

    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));

    if (_gstModState.bEnableVenc)
    {
        s32Ret = MI_VENC_GetChnDevid(0, &u32DevId);
        if (MI_SUCCESS != s32Ret)
        {
            printf("%s %d, MI_VENC_GetChnDevid %d error, %X\n", __func__, __LINE__, 0, s32Ret);
        }
        stBindInfo.stSrcChnPort= _gstVencSrcPort;
        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
        stBindInfo.stDstChnPort.u32DevId = u32DevId;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = 0;
#ifdef UVC_SUPPORT_LL
        stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = FALSE;
        stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 0;
#endif
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }
    if (_gstModState.bEnableCevaVx)
    {
    }
    else if (_gstModState.bEnableSd)
    {
        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_WARP;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = 0;
        stBindInfo.stSrcChnPort.u32PortId = 0;
        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_SD;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = 0;
#ifdef UVC_SUPPORT_LL
        stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = FALSE;
        stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 0;
#endif
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }
    if(_gstModState.bEnableWarp)
    {
        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = 0;
        stBindInfo.stSrcChnPort.u32PortId = MAIN_VENC_PORT;
        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_WARP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = 0;
#ifdef UVC_SUPPORT_LL
        stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = FALSE;
        stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 0;
#endif
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }

    return MI_SUCCESS;
}
static MI_S32 UVC_Init(void *uvc)
{
    return MI_SUCCESS;
}
static MI_S32 UVC_Deinit(void *uvc)
{
    return MI_SUCCESS;
}
static MI_S32 UVC_UP_FinishBUffer(int handle)
{
    return MI_SUCCESS;
}
static MI_S32 UVC_UP_FillBuffer(void *uvc,ST_UVC_BufInfo_t *bufInfo)
{
        //todo
    if(bufInfo->length ==0)
        return -1;
    return MI_SUCCESS;
}
static MI_S32 UVC_MM_FillBuffer(void *uvc,ST_UVC_BufInfo_t *bufInfo)
{
#ifndef SUPPORT_WRITE_FILE
    bufInfo->length = ST_DoGetData_mmap(&gstChnPort, bufInfo->b.buf,&(bufInfo->is_tail));
#endif
    if(bufInfo->length ==0)
        return -1;
    return MI_SUCCESS;
}
static MI_S32 UVC_StartCapture(void *uvc,Stream_Params_t format)
{
    memset(&gstChnPort, 0, sizeof(MI_SYS_ChnPort_t));
#if (ENABLE_PUTES_TO_UVC == 1)
#if (RD_OR_WR == 0)
    wd_fd = open("./venc_data", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (wd_fd < 0)
    {
        perror("open");
        return;
    }
    wd_fd_size = open("./venc_data_size", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (wd_fd_size < 0)
    {
        perror("open");
        return;
    }
#else
    rd_fd = open("./venc_data", O_RDONLY);
    if (wd_fd < 0)
    {
        perror("open");
        return;
    }
    rd_fd_size = open("./venc_data_size",O_RDONLY);
    if (wd_fd_size < 0)
    {
        perror("open");
        return;
    }
#endif
#endif
    printf("Fmt %s width %d height %d\n",
           uvc_get_format(format.fcc), format.width, format.height);
    switch(format.fcc)
    {
        case V4L2_PIX_FMT_NV12:
        {
            if (format.width == 1920 && format.height == 1080)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1920X1080_NV12);
            }
            else if (format.width == 1280 && format.height == 720)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1280X720_NV12);
            }
            else if (format.width == 640 && format.height == 480)
            {
                UVC_EnableWindow(E_UVC_TIMMING_640X480_NV12);
            }
            else if (format.width == 320 && format.height == 240)
            {
                UVC_EnableWindow(E_UVC_TIMMING_320X240_NV12);
            }
            else
            {
                printf("NV12 not support format! width %d height %d\n", format.width, format.height);
                return -1;
            }
            gstChnPort.eModId = E_MI_MODULE_ID_WARP;
            gstChnPort.u32DevId = 0;
            gstChnPort.u32ChnId = 0;
            gstChnPort.u32PortId = 0;
        }
        break;
        case V4L2_PIX_FMT_H264:
        {
            if (format.width == 3840 && format.height == 2160)
            {
                UVC_EnableWindow(E_UVC_TIMMING_4K2K_H264);
            }
            else if (format.width == 2560 && format.height == 1440)
            {
                UVC_EnableWindow(E_UVC_TIMMING_2560X1440P_H264);
            }
            else if (format.width == 1920 && format.height == 1080)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1920X1080P_H264);
            }
            else if (format.width == 1280 && format.height == 720)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1280X720P_H264);
            }
            else if (format.width == 640 && format.height == 480)
            {
                UVC_EnableWindow(E_UVC_TIMMING_640X480P_H264);
            }
            else if (format.width == 320 && format.height == 240)
            {
                UVC_EnableWindow(E_UVC_TIMMING_320X240P_H264);
            }
            else
            {
                printf("H264 not support format! width %d height %d\n", format.width, format.height);
                return -1;
            }
            gstChnPort.eModId = E_MI_MODULE_ID_VENC;
            MI_VENC_GetChnDevid(0, &gstChnPort.u32DevId);
            gstChnPort.u32ChnId = 0;
            gstChnPort.u32PortId = 0;
        }
        break;
        case V4L2_PIX_FMT_MJPEG:
        {
            if (format.width == 3840 && format.height == 2160)
            {
                UVC_EnableWindow(E_UVC_TIMMING_4K2K_JPG);
            }
            else if (format.width == 2560 && format.height == 1440)
            {
                UVC_EnableWindow(E_UVC_TIMMING_2560X1440P_JPG);
            }
            else if (format.width == 1920 && format.height == 1080)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1920X1080P_JPG);
            }
            else if (format.width == 1280 && format.height == 720)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1280X720P_JPG);
            }
            else if (format.width == 640 && format.height == 480)
            {
                UVC_EnableWindow(E_UVC_TIMMING_640X480P_JPG);
            }
            else if (format.width == 320 && format.height == 240)
            {
                UVC_EnableWindow(E_UVC_TIMMING_320X240P_JPG);
            }
            else
            {
                printf("Mjpeg not support format! width %d height %d\n", format.width, format.height);
                return -1;
            }
            gstChnPort.eModId = E_MI_MODULE_ID_VENC;
            MI_VENC_GetChnDevid(0, &gstChnPort.u32DevId);
            gstChnPort.u32ChnId = 0;
            gstChnPort.u32PortId = 0;
        }
        break;
        case V4L2_PIX_FMT_H265:
        {
            if (format.width == 3840 && format.height == 2160)
            {
                UVC_EnableWindow(E_UVC_TIMMING_4K2K_H265);
            }
            else if (format.width == 2560 && format.height == 1440)
            {
                UVC_EnableWindow(E_UVC_TIMMING_2560X1440P_H265);
            }
            else if (format.width == 1920 && format.height == 1080)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1920X1080P_H265);
            }
            else if (format.width == 1280 && format.height == 720)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1280X720P_H265);
            }
            else if (format.width == 640 && format.height == 480)
            {
                UVC_EnableWindow(E_UVC_TIMMING_640X480P_H265);
            }
            else if (format.width == 320 && format.height == 240)
            {
                UVC_EnableWindow(E_UVC_TIMMING_320X240P_H265);
            }
            else
            {
                printf("H265 not support format! width %d height %d\n", format.width, format.height);
                return -1;
            }
            gstChnPort.eModId = E_MI_MODULE_ID_VENC;
            MI_VENC_GetChnDevid(0, &gstChnPort.u32DevId);
            gstChnPort.u32ChnId = 0;
            gstChnPort.u32PortId = 0;
        }
        break;
        case V4L2_PIX_FMT_YUYV:
        {
            if (format.width == 320 && format.height == 240)
            {
                UVC_EnableWindow(E_UVC_TIMMING_320X240_YUV422_YUYV);
            }
            else if (format.width == 640 && format.height == 480)
            {
                UVC_EnableWindow(E_UVC_TIMMING_640X480_YUV422_YUYV);
            }
            else if (format.width == 1280 && format.height == 720)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1280X720_YUV422_YUYV);
            }
            else if (format.width == 1920 && format.height == 1080)
            {
                UVC_EnableWindow(E_UVC_TIMMING_1920X1080_YUV422_YUYV);
            }
            else
            {
                printf("Yuv 422 not support format! width %d height %d\n", format.width, format.height);
                return -1;
            }
            gstChnPort.eModId = E_MI_MODULE_ID_SD;
            gstChnPort.u32DevId = 0;
            gstChnPort.u32ChnId = 0;
            gstChnPort.u32PortId = 0;
        }
        break;
        default:
            printf("not support format!\n");
            return -1;
    }
    UVC_EnableBindRelation();
    if(MI_SUCCESS != MI_SYS_GetFd(&gstChnPort, (MI_S32 *)&gpfd[0].fd))
    {
        printf("MI_SYS_GetFd fail\n");
    }
#ifdef SUPPORT_WRITE_FILE
    ST_TemCreate();
#endif
    return MI_SUCCESS;
}
static MI_S32 UVC_StopCapture(void *uvc)
{
#ifdef SUPPORT_WRITE_FILE
    ST_TemDestroy();
#endif
#if(ENABLE_DUMPCIF_PORT1 == 1)
    DC_TemDestroy();
#endif
    if(MI_SUCCESS != MI_SYS_CloseFd(gpfd[0].fd))
    {
        printf("MI_SYS_CloseFd fail\n");
    }
    UVC_DisableBindRelation();
    UVC_DisableWindow();
#if (ENABLE_PUTES_TO_UVC == 1)
#if (RD_OR_WR == 0)
    close(wd_fd);
    close(wd_fd_size);
#else
    close(rd_fd);
    close(rd_fd_size);
#endif
#endif
    return MI_SUCCESS;
}
static MI_S32 St_UvcInit()
{
    ST_UVC_Setting_t pstSet={4,UVC_MEMORY_MODE,USB_ISOC_MODE};
    ST_UVC_OPS_t fops = { UVC_Init ,
                          UVC_Deinit,
#ifdef UVC_SUPPORT_MMAP
                          UVC_MM_FillBuffer,
#else
                          UVC_UP_FillBuffer,
                          UVC_UP_FinishBUffer,
#endif
                          UVC_StartCapture,
                          UVC_StopCapture};
    ST_UVC_ChnAttr_t pstAttr ={pstSet,fops};
    STCHECKRESULT(ST_UVC_Init("/dev/video0"));
    STCHECKRESULT(ST_UVC_CreateDev(&pstAttr));
    STCHECKRESULT(ST_UVC_StartDev());
    return MI_SUCCESS;
}
static MI_S32 St_UvcDeinit()
{
    STCHECKRESULT(ST_UVC_StopDev());
    STCHECKRESULT(ST_UVC_DestroyDev(0));
    STCHECKRESULT(ST_UVC_Uninit());
    return MI_SUCCESS;
}

static void *St_UacSendFrameWork()
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AiDevId = 1,UacDevId = UAC_CAPTURE_DEV;
    MI_AI_CHN AiChn = 0;
    MI_UAC_CHN UacChn   = 0;
    MI_AUDIO_Frame_t stAiFrm;
    MI_UAC_Frame_t dtUacFrm;

     while(false==_gUacSendFrameWorkExit){
        MI_AI_GetFrame(AiDevId, AiChn, &stAiFrm, NULL, 16);//256 / 16000 = 16ms
        if (0 == stAiFrm.u32Len){
            continue;
        }
        dtUacFrm.u32Len = stAiFrm.u32Len;
        dtUacFrm.pu8Addr = stAiFrm.apVirAddr[0];
        dtUacFrm.u64PTS = stAiFrm.u64TimeStamp;
        MI_UAC_SendFrame(UacChn,&dtUacFrm,16);
        MI_AI_ReleaseFrame(AiDevId,  AiChn, &stAiFrm, NULL);
     }
}

static MI_S32 St_UacCaptureInit(void)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AiDevId = 1,UacDevId = UAC_CAPTURE_DEV;
    MI_AUDIO_Attr_t stAiAttr;
    MI_AI_CHN AiChn = 0;
    MI_U32 UacChn   = 0;
    MI_SYS_ChnPort_t stAiPort;
    MI_U32 u32FrameDepth = 12;
    MI_U32 u32BuffQueue = 13;

/*UAC Init*/
    ExecFunc(MI_UAC_OpenDevice(UacDevId),MI_SUCCESS);
/*AI PARAM*/
    memset(&stAiAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stAiAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAiAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
    stAiAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAiAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stAiAttr.u32ChnCnt = 2;
    stAiAttr.u32FrmNum = 16;
    stAiAttr.u32PtNumPerFrm = 256;

    ExecFunc(MI_AI_SetPubAttr(AiDevId, &stAiAttr), MI_SUCCESS);
    ExecFunc(MI_AI_Enable(AiDevId), MI_SUCCESS);
    ExecFunc(MI_AI_EnableChn(AiDevId, AiChn), MI_SUCCESS);
/*AI Depth*/
    memset(&stAiPort, 0, sizeof(MI_SYS_ChnPort_t));
    stAiPort.eModId = E_MI_MODULE_ID_AI;
    stAiPort.u32ChnId = AiChn;
    stAiPort.u32DevId = AiDevId;
    stAiPort.u32PortId = 0;
    ExecFunc(MI_SYS_SetChnOutputPortDepth(&stAiPort , u32FrameDepth, u32BuffQueue), MI_SUCCESS);

/* Get A Buf And Send To Uac Capture Device */
    _gUacSendFrameWorkExit = false;
    ExecFunc(MI_UAC_StartDev(UacDevId),MI_SUCCESS);
    ExecFunc(pthread_create(&_gUacSendFrameThread,NULL,St_UacSendFrameWork,NULL),MI_SUCCESS);

    return MI_SUCCESS;
}

static MI_S32 St_UacCaptureDeinit(void)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AiDevId = 1,UacDevId = UAC_CAPTURE_DEV;
    MI_AI_CHN AiChn = 0;

    _gUacSendFrameWorkExit = true;
    ExecFunc(pthread_join(_gUacSendFrameThread,NULL),MI_SUCCESS);
    ExecFunc(MI_UAC_StopDev(UacDevId),MI_SUCCESS);
    ExecFunc(MI_UAC_CloseDevice(UacDevId),MI_SUCCESS);
    ExecFunc(MI_AI_DisableChn(AiDevId, AiChn), MI_SUCCESS);
    ExecFunc(MI_AI_Disable(AiDevId), MI_SUCCESS);
    return MI_SUCCESS;
}

static MI_S32 St_UacPlaybackInit(void)
{
    MI_AUDIO_Attr_t stAoAttr;
    ST_Sys_BindInfo_t stBindInfo;

    /************************************************
    Step1:  init AO
    *************************************************/
    memset(&stAoAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stAoAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAoAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
    stAoAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAoAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stAoAttr.u32ChnCnt = 1;
    stAoAttr.u32FrmNum = 6;
    stAoAttr.u32PtNumPerFrm = 800;
    ExecFunc(MI_AO_SetPubAttr(0, &stAoAttr), MI_SUCCESS);
    ExecFunc(MI_AO_GetPubAttr(0, &stAoAttr), MI_SUCCESS);
    ExecFunc(MI_AO_Enable(0), MI_SUCCESS);
    ExecFunc(MI_AO_EnableChn(0, 0), MI_SUCCESS);

    /************************************************
    Step2:  init UAC
    *************************************************/
    ExecFunc(MI_UAC_OpenDevice(UAC_PLAYBACK_DEV),MI_SUCCESS);

    /************************************************
    Step4:  bind UAC playback to A0
    *************************************************/
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_UAC;
    stBindInfo.stSrcChnPort.u32DevId = UAC_PLAYBACK_DEV;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    ExecFunc(MI_SYS_SetChnOutputPortDepth(&stBindInfo.stSrcChnPort, 3, 6), MI_SUCCESS);

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_AO;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 0;
    stBindInfo.u32DstFrmrate = 0;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

    /************************************************
    Step4:  start UAC playback DEV
    *************************************************/
    ExecFunc(MI_UAC_StartDev(UAC_PLAYBACK_DEV),MI_SUCCESS);

    return MI_SUCCESS;
}

static MI_S32 St_UacPlaybackDeinit(void)
{
    ST_Sys_BindInfo_t stBindInfo;

    ExecFunc(MI_UAC_StopDev(UAC_PLAYBACK_DEV),MI_SUCCESS);

    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_UAC;
    stBindInfo.stSrcChnPort.u32DevId = UAC_PLAYBACK_DEV;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_AO;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 0;
    stBindInfo.u32DstFrmrate = 0;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));

    ExecFunc(MI_AO_DisableChn(0, 0), MI_SUCCESS);
    ExecFunc(MI_AO_Disable(0), MI_SUCCESS);

    ExecFunc(MI_UAC_CloseDevice(UAC_PLAYBACK_DEV),MI_SUCCESS);

    return MI_SUCCESS;
}

static MI_S32 St_UacInit(MI_S32 s32CaptureFlag, MI_S32 s32PlaybackFlag)
{
    MI_S32 s32Ret;

    /************************************************
    Step1:  init UAC
    *************************************************/
    if (s32CaptureFlag == 1)
    {
        s32Ret = St_UacCaptureInit();
    }

    if (s32PlaybackFlag == 1)
    {
        s32Ret = St_UacPlaybackInit();
    }

    return s32Ret;
}

static MI_S32 St_UacDeinit(MI_S32 s32CaptureFlag, MI_S32 s32PlaybackFlag)
{
    if (s32CaptureFlag == 1)
    {
        St_UacCaptureDeinit();
    }

    if (s32PlaybackFlag == 1)
    {
        St_UacPlaybackDeinit();
    }

    return MI_SUCCESS;
}

static MI_S32 St_DisplayInit(void)
{
    ST_VPE_PortInfo_t stPortInfo;
    ST_Sys_BindInfo_t stBindInfo;
    ST_DispChnInfo_t stDispChnInfo;
    memset(&stPortInfo, 0x0, sizeof(ST_VPE_PortInfo_t));
    memset(&stDispChnInfo, 0x0, sizeof(ST_DispChnInfo_t));
    stPortInfo.DepVpeChannel = 0;
    stPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stPortInfo.u16OutputWidth = 1920;
    stPortInfo.u16OutputHeight = 1080;
    STCHECKRESULT(ST_Vpe_CreatePort(DISP_PORT, &stPortInfo)); //default support port0 --->>> vdisp
    STCHECKRESULT(ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, E_MI_DISP_OUTPUT_1080P60)); //Dispout timing
    stDispChnInfo.InputPortNum = 1;
    stDispChnInfo.stInputPortAttr[0].u32Port = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16X = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Y = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 1920;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 1080;
    STCHECKRESULT(ST_Disp_ChnInit(0, &stDispChnInfo));
    /************************************************
    Step8:  Bind VPE->VDISP
    *************************************************/
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = DISP_PORT;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
#ifdef UVC_SUPPORT_LL
    stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = TRUE;
    stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 5;
#endif
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    STCHECKRESULT(ST_Hdmi_Init());
    STCHECKRESULT(ST_Hdmi_Start(E_MI_HDMI_ID_0, E_MI_HDMI_TIMING_1080_60P)); //Hdmi timing
    return MI_SUCCESS;
}
static MI_S32 St_DisplayDeinit(void)
{
    ST_VPE_PortInfo_t stPortInfo;
    ST_Sys_BindInfo_t stBindInfo;
    ST_DispChnInfo_t stDispChnInfo;
    STCHECKRESULT(ST_Hdmi_DeInit(E_MI_HDMI_ID_0));
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = DISP_PORT;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
#ifdef UVC_SUPPORT_LL
    stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = FALSE;
    stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 0;
#endif
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    STCHECKRESULT(ST_Disp_DeInit(ST_DISP_DEV0, ST_DISP_LAYER0, 1));
    STCHECKRESULT(ST_Vpe_StopPort(0, DISP_PORT));
    return MI_SUCCESS;
}

static MI_S32 St_BaseModuleInit(MI_S32 s32RunMode)
{
    ST_VIF_PortInfo_t stVifPortInfoInfo;
    MI_SYS_ChnPort_t stChnPort;
    ST_VPE_ChannelInfo_t stVpeChannelInfo;
    ST_Sys_BindInfo_t stBindInfo;
    VIF_AD_WORK_MODE_E eVifWorkMode;
    MI_VPE_RunningMode_e eVpeRunningMode;
    MI_SYS_PixelFormat_e ePixelFormat;
    memset(&stVifPortInfoInfo, 0x0, sizeof(ST_VIF_PortInfo_t));
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stVpeChannelInfo, 0x0, sizeof(ST_VPE_ChannelInfo_t));
    STCHECKRESULT(ST_Sys_Init());
    switch (s32RunMode)
    {
        case 0:
            {
                eVifWorkMode = SAMPLE_VI_MODE_MIPI_1_1080P_REALTIME;
                eVpeRunningMode = E_MI_VPE_RUNNING_MODE_REALTIME_MODE;
                ePixelFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
                stVpeChannelInfo.eHDRType = E_MI_VPE_HDR_TYPE_OFF;
            }
            break;
        case 1:
            {
                eVifWorkMode = SAMPLE_VI_MODE_MIPI_1_1080P_FRAME;
                eVpeRunningMode = E_MI_VPE_RUNNING_MODE_FRAMEBUF_CAM_MODE;
                ePixelFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
                stVpeChannelInfo.eHDRType = E_MI_VPE_HDR_TYPE_OFF;
            }
            break;
        case 2:
            {
                eVifWorkMode = SAMPLE_VI_MODE_MIPI_1_1080P_REALTIME_HDR;
                eVpeRunningMode = E_MI_VPE_RUNNING_MODE_REALTIME_MODE;
                ePixelFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
                stVpeChannelInfo.eHDRType = E_MI_VPE_HDR_TYPE_DOL;
            }
            break;
        default:
            {
                printf("vif workmode %d error \n", s32RunMode);
                return 1;
            }
    }
    STCHECKRESULT(ST_Vif_CreateDev(0, eVifWorkMode));
    stVifPortInfoInfo.u32RectX = 0;
    stVifPortInfoInfo.u32RectY = 0;
    stVifPortInfoInfo.u32RectWidth = 3840;
    stVifPortInfoInfo.u32RectHeight = 2160;
    stVifPortInfoInfo.u32DestWidth = 3840;
    stVifPortInfoInfo.u32DestHeight = 2160;
    stVifPortInfoInfo.ePixFormat = ePixelFormat;
#ifdef UVC_SUPPORT_LL
	stVifPortInfoInfo.u32FrameModeLineCount = 10;
#endif
    STCHECKRESULT(ST_Vif_CreatePort(0, 0, &stVifPortInfoInfo));
    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;
    stChnPort.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 3);
    STCHECKRESULT(ST_Vif_StartPort(0, 0));
    /************************************************
    Step4:  init VPE
    *************************************************/
    stVpeChannelInfo.u16VpeMaxW = 3840;
    stVpeChannelInfo.u16VpeMaxH = 2160;
    stVpeChannelInfo.u32X = 0;
    stVpeChannelInfo.u32Y = 0;
    stVpeChannelInfo.u16VpeCropW = 3840;
    stVpeChannelInfo.u16VpeCropH = 2160;
    stVpeChannelInfo.eRunningMode = eVpeRunningMode;
    stVpeChannelInfo.eFormat = ePixelFormat;
    STCHECKRESULT(ST_Vpe_CreateChannel(0, &stVpeChannelInfo));
    STCHECKRESULT(ST_Vpe_StartChannel(0));
    /************************************************
    Step7:  Bind VIF->VPE
    *************************************************/
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = 0; //VIF dev == 0
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0; //Main stream
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
#ifdef UVC_SUPPORT_LL
    stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = TRUE;
    stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 8;
#endif
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    return MI_SUCCESS;
}
static MI_S32 St_BaseModuleDeinit(void)
{
    ST_Sys_BindInfo_t stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = 0; //VIF dev == 0
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0; //Main stream
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
#ifdef UVC_SUPPORT_LL
    stBindInfo.stDstChnPort.bEnableLowLatencyReceiveMode = FALSE;
    stBindInfo.stDstChnPort.u32LowLatencyDelayMs = 0;
#endif
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    STCHECKRESULT(ST_Vpe_StopChannel(0));
    STCHECKRESULT(ST_Vpe_DestroyChannel(0));
    STCHECKRESULT(ST_Vif_StopPort(0, 0));
    STCHECKRESULT(ST_Vif_DisableDev(0));
    STCHECKRESULT(ST_Sys_Exit());
    return MI_SUCCESS;
}
static void do_fix_mantis(int mantis)
{
    printf("To fix mantis 1574080!\n");
    return;
}
#if (ENABLE_DUMPCIF_PORT1 == 1)
MI_S32 test_vpe_PutOneFrame(int dstFd,  char *pDataFrame, int line_offset, int line_size, int lineNumber)
{
    int size = 0;
    int i = 0;
    char *pData = NULL;
    int yuvSize = line_size;
    // seek to file offset
    //lseek(dstFd, offset, SEEK_SET);
    for (i = 0; i < lineNumber; i++)
    {
        pData = pDataFrame + line_offset*i;
        yuvSize = line_size;
        do {
            if (yuvSize < 256)
            {
                size = yuvSize;
            }
            else
            {
                size = 256;
            }
            size = write(dstFd, pData, size);
            if (size == 0)
            {
                break;
            }
            else if (size < 0)
            {
                break;
            }
            pData += size;
            yuvSize -= size;
        } while (yuvSize > 0);
    }
    return 0;
}
#endif
MI_S32 main(int argc, char **argv)
{
    MI_S32 s32Mode = 0;
    char cmd = 0;
    int i = 0;
    if (signal(SIGINT, do_fix_mantis) != 0)
    {
        perror("signal");
        return -1;
    }
    if (signal(SIGTSTP, do_fix_mantis) != 0)
    {
        perror("signal");
        return -1;
    }
    printf("Which mode do you want ?\n 0 for real time mode 1 for frame mode 2 for HDR real time mode\n");
    scanf("%d", &s32Mode);
    printf("You select %s mode\n", s32Mode?"frame":"real time");

    _gs32Nv12ToMjpg = _UVC_IsSupportNv12ToMjpg();
	_gs32UseUac = _UVC_IsSupportAudio();
    memset(&_gstModState, 0, sizeof(ST_ModuleState_t));
    St_BaseModuleInit(s32Mode);
    St_DisplayInit();
    St_UvcInit();
    St_UacInit(_gs32UseUac, _gs32UseUac);
#if(ENABLE_DUMPCIF_PORT1 == 1)
    DC_TemCreate();
#endif
    while(1)
    {
        printf("Type \"q\" to exit\n");
        cmd = getchar();
        if (cmd == 'q')
            break;
        pthread_mutex_lock(&_gTime);
        for (i = 0; i < 5; i++)
        {
            printf("time [%d] = %d\n", i, curtime[i]);
        }
        printf("Buf size is %d\n", u32BufSize);
        pthread_mutex_unlock(&_gTime);
    }
    St_UacDeinit(_gs32UseUac, _gs32UseUac);
    St_UvcDeinit();
    printf("St_UvcDeinit deinit\n");
    St_DisplayDeinit();
    printf("St_DisplayDeinit deinit\n");
    St_BaseModuleDeinit();
    return 0;
}
