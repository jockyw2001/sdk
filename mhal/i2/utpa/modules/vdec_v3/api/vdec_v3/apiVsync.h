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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   apiVDEC_EX.h
/// @brief  VDEC EXTENSION API FOR DUAL STREAMS
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _API_VSYNC_H_
#define _API_VSYNC_H_


#include "UFO.h"
#include "apiVDEC_EX.h"
#include "apiDMS_Type.h"
#include "apiDMX.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if (!defined(MSOS_TYPE_NUTTX) && !defined(MSOS_TYPE_OPTEE)) || defined(SUPPORT_X_MODEL_FEATURE)

#ifndef ANDROID
#define VSYNC_PRINT printf
#else
#include <sys/mman.h>
#include <cutils/ashmem.h>
#include <cutils/log.h>
#define VSYNC_PRINT ALOGD
#endif

#ifdef MSOS_TYPE_LINUX_KERNEL
#if defined(CHIP_M5621) || defined(CHIP_M7621)
#define VSYNC_USE_DMS
#endif
#endif

#if defined(CHIP_K7U) || defined(CHIP_C2P) || defined(CONFIG_API_DMS)
#define VSYNC_USE_DMS
#endif

#if defined(MSOS_TYPE_LINUX_KERNEL)
#define __vdec_weak
#else
#define __vdec_weak      __attribute__((weak))
#endif

#define __vdec_if            extern

__vdec_if EN_DMS_RESULT MApi_DMS_Init(ST_DMS_INITDATA *pstDMS_InitData);
__vdec_if EN_DMS_RESULT MApi_DMS_Video_Flip(MS_U32 u32WindowID, ST_DMS_DISPFRAMEFORMAT* pstDispFrameFormat);
__vdec_if EN_DMS_RESULT MApi_DMS_DestroyWindow(MS_U32 u32WindowID);
__vdec_if EN_DMS_RESULT MApi_DMS_ClearDigitalDecodeSignalInfo(MS_U32 u32WindowID);
__vdec_if EN_DMS_RESULT MApi_DMS_SetDigitalDecodeSignalInfo(MS_U32 u32WindowID, ST_DMS_DISPFRAMEFORMAT *pstDispFrameFormat);
__vdec_if EN_DMS_RESULT MApi_DMS_GetCapability(EN_DMS_CAPABILITY *peCapability);
__vdec_if DMX_FILTER_STATUS MApi_DMX_Stc_Get(MS_U32* pu32Stc32, MS_U32* pu32Stc);


#define VSYNC_MAX_DISP_PATH 33
#define AVSYNC_REPEAT_THRESHOLD         1
#define AVSYNC_REPEAT_ALWAYS            0xff
#define VSYNC_INVALID_WIN_ID 0xFFFFFFFF

/// function return enumerator
typedef enum
{
    ///failed
    E_VSYNC_FAIL = 0,
    ///success
    E_VSYNC_OK,
    ///invalid parameter
    E_VSYNC_RET_INVALID_PARAM,
    ///access not allow
    E_VSYNC_RET_ILLEGAL_ACCESS,
     ///unsupported
    E_VSYNC_RET_UNSUPPORTED,
     ///timeout
    E_VSYNC_RET_TIMEOUT,
    ///not ready
    E_VSYNC_RET_NOT_READY,
    ///not initial
    E_VSYNC_RET_NOT_INIT,
    ///not exit after last initialization
    E_VSYNC_RET_NOT_EXIT,
    ///not running, counter does not change
    E_VSYNC_RET_NOT_RUNNING,
    ///max value
    E_VSYNC_RET_NUM,
} VSYNC_Result;

typedef enum
{
    ///stop
    E_VSYNC_STOP = 0,
    ///play
    E_VSYNC_PLAY,
    ///flush
    E_VSYNC_FLUSH,
    ///pause
    E_VSYNC_PAUSE,
    ///smooth trick (x2, slow motion)
    E_VSYNC_FREEZE,

    E_VSYNC_STEP_DISP,
    ///max value
    E_VSYNC_CMD_NUM,
} VSYNC_Vdec_Status;

typedef enum
{
    E_VSYNC_STC_MODE = 0,
    E_VSYNC_AUDIO_MASTER_MODE,
    E_VSYNC_VIDEO_MASTER_MODE,
    E_VSYNC_MAX_MODE,
} VSYNC_SyncMode;

