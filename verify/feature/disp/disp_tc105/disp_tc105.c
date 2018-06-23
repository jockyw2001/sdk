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

#include <sys/time.h>
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_hdmi.h"
#include "../mi_disp_test.h"

//#define DISP_FPGA_TEST
///#define MI_PRINT(fmt, args...)
#define MI_PRINT printf
#define DBG_INFO(fmt, args...)      ({do{printf("[MI INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);usleep(10 * 1000);}while(0);})

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

typedef struct test_disp_config_t
{
    MI_U8 u8CaseNum;
    char Desc[50];
    char FileName[50];
    MI_U16 u16LayWidth;
    MI_U16 u16LayHeight;
    MI_U16 u16InputWidth;
    MI_U16 u16InputHeight;
    MI_DISP_OutputTiming_e eDispTiming;
    MI_HDMI_TimingType_e eHdmiTiming;
}test_disp_config_s;

test_disp_config_s disp_tc105[]={
     {
        .u8CaseNum = 0,
        .Desc = "1280x720 P60",
        .FileName = "YUV422_YUYV1280_720.yuv",
        .u16LayWidth = 1280,
        .u16LayHeight = 720,
        .u16InputWidth = 1280,
        .u16InputHeight = 720,
        .eDispTiming = E_MI_DISP_OUTPUT_720P60,
        .eHdmiTiming = E_MI_HDMI_TIMING_720_60P,
    },
    {
        .u8CaseNum = 1,
        .Desc = "1280x720 P50",
        .FileName = "YUV422_YUYV1280_720.yuv",
        .u16LayWidth = 1280,
        .u16LayHeight = 720,
        .u16InputWidth = 1280,
        .u16InputHeight = 720,
        .eDispTiming = E_MI_DISP_OUTPUT_720P50,
        .eHdmiTiming = E_MI_HDMI_TIMING_720_50P,
    },
    {
        .u8CaseNum = 2,
        .Desc = "1920x1080 P60",
        .FileName = "YUV422_YUYV1920_1080.yuv",
        .u16LayWidth = 1920,
        .u16LayHeight = 1080,
        .u16InputWidth = 1920,
        .u16InputHeight = 1080,
        .eDispTiming = E_MI_DISP_OUTPUT_1080P60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1080_60P,
    },
    {
        .u8CaseNum = 3,
        .Desc = "1920x1080 P50",
        .FileName = "YUV422_YUYV1920_1080.yuv",
        .u16LayWidth = 1920,
        .u16LayHeight = 1080,
        .u16InputWidth = 1920,
        .u16InputHeight = 1080,
        .eDispTiming = E_MI_DISP_OUTPUT_1080P50,
        .eHdmiTiming = E_MI_HDMI_TIMING_1080_50P,
    },
    {
        .u8CaseNum = 4,
        .Desc = "1024x768 P60",
        .FileName = "YUV422_YUYV1024_768.yuv",
        .u16LayWidth = 1024,
        .u16LayHeight = 768,
        .u16InputWidth = 1024,
        .u16InputHeight = 768,
        .eDispTiming = E_MI_DISP_OUTPUT_1024x768_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1024x768_60P,
    },
    {
        .u8CaseNum = 5,
        .Desc = "1440x900 P60",
        .FileName = "YUV422_YUYV1440_900.yuv",
        .u16LayWidth = 1440,
        .u16LayHeight = 900,
        .u16InputWidth = 1440,
        .u16InputHeight = 900,
        .eDispTiming = E_MI_DISP_OUTPUT_1440x900_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1440x900_60P,
    },
    {
        .u8CaseNum = 6,
        .Desc = "1280x1024 P60",
        .FileName = "YUV422_YUYV1440_900.yuv",
        .u16LayWidth = 1280,
        .u16LayHeight = 1024,
        .u16InputWidth = 1440,
        .u16InputHeight = 900,
        .eDispTiming = E_MI_DISP_OUTPUT_1280x1024_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1280x1024_60P,
    },
    {
        .u8CaseNum = 7,
        .Desc = "1600x1200 P60",
        .FileName = "YUV422_YUYV1440_900.yuv",
        .u16LayWidth = 1600,
        .u16LayHeight = 1200,
        .u16InputWidth = 1440,
        .u16InputHeight = 900,
        .eDispTiming = E_MI_DISP_OUTPUT_1600x1200_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1600x1200_60P,
    },
    {
        .u8CaseNum = 8,
        .Desc = "3840x2160",
        .FileName = "YUV422_YUYV1600_1200.yuv",
        .u16LayWidth = 3840,
        .u16LayHeight = 2160,
        .u16InputWidth = 1600,
        .u16InputHeight = 1200,
        .eDispTiming = E_MI_DISP_OUTPUT_1600x1200_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1600x1200_60P,
    }
};

