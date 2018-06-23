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
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
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
#define __DRV_GOP_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include <string.h>
//#include "kernel.h"

//#include "sys_sys.h"
//#include "sys_sys_isw_uart.h"
//#include "hal_drv_util.h"

//#include "cam_os_wrapper.h"

#if defined (GOPOS_TYPE_LINUX_KERNEL)
#include "ms_platform.h"
#else
#include "hal_drv_util.h"
#endif

#include "hal_gop.h"
#include "drv_gop.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define MAX(X,Y) (X >Y) ? X : Y

#define TRUE 1
#define FALSE 0

#define DRV_GOP_DEBUG   1

#if DRV_GOP_DEBUG
#if defined(CAM_OS_RTK)
#define DRVGOPDBG(fmt, arg...) UartSendTrace(fmt, ##arg) //CamOsDebug(fmt, ##arg)
#else
#define DRVGOPDBG(fmt, arg...) printk(KERN_INFO fmt, ##arg) //CamOsDebug(fmt, ##arg)
#endif
#else
#define DRVGOPDBG(fmt, arg...)
#endif

#define DRV_GOP_I2_USAGE   1

#define IsDrvGOPStretchRatioType_1(x)          (x == E_DRV_GOP_STRETCH_RATIO_1)

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    bool bGwinEn;
    DrvGopGwinConfig_t tGwinGenSet;
    DrvGopGwinAlphaConfig_t tGwinAblSet;
    u32 u32MemPitch;
    u8 u8Argb1555Alpha0Val;
    u8 u8Argb1555Alpha1Val;
} _DrvGopGwinLocalSettingsConfig_t;

typedef struct
{
    bool bGopOsdbEn; ///scaler/dip to gop enable
    DrvGopOutFmtType_e      eGopOutFmtSet;
    DrvGopDisplayModeType_e eGopDispModeSet;
    DrvGopWindowConfig_t    tGopStrWinSet;
    DrvGopColorKeyConfig_t tGopColorKeySet;
    u8  u8GWinNum;
    _DrvGopGwinLocalSettingsConfig_t tGwinLocSet[E_DRV_GOP_GWIN_ID_NUM];
} _DrvGopLocalSettingsConfig_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

_DrvGopLocalSettingsConfig_t _tGopLocSettings[E_DRV_GOP_ID_NUM];

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

bool _DrvGopTransId(DrvGopIdType_e eGopId, HalGopIdType_e *pHalId)
{
    bool bRet = TRUE;

    switch(eGopId)
    {
        case E_DRV_ISPGOP_ID_0:
            *pHalId = E_HAL_ISPGOP_ID_00;
            break;

        case E_DRV_ISPGOP_ID_1:
            *pHalId = E_HAL_ISPGOP_ID_10;
            break;

        case E_DRV_ISPGOP_ID_2:
            *pHalId = E_HAL_ISPGOP_ID_20;
            break;

        case E_DRV_ISPGOP_ID_3:
            *pHalId = E_HAL_ISPGOP_ID_30;
            break;

        case E_DRV_DIPGOP_ID_4:
            *pHalId = E_HAL_DIPGOP_ID_40;
            break;

        case E_DRV_DISPGOP_ID_5:
            *pHalId = E_HAL_DISPGOP_ID_50;
            break;

        case E_DRV_DISPGOP_ID_6:
            *pHalId = E_HAL_DISPGOP_ID_60;
            break;

        case E_DRV_GOP_ID_NUM:
        default:
            *pHalId = E_HAL_ISPGOP_ID_00;
            bRet = FALSE;
            break;

    }
    return bRet;
}
bool _DrvGopTransCmdqId(DrvGopCmdqIdType_e eGopId,HalGopCmdqIdType_e *pHalCmdqId)
{
    bool bRet = TRUE;

    if(eGopId == E_DRV_GOP_VPE_CMDQ_ID_0)
    {
        *pHalCmdqId = E_HAL_GOP_CMDQ_VPE_ID_0;
        return bRet;
    }

    if(eGopId == E_DRV_GOP_DIVP_CMDQ_ID_1)
    {
        *pHalCmdqId = E_HAL_GOP_CMDQ_DIVP_ID_1;
        return bRet;
    }

    return FALSE;
}

