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
#define __DRV_PNL_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#if defined(MSOS_TYPE_NOS)
#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#else
#include <linux/delay.h>
#endif

#include "cam_os_wrapper.h"
#include "ms_platform.h"
#include "ms_types.h"

#include "drv_pnl_dbg.h"
#include "drv_pnl.h"
#include "hal_pnl.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define PARSING_LINKTYPE(x)         ( x == E_DRV_PNL_LINK_NONE     ? "NONE"      : \
                                      x == E_DRV_PNL_LINK_LVDS     ? "LVDS"      : \
                                      x == E_DRV_PNL_LINK_MIPI_DSI ? "MIPI_DSI"  : \
                                      x == E_DRV_PNL_LINK_TTL      ? "LINK_TTL"  : \
                                      x == E_DRV_PNL_LINK_VBY1     ? "VBY1"      : \
                                                                     "UNKNOWN")

#define PARSING_TI_BIT_MODE(x)      ( x == E_DRV_PNL_TI_BIT_10     ? "TI_BIT_10" :\
                                      x == E_DRV_PNL_TI_BIT_8      ? "TI_BIT_8"  :\
                                      x == E_DRV_PNL_TI_BIT_6      ? "TI_BIT_6"  :\
                                                                     "UNKNOWN")

#define PARSING_MIPI_DSI_CTRL(x)    ( x == E_DRV_PNL_MIPI_DSI_CMD_MODE   ? "CMD_MODE"   : \
                                      x == E_DRV_PNL_MIPI_DSI_SYNC_PULSE ? "SYNC_PULSE" : \
                                      x == E_DRV_PNL_MIPI_DSI_SYNC_EVENT ? "SYNC_EVENT" : \
                                      x == E_DRV_PNL_MIPI_DSI_BURST_MODE ? "BURST_MODE" : \
                                                                           "UNKNOWN")

#define PARSING_MIPI_DSI_FMT(x)     ( x == E_DRV_PNL_MIPI_DSI_RGB565             ? "RGB565" : \
                                      x == E_DRV_PNL_MIPI_DSI_RGB666             ? "RGB666" : \
                                      x == E_DRV_PNL_MIPI_DSI_RGB888             ? "RGB888" : \
                                      x == E_DRV_PNL_MIPI_DSI_LOOSELY_RGB666     ? "LOOSELY_RGB666" : \
                                                                                   "UNKOWN")

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    bool bUsed;
    DrvPnlTimingConfig_t stTimingCfg;
    DrvPnlSignalCtrlConfig_t stSignalCtrlCfg;
    DrvPnlSscConfig_t stSscCfg;
    DrvPnlPowerConfig_t stPowerCfg;
    DrvPnlBackLightConfig_t stBackLightCfg;
    DrvPnlDrvCurrentConfig_t stDrvCurrentCfg;
}DrvPnlCtxConfig_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvPnlCtxConfig_t _gstPnlCtxCfg[DRV_PNL_INSTANT_MAX];
bool bDrvPnlInit = FALSE;

