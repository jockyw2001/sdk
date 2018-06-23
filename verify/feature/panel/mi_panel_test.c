#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>

#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>

#include "mi_sys.h"
#include "mi_common_datatype.h"
#include "mi_disp.h"
#include "mi_panel.h"

extern MI_PANEL_ParamConfig_t astPanel_InnovLux1280x800[];
extern MI_PANEL_ParamConfig_t astPanel_720x1280_60[];

extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB888;
extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB565;
extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB666;
extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Sync_Pulse_Loosely_RGB666;

extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Sync_Event_RGB888;
extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Sync_Event_RGB565;
extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Sync_Event_RGB666;
extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Sync_Event_Loosely_RGB666;

extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Burst_RGB888;
extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Burst_RGB565;
extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Burst_RGB666;
extern MI_PANEL_MipiDsiConfig_t stPanel_RM68200_720x1280_4Lane_Burst_Loosely_RGB666;

extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Sync_Pulse_RGB888;
extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Sync_Pulse_RGB565;
extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Sync_Pulse_RGB666;
extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Sync_Pulse_Loosely_RGB666;

extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Sync_Event_RGB888;
extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Sync_Event_RGB565;
extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Sync_Event_RGB666;
extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Sync_Event_Loosely_RGB666;

extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Burst_RGB888;
extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Burst_RGB565;
extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Burst_RGB666;
extern MI_PANEL_MipiDsiConfig_t stPanel_HX8394_720x1280_2Lane_Burst_Loosely_RGB666;

void panel_test_001(void);

typedef void (*PANEL_TEST_FUNC)(void);
typedef struct stPanelTestFunc_s
{
    MI_U8 u8TestFuncIdx;
    const char *desc;
    PANEL_TEST_FUNC test_func;
}stPanelTestFunc_t;

stPanelTestFunc_t astPanelTestFunc[] = {
    {
        .u8TestFuncIdx = 0,
        .desc = "panel_1280x800",
        .test_func = panel_test_001,
    },
};

typedef struct stPanelTestSubParam_s
{
    const char *SubParamDesc;
    MI_DISP_OutputTiming_e edev0Timing;
    MI_U16 u16LayerWidth;
    MI_U16 u16LayerHeight;
    MI_U16 u16LayerDisplayWidth;
    MI_U16 u16LayerDisplayHeight;
    MI_U16 u16InputPortWidth;
    MI_U16 u16InputPortHeight;
}stPanelTestSubParam_t;

typedef struct stPanelTestParam_s
{
    MI_U8 u8TestParamIdx;
    const char *desc;
    const char *filePath;
    MI_DISP_OutputTiming_e edev0Timing;
    MI_PANEL_LinkType_e eLinkType;
    MI_U16 u16LayerWidth;
    MI_U16 u16LayerHeight;
    MI_U16 u16LayerDisplayWidth;
    MI_U16 u16LayerDisplayHeight;
    MI_U16 u16InputPortWidth;
    MI_U16 u16InputPortHeight;
    MI_SYS_PixelFormat_e ePixelFormat;
    MI_U8 u8TestSubParamNum;
    stPanelTestSubParam_t astPanelTestSubParam[10];
}stPanelTestParam_t;

static stPanelTestParam_t astPanelTestParam[] = {
    {
        .u8TestParamIdx = 0,
        .desc = "panel_test_001 LVDS 1280x800",
        .filePath = "/mnt/YUV422_YUYV1280_720.yuv",
        .edev0Timing = E_MI_DISP_OUTPUT_USER,
        .eLinkType = E_MI_PNL_LINK_LVDS,
        .u16LayerWidth = 1280,
        .u16LayerHeight = 720,
        .u16LayerDisplayWidth = 1280,
        .u16LayerDisplayHeight = 800,
        .u16InputPortWidth = 1280,
        .u16InputPortHeight = 720,
        .ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV,
        .u8TestSubParamNum = 3,
        .astPanelTestSubParam =
        {
            {
                .SubParamDesc = "pause",
            },
            {
                .SubParamDesc = "resume",
            },
            {
                .SubParamDesc = "exit",
            },
        },
    },
    {
        .u8TestParamIdx = 1,
        .desc = "panel_test_001 MIPI 1280x720",
        .filePath = "/mnt/YUV422_YUYV1280_720.yuv",
        .edev0Timing = E_MI_DISP_OUTPUT_USER,
        .eLinkType = E_MI_PNL_LINK_MIPI_DSI,
        .u16LayerWidth = 1280,
        .u16LayerHeight = 720,
        .u16LayerDisplayWidth = 1280,
        .u16LayerDisplayHeight = 720,
        .u16InputPortWidth = 1280,
        .u16InputPortHeight = 720,
        .ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV,
        .u8TestSubParamNum = 3,
        .astPanelTestSubParam =
        {
            {
                .SubParamDesc = "pause",
            },
            {
                .SubParamDesc = "resume",
            },
            {
                .SubParamDesc = "exit",
            },
        },
    },
};


