//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_hdmi.h"
#include "../mi_disp_test.h"

///#define MI_PRINT(fmt, args...)
#define MI_PRINT printf
#define DBG_INFO(fmt, args...)      ({do{printf("[MI INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})

#define ExecFunc(_func_, _ret_) \
    if (_func_ != _ret_)\
    {\
        MI_PRINT("[%d]exec function failed\n", __LINE__);\
        return 1;\
    }\
    else\
    {\
        MI_PRINT("(%d)exec function pass\n", __LINE__);\
    }

#define MIDISPCHECKRESULT(result)\
    if(result != MI_SUCCESS)\
    {\
        MI_PRINT("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }\
    else\
    {\
        MI_PRINT("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

typedef struct {
    char FilePath[256];
    MI_BOOL *pbStop;
    MI_BOOL bPause;
    MI_U16 u16FileWidth;
    MI_U16 u16FileHeight;
    MI_U32 u32Dev;
    MI_U32 u32Channel;
    MI_U32 u32InputPort;
    MI_U32 u32OutputPort;
    MI_SYS_WindowRect_t stCropWin;
    MI_SYS_WindowRect_t stDispWin;
    pthread_t thread;
    pthread_attr_t attr;
} test_disp_OverLapPara_t;

typedef void * (*test_func)(void *);
typedef struct {
    const char *desc;
    test_func  func;
    int        next_index;
    MI_BOOL    bEnd;
} test_disp_TestZoomMenu_t;

typedef enum {
    // Main menu
    E_DISP_TEST_DISABLE_LAYER0,
    E_DISP_TEST_ENABLE_LAYER0,

    E_DISP_TEST_OVERLAP_MAX,
}E_DISP_TEST_OVERLAP;

static void * test_disp_DisableLayer0(void *pData);
static void * test_disp_EnableLayer0(void *pData);


static test_disp_TestZoomMenu_t _gstDispTestZoomMenu[] = {
    // Main menu
    [E_DISP_TEST_DISABLE_LAYER0] = {"Disp disable layer0", test_disp_DisableLayer0, E_DISP_TEST_DISABLE_LAYER0, FALSE},
    [E_DISP_TEST_ENABLE_LAYER0] = {"Disp enable layer0", test_disp_EnableLayer0, E_DISP_TEST_DISABLE_LAYER0, TRUE},
};

static int getChoice(int *pData)
{
    char buffer[256];
    char *p = NULL;
    int err = 0;
    memset(buffer, 0, sizeof(buffer));
    while(1)
    {
        p = fgets(buffer, sizeof(buffer) - 1, stdin);
        if ((strlen(p) == 0) || (strcmp(p, "\n") == 0))
        {
            if (err++ > 3)
            {
                return 0;
            }
        }
        else
        {
            *pData = atoi(buffer);
            return 1;
        }
    }
}
static void * test_disp_DisableLayer0(void *pData)
{
    //MI_DISP_DisableVideoLayer(0);
    //ExecFunc(MI_DISP_UnBindVideoLayer(0, 0), MI_DISP_SUCCESS);
    return 0;
}

static void * test_disp_EnableLayer0(void *pData)
{
    return 0;
}

static int showMenu(int index)
{
    int i = 0;
    test_disp_TestZoomMenu_t *pstMenu = NULL;
    printf("\n");
    do
    {
        pstMenu = &_gstDispTestZoomMenu[index + i];
        printf("[%d]: %s.\n", i, pstMenu->desc);
        i++;
        if (pstMenu->bEnd == TRUE)
        {
            break;
        }
    }while(1);
    printf("\n[%d]: %s.\n", i, "Quit");
    printf("Enter you Choice: \n");
    return i;
}
#define PORTNUM 16

static void * test_disp_FileInVpeToDisp (void *pData)
{
    test_disp_OverLapPara_t *pstPara = (test_disp_OverLapPara_t *)pData;
    int srcFd = 0;
    MI_S32 s32Ret = 0;
    MI_U16 u16Width = 0, u16Height = 0;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_VPE_CHANNEL VpeChannel = 0;
    MI_VPE_PORT VpePort = 3;
    MI_SYS_WindowRect_t stCropWin;
    MI_SYS_ChnPort_t stVpeChnInputPort0;
    MI_SYS_ChnPort_t stVpeChnOutputPort0;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
    struct timeval stTv;
    int y_size = 0;
    int uv_size = 0;
    MI_U8 u8PortId =0;

    memset(&stCropWin, 0, sizeof(stCropWin));
    stCropWin.u16Width  = pstPara->u16FileWidth;
    stCropWin.u16Height = pstPara->u16FileHeight;

    test_disp_CreatVpeChannel(VpeChannel, VpePort, &stCropWin, &pstPara->stDispWin);
    // set vpe port buffer depth
    stVpeChnInputPort0.eModId = E_MI_MODULE_ID_VPE;
    stVpeChnInputPort0.u32DevId = 0;
    stVpeChnInputPort0.u32ChnId = VpeChannel;
    stVpeChnInputPort0.u32PortId = 0;

    stVpeChnOutputPort0.eModId = E_MI_MODULE_ID_VPE;
    stVpeChnOutputPort0.u32DevId = 0;
    stVpeChnOutputPort0.u32ChnId =  VpeChannel;
    stVpeChnOutputPort0.u32PortId = VpePort;
    MI_SYS_SetChnOutputPortDepth(&stVpeChnOutputPort0, 0, 3);

    u16Width = pstPara->u16FileWidth;
    u16Height = pstPara->u16FileHeight;
    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));
#if 0
    test_disp_vpeBinderDisp(VpePort, pstPara->u32InputPort);
#endif

    // Binder VPE Ch0 port0 to Display Layer0 input port 0
    for(u8PortId=0; u8PortId <PORTNUM; u8PortId++)
    {
        stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
        stSrcChnPort.u32DevId = 0;
        stSrcChnPort.u32ChnId = 0;
        stSrcChnPort.u32PortId = VpePort;

        stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
        stDstChnPort.u32DevId = 0;
        stDstChnPort.u32ChnId = 0;
        stDstChnPort.u32PortId = u8PortId;

        u32SrcFrmrate = 30;
        u32DstFrmrate = 30;
        MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate);
    }

    // Binder VPE Ch0 port0 to Display Layer1 input port 0
    for(u8PortId=0; u8PortId <PORTNUM; u8PortId++)
    {
        stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
        stDstChnPort.u32DevId = 0;
        stDstChnPort.u32ChnId = 1;
        stDstChnPort.u32PortId = u8PortId;
        MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate);
    }

    if (TRUE == test_disp_OpenSourceFile(pstPara->FilePath, &srcFd))
    {
        do
        {
            memset(&stBufConf , 0, sizeof(stBufConf));
            DISP_TEST_DBG("%s()@line: Start get input buffer.\n", __func__, __LINE__);
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            gettimeofday(&stTv, NULL);
            stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
            stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
            stBufConf.stFrameCfg.u16Width = u16Width;
            stBufConf.stFrameCfg.u16Height = u16Height;
            if(MI_SUCCESS  == MI_SYS_ChnInputPortGetBuf(&stVpeChnInputPort0, &stBufConf, &stBufInfo, &hHandle , 0))
            {
               // Start user put int buffer
                y_size  = u16Width*u16Height;
                uv_size  = y_size/2;
                test_disp_ShowFrameInfo("Input : ", &stBufInfo.stFrameData);

                DISP_TEST_INFO("%d X %d.\n", u16Width, u16Height);
                if(1 == test_disp_GetOneFrameYUV420ByStride(srcFd, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.pVirAddr[1], y_size, uv_size, u16Height, u16Width, stBufInfo.stFrameData.u32Stride[0], stBufInfo.stFrameData.u32Stride[1]))
                {
                    DISP_TEST_INFO("%s()@line%d: Start put input buffer.\n", __func__, __LINE__);
                    s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle,&stBufInfo, FALSE);
                }
                else
                {
                    DISP_TEST_INFO("%s()@line%d: Drop buffer.\n", __func__, __LINE__);
                    s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle,&stBufInfo, TRUE);
                    test_disp_FdRewind(srcFd);
                }
            }
            else
            {
                DISP_TEST_INFO("get buf fail\n");
            }
            usleep(10 * 1000);
        }while(*(pstPara->pbStop) == FALSE);
        test_disp_CloseFd(srcFd);
    }

    // Unbinder Layer0 input port0
    for(u8PortId=0; u8PortId <PORTNUM; u8PortId++)
    {
        stDstChnPort.u32ChnId  = 0;
        stDstChnPort.u32PortId = u8PortId;
        MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort);
    }

    // Unbinder Layer1 input port0
    for(u8PortId=0; u8PortId <PORTNUM; u8PortId++)
    {
        stDstChnPort.u32ChnId  = 1;
        stDstChnPort.u32PortId = u8PortId;
        MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort);
    }
    #if 0
    test_disp_vpeUnBinderDisp(VpePort, pstPara->u32InputPort);
    #endif
    test_Disp_DestroyVpeChannel(VpeChannel, VpePort);
    if (s32Ret == MI_SUCCESS)
    {
        return pstPara;
    }
    else
    {
        return NULL;
    }
}