u32 _gu32PnlDbgLevel =  PNL_DBG_LEVEL_DRV | PNL_DBG_LEVEL_HAL | PNL_DBG_LEVEL_MHAL;// | PNL_DBG_LEVE_REG_RW | PNL_DBG_LEVEL_RW_PACKET;
//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
void _DrvPnlTransMipiDsiCfgToHal(HalPnlMipiDsiConfig_t *pHalMipiDsiCfg, HalPnlLpllConfig_t *pLpllCfg, DrvPnlMipiDsiConfig_t *pDrvMipiDsiCfg)
{
    pLpllCfg->enMipiDsiLane = pDrvMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_1 ? E_HAL_PNL_MIPI_DSI_LANE_1 :
                              pDrvMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_2 ? E_HAL_PNL_MIPI_DSI_LANE_2 :
                              pDrvMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_3 ? E_HAL_PNL_MIPI_DSI_LANE_3 :
                              pDrvMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_4 ? E_HAL_PNL_MIPI_DSI_LANE_4 :
                                                                                       E_HAL_PNL_MIPI_DSI_LANE_NONE;

    pLpllCfg->enMipiDsiFmt = pDrvMipiDsiCfg->enformat == E_DRV_PNL_MIPI_DSI_RGB565 ? E_HAL_PNL_MIPI_DSI_RGB565 :
                             pDrvMipiDsiCfg->enformat == E_DRV_PNL_MIPI_DSI_RGB666 ? E_HAL_PNL_MIPI_DSI_RGB666 :
                                                                                     E_HAL_PNL_MIPI_DSI_RGB888;

    pHalMipiDsiCfg->u8HsTrail   =  pDrvMipiDsiCfg->u8HsTrail;
    pHalMipiDsiCfg->u8HsPrpr    =  pDrvMipiDsiCfg->u8HsPrpr;
    pHalMipiDsiCfg->u8HsZero    =  pDrvMipiDsiCfg->u8HsZero;
    pHalMipiDsiCfg->u8ClkHsPrpr =  pDrvMipiDsiCfg->u8ClkHsPrpr;
    pHalMipiDsiCfg->u8ClkHsExit =  pDrvMipiDsiCfg->u8ClkHsExit;
    pHalMipiDsiCfg->u8ClkTrail  =  pDrvMipiDsiCfg->u8ClkTrail;
    pHalMipiDsiCfg->u8ClkZero   =  pDrvMipiDsiCfg->u8ClkZero;
    pHalMipiDsiCfg->u8ClkHsPost =  pDrvMipiDsiCfg->u8ClkHsPost;
    pHalMipiDsiCfg->u8DaHsExit  =  pDrvMipiDsiCfg->u8DaHsExit;
    pHalMipiDsiCfg->u8ContDet   =  pDrvMipiDsiCfg->u8ContDet;

    pHalMipiDsiCfg->u8Lpx       =  pDrvMipiDsiCfg->u8Lpx;
    pHalMipiDsiCfg->u8TaGet     =  pDrvMipiDsiCfg->u8TaGet;
    pHalMipiDsiCfg->u8TaSure    =  pDrvMipiDsiCfg->u8TaSure;
    pHalMipiDsiCfg->u8TaGo      =  pDrvMipiDsiCfg->u8TaGo;

    pHalMipiDsiCfg->u32HActive  =  pDrvMipiDsiCfg->u16Hactive;
    pHalMipiDsiCfg->u32HPW      =  pDrvMipiDsiCfg->u16Hpw;
    pHalMipiDsiCfg->u32HBP      =  pDrvMipiDsiCfg->u16Hbp;
    pHalMipiDsiCfg->u32HFP      =  pDrvMipiDsiCfg->u16Hfp;
    pHalMipiDsiCfg->u32VActive  =  pDrvMipiDsiCfg->u16Vactive;
    pHalMipiDsiCfg->u32VPW      =  pDrvMipiDsiCfg->u16Vpw;
    pHalMipiDsiCfg->u32VBP      =  pDrvMipiDsiCfg->u16Vbp;
    pHalMipiDsiCfg->u32VFP      =  pDrvMipiDsiCfg->u16Vfp;
    pHalMipiDsiCfg->u32BLLP     =  pDrvMipiDsiCfg->u16Bllp;

    pHalMipiDsiCfg->enLaneNum   =
        pDrvMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_1 ? E_HAL_PNL_MIPI_DSI_LANE_1 :
        pDrvMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_2 ? E_HAL_PNL_MIPI_DSI_LANE_2 :
        pDrvMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_3 ? E_HAL_PNL_MIPI_DSI_LANE_3 :
        pDrvMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_4 ? E_HAL_PNL_MIPI_DSI_LANE_4 :
                                                                 E_HAL_PNL_MIPI_DSI_LANE_NONE;

    pHalMipiDsiCfg->enFormat    =
        pDrvMipiDsiCfg->enformat == E_DRV_PNL_MIPI_DSI_RGB565 ?  E_HAL_PNL_MIPI_DSI_RGB565 :
        pDrvMipiDsiCfg->enformat == E_DRV_PNL_MIPI_DSI_RGB666 ?  E_HAL_PNL_MIPI_DSI_RGB666 :
        pDrvMipiDsiCfg->enformat == E_DRV_PNL_MIPI_DSI_RGB888 ?  E_HAL_PNL_MIPI_DSI_RGB888:
                                                                 E_HAL_PNL_MIPI_DSI_LOOSELY_RGB666;
    pHalMipiDsiCfg->enCtrl      =
        pDrvMipiDsiCfg->enCtrl == E_DRV_PNL_MIPI_DSI_CMD_MODE   ? E_HAL_PNL_MIPI_DSI_CMD_MODE :
        pDrvMipiDsiCfg->enCtrl == E_DRV_PNL_MIPI_DSI_SYNC_PULSE ? E_HAL_PNL_MIPI_DSI_SYNC_PULSE :
        pDrvMipiDsiCfg->enCtrl == E_DRV_PNL_MIPI_DSI_SYNC_EVENT ? E_HAL_PNL_MIPI_DSI_SYNC_EVENT :
                                                                  E_HAL_PNL_MIPI_DSI_BURST_MODE;
}


void _DrvPnlTransMipiDsiLaneFmt(HalPnlLpllConfig_t *pLpllCfg, DrvPnlMipiDsiConfig_t *pMipiDsiCfg)
{
    pLpllCfg->enMipiDsiLane = pMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_1 ? E_HAL_PNL_MIPI_DSI_LANE_1 :
                              pMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_2 ? E_HAL_PNL_MIPI_DSI_LANE_2 :
                              pMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_3 ? E_HAL_PNL_MIPI_DSI_LANE_3 :
                              pMipiDsiCfg->enLaneNum == E_DRV_PNL_MIPI_DSI_LANE_4 ? E_HAL_PNL_MIPI_DSI_LANE_4 :
                                                                                    E_HAL_PNL_MIPI_DSI_LANE_NONE;

    pLpllCfg->enMipiDsiFmt = pMipiDsiCfg->enformat == E_DRV_PNL_MIPI_DSI_RGB565 ? E_HAL_PNL_MIPI_DSI_RGB565 :
                             pMipiDsiCfg->enformat == E_DRV_PNL_MIPI_DSI_RGB666 ? E_HAL_PNL_MIPI_DSI_RGB666 :
                                                                                  E_HAL_PNL_MIPI_DSI_RGB888;
}

