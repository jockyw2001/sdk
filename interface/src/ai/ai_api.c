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
/// @file   ai_api.c
/// @brief ai module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __KERNEL__
#include <stdlib.h>
#include <stdio.h>
#include<sys/time.h>

#endif

#include "mi_syscall.h"
#include "mi_print.h"
#include "mi_sys.h"
#include "mi_common.h"
#include "mi_ai.h"
#include "mi_aio_internal.h"
#include "ai_ioctl.h"

#if USE_CAM_OS
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#endif

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

MI_MODULE_DEFINE(ai)

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------
#define MI_AI_USR_CHECK_DEV(AiDevId)  \
    if(AiDevId < 0 || AiDevId >= MI_AI_DEV_NUM_MAX) \
    {   \
        DBG_ERR("AiDevId is invalid! AiDevId = %u.\n", AiDevId);   \
        return MI_AI_ERR_INVALID_DEVID;   \
    }

#define MI_AI_USR_CHECK_CHN(Aichn)  \
    if(Aichn < 0 || Aichn >= MI_AI_CHAN_NUM_MAX) \
    {   \
        DBG_ERR("Aichn is invalid! Aichn = %u.\n", Aichn);   \
        return MI_AI_ERR_INVALID_CHNID;   \
    }

#define MI_AI_USR_CHECK_POINTER(pPtr)  \
    if(NULL == pPtr)  \
    {   \
        DBG_ERR("Invalid parameter! NULL pointer.\n");   \
        return MI_AI_ERR_NULL_PTR;   \
    }

#define MI_AI_USR_CHECK_SAMPLERATE(eSamppleRate)    \
    if( (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_8000) && (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_16000) &&\
        (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_32000) && (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_48000) ) \
    { \
        DBG_ERR("Sample Rate is illegal = %u.\n", eSamppleRate);   \
        return MI_AI_ERR_ILLEGAL_PARAM;   \
    }

#if USE_CAM_OS
    #define MI_AI_Malloc CamOsMemAlloc
    #define MI_AI_Free  CamOsMemRelease
#else
    #define MI_AI_Malloc malloc
    #define MI_AI_Free free
#endif


#define MI_AI_APC_POINTER 128
#define MI_AI_APC_EQ_BAND_NUM 10

#define MI_AI_G711A 0
#define MI_AI_G711U 1
#define MI_AI_POINT_NUM_MAX (25600) //25K
#define MI_AI_AENC_G726_UNIT (16)
#define MI_AI_AENC_G726_UNIT_MAX (1200)

#define MI_AI_SRC_UNIT     (256)
#define MI_AI_SRC_UNIT_MAX (1024)

#define MI_AI_SRC_UNIT     (256)
#define MI_AI_SRC_UNIT_MAX (1024)

#define MI_AI_AEC_REF_BUFF_SIZE (1024 * 50)

//#define DUMP_FILE (1)

#ifdef DUMP_FILE
#define MI_AO_INPUT_FILE "/mnt/AI_File/ao_input.pcm"
#define MI_AI_INPUT_FILE "/mnt/AI_File/ai_input.pcm"
#define MI_AI_AEC_FILE  "/mnt/AI_File/aec.pcm"
#define MI_AI_AEC_BUFF_FILE "/mnt/AI_File/ai_aecbuff.pcm"
#define MI_AO_AEC_BUFF_FILE "/mnt/AI_File/ao_aecbuff.pcm"
#endif
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

typedef struct _MI_AI_QueueInfo_s{
    MI_S32 s32Front;
    MI_S32 s32Rear;
    MI_S32 s32Max;//Max len of queue
    MI_S32 s32Size; //current size
    MI_U8  *pu8Buff;
}_MI_AI_QueueInfo_t;

typedef struct _MI_AI_AECRefBuffInfo_s{
    void    *pvBuff;
    MI_U32  u32MaxSize;
    MI_U32  u32ReadPtr;
    MI_U32  u32WritePtr;
    MI_U32  u32IgnoreSize;
}_MI_AI_AECRefBuffInfo_t;

typedef struct _MI_AI_GetRefDataInfo_s{ //use for pthread
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
}_MI_AI_GetRefDataInfo_t;

typedef struct _MI_AI_ChanInfoUsr_s
{
    MI_BOOL                 bChanEnable;
    MI_BOOL                 bPortEnable;
    MI_S32                  s32ChnId;
    MI_S32                  s32OutputPortId;
    MI_BOOL                 bResampleEnable;
    MI_AUDIO_SampleRate_e   eOutResampleRate;
    MI_BOOL                 bVqeEnable;
    MI_BOOL                 bVqeAttrSet;
    MI_S32                  s32VolumeDb;
    MI_AI_VqeConfig_t       stAiVqeConfig;
    MI_AUDIO_SaveFileInfo_t stSaveFileInfo;
    MI_SYS_BUF_HANDLE       hBufHandle;
    //Resample(SRC)
    SRC_HANDLE              hSrcHandle;
    void*                   pSrcWorkingBuf;
    MI_U8*                  pu8SrcOutBuf;
    // VQE
    APC_HANDLE              hApcHandle;
    void*                   pApcWorkingBuf;
     // AEC
    AEC_HANDLE              hAecHandle;
    void*                   pAecWorkingBuf;
    MI_AUDIO_DEV            AoDevId;
    MI_AO_CHN               AoChn;
    MI_SYS_BUF_HANDLE       hRefBufHandle;
    _MI_AI_AECRefBuffInfo_t stAECTmpBuff;
    _MI_AI_AECRefBuffInfo_t stAECRefBuff;
    MI_BOOL                 bAecRefStartRead;
    MI_U64                  u64RefStartPts;
    MI_U64                  u64RefPts; //Ao old pts
    MI_BOOL                 bAiPtsLeap; //when ai pts is not contiue, it set to ture
    MI_U64                  u64AiOldPts;  //ai old pts
    MI_U64                  u64AiDiffPts; //diff pst when ai frame is not continue
    MI_U32                  u32DiffPtr;
    void*                   pAecRefUnitBuf;
    MI_BOOL                 bAiPthreadExit ;
    MI_BOOL                 _gbAECRestart;
#ifndef __KERNEL__
    pthread_t               stAiPthreadId;
#endif
    //aenc
    MI_BOOL                 bAencAttrSet;
    MI_BOOL                 bAencEnable;
    MI_AI_AencConfig_t      stAiAencConfig;


    //Queue
    _MI_AI_QueueInfo_t      stSrcQueue; //for IaaSrc
    _MI_AI_QueueInfo_t      stAECQueue; //for IaaAec
    _MI_AI_QueueInfo_t      stVqeQuque; //for IaaVqe
    _MI_AI_QueueInfo_t      stAencQueue; //for g711/g726 encoder
    _MI_AI_QueueInfo_t      stChnQueue;

    //g726
    g726_state_t            *hG726Handle;

    //Tmp buff
    MI_U8                   *pu8InputBuff;
    MI_U8                   *pu8OutputBuff;
    MI_U8                   *pu8ChnBuff;

    // debug
    MI_U32                   u32AecRefBufCnt;

    // VQE dumpfile
    MI_S32 s32FdSinWr;
    MI_S32 s32FdRinWr;
    MI_S32 s32FdSouWr;

    //for pthread
    _MI_AI_GetRefDataInfo_t  stGetRefDataInfo;

}_MI_AI_ChanInfoUsr_t;

typedef struct _MI_AI_DevInfoUsr_s
{
    MI_BOOL                 bDevEnable;
    MI_BOOL                 bDevAttrSet;
    MI_AUDIO_DEV            AiDevId;
    MI_AUDIO_Attr_t         stDevAttr;
    _MI_AI_ChanInfoUsr_t    astChanInfo[MI_AI_CHAN_NUM_MAX];
    MI_U64                  u64PhyBufAddr;          // for DMA HW address
    MI_U32                  u32ResPtNumPerFrm; //is enable resample, the u32PtNumPerFrm should change
}_MI_AI_DevInfoUsr_t;

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
// initial & Todo: need to protect variable
static _MI_AI_DevInfoUsr_t _gastAiDevInfoUsr[MI_AI_DEV_NUM_MAX]={
    {
        .bDevEnable = FALSE,
        .bDevAttrSet = FALSE,
        .astChanInfo[0] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[1] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[2] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[3] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[4] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[5] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[6] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[7] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[8] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[9] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[10] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[11] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[12] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[13] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[14] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
        .astChanInfo[15] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .hSrcHandle = NULL,
            .hApcHandle = NULL,
            .hAecHandle = NULL,
            .u64RefStartPts = 0,
            .u32AecRefBufCnt = 0,
        },
    },
};

#ifdef DUMP_FILE

#ifndef __KERNEL__
    FILE *_gpstAiInput;
    FILE *_gpstAoInput;
    FILE *_gpstAiAecBuff;
    FILE *_gpstAoAecBuff;
    FILE *_gpstAecOutput;
    FILE *_gpstAoRefBuff;
#endif

#endif
//-------------------------------------------------------------------------------------------------
// Local  Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

static SrcConversionMode _MI_AI_GetSrcConvertMode(MI_AUDIO_SampleRate_e  eInResampleRate, MI_AUDIO_SampleRate_e eOutSamplerate)
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

static MI_U32 _MI_AI_ChangePtFrmByResample(MI_AUDIO_SampleRate_e  eInResampleRate, MI_AUDIO_SampleRate_e eOutSamplerate, MI_U32 u32OldPtFrm)
{
    MI_U32 u32NewPtFrm = 0;

    if ( (MI_S32) eInResampleRate > (MI_S32)eOutSamplerate )
    {
        u32NewPtFrm = u32OldPtFrm / ((MI_S32) eInResampleRate / (MI_S32)eOutSamplerate);

        return u32NewPtFrm;
    }
    else if ( (MI_S32) eInResampleRate < (MI_S32)eOutSamplerate )
    {
        u32NewPtFrm = u32OldPtFrm * ((MI_S32) eOutSamplerate / (MI_S32)eInResampleRate);

        return u32NewPtFrm;
    }

    return MI_AI_ERR_NOT_PERM;
}

static MI_S32 _MI_AI_ReSmpInit(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_SampleRate_e eOutSampleRate)
{
    MI_S32 s32Ret = MI_SUCCESS;

     SRCStructProcess stSrcStruct;
     MI_AUDIO_SampleRate_e eAttrSampleRate;
     MI_U16 u16ChanlNum;
     MI_U32 u32SrcUnit;

    eAttrSampleRate = _gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate;
    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode);

    u32SrcUnit = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;
    _gastAiDevInfoUsr[AiDevId].u32ResPtNumPerFrm = _MI_AI_ChangePtFrmByResample(eAttrSampleRate, eOutSampleRate, u32SrcUnit);

    stSrcStruct.WaveIn_srate = (SrcInSrate) (eAttrSampleRate/1000);// get_sample_rate_enumeration(input_wave->wave_header.sample_per_sec);
    stSrcStruct.channel = u16ChanlNum;
    stSrcStruct.mode = _MI_AI_GetSrcConvertMode(eAttrSampleRate, eOutSampleRate);

    if (u32SrcUnit < MI_AI_SRC_UNIT)
    {
        stSrcStruct.point_number = MI_AI_SRC_UNIT;
    }
    else if (u32SrcUnit > MI_AI_SRC_UNIT_MAX)
    {
        stSrcStruct.point_number = MI_AI_SRC_UNIT_MAX;
    }
    else
    {
        stSrcStruct.point_number = u32SrcUnit - (u32SrcUnit % MI_AI_SRC_UNIT);
    }

    /* SRC init */
#ifndef __KERNEL__
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pSrcWorkingBuf = MI_AI_Malloc(IaaSrc_GetBufferSize(stSrcStruct.mode));
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hSrcHandle = IaaSrc_Init(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pSrcWorkingBuf, &stSrcStruct);
#endif

    // error handle
    if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hSrcHandle == NULL)
    {
        //s32Ret = MI_ERR_AI_VQE_ERR; //add ReSmp error ?
        DBG_ERR("_MI_AI_ReSmpInit Fail !!!!!\n");
    }

    return s32Ret;
}

#ifndef __KERNEL__

