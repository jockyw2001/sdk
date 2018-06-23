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
#include "mi_disp_datatype.h"
#include "mi_disp_impl.h"
#include "disp_ioctl.h"

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

static MI_S32 MI_DISP_IOCTL_Enable(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_DEV DispDev = *(MI_DISP_DEV *)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_Enable(DispDev);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_Disable(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_DEV DispDev = *(MI_DISP_DEV *)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_Disable(DispDev);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetPubAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetPubAttr_t *pstSetPubAttr = (MI_DISP_SetPubAttr_t *)ptr;
    MI_DISP_DEV DispDev = pstSetPubAttr->DispDev;
    const MI_DISP_PubAttr_t *pstPubAttr = &pstSetPubAttr->stPubAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_SetPubAttr(DispDev, pstPubAttr);
    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_GetPubAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetPubAttr_t *pstGetPubAttr = (MI_DISP_GetPubAttr_t *)ptr;
    MI_DISP_DEV DispDev = pstGetPubAttr->DispDev;
    MI_DISP_PubAttr_t *pstPubAttr = &pstGetPubAttr->stPubAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> Dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_GetPubAttr(DispDev, pstPubAttr);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_DeviceAttach(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_DeviceAttach_t *pstDeviceAttach = (MI_DISP_DeviceAttach_t *)ptr;
    MI_DISP_DEV DispSrcDev = pstDeviceAttach->DispSrcDev;
    MI_DISP_DEV DispDstDev = pstDeviceAttach->DispDstDev;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> source dev: %d --> dest dev: %d.\n", __func__, DispSrcDev, DispDstDev);
    s32Ret = MI_DISP_IMPL_DeviceAttach(DispSrcDev, DispDstDev);
    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_DeviceDetach(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_DeviceDetach_t *pstDeviceDetach = (MI_DISP_DeviceDetach_t *)ptr;
    MI_DISP_DEV DispSrcDev = pstDeviceDetach->DispSrcDev;
    MI_DISP_DEV DispDstDev = pstDeviceDetach->DispDstDev;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> source dev: %d  --> dest dev: %d.\n", __func__, DispSrcDev, DispDstDev);
    s32Ret = MI_DISP_IMPL_DeviceDetach(DispSrcDev, DispDstDev);
    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_EnableVideoLayer(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_LAYER DispLayer = *(MI_DISP_LAYER*)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d.\n", __func__, DispLayer);
    s32Ret = MI_DISP_IMPL_EnableVideoLayer(DispLayer);
    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_DisableVideoLayer(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_LAYER DispLayer = *(MI_DISP_LAYER *)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d.\n", __func__, DispLayer);
    s32Ret = MI_DISP_IMPL_DisableVideoLayer(DispLayer);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetVideoLayerAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetVideoLayerAttr_t *pstSetVideoLayerAttr = (MI_DISP_SetVideoLayerAttr_t *)ptr;
    MI_DISP_LAYER DispLayer = pstSetVideoLayerAttr->DispLayer;
    const MI_DISP_VideoLayerAttr_t *pstLayerAttr = &pstSetVideoLayerAttr->stLayerAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d.\n", __func__, DispLayer);
    s32Ret = MI_DISP_IMPL_SetVideoLayerAttr(DispLayer, pstLayerAttr);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_GetVideoLayerAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetVideoLayerAttr_t *pstGetVideoLayerAttr = (MI_DISP_GetVideoLayerAttr_t *)ptr;
    MI_DISP_LAYER DispLayer = pstGetVideoLayerAttr->DispLayer;
    MI_DISP_VideoLayerAttr_t *pstLayerAttr = &pstGetVideoLayerAttr->stLayerAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d.\n", __func__, DispLayer);
    s32Ret = MI_DISP_IMPL_GetVideoLayerAttr(DispLayer, pstLayerAttr);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_BindVideoLayer(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_BindVideoLayer_t *pstBindVideoLayer = (MI_DISP_BindVideoLayer_t *)ptr;
    MI_DISP_LAYER DispLayer = pstBindVideoLayer->DispLayer;
    MI_DISP_DEV DispDev = pstBindVideoLayer->DispDev;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d Dev: %d.\n", __func__, DispLayer, DispDev);
    s32Ret = MI_DISP_IMPL_BindVideoLayer(DispLayer, DispDev);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_UnBindVideoLayer(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_UnBindVideoLayer_t *pstUnBindVideoLayer = (MI_DISP_UnBindVideoLayer_t *)ptr;
    MI_DISP_LAYER DispLayer = pstUnBindVideoLayer->DispLayer;
    MI_DISP_DEV DispDev = pstUnBindVideoLayer->DispDev;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d dev: %d.\n", __func__, DispLayer, DispDev);
    s32Ret = MI_DISP_IMPL_UnBindVideoLayer(DispLayer, DispDev);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetPlayToleration(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetPlayToleration_t *pstSetPlayToleration = (MI_DISP_SetPlayToleration_t *)ptr;
    MI_DISP_LAYER DispLayer = pstSetPlayToleration->DispLayer;
    MI_U32 u32Toleration = pstSetPlayToleration->u32Toleration;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d u32Toleration: %d.\n", __func__, DispLayer, u32Toleration);
    s32Ret = MI_DISP_IMPL_SetPlayToleration(DispLayer, u32Toleration);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_GetPlayToleration(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetPlayToleration_t *pstGetPlayToleration = (MI_DISP_GetPlayToleration_t *)ptr;
    MI_DISP_LAYER DispLayer = pstGetPlayToleration->DispLayer;
    MI_U32 *pu32Toleration = &pstGetPlayToleration->u32Toleration;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_DISP_IMPL_GetPlayToleration(DispLayer, pu32Toleration);
    DBG_INFO("<%s> layer: %d u32Toleration: %d.\n", __func__, DispLayer, *pu32Toleration);

    return s32Ret;
}

static MI_S32 mi_disp_ioctl_GetOutputtiming(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetOutputTiming_t *pstOutputTiming = (MI_DISP_GetOutputTiming_t *)ptr;
    MI_DISP_LAYER DispLayer = pstOutputTiming->DispLayer;
    MI_DISP_OutputTiming_e *pOutputTiming = &(pstOutputTiming->eOutputTiming);
    MI_DISP_SyncInfo_t* pstSyncInfo = &(pstOutputTiming->stSyncInfo);
    AUTO_LOCK(env, mutex);
    s32Ret = mi_disp_impl_GetOutputTiming(DispLayer, pOutputTiming, pstSyncInfo);

    return s32Ret;
}


static MI_S32 MI_DISP_IOCTL_GetScreenFrame(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetScreenFrame_t *pstGetScreenFrame = (MI_DISP_GetScreenFrame_t *)ptr;
    MI_DISP_LAYER DispLayer = pstGetScreenFrame->DispLayer;
    MI_DISP_VideoFrame_t *pstVFrame = &pstGetScreenFrame->stVFrame;
    //MI_U32  u32MilliSec = pstGetScreenFrame->u32MilliSec;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d u32MilliSec: %d.\n", __func__, DispLayer);
    s32Ret = MI_DISP_IMPL_GetScreenFrame(DispLayer, pstVFrame);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_ReleaseScreenFrame(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_ReleaseScreenFrame_t *pstReleaseScreenFrame = (MI_DISP_ReleaseScreenFrame_t *)ptr;
    MI_DISP_LAYER DispLayer = pstReleaseScreenFrame->DispLayer;
    MI_DISP_VideoFrame_t *pstVFrame = &pstReleaseScreenFrame->stVFrame;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d.\n", __func__, DispLayer);
    s32Ret = MI_DISP_IMPL_ReleaseScreenFrame(DispLayer, pstVFrame);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetVideoLayerAttrBegin(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_LAYER DispLayer = *(MI_DISP_LAYER*)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d.\n", __func__, DispLayer);
    s32Ret = MI_DISP_IMPL_SetVideoLayerAttrBegin(DispLayer);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetVideoLayerAttrEnd(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_LAYER DispLayer = *(MI_DISP_LAYER*)ptr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d.\n", __func__, DispLayer);
    s32Ret = MI_DISP_IMPL_SetVideoLayerAttrEnd(DispLayer);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetInputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetInputPortAttr_t *pstSetInputPortAttr = (MI_DISP_SetInputPortAttr_t *)ptr;
    MI_DISP_LAYER DispLayer = pstSetInputPortAttr->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstSetInputPortAttr->LayerInputPort;
    const MI_DISP_InputPortAttr_t *pstInputPortAttr = &pstSetInputPortAttr->stInputPortAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d.\n", __func__, DispLayer);
    s32Ret = MI_DISP_IMPL_SetInputPortAttr(DispLayer, LayerInputPort, pstInputPortAttr);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_GetInputPortAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetInputPortAttr_t *pstGetInputPortAttr = (MI_DISP_GetInputPortAttr_t *)ptr;
    MI_DISP_LAYER DispLayer = pstGetInputPortAttr->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstGetInputPortAttr->LayerInputPort;
    MI_DISP_InputPortAttr_t *pstInputPortAttr = &pstGetInputPortAttr->stInputPortAttr;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_GetInputPortAttr(DispLayer, LayerInputPort, pstInputPortAttr);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_EnableInputPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_EnableInputPort_t *pstEnableInputPort = (MI_DISP_EnableInputPort_t *)ptr;
    MI_DISP_LAYER DispLayer = pstEnableInputPort->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstEnableInputPort->LayerInputPort;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_EnableInputPort(DispLayer, LayerInputPort);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_DisableInputPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_DisableInputPort_t *pstDisableInputPort = (MI_DISP_DisableInputPort_t *)ptr;
    MI_DISP_LAYER DispLayer = pstDisableInputPort->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstDisableInputPort->LayerInputPort;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_DisableInputPort(DispLayer, LayerInputPort);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetInputPortDispPos(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetInputPortDispPos_t *pstSetInputPortDispPos = (MI_DISP_SetInputPortDispPos_t *)ptr;
    MI_DISP_LAYER DispLayer = pstSetInputPortDispPos->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstSetInputPortDispPos->LayerInputPort;
    const MI_DISP_Position_t *pstDispPos = &pstSetInputPortDispPos->stDispPos;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_SetInputPortDispPos(DispLayer, LayerInputPort, pstDispPos);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_GetInputPortDispPos(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetInputPortDispPos_t *pstGetInputPortDispPos = (MI_DISP_GetInputPortDispPos_t *)ptr;
    MI_DISP_LAYER DispLayer = pstGetInputPortDispPos->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstGetInputPortDispPos->LayerInputPort;
    MI_DISP_Position_t *pstDispPos = &pstGetInputPortDispPos->stDispPos;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_GetInputPortDispPos(DispLayer, LayerInputPort, pstDispPos);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_PauseInputPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_PauseInputPort_t *pstPauseInputPort = (MI_DISP_PauseInputPort_t *)ptr;
    MI_DISP_LAYER DispLayer = pstPauseInputPort->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstPauseInputPort->LayerInputPort;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_PauseInputPort(DispLayer, LayerInputPort);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_ResumeInputPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_ResumeInputPort_t *pstResumeInputPort = (MI_DISP_ResumeInputPort_t *)ptr;
    MI_DISP_LAYER DispLayer = pstResumeInputPort->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstResumeInputPort->LayerInputPort;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_ResumeInputPort(DispLayer, LayerInputPort);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_StepInputPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_StepInputPort_t *pstStepInputPort = (MI_DISP_StepInputPort_t *)ptr;
    MI_DISP_LAYER DispLayer = pstStepInputPort->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstStepInputPort->LayerInputPort;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_StepInputPort(DispLayer, LayerInputPort);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_ShowInputPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_ShowInputPort_t *pstShowInputPort = (MI_DISP_ShowInputPort_t *)ptr;
    MI_DISP_LAYER DispLayer = pstShowInputPort->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstShowInputPort->LayerInputPort;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_ShowInputPort(DispLayer, LayerInputPort);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_HideInputPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_HideInputPort_t *pstHideInputPort = (MI_DISP_HideInputPort_t *)ptr;
    MI_DISP_LAYER DispLayer = pstHideInputPort->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstHideInputPort->LayerInputPort;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_HideInputPort(DispLayer, LayerInputPort);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetInputPortSyncMode(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetInputPortSyncMode_t *pstSetInputPortSyncMode = (MI_DISP_SetInputPortSyncMode_t *)ptr;
    MI_DISP_LAYER DispLayer = pstSetInputPortSyncMode->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstSetInputPortSyncMode->LayerInputPort;
    MI_DISP_SyncMode_e eMode = pstSetInputPortSyncMode->eMode;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d eMode: %d.\n", __func__, DispLayer, LayerInputPort, eMode);
    s32Ret = MI_DISP_IMPL_SetInputPortSyncMode(DispLayer, LayerInputPort, eMode);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_QueryInputPortStat(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_QueryInputPortStat_t *pstQueryInputPortStat = (MI_DISP_QueryInputPortStat_t *)ptr;
    MI_DISP_LAYER DispLayer = pstQueryInputPortStat->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstQueryInputPortStat->LayerInputPort;
    MI_DISP_QueryChannelStatus_t *pstStatus = &pstQueryInputPortStat->stStatus;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_QueryInputPortStat(DispLayer, LayerInputPort, pstStatus);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetZoomInWindow(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetZoomInWindow_t *pstZoomWindow = (MI_DISP_SetZoomInWindow_t *)ptr;
    MI_DISP_LAYER DispLayer = pstZoomWindow->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstZoomWindow->LayerInputPort;
    MI_DISP_VidWinRect_t stCropWin = pstZoomWindow->stCropWin;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d.\n", __func__, DispLayer, LayerInputPort);
    s32Ret = MI_DISP_IMPL_SetZoomInWindow(DispLayer, LayerInputPort, &stCropWin);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_GetVgaParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetVgaParam_t *pstGetVgaParam = (MI_DISP_GetVgaParam_t *)ptr;
    MI_DISP_DEV DispDev = pstGetVgaParam->DispDev;
    MI_DISP_VgaParam_t *pstVgaParam = &pstGetVgaParam->stVgaParam;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_GetVgaParam(DispDev, pstVgaParam);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetVgaParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetVgaParam_t *pstSetVgaParam = (MI_DISP_SetVgaParam_t *)ptr;
    MI_DISP_DEV DispDev = pstSetVgaParam->DispDev;
    MI_DISP_VgaParam_t *pstVgaParam = &pstSetVgaParam->stVgaParam;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_SetVgaParam(DispDev, pstVgaParam);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_GetHdmiParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetHdmiParam_t *pstGetHdmiParam = (MI_DISP_GetHdmiParam_t *)ptr;
    MI_DISP_DEV DispDev = pstGetHdmiParam->DispDev;
    MI_DISP_HdmiParam_t *pstHdmiParam = &pstGetHdmiParam->stHdmiParam;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_GetHdmiParam(DispDev, pstHdmiParam);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetHdmiParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetHdmiParam_t *pstSetHdmiParam = (MI_DISP_SetHdmiParam_t *)ptr;
    MI_DISP_DEV DispDev = pstSetHdmiParam->DispDev;
    MI_DISP_HdmiParam_t *pstHdmiParam = &pstSetHdmiParam->stHdmiParam;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_SetHdmiParam(DispDev, pstHdmiParam);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_GetCvbsParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetCvbsParam_t *pstGetCvbsParam = (MI_DISP_GetCvbsParam_t *)ptr;
    MI_DISP_DEV DispDev = pstGetCvbsParam->DispDev;
    MI_DISP_CvbsParam_t *pstCvbsParam = &pstGetCvbsParam->stCvbsParam;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_GetCvbsParam(DispDev, pstCvbsParam);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_SetCvbsParam(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetCvbsParam_t *pstSetCvbsParam = (MI_DISP_SetCvbsParam_t *)ptr;
    MI_DISP_DEV DispDev = pstSetCvbsParam->DispDev;
    MI_DISP_CvbsParam_t *pstCvbsParam = &pstSetCvbsParam->stCvbsParam;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> dev: %d.\n", __func__, DispDev);
    s32Ret = MI_DISP_IMPL_SetCvbsParam(DispDev, pstCvbsParam);

    return s32Ret;
}

static MI_S32 MI_DISP_IOCTL_ClearInputportBuffer(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_ClearInportBuff_t *pstClearInportBuff = (MI_DISP_ClearInportBuff_t*)ptr;   
    MI_DISP_LAYER DispLayer = pstClearInportBuff->DispLayer;
    MI_DISP_INPUTPORT LayerInputPort = pstClearInportBuff->LayerInputPort;
    MI_BOOL bClrAll = pstClearInportBuff->bClrAll;

    AUTO_LOCK(env, mutex);
    DBG_INFO("<%s> layer: %d input port: %d bClrAll: %d.\n", __func__, DispLayer, LayerInputPort, bClrAll);
    s32Ret = MI_DISP_IMPL_ClearInputPortBuffer(DispLayer, LayerInputPort, bClrAll);

    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_DISP_CMD_MAX] = {
    [E_MI_DISP_CMD_ENABLE] = MI_DISP_IOCTL_Enable,
    [E_MI_DISP_CMD_DISABLE] = MI_DISP_IOCTL_Disable,
    [E_MI_DISP_CMD_SET_PUB_ATTR] = MI_DISP_IOCTL_SetPubAttr,
    [E_MI_DISP_CMD_GET_PUB_ATTR] = MI_DISP_IOCTL_GetPubAttr,
    [E_MI_DISP_CMD_DEVICE_ATTACH] = MI_DISP_IOCTL_DeviceAttach,
    [E_MI_DISP_CMD_DEVICE_DETACH] = MI_DISP_IOCTL_DeviceDetach,
    [E_MI_DISP_CMD_ENABLE_VIDEO_LAYER] = MI_DISP_IOCTL_EnableVideoLayer,
    [E_MI_DISP_CMD_DISABLE_VIDEO_LAYER] = MI_DISP_IOCTL_DisableVideoLayer,

    [E_MI_DISP_CMD_SET_VIDEO_LAYER_ATTR] = MI_DISP_IOCTL_SetVideoLayerAttr,
    [E_MI_DISP_CMD_GET_VIDEO_LAYER_ATTR] = MI_DISP_IOCTL_GetVideoLayerAttr,
    [E_MI_DISP_CMD_BIND_VIDEO_LAYER] = MI_DISP_IOCTL_BindVideoLayer,
    [E_MI_DISP_CMD_UN_BIND_VIDEO_LAYER] = MI_DISP_IOCTL_UnBindVideoLayer,

    [E_MI_DISP_CMD_SET_PLAY_TOLERATION] = MI_DISP_IOCTL_SetPlayToleration,
    [E_MI_DISP_CMD_GET_PLAY_TOLERATION] = MI_DISP_IOCTL_GetPlayToleration,
    [E_MI_DISP_CMD_GET_SCREEN_FRAME] = MI_DISP_IOCTL_GetScreenFrame,
    [E_MI_DISP_CMD_RELEASE_SCREEN_FRAME] = MI_DISP_IOCTL_ReleaseScreenFrame,
    [E_MI_DISP_CMD_SET_VIDEO_LAYER_ATTR_BEGIN] = MI_DISP_IOCTL_SetVideoLayerAttrBegin,
    [E_MI_DISP_CMD_SET_VIDEO_LAYER_ATTR_END] = MI_DISP_IOCTL_SetVideoLayerAttrEnd,
    [E_MI_DISP_CMD_SET_INPUT_PORT_ATTR] = MI_DISP_IOCTL_SetInputPortAttr,
    [E_MI_DISP_CMD_GET_INPUT_PORT_ATTR] = MI_DISP_IOCTL_GetInputPortAttr,
    [E_MI_DISP_CMD_ENABLE_INPUT_PORT] = MI_DISP_IOCTL_EnableInputPort,
    [E_MI_DISP_CMD_DISABLE_INPUT_PORT] = MI_DISP_IOCTL_DisableInputPort,
    [E_MI_DISP_CMD_SET_INPUT_PORT_DISP_POS] = MI_DISP_IOCTL_SetInputPortDispPos,
    [E_MI_DISP_CMD_GET_INPUT_PORT_DISP_POS] = MI_DISP_IOCTL_GetInputPortDispPos,
    [E_MI_DISP_CMD_PAUSE_INPUT_PORT] = MI_DISP_IOCTL_PauseInputPort,
    [E_MI_DISP_CMD_RESUME_INPUT_PORT] = MI_DISP_IOCTL_ResumeInputPort,
    [E_MI_DISP_CMD_STEP_INPUT_PORT] = MI_DISP_IOCTL_StepInputPort,
    [E_MI_DISP_CMD_SHOW_INPUT_PORT] = MI_DISP_IOCTL_ShowInputPort,
    [E_MI_DISP_CMD_HIDE_INPUT_PORT] = MI_DISP_IOCTL_HideInputPort,
    [E_MI_DISP_CMD_SET_INPUT_PORT_SYNC_MODE] = MI_DISP_IOCTL_SetInputPortSyncMode,
    [E_MI_DISP_CMD_QUERY_INPUT_PORT_STAT] = MI_DISP_IOCTL_QueryInputPortStat,
    [E_MI_DISP_CMD_SET_ZOOM_WINDOW] = MI_DISP_IOCTL_SetZoomInWindow,
    [E_MI_DISP_CMD_GET_VGA_PARAM] = MI_DISP_IOCTL_GetVgaParam,
    [E_MI_DISP_CMD_SET_VGA_PARAM] = MI_DISP_IOCTL_SetVgaParam,
    [E_MI_DISP_CMD_GET_HDMI_PARAM] = MI_DISP_IOCTL_GetHdmiParam,
    [E_MI_DISP_CMD_SET_HDMI_PARAM] = MI_DISP_IOCTL_SetHdmiParam,
    [E_MI_DISP_CMD_GET_CVBS_PARAM] = MI_DISP_IOCTL_GetCvbsParam,
    [E_MI_DISP_CMD_SET_CVBS_PARAM] = MI_DISP_IOCTL_SetCvbsParam,
    [E_MI_DISP_CMD_CLEAR_INPORT_BUFFER] = MI_DISP_IOCTL_ClearInputportBuffer,
    [E_MI_DISP_CMD_GET_OUTPUTTIMING] = mi_disp_ioctl_GetOutputtiming,
};


static void mi_disp_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
}
static void mi_disp_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
    MI_DISP_IMPL_Init();
}
static void mi_disp_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
    MI_DISP_IMPL_DeInit();
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");

MI_DEVICE_DEFINE(mi_disp_insmod,
               ioctl_table, E_MI_DISP_CMD_MAX,
               mi_disp_process_init, mi_disp_process_exit);
