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


#ifndef _RGN_IOCTL_H_
#define _RGN_IOCTL_H_
#include <linux/ioctl.h>
#include "mi_rgn.h"

typedef enum
{
    E_MI_RGN_CMD_INIT,
    E_MI_RGN_CMD_DE_INIT,
    E_MI_RGN_CMD_CREATE,
    E_MI_RGN_CMD_DESTROY,
    E_MI_RGN_CMD_GET_ATTR,
    E_MI_RGN_CMD_SET_BIT_MAP,
    E_MI_RGN_CMD_ATTACH_TO_CHN,
    E_MI_RGN_CMD_DETACH_FROM_CHN,
    E_MI_RGN_CMD_SET_DISPLAY_ATTR,
    E_MI_RGN_CMD_GET_DISPLAY_ATTR,
    E_MI_RGN_CMD_GET_CANVAS_INFO,
    E_MI_RGN_CMD_UPDATE_CANVAS,
    E_MI_RGN_CMD_MAX,
} MI_RGN_Cmd_e;

typedef struct MI_RGN_CreatePara_s
{
    MI_RGN_HANDLE hHandle;
    MI_RGN_Attr_t stRegion;
} MI_RGN_CreatePara_t;


typedef struct MI_RGN_SetBitMapPara_s
{
    MI_RGN_HANDLE hHandle;
    MI_RGN_Bitmap_t stBitmap;
} MI_RGN_SetBitMapPara_t;

typedef struct MI_RGN_AttachToChnPara_s
{
    MI_RGN_HANDLE hHandle;
    MI_RGN_ChnPort_t stChnPort;
    MI_RGN_ChnPortParam_t stChnAttr;
} MI_RGN_AttachToChnPara_t;

typedef struct MI_RGN_DetachFromChnPara_s
{
    MI_RGN_HANDLE hHandle;
    MI_RGN_ChnPort_t stChnPort;
} MI_RGN_DetachFromChnPara_t;


typedef struct MI_RGN_GetCanvasInfoPara_s
{
    MI_RGN_HANDLE hHandle;
    MI_RGN_CanvasInfo_t stCanvasInfo;
} MI_RGN_GetCanvasInfoPara_t;

#define MI_RGN_INIT _IOW('i', E_MI_RGN_CMD_INIT, MI_RGN_PaletteTable_t)
#define MI_RGN_DE_INIT _IO('i', E_MI_RGN_CMD_DE_INIT)
#define MI_RGN_CREATE _IOW('i', E_MI_RGN_CMD_CREATE, MI_RGN_CreatePara_t)
#define MI_RGN_DESTROY _IOW('i', E_MI_RGN_CMD_DESTROY, MI_RGN_HANDLE)
#define MI_RGN_GET_ATTR _IOWR('i', E_MI_RGN_CMD_GET_ATTR, MI_RGN_CreatePara_t)
#define MI_RGN_SET_BIT_MAP _IOW('i', E_MI_RGN_CMD_SET_BIT_MAP, MI_RGN_SetBitMapPara_t)
#define MI_RGN_ATTACH_TO_CHN _IOW('i', E_MI_RGN_CMD_ATTACH_TO_CHN, MI_RGN_AttachToChnPara_t)
#define MI_RGN_DETACH_FROM_CHN _IOW('i', E_MI_RGN_CMD_DETACH_FROM_CHN, MI_RGN_DetachFromChnPara_t)
#define MI_RGN_SET_DISPLAY_ATTR _IOW('i', E_MI_RGN_CMD_SET_DISPLAY_ATTR, MI_RGN_AttachToChnPara_t)
#define MI_RGN_GET_DISPLAY_ATTR _IOWR('i', E_MI_RGN_CMD_GET_DISPLAY_ATTR, MI_RGN_AttachToChnPara_t)
#define MI_RGN_GET_CANVAS_INFO _IOWR('i', E_MI_RGN_CMD_GET_CANVAS_INFO, MI_RGN_GetCanvasInfoPara_t)
#define MI_RGN_UPDATE_CANVAS _IOW('i', E_MI_RGN_CMD_UPDATE_CANVAS, MI_RGN_HANDLE)
#endif /// _RGN_IOCTL_H_
