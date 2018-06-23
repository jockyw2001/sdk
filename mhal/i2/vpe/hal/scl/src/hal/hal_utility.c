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
#define HAL_SCLIRQ_C


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#include "ms_platform.h"
#include "drv_scl_os.h"
// Internal Definition
#include "hal_scl_reg.h"
#include "hal_utility.h"
#include "hal_scl_util.h"
#include "drv_scl_dbg.h"

//ISP Mload
#if defined(SCLOS_TYPE_MLOAD)
#include <mdrv_mload.h>
#endif
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DrvUtilityMutexLock()            DrvSclOsObtainMutexIrq(_UTILITY_Mutex)
#define DrvUtilityMutexUNLock()          DrvSclOsReleaseMutexIrq(_UTILITY_Mutex)
#define MLOAD_ID_BASE 33
#define CMDQ_CHECK_BY_INST 0
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
u32 UTILITY_RIU_BASE = 0;
u32 UTILITY_RIU_BASE_Vir = 0;
s32 _UTILITY_Mutex = -1;
void *gpvWdrTblBuffer = NULL;
void *gpvDefaultWdrTblBuffer = NULL;
void *gpvRegBuffer = NULL;
void *gpvDefaultRegBuffer = NULL;
void *gpvInquireBuffer = NULL;
void *gpvDefaultInquireBuffer = NULL;
void *gpvLastTimeInquireBuffer[3] = {NULL,NULL,NULL};
u32 *gu32TblCnt;
u32 *gu32DefaultTblCnt;
u32 *gp32WdrTblCnt = 0;
u32 *gp32DefaultWdrTblCnt;
DrvSclOsCmdqInterface_t gpstCmdq[E_HAL_SCL_UTI_CMDQID_MAX];
HalUtilityIdType_e genUtiId = E_HAL_SCL_UTI_ID_SC_DEFAULT;
void (*gpCmdqF)(u32 u32Reg,u16 u16Val,u16 u16Mask) = NULL;
//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
u32 _HalUtilitySwitchBankDefaultToType(u32 u32Bank)
{
    EN_REG_CONFIG_TYPE enType;
    switch(u32Bank)
    {
        case REG_SCL_HVSP0_BASE:
            enType = EN_REG_SCL_HVSP0_BASE;
            break;
        case REG_SCL_HVSP1_BASE:
            enType = EN_REG_SCL_HVSP1_BASE;
            break;
        case REG_SCL_HVSP2_BASE:
            enType = EN_REG_SCL_HVSP2_BASE;
            break;
        case REG_SCL_HVSP3_BASE:
            enType = EN_REG_SCL_HVSP3_BASE;
            break;
        case REG_SCL_DMA0_BASE:
            enType = EN_REG_SCL_DMA0_BASE;
            break;
        case REG_SCL_DMA1_BASE:
            enType = EN_REG_SCL_DMA1_BASE;
            break;
        case REG_SCL_DMA2_BASE:
            enType = EN_REG_SCL_DMA2_BASE;
            break;
        case REG_SCL_DMA3_BASE:
            enType = EN_REG_SCL_DMA3_BASE;
            break;
        case REG_SCL_NLM0_BASE:
            enType = EN_REG_SCL_NLM0_BASE;
            break;
        case REG_SCL0_BASE:
            enType = EN_REG_SCL0_BASE;
            break;
        case REG_SCL1_BASE:
            enType = EN_REG_SCL1_BASE;
            break;
        case REG_SCL2_BASE:
            enType = EN_REG_SCL2_BASE;
            break;
        case REG_VIP_ACE3_BASE:
            enType = EN_REG_VIP_ACE3_BASE;
            break;
        case REG_VIP_ACE_BASE:
            enType = EN_REG_VIP_ACE_BASE;
            break;
        case REG_VIP_PK_BASE:
            enType = EN_REG_VIP_PK_BASE;
            break;
        case REG_VIP_DLC_BASE:
            enType = EN_REG_VIP_DLC_BASE;
            break;
        case REG_VIP_MWE_BASE:
            enType = EN_REG_VIP_MWE_BASE;
            break;
        case REG_VIP_ACE2_BASE:
            enType = EN_REG_VIP_ACE2_BASE;
            break;
        case REG_VIP_LCE_BASE:
            enType = EN_REG_VIP_LCE_BASE;
            break;
        case REG_SCL_DNR1_BASE:
            enType = EN_REG_SCL_DNR1_BASE;
            break;
        case REG_SCL_DNR2_BASE:
            enType = EN_REG_SCL_DNR2_BASE;
            break;
        case REG_SCL_LDC_BASE:
            enType = EN_REG_SCL_LDC_BASE;
            break;
        case REG_SCL_LDC1_BASE:
            enType = EN_REG_SCL_LDC1_BASE;
            break;
        case REG_SCL_LDC2_BASE:
            enType = EN_REG_SCL_LDC2_BASE;
            break;
        case REG_SCL3_BASE:
            enType = EN_REG_SCL3_BASE;
            break;
        case REG_VIP_SCNR_BASE:
            enType = EN_REG_VIP_SCNR_BASE;
            break;
        case REG_VIP_WDR_BASE:
            enType = EN_REG_VIP_WDR_BASE;
            break;
        case REG_VIP_WDR1_BASE:
            enType = EN_REG_VIP_WDR1_BASE;
            break;
        case REG_VIP_YEE_BASE:
            enType = EN_REG_VIP_YEE_BASE;
            break;
        case REG_VIP_MCNR_BASE:
            enType = EN_REG_VIP_MCNR_BASE;
            break;
        case REG_VIP_NE_BASE:
            enType = EN_REG_VIP_NE_BASE;
            break;
        case REG_CMDQ_BASE:
            enType = EN_REG_CMDQ0_BASE;
            break;
        case REG_CMDQ1_BASE:
            enType = EN_REG_CMDQ1_BASE;
            break;
        case REG_CMDQ2_BASE:
            enType = EN_REG_CMDQ2_BASE;
            break;
        case REG_MDWIN1_BASE:
            enType = EN_REG_MDWIN1_BASE;
            break;
        case REG_MDWIN2_BASE:
            enType = EN_REG_MDWIN2_BASE;
            break;
        case REG_UCM1_BASE:
            enType = EN_REG_UCM1_BASE;
            break;
        case REG_UCM2_BASE:
            enType = EN_REG_UCM2_BASE;
            break;
        case REG_SCL_ARBSHP_BASE:
            enType = EN_REG_ARBSHP_BASE;
            break;
        case REG_SCL_MLOAD_BASE:
            enType = EN_REG_MLOAD_BASE;
            break;
        case REG_SCL_RSC_BASE:
            enType = EN_REG_RSC_BASE;
            break;
        case REG_SCL_DMAG0_BASE:
            enType = EN_REG_DMAG0_BASE;
            break;
        case REG_ISP_BASE:
            enType = EN_REG_ISP_BASE;
            break;
        case REG_SCL_LPLL_BASE:
            enType = EN_REG_SCL_LPLL_BASE;
            break;
        case REG_SCL_CLK_BASE:
            enType = EN_REG_SCL_CLK_BASE;
            break;
        case REG_BLOCK_BASE:
            enType = EN_REG_BLOCK_BASE;
            break;
        case REG_CHIPTOP_BASE:
            enType = EN_REG_CHIPTOP_BASE;
            break;
        default:
            enType = EN_REG_NUM_CONFIG;
            break;
    }
    return enType;
}
u32 _HalUtilitySwitchSclBankToType(u32 u32Bank)
{
    EN_REG_SCLALL_CONFIG_TYPE enType = EN_REG_SCLALL_SCL_HVSP0_BASE;
    switch(u32Bank)
    {
        case REG_SCL_HVSP0_BASE:
            enType = EN_REG_SCLALL_SCL_HVSP0_BASE;
            break;
        case REG_SCL_HVSP1_BASE:
            enType = EN_REG_SCLALL_SCL_HVSP1_BASE;
            break;
        case REG_SCL_HVSP2_BASE:
            enType = EN_REG_SCLALL_SCL_HVSP2_BASE;
            break;
        case REG_SCL_HVSP3_BASE:
            enType = EN_REG_SCLALL_SCL_HVSP3_BASE;
            break;
        case REG_SCL_DMA0_BASE:
            enType = EN_REG_SCLALL_SCL_DMA0_BASE;
            break;
        case REG_SCL_DMA1_BASE:
            enType = EN_REG_SCLALL_SCL_DMA1_BASE;
            break;
        case REG_SCL_DMA2_BASE:
            enType = EN_REG_SCLALL_SCL_DMA2_BASE;
            break;
        case REG_SCL_DMA3_BASE:
            enType = EN_REG_SCLALL_SCL_DMA3_BASE;
            break;
        case REG_SCL0_BASE:
            enType = EN_REG_SCLALL_SCL0_BASE;
            break;
        case REG_SCL1_BASE:
            enType = EN_REG_SCLALL_SCL1_BASE;
            break;
        case REG_SCL2_BASE:
            enType = EN_REG_SCLALL_SCL2_BASE;
            break;
        case REG_VIP_LCE_BASE:
            enType = EN_REG_SCLALL_VIP_LCE_BASE;
            break;
        case REG_SCL3_BASE:
            enType = EN_REG_SCLALL_SCL3_BASE;
            break;
        case REG_VIP_SCNR_BASE:
            enType = EN_REG_SCLALL_VIP_SCNR_BASE;
            break;
        case REG_VIP_WDR1_BASE:
            enType = EN_REG_SCLALL_VIP_WDR1_BASE;
            break;
        case REG_VIP_MCNR_BASE:
            enType = EN_REG_SCLALL_VIP_MCNR_BASE;
            break;
        case REG_MDWIN1_BASE:
            enType = EN_REG_SCLALL_MDWIN1_BASE;
            break;
        case REG_MDWIN2_BASE:
            enType = EN_REG_SCLALL_MDWIN2_BASE;
            break;
        case REG_UCM1_BASE:
            enType = EN_REG_SCLALL_UCM1_BASE;
            break;
        case REG_UCM2_BASE:
            enType = EN_REG_SCLALL_UCM2_BASE;
            break;
        case REG_SCL_MLOAD_BASE:
            enType = EN_REG_SCLALL_MLOAD_BASE;
            break;
        case REG_SCL_RSC_BASE:
            enType = EN_REG_SCLALL_RSC_BASE;
            break;
        case REG_VIP_NE_BASE:
            enType = EN_REG_SCLALL_VIP_NE_BASE;
            break;
        default:
            SCL_ERR("[BUG]%s %lx\n",__FUNCTION__,u32Bank);
            enType = 0xFF;
        break;
    }
    return enType;
}
u32 _HalUtilitySwitchVipBankToType(u32 u32Bank)
{
    EN_REG_SCLVIP_CONFIG_TYPE enType = EN_REG_SCLVIP_SCL_HVSP0_BASE;
    switch(u32Bank)
    {
        case REG_SCL_HVSP0_BASE:
            enType = EN_REG_SCLVIP_SCL_HVSP0_BASE;
            break;
        case REG_SCL_HVSP1_BASE:
            enType = EN_REG_SCLVIP_SCL_HVSP1_BASE;
            break;
        case REG_SCL_HVSP2_BASE:
            enType = EN_REG_SCLVIP_SCL_HVSP2_BASE;
            break;
        case REG_SCL_HVSP3_BASE:
            enType = EN_REG_SCLVIP_SCL_HVSP3_BASE;
            break;
        case REG_SCL_NLM0_BASE:
            enType = EN_REG_SCLVIP_SCL_NLM0_BASE;
            break;
        case REG_SCL0_BASE:
            enType = EN_REG_SCLVIP_SCL0_BASE;
            break;
        case REG_SCL1_BASE:
            enType = EN_REG_SCLVIP_SCL1_BASE;
            break;
        case REG_VIP_ACE3_BASE:
            enType = EN_REG_SCLVIP_VIP_ACE3_BASE;
            break;
        case REG_VIP_ACE_BASE:
            enType = EN_REG_SCLVIP_VIP_ACE_BASE;
            break;
        case REG_VIP_PK_BASE:
            enType = EN_REG_SCLVIP_VIP_PK_BASE;
            break;
        case REG_VIP_DLC_BASE:
            enType = EN_REG_SCLVIP_VIP_DLC_BASE;
            break;
        case REG_VIP_MWE_BASE:
            enType = EN_REG_SCLVIP_VIP_MWE_BASE;
            break;
        case REG_VIP_ACE2_BASE:
            enType = EN_REG_SCLVIP_VIP_ACE2_BASE;
            break;
        case REG_VIP_LCE_BASE:
            enType = EN_REG_SCLVIP_VIP_LCE_BASE;
            break;
        case REG_SCL_DNR2_BASE:
            enType = EN_REG_SCLVIP_SCL_DNR2_BASE;
            break;
        case REG_VIP_SCNR_BASE:
            enType = EN_REG_SCLVIP_VIP_SCNR_BASE;
            break;
        case REG_VIP_WDR_BASE:
            enType = EN_REG_SCLVIP_VIP_WDR_BASE;
            break;
        case REG_VIP_WDR1_BASE:
            enType = EN_REG_SCLVIP_VIP_WDR1_BASE;
            break;
        case REG_VIP_YEE_BASE:
            enType = EN_REG_SCLVIP_VIP_YEE_BASE;
            break;
        case REG_VIP_MCNR_BASE:
            enType = EN_REG_SCLVIP_VIP_MCNR_BASE;
            break;
        case REG_VIP_NE_BASE:
            enType = EN_REG_SCLVIP_VIP_NE_BASE;
            break;
        case REG_SCL_ARBSHP_BASE:
            enType = EN_REG_SCLVIP_ARBSHP_BASE;
            break;
        case REG_SCL_MLOAD_BASE:
            enType = EN_REG_SCLVIP_MLOAD_BASE;
            break;
        case REG_UCM1_BASE:
            enType = EN_REG_SCLVIP_UCM1_BASE;
            break;
        case REG_UCM2_BASE:
            enType = EN_REG_SCLVIP_UCM2_BASE;
            break;
        default:
            SCL_ERR("[BUG]%s %lx\n",__FUNCTION__,u32Bank);
            enType = 0xFF;
            break;
    }
    return enType;
}
u32 _HalUtilitySwitchDefaultTypeToBank(EN_REG_CONFIG_TYPE enType)
{
    u32 u32bankaddr;
    switch(enType)
    {
        case EN_REG_SCL_HVSP0_BASE:
            u32bankaddr = REG_SCL_HVSP0_BASE;
            break;
        case EN_REG_SCL_HVSP1_BASE:
            u32bankaddr = REG_SCL_HVSP1_BASE;
            break;
        case EN_REG_SCL_HVSP2_BASE:
            u32bankaddr = REG_SCL_HVSP2_BASE;
            break;
        case EN_REG_SCL_HVSP3_BASE:
            u32bankaddr = REG_SCL_HVSP3_BASE;
            break;
        case EN_REG_SCL_DMA0_BASE:
            u32bankaddr = REG_SCL_DMA0_BASE;
            break;
        case EN_REG_SCL_DMA1_BASE:
            u32bankaddr = REG_SCL_DMA1_BASE;
            break;
        case EN_REG_SCL_DMA2_BASE:
            u32bankaddr = REG_SCL_DMA2_BASE;
            break;
        case EN_REG_SCL_DMA3_BASE:
            u32bankaddr = REG_SCL_DMA3_BASE;
            break;
        case EN_REG_SCL_NLM0_BASE:
            u32bankaddr = REG_SCL_NLM0_BASE;
            break;
        case EN_REG_SCL0_BASE:
            u32bankaddr = REG_SCL0_BASE;
            break;
        case EN_REG_SCL1_BASE:
            u32bankaddr = REG_SCL1_BASE;
            break;
        case EN_REG_SCL2_BASE:
            u32bankaddr = REG_SCL2_BASE;
            break;
        case EN_REG_VIP_ACE3_BASE:
            u32bankaddr = REG_VIP_ACE3_BASE;
            break;
        case EN_REG_VIP_ACE_BASE:
            u32bankaddr = REG_VIP_ACE_BASE;
            break;
        case EN_REG_VIP_PK_BASE:
            u32bankaddr = REG_VIP_PK_BASE;
            break;
        case EN_REG_VIP_DLC_BASE:
            u32bankaddr = REG_VIP_DLC_BASE;
            break;
        case EN_REG_VIP_MWE_BASE:
            u32bankaddr = REG_VIP_MWE_BASE;
            break;
        case EN_REG_VIP_ACE2_BASE:
            u32bankaddr = REG_VIP_ACE2_BASE;
            break;
        case EN_REG_VIP_LCE_BASE:
            u32bankaddr = REG_VIP_LCE_BASE;
            break;
        case EN_REG_SCL_DNR1_BASE:
            u32bankaddr = REG_SCL_DNR1_BASE;
            break;
        case EN_REG_SCL_DNR2_BASE:
            u32bankaddr = REG_SCL_DNR2_BASE;
            break;
        case EN_REG_SCL_LDC_BASE:
            u32bankaddr = REG_SCL_LDC_BASE;
            break;
        case EN_REG_SCL_LDC1_BASE:
            u32bankaddr = REG_SCL_LDC1_BASE;
            break;
        case EN_REG_SCL_LDC2_BASE:
            u32bankaddr = REG_SCL_LDC2_BASE;
            break;
        case EN_REG_SCL3_BASE:
            u32bankaddr = REG_SCL3_BASE;
            break;
        case EN_REG_VIP_SCNR_BASE:
            u32bankaddr = REG_VIP_SCNR_BASE;
            break;
        case EN_REG_VIP_WDR_BASE:
            u32bankaddr = REG_VIP_WDR_BASE;
            break;
        case EN_REG_VIP_WDR1_BASE:
            u32bankaddr = REG_VIP_WDR1_BASE;
            break;
        case EN_REG_VIP_YEE_BASE:
            u32bankaddr = REG_VIP_YEE_BASE;
            break;
        case EN_REG_VIP_MCNR_BASE:
            u32bankaddr = REG_VIP_MCNR_BASE;
            break;
        case EN_REG_VIP_NE_BASE:
            u32bankaddr = REG_VIP_NE_BASE;
            break;
        case EN_REG_CMDQ0_BASE:
            u32bankaddr = REG_CMDQ_BASE;
            break;
        case EN_REG_CMDQ1_BASE:
            u32bankaddr = REG_CMDQ1_BASE;
            break;
        case EN_REG_CMDQ2_BASE:
            u32bankaddr = REG_CMDQ2_BASE;
            break;
        case EN_REG_MDWIN1_BASE:
            u32bankaddr = REG_MDWIN1_BASE;
            break;
        case EN_REG_MDWIN2_BASE:
            u32bankaddr = REG_MDWIN2_BASE;
            break;
        case EN_REG_UCM1_BASE:
            u32bankaddr = REG_UCM1_BASE;
            break;
        case EN_REG_UCM2_BASE:
            u32bankaddr = REG_UCM2_BASE;
            break;
        case EN_REG_RSC_BASE:
            u32bankaddr = REG_SCL_RSC_BASE;
            break;
        case EN_REG_DMAG0_BASE:
            u32bankaddr = REG_SCL_DMAG0_BASE;
            break;
        case EN_REG_ARBSHP_BASE:
            u32bankaddr = REG_SCL_ARBSHP_BASE;
            break;
        case EN_REG_MLOAD_BASE:
            u32bankaddr = REG_SCL_MLOAD_BASE;
            break;
        default:
            u32bankaddr = 0;
            break;
    }
    return u32bankaddr;
}
u32 _HalUtilitySwitchSclTypeToBank(EN_REG_SCLALL_CONFIG_TYPE enType)
{
    u32 u32bankaddr;
    switch(enType)
    {
        case EN_REG_SCLALL_SCL_HVSP0_BASE:
            u32bankaddr = REG_SCL_HVSP0_BASE;
            break;
        case EN_REG_SCLALL_SCL_HVSP1_BASE:
            u32bankaddr = REG_SCL_HVSP1_BASE;
            break;
        case EN_REG_SCLALL_SCL_HVSP2_BASE:
            u32bankaddr = REG_SCL_HVSP2_BASE;
            break;
        case EN_REG_SCLALL_SCL_HVSP3_BASE:
            u32bankaddr = REG_SCL_HVSP3_BASE;
            break;
        case EN_REG_SCLALL_SCL_DMA0_BASE:
            u32bankaddr = REG_SCL_DMA0_BASE;
            break;
        case EN_REG_SCLALL_SCL_DMA1_BASE:
            u32bankaddr = REG_SCL_DMA1_BASE;
            break;
        case EN_REG_SCLALL_SCL_DMA2_BASE:
            u32bankaddr = REG_SCL_DMA2_BASE;
            break;
        case EN_REG_SCLALL_SCL_DMA3_BASE:
            u32bankaddr = REG_SCL_DMA3_BASE;
            break;
        case EN_REG_SCLALL_SCL0_BASE:
            u32bankaddr = REG_SCL0_BASE;
            break;
        case EN_REG_SCLALL_SCL1_BASE:
            u32bankaddr = REG_SCL1_BASE;
            break;
        case EN_REG_SCLALL_SCL2_BASE:
            u32bankaddr = REG_SCL2_BASE;
            break;
        case EN_REG_SCLALL_VIP_LCE_BASE:
            u32bankaddr = REG_VIP_LCE_BASE;
            break;
        case EN_REG_SCLALL_SCL3_BASE:
            u32bankaddr = REG_SCL3_BASE;
            break;
        case EN_REG_SCLALL_VIP_SCNR_BASE:
            u32bankaddr = REG_VIP_SCNR_BASE;
            break;
        case EN_REG_SCLALL_VIP_WDR1_BASE:
            u32bankaddr = REG_VIP_WDR1_BASE;
            break;
        case EN_REG_SCLALL_VIP_MCNR_BASE:
            u32bankaddr = REG_VIP_MCNR_BASE;
            break;
        case EN_REG_SCLALL_MDWIN1_BASE:
            u32bankaddr = REG_MDWIN1_BASE;
            break;
        case EN_REG_SCLALL_MDWIN2_BASE:
            u32bankaddr = REG_MDWIN2_BASE;
            break;
        case EN_REG_SCLALL_UCM1_BASE:
            u32bankaddr = REG_UCM1_BASE;
            break;
        case EN_REG_SCLALL_UCM2_BASE:
            u32bankaddr = REG_UCM2_BASE;
            break;
        case EN_REG_SCLALL_RSC_BASE:
            u32bankaddr = REG_SCL_RSC_BASE;
            break;
        case EN_REG_SCLALL_MLOAD_BASE:
            u32bankaddr = REG_SCL_MLOAD_BASE;
            break;
        case EN_REG_SCLALL_VIP_NE_BASE:
            u32bankaddr = REG_VIP_NE_BASE;
            break;
        default:
            u32bankaddr = 0;
            break;
    }
    return u32bankaddr;
}
u32 _HalUtilitySwitchVipTypeToBank(EN_REG_SCLVIP_CONFIG_TYPE enType)
{
    u32 u32bankaddr;
    switch(enType)
    {
        case EN_REG_SCLVIP_SCL_HVSP0_BASE:
            u32bankaddr = REG_SCL_HVSP0_BASE;
            break;
        case EN_REG_SCLVIP_SCL_HVSP1_BASE:
            u32bankaddr = REG_SCL_HVSP1_BASE;
            break;
        case EN_REG_SCLVIP_SCL_HVSP2_BASE:
            u32bankaddr = REG_SCL_HVSP2_BASE;
            break;
        case EN_REG_SCLVIP_SCL_HVSP3_BASE:
            u32bankaddr = REG_SCL_HVSP3_BASE;
            break;
        case EN_REG_SCLVIP_SCL_NLM0_BASE:
            u32bankaddr = REG_SCL_NLM0_BASE;
            break;
        case EN_REG_SCLVIP_SCL0_BASE:
            u32bankaddr = REG_SCL0_BASE;
            break;
        case EN_REG_SCLVIP_SCL1_BASE:
            u32bankaddr = REG_SCL1_BASE;
            break;
        case EN_REG_SCLVIP_VIP_ACE3_BASE:
            u32bankaddr = REG_VIP_ACE3_BASE;
            break;
        case EN_REG_SCLVIP_VIP_ACE_BASE:
            u32bankaddr = REG_VIP_ACE_BASE;
            break;
        case EN_REG_SCLVIP_VIP_PK_BASE:
            u32bankaddr = REG_VIP_PK_BASE;
            break;
        case EN_REG_SCLVIP_VIP_DLC_BASE:
            u32bankaddr = REG_VIP_DLC_BASE;
            break;
        case EN_REG_SCLVIP_VIP_MWE_BASE:
            u32bankaddr = REG_VIP_MWE_BASE;
            break;
        case EN_REG_SCLVIP_VIP_ACE2_BASE:
            u32bankaddr = REG_VIP_ACE2_BASE;
            break;
        case EN_REG_SCLVIP_VIP_LCE_BASE:
            u32bankaddr = REG_VIP_LCE_BASE;
            break;
        case EN_REG_SCLVIP_SCL_DNR2_BASE:
            u32bankaddr = REG_SCL_DNR2_BASE;
            break;
        case EN_REG_SCLVIP_VIP_SCNR_BASE:
            u32bankaddr = REG_VIP_SCNR_BASE;
            break;
        case EN_REG_SCLVIP_VIP_WDR_BASE:
            u32bankaddr = REG_VIP_WDR_BASE;
            break;
        case EN_REG_SCLVIP_VIP_WDR1_BASE:
            u32bankaddr = REG_VIP_WDR1_BASE;
            break;
        case EN_REG_SCLVIP_VIP_YEE_BASE:
            u32bankaddr = REG_VIP_YEE_BASE;
            break;
        case EN_REG_SCLVIP_VIP_MCNR_BASE:
            u32bankaddr = REG_VIP_MCNR_BASE;
            break;
        case EN_REG_SCLVIP_VIP_NE_BASE:
            u32bankaddr = REG_VIP_NE_BASE;
            break;
        case EN_REG_SCLVIP_ARBSHP_BASE:
            u32bankaddr = REG_SCL_ARBSHP_BASE;
            break;
        case EN_REG_SCLVIP_MLOAD_BASE:
            u32bankaddr = REG_SCL_MLOAD_BASE;
            break;
        case EN_REG_SCLVIP_UCM1_BASE:
            u32bankaddr = REG_UCM1_BASE;
            break;
        case EN_REG_SCLVIP_UCM2_BASE:
            u32bankaddr = REG_UCM2_BASE;
            break;
        default:
            u32bankaddr = 0;
            break;
    }
    return u32bankaddr;
}
u32 _HalUtilitySwitchBankToType(u32 u32Bank)
{
    u32 enType = EN_REG_SCL_HVSP0_BASE;
    if(genUtiId == E_HAL_SCL_UTI_ID_SC_DEFAULT)
    {
        enType = _HalUtilitySwitchBankDefaultToType(u32Bank);
    }
    else if(genUtiId == E_HAL_SCL_UTI_ID_SC_ALL)
    {
        enType = _HalUtilitySwitchSclBankToType(u32Bank);
    }
    else if(genUtiId == E_HAL_SCL_UTI_ID_SC_VIP)
    {
        enType = _HalUtilitySwitchVipBankToType(u32Bank);
    }
    else
    {
        SCL_ERR("[BUG]%s %lx\n",__FUNCTION__,u32Bank);
        SCLOS_BUG();
    }
    return enType;
}
u32 _HalUtilitySwitchTypeToBank(u32 enType)
{
    u32 u32bankaddr = REG_SCL_HVSP0_BASE;
    if(genUtiId == E_HAL_SCL_UTI_ID_SC_DEFAULT)
    {
        u32bankaddr = _HalUtilitySwitchDefaultTypeToBank(enType);
    }
    else if(genUtiId == E_HAL_SCL_UTI_ID_SC_ALL)
    {
        u32bankaddr = _HalUtilitySwitchSclTypeToBank(enType);
    }
    else if(genUtiId == E_HAL_SCL_UTI_ID_SC_VIP)
    {
        u32bankaddr = _HalUtilitySwitchVipTypeToBank(enType);
    }
    else
    {
        SCL_ERR("[BUG]%s enType:%ld\n",__FUNCTION__,enType);
        SCLOS_BUG();
    }
    return u32bankaddr;
}
void _HalUtilityReadBufferToDebug(EN_REG_CONFIG_TYPE enType)
{
    u16 *p16buffer = NULL;
    u16 u16idx;
    if(gpvRegBuffer)
    {
        SCL_ERR("enType:%d\n",enType);
        // data
        // 4byte (2byte val+2byte 0)
        // 1bank
        // 128 addr 512byte
        p16buffer = (u16 *)(gpvRegBuffer +(BANKSIZE*enType));
        for(u16idx=0;u16idx<(BANKSIZE/4);u16idx+=8)
        {
            SCL_ERR("addr:%2hx  %04hx %04hx %04hx %04hx %04hx %04hx %04hx %04hx\n",u16idx
                ,*(p16buffer+u16idx*2),*(p16buffer+(u16idx+1)*2),*(p16buffer+(u16idx+2)*2),*(p16buffer+(u16idx+3)*2)
                ,*(p16buffer+(u16idx+4)*2),*(p16buffer+(u16idx+5)*2),*(p16buffer+(u16idx+6)*2),*(p16buffer+(u16idx+7)*2));
        }
    }
}
void _HalUtilityReadBankCpyToBuffer(EN_REG_CONFIG_TYPE enType)
{
    u32 u32Bank;
    void *pvBankaddr;
    if(gpvRegBuffer)
    {
        u32Bank = _HalUtilitySwitchTypeToBank(enType);
        pvBankaddr = RIU_GET_ADDR(u32Bank);
        DrvUtilityMutexLock();
        DrvSclOsMemcpy((void *)(gpvRegBuffer +(BANKSIZE*enType)),(void *)pvBankaddr,BANKSIZE);
        DrvUtilityMutexUNLock();
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_INIT, "[SCLUTILITY]%s(%d)::%d:%lx, @(%lx)\n",
            __FUNCTION__, __LINE__,enType,u32Bank,(u32)pvBankaddr);
    }
}
void _HalUtilityReadBufferCpyToBank(EN_REG_CONFIG_TYPE enType)
{
    u32 u32Bank;
    void *pvBankaddr;
    if(gpvRegBuffer)
    {
        u32Bank = _HalUtilitySwitchTypeToBank(enType);
        pvBankaddr = RIU_GET_ADDR(u32Bank);
        DrvUtilityMutexLock();
        DrvSclOsMemcpy((void *)pvBankaddr,(void *)(gpvRegBuffer +(BANKSIZE*enType)),BANKSIZE);
        DrvUtilityMutexUNLock();
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_INIT, "[SCLUTILITY]%s(%d)::%d:%lx, @(%lx)\n",
            __FUNCTION__, __LINE__,enType,u32Bank,(u32)pvBankaddr);
    }
}
void _HalUtilityDeInitRegBuffer(void)
{
    u16 u16idx;
    for(u16idx = 0;u16idx<EN_REG_NUM_CONFIG;u16idx++)
    {
        _HalUtilityReadBufferCpyToBank(u16idx);
    }
}

