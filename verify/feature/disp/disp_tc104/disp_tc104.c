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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_hdmi.h"
#include "../mi_disp_test.h"

#define DBG_INFO(fmt, args...)      //({do{printf("\n [MI INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);usleep(10000);})

#define ExecFunc(_func_, _ret_) \
    if (_func_ != _ret_)\
    {\
        DBG_INFO("exec function failed\n");\
        return 1;\
    }\
    else\
    {\
        DBG_INFO("exec function pass\n");\
    }

#define MIDISPCHECKRESULT(result)\
    if(result != MI_SUCCESS)\
    {\
        DBG_INFO("exec function failed\n");\
        return 1;\
    }\
    else\
    {\
        DBG_INFO("exec function pass\n");\
    }


typedef enum
{
    E_DISP_OUTPUT_NULL = 0,
    E_DISP_OUTPUT_PAL,
    E_DISP_OUTPUT_NTSC,
    E_DISP_OUTPUT_720P50,
    E_DISP_OUTPUT_720P60,
    E_DISP_OUTPUT_1080P50,
    E_DISP_OUTPUT_1080P60,
    E_DISP_OUTPUT_1024x768_60,           /* VESA 1024 x 768 at 60 Hz (non-interlaced) */
    E_DISP_OUTPUT_1440x900_60,           /* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */
    E_DISP_OUTPUT_1280x1024_60,          /* VESA 1280 x 1024 at 60 Hz (non-interlaced) */
    E_DISP_OUTPUT_1600x1200_60,          /* VESA 1600 x 1200 at 60 Hz (non-interlaced) */
    E_DISP_OUTPUT_3840x2160_30,          /* 3840x2160_30 */

    E_DISP_OUTPUT_SHOWTIMING,

    E_DISP_OUTPUT_MAX,
} Disp_timing_op_e;

typedef struct test_Disp_Param_s{
    MI_DISP_DEV  DispDev;
    MI_DISP_LAYER DispLayer;
    MI_DISP_OutputTiming_e eDispOutputTiming;
    MI_HDMI_TimingType_e eHdmiTiming;
    MI_U16 opt;
    MI_U16 u16CurrentWinNum;
    MI_U16 u16LayerWidth;
    MI_U16 u16LayerHeight;
    MI_U16 u16LayerDispWidth;
    MI_U16 u16LayerDispHeight;
    MI_U16 u16PortWidth;
    MI_U16 u16PortHeight;
}test_Disp_Param_t;

typedef int(*test_func)(test_Disp_Param_t *);

typedef struct test_Disp_Timing_s{
    const char *desc;
    test_func  func;
    int        next_index;
    MI_BOOL    bEnd;
}test_Disp_Timing_t;

MI_BOOL _gbStop = FALSE;