static test_disp_OverLapPara_t _gstDispOverLap;
static void * test_disp_OverLapVpeInCreate(void *pData)
{
    char buff[256];
    test_disp_OverLapPara_t *pstPara = (test_disp_OverLapPara_t *)pData;
    memset(buff, 0, sizeof(buff));
    pthread_attr_init(&pstPara->attr);
    pstPara->bPause = FALSE;
#if 0
    printf("\nPlease enter YUV 420 file Path. (File spec: width: %d, height: %d)\n", pstPara->u16FileWidth, pstPara->u16FileHeight);
    do {
        //fgets(buff, sizeof(buff -1), stdin);
        scanf("%s", buff);
    } while ((strlen(buff) == 0) || (strcmp(buff, "\n") == 0));
    strcpy(pstPara->FilePath, buff);
#endif
    pthread_create(&pstPara->thread, &pstPara->attr, test_disp_FileInVpeToDisp, pstPara);
    return NULL;
}

static void * test_disp_OverLapVpeInStop(void *pData)
{
    test_disp_OverLapPara_t *pstPara = (test_disp_OverLapPara_t *)pData;
    int s = 0;
    *pstPara->pbStop = TRUE;
    pthread_join(pstPara->thread, NULL);
    s = pthread_attr_destroy(&pstPara->attr);
    if (s != 0)
        perror("pthread_attr_destroy");
    return pData;
}

