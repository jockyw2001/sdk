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

#ifndef _MI_AIO_DEP_DATATYPE_H_
#define _MI_AIO_DEP_DATATYPE_H_

#include "mi_sys_datatype.h"

//=============================================================================
// Include files
//=============================================================================


//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================
#define MI_AUDIO_MAX_CHN_NUM            16
#define MI_AUDIO_MAX_SAMPLES_PER_FRAME     2048
#define MI_AUDIO_MAX_FRAME_NUM             6

//=============================================================================
// Data type definition
//=============================================================================
typedef enum
{
    E_MI_AUDIO_SAMPLE_RATE_8000 =8000, /* 8kHz sampling rate */
    E_MI_AUDIO_SAMPLE_RATE_16000 =16000, /* 16kHz sampling rate */
    E_MI_AUDIO_SAMPLE_RATE_32000 =32000, /* 32kHz sampling rate */
    E_MI_AUDIO_SAMPLE_RATE_48000 =48000, /* 48kHz sampling rate */
    E_MI_AUDIO_SAMPLE_RATE_INVALID,
}MI_AUDIO_SampleRate_e;

typedef enum
{
    E_MI_AUDIO_BIT_WIDTH_16 =0, /* 16bit width */
    E_MI_AUDIO_BIT_WIDTH_24 =1, /* 24bit width */
    E_MI_AUDIO_BIT_WIDTH_MAX,
}MI_AUDIO_BitWidth_e;

typedef enum
{
    E_MI_AUDIO_MODE_I2S_MASTER, /* I2S master mode */
    E_MI_AUDIO_MODE_I2S_SLAVE, /* I2S slave mode */
    E_MI_AUDIO_MODE_TDM_MASTER, /* TDM master mode */
    E_MI_AUDIO_MODE_MAX,
}MI_AUDIO_Mode_e;

typedef enum
{
    E_MI_AUDIO_SOUND_MODE_MONO =0, /* mono */
    E_MI_AUDIO_SOUND_MODE_STEREO =1, /* stereo */
    E_MI_AUDIO_SOUND_MODE_MAX,
}MI_AUDIO_SoundMode_e;

typedef enum
{
    E_MI_AUDIO_HPF_FREQ_80 = 80, /* 80Hz */
    E_MI_AUDIO_HPF_FREQ_120 = 120, /* 120Hz */
    E_MI_AUDIO_HPF_FREQ_150 = 150, /* 150Hz */
    E_MI_AUDIO_HPF_FREQ_INVALID,
}MI_AUDIO_HpfFreq_e;

typedef enum {
    E_MI_AUDIO_NR_SPEED_LOW,
    E_MI_AUDIO_NR_SPEED_MID,
    E_MI_AUDIO_NR_SPEED_HIGH
}MI_AUDIO_NrSpeed_e;




typedef struct MI_AUDIO_Attr_s
{
    MI_AUDIO_SampleRate_e eSamplerate; /*sample rate*/
    MI_AUDIO_BitWidth_e eBitwidth; /*bitwidth*/
    MI_AUDIO_Mode_e eWorkmode; /*master or slave mode*/
    MI_AUDIO_SoundMode_e eSoundmode; /*mono or stereo*/
    MI_U32 u32FrmNum; /*frame num in buffer*/
    MI_U32 u32PtNumPerFrm; /*number of samples*/
    MI_U32 u32CodecChnCnt; /*channel number on Codec */
    MI_U32 u32ChnCnt;
}MI_AUDIO_Attr_t;

typedef struct MI_AUDIO_Frame_s
{
    MI_AUDIO_BitWidth_e eBitwidth; /*audio frame bitwidth*/
    MI_AUDIO_SoundMode_e eSoundmode; /*audio frame momo or stereo mode*/
    void *apVirAddr[2];
    MI_U64 u64TimeStamp;/*audio frame timestamp*/
    MI_U32 u32Seq; /*audio frame seq*/
    MI_U32 u32Len; /*data lenth per channel in frame*/
    MI_U32 au32PoolId[2];
}MI_AUDIO_Frame_t;

typedef struct MI_AUDIO_AecFrame_s
{
    MI_AUDIO_Frame_t stRefFrame; /* aec reference audio frame */
    MI_BOOL bValid; /* whether frame is valid */
}MI_AUDIO_AecFrame_t;

typedef struct MI_AUDIO_SaveFileInfo_s
{
    MI_BOOL bCfg;
    MI_U8 szFilePath[256];
    MI_U32 u32FileSize; /*in KB*/
}MI_AUDIO_SaveFileInfo_t;

typedef struct MI_AUDIO_HpfConfig_s
{
    MI_BOOL  bUsrMode;
    MI_AUDIO_HpfFreq_e eHpfFreq; /*freq to be processed*/
}MI_AUDIO_HpfConfig_t;

typedef struct MI_AUDIO_AnrConfig_s
{
    MI_BOOL  bUsrMode;
    MI_U32   u32NrIntensity;
    MI_U32   u32NrSmoothLevel;
    MI_AUDIO_NrSpeed_e eNrSpeed;
}MI_AUDIO_AnrConfig_t;

typedef struct AgcGainInfo_s{
    MI_S32   s32GainMax;
    MI_S32   s32GainMin;
    MI_S32   s32GainInit;
}AgcGainInfo_t;

typedef struct MI_AUDIO_AgcConfig_s
{
    MI_BOOL    bUsrMode;
    AgcGainInfo_t stAgcGainInfo;
    MI_U32     u32DropGainMax;
    MI_U32     u32AttackTime;
    MI_U32     u32ReleaseTime;
    MI_U32     u32CompressionRatio;
    MI_S32     s32TargetLevelDb;
    MI_S32     s32NoiseGateDb;
    MI_U32     u32NoiseGateAttenuationDb;
}MI_AUDIO_AgcConfig_t;

typedef struct EqGainDb_s
{
    MI_S16    s16EqGain100Hz;
    MI_S16    s16EqGain200Hz;
    MI_S16    s16EqGain250Hz;
    MI_S16    s16EqGain350Hz;
    MI_S16    s16EqGain500Hz;
    MI_S16    s16EqGain800Hz;
    MI_S16    s16EqGain1200Hz;
    MI_S16    s16EqGain2500Hz;
    MI_S16    s16EqGain4000Hz;
    MI_S16    s16EqGain8000Hz;
}EqGainDb_t;

typedef struct MI_AUDIO_EqConfig_s
{
    MI_BOOL    bUsrMode;
    EqGainDb_t stEqGain;
}MI_AUDIO_EqConfig_t;

typedef enum{
    E_MI_AUDIO_G726_MODE_16 = 0,
    E_MI_AUDIO_G726_MODE_24,
    E_MI_AUDIO_G726_MODE_32,
    E_MI_AUDIO_G726_MODE_40,
    E_MI_AUDIO_G726_MODE_INVALID,
}MI_AUDIO_G726Mode_e;

//=============================================================================
// Variable definition
//=============================================================================


//=============================================================================
// Global function definition
//=============================================================================


#endif // _MI_AIO_DEP_DATATYPE_H_
