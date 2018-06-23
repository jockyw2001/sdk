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

#ifndef _MI_VPE_IMPL_INTERNAL_H_
#define _MI_VPE_IMPL_INTERNAL_H_
#include <linux/wait.h>
#include <linux/interrupt.h>

//#include "HAL_VPE.h"
#include "mhal_vpe.h"

#include "mi_vpe.h"
#include "mi_sys_internal.h"
#include "mhal_cmdq.h"
#define MI_VPE_TASK_3DNR_UPDATE              (1ul << 0)
#define MI_VPE_TASK_ROI_UPDATE               (1ul << 1)
#define MI_VPE_TASK_NO_DRAM_OUTPUT           (1ul << 2)

#define __MI_VPE_DEV_MAGIC__      (0x56504544)
#define __MI_VPE_CHN_MAGIC__      (0x56504543)
#define __MI_VPE_OUTPORT_MAGIC__  (0x5650454f)
#define __MI_VPE_ROI_MAGIC__      (0x56504552)

#define SUPPORT_ISP (E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE|E_MI_VPE_RUNNING_MODE_FRAMEBUF_CAM_MODE|E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
#define SUPPORT_SCL (E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE|E_MI_VPE_RUNNING_MODE_FRAMEBUF_CAM_MODE|E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
#define SUPPORT_IQ  (E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE)

#define SUPPORT_DNR_ROI (E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE)
#define SUPPORT_SWCROP (E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE)
#define SUPPORT_HWCROP (SUPPORT_SCL)
#define SUPPORT_ROATION (0)

#define SUPPORT_RGN (E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE|E_MI_VPE_RUNNING_MODE_FRAMEBUF_CAM_MODE|E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
typedef struct MI_VPE_OutPortInfo_s {
    MI_BOOL           bEnable;
    MI_VPE_PortMode_t stPortMode;
    MI_SYS_ChnPort_t  stPeerOutputPortInfo; // Port binder to: Just for debug
    #if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
    MI_U32            u32OutbufStride;
    MI_U64            u64GetOutputBufferCnt;
    MI_U64            u64GetOutputBuffFailCnt;             // VPE get ouput buffer fail counter.
    MI_U64            u64FinishOutputBufferCnt;
    MI_U64            u64BufToHalCnt;

    MI_U32            u32FrameCntpers;
    MI_U32            u32FrameCntJiffies;
    MI_U8             u8PortFps;
    #endif
    MI_SYS_PixelFormat_e  eRealOutputPixelFormat;  // Pixel format of target image
    MI_U64            u64SideBandMsg;
} mi_vpe_OutPortInfo_t;

typedef enum
{
    E_MI_VPE_CHANNEL_STATUS_INITED = 0,
    E_MI_VPE_CHANNEL_STATUS_START,
    E_MI_VPE_CHANNEL_STATUS_STOP,
    E_MI_VPE_CHANNEL_STATUS_DESTROYED,
    E_MI_VPE_CHANNEL_STATUS_NUM,
} mi_vpe_ChannelStatus_e;


typedef enum
{
    E_MI_VPE_3DNR_STATUS_INVALID = 0,
    E_MI_VPE_3DNR_STATUS_IDLE,
    E_MI_VPE_3DNR_STATUS_NEED_UPDATE,
    E_MI_VPE_3DNR_STATUS_RUNNING,
    E_MI_VPE_3DNR_STATUS_UPDATED,
    E_MI_VPE_3DNR_STATUS_NUM,
} mi_vpe_3DNrStatus_e;


typedef struct {
    mi_vpe_3DNrStatus_e eStatus;
    MI_VPE_CHANNEL VpeCh;
    MI_U32  u32WaitScriptNum;
} mi_vpe_3DNRUpdateStatus_t;

typedef struct {
    MI_U16 u16Width;
    MI_U16 u16Height;
    MI_SYS_CompressMode_e    eCompressMode;                      // Channel Input Compress mode
} mi_vpe_SourceVideoInfo_t;

typedef struct {
    MI_SYS_PixelFormat_e ePixelFormat;
    MI_SYS_CompressMode_e eCompressMode;
    MI_U16 u16Width;
    MI_U16 u16Height;
    MS_U64 u64MetaInfo;

    void* pVirAddr[3];
    MI_PHY phyAddr[3];//notice that this is miu bus addr,not cpu bus addr.
    MI_U32 u32Stride[3];
} mi_vpe_InBuffInfo_t;

