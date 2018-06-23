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
#include "mi_ao_impl.h"

#include <linux/string.h>
#if !USE_CAM_OS
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <asm/uaccess.h>

#endif
#include "mi_common_datatype.h"
#include "mi_common.h"
#include "mi_print.h"
#include "mi_sys.h"
#include "mi_sys_internal.h"
#include "mi_syscfg.h"

//#include "mi_ao_impl.h"
#include "mi_aio_datatype.h"
#include "mi_ao_datatype.h"
#include "mi_ao_datatype_internal.h"
#include "mhal_audio_datatype.h"
#include "mhal_audio.h"
#include "mi_sys_proc_fs_internal.h"

#if USE_CAM_OS
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
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

#define MI_AO_CHECK_DEV(AoDevId) \
    if(AoDevId < 0 || AoDevId >= MI_AO_DEV_NUM_MAX) \
    {   \
        DBG_ERR("AoDevId is invalid! AoDevId = %u.\n", AoDevId);   \
        return MI_AO_ERR_INVALID_DEVID;   \
    }

#define MI_AO_CHECK_CHN(Aochn)  \
    if(Aochn < 0 || Aochn >= MI_AO_CHAN_NUM_MAX) \
    {   \
        DBG_ERR("Aochn is invalid! Aochn = %u.\n", Aochn);   \
        return MI_AO_ERR_INVALID_CHNID;   \
    }

#define MI_AO_CHECK_POINTER(pPtr)  \
    if(NULL == pPtr)  \
    {   \
        DBG_ERR("Invalid parameter! NULL pointer.\n");   \
        return MI_AO_ERR_NULL_PTR;   \
    }

#define MI_AO_CHECK_SAMPLERATE(eSamppleRate)    \
    if( (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_8000) && (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_16000) &&\
        (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_32000) && (eSamppleRate != E_MI_AUDIO_SAMPLE_RATE_48000) ) \
    { \
        DBG_ERR("Sample Rate is illegal = %u.\n", eSamppleRate);   \
        return MI_AO_ERR_ILLEGAL_PARAM;   \
    }

#define MI_AO_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, eWidth)          \
    switch(eWidth) \
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

#if USE_CAM_OS
    #define MI_AO_ThreadWakeUp CamOsThreadWakeUp
    #define MI_AO_ThreadStop   CamOsThreadStop
    #define MI_AO_ThreadShouldStop() (CamOsThreadShouldStop() == CAM_OS_OK)
#else
    #define MI_AO_ThreadWakeUp wake_up_process
    #define MI_AO_ThreadStop   kthread_stop
    #define MI_AO_ThreadShouldStop() (kthread_should_stop())

#endif


//==== for kthread porting ====
#if USE_CAM_OS
#define MI_AO_CreateThreadWrapper(pfnStartRoutine) \
    void * _##pfnStartRoutine (void* pArg) \
    { \
        (void)pfnStartRoutine(pArg); \
        return NULL; \
    }
#define MI_AO_CreateThread(ptThread, szName, pfnStartRoutine, pArg) \
    _MI_AO_CreateThread(ptThread, szName, _##pfnStartRoutine, pArg)

MI_S32 _MI_AO_CreateThread(MI_AO_Thread_t *ptThread, char* szName, void *(*pfnStartRoutine)(void *), void* pArg)
{
    CamOsThreadAttrb_t stAttr = {0, 0};
    CamOsRet_e eRet;
    eRet = CamOsThreadCreate(ptThread, &stAttr, pfnStartRoutine, pArg);
    if (CAM_OS_OK != eRet)
    {
        return -1;
    }
    return MI_SUCCESS;
}
#else
#define MI_AO_CreateThreadWrapper(pfnStartRoutine)
MI_S32 MI_AO_CreateThread(MI_AO_Thread_t *ptThread, char* szName, int (*pfnStartRoutine)(void *), void* pArg)
{
    *ptThread = kthread_create(pfnStartRoutine, pArg, szName);
    if (IS_ERR(*ptThread))
    {
        DBG_ERR("Fail to create AO thread.\n");
        return -1;
    }
    return MI_SUCCESS;
}
#endif


//#define AO_DUMP_FILE (1)

#ifdef AO_DUMP_FILE
#define MI_AO_IMPL_DUMP_FILE "/mnt/AI_File/ao_impl.pcm"
static  struct file  *_gpstAoImplFile;
#endif

//=============================================================================
// Data type definition
//=============================================================================



//=============================================================================
// Variable definition
//=============================================================================
static _MI_AO_DevInfo_t _gastAoDevInfo[MI_AO_DEV_NUM_MAX] ={
    {
        .bDevEnable = FALSE,
        .bDevAttrSet = FALSE,
        .AoDevId = 0,
        .s32VolumeDb = 0,
        .bMuteEnable = FALSE,
        .hDevSysHandle = MI_HANDLE_NULL,
        .astChanInfo[0] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .u32WriteTotalFrmCnt = 0,
        },
    },
    {
        .bDevEnable = FALSE,
        .bDevAttrSet = FALSE,
        .AoDevId = 1,
        .s32VolumeDb = 0,
        .bMuteEnable = FALSE,
        .hDevSysHandle = MI_HANDLE_NULL,
        .astChanInfo[0] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .u32WriteTotalFrmCnt =0,
        },
    },
    {
        .bDevEnable = FALSE,
        .bDevAttrSet = FALSE,
        .AoDevId = 2,
        .s32VolumeDb = 0,
        .bMuteEnable = FALSE,
        .hDevSysHandle = MI_HANDLE_NULL,
        .astChanInfo[0] = {
            .bChanEnable = FALSE,
            .bPortEnable = FALSE,
            .bResampleEnable = FALSE,
            .bVqeEnable = FALSE,
            .bVqeAttrSet =FALSE,
            .u32WriteTotalFrmCnt = 0,
        },
    },
    {
        .bDevEnable = FALSE,
        .bDevAttrSet = FALSE,
        .AoDevId = 3,
        .s32VolumeDb = 0,
        .bMuteEnable = FALSE,
        .hDevSysHandle = MI_HANDLE_NULL,
        .astChanInfo[0] = {
        .bChanEnable = FALSE,
        .bPortEnable = FALSE,
        .bResampleEnable = FALSE,
        .bVqeEnable = FALSE,
        .bVqeAttrSet =FALSE,
        .u32WriteTotalFrmCnt = 0,
        },
    },
};

static _MI_AO_TimerInfo_t _gastAoTimerInfo[MI_AO_DEV_NUM_MAX];
static _MI_AO_DumpPcmInfo_t _gastAoDumpPcmInfo[MI_AO_DEV_NUM_MAX];

//=============================================================================
// Local function definition
//=============================================================================

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_AO_PROCFS_DEBUG ==1)
static MI_S32 _MI_AO_IMPL_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;

    _MI_AO_DevInfo_t* pstAoDevInfo  = (_MI_AO_DevInfo_t*)pUsrData;
    MI_U8 szWorkMode[15] = {'\0'};
    MI_U8 szBitWidth[15] = {'\0'};
    MI_U8 szSoundMode[15] = {'\0'};
    MI_U32 u32AttrSampleRate = (MI_U32)pstAoDevInfo->stDevAttr.eSamplerate;
    MI_BOOL bMuteEnable = pstAoDevInfo->bMuteEnable;
    MI_S32 s32VolumeDb = pstAoDevInfo->s32VolumeDb;
    MI_U32 u32PtNumPerFrm = pstAoDevInfo->stDevAttr.u32PtNumPerFrm;

    switch (pstAoDevInfo->stDevAttr.eWorkmode)
    {
        case E_MI_AUDIO_MODE_I2S_MASTER:
            strcpy(szWorkMode,"i2s-mas");
            break;

        case E_MI_AUDIO_MODE_I2S_SLAVE:
            strcpy(szWorkMode, "i2s-sla");
            break;

        case E_MI_AUDIO_MODE_TDM_MASTER:
            strcpy(szWorkMode, "tdm-mas");
            break;

        default:
            strcpy(szWorkMode, "not-set");
            break;
    }

    if(pstAoDevInfo->stDevAttr.eBitwidth == E_MI_AUDIO_BIT_WIDTH_16)
        strcpy(szBitWidth, "16bit");
    else if(pstAoDevInfo->stDevAttr.eBitwidth == E_MI_AUDIO_BIT_WIDTH_24)
        strcpy(szBitWidth, "24bit");
    else
        strcpy(szBitWidth, "not-set");

    if(pstAoDevInfo->stDevAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_MONO)
        strcpy(szSoundMode, "mono");
    else if(pstAoDevInfo->stDevAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_STEREO)
        strcpy(szSoundMode, "stereo");
    else
        strcpy(szSoundMode, "not-set");


    handle.OnPrintOut(handle,"\n======================================Private AO%d Info ======================================\n", u32DevId);
    handle.OnPrintOut(handle,"-----Start AO Dev%d Attr------------------------------------------------------------------------\n", u32DevId);
    handle.OnPrintOut(handle,"AoDev  WorkMode  SampR  BitWidth  SondMod  PtNumPerFrm\n");
    handle.OnPrintOut(handle,"%5d  %8s  %5d  %8s  %7s  %11d \n",pstAoDevInfo->AoDevId, szWorkMode, u32AttrSampleRate, szBitWidth, szSoundMode, u32PtNumPerFrm);
    handle.OnPrintOut(handle,"bMute  VolumedB  \n");
    handle.OnPrintOut(handle,"%5d  %8d  \n", bMuteEnable, s32VolumeDb);
    handle.OnPrintOut(handle,"-----End AO Dev%d Attr--------------------------------------------------------------------------\n", u32DevId);
    handle.OnPrintOut(handle,"\n");

    return s32Ret;

}

