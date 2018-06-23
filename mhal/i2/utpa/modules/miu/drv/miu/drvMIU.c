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
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    drvMIU.c
/// @brief  MIU Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------------------------------
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#include <linux/slab.h>
#else
#include <string.h>
#endif
#include "MsCommon.h"
#include "MsVersion.h"
#include "MsTypes.h"
#include "drvMIU.h"
#include "drvMIU_private.h"
#include "halMIU.h"
#include "drvMMIO.h"
#include "MsOS.h"
#include "utopia.h"
#include "utopia_dapi.h"
//#include <assert.h>
#include "ULog.h"
//-------------------------------------------------------------------------------------------------
// Local Defines
//-------------------------------------------------------------------------------------------------
#define TAG_MIU "MIU"


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------
static MSIF_Version _drv_miu_version = {
    .DDI = { MIU_DRV_VERSION },
};

MS_U8 MIU0_request_mask_count[MIU_CLIENT_MAX];
MS_U8 MIU1_request_mask_count[MIU_CLIENT_MAX];

void* pInstantMIU = NULL;
void* pAttributeMIU = NULL;
static MS_BOOL bMiuInit = FALSE;

//-------------------------------------------------------------------------------------------------
// Local Function Prototypes
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_MIU_Exit(void);

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _MDrv_MIU_Init
/// @brief \b Function  \b Description: set MIU I/O Map base address and parse occupied resource to software structure
/// @param None         \b IN :
/// @param None         \b OUT :
/// @param MS_BOOL      \b RET
/// @param None         \b GLOBAL :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_Init(void)
{
    MS_VIRT virtMMIOBaseAdr = 0;
    MS_PHY u32NonPMBankSize = 0;

    if( !MDrv_MMIO_GetBASE(&virtMMIOBaseAdr, &u32NonPMBankSize, MS_MODULE_MIU))
    {
        MS_DEBUG_MSG(ULOGE(TAG_MIU, "Get IOMap failure\n"));
        MS_ASSERT(0);
        return FALSE;
    }

    HAL_MIU_SetIOMapBase(virtMMIOBaseAdr);

    if( !MDrv_MMIO_GetBASE(&virtMMIOBaseAdr, &u32NonPMBankSize, MS_MODULE_PM))
    {
        MS_DEBUG_MSG(ULOGE(TAG_MIU, "Get IOMap failure\n"));
        MS_ASSERT(0);
        return FALSE;
    }
    HAL_MIU_SetPMIOMapBase(virtMMIOBaseAdr);
    HAL_MIU_ParseOccupiedResource();
    bMiuInit = TRUE;
    if( !MDrv_MMIO_GetBASE(&virtMMIOBaseAdr, &u32NonPMBankSize, MS_MODULE_EXTRIU))
    {
        MS_DEBUG_MSG(ULOGE(TAG_MIU, "Get IOMap failure\n"));
        return TRUE;
    }
    HAL_MIU_SetExtIOMapBase(virtMMIOBaseAdr);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_SetIOMapBase
/// @brief \b Function  \b Description: set MIU I/O Map base address
/// @param None         \b IN :
/// @param None         \b OUT :
/// @param MS_BOOL      \b RET
/// @param None         \b GLOBAL :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_SetIOMapBase(void)
{
    MS_VIRT virtMMIOBaseAdr = 0;
    MS_PHY u32NonPMBankSize = 0;

    if( !MDrv_MMIO_GetBASE(&virtMMIOBaseAdr, &u32NonPMBankSize, MS_MODULE_MIU))
    {
        MS_DEBUG_MSG(ULOGE(TAG_MIU, "Get IOMap failure\n"));
        MS_ASSERT(0);
        return FALSE;
    }
    HAL_MIU_SetIOMapBase(virtMMIOBaseAdr);


    if( !MDrv_MMIO_GetBASE(&virtMMIOBaseAdr, &u32NonPMBankSize, MS_MODULE_PM))
    {
        MS_DEBUG_MSG(ULOGE(TAG_MIU, "Get IOMap failure\n"));
        MS_ASSERT(0);
        return FALSE;
    }
    HAL_MIU_SetPMIOMapBase(virtMMIOBaseAdr);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIUGetDefaultClientID_KernelProtect()
/// @brief \b Function \b Description:  Get default client id array pointer for protect kernel
/// @param <RET>           \b     : The pointer of Array of client IDs
////////////////////////////////////////////////////////////////////////////////
MS_U8* MDrv_MIUGetDefaultClientID_KernelProtect(void)
{
    return HAL_MIU_GetDefaultClientID_KernelProtect();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function    \b Name: MDrv_MIU_ProtectAlign()
/// @brief \b Function    \b Description:  Get the page shift for MIU protect
/// @param <*u32PageShift>\b IN: Page shift
/// @param <RET>          \b OUT: None
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_ProtectAlign(MS_U32 *u32PageShift)
{
    *u32PageShift = HAL_MIU_ProtectAlign();

    if(*u32PageShift > 0)
        return TRUE;
    else
        return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_Dram_Size()
/// @brief \b Function \b Description:  Write the DRAM size reg for MIU protect
/// @param MiuID        \b IN     : MIU ID
/// @param DramSize     \b IN     : Specified Dram size for MIU protect
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_Dram_Size(MS_U8 MiuID, MS_U8 DramSize)
{
    return HAL_MIU_Dram_Size(MiuID, DramSize);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_Dram_ReadSize()
/// @brief \b Function \b Description:  Read the DRAM size
/// @param MiuID        \b IN     : MIU ID
/// @param DramSize     \b IN     :  Dram size for MIU protect
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_Dram_ReadSize(MS_U8 MiuID, MIU_DDR_SIZE *pDramSize)
{
    return HAL_MIU_Dram_ReadSize(MiuID, pDramSize);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_GetClinetNumber()
/// @brief \b Function \b Description:  Get the number of clients for specific MIU block
/// @param DramSize     \b IN     : MIU Block to protect (0 ~ 3)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MIU_ClinetNumber(MS_U8 u8Blockx)
{
    return HAL_MIU_ClinetNumber(u8Blockx);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Protect()
/// @brief \b Function \b Description:  Enable/Disable MIU Protection mode
/// @param u8Blockx        \b IN     : MIU Block to protect (0 ~ 3)
/// @param *pu8ProtectId   \b IN     : Allow specified client IDs to write
/// @param phy64Start      \b IN     : Starting address
/// @param phy64End        \b IN     : End address
/// @param bSetFlag        \b IN     : Disable or Enable MIU protection
///                                      - -Disable(0)
///                                      - -Enable(1)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_Protect(
                          MS_U8    u8Blockx,
                          MS_U8    *pu8ProtectId,
                          MS_PHY phy64Start,
                          MS_PHY phy64End,
                          MS_BOOL  bSetFlag
                         )
{
    MS_BOOL Result;

    Result = HAL_MIU_Protect(u8Blockx, pu8ProtectId, phy64Start, phy64End, bSetFlag);

    if(Result == TRUE)
    {
        return TRUE;
    }
    else
    {
       MS_ASSERT(0);
       return FALSE;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_ProtectEx()
/// @brief \b Function \b Description:  Enable/Disable MIU Protection mode
/// @param u8Blockx        \b IN     : MIU Block to protect (0 ~ 3)
/// @param *pu8ProtectId   \b IN     : Allow specified client IDs to write
/// @param phy64Start      \b IN     : Starting address
/// @param phy64End        \b IN     : End address
/// @param bSetFlag        \b IN     : Disable or Enable MIU protection
///                                      - -Disable(0)
///                                      - -Enable(1)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_ProtectEx(
                          MS_U8    u8Blockx,
                          MS_U32    *pu32ProtectId,
                          MS_PHY phy64Start,
                          MS_PHY phy64End,
                          MS_BOOL  bSetFlag
                         )
{
    MS_BOOL Result;

    Result = HAL_MIU_ProtectEx(u8Blockx, pu32ProtectId, phy64Start, phy64End, bSetFlag);

    if(Result == TRUE)
    {
        return TRUE;
    }
    else
    {
       MS_ASSERT(0);
       return FALSE;
    }
}



////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_SetSsc()
/// @brief \b Function \b Description: _MDrv_MIU_SetSsc, @Step & Span
/// @param u16Fmodulation   \b IN : 20KHz ~ 40KHz (Input Value = 20 ~ 40)
/// @param u16FDeviation    \b IN  : under 0.1% ~ 3% (Input Value = 1 ~ 30)
/// @param bEnable          \b IN    :
/// @param None             \b OUT  :
/// @param None             \b RET  :
/// @param None             \b GLOBAL :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_SetSsc(MS_U16 u16Fmodulation,
                        MS_U16 u16FDeviation,
                        MS_BOOL bEnable)
{
    return HAL_MIU_SetSsc(0, u16Fmodulation, u16FDeviation, bEnable);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_SetSscValue()
/// @brief \b Function \b Description: _MDrv_MIU_SetSscValue, @Step & Span
/// @param u8Miu            \b IN   : 0: MIU0 1:MIU1
/// @param u16Fmodulation   \b IN   : 20KHz ~ 40KHz (Input Value = 20 ~ 40)
/// @param u16FDeviation    \b IN   : under 0.1% ~ 3% (Input Value = 1 ~ 30)
/// @param bEnable          \b IN   :
/// @param None             \b OUT  :
/// @param None             \b RET  :
/// @param None             \b GLOBAL :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_SetSscValue(MS_U8 u8MiuDev,
                             MS_U16 u16Fmodulation,
                             MS_U16 u16FDeviation,
                             MS_BOOL bEnable)
{
    return HAL_MIU_SetSsc(u8MiuDev, u16Fmodulation, u16FDeviation, bEnable);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_MaskReq()
/// @brief \b Function \b Description: Mask MIU request
/// @param u8Miu        \b IN   : 0: MIU0 1:MIU1
/// @param eClientID    \b IN   : Client ID
/// @param None         \b OUT  :
/// @param None         \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_MaskReq(MS_U8 u8Miu, eMIUClientID eClientID)
{
    HAL_MIU_MaskReq(u8Miu, eClientID);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_UnMaskReq()
/// @brief \b Function \b Description: Unmak MIU request
/// @param u8Miu        \b IN   : 0: MIU0 1:MIU1
/// @param eClientID    \b IN   : Client ID
/// @param None         \b OUT  :
/// @param None         \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_UnMaskReq(MS_U8 u8Miu, eMIUClientID eClientID)
{
    HAL_MIU_UnMaskReq(u8Miu, eClientID);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_OPM_R()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_OPM_R(MS_U8 u8Mask, MS_U8 u8Miu)
{
    HAL_MIU_Mask_Req_OPM_R(u8Mask, u8Miu);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_DNRB_R()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_DNRB_R(MS_U8 u8Mask, MS_U8 u8Miu)
{
    HAL_MIU_Mask_Req_DNRB_R( u8Mask,  u8Miu);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_DNRB_W()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_DNRB_W(MS_U8 u8Mask, MS_U8 u8Miu)
{
    HAL_MIU_Mask_Req_DNRB_W( u8Mask,  u8Miu);

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_DNRB_RW()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_DNRB_RW(MS_U8 u8Mask, MS_U8 u8Miu)
{
   HAL_MIU_Mask_Req_DNRB_RW( u8Mask,  u8Miu);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_SC_RW()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_SC_RW(MS_U8 u8Mask, MS_U8 u8Miu)
{
    HAL_MIU_Mask_Req_SC_RW( u8Mask,  u8Miu);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_MVOP_R()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_MVOP_R(MS_U8 u8Mask, MS_U8 u8Miu)
{
    HAL_MIU_Mask_Req_MVOP_R( u8Mask,  u8Miu);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_MVD_R()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_MVD_R(MS_U8 u8Mask, MS_U8 u8Miu)
{
    HAL_MIU_Mask_Req_MVD_R( u8Mask,  u8Miu);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_MVD_W()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_MVD_W(MS_U8 u8Mask, MS_U8 u8Miu)
{
    HAL_MIU_Mask_Req_MVD_W( u8Mask,  u8Miu);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_MVD_RW()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_MVD_RW(MS_U8 u8Mask, MS_U8 u8Miu)
{
    HAL_MIU_Mask_Req_MVD_RW( u8Mask,  u8Miu);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_Mask_Req_AUDIO_RW()
/// @brief \b Function \b Description:
/// @param None  \b IN   :
/// @param None  \b OUT  :
/// @param None  \b RET  :
///////////////////////////////////////////////////////////////////////////////
void _MDrv_MIU_Mask_Req_AUDIO_RW(MS_U8 u8Mask, MS_U8 u8Miu)
{
    HAL_MIU_Mask_Req_AUDIO_RW( u8Mask,  u8Miu);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_InitCounter()
/// @brief \b Function \b Description:  This function clear all request mask counter of
///                                     MIU driver
/// @param None \b IN:
/// @param None \b RET:
////////////////////////////////////////////////////////////////////////////////
void MDrv_MIU_InitCounter(void)
{
    MS_U8 i;

    for(i=0;i<MIU_CLIENT_MAX;i++)
    {
        MIU0_request_mask_count[i]=0;
        MIU1_request_mask_count[i]=0;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_SwitchMIU()
/// @brief \b Function \b Description:  This function for switch miu
///
/// @param u8MiuID        \b IN     : select MIU0 or MIU1
/// @param None \b RET:
////////////////////////////////////////////////////////////////////////////////
void MDrv_MIU_SwitchMIU(MS_U8 u8MiuID)
{
      HAL_MIU_VOP_SwitchMIU(u8MiuID);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_SwitchMIU()
/// @brief \b Function \b Description:  This function for switch miu
///
/// @param u8MiuID        \b IN     : select MIU0 or MIU1
/// @param None \b RET:
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_SelMIU(eMIUClientID eClientID, eMIU_SelType eType)
{
    MS_BOOL ret = TRUE;

    _MDrv_MIU_MaskReq(0, eClientID);
    _MDrv_MIU_MaskReq(1, eClientID);
    ret &= HAL_MIU_SelMIU(eType, eClientID);
    if (MIU_SELTYPE_MIU_ALL == eType)
    {
        _MDrv_MIU_UnMaskReq(0, eClientID);
        _MDrv_MIU_UnMaskReq(1, eClientID);
    }
    else
        _MDrv_MIU_UnMaskReq((MS_U8)eType, eClientID);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_IsSupportMIU1()
/// @brief \b Function \b Description:  This function for checking if support miu1
///
/// @param None \b RET:   TRUE: Support miu1 FALSE: no miu1
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_IsSupportMIU1(void)
{
      return HAL_MIU_IsInitMiu1();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_GetBusWidth()
/// @brief \b Function \b Description:  This function for querying data bus width
///
/// @param None \b RET:   0: not support or 64 or 128 bits
////////////////////////////////////////////////////////////////////////////////
MS_U16 MDrv_MIU_GetBusWidth(void)
{
    return HAL_MIU_IsI64Mode();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_GetClientInfo()
/// @brief \b Function  \b Description:  This function for querying client ID info
/// @param u8MiuDev     \b IN   : select MIU0 or MIU1
/// @param eClientID    \b IN   : Client ID
/// @param pInfo        \b OUT  : Client Info
/// @param None \b RET:   0: Fail 1: Ok
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_GetClientInfo(MS_U8 u8MiuDev, eMIUClientID eClientID, MIU_ClientInfo *pInfo)
{
    MS_S16 sVal;
    if (!pInfo)
        return FALSE;

    sVal = HAL_MIU_GetClientInfo(u8MiuDev, eClientID);
    if (sVal < 0)
        return FALSE;
    pInfo->u8BitPos = (MS_U8)MIU_GET_CLIENT_POS(sVal);
    pInfo->u8Gp = (MS_U8)MIU_GET_CLIENT_GROUP(sVal);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _MDrv_MIU_GetProtectInfo()
/// @brief \b Function  \b Description:  This function for querying client ID info
/// @param u8MiuDev     \b IN   : select MIU0 or MIU1
/// @param eClientID    \b IN   : Client ID
/// @param pInfo        \b OUT  : Client Info
/// @param None \b RET:   0: Fail 1: Ok
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_GetProtectInfo(MS_U8 u8MiuDev, MIU_PortectInfo *pInfo)
{
    MS_U32 ret;
    ret = HAL_MIU_GetProtectInfo(u8MiuDev, pInfo);

    //if(pInfo->bHit)
    //{
    //    if(MsOS_GetKattribute("miuprot")==1)
    //        MS_ASSERT(0);
    //}

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _MDrv_MIU_SetGroupPriority()
/// @brief \b Function  \b Description:  This function for querying client ID info
/// @param u8MiuDev     \b IN   : select MIU0 or MIU1
/// @param eClientID    \b IN   : Client ID
/// @param pInfo        \b OUT  : Client Info
/// @param None \b RET:   0: Fail 1: Ok
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_SetGroupPriority(MS_U8 u8MiuDev, MIU_GroupPriority sPriority)
{
    return HAL_MIU_SetGroupPriority(u8MiuDev, sPriority);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _MDrv_MIU_SetHPriorityMask()
/// @brief \b Function  \b Description:  Set miu high priority mask
/// @param u8MiuDev     \b IN   : select MIU0 or MIU1
/// @param eClientID    \b IN   : Client ID
/// @param pInfo        \b OUT  : Client Info
/// @param None \b RET:   0: Fail 1: Ok
////////////////////////////////////////////////////////////////////////////////
MS_BOOL _MDrv_MIU_SetHPriorityMask(MS_U8 u8MiuDev, eMIUClientID eClientID, MS_BOOL bMask)
{
    return HAL_MIU_SetHPriorityMask(u8MiuDev, eClientID, bMask);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_GetLibVer
/// @brief \b Function  \b Description: Show the MIU driver version
/// @param ppVersion    \b Out: Library version string
/// @return             \b 0: Fail 1: Ok
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_GetLibVer(const MSIF_Version **ppVersion)
{
    if (!ppVersion)
        return FALSE;

    *ppVersion = &_drv_miu_version;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_EnableScramble
/// @brief \b Function  \b Description: Enable MIU Scramble
/// @param bEnable      \b In: Enable MIU Scramble
/// @return             \b 0: Fail 1: Ok
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_EnableScramble(MS_BOOL bEnable)
{
    return HAL_MIU_EnableScramble(bEnable);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_IsScrambleEnabled
/// @brief \b Function  \b Description: Check if the MIU Scramble enabled
/// @return             \b 0: Disabled 1: Enabled
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_IsScrambleEnabled(void)
{
    return HAL_MIU_IsScrambleEnabled();
}

#ifdef _MIU_INTERNEL_USE
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_GetLoading
/// @brief \b Function  \b Description: Get the percentage of MIU loading
/// @param u8MiuDev     \b IN   : select MIU0 or MIU1
/// @param *Loading     \b IN   : percentage of MIU loading
/// @return             \b 0: Fail 1: OK
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_GetLoading(MS_U8 u8MiuDev, MS_U32 *u32Loading)
{
    HAL_MIU_SetLoadingRequest(u8MiuDev);

    MsOS_DelayTask(1000);

    HAL_MIU_GetLoadingRequest(u8MiuDev, u32Loading);

    return TRUE;
}
#endif


void _MDrv_MIU_PrintProtectInfo(void)
{
    HAL_MIU_PrintMIUProtectInfo();
}

#ifndef UTOPIAXP_REMOVE_WRAPPER
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_Exit
/// @brief \b Function  \b Description: Exit MIU driver
/// @return             \b 0: Fail 1: OK
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_Exit(void)
{
    if (bMiuInit == FALSE)
    {
        ULOGE(TAG_MIU, "MIU is not be initialized\n");
        return TRUE;
    }

    if(UtopiaClose(pInstantMIU) != UTOPIA_STATUS_SUCCESS)
    {
        ULOGE(TAG_MIU, "close MIU fail\n");
        pInstantMIU = NULL;
        return FALSE;
    }

    pInstantMIU = NULL;
    bMiuInit = FALSE;

    return TRUE;
}

void _MDrv_MIU_MaskReqUtopia20IOCtrl(eMIUIoctlCmd eMIUIOCmd, MS_U8 u8Mask, MS_U8 u8Miu)
{
    PMIU_MIU_MASK pMIUMask;
    if (NULL != pInstantMIU)
    {
        pMIUMask = (PMIU_MIU_MASK)malloc(sizeof(MIU_MIU_MASK));
        if (pMIUMask == NULL)
            return;
        pMIUMask->u8Mask = u8Mask;
        pMIUMask->u8Miu = u8Miu;
        if(UtopiaIoctl(pInstantMIU,eMIUIOCmd,(void*)pMIUMask) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_Mask_Req fail in IP %d\n",eMIUIOCmd);
            free(pMIUMask);
    		return;
        }
        free(pMIUMask);
    }
}

void MDrv_MIU_Mask_Req_OPM_R(MS_U8 u8Mask, MS_U8 u8Miu)
{
    // utopia20 flow code
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_OPM_R, u8Mask, u8Miu);
}

void MDrv_MIU_Mask_Req_DNRB_R(MS_U8 u8Mask, MS_U8 u8Miu)
{
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_DNRB_R, u8Mask, u8Miu);
}

void MDrv_MIU_Mask_Req_DNRB_W(MS_U8 u8Mask, MS_U8 u8Miu)
{
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_DNRB_W, u8Mask, u8Miu);
}

void MDrv_MIU_Mask_Req_DNRB_RW(MS_U8 u8Mask, MS_U8 u8Miu)
{
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_DNRB_RW, u8Mask, u8Miu);
}

void MDrv_MIU_Mask_Req_SC_RW(MS_U8 u8Mask, MS_U8 u8Miu)
{
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_SC_RW, u8Mask, u8Miu);
}

void MDrv_MIU_Mask_Req_MVOP_R(MS_U8 u8Mask, MS_U8 u8Miu)
{
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_MVOP_R, u8Mask, u8Miu);
}

void MDrv_MIU_Mask_Req_MVD_R(MS_U8 u8Mask, MS_U8 u8Miu)
{
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_MVD_R, u8Mask, u8Miu);
}

void MDrv_MIU_Mask_Req_MVD_W(MS_U8 u8Mask, MS_U8 u8Miu)
{
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_MVD_W, u8Mask, u8Miu);
}

void MDrv_MIU_Mask_Req_MVD_RW(MS_U8 u8Mask, MS_U8 u8Miu)
{
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_MVD_RW, u8Mask, u8Miu);
}

void MDrv_MIU_Mask_Req_AUDIO_RW(MS_U8 u8Mask, MS_U8 u8Miu)
{
    _MDrv_MIU_MaskReqUtopia20IOCtrl(MDrv_CMD_MIU_Mask_Req_AUDIO_RW, u8Mask, u8Miu);
}

void  MDrv_MIU_MaskReq(MS_U8 u8Miu, eMIUClientID eClientID)
{
    PMIU_MIUREQ_MASK pMIUMaskReq;

    if (NULL != pInstantMIU)
    {
        pMIUMaskReq = (PMIU_MIUREQ_MASK)malloc(sizeof(MIU_MIUREQ_MASK));
        if (pMIUMaskReq == NULL)
            return;
        pMIUMaskReq->u8Miu = u8Miu;
        pMIUMaskReq->eClientID = eClientID;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_Mask_Req,(void*)pMIUMaskReq) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_MIU_MaskReq fail\n");
            free(pMIUMaskReq);
    		return;
        }
        free(pMIUMaskReq);
    }
}

void MDrv_MIU_UnMaskReq(MS_U8 u8Miu, eMIUClientID eClientID)
{
    PMIU_MIUREQ_MASK pMIUMaskReq;

    if (NULL != pInstantMIU)
    {
        pMIUMaskReq = (PMIU_MIUREQ_MASK)malloc(sizeof(MIU_MIUREQ_MASK));
        if (pMIUMaskReq == NULL)
            return;
        pMIUMaskReq->u8Miu = u8Miu;
        pMIUMaskReq->eClientID = eClientID;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_UnMask_Req,(void*)pMIUMaskReq) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_UnMask_Req fail\n");
            free(pMIUMaskReq);
    		return;
        }
        free(pMIUMaskReq);
    }
}

MS_BOOL MDrv_MIU_SetSsc(MS_U16 u16Fmodulation, MS_U16 u16FDeviation,MS_BOOL bEnable)
{
    PMIU_SET_SSC pMIUSetSsc;
    MS_BOOL bRet = TRUE;

    if (NULL != pInstantMIU)
    {
        pMIUSetSsc = (PMIU_SET_SSC)malloc(sizeof(MIU_SET_SSC));
        if (pMIUSetSsc == NULL)
            return FALSE;
        pMIUSetSsc->u16Fmodulation = u16Fmodulation;
        pMIUSetSsc->u16FDeviation = u16FDeviation;
        pMIUSetSsc->bEnable = bEnable;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_SetSsc,(void*)pMIUSetSsc) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_SetSsc fail\n");
            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;
        }
        free(pMIUSetSsc);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

MS_BOOL MDrv_MIU_SetSscValue(MS_U8 u8MiuDev, MS_U16 u16Fmodulation, MS_U16 u16FDeviation, MS_BOOL bEnable)
{
    PMIU_SET_SSCVALUE pMIUSetSscValue;
    MS_BOOL bRet = TRUE;

    if (NULL != pInstantMIU)
    {
        pMIUSetSscValue = (PMIU_SET_SSCVALUE)malloc(sizeof(MIU_SET_SSCVALUE));
        if (pMIUSetSscValue == NULL)
            return FALSE;
        pMIUSetSscValue->u8MiuDev = u8MiuDev;
        pMIUSetSscValue->u16Fmodulation = u16Fmodulation;
        pMIUSetSscValue->u16FDeviation = u16FDeviation;
        pMIUSetSscValue->bEnable = bEnable;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_SetSscValue,(void*)pMIUSetSscValue) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_SetSscValue fail\n");
            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;
        }
        free(pMIUSetSscValue);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

MS_BOOL MDrv_MIUProtect( MS_U8 u8Blockx, MS_U8 *pu8ProtectId, MS_PHY phy64Start, MS_PHY phy64End, MS_BOOL bSetFlag )
{
    PMIU_PROTECT pMIUProtect;
    MS_BOOL bRet = TRUE;

    if (NULL != pInstantMIU)
    {
        pMIUProtect = (PMIU_PROTECT)malloc(sizeof(MIU_PROTECT));
        if (pMIUProtect == NULL)
            return FALSE;
        pMIUProtect->u8Blockx = u8Blockx;
        pMIUProtect->pu8ProtectId = pu8ProtectId;
        pMIUProtect->phy64Start = phy64Start;
        pMIUProtect->phy64End = phy64End;
        pMIUProtect->bSetFlag = bSetFlag;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_Protect,(void*)pMIUProtect) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_Protect fail\n");
            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;
        }
        free(pMIUProtect);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

MS_BOOL MDrv_MIU_ProtectEx( MS_U8 u8Blockx, MS_U32 *pu32ProtectId, MS_PHY phy64Start, MS_PHY phy64End, MS_BOOL bSetFlag )
{
    PMIU_PROTECTEx pMIUProtect;

    if (NULL != pInstantMIU)
    {
        pMIUProtect = (PMIU_PROTECTEx)malloc(sizeof(MIU_PROTECTEx));
        if (pMIUProtect == NULL)
            return FALSE;
        pMIUProtect->u8Blockx = u8Blockx;
        pMIUProtect->pu32ProtectId = pu32ProtectId;
        pMIUProtect->phy64Start = phy64Start;
        pMIUProtect->phy64End = phy64End;
        pMIUProtect->bSetFlag = bSetFlag;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_ProtectEx,(void*)pMIUProtect) != UTOPIA_STATUS_SUCCESS)
        {
            printf("Ioctl MDrv_CMD_MIU_Protect fail\n");
            free(pMIUProtect);
            return FALSE;
        }
        free(pMIUProtect);
    }
    return TRUE;
}

MS_BOOL MDrv_MIU_SelMIU(eMIUClientID eClientID, eMIU_SelType eType)
{
    PMIU_SELMIU pMIUSelMIU;
    MS_BOOL bRet = TRUE;
    if (NULL != pInstantMIU)
    {
        pMIUSelMIU = (PMIU_SELMIU)malloc(sizeof(MIU_SELMIU));
        if (pMIUSelMIU == NULL)
            return FALSE;
        pMIUSelMIU->eClientID = eClientID;
        pMIUSelMIU->eType = eType;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_SelMIU,(void*)pMIUSelMIU) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_SelMIU fail\n");
            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;
        }

        free(pMIUSelMIU);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

MS_BOOL MDrv_MIU_GetProtectInfo(MS_U8 u8MiuDev, MIU_PortectInfo *pInfo)
{
    PMIU_GETPROTECTINFO pMIUGetProtectInfo;
    MS_BOOL bRet = TRUE;

    if (NULL != pInstantMIU)
    {
        pMIUGetProtectInfo = (PMIU_GETPROTECTINFO)malloc(sizeof(MIU_GETPROTECTINFO));
        if (pMIUGetProtectInfo == NULL)
            return FALSE;
        pMIUGetProtectInfo->u8MiuDev = u8MiuDev;
        pMIUGetProtectInfo->pInfo = pInfo;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_GetProtectInfo,(void*)pMIUGetProtectInfo) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_GetProtectInfo fail\n");
            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;
        }
        free(pMIUGetProtectInfo);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

MS_BOOL MDrv_MIU_SetGroupPriority(MS_U8 u8MiuDev, MIU_GroupPriority sPriority)
{
    PMIU_SETGROUPPRIORITY pMIUSetGroupPri;

    if (NULL != pInstantMIU)
    {
        pMIUSetGroupPri = (PMIU_SETGROUPPRIORITY)malloc(sizeof(MIU_SETGROUPPRIORITY));
        if (pMIUSetGroupPri == NULL)
            return FALSE;
        pMIUSetGroupPri->u8MiuDev = u8MiuDev;
        pMIUSetGroupPri->sPriority = sPriority;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_SetGroupPriority,(void*)pMIUSetGroupPri) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_SetGroupPriority fail\n");
            free(pMIUSetGroupPri);
    		return FALSE;
        }
        free(pMIUSetGroupPri);
    }
    return TRUE;
}


MS_BOOL MDrv_MIU_SetHPriorityMask(MS_U8 u8MiuDev, eMIUClientID eClientID, MS_BOOL bMask)
{
    PMIU_SETHIGHPRIORITY pMIUSetHighPri;

    if (NULL != pInstantMIU)
    {
        pMIUSetHighPri = (PMIU_SETHIGHPRIORITY)malloc(sizeof(MIU_SETHIGHPRIORITY));
        if (pMIUSetHighPri == NULL)
            return FALSE;
        pMIUSetHighPri->u8MiuDev = u8MiuDev;
        pMIUSetHighPri->eClientID = eClientID;
        pMIUSetHighPri->bMask = bMask;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_SetHighPriority,(void*)pMIUSetHighPri) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_MIU_SetHPriorityMask fail\n");
            free(pMIUSetHighPri);
    		return FALSE;
        }
        free(pMIUSetHighPri);
    }
    return TRUE;
}



MS_BOOL MDrv_MIUInit(void)
{
    PMIU_RET pMIURet;
    MS_BOOL bRet = TRUE;
    if ( bMiuInit == FALSE)
    {
        //if(UtopiaOpen(MODULE_MIU | KERNEL_MODE, &pInstantMIU, 0, pAttributeMIU) != UTOPIA_STATUS_SUCCESS)
        if(UtopiaOpen(MODULE_MIU, &pInstantMIU, 0, pAttributeMIU) != UTOPIA_STATUS_SUCCESS)
        {
	        ULOGE(TAG_MIU, "MIU had been initialized\n");
	        return FALSE;
        }
    }
    if (NULL != pInstantMIU)
    {
        pMIURet = (PMIU_RET)malloc(sizeof(MIU_RET));

        if(pMIURet == NULL)
            return FALSE;

        if(UtopiaIoctl(pInstantMIU, MDrv_CMD_MIU_Init,(void*)pMIURet) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_MIU_SetHPriorityMask fail\n");
            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;
        }

        free(pMIURet);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;

    //return _MDrv_MIU_Init();
}

void  MDrv_MIU_PrintMIUProtectInfo(void)
{
    return;
    if (NULL != pInstantMIU)
    {
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_PrintProtectInfo,(void*)NULL) != UTOPIA_STATUS_SUCCESS)
        {
			ULOGE(TAG_MIU, "Ioctl MDrv_MIU_SetHPriorityMask fail\n");
    	    return;
        }
    }
    return;
}

MS_BOOL MDrv_MIU_Dram_Size(MS_U8 u8MiuDev, MS_U8 DramSize)
{
    PMIU_DRAM_SIZE pMIUDramSize = NULL;
    MS_BOOL bRet = TRUE;

    if (NULL != pInstantMIU)
    {
        pMIUDramSize = (PMIU_DRAM_SIZE)malloc(sizeof(MIU_DRAM_SIZE));
        if (pMIUDramSize == NULL)
            return FALSE;
        pMIUDramSize->u8MiuDev = u8MiuDev;
        pMIUDramSize->DramSize = DramSize;
        pMIUDramSize->bRet = FALSE;
        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_Dram_Size,(void*)pMIUDramSize) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_Dram_Size fail\n");
            bRet = FALSE;
        }
        else
        {
            bRet = pMIUDramSize->bRet;
        }

        free(pMIUDramSize);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

MS_BOOL MDrv_MIU_ProtectAlign(MS_U32 *u32PageShift)
{
    PMIU_PROTECT_ALIGN pMIUProtectAlign = NULL;
    MS_BOOL bRet = TRUE;

    if (NULL != pInstantMIU)
    {
        pMIUProtectAlign = (PMIU_PROTECT_ALIGN)malloc(sizeof(MIU_PROTECT_ALIGN));
        if(pMIUProtectAlign == NULL)
            return FALSE;
        pMIUProtectAlign->u32PageShift = u32PageShift;
        pMIUProtectAlign->bRet = FALSE;
        if(UtopiaIoctl(pInstantMIU, MDrv_CMD_MIU_ProtectAlign,(void*)pMIUProtectAlign) != UTOPIA_STATUS_SUCCESS)
        {
            printf("Ioctl MDrv_CMD_MIU_ProtectAlign fail\n");
            bRet = FALSE;
        }
        else
        {
            bRet = pMIUProtectAlign->bRet;
        }

        free(pMIUProtectAlign);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

MS_BOOL MDrv_MIU_GetDramType(MIU_ID eMiu, MIU_DDR_TYPE* pType)
{
    MIU_GETDRAMTYPE args;
    MS_BOOL bRet = TRUE;

    if (NULL != pInstantMIU)
    {
        args.eMiu = eMiu;
        args.pType = pType;

        if(UtopiaIoctl(pInstantMIU,MDrv_CMD_MIU_GetDramType,(void*)&args) != UTOPIA_STATUS_SUCCESS)
        {
            ULOGE(TAG_MIU, "Ioctl MDrv_CMD_MIU_GetDramType fail\n");
            bRet = FALSE;
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}
#endif

MS_U32 MDrv_MIU_SetPowerState(EN_POWER_MODE u16PowerState)
{
    static EN_POWER_MODE u16PreMIUPowerState = E_POWER_MECHANICAL;
    MS_U32 u32Return = UTOPIA_STATUS_FAIL;

    if (u16PowerState == E_POWER_SUSPEND)
    {
        MDrv_MIU_Exit();
        u16PreMIUPowerState = u16PowerState;
        u32Return = UTOPIA_STATUS_SUCCESS;//SUSPEND_OK;
    }
    else if (u16PowerState == E_POWER_RESUME)
    {
        if (u16PreMIUPowerState == E_POWER_SUSPEND)
        {
            MDrv_MIUInit();

            u16PreMIUPowerState = u16PowerState;
            u32Return = UTOPIA_STATUS_SUCCESS;//RESUME_OK;
        }
        else
        {
            ULOGE(TAG_MIU, "[%s,%5d]It is not suspended yet. We shouldn't resume\n",__FUNCTION__,__LINE__);
            u32Return = UTOPIA_STATUS_FAIL;//SUSPEND_FAILED;
        }
    }
    else
    {
        ULOGE(TAG_MIU, "[%s,%5d]Do Nothing: %d\n",__FUNCTION__,__LINE__,u16PowerState);
        u32Return = UTOPIA_STATUS_FAIL;
    }

    return u32Return;// for success
}

void _MDrv_MIU_PrintMIUProtectInfo(void)
{
    HAL_MIU_PrintMIUProtectInfo();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_GetClientWidth
/// @brief \b Function  \b Description: Get MIU client bus width
/// @param u8MiuDev     \b IN   : select MIU0 or MIU1
/// @param eClientID    \b IN   : client ID
/// @param pClientWidth \b IN   : client bus width
/// @return             \b 0 : Fail  1: OK
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_GetClientWidth(MS_U8 u8MiuDevi, eMIUClientID eClientID, eMIU_ClientWidth *pClientWidth)
{
    return HAL_MIU_GetClientWidth(u8MiuDevi, eClientID, pClientWidth);
}

MS_BOOL _MDrv_MIU_GetDramType(MIU_ID eMiu, MIU_DDR_TYPE* pType)
{
    return HAL_MIU_GetDramType(eMiu, pType);
}

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
static void _MDrv_MIU_GetDramSize_String(MIU_DDR_SIZE eDramSize, char *pDramSizeString)
{
    switch(eDramSize)
    {
        case E_MIU_DDR_512MB:
            strcpy(pDramSizeString,"512");
            break;
        case E_MIU_DDR_1024MB:
            strcpy(pDramSizeString,"1024");
            break;
        case E_MIU_DDR_2048MB:
            strcpy(pDramSizeString,"2048");
            break;
        case E_MIU_DDR_4096MB:
            strcpy(pDramSizeString,"4096");
            break;
        case E_MIU_DDR_8192MB:
            strcpy(pDramSizeString,"8192");
            break;
        default:
            break;
    }
}

//static MS_U32 _MDrv_MIU_GetParaByChip(EN_MIU_GETPARABYCHIP eGetParaByChip, GetParaByChipHandle *pMIUPara)
//{
//    return HAL_MIU_GetParaByChip(eGetParaByChip, pMIUPara);
//}
static MS_BOOL _MDrv_MIU_GetSelectInfo(MS_U32 u32IdGroup, MS_U32 *pu32SelectInfo)
{
    return HAL_MIU_GetSelectInfo(u32IdGroup, pu32SelectInfo);
}

static MS_BOOL _MDrv_MIU_GetMaskInfo(MIU_ID eMIU, MS_U32 u32IdGroup, MS_U32 *pu32MaskInfo)
{
    return HAL_MIU_GetMaskInfo(eMIU, u32IdGroup, pu32MaskInfo);
}

static MS_BOOL _MDrv_MIU_MDBGetProtectInfo(MIU_ID eMIU, MIU_ProtectInfoHandle *pProtectInfo)
{
    return HAL_MIU_MDBGetProtectInfo(eMIU, pProtectInfo);
}

static MS_BOOL _MDrv_MIU_GetDramClock(MIU_ID eMIU, MS_U32 *pu32DramClock)
{
    return HAL_MIU_MDBGetDramClock(eMIU, pu32DramClock);
}

#define ParaNum_MIUProtect (6)
#define ParaNum_MIUMask (2)
#define CmdCnt (MIU_MAX_PROTECT_ID+ParaNum_MIUProtect)
static MS_U64 str2num[CmdCnt] = {0};
static void Reset_str2num(void)
{
    MS_U32 i = 0;
    for(i = 0; i < CmdCnt; i++)
        str2num[i] = 0;
}
static MS_U32 atoi_mstar(char *s, MS_U32 b)
{
    MS_U32 sum = 0;
    MS_U32 i = 0;
    printf("S's size: %d\n", (int)sizeof(s));
    for(i = 0;s[i] != '\0';i++)
    {
        if((s[i]=='A') || s[i]=='a')
        {
          sum = sum*16+10;
          str2num[b] = str2num[b]*16+10;
        }
        else if((s[i]=='B') || (s[i]=='b'))
        {
          sum = sum*16+11;
          str2num[b] = str2num[b]*16+11;
        }
        else if((s[i]=='C') || (s[i]=='c'))
        {
          sum = sum*16+12;
          str2num[b] = str2num[b]*16+12;
        }
        else if((s[i]=='D') || s[i]=='d')
        {
          sum = sum*16+13;
          str2num[b] = str2num[b]*16+13;
        }
        else if((s[i]=='E') || s[i]=='e')
        {
          sum = sum*16+14;
          str2num[b] = str2num[b]*16+14;
        }
        else if((s[i]=='F') || s[i]=='f')
        {
          sum = sum*16+15;
          str2num[b] = str2num[b]*16+15;
        }
        else if (s[i]==' ')
        {
          b++;
          if (b > CmdCnt-1)
            break;
        }
        else
        {
          if( s[i]=='0' && s[i+1]=='x' )
              i += 1;
          else
          {
          sum = sum*16+s[i]-'0';
          str2num[b] = str2num[b]*16+s[i]-'0';
          }
        }
    }
    printf("b=%d\n", (int)b);
    return b;
};

MS_BOOL MDrv_MIU_MdbCmdLine(MDBCMD_CMDLINE_PARAMETER *paraCmdLine, EN_MIU_MDBCMDLINE eMdbCmdLine)
{
    MIU_DDR_SIZE DramSize1 = E_MIU_DDR_32MB;
    char DramSizeString[20] = {0};
    char *pstr;
    char *pstr1;
    char *pch;
    MS_U32 index;
    MS_U32 u32chrtoint;
    MS_U32 u32chrtoint1;
    MS_U32 u32ParaInfo = 0x0;
    MS_U32 u32SelectInfo[MIU_MAX_DEVICE] = {0};
    MIU_ProtectInfoHandle ProtectInfo;
    char *strMIUProtect[ParaNum_MIUProtect] = {"MIU=0x","protect_num=0x","start_address=0x","end_address=0x","attribute=0x","whitelist=0x"};
    char *strMIUMask[ParaNum_MIUMask] = {"clientID=0x","mask=0x"};
    MS_U32 u32DramClock = 0x0;
    MS_U8 u8ClientID[MIU_MAX_PROTECT_ID] = {0};

    //for strint position
    char *strMIUCmdPara[] = {"=0x","MIU","miu_group"};
    MS_U32 CharLengthofStr_EqualtoHex = strlen(strMIUCmdPara[0]);
    MS_U32 CharLengthofStr = 0x0;
    //for initial
    memset(&ProtectInfo,0,sizeof(MIU_ProtectInfoHandle));
    _MDrv_MIU_Init();
    switch(eMdbCmdLine)
    {
        char temp[255];
        case MIU_MDBCMDLINE_HELP:
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU help---------\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"miu_protect\tget miu protect information\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"dram_size\tcheck miu dram size\n");
            break;
        case MIU_MDBCMDLINE_MIU_PROTECT:
            pstr = strstr(paraCmdLine->pcCmdLine, strMIUProtect[0]);
            if(pstr == NULL)
            {
                MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU0 protect Info---------\n");
                if(_MDrv_MIU_MDBGetProtectInfo(E_MIU_0, &ProtectInfo) == FALSE)
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"Not Suppurt MIU0!\n");
                    break;
                }
                MdbPrint(paraCmdLine->u64ReqHdl,"White list\tProtect0_en\tProtect1_en\tProtect2_en\tProtect3_en\n");
                for ( index = 0; index < MIU_MAX_PROTECT_ID; index ++)
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"ID%d:\t0x%x\t0x%x\t\t0x%x\t\t0x%x\t\t0x%x\n",index, ProtectInfo.WhiteList[index], ProtectInfo.WhiteListEnable[0][index], ProtectInfo.WhiteListEnable[1][index], ProtectInfo.WhiteListEnable[2][index], ProtectInfo.WhiteListEnable[3][index]);
                }
                MdbPrint(paraCmdLine->u64ReqHdl,"\nProtect range\n");
                for ( index = 0; index < MIU_MAX_PROTECT_BLOCK; index ++)
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"No%d: 0x%x - 0x%x\n",index, (unsigned int)ProtectInfo.ProtectAddrStart[index], (unsigned int)ProtectInfo.ProtectAddrEnd[index]);
                }
                MdbPrint(paraCmdLine->u64ReqHdl,"\nProtect attribute\n");
                for ( index = 0; index < MIU_MAX_PROTECT_BLOCK; index ++)
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"No%d: 0x%x\n",index, (ProtectInfo.ProtectAttribute >> index & BIT0));
                }
                MdbPrint(paraCmdLine->u64ReqHdl,"\nProtect hit info\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"hit protect area:\t0x%x\n",ProtectInfo.HitPortectInfo.u8Block);
                MdbPrint(paraCmdLine->u64ReqHdl,"hit Group:\t0x%x\n",ProtectInfo.HitPortectInfo.u8Group);
                MdbPrint(paraCmdLine->u64ReqHdl,"hit Client ID:\t0x%x\n",ProtectInfo.HitPortectInfo.u8ClientID);

                MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU1 protect Info---------\n");
                if(_MDrv_MIU_MDBGetProtectInfo(E_MIU_1, &ProtectInfo) == FALSE)
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"Not Suppurt MIU1!\n");
                    break;
                }
                MdbPrint(paraCmdLine->u64ReqHdl,"White list\tProtect0_en\tProtect1_en\tProtect2_en\tProtect3_en\n");
                for ( index = 0; index < MIU_MAX_PROTECT_ID; index ++)
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"ID%d:\t0x%x\t0x%x\t\t0x%x\t\t0x%x\t\t0x%x\n",index, ProtectInfo.WhiteList[index], ProtectInfo.WhiteListEnable[0][index], ProtectInfo.WhiteListEnable[1][index], ProtectInfo.WhiteListEnable[2][index], ProtectInfo.WhiteListEnable[3][index]);
                }
                MdbPrint(paraCmdLine->u64ReqHdl,"\nProtect range\n");
                for ( index = 0; index < MIU_MAX_PROTECT_BLOCK; index ++)
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"No%d: 0x%x - 0x%x\n",index, ProtectInfo.ProtectAddrStart[index], ProtectInfo.ProtectAddrEnd[index]);
                }
                MdbPrint(paraCmdLine->u64ReqHdl,"\nProtect attribute\n");
                for ( index = 0; index < MIU_MAX_PROTECT_BLOCK; index ++)
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"No%d: 0x%x\n",index, (ProtectInfo.ProtectAttribute >> index & BIT0));
                }
                MdbPrint(paraCmdLine->u64ReqHdl,"\nProtect hit info\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"hit protect area:\t0x%x\n",ProtectInfo.HitPortectInfo.u8Block);
                MdbPrint(paraCmdLine->u64ReqHdl,"hit Group:\t0x%x\n",ProtectInfo.HitPortectInfo.u8Group);
                MdbPrint(paraCmdLine->u64ReqHdl,"hit Client ID:\t0x%x\n",ProtectInfo.HitPortectInfo.u8ClientID);
                break;
            }
            //set miu protect
            Reset_str2num();
            for(index = 1; index < ParaNum_MIUProtect; index ++)
            {
                pstr1 = strstr(pstr, strMIUCmdPara[0]);
                if (pstr1 == NULL)
                    return FALSE;

                pstr = strstr(paraCmdLine->pcCmdLine, strMIUProtect[index]);  //find next parameter head
                if (pstr == NULL)
                    return FALSE;
                strncpy(temp,pstr1+CharLengthofStr_EqualtoHex,pstr-pstr1-CharLengthofStr_EqualtoHex); //parameter between "=0x" and next parameter head
                *(temp+(pstr-pstr1-CharLengthofStr_EqualtoHex)) = '\0';  //add the end char
                atoi_mstar(temp, index-1);  //chanege str to number to str2num[]
            }
            pch = strchr(paraCmdLine->pcCmdLine, '\0');  //end of cmdline
            if(pch == NULL)
                return FALSE;
            printf("%d\n",(int)sizeof(strMIUProtect[4]));
            CharLengthofStr = strlen(strMIUProtect[5]);
            strncpy(temp,pstr+CharLengthofStr,pch-pstr-CharLengthofStr);  //list after "whitelist=0x"
            *(temp+(pch-pstr-CharLengthofStr)) = '\0';
            atoi_mstar(temp, index-1);
            HAL_MIU_GetSWClientInfo((MIU_ID)str2num[0],&str2num[5]);
            for( index = 0; index < MIU_MAX_PROTECT_ID; index++)
                u8ClientID[index] = (MS_U8)str2num[index+5];
            if(_MDrv_MIU_Protect(str2num[1],u8ClientID,(MS_U32)str2num[2], (MS_U32) str2num[3], (MS_BOOL) str2num[4])==FALSE)
                MdbPrint(paraCmdLine->u64ReqHdl,"_MDrv_MIU_Protect FLASE\n");
            break;
        case MIU_MDBCMDLINE_MIU_SELECT:
            pstr = strstr(paraCmdLine->pcCmdLine, "miu_group");
            if(pstr == NULL)
                break;
            pstr1 = strstr(pstr, strMIUCmdPara[0]);
            if( pstr1 == NULL)
                break;
            pch = strchr(pstr1, '\0');
            if( pch == NULL)
                break;
            strncpy(temp,pstr1+CharLengthofStr_EqualtoHex,pch-pstr1-CharLengthofStr_EqualtoHex);
            MdbPrint(paraCmdLine->u64ReqHdl,"pstr1 = %x, pch = %x, temp = %c\n",pstr1, pch,temp[0]);
            u32chrtoint = temp[0] - '0';
            if(u32chrtoint > MIU_MAX_GROUP)
                break;
            MdbPrint(paraCmdLine->u64ReqHdl,"u32chrtoint = 0x%x\n",u32chrtoint);
            _MDrv_MIU_GetSelectInfo(u32chrtoint, u32SelectInfo);
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU select Info---------\n");
            switch(MIU_MAX_DEVICE)
            {
                case 3:
                    MdbPrint(paraCmdLine->u64ReqHdl,"ID\tMIU0\tMIU1\tMIU2\n");
                    for ( index = 0; index < MIU_MAX_PROTECT_ID; index ++)
                    {
                        MdbPrint(paraCmdLine->u64ReqHdl,"No%d\t0x%x\t0x%x\t0x%x\n",index,u32SelectInfo[0] >> index & BIT0, u32SelectInfo[1] >> index & BIT0, u32SelectInfo[2] >> index & BIT0);
                    }
                        break;
                case 2:
                    MdbPrint(paraCmdLine->u64ReqHdl,"ID\tMIU0\tMIU1\n");
                    for ( index = 0; index < MIU_MAX_PROTECT_ID; index ++)
                    {
                        MdbPrint(paraCmdLine->u64ReqHdl,"No%d\t0x%x\t0x%x\n",index,u32SelectInfo[0] >> index & BIT0, u32SelectInfo[1] >> index & BIT0);
                    }
                        break;
                case 1:
                    MdbPrint(paraCmdLine->u64ReqHdl,"ID\tMIU0\n");
                    for ( index = 0; index < MIU_MAX_PROTECT_ID; index ++)
                    {
                        MdbPrint(paraCmdLine->u64ReqHdl,"No%d\t0x%x\n",index,u32SelectInfo[0] >> index & BIT0);
                    }
                    break;
                default:
                break;
            }
            break;
        case MIU_MDBCMDLINE_MIU_MASK:
            pstr = strstr(paraCmdLine->pcCmdLine, strMIUCmdPara[1]);
            if(pstr == NULL)
                break;
            pstr1 = strstr(pstr, strMIUCmdPara[0]);
            if( pstr1 == NULL)
                break;
            pch = strchr(pstr1, ' ');
            if( pch == NULL)
                break;
            strncpy(temp,pstr1+CharLengthofStr_EqualtoHex,pch-pstr1-CharLengthofStr_EqualtoHex);
            MdbPrint(paraCmdLine->u64ReqHdl,"pstr1 = %x, pch = %x, temp = %c\n",pstr1, pch,temp[0]);
            u32chrtoint = temp[0] - '0';	 //'0' ASCI = 48
            if(u32chrtoint > MIU_MAX_DEVICE)
                break;
            pstr = strstr(paraCmdLine->pcCmdLine, strMIUCmdPara[2]);
            if(pstr == NULL)
            {
                //Set MIU mask
                Reset_str2num();
                pstr = strstr(paraCmdLine->pcCmdLine, strMIUMask[0]);
                if(pstr == NULL)
                    break;
                pstr1 = strstr(paraCmdLine->pcCmdLine, strMIUMask[1]);
                if (pstr1 == NULL)
                    break;
                CharLengthofStr = strlen(strMIUMask[0]);
                strncpy(temp,pstr+CharLengthofStr,pstr1-pstr-CharLengthofStr);  //parameter between "clientID=0x" and next parameter head
                *(temp+(pstr1-pstr-CharLengthofStr)) = '\0';  //add the end char
                atoi_mstar(temp, 0);  //chanege str to number to str2num[]
                HAL_MIU_GetSWClientInfo((MIU_ID)u32chrtoint,&str2num[0]);
                pch = strchr(paraCmdLine->pcCmdLine, '\0');
                if( pch == NULL)
                    break;
                CharLengthofStr = strlen(strMIUMask[1]);
                strncpy(temp,pstr1+CharLengthofStr,pch-pstr1-CharLengthofStr); //parameter between "mask=0x" and endh
                u32chrtoint1 = temp[0] - '0';  //minuse ASCII '0'
                if(u32chrtoint1 == 0x0)
                {
                    _MDrv_MIU_UnMaskReq(u32chrtoint, str2num[0]);
                    MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU SET unmask---------\n");
                }
                else
                {
                    _MDrv_MIU_MaskReq(u32chrtoint, str2num[0]);
                    MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU SET mask---------\n");
                }
                break;
            }

            //shoe info
            pstr1 = strstr(pstr, strMIUCmdPara[0]);
            if( pstr1 == NULL)
                break;
            pch = strchr(pstr1, '\0');
            if( pch == NULL)
                break;
            strncpy(&temp[1],pstr1+CharLengthofStr_EqualtoHex,pch-pstr1-CharLengthofStr_EqualtoHex);
            MdbPrint(paraCmdLine->u64ReqHdl,"pstr1 = %x, pch = %x, temp = %c\n",pstr1, pch,temp[1]);
            u32chrtoint1 = temp[1] - '0';
            if(u32chrtoint1 > MIU_MAX_GROUP)
                break;
            if(_MDrv_MIU_GetMaskInfo(u32chrtoint, u32chrtoint1, &u32ParaInfo) == FALSE)
                break;
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU mask Info---------\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"ID\tMask\n");
            for ( index = 0; index < MIU_MAX_PROTECT_ID; index ++)
            {
                MdbPrint(paraCmdLine->u64ReqHdl,"No%d\t0x%x\n",index, u32ParaInfo >> index &BIT0);
            }
            break;
        case MIU_MDBCMDLINE_MIU_DRAMSIZE:
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU DRAM Size Info---------\n");
            if( MDrv_MIU_Dram_ReadSize(E_MIU_0, &DramSize1) == FALSE )
            {
                MdbPrint(paraCmdLine->u64ReqHdl,"Not Support MIU0!\n");
                break;
            }
            _MDrv_MIU_GetDramSize_String(DramSize1, DramSizeString);
            MdbPrint(paraCmdLine->u64ReqHdl,"MIU0 size: %s MB\n",DramSizeString);
            if( MDrv_MIU_Dram_ReadSize(E_MIU_1, &DramSize1) == FALSE )
            {
                MdbPrint(paraCmdLine->u64ReqHdl,"Not Support MIU1!\n");
                break;
            }
            _MDrv_MIU_GetDramSize_String(DramSize1, DramSizeString);
            MdbPrint(paraCmdLine->u64ReqHdl,"MIU1 size: %s MB\n",DramSizeString);
            break;
        case MIU_MDBCMDLINE_MIU_DRAMCLOCK:
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU DRAM Clock Info---------\n");
            _MDrv_MIU_GetDramClock(E_MIU_0, &u32DramClock);
            MdbPrint(paraCmdLine->u64ReqHdl,"MIU0 clock: 0x%x\n",u32DramClock);
            _MDrv_MIU_GetDramClock(E_MIU_1, &u32DramClock);
            MdbPrint(paraCmdLine->u64ReqHdl,"MIU1 clock: 0x%x\n",u32DramClock);
            MdbPrint(paraCmdLine->u64ReqHdl,"\nEach DRAM clocks is different from each boards and actually DRAM clocks have to be calculate.\n\
If wanna get actually DRAM clocks, please contact to MSTAR FAE and provide informations about MIU clocks and borad!\n");
            break;
        case MIU_MDBCMDLINE_MIU_MSTV_MIU_PROTECT:
            MdbPrint(paraCmdLine->u64ReqHdl,"---------Setting MIU Protect by using MSTV Tool---------\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"1. Choose and move to MIU bank:	MIU0 0x1012, MIU1 0x1006n\n\
2. Use which block:	(x)->(0), (1), (2), (3)\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"3. Set MIU protect address(18bits)(8k byte aling):	\n\
\t(0)     Start=16bh68[1:0]|16bh60[15:0]    End=16bh68[3:2]|16bh61[15:0]\n\
\t(1)	Start=16bh68[5:4]|16bh62[15:0]    End=16bh68[7:6]|16bh63[15:0]\n\
\t(2)	Start=16bh68[9:8]|16bh64[15:0]    End=16bh68[11:10]|16bh65[15:0]\n\
\t(3)	Start=16bh68[13:12]|16bh66[15:0]  End=16bh68[15:14]|16bh67[15:0]\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"4. Set which MIU client ID could write block (x):	[y] = MIU Client ID\n\
\t[0]	16bh17[7:0]		[1]	16bh17[15:8]\n\
\t[2]	16bh18[7:0]		[3]	16bh18[15:8]\n\
\t[4]	16bh19[7:0]		[5]	16bh19[15:8]\n\
\t[6]	16bh1A[7:0]		[7]	16bh1A[15:8]\n\
\t[8]	16bh1B[7:0]		[9]	16bh1B[15:8]\n\
\t[10]	16bh1C[7:0]		[11]	16bh1C[15:8]\n\
\t[12]	16bh1D[7:0]		[13]	16bh1D[15:8]\n\
\t[14]	16bh1E[7:0]		[15]	16bh1E[15:8]\n\
\t==Set MIU client ID(See MIU_client_table.xls) which be added to access MIU protect range list.==\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"5. Enable [y] MIU client ID could write block (x):		Enable = (x)bit[y]\n\
	(0) 16bh10[y]	(1) 16bh11[y]	(2) 16bh12[y]	(3) 16bh13[y]\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"6. Enable block (x) of MIU protect:	Enable = (x)bit[x]\n\
	(0) 16bh69[0]	(1) 16bh69[1]	(2) 16bh69[2]	(3) 16bh69[3]\n\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"Example-MIU0:\n\t1. Bank 0x1012\n\
\t2. Block (0)\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"\t3. DRAM memory map: \n\
\t\t/* E_LX_MEM   */\n\
\t\t#define E_LX_MEM_LAYER                         0\n\
\t\t#define E_LX_MEM_AVAILABLE                     0x0000200000\n\
\t\t#define E_LX_MEM_ADR                           0x0000200000  //Alignment 0x100000\n\
\t\t#define E_LX_MEM_GAP_CHK                       0x0000000000\n\
\t\t#define E_LX_MEM_LEN                           0x0039000000\n\
\t\t#define E_LX_MEM_MEMORY_TYPE                   (MIU0 | TYPE_NONE | UNCACHE | TYPE_NONE)\n\
\t\t#define E_LX_MEM_CMA_HID\n\
\t\tStart address(PA) = 0x20000,	End address(PA) = 0x39200000\n\
\t\tif MIU1 need to be subtract the MIU1 offset\n\
\t\tStart address \n\
\t\t    ->0x20000/8k byte = 0x10	\n\
\t\t    ->high 2 bits:	16bh68[1:0] = 0x0\n\
\t\t    ->low 16 bits:	16bh60[15:0] = 0x10\n\
\t\tEnd address\n\
\t\t    ->0x39200000/8k byte = 0x1C900\n\
\t\t    -> high 2 bits:	16bh68[3:2] = 0x01\n\
\t\t    -> low 16 bits:	16bh61[15:0] = 0xC900\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"\t4. Set MIU client ID(See MIU_client_table.xls)\n\
\t\tIf wanted Group 7, client 0 is CPU => MIU client ID is 0x70 to be added into whitelist,\n\
\t\tsearch [y]16bh17-16bh1E (y=0~15) whether have value 0x70\n\
\t\tIf YES and found y = 1, enable (0)	16bh10[1] = 0x1\n\
\t\tIf No, find which [y] 16bh1A[7:0] = 0x0, y = 2 \n\
\t\tThen, set [2] = 0x70 and enable (0)	16bh10[2] = 0x1\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"\t5. Enable MIU protect	(0) 16bh69[0] = 0x1\n\n");
            break;
        default:
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar MIU---------\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"No have this cmd!\nPlease use 'echo help > /proc/utopia_mdb/miu' get cmdline!\n");
            return FALSE;
    }
    return TRUE;
}
#endif