void _DrvPnlTransSignalCtrlConfig(HalPnlModConfig_t *pModCfg, DrvPnlSignalCtrlConfig_t *pDrvSignalCtrlCfg)
{
    u8 i;

    pModCfg->bTiMode      = pDrvSignalCtrlCfg->stModCfg.bTiMode;
    pModCfg->bChPolarity  = pDrvSignalCtrlCfg->stModCfg.bChPolarity;
    pModCfg->bHsyncInvert = pDrvSignalCtrlCfg->stModCfg.bHsyncInvert;
    pModCfg->bVsyncInvert = pDrvSignalCtrlCfg->stModCfg.bVsyncInvert;
    pModCfg->bDeInvert    = pDrvSignalCtrlCfg->stModCfg.bDeInvert;

    pModCfg->enTiBitMode  = pDrvSignalCtrlCfg->stModCfg.enTiBitMode == E_DRV_PNL_TI_BIT_8 ? E_HAL_PNL_MOD_TI_BIT_8 :
                            pDrvSignalCtrlCfg->stModCfg.enTiBitMode == E_DRV_PNL_TI_BIT_6 ? E_HAL_PNL_MOD_TI_BIT_6 :
                                                                                   E_HAL_PNL_MOD_TI_BIT_10;

    for(i=0; i<5; i++)
    {
        pModCfg->enCh[i] = pDrvSignalCtrlCfg->stModCfg.enCh[i] == E_DRV_PNL_CH_SWAP_0 ? E_HAL_PNL_MOD_CH_SWAP_0 :
                           pDrvSignalCtrlCfg->stModCfg.enCh[i] == E_DRV_PNL_CH_SWAP_1 ? E_HAL_PNL_MOD_CH_SWAP_1 :
                           pDrvSignalCtrlCfg->stModCfg.enCh[i] == E_DRV_PNL_CH_SWAP_2 ? E_HAL_PNL_MOD_CH_SWAP_2 :
                           pDrvSignalCtrlCfg->stModCfg.enCh[i] == E_DRV_PNL_CH_SWAP_3 ? E_HAL_PNL_MOD_CH_SWAP_3 :
                                                                               E_HAL_PNL_MOD_CH_SWAP_4;
    }
}


void _DrvPnlTransTimingConfig(HalPnlTimingConfig_t *pHalTimingCfg, DrvPnlTimingConfig_t *pDrvTimingCfg)
{
    pHalTimingCfg->u16Htt       = pDrvTimingCfg->u16Htt;
    pHalTimingCfg->u16Hpw       = pDrvTimingCfg->u16Hpw;
    pHalTimingCfg->u16Hbp       = pDrvTimingCfg->u16Hbp;
    pHalTimingCfg->u16Hactive   = pDrvTimingCfg->u16Hactive;
    pHalTimingCfg->u16Hstart    = pDrvTimingCfg->u16Hstart;

    pHalTimingCfg->u16Vtt       = pDrvTimingCfg->u16Vtt;
    pHalTimingCfg->u16Vpw       = pDrvTimingCfg->u16Vpw;
    pHalTimingCfg->u16Vbp       = pDrvTimingCfg->u16Vbp;
    pHalTimingCfg->u16Vactive   = pDrvTimingCfg->u16Vactive;
    pHalTimingCfg->u16Vstart    = pDrvTimingCfg->u16Vstart;
}

void _DrvPnlTransLpllConfig(HalPnlLpllConfig_t *pLpllCfg, DrvPnlTimingConfig_t *pDrvTimingCfg)
{
    pLpllCfg->u16Htotal  = pDrvTimingCfg->u16Htt;
    pLpllCfg->u16Vtotal  = pDrvTimingCfg->u16Vtt;
    pLpllCfg->u16Hactive = pDrvTimingCfg->u16Hactive;
    pLpllCfg->u16Vactive = pDrvTimingCfg->u16Vactive;
    pLpllCfg->u16Fps     = pDrvTimingCfg->u16Fps;
}