int main(int argc, const char *argv[])
{
    MI_DISP_DEV DispDev = 0;
    MI_DISP_LAYER DispLayer = 0;
    MI_DISP_INPUTPORT LayerInputPort = 0;

    MI_DISP_InputPortAttr_t stInputPortAttr0[16];
    MI_DISP_InputPortAttr_t stInputPortAttr1[16];
    MI_DISP_PubAttr_t stPubAttr;
    static MI_BOOL _gbStop = FALSE;
    MI_U16 u16Width = 0;
    MI_U16 u16Height = 0;
    MI_U16 u16LayerWidth=0;
    MI_U16 u16LayerHeight =0;
    MI_U16 u16PortWidth =0;
    MI_U16 u16PortHeight =0;
    MI_U8 u8PortId =0;
    MI_U16 u16Avg =4;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    int max, opt, index;
    const char *pPath = NULL;
    index = 0;
    test_disp_TestZoomMenu_t *pstMenu = NULL;

    if(argc < 4)
    {
        printf("%s:\n%s <yuv420_file> <width> <heigt> \n", DISP_TEST_011_DESC, argv[0]);
        return 0;
    }
    pPath    = argv[1];
    u16Width = atoi(argv[2]);
    u16Height = atoi(argv[3]);

    u16LayerWidth = u16Width;
    u16LayerHeight = u16Height;
    u16PortWidth = u16LayerWidth/4/2*2;
    u16PortHeight = u16LayerHeight/8/2*2;

    printf("Layer width = %d, height = %d\n", u16Width, u16Height);

    MIDISPCHECKRESULT(MI_SYS_Init());
    // Test Device
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

    mi_disp_hdmiInit();
    u16Avg = 4;

#if 1
    // Layer 0
    DispDev = 0;
    DispLayer = 0;
    LayerInputPort = 0;
    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    stLayerAttr.stVidLayerSize.u16Width = u16LayerWidth;
    stLayerAttr.stVidLayerSize.u16Height= u16LayerHeight;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = u16LayerWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = u16LayerHeight;
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DISP_TEST_INFO("[%s %d]Get Video Layer Size [%d, %d] !!!\n", __FUNCTION__, __LINE__, stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DISP_TEST_INFO("[%s %d]Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", __FUNCTION__, __LINE__,\
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    // Layer0 input port0
    memset(&stInputPortAttr0, 0, sizeof(stInputPortAttr0));
    for(u8PortId =0; u8PortId< PORTNUM; u8PortId++)
    {
        ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, u8PortId, &stInputPortAttr0[u8PortId]), MI_DISP_SUCCESS);
        stInputPortAttr0[u8PortId].stDispWin.u16X    = (u8PortId%u16Avg)* u16PortWidth;
        stInputPortAttr0[u8PortId].stDispWin.u16Y    = (u8PortId/u16Avg) * u16PortHeight;
        stInputPortAttr0[u8PortId].stDispWin.u16Width = u16PortWidth;
        stInputPortAttr0[u8PortId].stDispWin.u16Height= u16PortHeight;
        ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, u8PortId, &stInputPortAttr0[u8PortId]), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_EnableInputPort(DispLayer, u8PortId), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_SetInputPortSyncMode(DispLayer, u8PortId, E_MI_DISP_SYNC_MODE_FREE_RUN) , MI_DISP_SUCCESS);
    }
