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
#define __HAL_PNL_C__

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
// Internal Definition
#ifdef MSOS_TYPE_NOS
#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#else
#include <linux/delay.h>
#endif

#include "cam_os_wrapper.h"
#include "ms_platform.h"
#include "drv_pnl_dbg.h"
#include "hal_pnl_util.h"
#include "hal_pnl_reg.h"
#include "hal_pnl_lpll_tbl.h"
#include "hal_pnl.h"
#include "hal_pnl_dsi_reg.h"
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define AS_UINT32(x)    (*(volatile u32 *)((void*)x))

#define ALIGN_TO(x, n)  (((x) + ((n) - 1)) & ~((n) - 1))

#define PARSING_HAL_MIPI_DSI_CTRL(x)    (x == E_HAL_PNL_MIPI_DSI_CMD_MODE   ? "Cmd_Mode"   : \
                                         x == E_HAL_PNL_MIPI_DSI_SYNC_PULSE ? "SYNC_PULSE" : \
                                         x == E_HAL_PNL_MIPI_DSI_SYNC_EVENT ? "Sync_Event" : \
                                         x == E_HAL_PNL_MIPI_DSI_BURST_MODE ? "Burst_Mode" : \
                                                                              "UNKNOWN")

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
void _HalPnlSetLpllSet(u16 u16Htotal, u16 u16Vtotal, u16 u16Fps, u8 *pLpllTbl)
{
    u8 u8LoopDiv, u8LoopGain;
    u64 u64Dividen, u64Divisor, u64LplLSet, u64Reminder;
    u16 u16Val;

    u8LoopGain = pLpllTbl[3];
    u8LoopDiv  = pLpllTbl[7];

    u64Dividen = (u64)432000 * (u64)524288 * (u64)u8LoopGain;
    u64Divisor = (u64)u16Htotal * (u64)u16Vtotal * (u64)u16Fps * (u64)u8LoopDiv / 1000;

    u64LplLSet= CamOsMathDivU64(u64Dividen, u64Divisor, &u64Reminder);

    u16Val = u64LplLSet & 0x0000FFFF;
    W2BYTE(REG_LPLL_0F_L, u16Val);

    u16Val = (u64LplLSet >> 16) & 0xFFFF;
    W2BYTE(REG_LPLL_10_L, u16Val);

    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, Htt=%d, Vtt=%d, Fps=%d, Gain=%d, Div=%d",
        __FUNCTION__, __LINE__, u16Htotal, u16Vtotal, u16Fps, u8LoopGain, u8LoopDiv);

    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, Divden=%llx, Divisor=%llx lpllset=%llx\n",
        __FUNCTION__, __LINE__, u64Dividen, u64Divisor, u64LplLSet);
}


void _HalPnlDumpLpllRegTab(u8 *pLpllTbl)
{
    u32 u32TabIdx = 0;
    u32 u32Addr;
    u8  u8Msk;
    u8  u8Val;
    u16 u16TimeOut = 0xFFFF;

    while(u16TimeOut--)
    {
        u32Addr = pLpllTbl[u32TabIdx] << 8 | pLpllTbl[u32TabIdx+1];
        u8Msk   = pLpllTbl[u32TabIdx+2];
        u8Val   = pLpllTbl[u32TabIdx+3];

        if(u32Addr == REG_TABLE_END)
        {
            break;
        }
        WBYTEMSK(u32Addr | 0x100000, u8Val, u8Msk);

        PNL_DBG(PNL_DBG_LEVE_REG_RW, "%s %d, %08x, %02x %02x\n",
            __FUNCTION__, __LINE__, u32Addr|0x100000, u8Val, u8Msk);

        if(u16TimeOut == 0)
        {
            PNL_ERR("%s %d, TimeOut\n", __FUNCTION__, __LINE__);
        }
        u32TabIdx = u32TabIdx + 4;
    }
}


void HalPnlInitClk(void)
{
    W2BYTE(REG_CLKGEN2_57_L, 0x1400);
    W2BYTE(REG_CLKGEN1_1E_L ,0x0000);
    W2BYTE(REG_CLKGEN0_53_L, 0x0000);
}



void HalPnlSetModConfig(HalPnlModConfig_t *pModCfg)
{
    u16 u16RegMod29, u16RegMod20, u16RegMod38, u16RegMod39;

    u16RegMod29 = BIT11;
    u16RegMod29 |= pModCfg->bDeInvert ? BIT8 : 0;
    u16RegMod29 |= pModCfg->bVsyncInvert ? BIT9 : 0;
    u16RegMod29 |= pModCfg->bHsyncInvert ? BIT10 : 0;

    u16RegMod20 = pModCfg->bTiMode ? BIT1 : 0;
    u16RegMod20 |= pModCfg->bChPolarity ? BIT3 : 0;
    u16RegMod20 |= (pModCfg->enTiBitMode <<8);


    u16RegMod38 = ((u16)(pModCfg->enCh[0] & 0x000F) << 0) | ((u16)(pModCfg->enCh[1] & 0x000F) << 4) |
                  ((u16)(pModCfg->enCh[2] & 0x000F) << 8) | ((u16)(pModCfg->enCh[3] & 0x000F) << 12);

    u16RegMod39 = (pModCfg->enCh[4] & 0x000F);

    W2BYTE(REG_MOD_34_L, 0x0000);
    W2BYTE(REG_MOD_29_L, u16RegMod29);
    W2BYTE(REG_MOD_20_L, u16RegMod20);
    W2BYTE(REG_MOD_38_L, u16RegMod38);
    W2BYTE(REG_MOD_39_L, u16RegMod39);

#if 0
    // Test pattern
    W2BYTE(REG_MOD_00_L, 0x3FFF);
    W2BYTE(REG_MOD_01_L, 0x3FFF);
    W2BYTE(REG_MOD_02_L, 0x3FFF);
    W2BYTE(REG_MOD_03_L, 0x0001);
    //Tgen
    W2BYTE(REG_MOD_08_L, 0x05A0);
    W2BYTE(REG_MOD_09_L, 0x0014);
    W2BYTE(REG_MOD_0A_L, 0x005A);
    W2BYTE(REG_MOD_0B_L, 0x0500);
    W2BYTE(REG_MOD_0C_L, 0x0337);
    W2BYTE(REG_MOD_0D_L, 0x0003);
    W2BYTE(REG_MOD_0E_L, 0x000D);
    W2BYTE(REG_MOD_0F_L, 0x0320);
    W2BYTE(REG_MOD_10_L, 0x00F3);
#endif
}


void HalPnlInitLvdsDphy(void)
{
    W2BYTE(REG_DPHY_00_L ,0x0008);
    W2BYTE(REG_DPHY_01_L ,0x0000);
    W2BYTE(REG_DPHY_2D_L ,0x0001);
    W2BYTE(REG_DPHY_36_L ,0x0000);
    W2BYTE(REG_DPHY_06_L ,0x0008);
    W2BYTE(REG_DPHY_09_L ,0x0020);
    W2BYTE(REG_DPHY_0C_L ,0x2004);
    W2BYTE(REG_DPHY_13_L ,0x006C);
    W2BYTE(REG_DPHY_16_L ,0x0090);
    W2BYTE(REG_DPHY_01_L ,0x0000);
    W2BYTE(REG_DPHY_21_L ,0x0100);
    W2BYTE(REG_DPHY_23_L ,0x001F);
    W2BYTE(REG_DPHY_28_L ,0x001F);
    W2BYTE(REG_DPHY_29_L ,0x6760);
    W2BYTE(REG_DPHY_04_L ,0x014F);
    W2BYTE(REG_DPHY_07_L ,0x014F);
    W2BYTE(REG_DPHY_0A_L ,0x014F);
    W2BYTE(REG_DPHY_11_L ,0x014F);
    W2BYTE(REG_DPHY_14_L ,0x014F);
}

