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
/// @file   ao_api.c
/// @brief vdec module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __KERNEL__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include "mi_syscall.h"
#include "mi_print.h"
#include "mi_sys.h"
#include "mi_common.h"
#include "mi_ao.h"
#include "mi_aio_internal.h"
#include "ao_ioctl.h"

#if USE_CAM_OS
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#endif
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

MI_MODULE_DEFINE(ao)

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------
#define MI_AO_USR_CHECK_DEV(AoDevId)  \
    if(AoDevId < 0 || AoDevId >= MI_AO_DEV_NUM_MAX) \
    {   \
        DBG_ERR("AoDevId is invalid! AoDevId = %u.\n", AoDevId);   \
        return MI_AO_ERR_INVALID_DEVID;   \
    }

#define MI_AO_USR_CHECK_CHN(Aochn)  \
    if(Aochn < 0 || Aochn >= MI_AO_CHAN_NUM_MAX) \
    {   \
        DBG_ERR("Aochn is invalid! Aochn = %u.\n", Aochn);   \
        return MI_AO_ERR_INVALID_CHNID;   \
    }

#define MI_AO_USR_CHECK_POINTER(pPtr)  \
    if(NULL == pPtr)  \
    {   \
        DBG_ERR("Invalid parameter! NULL pointer.\n");   \
        return MI_AO_ERR_NULL_PTR;   \
    }

#define MI_AO_USR_CHECK_SAMPLERATE(eSamppleRate)    \
    if( (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_8000) && (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_16000) &&\
        (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_32000) && (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_48000) ) \
    { \
        DBG_ERR("Sample Rate is illegal = %u.\n", eSamppleRate);   \
        return MI_AO_ERR_ILLEGAL_PARAM;   \
    }

#if USE_CAM_OS
    #define MI_AO_Malloc CamOsMemAlloc
    #define MI_AO_Free  CamOsMemRelease
#else
    #define MI_AO_Malloc malloc
    #define MI_AO_Free free
#endif

#define MI_AO_APC_EQ_BAND_NUM 10

#define MI_AO_G711A 0
#define MI_AO_G711U 1
#define MI_AO_POINT_NUM_MAX (25600) //25K
#define MI_AO_ADEC_G726_UNIT (60)
#define MI_AO_ADEC_G726_UNIT_MAX (1200)

#define MI_AO_SRC_UNIT     (256)
#define MI_AO_SRC_UNIT_MAX (1024)


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

typedef struct _MI_AO_QueueInfo_s{
    MI_S32 s32Front;
    MI_S32 s32Rear;
    MI_S32 s32Max;//Max len of queue
    MI_S32 s32Size; //current size
    MI_U8  *pu8Buff;
}_MI_AO_QueueInfo_t;

typedef struct _MI_AO_SendDataInfo_s{
    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
}_MI_AO_SendDataInfo_t;


typedef struct _MI_AO_ChanInfoUsr_s
{
    MI_BOOL                 bChanEnable;
    MI_BOOL                 bPortEnable;
    MI_S32                  s32ChnId;
    MI_S32                  s32InputPortId;
    MI_S32                  s32OutputPortId;
    MI_BOOL                 bChanPause;
    MI_BOOL                 bResampleEnable;
    MI_AUDIO_SampleRate_e   eInResampleRate;
    MI_BOOL                 bVqeEnable;
    MI_BOOL                 bVqeAttrSet;
    MI_AO_VqeConfig_t       stAoVqeConfig;
    MI_BOOL                 bAdecAttrSet;
    MI_BOOL                 bAdecEnable;
    MI_AO_AdecConfig_t      stAoAdecConfig;
    //Resample(SRC)
    SRC_HANDLE              hSrcHandle;
    void*                   pSrcWorkingBuf;
    // VQE
    APC_HANDLE              hApcHandle;
    void*                   pApcWorkingBuf;
    // g726
    g726_state_t            *hG72Handle;

    //Queue
    _MI_AO_QueueInfo_t      stAdecInputQueue;//store encoder data
    _MI_AO_QueueInfo_t      stSrcInputQueue;//store resample data
    _MI_AO_QueueInfo_t      stVqeInputQueue;
    _MI_AO_QueueInfo_t      stChnOutputQueue;//store channel data

    //Tmp buff
    MI_U8                   *pu8InputBuff;//cat't define huge array,so need to malloc
    MI_U8                   *pu8OutputBuff;
    MI_U64                  u64StartTime; //ms
    MI_U64                  u64GetTotalSize;

    _MI_AO_SendDataInfo_t   stAoSendDataInfo;
    MI_BOOL                 bAoPthreadExit;
#ifndef __KERNEL__
    pthread_t               stAoPthreadId;
    pthread_mutex_t         stAoMutex;
#endif
}_MI_AO_ChanInfoUsr_t;

typedef struct _MI_AO_DevInfoUsr_s
{
    MI_BOOL                 bDevEnable;
    MI_BOOL                 bDevAttrSet;
    MI_AUDIO_DEV            AoDevId;
    MI_AUDIO_Attr_t         stDevAttr;
    _MI_AO_ChanInfoUsr_t     astChanInfo[MI_AO_CHAN_NUM_MAX];
    MI_U64                  u64PhyBufAddr;          // for DMA HW address
}_MI_AO_DevInfoUsr_t;



//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
// initial & Todo: need to protect variable
static _MI_AO_DevInfoUsr_t _gastAoDevInfoUsr[MI_AO_DEV_NUM_MAX]={
    {
        .bDevEnable = FALSE,
        .bDevAttrSet = FALSE,
        .AoDevId = 0,
        .astChanInfo[0] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
        },
     },
    {
        .bDevEnable = FALSE,
        .bDevAttrSet = FALSE,
        .AoDevId = 1,
        .astChanInfo[0] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
        },
    },
    {
        .bDevEnable = FALSE,
        .bDevAttrSet = FALSE,
        .AoDevId = 2,
        .astChanInfo[0] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
        },
    },
    {
        .bDevEnable = FALSE,
        .bDevAttrSet = FALSE,
        .AoDevId = 3,
        .astChanInfo[0] = {
        .bChanEnable = FALSE,
        .bPortEnable = FALSE,
        .bResampleEnable = FALSE,
        .bVqeEnable = FALSE,
        .bVqeAttrSet =FALSE,
        .hSrcHandle = NULL,
        },
    },
};

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

static SrcConversionMode _MI_AO_GetSrcConvertMode(MI_AUDIO_SampleRate_e  eInResampleRate, MI_AUDIO_SampleRate_e eOutSamplerate)
{

    switch (eInResampleRate)
    {
        case E_MI_AUDIO_SAMPLE_RATE_8000:
            switch (eOutSamplerate)
            {
                case E_MI_AUDIO_SAMPLE_RATE_16000:
                    return SRC_8k_to_16k;

                case E_MI_AUDIO_SAMPLE_RATE_32000:
                    return SRC_8k_to_32k;

                case E_MI_AUDIO_SAMPLE_RATE_48000:
                    return SRC_8k_to_48k;

               default:
                    DBG_ERR("Can't found correct mode for sample rate %d to %d\n", eInResampleRate, eOutSamplerate);
                    break;
            }
            break;

        case E_MI_AUDIO_SAMPLE_RATE_16000:
            switch (eOutSamplerate)
            {
                case E_MI_AUDIO_SAMPLE_RATE_8000:
                    return SRC_16k_to_8k;

                case E_MI_AUDIO_SAMPLE_RATE_32000:
                    return SRC_16k_to_32k;

                case E_MI_AUDIO_SAMPLE_RATE_48000:
                    return SRC_16k_to_48k;

               default:
                    DBG_ERR("Can't found correct mode for sample rate %d to %d\n", eInResampleRate, eOutSamplerate);
                    break;
            }
            break;

        case E_MI_AUDIO_SAMPLE_RATE_32000:
            switch (eOutSamplerate)
            {
                case E_MI_AUDIO_SAMPLE_RATE_8000:
                    return SRC_32k_to_8k;

                case E_MI_AUDIO_SAMPLE_RATE_16000:
                    return SRC_32k_to_16k;

                case E_MI_AUDIO_SAMPLE_RATE_48000:
                    return SRC_32k_to_48k;

               default:
                    DBG_ERR("Can't found correct mode for sample rate %d to %d\n", eInResampleRate, eOutSamplerate);
                    break;
            }
            break;

        case E_MI_AUDIO_SAMPLE_RATE_48000:
            switch (eOutSamplerate)
            {
                case E_MI_AUDIO_SAMPLE_RATE_8000:
                    return SRC_48k_to_8k;

                case E_MI_AUDIO_SAMPLE_RATE_16000:
                    return SRC_48k_to_16k;

                case E_MI_AUDIO_SAMPLE_RATE_32000:
                    return SRC_48k_to_32k;

               default:
                    DBG_ERR("Can't found correct mode for sample rate %d to %d\n", eInResampleRate, eOutSamplerate);
                    break;
            }
            break;

        default:
            DBG_ERR("Can't found correct mode for sample rate %d to %d\n", eInResampleRate, eOutSamplerate);
            break;
    }

    return (SRC_48k_to_32k+1); // fail case

}


