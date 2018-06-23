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

#include "mi_hdmi_impl.h"
#include "hdmi_ioctl.h"

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
static MI_S32 MI_HDMI_IOCTL_Init(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_Init();

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_DeInit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_HDMI_IMPL_DeInit();

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_Open(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_HDMI_DeviceId_e *peHdmi = ptr;
    MI_HDMI_DeviceId_e eHdmi = *peHdmi;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_Open(eHdmi);

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_Close(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_DeviceId_e *peHdmi = ptr;
    MI_HDMI_DeviceId_e eHdmi = *peHdmi;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_Close(eHdmi);

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_SetAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_SetAttr_t *arg = ptr;
    MI_HDMI_DeviceId_e eHdmi = arg->eHdmi;
    MI_HDMI_Attr_t *pstAttr = &(arg->stAttr);
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_SetAttr(eHdmi, pstAttr);

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_GetAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_GetAttr_t *arg = ptr;
    MI_HDMI_DeviceId_e eHdmi = arg->eHdmi;
    MI_HDMI_Attr_t *pstAttr = &(arg->stAttr);
    MI_HDMI_Attr_t stAttr;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_GetAttr(eHdmi, &stAttr);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstAttr, &stAttr, sizeof(MI_HDMI_Attr_t));
    }

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_Start(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_DeviceId_e *peHdmi = ptr;
    MI_HDMI_DeviceId_e eHdmi = *peHdmi;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_Start(eHdmi);

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_Stop(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_DeviceId_e *peHdmi = ptr;
    MI_HDMI_DeviceId_e eHdmi = *peHdmi;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_Stop(eHdmi);

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_SetAvMute(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_SetAvMute_t *arg = ptr;
    MI_HDMI_DeviceId_e eHdmi = arg->eHdmi;
    MI_BOOL bAvMute = arg->bAvMute;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_SetAvMute(eHdmi, bAvMute);

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_ForceGetEdid(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_FroceGetEdid_t *arg = ptr;
    MI_HDMI_Edid_t *pstEdidData = &(arg->stEdidData);
    MI_HDMI_Edid_t stEdidData;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_ForceGetEdid(arg->eHdmi, &stEdidData);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstEdidData, &stEdidData, sizeof(MI_HDMI_Edid_t));
    }

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_SetInfoFrame(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_SetInfoFrame_t *arg = ptr;
    MI_HDMI_InfoFrame_t stInfoFrame;
    MI_HDMI_DeviceId_e eHdmi = arg->eHdmi;
    memcpy(&stInfoFrame, &(arg->stInfoFrame), sizeof(MI_HDMI_InfoFrame_t));
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_SetInfoFrame(eHdmi, &stInfoFrame);

    return s32Ret;
}

static MI_S32 MI_HDMI_IOCTL_GetInfoFrame(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_GetInfoFrame_t *arg = ptr;
    MI_HDMI_InfoFrame_t *pstInfoFrame = &(arg->stInfoFrame);
    MI_HDMI_InfoFrame_t stInfoFrame;
    MI_HDMI_DeviceId_e eHdmi = arg->eHdmi;
    MI_HDMI_InfoFrameType_e eInfoFrameType = arg->eInfoFrameType;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_HDMI_IMPL_GetInfoFrame(eHdmi, eInfoFrameType, &stInfoFrame);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstInfoFrame, &stInfoFrame, sizeof(MI_HDMI_InfoFrame_t));
    }

    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_HDMI_CMD_MAX] = {
    [E_MI_HDMI_CMD_INIT] = MI_HDMI_IOCTL_Init,
    [E_MI_HDMI_CMD_DEINIT] = MI_HDMI_IOCTL_DeInit,
    [E_MI_HDMI_CMD_OPEN] = MI_HDMI_IOCTL_Open,
    [E_MI_HDMI_CMD_CLOSE] = MI_HDMI_IOCTL_Close,
    [E_MI_HDMI_CMD_SET_ATTR] = MI_HDMI_IOCTL_SetAttr,
    [E_MI_HDMI_CMD_GET_ATTR] = MI_HDMI_IOCTL_GetAttr,
    [E_MI_HDMI_CMD_START] = MI_HDMI_IOCTL_Start,
    [E_MI_HDMI_CMD_STOP] = MI_HDMI_IOCTL_Stop,
    [E_MI_HDMI_CMD_SET_AV_MUTE] = MI_HDMI_IOCTL_SetAvMute,
    [E_MI_HDMI_CMD_FROCE_GET_EDID] = MI_HDMI_IOCTL_ForceGetEdid,
    [E_MI_HDMI_CMD_SET_INFO_FRAME] = MI_HDMI_IOCTL_SetInfoFrame,
    [E_MI_HDMI_CMD_GET_INFO_FRAME] = MI_HDMI_IOCTL_GetInfoFrame,
};

static void mi_hdmi_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
}

static void mi_hdmi_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
}

static void mi_hdmi_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");

MI_DEVICE_DEFINE(mi_hdmi_insmod,
               ioctl_table, E_MI_HDMI_CMD_MAX,
               mi_hdmi_process_init, mi_hdmi_process_exit);