void HalPnlSetLpllConfig(HalPnlLpllConfig_t *pLpllCfg)
{
    PNL_TAB_INFO *pTblInfo = NULL;

    if(pLpllCfg->enLinkType == E_HAL_PNL_LINK_LVDS)
    {
        if(pLpllCfg->u16Hactive == 1280 && pLpllCfg->u16Vactive == 800)
        {
            pTblInfo = &MipiDsiPanel_MainTbl[PNL_PANEL_LVDS_1CH_1280x800];
        }
        else
        {
            pTblInfo = NULL;
        }

    }
    else if(pLpllCfg->enLinkType == E_HAL_PNL_LINK_MIPI_DSI)
    {
        if(pLpllCfg->u16Hactive == 720 && pLpllCfg->u16Vactive == 1280)
        {
            if(pLpllCfg->enMipiDsiLane == E_HAL_PNL_MIPI_DSI_LANE_4)
            {
                pTblInfo = (pLpllCfg->enMipiDsiFmt == E_HAL_PNL_MIPI_DSI_RGB565) ? &MipiDsiPanel_MainTbl[PNL_PANEL_720x1280_RGB565_4Lane] :
                           (pLpllCfg->enMipiDsiFmt == E_HAL_PNL_MIPI_DSI_RGB666) ? &MipiDsiPanel_MainTbl[PNL_PANEL_720x1280_RGB666_4Lane] :
                                                                                   &MipiDsiPanel_MainTbl[PNL_PANEL_720x1280_RGB888_4Lane];

            }
            else if(pLpllCfg->enMipiDsiLane == E_HAL_PNL_MIPI_DSI_LANE_2)
            {
                pTblInfo = (pLpllCfg->enMipiDsiFmt == E_HAL_PNL_MIPI_DSI_RGB565) ? &MipiDsiPanel_MainTbl[PNL_PANEL_720x1280_RGB565_2Lane] :
                           (pLpllCfg->enMipiDsiFmt == E_HAL_PNL_MIPI_DSI_RGB666) ? &MipiDsiPanel_MainTbl[PNL_PANEL_720x1280_RGB666_2Lane] :
                                                                                   &MipiDsiPanel_MainTbl[PNL_PANEL_720x1280_RGB888_2Lane];
            }
            else
            {
                pTblInfo = NULL;
            }
        }
    }

    if(pTblInfo)
    {
        _HalPnlDumpLpllRegTab(pTblInfo->pDacINIT_LPLLTab);
        _HalPnlSetLpllSet(pLpllCfg->u16Htotal, pLpllCfg->u16Vtotal, pLpllCfg->u16Fps, pTblInfo->pDacINIT_LPLL_DIV_GAINTab);
    }

}

void HalPnlSetModeTestPattern(HalPnlModTestPatternConfig_t *pTestPatCfg)
{
    W2BYTE(REG_MOD_00_L, pTestPatCfg->u16R & 0x3FF);
    W2BYTE(REG_MOD_01_L, pTestPatCfg->u16G & 0x3FF);
    W2BYTE(REG_MOD_02_L, pTestPatCfg->u16B & 0x3FF);
    W2BYTEMSK(REG_MOD_03_L, pTestPatCfg->bEn ? 0x01 : 0x00, 0x01);
}


void HalPnlSetModTgenConfig(HalPnlModTgenConfig_t *pTgenCfg)
{
    W2BYTE(REG_MOD_08_L, pTgenCfg->stTimingCfg.u16Htt);
    W2BYTE(REG_MOD_09_L, pTgenCfg->stTimingCfg.u16Hpw);
    W2BYTE(REG_MOD_0A_L, pTgenCfg->stTimingCfg.u16Hpw + pTgenCfg->stTimingCfg.u16Hbp);
    W2BYTE(REG_MOD_0B_L, pTgenCfg->stTimingCfg.u16Hactive);

    W2BYTE(REG_MOD_0C_L, pTgenCfg->stTimingCfg.u16Vtt);
    W2BYTE(REG_MOD_0D_L, pTgenCfg->stTimingCfg.u16Vpw);
    W2BYTE(REG_MOD_0E_L, pTgenCfg->stTimingCfg.u16Vpw + pTgenCfg->stTimingCfg.u16Vbp);
    W2BYTE(REG_MOD_0F_L, pTgenCfg->stTimingCfg.u16Vactive);
    W2BYTE(REG_MOD_10_L, pTgenCfg->bEn ? 0x00F3: 0x0000);
}


void HalPnlSetLpllSscConfig(HalPnlLpllSscConfig_t *pSscCfg)
{
    W2BYTEMSK(REG_LPLL_4E_L, pSscCfg->u16Step, 0x0FFF);
    W2BYTEMSK(REG_LPLL_4F_L, pSscCfg->u16Span, 0x3FFF);
    W2BYTEMSK(REG_LPLL_4E_L, pSscCfg->bEn ? 0x8000: 0x0000, 0x8000);
}


void HalPnlSetTimingConfig(HalPnlTimingConfig_t *pTimingCfg)
{
    u16 u16VsyncStart, u16HsyncStart;

    u16HsyncStart = (pTimingCfg->u16Hstart == 0) ?
                    pTimingCfg->u16Htt - pTimingCfg->u16Hpw - pTimingCfg->u16Hbp :
                    0;

    u16VsyncStart = (pTimingCfg->u16Vstart == 0) ?
                    pTimingCfg->u16Vtt - pTimingCfg->u16Vpw - pTimingCfg->u16Vbp :
                    0;


    //Htt & Vtt
    W2BYTE(REG_BK10_VOP_0C_L, pTimingCfg->u16Htt-1);
    W2BYTE(REG_BK10_VOP_0D_L, pTimingCfg->u16Vtt-1);

    // Hde/Vde
    W2BYTE(REG_BK10_VOP_04_L, pTimingCfg->u16Hstart);
    W2BYTE(REG_BK10_VOP_05_L, pTimingCfg->u16Hstart + pTimingCfg->u16Hactive - 1);
    W2BYTE(REG_BK10_VOP_06_L, pTimingCfg->u16Vstart);
    W2BYTE(REG_BK10_VOP_07_L, pTimingCfg->u16Vstart + pTimingCfg->u16Vactive - 1);

    //Scaling image widnow
    W2BYTE(REG_BK10_VOP_08_L, pTimingCfg->u16Hstart);
    W2BYTE(REG_BK10_VOP_09_L, pTimingCfg->u16Hstart + pTimingCfg->u16Hactive - 1);
    W2BYTE(REG_BK10_VOP_0A_L, pTimingCfg->u16Vstart);
    W2BYTE(REG_BK10_VOP_0B_L, pTimingCfg->u16Vstart + pTimingCfg->u16Vactive - 1);

    //Hsync end
    W2BYTE(REG_BK10_VOP_01_L, u16HsyncStart + pTimingCfg->u16Hpw - 1);

    //Vsync start/end
    W2BYTEMSK(REG_BK10_VOP_02_L, ((u16VsyncStart & 0x7FF) | ((u16VsyncStart & 0x800) << 1)), 0x17FF);
    W2BYTE(REG_BK10_VOP_03_L, u16VsyncStart + pTimingCfg->u16Vpw - 1);
}

void HalPnlSetBackLightConfig(HalPnlBackLightConfig_t *pBackLightCfg)
{
    PNL_ERR("%s %d, TBD\n", __FUNCTION__, __LINE__);
}


void HalPnlSetPowerConfig(HalPnlPowerConfig_t *pPowerCfg)
{
    PNL_ERR("%s %d, TBD\n", __FUNCTION__, __LINE__);
}


void HalPnlSetDrvCurrentConfig(HalPnlDrvCurrentConfig_t *pDrvCurCfg)
{
    PNL_ERR("%s %d, TBD\n", __FUNCTION__, __LINE__);
}

//-------------------------------------------------------------------------------
// Mipi
//

void HalPnlInitMipiDsiDphy(void)
{
    W2BYTE(REG_DPHY_00_L, 0x0000);  //local sw reset, active low
    W2BYTE(REG_DPHY_01_L, 0x0000);  // clear power down hs mode, power down whole dphy analog
    //TBD: Not need to setup cken?
    W2BYTE(REG_DPHY_04_L, 0x0000);  //disable ch0 sw setting
    W2BYTE(REG_DPHY_08_L, 0x0000);  //disable ch1 sw setting
    W2BYTE(REG_DPHY_0A_L, 0x0180);  //disable ch2 sw setting
    W2BYTE(REG_DPHY_0E_L, 0x0080);  //dummy register 0
    W2BYTE(REG_DPHY_11_L, 0x0000);  //disable ch3 sw setting
    W2BYTE(REG_DPHY_14_L, 0x0000);  //disable ch4 sw setting
    W2BYTE(REG_DPHY_17_L, 0xC000);  //
    W2BYTE(REG_DPHY_18_L, 0x0080);  //LP bist pattern enable
    W2BYTE(REG_DPHY_22_L, 0x0000);  //clk csi dsi phy
    W2BYTE(REG_DPHY_28_L, 0x001F);
    W2BYTE(REG_DPHY_29_L, 0x6760);
}



