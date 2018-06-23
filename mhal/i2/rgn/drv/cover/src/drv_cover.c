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
#define __DRV_COVER_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include <string.h>
//#include "kernel.h"

//#include "sys_sys.h"
//#include "sys_sys_isw_uart.h"
//#include "hal_drv_util.h"

//#include "cam_os_wrapper.h"

#if defined (COVEROS_TYPE_LINUX_KERNEL)
#include "ms_platform.h"
#else
#include "hal_drv_util.h"
#endif

#include "hal_cover.h"
#include "drv_cover.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define MAX(X,Y) (X >Y) ? X : Y

#define TRUE 1
#define FALSE 0

#define DRV_COVER_DEBUG   1

#if DRV_COVER_DEBUG
#if defined(CAM_OS_RTK)
#define DRVCOVERDBG(fmt, arg...) UartSendTrace(fmt, ##arg) //CamOsDebug(fmt, ##arg)
#else
#define DRVCOVERDBG(fmt, arg...) printk(KERN_INFO fmt, ##arg) //CamOsDebug(fmt, ##arg)
#endif
#else
#define DRVCOVERDBG(fmt, arg...)
#endif

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    bool bGwinEn;
    DrvCoverWindowConfig_t tWinSet;
    DrvCoverColorConfig_t tWinColorSet;
} _DrvCoverWinLocalSettingsConfig_t;

typedef struct
{
    bool bDbEn;
    _DrvCoverWinLocalSettingsConfig_t tCoverWinsSet[E_HAL_COVER_BWIN_ID_NUM];
} _DrvCoverLocalSettingsConfig_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

_DrvCoverLocalSettingsConfig_t _tCoverLocSettings[E_DRV_COVER_ID_NUM];

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

bool _DrvCoverTransId(DrvCoverIdType_e eCoverId, HalCoverIdType_e *pHalId)
{
    bool bRet = TRUE;

    switch(eCoverId)
    {
        case E_DRV_ISPSC1_COVER_ID_0:
            *pHalId = E_HAL_ISPSCL1_COVER_ID_00;
            break;

        case E_DRV_ISPSC4_COVER_ID_1:
            *pHalId = E_HAL_ISPSCL4_COVER_ID_01;
            break;

        case E_DRV_DIP_COVER_ID_2:
            *pHalId = E_HAL_DIP_COVER_ID_02;
            break;

        case E_DRV_COVER_ID_NUM:
        default:
            *pHalId = E_HAL_ISPSCL1_COVER_ID_00;
            bRet = FALSE;
            break;

    }
    return bRet;
}