bool _DrvPnlCheckLinkType(DrvPnlLinkType_e eTimingLink, DrvPnlLinkType_e eSignalCtrlLink, DrvPnlLinkType_e enLink)
{
    if( (eTimingLink == E_DRV_PNL_LINK_NONE || eSignalCtrlLink == E_DRV_PNL_LINK_NONE))
    {
        return TRUE;
    }
    else if((enLink == eSignalCtrlLink) && (enLink == eTimingLink))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool _DrvPnlSetMipiDsiPacket(u8 *pu8PacketData, u32 u32PacketLen)
{
    u32 u32PacketIdx;
    u8 u8Cmd, u8Cnt;

    u32PacketIdx = 0;
    while (u32PacketIdx < (u32PacketLen-1))
    {
        u8Cmd = pu8PacketData[u32PacketIdx];
        u8Cnt = pu8PacketData[u32PacketIdx+1];

        if (u8Cmd == HAL_PNL_MIPI_DSI_FLAG_END_OF_TABLE && u8Cnt == HAL_PNL_MIPI_DSI_FLAG_END_OF_TABLE)
        {
            break;
        }
        else if (u8Cmd == HAL_PNL_MIPI_DSI_FLAG_DELAY && u8Cnt == HAL_PNL_MIPI_DSI_FLAG_DELAY)
        {
            udelay(pu8PacketData[u32PacketIdx+2]);
            u32PacketIdx += 3;
        }
        else
        {
            if ((u8Cnt+1) <= 2)
            {
                HalPnlSetMipiDsiShortPacket(u8Cnt+1, u8Cmd, &pu8PacketData[u32PacketIdx+2]);
            }
            else
            {
                HalPnlSetMipiDsiLongPacket(u8Cnt+1, u8Cmd, &pu8PacketData[u32PacketIdx+2]);
            }
            if (u8Cnt == 0)
            {
                u32PacketIdx += 3;
            }
            else
            {
                u32PacketIdx += (u8Cnt + 2);
            }
        }
    }
    return 1;
}


bool _DrvPnlSetTimingAndSignalCtrl(DrvPnlTimingConfig_t *pstTimingCfg, DrvPnlSignalCtrlConfig_t *pstSignalCtrlCfg)
{
    bool bRet = TRUE;
    HalPnlLpllConfig_t stLpllCfg;
    HalPnlTimingConfig_t stTimingCfg;
    HalPnlMipiDsiConfig_t stHalMipiDsiCfg;
    HalPnlModConfig_t stModCfg;

    _DrvPnlTransLpllConfig(&stLpllCfg, pstTimingCfg);
    _DrvPnlTransTimingConfig(&stTimingCfg, pstTimingCfg);

    switch(pstTimingCfg->enLinkType)
    {
        case E_DRV_PNL_LINK_LVDS:
            stLpllCfg.enLinkType = E_HAL_PNL_LINK_LVDS;

            HalPnlInitClk();
            HalPnlInitLvdsDphy();
            HalPnlSetLpllConfig(&stLpllCfg);
            HalPnlSetTimingConfig(&stTimingCfg);

            _DrvPnlTransSignalCtrlConfig(&stModCfg, pstSignalCtrlCfg);
            HalPnlSetModConfig(&stModCfg);
            break;

        case E_DRV_PNL_LINK_MIPI_DSI:
            stLpllCfg.enLinkType = E_HAL_PNL_LINK_MIPI_DSI;
            if(pstSignalCtrlCfg->stMipiDsiCfg.enLaneNum != E_DRV_PNL_MIPI_DSI_LANE_NONE)
            {
                _DrvPnlTransMipiDsiCfgToHal(&stHalMipiDsiCfg, &stLpllCfg, &pstSignalCtrlCfg->stMipiDsiCfg);

                HalPnlInitClk();
                HalPnlSetMipiDsiMisc();
                HalPnlSetLpllConfig(&stLpllCfg);
                HalPnlInitMipiDsiDphy();
                HalPnlSetTimingConfig(&stTimingCfg);

                HalPnlSetMipiDsiPadOutSel(stHalMipiDsiCfg.enLaneNum);
                HalPnlSetMipiDsiLaneNum(stHalMipiDsiCfg.enLaneNum);
                HalPnlSetMipiDsiCtrlMode(E_HAL_PNL_MIPI_DSI_CMD_MODE);
                HalPnlSetMpiDsiClkHsMode(1);

                if(pstSignalCtrlCfg->stMipiDsiCfg.pu8CmdBuf && pstSignalCtrlCfg->stMipiDsiCfg.u32CmdBufSize !=0)
                {
                    _DrvPnlSetMipiDsiPacket(pstSignalCtrlCfg->stMipiDsiCfg.pu8CmdBuf, pstSignalCtrlCfg->stMipiDsiCfg.u32CmdBufSize);
                }

                HalPnlSetMipiDsiPhyTimConfig(&stHalMipiDsiCfg);
                HalPnlSetMipiDsiVdoTimingConfig(&stHalMipiDsiCfg);
                HalPnlSetMipiDsiCtrlMode(stHalMipiDsiCfg.enCtrl);

                HalPnlEnableMipiDsiClk();
            }
            else
            {
                PNL_ERR("%s %d, LaneNum = 0 \n", __FUNCTION__, __LINE__);
            }
            break;

        case E_DRV_PNL_LINK_TTL:
            stLpllCfg.enLinkType = E_HAL_PNL_LINK_TTL;
            break;

        case E_DRV_PNL_LINK_VBY1:
            stLpllCfg.enLinkType = E_HAL_PNL_LINK_VBY1;
            break;

        default:
            bRet = FALSE;
            break;
    }
    return bRet;
}



//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool DrvPnlInit(void)
{
    u8 i;
    if(bDrvPnlInit == TRUE)
    {
        PNL_DBG(1, "%s %d, Already Init \n", __FUNCTION__, __LINE__);
        return TRUE;
    }

    memset(_gstPnlCtxCfg, 0, sizeof(DrvPnlCtxConfig_t)*DRV_PNL_INSTANT_MAX);

    for(i=0; i<DRV_PNL_INSTANT_MAX; i++)
    {
        _gstPnlCtxCfg[i].stTimingCfg.enLinkType = E_DRV_PNL_LINK_NONE;
        _gstPnlCtxCfg[i].stSignalCtrlCfg.enLinkType = E_DRV_PNL_LINK_NONE;
        _gstPnlCtxCfg[i].stSscCfg.enLinkType = E_DRV_PNL_LINK_NONE;
        _gstPnlCtxCfg[i].stPowerCfg.enLinkType = E_DRV_PNL_LINK_NONE;
        _gstPnlCtxCfg[i].stBackLightCfg.enLinkType = E_DRV_PNL_LINK_NONE;
    }
    bDrvPnlInit = TRUE;

    return TRUE;
}


void *DrvPnlCtxAllocate(void)
{
    s16 i;
    s16 s16Idx = -1;
    void *pRet;

    for(i=0; i<DRV_PNL_INSTANT_MAX; i++)
    {
        if(_gstPnlCtxCfg[i].bUsed == FALSE)
        {
            s16Idx = i;
            _gstPnlCtxCfg[i].bUsed = TRUE;
            break;
        }
    }

    if(s16Idx == -1)
    {
        pRet =  NULL;
    }
    else
    {
        pRet = (void *)&_gstPnlCtxCfg[s16Idx];
    }
    return pRet;
}

bool DrvPnlCtxFree(void *pCtx)
{
    DrvPnlCtxConfig_t *pSclMgwinCtx;

    pSclMgwinCtx = (DrvPnlCtxConfig_t *)pCtx;

    if(pSclMgwinCtx)
    {
        u16 i;
        memset(pSclMgwinCtx, 0, sizeof(DrvPnlCtxConfig_t));

        for(i=0; i<DRV_PNL_INSTANT_MAX; i++)
        {
            pSclMgwinCtx->bUsed = FALSE;
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool DrvPnlSetSignalCtrlConfig(void *pCtx, DrvPnlSignalCtrlConfig_t *pSignalCtrlCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(pPnlCtxCfg->stTimingCfg.enLinkType == E_DRV_PNL_LINK_NONE ||
       pPnlCtxCfg->stTimingCfg.enLinkType == pSignalCtrlCfg->enLinkType)
    {

        if(pSignalCtrlCfg->enLinkType == E_DRV_PNL_LINK_LVDS)
        {
            PNL_DBG(PNL_DBG_LEVEL_DRV,
                "%s %d, Type=%s, Ti=%d, TiBits=%s, Ch_Pol=%d, ChSwap(%d %d %d %d %d), Invert(%d %d %d)\n",
                __FUNCTION__, __LINE__, PARSING_LINKTYPE(pSignalCtrlCfg->enLinkType),
                pSignalCtrlCfg->stModCfg.bTiMode,  PARSING_TI_BIT_MODE(pSignalCtrlCfg->stModCfg.enTiBitMode),
                pSignalCtrlCfg->stModCfg.bChPolarity, pSignalCtrlCfg->stModCfg.enCh[0],
                pSignalCtrlCfg->stModCfg.enCh[1], pSignalCtrlCfg->stModCfg.enCh[2],
                pSignalCtrlCfg->stModCfg.enCh[3], pSignalCtrlCfg->stModCfg.enCh[4],
                pSignalCtrlCfg->stModCfg.bHsyncInvert, pSignalCtrlCfg->stModCfg.bVsyncInvert,
                pSignalCtrlCfg->stModCfg.bDeInvert);
        }
        else if(pSignalCtrlCfg->enLinkType == E_DRV_PNL_LINK_MIPI_DSI)
        {
            PNL_DBG(PNL_DBG_LEVEL_DRV,
                "%s %d, Type=%s, H(%d %d %d %d) V(%d %d %d %d) Lane=%d, Ctrl=%s, Fmt=%s, CmdSize=%d\n",
                __FUNCTION__, __LINE__, PARSING_LINKTYPE(pSignalCtrlCfg->enLinkType),
                pSignalCtrlCfg->stMipiDsiCfg.u16Hactive, pSignalCtrlCfg->stMipiDsiCfg.u16Hpw,
                pSignalCtrlCfg->stMipiDsiCfg.u16Hbp, pSignalCtrlCfg->stMipiDsiCfg.u16Hfp,
                pSignalCtrlCfg->stMipiDsiCfg.u16Vactive, pSignalCtrlCfg->stMipiDsiCfg.u16Vpw,
                pSignalCtrlCfg->stMipiDsiCfg.u16Vbp, pSignalCtrlCfg->stMipiDsiCfg.u16Vfp,
                pSignalCtrlCfg->stMipiDsiCfg.enLaneNum,
                PARSING_MIPI_DSI_CTRL(pSignalCtrlCfg->stMipiDsiCfg.enCtrl),
                PARSING_MIPI_DSI_FMT(pSignalCtrlCfg->stMipiDsiCfg.enformat),
                pSignalCtrlCfg->stMipiDsiCfg.u32CmdBufSize);
        }

        memcpy(&pPnlCtxCfg->stSignalCtrlCfg, pSignalCtrlCfg, sizeof(DrvPnlSignalCtrlConfig_t));

        if(pPnlCtxCfg->stTimingCfg.enLinkType == pSignalCtrlCfg->enLinkType)
        {
            bRet = _DrvPnlSetTimingAndSignalCtrl(&pPnlCtxCfg->stTimingCfg, &pPnlCtxCfg->stSignalCtrlCfg);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Timing(%x %s), Signal(%x %s) not Match\n",
            __FUNCTION__, __LINE__,
            pPnlCtxCfg->stTimingCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stTimingCfg.enLinkType),
            pSignalCtrlCfg->enLinkType,
            PARSING_LINKTYPE(pSignalCtrlCfg->enLinkType));
    }
    return bRet;
}


bool DrvPnlSetTimingConfig(void *pCtx, DrvPnlTimingConfig_t *pstTimingCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(pPnlCtxCfg->stSignalCtrlCfg.enLinkType == E_DRV_PNL_LINK_NONE ||
       pPnlCtxCfg->stSignalCtrlCfg.enLinkType == pstTimingCfg->enLinkType)
    {
        PNL_DBG(PNL_DBG_LEVEL_DRV, "%s %d, Type=%s, H(%d %d %d %d %d) V(%d %d %d %d %d), Fps:%d\n",
            __FUNCTION__, __LINE__, PARSING_LINKTYPE(pstTimingCfg->enLinkType),
            pstTimingCfg->u16Htt, pstTimingCfg->u16Hpw, pstTimingCfg->u16Hbp,
            pstTimingCfg->u16Hstart, pstTimingCfg->u16Hactive,
            pstTimingCfg->u16Vtt, pstTimingCfg->u16Vpw, pstTimingCfg->u16Vbp,
            pstTimingCfg->u16Vstart, pstTimingCfg->u16Vactive,
            pstTimingCfg->u16Fps);

        // backup to ctx
        memcpy(&pPnlCtxCfg->stTimingCfg, pstTimingCfg, sizeof(DrvPnlTimingConfig_t));

        if(pPnlCtxCfg->stSignalCtrlCfg.enLinkType == pstTimingCfg->enLinkType)
        {
            bRet = _DrvPnlSetTimingAndSignalCtrl(&pPnlCtxCfg->stTimingCfg, &pPnlCtxCfg->stSignalCtrlCfg);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d Timing(%d, %s) is not Match Signal(%d, %s)\n",
            __FUNCTION__, __LINE__,
            pstTimingCfg->enLinkType,
            PARSING_LINKTYPE(pstTimingCfg->enLinkType),
            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stSignalCtrlCfg.enLinkType));
    }
    return bRet;
}


bool DrvPnlSetTestPatternConfig(void *pCtx, DrvPnlTestPatternConfig_t *pTestPatCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;


    if(_DrvPnlCheckLinkType(pPnlCtxCfg->stTimingCfg.enLinkType,
                            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
                            pTestPatCfg->enLinkType))
    {
        PNL_DBG(PNL_DBG_LEVEL_DRV, "%s %d: Link:%s, En=%d, (%x %x %x)\n",
            __FUNCTION__, __LINE__, PARSING_LINKTYPE(pTestPatCfg->enLinkType),
            pTestPatCfg->bEn, pTestPatCfg->u16R, pTestPatCfg->u16G, pTestPatCfg->u16B);

        if(pTestPatCfg->enLinkType == E_DRV_PNL_LINK_LVDS)
        {
            HalPnlModTestPatternConfig_t stModTestPatCfg;

            stModTestPatCfg.bEn = pTestPatCfg->bEn;
            stModTestPatCfg.u16R = pTestPatCfg->u16R;
            stModTestPatCfg.u16G = pTestPatCfg->u16G;
            stModTestPatCfg.u16B = pTestPatCfg->u16B;

            HalPnlSetModeTestPattern(&stModTestPatCfg);
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, Link=(%d, %s), Not Suuport\n",
                __FUNCTION__, __LINE__, pTestPatCfg->enLinkType,
                PARSING_LINKTYPE(pTestPatCfg->enLinkType));
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Timing(%d, %s), Signal(%d, %s), TestPat(%d, %s) not match\n",
            __FUNCTION__, __LINE__,
            pPnlCtxCfg->stTimingCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stTimingCfg.enLinkType),
            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stSignalCtrlCfg.enLinkType),
            pTestPatCfg->enLinkType,
            PARSING_LINKTYPE(pTestPatCfg->enLinkType));
    }

    return bRet;
}

