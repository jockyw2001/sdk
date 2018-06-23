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

#ifndef _MHAL_RGN_DATATYPE_H_
#define _MHAL_RGN_DATATYPE_H_


typedef enum
{
    E_MHAL_COVER_VPE_PORT0 = 0,
    E_MHAL_COVER_VPE_PORT1,
    E_MHAL_COVER_VPE_PORT2,
    E_MHAL_COVER_VPE_PORT3,
    E_MHAL_COVER_TYPE_MAX,
    E_MHAL_COVER_DIVP_PORT0
}MHAL_RGN_CoverType_e;

typedef enum
{
    E_MHAL_COVER_LAYER_0 = 0,
    E_MHAL_COVER_LAYER_1,
    E_MHAL_COVER_LAYER_2,
    E_MHAL_COVER_LAYER_3,
    E_MHAL_COVER_LAYER_MAX
}MHAL_RGN_CoverLayerId_e;

typedef enum
{
    E_MHAL_GOP_VPE_PORT0 = 0,
    E_MHAL_GOP_VPE_PORT1,
    E_MHAL_GOP_VPE_PORT2,
    E_MHAL_GOP_VPE_PORT3,
    E_MHAL_GOP_DIVP_PORT0,
    E_MHAL_GOP_TYPE_MAX
}MHAL_RGN_GopType_e;

typedef enum
{
    E_MHAL_RGN_VPE_CMDQ = 0,
    E_MHAL_RGN_DIVP_CMDQ,
    E_MHAL_RGN_CMDQ_TYPE_MAX
}MHAL_RGN_CmdqType_e;

//----------------------------------------------------------------------------
// GWin
//------------------------------------------------------------------------------
typedef enum
{
    E_MHAL_GOP_GWIN_ID_0 = 0,
    E_MHAL_GOP_GWIN_ID_1,
    E_MHAL_GOP_GWIN_ID_MAX
}MHAL_RGN_GopGwinId_e;

typedef enum
{
    E_MHAL_GOP_GWIN_ALPHA_CONSTANT,
    E_MHAL_GOP_GWIN_ALPHA_PIXEL,
    E_MHAL_GOP_GWIN_ALPHA_NUM,
} MHAL_RGN_GopGwinAlphaType_e;

typedef enum
{
    E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0 = 0,
    E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1 = 1,
} MHAL_RGN_GopGwinArgb1555Def_e;

//=============================================================================
// struct
//=============================================================================

typedef enum
{
    E_MHAL_RGN_PIXEL_FORMAT_ARGB1555 = 0,
    E_MHAL_RGN_PIXEL_FORMAT_ARGB4444,
    E_MHAL_RGN_PIXEL_FORMAT_I2,
    E_MHAL_RGN_PIXEL_FORMAT_I4,
	E_MHAL_RGN_PIXEL_FORMAT_I8,
	E_MHAL_RGN_PIXEL_FORMAT_RGB565,
	E_MHAL_RGN_PIXEL_FORMAT_ARGB8888,
	E_MHAL_RGN_PIXEL_FORMAT_UV8Y8,
    E_MHAL_RGN_PIXEL_FORMAT_MAX
}MHAL_RGN_GopPixelFormat_e;

typedef enum 
{
	E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,
	E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT, // pixels
	E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,  // pixels
	E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT, // bytes
	E_MHAL_RGN_CHIP_OSD_HW_GWIN_CNT,
	E_MHAL_RGN_CHIP_MAX
}MHAL_RGN_ChipCapType_e;

#endif //_MHAL_RGN_DATATYPE_H_
