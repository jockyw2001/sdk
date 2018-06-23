////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

/**
* @file     hal_scl_util.h
* @version
* @Platform I3
* @brief    This file defines the HAL SCL utility interface
*
*/

#ifndef __HAL_GOP_H__
#define __HAL_GOP_H__

//=============================================================================
// Defines
//=============================================================================
#include "hal_gop_reg.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"
#define  bool   unsigned char

//=============================================================================
// enum
//=============================================================================
typedef enum
{
    E_HAL_ISPGOP_ID_00     = 0x00,
    E_HAL_ISPGOP_ID_01     = 0x01,
    E_HAL_ISPGOP_ID_0_ST   = 0x02,
    E_HAL_ISPGOP_ID_10     = 0x10,
    E_HAL_ISPGOP_ID_11     = 0x11,
    E_HAL_ISPGOP_ID_1_ST   = 0x12,
    E_HAL_ISPGOP_ID_20     = 0x20,
    E_HAL_ISPGOP_ID_21     = 0x21,
    E_HAL_ISPGOP_ID_2_ST   = 0x22,
    E_HAL_ISPGOP_ID_30     = 0x30,
    E_HAL_ISPGOP_ID_31     = 0x31,
    E_HAL_ISPGOP_ID_3_ST   = 0x32,
    E_HAL_DIPGOP_ID_40     = 0x40,
    E_HAL_DIPGOP_ID_41     = 0x41,
    E_HAL_DIPGOP_ID_4_ST   = 0x42,
    E_HAL_DISPGOP_ID_50     = 0x50,
    E_HAL_DISPGOP_ID_51     = 0x51,
    E_HAL_DISPGOP_ID_5_ST   = 0x52,
    E_HAL_DISPGOP_ID_60     = 0x60,
    E_HAL_DISPGOP_ID_61     = 0x61,
    E_HAL_DISPGOP_ID_6_ST   = 0x62,
} HalGopIdType_e;

typedef enum
{
    E_HAL_GOP_CMDQ_VPE_ID_0 = 0,
    E_HAL_GOP_CMDQ_DIVP_ID_1 = 1,
    E_HAL_GOP_CMDQ_ID_NUM = 2,
}HalGopCmdqIdType_e;

typedef enum
{
    E_HAL_GOP_GWIN_ID_0 = 0,
    E_HAL_GOP_GWIN_ID_1 = 1,
    E_HAL_GOP_GWIN_ID_NUM = 2,
} HalGopGwinIdType_e;

typedef enum
{
    E_HAL_GOP_DISPLAY_MODE_INTERLACE = 0x00,
    E_HAL_GOP_DISPLAY_MODE_PROGRESS  = 0x01,
} HalGopDisplayModeType_t;

typedef enum
{
    E_HAL_GOP_OUT_FMT_RGB = 0x00,
    E_HAL_GOP_OUT_FMT_YUV = 0x01,
} HalGopOutFormatType_e;

typedef enum
{
    E_HAL_GOP_GWIN_SRC_RGB1555    = 0x00,
    E_HAL_GOP_GWIN_SRC_RGB565     = 0x01,
    E_HAL_GOP_GWIN_SRC_ARGB4444   = 0x02,
    E_HAL_GOP_GWIN_SRC_2266       = 0x03,
    E_HAL_GOP_GWIN_SRC_I8_PALETTE = 0x04,
    E_HAL_GOP_GWIN_SRC_ARGB8888   = 0x05,
    E_HAL_GOP_GWIN_SRC_ARGB1555   = 0x06,
    E_HAL_GOP_GWIN_SRC_ABGR8888   = 0x07,
    E_HAL_GOP_GWIN_SRC_UV7Y8      = 0x08,
    E_HAL_GOP_GWIN_SRC_UV8Y8      = 0x09,
    E_HAL_GOP_GWIN_SRC_RGBA5551   = 0x0A,
    E_HAL_GOP_GWIN_SRC_RGBA4444   = 0x0B,
    E_HAL_GOP_GWIN_SRC_I4_PALETTE = 0x0D,
    E_HAL_GOP_GWIN_SRC_I2_PALETTE = 0x0E,
} HalGopGwinSrcFormat_e;