bool DrvPnlSetSscConfig(void *pCtx, DrvPnlSscConfig_t *pSscCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    HalPnlLpllSscConfig_t stLpllSscCfg;
    bool bRet = TRUE;

    if(_DrvPnlCheckLinkType(pPnlCtxCfg->stTimingCfg.enLinkType,
                            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
                            pSscCfg->enLinkType))
    {
        PNL_DBG(PNL_DBG_LEVEL_DRV, "%s %d: Link:(%d %s), En=%d, Step=%x, Span=%x\n",
            __FUNCTION__, __LINE__,
            pSscCfg->enLinkType,
            PARSING_LINKTYPE(pSscCfg->enLinkType),
            pSscCfg->bEn, pSscCfg->u16Step, pSscCfg->u16Span);

        memcpy(&pPnlCtxCfg->stSscCfg, pSscCfg, sizeof(DrvPnlSscConfig_t));

        if(pSscCfg->enLinkType == E_DRV_PNL_LINK_LVDS)
        {
            stLpllSscCfg.bEn = pSscCfg->bEn;
            stLpllSscCfg.u16Step = pSscCfg->u16Step;
            stLpllSscCfg.u16Span = pSscCfg->u16Span;

            HalPnlSetLpllSscConfig(&stLpllSscCfg);
        }
        else
        {
            PNL_ERR("%s %d, Link=(%d, %s) Not Supported\n",
                __FUNCTION__, __LINE__, pSscCfg->enLinkType,
                PARSING_LINKTYPE(pSscCfg->enLinkType));
            bRet = FALSE;
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Timing(%x %s), Signal(%x %s), TestPat(%x %s) not match\n",
            __FUNCTION__, __LINE__,
            pPnlCtxCfg->stTimingCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stTimingCfg.enLinkType),
            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stSignalCtrlCfg.enLinkType),
            pSscCfg->enLinkType,
            PARSING_LINKTYPE(pSscCfg->enLinkType));
    }

    return bRet;
}

