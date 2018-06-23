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
} test_disp_ZoomPara_t;

typedef int(*test_func)(void *);
typedef struct {
    const char *desc;
    test_func  func;
    int        next_index;
    MI_BOOL    bEnd;
} test_disp_TestZoomMenu_t;

typedef enum {
    // Main menu
    E_DISP_TEST_ZOOM_FILE_IN = 0,
    E_DISP_TEST_ZOOM_VPE_IN,
    E_DISP_TEST_ZOOM_DIVP_IN,

    // FILE IN
    E_DISP_TEST_ZOOM_FILE_IN_SET,
    E_DISP_TEST_ZOOM_FILE_IN_SET_RET,

    // VPE IN
    E_DISP_TEST_ZOOM_VPE_IN_SET,
    E_DISP_TEST_ZOOM_VPE_IN_SET_RET,

    // DIVP IN
    E_DISP_TEST_ZOOM_DIVP_IN_SET,
    E_DISP_TEST_ZOOM_DIVP_IN_PAUSE,
    E_DISP_TEST_ZOOM_DIVP_IN_RESUME,
    E_DISP_TEST_ZOOM_DIVP_IN_SET_RET,

    E_DISP_TEST_ZOOM_MAX,
}E_DISP_TEST_ZOOM;
static void * test_disp_SetZoom(void *pData);
static void * test_disp_ZoomVpeInCreate(void *pData);
static void * test_disp_ZoomFileInCreate(void *pData);
static void * test_disp_ZoomDivpInCreate(void *pData);
static void * test_disp_ZoomFileInStop(void *pData);
static void * test_disp_ZoomVpeInStop(void *pData);
static void * test_disp_ZoomDivpInPause(void *pData);
static void * test_disp_ZoomDivpInResume(void *pData);
static void * test_disp_ZoomDivpInStop(void *pData);

