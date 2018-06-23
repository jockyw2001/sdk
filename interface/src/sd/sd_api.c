
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
#include "mi_sd.h"
#include "sd_ioctl.h"

MI_MODULE_DEFINE(SD);

MI_S32 MI_SD_CreateChannel(MI_SD_CHANNEL SDCh, MI_SD_ChannelAttr_t *pstSDChAttr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_CreateChannel_t stCreateChannel;

    memset(&stCreateChannel, 0, sizeof(stCreateChannel));
    memcpy(&stCreateChannel.stSDChAttr, pstSDChAttr, sizeof(*pstSDChAttr));
    stCreateChannel.SDCh = SDCh;
    s32Ret = MI_SYSCALL(MI_SD_CREATE_CHANNEL, &stCreateChannel);

    return s32Ret;
}
EXPORT_SYMBOL(MI_SD_CreateChannel);

MI_S32 MI_SD_DestroyChannel(MI_SD_CHANNEL SDCh)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    s32Ret = MI_SYSCALL(MI_SD_DESTROY_CHANNEL, &SDCh);
    return s32Ret;
}
EXPORT_SYMBOL(MI_SD_DestroyChannel);

MI_S32 MI_SD_StartChannel(MI_SD_CHANNEL SDCh)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    s32Ret = MI_SYSCALL(MI_SD_START_CHANNEL, &SDCh);
    return s32Ret;
}
EXPORT_SYMBOL(MI_SD_StartChannel);

MI_S32 MI_SD_StopChannel(MI_SD_CHANNEL SDCh)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    s32Ret = MI_SYSCALL(MI_SD_STOP_CHANNEL, &SDCh);
    return s32Ret;
}
EXPORT_SYMBOL(MI_SD_StopChannel);


MI_S32 MI_SD_SetChannelAttr(MI_SD_CHANNEL SDCh, MI_SD_ChannelAttr_t *pstSDChAttr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_SetChannelAttr_t stSetChannelAttr;

    memset(&stSetChannelAttr, 0, sizeof(stSetChannelAttr));
    stSetChannelAttr.SDCh = SDCh;
    memcpy(&stSetChannelAttr.stSDChAttr, pstSDChAttr, sizeof(*pstSDChAttr));
    s32Ret = MI_SYSCALL(MI_SD_SET_CHANNEL_ATTR, &stSetChannelAttr);
    return s32Ret;
}
EXPORT_SYMBOL(MI_SD_SetChannelAttr);


MI_S32 MI_SD_GetChannelAttr(MI_SD_CHANNEL SDCh, MI_SD_ChannelAttr_t *pstSDChAttr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_GetChannelAttr_t stGetChannelAttr;

    memset(&stGetChannelAttr, 0, sizeof(stGetChannelAttr));
    stGetChannelAttr.SDCh = SDCh;
    memcpy(&stGetChannelAttr.stSDChAttr, pstSDChAttr, sizeof(*pstSDChAttr));
    s32Ret = MI_SYSCALL(MI_SD_GET_CHANNEL_ATTR, &stGetChannelAttr);
    if (s32Ret == MI_SD_OK)
    {
        memcpy(pstSDChAttr, &stGetChannelAttr.stSDChAttr, sizeof(*pstSDChAttr));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_SD_GetChannelAttr);


MI_S32 MI_SD_SetOutputPortAttr(MI_SD_CHANNEL SDCh, MI_SD_OuputPortAttr_t *pstSDMode)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_SetPortAttr_t stSetPortMode;

    memset(&stSetPortMode, 0, sizeof(stSetPortMode));
    stSetPortMode.SDCh = SDCh;
    memcpy(&stSetPortMode.stSDMode, pstSDMode, sizeof(*pstSDMode));
    s32Ret = MI_SYSCALL(MI_SD_SET_OUTPORT_ATTR, &stSetPortMode);
    return s32Ret;
}
EXPORT_SYMBOL(MI_SD_SetOutputPortAttr);

MI_S32 MI_SD_GetOutputPortAttr(MI_SD_CHANNEL SDCh, MI_SD_OuputPortAttr_t *pstSDMode)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_GetPortAttr_t stGetPortMode;

    memset(&stGetPortMode, 0, sizeof(stGetPortMode));
    stGetPortMode.SDCh = SDCh;
    s32Ret = MI_SYSCALL(MI_SD_GET_OUTPORT_ATTR, &stGetPortMode);

    if (s32Ret == MI_SD_OK)
    {
        memcpy(pstSDMode, &stGetPortMode.stSDMode, sizeof(*pstSDMode));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_SD_GetOutputPortAttr);
