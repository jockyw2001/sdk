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


#ifndef _AI_IOCTL_H_
#define _AI_IOCTL_H_

#include <linux/ioctl.h>
#include "mi_ai.h"

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
    E_MI_AI_CMD_SET_PUB_ATTR,
    E_MI_AI_CMD_GET_PUB_ATTR,
    E_MI_AI_CMD_ENABLE,
    E_MI_AI_CMD_DISABLE,
    E_MI_AI_CMD_ENABLE_CHN,
    E_MI_AI_CMD_DISABLE_CHN,
    E_MI_AI_CMD_SET_CHN_PARAM,
    E_MI_AI_CMD_GET_CHN_PARAM,
    E_MI_AI_CMD_ENABLE_RESMP,
    E_MI_AI_CMD_DISABLE_RESMP,
    E_MI_AI_CMD_SET_VQE_ATTR,
    E_MI_AI_CMD_ENABLE_VQE,
    E_MI_AI_CMD_DISABLE_VQE,
    E_MI_AI_CMD_CLR_PUB_ATTR,
    E_MI_AI_CMD_SET_AENC_ATTR,
    E_MI_AI_CMD_ENABLE_AENC,
    E_MI_AI_CMD_DISABLE_AENC,
    E_MI_AI_CMD_MAX,
} MI_AI_Cmd_e;


typedef struct MI_AI_SetPubAttr_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AUDIO_Attr_t stAttr;
} MI_AI_SetPubAttr_t;

typedef struct MI_AI_GetPubAttr_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AUDIO_Attr_t stAttr;
} MI_AI_GetPubAttr_t;

typedef struct MI_AI_EnableChn_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
} MI_AI_EnableChn_t;

typedef struct MI_AI_DisableChn_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
} MI_AI_DisableChn_t;

typedef struct MI_AI_GetFrame_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AUDIO_Frame_t stFrm;
    MI_AUDIO_AecFrame_t stAecFrm ;
    MI_S32 s32MilliSec;
} MI_AI_GetFrame_t;

typedef struct MI_AI_ReleaseFrame_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AUDIO_Frame_t stFrm;
    MI_AUDIO_AecFrame_t stAecFrm;
} MI_AI_ReleaseFrame_t;

typedef struct MI_AI_SetChnParam_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AI_ChnParam_t stChnParam;
} MI_AI_SetChnParam_t;

typedef struct MI_AI_GetChnParam_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AI_ChnParam_t stChnParam;
} MI_AI_GetChnParam_t;

typedef struct MI_AI_EnableReSmp_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AUDIO_SampleRate_e eOutSampleRate;
} MI_AI_EnableReSmp_t;

typedef struct MI_AI_DisableReSmp_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
} MI_AI_DisableReSmp_t;

typedef struct MI_AI_SetVqeAttr_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AUDIO_DEV AoDevId;
    MI_AI_CHN AoChn;
    MI_AI_VqeConfig_t stVqeConfig;
} MI_AI_SetVqeAttr_t;

typedef struct MI_AI_GetVqeAttr_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AI_VqeConfig_t stVqeConfig;
} MI_AI_GetVqeAttr_t;

typedef struct MI_AI_EnableVqe_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
} MI_AI_EnableVqe_t;

typedef struct MI_AI_DisableVqe_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
} MI_AI_DisableVqe_t;

typedef struct MI_AI_GetFd_s
{
    MI_AUDIO_DEV AiDevId ;
    MI_AI_CHN AiChn;
} MI_AI_GetFd_t;

typedef struct MI_AI_SaveFile_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AUDIO_SaveFileInfo_t stSaveFileInfo;
} MI_AI_SaveFile_t;

typedef struct MI_AI_SetVqeVolume_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_S32 s32VolumeDb;
} MI_AI_SetVqeVolume_t;


typedef struct MI_AI_EnableAenc_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
}MI_AI_EnableAenc_t;

typedef struct MI_AI_DisableAenc_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
}MI_AI_DisableAenc_t;

typedef struct MI_AI_SetAencAttr_s{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AI_AencConfig_t stAencConfig;
}MI_AI_SetAencAttr_t;