static MI_BOOL test_Disp_GetPixelParam(test_Disp_Param_t *PDispParam)
{
    MI_DISP_OutputTiming_e eSetMiTiming = E_MI_DISP_OUTPUT_MAX;
    Disp_timing_op_e eUsetSetTiming = (Disp_timing_op_e)(PDispParam->opt);
    MI_U16 u16LayerDispWidth = 0;
    MI_U16 u16LayerDispHeight = 0;
    MI_HDMI_TimingType_e eHdmiTiming = E_MI_HDMI_TIMING_MAX;

    switch(eUsetSetTiming)
    {
        case E_DISP_OUTPUT_PAL:
            {
                 eSetMiTiming = E_MI_DISP_OUTPUT_PAL;
                 u16LayerDispWidth = 720;
                 u16LayerDispHeight = 576;
                 eHdmiTiming = E_MI_HDMI_TIMING_MAX;
            }
             break;

        case E_DISP_OUTPUT_NTSC:
            {
                eSetMiTiming = E_MI_DISP_OUTPUT_NTSC;
                u16LayerDispWidth = 720;
                u16LayerDispHeight = 480;
                eHdmiTiming = E_MI_HDMI_TIMING_MAX;
            }
             break;

        case E_DISP_OUTPUT_720P50:
            {
                 eSetMiTiming = E_MI_DISP_OUTPUT_720P50;
                 u16LayerDispWidth = 1280;
                 u16LayerDispHeight = 720;
                 eHdmiTiming = E_MI_HDMI_TIMING_720_50P;
            }
             break;

        case E_DISP_OUTPUT_720P60:
            {
                eSetMiTiming = E_MI_DISP_OUTPUT_720P60;
                u16LayerDispWidth = 1280;
                u16LayerDispHeight = 720;
                eHdmiTiming = E_MI_HDMI_TIMING_720_60P;
            }
             break;

        case E_DISP_OUTPUT_1080P50:
            {
                eSetMiTiming = E_MI_DISP_OUTPUT_1080P50;
                u16LayerDispWidth = 1920;
                u16LayerDispHeight = 1080;
                eHdmiTiming = E_MI_HDMI_TIMING_1080_50P;
            }
             break;

        case E_DISP_OUTPUT_1080P60:
            {
                eSetMiTiming = E_MI_DISP_OUTPUT_1080P60;
                u16LayerDispWidth = 1920;
                u16LayerDispHeight = 1080;
                eHdmiTiming = E_MI_HDMI_TIMING_1080_60P;
            }
             break;

        case E_DISP_OUTPUT_1024x768_60:
            {
                eSetMiTiming = E_MI_DISP_OUTPUT_1024x768_60;
                u16LayerDispWidth = 1024;
                u16LayerDispHeight = 768;
                eHdmiTiming = E_MI_HDMI_TIMING_1024x768_60P;
            }
             break;

        case E_DISP_OUTPUT_1440x900_60:
            {
                eSetMiTiming = E_MI_DISP_OUTPUT_1440x900_60;
                u16LayerDispWidth = 1440;
                u16LayerDispHeight = 900;
                eHdmiTiming = E_MI_HDMI_TIMING_1440x900_60P;
            }
             break;

        case E_DISP_OUTPUT_1280x1024_60:
            {
                eSetMiTiming = E_MI_DISP_OUTPUT_1280x1024_60;
                u16LayerDispWidth = 1280;
                u16LayerDispHeight = 1024;
                eHdmiTiming = E_MI_HDMI_TIMING_1280x1024_60P;
            }
             break;

        case E_DISP_OUTPUT_1600x1200_60:
            {
                eSetMiTiming = E_MI_DISP_OUTPUT_1600x1200_60;
                u16LayerDispWidth = 1600;
                u16LayerDispHeight = 1200;
                eHdmiTiming = E_MI_HDMI_TIMING_1600x1200_60P;
            }
             break;

        case E_DISP_OUTPUT_3840x2160_30://mstar private format for Vdeec
            {
                eSetMiTiming = E_MI_DISP_OUTPUT_3840x2160_30;
                u16LayerDispWidth = 3840;
                u16LayerDispHeight = 2160;
                eHdmiTiming = E_MI_HDMI_TIMING_4K2K_30P;
            }
             break;
        default:
            {
                eSetMiTiming =  PDispParam->eDispOutputTiming;
                u16LayerDispWidth = PDispParam->u16LayerDispWidth;
                u16LayerDispHeight = PDispParam->u16LayerDispHeight;
                eHdmiTiming = PDispParam->eHdmiTiming;
            }
             break;
    }

    PDispParam->eHdmiTiming = eHdmiTiming;
    PDispParam->u16LayerDispHeight = u16LayerDispHeight;
    PDispParam->u16LayerDispWidth = u16LayerDispWidth;
    PDispParam->eDispOutputTiming = eSetMiTiming;

    return TRUE;
}

static int test_disp_GetOutputTiming(test_Disp_Param_t *pDispParam)
{
    MI_DISP_PubAttr_t stPubAttr;
    memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));

    MI_DISP_GetPubAttr(pDispParam->DispDev, &stPubAttr);
    printf("Device         %d\n", pDispParam->DispDev);
    printf("OutTiming      %d\n", stPubAttr.eIntfSync);
    printf("interface      %d\n", stPubAttr.eIntfType);
    printf("u32FrameRate   %d\n", stPubAttr.stSyncInfo.u32FrameRate);
    printf("BgColor        %d\n", stPubAttr.u32BgColor);

    return TRUE;
}