bool _DrvGopTransGwinId(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, HalGopIdType_e *pHalId)
{
    bool bRet = TRUE;

    switch(eGopId)
    {
        case E_DRV_ISPGOP_ID_0:
            if((eGwinId == E_DRV_GOP_GWIN_ID_0) || (eGwinId == E_DRV_GOP_GWIN_ID_1))
            {
                *pHalId = E_HAL_ISPGOP_ID_01;
            }
            else
            {
                bRet = FALSE;
            }
            break;

        case E_DRV_ISPGOP_ID_1:
            if((eGwinId == E_DRV_GOP_GWIN_ID_0) || (eGwinId == E_DRV_GOP_GWIN_ID_1))
            {
                *pHalId = E_HAL_ISPGOP_ID_11;
            }
            else
            {
                bRet = FALSE;
            }
            break;

        case E_DRV_ISPGOP_ID_2:
            if((eGwinId == E_DRV_GOP_GWIN_ID_0) || (eGwinId == E_DRV_GOP_GWIN_ID_1))
            {
                *pHalId = E_HAL_ISPGOP_ID_21;
            }
            else
            {
                bRet = FALSE;
            }
            break;

        case E_DRV_ISPGOP_ID_3:
            if((eGwinId == E_DRV_GOP_GWIN_ID_0) || (eGwinId == E_DRV_GOP_GWIN_ID_1))
            {
                *pHalId = E_HAL_ISPGOP_ID_31;
            }
            else
            {
                bRet = FALSE;
            }
            break;

        case E_DRV_DIPGOP_ID_4:
            if((eGwinId == E_DRV_GOP_GWIN_ID_0) || (eGwinId == E_DRV_GOP_GWIN_ID_1))
            {
                *pHalId = E_HAL_DIPGOP_ID_41;
            }
            else
            {
                bRet = FALSE;
            }
            break;


        case E_DRV_DISPGOP_ID_5:
            if(eGwinId == E_DRV_GOP_GWIN_ID_0)
            {
                *pHalId = E_HAL_DISPGOP_ID_51;
            }
            else
            {
                bRet = FALSE;
            }
            break;

        case E_DRV_DISPGOP_ID_6:
            if(eGwinId == E_DRV_GOP_GWIN_ID_0)
            {
                *pHalId = E_HAL_DISPGOP_ID_61;
            }
            else
            {
                bRet = FALSE;
            }
            break;

        case E_DRV_GOP_ID_NUM:
        default:
            DRVGOPDBG("%s %d, GopId=%x, GWinId=%x\n", __FUNCTION__, __LINE__, eGopId, eGwinId);
            *pHalId = E_HAL_ISPGOP_ID_00;
            bRet = FALSE;
            break;
    }

    return bRet;
}

//unuse
/*
u16 _DrvGopTransStretchHorizonRatio(DrvGopStretchRatioType_e eGopStrRatio)
{
    u16 u16Ratio = 0;

    switch(eGopStrRatio)
    {
        case E_DRV_GOP_STRETCH_RATIO_2:
            u16Ratio = 2;
            break;

        case E_DRV_GOP_STRETCH_RATIO_4:
            u16Ratio = 4;
            break;

        case E_DRV_GOP_STRETCH_RATIO_8:
            u16Ratio = 8;
            break;

        case E_DRV_GOP_STRETCH_RATIO_1:
        default:
            u16Ratio = 1;
            break;

    }
    return u16Ratio;
}
*/


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

