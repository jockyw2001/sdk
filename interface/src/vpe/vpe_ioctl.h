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

#ifndef _VPE_IOCTL_H_
#define _VPE_IOCTL_H_
#include <linux/ioctl.h>
#include "mi_vpe.h"

typedef enum
{
    E_MI_VPE_CMD_CREATE_CHANNEL,
    E_MI_VPE_CMD_DESTROY_CHANNEL,
    E_MI_VPE_CMD_GET_CHANNEL_ATTR,
    E_MI_VPE_CMD_SET_CHANNEL_ATTR,
    E_MI_VPE_CMD_START_CHANNEL,
    E_MI_VPE_CMD_STOP_CHANNEL,
    E_MI_VPE_CMD_SET_CHANNEL_PARAM,
    E_MI_VPE_CMD_GET_CHANNEL_PARAM,
    E_MI_VPE_CMD_SET_CHANNEL_CROP,
    E_MI_VPE_CMD_GET_CHANNEL_CROP,
    E_MI_VPE_CMD_GET_CHANNEL_REGION_LUMA,
    E_MI_VPE_CMD_SET_CHANNEL_ROTATION,
    E_MI_VPE_CMD_GET_CHANNEL_ROTATION,
    E_MI_VPE_CMD_ENABLE_PORT,
    E_MI_VPE_CMD_DISABLE_PORT,
    E_MI_VPE_CMD_SET_PORT_MODE,
    E_MI_VPE_CMD_GET_PORT_MODE,
    E_MI_VPE_CMD_MAX,
} MI_VPE_Cmd_e;


typedef struct CreateChannel_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_VPE_ChannelAttr_t stVpeChAttr;
} MI_VPE_CreateChannel_t;

typedef struct GetChannelAttr_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_VPE_ChannelAttr_t stVpeChAttr;
} MI_VPE_GetChannelAttr_t;

typedef struct SetChannelAttr_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_VPE_ChannelAttr_t stVpeChAttr;
} MI_VPE_SetChannelAttr_t;

typedef struct SetChannelParam_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_VPE_ChannelPara_t stVpeParam;
} MI_VPE_SetChannelParam_t;

typedef struct GetChannelParam_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_VPE_ChannelPara_t stVpeParam;
} MI_VPE_GetChannelParam_t;

typedef struct SetChannelCrop_s
{
    MI_VPE_CHANNEL VpeCh;
     MI_SYS_WindowRect_t stCropInfo;
} MI_VPE_SetChannelCrop_t;

typedef struct GetChannelCrop_s
{
    MI_VPE_CHANNEL VpeCh;
     MI_SYS_WindowRect_t stCropInfo;
} MI_VPE_GetChannelCrop_t;

typedef struct GetChannelRegionLuma_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_SYS_WindowRect_t astWinRect[6];            // region attribute
    MI_U32 u32RegionNum;                    // count of the region
    MI_U32 au32LumaData[6];
    MI_S32 s32MilliSec;
} MI_VPE_GetChannelRegionLuma_t;

typedef struct SetChannelRotation_s
{
    MI_VPE_CHANNEL VpeCh;
     MI_SYS_Rotate_e eType;
} MI_VPE_SetChannelRotation_t;

typedef struct GetChannelRotation_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_SYS_Rotate_e Type;
} MI_VPE_GetChannelRotation_t;

typedef struct EnablePort_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_VPE_PORT VpePort;
} MI_VPE_EnablePort_t;

typedef struct DisablePort_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_VPE_PORT VpePort;
} MI_VPE_DisablePort_t;

typedef struct SetPortMode_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_VPE_PORT VpePort;
    MI_VPE_PortMode_t stVpeMode;
} MI_VPE_SetPortMode_t;

typedef struct GetPortMode_s
{
    MI_VPE_CHANNEL VpeCh;
    MI_VPE_PORT VpePort;
    MI_VPE_PortMode_t stVpeMode;
} MI_VPE_GetPortMode_t;

#define MI_VPE_CREATE_CHANNEL _IOW('i', E_MI_VPE_CMD_CREATE_CHANNEL, MI_VPE_CreateChannel_t)
#define MI_VPE_DESTROY_CHANNEL _IOW('i', E_MI_VPE_CMD_DESTROY_CHANNEL, MI_VPE_CHANNEL)
#define MI_VPE_GET_CHANNEL_ATTR _IOWR('i', E_MI_VPE_CMD_GET_CHANNEL_ATTR, MI_VPE_GetChannelAttr_t)
#define MI_VPE_SET_CHANNEL_ATTR _IOW('i', E_MI_VPE_CMD_SET_CHANNEL_ATTR, MI_VPE_SetChannelAttr_t)
#define MI_VPE_START_CHANNEL _IOW('i', E_MI_VPE_CMD_START_CHANNEL, MI_VPE_CHANNEL)
#define MI_VPE_STOP_CHANNEL _IOW('i', E_MI_VPE_CMD_STOP_CHANNEL, MI_VPE_CHANNEL)
#define MI_VPE_SET_CHANNEL_PARAM _IOW('i', E_MI_VPE_CMD_SET_CHANNEL_PARAM, MI_VPE_SetChannelParam_t)
#define MI_VPE_GET_CHANNEL_PARAM _IOWR('i', E_MI_VPE_CMD_GET_CHANNEL_PARAM, MI_VPE_GetChannelParam_t)
#define MI_VPE_SET_CHANNEL_CROP _IOW('i', E_MI_VPE_CMD_SET_CHANNEL_CROP, MI_VPE_SetChannelCrop_t)
#define MI_VPE_GET_CHANNEL_CROP _IOWR('i', E_MI_VPE_CMD_GET_CHANNEL_CROP, MI_VPE_GetChannelCrop_t)
#define MI_VPE_GET_CHANNEL_REGION_LUMA _IOWR('i', E_MI_VPE_CMD_GET_CHANNEL_REGION_LUMA, MI_VPE_GetChannelRegionLuma_t)
#define MI_VPE_SET_CHANNEL_ROTATION _IOW('i', E_MI_VPE_CMD_SET_CHANNEL_ROTATION, MI_VPE_SetChannelRotation_t)
#define MI_VPE_GET_CHANNEL_ROTATION _IOWR('i', E_MI_VPE_CMD_GET_CHANNEL_ROTATION, MI_VPE_GetChannelRotation_t)
#define MI_VPE_ENABLE_PORT _IOW('i', E_MI_VPE_CMD_ENABLE_PORT, MI_VPE_EnablePort_t)
#define MI_VPE_DISABLE_PORT _IOW('i', E_MI_VPE_CMD_DISABLE_PORT, MI_VPE_DisablePort_t)
#define MI_VPE_SET_PORT_MODE _IOW('i', E_MI_VPE_CMD_SET_PORT_MODE, MI_VPE_SetPortMode_t)
#define MI_VPE_GET_PORT_MODE _IOWR('i', E_MI_VPE_CMD_GET_PORT_MODE, MI_VPE_GetPortMode_t)
#endif /// _VPE_IOCTL_H_