void _HalUtilityInitRegBuffer(void)
{
    u16 u16idx;
    for(u16idx = 0;u16idx<EN_REG_NUM_CONFIG;u16idx++)
    {
        _HalUtilityReadBankCpyToBuffer(u16idx);
    }
}
u32 _HalUtilityGetMax(void)
{
    if(genUtiId == E_HAL_SCL_UTI_ID_SC_DEFAULT)
    {
        return EN_REG_NUM_CONFIG;
    }
    else if(genUtiId == E_HAL_SCL_UTI_ID_SC_ALL)
    {
        return EN_REG_SCLALL_NUM_CONFIG;
    }
    else if(genUtiId == E_HAL_SCL_UTI_ID_SC_VIP)
    {
        return EN_REG_SCLVIP_NUM_CONFIG;
    }
    else
    {
        SCL_ERR("[BUG]%s Id:%d\n",__FUNCTION__,genUtiId);
        SCLOS_BUG();
	    return EN_REG_SCL_HVSP0_BASE;
    }
}
bool _HalUtilitySetCmdqBufferIdx(u16 u16Val,u32 u32Reg,EN_REG_CONFIG_TYPE enType,u32 u32TypeMax)
{
    u32 u32Bank;
    u16 u16Addr;
    u16 *pvBankaddr;
    u32Bank = (u32Reg&0xFFFF00);
    u16Addr = (u16)(u32Reg&0xFE); //only low byte
    if(enType< u32TypeMax &&gpvRegBuffer)
    {
        pvBankaddr = (u16 *)(gpvRegBuffer + (BANKSIZE*enType) + (u16Addr<<1) +2);
        *pvBankaddr = (u16Val|FLAG_CMDQ_IDXEXIST);
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_HIGH, "[SCLUTILITY]CMDQ 2ByteReg:%lx pvBankaddr:%lx val:%hx\n",
            u32Reg,(u32)pvBankaddr,*pvBankaddr);
        return 1;
    }
    else
    {
        return 0;
    }
}
bool _HalUtilityGetCmdqBufferIdx(u16 *u16Val,u32 u32Reg,EN_REG_CONFIG_TYPE enType,u32 u32TypeMax)
{
    u32 u32Bank;
    u16 u16Addr;
    u16 *pvBankaddr;
    u32Bank = (u32Reg&0xFFFF00);
    u16Addr = (u16)(u32Reg&0xFE); //only low byte
    if(enType< u32TypeMax &&gpvRegBuffer)
    {
        pvBankaddr = (u16 *)(gpvRegBuffer + (BANKSIZE*enType) + (u16Addr<<1) +2);
        *u16Val = ((*pvBankaddr)& (~FLAG_CMDQ_IDXEXIST));
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_HIGH, "[SCLUTILITY]CMDQ 2ByteReg:%lx pvBankaddr:%lx val:%hx\n",
            u32Reg,(u32)pvBankaddr,*pvBankaddr);
        if((*pvBankaddr) & FLAG_CMDQ_IDXEXIST)
        {
            SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_LOW, "[SCLUTILITY]CMDQ 2ByteReg:%lx val:%hx already exist CMDQ\n",
                u32Reg,(u16)*u16Val);
            return 0;
        }
        else
        {
            SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_LOW, "[SCLUTILITY]CMDQ 2ByteReg:%lx val:%hx not ready exist CMDQ\n",
                u32Reg,(u16)*u16Val);
            return 1;
        }
    }
    else
    {
        return 1;
    }
}
u16 _HalUtilityWrite2ByteRegBuffer(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    EN_REG_CONFIG_TYPE enType;
    u32 u32TypeMax;
    u32 u32Bank;
    u16 u16Addr;
    u16 *pvBankaddr;
    u32Bank = (u32Reg&0xFFFF00);
    u16Addr = (u16)(u32Reg&0xFF);
    enType = _HalUtilitySwitchBankToType(u32Bank);
    u32TypeMax = _HalUtilityGetMax();
    if(enType< u32TypeMax &&gpvRegBuffer)
    {
        pvBankaddr = (u16 *)(gpvRegBuffer + (BANKSIZE*enType) + (u16Addr<<1));
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_LOW, "[SCLUTILITY]2ByteReg:%lx val:%hx mask:%hx pvBankaddr:%lx val:%hx\n",
            u32Reg,u16Val,u16Mask,(u32)pvBankaddr,*pvBankaddr);
        //DrvUtilityMutexLock();
        *pvBankaddr = (u16)((u16Val&u16Mask)|(*pvBankaddr&(~u16Mask)));
        //DrvUtilityMutexUNLock();
        return 1;
    }
    else
    {
        SCL_DBGERR("[SCLUTILITY]gpvRegBuffer:%lx genUtiId:%d\n",(u32)gpvRegBuffer,genUtiId);
        SCL_DBGERR("[SCLUTILITY]enType %d Over 2ByteReg:%lx val:%hx mask:%hx max:%lu\n",enType,u32Reg,u16Val,u16Mask,u32TypeMax);
        return 0;
    }
}
u16 _HalUtilityReadRegBuffer(u32 u32Reg,EN_REG_CONFIG_TYPE enType,u32 u32TypeMax)
{
    u32 u32Bank;
    u16 u16Addr;
    u16 *pvBankaddr = NULL;
    if(gpvRegBuffer)
    {
        u32Bank = (u32Reg&0xFFFF00);
        u16Addr = (u16)(u32Reg&0xFF);
        if(enType==0 && u32TypeMax==0)
        {
            enType = _HalUtilitySwitchBankToType(u32Bank);
            u32TypeMax = _HalUtilityGetMax();
        }
        //DrvUtilityMutexLock();
        if(enType<u32TypeMax)
        {
            pvBankaddr = (u16 *)(gpvRegBuffer + (BANKSIZE*enType) + (u16Addr<<1));
            //DrvUtilityMutexUNLock();
            return *(pvBankaddr);
        }
        else
        {
            SCL_ERR("[BUG]%s Id:%d\n",__FUNCTION__,genUtiId);
            return 0;
        }
    }
    else
    {
        return 0;
    }
}
u16 _HalUtilityWrite2ByteInquireBuffer(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    EN_REG_CONFIG_TYPE enType;
    u16 u16BufferCnt;
    HalUtilityCmdReg_t *pvBankaddr;
    bool bNewBufferCnt;
    bool bHByte = 0;
    u32 u32Bank;
    u32 u32LocalReg;
    u32 u32TypeMax;
    u32LocalReg = (u32Reg&0xFFFFFE); // for 1Byte ignore Hbyte
    bHByte = (u32Reg&0x1);
    u32Bank = (u32Reg&0xFFFF00);
    enType = _HalUtilitySwitchBankToType(u32Bank);
    u32TypeMax = _HalUtilityGetMax();
    bNewBufferCnt = _HalUtilityGetCmdqBufferIdx(&u16BufferCnt,u32LocalReg,enType,u32TypeMax);
    if(enType< u32TypeMax &&gpvInquireBuffer)
    {
        if(*gu32TblCnt>MDRV_SCL_CTX_CMDQ_BUFFER_CNT)
        {
            SCL_ERR("[SCLUTILITY]InquireBuffer Full\n");
            return 0;
        }
        if(bNewBufferCnt)
        {
            SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_HIGH, "[SCLUTILITY]%s:not ready exist CMDQ ,so add Cnt:%lu Reg:%lx val:%hx Msk:%hx @%lx\n",
                __FUNCTION__,(*gu32TblCnt),u32LocalReg,u16Val,u16Mask,(u32)gpvInquireBuffer);
            _HalUtilitySetCmdqBufferIdx((*gu32TblCnt),u32LocalReg,enType,u32TypeMax);
            pvBankaddr = (HalUtilityCmdReg_t *)(gpvInquireBuffer + ((*gu32TblCnt)*SIZE_OF_CMDREG));
            pvBankaddr->u32Addr = u32LocalReg;
            pvBankaddr->u16Mask = u16Mask<<((bHByte)? 8 : 0);
            (*gu32TblCnt)++;
        }
        else
        {
            pvBankaddr = (HalUtilityCmdReg_t *)(gpvInquireBuffer + (u16BufferCnt*SIZE_OF_CMDREG));
            if(pvBankaddr->u32Addr != u32LocalReg)
            {
                // if has only one not to create inst ,may will in this condition.
                SCL_ERR("[SCLUTILITY]Inquire & Reg NOT MATCH :%lx,%lx\n",u32LocalReg,pvBankaddr->u32Addr);
                _HalUtilitySetCmdqBufferIdx((*gu32TblCnt),u32LocalReg,enType,u32TypeMax);
                pvBankaddr = (HalUtilityCmdReg_t *)(gpvInquireBuffer + ((*gu32TblCnt)*SIZE_OF_CMDREG));
                pvBankaddr->u32Addr = u32LocalReg;
                pvBankaddr->u16Mask = u16Mask<<((bHByte)? 8 : 0);
                SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_HIGH, "[SCLUTILITY]%s:not ready exist CMDQ ,so add Cnt:%lu Reg:%lx val:%hx Msk:%hx \n",
                    __FUNCTION__,(*gu32TblCnt),u32LocalReg,u16Val,u16Mask);
                (*gu32TblCnt)++;
            }
            else
            {
                pvBankaddr->u16Mask = (pvBankaddr->u16Mask|(u16Mask<<((bHByte)? 8 : 0)));
                SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_HIGH, "[SCLUTILITY]%s:%lx val:%hx already exist CMDQ Cnt:%hu\n",
                    __FUNCTION__,u32Reg,u16Val,u16BufferCnt);
            }
        }
        pvBankaddr->u16Data = _HalUtilityReadRegBuffer(u32LocalReg,enType,u32TypeMax);
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_HIGH, "[SCLUTILITY]%s:Real Reg:%lx val:%hx\n",
            __FUNCTION__,pvBankaddr->u32Addr,pvBankaddr->u16Data);
        return pvBankaddr->u16Data;
    }
    else
    {
        SCL_ERR("[SCLUTILITY]enType Not true 2ByteInquire:%lx val:%hx mask:%hx \n",u32Reg,u16Val,u16Mask);
        SCLOS_BUG();
        return 0;
    }
}
u16 _HalUtilityWrite2ByteInquireBufferWithoutRegBuf(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    HalUtilityCmdReg_t *pvBankaddr;
    bool bHByte = 0;
    u32 u32LocalReg;
    u32LocalReg = (u32Reg&0xFFFFFE); // for 1Byte ignore Hbyte
    bHByte = (u32Reg&0x1);
    if(*gu32TblCnt>MDRV_SCL_CTX_CMDQ_BUFFER_CNT)
    {
        SCL_ERR("[SCLUTILITY]InquireBuffer Full WithoutRegBuf\n");
        return 0;
    }
    SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_HIGH, "[SCLUTILITY]%s:Exception add Cnt:%lu Reg:%lx val:%hx Msk:%hx @%lx\n",
        __FUNCTION__,(*gu32TblCnt),u32LocalReg,u16Val,u16Mask,(u32)gpvInquireBuffer);
    pvBankaddr = (HalUtilityCmdReg_t *)(gpvInquireBuffer + ((*gu32TblCnt)*SIZE_OF_CMDREG));
    pvBankaddr->u32Addr = u32LocalReg;
    pvBankaddr->u16Mask = u16Mask<<((bHByte)? 8 : 0);
    pvBankaddr->u16Data = (u16Val&u16Mask)<<((bHByte)? 8 : 0);
    (*gu32TblCnt)++;
    SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_LOW, "[SCLUTILITY]%s:Real Reg:%lx val:%hx\n",
        __FUNCTION__,pvBankaddr->u32Addr,pvBankaddr->u16Data);
    return pvBankaddr->u16Data;
}
u16 _HalUtilityWrite2ByteTblBuffer(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    HalUtilityCmdReg_t *pvBankaddr;
    u32 u32LocalReg;
    u32LocalReg = (u32Reg&0xFFFFFE); // for 1Byte ignore Hbyte
    if(gpvWdrTblBuffer)
    {
        if(*gp32WdrTblCnt>MDRV_SCL_CTX_CMDQ_BUFFER_CNT)
        {
            SCL_ERR("[SCLUTILITY]TblBuffer Full\n");
            return 0;
        }
        pvBankaddr = (HalUtilityCmdReg_t *)(gpvWdrTblBuffer + ((*gp32WdrTblCnt)*SIZE_OF_CMDREG));
        pvBankaddr->u32Addr = u32LocalReg;
        pvBankaddr->u16Data = u16Val;
        pvBankaddr->u16Mask = 0xFFFF;
        (*gp32WdrTblCnt)++;
        return 1;
    }
    else
    {
        return 0;
    }
}
u8 _HalUtilityWriteByteRegBuffer(u32 u32Reg,u8 u8Val,u8 u8Mask)
{
    EN_REG_CONFIG_TYPE enType;
    u32 u32Bank;
    u16 u16Addr;
    u8 *pvBankaddr;
    u32 u32TypeMax;
    u32Bank = (u32Reg&0xFFFF00);
    u16Addr = (u16)(u32Reg&0xFF);
    enType = _HalUtilitySwitchBankToType(u32Bank);
    u32TypeMax = _HalUtilityGetMax();
    if(enType< u32TypeMax  && gpvRegBuffer)
    {
        //(((u32Reg)<<1) - ((u32Reg) & 1))
        pvBankaddr = (u8 *)(gpvRegBuffer + (BANKSIZE*enType) + ((u16Addr<<1)-((u16Addr) & 1)));
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_LOW, "[SCLUTILITY]ByteReg:%lx val:%hhx mask:%hhx pvBankaddr:%lx val:%hx\n",
            u32Reg,u8Val,u8Mask,(u32)pvBankaddr,(u8)*pvBankaddr);
        //DrvUtilityMutexLock();
        *pvBankaddr = (u8)((u8Val&u8Mask)|(*pvBankaddr&(~u8Mask)));
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_HIGH, "[SCLUTILITY]ByteRegval:%hhx\n",
            (u8)*pvBankaddr);
        //DrvUtilityMutexUNLock();
        return 1;
    }
    else
    {
        return 0;
    }
}
void HalUtilityDumpRegSettingAll(u32 u32RegAddr)
{
    EN_REG_CONFIG_TYPE enType;
    enType = _HalUtilitySwitchBankToType(u32RegAddr&0xFFFF00);
    if(enType != 0xFF)
    {
        _HalUtilityReadBufferToDebug(enType);
    }
}
u32 HalUtilityR4BYTEDirect(u32 u32Reg)
{
    u32 u32RetVal;
    u32RetVal = R4BYTE(u32Reg);
    return u32RetVal;
}
u16 HalUtilityR2BYTEDirect(u32 u32Reg)
{
    u16 u16RetVal;
    u16RetVal = R2BYTE(u32Reg);
    return u16RetVal;
}
u16 HalUtilityR2BYTEMaskDirect(u32 u32Reg,u16 u16Mask)
{
    u16 u16RetVal;
    u16RetVal = R2BYTEMSK(u32Reg,u16Mask);
    return u16RetVal;
}

