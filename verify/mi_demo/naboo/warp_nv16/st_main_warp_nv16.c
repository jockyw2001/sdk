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
#include "mi_venc.h"
#include "st_uvc.h"
#include "st_common.h"
#include "st_hdmi.h"
#include "st_disp.h"
#include "st_vpe.h"
#include "st_vdisp.h"
#include "st_vif.h"
#include "st_fb.h"
#include "st_warp.h"
#include "st_sd.h"
#include "list.h"
#include "mi_rgn.h"
#include "i2c.h"
#include "tem.h"


#define DEBUG_OPEN

typedef enum
{
    E_DEBUG_LEVEL_ERR,
    E_DEBUG_LEVEL_WARN,
    E_DEBUG_LEVEL_INFO,
    E_DEBUG_LEVEL_NORMAL,
    E_DEBUG_LEVEL_ALL
}ST_DEBUG_LEVEL_e;

#ifdef DEBUG_OPEN
#define DEBUG_LEVEL E_DEBUG_LEVEL_INFO
#else
#define DEBUG_LEVEL E_DEBUG_LEVEL_WARN
#endif

#define ST_WARP_LOG_ERR(fmt, args...) {if(DEBUG_LEVEL >= E_DEBUG_LEVEL_ERR) printf("\033[1;31m");printf(fmt, ##args);printf("\033[0m");}
#define ST_WARP_LOG_WARN(fmt, args...) {if(DEBUG_LEVEL >= E_DEBUG_LEVEL_WARN) printf("\033[1;33m");printf(fmt, ##args);printf("\033[0m");}
#define ST_WARP_LOG_INFO(fmt, args...) {if(DEBUG_LEVEL >= E_DEBUG_LEVEL_INFO) {printf("\033[1;34m");printf(fmt, ##args);printf("\033[0m");}}
#define ST_WARP_LOG_NORMAL(fmt, args...) {if(DEBUG_LEVEL >= E_DEBUG_LEVEL_ALL) {printf(fmt, ##args);}}