bool DrvPnlSetTestTgenConfig(void *pCtx, DrvPnlTestTgenConfig_t *pTgenCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    HalPnlModTgenConfig_t stModTgenCfg;
    bool bRet = TRUE;

    if(_DrvPnlCheckLinkType(pPnlCtxCfg->stTimingCfg.enLinkType,
                            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
                            pTgenCfg->enLinkType))
    {

        PNL_DBG(PNL_DBG_LEVEL_DRV, "%s %d, bEn=%d, Timing(%d %d %d %d, %d %d %d %d)\n",
            __FUNCTION__, __LINE__,
            pTgenCfg->bEn,
            pTgenCfg->u16Htt, pTgenCfg->u16Hactive,
            pTgenCfg->u16Hpw, pTgenCfg->u16Hbp,
            pTgenCfg->u16Vtt, pTgenCfg->u16Vactive,
            pTgenCfg->u16Vpw, pTgenCfg->u16Vbp);


        stModTgenCfg.bEn = pTgenCfg->bEn;

        stModTgenCfg.stTimingCfg.u16Htt     = pTgenCfg->u16Htt;
        stModTgenCfg.stTimingCfg.u16Hpw     = pTgenCfg->u16Hpw;
        stModTgenCfg.stTimingCfg.u16Hbp     = pTgenCfg->u16Hbp;
        stModTgenCfg.stTimingCfg.u16Hactive = pTgenCfg->u16Hactive;

        stModTgenCfg.stTimingCfg.u16Vtt     = pTgenCfg->u16Vtt;
        stModTgenCfg.stTimingCfg.u16Vpw     = pTgenCfg->u16Vpw;
        stModTgenCfg.stTimingCfg.u16Vbp     = pTgenCfg->u16Vbp;
        stModTgenCfg.stTimingCfg.u16Vactive = pTgenCfg->u16Vactive;

        stModTgenCfg.stTimingCfg.u16Hstart = 0;
        stModTgenCfg.stTimingCfg.u16Vstart = 0;
        HalPnlSetModTgenConfig(&stModTgenCfg);
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Timing(%x %s), Signal(%x %s), TestPat(%x %s) not match\n",
            __FUNCTION__, __LINE__,
            pPnlCtxCfg->stTimingCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stTimingCfg.enLinkType),
            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stSignalCtrlCfg.enLinkType),
            pTgenCfg->enLinkType,
            PARSING_LINKTYPE(pTgenCfg->enLinkType));
    }

    return bRet;
}

