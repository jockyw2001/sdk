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
/// @file   mdrv_graphic_gop.c
/// @brief  MStar graphic Interface
/// @author MStar Semiconductor Inc.
/// @attention
/// <b><em></em></b>
///////////////////////////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Include Files
//=============================================================================
#include "mdrv_graphic_adapter.h"
#include "drvSYS.h"
#define INVALID_FBID    0xFF
//=============================================================================
// Macros
//=============================================================================
#define ALIGN_CHECK(value,factor) ((value + factor-1) & (~(factor-1)))

static MS_U32 _SetFBFmt(MS_U16 pitch, MS_PHY addr, MS_U16 fmt) {
    // Set FB info to GE (For GOP callback fucntion use)
    return 1;
}

static MS_BOOL _XC_IsInterlace(void) {
    return 0;
}

static void _SetPQReduceBWForOSD(MS_U8 PqWin, MS_BOOL enable) {
}

static MS_U16 _XC_GetCapwinHStr(void) {
    return 0x80;
}

static  EN_GOP_COLOR_TYPE convertToGopColorType(DRV_FB_GOP_ColorFmt_e clrType)
{
    switch (clrType)
    {
        case E_DRV_FB_GOP_COLOR_RGB555_BLINK:
            return E_GOP_COLOR_RGB555_BLINK;
        case E_DRV_FB_GOP_COLOR_RGB565:
            return E_GOP_COLOR_RGB565;
        case E_DRV_FB_GOP_COLOR_ARGB4444:
            return E_GOP_COLOR_ARGB4444;
        case E_DRV_FB_GOP_COLOR_2266:
            return  E_GOP_COLOR_2266;
        case E_DRV_FB_GOP_COLOR_I8:
            return E_GOP_COLOR_I8;
        case E_DRV_FB_GOP_COLOR_ARGB8888:
            return E_GOP_COLOR_ARGB8888;
        case E_DRV_FB_GOP_COLOR_ARGB1555:
            return E_GOP_COLOR_ARGB1555;
        case E_DRV_FB_GOP_COLOR_ABGR8888:
            return E_GOP_COLOR_ABGR8888;
        case E_DRV_FB_GOP_COLOR_RGB555YUV422:
            return E_GOP_COLOR_RGB555YUV422;
       case E_DRV_FB_GOP_COLOR_YUV422:
            return E_GOP_COLOR_YUV422;
       case E_DRV_FB_GOP_COLOR_RGBA5551:
            return E_GOP_COLOR_RGBA5551;
        case E_DRV_FB_GOP_COLOR_RGBA4444:
            return E_GOP_COLOR_RGBA4444;
        default:
            return E_GOP_COLOR_INVALID;
    }
}

static EN_GOP_DST_TYPE convertToGopDstType(DRV_FB_GOP_DstType_e eGopDst)
{
    switch (eGopDst) {
        case E_DRV_FB_GOP_DST_IP0:
            return E_GOP_DST_IP0;
        case E_DRV_FB_GOP_DST_IP0_SUB:
            return E_GOP_DST_IP_SUB;
        case E_DRV_FB_GOP_DST_MIXER2VE:
            return E_GOP_DST_MIXER2VE;
        case E_DRV_FB_GOP_DST_OP0:
            return E_GOP_DST_OP0;
        case E_DRV_FB_GOP_DST_VOP:
            return E_GOP_DST_VOP;
        case E_DRV_FB_GOP_DST_IP1:
            return E_GOP_DST_IP1;
        case E_DRV_FB_GOP_DST_MIXER2OP:
            return E_GOP_DST_MIXER2OP;
        case E_DRV_FB_GOP_DST_FRC:
            return E_GOP_DST_FRC;
        case E_DRV_FB_GOP_DST_VE:
            return E_GOP_DST_VE;
        case E_DRV_FB_GOP_DST_BYPASS:
            return E_GOP_DST_BYPASS;
        case E_DRV_FB_GOP_DST_OP1:
            return E_GOP_DST_OP1;
        case  E_DRV_FB_GOP_DST_MIXER2OP1:
            return E_GOP_DST_MIXER2OP1;
        case E_DRV_FB_GOP_DST_DIP:
            return E_GOP_DST_DIP;
        case E_DRV_FB_GOP_DST_GOPScaling:
            return E_GOP_DST_GOPScaling;
        case E_DRV_FB_GOP_DST_OP_DUAL_RATE:
            return E_GOP_DST_OP_DUAL_RATE;
        case  E_DRV_FB_GOP_DST_IP1_SUB:
        case E_DRV_FB_GOP_DST_VOP_SUB:
        default:
            return MAX_GOP_DST_SUPPORT;
    }
}

