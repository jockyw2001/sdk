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
/// @file   vdec_ioctl.c
/// @brief vdec module ioctl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/semaphore.h>

#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_sys.h"
#include "mi_sys_internal.h"
#include "mi_vdisp.h"
#include "sub_buf_allocator.h"
#include "mi_vdisp_impl.h"
#include "vdisp_ioctl.h"

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

static MI_S32 MI_VDISP_IOCTL_Init(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> \n", __func__);
    s32Ret = MI_VDISP_IMPL_Init();

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_Exit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s>\n", __func__);
    s32Ret = MI_VDISP_IMPL_Exit();

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_OpenDevice(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_DEV DevId = *(MI_VDISP_DEV*)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> DevId: %d.\n", __func__, DevId);
    s32Ret = MI_VDISP_IMPL_OpenDevice(DevId);

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_CloseDevice(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_DEV DevId = *(MI_VDISP_DEV*)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> DevId: %d.\n", __func__, DevId);
    s32Ret = MI_VDISP_IMPL_CloseDevice(DevId);

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_SetOutputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_SetOutputPortAttr_t *pstSetOutputPortAttr = (MI_VDISP_SetOutputPortAttr_t *)ptr;
    MI_VDISP_DEV DevId=pstSetOutputPortAttr->DevId;
    MI_VDISP_PORT PortId=pstSetOutputPortAttr->PortId;
    MI_VDISP_OutputPortAttr_t *pstOutputPortAttr=&pstSetOutputPortAttr->stOutputPortAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s>\n", __func__);
    s32Ret = MI_VDISP_IMPL_SetOutputPortAttr(DevId,PortId,pstOutputPortAttr);

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_GetOutputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_GetOutputPortAttr_t *pstGetOutputPortAttr = (MI_VDISP_GetOutputPortAttr_t *)ptr;
    MI_VDISP_DEV DevId=pstGetOutputPortAttr->DevId;
    MI_VDISP_PORT PortId=pstGetOutputPortAttr->PortId;
    MI_VDISP_OutputPortAttr_t *pstOutputPortAttr=&pstGetOutputPortAttr->stOutputPortAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s>\n", __func__);
    s32Ret = MI_VDISP_IMPL_GetOutputPortAttr(DevId,PortId,pstOutputPortAttr);

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_SetInputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_SetInputPortAttr_t *pstSetInputPortAttr = (MI_VDISP_SetInputPortAttr_t *)ptr;
    MI_VDISP_DEV DevId=pstSetInputPortAttr->DevId;
    MI_VDISP_PORT PortId=pstSetInputPortAttr->PortId;
    MI_VDISP_InputPortAttr_t *pstInputPortAttr=&pstSetInputPortAttr->stInputPortAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> .\n", __func__);
    s32Ret = MI_VDISP_IMPL_SetInputPortAttr(DevId,PortId,pstInputPortAttr);

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_GetInputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_GetInputPortAttr_t *pstGetInputPortAttr = (MI_VDISP_GetInputPortAttr_t *)ptr;
    MI_VDISP_DEV DevId=pstGetInputPortAttr->DevId;
    MI_VDISP_PORT PortId=pstGetInputPortAttr->PortId;
    MI_VDISP_InputPortAttr_t *pstInputPortAttr=&pstGetInputPortAttr->stInputPortAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> .\n", __func__);
    s32Ret = MI_VDISP_IMPL_GetInputPortAttr(DevId,PortId,pstInputPortAttr);

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_EnableInputPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_EnableInputPort_t *pstEnableInputPort = (MI_VDISP_EnableInputPort_t *)ptr;
    MI_VDISP_DEV DevId=pstEnableInputPort->DevId;
    MI_VDISP_PORT PortId=pstEnableInputPort->PortId;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Dev: %d, Port: %d.\n", __func__, DevId, PortId);
    s32Ret = MI_VDISP_IMPL_EnableInputPort(DevId,PortId);

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_DisableInputPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_DisableInputPort_t *pstDisableInputPort = (MI_VDISP_DisableInputPort_t *)ptr;
    MI_VDISP_DEV DevId=pstDisableInputPort->DevId;
    MI_VDISP_PORT PortId=pstDisableInputPort->PortId;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Dev: %d, Port: %d.\n", __func__, DevId, PortId);
    s32Ret = MI_VDISP_IMPL_DisableInputPort(DevId,PortId);

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_StartDev(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_DEV *pstStartDev = (MI_VDISP_DEV *)ptr;
    MI_VDISP_DEV DevId=*pstStartDev;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Dev: %d.\n", __func__, DevId);
    s32Ret = MI_VDISP_IMPL_StartDev(DevId);

    return s32Ret;
}

static MI_S32 MI_VDISP_IOCTL_StopDev(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret;
    MI_VDISP_DEV *pstStartDev = (MI_VDISP_DEV *)ptr;
    MI_VDISP_DEV DevId=*pstStartDev;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Dev: %d.\n", __func__, DevId);
    s32Ret = MI_VDISP_IMPL_StopDev(DevId);

    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_VDISP_CMD_MAX] = {
    [E_MI_VDISP_CMD_INIT] = MI_VDISP_IOCTL_Init,
    [E_MI_VDISP_CMD_EXIT] = MI_VDISP_IOCTL_Exit,
    [E_MI_VDISP_CMD_OPEN_DEVICE] = MI_VDISP_IOCTL_OpenDevice,
    [E_MI_VDISP_CMD_CLOSE_DEVICE] = MI_VDISP_IOCTL_CloseDevice,
    [E_MI_VDISP_CMD_SET_OUTPUT_PORT_ATTR] = MI_VDISP_IOCTL_SetOutputPortAttr,
    [E_MI_VDISP_CMD_GET_OUTPUT_PORT_ATTR] = MI_VDISP_IOCTL_GetOutputPortAttr,
    [E_MI_VDISP_CMD_SET_INPUT_PORT_ATTR] = MI_VDISP_IOCTL_SetInputPortAttr,
    [E_MI_VDISP_CMD_GET_INPUT_PORT_ATTR] = MI_VDISP_IOCTL_GetInputPortAttr,
    [E_MI_VDISP_CMD_ENABLE_INPUT_PORT] = MI_VDISP_IOCTL_EnableInputPort,
    [E_MI_VDISP_CMD_DISABLE_INPUT_PORT] = MI_VDISP_IOCTL_DisableInputPort,
    [E_MI_VDISP_CMD_START_DEV] = MI_VDISP_IOCTL_StartDev,
    [E_MI_VDISP_CMD_STOP_DEV] = MI_VDISP_IOCTL_StopDev,
};


static void mi_vdisp_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
}
static void mi_vdisp_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
}
static void mi_vdisp_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");

MI_DEVICE_DEFINE(mi_vdisp_insmod,
               ioctl_table, E_MI_VDISP_CMD_MAX,
               mi_vdisp_process_init, mi_vdisp_process_exit);
