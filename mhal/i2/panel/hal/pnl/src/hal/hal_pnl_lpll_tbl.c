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
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
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

//****************************************************
// MIPI DSI Drive Chip           : INIT
// MIPI DSI Excel CodeGen Version: 1.09
// MIPI DSI Excel SW      Version: 1.08
// MIPI DSI Excel update date    : 2018/6/19 16:10
//****************************************************

#ifndef _HAL_PNL_LPLL_TBL_C_
#define _HAL_PNL_LPLL_TBL_C_

#include "cam_os_wrapper.h"
#include "hal_pnl_lpll_tbl.h"

//****************************************************
// INIT_LPLL  LVDS_1CH_1280x800
//****************************************************
u8 MST_PNL_LVDS_1CH_1280x800_INIT_LPLL_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_LVDS_1CH_1280x800_INIT_LPLL_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_40_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x03, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x30, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_H), 0x0F, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0x03, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0xF0, 0x70/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_L), 0x1F, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_H), 0x0F, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_H), 0x07, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x10, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x08, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x01, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x02, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x20, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x02, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x80, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL_DIV_GAIN  LVDS_1CH_1280x800
//****************************************************
u8 MST_PNL_LVDS_1CH_1280x800_INIT_LPLL_DIV_GAIN_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_LVDS_1CH_1280x800_INIT_LPLL_DIV_GAIN_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x0E/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL  720x1280_RGB888_4Lane
//****************************************************
u8 MST_PNL_720x1280_RGB888_4Lane_INIT_LPLL_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB888_4Lane_INIT_LPLL_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_40_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x03, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x30, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_H), 0x0F, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0x03, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0xF0, 0x30/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_L), 0x1F, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_H), 0x0F, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_H), 0x07, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x08, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x01, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x02, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x20, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x04, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x02, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x80, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL_DIV_GAIN  720x1280_RGB888_4Lane
//****************************************************
u8 MST_PNL_720x1280_RGB888_4Lane_INIT_LPLL_DIV_GAIN_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB888_4Lane_INIT_LPLL_DIV_GAIN_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x0C/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL  720x1280_RGB565_4Lane
//****************************************************
u8 MST_PNL_720x1280_RGB565_4Lane_INIT_LPLL_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB565_4Lane_INIT_LPLL_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_40_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x03, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x30, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_H), 0x0F, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0x03, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0xF0, 0x40/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_L), 0x1F, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_H), 0x0F, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x07, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_H), 0x07, 0x03/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x08, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x01, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x02, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x20, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x04, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x02, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x80, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL_DIV_GAIN  720x1280_RGB565_4Lane
//****************************************************
u8 MST_PNL_720x1280_RGB565_4Lane_INIT_LPLL_DIV_GAIN_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB565_4Lane_INIT_LPLL_DIV_GAIN_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL  720x1280_RGB666_4Lane
//****************************************************
u8 MST_PNL_720x1280_RGB666_4Lane_INIT_LPLL_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB666_4Lane_INIT_LPLL_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_40_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x03, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x30, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_H), 0x0F, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0x03, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0xF0, 0x90/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_L), 0x1F, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_H), 0x0F, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x07, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_H), 0x07, 0x03/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x08, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x01, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x02, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x20, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x04, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x02, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x80, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL_DIV_GAIN  720x1280_RGB666_4Lane
//****************************************************
u8 MST_PNL_720x1280_RGB666_4Lane_INIT_LPLL_DIV_GAIN_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB666_4Lane_INIT_LPLL_DIV_GAIN_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x12/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL  720x1280_RGB888_2Lane
//****************************************************
u8 MST_PNL_720x1280_RGB888_2Lane_INIT_LPLL_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB888_2Lane_INIT_LPLL_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_40_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x03, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x30, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_H), 0x0F, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0x03, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0xF0, 0x30/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_L), 0x1F, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_H), 0x0F, 0x05/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x07, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_H), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x08, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x01, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x02, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x20, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x04, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x02, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x80, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL_DIV_GAIN  720x1280_RGB888_2Lane
//****************************************************
u8 MST_PNL_720x1280_RGB888_2Lane_INIT_LPLL_DIV_GAIN_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB888_2Lane_INIT_LPLL_DIV_GAIN_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x0C/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL  720x1280_RGB565_2Lane
//****************************************************
u8 MST_PNL_720x1280_RGB565_2Lane_INIT_LPLL_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB565_2Lane_INIT_LPLL_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_40_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x03, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x30, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_H), 0x0F, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0x03, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0xF0, 0x40/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_L), 0x1F, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_H), 0x0F, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_H), 0x07, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x08, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x01, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x02, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x20, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x04, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x02, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x80, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL_DIV_GAIN  720x1280_RGB565_2Lane
//****************************************************
u8 MST_PNL_720x1280_RGB565_2Lane_INIT_LPLL_DIV_GAIN_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB565_2Lane_INIT_LPLL_DIV_GAIN_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL  720x1280_RGB666_2Lane
//****************************************************
u8 MST_PNL_720x1280_RGB666_2Lane_INIT_LPLL_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB666_2Lane_INIT_LPLL_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_40_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x03, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x30, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_H), 0x0F, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0x03, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_L), 0xF0, 0x90/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_L), 0x1F, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_44_H), 0x0F, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x07, 0x01/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_42_H), 0x07, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_43_L), 0x10, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x08, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x01, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_41_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x02, 0x02/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x20, 0x20/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x04, 0x04/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x02, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_4B_L), 0x04, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_40_H), 0x80, 0x00/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

