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
/// file   mdrvSC.c
/// @brief   API Interface
/// @author MStar Semiconductor,Inc.
/// @attention
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "MsTypes.h"

#include "drvSC.h"
#include "drvSC_v2.h"
#include "drvSC_private.h"

#include "utopia.h"
#include "utopia_dapi.h"
#include "MsOS.h"
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define SC_MDRV_DBG 0

#if defined(MSOS_TYPE_LINUX_KERNEL)
#if SC_MDRV_DBG
#define SC_MSG(_f, _a...)                             printk(_f, ##_a)
#else
#define SC_MSG(_f, _a...)
#endif

#else //defined(MSOS_TYPE_LINUX_KERNEL)
#if SC_MDRV_DBG
#define SC_MSG(_f, _a...)                             printf(_f, ##_a)
#else
#define SC_MSG(_f, _a...)
#endif
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
enum
{
    SC_POOL_ID_SC0 = 0
} eSC_PoolID;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void SCRegisterToUtopia(FUtopiaOpen ModuleType)
{
    // 1. create a module(module_name, SHM_size), and register to utopia2.0
    void* pUtopiaModule = NULL;
    UtopiaModuleCreate(MODULE_SC, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);

    // register func for module, after register here, then ap call UtopiaOpen/UtopiaIoctl/UtopiaClose can call to these registered standard func
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)SCOpen, (FUtopiaClose)SCClose, (FUtopiaIOctl)SCIoctl);

    // 2. Resource register
    void* psResource = NULL;

    // start func to add resources of a certain Resource_Pool
    UtopiaModuleAddResourceStart(pUtopiaModule, SC_POOL_ID_SC0);

    // create a resouce and regiter it to a certain Resource_Pool, resource can alloc private for internal use
    UtopiaResourceCreate("sc", sizeof(SC_RESOURCE_PRIVATE), &psResource);
    UtopiaResourceRegister(pUtopiaModule, psResource, SC_POOL_ID_SC0);

    // end func to add resources of a certain Resource_Pool(this will set the ResourceSemaphore of this ResourcePool)
    UtopiaModuleAddResourceEnd(pUtopiaModule, SC_POOL_ID_SC0);
}

