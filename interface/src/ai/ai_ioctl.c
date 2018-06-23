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
/// @file   ai_ioctl.c
/// @brief vdec module ioctl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_ai_impl.h"
#include "mi_aio_datatype.h"
#include "ai_ioctl.h"


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

static MI_S32 MI_AI_IOCTL_SetPubAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_SetPubAttr_t *pstSetPubAttr = (MI_AI_SetPubAttr_t *)ptr;

    MI_AUDIO_DEV AiDevId = pstSetPubAttr->AiDevId;
    MI_AUDIO_Attr_t* pstAttr = &(pstSetPubAttr->stAttr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_SetPubAttr(AiDevId, pstAttr);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_GetPubAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_GetPubAttr_t *pstGetPubAttr = (MI_AI_GetPubAttr_t *)ptr;

    MI_AUDIO_DEV AiDevId = pstGetPubAttr->AiDevId;
    MI_AUDIO_Attr_t *pstAttr = &(pstGetPubAttr->stAttr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_GetPubAttr(AiDevId, pstAttr);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_Enable(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AiDevId = *((MI_AUDIO_DEV *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_Enable(AiDevId);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_Disable(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AiDevId = *((MI_AUDIO_DEV *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_Disable(AiDevId);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_EnableChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_EnableChn_t *pstEnableChn = (MI_AI_EnableChn_t *)ptr;

    MI_AUDIO_DEV AiDevId = pstEnableChn->AiDevId;
    MI_AI_CHN AiChn = pstEnableChn->AiChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_EnableChn(AiDevId, AiChn);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_DisableChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_DisableChn_t *pstDisableChn = (MI_AI_DisableChn_t *)ptr;

    MI_AUDIO_DEV AiDevId = pstDisableChn->AiDevId;
    MI_AI_CHN AiChn = pstDisableChn->AiChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_DisableChn(AiDevId, AiChn);

    return s32Ret;
}


static MI_S32 MI_AI_IOCTL_SetChnParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_SetChnParam_t *pstSetChnParam = (MI_AI_SetChnParam_t *)ptr;

    MI_AUDIO_DEV AiDevId = pstSetChnParam->AiDevId;
    MI_AI_CHN AiChn = pstSetChnParam->AiChn;
    MI_AI_ChnParam_t *pstChnParam = &pstSetChnParam->stChnParam;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_SetChnParam(AiDevId, AiChn, pstChnParam);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_GetChnParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_GetChnParam_t *pstGetChnParam = (MI_AI_GetChnParam_t *)ptr;

    MI_AUDIO_DEV AiDevId = pstGetChnParam->AiDevId;
    MI_AI_CHN AiChn = pstGetChnParam->AiChn;
    MI_AI_ChnParam_t *pstChnParam = &pstGetChnParam->stChnParam;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_GetChnParam(AiDevId, AiChn, pstChnParam);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_EnableReSmp(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_EnableReSmp_t* pstEnableReSmp = (MI_AI_EnableReSmp_t*)ptr;

    MI_AUDIO_DEV AiDevId = pstEnableReSmp->AiDevId;
    MI_AI_CHN AiChn = pstEnableReSmp->AiChn;
    MI_AUDIO_SampleRate_e eOutSampleRate = pstEnableReSmp->eOutSampleRate;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_EnableReSmp(AiDevId,  AiChn, eOutSampleRate);
    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_DisableReSmp(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_DisableReSmp_t* pstDisableReSmp = (MI_AI_DisableReSmp_t*)ptr;

    MI_AUDIO_DEV AiDevId = pstDisableReSmp->AiDevId;
    MI_AI_CHN AiChn = pstDisableReSmp->AiChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_DisableReSmp(AiDevId,  AiChn);
    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_SetVqeAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_SetVqeAttr_t* pstSetVqeAttr = (MI_AI_SetVqeAttr_t*)ptr;

    MI_AUDIO_DEV AiDevId = pstSetVqeAttr->AiDevId;
    MI_AI_CHN AiChn = pstSetVqeAttr->AiChn;
    MI_AUDIO_DEV AoDevId = pstSetVqeAttr->AoDevId;
    MI_AO_CHN AoChn = pstSetVqeAttr->AoChn;
    MI_AI_VqeConfig_t* pstVqeConfig = &(pstSetVqeAttr->stVqeConfig);
    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_SetVqeAttr(AiDevId, AiChn, AoDevId, AoChn, pstVqeConfig);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_EnableVqe(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_EnableVqe_t* pstEnableVqe = (MI_AI_EnableVqe_t*)ptr;

    MI_AUDIO_DEV AiDevId = pstEnableVqe->AiDevId;
    MI_AI_CHN AiChn = pstEnableVqe->AiChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_EnableVqe(AiDevId, AiChn);
    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_DisableVqe(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_DisableVqe_t* pstDisableVqe = (MI_AI_DisableVqe_t*)ptr;

    MI_AUDIO_DEV AiDevId = pstDisableVqe->AiDevId;
    MI_AI_CHN AiChn = pstDisableVqe->AiChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_DisableVqe(AiDevId, AiChn);
    return s32Ret;
}


static MI_S32 MI_AI_IOCTL_ClrPubAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV AiDevId = *((MI_AUDIO_DEV *)ptr);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_ClrPubAttr(AiDevId);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_SetAencAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_SetAencAttr_t *pstSetAencAttr = (MI_AI_SetAencAttr_t *)ptr;

    MI_AUDIO_DEV AiDevId = pstSetAencAttr->AiDevId;
    MI_AI_CHN AiChn = pstSetAencAttr->AiChn;
    MI_AI_AencConfig_t *pstAencConfig = &pstSetAencAttr->stAencConfig;

    AUTO_LOCK(env, mutex);

    s32Ret = MI_AI_IMPL_SetAencAttr(AiDevId, AiChn, pstAencConfig);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_EnableAenc(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_EnableAenc_t* pstEnableAnec = (MI_AI_EnableAenc_t*)ptr;

    MI_AUDIO_DEV AiDevId = pstEnableAnec->AiDevId;
    MI_AI_CHN AiChn = pstEnableAnec->AiChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_EnableAenc(AiDevId, AiChn);

    return s32Ret;
}

static MI_S32 MI_AI_IOCTL_DisableAenc(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AI_DisableAenc_t *pstDisableAenc = (MI_AI_DisableAenc_t*)ptr;

    MI_AUDIO_DEV AiDevId = pstDisableAenc->AiDevId;
    MI_AI_CHN AiChn = pstDisableAenc->AiChn;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_AI_IMPL_DisableAenc(AiDevId, AiChn);

    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_AI_CMD_MAX] = {
    [E_MI_AI_CMD_SET_PUB_ATTR] = MI_AI_IOCTL_SetPubAttr,
    [E_MI_AI_CMD_GET_PUB_ATTR] = MI_AI_IOCTL_GetPubAttr,
    [E_MI_AI_CMD_ENABLE] = MI_AI_IOCTL_Enable,
    [E_MI_AI_CMD_DISABLE] = MI_AI_IOCTL_Disable,
    [E_MI_AI_CMD_ENABLE_CHN] = MI_AI_IOCTL_EnableChn,
    [E_MI_AI_CMD_DISABLE_CHN] = MI_AI_IOCTL_DisableChn,
    [E_MI_AI_CMD_SET_CHN_PARAM] = MI_AI_IOCTL_SetChnParam,
    [E_MI_AI_CMD_GET_CHN_PARAM] = MI_AI_IOCTL_GetChnParam,
    [E_MI_AI_CMD_ENABLE_RESMP] = MI_AI_IOCTL_EnableReSmp,
    [E_MI_AI_CMD_DISABLE_RESMP] = MI_AI_IOCTL_DisableReSmp,
    [E_MI_AI_CMD_SET_VQE_ATTR] = MI_AI_IOCTL_SetVqeAttr,
    [E_MI_AI_CMD_ENABLE_VQE] = MI_AI_IOCTL_EnableVqe,
    [E_MI_AI_CMD_DISABLE_VQE] = MI_AI_IOCTL_DisableVqe,
    [E_MI_AI_CMD_CLR_PUB_ATTR] = MI_AI_IOCTL_ClrPubAttr,
    [E_MI_AI_CMD_SET_AENC_ATTR] = MI_AI_IOCTL_SetAencAttr,
    [E_MI_AI_CMD_ENABLE_AENC] = MI_AI_IOCTL_EnableAenc,
    [E_MI_AI_CMD_DISABLE_AENC] = MI_AI_IOCTL_DisableAenc,

};


static void mi_ai_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
}
static void mi_ai_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
    _MI_AI_Init();
}
static void mi_ai_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
    _MI_AI_DeInit();
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");

MI_DEVICE_DEFINE(mi_ai_insmod,
               ioctl_table, E_MI_AI_CMD_MAX,
               mi_ai_process_init, mi_ai_process_exit);