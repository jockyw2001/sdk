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
// By accessing, browsing and/or usin
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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file   regAKL.h
/// @brief  AKL Register
/// @author MStar Semiconductor,Inc.
/// @attention
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __REG_AKL_H__
#define __REG_AKL_H__


#include "MsTypes.h"

#define REG_AKL_KS_INDEX_MAX   (3UL)
#define REG_AKL_KT_INDEX_MAX   (128UL)

#define REG_AKL0_BANK (0x1A19UL)

#define REG_AKL0_MACK     (0x3UL)
#define REG_AKL0_MACK_SFT (0UL)
#define REG_AKL0_MACK_MSK (0x01UL<<REG_AKL0_MACK_SFT)

#define REG_AKL0_MACK_En     (0x3UL)
#define REG_AKL0_MACK_En_SFT (8UL)
#define REG_AKL0_MACK_En_MSK (0x01UL<<REG_AKL0_MACK_En_SFT)

#define REG_AKL0_VALID (0x3UL)
#define REG_AKL0_VALID_SFT (16UL)
#define REG_AKL0_VALID_MSK (0x1UL<<REG_AKL0_VALID_SFT)

#define REG_AKL0_KT_GO (0x4UL)
#define REG_AKL0_KT_GO_SFT (0UL)
#define REG_AKL0_KT_GO_MSK (0x01UL<<REG_AKL0_KT_GO_SFT)

#define REG_AKL0_TSIO_GO (0x4UL)
#define REG_AKL0_TSIO_GO_SFT (1UL)
#define REG_AKL0_TSIO_GO_MSK (0x01UL<<REG_AKL0_TSIO_GO_SFT)

#define REG_AKL0_SCB (0x4UL)
#define REG_AKL0_SCB_SFT (12UL)
#define REG_AKL0_SCB_MSK (0x03UL<<REG_AKL0_SCB_SFT)

#define REG_AKL0_FIELD (0x4UL)
#define REG_AKL0_FIELD_SFT (16UL)
#define REG_AKL0_FIELD_MSK (0x03UL<<REG_AKL0_FIELD_SFT)

#define REG_AKL0_PIDNO (0x4UL)
#define REG_AKL0_PIDNO_SFT (20UL)
#define REG_AKL0_PIDNO_MSK (0xFFUL<<REG_AKL0_PIDNO_SFT)

#define REG_AKL0_DMA_GO (0x5UL)
#define REG_AKL0_DMA_GO_SFT (0UL)
#define REG_AKL0_DMA_GO_MSK (0x01UL<<REG_AKL0_DMA_GO_SFT)

#define REG_AKL0_DMAIdx (0x4UL)
#define REG_AKL0_DMAIdx_SFT (8UL)
#define REG_AKL0_DMAIdx_MSK (0x0FUL<<REG_AKL0_DMAIdx_SFT)

#define REG_AKL0_RSTN (0x6UL)
#define REG_AKL0_RSTN_SFT (0UL)
#define REG_AKL0_RSTN_MSK (0x01UL<<REG_AKL0_RSTN_SFT)
#define REG_AKL0_ND_RSTN_SFT (4UL)
#define REG_AKL0_ND_RSTN_MSK (0x01UL<<REG_AKL0_ND_RSTN_SFT)

#define REG_AKL0_RESP (0x7UL)
#define REG_AKL0_RESP_SFT (0UL)
#define REG_AKL0_RESP_MSK (0x1FUL<<REG_AKL0_RSTN_SFT)

#define REG_AKL0_ACK (0x7UL)
#define REG_AKL0_ACK_SFT (8UL)
#define REG_AKL0_ACK_MSK (0x01UL<<REG_AKL0_ACK_SFT)

#define REG_AKL0_DMA_ACK (0x7UL)
#define REG_AKL0_DMA_ACK_SFT (12UL)
#define REG_AKL0_DMA_ACK_MSK (0x01UL<<REG_AKL0_DMA_ACK_SFT)

#define REG_AKL0_TSIO_ACK (0x7UL)
#define REG_AKL0_TSIO_ACK_SFT (13UL)
#define REG_AKL0_TSIO_ACK_MSK (0x01UL<<REG_AKL0_TSIO_ACK_SFT)

#define REG_AKL0_DONE (0xAUL)
#define REG_AKL0_DONE_SFT (0UL)
#define REG_AKL0_DONE_MSK (0x01UL<<REG_AKL0_DONE_SFT)

#define REG_AKL0_DMA_DONE (0xAUL)
#define REG_AKL0_DMA_DONE_SFT (1UL)
#define REG_AKL0_DMA_DONE_MSK (0x01UL<<REG_AKL0_DMA_DONE_SFT)

#define REG_AKL0_TSIO_DONE (0xAUL)
#define REG_AKL0_TSIO_DONE_SFT (2UL)
#define REG_AKL0_TSIO_DONE_MSK (0x01UL<<REG_AKL0_TSIO_DONE_SFT)

//------------CERT COMMAND------------
#define REG_AKL1_BANK (0x1A1AUL)

#define REG_AKL1_INPUT    (0x00UL)
#define REG_AKL1_OUTPUT   (0x08UL)
#define REG_AKL1_STATUS   (0x10UL)
#define REG_AKL1_COMMAND  (0x11UL)

#endif // #ifndef __REG_AKL_H__

