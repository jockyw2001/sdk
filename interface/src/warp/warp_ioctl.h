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

#ifndef __WARP_IOCTL_H__
#define __WARP_IOCTL_H__
#include <linux/ioctl.h>
#include "mi_warp.h"


typedef enum
{
    E_MI_WARP_INIT,
    E_MI_WARP_DEINIT,
    E_MI_WARP_CREATE_DEV,
    E_MI_WARP_DESTROY_DEV,
    E_MI_WARP_STARTDEV,
    E_MI_WARP_STOPDEV,
    E_MI_WARP_CREATE_CHN,
    E_MI_WARP_DESTROY_CHN,
    E_MI_WARP_ENABLECHANNEL,
    E_MI_WARP_DISABLECHANNEL,
    E_MI_WARP_ENABLEINPUTPORT,
    E_MI_WARP_DISABLEINPUTPORT,
    E_MI_WARP_ENABLEOUTPUTPORT,
    E_MI_WARP_DISABLEOUTPUTPORT,
    E_MI_WARP_SETTABLE,
    E_MI_WARP_CMD_MAX
} MI_WRAP_Cmd_e;


typedef struct MI_WARP_CreateDevice_s
{
    //MI_WARP_ModuleDevInfo_t stModuleInfo;
    MI_WARP_DEV devId;
} MI_WARP_CreateDevice_t;

typedef struct MI_WARP_DestroyDevice_s
{
//    MI_SYS_DRV_HANDLE hHandle;
    MI_WARP_DEV devId;
} MI_WARP_DestroyDevice_t;

typedef struct MI_WARP_CreateChannel_s
{
    MI_WARP_DEV devId;
    MI_WARP_CHN chnId;
} MI_WARP_CreateChannel_t;

typedef struct MI_WARP_DestroyChannel_s
{
    MI_WARP_DEV devId;
    MI_WARP_CHN chnId;
} MI_WARP_DestroyChannel_t;

typedef struct MI_WARP_EnableChannel_s
{
    MI_WARP_DEV devId;
    MI_WARP_CHN chnId;
} MI_WARP_EnableChannel_t;

typedef struct MI_WARP_DisableChannel_s
{
    MI_WARP_DEV devId;
    MI_WARP_CHN chnId;
} MI_WARP_DisableChannel_t;

typedef struct MI_WARP_EnableInputPort_s
{
    MI_WARP_DEV devId;
    MI_WARP_CHN chnId;
} MI_WARP_EnableInputPort_t;

typedef struct MI_WARP_DisableInputPort_s
{
    MI_WARP_DEV devId;
    MI_WARP_CHN chnId;
} MI_WARP_DisableInputPort_t;

typedef struct MI_WARP_EnableOutputPort_s
{
    MI_WARP_DEV devId;
    MI_WARP_CHN chnId;
} MI_WARP_EnableOutputPort_t;

typedef struct MI_WARP_DisableOutputPort_s
{
    MI_WARP_DEV devId;
    MI_WARP_CHN chnId;
} MI_WARP_DisableOutputPort_t;

typedef struct MI_WARP_ChnConfig_s
{
    MI_WARP_DEV devId;
    MI_WARP_CHN chnId;
    MI_WARP_TableType_e eTableType;
    void * pTableAddr;
    MI_U32 u32TableSize;
} MI_WARP_SetTable_t;


#define MI_WARP_MAGIC      'i'
#define MI_WARP_INIT                _IO(MI_WARP_MAGIC, E_MI_WARP_INIT)
#define MI_WARP_DEINIT              _IO(MI_WARP_MAGIC, E_MI_WARP_DEINIT)
#define MI_WARP_CREATE_DEV          _IOW(MI_WARP_MAGIC, E_MI_WARP_CREATE_DEV, MI_WARP_DEV)
#define MI_WARP_DESTROY_DEV        _IOW(MI_WARP_MAGIC, E_MI_WARP_DESTROY_DEV, MI_WARP_DEV)
#define MI_WARP_STARTDEV            _IOW(MI_WARP_MAGIC, E_MI_WARP_STARTDEV, MI_WARP_DEV)
#define MI_WARP_STOPDEV             _IOW(MI_WARP_MAGIC, E_MI_WARP_STOPDEV, MI_WARP_DEV)

#define MI_WARP_CREATE_CHANNEL      _IOW(MI_WARP_MAGIC, E_MI_WARP_CREATE_CHN, MI_WARP_CreateChannel_t)
#define MI_WARP_DESTROY_CHANNEL     _IOW(MI_WARP_MAGIC, E_MI_WARP_DESTROY_CHN, MI_WARP_DestroyChannel_t)

#define MI_WARP_ENABLECHANNEL       _IOW(MI_WARP_MAGIC, E_MI_WARP_ENABLECHANNEL, MI_WARP_EnableChannel_t)
#define MI_WARP_DISABLECHANNEL      _IOW(MI_WARP_MAGIC, E_MI_WARP_DISABLECHANNEL, MI_WARP_DisableChannel_t)
#define MI_WARP_ENABLEINPUTPORT     _IOW(MI_WARP_MAGIC, E_MI_WARP_ENABLEINPUTPORT, MI_WARP_EnableInputPort_t)
#define MI_WARP_DISABLEINPUTPORT    _IOW(MI_WARP_MAGIC, E_MI_WARP_DISABLEINPUTPORT, MI_WARP_DisableInputPort_t)
#define MI_WARP_ENABLEOUTPUTPORT    _IOW(MI_WARP_MAGIC, E_MI_WARP_ENABLEOUTPUTPORT, MI_WARP_EnableOutputPort_t)
#define MI_WARP_DISABLEOUTPUTPORT   _IOW(MI_WARP_MAGIC, E_MI_WARP_DISABLEOUTPUTPORT, MI_WARP_DisableOutputPort_t)

#define MI_WARP_SETTABLE   _IOW(MI_WARP_MAGIC, E_MI_WARP_SETTABLE, MI_WARP_SetTable_t)


#endif /// _VPE_IOCTL_H_
