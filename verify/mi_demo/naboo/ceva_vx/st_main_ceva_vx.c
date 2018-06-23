#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <poll.h>
#include <time.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include<sys/time.h>
#include <linux/i2c-dev.h>
#include <getopt.h>

#include "mi_sys.h"
#include "mi_venc.h"


#include "mi_uac.h"
#include "mi_aio.h"
#include "mi_aio_datatype.h"
#include "st_uvc.h"
#include "st_common.h"
#include "st_hdmi.h"
#include "st_disp.h"
#include "st_vpe.h"
#include "st_sd.h"
#include "st_vif.h"
#include "st_fb.h"
#include "st_ceva_vx.h"
#include "st_warp.h"
#include "i2c.h"
#include "tem.h"

#define SUPPORT_VIDEO_ENCODE
#define SUPPORT_UVC
#define UVC_SUPPORT_MMAP
//#define UVC_SUPPORT_LL
#define SUPPORT_WARP

#define ENABLEFLAG              0x01
#define ENABLE_VENC             (ENABLEFLAG)
#define ENABLE_WARP             (ENABLEFLAG << 1)
#define ENABLE_VX               (ENABLEFLAG << 2)
#define ENABLE_SD               (ENABLEFLAG << 3)        // use Scl2 as MI_SD for warp 422SP format
#define ENABLE_VPE_YUV422SP     (ENABLEFLAG << 4)        // VPE port0 output YUV422
#define ENABLE_DISP             (ENABLEFLAG << 5)

#ifdef UVC_SUPPORT_USERPTR
#define UVC_MEMORY_MODE UVC_MEMORY_USERPTR
//yuv420 orz...
#else
#define UVC_MEMORY_MODE UVC_MEMORY_MMAP
#endif
//#define SUPPORT_CDNN
//#define SUPPORT_WRITE_FILE

#define ENABLE_PUTES_TO_UVC 0
#define RD_OR_WR 1
#define ENABLE_DUMPCIF_PORT1 0


#ifndef ASSERT
#define ASSERT(_x_)                                                                         \
    do  {                                                                                   \
        if ( ! ( _x_ ) )                                                                    \
        {                                                                                   \
            printf("ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);     \
            abort();                                                                        \
        }                                                                                   \
    } while (0)
#endif


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

    // YUY2
    E_UVC_TIMMING_1920X1080_YUV422_YUYV,
    E_UVC_TIMMING_1280X720_YUV422_YUYV,
    E_UVC_TIMMING_640X480_YUV422_YUYV,
    E_UVC_TIMMING_320X240_YUV422_YUYV,

    /*
    // YUV422SP
    E_UVC_TIMMING_1920X1080P_YUV420SP,
    E_UVC_TIMMING_1280X720P_YUV420SP,
    E_UVC_TIMMING_640X480P_YUV420SP,
    E_UVC_TIMMING_320X240P_YUV420SP,

    */
}UVC_FormatTimming_e;

static MI_SYS_ChnPort_t gstChnPort;
static MI_BOOL gbUvcSinkFromVx = FALSE;
static struct pollfd gpfd[1] =
{
    {0, POLLIN | POLLERR},
};

typedef enum
{
    E_INJECT_MOD_UVC,
    E_INJECT_MOD_VENC,
    E_INJECT_MOD_SD
}UVC_InjectMod_e;

extern MI_U32 Mif_Syscfg_GetTime0();
static MI_U32 curtime[5];
static MI_U32 u32BufSize = 0;;
static pthread_mutex_t _gTime = PTHREAD_MUTEX_INITIALIZER;

//static const char _gImageName[] = "image.bin";
static const char _gIniPath[] = "dsp_boot.ini";
static MI_SYS_ChnPort_t _gstVxSinkChnPort;
static MI_SYS_ChnPort_t _gstVxInjectChnPort;
static MI_U32 _gu32FlowFlag = 0;
static MI_U32 _gu32CaseId = 0;
static MI_U32 _gbEnableDisp = 1;    //0;
static MI_BOOL _gbNV12ToVenc = 0;
static UVC_InjectMod_e _geInjectMod = E_INJECT_MOD_UVC;


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
#if 0
    if (gstChnPort.eModId == E_MI_MODULE_ID_WARP)
    {
        if(MI_SUCCESS != MI_SYS_ChnOutputPortGetBuf(pstChnPort, &stBufInfo, &stBufHandle))
        {
            printf("GetBuf fail\n");
            pthread_mutex_unlock(&_gTime);
            return NULL;
        }
        curtime[2] =  Mif_Syscfg_GetTime0();
        u32WriteSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
        write_len = write(fd, stBufInfo.stFrameData.pVirAddr[0], u32WriteSize);
        ASSERT(write_len == u32WriteSize);
        u32BufSize = u32WriteSize;
        u32WriteSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[1] / 2;
        write_len = write(fd, stBufInfo.stFrameData.pVirAddr[0], u32WriteSize);
        ASSERT(write_len == u32WriteSize);
        u32BufSize += u32WriteSize;
        curtime[3] =  Mif_Syscfg_GetTime0();
        MI_SYS_ChnOutputPortPutBuf(stBufHandle);
        curtime[4] =  Mif_Syscfg_GetTime0();
    }
    else //raw data
    {
        pData = malloc(500 * 1024);
        ASSERT(pData);
        stBufInfo.stRawData.pVirAddr = pData;
        if (MI_SUCCESS != MI_SYS_ChnOutputPortCopyToUsr(pstChnPort, &stBufInfo, &stBufHandle))
        {
            printf("Get buffer error!\n");
            pthread_mutex_unlock(&_gTime);
            return 0;
        }
        curtime[2] =  Mif_Syscfg_GetTime0();
        u32WriteSize = stBufInfo.stRawData.u32ContentSize;
        write_len = write(fd, stBufInfo.stRawData.pVirAddr, u32WriteSize);
        ASSERT(write_len == u32WriteSize);
        u32BufSize = u32WriteSize;
        free(pData);
        curtime[3] =  Mif_Syscfg_GetTime0();
    }
#endif 

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
            //memcpy(pData, stBufInfo.stRawData.pVirAddr, u32RetSize);
            memcpy(u8CopyData, stBufInfo.stRawData.pVirAddr, u32RetSize);
            write(wd_fd, stBufInfo.stRawData.pVirAddr, u32RetSize);
            write(wd_fd_size, &u32RetSize, sizeof(MI_U32));
        }
        break;
        case E_MI_SYS_BUFDATA_FRAME:
        {
#if 0
            u32RetSize = stBufInfo.stFrameData.u16Height * (stBufInfo.stFrameData.u32Stride[0] +  stBufInfo.stFrameData.u32Stride[1] / 2);
            memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0]);
            u8CopyData += stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[1], stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[1] /2);
