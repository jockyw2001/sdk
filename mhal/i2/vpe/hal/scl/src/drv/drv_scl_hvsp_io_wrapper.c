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
#define __DRV_SCL_HVSP_IO_WRAPPER_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"

#include "drv_scl_verchk.h"
#include "drv_scl_irq_st.h"
#include "drv_scl_hvsp_m.h"
#include "drv_scl_dma_m.h"
#include "drv_scl_hvsp_m.h"
#include "drv_scl_hvsp_io_st.h"
#include "drv_scl_hvsp_io_wrapper.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define DRV_SCLHVSP_IO_LOCK_MUTEX(x)    DrvSclOsObtainMutex(x, SCLOS_WAIT_FOREVER)
#define DRV_SCLHVSP_IO_UNLOCK_MUTEX(x)  DrvSclOsReleaseMutex(x)
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    s32 s32Id;
    DrvSclHvspIoIdType_e enIdType;
    MDrvSclCtxLockConfig_t *pLockCfg;
    MDrvSclCtxIdType_e enCtxId;
}DrvSclHvspIoCtxConfig_t;


typedef struct
{
    s32 s32Handle;
    DrvSclHvspIoIdType_e enSclHvspId;
    DrvSclHvspIoCtxConfig_t stCtxCfg;
} DrvSclHvspIoHandleConfig_t;

typedef struct
{
    u32 u32StructSize;
    u32 *pVersion;
    u32 u32VersionSize;
} DrvSclHvspIoVersionChkConfig_t;


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
//keep
DrvSclHvspIoHandleConfig_t _gstSclHvspHandler[DRV_SCLHVSP_HANDLER_MAX];
DrvSclHvspIoFunctionConfig_t _gstSclHvspIoFunc;
s32 _s32SclHvspIoHandleMutex = -1;
u8  gbdbgmessage[EN_DBGMG_NUM_CONFIG];//extern

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
void _DrvSclHvspIoFillVersionChkStruct
    (u32 u32StructSize, u32 u32VersionSize, u32 *pVersion,DrvSclHvspIoVersionChkConfig_t *stVersion)
{
    stVersion->u32StructSize  = (u32)u32StructSize;
    stVersion->u32VersionSize = (u32)u32VersionSize;
    stVersion->pVersion      = (u32 *)pVersion;
}