void HalPnlSetMipiDsiPadOutSel(HalPnlMipiDsiLaneMode_e enLaneMode)
{
    W2BYTEMSK(REG_PAD_TOP1_0A_L, 0x0000, 0x0003); //Disable RGB PAD

    switch(enLaneMode)
    {
        case E_HAL_PNL_MIPI_DSI_LANE_4:
            W2BYTEMSK(REG_PAD_TOP1_07_L, BIT5, BIT4|BIT5);
            break;

        case E_HAL_PNL_MIPI_DSI_LANE_2:
            W2BYTEMSK(REG_PAD_TOP1_07_L, BIT4, BIT4|BIT5);
            break;

        default:
            PNL_ERR("%s %d, LaneNum=%d Not Support\n", __FUNCTION__, __LINE__, enLaneMode);
            return;
    }

    W2BYTE(REG_PAD_TOP1_46_L, 0x0000);
    W2BYTE(REG_PAD_TOP1_47_L, 0x0000);
    W2BYTE(REG_PAD_TOP1_48_L, 0x0000);
    W2BYTE(REG_PAD_TOP1_49_L, 0x0000);
}

void HalPnlResetMipiDsi(void)
{
    W4BYTE(DSI_REG_BASE +(REG_DSI_COM_CON>>1), DSI_DSI_RESET | DSI_DPHY_RESET);
    W4BYTE(DSI_REG_BASE +(REG_DSI_COM_CON>>1), 0);
}

void HalPnlEnableMipiDsiClk(void)
{
    W4BYTEMSK(DSI_REG_BASE +(REG_DSI_START>>1), 0, DSI_DSI_START); //clear first
    W4BYTEMSK(DSI_REG_BASE +(REG_DSI_START>>1), DSI_DSI_START, DSI_DSI_START);
}

void HalPnlDisableMipiDsiClk(void)
{
    W4BYTEMSK(DSI_REG_BASE +(REG_DSI_START>>1), 0, DSI_DSI_START);
}

void HalPnlSetMipiDsiLaneNum(HalPnlMipiDsiLaneMode_e enLaneMode)
{
    switch(enLaneMode)
    {
        case E_HAL_PNL_MIPI_DSI_LANE_1:
            W4BYTEMSK(DSI_REG_BASE +(REG_DSI_TXRX_CON>>1), 0x1<<2, DSI_LANE_NUM);
            break;
        case E_HAL_PNL_MIPI_DSI_LANE_2:
            W4BYTEMSK(DSI_REG_BASE +(REG_DSI_TXRX_CON>>1), 0x3<<2, DSI_LANE_NUM);
            break;
        case E_HAL_PNL_MIPI_DSI_LANE_3:
            W4BYTEMSK(DSI_REG_BASE +(REG_DSI_TXRX_CON>>1), 0x7<<2, DSI_LANE_NUM);
            break;
        case E_HAL_PNL_MIPI_DSI_LANE_4:
            W4BYTEMSK(DSI_REG_BASE +(REG_DSI_TXRX_CON>>1), 0xF<<2, DSI_LANE_NUM);
            break;
        case E_HAL_PNL_MIPI_DSI_LANE_NONE:
            W4BYTEMSK(DSI_REG_BASE +(REG_DSI_TXRX_CON>>1), 0x0<<2, DSI_LANE_NUM);
            break;
    }
}

void HalPnlSetMipiDsiCtrlMode(HalPnlMipiDsiCtrlMode_e enCtrlMode)
{
    HalPnlResetMipiDsi();

    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, Ctrl=%s\n",
        __FUNCTION__, __LINE__, PARSING_HAL_MIPI_DSI_CTRL(enCtrlMode));

    switch(enCtrlMode)
    {
        case E_HAL_PNL_MIPI_DSI_CMD_MODE:
            W4BYTEMSK(DSI_REG_BASE +(REG_DSI_MODE_CON>>1), DSI_MODE_CON_CMD, DSI_MODE_MASK);
            break;

        case E_HAL_PNL_MIPI_DSI_SYNC_PULSE:
            W4BYTEMSK(DSI_REG_BASE +(REG_DSI_MODE_CON>>1), DSI_MODE_CON_VID_SYNC_PULSE, DSI_MODE_MASK);
            break;

        case E_HAL_PNL_MIPI_DSI_SYNC_EVENT:
            W4BYTEMSK(DSI_REG_BASE +(REG_DSI_MODE_CON>>1), DSI_MODE_CON_VID_SYNC_EVENT, DSI_MODE_MASK);
            break;

        case E_HAL_PNL_MIPI_DSI_BURST_MODE:
            W4BYTEMSK(DSI_REG_BASE +(REG_DSI_MODE_CON>>1), DSI_MODE_CON_BURST, DSI_MODE_MASK);
            break;
    }
}

bool HalPnlGetMipiDsiClkHsMode(void)
{
    return R4BYTEMSK(DSI_REG_BASE +(REG_DSI_PHY_LCCON>>1), DSI_LC_HSTX_EN) ? 1 : 0;
}

void HalPnlSetMpiDsiClkHsMode(bool bEn)
{
    if(bEn && !HalPnlGetMipiDsiClkHsMode())
    {
        W4BYTEMSK(DSI_REG_BASE +(REG_DSI_PHY_LCCON>>1), 1, DSI_LC_HSTX_EN); //
    }
    else if (!bEn && HalPnlGetMipiDsiClkHsMode())
    {
        W4BYTEMSK(DSI_REG_BASE +(REG_DSI_PHY_LCCON>>1), 0, DSI_LC_HSTX_EN); //
    }
}

bool HalPnlSetMipiDsiShortPacket(u8 u8Count, u8 u8Cmd, u8 *pu8ParamList)
{
    u16 u16CmdqIdx=0;
    //PDSI_T0_INS ptrT0;
    HalPnlMipiDsiCmdqConfig_t stCmdqCfg;
    HalPnlMipiDsiT0Ins_t stT0;
    u32 u32DsiIntSta = 0;
    u32 u32TimeOut = 0xFF;

    u16CmdqIdx     = 0;

    stCmdqCfg.u8Bta  = 0;
    stCmdqCfg.u8Hs   = 0; //Low power mode transfer
    stCmdqCfg.u8Type = 0; //type0
    stCmdqCfg.u8Te   = 0;
    stCmdqCfg.u8Rsv  = 0;
    stCmdqCfg.u8Cl   = 0;
    stCmdqCfg.u8Rpt  = 0;

    stT0.u8Confg= 0;

    if((pu8ParamList == NULL)  && (u8Count != 0))
    {
        return 0;
    }

    if(u8Count > 2)
    {
        return 0;
    }
    else
    {
        if(u8Count==2)
        {
             stT0.u8DataId = HAL_PNL_MIPI_DSI_DCS_SHORT_PACKET_ID_1 ;  //
             stT0.u8Data0  = u8Cmd;
             stT0.u8Data1  = *(pu8ParamList);
        }
        else if(u8Count==1)
        {
             stT0.u8DataId = HAL_PNL_MIPI_DSI_DCS_SHORT_PACKET_ID_0; // ;
             stT0.u8Data0  = u8Cmd;
             stT0.u8Data1  = 0;
        }
    }
    PNL_DBG(PNL_DBG_LEVEL_RW_PACKET, "%s %d, stT0 = %08x\n", __FUNCTION__, __LINE__, AS_UINT32(&stT0));

    //clear u8Cmd done flag first
    WriteLongRegBit(DSI_REG_BASE +(REG_DSI_INTSTA>>1), 0, DSI_CMD_DONE_INT_FLAG);

    W4BYTE(DSI_CMDQ_BASE +(u16CmdqIdx*2),  AS_UINT32(&stT0));

    W4BYTEMSK(DSI_REG_BASE +(REG_DSI_CMDQ_CON>>1), 1, DSI_CMDQ_SIZE);
    WriteLongRegBit(DSI_REG_BASE +(REG_DSI_START>>1), 0, DSI_DSI_START); //clear first
    WriteLongRegBit(DSI_REG_BASE +(REG_DSI_START>>1), 1, DSI_DSI_START);

    do
    {
        u32DsiIntSta =R4BYTE(DSI_REG_BASE + (REG_DSI_INTSTA>>1));
        u32TimeOut--;
        PNL_DBG(PNL_DBG_LEVEL_RW_PACKET, "%s %d, u32DsiIntSta = %08x \n", __FUNCTION__, __LINE__, u32DsiIntSta);
     } while( !(u32DsiIntSta & DSI_CMD_DONE_INT_FLAG) && u32TimeOut);
      WriteLongRegBit(DSI_REG_BASE +(REG_DSI_INTSTA>>1), 0, DSI_CMD_DONE_INT_FLAG);

    if(u32TimeOut == 0)
    {
        PNL_ERR("%s %d, CMD Done Time Out, DsiIntSta=%x\n",  __FUNCTION__,  __LINE__, u32DsiIntSta);
    }

    #if 0
    W4BYTEMSK(DSI_CMDQ_BASE +u16Offset, 0, DSI_TYPE);

    W4BYTEMSK(DSI_CMDQ_BASE +u16Offset,  dataID<<8, DSI_DATA_ID);
    WriteLongRegBit(DSI_CMDQ_BASE +u16Offset, 0, DSI_HS);
    WriteLongRegBit(DSI_CMDQ_BASE +u16Offset, 0, DSI_BTA);

    W4BYTEMSK(DSI_CMDQ_BASE +u16Offset,  data0<<16, DSI_DATA_0);


    W4BYTEMSK(DSI_REG_BASE +(REG_DSI_CMDQ_CON>>1), u8Count, DSI_CMDQ_SIZE);

    WriteLongRegBit(DSI_REG_BASE +(REG_DSI_START>>1), 0, DSI_DSI_START); //clear first
    WriteLongRegBit(DSI_REG_BASE +(REG_DSI_START>>1), 1, DSI_DSI_START);
    #endif

    return 1;
}