#define ST_CHECK_RESULT(parseline, label, jumpFlag) do{  \
    if (MI_SUCCESS == parseline)\
    {\
        ST_WARP_LOG_NORMAL("[%s %d]exec function pass\n", __FUNCTION__, __LINE__);  \
    }\
    else\
    {\
        ST_WARP_LOG_ERR("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);    \
        if (jumpFlag)   \
            goto label; \
    }\
}while (0);

#define ST_SHOW_CURRENT_TIMING(eTiming) do{ \
    switch(eTiming) \
    {   \
        case E_WARP_320_240_NV12:   \
            ST_WARP_LOG_INFO("current timing:%d %s_%d_%d\n", eTiming, "NV12", 320, 240); \
            break;  \
        case E_WARP_640_480_NV12:   \
            ST_WARP_LOG_INFO("current timing:%d %s_%d_%d\n", eTiming, "NV12", 640, 480); \
            break;  \
        case E_WARP_1280_720_NV12:  \
            ST_WARP_LOG_INFO("current timing:%d %s_%d_%d\n", eTiming, "NV12", 1280, 720); \
            break;  \
        case E_WARP_1920_1080_NV12: \
            ST_WARP_LOG_INFO("current timing:%d %s_%d_%d\n", eTiming, "NV12", 1920, 1080); \
            break;  \
        case E_WARP_320_240_NV16:   \
            ST_WARP_LOG_INFO("current timing:%d %s_%d_%d\n", eTiming, "YUV422SP", 320, 240); \
            break;  \
        case E_WARP_640_480_NV16:\
            ST_WARP_LOG_INFO("current timing:%d %s_%d_%d\n", eTiming, "YUV422SP", 640, 480); \
            break;  \
        case E_WARP_1280_720_NV16:  \
            ST_WARP_LOG_INFO("current timing:%d %s_%d_%d\n", eTiming, "YUV422SP", 1280, 720); \
            break;  \
        case E_WARP_1920_1080_NV16: \
            ST_WARP_LOG_INFO("current timing:%d %s_%d_%d\n", eTiming, "YUV422SP", 1920, 1080); \
            break;  \
        default:    \
            ST_WARP_LOG_INFO("Timing not support\n"); \
    }\
}while (0);




// vif -> vpe -> warp -> sd -> disp
static MI_S32 St_BaseModuleInit(void)
{
    ST_VIF_PortInfo_t stVifPortInfoInfo;
    MI_SYS_ChnPort_t stChnPort;
    ST_VPE_ChannelInfo_t stVpeChannelInfo;
    ST_Sys_BindInfo_t stBindInfo;

    memset(&stVifPortInfoInfo, 0x0, sizeof(ST_VIF_PortInfo_t));
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stVpeChannelInfo, 0x0, sizeof(ST_VPE_ChannelInfo_t));

    ST_CHECK_RESULT(ST_Sys_Init(), exit, 0);

    // init vif
    ST_CHECK_RESULT(ST_Vif_CreateDev(0, SAMPLE_VI_MODE_MIPI_1_1080P_REALTIME), sys_deinit, 0);
    stVifPortInfoInfo.u32RectX = 0;
    stVifPortInfoInfo.u32RectY = 0;
    stVifPortInfoInfo.u32RectWidth = 3840;
    stVifPortInfoInfo.u32RectHeight = 2160;
    stVifPortInfoInfo.u32DestWidth = 3840;
    stVifPortInfoInfo.u32DestHeight = 2160;
    stVifPortInfoInfo.ePixFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
    ST_CHECK_RESULT(ST_Vif_CreatePort(0, 0, &stVifPortInfoInfo), destroy_vif_dev, 0);
    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;
    stChnPort.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 3);
    ST_CHECK_RESULT(ST_Vif_StartPort(0, 0), destroy_vif_dev, 0);

    // init vpe
    stVpeChannelInfo.u16VpeMaxW = 3840;
    stVpeChannelInfo.u16VpeMaxH = 2160;
    stVpeChannelInfo.u32X = 0;
    stVpeChannelInfo.u32Y = 0;
    stVpeChannelInfo.u16VpeCropW = 3840;
    stVpeChannelInfo.u16VpeCropH = 2160;
    stVpeChannelInfo.eRunningMode = E_MI_VPE_RUNNING_MODE_REALTIME_MODE;
    stVpeChannelInfo.eFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
    ST_CHECK_RESULT(ST_Vpe_CreateChannel(0, &stVpeChannelInfo), stop_vif_port, 0);
    ST_CHECK_RESULT(ST_Vpe_StartChannel(0), destroy_vpe_chn0, 0);

    // bind vif to vpe
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

    ST_CHECK_RESULT(ST_Sys_Bind(&stBindInfo), stop_vpe_chn0, 0);
    return MI_SUCCESS;

stop_vpe_chn0:
    ST_Vpe_StopChannel(0);

destroy_vpe_chn0:
    ST_Vpe_DestroyChannel(0);

stop_vif_port:
    ST_Vif_StopPort(0, 0);

destroy_vif_dev:
    ST_Vif_DisableDev(0);

sys_deinit:
    ST_Sys_Exit();

exit:
    return -1;
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
    ST_CHECK_RESULT(ST_Sys_UnBind(&stBindInfo), exit, 0);
#if 0
    ST_CHECK_RESULT(ST_Vpe_StopChannel(1), exit, 0);
    ST_CHECK_RESULT(ST_Vpe_DestroyChannel(1), exit, 0);
#endif
    ST_CHECK_RESULT(ST_Vpe_StopChannel(0), exit, 0);
    ST_CHECK_RESULT(ST_Vpe_DestroyChannel(0), exit, 0);
    ST_CHECK_RESULT(ST_Vif_StopPort(0, 0), exit, 0);
    ST_CHECK_RESULT(ST_Vif_DisableDev(0), exit, 0);
    ST_CHECK_RESULT(ST_Sys_Exit(), exit, 0);

