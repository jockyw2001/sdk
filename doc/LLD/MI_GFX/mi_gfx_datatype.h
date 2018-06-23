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
    E_MI_GFX_FMT_I1                      = 0x0, /* MS_ColorFormat */
    E_MI_GFX_FMT_I2                      = 0x1,
    E_MI_GFX_FMT_I4                      = 0x2,
    E_MI_GFX_FMT_I8                      = 0x4,
    E_MI_GFX_FMT_FABAFGBG2266            = 0x6,
    E_MI_GFX_FMT_1ABFGBG12355            = 0x7,
    E_MI_GFX_FMT_RGB565                  = 0x8,
    E_MI_GFX_FMT_ARGB1555                = 0x9,
    E_MI_GFX_FMT_ARGB4444                = 0xa,
    E_MI_GFX_FMT_ARGB1555_DST            = 0xc,
    E_MI_GFX_FMT_YUV422                  = 0xe,
    E_MI_GFX_FMT_ARGB8888                = 0xf,
    E_MI_GFX_FMT_RGBA5551                = 0x10,
    E_MI_GFX_FMT_RGBA4444                = 0x11,
    E_MI_GFX_FMT_ABGR8888                = 0x1f,
    E_MI_GFX_FMT_BGRA5551                = 0x12,
    E_MI_GFX_FMT_ABGR1555                = 0x13,
    E_MI_GFX_FMT_ABGR4444                = 0x14,
    E_MI_GFX_FMT_BGRA4444                = 0x15,
    E_MI_GFX_FMT_BGR565                  = 0x16,
    E_MI_GFX_FMT_RGBA8888                = 0x1d,
    E_MI_GFX_FMT_BGRA8888                = 0x1e,
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
    E_MI_GFX_ROP_MAX,
    E_MI_GFX_ROP_NONE = 0xFF;
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
    E_MI_GFX_DFB_BLD_ZERO                 = 0,
    E_MI_GFX_DFB_BLD_ONE                  = 1,
    E_MI_GFX_DFB_BLD_SRCCOLOR             = 2,
    E_MI_GFX_DFB_BLD_INVSRCCOLOR          = 3,
    E_MI_GFX_DFB_BLD_SRCALPHA             = 4,
    E_MI_GFX_DFB_BLD_INVSRCALPHA          = 5,
    E_MI_GFX_DFB_BLD_DESTALPHA            = 6,
    E_MI_GFX_DFB_BLD_INVDESTALPHA         = 7,
    E_MI_GFX_DFB_BLD_DESTCOLOR            = 8,
    E_MI_GFX_DFB_BLD_INVDESTCOLOR         = 9,
    E_MI_GFX_DFB_BLD_SRCALPHASAT          = 10,
    E_MI_GFX_DFB_BLD_MAX,
    E_MI_GFX_DFB_BLD_NONE = 0xFF,
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
    E_MI_GFX_ROTATE_0  = 0,
    E_MI_GFX_ROTATE_90,
    E_MI_GFX_ROTATE_180,
    E_MI_GFX_ROTATE_270
} MI_GFX_Rotate_e;

typedef enum
{
    E_MI_GFX_YUV_RGB2YUV_PC  = 0,  //Y：16~235 UV:16~240
    E_MI_GFX_YUV_RGB2YUV_255 = 1,  //Y：0-255 UV：0~255
} MI_GFX_Rgb2Yuv_e;

typedef enum
{
    E_MI_GFX_YUV_OUT_255   = 0,    // 0~255
    E_MI_GFX_YUV_OUT_PC    = 1,    // Y: 16~235, UV:16~240
}MI_GFX_YuvOutRange_e;

typedef enum
{
    E_MI_GFX_UV_IN_255     = 0,    // 0~255
    E_MI_GFX_UV_IN_127     = 1     // -128~127
} MI_GFX_UvInRange_e;

typedef enum
{
    E_MI_GFX_YUV_YVYU      = 0,    // YUV422 format
    E_MI_GFX_YUV_YUYV      = 1,
    E_MI_GFX_YUV_VYUY      = 2,
    E_MI_GFX_YUV_UYVY      = 3,
} MI_GFX_Yuv422_e;

typedef struct MI_GFX_Rect_s
{
    MI_S32 s32Xpos;
    MI_S32 s32Ypos;
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_GFX_Rect_t;

typedef struct MI_GFX_ColorKey_s
{
    MI_U32 Color_s; //ColoKey 范围起始值
    MI_U32 Color_e; //ColoKey 范围结束值
} MI_GFX_ColorKeyValue_t;

typedef struct MI_GFX_ColorKeyInfo_s
{
    MI_BOOL bEnColorKey;
    MI_GFX_ColorKeyOp_e eCKeyOp;
    MI_GFX_ColorFmt_e       eCKeyFmt;
    MI_GFX_ColorKeyValue_t  stCKeyVal;
} MI_GFX_ColorKeyInfo_t;

typedef struct MI_GFX_Composor_s
{
    MI_GFX_Surface_t stSrcSurface;
    MI_GFX_Rect_t stInRect;
    MI_GFX_Rect_t stOutRect;
    MI_GFX_Opt_t stOpt;
} MI_GFX_Composor_t;

typedef struct MI_GFX_SurfaceList_s
{
    MI_U32 u32SurfaceNum;
    MI_GFX_Surface_t *pDstSurface;
    MI_GFX_Composor_t*pstComposor;
} MI_GFX_SurfaceList_t;

typedef struct MI_GFX_Surface_s
{
    MI_U32 u32VirAddr;
    MI_GFX_ColorFmt_e eColorFmt;
    MI_U32 u32Height;
    MI_U32 u32Width;
    MI_U32 u32Stride;
} MI_GFX_Surface_t;

typedef struct MI_GFX_Opt_s
{
    MI_BOOL bEnGfxRop;                          /* 是否使能ROP操作 */
    MI_GFX_RopCode_e eRopCode;                  /* 颜色空间ROP类型 */
    MI_GFX_Rect_t stClipRect;                   /* clip区域定义 */
    MI_GFX_ColorKeyInfo_t stSrcColorKeyInfo;    /* Src surface colorkey 操作 */
    MI_GFX_ColorKeyInfo_t stDstColorKeyInfo;    /* Dst surface colorkey操作  */
    MI_GFX_DfbBldOp_e eSrcDfbBldOp;             /* Src Surface BLEND操作类型 */
    MI_GFX_DfbBldOp_e eDstDfbBldOp;             /* Dst Surface BLEND操作类型 */
    MI_GFX_Mirror_e eMirror;                    /* 镜像类型 */
    MI_GFX_Yuv422_e eSrcYuvFmt;                 /* 源Surface的YUV格式 */
    MI_GFX_Yuv422_e eDstYuvFmt;                 /* 目的Surface的YUV格式 */
    MI_GFX_Rotate_e eRotate;                    /* 支持旋转操作 */
} MI_GFX_Opt_t;

#ifdef __cplusplus
}
#endif

#endif///_MI_GFX_DATATYPE_H_