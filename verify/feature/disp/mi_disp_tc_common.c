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
#include "mi_disp_test.h"
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

MI_S32 mi_disp_test_SetDevPubAttr(MI_DISP_DEV DispChn, MI_DISP_PubAttr_t* pstDevAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    s32Ret = MI_DISP_SetPubAttr(DispChn, pstDevAttr);
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_disp_test_SetDevBgColor(MI_DISP_DEV DispChn, MI_U32 u32BgColor)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_PubAttr_t stDevAttr;
    memset(&stDevAttr, 0, sizeof(MI_DISP_PubAttr_t));

    s32Ret = MI_DISP_GetPubAttr(DispChn, &stDevAttr);
    MIDISPCHECKRESULT(s32Ret);
    stDevAttr.u32BgColor = u32BgColor;
    s32Ret = MI_DISP_SetPubAttr(DispChn, &stDevAttr);
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_disp_test_SetOutPutTiming(MI_DISP_DEV DispChn, MI_DISP_Interface_e eIntfType, MI_DISP_OutputTiming_e eOuttiming, MI_DISP_SyncInfo_t* pstSyncInfo)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_PubAttr_t stDevAttr;
    memset(&stDevAttr, 0, sizeof(MI_DISP_PubAttr_t));

    s32Ret = MI_DISP_GetPubAttr(DispChn, &stDevAttr);
    MIDISPCHECKRESULT(s32Ret);
    stDevAttr.eIntfType = eIntfType;
    stDevAttr.eIntfSync = eOuttiming;
    stDevAttr.stSyncInfo = *pstSyncInfo;
    s32Ret = MI_DISP_SetPubAttr(DispChn, &stDevAttr);
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_disp_test_EnableDev(MI_DISP_DEV DispChn, MI_BOOL bEnable)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    if(bEnable)
    {
        s32Ret = MI_DISP_Enable(DispChn);
    }
    else
    {
        s32Ret = MI_DISP_Disable(DispChn);
    }
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_disp_test_SetVideoLayerAttr(MI_DISP_DEV DispChn, MI_DISP_VideoLayerAttr_t* pstLayerAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_DISP_SetVideoLayerAttr(DispChn, pstLayerAttr);
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_disp_test_SetVideoLayerDispWin(MI_DISP_DEV DispChn, MI_U16 u16X, MI_U16 u16Y, MI_U16 u16Width, MI_U16 u16Height)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    memset(&stLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
    s32Ret = MI_DISP_GetVideoLayerAttr(DispChn, &stLayerAttr);
    MIDISPCHECKRESULT(s32Ret);
    stLayerAttr.stVidLayerDispWin.u16X = u16X;
    stLayerAttr.stVidLayerDispWin.u16Y = u16Y;
    stLayerAttr.stVidLayerDispWin.u16Width = u16Width;
    stLayerAttr.stVidLayerDispWin.u16Height = u16Height;
    s32Ret = MI_DISP_SetVideoLayerAttr(DispChn, &stLayerAttr);
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_disp_test_SetVideoLayerSize(MI_DISP_DEV DispChn, MI_U16 u16Width, MI_U16 u16Height)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    memset(&stLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
    s32Ret = MI_DISP_GetVideoLayerAttr(DispChn, &stLayerAttr);
    MIDISPCHECKRESULT(s32Ret);
    stLayerAttr.stVidLayerSize.u16Width = u16Width;
    stLayerAttr.stVidLayerSize.u16Height = u16Height;
    s32Ret = MI_DISP_SetVideoLayerAttr(DispChn, &stLayerAttr);
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}


MI_S32 mi_disp_test_EnableVideoLayer(MI_DISP_LAYER VideoLayer, MI_BOOL bEnable)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    if(bEnable)
    {
        s32Ret = MI_DISP_EnableVideoLayer(VideoLayer);
    }
    else
    {
        s32Ret = MI_DISP_EnableVideoLayer(VideoLayer);
    }
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_disp_test_BindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev, MI_BOOL bBind)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    if(bBind)
    {
        s32Ret = MI_DISP_BindVideoLayer(DispLayer, DispDev);
    }
    else
    {
        s32Ret = MI_DISP_UnBindVideoLayer(DispLayer, DispDev);
    }
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_divp_test_SetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_InputPortAttr_t* pstPortAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, pstPortAttr);
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_divp_test_SetInputPortRect(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_U16 u16x, MI_U16 u16y, MI_U16 u16Width, MI_U16 u16height)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
    s32Ret = MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr);
    MIDISPCHECKRESULT(s32Ret);
    stInputPortAttr.stDispWin.u16X = u16x;
    stInputPortAttr.stDispWin.u16Y = u16y;
    stInputPortAttr.stDispWin.u16Width = u16Width;
    stInputPortAttr.stDispWin.u16Height = u16height;
    s32Ret = MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr);
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_disp_test_EnableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_BOOL bEnable)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    if(bEnable)
    {
        s32Ret = MI_DISP_EnableInputPort(DispLayer, LayerInputPort);
    }
    else
    {
        s32Ret = MI_DISP_DisableInputPort(DispLayer, LayerInputPort);
    }
    MIDISPCHECKRESULT(s32Ret);
    return s32Ret;
}