static MI_BOOL test_disp_changeHdmiTiming(test_Disp_Param_t *pDispParam)
{
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;
    MI_HDMI_Attr_t stHdmiAttr;
    memset(&stHdmiAttr, 0, sizeof(MI_HDMI_Attr_t));

    MI_HDMI_SetAvMute(eHdmi, TRUE);
    MI_HDMI_GetAttr(eHdmi,&stHdmiAttr);
    stHdmiAttr.stVideoAttr.eTimingType = pDispParam->eHdmiTiming;
    MI_HDMI_SetAttr(eHdmi,&stHdmiAttr);
    MI_HDMI_SetAvMute(eHdmi,FALSE);

    return TRUE;
}

static MI_BOOL test_disp_changeDispTiming(test_Disp_Param_t *pDispParam)
{
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_DEV  DispDev = 0;
    MI_DISP_LAYER DispLayer = 0;

    memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    DispLayer = pDispParam->DispLayer;
    DispDev = pDispParam->DispDev;

    if(DispDev == 0)
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    else if(DispDev == 1)
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;

    MI_DISP_GetPubAttr(DispDev, &stPubAttr);
    stPubAttr.eIntfSync = pDispParam->eDispOutputTiming;
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);

    stLayerAttr.stVidLayerSize.u16Width = pDispParam->u16LayerWidth;
    stLayerAttr.stVidLayerSize.u16Height= pDispParam->u16LayerHeight;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = pDispParam->u16LayerDispWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = pDispParam->u16LayerDispHeight;
    stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);

    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    return TRUE;
}

pthread_mutex_t seq_lock = PTHREAD_MUTEX_INITIALIZER;

static int test_disp_changeOutputTiming(test_Disp_Param_t *pDispParam)
{
    MI_U8 u8InputPort = 0;
    MI_U8 u8InputPortNum = 0;
    MI_DISP_DEV  DispDev = 0;
    MI_DISP_LAYER DispLayer = 0;
    MI_U16 u16Avg = 0;
    MI_U16 u16Row = 0;
    MI_U16 u16Column = 0;

    test_Disp_GetPixelParam(pDispParam);

    #if 1
    u8InputPortNum = pDispParam->u16CurrentWinNum;
    DispLayer = pDispParam->DispLayer;
    DispDev = pDispParam->DispDev;

    if (u8InputPortNum <= 1)
    {
        u16Avg = 1;
    }
    else if (u8InputPortNum <= 4)
    {
        u16Avg = 2;
    }
    else if (u8InputPortNum <= 9)
    {
        u16Avg = 3;
    }
    else if (u8InputPortNum <= 16)
    {
        u16Avg = 4;
    }

    ExecFunc(MI_DISP_SetVideoLayerAttrBegin(DispLayer), MI_DISP_SUCCESS);
    // ---> Disable input
    for (u8InputPort = 0; u8InputPort < u8InputPortNum; u8InputPort++)
    {
        ExecFunc(MI_DISP_DisableInputPort(DispLayer, u8InputPort), MI_SUCCESS);
    }
    // ---> Disable layer
    ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_SUCCESS);
    ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_SUCCESS);

    // ---> Disable Device
    ExecFunc(MI_DISP_Disable(DispDev), MI_SUCCESS);

    test_disp_changeDispTiming(pDispParam);
    // ---> Enable Device
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

    test_disp_changeHdmiTiming(pDispParam);//hdmi init after DISP

    //  Restore all input port on Layer

    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));

    for(u8InputPort = 0; u8InputPort < u8InputPortNum; u8InputPort ++)
    {
       stInputPortAttr.stDispWin.u16Width = pDispParam->u16PortWidth;
       stInputPortAttr.stDispWin.u16Height = pDispParam->u16PortHeight;

       u16Row = u8InputPort / u16Avg;
       u16Column = u8InputPort - u16Row * u16Avg;
       stInputPortAttr.stDispWin.u16X = u16Column * pDispParam->u16PortWidth;
       stInputPortAttr.stDispWin.u16Y = u16Row * pDispParam->u16PortHeight;

       ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, u8InputPort, &(stInputPortAttr)), MI_DISP_SUCCESS);
       ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, u8InputPort, &(stInputPortAttr)), MI_DISP_SUCCESS);
       ExecFunc(MI_DISP_EnableInputPort(DispLayer, u8InputPort), MI_SUCCESS)
    }
    ExecFunc(MI_DISP_SetVideoLayerAttrEnd(DispLayer), MI_DISP_SUCCESS);
    #else
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    u8InputPortNum = pDispParam->u16CurrentWinNum;
    DispLayer = pDispParam->DispLayer;
    DispDev = pDispParam->DispDev;

    //MI_DISP_GetPubAttr(DispDev, &stPubAttr);
    if(DispDev == 0)
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    else if(DispDev == 1)
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;

    stPubAttr.eIntfSync = pDispParam->eDispOutputTiming;
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);

    //ExecFunc(MI_DISP_SetVideoLayerAttrBegin(DispLayer), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    stLayerAttr.stVidLayerDispWin.u16Width = pDispParam->u16LayerDispWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = pDispParam->u16LayerDispHeight;
    stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    //ExecFunc(MI_DISP_SetVideoLayerAttrEnd(DispLayer), MI_DISP_SUCCESS);

    test_disp_changeHdmiTiming(pDispParam);

    #endif

    return TRUE;
}