#define DISP_MAX_DEVNum 2

static MI_U32 _test_disp_TransLayerToDevChnn(MI_DISP_DEV DispDev, MI_DISP_LAYER DispLayer)
{
    if ((DispLayer == 1) && (DispDev == 1))
    {
        return 0;
    }
    else
    {
        return DispLayer;
    }

}

int SetUserConfig(test_disp_config_s *pdisp_tc105_tmp,char *FilePath, MI_U8 *u8Index, const char* pbaseDir)
{
    MI_U8 u8CaseNum = 0;
    MI_U8 i =0;
    char TestNum[8];

    u8CaseNum = 9;

    for(i = 0; i <u8CaseNum ; i++)
    {
        printf("%d:%s\n",pdisp_tc105_tmp[i].u8CaseNum,pdisp_tc105_tmp[i].Desc);
    }
    fgets((TestNum), (sizeof(TestNum) - 1), stdin);

    *u8Index = atoi(TestNum);
    sprintf(FilePath, "%s/%s", pbaseDir, pdisp_tc105_tmp[*u8Index].FileName);

    return MI_DISP_SUCCESS;
}

MI_S32 mi_disp_hdmiInit_withPixel(MI_HDMI_TimingType_e eHdmiTiming)
{
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_Attr_t stAttr;
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;

    stInitParam.pCallBackArgs = NULL;
    stInitParam.pfnHdmiEventCallback = NULL;

    MI_HDMI_Init(&stInitParam);

    ExecFunc(MI_HDMI_Open(eHdmi), MI_SUCCESS);
    memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
    stAttr.stEnInfoFrame.bEnableAudInfoFrame  = FALSE;
    stAttr.stEnInfoFrame.bEnableAviInfoFrame  = FALSE;
    stAttr.stEnInfoFrame.bEnableSpdInfoFrame  = FALSE;
    stAttr.stAudioAttr.bEnableAudio = TRUE;
    stAttr.stAudioAttr.bIsMultiChannel = 0;
    stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
    stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
    stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
    stAttr.stVideoAttr.bEnableVideo = TRUE;
    stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_RGB444;//default color type
    stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_MAX;
    stAttr.stVideoAttr.eTimingType = eHdmiTiming;
    stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
    MI_HDMI_SetAttr(eHdmi, &stAttr);

    MI_HDMI_Start(eHdmi);

    return MI_DISP_SUCCESS;
}

int DeviceConfig(MI_DISP_DEV DispDev, MI_DISP_LAYER DispLayer, test_disp_config_s *pdisp_tc105, MI_SYS_PixelFormat_e ePixelFormat)
{
    MI_DISP_PubAttr_t stPubAttr;
    MI_HDMI_DeviceId_e eHdmi = 0;
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = pdisp_tc105->eDispTiming;

    if(DispDev == 0)
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    else
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;

    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

    if(DispDev == 0)
        ExecFunc(mi_disp_hdmiInit_withPixel(pdisp_tc105->eHdmiTiming), MI_DISP_SUCCESS);

    MI_U32 u32Toleration = 100000;
    MI_DISP_CompressAttr_t stCompressAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr, stLayerAttrBackup;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    stLayerAttr.stVidLayerSize.u16Width = pdisp_tc105->u16InputWidth;
    stLayerAttr.stVidLayerSize.u16Height= pdisp_tc105->u16InputHeight;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = pdisp_tc105->u16LayWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = pdisp_tc105->u16LayHeight;
    stLayerAttr.ePixFormat = ePixelFormat;
    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);

    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DBG_INFO("Get Video Layer Size [%d, %d] !!!\n", stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("Get Video Layer DispWin [%d, %d, %d, %d] !!!\n",\
    stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    MI_DISP_INPUTPORT LayerInputPort = 0;

    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));

    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    stInputPortAttr.stDispWin.u16Width = pdisp_tc105->u16InputWidth;
    stInputPortAttr.stDispWin.u16Height = pdisp_tc105->u16InputHeight;

    ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_SUCCESS);

    return MI_DISP_SUCCESS;
}