MI_S32 mi_disp_hdmiInit(void)
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
    stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
    stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
    MI_HDMI_SetAttr(eHdmi, &stAttr);

    MI_HDMI_Start(eHdmi), MI_SUCCESS;
    return MI_SUCCESS;
}

void test_disp_FdRewind(int srcFd)
{
    lseek(srcFd, 0, SEEK_SET);
}

MI_S32 test_disp_GetOneFrame(int srcFd, int offset, char *pData, int yuvSize)
{
    int size = 0;
    off_t current = lseek(srcFd,0L, SEEK_CUR);
    off_t end = lseek(srcFd,0L, SEEK_END);

    if ((end - current) < yuvSize)
    {
        return -1;
    }
    lseek(srcFd, current, SEEK_SET);
    if (read(srcFd, pData, yuvSize) < yuvSize)
    {
        return 0;
    }

    return 1;
}
#define trace() //printf("%s()@line %d.\n", __func__, __LINE__)
MI_S32 test_disp_GetOneFrameYUV420ByStride(int srcFd, char *pYData, char *pUvData, int ySize, int uvSize, int height, int width, int yStride, int uvStride)
{
    int size = 0;
    int i = 0;
    off_t current = lseek(srcFd,0L, SEEK_CUR);
    off_t end = lseek(srcFd,0L, SEEK_END);

    if ((end - current) < (ySize + uvSize))
    {
        trace();
        return -1;
    }
    lseek(srcFd, current, SEEK_SET);
#if 0
    if (read(srcFd, pYData, ySize) < ySize)
    {
        return 0;
    }
    else if (read(srcFd, pUvData, uvSize) < uvSize)
    {
        return 0;
    }
#endif

    for (i = 0; i < height; i++)
    {
        if (read(srcFd, pYData+ i * yStride, width) < width)
        {
            trace();

            return 0;
        }
    }

    for (i = 0; i < height/2; i++)
    {
        if (read(srcFd, pUvData+ i * uvStride, width) < width)
        {
            trace();

            return 0;
        }
    }

    return 1;
}


MI_S32 test_disp_GetOneYuv422Frame(int srcFd, int offset, char *pData, int width, int height, int yStride)
{
    int size = 0;
    int yuvSize = width*height*2;
    off_t current = lseek(srcFd,0L, SEEK_CUR);
    off_t end = lseek(srcFd,0L, SEEK_END);

    if ((end - current) < yuvSize)
    {
        //printf("end: %d, current: %d yuvSize: %d.\n", end, current, yuvSize);
        return -1;
    }
    lseek(srcFd, current, SEEK_SET);
    for (;height != 0; height--)
    {
        size +=read(srcFd, pData, width*2);
        pData += yStride;
    }

    if (size < yuvSize)
    {
        //printf("size: %d yuvSize: %d.\n", size, yuvSize);
        return 0;
    }
   // printf("size: %d yuvSize: %d.\n", size, yuvSize);

    return 1;
}

