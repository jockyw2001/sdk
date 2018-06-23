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
/// @file   shadow_ioctl.c
/// @brief shadow module ioctl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/sched.h>
#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_shadow_impl.h"
#include "shadow_ioctl.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
static DEFINE_MUTEX(mutex);


//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

MI_S32 MI_SHADOW_IOCTL_RegisterDev(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_RegisterDev_t *arg = ptr;
    MI_SHADOW_ModuleDevInfo_t *pstModDevInfo = &(arg->stModDevInfo);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SHADOW_IMPL_RegisterDev(pstModDevInfo, (MI_SHADOW_HANDLE *)(&(arg->hShadow)));
    return s32Ret;
}

MI_S32 MI_SHADOW_IOCTL_UnRegisterDev(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_UnRegisterDev_t *pstUnRegDevInfo = (MI_SHADOW_UnRegisterDev_t *)ptr;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SHADOW_IMPL_UnRegisterDev(pstUnRegDevInfo->hShadow);
    return s32Ret;
}

MI_S32 MI_SHADOW_IOCTL_GetOutputPortBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_GetOutputPortBuf_t *arg = ptr;
    MI_SHADOW_GetPortBuf_t *pstGetPortBuf = &(arg->stGetPortBuf);
    MI_SYS_BufConf_t *pstBufCfg = &(arg->stBufCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SHADOW_IMPL_GetOutputPortBuf(
        pstGetPortBuf->hShadow,
        pstGetPortBuf->u32ChnId,
        pstGetPortBuf->u32PortId,
        pstBufCfg,

        &(pstGetPortBuf->stBufInfo),
        &(arg->bBlockedByRateCtrl),
        &(arg->stGetPortBuf.hBufHandle));
    return s32Ret;
}

MI_S32 MI_SHADOW_IOCTL_GetInputPortBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_GetPortBuf_t *pstGetPortBuf = ptr;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SHADOW_IMPL_GetInputPortBuf(
        pstGetPortBuf->hShadow,
        pstGetPortBuf->u32ChnId,
        pstGetPortBuf->u32PortId,
        &(pstGetPortBuf->stBufInfo),
        &(pstGetPortBuf->hBufHandle));
    return s32Ret;
}

MI_S32 MI_SHADOW_IOCTL_FinishBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_FinishBuf_t *pstFinishBuf = ptr;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SHADOW_IMPL_FinishBuf(pstFinishBuf->hShadow, pstFinishBuf->hBufHandle);
    return s32Ret;
}

MI_S32 MI_SHADOW_IOCTL_RewindBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_FinishBuf_t *pstRewindBuf = ptr;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SHADOW_IMPL_RewindBuf(pstRewindBuf->hShadow, pstRewindBuf->hBufHandle);
    return s32Ret;
}

MI_S32 MI_SHADOW_IOCTL_WaitOnInputTaskAvailable(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_WaitOnInputTaskAvailable_t *pstWaitOnInputTaskAvailable = ptr;

    s32Ret = MI_SHADOW_IMPL_WaitOnInputTaskAvailable(
        pstWaitOnInputTaskAvailable->hShadow,
        pstWaitOnInputTaskAvailable->u32TimeOutMs);
    return s32Ret;
}

MI_S32 MI_SHADOW_IOCTL_WaitCallback(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_CallbackInfo_t *pstCallbackInfo = ptr;
    s32Ret = MI_SHADOW_IMPL_WaitCallBack(
        pstCallbackInfo->hShadow,
        pstCallbackInfo->s32TimeOutMs,
        &(pstCallbackInfo->stChnCurryPort),
        &(pstCallbackInfo->stChnPeerPort),
        &(pstCallbackInfo->eCallbackEvent));
    return s32Ret;
}