static test_disp_TestZoomMenu_t _gstDispTestZoomMenu[] = {
    // Main menu
    [E_DISP_TEST_ZOOM_FILE_IN] = {"Disp zoom test for file in", test_disp_ZoomFileInCreate, E_DISP_TEST_ZOOM_FILE_IN_SET, FALSE},
    [E_DISP_TEST_ZOOM_VPE_IN] = {"Disp zoom test for vpe in", test_disp_ZoomVpeInCreate, E_DISP_TEST_ZOOM_VPE_IN_SET, FALSE},
    [E_DISP_TEST_ZOOM_DIVP_IN] = {"Disp zoom test for divp in", test_disp_ZoomDivpInCreate, E_DISP_TEST_ZOOM_DIVP_IN_SET, TRUE},

    // File in test
    [E_DISP_TEST_ZOOM_FILE_IN_SET] = {"File Zoom set", test_disp_SetZoom, E_DISP_TEST_ZOOM_FILE_IN_SET, FALSE},
    [E_DISP_TEST_ZOOM_FILE_IN_SET_RET] = {"Return", test_disp_ZoomFileInStop, E_DISP_TEST_ZOOM_FILE_IN, TRUE},

    // VPE in test
    [E_DISP_TEST_ZOOM_VPE_IN_SET] = {"VPE in Zoom set", test_disp_SetZoom, E_DISP_TEST_ZOOM_VPE_IN_SET, FALSE},
    [E_DISP_TEST_ZOOM_VPE_IN_SET_RET] = {"Return", test_disp_ZoomVpeInStop, E_DISP_TEST_ZOOM_FILE_IN, TRUE},

    // DIVP in test
    [E_DISP_TEST_ZOOM_DIVP_IN_SET] = {"DIVP in Zoom set", test_disp_SetZoom, E_DISP_TEST_ZOOM_DIVP_IN_SET, FALSE},
    [E_DISP_TEST_ZOOM_DIVP_IN_PAUSE] = {"DIVP in Zoom pause", test_disp_ZoomDivpInPause, E_DISP_TEST_ZOOM_DIVP_IN_SET, FALSE},
    [E_DISP_TEST_ZOOM_DIVP_IN_RESUME] = {"DIVP in Zoom resume", test_disp_ZoomDivpInResume, E_DISP_TEST_ZOOM_DIVP_IN_SET, FALSE},
    [E_DISP_TEST_ZOOM_DIVP_IN_SET_RET] = {"Return", test_disp_ZoomDivpInStop, E_DISP_TEST_ZOOM_FILE_IN, TRUE},

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

static void * test_disp_SetZoom(void *pData)
{
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    MI_DISP_VidWinRect_t stCropWin;
    int x, y, width, height;
    printf("input: x y width height.\n");
    scanf("%d %d %d %d.\n", &x, &y, &width, &height);
    memset(&stCropWin, 0, sizeof(MI_DISP_VidWinRect_t));
    stCropWin.u16X = x;
    stCropWin.u16Y = y;
    stCropWin.u16Width = width;
    stCropWin.u16Height = height;
    MI_DISP_SetZoomInWindow(0, 0, &stCropWin);
    if (pstPara->bPause == TRUE)
    {
        // Call Divp refresh
        printf("MI_DIVP_RefreshChn()\n");
        MI_DIVP_RefreshChn(0);
    }

    return pData;
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

static void * test_disp_FileInVpeToDisp (void *pData)
{
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
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
    struct timeval stTv;
    int y_size = 0;
    int uv_size = 0;

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

    test_disp_vpeBinderDisp(VpePort, pstPara->u32InputPort);
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
            usleep(100 * 1000);
        }while(*(pstPara->pbStop) == FALSE);
        test_disp_CloseFd(srcFd);
    }

    test_disp_vpeUnBinderDisp(VpePort, pstPara->u32InputPort);
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

MI_S32 test_disp_CreateDivpChannel(MI_DIVP_CHN DivpChn, MI_SYS_WindowRect_t *pstCapWin, MI_SYS_WindowRect_t *pstDispWin, MI_DIVP_DiType_e eDiType)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_DIVP_ChnAttr_t stAttr;
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = eDiType;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = pstCapWin->u16X;
    stAttr.stCropRect.u16Y = pstCapWin->u16Y;
    stAttr.stCropRect.u16Width = pstCapWin->u16Width;
    stAttr.stCropRect.u16Height = pstCapWin->u16Height;
    stAttr.u32MaxWidth = pstCapWin->u16Width;
    stAttr.u32MaxHeight = pstCapWin->u16Height;

    s32Ret = MI_DIVP_CreateChn(DivpChn, &stAttr);
    DBG_INFO("\n s32Ret = %d", s32Ret);

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;//E_MI_SYS_PIXEL_FRAME_YUV_MST_420;//E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;//
    stOutputPortAttr.u32Width = pstCapWin->u16Width;
    stOutputPortAttr.u32Height = pstCapWin->u16Height;
    ExecFunc(MI_DIVP_SetOutputPortAttr(DivpChn, &stOutputPortAttr), MI_SUCCESS);
    ExecFunc(MI_DIVP_StartChn(DivpChn), MI_SUCCESS);
    return s32Ret;
}

MI_S32 test_disp_DestroyDivpChannel(MI_DIVP_CHN u32ChnId)
{

    MI_S32 s32Ret = MI_SUCCESS;
    ExecFunc(MI_DIVP_StopChn(u32ChnId), MI_SUCCESS);
    ExecFunc(MI_DIVP_DestroyChn(u32ChnId), MI_SUCCESS);
    return s32Ret;
}