static MI_S32 _MI_AO_ReSmpInit(MI_AUDIO_DEV AoDevId, MI_AI_CHN AoChn, MI_AUDIO_SampleRate_e eInSampleRate)
{
    MI_S32 s32Ret = MI_SUCCESS;

    SRCStructProcess stSrcStruct;
    MI_AUDIO_SampleRate_e eAttrSampleRate;
    MI_U16 u16ChanlNum;
    MI_U32 u32SrcUnit;
    MI_U32 u32BitWidthByte;


     /* SRC parameter setting */
    eAttrSampleRate = _gastAoDevInfoUsr[AoDevId].stDevAttr.eSamplerate;
    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAoDevInfoUsr[AoDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAoDevInfoUsr[AoDevId].stDevAttr.eBitwidth);
    u32SrcUnit = _gastAoDevInfoUsr[AoDevId].stDevAttr.u32PtNumPerFrm;

    stSrcStruct.WaveIn_srate = (SrcInSrate) (eInSampleRate/1000);// get_sample_rate_enumeration(input_wave->wave_header.sample_per_sec);
    stSrcStruct.channel = u16ChanlNum;
    stSrcStruct.mode = _MI_AO_GetSrcConvertMode(eInSampleRate, eAttrSampleRate);

    if (u32SrcUnit < MI_AO_SRC_UNIT)
    {
        stSrcStruct.point_number = MI_AO_SRC_UNIT;
    }
    else if (u32SrcUnit > MI_AO_SRC_UNIT_MAX)
    {
        stSrcStruct.point_number = MI_AO_SRC_UNIT_MAX;
    }
    else
    {
        stSrcStruct.point_number = u32SrcUnit - (u32SrcUnit % MI_AO_SRC_UNIT);
    }
    /* SRC init */
#ifndef __KERNEL__
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pSrcWorkingBuf = MI_AO_Malloc(IaaSrc_GetBufferSize(stSrcStruct.mode));
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hSrcHandle = IaaSrc_Init(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pSrcWorkingBuf, &stSrcStruct);

#endif

    if(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hSrcHandle == NULL)
    {
         //s32Ret = MI_ERR_AO_VQE_ERR; //add ReSmp error
        DBG_ERR("_MI_AO_ReSmpInit Fail !!!!! \n");
    }

    return s32Ret;
}

#ifndef __KERNEL__