MI_S32 MI_SHADOW_IOCTL_SetCallbackResult(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_SHADOW_SetCallbackResult_t *pstSetCallbackResult = ptr;
    MI_SHADOW_IMPL_SetCallbackResult(
        pstSetCallbackResult->hShadow,
        pstSetCallbackResult->s32Result);
    return MI_SUCCESS;
}

MI_S32 MI_SHADOW_IOCTL_SetChannelStatus(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_SHADOW_SetChannelStatus_t *pstSetChnStatus = ptr;
    return MI_SHADOW_IMPL_SetChannelStatus(
        pstSetChnStatus->hShadow,
        pstSetChnStatus->u32ChnId,
        pstSetChnStatus->u32PortId,
        pstSetChnStatus->bEnable,
        pstSetChnStatus->bChn,
        pstSetChnStatus->bInputPort);
}

MI_S32 MI_SHADOW_IOCTL_SetInputPortBufExtConf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_SHADOW_SetPortExtraConf_t *pstSetPortExtraConf = ptr;

    return MI_SHADOW_IMPL_SetInputPortBufExtConf(
        pstSetPortExtraConf->hShadow,
        pstSetPortExtraConf->u32ChnId,
        pstSetPortExtraConf->u32PortId,
        &(pstSetPortExtraConf->stBufExtraConf));
}

MI_S32 MI_SHADOW_IOCTL_SetOutputPortBufExtConf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_SHADOW_SetPortExtraConf_t *pstSetPortExtraConf = ptr;

    return MI_SHADOW_IMPL_SetOutputPortBufExtConf(
        pstSetPortExtraConf->hShadow,
        pstSetPortExtraConf->u32ChnId,
        pstSetPortExtraConf->u32PortId,
        &(pstSetPortExtraConf->stBufExtraConf));
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_SHADOW_CMD_MAX] = {
    [E_MI_SHADOW_CMD_REGISTER_DEV] = MI_SHADOW_IOCTL_RegisterDev,
    [E_MI_SHADOW_CMD_UNREGISTER_DEV] = MI_SHADOW_IOCTL_UnRegisterDev,
    [E_MI_SHADOW_CMD_GET_OUTPUTPORT_BUFFER] = MI_SHADOW_IOCTL_GetOutputPortBuf,
    [E_MI_SHADOW_CMD_GET_INPUTPORT_BUFFER] = MI_SHADOW_IOCTL_GetInputPortBuf,
    [E_MI_SHADOW_CMD_FINISH_BUFFER] = MI_SHADOW_IOCTL_FinishBuf,
    [E_MI_SHADOW_CMD_REWIND_BUFFER] = MI_SHADOW_IOCTL_RewindBuf,
    [E_MI_SHADOW_CMD_WAIT_BUFFER_AVAIlLABLE] = MI_SHADOW_IOCTL_WaitOnInputTaskAvailable,
    [E_MI_SHADOW_CMD_WAIT_CALLBACK] = MI_SHADOW_IOCTL_WaitCallback,
    [E_MI_SHADOW_CMD_SET_CALLBACK_RESULT] = MI_SHADOW_IOCTL_SetCallbackResult,
    [E_MI_SHADOW_CMD_SET_CHANNEL_STATUS] = MI_SHADOW_IOCTL_SetChannelStatus,
    [E_MI_SHADOW_CMD_SET_INPUTPORT_BUFFER_EXTCONF] = MI_SHADOW_IOCTL_SetInputPortBufExtConf,
    [E_MI_SHADOW_CMD_SET_OUTPUTPORT_BUFFER_EXTCONF] = MI_SHADOW_IOCTL_SetOutputPortBufExtConf,
};

static void mi_shadow_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
    MI_SHADOW_IMPL_Init();
}

static void mi_shadow_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
}

static void mi_shadow_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("colin.hu <colin.hu@mstarsemi.com>");

MI_DEVICE_DEFINE(mi_shadow_insmod,
               ioctl_table, E_MI_SHADOW_CMD_MAX,
               mi_shadow_process_init, mi_shadow_process_exit);