static void * test_disp_FileInDivpToDisp (void *pData)
{
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    int srcFd = 0;
    MI_S32 s32Ret = 0;
    MI_U16 u16Width = 0, u16Height = 0;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    struct timeval stTv;
    int y_size = 0;
    int uv_size = 0;
    MI_DIVP_CHN u32DivpChnId = 0;
    MI_U32 u32DivpOutportPort = 0;
    MI_SYS_WindowRect_t stCapWin;
    MI_SYS_WindowRect_t stDispWin;
    MI_SYS_ChnPort_t stDivpChnInputPort;
    MI_SYS_ChnPort_t stDivpChnOutputPort;

    u16Width = pstPara->u16FileWidth;
    u16Height = pstPara->u16FileHeight;

    memset(&stDispWin, 0, sizeof(stDispWin));
    memset(&stCapWin, 0, sizeof(stCapWin));
    stCapWin.u16X = 0;
    stCapWin.u16Y = 0;
    stCapWin.u16Width= u16Width;
    stCapWin.u16Height= u16Height;
    // create divp channel
    stDispWin.u16Width = pstPara->stDispWin.u16Width;
    stDispWin.u16Height= pstPara->stDispWin.u16Height;
    test_disp_CreateDivpChannel(u32DivpChnId, &stCapWin, &stDispWin, E_MI_DIVP_DI_TYPE_OFF);

    // set divp output port fifo
    memset(&stDivpChnOutputPort, 0, sizeof(stDivpChnOutputPort));
    stDivpChnOutputPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivpChnOutputPort.u32DevId = 0;
    stDivpChnOutputPort.u32ChnId = u32DivpChnId;
    stDivpChnOutputPort.u32PortId = u32DivpOutportPort;
    MI_SYS_SetChnOutputPortDepth(&stDivpChnOutputPort, 0, 3);

    // set divp input port
    memset(&stDivpChnInputPort, 0, sizeof(stDivpChnInputPort));
    stDivpChnInputPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivpChnInputPort.u32DevId = 0;
    stDivpChnInputPort.u32ChnId = u32DivpChnId;
    stDivpChnInputPort.u32PortId = 0;

    test_disp_divpBinderDisp(u32DivpOutportPort, pstPara->u32InputPort);
    if (TRUE == test_disp_OpenSourceFile(pstPara->FilePath, &srcFd))
    {
        do
        {
            if (pstPara->bPause == TRUE)
            {
                sleep(1);
                continue;
            }
            memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
            memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));
            DISP_TEST_DBG("%s()@line: Start get input buffer.\n", __func__, __LINE__);
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            gettimeofday(&stTv, NULL);
            stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
            stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
            stBufConf.stFrameCfg.u16Width = u16Width;
            stBufConf.stFrameCfg.u16Height = u16Height;
            if(MI_SUCCESS  == MI_SYS_ChnInputPortGetBuf(&stDivpChnInputPort, &stBufConf, &stBufInfo, &hHandle , 0))
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
            usleep(100 * 1000);
        }while(*(pstPara->pbStop) == FALSE);
        test_disp_CloseFd(srcFd);
    }

    test_disp_divpUnBinderDisp(u32DivpOutportPort, pstPara->u32InputPort);
    test_disp_DestroyDivpChannel(u32DivpChnId);


    if (s32Ret == MI_SUCCESS)
    {
        return pstPara;
    }
    else
    {
        return NULL;
    }
}