#define EMPTY 0
#define OCCUPY 1

typedef struct {
    MI_BOOL bStatus;
    MI_U16  u16DropFence;
}mi_vpe_DropFenceInfo_t;

typedef enum{
    E_MI_VPE_SCL_NULL_PORTMODE =0,
    E_MI_VPE_SCL_3_PORTMODE,
    E_MI_VPE_SCL_4_PORTMODE,
    E_MI_VPE_SCL_NUM_PORTMODE,
}mi_vpe_SclPortMode_e;

typedef struct mi_vpe_ChannelInfo_s {
    struct list_head         list;                               // Device active list
    MI_BOOL                  bCreated;                           // Channel: Created
    void                     *pIspCtx;                           // HAL layer: ISP context pointer
    void                     *pIqCtx;                            // HAL layer: IQ context pointer
    void                     *pSclCtx;                           // HAL layer: SCL context pointer
    MI_VPE_ChannelAttr_t     stChnnAttr;                         // Channel attribution
    MI_VPE_ChannelPara_t     stChnnPara;                         // Channel parameter
    mi_vpe_ChannelStatus_e   eStatus;                            // Channel status
    MI_SYS_CompressMode_e    eCompressMode;                      // Channel Input Compress mode
    mi_vpe_SourceVideoInfo_t stSrcWin;                           // Chanel input source size
    MI_SYS_WindowRect_t      stCropWin;                          // Channel user setting crop window
    MI_SYS_WindowRect_t      stRealCrop;                         // Channel real crop window
    MI_SYS_Rotate_e          eRotationType;                      // Channel rotation type
    MI_SYS_Rotate_e          eRealRotationType;                  // Channel real rotation type
    MI_SYS_ChnPort_t         stPeerInputPortInfo;                // Channel binder to: Just for debug
    mi_vpe_OutPortInfo_t     stOutPortInfo[MI_VPE_MAX_PORT_NUM]; // Information of output
    MI_PHY                   u64PhyAddrOffset[3];                // Channel address offset for sw crop window.
    MI_VPE_CHANNEL           VpeCh;
    MI_VPE_HDRType_e         eHDRType;
    struct semaphore         stChnnMutex;
    //parameters for sideband crop for saving bw
    MI_U32  x_zoom_in_ratio;
    MI_U32  y_zoom_in_ratio;
    MI_U32  w_zoom_in_ratio;
    MI_U32  h_zoom_in_ratio;
    atomic_t                 stAtomTask;
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
    MI_U64                  u64GetToReleaseMaxTime;
    MI_U64                  u64GetToReleaseSumTime;
    MI_U64                  u64GetToKickOffSumTime;
    MI_U64                  u64KickOffToReleaseSumTime;
    MI_U32                  u32KOToReleaseMeanTime;

    MI_U64                  u64ReleaseBufcnt;
    MI_U64                  u64GetInputBufferCnt;
    MI_U64                  u64InBuffStride;
    MI_U64                  u64GetInputBufferTodoCnt;

    MI_BOOL                 bCheckFramePts;
    MI_U16                  u16DumpTaskFileNum;

    MHalSclOutputMode_e eSclOutputMode;
    MHalSclInputPortMode_e eSclInPutMode;
#endif
} mi_vpe_ChannelInfo_t;

typedef enum {
    E_MI_VPE_ROI_STATUS_INVALID = 0,
    E_MI_VPE_ROI_STATUS_IDLE,        //---> free
    E_MI_VPE_ROI_STATUS_NEED_UPDATE, // user call ROI
    E_MI_VPE_ROI_STATUS_RUNNING,     // ROI in cmdQ
    E_MI_VPE_ROI_STATUS_UPDATED,     // ISR ROI finish
    E_MI_VPE_ROI_STATUS_NUM,
} mi_vpe_RoiStatus_e;

typedef struct {
    mi_vpe_RoiStatus_e  eRoiStatus;
    MI_VPE_CHANNEL      VpeCh;
    MI_VPE_RegionInfo_t stRegion; // user region
} mi_vpe_RoiInfo_t;