void mstar_FB_InitContext()
{
    MsOS_MPool_Init();
    MDrv_SYS_GlobalInit();
    //setup the gop driver local structure
    MApi_GOP_InitByGOP(NULL, 0);
    MApi_GOP_GWIN_ResetPool();
}

MS_BOOL mstar_FB_Init(MS_U32 u32GopIdx, MS_U16 timingWidth, MS_U16 timingHeight, MS_U16 hstart)
{
    GOP_InitInfo gopInitInfo;
    EN_GOP_IGNOREINIT Initlist = E_GOP_IGNORE_MUX;
    //int curTimingWidth, curTimingHeight, hstart;
    Mapi_GOP_GWIN_ResetGOP(u32GopIdx);
    memset(&gopInitInfo, 0, sizeof(GOP_InitInfo));
    //mstar_FB_getCurOPTiming(&curTimingWidth, &curTimingHeight, &hstart);
    gopInitInfo.u16PanelWidth = timingWidth;
    gopInitInfo.u16PanelHeight = timingHeight;
    gopInitInfo.u16PanelHStr   = hstart;
    gopInitInfo.u32GOPRBAdr = 0;
    gopInitInfo.u32GOPRBLen = 0;
    gopInitInfo.bEnableVsyncIntFlip = TRUE;

    MApi_GOP_SetConfigEx(u32GopIdx, E_GOP_IGNOREINIT, (MS_U32 *)(&Initlist));
    MApi_GOP_InitByGOP(&gopInitInfo, u32GopIdx);

    MApi_GOP_RegisterFBFmtCB(_SetFBFmt);
    MApi_GOP_RegisterXCIsInterlaceCB(_XC_IsInterlace);
    MApi_GOP_RegisterXCGetCapHStartCB(_XC_GetCapwinHStr);
    MApi_GOP_RegisterXCReduceBWForOSDCB(_SetPQReduceBWForOSD);
    return TRUE;
}
void mstar_FB_SetHMirror(MS_U8 u8GOP, MS_BOOL bEnable)
{
    MApi_GOP_GWIN_SetHMirror_EX(u8GOP, bEnable);
}
void mstar_FB_SetVMirror(MS_U8 u8GOP, MS_BOOL bEnable)
{
    MApi_GOP_GWIN_SetVMirror_EX(u8GOP, bEnable);
}
void mstar_FB_OutputColor_EX(MS_U8 u8GOP, DRV_FB_OutputColorSpace_e type)
{
    MApi_GOP_GWIN_OutputColor_EX(u8GOP, (EN_GOP_OUTPUT_COLOR)type);
}
void mstar_FB_MIUSel(MS_U8 u8GOP, DRV_FB_GOP_MiuSel_e MiuSel)
{
    MApi_GOP_MIUSel(u8GOP, (EN_GOP_SEL_TYPE)MiuSel);
}
void mstar_FB_SetDst(MS_U8 u8GOP, DRV_FB_GOP_DstType_e eDsttype, MS_BOOL bOnlyCheck)
{
    MApi_GOP_GWIN_SetGOPDst(u8GOP, convertToGopDstType(eDsttype));
}
void mstar_FB_EnableTransClr_EX(MS_U8 u8GOP, DRV_FB_GOP_TransClrFmt_e eFmt, MS_BOOL bEnable)
{
    MApi_GOP_GWIN_EnableTransClr_EX(u8GOP, (EN_GOP_TRANSCLR_FMT)eFmt, bEnable);
}
void mstar_FB_SetBlending(MS_U8 u8GOP, MS_U8 u8win, MS_BOOL bEnable, MS_U8 u8coef)
{
    MApi_GOP_GWIN_SetBlending(u8win, bEnable, u8coef);
}
void mstar_FB_SetGwinInfo(MS_U8 u8GOP, MS_U8 u8win, DRV_FB_GwinInfo_t WinInfo)
{
    GOP_GwinInfo pinfo;
    //MS_U16 u16bpp = mstar_FB_GetBpp(WinInfo.clrType);
    //the gwin width must be 2 pixel alignment
    MS_U16 width = (WinInfo.u16HEnd - WinInfo.u16HStart) &(~0x1);
    memset(&pinfo,0,sizeof(pinfo));
    MApi_GOP_GWIN_GetWinInfo(u8win, &pinfo);
    //convert member from DRV_FB_GwinInfo_t to GOP_GwinInfo
    pinfo.u16DispHPixelStart = WinInfo.u16HStart;
    pinfo.u16DispHPixelEnd = WinInfo.u16HStart + width;
    pinfo.u16DispVPixelStart = WinInfo.u16VStart;
    pinfo.u16DispVPixelEnd = WinInfo.u16VEnd;
    pinfo.u16RBlkHRblkSize = WinInfo.u16Pitch;
    pinfo.u32DRAMRBlkStart = WinInfo.u32Addr;
    pinfo.clrType = convertToGopColorType(WinInfo.clrType);
    MApi_GOP_GWIN_SetWinInfo(u8win, &pinfo);
}
void mstar_FB_SetStretchWin(MS_U8 u8GOP,MS_U16 u16x, MS_U16 u16y, MS_U16 u16width, MS_U16 u16height)
{
    MApi_GOP_GWIN_Set_STRETCHWIN(u8GOP, E_GOP_DST_OP0, u16x, u16y, u16width, u16height);
}