bool DrvGopRgnInit(void)
{
    bool bRet = TRUE;
    u8 u8GopRgnDevNum = E_DRV_DIPGOP_ID_4 + 1;
    u8 u8GopIdx;
    u8 u8GwinIdx;
    HalGopIdType_e eHalGopId;
    HalGopParamConfig_t tParamCfg;
    HalGopGwinParamConfig_t tGwinParamCfg;

    HalGopInitCmdq();

    HalGop0SetOsdBypassForSc2Enable(TRUE);

    //initialize gop parameters
    for(u8GopIdx = E_DRV_ISPGOP_ID_0; u8GopIdx < u8GopRgnDevNum; u8GopIdx++)
    {
        //Gop settings
        _DrvGopTransId((DrvGopIdType_e)u8GopIdx, &eHalGopId);

        _tGopLocSettings[u8GopIdx].bGopOsdbEn = FALSE; // default close osdb
        HalGopSetScalerEnableGop(eHalGopId, _tGopLocSettings[u8GopIdx].bGopOsdbEn);

        _tGopLocSettings[u8GopIdx].eGopOutFmtSet = E_DRV_GOP_OUT_FMT_YUV;
        _tGopLocSettings[u8GopIdx].eGopDispModeSet = E_DRV_GOP_DISPLAY_MD_PROGRESS;
        _tGopLocSettings[u8GopIdx].tGopStrWinSet.u16X = 0;
        _tGopLocSettings[u8GopIdx].tGopStrWinSet.u16Y = 0;
        _tGopLocSettings[u8GopIdx].tGopStrWinSet.u16Width= 0;
        _tGopLocSettings[u8GopIdx].tGopStrWinSet.u16Height= 0;
        tParamCfg.eOutFormat = (HalGopOutFormatType_e)_tGopLocSettings[u8GopIdx].eGopOutFmtSet;
        tParamCfg.eDisplayMode = (HalGopDisplayModeType_t)_tGopLocSettings[u8GopIdx].eGopDispModeSet;
        tParamCfg.tStretchWindow.u16X = _tGopLocSettings[u8GopIdx].tGopStrWinSet.u16X;
        tParamCfg.tStretchWindow.u16Y = _tGopLocSettings[u8GopIdx].tGopStrWinSet.u16Y;
        tParamCfg.tStretchWindow.u16Width= _tGopLocSettings[u8GopIdx].tGopStrWinSet.u16Width;
        tParamCfg.tStretchWindow.u16Height= _tGopLocSettings[u8GopIdx].tGopStrWinSet.u16Height;
        HalGopUpdateParam(eHalGopId, tParamCfg);

        _tGopLocSettings[u8GopIdx].tGopColorKeySet.bEn = FALSE;
        _tGopLocSettings[u8GopIdx].tGopColorKeySet.u8R = 0;
        _tGopLocSettings[u8GopIdx].tGopColorKeySet.u8G = 0;
        _tGopLocSettings[u8GopIdx].tGopColorKeySet.u8B = 0;
        HalGopSetColorKey(eHalGopId,
            _tGopLocSettings[u8GopIdx].tGopColorKeySet.bEn,
            _tGopLocSettings[u8GopIdx].tGopColorKeySet.u8R,
            _tGopLocSettings[u8GopIdx].tGopColorKeySet.u8G,
            _tGopLocSettings[u8GopIdx].tGopColorKeySet.u8B);

        //Gwin settings
        _tGopLocSettings[u8GopIdx].u8GWinNum = E_DRV_GOP_GWIN_ID_NUM;
        for(u8GwinIdx = E_DRV_GOP_GWIN_ID_0; u8GwinIdx < E_DRV_GOP_GWIN_ID_NUM; u8GwinIdx++)
        {
            _DrvGopTransGwinId((DrvGopIdType_e)u8GopIdx, (DrvGopGwinIdType_e)u8GwinIdx, &eHalGopId);

            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].bGwinEn = FALSE;
            HalGopSetEnableGwin(eHalGopId, (HalGopGwinIdType_e)u8GwinIdx, _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].bGwinEn);

            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.eGwinId = (HalGopGwinIdType_e)u8GwinIdx;
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.eSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_I8_PALETTE;
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.u32BaseAddr = 0;
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.u16Base_XOffset = 0;
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.u32Base_YOffset = 0;
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.tDisplayWin.u16X = 0;
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.tDisplayWin.u16Y = 0;
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.tDisplayWin.u16Width = 0;
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.tDisplayWin.u16Height = 0;
            tGwinParamCfg.eSrcFmt = (HalGopGwinSrcFormat_e)_tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.eSrcFmt;
            tGwinParamCfg.u32BaseAddr = _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.u32BaseAddr;
            tGwinParamCfg.u16Base_XOffset = _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.u16Base_XOffset;
            tGwinParamCfg.u32Base_YOffset = _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.u32Base_YOffset;
            tGwinParamCfg.tDispWindow.u16X = _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.tDisplayWin.u16X;
            tGwinParamCfg.tDispWindow.u16Y = _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.tDisplayWin.u16Y;
            tGwinParamCfg.tDispWindow.u16Width= _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.tDisplayWin.u16Width;
            tGwinParamCfg.tDispWindow.u16Height= _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinGenSet.tDisplayWin.u16Height;
            HalGopUpdateGwinParam(eHalGopId, (HalGopGwinIdType_e)u8GwinIdx, tGwinParamCfg);

            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinAblSet.eAlphaType = E_DRV_GOP_GWIN_ALPHA_CONSTANT;
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinAblSet.u8ConstantAlpahValue = 0xFF;
            HalGopSetAlphaBlending(eHalGopId, (HalGopGwinIdType_e)u8GwinIdx,
                _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinAblSet.eAlphaType == E_DRV_GOP_GWIN_ALPHA_CONSTANT ? TRUE : FALSE,
                _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].tGwinAblSet.u8ConstantAlpahValue);

            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].u8Argb1555Alpha0Val=0x0;
            HalGopSetArgb1555Alpha(eHalGopId, (HalGopGwinIdType_e)u8GwinIdx, E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0,
                _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].u8Argb1555Alpha0Val);
            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].u8Argb1555Alpha1Val=0xFF;
            HalGopSetArgb1555Alpha(eHalGopId, (HalGopGwinIdType_e)u8GwinIdx, E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1,
                _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].u8Argb1555Alpha1Val);

            _tGopLocSettings[u8GopIdx].tGwinLocSet[u8GwinIdx].u32MemPitch = 0;

        }
    }

    return bRet;
}