static MI_S32 _MI_AO_SetEqGainDb(EqGainDb_t *pstEqGainInfo, MI_S16  *pS16Buff, MI_U32 u32Size)
{
    MI_S16 *ps16Tmp = (MI_S16 *)pstEqGainInfo;
    MI_S32 s32i = 0;

    MI_AO_USR_CHECK_POINTER(pstEqGainInfo);
    MI_AO_USR_CHECK_POINTER(pS16Buff);

    for (s32i = 0; s32i < u32Size; s32i++)
    {
        pS16Buff[s32i] = *ps16Tmp;
        ps16Tmp ++;
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_AO_IaaApc_Config_Printf(
                                        AudioAnrConfig *anr_config,
                                        AudioEqConfig *eq_config,
                                        AudioHpfConfig *hpf_config,
                                        AudioVadConfig *vad_config,
                                        AudioDereverbConfig *dereverb_config,
                                        AudioAgcConfig *agc_config
                                        )
{
    MI_S32 s32i= 0;
    MI_PRINT("###########Vqe arguments start##########\n");

    //anr
    MI_PRINT("=========anr==============\n");
    MI_PRINT("mode = %d\n", anr_config->user_mode);
    MI_PRINT("enabel = %d\n", anr_config->anr_enable);
    MI_PRINT("eNrSpeed = %d\n", anr_config->anr_converge_speed);
    MI_PRINT("u32NrIntensity = %d\n", anr_config->anr_intensity);
    MI_PRINT("u32NrSmoothLevel = %d\n", anr_config->anr_smooth_level);

    //eq
    MI_PRINT("===============eq==========\n");
    MI_PRINT("mode = %d\n", eq_config->user_mode);
    MI_PRINT("enable = %d\n", eq_config->eq_enable);
    for (s32i = 0; s32i < MI_AO_APC_EQ_BAND_NUM; s32i++)
    {
        MI_PRINT("db[%d] = %d\n", s32i, eq_config->eq_gain_db[s32i]);
    }

    MI_PRINT("===========hpf==================\n");
    MI_PRINT("mode = %d\n", hpf_config->user_mode);
    MI_PRINT("enabel = %d\n", hpf_config->hpf_enable);
    MI_PRINT("eHpfFreq = %d\n", hpf_config->cutoff_frequency);

    MI_PRINT("=========agc==================\n");
    MI_PRINT("mode = %d\n", agc_config->user_mode);
    MI_PRINT("enable = %d\n", agc_config->agc_enable);
    MI_PRINT("u32AttackTime = %d\n", agc_config->attack_time);
    MI_PRINT("u32ReleaseTime = %d\n", agc_config->release_time);
    MI_PRINT("u32CompressionRatio = %d\n", agc_config->compression_ratio);
    MI_PRINT("u32DropGainMax = %d\n", agc_config->drop_gain_max);
    MI_PRINT("stAgcGainInfo.S32GainInit = %d\n", agc_config->gain_info.gain_init);
    MI_PRINT("stAgcGainInfo.s32GainMax = %d\n", agc_config->gain_info.gain_max);
    MI_PRINT("stAgcGainInfo.S32GainMin = %d\n", agc_config->gain_info.gain_min);
    MI_PRINT("u32NoiseGateAttenuationDb = %d\n", agc_config->noise_gate_attenuation_db);
    MI_PRINT("s32NoiseGateDb = %d\n", agc_config->noise_gate_db);
    MI_PRINT("s32TargetLevelDb = %d\n", agc_config->target_level_db);

    MI_PRINT("###########Vqe arguments End##########\n");
    return MI_SUCCESS;
}

static MI_S32 _MI_AO_IaaApc_Print(APC_HANDLE hApc)
{
    MI_S32 s32Ret = MI_SUCCESS;
    AudioProcessInit  stGetApcInfo;
    AudioAnrConfig stGetAnrInfo;
    AudioEqConfig   stGetEqInfo;
    AudioHpfConfig  stGetHpfInfo;
    AudioVadConfig  stGetVadInfo;
    AudioDereverbConfig stGetDeverbInfo;
    AudioAgcConfig  stGetAgcInfo;

    s32Ret = IaaApc_GetConfig(hApc, &stGetApcInfo, &stGetAnrInfo, &stGetEqInfo, &stGetHpfInfo, &stGetVadInfo, &stGetDeverbInfo, &stGetAgcInfo);
    if (0 != s32Ret)
    {
        MI_PRINT("IaaApc_GetConfig failed !!!!!\n");
        return MI_AO_ERR_VQE_ERR;
    }

    _MI_AO_IaaApc_Config_Printf(&stGetAnrInfo, &stGetEqInfo, &stGetHpfInfo, NULL, NULL, &stGetAgcInfo);

    return MI_SUCCESS;
}
#endif

static MI_S32 _MI_AO_VqeInit(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

#ifndef __KERNEL__
    AudioProcessInit stApstruct;
    APC_HANDLE hApcHandle;
    MI_U16 u16ChanlNum;
    MI_AO_VqeConfig_t stAoVqeConfig;

    AudioAnrConfig stAnrInfo;
    AudioEqConfig  stEqInfo;
    AudioHpfConfig stHpfInfo;

    AudioVadConfig stVadInfo;
    AudioDereverbConfig stDereverbInfo;
    AudioAgcConfig stAgcInfo;

    //Apc init arguments
    memset(&stApstruct, 0, sizeof(AudioProcessInit));
    stApstruct.point_number = MI_AUDIO_VQE_SAMPLES_UNIT;
    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAoDevInfoUsr[AoDevId].stDevAttr.eSoundmode);
    stApstruct.channel = u16ChanlNum;
    MI_AUDIO_VQE_SAMPLERATE_TRANS_TYPE(_gastAoDevInfoUsr[AoDevId].stDevAttr.eSamplerate, stApstruct.sample_rate);

    /* APC init */
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pApcWorkingBuf = MI_AO_Malloc(IaaApc_GetBufferSize());
    if (NULL == _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pApcWorkingBuf)
    {
        DBG_ERR("Malloc IaaApc_GetBuffer failed\n");
        return MI_AO_ERR_NOBUF;
    }
    else
    {
        MI_PRINT("Malloc IaaApc_GetBuffer ok\n");
    }
    hApcHandle  = IaaApc_Init((char* const)_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pApcWorkingBuf, &stApstruct);
    if(hApcHandle == NULL)
    {
        MI_PRINT("IaaApc_Init FAIL !!!!!!!!!!!!!!!!!!!\n");
        return MI_AO_ERR_VQE_ERR;
    }
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hApcHandle = hApcHandle;

    stAoVqeConfig =  _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoVqeConfig;

    //set Anr
    stAnrInfo.anr_enable = (MI_U32)stAoVqeConfig.bAnrOpen;
    stAnrInfo.user_mode = (MI_U32)stAoVqeConfig.stAnrCfg.bUsrMode;
     MI_AUDIO_VQE_NR_SPEED_TRANS_TYPE(stAoVqeConfig.stAnrCfg.eNrSpeed, stAnrInfo.anr_converge_speed);
    stAnrInfo.anr_intensity = stAoVqeConfig.stAnrCfg.u32NrIntensity;
    stAnrInfo.anr_smooth_level = stAoVqeConfig.stAnrCfg.u32NrSmoothLevel;

    //set Eq
    stEqInfo.eq_enable = (MI_U32)stAoVqeConfig.bEqOpen;
    stEqInfo.user_mode = (MI_U32)stAoVqeConfig.stEqCfg.bUsrMode;
    s32Ret = _MI_AO_SetEqGainDb(&stAoVqeConfig.stEqCfg.stEqGain, stEqInfo.eq_gain_db, MI_AO_APC_EQ_BAND_NUM);

    //set Hpf
    stHpfInfo.hpf_enable = (MI_U32)stAoVqeConfig.bHpfOpen;
    stHpfInfo.user_mode = (MI_U32)stAoVqeConfig.stHpfCfg.bUsrMode;
    MI_AUDIO_VQE_HPF_TRANS_TYPE(stAoVqeConfig.stHpfCfg.eHpfFreq, stHpfInfo.cutoff_frequency);

    //set Vad
    stVadInfo.vad_enable = 0;
    stVadInfo.user_mode = 1;
    stVadInfo.vad_threshold = -10;

    //set De-reverberation
    stDereverbInfo.dereverb_enable = 0;

    //set Agc
    stAgcInfo.agc_enable = (MI_U32)stAoVqeConfig.bAgcOpen;
    stAgcInfo.user_mode = (MI_U32)stAoVqeConfig.stAgcCfg.bUsrMode;
    stAgcInfo.attack_time = stAoVqeConfig.stAgcCfg.u32AttackTime;
    stAgcInfo.release_time = stAoVqeConfig.stAgcCfg.u32ReleaseTime;
    stAgcInfo.compression_ratio = stAoVqeConfig.stAgcCfg.u32CompressionRatio;
    stAgcInfo.drop_gain_max = stAoVqeConfig.stAgcCfg.u32DropGainMax;
    stAgcInfo.gain_info.gain_init = stAoVqeConfig.stAgcCfg.stAgcGainInfo.s32GainInit;
    stAgcInfo.gain_info.gain_max = stAoVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMax;
    stAgcInfo.gain_info.gain_min = stAoVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMin;
    stAgcInfo.noise_gate_attenuation_db = stAoVqeConfig.stAgcCfg.u32NoiseGateAttenuationDb;
    stAgcInfo.noise_gate_db = stAoVqeConfig.stAgcCfg.s32NoiseGateDb;
    stAgcInfo.target_level_db = stAoVqeConfig.stAgcCfg.s32TargetLevelDb;

                                                                       // Vad   dereverb is not set
    s32Ret = IaaApc_Config(hApcHandle, &stAnrInfo, &stEqInfo, &stHpfInfo, NULL, NULL, &stAgcInfo);
    if (0 != s32Ret)
    {
        MI_PRINT("IaaPac_config FAIL !!!!!!!!!!!!!!!!!!!\n");
        return MI_AO_ERR_VQE_ERR;
    }

    _MI_AO_IaaApc_Print(hApcHandle);
#endif

    return s32Ret;

}

MI_S32 _MI_AO_G726Init(g726_state_t *pstG726Info, MI_S32 s32BitRate)
{
    MI_AO_USR_CHECK_POINTER(pstG726Info);

#ifndef __KERNEL__
    g726_init(pstG726Info, s32BitRate);
#endif

    return MI_SUCCESS;
}

static MI_S32 _MI_AO_AdecInit(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_AdecConfig_t stAoAdecConfig;
    g726_state_t *pstG726Info = NULL;

    stAoAdecConfig = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoAdecConfig;

    if (E_MI_AUDIO_ADEC_TYPE_G726 == stAoAdecConfig.eAdecType)
    {
#ifndef __KERNEL__
        pstG726Info = MI_AO_Malloc(sizeof(g726_state_t));
#endif
        MI_AO_USR_CHECK_POINTER(pstG726Info);
        memset(pstG726Info, 0, sizeof(g726_state_t));

        switch(stAoAdecConfig.stAdecG726Cfg.eG726Mode)
        {
            case E_MI_AUDIO_G726_MODE_16:
                _MI_AO_G726Init(pstG726Info, 8000 * 2);
                DBG_WRN("_MI_AO_G726Init 16k\n");
                break;
            case E_MI_AUDIO_G726_MODE_24:
                _MI_AO_G726Init(pstG726Info, 8000 * 3);
                DBG_WRN("_MI_AO_G726Init 23k\n");
                break;
            case E_MI_AUDIO_G726_MODE_32:
                _MI_AO_G726Init(pstG726Info, 8000 * 4);
                DBG_WRN("_MI_AO_G726Init 32k\n");
                break;
            case E_MI_AUDIO_G726_MODE_40:
                _MI_AO_G726Init(pstG726Info, 8000 * 5);
                DBG_WRN("_MI_AO_G726Init 40k\n");
                break;
            default:
                DBG_WRN("G726 Mode is not find:%d\n", stAoAdecConfig.stAdecG726Cfg.eG726Mode);
#ifndef __KERNEL__
                MI_AO_Free(pstG726Info);
#endif
            return E_MI_AO_ADEC_ERR;
        }
    }

    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hG72Handle = pstG726Info;

    return s32Ret;
}

static MI_S32 _MI_AO_QueueInit(_MI_AO_QueueInfo_t *pstQueue, MI_S32 s32Size)
{
    MI_AO_USR_CHECK_POINTER(pstQueue);

#ifndef __KERNEL__
    pstQueue->pu8Buff = (MI_U8 *)MI_AO_Malloc(s32Size);
    MI_AO_USR_CHECK_POINTER(pstQueue->pu8Buff);
#endif

    if (s32Size < 0)
    {
        s32Size = 0;
    }

    pstQueue->s32Front = 0;
    pstQueue->s32Rear = 0;
    pstQueue->s32Size = 0;
    pstQueue->s32Max = s32Size;

    return MI_SUCCESS;
}

static MI_S32 _MI_AO_QueueDeinit(_MI_AO_QueueInfo_t *pstQueue)
{
    MI_AO_USR_CHECK_POINTER(pstQueue);
    MI_AO_USR_CHECK_POINTER(pstQueue->pu8Buff);

#ifndef __KERNEL__
    MI_AO_Free(pstQueue->pu8Buff);
    pstQueue->pu8Buff = NULL;
#endif
    pstQueue->s32Front = 0;
    pstQueue->s32Rear = 0;
    pstQueue->s32Size = 0;
    pstQueue->s32Max = 0;

    return MI_SUCCESS;
}

static MI_S32 _MI_AO_QueueClear(_MI_AO_QueueInfo_t *pstQueue)
{
    if (NULL != pstQueue)
    {
        pstQueue->s32Front = 0;
        pstQueue->s32Rear = 0;
        pstQueue->s32Size = 0;
    }

    return MI_SUCCESS;
}


static MI_S32 _MI_AO_QueueInsert(_MI_AO_QueueInfo_t *pstQueue, MI_U8 *pu8InputBuff, MI_S32 s32Size)
{
    MI_S32 s32Tmp = 0;

    MI_AO_USR_CHECK_POINTER(pstQueue);
    MI_AO_USR_CHECK_POINTER(pstQueue->pu8Buff);
    MI_AO_USR_CHECK_POINTER(pu8InputBuff);

    if (s32Size < 0)
    {
        s32Size = 0;
    }

    s32Tmp = pstQueue->s32Size + s32Size;
    if (s32Tmp > pstQueue->s32Max)
    {
        DBG_WRN("Queue has not enough space\n");
        DBG_WRN("Input Size:%d, Max Size:%d,Remain Size:%d\n", s32Size, pstQueue->s32Max, pstQueue->s32Max- pstQueue->s32Size);
        return MI_AO_ERR_NOBUF;
    }

    if (pstQueue->s32Front + s32Size > pstQueue->s32Max)
    {
        s32Tmp = pstQueue->s32Max - pstQueue->s32Front;
        memcpy(pstQueue->pu8Buff + pstQueue->s32Front, pu8InputBuff, s32Tmp);
        memcpy(pstQueue->pu8Buff, pu8InputBuff + s32Tmp, s32Size - s32Tmp);
    }
    else
    {
        memcpy(pstQueue->pu8Buff + pstQueue->s32Front, pu8InputBuff, s32Size);
    }

    pstQueue->s32Front = (pstQueue->s32Front + s32Size) % pstQueue->s32Max;
    pstQueue->s32Size += s32Size;

    return MI_SUCCESS;
}

static MI_S32 _MI_AO_QueueDraw(_MI_AO_QueueInfo_t *pstQueue, MI_U8 *pu8OutputBuff, MI_S32 s32Size)
{
    MI_S32 s32Tmp = 0;

    MI_AO_USR_CHECK_POINTER(pstQueue)
    MI_AO_USR_CHECK_POINTER(pu8OutputBuff)

    if (s32Size < 0)
    {
        s32Size = 0;
    }

    s32Tmp = pstQueue->s32Size - s32Size;
    if (s32Tmp < 0)
    {
        DBG_WRN("Queue has not enough data\n");
        DBG_WRN("Input Size:%d, Remain data Size:%d\n", s32Size, pstQueue->s32Size);
        return MI_AO_ERR_BUF_EMPTY;
    }

    if (pstQueue->s32Rear + s32Size > pstQueue->s32Max)
    {
        s32Tmp = pstQueue->s32Max - pstQueue->s32Rear;
        memcpy(pu8OutputBuff,pstQueue->pu8Buff +  pstQueue->s32Rear, s32Tmp);
        memcpy(pu8OutputBuff + s32Tmp, pstQueue->pu8Buff, s32Size - s32Tmp);
    }
    else
    {
        memcpy(pu8OutputBuff, pstQueue->pu8Buff + pstQueue->s32Rear, s32Size);
    }

    pstQueue->s32Rear = (pstQueue->s32Rear + s32Size) % pstQueue->s32Max;
    pstQueue->s32Size -= s32Size;

    return MI_SUCCESS;
}

static MI_S32 _MI_AO_DoVqe(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AUDIO_Frame_t *pstData, _MI_AO_QueueInfo_t *pstInputQueue, _MI_AO_QueueInfo_t *pstOutputQueue)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32GetSize;
    MI_S32 s32BitwidthByte;
    MI_U16 u16ChanlNum;
    MI_U8 *pu8InputBuff = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8InputBuff;
    APC_HANDLE hApcHandle;

    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(s32BitwidthByte, pstData->eBitwidth);
    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, pstData->eSoundmode);

    s32GetSize = MI_AUDIO_VQE_SAMPLES_UNIT * s32BitwidthByte * u16ChanlNum;
    hApcHandle = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hApcHandle;

    if (pstInputQueue->s32Size < s32GetSize)
    {
        return s32Ret;
    }

    while(pstInputQueue->s32Size >= s32GetSize)
    {
        memset(pu8InputBuff, 0, MI_AO_POINT_NUM_MAX);

        s32Ret = _MI_AO_QueueDraw(pstInputQueue, pu8InputBuff, s32GetSize);
        if (MI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

#ifndef __KERNEL__
        s32Ret = IaaApc_Run(hApcHandle, (MI_S16*)(pu8InputBuff));
#endif
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("IaaApc_Run failed\n");
            return MI_AO_ERR_VQE_ERR;
        }

        s32Ret = _MI_AO_QueueInsert(pstOutputQueue, pu8InputBuff, s32GetSize);
    }

    return s32Ret;
}