//****************************************************
// INIT_LPLL_DIV_GAIN  720x1280_RGB666_2Lane
//****************************************************
u8 MST_PNL_720x1280_RGB666_2Lane_INIT_LPLL_DIV_GAIN_TBL[][REG_ADDR_SIZE+REG_MASK_SIZE+PNL_TAB_720x1280_RGB666_2Lane_INIT_LPLL_DIV_GAIN_NUMS]=
{                 // Reg           Mask  Value
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x10/*ALL*/, },
 { DRV_PNL_REG(REG_TC_LPLL_7F_L), 0xFF, 0x12/*ALL*/, },
 { DRV_PNL_REG(REG_TABLE_END)   , 0x00, 0x00,  }
};

PNL_TAB_INFO MipiDsiPanel_MainTbl[PNL_PANEL_NUMS]=
{
{
*MST_PNL_LVDS_1CH_1280x800_INIT_LPLL_TBL, PNL_TABTYPE_INIT_LPLL,
*MST_PNL_LVDS_1CH_1280x800_INIT_LPLL_DIV_GAIN_TBL, PNL_TABTYPE_INIT_LPLL_DIV_GAIN,
},
{
*MST_PNL_720x1280_RGB888_4Lane_INIT_LPLL_TBL, PNL_TABTYPE_INIT_LPLL,
*MST_PNL_720x1280_RGB888_4Lane_INIT_LPLL_DIV_GAIN_TBL, PNL_TABTYPE_INIT_LPLL_DIV_GAIN,
},
{
*MST_PNL_720x1280_RGB565_4Lane_INIT_LPLL_TBL, PNL_TABTYPE_INIT_LPLL,
*MST_PNL_720x1280_RGB565_4Lane_INIT_LPLL_DIV_GAIN_TBL, PNL_TABTYPE_INIT_LPLL_DIV_GAIN,
},
{
*MST_PNL_720x1280_RGB666_4Lane_INIT_LPLL_TBL, PNL_TABTYPE_INIT_LPLL,
*MST_PNL_720x1280_RGB666_4Lane_INIT_LPLL_DIV_GAIN_TBL, PNL_TABTYPE_INIT_LPLL_DIV_GAIN,
},
{
*MST_PNL_720x1280_RGB888_2Lane_INIT_LPLL_TBL, PNL_TABTYPE_INIT_LPLL,
*MST_PNL_720x1280_RGB888_2Lane_INIT_LPLL_DIV_GAIN_TBL, PNL_TABTYPE_INIT_LPLL_DIV_GAIN,
},
{
*MST_PNL_720x1280_RGB565_2Lane_INIT_LPLL_TBL, PNL_TABTYPE_INIT_LPLL,
*MST_PNL_720x1280_RGB565_2Lane_INIT_LPLL_DIV_GAIN_TBL, PNL_TABTYPE_INIT_LPLL_DIV_GAIN,
},
{
*MST_PNL_720x1280_RGB666_2Lane_INIT_LPLL_TBL, PNL_TABTYPE_INIT_LPLL,
*MST_PNL_720x1280_RGB666_2Lane_INIT_LPLL_DIV_GAIN_TBL, PNL_TABTYPE_INIT_LPLL_DIV_GAIN,
},
};

#endif
