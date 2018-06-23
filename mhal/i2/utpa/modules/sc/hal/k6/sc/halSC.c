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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    drvSC.c
/// @brief  SmartCard Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#else
#include <string.h>
#endif
#include "MsCommon.h"
#include "drvSC.h"
#include "drvSYS.h"

// Internal Definition
#include "regSC.h"
#include "halSC.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define SC_DBG_ENABLE               2UL
#define SC_DEV_NUM_MAX				2UL
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#if SC_DBG_ENABLE
#define SC_DBG(fmt, args...)        printf("[DEV][SMART][%06d] " fmt, __LINE__, ##args)
#define SC_ERR(fmt, args...)        printf("[DEV][SMART][%06d] " fmt, __LINE__, ##args)
#else
#define SC_DBG(fmt, args...)        {}
#define SC_ERR(fmt, args...)        printf("[DEV][SMART][%06d] " fmt, __LINE__, ##args)
#endif

#define SC_TOP_CKG_UART1_CLK        TOP_CKG_UART1_CLK_144M
#define SC_TOP_CKG_UART2_CLK        TOP_CKG_UART2_CLK_144M

#define SC_ACTIVE_VCC_CNT           0x01
#define SC_ACTIVE_IO_CNT            0x10
#define SC_ACTIVE_CLK_CNT           0x13
#define SC_ACTIVE_RST_CNT           0x16

#define SC_DEACTIVE_RST_CNT         0x03
#define SC_DEACTIVE_CLK_CNT         0x06
#define SC_DEACTIVE_IO_CNT          0x09
#define SC_DEACTIVE_VCC_CNT         0x0C

#define K6U02         (MDrv_SYS_GetChipRev() >= 0x1)  // This feature/behavior is supported after K6 U02

// Setting
//#define SC_EVENT()                  MsOS_SetEvent(s32CardEventId, SC_EVENT_CARD)
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

MS_VIRT                              _regSCBase[SC_DEV_NUM_MAX];
MS_VIRT                              _regSCHWBase = 0;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static MS_U8 _HAL_SC_GetLsr(MS_U8 u8SCID)
{
    MS_U8 u8Data;

    // Since some bitfield of LSR is "read clear", and we found these bitfield data will be cleared before RIU read back for some chips (ex: kirin)
    // So we need to enable LCR DLAB to make these bitfield can not be cleared based on HW design
    SC_OR(u8SCID,UART_LCR, UART_LCR_DLAB);
    u8Data = SC_READ(u8SCID,UART_LSR);
    SC_AND(u8SCID,UART_LCR, ~(UART_LCR_DLAB));

    return u8Data;
}

static MS_U8 _HAL_SC_ClearLsr(MS_U8 u8SCID)
{
    MS_U8 u8Data;

    u8Data = SC_READ(u8SCID,UART_LSR);

    return u8Data;
}

