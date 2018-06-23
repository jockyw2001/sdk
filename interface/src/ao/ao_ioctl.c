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

#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_ao_impl.h"
#include "mi_aio_datatype.h"
//#include "mi_ao_datatype_internal.h"
#include "ao_ioctl.h"


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
static DEFINE_MUTEX(mutex);


//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
static MI_S32 MI_AO_IOCTL_SetPubAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetPubAttr_t* pstSetPubAttr = (MI_AO_SetPubAttr_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstSetPubAttr->AoDevId;
    MI_AUDIO_Attr_t* pstAttr = &(pstSetPubAttr->stPubAttr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_SetPubAttr(AoDevId, pstAttr);
    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_GetPubAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_GetPubAttr_t* pstGetPubattr = (MI_AO_GetPubAttr_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstGetPubattr->AoDevId;
    MI_AUDIO_Attr_t stAttr;

    memset(&stAttr, 0, sizeof(stAttr));

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_GetPubAttr(AoDevId, &stAttr);
    memcpy(&pstGetPubattr->stPubAttr, &stAttr, sizeof(stAttr));

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_Enable(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AoDevId = *((MI_AUDIO_DEV *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_Enable(AoDevId);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_Disable(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AoDevId = *((MI_AUDIO_DEV *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_Disable(AoDevId);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_EnableChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_EnableChn_t* pstEnableChn = (MI_AO_EnableChn_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstEnableChn->AoDevId;
    MI_AO_CHN AoChn = pstEnableChn->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_EnableChn(AoDevId, AoChn);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_DisableChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_DisableChn_t* pstDisableChn = (MI_AO_DisableChn_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstDisableChn->AoDevId;
    MI_AO_CHN AoChn = pstDisableChn->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_DisableChn(AoDevId, AoChn);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_EnableReSmp(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_EnableReSmp_t* pstEnableReSmp = (MI_AO_EnableReSmp_t*)ptr;

    MI_AUDIO_DEV AoDevId = pstEnableReSmp->AoDevId;
    MI_AO_CHN AoChn = pstEnableReSmp->AoChn;
    MI_AUDIO_SampleRate_e eInSampleRate = pstEnableReSmp->eInSampleRate;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_EnableReSmp(AoDevId,  AoChn, eInSampleRate);
    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_DisableReSmp(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_DisableReSmp_t* pstDisableReSmp = (MI_AO_DisableReSmp_t*)ptr;

    MI_AUDIO_DEV AoDevId = pstDisableReSmp->AoDevId;
    MI_AO_CHN AoChn = pstDisableReSmp->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_DisableReSmp(AoDevId,  AoChn);
    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_PauseChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_PauseChn_t* pstPauseChn = (MI_AO_PauseChn_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstPauseChn->AoDevId;
    MI_AO_CHN AoChn = pstPauseChn->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_PauseChn(AoDevId, AoChn);
    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_ResumeChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_ResumeChn_t* pstResumeChn = (MI_AO_ResumeChn_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstResumeChn->AoDevId;
    MI_AO_CHN AoChn = pstResumeChn->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_ResumeChn(AoDevId, AoChn);
    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_ClearChnBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_ClearChnBuf_t* pstClearChnBuf = (MI_AO_ClearChnBuf_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstClearChnBuf->AoDevId;
    MI_AO_CHN AoChn = pstClearChnBuf->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_ClearChnBuf(AoDevId, AoChn);
    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_QueryChnStat(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_QueryChnStat_t* pstQueryChnStat = (MI_AO_QueryChnStat_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstQueryChnStat->AoDevId;
    MI_AO_CHN AoChn = pstQueryChnStat->AoChn;
    MI_AO_ChnState_t stStatus;
    memset(&stStatus, 0, sizeof(stStatus));

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_QueryChnStat(AoDevId, AoChn, &stStatus);
    memcpy(&pstQueryChnStat->stStatus ,&stStatus, sizeof(stStatus));

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_SetVolume(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetVolume_t* pstSetVolume = (MI_AO_SetVolume_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstSetVolume->AoDevId;
    MI_S32 s32VolumeDb = pstSetVolume->s32VolumeDb;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_SetVolume(AoDevId, s32VolumeDb);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_GetVolume(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_GetVolume_t* pstGetVolume = (MI_AO_GetVolume_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstGetVolume->AoDevId;
    MI_S32 s32VolumeDb;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_GetVolume(AoDevId, &s32VolumeDb);
    pstGetVolume->s32VolumeDb = s32VolumeDb;

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_SetMute(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetMute_t* pstSetMute = (MI_AO_SetMute_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstSetMute->AoDevId;
    MI_BOOL bEnable = pstSetMute->bEnable;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_SetMute(AoDevId, bEnable);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_GetMute(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_GetMute_t* pstGetMute = (MI_AO_GetMute_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstGetMute->AoDevId;
    MI_BOOL bEnable;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_GetMute(AoDevId, &bEnable);
    pstGetMute->bEnable = bEnable;

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_ClrPubAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AoDevId = *((MI_AUDIO_DEV *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_ClrPubAttr(AoDevId);
    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_SetVqeAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetVqeAttr_t* pstSetVqeAttr = (MI_AO_SetVqeAttr_t*)ptr;

    MI_AUDIO_DEV AoDevId = pstSetVqeAttr->AoDevId;
    MI_AO_CHN AoChn = pstSetVqeAttr->AoChn;
    MI_AO_VqeConfig_t* pstVqeConfig = &(pstSetVqeAttr->stVqeConfig);
    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_SetVqeAttr(AoDevId, AoChn, pstVqeConfig);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_EnableVqe(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_EnableVqe_t* pstEnableVqe = (MI_AO_EnableVqe_t*)ptr;

    MI_AUDIO_DEV AoDevId = pstEnableVqe->AoDevId;
    MI_AO_CHN AoChn = pstEnableVqe->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_EnableVqe(AoDevId, AoChn);
    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_DisableVqe(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_DisableVqe_t* pstDisableVqe = (MI_AO_DisableVqe_t*)ptr;

    MI_AUDIO_DEV AoDevId = pstDisableVqe->AoDevId;
    MI_AO_CHN AoChn = pstDisableVqe->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_DisableVqe(AoDevId, AoChn);
    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_SetAdecAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_SetAdecAttr_t *pstSetAdecAttr = (MI_AO_SetAdecAttr_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstSetAdecAttr->AoDevId;
    MI_AO_CHN AoChn = pstSetAdecAttr->AoChn;
    MI_AO_AdecConfig_t *pstAdecConfig = &(pstSetAdecAttr->stAdecConfig);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_SetAdecAttr(AoDevId, AoChn, pstAdecConfig);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_EnableAdec(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_EnableAdec_t *pstEnableAdec = (MI_AO_EnableAdec_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstEnableAdec->AoDevId;
    MI_AO_CHN AoChn = pstEnableAdec->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_EnableAdec(AoDevId, AoChn);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_DisableAdec(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_DisableAdec_t *pstDisableAdec = (MI_AO_DisableAdec_t *)ptr;

    MI_AUDIO_DEV AoDevId = pstDisableAdec->AoDevId;
    MI_AO_CHN AoChn = pstDisableAdec->AoChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_DisableAdec(AoDevId, AoChn);

    return s32Ret;
}

static MI_S32 MI_AO_IOCTL_UpdateQueueStatus(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AO_QueueStatus_t *pstQueueStatus = (MI_AO_QueueStatus_t *)ptr;
    _MI_AO_IMPL_QueueInfo_t stImplQueueStatus;

    MI_AUDIO_DEV AoDevId = pstQueueStatus->AoDevId;
    MI_AO_CHN AoChn = pstQueueStatus->AoChn;
    stImplQueueStatus.u64GetTotalSize = pstQueueStatus->u64GetTotalSize;
    stImplQueueStatus.s32Max = pstQueueStatus->s32Max;
    stImplQueueStatus.s32Size = pstQueueStatus->s32Size;
    stImplQueueStatus.u64RunTime = pstQueueStatus->u64RunTime;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AO_IMPL_UpdateQueueStatus(AoDevId, AoChn, &stImplQueueStatus);

    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_AO_CMD_MAX] = {
    [E_MI_AO_CMD_SET_PUB_ATTR] = MI_AO_IOCTL_SetPubAttr,
    [E_MI_AO_CMD_GET_PUB_ATTR] = MI_AO_IOCTL_GetPubAttr,
    [E_MI_AO_CMD_ENABLE] = MI_AO_IOCTL_Enable,
    [E_MI_AO_CMD_DISABLE] = MI_AO_IOCTL_Disable,
    [E_MI_AO_CMD_ENABLE_CHN] = MI_AO_IOCTL_EnableChn,
    [E_MI_AO_CMD_DISABLE_CHN] = MI_AO_IOCTL_DisableChn,
    [E_MI_AO_CMD_ENABLE_RESMP] = MI_AO_IOCTL_EnableReSmp,
    [E_MI_AO_CMD_DISABLE_RESMP] = MI_AO_IOCTL_DisableReSmp,
    [E_MI_AO_CMD_PAUSE_CHN] = MI_AO_IOCTL_PauseChn,
    [E_MI_AO_CMD_RESUME_CHN] = MI_AO_IOCTL_ResumeChn,
    [E_MI_AO_CMD_CLEAR_CHN_BUF] = MI_AO_IOCTL_ClearChnBuf,
    [E_MI_AO_CMD_QUERY_CHN_STAT] = MI_AO_IOCTL_QueryChnStat,
    [E_MI_AO_CMD_SET_VOLUME] = MI_AO_IOCTL_SetVolume,
    [E_MI_AO_CMD_GET_VOLUME] = MI_AO_IOCTL_GetVolume,
    [E_MI_AO_CMD_SET_MUTE] = MI_AO_IOCTL_SetMute,
    [E_MI_AO_CMD_GET_MUTE] = MI_AO_IOCTL_GetMute,
    [E_MI_AO_CMD_CLR_PUB_ATTR] = MI_AO_IOCTL_ClrPubAttr,
    [E_MI_AO_CMD_SET_VQE_ATTR] = MI_AO_IOCTL_SetVqeAttr,
    [E_MI_AO_CMD_ENABLE_VQE] = MI_AO_IOCTL_EnableVqe,
    [E_MI_AO_CMD_DISABLE_VQE] = MI_AO_IOCTL_DisableVqe,
    [E_MI_AO_CMD_SET_ADEC_ATTR] = MI_AO_IOCTL_SetAdecAttr,
    [E_MI_AO_CMD_ENABLE_ADEC] = MI_AO_IOCTL_EnableAdec,
    [E_MI_AO_CMD_DISABLE_ADEC] = MI_AO_IOCTL_DisableAdec,
    [E_MI_AO_CMD_UPDATE_QUEUE_STATUS] = MI_AO_IOCTL_UpdateQueueStatus,
};

static void mi_ao_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
}

static void mi_ao_process_init(MI_COMMON_Client_t *client)
{
    MI_PRINT("client [%d] connected\n", client->pid);
    _MI_AO_Init();
}

static void mi_ao_process_exit(MI_COMMON_Client_t *client)
{
    MI_PRINT("client [%d] disconnected\n", client->pid);
    _MI_AO_DeInit();
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");

MI_DEVICE_DEFINE(mi_ao_insmod,
               ioctl_table, E_MI_AO_CMD_MAX,
               mi_ao_process_init, mi_ao_process_exit);
