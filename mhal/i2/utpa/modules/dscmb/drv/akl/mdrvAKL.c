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
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2010 MStar Semiconductor, Inc.
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
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file   mdrvAKL.c
/// @brief   API Interface
/// @author MStar Semiconductor,Inc.
/// @attention
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#else
#include <stdio.h>
#include <string.h>
#endif

#include "MsTypes.h"

#include "drvAKL_v2.h"
#include "drvAKL_private.h"

#include "utopia.h"
#include "utopia_dapi.h"
#include "MsOS.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
void* pModuleAKL = NULL;

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define AKL_MSG(_f, _a...)                             //printf(_f, ##_a)

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void AKLRegisterToUtopia(void)
{
    AKL_RESOURCE_PRIVATE * pResPri = NULL;
    void* psResource = NULL;

    // 1. deal with module

    // create a AKL module
    // a. size = 8
    // b. Dst: pModuleAKL
    UtopiaModuleCreate(MODULE_AKL, 8, &pModuleAKL);
    // register a akl module
    UtopiaModuleRegister(pModuleAKL);
    //setup module functions
    UtopiaModuleSetupFunctionPtr(pModuleAKL, (FUtopiaOpen)AKLOpen, (FUtopiaClose)AKLClose, (FUtopiaIOctl)AKLIoctl);

    // 2. deal with resource
    // create and allocate resource private data(drv Global shared data)
    UtopiaResourceCreate("AKL", sizeof(AKL_RESOURCE_PRIVATE), &psResource);
    UtopiaResourceRegister(pModuleAKL, psResource, E_AKL_RESOURCE);
    UtopiaModuleAddResourceEnd(pModuleAKL, E_AKL_RESOURCE);

    if(UtopiaResourceObtain(pModuleAKL, E_AKL_RESOURCE, &psResource) != UTOPIA_STATUS_SUCCESS)
    {
         AKL_MSG("UtopiaResourceObtain fail\n");
         return ;
    }
    UtopiaResourceGetPrivate(psResource, (void**)&pResPri);

    if(pResPri->u32Magic != MAGIC_NUM)
    {
        // not regiser yet
        pResPri->u32Magic = MAGIC_NUM;
        pResPri->bInited_Drv = FALSE;
    }

    UtopiaResourceRelease(psResource);
}