bool DrvGopRgnSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,DrvGopCmdqIdType_e eGopCmdqId)
{
    bool bRet = FALSE;
    HalGopCmdqIdType_e eHalCmdqId;
    if(pstCmdq)
    {
        if(_DrvGopTransCmdqId(eGopCmdqId, &eHalCmdqId) == TRUE)
        {
            HalGopSetCmdq(pstCmdq,eHalCmdqId);
        }
        bRet = TRUE;
    }
    return bRet;
}
bool DrvGopRgnSetEnable(DrvGopIdType_e eGopId,  DrvGopGwinIdType_e eGwinId, bool bEn)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].bGwinEn = bEn;
        HalGopSetEnableGwin(eHalGopId, (HalGopGwinIdType_e)eGwinId, bEn);

        if(bEn)//if open, then open osdb
        {
            _tGopLocSettings[eGopId].bGopOsdbEn = bEn;
            HalGopSetScalerEnableGop(eHalGopId, bEn);
        }
        else
        {
            //check if all gwins of one gop are all closed, then close osdb enable
            u8 u8GwinIdx;
            bool bGwinsEn = FALSE;
            for(u8GwinIdx = E_DRV_GOP_GWIN_ID_0; u8GwinIdx < E_DRV_GOP_GWIN_ID_NUM; u8GwinIdx++)
            {
                if(_tGopLocSettings[eGopId].tGwinLocSet[u8GwinIdx].bGwinEn == TRUE)
                {
                    bGwinsEn = TRUE;
                }
            }
            if(bGwinsEn == FALSE)
            {
                _tGopLocSettings[eGopId].bGopOsdbEn = bEn;
                HalGopSetScalerEnableGop(eHalGopId, bEn);
            }
        }
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }

    return bRet;
}