exit:
    return MI_SUCCESS;
}

static MI_S32 _St_CreateSd(ST_Warp_Timming_e eTiming)
{
    ST_SD_ChannelInfo_t stSDChannelInfo;
    ST_SD_PortInfo_t stSDPortInfo;
    MI_SYS_ChnPort_t stChnPort;

    stSDChannelInfo.u32X =0;
    stSDChannelInfo.u32Y =0;
    stSDPortInfo.DepSDChannel = 0;
    stSDPortInfo.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stSDPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    switch(eTiming)
    {
        case E_WARP_320_240_NV12:
            stSDChannelInfo.u16SDCropW = 320;
            stSDChannelInfo.u16SDCropH = 240;
            stSDChannelInfo.u16SDMaxW = 320;
            stSDChannelInfo.u16SDMaxH = 240;
            stSDPortInfo.u16OutputWidth = 320;
            stSDPortInfo.u16OutputHeight = 240;
            break;
        case E_WARP_640_480_NV12:
            stSDChannelInfo.u16SDCropW = 640;
            stSDChannelInfo.u16SDCropH = 480;
            stSDChannelInfo.u16SDMaxW = 640;
            stSDChannelInfo.u16SDMaxH = 480;
            stSDPortInfo.u16OutputWidth = 640;
            stSDPortInfo.u16OutputHeight = 480;
            break;
        case E_WARP_1280_720_NV12:
            stSDChannelInfo.u16SDCropW = 1280;
            stSDChannelInfo.u16SDCropH = 720;
            stSDChannelInfo.u16SDMaxW = 1280;
            stSDChannelInfo.u16SDMaxH = 720;
            stSDPortInfo.u16OutputWidth = 1280;
            stSDPortInfo.u16OutputHeight = 720;
            break;
        case E_WARP_1920_1080_NV12:
            stSDChannelInfo.u16SDCropW = 1920;
            stSDChannelInfo.u16SDCropH = 1080;
            stSDChannelInfo.u16SDMaxW = 1920;
            stSDChannelInfo.u16SDMaxH = 1080;
            stSDPortInfo.u16OutputWidth = 1920;
            stSDPortInfo.u16OutputHeight = 1080;
            break;
        case E_WARP_320_240_NV16:
            stSDChannelInfo.u16SDCropW = 320;
            stSDChannelInfo.u16SDCropH = 240;
            stSDChannelInfo.u16SDMaxW = 320;
            stSDChannelInfo.u16SDMaxH = 240;
            stSDPortInfo.u16OutputWidth = 320;
            stSDPortInfo.u16OutputHeight = 240;
            break;
        case E_WARP_640_480_NV16:
            stSDChannelInfo.u16SDCropW = 640;
            stSDChannelInfo.u16SDCropH = 480;
            stSDChannelInfo.u16SDMaxW = 640;
            stSDChannelInfo.u16SDMaxH = 480;
            stSDPortInfo.u16OutputWidth = 640;
            stSDPortInfo.u16OutputHeight = 480;
            break;
        case E_WARP_1280_720_NV16:
            stSDChannelInfo.u16SDCropW = 1280;
            stSDChannelInfo.u16SDCropH = 720;
            stSDChannelInfo.u16SDMaxW = 1280;
            stSDChannelInfo.u16SDMaxH = 720;
            stSDPortInfo.u16OutputWidth = 1280;
            stSDPortInfo.u16OutputHeight = 720;
            break;
        case E_WARP_1920_1080_NV16:
            stSDChannelInfo.u16SDCropW = 1920;
            stSDChannelInfo.u16SDCropH = 1080;
            stSDChannelInfo.u16SDMaxW = 1920;
            stSDChannelInfo.u16SDMaxH = 1080;
            stSDPortInfo.u16OutputWidth = 1920;
            stSDPortInfo.u16OutputHeight = 1080;
            break;
        default:
            printf("Timing not support\n");
            return -1;
    }

    ST_CHECK_RESULT(ST_SD_Init(), exit, 0);
    ST_CHECK_RESULT(ST_SD_CreateChannel(0, &stSDChannelInfo), exit, 0);
    ST_CHECK_RESULT(ST_SD_CreatePort(&stSDPortInfo), exit, 0);

    stChnPort.eModId = E_MI_MODULE_ID_SD;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;
    stChnPort.u32PortId = 0;
    ST_CHECK_RESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 1, 4), exit, 0);

    return MI_SUCCESS;