static MI_S32 _MI_AO_IMPL_OnDumpChannelAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    _MI_AO_DevInfo_t* pstAoDevInfo  = (_MI_AO_DevInfo_t*)pUsrData;
    _MI_AO_ChanInfo_t  stChanInfo = pstAoDevInfo->astChanInfo[0];
    MI_AUDIO_DEV    AoDevId = pstAoDevInfo->AoDevId;
    MI_AO_CHN   AoChn;
    MI_U32 u32OutSampleRate = (MI_U32)pstAoDevInfo->stDevAttr.eSamplerate;
    MI_BOOL  bResampleEnable;
    MI_AUDIO_SampleRate_e   eInResampleRate;
    MI_BOOL bVqeEnable;
    MI_AO_VqeConfig_t   stAoVqeConfig;
    MI_AO_AdecConfig_t  stAoAdecConfig;
    _MI_AO_IMPL_QueueInfo_t stAoUsrQueueInfo;
    MI_U64 u64RunTime;
    MI_U64 u64GetTotalSize;
    MI_U32 u32WriteTotalFrmCnt;
    struct timespec stAoHwCurrentTime;

    MI_U8 szAnrSpeed[15] = {'\0'};
    MI_U8 szAdecMode[15] = {'\0'};
    MI_U8 szAdecType[15] = {'\0'};
    MI_U8 szSoundMode[15] = {'\0'};

    AoChn = stChanInfo.s32ChnId;
    bResampleEnable = stChanInfo.bResampleEnable;
    eInResampleRate = stChanInfo.eInResampleRate;
    bVqeEnable = stChanInfo.bVqeEnable;
    stAoVqeConfig = stChanInfo.stAoVqeConfig;
    stAoAdecConfig = stChanInfo.stAoAdecConfig;
    stAoUsrQueueInfo = stChanInfo.stUsrQueueInfo;

    if (E_MI_AUDIO_NR_SPEED_LOW == stAoVqeConfig.stAnrCfg.eNrSpeed)
    {
        strcpy(szAnrSpeed, "speed-low");
    }
    else if (E_MI_AUDIO_NR_SPEED_MID == stAoVqeConfig.stAnrCfg.eNrSpeed)
    {
        strcpy(szAnrSpeed, "speed-mid");
    }
    else if (E_MI_AUDIO_NR_SPEED_HIGH == stAoVqeConfig.stAnrCfg.eNrSpeed)
    {
        strcpy(szAnrSpeed, "speed-high");
    }
    else
    {
        strcpy(szAnrSpeed, "speed-notset");
    }

    switch(stAoAdecConfig.eAdecType)
    {
        case E_MI_AUDIO_ADEC_TYPE_G726:
            strcpy(szAdecType, "g726");
            break;
        case E_MI_AUDIO_ADEC_TYPE_G711A:
            strcpy(szAdecType, "g711a");
            break;
        case E_MI_AUDIO_ADEC_TYPE_G711U:
            strcpy(szAdecType, "g711u");
            break;
        default:
            strcpy(szAdecType, "notset");
            break;
    }

    if (E_MI_AUDIO_ADEC_TYPE_G726 == stAoAdecConfig.eAdecType)
    {
        switch(stAoAdecConfig.stAdecG726Cfg.eG726Mode)
        {
            case E_MI_AUDIO_G726_MODE_16:
                strcpy(szAdecMode, "g726_16");
                break;
            case E_MI_AUDIO_G726_MODE_24:
                strcpy(szAdecMode, "g726_24");
                break;
            case E_MI_AUDIO_G726_MODE_32:
                strcpy(szAdecMode, "g726_32");
                break;
            case E_MI_AUDIO_G726_MODE_40:
                strcpy(szAdecMode, "g726_40");
                break;
            default:
                strcpy(szAdecMode, "notset");
                break;
        }

        switch(stAoAdecConfig.stAdecG726Cfg.eSoundmode)
        {
            case E_MI_AUDIO_SOUND_MODE_MONO:
                strcpy(szSoundMode, "mono");
                break;
            case E_MI_AUDIO_SOUND_MODE_STEREO:
                strcpy(szSoundMode, "stero");
                break;
            default:
                strcpy(szSoundMode, "notset");
                break;
        }
    }
    else
    {
        switch(stAoAdecConfig.stAdecG711Cfg.eSoundmode)
        {
            case E_MI_AUDIO_SOUND_MODE_MONO:
                strcpy(szSoundMode, "mono");
                break;
            case E_MI_AUDIO_SOUND_MODE_STEREO:
                strcpy(szSoundMode, "stero");
                break;
            default:
                strcpy(szSoundMode, "notset");
                break;
        }
    }

    ktime_get_ts(&stAoHwCurrentTime);
    u64RunTime = stAoHwCurrentTime.tv_sec - pstAoDevInfo->astChanInfo[AoChn].u64StartTime;
    u64GetTotalSize = pstAoDevInfo->astChanInfo[AoChn].u64GetTotalSize;
    u32WriteTotalFrmCnt = pstAoDevInfo->astChanInfo[AoChn].u32WriteTotalFrmCnt;

    handle.OnPrintOut(handle,"-----Start AO CHN%d STATUS----------------------------------------------------------------------\n", AoChn);
    handle.OnPrintOut(handle,"AoDev  AoChn  bReSmp  InSampR  OutSampR \n");
    handle.OnPrintOut(handle,"%5d  %5d  %6d  %7d  %8d \n",AoDevId, AoChn, bResampleEnable, eInResampleRate, u32OutSampleRate);
    handle.OnPrintOut(handle,"AoDev  AoChn    TotalFrmCnt      TotalSize        RunTime\n");
    handle.OnPrintOut(handle,"%5d  %5d  %13d  %13lld  %13lld\n", AoDevId, AoChn, u32WriteTotalFrmCnt, u64GetTotalSize, u64RunTime);

    handle.OnPrintOut(handle,"-----Start AO CHN%d Usr Queue STATUS------------------------------------------------------------\n", AoChn);
    handle.OnPrintOut(handle,"AoDev  AoChn  MaxSize  RemainSize      TotalSize        RunTime\n");
    handle.OnPrintOut(handle,"%5d  %5d  %7d  %10d  %13lld  %13lld\n", AoDevId, AoChn,
                                                                     stAoUsrQueueInfo.s32Max,
                                                                     stAoUsrQueueInfo.s32Size,
                                                                     stAoUsrQueueInfo.u64GetTotalSize,
                                                                     stAoUsrQueueInfo.u64RunTime);

    // Vqe status
    handle.OnPrintOut(handle,"-----AO CHN%d Vqe STATUS------------------------------------------------------------------------\n", AoChn);
    handle.OnPrintOut(handle,"AoDev  AoChn  bVqe  WorkRate  PoiNum \n");
    handle.OnPrintOut(handle,"%5d  %5d  %4d  %8d  %6d \n",AoDevId, AoChn, bVqeEnable, stAoVqeConfig.s32WorkSampleRate, stAoVqeConfig.s32FrameSample);

     // Anr Status
    handle.OnPrintOut(handle,"-----AO CHN%d Anr STATUS------------------------------------------------------------------------\n", AoChn);
    handle.OnPrintOut(handle,"AoDev  AoChn  bAnr  bUsr       Speed  Intensity  SmoothLevel\n");
    handle.OnPrintOut(handle,"%5d  %5d  %4d  %4d  %10s  %9d  %11d\n",AoDevId,
                                                                    AoChn,
                                                                   stAoVqeConfig.bAnrOpen,
                                                                   stAoVqeConfig.stAnrCfg.bUsrMode,
                                                                   szAnrSpeed,
                                                                   stAoVqeConfig.stAnrCfg.u32NrIntensity,
                                                                   stAoVqeConfig.stAnrCfg.u32NrSmoothLevel);

    // Eq Status
    handle.OnPrintOut(handle,"-----AO CHN%d Eq TATUS--------------------------------------------------------------------------\n", AoChn);
    handle.OnPrintOut(handle,"AoDev  AoChn  bEq  bUsr  100Hz  200Hz  250Hz  350Hz  500Hz  800Hz  1.2KHz  2.5KHz  4KHz  8KHz  \n");
    handle.OnPrintOut(handle,"%5d  %5d  %3d  %4d  %5d  %5d  %5d  %5d  %5d  %5d  %6d  %6d  %4d  %4d  \n",
        AoDevId, AoChn, stAoVqeConfig.bEqOpen, stAoVqeConfig.stEqCfg.bUsrMode,
        stAoVqeConfig.stEqCfg.stEqGain.s16EqGain100Hz, stAoVqeConfig.stEqCfg.stEqGain.s16EqGain200Hz, stAoVqeConfig.stEqCfg.stEqGain.s16EqGain250Hz,
        stAoVqeConfig.stEqCfg.stEqGain.s16EqGain350Hz, stAoVqeConfig.stEqCfg.stEqGain.s16EqGain500Hz, stAoVqeConfig.stEqCfg.stEqGain.s16EqGain800Hz,
        stAoVqeConfig.stEqCfg.stEqGain.s16EqGain1200Hz, stAoVqeConfig.stEqCfg.stEqGain.s16EqGain2500Hz, stAoVqeConfig.stEqCfg.stEqGain.s16EqGain4000Hz,
        stAoVqeConfig.stEqCfg.stEqGain.s16EqGain8000Hz);

     // Hpf Status
    handle.OnPrintOut(handle,"-----AO CHN%d Hpf STATUS------------------------------------------------------------------------\n", AoChn);
    handle.OnPrintOut(handle,"AoDev  AoChn  bHpf  bUsr  HpfFreq  \n");
    handle.OnPrintOut(handle,"%5d  %5d  %4d  %4d  %7d  \n",AoDevId, AoChn, stAoVqeConfig.bHpfOpen, stAoVqeConfig.stHpfCfg.bUsrMode, stAoVqeConfig.stHpfCfg.eHpfFreq);

    // Agc Status
    handle.OnPrintOut(handle,"-----AO CHN%d Agc STATUS------------------------------------------------------------------------\n", AoChn);
    handle.OnPrintOut(handle,"AoDev  AoChn  bAgc  bUsr  AttackTime  ReleaseTime  CompressionRatio  DropGainMax  \n");
    handle.OnPrintOut(handle,"%5d  %5d  %4d  %4d  %10d  %11d  %16d  %11d   \n", AoDevId, AoChn,
                                                                        stAoVqeConfig.bAgcOpen,
                                                                        stAoVqeConfig.stAgcCfg.bUsrMode,
                                                                        (MI_S32)stAoVqeConfig.stAgcCfg.u32AttackTime,
                                                                        stAoVqeConfig.stAgcCfg.u32ReleaseTime,
                                                                        stAoVqeConfig.stAgcCfg.u32CompressionRatio,
                                                                        stAoVqeConfig.stAgcCfg.u32DropGainMax);

    handle.OnPrintOut(handle,"GainInit  GainMin  GainMax  NoiseGateAttenuationDb  NoiseGateDb  TargetLevelDb  \n");
    handle.OnPrintOut(handle,"%8d  %7d  %7d  %22d  %11d  %13d  \n", stAoVqeConfig.stAgcCfg.stAgcGainInfo.s32GainInit,
                                                                    stAoVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMin,
                                                                    stAoVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMax,
                                                                    stAoVqeConfig.stAgcCfg.u32NoiseGateAttenuationDb,
                                                                    stAoVqeConfig.stAgcCfg.s32NoiseGateDb,
                                                                    stAoVqeConfig.stAgcCfg.s32TargetLevelDb);

    handle.OnPrintOut(handle,"-----AO CHN%d Adec STATUS-----------------------------------------------------------------------\n", AoChn);
    if (E_MI_AUDIO_ADEC_TYPE_G726 == stAoAdecConfig.eAdecType)
    {
        handle.OnPrintOut(handle, "AoDev  AoChn    Type       Mode    SondMod      SampR \n");
        handle.OnPrintOut(handle, "%5d  %5d    %4s    %4s    %7s      %5d \n", AoDevId, AoChn,
                                                        szAdecType,
                                                        szAdecMode,
                                                        szSoundMode,
                                                        stAoAdecConfig.stAdecG726Cfg.eSamplerate);
    }
    else
    {
        handle.OnPrintOut(handle, "AoDev  AoChn     Type    SondMod    SampR\n");
        handle.OnPrintOut(handle, "%5d  %5d    %4s    %7s    %5d\n", AoDevId, AoChn,
                                                        szAdecType,
                                                        szSoundMode,
                                                        stAoAdecConfig.stAdecG711Cfg.eSamplerate);
    }

    handle.OnPrintOut(handle,"-----End AO Chn%d STATUS------------------------------------------------------------------------\n", AoChn);

    return s32Ret;
}