typedef enum
{
    E_HAL_GOP_ALLBANK_DOUBLE_WR = GOP_BANK_ALLBANK_DOUBLE_WR,
    E_HAL_GOP_FORCE_WR = GOP_BANK_FORCE_WR,
    E_HAL_GOP_ONEBANK_DOUBLE_WR = GOP_BANK_ONEBANK_DOUBLE_WR,
} HalGopWrRegType_e;

typedef enum
{
    E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0 = 0,
    E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1 = 1,
} HalGopGwinArgb1555Def_e;

//=============================================================================
// struct
//=============================================================================
typedef struct
{
    u16 u16X;
    u16 u16Y;
    u16 u16Width;
    u16 u16Height;
} HalGopWindowType_t;

typedef struct
{
    HalGopDisplayModeType_t eDisplayMode;
    HalGopOutFormatType_e eOutFormat;
    HalGopWindowType_t tStretchWindow;
} HalGopParamConfig_t;

typedef struct
{
    HalGopGwinSrcFormat_e eSrcFmt;
    HalGopWindowType_t tDispWindow;
    u32 u32BaseAddr;
    u16 u16Base_XOffset;
    u32 u32Base_YOffset;
} HalGopGwinParamConfig_t;
//=============================================================================

//=============================================================================
#ifndef __HAL_GOP_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE void HalGopInitCmdq(void);
INTERFACE void HalGopSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,HalGopCmdqIdType_e eHalCmdqId);
INTERFACE void HalGopUpdateBase(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32BaseAddr);
INTERFACE void HalGopSetGwinMemPitch(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32MemPixelWidth, HalGopGwinSrcFormat_e eSrcFmt);
INTERFACE void HalGopSetGwinMemPitchDirect(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32MemPitch);
INTERFACE void HalGopUpdateParam(HalGopIdType_e eGopId, HalGopParamConfig_t tParamCfg);
INTERFACE void HalGopUpdateGwinParam(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId,HalGopGwinParamConfig_t tParamCfg);
INTERFACE void HalGopSetStretchWindowSize(HalGopIdType_e eGopId, HalGopWindowType_t tGwinCfg);
INTERFACE void HalGopSetGwinSize(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopWindowType_t tGwinCfg, HalGopGwinSrcFormat_e eSrcFmt);
INTERFACE void HalGopSetGwinSrcFmt(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinSrcFormat_e eSrcFmt);
INTERFACE void HalGopSetOutFormat(HalGopIdType_e eGopId, bool bYUVOutput);
INTERFACE void HalGopSetAlphaBlending(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool bConstantAlpha, u8 Alpha);
INTERFACE void HalGopSetColorKey(HalGopIdType_e eGopId, bool bEn, u8 u8R, u8 u8G, u8 u8B);
INTERFACE void HalGopSetPipeDelay(HalGopIdType_e eGopId, u8 delay);
INTERFACE void HalGopSetPaletteRiu(HalGopIdType_e eGopId, u8 u8Index, u8 u8A, u8 u8R, u8 u8G, u8 u8B);
INTERFACE void HalGopSetEnableGwin(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool bEn);
INTERFACE void HalGopSetScalerEnableGop(HalGopIdType_e eGopId, bool bEn);
INTERFACE void HalGopSetArgb1555Alpha(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinArgb1555Def_e eAlphaType, MS_U8 eAlphaVal);
INTERFACE void HalGop0SetOsdBypassForSc2Enable(bool bEn);
INTERFACE void HalGopGetEnableGwin(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool *bEn);
INTERFACE void HalGopGetScalerEnableGop(HalGopIdType_e eGopId, bool *bEn);
INTERFACE void HalGopGetColorKey(HalGopIdType_e eGopId, bool *bEn, u8 *u8R, u8  *u8G, u8  *u8B);
INTERFACE void HalGopSetAlphaBlending(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool bConstantAlpha, u8 Alpha);
INTERFACE void HalGopGetAlpha(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool *pbConstantAlpha, u8 *pu8ConstantAlphaValue);

#undef INTERFACE
#endif /* __HAL_GOP_H__ */