exit:
    return -1;
}

static MI_S32 _St_DestroySd(MI_SYS_ChnPort_t stChnPort)
{
    ST_CHECK_RESULT(ST_SD_StopPort(stChnPort.u32ChnId, stChnPort.u32PortId), exit, 0);
    ST_CHECK_RESULT(ST_SD_DestroyChannel(stChnPort.u32ChnId), exit, 0);
    ST_CHECK_RESULT(ST_SD_Exit(), exit, 0);

    return MI_SUCCESS;

exit:
    return -1;
}

static MI_S32 _St_DisplayInit(ST_Warp_Timming_e eTiming)
{
    ST_DispChnInfo_t stDispChnInfo;
    ST_Sys_BindInfo_t stBindInfo;

    memset(&stDispChnInfo, 0x0, sizeof(ST_DispChnInfo_t));

    // create sd
    _St_CreateSd(eTiming);

    // init disp
    ST_CHECK_RESULT(ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, E_MI_DISP_OUTPUT_1080P60), stop_vpe_port, 0); //Dispout timing
    stDispChnInfo.InputPortNum = 1;
    stDispChnInfo.stInputPortAttr[0].u32Port = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16X = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Y = 0;

    switch(eTiming)
    {
        case E_WARP_320_240_NV12:
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 320;
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 240;
            break;
        case E_WARP_640_480_NV12:
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 640;
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 480;
            break;
        case E_WARP_1280_720_NV12:
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 1280;
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 720;
            break;
        case E_WARP_1920_1080_NV12:
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 1920;
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 1080;
            break;
        case E_WARP_320_240_NV16:
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 320;
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 240;
            break;
        case E_WARP_640_480_NV16:
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 640;
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 480;
            break;
        case E_WARP_1280_720_NV16:
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 1280;
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 720;
            break;
        case E_WARP_1920_1080_NV16:
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 1920;
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 1080;
            break;
        default:
            printf("Timing not support\n");
            return -1;

    }

    ST_CHECK_RESULT(ST_Disp_ChnInit(0, &stDispChnInfo), deinit_disp_dev, 0);

    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_SD;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;

    ST_CHECK_RESULT(ST_Sys_Bind(&stBindInfo), deinit_disp_dev, 0);

    // init hdmi
    ST_CHECK_RESULT(ST_Hdmi_Init(), unbind, 0);
    ST_CHECK_RESULT(ST_Hdmi_Start(E_MI_HDMI_ID_0, E_MI_HDMI_TIMING_1080_60P), deinit_hdmi, 0); //Hdmi timing

    return MI_SUCCESS;

deinit_hdmi:
    ST_Hdmi_DeInit(E_MI_HDMI_ID_0);

unbind:
    ST_Sys_UnBind(&stBindInfo);

deinit_disp_dev:
    ST_Disp_DeInit(ST_DISP_DEV0, ST_DISP_LAYER0, 1);

stop_vpe_port:
    ST_Vpe_StopPort(1, 0);

exit:
    return -1;
}

static MI_S32 _St_DisplayDeinit(void)
{
    ST_Sys_BindInfo_t stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_SD;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;

    // deinit hdmi
    ST_CHECK_RESULT(ST_Hdmi_DeInit(E_MI_HDMI_ID_0), exit, 0);

    // unbind
    ST_Sys_UnBind(&stBindInfo);

    // deinit sd
    _St_DestroySd(stBindInfo.stSrcChnPort);

    // deinit disp
    ST_CHECK_RESULT(ST_Disp_DeInit(ST_DISP_DEV0, ST_DISP_LAYER0, 1), exit, 0);

exit:
    return MI_SUCCESS;
}