static MI_S32 _MI_AO_IMPL_OnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;

    return s32Ret;
}

static MI_S32 _MI_AO_IMPL_OnDumpOutputPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;

    return s32Ret;
}

static MI_S32 _MI_AO_IMPL_OnHelp(MI_SYS_DEBUG_HANDLE_t  handle,MI_U32  u32DevId,void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    handle.OnPrintOut(handle, "setaomute [ON, OFF];         Enable/Disable AO mute.\n");
    handle.OnPrintOut(handle, "setaovolume [-114 ~ +12dB];  set AO volume.\n");
    handle.OnPrintOut(handle, "checksize [ON, OFF];         checksize pcm size per second\n");
    handle.OnPrintOut(handle, "dump [Path] [Count];         dump pcm data\n");
    return s32Ret;
}


static MI_S32 _MI_AO_ProcSetAoMute(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    _MI_AO_DevInfo_t* pstAoDevInfo  = (_MI_AO_DevInfo_t*)pUsrData;
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

     if (argc > 1)
    {
        if (strcmp(argv[1], "ON") == 0)
        {
            pstAoDevInfo->bMuteEnable= TRUE;
        }
        else if (strcmp(argv[1], "OFF") == 0)
        {
            pstAoDevInfo->bMuteEnable = FALSE;
        }
        else
        {
            handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
            s32Ret = MI_AO_ERR_NOT_SUPPORT;
        }
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "setaomute [ON, OFF];              Enable/Disable AO mute.\n");
        s32Ret = MI_AO_ERR_NOT_SUPPORT;
    }

    return s32Ret;
}

static MI_S32 _MI_AO_ProcSetAoVolume(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    _MI_AO_DevInfo_t* pstAoDevInfo  = (_MI_AO_DevInfo_t*)pUsrData;
    MI_S32 s32VolumeDb;
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

     if (argc > 1)
    {
        s32VolumeDb = simple_strtol(argv[1], NULL, 10);
        pstAoDevInfo->s32VolumeDb = s32VolumeDb;
        s32Ret = MHAL_AUDIO_SetGainOut(pstAoDevInfo->AoDevId, (MI_S16)s32VolumeDb);
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "setaovolume [-114 ~ +12dB];            set AO volume.\n");
        s32Ret = MI_AO_ERR_NOT_SUPPORT;
    }

    return s32Ret;
}

static void _MI_AO_TimerFunc(unsigned long data)
{
    MI_AUDIO_DEV AoDevId = data;
    MI_U32 u32Size = 0;

    u32Size = _gastAoTimerInfo[AoDevId].u32CurrentSize - _gastAoTimerInfo[AoDevId].u32OldSize;
    printk("MI_AO Dev%d Size Per Second: %d\n", AoDevId, u32Size);
    _gastAoTimerInfo[AoDevId].u32OldSize = _gastAoTimerInfo[AoDevId].u32CurrentSize;

    mod_timer(&_gastAoTimerInfo[AoDevId].stTimer, jiffies + HZ); //1s

}

static MI_S32 _MI_AO_ProcCheckSize(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    _MI_AO_DevInfo_t* pstAoDevInfo  = (_MI_AO_DevInfo_t*)pUsrData;
    MI_AUDIO_DEV AoDevId = pstAoDevInfo->AoDevId;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (TRUE != _gastAoDevInfo[AoDevId].bDevEnable)
    {
        handle.OnPrintOut(handle, "MI_AO Dev%d is not enable. \n", AoDevId);
        return MI_AO_ERR_NOT_ENABLED;
    }

    if (argc > 1)
    {
        if(strncmp(argv[1], "ON", 2) == 0)
        {
            if (FALSE != _gastAoTimerInfo[AoDevId].bTimerEnable)
            {
                handle.OnPrintOut(handle, "MI_AO Dev%d check size is running. \n", AoDevId);
                return s32Ret;
            }

            _gastAoTimerInfo[AoDevId].bTimerEnable = TRUE;
            _gastAoTimerInfo[AoDevId].u32CurrentSize = 0;
            _gastAoTimerInfo[AoDevId].u32OldSize = 0;

            //init kernel timer
            init_timer(&_gastAoTimerInfo[AoDevId].stTimer);
            _gastAoTimerInfo[AoDevId].stTimer.function = _MI_AO_TimerFunc;
            _gastAoTimerInfo[AoDevId].stTimer.data = AoDevId;
            mod_timer(&_gastAoTimerInfo[AoDevId].stTimer, jiffies + HZ); //1s

            handle.OnPrintOut(handle, "MI_AO Dev%d check size ON success. \n", AoDevId);
        }
        else if (strncmp(argv[1], "OFF", 3) == 0)
        {
            if (TRUE != _gastAoTimerInfo[AoDevId].bTimerEnable)
            {
                 handle.OnPrintOut(handle, "MI_AO Dev%d check size is not running. \n", AoDevId);
                 return s32Ret;
            }

            _gastAoTimerInfo[AoDevId].bTimerEnable = FALSE;
            _gastAoTimerInfo[AoDevId].u32CurrentSize = 0;
            _gastAoTimerInfo[AoDevId].u32OldSize = 0;

            del_timer(&_gastAoTimerInfo[AoDevId].stTimer);

            handle.OnPrintOut(handle, "MI_AO Dev%d check size OFF success. \n", AoDevId);
        }
        else
        {
            handle.OnPrintOut(handle, "checkszie [ON/OFF]; Enable/Disable  Check data size per second .\n");
            s32Ret = MI_AO_ERR_NOT_SUPPORT;
        }
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command \n", argc);
        handle.OnPrintOut(handle, "checkszie [ON/OFF]; Enable/Disable  Check data size per second .\n");
        s32Ret = MI_AO_ERR_NOT_SUPPORT;
    }

    return s32Ret;
}

