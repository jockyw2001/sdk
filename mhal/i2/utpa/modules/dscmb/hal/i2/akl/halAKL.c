//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2014 MStar Semiconductor, Inc. All rights reserved.
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
// Copyright (c) 2014 MStar Semiconductor, Inc.
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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file   halAKL.c
/// @brief  AKL HAL Interface
/// @author MStar Semiconductor,Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#else
#include "string.h"
#endif
#include "MsCommon.h"
#include "MsTypes.h"
#include "MsDevice.h"

#include "drvAKL.h"
#include "halAKL.h"
#include "regAKL.h"

//--------------------------------------------------------------------------------------------------
//  Driver Compiler Option
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
static MS_U32   _u32RIUBase    = 0x0;
static MS_U32   _u32AKLBase    = 0x0;
static MS_U32   _u32CERTBase   = 0x0;

#define AKL0_REG32(_offset) (*(volatile unsigned int *  )(_u32AKLBase+((_offset)<<2)))
#define AKL1_REG32(_offset) (*(volatile unsigned int *  )(_u32CERTBase+((_offset)<<2)))

//--------------------------------------------------------------------------------------------------
//  Global Variable
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Internal Variable
//--------------------------------------------------------------------------------------------------
static AKL_DBGMSG_LEVEL _HAL_AKLDBGLevel = E_AKL_DBG_ERR;