static MI_S32 _MI_AI_SetEqGainDb(EqGainDb_t *pstEqGainInfo, MI_S16  *pS16Buff, MI_U32 u32Size)
{
    MI_S16 *ps16Tmp = (MI_S16 *)pstEqGainInfo;
    MI_S32 s32i = 0;

    MI_AI_USR_CHECK_POINTER(pstEqGainInfo);
    MI_AI_USR_CHECK_POINTER(pS16Buff);

    for (s32i = 0; s32i < u32Size; s32i++)
    {
        pS16Buff[s32i] = *ps16Tmp;
        ps16Tmp ++;
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_AI_IaaApc_Config_Printf(
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
    for (s32i = 0; s32i < MI_AI_APC_EQ_BAND_NUM; s32i++)
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

static MI_S32 _MI_AI_IaaApc_Print(APC_HANDLE hApc)
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
        DBG_ERR("IaaApc_GetConfig failed !!!!!\n");
        return MI_AI_ERR_VQE_ERR;
    }

    _MI_AI_IaaApc_Config_Printf(&stGetAnrInfo, &stGetEqInfo, &stGetHpfInfo, NULL, NULL, &stGetAgcInfo);

    return MI_SUCCESS;
}


#endif

static MI_S32 _MI_AI_QueueInit(_MI_AI_QueueInfo_t *pstQueue, MI_S32 s32Size)
{
    MI_AI_USR_CHECK_POINTER(pstQueue);

#ifndef __KERNEL__
    pstQueue->pu8Buff = (MI_U8 *)MI_AI_Malloc(s32Size);
    MI_AI_USR_CHECK_POINTER(pstQueue->pu8Buff);
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

static MI_S32 _MI_AI_QueueDeinit(_MI_AI_QueueInfo_t *pstQueue)
{
    MI_AI_USR_CHECK_POINTER(pstQueue);
    MI_AI_USR_CHECK_POINTER(pstQueue->pu8Buff);

#ifndef __KERNEL__
    MI_AI_Free(pstQueue->pu8Buff);
    pstQueue->pu8Buff = NULL;
#endif
    pstQueue->s32Front = 0;
    pstQueue->s32Rear = 0;
    pstQueue->s32Size = 0;
    pstQueue->s32Max = 0;

    return MI_SUCCESS;
}

static MI_S32 _MI_AI_QueueInsert(_MI_AI_QueueInfo_t *pstQueue, MI_U8 *pu8InputBuff, MI_S32 s32Size)
{
    MI_S32 s32Tmp = 0;

    MI_AI_USR_CHECK_POINTER(pstQueue);
    MI_AI_USR_CHECK_POINTER(pstQueue->pu8Buff);
    MI_AI_USR_CHECK_POINTER(pu8InputBuff);

    if (s32Size < 0)
    {
        s32Size = 0;
    }

    s32Tmp = pstQueue->s32Size + s32Size;
    if (s32Tmp > pstQueue->s32Max)
    {
        DBG_WRN("Queue has not enough space\n");
        DBG_WRN("Input Size:%d, Max Size:%d,Remain Size:%d\n", s32Size, pstQueue->s32Max, pstQueue->s32Max- pstQueue->s32Size);
        return MI_AI_ERR_NOBUF;
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

static MI_S32 _MI_AI_QueueDraw(_MI_AI_QueueInfo_t *pstQueue, MI_U8 *pu8OutputBuff, MI_S32 s32Size)
{
    MI_S32 s32Tmp = 0;

    MI_AI_USR_CHECK_POINTER(pstQueue)
    MI_AI_USR_CHECK_POINTER(pu8OutputBuff)

    if (s32Size < 0)
    {
        s32Size = 0;
    }

    s32Tmp = pstQueue->s32Size - s32Size;
    if (s32Tmp < 0)
    {
        DBG_WRN("Queue has not enough data\n");
        DBG_WRN("Input Size:%d, Remain data Size:%d\n", s32Size, pstQueue->s32Size);
        return MI_AI_ERR_BUF_EMPTY;
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

MI_S32 _MI_AI_G726Init(g726_state_t *pstG726Info, MI_S32 s32BitRate)
{
    MI_AI_USR_CHECK_POINTER(pstG726Info);

#ifndef __KERNEL__
    g726_init(pstG726Info, s32BitRate);
#endif

    return MI_SUCCESS;
}

static MI_S32  _MI_AI_AencInit(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_AencConfig_t stAiAencConfig;
    g726_state_t *pstG726Info = NULL;

    stAiAencConfig = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiAencConfig;

    if(E_MI_AUDIO_AENC_TYPE_G726 == stAiAencConfig.eAencType)
    {
#ifndef __KERNEL__
        pstG726Info = MI_AI_Malloc(sizeof(g726_state_t));
#endif
        MI_AI_USR_CHECK_POINTER(pstG726Info);
        memset(pstG726Info, 0, sizeof(g726_state_t));

        switch(stAiAencConfig.stAencG726Cfg.eG726Mode)
        {
            case E_MI_AUDIO_G726_MODE_16:
                _MI_AI_G726Init(pstG726Info, 8000 * 2);
                DBG_INFO("_MI_AI_G726Init 16k\n");
                break;
            case E_MI_AUDIO_G726_MODE_24:
                _MI_AI_G726Init(pstG726Info, 8000 * 3);
                DBG_INFO("_MI_AI_G726Init 24k\n");
                break;
            case E_MI_AUDIO_G726_MODE_32:
                _MI_AI_G726Init(pstG726Info, 8000 * 4);
                DBG_INFO("_MI_AI_G726Init 32k\n");
                break;
            case E_MI_AUDIO_G726_MODE_40:
                _MI_AI_G726Init(pstG726Info, 8000 * 5);
                DBG_INFO("_MI_AI_G726Init 40k\n");
                break;
            default:
                DBG_WRN("G726 Mode is not find:%d\n", stAiAencConfig.stAencG726Cfg.eG726Mode);
#ifndef __KERNEL__
                MI_AI_Free(pstG726Info);
#endif
            return MI_AI_ERR_AENC_ERR;
        }
    }

    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hG726Handle = pstG726Info;

    return s32Ret;
}


static MI_S32 _MI_AI_VqeInit(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

#ifndef __KERNEL__
    AudioProcessInit stApstruct;
    void* pApcWorkingBuf;
    APC_HANDLE hApcHandle;
    MI_U16 u16ChanlNum;

    MI_AI_VqeConfig_t stAiVqeConfig;

    AudioAnrConfig stAnrInfo;
    AudioEqConfig  stEqInfo;
    AudioHpfConfig stHpfInfo;

    AudioVadConfig stVadInfo;
    AudioDereverbConfig stDereverbInfo;
    AudioAgcConfig stAgcInfo;

    MI_AUDIO_SampleRate_e eSampleReate;

    if (_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bResampleEnable)
    {
        eSampleReate = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].eOutResampleRate;
    }
    else
    {
        eSampleReate = _gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate;
    }

    /* APC init parameter setting */
    stApstruct.point_number = MI_AI_APC_POINTER;
    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode);
    stApstruct.channel = u16ChanlNum;
    MI_AUDIO_VQE_SAMPLERATE_TRANS_TYPE(eSampleReate, stApstruct.sample_rate);

    /* APC init */
    pApcWorkingBuf = MI_AI_Malloc(IaaApc_GetBufferSize());
    if (NULL == pApcWorkingBuf)
    {
        DBG_ERR("Malloc IaaApc_GetBuffer failed\n");
        return MI_AI_ERR_NOBUF;
    }

    hApcHandle  = IaaApc_Init((char* const)pApcWorkingBuf, &stApstruct);
    if(hApcHandle == NULL)
    {
        DBG_ERR("IaaApc_Init FAIL !!!!!!!!!!!!!!!!!!!");
        return MI_AI_ERR_VQE_ERR;
    }
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pApcWorkingBuf = pApcWorkingBuf;
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hApcHandle = hApcHandle;

    /* VQE Setting */
    stAiVqeConfig = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig;

       //set Anr
    stAnrInfo.anr_enable = (MI_U32)stAiVqeConfig.bAnrOpen;
    stAnrInfo.user_mode = (MI_U32)stAiVqeConfig.stAnrCfg.bUsrMode;
     MI_AUDIO_VQE_NR_SPEED_TRANS_TYPE(stAiVqeConfig.stAnrCfg.eNrSpeed, stAnrInfo.anr_converge_speed);
    stAnrInfo.anr_intensity = stAiVqeConfig.stAnrCfg.u32NrIntensity;
    stAnrInfo.anr_smooth_level = stAiVqeConfig.stAnrCfg.u32NrSmoothLevel;

    //set Eq
    stEqInfo.eq_enable = (MI_U32)stAiVqeConfig.bEqOpen;
    stEqInfo.user_mode = (MI_U32)stAiVqeConfig.stEqCfg.bUsrMode;
    s32Ret = _MI_AI_SetEqGainDb(&stAiVqeConfig.stEqCfg.stEqGain, stEqInfo.eq_gain_db, MI_AI_APC_EQ_BAND_NUM);

    //set Hpf
    stHpfInfo.hpf_enable = (MI_U32)stAiVqeConfig.bHpfOpen;
    stHpfInfo.user_mode = (MI_U32)stAiVqeConfig.stHpfCfg.bUsrMode;

    if (TRUE == stAiVqeConfig.bHpfOpen)
    {
        MI_AUDIO_VQE_HPF_TRANS_TYPE(stAiVqeConfig.stHpfCfg.eHpfFreq, stHpfInfo.cutoff_frequency);
    }
    else
    {
        stAiVqeConfig.stHpfCfg.eHpfFreq = E_MI_AUDIO_HPF_FREQ_150;
    }
    //set Vad
    stVadInfo.vad_enable = 0;
    stVadInfo.user_mode = 1;
    stVadInfo.vad_threshold = -10;

    //set De-reverberation
    stDereverbInfo.dereverb_enable = 0;

    //set Agc
    stAgcInfo.agc_enable = (MI_U32)stAiVqeConfig.bAgcOpen;
    stAgcInfo.user_mode = (MI_U32)stAiVqeConfig.stAgcCfg.bUsrMode;
    stAgcInfo.attack_time = stAiVqeConfig.stAgcCfg.u32AttackTime;
    stAgcInfo.release_time = stAiVqeConfig.stAgcCfg.u32ReleaseTime;
    stAgcInfo.compression_ratio = stAiVqeConfig.stAgcCfg.u32CompressionRatio;
    stAgcInfo.drop_gain_max = stAiVqeConfig.stAgcCfg.u32DropGainMax;
    stAgcInfo.gain_info.gain_init = stAiVqeConfig.stAgcCfg.stAgcGainInfo.s32GainInit;
    stAgcInfo.gain_info.gain_max = stAiVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMax;
    stAgcInfo.gain_info.gain_min = stAiVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMin;
    stAgcInfo.noise_gate_attenuation_db = stAiVqeConfig.stAgcCfg.u32NoiseGateAttenuationDb;
    stAgcInfo.noise_gate_db = stAiVqeConfig.stAgcCfg.s32NoiseGateDb;
    stAgcInfo.target_level_db = stAiVqeConfig.stAgcCfg.s32TargetLevelDb;

                                                                       // Vad   dereverb is not set
    s32Ret = IaaApc_Config(hApcHandle, &stAnrInfo, &stEqInfo, &stHpfInfo, NULL, NULL, &stAgcInfo);
    if (0 != s32Ret)
    {
        DBG_ERR("IaaApc_config FAIL !!!!!!!!!!!!!!!!!!!\n");
        return MI_AI_ERR_VQE_ERR;
    }

     _MI_AI_IaaApc_Print(hApcHandle);
#endif

    return s32Ret;
}

#ifndef __KERNEL__
static MI_S32 _MI_AI_WriteTmpBuf(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_SYS_BufInfo_t stBufInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_U32 u32AecTmpBufSize;
    void* pAecTmpBuf;
    MI_U32 u32AecTmpWrPtr;
    MI_U32 u32ResidualSize;
    _MI_AI_AECRefBuffInfo_t *pstAECTmpBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECTmpBuff;

    u32AecTmpBufSize = pstAECTmpBuff->u32MaxSize ;
    pAecTmpBuf = pstAECTmpBuff->pvBuff;
    u32AecTmpWrPtr = pstAECTmpBuff->u32WritePtr;

#ifndef __KERNEL__
#ifdef DUMP_FILE
    fwrite(stBufInfo.stRawData.pVirAddr, stBufInfo.stRawData.u32BufSize, 1, _gpstAoInput);
#endif
#endif

    if( (u32AecTmpWrPtr + stBufInfo.stRawData.u32BufSize) < u32AecTmpBufSize)
    {
        memcpy(pAecTmpBuf+u32AecTmpWrPtr, stBufInfo.stRawData.pVirAddr, stBufInfo.stRawData.u32BufSize);
        u32AecTmpWrPtr += stBufInfo.stRawData.u32BufSize;
        pstAECTmpBuff->u32WritePtr = u32AecTmpWrPtr;
    }
    else // (u32AecTmpWrPtr +stRefBufInfo.stRawData.u32BufSize)  >= u32AecTmpBufSize
    {
        memcpy(pAecTmpBuf+u32AecTmpWrPtr, stBufInfo.stRawData.pVirAddr, u32AecTmpBufSize -u32AecTmpWrPtr);
        u32ResidualSize = stBufInfo.stRawData.u32BufSize - (u32AecTmpBufSize -u32AecTmpWrPtr);

        u32AecTmpWrPtr = 0;
        if(u32ResidualSize != 0)
        {
            memcpy(pAecTmpBuf+u32AecTmpWrPtr, stBufInfo.stRawData.pVirAddr+stBufInfo.stRawData.u32BufSize - u32ResidualSize, u32ResidualSize);
            u32AecTmpWrPtr += u32ResidualSize;
        }
        pstAECTmpBuff->u32WritePtr = u32AecTmpWrPtr;
    }

    return s32Ret;
}
#endif

static MI_S32 _MI_AI_WriteAECBuff(_MI_AI_AECRefBuffInfo_t *pstAECBuff, void *pvBuff, MI_U32 u32WriteSize, MI_BOOL bNullFlag)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32Residual = 0;
    MI_U32 u32TmpWriteSize = 0;

    MI_AI_USR_CHECK_POINTER(pstAECBuff);

    if (TRUE != bNullFlag)
    {
       MI_AI_USR_CHECK_POINTER(pvBuff);
    }

    if (u32WriteSize <= pstAECBuff->u32IgnoreSize)
    {
        pstAECBuff->u32IgnoreSize -= u32WriteSize;
        u32TmpWriteSize = 0;
    }
    else
    {
        u32TmpWriteSize = u32WriteSize - pstAECBuff->u32IgnoreSize;
        pstAECBuff->u32IgnoreSize = 0;
    }

    if (TRUE == bNullFlag)
    {
        if (pstAECBuff->u32WritePtr + u32TmpWriteSize >= pstAECBuff->u32MaxSize)
        {
            u32Residual = u32TmpWriteSize - (pstAECBuff->u32MaxSize - pstAECBuff->u32WritePtr);
            memset(pstAECBuff->pvBuff + pstAECBuff->u32WritePtr, 0, pstAECBuff->u32MaxSize - pstAECBuff->u32WritePtr);
            memset(pstAECBuff->pvBuff, 0, u32Residual);
        }
        else
        {
            memset(pstAECBuff->pvBuff + pstAECBuff->u32WritePtr, 0, u32TmpWriteSize);
        }
    }
    else
    {

        if (pstAECBuff->u32WritePtr + u32TmpWriteSize >= pstAECBuff->u32MaxSize)
        {
            u32Residual = u32TmpWriteSize - (pstAECBuff->u32MaxSize - pstAECBuff->u32WritePtr);
            memcpy(pstAECBuff->pvBuff + pstAECBuff->u32WritePtr, pvBuff, pstAECBuff->u32MaxSize - pstAECBuff->u32WritePtr);
            memcpy(pstAECBuff->pvBuff, pvBuff + (pstAECBuff->u32MaxSize - pstAECBuff->u32WritePtr) , u32Residual);
        }
        else
        {
            memcpy(pstAECBuff->pvBuff + pstAECBuff->u32WritePtr, pvBuff, u32TmpWriteSize);
        }
    }

    pstAECBuff->u32WritePtr = (pstAECBuff->u32WritePtr + u32TmpWriteSize) % pstAECBuff->u32MaxSize;

    return s32Ret;
}

static MI_S32 _MI_AI_ReadAECBuff(_MI_AI_AECRefBuffInfo_t *pstAECBuff, void *pvBuff, MI_U32 u32ReadSize)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32Residual = 0;
    MI_U32 u32Diff;

    MI_AI_USR_CHECK_POINTER(pstAECBuff);
    MI_AI_USR_CHECK_POINTER(pvBuff);

    if (pstAECBuff->u32WritePtr >= pstAECBuff->u32ReadPtr)
    {
        u32Diff = pstAECBuff->u32WritePtr - pstAECBuff->u32ReadPtr;
    }
    else
    {
        u32Diff = pstAECBuff->u32WritePtr - pstAECBuff->u32ReadPtr + pstAECBuff->u32MaxSize;
    }

    if (0 == u32Diff)
    {
        memset(pvBuff, 0, u32ReadSize);
    }
    else if (u32Diff >= u32ReadSize)
    {
        if (pstAECBuff->u32ReadPtr + u32ReadSize >= pstAECBuff->u32MaxSize)
        {
            u32Residual = u32ReadSize - (pstAECBuff->u32MaxSize - pstAECBuff->u32ReadPtr);
            memcpy(pvBuff, pstAECBuff->pvBuff + pstAECBuff->u32ReadPtr, pstAECBuff->u32MaxSize - pstAECBuff->u32ReadPtr);
            memcpy(pvBuff + (pstAECBuff->u32MaxSize - pstAECBuff->u32ReadPtr), pstAECBuff->pvBuff, u32Residual);
        }
        else
        {
            memcpy(pvBuff, pstAECBuff->pvBuff + pstAECBuff->u32ReadPtr, u32ReadSize);
        }
        pstAECBuff->u32ReadPtr = (pstAECBuff->u32ReadPtr + u32ReadSize) % pstAECBuff->u32MaxSize;
    }
    else //u32Diff , u32ReadSize
    {
        if (pstAECBuff->u32ReadPtr + u32Diff >= pstAECBuff->u32MaxSize)
        {
            u32Residual = u32Diff - (pstAECBuff->u32MaxSize - pstAECBuff->u32ReadPtr);
            memcpy(pvBuff, pstAECBuff->pvBuff + pstAECBuff->u32ReadPtr, pstAECBuff->u32MaxSize - pstAECBuff->u32ReadPtr);
            memcpy(pvBuff + (pstAECBuff->u32MaxSize - pstAECBuff->u32ReadPtr), pstAECBuff->pvBuff, u32Residual);
        }
        else
        {
            memcpy(pvBuff, pstAECBuff->pvBuff + pstAECBuff->u32ReadPtr, u32Diff);
        }
        memset(pvBuff, 0, u32ReadSize  - u32Diff);
        pstAECBuff->u32ReadPtr = (pstAECBuff->u32ReadPtr + u32Diff) % pstAECBuff->u32MaxSize;
    }

    return s32Ret;
}


#ifndef __KERNEL__
static MI_S32 _MI_AI_GetRefBufByOutputPort(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_SYS_ChnPort_t stAoChnOutputPort0;
    MI_SYS_BufInfo_t stRefBufInfo;
    MI_SYS_BUF_HANDLE hRefHandle;
    MI_U32 u32PtNumPerFrm;
    MI_U32 u32BitwidthByte;
    MI_AUDIO_SampleRate_e eAiSampleRate;
    MI_U64 pu64AoDiffPts;
    MI_U32 u32Tmp;
    MI_U32 u32DiffPtr;
    MI_U64 *pu64RefPts = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64RefPts;
    _MI_AI_AECRefBuffInfo_t *pstAECTmpBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECTmpBuff;
    _MI_AI_AECRefBuffInfo_t *pstAECRefBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECRefBuff;
     void* pAecTmpBuf =  pstAECTmpBuff->pvBuff;
    MI_U32 u32AecTmpBufSize = pstAECTmpBuff->u32MaxSize;
    MI_U32 u32AecTmpRdPtr = pstAECTmpBuff->u32ReadPtr;
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitwidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);

    if (TRUE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bResampleEnable)
    {
        eAiSampleRate = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].eOutResampleRate;
        u32PtNumPerFrm = _gastAiDevInfoUsr[AiDevId].u32ResPtNumPerFrm;
    }
    else
    {
        eAiSampleRate = _gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate;
        u32PtNumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;
    }

    stAoChnOutputPort0.eModId = E_MI_MODULE_ID_AO;
    stAoChnOutputPort0.u32DevId = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].AoDevId;
    stAoChnOutputPort0.u32ChnId = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].AoChn;
    stAoChnOutputPort0.u32PortId = 0;

    s32Ret = MI_SYS_ChnOutputPortGetBuf(&stAoChnOutputPort0 ,&stRefBufInfo, &hRefHandle);
    if(MI_SUCCESS == s32Ret)
    {
        //write referece buffer cnt for dubug, check if reference buffer lose ?
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u32AecRefBufCnt += 1;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hRefBufHandle = hRefHandle;

        //check if reference data playback ?
        if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64RefStartPts == 0 && stRefBufInfo.u64Pts != 0)
        {
            _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64RefStartPts = stRefBufInfo.u64Pts;
            _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAecRefStartRead = FALSE;
        }

        if (0 == stRefBufInfo.u64Pts)
        {
            //reset Ao
            _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64RefStartPts = 0;
            pstAECTmpBuff->u32WritePtr = 0;

            u32DiffPtr = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u32DiffPtr;
            if (u32DiffPtr != 0)
            {
//                DBG_WRN("u32DiffPtr:%d\n", u32DiffPtr);
                u32Tmp = u32DiffPtr % (u32PtNumPerFrm * u32BitwidthByte);

                if (u32AecTmpRdPtr + u32DiffPtr > u32AecTmpBufSize)
                {
                    _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf + u32AecTmpRdPtr, u32AecTmpBufSize - u32AecTmpRdPtr, FALSE);
                    _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf , u32DiffPtr - (u32AecTmpBufSize - u32AecTmpRdPtr), FALSE);
                }
                else
                {
                    _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf + u32AecTmpRdPtr, u32DiffPtr, FALSE);
                }
                pstAECRefBuff->u32IgnoreSize = u32DiffPtr;
            }
            DBG_WRN("AEC Restart\n");
        }

        //check if reference buffer lose? current reference buffer pts - previous buffer pts is stable
        if((stRefBufInfo.u64Pts - *pu64RefPts) != (1000000ULL * u32PtNumPerFrm / (MI_U64)_gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate ))
        {
           // if( (stRefBufInfo.u64Pts != 0) && (*pu64RefPts != 0) )
            if(stRefBufInfo.u64Pts != 0 && *pu64RefPts != 0)
            {
                pu64AoDiffPts = stRefBufInfo.u64Pts - *pu64RefPts;
                DBG_WRN("Ao Current pts: %llu, Previous pts: %llu , Delta-pts: %d !!!\n", stRefBufInfo.u64Pts, *pu64RefPts, pu64AoDiffPts);
            }
        }

        //update reference buffer PTS
        *pu64RefPts = stRefBufInfo.u64Pts;

        //copy reference data to another buffer (pstAECTmpBuff->pvBuff)
        _MI_AI_WriteTmpBuf(AiDevId, AiChn, stRefBufInfo);

        // release this output port buffer
        MI_SYS_ChnOutputPortPutBuf( _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hRefBufHandle);
    }

    return s32Ret;
}
#endif


