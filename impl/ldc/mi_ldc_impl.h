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

#ifndef __MI_LDC_IMPL_H__
#define __MI_LDC_IMPL_H__

#include "mi_sys_internal.h"
#include "mi_ldc.h"
//#include "mhal_ldc.h"
#include "mhal_warp.h"


#define MI_LDC_VALID_MODID(a)          (E_MI_MODULE_ID_LDC == (a))        // (E_MI_MODULE_ID_LDC == (a))
#define MI_LDC_VALID_DEVID(a)          ((MI_U32)(a) < MI_LDC_MAX_DEVICE_NUM)
#define MI_LDC_VALID_CHNID(a)          ((MI_U32)(a) < MI_LDC_MAX_CHN_NUM)
#define MI_LDC_VALID_INPUTPORTID(a)    ((MI_U32)(!a))
#define MI_LDC_VALID_OUTPUTPORTID(a)   ((MI_U32)(!a))


typedef enum
{
    E_MI_LDC_INPUTPORT_UNINIT,
    E_MI_LDC_INPUTPORT_INIT,
    E_MI_LDC_INPUTPORT_ENABLED,
    E_MI_LDC_INPUTPORT_DISABLED,
}MI_LDC_InputPort_Status_e;

typedef enum{
    E_MI_LDC_CHN_UNINIT,
    E_MI_LDC_CHN_INIT,
    E_MI_LDC_CHN_ENABLED,
    E_MI_LDC_CHN_DISABLED,
}MI_LDC_Channel_Status_e;

typedef enum
{
    E_MI_LDC_DEVICE_UNINIT,
    E_MI_LDC_DEVICE_INIT,
    E_MI_LDC_DEVICE_START,
    E_MI_LDC_DEVICE_STOP,
}MI_LDC_Device_Status_e;

typedef enum
{
    E_MI_LDC_WORKER_WAKE_VSYNC=0,
    E_MI_LDC_WORKER_PAUSE,
    E_MI_LDC_WORKER_PAUSED,
}MI_LDC_Worker_Wake_Event_e;


typedef struct MI_LDC_Inputport_s
{
    MI_SYS_ChnPort_t stInputPort;
    MI_BOOL bBind;
    MI_SYS_ChnPort_t stBindPort;
    MI_U64 u64Try;
    MI_U64 u64RecvOk;
    MI_SYS_BufInfo_t *pstInBufInfo;
}MI_LDC_Inputport_t;

typedef struct MI_LDC_Outputport_s
{
    MI_BOOL bInited;
    MI_SYS_ChnPort_t stOutputPort;
    MI_BOOL bBind;                  //is bound
    MI_SYS_ChnPort_t stBindPort;
    MI_U64 u64Interval;
    MI_U64 u64SendOk;
    MI_SYS_BufInfo_t *pstOutBufInfo;
}MI_LDC_Outputport_t;

typedef struct MI_LDC_BufQueue_s
{
    struct list_head bufList;
    MI_U32 u32BufCnt;
    struct semaphore semQueue;
}MI_LDC_BufQueue_t;

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
typedef struct MI_LDC_CalcSpendTime_s
{
    struct timespec stStartTmSpec;
    MI_U64 u64TotalTime;
    MI_U64 u64MaxSpendTime;
    MI_U64 u64MinSpendTime;
    MI_U64 u64CurSpendTime;
}MI_LDC_CalcSpendTime_t;

typedef struct
{
    MI_U32 u32GetInputCnt;
    MI_U32 u32FinishInputCnt;
    MI_U32 u32RewindInputCnt;
    MI_U32 u32GetOutputCnt;
    MI_U32 u32FinishOutputCnt;
    MI_U32 u32RewindOutputCnt;
    MI_U32 u32TryTriggerCnt;
    MI_U32 u32FinishTriggerCnt;
}MI_LDC_BufOptCnt_t;

