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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file   drvDSCMB.c
/// @brief  Descrambler (DSCMB) Driver Interface
/// @author MStar Semiconductor,Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MsCommon.h"
#include "MsVersion.h"
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#include <linux/limits.h>
#else
#include <limits.h>
#include <string.h>
#endif

#include "drvMMIO.h"

#include "drvAKL.h"
#include "regAKL.h"
#include "halAKL.h"
#include "asmCPU.h"
#include "utopia.h"

#ifdef AKL_UTOPIA_20
#define  AKL_UTOPIA20
#else
#undef   AKL_UTOPIA20
#endif

#ifdef AKL_UTOPIA20
#include "drvAKL_v2.h"
#endif

#include "drvAKL_private.h"

//--------------------------------------------------------------------------------------------------
//  Driver Compiler Option
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Local Defines
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Global Variable
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Internal Variable
//--------------------------------------------------------------------------------------------------
static MS_BOOL  bdrvAKLInited  = FALSE;
static AKL_DBGMSG_LEVEL _AKLDBGLevel = E_AKL_DBG_ERR;

#ifdef AKL_UTOPIA20
static void *pu32AKLInst = NULL;
#endif

//--------------------------------------------------------------------------------------------------
//  Debug Function
//--------------------------------------------------------------------------------------------------
#define DRV_AKL_ASSERT( _bool_, _msg_, ret) if(!(_bool_)){ printf(_msg_); return (ret);}
#define DRV_AKL_INIT_CHECK(ret) DRV_AKL_ASSERT(TRUE == bdrvAKLInited, "drvAKL is not initialized\n", ret)

#define DEBUG_AKL(debug_level, fmt, args...) do{ if (_AKLDBGLevel >= (debug_level))\
        printf("[%s][%d]"fmt"\n", __FUNCTION__, __LINE__,  ## args);}\
        while(0)

//--------------------------------------------------------------------------------------------------
//  Internal Function
//--------------------------------------------------------------------------------------------------
MS_BOOL _MDrv_AKL_Init(void)
{
    MS_VIRT u32Bank = 0;
    MS_PHY u32BankSize = 0;

    if(!bdrvAKLInited)
    {
        if (FALSE == MDrv_MMIO_GetBASE(&u32Bank, &u32BankSize, MS_MODULE_PM))
        {
            DEBUG_AKL(E_AKL_DBG_ERR, "MDrv_MMIO_GetBASE fail\n");
            return FALSE;
        }

        HAL_AKL_SetBank(u32Bank);
        HAL_AKL_Reset();

        bdrvAKLInited = TRUE;
    }
    else
    {
        DEBUG_AKL(E_AKL_DBG_WARN, "AKL driver already inited\n");
    }

    return TRUE;
}

MS_BOOL _MDrv_AKL_Reset(void)
{
    DRV_AKL_INIT_CHECK(FALSE);

    HAL_AKL_Reset();

    DEBUG_AKL(E_AKL_DBG_WARN, "AKL Reset\n");

    return TRUE;
}

MS_BOOL _MDrv_AKL_SetDbgLevel(AKL_DBGMSG_LEVEL eDbgLevel)
{
    _AKLDBGLevel = eDbgLevel;
    HAL_AKL_SetDbgLevel(eDbgLevel);

    DEBUG_AKL(E_AKL_DBG_INFO, "Set Debug Level = [%d]\n", _AKLDBGLevel);

    return TRUE;
}

MS_BOOL _MDrv_AKL_SetManualACK(void)
{
    MS_BOOL bRet = TRUE;
    DRV_AKL_INIT_CHECK(FALSE);

    bRet = HAL_AKL_SetManualAck();

    return bRet;
}

DRV_AKL_RET _MDrv_AKL_SetDMAKey(MS_U32 u32KeyIndex)
{
    MS_U32 u32TimeOut = 0;
    DRV_AKL_INIT_CHECK(FALSE);

    if(REG_AKL_KS_INDEX_MAX < u32KeyIndex)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "Invalid key index[0x%x]\n", (unsigned int)u32KeyIndex);
        return DRV_AKL_BAD_PARAM;
    }

    //Check CERT Key valid
    if(FALSE == HAL_AKL_GetKeyValid())
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "Key is not valid\n");
        return DRV_AKL_FAIL;
    }

    // Set index & Go
    if(FALSE == HAL_AKL_SetDMAKey(u32KeyIndex))
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "Set DMA key index[0x%x] fail\n", (unsigned int)u32KeyIndex);
        return DRV_AKL_FAIL;
    }

    HAL_AKL_EnableDMAKey(TRUE);

    //// Polling AKL_ack
    while((TRUE != HAL_AKL_GetDMAAck()) && (u32TimeOut < AKL_SET_KEY_TIMEOUT_VALUE))
    {
        MsOS_DelayTask(1);
        u32TimeOut++;
    }

    if(AKL_SET_KEY_TIMEOUT_VALUE == u32TimeOut)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "AKL set key to DMA timeout!\n");
        HAL_AKL_EnableDMAKey(FALSE);
        return DRV_AKL_TIMEOUT;
    }

    u32TimeOut = 0;

    //// Polling Done
    while((HAL_AKL_DONE != HAL_AKL_GetDMAResponse()) && (u32TimeOut < AKL_SET_KEY_TIMEOUT_VALUE))
    {
        MsOS_DelayTask(1);
        u32TimeOut++;
    }

    HAL_AKL_EnableDMAKey(FALSE);

    if(AKL_SET_KEY_TIMEOUT_VALUE == u32TimeOut)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "AKL set key to DMA timeout!\n");
        return DRV_AKL_TIMEOUT;
    }

    return DRV_AKL_OK;
}

