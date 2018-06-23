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

#ifndef __MI_AO_DATATYPE_INTERNAL_H__
#define __MI_AO_DATATYPE_INTERNAL_H__

#include "mi_sys_internal.h"

#include <linux/timer.h>

//=============================================================================
// Include files
//=============================================================================


//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================
#if USE_CAM_OS
    #define MI_AO_Thread_t CamOsThread
#else
    typedef struct task_struct* MI_AO_Thread_t;
#endif

#define MI_AO_START_WRTIE_THRESHOLD_MS   150

#define MI_AO_PCM_BUF_SIZE_BYTE      (MI_AUDIO_MAX_SAMPLES_PER_FRAME * MI_AUDIO_MAX_FRAME_NUM * 2 * 4)
#define MI_AO_WRITE_PEROID_SAMPLES     128

//=============================================================================
// Data type definition
//=============================================================================

typedef struct _MI_AO_IMPL_QueueInfo_s{
    MI_S32 s32Max;//Max len of queue
    MI_S32 s32Size; //current size
    MI_U64 u64RunTime;  //running tims ms
    MI_U64 u64GetTotalSize;
}_MI_AO_IMPL_QueueInfo_t;

typedef struct _MI_AO_ChanInfo_s
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
    MI_BOOL                 bAdecEnable;
    MI_BOOL                 bAdecAttrSet;
    MI_AO_VqeConfig_t       stAoVqeConfig;
    MI_AO_AdecConfig_t      stAoAdecConfig;
    MI_U32                  u32WriteTotalFrmCnt;
    MI_U64                  u64StartTime;
    MI_U64                  u64GetTotalSize;
    _MI_AO_IMPL_QueueInfo_t stUsrQueueInfo;
}_MI_AO_ChanInfo_t;


typedef struct _MI_AO_DevInfo_s
{
    MI_BOOL                 bDevEnable;
    MI_BOOL                 bDevAttrSet;
    MI_AUDIO_DEV            AoDevId;
    MI_AUDIO_Attr_t         stDevAttr;
    _MI_AO_ChanInfo_t        astChanInfo[MI_AO_CHAN_NUM_MAX];
    MI_S32                  s32VolumeDb;
    MI_BOOL                 bMuteEnable;
    MI_U64                    u64PhyBufAddr;          // for DMA HW address
    void*                       pVirBufAddr;                    // for DMA SW address
    MI_AO_Thread_t    pstAoWriteDataThread;    // Kernel thread for _MI_AO_WriteDataThread
    MI_SYS_DRV_HANDLE       hDevSysHandle;

}_MI_AO_DevInfo_t;

typedef struct _MI_AO_TimerInfo_s
{
    struct timer_list       stTimer;    //kernel timer structer
    MI_U32                  u32CurrentSize;
    MI_U32                  u32OldSize;
    MI_BOOL                 bTimerEnable;
}_MI_AO_TimerInfo_t;

typedef struct _MI_AO_DumpPcmInfo_s
{
    MI_U32       u32Count;
    MI_U32       u32MaxCount;
    MI_U32       u32TotalSize;
    MI_BOOL      bDumpEnable;
    struct file  *pstFile;
}_MI_AO_DumpPcmInfo_t;

//=============================================================================
// Variable definition
//=============================================================================


//=============================================================================
// Global function definition
//=============================================================================


#endif // __MI_AO_DATATYPE_INTERNAL_H__