static void * test_disp_FileInDisp(void *pData)
{
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    int srcFd = 0;
    MI_U16 u16Width = 0, u16Height = 0;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BUF_HANDLE hHandle;

    u16Width = pstPara->u16FileWidth;
    u16Height = pstPara->u16FileHeight;
    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));
    memset(&stChnPort , 0 , sizeof(stChnPort));
    stChnPort.eModId = E_MI_MODULE_ID_DISP;
    stChnPort.u32ChnId = pstPara->u32Channel;
    stChnPort.u32DevId = pstPara->u32Dev;
    stChnPort.u32PortId = pstPara->u32InputPort;
    if (TRUE == test_disp_OpenSourceFile(pstPara->FilePath, &srcFd))
    {
        while(*(pstPara->pbStop) == FALSE)
        {
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
            stBufConf.stFrameCfg.u16Width = u16Width;
            stBufConf.stFrameCfg.u16Height = u16Height;


            stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stChnPort, &stBufConf,&stBufInfo,&hHandle, -1))
            {
                stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
                stBufInfo.bEndOfStream = FALSE;
                DISP_TEST_INFO(" [%s %d] virAddr = %p  !!!!!\n", __FUNCTION__, __LINE__,stBufInfo.stFrameData.pVirAddr[0]);
                DISP_TEST_INFO(" [%s %d] u32Stride = %d  \n", __FUNCTION__, __LINE__, stBufInfo.stFrameData.u32Stride[0]);

                if (1 == test_disp_GetOneYuv422Frame(srcFd, 0, stBufInfo.stFrameData.pVirAddr[0], u16Width, u16Height, stBufInfo.stFrameData.u32Stride[0]))
                {
                    MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
                }
                else
                {
                    test_disp_FdRewind(srcFd);
                    MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, TRUE);
                }

            }
            else
            {
                DISP_TEST_INFO("get buf fail\n");
            }
            usleep(100 * 1000);
        }
        test_disp_CloseFd(srcFd);
    }

    return pData;
}

static void * test_disp_ZoomDivpInPause(void *pData)
{
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    pstPara->bPause = TRUE;
    return NULL;
}


static void * test_disp_ZoomDivpInResume(void *pData)
{
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    pstPara->bPause = FALSE;

    return NULL;
}

static test_disp_ZoomPara_t _gstDispZoom;
static void * test_disp_ZoomFileInCreate(void *pData)
{
    char buff[256];
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    memset(buff, 0, sizeof(buff));
    pthread_attr_init(&pstPara->attr);
    pstPara->bPause = FALSE;

    printf("\nPlease enter YUV 422 file Path. (File spec: width: %d, height: %d)\n", pstPara->u16FileWidth, pstPara->u16FileHeight);
    do {
        //fgets(buff, sizeof(buff -1), stdin);
        scanf("%s", buff);
        printf("buff: %s.\n", buff);
    } while ((strlen(buff) == 0) || (strcmp(buff, "\n") == 0));
    strcpy(pstPara->FilePath, buff);

    pthread_create(&pstPara->thread, &pstPara->attr, test_disp_FileInDisp, pstPara);
    return NULL;
}

static void * test_disp_ZoomVpeInCreate(void *pData)
{
    char buff[256];
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    memset(buff, 0, sizeof(buff));
    pthread_attr_init(&pstPara->attr);
    pstPara->bPause = FALSE;
    printf("\nPlease enter YUV 420 file Path. (File spec: width: %d, height: %d)\n", pstPara->u16FileWidth, pstPara->u16FileHeight);
    do {
        //fgets(buff, sizeof(buff -1), stdin);
        scanf("%s", buff);

        printf("buff: %s.\n", buff);
    } while ((strlen(buff) == 0) || (strcmp(buff, "\n") == 0));
    strcpy(pstPara->FilePath, buff);

    pthread_create(&pstPara->thread, &pstPara->attr, test_disp_FileInVpeToDisp, pstPara);
    return NULL;
}

static void * test_disp_ZoomDivpInCreate(void *pData)
{
    char buff[256];
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    memset(buff, 0, sizeof(buff));
    pthread_attr_init(&pstPara->attr);
    pstPara->bPause = FALSE;
    printf("\nPlease enter YUV 420 file Path. (File spec: width: %d, height: %d)\n", pstPara->u16FileWidth, pstPara->u16FileHeight);
    do {
        //fgets(buff, sizeof(buff -1), stdin);
        scanf("%s", buff);

        printf("buff: %s.\n", buff);
    } while ((strlen(buff) == 0) || (strcmp(buff, "\n") == 0));
    strcpy(pstPara->FilePath, buff);

    pthread_create(&pstPara->thread, &pstPara->attr, test_disp_FileInDivpToDisp, pstPara);
    return NULL;
}