#define FILE_BLOCK (256)
MI_S32 test_disp_PutOneFrame(int dstFd, int offset, char *pDataFrame, int line_offset, int line_size, int lineNumber)
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
            if (yuvSize < FILE_BLOCK)
            {
                size = yuvSize;
            }
            else
            {
                size = FILE_BLOCK;
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

MI_BOOL test_disp_OpenSourceFile(const char *pFileName, int *pSrcFd)
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

void test_disp_CloseFd(int fd)
{
    close(fd);
}

void test_disp_ShowFrameInfo (const char *s, MI_SYS_FrameData_t *pstFrameInfo)
{
    DISP_TEST_INFO("%s %u x %u  stride: %u Phy: 0x%llx VirtualAddr: %p.\n", s, pstFrameInfo->u16Width, pstFrameInfo->u16Height,
        pstFrameInfo->u32Stride[0], pstFrameInfo->phyAddr[0], pstFrameInfo->pVirAddr[0]);
}

MI_S32 test_disp_CreatVpeChannel(MI_VPE_CHANNEL VpeChannel, MI_VPE_PORT VpePort, MI_SYS_WindowRect_t *pstCropWin, MI_SYS_WindowRect_t *pstDispWin)
{
    MI_VPE_ChannelAttr_t stChannelVpeAttr;
    MI_SYS_WindowRect_t stCropWin;
    stChannelVpeAttr.u16MaxW = 1920;
    stChannelVpeAttr.u16MaxH = 1080;
    stChannelVpeAttr.bNrEn= FALSE;
    stChannelVpeAttr.bEdgeEn= FALSE;
    stChannelVpeAttr.bEsEn= FALSE;
    stChannelVpeAttr.bContrastEn= FALSE;
    stChannelVpeAttr.bUvInvert= FALSE;
    stChannelVpeAttr.ePixFmt = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    ExecFunc(MI_VPE_CreateChannel(VpeChannel, &stChannelVpeAttr), MI_VPE_OK);

    ExecFunc(MI_VPE_GetChannelAttr(VpeChannel, &stChannelVpeAttr), MI_VPE_OK);

    stChannelVpeAttr.bContrastEn = FALSE;
    stChannelVpeAttr.bNrEn = FALSE;
    ExecFunc(MI_VPE_SetChannelAttr(VpeChannel, &stChannelVpeAttr), MI_VPE_OK);

    ExecFunc(MI_VPE_GetChannelCrop(VpeChannel, &stCropWin), MI_VPE_OK);
    stCropWin.u16X = pstCropWin->u16X;
    stCropWin.u16Y = pstCropWin->u16Y;
    stCropWin.u16Width = pstCropWin->u16Width;
    stCropWin.u16Height = pstCropWin->u16Height;
    ExecFunc(MI_VPE_SetChannelCrop(VpeChannel, &stCropWin), MI_VPE_OK);
    MI_VPE_PortMode_t stVpeMode;
    memset(&stVpeMode, 0, sizeof(stVpeMode));
    ExecFunc(MI_VPE_GetPortMode(VpeChannel, VpePort, &stVpeMode), MI_VPE_OK);
    stVpeMode.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stVpeMode.u16Width = pstDispWin->u16Width;
    stVpeMode.u16Height= pstDispWin->u16Height;
    ExecFunc(MI_VPE_SetPortMode(VpeChannel, VpePort, &stVpeMode), MI_VPE_OK);
    ExecFunc(MI_VPE_EnablePort(VpeChannel, VpePort), MI_VPE_OK);
    ExecFunc(MI_VPE_StartChannel (VpeChannel), MI_VPE_OK);
    return 0;
}

MI_S32 test_Disp_DestroyVpeChannel(MI_VPE_CHANNEL VpeChannel, MI_VPE_PORT VpePort)
{
    /*****************************/
    /*  call sys bind interface */
    /*****************************/
    ExecFunc(MI_VPE_StopChannel (VpeChannel), MI_VPE_OK);

    ExecFunc(MI_VPE_DisablePort(VpeChannel, VpePort), MI_VPE_OK);

    /*****************************/
    /*  call sys unbind interface */
    /*****************************/
    ExecFunc(MI_VPE_DestroyChannel(VpeChannel), MI_VPE_OK);
    return 0;
}

MI_S32 test_disp_vpeUnBinderDisp(MI_U32 VpeOutputPort, MI_U32 DispInputPort)
{
    //Bind VPE to DISP
     MI_SYS_ChnPort_t stSrcChnPort;
     MI_SYS_ChnPort_t stDstChnPort;
     MI_U32 u32SrcFrmrate;
     MI_U32 u32DstFrmrate;

     stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
     stSrcChnPort.u32DevId = 0;
     stSrcChnPort.u32ChnId = 0;
     stSrcChnPort.u32PortId = VpeOutputPort;

     stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
     stDstChnPort.u32DevId = 0;
     stDstChnPort.u32ChnId = 0;
     stDstChnPort.u32PortId = DispInputPort;

     ExecFunc(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort), MI_SUCCESS);
     return 0;
}

