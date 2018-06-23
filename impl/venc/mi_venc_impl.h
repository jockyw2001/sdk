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

#ifndef _MI_VENC_IMPL_H_
#define _MI_VENC_IMPL_H_
#include "mi_venc_datatype.h"

MI_S32 MI_VENC_IMPL_SetModParam(MI_VENC_ModParam_t *pstModParam);
MI_S32 MI_VENC_IMPL_GetModParam(MI_VENC_ModParam_t *pstModParam);
MI_S32 MI_VENC_IMPL_CreateChn(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr);
MI_S32 MI_VENC_IMPL_DestroyChn(MI_VENC_CHN VeChn);
MI_S32 MI_VENC_IMPL_ResetChn(MI_VENC_CHN VeChn);
MI_S32 MI_VENC_IMPL_StartRecvPic(MI_VENC_CHN VeChn);
MI_S32 MI_VENC_IMPL_StartRecvPicEx(MI_VENC_CHN VeChn, MI_VENC_RecvPicParam_t *pstRecvParam);
MI_S32 MI_VENC_IMPL_StopRecvPic(MI_VENC_CHN VeChn);
MI_S32 MI_VENC_IMPL_Query(MI_VENC_CHN VeChn, MI_VENC_ChnStat_t *pstStat);
MI_S32 MI_VENC_IMPL_SetChnAttr(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t* pstAttr);
MI_S32 MI_VENC_IMPL_GetChnAttr(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t*pstAttr);
MI_S32 MI_VENC_IMPL_GetStream(MI_VENC_CHN VeChn, MI_VENC_Stream_t *pstStream,MI_S32 s32MilliSec);
MI_S32 MI_VENC_IMPL_ReleaseStream(MI_VENC_CHN VeChn, MI_VENC_Stream_t *pstStream);
MI_S32 MI_VENC_IMPL_InsertUserData(MI_VENC_CHN VeChn, MI_U8 *pu8Data, MI_U32 u32Len);
MI_S32 MI_VENC_IMPL_SetMaxStreamCnt(MI_VENC_CHN VeChn,MI_U32 u32MaxStrmCnt);
MI_S32 MI_VENC_IMPL_GetMaxStreamCnt(MI_VENC_CHN VeChn,MI_U32 *pu32MaxStrmCnt);
MI_S32 MI_VENC_IMPL_RequestIdr(MI_VENC_CHN VeChn, MI_BOOL bInstant);
MI_S32 MI_VENC_IMPL_EnableIdr(MI_VENC_CHN VeChn, MI_BOOL bEnableIdr);
MI_S32 MI_VENC_IMPL_SetH264IdrPicId(MI_VENC_CHN VeChn, MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg);
MI_S32 MI_VENC_IMPL_GetH264IdrPicId( MI_VENC_CHN VeChn, MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg );
MI_S32 MI_VENC_IMPL_GetFd(MI_VENC_CHN VeChn);
MI_S32 MI_VENC_IMPL_CloseFd(MI_VENC_CHN VeChn);
MI_S32 MI_VENC_IMPL_SetRoiCfg(MI_VENC_CHN VeChn, MI_VENC_RoiCfg_t *pstVencRoiCfg);
MI_S32 MI_VENC_IMPL_GetRoiCfg(MI_VENC_CHN VeChn, MI_U32 u32Index, MI_VENC_RoiCfg_t *pstVencRoiCfg);
MI_S32 MI_VENC_IMPL_SetRoiBgFrameRate(MI_VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t * pstRoiBgFrmRate);
MI_S32 MI_VENC_IMPL_GetRoiBgFrameRate(MI_VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t *pstRoiBgFrmRate);
MI_S32 MI_VENC_IMPL_SetH264SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH264SliceSplit_t *pstSliceSplit);
MI_S32 MI_VENC_IMPL_GetH264SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH264SliceSplit_t *pstSliceSplit);
MI_S32 MI_VENC_IMPL_SetH264InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264InterPred_t *pstH264InterPred);
MI_S32 MI_VENC_IMPL_GetH264InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264InterPred_t *pstH264InterPred);
MI_S32 MI_VENC_IMPL_SetH264IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264IntraPred_t *pstH264IntraPred);
MI_S32 MI_VENC_IMPL_GetH264IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264IntraPred_t *pstH264IntraPred);
MI_S32 MI_VENC_IMPL_SetH264Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans);
MI_S32 MI_VENC_IMPL_GetH264Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans);
MI_S32 MI_VENC_IMPL_SetH264Entropy(MI_VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc);
MI_S32 MI_VENC_IMPL_GetH264Entropy(MI_VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc);
MI_S32 MI_VENC_IMPL_SetH264Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk);
MI_S32 MI_VENC_IMPL_GetH264Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk);
MI_S32 MI_VENC_IMPL_SetH264Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH264Vui_t*pstH264Vui);
MI_S32 MI_VENC_IMPL_GetH264Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH264Vui_t *pstH264Vui);
MI_S32 MI_VENC_IMPL_SetH265SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit);
MI_S32 MI_VENC_IMPL_GetH265SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit);
MI_S32 MI_VENC_IMPL_SetH265InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265InterPred_t *pstH265InterPred);
MI_S32 MI_VENC_IMPL_GetH265InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265InterPred_t *pstH265InterPred);
MI_S32 MI_VENC_IMPL_SetH265IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265IntraPred_t *pstH265IntraPred);
MI_S32 MI_VENC_IMPL_GetH265IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265IntraPred_t *pstH265IntraPred);
MI_S32 MI_VENC_IMPL_SetH265Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH265Trans_t *pstH265Trans);
MI_S32 MI_VENC_IMPL_GetH265Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH265Trans_t *pstH265Trans);
MI_S32 MI_VENC_IMPL_SetH265Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH265Dblk_t *pstH265Dblk);
MI_S32 MI_VENC_IMPL_GetH265Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH265Dblk_t *pstH265Dblk);
MI_S32 MI_VENC_IMPL_SetH265Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH265Vui_t*pstH265Vui);
MI_S32 MI_VENC_IMPL_GetH265Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH265Vui_t *pstH265Vui);
MI_S32 MI_VENC_IMPL_SetJpegParam(MI_VENC_CHN VeChn, MI_VENC_ParamJpeg_t *pstJpegParam);
MI_S32 MI_VENC_IMPL_GetJpegParam(MI_VENC_CHN VeChn, MI_VENC_ParamJpeg_t *pstJpegParam);
MI_S32 MI_VENC_IMPL_SetRcParam(MI_VENC_CHN VeChn,MI_VENC_RcParam_t *pstRcParam);
MI_S32 MI_VENC_IMPL_GetRcParam(MI_VENC_CHN VeChn, MI_VENC_RcParam_t *pstRcParam);
MI_S32 MI_VENC_IMPL_SetRefParam(MI_VENC_CHN VeChn,MI_VENC_ParamRef_t * pstRefParam);
MI_S32 MI_VENC_IMPL_GetRefParam(MI_VENC_CHN VeChn, MI_VENC_ParamRef_t * pstRefParam);
MI_S32 MI_VENC_IMPL_SetCrop(MI_VENC_CHN VeChn, MI_VENC_CropCfg_t *pstCropCfg);
MI_S32 MI_VENC_IMPL_GetCrop(MI_VENC_CHN VeChn, MI_VENC_CropCfg_t *pstCropCfg);
MI_S32 MI_VENC_IMPL_SetFrameLostStrategy(MI_VENC_CHN VeChn, MI_VENC_ParamFrameLost_t *pstFrmLostParam);
MI_S32 MI_VENC_IMPL_GetFrameLostStrategy(MI_VENC_CHN VeChn, MI_VENC_ParamFrameLost_t *pstFrmLostParam);
MI_S32 MI_VENC_IMPL_SetSuperFrameCfg(MI_VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam);
MI_S32 MI_VENC_IMPL_GetSuperFrameCfg(MI_VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam);
MI_S32 MI_VENC_IMPL_SetRcPriority(MI_VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority);
MI_S32 MI_VENC_IMPL_GetRcPriority(MI_VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority);
MI_S32 MI_VENC_IMPL_GetChnDevid(MI_VENC_CHN VeChn, MI_U32 *pu32Devid);

#endif //_MI_VENC_IMPL_H_
