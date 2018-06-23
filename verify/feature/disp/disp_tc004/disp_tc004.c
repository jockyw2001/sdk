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


#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_hdmi.h"

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

MI_S32 mi_disp_test_HdmTxConfig(MI_U8 u8Timing)
{
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_Attr_t stAttr;
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;
    MI_HDMI_TimingType_e eTiming = E_MI_HDMI_TIMING_MAX;
    switch(u8Timing)
    {
        case 0:
            eTiming = E_MI_HDMI_TIMING_1080_60P;
            break;
        case 1:
            eTiming = E_MI_HDMI_TIMING_720_60P;
            break;
        default:
            break;
    }

    memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
    MI_HDMI_SetAvMute(eHdmi, TRUE);
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

   stAttr.stVideoAttr.eTimingType = eTiming;
   stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
   ExecFunc(MI_HDMI_SetAttr(eHdmi, &stAttr), MI_SUCCESS);
   MI_HDMI_SetAvMute(eHdmi, FALSE);
   return MI_SUCCESS;
}

int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufConf_t stBufConf;
    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));

    if(argc < 7)
    {
        printf("Test Pattern:[disp filePath width heigt FrameCount dev timing]\n");
        printf("Dev: 0 --> HDMI\n");
        printf("   : 1 --> CVBS\n");
        printf("Timing: 0 --> 1080p60hz\n");
        printf("      : 1 --> 720p60hz\n");
        printf("[filePath width heigt FrameCount] --> optional, if not care video \n");
        return 0;
    }
    int i = 0;
    for(i = 0; i < argc; i++)
    {
        printf("Argument %d is %s./n", i, argv[i]);
    }

    const char* filePath = argv[1];
    MI_U16 u16Width = atoi(argv[2]);
    MI_U16 u16Height = atoi(argv[3]);
    MI_U64 u64Times = atoi(argv[4]);
    printf("File Path:%s, width = %d, height = %d , FrameCount = %d\n", filePath, u16Width, u16Height, u64Times);

    MIDISPCHECKRESULT(MI_SYS_Init());
    //Default Config HDMI Dev
    MI_SYS_ChnPort_t stDispChn0InputPort0;
    stDispChn0InputPort0.eModId = E_MI_MODULE_ID_DISP;
    stDispChn0InputPort0.u32DevId = 0;
    stDispChn0InputPort0.u32ChnId = 0;
    stDispChn0InputPort0.u32PortId = 0;
    // Test Device
    MI_DISP_DEV DispDev = 0;
    MI_DISP_PubAttr_t stPubAttr;
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);
    mi_disp_hdmiInit();

    const char* adev = argv[5];
    const char* aTiming = argv[6];

    MI_U8 u8Dev = atoi(adev);
    MI_U8 u8Timing = atoi(aTiming);
    printf("Dev = %d, Timing = %d \n", u8Dev, u8Timing);
    MI_DISP_Interface_e eDev = E_MI_DISP_INTF_HDMI;
    MI_DISP_OutputTiming_e eTiming = E_MI_DISP_OUTPUT_MAX;
    MI_U16 u16DispWidth = 0;
    MI_U16 u16DispHeight = 0;
    switch(u8Dev)
    {
        case 0:
            eDev  = E_MI_DISP_INTF_HDMI;
            break;
        case 1:
            eDev  = E_MI_DISP_INTF_CVBS;
            break;
        default:
            break;
    }

    switch(u8Timing)
    {
        case 0:
            eTiming  = E_MI_DISP_OUTPUT_1080P60;
            u16DispWidth = 1920;
            u16DispHeight = 1080;
            break;
        case 1:
            eTiming  = E_MI_DISP_OUTPUT_720P60;
            u16DispWidth = 1280;
            u16DispHeight = 720;
            break;
        default:
            break;
    }

    ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = eTiming;
    stPubAttr.eIntfType = eDev;
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);
    ExecFunc(mi_disp_test_HdmTxConfig(u8Timing), MI_DISP_SUCCESS);


    MI_DISP_LAYER DispLayer = 0;
    MI_U32 u32Toleration = 100000;
    MI_DISP_CompressAttr_t stCompressAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr, stLayerAttrBackup;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    stLayerAttr.stVidLayerSize.u16Width = u16Width;
    stLayerAttr.stVidLayerSize.u16Height= u16Height;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = u16DispWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = u16DispHeight;
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    printf("[%s %d]Get Video Layer Size [%d, %d] !!!\n", __FUNCTION__, __LINE__, stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    printf("[%s %d]Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", __FUNCTION__, __LINE__,\
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    MI_DISP_INPUTPORT LayerInputPort = 0;

    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    stInputPortAttr.stDispWin.u16Width    = u16Width;
    stInputPortAttr.stDispWin.u16Height = u16Height;
    ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_SUCCESS)

    FILE *fd = fopen(filePath, "r+");
    if(fd!=NULL)
    {
        while(1)
        {
            int n = 0;
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
            stBufConf.stFrameCfg.u16Width = u16Width;
            stBufConf.stFrameCfg.u16Height = u16Height;


            stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stDispChn0InputPort0,&stBufConf,&stBufInfo,&hHandle, -1))
            {
                stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
                stBufInfo.bEndOfStream = FALSE;
                printf(" [%s %d] virAddr = %p  !!!!!\n", __FUNCTION__, __LINE__,stBufInfo.stFrameData.pVirAddr[0]);
                int size = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];

                printf(" [%s %d] u32Stride = %d  \n", __FUNCTION__, __LINE__, stBufInfo.stFrameData.u32Stride[0]);
                printf(" [%s %d] size = %d  \n", __FUNCTION__, __LINE__, size);

                n = fread(stBufInfo.stFrameData.pVirAddr[0], 1, size, fd);

                printf("[%s %d] size = %d, n = %d!!!!!!!!!!!!!!!!!!!!!!!!\n", __FUNCTION__, __LINE__, size, n);
                //MI_SYS_FlushInvCache(stBufInfo.stFrameData.pVirAddr[0], size);
                if(n == size)
                {
                    MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                }
                else if(n < size)
                {
                    printf("[%s %d] size = %d, n = %d!!!!!!!!!!!!!!!!!!!!!!!!\n", __FUNCTION__, __LINE__, size, n);
                    //stBufInfo.bEndOfStream = TRUE;
                    //MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                    MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , TRUE);
                    rewind(fd);
                    //break;
                }

                u64Times--;
                if(u64Times == 0)
                {
                    break;
                }
            }
            else
            {
                printf("get buf fail\n");
            }
            //usleep(100 * 1000);
        }
    }
    if(fd > 0)
    {
        fclose(fd);
        ExecFunc(MI_DISP_DisableInputPort(DispLayer, LayerInputPort), MI_SUCCESS);
        ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    }
    printf("disp test end\n");

    return MI_SUCCESS;
}