DRV_AKL_RET _MDrv_AKL_SetDMAParserKey(MS_U32 u32OddKeyIndex, MS_U32 u32EvenKeyIndex)
{
    MS_U32 u32TimeOut = 0;
    DRV_AKL_INIT_CHECK(FALSE);

    if(REG_AKL_KS_INDEX_MAX < u32OddKeyIndex || REG_AKL_KS_INDEX_MAX < u32EvenKeyIndex)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "Invalid key index odd[0x%x], even[0x%x]\n", (unsigned int)u32OddKeyIndex, (unsigned int)u32EvenKeyIndex);
        return DRV_AKL_BAD_PARAM;
    }

    //Check CERT Key valid
    if(FALSE == HAL_AKL_GetKeyValid())
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "Key is not valid\n");
        return DRV_AKL_FAIL;
    }

    // Set index & Go
    if(FALSE == HAL_AKL_SetDMAParserKey(u32OddKeyIndex, u32EvenKeyIndex))
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "Set DMA Parser key index odd[0x%x], even[0x%x] fail\n", (unsigned int)u32OddKeyIndex, (unsigned int)u32EvenKeyIndex);
        return DRV_AKL_FAIL;
    }

    HAL_AKL_EnableDMAKey(TRUE);

    //// Polling AKL_ack
    while((TRUE != HAL_AKL_GetDMAAck()) && (u32TimeOut < AKL_SET_KEY_TIMEOUT_VALUE))
    {
        MsOS_DelayTask(1);
        u32TimeOut++;
    }

    if(AKL_SET_KEY_TIMEOUT_VALUE == u32TimeOut)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "AKL set key to DMA Parser timeout!\n");
        HAL_AKL_EnableDMAKey(FALSE);
        return DRV_AKL_TIMEOUT;
    }

    u32TimeOut = 0;

    //// Polling Done
    while((HAL_AKL_DONE != HAL_AKL_GetDMAResponse()) && (u32TimeOut < AKL_SET_KEY_TIMEOUT_VALUE))
    {
        MsOS_DelayTask(1);
        u32TimeOut++;
    }

    HAL_AKL_EnableDMAKey(FALSE);

    if(AKL_SET_KEY_TIMEOUT_VALUE == u32TimeOut)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "AKL set key to DMA Parser timeout!\n");
        return DRV_AKL_TIMEOUT;
    }

    return DRV_AKL_OK;
}

