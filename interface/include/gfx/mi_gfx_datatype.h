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

#ifndef _MI_GFX_DATATYPE_H_
#define _MI_GFX_DATATYPE_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "mi_common.h"

typedef enum
{
    E_MI_GFX_FMT_I1 = 0, /* MS_ColorFormat */
    E_MI_GFX_FMT_I2,
    E_MI_GFX_FMT_I4,
    E_MI_GFX_FMT_I8,
    E_MI_GFX_FMT_FABAFGBG2266,
    E_MI_GFX_FMT_1ABFGBG12355,
    E_MI_GFX_FMT_RGB565,
    E_MI_GFX_FMT_ARGB1555,
    E_MI_GFX_FMT_ARGB4444,
    E_MI_GFX_FMT_ARGB1555_DST,
    E_MI_GFX_FMT_YUV422,
    E_MI_GFX_FMT_ARGB8888,
    E_MI_GFX_FMT_RGBA5551,
    E_MI_GFX_FMT_RGBA4444,
    E_MI_GFX_FMT_ABGR8888,
    E_MI_GFX_FMT_BGRA5551,
    E_MI_GFX_FMT_ABGR1555,
    E_MI_GFX_FMT_ABGR4444,
    E_MI_GFX_FMT_BGRA4444,
    E_MI_GFX_FMT_BGR565,
    E_MI_GFX_FMT_RGBA8888,
    E_MI_GFX_FMT_BGRA8888,
    E_MI_GFX_FMT_MAX
} MI_GFX_ColorFmt_e;


typedef enum
{
    E_MI_GFX_ROP_BLACK = 0,     /*Blackness*/
    E_MI_GFX_ROP_NOTMERGEPEN,   /*~(S2+S1)*/
    E_MI_GFX_ROP_MASKNOTPEN,    /*~S2&S1*/
    E_MI_GFX_ROP_NOTCOPYPEN,    /* ~S2*/
    E_MI_GFX_ROP_MASKPENNOT,    /* S2&~S1 */
    E_MI_GFX_ROP_NOT,           /* ~S1 */
    E_MI_GFX_ROP_XORPEN,        /* S2^S1 */
    E_MI_GFX_ROP_NOTMASKPEN,    /* ~(S2&S1) */
    E_MI_GFX_ROP_MASKPEN,       /* S2&S1 */
    E_MI_GFX_ROP_NOTXORPEN,     /* ~(S2^S1) */
    E_MI_GFX_ROP_NOP,           /* S1 */
    E_MI_GFX_ROP_MERGENOTPEN,   /* ~S2+S1 */
    E_MI_GFX_ROP_COPYPEN,       /* S2 */
    E_MI_GFX_ROP_MERGEPENNOT,   /* S2+~S1 */
    E_MI_GFX_ROP_MERGEPEN,      /* S2+S1 */
    E_MI_GFX_ROP_WHITE,         /* Whiteness */
    E_MI_GFX_ROP_NONE,          /* No Rop Op */
    E_MI_GFX_ROP_MAX,
} MI_GFX_RopCode_e;

typedef enum
{
    E_MI_GFX_RGB_OP_EQUAL = 0,
    E_MI_GFX_RGB_OP_NOT_EQUAL,
    E_MI_GFX_ALPHA_OP_EQUAL,
    E_MI_GFX_ALPHA_OP_NOT_EQUAL,
    E_MI_GFX_ARGB_OP_EQUAL,
    E_MI_GFX_ARGB_OP_NOT_EQUAL,
    E_MI_GFX_CKEY_OP_MAX,
} MI_GFX_ColorKeyOp_e;

typedef enum
{
    E_MI_GFX_DFB_BLD_ZERO = 0,
    E_MI_GFX_DFB_BLD_ONE,
    E_MI_GFX_DFB_BLD_SRCCOLOR,
    E_MI_GFX_DFB_BLD_INVSRCCOLOR,
    E_MI_GFX_DFB_BLD_SRCALPHA,
    E_MI_GFX_DFB_BLD_INVSRCALPHA,
    E_MI_GFX_DFB_BLD_DESTALPHA,
    E_MI_GFX_DFB_BLD_INVDESTALPHA,
    E_MI_GFX_DFB_BLD_DESTCOLOR,
    E_MI_GFX_DFB_BLD_INVDESTCOLOR,
    E_MI_GFX_DFB_BLD_SRCALPHASAT,
    E_MI_GFX_DFB_BLD_NONE,
    E_MI_GFX_DFB_BLD_MAX,
} MI_GFX_DfbBldOp_e;

typedef enum
{
    E_MI_GFX_MIRROR_NONE = 0,
    E_MI_GFX_MIRROR_HORIZONTAL,
    E_MI_GFX_MIRROR_VERTICAL,
    E_MI_GFX_MIRROR_BOTH,
    E_MI_GFX_MIRROR_MAX
} MI_GFX_Mirror_e;