bool DrvPnlSetBackLightConfig(void *pCtx,  DrvPnlBackLightConfig_t *pBackLightCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(_DrvPnlCheckLinkType(pPnlCtxCfg->stTimingCfg.enLinkType,
                            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
                            pBackLightCfg->enLinkType))
    {
        PNL_DBG(PNL_DBG_LEVEL_DRV, "%s %d, Link:%s, En=%d, Level=%x\n",
            __FUNCTION__, __LINE__, PARSING_LINKTYPE(pBackLightCfg->enLinkType),
            pBackLightCfg->bEn, pBackLightCfg->u16Level);

        memcpy(&pPnlCtxCfg->stBackLightCfg, pBackLightCfg, sizeof(DrvPnlBackLightConfig_t));
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Timing(%x %s), Signal(%x %s), TestPat(%x %s) not match\n",
            __FUNCTION__, __LINE__,
            pPnlCtxCfg->stTimingCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stTimingCfg.enLinkType),
            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stSignalCtrlCfg.enLinkType),
            pBackLightCfg->enLinkType,
            PARSING_LINKTYPE(pBackLightCfg->enLinkType));
    }
    return bRet;
}

bool DrvPnlSetPowerConfig(void *pCtx, DrvPnlPowerConfig_t *pPowerCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(_DrvPnlCheckLinkType(pPnlCtxCfg->stTimingCfg.enLinkType,
                            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
                            pPowerCfg->enLinkType))
    {

        PNL_DBG(PNL_DBG_LEVEL_DRV, "%s %d, Link:%s, En=%d \n",
            __FUNCTION__, __LINE__, PARSING_LINKTYPE(pPowerCfg->enLinkType),
            pPowerCfg->bEn);

        memcpy(&pPnlCtxCfg->stPowerCfg, pPowerCfg, sizeof(DrvPnlPowerConfig_t));
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Timing(%x %s), Signal(%x %s), TestPat(%x %s) not match\n",
            __FUNCTION__, __LINE__,
            pPnlCtxCfg->stTimingCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stTimingCfg.enLinkType),
            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stSignalCtrlCfg.enLinkType),
            pPowerCfg->enLinkType,
            PARSING_LINKTYPE(pPowerCfg->enLinkType));
    }
    return bRet;
}