int DeviceAttachConfig(MI_DISP_DEV DispDev, MI_DISP_LAYER DispLayer, test_disp_config_s *pdisp_tc105, MI_SYS_PixelFormat_e ePixelFormat)
{
    MI_DISP_PubAttr_t stPubAttr;
    MI_HDMI_DeviceId_e eHdmi = 0;
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = pdisp_tc105->eDispTiming;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;

    ExecFunc(MI_DISP_SetPubAttr(0,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(0), MI_DISP_SUCCESS);

    if(DispDev == 0)
        ExecFunc(mi_disp_hdmiInit_withPixel(pdisp_tc105->eHdmiTiming), MI_DISP_SUCCESS);

    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = pdisp_tc105->eDispTiming;
    stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;

    ExecFunc(MI_DISP_SetPubAttr(1,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(1), MI_DISP_SUCCESS);
    DispLayer = 0;

    MI_U32 u32Toleration = 100000;
    MI_DISP_CompressAttr_t stCompressAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr, stLayerAttrBackup;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    stLayerAttr.stVidLayerSize.u16Width = pdisp_tc105->u16InputWidth;
    stLayerAttr.stVidLayerSize.u16Height= pdisp_tc105->u16InputHeight;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = pdisp_tc105->u16LayWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = pdisp_tc105->u16LayHeight;
    stLayerAttr.ePixFormat = ePixelFormat;
    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);

    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DBG_INFO("Get Video Layer Size [%d, %d] !!!\n", stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("Get Video Layer DispWin [%d, %d, %d, %d] !!!\n",\
    stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    MI_DISP_INPUTPORT LayerInputPort = 0;

    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));

    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    stInputPortAttr.stDispWin.u16Width = pdisp_tc105->u16InputWidth;
    stInputPortAttr.stDispWin.u16Height = pdisp_tc105->u16InputHeight;

    ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_SUCCESS);

    ExecFunc(MI_DISP_DeviceAttach(0, 1), MI_DISP_SUCCESS);
    return MI_DISP_SUCCESS;
}

typedef struct test_Disp_data_s
{
    char *FilePath;
    test_disp_config_s *pdisp_tc105_data;
    MI_SYS_ChnPort_t *pstDispDevInputPort0;
    MI_SYS_PixelFormat_e ePixelFormat;
    MI_U64 u64Times;
}test_Disp_data_t;