void mstar_FB_SetHScale(MS_U8 u8GOP,MS_BOOL bEnable, MS_U16 u16src, MS_U16 u16dst)
{
    MApi_GOP_GWIN_Set_HSCALE_EX(u8GOP, bEnable, u16src, u16dst);
}
void mstar_FB_SetVScale(MS_U8 u8GOP,MS_BOOL bEnable, MS_U16 u16src, MS_U16 u16dst)
{
    MApi_GOP_GWIN_Set_VSCALE_EX(u8GOP, bEnable, u16src, u16dst);
}
void mstar_FB_EnableGwin(MS_U8 u8GOP, MS_U8 u8win, MS_BOOL bEnable)
{
    MApi_GOP_GWIN_Enable(u8win, bEnable);
}
void mstar_FB_BeginTransaction(MS_U8 u8Gop)
{
    MApi_GOP_GWIN_UpdateRegOnceEx2(u8Gop, TRUE, TRUE);
}
void mstar_FB_EndTransaction(MS_U8 u8Gop)
{
    MApi_GOP_GWIN_UpdateRegOnceEx2(u8Gop, FALSE, TRUE);
}
void mstar_FB_SetForceWrite(MS_BOOL bEnable)
{
    MApi_GOP_GWIN_SetForceWrite(bEnable);
}
MS_U16 mstar_FB_GetBpp(DRV_FB_GOP_ColorFmt_e eColorFmt)
{
    MS_U16 u16bpp=0;

    switch (eColorFmt)
    {
        case E_DRV_FB_GOP_COLOR_RGB555_BLINK:
        case E_DRV_FB_GOP_COLOR_RGB565:
        case E_DRV_FB_GOP_COLOR_ARGB1555:
        case E_DRV_FB_GOP_COLOR_RGBA5551:
        case E_DRV_FB_GOP_COLOR_ARGB4444:
        case E_DRV_FB_GOP_COLOR_RGBA4444:
        case E_DRV_FB_GOP_COLOR_YUV422:
        case E_DRV_FB_GOP_COLOR_RGB555YUV422:
        case E_DRV_FB_GOP_COLOR_2266:
            u16bpp = 2;
            break;
        case E_DRV_FB_GOP_COLOR_ARGB8888:
        case E_DRV_FB_GOP_COLOR_ABGR8888:
            u16bpp = 4;
            break;
        case E_DRV_FB_GOP_COLOR_I8:
            u16bpp = 1;
            break;
        default:
            u16bpp = 0xFFFF;
            break;
    }
    return u16bpp;
}
void mstar_FB_SetTransClr_8888(MS_U8 u8Gop, MS_U32 clr, MS_U32 mask)
{
    MApi_GOP_GWIN_SetTransClr_8888_EX(u8Gop,clr,mask);
}
MS_BOOL mstar_FB_IsSupportMultiAlpha(MS_U8 u8Gop)
{
    //K6lite always support MultiAlpha
    //Should get capicity via utopia interface??
    return TRUE;
}
void mstar_FB_EnableMultiAlpha(MS_U8 u8Gop, MS_BOOL bEnable)
{
    MApi_GOP_GWIN_EnableMultiAlpha(u8Gop, bEnable);
}
void mstar_FB_SetAlpha0(MS_U8 u8GwinId, MS_U8 u8Alpha0)
{
    //K6lite not support it. I2 will implement it
 #if SUPPORT_SET_ARGB1555_ALPHA
    MApi_GOP_GWIN_SetARGB1555_Alpha(u8GwinId,FALSE,u8Alpha0);
 #endif
}
void mstar_FB_SetAlpha1(MS_U8 u8GwinId, MS_U8 u8Alpha1)
{
     //K6lite not support it. I2 will implement it
 #if SUPPORT_SET_ARGB1555_ALPHA
    MApi_GOP_GWIN_SetARGB1555_Alpha(u8GwinId,TRUE,u8Alpha1);
 #endif
}
void mstar_FB_SetNewAlphaMode(MS_U8 u8Gop, MS_U8 u8win, MS_BOOL bEnable)
{
    MApi_GOP_GWIN_SetNewAlphaMode(u8win,bEnable);
}
int mstar_FB_getCurOPTiming(int *ret_width, int *ret_height, int *ret_hstart)
{
    MS_PNL_DST_DispInfo dstDispInfo;
    memset(&dstDispInfo,0,sizeof(MS_PNL_DST_DispInfo));
    MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo));
    if(ret_width != NULL) {
        *ret_width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
    }
    if(ret_height != NULL) {
        *ret_height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;
    }
    if(ret_hstart != NULL) {
        *ret_hstart = dstDispInfo.DEHST;
    }
    return 0;
}
void mstar_FB_setHstart(MS_U8 u8GOP,MS_U16 u16PanelHStr)
{
    MApi_GOP_SetGOPHStart(u8GOP, u16PanelHStr);
}

