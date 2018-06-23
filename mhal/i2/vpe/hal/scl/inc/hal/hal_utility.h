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
// (!¡±MStar Confidential Information!¡L) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
//==============================================================================
#ifndef _HAL_UTILITY_H
#define _HAL_UTILITY_H

#include "hal_scl_reg.h"

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_SCL_UTI_CMDQID_SC_DEFAULT =0,
    E_HAL_SCL_UTI_CMDQID_SC_ALL = 0,     //for main inst
    E_HAL_SCL_UTI_CMDQID_VIP = 0,        //for vip/iq
    E_HAL_SCL_UTI_CMDQID_RSC = 1,        //for sub inst Rdma/rsc
    E_HAL_SCL_UTI_CMDQID_MAX = 2,       //I2 =2 I5 =4
    E_HAL_SCL_UTI_CMDQID_LDC = 2,        //for sub inst ldc
    E_HAL_SCL_UTI_CMDQID_WDMA_ROT = 3,   //for main inst rot_wdma
}HalUtilityCMDQIdType_e;
typedef enum
{
    E_HAL_SCL_UTI_INSTID_SC_DEFAULT =0,
    E_HAL_SCL_UTI_INSTID_SC_ALL = 0,     //for main inst
    E_HAL_SCL_UTI_INSTID_VIP = 1,        //for vip/iq
    E_HAL_SCL_UTI_INSTID_RSC = 2,        //for sub inst Rdma/rsc
    E_HAL_SCL_UTI_INSTID_MAX = 3,       //I2 =3 I5 =5
    E_HAL_SCL_UTI_INSTID_LDC = 3,        //for sub inst ldc
    E_HAL_SCL_UTI_INSTID_WDMA_ROT = 4,   //for main inst rot_wdma
}HalUtilityInstIdType_e;
typedef enum
{
    E_HAL_SCL_UTI_ID_SC_DEFAULT,
    E_HAL_SCL_UTI_ID_SC_ALL,
    E_HAL_SCL_UTI_ID_SC_VIP,        //for vip/iq
    E_HAL_SCL_UTI_ID_NUM,
}HalUtilityIdType_e;
typedef enum
{
    E_HAL_SCL_UTI_RACE_SCL002 = REG_SCL0_02_L,
    E_HAL_SCL_UTI_RACE_SCL008 = REG_SCL0_08_L,
    E_HAL_SCL_UTI_RACE_SCL009 = REG_SCL0_09_L,
    E_HAL_SCL_UTI_RACE_SCL00A = REG_SCL0_0A_L,
    E_HAL_SCL_UTI_RACE_SCL00B = REG_SCL0_0B_L,
    E_HAL_SCL_UTI_RACE_SCL010 = REG_SCL0_10_L,
    E_HAL_SCL_UTI_RACE_SCL011 = REG_SCL0_11_L,
    E_HAL_SCL_UTI_RACE_SCL012 = REG_SCL0_12_L,
    E_HAL_SCL_UTI_RACE_SCL013 = REG_SCL0_13_L,
    E_HAL_SCL_UTI_RACE_NE18 = REG_VIP_NE_18_L,
    E_HAL_SCL_UTI_RACE_NE10 = REG_VIP_NE_10_L,
    E_HAL_SCL_UTI_RACE_NE16 = REG_VIP_NE_16_L,
    E_HAL_SCL_UTI_RACE_NE70 = REG_VIP_NE_70_L,
    E_HAL_SCL_UTI_RACE_NE71 = REG_VIP_NE_71_L,
    E_HAL_SCL_UTI_RACE_NE72 = REG_VIP_NE_72_L,
    E_HAL_SCL_UTI_RACE_NUM = 9,
}HalUtilityRaceReg_e;


