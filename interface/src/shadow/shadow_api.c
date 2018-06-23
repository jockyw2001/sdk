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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   shadow_api.c
/// @brief shadow module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __KERNEL__
#include <pthread.h>
#include <sys/prctl.h>
#endif
#include "mi_syscall.h"
#include "mi_print.h"

#include "mi_shadow.h"
#include "shadow_ioctl.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
MI_MODULE_DEFINE(shadow)

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct MI_SHADOW_CallbackThreadInfo_s
{
    MI_SHADOW_HANDLE hShadow;
    MI_SHADOW_Callback pafCallback[MI_SHADOW_CALLBACK_EVENT_EXIT];
    void *pUsrData;
#ifndef __KERNEL__
    pthread_t pthrCb;
#endif
    MI_BOOL bRun;
} MI_SHADOW_CallbackThreadInfo_t;

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------

static MI_SHADOW_CallbackThreadInfo_t _stCallbackInfo;
//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
MI_S32 _MI_SHADOW_SetCallbackResult(MI_SHADOW_HANDLE hShadow, MI_S32 s32Result)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_SetCallbackResult_t stSetCallbackResult;
    stSetCallbackResult.hShadow = hShadow;
    stSetCallbackResult.s32Result = s32Result;
    s32Ret = MI_SYSCALL(MI_SHADOW_SET_CALLBACK_RESULT, &stSetCallbackResult);
    return s32Ret;
}

#ifndef __KERNEL__
static void *_MI_SHADOW_WaitCallbackThread(void *p)
{
    MI_SHADOW_CallbackInfo_t stCallbackInfo;
    prctl(PR_SET_NAME,"MI_SHADOW_WaitCallbackThread");
    MI_SHADOW_Callback pfCallback = NULL;
    while (_stCallbackInfo.bRun)
    {
        ///call wait callback funtion
        memset(&stCallbackInfo, 0x0, sizeof(MI_SHADOW_CallbackInfo_t));
        stCallbackInfo.s32TimeOutMs = 500;
        stCallbackInfo.hShadow = _stCallbackInfo.hShadow;
        MI_SYSCALL(MI_SHADOW_WAIT_CALLBACK, &stCallbackInfo);
        if (MI_SHADOW_CALLBACK_EVENT_EXIT == stCallbackInfo.eCallbackEvent)
        {
            ///exit
            DBG_INFO("Shadow Callback Exit\n");
            break;
        }
        else if (MI_SHADOW_CALLBACK_EVENT_TIMEOUT == stCallbackInfo.eCallbackEvent)
        {
            continue;
        }

        ///get callback status
        DBG_INFO("Shadow Callback(%d)\n", stCallbackInfo.eCallbackEvent);
        pfCallback = _stCallbackInfo.pafCallback[stCallbackInfo.eCallbackEvent];
        if (pfCallback)
        {
            MI_S32 s32Result = E_MI_ERR_FAILED;
            s32Result = pfCallback(
                &stCallbackInfo.stChnCurryPort,
                &stCallbackInfo.stChnPeerPort,
                _stCallbackInfo.pUsrData);
            _MI_SHADOW_SetCallbackResult(stCallbackInfo.hShadow, s32Result);
        }
    }
    return NULL;
}
#endif

static void _MI_SHADOW_CreateWaitCallbackThead(
    MI_SHADOW_HANDLE hShadow, MI_SHADOW_RegisterDevParams_t *pstDevParams)
{
    _stCallbackInfo.pafCallback[MI_SHADOW_CALLBACK_EVENT_ONBINDINPUTPORT] = pstDevParams->OnBindInputPort;
    _stCallbackInfo.pafCallback[MI_SHADOW_CALLBACK_EVENT_ONBINDOUTPUTPORT] = pstDevParams->OnBindOutputPort;
    _stCallbackInfo.pafCallback[MI_SHADOW_CALLBACK_EVENT_ONUNBINDINPUTPORT] = pstDevParams->OnUnBindInputPort;
    _stCallbackInfo.pafCallback[MI_SHADOW_CALLBACK_EVENT_ONUNBINDOUTPUTPORT] = pstDevParams->OnUnBindOutputPort;
    _stCallbackInfo.pUsrData = pstDevParams->pUsrData;
    if (_stCallbackInfo.pafCallback[MI_SHADOW_CALLBACK_EVENT_ONBINDINPUTPORT]
        || _stCallbackInfo.pafCallback[MI_SHADOW_CALLBACK_EVENT_ONBINDOUTPUTPORT]
        || _stCallbackInfo.pafCallback[MI_SHADOW_CALLBACK_EVENT_ONUNBINDINPUTPORT]
        || _stCallbackInfo.pafCallback[MI_SHADOW_CALLBACK_EVENT_ONUNBINDOUTPUTPORT])
    {
        _stCallbackInfo.bRun = 1;
        _stCallbackInfo.hShadow = hShadow;
#ifndef __KERNEL__
        pthread_create(&(_stCallbackInfo.pthrCb), NULL, _MI_SHADOW_WaitCallbackThread, NULL);
#endif
    }
}