bool _DrvCoverTransCmdqId(DrvCoverCmdqIdType_e eCoverId,HalCoverCmdqIdType_e *pHalCmdqId)
{
    bool bRet = TRUE;

    if(eCoverId == E_DRV_COVER_CMDQ_VPE_ID_0)
    {
        *pHalCmdqId = E_HAL_COVER_CMDQ_VPE_ID_0;
        return bRet;
    }

    if(eCoverId == E_DRV_COVER_CMDQ_DIVP_ID_1)
    {
        *pHalCmdqId = E_HAL_COVER_CMDQ_DIVP_ID_1;
        return bRet;
    }

    *pHalCmdqId = E_HAL_COVER_CMDQ_ID_NUM;
    return FALSE;
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------


bool DrvCoverRgnInit(void)
{
    bool bRet = TRUE;
    u8 u8CoverId;
    u8 u8WinId;
    HalCoverWindowType_t tHalWinCfg;

    for(u8CoverId = E_DRV_ISPSC1_COVER_ID_0; u8CoverId < E_DRV_COVER_ID_NUM; u8CoverId++)
    {
        _tCoverLocSettings[u8CoverId].bDbEn = FALSE;

        for(u8WinId = E_DRV_COVER_WIN_ID_0; u8WinId < E_DRV_COVER_WIN_ID_NUM; u8WinId++)
        {
            _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].bGwinEn = FALSE;
            _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinColorSet.u8R = 0;
            _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinColorSet.u8G = 0;
            _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinColorSet.u8B = 0;
            _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16X = 0;
            _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16Y= 0;
            _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16Width= 0;
            _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16Height= 0;

            HalCoverSetEnableWin((DrvCoverIdType_e)u8CoverId, (HalCoverWinIdType_e)u8WinId,
                _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].bGwinEn);
            tHalWinCfg.u16X = _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16X;
            tHalWinCfg.u16Y = _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16Y;
            tHalWinCfg.u16Width= _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16Width;
            tHalWinCfg.u16Height= _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16Height;
            HalCoverSetWindowSize((DrvCoverIdType_e)u8CoverId, (HalCoverWinIdType_e)u8WinId, tHalWinCfg);
            HalCoverSetColor((DrvCoverIdType_e)u8CoverId, (HalCoverWinIdType_e)u8WinId,
                _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinColorSet.u8R,
                _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinColorSet.u8G,
                _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinColorSet.u8B);
        }
    }

    return bRet;
}
bool DrvCoverRgnSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,DrvCoverCmdqIdType_e eCoverDrvId)
{
    bool bRet = FALSE;
    HalCoverCmdqIdType_e eHalCmdqId;
    if(pstCmdq)
    {
        if(_DrvCoverTransCmdqId(eCoverDrvId, &eHalCmdqId) == TRUE)
        {
            HalCoverSetCmdq(pstCmdq,eHalCmdqId);
        }
        bRet = TRUE;
    }
    return bRet;
}
bool DrvCoverSetWinSize(DrvCoverIdType_e eCoverId, DrvCoverWinIdType_e eWinId, DrvCoverWindowConfig_t tWinCfg)
{
    bool bRet = TRUE;
    HalCoverIdType_e eHalCoverId;
    HalCoverWindowType_t tHalWinCfg;

    if(_DrvCoverTransId(eCoverId, &eHalCoverId) == TRUE)
    {
        _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16X = tWinCfg.u16X;
        _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16Y = tWinCfg.u16Y;
        _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16Width = tWinCfg.u16Width;
        _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16Height = tWinCfg.u16Height;
        tHalWinCfg.u16X = tWinCfg.u16X;
        tHalWinCfg.u16Y = tWinCfg.u16Y;
        tHalWinCfg.u16Width = tWinCfg.u16Width;
        tHalWinCfg.u16Height = tWinCfg.u16Height;
        HalCoverSetWindowSize(eHalCoverId, (HalCoverWinIdType_e)eWinId, tHalWinCfg);
        bRet = TRUE;
    }
    else
    {
        DRVCOVERDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvCoverSetColor(DrvCoverIdType_e eCoverId, DrvCoverWinIdType_e eWinId, DrvCoverColorConfig_t tColorCfg)
{
    bool bRet = TRUE;
    HalCoverIdType_e eHalCoverId;

    if(_DrvCoverTransId(eCoverId, &eHalCoverId) == TRUE)
    {
        _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinColorSet.u8R= tColorCfg.u8R;
        _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinColorSet.u8G= tColorCfg.u8G;
        _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinColorSet.u8B = tColorCfg.u8B;
        HalCoverSetColor(eHalCoverId, (HalCoverWinIdType_e)eWinId, tColorCfg.u8R, tColorCfg.u8G, tColorCfg.u8B);
        bRet = TRUE;
    }
    else
    {
        DRVCOVERDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvCoverSetEnableWin(DrvCoverIdType_e eCoverId,  DrvCoverWinIdType_e eWinId, bool bEn)
{
    bool bRet = TRUE;
    HalCoverIdType_e eHalCoverId;

    if(_DrvCoverTransId(eCoverId, &eHalCoverId) == TRUE)
    {
        _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].bGwinEn= bEn;
        HalCoverSetEnableWin(eHalCoverId, (HalCoverWinIdType_e)eWinId, bEn);
        bRet = TRUE;
    }
    else
    {
        DRVCOVERDBG("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}