bool DrvGopSetModeConfig(DrvGopIdType_e eGopId, DrvGopModeConfig_t *pGopParamCfg)
{
    bool bRet = TRUE;
    HalGopParamConfig_t tHalGopCfg;
    HalGopGwinParamConfig_t tHalGopGwinCfg;
    u8 u8GwinIdx;
    HalGopIdType_e eHalGopId;

    //Update GWin Config
    for(u8GwinIdx = 0; u8GwinIdx < pGopParamCfg->u8GWinNum; u8GwinIdx++)
    {
        if(_DrvGopTransGwinId(eGopId, pGopParamCfg->pstGwinCfg[u8GwinIdx].eGwinId, &eHalGopId) == TRUE)
        {
            tHalGopGwinCfg.eSrcFmt = (HalGopGwinSrcFormat_e)pGopParamCfg->pstGwinCfg[u8GwinIdx].eSrcFmt;
            #if defined (GOPOS_TYPE_LINUX_KERNEL)
            tHalGopGwinCfg.u32BaseAddr = (u32)Chip_Phys_to_MIU((u64)pGopParamCfg->pstGwinCfg[u8GwinIdx].u32BaseAddr);
            #else
            tHalGopGwinCfg.u32BaseAddr = HalUtilPHY2MIUAddr(pGopParamCfg->pstGwinCfg[u8GwinIdx].u32BaseAddr);
            #endif
            tHalGopGwinCfg.u32Base_YOffset = pGopParamCfg->pstGwinCfg[u8GwinIdx].u32Base_YOffset;
            tHalGopGwinCfg.u16Base_XOffset = pGopParamCfg->pstGwinCfg[u8GwinIdx].u16Base_XOffset;
            tHalGopGwinCfg.tDispWindow.u16X = pGopParamCfg->pstGwinCfg[u8GwinIdx].tDisplayWin.u16X;
            tHalGopGwinCfg.tDispWindow.u16Y = pGopParamCfg->pstGwinCfg[u8GwinIdx].tDisplayWin.u16X;
            tHalGopGwinCfg.tDispWindow.u16Width = pGopParamCfg->pstGwinCfg[u8GwinIdx].tDisplayWin.u16Width;
            tHalGopGwinCfg.tDispWindow.u16Height = pGopParamCfg->pstGwinCfg[u8GwinIdx].tDisplayWin.u16Height;

            HalGopUpdateGwinParam(eHalGopId, pGopParamCfg->pstGwinCfg[u8GwinIdx].eGwinId, tHalGopGwinCfg);
        }
        else
        {
            DRVGOPDBG("%s %d: (%d %d), Fail ID\n", __FUNCTION__, __LINE__, eGopId, pGopParamCfg->pstGwinCfg[u8GwinIdx].eGwinId);

        }
    }

    //Update Gop Config
    if(_DrvGopTransId(eGopId, &eHalGopId) == TRUE)
    {
        tHalGopCfg.eDisplayMode =
            pGopParamCfg->eDisplyMode == E_DRV_GOP_DISPLAY_MD_INTERLACE ? E_HAL_GOP_DISPLAY_MODE_INTERLACE :
            E_HAL_GOP_DISPLAY_MODE_PROGRESS;
        tHalGopCfg.eOutFormat =
            pGopParamCfg->eOutFmt == E_DRV_GOP_OUT_FMT_RGB ? E_HAL_GOP_OUT_FMT_RGB :
            E_HAL_GOP_OUT_FMT_YUV;

        //unuse
        //if(IsDrvGOPStretchRatioType_1(pGopParamCfg->eStretchHRatio))
        {
            tHalGopCfg.tStretchWindow.u16X = pGopParamCfg->tOsdDisplayWindow.u16X;
            tHalGopCfg.tStretchWindow.u16Y = pGopParamCfg->tOsdDisplayWindow.u16Y;
            tHalGopCfg.tStretchWindow.u16Width = pGopParamCfg->tOsdDisplayWindow.u16Width;
            tHalGopCfg.tStretchWindow.u16Height = pGopParamCfg->tOsdDisplayWindow.u16Height;
        }
        /*else
        {
            tHalGopCfg.tStretchWindow.u16X = pGopParamCfg->tOsdDisplayWindow.u16X;
            tHalGopCfg.tStretchWindow.u16Y = pGopParamCfg->tOsdDisplayWindow.u16Y;
            tHalGopCfg.tStretchWindow.u16Width = (pGopParamCfg->tOsdDisplayWindow.u16Width / _DrvGopTransStretchHorizonRatio(pGopParamCfg->eStretchHRatio));
            tHalGopCfg.tStretchWindow.u16Height = pGopParamCfg->tOsdDisplayWindow.u16Height;
        }*/

        HalGopUpdateParam(eHalGopId, tHalGopCfg);
    }


    return bRet;
}


