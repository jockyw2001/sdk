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
/// @file   mi_gfx_impl.c
/// @brief gfx module impl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define _LINUX_OS_ (0)
#define _MI_SYS_ (0)

#if _LINUX_OS_
#include <linux/sched.h>
#include <string.h>
#endif

#include "mi_print.h"
#include "mi_gfx_impl.h"
#include "mi_gfx_datatype.h"
#include "mi_gfx_internal.h"

#include "mi_common.h"

#define TRUE (1)
#define FALSE (0)
#define NULL (0)
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define _BRINGUP_ (0)

#if _BRINGUP_
#include "apiGFX.h"
#endif
//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------
#define GFX_ISVALID_POINT(X)  \
    {   \
        if( X == NULL)  \
        {   \
            DBG_ERR("MI_ERR_INVALID_PARAMETER!\n");  \
            return E_MI_ERR_ILLEGAL_PARAM;   \
        }   \
    }   \

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct _MI_GFX_ResMgr_s
{
    MI_BOOL bInitFlag;
    MI_U8 Thresholdvalue;
} _MI_GFX_ResMgr_t;

#if 0
typedef struct _MI_GFX_CscOpt_s
{
    MI_GFX_Rgb2Yuv_e     eRgb2Yuv;  /** RGB转到YUV的格式*/
    MI_GFX_YuvOutRange_e eOutRange; /** YUV输出的范围*/
    MI_GFX_UvInRange_e   eInRange;  /** YUV输入的范围*/
    MI_GFX_Yuv422_e      eSrcFmt;   /** 源的YUV格式*/
    MI_GFX_Yuv422_e      eDstFmt;   /** 目的的YUV格式*/
} _MI_GFX_CscOpt_t;
#endif

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------
static _MI_GFX_ResMgr_t _stGfxMgr = {0};
//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

#if _BRINGUP_
static MS_ColorFormat _MI_GFX_MappingColorFmt(MI_GFX_ColorFmt_e eColorFmt)
{
    switch(eColorFmt)
    {
        case E_MI_GFX_FMT_ARGB8888:
            return E_MS_FMT_ARGB8888;
        case E_MI_GFX_FMT_RGB565:
            return E_MS_FMT_RGB565;
        case E_MI_GFX_FMT_ARGB4444:
            return E_MS_FMT_ARGB4444;
        case E_MI_GFX_FMT_ARGB1555:
            return E_MS_FMT_ARGB1555;
        //case E_MS_FMT_ARGB1555_DST:
            //return E_MS_FMT_ARGB1555_DST;
        case E_MI_GFX_FMT_YUV422:
            return E_MS_FMT_YUV422;
        case E_MI_GFX_FMT_I8:
            return E_MS_FMT_I8;
        case E_MI_GFX_FMT_I1:
            return E_MS_FMT_I1;
        case E_MI_GFX_FMT_I2:
            return E_MS_FMT_I2;
        case E_MI_GFX_FMT_I4:
            return E_MS_FMT_I4;
        default:
            return E_MS_FMT_ARGB8888;
    }
    return E_MS_FMT_ARGB8888;
}

static GFX_DFBBldOP _MI_GFX_GetDfbBlendMode(MI_GFX_DfbBldOp_e eDfbBlendMode)
{
    switch(eDfbBlendMode)
    {
        case E_MI_GFX_DFB_BLD_ZERO:
            return GFX_DFB_BLD_OP_ZERO;
        case E_MI_GFX_DFB_BLD_ONE:
            return GFX_DFB_BLD_OP_ONE;
        case E_MI_GFX_DFB_BLD_SRCCOLOR:
            return GFX_DFB_BLD_OP_SRCCOLOR;
        case E_MI_GFX_DFB_BLD_INVSRCCOLOR:
            return GFX_DFB_BLD_OP_INVSRCCOLOR;
        case E_MI_GFX_DFB_BLD_SRCALPHA:
            return GFX_DFB_BLD_OP_SRCALPHA;
        case E_MI_GFX_DFB_BLD_INVSRCALPHA:
            return GFX_DFB_BLD_OP_INVSRCALPHA;
        case E_MI_GFX_DFB_BLD_DESTALPHA:
            return GFX_DFB_BLD_OP_DESTALPHA;
        case E_MI_GFX_DFB_BLD_INVDESTALPHA:
            return GFX_DFB_BLD_OP_INVDESTALPHA;
        case E_MI_GFX_DFB_BLD_DESTCOLOR:
            return GFX_DFB_BLD_OP_DESTCOLOR;
        case E_MI_GFX_DFB_BLD_INVDESTCOLOR:
            return GFX_DFB_BLD_OP_INVDESTCOLOR;
        case E_MI_GFX_DFB_BLD_SRCALPHASAT:
            return GFX_DFB_BLD_OP_SRCALPHASAT;
        default:
        case E_MI_GFX_DFB_BLD_NONE:
            return GFX_DFB_BLD_OP_ZERO;
    }
}