#ifndef __KERNEL__
static void *_MI_AI_GetRefData(void *pdata)
{
    _MI_AI_GetRefDataInfo_t *pstAiGetRefData = (_MI_AI_GetRefDataInfo_t *)pdata;
    MI_AUDIO_DEV AiDevId = pstAiGetRefData->AiDevId;
    MI_AI_CHN AiChn = pstAiGetRefData->AiChn;

    while(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAiPthreadExit != TRUE)
    {
        _MI_AI_GetRefBufByOutputPort(AiDevId, AiChn);

        usleep(1000 * 10);
    }
}
#endif


static MI_S32 _MI_AI_AECInit(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

#ifndef __KERNEL__
    AudioAecInit  stAecInit;
    AudioAecConfig stAecConfig;
    void* pAecWorkingBuf = NULL;
    AEC_HANDLE hAecHandle;
    MI_U32 u32BitwidthByte;
    void* pAecRefUnitBuf = NULL;
    MI_AUDIO_SampleRate_e eSampleReate;
    MI_AI_VqeConfig_t stAiVqeConfig = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig;
    _MI_AI_AECRefBuffInfo_t *pstAECTmpBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECTmpBuff;
    _MI_AI_AECRefBuffInfo_t *pstAECRefBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECRefBuff;

    if (_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bResampleEnable)
    {
        eSampleReate = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].eOutResampleRate;
    }
    else
    {
        eSampleReate = _gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate;
    }

    stAecInit.point_number = MI_AUDIO_VQE_SAMPLES_UNIT;
    stAecInit.farend_channel = 1;
    stAecInit.nearend_channel = 1;
    MI_AUDIO_VQE_SAMPLERATE_TRANS_TYPE(eSampleReate, stAecInit.sample_rate);

    stAecConfig.comfort_noise_enable =  (IAA_AEC_BOOL)stAiVqeConfig.stAecCfg.bComfortNoiseEnable;
    memcpy(&(stAecConfig.suppression_mode_freq[0]), stAiVqeConfig.stAecCfg.u32AecSupfreq, sizeof(MI_U32)*6);
    memcpy(&(stAecConfig.suppression_mode_intensity[0]), stAiVqeConfig.stAecCfg.u32AecSupIntensity, sizeof(MI_U32)*7);

    /* AEC init */
    pAecWorkingBuf = MI_AI_Malloc(IaaAec_GetBufferSize());
    if (NULL == pAecWorkingBuf)
    {
        DBG_ERR("Malloc IaaAec_GetBuffer failed\n");
        return MI_AI_ERR_NOBUF;
    }

    hAecHandle = IaaAec_Init((char*)pAecWorkingBuf, &stAecInit);
    if(hAecHandle == NULL)
    {
        DBG_ERR("IaaAec_Init FAIL !!!!!!!!!!!!!!!!!!!");
        return MI_AI_ERR_VQE_ERR;
    }

    IaaAec_Config(hAecHandle ,&(stAecConfig));

    //malloc AEC Tmp Buff
    pstAECTmpBuff->u32MaxSize = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm *2* 50;
    pstAECTmpBuff->pvBuff = MI_AI_Malloc(pstAECTmpBuff->u32MaxSize);
    if(NULL == pstAECTmpBuff->pvBuff)
    {
        DBG_ERR("Malloc Aec reference buffer failed\n");
        return MI_AI_ERR_NOBUF;
    }
    memset(pstAECTmpBuff->pvBuff, 0, pstAECTmpBuff->u32MaxSize);
    pstAECTmpBuff->u32ReadPtr = 0;
    pstAECTmpBuff->u32WritePtr = 0;
    pstAECTmpBuff->u32IgnoreSize = 0;

    //malloc AEC ref unit buf
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitwidthByte,  _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    pAecRefUnitBuf = MI_AI_Malloc(_gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm *u32BitwidthByte);
    if(NULL == pAecRefUnitBuf)
    {
        DBG_ERR("Malloc Aec reference unit buffer failed\n");
        return MI_AI_ERR_NOBUF;
    }

    //malloc AEC Ref Buff
    pstAECRefBuff->u32MaxSize = MI_AI_AEC_REF_BUFF_SIZE;
    pstAECRefBuff->pvBuff = MI_AI_Malloc(pstAECRefBuff->u32MaxSize);
    if (NULL == pstAECRefBuff->pvBuff)
    {
        DBG_ERR("Malloc Aec ref buff failed \n");
        return MI_AI_ERR_NOBUF;
    }
    pstAECRefBuff->u32ReadPtr = 0;
    pstAECRefBuff->u32WritePtr = 0;
    pstAECRefBuff->u32IgnoreSize = 0;

    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pAecWorkingBuf = pAecWorkingBuf;
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hAecHandle = hAecHandle;
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pAecRefUnitBuf = pAecRefUnitBuf;
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAecRefStartRead = FALSE;
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u32DiffPtr = 0;

    //create thread
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stGetRefDataInfo.AiChn = AiChn;
    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stGetRefDataInfo.AiDevId = AiDevId;

    s32Ret = pthread_create(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiPthreadId, NULL, _MI_AI_GetRefData, &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stGetRefDataInfo);
    if (0 != s32Ret)
    {
        DBG_WRN("create Ai:%d Chn:%d pthread failed\n", AiDevId, AiChn);
        return MI_AI_ERR_BUSY;
    }
