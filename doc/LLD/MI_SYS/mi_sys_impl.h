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
#ifndef _MI_SYS_IMPL_H_
#define _MI_SYS_IMPL_H_


#include <linux/wait.h>
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/idr.h>

#include "mi_sys.h"
#include "mi_sys_internal.h"
#include "mi_sys_buf_mgr.h"

#define MI_ERR_NO_MEM (2)
#define MI_SUCCESS (0)
#define MI_SYS_ERR_BUSY (1)
#define E_MI_ERR_ILLEGAL_PARAM (3)
#define MI_SYS_MAX_TASK_INFO (100)

#define __MI_SYS_MAGIC_NUM__ 0x4D535953
#define __MI_SYS_CHN_MAGIC_NUM__ 0x4D43484E
#define __MI_SYS_INPUTPORT_MAGIC_NUM__ 0x4D5F494E
#define __MI_SYS_OUTPORT_MAGIC_NUM__ 0x4D4F5554

#define MI_SYS_OUTPUT_BUF_CNT_QUOTA_DEFAULT 6

struct MI_SYS_BufferAllocation_t;



struct MI_SYS_OutputPort_s;
struct MI_SYS_Channel_s;
typedef struct MI_SYS_InputPort_s
{
	MI_BOOL bPortEnable;
    MI_U32 u32MagicNumber;
	MI_U32 u32PortIndex;
	MI_U32 u32SrcFrmrate;
	MI_U32 u32DstFrmrate;
	MI_U32 u32CurRcParam;
	
	struct list_head stBindRelationShipList;
	
	MI_SYS_BufferQueue_t stUsrInjectBufQueue; 
	MI_SYS_BufferQueue_t stBindInputBufQueue;
	MI_SYS_BufferQueue_t stWorkingQueue;
	
	struct MI_SYS_Channel_s *pstChannel;
	struct MI_SYS_OutputPort_s *pstBindPeerOutputPort;
	struct mi_sys_Allocator_s *pstCusBufAllocator;

}MI_SYS_InputPort_t;


typedef struct MI_SYS_OutputPort_s
{
	MI_BOOL bPortEnable;
	MI_U32 u32MagicNumber;
	MI_U32 u32UsrFifoCount;

	MI_U32 u32OutputPortBufCntQuota;

	atomic_t usrLockedBufCnt;
	atomic_t totalOutputPortInUsedBuf;

	struct MI_SYS_BufferQueue_s stUsrGetFifoBufQueue;
	struct MI_SYS_BufferQueue_s stWorkingQueue; 

	struct mi_sys_Allocator_s *pstCusBufAllocator;
	struct MI_SYS_Channel_s *pstChannel;

	struct semaphore stBindedInputListSemlock;
	struct list_head stBindPeerInputPortList;//output
}MI_SYS_OutputPort_t;


struct MI_SYS_MOD_DEV_s;
typedef struct MI_SYS_Channel_s
{
	MI_BOOL bChnEnable;
	MI_U32 u32MagicNumber;
	MI_U32 u32ChannelId;
	struct MI_SYS_Allocator_Collection_s stAllocatorCollection;
	struct MI_SYS_MOD_DEV_s *pstModDev;
	struct MI_SYS_InputPort_s *pastInputPorts[MI_SYS_MAX_INPUT_PORT_CNT];
	struct MI_SYS_OutputPort_s *pastOutputPorts[MI_SYS_MAX_OUTPUT_PORT_CNT];
}MI_SYS_Channel_t;

typedef struct MI_SYS_MOD_DEV_s
{
    MI_U32 u32MagicNumber;
    MI_SYS_ModuleId_e eModuleId;
    MI_U32 u32DevId;
    MI_U32 u32InputPortNum;
    MI_U32 u32OutputPortNum;
    MI_U32 u32DevChnNum;

	MI_BOOL bCondition;  
	wait_queue_head_t inputWaitqueue;
	struct list_head listModDev;
    struct mi_sys_ModuleDevBindOps_s ops;
    struct MI_SYS_Allocator_Collection_s stAllocatorCollection;
    struct MI_SYS_Channel_s  astChannels[0];
	
	void *pUsrData;
}MI_SYS_MOD_DEV_t;