#if USE_Utility
void _HalUtilityCreateSclInst(void *pvRegBuffer)
{
    //cpy Reg buffer
    EN_REG_SCLALL_CONFIG_TYPE enType;
    EN_REG_CONFIG_TYPE enTotalType;
    u32 u32bankaddr;
    for(enType = 0 ;enType<EN_REG_SCLALL_NUM_CONFIG;enType++)
    {
        u32bankaddr = _HalUtilitySwitchSclTypeToBank(enType);
        enTotalType = _HalUtilitySwitchBankDefaultToType(u32bankaddr);
        DrvSclOsMemcpy(pvRegBuffer+enType*MDRV_SCL_CTX_REG_BANK_SIZE,
            gpvDefaultRegBuffer+enTotalType*MDRV_SCL_CTX_REG_BANK_SIZE,(MDRV_SCL_CTX_REG_BANK_SIZE));
    }
}
void _HalUtilityCreateVipInst(void *pvRegBuffer)
{
    //cpy Reg buffer
    EN_REG_SCLVIP_CONFIG_TYPE enType;
    u32 u32bankaddr;
    EN_REG_CONFIG_TYPE enTotalType;
    for(enType = 0 ;enType<EN_REG_SCLVIP_NUM_CONFIG;enType++)
    {
        u32bankaddr = _HalUtilitySwitchVipTypeToBank(enType);
        enTotalType = _HalUtilitySwitchBankDefaultToType(u32bankaddr);
        DrvSclOsMemcpy(pvRegBuffer+enType*MDRV_SCL_CTX_REG_BANK_SIZE,
            gpvDefaultRegBuffer+enTotalType*MDRV_SCL_CTX_REG_BANK_SIZE,(MDRV_SCL_CTX_REG_BANK_SIZE));
    }
}
void HalUtilityCreateInst(HalUtilityIdType_e enId,void *pvRegBuffer)
{
    //cpy Reg buffer
    //To Do :by VIP/SC 2 inst
    if(enId==E_HAL_SCL_UTI_ID_SC_ALL)
    {
        _HalUtilityCreateSclInst(pvRegBuffer);
    }
    else if(enId==E_HAL_SCL_UTI_ID_SC_VIP)
    {
        _HalUtilityCreateVipInst(pvRegBuffer);
    }
    else if(enId==E_HAL_SCL_UTI_ID_SC_DEFAULT)
    {
        DrvSclOsMemcpy(pvRegBuffer,gpvDefaultRegBuffer,(MDRV_SCL_CTX_REG_BANK_NUM * MDRV_SCL_CTX_REG_BANK_SIZE));
    }
    else
    {
        SCL_ERR("[BUG]%s %d\n",__FUNCTION__,enId);
        SCLOS_BUG();
    }
}
void HalUtilityFreeMloadHandler(void *pvMloadhandler)
{
#if defined(SCLOS_TYPE_MLOAD)
    if(pvMloadhandler)
    {
        IspMLoadDeInit(pvMloadhandler);
    }
#endif
}
void HalUtilityGetMloadHandler(void **pvMloadhandler)
{
#if defined(SCLOS_TYPE_MLOAD)
    MLOAD_ATTR attr;
    if(*pvMloadhandler==NULL)
    {
        attr.mode = (gpstCmdq[E_HAL_SCL_UTI_CMDQID_SC_ALL].MHAL_CMDQ_WriteRegCmdqMask) ? CMDQ_MODE : RIU_MODE;
        attr.pCmqInterface_t = &gpstCmdq[E_HAL_SCL_UTI_CMDQID_SC_ALL];
        *pvMloadhandler = IspMLoadInit(attr);
    }
#endif
}
u16 _HalUtilitySetMloadAmount(u32 u32Id)
{
    u16 u16bank1;
    switch(u32Id)
    {
        case 33 ... 36:
        case 49 ... 60:
            u16bank1 = (512/8)-1;
            break;
        case 43:
            u16bank1 = (2048/8)-1;
            break;
        case 39:
        case 41:
        case 42:
            u16bank1 = 255;
            break;
        case 40:
            u16bank1 = 127;
            break;
        case 37 ... 38:
            u16bank1 = 288;
            break;
    }
    return u16bank1;
}
void HalUtilitySetMloadTbl(u32 u32Id ,void *pvMloadHandler,void *pvBuf)
{
#if defined(SCLOS_TYPE_MLOAD)
    IspMLoadTableSet(pvMloadHandler,u32Id,pvBuf);
#else
    u32 u32Addr = ((u32)pvBuf>>4);
    u16 u16bankB = 0xa300;
    u16 u16bank1 = 0x0;
    u16bankB |= (u32Id==39) ? 0x0 : (u32Id==40) ? 0x1 :0x2;
    u16bank1 = _HalUtilitySetMloadAmount(u32Id);
    if(gpCmdqF)
    {
        gpCmdqF(REG_SCL0_13_L,0,0x1000);//mload irq
        gpCmdqF(REG_SCL_MLOAD_0B_L,u16bankB,0xFFFF);//B
        gpCmdqF(REG_SCL_MLOAD_00_L,u32Id-MLOAD_ID_BASE,0x1F);//0
        gpCmdqF(REG_SCL_MLOAD_03_L,(u32Addr&0xFFFF),0xFFFF);// 3
        gpCmdqF(REG_SCL_MLOAD_04_L,((u32Addr>>16)),0xFFFF);// 4
        gpCmdqF(REG_SCL_MLOAD_01_L,u16bank1,0xFFFF);// 1
        gpCmdqF(REG_SCL_MLOAD_02_L,0x0000,0xFFFF);// 2
        gpCmdqF(REG_SCL_MLOAD_09_L,0x1,0x1);// 9
    }
    //SCL_ERR("[%s]Use Hard code to set mload %lu\n",__FUNCTION__,u32Id);
#endif
}
void HalUtilityMloadFire(void *pvMloadHandler)
{
#if defined(SCLOS_TYPE_MLOAD)
    SclMLoadApply(pvMloadHandler);
#else
    if(gpCmdqF)
    {
        gpCmdqF(REG_SCL_MLOAD_09_L,0x2,0x2);// 9
        HalUtilitySetWaitMload();
    }
#endif
}
void HalUtilityTriggerM2M(void *pvHandler)
{
    HalUtilityW2BYTEMSKDirectCmdq(REG_SCL0_22_L,BIT2,BIT2);
}
void HalUtilityW2BYTEMSKDirectCmdqSCL(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    if((DrvSclOsGetAccessRegMode() ==  E_DRV_SCLOS_AccessReg_CMDQ)&& gpstCmdq[E_HAL_SCL_UTI_CMDQID_SC_ALL].MHAL_CMDQ_WriteRegCmdqMask)
    {
        //_HalUtilityWrite2ByteRegBuffer(u32Reg, u16Val, u16Mask);
        gpstCmdq[E_HAL_SCL_UTI_CMDQID_SC_ALL].MHAL_CMDQ_WriteRegCmdqMask(&gpstCmdq[E_HAL_SCL_UTI_CMDQID_SC_ALL],u32Reg,u16Val,u16Mask);
    }
    else
    {
        if(genUtiId == E_HAL_SCL_UTI_ID_SC_DEFAULT)
        _HalUtilityWrite2ByteRegBuffer(u32Reg, u16Val, u16Mask);
        W2BYTEMSK(u32Reg,u16Val,u16Mask);
    }
}
void HalUtilityW2BYTEMSKDirectCmdqM2M(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    if((DrvSclOsGetAccessRegMode() ==  E_DRV_SCLOS_AccessReg_CMDQ)&& gpstCmdq[E_HAL_SCL_UTI_CMDQID_RSC].MHAL_CMDQ_WriteRegCmdqMask)
    {
        //_HalUtilityWrite2ByteRegBuffer(u32Reg, u16Val, u16Mask);
        gpstCmdq[E_HAL_SCL_UTI_CMDQID_RSC].MHAL_CMDQ_WriteRegCmdqMask(&gpstCmdq[E_HAL_SCL_UTI_CMDQID_RSC],u32Reg,u16Val,u16Mask);
    }
    else
    {
        if(genUtiId == E_HAL_SCL_UTI_ID_SC_DEFAULT)
        _HalUtilityWrite2ByteRegBuffer(u32Reg, u16Val, u16Mask);
        W2BYTEMSK(u32Reg,u16Val,u16Mask);
    }
}
void HalUtilitySetWaitMload(void)
{
    u8 u8Cnt = 0;
    DrvSclOsCmdqInterface_t pstCmdq;
    HalUtilityCMDQIdType_e enCmdqId = E_HAL_SCL_UTI_CMDQID_SC_ALL;
    if(gpCmdqF == HalUtilityW2BYTEMSKDirectCmdqM2M)
    {
        enCmdqId = E_HAL_SCL_UTI_CMDQID_RSC;
    }
    pstCmdq = gpstCmdq[enCmdqId];
    if((DrvSclOsGetAccessRegMode() ==  E_DRV_SCLOS_AccessReg_CMDQ)&& pstCmdq.MHAL_CMDQ_CmdqPollRegBits)
    {
        pstCmdq.MHAL_CMDQ_CmdqPollRegBits(&pstCmdq,REG_SCL0_17_L,0x1000,0x1000,1);
        pstCmdq.MHAL_CMDQ_WriteRegCmdqMask(&pstCmdq,REG_SCL0_13_L,0x1000,0x1000);
        pstCmdq.MHAL_CMDQ_WriteRegCmdqMask(&pstCmdq,REG_SCL0_13_L,0,0x1000);
    }
    else
    {
        for(u8Cnt=0;u8Cnt<100;u8Cnt++)
        {
            if(HalUtilityR2BYTEDirect(REG_SCL0_17_L)&0x1000)
            {
                HalUtilityW2BYTEMSKDirect(REG_SCL0_13_L,0x1000,0x1000);
                HalUtilityW2BYTEMSKDirect(REG_SCL0_13_L,0,0x1000);
                break;
            }
            DrvSclOsDelayTaskUs(500);
        }
    }
}
void HalUtilityDumpSetting(void)
{
    u32 u32TblCnt;
    HalUtilityCmdReg_t *pvBankaddr;
    u32 i;
    u32TblCnt = *gu32TblCnt;
    for(i=0;i<u32TblCnt;i++)
    {
       pvBankaddr = (HalUtilityCmdReg_t *)(gpvInquireBuffer + (i*SIZE_OF_CMDREG));
       SCL_DBGERR("[SCLUTILITY]%s::u32Addr:%lx Offset:%lx u16Data:%hx u16Mask:%hx\n",__FUNCTION__,
           (pvBankaddr->u32Addr>>8),(pvBankaddr->u32Addr&0xFF)/2,pvBankaddr->u16Data,pvBankaddr->u16Mask);
    }
    SCL_DBGERR("[SCLUTILITY]InstTblCnt:%lu\n",u32TblCnt);
    if(gpvWdrTblBuffer)
    {
        u32TblCnt = *gp32WdrTblCnt;
        for(i=0;i<u32TblCnt;i++)
        {
           pvBankaddr = (HalUtilityCmdReg_t *)(gpvWdrTblBuffer + (i*SIZE_OF_CMDREG));
           SCL_DBGERR("[SCLUTILITY]WdrTbl %s::u32Addr:%lx Offset:%lx u16Data:%hx u16Mask:%hx\n",__FUNCTION__,
               (pvBankaddr->u32Addr>>8),(pvBankaddr->u32Addr&0xFF)/2,pvBankaddr->u16Data,pvBankaddr->u16Mask);
        }
        SCL_DBGERR("[SCLUTILITY]WdrTblCnt:%lu\n",u32TblCnt);
    }
}
bool _HalUtilityIsRaceReg(u32 u32Reg)
{
    bool ret = 0;
    switch(u32Reg)
    {
        case E_HAL_SCL_UTI_RACE_SCL002:
        case E_HAL_SCL_UTI_RACE_SCL010:
        case E_HAL_SCL_UTI_RACE_SCL011:
        case E_HAL_SCL_UTI_RACE_SCL012:
        case E_HAL_SCL_UTI_RACE_SCL013:
        case E_HAL_SCL_UTI_RACE_SCL008:
        case E_HAL_SCL_UTI_RACE_SCL009:
        case E_HAL_SCL_UTI_RACE_SCL00A:
        case E_HAL_SCL_UTI_RACE_SCL00B:
        case E_HAL_SCL_UTI_RACE_NE10:
        case E_HAL_SCL_UTI_RACE_NE16:
        case E_HAL_SCL_UTI_RACE_NE18:
        case E_HAL_SCL_UTI_RACE_NE70:
        case E_HAL_SCL_UTI_RACE_NE71:
        case E_HAL_SCL_UTI_RACE_NE72:
            ret = 1;
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}
void HalUtilityFilpRIURegFire(void)
{
    //ToDo:Fire by ISR Flip
    //get cnt (for loop)
    u32 u32TblCnt;
    HalUtilityCmdReg_t *pvBankaddr;
    u32 i;
    SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_FLIP,"[SCLUTILITY]%s::Use RIU\n",__FUNCTION__);
    u32TblCnt = *gu32TblCnt;
    for(i=0;i<u32TblCnt;i++)
    {
        //get buffer (get addr)
       pvBankaddr = (HalUtilityCmdReg_t *)(gpvInquireBuffer + (i*SIZE_OF_CMDREG));
       //write by RIU
       //get cmd(by addr)
       W2BYTEMSK(pvBankaddr->u32Addr,pvBankaddr->u16Data,pvBankaddr->u16Mask);
       SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_FLIP, "[SCLUTILITY]%s::u32Addr:%lx Offset:%lx u16Data:%hx u16Mask:%hx\n",__FUNCTION__,
           (pvBankaddr->u32Addr>>8),(pvBankaddr->u32Addr&0xFF)/2,pvBankaddr->u16Data,pvBankaddr->u16Mask);
    }
    if(gpvWdrTblBuffer)
    {
        u32TblCnt = *gp32WdrTblCnt;
        for(i=0;i<u32TblCnt;i++)
        {
            //get buffer (get addr)
           pvBankaddr = (HalUtilityCmdReg_t *)(gpvWdrTblBuffer + (i*SIZE_OF_CMDREG));
           W2BYTEMSK(pvBankaddr->u32Addr,pvBankaddr->u16Data,pvBankaddr->u16Mask);
           SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_FLIP, "[SCLUTILITY]WDR %s::u32Addr:%lx Offset:%lx u16Data:%hx u16Mask:%hx\n",__FUNCTION__,
               (pvBankaddr->u32Addr>>8),(pvBankaddr->u32Addr&0xFF)/2,pvBankaddr->u16Data,pvBankaddr->u16Mask);
        }
    }
}
HalUtilityInstIdType_e _HalUtilitySwitchInstType(HalUtilityCMDQIdType_e enCmdqId)
{
    HalUtilityInstIdType_e enInstType;
    if(enCmdqId==E_HAL_SCL_UTI_CMDQID_RSC)
    {
        enInstType = E_HAL_SCL_UTI_INSTID_RSC;
    }
    else
    {
        if(genUtiId==E_HAL_SCL_UTI_ID_SC_ALL)
        {
            enInstType = E_HAL_SCL_UTI_INSTID_SC_ALL;
        }
        else
        {
            enInstType = E_HAL_SCL_UTI_INSTID_VIP;
        }
    }
    return enInstType;
}