static test_Disp_Timing_t _gTestDispTiming[] = {
    // Main menu
    [E_DISP_OUTPUT_PAL]           = {"DISP_OUTPUT_PAL",          test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_NTSC]          = {"DISP_OUTPUT_NTSC",         test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_720P50]        = {"DISP_OUTPUT_720P50",       test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_720P60]        = {"DISP_OUTPUT_720P60",       test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_1080P50]       = {"DISP_OUTPUT_1080P50",      test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_1080P60]       = {"DISP_OUTPUT_1080P60",      test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_1024x768_60]   = {"DISP_OUTPUT_1024x768_60",  test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_1440x900_60]   = {"DISP_OUTPUT_1440x900_60",  test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_1280x1024_60]  = {"DISP_OUTPUT_1280x1024_60", test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_1600x1200_60]  = {"DISP_OUTPUT_1600x1200_60", test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_3840x2160_30]  = {"DISP_OUTPUT_3840x2160_30", test_disp_changeOutputTiming, E_DISP_OUTPUT_NULL, FALSE},
    [E_DISP_OUTPUT_SHOWTIMING]    = {"DISP_OUTPUT_SHOWTIMING",   test_disp_GetOutputTiming,    E_DISP_OUTPUT_NULL, TRUE},
};

static int getChoice(void)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);
    return atoi(buffer);
}

