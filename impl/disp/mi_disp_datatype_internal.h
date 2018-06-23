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
#ifndef _MI_DISP_INTERNAL_INTERNAL_H_
#define _MI_DISP_INTERNAL_INTERNAL_H_
#include "mi_disp.h"
#include "mi_sys_datatype.h"

// TODO: MAX input port should define in mhal
#define MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX 16
#define  MI_DISP_VIDEO_LAYER_MAX 2
// TODO: Shoke need check IRQ number
#define DISP_DEV0_ISR_IDX (10)
#define DISP_DEV1_ISR_IDX (11)
//#define trace() DBG_ERR("%s()@line %d!!!\n", __func__, __LINE__)
#define trace()

#define MI_DISP_DOWN(x) down((x))
#define MI_DISP_UP(x)   up((x))


typedef enum
{
   MI_OK = 0x0,                   ///< succeeded
   MI_FAIL,                  ///< Error
}MI_RESULT;

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
typedef struct stDispCheckFramePts_s
{
    MI_U64 u64Pts;
    MI_BOOL bCheckFramePts;
    MI_U32 u32CheckFramePtsBufCnt;
}stDispCheckFramePts_t;

typedef struct stDispCheckFrameRate_s
{
    MI_BOOL bstat;
    MI_BOOL bFirstSet;
    MI_U32 u32GetBufCnt;
    MI_U32 u32FlipBufCnt;
    MI_U64 u64LastInputFrameTime;
    MI_U64 u64CurrentInputFrameTime;
    MI_U64 u64InterValTimeMAX[3];
    MI_U64 u64InterValTimeMIN[3];
    MI_U64 u64InterValTimeSum;
}stDispCheckFrameRate_t;

typedef struct stDispCheckBuffInfo_s
{
    MI_U32 u32InportRecvBufCnt;
    MI_U32 u32RecvBufWidth;
    MI_U32 u32RecvBufHeight;
    MI_U32 u32RecvBufStride;
}stDispCheckBuffInfo_t;

typedef struct stDispCheckIsr_s
{
    MI_BOOL bCheckIsr;
    MI_U32 u32IsrCnt;
}stDispCheckIsr_t;

typedef struct stDispCheckFrameRateTimer_s
{
    struct timer_list Timer;
    MI_BOOL bCreate;
}stDispCheckFrameRateTimer_t;
#endif

typedef struct mi_disp_PortPendingBuf_s
{
    MI_SYS_BufInfo_t* pstInputBuffer;
    struct list_head stPortPendingBufNode;
    MI_U64 u64GotTime;
    MI_U64 u64ReleaseTime;
}mi_disp_PortPendingBuf_t;

typedef struct mi_disp_InputPortStatus_s
{
    MI_DISP_VidWinRect_t stDispWin;
    MI_DISP_VidWinRect_t stCropWin;
    MI_U16 u16Fence;
    MI_BOOL bGeFlip;
    MI_BOOL bPause;
    MI_BOOL bEnable;
    MI_BOOL bClrAllBuff;
    MI_DISP_SyncMode_e eMode;
    MI_DISP_SyncMode_e eUserSetSyncMode;
    MI_DISP_InputPortStatus_e eStatus;

    //Check PTS
    MI_U64 u64LastFiredTimeStamp;
    MI_U64 u64LastFramePts;
    MI_U64 u64RecvCurPts;
    MI_U64 u64NextPts;
    MI_U64 u64FiredDiff;
    MI_U64 u64PtsResetCnt;
    MI_BOOL bFramePtsBefore;
    //when input port is enabled by APP, don't call MHAL_DISP_InputPortEnable immediately and set this flay only.
    //Call MHAL_DISP_InputPortEnable when flip the first frame of this port and then reset this flag.
    MI_BOOL bFirstFrame;
    //disable input port in MI_DISP_IMPL_SetVideoLayerAttrBegin, then enable input port in MI_DISP_IMPL_SetVideoLayerAttrEnd.
    MI_BOOL bDisabledInside;
    //Hal layer instance
    void* apInputObjs;

    MI_U32            u32FrameCntpers;
    MI_U32            u32FrameCntJiffies;
    MI_U8             u8PortFps;
    //struct semaphore stPortMutex;
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
    MI_BOOL bDumpFrame;
    MI_BOOL bStopGetInBuf;
    stDispCheckFramePts_t stDispCheckFramePts;
    stDispCheckFrameRate_t stDispCheckFrameRate;
    stDispCheckBuffInfo_t stDispCheckBuffInfo;
#endif
}mi_disp_InputPortStatus_t;