#ifdef AO_DUMP_FILE
static MI_S32 _MI_AO_WriteFile(struct file *pstWriteFile, MI_S8 *ps8Buff, MI_U32 u32Len)
{
    mm_segment_t old_fs;

    MI_SYS_BUG_ON(NULL == pstWriteFile);

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    pstWriteFile->f_op->write(pstWriteFile, ps8Buff, u32Len, &pstWriteFile->f_pos);

    set_fs(old_fs);

    return MI_SUCCESS;
}
#endif

static MI_S32 _MI_AO_WriteFilePcm(_MI_AO_DumpPcmInfo_t *pstDumpInfo, MI_S8 *ps8Buff, MI_U32 u32Len)
{
    mm_segment_t old_fs;
    struct file *pstWriteFile = pstDumpInfo->pstFile;

    MI_SYS_BUG_ON(NULL == pstWriteFile);

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    pstWriteFile->f_op->write(pstWriteFile, ps8Buff, u32Len, &pstWriteFile->f_pos);

    set_fs(old_fs);

    pstDumpInfo->u32Count += 1;
    pstDumpInfo->u32TotalSize += u32Len;

    printk("Count:%d, Size: %d, TotalSize:%d\n", pstDumpInfo->u32Count, u32Len, pstDumpInfo->u32TotalSize);

    if (pstDumpInfo->u32Count >= pstDumpInfo->u32MaxCount)
    {
        pstDumpInfo->bDumpEnable = FALSE;
        pstDumpInfo->u32Count = 0;
        pstDumpInfo->u32TotalSize = 0;
        filp_close(pstDumpInfo->pstFile, NULL);
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_AO_ProcDumpPcm(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    _MI_AO_DevInfo_t* pstAoDevInfo = (_MI_AO_DevInfo_t*)pUsrData;
    MI_AUDIO_DEV AoDevId = pstAoDevInfo->AoDevId;
    MI_U8 au8Path[64] = {0};
    MI_U32 u32OutSampleRate = (MI_U32)pstAoDevInfo->stDevAttr.eSamplerate;
    MI_U8 au8SoundMode[15] = {0};
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (TRUE != _gastAoDevInfo[AoDevId].bDevEnable)
    {
        handle.OnPrintOut(handle, "MI_AO Dev%d is not enable. \n", AoDevId);
        return MI_AO_ERR_NOT_ENABLED;
    }

    if (TRUE == _gastAoDumpPcmInfo[AoDevId].bDumpEnable)
    {
        handle.OnPrintOut(handle, "MI_AO Dev%d dump is running. \n", AoDevId);
        return s32Ret;
    }

    if(pstAoDevInfo->stDevAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_MONO)
        strcpy(au8SoundMode, "MONO");
    else if(pstAoDevInfo->stDevAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_STEREO)
        strcpy(au8SoundMode, "STERO");

    if (argc < 2)
    {
        handle.OnPrintOut(handle, "Unsupport command \n", argc);
        handle.OnPrintOut(handle, "dump [Path] [Count] ; Dump pcm data .\n");

        return MI_AO_ERR_NOT_SUPPORT;
    }

    if ( '/'== argv[1][strlen(argv[1]) - 1])
    {
       sprintf(au8Path, "%sDev%d_%d_%s.pcm", argv[1], AoDevId, u32OutSampleRate, au8SoundMode);
    }
    else
    {
        sprintf(au8Path, "%s/Dev%d_%d_%s.pcm", argv[1], AoDevId, u32OutSampleRate, au8SoundMode);
    }

    kstrtou32(argv[2], 0, &_gastAoDumpPcmInfo[AoDevId].u32MaxCount);

    handle.OnPrintOut(handle, "MaxCount:%d\n", _gastAoDumpPcmInfo[AoDevId].u32MaxCount);
    handle.OnPrintOut(handle, "Path:%s\n", au8Path);

    //open file
    _gastAoDumpPcmInfo[AoDevId].pstFile = filp_open(au8Path, O_RDWR | O_CREAT, 0644);
    if (IS_ERR(_gastAoDumpPcmInfo[AoDevId].pstFile))
    {
        handle.OnPrintOut(handle, "Open Failed:%s\n", au8Path);

        return MI_AO_ERR_ILLEGAL_PARAM;
    }

    _gastAoDumpPcmInfo[AoDevId].bDumpEnable = TRUE;

    return s32Ret;
}


#endif

//------------------------------------------------------------------------------
/// @brief create AO device
/// @param[in]  AoDevId: AO device ID.
/// @return MI_SUCCESS: succeed in creating AO device .
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
//------------------------------------------------------------------------------
static MI_S32 _MI_AO_CreateDevice(MI_AUDIO_DEV AoDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    mi_sys_ModuleDevInfo_t stModInfo;
    mi_sys_ModuleDevBindOps_t stAoOps;
    MI_SYS_DRV_HANDLE hDevSysHandle;
    MS_U32 u32BlkSize;

#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
#endif

     //check input parameter
    MI_AO_CHECK_DEV(AoDevId);

    memset(&stModInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId = E_MI_MODULE_ID_AO;
    stModInfo.u32DevId = AoDevId;
    stModInfo.u32DevChnNum = 1;
    stModInfo.u32InputPortNum = 1; // for output speaker
    stModInfo.u32OutputPortNum = 1; // for reference data of AEC

    memset(&stAoOps, 0x0, sizeof(stAoOps));
    stAoOps.OnBindInputPort   = NULL; //_MI_AI_OnBindChnnInputCallback; ???
    stAoOps.OnUnBindInputPort = NULL; // _MI_AI_OnUnBindChnnInputCallback; ???

#ifdef MI_SYS_PROC_FS_DEBUG
    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
#if (MI_AO_PROCFS_DEBUG ==1)
    pstModuleProcfsOps.OnDumpDevAttr = _MI_AO_IMPL_OnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = _MI_AO_IMPL_OnDumpChannelAttr;
    pstModuleProcfsOps.OnDumpInputPortAttr = _MI_AO_IMPL_OnDumpInputPortAttr;
    pstModuleProcfsOps.OnDumpOutPortAttr = _MI_AO_IMPL_OnDumpOutputPortAttr;
    pstModuleProcfsOps.OnHelp = _MI_AO_IMPL_OnHelp;
#else
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif

#endif

    hDevSysHandle = mi_sys_RegisterDev(&stModInfo, &stAoOps, &_gastAoDevInfo[AoDevId]
                                              #ifdef MI_SYS_PROC_FS_DEBUG
                                                      , &pstModuleProcfsOps
                                                      ,MI_COMMON_GetSelfDir
                                              #endif
                                          );
    _gastAoDevInfo[AoDevId].hDevSysHandle = hDevSysHandle;

    if (hDevSysHandle == NULL)
    {
        DBG_ERR("MI AO device %d fail to register dev.\n", AoDevId);
    }

#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_AO_PROCFS_DEBUG == 1)
    mi_sys_RegistCommand("setaomute",  1,  _MI_AO_ProcSetAoMute, hDevSysHandle);
    mi_sys_RegistCommand("setaovolume", 1, _MI_AO_ProcSetAoVolume, hDevSysHandle);
    mi_sys_RegistCommand("checksize", 1, _MI_AO_ProcCheckSize, hDevSysHandle);
    mi_sys_RegistCommand("dump", 2, _MI_AO_ProcDumpPcm, hDevSysHandle);
#endif

    // malloc audio HW memory
    u32BlkSize =  MI_AO_PCM_BUF_SIZE_BYTE;
    mi_sys_MMA_Alloc(NULL, u32BlkSize, (MI_PHY*)&_gastAoDevInfo[AoDevId].u64PhyBufAddr);// szMMAHeapName ???
    _gastAoDevInfo[AoDevId].pVirBufAddr = mi_sys_Vmap(_gastAoDevInfo[AoDevId].u64PhyBufAddr, u32BlkSize, FALSE);

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief destroy AO device
/// @param[in]  AoDevId: AO device ID.
/// @return MI_SUCCESS: succeed in destroying AO device .
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
//------------------------------------------------------------------------------
static MI_S32 _MI_AO_DestroyDevice(MI_AUDIO_DEV AoDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    //check input parameter
    MI_AO_CHECK_DEV(AoDevId);

    s32Ret = mi_sys_UnRegisterDev(_gastAoDevInfo[AoDevId].hDevSysHandle);
    mi_sys_UnVmap(_gastAoDevInfo[AoDevId].pVirBufAddr);
    s32Ret = mi_sys_MMA_Free(_gastAoDevInfo[AoDevId].u64PhyBufAddr);

    if(s32Ret != MI_SUCCESS)
    {
        DBG_ERR("MI AO device %d fail to unregister dev.\n", AoDevId);
    }

    return s32Ret;

}

MI_S32 MI_AO_IMPL_UpdateQueueStatus(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, _MI_AO_IMPL_QueueInfo_t *pstQueueInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;

    memcpy(&_gastAoDevInfo[AoDevId].astChanInfo[AoChn].stUsrQueueInfo, pstQueueInfo, sizeof(_MI_AO_IMPL_QueueInfo_t));

    return s32Ret;
}
//------------------------------------------------------------------------------
/// @brief MI AO device write data thread
/// @param[in]  AoDevId: AO device ID.
/// @return MI_SUCCESS: succeed in disabling AO device .
///             MI_AO_ERR_INVALID_DEVID: invalid AO device ID
///             MI_AO_ERR_NOT_PERM: not permit
//------------------------------------------------------------------------------
int _MI_AO_WriteDataThread(void* data)
{
    _MI_AO_DevInfo_t* pstAoDevInfo = (_MI_AO_DevInfo_t*)data;
    MHAL_AUDIO_DEV AoutDevId = pstAoDevInfo->AoDevId;
    MI_SYS_DRV_HANDLE hDevSysHandle = pstAoDevInfo->hDevSysHandle;
    MI_U32 u32ChnId = pstAoDevInfo->astChanInfo[0].s32ChnId;
    MI_U32 u32PortId = pstAoDevInfo->astChanInfo[0].s32InputPortId;

    MI_SYS_BufInfo_t* pstInputBufInfo = NULL;
    MI_SYS_BufConf_t  stBuf_config;
    MI_SYS_BufInfo_t* pstOutputBufInfo = NULL;

    MI_U32 u32WriteSize = 0;
    MI_S32 s32WriteActualSize = 0;

    MI_U32 u32StartWriteSize = 0;
    MI_U64 u64GetTotalSize = 0;
    MI_U32 u32AoStartThreshold;
    MI_U32 u32BitWidthByte;

    MS_BOOL bMhalRet;

    struct timespec stAoHwWriteTime;
    u64 u64AoHwStartPts = 0; // us
    MI_BOOL bInitAoPts = FALSE;
    MI_AO_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, pstAoDevInfo->stDevAttr.eBitwidth);
    u32AoStartThreshold = MI_AO_START_WRTIE_THRESHOLD_MS * ((MI_S32)pstAoDevInfo->stDevAttr.eSamplerate / 1000) * pstAoDevInfo->stDevAttr.u32ChnCnt * u32BitWidthByte;
    DBG_INFO("u32AoStartThreshold is %d \n", u32AoStartThreshold);

    ktime_get_ts(&stAoHwWriteTime);
    pstAoDevInfo->astChanInfo[u32ChnId].u64StartTime = stAoHwWriteTime.tv_sec;

    while(!MI_AO_ThreadShouldStop())
    {
        // 1.0 check if input port buffer to output AO device from MI_SYS  ???
        if(mi_sys_WaitOnInputTaskAvailable(hDevSysHandle, 3) != MI_SUCCESS)
        {
            continue;
        }

        /*
        Flag : No Need set MI_SYS_MAP_CPU_READ
        since the buf is clean & invalid.
        */
        pstInputBufInfo = mi_sys_GetInputPortBuf(hDevSysHandle, u32ChnId, u32PortId, MI_SYS_MAP_VA);

        // 1.1 if 1.0 yes, write data to AO device HW
        if(NULL != pstInputBufInfo)
        {
            void* pWrBuffer =  pstInputBufInfo->stRawData.pVirAddr;
            u32WriteSize = pstInputBufInfo->stRawData.u32BufSize;
            if(pstAoDevInfo->bMuteEnable == TRUE) // if mute enable, set zero to pWrBuffer
            {
                memset(pWrBuffer, 0, u32WriteSize);
            }
            s32WriteActualSize = MHAL_AUDIO_WriteDataOut(AoutDevId, pWrBuffer, u32WriteSize, TRUE);

            if(s32WriteActualSize >= 0)
            {

                pstAoDevInfo->astChanInfo[u32ChnId].u32WriteTotalFrmCnt += 1;

                //for start AO HW device
                u64GetTotalSize += u32WriteSize;
                if(u32StartWriteSize < u32AoStartThreshold )
                {
                    u32StartWriteSize += s32WriteActualSize;
                    if(u32StartWriteSize >= u32AoStartThreshold)
                    {
                        MHAL_AUDIO_StartPcmOut(AoutDevId);
                        ktime_get_ts(&stAoHwWriteTime);
                        //get_monotonic_boottime(&stAoHwWriteTime);
                        u64AoHwStartPts = stAoHwWriteTime.tv_sec*1000000ULL+stAoHwWriteTime.tv_nsec/1000;
                    }
                }

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_AO_PROCFS_DEBUG ==1)

                if(TRUE == _gastAoTimerInfo[AoutDevId].bTimerEnable)
                {
                    _gastAoTimerInfo[AoutDevId].u32CurrentSize = u64GetTotalSize;
                }

                if(TRUE == _gastAoDumpPcmInfo[AoutDevId].bDumpEnable)
                {
                    _MI_AO_WriteFilePcm(&_gastAoDumpPcmInfo[AoutDevId], pWrBuffer, u32WriteSize);
                }
#endif
                // 1.2 copy data to ouput port buffer ???
                if(u64AoHwStartPts == 0)
                {
                    stBuf_config.u64TargetPts = 0;  // TODO
                }
                else
                {
                    if(bInitAoPts != TRUE)
                    {
                        stBuf_config.u64TargetPts = u64AoHwStartPts;
                        bInitAoPts = TRUE;
                    }
                    else // bInitAoPts == TRUE
                    {
                        //stBuf_config.u64TargetPts = u64AoHwStartPts +  (1000000ULL/(MI_S32)pstAoDevInfo->stDevAttr.eSamplerate) * (s32WriteActualSize / (pstAoDevInfo->stDevAttr.u32ChnCnt * u32BitWidthByte));
                        //u64AoHwStartPts = stBuf_config.u64TargetPts;
                        u64AoHwStartPts += ((1000000ULL * s32WriteActualSize / (pstAoDevInfo->stDevAttr.u32ChnCnt * u32BitWidthByte)) / (MI_U64)pstAoDevInfo->stDevAttr.eSamplerate);
                        stBuf_config.u64TargetPts = u64AoHwStartPts;
                        //DBG_WRN("u64TargetPts is %llu \n", stBuf_config.u64TargetPts);
                     }
                }

                stBuf_config.eBufType = E_MI_SYS_BUFDATA_RAW;
                //stBuf_config.u64TargetPts = 100; // TODO
                stBuf_config.stRawCfg.u32Size = s32WriteActualSize;


                /*
                Flag : only MI_SYS_MAP_VA
                No need to set MI_SYS_MAP_CPU_READ & MI_SYS_MAP_CPU_WRITE flag.
                Since output is picked by cpu(user).
                */
                stBuf_config.u32Flags = MI_SYS_MAP_VA;

                DBG_INFO("Input port buffer size:%d \n", stBuf_config.stRawCfg.u32Size);
                pstOutputBufInfo  = mi_sys_GetOutputPortBuf(hDevSysHandle, 0, 0, &stBuf_config, FALSE);

#ifdef AO_DUMP_FILE
                _MI_AO_WriteFile(_gpstAoImplFile, pWrBuffer, s32WriteActualSize);
#endif

                //error handle
                if(NULL != pstOutputBufInfo)
                {
                    DBG_INFO("Get Output port buffer success;\n");
                    memcpy(pstOutputBufInfo->stRawData.pVirAddr, pWrBuffer, s32WriteActualSize);
                }
                else
                {
                    //because mi_sys_GetOutputPortBuf is change to block mode,
                    //so it will always be failed unitl someone call MI_SYS_ChnOutputPortGetBuf to get buff
                    //DBG_WRN("Get Output port buffer fail \n");
                    DBG_INFO("Get Output port buffer fail \n");

                }

                // 1.3  return inport buffer to MI_SYS
                mi_sys_FinishBuf(pstInputBufInfo);

                // 1.4 return output buffer to MI_SYS
                if(NULL != pstOutputBufInfo)
                    mi_sys_FinishBuf(pstOutputBufInfo);

            }
            else // s32WriteActualSize < 0,
            {
                DBG_ERR("s32WriteActualSize < 0, s32WriteActualSize: %d \n", s32WriteActualSize);
                bMhalRet = MHAL_AUDIO_IsPcmOutXrun( AoutDevId);
                if(bMhalRet == TRUE)
                {
                    DBG_WRN("Device %d is empty !\n", AoutDevId);
                    u32StartWriteSize = 0;  // reset u32StartWriteSize
                    u64AoHwStartPts = 0; // reset AO PTS
                    bInitAoPts = FALSE;
                }
                // return inport buffer to MI_SYS; abandon this input port buffer
                mi_sys_FinishBuf(pstInputBufInfo);
            }
        }

        pstAoDevInfo[u32ChnId].astChanInfo[u32ChnId].u64GetTotalSize = u64GetTotalSize;
        DBG_INFO("Total write size is %lld \n", u64GetTotalSize);
        DBG_INFO("Total write frame count is %d \n", pstAoDevInfo->astChanInfo[u32ChnId].u32WriteTotalFrmCnt);
    }

    return 0;
}

static MI_S32 _MI_AO_IMPL_Disable(MI_AUDIO_DEV AoDevId)
{
     MI_S32 s32Ret = MI_SUCCESS;
     MHAL_AUDIO_DEV AoutDevId = (MHAL_AUDIO_DEV) AoDevId;
     MI_S32 s32ChanlIdx;

     ///check input parameter
     MI_AO_CHECK_DEV(AoDevId);

     // check if channels of AO device are all disable ?

     for(s32ChanlIdx = 0; s32ChanlIdx<MI_AO_CHAN_NUM_MAX; s32ChanlIdx++)
     {
         if(TRUE == _gastAoDevInfo[AoDevId].astChanInfo[s32ChanlIdx].bChanEnable)
         {
            s32Ret = MI_AO_IMPL_DisableChn(AoDevId, s32ChanlIdx);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("Disable Chn failed\n");
                return MI_AO_ERR_BUSY;
            }
         }
     }

     if(_gastAoDevInfo[AoDevId].bDevEnable == FALSE)
         return MI_SUCCESS;

      _gastAoDevInfo[AoDevId].bDevEnable = FALSE;

      MI_AO_ThreadStop(_gastAoDevInfo[AoDevId].pstAoWriteDataThread);

     // 1.1 Stop PCM Out;
     s32Ret = MHAL_AUDIO_StopPcmOut(AoutDevId);
     s32Ret = MHAL_AUDIO_ClosePcmOut(AoutDevId);

     if (TRUE == _gastAoTimerInfo[AoDevId].bTimerEnable)
     {
         _gastAoTimerInfo[AoDevId].bTimerEnable = FALSE;
         _gastAoTimerInfo[AoDevId].u32CurrentSize = 0;
         _gastAoTimerInfo[AoDevId].u32OldSize = 0;

         del_timer(&_gastAoTimerInfo[AoDevId].stTimer);
     }

     DBG_INFO("MI_AO IMPL Disable success. \n");

     return s32Ret;
}


MI_S32 _MI_AO_Init()
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AoDevId;
    MI_SYSCFG_MmapInfo_t *pstMmap = NULL;
    MHAL_AUDIO_MmapCfg_t stMmapConfig;

    // enable device 0
    for(AoDevId = 0; AoDevId<MI_AO_DEV_NUM_MAX; AoDevId++)
        s32Ret = _MI_AO_CreateDevice(AoDevId);

    //get Mmap info
    if (!MI_SYSCFG_GetMmapInfo("E_MMAP_ID_MAD_R2", (const MI_SYSCFG_MmapInfo_t **)&pstMmap))
    {
        s32Ret = MHAL_AUDIO_Init(NULL); //i2 don't have E_MMAP_ID_MAD_R2
    }
    else
    {
        // for k6 and k6l, need to read E_MMAP_ID_MAD_R2
        memset(&stMmapConfig, 0, sizeof(MHAL_AUDIO_MmapCfg_t));
        stMmapConfig.u32Addr = pstMmap->u32Addr;
        stMmapConfig.u32Size = pstMmap->u32Size;
        stMmapConfig.u8MiuNo = pstMmap->u8MiuNo;

        s32Ret = MHAL_AUDIO_Init(&stMmapConfig);
    }

    //clean global struct data
    memset(_gastAoTimerInfo, 0, sizeof(_gastAoTimerInfo));
    memset(_gastAoDumpPcmInfo, 0, sizeof(_gastAoDumpPcmInfo));

    if( MI_SUCCESS == s32Ret)
        DBG_INFO("MI_AO init success. \n");
    else
        DBG_INFO("MI_AO init fail. \n");

    return s32Ret;
}

