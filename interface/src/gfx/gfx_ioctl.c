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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   vdec_ioctl.c
/// @brief vdec module ioctl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_gfx_impl.h"
#include "gfx_ioctl.h"
#include "mi_gfx_internal.h"
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
static DEFINE_MUTEX(mutex);

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
static MI_S32 MI_GFX_IOCTL_Open(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_GFX_IMPL_Open();

    return s32Ret;
}

static MI_S32 MI_GFX_IOCTL_Close(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_GFX_IMPL_Close();

    return s32Ret;
}

static MI_S32 MI_GFX_IOCTL_PendingDone(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U16 *arg = ptr;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_GFX_IMPL_PendingDone(*arg);

    return s32Ret;
}

static MI_S32 MI_GFX_IOCTL_WaitAllDone(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_GFX_WaitAllDone_t *arg = ptr;
    MI_BOOL bWaitAllDone = arg->bWaitAllDone;
    MI_U16 u16TargetFence = arg->u16TargetFence;

    AUTO_LOCK(env, mutex);

    s32Ret = MI_GFX_IMPL_WaitAllDone(bWaitAllDone, u16TargetFence, &mutex);

    return s32Ret;
}

MI_S32 MI_GFX_IOCTL_QuickFill(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_GFX_QuickFill_t *arg = ptr;
    MI_GFX_Surface_t *pstDst = &(arg->stDst);
    MI_GFX_Rect_t *pstDstRect = &(arg->stDstRect);
    MI_U32 u32ColorVal = arg->u32ColorVal;
    MI_U16 *pu16Fence = &(arg->u16Fence);
    MI_U16 u16Fence = 0;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_GFX_IMPL_QuickFill(pstDst, pstDstRect, u32ColorVal, &u16Fence);
    if (MI_SUCCESS == s32Ret)
    {
        *pu16Fence = u16Fence;
    }

    return s32Ret;
}

MI_S32 MI_GFX_IOCTL_GetAlphaThresholdValue(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8ThresholdValue = ptr;
    MI_U8 u8ThresholdValue = 1;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_GFX_IMPL_GetAlphaThresholdValue(&u8ThresholdValue);
    if (MI_SUCCESS == s32Ret)
    {
        *pu8ThresholdValue = u8ThresholdValue;
    }

    return s32Ret;
}

MI_S32 MI_GFX_IOCTL_SetAlphaThresholdValue(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8ThresholdValue = ptr;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_GFX_IMPL_SetAlphaThresholdValue(*pu8ThresholdValue);

    return s32Ret;
}

static MI_S32 MI_GFX_IOCTL_BitBlit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_GFX_BitBlit_t *arg = ptr;
    MI_GFX_Surface_t *pstSrc = &(arg->stSrc);
    MI_GFX_Rect_t *pstSrcRect = &(arg->stSrcRect);
    MI_GFX_Surface_t *pstDst = &(arg->stDst);
    MI_GFX_Rect_t *pstDstRect = &(arg->stDstRect);
    MI_GFX_Opt_t *pstOpt = &(arg->stOpt);
    MI_U16 *pu16Fence = &(arg->u16Fence);
    MI_U16 u16Fence = 0;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_GFX_IMPL_BitBlit(pstSrc, pstSrcRect, pstDst, pstDstRect, pstOpt, &u16Fence);
    if (MI_SUCCESS == s32Ret)
    {
        *pu16Fence = u16Fence;
    }

    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_GFX_CMD_MAX] = {
    [E_MI_GFX_CMD_OPEN_DEVICE] = MI_GFX_IOCTL_Open,
    [E_MI_GFX_CMD_CLOSE_DEVICE] = MI_GFX_IOCTL_Close,
    [E_MI_GFX_CMD_PENDING_DONE] = MI_GFX_IOCTL_PendingDone,
    [E_MI_GFX_CMD_WAIT_ALL_DONE] = MI_GFX_IOCTL_WaitAllDone,
    [E_MI_GFX_CMD_QUICK_FILL] = MI_GFX_IOCTL_QuickFill,
    [E_MI_GFX_CMD_GET_ALPHA_THRESHOLD_VAL] = MI_GFX_IOCTL_GetAlphaThresholdValue,
    [E_MI_GFX_CMD_SET_ALPHA_THRESHOLD_VAL] = MI_GFX_IOCTL_SetAlphaThresholdValue,
    [E_MI_GFX_CMD_BITBLIT] = MI_GFX_IOCTL_BitBlit,
};

static void mi_gfx_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
}

static void mi_gfx_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
}

static void mi_gfx_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");

MI_DEVICE_DEFINE(mi_gfx_insmod,
               ioctl_table, E_MI_GFX_CMD_MAX,
               mi_gfx_process_init, mi_gfx_process_exit);