static MS_U32 _HAL_SC_GetTopClkNum(MS_U8 u8SCID)
{
    MS_U16 u16ClkCtrl;
    MS_U32 u32ClkNum;

    if (u8SCID == 0)
    {
        u16ClkCtrl = SC_TOP_CKG_UART1_CLK;
        switch(u16ClkCtrl)
        {
            case TOP_CKG_UART1_CLK_108M:
                u32ClkNum = 108000000;
                break;

            case TOP_CKG_UART1_CLK_123M:
                u32ClkNum = 123000000;
                break;

            case TOP_CKG_UART1_CLK_144M:
                u32ClkNum = 144000000;
                break;

            case TOP_CKG_UART1_CLK_160M:
                u32ClkNum = 160000000;
                break;

            case TOP_CKG_UART1_CLK_170M:
                u32ClkNum = 170000000;
                break;

            default:
                u32ClkNum = 0;
                break;
        }
    }
    else
    {
        u16ClkCtrl = SC_TOP_CKG_UART2_CLK;
        switch(u16ClkCtrl)
        {
            case TOP_CKG_UART2_CLK_108M:
                u32ClkNum = 108000000;
                break;

            case TOP_CKG_UART2_CLK_123M:
                u32ClkNum = 123000000;
                break;

            case TOP_CKG_UART2_CLK_144M:
                u32ClkNum = 144000000;
                break;

            case TOP_CKG_UART2_CLK_160M:
                u32ClkNum = 160000000;
                break;

            case TOP_CKG_UART2_CLK_170M:
                u32ClkNum = 170000000;
                break;

            default:
                u32ClkNum = 0;
                break;
        }
    }

    return u32ClkNum;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

void HAL_SC_RegMap(MS_VIRT u32RegBase)
{
    _regSCBase[0] = u32RegBase;
}

void HAL_SC1_RegMap(MS_VIRT u32RegBase)
{
    _regSCBase[1] = u32RegBase;
}

void HAL_SC_HW_RegMap(MS_VIRT u32RegBase)
{
    _regSCHWBase = u32RegBase;
}

void HAL_SC_SetClk(MS_U8 u8SCID, HAL_SC_CLK_CTRL eClk)
{
    MS_U16              reg;

    reg = HW_READ(REG_TOP_CKG_SM_CA);

    if (u8SCID == 0)
    {
        reg &= (~TOP_CKG_SM_CA0_CLK_MASK);
        HW_WRITE(REG_TOP_CKG_SM_CACLK_EXT,HW_READ(REG_TOP_CKG_SM_CACLK_EXT)&0x7FFF);
        switch (eClk)
        {
            case E_HAL_SC_CLK_3M :
                reg |= TOP_CKG_SM_CA0_CLK_27M_D8;
                break;
            case E_HAL_SC_CLK_4P5M :
                reg |= TOP_CKG_SM_CA0_CLK_27M_D6;
                break;
            case E_HAL_SC_CLK_6M :
                reg |= TOP_CKG_SM_CA0_CLK_27M_D4;
                break;
            case E_HAL_SC_CLK_13M :
                reg |= TOP_CKG_SM_CA0_CLK_27M_D2;
                break;
            case E_HAL_SC_CLK_4M:
                HW_WRITE(REG_TOP_CKG_SM_CACLK_N,0x6C);
                HW_WRITE(REG_TOP_CKG_SM_CACLK_M,HW_READ(REG_TOP_CKG_SM_CACLK_M)& ~0x00FF);
                HW_WRITE(REG_TOP_CKG_SM_CACLK_M,HW_READ(REG_TOP_CKG_SM_CACLK_M)|0x0001);

                HW_WRITE(REG_TOP_CKG_SM_CACLK_EXT,HW_READ(REG_TOP_CKG_SM_CACLK_EXT)&0xBFFF);
                HW_WRITE(REG_TOP_CKG_SM_CACLK_EXT,HW_READ(REG_TOP_CKG_SM_CACLK_EXT)|0x4000);
                HW_WRITE(REG_TOP_CKG_SM_CACLK_EXT,HW_READ(REG_TOP_CKG_SM_CACLK_EXT)&0xBFFF);

                HW_WRITE(REG_TOP_CKG_SM_CACLK_EXT,HW_READ(REG_TOP_CKG_SM_CACLK_EXT)|0x8000);
                break;
            default:
                SC_ERR("unknown SC_Set_Clk\n");
            break;
        }
    }
    else if (u8SCID == 1)
    {
        reg &= (~TOP_CKG_SM_CA1_CLK_MASK);
        HW_WRITE(REG_TOP_CKG_SM1_CACLK_EXT,HW_READ(REG_TOP_CKG_SM1_CACLK_EXT)&0x7FFF);
        switch (eClk)
        {
            case E_HAL_SC_CLK_3M :
                reg |= TOP_CKG_SM_CA1_CLK_27M_D8;
                break;
            case E_HAL_SC_CLK_4P5M :
                reg |= TOP_CKG_SM_CA1_CLK_27M_D6;
                break;
            case E_HAL_SC_CLK_6M :
                reg |= TOP_CKG_SM_CA1_CLK_27M_D4;
                break;
            case E_HAL_SC_CLK_13M :
                reg |= TOP_CKG_SM_CA1_CLK_27M_D2;
                break;
            case E_HAL_SC_CLK_4M:
                HW_WRITE(REG_TOP_CKG_SM1_CACLK_N,0x6C);
                HW_WRITE(REG_TOP_CKG_SM1_CACLK_M,HW_READ(REG_TOP_CKG_SM1_CACLK_M)& ~0x00FF);
                HW_WRITE(REG_TOP_CKG_SM1_CACLK_M,HW_READ(REG_TOP_CKG_SM1_CACLK_M)|0x0001);

                HW_WRITE(REG_TOP_CKG_SM1_CACLK_EXT,HW_READ(REG_TOP_CKG_SM1_CACLK_EXT)&0xBFFF);
                HW_WRITE(REG_TOP_CKG_SM1_CACLK_EXT,HW_READ(REG_TOP_CKG_SM1_CACLK_EXT)|0x4000);
                HW_WRITE(REG_TOP_CKG_SM1_CACLK_EXT,HW_READ(REG_TOP_CKG_SM1_CACLK_EXT)&0xBFFF);

                HW_WRITE(REG_TOP_CKG_SM1_CACLK_EXT,HW_READ(REG_TOP_CKG_SM1_CACLK_EXT)|0x8000);
                break;
            default:
                SC_ERR("unknown SC_Set_Clk\n");
            break;
        }
    }

    HW_WRITE(REG_TOP_CKG_SM_CA, reg);
}

MS_U32 HAL_SC_GetHwVersion(void)
{
    if (K6U02)
        return (MS_U32)(0x00010110); //0.1.1.2
    else
        return (MS_U32)(0x00010100); //0.1.1.0
}

void HAL_SC_Init(MS_U8 u8SCID)
{
    //reg_allpad_in
    HW_WRITE(REG_TOP_ALLPAD_IN, HW_READ(REG_TOP_ALLPAD_IN) & ~TOP_ALLPAD_IN_EN);

    // Setup smc CLK driving
    HW_WRITE(REG_TOP_SM0_DRV0, (HW_READ(REG_TOP_SM0_DRV0) & ~(TOP_SM0_DRV0_MASK)) | TOP_SM0_DRV0_CLK);
    HW_WRITE(REG_TOP_SM0_DRV1, (HW_READ(REG_TOP_SM0_DRV1) & ~(TOP_SM0_DRV1_MASK)) | TOP_SM0_DRV1_CLK);

    // Setup smc IO 11K pu
    HW_WRITE(REG_TOP_SM0_PU, (HW_READ(REG_TOP_SM0_PU) & ~(TOP_SM0_PU_11K_MASK)) | TOP_SM0_PU_11K_IO);

    // enable the source clock of smart card.
    HAL_SC_PowerCtrl(u8SCID, TRUE);

    // Let the related I/O Low
    SC_WRITE(u8SCID,UART_LCR, UART_LCR_SBC); //I/O

    SC_AND(u8SCID, UART_CTRL2, ~(UART_CTRL2_TX_LEVEL)); // Clear tx level
    SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2)|UART_CTRL2_TX_LEVEL_9_TO_8|UART_CTRL2_CGWT_MASK|UART_CTRL2_BGWT_MASK); // tx fifo from 9 -> 8

    // enable nds flow control
    SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2)|UART_CTRL2_NDS_FLC_EN);
    SC_WRITE(u8SCID, UART_CTRL3, SC_READ(u8SCID, UART_CTRL3)|UART_NDS_FLC_BLK_DATA_CTRL);
    SC_WRITE(u8SCID, UART_SCFR, SC_READ(u8SCID, UART_SCFR)|UART_SCFR_STOP_BIT_CNT_REACH_NDS_FLC_EN);

    // BGT default value will be 0x16, reset to 0 here to disable the BGT int
    HAL_SC_SetBGT(u8SCID, 0x00);

    // Set external 8024 to inactive mode
    HAL_SC_Ext8024DeactiveSeq(u8SCID);

    // open smard_card_mode = 1
    SC_WRITE(u8SCID, UART_SCMR, SC_READ(u8SCID, UART_SCMR)|UART_SCMR_SMARTCARD);

    // disable smc clock
    HAL_SC_SmcClkCtrl(u8SCID, FALSE);

}

void HAL_SC_Exit(MS_U8 u8SCID)
{
    HAL_SC_PowerCtrl(u8SCID, FALSE);
}