MI_S32 test_disp_vpeBinderDisp(MI_U32 VpeOutputPort, MI_U32 DispInputPort)
{
    //Bind VPE to DISP
     MI_SYS_ChnPort_t stSrcChnPort;
     MI_SYS_ChnPort_t stDstChnPort;
     MI_U32 u32SrcFrmrate;
     MI_U32 u32DstFrmrate;

     stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
     stSrcChnPort.u32DevId = 0;
     stSrcChnPort.u32ChnId = 0;
     stSrcChnPort.u32PortId = VpeOutputPort;

     stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
     stDstChnPort.u32DevId = 0;
     stDstChnPort.u32ChnId = 0;
     stDstChnPort.u32PortId = DispInputPort;

     u32SrcFrmrate = 30;
     u32DstFrmrate = 30;

     ExecFunc(MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate), MI_SUCCESS);
     return 0;
}

MI_S32 test_disp_divpBinderDisp(MI_U32 u32DivpOutputPort, MI_U32 DispInputPort)
{
    //Bind VPE to DISP
     MI_SYS_ChnPort_t stSrcChnPort;
     MI_SYS_ChnPort_t stDstChnPort;
     MI_U32 u32SrcFrmrate;
     MI_U32 u32DstFrmrate;

     stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
     stSrcChnPort.u32DevId = 0;
     stSrcChnPort.u32ChnId = 0;
     stSrcChnPort.u32PortId = u32DivpOutputPort;

     stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
     stDstChnPort.u32DevId = 0;
     stDstChnPort.u32ChnId = 0;
     stDstChnPort.u32PortId = DispInputPort;

     u32SrcFrmrate = 30;
     u32DstFrmrate = 30;

     ExecFunc(MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate), MI_SUCCESS);
     return 0;
}

MI_S32 test_disp_divpUnBinderDisp(MI_U32 u32DivpOutputPort, MI_U32 DispInputPort)
{
    //Bind VPE to DISP
     MI_SYS_ChnPort_t stSrcChnPort;
     MI_SYS_ChnPort_t stDstChnPort;
     MI_U32 u32SrcFrmrate;
     MI_U32 u32DstFrmrate;

     stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
     stSrcChnPort.u32DevId = 0;
     stSrcChnPort.u32ChnId = 0;
     stSrcChnPort.u32PortId = u32DivpOutputPort;

     stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
     stDstChnPort.u32DevId = 0;
     stDstChnPort.u32ChnId = 0;
     stDstChnPort.u32PortId = DispInputPort;

     u32SrcFrmrate = 30;
     u32DstFrmrate = 30;

     ExecFunc(MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate), MI_SUCCESS);
     return 0;
}