static GFX_ROP2_Op _MI_GFX_GetROP(MI_GFX_RopCode_e eROP)
{
    switch(eROP)
    {
        case E_MI_GFX_ROP_BLACK:
            return ROP2_OP_ZERO;
        case E_MI_GFX_ROP_NOTMERGEPEN:
            return ROP2_OP_NOT_PS_OR_PD;
        case E_MI_GFX_ROP_MASKNOTPEN:
            return ROP2_OP_NS_AND_PD;
        case E_MI_GFX_ROP_NOTCOPYPEN:
            return ROP2_OP_NS;
        case E_MI_GFX_ROP_MASKPENNOT:
            return ROP2_OP_PS_AND_ND;
        case E_MI_GFX_ROP_NOT:
            return ROP2_OP_ND;
        case E_MI_GFX_ROP_XORPEN:
            return ROP2_OP_PS_XOR_PD;
        case E_MI_GFX_ROP_NOTMASKPEN:
            return ROP2_OP_NOT_PS_AND_PD;
        case E_MI_GFX_ROP_MASKPEN:
            return ROP2_OP_PS_AND_PD;
        case E_MI_GFX_ROP_NOTXORPEN:
            return ROP2_OP_PS_XOR_PD;
        case E_MI_GFX_ROP_NOP:
            return ROP2_OP_PD;
        case E_MI_GFX_ROP_MERGENOTPEN:
            return ROP2_OP_NS_OR_PD;
        case E_MI_GFX_ROP_COPYPEN:
            return ROP2_OP_PS;
        case E_MI_GFX_ROP_MERGEPENNOT:
            return ROP2_OP_PS_OR_ND;
        case E_MI_GFX_ROP_MERGEPEN:
            return ROP2_OP_PD_OR_PS;
        case E_MI_GFX_ROP_WHITE:
            return ROP2_OP_ONE;
        default:
            break;
    }
    return ROP2_OP_PS;
}

static MI_S32 _MI_GFX_GetDefaultBlitOpt(MI_GFX_Opt_t *pstBlitOpt)
{
    pstBlitOpt->bEnGfxRop = FALSE;
    pstBlitOpt->eRopCodeColor = E_MI_GFX_ROP_NONE;

    pstBlitOpt->eSrcDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    pstBlitOpt->eDstDfbBldOp = E_MI_GFX_DFB_BLD_NONE;

    pstBlitOpt->eMirror  = E_MI_GFX_MIRROR_NONE;
    pstBlitOpt->eRotate  =  E_MI_GFX_ROTATE_0;

    pstBlitOpt->eSrcYUVFmt = E_MI_GFX_YUV_YVYU;
    pstBlitOpt->eDstYUVFmt = E_MI_GFX_YUV_YVYU;
    pstBlitOpt->stClipRect.s32Xpos = 0;
    pstBlitOpt->stClipRect.s32Ypos = 0;
    pstBlitOpt->stClipRect.u32Width  = 0;
    pstBlitOpt->stClipRect.u32Height = 0;

    return MI_SUCCESS;
}