typedef struct mi_disp_LayerStatus_s
{
   //Video Layer properities
   MI_U8 u8LayerID;
   MI_BOOL  bLayerEnabled;
   MI_DISP_VideoLayerAttr_t stVideoLayerAttr;
   MI_BOOL bCompress;
   MI_U32 u32Priority;
   MI_U32 u32Toleration;
   MI_U8 u8BindedDevID;

   //Input port status in video Layer
   mi_disp_InputPortStatus_t astPortStatus[MI_DISP_INPUTPORT_MAX];

   //Input port pending buffer
   struct semaphore stDispLayerPendingQueueMutex;
   struct list_head stPortPendingBufQueue[MI_DISP_INPUTPORT_MAX];

   MI_SYS_BufInfo_t *pstOnScreenBufInfo[MI_DISP_INPUTPORT_MAX];
   MI_SYS_BufInfo_t *pstCurrentFiredBufInfo[MI_DISP_INPUTPORT_MAX];

   //Hal layer instance
   void* apLayerObjs;

   //Layer List node
   struct list_head stLayerNode;
   //struct semaphore stLayerMutex;
   MI_BOOL bInited;
   MI_BOOL bLayerCreated;
}mi_disp_LayerStatus_t;

#define MI_DISP_OUTDEVICE_MAX 10
typedef struct MI_DISP_DEV_Status_s
{
    MI_BOOL bDISPEnabled;
    MI_BOOL bIrqEnabled;
    atomic_t disp_isr_waitqueue_red_flag;
    wait_queue_head_t stWaitQueueHead;
    MI_U32 u32DevId;
    MI_U32 u32DevIrq;
    MI_SYS_DRV_HANDLE hDevSysHandle;

    //Binded layer list, Use List for extented
    struct list_head stBindedLayer;
    MI_U32 u32SrcW; //layer width
    MI_U32 u32SrcH; //layer height

    MI_U32 u32BgColor;
    //MI_DISP_Interface_e eIntfType;
    MI_U32 u32Interface;
    MI_DISP_OutputTiming_e eDeviceTiming[E_MI_DISP_INTF_MAX + 1];
    MI_DISP_SyncInfo_t       stSyncInfo;          /* Information about VO interface timings */

    //Hdmi or Vga
    MI_DISP_CscMattrix_e eCscMatrix;
    MI_U32 u32Luma;                     /* luminance:   0 ~ 100 default: 50 */
    MI_U32 u32Contrast;                 /* contrast :   0 ~ 100 default: 50 */
    MI_U32 u32Hue;                      /* hue      :   0 ~ 100 default: 50 */
    MI_U32 u32Saturation;               /* saturation:  0 ~ 100 default: 50 */

    //vga only
    MI_U32 u32Gain;                          /* current gain of VGA signals. [0, 64). default:0x30 */
    MI_U32 u32Sharpness;                /* For VGA signals*/

    //cvbs
    MI_BOOL bCvbsEnable;

    // Hal Dev instance
    void* pstDevObj;

    //Dev List node
    struct list_head stDevNode;

    //Dev Task
    struct task_struct* pstWorkTask;
    struct task_struct* pstIsrTask;

    //Check pts
    MI_U64 u64LastIntTimeStamp;
    MI_U64 u64CurrentIntTimeStamp;
    MI_U32 u32AccumInterruptCnt;//debug purpose, for measure real interrupt interval
    MI_U32 u32AccumInterruptTimeStamp;//debug purpose, for measure real interrupt interval

    MI_U32 u32VsyncInterval;

    MI_BOOL bSupportIrq;

    MI_U32            u32DevFrameCntpers;
    MI_U32            u32DevFrameCntJiffies;
    MI_U16             u16devFps;
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
    MI_BOOL bProcDisableIrq;
    stDispCheckIsr_t stDispCheckIsr;
#endif
   //struct semaphore stDevMutex;
}mi_disp_DevStatus_t;


#endif /* _MI_VDEC_INTERNAL_H_ */