static void * test_Disp_PutBuffer (void *pData)
{
    int srcFd = 0;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BUF_HANDLE hHandle;
    struct timeval stTv;
    struct timeval stGetBuffer, stReadFile, stFlushData, stPutBuffer, stRelease;

    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));
    memset(&stTv, 0, sizeof(stTv));
    time_t stTime = 0;

    test_Disp_data_t *pstDispData = (test_Disp_data_t *)pData;
    test_disp_config_s *pdisp_tc105_data = pstDispData->pdisp_tc105_data;
    MI_SYS_ChnPort_t *pstDispDevInputPort0 = pstDispData->pstDispDevInputPort0;
    char *FilePath = pstDispData->FilePath;

    if (TRUE == test_disp_OpenSourceFile(FilePath, &srcFd))
    {
        while(1)
        {
            int n = 0;
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            gettimeofday(&stTv, NULL);

            stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
            stBufConf.stFrameCfg.u16Width = pdisp_tc105_data->u16InputWidth;
            stBufConf.stFrameCfg.u16Height = pdisp_tc105_data->u16InputHeight;
            stBufConf.stFrameCfg.eFormat = pstDispData->ePixelFormat;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

            gettimeofday(&stGetBuffer, NULL);
            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(pstDispDevInputPort0,&stBufConf,&stBufInfo,&hHandle, -1))
            {
                stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;

                stBufInfo.bEndOfStream = FALSE;
                int size = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                gettimeofday(&stReadFile, NULL);

                if (test_disp_GetOneYuv422Frame(srcFd, 0, stBufInfo.stFrameData.pVirAddr[0], pdisp_tc105_data->u16InputWidth, pdisp_tc105_data->u16InputHeight, stBufInfo.stFrameData.u32Stride[0]) == 1)
                {
                    gettimeofday(&stFlushData, NULL);
                    //remove un-needed flushcache.
                    //MI_SYS_FlushInvCache(stBufInfo.stFrameData.pVirAddr[0], size);
                    gettimeofday(&stPutBuffer, NULL);

                    MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                    gettimeofday(&stRelease, NULL);
                }
                else
                {
                    //DBG_INFO(" test_disp_GetOneFrame fail.\n");
                    test_disp_FdRewind(srcFd);
                    MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , TRUE);
                }

                pstDispData->u64Times --;
                if(pstDispData->u64Times == 0)
                {
                    break;
                }
                usleep(30*1000);
            }
            else
            {
                DBG_INFO("get buf fail\n");
            }
        }
    }
    else
    {
        DBG_INFO(" open file fail. \n");
    }

    if(srcFd > 0)
    {
        MI_DISP_LAYER DispLayer =0;
        test_disp_CloseFd(srcFd);

        MI_DISP_DisableInputPort(pstDispDevInputPort0->u32DevId, pstDispDevInputPort0->u32PortId);

        if(pstDispDevInputPort0->u32DevId == 0)
            DispLayer = 0;
        else
            DispLayer = 1;

        MI_DISP_DisableVideoLayer(DispLayer);
        MI_DISP_UnBindVideoLayer(DispLayer, pstDispDevInputPort0->u32DevId);
        MI_DISP_Disable(pstDispDevInputPort0->u32DevId);
    }

    return MI_DISP_SUCCESS;
}