//--------------------------------------------------------------------------------------------------
//  Debug Function
//--------------------------------------------------------------------------------------------------
#define HAL_DEBUG_AKL(debug_level, fmt, args...) do{ if (_HAL_AKLDBGLevel >= (debug_level))\
        printf("[%s][%d]"fmt"\n", __FUNCTION__, __LINE__,  ## args);}\
        while(0)

//--------------------------------------------------------------------------------------------------
//  Internal Function
//--------------------------------------------------------------------------------------------------
MS_U8 _HAL_AKL_Trans_Eng(AKL_Eng_Type eEngType)
{
    if(E_AKL_ENG_LSAS == eEngType)       return HAL_AKL_ENG_LSAS;
    else if(E_AKL_ENG_LSAD == eEngType)  return HAL_AKL_ENG_LSAD;
    else if(E_AKL_ENG_ESA == eEngType)   return HAL_AKL_ENG_ESA;
    else return HAL_AKL_ENG_ESA; //default
}

MS_U8 _HAL_AKL_Trans_Key(AKL_Key_Type eKeyType)
{
    if(E_AKL_KEY_CLEAR == eKeyType)       return HAL_AKL_KEY_CLEAR;
    else if(E_AKL_KEY_EVEN == eKeyType)  return HAL_AKL_KEY_EVEN;
    else if(E_AKL_KEY_ODD == eKeyType)   return HAL_AKL_KEY_ODD;
    else return HAL_AKL_KEY_EVEN; //default
}

void HAL_AKL_SetBank(MS_U32 u32Bank)
{
    _u32RIUBase  = u32Bank;
    _u32AKLBase  = _u32RIUBase + (REG_AKL0_BANK<<9);
    _u32CERTBase = _u32RIUBase + (REG_AKL1_BANK<<9);

    return;
}

void HAL_AKL_Reset(void)
{
    AKL0_REG32(REG_AKL0_RSTN) |= (REG_AKL0_RSTN_MSK|REG_AKL0_ND_RSTN_MSK);
    return;
}

void HAL_AKL_SetDbgLevel(AKL_DBGMSG_LEVEL eDbgLevel)
{
    _HAL_AKLDBGLevel = eDbgLevel;
    HAL_DEBUG_AKL(E_AKL_DBG_INFO, "Set Debug Level = [%d]\n", _HAL_AKLDBGLevel);

    return;
}

MS_BOOL HAL_AKL_GetKeyValid(void)
{
    if(0 == (AKL0_REG32(REG_AKL0_VALID)&REG_AKL0_VALID_MSK))
    {
        HAL_DEBUG_AKL(E_AKL_DBG_WARN, "Key is not valid\n");
        return FALSE;
    }

    return TRUE;
}

MS_BOOL HAL_AKL_SetManualAck(void)
{
    AKL0_REG32(REG_AKL0_MACK) |= REG_AKL0_MACK_MSK;
    HAL_DEBUG_AKL(E_AKL_DBG_INFO, "Set manual ack\n");

    return TRUE;
}

MS_BOOL HAL_AKL_GetKTAck(void)
{
    if(0==(AKL0_REG32(REG_AKL0_ACK)&REG_AKL0_ACK_MSK))
    {
        HAL_DEBUG_AKL(E_AKL_DBG_WARN, "KT no ack\n");
        return FALSE;
    }

    return TRUE;
}

MS_BOOL HAL_AKL_GetDMAAck(void)
{
    if(0==(AKL0_REG32(REG_AKL0_DMA_ACK)&REG_AKL0_DMA_ACK_MSK))
    {
        HAL_DEBUG_AKL(E_AKL_DBG_WARN, "DMA no ack\n");
        return FALSE;
    }

    return TRUE;
}

MS_BOOL HAL_AKL_GetTSIOAck(void)
{
    if(0==(AKL0_REG32(REG_AKL0_TSIO_ACK)&REG_AKL0_TSIO_ACK_MSK))
    {
        HAL_DEBUG_AKL(E_AKL_DBG_WARN, "TSIO no ack\n");
        return FALSE;
    }

    return TRUE;
}

MS_BOOL HAL_AKL_GetKTResponse(void)
{
    if(0 == ((AKL0_REG32(REG_AKL0_DONE)&REG_AKL0_DONE_MSK)>>REG_AKL0_DONE_SFT))
    {
        HAL_DEBUG_AKL(E_AKL_DBG_WARN, "KT not done\n");
        return FALSE;
    }

    return TRUE;
}

MS_BOOL HAL_AKL_GetDMAResponse(void)
{
    if(0 == ((AKL0_REG32(REG_AKL0_DONE)&REG_AKL0_DMA_DONE_MSK)>>REG_AKL0_DMA_DONE_SFT))
    {
        HAL_DEBUG_AKL(E_AKL_DBG_WARN, "DMA not done\n");
        return FALSE;
    }

    return TRUE;
}

MS_BOOL HAL_AKL_GetTSIOResponse(void)
{
    if(0 == ((AKL0_REG32(REG_AKL0_DONE)&REG_AKL0_TSIO_DONE_MSK)>>REG_AKL0_TSIO_DONE_SFT))
    {
        HAL_DEBUG_AKL(E_AKL_DBG_WARN, "TSIO not done\n");
        return FALSE;
    }

    return TRUE;
}

MS_BOOL HAL_AKL_SetKTKey(MS_U32 u32EngId, AKL_Eng_Type eEngType, MS_U32 u32Index, AKL_Key_Type eKeyType)
{
    MS_U32 u32Val = 0;
    MS_U8 u8Field = 0;
    MS_U8 u8SCB   = 0;

    u8Field = _HAL_AKL_Trans_Eng(eEngType);
    u8SCB   = _HAL_AKL_Trans_Key(eKeyType);

    u32Val = ((u8Field << REG_AKL0_FIELD_SFT) & REG_AKL0_FIELD_MSK)
            |((u32Index << REG_AKL0_PIDNO_SFT) & REG_AKL0_PIDNO_MSK)
            |((u8SCB   << REG_AKL0_SCB_SFT  ) & REG_AKL0_SCB_MSK)
            | 0x1; /*go*/

    AKL0_REG32(REG_AKL0_KT_GO) = u32Val;

    HAL_DEBUG_AKL(E_AKL_DBG_INFO, "Set KT key field[0x%x], index[0x%x], scb[0x%x]\n",
        u8Field, (unsigned int)u32Val, u8SCB);

    return TRUE;
}

MS_BOOL HAL_AKL_SetDMAKey(MS_U32 u32KeyIdx)
{
    MS_U32 u32Val = 0;

    u32Val = ((u32KeyIdx << REG_AKL0_DMAIdx_SFT) & REG_AKL0_DMAIdx_MSK)
             | 0x1; /*go*/

    AKL0_REG32(REG_AKL0_DMA_GO) = u32Val;

    HAL_DEBUG_AKL(E_AKL_DBG_INFO, "Set DMA key index[0x%x]\n", (unsigned int)u32KeyIdx);

    return TRUE;
}

MS_BOOL HAL_AKL_SetDMAParserKey(MS_U32 u32OddKeyIdx, MS_U32 u32EvenKeyIdx)
{
    HAL_DEBUG_AKL(E_AKL_DBG_INFO, "Not implemented\n");
    return TRUE;
}

MS_BOOL HAL_AKL_SetTSIOKey(MS_U32 u32ServiceId, AKL_Key_Type eKeyType)
{
    MS_U32 u32Val = 0;
    MS_U8 u8Scb = 0;

    u8Scb   = _HAL_AKL_Trans_Key(eKeyType);

    u32Val = ((u32ServiceId << REG_AKL0_PIDNO_SFT) & REG_AKL0_PIDNO_MSK)
            |((u8Scb << REG_AKL0_SCB_SFT  ) & REG_AKL0_SCB_MSK)
            | (0x1 << REG_AKL0_TSIO_GO_SFT); /*go*/

    AKL0_REG32(REG_AKL0_TSIO_GO) = u32Val;

    return TRUE;
}

MS_BOOL HAL_AKL_SetManualAckMode(MS_BOOL bEnable)
{
    MS_U32  u32Val = AKL0_REG32(REG_AKL0_MACK_En);

    if(bEnable){
        AKL0_REG32(REG_AKL0_MACK_En) = (u32Val|REG_AKL0_MACK_En_MSK);
    }
    else{
        AKL0_REG32(REG_AKL0_MACK_En) = (u32Val&(~REG_AKL0_MACK_En_MSK));
    }

    HAL_DEBUG_AKL(E_AKL_DBG_INFO, "Set manual ack mode[%d]\n", bEnable);

    return TRUE;
}

MS_BOOL HAL_AKL_CERTCmd(HAL_AKL_CERT_Cmd eCmd, MS_U8 *pu8Data, MS_U32 u32DataSize, MS_BOOL bRead)
{
    MS_U32 u32CmdBase = 0;
    MS_U32 u32Data = 0;
    MS_U32 i = 0;

    if(NULL == pu8Data || 0 == u32DataSize)
    {
        HAL_DEBUG_AKL(E_AKL_DBG_ERR, "Invalid data\n");
        return FALSE;
    }

    switch(eCmd){
        case E_HAL_AKL_CERT_CMD_INPUT:
            u32CmdBase = REG_AKL1_INPUT;
            break;

        case E_HAL_AKL_CERT_CMD_OUTPUT:
            u32CmdBase = REG_AKL1_OUTPUT;
            break;

        case E_HAL_AKL_CERT_CMD_OPCODE:
            u32CmdBase = REG_AKL1_COMMAND;
            break;

        case E_HAL_AKL_CERT_CMD_STATUS:
            u32CmdBase = REG_AKL1_STATUS;
            break;

        default:
            HAL_DEBUG_AKL(E_AKL_DBG_ERR, "Invalid command\n");
            return FALSE;
    }

    for(i = 0;i < u32DataSize/4;i++)
    {
        if(bRead)
        {
            u32Data = AKL1_REG32(u32CmdBase + i);
            pu8Data[i * 4 + 0] = ((u32Data >> 24) & 0xFF);
            pu8Data[i * 4 + 1] = ((u32Data >> 16) & 0xFF);
            pu8Data[i * 4 + 2] = ((u32Data >> 8 ) & 0xFF);
            pu8Data[i * 4 + 3] = ((u32Data >> 0 ) & 0xFF);

        }
        else
        {
            AKL1_REG32(u32CmdBase + i) = ((pu8Data[i * 4 + 0]) << 24)
                                           |((pu8Data[i * 4 + 1]) << 16)
                                           |((pu8Data[i * 4 + 2]) << 8)
                                           |((pu8Data[i * 4 + 3]) << 0);
        }

    }

    return TRUE;

}

void HAL_AKL_EnableDMAKey(MS_BOOL bEnable)
{
    HAL_DEBUG_AKL(E_AKL_DBG_INFO, "Not support\n");

}

void HAL_AKL_EnableKTKey(MS_BOOL bEnable, MS_U32 u32EngId)
{
    HAL_DEBUG_AKL(E_AKL_DBG_INFO, "Not support\n");
}

