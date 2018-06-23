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
/// @file   vdec_api.c
/// @brief vdec module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "mi_syscall.h"
#include "mi_print.h"

#include "mi_gfx.h"
#include "gfx_ioctl.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
MI_MODULE_DEFINE(gfx);

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
/*-------------------------------------------------------------------------------------------
 * @brief Init GFX module.
 * @return MI_OK: Process success.
 * @return MI_HAS_INITED: Module has inited.
 * @return MI_ERR_FAILED: Process fail..
 ------------------------------------------------------------------------------------------*/
MI_S32 MI_GFX_Open(void)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL_VOID(MI_GFX_OPEN);

    return s32Ret;
}

MI_S32 MI_GFX_Close(void)
{
	MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL_VOID(MI_GFX_CLOSE);

    return s32Ret;
}

MI_S32 MI_GFX_WaitAllDone(MI_BOOL bWaitAllDone, MI_U16 u16TargetFence)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_GFX_WaitAllDone_t stWait;
    stWait.bWaitAllDone = bWaitAllDone;
    stWait.u16TargetFence = u16TargetFence;
    s32Ret = MI_SYSCALL(MI_GFX_WAIT_ALL_DONE, &stWait);

    return s32Ret;
}

MI_S32 MI_GFX_QuickFill(MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect,
    MI_U32 u32ColorVal, MI_U16 *pu16Fence)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_GFX_QuickFill_t stQuickFill;

    if (NULL == pstDst)
    {
        DBG_ERR("[%s][%d]MI_GFX_QuickFill param invaild!\n", __FILE__, __LINE__);
        return MI_ERR_GFX_INVALID_PARAM;
    }
    if (NULL == pstDstRect)
    {
        stQuickFill.stDstRect.s32Xpos = 0;
        stQuickFill.stDstRect.s32Ypos = 0;
        stQuickFill.stDstRect.u32Width = pstDst->u32Width;
        stQuickFill.stDstRect.u32Height = pstDst->u32Height;
    }
    else
    {
        memcpy(&stQuickFill.stDstRect, pstDstRect, sizeof(MI_GFX_Rect_t));
    }
    memcpy(&stQuickFill.stDst, pstDst, sizeof(MI_GFX_Surface_t));
    stQuickFill.u32ColorVal = u32ColorVal;
    s32Ret = MI_SYSCALL(MI_GFX_QUICK_FILL, &stQuickFill);
    if ((MI_SUCCESS == s32Ret) && (NULL != pu16Fence))
    {
        *pu16Fence = stQuickFill.u16Fence;
    }

    return s32Ret;
}

MI_S32 MI_GFX_GetAlphaThresholdValue(MI_U8 *pu8ThresholdValue)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 u8ThresholdValue = 1; //default value

    if (NULL == pu8ThresholdValue)
    {
        DBG_ERR("[%s][%d]MI_GFX_GetAlphaThresholdValue param invaild!\n", __FILE__, __LINE__);
        return MI_ERR_GFX_INVALID_PARAM;
    }
    s32Ret = MI_SYSCALL(MI_GFX_GET_ALPHA_GET_THRESHOLD_VALUE, &u8ThresholdValue);
    if (MI_SUCCESS == s32Ret)
    {
        *pu8ThresholdValue = u8ThresholdValue;
    }

    return s32Ret;
}

MI_S32 MI_GFX_SetAlphaThresholdValue(MI_U8 u8ThresholdValue)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_GFX_SET_ALPHA_SET_THRESHOLD_VALUE, &u8ThresholdValue);

    return s32Ret;
}

MI_S32 MI_GFX_BitBlit(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect, MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_GFX_BitBlit_t stBitBlit;
    memset(&stBitBlit, 0, sizeof(&stBitBlit));
    if ((NULL == pstSrc) || (NULL == pstDst))
    {
        DBG_ERR("[%s][%d]MI_GFX_BitBlit param invaild!\n", __FILE__, __LINE__);
        return MI_ERR_GFX_INVALID_PARAM;
    }
    if (NULL == pstSrcRect)
    {
        stBitBlit.stSrcRect.s32Xpos = 0;
        stBitBlit.stSrcRect.s32Ypos = 0;
        stBitBlit.stSrcRect.u32Width = pstSrc->u32Width;
        stBitBlit.stSrcRect.u32Height = pstSrc->u32Height;
    }
    else
    {
        memcpy(&stBitBlit.stSrcRect, pstSrcRect, sizeof(MI_GFX_Rect_t));
    }
    if (NULL == pstDstRect)
    {
        stBitBlit.stDstRect.s32Xpos = 0;
        stBitBlit.stDstRect.s32Ypos = 0;
        stBitBlit.stDstRect.u32Width = pstDst->u32Width;
        stBitBlit.stDstRect.u32Height = pstDst->u32Height;
    }
    else
    {
        memcpy(&stBitBlit.stDstRect, pstDstRect, sizeof(MI_GFX_Rect_t));
    }
    if (NULL == pstOpt)
    {
        stBitBlit.stOpt.bEnGfxRop = 0;
        stBitBlit.stOpt.eRopCode  = E_MI_GFX_ROP_COPYPEN;
        stBitBlit.stOpt.stSrcColorKeyInfo.bEnColorKey = 0;
        stBitBlit.stOpt.stDstColorKeyInfo.bEnColorKey = 0;
        stBitBlit.stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
        stBitBlit.stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
        stBitBlit.stOpt.eSrcYuvFmt = E_MI_GFX_YUV_YVYU;
        stBitBlit.stOpt.eDstYuvFmt = E_MI_GFX_YUV_YVYU;
        stBitBlit.stOpt.eMirror = E_MI_GFX_ROTATE_0;
        memset(&stBitBlit.stOpt.stClipRect, 0, sizeof(MI_GFX_Rect_t));
    }
    else
    {
        memcpy(&stBitBlit.stOpt, pstOpt, sizeof(MI_GFX_Opt_t));
    }
    memcpy(&stBitBlit.stSrc, pstSrc, sizeof(MI_GFX_Surface_t));
    memcpy(&stBitBlit.stDst, pstDst, sizeof(MI_GFX_Surface_t));

    s32Ret = MI_SYSCALL(MI_GFX_BIT_BLIT, &stBitBlit);
    if ((MI_SUCCESS == s32Ret) && (NULL != pu16Fence))
    {
        *pu16Fence = stBitBlit.u16Fence;
    }

    return s32Ret;
}

MI_S32 mi_gfx_PendingDone(MI_U16 u16TargetFence)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_GFX_PENDING_DONE, &u16TargetFence);

    return s32Ret;
}
EXPORT_SYMBOL(mi_gfx_PendingDone);

EXPORT_SYMBOL(MI_GFX_Open);
EXPORT_SYMBOL(MI_GFX_Close);
EXPORT_SYMBOL(MI_GFX_WaitAllDone);
EXPORT_SYMBOL(MI_GFX_QuickFill);
EXPORT_SYMBOL(MI_GFX_GetAlphaThresholdValue);
EXPORT_SYMBOL(MI_GFX_SetAlphaThresholdValue);
EXPORT_SYMBOL(MI_GFX_BitBlit);
