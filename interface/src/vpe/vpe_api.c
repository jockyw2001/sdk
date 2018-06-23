
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
/// @file   vdec_api.c
/// @brief vdec module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "mi_syscall.h"
#include "mi_print.h"
#include "mi_sys.h"
#include "mi_vpe.h"
#include "vpe_ioctl.h"

MI_MODULE_DEFINE(disp);

MI_S32 MI_VPE_CreateChannel(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelAttr_t *pstVpeChAttr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_CreateChannel_t stCreateChannel;

    memset(&stCreateChannel, 0, sizeof(stCreateChannel));
    memcpy(&stCreateChannel.stVpeChAttr, pstVpeChAttr, sizeof(*pstVpeChAttr));
    stCreateChannel.VpeCh = VpeCh;
    s32Ret = MI_SYSCALL(MI_VPE_CREATE_CHANNEL, &stCreateChannel);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_CreateChannel);

MI_S32 MI_VPE_DestroyChannel(MI_VPE_CHANNEL VpeCh)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    s32Ret = MI_SYSCALL(MI_VPE_DESTROY_CHANNEL, &VpeCh);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_DestroyChannel);

MI_S32 MI_VPE_GetChannelAttr(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelAttr_t *pstVpeChAttr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelAttr_t stGetChannelAttr;

    memset(&stGetChannelAttr, 0, sizeof(stGetChannelAttr));
    stGetChannelAttr.VpeCh = VpeCh;
    memcpy(&stGetChannelAttr.stVpeChAttr, pstVpeChAttr, sizeof(*pstVpeChAttr));
    s32Ret = MI_SYSCALL(MI_VPE_GET_CHANNEL_ATTR, &stGetChannelAttr);
    if (s32Ret == MI_VPE_OK)
    {
        memcpy(pstVpeChAttr, &stGetChannelAttr.stVpeChAttr, sizeof(*pstVpeChAttr));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_GetChannelAttr);

MI_S32 MI_VPE_SetChannelAttr(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelAttr_t *pstVpeChAttr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetChannelAttr_t stSetChannelAttr;

    memset(&stSetChannelAttr, 0, sizeof(stSetChannelAttr));
    stSetChannelAttr.VpeCh = VpeCh;
    memcpy(&stSetChannelAttr.stVpeChAttr, pstVpeChAttr, sizeof(*pstVpeChAttr));
    s32Ret = MI_SYSCALL(MI_VPE_SET_CHANNEL_ATTR, &stSetChannelAttr);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_SetChannelAttr);

MI_S32 MI_VPE_StartChannel(MI_VPE_CHANNEL VpeCh)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    s32Ret = MI_SYSCALL(MI_VPE_START_CHANNEL, &VpeCh);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_StartChannel);

MI_S32 MI_VPE_StopChannel(MI_VPE_CHANNEL VpeCh)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    s32Ret = MI_SYSCALL(MI_VPE_STOP_CHANNEL, &VpeCh);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_StopChannel);

MI_S32 MI_VPE_SetChannelParam(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelPara_t *pstVpeParam)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetChannelParam_t stSetChannelParam;

    memset(&stSetChannelParam, 0, sizeof(stSetChannelParam));
    stSetChannelParam.VpeCh = VpeCh;
    memcpy(&stSetChannelParam.stVpeParam, pstVpeParam, sizeof(*pstVpeParam));
    s32Ret = MI_SYSCALL(MI_VPE_SET_CHANNEL_PARAM, &stSetChannelParam);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_SetChannelParam);

MI_S32 MI_VPE_GetChannelParam(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelPara_t *pstVpeParam)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelParam_t stGetChannelParam;

    memset(&stGetChannelParam, 0, sizeof(stGetChannelParam));
    stGetChannelParam.VpeCh = VpeCh;
    memcpy(&stGetChannelParam.stVpeParam, pstVpeParam, sizeof(*pstVpeParam));
    s32Ret = MI_SYSCALL(MI_VPE_GET_CHANNEL_PARAM, &stGetChannelParam);
    if (s32Ret == MI_VPE_OK)
    {
        memcpy(pstVpeParam, &stGetChannelParam.stVpeParam, sizeof(*pstVpeParam));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_GetChannelParam);

MI_S32 MI_VPE_SetChannelCrop(MI_VPE_CHANNEL VpeCh,  MI_SYS_WindowRect_t *pstCropInfo)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetChannelCrop_t stSetChannelCrop;

    memset(&stSetChannelCrop, 0, sizeof(stSetChannelCrop));
    stSetChannelCrop.VpeCh = VpeCh;
    memcpy(&stSetChannelCrop.stCropInfo, pstCropInfo, sizeof(*pstCropInfo));
    s32Ret = MI_SYSCALL(MI_VPE_SET_CHANNEL_CROP, &stSetChannelCrop);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_SetChannelCrop);