bool HalPnlSetMipiDsiLongPacket(u8 u8Count, u8 u8Cmd, u8 *pu8ParamList)
{
    u8  i;
    u16 u16Offset = 0;
    u8  u8CmdStIdx = 0;

    HalPnlMipiDsiT2Ins_t  *pstT2;
    HalPnlMipiDsiCmdqConfig_t stCmdqCfg;
    HalPnlMipiDsiCmdq_t stCmdqDataSeq;
    HalPnlMipiDsiCmdq_t *pstCmdqDataSeq;
    u8 u8CMDQNum = 0;
    u32 u32DsiIntSta = 0;
    bool bFirstCmd = 0;
    u32 u32TimeOut = 0xFF;

    if(pu8ParamList == NULL)
    {
        PNL_ERR("%s %d, pu8ParamList NULL \n", __FUNCTION__, __LINE__);
        return 0;
    }

    pstT2=(HalPnlMipiDsiT2Ins_t *) (CHIP_BASE(u32, DSI_CMDQ_BASE_2, u16Offset, 2));

    stCmdqCfg.u8Bta   = 0;
    stCmdqCfg.u8Hs    = 0;   //Low power mode transfer
    stCmdqCfg.u8Type  = 2; //type2
    stCmdqCfg.u8Te    = 0;

    pstT2->u8Confg= 0x02; //Low power mode transfer, type2

    pstT2->u8DataId = HAL_PNL_MIPI_DSI_DCS_LONG_WRITE_PACKET_ID; //DSI_GERNERIC_LONG_PACKET_ID ;
    pstT2->u16Wc = u8Count;

     //HAL_MIPI_DSI_DBG("u32Addr =%x,  val=%x ", CHIP_BASE(u32, DSI_CMDQ_BASE_2, u16Offset, 2) , R4BYTE(DSI_CMDQ_BASE_2+u16Offset<<1) );
    u16Offset = 1;
    pstCmdqDataSeq=(HalPnlMipiDsiCmdq_t *) (CHIP_BASE(u32, DSI_CMDQ_BASE_2, u16Offset, 2));

    u8CMDQNum = (u8Count/4);
    u8CmdStIdx = 1;



    for(i=0; i <u8CMDQNum ; i++)
    {
        if(!bFirstCmd)
        {
            stCmdqDataSeq.byte0 = u8Cmd;
            bFirstCmd = 1;
        }
        else
        {
            stCmdqDataSeq.byte0 = *pu8ParamList;
            pu8ParamList += 1;
        }
        stCmdqDataSeq.byte1 = *pu8ParamList;
        pu8ParamList += 1;
        stCmdqDataSeq.byte2 = *pu8ParamList;
        pu8ParamList += 1;
        stCmdqDataSeq.byte3 = *pu8ParamList;
        pu8ParamList += 1;

        W4BYTE(DSI_CMDQ_BASE +((u8CmdStIdx)*2),  AS_UINT32(&stCmdqDataSeq));

        //if (i==0)
        //{
        //    HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, "byte0=%x, byte1=%x, byte2=%x, byte3=%x, \n", stCmdqDataSeq.byte0, stCmdqDataSeq.byte1, stCmdqDataSeq.byte2, stCmdqDataSeq.byte3);
        //}
        //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, " u8CmdStIdx =%d, CMDQ=%x \n", u8CmdStIdx, R4BYTE(DSI_CMDQ_BASE +(u8CmdStIdx*2)));
        u8CmdStIdx++;

        //u8Index++;
    }

    u8CMDQNum += (u8Count % 4) ? 1 : 0;

    stCmdqDataSeq.byte0 = 0x0;
    stCmdqDataSeq.byte1 = 0x0;
    stCmdqDataSeq.byte2 = 0x0;
    stCmdqDataSeq.byte3 = 0x0;

    if((u8Count % 4) == 3)
    {
        if(!bFirstCmd)
        {
            stCmdqDataSeq.byte0=u8Cmd;
            bFirstCmd = 1;
        }
        else
        {
            stCmdqDataSeq.byte0=*pu8ParamList;
            pu8ParamList += 1;
        }
        stCmdqDataSeq.byte1 = *pu8ParamList;
        pu8ParamList += 1;
        stCmdqDataSeq.byte2 = *pu8ParamList;
        pu8ParamList += 1;
       //u8CmdStIdx++;

        //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, "byte0=%x, byte1=%x, byte2=%x, byte3=%x, \n", stCmdqDataSeq.byte0, stCmdqDataSeq.byte1, stCmdqDataSeq.byte2, stCmdqDataSeq.byte3);
        if(u8Count == 3)
        {
            u8CmdStIdx = 1;
        }
        W4BYTE(DSI_CMDQ_BASE +(u8CmdStIdx*2),  AS_UINT32(&stCmdqDataSeq));
        //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, " u8CmdStIdx =%d, CMDQ=%x \n", u8CmdStIdx, R4BYTE(DSI_CMDQ_BASE +(u8CmdStIdx*2)));
    }
    else if((u8Count%4) == 2)
    {
        if(!bFirstCmd)
        {
            stCmdqDataSeq.byte0 = u8Cmd;
            bFirstCmd = 1;
        }
        else
        {
            stCmdqDataSeq.byte0 = *pu8ParamList;
            pu8ParamList += 1;
        }
        stCmdqDataSeq.byte1 = *pu8ParamList;
        pu8ParamList += 1;
        //pstCmdqDataSeq[u8Index]=stCmdqDataSeq ;
       // u8CmdStIdx++;
       //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, "Case 2 byte0=%x, byte1=%x, byte2=%x, byte3=%x, \n", stCmdqDataSeq.byte0, stCmdqDataSeq.byte1, stCmdqDataSeq.byte2, stCmdqDataSeq.byte3);
        W4BYTE(DSI_CMDQ_BASE +(u8CmdStIdx*2),  AS_UINT32(&stCmdqDataSeq));
        //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, "Case 2   u8CmdStIdx =%d, CMDQ=%x \n", u8CmdStIdx, R4BYTE(DSI_CMDQ_BASE +(u8CmdStIdx*2)));
    }
    else if((u8Count%4) == 1)
    {
         if(!bFirstCmd)
         {
            stCmdqDataSeq.byte0 = u8Cmd;
            bFirstCmd = 1;
        }
        else
        {
            stCmdqDataSeq.byte0=*pu8ParamList;
            pu8ParamList+=1;
        }
        //pstCmdqDataSeq[u8Index]=stCmdqDataSeq ;
       // u8CmdStIdx++;
        //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, "byte0=%x, byte1=%x, byte2=%x, byte3=%x, \n", stCmdqDataSeq.byte0, stCmdqDataSeq.byte1, stCmdqDataSeq.byte2, stCmdqDataSeq.byte3);
        W4BYTE(DSI_CMDQ_BASE +(u8CmdStIdx*2),  AS_UINT32(&stCmdqDataSeq));
       //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, "Case3 u8CmdStIdx =%d, CMDQ=%x \n", u8CmdStIdx, R4BYTE(DSI_CMDQ_BASE +(u8CmdStIdx*2)));
    }

    u8CMDQNum += 1;
    //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, " u8CMDQNum =%d \n", u8CMDQNum);
    //if(u8CmdStIdx !=u8CMDQNum )
    //{
    //   HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, " Wrong CMDQNum\n ");
    //}
    //clear u8Cmd done flag first
    #if 0
        {
    UartSendTrace("------------------------------------\n");
    UartSendTrace("Bank Addr : 0x1A2200     \n");
    UartSendTrace("------------------------------------\n");
    u16Offset =0;

    UartSendTrace("        ", 0);
    for(i=0 ; i < 8; i++) {
        UartSendTrace("%08X ",i*4);
    }
    UartSendTrace("\n");
    for(i=0 ; i < 16; i++) {
        UartSendTrace("%03X     ", u16Offset<<1);
        for(j=0; j<8 ; j++) {
            UartSendTrace("%08X ", Hal_MIPI_DSI_RegisterRead(0x1A2200+u16Offset));
            u16Offset +=2;
        }
        UartSendTrace("  \n");
    }
    }
#endif
    WriteLongRegBit(DSI_REG_BASE +(REG_DSI_INTSTA>>1), 0, DSI_CMD_DONE_INT_FLAG);

    W4BYTEMSK(DSI_REG_BASE +(REG_DSI_CMDQ_CON>>1), u8CMDQNum, DSI_CMDQ_SIZE);

    WriteLongRegBit(DSI_REG_BASE +(REG_DSI_START>>1), 0, DSI_DSI_START); //clear first
    WriteLongRegBit(DSI_REG_BASE +(REG_DSI_START>>1), 1, DSI_DSI_START);

    do
    {
        u32DsiIntSta =R4BYTE(DSI_REG_BASE + (REG_DSI_INTSTA>>1));
        //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_PACKET, " u32DsiIntSta = %x \n", u32DsiIntSta);
        u32TimeOut --;
    } while( !(u32DsiIntSta & DSI_CMD_DONE_INT_FLAG) && u32TimeOut);
    WriteLongRegBit(DSI_REG_BASE +(REG_DSI_INTSTA>>1), 0, DSI_CMD_DONE_INT_FLAG);

    if(u32TimeOut == 0)
    {
      PNL_ERR("%s %d, CMD Done Time Out, DsiIntSta=%x\n", __FUNCTION__, __LINE__, u32DsiIntSta);
    }

    //_HalIsEngineBusy();
    return 1;
}


