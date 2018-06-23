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

#ifndef __LDC_IOCTL_H__
#define __LDC_IOCTL_H__
#include <linux/ioctl.h>
#include "mi_ldc.h"


typedef enum
{
    E_MI_LDC_INIT,
    E_MI_LDC_DEINIT,
    E_MI_LDC_CREATE_DEV,
    E_MI_LDC_DESTROY_DEV,
    E_MI_LDC_STARTDEV,
    E_MI_LDC_STOPDEV,
    E_MI_LDC_CREATE_CHN,
    E_MI_LDC_DESTROY_CHN,
    E_MI_LDC_ENABLECHANNEL,
    E_MI_LDC_DISABLECHANNEL,
    E_MI_LDC_ENABLEINPUTPORT,
    E_MI_LDC_DISABLEINPUTPORT,
    E_MI_LDC_ENABLEOUTPUTPORT,
    E_MI_LDC_DISABLEOUTPUTPORT,
    E_MI_LDC_SETTABLE,
    E_MI_LDC_CMD_MAX
} MI_LDC_Cmd_e;


typedef struct MI_LDC_CreateDevice_s
{
    //MI_LDC_ModuleDevInfo_t stModuleInfo;
    MI_LDC_DEV devId;
} MI_LDC_CreateDevice_t;

typedef struct MI_LDC_DestroyDevice_s
{
//    MI_SYS_DRV_HANDLE hHandle;
    MI_LDC_DEV devId;
} MI_LDC_DestroyDevice_t;

typedef struct MI_LDC_CreateChannel_s
{
    MI_LDC_DEV devId;
    MI_LDC_CHN chnId;
} MI_LDC_CreateChannel_t;

typedef struct MI_LDC_DestroyChannel_s
{
    MI_LDC_DEV devId;
    MI_LDC_CHN chnId;
} MI_LDC_DestroyChannel_t;

typedef struct MI_LDC_EnableChannel_s
{
    MI_LDC_DEV devId;
    MI_LDC_CHN chnId;
} MI_LDC_EnableChannel_t;

typedef struct MI_LDC_DisableChannel_s
{
    MI_LDC_DEV devId;
    MI_LDC_CHN chnId;
} MI_LDC_DisableChannel_t;

typedef struct MI_LDC_EnableInputPort_s
{
    MI_LDC_DEV devId;
    MI_LDC_CHN chnId;
} MI_LDC_EnableInputPort_t;

typedef struct MI_LDC_DisableInputPort_s
{
    MI_LDC_DEV devId;
    MI_LDC_CHN chnId;
} MI_LDC_DisableInputPort_t;

typedef struct MI_LDC_EnableOutputPort_s
{
    MI_LDC_DEV devId;
    MI_LDC_CHN chnId;
} MI_LDC_EnableOutputPort_t;

typedef struct MI_LDC_DisableOutputPort_s
{
    MI_LDC_DEV devId;
    MI_LDC_CHN chnId;
} MI_LDC_DisableOutputPort_t;

typedef struct MI_LDC_ChnConfig_s
{
    MI_LDC_DEV devId;
    MI_LDC_CHN chnId;
    void * pTableAddr;
    MI_U32 u32TableSize;
} MI_LDC_SetTable_t;


#define MI_LDC_MAGIC      'i'
#define MI_LDC_INIT                _IO(MI_LDC_MAGIC, E_MI_LDC_INIT)
#define MI_LDC_DEINIT              _IO(MI_LDC_MAGIC, E_MI_LDC_DEINIT)
#define MI_LDC_CREATE_DEV          _IOW(MI_LDC_MAGIC, E_MI_LDC_CREATE_DEV, MI_LDC_DEV)
#define MI_LDC_DESTROY_DEV        _IOW(MI_LDC_MAGIC, E_MI_LDC_DESTROY_DEV, MI_LDC_DEV)
#define MI_LDC_STARTDEV            _IOW(MI_LDC_MAGIC, E_MI_LDC_STARTDEV, MI_LDC_DEV)
#define MI_LDC_STOPDEV             _IOW(MI_LDC_MAGIC, E_MI_LDC_STOPDEV, MI_LDC_DEV)

#define MI_LDC_CREATE_CHANNEL      _IOW(MI_LDC_MAGIC, E_MI_LDC_CREATE_CHN, MI_LDC_CreateChannel_t)
#define MI_LDC_DESTROY_CHANNEL     _IOW(MI_LDC_MAGIC, E_MI_LDC_DESTROY_CHN, MI_LDC_DestroyChannel_t)

#define MI_LDC_ENABLECHANNEL       _IOW(MI_LDC_MAGIC, E_MI_LDC_ENABLECHANNEL, MI_LDC_EnableChannel_t)
#define MI_LDC_DISABLECHANNEL      _IOW(MI_LDC_MAGIC, E_MI_LDC_DISABLECHANNEL, MI_LDC_DisableChannel_t)
#define MI_LDC_ENABLEINPUTPORT     _IOW(MI_LDC_MAGIC, E_MI_LDC_ENABLEINPUTPORT, MI_LDC_EnableInputPort_t)
#define MI_LDC_DISABLEINPUTPORT    _IOW(MI_LDC_MAGIC, E_MI_LDC_DISABLEINPUTPORT, MI_LDC_DisableInputPort_t)
#define MI_LDC_ENABLEOUTPUTPORT    _IOW(MI_LDC_MAGIC, E_MI_LDC_ENABLEOUTPUTPORT, MI_LDC_EnableOutputPort_t)
#define MI_LDC_DISABLEOUTPUTPORT   _IOW(MI_LDC_MAGIC, E_MI_LDC_DISABLEOUTPUTPORT, MI_LDC_DisableOutputPort_t)

#define MI_LDC_SETTABLE   _IOW(MI_LDC_MAGIC, E_MI_LDC_SETTABLE, MI_LDC_SetTable_t)


#endif /// _VPE_IOCTL_H_
