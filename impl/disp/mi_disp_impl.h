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

#ifndef _MI_DISP_IMPL_H_
#define _MI_DISP_IMPL_H_
#include "mi_disp.h"

MI_S32 MI_DISP_IMPL_Enable(MI_DISP_DEV DispDev);
MI_S32 MI_DISP_IMPL_Disable(MI_DISP_DEV DispDev);
MI_S32 MI_DISP_IMPL_SetPubAttr(MI_DISP_DEV DispDev, const MI_DISP_PubAttr_t *pstPubAttr);
MI_S32 MI_DISP_IMPL_GetPubAttr(MI_DISP_DEV DispDev, MI_DISP_PubAttr_t *pstPubAttr);
MI_S32 MI_DISP_IMPL_DeviceAttach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev);
MI_S32 MI_DISP_IMPL_DeviceDetach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev);
MI_S32 MI_DISP_IMPL_EnableVideoLayer(MI_DISP_LAYER DispLayer);
MI_S32 MI_DISP_IMPL_DisableVideoLayer(MI_DISP_LAYER DispLayer);
MI_S32 MI_DISP_IMPL_GetVideoLayerCompressAttr(MI_DISP_LAYER DispLayer, MI_DISP_CompressAttr_t *pstCompressAttr);
MI_S32 MI_DISP_IMPL_SetVideoLayerAttr(MI_DISP_LAYER DispLayer, const MI_DISP_VideoLayerAttr_t *pstLayerAttr);
MI_S32 MI_DISP_IMPL_GetVideoLayerAttr(MI_DISP_LAYER DispLayer, MI_DISP_VideoLayerAttr_t *pstLayerAttr);
MI_S32 MI_DISP_IMPL_BindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev);
MI_S32 MI_DISP_IMPL_UnBindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev);
MI_S32 MI_DISP_IMPL_GetVideoLayerPriority(MI_DISP_LAYER DispLayer, MI_U32 *pu32Priority);
MI_S32 MI_DISP_IMPL_SetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 u32Toleration);
MI_S32 MI_DISP_IMPL_GetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 *pu32Toleration);
MI_S32 MI_DISP_IMPL_GetScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame);
MI_S32 MI_DISP_IMPL_ReleaseScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame);
MI_S32 MI_DISP_IMPL_SetVideoLayerAttrBegin(MI_DISP_LAYER DispLayer);
MI_S32 MI_DISP_IMPL_SetVideoLayerAttrEnd(MI_DISP_LAYER DispLayer);
MI_S32 MI_DISP_IMPL_SetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_InputPortAttr_t *pstInputPortAttr);
MI_S32 MI_DISP_IMPL_GetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_InputPortAttr_t *pstInputPortAttr);
MI_S32 MI_DISP_IMPL_EnableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_IMPL_DisableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_IMPL_SetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_Position_t *pstDispPos);
MI_S32 MI_DISP_IMPL_GetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_Position_t *pstDispPos);
MI_S32 MI_DISP_IMPL_PauseInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_IMPL_ResumeInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_IMPL_StepInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_IMPL_ShowInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_IMPL_HideInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_IMPL_SetInputPortSyncMode(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_SyncMode_e eMode);
MI_S32 MI_DISP_IMPL_QueryInputPortStat(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_QueryChannelStatus_t *pstStatus);
MI_S32 MI_DISP_IMPL_SetZoomInWindow(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_VidWinRect_t* pstCropWin);

MI_S32 MI_DISP_IMPL_GetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam);
MI_S32 MI_DISP_IMPL_SetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam);
MI_S32 MI_DISP_IMPL_GetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam);
MI_S32 MI_DISP_IMPL_SetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam);
MI_S32 MI_DISP_IMPL_GetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam);
MI_S32 MI_DISP_IMPL_SetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam);
MI_S32 MI_DISP_IMPL_DeInit(void);
MI_S32 MI_DISP_IMPL_Init(void);
MI_S32 mi_disp_impl_GetOutputTiming(MI_DISP_LAYER DispLayer, MI_DISP_OutputTiming_e* pOutputTiming, MI_DISP_SyncInfo_t* pstSyncInfo);
MI_S32 MI_DISP_IMPL_FrameFlip (MI_PHY phyAddr, MI_U32 u32Width, MI_U32 u32Height);
MI_S32 MI_DISP_IMPL_ClearInputPortBuffer(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_BOOL bClrAll);
#endif //_MI_DISP_IMPL_H_