static MI_S32 _MI_GFX_SetBitBlitOption(MI_GFX_Opt_t stBlitOpt)
{
    if (_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module has been initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }

    //handle colorkey
    MI_GFX_ColorKeyOp_e eSrcCKeyOP = E_MI_GFX_RGB_OP_EQUAL;
    MI_GFX_ColorKeyOp_e eDstCKeyOP = E_MI_GFX_RGB_OP_EQUAL;
    MI_BOOL bEnSrcColorKey = FALSE;
    MI_BOOL bEnDstColorKey = FALSE;
    GFX_ColorKeyMode eColorKeyMode = CK_OP_EQUAL;
    GFX_Buffer_Format u8SrcFmt = E_MS_FMT_ARGB8888;
    GFX_Buffer_Format u8DstFmt = E_MS_FMT_ARGB8888;
    GFX_RgbColor eColorkeySrc_s = {0,0,0,0};
    GFX_RgbColor eColorkeySrc_e = {0,0,0,0};
    GFX_RgbColor eColorkeyDst_s = {0,0,0,0};
    GFX_RgbColor eColorkeyDst_e = {0,0,0,0};

    bEnSrcColorKey = stBlitOpt.stSrcColorKeyInfo.bEnColorKey;
    eSrcCKeyOP = stBlitOpt.stSrcColorKeyInfo.eCKeyOp;
    u8SrcFmt = (GFX_Buffer_Format)_MI_GFX_MappingColorFmt(stBlitOpt.stSrcColorKeyInfo.eCKeyFmt);

    eColorkeySrc_s.b = (stBlitOpt.stSrcColorKeyInfo.stCKeyVal.Color_s >> 24) & 0xFF;
    eColorkeySrc_s.g = (stBlitOpt.stSrcColorKeyInfo.stCKeyVal.Color_s >> 16) & 0xFF;
    eColorkeySrc_s.r = (stBlitOpt.stSrcColorKeyInfo.stCKeyVal.Color_s >> 8) & 0xFF;
    eColorkeySrc_s.a = stBlitOpt.stSrcColorKeyInfo.stCKeyVal.Color_s & 0xFF;

    eColorkeySrc_e.b = (stBlitOpt.stSrcColorKeyInfo.stCKeyVal.Color_e >> 24) & 0xFF;
    eColorkeySrc_e.g = (stBlitOpt.stSrcColorKeyInfo.stCKeyVal.Color_e >> 16) & 0xFF;
    eColorkeySrc_e.r = (stBlitOpt.stSrcColorKeyInfo.stCKeyVal.Color_e >> 8) & 0xFF;
    eColorkeySrc_e.a = stBlitOpt.stSrcColorKeyInfo.stCKeyVal.Color_e & 0xFF;

    if(E_MI_GFX_ARGB_OP_EQUAL == eSrcCKeyOP) // process rgb & alpha
    {
        eColorKeyMode = CK_OP_EQUAL;
        MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
        eColorKeyMode = AK_OP_EQUAL;
        MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
    }
    else if(E_MI_GFX_ARGB_OP_NOT_EQUAL == eSrcCKeyOP)
    {
        eColorKeyMode = CK_OP_NOT_EQUAL;
        MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
        eColorKeyMode = AK_OP_NOT_EQUAL;
        MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
    }
    else
    {
        eColorKeyMode = (GFX_ColorKeyMode)eSrcCKeyOP;
        MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
    }

    bEnDstColorKey = stBlitOpt.stDstColorKeyInfo.bEnColorKey;
    eDstCKeyOP = stBlitOpt.stDstColorKeyInfo.eCKeyOp;
    u8DstFmt = (GFX_Buffer_Format)_MI_GFX_MappingColorFmt(stBlitOpt.stDstColorKeyInfo.eCKeyFmt);

    eColorkeySrc_s.b = (stBlitOpt.stDstColorKeyInfo.stCKeyVal.Color_s >> 24) & 0xFF;
    eColorkeySrc_s.g = (stBlitOpt.stDstColorKeyInfo.stCKeyVal.Color_s >> 16) & 0xFF;
    eColorkeySrc_s.r = (stBlitOpt.stDstColorKeyInfo.stCKeyVal.Color_s >> 8) & 0xFF;
    eColorkeySrc_s.a = stBlitOpt.stDstColorKeyInfo.stCKeyVal.Color_s & 0xFF;

    eColorkeySrc_e.b = (stBlitOpt.stDstColorKeyInfo.stCKeyVal.Color_e >> 24) & 0xFF;
    eColorkeySrc_e.g = (stBlitOpt.stDstColorKeyInfo.stCKeyVal.Color_e >> 16) & 0xFF;
    eColorkeySrc_e.r = (stBlitOpt.stDstColorKeyInfo.stCKeyVal.Color_e >> 8) & 0xFF;
    eColorkeySrc_e.a = stBlitOpt.stDstColorKeyInfo.stCKeyVal.Color_e & 0xFF;

    if(E_MI_GFX_ARGB_OP_EQUAL == eSrcCKeyOP) // process rgb & alpha
    {
        eColorKeyMode = CK_OP_EQUAL;
        MApi_GFX_SetDrcColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
        eColorKeyMode = AK_OP_EQUAL;
        MApi_GFX_SetDrcColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
    }
    else if(E_MI_GFX_ARGB_OP_NOT_EQUAL == eSrcCKeyOP)
    {
        eColorKeyMode = CK_OP_NOT_EQUAL;
        MApi_GFX_SetDrcColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
        eColorKeyMode = AK_OP_NOT_EQUAL;
        MApi_GFX_SetDrcColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
    }
    else
    {
        eColorKeyMode = (GFX_ColorKeyMode)eSrcCKeyOP;
        MApi_GFX_SetDrcColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &eColorkeySrc_s, &eColorkeySrc_e);
    }

    //handle alpha blend mode
    MApi_GFX_EnableAlphaBlending(FALSE);
    GFX_DFBBldOP eSrcDfbBldOp = _MI_GFX_GetDfbBlendMode(stBlitOpt.eSrcDfbBldOp);
    GFX_DFBBldOP eDstDfbBldOp = _MI_GFX_GetDfbBlendMode(stBlitOpt.eDstDfbBldOp);
    MApi_GFX_SetDFBBldOP(eSrcDfbBldOp, eDstDfbBldOp);
    MApi_GFX_EnableDFBBlending(TRUE);

    //handle rop mode
    MI_BOOL bRopEnable = (E_MI_GFX_ROP_NONE == stBlitOpt.bEnGfxRop)?FALSE:TRUE;
    GFX_ROP2_Op eRopMode = _MI_GFX_GetROP(stBlitOpt.eRopCode);
    MApi_GFX_SetROP2(bRopEnable, eRopMode);

    //handle YUV fmt
    MApi_GFX_SetDC_CSC_FMT(GFX_YUV_RGB2YUV_PC, GFX_YUV_OUT_PC, GFX_YUV_IN_255, stBlitOpt.eSrcYuvFmt, stBlitOpt.eDstYuvFmt);

    //handle rotation
    MApi_GFX_SetRotate((GFX_RotateAngle)stBlitOpt.eRotate);

    return MI_SUCCESS;
}

