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


#ifndef _AO_IOCTL_H_
#define _AO_IOCTL_H_

#include <linux/ioctl.h>
#include "mi_ao.h"

//=============================================================================
// Include files
//=============================================================================

//=============================================================================
// Extern definition
//=============================================================================


//=============================================================================
// Data type definition
//=============================================================================
typedef enum
{
    E_MI_AO_CMD_SET_PUB_ATTR,
    E_MI_AO_CMD_GET_PUB_ATTR,
    E_MI_AO_CMD_ENABLE,
    E_MI_AO_CMD_DISABLE,
    E_MI_AO_CMD_ENABLE_CHN,
    E_MI_AO_CMD_DISABLE_CHN,
    E_MI_AO_CMD_ENABLE_RESMP,
    E_MI_AO_CMD_DISABLE_RESMP,
    E_MI_AO_CMD_PAUSE_CHN,
    E_MI_AO_CMD_RESUME_CHN,
    E_MI_AO_CMD_CLEAR_CHN_BUF,
    E_MI_AO_CMD_QUERY_CHN_STAT,
    E_MI_AO_CMD_SET_VOLUME,
    E_MI_AO_CMD_GET_VOLUME,
    E_MI_AO_CMD_SET_MUTE,
    E_MI_AO_CMD_GET_MUTE,
    E_MI_AO_CMD_CLR_PUB_ATTR,
    E_MI_AO_CMD_SET_VQE_ATTR,
    E_MI_AO_CMD_ENABLE_VQE,
    E_MI_AO_CMD_DISABLE_VQE,
    E_MI_AO_CMD_SET_ADEC_ATTR,
    E_MI_AO_CMD_ENABLE_ADEC,
    E_MI_AO_CMD_DISABLE_ADEC,
    E_MI_AO_CMD_UPDATE_QUEUE_STATUS,
    E_MI_AO_CMD_MAX,
} MI_AO_Cmd_e;


typedef struct MI_AO_SetPubAttr_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AUDIO_Attr_t stPubAttr;
} MI_AO_SetPubAttr_t;

typedef struct MI_AO_GetPubAttr_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AUDIO_Attr_t stPubAttr;
} MI_AO_GetPubAttr_t;

typedef struct MI_AO_EnableChn_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
} MI_AO_EnableChn_t;

typedef struct MI_AO_DisableChn_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
} MI_AO_DisableChn_t;

typedef struct MI_AO_SendFrame_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
    MI_AUDIO_Frame_t stData;
    MI_S32 s32MilliSec;
}MI_AO_SendFrame_t;

typedef struct MI_AO_EnableReSmp_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
    MI_AUDIO_SampleRate_e eInSampleRate;
}MI_AO_EnableReSmp_t;

typedef struct MI_AO_DisableReSmp_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
}MI_AO_DisableReSmp_t;

typedef struct MI_AO_PauseChn_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
}MI_AO_PauseChn_t;

typedef struct MI_AO_ResumeChn_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
}MI_AO_ResumeChn_t;

typedef struct MI_AO_ClearChnBuf_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
}MI_AO_ClearChnBuf_t;

typedef struct MI_AO_QueryChnStat_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
    MI_AO_ChnState_t stStatus;
}MI_AO_QueryChnStat_t;

typedef struct MI_AO_SetVolume_s
{
    MI_AUDIO_DEV AoDevId;
    MI_S32 s32VolumeDb;
}MI_AO_SetVolume_t;

typedef struct MI_AO_GetVolume_s
{
    MI_AUDIO_DEV AoDevId;
    MI_S32 s32VolumeDb;
}MI_AO_GetVolume_t;

typedef struct MI_AO_SetMute_s
{
    MI_AUDIO_DEV AoDevId;
    MI_BOOL bEnable;
}MI_AO_SetMute_t;

typedef struct MI_AO_GetMute_s
{
    MI_AUDIO_DEV AoDevId;
    MI_BOOL bEnable;
}MI_AO_GetMute_t;

typedef struct MI_AO_SetVqeAttr_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
    MI_AO_VqeConfig_t stVqeConfig;
}MI_AO_SetVqeAttr_t;

typedef struct MI_AO_GetVqeAttr_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
    MI_AO_VqeConfig_t stVqeConfig;
}MI_AO_GetVqeAttr_t;

typedef struct MI_AO_EnableVqe_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
}MI_AO_EnableVqe_t;

typedef struct MI_AO_DisableVqe_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
}MI_AO_DisableVqe_t;

typedef struct MI_AO_EnableAdec_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
}MI_AO_EnableAdec_t;

typedef struct MI_AO_DisableAdec_s
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
}MI_AO_DisableAdec_t;

typedef struct MI_AO_SetAdecAttr_s{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
    MI_AO_AdecConfig_t stAdecConfig;
}MI_AO_SetAdecAttr_t;