MS_BOOL HAL_SC_Open(MS_U8 u8SCID, HAL_SC_OPEN_SETTING *pstOpenSetting)
{
    MS_U8 u8RegTmp;

    if (pstOpenSetting == NULL)
        return FALSE;

    if (pstOpenSetting->eVccCtrlType == E_HAL_SC_VCC_INT_8024)
    {
        // Config SMC PAD for internal analog circuit
        SC_AND(u8SCID, UART_CTRL6, ~UART_CTRL6_GCR_SMC_3P3_GPIO_EN);

        // Power down analog circuit and SMC PAD
        SC_OR(u8SCID, UART_CTRL6, UART_CTRL6_PD_SMC_LDO);
        SC_OR(u8SCID, UART_CTRL7, UART_CTRL7_PD_SMC_PAD);
        SC_OR(u8SCID, UART_CTRL14, UART_CTRL14_PD_SD_LDO);

        // Enable auto seq control and set all interface off
        SC_WRITE(u8SCID, UART_CTRL5, SC_READ(u8SCID, UART_CTRL5)|UART_CTRL5_AUTO_SEQ_CTRL);
        SC_WRITE(u8SCID, UART_CTRL5, SC_READ(u8SCID, UART_CTRL5)|UART_CTRL5_PAD_MASK);
        SC_WRITE(u8SCID, UART_CTRL5, SC_READ(u8SCID, UART_CTRL5)&(~UART_CTRL5_PAD_MASK));

        // need to call it, otherwise tx/rx will be failed
        SC_OR(u8SCID, UART_CTRL7, UART_CTRL7_ACT_PULLUP_EN); // active pull up en
    }
    else
    {
        // Enable GPIO function for external 8024 use case
        SC_OR(u8SCID, UART_CTRL6, UART_CTRL6_GCR_SMC_3P3_GPIO_EN);

        // Power on SMC PAD
        SC_AND(u8SCID, UART_CTRL7, ~UART_CTRL7_PD_SMC_PAD);
}

    // Set card detect type
    HAL_SC_CardInvert(u8SCID, pstOpenSetting->eCardDetType);

    // Set analog circuit donot care card reset and counter donot care card reset
    // *****About counter donot care card reset*****
    // For 5V case, if RST is '0', then HW internal counter will be also 0 to cause VCC always stays at 3V
    // So we must set UART_CTRL8_ANALOG_CNT_DONT_CARE_CARD_RESET to ignore RST
    SC_OR(u8SCID, UART_CTRL8, UART_CTRL8_ANALOG_CNT_DONT_CARE_CARD_RESET|UART_CTRL8_ANALOG_DC_CARD_RESET);

    // power on internal analog circuit and set card voltage
    HAL_SC_CardVoltage_Config(u8SCID, pstOpenSetting->eVoltageCtrl, pstOpenSetting->eVccCtrlType);

    if (HAL_SC_SetUartDiv(u8SCID, pstOpenSetting->eClkCtrl, pstOpenSetting->u16ClkDiv) == FALSE)
    {
        return FALSE;
    }
    if(HAL_SC_SetUartMode(u8SCID, pstOpenSetting->u8UartMode) == FALSE)
    {
        return FALSE;
    }
    if (HAL_SC_SetConvention(u8SCID, pstOpenSetting->eConvCtrl) == FALSE)
    {
        return FALSE;
    }

    if (pstOpenSetting->eVccCtrlType == E_HAL_SC_VCC_EXT_8024)
    {
        HAL_SC_Ext8024DeactiveSeq(u8SCID);

        MsOS_DelayTask(1); // For falling edge after CardIn detect.
    }
    else if (pstOpenSetting->eVccCtrlType == E_HAL_SC_VCC_INT_8024)
    {

    }
    else if (pstOpenSetting->eVccCtrlType == E_HAL_SC_VCC_OCP_HIGH)//OCP init
    {
        SC_OR(u8SCID,UART_SCMR, UART_SCMR_SMARTCARD);
    }
    else
    {
        if (pstOpenSetting->eVccCtrlType == E_HAL_SC_VCC_HIGH)
        {
            SC_AND(u8SCID, UART_SCFR, ~(UART_SCFR_V_ENABLE));// 'b0
        }
        else if (pstOpenSetting->eVccCtrlType == E_HAL_SC_VCC_LOW)
        {
            SC_OR(u8SCID, UART_SCFR, (UART_SCFR_V_ENABLE)); // 'b1
        }

    }

    // Init UART_SCSR
    u8RegTmp = (SC_READ(u8SCID,UART_SCSR) & UART_SCSR_CLK);
    SC_WRITE(u8SCID, UART_SCSR, u8RegTmp);

    // Power cut config
    HAL_SC_PwrCutDeactiveCtrl(u8SCID, FALSE);
    HAL_SC_PwrCutDeactiveCfg(u8SCID, TRUE);

    return TRUE;
}

void HAL_SC_Close(MS_U8 u8SCID, HAL_SC_VCC_CTRL eVccCtrlType)
{
    // Disable Smartcard Clk ----------------------------------------------------------------------

    SC_WRITE(u8SCID,UART_SCSR, 0x00);                                       // disable clock
    // Disable Smartcard IC Vcc Control -----------------------------------------------------------

    if (eVccCtrlType == E_HAL_SC_VCC_EXT_8024)
    {
        SC_AND(u8SCID,UART_SCFR, ~(UART_SCFR_V_ENABLE));
    }
    else if (eVccCtrlType == E_HAL_SC_VCC_INT_8024)
    {
    }
    else
    {
        if ((eVccCtrlType == E_HAL_SC_VCC_HIGH) ||
             (eVccCtrlType == E_HAL_SC_VCC_OCP_HIGH))
        {
            HAL_SC_SmcVccPadCtrl(u8SCID, E_HAL_SC_LEVEL_LOW);
        }
        else
        {
            HAL_SC_SmcVccPadCtrl(u8SCID, E_HAL_SC_LEVEL_HIGH);
        }
    }
}

void HAL_SC_PowerCtrl(MS_U8 u8SCID, MS_BOOL bEnable)
{
    if (u8SCID == 0)
    {
        if (bEnable)
        {
            // SM0: enable switch to specified clk
            HW_WRITE(REG_TOP_CKG_UART_SRC,  HW_READ(REG_TOP_CKG_UART_SRC) & ~TOP_CKG_UART1_SRC_CLK);
            HW_WRITE(REG_TOP_CKG_UART_CLK, (HW_READ(REG_TOP_CKG_UART_CLK) & ~(TOP_CKG_UART1_CLK_DIS | TOP_CKG_UART1_CLK_MASK)) | SC_TOP_CKG_UART1_CLK);
            HW_WRITE(REG_TOP_CKG_UART_SRC,  HW_READ(REG_TOP_CKG_UART_SRC) |  TOP_CKG_UART1_SRC_CLK);

            // dont enable the clock when init. Let the flow choose the clock enable time
            //HW_WRITE(REG_TOP_CKG_SM_CA, HW_READ(REG_TOP_CKG_SM_CA) & ~TOP_CKG_SM_CA0_DIS);
        }
        else
        {
            // SM0: disable clk
            HAL_SC_SmcClkCtrl(u8SCID, FALSE); // disable clock
            HW_WRITE(REG_TOP_CKG_UART_CLK, (HW_READ(REG_TOP_CKG_UART_CLK) | TOP_CKG_UART1_CLK_DIS)); //disable source clock
        }
    }
    if (u8SCID ==1)
    {
        if (bEnable)
        {
            // SM1: enable switch to specified clk
            HW_WRITE(REG_TOP_CKG_UART_SRC,  HW_READ(REG_TOP_CKG_UART_SRC) & ~TOP_CKG_UART2_SRC_CLK);
            HW_WRITE(REG_TOP_CKG_UART_CLK, (HW_READ(REG_TOP_CKG_UART_CLK) & ~(TOP_CKG_UART2_CLK_DIS | TOP_CKG_UART2_CLK_MASK)) | SC_TOP_CKG_UART2_CLK);
            HW_WRITE(REG_TOP_CKG_UART_SRC,  HW_READ(REG_TOP_CKG_UART_SRC) |  TOP_CKG_UART2_SRC_CLK);

            // dont enable the clock when init. Let the flow choose the clock enable time
            //HW_WRITE(REG_TOP_CKG_SM_CA, HW_READ(REG_TOP_CKG_SM_CA) & ~TOP_CKG_SM_CA1_DIS);
        }
        else
        {
            // SM1: disable switch to specified clk
            HAL_SC_SmcClkCtrl(u8SCID, FALSE); // disable clock
            HW_WRITE(REG_TOP_CKG_UART_CLK, (HW_READ(REG_TOP_CKG_UART_CLK) | TOP_CKG_UART2_CLK_DIS)); // disable source clock
        }
    }
}