typedef enum
{
    E_VSYNC_USER_CMD_SET_CONTROL_BASE = 0x0,
    E_VSYNC_USER_CMD_SET_STATUS,
    E_VSYNC_USER_CMD_SET_3D_MODE,
    E_VSYNC_USER_CMD_SET_3DLAYOUTUNLOCKED,
    E_VSYNC_USER_CMD_SET_LOW_LATENCY,
    E_VSYNC_USER_CMD_SET_SLOW_SYNC,
    E_VSYNC_USER_CMD_SET_AVSYNC_FREERUN_THRESHOLD,
    E_VSYNC_USER_CMD_SET_DISP_ONE_FIELD,
    E_VSYNC_USER_CMD_SET_FREEZE_DISP,
    E_VSYNC_USER_CMD_SET_MVOP_MCU_MODE,// may use by vsync bridge
    E_VSYNC_USER_CMD_SET_SHOW_FIRST_FRAME_DIRECT,
    E_VSYNC_USER_CMD_SET_TRICKPLAY_2X_MODE,
    E_VSYNC_USER_CMD_SET_DISP_WIN_ID,
    E_VSYNC_USER_CMD_PRESET_STC,
    E_VSYNC_USER_CMD_FD_MASK_DELAY_COUNT,
    E_VSYNC_USER_CMD_FRC_ONLY_SHOW_TOP_FIELD,
    E_VSYNC_USER_CMD_AVC_DISP_IGNORE_CROP,
    E_VSYNC_USER_CMD_AVSYNC_REPEAT_TH,
    E_VSYNC_USER_CMD_SET_BLUE_SCREEN,
    E_VSYNC_USER_CMD_DISP_ONE_FIELD,
    E_VSYNC_USER_CMD_STEP_DISP,
    E_VSYNC_USER_CMD_ENABLE_LAST_FRAME_SHOW,// how do I know we meet file end ?
    E_VSYNC_USER_CMD_SET_MULTI_SPEED_ENCODE,
    E_VSYNC_USER_CMD_DBG_IDX,
    E_VSYNC_USER_CMD_SET_SELF_SEQCHANGE,
    E_VSYNC_USER_CMD_SUSPEND_DYNAMIC_SCALE,
    //=====================================
    E_VSYNC_USER_CMD_GET_CONTROL_BASE = 0x1000,
    E_VSYNC_USER_CMD_GET_IsAVSyncOn,
    E_VSYNC_USER_CMD_GET_IS_REACH_SYNC,
    E_VSYNC_USER_CMD_GET_DROP_CNT,
    E_VSYNC_USER_CMD_GET_DISP_CNT,
    E_VSYNC_USER_CMD_GET_PTS,
    E_VSYNC_USER_CMD_GET_NEXT_PTS,
    E_VSYNC_USER_CMD_GET_U64PTS,
    E_VSYNC_USER_CMD_GET_IS_FRAME_RDY,
    E_VSYNC_USER_CMD_GET_IS_STEP_DISP_DONE,
    E_VSYNC_USER_CMD_IS_DISP_FINISH, // Huston, we got a problem !!!
    E_VSYNC_USER_CMD_GET_VIDEO_PTS_STC_DELTA,
    E_VSYNC_USER_CMD_IS_FRAME_RDY,
    E_VSYNC_USER_CMD_IS_DISPQ_EMPTY,
    E_VSYNC_USER_CMD_GET_DISP_INFO,
    E_VSYNC_USER_CMD_CHECK_DISPINFO_READY,
    //=====================================
    E_VSYNC_USER_CMD_MAX,
}VSYNC_User_Cmd;


typedef EN_DMS_RESULT (*VsyncDispConnectCb)(MS_U32 u32WindowID, ST_DMS_DISPFRAMEFORMAT* pstDispFrameFormat);
typedef EN_DMS_RESULT (*VsyncDispDisconnectCb)(MS_U32 u32WindowID);
typedef EN_DMS_RESULT (*VsyncDispFlipCb)(MS_U32 u32WindowID, ST_DMS_DISPFRAMEFORMAT* pstDispFrameFormat);
typedef EN_DMS_RESULT (*VsyncDispGetStatusCb)(EN_DMS_STATUS *peStatus);
typedef void (*VDEC_EX_FireUserCallbackFunc)(VDEC_StreamId *pStreamId, MS_U32 u32EventFlag);


//void MApi_VSync_Dbg_test(void);
//void MApi_VSync_Dbg_selftest(VDEC_StreamId *pStreamId, VDEC_EX_InitParam *pInitParam);
//void MApi_VSync_Dbg_stop_selftest(VDEC_StreamId *pStreamId);

VSYNC_Result MApi_VSync_Init(int VsynDispId, VDEC_StreamId *pStreamId, VDEC_EX_InitParam *pInitParam, MS_U32 eWinID, MS_U32 u32OverlayId);
VSYNC_Result MApi_VSync_Deinit(int VsynDispId);
VDEC_EX_Result MApi_VSync_SetControl(int VsynDispId, VSYNC_User_Cmd cmd_id, void* param);
VDEC_EX_Result MApi_VSync_GetControl(int VsynDispId, VSYNC_User_Cmd cmd_id, void* param);
VSYNC_Result MApi_VSync_AVSyncOn(int VsynDispId, MS_BOOL bOn, MS_U32 u32SyncDelay, MS_U16 u16SyncTolerance);
VSYNC_Result MApi_VSync_SetSpeed(int VsynDispId, VDEC_EX_SpeedType eSpeedType, VDEC_EX_DispSpeed eSpeed);
VSYNC_Result MApi_VSync_IsInited(int VsynDispId);
VSYNC_Result MApi_VSync_SetCallbackFunc(int VsynDispId, VsyncDispConnectCb VsyncDispConnect, VsyncDispDisconnectCb VsyncDispDisconnect, VsyncDispFlipCb VsyncDispFlip);
VSYNC_Result MApi_VSync_SetVdecCBFunc(int VsynDispId, VDEC_EX_FireUserCallbackFunc VDEC_EX_FireUserCBFunc);

#endif

#ifdef __cplusplus
}
#endif


#endif // _API_VSYNC_H
