///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2016-2017 MStar Semiconductor, Inc.
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
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// @file   mdrv_graphic_type.h
// @brief  Graphic Driver Interface
// @author MStar Semiconductor Inc.
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MDRV_GRAPHIC_TYPE_H
#define _MDRV_GRAPHIC_TYPE_H

#include "MsTypes.h"
#if defined(__cplusplus)
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

/// GWIN output color domain
typedef enum
{
    E_DRV_FB_GOPOUT_RGB,     ///< 0: output color RGB
    E_DRV_FB_GOPOUT_YUV,     ///< 1: output color YUV
} DRV_FB_OutputColorSpace_e;

/// Define GOP MIU SEL
typedef enum
{
    /// E_DRV_GOP_SEL_MIU0. gop access miu 0
    E_DRV_FB_GOP_SEL_MIU0    = 0,
    /// E_GOP_SEL_MIU1. gop access miu1
    E_DRV_FB_GOP_SEL_MIU1    = 1,
    /// E_GOP_SEL_MIU2. gop access miu2
    E_DRV_FB_GOP_SEL_MIU2    = 2,
    /// E_GOP_SEL_MIU3. gop access miu3
    E_DRV_FB_GOP_SEL_MIU3    = 3,
} DRV_FB_GOP_MiuSel_e;

/// Define GOP destination displayplane type
typedef enum
{
    E_DRV_FB_GOP_DST_IP0       =   0,
    E_DRV_FB_GOP_DST_IP0_SUB   =   1,
    E_DRV_FB_GOP_DST_MIXER2VE  =   2,
    E_DRV_FB_GOP_DST_OP0       =   3,
    E_DRV_FB_GOP_DST_VOP       =   4,
    E_DRV_FB_GOP_DST_IP1       =   5,
    E_DRV_FB_GOP_DST_IP1_SUB   =   6,
    E_DRV_FB_GOP_DST_MIXER2OP  =   7,
    E_DRV_FB_GOP_DST_VOP_SUB   =   8,
    E_DRV_FB_GOP_DST_FRC       =   9,
    E_DRV_FB_GOP_DST_VE        =   10,
    E_DRV_FB_GOP_DST_BYPASS    =   11,
    E_DRV_FB_GOP_DST_OP1       =   12,
    E_DRV_FB_GOP_DST_MIXER2OP1 =  13,
    E_DRV_FB_GOP_DST_DIP       =   14,
    E_DRV_FB_GOP_DST_GOPScaling  = 15,
    E_DRV_FB_GOP_DST_OP_DUAL_RATE   = 16,
    MAX_FB_DRV_GOP_DST_SUPPORT ,
    E_FB_DRV_GOP_DST_INVALID,
} DRV_FB_GOP_DstType_e;

/// Transparent color format
typedef enum
{
    /// RGB mode transparent color.
    E_DRV_FB_GOPTRANSCLR_FMT0,
   /// YUV mode transparent color.
    E_DRV_FB_GOPTRANSCLR_FMT1,
} DRV_FB_GOP_TransClrFmt_e;

typedef enum
{
    /// Color format RGB555 and Blink.
    E_DRV_FB_GOP_COLOR_RGB555_BLINK    =0,
    /// Color format RGB565.
    E_DRV_FB_GOP_COLOR_RGB565          =1,
    /// Color format ARGB4444.
    E_DRV_FB_GOP_COLOR_ARGB4444        =2,
    /// Color format alpha blink.
    E_DRV_FB_GOP_COLOR_2266      =3,
    /// Color format I8 (256-entry palette).
    E_DRV_FB_GOP_COLOR_I8              =4,
    /// Color format ARGB8888.
    E_DRV_FB_GOP_COLOR_ARGB8888        =5,
    /// Color format ARGB1555.
    E_DRV_FB_GOP_COLOR_ARGB1555        =6,
    /// Color format ABGR8888.  - Andriod format
    E_DRV_FB_GOP_COLOR_ABGR8888        =7,
    /// Color format RGB555/YUV422.
     E_DRV_FB_GOP_COLOR_RGB555YUV422    =8,
    /// Color format YUV422.
    E_DRV_FB_GOP_COLOR_YUV422          =9,
    /// Color format ARGB8888.  - Andriod format
    E_DRV_FB_GOP_COLOR_RGBA5551        =10,
    /// Color format ARGB8888.  - Andriod format
    E_DRV_FB_GOP_COLOR_RGBA4444        =11,

    /// Invalid color format.
    E_DRV_FB_GOP_COLOR_INVALID,
} DRV_FB_GOP_ColorFmt_e;

typedef struct DRV_FB_GOP_InitInfo_s
{
    ///panel width.
    MS_U16 u16PanelWidth;
    ///panel height.
    MS_U16 u16PanelHeight;
    ///panel h-start.
    MS_U16 u16PanelHStr;
    ///vsync interrupt flip enable flag.
    MS_BOOL bEnableVsyncIntFlip;
    ///gop frame buffer starting address.
    MS_PHY u32GOPRBAdr;
    ///gop frame buffer length.
    MS_U32 u32GOPRBLen;
    ///gop regdma starting address.
    MS_PHY u32GOPRegdmaAdr;
    ///gop regdma length.
    MS_U32 u32GOPRegdmaLen;
}DRV_FB_GOP_InitInfo_t;

typedef struct DRV_FB_GwinInfo_s
{
    MS_U16 u16HStart;              //!< unit: pix
    MS_U16 u16HEnd;                //!< unit: pix
    MS_U16 u16VStart;              //!< unit: pix
    MS_U16 u16VEnd;                //!< unit: pix
    MS_U32 u16Pitch;               //!< unit: Byte
    MS_U32 u32Addr;                //!< unit: pix
    DRV_FB_GOP_ColorFmt_e clrType;       //!< color format of the buffer
} DRV_FB_GwinInfo_t;

typedef struct DRV_FB_StretchWinInfo_s
{
    MS_U16 u16Xpos; //!< unit: pix
    MS_U16 u16Ypos; //!< unit: pix
    MS_U16 u16SrcWidth; //!< unit: pix
    MS_U16 u16SrcHeight; //!< unit: pix
    MS_U16 u16DstWidth; //!< unit: pix
    MS_U16 u16DstHeight;//!< unit: pix
}DRV_FB_StretchWinInfo_t;
#if defined(__cplusplus)
}
#endif
#endif //_MHAL_GRAPHIC_COP_H