// use venc port
static MI_S32 _St_CreateVpePort(MI_SYS_ChnPort_t stChnPort, ST_Warp_Timming_e eTiming)
{
    ST_VPE_PortInfo_t stVpePortInfo;
    stVpePortInfo.DepVpeChannel = stChnPort.u32ChnId;
    stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    switch(eTiming)
    {
        case E_WARP_320_240_NV12:
            stVpePortInfo.u16OutputWidth = 320;
            stVpePortInfo.u16OutputHeight = 240;
            break;
        case E_WARP_640_480_NV12:
            stVpePortInfo.u16OutputWidth = 640;
            stVpePortInfo.u16OutputHeight = 480;
            break;
        case E_WARP_1280_720_NV12:
            stVpePortInfo.u16OutputWidth = 1280;
            stVpePortInfo.u16OutputHeight = 720;
            break;
        case E_WARP_1920_1080_NV12:
            stVpePortInfo.u16OutputWidth = 1920;
            stVpePortInfo.u16OutputHeight = 1080;
            break;
        case E_WARP_320_240_NV16:
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
            stVpePortInfo.u16OutputWidth = 320;
            stVpePortInfo.u16OutputHeight = 240;
            break;
        case E_WARP_640_480_NV16:
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
            stVpePortInfo.u16OutputWidth = 640;
            stVpePortInfo.u16OutputHeight = 480;
            break;
        case E_WARP_1280_720_NV16:
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
            stVpePortInfo.u16OutputWidth = 1280;
            stVpePortInfo.u16OutputHeight = 720;
            break;
        case E_WARP_1920_1080_NV16:
            stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
            stVpePortInfo.u16OutputWidth = 1920;
            stVpePortInfo.u16OutputHeight = 1080;
            break;
        default:
            printf("Timing not support\n");
            return -1;
    }

//    stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    ST_CHECK_RESULT(ST_Vpe_CreatePort(stChnPort.u32PortId, &stVpePortInfo), exit, 0);

    return MI_SUCCESS;

exit:
    return -1;
}

static MI_S32 _St_DestroyVpePort(MI_SYS_ChnPort_t stChnPort)
{
    ST_CHECK_RESULT(ST_Vpe_StopPort(stChnPort.u32ChnId, stChnPort.u32PortId), exit, 0);
    return MI_SUCCESS;

exit:
    return -1;
}

static MI_S32 _St_CreateWarp(MI_SYS_ChnPort_t stChnPort, ST_Warp_Timming_e eTiming)
{
    ST_CHECK_RESULT(ST_Warp_Init(eTiming), exit, 0);
    ST_CHECK_RESULT(ST_Warp_CreateChannel(stChnPort.u32ChnId), deinit_warp, 0);

    return MI_SUCCESS;

deinit_warp:
    ST_CHECK_RESULT(ST_Warp_Exit(), exit, 0);

exit:
    return -1;
}

static MI_S32 _St_DestroyWarp(MI_SYS_ChnPort_t stChnPort)
{
    ST_CHECK_RESULT(ST_Warp_DestroyChannel(stChnPort.u32ChnId), exit, 0);
    ST_CHECK_RESULT(ST_Warp_Exit(), exit, 0);

    return MI_SUCCESS;

exit:
    return -1;
}