int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_BOOL bsleep = true;

    MI_SYS_PixelFormat_e ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_U32 u32ChnId0 = 0;
    MI_U32 u32ChnId1 = 0;
    MI_U8 u8Index0;
    MI_U8 u8Index1;
    char pbaseDir[40];

    pthread_t thread0;
    pthread_attr_t attr0;
    int s0;

    pthread_t thread1;
    pthread_attr_t attr1;
    int s1;

    char FilePath0[100];
    char FilePath1[100];

    if(argc < 5)
    {
        printf("@@@@@ Test Pattern @@@@:\n [disp src dest dir]\n");
        printf("@@@@@ Test Pattern @@@@:\n src == dest for different source, src != dest for same source\n");
        return 0;
    }

    int i = 0,j=0;

    MI_DISP_DEV  DispDev0 = 0;
    MI_DISP_DEV  DispDev1 = 1;
    MI_DISP_LAYER DispLayer0 = 0;
    MI_DISP_LAYER DispLayer1 = 1;

    MI_DISP_DEV DispAttachSrcDev = atoi(argv[1]);
    MI_DISP_DEV DispAttachDstDev = atoi(argv[2]);
    snprintf(pbaseDir, (sizeof(pbaseDir)-1), "%s", argv[3]);
    MI_U64 u64Times = atoi(argv[4]);

    if(DispAttachSrcDev == DispAttachDstDev)
    {
        printf("same source output  ");
    }
    else
    {
        printf("different source output  ");
    }

    printf("please input HDMI resolution:\n");
    ExecFunc(SetUserConfig(disp_tc105, FilePath0, &u8Index0,pbaseDir), MI_DISP_SUCCESS);

    printf("please input VGA resolution:\n");
    ExecFunc(SetUserConfig(disp_tc105, FilePath1, &u8Index1,pbaseDir), MI_DISP_SUCCESS);

    MIDISPCHECKRESULT(MI_SYS_Init());

    u32ChnId0 = _test_disp_TransLayerToDevChnn(DispDev0, DispLayer0);
    u32ChnId1 = _test_disp_TransLayerToDevChnn(DispDev1, DispLayer1);

    // Test Device

    //ExecFunc(DeviceAttachConfig(0,0, &disp_tc105[u8Index0], ePixelFormat),MI_DISP_SUCCESS);
    if(DispAttachSrcDev != DispAttachDstDev)
    {
        ExecFunc(DeviceAttachConfig(0,0,&disp_tc105[u8Index0], ePixelFormat),MI_DISP_SUCCESS);
    }
    else
    {
        ExecFunc(DeviceConfig(DispDev0, DispLayer0, &disp_tc105[u8Index0], ePixelFormat), MI_DISP_SUCCESS);
        ExecFunc(DeviceConfig(DispDev1, DispLayer1, &disp_tc105[u8Index1], ePixelFormat), MI_DISP_SUCCESS);
    }

    MI_SYS_ChnPort_t stDispDev0InputPort0;
    stDispDev0InputPort0.eModId = E_MI_MODULE_ID_DISP;
    stDispDev0InputPort0.u32DevId = DispDev0;
    stDispDev0InputPort0.u32ChnId = u32ChnId0;
    stDispDev0InputPort0.u32PortId = 0;

    MI_SYS_ChnPort_t stDispDev1InputPort0;
    stDispDev1InputPort0.eModId = E_MI_MODULE_ID_DISP;
    stDispDev1InputPort0.u32DevId = DispDev1;
    stDispDev1InputPort0.u32ChnId = u32ChnId1;
    stDispDev1InputPort0.u32PortId = 0;

    test_Disp_data_t stDispDev0Param, stDispDev1Param;
    memset(&stDispDev0Param, 0, sizeof(test_Disp_data_t));
    memset(&stDispDev1Param, 0, sizeof(test_Disp_data_t));

    stDispDev0Param.FilePath = FilePath0;
    stDispDev0Param.ePixelFormat = ePixelFormat;
    stDispDev0Param.pdisp_tc105_data = &disp_tc105[u8Index0];
    stDispDev0Param.pstDispDevInputPort0 = &stDispDev0InputPort0;
    stDispDev0Param.u64Times = u64Times;

    stDispDev1Param.FilePath = FilePath1;
    stDispDev1Param.ePixelFormat = ePixelFormat;
    stDispDev1Param.pdisp_tc105_data = &disp_tc105[u8Index1];
    stDispDev1Param.pstDispDevInputPort0 = &stDispDev1InputPort0;
    stDispDev1Param.u64Times = u64Times;

    s0 = pthread_attr_init(&attr0);
    if (s0 != 0)
        perror("pthread_attr_init");

    if(DispAttachSrcDev == DispAttachDstDev)
    {
        s1 = pthread_attr_init(&attr1);
        if (s1 != 0)
            perror("pthread_attr_init");
        pthread_create(&thread1, &attr1, test_Disp_PutBuffer, &stDispDev1Param);
    }

    pthread_create(&thread0, &attr0, test_Disp_PutBuffer, &stDispDev0Param);

    pthread_join(thread0, NULL);

    if(DispAttachSrcDev == DispAttachDstDev)
    {
        pthread_join(thread1, NULL);

        s1 = pthread_attr_destroy(&attr1);
        if (s1 != 0)
            perror("pthread_attr_destroy");
    }

    s0 = pthread_attr_destroy(&attr0);
    if (s0 != 0)
        perror("pthread_attr_destroy");

    DBG_INFO(" Prepare to Exit. \n");

    usleep(2*1000*1000);
    DBG_INFO("####### disp test100 end ##############\n");
    while(0)
    {
        if(bsleep)
        {
            bsleep = false;
            DBG_INFO("sleep. \n");
        }
        usleep(100*1000);
    }
    return MI_SUCCESS;
}
