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
// Copyright (c) 2011-2013 MStar Semiconductor, Inc.
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
/// file    drvCILINK.c
/// @brief  CILINK Driver Interface
/// @author MStar Semiconductor,Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

// Common Definition
#include "MsCommon.h"
#include "MsVersion.h"
#include "drvMMIO.h"

// Internal Definition
#include "asmCPU.h"
#include "regCILINK.h"
#include "halCILINK.h"
#include "drvCILINK.h"

#include "ULog.h"

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

typedef struct _CILINK_RESOURCE_PRIVATE
{
    MS_U32                       _bUsed[REG_CILINK_ENG_NUM];
    MS_BOOL                      _bSWInit;
}CILINK_RESOURCE_PRIVATE;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
#ifndef TSO_UTOPIA_20
static CILINK_RESOURCE_PRIVATE _Cilink_res = {
    ._bUsed                         = {[0 ... (REG_CILINK_ENG_NUM-1)] = FALSE},
    ._bSWInit                       = FALSE,
};

static CILINK_RESOURCE_PRIVATE*    _pCilink_res = &_Cilink_res;
#else
static CILINK_RESOURCE_PRIVATE*    _pCilink_res = NULL;
#endif

static MS_BOOL                      _bBankInit = FALSE;
//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

DrvCILINK_Result MDrv_CILINK_GetCap(DrvCILINK_Cap eCap, void* pOutput)
{
    *((MS_U32*)pOutput) = 0UL;

    if (eCap >= E_DRVCILINK_CAP_NULL)
        return DRVCILINK_FAIL;

    if(eCap == E_DRVCILINK_CAP_RESOURCE_SIZE)
    {
        *((MS_U32*)pOutput) = sizeof(CILINK_RESOURCE_PRIVATE);
        return DRVCILINK_OK;
    }
    else
    {
        return DRVCILINK_FAIL;
    }
}


//-------------------------------------------------------------------------------------------------
/// Initialize lib resource API
/// @param pResMemAddr                \b IN: Pointer to store resource memory address
/// @return MMFI_Result
/// @note
//-------------------------------------------------------------------------------------------------
DrvCILINK_Result MDrv_CILINK_InitLibResource(void* pResMemAddr)
{
    MS_VIRT virtBank = 0;
    MS_PHY u32BankSize = 0;
    MS_U32 i;

    if(pResMemAddr == 0)
        return DRVCILINK_FAIL;

    _pCilink_res = (CILINK_RESOURCE_PRIVATE*)pResMemAddr;

    if(_pCilink_res->_bSWInit != TRUE)
    {
        for(i = 0; i < REG_CILINK_ENG_NUM; i++)
        {
            _pCilink_res->_bUsed[i] = 0;
        }

        _pCilink_res->_bSWInit = FALSE;

    }

    if(_bBankInit == FALSE)
    {
        if (FALSE == MDrv_MMIO_GetBASE(&virtBank, &u32BankSize, MS_MODULE_EXTRIU))
        {
            return DRVCILINK_FAIL;
        }
        HAL_CILINK_SetExtBank(virtBank);
        if (FALSE == MDrv_MMIO_GetBASE(&virtBank, &u32BankSize, MS_MODULE_CHIPTOP))
        {
            return DRVCILINK_FAIL;
        }
        HAL_CILINK_SetBank(virtBank);
        _bBankInit = TRUE;
    }

    return DRVCILINK_OK;
}

//-------------------------------------------------------------------------------------------------
/// Initialize CILINK driver
/// @return CILINK_Result
/// @note
/// MDrv_CILINK_Open should be called before any other CILINK control functions
//-------------------------------------------------------------------------------------------------
DrvCILINK_Result MDrv_CILINK_Open(DrvCILINK_ENG eEng)
{
    MS_VIRT virtBank;
    MS_PHY u32BankSize;

    if (FALSE == MDrv_MMIO_GetBASE(&virtBank, &u32BankSize, MS_MODULE_EXTRIU))
    {
        return DRVCILINK_FAIL;
    }
    HAL_CILINK_SetExtBank(virtBank);
    if (FALSE == MDrv_MMIO_GetBASE(&virtBank, &u32BankSize, MS_MODULE_CHIPTOP))
    {
        return DRVCILINK_FAIL;
    }
    HAL_CILINK_SetBank(virtBank);

    if(!HAL_CILINK_IsEnable())
        HAL_CILINK_PowerCtrl(TRUE);

    HAL_CILINK_PadEnable(eEng);
    
    _pCilink_res->_bUsed[eEng] = TRUE;
    
    _pCilink_res->_bSWInit = TRUE;

    return DRVCILINK_OK;
}

//-------------------------------------------------------------------------------------------------
/// Initialize CILINK driver
/// @return CILINK_Result
/// @note
/// It should be called before calling any other TSO DDI functions.
/// Driver task is in @ref E_TASK_PRI_SYS level.
//-------------------------------------------------------------------------------------------------
DrvCILINK_Result MDrv_CILINK_Close(DrvCILINK_ENG eEng)
{
    MS_U32 i;
    MS_BOOL bPowerFlag = FALSE;
    
    if(!HAL_CILINK_IsEnable())
        return DRVCILINK_OK;

    HAL_CILINK_PadDisable(eEng);

    _pCilink_res->_bUsed[eEng] = FALSE;

    for(i = 0; i < REG_CILINK_ENG_NUM; i++)
    {
        if(_pCilink_res->_bUsed[i] == TRUE)
        {
            bPowerFlag = TRUE;
            break;
        }
    }
    
    if(!bPowerFlag)
        HAL_CILINK_PowerCtrl(FALSE);
    
    return DRVCILINK_OK;
}



//-------------------------------------------------------------------------------------------------
/// CILINK driver
/// @return CILINK_Result
/// @note
/// It should be called before calling any other TSO DDI functions.
/// Driver task is in @ref E_TASK_PRI_SYS level.
//-------------------------------------------------------------------------------------------------
DrvCILINK_Result MDrv_CILINK_CPN_ToCard_CLK(MS_U8 u8Eng,MS_BOOL bSet, MS_BOOL *bClkInv, MS_BOOL *bSampleEnable, MS_U8 *u8SampleCnt)
{
    if(bSet)
    {
        HAL_COMPANION_ToCard_CLK_Set(u8Eng, *bClkInv, *bSampleEnable, *u8SampleCnt);
    }
    else
    {
        HAL_COMPANION_ToCard_CLK_Get(u8Eng, bClkInv, bSampleEnable, u8SampleCnt);
    }

    return DRVCILINK_OK;
}

//-------------------------------------------------------------------------------------------------
/// CILINK driver
/// @return CILINK_Result
/// @note
/// It should be called before calling any other TSO DDI functions.
/// Driver task is in @ref E_TASK_PRI_SYS level.
//-------------------------------------------------------------------------------------------------
DrvCILINK_Result MDrv_CILINK_CPN_FromCard_CLK(MS_U8 u8Eng,MS_BOOL bSet, MS_BOOL *bClkInv, MS_BOOL *bSampleEnable, MS_U8 *u8SampleCnt)
{
    if(bSet)
    {
        HAL_COMPANION_FromCard_CLK_Set(u8Eng, *bClkInv, *bSampleEnable, *u8SampleCnt);
    }
    else
    {
        HAL_COMPANION_FromCard_CLK_Get(u8Eng, bClkInv, bSampleEnable, u8SampleCnt);
    }

    return DRVCILINK_OK;
}