bool DrvGopUpdateBase(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, u32 u32BaseAddr)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.u32BaseAddr = u32BaseAddr;
        #if defined (GOPOS_TYPE_LINUX_KERNEL)
        //HalGopUpdateBase(eHalGopId, (HalGopGwinIdType_e)eGwinId, (u32)Chip_Phys_to_MIU((u64)u32BaseAddr));
        HalGopUpdateBase(eHalGopId, (HalGopGwinIdType_e)eGwinId, u32BaseAddr);//I2 already is miu address
        #else
        //HalGopUpdateBase(eHalGopId, (HalGopGwinIdType_e)eGwinId, HalUtilPHY2MIUAddr(u32BaseAddr));
        HalGopUpdateBase(eHalGopId, (HalGopGwinIdType_e)eGwinId, u32BaseAddr);//I2 already is miu address
        #endif

        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetMemPitchDirect(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, u32 u32MemPitch)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].u32MemPitch = u32MemPitch;
        HalGopSetGwinMemPitchDirect(eHalGopId, (HalGopGwinIdType_e)eGwinId,
            _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].u32MemPitch);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetOutFormat(DrvGopIdType_e eGopId, DrvGopOutFmtType_e eFmt)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransId(eGopId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].eGopOutFmtSet = eFmt;
        HalGopSetOutFormat(eHalGopId,  eFmt == E_DRV_GOP_OUT_FMT_YUV ? TRUE : FALSE);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetSrcFmt(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, DrvGopGwinSrcFmtType_e eSrcFmt)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.eSrcFmt = eSrcFmt;
        HalGopSetGwinSrcFmt(eHalGopId, (HalGopGwinIdType_e)eGwinId, (HalGopGwinSrcFormat_e)eSrcFmt);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetAlphaBlending(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, DrvGopGwinAlphaConfig_t tAlphaCfg)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinAblSet.eAlphaType = tAlphaCfg.eAlphaType;
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinAblSet.u8ConstantAlpahValue = tAlphaCfg.u8ConstantAlpahValue;
        HalGopSetAlphaBlending(eHalGopId, (HalGopGwinIdType_e)eGwinId,
                               tAlphaCfg.eAlphaType == E_DRV_GOP_GWIN_ALPHA_CONSTANT ? TRUE : FALSE,
                               tAlphaCfg.u8ConstantAlpahValue);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetColorKey(DrvGopIdType_e eGopId, DrvGopColorKeyConfig_t tColorKeyCfg)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransId(eGopId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGopColorKeySet.bEn = tColorKeyCfg.bEn;
        _tGopLocSettings[eGopId].tGopColorKeySet.u8R = tColorKeyCfg.u8R;
        _tGopLocSettings[eGopId].tGopColorKeySet.u8G = tColorKeyCfg.u8G;
        _tGopLocSettings[eGopId].tGopColorKeySet.u8B = tColorKeyCfg.u8B;
        HalGopSetColorKey(eHalGopId, tColorKeyCfg.bEn, tColorKeyCfg.u8R, tColorKeyCfg.u8G, tColorKeyCfg.u8B);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetPipeDelay(DrvGopIdType_e eGopId, u8 u8Delay)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransId(eGopId, &eHalGopId) == TRUE)
    {
        HalGopSetPipeDelay(eHalGopId, u8Delay);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetPaletteRiu(DrvGopIdType_e eGopId, DrvGopGwinPaletteConfig_t tPaletteCfg)
{
    bool bRet = TRUE;
    u16 i;

    HalGopIdType_e eHalGopId;

    if(_DrvGopTransId(eGopId, &eHalGopId) == TRUE)
    {
        for(i = 0; i < tPaletteCfg.u16Size; i++)
        {
            HalGopSetPaletteRiu(eHalGopId, (u8)i,
                                tPaletteCfg.pu8Palette_A[i], tPaletteCfg.pu8Palette_R[i], tPaletteCfg.pu8Palette_G[i], tPaletteCfg.pu8Palette_B[i]);
        }
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetPaletteRiuOneMem(DrvGopIdType_e eGopId, u8 u8Idx, u8 u8A, u8 u8R, u8 u8G, u8 u8B)
{
    bool bRet = TRUE;

    HalGopIdType_e eHalGopId;

    if(_DrvGopTransId(eGopId, &eHalGopId) == TRUE)
    {
        HalGopSetPaletteRiu(eHalGopId, u8Idx, u8A, u8R, u8G, u8B);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}


bool DrvGopSetEnableGwin(DrvGopIdType_e eGopId,  DrvGopGwinIdType_e eGwinId, bool bEn)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].bGwinEn = bEn;
        HalGopSetEnableGwin(eHalGopId, (HalGopGwinIdType_e)eGwinId, bEn);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetScalerEnableGop(DrvGopIdType_e eGopId,  DrvGopGwinIdType_e eGwinId, bool bEn)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].bGopOsdbEn = bEn;
        HalGopSetScalerEnableGop(eHalGopId, bEn);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetGwinSize(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, DrvGopWindowConfig_t tWinCfg, DrvGopGwinSrcFmtType_e eSrcFmt)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;
    HalGopWindowType_t tHalGwinCfg;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.tDisplayWin.u16X = tWinCfg.u16X;
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.tDisplayWin.u16Y = tWinCfg.u16Y;
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.tDisplayWin.u16Width = tWinCfg.u16Width;
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.tDisplayWin.u16Height = tWinCfg.u16Height;
        tHalGwinCfg.u16X = tWinCfg.u16X;
        tHalGwinCfg.u16Y = tWinCfg.u16Y;
        tHalGwinCfg.u16Width = tWinCfg.u16Width;
        tHalGwinCfg.u16Height = tWinCfg.u16Height;
        HalGopSetGwinSize(eHalGopId, (HalGopGwinIdType_e)eGwinId, tHalGwinCfg, (HalGopGwinSrcFormat_e)eSrcFmt);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetGwinSizeWithoutFmt(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, DrvGopWindowConfig_t tWinCfg)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;
    HalGopWindowType_t tHalGwinCfg;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.tDisplayWin.u16X = tWinCfg.u16X;
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.tDisplayWin.u16Y = tWinCfg.u16Y;
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.tDisplayWin.u16Width = tWinCfg.u16Width;
        _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.tDisplayWin.u16Height = tWinCfg.u16Height;
        tHalGwinCfg.u16X = tWinCfg.u16X;
        tHalGwinCfg.u16Y = tWinCfg.u16Y;
        tHalGwinCfg.u16Width = tWinCfg.u16Width;
        tHalGwinCfg.u16Height = tWinCfg.u16Height;
        HalGopSetGwinSize(eHalGopId, (HalGopGwinIdType_e)eGwinId, tHalGwinCfg,
            (HalGopGwinSrcFormat_e)_tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinGenSet.eSrcFmt );
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetStretchWindowSize(DrvGopIdType_e eGopId, DrvGopWindowConfig_t tWinCfg)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;
    HalGopWindowType_t tHalGwinCfg;

    if(_DrvGopTransId(eGopId, &eHalGopId) == TRUE)
    {
        _tGopLocSettings[eGopId].tGopStrWinSet.u16X = tWinCfg.u16X;
        _tGopLocSettings[eGopId].tGopStrWinSet.u16Y = tWinCfg.u16Y;
        _tGopLocSettings[eGopId].tGopStrWinSet.u16Width = tWinCfg.u16Width;
        _tGopLocSettings[eGopId].tGopStrWinSet.u16Height = tWinCfg.u16Height;
        tHalGwinCfg.u16X = tWinCfg.u16X;
        tHalGwinCfg.u16Y = tWinCfg.u16Y;
        tHalGwinCfg.u16Width = tWinCfg.u16Width;
        tHalGwinCfg.u16Height = tWinCfg.u16Height;
        HalGopSetStretchWindowSize(eHalGopId, tHalGwinCfg);
        bRet = TRUE;
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopSetArgb1555AlphaDefVal(DrvGopIdType_e eGopId,  DrvGopGwinIdType_e eGwinId, DrvGopGwinArgb1555Def_e eAlphaType, MS_U8 u8AlphaVal)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
        if(eAlphaType==E_DRV_GOP_GWIN_ARGB1555_DEFINE_ALPHA0)
        {
            _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].u8Argb1555Alpha0Val=u8AlphaVal;
            HalGopSetArgb1555Alpha(eHalGopId, (HalGopGwinIdType_e)eGwinId, E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0,
                _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].u8Argb1555Alpha0Val);
            bRet = TRUE;
        }
        else if(eAlphaType==E_DRV_GOP_GWIN_ARGB1555_DEFINE_ALPHA1)
        {
            _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].u8Argb1555Alpha1Val=u8AlphaVal;
            HalGopSetArgb1555Alpha(eHalGopId, (HalGopGwinIdType_e)eGwinId, E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1,
                _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].u8Argb1555Alpha1Val);
            bRet = TRUE;
        }
        else
        {
            DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
            bRet = FALSE;
        }

    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvGopGetEnableGwin(DrvGopIdType_e eGopId,  DrvGopGwinIdType_e eGwinId, bool *bEn)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
