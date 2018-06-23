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
#define __HAL_COVER_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "cam_os_wrapper.h"
#if defined(CAM_OS_RTK)
#include "cam_os_wrapper.h"
#include <string.h>
#else
#include <linux/string.h>
#endif

#include "hal_cover_util.h"
#include "hal_cover.h"
#include "hal_cover_reg.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define HAL_COVER_DEBUG   0

#if HAL_COVER_DEBUG
#if defined(CAM_OS_RTK)
#define HALCOVERDBG(fmt, arg...) UartSendTrace( fmt, ##arg) //CamOsDebug(fmt, ##arg)
#define HALCOVERERR(fmt, arg...) UartSendTrace( fmt, ##arg) //CamOsDebug(fmt, ##arg)
#else
#define HALCOVERDBG(fmt, arg...) printk(KERN_INFO fmt, ##arg) //CamOsDebug(fmt, ##arg)
#define HALCOVERERR(fmt, arg...) printk(KERN_INFO fmt, ##arg) //CamOsDebug(fmt, ##arg)
#endif
#else
#if defined(CAM_OS_RTK)
#define HALCOVERERR(fmt, arg...) UartSendTrace(fmt, ##arg) //CamOsDebug(fmt, ##arg)
#define HALCOVERDBG(fmt, arg...)
#else
#define HALCOVERERR(fmt, arg...) printk(KERN_INFO fmt, ##arg) //CamOsDebug(fmt, ##arg)
#define HALCOVERDBG(fmt, arg...)
#endif
#endif


#define IsHalIspCoverBaseIdType(x)          (x == REG_ISP_COVER_BASE)
#define IsHalDipCoverBaseIdType(x)          (x == REG_DIP_COVER_BASE)

//if set 1, use cmdq to set register, if set 0, use cpu to set register
#define COVER_REG_CMDQ_MD 1

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

MHAL_CMDQ_CmdqInterface_t _tCoverCmdqLocal[E_HAL_COVER_CMDQ_ID_NUM];


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

void _HalCoverWrite2Byte(u32  u32Reg, u16 u16Val,HalCoverCmdqIdType_e eCmdqId)
{
    if(eCmdqId < E_HAL_COVER_CMDQ_ID_NUM && COVER_REG_CMDQ_MD && _tCoverCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdq)
    {
        _tCoverCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdq(&_tCoverCmdqLocal[eCmdqId],u32Reg,u16Val);
    }
    else
    {
        COVER_W2BYTE(u32Reg, u16Val);
    }
}
void _HalCoverWrite2ByteMsk(u32 u32Reg, u16 u16Val, u16 u16Mask,HalCoverCmdqIdType_e eCmdqId)
{
    if(eCmdqId < E_HAL_COVER_CMDQ_ID_NUM && COVER_REG_CMDQ_MD && _tCoverCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdqMask)
    {
        _tCoverCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdqMask(&_tCoverCmdqLocal[eCmdqId],u32Reg,u16Val,u16Mask);
    }
    else
    {
        COVER_W2BYTEMSK(u32Reg, u16Val, u16Mask);
    }
}
u32 _HalCoverGetBaseAddr(HalCoverIdType_e eCoverId)
{
    u32 u32Base;

    switch(eCoverId)
    {
        case E_HAL_ISPSCL1_COVER_ID_00:
        case E_HAL_ISPSCL4_COVER_ID_01:
            u32Base = REG_ISP_COVER_BASE;
            break;
        case E_HAL_DIP_COVER_ID_02:
            u32Base = REG_DIP_COVER_BASE;
            break;
        default:
            u32Base = REG_ISP_COVER_BASE;
            HALCOVERERR("[COVER]%s %d: wrong COVERId\n", __FUNCTION__, __LINE__);
            break;

    }
    return u32Base;
}