DRV_AKL_RET _MDrv_AKL_SetDSCMBKey(MS_U32 u32EngId, MS_U32 u32Index, AKL_Eng_Type eEngType, AKL_Key_Type eKeyType)
{
    MS_U32 u32TimeOut = 0;
    DRV_AKL_INIT_CHECK(FALSE);

    //Check CERT Key valid
    if(FALSE == HAL_AKL_GetKeyValid())
        return DRV_AKL_FAIL;

    // Set PIDNo, Field, SCB & Go
    if(FALSE == HAL_AKL_SetKTKey(u32EngId, eEngType, u32Index, eKeyType))
    {
        return DRV_AKL_FAIL;
    }

    HAL_AKL_EnableKTKey(TRUE, u32EngId);

    //// Polling AKL_ack
    while((TRUE != HAL_AKL_GetKTAck()) && (u32TimeOut < AKL_SET_KEY_TIMEOUT_VALUE))
    {
        MsOS_DelayTask(1);
        u32TimeOut++;
    }

    if(AKL_SET_KEY_TIMEOUT_VALUE == u32TimeOut)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "AKL set key to DSCMB timeout!\n");
        HAL_AKL_EnableKTKey(FALSE, u32EngId);
        return DRV_AKL_TIMEOUT;
    }

    u32TimeOut = 0;
    while((HAL_AKL_DONE != HAL_AKL_GetKTResponse()) && (u32TimeOut < AKL_SET_KEY_TIMEOUT_VALUE))
    {
        MsOS_DelayTask(1);
        u32TimeOut++;
    }

    HAL_AKL_EnableKTKey(FALSE, u32EngId);

    if(AKL_SET_KEY_TIMEOUT_VALUE == u32TimeOut)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "AKL set key to DSCMB timeout!\n");
        return DRV_AKL_TIMEOUT;
    }

    return DRV_AKL_OK;

}

DRV_AKL_RET _MDrv_AKL_SetTSIOKey(MS_U32 u32ServiceId, AKL_Key_Type eKeyType)
{
    MS_U32 u32TimeOut = 0;
    DRV_AKL_INIT_CHECK(FALSE);

    //Check CERT Key valid
    if(FALSE == HAL_AKL_GetKeyValid())
        return DRV_AKL_FAIL;

    // Set index & Go
    if( HAL_AKL_SetTSIOKey(u32ServiceId, eKeyType))
    {
        return DRV_AKL_FAIL;
    }

    //// Polling AKL_ack
    while((TRUE != HAL_AKL_GetTSIOAck()) && (u32TimeOut < AKL_SET_KEY_TIMEOUT_VALUE))
    {
        MsOS_DelayTask(1);
        u32TimeOut++;
    }

    if(AKL_SET_KEY_TIMEOUT_VALUE == u32TimeOut)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "AKL set key to TSIO timeout!\n");
        return DRV_AKL_TIMEOUT;
    }

    u32TimeOut = 0;
    while((HAL_AKL_DONE != HAL_AKL_GetTSIOResponse()) && (u32TimeOut < AKL_SET_KEY_TIMEOUT_VALUE))
    {
        MsOS_DelayTask(1);
        u32TimeOut++;
    }

    if(AKL_SET_KEY_TIMEOUT_VALUE == u32TimeOut)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "AKL set key to TSIO timeout!\n");
        return DRV_AKL_TIMEOUT;
    }

    return DRV_AKL_OK;
}

MS_BOOL _MDrv_AKL_SetManualACKMode(MS_BOOL bEnable)
{
    MS_BOOL bRet = TRUE;
    DRV_AKL_INIT_CHECK(FALSE);

    bRet = HAL_AKL_SetManualAckMode(bEnable);

    return bRet;
}