bool DrvPnlGetBackLightConfig(void *pCtx,  DrvPnlBackLightConfig_t *pBackLightCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    HalPnlBackLightConfig_t stHalBackLigtCfg;
    bool bRet = TRUE;

    if(_DrvPnlCheckLinkType(pPnlCtxCfg->stTimingCfg.enLinkType,
                            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
                            pBackLightCfg->enLinkType))
    {
        PNL_DBG(PNL_DBG_LEVEL_DRV, "%s %d, Link:%s, En=%d, Level=%x\n",
            __FUNCTION__, __LINE__, PARSING_LINKTYPE(pBackLightCfg->enLinkType),
            pBackLightCfg->bEn, pBackLightCfg->u16Level);

        memcpy(pBackLightCfg, &pPnlCtxCfg->stBackLightCfg, sizeof(DrvPnlBackLightConfig_t));

        stHalBackLigtCfg.bEn      = pBackLightCfg->bEn;
        stHalBackLigtCfg.u16Level = pBackLightCfg->u16Level;
        HalPnlSetBackLightConfig(&stHalBackLigtCfg);
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Timing(%x %s), Signal(%x %s), TestPat(%x %s) not match\n",
            __FUNCTION__, __LINE__,
            pPnlCtxCfg->stTimingCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stTimingCfg.enLinkType),
            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stSignalCtrlCfg.enLinkType),
            pBackLightCfg->enLinkType,
            PARSING_LINKTYPE(pBackLightCfg->enLinkType));
    }

    return bRet;
}

bool DrvPnlGetPowerConfig(void *pCtx, DrvPnlPowerConfig_t *pPowerCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    HalPnlPowerConfig_t stHalPowerCfg;
    bool bRet = TRUE;


    if(_DrvPnlCheckLinkType(pPnlCtxCfg->stTimingCfg.enLinkType,
                            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
                            pPowerCfg->enLinkType))
    {

        PNL_DBG(PNL_DBG_LEVEL_DRV, "%s %d, Link:%s, En=%d \n",
            __FUNCTION__, __LINE__, PARSING_LINKTYPE(pPowerCfg->enLinkType),
            pPowerCfg->bEn);

        memcpy(pPowerCfg, &pPnlCtxCfg->stPowerCfg, sizeof(DrvPnlPowerConfig_t));

        stHalPowerCfg.bEn = pPowerCfg->bEn;
        HalPnlSetPowerConfig(&stHalPowerCfg);
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Timing(%x %s), Signal(%x %s), TestPat(%x %s) not match\n",
            __FUNCTION__, __LINE__,
            pPnlCtxCfg->stTimingCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stTimingCfg.enLinkType),
            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stSignalCtrlCfg.enLinkType),
            pPowerCfg->enLinkType,
            PARSING_LINKTYPE(pPowerCfg->enLinkType));
    }
    return bRet;
}


bool DrvPnlSetDrvCurrentConfig(void *pCtx, DrvPnlDrvCurrentConfig_t *pDrvCurrentCfg)
{
    DrvPnlCtxConfig_t *pPnlCtxCfg = (DrvPnlCtxConfig_t *)pCtx;
    HalPnlDrvCurrentConfig_t stHalDrvCurrentCfg;
    bool bRet = TRUE;

    if(_DrvPnlCheckLinkType(pPnlCtxCfg->stTimingCfg.enLinkType,
                            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
                            pDrvCurrentCfg->enLinkType))
    {

        PNL_DBG(PNL_DBG_LEVEL_DRV, "%s %d, Link:%s, DrvCur=%04x \n",
            __FUNCTION__, __LINE__, PARSING_LINKTYPE(pDrvCurrentCfg->enLinkType),
            pDrvCurrentCfg->u16Val);

        memcpy(&pPnlCtxCfg->stDrvCurrentCfg, pDrvCurrentCfg, sizeof(DrvPnlDrvCurrentConfig_t));

        stHalDrvCurrentCfg.u16Val = pDrvCurrentCfg->u16Val;
        HalPnlSetDrvCurrentConfig(&stHalDrvCurrentCfg);
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Timing(%x %s), Signal(%x %s), TestPat(%x %s) not match\n",
            __FUNCTION__, __LINE__,
            pPnlCtxCfg->stTimingCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stTimingCfg.enLinkType),
            pPnlCtxCfg->stSignalCtrlCfg.enLinkType,
            PARSING_LINKTYPE(pPnlCtxCfg->stSignalCtrlCfg.enLinkType),
            pDrvCurrentCfg->enLinkType,
            PARSING_LINKTYPE(pDrvCurrentCfg->enLinkType));
    }
    return bRet;
}

bool DrvPnlSetDbgLevelConfig(void *pCtx, u32 u32Level)
{
    _gu32PnlDbgLevel = u32Level;
    return TRUE;
}

