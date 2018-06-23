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

#ifndef _MI_SD_IMPL_INTERNAL_H_
#define _MI_SD_IMPL_INTERNAL_H_
#include <linux/wait.h>
#include <linux/interrupt.h>

//#include "HAL_SD.h"
#include "mhal_vpe.h"

#include "mi_sd.h"
#include "mi_sys_internal.h"
#include "mhal_cmdq.h"

#define MI_SD_TASK_NO_DRAM_OUTPUT           (1ul << 2)

#define __MI_SD_DEV_MAGIC__      (0x56512324)
#define __MI_SD_CHN_MAGIC__      (0x56512388)
#define __MI_SD_OUTPORT_MAGIC__  (0x56512394)

typedef struct MI_SD_OutPortInfo_s {
    MI_BOOL           bEnable;
    MI_SD_OuputPortAttr_t stPortMode;
    MI_SYS_ChnPort_t  stPeerOutputPortInfo; // Port binder to: Just for debug
    MI_SYS_PixelFormat_e  eRealOutputPixelFormat;  // Pixel format of target image
    MI_U64            u64SideBandMsg;

    MI_U64            u64GetOutputBufferCnt;
    MI_U64            u64GetOutputBuffFailCnt;             // SD get ouput buffer fail counter.
    MI_U64            u64FinishOutputBufferCnt;
    MI_U64            u64BufToHalCnt;

    MI_U32            u32FrameCntpers;
    MI_U32            u32FrameCntJiffies;
    MI_U8             u8PortFps;

    MI_U32            u32OutOffset;
    MI_U32            u32InOffset;
} mi_SD_OutPortInfo_t;

typedef enum
{
    E_MI_SD_CHANNEL_STATUS_INITED = 0,
    E_MI_SD_CHANNEL_STATUS_START,
    E_MI_SD_CHANNEL_STATUS_STOP,
    E_MI_SD_CHANNEL_STATUS_DESTROYED,
    E_MI_SD_CHANNEL_STATUS_NUM,
} mi_SD_ChannelStatus_e;

typedef struct {
    MI_U16 u16Width;
    MI_U16 u16Height;
    MI_SYS_CompressMode_e    eCompressMode;                      // Channel Input Compress mode
} mi_SD_SourceVideoInfo_t;

typedef struct {
    MI_SYS_PixelFormat_e ePixelFormat;
    MI_SYS_CompressMode_e eCompressMode;
    MI_U16 u16Width;
    MI_U16 u16Height;
    MS_U64 u64MetaInfo;

    void* pVirAddr[3];
    MI_PHY phyAddr[3];//notice that this is miu bus addr,not cpu bus addr.
    MI_U32 u32Stride[3];
} mi_SD_InBuffInfo_t;

#define EMPTY 0
#define OCCUPY 1

typedef struct {
    MI_BOOL bStatus;
    MI_U16  u16DropFence;
}mi_SD_DropFenceInfo_t;

typedef struct mi_SD_ChannelInfo_s {
    struct list_head         list;                               // Device active list
    MI_BOOL                  bCreated;                           // Channel: Created
    void                     *pSclCtx;                           // HAL layer: SCL context pointer
    MI_SD_ChannelAttr_t     stChnnAttr;                         // Channel attribution
    MI_SYS_PixelFormat_e    eInPixFmt;
    mi_SD_ChannelStatus_e   eStatus;                            // Channel status
    MI_SYS_CompressMode_e    eCompressMode;                      // Channel Input Compress mode
    mi_SD_SourceVideoInfo_t stSrcWin;                           // Chanel input source size
    MI_SYS_WindowRect_t      stCropWin;                          // Channel user setting crop window
    MI_SYS_WindowRect_t      stRealCrop;                         // Channel real crop window
    MI_SYS_ChnPort_t         stPeerInputPortInfo;                // Channel binder to: Just for debug
    mi_SD_OutPortInfo_t     stOutPortInfo[MI_SD_MAX_PORT_NUM]; // Information of output
    MI_PHY                   u64PhyAddrOffset[3];                // Channel address offset for sw crop window.
    MI_SD_CHANNEL           SDCh;
    struct semaphore         stChnnMutex;
    MI_U64                    u64TotalCreatedArea;               // Created channel total area
    MI_U32                    u32ChannelCreatedNum;              // Total channel number for user created
    //parameters for sideband crop for saving bw
    MI_U32  x_zoom_in_ratio;
    MI_U32  y_zoom_in_ratio;
    MI_U32  w_zoom_in_ratio;
    MI_U32  h_zoom_in_ratio;
    atomic_t                 stAtomTask;

    MI_U64                  u64ReleaseBufcnt;
    MI_U64                  u64GetInputBufferCnt;
    MI_U64                  u64GetInputBufferTodoCnt;
    MI_U16                   u16DumpTaskFileNum;

} mi_SD_ChannelInfo_t;


typedef struct {
    MI_U32                    u32MagicNumber;
    mi_SD_ChannelInfo_t      stChnnInfo[MI_SD_MAX_CHANNEL_NUM];// All channel information
    MI_SYS_DRV_HANDLE         hDevSysHandle;                     // Handle for MI_SYS device
    MI_BOOL                   bInited;                           // Weather MI_SD already inited.
    MHAL_CMDQ_CmdqInterface_t       *pstCmdMloadInfo;                  // Command Queue/Menuload interface
    unsigned int              uSDIrqNum;                        // SD irq number
    struct task_struct        *pstWorkThread;                    // Kernel thread for SDWorkThread
    struct task_struct        *pstProcThread;                    // Kernel thread for SD IRQ bottom
    MI_U32                    u32ChannelCreatedNum;              // Total channel number for user created
    MHalVpeGetCap_t           stCaps;
    MI_U64                    u64FramePts;
    MI_U16                    u16CmdqFence;
    mi_SD_DropFenceInfo_t    stDropFrameFence[MI_SD_MAX_WORKINGLIST_NODE];
//    mi_SD_RoiInfo_t          stRoiInfo;          // Roi information
    MI_U64                    u64TotalCreatedArea;               // Created channel total area
    MI_U32                    u32TaskNoToDramCnt;
    MI_BOOL                   bEnbaleIrq;                        // Irq enable
    MI_BOOL                   bSupportIrq;                        // Support Irq
    MHalVpeIrqEnableMode_e    eIrqEnableMode;
    atomic_t                  wqRedFlag;
} mi_SD_DevInfo_t;

typedef struct
{
   int totalAddedTask;
} mi_SD_IteratorWorkInfo_t;

#endif //_MI_SD_IMPL_INTERNAL_H_