#if DRV_GOP_I2_USAGE
        *bEn = _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].bGwinEn;
#else
        HalGopGetEnableGwin(eHalGopId, (HalGopGwinIdType_e)eGwinId, bEn);
        bRet = TRUE;
#endif
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }

    return bRet;
}

bool DrvGopGetScalerEnableGop(DrvGopIdType_e eGopId,  DrvGopGwinIdType_e eGwinId, bool *bEn)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
#if DRV_GOP_I2_USAGE
        *bEn = _tGopLocSettings[eGopId].bGopOsdbEn;
#else
        HalGopGetScalerEnableGop(eHalGopId, bEn);
        bRet = TRUE;
#endif
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }

    return bRet;
}

bool DrvGopGetAlpha(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, DrvGopGwinAlphaConfig_t *pstAlphaCfg)
{

    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;

    if(_DrvGopTransGwinId(eGopId, eGwinId, &eHalGopId) == TRUE)
    {
#if DRV_GOP_I2_USAGE
        pstAlphaCfg->eAlphaType = _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinAblSet.eAlphaType;
        pstAlphaCfg->u8ConstantAlpahValue = _tGopLocSettings[eGopId].tGwinLocSet[eGwinId].tGwinAblSet.u8ConstantAlpahValue;
#else
        bool bConstantAlpha;
        u8   u8Value;

        HalGopGetAlpha(eHalGopId, (HalGopGwinIdType_e)eGwinId, &bConstantAlpha, &u8Value);
        pstAlphaCfg->eAlphaType = bConstantAlpha ? E_DRV_GOP_GWIN_ALPHA_CONSTANT : E_DRV_GOP_GWIN_ALPHA_PIXEL;
        pstAlphaCfg->u8ConstantAlpahValue = u8Value;
        bRet = TRUE;
#endif
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }

    return bRet;
}


bool DrvGopGetColorKey(DrvGopIdType_e eGopId, DrvGopColorKeyConfig_t *pstColorKeyCfg)
{
    bool bRet = TRUE;
    HalGopIdType_e eHalGopId;


    if(_DrvGopTransId(eGopId, &eHalGopId) == TRUE)
    {
#if DRV_GOP_I2_USAGE
        pstColorKeyCfg->bEn = _tGopLocSettings[eGopId].tGopColorKeySet.bEn;
        pstColorKeyCfg->u8R = _tGopLocSettings[eGopId].tGopColorKeySet.u8R;
        pstColorKeyCfg->u8G = _tGopLocSettings[eGopId].tGopColorKeySet.u8G;
        pstColorKeyCfg->u8B = _tGopLocSettings[eGopId].tGopColorKeySet.u8B;
#else
        HalGopGetColorKey(eHalGopId,  &pstColorKeyCfg->bEn, &pstColorKeyCfg->u8R, &pstColorKeyCfg->u8G, &pstColorKeyCfg->u8B);
        bRet = TRUE;
#endif
    }
    else
    {
        DRVGOPDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }

    return bRet;
}