void HAL_SC_CardInvert(MS_U8 u8SCID, HAL_SC_CARD_DET_TYPE eTpye)
{
    if (eTpye == E_SC_CARD_DET_LOW_ACTIVE)
    {
        SC_OR(u8SCID,UART_CTRL3, UART_INVERT_CD);
    }
    else
    {
        SC_AND(u8SCID,UART_CTRL3, ~UART_INVERT_CD);
    }
}

MS_BOOL HAL_SC_CardVoltage_Config(MS_U8 u8SCID, HAL_SC_VOLTAGE_CTRL eVoltage, HAL_SC_VCC_CTRL eVccCtrl)
{
    if (eVccCtrl == E_HAL_SC_VCC_INT_8024)
    {
        SC_WRITE(u8SCID,UART_CTRL6, SC_READ(u8SCID, UART_CTRL6)|UART_CTRL6_SHORT_CIRCUIT_DISABLE); // short circuit disable
        if (eVoltage == E_HAL_SC_VOL_CTRL_5V) // go 5v
        {
            // Disable 1.8v and set PD_SD_LDO
            SC_AND(u8SCID, UART_CTRL14, ~(UART_CTRL14_GCR_SMC_LDO_1P8V_EN));
            SC_OR(u8SCID, UART_CTRL14, UART_CTRL14_PD_SD_LDO);

            // Set ldo_selfb of 0x08 for 5V/3V
            SC_AND(u8SCID, UART_CTRL11, ~(UART_CTRL11_TEST_SMC_LDO_SELFB));
            SC_WRITE(u8SCID, UART_CTRL11, SC_READ(u8SCID, UART_CTRL11)|0x08);
            SC_AND(u8SCID,UART_CTRL11, ~(UART_CTRL11_SMC_SW_1P8V_EN));

            SC_AND(u8SCID, UART_CTRL6, ~(UART_CTRL6_PD_SMC_LDO|UART_CTRL6_GCR_SMC_3P3_GPIO_EN));
            SC_WRITE(u8SCID, UART_CTRL6, SC_READ(u8SCID, UART_CTRL6)|UART_CTRL6_LDO_3V_5V_EN); // enable 5V mode
        }
        else if (eVoltage == E_HAL_SC_VOL_CTRL_3P3V) // go 3.3v
        {
            // Disable 1.8v and set PD_SD_LDO
            SC_AND(u8SCID, UART_CTRL14, ~(UART_CTRL14_GCR_SMC_LDO_1P8V_EN));
            SC_OR(u8SCID, UART_CTRL14, UART_CTRL14_PD_SD_LDO);

            // Set ldo_selfb of 0x0C for 3.3V
            SC_AND(u8SCID, UART_CTRL11, ~(UART_CTRL11_TEST_SMC_LDO_SELFB));
            SC_WRITE(u8SCID, UART_CTRL11, SC_READ(u8SCID, UART_CTRL11)|0x0C);
            SC_AND(u8SCID,UART_CTRL11, ~(UART_CTRL11_SMC_SW_1P8V_EN));

            SC_AND(u8SCID, UART_CTRL6, ~(UART_CTRL6_PD_SMC_LDO|UART_CTRL6_GCR_SMC_3P3_GPIO_EN));
            SC_AND(u8SCID, UART_CTRL6, ~(UART_CTRL6_LDO_3V_5V_EN));
        }
        else if (eVoltage == E_HAL_SC_VOL_CTRL_3V) // go 3v
        {
            // Disable 1.8v and set PD_SD_LDO
            SC_AND(u8SCID, UART_CTRL14, ~(UART_CTRL14_GCR_SMC_LDO_1P8V_EN));
            SC_OR(u8SCID, UART_CTRL14, UART_CTRL14_PD_SD_LDO);

            // Set ldo_selfb of 0x08 for 5V/3V
            SC_AND(u8SCID, UART_CTRL11, ~(UART_CTRL11_TEST_SMC_LDO_SELFB));
            SC_WRITE(u8SCID, UART_CTRL11, SC_READ(u8SCID, UART_CTRL11)|0x08);
            SC_AND(u8SCID,UART_CTRL11, ~(UART_CTRL11_SMC_SW_1P8V_EN));

            SC_AND(u8SCID, UART_CTRL6, ~(UART_CTRL6_PD_SMC_LDO|UART_CTRL6_GCR_SMC_3P3_GPIO_EN));
            SC_AND(u8SCID, UART_CTRL6, ~(UART_CTRL6_LDO_3V_5V_EN));
        }
        else if (eVoltage == E_HAL_SC_VOL_CTRL_1P8V) // go 1.8v
        {
            // Set PD_SMC_LDO
            SC_OR(u8SCID, UART_CTRL6, UART_CTRL6_PD_SMC_LDO);
            SC_AND(u8SCID, UART_CTRL6, ~(UART_CTRL6_GCR_SMC_3P3_GPIO_EN));
            SC_AND(u8SCID, UART_CTRL6, ~(UART_CTRL6_LDO_3V_5V_EN));

            // Enable 1.8v and clear PD_SD_LDO
            SC_OR(u8SCID, UART_CTRL14, UART_CTRL14_GCR_SMC_LDO_1P8V_EN);
            SC_AND(u8SCID, UART_CTRL14, ~(UART_CTRL14_PD_SD_LDO));

            // Set ldo_selfb of 0x0C for 1.8V
            SC_AND(u8SCID, UART_CTRL11, ~(UART_CTRL11_TEST_SMC_LDO_SELFB));
            SC_WRITE(u8SCID, UART_CTRL11, SC_READ(u8SCID, UART_CTRL11)|0x04);
            SC_OR(u8SCID,UART_CTRL11, UART_CTRL11_SMC_SW_1P8V_EN);
        }
        else
        {
            return FALSE;
        }
        MsOS_DelayTask(1);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

MS_BOOL HAL_SC_IsParityError(MS_U8 u8SCID)
{
    MS_U8 u8Data;

    u8Data = _HAL_SC_GetLsr(u8SCID);
    _HAL_SC_ClearLsr(u8SCID);

    if ((u8Data & UART_LSR_PE))
        return TRUE;
    else
        return FALSE;
}

MS_BOOL HAL_SC_IsRxDataReady(MS_U8 u8SCID)
{
    MS_U8 u8Data;

    u8Data = _HAL_SC_GetLsr(u8SCID);
    if ((u8Data & UART_LSR_DR))
        return TRUE;
    else
        return FALSE;
}

MS_BOOL HAL_SC_IsTxFIFO_Empty(MS_U8 u8SCID)
{
    if ((SC_READ(u8SCID, UART_LSR) & UART_LSR_THRE))
        return TRUE;
    else
        return FALSE;
}

void HAL_SC_WriteTxData(MS_U8 u8SCID, MS_U8 u8TxData)
{
    SC_WRITE(u8SCID, UART_TX, u8TxData);
}

MS_U8 HAL_SC_ReadRxData(MS_U8 u8SCID)
{
    return SC_READ(u8SCID,UART_RX);
}

MS_BOOL HAL_SC_IsCardDetected(MS_U8 u8SCID)
{
    if ((SC_READ(u8SCID, UART_SCSR) & UART_SCSR_DETECT))
        return TRUE;
    else
        return FALSE;
}

void HAL_SC_SetIntCGWT(MS_U8 u8SCID, HAL_SC_CGWT_INT eIntCGWT)
{
    switch(eIntCGWT)
    {
        case E_HAL_SC_CGWT_INT_DISABLE:
        default:
            SC_WRITE(u8SCID, UART_SCSR, SC_READ(u8SCID, UART_SCSR)|(UART_SCSR_CWT_MASK+UART_SCSR_CGT_MASK));
            SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2)|(UART_CTRL2_CGWT_MASK));
            break;

        case E_HAL_SC_CGWT_INT_ENABLE:
            SC_WRITE(u8SCID, UART_SCSR, SC_READ(u8SCID, UART_SCSR)&(~(UART_SCSR_CWT_MASK+UART_SCSR_CGT_MASK)));
            SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2)&(~UART_CTRL2_CGWT_MASK));
            break;

        case E_HAL_SC_CWT_ENABLE_CGT_DISABLE:
            SC_WRITE(u8SCID, UART_SCSR, SC_READ(u8SCID, UART_SCSR)&(~(UART_SCSR_CWT_MASK)));
            SC_WRITE(u8SCID, UART_SCSR, SC_READ(u8SCID, UART_SCSR)|(UART_SCSR_CGT_MASK));
            SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2)&(~UART_CTRL2_CGWT_MASK));
            break;

    }
}