typedef struct MI_AO_GetAdecAttr_s{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
    MI_AO_AdecConfig_t stAdecConfig;
}MI_AO_GetAdecAttr_t;

typedef struct MI_AO_QueueStatus_s{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
    MI_S32 s32Max;//Max len of queue
    MI_S32 s32Size; //current size
    MI_U64 u64RunTime;  //running tims ms
    MI_U64 u64GetTotalSize;
}MI_AO_QueueStatus_t;
//=============================================================================
// Macro definition
//=============================================================================

#define MI_AO_SET_PUB_ATTR _IOW('i', E_MI_AO_CMD_SET_PUB_ATTR, MI_AO_SetPubAttr_t)
#define MI_AO_GET_PUB_ATTR _IOWR('i', E_MI_AO_CMD_GET_PUB_ATTR, MI_AO_GetPubAttr_t)
#define MI_AO_ENABLE _IOW('i', E_MI_AO_CMD_ENABLE, MI_AUDIO_DEV)
#define MI_AO_DISABLE _IOW('i', E_MI_AO_CMD_DISABLE, MI_AUDIO_DEV)
#define MI_AO_ENABLE_CHN _IOW('i', E_MI_AO_CMD_ENABLE_CHN, MI_AO_EnableChn_t)
#define MI_AO_DISABLE_CHN _IOW('i', E_MI_AO_CMD_DISABLE_CHN, MI_AO_DisableChn_t)
//#define MI_AO_SEND_FRAME _IOW('i', E_MI_AO_CMD_SEND_FRAME, MI_AO_SendFrame_t)
#define MI_AO_ENABLE_RESMP _IOW('i', E_MI_AO_CMD_ENABLE_RESMP, MI_AO_EnableReSmp_t)
#define MI_AO_DISABLE_RESMP _IOW('i', E_MI_AO_CMD_DISABLE_RESMP, MI_AO_DisableReSmp_t)
#define MI_AO_PAUSE_CHN _IOW('i', E_MI_AO_CMD_PAUSE_CHN, MI_AO_PauseChn_t)
#define MI_AO_RESUME_CHN _IOW('i', E_MI_AO_CMD_RESUME_CHN, MI_AO_ResumeChn_t)
#define MI_AO_CLEAR_CHN_BUF _IOW('i', E_MI_AO_CMD_CLEAR_CHN_BUF, MI_AO_ClearChnBuf_t)
#define MI_AO_QUERY_CHN_STAT _IOWR('i', E_MI_AO_CMD_QUERY_CHN_STAT, MI_AO_QueryChnStat_t)
#define MI_AO_SET_VOLUME _IOW('i', E_MI_AO_CMD_SET_VOLUME, MI_AO_SetVolume_t)
#define MI_AO_GET_VOLUME _IOWR('i', E_MI_AO_CMD_GET_VOLUME, MI_AO_GetVolume_t)
#define MI_AO_SET_MUTE _IOW('i', E_MI_AO_CMD_SET_MUTE, MI_AO_SetMute_t)
#define MI_AO_GET_MUTE _IOWR('i', E_MI_AO_CMD_GET_MUTE, MI_AO_GetMute_t)
#define MI_AO_CLR_PUB_ATTR _IOW('i', E_MI_AO_CMD_CLR_PUB_ATTR, MI_AUDIO_DEV)
#define MI_AO_SET_VQE_ATTR _IOW('i', E_MI_AO_CMD_SET_VQE_ATTR, MI_AO_SetVqeAttr_t)
//#define MI_AO_GET_VQE_ATTR _IOWR('i', E_MI_AO_CMD_GET_VQE_ATTR, MI_AO_GetVqeAttr_t)
#define MI_AO_ENABLE_VQE _IOW('i', E_MI_AO_CMD_ENABLE_VQE, MI_AO_EnableVqe_t)
#define MI_AO_DISABLE_VQE _IOW('i', E_MI_AO_CMD_DISABLE_VQE, MI_AO_DisableVqe_t)
#define MI_AO_SET_ADEC_ATTR _IOW('i', E_MI_AO_CMD_SET_ADEC_ATTR, MI_AO_SetAdecAttr_t)
#define MI_AO_ENABLE_ADEC _IOW('i', E_MI_AO_CMD_ENABLE_ADEC, MI_AO_EnableAdec_t)
#define MI_AO_DISABLE_ADEC _IOW('i', E_MI_AO_CMD_DISABLE_ADEC, MI_AO_DisableAdec_t)
#define MI_AO_UPDATE_QUEUE_STATUS _IOW('i', E_MI_AO_CMD_UPDATE_QUEUE_STATUS, MI_AO_QueueStatus_t)

#endif /// _AO_IOCTL_H_
