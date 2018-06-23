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

#ifndef _MI_DIVP_DATATYPE_H_
#define _MI_DIVP_DATATYPE_H_
#pragma pack(push)
#pragma pack(4)

#define MI_DIVP_CHN_MAX_NUM (32)
#define MI_DIVP_OK (0x80000)
#define MI_DIVP_ERR_INVALID_DEVID (0x80001)
#define MI_DIVP_ERR_INVALID_CHNID (0x80002)
#define MI_DIVP_ERR_INVALID_PARAM (0x80003)
#define MI_DIVP_ERR_NULL_PTR (0x80004)
#define MI_DIVP_ERR_FAILED (0x80005)
#define MI_DIVP_ERR_CHN_NOT_STARTED (0x80006)
#define MI_DIVP_ERR_CHN_NOT_STOPED (0x80007)
#define MI_DIVP_ERR_CHN_NOT_SUPPORT (0x80008)
#define MI_DIVP_ERR_NO_RESOUCE (0x80009)
#define MI_DIVP_ERR_CHN_BUSY (0x800010)

typedef MI_U32 DIVP_CHN;

typedef enum
{
    E_MI_DIVP_DI_TYPE_OFF,//off
    E_MI_DIVP_DI_TYPE_2D,///2.5D DI
    E_MI_DIVP_DI_TYPE_3D,///3D DI
    E_MI_DIVP_DI_TYPE_NUM,
} MI_DIVP_DiType_e;

typedef enum
{
    E_MI_DIVP_TNR_LEVEL_OFF,
    E_MI_DIVP_TNR_LEVEL_LOW,
    E_MI_DIVP_TNR_LEVEL_MIDDLE,
    E_MI_DIVP_TNR_LEVEL_HIGH,
    E_MI_DIVP_TNR_LEVEL_NUM,
} MI_DIVP_TnrLevel_e;

typedef struct MI_DIVP_OutputPortAttr_s
{
    MI_U16 u16Width;//output width
    MI_U16 u16Height;//output height
    MI_SYS_PixelFormat_e ePixelFormat;//output pixel format
    MI_SYS_CompressMode_e eCompMode;//compress mode
}MI_DIVP_OutputPortAttr_t;

typedef struct MI_DIVP_ChnAttrParams_s
{
    MI_U32 u32MaxWidth;//support max input width
    MI_U32 u32MaxHeight;//support max input height
    MI_DIVP_TnrLevel_e eTnrLevel;//TNR level
    MI_DIVP_DiType_e eDiType;//DI type
    MI_SYS_Rotate_e eRotateType;//rotate angle
    MI_SYS_WindowRect_t stCropRect;//crop information
    MI_BOOL bHorMirror;//horizontal mirror
    MI_BOOL bVerMirror;//vertical mirror
}MI_DIVP_ChnAttrParams_t;


#pragma pack(pop)
#endif