#endif

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

#if 0
    if (gstChnPort.eModId == E_MI_MODULE_ID_WARP || gstChnPort.eModId == E_MI_MODULE_ID_SD) //Frame data
    {
        if (MI_SUCCESS != MI_SYS_ChnOutputPortGetBuf(pstChnPort, &stBufInfo, &stBufHandle))
        {
            printf("Get buffer error!\n");
            pthread_mutex_unlock(&_gTime);
            return 0;
        }
        curtime[2] =  Mif_Syscfg_GetTime0();
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
        curtime[3] =  Mif_Syscfg_GetTime0();
        MI_SYS_ChnOutputPortPutBuf(stBufHandle);
        curtime[4] =  Mif_Syscfg_GetTime0();
    }
    else if (gstChnPort.eModId == E_MI_MODULE_ID_VENC)//raw data
    {
        stBufInfo.stRawData.pVirAddr = pData;
        if (MI_SUCCESS != MI_SYS_ChnOutputPortCopyToUsr(pstChnPort, &stBufInfo, &stBufHandle))
        {
            printf("Get buffer error!\n");
            pthread_mutex_unlock(&_gTime);

            return 0;
        }
        curtime[2] =  Mif_Syscfg_GetTime0();
        u32RetSize = stBufInfo.stRawData.u32ContentSize;
#ifdef UVC_SUPPORT_LL
        if((unsigned int)(stBufInfo.u64SidebandMsg) ==2)
            *is_tail = 1;
        else
            *is_tail = 0;
#else
        *is_tail = 1;
#endif
    }
    else
    {
        printf("not impossible\n");
    }
#endif

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