MI_S32 MI_SHADOW_RegisterDev(MI_SHADOW_RegisterDevParams_t *pstDevParams, MI_SHADOW_HANDLE *phShadow)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_RegisterDev_t stRegisterDev;

    if (pstDevParams && phShadow)
    {
        memset(&stRegisterDev, 0x0, sizeof(MI_SHADOW_RegisterDev_t));
        memcpy(&(stRegisterDev.stModDevInfo), &(pstDevParams->stModDevInfo), sizeof(MI_SHADOW_ModuleDevInfo_t));
        s32Ret = MI_SYSCALL(MI_SHADOW_REGISTER_DEV, &stRegisterDev);
        if (MI_SUCCESS == s32Ret)
        {
            *phShadow = stRegisterDev.hShadow;
            _MI_SHADOW_CreateWaitCallbackThead(stRegisterDev.hShadow, pstDevParams);
        }
        else
        {
            *phShadow = 0x12345687;
        }
    }
    return s32Ret;
}

MI_S32 MI_SHADOW_UnRegisterDev(MI_SHADOW_HANDLE hShadow)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_UnRegisterDev_t stUnRegDev;
    stUnRegDev.hShadow = hShadow;
    _stCallbackInfo.bRun = FALSE;
    s32Ret = MI_SYSCALL(MI_SHADOW_UNREGISTER_DEV, &stUnRegDev);
#ifndef __KERNEL__
    pthread_join(_stCallbackInfo.pthrCb, NULL);
#endif
    return s32Ret;
}

MI_S32 MI_SHADOW_GetOutputPortBuf(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_SYS_BufConf_t *pstBufConfig,
    MI_BOOL *pbBlockedByRateCtrl,
    MI_SYS_BufInfo_t *pstBufInfo,
    MI_SYS_BUF_HANDLE *phBufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_GetOutputPortBuf_t stGetOutputPortBuf;

    if (pstBufConfig && pstBufInfo)
    {
        memset(&stGetOutputPortBuf, 0x0, sizeof(MI_SHADOW_GetOutputPortBuf_t));
        stGetOutputPortBuf.stGetPortBuf.hShadow = hShadow;
        stGetOutputPortBuf.stGetPortBuf.u32ChnId = u32ChnId;
        stGetOutputPortBuf.stGetPortBuf.u32PortId = u32PortId;
        memcpy(&stGetOutputPortBuf.stBufCfg, pstBufConfig, sizeof(MI_SYS_BufConf_t));
        s32Ret = MI_SYSCALL(MI_SHADOW_GET_OUTPUTPORT_BUFFER, &stGetOutputPortBuf);
        if (MI_SUCCESS == s32Ret)
        {
            memcpy(pstBufInfo, &stGetOutputPortBuf.stGetPortBuf.stBufInfo, sizeof(MI_SYS_BufInfo_t));
            *phBufHandle = stGetOutputPortBuf.stGetPortBuf.hBufHandle;
        }
        else
        {
            memset(pstBufInfo, 0x1A, sizeof(MI_SYS_BufInfo_t));
        }
    }

    if (pbBlockedByRateCtrl)
    {
        *pbBlockedByRateCtrl = stGetOutputPortBuf.bBlockedByRateCtrl;
    }
    return s32Ret;
}

MI_S32 MI_SHADOW_GetInputPortBuf(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_SYS_BufInfo_t *pstBufInfo,
    MI_SYS_BUF_HANDLE *phBufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_GetPortBuf_t stGetPortBuf;

    if (pstBufInfo)
    {
        memset(&stGetPortBuf, 0x0, sizeof(MI_SHADOW_GetPortBuf_t));
        stGetPortBuf.hShadow = hShadow;
        stGetPortBuf.u32ChnId = u32ChnId;
        stGetPortBuf.u32PortId = u32PortId;
        s32Ret = MI_SYSCALL(MI_SHADOW_GET_INPUTPORT_BUFFER, &stGetPortBuf);
        if (MI_SUCCESS == s32Ret)
        {
            memcpy(pstBufInfo, &stGetPortBuf.stBufInfo, sizeof(MI_SYS_BufInfo_t));
            *phBufHandle = stGetPortBuf.hBufHandle;
        }
        else
        {
            memset(pstBufInfo, 0xF1, sizeof(MI_SYS_BufInfo_t));
        }
    }
    return s32Ret;
}