bool HalPnlSetMipiDsiPhyTimConfig(HalPnlMipiDsiConfig_t *pstMipiDsiCfg)
{

    HalPnlMipiDsiPhyTimCon0Reg_t stTimCon0;
    //HalPnlMipiDsiPhyTimCon0Reg_t *pstTimCon0;
    HalPnlMipiDsiPhyTimCon1Reg_t stTimCon1;
    //HalPnlMipiDsiPhyTimCon1Reg_t *pstTimCon1;
    HalPnlMipiDsiPhyTimCon2Reg_t stTimCon2;
    //HalPnlMipiDsiPhyTimCon2Reg_t *pstTimCon2;

    HalPnlMipiDsiPhyTimCon3Reg_t stTimCon3;
    //HalPnlMipiDsiPhyTimCon3Reg_t *pstTimCon3;
    u32 lane_no = pstMipiDsiCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_4 ? 4 :
                  pstMipiDsiCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_3 ? 3 :
                  pstMipiDsiCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_2 ? 2 :
                                                                          1;

    //unsigned int div2_real;
    u32 cycle_time;
    u32 ui;
    //u32 hs_trail_m, hs_trail_n;
    //u32 div1 = 0;
    //u32 div2 = 0;
    //u32 fbk_sel = 0;
    //u32 pre_div = 0;
    //u32 fbk_div = 0;

#if 0
    //	div2_real=div2 ? div2*0x02 : 0x1;
    cycle_time = (8 * 1000 * div2 * div1 * pre_div)/ (26 * (fbk_div+0x01) * fbk_sel * 2);
    ui = (1000 * div2 * div1 * pre_div)/ (26 * (fbk_div+0x01) * fbk_sel * 2) + 1;

    //DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_PHY_TIMCONFIG, Cycle Time = %d(ns), Unit Interval = %d(ns). div1 = %d, div2 = %d, fbk_div = %d, lane# = %d \n", cycle_time, ui, div1, div2, fbk_div, lane_no);

#define NS_TO_CYCLE(n, c)	((n) / c + (( (n) % c) ? 1 : 0))

    hs_trail_m=lane_no;
    hs_trail_n= (pstMipiDsiCfg->dsi.u8HsTrail == 0) ? NS_TO_CYCLE(((lane_no * 4 * ui) + 60), cycle_time) : pstMipiDsiCfg->dsi.u8HsTrail;

    // +3 is recommended from designer becauase of HW latency
    stTimCon0.u8HsTrail	= ((hs_trail_m > hs_trail_n) ? hs_trail_m : hs_trail_n) + 0x0a;

    stTimCon0.u8HsPrpr 	= (pstMipiDsiCfg->dsi.u8HsPrpr == 0) ? NS_TO_CYCLE((60 + 5 * ui), cycle_time) : pstMipiDsiCfg->dsi.u8HsPrpr;
    // u8HsPrpr can't be 1.
    if (stTimCon0.u8HsPrpr == 0)
    	stTimCon0.u8HsPrpr = 1;

    stTimCon0.u8HsZero 	= (pstMipiDsiCfg->dsi.u8HsZero == 0) ? NS_TO_CYCLE((0xC8 + 0x0a * ui - stTimCon0.u8HsPrpr * cycle_time), cycle_time) : pstMipiDsiCfg->dsi.u8HsZero;

    stTimCon0.u8Lpx 		= (pstMipiDsiCfg->dsi.u8Lpx == 0) ? NS_TO_CYCLE(65, cycle_time) : pstMipiDsiCfg->dsi.u8Lpx;
    if(stTimCon0.u8Lpx == 0) stTimCon0.u8Lpx = 1;
    //	stTimCon1.TA_SACK 	= (pstMipiDsiCfg->dsi.TA_SACK == 0) ? 1 : pstMipiDsiCfg->dsi.TA_SACK;
    stTimCon1.u8TaGet 		= (pstMipiDsiCfg->dsi.u8TaGet == 0) ? (5 * stTimCon0.u8Lpx) : pstMipiDsiCfg->dsi.u8TaGet;
    stTimCon1.u8TaSure 	= (pstMipiDsiCfg->dsi.u8TaSure == 0) ? (3 * stTimCon0.u8Lpx / 2) : pstMipiDsiCfg->dsi.u8TaSure;
    stTimCon1.u8TaGo 		= (pstMipiDsiCfg->dsi.u8TaGo == 0) ? (4 * stTimCon0.u8Lpx) : pstMipiDsiCfg->dsi.u8TaGo;
    stTimCon1.u8DaHsExit  = (pstMipiDsiCfg->dsi.u8DaHsExit == 0) ? (2 * stTimCon0.u8Lpx) : pstMipiDsiCfg->dsi.u8DaHsExit;

    stTimCon2.u8ClkTrail 	= ((pstMipiDsiCfg->dsi.u8ClkTrail == 0) ? NS_TO_CYCLE(0x64, cycle_time) : pstMipiDsiCfg->dsi.u8ClkTrail) + 0x0a;
    // u8ClkTrail can't be 1.
    if (stTimCon2.u8ClkTrail < 2)
        stTimCon2.u8ClkTrail = 2;

    //  imcon2.LPX_WAIT 	= (pstMipiDsiCfg->dsi.LPX_WAIT == 0) ? 1 : pstMipiDsiCfg->dsi.LPX_WAIT;
    stTimCon2.u8ContDet 	= pstMipiDsiCfg->dsi.u8ContDet;

    stTimCon3.u8ClkHsPrpr	= (pstMipiDsiCfg->dsi.u8ClkHsPrpr == 0) ? NS_TO_CYCLE(0x40, cycle_time) : pstMipiDsiCfg->dsi.u8ClkHsPrpr;
    if(stTimCon3.u8ClkHsPrpr == 0) stTimCon3.u8ClkHsPrpr = 1;

    stTimCon2.u8ClkZero	= (pstMipiDsiCfg->dsi.u8ClkZero == 0) ? NS_TO_CYCLE(0x190 - stTimCon3.u8ClkHsPrpr * cycle_time, cycle_time) : pstMipiDsiCfg->dsi.u8ClkZero;

    stTimCon3.u8ClkHsExit= (pstMipiDsiCfg->dsi.u8ClkHsExit == 0) ? (2 * stTimCon0.u8Lpx) : pstMipiDsiCfg->dsi.u8ClkHsExit;

    stTimCon3.u8ClkHsPost= (pstMipiDsiCfg->dsi.u8ClkHsPost == 0) ? NS_TO_CYCLE((80 + 52 * ui), cycle_time) : pstMipiDsiCfg->dsi.u8ClkHsPost;

    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_PHY_TIMCONFIG, u8HsTrail = %d, u8HsZero = %d, u8HsPrpr = %d, u8Lpx = %d, u8TaGet = %d, u8TaSure = %d, u8TaGo = %d, u8ClkTrail = %d, u8ClkZero = %d, u8ClkHsPrpr = %d \n", \
        stTimCon0.u8HsTrail, stTimCon0.u8HsZero, stTimCon0.u8HsPrpr, stTimCon0.u8Lpx, stTimCon1.u8TaGet, stTimCon1.u8TaSure, stTimCon1.u8TaGo, stTimCon2.u8ClkTrail, stTimCon2.u8ClkZero, stTimCon3.u8ClkHsPrpr);

    //DSI_REG->DSI_PHY_TIMECON0=stTimCon0;
    //DSI_REG->DSI_PHY_TIMECON1=stTimCon1;
    //DSI_REG->DSI_PHY_TIMECON2=stTimCon2;
    //DSI_REG->DSI_PHY_TIMECON3=stTimCon3;
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON0,AS_UINT32(&stTimCon0));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON1,AS_UINT32(&stTimCon1));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON2,AS_UINT32(&stTimCon2));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON3,AS_UINT32(&stTimCon3));
    dsi_cycle_time = cycle_time;
