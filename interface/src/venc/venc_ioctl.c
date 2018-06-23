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
/// @file   vdec_ioctl.c
/// @brief vdec module ioctl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_venc_impl.h"
#include "venc_ioctl.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
static DEFINE_MUTEX(mutex);

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------
#define PRINT_CHN(VeChn, fmt, ...) DBG_INFO("Ch %d." fmt "\n", VeChn, __VA_ARGS__)
#define PRINTLN_CHN(VeChn) DBG_INFO("Ch %d.\n", VeChn)

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

static MI_S32 MI_VENC_IOCTL_SetModParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_ModParam_t *pstModParam = (MI_VENC_ModParam_t *)ptr;
    //MI_VENC_ModType_e eType = pstModParam->eVencModType;

    AUTO_LOCK(env, mutex);
    //DBG_INFO("<%s> layer: %d.\n", __func__, eType);
    s32Ret = MI_VENC_IMPL_SetModParam(pstModParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetModParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_ModParam_t *pstModParam = (MI_VENC_ModParam_t *)ptr;
    //MI_VENC_ModType_e eType = pstModParam->eVencModType;

    AUTO_LOCK(env, mutex);
    //DBG_INFO("<%s> layer: %d.\n", __func__, eType);
    s32Ret = MI_VENC_IMPL_GetModParam(pstModParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_CreateChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_CreateChn_t *pstCreateChn = (MI_VENC_CreateChn_t *)ptr;
    //mi_sys_ModuleDevInfo_t *pstModDev = (mi_sys_ModuleDevInfo_t *)ptr;
    MI_VENC_CHN VeChn;
    MI_VENC_ChnAttr_t *pstAttr;
    VeChn = pstCreateChn->VeChn;
    pstAttr = &pstCreateChn->stAttr;


    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);

    s32Ret = MI_VENC_IMPL_CreateChn(VeChn, pstAttr);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_DestroyChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_CHN VeChn = *(MI_VENC_CHN*)(ptr);

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_DestroyChn(VeChn);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_ResetChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_CHN VeChn = *(MI_VENC_CHN*)(ptr);

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_ResetChn(VeChn);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_StartRecvPic(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_CHN VeChn = *(MI_VENC_CHN*)(ptr);

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_StartRecvPic(VeChn);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_StartRecvPicEx(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_StartRecvPicEx_t *pstStartRecvPicEx = (MI_VENC_StartRecvPicEx_t *)ptr;
    MI_VENC_CHN VeChn = pstStartRecvPicEx->VeChn;
    MI_VENC_RecvPicParam_t *pstRecvParam = &pstStartRecvPicEx->stRecvParam;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_StartRecvPicEx(VeChn, pstRecvParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_StopRecvPic(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_CHN VeChn = *(MI_VENC_CHN*)(ptr);

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_StopRecvPic(VeChn);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_Query(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_Query_t *pstQuery = (MI_VENC_Query_t *)ptr;
    MI_VENC_CHN VeChn = pstQuery->VeChn;
    MI_VENC_ChnStat_t *pstStat = &pstQuery->stStat;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_Query(VeChn, pstStat);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetChnAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetChnAttr_t *pstSetChnAttr = (MI_VENC_SetChnAttr_t *)ptr;
    MI_VENC_CHN VeChn = pstSetChnAttr->VeChn;
    MI_VENC_ChnAttr_t* pstAttr = &pstSetChnAttr->stAttr;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetChnAttr(VeChn, pstAttr);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetChnAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetChnAttr_t *pstGetChnAttr = (MI_VENC_GetChnAttr_t *)ptr;
    MI_VENC_CHN VeChn = pstGetChnAttr->VeChn;
    MI_VENC_ChnAttr_t*pstAttr = &pstGetChnAttr->stAttr;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetChnAttr(VeChn, pstAttr);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetStream(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetStream_t *pstGetStream = (MI_VENC_GetStream_t *)ptr;
    MI_VENC_CHN VeChn = pstGetStream->VeChn;
    MI_VENC_Stream_t *pstStream = &pstGetStream->stStream;
    MI_S32 s32MilliSec = pstGetStream->s32MilliSec;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetStream(VeChn, pstStream, s32MilliSec);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_ReleaseStream(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_ReleaseStream_t *pstReleaseStream = (MI_VENC_ReleaseStream_t *)ptr;
    MI_VENC_CHN VeChn = pstReleaseStream->VeChn;
    MI_VENC_Stream_t *pstStream = &pstReleaseStream->stStream;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_ReleaseStream(VeChn, pstStream);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_InsertUserData(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_InsertUserData_t *pstInsertUserData = (MI_VENC_InsertUserData_t *)ptr;
    MI_VENC_CHN VeChn = pstInsertUserData->VeChn;
    MI_U8 *pu8Data = pstInsertUserData->u8Data;
    MI_U32 u32Len = pstInsertUserData->u32Len;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_InsertUserData(VeChn, pu8Data, u32Len);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetMaxStreamCnt(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetMaxStreamCnt_t *pstSetMaxStreamCnt = (MI_VENC_SetMaxStreamCnt_t *)ptr;
    MI_VENC_CHN VeChn = pstSetMaxStreamCnt->VeChn;
    MI_U32 u32MaxStrmCnt = pstSetMaxStreamCnt->u32MaxStrmCnt;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetMaxStreamCnt(VeChn,u32MaxStrmCnt);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetMaxStreamCnt(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetMaxStreamCnt_t *pstGetMaxStreamCnt = (MI_VENC_GetMaxStreamCnt_t *)ptr;
    MI_VENC_CHN VeChn = pstGetMaxStreamCnt->VeChn;
    MI_U32 *pu32MaxStrmCnt = &pstGetMaxStreamCnt->u32MaxStrmCnt;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetMaxStreamCnt(VeChn, pu32MaxStrmCnt);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_RequestIdr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_RequestIdr_t *pstRequestIdr = (MI_VENC_RequestIdr_t *)ptr;
    MI_VENC_CHN VeChn = pstRequestIdr->VeChn;
    MI_BOOL bInstant = pstRequestIdr->bInstant;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_RequestIdr(VeChn, bInstant);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_EnableIdr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_EnableIdr_t *pstEnableIdr = (MI_VENC_EnableIdr_t *)ptr;
    MI_VENC_CHN VeChn = pstEnableIdr->VeChn;
    MI_BOOL bEnableIdr = pstEnableIdr->bEnableIdr;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_EnableIdr(VeChn, bEnableIdr);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH264IdrPicId(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264IdrPicId_t *pstSetH264IdrPicId = (MI_VENC_SetH264IdrPicId_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH264IdrPicId->VeChn;
    MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg = &pstSetH264IdrPicId->stH264eIdrPicIdCfg;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH264IdrPicId(VeChn, pstH264eIdrPicIdCfg);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH264IdrPicId(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264IdrPicId_t *pstGetH264IdrPicId = (MI_VENC_GetH264IdrPicId_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH264IdrPicId->VeChn;
    MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg = &pstGetH264IdrPicId->stH264eIdrPicIdCfg;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH264IdrPicId(VeChn, pstH264eIdrPicIdCfg);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetFd(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetFd_t *pstGetFd = (MI_VENC_GetFd_t *)ptr;
    MI_VENC_CHN VeChn = pstGetFd->VeChn;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetFd(VeChn);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_CloseFd(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_CloseFd_t *pstCloseFd = (MI_VENC_CloseFd_t *)ptr;
    MI_VENC_CHN VeChn = pstCloseFd->VeChn;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_CloseFd(VeChn);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetRoiCfg(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRoiCfg_t *pstSetRoiCfg = (MI_VENC_SetRoiCfg_t *)ptr;
    MI_VENC_CHN VeChn = pstSetRoiCfg->VeChn;
    MI_VENC_RoiCfg_t *pstVencRoiCfg = &pstSetRoiCfg->stVencRoiCfg;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetRoiCfg(VeChn, pstVencRoiCfg);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetRoiCfg(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRoiCfg_t *pstGetRoiCfg = (MI_VENC_GetRoiCfg_t *)ptr;
    MI_VENC_CHN VeChn = pstGetRoiCfg->VeChn;
    MI_U32 u32Index = pstGetRoiCfg->u32Index;
    MI_VENC_RoiCfg_t *pstVencRoiCfg = &pstGetRoiCfg->stVencRoiCfg;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetRoiCfg(VeChn, u32Index, pstVencRoiCfg);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetRoiBgFrameRate(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRoiBgFrameRate_t *pstSetRoiBgFrameRate = (MI_VENC_SetRoiBgFrameRate_t *)ptr;
    MI_VENC_CHN VeChn = pstSetRoiBgFrameRate->VeChn;
    MI_VENC_RoiBgFrameRate_t * pstRoiBgFrmRate = &pstSetRoiBgFrameRate->pstRoiBgFrmRate;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetRoiBgFrameRate(VeChn, pstRoiBgFrmRate);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetRoiBgFrameRate(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRoiBgFrameRate_t *pstGetRoiBgFrameRate = (MI_VENC_GetRoiBgFrameRate_t *)ptr;
    MI_VENC_CHN VeChn = pstGetRoiBgFrameRate->VeChn;
    MI_VENC_RoiBgFrameRate_t *pstRoiBgFrmRate = &pstGetRoiBgFrameRate->stRoiBgFrmRate;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetRoiBgFrameRate(VeChn, pstRoiBgFrmRate);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH264SliceSplit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264SliceSplit_t *pstSetH264SliceSplit = (MI_VENC_SetH264SliceSplit_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH264SliceSplit->VeChn;
    MI_VENC_ParamH264SliceSplit_t *pstSliceSplit = &pstSetH264SliceSplit->stSliceSplit;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH264SliceSplit(VeChn, pstSliceSplit);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH264SliceSplit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264SliceSplit_t *pstGetH264SliceSplit = (MI_VENC_GetH264SliceSplit_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH264SliceSplit->VeChn;
    MI_VENC_ParamH264SliceSplit_t *pstSliceSplit = &pstGetH264SliceSplit->stSliceSplit;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH264SliceSplit(VeChn, pstSliceSplit);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH264InterPred(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264InterPred_t *pstSetH264InterPred = (MI_VENC_SetH264InterPred_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH264InterPred->VeChn;
    MI_VENC_ParamH264InterPred_t *pstH264InterPred = &pstSetH264InterPred->stH264InterPred;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH264InterPred(VeChn, pstH264InterPred);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH264InterPred(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264InterPred_t *pstGetH264InterPred = (MI_VENC_GetH264InterPred_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH264InterPred->VeChn;
    MI_VENC_ParamH264InterPred_t *pstH264InterPred = &pstGetH264InterPred->stH264InterPred;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH264InterPred(VeChn, pstH264InterPred);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH264IntraPred(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264IntraPred_t *pstSetH264IntraPred = (MI_VENC_SetH264IntraPred_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH264IntraPred->VeChn;
    MI_VENC_ParamH264IntraPred_t *pstH264IntraPred = &pstSetH264IntraPred->stH264IntraPred;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH264IntraPred(VeChn, pstH264IntraPred);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH264IntraPred(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264IntraPred_t *pstGetH264IntraPred = (MI_VENC_GetH264IntraPred_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH264IntraPred->VeChn;
    MI_VENC_ParamH264IntraPred_t *pstH264IntraPred = &pstGetH264IntraPred->stH264IntraPred;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH264IntraPred(VeChn, pstH264IntraPred);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH264Trans(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264Trans_t *pstSetH264Trans = (MI_VENC_SetH264Trans_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH264Trans->VeChn;
    MI_VENC_ParamH264Trans_t *pstH264Trans = &pstSetH264Trans->stH264Trans;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH264Trans(VeChn, pstH264Trans);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH264Trans(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264Trans_t *pstGetH264Trans = (MI_VENC_GetH264Trans_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH264Trans->VeChn;
    MI_VENC_ParamH264Trans_t *pstH264Trans = &pstGetH264Trans->stH264Trans;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH264Trans(VeChn, pstH264Trans);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH264Entropy(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264Entropy_t *pstSetH264Entropy = (MI_VENC_SetH264Entropy_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH264Entropy->VeChn;
    MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc = &pstSetH264Entropy->stH264EntropyEnc;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH264Entropy(VeChn, pstH264EntropyEnc);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH264Entropy(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264Entropy_t *pstGetH264Entropy = (MI_VENC_GetH264Entropy_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH264Entropy->VeChn;
    MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc = &pstGetH264Entropy->stH264EntropyEnc;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH264Entropy(VeChn, pstH264EntropyEnc);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH264Dblk(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264Dblk_t *pstSetH264Dblk = (MI_VENC_SetH264Dblk_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH264Dblk->VeChn;
    MI_VENC_ParamH264Dblk_t *pstH264Dblk = &pstSetH264Dblk->stH264Dblk;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH264Dblk(VeChn, pstH264Dblk);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH264Dblk(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264Dblk_t *pstGetH264Dblk = (MI_VENC_GetH264Dblk_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH264Dblk->VeChn;
    MI_VENC_ParamH264Dblk_t *pstH264Dblk = &pstGetH264Dblk->stH264Dblk;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH264Dblk(VeChn, pstH264Dblk);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH264Vui(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264Vui_t *pstSetH264Vui = (MI_VENC_SetH264Vui_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH264Vui->VeChn;
    MI_VENC_ParamH264Vui_t*pstH264Vui = &pstSetH264Vui->stH264Vui;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH264Vui(VeChn, pstH264Vui);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH264Vui(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264Vui_t *pstGetH264Vui = (MI_VENC_GetH264Vui_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH264Vui->VeChn;
    MI_VENC_ParamH264Vui_t *pstH264Vui = &pstGetH264Vui->stH264Vui;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH264Vui(VeChn, pstH264Vui);
    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH265SliceSplit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265SliceSplit_t *pstSetH265SliceSplit = (MI_VENC_SetH265SliceSplit_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH265SliceSplit->VeChn;
    MI_VENC_ParamH265SliceSplit_t *pstSliceSplit = &pstSetH265SliceSplit->stSliceSplit;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH265SliceSplit(VeChn, pstSliceSplit);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH265SliceSplit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265SliceSplit_t *pstGetH265SliceSplit = (MI_VENC_GetH265SliceSplit_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH265SliceSplit->VeChn;
    MI_VENC_ParamH265SliceSplit_t *pstSliceSplit = &pstGetH265SliceSplit->stSliceSplit;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH265SliceSplit(VeChn, pstSliceSplit);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH265InterPred(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265InterPred_t *pstSetH265InterPred = (MI_VENC_SetH265InterPred_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH265InterPred->VeChn;
    MI_VENC_ParamH265InterPred_t *pstH265InterPred = &pstSetH265InterPred->stH265InterPred;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH265InterPred(VeChn, pstH265InterPred);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH265InterPred(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265InterPred_t *pstGetH265InterPred = (MI_VENC_GetH265InterPred_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH265InterPred->VeChn;
    MI_VENC_ParamH265InterPred_t *pstH265InterPred = &pstGetH265InterPred->stH265InterPred;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH265InterPred(VeChn, pstH265InterPred);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH265IntraPred(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265IntraPred_t *pstSetH265IntraPred = (MI_VENC_SetH265IntraPred_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH265IntraPred->VeChn;
    MI_VENC_ParamH265IntraPred_t *pstH265IntraPred = &pstSetH265IntraPred->stH265IntraPred;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH265IntraPred(VeChn, pstH265IntraPred);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH265IntraPred(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265IntraPred_t *pstGetH265IntraPred = (MI_VENC_GetH265IntraPred_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH265IntraPred->VeChn;
    MI_VENC_ParamH265IntraPred_t *pstH265IntraPred = &pstGetH265IntraPred->stH265IntraPred;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH265IntraPred(VeChn, pstH265IntraPred);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH265Trans(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265Trans_t *pstSetH265Trans = (MI_VENC_SetH265Trans_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH265Trans->VeChn;
    MI_VENC_ParamH265Trans_t *pstH265Trans = &pstSetH265Trans->stH265Trans;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH265Trans(VeChn, pstH265Trans);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH265Trans(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265Trans_t *pstGetH265Trans = (MI_VENC_GetH265Trans_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH265Trans->VeChn;
    MI_VENC_ParamH265Trans_t *pstH265Trans = &pstGetH265Trans->stH265Trans;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH265Trans(VeChn, pstH265Trans);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH265Dblk(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265Dblk_t *pstSetH265Dblk = (MI_VENC_SetH265Dblk_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH265Dblk->VeChn;
    MI_VENC_ParamH265Dblk_t *pstH265Dblk = &pstSetH265Dblk->stH265Dblk;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH265Dblk(VeChn, pstH265Dblk);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH265Dblk(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265Dblk_t *pstGetH265Dblk = (MI_VENC_GetH265Dblk_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH265Dblk->VeChn;
    MI_VENC_ParamH265Dblk_t *pstH265Dblk = &pstGetH265Dblk->stH265Dblk;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH265Dblk(VeChn, pstH265Dblk);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetH265Vui(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265Vui_t *pstSetH265Vui = (MI_VENC_SetH265Vui_t *)ptr;
    MI_VENC_CHN VeChn = pstSetH265Vui->VeChn;
    MI_VENC_ParamH265Vui_t*pstH265Vui = &pstSetH265Vui->stH265Vui;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetH265Vui(VeChn, pstH265Vui);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetH265Vui(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265Vui_t *pstGetH265Vui = (MI_VENC_GetH265Vui_t *)ptr;
    MI_VENC_CHN VeChn = pstGetH265Vui->VeChn;
    MI_VENC_ParamH265Vui_t *pstH265Vui = &pstGetH265Vui->stH265Vui;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetH265Vui(VeChn, pstH265Vui);
    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetJpegParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetJpegParam_t *pstSetJpegParam = (MI_VENC_SetJpegParam_t *)ptr;
    MI_VENC_CHN VeChn = pstSetJpegParam->VeChn;
    MI_VENC_ParamJpeg_t *pstJpegParam = &pstSetJpegParam->stParamJpeg;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetJpegParam(VeChn, pstJpegParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetJpegParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetJpegParam_t *pstGetJpegParam = (MI_VENC_GetJpegParam_t *)ptr;
    MI_VENC_CHN VeChn = pstGetJpegParam->VeChn;
    MI_VENC_ParamJpeg_t *pstJpegParam = &pstGetJpegParam->stParamJpeg;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetJpegParam(VeChn, pstJpegParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetRcParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRcParam_t *pstSetRcParam = (MI_VENC_SetRcParam_t *)ptr;
    MI_VENC_CHN VeChn = pstSetRcParam->VeChn;
    MI_VENC_RcParam_t *pstRcParam = &pstSetRcParam->stRcParam;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetRcParam(VeChn, pstRcParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetRcParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRcParam_t *pstGetRcParam = (MI_VENC_GetRcParam_t *)ptr;
    MI_VENC_CHN VeChn = pstGetRcParam->VeChn;
    MI_VENC_RcParam_t *pstRcParam = &pstGetRcParam->stRcParam;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetRcParam(VeChn, pstRcParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetRefParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRefParam_t *pstSetRefParam = (MI_VENC_SetRefParam_t *)ptr;
    MI_VENC_CHN VeChn = pstSetRefParam->VeChn;
    MI_VENC_ParamRef_t * pstRefParam = &pstSetRefParam->stRefParam;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetRefParam(VeChn, pstRefParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetRefParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRefParam_t *pstGetRefParam = (MI_VENC_GetRefParam_t *)ptr;
    MI_VENC_CHN VeChn = pstGetRefParam->VeChn;
    MI_VENC_ParamRef_t * pstRefParam = &pstGetRefParam->stRefParam;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetRefParam(VeChn, pstRefParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetCrop(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetCrop_t *pstSetCrop = (MI_VENC_SetCrop_t *)ptr;
    MI_VENC_CHN VeChn = pstSetCrop->VeChn;
    MI_VENC_CropCfg_t *pstCropCfg = &pstSetCrop->stCropCfg;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetCrop(VeChn, pstCropCfg);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetCrop(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetCrop_t *pstGetCrop = (MI_VENC_SetCrop_t *)ptr;
    MI_VENC_CHN VeChn = pstGetCrop->VeChn;
    MI_VENC_CropCfg_t *pstCropCfg = &pstGetCrop->stCropCfg;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetCrop(VeChn, pstCropCfg);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetFrameLostStrategy(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetFrameLostStrategy_t *pstSetFrameLostStrategy = (MI_VENC_SetFrameLostStrategy_t *)ptr;
    MI_VENC_CHN VeChn = pstSetFrameLostStrategy->VeChn;
    MI_VENC_ParamFrameLost_t *pstFrmLostParam = &pstSetFrameLostStrategy->stFrmLostParam;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetFrameLostStrategy(VeChn, pstFrmLostParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetFrameLostStrategy(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetFrameLostStrategy_t *pstGetFrameLostStrategy = (MI_VENC_GetFrameLostStrategy_t *)ptr;
    MI_VENC_CHN VeChn = pstGetFrameLostStrategy->VeChn;
    MI_VENC_ParamFrameLost_t *pstFrmLostParam = &pstGetFrameLostStrategy->stFrmLostParam;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetFrameLostStrategy(VeChn, pstFrmLostParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetSuperFrameCfg(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetSuperFrameCfg_t *pstSetSuperFrameCfg = (MI_VENC_SetSuperFrameCfg_t *)ptr;
    MI_VENC_CHN VeChn = pstSetSuperFrameCfg->VeChn;
    MI_VENC_SuperFrameCfg_t *pstSuperFrmParam = &pstSetSuperFrameCfg->stSuperFrmParam;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetSuperFrameCfg(VeChn, pstSuperFrmParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetSuperFrameCfg(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetSuperFrameCfg_t *pstGetSuperFrameCfg = (MI_VENC_GetSuperFrameCfg_t *)ptr;
    MI_VENC_CHN VeChn = pstGetSuperFrameCfg->VeChn;
    MI_VENC_SuperFrameCfg_t *pstSuperFrmParam = &pstGetSuperFrameCfg->stSuperFrmParam;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetSuperFrameCfg(VeChn, pstSuperFrmParam);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_SetRcPriority(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRcPriority_t *pstSetRcPriority = (MI_VENC_SetRcPriority_t *)ptr;
    MI_VENC_CHN VeChn = pstSetRcPriority->VeChn;
    MI_VENC_RcPriority_e *peRcPriority = &pstSetRcPriority->eRcPriority;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_SetRcPriority(VeChn, peRcPriority);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetRcPriority(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRcPriority_t *pstGetRcPriority = (MI_VENC_GetRcPriority_t *)ptr;
    MI_VENC_CHN VeChn = pstGetRcPriority->VeChn;
    MI_VENC_RcPriority_e *peRcPriority = &pstGetRcPriority->eRcPriority;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetRcPriority(VeChn, peRcPriority);

    return s32Ret;
}

static MI_S32 MI_VENC_IOCTL_GetChnDevid(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetChnDevid_t *pstGetChnDevid = (MI_VENC_GetChnDevid_t *)ptr;
    MI_VENC_CHN VeChn = pstGetChnDevid->VeChn;
    MI_U32 *pu32Devid = &pstGetChnDevid->u32DevId;

    AUTO_LOCK(env, mutex);
    PRINTLN_CHN(VeChn);
    s32Ret = MI_VENC_IMPL_GetChnDevid(VeChn, pu32Devid);

    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_VENC_CMD_MAX] = {
    [E_MI_VENC_CMD_SET_MOD_PARAM] = MI_VENC_IOCTL_SetModParam,
    [E_MI_VENC_CMD_GET_MOD_PARAM] = MI_VENC_IOCTL_GetModParam,
    [E_MI_VENC_CMD_CREATE_CHN] = MI_VENC_IOCTL_CreateChn,
    [E_MI_VENC_CMD_DESTROY_CHN] = MI_VENC_IOCTL_DestroyChn,
    [E_MI_VENC_CMD_RESET_CHN] = MI_VENC_IOCTL_ResetChn,
    [E_MI_VENC_CMD_START_RECV_PIC] = MI_VENC_IOCTL_StartRecvPic,
    [E_MI_VENC_CMD_START_RECV_PIC_EX] = MI_VENC_IOCTL_StartRecvPicEx,
    [E_MI_VENC_CMD_STOP_RECV_PIC] = MI_VENC_IOCTL_StopRecvPic,
    [E_MI_VENC_CMD_QUERY] = MI_VENC_IOCTL_Query,
    [E_MI_VENC_CMD_SET_CHN_ATTR] = MI_VENC_IOCTL_SetChnAttr,
    [E_MI_VENC_CMD_GET_CHN_ATTR] = MI_VENC_IOCTL_GetChnAttr,
    [E_MI_VENC_CMD_GET_STREAM] = MI_VENC_IOCTL_GetStream,
    [E_MI_VENC_CMD_RELEASE_STREAM] = MI_VENC_IOCTL_ReleaseStream,
    [E_MI_VENC_CMD_INSERT_USER_DATA] = MI_VENC_IOCTL_InsertUserData,
    [E_MI_VENC_CMD_SET_MAX_STREAM_CNT] = MI_VENC_IOCTL_SetMaxStreamCnt,
    [E_MI_VENC_CMD_GET_MAX_STREAM_CNT] = MI_VENC_IOCTL_GetMaxStreamCnt,
    [E_MI_VENC_CMD_REQUEST_IDR] = MI_VENC_IOCTL_RequestIdr,
    [E_MI_VENC_CMD_ENABLE_IDR] = MI_VENC_IOCTL_EnableIdr,
    [E_MI_VENC_CMD_SET_H264_IDR_PIC_ID] = MI_VENC_IOCTL_SetH264IdrPicId,
    [E_MI_VENC_CMD_GET_H264_IDR_PIC_ID] = MI_VENC_IOCTL_GetH264IdrPicId,
    [E_MI_VENC_CMD_GET_FD] = MI_VENC_IOCTL_GetFd,
    [E_MI_VENC_CMD_CLOSE_FD] = MI_VENC_IOCTL_CloseFd,
    [E_MI_VENC_CMD_SET_ROI_CFG] = MI_VENC_IOCTL_SetRoiCfg,
    [E_MI_VENC_CMD_GET_ROI_CFG] = MI_VENC_IOCTL_GetRoiCfg,
    [E_MI_VENC_CMD_SET_ROI_BG_FRAME_RATE] = MI_VENC_IOCTL_SetRoiBgFrameRate,
    [E_MI_VENC_CMD_GET_ROI_BG_FRAME_RATE] = MI_VENC_IOCTL_GetRoiBgFrameRate,
    [E_MI_VENC_CMD_SET_H264_SLICE_SPLIT] = MI_VENC_IOCTL_SetH264SliceSplit,
    [E_MI_VENC_CMD_GET_H264_SLICE_SPLIT] = MI_VENC_IOCTL_GetH264SliceSplit,
    [E_MI_VENC_CMD_SET_H264_INTER_PRED] = MI_VENC_IOCTL_SetH264InterPred,
    [E_MI_VENC_CMD_GET_H264_INTER_PRED] = MI_VENC_IOCTL_GetH264InterPred,
    [E_MI_VENC_CMD_SET_H264_INTRA_PRED] = MI_VENC_IOCTL_SetH264IntraPred,
    [E_MI_VENC_CMD_GET_H264_INTRA_PRED] = MI_VENC_IOCTL_GetH264IntraPred,
    [E_MI_VENC_CMD_SET_H264_TRANS] = MI_VENC_IOCTL_SetH264Trans,
    [E_MI_VENC_CMD_GET_H264_TRANS] = MI_VENC_IOCTL_GetH264Trans,
    [E_MI_VENC_CMD_SET_H264_ENTROPY] = MI_VENC_IOCTL_SetH264Entropy,
    [E_MI_VENC_CMD_GET_H264_ENTROPY] = MI_VENC_IOCTL_GetH264Entropy,
    [E_MI_VENC_CMD_SET_H264_DBLK] = MI_VENC_IOCTL_SetH264Dblk,
    [E_MI_VENC_CMD_GET_H264_DBLK] = MI_VENC_IOCTL_GetH264Dblk,
    [E_MI_VENC_CMD_SET_H264_VUI] = MI_VENC_IOCTL_SetH264Vui,
    [E_MI_VENC_CMD_GET_H264_VUI] = MI_VENC_IOCTL_GetH264Vui,
    [E_MI_VENC_CMD_SET_H265_SLICE_SPLIT] = MI_VENC_IOCTL_SetH265SliceSplit,
    [E_MI_VENC_CMD_GET_H265_SLICE_SPLIT] = MI_VENC_IOCTL_GetH265SliceSplit,
    [E_MI_VENC_CMD_SET_H265_INTER_PRED] = MI_VENC_IOCTL_SetH265InterPred,
    [E_MI_VENC_CMD_GET_H265_INTER_PRED] = MI_VENC_IOCTL_GetH265InterPred,
    [E_MI_VENC_CMD_SET_H265_INTRA_PRED] = MI_VENC_IOCTL_SetH265IntraPred,
    [E_MI_VENC_CMD_GET_H265_INTRA_PRED] = MI_VENC_IOCTL_GetH265IntraPred,
    [E_MI_VENC_CMD_SET_H265_TRANS] = MI_VENC_IOCTL_SetH265Trans,
    [E_MI_VENC_CMD_GET_H265_TRANS] = MI_VENC_IOCTL_GetH265Trans,
    [E_MI_VENC_CMD_SET_H265_DBLK] = MI_VENC_IOCTL_SetH265Dblk,
    [E_MI_VENC_CMD_GET_H265_DBLK] = MI_VENC_IOCTL_GetH265Dblk,
    [E_MI_VENC_CMD_SET_H265_VUI] = MI_VENC_IOCTL_SetH265Vui,
    [E_MI_VENC_CMD_GET_H265_VUI] = MI_VENC_IOCTL_GetH265Vui,
    [E_MI_VENC_CMD_SET_JPEG_PARAM] = MI_VENC_IOCTL_SetJpegParam,
    [E_MI_VENC_CMD_GET_JPEG_PARAM] = MI_VENC_IOCTL_GetJpegParam,
    [E_MI_VENC_CMD_SET_RC_PARAM] = MI_VENC_IOCTL_SetRcParam,
    [E_MI_VENC_CMD_GET_RC_PARAM] = MI_VENC_IOCTL_GetRcParam,
    [E_MI_VENC_CMD_SET_REF_PARAM] = MI_VENC_IOCTL_SetRefParam,
    [E_MI_VENC_CMD_GET_REF_PARAM] = MI_VENC_IOCTL_GetRefParam,
    [E_MI_VENC_CMD_SET_CROP] = MI_VENC_IOCTL_SetCrop,
    [E_MI_VENC_CMD_GET_CROP] = MI_VENC_IOCTL_GetCrop,
    [E_MI_VENC_CMD_SET_FRAME_LOST_STRATEGY] = MI_VENC_IOCTL_SetFrameLostStrategy,
    [E_MI_VENC_CMD_GET_FRAME_LOST_STRATEGY] = MI_VENC_IOCTL_GetFrameLostStrategy,
    [E_MI_VENC_CMD_SET_SUPER_FRAME_CFG] = MI_VENC_IOCTL_SetSuperFrameCfg,
    [E_MI_VENC_CMD_GET_SUPER_FRAME_CFG] = MI_VENC_IOCTL_GetSuperFrameCfg,
    [E_MI_VENC_CMD_SET_RC_PRIORITY] = MI_VENC_IOCTL_SetRcPriority,
    [E_MI_VENC_CMD_GET_RC_PRIORITY] = MI_VENC_IOCTL_GetRcPriority,
    [E_MI_VENC_CMD_GET_CHN_DEVID] = MI_VENC_IOCTL_GetChnDevid,
};


void MI_VENC_IMPL_Init(void);
void MI_VENC_IMPL_DeInit(void);

static void mi_venc_insmod(const char *name){
    MI_PRINT("module [%s] init "__DATE__" "__TIME__ "\n", name);
}
static void mi_venc_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
    MI_VENC_IMPL_Init();
}
static void mi_venc_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
    MI_VENC_IMPL_DeInit();
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");

MI_DEVICE_DEFINE(mi_venc_insmod,
               ioctl_table, E_MI_VENC_CMD_MAX,
               mi_venc_process_init, mi_venc_process_exit);