u16 _HalCoverGetDoubleRW(HalCoverIdType_e eCoverId)
{
    u16 u16DoubleRW;

    switch(eCoverId)
    {
        case E_HAL_ISPSCL1_COVER_ID_00:
            u16DoubleRW = REG_ISPSC1_COVER_BWIN_TRIG;
            break;
        case E_HAL_ISPSCL4_COVER_ID_01:
            u16DoubleRW = REG_ISPSC4_COVER_BWIN_TRIG;
            break;
        case E_HAL_DIP_COVER_ID_02:
            u16DoubleRW = REG_DIP_COVER_BWIN_TRIG;
            break;
        default:
            u16DoubleRW = REG_ISPSC1_COVER_BWIN_TRIG;
            HALCOVERERR("[COVER]%s %d: wrong CoverId\n", __FUNCTION__, __LINE__);
            break;
    }

    return u16DoubleRW;

}

bool _HalCoverIdTransCmdqID(HalCoverIdType_e eCoverId,HalCoverCmdqIdType_e *pCmdqId)
{
    bool bRet = FALSE;
    switch(eCoverId)
    {
      case E_HAL_ISPSCL1_COVER_ID_00:
      case E_HAL_ISPSCL4_COVER_ID_01:
          *pCmdqId = E_HAL_COVER_CMDQ_VPE_ID_0;
          bRet = TRUE;
          break;
      case E_HAL_DIP_COVER_ID_02:
          *pCmdqId = E_HAL_COVER_CMDQ_DIVP_ID_1;
          bRet = TRUE;
          break;
      default:
          *pCmdqId = E_HAL_COVER_CMDQ_ID_NUM;
          bRet = FALSE;
          break;
    }

    return bRet;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

void HalCoverInitCmdq(void)
{
    memset(&_tCoverCmdqLocal[0],0,sizeof(MHAL_CMDQ_CmdqInterface_t)*E_HAL_COVER_CMDQ_ID_NUM);
    HALCOVERDBG("[COVER]%s %d: out \n", __FUNCTION__, __LINE__);
}
void HalCoverSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,HalCoverCmdqIdType_e eHalCmdqId)
{
    if(eHalCmdqId >= E_HAL_COVER_CMDQ_ID_NUM)
    {
        HALCOVERERR("[GOP]%s %d: wrong CMDQId\n", __FUNCTION__, __LINE__);
        return ;
    }

    if(pstCmdq)
    {
        memcpy(&_tCoverCmdqLocal[eHalCmdqId],pstCmdq,sizeof(MHAL_CMDQ_CmdqInterface_t));
    }
    else
    {
        memset(&_tCoverCmdqLocal[eHalCmdqId],0,sizeof(MHAL_CMDQ_CmdqInterface_t));
    }
    HALCOVERDBG("[GOP]%s %d: out \n", __FUNCTION__, __LINE__);
}
void HalCoverSetWindowSize(HalCoverIdType_e eCoverId, HalCoverWinIdType_e eWinId, HalCoverWindowType_t tWinCfg)
{
    u32 COVER_Reg_Base = 0;
    u32 COVER_Reg_HStart_Base = 0;
    u32 COVER_Reg_VStart_Base = 0;
    u32 COVER_Reg_HEnd_Base = 0;
    u32 COVER_Reg_VEnd_Base = 0;
    u16 COVER_Reg_DB   = 0;
    u16 u16DispImage_HStart, u16DispImage_HEnd;
    u16 u16DispImage_VStart, u16DispImage_VEnd;
    HalCoverCmdqIdType_e eCmdqId;

    COVER_Reg_Base = _HalCoverGetBaseAddr(eCoverId);
    COVER_Reg_DB = _HalCoverGetDoubleRW(eCoverId);

    switch(eCoverId)
    {
        case E_HAL_ISPSCL1_COVER_ID_00:
            if(eWinId == E_HAL_COVER_BWIN_ID_0)
            {
                COVER_Reg_HStart_Base = REG_ISPSC1_COVER_BWIN0_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC1_COVER_BWIN0_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC1_COVER_BWIN0_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC1_COVER_BWIN0_V_END;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_1)
            {
                COVER_Reg_HStart_Base = REG_ISPSC1_COVER_BWIN1_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC1_COVER_BWIN1_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC1_COVER_BWIN1_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC1_COVER_BWIN1_V_END;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_2)
            {
                COVER_Reg_HStart_Base = REG_ISPSC1_COVER_BWIN2_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC1_COVER_BWIN2_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC1_COVER_BWIN2_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC1_COVER_BWIN2_V_END;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_3)
            {
                COVER_Reg_HStart_Base = REG_ISPSC1_COVER_BWIN3_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC1_COVER_BWIN3_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC1_COVER_BWIN3_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC1_COVER_BWIN3_V_END;
            }
            else
            {
                COVER_Reg_HStart_Base = REG_ISPSC1_COVER_BWIN0_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC1_COVER_BWIN0_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC1_COVER_BWIN0_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC1_COVER_BWIN0_V_END;
                HALCOVERERR("[COVER]%s %d: wrong WinId\n", __FUNCTION__, __LINE__);
            }
            break;
        case E_HAL_ISPSCL4_COVER_ID_01:
            if(eWinId == E_HAL_COVER_BWIN_ID_0)
            {
                COVER_Reg_HStart_Base = REG_ISPSC4_COVER_BWIN0_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC4_COVER_BWIN0_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC4_COVER_BWIN0_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC4_COVER_BWIN0_V_END;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_1)
            {
                COVER_Reg_HStart_Base = REG_ISPSC4_COVER_BWIN1_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC4_COVER_BWIN1_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC4_COVER_BWIN1_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC4_COVER_BWIN1_V_END;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_2)
            {
                COVER_Reg_HStart_Base = REG_ISPSC4_COVER_BWIN2_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC4_COVER_BWIN2_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC4_COVER_BWIN2_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC4_COVER_BWIN2_V_END;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_3)
            {
                COVER_Reg_HStart_Base = REG_ISPSC4_COVER_BWIN3_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC4_COVER_BWIN3_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC4_COVER_BWIN3_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC4_COVER_BWIN3_V_END;
            }
            else
            {
                COVER_Reg_HStart_Base = REG_ISPSC4_COVER_BWIN0_H_STR;
                COVER_Reg_HEnd_Base = REG_ISPSC4_COVER_BWIN0_H_END;
                COVER_Reg_VStart_Base = REG_ISPSC4_COVER_BWIN0_V_STR;
                COVER_Reg_VEnd_Base = REG_ISPSC4_COVER_BWIN0_V_END;
                HALCOVERERR("[COVER]%s %d: wrong WinId\n", __FUNCTION__, __LINE__);
            }
            break;
        case E_HAL_DIP_COVER_ID_02:
            if(eWinId == E_HAL_COVER_BWIN_ID_0)
            {
                COVER_Reg_HStart_Base = REG_DIP_COVER_BWIN0_H_STR;
                COVER_Reg_HEnd_Base = REG_DIP_COVER_BWIN0_H_END;
                COVER_Reg_VStart_Base = REG_DIP_COVER_BWIN0_V_STR;
                COVER_Reg_VEnd_Base = REG_DIP_COVER_BWIN0_V_END;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_1)
            {
                COVER_Reg_HStart_Base = REG_DIP_COVER_BWIN1_H_STR;
                COVER_Reg_HEnd_Base = REG_DIP_COVER_BWIN1_H_END;
                COVER_Reg_VStart_Base = REG_DIP_COVER_BWIN1_V_STR;
                COVER_Reg_VEnd_Base = REG_DIP_COVER_BWIN1_V_END;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_2)
            {
                COVER_Reg_HStart_Base = REG_DIP_COVER_BWIN2_H_STR;
                COVER_Reg_HEnd_Base = REG_DIP_COVER_BWIN2_H_END;
                COVER_Reg_VStart_Base = REG_DIP_COVER_BWIN2_V_STR;
                COVER_Reg_VEnd_Base = REG_DIP_COVER_BWIN2_V_END;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_3)
            {
                COVER_Reg_HStart_Base = REG_DIP_COVER_BWIN3_H_STR;
                COVER_Reg_HEnd_Base = REG_DIP_COVER_BWIN3_H_END;
                COVER_Reg_VStart_Base = REG_DIP_COVER_BWIN3_V_STR;
                COVER_Reg_VEnd_Base = REG_DIP_COVER_BWIN3_V_END;
            }
            else
            {
                COVER_Reg_HStart_Base = REG_DIP_COVER_BWIN0_H_STR;
                COVER_Reg_HEnd_Base = REG_DIP_COVER_BWIN0_H_END;
                COVER_Reg_VStart_Base = REG_DIP_COVER_BWIN0_V_STR;
                COVER_Reg_VEnd_Base = REG_DIP_COVER_BWIN0_V_END;
                HALCOVERERR("[COVER]%s %d: wrong WinId\n", __FUNCTION__, __LINE__);
            }
            break;
        default:
            COVER_Reg_HStart_Base = REG_ISPSC1_COVER_BWIN0_H_STR;
            COVER_Reg_HEnd_Base = REG_ISPSC1_COVER_BWIN0_H_END;
            COVER_Reg_VStart_Base = REG_ISPSC1_COVER_BWIN0_V_STR;
            COVER_Reg_VEnd_Base = REG_ISPSC1_COVER_BWIN0_V_END;
            HALCOVERERR("[COVER]%s %d: wrong COVERId\n", __FUNCTION__, __LINE__);
            break;

    }

    u16DispImage_HStart = tWinCfg.u16X;
    u16DispImage_VStart = tWinCfg.u16Y;
    u16DispImage_HEnd   = tWinCfg.u16X + tWinCfg.u16Width;
    u16DispImage_VEnd   = tWinCfg.u16Y + tWinCfg.u16Height;

    _HalCoverIdTransCmdqID(eCoverId,&eCmdqId);

    // cover bwin settings
    _HalCoverWrite2Byte(COVER_Reg_Base + COVER_Reg_HStart_Base,u16DispImage_HStart,eCmdqId);
    _HalCoverWrite2Byte(COVER_Reg_Base + COVER_Reg_HEnd_Base, u16DispImage_HEnd,eCmdqId);
    _HalCoverWrite2Byte(COVER_Reg_Base + COVER_Reg_VStart_Base, u16DispImage_VStart,eCmdqId);
    _HalCoverWrite2Byte(COVER_Reg_Base + COVER_Reg_VEnd_Base, u16DispImage_VEnd,eCmdqId);

    // Double Buffer Write
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_DB, 0, MSK_COVER_BWIN_TRIG,eCmdqId);
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_DB, 1, MSK_COVER_BWIN_TRIG,eCmdqId);
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_DB, 0, MSK_COVER_BWIN_TRIG,eCmdqId);

}