void HAL_SC_SetIntBGWT(MS_U8 u8SCID, HAL_SC_BGWT_INT eIntBGWT)
{
    switch(eIntBGWT)
    {
        case E_HAL_SC_BGWT_INT_DISABLE:
        default:
            SC_WRITE(u8SCID, UART_SCSR, SC_READ(u8SCID, UART_SCSR)|(UART_SCSR_BGT_MASK));
            SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2)|(UART_CTRL2_BGWT_MASK));
            break;

        case E_HAL_SC_BGWT_INT_ENABLE:
            SC_WRITE(u8SCID, UART_SCSR, SC_READ(u8SCID, UART_SCSR)&(~(UART_SCSR_BGT_MASK)));
            SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2)&(~UART_CTRL2_BGWT_MASK));
            break;

        case E_HAL_SC_BWT_ENABLE_BGT_DISABLE:
            SC_WRITE(u8SCID, UART_SCSR, SC_READ(u8SCID, UART_SCSR)|(UART_SCSR_BGT_MASK));
            SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2)&(~UART_CTRL2_BGWT_MASK));
            break;
    }
}

void HAL_SC_GetIntTxLevelAndGWT(MS_U8 u8SCID, HAL_SC_TX_LEVEL_GWT_INT *pstTxLevelGWT)
{
    MS_U8 u8RegData;

    u8RegData = SC_READ(u8SCID,UART_GWT_INT);
    memset(pstTxLevelGWT, 0x00, sizeof(HAL_SC_TX_LEVEL_GWT_INT));

    if (u8RegData & UART_GWT_TX_LEVEL_INT)
        pstTxLevelGWT->bTxLevelInt = TRUE;

    if (u8RegData & UART_GWT_CWT_RX_FAIL)
        pstTxLevelGWT->bCWT_RxFail = TRUE;

    if (u8RegData & UART_GWT_CWT_TX_FAIL)
        pstTxLevelGWT->bCWT_TxFail = TRUE;

    if (u8RegData & UART_GWT_CGT_RX_FAIL)
        pstTxLevelGWT->bCGT_RxFail = TRUE;

    if (u8RegData & UART_GWT_CGT_TX_FAIL)
        pstTxLevelGWT->bCGT_TxFail = TRUE;

    if (u8RegData & UART_GWT_BGT_FAIL)
        pstTxLevelGWT->bBGT_Fail = TRUE;

    if (u8RegData & UART_GWT_BWT_FAIL)
        pstTxLevelGWT->bBWT_Fail = TRUE;
}

void HAL_SC_ClearIntTxLevelAndGWT(MS_U8 u8SCID)
{
    SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2) | UART_CTRL2_FLAG_CLEAR);
    SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2) & (~UART_CTRL2_FLAG_CLEAR));
}

void HAL_SC_SetBWT(MS_U8 u8SCID, MS_U32 u32BWT_etu)
{
    // The HW counter is start from '0', so we need to subtract '1' from u32BWT_etu
    if (u32BWT_etu > 0)
        u32BWT_etu = u32BWT_etu - 1;

    SC_WRITE(u8SCID, UART_SCBWT_0, (MS_U8)(u32BWT_etu&0xFF));
    SC_WRITE(u8SCID, UART_SCBWT_1, (MS_U8)((u32BWT_etu>>8)&0xFF));
    SC_WRITE(u8SCID, UART_SCBWT_2, (MS_U8)((u32BWT_etu>>16)&0xFF));
    SC_WRITE(u8SCID, UART_SCBWT_3, (MS_U8)((u32BWT_etu>>24)&0xFF));
}

MS_BOOL HAL_SC_SetRstToAtrByBWT(MS_U8 u8SCID, MS_U32 u32Timeout, MS_U32 u32CardClk)
{
    MS_U32 u32Num;
    MS_U32 u32Clk;

    u32Clk = _HAL_SC_GetTopClkNum(u8SCID);
    if (u32Clk == 0 || u32CardClk == 0)
        return FALSE;

    u32Num= (MS_U32)(u32Clk/u32CardClk);
    u32Num = (u32Num * u32Timeout)/16UL;

    HAL_SC_SetBWT(u8SCID, u32Num);

    return TRUE;
}

void HAL_SC_SetBGT(MS_U8 u8SCID, MS_U8 u8BGT_etu)
{
    // The HW counter is start from '0', so we need to subtract '1' from u8BGT_etu
    if (u8BGT_etu > 0)
        u8BGT_etu = u8BGT_etu - 1;

    SC_WRITE(u8SCID, UART_SCBGT, u8BGT_etu);
}

void HAL_SC_SetCWT(MS_U8 u8SCID, MS_U32 u32CWT_etu)
{
    // The HW counter is start from '0', so we need to subtract '1' from u32CWT_etu
    if (u32CWT_etu > 0)
        u32CWT_etu = u32CWT_etu - 1;

    SC_WRITE(u8SCID, UART_SCCWT_L, (MS_U8)(u32CWT_etu&0xFF));
    SC_WRITE(u8SCID, UART_SCCWT_H, (MS_U8)((u32CWT_etu>>8)&0xFF));

    // Fix ext-CWT issue
    if (K6U02)
    {
        SC_OR(u8SCID,UART_CTRL14, UART_CTRL4_EXT_CWT_ECO_EN_BIT);
        SC_OR(u8SCID,UART_CTRL8, UART_CTRL8_CWT_EXT_EN);
        SC_WRITE(u8SCID, UART_CTRL9, (MS_U8)((u32CWT_etu>>16)&0xFF));
    }
}