DRV_AKL_RET _MDrv_AKL_CERTCmd(AKL_CERT_Cmd eCmd, MS_U8 *pu8Data, MS_U32 u32DataSize)
{
    MS_BOOL bRead = 0;
    MS_U32 u32CmdSize = 0;
    HAL_AKL_CERT_Cmd eCERTCmd = 0;

    DRV_AKL_INIT_CHECK(FALSE);

    if(NULL == pu8Data || 0 == u32DataSize)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "pu8Data is NULL, u32DataSize is 0!\n");
        return DRV_AKL_BAD_PARAM;
    }

    switch(eCmd){
        case E_AKL_CERT_CMD_SET_INPUT:
            eCERTCmd = E_HAL_AKL_CERT_CMD_INPUT;
            u32CmdSize = HAL_AKL_INPUT_SIZE;
            bRead = FALSE;
            break;

        case E_AKL_CERT_CMD_SET_OPCODE:
            eCERTCmd = E_HAL_AKL_CERT_CMD_OPCODE;
            u32CmdSize = HAL_AKL_OPCODE_SIZE;
            bRead = FALSE;
            break;

        case E_AKL_CERT_CMD_GET_OPCODE:
            eCERTCmd = E_HAL_AKL_CERT_CMD_OPCODE;
            u32CmdSize = HAL_AKL_OPCODE_SIZE;
            bRead = TRUE;
            break;

        case E_AKL_CERT_CMD_GET_STATUS:
            eCERTCmd = E_HAL_AKL_CERT_CMD_STATUS;
            u32CmdSize = HAL_AKL_STATUS_SIZE;
            bRead = TRUE;
            break;

        case E_AKL_CERT_CMD_GET_OUPUT:
            eCERTCmd = E_HAL_AKL_CERT_CMD_OUTPUT;
            u32CmdSize = HAL_AKL_OUTPUT_SIZE;
            bRead = TRUE;
            break;

        default:
            DEBUG_AKL(E_AKL_DBG_ERR, "Unknown AKL command[0x%x]\n", (unsigned int)eCmd);
            return DRV_AKL_BAD_PARAM;
    }

    if(u32CmdSize != u32DataSize)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "Wrong data size[0x%x]\n", (unsigned int)u32DataSize);
        return DRV_AKL_BAD_PARAM;
    }

    if(FALSE == HAL_AKL_CERTCmd(eCERTCmd, pu8Data, u32DataSize, bRead))
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "Set CERT command fail\n");
        return DRV_AKL_FAIL;
    }

    return DRV_AKL_OK;
}


MS_BOOL MDrv_AKL_Init(void)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;

    if(pu32AKLInst == NULL)
    {
        if(UTOPIA_STATUS_FAIL == UtopiaOpen(MODULE_AKL, &pu32AKLInst, 0x0, NULL))
        {
            DRVDSCMB2_DBG(DSCMB2_DBGLV_ERR, "UtopiaOpen AKL fail\n");
            return FALSE;
        }
    }

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_Init, NULL);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return FALSE;
    }

    return TRUE;
#else
    return _MDrv_AKL_Init();
#endif
}

MS_BOOL MDrv_AKL_Reset(void)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;

    if(pu32AKLInst == NULL)
    {
        return FALSE;
    }

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_Reset, NULL);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return FALSE;
    }

    return TRUE;

#else
    return _MDrv_AKL_Reset();
#endif
}

MS_BOOL MDrv_AKL_SetDbgLevel(AKL_DBGMSG_LEVEL eDbgLevel)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;
    AKL_DBGLEVEL pArgs;

    if(pu32AKLInst == NULL)
    {
        return FALSE;
    }

    pArgs.eDBGMsgLevel = eDbgLevel;

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_SetDbgLevel, (void*)&pArgs);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(DSCMB2_DBGLV_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return FALSE;
    }

    return TRUE;

#else
    return _MDrv_AKL_SetDbgLevel(eDbgLevel);
#endif
}

MS_BOOL MDrv_AKL_SetManualACK(void)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;

    if(pu32AKLInst == NULL)
    {
        return FALSE;
    }

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_SetManualACK, NULL);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return FALSE;
    }

    return TRUE;

#else
    return _MDrv_AKL_SetManualACK();
#endif
}