MI_S32 _MI_AO_DeInit()
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AoDevId;

    for(AoDevId = 0; AoDevId < MI_AO_DEV_NUM_MAX; AoDevId++)
    {
       if (TRUE == _gastAoDevInfo[AoDevId].bDevEnable)
       {
            s32Ret = _MI_AO_IMPL_Disable(AoDevId);
            if (MI_SUCCESS != s32Ret)
            {
                DBG_WRN("Dev%d _MI_AO_IMPL_Disable Failed\n", AoDevId);
            }
        }

        s32Ret = _MI_AO_DestroyDevice(AoDevId);
    }
    if( MI_SUCCESS == s32Ret)
        DBG_INFO("MI_AO Deinit success. \n");
    else
        DBG_INFO("MI_AO Deinit fail. \n");

    return s32Ret;
}
//=============================================================================
// Global function definition
//=============================================================================


//------------------------------------------------------------------------------
/// @brief set attribute of AO device
/// @param[in] AoDevId: AO device ID.
/// @param[in] pstAttr: Attribute of AO device.
/// @return MI_SUCCESS: succeed in setting attribute of AO device .
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_ILLEGAL_PARAM:    invalid input patamter.
///             MI_AO_ERR_NULL_PTR:         NULL point error
///             MI_AO_ERR_NOT_PERM:         not permit
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_SetPubAttr(MI_AUDIO_DEV AoDevId, MI_AUDIO_Attr_t *pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    //check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_POINTER(pstAttr);

    //check if AO device is disable ?
    if(FALSE != _gastAoDevInfo[AoDevId].bDevEnable)
        return MI_AO_ERR_NOT_PERM;

    //check if input parameter is legal ?
    MI_AO_CHECK_SAMPLERATE(pstAttr->eSamplerate);

    if( (pstAttr->u32ChnCnt > 2) || (pstAttr->eSoundmode >= E_MI_AUDIO_SOUND_MODE_MAX)
        || (pstAttr->eBitwidth >= E_MI_AUDIO_BIT_WIDTH_MAX)
        || (pstAttr->eWorkmode >= E_MI_AUDIO_MODE_MAX) )
    {
        DBG_ERR("Attribute of AO is illegal. \n");
        return MI_AO_ERR_ILLEGAL_PARAM;
    }

    if( ((pstAttr->u32ChnCnt == 2) && (pstAttr->eSoundmode == E_MI_AUDIO_SOUND_MODE_MONO))
        || ((pstAttr->u32ChnCnt == 1) && (pstAttr->eSoundmode == E_MI_AUDIO_SOUND_MODE_STEREO)) )
    {
        DBG_ERR("Error channel configure of AO . \n");
        return MI_AO_ERR_ILLEGAL_PARAM;
    }

    _gastAoDevInfo[AoDevId].AoDevId = AoDevId;
    _gastAoDevInfo[AoDevId].bDevAttrSet = TRUE;

    // save attribute of AO device
    memcpy(&_gastAoDevInfo[AoDevId].stDevAttr, pstAttr, sizeof(MI_AUDIO_Attr_t));

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief get attribute of AO device
/// @param[in]  AoDevId: AO device ID.
/// @param[out] pstAttr: Attribute of AO device.
/// @return MI_SUCCESS: succeed in setting attribute of AO device .
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_NULL_PTR:         NULL point error
///             MI_AO_ERR_NOT_CONFIG:       AO device not configure
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_GetPubAttr(MI_AUDIO_DEV AoDevId, MI_AUDIO_Attr_t*pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///check input parameter
    MI_AO_CHECK_DEV(AoDevId);     // ToDo
    MI_AO_CHECK_POINTER(pstAttr); // ToDo

    // check if AO device attr is set ?
    if(TRUE != _gastAoDevInfo[AoDevId].bDevAttrSet)
        return MI_AO_ERR_NOT_CONFIG;

    memcpy(pstAttr, &_gastAoDevInfo[AoDevId].stDevAttr, sizeof(MI_AUDIO_Attr_t));

    return s32Ret;
}


//------------------------------------------------------------------------------
/// @brief enable AO device
/// @param[in]  AoDevId: AO device ID.
/// @return MI_SUCCESS: succeed in enabling AO device .
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_NOT_CONFIG:       AO device not configure
///             MI_AO_ERR_NOT_ENABLED:      AO device not enable
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_Enable(MI_AUDIO_DEV AoDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_U32 u32BitWidthByte;
    MHAL_AUDIO_PcmCfg_t stDmaConfig;
    MHAL_AUDIO_I2sCfg_t stI2sConfig;
    MHAL_AUDIO_DEV AoutDevId;

    ///check input parameter
    MI_AO_CHECK_DEV(AoDevId);

    // check if attribute of AO device is set ?
    if(TRUE != _gastAoDevInfo[AoDevId].bDevAttrSet)
        return MI_AO_ERR_NOT_CONFIG;

    // check if AO device is enable ?
    if(TRUE == _gastAoDevInfo[AoDevId].bDevEnable)
        return MI_SUCCESS;

    _gastAoDevInfo[AoDevId].bDevEnable = TRUE;

    // 1.0 write AO device DMA HW configure
    memset(&stDmaConfig, 0, sizeof(MHAL_AUDIO_PcmCfg_t));

    AoutDevId = (MHAL_AUDIO_DEV)AoDevId;
    stDmaConfig.eWidth = (MHAL_AUDIO_BitWidth_e)_gastAoDevInfo[AoDevId].stDevAttr.eBitwidth;
    stDmaConfig.eRate = (MHAL_AUDIO_Rate_e)_gastAoDevInfo[AoDevId].stDevAttr.eSamplerate;
    stDmaConfig.u16Channels = _gastAoDevInfo[AoDevId].stDevAttr.u32ChnCnt;
    stDmaConfig.bInterleaved = TRUE;

    MI_AO_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, stDmaConfig.eWidth);

    stDmaConfig.u32PeriodSize = _gastAoDevInfo[AoDevId].stDevAttr.u32PtNumPerFrm * u32BitWidthByte * stDmaConfig.u16Channels ;
    stDmaConfig.phyDmaAddr = _gastAoDevInfo[AoDevId].u64PhyBufAddr;
    stDmaConfig.u32BufferSize = MI_AO_PCM_BUF_SIZE_BYTE ;
    stDmaConfig.u32StartThres = MI_AO_START_WRTIE_THRESHOLD_MS * stDmaConfig.u16Channels * ((MI_S32)stDmaConfig.eRate /1000) * u32BitWidthByte;
    // virual address:
    stDmaConfig.pu8DmaArea = (MS_U8 *)_gastAoDevInfo[AoDevId].pVirBufAddr;
    s32Ret = MHAL_AUDIO_ConfigPcmOut(AoutDevId, &stDmaConfig);

    // 1.1 write AO device I2S HW configure
    memset(&stI2sConfig, 0, sizeof(MHAL_AUDIO_I2sCfg_t));

    stI2sConfig.eMode = (MHAL_AUDIO_I2sMode_e) _gastAoDevInfo[AoDevId].stDevAttr.eWorkmode;
    stI2sConfig.eWidth = (MI_AUDIO_BitWidth_e)_gastAoDevInfo[AoDevId].stDevAttr.eBitwidth;
    stI2sConfig.eFmt = E_MHAL_AUDIO_I2S_FMT_LEFT_JUSTIFY; // ??? how to decide
    stI2sConfig.u16Channels = _gastAoDevInfo[AoDevId].stDevAttr.u32ChnCnt;

    s32Ret = MHAL_AUDIO_ConfigI2sOut(AoutDevId, &stI2sConfig);
    s32Ret = MHAL_AUDIO_OpenPcmOut(AoutDevId);

    // 2.0 Create Write data thread
    //_gastAoDevInfo[AoDevId].pstAoWriteDataThread = kthread_create(_MI_AO_WriteDataThread, &_gastAoDevInfo[AoDevId], "MIAoWriteDataThread");
    //wake_up_process( _gastAoDevInfo[AoDevId].pstAoWriteDataThread);
 #if !USE_CAM_OS
    _gastAoDevInfo[AoDevId].pstAoWriteDataThread = kthread_run(_MI_AO_WriteDataThread, &_gastAoDevInfo[AoDevId], "MIAoWriteDataThread");

    if (IS_ERR(_gastAoDevInfo[AoDevId].pstAoWriteDataThread))
    {
        kthread_stop(_gastAoDevInfo[AoDevId].pstAoWriteDataThread);
        s32Ret = MI_AO_ERR_BUSY;
    }

    DBG_INFO("MI_AO Cerate Thread success. \n");
#else
     s32Ret = MI_AO_CreateThread(_gastAoDevInfo[AoDevId].pstAoWriteDataThread, "MIAoWriteDataThread", _MI_AO_WriteDataThread, &_gastAoDevInfo[AoDevId]);
     if(s32Ret != MI_SUCCESS)
        return MI_AO_ERR_BUSY;

     DBG_INFO("MI_AO Cerate Thread success. \n");
#endif

    DBG_INFO("MI_AO IMPL Enable success. \n");

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief disable AO device
/// @param[in]  AoDevId: AO device ID.
/// @return MI_SUCCESS: succeed in disabling AO device .
///             MI_AO_ERR_INVALID_DEVID: invalid AO device ID
///             MI_AO_ERR_NOT_PERM: not permit
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_Disable(MI_AUDIO_DEV AoDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (TRUE == _gastAoDevInfo[AoDevId].bDevEnable)
    {
        s32Ret = _MI_AO_IMPL_Disable(AoDevId);
    }

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief enable AO device channel
/// @param[in]  AoDevId: AO device ID.
/// @param[in]  AoChn:   AO device Channel.
/// @return MI_SUCCESS: succeed in enabling AO device channel .
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_INVALID_CHNID:    invalid AO device channel
///             MI_AO_ERR_NOT_ENABLED:      AO device not enable
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_EnableChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);

    // 1. chehck if AO deivec ID is enable ?
    if(TRUE != _gastAoDevInfo[AoDevId].bDevEnable)
        return MI_AO_ERR_NOT_ENABLED;

    // 2. enable channel of AO device
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable = TRUE;
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].s32ChnId = AoChn;
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bPortEnable = TRUE;
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].s32InputPortId = 0;
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].u32WriteTotalFrmCnt = 0;