#endif

    return s32Ret;
}

#if 1
static MI_S32 _MI_AI_ReadRefBuf(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_SYS_BufInfo_t stBufInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;

    void* pAecTmpBuf;
    MI_U32 u32AecTmpBufSize;
    MI_U32 u32AecTmpRdPtr;
    MI_U32 u32AecTmpWrPtr;
    MI_U64 u64AecRefPts;
    MI_U32 u32BitwidthByte;
    MI_U64 u64AiPts;
    MI_U64 u64AecRefOffSet;
    void* pAecRefUnitBuf;
    MI_U32 u32ResidualSize;
    MI_U32 u32PtNumPerFrm;
    MI_U32 u32DiffWrRdPtr;
    MI_AUDIO_SampleRate_e eAiSampleRate;
    _MI_AI_AECRefBuffInfo_t *pstAECTmpBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECTmpBuff;
    _MI_AI_AECRefBuffInfo_t *pstAECRefBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECRefBuff;
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitwidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    pAecRefUnitBuf = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pAecRefUnitBuf;

    if (TRUE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bResampleEnable)
    {
        eAiSampleRate = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].eOutResampleRate;
        u32PtNumPerFrm = _gastAiDevInfoUsr[AiDevId].u32ResPtNumPerFrm;
    }
    else
    {
        eAiSampleRate = _gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate;
        u32PtNumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;
    }

    if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64RefStartPts != 0)
    {
        pAecTmpBuf = pstAECTmpBuff->pvBuff;
        u32AecTmpBufSize = pstAECTmpBuff->u32MaxSize;

        if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAecRefStartRead == FALSE)
        {
            u64AecRefPts =  _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64RefStartPts;

            u64AiPts = stBufInfo.u64Pts;

//            DBG_WRN("Ao:%lld, Ai:%lld, Diff:%lld\n", u64AecRefPts, u64AiPts, u64AecRefPts - u64AiPts);
            if(u64AecRefPts > u64AiPts) // AI read data early than AO write data
            {
                u64AecRefOffSet = ((u64AecRefPts - u64AiPts) * u32BitwidthByte * (MI_U32)eAiSampleRate)/1000000ULL ;
                u64AecRefOffSet = (u64AecRefOffSet >> 1) << 1; //align 1 audio sample
                u32AecTmpRdPtr = u32AecTmpBufSize - u64AecRefOffSet;

                if(u64AecRefOffSet > u32PtNumPerFrm*u32BitwidthByte)
                {
                    _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf+u32AecTmpRdPtr, u32PtNumPerFrm*u32BitwidthByte, FALSE);
                    u32AecTmpRdPtr += u32PtNumPerFrm*u32BitwidthByte;
                }
                else //u32AecRefOffSet <= u32PtNumPerFrm*u32BitwidthByte;   u32AecTmpRdPtr need to trun around
                {
                    if(u64AecRefOffSet != 0)
                         _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf+u32AecTmpRdPtr, u64AecRefOffSet, FALSE);

                    u32AecTmpRdPtr = 0;
                    _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf, u32PtNumPerFrm*u32BitwidthByte -u64AecRefOffSet, FALSE);
                    u32AecTmpRdPtr += u32PtNumPerFrm*u32BitwidthByte -u64AecRefOffSet;
                }
                pstAECTmpBuff->u32ReadPtr = u32AecTmpRdPtr;
            }
            else // u64AecRefPts < u64AiPts : AO write data early than AI read data
            {
                u64AecRefOffSet = ((u64AiPts - u64AecRefPts ) * u32BitwidthByte * (MI_U32)_gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate)/1000000ULL ;
                u64AecRefOffSet = (u64AecRefOffSet >> 1) << 1; //align 1 audio sample
                u32AecTmpRdPtr = u64AecRefOffSet;
                _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf+u32AecTmpRdPtr, u32PtNumPerFrm*u32BitwidthByte, FALSE);
                u32AecTmpRdPtr += u32PtNumPerFrm*u32BitwidthByte;

                //MI_PRINT("NearEnd PTS: %llu, > FarEnd PTS: %llu, u64AecRefOffSet: %llu \n",stBufInfo.u64Pts, u64AecRefPts, u64AecRefOffSet);
                pstAECTmpBuff->u32ReadPtr = u32AecTmpRdPtr;
            }
            _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAecRefStartRead = TRUE;
        }
        else //_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAecRefStartRead == TRUE
        {

            u32AecTmpRdPtr = pstAECTmpBuff->u32ReadPtr;
            u32AecTmpWrPtr = pstAECTmpBuff->u32WritePtr;

            //check if u32AecTmpRdPtr + u32PtNumPerFrm*u32BitwidthByte <= u32AecTmpWrPtr
            //if (FALSE == _gbDiffPts)
            {
                if(u32AecTmpWrPtr>=u32AecTmpRdPtr)
                {
                    u32DiffWrRdPtr =  u32AecTmpWrPtr - u32AecTmpRdPtr;
                }
                else // u32AecTmpWrPtr < u32AecTmpRdPtr
                {
                    u32DiffWrRdPtr = u32AecTmpWrPtr - u32AecTmpRdPtr + u32AecTmpBufSize;
                }
                _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u32DiffPtr = u32DiffWrRdPtr;
            }
            if(u32DiffWrRdPtr >= u32PtNumPerFrm*u32BitwidthByte)
            {
                if(u32AecTmpRdPtr+u32PtNumPerFrm*u32BitwidthByte <= u32AecTmpBufSize)
                {
                    _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf+u32AecTmpRdPtr, u32PtNumPerFrm*u32BitwidthByte, FALSE);
                    u32AecTmpRdPtr += u32PtNumPerFrm*u32BitwidthByte;
                }
                else // u32AecTmpRdPtr+u32PtNumPerFrm*u32BitwidthByte > u32AecTmpBufSize
                {
                    _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf+u32AecTmpRdPtr, u32AecTmpBufSize-u32AecTmpRdPtr, FALSE);
                    u32ResidualSize = u32PtNumPerFrm*u32BitwidthByte - (u32AecTmpBufSize-u32AecTmpRdPtr);

                    if(u32ResidualSize != 0)
                    {
                        _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf, u32ResidualSize, FALSE);
                        u32AecTmpRdPtr = u32ResidualSize;
                    }
                }
                pstAECTmpBuff->u32ReadPtr = u32AecTmpRdPtr;

            }
            else // u32DiffWrRdPtr < u32PtNumPerFrm*u32BitwidthByte
            {
                if(u32DiffWrRdPtr > 0)
                {
                    if (u32DiffWrRdPtr + u32AecTmpRdPtr <= u32AecTmpBufSize )
                    {
                        _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf+u32AecTmpRdPtr, u32DiffWrRdPtr, FALSE);
                        _MI_AI_WriteAECBuff(pstAECRefBuff, NULL, u32PtNumPerFrm * u32BitwidthByte - u32DiffWrRdPtr, TRUE);
                        u32AecTmpRdPtr += u32DiffWrRdPtr;
                    }
                    else
                    {
                        _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf+u32AecTmpRdPtr, u32AecTmpBufSize - u32AecTmpRdPtr, FALSE);
                        u32ResidualSize = u32DiffWrRdPtr - (u32AecTmpBufSize - u32AecTmpRdPtr);
                        if (u32ResidualSize != 0)
                        {
                            _MI_AI_WriteAECBuff(pstAECRefBuff, pAecTmpBuf, u32ResidualSize, FALSE);
                        }
                        u32AecTmpRdPtr = u32ResidualSize;
                    }
                     pstAECTmpBuff->u32ReadPtr = u32AecTmpRdPtr;

                    //MI_PRINT("Spk play ending !!! Ref read ptr  == write ptr, u32AecTmpRdPtr: %d, u32AecTmpWrPtr:%d \n",u32AecTmpRdPtr,  u32AecTmpWrPtr);
                }
                else //(u32DiffWrRdPtr <= 0)
                {
                    _MI_AI_WriteAECBuff(pstAECRefBuff, NULL, u32PtNumPerFrm*u32BitwidthByte, TRUE);
                }
            }
        }
    }
    else // reference data do not playback, so set zero data to pAecRefUnitBuf
    {
        _MI_AI_WriteAECBuff(pstAECRefBuff, NULL, u32PtNumPerFrm*u32BitwidthByte, TRUE);
    }

    _MI_AI_ReadAECBuff(pstAECRefBuff, pAecRefUnitBuf, u32PtNumPerFrm*u32BitwidthByte);

    return s32Ret;
}