static int showMenu(int index)
{
    int i = 1;
    test_Disp_Timing_t *pstMenu = NULL;
    printf("\n");
    do
    {
        pstMenu = &_gTestDispTiming[index + i];
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

static void * test_DispTiming (void *pData)
{
    int index = 0;
    int opt = 0;
    int max = 0;

    test_Disp_Timing_t *pstMenu = NULL;
    test_Disp_Param_t *pstDispParam = (test_Disp_Param_t *)pData;

    printf("Welcome: Disp Timing test.\n");
    while(1)
    {
        max = showMenu(index);
        opt = getChoice();
        if ((opt <= 0) || (opt > max))
        {
            printf("Invalid input option !!.\n");
            continue;
        }
        if (opt == max)
        {
            break;
        }

        pstMenu = &_gTestDispTiming[index + opt];
        pstDispParam->opt = opt;
        if (pstMenu->func != NULL)
        {
            pthread_mutex_lock(&seq_lock);
            pstMenu->func(pstDispParam);
            pthread_mutex_unlock(&seq_lock);
        }
        index = pstMenu->next_index;
    }

    _gbStop = TRUE;
    return 0;
}


//change output timing
int main(int argc, const char *argv[])
{
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufConf_t stBufConf;
    struct timeval stTv;
    struct timeval stGetBuffer, stReadFile, stFlushData, stPutBuffer, stRelease;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_INPUTPORT u32InputPort[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    MI_DISP_InputPortAttr_t stInputPortAttr[16];
    // Test Device

    MI_DISP_PubAttr_t stPubAttr;
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;
    MI_HDMI_Attr_t stHdmiAttr;
    int i = 0;
    int srcFd = 0;
    int framesize = 0;
    MI_U32 u32PortId =0;
    MI_SYS_ChnPort_t stDispChnInputPort[16];
    MI_SYS_PixelFormat_e ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_U16 u16Row = 0;
    MI_U16 u16Column = 0;
    MI_U16 u16Avg = 0;

    MI_U32 DispinitWidth = 1920, DispinitHeight = 1080;
    MI_HDMI_TimingType_e eHdmiTiming = E_MI_HDMI_TIMING_1080_60P;
    MI_DISP_OutputTiming_e eInitDispTiming = E_MI_DISP_OUTPUT_1080P60;

    test_Disp_Param_t stDispParam;

    if(argc < 7)
    {
        printf("Test Pattern: [disp DispDev DispLayer filePath width heigt u32PortNum]\n");
        return 0;
    }

    MI_DISP_DEV DispDev = atoi(argv[1]);
    MI_DISP_LAYER DispLayer = atoi(argv[2]);
    const char* filePath = argv[3];
    MI_U16 u16Width = atoi(argv[4]);
    MI_U16 u16Height = atoi(argv[5]);
    MI_U32 u32PortNum = atoi(argv[6]);


    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    memset(&stTv, 0, sizeof(stTv));
    memset(&stBufInfo , 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0, sizeof(MI_SYS_BufConf_t));
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    memset(&stDispParam, 0, sizeof(stDispParam));
    memset(&stHdmiAttr, 0, sizeof(stHdmiAttr));

    for(i = 0; i < argc; i++)
    {
        DBG_INFO("Argument %d is %s.\n", i, argv[i]);
    }

    DBG_INFO("File Path:%s, format:(%u), width = %d, height = %d , port number = %u.\n", filePath,ePixelFormat,u16Width, u16Height,u32PortNum);

    MIDISPCHECKRESULT(MI_SYS_Init());
    if(u32PortNum <= 4)
    {
        u16Avg = 2;
    }
    else if(u32PortNum <= 9)
    {
        u16Avg = 3;
    }
    else if(u32PortNum <= 16)
    {
        u16Avg = 4;
    }

    {
        MI_DISP_PubAttr_t stCvbsPubAttr;
        memset(&stCvbsPubAttr, 0, sizeof(stCvbsPubAttr));
        stCvbsPubAttr.eIntfType = E_MI_DISP_INTF_CVBS;
        stCvbsPubAttr.eIntfSync = E_MI_DISP_OUTPUT_PAL;
        ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    }

    stPubAttr.eIntfSync = eInitDispTiming;
    if(DispDev == 0)
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    else if(DispDev == 1)
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;

    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

    mi_disp_hdmiInit();
    MI_HDMI_GetAttr(eHdmi,&stHdmiAttr);
    stHdmiAttr.stVideoAttr.eTimingType = eHdmiTiming;
    MI_HDMI_SetAttr(eHdmi,&stHdmiAttr);

    stLayerAttr.stVidLayerSize.u16Width = DispinitWidth;
    stLayerAttr.stVidLayerSize.u16Height= DispinitHeight;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = DispinitWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = DispinitHeight;
    stLayerAttr.ePixFormat = ePixelFormat;

    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);

    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    ExecFunc(MI_DISP_SetVideoLayerAttrBegin(DispLayer), MI_DISP_SUCCESS);
    for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
    {
        stDispChnInputPort[u32PortId].eModId = E_MI_MODULE_ID_DISP;
        stDispChnInputPort[u32PortId].u32DevId = DispDev;
        stDispChnInputPort[u32PortId].u32ChnId = 0;
        stDispChnInputPort[u32PortId].u32PortId = u32PortId;

        stInputPortAttr[u32PortId].stDispWin.u16Width = u16Width;
        stInputPortAttr[u32PortId].stDispWin.u16Height = u16Height;

        u16Row = u32PortId / u16Avg;
        u16Column = u32PortId - u16Row * u16Avg;
        stInputPortAttr[u32PortId].stDispWin.u16X = u16Column * u16Width;
        stInputPortAttr[u32PortId].stDispWin.u16Y = u16Row * u16Height;

        ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId])), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_EnableInputPort(DispLayer, u32InputPort[u32PortId]), MI_SUCCESS)
    }
    ExecFunc(MI_DISP_SetVideoLayerAttrEnd(DispLayer), MI_DISP_SUCCESS);

    stDispParam.DispDev = DispDev;
    stDispParam.DispLayer = DispLayer;
    stDispParam.eDispOutputTiming = eInitDispTiming;
    stDispParam.eHdmiTiming = eHdmiTiming;
    stDispParam.u16CurrentWinNum = u32PortNum;
    stDispParam.u16LayerDispHeight = DispinitHeight;
    stDispParam.u16LayerDispWidth = DispinitWidth;
    stDispParam.u16LayerWidth = DispinitWidth;
    stDispParam.u16LayerHeight = DispinitHeight;
    stDispParam.u16PortWidth = u16Width;
    stDispParam.u16PortHeight = u16Height;

    pthread_t thread;
    pthread_attr_t attr;

    int s = pthread_attr_init(&attr);
    if (s != 0)
       perror("pthread_attr_init");

    pthread_create(&thread, &attr, test_DispTiming, &stDispParam);

    if (TRUE == test_disp_OpenSourceFile(filePath, &srcFd))
    {
        while(_gbStop == FALSE)
        {
            gettimeofday(&stTv, NULL);

            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
            stBufConf.stFrameCfg.u16Width = u16Width;
            stBufConf.stFrameCfg.u16Height = u16Height;
            stBufConf.stFrameCfg.eFormat = ePixelFormat;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

            for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
            {
                gettimeofday(&stGetBuffer, NULL);
                pthread_mutex_lock(&seq_lock);
                if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stDispChnInputPort[u32PortId],&stBufConf,&stBufInfo,&hHandle, -1))
                {
                    stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                    stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                    stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
                    stBufInfo.bEndOfStream = FALSE;

                    framesize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                    u16Height = stBufInfo.stFrameData.u16Height;
                    u16Width  = stBufInfo.stFrameData.u16Width;
                    DBG_INFO(" u32PortId = %d, u32Stride = %d, framesize = %d, virAddr = %p  ! \n",
                        u32PortId, stBufInfo.stFrameData.u32Stride[0], framesize, stBufInfo.stFrameData.pVirAddr[0]);
                    //usleep(100 * 1000);

                    gettimeofday(&stReadFile, NULL);
                    if (test_disp_GetOneYuv422Frame(srcFd, 0, stBufInfo.stFrameData.pVirAddr[0], u16Width, u16Height, stBufInfo.stFrameData.u32Stride[0]) == 1)
                    {
                        gettimeofday(&stFlushData, NULL);
                        //MI_SYS_FlushInvCache(stBufInfo.stFrameData.pVirAddr[0], framesize);

                        gettimeofday(&stPutBuffer, NULL);
                        MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);

                        gettimeofday(&stRelease, NULL);
                        DBG_INFO(" %d : %d : %d: %d: %d\n", (stGetBuffer.tv_sec*1000000 + stGetBuffer.tv_usec), (stReadFile.tv_sec*1000000 + stReadFile.tv_usec),
                            (stFlushData.tv_sec*1000000 + stFlushData.tv_usec), (stPutBuffer.tv_sec*1000000 + stPutBuffer.tv_usec),
                            (stRelease.tv_sec*1000000 + stRelease.tv_usec));
                    }
                    else
                    {
                        //DBG_INFO(" _test_disp_GetOneFrame fail.\n");
                        test_disp_FdRewind(srcFd);
                        MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , TRUE);
                    }
                }
                else
                {
                    DBG_INFO("get buf fail\n");
                }
                pthread_mutex_unlock(&seq_lock);
            }
            usleep(30 * 1000);
        }
        s = pthread_attr_destroy(&attr);
        if (s != 0)
            perror("pthread_attr_destroy");
    }
    else
    {
        DBG_INFO(" open file fail. \n");
    }

    DBG_INFO(" Prepare to Exit. \n");

    if(srcFd > 0)
    {
        test_disp_CloseFd(srcFd);
        for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
        {
            ExecFunc(MI_DISP_DisableInputPort(DispLayer, u32InputPort[u32PortId]), MI_SUCCESS);
        }
        ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    }

    usleep(2*1000*1000);
    DBG_INFO("####### disp test104 end ##############\n");

    return MI_SUCCESS;
}