void HAL_SC_SetCGT(MS_U8 u8SCID, MS_U8 u8gCGT_etu)
{
    MS_U8 u8ExtraGT;

    if (u8gCGT_etu <= 12)
        u8ExtraGT = 0;
    else
        u8ExtraGT = u8gCGT_etu - 12;

    SC_WRITE(u8SCID, UART_SCCGT, u8ExtraGT);
}

MS_BOOL HAL_SC_SetUartDiv(MS_U8 u8SCID, HAL_SC_CLK_CTRL eClk, MS_U16 u16ClkDiv)
{
    MS_U16 u16div;
    MS_U32 clk;

    if(u8SCID >= SC_DEV_NUM)
        return FALSE;

    clk = _HAL_SC_GetTopClkNum(u8SCID);
    if (clk == 0)
                return FALSE;

    SC_WRITE(u8SCID,UART_MCR, 0);

    switch (eClk)
    {
    case E_HAL_SC_CLK_3M:
        //u16clk = TOP_CKG_CAM_SRC_27M_D8;/* 3.375M */
        clk=clk/1000;
        u16div = (clk*u16ClkDiv)/(16*3375);
        break;
    case E_HAL_SC_CLK_4P5M:
        //u16clk = TOP_CKG_CAM_SRC_27M_D6;/* 4.5M */
        clk=clk/1000;
        u16div = (clk*u16ClkDiv)/(16*4500);
        break;
    case E_HAL_SC_CLK_6M:
        //u16clk = TOP_CKG_CAM_SRC_27M_D4;/* 6.75M */
        clk=clk/1000;
        u16div = (clk*u16ClkDiv)/(16*6750);
        break;
    case E_HAL_SC_CLK_13M:
        //u16clk = TOP_CKG_CAM_SRC_27M_D2;/* 13.5M */
        clk=clk/1000;
        SC_WRITE(u8SCID,UART_MCR, UART_MCR_FAST);  // UART_MCR_FAST on mode
        clk=clk/16;
        u16div = (13500*65536)/(clk*u16ClkDiv);
        break;
    case E_HAL_SC_CLK_4M:
        clk=clk/1000;
        u16div = (clk*u16ClkDiv)/(16*4000);
        break;
    default:
        return FALSE;
    }

    // Switch to specified clk
    HAL_SC_SetClk(u8SCID,eClk);

    // Set divider
    SC_OR(u8SCID,UART_LCR, UART_LCR_DLAB);                                     // Line Control Register
    SC_WRITE(u8SCID,UART_DLL, u16div & 0x00FF);                                // Divisor Latch Low
    SC_WRITE(u8SCID,UART_DLM, u16div >> 8);                                  // Divisor Latch High
    SC_AND(u8SCID,UART_LCR, ~(UART_LCR_DLAB));                              // Line Control Register

    return TRUE;
}

MS_BOOL HAL_SC_SetUartMode(MS_U8 u8SCID, MS_U8 u8UartMode)
{
    if(u8SCID >= SC_DEV_NUM)
        return FALSE;

    SC_WRITE(u8SCID,UART_LCR, (SC_READ(u8SCID,UART_LCR)&0xE0) | u8UartMode);

    // If parity check is enable, open parity error record
    if (u8UartMode & UART_LCR_PARITY)
    {
        SC_OR(u8SCID,UART_CTRL2, UART_CTRL2_REC_PE_EN);
        SC_WRITE(u8SCID, UART_CTRL3, SC_READ(u8SCID, UART_CTRL3)|UART_TRAN_ETU_CTL);
    }
    return TRUE;
}

MS_BOOL HAL_SC_SetConvention(MS_U8 u8SCID, HAL_SC_CONV_CTRL eConvCtrl)
{
    if(u8SCID >= SC_DEV_NUM)
        return FALSE;

    if (eConvCtrl == E_HAL_SC_CONV_DIRECT)
    {
        // direct convention
        SC_WRITE(u8SCID,UART_SCCR, SC_READ(u8SCID,UART_SCCR)&~( UART_SCCR_RX_BSWAP | UART_SCCR_RX_BINV |
                                    UART_SCCR_TX_BSWAP | UART_SCCR_TX_BINV ));
    }
    else
    {
        // inverse convention
        SC_WRITE(u8SCID,UART_SCCR, SC_READ(u8SCID,UART_SCCR) | (UART_SCCR_RX_BSWAP | UART_SCCR_RX_BINV |
                                 UART_SCCR_TX_BSWAP  | UART_SCCR_TX_BINV));
    }

    return TRUE;
}

MS_BOOL HAL_SC_SetInvConvention(MS_U8 u8SCID)
{
    if(u8SCID >= SC_DEV_NUM)
    return FALSE;

    SC_XOR(u8SCID, UART_SCCR, UART_SCCR_RX_BSWAP | UART_SCCR_RX_BINV | UART_SCCR_TX_BSWAP | UART_SCCR_TX_BINV);
    return TRUE;
}

void HAL_SC_SetUartInt(MS_U8 u8SCID, MS_U8 u8IntBitMask)
{
    MS_U8 u8RegData = 0x00;

    if (u8IntBitMask & E_HAL_SC_UART_INT_RDI)
        u8RegData |= UART_IER_RDI;
    if (u8IntBitMask & E_HAL_SC_UART_INT_THRI)
        u8RegData |= UART_IER_THRI;
    if (u8IntBitMask & E_HAL_SC_UART_INT_RLSI)
        u8RegData |= UART_IER_RLSI;
    if (u8IntBitMask & E_HAL_SC_UART_INT_MSI)
        u8RegData |= UART_IER_MSI;

    SC_WRITE(u8SCID,UART_IER, u8RegData);
}

HAL_SC_UART_INT_ID HAL_SC_GetUartIntID(MS_U8 u8SCID)
{
    HAL_SC_UART_INT_ID eIntID;
    MS_U8 u8GetIntID = 0x00;
    MS_U16 u16IIRStatus = SC_READ(u8SCID,UART_IIR);

    if (u16IIRStatus & UART_IIR_NO_INT)
    {
        eIntID = E_HAL_SC_UART_INT_ID_NONE;
    }
    else
    {
        u8GetIntID = (u16IIRStatus & UART_IIR_ID);
        switch(u8GetIntID)
        {
            case UART_IIR_RLSI:
                eIntID = E_HAL_SC_UART_INT_ID_RLS;
                break;

            case UART_IIR_RDI:
                eIntID = E_HAL_SC_UART_INT_ID_RDA;
                break;

            case UART_IIR_TOI:
                eIntID = E_HAL_SC_UART_INT_ID_CT;
                break;

            case UART_IIR_THRI:
                eIntID = E_HAL_SC_UART_INT_ID_THRE;
                break;

            case UART_IIR_MSI:
                eIntID = E_HAL_SC_UART_INT_ID_MS;
                break;

            default:
                eIntID = E_HAL_SC_UART_INT_ID_NONE;
                break;
        }
    }

    return eIntID;
}