#endif

static MI_S32 _MI_AI_DoSrc(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, _MI_AI_QueueInfo_t *pstInputQueue, _MI_AI_QueueInfo_t *pstOutputQueue)
{
    MI_S32 s32GetSize= 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8InputBuff = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8InputBuff;
    MI_U8 *pu8OutputBuff = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8OutputBuff;

    MI_S32 s32BitwidthByte;
    SRC_HANDLE hSrcHandle;
    MI_U32 u32SrcUnit;
    MI_S32 s32SrcOutSample;

    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(s32BitwidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);

    hSrcHandle = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hSrcHandle;
    u32SrcUnit = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;

    if (u32SrcUnit < MI_AI_SRC_UNIT)
    {
        u32SrcUnit = MI_AI_SRC_UNIT;
    }
    else if (u32SrcUnit > MI_AI_SRC_UNIT_MAX)
    {
        u32SrcUnit = MI_AI_SRC_UNIT_MAX;
    }
    else
    {
        u32SrcUnit = u32SrcUnit - (u32SrcUnit % MI_AI_SRC_UNIT);
    }

    s32GetSize = u32SrcUnit * s32BitwidthByte;

    while(pstInputQueue->s32Size >= s32GetSize)
    {
        memset(pu8InputBuff, 0, MI_AI_POINT_NUM_MAX);
        memset(pu8OutputBuff, 0, MI_AI_POINT_NUM_MAX);

        s32Ret = _MI_AI_QueueDraw(pstInputQueue, pu8InputBuff, s32GetSize);
        if (MI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        s32SrcOutSample = 0;
#ifndef __KERNEL__
        s32SrcOutSample = IaaSrc_Run(hSrcHandle, (MI_S16 *)pu8InputBuff, (MI_S16 *)pu8OutputBuff, u32SrcUnit);
#endif
        //if (s32SrcOutSample * s32BitwidthByte * u16ChanlNum > MI_AI_POINT_NUM_MAX)
        if (s32SrcOutSample * s32BitwidthByte > MI_AI_POINT_NUM_MAX)
        {
            DBG_ERR("Src out size is too big:%d\n", s32SrcOutSample * s32BitwidthByte);
            return MI_AI_ERR_NOBUF;
        }

        //s32Ret =_MI_AI_QueueInsert(pstOutputQueue, pu8OutputBuff, s32SrcOutSample * s32BitwidthByte * u16ChanlNum);
        s32Ret =_MI_AI_QueueInsert(pstOutputQueue, pu8OutputBuff, s32SrcOutSample * s32BitwidthByte);
        if (MI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_AI_DoAEC(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_SYS_BufInfo_t stAiBufInfo, _MI_AI_QueueInfo_t *pstInputQueue, _MI_AI_QueueInfo_t *pstOutputQueue)
{
    MI_S32 s32Ret = MI_SUCCESS;

    AEC_HANDLE  hAecHandle;
    void* pAecRefUnitBuf;
    MI_U32 u32PtNumPerFrm;
    MI_U32 u32VqeWrPtr;
    MI_U32 u32VqeSampleUnitByte;
    MI_U32 u32VqeOutBytes;
    MI_U32 u32BitwidthByte;
    MI_S32 s32GetSize;
    MI_U8 *pu8InputBuff = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8InputBuff;

    u32VqeWrPtr = 0;
    u32VqeSampleUnitByte = 0;
    u32VqeOutBytes = 0;

    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitwidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    u32PtNumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;

    if(TRUE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bResampleEnable)
    {
        s32GetSize = _gastAiDevInfoUsr[AiDevId].u32ResPtNumPerFrm * u32BitwidthByte;
    }
    else
    {
        s32GetSize = u32PtNumPerFrm*u32BitwidthByte;
    }

    //read reference data to another buffer
    //_MI_AI_GetRefBufByOutputPort(AiDevId, AiChn);

    // read referece data from pstAECTmpBuff->pvBuff to pAecRefUnitBuf
    _MI_AI_ReadRefBuf(AiDevId, AiChn, stAiBufInfo);

    memset(pu8InputBuff, 0, MI_AI_POINT_NUM_MAX);
    s32Ret = _MI_AI_QueueDraw(pstInputQueue, pu8InputBuff, s32GetSize);
    if (MI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    pAecRefUnitBuf = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pAecRefUnitBuf;
    hAecHandle = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hAecHandle;
#ifndef __KERNEL__
    if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSaveFileInfo.bCfg == TRUE)
    {
        //write(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdSinWr, pu8InputBuff, s32GetSize);
        //write(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdRinWr, pAecRefUnitBuf, u32PtNumPerFrm*u32BitwidthByte);
    }

#ifdef DUMP_FILE
    fwrite(pu8InputBuff, s32GetSize, 1, _gpstAiAecBuff);
    fwrite(pAecRefUnitBuf, s32GetSize, 1, _gpstAoAecBuff);
#endif
    u32VqeSampleUnitByte = MI_AUDIO_VQE_SAMPLES_UNIT * u32BitwidthByte;
    u32VqeOutBytes = s32GetSize;
    u32VqeWrPtr = 0;
    while(u32VqeOutBytes)
    {
        IaaAec_Run(hAecHandle, (MI_S16*)(pu8InputBuff + u32VqeWrPtr), (MI_S16*)(pAecRefUnitBuf+ u32VqeWrPtr) );
        u32VqeOutBytes -= u32VqeSampleUnitByte;
        u32VqeWrPtr += u32VqeSampleUnitByte;
    }

#ifdef DUMP_FILE
    fwrite(pu8InputBuff, s32GetSize, 1, _gpstAecOutput);
#endif
    if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSaveFileInfo.bCfg == TRUE)
    {
        //write(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdSouWr, stAiBufInfo.stRawData.pVirAddr, stAiBufInfo.stRawData.u32BufSize);
    }

    s32Ret = _MI_AI_QueueInsert(pstOutputQueue, pu8InputBuff, s32GetSize);
#endif

    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn]._gbAECRestart = FALSE;

    return s32Ret;
}

static MI_S32 _MI_AI_DoVqe(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, _MI_AI_QueueInfo_t *pstInputQueue, _MI_AI_QueueInfo_t *pstOutputQueue)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32GetSize;
    MI_S32 s32BitwidthByte;
    MI_U8 *pu8InputBuff = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8InputBuff;
    APC_HANDLE hApcHandle;

    //s32GetSize = MI_AUDIO_VQE_SAMPLES_UNIT * s32BitwidthByte * u16ChanlNum;
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(s32BitwidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    s32GetSize = MI_AUDIO_VQE_SAMPLES_UNIT * s32BitwidthByte;

    hApcHandle = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hApcHandle;

    if (pstInputQueue->s32Size < s32GetSize)
    {
        return s32Ret;
    }

    while(pstInputQueue->s32Size >= s32GetSize)
    {
        memset(pu8InputBuff, 0, MI_AI_POINT_NUM_MAX);

        s32Ret = _MI_AI_QueueDraw(pstInputQueue, pu8InputBuff, s32GetSize);
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
            return MI_AI_ERR_VQE_ERR;
        }
        s32Ret = _MI_AI_QueueInsert(pstOutputQueue, pu8InputBuff, s32GetSize);
    }

    return s32Ret;
}

static MI_S32 _MI_AI_G711Encoder(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, _MI_AI_QueueInfo_t *pstInputQueue, _MI_AI_QueueInfo_t *pstOutputQueue, MI_AUDIO_AencType_e eG711Type)
{
    MI_S32 s32GetSize= 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8InputBuff = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8InputBuff;
    MI_U8 *pu8OutputBuff = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8OutputBuff;

    memset(pu8InputBuff, 0, MI_AI_POINT_NUM_MAX);
    memset(pu8OutputBuff, 0, MI_AI_POINT_NUM_MAX);

    if (pstInputQueue->s32Size > 0 || pstInputQueue->s32Size < MI_AI_POINT_NUM_MAX)
    {
        s32GetSize = pstInputQueue->s32Size;

        s32Ret = _MI_AI_QueueDraw(pstInputQueue, pu8InputBuff, s32GetSize);
        if (s32Ret != MI_SUCCESS)
        {
            return s32Ret;
        }

#ifndef __KERNEL__
        switch(eG711Type)
        {
            case E_MI_AUDIO_AENC_TYPE_G711A:
                G711Encoder((MI_S16 *)pu8InputBuff, pu8OutputBuff, s32GetSize >> 1, MI_AI_G711A);
                break;
            case E_MI_AUDIO_AENC_TYPE_G711U:
                G711Encoder((MI_S16 *)pu8InputBuff, pu8OutputBuff, s32GetSize >> 1, MI_AI_G711U);
                break;
            default:
                DBG_WRN("G711 Enocder Type not find:%d\n", eG711Type);
                return MI_AI_ERR_NOT_PERM;
        }
#endif
        s32Ret = _MI_AI_QueueInsert(pstOutputQueue, pu8OutputBuff, s32GetSize >> 1);
    }

    return s32Ret;

}

static MI_S32 _MI_AI_G726Encoder(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, _MI_AI_QueueInfo_t *pstInputQueue, _MI_AI_QueueInfo_t *pstOutputQueue, MI_AUDIO_G726Mode_e eG726Mode)
{
    MI_S32 s32WriteSize = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32GetSize = 0;
    MI_U8 *pu8InputBuff = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8InputBuff;
    MI_U8 *pu8OutputBuff = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8OutputBuff;
    g726_state_t *hG726Handle = NULL;

    //g726_16   8bytes ->1bytes
    //g726_24   16bytes->3bytes
    //g726_32   4bytes ->1bytes
    //g726_40   16bytes->5bytes

    if (pstInputQueue->s32Size < MI_AI_AENC_G726_UNIT)
    {
        return s32Ret;
    }

    s32GetSize = pstInputQueue->s32Size;

    if (0 != (s32GetSize % MI_AI_AENC_G726_UNIT) )
    {
         s32GetSize = s32GetSize - (s32GetSize % MI_AI_AENC_G726_UNIT);
    }

    memset(pu8InputBuff, 0, MI_AI_POINT_NUM_MAX);
    memset(pu8OutputBuff, 0, MI_AI_POINT_NUM_MAX);

    s32Ret = _MI_AI_QueueDraw(pstInputQueue, pu8InputBuff, s32GetSize);
    if (MI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    hG726Handle = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hG726Handle;
#ifndef __KERNEL__
    s32WriteSize = g726_encode(hG726Handle, pu8OutputBuff, (MI_S16*) pu8InputBuff, s32GetSize >> 1);
#endif

    if (s32WriteSize <= 0)
    {
        DBG_WRN("s32WriteSize:%d\n", s32WriteSize);
        return MI_AI_ERR_AENC_ERR;
    }

    s32Ret = _MI_AI_QueueInsert(pstOutputQueue, pu8OutputBuff, s32WriteSize);

    return s32Ret;
}

static MI_S32 _MI_AI_DoAenc(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, _MI_AI_QueueInfo_t *pstInputQueue, _MI_AI_QueueInfo_t *pstOutputQueue, MI_AI_AencConfig_t *pstAiAencInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;

    switch(pstAiAencInfo->eAencType)
    {
        case E_MI_AUDIO_AENC_TYPE_G711A:
        case E_MI_AUDIO_AENC_TYPE_G711U:
            s32Ret = _MI_AI_G711Encoder(AiDevId, AiChn, pstInputQueue, pstOutputQueue, pstAiAencInfo->eAencType);
            break;
        case E_MI_AUDIO_AENC_TYPE_G726:
            s32Ret = _MI_AI_G726Encoder(AiDevId, AiChn, pstInputQueue, pstOutputQueue, pstAiAencInfo->stAencG726Cfg.eG726Mode);
            break;
        default:
            DBG_WRN("AencType is not find:%d\n", pstAiAencInfo->eAencType);
            s32Ret = MI_AI_ERR_NOT_PERM;
            break;
    }

    return s32Ret;
}

static MI_S32 _MI_AI_SendToChnOutputPort(MI_AUDIO_DEV AiDevId,MI_AI_CHN AiChn, _MI_AI_QueueInfo_t *pstChnQueue, MI_AUDIO_Frame_t *pstFrm, MI_AUDIO_AecFrame_t *pstAecFrm)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8ChnBuff = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8ChnBuff;
    MI_S32 s32GetSize = 0;
    MI_BOOL bAecEnable = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig.bAecOpen;

    if ( (pstChnQueue->s32Size > 0) && (pstChnQueue->s32Size < MI_AI_POINT_NUM_MAX) )
    {
        memset(pu8ChnBuff, 0, MI_AI_POINT_NUM_MAX);
        s32GetSize = pstChnQueue->s32Size;

        s32Ret = _MI_AI_QueueDraw(pstChnQueue , pu8ChnBuff, s32GetSize);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("_MI_AI_QueueDraw failed\n");
            return s32Ret;
        }

        if (TRUE == bAecEnable)
        {
            pstAecFrm->stRefFrame.u32Len = s32GetSize;
            pstAecFrm->stRefFrame.apVirAddr[0] = pu8ChnBuff;
            pstAecFrm->stRefFrame.apVirAddr[1] = NULL;
        }
        else
        {
            pstFrm->u32Len = s32GetSize;
            pstFrm->apVirAddr[0] = pu8ChnBuff;
            pstFrm->apVirAddr[1] = NULL;
        }
    }

    return s32Ret;
}