MS_BOOL mstar_FB_CreateFBInfo(MS_U8 gwinId, MS_U16 width, MS_U16 height,
    DRV_FB_GOP_ColorFmt_e fmt, MS_U16 pitch, MS_PHY phyAddr)
{
    EN_GOP_COLOR_TYPE clrType = convertToGopColorType(fmt);
    MS_U32 fbId =  MApi_GOP_GWIN_Get32FBfromGWIN(gwinId);
    if (fbId != INVALID_FBID) {
       MApi_GOP_GWIN_DeleteFB(fbId);
    }
    MApi_GOP_GWIN_Create32FBFrom3rdSurf(width, height, clrType, phyAddr, pitch, &fbId);
    MApi_GOP_GWIN_MapFB2Win(fbId, gwinId);
    return TRUE;
}
void mstar_FB_DestroyFBInfo(MS_U8 gwinId)
{
    MS_U32 fbId =  MApi_GOP_GWIN_Get32FBfromGWIN(gwinId);
    if (fbId != INVALID_FBID) {
        MApi_GOP_GWIN_DeleteFB(fbId);
    }
}
MS_BOOL mstar_FB_SetOSDBlendingFormula(MS_U8 u8Gop,  MS_BOOL bPremul)
{
    E_APIXC_ReturnValue ret = E_APIXC_RET_OK;
    MS_U32 u32Layer = MApi_GOP_GWIN_GetLayerFromGOP(u8Gop);
    if (bPremul) {
        ret = MApi_XC_SetOSDBlendingFormula((E_XC_OSD_INDEX)u32Layer,
            E_XC_OSD_BlENDING_MODE1,MAIN_WINDOW);
    } else {
        ret = MApi_XC_SetOSDBlendingFormula((E_XC_OSD_INDEX)u32Layer,
            E_XC_OSD_BlENDING_MODE0,MAIN_WINDOW);
    }
    return (ret==E_APIXC_RET_OK);
}
void mstar_FB_EnableAlphaInverse(MS_U8 u8Gop, MS_BOOL bEnable)
{
    MApi_GOP_GWIN_SetAlphaInverse_EX(u8Gop, bEnable);
}
 MS_U32 mstar_FB_GetGopWorldUnit()
{
    MS_U32 gopWorldUnit = 32;
    MApi_GOP_GetChipCaps(E_GOP_CAP_WORD_UNIT, &gopWorldUnit, sizeof(MS_U32));
    return gopWorldUnit;
}
void mstar_FB_InitLayerSetting(MS_U32* u32GopIdx, MS_U32* u32LayerIdx, MS_U8 u8GopCount)
{
    int i = 0;
    GOP_LayerConfig stLayerSetting;
    memset(&stLayerSetting, 0, sizeof(GOP_LayerConfig));
    stLayerSetting.u32LayerCounts = u8GopCount;
    for(i=0;i < u8GopCount;i++) {
        stLayerSetting.stGopLayer[i].u32GopIndex = u32GopIdx[i];
        stLayerSetting.stGopLayer[i].u32LayerIndex = u32LayerIdx[i];
    }
    MApi_GOP_GWIN_SetLayer(&stLayerSetting, sizeof(GOP_LayerConfig));
}