MI_S32 MI_VPE_GetChannelCrop(MI_VPE_CHANNEL VpeCh,  MI_SYS_WindowRect_t *pstCropInfo)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelCrop_t stGetChannelCrop;

    memset(&stGetChannelCrop, 0, sizeof(stGetChannelCrop));
    stGetChannelCrop.VpeCh = VpeCh;
    memcpy(&stGetChannelCrop.stCropInfo, pstCropInfo, sizeof(*pstCropInfo));
    s32Ret = MI_SYSCALL(MI_VPE_GET_CHANNEL_CROP, &stGetChannelCrop);
    if (s32Ret == MI_VPE_OK)
    {
        memcpy(pstCropInfo, &stGetChannelCrop.stCropInfo, sizeof(*pstCropInfo));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_GetChannelCrop);

MI_S32 MI_VPE_GetChannelRegionLuma(MI_VPE_CHANNEL VpeCh, MI_VPE_RegionInfo_t *pstRegionInfo, MI_U32 *pu32LumaData,MI_S32 s32MilliSec)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelRegionLuma_t stGetChannelRegionLuma;

    memset(&stGetChannelRegionLuma, 0, sizeof(stGetChannelRegionLuma));
    stGetChannelRegionLuma.VpeCh = VpeCh;
    stGetChannelRegionLuma.s32MilliSec = s32MilliSec;
    stGetChannelRegionLuma.u32RegionNum = pstRegionInfo->u32RegionNum;
    memcpy(stGetChannelRegionLuma.astWinRect, pstRegionInfo->pstWinRect, sizeof(*(pstRegionInfo->pstWinRect))*pstRegionInfo->u32RegionNum);
    s32Ret = MI_SYSCALL(MI_VPE_GET_CHANNEL_REGION_LUMA, &stGetChannelRegionLuma);
    if (s32Ret == MI_VPE_OK)
    {
        memcpy(pu32LumaData, (void *)stGetChannelRegionLuma.au32LumaData, sizeof(*pu32LumaData)*pstRegionInfo->u32RegionNum);
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_GetChannelRegionLuma);

MI_S32 MI_VPE_SetChannelRotation(MI_VPE_CHANNEL VpeCh,  MI_SYS_Rotate_e eType)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetChannelRotation_t stSetChannelRotation;

    memset(&stSetChannelRotation, 0, sizeof(stSetChannelRotation));
    stSetChannelRotation.VpeCh = VpeCh;
    stSetChannelRotation.eType = eType;
    s32Ret = MI_SYSCALL(MI_VPE_SET_CHANNEL_ROTATION, &stSetChannelRotation);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_SetChannelRotation);

MI_S32 MI_VPE_GetChannelRotation(MI_VPE_CHANNEL VpeCh,  MI_SYS_Rotate_e *pType)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetChannelRotation_t stGetChannelRotation;

    memset(&stGetChannelRotation, 0, sizeof(stGetChannelRotation));
    stGetChannelRotation.VpeCh = VpeCh;
    s32Ret = MI_SYSCALL(MI_VPE_GET_CHANNEL_ROTATION, &stGetChannelRotation);
    if (s32Ret == MI_VPE_OK)
    {
        *pType = stGetChannelRotation.Type;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_GetChannelRotation);

MI_S32 MI_VPE_EnablePort(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_EnablePort_t stEnablePort;

    memset(&stEnablePort, 0, sizeof(stEnablePort));
    stEnablePort.VpeCh   = VpeCh;
    stEnablePort.VpePort = VpePort;
    s32Ret = MI_SYSCALL(MI_VPE_ENABLE_PORT, &stEnablePort);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_EnablePort);

MI_S32 MI_VPE_DisablePort(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_DisablePort_t stDisablePort;

    memset(&stDisablePort, 0, sizeof(stDisablePort));
    stDisablePort.VpeCh   = VpeCh;
    stDisablePort.VpePort = VpePort;
    s32Ret = MI_SYSCALL(MI_VPE_DISABLE_PORT, &stDisablePort);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_DisablePort);

MI_S32 MI_VPE_SetPortMode(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort, MI_VPE_PortMode_t *pstVpeMode)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_SetPortMode_t stSetPortMode;

    memset(&stSetPortMode, 0, sizeof(stSetPortMode));
    stSetPortMode.VpeCh = VpeCh;
    stSetPortMode.VpePort = VpePort;
    memcpy(&stSetPortMode.stVpeMode, pstVpeMode, sizeof(*pstVpeMode));
    s32Ret = MI_SYSCALL(MI_VPE_SET_PORT_MODE, &stSetPortMode);
    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_SetPortMode);

MI_S32 MI_VPE_GetPortMode(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort, MI_VPE_PortMode_t *pstVpeMode)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_VPE_GetPortMode_t stGetPortMode;

    memset(&stGetPortMode, 0, sizeof(stGetPortMode));
    stGetPortMode.VpeCh = VpeCh;
    stGetPortMode.VpePort = VpePort;
    s32Ret = MI_SYSCALL(MI_VPE_GET_PORT_MODE, &stGetPortMode);
    if (s32Ret == MI_VPE_OK)
    {
        memcpy(pstVpeMode, &stGetPortMode.stVpeMode, sizeof(*pstVpeMode));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VPE_GetPortMode);
