////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_graphic_adapter.h
/// @brief  MStar graphic Interface header file
/// @author MStar Semiconductor Inc.
/// @attention
/// <b><em></em></b>
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_GRAPHIC_ADAPTER_H
#define _MDRV_GRAPHIC_ADAPTER_H

#ifndef MSOS_TYPE_LINUX_KERNEL
#define MSOS_TYPE_LINUX_KERNEL
#endif

#include "mdrv_graphic_type.h"
/* utopia api header */
#include "MsCommon.h"
#include "MsOS.h"

#include "apiGFX.h"
#include "apiGOP.h"
#include "apiPNL.h"
#include "apiXC.h"

#ifdef _MDRV_GRAPHIC_ADAPTER_H
#define INTERFACE
#else
#define INTERFACE extern
#endif

#if defined(__cplusplus)
extern "C" {
#endif

//=============================================================================
// Function
//=============================================================================
INTERFACE void mstar_FB_InitContext(void);
INTERFACE MS_BOOL mstar_FB_Init(MS_U32 u32GopIdx, MS_U16 timingWidth, MS_U16 timingHeight, MS_U16 hstart);
INTERFACE void mstar_FB_SetHMirror(MS_U8 u8GOP,MS_BOOL bEnable);
INTERFACE void  mstar_FB_SetVMirror(MS_U8 u8GOP,MS_BOOL bEnable);
INTERFACE void mstar_FB_OutputColor_EX(MS_U8 u8GOP, DRV_FB_OutputColorSpace_e type);
INTERFACE void mstar_FB_MIUSel(MS_U8 u8GOP, DRV_FB_GOP_MiuSel_e MiuSel);
INTERFACE void mstar_FB_SetDst(MS_U8 u8GOP, DRV_FB_GOP_DstType_e eDsttype, MS_BOOL bOnlyCheck);
INTERFACE void mstar_FB_EnableTransClr_EX(MS_U8 u8GOP, DRV_FB_GOP_TransClrFmt_e eFmt, MS_BOOL bEnable);
INTERFACE void mstar_FB_SetBlending(MS_U8 u8GOP, MS_U8 u8win, MS_BOOL bEnable, MS_U8 u8coef);
INTERFACE void mstar_FB_SetGwinInfo(MS_U8 u8GOP, MS_U8 u8win, DRV_FB_GwinInfo_t WinInfo);
INTERFACE void mstar_FB_SetStretchWin(MS_U8 u8GOP,MS_U16 u16x, MS_U16 u16y, MS_U16 u16width, MS_U16 u16height);
INTERFACE void mstar_FB_SetHScale(MS_U8 u8GOP,MS_BOOL bEnable, MS_U16 u16src, MS_U16 u16dst);
INTERFACE void mstar_FB_SetVScale(MS_U8 u8GOP,MS_BOOL bEnable, MS_U16 u16src, MS_U16 u16dst);
INTERFACE void mstar_FB_EnableGwin(MS_U8 u8GOP, MS_U8 u8win,MS_BOOL bEnable);
INTERFACE void mstar_FB_BeginTransaction(MS_U8 u8Gop);
INTERFACE void mstar_FB_EndTransaction(MS_U8 u8Gop);
INTERFACE void mstar_FB_GOP_SetForceWrite(MS_BOOL bEnable);
INTERFACE MS_U16 mstar_FB_GetBpp(DRV_FB_GOP_ColorFmt_e eColorFmt);
INTERFACE void mstar_FB_SetTransClr_8888(MS_U8 u8Gop, MS_U32 clr, MS_U32 mask);
INTERFACE MS_BOOL mstar_FB_IsSupportMultiAlpha(MS_U8 u8Gop);
INTERFACE void mstar_FB_EnableMultiAlpha(MS_U8 u8Gop, MS_BOOL bEnable);
INTERFACE void mstar_FB_SetAlpha0(MS_U8 u8GwinId, MS_U8 u8Alpha0);
INTERFACE void mstar_FB_SetAlpha1(MS_U8 u8GwinId, MS_U8 u8Alpha1);
INTERFACE void mstar_FB_SetNewAlphaMode(MS_U8 u8Gop, MS_U8 u8win, MS_BOOL bEnable);
INTERFACE int mstar_FB_getCurOPTiming(int *ret_width, int *ret_height, int *ret_hstart);
INTERFACE void mstar_FB_setHstart(MS_U8 u8GOP,MS_U16 u16PanelHStr);
INTERFACE MS_BOOL mstar_FB_CreateFBInfo(MS_U8 gwinId, MS_U16 width, MS_U16 height,
    DRV_FB_GOP_ColorFmt_e fmt, MS_U16 pitch, MS_PHY phyAddr);
INTERFACE void mstar_FB_DestroyFBInfo(MS_U8 gwinId);
INTERFACE MS_BOOL mstar_FB_SetOSDBlendingFormula(MS_U8 u8Gop,  MS_BOOL bPremul);
INTERFACE void mstar_FB_EnableAlphaInverse(MS_U8 u8Gop, MS_BOOL bEnable);
INTERFACE MS_U32 mstar_FB_GetGopWorldUnit(void);
INTERFACE void mstar_FB_InitLayerSetting(MS_U32* u32GopIdx, MS_U32* u32LayerIdx, MS_U8 u8GopCount);
#if defined(__cplusplus)
}
#endif

#undef INTERFACE

#endif //_MDRV_GRAPHIC_H