static MI_S32 _MI_AO_G711Decoder(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, _MI_AO_QueueInfo_t *pstInputQueue, _MI_AO_QueueInfo_t *pstOutputQueue, MI_AUDIO_AdecType_e eG711Type)
{
    MI_S32 s32GetSize= 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8InputBuff = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8InputBuff;
    MI_U8 *pu8OutputBuff = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8OutputBuff;

    memset(pu8InputBuff, 0, MI_AO_POINT_NUM_MAX);
    memset(pu8OutputBuff, 0, MI_AO_POINT_NUM_MAX);

    //g711 decoder
    if (pstInputQueue->s32Size < MI_AO_POINT_NUM_MAX / 2)
    {
        s32GetSize = pstInputQueue->s32Size;
    }
    else
    {
        s32GetSize =  MI_AO_POINT_NUM_MAX / 2;
    }

    s32Ret = _MI_AO_QueueDraw(pstInputQueue, pu8InputBuff, s32GetSize);
    if (s32Ret != MI_SUCCESS)
    {
        return s32Ret;
    }

#ifndef __KERNEL__
    switch(eG711Type)
    {
        case E_MI_AUDIO_ADEC_TYPE_G711A:
            G711Decoder((MI_S16 *)pu8OutputBuff, pu8InputBuff, s32GetSize, MI_AO_G711A);
            break;
        case E_MI_AUDIO_ADEC_TYPE_G711U:
            G711Decoder((MI_S16 *)pu8OutputBuff, pu8InputBuff, s32GetSize, MI_AO_G711U);
            break;
        default:
            DBG_WRN("G711 Decoder Type not find:%d\n", eG711Type);
            return MI_AO_ERR_NOT_PERM;
    }
#endif
    s32Ret = _MI_AO_QueueInsert(pstOutputQueue, pu8OutputBuff, s32GetSize * 2);

    return s32Ret;
}

static MI_S32 _MI_AO_G726Decoder(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, _MI_AO_QueueInfo_t *pstInputQueue, _MI_AO_QueueInfo_t *pstOutputQueue, MI_AUDIO_G726Mode_e eG726Mode)
{
    MI_S32 s32WriteSize = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32AdecUnit = _gastAoDevInfoUsr[AoDevId].stDevAttr.u32PtNumPerFrm;
    MI_U8 *pu8InputBuff = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8InputBuff;
    MI_U8 *pu8OutputBuff = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8OutputBuff;
    g726_state_t *hG726Handle = NULL;

    //g726_16  1bytes->8bytes  4short
    //g726_24  3bytes->16bytes 8short
    //g726_32  1bytes->4bytes  2short
    //g726_40  5bytes->16bytes 8short

    if (u32AdecUnit < MI_AO_ADEC_G726_UNIT)
    {
        u32AdecUnit = MI_AO_ADEC_G726_UNIT;
    }
    else if (u32AdecUnit > MI_AO_ADEC_G726_UNIT_MAX)
    {
        u32AdecUnit = MI_AO_ADEC_G726_UNIT_MAX;
    }else
    {
        u32AdecUnit = u32AdecUnit - (u32AdecUnit % MI_AO_ADEC_G726_UNIT);
    }

    if (pstInputQueue->s32Size < u32AdecUnit)
    {
        return MI_SUCCESS;
    }

    memset(pu8InputBuff, 0, MI_AO_POINT_NUM_MAX);
    memset(pu8OutputBuff, 0, MI_AO_POINT_NUM_MAX);

    s32Ret = _MI_AO_QueueDraw(pstInputQueue, pu8InputBuff, u32AdecUnit);
    if (MI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    hG726Handle = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hG72Handle;
#ifndef __KERNEL__
    s32WriteSize = g726_decode(hG726Handle, (MI_S16 *)pu8OutputBuff, pu8InputBuff, u32AdecUnit);
#endif

    if (s32WriteSize <= 0)
    {
        DBG_WRN("s32WriteSize:%d\n", s32WriteSize);
        return E_MI_AO_ADEC_ERR;
    }

    if (s32WriteSize * 2 > MI_AO_POINT_NUM_MAX)
    {
       DBG_WRN("g726 decode size is too big:%d\n", s32WriteSize);
       return E_MI_AO_ADEC_ERR;
    }

    s32Ret = _MI_AO_QueueInsert(pstOutputQueue, pu8OutputBuff, s32WriteSize * 2);

    return s32Ret;
}

static MI_S32 _MI_AO_DoAdec(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, _MI_AO_QueueInfo_t *pstInputQueue, _MI_AO_QueueInfo_t *pstOutputQueue, MI_AO_AdecConfig_t *pstAoAdecInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;

    switch(pstAoAdecInfo->eAdecType)
    {
        case E_MI_AUDIO_ADEC_TYPE_G711A:
        case E_MI_AUDIO_ADEC_TYPE_G711U:
            s32Ret = _MI_AO_G711Decoder(AoDevId, AoChn, pstInputQueue, pstOutputQueue, pstAoAdecInfo->eAdecType);
            break;
        case E_MI_AUDIO_ADEC_TYPE_G726:
            s32Ret = _MI_AO_G726Decoder(AoDevId, AoChn, pstInputQueue, pstOutputQueue, pstAoAdecInfo->stAdecG726Cfg.eG726Mode);
            break;
        default:
            DBG_WRN("AdecType is not find:%d\n", pstAoAdecInfo->eAdecType);
            s32Ret = MI_AO_ERR_NOT_PERM;
            break;
    }

    return s32Ret;
}

static MI_S32 _MI_AO_DoSrc(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AUDIO_Frame_t *pstData, _MI_AO_QueueInfo_t *pstInputQueue, _MI_AO_QueueInfo_t *pstOutputQueue)
{
    MI_S32 s32GetSize= 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8InputBuff = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8InputBuff;
    MI_U8 *pu8OutputBuff = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8OutputBuff;

    MI_S32 s32BitwidthByte;
    MI_U16 u16ChanlNum;
    SRC_HANDLE hSrcHandle;
    MI_U32 u32SrcUnit;
    MI_S32 s32SrcOutSample;

    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(s32BitwidthByte, pstData->eBitwidth);
    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, pstData->eSoundmode);

    hSrcHandle = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hSrcHandle;
    u32SrcUnit = _gastAoDevInfoUsr[AoDevId].stDevAttr.u32PtNumPerFrm;

    if (u32SrcUnit < MI_AO_SRC_UNIT)
    {
        u32SrcUnit = MI_AO_SRC_UNIT;
    }
    else if (u32SrcUnit > MI_AO_SRC_UNIT_MAX)
    {
        u32SrcUnit = MI_AO_SRC_UNIT_MAX;
    }
    else
    {
        u32SrcUnit = u32SrcUnit - (u32SrcUnit % MI_AO_SRC_UNIT);
    }

    s32GetSize = u32SrcUnit * s32BitwidthByte * u16ChanlNum;

    while(pstInputQueue->s32Size > s32GetSize)
    {
        memset(pu8InputBuff, 0, MI_AO_POINT_NUM_MAX);
        memset(pu8OutputBuff, 0, MI_AO_POINT_NUM_MAX);

        s32Ret = _MI_AO_QueueDraw(pstInputQueue, pu8InputBuff, s32GetSize);
        if (MI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        s32SrcOutSample = 0;
#ifndef __KERNEL__
        s32SrcOutSample = IaaSrc_Run(hSrcHandle, (MI_S16 *)pu8InputBuff, (MI_S16 *)pu8OutputBuff, u32SrcUnit);
#endif
        if (s32SrcOutSample * s32BitwidthByte * u16ChanlNum > MI_AO_POINT_NUM_MAX)
        {
            DBG_ERR("Src out size is too big:%d\n", s32SrcOutSample * s32BitwidthByte * u16ChanlNum);
            return MI_AO_ERR_NOBUF;
        }

        s32Ret =_MI_AO_QueueInsert(pstOutputQueue, pu8OutputBuff, s32SrcOutSample * s32BitwidthByte * u16ChanlNum);
        if (MI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }
    }


    return MI_SUCCESS;
}

static MI_S32 _MI_AO_SendToChnInputPort(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, _MI_AO_QueueInfo_t *pstInputQueue , MI_AUDIO_Frame_t *pstData, MI_S32 s32MilliSec, MI_BOOL bReturnErr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32BitwidthByte;
    MI_S32 s32GetSize;
    MI_U16 u16ChanlNum;
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hBufHandle;
    MI_U32 u32NumPerFrm;
    MI_U64 u64StartTime;
    MI_U64 u64CurrentTime = 0;
    MI_AO_QueueStatus_t stQueueStatusInfo;

#ifndef __KERNEL__
    struct  timeval tv;
#endif

    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(s32BitwidthByte, pstData->eBitwidth);
    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, pstData->eSoundmode);

    u32NumPerFrm = _gastAoDevInfoUsr[AoDevId].stDevAttr.u32PtNumPerFrm;

    s32GetSize = u32NumPerFrm * s32BitwidthByte * u16ChanlNum;

    stChnPort.eModId = E_MI_MODULE_ID_AO;
    stChnPort.u32DevId = AoDevId;
    stChnPort.u32ChnId = AoChn;
    stChnPort.u32PortId = 0;  //input port id

    while( (pstInputQueue->s32Size) >= s32GetSize)
    {
        stBufConf.eBufType = E_MI_SYS_BUFDATA_RAW;
        stBufConf.u64TargetPts = pstData->u64TimeStamp;
        stBufConf.stRawCfg.u32Size = s32GetSize;
        stBufConf.u32Flags = 0;

        s32Ret = MI_SYS_ChnInputPortGetBuf(&stChnPort, &stBufConf, &stBufInfo, &hBufHandle , s32MilliSec);
        if (s32Ret != MI_SUCCESS)
        {
            if (TRUE == bReturnErr)
            {
               return MI_AO_ERR_NOBUF;
            }
            else
            {
                return MI_SUCCESS;
            }
        }

        _MI_AO_QueueDraw(pstInputQueue, stBufInfo.stRawData.pVirAddr, s32GetSize);
        stBufInfo.stRawData.u32ContentSize  = stBufConf.stRawCfg.u32Size ;

        s32Ret =  MI_SYS_ChnInputPortPutBuf(hBufHandle, &stBufInfo, FALSE);
        if(s32Ret!= MI_SUCCESS)
        {
            DBG_WRN("Put buff failed\n");
            return  MI_AO_ERR_BUF_FULL;
        }

        u64StartTime = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].u64StartTime;

        stQueueStatusInfo.AoDevId = AoDevId;
        stQueueStatusInfo.AoChn = AoChn;
        stQueueStatusInfo.s32Max = pstInputQueue->s32Max;
        stQueueStatusInfo.s32Size = pstInputQueue->s32Size;
        stQueueStatusInfo.u64GetTotalSize = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].u64GetTotalSize;
