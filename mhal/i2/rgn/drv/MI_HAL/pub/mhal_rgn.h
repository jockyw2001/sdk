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

////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _MHAL_RGN_
#define _MHAL_RGN_

#include "mhal_common.h"
#include "mhal_cmdq.h"

#include "mhal_rgn_datatype.h"

/*Setup cmdq*/
MS_S32 MHAL_RGN_SetupCmdQ(MHAL_CMDQ_CmdqInterface_t* pstCmdInf,MHAL_RGN_CmdqType_e eRgnCmdqId);
/*Cover init*/
MS_S32 MHAL_RGN_CoverInit(void);
/*Set cover color NA:[31~24] Cr:[23~16] Y:[15~8] Cb:[7~0]*/
MS_S32 MHAL_RGN_CoverSetColor(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer, MS_U32 u32Color);
/*Cover  size*/
MS_S32 MHAL_RGN_CoverSetWindow(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer, MS_U32 u32X, MS_U32 u32Y, MS_U32 u32Width, MS_U32 u32Height);
/*Enable cover*/
MS_S32 MHAL_RGN_CoverEnable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer);
/*Disable cover*/
MS_S32 MHAL_RGN_CoverDisable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer);

/*Gop init*/
MS_S32 MHAL_RGN_GopInit(void);
/*Set palette*/
MS_S32 MHAL_RGN_GopSetPalette(MHAL_RGN_GopType_e eGopId, MS_U8 u8Alpha, MS_U8 u8Red, MS_U8 u8Green, MS_U8 u8Blue, MS_U8 u8Idx);
/*Set base gop window size*/
MS_S32 MHAL_RGN_GopSetBaseWindow(MHAL_RGN_GopType_e eGopId, MS_U32 u32Width, MS_U32 u32Height);
/*Set base gop gwin pix format*/
MS_S32 MHAL_RGN_GopGwinSetPixelFormat(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopPixelFormat_e eFormat);
/*Set base gop gwin osd window*/
MS_S32 MHAL_RGN_GopGwinSetWindow(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_U32 u32Width, MS_U32 u32Height, MS_U32 u32Stride, MS_U32 u32X, MS_U32 u32Y);
/*Set base gop gwin osd bufdfer addr*/
MS_S32 MHAL_RGN_GopGwinSetBuffer(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_PHYADDR phyAddr);
/*enable Gwin*/
MS_S32 MHAL_RGN_GopGwinEnable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId);
/*disable Gwin*/
MS_S32 MHAL_RGN_GopGwinDisable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId);
/*Set gop colorkey*/
MS_S32 MHAL_RGN_GopSetColorkey(MHAL_RGN_GopType_e eGopId, MS_BOOL bEn, MS_U8 u8R, MS_U8 u8G, MS_U8 u8B);
/*Set gop alpha blending type*/
MS_S32 MHAL_RGN_GopSetAlphaType(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinAlphaType_e eAlphaType, MS_U8 u8ConstAlphaVal);
/*Set gop ARGB1555 alpha value for alpha0 or alpha1*/
MS_S32 MHAL_RGN_GopSetArgb1555AlphaVal(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinArgb1555Def_e eAlphaType, MS_U8 u8AlphaVal);
//------------------------------------------------------------------------------
/// @brief: Get Chip Capability
/// @param[in]  eType : reference to MHAL_RGN_ChipCapType_e
///             eFormat: Color format
/// @return corresponding capability to type
//------------------------------------------------------------------------------
MS_S32 MHAL_RGN_GetChipCapability(MHAL_RGN_ChipCapType_e eType, MHAL_RGN_GopPixelFormat_e eFormat);

#endif //_MHAL_RGN_