#else
        //	div2_real=div2 ? div2*0x02 : 0x1;

    #if 0
    cycle_time = (8 * 1000 * div2 * div1 * pre_div)/ (26 * (fbk_div+0x01) * fbk_sel * 2);
    ui = (1000 * div2 * div1 * pre_div)/ (26 * (fbk_div+0x01) * fbk_sel * 2) + 1;
    #else

    #endif

    //DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_PHY_TIMCONFIG, Cycle Time = %d(ns), Unit Interval = %d(ns). div1 = %d, div2 = %d, fbk_div = %d, lane# = %d \n", cycle_time, ui, div1, div2, fbk_div, lane_no);

#define NS_TO_CYCLE(n, c)	((n) / c + (( (n) % c) ? 1 : 0))

#if 0  //TBD
    hs_trail_m=lane_no;
    hs_trail_n= (pstMipiDsiCfg->u8HsTrail == 0) ? NS_TO_CYCLE(((lane_no * 4 * ui) + 60), cycle_time) : pstMipiDsiCfg->u8HsTrail;

    // +3 is recommended from designer becauase of HW latency
    stTimCon0.u8HsTrail	= ((hs_trail_m > hs_trail_n) ? hs_trail_m : hs_trail_n) + 0x0a;
#else
    stTimCon0.u8HsTrail    =(pstMipiDsiCfg->u8HsTrail == 0) ? NS_TO_CYCLE(((lane_no * 4 * ui) + 60), cycle_time) : pstMipiDsiCfg->u8HsTrail;
#endif

    stTimCon0.u8HsPrpr     = (pstMipiDsiCfg->u8HsPrpr == 0) ? NS_TO_CYCLE((60 + 5 * ui), cycle_time) : pstMipiDsiCfg->u8HsPrpr;

    // u8HsPrpr can't be 1.
    if (stTimCon0.u8HsPrpr == 0)
    {
        stTimCon0.u8HsPrpr = 1;
    }

    stTimCon0.u8HsZero 	= (pstMipiDsiCfg->u8HsZero == 0) ? NS_TO_CYCLE((0xC8 + 0x0a * ui - stTimCon0.u8HsPrpr * cycle_time), cycle_time) : pstMipiDsiCfg->u8HsZero;

    stTimCon0.u8Lpx     = (pstMipiDsiCfg->u8Lpx == 0) ? NS_TO_CYCLE(65, cycle_time) : pstMipiDsiCfg->u8Lpx;
    if(stTimCon0.u8Lpx == 0) stTimCon0.u8Lpx = 1;

    //stTimCon1.TA_SACK 	= (pstMipiDsiCfg->dsi.TA_SACK == 0) ? 1 : pstMipiDsiCfg->dsi.TA_SACK;
    stTimCon1.u8TaGet      = (pstMipiDsiCfg->u8TaGet == 0) ? (5 * stTimCon0.u8Lpx) : pstMipiDsiCfg->u8TaGet;
    stTimCon1.u8TaSure     = (pstMipiDsiCfg->u8TaSure == 0) ? (3 * stTimCon0.u8Lpx / 2) : pstMipiDsiCfg->u8TaSure;
    stTimCon1.u8TaGo       = (pstMipiDsiCfg->u8TaGo == 0) ? (4 * stTimCon0.u8Lpx) : pstMipiDsiCfg->u8TaGo;
    stTimCon1.u8DaHsExit  = (pstMipiDsiCfg->u8DaHsExit == 0) ? (2 * stTimCon0.u8Lpx) : pstMipiDsiCfg->u8DaHsExit;

    stTimCon2.u8ClkTrail 	= ((pstMipiDsiCfg->u8ClkTrail == 0) ? NS_TO_CYCLE(0x64, cycle_time) : pstMipiDsiCfg->u8ClkTrail) + 0x0a;
    // u8ClkTrail can't be 1.
    if (stTimCon2.u8ClkTrail < 2)
    {
        stTimCon2.u8ClkTrail = 2;
    }

    //  imcon2.LPX_WAIT 	= (pstMipiDsiCfg->dsi.LPX_WAIT == 0) ? 1 : pstMipiDsiCfg->dsi.LPX_WAIT;
    stTimCon2.u8ContDet 	= pstMipiDsiCfg->u8ContDet;

    stTimCon3.u8ClkHsPrpr	= (pstMipiDsiCfg->u8ClkHsPrpr == 0) ? NS_TO_CYCLE(0x40, cycle_time) : pstMipiDsiCfg->u8ClkHsPrpr;

    if(stTimCon3.u8ClkHsPrpr == 0)
    {
        stTimCon3.u8ClkHsPrpr = 1;
    }

    stTimCon2.u8ClkZero	= (pstMipiDsiCfg->u8ClkZero == 0) ? NS_TO_CYCLE(0x190 - stTimCon3.u8ClkHsPrpr * cycle_time, cycle_time) : pstMipiDsiCfg->u8ClkZero;
    stTimCon2.RSV8 =0x01;
    stTimCon3.u8ClkHsExit= (pstMipiDsiCfg->u8ClkHsExit == 0) ? (2 * stTimCon0.u8Lpx) : pstMipiDsiCfg->u8ClkHsExit;

    stTimCon3.u8ClkHsPost= (pstMipiDsiCfg->u8ClkHsPost == 0) ? NS_TO_CYCLE((80 + 52 * ui), cycle_time) : pstMipiDsiCfg->u8ClkHsPost;

    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, HsTrail=%d, HsZero=%d, HsPrpr=%d, Lpx=%d, TaGet=%d, TaSure=%d, TaGo=%d, ClkTrail=%d, u8ClkZero=%d, ClkHsPrpr=%d \n", \
        __FUNCTION__, __LINE__,
        stTimCon0.u8HsTrail, stTimCon0.u8HsZero, stTimCon0.u8HsPrpr,
        stTimCon0.u8Lpx, stTimCon1.u8TaGet, stTimCon1.u8TaSure, stTimCon1.u8TaGo,
        stTimCon2.u8ClkTrail, stTimCon2.u8ClkZero, stTimCon3.u8ClkHsPrpr);

    W4BYTE(DSI_REG_BASE + (REG_DSI_PHY_TIMCON0>>1)  ,  AS_UINT32(&stTimCon0));
    W4BYTE(DSI_REG_BASE + (REG_DSI_PHY_TIMCON1>>1)  ,  AS_UINT32(&stTimCon1));

    //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_TIMING, "[MIPI DSI] %s,  %d,    %x \n", __FUNCTION__, __LINE__, R4BYTE(DSI_REG_BASE+(REG_DSI_PHY_TIMCON2>>1)));
    //HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_TIMING, "AS_UINT32(&stTimCon2) =%x \n", AS_UINT32(&stTimCon2) );
    //*pstTimCon2=(PDSI_PHY_TIMCON2_REG) (CHIP_BASE(u32, DSI_REG_BASE_2, REG_DSI_PHY_TIMCON2, 0));
    W4BYTE(DSI_REG_BASE + (REG_DSI_PHY_TIMCON2>>1)  ,  AS_UINT32(&stTimCon2));
    W4BYTE(DSI_REG_BASE + (REG_DSI_PHY_TIMCON3>>1)  ,  AS_UINT32(&stTimCon3));