static MI_S32 UVC_EnableVpe(MI_U16 u16Width, MI_U16 u16Height, MI_SYS_PixelFormat_e eFormat)
{
    printf("UVC_EnableVpe\n");
    ST_VPE_PortInfo_t stVpePortInfo;
    memset(&stVpePortInfo, 0, sizeof(stVpePortInfo));
    stVpePortInfo.u16OutputWidth = u16Width;
    stVpePortInfo.u16OutputHeight = u16Height;
    stVpePortInfo.ePixelFormat = eFormat;
    stVpePortInfo.DepVpeChannel = 0;
    stVpePortInfo.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    STCHECKRESULT(ST_Vpe_CreatePort(MAIN_VENC_PORT, &stVpePortInfo)); //default support port2 --->>> venc

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
    return MI_SUCCESS;
}
static MI_S32 UVC_DisableVpe(void)
{
    STCHECKRESULT(ST_Vpe_StopPort(0, MAIN_VENC_PORT)); //default support port2 --->>> venc

    printf("UVC_DisableVpe\n");
    return MI_SUCCESS;
}
static MI_S32 UVC_EnableWarp(ST_Warp_Timming_e eWarpTimming)
{
    MI_SYS_ChnPort_t stChnPort;
    ST_Sys_BindInfo_t stBindInfo;

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
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = MAIN_VENC_PORT;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_WARP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

    _gstVxSinkChnPort = stChnPort;        // vx sink from warp

    printf("####FUNC %s\n", __FUNCTION__);
    return MI_SUCCESS;
}

static MI_S32 UVC_DisableWarp(void)
{
    ST_Sys_BindInfo_t stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = MAIN_VENC_PORT;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_WARP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    STCHECKRESULT(ST_Warp_DestroyChannel(0)); //default support port2 --->>> venc
    STCHECKRESULT(ST_Warp_Exit());
    printf("####FUNC %s\n", __FUNCTION__);
    return MI_SUCCESS;
}

static MI_S32 UVC_EnableCevaVx(MI_U32 u32Width, MI_U32 u32Height, MI_SYS_PixelFormat_e eFormat)
{
    ST_CEVA_VX_ImageAttr_t stImageAttr;

    if (E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 != eFormat && E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422 != eFormat)
    {
        printf("ceva_vx not support format %d\n", eFormat);
        return -1;
    }

    memset(&stImageAttr, 0, sizeof(ST_CEVA_VX_ImageAttr_t));
    stImageAttr.width = u32Width;
    stImageAttr.height = u32Height;
    stImageAttr.depth = 8;
    stImageAttr.eFormat = eFormat;
    printf("UVC_EnableCevaVx\n");
    ST_CEVA_VX_SetImageAttr(&stImageAttr, _gstVxSinkChnPort, _gstVxInjectChnPort);
    return ST_CEVA_VX_Start(gbUvcSinkFromVx);
}

static MI_S32 UVC_DisableCevaVx(void)
{
    printf("UVC_DisableCevaVx\n");
    return ST_CEVA_VX_Stop(gbUvcSinkFromVx);
}

static MI_S32 UVC_EnableSd(MI_U16 u16Width, MI_U16 u16Height, MI_SYS_PixelFormat_e eFormat)
{
    ST_SD_ChannelInfo_t stSDChannelInfo;
    ST_SD_PortInfo_t stPortInfo;

    stSDChannelInfo.u32X =0;
    stSDChannelInfo.u32Y =0;
    stSDChannelInfo.u16SDCropW = 1920;
    stSDChannelInfo.u16SDCropH = 1080;
    stSDChannelInfo.u16SDMaxW = 1920;
    stSDChannelInfo.u16SDMaxH = 1080;
    STCHECKRESULT(ST_SD_CreateChannel(0, &stSDChannelInfo));

    memset(&stPortInfo, 0, sizeof(ST_SD_PortInfo_t));
    stPortInfo.u16OutputWidth = u16Width;
    stPortInfo.u16OutputHeight = u16Height;
    stPortInfo.ePixelFormat = eFormat;
    stPortInfo.DepSDChannel =0;
    stPortInfo.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    STCHECKRESULT(ST_SD_CreatePort(&stPortInfo));

    if (E_INJECT_MOD_SD == _geInjectMod)
    {
        _gstVxInjectChnPort.eModId = E_MI_MODULE_ID_SD;
        _gstVxInjectChnPort.u32DevId = 0;
        _gstVxInjectChnPort.u32ChnId = 0;
        _gstVxInjectChnPort.u32PortId = 0;
    }

    return MI_SUCCESS;
}
static MI_S32 UVC_DisableSd(void)
{
    STCHECKRESULT(ST_SD_StopPort(0, 0));
    STCHECKRESULT(ST_SD_DestroyChannel(0));

    return MI_SUCCESS;
}