//-------------------------------------------------------------------------------------------------
//  global function  prototypes
//-------------------------------------------------------------------------------------------------
MI_S32 MI_AI_SetPubAttr(MI_AUDIO_DEV AiDevId, MI_AUDIO_Attr_t *pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_SetPubAttr_t stSetPubAttr;

    memset(&stSetPubAttr, 0, sizeof(stSetPubAttr));
    stSetPubAttr.AiDevId = AiDevId;
    memcpy(&stSetPubAttr.stAttr, pstAttr, sizeof(MI_AUDIO_Attr_t));
    s32Ret = MI_SYSCALL(MI_AI_SET_PUB_ATTR, &stSetPubAttr);

    if (s32Ret == MI_SUCCESS)
    {
        _gastAiDevInfoUsr[AiDevId].bDevAttrSet = TRUE;
        memcpy(&_gastAiDevInfoUsr[AiDevId].stDevAttr, pstAttr, sizeof(MI_AUDIO_Attr_t));
    }

    return s32Ret;
}

MI_S32 MI_AI_GetPubAttr(MI_AUDIO_DEV AiDevId, MI_AUDIO_Attr_t*pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_GetPubAttr_t stGetPubAttr;

    memset(&stGetPubAttr, 0, sizeof(stGetPubAttr));
    stGetPubAttr.AiDevId = AiDevId;
    s32Ret = MI_SYSCALL(MI_AI_GET_PUB_ATTR, &stGetPubAttr);

    if (s32Ret == MI_SUCCESS)
    {
        memcpy(pstAttr, &stGetPubAttr.stAttr, sizeof(MI_AUDIO_Attr_t));
    }

    return s32Ret;
}

MI_S32 MI_AI_Enable(MI_AUDIO_DEV AiDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_AI_ENABLE, &AiDevId);

    if (s32Ret == MI_SUCCESS)
    {
        _gastAiDevInfoUsr[AiDevId].bDevEnable = TRUE;
    }

#ifndef __KERNEL__
#ifdef DUMP_FILE
    _gpstAiInput = fopen(MI_AI_INPUT_FILE, "w+");
    _gpstAoInput = fopen(MI_AO_INPUT_FILE, "w+");
    _gpstAiAecBuff =  fopen(MI_AI_AEC_BUFF_FILE, "w+");
    _gpstAecOutput = fopen(MI_AI_AEC_FILE, "w+");
    _gpstAoAecBuff = fopen(MI_AO_AEC_BUFF_FILE, "w+");
#endif

#endif
    return s32Ret;
}

MI_S32 MI_AI_Disable(MI_AUDIO_DEV AiDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_AI_DISABLE, &AiDevId);

    if (s32Ret == MI_SUCCESS)
    {
        _gastAiDevInfoUsr[AiDevId].bDevEnable = FALSE;
    }

#ifndef __KERNEL__
#ifdef DUMP_FILE
    fclose(_gpstAiInput);
    fclose(_gpstAoInput);
    fclose(_gpstAiAecBuff);
    fclose(_gpstAoAecBuff);
    fclose(_gpstAecOutput);
#endif
#endif
    return s32Ret;
}

MI_S32 MI_AI_EnableChn(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_EnableChn_t stEnableChn;
    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;

    if (TRUE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable)
    {
        return MI_SUCCESS;
    }

    memset(&stEnableChn, 0, sizeof(stEnableChn));
    stEnableChn.AiDevId = AiDevId;
    stEnableChn.AiChn = AiChn;
    s32Ret = MI_SYSCALL(MI_AI_ENABLE_CHN, &stEnableChn);

    if(s32Ret == MI_SUCCESS)
    {
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable = TRUE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32ChnId = AiChn;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bPortEnable = TRUE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64AiOldPts = 0;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64AiDiffPts = 0;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAiPtsLeap = FALSE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAiPthreadExit = FALSE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn]._gbAECRestart = FALSE;
#ifndef __KERNEL__
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8InputBuff = MI_AI_Malloc(MI_AI_POINT_NUM_MAX);
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8OutputBuff = MI_AI_Malloc(MI_AI_POINT_NUM_MAX);
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8ChnBuff = MI_AI_Malloc(MI_AI_POINT_NUM_MAX);
        MI_AI_USR_CHECK_POINTER(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8InputBuff);
        MI_AI_USR_CHECK_POINTER(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8OutputBuff);
        MI_AI_USR_CHECK_POINTER(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8ChnBuff);

#endif

        _MI_AI_QueueInit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stChnQueue, u32NumPerFrm * u32BitWidthByte * u16ChanlNum *8);
    }

    return s32Ret;
}

MI_S32 MI_AI_DisableChn(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_DisableChn_t stDisableChn;

    if (FALSE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable)
    {
        return MI_SUCCESS;
    }

    memset(&stDisableChn, 0, sizeof(stDisableChn));
    stDisableChn.AiDevId = AiDevId;
    stDisableChn.AiChn = AiChn;
    s32Ret = MI_SYSCALL(MI_AI_DISABLE_CHN, &stDisableChn);

    if (s32Ret == MI_SUCCESS)
    {
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable = FALSE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bPortEnable = FALSE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeEnable = FALSE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bResampleEnable = FALSE;

        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAecRefStartRead = FALSE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64RefStartPts = 0;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64RefPts = 0;

#ifndef __KERNEL__
       //pthread_join(stAiPthreadId, NULL);

       if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSaveFileInfo.bCfg == TRUE)
       {
            if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdSinWr >0)
                close(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdSinWr);
            if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdRinWr >0)
                close(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdRinWr);
            if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdSouWr >0)
                close(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdSouWr);
       }
       //MI_PRINT("u32AecRefBufCnt : %d \n", _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u32AecRefBufCnt);

        MI_AI_Free(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8InputBuff);
        MI_AI_Free(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8OutputBuff);
        MI_AI_Free(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8ChnBuff);
#endif

       _MI_AI_QueueDeinit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stChnQueue);
    }

    return s32Ret;
}

