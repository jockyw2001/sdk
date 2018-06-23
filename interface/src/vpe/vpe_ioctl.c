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

#include "mi_vpe_impl.h"
#include "vpe_ioctl.h"
#include "mi_vpe_datatype.h"

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

static MI_S32 MI_VPE_IOCTL_CreateChannel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_CreateChannel_t *pstCreateChannel = (MI_VPE_CreateChannel_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstCreateChannel->VpeCh;
    MI_VPE_ChannelAttr_t *pstVpeChAttr = &pstCreateChannel->stVpeChAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_CreateChannel(VpeCh, pstVpeChAttr);
    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_DestroyChannel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_CHANNEL VpeCh = *(MI_VPE_CHANNEL *)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_DestroyChannel(VpeCh);
    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_GetChannelAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelAttr_t *pstGetChannelAttr = (MI_VPE_GetChannelAttr_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstGetChannelAttr->VpeCh;
    MI_VPE_ChannelAttr_t *pstVpeChAttr = &pstGetChannelAttr->stVpeChAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_GetChannelAttr(VpeCh, pstVpeChAttr);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_SetChannelAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetChannelAttr_t *pstSetChannelAttr = (MI_VPE_SetChannelAttr_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstSetChannelAttr->VpeCh;
    MI_VPE_ChannelAttr_t *pstVpeChAttr = &pstSetChannelAttr->stVpeChAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_SetChannelAttr(VpeCh, pstVpeChAttr);
    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_StartChannel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_CHANNEL VpeCh = *(MI_VPE_CHANNEL *)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_StartChannel(VpeCh);
    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_StopChannel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_CHANNEL VpeCh = *(MI_VPE_CHANNEL *)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_StopChannel(VpeCh);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_SetChannelParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetChannelParam_t *pstSetChannelParam = (MI_VPE_SetChannelParam_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstSetChannelParam->VpeCh;
    MI_VPE_ChannelPara_t *pstVpeParam = &pstSetChannelParam->stVpeParam;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_SetChannelParam(VpeCh, pstVpeParam);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_GetChannelParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelParam_t *pstGetChannelParam = (MI_VPE_GetChannelParam_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstGetChannelParam->VpeCh;
    MI_VPE_ChannelPara_t *pstVpeParam = &pstGetChannelParam->stVpeParam;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_GetChannelParam(VpeCh, pstVpeParam);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_SetChannelCrop(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetChannelCrop_t *pstSetChannelCrop = (MI_VPE_SetChannelCrop_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstSetChannelCrop->VpeCh;
    MI_SYS_WindowRect_t *pstCropInfo = &pstSetChannelCrop->stCropInfo;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_SetChannelCrop(VpeCh,  pstCropInfo);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_GetChannelCrop(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelCrop_t *pstGetChannelCrop = (MI_VPE_GetChannelCrop_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstGetChannelCrop->VpeCh;
    MI_SYS_WindowRect_t *pstCropInfo = &pstGetChannelCrop->stCropInfo;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_GetChannelCrop(VpeCh,  pstCropInfo);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_GetChannelRegionLuma(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelRegionLuma_t *pstGetChannelRegionLuma = (MI_VPE_GetChannelRegionLuma_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstGetChannelRegionLuma->VpeCh;
    MI_VPE_RegionInfo_t stRegionInfo;
    MI_U32 *pu32LumaData = pstGetChannelRegionLuma->au32LumaData;
    MI_S32 s32MilliSec   = pstGetChannelRegionLuma->s32MilliSec;
    memset(&stRegionInfo, 0, sizeof(stRegionInfo));
    stRegionInfo.pstWinRect = pstGetChannelRegionLuma->astWinRect;
    stRegionInfo.u32RegionNum = pstGetChannelRegionLuma->u32RegionNum;
    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_GetChannelRegionLuma(VpeCh, &stRegionInfo, pu32LumaData, s32MilliSec);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_SetChannelRotation(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetChannelRotation_t *pstSetChannelRotation = (MI_VPE_SetChannelRotation_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstSetChannelRotation->VpeCh;
    MI_SYS_Rotate_e eType = pstSetChannelRotation->eType;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d eType: %d.\n", __func__, VpeCh, eType);
    s32Ret = MI_VPE_IMPL_SetChannelRotation(VpeCh, eType);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_GetChannelRotation(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelRotation_t *pstGetChannelRotation = (MI_VPE_GetChannelRotation_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstGetChannelRotation->VpeCh;
    MI_SYS_Rotate_e *pType = &pstGetChannelRotation->Type;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d.\n", __func__, VpeCh);
    s32Ret = MI_VPE_IMPL_GetChannelRotation(VpeCh,  pType);
    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_EnablePort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_EnablePort_t *pstEnablePort = (MI_VPE_EnablePort_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstEnablePort->VpeCh;
    MI_VPE_PORT VpePort  = pstEnablePort->VpePort;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d Port: %d.\n", __func__, VpeCh, VpePort);
    s32Ret = MI_VPE_IMPL_EnablePort(VpeCh, VpePort);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_DisablePort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_DisablePort_t *pstDisablePort = (MI_VPE_DisablePort_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstDisablePort->VpeCh;
    MI_VPE_PORT VpePort  = pstDisablePort->VpePort;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d Port: %d.\n", __func__, VpeCh, VpePort);
    s32Ret = MI_VPE_IMPL_DisablePort(VpeCh, VpePort);
    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_SetPortMode(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetPortMode_t *pstSetPortMode = (MI_VPE_SetPortMode_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstSetPortMode->VpeCh;
    MI_VPE_PORT VpePort  = pstSetPortMode->VpePort;
    MI_VPE_PortMode_t *pstVpeMode = &pstSetPortMode->stVpeMode;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d Port: %d.\n", __func__, VpeCh, VpePort);
    s32Ret = MI_VPE_IMPL_SetPortMode(VpeCh, VpePort, pstVpeMode);

    return s32Ret;
}

static MI_S32 MI_VPE_IOCTL_GetPortMode(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetPortMode_t *pstGetPortMode = (MI_VPE_GetPortMode_t *)ptr;
    MI_VPE_CHANNEL VpeCh = pstGetPortMode->VpeCh;
    MI_VPE_PORT VpePort  = pstGetPortMode->VpePort;
    MI_VPE_PortMode_t *pstVpeMode = &pstGetPortMode->stVpeMode;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Ch: %d Port: %d.\n", __func__, VpeCh, VpePort);
    s32Ret = MI_VPE_IMPL_GetPortMode(VpeCh, VpePort, pstVpeMode);
    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_VPE_CMD_MAX] = {
    [E_MI_VPE_CMD_CREATE_CHANNEL] = MI_VPE_IOCTL_CreateChannel,
    [E_MI_VPE_CMD_DESTROY_CHANNEL] = MI_VPE_IOCTL_DestroyChannel,
    [E_MI_VPE_CMD_GET_CHANNEL_ATTR] = MI_VPE_IOCTL_GetChannelAttr,
    [E_MI_VPE_CMD_SET_CHANNEL_ATTR] = MI_VPE_IOCTL_SetChannelAttr,
    [E_MI_VPE_CMD_START_CHANNEL] = MI_VPE_IOCTL_StartChannel,
    [E_MI_VPE_CMD_STOP_CHANNEL] = MI_VPE_IOCTL_StopChannel,
    [E_MI_VPE_CMD_SET_CHANNEL_PARAM] = MI_VPE_IOCTL_SetChannelParam,
    [E_MI_VPE_CMD_GET_CHANNEL_PARAM] = MI_VPE_IOCTL_GetChannelParam,
    [E_MI_VPE_CMD_SET_CHANNEL_CROP] = MI_VPE_IOCTL_SetChannelCrop,
    [E_MI_VPE_CMD_GET_CHANNEL_CROP] = MI_VPE_IOCTL_GetChannelCrop,
    [E_MI_VPE_CMD_GET_CHANNEL_REGION_LUMA] = MI_VPE_IOCTL_GetChannelRegionLuma,
    [E_MI_VPE_CMD_SET_CHANNEL_ROTATION] = MI_VPE_IOCTL_SetChannelRotation,
    [E_MI_VPE_CMD_GET_CHANNEL_ROTATION] = MI_VPE_IOCTL_GetChannelRotation,
    [E_MI_VPE_CMD_ENABLE_PORT] = MI_VPE_IOCTL_EnablePort,
    [E_MI_VPE_CMD_DISABLE_PORT] = MI_VPE_IOCTL_DisablePort,
    [E_MI_VPE_CMD_SET_PORT_MODE] = MI_VPE_IOCTL_SetPortMode,
    [E_MI_VPE_CMD_GET_PORT_MODE] = MI_VPE_IOCTL_GetPortMode,
};

static void mi_vpe_insmod(const char *name);
static void mi_vpe_process_init(MI_COMMON_Client_t *client);
static void mi_vpe_process_exit(MI_COMMON_Client_t *client);


MI_DEVICE_DEFINE(mi_vpe_insmod,
               ioctl_table, E_MI_VPE_CMD_MAX,
               mi_vpe_process_init, mi_vpe_process_exit);


static void mi_vpe_insmod(const char *name)
{
    MI_PRINT("module [%s] init\n", name);
}
// TODO: Tommy: IOCTRL  OPEN allow return value.
static void mi_vpe_process_init(MI_COMMON_Client_t *client)
{
    MI_PRINT("client [%d] connected\n", client->pid);
    //debug_level = MI_DBG_ALL;
    MI_VPE_IMPL_Init();
}
static void mi_vpe_process_exit(MI_COMMON_Client_t *client)
{
    MI_PRINT("client [%d] disconnected\n", client->pid);
    MI_VPE_IMPL_DeInit();
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");