DRV_AKL_RET MDrv_AKL_SetDMAKey(MS_U32 u32KeyIndex)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;
    AKL_DMA_KEY pArgs;

    if(pu32AKLInst == NULL)
    {
        return DRV_AKL_FAIL;
    }

    pArgs.u32KeyIdx = u32KeyIndex;

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_SetDMAKey, (void*)&pArgs);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return DRV_AKL_FAIL;
    }

    return DRV_AKL_OK;

#else
    return _MDrv_AKL_SetDMAKey(u32KeyIndex);
#endif
}

DRV_AKL_RET MDrv_AKL_SetDMAParserKey(MS_U32 u32OddKeyIndex, MS_U32 u32EvenKeyIndex)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;
    AKL_DMAPARSER_KEY pArgs;

    if(pu32AKLInst == NULL)
    {
        return DRV_AKL_FAIL;
    }

    pArgs.u32OddKeyIndex  = u32OddKeyIndex;
    pArgs.u32EvenKeyIndex = u32EvenKeyIndex;

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_SetDMAParserKey, (void*)&pArgs);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return DRV_AKL_FAIL;
    }

    return DRV_AKL_OK;

#else
    return _MDrv_AKL_SetDMAParserKey(u32OddKeyIndex, u32EvenKeyIndex);
#endif
}

DRV_AKL_RET MDrv_AKL_SetDSCMBKey(MS_U32 u32EngId, MS_U32 u32Index, AKL_Eng_Type eEngType, AKL_Key_Type eKeyType)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;
    AKL_DSCMB_KEY pArgs;

    if(pu32AKLInst == NULL)
    {
        return DRV_AKL_FAIL;
    }

    pArgs.u32EngId  = u32EngId;
    pArgs.u32Index  = u32Index;
    pArgs.eEngType  = eEngType;
    pArgs.eKeyType  = eKeyType;

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_SetDSCMBKey, (void*)&pArgs);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return DRV_AKL_FAIL;
    }

    return DRV_AKL_OK;

#else
    return _MDrv_AKL_SetDSCMBKey(u32EngId, u32Index, eEngType, eKeyType);
#endif
}

DRV_AKL_RET MDrv_AKL_SetTSIOKey(MS_U32 u32ServiceId, AKL_Key_Type eKeyType)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;
    AKL_TSIO_KEY pArgs;

    if(pu32AKLInst == NULL)
    {
        return DRV_AKL_FAIL;
    }

    pArgs.u32ServiceId  = u32ServiceId;
    pArgs.eKeyType      = eKeyType;

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_SetTSIOKey, (void*)&pArgs);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return DRV_AKL_FAIL;
    }

    return DRV_AKL_OK;

#else
    return _MDrv_AKL_SetTSIOKey(u32ServiceId, eKeyType);
#endif
}


MS_BOOL MDrv_AKL_SetManualACKMode(MS_BOOL bEnable)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;
    AKL_ACKMODE pArgs;

    if(pu32AKLInst == NULL)
    {
        return FALSE;
    }

    pArgs.bEnable = bEnable;

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_SetManualACKMode, (void*)&pArgs);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return FALSE;
    }

    return TRUE;

#else
    return _MDrv_AKL_SetManualACKMode(bEnable);
#endif
}

DRV_AKL_RET MDrv_AKL_CERTCmd(AKL_CERT_Cmd eCmd, MS_U8 *pu8Data, MS_U32 u32DataSize)
{
#ifdef AKL_UTOPIA20
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;
    AKL_EXCHANGE pArgs;

    if(pu32AKLInst == NULL)
    {
        return DRV_AKL_FAIL;
    }

    pArgs.eCmd        = eCmd;
    pArgs.pu8Data     = pu8Data;
    pArgs.u32DataSize = u32DataSize;

    u32Ret = UtopiaIoctl(pu32AKLInst, E_MDRV_CMD_AKL_CERTCmd, (void*)&pArgs);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        DEBUG_AKL(E_AKL_DBG_ERR, "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return DRV_AKL_FAIL;
    }

    return DRV_AKL_OK;

#else
    return _MDrv_AKL_CERTCmd(eCmd, pu8Data, u32DataSize);
#endif
}

