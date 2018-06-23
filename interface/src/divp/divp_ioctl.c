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
/// @file   DIVP_ioctl.c
/// @brief DIVP module ioctl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_divp_datatype.h"
#include "mi_divp_datatype_internal.h"
#include "mi_divp_impl.h"
#include "divp_ioctl.h"


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
static MI_S32 MI_DIVP_IOCTL_CreateChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DIVP_ChnAttrParams_t *arg = ptr;
    MI_DIVP_CHN DivpChn = arg->DivpChn;
    MI_DIVP_ChnAttr_t *pstChnAttr = &(arg->stChnAttr);

    AUTO_LOCK(env, mutex);
    DBG_INFO(" module name: %s, eFrom:%d\n", env->eFrom, env->strModuleName);
    s32Ret = MI_IMPL_DIVP_CreateChn(DivpChn, pstChnAttr);
    return s32Ret;
}

static MI_S32 MI_DIVP_IOCTL_DestroyChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DIVP_CHN DivpChn = *((MI_DIVP_CHN *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_IMPL_DIVP_DestroyChn(DivpChn);
    return s32Ret;
}

static MI_S32 MI_DIVP_IOCTL_SetChnAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DIVP_ChnAttrParams_t *arg = ptr;
    MI_DIVP_CHN DivpChn = arg->DivpChn;
    MI_DIVP_ChnAttr_t stChnAttr;
    memset(&stChnAttr, 0, sizeof(stChnAttr));
    memcpy(&stChnAttr, &arg->stChnAttr, sizeof(stChnAttr));

    AUTO_LOCK(env, mutex);
    s32Ret = MI_IMPL_DIVP_SetChnAttr(DivpChn, &stChnAttr);
    return s32Ret;
}

static MI_S32 MI_DIVP_IOCTL_GetChnAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DIVP_ChnAttrParams_t *arg = ptr;
    MI_DIVP_CHN DivpChn = arg->DivpChn;
    MI_DIVP_ChnAttr_t stChnAttr;
    memset(&stChnAttr, 0, sizeof(stChnAttr));

    AUTO_LOCK(env, mutex);
    s32Ret = MI_IMPL_DIVP_GetChnAttr(DivpChn, &stChnAttr);
    memcpy(&arg->stChnAttr, &stChnAttr, sizeof(stChnAttr));
    return s32Ret;
}

static MI_S32 MI_DIVP_IOCTL_StartChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DIVP_CHN DivpChn = *((MI_DIVP_CHN *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_IMPL_DIVP_StartChn(DivpChn);
    return s32Ret;
}

static MI_S32 MI_DIVP_IOCTL_StopChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DIVP_CHN DivpChn = *((MI_DIVP_CHN *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_IMPL_DIVP_StopChn(DivpChn);
    return s32Ret;
}

static MI_S32 MI_DIVP_IOCTL_SetOutputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DIVP_OutputPortAttrParams_t *arg = ptr;
    MI_DIVP_CHN DivpChn = arg->DivpChn;
    MI_DIVP_OutputPortAttr_t *pstOutputPortAttr = &(arg->stOutputPortAttr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_IMPL_DIVP_SetOutputPortAttr(DivpChn, pstOutputPortAttr);
    return s32Ret;
}

static MI_S32 MI_DIVP_IOCTL_GetOutputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DIVP_OutputPortAttrParams_t *arg = ptr;
    MI_DIVP_CHN DivpChn = arg->DivpChn;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));

    AUTO_LOCK(env, mutex);
    s32Ret = MI_IMPL_DIVP_GetOutputPortAttr(DivpChn, &stOutputPortAttr);
    memcpy(&arg->stOutputPortAttr, &stOutputPortAttr, sizeof(stOutputPortAttr));
    return s32Ret;
}

static MI_S32 MI_DIVP_IOCTL_RefreshChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DIVP_CHN DivpChn = *((MI_DIVP_CHN *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_IMPL_DIVP_RefreshChn(DivpChn);
    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_DIVP_CMD_MAX] = {
    [E_MI_DIVP_CMD_CREATE_CHN] = MI_DIVP_IOCTL_CreateChn,
    [E_MI_DIVP_CMD_DESTROY_CHN] = MI_DIVP_IOCTL_DestroyChn,
    [E_MI_DIVP_CMD_SET_CHN_ATTR] = MI_DIVP_IOCTL_SetChnAttr,
    [E_MI_DIVP_CMD_GET_CHN_ATTR] = MI_DIVP_IOCTL_GetChnAttr,
    [E_MI_DIVP_CMD_START_CHN] = MI_DIVP_IOCTL_StartChn,
    [E_MI_DIVP_CMD_STOP_CHN] = MI_DIVP_IOCTL_StopChn,
    [E_MI_DIVP_CMD_SET_OUTPUT_PORT_ATTR] = MI_DIVP_IOCTL_SetOutputPortAttr,
    [E_MI_DIVP_CMD_GET_OUTPUT_PORT_ATTR] = MI_DIVP_IOCTL_GetOutputPortAttr,
    [E_MI_DIVP_CMD_REFRESH_CHN] = MI_DIVP_IOCTL_RefreshChn,
};

static void mi_divp_insmod(const char *name)
{
    MI_PRINT("module [%s] init\n", name);
}

static void mi_divp_process_init(MI_COMMON_Client_t *client)
{
    MI_PRINT("client [%d] connected\n", client->pid);
    mi_divp_Init();
}

static void mi_divp_process_exit(MI_COMMON_Client_t *client)
{
    MI_PRINT("client [%d] disconnected\n", client->pid);
    mi_divp_DeInit();
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("colin.hu <colin.hu@mstarsemi.com>");

MI_DEVICE_DEFINE(mi_divp_insmod,
               ioctl_table, E_MI_DIVP_CMD_MAX,
               mi_divp_process_init, mi_divp_process_exit);