// vpe->warp->sd,sd bind to disp        ST_Disp_ChnInit
static MI_S32 St_WarpFlowInit(ST_Warp_Timming_e eTiming)
{
    MI_SYS_ChnPort_t stVpeChnPort, stWarpChnPort, stSdChnPort, stDispChnPort;
    ST_Sys_BindInfo_t stBindVpeToWarp, stBindWarpToSd, stBindSdToDisp;
    memset(&stBindVpeToWarp, 0, sizeof(ST_Sys_BindInfo_t));
    memset(&stBindWarpToSd, 0, sizeof(ST_Sys_BindInfo_t));
    memset(&stBindSdToDisp, 0, sizeof(ST_Sys_BindInfo_t));

    stVpeChnPort.eModId = E_MI_MODULE_ID_VPE;
    stVpeChnPort.u32DevId = 0;
    stVpeChnPort.u32ChnId = 0;
    stVpeChnPort.u32PortId = 0;

    stWarpChnPort.eModId = E_MI_MODULE_ID_WARP;
    stWarpChnPort.u32DevId = 0;
    stWarpChnPort.u32ChnId = 0;
    stWarpChnPort.u32PortId = 0;

    stSdChnPort.eModId = E_MI_MODULE_ID_SD;
    stSdChnPort.u32DevId = 0;
    stSdChnPort.u32ChnId = 0;
    stSdChnPort.u32PortId = 0;

    ST_SHOW_CURRENT_TIMING(eTiming);

    // create vpe port
    _St_CreateVpePort(stVpeChnPort, eTiming);

    // create warp
    _St_CreateWarp(stWarpChnPort, eTiming);

    // create sd&disp
    _St_DisplayInit(eTiming);

    // bind vpe to warp
    stBindVpeToWarp.stSrcChnPort = stVpeChnPort;
    stBindVpeToWarp.stDstChnPort = stWarpChnPort;
    stBindVpeToWarp.u32SrcFrmrate = 30;
    stBindVpeToWarp.u32DstFrmrate = 30;
    ST_CHECK_RESULT(MI_SYS_SetChnOutputPortDepth(&stVpeChnPort, 2, 5), exit, 0);
    ST_CHECK_RESULT(ST_Sys_Bind(&stBindVpeToWarp), exit, 0);

    // bind warp to sd
    stBindWarpToSd.stSrcChnPort = stWarpChnPort;
    stBindWarpToSd.stDstChnPort = stSdChnPort;
    stBindWarpToSd.u32SrcFrmrate = 30;
    stBindWarpToSd.u32DstFrmrate = 30;
    ST_CHECK_RESULT(MI_SYS_SetChnOutputPortDepth(&stWarpChnPort, 2, 4), unbind_vpe_warp, 0);
    ST_CHECK_RESULT(ST_Sys_Bind(&stBindWarpToSd), unbind_vpe_warp, 0);

    return MI_SUCCESS;

unbind_vpe_warp:
    ST_Sys_UnBind(&stBindVpeToWarp);

exit:
    printf("St_WarpFlowInit goto exit\n");
    _St_DisplayDeinit();
    _St_DestroyWarp(stWarpChnPort);
    _St_DestroyVpePort(stVpeChnPort);

    return -1;
}


static MI_S32 St_WarpFlowDeinit()
{
    ST_Sys_BindInfo_t stBindVpeToWarp, stBindWarpToSd;

    stBindVpeToWarp.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindVpeToWarp.stSrcChnPort.u32DevId = 0;
    stBindVpeToWarp.stSrcChnPort.u32ChnId = 0;
    stBindVpeToWarp.stSrcChnPort.u32PortId = MAIN_VENC_PORT;
    stBindVpeToWarp.stDstChnPort.eModId = E_MI_MODULE_ID_WARP;
    stBindVpeToWarp.stDstChnPort.u32DevId = 0;
    stBindVpeToWarp.stDstChnPort.u32ChnId = 0;
    stBindVpeToWarp.stDstChnPort.u32PortId = 0;
    stBindVpeToWarp.u32SrcFrmrate = 30;
    stBindVpeToWarp.u32DstFrmrate = 30;

    stBindWarpToSd.stSrcChnPort.eModId = E_MI_MODULE_ID_WARP;
    stBindWarpToSd.stSrcChnPort.u32DevId = 0;
    stBindWarpToSd.stSrcChnPort.u32ChnId = 0;
    stBindWarpToSd.stSrcChnPort.u32PortId = 0;
    stBindWarpToSd.stDstChnPort.eModId = E_MI_MODULE_ID_SD;
    stBindWarpToSd.stDstChnPort.u32DevId = 0;
    stBindWarpToSd.stDstChnPort.u32ChnId = 0;
    stBindWarpToSd.stDstChnPort.u32PortId = 0;
    stBindWarpToSd.u32SrcFrmrate = 30;
    stBindWarpToSd.u32DstFrmrate = 30;

    ST_CHECK_RESULT(ST_Sys_UnBind(&stBindWarpToSd), exit, 0);
    ST_CHECK_RESULT(ST_Sys_UnBind(&stBindVpeToWarp), exit, 0);

    _St_DestroyWarp(stBindWarpToSd.stSrcChnPort);
    _St_DestroyVpePort(stBindVpeToWarp.stSrcChnPort);
    _St_DisplayDeinit();

    return MI_SUCCESS;

exit:
    return -1;
}