void HAL_SC_SetUartFIFO(MS_U8 u8SCID, HAL_SC_FIFO_CTRL *pstCtrlFIFO)
{
    MS_U8 u8RegData = 0x00;

    if (pstCtrlFIFO->bEnableFIFO)
        u8RegData |= UART_FCR_ENABLE_FIFO;
    if (pstCtrlFIFO->bClearRxFIFO)
        u8RegData |= UART_FCR_CLEAR_RCVR;
    if (pstCtrlFIFO->bClearTxFIFO)
        u8RegData |= UART_FCR_CLEAR_XMIT;

    if (pstCtrlFIFO->eTriLevel == E_HAL_SC_RX_FIFO_INT_TRI_1)
        u8RegData |= UART_FCR_TRIGGER_1;
    if (pstCtrlFIFO->eTriLevel == E_HAL_SC_RX_FIFO_INT_TRI_4)
        u8RegData |= UART_FCR_TRIGGER_4;
    if (pstCtrlFIFO->eTriLevel == E_HAL_SC_RX_FIFO_INT_TRI_8)
        u8RegData |= UART_FCR_TRIGGER_8;
    if (pstCtrlFIFO->eTriLevel == E_HAL_SC_RX_FIFO_INT_TRI_14)
        u8RegData |= UART_FCR_TRIGGER_14;

    SC_WRITE(u8SCID,UART_FCR, u8RegData);
}

void HAL_SC_SetSmcModeCtrl(MS_U8 u8SCID, HAL_SC_MODE_CTRL *pstModeCtrl)
{
    MS_U8 u8RegData = 0x00;

    if (pstModeCtrl->bFlowCtrlEn)
        u8RegData |= UART_SCMR_FLOWCTRL;
    if (pstModeCtrl->bParityChk)
        u8RegData |= UART_SCMR_PARITYCHK;
    if (pstModeCtrl->bSmartCardMdoe)
        u8RegData |= UART_SCMR_SMARTCARD;

    u8RegData |= (pstModeCtrl->u8ReTryTime & UART_SCMR_RETRY_MASK);

    SC_WRITE(u8SCID, UART_SCMR, u8RegData);
}

void HAL_SC_ClearCardDetectInt(MS_U8 u8SCID, HAL_SC_CD_INT_CLEAR eIntClear)
{
    switch(eIntClear)
    {
        case E_HAL_SC_CD_IN_CLEAR:
            SC_WRITE(u8SCID, UART_SCSR, (SC_READ(u8SCID,UART_SCSR) | UART_SCSR_INT_CARDIN));
            break;

        case E_HAL_SC_CD_OUT_CLEAR:
            SC_WRITE(u8SCID, UART_SCSR, (SC_READ(u8SCID,UART_SCSR) | UART_SCSR_INT_CARDOUT));
            break;

        case E_HAL_SC_CD_ALL_CLEAR:
            SC_WRITE(u8SCID, UART_SCSR, (SC_READ(u8SCID,UART_SCSR) | UART_SCSR_INT_CARDOUT | UART_SCSR_INT_CARDIN));
            break;
    }
}

void HAL_SC_GetCardDetectInt(MS_U8 u8SCID, HAL_SC_CD_INT_STATUS *pstCardDetectInt)
{
    MS_U8 u8RegData = 0x00;

    u8RegData = SC_READ(u8SCID, UART_SCSR);
    memset(pstCardDetectInt, 0x00, sizeof(HAL_SC_CD_INT_STATUS));
    if (u8RegData & UART_SCSR_INT_CARDIN)
        pstCardDetectInt->bCardInInt = TRUE;
    if (u8RegData & UART_SCSR_INT_CARDOUT)
        pstCardDetectInt->bCardOutInt = TRUE;
}

void HAL_SC_ResetPadCtrl(MS_U8 u8SCID, HAL_SC_LEVEL_CTRL eLogicalLevel)
{
    if (eLogicalLevel == E_HAL_SC_LEVEL_HIGH)
    {
        SC_OR(u8SCID, UART_SCCR, UART_SCCR_RST);//Pull RSTIN high
    }
    else
    {
        SC_AND(u8SCID, UART_SCCR, ~UART_SCCR_RST);//Pull RSTIN low
    }
}

void HAL_SC_SmcVccPadCtrl(MS_U8 u8SCID, HAL_SC_LEVEL_CTRL eLogicalLevel)
{
    if (eLogicalLevel == E_HAL_SC_LEVEL_HIGH)
    {
        SC_OR(u8SCID, UART_SCFR, UART_SCFR_V_HIGH);
    }
    else
    {
        SC_AND(u8SCID, UART_SCFR, ~UART_SCFR_V_HIGH);
    }
}

void HAL_SC_InputOutputPadCtrl(MS_U8 u8SCID, HAL_SC_IO_CTRL eCtrl)
{
    if (eCtrl == E_HAL_SC_IO_FORCED_LOW)
    {
        SC_OR(u8SCID, UART_LCR, UART_LCR_SBC); // As general I/O pin, idle is logical high

    }
    else
    {
        SC_AND(u8SCID, UART_LCR, ~UART_LCR_SBC); // I/O pin forced to logical low
    }
}

void HAL_SC_Ext8024ActiveSeq(MS_U8 u8SCID)
{
    // Pull 8024 CMDVCC pin low
    SC_AND(u8SCID,UART_SCFR, ~(UART_SCFR_V_ENABLE|UART_SCFR_V_HIGH));
}

void HAL_SC_Ext8024DeactiveSeq(MS_U8 u8SCID)
{
    // Pull 8024 CMDVCC pin high
    SC_OR(u8SCID, UART_SCFR, UART_SCFR_V_HIGH);
    SC_AND(u8SCID, UART_SCFR, ~(UART_SCFR_V_ENABLE));
}

void HAL_SC_Int8024ActiveSeq(MS_U8 u8SCID)
{
    // Enable power cut auto deactive function
    HAL_SC_PwrCutDeactiveCtrl(u8SCID, TRUE);

    //////////////////////////////////////////////
    // We have 2 limitation due to analog design
    // 1. IO/CLK/RST has pulse while vcc is truned on due to active sequence enable
    // 2. There is CLK signal found due to active sequence enable if clk is already enabled.
    //     CLK seems not fully be restrainted by PD_SMC_PAD
    //
    // So active sequence will be:
    //  (a) Set PD_SMC_PAD=1 (to avoid following IO/CLK/RST pulse output to smc pads)
    //  (b) Set ACTIVE_SEQ_EN=1
    //  (c) Delay few time (to wait vcc ready and IO/CLK/RST pulse disappeared)
    //  (d) Enable smc clk and set PD_SMC_PAD=0 to power on smc pad (do not add any time delay between smc clk and PD_SMC_PAD for system busy issue)
    //  (e) Wait some time to complete active sequence and set ACTIVE_SEQ_EN=0
    //
    SC_WRITE(u8SCID, UART_CTRL7, SC_READ(u8SCID, UART_CTRL7)|UART_CTRL7_ACT_PULLUP_EN); // active pull up en
    SC_WRITE(u8SCID, UART_ACTIVE_VCC, SC_ACTIVE_VCC_CNT);
    SC_WRITE(u8SCID, UART_ACTIVE_IO, SC_ACTIVE_IO_CNT);
    SC_WRITE(u8SCID, UART_ACTIVE_CLK, SC_ACTIVE_CLK_CNT);
    SC_WRITE(u8SCID, UART_ACTIVE_RST, SC_ACTIVE_RST_CNT);
    SC_WRITE(u8SCID, UART_CTRL5, SC_READ(u8SCID, UART_CTRL5)|UART_CTRL5_ACTIVE_SEQ_EN); // ori is 1
    MsOS_DelayTask(1);
    HAL_SC_SmcClkCtrl(u8SCID, TRUE);
    SC_WRITE(u8SCID, UART_CTRL7, SC_READ(u8SCID, UART_CTRL7)&(~UART_CTRL7_PD_SMC_PAD)); // reg_pd_smc_pad=0, power on pad
    MsOS_DelayTask(5); // ori 5
    SC_WRITE(u8SCID, UART_CTRL5, SC_READ(u8SCID, UART_CTRL5)&(~UART_CTRL5_ACTIVE_SEQ_EN)); // ori is 1
}

