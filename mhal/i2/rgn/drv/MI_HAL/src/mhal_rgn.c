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

////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
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

#define __MHAL_RGN_C__

#if defined(CAM_OS_RTK)
#include "cam_os_wrapper.h"
#else
#include "ms_platform.h"
#endif

#include "mhal_rgn.h"

#include "drv_gop.h"
#include "drv_cover.h"

MS_BOOL _MHAL_RGN_CoverTransId(MHAL_RGN_CoverType_e eCoverId, DrvCoverIdType_e *pDrvId)
{
    MS_BOOL bRet = TRUE;

    switch(eCoverId)
    {
        case E_MHAL_COVER_VPE_PORT0:
            *pDrvId = E_DRV_ISPSC1_COVER_ID_0;
            break;

        case E_MHAL_COVER_VPE_PORT3:
            *pDrvId = E_DRV_ISPSC4_COVER_ID_1;
            break;

        case E_MHAL_COVER_DIVP_PORT0:
            *pDrvId = E_DRV_DIP_COVER_ID_2;
            break;

        case E_MHAL_COVER_TYPE_MAX:
        default:
            *pDrvId = E_DRV_COVER_ID_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}

MS_BOOL _MHAL_RGN_CmdqCoverTransId(MHAL_RGN_CmdqType_e eCoverCmdId, DrvCoverCmdqIdType_e *pDrvId)
{
    MS_BOOL bRet = TRUE;

    switch(eCoverCmdId)
    {
        case E_MHAL_RGN_VPE_CMDQ:
            *pDrvId = E_DRV_COVER_CMDQ_VPE_ID_0;
            break;

        case E_MHAL_RGN_DIVP_CMDQ:
            *pDrvId = E_DRV_COVER_CMDQ_DIVP_ID_1;
            break;
        default:
            *pDrvId = E_DRV_COVER_CMDQ_ID_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}

MS_BOOL _MHAL_RGN_CmdqGopTransId(MHAL_RGN_CmdqType_e eGopCmdId, DrvGopCmdqIdType_e *pDrvId)
{
    MS_BOOL bRet = TRUE;

    switch(eGopCmdId)
    {
        case E_MHAL_RGN_VPE_CMDQ:
            *pDrvId = E_DRV_GOP_VPE_CMDQ_ID_0;
            break;

        case E_MHAL_RGN_DIVP_CMDQ:
            *pDrvId = E_DRV_GOP_DIVP_CMDQ_ID_1;
            break;
        default:
            *pDrvId = E_DRV_GOP_CMDQ_ID_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}


MS_BOOL _MHAL_RGN_GopTransId(MHAL_RGN_GopType_e eGopId, DrvGopIdType_e *pDrvId)
{
    MS_BOOL bRet = TRUE;

    switch(eGopId)
    {
        case E_MHAL_GOP_VPE_PORT0:
            *pDrvId = E_DRV_ISPGOP_ID_0;
            break;

        case E_MHAL_GOP_VPE_PORT1:
            *pDrvId = E_DRV_ISPGOP_ID_1;
            break;

        case E_MHAL_GOP_VPE_PORT2:
            *pDrvId = E_DRV_ISPGOP_ID_2;
            break;

        case E_MHAL_GOP_VPE_PORT3:
            *pDrvId = E_DRV_ISPGOP_ID_3;
            break;

        case E_MHAL_GOP_DIVP_PORT0:
            *pDrvId = E_DRV_DIPGOP_ID_4;
            break;

        case E_MHAL_GOP_TYPE_MAX:
        default:
            *pDrvId = E_DRV_GOP_ID_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}

MS_BOOL _MHAL_RGN_GopTransSrcFmt(MHAL_RGN_GopPixelFormat_e eRgnSrcFmt, DrvGopGwinSrcFmtType_e *pDrvSrcFmt)
{
    MS_BOOL bRet = TRUE;

    switch(eRgnSrcFmt)
    {
        case E_MHAL_RGN_PIXEL_FORMAT_ARGB1555:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_ARGB1555;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_ARGB4444:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_ARGB4444;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_I2:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_I2_PALETTE;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_I4:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_I4_PALETTE;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_MAX:
        default:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}

/*Setup cmdq*/
MS_S32 MHAL_RGN_SetupCmdQ(MHAL_CMDQ_CmdqInterface_t* pstCmdInf,MHAL_RGN_CmdqType_e eRgnCmdqId)
{
    DrvGopCmdqIdType_e eGopDrvId;
    DrvCoverCmdqIdType_e eCoverDrvId;
    if(pstCmdInf)
    {
        if(_MHAL_RGN_CmdqGopTransId(eRgnCmdqId,&eGopDrvId))
        {
            DrvGopRgnSetCmdq(pstCmdInf,eGopDrvId);
        }
        else
        {
            return MHAL_FAILURE;
        }

        if(_MHAL_RGN_CmdqCoverTransId(eRgnCmdqId,&eCoverDrvId))
        {
            DrvCoverRgnSetCmdq(pstCmdInf,eCoverDrvId);
        }
        else
        {
            return MHAL_FAILURE;
        }
    }
    return MHAL_SUCCESS;
}

/*Cover init*/
MS_S32 MHAL_RGN_CoverInit(void)
{
    DrvCoverRgnInit();

    return MHAL_SUCCESS;
}

/*Set cover color NA:[31~24] Cr:[23~16] Y:[15~8] Cb:[7~0]*/
MS_S32 MHAL_RGN_CoverSetColor(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer, MS_U32 u32Color)
{
    DrvCoverIdType_e eDrvCoverId;
    DrvCoverWinIdType_e eDrvWinId;
    DrvCoverColorConfig_t tColorCfg;

    if(_MHAL_RGN_CoverTransId(eCoverId, &eDrvCoverId) == TRUE)
    {
        eDrvWinId = (DrvCoverWinIdType_e)eLayer;
        tColorCfg.u8B = (MS_U8)(u32Color & 0xFF);
        tColorCfg.u8G = (MS_U8)((u32Color>>8) & 0xFF);
        tColorCfg.u8R = (MS_U8)((u32Color>>16) & 0xFF);
        DrvCoverSetColor(eDrvCoverId, eDrvWinId, tColorCfg);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Cover  size*/
MS_S32 MHAL_RGN_CoverSetWindow(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer, MS_U32 u32X, MS_U32 u32Y, MS_U32 u32Width, MS_U32 u32Height)
{
    DrvCoverIdType_e eDrvCoverId;
    DrvCoverWinIdType_e eDrvWinId;
    DrvCoverWindowConfig_t tCoverWinCfg;

    if(_MHAL_RGN_CoverTransId(eCoverId, &eDrvCoverId) == TRUE)
    {
        eDrvWinId = (DrvCoverWinIdType_e)eLayer;
        tCoverWinCfg.u16X = (MS_U16)u32X;
        tCoverWinCfg.u16Y = (MS_U16)u32Y;
        tCoverWinCfg.u16Width = (MS_U16)u32Width;
        tCoverWinCfg.u16Height = (MS_U16)u32Height;
        DrvCoverSetWinSize(eDrvCoverId, eDrvWinId, tCoverWinCfg);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Enable cover*/
MS_S32 MHAL_RGN_CoverEnable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer)
{
    DrvCoverIdType_e eDrvCoverId;
    DrvCoverWinIdType_e eDrvWinId;
    MS_BOOL bEn = TRUE;

    if(_MHAL_RGN_CoverTransId(eCoverId, &eDrvCoverId) == TRUE)
    {
        eDrvWinId = (DrvCoverWinIdType_e)eLayer;
        DrvCoverSetEnableWin(eDrvCoverId, eDrvWinId, bEn);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Disable cover*/
MS_S32 MHAL_RGN_CoverDisable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer)
{
    DrvCoverIdType_e eDrvCoverId;
    DrvCoverWinIdType_e eDrvWinId;
    MS_BOOL bDisEn = FALSE;

    if(_MHAL_RGN_CoverTransId(eCoverId, &eDrvCoverId) == TRUE)
    {
        eDrvWinId = (DrvCoverWinIdType_e)eLayer;
        DrvCoverSetEnableWin(eDrvCoverId, eDrvWinId, bDisEn);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Gop init*/
MS_S32 MHAL_RGN_GopInit(void)
{
    DrvGopRgnInit();
    return MHAL_SUCCESS;
}

/*Set palette*/
MS_S32 MHAL_RGN_GopSetPalette(MHAL_RGN_GopType_e eGopId, MS_U8 u8Alpha, MS_U8 u8Red, MS_U8 u8Green, MS_U8 u8Blue, MS_U8 u8Idx)
{
    DrvGopIdType_e eDrvGopId;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopSetPaletteRiuOneMem(eDrvGopId, u8Idx, u8Alpha, u8Red, u8Green, u8Blue);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Set base gop window size*/
MS_S32 MHAL_RGN_GopSetBaseWindow(MHAL_RGN_GopType_e eGopId, MS_U32 u32Width, MS_U32 u32Height)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopWindowConfig_t tDrvWinCfg;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        tDrvWinCfg.u16X = 0;
        tDrvWinCfg.u16Y = 0;
        tDrvWinCfg.u16Width = (MS_U16)u32Width;
        tDrvWinCfg.u16Height = (MS_U16)u32Height;
        DrvGopSetStretchWindowSize(eDrvGopId, tDrvWinCfg);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Set base gop gwin pix format*/
MS_S32 MHAL_RGN_GopGwinSetPixelFormat(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopPixelFormat_e eFormat)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopGwinSrcFmtType_e eSrcFmt;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        if(_MHAL_RGN_GopTransSrcFmt(eFormat, &eSrcFmt)== TRUE)
        {
            DrvGopSetSrcFmt(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, eSrcFmt);
            return MHAL_SUCCESS;
        }
        else
        {
            return MHAL_FAILURE;
        }
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set base gop gwin osd window*/
MS_S32 MHAL_RGN_GopGwinSetWindow(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_U32 u32Width, MS_U32 u32Height, MS_U32 u32Stride, MS_U32 u32X, MS_U32 u32Y)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopWindowConfig_t tDrvWinCfg;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        //set gwin size
        tDrvWinCfg.u16X = (MS_U16)u32X;
        tDrvWinCfg.u16Y = (MS_U16)u32Y;
        tDrvWinCfg.u16Width = (MS_U16)u32Width;
        tDrvWinCfg.u16Height = (MS_U16)u32Height;
        DrvGopSetGwinSizeWithoutFmt(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, tDrvWinCfg);

        // set memory pitch
        DrvGopSetMemPitchDirect(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, u32Stride);

        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Set base gop gwin osd bufdfer addr*/
MS_S32 MHAL_RGN_GopGwinSetBuffer(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_PHYADDR phyAddr)
{
    DrvGopIdType_e eDrvGopId;
    MS_U32 u32BaseAddr=0;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        u32BaseAddr = phyAddr;
        DrvGopUpdateBase(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, u32BaseAddr);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*enable Gwin*/
MS_S32 MHAL_RGN_GopGwinEnable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId)
{
    DrvGopIdType_e eDrvGopId;
    MS_BOOL bEn = TRUE;
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopRgnSetEnable(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, bEn);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*disable Gwin*/
MS_S32 MHAL_RGN_GopGwinDisable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId)
{
    DrvGopIdType_e eDrvGopId;
    MS_BOOL bEn = FALSE;
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopRgnSetEnable(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, bEn);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set gop colorkey*/
MS_S32 MHAL_RGN_GopSetColorkey(MHAL_RGN_GopType_e eGopId, MS_BOOL bEn, MS_U8 u8R, MS_U8 u8G, MS_U8 u8B)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopColorKeyConfig_t tColorKeyCfg;
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        tColorKeyCfg.bEn = bEn;
        tColorKeyCfg.u8R = u8R;
        tColorKeyCfg.u8G = u8G;
        tColorKeyCfg.u8B = u8B;
        DrvGopSetColorKey(eDrvGopId, tColorKeyCfg);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set gop alpha blending type*/
MS_S32 MHAL_RGN_GopSetAlphaType(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinAlphaType_e eAlphaType, MS_U8 u8ConstAlphaVal)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopGwinAlphaConfig_t tAlphaCfg;

    switch(eAlphaType)
    {
        case E_MHAL_GOP_GWIN_ALPHA_CONSTANT:
            tAlphaCfg.eAlphaType = E_DRV_GOP_GWIN_ALPHA_CONSTANT;
            break;

        case E_MHAL_GOP_GWIN_ALPHA_PIXEL:
            tAlphaCfg.eAlphaType = E_DRV_GOP_GWIN_ALPHA_PIXEL;
            break;

        default:
            return MHAL_FAILURE;
            break;

    }

    tAlphaCfg.u8ConstantAlpahValue = u8ConstAlphaVal;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopSetAlphaBlending(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, tAlphaCfg);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set gop ARGB1555 alpha value for alpha0 or alpha1*/
MS_S32 MHAL_RGN_GopSetArgb1555AlphaVal(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinArgb1555Def_e eAlphaType, MS_U8 u8AlphaVal)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopGwinArgb1555Def_e eDrvAlphaType;

    switch(eAlphaType)
    {
        case E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0:
            eDrvAlphaType = E_DRV_GOP_GWIN_ARGB1555_DEFINE_ALPHA0;
            break;

        case E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1:
            eDrvAlphaType = E_DRV_GOP_GWIN_ARGB1555_DEFINE_ALPHA1;
            break;

        default:
            return MHAL_FAILURE;
            break;

    }

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopSetArgb1555AlphaDefVal(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, eDrvAlphaType, u8AlphaVal);
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}
MS_S32 MHAL_RGN_GetChipCapability(MHAL_RGN_ChipCapType_e eType, MHAL_RGN_GopPixelFormat_e eFormat)
{
    switch(eType) {
        case E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT:
            if(eFormat <= E_MHAL_RGN_PIXEL_FORMAT_I4) {
                return 1;
            } else {
                return 0;
            }
        case E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT:   // unit:pixel
        case E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT:
            switch(eFormat) {
                case E_MHAL_RGN_PIXEL_FORMAT_I2:
                    return 4;
                default:
                    return 2;
            }
        case E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT:
            return 16; // unit:byte
        case E_MHAL_RGN_CHIP_OSD_HW_GWIN_CNT: // gwin number
            return 2;
        default:
            return MHAL_FAILURE;
    }
}
#if !defined(CAM_OS_RTK)
EXPORT_SYMBOL(MHAL_RGN_SetupCmdQ);
EXPORT_SYMBOL(MHAL_RGN_CoverInit);
EXPORT_SYMBOL(MHAL_RGN_CoverSetColor);
EXPORT_SYMBOL(MHAL_RGN_CoverSetWindow);
EXPORT_SYMBOL(MHAL_RGN_CoverEnable);
EXPORT_SYMBOL(MHAL_RGN_CoverDisable);
EXPORT_SYMBOL(MHAL_RGN_GopInit);
EXPORT_SYMBOL(MHAL_RGN_GopSetPalette);
EXPORT_SYMBOL(MHAL_RGN_GopSetBaseWindow);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetPixelFormat);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetWindow);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetBuffer);
EXPORT_SYMBOL(MHAL_RGN_GopGwinEnable);
EXPORT_SYMBOL(MHAL_RGN_GopGwinDisable);
EXPORT_SYMBOL(MHAL_RGN_GopSetColorkey);
EXPORT_SYMBOL(MHAL_RGN_GopSetAlphaType);
EXPORT_SYMBOL(MHAL_RGN_GopSetArgb1555AlphaVal);
EXPORT_SYMBOL(MHAL_RGN_GetChipCapability);
#endif