// only change timing
static MI_S32 St_WarpSetTiming(ST_Warp_Timming_e eTiming)
{
    switch(eTiming)
    {
        case E_WARP_320_240_NV12:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_320x240_nv12/bb_320x240_nv12.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_320x240_nv12/mi_320x240_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        case E_WARP_640_480_NV12:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_640x480_nv12/bb_640x480_nv12.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_640x480_nv12/mi_640x480_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        case E_WARP_1280_720_NV12:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_1280x720_nv12/bb_1280x720_nv12.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_1280x720_nv12/mi_1280x720_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        case E_WARP_1920_1080_NV12:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_1920x1080_nv12/bb_1920x1080_nv12.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_1920x1080_nv12/mi_1920x1080_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        case E_WARP_2560_1440_NV12:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_2560x1440_nv12/bb_2560x1440_nv12.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_2560x1440_nv12/mi_2560x1440_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        case E_WARP_3840_2160_NV12:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_3840x2160_nv12/bb_3840x2160_nv12.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_3840x2160_nv12/mi_3840x2160_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        case E_WARP_320_240_NV16:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_320x240_nv16/bb_320x240_nv16.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_320x240_nv16/mi_320x240_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        case E_WARP_640_480_NV16:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_640x480_nv16/bb_640x480_nv16.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_640x480_nv16/mi_640x480_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        case E_WARP_1280_720_NV16:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_1280x720_nv16/bb_1280x720_nv16.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_1280x720_nv16/mi_1280x720_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        case E_WARP_1920_1080_NV16:
        {
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_BOUND_BOX_TABLE, "warp/disp_abs_1920x1080_nv16/bb_1920x1080_nv16.bin"), MI_WARP_OK);
            ExecFunc(MI_WARP_SetChnBin(0, 0, MI_WARP_DISP_ABSOLUTE_TABLE, "warp/disp_abs_1920x1080_nv16/mi_1920x1080_abs_disp.bin"), MI_WARP_OK);
        }
        break;
        default:
            printf("error warp timming!\n");
            break;
    }

    return MI_SUCCESS;
}

MI_BOOL _gbWarpThreadExit = FALSE;
MI_BOOL _gbWarpSwitchTiming = FALSE;

int main(int argc, char **argv)
{
    char cmd = 0;
    ST_Warp_Timming_e eTiming = E_WARP_320_240_NV12;

    St_BaseModuleInit();

    St_WarpFlowInit(eTiming);

    printf("Type \"q\" to exit, \"t\" to change timing\n");

    while(1)
    {

        cmd = getchar();
        if (cmd == 'q')
        {
            break;
        }

        if (cmd == 't')
        {
            printf("Type \"q\" to exit, \"t\" to change timing\n");
            eTiming++;

            if (eTiming > E_WARP_1920_1080_NV16)
                eTiming = E_WARP_320_240_NV12;

            if (eTiming == E_WARP_2560_1440_NV12 || eTiming == E_WARP_3840_2160_NV12)
                eTiming = E_WARP_320_240_NV16;

            St_WarpFlowDeinit();
            St_WarpFlowInit(eTiming);
        }
    }

    St_WarpFlowDeinit();
    St_BaseModuleDeinit();

    return 0;
}