typedef enum
{
    E_MI_GFX_ROTATE_0 = 0,
    E_MI_GFX_ROTATE_90,
    E_MI_GFX_ROTATE_180,
    E_MI_GFX_ROTATE_270,
    E_MI_GFX_ROTATE_MAX
} MI_GFX_Rotate_e;

typedef enum
{
    E_MI_GFX_YUV_YVYU = 0, // YUV422 format
    E_MI_GFX_YUV_YUYV,
    E_MI_GFX_YUV_VYUY,
    E_MI_GFX_YUV_UYVY,
    E_MI_GFX_YUV_MAX
} MI_GFX_Yuv422_e;

typedef enum
{
    E_MI_GFX_ERR_NOT_INIT = MI_GFX_INITIAL_ERROR_CODE,
    E_MI_GFX_ERR_GFX_DRV_NOT_SUPPORT,
    E_MI_GFX_ERR_GFX_DRV_FAIL_FORMAT,
    E_MI_GFX_ERR_GFX_NON_ALIGN_ADDRESS,
    E_MI_GFX_ERR_GFX_NON_ALIGN_PITCH,
    E_MI_GFX_ERR_GFX_DRV_FAIL_OVERLAP,
    E_MI_GFX_ERR_GFX_DRV_FAIL_STRETCH,
    E_MI_GFX_ERR_GFX_DRV_FAIL_ITALIC,
    E_MI_GFX_ERR_GFX_DRV_FAIL_LOCKED,
    E_MI_GFX_ERR_GFX_DRV_FAIL_BLTADDR,
    E_MI_GFX_ERR_MAX
} MI_GFX_ErrCode_e;


/* GFX Module ErrorCode */
#define MI_ERR_GFX_INVALID_PARAM MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_GFX_INVALID_DEVID MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
#define MI_ERR_GFX_DEV_BUSY MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)

#define MI_ERR_GFX_NOT_INIT MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_NOT_INIT)
#define MI_ERR_GFX_DRV_NOT_SUPPORT MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_NOT_SUPPORT)
#define MI_ERR_GFX_DRV_FAIL_FORMAT MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_FORMAT)
#define MI_ERR_GFX_NON_ALIGN_ADDRESS MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_NON_ALIGN_ADDRESS)
#define MI_ERR_GFX_NON_ALIGN_PITCH MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_NON_ALIGN_PITCH)
#define MI_ERR_GFX_DRV_FAIL_OVERLAP MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_OVERLAP)
#define MI_ERR_GFX_DRV_FAIL_STRETCH MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_STRETCH)
#define MI_ERR_GFX_DRV_FAIL_ITALIC MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_ITALIC)
#define MI_ERR_GFX_DRV_FAIL_LOCKED MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_LOCKED)
#define MI_ERR_GFX_DRV_FAIL_BLTADDR MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_BLTADDR)

typedef struct MI_GFX_Rect_s
{
    MI_S32 s32Xpos;
    MI_S32 s32Ypos;
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_GFX_Rect_t;

typedef struct MI_GFX_ColorKey_s
{
    MI_U32 u32ColorStart;
    MI_U32 u32ColorEnd;
} MI_GFX_ColorKeyValue_t;

typedef struct MI_GFX_ColorKeyInfo_s
{
    MI_BOOL bEnColorKey;
    MI_GFX_ColorKeyOp_e eCKeyOp;
    MI_GFX_ColorFmt_e eCKeyFmt;
    MI_GFX_ColorKeyValue_t stCKeyVal;
} MI_GFX_ColorKeyInfo_t;

typedef struct MI_GFX_Surface_s
{
    MI_PHY phyAddr;
    MI_GFX_ColorFmt_e eColorFmt;
    MI_U32 u32Width;
    MI_U32 u32Height;
    MI_U32 u32Stride;
} MI_GFX_Surface_t;

typedef struct MI_GFX_Opt_s
{
    MI_BOOL bEnGfxRop;
    MI_GFX_RopCode_e eRopCode;
    MI_GFX_Rect_t stClipRect;
    MI_GFX_ColorKeyInfo_t stSrcColorKeyInfo;
    MI_GFX_ColorKeyInfo_t stDstColorKeyInfo;
    MI_GFX_DfbBldOp_e eSrcDfbBldOp;
    MI_GFX_DfbBldOp_e eDstDfbBldOp;
    MI_GFX_Mirror_e eMirror;
    MI_GFX_Yuv422_e eSrcYuvFmt;
    MI_GFX_Yuv422_e eDstYuvFmt;
    MI_GFX_Rotate_e eRotate;
} MI_GFX_Opt_t;

#ifdef __cplusplus
}
#endif

#endif///_MI_GFX_DATATYPE_H_
