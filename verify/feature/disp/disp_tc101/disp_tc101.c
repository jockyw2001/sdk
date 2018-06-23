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

test_disp_config_s disp_tc102[]={
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
        .Desc = "2560x1440 P30",
        .FileName = "YUV422_YUYV1920_1080.yuv",
        .u16LayWidth = 2560,
        .u16LayHeight = 1440,
        .u16InputWidth = 1920,
        .u16InputHeight = 1080,
        .eDispTiming = E_MI_DISP_OUTPUT_2560x1440_30,
        .eHdmiTiming = E_MI_HDMI_TIMING_1440_30P,
    },
    {
        .u8CaseNum = 5,
        .Desc = "3840x2160 P30",
        .FileName = "YUV422_YUYV1920_1080.yuv",
        .u16LayWidth = 3840,
        .u16LayHeight = 2160,
        .u16InputWidth = 1920,
        .u16InputHeight = 1080,
        .eDispTiming = E_MI_DISP_OUTPUT_3840x2160_30,
        .eHdmiTiming = E_MI_HDMI_TIMING_4K2K_30P,
    },
    {
        .u8CaseNum = 6,
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
        .u8CaseNum = 7,
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
        .u8CaseNum = 8,
        .Desc = "1280x1024 P60",
        .FileName = "YUV422_YUYV1024_768.yuv",
        .u16LayWidth = 1280,
        .u16LayHeight = 1024,
        .u16InputWidth = 1024,
        .u16InputHeight = 768,
        .eDispTiming = E_MI_DISP_OUTPUT_1280x1024_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1280x1024_60P,
    },
    {
        .u8CaseNum = 9,
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
        .u8CaseNum = 10,
        .Desc = "1366x768 P60VGA",
        .FileName = "YUV422_YUYV1440_900.yuv",
        .u16LayWidth = 1366,
        .u16LayHeight = 768,
        .u16InputWidth = 1440,
        .u16InputHeight = 900,
        .eDispTiming = E_MI_DISP_OUTPUT_1366x768_60,
        //.eHdmiTiming = E_MI_HDMI_TIMING_1600x1200_60P,
    },
    {
        .u8CaseNum = 11,
        .Desc = "1280x800 P60VGA",
        .FileName = "YUV422_YUYV1440_900.yuv",
        .u16LayWidth = 1280,
        .u16LayHeight = 800,
        .u16InputWidth = 1440,
        .u16InputHeight = 900,
        .eDispTiming = E_MI_DISP_OUTPUT_1280x800_60,
        //.eHdmiTiming = E_MI_HDMI_TIMING_1600x1200_60P,
    },
    {
        .u8CaseNum = 12,
        .Desc = "1680x1050 P60VGA",
        .FileName = "YUV422_YUYV1440_900.yuv",
        .u16LayWidth = 1680,
        .u16LayHeight = 1050,
        .u16InputWidth = 1440,
        .u16InputHeight = 900,
        .eDispTiming = E_MI_DISP_OUTPUT_1680x1050_60,
        //.eHdmiTiming = E_MI_HDMI_TIMING_1600x1200_60P,
    },
    {
        .u8CaseNum = 13,
        .Desc = "CVBS PAL 720X576",
        .FileName = "YUV422_YUYV720_480.yuv",
        .u16LayWidth = 720,
        .u16LayHeight = 576,
        .u16InputWidth = 720,
        .u16InputHeight = 480,
        .eDispTiming = E_MI_DISP_OUTPUT_PAL,
        .eHdmiTiming = E_MI_HDMI_TIMING_1080_60P,
    },
    {
        .u8CaseNum = 14,
        .Desc = "CVBS NTSC 720X480",
        .FileName = "YUV422_YUYV720_480.yuv",
        .u16LayWidth = 720,
        .u16LayHeight = 480,
        .u16InputWidth = 720,
        .u16InputHeight = 480,
        .eDispTiming = E_MI_DISP_OUTPUT_NTSC,
        .eHdmiTiming = E_MI_HDMI_TIMING_1080_60P,
    },
    {
        .u8CaseNum = 15,
        .Desc = "800x600 P60",
        .FileName = "YUV422_YUYV1024_768.yuv",
        .u16LayWidth = 800,
        .u16LayHeight = 600,
        .u16InputWidth = 1024,
        .u16InputHeight = 768,
        .eDispTiming = E_MI_DISP_OUTPUT_800x600_60,
        //.eHdmiTiming = E_MI_HDMI_TIMING_1280x1024_60P,
    },
    {
        .u8CaseNum = 16,
        .Desc = "576 P50",
        .FileName = "YUV422_YUYV1024_768.yuv",
        .u16LayWidth = 720,
        .u16LayHeight = 576,
        .u16InputWidth = 1024,
        .u16InputHeight = 768,
        .eDispTiming = E_MI_DISP_OUTPUT_576P50,
        .eHdmiTiming = E_MI_HDMI_TIMING_576_50P,
    },
    {
        .u8CaseNum = 17,
        .Desc = "480 P60",
        .FileName = "YUV422_YUYV1024_768.yuv",
        .u16LayWidth = 640,
        .u16LayHeight = 480,
        .u16InputWidth = 1024,
        .u16InputHeight = 768,
        .eDispTiming = E_MI_DISP_OUTPUT_480P60,
        .eHdmiTiming = E_MI_HDMI_TIMING_480_60P,
    },
};

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