MI_S32 MI_SHADOW_FinishBuf(MI_SHADOW_HANDLE hShadow, MI_SYS_BUF_HANDLE hBufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_FinishBuf_t stFinishBuf;
    memset(&stFinishBuf, 0x0, sizeof(MI_SHADOW_FinishBuf_t));
    stFinishBuf.hBufHandle = hBufHandle;
    stFinishBuf.hShadow = hShadow;
    s32Ret = MI_SYSCALL(MI_SHADOW_FINISH_BUFFER, &stFinishBuf);
    return s32Ret;
}

MI_S32 MI_SHADOW_RewindBuf(MI_SHADOW_HANDLE hShadow, MI_SYS_BUF_HANDLE hBufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_FinishBuf_t stRewindBuf;
    memset(&stRewindBuf, 0x0, sizeof(MI_SHADOW_FinishBuf_t));
    stRewindBuf.hBufHandle = hBufHandle;
    stRewindBuf.hShadow = hShadow;
    s32Ret = MI_SYSCALL(MI_SHADOW_REWIND_BUFFER, &stRewindBuf);
    return s32Ret;
}

MI_S32 MI_SHADOW_WaitOnInputTaskAvailable(MI_SHADOW_HANDLE hShadow ,MI_S32 u32TimeOutMs)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_WaitOnInputTaskAvailable_t stWaitBufAvailable;
    memset(&stWaitBufAvailable, 0x0, sizeof(MI_SHADOW_WaitOnInputTaskAvailable_t));
    stWaitBufAvailable.hShadow = hShadow;
    stWaitBufAvailable.u32TimeOutMs = u32TimeOutMs;
    s32Ret = MI_SYSCALL(MI_SHADOW_WAIT_BUFFER_AVAIlLABLE, &stWaitBufAvailable);
    return s32Ret;
}

MI_S32 MI_SHADOW_EnableChannel(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_SetChannelStatus_t stChnStatus;
    memset(&stChnStatus, 0x0, sizeof(MI_SHADOW_SetChannelStatus_t));
    stChnStatus.hShadow = hShadow;
    stChnStatus.bChn = TRUE;
    stChnStatus.u32ChnId = u32ChnId;
    stChnStatus.bEnable = TRUE;
    s32Ret = MI_SYSCALL(MI_SHADOW_SET_CHANNEL_STATUS, &stChnStatus);
    return s32Ret;
}

MI_S32 MI_SHADOW_DisableChannel(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_SetChannelStatus_t stChnStatus;
    memset(&stChnStatus, 0x0, sizeof(MI_SHADOW_SetChannelStatus_t));
    stChnStatus.hShadow = hShadow;
    stChnStatus.bChn = TRUE;
    stChnStatus.u32ChnId = u32ChnId;
    stChnStatus.bEnable = FALSE;
    s32Ret = MI_SYSCALL(MI_SHADOW_SET_CHANNEL_STATUS, &stChnStatus);
    return s32Ret;
}

MI_S32 MI_SHADOW_EnableOutputPort(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId, MI_U32 u32PortId)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_SetChannelStatus_t stChnStatus;
    memset(&stChnStatus, 0x0, sizeof(MI_SHADOW_SetChannelStatus_t));
    stChnStatus.hShadow = hShadow;
    stChnStatus.bChn = FALSE;
    stChnStatus.u32ChnId = u32ChnId;
    stChnStatus.u32PortId = u32PortId;
    stChnStatus.bInputPort = FALSE;
    stChnStatus.bEnable = TRUE;
    s32Ret = MI_SYSCALL(MI_SHADOW_SET_CHANNEL_STATUS, &stChnStatus);
    return s32Ret;
}

MI_S32 MI_SHADOW_DisableOutputPort(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId, MI_U32 u32PortId)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_SetChannelStatus_t stChnStatus;
    memset(&stChnStatus, 0x0, sizeof(MI_SHADOW_SetChannelStatus_t));
    stChnStatus.hShadow = hShadow;
    stChnStatus.bChn = FALSE;
    stChnStatus.u32ChnId = u32ChnId;
    stChnStatus.u32PortId = u32PortId;
    stChnStatus.bInputPort = FALSE;
    stChnStatus.bEnable = FALSE;
    s32Ret = MI_SYSCALL(MI_SHADOW_SET_CHANNEL_STATUS, &stChnStatus);
    return s32Ret;
}

