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
#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_sd_impl.h"
#include "sd_ioctl.h"
#include "mi_sd_datatype.h"

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

static MI_S32 MI_SD_IOCTL_CreateChannel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_CreateChannel_t *pstCreateChannel = (MI_SD_CreateChannel_t *)ptr;
    MI_SD_CHANNEL SDCh = pstCreateChannel->SDCh;
    MI_SD_ChannelAttr_t *pstSDChAttr = &pstCreateChannel->stSDChAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, SDCh);
    s32Ret = MI_SD_IMPL_CreateChannel(SDCh, pstSDChAttr);
    return s32Ret;
}

static MI_S32 MI_SD_IOCTL_DestroyChannel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_CHANNEL SDCh = *(MI_SD_CHANNEL *)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, SDCh);
    s32Ret = MI_SD_IMPL_DestroyChannel(SDCh);
    return s32Ret;
}

static MI_S32 MI_SD_IOCTL_StartChannel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_CHANNEL SDCh = *(MI_SD_CHANNEL *)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, SDCh);
    s32Ret = MI_SD_IMPL_StartChannel(SDCh);
    return s32Ret;
}

static MI_S32 MI_SD_IOCTL_StopChannel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_CHANNEL SDCh = *(MI_SD_CHANNEL *)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, SDCh);
    s32Ret = MI_SD_IMPL_StopChannel(SDCh);

    return s32Ret;
}

static MI_S32 MI_SD_IOCTL_GetChannelAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_GetChannelAttr_t *pstGetChannelAttr = (MI_SD_GetChannelAttr_t *)ptr;
    MI_SD_CHANNEL SDCh = pstGetChannelAttr->SDCh;
    MI_SD_ChannelAttr_t *pstSDChAttr = &pstGetChannelAttr->stSDChAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, SDCh);
    s32Ret = MI_SD_IMPL_GetChannelAttr(SDCh, pstSDChAttr);

    return s32Ret;
}

static MI_S32 MI_SD_IOCTL_SetChannelAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_SetChannelAttr_t *pstSetChannelAttr = (MI_SD_SetChannelAttr_t *)ptr;
    MI_SD_CHANNEL SDCh = pstSetChannelAttr->SDCh;
    MI_SD_ChannelAttr_t *pstSDChAttr = &pstSetChannelAttr->stSDChAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, SDCh);
    s32Ret = MI_SD_IMPL_SetChannelAttr(SDCh, pstSDChAttr);
    return s32Ret;
}


static MI_S32 MI_SD_IOCTL_SetOutputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_SetPortAttr_t *pstSetPortMode = (MI_SD_SetPortAttr_t *)ptr;
    MI_SD_CHANNEL SDCh = pstSetPortMode->SDCh;
    MI_SD_OuputPortAttr_t *pstSDMode = &pstSetPortMode->stSDMode;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d\n", __func__, SDCh);
    s32Ret = MI_SD_IMPL_SetOutputPortAttr(SDCh, pstSDMode);

    return s32Ret;
}

static MI_S32 MI_SD_IOCTL_GetOutputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_GetPortAttr_t *pstGetPortMode = (MI_SD_GetPortAttr_t *)ptr;
    MI_SD_CHANNEL SDCh = pstGetPortMode->SDCh;
    MI_SD_OuputPortAttr_t *pstSDMode = &pstGetPortMode->stSDMode;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d\n", __func__, SDCh);
    s32Ret = MI_SD_IMPL_GetOutputPortAttr(SDCh, pstSDMode);
    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_SD_CMD_MAX] = {
    [E_MI_SD_CMD_CREATE_CHANNEL] = MI_SD_IOCTL_CreateChannel,
    [E_MI_SD_CMD_DESTROY_CHANNEL] = MI_SD_IOCTL_DestroyChannel,
    [E_MI_SD_CMD_START_CHANNEL] = MI_SD_IOCTL_StartChannel,
    [E_MI_SD_CMD_STOP_CHANNEL] = MI_SD_IOCTL_StopChannel,
    [E_MI_SD_CMD_SET_CHANNEL_ATTR] = MI_SD_IOCTL_SetChannelAttr,
    [E_MI_SD_CMD_GET_CHANNEL_ATTR] = MI_SD_IOCTL_GetChannelAttr,
    [E_MI_SD_CMD_SET_OUTPORT_ATTR] = MI_SD_IOCTL_SetOutputPortAttr,
    [E_MI_SD_CMD_GET_OUTPORT_ATTR] = MI_SD_IOCTL_GetOutputPortAttr,
};

static void mi_SD_insmod(const char *name);
static void mi_SD_process_init(MI_COMMON_Client_t *client);
static void mi_SD_process_exit(MI_COMMON_Client_t *client);


MI_DEVICE_DEFINE(mi_SD_insmod,
               ioctl_table, E_MI_SD_CMD_MAX,
               mi_SD_process_init, mi_SD_process_exit);


static void mi_SD_insmod(const char *name)
{
    MI_PRINT("module [%s] init\n", name);
}
// TODO: Tommy: IOCTRL  OPEN allow return value.
static void mi_SD_process_init(MI_COMMON_Client_t *client)
{
    MI_PRINT("client [%d] connected\n", client->pid);
    //debug_level = MI_DBG_ALL;
    MI_SD_IMPL_Init();
}
static void mi_SD_process_exit(MI_COMMON_Client_t *client)
{
    MI_PRINT("client [%d] disconnected\n", client->pid);
    MI_SD_IMPL_DeInit();
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");