typedef enum
{
    EN_REG_SCL_HVSP0_BASE        = 0,
    EN_REG_SCL_HVSP1_BASE       ,//= 1
    EN_REG_SCL_HVSP2_BASE       ,      ///<  = 2
    EN_REG_SCL_HVSP3_BASE       ,      ///<  = 2
    EN_REG_SCL_DMA0_BASE      ,      ///= 3
    EN_REG_SCL_DMA1_BASE         ,    ///= 4
    EN_REG_SCL_DMA2_BASE         ,     ///= 5
    EN_REG_SCL_DMA3_BASE         ,     ///= 5
    EN_REG_SCL_NLM0_BASE  ,
    EN_REG_SCL0_BASE   ,
    EN_REG_SCL1_BASE   ,
    EN_REG_SCL2_BASE     ,
    EN_REG_VIP_ACE3_BASE   ,
    EN_REG_VIP_ACE_BASE   ,
    EN_REG_VIP_PK_BASE     ,
    EN_REG_VIP_DLC_BASE      ,
    EN_REG_VIP_MWE_BASE    ,
    EN_REG_VIP_ACE2_BASE     ,
    EN_REG_VIP_LCE_BASE  ,
    EN_REG_SCL_DNR2_BASE   ,
    EN_REG_SCL3_BASE   ,
    EN_REG_VIP_SCNR_BASE   ,
    EN_REG_VIP_WDR_BASE     ,
    EN_REG_VIP_WDR1_BASE     ,
    EN_REG_VIP_YEE_BASE      ,
    EN_REG_VIP_MCNR_BASE    ,
    EN_REG_VIP_NE_BASE    ,
    EN_REG_MDWIN1_BASE     ,
    EN_REG_MDWIN2_BASE  ,
    EN_REG_UCM1_BASE   ,
    EN_REG_UCM2_BASE   ,
    EN_REG_ARBSHP_BASE      ,
    EN_REG_MLOAD_BASE    ,
    EN_REG_RSC_BASE     ,
    EN_REG_NUM_CONFIG         ,
    EN_REG_ISP_BASE    ,
    EN_REG_SCL_LPLL_BASE    ,
    EN_REG_SCL_CLK_BASE    ,
    EN_REG_BLOCK_BASE    ,
    EN_REG_CHIPTOP_BASE      ,
    EN_REG_CMDQ0_BASE    ,
    EN_REG_CMDQ1_BASE    ,
    EN_REG_CMDQ2_BASE    ,
    EN_REG_SCL_LDC_BASE   ,
    EN_REG_SCL_LDC1_BASE   ,
    EN_REG_SCL_LDC2_BASE     ,
    EN_REG_DMAG0_BASE    ,
    EN_REG_SCL_DNR1_BASE   ,
}EN_REG_CONFIG_TYPE;
typedef enum
{
    EN_REG_SCLALL_SCL_HVSP0_BASE        = 0,
    EN_REG_SCLALL_SCL_HVSP1_BASE       ,//= 1
    EN_REG_SCLALL_SCL_HVSP2_BASE       ,      ///<  = 2
    EN_REG_SCLALL_SCL_HVSP3_BASE       ,      ///<  = 2
    EN_REG_SCLALL_SCL_DMA0_BASE      ,      ///= 3
    EN_REG_SCLALL_SCL_DMA1_BASE         ,    ///= 4
    EN_REG_SCLALL_SCL_DMA2_BASE         ,     ///= 5
    EN_REG_SCLALL_SCL_DMA3_BASE         ,     ///= 5
    EN_REG_SCLALL_SCL0_BASE   ,
    EN_REG_SCLALL_SCL1_BASE   ,
    EN_REG_SCLALL_SCL2_BASE     ,
    EN_REG_SCLALL_VIP_LCE_BASE  ,  //for SetVipSize
    EN_REG_SCLALL_SCL3_BASE   ,
    EN_REG_SCLALL_VIP_SCNR_BASE   , //for AIP size
    EN_REG_SCLALL_VIP_WDR1_BASE     ,//for wdr size
    EN_REG_SCLALL_VIP_MCNR_BASE    ,// for IPM
    EN_REG_SCLALL_MDWIN1_BASE     ,
    EN_REG_SCLALL_MDWIN2_BASE  ,
    EN_REG_SCLALL_UCM1_BASE   ,
    EN_REG_SCLALL_UCM2_BASE   ,
    EN_REG_SCLALL_MLOAD_BASE    ,
    EN_REG_SCLALL_RSC_BASE     ,
    EN_REG_SCLALL_VIP_NE_BASE    ,
    EN_REG_SCLALL_NUM_CONFIG         ,
}EN_REG_SCLALL_CONFIG_TYPE;
typedef enum
{
    EN_REG_SCLVIP_SCL_HVSP0_BASE        = 0,
    EN_REG_SCLVIP_SCL_HVSP1_BASE       ,//= 1
    EN_REG_SCLVIP_SCL_HVSP2_BASE       ,      ///<  = 2
    EN_REG_SCLVIP_SCL_HVSP3_BASE       ,      ///<  = 2
    EN_REG_SCLVIP_SCL_NLM0_BASE  ,
    EN_REG_SCLVIP_SCL0_BASE   ,
    EN_REG_SCLVIP_SCL1_BASE   ,
    EN_REG_SCLVIP_VIP_ACE3_BASE   ,
    EN_REG_SCLVIP_VIP_ACE_BASE   ,
    EN_REG_SCLVIP_VIP_PK_BASE     ,
    EN_REG_SCLVIP_VIP_DLC_BASE      ,
    EN_REG_SCLVIP_VIP_MWE_BASE    ,
    EN_REG_SCLVIP_VIP_ACE2_BASE     ,
    EN_REG_SCLVIP_VIP_LCE_BASE  ,
    EN_REG_SCLVIP_SCL_DNR2_BASE   ,
    EN_REG_SCLVIP_VIP_SCNR_BASE   ,
    EN_REG_SCLVIP_VIP_WDR_BASE     ,
    EN_REG_SCLVIP_VIP_WDR1_BASE     ,
    EN_REG_SCLVIP_VIP_YEE_BASE      ,
    EN_REG_SCLVIP_VIP_MCNR_BASE    ,
    EN_REG_SCLVIP_VIP_NE_BASE    ,
    EN_REG_SCLVIP_ARBSHP_BASE      ,
    EN_REG_SCLVIP_MLOAD_BASE    ,
    EN_REG_SCLVIP_UCM1_BASE   ,
    EN_REG_SCLVIP_UCM2_BASE   ,
    EN_REG_SCLVIP_NUM_CONFIG         ,

}EN_REG_SCLVIP_CONFIG_TYPE;

