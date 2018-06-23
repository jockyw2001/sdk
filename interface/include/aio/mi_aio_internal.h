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

#ifndef _MI_AIO_INTERNAL_H_
#define _MI_AIO_INTERNAL_H_

#define  USE_CAM_OS  (0)

//=============================================================================
// Include files
//=============================================================================
#include "AudioSRCProcess.h"
#include "AudioProcess.h"
#include "AudioAecProcess.h"
#include "mi_aio_datatype.h"
#include "g711.h"
#include "g726.h"

//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================
#define MI_AUDIO_VQE_SAMPLES_UNIT   128

#define MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, eWidth)          \
    switch(eWidth)  \
    {   \
        case E_MI_AUDIO_BIT_WIDTH_16:   \
            u32BitWidthByte = 2;    \
            break;  \
        case E_MI_AUDIO_BIT_WIDTH_24:   \
            u32BitWidthByte = 4;    \
            break;  \
        default:    \
            u32BitWidthByte = 0; \
            DBG_ERR("BitWidth is illegal = %u.\n", eWidth); \
            break; \
    }

#define MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u32Chan, eSoundmode) \
    switch(eSoundmode)  \
    {   \
        case E_MI_AUDIO_SOUND_MODE_MONO:        \
            u32Chan = 1;    \
            break;      \
        case E_MI_AUDIO_SOUND_MODE_STEREO:   \
            u32Chan = 2;    \
            break;  \
        default:    \
            u32Chan = 0;    \
            DBG_ERR("eSoundmode is illegal = %u.\n", u32Chan); \
            break; \
    }

#define MI_AUDIO_VQE_HPF_TRANS_TYPE(eHpfFreq, eIaaHpfFreq)          \
    switch(eHpfFreq)  \
    {   \
        case E_MI_AUDIO_HPF_FREQ_80:   \
            eIaaHpfFreq = AUDIO_HPF_FREQ_80;    \
            break;  \
        case E_MI_AUDIO_HPF_FREQ_120:   \
            eIaaHpfFreq = AUDIO_HPF_FREQ_120;    \
            break;  \
         case E_MI_AUDIO_HPF_FREQ_150:   \
            eIaaHpfFreq = AUDIO_HPF_FREQ_150; \
            break; \
        default:    \
            eIaaHpfFreq = AUDIO_HPF_FREQ_BUTT; \
            DBG_ERR("eHpfFreq is illegal = %d \n", eHpfFreq); \
            break; \
    }

#define MI_AUDIO_VQE_NR_SPEED_TRANS_TYPE(eNrSpeed, eIaaNrSpeed)\
    switch(eNrSpeed)\
    {\
        case E_MI_AUDIO_NR_SPEED_LOW:\
            eIaaNrSpeed = NR_SPEED_LOW;\
            break;\
        case E_MI_AUDIO_NR_SPEED_MID:\
            eIaaNrSpeed = NR_SPEED_MID;\
            break;\
        case E_MI_AUDIO_NR_SPEED_HIGH:\
            eIaaNrSpeed = NR_SPEED_HIGH;\
            break;\
        default:\
            DBG_ERR("eIaaNrSpeed is illegal %d\n", eNrSpeed);\
            break;\
    }

#define MI_AUDIO_VQE_SAMPLERATE_TRANS_TYPE(eSampleRate, eIaaSampleRate)\
    switch(eSampleRate)\
    {\
        case E_MI_AUDIO_SAMPLE_RATE_8000:\
            eIaaSampleRate = IAA_APC_SAMPLE_RATE_8000;\
            break;\
        case E_MI_AUDIO_SAMPLE_RATE_16000:\
             eIaaSampleRate = IAA_APC_SAMPLE_RATE_16000;\
             break;\
        default:\
             DBG_ERR("eIaaSampleRate is illegal %d\n", eSampleRate);\
             while(1);\
             break;\
    }

//=============================================================================
// Data type definition
//=============================================================================




//=============================================================================
// Variable definition
//=============================================================================


//=============================================================================
// Global function definition
//=============================================================================


#endif // _MI_AIO_INTERNAL_H_