MI_S32 MI_SHADOW_EnableInputPort(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId, MI_U32 u32PortId)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_SetChannelStatus_t stChnStatus;
    memset(&stChnStatus, 0x0, sizeof(MI_SHADOW_SetChannelStatus_t));
    stChnStatus.hShadow = hShadow;
    stChnStatus.bChn = FALSE;
    stChnStatus.u32ChnId = u32ChnId;
    stChnStatus.u32PortId = u32PortId;
    stChnStatus.bInputPort = TRUE;
    stChnStatus.bEnable = TRUE;
    s32Ret = MI_SYSCALL(MI_SHADOW_SET_CHANNEL_STATUS, &stChnStatus);
    return s32Ret;
}

MI_S32 MI_SHADOW_DisableInputPort(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId, MI_U32 u32PortId)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_SetChannelStatus_t stChnStatus;
    memset(&stChnStatus, 0x0, sizeof(MI_SHADOW_SetChannelStatus_t));
    stChnStatus.hShadow = hShadow;
    stChnStatus.bChn = FALSE;
    stChnStatus.u32ChnId = u32ChnId;
    stChnStatus.u32PortId = u32PortId;
    stChnStatus.bInputPort = TRUE;
    stChnStatus.bEnable = FALSE;
    s32Ret = MI_SYSCALL(MI_SHADOW_SET_CHANNEL_STATUS, &stChnStatus);
    return s32Ret;
}

MI_S32 MI_SHADOW_SetInputPortBufExtConf(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId , MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_SetPortExtraConf_t stSetPortExtraConf;

    if (pstBufExtraConf)
    {
        memset(&stSetPortExtraConf, 0x0, sizeof(MI_SHADOW_SetPortExtraConf_t));
        stSetPortExtraConf.hShadow = hShadow;
        stSetPortExtraConf.u32ChnId = u32ChnId;
        stSetPortExtraConf.u32PortId = u32PortId;
        memcpy(&(stSetPortExtraConf.stBufExtraConf), pstBufExtraConf, sizeof(MI_SYS_FrameBufExtraConfig_t));
        s32Ret = MI_SYSCALL(MI_SHADOW_SET_INPUTPORT_BUFFER_EXTCONF, &stSetPortExtraConf);
    }

    return s32Ret;
}

MI_S32 MI_SHADOW_SetOutputPortBufExtConf(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId , MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_SetPortExtraConf_t stSetPortExtraConf;

    if (pstBufExtraConf)
    {
        memset(&stSetPortExtraConf, 0x0, sizeof(MI_SHADOW_SetPortExtraConf_t));
        stSetPortExtraConf.hShadow = hShadow;
        stSetPortExtraConf.u32ChnId = u32ChnId;
        stSetPortExtraConf.u32PortId = u32PortId;
        memcpy(&(stSetPortExtraConf.stBufExtraConf), pstBufExtraConf, sizeof(MI_SYS_FrameBufExtraConfig_t));
        s32Ret = MI_SYSCALL(MI_SHADOW_SET_OUTPUTPORT_BUFFER_EXTCONF, &stSetPortExtraConf);
    }

    return s32Ret;
}

EXPORT_SYMBOL(MI_SHADOW_RegisterDev);
EXPORT_SYMBOL(MI_SHADOW_UnRegisterDev);
EXPORT_SYMBOL(MI_SHADOW_GetOutputPortBuf);
EXPORT_SYMBOL(MI_SHADOW_GetInputPortBuf);
EXPORT_SYMBOL(MI_SHADOW_FinishBuf);
EXPORT_SYMBOL(MI_SHADOW_RewindBuf);
EXPORT_SYMBOL(MI_SHADOW_WaitOnInputTaskAvailable);
EXPORT_SYMBOL(MI_SHADOW_EnableChannel);
EXPORT_SYMBOL(MI_SHADOW_DisableChannel);
EXPORT_SYMBOL(MI_SHADOW_EnableOutputPort);
EXPORT_SYMBOL(MI_SHADOW_DisableOutputPort);
EXPORT_SYMBOL(MI_SHADOW_EnableInputPort);
EXPORT_SYMBOL(MI_SHADOW_DisableInputPort);
EXPORT_SYMBOL(MI_SHADOW_SetInputPortBufExtConf);
EXPORT_SYMBOL(MI_SHADOW_SetOutputPortBufExtConf);