#ifndef __KERNEL__
        gettimeofday(&tv, NULL);
        u64CurrentTime = tv.tv_sec;
#endif
        stQueueStatusInfo.u64RunTime = u64CurrentTime - u64StartTime ;

        s32Ret = MI_SYSCALL(MI_AO_UPDATE_QUEUE_STATUS, &stQueueStatusInfo);
    }

    return s32Ret;
}

#ifndef __KERNEL__
static void *_MI_AO_SendData(void *pdata)
{
    MI_S32 s32Ret = MI_SUCCESS;
    _MI_AO_SendDataInfo_t *pstSendDataInfo = (_MI_AO_SendDataInfo_t *) pdata;
    MI_AUDIO_DEV AoDevId = pstSendDataInfo->AoDevId;
    MI_AO_CHN    AoChn = pstSendDataInfo->AoChn;
    _MI_AO_QueueInfo_t * pstChnOutputQueue = &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stChnOutputQueue;
    MI_AUDIO_Frame_t stData;

    MI_U64 u64CurrentTime = 0;

#ifndef __KERNEL__
        struct  timeval tv;
#endif

    memset(&stData, 0, sizeof(MI_AUDIO_Frame_t));
    stData.eBitwidth = _gastAoDevInfoUsr[AoDevId].stDevAttr.eBitwidth;
    stData.eSoundmode = _gastAoDevInfoUsr[AoDevId].stDevAttr.eSoundmode;
    stData.u64TimeStamp = 0;

    while (_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAoPthreadExit != TRUE)
    {

#ifndef __KERNEL__
        gettimeofday(&tv, NULL);
        u64CurrentTime = tv.tv_sec;
#endif

#ifndef __KERNEL__
        pthread_mutex_lock(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoMutex);
#endif

        //s32Ret =  _MI_AO_SendToChnInputPort(AoDevId, AoChn, pstChnOutputQueue, &stData, 1, FALSE);
        //if (MI_SUCCESS == s32Ret)
        //{
        //    MI_PRINT("Thread Time:%lld, Yes\n", u64CurrentTime - _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].u64StartTime);
        //}
        //else
        //{
        //    MI_PRINT("Thread Time:%lld, No\n", u64CurrentTime - _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].u64StartTime);
        //}

#ifndef __KERNEL__
        pthread_mutex_unlock(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoMutex);
#endif
        usleep(50 * 1000);
        //MI_PRINT("11\n");
    }
}
#endif


//-------------------------------------------------------------------------------------------------
//  global function  prototypes
//-------------------------------------------------------------------------------------------------
MI_S32 MI_AO_SetPubAttr(MI_AUDIO_DEV AoDevId, MI_AUDIO_Attr_t *pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetPubAttr_t stSetPubAttr;

    memset(&stSetPubAttr, 0, sizeof(stSetPubAttr));
    stSetPubAttr.AoDevId = AoDevId;
    memcpy(&stSetPubAttr.stPubAttr, pstAttr, sizeof(MI_AUDIO_Attr_t));

    s32Ret = MI_SYSCALL(MI_AO_SET_PUB_ATTR, &stSetPubAttr);

    // save attribute of AO device for user mode
   if(s32Ret == MI_SUCCESS)
   {
        _gastAoDevInfoUsr[AoDevId].bDevAttrSet = TRUE;
        memcpy(&_gastAoDevInfoUsr[AoDevId].stDevAttr, pstAttr, sizeof(MI_AUDIO_Attr_t));
    }

    return s32Ret;
}


MI_S32 MI_AO_GetPubAttr(MI_AUDIO_DEV AoDevId, MI_AUDIO_Attr_t *pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_GetPubAttr_t stGetPubAttr;

    memset(&stGetPubAttr, 0, sizeof(stGetPubAttr));
    stGetPubAttr.AoDevId = AoDevId;
    s32Ret = MI_SYSCALL(MI_AO_GET_PUB_ATTR, &stGetPubAttr);
    if (s32Ret == MI_SUCCESS)
    {
        memcpy(pstAttr, &stGetPubAttr.stPubAttr, sizeof(MI_AUDIO_Attr_t));
    }

    return s32Ret;
}

MI_S32 MI_AO_Enable(MI_AUDIO_DEV AoDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret = MI_SYSCALL(MI_AO_ENABLE, &AoDevId);
    if(s32Ret == MI_SUCCESS)
    {
        _gastAoDevInfoUsr[AoDevId].bDevEnable = TRUE;
    }

    return s32Ret;
}

MI_S32 MI_AO_Disable(MI_AUDIO_DEV AoDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_AO_DISABLE, &AoDevId);

    if(s32Ret == MI_SUCCESS)
    {
        _gastAoDevInfoUsr[AoDevId].bDevEnable = FALSE;
    }

    return s32Ret;
}

MI_S32 MI_AO_EnableChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_EnableChn_t stEnableChn;
    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;