typedef struct
{
   u32 u32Addr;  // 16bit Bank addr + 8bit 16bit-regaddr
   u16 u16Data;  // 16bit data
   u16 u16Mask;  // inverse normal case
}HalUtilityCmdReg_t;

#define MDRV_SCL_CTX_REG_BANK_NUM       (EN_REG_NUM_CONFIG)  // MAX number of reigster bank
#define MDRV_SCL_CTX_SCLALL_REG_BANK_NUM       (EN_REG_SCLALL_NUM_CONFIG)  // MAX number of reigster bank
#define MDRV_SCL_CTX_SCLVIP_REG_BANK_NUM       (EN_REG_SCLVIP_NUM_CONFIG)  // MAX number of reigster bank
#define MDRV_SCL_CTX_REG_BANK_SIZE      (128 * 4) // each bank has 128 register and each 32bit for value & Index

#define FLAG_CMDQ_IDXEXIST 0x8000
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifndef HAL_UTILITY_C
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE bool HalUtilityInit(s32 s32Mutex);
INTERFACE void HalUtilitySetUtiId(HalUtilityIdType_e enId);
INTERFACE bool HalUtilityDeInit(void);
INTERFACE bool HalUtilitySetDefaultWdrTblBuffer(void *pvDefaultRegBuffer,u32 *p32DefaultTblCnt);
INTERFACE bool HalUtilitySetDefaultBuffer(void *pvDefaultRegBuffer, void *pvDefaultInquireBuffer,u32 *p32DefaultTblCnt);
INTERFACE bool HalUtilitySwapInquireTbl(void *pvCtx);
INTERFACE bool HalUtilitySwapWdrTbl(void *pvCtx);
INTERFACE bool HalUtilitySwapActiveRegTbl(void *pvCtx);
INTERFACE bool HalUtilitySwapTblCnt(u32 *pu32Count);
INTERFACE bool HalUtilitySwapCmdqPf(HalUtilityCMDQIdType_e enCmdqId);
INTERFACE bool HalUtilitySwapWdrTblCnt(u32 *pu32Count);
INTERFACE u16 HalUtilityR2BYTE(u32 u32Reg);
INTERFACE void HalUtilityFilpRegFire(HalUtilityCMDQIdType_e enCmdqId);
INTERFACE void HalUtilityDumpSetting(void);
INTERFACE void HalUtilitySetWaitMload(void);
INTERFACE void HalUtilityTriggerM2M(void *pvHandler);
INTERFACE void HalUtilityCreateInst(HalUtilityIdType_e enId,void *pvRegBuffer);
INTERFACE void HalUtilityGetMloadHandler(void **pvMloadhandler);
INTERFACE void HalUtilityFreeMloadHandler(void *pvMloadhandler);
INTERFACE void HalUtilitySetMloadTbl(u32 u32Id ,void *pvMloadHandler,void *pvBuf);
INTERFACE void HalUtilityMloadFire(void *pvMloadHandler);
INTERFACE u16 HalUtilityR2BYTEMaskDirect(u32 u32Reg,u16 u16Mask);
INTERFACE bool HalUtilityGetCmdqBufferIdx(u16 *u16BufferCnt,u32 u32Reg);
INTERFACE bool HalUtilitySetCmdqBufferIdx(u16 u16BufferCnt,u32 u32Reg);
INTERFACE u16 HalUtilityR2BYTEDirect(u32 u32Reg);
INTERFACE u32 HalUtilityR4BYTEDirect(u32 u32Reg);
INTERFACE void HalUtilityDumpRegSettingAll(u32 u32RegAddr);
INTERFACE void HalUtilityW2BYTE(u32 u32Reg,u16 u16Val);
INTERFACE void HalUtilityW2BYTEDirect(u32 u32Reg,u16 u16Val);
INTERFACE void HalUtilityCloseShadow(void);
INTERFACE void HalUtilityOpenShadow(void);
INTERFACE void HalUtilityW2BYTETbl(u32 u32Reg,u16 u16Val,u16 u16Mask);
INTERFACE void HalUtilityW2BYTEMSK(u32 u32Reg,u16 u16Val,u16 u16Mask);
INTERFACE void HalUtilityW2BYTEMSKDirect(u32 u32Reg,u16 u16Val,u16 u16Mask);
INTERFACE void HalUtilityW2BYTEMSKDirectCmdq(u32 u32Reg,u16 u16Val,u16 u16Mask);
INTERFACE void HalUtilityW2BYTEMSKBuffer(u32 u32Reg,u16 u16Val,u16 u16Mask);
INTERFACE void HalUtilityW4BYTE(u32 u32Reg,u32 u32Val);
INTERFACE void HalUtilityW4BYTEDirect(u32 u32Reg,u32 u32Val);
INTERFACE void HalUtilityWBYTEMSKBuffer(u32 u32Reg,u8 u8Val,u8 u8Mask);
INTERFACE void HalUtilityWBYTEMSK(u32 u32Reg,u16 u16Val,u16 u16Mask);
INTERFACE void HalUtilityWBYTEMSKDirect(u32 u32Reg,u16 u16Val,u16 u16Mask);
INTERFACE void HalUtilityKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq);
INTERFACE void HalUtilityKeepM2MCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq);



#undef INTERFACE

#endif