typedef struct stPanelTestPutBuffThreadArgs_s
{
    pthread_t pt;
    pthread_t ptsnap;
    const char *FileName;
    MI_BOOL bRunFlag;
    MI_U16 u16BuffWidth;
    MI_U16 u16BuffHeight;
    MI_SYS_PixelFormat_e ePixelFormat;
    MI_SYS_ChnPort_t *pstSysChnPort;
}stPanelTestPutBuffThreadArgs_t;

static MI_BOOL g_PushDataExit = FALSE;
static MI_BOOL g_PushDataStop = FALSE;
stPanelTestPutBuffThreadArgs_t _gstPanelTestPutBuffThreadArgs;

static int Get_Panel_TestNum(void)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    printf("please input test num\n");
    fgets(buffer, sizeof(buffer), stdin);
    return atoi(buffer);
}


MI_BOOL panel_test_OpenSourceFile(const char *pFileName, int *pSrcFd)
{
    int src_fd = open(pFileName, O_RDONLY);
    if (src_fd < 0)
    {
        printf("src_file: %s.\n", pFileName);

        perror("open");
        return -1;
    }
    *pSrcFd = src_fd;

    return TRUE;
}

MI_S32 panel_test_GetOneFrame(int srcFd, int offset, char *pData, int yuvSize)
{
    if (read(srcFd, pData, yuvSize) < yuvSize)
    {
        lseek(srcFd, 0, SEEK_SET);
        if (read(srcFd, pData, yuvSize) < yuvSize)
        {
            printf(" [%s %d] read file error.\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
    }
    return TRUE;
}

static void *panel_test_PutBuffer (void *pData)
{
    int srcFd = 0;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BufConf_t stBufConf;
    MI_U32 u32BuffSize;
    MI_SYS_BUF_HANDLE hHandle;
    struct timeval stTv;
    struct timeval stGetBuffer, stReadFile, stFlushData, stPutBuffer, stRelease;
    time_t stTime = 0;
    stPanelTestPutBuffThreadArgs_t *pstPanelTestPutBuffThreadArgs = (stPanelTestPutBuffThreadArgs_t *)pData;
    MI_SYS_ChnPort_t *pstSysChnPort = pstPanelTestPutBuffThreadArgs->pstSysChnPort;
    const char *filePath =  pstPanelTestPutBuffThreadArgs->FileName;

    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));
    memset(&stTv, 0, sizeof(stTv));
    printf("----------------------start----------------------\n");

    if (TRUE == panel_test_OpenSourceFile(filePath, &srcFd))
    {
        while(!g_PushDataExit)
        {
            if(g_PushDataStop == FALSE)
            {
                gettimeofday(&stTv, NULL);
                stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
                stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
                stBufConf.stFrameCfg.u16Width = pstPanelTestPutBuffThreadArgs->u16BuffWidth;
                stBufConf.stFrameCfg.u16Height = pstPanelTestPutBuffThreadArgs->u16BuffHeight;
                stBufConf.stFrameCfg.eFormat = pstPanelTestPutBuffThreadArgs->ePixelFormat;
                stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
                gettimeofday(&stGetBuffer, NULL);

                if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(pstSysChnPort,&stBufConf,&stBufInfo,&hHandle, -1))
                {
                    stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                    stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                    stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
                    stBufInfo.bEndOfStream = FALSE;

                    u32BuffSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                    gettimeofday(&stReadFile, NULL);

                    if(panel_test_GetOneFrame(srcFd, 0, stBufInfo.stFrameData.pVirAddr[0], u32BuffSize) == TRUE)
                    {
                        gettimeofday(&stFlushData, NULL);
                        gettimeofday(&stPutBuffer, NULL);

                        MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                        gettimeofday(&stRelease, NULL);
                    }
                    else
                    {
                        printf("disp_test_001 getframe failed\n");
                        MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , TRUE);
                    }

               }
               else
               {
                    printf("disp_test_001 sys get buf fail\n");
               }
               usleep(200*1000);
            }

        }
        close(srcFd);
    }
    else
    {
        printf(" open file fail. \n");
    }
    printf("----------------------end----------------------\n");

    return MI_DISP_SUCCESS;
}