static MI_S32 _MI_GFX_ResetOption()
{
    MI_GFX_Opt_t stBlitOpt;

    _MI_GFX_GetDefaultBlitOpt(&stBlitOpt);
    _MI_GFX_SetBitBlitOption(stBlitOpt); //need reset?

    MApi_GFX_SetMirror(FALSE, FALSE);

    return MI_SUCCESS;
}
#endif //end of _BRINGUP_

MI_S32 MI_GFX_IMPL_Open()
{
    if (_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module has been initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }
#if _BRINGUP_
    GFX_Config stGFXcfg;
    MI_S8 sMMapName[30];
    memset(sMMapName, 0, sizeof(sMMapName));
    memset(&stGFXcfg,0,sizeof(GFX_Config));
    sprintf((char *)sMMapName, "%s", MI_GEVQ_MMAP_ITEM); /* use mmap.ini ??? */
    MMapInfo_t* pMMap = MMAPInfo::GetInstance((EN_MMAP_Type)0)->get_mmap(MMAPInfo::GetInstance((EN_MMAP_Type)MMAP_TYPE_ORIGINAL)->StrToMMAPID((char *)sMMapName));
    if(pMMap)
    {
        DBG_INFO("Get MMAP Info [%s]\n", sMMapName);
        DBG_INFO("u32Addr=0x%x, u32Size=0x%x miu=0x%x\n", pMMap->u32Addr, pMMap->u32Size, pMMap->u32MiuNo);
        stGFXcfg.u32VCmdQAddr = pMMap->u32Addr;
        stGFXcfg.u32VCmdQSize = pMMap->u32Size;
        stGFXcfg.u8Miu = pMMap->u32MiuNo;
    }
    else
    {
        DBG_ERR("Get MMAP Info [%s] failed!!!\n", sMMapName);
        return E_MI_ERR_FAILED;
    }
    stGFXcfg.bIsCompt = TRUE;
    stGFXcfg.bIsHK = TRUE;
    MApi_GFX_Init(&stGFXcfg);

    if(pMMap->u32Size >= 4*1024)
    {
        GFX_VcmqBufSize vqSize;
        GFX_Result ret = GFX_FAIL;

        if(pMMap->u32Size >= 512*1024)
            vqSize = GFX_VCMD_512K;
        else if(pMMap->u32Size >= 256*1024)
            vqSize = GFX_VCMD_256K;
        else if(pMMap->u32Size >= 128*1024)
            vqSize = GFX_VCMD_128K;
        else if(pMMap->u32Size >= 64*1024)
            vqSize = GFX_VCMD_64K;
        else if(pMMap->u32Size >= 32*1024)
            vqSize = GFX_VCMD_32K;
        else if(pMMap->u32Size >= 16*1024)
            vqSize = GFX_VCMD_16K;
        else if(pMMap->u32Size >= 8*1024)
            vqSize = GFX_VCMD_8K;
        else
            vqSize = GFX_VCMD_4K;
        ret = MApi_GFX_SetVCmdBuffer(pMMap->u32Addr, vqSize);

        if(GFX_SUCCESS == ret)
            MApi_GFX_EnableVCmdQueue(TRUE);
        else
            MApi_GFX_EnableVCmdQueue(FALSE);
    }
    else
    {
        MApi_GFX_EnableVCmdQueue(FALSE);
    }
#endif
    _stGfxMgr.bInitFlag = TRUE;
    _stGfxMgr.Thresholdvalue = 0xFF; //default

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_Close()
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }
#if _BRINGUP_
    //driver not implement , need add func: GFX_Result MApi_GFX_Close(MS_U8* u8Thres)