typedef struct MI_LDC_CalcFps_s
{
    MI_U32 u32SampleCnt;
    MI_U32 u32StatDepth;
    MI_U32 u32CurIndex;
    MI_U64 u64PrePts;
    MI_U64 u64LastPts;
    MI_U64 *pu64StatSet;
}MI_LDC_CalcFps_t;

typedef struct MI_LDC_StatPortInfo_s
{
    MI_SYS_PixelFormat_e eFormat;
    MI_U16 u16Width;
    MI_U16 u16Height;
}MI_LDC_StatPortInfo_t;

typedef struct
{
    MI_LDC_CalcSpendTime_t stCalcHalTime;
    MI_LDC_BufOptCnt_t stBufCnt;
    MI_LDC_CalcFps_t stCalcInputFps;
    MI_LDC_CalcFps_t stCalcOutputFps;
    MI_LDC_StatPortInfo_t stInputPortInfo;
    MI_LDC_StatPortInfo_t stOutputPortInfo;
}MI_LDC_DebugInfo_t;
#endif

typedef struct MI_LDC_Channel_s
{
    MI_LDC_Channel_Status_e eChnStatus;
    MI_U32 u32DevId;
    MI_U32 u32ChnId;
    MHAL_WARP_INST_HANDLE hInstHandle;
    MI_LDC_Inputport_t stInputPort;
    MI_LDC_Outputport_t stOutputPort;
    MHAL_WARP_CONFIG stWarpConfig;
    struct task_struct *stWorkThread;
    struct completion stChnCompletion;
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
    MI_LDC_DebugInfo_t stDebugInfo;
#endif
}MI_LDC_Channel_t;

typedef struct MI_LDC_Device_s
{
    MI_U32 u32DeviceId;
    MI_LDC_Channel_t stChannel[MI_LDC_MAX_CHN_NUM];   // MI_LDC_INSTANCE;
    MI_LDC_Device_Status_e eStatus;
    MI_SYS_DRV_HANDLE hDevHandle;
    MHAL_WARP_DEV_HANDLE hMhalHandle;
    struct mutex mtx;
}MI_LDC_Device_t;

typedef struct MI_LDC_Module_s
{
    MI_BOOL bInited;
    MI_LDC_Device_t stDev[MI_LDC_MAX_DEVICE_NUM]; // MI_LDC_HANDLE;
}MI_LDC_Module_t;


MI_S32 MI_LDC_IMPL_Init(void);
MI_S32 MI_LDC_IMPL_DeInit(void);
MI_S32 MI_LDC_IMPL_CreateDevice(MI_LDC_DEV devId);
MI_S32 MI_LDC_IMPL_DestroyDevice(MI_LDC_DEV devId);
MI_S32 MI_LDC_IMPL_StartDev(MI_LDC_DEV devId);
MI_S32 MI_LDC_IMPL_StopDev(MI_LDC_DEV devId);

MI_S32 MI_LDC_IMPL_CreateChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);
MI_S32 MI_LDC_IMPL_DestroyChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);

MI_S32 MI_LDC_IMPL_EnableChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);
MI_S32 MI_LDC_IMPL_DisableChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);
MI_S32 MI_LDC_IMPL_EnableInputPort(MI_LDC_DEV devId, MI_LDC_CHN chnId);
MI_S32 MI_LDC_IMPL_DisableInputPort(MI_LDC_DEV devId, MI_LDC_CHN chnId);
MI_S32 MI_LDC_IMPL_EnableOutputPort(MI_LDC_DEV devId, MI_LDC_CHN chnId);
MI_S32 MI_LDC_IMPL_DisableOutputPort(MI_LDC_DEV devId, MI_LDC_CHN chnId);

MI_S32 MI_LDC_IMPL_SetTable(MI_LDC_DEV devId, MI_LDC_CHN chnId, void *pTableAddr, MI_U32 u32TableSize);



#endif //_MI_VPE_IMPL_H_
