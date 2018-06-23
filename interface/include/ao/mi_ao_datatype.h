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

#ifndef __MI_AO_DATATYPE_H__
#define __MI_AO_DATATYPE_H__

#include "mi_common_datatype.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Include files
//=============================================================================


//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================
#define MI_AO_DEV_NUM_MAX   4
#define MI_AO_CHAN_NUM_MAX  1

#define MI_AO_ERR_INVALID_DEVID     (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID))
#define MI_AO_ERR_ILLEGAL_PARAM     (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM))
#define MI_AO_ERR_NOT_ENABLED       (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_ENABLE))
#define MI_AO_ERR_NULL_PTR          (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR))
#define MI_AO_ERR_INVALID_CHNID     (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_CHNID))
#define MI_AO_ERR_NOT_CONFIG        (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG))
#define MI_AO_ERR_NOT_SUPPORT       (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT))
#define MI_AO_ERR_NOT_PERM          (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_PERM))
#define MI_AO_ERR_NOMEM             (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM))
#define MI_AO_ERR_NOBUF             (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOBUF))
#define MI_AO_ERR_BUF_EMPTY         (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_EMPTY))
#define MI_AO_ERR_BUF_FULL          (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_FULL))
#define MI_AO_ERR_SYS_NOTREADY      (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SYS_NOTREADY))
#define MI_AO_ERR_BUSY              (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY))
#define MI_AO_ERR_VQE_ERR           (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_AO_VQE_ERR))
#define MI_AO_ERR_ADEC_ERR          (MI_DEF_ERR( E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_AO_ADEC_ERR))
//=============================================================================
// Data type definition
//=============================================================================
typedef enum
{
    E_MI_AO_VQE_ERR = MI_AO_INITIAL_ERROR_CODE, /* VQE of AO error */
    E_MI_AO_ADEC_ERR,                           //ADEC of AO error
}MI_AO_ErrCode_e;


typedef struct MI_AO_ChnState_s
{
    MI_U32 u32ChnTotalNum;
    MI_U32 u32ChnFreeNum;
    MI_U32 u32ChnBusyNum;
} MI_AO_ChnState_t;

typedef struct MI_AO_VqeConfig_s
{
    MI_BOOL              bHpfOpen;
    MI_BOOL              bAnrOpen;
    MI_BOOL              bAgcOpen;
    MI_BOOL              bEqOpen;
    MI_S32               s32WorkSampleRate;
    MI_S32               s32FrameSample;
    MI_AUDIO_HpfConfig_t stHpfCfg;
    MI_AUDIO_AnrConfig_t stAnrCfg;
    MI_AUDIO_AgcConfig_t stAgcCfg;
    MI_AUDIO_EqConfig_t  stEqCfg;
}MI_AO_VqeConfig_t;


typedef enum{
    E_MI_AUDIO_ADEC_TYPE_G711A = 0,
    E_MI_AUDIO_ADEC_TYPE_G711U,
    E_MI_AUDIO_ADEC_TYPE_G726,
    E_MI_AUDIO_ADEC_TYPE_INVALID,
}MI_AUDIO_AdecType_e;


typedef struct MI_AUDIO_AdecG711Config_s{
    MI_AUDIO_SampleRate_e eSamplerate;
    MI_AUDIO_SoundMode_e eSoundmode;
}MI_AUDIO_AdecG711Config_t;

typedef struct MI_AUDIO_AdecG726Config_s{
    MI_AUDIO_SampleRate_e eSamplerate;
    MI_AUDIO_SoundMode_e eSoundmode;
    MI_AUDIO_G726Mode_e eG726Mode;
}MI_AUDIO_AdecG726Config_t;

typedef struct MI_AO_AdecConfig_s{
    MI_AUDIO_AdecType_e  eAdecType;
    union
    {
        MI_AUDIO_AdecG711Config_t  stAdecG711Cfg;
        MI_AUDIO_AdecG726Config_t  stAdecG726Cfg;
    };
}MI_AO_AdecConfig_t;

//=============================================================================
// Variable definition
//=============================================================================


//=============================================================================
// Global function definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif //__MI_AO_DATATYPE_H__