#ifndef __KERNEL__
    struct  timeval tv;
#endif

    memset(&stEnableChn, 0, sizeof(stEnableChn));
    stEnableChn.AoDevId = AoDevId;
    stEnableChn.AoChn = AoChn;

    if (TRUE == _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable)
    {
        return MI_SUCCESS;
    }

    s32Ret = MI_SYSCALL(MI_AO_ENABLE_CHN, &stEnableChn);

    if(s32Ret == MI_SUCCESS)
    {
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable = TRUE;
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].s32ChnId = AoChn;
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bPortEnable = TRUE;
    }

     /* SRC parameter setting */

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAoDevInfoUsr[AoDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAoDevInfoUsr[AoDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAoDevInfoUsr[AoDevId].stDevAttr.u32PtNumPerFrm;

     //5, G726 Max 5 byte alignment
     //8, G726_16, 2bit convert 16 bit
     //6, Res 8K->48K
    _MI_AO_QueueInit(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stChnOutputQueue, u32NumPerFrm * u32BitWidthByte * u16ChanlNum * 5 * 8 * 6);
#ifndef __KERNEL__
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8InputBuff = MI_AO_Malloc(MI_AO_POINT_NUM_MAX);
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8OutputBuff = MI_AO_Malloc(MI_AO_POINT_NUM_MAX);
    MI_AO_USR_CHECK_POINTER(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8InputBuff);
    MI_AO_USR_CHECK_POINTER(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8OutputBuff);

    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoSendDataInfo.AoDevId = AoDevId;
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoSendDataInfo.AoChn = AoChn;

    pthread_mutex_init(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoMutex, NULL);

    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAoPthreadExit = FALSE;
    s32Ret = pthread_create(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoPthreadId, NULL, _MI_AO_SendData, &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoSendDataInfo);
    if (0 != s32Ret)
    {
        DBG_WRN("create Ao:%d Chn:%d pthread failed\n", AoDevId, AoChn);
        return MI_AO_ERR_BUSY;
    }

    gettimeofday(&tv, NULL);
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].u64StartTime = tv.tv_sec ;
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].u64GetTotalSize = 0;
#endif

    return s32Ret;
}

MI_S32 MI_AO_DisableChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_DisableChn_t stDisableChn;

    memset(&stDisableChn, 0, sizeof(stDisableChn));
    stDisableChn.AoDevId = AoDevId;
    stDisableChn.AoChn = AoChn;

    if (FALSE == _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable)
    {
        return MI_SUCCESS;
    }

    s32Ret = MI_SYSCALL(MI_AO_DISABLE_CHN, &stDisableChn);

    if(s32Ret == MI_SUCCESS)
   {
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable = FALSE;
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bPortEnable = FALSE;
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeEnable = FALSE;
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bResampleEnable = FALSE;
        _MI_AO_QueueDeinit(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stChnOutputQueue);

        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAoPthreadExit = TRUE;
#ifndef __KERNEL__
        MI_AO_Free(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8InputBuff);
        MI_AO_Free(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8OutputBuff);
        pthread_join(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoPthreadId, NULL);
        pthread_mutex_destroy(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoMutex);
#endif

    }
    return s32Ret;
}

MI_S32 MI_AO_SendFrame(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AUDIO_Frame_t *pstData, MI_S32 s32MilliSec)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_S32 s32BitwidthByte;
    MI_U16 u16ChanlNum;
    MI_U32  u32LenPerChanl;
    MI_U64 u64GetTotalSize;

    MI_AO_AdecConfig_t *pstAoAdecInfo = NULL;

    _MI_AO_QueueInfo_t *pstAdecInputQueue = NULL;
    _MI_AO_QueueInfo_t *pstChnOutputQueue = NULL;
    _MI_AO_QueueInfo_t *pstSrcInputQueue = NULL;
    _MI_AO_QueueInfo_t *pstVqeInputQueue = NULL;

    MI_BOOL bResampleEnable = FALSE;
    MI_BOOL bAdecEnable = FALSE;
    MI_BOOL bVqeEnable = FALSE;

    ///check input parameter
    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);
    MI_AO_USR_CHECK_POINTER(pstData);

    if(TRUE != _gastAoDevInfoUsr[AoDevId].bDevEnable)
        return MI_AO_ERR_NOT_ENABLED;

    if(TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(s32BitwidthByte, pstData->eBitwidth);
    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, pstData->eSoundmode);

    u32LenPerChanl = pstData->u32Len;

    bResampleEnable = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bResampleEnable;
    bAdecEnable = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAdecEnable;
    bVqeEnable  = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeEnable;

    pstAdecInputQueue = &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAdecInputQueue;
    pstChnOutputQueue = &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stChnOutputQueue;
    pstSrcInputQueue = &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stSrcInputQueue;
    pstVqeInputQueue = &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stVqeInputQueue;
    pstAoAdecInfo =  &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoAdecConfig;

    //process remained data
    //s32Ret = _MI_AO_SendToChnInputPort(AoDevId, AoChn, pstChnOutputQueue, pstData, s32MilliSec, TRUE);
    //if (MI_SUCCESS != s32Ret)
    //{
    //    return s32Ret;
    //}

    /*******  data flow  ********/
    /*
        1, decoder + resample + vqe
            AdecInputQueue->SrcInputQueue->VqeInputQueue->ChnOutputQueue
        2, decoder + resample
            AdecInputQueue->SrcInputQuque->ChnOutputQueue
        3, decoder + vqe
            AdecInputQueue->VqeInputQueue->ChnOutputQueue
        4, decoder
            AdedInputQuque->ChnOutputQueue
        5, resample + vqe
            SrcInputQueue->VqeInputQueue->ChnOutputQueue
        6, resample
            SrcInputQueue->ChnOutputQueue
        7, vqe
            VqeInputQueue->ChnOutputQueue
        8, cpm data
            cpm->ChnOutputQueue
    */
#ifndef __KERNEL__
        pthread_mutex_lock(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoMutex);
#endif

    if (bAdecEnable)
    {
        s32Ret = _MI_AO_QueueInsert(pstAdecInputQueue, pstData->apVirAddr[0], u32LenPerChanl);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_QueueInsert failed\n");
            return s32Ret;
        }
    }
    else if (bResampleEnable)
    {
        s32Ret = _MI_AO_QueueInsert(pstSrcInputQueue, pstData->apVirAddr[0], u32LenPerChanl);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_QueueInsert failed\n");
            return s32Ret;
        }
    }
    else if (bVqeEnable)
    {
        s32Ret = _MI_AO_QueueInsert(pstVqeInputQueue, pstData->apVirAddr[0], u32LenPerChanl);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_QueueInsert failed\n");
            return s32Ret;
        }
    }
    else
    {
        s32Ret = _MI_AO_QueueInsert(pstChnOutputQueue, pstData->apVirAddr[0], u32LenPerChanl);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_QueueInsert failed\n");
            return s32Ret;
        }
    }

    if (bAdecEnable && bResampleEnable)
    {
        s32Ret = _MI_AO_DoAdec(AoDevId, AoChn, pstAdecInputQueue, pstSrcInputQueue, pstAoAdecInfo);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_DoAdec failed\n");
            return s32Ret;
        }
    }
    else if(bAdecEnable && bVqeEnable)
    {
        s32Ret = _MI_AO_DoAdec(AoDevId, AoChn, pstAdecInputQueue, pstVqeInputQueue, pstAoAdecInfo);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_DoAdec failed\n");
            return s32Ret;
        }
    }
    else if (bAdecEnable)
    {
        s32Ret = _MI_AO_DoAdec(AoDevId, AoChn, pstAdecInputQueue, pstChnOutputQueue, pstAoAdecInfo);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_DoAdec failed\n");
            return s32Ret;
        }
    }

    if (bResampleEnable && bVqeEnable)
    {
         s32Ret = _MI_AO_DoSrc(AoDevId, AoChn, pstData, pstSrcInputQueue, pstVqeInputQueue);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_DoSrc failed\n");
            return s32Ret;
        }
    }
    else if (bResampleEnable)
    {
         s32Ret = _MI_AO_DoSrc(AoDevId, AoChn, pstData, pstSrcInputQueue, pstChnOutputQueue);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_DoSrc failed\n");
            return s32Ret;
        }
    }

    if (bVqeEnable)
    {
        s32Ret = _MI_AO_DoVqe(AoDevId, AoChn, pstData, pstVqeInputQueue, pstChnOutputQueue);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AO_DoVqe failed\n");
            return s32Ret;
        }
    }

    u64GetTotalSize = _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].u64GetTotalSize;
    u64GetTotalSize += u32LenPerChanl;
    _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].u64GetTotalSize = u64GetTotalSize;
    s32Ret = _MI_AO_SendToChnInputPort(AoDevId, AoChn, pstChnOutputQueue, pstData, s32MilliSec, FALSE);

#ifndef __KERNEL__
    pthread_mutex_unlock(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoMutex);
#endif


    return s32Ret;
}