MI_S32 MI_AI_GetFrame(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_Frame_t *pstFrm, MI_AUDIO_AecFrame_t *pstAecFrm , MI_S32 s32MilliSec)
{
    MI_S32 s32Ret = MI_SUCCESS;

    //get output port buffer
    MI_SYS_BufInfo_t stAiBufInfo;
    MI_SYS_BUF_HANDLE hAiHandle;
    MI_SYS_ChnPort_t stAiChnOutputPort0;
    MI_U64 *pu64AiOldPts = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64AiOldPts;
    MI_U64 *pu64AiDiffPts = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].u64AiDiffPts;
    MI_U32 u32PtNumPerFrm;
    MI_U32 u32AiOffset;
    _MI_AI_AECRefBuffInfo_t *pstAECTmpBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECTmpBuff;

    MI_U32 u32AecTmpBufSize = pstAECTmpBuff->u32MaxSize ;
    MI_U32 u32AecTmpRdPtr = pstAECTmpBuff->u32ReadPtr;
    MI_AI_AencConfig_t *pstAiAencInfo = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiAencConfig;

    //Enable Flag
    MI_BOOL bResampleEnbale = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bResampleEnable;
    MI_BOOL bAecEnable = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig.bAecOpen;
    MI_BOOL bVqeEnable = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeEnable;
    MI_BOOL bAencEnable = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAencEnable;

    //Queue
    _MI_AI_QueueInfo_t *pstSrcQueue = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSrcQueue;
    _MI_AI_QueueInfo_t *pstAECQueue = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECQueue;
    _MI_AI_QueueInfo_t *pstVqeQuque = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stVqeQuque;
    _MI_AI_QueueInfo_t *pstAencQueue = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAencQueue;
    _MI_AI_QueueInfo_t *pstChnQueue = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stChnQueue;

    // check input parameter
    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);
    MI_AI_USR_CHECK_POINTER(pstFrm);
    //MI_AI_USR_CHECK_POINTER(pstAecFrm);

    if(TRUE != _gastAiDevInfoUsr[AiDevId].bDevEnable)
        return MI_AI_ERR_NOT_ENABLED;

    if(TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable)
        return MI_AI_ERR_NOT_ENABLED;

    if(TRUE == bAecEnable)
    {
        MI_AI_USR_CHECK_POINTER(pstAecFrm);
    }

    /* data flow
    1, resample -> Aec -> Vqe -> Encode
        pstSrcQueue->pstAECQueue->pstVqeInputQueue->pstAencQueue->pstChnQueue

    2, resample -> Aec -> Vqe
        pstSrcQueue->pstAECQueue->pstVqeInputQueue->pstChnQueue

    3, resample -> Aec -> Encode
        pstSrcQueue->pstAECQueue->pstAencQueue->pstChnQueue

    4, resample -> Vqe -> Encode
        pstSrcQueue->pstVqeQueue->pstAencQueue->pstChnQueue

    5, resample -> Aec
        pstSrcQueue->pstAECQueue->pstChnQueue

    6, resample -> Vqe
        pstSrcQueue->pstVqeQueue->pstChnQueue

    7, resample -> Enocde
        pstSrcQueue->pstAencQueue->pstChnQueue

    8, resample
        pstSrcQueue->pstChnQueue

    9,  Aec -> Vqe -> Encode
        pstAencQueue->pstVqeQueue->pstAencQueue->pstChnQueue

    10, Aec -> Vqe
        pstAECQueue->pstVqeQueue->pstChnQueue

    11, Aec -> Enocde
        pstAECQueue->pstAencQueue->pstChnQueue

    12, Aec
        pstAECQueue->pstChnQueue

    13, Vqe -> Enocde
        pstVqeQueue->pstAencQueue->pstChnQueue

    14, Vqe
        pstVqeQueue->pstChnQueue

    15, Enocde
        pstAencQueue->pstChnQueue

    16, pcm
    */

     //_MI_AI_GetRefBufByOutputPort(AiDevId, AiChn);

    memset(pstFrm, 0, sizeof(MI_AUDIO_Frame_t));

    stAiChnOutputPort0.eModId = E_MI_MODULE_ID_AI;
    stAiChnOutputPort0.u32DevId = AiDevId;
    stAiChnOutputPort0.u32ChnId = AiChn;
    stAiChnOutputPort0.u32PortId = 0;

    s32Ret = MI_SYS_ChnOutputPortGetBuf(&stAiChnOutputPort0, &stAiBufInfo, &hAiHandle);
    if (MI_SUCCESS == s32Ret)
    {
        //MI_PRINT("Ai Get\n");
        pstFrm->eBitwidth = _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth;
        pstFrm->eSoundmode = _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode;
        pstFrm->u64TimeStamp = stAiBufInfo.u64Pts;

#ifndef __KERNEL__
#ifdef DUMP_FILE
        fwrite(stAiBufInfo.stRawData.pVirAddr, stAiBufInfo.stRawData.u32BufSize, 1, _gpstAiInput);
#endif
#endif

        u32PtNumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;

        if((stAiBufInfo.u64Pts - *pu64AiOldPts) != (1000000ULL * u32PtNumPerFrm / (MI_U64)_gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate ))
        {
            if (*pu64AiOldPts != 0)
            {
                *pu64AiDiffPts = stAiBufInfo.u64Pts - *pu64AiOldPts;
                _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAiPtsLeap = TRUE;

                DBG_WRN("Ai Current pts: %llu, Previous pts: %llu , Delta-pts: %d !!!\n", stAiBufInfo.u64Pts, *pu64AiOldPts, *pu64AiDiffPts);

                //TO DO if do resample need to check
                //u32AiOffset = (*pu64AiDiffPts / (1000ULL * u32PtNumPerFrm / (MI_U64)_gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate )) / 1000 * u32PtNumPerFrm * 2;
                u32AiOffset = *pu64AiDiffPts * (MI_U64)_gastAiDevInfoUsr[AiDevId].stDevAttr.eSamplerate / 1000000ULL * 2;
                //DBG_WRN("u32AiOffset:%d\n", u32AiOffset);
                u32AecTmpRdPtr += u32AiOffset;
                u32AecTmpRdPtr -= u32PtNumPerFrm * 2;
                u32AecTmpRdPtr %= u32AecTmpBufSize;
                pstAECTmpBuff->u32ReadPtr = u32AecTmpRdPtr;
            }
        }

        //store old pts
        *pu64AiOldPts = stAiBufInfo.u64Pts;

        if (TRUE == bAecEnable)
        {
            pstAecFrm->stRefFrame.eBitwidth = _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth;
            pstAecFrm->stRefFrame.eSoundmode = _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode;
            pstAecFrm->stRefFrame.u64TimeStamp = stAiBufInfo.u64Pts;
        }

        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hBufHandle = hAiHandle;

        //get data
        if (bResampleEnbale)
        {
            s32Ret = _MI_AI_QueueInsert(pstSrcQueue, (MI_U8 *)stAiBufInfo.stRawData.pVirAddr, stAiBufInfo.stRawData.u32BufSize);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_QueueInsert failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
        else if (bAecEnable)
        {
            s32Ret= _MI_AI_QueueInsert(pstAECQueue, (MI_U8 *)stAiBufInfo.stRawData.pVirAddr, stAiBufInfo.stRawData.u32BufSize);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_QueueInsert failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
        else if (bVqeEnable)
        {
            s32Ret = _MI_AI_QueueInsert(pstVqeQuque, (MI_U8 *)stAiBufInfo.stRawData.pVirAddr, stAiBufInfo.stRawData.u32BufSize);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_QueueInsert failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
        else if (bAencEnable)
        {
            s32Ret = _MI_AI_QueueInsert(pstAencQueue, (MI_U8 *)stAiBufInfo.stRawData.pVirAddr, stAiBufInfo.stRawData.u32BufSize);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_QueueInsert failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }

        pstFrm->u32Len = stAiBufInfo.stRawData.u32BufSize;
        pstFrm->apVirAddr[0] = stAiBufInfo.stRawData.pVirAddr;
        pstFrm->apVirAddr[1] = NULL;


        //do Resample
        if (bResampleEnbale && bAecEnable)
        {
            s32Ret = _MI_AI_DoSrc(AiDevId, AiChn, pstSrcQueue, pstAECQueue);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoSrc failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
        else if (bResampleEnbale && bVqeEnable)
        {
            s32Ret = _MI_AI_DoSrc(AiDevId, AiChn, pstSrcQueue, pstVqeQuque);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoSrc failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
        else if (bResampleEnbale && bAencEnable)
        {
            s32Ret = _MI_AI_DoSrc(AiDevId, AiChn, pstSrcQueue, pstAencQueue);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoSrc failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
        else if (bResampleEnbale)
        {
            s32Ret = _MI_AI_DoSrc(AiDevId, AiChn, pstSrcQueue, pstChnQueue);
            //DBG_WRN("_MI_AI_DoSrc\n");
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoSrc failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }

        //do Aec
        if (bAecEnable && bVqeEnable)
        {
            s32Ret = _MI_AI_DoAEC(AiDevId, AiChn, stAiBufInfo, pstAECQueue, pstVqeQuque);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoAEC failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
        else if (bAecEnable && bAencEnable)
        {
            s32Ret = _MI_AI_DoAEC(AiDevId, AiChn, stAiBufInfo, pstAECQueue, pstAencQueue);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoAEC failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
        else if (bAecEnable)
        {
            s32Ret = _MI_AI_DoAEC(AiDevId, AiChn, stAiBufInfo, pstAECQueue, pstChnQueue);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoAEC failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }

        //do Vqe
        if (bVqeEnable && bAencEnable)
        {
            s32Ret = _MI_AI_DoVqe(AiDevId, AiChn, pstVqeQuque, pstAencQueue);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoVqe failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
        else if (bVqeEnable)
        {
            s32Ret = _MI_AI_DoVqe(AiDevId, AiChn, pstVqeQuque, pstChnQueue);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoVqe failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }

        //do Aenc
        if (bAencEnable)
        {
            s32Ret = _MI_AI_DoAenc(AiDevId, AiChn, pstAencQueue, pstChnQueue, pstAiAencInfo);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_DoAenc failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }

        if (bResampleEnbale || bAecEnable || bVqeEnable || bAencEnable)
        {
            s32Ret = _MI_AI_SendToChnOutputPort(AiDevId, AiChn, pstChnQueue, pstFrm, pstAecFrm);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("_MI_AI_SendToChnOutputPort failed\n");
                MI_SYS_ChnOutputPortPutBuf(hAiHandle);
                return s32Ret;
            }
        }
    }

    return s32Ret;
}

MI_S32 MI_AI_ReleaseFrame(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_Frame_t *pstFrm, MI_AUDIO_AecFrame_t *pstAecFrm)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if(TRUE != _gastAiDevInfoUsr[AiDevId].bDevEnable)
        return MI_AI_ERR_NOT_ENABLED;

    if(TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable)
        return MI_AI_ERR_NOT_ENABLED;

    MI_SYS_ChnOutputPortPutBuf(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hBufHandle);
    pstFrm->apVirAddr[0] = NULL;
    pstFrm->apVirAddr[1] = NULL;
    pstFrm->u32Len = 0;

    //release reference buffer
    if(pstAecFrm != NULL && pstAecFrm->bValid == TRUE)
    {
        //MI_SYS_ChnOutputPortPutBuf( _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hRefBufHandle);
        pstAecFrm->stRefFrame.apVirAddr[0] = NULL;
        pstAecFrm->stRefFrame.apVirAddr[1] = NULL;
        pstAecFrm->stRefFrame.u32Len = 0;
    }

    return s32Ret;
}

MI_S32 MI_AI_SetChnParam(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_ChnParam_t *pstChnParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_SetChnParam_t stSetChnParam;

    memset(&stSetChnParam, 0, sizeof(stSetChnParam));
    stSetChnParam.AiDevId = AiDevId;
    stSetChnParam.AiChn = AiChn;
    memcpy(&stSetChnParam.stChnParam, pstChnParam, sizeof(MI_AI_ChnParam_t));

    s32Ret = MI_SYSCALL(MI_AI_SET_CHN_PARAM, &stSetChnParam);
    return s32Ret;
}

MI_S32 MI_AI_GetChnParam(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_ChnParam_t *pstChnParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_GetChnParam_t stGetChnParam;

    memset(&stGetChnParam, 0, sizeof(stGetChnParam));
    stGetChnParam.AiDevId = AiDevId;
    stGetChnParam.AiChn = AiChn;

    s32Ret = MI_SYSCALL(MI_AI_GET_CHN_PARAM, &stGetChnParam);
    if (s32Ret == MI_SUCCESS)
    {
        memcpy(pstChnParam, &stGetChnParam.stChnParam, sizeof(MI_AI_ChnParam_t));
    }

    return s32Ret;
}

MI_S32 MI_AI_EnableReSmp(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_SampleRate_e eOutSampleRate)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_EnableReSmp_t stEnableResmp;
    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;

    ///check input parameter
    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);
    MI_AI_USR_CHECK_SAMPLERATE(eOutSampleRate);

    if(TRUE != _gastAiDevInfoUsr[AiDevId].bDevEnable)
        return MI_AI_ERR_NOT_ENABLED;

    if(TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable)
        return MI_AI_ERR_NOT_ENABLED;


    // put resample status to kernel mode for DebugFs
    memset(&stEnableResmp, 0, sizeof(stEnableResmp));
    stEnableResmp.AiDevId = AiDevId;
    stEnableResmp.AiChn= AiChn;
    stEnableResmp.eOutSampleRate= eOutSampleRate;
    s32Ret = MI_SYSCALL(MI_AI_ENABLE_RESMP, &stEnableResmp);

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;

    if(s32Ret == MI_SUCCESS)
    {
        // Enable channel of AI device resample
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bResampleEnable = TRUE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].eOutResampleRate = eOutSampleRate;

        /* SRC parameter setting */
        s32Ret = _MI_AI_ReSmpInit(AiDevId, AiChn, eOutSampleRate);

        _MI_AI_QueueInit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSrcQueue, u32NumPerFrm * u32BitWidthByte * u16ChanlNum * 8);
    }

    return s32Ret;
}

MI_S32 MI_AI_DisableReSmp(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_DisableReSmp_t stDisableResmp;
    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;

    // check input parameter
    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);

    //put resample status to kernel mode for DebugFs
    memset(&stDisableResmp, 0, sizeof(stDisableResmp));
    stDisableResmp.AiDevId = AiDevId;
    stDisableResmp.AiChn = AiChn;
    s32Ret = MI_SYSCALL(MI_AI_DISABLE_RESMP, &stDisableResmp);

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;

    if(s32Ret == MI_SUCCESS)
    {
        // Disable channel of AI device resample
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bResampleEnable = FALSE;

#ifndef __KERNEL__
        if(NULL != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pSrcWorkingBuf)
        {
            MI_AI_Free(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pSrcWorkingBuf);
            _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pSrcWorkingBuf = NULL;
        }

        if(NULL != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8SrcOutBuf)
        {
            MI_AI_Free(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8SrcOutBuf);
            _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pu8SrcOutBuf = NULL;
        }
        IaaSrc_Release( _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hSrcHandle);
#endif

        _MI_AI_QueueDeinit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSrcQueue);
    }

    return s32Ret;
}

MI_S32 MI_AI_SetVqeAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AI_VqeConfig_t *pstVqeConfig)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_SetVqeAttr_t stSetVqeAttr;

    // check input parameter
    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);
    MI_AI_USR_CHECK_POINTER(pstVqeConfig);
    //MI_AO_USR_CHECK_DEV(AoDevId); //ToDo
    //MI_AO_USR_CHECK_CHN(AoChn); //ToDo

    // check if Vqe of AI device channel is disable ?
    if( FALSE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeEnable)
        return MI_AI_ERR_NOT_PERM;

    // check if AI device channel is enable ?
    if( TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable)
        return MI_AI_ERR_NOT_ENABLED;

    // check if Vqe Configure legal ?

    // put VQE status to kernel mode for DebugFs
    memset(&stSetVqeAttr, 0, sizeof(stSetVqeAttr));
    stSetVqeAttr.AiDevId = AiDevId;
    stSetVqeAttr.AiChn= AiChn;
    stSetVqeAttr.AoChn = AoChn;
    stSetVqeAttr.AoDevId = AoDevId;
    memcpy(&stSetVqeAttr.stVqeConfig, pstVqeConfig, sizeof(MI_AI_VqeConfig_t));
    s32Ret = MI_SYSCALL(MI_AI_SET_VQE_ATTR, &stSetVqeAttr);

    if(s32Ret == MI_SUCCESS)
    {
        // save Vqe configure of AI device channel
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeAttrSet = TRUE;
        memcpy(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig, pstVqeConfig, sizeof(MI_AI_VqeConfig_t));
        // save AO device ID/Chan for AEC ?
         _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].AoDevId = AoDevId;
         _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].AoChn = AoChn;
    }

    return s32Ret;
}

MI_S32 MI_AI_GetVqeAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn,  MI_AI_VqeConfig_t *pstVqeConfig)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);
    MI_AI_USR_CHECK_POINTER(pstVqeConfig);

    // check if Vqe attribute of AO device channel is set ?
    if(TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeAttrSet)
        return MI_AI_ERR_NOT_PERM;

    // load Vqe config
    memcpy(pstVqeConfig, &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig, sizeof(MI_AI_VqeConfig_t));

    return s32Ret;
}

