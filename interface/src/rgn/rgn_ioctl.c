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

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_rgn_datatype.h"
#include "mi_rgn_impl.h"
#include "mi_rgn_drv.h"
#include "rgn_ioctl.h"


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

static MI_S32 MI_RGN_IOCTL_Init(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_PaletteTable_t *pstInit = *((MI_RGN_PaletteTable_t **)ptr);
    //const MI_RGN_PaletteTable_t *pstPaletteTable

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_Init(pstInit);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_DeInit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    //void

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_DeInit();

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_Create(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_CreatePara_t *pstCreate = (MI_RGN_CreatePara_t *)ptr;
    MI_RGN_HANDLE hHandle = pstCreate->hHandle;
    MI_RGN_Attr_t *pstRgnAttr = &(pstCreate->stRegion);
    //MI_RGN_HANDLE hHandle, const MI_RGN_Attr_t *pstRegion

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_Create(hHandle, pstRgnAttr);
    //memcpy(MI_RGN_HANDLE hHandle, const MI_RGN_Attr_t *pstRegion);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_Destroy(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_HANDLE hHandle = *((MI_RGN_HANDLE *)ptr);
    //MI_RGN_HANDLE hHandle

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_Destroy(hHandle);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_GetAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_CreatePara_t *pstGetAttr = (MI_RGN_CreatePara_t *)ptr;
    MI_RGN_HANDLE hHandle = pstGetAttr->hHandle;
    MI_RGN_Attr_t *pstRegion = &pstGetAttr->stRegion;
    //MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_GetAttr(hHandle, pstRegion);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_SetBitMap(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_SetBitMapPara_t *pstSetBitMap = (MI_RGN_SetBitMapPara_t *)ptr;
    MI_RGN_HANDLE hHandle = pstSetBitMap->hHandle;
    MI_RGN_Bitmap_t *pstBitMap = &pstSetBitMap->stBitmap;
    //MI_RGN_HANDLE hHandle, const MI_RGN_Bitmap_t *pstBitmap

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_SetBitMap(hHandle, pstBitMap);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_AttachToChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_AttachToChnPara_t *pstAttachToChn = (MI_RGN_AttachToChnPara_t *)ptr;
    MI_RGN_HANDLE hHandle = pstAttachToChn->hHandle;
    MI_RGN_ChnPort_t* pstChnPort = &pstAttachToChn->stChnPort;
    MI_RGN_ChnPortParam_t *pstChnAttr = &pstAttachToChn->stChnAttr;
    //MI_RGN_HANDLE hHandle, const MI_RGN_ChnPort_t* pstChnPort, const MI_RGN_ChnPortParam_t *pstChnAttr

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_AttachToChn(hHandle, pstChnPort, pstChnAttr);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_DetachFromChn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_DetachFromChnPara_t *pstDetechAttr = (MI_RGN_DetachFromChnPara_t *)ptr;
    MI_RGN_HANDLE hHandle = pstDetechAttr->hHandle;
    MI_RGN_ChnPort_t *pstChnPort = &pstDetechAttr->stChnPort;
    //MI_RGN_HANDLE hHandle, const MI_RGN_ChnPort_t *pstChnPort

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_DetachFromChn(hHandle, pstChnPort);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_SetDisplayAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_AttachToChnPara_t *pstSetDispAttr = (MI_RGN_AttachToChnPara_t *)ptr;
    MI_RGN_HANDLE hHandle = pstSetDispAttr->hHandle;
    MI_RGN_ChnPort_t *pstChnPort = &pstSetDispAttr->stChnPort;
    MI_RGN_ChnPortParam_t *pstChnPortPara = &pstSetDispAttr->stChnAttr;
    //MI_RGN_HANDLE hHandle, const MI_RGN_ChnPort_t *pstChnPort, const MI_RGN_ChnPortParam_t *pstChnPortAttr

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_SetDisplayAttr(hHandle, pstChnPort, pstChnPortPara);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_GetDisplayAttr(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_AttachToChnPara_t *pstGetDispAttr = (MI_RGN_AttachToChnPara_t *)ptr;
    MI_RGN_HANDLE hHandle = pstGetDispAttr->hHandle;
    MI_RGN_ChnPort_t *pstChnPort = &pstGetDispAttr->stChnPort;
    MI_RGN_ChnPortParam_t *pstChnPortPara = &pstGetDispAttr->stChnAttr;

    //MI_RGN_HANDLE hHandle, const MI_RGN_ChnPort_t *pstChnPort, MI_RGN_ChnPortParam_t *pstChnPortAttr

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_GetDisplayAttr(hHandle, pstChnPort, pstChnPortPara);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_GetCanvasInfo(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_GetCanvasInfoPara_t *pstGetCanvasInfo = (MI_RGN_GetCanvasInfoPara_t *)ptr;
    MI_RGN_HANDLE hHandle = pstGetCanvasInfo->hHandle;
    MI_RGN_CanvasInfo_t *pstCanvasInfo = &pstGetCanvasInfo->stCanvasInfo;
    //MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t* pstCanvasInfo

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_GetCanvasInfo(hHandle, pstCanvasInfo);

    return s32Ret;
}

static MI_S32 MI_RGN_IOCTL_UpdateCanvas(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_HANDLE hHandle = *((MI_RGN_HANDLE *)ptr);
    //MI_RGN_HANDLE hHandle

    AUTO_LOCK(env, mutex);
    s32Ret = MI_RGN_IMPL_UpdateCanvas(hHandle);

    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_RGN_CMD_MAX] = {
    [E_MI_RGN_CMD_INIT] = MI_RGN_IOCTL_Init,
    [E_MI_RGN_CMD_DE_INIT] = MI_RGN_IOCTL_DeInit,
    [E_MI_RGN_CMD_CREATE] = MI_RGN_IOCTL_Create,
    [E_MI_RGN_CMD_DESTROY] = MI_RGN_IOCTL_Destroy,
    [E_MI_RGN_CMD_GET_ATTR] = MI_RGN_IOCTL_GetAttr,
    [E_MI_RGN_CMD_SET_BIT_MAP] = MI_RGN_IOCTL_SetBitMap,
    [E_MI_RGN_CMD_ATTACH_TO_CHN] = MI_RGN_IOCTL_AttachToChn,
    [E_MI_RGN_CMD_DETACH_FROM_CHN] = MI_RGN_IOCTL_DetachFromChn,
    [E_MI_RGN_CMD_SET_DISPLAY_ATTR] = MI_RGN_IOCTL_SetDisplayAttr,
    [E_MI_RGN_CMD_GET_DISPLAY_ATTR] = MI_RGN_IOCTL_GetDisplayAttr,
    [E_MI_RGN_CMD_GET_CANVAS_INFO] = MI_RGN_IOCTL_GetCanvasInfo,
    [E_MI_RGN_CMD_UPDATE_CANVAS] = MI_RGN_IOCTL_UpdateCanvas,
};


static void mi_rgn_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
}
static void mi_rgn_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
    mi_rgn_Init();
    mi_rgn_drv_init();
}
static void mi_rgn_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
    mi_rgn_ClearRgnBuffer();
    mi_rgn_drv_deinit();
    mi_rgn_Deinit();
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("malloc.peng@mstarsemi.com");

MI_DEVICE_DEFINE(mi_rgn_insmod,
               ioctl_table, E_MI_RGN_CMD_MAX,
               mi_rgn_process_init, mi_rgn_process_exit);