void HalUtilityFilpRegFire(HalUtilityCMDQIdType_e enCmdqId)
{
    u32 u32TblCnt;
    HalUtilityCmdReg_t *pvBankaddr;
    u32 i;
    //bool bset = 1;
    u32 u32Cnt = 0;
    //void *pvLastTimeInquireBuffer;
    //HalUtilityInstIdType_e enInstType;
#if CMDQ_CHECK_BY_INST
    HalUtilityCmdReg_t *pvBankaddrchk;
#endif
    //enInstType = _HalUtilitySwitchInstType(enCmdqId);
    if((DrvSclOsGetAccessRegMode() ==  E_DRV_SCLOS_AccessReg_CMDQ)&& gpstCmdq[enCmdqId].MHAL_CMDQ_WriteRegCmdqMask)
    {
        //get cnt (for loop)
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_FLIP, "[SCLUTILITY]%s::Use CMDQ\n",__FUNCTION__);
        u32TblCnt = *gu32TblCnt;
        /*
        pvLastTimeInquireBuffer = gpvLastTimeInquireBuffer[enInstType];
        if(bset)
        */
        gpstCmdq[enCmdqId].MHAL_CMDQ_WriteRegCmdqMaskMulti(&gpstCmdq[enCmdqId],(MHAL_CMDQ_MultiCmdBufMask_t *)gpvInquireBuffer,u32TblCnt);
            /*
        for(i=0;i<u32TblCnt;i++)
        {
            //get buffer (get addr)
           pvBankaddr = (HalUtilityCmdReg_t *)(gpvInquireBuffer + (i*SIZE_OF_CMDREG));
#if CMDQ_CHECK_BY_INST
           if(pvLastTimeInquireBuffer && pvLastTimeInquireBuffer!=gpvInquireBuffer)
           {
               pvBankaddrchk = (HalUtilityCmdReg_t *)(pvLastTimeInquireBuffer + (i*SIZE_OF_CMDREG));
               if((pvBankaddr->u32Addr == pvBankaddrchk->u32Addr) &&
               ((pvBankaddr->u16Data==pvBankaddrchk->u16Data)&&(pvBankaddr->u16Mask==pvBankaddrchk->u16Mask)))
               {
                    bset = 0;
               }
               else
               {
                   bset = 1;
               }
           }
           else
           {
               bset = 1;
           }
#else
            bset = 1;
#endif
           //put in cmdq buffer
           //get cmd(by addr)
           //pf();
           if(bset)
           {
                gpstCmdq[enCmdqId].MHAL_CMDQ_WriteRegCmdqMask(&gpstCmdq[enCmdqId],pvBankaddr->u32Addr,pvBankaddr->u16Data,pvBankaddr->u16Mask);
                u32Cnt++;
            }
           //W2BYTEMSK(pvBankaddr->u32Addr,pvBankaddr->u16Data,pvBankaddr->u16Mask);
           SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_FLIP, "[SCLUTILITY]%s::u32Addr:%lx Offset:%lx u16Data:%hx u16Mask:%hx\n",__FUNCTION__,
               (pvBankaddr->u32Addr>>8),(pvBankaddr->u32Addr&0xFF)/2,pvBankaddr->u16Data,pvBankaddr->u16Mask);
        }
        */
        //gpvLastTimeInquireBuffer[enInstType] = gpvInquireBuffer;
        if(gpvWdrTblBuffer)
        {
            u32TblCnt = *gp32WdrTblCnt;
            for(i=0;i<u32TblCnt;i++)
            {
                //get buffer (get addr)
               pvBankaddr = (HalUtilityCmdReg_t *)(gpvWdrTblBuffer + (i*SIZE_OF_CMDREG));
               //put in cmdq buffer
               //get cmd(by addr)
               //pf();
               gpstCmdq[enCmdqId].MHAL_CMDQ_WriteRegCmdqMask(&gpstCmdq[enCmdqId],pvBankaddr->u32Addr,pvBankaddr->u16Data,pvBankaddr->u16Mask);
               //W2BYTEMSK(pvBankaddr->u32Addr,pvBankaddr->u16Data,pvBankaddr->u16Mask);
               SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_FLIP,"[SCLUTILITY]WdrTbl %s::u32Addr:%lx Offset:%lx u16Data:%hx u16Mask:%hx\n",__FUNCTION__,
                   (pvBankaddr->u32Addr>>8),(pvBankaddr->u32Addr&0xFF)/2,pvBankaddr->u16Data,pvBankaddr->u16Mask);
            }
        }
    }
    else
    {
        HalUtilityFilpRIURegFire();
    }
    SCL_DBG(SCL_DBG_LV_UTILITY()&(EN_DBGMG_UTILITY_HIGH|EN_DBGMG_UTILITY_FLIP), "[SCLUTILITY]%s::Cnt:%lu\n",__FUNCTION__,
        u32Cnt);
}
u16 HalUtilityR2BYTE(u32 u32Reg)
{
    u16 u16RetVal;
    u16 u16ChkVal;
    u16RetVal = _HalUtilityReadRegBuffer(u32Reg,0,0);
    u16ChkVal = HalUtilityR2BYTEDirect(u32Reg);
    if(u16ChkVal != u16RetVal)
    {
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_LOW, "[SCLUTILITY]u32Reg:%lx chkval:%hx RetVal:%hx \n",
            u32Reg,u16ChkVal,u16RetVal);
    }
    return u16RetVal;
}
void HalUtilityW2BYTECmdq(u32 u32Reg,u16 u16Val)
{
    if(_HalUtilityWrite2ByteRegBuffer(u32Reg, u16Val, 0xFFFF))
    {
        //DrvSclCmdqWrite(E_DRV_SCLCMDQ_TYPE_IP0, u32Reg, u16Val, 0xFFFF);
        //write to Inqire buffer
        _HalUtilityWrite2ByteInquireBuffer(u32Reg, u16Val, 0xFFFF);
    }
    else
    {
        _HalUtilityWrite2ByteInquireBufferWithoutRegBuf(u32Reg, u16Val, 0xFFFF);
    }
}
void HalUtilityCloseShadow(void)
{
    if(gpstCmdq[E_HAL_SCL_UTI_CMDQID_SC_ALL].MHAL_CMDQ_WriteRegCmdqMask == NULL)
    {
        W2BYTEMSK(REG_SCL0_02_L,BIT3,BIT3);
    }
}
void HalUtilityOpenShadow(void)
{
    if(gpstCmdq[E_HAL_SCL_UTI_CMDQID_SC_ALL].MHAL_CMDQ_WriteRegCmdqMask == NULL)
    {
        W2BYTEMSK(REG_SCL0_02_L,0,BIT3);
    }
}