MI_S32 MI_AI_EnableVqe(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_EnableVqe_t stEnableVqe;
    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;

    // check input parameter
    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);

    //
    if( TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeAttrSet)
        return MI_AI_ERR_NOT_PERM;

    if( TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable)
        return MI_AI_ERR_NOT_ENABLED;

    if(TRUE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeEnable)
        return s32Ret;

     // put VQE status to kernel mode for DebugFs
    memset(&stEnableVqe, 0, sizeof(stEnableVqe));
    stEnableVqe.AiDevId = AiDevId;
    stEnableVqe.AiChn= AiChn;
    s32Ret = MI_SYSCALL(MI_AI_ENABLE_VQE, &stEnableVqe);

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;

    if(s32Ret == MI_SUCCESS)
    {
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeEnable = TRUE;

        _MI_AI_QueueInit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stVqeQuque, u32NumPerFrm * u32BitWidthByte * u16ChanlNum *8);

        // need to call VQE init here
        s32Ret = _MI_AI_VqeInit(AiDevId, AiChn);
        if (s32Ret != MI_SUCCESS)
        {
            return s32Ret;
        }

        if (TRUE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig.bAecOpen)
        {
            _MI_AI_QueueInit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECQueue, u32NumPerFrm * u32BitWidthByte * u16ChanlNum *8);
            s32Ret  = _MI_AI_AECInit(AiDevId, AiChn);
        }
    }

    return s32Ret;
}

MI_S32 MI_AI_DisableVqe(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_DisableVqe_t stDisableVqe;
    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;

#ifndef __KERNEL__
    _MI_AI_AECRefBuffInfo_t *pstAECTmpBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECTmpBuff;
    _MI_AI_AECRefBuffInfo_t *pstAECRefBuff = &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECRefBuff;
#endif
    // check input parameter
    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);

    if(FALSE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeEnable)
        return s32Ret;

    // put vqe status to kernel mode for DebugFs
    memset(&stDisableVqe, 0, sizeof(stDisableVqe));
    stDisableVqe.AiDevId = AiDevId;
    stDisableVqe.AiChn = AiChn;
    s32Ret = MI_SYSCALL(MI_AI_DISABLE_VQE, &stDisableVqe);

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;
    if(s32Ret == MI_SUCCESS)
    {

#ifndef __KERNEL__
         _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAiPthreadExit = TRUE;
        if (TRUE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig.bAecOpen)
        {
            pthread_join(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiPthreadId, NULL);
        }

        if(NULL != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pApcWorkingBuf)
        {
            MI_AI_Free( _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pApcWorkingBuf);
            _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pApcWorkingBuf = NULL;
        }

        IaaApc_Free(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hApcHandle);

        if(NULL != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pAecWorkingBuf)
        {
            MI_AI_Free(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pAecWorkingBuf);
            _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].pAecWorkingBuf = NULL;
        }

        if(NULL != pstAECTmpBuff->pvBuff)
        {
            MI_AI_Free(pstAECTmpBuff->pvBuff);
            pstAECTmpBuff->pvBuff = NULL;
        }

        if(NULL !=  pstAECRefBuff->pvBuff)
        {
            MI_AI_Free( pstAECRefBuff->pvBuff);
            pstAECRefBuff->pvBuff = NULL;
        }

        if(NULL != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hAecHandle)
            IaaAec_Free(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hAecHandle);

#endif
        _MI_AI_QueueDeinit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stVqeQuque);
        _MI_AI_QueueDeinit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAECQueue);

        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeEnable = FALSE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig.bAecOpen = FALSE;
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeAttrSet = FALSE; // ???

        //memset(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiVqeConfig, 0, sizeof(MI_AI_VqeConfig_t));
    }

    return s32Ret;
}

MI_S32 MI_AI_SetAencAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_AencConfig_t *pstAencConfig)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_AI_SetAencAttr_t stSetAencAttr;

    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);
    MI_AI_USR_CHECK_POINTER(pstAencConfig);

       // check if Adec of AO device channel is disable ?
    if( FALSE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAencEnable)
    {
        DBG_WRN("Dev%d Chn%d Adec is already enable\n", AiDevId, AiChn);
        return MI_AI_ERR_NOT_PERM;
    }

    // check if AO device channel is enable ?
    if( TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable)
    {
        DBG_WRN("Dev%d Chn%d is not enable\n", AiDevId, AiChn);
        return MI_AI_ERR_NOT_ENABLED;
    }

    memset(&stSetAencAttr, 0, sizeof(MI_AI_SetAencAttr_t));
    stSetAencAttr.AiChn = AiChn;
    stSetAencAttr.AiDevId = AiDevId;
    memcpy(&stSetAencAttr.stAencConfig, pstAencConfig, sizeof(MI_AI_AencConfig_t));
    s32Ret = MI_SYSCALL(MI_AI_SET_AENC_ATTR, &stSetAencAttr);

    if (MI_SUCCESS == s32Ret)
    {
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAencAttrSet = TRUE;
        memcpy(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAiAencConfig, pstAencConfig, sizeof(MI_AI_AencConfig_t));
    }

    return s32Ret;
}

MI_S32 MI_AI_GetAencAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_AencConfig_t *pstAencConfig)
{
    MI_S32 s32Ret =MI_SUCCESS;

    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);
    MI_AI_USR_CHECK_POINTER(pstAencConfig);

    if (TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAencAttrSet)
    {
        DBG_WRN("Dev%d Chn%d Aenc Attr is not set\n", AiDevId, AiChn);
        return MI_AI_ERR_NOT_PERM;
    }

    memcpy(pstAencConfig, &_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn], sizeof(MI_AI_AecConfig_t));

    return s32Ret;
}

MI_S32 MI_AI_EnableAenc(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_EnableAenc_t stEnableAenc;

    MI_U16 u16ChanlNum;
    MI_U32 u32NumPerFrm;
    MI_U32 u32BitWidthByte;


    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);

    if (TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAencAttrSet)
    {
         DBG_WRN("Dev%d Chn%d Aenc Attr is not set\n", AiDevId, AiChn);
         return MI_AI_ERR_NOT_PERM;
    }

    if (TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bChanEnable)
    {
        DBG_WRN("Dev%d Chn%d is not enable\n", AiDevId, AiChn);
        return MI_AI_ERR_NOT_ENABLED;
    }

    if (TRUE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAencEnable)
    {
        return s32Ret;
    }

    MI_AUDIO_USR_TRANS_EMODE_TO_CHAN(u16ChanlNum, _gastAiDevInfoUsr[AiDevId].stDevAttr.eSoundmode);
    MI_AUDIO_USR_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, _gastAiDevInfoUsr[AiDevId].stDevAttr.eBitwidth);
    u32NumPerFrm = _gastAiDevInfoUsr[AiDevId].stDevAttr.u32PtNumPerFrm;

    memset(&stEnableAenc, 0, sizeof(MI_AI_EnableAenc_t));
    stEnableAenc.AiChn = AiChn;
    stEnableAenc.AiDevId = AiDevId;

    s32Ret = MI_SYSCALL(MI_AI_ENABLE_AENC, &stEnableAenc);

    if (MI_SUCCESS == s32Ret)
    {
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAencEnable = TRUE;

        _MI_AI_AencInit(AiDevId, AiChn);

        _MI_AI_QueueInit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAencQueue, u32NumPerFrm * u32BitWidthByte * u16ChanlNum * 8);
    }

    return s32Ret;
}

MI_S32 MI_AI_DisableAenc(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_DisableAenc_t stDisableAenc;

    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);

    if (FALSE == _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAencEnable)
    {
        return s32Ret;
    }

    memset(&stDisableAenc, 0, sizeof(MI_AI_DisableAenc_t));
    stDisableAenc.AiChn = AiChn;
    stDisableAenc.AiDevId = AiDevId;
    s32Ret = MI_SYSCALL(MI_AI_DISABLE_AENC, &stDisableAenc);

    if (MI_SUCCESS == s32Ret)
    {
        _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bAencEnable = FALSE;
        _MI_AI_QueueDeinit(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stAencQueue);

#ifndef __KERNEL__
        MI_AI_Free(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].hG726Handle);
#endif

    }

    return s32Ret;
}

MI_S32 MI_AI_ClrPubAttr(MI_AUDIO_DEV AiDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_AI_CLR_PUB_ATTR, &AiDevId);

    return s32Ret;
}

MI_S32 MI_AI_SaveFile(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_SaveFileInfo_t *pstSaveFileInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;


    MI_S8 szSinFilePath[128];
    MI_S8 szRinFilePath[128];
    MI_S8 szSoutFilePath[128];

    // check input parameter
    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);
    MI_AI_USR_CHECK_POINTER(pstSaveFileInfo);

    // check if AI Vqe enable ?
    if(TRUE != _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].bVqeEnable)
        return MI_AI_ERR_NOT_PERM;

    memcpy(&_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSaveFileInfo, pstSaveFileInfo, sizeof(MI_AUDIO_SaveFileInfo_t));

    sprintf(szSinFilePath, "%s/sin%d.pcm", _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSaveFileInfo.szFilePath, AiChn);
    sprintf(szRinFilePath, "%s/rin%d.pcm", _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSaveFileInfo.szFilePath, AiChn);
    sprintf(szSoutFilePath, "%s/sou%d.pcm", _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSaveFileInfo.szFilePath, AiChn);
    if(_gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].stSaveFileInfo.bCfg == TRUE)
    {
#ifndef __KERNEL__
       _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdSinWr = open(szSinFilePath, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
       _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdRinWr = open(szRinFilePath, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
       _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32FdSouWr = open(szSoutFilePath, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#endif
    }

    return s32Ret;
}

MI_S32 MI_AI_SetVqeVolume(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_S32 s32VolumeDb)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AI_USR_CHECK_DEV(AiDevId);
    MI_AI_USR_CHECK_CHN(AiChn);

    // check s32VolumeDb range ???

    _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32VolumeDb = s32VolumeDb;

    return s32Ret;
}

MI_S32 MI_AI_GetVqeVolume(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_S32 *ps32VolumeDb)
{
    MI_S32 s32Ret = MI_SUCCESS;

     // check input parameter
     MI_AI_USR_CHECK_DEV(AiDevId);
     MI_AI_USR_CHECK_CHN(AiChn);
     MI_AI_USR_CHECK_POINTER(ps32VolumeDb);

     //
     *ps32VolumeDb = _gastAiDevInfoUsr[AiDevId].astChanInfo[AiChn].s32VolumeDb;

     return s32Ret;
}

EXPORT_SYMBOL(MI_AI_SetPubAttr);
EXPORT_SYMBOL(MI_AI_GetPubAttr);
EXPORT_SYMBOL(MI_AI_Enable);
EXPORT_SYMBOL(MI_AI_Disable);
EXPORT_SYMBOL(MI_AI_EnableChn);
EXPORT_SYMBOL(MI_AI_DisableChn);
EXPORT_SYMBOL(MI_AI_GetFrame);
EXPORT_SYMBOL(MI_AI_ReleaseFrame);
EXPORT_SYMBOL(MI_AI_SetChnParam);
EXPORT_SYMBOL(MI_AI_GetChnParam);
EXPORT_SYMBOL(MI_AI_EnableReSmp);
EXPORT_SYMBOL(MI_AI_DisableReSmp);
EXPORT_SYMBOL(MI_AI_SetVqeAttr);
EXPORT_SYMBOL(MI_AI_GetVqeAttr);
EXPORT_SYMBOL(MI_AI_EnableVqe);
EXPORT_SYMBOL(MI_AI_DisableVqe);
EXPORT_SYMBOL(MI_AI_ClrPubAttr);
EXPORT_SYMBOL(MI_AI_SaveFile);
EXPORT_SYMBOL(MI_AI_SetVqeVolume);
EXPORT_SYMBOL(MI_AI_GetVqeVolume);
EXPORT_SYMBOL(MI_AI_SetAencAttr);
EXPORT_SYMBOL(MI_AI_GetAencAttr);
EXPORT_SYMBOL(MI_AI_EnableAenc);
EXPORT_SYMBOL(MI_AI_DisableAenc);