#ifdef  AO_DUMP_FILE
    _gpstAoImplFile = filp_open(MI_AO_IMPL_DUMP_FILE, O_RDWR | O_CREAT, 0644);
    if (IS_ERR(_gpstAoImplFile))
    {
        DBG_ERR( "Open Failed:%s\n", MI_AO_IMPL_DUMP_FILE);
    }

    DBG_WRN("Open file :%s  Ok", MI_AO_IMPL_DUMP_FILE);
#endif

    mi_sys_EnableChannel(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn);
    mi_sys_EnableInputPort(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn , 0);

    // 3. enable output port
    mi_sys_EnableOutputPort(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn , 0);

    DBG_INFO("MI_AO Enable Chn success. \n");

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief disabel AO device channel
/// @param[in]  AoDevId: AO device ID.
/// @param[in]  AoChn:   AO device Channel.
/// @return MI_SUCCESS: succeed in enabling AO device channel .
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_INVALID_CHNID:    invalid AO device channel
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_DisableChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///check input parameter
    MI_AO_CHECK_DEV(AoDevId);     // ToDo
    MI_AO_CHECK_CHN(AoChn);      // ToDo

    // 1. disble channel of AO device
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable = FALSE;
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].s32ChnId = AoChn;
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bPortEnable = FALSE;
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].s32InputPortId = 0;

    // disable channel & input port
    mi_sys_DisableChannel(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn);
    mi_sys_DisableInputPort(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn , 0);

    // 2. disable output channel
    mi_sys_DisableOutputPort(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn , 0);

    DBG_INFO("MI_AO Disbale Chn success. \n");

    return s32Ret;
}