typedef struct MI_AI_GetAencAttr_s{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_AI_AencConfig_t stAencConfig;
}MI_AI_GetAencAttr_t;

#define MI_AI_SET_PUB_ATTR _IOW('i', E_MI_AI_CMD_SET_PUB_ATTR, MI_AI_SetPubAttr_t)
#define MI_AI_GET_PUB_ATTR _IOWR('i', E_MI_AI_CMD_GET_PUB_ATTR, MI_AI_GetPubAttr_t)
#define MI_AI_ENABLE _IOW('i', E_MI_AI_CMD_ENABLE, MI_AUDIO_DEV)
#define MI_AI_DISABLE _IOW('i', E_MI_AI_CMD_DISABLE, MI_AUDIO_DEV)
#define MI_AI_ENABLE_CHN _IOW('i', E_MI_AI_CMD_ENABLE_CHN, MI_AI_EnableChn_t)
#define MI_AI_DISABLE_CHN _IOW('i', E_MI_AI_CMD_DISABLE_CHN, MI_AI_DisableChn_t)
//#define MI_AI_GET_FRAME _IOWR('i', E_MI_AI_CMD_GET_FRAME, MI_AI_GetFrame_t)
//#define MI_AI_RELEASE_FRAME _IOW('i', E_MI_AI_CMD_RELEASE_FRAME, MI_AI_ReleaseFrame_t)
#define MI_AI_SET_CHN_PARAM _IOW('i', E_MI_AI_CMD_SET_CHN_PARAM, MI_AI_SetChnParam_t)
#define MI_AI_GET_CHN_PARAM _IOWR('i', E_MI_AI_CMD_GET_CHN_PARAM, MI_AI_GetChnParam_t)
#define MI_AI_ENABLE_RESMP _IOW('i', E_MI_AI_CMD_ENABLE_RESMP, MI_AI_EnableReSmp_t)
#define MI_AI_DISABLE_RESMP _IOW('i', E_MI_AI_CMD_DISABLE_RESMP, MI_AI_DisableReSmp_t)
#define MI_AI_SET_VQE_ATTR _IOW('i', E_MI_AI_CMD_SET_VQE_ATTR, MI_AI_SetVqeAttr_t)
//#define MI_AI_GET_VQE_ATTR _IOWR('i', E_MI_AI_CMD_GET_VQE_ATTR, MI_AI_GetVqeAttr_t)
#define MI_AI_ENABLE_VQE _IOW('i', E_MI_AI_CMD_ENABLE_VQE, MI_AI_EnableVqe_t)
#define MI_AI_DISABLE_VQE _IOW('i', E_MI_AI_CMD_DISABLE_VQE, MI_AI_DisableVqe_t)
//#define MI_AI_GET_FD _IOWR('i', E_MI_AI_CMD_GET_FD, MI_AI_GetFd_t)
#define MI_AI_CLR_PUB_ATTR _IOW('i', E_MI_AI_CMD_CLR_PUB_ATTR, MI_AUDIO_DEV)
#define MI_AI_SET_AENC_ATTR _IOW('i', E_MI_AI_CMD_SET_AENC_ATTR, MI_AI_SetAencAttr_t)
//#define MI_AI_GET_AENC_ATTR _IOW('i', E_MI_AI_CMD_GET_AENC_ATTR, MI_AI_GetAencAttr_t)
#define MI_AI_ENABLE_AENC   _IOW('i', E_MI_AI_CMD_ENABLE_AENC, MI_AI_EnableAenc_t)
#define MI_AI_DISABLE_AENC  _IOW('i', E_MI_AI_CMD_DISABLE_AENC, MI_AI_DisableAenc_t)
//#define MI_AI_SAVE_FILE _IOW('i', E_MI_AI_CMD_SAVE_FILE, MI_AI_SaveFile_t)
//#define MI_AI_SET_VQE_VOLUME _IOW('i', E_MI_AI_CMD_SET_VQE_VOLUME, MI_AI_SetVqeVolume_t)

#endif /// _AI_IOCTL_H_
