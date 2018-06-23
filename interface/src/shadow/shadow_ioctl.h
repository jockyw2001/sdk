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
#ifndef _MI_SHADOW_IOCTL_H_
#define _MI_SHADOW_IOCTL_H_
#include <linux/ioctl.h>
#include "mi_shadow.h"

typedef enum
{
    E_MI_SHADOW_CMD_REGISTER_DEV,
    E_MI_SHADOW_CMD_UNREGISTER_DEV,
    E_MI_SHADOW_CMD_GET_OUTPUTPORT_BUFFER,
    E_MI_SHADOW_CMD_GET_INPUTPORT_BUFFER,
    E_MI_SHADOW_CMD_FINISH_BUFFER,
    E_MI_SHADOW_CMD_REWIND_BUFFER,
    E_MI_SHADOW_CMD_WAIT_BUFFER_AVAIlLABLE,
    E_MI_SHADOW_CMD_WAIT_CALLBACK,
    E_MI_SHADOW_CMD_SET_CALLBACK_RESULT,
    E_MI_SHADOW_CMD_SET_CHANNEL_STATUS,
    E_MI_SHADOW_CMD_SET_INPUTPORT_BUFFER_EXTCONF,
    E_MI_SHADOW_CMD_SET_OUTPUTPORT_BUFFER_EXTCONF,
    E_MI_SHADOW_CMD_MAX,
} MI_SHADOW_Cmd_e;

typedef struct MI_SHADOW_RegisterDev_s
{
    MI_SHADOW_ModuleDevInfo_t stModDevInfo;
    MI_SHADOW_HANDLE hShadow;
} MI_SHADOW_RegisterDev_t;

typedef struct MI_SHADOW_GetPortBuf_s
{
    MI_SHADOW_HANDLE hShadow;
    MI_U32 u32ChnId;
    MI_U32 u32PortId;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hBufHandle;
} MI_SHADOW_GetPortBuf_t;

typedef struct MI_SHADOW_GetOutputPortBuf_s
{
    MI_SHADOW_GetPortBuf_t stGetPortBuf;
    MI_SYS_BufConf_t stBufCfg;
    MI_BOOL bBlockedByRateCtrl;
} MI_SHADOW_GetOutputPortBuf_t;

typedef struct MI_SHADOW_FinishBuf_s
{
    MI_SHADOW_HANDLE hShadow;
    MI_SYS_BUF_HANDLE hBufHandle;
} MI_SHADOW_FinishBuf_t;

typedef struct MI_SHADOW_WaitOnInputTaskAvailable_s
{
    MI_SHADOW_HANDLE hShadow;
    MI_S32 u32TimeOutMs;
} MI_SHADOW_WaitOnInputTaskAvailable_t;

typedef struct MI_SHADOW_CallbackInfo_s
{
    MI_SHADOW_HANDLE hShadow;
    MI_SHADOW_CALLBACK_EVENT_e eCallbackEvent;
    MI_SYS_ChnPort_t stChnCurryPort;
    MI_SYS_ChnPort_t stChnPeerPort;
    MI_S32 s32TimeOutMs;
} MI_SHADOW_CallbackInfo_t;

typedef struct MI_SHADOW_SetCallbackResult_s
{
    MI_SHADOW_HANDLE hShadow;
    MI_S32 s32Result;
} MI_SHADOW_SetCallbackResult_t;

typedef struct MI_SHADOW_UnRegisterDev_s
{
    MI_SHADOW_HANDLE hShadow;
} MI_SHADOW_UnRegisterDev_t;

typedef struct MI_SHADOW_SetChannelStatus_s
{
    MI_SHADOW_HANDLE hShadow;
    MI_U32 u32ChnId;
    MI_U32 u32PortId;
    MI_BOOL bEnable;
    MI_BOOL bChn;///change channel status
    MI_BOOL bInputPort;///change port status
} MI_SHADOW_SetChannelStatus_t;

typedef struct MI_SHADOW_SetPortExtraConf_s
{
    MI_SHADOW_HANDLE hShadow;
    MI_U32 u32ChnId;
    MI_U32 u32PortId;
    MI_SYS_FrameBufExtraConfig_t stBufExtraConf;
} MI_SHADOW_SetPortExtraConf_t;

#define MI_SHADOW_REGISTER_DEV _IOWR('i', E_MI_SHADOW_CMD_REGISTER_DEV, MI_SHADOW_RegisterDev_t)
#define MI_SHADOW_UNREGISTER_DEV _IOW('i', E_MI_SHADOW_CMD_UNREGISTER_DEV, MI_SHADOW_UnRegisterDev_t)
#define MI_SHADOW_GET_OUTPUTPORT_BUFFER _IOWR('i', E_MI_SHADOW_CMD_GET_OUTPUTPORT_BUFFER, MI_SHADOW_GetOutputPortBuf_t)
#define MI_SHADOW_GET_INPUTPORT_BUFFER _IOWR('i', E_MI_SHADOW_CMD_GET_INPUTPORT_BUFFER, MI_SHADOW_GetPortBuf_t)
#define MI_SHADOW_FINISH_BUFFER _IOW('i', E_MI_SHADOW_CMD_FINISH_BUFFER, MI_SHADOW_FinishBuf_t)
#define MI_SHADOW_REWIND_BUFFER _IOW('i', E_MI_SHADOW_CMD_REWIND_BUFFER, MI_SHADOW_FinishBuf_t)
#define MI_SHADOW_WAIT_BUFFER_AVAIlLABLE _IOW('i', E_MI_SHADOW_CMD_WAIT_BUFFER_AVAIlLABLE, MI_SHADOW_WaitOnInputTaskAvailable_t)
#define MI_SHADOW_WAIT_CALLBACK _IOWR('i', E_MI_SHADOW_CMD_WAIT_CALLBACK, MI_SHADOW_CallbackInfo_t)
#define MI_SHADOW_SET_CALLBACK_RESULT _IOW('i', E_MI_SHADOW_CMD_SET_CALLBACK_RESULT, MI_SHADOW_SetCallbackResult_t)
#define MI_SHADOW_SET_CHANNEL_STATUS _IOW('i', E_MI_SHADOW_CMD_SET_CHANNEL_STATUS, MI_SHADOW_SetChannelStatus_t)
#define MI_SHADOW_SET_INPUTPORT_BUFFER_EXTCONF _IOWR('i', E_MI_SHADOW_CMD_SET_INPUTPORT_BUFFER_EXTCONF, MI_SHADOW_SetPortExtraConf_t)
#define MI_SHADOW_SET_OUTPUTPORT_BUFFER_EXTCONF _IOWR('i', E_MI_SHADOW_CMD_SET_OUTPUTPORT_BUFFER_EXTCONF, MI_SHADOW_SetPortExtraConf_t)
#endif /* _MI_SHADOW_IOCTL_H_ */