void panel_test_001(void)
{
    MI_U8 n = 0;
    pthread_t thread;
    MI_U8 u8TestParamIdx = 0;
    MI_U8 u8TestSubParamIdx = 0;
    MI_DISP_DEV DispDev0 = 0;
    MI_DISP_LAYER DispLayer0 = 0;
    MI_DISP_INPUTPORT InputPort = 0;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_PANEL_LinkType_e eLinkType;
    MI_PANEL_PowerConfig_t stPowerCfg;
    MI_PANEL_BackLightConfig_t stBackLightCfg;
    MI_PANEL_MipiDsiConfig_t stMipiDsiCfg;
    MI_PANEL_ParamConfig_t stParamCfg;
    MI_SYS_PixelFormat_e ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_SYS_ChnPort_t stSysChnPort;

    for(n = 0; n < (sizeof(astPanelTestParam)/sizeof(stPanelTestParam_t)); n++)
    {
        printf("[%d]%s\n",n,astPanelTestParam[n].desc);
    }
    u8TestParamIdx = Get_Panel_TestNum();

    stSysChnPort.eModId = E_MI_MODULE_ID_PANEL;
    stSysChnPort.u32DevId = DispDev0;
    stSysChnPort.u32ChnId = 0;
    stSysChnPort.u32PortId = 0;

    //set disp0 pub
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = astPanelTestParam[u8TestParamIdx].edev0Timing;
    stPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stPubAttr.stSyncInfo.u16Vact = 0;
    stPubAttr.stSyncInfo.u16Vbb = 0;
    stPubAttr.stSyncInfo.u16Vfb = 0;
    stPubAttr.stSyncInfo.u16Hact = 0;
    stPubAttr.stSyncInfo.u16Hbb = 0;
    stPubAttr.stSyncInfo.u16Hfb = 0;
    stPubAttr.stSyncInfo.u16Bvact = 0;
    stPubAttr.stSyncInfo.u16Bvbb = 0;
    stPubAttr.stSyncInfo.u16Bvfb = 0;
    stPubAttr.stSyncInfo.u16Hpw = 0;
    stPubAttr.stSyncInfo.u16Vpw = 0;
    stPubAttr.stSyncInfo.u32FrameRate = 0;
    MI_DISP_SetPubAttr(DispDev0,  &stPubAttr);
    MI_DISP_Enable(DispDev0);

    //set layer0
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    stLayerAttr.stVidLayerSize.u16Width = astPanelTestParam[u8TestParamIdx].u16LayerWidth;
    stLayerAttr.stVidLayerSize.u16Height= astPanelTestParam[u8TestParamIdx].u16LayerHeight;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = astPanelTestParam[u8TestParamIdx].u16LayerDisplayWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = astPanelTestParam[u8TestParamIdx].u16LayerDisplayHeight;
    stLayerAttr.ePixFormat = ePixelFormat;
    MI_DISP_BindVideoLayer(DispLayer0,DispDev0);
    MI_DISP_SetVideoLayerAttr(DispLayer0, &stLayerAttr);
    MI_DISP_GetVideoLayerAttr(DispLayer0, &stLayerAttr);
    MI_DISP_EnableVideoLayer(DispLayer0);

    //set layer0 InputPort
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    MI_DISP_GetInputPortAttr(DispLayer0, InputPort, &stInputPortAttr);
    stInputPortAttr.stDispWin.u16Width = astPanelTestParam[u8TestParamIdx].u16InputPortWidth;
    stInputPortAttr.stDispWin.u16Height = astPanelTestParam[u8TestParamIdx].u16InputPortHeight;
    MI_DISP_SetInputPortAttr(DispLayer0, InputPort, &stInputPortAttr);
    MI_DISP_EnableInputPort(DispLayer0, InputPort);

    //set panel config
    eLinkType = astPanelTestParam[u8TestParamIdx].eLinkType;
    MI_PANEL_Init(eLinkType);

    stPowerCfg.bEn = TRUE;
    MI_PANEL_SetPowerOn(&stPowerCfg);

    stBackLightCfg.bEn = TRUE;
    stBackLightCfg.u16Level = 100;
    MI_PANEL_SetBackLight(&stBackLightCfg);
    MI_PANEL_SetBackLightLevel(&stBackLightCfg);

    if(eLinkType == E_MI_PNL_LINK_LVDS)
    {
        memcpy(&stParamCfg, astPanel_InnovLux1280x800, sizeof(MI_PANEL_ParamConfig_t));
        MI_PANEL_SetPanelParam(&stParamCfg);
    }
    else if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
    {
        memcpy(&stParamCfg, &astPanel_720x1280_60[0], sizeof(MI_PANEL_ParamConfig_t));
        memcpy(&stMipiDsiCfg, &stPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB888, sizeof(MI_PANEL_MipiDsiConfig_t));
        MI_PANEL_SetMipiDsiConfig(&stMipiDsiCfg);
    }
    else
    {
        printf("[%s][%d] eLinkType is invalid\n",__FUNCTION__,__LINE__);
    }
    //MI_PANEL_SetSscConfig(MI_PANEL_SscConfig_t *pstSscCfg);
    //MI_PANEL_SetTimingConfig(MI_PANEL_TimingConfig_t *pstTimingCfg);
    //MI_PANEL_SetDrvCurrentConfig(MI_PANEL_DrvCurrentConfig_t *pstDrvCurrentCfg);
    //MI_PANEL_SetOutputPattern(MI_PANEL_TestPatternConfig_t * pstTestPatternCfg);

    //creat send frame thread
    _gstPanelTestPutBuffThreadArgs.FileName = astPanelTestParam[u8TestParamIdx].filePath;
    _gstPanelTestPutBuffThreadArgs.bRunFlag = TRUE;
    _gstPanelTestPutBuffThreadArgs.u16BuffWidth = astPanelTestParam[u8TestParamIdx].u16InputPortWidth;
    _gstPanelTestPutBuffThreadArgs.u16BuffHeight = astPanelTestParam[u8TestParamIdx].u16InputPortHeight;
    _gstPanelTestPutBuffThreadArgs.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    _gstPanelTestPutBuffThreadArgs.pstSysChnPort = &stSysChnPort;
    pthread_create(&_gstPanelTestPutBuffThreadArgs.pt, NULL, panel_test_PutBuffer, &_gstPanelTestPutBuffThreadArgs);

    while(1)
    {
        for(n = 0; n < astPanelTestParam[u8TestParamIdx].u8TestSubParamNum; n++)
        {
            printf("[%d]%s\n",n,astPanelTestParam[u8TestParamIdx].astPanelTestSubParam[n].SubParamDesc);
        }
        u8TestSubParamIdx = Get_Panel_TestNum();
        if(0 == strncmp(astPanelTestParam[u8TestParamIdx].astPanelTestSubParam[u8TestSubParamIdx].SubParamDesc,"pause",5))
        {
            g_PushDataStop = TRUE;
        }
        if(0 == strncmp(astPanelTestParam[u8TestParamIdx].astPanelTestSubParam[u8TestSubParamIdx].SubParamDesc,"resume",6))
        {
            g_PushDataStop = FALSE;
        }
        if(0 == strncmp(astPanelTestParam[u8TestParamIdx].astPanelTestSubParam[u8TestSubParamIdx].SubParamDesc,"exit",4))
        {
            g_PushDataExit = TRUE;
            pthread_join(_gstPanelTestPutBuffThreadArgs.pt,NULL);
            _gstPanelTestPutBuffThreadArgs.pt = 0;

            MI_DISP_DisableInputPort(DispLayer0,InputPort);
            MI_DISP_DisableVideoLayer(DispLayer0);
            MI_DISP_UnBindVideoLayer(DispLayer0,DispDev0);
            MI_DISP_Disable(DispDev0);
            break;
        }
    }
}

int main(int argc, char **argv)
{
    MI_U8 u8TestFuncIdx;
    MI_U8 n = 0;

    for(n = 0; n < (sizeof(astPanelTestFunc)/sizeof(stPanelTestFunc_t)); n++)
    {
        printf("[%d] %s\n",n,astPanelTestFunc[n].desc);
    }
    u8TestFuncIdx = Get_Panel_TestNum();

    printf("start test func_%d\n",u8TestFuncIdx);
    astPanelTestFunc[u8TestFuncIdx].test_func();

    printf("--------EXIT--------\n");
    return 0;
}