MS_U32 AKLOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{
    AKL_MSG("AKL open \n");

    if(ppInstance == 0)
    {
        return UTOPIA_STATUS_PARAMETER_ERROR;
    }

    AKL_INSTANT_PRIVATE*    psAKLInstPri = NULL;

    //create instance
    //1.  Allocate instant private data, prepare argument variable.
    //2.  Dst: ppInstance
    UtopiaInstanceCreate(sizeof(AKL_INSTANT_PRIVATE), ppInstance);

    //get instant private data pointer
    // 1. Dst: psAKLInstPri
    UtopiaInstanceGetPrivate(*ppInstance, (void**)&psAKLInstPri);

    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 AKLIoctl(void* pInstance, MS_U32 u32Cmd, void* pu32Args)
{
    void* pstRes = NULL;
    MS_U32 u32Ret = UTOPIA_STATUS_FAIL;
    AKL_RESOURCE_PRIVATE* pstResPri  = NULL;
    //AKL_INSTANT_PRIVATE*  pstInstPri = NULL;

    if(UtopiaResourceObtain(pModuleAKL, E_AKL_RESOURCE, &pstRes) != UTOPIA_STATUS_SUCCESS)
    {
         AKL_MSG("UtopiaResourceObtain fail\n");
         return UTOPIA_STATUS_ERR_RESOURCE;
    }
    UtopiaResourceGetPrivate(pstRes, (void**)&pstResPri);

    if((FALSE == pstResPri->bInited_Drv)  &&
        (u32Cmd != E_MDRV_CMD_AKL_Init) &&
        (u32Cmd != E_MDRV_CMD_AKL_SetDbgLevel))
    {
        UtopiaResourceRelease(pstRes);
        return UTOPIA_STATUS_FAIL;
    }

    switch(u32Cmd)
    {
        case E_MDRV_CMD_AKL_Init:
        {
            if(pstResPri->bInited_Drv == FALSE)
            {
                if(TRUE == _MDrv_AKL_Init())
                {
                     pstResPri->bInited_Drv = TRUE;
                }
                else
                {
                    AKL_MSG("E_MDRV_CMD_AKL_Init fail\n");
                }
            }

            AKL_MSG("E_MDRV_CMD_AKL_Init\n");

            u32Ret = UTOPIA_STATUS_SUCCESS;
        }
        break;

        case E_MDRV_CMD_AKL_Reset:
        {
            if(TRUE == _MDrv_AKL_Reset())
            {
                u32Ret = UTOPIA_STATUS_SUCCESS;
            }
            AKL_MSG("E_MDRV_CMD_AKL_Reset\n");
        }
        break;

        case E_MDRV_CMD_AKL_SetDbgLevel:
        {
            AKL_DBGLEVEL* pDbgLevel = (AKL_DBGLEVEL* )pu32Args;
            if(TRUE == _MDrv_AKL_SetDbgLevel(pDbgLevel->eDBGMsgLevel))
            {
                u32Ret = UTOPIA_STATUS_SUCCESS;
            }
            AKL_MSG("E_MDRV_CMD_AKL_SetDbgLevel\n");
        }
        break;

        case E_MDRV_CMD_AKL_SetManualACK:
        {
            if(TRUE == _MDrv_AKL_SetManualACK())
            {
                u32Ret = UTOPIA_STATUS_SUCCESS;
            }
            AKL_MSG("E_MDRV_CMD_AKL_SetManualACK\n");
        }
        break;

        case E_MDRV_CMD_AKL_SetDMAKey:
        {
            AKL_DMA_KEY* pDMAKey = (AKL_DMA_KEY* )pu32Args;
            if(DRV_AKL_OK == _MDrv_AKL_SetDMAKey(pDMAKey->u32KeyIndex))
            {
                u32Ret = UTOPIA_STATUS_SUCCESS;
            }
            AKL_MSG("E_MDRV_CMD_AKL_SetDMAKey\n");
        }
        break;

        case E_MDRV_CMD_AKL_SetDMAParserKey:
        {
            AKL_DMAPARSER_KEY* pDMAParserKey = (AKL_DMAPARSER_KEY* )pu32Args;
            if(DRV_AKL_OK == _MDrv_AKL_SetDMAParserKey(pDMAParserKey->u32OddKeyIndex, pDMAParserKey->u32EvenKeyIndex))
            {
                u32Ret = UTOPIA_STATUS_SUCCESS;
            }
            AKL_MSG("E_MDRV_CMD_AKL_SetDMAParserKey\n");
        }
        break;

        case E_MDRV_CMD_AKL_SetDSCMBKey:
        {
            AKL_DSCMB_KEY* pDSCMBKey = (AKL_DSCMB_KEY* )pu32Args;
            if(DRV_AKL_OK == _MDrv_AKL_SetDSCMBKey(pDSCMBKey->u32EngId, pDSCMBKey->u32Index, pDSCMBKey->eEngType, pDSCMBKey->eKeyType))
            {
                u32Ret = UTOPIA_STATUS_SUCCESS;
            }
            AKL_MSG("E_MDRV_CMD_AKL_SetDSCMBKey\n");
        }
        break;

        case E_MDRV_CMD_AKL_SetTSIOKey:
        {
            AKL_TSIO_KEY* pTSIOKey = (AKL_TSIO_KEY* )pu32Args;
            if(DRV_AKL_OK == _MDrv_AKL_SetTSIOKey(pTSIOKey->u32ServiceId, pTSIOKey->eKeyType))
            {
                u32Ret = UTOPIA_STATUS_SUCCESS;
            }
            AKL_MSG("E_MDRV_CMD_AKL_SetTSIOKey\n");
        }
        break;

        case E_MDRV_CMD_AKL_SetManualACKMode:
        {
            AKL_ACKMODE* pAckMode = (AKL_ACKMODE* )pu32Args;
            if(TRUE == _MDrv_AKL_SetManualACKMode(pAckMode->bEnable))
            {
                u32Ret = UTOPIA_STATUS_SUCCESS;
            }
            AKL_MSG("E_MDRV_CMD_AKL_SetManualACKMode\n");
        }
        break;

        case E_MDRV_CMD_AKL_CERTCmd:
        {
            AKL_EXCHANGE* pExchange = (AKL_EXCHANGE* )pu32Args;
            if(TRUE == _MDrv_AKL_CERTCmd(pExchange->eCmd, pExchange->pu8Data, pExchange->u32DataSize))
            {
                u32Ret = UTOPIA_STATUS_SUCCESS;
            }
            AKL_MSG("_MDrv_AKL_Exchange\n");
        }
        break;


        default:
            AKL_MSG("AKLIoctl - Unknown commend 0x%x!!!\n", (int)u32Cmd);
            return UTOPIA_STATUS_FAIL;
    }


    UtopiaResourceRelease(pstRes);

    return u32Ret;
}

MS_U32 AKLClose(void* pInstance)
{

    UtopiaInstanceDelete(pInstance);

    return UTOPIA_STATUS_SUCCESS;
}