MI_S32 MI_AO_IMPL_EnableReSmp(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AUDIO_SampleRate_e eInSampleRate)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);
    MI_AO_CHECK_SAMPLERATE(eInSampleRate);

    if(TRUE != _gastAoDevInfo[AoDevId].bDevEnable)
        return MI_AO_ERR_NOT_ENABLED;

    if(TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    // set resample parameter
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bResampleEnable = TRUE;
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].eInResampleRate = eInSampleRate;

    return s32Ret;
}

MI_S32 MI_AO_IMPL_DisableReSmp(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);

    // set resample parameter
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bResampleEnable = FALSE;

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief Pause an AO channel
/// @param[in]  AoiDevId: AO device ID.
/// @param[in]  AoiChn: AO device channel.
/// @return MI_SUCCESS: succeed in pausing AO channel
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_INVALID_CHNID:    invalid AO device channel
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_PauseChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MHAL_AUDIO_DEV AoutDevId = (MHAL_AUDIO_DEV) AoDevId;

    ///check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);

    if(TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    mi_sys_DisableInputPort(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn , 0);
    mi_sys_DisableOutputPort(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn , 0); // ???

    // 1. pause channel of AO device ??? disable channel ???
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanPause = TRUE;
    MHAL_AUDIO_PausePcmOut(AoutDevId);

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief Resume an AO channel
/// @param[in]  AoiDevId: AO device ID.
/// @param[in]  AoiChn: AO device channel.
/// @return MI_SUCCESS: succeed in resuming AO channel
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_INVALID_CHNID:    invalid AO device channel
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_ResumeChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MHAL_AUDIO_DEV AoutDevId = (MHAL_AUDIO_DEV) AoDevId;

    ///check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);

    if(TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable
       || TRUE !=  _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanPause)
    {
        return MI_AO_ERR_NOT_ENABLED;
    }

    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanPause = FALSE;

    mi_sys_EnableInputPort(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn , 0);
    mi_sys_EnableOutputPort(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn , 0);

    //1. resume channel of AO device
    MHAL_AUDIO_ResumePcmOut(AoutDevId);

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief Clear the current audio data buffer on an AO channel.
/// @param[in]  AoiDevId: AO device ID.
/// @param[in]  AoiChn: AO device channel.
/// @return MI_SUCCESS: succeed in resuming AO channel
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_INVALID_CHNID:    invalid AO device channel
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_ClearChnBuf(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);

    // clear channel buffer by MI_SYS API
    mi_sys_DisableChannel(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn);
    mi_sys_EnableChannel(_gastAoDevInfo[AoDevId].hDevSysHandle, AoChn);

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief Queries the status of the current audio data buffer on the AO channel.
/// @param[in]  AoiDevId: AO device ID.
/// @param[in]  AoiChn: AO device channel.
/// @return MI_SUCCESS: succeed in resuming AO channel
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_INVALID_CHNID:    invalid AO device channel
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_QueryChnStat(MI_AUDIO_DEV AoDevId , MI_AO_CHN AoChn, MI_AO_ChnState_t *pstStatus)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);     // ToDo
    MI_AO_CHECK_CHN(AoChn);       // ToDo

    if(TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    //Query channel state by MI_SYS API
    // Need to MI_SYS help

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief Set volume of AO device
/// @param[in] AoDevId: AO device ID.
/// @param[in] s32VolumeDb: Volume (in dB)
/// @return MI_SUCCESS: succeed in setting  volume of AO device channel.
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_ILLEGAL_PARAM:    invalid input patamter.
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_SetVolume(MI_AUDIO_DEV AoDevId, MI_S32 s32VolumeDb)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MHAL_AUDIO_DEV AoutDevId = (MHAL_AUDIO_DEV) AoDevId;
    MS_S16 s16Gain = (MS_S16) s32VolumeDb;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);

    // check s32VolumeDb range ???

    //1. chehck if AO deivec ID is enable ?
    if(TRUE != _gastAoDevInfo[AoDevId].bDevEnable)
        return MI_AO_ERR_NOT_ENABLED;

    // 2. save s32VolumeDb
    _gastAoDevInfo[AoDevId].s32VolumeDb = s32VolumeDb;

    // 3. call MHal audio API
    s32Ret = MHAL_AUDIO_SetGainOut(AoutDevId, s16Gain);

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief Get volume of AO device
/// @param[in] AoDevId: AO device ID.
/// @param[in] ps32VolumeDb: Ptr to Volume
/// @return MI_SUCCESS: succeed in getting  volume of AO device channel.
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_NULL_PTR:         NULL point error
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_GetVolume(MI_AUDIO_DEV AoDevId, MI_S32 *ps32VolumeDb)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_POINTER(ps32VolumeDb);

    // chehck if AO deivec ID is enable ?
    if(TRUE != _gastAoDevInfo[AoDevId].bDevEnable)
        return MI_AO_ERR_NOT_ENABLED;

    // get volume from _gastAoDevInfo
    *ps32VolumeDb = _gastAoDevInfo[AoDevId].s32VolumeDb;

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief Set the mute of an AO device
/// @param[in] AoDevId: AO device ID.
/// @param[in] bEnable: AO device mute enable
/// @return MI_SUCCESS: succeed in muting AO device.
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_NOT_ENABLED:      AO device not enable
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_SetMute(MI_AUDIO_DEV AoDevId, MI_BOOL bEnable)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);

    // chehck if AO deivec ID is enable ?
    if(TRUE != _gastAoDevInfo[AoDevId].bDevEnable)
        return MI_AO_ERR_NOT_ENABLED;

    // save mute enable
    _gastAoDevInfo[AoDevId].bMuteEnable = bEnable;

    // mute time ? hardware buffer

    // stop hardware

    // disable port ?

    // _gastAoDevInfo[AoDevId].s32VolumeDb = 0xffff => mute gain

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief Get the mute status of an AO device
/// @param[in] AoDevId: AO device ID.
/// @param[in] pbEnable: Ptr to  mute status of AO device
/// @return MI_SUCCESS: succeed in getting  mute status of AO device.
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_NULL_PTR:         NULL point error
///             MI_AO_ERR_NOT_ENABLED:      AO device not enable
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_GetMute(MI_AUDIO_DEV AoDevId, MI_BOOL *pbEnable)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_POINTER(pbEnable);

    // chehck if AO deivec ID is enable ?
    if(TRUE != _gastAoDevInfo[AoDevId].bDevEnable)
        return MI_AO_ERR_NOT_ENABLED;

    *pbEnable = _gastAoDevInfo[AoDevId].bMuteEnable;

    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief Clear attribute of AO device