static void * test_disp_ZoomFileInStop(void *pData)
{
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    int s = 0;
    *pstPara->pbStop = TRUE;
    pthread_join(pstPara->thread, NULL);
    s = pthread_attr_destroy(&pstPara->attr);
    if (s != 0)
        perror("pthread_attr_destroy");
}

static void * test_disp_ZoomVpeInStop(void *pData)
{
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    int s = 0;
    *pstPara->pbStop = TRUE;
    pthread_join(pstPara->thread, NULL);
    s = pthread_attr_destroy(&pstPara->attr);
    if (s != 0)
        perror("pthread_attr_destroy");
}


static void * test_disp_ZoomDivpInStop(void *pData)
{
    test_disp_ZoomPara_t *pstPara = (test_disp_ZoomPara_t *)pData;
    int s = 0;
    *pstPara->pbStop = TRUE;
    pthread_join(pstPara->thread, NULL);
    s = pthread_attr_destroy(&pstPara->attr);
    if (s != 0)
        perror("pthread_attr_destroy");
}

int main(int argc, const char *argv[])
{
    MI_DISP_DEV DispDev = 0;
    MI_DISP_LAYER DispLayer = 0;
    MI_DISP_INPUTPORT LayerInputPort = 0;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_DISP_PubAttr_t stPubAttr;
    static MI_BOOL _gbStop = FALSE;
    const char* filePath = NULL;
    MI_U16 u16Width = 0;
    MI_U16 u16Height = 0;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    int max, opt, index;
    index = 0;
    test_disp_TestZoomMenu_t *pstMenu = NULL;

    if(argc < 3)
    {
        printf("%s:\n%s <width> <heigt>\n", DISP_TEST_011_DESC, argv[0]);
        return 0;
    }

    u16Width = atoi(argv[1]);
    u16Height = atoi(argv[2]);
    printf("Layer width = %d, height = %d\n", u16Width, u16Height);

    MIDISPCHECKRESULT(MI_SYS_Init());
    // Test Device
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);
    mi_disp_hdmiInit();

    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    stLayerAttr.stVidLayerSize.u16Width = u16Width;
    stLayerAttr.stVidLayerSize.u16Height= u16Height;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = 1920;
    stLayerAttr.stVidLayerDispWin.u16Height = 1080;
    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DISP_TEST_INFO("[%s %d]Get Video Layer Size [%d, %d] !!!\n", __FUNCTION__, __LINE__, stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DISP_TEST_INFO("[%s %d]Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", __FUNCTION__, __LINE__,\
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    stInputPortAttr.stDispWin.u16Width    = u16Width;
    stInputPortAttr.stDispWin.u16Height = u16Height;
    ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_DISP_SUCCESS);


    memset(&_gstDispZoom, 0, sizeof(_gstDispZoom));
    _gstDispZoom.pbStop = &_gbStop;
    _gstDispZoom.u16FileHeight = u16Height;
    _gstDispZoom.u16FileWidth  = u16Width;
    _gstDispZoom.u32Dev = DispDev;
    _gstDispZoom.u32Channel = DispLayer;
    _gstDispZoom.u32InputPort = LayerInputPort;
    _gstDispZoom.u32OutputPort = 0;
    _gstDispZoom.stDispWin.u16X = 0;
    _gstDispZoom.stDispWin.u16Y = 0;
    _gstDispZoom.stDispWin.u16Width= u16Width;
    _gstDispZoom.stDispWin.u16Height= u16Height;

    printf("Welcome: Disp Zoom test.\n");
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
            pstMenu->func(&_gstDispZoom);
        }
        index = pstMenu->next_index;
    }

    ExecFunc(MI_DISP_DisableInputPort(DispLayer, LayerInputPort), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    DISP_TEST_INFO("disp test end\n");

    return MI_SUCCESS;
}