#endif

    // Layer1
    DispDev = 0;
    DispLayer = 1;
    LayerInputPort = 0;

    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    stLayerAttr.stVidLayerSize.u16Width = u16LayerWidth;
    stLayerAttr.stVidLayerSize.u16Height= u16LayerHeight;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = u16LayerWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = u16LayerHeight;

    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DISP_TEST_INFO("[%s %d]Get Video Layer Size [%d, %d] !!!\n", __FUNCTION__, __LINE__, stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DISP_TEST_INFO("[%s %d]Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", __FUNCTION__, __LINE__,\
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    memset(&stInputPortAttr1, 0, sizeof(stInputPortAttr1));
    for(u8PortId =0; u8PortId< PORTNUM; u8PortId++)
    {
        ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, u8PortId, &stInputPortAttr1[u8PortId]), MI_DISP_SUCCESS);

        stInputPortAttr1[u8PortId].stDispWin.u16X  = (u8PortId%u16Avg)* u16PortWidth;
        stInputPortAttr1[u8PortId].stDispWin.u16Y  = (u8PortId/u16Avg) * u16PortHeight + 540;
        stInputPortAttr1[u8PortId].stDispWin.u16Width    = u16PortWidth;
        stInputPortAttr1[u8PortId].stDispWin.u16Height = u16PortHeight;
        ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, u8PortId, &stInputPortAttr1[u8PortId]), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_EnableInputPort(DispLayer, u8PortId), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_SetInputPortSyncMode(DispLayer, u8PortId, E_MI_DISP_SYNC_MODE_FREE_RUN), MI_DISP_SUCCESS);
    }

    memset(&_gstDispOverLap, 0, sizeof(_gstDispOverLap));
    _gstDispOverLap.pbStop = &_gbStop;
    _gstDispOverLap.u16FileHeight = u16Height;
    _gstDispOverLap.u16FileWidth  = u16Width;
    _gstDispOverLap.u32Dev = DispDev;
    _gstDispOverLap.u32Channel = DispLayer;
    _gstDispOverLap.u32InputPort = LayerInputPort;
    _gstDispOverLap.u32OutputPort = 0;
    _gstDispOverLap.stDispWin.u16X = 0;
    _gstDispOverLap.stDispWin.u16Y = 0;
    _gstDispOverLap.stDispWin.u16Width= u16PortWidth;
    _gstDispOverLap.stDispWin.u16Height= u16PortHeight;
    sprintf(_gstDispOverLap.FilePath, "%s", pPath);

    test_disp_OverLapVpeInCreate(&_gstDispOverLap);
    printf("Welcome: Disp overlay test.\n");
    while(1)
    {
        max = showMenu(index);
        if (0 == getChoice(&opt))
        {
            continue;
        }

        if ((opt < 0) || (opt > max))
        {
            printf("Invalid input option !!.\n");
            continue;
        }
        if (opt == max)
        {
            break;
        }
        pstMenu = &_gstDispTestZoomMenu[index + opt];
        if (pstMenu->func != NULL)
        {
            pstMenu->func(&_gstDispOverLap);
        }
        index = pstMenu->next_index;
    }

    test_disp_OverLapVpeInStop(&_gstDispOverLap);

    DispLayer  = 0;
    for(u8PortId =0; u8PortId< PORTNUM; u8PortId++)
    {
        ExecFunc(MI_DISP_DisableInputPort(DispLayer, u8PortId), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_DisableInputPort(1, u8PortId), MI_DISP_SUCCESS);
    }

    //Disable Layer and Unbind Layer 0,1 must like this
    MI_DISP_DisableVideoLayer(1);
    MI_DISP_UnBindVideoLayer(1, DispDev);

    ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);

    ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    DISP_TEST_INFO("disp test end\n");

    return MI_SUCCESS;
}