/// @param[in] AoDevId: AO device ID.
/// @return MI_SUCCESS: succeed in clearing attribute of AO device .
///             MI_AO_ERR_INVALID_DEVID:    invalid AO device ID
///             MI_AO_ERR_NOT_PERM:         not permit
//------------------------------------------------------------------------------
MI_S32 MI_AO_IMPL_ClrPubAttr(MI_AUDIO_DEV AoDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);

    // chehck if AO deivec ID is disable ?
    if(FALSE != _gastAoDevInfo[AoDevId].bDevEnable)
        return MI_AO_ERR_BUSY;

    _gastAoDevInfo[AoDevId].bDevAttrSet = FALSE;
    memset(&_gastAoDevInfo[AoDevId].stDevAttr, 0, sizeof(MI_AUDIO_Attr_t));

    return s32Ret;
}

MI_S32 MI_AO_IMPL_SetVqeAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_VqeConfig_t *pstVqeConfig)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);
    MI_AO_CHECK_POINTER(pstVqeConfig);

    // check if Vqe of AO device channel is disable ?
    if( FALSE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bVqeEnable)
        return MI_AO_ERR_NOT_PERM;

    // check if AO device channel is enable ?
    if( TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    // save Vqe configure of AO device channel
    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bVqeAttrSet = TRUE;
    memcpy(&_gastAoDevInfo[AoDevId].astChanInfo[AoChn].stAoVqeConfig, pstVqeConfig, sizeof(MI_AO_VqeConfig_t));

    return s32Ret;
}


MI_S32 MI_AO_IMPL_EnableVqe(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    // check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);

    //
    if( TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bVqeAttrSet)
        return MI_AO_ERR_NOT_PERM;

    if( TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    if(TRUE == _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bVqeEnable)
        return s32Ret;

    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bVqeEnable = TRUE;

    return s32Ret;
}

MI_S32 MI_AO_IMPL_DisableVqe(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

     // check input parameter
    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);

    if(FALSE == _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bVqeEnable)
        return s32Ret;

    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bVqeEnable = FALSE;

    return s32Ret;
}

MI_S32 MI_AO_IMPL_SetAdecAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_AdecConfig_t *pstAdecConfig)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);
    MI_AO_CHECK_POINTER(pstAdecConfig);

        // check if Adec of AO device channel is disable ?
    if( FALSE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bAdecEnable)
        return MI_AO_ERR_NOT_PERM;

    // check if AO device channel is enable ?
    if( TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable)
        return MI_AO_ERR_NOT_ENABLED;

    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bAdecAttrSet = TRUE;
    memcpy(&_gastAoDevInfo[AoDevId].astChanInfo[AoChn].stAoAdecConfig, pstAdecConfig, sizeof(MI_AO_AdecConfig_t));

    return s32Ret;
}

MI_S32 MI_AO_IMPL_EnableAdec(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);

    if (TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bAdecAttrSet)
    {
        return MI_AO_ERR_NOT_PERM;
    }

    if (TRUE != _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bChanEnable)
    {
        return MI_AO_ERR_NOT_ENABLED;
    }

    if (TRUE == _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bAdecEnable)
    {
        return s32Ret;
    }

    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bAdecEnable = TRUE;

    return s32Ret;
}

MI_S32 MI_AO_IMPL_DisableAdec(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_AO_CHECK_DEV(AoDevId);
    MI_AO_CHECK_CHN(AoChn);

    if (FALSE == _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bAdecEnable)
    {
        return s32Ret;
    }

    _gastAoDevInfo[AoDevId].astChanInfo[AoChn].bAdecEnable = FALSE;

    return s32Ret;
}