void HalUtilityW2BYTEDirect(u32 u32Reg,u16 u16Val)
{
    if(genUtiId == E_HAL_SCL_UTI_ID_SC_DEFAULT)
    _HalUtilityWrite2ByteRegBuffer(u32Reg, u16Val, 0xFFFF);
    W2BYTE(u32Reg,u16Val);
}
void HalUtilityW2BYTE(u32 u32Reg,u16 u16Val)
{
    if((DrvSclOsGetAccessRegMode() ==  E_DRV_SCLOS_AccessReg_CMDQ))
    {
        HalUtilityW2BYTECmdq(u32Reg,u16Val);
    }
    else
    {
        HalUtilityW2BYTEDirect(u32Reg,u16Val);
    }
}
void HalUtilityW4BYTECmdq(u32 u32Reg,u32 u32Val)
{
    if(_HalUtilityWrite2ByteRegBuffer(u32Reg, (u16)(u32Val&0xFFFF), 0xFFFF))
    {
        _HalUtilityWrite2ByteRegBuffer(u32Reg+2, (u16)((u32Val&0xFFFF0000)>>16), 0xFFFF);
        //DrvSclCmdqWrite(E_DRV_SCLCMDQ_TYPE_IP0, u32Reg, (u16)(u32Val&0xFFFF), 0xFFFF);
        //DrvSclCmdqWrite(E_DRV_SCLCMDQ_TYPE_IP0, u32Reg+2, (u16)((u32Val&0xFFFF0000)>>16), 0xFFFF);
        //write to Inqire buffer
        _HalUtilityWrite2ByteInquireBuffer(u32Reg, (u16)(u32Val&0xFFFF), 0xFFFF);
        _HalUtilityWrite2ByteInquireBuffer(u32Reg+2, (u16)((u32Val&0xFFFF0000)>>16), 0xFFFF);
    }
    else
    {
        _HalUtilityWrite2ByteInquireBufferWithoutRegBuf(u32Reg, (u16)(u32Val&0xFFFF), 0xFFFF);
        _HalUtilityWrite2ByteInquireBufferWithoutRegBuf(u32Reg+2, (u16)((u32Val&0xFFFF0000)>>16), 0xFFFF);
    }
}
void HalUtilityW4BYTEDirect(u32 u32Reg,u32 u32Val)
{
    if(genUtiId == E_HAL_SCL_UTI_ID_SC_DEFAULT)
    {
        _HalUtilityWrite2ByteRegBuffer(u32Reg, (u16)(u32Val&0xFFFF), 0xFFFF);
        _HalUtilityWrite2ByteRegBuffer(u32Reg+2, (u16)((u32Val&0xFFFF0000)>>16), 0xFFFF);
    }
    W4BYTE(u32Reg,u32Val);
}
void HalUtilityW4BYTE(u32 u32Reg,u32 u32Val)
{
    if((DrvSclOsGetAccessRegMode() ==  E_DRV_SCLOS_AccessReg_CMDQ))
    {
        //write to Inqire buffer
        HalUtilityW4BYTECmdq(u32Reg,u32Val);
    }
    else
    {
        HalUtilityW4BYTEDirect(u32Reg,u32Val);
    }
}
void HalUtilityW2BYTEMSKCmdq(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    if(_HalUtilityWrite2ByteRegBuffer(u32Reg, u16Val, u16Mask))
    {
        //DrvSclCmdqWrite(E_DRV_SCLCMDQ_TYPE_IP0, u32Reg, u16Val, u16Mask);
        //write to Inqire buffer
        _HalUtilityWrite2ByteInquireBuffer(u32Reg, u16Val, u16Mask);
    }
    else
    {
        _HalUtilityWrite2ByteInquireBufferWithoutRegBuf(u32Reg, u16Val, u16Mask);
    }
}
void HalUtilityW2BYTEMSKDirect(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    //DrvUtilityMutexLock();
    if(genUtiId == E_HAL_SCL_UTI_ID_SC_DEFAULT)
    _HalUtilityWrite2ByteRegBuffer(u32Reg, u16Val, u16Mask);
    //DrvUtilityMutexUNLock();
    W2BYTEMSK(u32Reg,u16Val,u16Mask);
}
void HalUtilityW2BYTEMSKDirectCmdq(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    if(gpCmdqF)
    {
        gpCmdqF(u32Reg,u16Val,u16Mask);
    }
    else
    {
        SCL_ERR("%s without pf\n",__FUNCTION__);
    }
}
void _HalUtilityW2BYTETbl(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    //if SRAM Tbl pbuf != NULL
    if(gpvWdrTblBuffer!=NULL)
    {
        // write to Sram buffer
        _HalUtilityWrite2ByteTblBuffer(u32Reg, u16Val, u16Mask);
    }
    else
    {
        W2BYTEMSK(u32Reg,u16Val,u16Mask);
    }
}
void HalUtilityW2BYTETbl(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    if((DrvSclOsGetAccessRegMode() ==  E_DRV_SCLOS_AccessReg_CMDQ))
    {
        //write to Inqire buffer
        _HalUtilityW2BYTETbl(u32Reg,u16Val,u16Mask);
    }
    else
    {
        HalUtilityW2BYTEMSKDirect(u32Reg,u16Val,u16Mask);
    }
}
void HalUtilityW2BYTEMSK(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    if((DrvSclOsGetAccessRegMode() ==  E_DRV_SCLOS_AccessReg_CMDQ))
    {
        //write to Inqire buffer
        HalUtilityW2BYTEMSKCmdq(u32Reg,u16Val,u16Mask);
    }
    else
    {
        HalUtilityW2BYTEMSKDirect(u32Reg,u16Val,u16Mask);
    }
}
void HalUtilityW2BYTEMSKBuffer(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    u16 u16RetVal;
    u16RetVal = _HalUtilityWrite2ByteRegBuffer(u32Reg, u16Val, u16Mask);
    u16RetVal = u16RetVal;
}
void HalUtilityWBYTEMSKCmdq(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    if(_HalUtilityWriteByteRegBuffer(u32Reg, (u8)u16Val, (u8)u16Mask))
    {
        //DrvSclCmdqWrite(E_DRV_SCLCMDQ_TYPE_IP0, u32Reg, u16Val, u16Mask);
        //write to Inqire buffer
        _HalUtilityWrite2ByteInquireBuffer(u32Reg, u16Val, u16Mask);
    }
    else
    {
        _HalUtilityWrite2ByteInquireBufferWithoutRegBuf(u32Reg, u16Val, u16Mask);
    }
}
void HalUtilityWBYTEMSKDirect(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    //DrvUtilityMutexLock();
    if(genUtiId == E_HAL_SCL_UTI_ID_SC_DEFAULT)
    _HalUtilityWriteByteRegBuffer(u32Reg, (u8)u16Val, (u8)u16Mask);
    //DrvUtilityMutexUNLock();
    WBYTEMSK(u32Reg,u16Val,u16Mask);
}
void HalUtilityWBYTEMSK(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    if((DrvSclOsGetAccessRegMode() ==  E_DRV_SCLOS_AccessReg_CMDQ))
    {
        //write to Inqire buffer
        HalUtilityWBYTEMSKCmdq(u32Reg,u16Val,u16Mask);
    }
    else
    {
        HalUtilityWBYTEMSKDirect(u32Reg,u16Val,u16Mask);
    }
}
void HalUtilityWBYTEMSKBuffer(u32 u32Reg,u8 u8Val,u8 u8Mask)
{
    u8 u8RetVal;
    u8RetVal = _HalUtilityWriteByteRegBuffer(u32Reg, (u8)u8Val, (u8)u8Mask);
    u8RetVal = u8RetVal;
}
#else
u16 HalUtilityR2BYTE(u32 u32Reg)
{
    u16 u16RetVal;
    u16RetVal = HalUtilityR2BYTEDirect(u32Reg);
    return u16RetVal;
}
void HalUtilityW2BYTE(u32 u32Reg,u16 u16Val)
{
    W2BYTE(u32Reg,u16Val);
}
void HalUtilityW4BYTE(u32 u32Reg,u32 u32Val)
{
    W4BYTE(u32Reg,u32Val);
}
void HalUtilityW2BYTEMSK(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    W2BYTEMSK(u32Reg,u16Val,u16Mask);
}
void HalUtilityW2BYTEMSKBuffer(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
}
void HalUtilityWBYTEMSK(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    WBYTEMSK(u32Reg,u16Val,u16Mask);
}
void HalUtilityWBYTEMSKBuffer(u32 u32Reg,u8 u8Val,u8 u8Mask)
{
}
#endif
bool HalUtilitySwapCmdqPf(HalUtilityCMDQIdType_e enCmdqId)
{
    if(enCmdqId==E_HAL_SCL_UTI_CMDQID_RSC)
    {
        gpCmdqF = HalUtilityW2BYTEMSKDirectCmdqM2M;
    }
    else
    {
        gpCmdqF = HalUtilityW2BYTEMSKDirectCmdqSCL;
    }
    return 1;
}
bool HalUtilitySwapTblCnt(u32 *pu32Count)
{
    gu32TblCnt = pu32Count;
    return 1;
}
bool HalUtilitySwapWdrTblCnt(u32 *pu32Count)
{
    gp32WdrTblCnt = pu32Count;
    return 1;
}
bool HalUtilitySwapActiveRegTbl(void *pvCtx)
{
    gpvRegBuffer = pvCtx;
    return 1;
}
bool HalUtilitySwapInquireTbl(void *pvCtx)
{
    gpvInquireBuffer = pvCtx;
    return 1;
}
bool HalUtilitySwapWdrTbl(void *pvCtx)
{
    gpvWdrTblBuffer = pvCtx;
    return 1;
}
bool HalUtilitySetDefaultWdrTblBuffer(void *pvDefaultRegBuffer,u32 *p32DefaultTblCnt)
{
    bool bret = 0;
    if(pvDefaultRegBuffer)
    {
        gpvDefaultWdrTblBuffer = pvDefaultRegBuffer;
        gp32DefaultWdrTblCnt = p32DefaultTblCnt;
        bret = 1;
    }
    else
    {
        bret = 0;
    }
    return bret;
}
bool HalUtilitySetDefaultBuffer(void *pvDefaultRegBuffer, void *pvDefaultInquireBuffer,u32 *p32DefaultTblCnt)
{
    bool bret = 0;
    if(pvDefaultRegBuffer)
    {
        gpvDefaultRegBuffer = pvDefaultRegBuffer;
        if(pvDefaultInquireBuffer)
        {
            gpvDefaultInquireBuffer = pvDefaultInquireBuffer;
            gu32DefaultTblCnt = p32DefaultTblCnt;
            bret = 1;
        }
        else
        {
            bret = 0;
        }
    }
    else
    {
        bret = 0;
    }
    return bret;

}
void HalUtilityKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq)
{
    if(pstCmdq)
    {
        //ToDo
        HalUtilityCloseShadow();
        DrvSclOsMemcpy(&gpstCmdq[E_HAL_SCL_UTI_CMDQID_SC_ALL],pstCmdq,sizeof(DrvSclOsCmdqInterface_t));
    }
    else
    {
        DrvSclOsMemset(&gpstCmdq[E_HAL_SCL_UTI_CMDQID_SC_ALL],0,sizeof(DrvSclOsCmdqInterface_t));
        HalUtilityOpenShadow();
    }
}
void HalUtilityKeepM2MCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq)
{
    if(pstCmdq)
    {
        //ToDo
        DrvSclOsMemcpy(&gpstCmdq[E_HAL_SCL_UTI_CMDQID_RSC],pstCmdq,sizeof(DrvSclOsCmdqInterface_t));
    }
    else
    {
        DrvSclOsMemset(&gpstCmdq[E_HAL_SCL_UTI_CMDQID_RSC],0,sizeof(DrvSclOsCmdqInterface_t));
    }
}
bool HalUtilityDeInit(void)
{
    HalUtilitySetUtiId(E_HAL_SCL_UTI_ID_SC_DEFAULT);
    _HalUtilityDeInitRegBuffer();
    gpvRegBuffer = gpvDefaultRegBuffer = NULL;
    gpvInquireBuffer = gpvDefaultInquireBuffer =NULL;
    gu32TblCnt = gu32DefaultTblCnt =NULL;
    gp32WdrTblCnt = gp32DefaultWdrTblCnt =NULL;
    gpvWdrTblBuffer = gpvDefaultWdrTblBuffer =NULL;
    if(_UTILITY_Mutex != -1)
    {
        _UTILITY_Mutex = -1;
    }
    return TRUE;
}
void HalUtilitySetUtiId(HalUtilityIdType_e enId)
{
    genUtiId = enId;
}
bool HalUtilityInit(s32 s32Mutex)
{
    //1.create mutex
    //2.allocate buffer 1bank=256byte x EN_REG_NUM_CONFIG
    //3.read all address and keep in buffer
    //char mutx_word[] = {"_UTILITY_Mutex"};
    UTILITY_RIU_BASE_Vir = VIR_RIUBASE;
    DrvSclOsMemset(gpstCmdq,0,sizeof(DrvSclOsCmdqInterface_t)*E_HAL_SCL_UTI_CMDQID_MAX);
    gpCmdqF = HalUtilityW2BYTEMSKDirectCmdqSCL;
    // 1.
    if(_UTILITY_Mutex != -1)
    {
        SCL_DBG(SCL_DBG_LV_UTILITY()&EN_DBGMG_UTILITY_INIT, "[SCLUTILITY]%s(%d)::Already Done\n", __FUNCTION__, __LINE__);
        return TRUE;
    }
    if(DrvSclOsInit() == FALSE)
    {
        SCL_ERR("[SCLUTILITY]%s(%d)::MsOS_Init Fail\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    _UTILITY_Mutex = s32Mutex;
    //2.
    #if USE_Utility
    HalUtilitySetUtiId(E_HAL_SCL_UTI_ID_SC_DEFAULT);
    HalUtilitySetDefaultBuffer(gpvDefaultRegBuffer, gpvDefaultInquireBuffer,gu32DefaultTblCnt);
    HalUtilitySetDefaultWdrTblBuffer(gpvDefaultWdrTblBuffer,gp32DefaultWdrTblCnt);
    if(!gpvDefaultRegBuffer || !gpvDefaultInquireBuffer)
    {
        SCL_ERR("[SCLUTILITY]%s(%d)::Allocate Fail\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    else
    {
        gpvRegBuffer = gpvDefaultRegBuffer;
        gpvInquireBuffer = gpvDefaultInquireBuffer;
        gu32TblCnt = gu32DefaultTblCnt;
        SCL_ERR("[SCLUTILITY]%s(%d)::Allocate Success!\n", __FUNCTION__, __LINE__);
    }
    if(gpvDefaultWdrTblBuffer)
    {
        gpvWdrTblBuffer = gpvDefaultWdrTblBuffer;
        gp32WdrTblCnt = gp32DefaultWdrTblCnt;
    }
    //3.
    _HalUtilityInitRegBuffer();
    #endif
    return TRUE;
}

#undef HAL_SCLIRQ_C