void HAL_SC_Int8024DeactiveSeq(MS_U8 u8SCID)
{
    SC_WRITE(u8SCID, UART_DEACTIVE_RST, SC_DEACTIVE_RST_CNT);
    SC_WRITE(u8SCID, UART_DEACTIVE_CLK, SC_DEACTIVE_CLK_CNT);
    SC_WRITE(u8SCID, UART_DEACTIVE_IO, SC_DEACTIVE_IO_CNT);
    SC_WRITE(u8SCID, UART_DEACTIVE_VCC, SC_DEACTIVE_VCC_CNT);
    SC_WRITE(u8SCID, UART_CTRL3, SC_READ(u8SCID, UART_CTRL3)|UART_DEACTIVE_SEQ_EN); // ori is 1
    MsOS_DelayTask(5);
    SC_WRITE(u8SCID, UART_CTRL3, SC_READ(u8SCID, UART_CTRL3)&(~UART_DEACTIVE_SEQ_EN)); // ori is 1
    SC_WRITE(u8SCID, UART_CTRL7, SC_READ(u8SCID, UART_CTRL7)|UART_CTRL7_PD_SMC_PAD); // reg_pd_smc_pad=1, power down pad

    // Set all interface off
    SC_WRITE(u8SCID, UART_CTRL5, SC_READ(u8SCID, UART_CTRL5)|UART_CTRL5_PAD_MASK);
    SC_WRITE(u8SCID, UART_CTRL5, SC_READ(u8SCID, UART_CTRL5)&(~UART_CTRL5_PAD_MASK));

    //Disable clk
    HAL_SC_SmcClkCtrl(u8SCID, FALSE);
}

void HAL_SC_Int8024PullResetPadLow(MS_U8 u8SCID, MS_U32 u32HoldTimeInMs)
{
    HAL_SC_ResetPadCtrl(u8SCID, E_HAL_SC_LEVEL_LOW);
    MsOS_DelayTask(u32HoldTimeInMs);
    SC_WRITE(u8SCID, UART_CTRL5, SC_READ(u8SCID, UART_CTRL5)|UART_CTRL5_PAD_RELEASE); // all interface release
    SC_WRITE(u8SCID, UART_CTRL5, SC_READ(u8SCID, UART_CTRL5)&(~UART_CTRL5_PAD_RELEASE)); // all interface release
}

void HAL_SC_SmcClkCtrl(MS_U8 u8SCID, MS_BOOL bEnableClk)
{
    if (u8SCID == 0)
    {
        if (bEnableClk)
        {
            HW_WRITE(REG_TOP_CKG_SM_CA, HW_READ(REG_TOP_CKG_SM_CA) & ~TOP_CKG_SM_CA0_DIS);// enable clock
        }
        else
        {
            HW_WRITE(REG_TOP_CKG_SM_CA, (HW_READ(REG_TOP_CKG_SM_CA) | TOP_CKG_SM_CA0_DIS)); // clock disable
        }
    }
#if (SC_DEV_NUM > 1) // no more than 2
    if (u8SCID == 1)
    {
        if (bEnableClk)
        {
            HW_WRITE(REG_TOP_CKG_SM_CA, HW_READ(REG_TOP_CKG_SM_CA) & ~TOP_CKG_SM_CA1_DIS);// enable clock
        }
        else
        {
            HW_WRITE(REG_TOP_CKG_SM_CA, (HW_READ(REG_TOP_CKG_SM_CA) | TOP_CKG_SM_CA1_DIS)); // clock disable
        }
    }
#endif
}

void HAL_SC_PwrCutDeactiveCfg(MS_U8 u8SCID, MS_BOOL bVccOffPolHigh)
{
    SC_WRITE(u8SCID, UART_DEACTIVE_RST, SC_DEACTIVE_RST_CNT);
    SC_WRITE(u8SCID, UART_DEACTIVE_CLK, SC_DEACTIVE_CLK_CNT);
    SC_WRITE(u8SCID, UART_DEACTIVE_IO, SC_DEACTIVE_IO_CNT);
    SC_WRITE(u8SCID, UART_DEACTIVE_VCC, SC_DEACTIVE_VCC_CNT);

    if (bVccOffPolHigh)
    {
        SC_OR(u8SCID, UART_CTRL3, (UART_AC_PWR_OFF_CTL | UART_VCC_OFF_POL));
    }
    else
    {
        SC_OR(u8SCID, UART_CTRL3, UART_AC_PWR_OFF_CTL);
    }
}

void HAL_SC_PwrCutDeactiveCtrl(MS_U8 u8SCID, MS_BOOL bEnable)
{
    if (bEnable)
    {
        SC_AND(u8SCID, UART_CTRL3, ~(UART_AC_PWR_OFF_MASK));
    }
    else
    {
        SC_OR(u8SCID, UART_CTRL3, UART_AC_PWR_OFF_MASK);
    }
}

void HAL_SC_RxFailAlwaysDetCWT(MS_U8 u8SCID, MS_BOOL bEnable)
{
    if (bEnable)
    {
        SC_OR(u8SCID, UART_TX_FIFO_COUNT, UART_TX_FIFO_CWT_RX_FAIL_DET_EN);
    }
    else
    {
        SC_AND(u8SCID, UART_TX_FIFO_COUNT, ~(UART_TX_FIFO_CWT_RX_FAIL_DET_EN));
    }
}

MS_BOOL HAL_SC_SetRstToIoTimeout(MS_U8 u8SCID, MS_U32 u32Timeout, MS_U32 u32CardClk)
{
    // Not Support
    (void)u8SCID;
    (void)u32Timeout;
    (void)u32CardClk;

    return TRUE;
}

void HAL_SC_RstToIoEdgeDetCtrl(MS_U8 u8SCID, MS_BOOL bEnable)
{
    // Not Support
    (void)u8SCID;
    (void)bEnable;
}

MS_BOOL HAL_SC_CheckIntRstToIoEdgeFail(MS_U8 u8SCID)
{
    // Not Support
    (void)u8SCID;

    return FALSE;
}

void HAL_SC_SetIntRstToIoEdgeFail(MS_U8 u8SCID, MS_BOOL bEnable)
{
    // Not Support
    (void)u8SCID;
    (void)bEnable;
}