static MI_S32 UVC_EnableVenc(MI_VENC_ModType_e eType, MI_U32 u32Width, MI_U32 u32Height, MI_U32 u32FrmRate, MI_BOOL bByFrame)
{
    MI_U32 u32DevId = 0;
    MI_S32 s32Ret = 0;
    MI_SYS_ChnPort_t stChnPort;
    ST_Sys_BindInfo_t stBindInfo;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ParamJpeg_t stJpegPara;

    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    memset(&stJpegPara, 0, sizeof(MI_VENC_ParamJpeg_t));

    switch (eType)
    {
        case E_MI_VENC_MODTYPE_JPEGE:
            {
                stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
                stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = u32Width;
                stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = u32Height;
                stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = u32Width;
                stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = u32Height;
                stChnAttr.stVeAttr.stAttrJpeg.bByFrame = bByFrame;
                if (3840 == u32Width && 2160 == u32Height)
                {
                    MI_VENC_GetJpegParam(0, &stJpegPara);
                    stJpegPara.u32Qfactor = 50;
                    MI_VENC_SetJpegParam(0, &stJpegPara);
                }
            }
            break;
        case E_MI_VENC_MODTYPE_H264E:
            {
                stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
                stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2; // not support B frame
                stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = u32Width;
                stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = u32Height;
                stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = u32Width;
                stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = u32Height;
                stChnAttr.stVeAttr.stAttrH264e.bByFrame = bByFrame;
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = u32FrmRate;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 36;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 36;
            }
            break;
        case E_MI_VENC_MODTYPE_H265E:
            {
                stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
                stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2; // not support B frame
                stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = u32Width;
                stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = u32Height;
                stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = u32Width;
                stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = u32Height;
                stChnAttr.stVeAttr.stAttrH265e.bByFrame = bByFrame;
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum= u32FrmRate;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen= 1;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = 30;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate=1024*1024;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp=45;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp=25;
            }
            break;
        default:
            printf("error type %d\n", eType);
            return -1;
    }

    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    s32Ret = MI_VENC_CreateChn(0, &stChnAttr);
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
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));

    if (E_INJECT_MOD_VENC == _geInjectMod)
    {
        _gstVxInjectChnPort = stChnPort;
    }
    else
    {
        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_SD;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = 0;
        stBindInfo.stSrcChnPort.u32PortId = 0;
        stBindInfo.stDstChnPort = stChnPort;
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }

    printf("####FUNC %s\n", __FUNCTION__);

    return MI_SUCCESS;
}
static MI_S32 UVC_DisableVenc(void)
{
    ST_Sys_BindInfo_t stBindInfo;
    MI_U32 u32DevId = 0;
    MI_S32 s32Ret = 0;

    s32Ret = MI_VENC_GetChnDevid(0, &u32DevId);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s %d, MI_VENC_GetChnDevid %d error, %X\n", __func__, __LINE__, 0, s32Ret);
    }
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));

    if (E_INJECT_MOD_VENC != _geInjectMod)
    {
        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_SD;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
        stBindInfo.stDstChnPort.u32DevId = u32DevId;
        stBindInfo.stDstChnPort.u32ChnId = 0;
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate = 30;
        stBindInfo.u32DstFrmrate = 30;
        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }

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

    return MI_SUCCESS;
}