MI_S32 MI_AO_EnableReSmp(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AUDIO_SampleRate_e eInSampleRate)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_EnableReSmp_t stEnableResmp;
    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;

    ///check input parameter
    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);
    MI_AO_USR_CHECK_SAMPLERATE(eInSampleRate);

    if(TRUE != _gastAoDevInfoUsr[AoDevId].bDevEnable)
        return MI_AO_ERR_NOT_ENABLED;

    if(TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    // put resample status to kernel mode for DebugFs
    memset(&stEnableResmp, 0, sizeof(stEnableResmp));
    stEnableResmp.AoDevId = AoDevId;
    stEnableResmp.AoChn= AoChn;
    stEnableResmp.eInSampleRate = eInSampleRate;
    s32Ret = MI_SYSCALL(MI_AO_ENABLE_RESMP, &stEnableResmp);

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAoDevInfoUsr[AoDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAoDevInfoUsr[AoDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAoDevInfoUsr[AoDevId].stDevAttr.u32PtNumPerFrm;

    if(s32Ret == MI_SUCCESS)
    {
        // Enable channel of AO device resample
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bResampleEnable = TRUE;
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].eInResampleRate = eInSampleRate;

        /* SRC parameter setting */
        s32Ret = _MI_AO_ReSmpInit(AoDevId, AoChn, eInSampleRate);

        //5, G726 Max 5 byte alignment
        //8, G726_16, 2bit convert 16 bit
        //6, Res 8K->48K
        _MI_AO_QueueInit(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stSrcInputQueue, u32NumPerFrm * u32BitWidthByte * u16ChanlNum * 5 * 8 * 6);
    }

    return s32Ret;
}

MI_S32 MI_AO_DisableReSmp(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_DisableReSmp_t stDisableResmp;

    // check input parameter
    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);

    //put resample status to kernel mode for DebugFs
    memset(&stDisableResmp, 0, sizeof(stDisableResmp));
    stDisableResmp.AoDevId = AoDevId;
    stDisableResmp.AoChn = AoChn;
    s32Ret = MI_SYSCALL(MI_AO_DISABLE_RESMP, &stDisableResmp);

    if(s32Ret == MI_SUCCESS)
    {
        // Disable channel of AO device resample
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bResampleEnable = FALSE;
        _MI_AO_QueueDeinit(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stSrcInputQueue);
#ifndef __KERNEL__
    if(NULL != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pSrcWorkingBuf)
    {
        MI_AO_Free(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pSrcWorkingBuf);
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pSrcWorkingBuf = NULL;
    }

#if 0
    if(NULL != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8SrcInBuf)
    {
        MI_AO_Free(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8SrcInBuf);
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8SrcInBuf = NULL;
     }

    if(NULL != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8SrcOutBuf)
    {
        MI_AO_Free(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8SrcOutBuf);
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pu8SrcOutBuf = NULL;
     }
#endif
    IaaSrc_Release( _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hSrcHandle);

#endif
    }

    return s32Ret;
}

MI_S32 MI_AO_PauseChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_PauseChn_t stPauseChn;

    memset(&stPauseChn, 0, sizeof(stPauseChn));
    stPauseChn.AoDevId = AoDevId;
    stPauseChn.AoChn = AoChn;
    s32Ret = MI_SYSCALL(MI_AO_PAUSE_CHN, &stPauseChn);

    return s32Ret;
}

MI_S32 MI_AO_ResumeChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_ResumeChn_t stResumeChn;

    memset(&stResumeChn, 0, sizeof(stResumeChn));
    stResumeChn.AoDevId = AoDevId;
    stResumeChn.AoChn = AoChn;
    s32Ret = MI_SYSCALL(MI_AO_RESUME_CHN, &stResumeChn);

    return s32Ret;
}

MI_S32 MI_AO_ClearChnBuf(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_ClearChnBuf_t stClearChnBuf;

    _MI_AO_QueueInfo_t *pstAdecInputQueue = NULL;
    _MI_AO_QueueInfo_t *pstChnOutputQueue = NULL;
    _MI_AO_QueueInfo_t *pstSrcInputQueue = NULL;
    _MI_AO_QueueInfo_t *pstVqeInputQueue = NULL;



    memset(&stClearChnBuf, 0, sizeof(stClearChnBuf));
    stClearChnBuf.AoDevId = AoDevId;
    stClearChnBuf.AoChn = AoChn;
    s32Ret = MI_SYSCALL(MI_AO_CLEAR_CHN_BUF, &stClearChnBuf);

    if (MI_SUCCESS == s32Ret)
    {

        pstAdecInputQueue = &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAdecInputQueue;
        pstChnOutputQueue = &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stChnOutputQueue;
        pstSrcInputQueue = &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stSrcInputQueue;
        pstVqeInputQueue = &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stVqeInputQueue;

        _MI_AO_QueueClear(pstAdecInputQueue);
        _MI_AO_QueueClear(pstChnOutputQueue);
        _MI_AO_QueueClear(pstSrcInputQueue);
        _MI_AO_QueueClear(pstVqeInputQueue);
    }

    return s32Ret;
}

MI_S32 MI_AO_QueryChnStat(MI_AUDIO_DEV AoDevId , MI_AO_CHN AoChn, MI_AO_ChnState_t *pstStatus)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_QueryChnStat_t stQueryChnStat;

    memset(&stQueryChnStat, 0, sizeof(stQueryChnStat));
    stQueryChnStat.AoDevId = AoDevId;
    stQueryChnStat.AoChn = AoChn;
    s32Ret = MI_SYSCALL(MI_AO_QUERY_CHN_STAT, &stQueryChnStat);

    if(s32Ret == MI_SUCCESS)
    {
        memcpy(pstStatus, &stQueryChnStat.stStatus, sizeof(MI_AO_ChnState_t));
    }

    return s32Ret;
}

MI_S32 MI_AO_SetVolume(MI_AUDIO_DEV AoDevId, MI_S32 s32VolumeDb)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetVolume_t stSetVolume;

    memset(&stSetVolume, 0, sizeof(stSetVolume));
    stSetVolume.AoDevId = AoDevId;
    stSetVolume.s32VolumeDb = s32VolumeDb;
    s32Ret = MI_SYSCALL(MI_AO_SET_VOLUME, &stSetVolume);

    return s32Ret;
}

MI_S32 MI_AO_GetVolume(MI_AUDIO_DEV AoDevId, MI_S32 *ps32VolumeDb)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_GetVolume_t stGetVolume;

    memset(&stGetVolume, 0, sizeof(stGetVolume));
    stGetVolume.AoDevId = AoDevId;
    s32Ret = MI_SYSCALL(MI_AO_GET_VOLUME, &stGetVolume);

    if(s32Ret == MI_SUCCESS)
    {
        memcpy(ps32VolumeDb, &stGetVolume.s32VolumeDb, sizeof(MI_S32));
    }

    return s32Ret;
}

MI_S32 MI_AO_SetMute(MI_AUDIO_DEV AoDevId, MI_BOOL bEnable)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetMute_t stSetMute;

    memset(&stSetMute, 0, sizeof(stSetMute));
    stSetMute.AoDevId = AoDevId;
    stSetMute.bEnable = bEnable;
    s32Ret = MI_SYSCALL(MI_AO_SET_MUTE, &stSetMute);

    return s32Ret;
}

MI_S32 MI_AO_GetMute(MI_AUDIO_DEV AoDevId, MI_BOOL *pbEnable)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_GetMute_t stGetMute;

    memset(&stGetMute, 0, sizeof(stGetMute));
    stGetMute.AoDevId = AoDevId;
    s32Ret = MI_SYSCALL(MI_AO_GET_MUTE, &stGetMute);

    if(s32Ret == MI_SUCCESS)
    {
        memcpy(pbEnable, &stGetMute.bEnable, sizeof(MI_BOOL));
    }

    return s32Ret;
}

MI_S32 MI_AO_ClrPubAttr(MI_AUDIO_DEV AoDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_AO_CLR_PUB_ATTR, &AoDevId);

    return s32Ret;
}

MI_S32 MI_AO_SetVqeAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_VqeConfig_t *pstVqeConfig)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetVqeAttr_t stSetVqeAttr;


    // check input parameter
    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);
    MI_AO_USR_CHECK_POINTER(pstVqeConfig);

    // check if Vqe of AO device channel is disable ?
    if( FALSE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeEnable)
        return MI_AO_ERR_NOT_PERM;

    // check if AO device channel is enable ?
    if( TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    // check if Vqe Configure legal ?

    // put VQE status to kernel mode for DebugFs
    memset(&stSetVqeAttr, 0, sizeof(stSetVqeAttr));
    stSetVqeAttr.AoDevId = AoDevId;
    stSetVqeAttr.AoChn= AoChn;
    memcpy(&stSetVqeAttr.stVqeConfig, pstVqeConfig, sizeof(MI_AO_VqeConfig_t));
    s32Ret = MI_SYSCALL(MI_AO_SET_VQE_ATTR, &stSetVqeAttr);

    if(s32Ret == MI_SUCCESS)
    {
        // save Vqe configure of AO device channel
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeAttrSet = TRUE;
        memcpy(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoVqeConfig, pstVqeConfig, sizeof(MI_AO_VqeConfig_t));
    }

    return s32Ret;
}