#if 0
    W4BYTE(DSI_REG_BASE +(REG_DSI_PHY_TIMCON0>>1), (u32) stTimCon0);
    W4BYTE(DSI_REG_BASE +(REG_DSI_PHY_TIMCON1>>1), (u32) stTimCon1);
    W4BYTE(DSI_REG_BASE +(REG_DSI_PHY_TIMCON2>>1), (u32) stTimCon2);
    W4BYTE(DSI_REG_BASE +(REG_DSI_PHY_TIMCON3>>1), (u32) stTimCon3);
#endif
    //dsi_cycle_time = cycle_time;

#endif
    return 1;
}

bool HalPnlSetMipiDsiVdoTimingConfig(HalPnlMipiDsiConfig_t *pstMipiDsiCfg)
{
    //unsigned int line_byte;
    u32 horizontal_sync_active_byte = 0;
    u32 horizontal_backporch_byte = 0;
    u32 horizontal_frontporch_byte = 0;
    u32 horizontal_bllp_byte;
    u32 dsiTmpBufBpp;
    u32 u32LaneNum = pstMipiDsiCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_4 ? 4 :
                     pstMipiDsiCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_3 ? 3 :
                     pstMipiDsiCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_1 ? 2 :
                                                                             1 ;
    if(pstMipiDsiCfg->enFormat == E_HAL_PNL_MIPI_DSI_RGB565)
    {
        dsiTmpBufBpp = 16;
    }
    else if( (pstMipiDsiCfg->enFormat == E_HAL_PNL_MIPI_DSI_RGB888 ||
              pstMipiDsiCfg->enFormat == E_HAL_PNL_MIPI_DSI_LOOSELY_RGB666))
    {
        dsiTmpBufBpp = 24;
    }
    else if( (pstMipiDsiCfg->enFormat == E_HAL_PNL_MIPI_DSI_RGB666) )
    {
        dsiTmpBufBpp = 18;
    }

    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, Bpp=%d, HTotal=%d, HPW=%d, HBP= %d, HFP=%d, BLLP=%d\n",
        __FUNCTION__, __LINE__, dsiTmpBufBpp,
        pstMipiDsiCfg->u32HActive + pstMipiDsiCfg->u32HPW + pstMipiDsiCfg->u32HBP + pstMipiDsiCfg->u32HFP + pstMipiDsiCfg->u32BLLP,
        pstMipiDsiCfg->u32HPW, pstMipiDsiCfg->u32HBP, pstMipiDsiCfg->u32HFP, pstMipiDsiCfg->u32BLLP);

    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, VTotal=%d, VPW=%d, VBP=%d, VFP=%d\n",
        __FUNCTION__, __LINE__,
        pstMipiDsiCfg->u32VActive + pstMipiDsiCfg->u32VPW + pstMipiDsiCfg->u32VBP + pstMipiDsiCfg->u32VFP,
        pstMipiDsiCfg->u32VPW,pstMipiDsiCfg->u32VBP, pstMipiDsiCfg->u32VFP);


    W4BYTE(DSI_REG_BASE +(REG_DSI_VACT_NL>>1), pstMipiDsiCfg->u32VActive);
    W4BYTE(DSI_REG_BASE +(REG_DSI_VSA_NL>>1),  pstMipiDsiCfg->u32VPW);
    W4BYTE(DSI_REG_BASE +(REG_DSI_VBP_NL>>1),  pstMipiDsiCfg->u32VBP);
    W4BYTE(DSI_REG_BASE +(REG_DSI_VFP_NL>>1),  pstMipiDsiCfg->u32VFP);

    if (pstMipiDsiCfg->enCtrl == E_HAL_PNL_MIPI_DSI_SYNC_EVENT || pstMipiDsiCfg->enCtrl == E_HAL_PNL_MIPI_DSI_BURST_MODE )
    {
        //ASSERT((pstMipiDsiCfg->dsi.horizontal_backporch + pstMipiDsiCfg->dsi.horizontal_sync_active) * dsiTmpBufBpp> 9);
        horizontal_backporch_byte    =    ((pstMipiDsiCfg->u32HBP+ pstMipiDsiCfg->u32HPW)* dsiTmpBufBpp - 10);
        PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, horizontal_backporch_byte=%d\n", __FUNCTION__, __LINE__, horizontal_backporch_byte);
    }
    else
    {
    //ASSERT(pstMipiDsiCfg->dsi.horizontal_sync_active * dsiTmpBufBpp > 9);
    if( ((pstMipiDsiCfg->u32HPW * dsiTmpBufBpp)/8) < 9)
    {
        PNL_ERR("%s %d, horizontal_sync_active byte count less than 9\n", __FUNCTION__, __LINE__);
    }
    horizontal_sync_active_byte = (((pstMipiDsiCfg->u32HPW * dsiTmpBufBpp)/8) > 9) ? ( (pstMipiDsiCfg->u32HPW* dsiTmpBufBpp)/8 - 10):0;
    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, horizontal_sync_active_byte = %d \n", __FUNCTION__, __LINE__, horizontal_sync_active_byte);

    //ASSERT(pstMipiDsiCfg->dsi.horizontal_backporch * dsiTmpBufBpp > 9);
    if( ((pstMipiDsiCfg->u32HBP * dsiTmpBufBpp)/8) < 9)
    {
            PNL_ERR("%s %d, horizontal_backporch byte count less than 9 \n", __FUNCTION__, __LINE__);
            PNL_ERR("%s %d, horizontal_backporch_byte = %d \n",  __FUNCTION__, __LINE__, (pstMipiDsiCfg->u32HBP * dsiTmpBufBpp)/8);
        }
    horizontal_backporch_byte =    (((pstMipiDsiCfg->u32HBP * dsiTmpBufBpp)/8) > 9 )? ( (pstMipiDsiCfg->u32HBP * dsiTmpBufBpp)/8 - 10):0;
    }

    //ASSERT(pstMipiDsiCfg->dsi.horizontal_frontporch * dsiTmpBufBpp > 11);
    horizontal_frontporch_byte        =    ((pstMipiDsiCfg->u32HFP* dsiTmpBufBpp)/8 > 12) ?((pstMipiDsiCfg->u32HFP* dsiTmpBufBpp)/8 - 12) : 0;
    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d HFP = %d \n",  __FUNCTION__, __LINE__, pstMipiDsiCfg->u32HFP);
    horizontal_bllp_byte = ((pstMipiDsiCfg->u32BLLP* dsiTmpBufBpp)/8 >6)?    ((pstMipiDsiCfg->u32BLLP* dsiTmpBufBpp)/8 -6):0;

    //ASSERT(pstMipiDsiCfg->dsi.horizontal_frontporch * dsiTmpBufBpp > ((300/dsi_cycle_time) * pstMipiDsiCfg->dsi.LANE_NUM));
    //horizontal_frontporch_byte -= ((300/dsi_cycle_time) * pstMipiDsiCfg->dsi.LANE_NUM);

    horizontal_frontporch_byte -= ( (pstMipiDsiCfg->u8ClkHsExit + pstMipiDsiCfg->u8Lpx + pstMipiDsiCfg->u8HsPrpr + pstMipiDsiCfg->u8HsZero)* u32LaneNum);

    W4BYTE(DSI_REG_BASE +(REG_DSI_HSA_WC>>1),  ALIGN_TO(horizontal_sync_active_byte,4));
    W4BYTE(DSI_REG_BASE +(REG_DSI_HBP_WC>>1),  ALIGN_TO(horizontal_backporch_byte,4));
    W4BYTE(DSI_REG_BASE +(REG_DSI_HFP_WC>>1),  ALIGN_TO(horizontal_frontporch_byte,4));
    W4BYTE(DSI_REG_BASE +(REG_DSI_BLLP_WC>>1), ALIGN_TO(horizontal_bllp_byte,4));

    W4BYTEMSK(DSI_REG_BASE +(REG_DSI_PSCON>>1), ALIGN_TO((dsiTmpBufBpp*pstMipiDsiCfg->u32HActive)/8,4) ,DSI_TDSI_PS_WC);

    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, PS_WC = 0x%x \n",  __FUNCTION__, __LINE__, R4BYTE(DSI_REG_BASE +(REG_DSI_PSCON>>1)) & 0x1FFF);
    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, HSA_WC= 0x%x \n",  __FUNCTION__, __LINE__, R4BYTE(DSI_REG_BASE +(REG_DSI_HSA_WC>>1)));
    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, HBP_WC= 0x%x \n",  __FUNCTION__, __LINE__, R4BYTE(DSI_REG_BASE +(REG_DSI_HBP_WC>>1)));
    PNL_DBG(PNL_DBG_LEVEL_HAL, "%s %d, HFP_WC= 0x%x \n",  __FUNCTION__, __LINE__, R4BYTE(DSI_REG_BASE +(REG_DSI_HFP_WC>>1)));

    if(pstMipiDsiCfg->enFormat == E_HAL_PNL_MIPI_DSI_RGB565)
    {
        W4BYTEMSK(DSI_REG_BASE +(REG_DSI_PSCON>>1), 0<<16 ,DSI_DSI_PS_SEL);
    }
    else if (pstMipiDsiCfg->enFormat == E_HAL_PNL_MIPI_DSI_RGB666)
    {
        W4BYTEMSK(DSI_REG_BASE +(REG_DSI_PSCON>>1), 1<<16 ,DSI_DSI_PS_SEL);
    }
    else if (pstMipiDsiCfg->enFormat == E_HAL_PNL_MIPI_DSI_LOOSELY_RGB666)
    {
        W4BYTEMSK(DSI_REG_BASE +(REG_DSI_PSCON>>1), 2<<16 ,DSI_DSI_PS_SEL);
    }
    else if (pstMipiDsiCfg->enFormat == E_HAL_PNL_MIPI_DSI_RGB888)
    {
        W4BYTEMSK(DSI_REG_BASE +(REG_DSI_PSCON>>1), 3<<16 ,DSI_DSI_PS_SEL);
    }
 #if 0
    #define LINE_PERIOD_US        (8 * line_byte * _dsiContext.bit_time_ns / 1000)

    OUTREG32(&DSI_REG->DSI_VSA_NL, pstMipiDsiCfg->dsi.vertical_sync_active);
    OUTREG32(&DSI_REG->DSI_VBP_NL, pstMipiDsiCfg->dsi.vertical_backporch);
    OUTREG32(&DSI_REG->DSI_VFP_NL, pstMipiDsiCfg->dsi.vertical_frontporch);
    OUTREG32(&DSI_REG->DSI_VACT_NL, pstMipiDsiCfg->dsi.vertical_active_line);

    line_byte                =    (pstMipiDsiCfg->dsi.horizontal_sync_active \
                        + pstMipiDsiCfg->dsi.horizontal_backporch \
                        + pstMipiDsiCfg->dsi.horizontal_frontporch \
                        + pstMipiDsiCfg->dsi.horizontal_active_pixel) * dsiTmpBufBpp;

    if (pstMipiDsiCfg->dsi.mode == SYNC_EVENT_VDO_MODE || pstMipiDsiCfg->dsi.mode == BURST_VDO_MODE ){
    ASSERT((pstMipiDsiCfg->dsi.horizontal_backporch + pstMipiDsiCfg->dsi.horizontal_sync_active) * dsiTmpBufBpp> 9);
    horizontal_backporch_byte    =    ((pstMipiDsiCfg->dsi.horizontal_backporch + pstMipiDsiCfg->dsi.horizontal_sync_active)* dsiTmpBufBpp - 10);
    }
    else{
    ASSERT(pstMipiDsiCfg->dsi.horizontal_sync_active * dsiTmpBufBpp > 9);
    horizontal_sync_active_byte     =    (pstMipiDsiCfg->dsi.horizontal_sync_active * dsiTmpBufBpp - 10);

    ASSERT(pstMipiDsiCfg->dsi.horizontal_backporch * dsiTmpBufBpp > 9);
    horizontal_backporch_byte    =    (pstMipiDsiCfg->dsi.horizontal_backporch * dsiTmpBufBpp - 10);
    }

    ASSERT(pstMipiDsiCfg->dsi.horizontal_frontporch * dsiTmpBufBpp > 11);
    horizontal_frontporch_byte        =    (pstMipiDsiCfg->dsi.horizontal_frontporch * dsiTmpBufBpp - 12);
    horizontal_bllp_byte        =    (pstMipiDsiCfg->dsi.horizontal_bllp * dsiTmpBufBpp);