static void UVC_EnableWindow(UVC_FormatTimming_e eFormatTimming)
{
    ST_Sys_BindInfo_t stBindInfo;
    MI_S32 s32Ret = 0;
    MI_BOOL bByframe = FALSE;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ParamJpeg_t stJpegPara;

    printf("####FUNC %s\n", __FUNCTION__);
    printf("Timming! %d\n", eFormatTimming);
    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    memset(&stJpegPara, 0, sizeof(MI_VENC_ParamJpeg_t));

    _geInjectMod = E_INJECT_MOD_UVC;

#ifdef UVC_SUPPORT_LL
    bByframe = FALSE;
#else
    bByframe = TRUE;
#endif
    switch(eFormatTimming)
    {
        case E_UVC_TIMMING_4K2K_JPG:    // warp -> vx -> venc -> uvc
            {
                UVC_EnableVpe(3840, 2160, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_3840_2160_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;   // vx inject into venc
                UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 3840, 2160, 30, bByframe);
                UVC_EnableCevaVx(3840, 2160, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_2560X1440P_JPG:
            {
                UVC_EnableVpe(2560, 1440, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_2560_1440_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;   // vx inject into venc
                UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 2560, 1440, 30, bByframe);
                UVC_EnableCevaVx(2560, 1440, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_1920X1080P_JPG:  // warp->vx->sd->venc(NV16->YUYV->MJPG) & warp->vx->venc(NV12->MJPG)
            {
                if (!_gbNV12ToVenc)
                {
                    UVC_EnableVpe(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                    UVC_EnableWarp(E_WARP_1920_1080_NV16);
                    _geInjectMod = E_INJECT_MOD_SD;
                    UVC_EnableSd(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
                    UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 1920, 1080, 30, bByframe);
                    UVC_EnableCevaVx(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                }
                else
                {
                    UVC_EnableVpe(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                    UVC_EnableWarp(E_WARP_1920_1080_NV12);
                    _geInjectMod = E_INJECT_MOD_VENC;
                    UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 1920, 1080, 30, bByframe);
                    UVC_EnableCevaVx(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                }
            }
            break;
        case E_UVC_TIMMING_1280X720P_JPG:
            {
                if (!_gbNV12ToVenc)
                {
                    UVC_EnableVpe(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                    UVC_EnableWarp(E_WARP_1280_720_NV16);
                    _geInjectMod = E_INJECT_MOD_SD;
                    UVC_EnableSd(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
                    UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 1280, 720, 30, bByframe);
                    UVC_EnableCevaVx(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                }
                else
                {
                    UVC_EnableVpe(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                    UVC_EnableWarp(E_WARP_1280_720_NV12);
                    _geInjectMod = E_INJECT_MOD_VENC;
                    UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 1280, 720, 30, bByframe);
                    UVC_EnableCevaVx(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                }
            }
            break;
        case E_UVC_TIMMING_640X480P_JPG:
            {
                if (!_gbNV12ToVenc)
                {
                    UVC_EnableVpe(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                    UVC_EnableWarp(E_WARP_640_480_NV16);
                    _geInjectMod = E_INJECT_MOD_SD;
                    UVC_EnableSd(640, 480, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
                    UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 640, 480, 30, bByframe);
                    UVC_EnableCevaVx(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                }
                else
                {
                    UVC_EnableVpe(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                    UVC_EnableWarp(E_WARP_640_480_NV12);
                    _geInjectMod = E_INJECT_MOD_VENC;
                    UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 640, 480, 30, bByframe);
                    UVC_EnableCevaVx(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                }
            }
            break;
        case E_UVC_TIMMING_320X240P_JPG:
            {
                if (!_gbNV12ToVenc)
                {
                    UVC_EnableVpe(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                    UVC_EnableWarp(E_WARP_320_240_NV16);
                    _geInjectMod = E_INJECT_MOD_SD;
                    UVC_EnableSd(320, 240, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
                    UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 320, 240, 30, bByframe);
                    UVC_EnableCevaVx(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                }
                else
                {
                    UVC_EnableVpe(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                    UVC_EnableWarp(E_WARP_320_240_NV12);
                    _geInjectMod = E_INJECT_MOD_VENC;
                    UVC_EnableVenc(E_MI_VENC_MODTYPE_JPEGE, 320, 240, 30, bByframe);
                    UVC_EnableCevaVx(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                }
            }
            break;
        case E_UVC_TIMMING_4K2K_H264:   // warp -> vx -> venc -> uvc
            {
                UVC_EnableVpe(3840, 2160, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_3840_2160_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H264E, 3840, 2160, 30, bByframe);
                UVC_EnableCevaVx(3840, 2160, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_2560X1440P_H264:
            {
                UVC_EnableVpe(2560, 1440, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_2560_1440_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H264E, 2560, 1440, 30, bByframe);
                UVC_EnableCevaVx(2560, 1440, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_1920X1080P_H264:
            {
                UVC_EnableVpe(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_1920_1080_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H264E, 1920, 1080, 30, bByframe);
                UVC_EnableCevaVx(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_1280X720P_H264:
            {
                UVC_EnableVpe(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_1280_720_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H264E, 1280, 720, 30, bByframe);
                UVC_EnableCevaVx(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_640X480P_H264:
            {
                UVC_EnableVpe(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_640_480_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H264E, 640, 480, 30, bByframe);
                UVC_EnableCevaVx(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_320X240P_H264:
            {
                UVC_EnableVpe(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_320_240_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H264E, 320, 240, 30, bByframe);
                UVC_EnableCevaVx(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_4K2K_H265:   // warp -> vx -> venc -> uvc
            {
                UVC_EnableVpe(3840, 2160, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_3840_2160_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H265E, 3840, 2160, 30, bByframe);
                UVC_EnableCevaVx(3840, 2160, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_2560X1440P_H265:
            {
                UVC_EnableVpe(2560, 1440, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_2560_1440_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H265E, 2560, 1440, 30, bByframe);
                UVC_EnableCevaVx(2560, 1440, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_1920X1080P_H265:
            {
                UVC_EnableVpe(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_1920_1080_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H265E, 1920, 1080, 30, bByframe);
                UVC_EnableCevaVx(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_1280X720P_H265:
            {
                UVC_EnableVpe(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_1280_720_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H265E, 1280, 720, 30, bByframe);
                UVC_EnableCevaVx(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_640X480P_H265:
            {
                UVC_EnableVpe(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_640_480_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H265E, 640, 480, 30, bByframe);
                UVC_EnableCevaVx(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_320X240P_H265:
            {
                UVC_EnableVpe(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_320_240_NV12);
                _geInjectMod = E_INJECT_MOD_VENC;
                UVC_EnableVenc(E_MI_VENC_MODTYPE_H265E, 320, 240, 30, bByframe);
                UVC_EnableCevaVx(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_1920X1080_NV12:      // warp -> vx -> uvc
            {
                UVC_EnableVpe(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_1920_1080_NV12);
                UVC_EnableCevaVx(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_1280X720_NV12:
            {
                UVC_EnableVpe(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_1280_720_NV12);
                UVC_EnableCevaVx(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_640X480_NV12:
            {
                UVC_EnableVpe(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_640_480_NV12);
                UVC_EnableCevaVx(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_320X240_NV12:
            {
                UVC_EnableVpe(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                UVC_EnableWarp(E_WARP_320_240_NV12);
                UVC_EnableCevaVx(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
            }
            break;
        case E_UVC_TIMMING_1920X1080_YUV422_YUYV:   // warp -> vx -> sd -> uvc
            {
                UVC_EnableVpe(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                UVC_EnableWarp(E_WARP_1920_1080_NV16);
                _geInjectMod = E_INJECT_MOD_SD;
                UVC_EnableSd(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
                UVC_EnableCevaVx(1920, 1080, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
            }
            break;
        case E_UVC_TIMMING_1280X720_YUV422_YUYV:
            {
                UVC_EnableVpe(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                UVC_EnableWarp(E_WARP_1280_720_NV16);
                _geInjectMod = E_INJECT_MOD_SD;
                UVC_EnableSd(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
                UVC_EnableCevaVx(1280, 720, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
            }
            break;
        case E_UVC_TIMMING_640X480_YUV422_YUYV:
            {
                UVC_EnableVpe(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                UVC_EnableWarp(E_WARP_640_480_NV16);
                _geInjectMod = E_INJECT_MOD_SD;
                UVC_EnableSd(640, 480, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
                UVC_EnableCevaVx(640, 480, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
            }
            break;
        case E_UVC_TIMMING_320X240_YUV422_YUYV:
            {
                UVC_EnableVpe(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
                UVC_EnableWarp(E_WARP_320_240_NV16);
                _geInjectMod = E_INJECT_MOD_SD;
                UVC_EnableSd(320, 240, E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
                UVC_EnableCevaVx(320, 240, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422);
            }
            break;
        default:
            printf("Format not support!\n");
            return;
    }
}
static void UVC_DisableWindow(MI_BOOL bDisableVenc,MI_BOOL bDisableWarp, MI_BOOL bDisableSd)
{
    UVC_DisableCevaVx();

    if (bDisableVenc)
    {
        UVC_DisableVenc();
    }
    if (bDisableSd)
    {
        UVC_DisableSd();
    }
    if(bDisableWarp)
    {
        UVC_DisableWarp();
    }
    UVC_DisableVpe();

    printf("####FUNC %s\n", __FUNCTION__);
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
#if 0
    FILE *pFile = NULL;
    char szFilePath[50];
    static MI_S32 s32FileIndex = 0;
#endif

#ifndef SUPPORT_WRITE_FILE
    if (gbUvcSinkFromVx)
    {
        bufInfo->length = ST_CEVA_VX_GetOutputBuf(bufInfo->b.buf);
//        printf("get buf length %d\n", bufInfo->length);

#if 0
        if (bufInfo->length > 0)
        {
            s32FileIndex++;
            memset(szFilePath, 0, sizeof(szFilePath));
            sprintf(szFilePath, "UvcGetBuf_%d.yuv", s32FileIndex);
            pFile = fopen(szFilePath, "ab+");
            if (pFile)
            {
                fwrite(bufInfo->b.buf, bufInfo->length, 1, pFile);
                fclose(pFile);
                pFile = NULL;
            }
        }
#endif

    }
    else
    {
        bufInfo->length = ST_DoGetData_mmap(&gstChnPort, bufInfo->b.buf,&(bufInfo->is_tail));
    }
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
            gbUvcSinkFromVx = TRUE; // get buf from vx

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
            gbUvcSinkFromVx = FALSE;

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
            gbUvcSinkFromVx = FALSE;

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
            gbUvcSinkFromVx = FALSE;

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
            gbUvcSinkFromVx = FALSE;

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

    if (gbUvcSinkFromVx)
    {
        UVC_DisableWindow(FALSE, TRUE, FALSE);
    }
    else
    {
        if (gstChnPort.eModId == E_MI_MODULE_ID_VENC)
        {
            if (_geInjectMod == E_INJECT_MOD_SD)
                UVC_DisableWindow(TRUE, TRUE,TRUE);
            else
                UVC_DisableWindow(TRUE, TRUE,FALSE);
        }
        else if (gstChnPort.eModId == E_MI_MODULE_ID_SD)
        {
            UVC_DisableWindow(FALSE, TRUE, TRUE);
        }
        else
            printf("Not support!\n");
    }


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

    printf("ceva_vx dsp boot up\n");
    STCHECKRESULT(ST_CEVA_VX_DspBootUp(_gIniPath));
    return MI_SUCCESS;
}
static MI_S32 St_UvcDeinit()
{
    printf("uvc deinit\n");

    printf("ceva_vx dsp shut down\n");
    STCHECKRESULT(ST_CEVA_VX_DspShutDown());

    STCHECKRESULT(ST_UVC_StopDev());
    STCHECKRESULT(ST_UVC_DestroyDev(0));
    STCHECKRESULT(ST_UVC_Uninit());

    return MI_SUCCESS;
}

static MI_S32 St_UacInit()
{
    MI_AUDIO_Attr_t stAiAttr, stAoAttr;
    ST_Sys_BindInfo_t stBindInfo;

    /************************************************
    Step1:  init AI
    *************************************************/
    memset(&stAiAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stAiAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAiAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
    stAiAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAiAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stAiAttr.u32ChnCnt = 2;
    stAiAttr.u32FrmNum = 6;
    stAiAttr.u32PtNumPerFrm = 256;
    ExecFunc(MI_AI_SetPubAttr(1, &stAiAttr), MI_SUCCESS);
    ExecFunc(MI_AI_GetPubAttr(1, &stAiAttr), MI_SUCCESS);
    ExecFunc(MI_AI_Enable(1), MI_SUCCESS);
    ExecFunc(MI_AI_EnableChn(1, 0), MI_SUCCESS);

    /************************************************
    Step2:  init AO
    *************************************************/
    memset(&stAoAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stAoAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAoAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
    stAoAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAoAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stAoAttr.u32ChnCnt = 1;
    stAoAttr.u32FrmNum = 6;
    stAoAttr.u32PtNumPerFrm = 256;
    ExecFunc(MI_AO_SetPubAttr(0, &stAoAttr), MI_SUCCESS);
    ExecFunc(MI_AO_GetPubAttr(0, &stAoAttr), MI_SUCCESS);
    ExecFunc(MI_AO_Enable(0), MI_SUCCESS);
    ExecFunc(MI_AO_EnableChn(0, 0), MI_SUCCESS);

    /************************************************
    Step3:  init UAC
    *************************************************/
    ExecFunc(MI_UAC_Init(),MI_SUCCESS);
    ExecFunc(MI_UAC_OpenDevice(UAC_CAPTURE_DEV),MI_SUCCESS);
    ExecFunc(MI_UAC_OpenDevice(UAC_PLAYBACK_DEV),MI_SUCCESS);

    /************************************************
    Step3:  bind AI to UAC capture
    *************************************************/
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_AI;
    stBindInfo.stSrcChnPort.u32DevId = 1;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    ExecFunc(MI_SYS_SetChnOutputPortDepth(&stBindInfo.stSrcChnPort, 12, 13), MI_SUCCESS);

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_UAC;
    stBindInfo.stDstChnPort.u32DevId = UAC_CAPTURE_DEV;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 0;
    stBindInfo.u32DstFrmrate = 0;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

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

    ExecFunc(MI_UAC_StartDev(UAC_CAPTURE_DEV),MI_SUCCESS);
    ExecFunc(MI_UAC_StartDev(UAC_PLAYBACK_DEV),MI_SUCCESS);

    return MI_SUCCESS;
}

static MI_S32 St_UacDeinit()
{
    ST_Sys_BindInfo_t stBindInfo;

    ExecFunc(MI_UAC_StopDev(UAC_PLAYBACK_DEV),MI_SUCCESS);
    ExecFunc(MI_UAC_StopDev(UAC_CAPTURE_DEV),MI_SUCCESS);
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

    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_AI;
    stBindInfo.stSrcChnPort.u32DevId = 1;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_UAC;
    stBindInfo.stDstChnPort.u32DevId = UAC_CAPTURE_DEV;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 0;
    stBindInfo.u32DstFrmrate = 0;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));

    ExecFunc(MI_AI_DisableChn(1, 0), MI_SUCCESS);
    ExecFunc(MI_AI_Disable(1), MI_SUCCESS);
    ExecFunc(MI_AO_DisableChn(0, 0), MI_SUCCESS);
    ExecFunc(MI_AO_Disable(0), MI_SUCCESS);

    ExecFunc(MI_UAC_CloseDevice(UAC_CAPTURE_DEV),MI_SUCCESS);
    ExecFunc(MI_UAC_CloseDevice(UAC_PLAYBACK_DEV),MI_SUCCESS);
    ExecFunc(MI_UAC_Exit(),MI_SUCCESS);

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
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    STCHECKRESULT(ST_Disp_DeInit(ST_DISP_DEV0, ST_DISP_LAYER0, 1));
    STCHECKRESULT(ST_Vpe_StopPort(0, DISP_PORT));

    return MI_SUCCESS;
}

static MI_S32 St_BaseModuleInit(MI_BOOL bFramOrReal)
{
    ST_VIF_PortInfo_t stVifPortInfoInfo;
    MI_SYS_ChnPort_t stChnPort;
    ST_VPE_ChannelInfo_t stVpeChannelInfo;
    ST_Sys_BindInfo_t stBindInfo;

    memset(&stVifPortInfoInfo, 0x0, sizeof(ST_VIF_PortInfo_t));
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stVpeChannelInfo, 0x0, sizeof(ST_VPE_ChannelInfo_t));

    // init vif
    STCHECKRESULT(ST_Sys_Init());
    STCHECKRESULT(ST_Vif_CreateDev(0, bFramOrReal?SAMPLE_VI_MODE_MIPI_1_1080P_FRAME:SAMPLE_VI_MODE_MIPI_1_1080P_REALTIME));
    stVifPortInfoInfo.u32RectX = 0;
    stVifPortInfoInfo.u32RectY = 0;
    stVifPortInfoInfo.u32RectWidth = 3840;
    stVifPortInfoInfo.u32RectHeight = 2160;
    stVifPortInfoInfo.u32DestWidth = 3840;
    stVifPortInfoInfo.u32DestHeight = 2160;
    stVifPortInfoInfo.ePixFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
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
    stVpeChannelInfo.eRunningMode = bFramOrReal?E_MI_VPE_RUNNING_MODE_FRAMEBUF_CAM_MODE:E_MI_VPE_RUNNING_MODE_REALTIME_MODE;
    stVpeChannelInfo.eFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
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

static MI_S32 _IsNv12ToVenc(void)
{
    MI_S32 s32Choose = 0;
    printf("Which flow do you want?\n 0: YUY2 to MJPG, 1: NV12 to MJPG\n");
    scanf("%d", &s32Choose);
    printf("You select input %s to venc by default\n", s32Choose?"YUY2":"NV12");

    return s32Choose;
}

MI_S32 main(int argc, char **argv)
{
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

    _gbNV12ToVenc = _IsNv12ToVenc();

    if (argc > 1)
        _gbEnableDisp = atoi(argv[1]);

    St_BaseModuleInit(0);   // test realtime mode

    if (_gbEnableDisp)
        St_DisplayInit();

    St_UvcInit();// vif->vpe->xm6->venc->uvc
//    St_UacInit();
#if(ENABLE_DUMPCIF_PORT1 == 1)
    DC_TemCreate();
#endif
    // wait for quit command, if true jion thread
    while(1)
    {
        printf("Type \"q\" to exit\n");
        cmd = getchar();
        if (cmd == 'q')
            break;

        if (cmd == 'p')
            ST_CEVA_VX_SwitchLog();

        pthread_mutex_lock(&_gTime);
        for (i = 0; i < 5; i++)
        {
            printf("time [%d] = %d\n", i, curtime[i]);
        }
        printf("Buf size is %d\n", u32BufSize);
        pthread_mutex_unlock(&_gTime);
    }
//    St_UacDeinit();
    St_UvcDeinit();
    printf("St_UvcDeinit deinit\n");

    if (_gbEnableDisp)
    {
        St_DisplayDeinit();
        printf("St_DisplayDeinit deinit\n");
    }

    St_BaseModuleDeinit();
    return 0;
}