typedef struct {
    MI_U32                    u32MagicNumber;
    mi_vpe_ChannelInfo_t      stChnnInfo[MI_VPE_MAX_CHANNEL_NUM];// All channel information
    MI_SYS_DRV_HANDLE         hDevSysHandle;                     // Handle for MI_SYS device
    MI_BOOL                   bInited;                           // Weather MI_VPE already inited.
    MHAL_CMDQ_CmdqInterface_t       *pstCmdMloadInfo;                  // Command Queue/Menuload interface
    unsigned int              uVpeIrqNum;                        // VPE irq number
    struct task_struct        *pstWorkThread;                    // Kernel thread for VPEWorkThread
    struct task_struct        *pstProcThread;                    // Kernel thread for VPE IRQ bottom
    MI_U32                    u32ChannelCreatedNum;              // Total channel number for user created
    MHalVpeGetCap_t           stCaps;
    MI_U64                    u64FramePts;
    MI_U16                    u16RTCmdqFence;
    mi_vpe_DropFenceInfo_t    stDropFrameFence[MI_VPE_MAX_WORKINGLIST_NODE];
    MI_VPE_RunningMode_e      eRunningMode;
    mi_vpe_SclPortMode_e      eSclPortMode;
    wait_queue_head_t         inputWaitqueue;
//    mi_vpe_RoiInfo_t          stRoiInfo;          // Roi information
    mi_vpe_RoiStatus_e        eRoiStatus;
    mi_vpe_3DNRUpdateStatus_t st3DNRUpdate;                      // 3DNR update status
    MI_U64                    u64TotalCreatedArea;               // Created channel total area
    MI_U32                    u32TaskNoToDramCnt;
    MI_BOOL                   bEnbaleIrq;                        // Irq enable
    MI_BOOL                   bSupportIrq;                        // Support Irq
    MHalVpeIrqEnableMode_e    eIrqEnableMode;
    MI_BOOL                   bCameraUse;
#ifdef MI_SYS_PROC_FS_DEBUG
    MI_BOOL                   bProcDisableIrq;                   // User Debug Disable IRQ via proc.
    MI_BOOL                   bProcDisableCmdq;                  // User Debug Disable CMDQ via proc.
    struct task_struct        *pstCmdqbusy;
#endif
} mi_vpe_DevInfo_t;

typedef struct
{
   int totalAddedTask;
} mi_vpe_IteratorWorkInfo_t;


typedef struct ROI_Task_s
{
    struct list_head      list;
    MI_U32                u32MagicNumber;
    MI_VPE_RegionInfo_t  *pstRegion; // IN
    wait_queue_head_t    queue;
    mi_vpe_RoiStatus_e   eRoiStatus;
    MI_U32               u32LumaData[ROI_WINDOW_MAX]; // Channel luma region data
    mi_vpe_ChannelInfo_t *pstChnnInfo;
} ROI_Task_t;


// mi vpe update 3d nr
void mi_vpe_impl_3DnrUpdateSwitchChannel(mi_vpe_DevInfo_t *pstDevInfo, struct list_head *pListHead, struct semaphore *pSemphore);
MI_BOOL mi_vpe_impl_3DnrUpdateProcessStart(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t * pstChnnInfo, mi_sys_ChnTaskInfo_t *pstTask);
void mi_vpe_impl_3DnrUpdateProcessEnd(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo);
void mi_vpe_impl_3DnrUpdateProcessFinish(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo);

// mi vpe update Roi
MI_BOOL mi_vpe_impl_RoiGetTask(mi_sys_ChnTaskInfo_t *pstTask, mi_vpe_DevInfo_t *pstDevInfo, ROI_Task_t **ppstRoiTask, struct list_head *pListHead, struct semaphore *pSemphore);
MI_BOOL mi_vpe_impl_RoiProcessTaskStart(mi_sys_ChnTaskInfo_t *pstTask, mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo, MI_SYS_FrameData_t *pstBuffFrame, ROI_Task_t *pstRoiTask);
void mi_vpe_impl_RoiProcessTaskEnd(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo);
void mi_vpe_impl_RoiProcessFinish(mi_vpe_DevInfo_t * pstDevInfo, mi_vpe_ChannelInfo_t * pstChnnInfo, mi_sys_ChnTaskInfo_t *pstTask);
#endif //_MI_VPE_IMPL_INTERNAL_H_