void HalCoverSetColor(HalCoverIdType_e eCoverId, HalCoverWinIdType_e eWinId, u8 u8R, u8 u8G, u8 u8B)
{
    u32 COVER_Reg_Base = 0;
    u16 COVER_Reg_DB   = 0;
    u32 COVER_Reg_Replace_R_Base = 0;
    u32 COVER_Reg_Replace_G_Base = 0;
    u32 COVER_Reg_Replace_B_Base = 0;
    HalCoverCmdqIdType_e eCmdqId;

    COVER_Reg_DB = _HalCoverGetDoubleRW(eCoverId);
    COVER_Reg_Base = _HalCoverGetBaseAddr(eCoverId);

    switch(eCoverId)
    {
        case E_HAL_ISPSCL1_COVER_ID_00:
            if(eWinId == E_HAL_COVER_BWIN_ID_0)
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC1_COVER_BWIN0_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC1_COVER_BWIN0_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC1_COVER_BWIN0_REPLACE_B;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_1)
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC1_COVER_BWIN1_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC1_COVER_BWIN1_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC1_COVER_BWIN1_REPLACE_B;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_2)
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC1_COVER_BWIN2_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC1_COVER_BWIN2_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC1_COVER_BWIN2_REPLACE_B;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_3)
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC1_COVER_BWIN3_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC1_COVER_BWIN3_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC1_COVER_BWIN3_REPLACE_B;
            }
            else
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC1_COVER_BWIN0_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC1_COVER_BWIN0_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC1_COVER_BWIN0_REPLACE_B;
                HALCOVERERR("[COVER]%s %d: wrong WinId\n", __FUNCTION__, __LINE__);
            }
            break;
        case E_HAL_ISPSCL4_COVER_ID_01:
            if(eWinId == E_HAL_COVER_BWIN_ID_0)
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC4_COVER_BWIN0_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC4_COVER_BWIN0_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC4_COVER_BWIN0_REPLACE_B;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_1)
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC4_COVER_BWIN1_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC4_COVER_BWIN1_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC4_COVER_BWIN1_REPLACE_B;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_2)
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC4_COVER_BWIN2_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC4_COVER_BWIN2_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC4_COVER_BWIN2_REPLACE_B;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_3)
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC4_COVER_BWIN3_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC4_COVER_BWIN3_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC4_COVER_BWIN3_REPLACE_B;
            }
            else
            {
                COVER_Reg_Replace_R_Base = REG_ISPSC4_COVER_BWIN0_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_ISPSC4_COVER_BWIN0_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_ISPSC4_COVER_BWIN0_REPLACE_B;
                HALCOVERERR("[COVER]%s %d: wrong WinId\n", __FUNCTION__, __LINE__);
            }
            break;
        case E_HAL_DIP_COVER_ID_02:
            if(eWinId == E_HAL_COVER_BWIN_ID_0)
            {
                COVER_Reg_Replace_R_Base = REG_DIP_COVER_BWIN0_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_DIP_COVER_BWIN0_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_DIP_COVER_BWIN0_REPLACE_B;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_1)
            {
                COVER_Reg_Replace_R_Base = REG_DIP_COVER_BWIN1_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_DIP_COVER_BWIN1_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_DIP_COVER_BWIN1_REPLACE_B;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_2)
            {
                COVER_Reg_Replace_R_Base = REG_DIP_COVER_BWIN2_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_DIP_COVER_BWIN2_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_DIP_COVER_BWIN2_REPLACE_B;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_3)
            {
                COVER_Reg_Replace_R_Base = REG_DIP_COVER_BWIN3_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_DIP_COVER_BWIN3_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_DIP_COVER_BWIN3_REPLACE_B;
            }
            else
            {
                COVER_Reg_Replace_R_Base = REG_DIP_COVER_BWIN0_REPLACE_R;
                COVER_Reg_Replace_G_Base = REG_DIP_COVER_BWIN0_REPLACE_G;
                COVER_Reg_Replace_B_Base = REG_DIP_COVER_BWIN0_REPLACE_B;
                HALCOVERERR("[COVER]%s %d: wrong WinId\n", __FUNCTION__, __LINE__);
            }
            break;
        default:
            COVER_Reg_Replace_R_Base = REG_ISPSC1_COVER_BWIN0_REPLACE_R;
            COVER_Reg_Replace_G_Base = REG_ISPSC1_COVER_BWIN0_REPLACE_G;
            COVER_Reg_Replace_B_Base = REG_ISPSC1_COVER_BWIN0_REPLACE_B;
            HALCOVERERR("[COVER]%s %d: wrong COVERId\n", __FUNCTION__, __LINE__);
            break;

    }

    _HalCoverIdTransCmdqID(eCoverId,&eCmdqId);

    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_Replace_R_Base, u8R, MSK_COVER_BWIN_REPLACE_R,eCmdqId);
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_Replace_G_Base, (u8G<<8) , MSK_COVER_BWIN_REPLACE_G,eCmdqId);
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_Replace_B_Base, u8B, MSK_COVER_BWIN_REPLACE_B,eCmdqId);

    // Double Buffer Write
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_DB, 0, MSK_COVER_BWIN_TRIG,eCmdqId);
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_DB, 1, MSK_COVER_BWIN_TRIG,eCmdqId);
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_DB, 0, MSK_COVER_BWIN_TRIG,eCmdqId);
}