//    ASSERT(pstMipiDsiCfg->dsi.horizontal_frontporch * dsiTmpBufBpp > ((300/dsi_cycle_time) * pstMipiDsiCfg->dsi.LANE_NUM));
//    horizontal_frontporch_byte -= ((300/dsi_cycle_time) * pstMipiDsiCfg->dsi.LANE_NUM);

    OUTREG32(&DSI_REG->DSI_HSA_WC, ALIGN_TO((horizontal_sync_active_byte), 4));
    OUTREG32(&DSI_REG->DSI_HBP_WC, ALIGN_TO((horizontal_backporch_byte), 4));
    OUTREG32(&DSI_REG->DSI_HFP_WC, ALIGN_TO((horizontal_frontporch_byte), 4));
    OUTREG32(&DSI_REG->DSI_BLLP_WC, ALIGN_TO((horizontal_bllp_byte), 4));

    _dsiContext.vfp_period_us     = LINE_PERIOD_US * pstMipiDsiCfg->dsi.vertical_frontporch / 1000;
    _dsiContext.vsa_vs_period_us    = LINE_PERIOD_US * 1 / 1000;
    _dsiContext.vsa_hs_period_us    = LINE_PERIOD_US * (pstMipiDsiCfg->dsi.vertical_sync_active - 2) / 1000;
    _dsiContext.vsa_ve_period_us    = LINE_PERIOD_US * 1 / 1000;
    _dsiContext.vbp_period_us    = LINE_PERIOD_US * pstMipiDsiCfg->dsi.vertical_backporch / 1000;


    HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_TIMING, "DSI [DISP] kernel - video timing, mode = %d \n", pstMipiDsiCfg->dsi.mode);
    HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_TIMING, "DSI [DISP] kernel - VSA : %d %d(us)\n", DSI_REG->DSI_VSA_NL, (_dsiContext.vsa_vs_period_us+_dsiContext.vsa_hs_period_us+_dsiContext.vsa_ve_period_us));
    HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_TIMING, "DSI [DISP] kernel - VBP : %d %d(us)\n", DSI_REG->DSI_VBP_NL, _dsiContext.vbp_period_us);
    HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_TIMING, "DSI [DISP] kernel - VFP : %d %d(us)\n", DSI_REG->DSI_VFP_NL, _dsiContext.vfp_period_us);
    HAL_MIPI_DSI_DBG(HAL_MIPI_DSI_DBG_LEVEL_TIMING, "DSI [DISP] kernel - VACT: %d \n", DSI_REG->DSI_VACT_NL);
#endif
    return 1;
}


void HalPnlSetMipiDsiMisc(void)
{
    W2BYTE(REG_SC_BK0F_2A_L, 0x0084);
    W2BYTE(REG_SC_BK0F_2C_L, 0x0001);

    W2BYTE(REG_SC_BK33_01_L, 0x0001);
    W2BYTE(REG_SC_BK33_23_L, 0x0010);
}

#undef __HAL_PNL_C__