MS_U32 SCOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{
    SC_INSTANT_PRIVATE *psScInstPri = NULL;
    void *pScPriVoid = NULL;

    SC_MSG("[%s][%d]\n", __FUNCTION__, __LINE__);

    if(ppInstance == 0)
    {
        return UTOPIA_STATUS_PARAMETER_ERROR;
    }

    //create instance
    //1.  Allocate instant private data, prepare argument variable.
    //2.  Dst: ppInstance
    UtopiaInstanceCreate(sizeof(SC_INSTANT_PRIVATE), ppInstance);

    //get instant private data pointer
    // 1. Dst: pScPriVoid
    UtopiaInstanceGetPrivate(*ppInstance, (void**)&pScPriVoid);
    psScInstPri = (SC_INSTANT_PRIVATE*)pScPriVoid;
    psScInstPri->fpSCInit = _MDrv_SC_Init;
    psScInstPri->fpSCExit = _MDrv_SC_Exit;
    psScInstPri->fpSCOpen = _MDrv_SC_Open;
    psScInstPri->fpSCActivate = _MDrv_SC_Activate;
    psScInstPri->fpSCDeactivate = _MDrv_SC_Deactivate;
    psScInstPri->fpSCClose = _MDrv_SC_Close;
    psScInstPri->fpSCReset = _MDrv_SC_Reset;
    psScInstPri->fpSCClearState = _MDrv_SC_ClearState;
    psScInstPri->fpSCGetATR = _MDrv_SC_GetATR;
    psScInstPri->fpSCConfig = _MDrv_SC_Config;
    psScInstPri->fpSCSend = _MDrv_SC_Send;
    psScInstPri->fpSCRecv = _MDrv_SC_Recv;
    psScInstPri->fpSCGetCaps = _MDrv_SC_GetCaps;
    psScInstPri->fpSCSetPPS = _MDrv_SC_SetPPS;
    psScInstPri->fpSCPPS = _MDrv_SC_PPS;
    psScInstPri->fpSCGetStatus = _MDrv_SC_GetStatus;
    psScInstPri->fpSCSetDbgLevel = _MDrv_SC_SetDbgLevel;
    psScInstPri->fpSCRawExchange = _MDrv_SC_RawExchange;
    psScInstPri->fpSCCardVoltage_Config = _MDrv_SC_CardVoltage_Config;
    psScInstPri->fpSCReset_ATR = _MDrv_SC_Reset_ATR;
    psScInstPri->fpSCT0_SendRecv = _MDrv_SC_T0_SendRecv;
    psScInstPri->fpSCT1_SendRecv = _MDrv_SC_T1_SendRecv;
    psScInstPri->fpSCT14_SendRecv = _MDrv_SC_T14_SendRecv;
    psScInstPri->fpSCGetInfo = _MDrv_SC_GetInfo;
    psScInstPri->fpSCPowerOff = _MDrv_SC_PowerOff;
    psScInstPri->fpSCSetGuardTime = _MDrv_SC_SetGuardTime;
    psScInstPri->fpSCSetPowerState = _MDrv_SC_SetPowerState;
#if defined(MSOS_TYPE_LINUX_KERNEL)
    psScInstPri->fpSCKDrvKernelModePoll = _SC_KDrvKernelModePoll;
#endif

    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 SCIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
    MS_U32 u32Ret = 0;
    void* pModule = NULL;
	UtopiaInstanceGetModule(pInstance, &pModule);

    SC_INSTANT_PRIVATE *psScInstPri = NULL;
    void *pScPriVoid = NULL;
	UtopiaInstanceGetPrivate(pInstance, (void**)&pScPriVoid);
    psScInstPri = (SC_INSTANT_PRIVATE*)pScPriVoid;

    switch(u32Cmd)
    {
        // General API
        case E_MDRV_CMD_SC_Init:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Init\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            SC_GENERIC_PARAM u8Param = *((SC_GENERIC_PARAM*)pArgs);
            u32Ret = psScInstPri->fpSCInit(u8Param);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_Exit:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Exit\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            SC_GENERIC_PARAM u8Param = *((SC_GENERIC_PARAM*)pArgs);
            u32Ret = psScInstPri->fpSCExit(u8Param);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_Open:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Open\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_OPEN_PARAM pstParam = (PSC_OPEN_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCOpen(pstParam->u8SCID, pstParam->u8Protocol, pstParam->pParam, pstParam->pfSmartNotify);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_Activate:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Activate\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            SC_GENERIC_PARAM u8Param = *((SC_GENERIC_PARAM*)pArgs);
            u32Ret = psScInstPri->fpSCActivate(u8Param);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_Deactivate:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Deactivate\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            SC_GENERIC_PARAM u8Param = *((SC_GENERIC_PARAM*)pArgs);
            u32Ret = psScInstPri->fpSCDeactivate(u8Param);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_Close:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Close\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            SC_GENERIC_PARAM u8Param = *((SC_GENERIC_PARAM*)pArgs);
            u32Ret = psScInstPri->fpSCClose(u8Param);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_Reset:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Reset\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_RESET_PARAM pstParam = (PSC_RESET_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCReset(pstParam->u8SCID, pstParam->pParam);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_ClearState:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_ClearState\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            SC_GENERIC_PARAM u8Param = *((SC_GENERIC_PARAM*)pArgs);
            u32Ret = psScInstPri->fpSCClearState(u8Param);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_GetATR:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_GetATR\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_GETATR_PARAM pstParam = (PSC_GETATR_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCGetATR(pstParam->u8SCID, pstParam->u32TimeOut, pstParam->pu8Atr, pstParam->pu16AtrLen, pstParam->pu8His, pstParam->pu16HisLen);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_Conig:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Conig\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_CONFIG_PARAM pstParam = (PSC_CONFIG_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCConfig(pstParam->u8SCID, pstParam->pParam);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_Send:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Send\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_SEND_PARAM pstParam = (PSC_SEND_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCSend(pstParam->u8SCID, pstParam->pu8SendData, pstParam->u16SendDataLen, pstParam->u32TimeoutMs);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_Recv:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_Recv\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_RECV_PARAM pstParam = (PSC_RECV_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCRecv(pstParam->u8SCID, pstParam->pu8ReadData, pstParam->pu16ReadDataLen, pstParam->u32TimeoutMs);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_GetCaps:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_GetCaps\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_GETCAPS_PARAM pstParam = (PSC_GETCAPS_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCGetCaps(pstParam->pstCaps);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_SetPPS:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_SetPPS\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_SETPPS_PARAM pstParam = (PSC_SETPPS_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCSetPPS(pstParam->u8SCID, pstParam->u8SCProtocol, pstParam->u8Di, pstParam->u8Fi);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_PPS:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_PPS\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            SC_GENERIC_PARAM u8Param = *((SC_GENERIC_PARAM*)pArgs);
            u32Ret = psScInstPri->fpSCPPS(u8Param);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_GETSTATUS:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_GETSTATUS\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_GETSTATUS_PARAM pstParam = (PSC_GETSTATUS_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCGetStatus(pstParam->u8SCID, pstParam->pstStatus);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_SETDBGLEVEL:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_SETDBGLEVEL\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            SC_SETDBGLEVEL_PARAM eParam = *((SC_SETDBGLEVEL_PARAM*)pArgs);
            psScInstPri->fpSCSetDbgLevel(eParam);
            return UTOPIA_STATUS_SUCCESS;
        }
        break;

        case E_MDRV_CMD_SC_RAWEXCHANGE:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_RAWEXCHANGE\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_RAWEXCHANGE_PARAM pstParam = (PSC_RAWEXCHANGE_PARAM)pArgs;
            u32Ret = psScInstPri->fpSCRawExchange(pstParam->u8SCID, pstParam->pu8SendData, pstParam->u16SendDataLen, pstParam->pu8ReadData, pstParam->u16ReadDataLen);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_CARDVOLTAGE_CONFIG:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_CARDVOLTAGE_CONFIG\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_CARDVOLTAGE_CONFIG_PARAM pstParam = (PSC_CARDVOLTAGE_CONFIG_PARAM)pArgs;
            if (psScInstPri->fpSCCardVoltage_Config(pstParam->u8SCID, pstParam->eVoltage))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

#if defined(MSOS_TYPE_LINUX_KERNEL)
        case E_MDRV_CMD_SC_KDRVKERNELMODEPOLL:
        {
            //SC_MSG("[%s][%d] E_MDRV_CMD_SC_KDRVKERNELMODEPOLL\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_KDRVKERNELMODEPOLL_PARAM pstParam = (PSC_KDRVKERNELMODEPOLL_PARAM)pArgs;
            if (psScInstPri->fpSCKDrvKernelModePoll(pstParam->u8SCID, pstParam->pu32Event, pstParam->u32TimeoutMs))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;
#endif

        case E_MDRV_CMD_SC_RESET_ATR:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_RESET_ATR\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_RESET_ATR_PARAM pstParam = (PSC_RESET_ATR_PARAM)pArgs;
            if (psScInstPri->fpSCReset_ATR(pstParam->u8SCID, pstParam->pParam, pstParam->pu8Atr, pstParam->pu16AtrLen, pstParam->pu8His, pstParam->pu16HisLen))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_T0_SENDRECV:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_T0_SENDRECV\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_T0_SENDRECV_PARAM pstParam = (PSC_T0_SENDRECV_PARAM)pArgs;
            if (psScInstPri->fpSCT0_SendRecv(pstParam->u8SCID, pstParam->pu8SendData, pstParam->u16SendLen, pstParam->pu8RecvData, pstParam->pu16RecvLen))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_T1_SENDRECV:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_T1_SENDRECV\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_T1_SENDRECV_PARAM pstParam = (PSC_T1_SENDRECV_PARAM)pArgs;
            if (psScInstPri->fpSCT1_SendRecv(pstParam->u8SCID, pstParam->pu8SendData, pstParam->u16SendDataLen, pstParam->pu8ReadData, pstParam->u16ReadDataLen))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_T14_SENDRECV:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_T14_SENDRECV\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_T14_SENDRECV_PARAM pstParam = (PSC_T14_SENDRECV_PARAM)pArgs;
            if (psScInstPri->fpSCT14_SendRecv(pstParam->u8SCID, pstParam->pu8SendData, pstParam->u16SendLen, pstParam->pu8RecvData, pstParam->pu16RecvLen))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_GETINFO:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_GETINFO\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_GETINFO_PARAM pstParam = (PSC_GETINFO_PARAM)pArgs;
            if (psScInstPri->fpSCGetInfo(pstParam->u8SCID, pstParam->pstInfo))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_POWEROFF:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_POWEROFF\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            if (psScInstPri->fpSCPowerOff())
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_SETGUARDTIME:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_SETGUARDTIME\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PSC_SETGUARDTIME_PARAM pstParam = (PSC_SETGUARDTIME_PARAM)pArgs;
            if (psScInstPri->fpSCSetGuardTime(pstParam->u8SCID, pstParam->u8GuardTime))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        case E_MDRV_CMD_SC_SETPOWERSTATE:
        {
            SC_MSG("[%s][%d] E_MDRV_CMD_SC_SETPOWERSTATE\n", __FUNCTION__, __LINE__);
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            SC_SETPOWERSTATE_PARAM u8Param = *((SC_SETPOWERSTATE_PARAM*)pArgs);
            u32Ret = psScInstPri->fpSCSetPowerState(u8Param);
            if (E_SC_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        break;

        default:
            SC_MSG("[%s][%d] Unknown commend 0x%x!!!\n", __FUNCTION__, __LINE__, (int)u32Cmd);
            return UTOPIA_STATUS_FAIL;
    }

    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 SCClose(void* pInstance)
{
    SC_MSG("[%s][%d]\n", __FUNCTION__, __LINE__);
    UtopiaInstanceDelete(pInstance);

    return UTOPIA_STATUS_SUCCESS;
}