void HalCoverSetEnableWin(HalCoverIdType_e eCoverId, HalCoverWinIdType_e eWinId, bool bEn)
{
    u32 COVER_Reg_Base = 0;
    u16 COVER_Reg_DB   = 0;
    u32 COVER_Reg_Enable_Base = 0;
    HalCoverCmdqIdType_e eCmdqId;

    COVER_Reg_DB = _HalCoverGetDoubleRW(eCoverId);
    COVER_Reg_Base = _HalCoverGetBaseAddr(eCoverId);

    switch(eCoverId)
    {
        case E_HAL_ISPSCL1_COVER_ID_00:
            if(eWinId == E_HAL_COVER_BWIN_ID_0)
            {
                COVER_Reg_Enable_Base = REG_ISPSC1_COVER_BWIN0_EN;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_1)
            {
                COVER_Reg_Enable_Base = REG_ISPSC1_COVER_BWIN1_EN;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_2)
            {
                COVER_Reg_Enable_Base = REG_ISPSC1_COVER_BWIN2_EN;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_3)
            {
                COVER_Reg_Enable_Base = REG_ISPSC1_COVER_BWIN3_EN;
            }
            else
            {
                COVER_Reg_Enable_Base = REG_ISPSC1_COVER_BWIN0_EN;
                HALCOVERERR("[COVER]%s %d: wrong WinId\n", __FUNCTION__, __LINE__);
            }
            break;
        case E_HAL_ISPSCL4_COVER_ID_01:
            if(eWinId == E_HAL_COVER_BWIN_ID_0)
            {
                COVER_Reg_Enable_Base = REG_ISPSC4_COVER_BWIN0_EN;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_1)
            {
                COVER_Reg_Enable_Base = REG_ISPSC4_COVER_BWIN1_EN;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_2)
            {
                COVER_Reg_Enable_Base = REG_ISPSC4_COVER_BWIN2_EN;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_3)
            {
                COVER_Reg_Enable_Base = REG_ISPSC4_COVER_BWIN3_EN;
            }
            else
            {
                COVER_Reg_Enable_Base = REG_ISPSC4_COVER_BWIN0_EN;
                HALCOVERERR("[COVER]%s %d: wrong WinId\n", __FUNCTION__, __LINE__);
            }
            break;
        case E_HAL_DIP_COVER_ID_02:
            if(eWinId == E_HAL_COVER_BWIN_ID_0)
            {
                COVER_Reg_Enable_Base = REG_DIP_COVER_BWIN0_EN;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_1)
            {
                COVER_Reg_Enable_Base = REG_DIP_COVER_BWIN1_EN;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_2)
            {
                COVER_Reg_Enable_Base = REG_DIP_COVER_BWIN2_EN;
            }
            else if(eWinId == E_HAL_COVER_BWIN_ID_3)
            {
                COVER_Reg_Enable_Base = REG_DIP_COVER_BWIN3_EN;
            }
            else
            {
                COVER_Reg_Enable_Base = REG_DIP_COVER_BWIN0_EN;
                HALCOVERERR("[COVER]%s %d: wrong WinId\n", __FUNCTION__, __LINE__);
            }
            break;
        default:
            COVER_Reg_Enable_Base = REG_ISPSC1_COVER_BWIN0_EN;
            HALCOVERERR("[COVER]%s %d: wrong COVERId\n", __FUNCTION__, __LINE__);
            break;

    }

    _HalCoverIdTransCmdqID(eCoverId,&eCmdqId);
    // enable/disable gwin
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_Enable_Base, bEn ? 1 : 0,MSK_COVER_BWIN_EN,eCmdqId);

    // Double Buffer Write
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_DB, 0, MSK_COVER_BWIN_TRIG,eCmdqId);
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_DB, 1, MSK_COVER_BWIN_TRIG,eCmdqId);
    _HalCoverWrite2ByteMsk(COVER_Reg_Base + COVER_Reg_DB, 0, MSK_COVER_BWIN_TRIG,eCmdqId);
}