typedef enum
{
	E_MI_SYS_IDR_BUF_TYPE_INPUT_PORT = 0,
	E_MI_SYS_IDR_BUF_TYPE_OUTPUT_PORT
}MI_SYS_IDR_BUF_TYPE_e;

typedef struct MI_SYS_BufHandleIdrData_s
{
	MI_SYS_IDR_BUF_TYPE_e eBufType;
	MI_SYS_BufRef_t *pstBufRef;
	MI_SYS_ChnPort_t stChnPort;
}MI_SYS_BufHandleIdrData_t;

MI_S32 MI_SYS_IMPL_Init(void);

MI_S32 MI_SYS_IMPL_Exit(void);

MI_S32 MI_SYS_IMPL_BindChnPort(struct MI_SYS_ChnPort_s *pstSrcChnPort, struct MI_SYS_ChnPort_s *pstDstChnPort , MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate);

MI_S32 MI_SYS_IMPL_UnBindChnPort(struct MI_SYS_ChnPort_s *pstSrcChnPort, struct MI_SYS_ChnPort_s *pstDstChnPort);

MI_S32 MI_SYS_IMPL_GetBindbyDest (struct MI_SYS_ChnPort_s *pstDstChnPort, struct MI_SYS_ChnPort_s *pstSrcChnPort);

MI_S32 MI_SYS_IMPL_GetVersion(MI_SYS_Version_t *pstVersion);

MI_S32 MI_SYS_IMPL_GetCurPts(MI_U64 *pu64Pts);

MI_S32 MI_SYS_IMPL_InitPtsBase(MI_U64 u64PtsBase);

MI_S32 MI_SYS_IMPL_SyncPts(MI_U64 u64Pts);

MI_S32 MI_SYS_IMPL_Mmap(MI_U64 phyAddr, MI_U32 u32Size , void *pVirtualAddress);

MI_S32 MI_SYS_IMPL_Munmap(void *pVirtualAddress, MI_U32 u32Size);

MI_S32 MI_SYS_IMPL_SetReg(MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask);

MI_S32 MI_SYS_IMPL_GetReg(MI_U32 u32RegAddr, MI_U16 *pu16Value);

MI_S32 MI_SYS_IMPL_SetChnPortMMAConf(struct MI_SYS_ChnPort_s *pstChnPort,MI_U8 *pu8MMAHeapName);

MI_S32 MI_SYS_IMPL_GetChnPortMMAConf(struct MI_SYS_ChnPort_s *pstChnPort,void  *data, MI_U32 u32Length);

MI_S32 MI_SYS_IMPL_SetChnOutputFrcCtrl(struct MI_SYS_ChnPort_s *pstChnPort, MI_U32 u32FrcCtrlNumerator , MI_U32 u32FrcCtrlDenominator);

MI_S32 MI_SYS_IMPL_ChnInputPortGetBuf(struct MI_SYS_ChnPort_s *pstChnPort ,struct MI_SYS_BufConf_s *pstBufConf, struct MI_SYS_BufInfo_s *pstBufInfo , MI_SYS_BUF_HANDLE *BufHandle);

MI_S32 MI_SYS_IMPL_ChnInputPortPutBuf(MI_SYS_BUF_HANDLE BufHandle , MI_BOOL bDropBuf);

MI_S32 MI_SYS_IMPL_ChnOutputPortGetBuf(struct MI_SYS_ChnPort_s *pstChnPort,struct MI_SYS_BufInfo_s *pstBufInfo, MI_SYS_BUF_HANDLE *BufHandle);

MI_S32 MI_SYS_IMPL_ChnOutputPortPutBuf(MI_SYS_BUF_HANDLE BufHandle);