s32 _DrvSclHvspIoVersionCheck(DrvSclHvspIoVersionChkConfig_t *stVersion)
{
    if ( CHK_VERCHK_HEADER(stVersion->pVersion) )
    {
        if( CHK_VERCHK_MAJORVERSION_LESS( stVersion->pVersion, DRV_SCLHVSP_VERSION) )
        {

            VERCHK_ERR("[HVSP] Version(%04lx) < %04x!!! \n",
                       *(stVersion->pVersion) & VERCHK_VERSION_MASK,
                       DRV_SCLHVSP_VERSION);

            return -1;
        }
        else
        {
            if( CHK_VERCHK_SIZE( &stVersion->u32VersionSize, stVersion->u32StructSize) == 0 )
            {
                VERCHK_ERR("[HVSP] Size(%04lx) != %04lx!!! \n",
                           stVersion->u32StructSize,
                           stVersion->u32VersionSize);

                return -1;
            }
            else
            {
                return VersionCheckSuccess;
            }
        }
    }
    else
    {
        VERCHK_ERR("[HVSP] No Header !!! \n");
        SCL_ERR( "[HVSP]   %s  \n", __FUNCTION__);
        return -1;
    }
}
bool _DrvSclHvspIoGetMdrvIdType(s32 s32Handler, MDrvSclHvspIdType_e *penHvspId)
{
    s16 i;
    s16 s16Idx = -1;
    bool bRet = TRUE;

    for(i = 0; i < DRV_SCLHVSP_HANDLER_MAX; i++)
    {
        if(_gstSclHvspHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        *penHvspId = E_MDRV_SCLHVSP_ID_MAX;
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
        switch(_gstSclHvspHandler[s16Idx].enSclHvspId)
        {
            case E_DRV_SCLHVSP_IO_ID_1:
                *penHvspId = E_MDRV_SCLHVSP_ID_1;
                break;
            case E_DRV_SCLHVSP_IO_ID_2:
                *penHvspId = E_MDRV_SCLHVSP_ID_2;
                break;
            case E_DRV_SCLHVSP_IO_ID_3:
                *penHvspId = E_MDRV_SCLHVSP_ID_3;
                break;
            case E_DRV_SCLHVSP_IO_ID_4:
                *penHvspId = E_MDRV_SCLHVSP_ID_4;
                break;
            default:
                *penHvspId = E_MDRV_SCLHVSP_ID_MAX;
                bRet = FALSE;
                break;
        }
    }
    return bRet;
}

DrvSclHvspIoCtxConfig_t *_DrvSclHvspIoGetCtxConfig(s32 s32Handler)
{
    s16 i;
    s16 s16Idx = -1;
    DrvSclHvspIoCtxConfig_t * pCtxCfg;

    DRV_SCLHVSP_IO_LOCK_MUTEX(_s32SclHvspIoHandleMutex);

    for(i = 0; i < DRV_SCLHVSP_HANDLER_MAX; i++)
    {
        if(_gstSclHvspHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        pCtxCfg = NULL;
    }
    else
    {
        pCtxCfg = &_gstSclHvspHandler[s16Idx].stCtxCfg;
    }

    DRV_SCLHVSP_IO_UNLOCK_MUTEX(_s32SclHvspIoHandleMutex);
    return pCtxCfg;
}

MDrvSclHvspIdType_e _DrvSclHvspGetHvspId(DrvSclHvspIoIdType_e enSclHvspId)
{
    MDrvSclHvspIdType_e enDev;
    switch(enSclHvspId)
    {
        case E_DRV_SCLHVSP_IO_ID_1:
            enDev = E_MDRV_SCLHVSP_ID_1;
            break;
        case E_DRV_SCLHVSP_IO_ID_2:
            enDev = E_MDRV_SCLHVSP_ID_2;
            break;
        case E_DRV_SCLHVSP_IO_ID_3:
            enDev = E_MDRV_SCLHVSP_ID_3;
            break;
        case E_DRV_SCLHVSP_IO_ID_4:
            enDev = E_MDRV_SCLHVSP_ID_4;
            break;
        default:
            enDev = E_MDRV_SCLHVSP_ID_MAX;
                break;
    }
    return enDev;
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool _DrvSclHvspIoDeInit(DrvSclHvspIoIdType_e enSclHvspId)
{
    DrvSclosProbeType_e enType;
    enType = (enSclHvspId==E_DRV_SCLHVSP_IO_ID_1) ? E_DRV_SCLOS_INIT_HVSP_1 :
            (enSclHvspId==E_DRV_SCLHVSP_IO_ID_2) ? E_DRV_SCLOS_INIT_HVSP_2 :
            (enSclHvspId==E_DRV_SCLHVSP_IO_ID_3) ?   E_DRV_SCLOS_INIT_HVSP_3 :
             (enSclHvspId==E_DRV_SCLHVSP_IO_ID_4) ?    E_DRV_SCLOS_INIT_HVSP_4:
                E_DRV_SCLOS_INIT_NONE;
    DrvSclOsClearProbeInformation(enType);
    if(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_ALL) == 0)
    {
        //Ctx DeInit
        MDrvSclHvspExit(1);
    }
    else if(!DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP))
    {
        MDrvSclHvspExit(0);
    }
    if(_s32SclHvspIoHandleMutex != -1)
    {
         DrvSclOsDeleteMutex(_s32SclHvspIoHandleMutex);
         _s32SclHvspIoHandleMutex = -1;
    }
    return TRUE;
}
bool _DrvSclHvspIoInit(DrvSclHvspIoIdType_e enSclHvspId)
{
    u16 i, start, end;
    MDrvSclHvspInitConfig_t stHVSPInitCfg;
    MDrvSclHvspIdType_e enSclMHvspId;
    MDrvSclCtxConfig_t *pvCfg;
    DrvSclOsMemset(&stHVSPInitCfg,0,sizeof(MDrvSclHvspInitConfig_t));
    if(enSclHvspId >= E_DRV_SCLHVSP_IO_ID_NUM)
    {
        SCL_ERR("%s %d, Id out of range %d\n", __FUNCTION__, __LINE__, enSclHvspId);
        return FALSE;
    }

    if(_s32SclHvspIoHandleMutex == -1)
    {
        _s32SclHvspIoHandleMutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, "SCLHVSP_IO", SCLOS_PROCESS_SHARED);
        if(_s32SclHvspIoHandleMutex == -1)
        {
            SCL_ERR("%s %d, Create Mutex Fail\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
    }
    enSclMHvspId = _DrvSclHvspGetHvspId(enSclHvspId);

    start = (u16)enSclHvspId * DRV_SCLHVSP_HANDLER_INSTANCE_NUM;
    end = start + DRV_SCLHVSP_HANDLER_INSTANCE_NUM;

    for(i = start; i < end; i++)
    {
        _gstSclHvspHandler[i].s32Handle = -1;
        _gstSclHvspHandler[i].enSclHvspId = E_DRV_SCLHVSP_IO_ID_NUM;

        _gstSclHvspHandler[i].stCtxCfg.s32Id = -1;
        _gstSclHvspHandler[i].stCtxCfg.enIdType = E_DRV_SCLHVSP_IO_ID_NUM;
        _gstSclHvspHandler[i].stCtxCfg.pLockCfg = NULL;
    }

    //Ctx Init
    if( MDrvSclCtxInit() == FALSE)
    {
        SCL_ERR("%s %d, Init Ctx\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    if(enSclHvspId == E_DRV_SCLHVSP_IO_ID_1)
    {
        DrvSclOsMemset(&_gstSclHvspIoFunc, 0, sizeof(DrvSclHvspIoFunctionConfig_t));
        _gstSclHvspIoFunc.DrvSclHvspIoSetInputConfig        = _DrvSclHvspIoSetInputConfig;
        _gstSclHvspIoFunc.DrvSclHvspIoSetScalingConfig      = _DrvSclHvspIoSetScalingConfig;
        _gstSclHvspIoFunc.DrvSclHvspIoSetMiscConfig         = _DrvSclHvspIoSetMiscConfig;
        _gstSclHvspIoFunc.DrvSclHvspIoSetVtrackConfig       = _DrvSclHvspIoSetVtrackConfig;
        _gstSclHvspIoFunc.DrvSclHvspIoSetVtrackOnOffConfig  = _DrvSclHvspIoSetVtrackOnOffConfig;
    }


    //ToDo Init
    stHVSPInitCfg.u32IRQNUM     = DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_SC0);
    pvCfg = MDrvSclCtxGetDefaultCtx();
    stHVSPInitCfg.pvCtx = (void *)&(pvCfg->stCtx);
    if( MDrvSclHvspInit(enSclMHvspId, &stHVSPInitCfg) == 0)
    {
        return -EFAULT;
    }
    return TRUE;
}

s32 _DrvSclHvspIoOpen(DrvSclHvspIoIdType_e enSclHvspId)
{
    s32 s32Handle = -1;
    s16 s16Idx = -1;
    s16 i ;
    MDrvSclCtxIdType_e enCtxId;
    enCtxId = E_MDRV_SCL_CTX_ID_SC_ALL;
    DRV_SCLHVSP_IO_LOCK_MUTEX(_s32SclHvspIoHandleMutex);
    for(i = 0; i < DRV_SCLHVSP_HANDLER_MAX; i++)
    {
        if(_gstSclHvspHandler[i].s32Handle == -1)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        s32Handle = -1;
        SCL_ERR( "[HVSP]   %s %d \n", __FUNCTION__, __LINE__);
    }
    else
    {
        s32Handle = s16Idx | DRV_SCLHVSP_HANDLER_PRE_FIX | (enSclHvspId<<(HANDLER_PRE_FIX_SHIFT));
        _gstSclHvspHandler[s16Idx].s32Handle = s32Handle ;
        _gstSclHvspHandler[s16Idx].enSclHvspId = enSclHvspId;
        _gstSclHvspHandler[s16Idx].stCtxCfg.enCtxId= enCtxId;
        _gstSclHvspHandler[s16Idx].stCtxCfg.s32Id = s32Handle;
        _gstSclHvspHandler[s16Idx].stCtxCfg.enIdType = enSclHvspId;
        _gstSclHvspHandler[s16Idx].stCtxCfg.pLockCfg = MDrvSclCtxGetLockConfig(enCtxId);
    }
    DRV_SCLHVSP_IO_UNLOCK_MUTEX(_s32SclHvspIoHandleMutex);
    return s32Handle;
}

DrvSclHvspIoErrType_e _DrvSclHvspIoRelease(s32 s32Handler)
{
    s16 s16Idx = -1;
    s16 i ;
    u16 u16loop = 0;
    DrvSclHvspIoErrType_e eRet = TRUE;
    MDrvSclHvspIdType_e enMdrvIdType;
    MDrvSclCtxCmdqConfig_t *pvCfg;
    DRV_SCLHVSP_IO_LOCK_MUTEX(_s32SclHvspIoHandleMutex);
    for(i = 0; i < DRV_SCLHVSP_HANDLER_MAX; i++)
    {
        if(_gstSclHvspHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        SCL_DBG(SCL_DBG_LV_IOCTL(),"[HVSP]   %s %d not support s32Handler:%lx\n", __FUNCTION__, __LINE__,s32Handler);
        eRet = E_DRV_SCLHVSP_IO_ERR_INVAL;
    }
    else
    {

        //ToDo Free Ctx

        _gstSclHvspHandler[s16Idx].s32Handle = -1;
        _gstSclHvspHandler[s16Idx].enSclHvspId = E_DRV_SCLHVSP_IO_ID_NUM;

        _gstSclHvspHandler[s16Idx].stCtxCfg.s32Id = -1;
        _gstSclHvspHandler[s16Idx].stCtxCfg.enIdType = E_DRV_SCLHVSP_IO_ID_NUM;
        _gstSclHvspHandler[s16Idx].stCtxCfg.pLockCfg = NULL;
        for(i = 0; i < DRV_SCLHVSP_HANDLER_MAX; i++)
        {
            if(_gstSclHvspHandler[i].s32Handle != -1)
            {
                u16loop = 1;
                break;
            }
        }
        if(!u16loop)
        {
            _DrvSclHvspIoGetMdrvIdType(s32Handler, &enMdrvIdType);
            MDrvSclCtxSetLockConfig(s32Handler,_gstSclHvspHandler[s16Idx].stCtxCfg.enCtxId);
            pvCfg = MDrvSclCtxGetConfigCtx(_gstSclHvspHandler[s16Idx].stCtxCfg.enCtxId);
            MDrvSclHvspRelease(enMdrvIdType,pvCfg);
            MDrvSclHvspReSetHw(pvCfg);
            MDrvSclCtxSetUnlockConfig(s32Handler,_gstSclHvspHandler[s16Idx].stCtxCfg.enCtxId);
        }
        eRet = E_DRV_SCLHVSP_IO_ERR_OK;
    }
    DRV_SCLHVSP_IO_UNLOCK_MUTEX(_s32SclHvspIoHandleMutex);
    return eRet;
}


DrvSclHvspIoErrType_e _DrvSclHvspIoSetInputConfig(s32 s32Handler, DrvSclHvspIoInputConfig_t *pstIoInCfg)
{
    DrvSclHvspIoErrType_e eRet = E_DRV_SCLHVSP_IO_ERR_FAULT;
    MDrvSclHvspInputConfig_t stInCfg;
    DrvSclHvspIoVersionChkConfig_t stVersion;
    DrvSclHvspIoCtxConfig_t *pstSclHvspCtxCfg;
    MDrvSclHvspIdType_e enMdrvIdType;
    MDrvSclCtxIdType_e enCtxId;

    _DrvSclHvspIoFillVersionChkStruct(sizeof(DrvSclHvspIoInputConfig_t),
                 pstIoInCfg->VerChk_Size,
                 &pstIoInCfg->VerChk_Version,&stVersion);
    DrvSclOsMemset(&stInCfg,0,sizeof(MDrvSclHvspInputConfig_t));
    if(_DrvSclHvspIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[HVSP]   %s %d \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_INVAL;
    }

    pstSclHvspCtxCfg = _DrvSclHvspIoGetCtxConfig(s32Handler);

    if(pstSclHvspCtxCfg == NULL)
    {
        SCL_ERR( "[HVSP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_INVAL;
    }

    if(_DrvSclHvspIoGetMdrvIdType(s32Handler, &enMdrvIdType) == FALSE)
    {
        SCL_ERR( "[HVSP]   %s %d \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_INVAL;
    }

    stInCfg.enColor       = pstIoInCfg->enColor;
    stInCfg.enSrcType     = pstIoInCfg->enSrcType;
    DrvSclOsMemcpy(&stInCfg.stCaptureWin, &pstIoInCfg->stCaptureWin, sizeof(MDrvSclHvspWindowConfig_t));
    DrvSclOsMemcpy(&stInCfg.stTimingCfg, &pstIoInCfg->stTimingCfg, sizeof(MDrvSclHvspTimingConfig_t));

    stInCfg.stclk = NULL;
    enCtxId = pstSclHvspCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    stInCfg.pvCtx = (void *)MDrvSclCtxGetConfigCtx(enCtxId);
    if(!MDrvSclHvspSetInputConfig(enMdrvIdType,  &stInCfg))
    {
        eRet = E_DRV_SCLHVSP_IO_ERR_FAULT;
    }
    else
    {
        eRet = E_DRV_SCLHVSP_IO_ERR_OK;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return eRet;
}

DrvSclHvspIoErrType_e _DrvSclHvspIoSetScalingConfig(s32 s32Handler, DrvSclHvspIoScalingConfig_t *pstIOSclCfg)
{
    MDrvSclHvspScalingConfig_t stSclCfg;
    DrvSclHvspIoErrType_e eRet = E_DRV_SCLHVSP_IO_ERR_OK;
    DrvSclHvspIoVersionChkConfig_t stVersion;
    DrvSclHvspIoCtxConfig_t *pstSclHvspCtxCfg;
    MDrvSclHvspIdType_e enMdrvIdType;
    MDrvSclCtxIdType_e enCtxId;

    _DrvSclHvspIoFillVersionChkStruct(sizeof(DrvSclHvspIoScalingConfig_t),
                pstIOSclCfg->VerChk_Size,
                &pstIOSclCfg->VerChk_Version,&stVersion);
    DrvSclOsMemset(&stSclCfg,0,sizeof(MDrvSclHvspScalingConfig_t));
    if(_DrvSclHvspIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[HVSP]   %s  \n", __FUNCTION__);
        return E_DRV_SCLHVSP_IO_ERR_INVAL;
    }
    pstSclHvspCtxCfg = _DrvSclHvspIoGetCtxConfig(s32Handler);

    if(pstSclHvspCtxCfg == NULL)
    {
        SCL_ERR( "[HVSP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_INVAL;
    }

    if(_DrvSclHvspIoGetMdrvIdType(s32Handler, &enMdrvIdType) == FALSE)
    {
        SCL_ERR( "[HVSP]   %s %d \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_INVAL;
    }

    stSclCfg.stclk = NULL;
    stSclCfg.stCropWin.bEn = pstIOSclCfg->bCropEn;
    stSclCfg.stCropWin.u16Height = pstIOSclCfg->stCropWin.u16Height;
    stSclCfg.stCropWin.u16Width = pstIOSclCfg->stCropWin.u16Width;
    stSclCfg.stCropWin.u16X = pstIOSclCfg->stCropWin.u16X;
    stSclCfg.stCropWin.u16Y = pstIOSclCfg->stCropWin.u16Y;
    stSclCfg.u16Dsp_Height = pstIOSclCfg->u16Dsp_Height;
    stSclCfg.u16Dsp_Width = pstIOSclCfg->u16Dsp_Width;
    stSclCfg.u16Src_Height = pstIOSclCfg->u16Src_Height;
    stSclCfg.u16Src_Width = pstIOSclCfg->u16Src_Width;
    enCtxId = pstSclHvspCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    stSclCfg.pvCtx = (void *)MDrvSclCtxGetConfigCtx(enCtxId);

    if(!MDrvSclHvspSetScalingConfig(enMdrvIdType,  &stSclCfg ))
    {
        eRet = E_DRV_SCLHVSP_IO_ERR_FAULT;
    }
    else
    {
        eRet = E_DRV_SCLHVSP_IO_ERR_OK;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return eRet;
}

DrvSclHvspIoErrType_e _DrvSclHvspIoSetMiscConfig(s32 s32Handler, DrvSclHvspIoMiscConfig_t *pstIOMiscCfg)
{
    MDrvSclHvspMiscConfig_t stMiscCfg;
    MDrvSclHvspIdType_e enMdrvIdType;
    DrvSclOsMemset(&stMiscCfg,0,sizeof(MDrvSclHvspMiscConfig_t));
    if(_DrvSclHvspIoGetMdrvIdType(s32Handler, &enMdrvIdType) == FALSE)
    {
        SCL_ERR( "[HVSP]   %s %d \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_INVAL;
    }
    if(enMdrvIdType != E_MDRV_SCLHVSP_ID_1)
    {
        SCL_ERR( "[HVSP]   %s %d \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_INVAL;
    }

    stMiscCfg.u8Cmd = pstIOMiscCfg->u8Cmd;
    stMiscCfg.u32Size = pstIOMiscCfg->u32Size;
    stMiscCfg.u32Addr = pstIOMiscCfg->u32Addr;

    if(MDrvSclHvspSetMiscConfig(&stMiscCfg))
    {
        return E_DRV_SCLHVSP_IO_ERR_OK;
    }
    else
    {
        return E_DRV_SCLHVSP_IO_ERR_FAULT;
    }
}
DrvSclHvspIoErrType_e _DrvSclHvspIoSetVtrackConfig(s32 s32Handler, DrvSclHvspIoVtrackConfig_t *pstCfg)
{
    DrvSclHvspIoVersionChkConfig_t stVersion;
    DrvSclHvspIoCtxConfig_t *pstSclHvspCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclHvspIoFillVersionChkStruct(sizeof(DrvSclHvspIoVtrackConfig_t),
                                               (pstCfg->VerChk_Size),
                                               &(pstCfg->VerChk_Version),&stVersion);

    if(_DrvSclHvspIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_FAULT;
    }
    pstSclHvspCtxCfg = _DrvSclHvspIoGetCtxConfig(s32Handler);
    if(pstSclHvspCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_FAULT;
    }
    enCtxId = pstSclHvspCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    if(pstCfg->bSetKey)
    {
        MDrvSclHvspVtrackSetPayloadData(pstCfg->u16Timecode, pstCfg->u8OperatorID);
        MDrvSclHvspVtrackSetKey(pstCfg->bSetKey, pstCfg->u8SetKey);
    }
    else
    {
        MDrvSclHvspVtrackSetPayloadData(pstCfg->u16Timecode, pstCfg->u8OperatorID);
        MDrvSclHvspVtrackSetKey(0,NULL);
    }
    if(pstCfg->bSetUserDef)
    {
        MDrvSclHvspVtrackSetUserDefindedSetting(pstCfg->bSetUserDef, pstCfg->u8SetUserDef);
    }
    else
    {
        MDrvSclHvspVtrackSetUserDefindedSetting(0,NULL);
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLHVSP_IO_ERR_OK;
}


DrvSclHvspIoErrType_e _DrvSclHvspIoSetVtrackOnOffConfig(s32 s32Handler, DrvSclHvspIoVtrackOnOffConfig_t *pstCfg)
{
    DrvSclHvspIoVersionChkConfig_t stVersion;
    DrvSclHvspIoCtxConfig_t *pstSclHvspCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclHvspIoFillVersionChkStruct(sizeof(DrvSclHvspIoVtrackOnOffConfig_t),
                                               (pstCfg->VerChk_Size),
                                               &(pstCfg->VerChk_Version),&stVersion);

    if(_DrvSclHvspIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_FAULT;
    }
    pstSclHvspCtxCfg = _DrvSclHvspIoGetCtxConfig(s32Handler);
    if(pstSclHvspCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLHVSP_IO_ERR_FAULT;
    }
    enCtxId = pstSclHvspCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    if(!MDrvSclHvspVtrackEnable(pstCfg->u8framerate,  pstCfg->EnType))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLHVSP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLHVSP_IO_ERR_OK;
}