MI_S32 MI_AO_GetVqeAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_VqeConfig_t *pstVqeConfig)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);
    MI_AO_USR_CHECK_POINTER(pstVqeConfig);

    // check if Vqe attribute of AO device channel is set ?
    if(TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeAttrSet)
        return MI_AO_ERR_NOT_PERM;

    // load Vqe config
    memcpy(pstVqeConfig, &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoVqeConfig, sizeof(MI_AO_VqeConfig_t));

    return s32Ret;
}

MI_S32 MI_AO_EnableVqe(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
     MI_S32 s32Ret = MI_SUCCESS;
     MI_AO_EnableVqe_t stEnableVqe;
    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;

    // check input parameter
    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);

    //
    if( TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeAttrSet)
        return MI_AO_ERR_NOT_PERM;

    if( TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    if(TRUE == _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeEnable)
        return s32Ret;

    // put VQE status to kernel mode for DebugFs
    memset(&stEnableVqe, 0, sizeof(stEnableVqe));
    stEnableVqe.AoDevId = AoDevId;
    stEnableVqe.AoChn= AoChn;
    s32Ret = MI_SYSCALL(MI_AO_ENABLE_VQE, &stEnableVqe);

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAoDevInfoUsr[AoDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAoDevInfoUsr[AoDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAoDevInfoUsr[AoDevId].stDevAttr.u32PtNumPerFrm;

    if(s32Ret == MI_SUCCESS)
    {
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeEnable = TRUE;

        //5, G726 Max 5 byte alignment
        //8, G726_16, 2bit convert 16 bit
        //6, Res 8K->48K
        _MI_AO_QueueInit(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stVqeInputQueue, u32NumPerFrm * u32BitWidthByte * u16ChanlNum * 5 * 8 * 6);
        //VQE init
        s32Ret = _MI_AO_VqeInit(AoDevId, AoChn);
    }

    return s32Ret;
}

MI_S32 MI_AO_DisableVqe(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_DisableVqe_t stDisableVqe;

    // check input parameter
    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);

    if(FALSE == _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeEnable)
        return s32Ret;

    // put vqe status to kernel mode for DebugFs
    memset(&stDisableVqe, 0, sizeof(stDisableVqe));
    stDisableVqe.AoDevId = AoDevId;
    stDisableVqe.AoChn = AoChn;
    s32Ret = MI_SYSCALL(MI_AO_DISABLE_VQE, &stDisableVqe);

    if(s32Ret == MI_SUCCESS)
    {
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeEnable = FALSE;
        //_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bVqeAttrSet = FALSE; // ???

#ifndef __KERNEL__
    if(NULL != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pApcWorkingBuf)
    {
        MI_AO_Free( _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pApcWorkingBuf);
         _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].pApcWorkingBuf = NULL;
    }

    IaaApc_Free(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hApcHandle);
#endif
        _MI_AO_QueueDeinit(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stVqeInputQueue);
    }

    return s32Ret;
}

MI_S32 MI_AO_SetAdecAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_AdecConfig_t *pstAdecConfig)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetAdecAttr_t stSetAdecAttr;

    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);
    MI_AO_USR_CHECK_POINTER(pstAdecConfig);


    // check if Adec of AO device channel is disable ?
    if( FALSE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAdecEnable)
    {
        DBG_WRN("Dev%d Chn%d Adec is already enable\n", AoDevId, AoChn);
        return MI_AO_ERR_NOT_PERM;
    }

    // check if AO device channel is enable ?
    if( TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable)
    {
        DBG_WRN("Dev%d Chn%d is not enable\n", AoDevId, AoChn);
        return MI_AO_ERR_NOT_ENABLED;
    }

    memset(&stSetAdecAttr, 0, sizeof(MI_AO_SetAdecAttr_t));
    stSetAdecAttr.AoDevId = AoDevId;
    stSetAdecAttr.AoChn = AoChn;
    memcpy(&stSetAdecAttr.stAdecConfig, pstAdecConfig, sizeof(MI_AO_AdecConfig_t));
    s32Ret = MI_SYSCALL(MI_AO_SET_ADEC_ATTR, &stSetAdecAttr);

    if (MI_SUCCESS == s32Ret)
    {
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAdecAttrSet = TRUE;
        memcpy(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoAdecConfig, pstAdecConfig, sizeof(MI_AO_AdecConfig_t));
    }

    return s32Ret;
}

MI_S32 MI_AO_GetAdecAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_AdecConfig_t *pstAdecConfig)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);
    MI_AO_USR_CHECK_POINTER(pstAdecConfig);

    if (TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAdecAttrSet)
    {
        DBG_WRN("Dev%d Chn%d Adec Attr is not set\n", AoDevId, AoChn);
       return MI_AO_ERR_NOT_PERM;
    }

    memcpy(pstAdecConfig, &_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAoAdecConfig, sizeof(MI_AO_AdecConfig_t));

    return s32Ret;
}

MI_S32 MI_AO_EnableAdec(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_EnableAdec_t stEnableAdec;
    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;

    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);

    if (TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAdecAttrSet)
    {
        DBG_WRN("Dev%d Chn%d Adec Attr is not set\n", AoDevId, AoChn);
        return MI_AO_ERR_NOT_PERM;
    }

    if (TRUE != _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bChanEnable)
    {
        DBG_WRN("Dev%d Chn%d is not enable\n", AoDevId, AoChn);
        return MI_AO_ERR_NOT_ENABLED;
    }

    if (TRUE == _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAdecEnable)
    {
        return s32Ret;
    }

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAoDevInfoUsr[AoDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAoDevInfoUsr[AoDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAoDevInfoUsr[AoDevId].stDevAttr.u32PtNumPerFrm;

    memset(&stEnableAdec, 0, sizeof(MI_AO_EnableAdec_t));
    stEnableAdec.AoDevId = AoDevId;
    stEnableAdec.AoChn = AoChn;
    s32Ret = MI_SYSCALL(MI_AO_ENABLE_ADEC, &stEnableAdec);

    if (MI_SUCCESS == s32Ret)
    {
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAdecEnable = TRUE;
        _MI_AO_AdecInit(AoDevId, AoChn);
        //5, G726 Max 5 byte alignment
        //8, G726_16, 2bit convert 16 bit
        //6, Res 8K->48K
        _MI_AO_QueueInit(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAdecInputQueue, u32NumPerFrm * u32BitWidthByte * u16ChanlNum * 5 * 8 * 6);
    }

    return s32Ret;
}

MI_S32 MI_AO_DisableAdec(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_DisableAdec_t stDisableAdec;

    MI_AO_USR_CHECK_DEV(AoDevId);
    MI_AO_USR_CHECK_CHN(AoChn);

    //should enable before

    if (FALSE == _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAdecEnable)
    {
        return s32Ret;
    }

    memset(&stDisableAdec, 0, sizeof(MI_AO_DisableAdec_t));
    stDisableAdec.AoDevId = AoDevId;
    stDisableAdec.AoChn = AoChn;
    s32Ret = MI_SYSCALL(MI_AO_DISABLE_ADEC, &stDisableAdec);

    if(MI_SUCCESS  == s32Ret)
    {
        _gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].bAdecEnable = FALSE;
        _MI_AO_QueueDeinit(&_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].stAdecInputQueue);
#ifndef __KERNEL__
        MI_AO_Free(_gastAoDevInfoUsr[AoDevId].astChanInfo[AoChn].hG72Handle);
#endif
    }

    return s32Ret;
}


EXPORT_SYMBOL(MI_AO_SetPubAttr);
EXPORT_SYMBOL(MI_AO_GetPubAttr);
EXPORT_SYMBOL(MI_AO_Enable);
EXPORT_SYMBOL(MI_AO_Disable);
EXPORT_SYMBOL(MI_AO_EnableChn);
EXPORT_SYMBOL(MI_AO_DisableChn);
EXPORT_SYMBOL(MI_AO_SendFrame);
EXPORT_SYMBOL(MI_AO_EnableReSmp);
EXPORT_SYMBOL(MI_AO_DisableReSmp);
EXPORT_SYMBOL(MI_AO_PauseChn);
EXPORT_SYMBOL(MI_AO_ResumeChn);
EXPORT_SYMBOL(MI_AO_ClearChnBuf);
EXPORT_SYMBOL(MI_AO_QueryChnStat);
EXPORT_SYMBOL(MI_AO_SetVolume);
EXPORT_SYMBOL(MI_AO_GetVolume);
EXPORT_SYMBOL(MI_AO_SetMute);
EXPORT_SYMBOL(MI_AO_GetMute);
EXPORT_SYMBOL(MI_AO_ClrPubAttr);
EXPORT_SYMBOL(MI_AO_SetVqeAttr);
EXPORT_SYMBOL(MI_AO_GetVqeAttr);
EXPORT_SYMBOL(MI_AO_EnableVqe);
EXPORT_SYMBOL(MI_AO_DisableVqe);
EXPORT_SYMBOL(MI_AO_SetAdecAttr);
EXPORT_SYMBOL(MI_AO_GetAdecAttr);
EXPORT_SYMBOL(MI_AO_EnableAdec);
EXPORT_SYMBOL(MI_AO_DisableAdec);