#endif
    _stGfxMgr.bInitFlag = FALSE;

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_WaitAllDone(MI_BOOL WaitAllDone, MI_U16 TargetFence)
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }
#if _BRINGUP_
    if (TRUE == WaitAllDone)
    {
        MI_U16 u16TagId = MApi_GFX_SetNextTAGID();
        MApi_GFX_WaitForTAGID(u16TagId);
    }
    else
    {
        if (0xFFFF != TargetFence)
        {
            MApi_GFX_WaitForTAGID(u16TagId);
        }
    }
    _MI_GFX_ResetOption();//如果都不等待，则直接reset可能会有问题
#endif
    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_QuickFill(MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect,
    MI_U32 u32ColorVal, MI_U16 *pu16Fence)
{
#if _BRINGUP_
    MI_GFX_Opt_t stBlitOpt;
#endif

    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }

#if _BRINGUP_
    GFX_ISVALID_POINT(pstDst);
    GFX_ISVALID_POINT(pstDstRect);
    GFX_ISVALID_POINT(pu16Fence);
    memset(&stBlitOpt, 0x0, sizeof(MI_GFX_Opt_t));
    _MI_GFX_GetDefaultBlitOpt(&stBlitOpt);

    //set destination area color
    GFX_RgbColor stColor = {0,0,0,0};
    memset(&stColor, 0x0,sizeof(GFX_RgbColor));
    stColor.b = (u32ColorVal >> 24) & 0xFF;
    stColor.g = (u32ColorVal >> 16) & 0xFF;
    stColor.r = (u32ColorVal >> 8) & 0xFF;
    stColor.a = (u32ColorVal) & 0xFF;

    // set destination clip area
    GFX_Point v0, v1;
    v0.x = 0;
    v0.y = 0;
    v1.x = pstDstRect->u32Width;
    v1.y = pstDstRect->u32Height;
    MApi_GFX_SetClip(&v0, &v1);

    // set destination buffer info
    GFX_BufferInfo dstBuff;
    dstBuff.u32Addr = MS_VA2PA(pstDst->u32VirAddr);
    dstBuff.u32Width = pstDst->u32Width;
    dstBuff.u32Height = pstDst->u32Height;
    dstBuff.u32Pitch = pstDst->u32Stride;
    dstBuff.u32ColorFmt = pstDst->eColorFmt;
    if(E_MI_GFX_FMT_ARGB1555 == pstDst->eColorFmt)
        dstBuff.u32ColorFmt = E_MI_GFX_FMT_ARGB1555_DST;
    MApi_GFX_SetDstBufferInfo(&dstBuff,0);

    //Config the fill rect params to vars
    GFX_RectFillInfo fillblk;
    fillblk.dstBlock.x = pstDstRect->s32Xpos;
    fillblk.dstBlock.y = pstDstRect->s32Ypos;
    fillblk.dstBlock.width = pstDstRect->u32Width;
    fillblk.dstBlock.height = pstDstRect->u32Height;
    fillblk.colorRange.color_s = stColor;
    fillblk.colorRange.color_e = stColor;
    fillblk.fmt = pstDst->eColorFmt;
    fillblk.flag = GFXRECT_FLAG_COLOR_CONSTANT;
    MApi_GFX_RectFill(&fillblk);
    *pu16Fence = MApi_GFX_SetNextTAGID();