MI_S32 MI_SYS_IMPL_SetChnOutputPortDepth(struct MI_SYS_ChnPort_s *pstChnPort , MI_U32 u32UserFrameDepth , MI_U32 u32BufQueueDepth);

MI_S32 MI_SYS_IMPL_GetFd(struct MI_SYS_ChnPort_s *pstChnPort);

MI_S32 MI_SYS_IMPL_CloseFd(struct MI_SYS_ChnPort_s *pstChnPort);

MI_S32 MI_SYS_IMPL_ChnPortInjectBufBlk(struct MI_SYS_ChnPort_s *pstChnPort,struct MI_VB_BufBlkInfo_s * pstBufBlkInfo);

MI_S32 MI_SYS_IMPL_GetFd(struct MI_SYS_ChnPort_s *pstChnPort);
MI_S32 MI_SYS_IMPL_CloseFd(struct MI_SYS_ChnPort_s *pstChnPort);
MI_S32 MI_SYS_IMPL_SetChnOutPortDepth(struct MI_SYS_ChnPort_s *pstChnPort , MI_U32 u32UserFrameDepth , MI_U32 u32BufQueueDepth);


MI_SYS_DRV_HANDLE MI_SYS_IMPL_RegisterDev(struct mi_sys_ModuleDevInfo_s *pstMouleInfo, struct mi_sys_ModuleDevBindOps_s *pstModuleBindOps , void *pUsrData);
MI_S32 MI_SYS_IMPL_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle);


MI_S32 MI_SYS_IMPL_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, mi_sys_ChnBufInfo_t *pstChnBufInfo);

//typedef MI_S32 (* mi_sys_TaskIteratorCallBack)(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData);
MI_S32 MI_SYS_IMPL_DevTaskIterator(MI_SYS_DRV_HANDLE miSysDrvHandle, mi_sys_TaskIteratorCallBack pfCallBack,void *pUsrData);
MI_S32 MI_SYS_IMPL_PrepareTaskOutputBuf(mi_sys_ChnTaskInfo_t *pstTask);

//¥¶¿Ìinput /output buf£¨≤¢release task
MI_S32 MI_SYS_IMPL_FinishAndReleaseTask(mi_sys_ChnTaskInfo_t *pstTask);

MI_S32 MI_SYS_IMPL_RewindTask(mi_sys_ChnTaskInfo_t *pstTask);

MI_S32 MI_SYS_IMPL_WaitOnInputTaskAvailable(MI_SYS_DRV_HANDLE miSysDrvHandle , MI_S32 u32TimeOutMs);

MI_SYS_BufInfo_t *MI_SYS_IMPL_GetOutputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct MI_SYS_BufConf_s *stBufConfig , MI_BOOL *pbBlockedByRateCtrl);
MI_SYS_BufInfo_t *MI_SYS_IMPL_GetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId);

MI_S32 MI_SYS_IMPL_EnableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);
MI_S32 MI_SYS_IMPL_DisableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);

MI_S32 MI_SYS_IMPL_EnableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);
MI_S32 MI_SYS_IMPL_DisableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);

MI_S32 MI_SYS_IMPL_SetReg (MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask);
MI_S32 MI_SYS_IMPL_GetReg (MI_U32 u32RegAddr, MI_U16 *pu16Value);

MI_S32 MI_SYS_IMPL_FinishAndReleaseBuf(MI_SYS_BufInfo_t *buf);
MI_S32 MI_SYS_IMPL_RewindBuf(struct MI_SYS_BufInfo_s *buf);

MI_S32 MI_SYS_IMPL_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U32 u32Size ,MI_PHY *phyAddr);
MI_S32 MI_SYS_IMPL_MMA_Free(MI_PHY phyAddr);


void * MI_SYS_IMPL_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size , MI_BOOL bCache);
MI_S32 MI_SYS_IMPL_UnVmap(void *pVirtAddr);

struct mi_sys_Allocator_s;

MI_S32 MI_SYS_IMPL_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);
MI_S32 MI_SYS_IMPL_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);


#endif