int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufConf_t stBufConf;
    MI_BOOL bsleep = true;
    MI_U8 u8Interface =0;
    struct timeval stTv;
    struct timeval stGetBuffer, stReadFile, stFlushData, stPutBuffer, stRelease;
    MI_SYS_PixelFormat_e ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_HDMI_DeviceId_e eHdmi = 0;
    MI_U32 u32ChnId = 0;
    MI_U8 u8Index=0;
    char TestNum[5];
    char FilePath[100];

    int i = 0;
    MI_DISP_DEV DispDev =0;
    MI_DISP_LAYER DispLayer =0;
    char pbaseDir[40];
    MI_U64 u64Times =0;

    memset(&stTv, 0, sizeof(stTv));
    time_t stTime = 0;
    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));
    if(argc < 5)
    {
        printf("@@@@@ Test Pattern @@@@:\n [disp dev layer dir times]\n");
        return 0;
    }
    else
    {
        DispDev = atoi(argv[1]);
        DispLayer = atoi(argv[2]);
        snprintf(pbaseDir, (sizeof(pbaseDir)-1), "%s", argv[3]);
        u64Times = atoi(argv[4]);
    }

    printf("please input test num:\n");
    for(i = 0; i < sizeof(disp_tc102)/sizeof(disp_tc102[0]); i++)
    {
        printf("%d:%s\n",disp_tc102[i].u8CaseNum,disp_tc102[i].Desc);
    }

    memset(TestNum,0,sizeof(TestNum));
    fgets((TestNum), (sizeof(TestNum) - 1), stdin);
    u8Index = atoi(TestNum) & 0xFF;
    if(u8Index > sizeof(disp_tc102)/sizeof(disp_tc102[0]))
    {
        return 0;
    }
    else
        snprintf(FilePath, (sizeof(FilePath)-1), "%s/%s", pbaseDir, disp_tc102[u8Index].FileName);

    MIDISPCHECKRESULT(MI_SYS_Init());

    // Test Device

    MI_DISP_PubAttr_t stPubAttr;
    memset(&stPubAttr, 0, sizeof(stPubAttr));

    if(DispDev == 0)
    {
        u8Interface =DispDev;
        stPubAttr.eIntfSync = disp_tc102[u8Index].eDispTiming;
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
        ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    }
    else if(DispDev == 1)
    {
        u8Interface =DispDev;
        stPubAttr.eIntfSync = disp_tc102[u8Index].eDispTiming;
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
        ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    }
    else if(DispDev == 2)
    {
        u8Interface =DispDev;
        DispDev = 0;
        stPubAttr.eIntfSync = disp_tc102[u8Index].eDispTiming;
        stPubAttr.eIntfType = E_MI_DISP_INTF_CVBS;
        ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    }
    else if(DispDev == 3)
    {
        //MI_U8 tmp_Index = u8Index;
        u8Interface =DispDev;
        DispDev = 0;

        stPubAttr.eIntfSync = disp_tc102[u8Index].eDispTiming;
        stPubAttr.eIntfType = E_MI_DISP_INTF_CVBS;
        printf("set interface cvbs\n");
        ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);

        printf("please input HDMI test num:\n");
        for(i = 0; i < sizeof(disp_tc102)/sizeof(disp_tc102[0]); i++)
        {
            printf("%d:%s\n",disp_tc102[i].u8CaseNum,disp_tc102[i].Desc);
        }

        memset(TestNum,0,sizeof(TestNum));
        fgets((TestNum), (sizeof(TestNum) - 1), stdin);
        u8Index = atoi(TestNum) & 0xff;
        if(u8Index > sizeof(disp_tc102)/sizeof(disp_tc102[0]))
        {
            return 0;
        }
        else
            snprintf(FilePath, sizeof(FilePath), "%s/%s", pbaseDir, disp_tc102[u8Index].FileName);

        stPubAttr.eIntfSync = disp_tc102[u8Index].eDispTiming;
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
        printf("interface %d \n", stPubAttr.eIntfType);
        ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);

        //u8Index = tmp_Index;
    }

    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);
    u32ChnId = _test_disp_TransLayerToDevChnn(DispDev,DispLayer);

    if(u8Interface !=1 && u8Interface !=2)
    {
        mi_disp_hdmiInit();
        printf("HDMI_INIT BEGIN");
        MI_HDMI_Attr_t stHdmiAttr;
        MI_HDMI_GetAttr(eHdmi,&stHdmiAttr);
        stHdmiAttr.stVideoAttr.eTimingType = disp_tc102[u8Index].eHdmiTiming;
        MI_HDMI_SetAttr(eHdmi,&stHdmiAttr);
    }

    MI_U32 u32Toleration = 100000;
    MI_DISP_CompressAttr_t stCompressAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr, stLayerAttrBackup;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    stLayerAttr.stVidLayerSize.u16Width = disp_tc102[u8Index].u16InputWidth;
    stLayerAttr.stVidLayerSize.u16Height= disp_tc102[u8Index].u16InputHeight;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = disp_tc102[u8Index].u16LayWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = disp_tc102[u8Index].u16LayHeight;
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
    stInputPortAttr.stDispWin.u16Width = disp_tc102[u8Index].u16InputWidth;
    stInputPortAttr.stDispWin.u16Height = disp_tc102[u8Index].u16InputHeight;

    ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_SUCCESS)

    MI_SYS_ChnPort_t stDispChn0InputPort0;
    stDispChn0InputPort0.eModId = E_MI_MODULE_ID_DISP;
    stDispChn0InputPort0.u32DevId = DispDev;
    stDispChn0InputPort0.u32ChnId = u32ChnId;
    stDispChn0InputPort0.u32PortId = 0;

    int srcFd = 0;
    if (TRUE == test_disp_OpenSourceFile(FilePath, &srcFd))
    {
        while(1)
        {
            int n = 0;
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            gettimeofday(&stTv, NULL);

            stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
            stBufConf.stFrameCfg.u16Width = disp_tc102[u8Index].u16InputWidth;
            stBufConf.stFrameCfg.u16Height = disp_tc102[u8Index].u16InputHeight;
            stBufConf.stFrameCfg.eFormat = ePixelFormat;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

            gettimeofday(&stGetBuffer, NULL);
            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stDispChn0InputPort0,&stBufConf,&stBufInfo,&hHandle, -1))
            {
                stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;

                stBufInfo.bEndOfStream = FALSE;
                int size = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                gettimeofday(&stReadFile, NULL);

                if (1 == test_disp_GetOneYuv422Frame(srcFd, 0, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.u16Width, stBufInfo.stFrameData.u16Height, stBufInfo.stFrameData.u32Stride[0]))
                {
                    gettimeofday(&stFlushData, NULL);
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

                u64Times--;
                if(u64Times == 0)
                {
                    break;
                }
                usleep(100*1000);
            }
            else
            {
                printf("get buf fail\n");
            }
        }
    }
    else
    {
        DBG_INFO(" open file fail. \n");
    }

    DBG_INFO(" Prepare to Exit. \n");

    if(u8Interface !=1 && u8Interface !=2)
    {
        MI_HDMI_Close(eHdmi);
        MI_HDMI_Stop(eHdmi);
    }

    if(srcFd > 0)
    {
        test_disp_CloseFd(srcFd);
        ExecFunc(MI_DISP_DisableInputPort(DispLayer, LayerInputPort), MI_SUCCESS);
        ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    }

    usleep(2*1000*1000);
    DBG_INFO("####### disp test101 end ##############\n");
    return MI_SUCCESS;
}