#endif

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_GetAlphaThresholdValue(MI_U8 *pu8ThresholdValue)
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }

#if _BRINGUP_
    GFX_ISVALID_POINT(pu8ThresholdValue);
    *pu8ThresholdValue = _stGfxMgr.Thresholdvalue;
#endif

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_SetAlphaThresholdValue(MI_U8 u8ThresholdValue)
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }
#if _BRINGUP_

    //driver not implement , need add func: GFX_Result MApi_GFX_Close(MS_U8* u8Thres)
#endif
    _stGfxMgr.Thresholdvalue = u8ThresholdValue;

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_BitBlit(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect, MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence)
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }
#if _BRINGUP_
    GFX_DrawRect bitbltInfo;
    GFX_BufferInfo gfxSrcBuf, gfxDstBuf;
    GFX_Point v0, v1;

    GFX_ISVALID_POINT(pstSrc);
    GFX_ISVALID_POINT(pstSrcRect);
    GFX_ISVALID_POINT(pstDst);
    GFX_ISVALID_POINT(pstDstRect);
    GFX_ISVALID_POINT(pu16Fence);

    if (NULL == pstOpt)
    {
        //set default option
    }
    else
    {
        if (MI_SUCCESS == _MI_GFX_SetBitBlitOption(*pstOpt))
        {
            DBG_INFO("_MI_GFX_SetBitBlitOption success\n");
        }
        else
        {
            DBG_ERR("_MI_GFX_SetBitBlitOption Fail!!!\n");
            return E_MI_ERR_FAILED;
        }
    }

    switch(pstOpt->eMirror)
    {
        case E_MI_GFX_MIRROR_VERTICAL:
            pstSrcRect->s32Xpos += pstSrcRect->u32Width -1;
            MApi_GFX_SetMirror(TRUE, FALSE);
            break;
        case E_MI_GFX_MIRROR_HORIZONTAL:
            pstSrcRect->s32Ypos += pstSrcRect->u32Height -1;
            MApi_GFX_SetMirror(FALSE, TRUE);
            break;
        case E_MI_GFX_MIRROR_BOTH:
            pstSrcRect->s32Xpos += pstSrcRect->u32Width -1;
            pstSrcRect->s32Ypos += pstSrcRect->u32Height -1;
            MApi_GFX_SetMirror(TRUE, TRUE);
            break;
        case E_MI_GFX_MIRROR_NONE:
        default:
            MApi_GFX_SetMirror(FALSE, FALSE);
            break;
    }
    gfxSrcBuf.u32ColorFmt = pstSrc->eColorFmt;
    gfxSrcBuf.u32Addr = MS_VA2PA(pstSrc->u32VirAddr);
    gfxSrcBuf.u32Pitch = pstSrc->u32Stride;
    gfxSrcBuf.u32Width = pstSrc->u32Width;
    gfxSrcBuf.u32Height = pstSrc->u32Height;
    MApi_GFX_SetSrcBufferInfo(&gfxSrcBuf, 0);

    gfxDstBuf.u32ColorFmt = pstDst->eColorFmt;
    gfxDstBuf.u32Addr = MS_VA2PA(pstDst->u32VirAddr);
    gfxDstBuf.u32Pitch = pstDst->u32Stride;
    gfxDstBuf.u32Width = pstDst->u32Width;
    gfxDstBuf.u32Height = pstDst->u32Height;
    if(E_MI_GFX_FMT_ARGB1555 == gfxDstBuf.u32ColorFmt)
        gfxDstBuf.u32ColorFmt = E_MI_GFX_FMT_ARGB1555_DST;
    MApi_GFX_SetDstBufferInfo(&gfxDstBuf, 0);

    v0.x = pstDstRect->s32Xpos;
    v0.y = pstDstRect->s32Ypos;
    v1.x = pstDstRect->u32Width + v0.x;
    v1.y = pstDstRect->u32Height + v0.y;
    MApi_GFX_SetClip(&v0, &v1);

    bitbltInfo.srcblk.height = pstSrcRect->u32Height;
    bitbltInfo.srcblk.width = pstSrcRect->u32Width;
    bitbltInfo.srcblk.x = pstSrcRect->s32Xpos;
    bitbltInfo.srcblk.y = pstSrcRect->s32Ypos;

    bitbltInfo.dstblk.height = pstDstRect->u32Height;
    bitbltInfo.dstblk.width = pstSrcRect->u32Width;
    bitbltInfo.dstblk.x = pstDstRect->s32Xpos;
    bitbltInfo.dstblk.y = pstDstRect->s32Ypos;

    MI_U32 u32DrawFlag = 0;
    if ((pstSrcRect->u32Width == pstSrcRect->u32Width) && (pstSrcRect->u32Height == pstDstRect->u32Height))
    {
        u32DrawFlag = GFXDRAW_FLAG_DEFAULT;
    }
    else  // stretch bitblt, driver will switch  to 1P mode automatically
    {
        u32DrawFlag = GFXDRAW_FLAG_SCALE; // scale
    }
    GFX_Result rRet = MApi_GFX_BitBlt(&bitbltInfo, u32DrawFlag);
    if(rRet != GFX_SUCCESS)
    {
        DBG_INFO("rRet :%d \n",rRet);
    }
    *pu16Fence = MApi_GFX_SetNextTAGID();

#endif
    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_MultiBlending(MI_GFX_SurfaceList_t *pstSurfaceList, MI_U16 *pu16Fence)
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }
#if _BRINGUP_
    MI_S32 i = 0, retval = 0;
    GFX_ISVALID_POINT(pstSurfaceList);

    for (i = 0; i < pstSurfaceList->u32SurfaceNum; i++)
    {
        retval |= MI_GFX_IMPL_BitBlit(pstSurfaceList->pstComposor.stSrcSurface, pstSurfaceList->pstComposor.stInRect,
        pstSurfaceList->pDstSurface, pstSurfaceList->pstComposor.stOutRect, pstSurfaceList->pstComposor.stOpt, pu16Fence);
        if (MI_SUCCESS != retval)
        {
            DBG_ERR("MI_GFX_IMPL_MultiBlending err(0x%x)\n", retval);
            return retval;
        }
    }
#endif

    return MI_SUCCESS;
}