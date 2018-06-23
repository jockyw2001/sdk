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
/// @file   mi_vdec_impl.c
/// @brief vdec module impl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>


#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "mi_sys_buf_mgr.h"
#include "mi_sys_impl.h"
#include "mi_sys_internal.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_sys_mma_heap_impl.h"
#include "mi_sys_vb_pool_impl.h"
#include "mi_sys_meta_impl.h"
#include "mi_sys_chunk_impl.h"
#include "mi_sys_sideband_msg.h"
#include "mi_sys_log_impl.h"

#include <linux/version.h>
#include <linux/kernel.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
#include <../../mstar2/drv/miu/mdrv_miu.h>
#include <../../mstar2/drv/mma_heap/mdrv_mma_heap.h>
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
#include <mdrv_miu.h>
#include <mdrv_mma_heap.h>
#else
#error not support this kernel version
#endif

#include "mi_sys_mma_miu_protect_impl.h"

#define CHECK_MMA_HEAP_INFO 1//default enable for starting up log

#include <linux/fs.h>
#include <linux/anon_inodes.h>
#include <linux/mman.h>
#include <linux/file.h>
#include <linux/io.h>

#ifdef MI_SYS_PROC_FS_DEBUG
#include "drv_proc_ext.h"
#include "drv_proc_ext_k.h"
#endif

#include "mi_sys_debug.h"

#include "mi_syscfg.h"

#define MI_SYS_INPUT_PORT_HANDLE_START 0x40
#define MI_SYS_INPUT_PORT_HANDLE_END (0x4000-1)
#define MI_SYS_OUTPUT_PORT_HANDLE_START 0x4000
#define MI_SYS_OUTPUT_PORT_HANDLE_END (0x8000-1)
extern void mi_sys_vbpool_fill_bufinfo(MI_SYS_BufferAllocation_t *pstBufAllocation, MI_SYS_BufInfo_t pstBufInfo);

struct idr g_mi_sys_buf_handle_idr;
struct kmem_cache *mi_sys_bufref_cachep;
struct kmem_cache *mi_sys_inputport_cachep;
struct kmem_cache *mi_sys_outputport_cachep;
struct kmem_cache *mi_sys_taskinfo_cachep;
struct list_head mi_sys_global_dev_list;
struct semaphore mi_sys_global_dev_list_semlock;
struct mutex idr_alloc_mutex;
static MI_U64 g_pts_base=0;
static MI_U64 g_pts_base_mono_us=0;

extern struct kmem_cache *mi_sys_mma_allocation_cachep;
extern struct kmem_cache *mi_sys_meta_allocation_cachep;
extern struct kmem_cache *g_mi_sys_chunk_cachep;
extern struct kmem_cache *mi_sys_vbpool_allocation_cachep;
extern buf_allocation_ops_t _mi_impl_output_wrapper_allocation_ops;
extern struct MMA_BootArgs_Config mma_config[MAX_MMA_AREAS];

bool mma_config_exist_max_offset_to_curr_lx_mem = FALSE;//current only for vdec use
EXPORT_SYMBOL(mma_config_exist_max_offset_to_curr_lx_mem);

extern int mstar_driver_boot_mma_buffer_num ;
extern struct semaphore g_mma_list_semlock;
extern struct list_head g_mma_heap_list;
extern int g_kprotect_enabled;

extern int check_pstBufRef (const char *func, int line,
                    MI_SYS_BufRef_t *pstBufRef);
static int gSysInitCount = 0;

DEFINE_SEMAPHORE(mi_sys_global_SysInit);

static void _MI_SYS_IMPL_SetPollState(MI_SYS_OutputPort_t *pstOutputPort);

typedef void (*AllocationOnReleaseCB)(struct MI_SYS_BufferAllocation_s *thiz);



void _MI_SYS_IMPL_UserPutInputPortBuf_Ref_CB(MI_SYS_BufRef_t *pstBufRef, void *pCBData)
{
    int val;

    MI_SYS_InputPort_t *pstInputPort = (MI_SYS_InputPort_t*)pCBData;
    MI_SYS_BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

    val = atomic_dec_return(&pstInputPort->usrLockedInjectBufCnt);
    MI_SYS_BUG_ON(val <0);
    WAKE_UP_QUEUE_IF_NECESSARY(pstInputPort->inputBufCntWaitqueue);
}

void _MI_SYS_IMPL_OutputPortOnBufRefRelFunc(MI_SYS_BufRef_t *pstBufRef, void *pCBData)
{

}
void _MI_SYS_IMPL_InputPortOnBufRefRelFunc(MI_SYS_BufRef_t *pstBufRef, void *pCBData)
{

}

void _MI_SYS_IMPL_OutputUsrBufOnBufRefRelFunc(MI_SYS_BufRef_t *pstBufRef, void *pCBData)
{
    int val;

    MI_SYS_OutputPort_t *pstOutputPort = (MI_SYS_OutputPort_t*)pCBData;
    MI_SYS_BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

    val = atomic_dec_return(&pstOutputPort->usrLockedBufCnt);

    MI_SYS_BUG_ON(val<0);
}

void _MI_SYS_IMPL_OnAllocationFreeCB(MI_SYS_BufferAllocation_t *allocation)
{
    MI_SYS_OutputPort_t *pstOutputPort;
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufInfo_t *pstBufInfo;
    void *pVirtAddr;

    pstOutputPort = (MI_SYS_OutputPort_t *)allocation->private2;
    pstBufInfo = (MI_SYS_BufInfo_t *)&allocation->stBufInfo;

    MI_SYS_BUG_ON(!pstOutputPort);

    if(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__)
    {
          DBG_ERR("In %s, output magic number[0x%08x] incorrect, maybe you meet race condition between dev unregister & buf release!!!\n", __FUNCTION__, pstOutputPort->u32MagicNumber);
          // atomic_dec_return(&pstOutputPort->totalOutputPortInUsedBuf);
          return;
    }


    MI_SYS_BUG_ON(!pstOutputPort->pstChannel ||
                  pstOutputPort->pstChannel->u32MagicNumber!= __MI_SYS_CHN_MAGIC_NUM__);

    MI_SYS_BUG_ON(!pstOutputPort->pstChannel->pstModDev ||
                  pstOutputPort->pstChannel->pstModDev->u32MagicNumber!= __MI_SYS_MAGIC_NUM__);

    if(pstOutputPort->pstChannel->pstModDev->ops.OnOutputPortBufRelease)
    {
        stChnPort.eModId = pstOutputPort->pstChannel->pstModDev->eModuleId;
        stChnPort.u32DevId = pstOutputPort->pstChannel->pstModDev->u32DevId;
        stChnPort.u32ChnId = pstOutputPort->pstChannel->u32ChannelId;
        stChnPort.u32PortId = pstOutputPort->u32PortId;
        MI_SYS_BUG_ON(stChnPort.u32PortId<0 ||stChnPort.u32PortId>=MI_SYS_MAX_OUTPUT_PORT_CNT);
        pVirtAddr = allocation->ops.vmap_kern(allocation);
        MI_SYS_BUG_ON(!pVirtAddr);
        pstBufInfo->stMetaData.pVirAddr = pVirtAddr;
        pstOutputPort->pstChannel->pstModDev->ops.OnOutputPortBufRelease(&stChnPort , pstBufInfo);
        allocation->ops.vunmap_kern(allocation);
        pstBufInfo->stMetaData.pVirAddr = NULL;
    }
    atomic_dec_return(&pstOutputPort->totalOutputPortInUsedBuf);

}

static void _MI_SYS_IMPL_Ouputport_wrapper_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation)
{

    AllocationOnReleaseCB func = (AllocationOnReleaseCB)pst_allocation->private1;

    MI_SYS_BUG_ON(pst_allocation->ops.OnRelease != _MI_SYS_IMPL_Ouputport_wrapper_allocation_on_release);
    MI_SYS_BUG_ON(func == NULL);

    pst_allocation->ops.OnRelease = func;
    _MI_SYS_IMPL_OnAllocationFreeCB(pst_allocation);
    pst_allocation->ops.OnRelease(pst_allocation);
}

MI_S32 _MI_SYS_IMPL_UnMmapUserVirAddr(MI_SYS_BufRef_t *pstBufRef, MI_BOOL bFlushCache)
{
    DBG_INFO("pstBufRef:%p\n", pstBufRef);

    if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_RAW)
    {
        if (bFlushCache && (pstBufRef->bufinfo.stRawData.u32ContentSize != 0))
            MI_SYS_IMPL_FlushInvCache(pstBufRef->bufinfo.stRawData.pVirAddr, pstBufRef->bufinfo.stRawData.u32ContentSize);
    }
    else if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_META)
    {
        //do nothing.
    }
    else
    {
        if(bFlushCache && pstBufRef->bufinfo.stFrameData.u32BufSize)
           MI_SYS_IMPL_FlushInvCache(pstBufRef->bufinfo.stFrameData.pVirAddr[0], pstBufRef->bufinfo.stFrameData.u32BufSize);
    }


    pstBufRef->pstBufAllocation->ops.unmap_user(pstBufRef->pstBufAllocation);

    if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_RAW)
    {
        pstBufRef->bufinfo.stRawData.pVirAddr = NULL;
    }
    else if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_META)
    {
        pstBufRef->bufinfo.stMetaData.pVirAddr = NULL;
    }
    else
    {
        pstBufRef->bufinfo.stFrameData.pVirAddr[0] = NULL;
        pstBufRef->bufinfo.stFrameData.pVirAddr[1] = NULL;
        pstBufRef->bufinfo.stFrameData.pVirAddr[2] = NULL;
    }
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_IMPL_MmapToUserVirAddr(MI_SYS_BufRef_t *pstBufRef, MI_BOOL bFlushCache)
{
    MI_U64 u64Offset;

    void *pVirAddr = pstBufRef->pstBufAllocation->ops.map_user(pstBufRef->pstBufAllocation);

    if(pVirAddr == NULL)
    {
        DBG_ERR("failed to map va for pid %d\n");
        return MI_ERR_SYS_FAILED;
    }
    if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_RAW)
    {
        if (bFlushCache && (pstBufRef->bufinfo.stRawData.u32ContentSize != 0))
            MI_SYS_IMPL_FlushInvCache(pVirAddr, pstBufRef->bufinfo.stRawData.u32ContentSize);

        pstBufRef->bufinfo.stRawData.pVirAddr = pVirAddr;
    }
    else if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_META)
    {
        //Meta data is pure cpu access, no L1,L2& L3 cache flush needed
        //if（bFlushCache�?       //    MI_SYS_IMPL_FlushInvCache(pVirAddr，pstBufRef->bufinfo.stMetaData.u32Size);
        pstBufRef->bufinfo.stMetaData.pVirAddr = pVirAddr;
    }
    else
    {
        if(bFlushCache && pstBufRef->bufinfo.stFrameData.u32BufSize)
           MI_SYS_IMPL_FlushInvCache(pVirAddr, pstBufRef->bufinfo.stFrameData.u32BufSize);
        pstBufRef->bufinfo.stFrameData.pVirAddr[0] = pVirAddr;
        if(pVirAddr && pstBufRef->bufinfo.stFrameData.phyAddr[1])
        {
            u64Offset = pstBufRef->bufinfo.stFrameData.phyAddr[1]
                - pstBufRef->bufinfo.stFrameData.phyAddr[0];
            pstBufRef->bufinfo.stFrameData.pVirAddr[1] = (void*)((char*)pVirAddr + u64Offset);
            if(pstBufRef->bufinfo.stFrameData.phyAddr[2])
            {
                u64Offset = pstBufRef->bufinfo.stFrameData.phyAddr[2]
                    - pstBufRef->bufinfo.stFrameData.phyAddr[0];
                pstBufRef->bufinfo.stFrameData.pVirAddr[2] = (void*)((char*)pVirAddr + u64Offset);
            }
        }
    }

    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_MmapBufToUser(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_SYS_BufInfo_t *pstBufInfo, void **ppBufHandle)
{
    MI_SYS_BufRef_t *pstBufRef = NULL;
    MI_SYS_BufHandleIdrData_t *phBufIdrData = NULL;

    phBufIdrData = kmalloc(sizeof(MI_SYS_BufHandleIdrData_t), GFP_ATOMIC);
    if(phBufIdrData == NULL)
    {
        DBG_ERR("Malloc BufIdr Faild\n");
        return MI_ERR_SYS_NOMEM;
    }

    DBG_INFO("pstBufInfo:%p\n", pstBufInfo);
    pstBufRef = container_of(pstBufInfo, MI_SYS_BufRef_t, bufinfo);
    if(!pstBufRef)
    {
        kfree(phBufIdrData);
        DBG_ERR("Get BufRef Faild\n");
        return MI_ERR_SYS_NOBUF;
    }

    phBufIdrData->eBufType = E_MI_SYS_IDR_BUF_TYPE_MMAP_TO_USER_SPACE;
    phBufIdrData->pstBufRef = pstBufRef;
    phBufIdrData->miSysDrvHandle = miSysDrvHandle;
    if (MI_SUCCESS != _MI_SYS_IMPL_MmapToUserVirAddr(pstBufRef, FALSE))
    {
        kfree(phBufIdrData);
        phBufIdrData = NULL;
        DBG_ERR("Mmap To User Vir Addr Faild\n");
        return MI_ERR_SYS_FAILED;
    }

    *ppBufHandle = phBufIdrData;
    DBG_INFO("ppBufHandle:%p\n", *ppBufHandle);
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_UnmapBufToUser(MI_SYS_DRV_HANDLE miSysDrvHandle, void *pBufHandle, MI_SYS_BufInfo_t **ppstBufInfo)
{
    MI_SYS_BufHandleIdrData_t *pstBufHandleData = (MI_SYS_BufHandleIdrData_t *)pBufHandle;

    DBG_INFO("pstBufHandleData:%p\n", pstBufHandleData);
    MI_SYS_BUG_ON(pstBufHandleData->miSysDrvHandle != miSysDrvHandle);
    MI_SYS_BUG_ON(pstBufHandleData->eBufType != E_MI_SYS_IDR_BUF_TYPE_MMAP_TO_USER_SPACE);
    if (MI_SUCCESS != _MI_SYS_IMPL_UnMmapUserVirAddr(pstBufHandleData->pstBufRef, FALSE))
    {
        DBG_ERR("unmmap buf to user faild\n");
        return MI_ERR_SYS_FAILED;
    }

    *ppstBufInfo = &(pstBufHandleData->pstBufRef->bufinfo);
    DBG_INFO("bufinfo:0x%x\n", *ppstBufInfo);

    kfree(pstBufHandleData);

    return MI_SUCCESS;
}

static inline MI_S32  _MI_SYS_IMPL_CheckInputOutputBindCoherence(MI_SYS_InputPort_t *pstInputPort, MI_SYS_OutputPort_t *pstOutputPort)
{
    MI_S32 find = 0;
    struct list_head *pos;
    MI_SYS_InputPort_t *pstInputportCur;
    down(&pstOutputPort->stBindedInputListSemlock);
    list_for_each(pos , &pstOutputPort->stBindPeerInputPortList)
    {
        pstInputportCur = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
        if(pstInputportCur == pstInputPort)
        {
            find = 1;
            MI_SYS_BUG_ON(pstInputportCur->pstBindPeerOutputPort != pstOutputPort);
            break;
        }
    }
    up(&pstOutputPort->stBindedInputListSemlock);
    MI_SYS_BUG_ON(find == 0);
    return find;
}

static inline MI_BOOL _MI_SYS_IMPL_CheckBufConfValid(MI_SYS_BufConf_t *pstBufConf)
{
    MI_BOOL ret = FALSE;
    switch(pstBufConf->eBufType)
    {
        case E_MI_SYS_BUFDATA_RAW:
            if(pstBufConf->stRawCfg.u32Size > 0)
                ret = TRUE;
            break;
        case E_MI_SYS_BUFDATA_FRAME:
            if(pstBufConf->stFrameCfg.u16Width > 0 && pstBufConf->stFrameCfg.u16Height > 0
                &&pstBufConf->stFrameCfg.eFrameScanMode < E_MI_SYS_FRAME_SCAN_MODE_MAX
                &&pstBufConf->stFrameCfg.eFormat < E_MI_SYS_PIXEL_FRAME_FORMAT_MAX)
                ret = TRUE;
            break;
        case E_MI_SYS_BUFDATA_META:
            if(pstBufConf->stMetaCfg.u32Size > 0)
                ret = TRUE;
            break;
        default:
            ret = FALSE;
    }
    return ret;
}

static void _mi_sys_InitBufExtraConf(MI_SYS_FrameBufExtraConfig_t *pstFrameBufExtConf)
{
  pstFrameBufExtConf->u16BufHAlignment = 32;
  pstFrameBufExtConf->u16BufVAlignment = 1;
  pstFrameBufExtConf->bClearPadding = FALSE;
}

MI_SYS_InputPort_t *_MI_SYS_IMPL_CreateInputPort(MI_SYS_Channel_t *pstChannel, MI_U32 u32PortIndex)
{
    MI_SYS_InputPort_t *pstInputPort;

    MI_SYS_BUG_ON(!pstChannel);

    pstInputPort = (MI_SYS_InputPort_t*)kmem_cache_alloc(mi_sys_inputport_cachep, GFP_KERNEL);
    if(!pstInputPort)
    {
        DBG_ERR("Creat Inpot Failed\n");
        return NULL;
    }

    MI_SYS_BUG_ON(u32PortIndex >= pstChannel->pstModDev->u32InputPortNum);

    memset(pstInputPort, 0x0, sizeof(MI_SYS_InputPort_t));
    pstInputPort->pstChannel = pstChannel;
    pstInputPort->u32PortId = u32PortIndex;
    mi_sys_init_buf_queue(&pstInputPort->stUsrInjectBufQueue,
                           pstChannel->pstModDev->eModuleId,
                           MI_SYS_QUEUE_ID_USR_INJECT,
                           pstChannel->pstModDev->u32DevId,
                           pstChannel->u32ChannelId,
                           u32PortIndex, 0);
    mi_sys_init_buf_queue(&pstInputPort->stBindInputBufQueue,
                           pstChannel->pstModDev->eModuleId,
                           MI_SYS_QUEUE_ID_BIND_INPUT,
                           pstChannel->pstModDev->u32DevId,
                           pstChannel->u32ChannelId,
                           u32PortIndex, 0);
    mi_sys_init_buf_queue(&pstInputPort->stWorkingQueue,
                           pstChannel->pstModDev->eModuleId,
                           MI_SYS_QUEUE_ID_WORKING,
                           pstChannel->pstModDev->u32DevId,
                           pstChannel->u32ChannelId,
                           u32PortIndex, 0);
    pstInputPort->pstBindPeerOutputPort = NULL;
    INIT_LIST_HEAD(&pstInputPort->stBindRelationShipList);
    pstInputPort->bPortEnable = FALSE;
    pstInputPort->u32MagicNumber = __MI_SYS_INPUTPORT_MAGIC_NUM__;
    pstInputPort->pstCusBufAllocator = NULL;
    pstInputPort->u32PortId = u32PortIndex;
    pstInputPort->u64SidebandMsg = MI_SYS_SIDEBAND_MSG_NULL;
    atomic_set(&pstInputPort->usrLockedInjectBufCnt , 0);
    init_waitqueue_head(&pstInputPort->inputBufCntWaitqueue);
    pstInputPort->bEnableLowLatencyReceiveMode = FALSE;
    pstInputPort->u32LowLatencyDelayMs = 0;
    atomic_set(&pstInputPort->scheduledDelayTaskCnt, 0);
    pstInputPort->u32LastStaticDelayTotalMS = 0;
    pstInputPort->u32LastStaticDelayTotalCnt = 0;
    pstInputPort->u32StaticDelayAccmMS = 0;
    pstInputPort->u32StaticDelayAccmCnt=0;
    _mi_sys_InitBufExtraConf(&pstInputPort->stInputBufExtraConf);

    return pstInputPort;
}

MI_S32 _MI_SYS_IMPL_UnBindChannelPort(MI_SYS_InputPort_t *pstInputPort,MI_SYS_OutputPort_t *pstOutputPort);

MI_S32 _MI_SYS_IMPL_ReleaseInputPort(MI_SYS_InputPort_t *pstInputPort)
{
    MI_SYS_BufRef_t *pstBufRef = NULL;
    int last_jiffies;
    MI_U32 u32LoopCnt = 0;
    int iScheduledDelayTaskCnt;

    MI_SYS_BUG_ON(!pstInputPort | !pstInputPort->pstChannel || !pstInputPort->pstChannel->pstModDev);
    MI_SYS_BUG_ON(pstInputPort->pstChannel->pstModDev->u32InputPortNum <= pstInputPort->u32PortId);

    if(pstInputPort->stWorkingQueue.queue_buf_count)
        return MI_ERR_SYS_BUSY;

    last_jiffies = jiffies;
    //wait all inputport delay low latency task done
    while((iScheduledDelayTaskCnt = atomic_read(&pstInputPort->scheduledDelayTaskCnt)))
    {
        BUG_ON(iScheduledDelayTaskCnt < 0);
        msleep(1);
        u32LoopCnt++;

        if(u32LoopCnt%1000 == 0)
        {
            DBG_ERR(" waiting delay input low latency task for more than %ums\n", jiffies_to_msecs(jiffies-last_jiffies));
        }
    }


    pstInputPort->bPortEnable = FALSE;
    if(pstInputPort->pstBindPeerOutputPort)
    {
        //unbind channel input port
        MI_SYS_OutputPort_t *pstOutputPort  = pstInputPort->pstBindPeerOutputPort;
        MI_SYS_BUG_ON(list_empty(&pstInputPort->stBindRelationShipList));
        _MI_SYS_IMPL_UnBindChannelPort(pstInputPort, pstOutputPort);
        MI_SYS_BUG_ON(pstInputPort->pstBindPeerOutputPort);
        MI_SYS_BUG_ON(!list_empty(&pstInputPort->stBindRelationShipList));
    }

    pstBufRef=mi_sys_remove_from_queue_head(&pstInputPort->stUsrInjectBufQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef=mi_sys_remove_from_queue_head(&pstInputPort->stUsrInjectBufQueue);
    }

    pstBufRef=mi_sys_remove_from_queue_head(&pstInputPort->stBindInputBufQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef=mi_sys_remove_from_queue_head(&pstInputPort->stBindInputBufQueue);
    }

    //for debug purpose
    memset(pstInputPort, 0x22, sizeof(*pstInputPort));
    kmem_cache_free(mi_sys_inputport_cachep, pstInputPort);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_IMPL_DestroyOutputPort(struct  MI_SYS_OutputPort_s *pstOutputPort)
{
    MI_SYS_BufRef_t *pstBufRef;

    MI_SYS_BUG_ON(!pstOutputPort | !pstOutputPort->pstChannel || !pstOutputPort->pstChannel->pstModDev);

    if(pstOutputPort->stWorkingQueue.queue_buf_count)
        return MI_ERR_SYS_BUSY;

    pstBufRef=mi_sys_remove_from_queue_head(&pstOutputPort->stUsrGetFifoBufQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef=mi_sys_remove_from_queue_head(&pstOutputPort->stUsrGetFifoBufQueue);
    }

    pstBufRef=mi_sys_remove_from_queue_head(&pstOutputPort->stDrvBkRefFifoQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef=mi_sys_remove_from_queue_head(&pstOutputPort->stDrvBkRefFifoQueue);
    }

    //for debug purpose
    memset(pstOutputPort, 0x23, sizeof(*pstOutputPort));
    kmem_cache_free(mi_sys_outputport_cachep, pstOutputPort);
    return MI_SUCCESS;
}

MI_SYS_OutputPort_t * _MI_SYS_IMPL_GetInputportPeer(MI_SYS_InputPort_t *pstInputPort)
{
    MI_SYS_BUG_ON(!pstInputPort);

    if(list_empty(&pstInputPort->stBindRelationShipList))
    {
        MI_SYS_BUG_ON(pstInputPort->pstBindPeerOutputPort);
        return NULL;
    }

    MI_SYS_BUG_ON(!pstInputPort->pstBindPeerOutputPort);

    _MI_SYS_IMPL_CheckInputOutputBindCoherence(pstInputPort, pstInputPort->pstBindPeerOutputPort);

    return pstInputPort->pstBindPeerOutputPort;
}

MI_S32 _MI_SYS_IMPL_BindChannelPort(MI_SYS_InputPort_t *pstInputPort,  MI_SYS_OutputPort_t *pstOutputPort,
                                            MI_U32 u32SrcFrmrate,MI_U32 u32DstFrmrate)
{
    MI_SYS_ChnPort_t stChnInputPort;
    MI_SYS_ChnPort_t stChnOutputPort;

    MI_SYS_BUG_ON(!pstOutputPort || !pstInputPort);

    MI_SYS_BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

    MI_SYS_BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

    if(pstOutputPort->pstChannel == pstInputPort->pstChannel)
    {
        DBG_ERR("Same Channel\n");
        return MI_ERR_SYS_ILLEGAL_PARAM;
    }

    if(u32DstFrmrate>u32SrcFrmrate)
    u32DstFrmrate = u32SrcFrmrate;

    memset(&stChnInputPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stChnOutputPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    down(&pstOutputPort->stBindedInputListSemlock);

    if(!list_empty(&pstInputPort->stBindRelationShipList))
    {
        MI_S32 s32Ret = MI_ERR_SYS_BUSY;
        DBG_INFO("Port %p\n", pstInputPort->pstBindPeerOutputPort);
        MI_SYS_BUG_ON(!pstInputPort->pstBindPeerOutputPort);
        if (pstInputPort->pstBindPeerOutputPort == pstOutputPort)
        {
            if ((pstInputPort->u32SrcFrmrate != u32SrcFrmrate) || (pstInputPort->u32DstFrmrate != u32DstFrmrate))
            {
                pstInputPort->u32SrcFrmrate = u32SrcFrmrate;
                pstInputPort->u32DstFrmrate = u32DstFrmrate;
                pstInputPort->u32CurRcParam = u32SrcFrmrate;
            }
            s32Ret = MI_SUCCESS;
        }
        up(&pstOutputPort->stBindedInputListSemlock);
        if (MI_SUCCESS != s32Ret)
        {
            DBG_ERR("Chn Already Bind\n");
        }

        return s32Ret;
    }

    MI_SYS_BUG_ON(pstInputPort->pstBindPeerOutputPort);
    list_add(&pstInputPort->stBindRelationShipList, &pstOutputPort->stBindPeerInputPortList);
    pstInputPort->pstBindPeerOutputPort = pstOutputPort;
    //reject to receive buf before channel bind callbacks
    pstInputPort->u32SrcFrmrate= 1;
    pstInputPort->u32DstFrmrate = 0;
    pstInputPort->u32CurRcParam= 0;

    up(&pstOutputPort->stBindedInputListSemlock);

    stChnInputPort.eModId = pstInputPort->pstChannel->pstModDev->eModuleId;
    stChnInputPort.u32DevId = pstInputPort->pstChannel->pstModDev->u32DevId;
    stChnInputPort.u32ChnId = pstInputPort->pstChannel->u32ChannelId;
    stChnInputPort.u32PortId = pstInputPort->u32PortId;

    stChnInputPort.bEnableLowLatencyReceiveMode = pstInputPort->bEnableLowLatencyReceiveMode;
    stChnInputPort.u32LowLatencyDelayMs = pstInputPort->u32LowLatencyDelayMs;

    stChnOutputPort.eModId = pstOutputPort->pstChannel->pstModDev->eModuleId;
    stChnOutputPort.u32DevId = pstOutputPort->pstChannel->pstModDev->u32DevId;
    stChnOutputPort.u32ChnId = pstOutputPort->pstChannel->u32ChannelId;
    stChnOutputPort.u32PortId = pstOutputPort->u32PortId;

    DBG_INFO("InputPort(%d %d %d %d), OutputPort(%d %d %d %d)\n",
    stChnInputPort.eModId, stChnInputPort.u32DevId, stChnInputPort.u32ChnId, stChnInputPort.u32PortId,
    stChnOutputPort.eModId, stChnOutputPort.u32DevId, stChnOutputPort.u32ChnId, stChnOutputPort.u32PortId);
    if(pstInputPort->pstChannel->pstModDev->ops.OnBindInputPort)
    {

        DBG_INFO("pUsrData:%p\n", pstInputPort->pstChannel->pstModDev->pUsrData);
        pstInputPort->pstChannel->pstModDev->ops.OnBindInputPort(&stChnInputPort, &stChnOutputPort, pstInputPort->pstChannel->pstModDev->pUsrData);
    }

    if(pstOutputPort->pstChannel->pstModDev->ops.OnBindOutputPort)
    {

        DBG_INFO("pUsrData:%p\n", pstInputPort->pstChannel->pstModDev->pUsrData);
        pstOutputPort->pstChannel->pstModDev->ops.OnBindOutputPort(&stChnOutputPort, &stChnInputPort, pstOutputPort->pstChannel->pstModDev->pUsrData);
    }

    //begin to receive
    pstInputPort->u32SrcFrmrate= u32SrcFrmrate;
    pstInputPort->u32DstFrmrate = u32DstFrmrate;
    pstInputPort->u32CurRcParam= u32SrcFrmrate;

    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_IMPL_UnBindChannelPort(MI_SYS_InputPort_t *pstInputPort,MI_SYS_OutputPort_t *pstOutputPort)
{
    MI_SYS_ChnPort_t stChnInputPort;
    MI_SYS_ChnPort_t stChnOutputPort;
    MI_SYS_InputPort_t *pstCur = NULL;
    struct list_head *pos;

    MI_SYS_BUG_ON(!pstOutputPort || !pstInputPort);

    MI_SYS_BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

    if(list_empty(&pstInputPort->stBindRelationShipList))
    {
        MI_SYS_BUG_ON(pstInputPort->pstBindPeerOutputPort);
        return MI_ERR_SYS_ILLEGAL_PARAM;
    }

    down(&pstOutputPort->stBindedInputListSemlock);
    list_for_each(pos, &pstOutputPort->stBindPeerInputPortList)
    {
        pstCur = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
        MI_SYS_BUG_ON(pstCur->pstBindPeerOutputPort != pstOutputPort);
        if(pstCur == pstInputPort)
        {
            break;
        }
    }
    up(&pstOutputPort->stBindedInputListSemlock);

    if(pstCur == pstInputPort)
    {
        memset(&stChnInputPort, 0x0, sizeof(MI_SYS_ChnPort_t));
        memset(&stChnOutputPort, 0x0, sizeof(MI_SYS_ChnPort_t));
        stChnInputPort.eModId = pstInputPort->pstChannel->pstModDev->eModuleId;
        stChnInputPort.u32DevId = pstInputPort->pstChannel->pstModDev->u32DevId;
        stChnInputPort.u32ChnId = pstInputPort->pstChannel->u32ChannelId;
        stChnInputPort.u32PortId = pstInputPort->u32PortId;

        stChnInputPort.bEnableLowLatencyReceiveMode = pstInputPort->bEnableLowLatencyReceiveMode;
        stChnInputPort.u32LowLatencyDelayMs = pstInputPort->u32LowLatencyDelayMs;

        stChnOutputPort.eModId = pstOutputPort->pstChannel->pstModDev->eModuleId;
        stChnOutputPort.u32DevId = pstOutputPort->pstChannel->pstModDev->u32DevId;
        stChnOutputPort.u32ChnId = pstOutputPort->pstChannel->u32ChannelId;
        stChnOutputPort.u32PortId = pstOutputPort->u32PortId;

        DBG_INFO("InputPort(%d %d %d %d), OutputPort(%d %d %d %d)\n",
            stChnInputPort.eModId, stChnInputPort.u32DevId, stChnInputPort.u32ChnId, stChnInputPort.u32PortId,
            stChnOutputPort.eModId, stChnOutputPort.u32DevId, stChnOutputPort.u32ChnId, stChnOutputPort.u32PortId);
        if(pstInputPort->pstChannel->pstModDev->ops.OnUnBindInputPort)
            pstInputPort->pstChannel->pstModDev->ops.OnUnBindInputPort(&stChnInputPort, &stChnOutputPort, pstInputPort->pstChannel->pstModDev->pUsrData);
        if(pstOutputPort->pstChannel->pstModDev->ops.OnUnBindOutputPort)
            pstOutputPort->pstChannel->pstModDev->ops.OnUnBindOutputPort(&stChnOutputPort, &stChnInputPort, pstOutputPort->pstChannel->pstModDev->pUsrData);

        down(&pstOutputPort->stBindedInputListSemlock);
        if(pstInputPort->pstBindPeerOutputPort)
        {
            MI_SYS_BUG_ON(pstInputPort->pstBindPeerOutputPort != pstOutputPort);
            MI_SYS_BUG_ON(list_empty(&pstInputPort->stBindRelationShipList));
            pstInputPort->pstBindPeerOutputPort = NULL;
            list_del(&pstInputPort->stBindRelationShipList);
            INIT_LIST_HEAD(&pstInputPort->stBindRelationShipList);
        }
        else
        {
            DBG_WRN("inconsistant state in %s\n", __FUNCTION__);
        }
        up(&pstOutputPort->stBindedInputListSemlock);
        return MI_SUCCESS;
    }

    return MI_ERR_SYS_ILLEGAL_PARAM;
}

MI_SYS_OutputPort_t *_MI_SYS_IMPL_CreateOutputPort(MI_SYS_Channel_t *pstChannel, MI_U32 u32PortIndex)
{
    MI_SYS_OutputPort_t *pstOutputPort;

    MI_SYS_BUG_ON(!pstChannel);

    pstOutputPort = (MI_SYS_OutputPort_t *)kmem_cache_alloc(mi_sys_outputport_cachep, GFP_KERNEL);
    if(!pstOutputPort)
    {
        DBG_ERR("Alloc Output Port Faild\n");
        return NULL;
    }
    MI_SYS_BUG_ON(u32PortIndex >= pstChannel->pstModDev->u32OutputPortNum);

    memset(pstOutputPort, 0x0, sizeof(MI_SYS_OutputPort_t));
    pstOutputPort->pstChannel = pstChannel;
    mi_sys_init_buf_queue(&pstOutputPort->stUsrGetFifoBufQueue,
                           pstChannel->pstModDev->eModuleId,
                           MI_SYS_QUEUE_ID_USR_GET_FIFO,
                           pstChannel->pstModDev->u32DevId,
                           pstChannel->u32ChannelId,
                           u32PortIndex, 1);
    mi_sys_init_buf_queue(&pstOutputPort->stDrvBkRefFifoQueue,
                           pstChannel->pstModDev->eModuleId,
                           MI_SYS_QUEUE_ID_DRIVER_BACK_REF,
                           pstChannel->pstModDev->u32DevId,
                           pstChannel->u32ChannelId,
                           u32PortIndex, 1);
    mi_sys_init_buf_queue(&pstOutputPort->stWorkingQueue,
                           pstChannel->pstModDev->eModuleId,
                           MI_SYS_QUEUE_ID_WORKING,
                           pstChannel->pstModDev->u32DevId,
                           pstChannel->u32ChannelId,
                           u32PortIndex, 1);
    sema_init(&pstOutputPort->stBindedInputListSemlock, 1);
    sema_init(&pstOutputPort->stPollSemlock, 1);
    INIT_LIST_HEAD(&pstOutputPort->stBindPeerInputPortList);
    pstOutputPort->u32MagicNumber = __MI_SYS_OUTPORT_MAGIC_NUM__;
    atomic_set(&pstOutputPort->usrLockedBufCnt, 0);
    atomic_set(&pstOutputPort->totalOutputPortInUsedBuf , 0);
    pstOutputPort->pstCusBufAllocator = NULL;
    pstOutputPort->bPortEnable = FALSE;
    pstOutputPort->u32DrvBkRefFifoDepth = 0;
    pstOutputPort->u32UsrFifoCount = 0;
    pstOutputPort->u64UsrGetFifoBufQueueAddCnt = 0;
    pstOutputPort->u64UsrGetFifoBufQueueDiscardCnt = 0;
    pstOutputPort->u32OutputPortBufCntQuota = MI_SYS_OUTPUT_BUF_CNT_QUOTA_DEFAULT;
    pstOutputPort->u32PortId = u32PortIndex;
    _mi_sys_InitBufExtraConf(&pstOutputPort->stOutputBufExtraRequirements);

    return pstOutputPort;
}

MI_S32 _MI_SYS_IMPL_ReleaseOutputPort(MI_SYS_OutputPort_t *pstOutputPort)
{
    MI_SYS_BufRef_t *pstBufRef;
    MI_SYS_InputPort_t *pstInputPort;

    MI_SYS_BUG_ON(!pstOutputPort);

    if(pstOutputPort->stWorkingQueue.queue_buf_count)
        return  MI_ERR_SYS_BUSY;

    pstOutputPort->bPortEnable = FALSE;

    while(!list_empty(&pstOutputPort->stBindPeerInputPortList))
    {
        pstInputPort  = container_of((struct list_head*)pstOutputPort->stBindPeerInputPortList.next, struct MI_SYS_InputPort_s, stBindRelationShipList);
        MI_SYS_BUG_ON(list_empty(&pstInputPort->stBindRelationShipList));
        MI_SYS_BUG_ON(pstInputPort->pstBindPeerOutputPort==NULL);
        _MI_SYS_IMPL_UnBindChannelPort(pstInputPort, pstOutputPort);
        MI_SYS_BUG_ON(pstInputPort->pstBindPeerOutputPort);
        MI_SYS_BUG_ON(!list_empty(&pstInputPort->stBindRelationShipList));
    }

    pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stUsrGetFifoBufQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stUsrGetFifoBufQueue);
    }

    pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stDrvBkRefFifoQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stDrvBkRefFifoQueue);
    }
    memset(pstOutputPort, 0x24, sizeof(*pstOutputPort));
    kmem_cache_free(mi_sys_outputport_cachep, pstOutputPort);

    return MI_SUCCESS;
}

MI_S32 _MI_SYS_IMPL_InitChannel( MI_SYS_Channel_t *pstChannel, MI_SYS_MOD_DEV_t* pstModDev)
{
    MI_U32 u32PortId;

    MI_SYS_BUG_ON(!pstChannel ||!pstModDev);
    MI_SYS_BUG_ON(pstModDev->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
    MI_SYS_BUG_ON(pstModDev->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);

    DBG_ENTER();
    memset(pstChannel, 0, sizeof(MI_SYS_Channel_t));

    pstChannel->pstModDev = pstModDev;

    for( u32PortId = 0 ; u32PortId < pstModDev->u32InputPortNum ; u32PortId++)
    {
        pstChannel->pastInputPorts[u32PortId] = _MI_SYS_IMPL_CreateInputPort( pstChannel,u32PortId);
        if(!pstChannel->pastInputPorts[u32PortId])
            goto Failed_Rel;
    }

    for( u32PortId = 0 ; u32PortId < pstModDev->u32OutputPortNum ; u32PortId ++)
    {
        pstChannel->pastOutputPorts[u32PortId] = _MI_SYS_IMPL_CreateOutputPort( pstChannel,  u32PortId);
        if(!pstChannel->pastOutputPorts[u32PortId])
            goto Failed_Rel;
    }
    pstChannel->u32MagicNumber = __MI_SYS_CHN_MAGIC_NUM__;
    pstChannel->pu8MMAHeapName = NULL;

    mi_sys_init_allocator_collection(&pstChannel->stAllocatorCollection);
    pstChannel->bChnEnable = FALSE;

    DBG_EXIT_OK();
    return MI_SUCCESS;

Failed_Rel:
    for( u32PortId = 0 ; u32PortId < pstModDev->u32InputPortNum ; u32PortId ++)
    {
        if(pstChannel->pastInputPorts[u32PortId])
             _MI_SYS_IMPL_ReleaseInputPort(pstChannel->pastInputPorts[u32PortId]);
        else
            break;
    }

    for( u32PortId = 0;u32PortId < pstModDev->u32OutputPortNum ; u32PortId ++)
    {
        if( pstChannel->pastOutputPorts[u32PortId])
            _MI_SYS_IMPL_DestroyOutputPort(pstChannel->pastOutputPorts[u32PortId]);
        else
            break;
    }
    memset(pstChannel, 0x25, sizeof(*pstChannel));
    DBG_EXIT_ERR("Create Faild\n");
    return MI_ERR_SYS_NOMEM;
}
void _MI_SYS_IMPL_DeinitChannel( MI_SYS_Channel_t *pstChannel)
{
    MI_U32 u32PortId;
    MI_S32 ret;

    MI_SYS_BUG_ON(!pstChannel);
    MI_SYS_BUG_ON(!pstChannel->pstModDev);
    MI_SYS_BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    for( u32PortId=0 ; u32PortId < pstChannel->pstModDev->u32InputPortNum ; u32PortId ++)
    {
        MI_SYS_BUG_ON(!pstChannel->pastInputPorts[u32PortId]);
        MI_SYS_BUG_ON(pstChannel->pastInputPorts[u32PortId]->pstChannel != pstChannel);
        ret =_MI_SYS_IMPL_ReleaseInputPort(pstChannel->pastInputPorts[u32PortId]);
        if(MI_SUCCESS != ret)
        {
            DBG_ERR("release Input Port(%ul of module%d:%d) fail %d\n",
                u32PortId, pstChannel->pstModDev->u32DevId,
                pstChannel->pstModDev->eModuleId,ret);
            MI_SYS_BUG();
        }
    }

    for( u32PortId = 0;u32PortId < pstChannel->pstModDev->u32OutputPortNum ; u32PortId ++)
    {
        MI_SYS_BUG_ON(!pstChannel->pastOutputPorts[u32PortId]);
        MI_SYS_BUG_ON(pstChannel->pastOutputPorts[u32PortId]->pstChannel != pstChannel);
        ret = _MI_SYS_IMPL_ReleaseOutputPort(pstChannel->pastOutputPorts[u32PortId]);
        if(MI_SUCCESS != ret)
        {
            DBG_ERR("release Output Port(%ul of module%d:%d) fail %d\n",
                u32PortId, pstChannel->pstModDev->u32DevId,
                pstChannel->pstModDev->eModuleId, ret);
            MI_SYS_BUG();
        }
    }

    mi_sys_deinit_allocator_collection(&pstChannel->stAllocatorCollection);
    if(pstChannel->pu8MMAHeapName)
        kfree(pstChannel->pu8MMAHeapName);

    memset(pstChannel, 0x26, sizeof(*pstChannel));
}

MI_S32 _MI_SYS_IMPL_IsOutputPortEnabled(MI_SYS_OutputPort_t *pstOutputPort)
{
    return pstOutputPort->pstChannel->bChnEnable && pstOutputPort->bPortEnable;
}
MI_S32 _MI_SYS_IMPL_IsInputPortEnabled(MI_SYS_InputPort_t *pstInputPort)
{
    DBG_INFO("pstInputPort:%p, %p\n", pstInputPort, pstInputPort->pstChannel);
    return (pstInputPort->pstChannel->bChnEnable && pstInputPort->bPortEnable);
}

void _MI_SYS_IMPL_DupBufrefIntoDrvBkRefQueue(MI_SYS_OutputPort_t *pstOutputPort,MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_BufRef_t *pstNewBufRef;
    MI_S32 isOutputEnabled;

    MI_SYS_BUG_ON(!pstOutputPort ||pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstOutputPort->pstChannel ||pstOutputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufRef);

    isOutputEnabled = _MI_SYS_IMPL_IsOutputPortEnabled(pstOutputPort);

    if(isOutputEnabled && pstOutputPort->u32DrvBkRefFifoDepth)
    {
        pstNewBufRef = mi_sys_dup_bufref(pstBufRef, NULL, NULL);//mi_sys_dup_bufref(pstBufRef, _MI_SYS_IMPL_OutputUsrBufOnBufRefRelFunc, pstOutputPort);
        if(pstNewBufRef)
            mi_sys_add_to_queue_tail(pstNewBufRef, &pstOutputPort->stDrvBkRefFifoQueue);
    }
    while(!isOutputEnabled || (int)pstOutputPort->stDrvBkRefFifoQueue.queue_buf_count>pstOutputPort->u32DrvBkRefFifoDepth)
    {
        MI_SYS_BufRef_t *pstTmpBufRef = mi_sys_remove_from_queue_head(&pstOutputPort->stDrvBkRefFifoQueue);
        if(!pstTmpBufRef)
            break;
        mi_sys_release_bufref(pstTmpBufRef);
    }
}

void _MI_SYS_IMPL_DupBufrefIntoOutputFifoQueue(MI_SYS_OutputPort_t *pstOutputPort,MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_BufRef_t *pstNewBufRef;
    int i_valid_fifo_cnt;

    MI_SYS_BUG_ON(!pstOutputPort ||pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstOutputPort->pstChannel ||pstOutputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufRef);

    i_valid_fifo_cnt = pstOutputPort->u32UsrFifoCount - atomic_read(&pstOutputPort->usrLockedBufCnt);

    if(i_valid_fifo_cnt > 0 && _MI_SYS_IMPL_IsOutputPortEnabled(pstOutputPort))
    {
        pstNewBufRef = mi_sys_dup_bufref(pstBufRef, NULL, NULL);//mi_sys_dup_bufref(pstBufRef, _MI_SYS_IMPL_OutputUsrBufOnBufRefRelFunc, pstOutputPort);
        if(!pstNewBufRef)
        {
            DBG_WRN("mi_sys_dup_bufref_into_output_fifo_queue no mem!\n");
            return;
        }
        mi_sys_add_to_queue_tail(pstNewBufRef, &pstOutputPort->stUsrGetFifoBufQueue);
        pstOutputPort->u64UsrGetFifoBufQueueAddCnt++;
        while((int)pstOutputPort->stUsrGetFifoBufQueue.queue_buf_count>i_valid_fifo_cnt)
        {
            MI_SYS_BufRef_t *pstTmpBufRef = mi_sys_remove_from_queue_head(&pstOutputPort->stUsrGetFifoBufQueue);
            if(!pstTmpBufRef)
                MI_SYS_BUG();
            mi_sys_release_bufref(pstTmpBufRef);
            pstOutputPort->u64UsrGetFifoBufQueueDiscardCnt++;
        }
        _MI_SYS_IMPL_SetPollState(pstOutputPort);
    }
    else
    {
        while(1)
        {
            MI_SYS_BufRef_t *pstTmpBufRef = mi_sys_remove_from_queue_head(&pstOutputPort->stUsrGetFifoBufQueue);
            if(!pstTmpBufRef)
                break;
            mi_sys_release_bufref(pstTmpBufRef);
        }
    }

}

static void _MI_SYS_IMPL_DupBufrefIntoInputBindQueue(MI_SYS_InputPort_t *pstInputPort, MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_BufRef_t *pstNewBufRef = NULL;

    MI_SYS_BUG_ON(!pstInputPort ||pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstInputPort->pstChannel ||pstInputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufRef);

    if(_MI_SYS_IMPL_IsInputPortEnabled(pstInputPort))
    {
        if(!pstInputPort->bEnableLowLatencyReceiveMode || !pstBufRef->bPreNotified)
        {
            if(pstBufRef->bPreNotified == FALSE && pstInputPort->bEnableLowLatencyReceiveMode)
            {
                DBG_WRN("warning !!! enabled LowLatencyReceiveMode for inputport ,but when finish pre-stage outputport,not done PreNotified yet!!!\n");
            }
            pstNewBufRef = mi_sys_dup_bufref(pstBufRef, _MI_SYS_IMPL_InputPortOnBufRefRelFunc, pstInputPort);
            if(!pstNewBufRef)
            {
               DBG_ERR("Dup BufRef Faild\n");
               return;
            }
            mi_sys_add_to_queue_tail(pstNewBufRef,  &pstInputPort->stBindInputBufQueue);
            DBG_INFO("Add To Warking Queue Done, Dup pstNewBufRef:%p, pstBufAllocation:%p\n", pstNewBufRef, pstNewBufRef->pstBufAllocation);
            WAKE_UP_QUEUE_IF_NECESSARY(pstInputPort->pstChannel->pstModDev->inputWaitqueue);
        }
    }
    else
    {
        while(1)
        {
            MI_SYS_BufRef_t *pstTmpBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stBindInputBufQueue);
            if(!pstTmpBufRef)
                break;
            mi_sys_release_bufref(pstTmpBufRef);
        }
    }
}

MI_S32 _MI_SYS_IMPL_InputPortRewindBuf(MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_InputPort_t *pstInputPort = (MI_SYS_InputPort_t*)pstBufRef->pCBData;

    MI_SYS_BUG_ON(!pstInputPort ||pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstInputPort->pstChannel ||pstInputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufRef);

    DBG_INFO("pstBufRef:%p, pstBufAllocation:%p\n", pstBufRef, pstBufRef->pstBufAllocation);
    mi_sys_remove_from_queue(pstBufRef, &(pstInputPort->stWorkingQueue));
    if(_MI_SYS_IMPL_IsInputPortEnabled(pstInputPort))
    {
        if(pstBufRef->bufinfo.bUsrBuf)
        {
            mi_sys_add_to_queue_head(pstBufRef, &pstInputPort->stUsrInjectBufQueue);
        }
        else
        {
            mi_sys_add_to_queue_head(pstBufRef, &pstInputPort->stBindInputBufQueue);
        }
    }
    else
    {
        mi_sys_release_bufref(pstBufRef);
        while(1)
        {
            MI_SYS_BufRef_t *pstTmpBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stUsrInjectBufQueue);
            if(!pstTmpBufRef)
                break;
            mi_sys_release_bufref(pstTmpBufRef);
        }
        while(1)
        {
            MI_SYS_BufRef_t *pstTmpBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stBindInputBufQueue);
            if(!pstTmpBufRef)
                break;
            mi_sys_release_bufref(pstTmpBufRef);
        }
    }
    return MI_SUCCESS;
}
MI_S32 _MI_SYS_IMPL_AddBufrefIntoInputUsrInjectQueue(MI_SYS_InputPort_t *pstInputPort, MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_BufRef_t *pstTmp;
    MI_SYS_BUG_ON(!pstInputPort ||pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstInputPort->pstChannel ||pstInputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufRef);

    if(_MI_SYS_IMPL_IsInputPortEnabled(pstInputPort))
    {
        pstTmp = mi_sys_dup_bufref(pstBufRef, _MI_SYS_IMPL_InputPortOnBufRefRelFunc,pstInputPort);
        if(pstTmp)
        {
            mi_sys_add_to_queue_tail(pstTmp, &pstInputPort->stUsrInjectBufQueue);
        }
        else
        {
            DBG_WRN("Dup BufRef Faild\n");
        }

        WAKE_UP_QUEUE_IF_NECESSARY(pstInputPort->pstChannel->pstModDev->inputWaitqueue);
    }
    else
    {
        while(1)
        {
            MI_SYS_BufRef_t *pstTmpBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stUsrInjectBufQueue);
            if(!pstTmpBufRef)
                break;
            mi_sys_release_bufref(pstTmpBufRef);
        }
    }
    return MI_SUCCESS;
}


MI_SYS_Allocator_Collection_t g_stGlobalVBPOOLAllocatorCollection;

int _MI_SYS_IMPL_RcAcceptBuf(MI_SYS_InputPort_t *pstCurInputPort)
{
    DBG_INFO("pstCurInputPort:%p 0x%x\n", pstCurInputPort, pstCurInputPort->u32MagicNumber);
    MI_SYS_BUG_ON(!pstCurInputPort);
    MI_SYS_BUG_ON(pstCurInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

    if(!_MI_SYS_IMPL_IsInputPortEnabled(pstCurInputPort))
        return 0;

    if(pstCurInputPort->u32SrcFrmrate == pstCurInputPort->u32DstFrmrate)
        return 1;

    if(pstCurInputPort->u32CurRcParam>=pstCurInputPort->u32SrcFrmrate)
        return 1;

    return 0;
}
void _MI_SYS_IMPL_StepRc(MI_SYS_InputPort_t *pstCurInputPort)
{
    MI_SYS_BUG_ON(!pstCurInputPort);

    if(pstCurInputPort->u32SrcFrmrate == pstCurInputPort->u32DstFrmrate)
        return;
    if(pstCurInputPort->u32CurRcParam >= pstCurInputPort->u32SrcFrmrate)
          pstCurInputPort->u32CurRcParam -= pstCurInputPort->u32SrcFrmrate;
    pstCurInputPort->u32CurRcParam+= pstCurInputPort->u32DstFrmrate;
}
MI_S32 _MI_SYS_IMPL_OutputPortFinishBuf(MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_OutputPort_t *pstOutputPort = (MI_SYS_OutputPort_t*)pstBufRef->pCBData;
    MI_SYS_InputPort_t *pstCurInputPort = NULL;
    struct list_head *pos;

    MI_SYS_BUG_ON(!pstOutputPort);
    MI_SYS_BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

    DBG_INFO("Finish pstBufAllocation:%p\n", pstBufRef->pstBufAllocation);

    if(pstBufRef->bPreNotified == FALSE)
        MI_SYS_BUG_ON(atomic_read(&pstBufRef->pstBufAllocation->ref_cnt) !=1);
    else
        MI_SYS_BUG_ON(atomic_read(&pstBufRef->pstBufAllocation->ref_cnt) < 1);

    mi_sys_remove_from_queue(pstBufRef, &pstOutputPort->stWorkingQueue);

    if(!MI_SYS_SIDEBAND_MSG_ACKED(pstBufRef->bufinfo.u64SidebandMsg))
    {
        //buffer provider doesn't provide response to sideband msg
        pstBufRef->bufinfo.u64SidebandMsg = MI_SYS_SIDEBAND_MSG_NULL;
    }

    _MI_SYS_IMPL_DupBufrefIntoOutputFifoQueue(pstOutputPort, pstBufRef);

    _MI_SYS_IMPL_DupBufrefIntoDrvBkRefQueue(pstOutputPort, pstBufRef);

    down(&pstOutputPort->stBindedInputListSemlock);
    list_for_each(pos, &pstOutputPort->stBindPeerInputPortList)
    {
        pstCurInputPort = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
        MI_SYS_BUG_ON(pstCurInputPort->pstBindPeerOutputPort != pstOutputPort);

        if(_MI_SYS_IMPL_RcAcceptBuf(pstCurInputPort))
        {
            _MI_SYS_IMPL_DupBufrefIntoInputBindQueue(pstCurInputPort, pstBufRef);
        }

        _MI_SYS_IMPL_StepRc(pstCurInputPort);
    }
    up(&pstOutputPort->stBindedInputListSemlock);

    if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_RAW)
    {
        if ((pstBufRef->u32ExtFlag & MI_SYS_MAP_CPU_WRITE) && (pstBufRef->bufinfo.stRawData.u32ContentSize != 0))
            MI_SYS_IMPL_FlushInvCache(pstBufRef->bufinfo.stRawData.pVirAddr, pstBufRef->bufinfo.stRawData.u32ContentSize);
    }
    else if (pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_META)
    {
        //do nothing.
    }
    else
    {
        if ((pstBufRef->u32ExtFlag & MI_SYS_MAP_CPU_WRITE) && pstBufRef->bufinfo.stFrameData.u32BufSize)
           MI_SYS_IMPL_FlushInvCache(pstBufRef->bufinfo.stFrameData.pVirAddr[0], pstBufRef->bufinfo.stFrameData.u32BufSize);
    }

    mi_sys_release_bufref(pstBufRef);

    return MI_SUCCESS;
}

MI_S32 _MI_SYS_IMPL_OutputPortFinishBuf_LL(MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_OutputPort_t *pstOutputPort = (MI_SYS_OutputPort_t*)pstBufRef->pCBData;
    
    
    MI_SYS_BUG_ON(!pstOutputPort);
    MI_SYS_BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

    DBG_INFO("Finish pstBufAllocation:%p\n", pstBufRef->pstBufAllocation);

    if(pstBufRef->bPreNotified == FALSE)
        MI_SYS_BUG_ON(atomic_read(&pstBufRef->pstBufAllocation->ref_cnt) !=1);
    else
        MI_SYS_BUG_ON(atomic_read(&pstBufRef->pstBufAllocation->ref_cnt) < 1);

    mi_sys_remove_from_queue(pstBufRef, &pstOutputPort->stWorkingQueue);

    mi_sys_release_bufref(pstBufRef);

    return MI_SUCCESS;
}



MI_S32 _MI_SYS_IMPL_InputPortFinishBuf(MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_InputPort_t *pstInputPort = (MI_SYS_InputPort_t*)pstBufRef->pCBData;

    MI_SYS_BUG_ON(!pstInputPort);
    DBG_INFO("pstBufAllocation:%p\n", pstBufRef->pstBufAllocation);
    mi_sys_remove_from_queue(pstBufRef, &pstInputPort->stWorkingQueue);

    if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_RAW)
    {
        if ((pstBufRef->u32ExtFlag & MI_SYS_MAP_CPU_WRITE) && (pstBufRef->bufinfo.stRawData.u32ContentSize != 0))
            MI_SYS_IMPL_FlushInvCache(pstBufRef->bufinfo.stRawData.pVirAddr, pstBufRef->bufinfo.stRawData.u32ContentSize);
    }
    else if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_META)
    {
        //do nothing.
    }
    else
    {
        if ((pstBufRef->u32ExtFlag & MI_SYS_MAP_CPU_WRITE) && pstBufRef->bufinfo.stFrameData.u32BufSize)
           MI_SYS_IMPL_FlushInvCache(pstBufRef->bufinfo.stFrameData.pVirAddr[0], pstBufRef->bufinfo.stFrameData.u32BufSize);
    }

    mi_sys_release_bufref(pstBufRef);

    return MI_SUCCESS;
}

MI_S32 _MI_SYS_IMPL_OutputPortRewindBuf(MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_OutputPort_t *pstOutputPort = (MI_SYS_OutputPort_t*)pstBufRef->pCBData;

    MI_SYS_BUG_ON(!pstBufRef ||!pstBufRef->pstBufAllocation);
    MI_SYS_BUG_ON(pstBufRef->pstBufAllocation->ops.OnRelease != _MI_SYS_IMPL_Ouputport_wrapper_allocation_on_release);
    pstOutputPort = (MI_SYS_OutputPort_t*)pstBufRef->pCBData;

    MI_SYS_BUG_ON(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

    MI_SYS_BUG_ON(pstBufRef->pstBufAllocation->private1 == NULL);
    pstBufRef->pstBufAllocation->ops.OnRelease = (AllocationOnReleaseCB)pstBufRef->pstBufAllocation->private1;
    pstBufRef->pstBufAllocation->private1 = NULL;
    pstBufRef->pstBufAllocation->private2 = NULL;

    atomic_dec_return(&pstOutputPort->totalOutputPortInUsedBuf);

    mi_sys_remove_from_queue(pstBufRef, &pstOutputPort->stWorkingQueue);

    mi_sys_release_bufref(pstBufRef);

    return MI_SUCCESS;
}

static inline MI_S32 _MI_SYS_IMPL_CopyBufinfoFromUsr(MI_SYS_BufInfo_t *pstInternalBufInfo, MI_SYS_BufInfo_t *pstUsrBufInfo)
{
    pstInternalBufInfo->u64Pts = pstUsrBufInfo->u64Pts;
    pstInternalBufInfo->bEndOfStream = pstUsrBufInfo->bEndOfStream;

    switch(pstInternalBufInfo->eBufType)
    {
        case E_MI_SYS_BUFDATA_RAW:
        if(pstUsrBufInfo->stRawData.u32ContentSize >  pstInternalBufInfo->stRawData.u32BufSize)
        {
            DBG_WRN("Usr BufInfo invalid\n");
            return MI_ERR_SYS_ILLEGAL_PARAM;
        }
        pstInternalBufInfo->stRawData.u32ContentSize =
                min(pstUsrBufInfo->stRawData.u32ContentSize, pstInternalBufInfo->stRawData.u32BufSize);
            break;
        case E_MI_SYS_BUFDATA_META:
            //Doesn't accept any changes from user mode!!!
            DBG_WRN(" should not arrive here!");
            break;
        case E_MI_SYS_BUFDATA_FRAME:
            pstInternalBufInfo->stFrameData.eCompressMode = pstUsrBufInfo->stFrameData.eCompressMode;
            pstInternalBufInfo->stFrameData.eFieldType = pstUsrBufInfo->stFrameData.eFieldType;
            pstInternalBufInfo->stFrameData.eTileMode = pstUsrBufInfo->stFrameData.eTileMode;
            break;
        default:
            MI_SYS_BUG();
    }
   return MI_SUCCESS;
}

MI_SYS_BufRef_t *_MI_SYS_IMPL_AllocBufDefaultPolicy(struct MI_SYS_Channel_s *pstChannel, struct MI_SYS_BufConf_s *pstBufConfig,
                                                                                            OnBufRefRelFunc onRelCB, void *pCBData)
{
    MI_SYS_BufferAllocation_t *pBufAllocation;
    MI_SYS_BufRef_t *pstBufRef;
    MI_SYS_MOD_DEV_t *pstModDev;

    MI_SYS_BUG_ON(!pstChannel || pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufConfig);

    pstModDev = pstChannel->pstModDev;
    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);


    pBufAllocation = mi_sys_alloc_from_allocator_collection(&pstModDev->stAllocatorCollection, pstBufConfig);
    if(!pBufAllocation)
    {
        pBufAllocation = mi_sys_alloc_from_allocator_collection(&g_stGlobalVBPOOLAllocatorCollection, pstBufConfig);
    }
    if(!pBufAllocation)
    {
        pBufAllocation = mi_sys_alloc_from_mma_allocators(pstChannel->pu8MMAHeapName, pstBufConfig);
    }
    if(!pBufAllocation)
    {
        DBG_WRN("Can't Find Buf Allocation Error\n");
        return NULL;
    }

    pstBufRef = mi_sys_create_bufref(pBufAllocation,pstBufConfig ,onRelCB, pCBData);
    if(pstBufRef == NULL)
    {
        pBufAllocation->ops.OnRelease(pBufAllocation);
        DBG_ERR("Create BufRef Faild\n");
        return NULL;
    }

    DBG_EXIT_OK();
    MI_SYS_BUG_ON(atomic_read(&pstBufRef->pstBufAllocation->ref_cnt) !=1);
    return pstBufRef;
}

MI_SYS_InputPort_t *_MI_SYS_IMPL_GetInputPortInfo(MI_SYS_ChnPort_t *pstChnPort)
{
    MI_SYS_MOD_DEV_t *pstModDev;
    struct list_head *pos;
    MI_SYS_InputPort_t *pstInputPort = NULL;

    MI_SYS_BUG_ON(pstChnPort == NULL);
    down(&mi_sys_global_dev_list_semlock);
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);
        MI_SYS_BUG_ON(pstModDev->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
        MI_SYS_BUG_ON(pstModDev->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);
        //MI_SYS_BUG_ON(pstModDev->u32InputPortNum ==0 && pstModDev->u32OutputPortNum ==0);
        //MI_SYS_BUG_ON(pstModDev->u32DevChnNum ==0);
        MI_SYS_BUG_ON(pstModDev->u32DevChnNum > MI_SYS_MAX_DEV_CHN_CNT);
        DBG_INFO(" found pstModDev[%d,%u] ,   want pstChnPort[%d,%u]\n",pstModDev->eModuleId,pstModDev->u32DevId,pstChnPort->eModId,pstChnPort->u32DevId);
        if(pstModDev->eModuleId == pstChnPort->eModId && pstModDev->u32DevId == pstChnPort->u32DevId)
        {
            if(pstChnPort->u32PortId >= pstModDev->u32InputPortNum)
                break;
            if(pstChnPort->u32ChnId >= pstModDev->u32DevChnNum)
                break;
            pstInputPort = pstModDev->astChannels[pstChnPort->u32ChnId].pastInputPorts[pstChnPort->u32PortId];
                break;
        }
    }
    if(pstInputPort == NULL)
    {
        DBG_ERR("not found [eModId %d,u32DevId %u]\n",pstChnPort->eModId,pstChnPort->u32DevId);
    }
    up(&mi_sys_global_dev_list_semlock);
    return pstInputPort;
}

MI_SYS_OutputPort_t *_MI_SYS_IMPL_GetOutputPortInfo(MI_SYS_ChnPort_t *pstChnPort)
{
    MI_SYS_MOD_DEV_t *pstModDev = NULL;
    struct list_head *pos = NULL;
    MI_SYS_OutputPort_t *pstOutputPort = NULL;

    down(&mi_sys_global_dev_list_semlock);
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);
        MI_SYS_BUG_ON(pstModDev->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
        MI_SYS_BUG_ON(pstModDev->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);
        //MI_SYS_BUG_ON(pstModDev->u32InputPortNum ==0 && pstModDev->u32OutputPortNum ==0);
        //MI_SYS_BUG_ON(pstModDev->u32DevChnNum ==0);
        MI_SYS_BUG_ON(pstModDev->u32DevChnNum > MI_SYS_MAX_DEV_CHN_CNT);

        if(pstModDev->eModuleId == pstChnPort->eModId && pstModDev->u32DevId == pstChnPort->u32DevId)
        {
            if(pstChnPort->u32PortId >= pstModDev->u32OutputPortNum)
               break;
            if(pstChnPort->u32ChnId >= pstModDev->u32DevChnNum)
                break;
            pstOutputPort = pstModDev->astChannels[pstChnPort->u32ChnId].pastOutputPorts[pstChnPort->u32PortId];
            break;
        }
    }
    up(&mi_sys_global_dev_list_semlock);

    return pstOutputPort;
}

MI_SYS_BufRef_t *_MI_SYS_IMPL_UserGetOutputPortBuf(MI_SYS_ChnPort_t *pstChnPort)
{
    MI_SYS_OutputPort_t *pstOutputPort = NULL;
    MI_SYS_BufRef_t *pstBufRef = NULL;
    MI_SYS_BufRef_t *pstRetBufRef = NULL;

    pstOutputPort = _MI_SYS_IMPL_GetOutputPortInfo(pstChnPort);

    if (!pstOutputPort)
        return NULL;

    MI_SYS_BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

    if(!_MI_SYS_IMPL_IsOutputPortEnabled(pstOutputPort))
        return NULL;

    pstBufRef = mi_sys_remove_from_queue_head(&pstOutputPort->stUsrGetFifoBufQueue);
    if(pstBufRef)
    {
        pstRetBufRef = mi_sys_dup_bufref(pstBufRef, _MI_SYS_IMPL_OutputUsrBufOnBufRefRelFunc, pstOutputPort);//mi_sys_dup_bufref(pstBufRef, _MI_SYS_IMPL_OutputUsrBufOnBufRefRelFunc, pstOutputPort);
        if(pstRetBufRef)
            atomic_inc(&pstOutputPort->usrLockedBufCnt);

        mi_sys_release_bufref(pstBufRef);
    }
    return pstRetBufRef;
}

MI_S32 _MI_SYS_IMPL_MemCacheCreate(void)
{
    mi_sys_bufref_cachep = kmem_cache_create("mi_sys_bufref_cachep", sizeof(MI_SYS_BufRef_t), 0,SLAB_HWCACHE_ALIGN,NULL);
    mi_sys_inputport_cachep = kmem_cache_create("mi_sys_inputport_cachep", sizeof(MI_SYS_InputPort_t),0,SLAB_HWCACHE_ALIGN,NULL);
    mi_sys_outputport_cachep = kmem_cache_create("mi_sys_outputport_cachep", sizeof(MI_SYS_OutputPort_t),0,SLAB_HWCACHE_ALIGN,NULL);
    mi_sys_taskinfo_cachep = kmem_cache_create("mi_sys_taskinfo_cachep", sizeof(mi_sys_ChnTaskInfo_t),0,SLAB_HWCACHE_ALIGN,NULL);
    mi_sys_mma_allocation_cachep = kmem_cache_create("mi_sys_mma_allocation_cachep", sizeof(mi_sys_mma_buf_allocation_t),0,SLAB_HWCACHE_ALIGN,NULL);
    mi_sys_meta_allocation_cachep = kmem_cache_create("mi_sys_meta_allocation_cachep", sizeof(mi_sys_meta_buf_allocation_t),0,SLAB_HWCACHE_ALIGN,NULL);
    g_mi_sys_chunk_cachep = kmem_cache_create("g_mi_sys_chunk_cachep", sizeof(mi_sys_chunk_t),0,SLAB_HWCACHE_ALIGN,NULL);
    mi_sys_vbpool_allocation_cachep = kmem_cache_create("mi_sys_vbpool_allocation_cachep", sizeof(mi_sys_vbpool_allocation_t),0,SLAB_HWCACHE_ALIGN,NULL);

    DBG_INFO("%p, %p, %p, %p\n", mi_sys_bufref_cachep, mi_sys_inputport_cachep, mi_sys_outputport_cachep, mi_sys_taskinfo_cachep);
    MI_SYS_BUG_ON(!(mi_sys_bufref_cachep && mi_sys_inputport_cachep && mi_sys_outputport_cachep && mi_sys_taskinfo_cachep));

    DBG_INFO("%p, %p, %p, %p\n", mi_sys_mma_allocation_cachep, mi_sys_meta_allocation_cachep, g_mi_sys_chunk_cachep, mi_sys_vbpool_allocation_cachep);
    MI_SYS_BUG_ON(!(mi_sys_mma_allocation_cachep && mi_sys_meta_allocation_cachep && g_mi_sys_chunk_cachep && mi_sys_vbpool_allocation_cachep));

    return MI_SUCCESS;
}

MI_S32 _MI_SYS_IMPL_MemCacheDestroy(void)
{
    kmem_cache_destroy(mi_sys_bufref_cachep);
    mi_sys_bufref_cachep = NULL;

    kmem_cache_destroy(mi_sys_inputport_cachep);
    mi_sys_inputport_cachep = NULL;

    kmem_cache_destroy(mi_sys_outputport_cachep);
    mi_sys_outputport_cachep =NULL;

    kmem_cache_destroy(mi_sys_taskinfo_cachep);
    mi_sys_taskinfo_cachep =NULL;

    kmem_cache_destroy(mi_sys_mma_allocation_cachep);
    mi_sys_mma_allocation_cachep = NULL;

    kmem_cache_destroy(mi_sys_meta_allocation_cachep);
    mi_sys_meta_allocation_cachep =NULL;

    kmem_cache_destroy(g_mi_sys_chunk_cachep);
    g_mi_sys_chunk_cachep = NULL;

    kmem_cache_destroy(mi_sys_vbpool_allocation_cachep);
    mi_sys_vbpool_allocation_cachep = NULL;

    return MI_SUCCESS;
}

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
#define INVALID_PHY_ADDR 0xffffffff
#endif

static MI_S32  _MI_SYS_IMPL_MmaHeapInit(void)
{
    int j,k;
    mi_sys_mma_allocator_t * pmma_allocator =NULL;
    for(j=0;(j<mstar_driver_boot_mma_buffer_num) ;j++)
    {
        if(mma_config[j].size !=0)//inside for
        {
        #if 1
            for(k=0;k<strlen(mma_config[j].name);k++)
            {
                printk("%c",mma_config[j].name[k]);
            }
            printk("    miu=%d,sz=%lx  reserved_start=%p\n", mma_config[j].miu, mma_config[j].size, (void*)mma_config[j].reserved_start);
            #endif

            if(INVALID_PHY_ADDR == mma_config[j].reserved_start)
            {
                //in kernel code,memblock_find_in_range fail not MI_SYS_BUG_ON(1),but many printk,
                //here also check this.
                printk("fail,error,reserved size is not 0,but reserved_start is INVALID_PHY_ADDR,may memblock_find_in_range fail\n");
                MI_SYS_BUG_ON(1);
    			#if 0
    			printk("=======%s: %d fail,error========================",__FUNCTION__,__LINE__);
    			printk("%s: %d fail,error  mma_config reserved_start INVALID",__FUNCTION__,__LINE__);
    			printk("%s: %d fail,error  mma_config reserved_start INVALID",__FUNCTION__,__LINE__);
    			printk("%s: %d fail,error  mma_config reserved_start INVALID",__FUNCTION__,__LINE__);
    			printk("%s: %d fail,error  mma_config reserved_start INVALID",__FUNCTION__,__LINE__);
    			printk("%s: %d fail,error  mma_config reserved_start INVALID",__FUNCTION__,__LINE__);
    			printk("%s: %d fail,error  mma_config reserved_start INVALID",__FUNCTION__,__LINE__);
    			printk("=======%s: %d fail,error========================",__FUNCTION__,__LINE__);
    			#endif
                return MI_ERR_SYS_FAILED;
            }
            MI_SYS_BUG_ON(mma_config[j].reserved_start%PAGE_SIZE);

            #if LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            if(mma_config[j].max_offset_to_curr_lx_mem && mma_config[j].max_offset_to_curr_lx_mem != -1UL)
            {
                mma_config_exist_max_offset_to_curr_lx_mem = TRUE;
            }
            #endif

            pmma_allocator = mi_sys_mma_allocator_create(mma_config[j].name, mma_config[j].reserved_start, mma_config[j].size);
            if(pmma_allocator)
            {
                /*
                    pmma_allocator->heap_base_cpu_bus_addr should use %llx ,do not use %lx,
                    or will get wrong value.
                    in MI,now MI_PHY is 64 ,but in kernel
                    #ifdef CONFIG_PHYS_ADDR_T_64BIT
                    typedef u64 phys_addr_t;
                    #else
                    typedef u32 phys_addr_t;
                    #endif
                    and for K6L ,CONFIG_PHYS_ADDR_T_64BIT is not enable
                */
                #if 1
                printk("mi_sys_mma_allocator_create success, heap_base_addr=%llx length=%lx \n",pmma_allocator->heap_base_cpu_bus_addr,pmma_allocator->length);
                #endif
            }
            else
            {
                #if 1
                DBG_ERR("mi_sys_mma_allocator_create fail \n");
                #endif
                return MI_ERR_SYS_NOMEM;
            }
        }
    }
    return MI_SUCCESS;
}

static MI_S32  _MI_SYS_IMPL_MmaHeapExit(void)
{
    int ret;
    int ref_cnt;
    mi_sys_mma_allocator_t *pst_mma_allocator = NULL;
    struct list_head *pos, *q;

    down(&g_mma_list_semlock);
    list_for_each_safe(pos, q, &g_mma_heap_list)
    {
        pst_mma_allocator = list_entry(pos, mi_sys_mma_allocator_t, list);
        MI_SYS_BUG_ON(pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
        MI_SYS_BUG_ON(list_empty(&pst_mma_allocator->list));///must be in g_mma_heap_list
        list_del(&pst_mma_allocator->list);
        INIT_LIST_HEAD(&pst_mma_allocator->list);
        break;
    }
    up(&g_mma_list_semlock);

    if(pst_mma_allocator != NULL)
    {
        down(&pst_mma_allocator->semlock);
        mi_sys_deinit_chunk(&pst_mma_allocator->chunk_mgr);
        up(&pst_mma_allocator->semlock);
        ref_cnt = atomic_dec_return(&pst_mma_allocator->stdAllocator.ref_cnt);//do not use pst_mma_allocator->stdAllocator.ops->OnUnref
        MI_SYS_BUG_ON(ref_cnt != 0);
        ret = addKRange(pst_mma_allocator->heap_base_cpu_bus_addr,  pst_mma_allocator->length);
        MI_SYS_BUG_ON(ret != MI_SUCCESS);
        memset(pst_mma_allocator, 0x36, sizeof(*pst_mma_allocator));
        kfree(pst_mma_allocator);
    }
    return MI_SUCCESS;
}

MI_SYS_BufRef_t *_MI_SYS_IMPL_UserGetInputPortBuf(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_BufConf_t *pstBufConfig , MI_S32 s32TimeOutMs)
{
    MI_SYS_InputPort_t *pstInputPort = NULL;
    MI_SYS_BufferAllocation_t *pBufAllocation = NULL;
    MI_SYS_BufRef_t *pstBufRef = NULL;
    MI_BOOL bHaveInputBuf = FALSE;

    DBG_ENTER();

    pstInputPort = _MI_SYS_IMPL_GetInputPortInfo(pstChnPort);
    if (!pstInputPort)
    {
        DBG_ERR("not found InputPort in [eModId %d,u32DevId %u, u32ChnId %u]\n",pstChnPort->eModId,pstChnPort->u32DevId, pstChnPort->u32ChnId);
        return NULL;
    }

    MI_SYS_BUG_ON(!pstInputPort || pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
    if(!_MI_SYS_IMPL_IsInputPortEnabled(pstInputPort))
    {
        DBG_WRN("Input port(%d , %d , %d) is diabled\n",pstInputPort->pstChannel->pstModDev->eModuleId,
            pstInputPort->pstChannel->u32ChannelId , pstInputPort->u32PortId);
        return NULL;
    }

    if(s32TimeOutMs < 0)
        s32TimeOutMs = 20;

    wait_event_timeout(pstInputPort->inputBufCntWaitqueue,
        atomic_read(&pstInputPort->usrLockedInjectBufCnt)+pstInputPort->stUsrInjectBufQueue.queue_buf_count< MI_SYS_USR_INJECT_BUF_CNT_QUOTA_DEFAULT, msecs_to_jiffies(s32TimeOutMs));

    bHaveInputBuf = atomic_read(&pstInputPort->usrLockedInjectBufCnt)+pstInputPort->stUsrInjectBufQueue.queue_buf_count
                < MI_SYS_USR_INJECT_BUF_CNT_QUOTA_DEFAULT;

    if(!bHaveInputBuf)
    {
        //DBG_WRN("Input port(module : %d , Chn : %d , Port : %d) get too much input buf(%d)\n",pstInputPort->pstChannel->pstModDev->eModuleId,
        //    pstInputPort->pstChannel->u32ChannelId , pstInputPort->u32PortId, atomic_read(&pstInputPort->usrLockedInjectBufCnt) );
        return NULL;
    }

    if(pstBufConfig->eBufType == E_MI_SYS_BUFDATA_FRAME)
        pstBufConfig->stFrameCfg.stFrameBufExtraConf = pstInputPort->stInputBufExtraConf;

    if(pstInputPort->pstCusBufAllocator)
    {
        pBufAllocation = pstInputPort->pstCusBufAllocator->ops->alloc(pstInputPort->pstCusBufAllocator, pstBufConfig);
        if(!pBufAllocation)
            return NULL;
        pstBufRef = mi_sys_create_bufref(pBufAllocation, pstBufConfig ,_MI_SYS_IMPL_UserPutInputPortBuf_Ref_CB,pstInputPort);
        if(pstBufRef == NULL)
        {
            pstInputPort->pstCusBufAllocator->ops->OnRelease(pstInputPort->pstCusBufAllocator);
            return NULL;
        }
    }
    if(!pstBufRef)
        pstBufRef = _MI_SYS_IMPL_AllocBufDefaultPolicy(pstInputPort->pstChannel, pstBufConfig, _MI_SYS_IMPL_UserPutInputPortBuf_Ref_CB, pstInputPort);
    if(pstBufRef)
    {
        MI_SYS_BUG_ON(pstBufRef->onRelCB != _MI_SYS_IMPL_UserPutInputPortBuf_Ref_CB);
        atomic_inc(&pstInputPort->usrLockedInjectBufCnt);
    }
    return pstBufRef;
}


static int _MI_SYS_IMPL_ANON_MMap(struct file *filp, struct vm_area_struct *vma)
{
    mi_sys_mmap_data_t *pst_mmap_data;
    unsigned long vm_len;
    MI_SYS_BUG_ON(!filp);
    MI_SYS_BUG_ON(!vma);

    DBG_ENTER();
    pst_mmap_data = (mi_sys_mmap_data_t *)(filp->private_data);

    vm_len = vma->vm_end - vma->vm_start;

    if(vm_len >> PAGE_SHIFT > pst_mmap_data->pages)
        return  -EINVAL;

    vma->vm_pgoff = 0;
    if(pst_mmap_data->bCaChe)
        vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
    else
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    io_remap_pfn_range(vma, vma->vm_start, pst_mmap_data->pfn, vm_len, vma->vm_page_prot);
    return 0;
}

static int _MI_SYS_IMPL_ANON_Release(struct inode *inode, struct file *filp)
{
    mi_sys_mmap_data_t *pst_mmap_data;
    MI_SYS_BUG_ON(!filp);
    pst_mmap_data = (mi_sys_mmap_data_t *)(filp->private_data);
    kfree(pst_mmap_data);
    return 0;
}

/*static */struct file_operations msys_api_mmap_fops = {
    .owner        = THIS_MODULE,
    .mmap        =  _MI_SYS_IMPL_ANON_MMap,
    .release = _MI_SYS_IMPL_ANON_Release,
};

//////////////////////////////////////////////
////////////public api//////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

#if 0
void dump_tread()
{
     whil(1)
    {
        struct task_struct *g, *p;
        msleep(60000);//60 secs
        rcu_read_lock();
        do_each_thread(g, p)
        {
            show_stack(p,NULL);
        } while_each_thread(g, p);
        rcu_read_unlock();
    }
}
#endif

int check_pstBufRef (const char *func, int line,
                    MI_SYS_BufRef_t *pstBufRef)
{
    int flag = 0;

    /* TODO: fix here */
    if (pstBufRef->bufinfo.eBufType > 2 || pstBufRef->pstBufAllocation->stBufInfo.eBufType > 2) flag = 1;
    /*
     * if ((unsigned long)pstBufRef->pstBufAllocation->ops.OnRef < 0x7f000000 ||
     *     (unsigned long)pstBufRef->pstBufAllocation->ops.OnRelease < 0x7f000000 ||
     *     (unsigned long)pstBufRef->pstBufAllocation->ops.OnUnref < 0x7f000000 ||
     *     (unsigned long)pstBufRef->pstBufAllocation->ops.map_user < 0x7f000000 ||
     *     (unsigned long)pstBufRef->pstBufAllocation->ops.unmap_user < 0x7f000000 ||
     *     (unsigned long)pstBufRef->pstBufAllocation->ops.vmap_kern < 0x7f000000 ||
     *     (unsigned long)pstBufRef->pstBufAllocation->ops.vunmap_kern < 0x7f000000)
     *     flag = 2;
     */

    if (flag) {
        DBG_WRN("check_pstBufRef_error_flag:%d\n", flag);

        DBG_WRN("%s:%d pstBufRef bufinfo:%p [eBufType:%d phyAddr:%llx pVirAddr:%llx u32BufSize:%d u32ContentSize:%d stRawData.pVirAddr:%d PAGE_OFFSET:%d]\n",
                func, line, &pstBufRef->bufinfo,
                pstBufRef->bufinfo.eBufType, pstBufRef->bufinfo.stRawData.phyAddr,
                pstBufRef->bufinfo.stRawData.pVirAddr, pstBufRef->bufinfo.stRawData.u32BufSize,
                pstBufRef->bufinfo.stRawData.u32ContentSize,pstBufRef->bufinfo.stRawData.pVirAddr, (unsigned long )PAGE_OFFSET);

        DBG_WRN("%s:%d pstBufAllocation:%p ops[OnRef:%p OnRelease:%p OnUnref:%p map_user:%p unmap_user:%p vmap_kern:%p vunmap_kern:%p]\n",
                func, line, pstBufRef->pstBufAllocation,
                pstBufRef->pstBufAllocation->ops.OnRef,
                pstBufRef->pstBufAllocation->ops.OnRelease,
                pstBufRef->pstBufAllocation->ops.OnUnref,
                pstBufRef->pstBufAllocation->ops.map_user,
                pstBufRef->pstBufAllocation->ops.unmap_user,
                pstBufRef->pstBufAllocation->ops.vmap_kern,
                pstBufRef->pstBufAllocation->ops.vunmap_kern);

        DBG_WRN("%s:%d pstBufRef->pstBufAllocation bufinfo:%p [eBufType:%d phyAddr:%llx pVirAddr:%llx u32BufSize:%d u32ContentSize:%d]\n",
                func, line, &pstBufRef->pstBufAllocation->stBufInfo,
                pstBufRef->pstBufAllocation->stBufInfo.eBufType, pstBufRef->pstBufAllocation->stBufInfo.stRawData.phyAddr,
                pstBufRef->pstBufAllocation->stBufInfo.stRawData.pVirAddr, pstBufRef->pstBufAllocation->stBufInfo.stRawData.u32BufSize,
                pstBufRef->pstBufAllocation->stBufInfo.stRawData.u32ContentSize);

        DBG_WRN("%s:%d pstBufRef->pstBufAllocation bufinfo:%p [eBufType:%d phyAddr:%llx pVirAddr:%llx u32BufSize:%d u32ContentSize:%d]\n",
                func, line, &pstBufRef->pstBufAllocation->stBufInfo,
                pstBufRef->pstBufAllocation->stBufInfo.eBufType, pstBufRef->pstBufAllocation->stBufInfo.stRawData.phyAddr,
                pstBufRef->pstBufAllocation->stBufInfo.stRawData.pVirAddr, pstBufRef->pstBufAllocation->stBufInfo.stRawData.u32BufSize,
                pstBufRef->pstBufAllocation->stBufInfo.stRawData.u32ContentSize);

        return flag;
    }
    return 0;
}

void dump_mi_sys_bufinfo(void)
{
    MI_SYS_MOD_DEV_t *pstModDev = NULL;
    struct list_head *pos = NULL;
    MI_U32 u32ChnId = 0 , u32PortId = 0;

    down(&mi_sys_global_dev_list_semlock);
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);

        printk("=============eModuleId %d==============\n",pstModDev->eModuleId);
        for(u32ChnId = 0  ; u32ChnId < pstModDev->u32DevChnNum ; u32ChnId ++)
        {
            if(pstModDev->astChannels[u32ChnId].bChnEnable)
            {
                printk("===========Channel Id(%d)===========\n",u32ChnId);
                for(u32PortId = 0  ; u32PortId < pstModDev->u32InputPortNum ; u32PortId ++)
                {
                    MI_SYS_InputPort_t *pstInputPort = pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
                    if(pstInputPort && pstInputPort->bPortEnable)
                    {
                        printk("===========Input port(%d)===========\n",u32PortId);
                        printk("BindInputBufQueueCount:%d  UsrInjectBufQueueCount:%d ,WorkingQueueCount:%d ,usrLockedInjectBufCnt:%d\n",
                        pstInputPort->stBindInputBufQueue.queue_buf_count,
                        pstInputPort->stUsrInjectBufQueue.queue_buf_count,
                        pstInputPort->stWorkingQueue.queue_buf_count,
                        atomic_read(&pstInputPort->usrLockedInjectBufCnt));
                    }
                }
                for(u32PortId = 0  ; u32PortId < pstModDev->u32OutputPortNum ; u32PortId ++)
                {
                    MI_SYS_OutputPort_t *pstOutputPort = pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
                    if(pstOutputPort && pstOutputPort->bPortEnable)
                    {
                        printk("===========Output port(%d)===========\n",u32PortId);
                        printk("UsrGetFifoBufQueueCount:%d  WorkingQueueCount:%d , totalOutputPortInUsedBufCount:%d ,usrLockedBufCnt:%d\n",
                        pstOutputPort->stUsrGetFifoBufQueue.queue_buf_count,
                        pstOutputPort->stWorkingQueue.queue_buf_count,
                        atomic_read(&pstOutputPort->totalOutputPortInUsedBuf),
                        atomic_read(&pstOutputPort->usrLockedBufCnt));

                    }
                }

            }
        }
    }
    up(&mi_sys_global_dev_list_semlock);
}

#define CMD_LINE_LETH 512
static char* parese_Cmdline(char* str)
{
    struct file* filp = NULL;
    int ret = 0;
    unsigned int n = 0;
    mm_segment_t fs;
    char* pCmdline = NULL;
    char cmdline[CMD_LINE_LETH];
    static char* pCmd_Section = NULL;

    filp = filp_open("/proc/cmdline", O_RDONLY, 0644);
    if (IS_ERR_OR_NULL(filp))
    {
        printk(KERN_ERR "\33[0;36m read /proc/cmdline failed!\n");
        return NULL;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    filp->f_op->llseek(filp, 0, SEEK_SET);

    ret = vfs_read(filp, cmdline, CMD_LINE_LETH-1, &filp->f_pos);
    filp_close(filp , NULL);
    set_fs(fs);
    if(ret == 0)
    {
        printk(KERN_ERR "\33[0;36m read /proc/cmdline failed!\n");
        return NULL;
    }

    cmdline[CMD_LINE_LETH-1] = '\0';
    pCmdline = strstr(cmdline, str);
    if(NULL == pCmdline)
    {
        printk(KERN_ERR "\33[0;36m Can't find str:%s in cmdline failed!\n",str);
        return NULL;
    }
    pCmdline = strstr(pCmdline, "=");
    while(*(pCmdline + n) != '\0' && *(pCmdline + n) != ' ')
    {
        n++;
    }

    if(NULL == pCmd_Section)
    {
        pCmd_Section = kmalloc(n * sizeof(char), GFP_KERNEL);
        if(NULL == pCmd_Section)
        {
            printk(KERN_ERR "\33[0;36m malloc fail.. str:%s in cmdline failed!\n",str);
            return NULL;
        }
    }
    pCmdline += 1;
    strncpy(pCmd_Section, pCmdline, n-1);
    pCmd_Section[n-1] = '\0';

    printk("function:%s,pCmd_Section:%s\n",__FUNCTION__,pCmd_Section);
    return pCmd_Section;
}

static unsigned short miuProtectSetValue[16] = {0};
static MI_S32 parse_miuprotect_file(void)
{
    struct file* filp = NULL;
    MI_S32 ret = 0;
    mm_segment_t fs;
    unsigned char uclength = 0;
    char *dString[16] = {NULL};
    char *ps8After = NULL;
    char *pMiuProtect = NULL;

    pMiuProtect = kmalloc(100 * sizeof(char), GFP_KERNEL);
    if (pMiuProtect == NULL)
    {
        DBG_ERR("parse_miuprotect_file malloc failed!\n");
        return MI_ERR_SYS_FAILED;
    }
    memset(pMiuProtect, 0, 100;);

    filp = filp_open("/config/miuprotect.txt", O_RDONLY, 0644);
    if (IS_ERR_OR_NULL(filp))
    {
        DBG_INFO("read miuprotect.txt failed! The miu whitelist will use default values.\n");
        kfree(pMiuProtect);
        return MI_ERR_SYS_FAILED;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    filp->f_op->llseek(filp, 0, SEEK_SET);

    ret = vfs_read(filp, pMiuProtect, 100, &filp->f_pos);
    filp_close(filp , NULL);
    set_fs(fs);
    if(ret == 0)
    {
        DBG_ERR("read miuprotect.txt failed!\n");
        kfree(pMiuProtect);
        return MI_ERR_SYS_FAILED;
    }

    while ((dString[uclength] = strsep(&pMiuProtect, ",")) != NULL)
    {
        miuProtectSetValue[uclength] = simple_strtoul(dString[uclength], &ps8After, 0);
        uclength++;

        if(uclength >= 16)
            break;
    }

    if (0 == uclength)
    {
        kfree(pMiuProtect);
        return MI_ERR_SYS_FAILED;
    }

    kfree(pMiuProtect);
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_Init(void)
{
    MI_S32 ret = MI_ERR_SYS_FAILED;

    char* str = NULL;
    const MI_SYSCFG_MmapInfo_t *mmap;
    MI_SYS_Version_t mi_version;
    DBG_ENTER();
    down(&mi_sys_global_SysInit);
    if(gSysInitCount == 0)
    {
    #if (1 == CHECK_MMA_HEAP_INFO)
        int k=0,j=0;
    #endif

        idr_init(&g_mi_sys_buf_handle_idr);
        mutex_init(&idr_alloc_mutex);
        INIT_LIST_HEAD(&mi_sys_global_dev_list);
        sema_init(&mi_sys_global_dev_list_semlock,1);
        _MI_SYS_IMPL_MemCacheCreate();

        ///init_glob_miu_kranges
        if(FALSE == MDrv_MIU_Init())
        {
            ret = MI_ERR_SYS_FAILED;
            DBG_EXIT_ERR("MDrv_MIU_Init FAIL \n");
            goto EXIT;
        }

#ifdef MI_SYS_PROC_FS_DEBUG
        _MI_SYS_IMPL_Drv_Proc_Init();//call this must before call _MI_SYS_IMPL_MmaHeapInit
#endif
        MI_SYSCFG_SetupMmapLoader();//call this after call _MI_SYS_IMPL_Drv_Proc_Init

        if(parse_miuprotect_file() == MI_SUCCESS)
        {
            g_kernel_protect_client_id = miuProtectSetValue;
            g_kprotect_enabled = 0;//while user wanna reset miuwhitelist, remove miu protect in heap first.
        }
        else
        {
            g_kernel_protect_client_id = MDrv_MIU_GetDefaultClientID_KernelProtect();
            g_kprotect_enabled = 1;
        }
        /* to set kernel_protect for each lxmem */

        init_glob_miu_kranges();
        DBG_INFO("debgug:init_glob_miu_kranges done\n");

#if (1 == CHECK_MMA_HEAP_INFO)//debug code
        DBG_INFO("[[debug]] INVALID_PHY_ADDR=%lx   %s %d   name:",INVALID_PHY_ADDR,__FUNCTION__,__LINE__);
        for(j=0;j<mstar_driver_boot_mma_buffer_num ;j++)
        {
            if(mma_config[j].size !=0)//inside for
            {
                for(k=0;k<strlen(mma_config[j].name);k++)
                {
                    DBG_INFO("%c",mma_config[j].name[k]);
                }
                DBG_INFO("    miu=%d,sz=%lx  reserved_start=%lx\n", mma_config[j].miu, mma_config[j].size,mma_config[j].reserved_start);
            }
        }
#endif

        MI_SYSCFG_SetupIniLoader();//call this after call _MI_SYS_IMPL_Drv_Proc_Init
        MI_SYSCFG_InitCmdqMmapInfo();

        str = parese_Cmdline("LX_MEM");
        if(str == NULL)
        {
            printk(KERN_ERR "\33[0;36m LX MEM length is not set in cmdline...\n");
            //return 0;
        }
        else
        {
            if(MI_SYSCFG_GetMmapInfo("E_LX_MEM", &mmap))
            {
                 if(mmap->u32Size != simple_strtoul(str, NULL, 0))
                 {
                     printk(KERN_ERR "\33[0;36m sn lx len:mmap->u32Size:0x%x,but kernel len:0x%lx,fail!!!!\n",mmap->u32Size, simple_strtoul(str, NULL, 0));
                 }
            }
            else
            {
                printk("Get LX_MEM fail in mmap ....\n");
            }
        }

        ret  = _MI_SYS_IMPL_MmaHeapInit();
        if(ret != MI_SUCCESS)
        {
            DBG_EXIT_ERR("mma heap init fail\n");
            goto EXIT;
        }
        mi_sys_init_allocator_collection(&g_stGlobalVBPOOLAllocatorCollection);

        MI_SYS_LOG_IMPL_Init();
    	MI_SYS_IMPL_GetVersion(&mi_version);
    	printk("%s\n", &mi_version.u8Version[0]);
    }
    gSysInitCount ++;
    ret = MI_SUCCESS;
    DBG_EXIT_OK();
EXIT:
    up(&mi_sys_global_SysInit);
    return ret;
}

MI_S32 MI_SYS_IMPL_Exit(void)
{
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();
    down(&mi_sys_global_SysInit);
    if(gSysInitCount == 1)
    {
        idr_destroy(&g_mi_sys_buf_handle_idr);

        ret  = _MI_SYS_IMPL_MmaHeapExit();

        #ifdef MI_SYS_PROC_FS_DEBUG

        _MI_SYS_IMPL_Drv_Proc_Exit();
        #endif

        if(ret != MI_SUCCESS)
        {
            DBG_EXIT_ERR("mma heap exit fail\n");
            goto EXIT;
        }

        mi_sys_deinit_allocator_collection(&g_stGlobalVBPOOLAllocatorCollection);

        _MI_SYS_IMPL_MemCacheDestroy();//MemCacheDestroy must in last step.

        MI_SYS_LOG_IMPL_Exit();
    }
    if(gSysInitCount >= 1)
        gSysInitCount --;
    ret = MI_SUCCESS;
    DBG_EXIT_OK();
EXIT:
    up(&mi_sys_global_SysInit);
    return ret;
}

MI_S32 MI_SYS_IMPL_BindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort ,MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate)
{
    MI_S32 ret = MI_ERR_SYS_FAILED;
    MI_SYS_InputPort_t *pstInputPort = NULL;
    MI_SYS_OutputPort_t *pstOutputPort = NULL;

    DBG_ENTER();

    pstInputPort = _MI_SYS_IMPL_GetInputPortInfo(pstDstChnPort);

    if(!pstInputPort)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("InputPort(%d %d %d %d)\n",pstDstChnPort->eModId , pstDstChnPort->u32DevId , pstDstChnPort->u32ChnId , pstDstChnPort->u32PortId);
        goto EXIT;
    }

    pstOutputPort = _MI_SYS_IMPL_GetOutputPortInfo(pstSrcChnPort);

    DBG_INFO("InputPort(%d %d %d %d), OutputPort(%d %d %d %d)\n",
        pstDstChnPort->eModId, pstDstChnPort->u32DevId, pstDstChnPort->u32ChnId, pstDstChnPort->u32PortId,
        pstSrcChnPort->eModId, pstSrcChnPort->u32DevId, pstSrcChnPort->u32ChnId, pstSrcChnPort->u32PortId);

    if(!pstOutputPort)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("OutPort(%d %d %d %d)\n",pstSrcChnPort->eModId , pstSrcChnPort->u32DevId , pstSrcChnPort->u32ChnId , pstSrcChnPort->u32PortId);
        goto EXIT;
    }

    pstInputPort->bEnableLowLatencyReceiveMode = pstDstChnPort->bEnableLowLatencyReceiveMode;
    pstInputPort->u32LowLatencyDelayMs = pstDstChnPort->u32LowLatencyDelayMs;

    ret = _MI_SYS_IMPL_BindChannelPort(pstInputPort, pstOutputPort, u32SrcFrmrate, u32DstFrmrate);
    if(ret != MI_SUCCESS)
        DBG_EXIT_ERR("InputPort(%d %d %d %d), OutputPort(%d %d %d %d)\n",
        pstDstChnPort->eModId, pstDstChnPort->u32DevId, pstDstChnPort->u32ChnId, pstDstChnPort->u32PortId,
        pstSrcChnPort->eModId, pstSrcChnPort->u32DevId, pstSrcChnPort->u32ChnId, pstSrcChnPort->u32PortId);
    DBG_EXIT_OK();
EXIT:
    return ret ;
}

MI_S32 MI_SYS_IMPL_UnBindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort)
{
    MI_SYS_InputPort_t *pstInputPort = NULL;
    MI_SYS_OutputPort_t *pstOutputPort = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();

    pstInputPort = _MI_SYS_IMPL_GetInputPortInfo(pstDstChnPort);
    if(!pstInputPort)
    {
        DBG_EXIT_ERR("InputPort(%d %d %d %d)\n",pstDstChnPort->eModId , pstDstChnPort->u32DevId , pstDstChnPort->u32ChnId , pstDstChnPort->u32PortId);
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        goto EXIT;
    }

    pstOutputPort = _MI_SYS_IMPL_GetOutputPortInfo(pstSrcChnPort);
    if(!pstOutputPort)
    {
        DBG_EXIT_ERR("OutPort(%d %d %d %d)\n",pstSrcChnPort->eModId , pstSrcChnPort->u32DevId , pstSrcChnPort->u32ChnId , pstSrcChnPort->u32PortId);
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        goto EXIT;
    }

    pstInputPort->bEnableLowLatencyReceiveMode = pstDstChnPort->bEnableLowLatencyReceiveMode;
    pstInputPort->u32LowLatencyDelayMs = pstDstChnPort->u32LowLatencyDelayMs;

    ret = _MI_SYS_IMPL_UnBindChannelPort(pstInputPort , pstOutputPort);
    if(ret != MI_SUCCESS)
        DBG_EXIT_ERR("InputPort(%d %d %d %d), OutputPort(%d %d %d %d) , ret = %d\n",
        pstDstChnPort->eModId, pstDstChnPort->u32DevId, pstDstChnPort->u32ChnId, pstDstChnPort->u32PortId,
        pstSrcChnPort->eModId, pstSrcChnPort->u32DevId, pstSrcChnPort->u32ChnId, pstSrcChnPort->u32PortId ,ret);
    DBG_EXIT_OK();
EXIT:
    return ret;
}

MI_S32 MI_SYS_IMPL_EnsureOutportBKRefFifoDepth(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId,
                                                                                                  MI_U32 u32PortId, MI_U32 u32BkRefFifoDepth)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
    MI_SYS_Channel_t *pstChannel = NULL;
    MI_SYS_OutputPort_t *pstOutputPort = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();

    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

    MI_SYS_BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >=pstModDev->u32OutputPortNum);

    pstChannel = &pstModDev->astChannels[u32ChnId];
    MI_SYS_BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    pstOutputPort = pstChannel->pastOutputPorts[u32PortId];
    MI_SYS_BUG_ON(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
    if(u32BkRefFifoDepth> pstOutputPort->u32OutputPortBufCntQuota-1)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("u32BkRefFifoDepth:%d ,u32OutputPortBufCntQuota:%d\n" ,u32BkRefFifoDepth,pstOutputPort->u32OutputPortBufCntQuota);
        goto EXIT;
    }

    pstOutputPort->u32DrvBkRefFifoDepth = u32BkRefFifoDepth;
    MI_SYS_BUG_ON(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
    while((int)pstOutputPort->stDrvBkRefFifoQueue.queue_buf_count>pstOutputPort->u32DrvBkRefFifoDepth)
    {
        MI_SYS_BufRef_t *pstTmpBufRef = mi_sys_remove_from_queue_head(&pstOutputPort->stDrvBkRefFifoQueue);
        if(!pstTmpBufRef)
            MI_SYS_BUG();
        mi_sys_release_bufref(pstTmpBufRef);
    }
    DBG_EXIT_OK();
    ret = MI_SUCCESS;
EXIT:
    return ret;
}

MI_S32 MI_SYS_IMPL_InjectBuf(MI_SYS_BUF_HANDLE handle , MI_SYS_ChnPort_t *pstChnInputPort)
{
    MI_SYS_BufHandleIdrData_t *pstHandleData = NULL;
    MI_SYS_InputPort_t *pstInputPort = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();

    pstHandleData = idr_find(&g_mi_sys_buf_handle_idr, handle);
    if(!pstHandleData)
    {
        DBG_EXIT_ERR("handle illegal %d\n" , handle);
        ret =  MI_ERR_SYS_ILLEGAL_PARAM;
        goto EXIT;
    }
    if(pstHandleData->eBufType!= E_MI_SYS_IDR_BUF_TYPE_OUTPUT_PORT)
    {
        DBG_EXIT_ERR("pstHandleData->eBufType(%d) error\n" , pstHandleData->eBufType);
        ret =  MI_ERR_SYS_ILLEGAL_PARAM;
        goto EXIT;
    }

    _MI_SYS_IMPL_UnMmapUserVirAddr(pstHandleData->pstBufRef, TRUE);

    pstInputPort = _MI_SYS_IMPL_GetInputPortInfo(pstChnInputPort);
    if(!pstInputPort)
    {
        DBG_EXIT_ERR("InputPort(%d %d %d %d)\n",pstChnInputPort->eModId , pstChnInputPort->u32DevId , pstChnInputPort->u32ChnId , pstChnInputPort->u32PortId);
        ret =  MI_ERR_SYS_ILLEGAL_PARAM;
        goto EXIT;
    }

    ret = _MI_SYS_IMPL_AddBufrefIntoInputUsrInjectQueue(pstInputPort , pstHandleData->pstBufRef);
    idr_remove(&g_mi_sys_buf_handle_idr, handle);
    mi_sys_release_bufref(pstHandleData->pstBufRef);
    memset(pstHandleData,0x27, sizeof(*pstHandleData));
    kfree(pstHandleData);
    if(ret != MI_SUCCESS)
        DBG_EXIT_ERR("AddBufrefIntoInputUsrInjectQueue Fail ret = %d\n",ret);
    DBG_EXIT_OK();
EXIT:
    return ret;
}

MI_S32 MI_SYS_IMPL_GetVersion (MI_SYS_Version_t *pstVersion)
{
    DBG_ENTER();
    memset(&pstVersion->u8Version, 0, sizeof(pstVersion->u8Version));
    memcpy(&pstVersion->u8Version, &MACRO_TO_STRING(MSTAR_MODULE_VERSION), sizeof(pstVersion->u8Version));
    DBG_INFO("%s:%d\n", &pstVersion->u8Version, sizeof(pstVersion->u8Version));
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_GetCurPts (MI_U64 *pu64Pts)
{
    struct timespec ts;
    MI_U64 u64_current_boot_us;

    DBG_ENTER();
    get_monotonic_boottime(&ts);
    u64_current_boot_us = ts.tv_sec*1000000ULL+ts.tv_nsec/1000;

    *pu64Pts = g_pts_base+(u64_current_boot_us-g_pts_base_mono_us);

    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_InitPtsBase (MI_U64 u64PtsBase)
{
    struct timespec ts;

    DBG_ENTER();
    g_pts_base =u64PtsBase;
    get_monotonic_boottime(&ts);
    g_pts_base_mono_us = ts.tv_sec*1000000ULL+ts.tv_nsec/1000;

    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_SyncPts (MI_U64 u64Pts)
{
    struct timespec ts;

    DBG_ENTER();
    g_pts_base =u64Pts;
    get_monotonic_boottime(&ts);
    g_pts_base_mono_us = ts.tv_sec*1000000ULL+ts.tv_nsec/1000;
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_Mmap(MI_PHY phyAddr, MI_U32 u32Size , void **ppVirtualAddress , MI_BOOL bCaChe)
{
    mi_sys_mmap_data_t *pst_mmap_data = NULL;
    unsigned long populate;
    unsigned long pfn;
    unsigned long pages;
    unsigned long long cpu_bus_addr;
    struct file* file = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;
    u32Size = ALIGN_UP(u32Size,PAGE_SIZE);

    DBG_ENTER();

    if(phyAddr&~PAGE_MASK || u32Size&~PAGE_MASK)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("phyAddr:%lld u32Size:%d\n",phyAddr,u32Size);
        goto EXIT;
    }

    if(u32Size == 0||ppVirtualAddress==NULL)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("\n");
        goto EXIT;
    }

    cpu_bus_addr = mi_sys_Miu2Cpu_BusAddr(phyAddr);

    *ppVirtualAddress = NULL;

    pfn = __phys_to_pfn(cpu_bus_addr);
    pages =  (u32Size>>PAGE_SHIFT);

    if(pfn+pages <= pfn)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("\n");
        goto EXIT;
    }

    //valid check
    if(!mi_sys_check_page_range_mapable(pfn, pages))
    {
        //invalid parameter
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("mi_sys_check_page_range_mapable failed\n");
        goto EXIT;
    }

    pst_mmap_data = (mi_sys_mmap_data_t*)kmalloc(sizeof(*pst_mmap_data),GFP_KERNEL);

    if (unlikely(!pst_mmap_data))
    {
        ret = MI_ERR_SYS_NOMEM;
        DBG_EXIT_ERR("\n");
        goto EXIT;
    }

    pst_mmap_data->pfn = pfn;
    pst_mmap_data->pages = pages;
    pst_mmap_data->bCaChe = bCaChe;

    file = anon_inode_getfile("mi_sys_api_mmap_file", &msys_api_mmap_fops, pst_mmap_data, O_RDWR);

    if (IS_ERR(file))
    {
        kfree(pst_mmap_data);
        ret = MI_ERR_SYS_FAILED;
        DBG_EXIT_ERR("anon_inode_getfile failed \n");
        goto EXIT;
    }

    down_write(&current->mm->mmap_sem);

    *ppVirtualAddress = (void *)do_mmap_pgoff(file, 0, u32Size, PROT_READ|PROT_WRITE, MAP_SHARED, 0, &populate);

    up_write(&current->mm->mmap_sem);

    fput(file);

    if(IS_ERR(*ppVirtualAddress))
    {
        *ppVirtualAddress = NULL;
        ret =  MI_ERR_SYS_FAILED;
        DBG_EXIT_ERR("anon_inode_getfile failed \n");
        goto EXIT;
    }
    if (populate)
        mm_populate((unsigned long )(*ppVirtualAddress), populate);
    ret = MI_SUCCESS;
    DBG_EXIT_OK();
EXIT:
    return ret;
}

//unmap user va.
MI_S32 MI_SYS_IMPL_Munmap(void *pVirtualAddress, MI_U32 u32Size)
{
    MI_S32 ret = MI_ERR_SYS_FAILED;
    int retValue = 0;
    DBG_ENTER();

    u32Size = ALIGN_UP(u32Size,PAGE_SIZE);
    if(pVirtualAddress == NULL || u32Size == 0)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("pVirtualAddress=%p , u32Size=%d\n",pVirtualAddress , u32Size);
        goto EXIT;
    }

    down_write(&current->mm->mmap_sem);
    retValue = do_munmap(current->mm, (unsigned long)pVirtualAddress, u32Size);
    up_write(&current->mm->mmap_sem);
    if(retValue != 0)//if not equal with 0,means fail.0 means success
    {
        ret = MI_ERR_SYS_FAILED;
        DBG_EXIT_ERR("%d \n",retValue);
        goto EXIT;
    }
    ret = MI_SUCCESS;
    DBG_EXIT_OK();
EXIT:
    return ret;
}

MI_S32 MI_SYS_IMPL_FlushInvCache(void * va, MI_U32 u32Size)
{
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();

    if(va == NULL || u32Size == 0)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("va:%p , u32Size :%d\n",va , u32Size);
        goto EXIT;
    }

    ret = mi_sys_buf_mgr_flush_inv_cache(va, u32Size);
    if(ret != MI_SUCCESS)
    {
        DBG_EXIT_ERR("ret = %d\n",ret);
        goto EXIT;
    }
    ret = MI_SUCCESS;
    DBG_EXIT_OK();
EXIT:
    return ret;
}

MI_S32 MI_SYS_IMPL_SetReg (MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask)
{
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_GetReg (MI_U32 u32RegAddr, MI_U16 *pu16Value)
{
    return MI_SUCCESS;
}

static inline char *mi_strndup(MI_U8 *pu8MMAHeapName, size_t u32MaxLen)
{
    size_t len = strlen(pu8MMAHeapName);
    char *duplicated_name = NULL;

    len = min(u32MaxLen, len);

    duplicated_name= (char*)kmalloc(len+1, GFP_KERNEL);
    if(!duplicated_name)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

    strncpy(duplicated_name, pu8MMAHeapName, len);
    duplicated_name[len] = 0;
    return duplicated_name;
}

MI_S32 MI_SYS_IMPL_SetChnMMAConf ( MI_ModuleId_e eModId,  MI_U32  u32DevId, MI_U32 u32ChnId, MI_U8 *pu8MMAHeapName)
{
    MI_SYS_MOD_DEV_t *pstModDev = NULL;
    struct list_head *pos = NULL;
    char *duplicated_name = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();

    duplicated_name = mi_strndup(pu8MMAHeapName, 64);
    if(!duplicated_name)
    {
        ret = MI_ERR_SYS_NOMEM;
        DBG_EXIT_ERR("\n");
        goto EXIT;
    }

    down(&mi_sys_global_dev_list_semlock);
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);
        MI_SYS_BUG_ON(pstModDev->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
        MI_SYS_BUG_ON(pstModDev->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);
        //MI_SYS_BUG_ON(pstModDev->u32InputPortNum ==0 && pstModDev->u32OutputPortNum ==0);
        //MI_SYS_BUG_ON(pstModDev->u32DevChnNum ==0);
        MI_SYS_BUG_ON(pstModDev->u32DevChnNum > MI_SYS_MAX_DEV_CHN_CNT);

        if(pstModDev->eModuleId == eModId && pstModDev->u32DevId == u32DevId)
        {
            if(u32ChnId >=pstModDev->u32DevChnNum)
            {
                ret = MI_ERR_SYS_ILLEGAL_PARAM;
                DBG_EXIT_ERR("ModuleId:%d ChnId:%d, DevChnNum:%d",eModId , u32ChnId , pstModDev->u32DevChnNum);
                up(&mi_sys_global_dev_list_semlock);
                goto EXIT;
            }
            if(pstModDev->astChannels[u32ChnId].pu8MMAHeapName)
                kfree(pstModDev->astChannels[u32ChnId].pu8MMAHeapName);
            pstModDev->astChannels[u32ChnId].pu8MMAHeapName = duplicated_name;
            duplicated_name = NULL;
            ret = MI_SUCCESS;
            DBG_EXIT_OK();
            up(&mi_sys_global_dev_list_semlock);
            goto EXIT;
        }
    }
    up(&mi_sys_global_dev_list_semlock);
    ret = MI_ERR_SYS_FAILED;
    DBG_EXIT_ERR("ModuleId:%d ChnId:%d",eModId , u32ChnId);
EXIT:
    if(duplicated_name)
        kfree(duplicated_name);
    return ret;
}

MI_S32 MI_SYS_IMPL_GetChnMMAConf ( MI_ModuleId_e eModId,  MI_U32  u32DevId, MI_U32 u32ChnId, void  *data, MI_U32 u32Length)
{
    MI_SYS_MOD_DEV_t *pstModDev = NULL;
    struct list_head *pos = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;
    char *pu8MMAHeapName = (char*)data;

    DBG_ENTER();

    u32Length = min(u32Length, (MI_U32)64);

    down(&mi_sys_global_dev_list_semlock);
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);
        MI_SYS_BUG_ON(pstModDev->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
        MI_SYS_BUG_ON(pstModDev->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);
        //MI_SYS_BUG_ON(pstModDev->u32InputPortNum ==0 && pstModDev->u32OutputPortNum ==0);
        //MI_SYS_BUG_ON(pstModDev->u32DevChnNum ==0);
        MI_SYS_BUG_ON(pstModDev->u32DevChnNum > MI_SYS_MAX_DEV_CHN_CNT);

        if(pstModDev->eModuleId == eModId && pstModDev->u32DevId == u32DevId)
        {
            if(u32ChnId >=pstModDev->u32DevChnNum)
            {
                ret = MI_ERR_SYS_ILLEGAL_PARAM;
                DBG_EXIT_ERR("ModuleId:%d ChnId:%d, DevChnNum:%d",eModId , u32ChnId , pstModDev->u32DevChnNum);
                goto EXIT;
            }
            if(pstModDev->astChannels[u32ChnId].pu8MMAHeapName)
            {
                strncpy(pu8MMAHeapName, pstModDev->astChannels[u32ChnId].pu8MMAHeapName, u32Length);
                pu8MMAHeapName[u32Length-1] = 0;
            }
            else
                pu8MMAHeapName[0] = 0;
            ret =MI_SUCCESS;
            DBG_EXIT_OK();
            goto EXIT;;
        }
    }

    ret = MI_ERR_SYS_FAILED;
    DBG_EXIT_ERR("ModuleId:%d ChnId:%d\n",eModId , u32ChnId);

EXIT:
    up(&mi_sys_global_dev_list_semlock);
    return ret;
}

MI_S32 MI_SYS_IMPL_GetBindbyDest (MI_SYS_ChnPort_t *pstChnInputPort, MI_SYS_ChnPort_t *pstChnOutputPort)
{
    MI_SYS_InputPort_t *pstInputPort = NULL;
    MI_SYS_OutputPort_t *pstOutputPort = NULL;
    DBG_ENTER();
    pstInputPort = _MI_SYS_IMPL_GetInputPortInfo(pstChnInputPort);
    MI_SYS_BUG_ON(!pstInputPort);

    pstOutputPort = _MI_SYS_IMPL_GetInputportPeer(pstInputPort);
    MI_SYS_BUG_ON(!pstOutputPort);

    pstChnOutputPort->eModId = pstOutputPort->pstChannel->pstModDev->eModuleId;
    pstChnOutputPort->u32DevId = pstOutputPort->pstChannel->pstModDev->u32DevId;
    pstChnOutputPort->u32ChnId = pstOutputPort->pstChannel->u32ChannelId;
    pstChnOutputPort->u32PortId = pstOutputPort->u32PortId;
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_SetChnOutputPortDepth(MI_SYS_ChnPort_t *pstChnPort , MI_U32 u32UserFrameDepth , MI_U32 u32BufQueueDepth)
{
    MI_SYS_OutputPort_t *pstOutputPort = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();

    if(!pstChnPort || u32UserFrameDepth > u32BufQueueDepth)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("\n");
        goto EXIT;
    }

    pstOutputPort = _MI_SYS_IMPL_GetOutputPortInfo(pstChnPort);
    if(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("\n");
        goto EXIT;

    }

    pstOutputPort->u32UsrFifoCount= u32UserFrameDepth;
    pstOutputPort->u32OutputPortBufCntQuota = u32BufQueueDepth;

    ret = MI_SUCCESS;
    DBG_EXIT_OK();
EXIT:
    return ret ;
}

MI_S32 MI_SYS_IMPL_ChnOutputPortGetBuf(MI_SYS_ChnPort_t *pstChnPort,MI_SYS_BufInfo_t *pstBufInfo, MI_SYS_BUF_HANDLE *BufHandle, MI_U32 u32ExtraFlags)
{
    MI_SYS_BufHandleIdrData_t *pstHandleData = NULL;
    MI_SYS_BufRef_t *pstBufRef = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();

    pstBufRef = _MI_SYS_IMPL_UserGetOutputPortBuf(pstChnPort);
    if(!pstBufRef)
    {
        ret =  MI_ERR_SYS_NOBUF;
        goto EXIT;
    }


    pstHandleData = (MI_SYS_BufHandleIdrData_t*)kmalloc(sizeof(MI_SYS_BufHandleIdrData_t) ,GFP_KERNEL);
    if(pstHandleData == NULL)
    {
        ret =  MI_ERR_SYS_NOMEM;
        DBG_EXIT_ERR("\n");
        goto EXIT;
    }

    pstHandleData->eBufType = E_MI_SYS_IDR_BUF_TYPE_OUTPUT_PORT;
    pstHandleData->pstBufRef = pstBufRef;
    pstHandleData->stChnPort = *pstChnPort;


    if(u32ExtraFlags & MI_SYS_MAP_VA)
    {
        ret = _MI_SYS_IMPL_MmapToUserVirAddr(pstHandleData->pstBufRef, (u32ExtraFlags & MI_SYS_MAP_CPU_READ) > 0);
        if(ret != MI_SUCCESS)
        {
            kfree(pstHandleData);
            mi_sys_release_bufref(pstBufRef);
            *BufHandle = -1;
            DBG_EXIT_ERR("ret = %d\n",ret);
            goto EXIT;
        }
    }

    *pstBufInfo = pstBufRef->bufinfo;
    *BufHandle = idr_alloc(&g_mi_sys_buf_handle_idr, pstHandleData, MI_SYS_OUTPUT_PORT_HANDLE_START, MI_SYS_OUTPUT_PORT_HANDLE_END, GFP_KERNEL);
    if((*BufHandle) < 0)
    {
        memset(pstHandleData,0x28, sizeof(*pstHandleData));
        kfree(pstHandleData);
        //do nothing, so no need flush.
        _MI_SYS_IMPL_UnMmapUserVirAddr(pstBufRef, FALSE);
        mi_sys_release_bufref(pstBufRef);

        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("\n");
        goto EXIT;
    }
    DBG_EXIT_OK();

    pstHandleData->u32ExtraFlags = u32ExtraFlags;
    ret = MI_SUCCESS;
EXIT:
    return ret;
}

MI_S32 MI_SYS_IMPL_ChnOutputPortPutBuf(MI_SYS_BUF_HANDLE BufHandle)
{
    MI_SYS_BufHandleIdrData_t *pstHandleData = NULL;
    MI_SYS_OutputPort_t *pstOutputPort = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();

    pstHandleData = idr_find(&g_mi_sys_buf_handle_idr, BufHandle);
    if(!pstHandleData)
    {
        ret =  MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("BufHandle error %p\n",BufHandle);
        goto EXIT;
    }
    if(pstHandleData->eBufType != E_MI_SYS_IDR_BUF_TYPE_OUTPUT_PORT)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("pstHandleData->eBufType(%d) error\n",pstHandleData->eBufType);
        goto EXIT;
    }

    if(pstHandleData->u32ExtraFlags & MI_SYS_MAP_VA)
       _MI_SYS_IMPL_UnMmapUserVirAddr(pstHandleData->pstBufRef, (pstHandleData->u32ExtraFlags & MI_SYS_MAP_CPU_READ) > 0);

    pstOutputPort = (MI_SYS_OutputPort_t *)pstHandleData->pstBufRef->pCBData;
    MI_SYS_BUG_ON(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

    mi_sys_release_bufref(pstHandleData->pstBufRef);

    idr_remove(&g_mi_sys_buf_handle_idr, BufHandle);

    memset(pstHandleData,0x29, sizeof(*pstHandleData));
    kfree(pstHandleData);
    ret =  MI_SUCCESS;
    DBG_EXIT_OK();
EXIT:
    return ret;
}

MI_S32 MI_SYS_IMPL_ChnInputPortGetBuf(MI_SYS_ChnPort_t *pstChnPort ,MI_SYS_BufConf_t *pstBufConf, MI_SYS_BufInfo_t *pstBufInfo , MI_SYS_BUF_HANDLE *BufHandle , MI_S32 s32TimeOutMS, MI_U32 u32ExtraFlags)
{
    MI_SYS_BufHandleIdrData_t *pstHandleData = NULL;
    MI_SYS_BufRef_t *pstBufRef = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;

    DBG_ENTER();
    pstBufRef = _MI_SYS_IMPL_UserGetInputPortBuf(pstChnPort, pstBufConf ,s32TimeOutMS);
    if(!pstBufRef)
    {
        *BufHandle  = -1;
        ret =  MI_ERR_SYS_NOBUF;
        goto EXIT;
    }

    pstHandleData = (MI_SYS_BufHandleIdrData_t*)kmalloc(sizeof(MI_SYS_BufHandleIdrData_t) ,GFP_KERNEL);
    if(pstHandleData == NULL)
    {
        DBG_EXIT_ERR("\n");
        ret =  MI_ERR_SYS_NOBUF;
        goto EXIT;
    }

    pstHandleData->eBufType= E_MI_SYS_IDR_BUF_TYPE_INPUT_PORT;
    pstHandleData->pstBufRef = pstBufRef;
    pstHandleData->stChnPort = *pstChnPort;

    if(check_pstBufRef(__FUNCTION__, __LINE__, pstHandleData->pstBufRef)) {
        DBG_WRN("BufHandle:%d\n", pstHandleData);
        DBG_WRN("pstHandleData [eBufType:%d pstBufRef:%p stChnPort addr:%p miSysDrvHandle:%d]\n",
                pstHandleData->eBufType, pstHandleData->pstBufRef,
                &pstHandleData->stChnPort,pstHandleData->miSysDrvHandle);

        DBG_WRN("stChnPort [eModId:%d u32ChnId:%d u32DevId:%d u32PortId:%d]\n",
                pstHandleData->stChnPort.eModId,pstHandleData->stChnPort.u32ChnId,
                pstHandleData->stChnPort.u32DevId,pstHandleData->stChnPort.u32PortId);
        dump_mi_sys_bufinfo();
    }

    if(u32ExtraFlags & MI_SYS_MAP_VA)
    {
        ret = _MI_SYS_IMPL_MmapToUserVirAddr(pstHandleData->pstBufRef, FALSE);
        if(ret != MI_SUCCESS)
        {
            memset(pstHandleData,0x2B, sizeof(*pstHandleData));
            kfree(pstHandleData);
            mi_sys_release_bufref(pstBufRef);
            *BufHandle  = -1;
            DBG_EXIT_ERR("ret = %d\n",ret);
            goto EXIT;
        }
    }

    *pstBufInfo = pstBufRef->bufinfo;
    mutex_lock(&idr_alloc_mutex);
    *BufHandle = idr_alloc(&g_mi_sys_buf_handle_idr, pstHandleData,  MI_SYS_INPUT_PORT_HANDLE_START, MI_SYS_INPUT_PORT_HANDLE_END, GFP_KERNEL);
    mutex_unlock(&idr_alloc_mutex);

    if((*BufHandle) < 0)
    {
        memset(pstHandleData,0x2C, sizeof(*pstHandleData));
        kfree(pstHandleData);
        _MI_SYS_IMPL_UnMmapUserVirAddr(pstBufRef, FALSE);
        mi_sys_release_bufref(pstBufRef );
        *BufHandle = -1;
        ret = MI_ERR_SYS_FAILED;
        DBG_EXIT_ERR("\n");
        goto EXIT;
    }
    DBG_EXIT_OK();

    pstHandleData->u32ExtraFlags = u32ExtraFlags;
    ret = MI_SUCCESS;
EXIT:
    return ret;
}

MI_S32 MI_SYS_IMPL_ChnInputPortPutBuf(MI_SYS_BUF_HANDLE BufHandle , MI_SYS_BufInfo_t *pstBufInfo , MI_BOOL bDropBuf)
{
    MI_SYS_BufHandleIdrData_t *pstHandleData = NULL;
    MI_SYS_InputPort_t *pstInputPort = NULL;
    MI_S32 ret = MI_ERR_SYS_FAILED;
    DBG_ENTER();
    pstHandleData = idr_find(&g_mi_sys_buf_handle_idr, BufHandle);
    if(!pstHandleData)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("BufHandle error %p\n",BufHandle);
        goto EXIT;
    }

    if(pstHandleData->eBufType!= E_MI_SYS_IDR_BUF_TYPE_INPUT_PORT)
    {
        ret = MI_ERR_SYS_ILLEGAL_PARAM;
        DBG_EXIT_ERR("pstHandleData->eBufType(%d) error\n",pstHandleData->eBufType);
        goto EXIT;
    }

    if(bDropBuf)
    {
        _MI_SYS_IMPL_UnMmapUserVirAddr(pstHandleData->pstBufRef, TRUE);
        mi_sys_release_bufref(pstHandleData->pstBufRef);
        idr_remove(&g_mi_sys_buf_handle_idr, BufHandle);

        memset(pstHandleData,0x2D, sizeof(*pstHandleData));
        kfree(pstHandleData);
        ret = MI_SUCCESS;
        DBG_EXIT_OK();
        goto EXIT;
    }

    if(check_pstBufRef(__FUNCTION__, __LINE__, pstHandleData->pstBufRef)) {
        DBG_WRN("pstHandleData [eBufType:%d pstBufRef:%p stChnPort addr:%p miSysDrvHandle:%d]\n",
                pstHandleData->eBufType, pstHandleData->pstBufRef,
                &pstHandleData->stChnPort,pstHandleData->miSysDrvHandle);

        DBG_WRN("stChnPort [eModId:%d u32ChnId:%d u32DevId:%d u32PortId:%d]\n",
                pstHandleData->stChnPort.eModId,pstHandleData->stChnPort.u32ChnId,
                pstHandleData->stChnPort.u32DevId,pstHandleData->stChnPort.u32PortId);
        dump_mi_sys_bufinfo();
    }

    _MI_SYS_IMPL_CopyBufinfoFromUsr(&pstHandleData->pstBufRef->bufinfo , pstBufInfo);

    //Map buf is writeble , so must be will flush cache.
    if(pstHandleData->u32ExtraFlags & MI_SYS_MAP_VA)
        _MI_SYS_IMPL_UnMmapUserVirAddr(pstHandleData->pstBufRef, TRUE);

    pstInputPort = (MI_SYS_InputPort_t *)pstHandleData->pstBufRef->pCBData;
    MI_SYS_BUG_ON(!pstInputPort || pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

    idr_remove(&g_mi_sys_buf_handle_idr, BufHandle);

    _MI_SYS_IMPL_AddBufrefIntoInputUsrInjectQueue(pstInputPort , pstHandleData->pstBufRef);

    mi_sys_release_bufref(pstHandleData->pstBufRef);

    memset(pstHandleData,0x2D, sizeof(*pstHandleData));
    kfree(pstHandleData);
    ret = MI_SUCCESS;
    DBG_EXIT_OK();
EXIT:
    return ret;
}

//////////////////////////////////////////////
////////////////internal API////////////////////

MI_SYS_DRV_HANDLE  MI_SYS_IMPL_RegisterDev(mi_sys_ModuleDevInfo_t *pstMouleInfo, mi_sys_ModuleDevBindOps_t *pstModuleBindOps , void *pUsrData
                                                            #ifdef MI_SYS_PROC_FS_DEBUG
                                                              ,mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps
                                                              ,struct proc_dir_entry *proc_dir_entry
                                                            #endif
                                                        )
{
    int size = 0;
    struct list_head *pos = NULL;
    MI_SYS_MOD_DEV_t *pListCurDev = NULL;
    MI_SYS_MOD_DEV_t *pstModDev = NULL;
    MI_U32 i = 0;

    DBG_ENTER();

    MI_SYS_BUG_ON(pstMouleInfo->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
    MI_SYS_BUG_ON(pstMouleInfo->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);
    MI_SYS_BUG_ON(pstMouleInfo->u32DevChnNum > MI_SYS_MAX_DEV_CHN_CNT);

    down(&mi_sys_global_dev_list_semlock);

    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pListCurDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        if (pListCurDev)
        {
            MI_SYS_BUG_ON(pListCurDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
            if(pListCurDev->eModuleId == pstMouleInfo->eModuleId &&
                pListCurDev->u32DevId == pstMouleInfo->u32DevId)
            {
                pstModDev = pListCurDev;
                up(&mi_sys_global_dev_list_semlock);
                DBG_EXIT_OK();
                goto EXIT;
            }
        }
    }

    up(&mi_sys_global_dev_list_semlock);

    size = sizeof(MI_SYS_MOD_DEV_t)+sizeof(MI_SYS_Channel_t)*pstMouleInfo->u32DevChnNum;

    DBG_INFO("eModuleId:%d, u32DevChnNum:%d, u32DevId:%d, u32InputPortNum:%d, u32OutputPortNum:%d, size:%d\n",
        pstMouleInfo->eModuleId,
        pstMouleInfo->u32DevChnNum,
        pstMouleInfo->u32DevId,
        pstMouleInfo->u32InputPortNum,
        pstMouleInfo->u32OutputPortNum,
        size);

    pstModDev = kmalloc(size , GFP_KERNEL);
    if(!pstModDev)
    {
        pstModDev = MI_HANDLE_NULL;
        DBG_EXIT_ERR("Kmalloc pstModDev Faild\n");
        goto EXIT;
    }

    pstModDev->u32InputPortNum =pstMouleInfo->u32InputPortNum;
    pstModDev->u32OutputPortNum =pstMouleInfo->u32OutputPortNum;
    pstModDev->u32DevChnNum =pstMouleInfo->u32DevChnNum;
    pstModDev->u32DevId =pstMouleInfo->u32DevId;
    pstModDev->eModuleId =pstMouleInfo->eModuleId;
    pstModDev->u32MagicNumber = __MI_SYS_MAGIC_NUM__;
    init_waitqueue_head(&(pstModDev->inputWaitqueue));

    if(pstModuleBindOps)
        memcpy(&pstModDev->ops , pstModuleBindOps, sizeof(pstModDev->ops));
    else
        memset(&pstModDev->ops , 0, sizeof(pstModDev->ops));
    pstModDev->pUsrData = pUsrData;

#ifdef MI_SYS_PROC_FS_DEBUG
    _MI_SYS_IMPL_Common_RegProc(pstModDev,pstModuleProcfsOps,proc_dir_entry);
#endif

    for( i=0; i<pstModDev->u32DevChnNum; i++)
    {
        if(MI_SUCCESS != _MI_SYS_IMPL_InitChannel(&pstModDev->astChannels[i], pstModDev))
            break;
        pstModDev->astChannels[i].u32ChannelId = i;
    }

    if( i != pstModDev->u32DevChnNum)
    {
        DBG_ERR("Create Channel Faild\n");
        goto ERR_REGISTER;
    }

    mi_sys_init_allocator_collection(&pstModDev->stAllocatorCollection);

    down(&mi_sys_global_dev_list_semlock);
    list_add_tail(&pstModDev->listModDev, &mi_sys_global_dev_list);
    up(&mi_sys_global_dev_list_semlock);

    DBG_EXIT_OK();
EXIT:
    return (MI_SYS_DRV_HANDLE)pstModDev;
ERR_REGISTER:

    for( i=0; i<pstModDev->u32DevChnNum; i++)
    {
        _MI_SYS_IMPL_DeinitChannel(&pstModDev->astChannels[i]);
    }
    memset(pstModDev, 0x2E, sizeof(*pstModDev));
    kfree(pstModDev);
    return MI_HANDLE_NULL;
}
MI_S32 MI_SYS_IMPL_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle)
{
    MI_U32 i;
    struct list_head *pos;
    int find = 0;

    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;

    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

    down(&mi_sys_global_dev_list_semlock);

    list_for_each(pos, &mi_sys_global_dev_list)
    {
        if(pos == &pstModDev->listModDev)
        {
            find = 1;
            break;
        }
    }
    MI_SYS_BUG_ON(!find);
    list_del(&pstModDev->listModDev);

    up(&mi_sys_global_dev_list_semlock);

#ifdef MI_SYS_PROC_FS_DEBUG
    _MI_SYS_IMPL_CommonClearCmd(pstModDev);
    _MI_SYS_IMPL_Common_UnRegProc(pstModDev);
#endif

    for( i=0; i<pstModDev->u32DevChnNum; i++)
    {
        _MI_SYS_IMPL_DeinitChannel(&pstModDev->astChannels[i]);
    }
    mi_sys_deinit_allocator_collection(&pstModDev->stAllocatorCollection);
    //for bug check purpose
    memset(pstModDev, 0x2F, sizeof(MI_SYS_MOD_DEV_t));

    kfree(pstModDev);

    return MI_SUCCESS;
}
#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 MI_SYS_IMPL_RegistCommand(MI_U8 *u8Cmd, MI_U8 u8MaxPara,
                    MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t,MI_U32,MI_U8,MI_U8 **,void *),
                    MI_SYS_DRV_HANDLE hHandle)
{
    MI_SYS_BUG_ON(!hHandle);

    _MI_SYS_IMPL_CommonRegCmd(u8Cmd, u8MaxPara, fpExecCmd, (MI_SYS_MOD_DEV_t *)hHandle);

    return MI_SUCCESS;
}
#endif

MI_S32 MI_SYS_IMPL_DevTaskIterator(MI_SYS_DRV_HANDLE miSysDrvHandle, mi_sys_TaskIteratorCallBack pfCallBack,void *pUsrData)
{
    mi_sys_ChnTaskInfo_t *pstTask;
    MI_U32 u32ChannelIdx = 0 , u32PortId = 0;
    MI_SYS_InputPort_t *pstInputPort;
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t *)miSysDrvHandle;
    DECLARE_BITMAP(chn_mask, MI_SYS_MAX_DEV_CHN_CNT);
    int accept_one;
    MI_S32 ret;
    MI_SYS_BUG_ON(!pstModDev || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    bitmap_clear(chn_mask, 0, MI_SYS_MAX_DEV_CHN_CNT);
SCAN_AGAIN:

    accept_one = 0;
    pstTask = NULL;

    //check all channels
    for(u32ChannelIdx = 0 ; u32ChannelIdx < pstModDev->u32DevChnNum;u32ChannelIdx++)
    {
        int has_input_data = 0;
        //skip the current channel?
        if(test_bit(u32ChannelIdx , chn_mask))
            continue;
        if(!pstModDev->astChannels[u32ChannelIdx].bChnEnable)
            continue;
        pstTask = (mi_sys_ChnTaskInfo_t *)kmem_cache_alloc(mi_sys_taskinfo_cachep, GFP_KERNEL);
        if(!pstTask)
        {
            return MI_ERR_SYS_NOMEM;
        }

        memset(pstTask, 0, sizeof(*pstTask));
        INIT_LIST_HEAD(&pstTask->listChnTask);
        //Get a buf from each input port
        for(u32PortId = 0 ; u32PortId < pstModDev->u32InputPortNum ; u32PortId ++)
        {
            pstInputPort = pstModDev->astChannels[u32ChannelIdx].pastInputPorts[u32PortId];
            if(!pstInputPort->bPortEnable)
                continue;

            pstTask->astInputPortBufInfo[u32PortId] = MI_SYS_IMPL_GetInputPortBuf(miSysDrvHandle, u32ChannelIdx,u32PortId,0);
            if(pstTask->astInputPortBufInfo[u32PortId])
                has_input_data = 1;
        }
        if(has_input_data == 0)
        {
            memset(pstTask, 0x30, sizeof(*pstTask));
            kmem_cache_free(mi_sys_taskinfo_cachep,pstTask);
            continue;
        }
        pstTask->u32ChnId = u32ChannelIdx;
        pstTask->miSysDrvHandle = miSysDrvHandle;

        ret = pfCallBack(pstTask , pUsrData);

        switch((mi_sys_TaskIteratorCBAction_e)ret)
        {
            case MI_SYS_ITERATOR_ACCEPT_CONTINUTE:
                accept_one = 1;
                break;
            case MI_SYS_ITERATOR_SKIP_CONTINUTE: //µ±Ç°channel buf ,module²»ÐèÒª£¬´ÓÏÂ¸öchannle²éÑ¯
                __set_bit(u32ChannelIdx , chn_mask);
                mi_sys_RewindTask(pstTask);
                break;
            case MI_SYS_ITERATOR_ACCEPT_STOP:
                return MI_SUCCESS;
            default:
                MI_SYS_BUG();
        }
    }

    if(accept_one)
        goto SCAN_AGAIN;
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_PrepareTaskOutputBuf(mi_sys_ChnTaskInfo_t *pstTask)
{
    MI_SYS_OutputPort_t *pstOutputPort;
    MI_SYS_MOD_DEV_t *pstModDev;
    MI_SYS_Channel_t *pstChannel;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    MI_U32 u32PortId = 0;

    pstModDev = (struct MI_SYS_MOD_DEV_s *)(pstTask->miSysDrvHandle);
    MI_SYS_BUG_ON(!pstModDev
          || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);

    pstChannel = &pstModDev->astChannels[pstTask->u32ChnId];
    MI_SYS_BUG_ON(!pstChannel || pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    for( u32PortId=0;u32PortId<pstModDev->u32OutputPortNum;u32PortId++)
    {
        pstOutputPort = pstChannel->pastOutputPorts[u32PortId];

        MI_SYS_BUG_ON(pstTask->astOutputPortBufInfo[u32PortId]);

        if(!pstOutputPort->bPortEnable)
        {
              pstTask->bOutputPortMaskedByFrmrateCtrl[u32PortId] = FALSE;
              continue;
        }
        if(!_MI_SYS_IMPL_CheckBufConfValid(&pstTask->astOutputPortPerfBufConfig[u32PortId]))
        {
              pstTask->bOutputPortMaskedByFrmrateCtrl[u32PortId] = FALSE;
              continue;
        }
        pstTask->astOutputPortBufInfo[u32PortId]  = MI_SYS_IMPL_GetOutputPortBuf(pstTask->miSysDrvHandle, pstTask->u32ChnId,
        u32PortId, &pstTask->astOutputPortPerfBufConfig[u32PortId], &bBlockedByRateCtrl);
        pstTask->bOutputPortMaskedByFrmrateCtrl[u32PortId] = bBlockedByRateCtrl;
    }
    return MI_SUCCESS;
}
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
static inline void MI_SYS_IMPL_Auto_Sync_TrackInfo(MI_SYS_BufInfo_t *pstInputBuf, MI_SYS_BufInfo_t *pstOutputBuf)
{
   MI_SYS_BufRef_t *pstInputBufRef = container_of(pstInputBuf, MI_SYS_BufRef_t, bufinfo);
   MI_SYS_BufRef_t *pstOutputBufRef = container_of(pstOutputBuf, MI_SYS_BufRef_t, bufinfo);

   BUG_ON(!pstInputBufRef || !pstOutputBufRef);

   if(pstOutputBufRef->is_traceInfoValid == FALSE)
   {
       pstOutputBufRef->u32ObjectInitTimeInUS = pstInputBufRef->u32ObjectInitTimeInUS;
       pstOutputBufRef->u32ObjectAccessedTimeInUS = pstInputBufRef->u32ObjectAccessedTimeInUS;
       pstOutputBufRef->is_traceInfoValid = pstInputBufRef->is_traceInfoValid;
   }
}
#endif
MI_S32 MI_SYS_IMPL_FinishAndReleaseTask(struct mi_sys_ChnTaskInfo_s *pstTask)
{
    int i = 0;
    MI_S32 ret = 0;
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    int j = 0;
    MI_BOOL bData = FALSE;

    for (i = 0 ; i < MI_SYS_MAX_INPUT_PORT_CNT ; i ++)
    {
    	if(!pstTask->astInputPortBufInfo[i])
    		continue;
    	bData = TRUE;
    	break;
    }
    if(bData)
    {
    	for (j = 0 ; j < MI_SYS_MAX_OUTPUT_PORT_CNT ; j ++)
    	{
    		if(!pstTask->astOutputPortBufInfo[j])
    			continue;
    		MI_SYS_IMPL_Auto_Sync_TrackInfo(pstTask->astInputPortBufInfo[i],pstTask->astOutputPortBufInfo[j]);
    		break;
    	}
    }
#endif
    for (i = 0 ; i < MI_SYS_MAX_INPUT_PORT_CNT ; i ++)
    {
        if(!pstTask->astInputPortBufInfo[i])
            continue;

        ret = MI_SYS_IMPL_FinishBuf(pstTask->astInputPortBufInfo[i]);
        if(ret != MI_SUCCESS)
            MI_SYS_BUG();
    }
    for (i = 0 ; i < MI_SYS_MAX_OUTPUT_PORT_CNT ; i ++)
    {
        if(!pstTask->astOutputPortBufInfo[i])
            continue;
        ret = MI_SYS_IMPL_FinishBuf(pstTask->astOutputPortBufInfo[i]);
        if(ret != MI_SUCCESS)
            MI_SYS_BUG();
    }

    memset(pstTask, 0x31, sizeof(*pstTask));

    kmem_cache_free(mi_sys_taskinfo_cachep,pstTask);

    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_FinishAndReleaseTask_LL(struct mi_sys_ChnTaskInfo_s *pstTask)
{
    int i = 0;
    MI_S32 ret = 0;

    for (i = 0 ; i < MI_SYS_MAX_INPUT_PORT_CNT ; i ++)
    {
        if(!pstTask->astInputPortBufInfo[i])
            continue;

        ret = MI_SYS_IMPL_FinishBuf(pstTask->astInputPortBufInfo[i]);
        if(ret != MI_SUCCESS)
            MI_SYS_BUG();
    }
    for (i = 0 ; i < MI_SYS_MAX_OUTPUT_PORT_CNT ; i ++)
    {
        if(!pstTask->astOutputPortBufInfo[i])
            continue;
        ret = MI_SYS_IMPL_FinishBuf_LL(pstTask->astOutputPortBufInfo[i]);
        if(ret != MI_SUCCESS)
            MI_SYS_BUG();
    }

    memset(pstTask, 0x31, sizeof(*pstTask));

    kmem_cache_free(mi_sys_taskinfo_cachep,pstTask);

    return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_RewindTask(struct mi_sys_ChnTaskInfo_s *pstTask)
{
    int i = 0;
    MI_S32 ret = 0;

    for (i = 0 ; i < MI_SYS_MAX_INPUT_PORT_CNT ; i ++)
    {
        if(!pstTask->astInputPortBufInfo[i])
            continue;
        ret = MI_SYS_IMPL_RewindBuf(pstTask->astInputPortBufInfo[i]);
        if(ret != MI_SUCCESS)
            MI_SYS_BUG();
    }
    for (i = 0 ; i < MI_SYS_MAX_OUTPUT_PORT_CNT ; i ++)
    {
        if(!pstTask->astOutputPortBufInfo[i])
            continue;
        ret = MI_SYS_IMPL_RewindBuf(pstTask->astOutputPortBufInfo[i]);
        if(ret != MI_SUCCESS)
            MI_SYS_BUG();
    }

    memset(pstTask, 0x32, sizeof(*pstTask));
    kmem_cache_free(mi_sys_taskinfo_cachep,(void *)pstTask);

    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_DropTask(struct mi_sys_ChnTaskInfo_s *pstTask)
{
    int i = 0;
    MI_S32 ret = 0;

    for (i = 0 ; i < MI_SYS_MAX_INPUT_PORT_CNT ; i ++)
    {
        if(!pstTask->astInputPortBufInfo[i])
            continue;
        ret = MI_SYS_IMPL_FinishBuf(pstTask->astInputPortBufInfo[i]);
        if(ret != MI_SUCCESS)
            MI_SYS_BUG();
    }
    for (i = 0 ; i < MI_SYS_MAX_OUTPUT_PORT_CNT ; i ++)
    {
        if(!pstTask->astOutputPortBufInfo[i])
            continue;
        ret = MI_SYS_IMPL_RewindBuf(pstTask->astOutputPortBufInfo[i]);
        if(ret != MI_SUCCESS)
            MI_SYS_BUG();
    }

    memset(pstTask, 0x32, sizeof(*pstTask));
    kmem_cache_free(mi_sys_taskinfo_cachep,(void *)pstTask);

    return MI_SUCCESS;
}

static inline MI_U16 _mi_sys_lcm(MI_U16 value1, MI_U16 value2)
{
  MI_U16 r, num1, num2;

  if(value1 == value2)
     return value1;
  if(value1==0)
     return value2;
  if(value2==0)
     return value1;

  if (value2 > value1) {
      num2 = value1;
      num1 = value2;

  } else {
      num1 = value1;
      num2 = value2;
  }

  while(num2 > 0)
  {
      r = num1 % num2;
      num1 = num2;
      num2 = r;
  }
  return value1*value2/num1;
}

static inline void  _mi_sys_mergeBufExtraRequirement(MI_SYS_FrameBufExtraConfig_t *pstDstConf, MI_SYS_FrameBufExtraConfig_t *pstSrcConf)
{
   pstDstConf->u16BufHAlignment = _mi_sys_lcm(pstDstConf->u16BufHAlignment, pstSrcConf->u16BufHAlignment);
   pstDstConf->u16BufVAlignment = _mi_sys_lcm(pstDstConf->u16BufVAlignment, pstSrcConf->u16BufVAlignment);
   pstDstConf->bClearPadding = pstSrcConf->bClearPadding;
}

MI_SYS_BufInfo_t *MI_SYS_IMPL_GetOutputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId,
                                            MI_U32 u32PortId, MI_SYS_BufConf_t *pstBufConfig, MI_BOOL *bBlockedByRateCtrl)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
    MI_SYS_Channel_t *pstChannel;
    MI_SYS_OutputPort_t *pstOutputPort;
    int i_valid_fifo_cnt;
    struct list_head *pos;
    MI_SYS_BufferAllocation_t *pBufAllocation;
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufRef_t *pstBufRef;
    MI_BOOL  bNeedAllocBuf;
    MI_BOOL  bSideBandMsgSetted = FALSE;
    MI_U64 u64SidebandMsg = MI_SYS_SIDEBAND_MSG_NULL;
    MI_SYS_FrameBufExtraConfig_t stBufExtraConf;

    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufConfig);

    DBG_INFO("u32ChnId:%d, u32PortId:%d, u32DevChnNum:%d, u32OutputPortNum:%d\n", u32ChnId, u32PortId, pstModDev->u32DevChnNum, pstModDev->u32OutputPortNum);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >= pstModDev->u32OutputPortNum);

    pstChannel = &pstModDev->astChannels[u32ChnId];

    MI_SYS_BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    pstOutputPort = pstChannel->pastOutputPorts[u32PortId];
    MI_SYS_BUG_ON(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

    //Check whether the channel or port is enabled
    if(!_MI_SYS_IMPL_IsOutputPortEnabled(pstOutputPort))
    {
        DBG_INFO("Current Port Is Disable\n");
        return NULL;
    }

    stBufExtraConf = pstOutputPort->stOutputBufExtraRequirements;

    i_valid_fifo_cnt = (int)pstOutputPort->u32UsrFifoCount - atomic_read(&pstOutputPort->usrLockedBufCnt);
    MI_SYS_BUG_ON(pstOutputPort->u32OutputPortBufCntQuota == 0);

    if(atomic_read(&pstOutputPort->totalOutputPortInUsedBuf) >= pstOutputPort->u32OutputPortBufCntQuota)
    {
#ifdef  MI_SYS_SERIOUS_ERR_MAY_MULTI_TIMES_SHOW
        DBG_INFO("(Module:%d,ChnId:%d,PortId:%d)User Queue Buffer >= Total Output Port Buffer"
                "pstOutputPort->u32UsrFifoCount:%d u32OutputPortBufCntQuota:%d totalOutputPortInUsedBuf:%d\n",
                pstOutputPort->pstChannel->pstModDev->eModuleId,
                pstOutputPort->pstChannel->u32ChannelId,
                pstOutputPort->u32PortId,
                pstOutputPort->u32UsrFifoCount, pstOutputPort->u32OutputPortBufCntQuota,
                atomic_read(&pstOutputPort->totalOutputPortInUsedBuf));
#endif
        return NULL;
    }

    if(i_valid_fifo_cnt > 0)
    {
       //in case there is empty slot in output User FIFO, we will alloc to alloc outputbuf
       //else, the only alloc buf for pear input ports[ in this case, stUsrGetFifoBufQueue will execute FIFO policy]
        if(pstOutputPort->stUsrGetFifoBufQueue.queue_buf_count<i_valid_fifo_cnt)
             goto alloc_buf;

    }

    if(list_empty(&pstOutputPort->stBindPeerInputPortList))
    {
        //UsrFifo is NULL or UsrFifo is full, not allowed get OutputportBuf.
        if( 0 == pstOutputPort->u32UsrFifoCount ||
            pstOutputPort->u32UsrFifoCount + 1 <= atomic_read(&pstOutputPort->totalOutputPortInUsedBuf))
        {
            return NULL;
        }
    }

    down(&pstOutputPort->stBindedInputListSemlock);
    bNeedAllocBuf = FALSE;
    list_for_each(pos, &pstOutputPort->stBindPeerInputPortList)
    {
        MI_SYS_InputPort_t *pstCurInputPort = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
        MI_SYS_BUG_ON(pstCurInputPort->pstBindPeerOutputPort != pstOutputPort);

        if(_MI_SYS_IMPL_RcAcceptBuf(pstCurInputPort))
        {
            bNeedAllocBuf = TRUE;
            if(bSideBandMsgSetted && u64SidebandMsg!=pstCurInputPort->u64SidebandMsg)
            {
                // if the side msgs from input ports  are different, we will ingore it instead.
                u64SidebandMsg = MI_SYS_SIDEBAND_MSG_NULL;
            }
            else
            {
                u64SidebandMsg = pstCurInputPort->u64SidebandMsg;
                bSideBandMsgSetted = TRUE;
            }
            if(pstBufConfig->eBufType == E_MI_SYS_BUFDATA_FRAME)
                _mi_sys_mergeBufExtraRequirement(&stBufExtraConf, &pstCurInputPort->stInputBufExtraConf);
        }
    }

    if(!bNeedAllocBuf)
    {
        //No buf allocated means that  _MI_SYS_IMPL_OutputPortFinishBuf will never been called later, so we need to step rc here in advance
        list_for_each(pos, &pstOutputPort->stBindPeerInputPortList)
        {
            MI_SYS_InputPort_t *pstCurInputPort = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
            _MI_SYS_IMPL_StepRc(pstCurInputPort);
        }

        up(&pstOutputPort->stBindedInputListSemlock);
        if(bBlockedByRateCtrl)
        {
            *bBlockedByRateCtrl = TRUE;
        }

        DBG_INFO("Can't Get Buffer, Frame Rate Control\n");
        return NULL;
    }
    up(&pstOutputPort->stBindedInputListSemlock);

alloc_buf:
    //MI_SYS_INFO("%s : %d\n",__FUNCTION__,__LINE__);
    if(bBlockedByRateCtrl)
        *bBlockedByRateCtrl = FALSE;

    stChnPort.eModId = pstModDev->eModuleId;
    stChnPort.u32DevId = pstModDev->u32DevId;
    stChnPort.u32ChnId = u32ChnId;
    stChnPort.u32PortId = u32PortId;

    pBufAllocation = NULL;
    pstBufRef = NULL;
    if(pstBufConfig->eBufType == E_MI_SYS_BUFDATA_FRAME)
        pstBufConfig->stFrameCfg.stFrameBufExtraConf = stBufExtraConf;

    if(pstBufConfig->eBufType == E_MI_SYS_BUFDATA_META)
    {
        mi_sys_Allocator_t *pstMetaAllocator = mi_sys_get_global_meta_allocator();
        if(pstMetaAllocator)
            pBufAllocation = pstMetaAllocator->ops->alloc(pstMetaAllocator, pstBufConfig);
        if(!pBufAllocation)
        {
            DBG_ERR("Get Meta Data Buffer Faild\n");
            return NULL;
        }
    }
    else if(pstOutputPort->pstCusBufAllocator)
    {
        pBufAllocation = pstOutputPort->pstCusBufAllocator->ops->alloc(pstOutputPort->pstCusBufAllocator, pstBufConfig);
        if(!pBufAllocation)
        {
            //it's normal when failed to alloc from customer allocator
            //we will not retry fallback path in case cus allocator on
            // DBG_ERR("Get Cust Allocator Faild\n");
            return NULL;
        }
    }

    if(pBufAllocation)
    {
        //MI_SYS_INFO("%s : %d\n",__FUNCTION__,__LINE__);
        pstBufRef = mi_sys_create_bufref(pBufAllocation, pstBufConfig ,_MI_SYS_IMPL_OutputPortOnBufRefRelFunc, pstOutputPort);
        if(pstBufRef == NULL)
        {
            pstBufRef->pstBufAllocation->ops.OnRelease(pstBufRef->pstBufAllocation);
            DBG_ERR("Create BufRef Error in %s\n",__FUNCTION__);
            return NULL;
        }
    }
    if(!pstBufRef)
        pstBufRef =  _MI_SYS_IMPL_AllocBufDefaultPolicy(pstChannel, pstBufConfig,_MI_SYS_IMPL_OutputPortOnBufRefRelFunc, pstOutputPort);
    if(pstBufRef)
    {

        if(check_pstBufRef(__FUNCTION__, __LINE__, pstBufRef)) {

            DBG_WRN("stChnPort [eModId:%d u32ChnId:%d u32DevId:%d u32PortId:%d]\n",
                    stChnPort.eModId,stChnPort.u32ChnId,
                    stChnPort.u32DevId,stChnPort.u32PortId);
            dump_mi_sys_bufinfo();
        }

        //MI_SYS_INFO("%s : %d\n",__FUNCTION__,__LINE__);
        mi_sys_add_to_queue_tail(pstBufRef, &pstOutputPort->stWorkingQueue);
        atomic_inc(&pstOutputPort->totalOutputPortInUsedBuf);
        pstBufRef->pstBufAllocation->private1 = (void*)pstBufRef->pstBufAllocation->ops.OnRelease;
        pstBufRef->pstBufAllocation->private2 = (void*)pstOutputPort;
        pstBufRef->pstBufAllocation->ops.OnRelease = _MI_SYS_IMPL_Ouputport_wrapper_allocation_on_release;

        DBG_INFO("Get Buffer From Default Pool Done:%p, pstBufAllocation:%p\n", &(pstBufRef->bufinfo), pstBufRef->pstBufAllocation);

        /*
        MI_SYS_IMPL_GetOutputPortBuf is vacant buf.
        No need flushCache here..
        */
        if(pstBufConfig->u32Flags & MI_SYS_MAP_VA)
        {
            //MI_SYS_INFO("   pstBufAllocation=%p\n",pstBufRef->pstBufAllocation);
            mi_sys_buf_map_kern_va(pstBufRef->pstBufAllocation, &pstBufRef->bufinfo, FALSE);
        }
        MI_SYS_BUG_ON(atomic_read(&pstBufRef->pstBufAllocation->ref_cnt) !=1);
        //set sideband msg to buffer productor
        MI_SYS_BUG_ON(MI_SYS_SIDEBAND_MSG_ACKED(u64SidebandMsg));

        pstBufRef->bufinfo.u64SidebandMsg = u64SidebandMsg;

        pstBufRef->u32ExtFlag = pstBufConfig->u32Flags;
        return &(pstBufRef->bufinfo);
    }

    DBG_EXIT_ERR("Get Buffer Faild\n");
    return NULL;
}

MI_SYS_BufInfo_t *MI_SYS_IMPL_GetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId,
                                            MI_U32 u32PortId, MI_U32 u32Flags)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
    MI_SYS_Channel_t *pstChannel = NULL;
    MI_SYS_InputPort_t *pstInputPort = NULL;
    MI_SYS_BufRef_t *pstBufRef = NULL;
    MI_SYS_BufferQueue_t *pstBufferQue = NULL;
    MI_BOOL bUsrBuf = FALSE;

    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

    MI_SYS_BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >=pstModDev->u32InputPortNum);

    pstChannel = &pstModDev->astChannels[u32ChnId];
    MI_SYS_BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    pstInputPort = pstChannel->pastInputPorts[u32PortId];
    MI_SYS_BUG_ON(!pstInputPort || pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

    if(!_MI_SYS_IMPL_IsInputPortEnabled(pstInputPort))
    {
        DBG_INFO("Input Port Disable\n");
        return NULL;
    }

    if(pstInputPort->stBindInputBufQueue.queue_buf_count > 0)
        pstBufferQue = &pstInputPort->stBindInputBufQueue;
    else if(pstInputPort->stUsrInjectBufQueue.queue_buf_count > 0)
    {
        pstBufferQue = &pstInputPort->stUsrInjectBufQueue;
        bUsrBuf = TRUE;
    }
    else
        pstBufferQue = NULL;

    if(pstBufferQue)
        DBG_INFO("bind queue:%d, user queue:%d , WorkQueue:%d  ModuleId:%d ChannelId:%d  PortId:%d\n",
            pstInputPort->stBindInputBufQueue.queue_buf_count,
            pstInputPort->stUsrInjectBufQueue.queue_buf_count,
            pstInputPort->stWorkingQueue.queue_buf_count ,
            pstInputPort->pstChannel->pstModDev->eModuleId,
            pstInputPort->pstChannel->u32ChannelId , pstInputPort->u32PortId);
    if(pstBufferQue)
    {
        pstBufRef = mi_sys_remove_from_queue_head(pstBufferQue);
        if(pstBufRef)
        {
            mi_sys_add_to_queue_tail(pstBufRef , &pstInputPort->stWorkingQueue);
            if(bUsrBuf)
            {
                WAKE_UP_QUEUE_IF_NECESSARY(pstInputPort->inputBufCntWaitqueue);
            }
        }
        //in case racing with disable channel or disable inport, we may meet null here
    }

    if (NULL == pstBufRef)
    {
        DBG_INFO("No Input Buffer\n");
        return NULL;
    }
    if(check_pstBufRef(__FUNCTION__, __LINE__, pstBufRef)) {
        DBG_WRN("pstInputPort bind queue:%d, user queue:%d , WorkQueue:%d  \
            ModuleId:%d ChannelId:%d  PortId:%d\n",
            pstInputPort->stBindInputBufQueue.queue_buf_count,
            pstInputPort->stUsrInjectBufQueue.queue_buf_count,
            pstInputPort->stWorkingQueue.queue_buf_count ,
            pstInputPort->pstChannel->pstModDev->eModuleId,
            pstInputPort->pstChannel->u32ChannelId , pstInputPort->u32PortId);

        dump_mi_sys_bufinfo();
    }
    pstBufRef->bufinfo.bUsrBuf = bUsrBuf;
    DBG_INFO("pstBufRef:%p, pstBufAllocation:%p\n", &(pstBufRef->bufinfo), pstBufRef->pstBufAllocation);

    /*
    Input buf is finished by pre module.
    During finish buf or chn put buf, sys will make sure the buf's cache is clean & invalid.
    So, no need flushCache here.
    */
    if(u32Flags & MI_SYS_MAP_VA)
    {
        mi_sys_buf_map_kern_va(pstBufRef->pstBufAllocation, &pstBufRef->bufinfo, FALSE);
    }

    pstBufRef->u32ExtFlag = u32Flags;

    return &(pstBufRef->bufinfo);
}

MI_S32 MI_SYS_IMPL_FinishBuf(MI_SYS_BufInfo_t *buf)
{
    MI_SYS_BufRef_t *pstBufRef = container_of(buf, MI_SYS_BufRef_t, bufinfo);

    if(pstBufRef->onRelCB == _MI_SYS_IMPL_OutputPortOnBufRefRelFunc)
    {
        return _MI_SYS_IMPL_OutputPortFinishBuf(pstBufRef);
    }
     else if(pstBufRef->onRelCB == _MI_SYS_IMPL_InputPortOnBufRefRelFunc)
    {
        return _MI_SYS_IMPL_InputPortFinishBuf(pstBufRef);
    }
    else
    {
        MI_SYS_BUG();
    }

    return MI_ERR_SYS_ILLEGAL_PARAM;
}

MI_S32 MI_SYS_IMPL_FinishBuf_LL(MI_SYS_BufInfo_t *buf)
{
    MI_SYS_BufRef_t *pstBufRef = container_of(buf, MI_SYS_BufRef_t, bufinfo);

    if(pstBufRef->onRelCB == _MI_SYS_IMPL_OutputPortOnBufRefRelFunc)
    {
        return _MI_SYS_IMPL_OutputPortFinishBuf_LL(pstBufRef);
    }
     else if(pstBufRef->onRelCB == _MI_SYS_IMPL_InputPortOnBufRefRelFunc)
    {
        return _MI_SYS_IMPL_InputPortFinishBuf(pstBufRef);
    }
    else
    {
        MI_SYS_BUG();
    }

    return MI_ERR_SYS_ILLEGAL_PARAM;
}


typedef struct MI_SYS_InputPort_DelayWork_s
{
    struct delayed_work stDelayWork;
    MI_SYS_InputPort_t *pstInputPort;
    MI_SYS_BufRef_t *pstBufRef;
    MI_U64   u64TaskStartUs;
}MI_SYS_InputPort_DelayWork_t;

static void _MI_SYS_IMPL_DelayLowLatencyWorkFunc(struct work_struct * pstWork)
{
    MI_SYS_InputPort_DelayWork_t *pstDelayWork = container_of(pstWork, MI_SYS_InputPort_DelayWork_t, stDelayWork.work);
    MI_SYS_BufRef_t *pstBufRef;
    MI_SYS_InputPort_t *pstInputPort;
    int iScheduledDelayTaskCnt;
    struct timeval tv;

    BUG_ON(!pstWork);

    pstInputPort = pstDelayWork->pstInputPort;
    pstBufRef = pstDelayWork->pstBufRef;

    BUG_ON(!pstInputPort || pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
    BUG_ON(!pstInputPort->pstChannel ||pstInputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    mi_sys_add_to_queue_tail(pstBufRef,  &pstInputPort->stBindInputBufQueue);
    WAKE_UP_QUEUE_IF_NECESSARY(pstInputPort->pstChannel->pstModDev->inputWaitqueue);

    iScheduledDelayTaskCnt = atomic_dec_return(&pstInputPort->scheduledDelayTaskCnt);
    BUG_ON(iScheduledDelayTaskCnt<0);

    do_gettimeofday(&tv);
    pstInputPort->u32StaticDelayAccmMS += (tv.tv_sec*1000*1000 + tv.tv_usec-pstDelayWork->u64TaskStartUs)/1000;
    pstInputPort->u32StaticDelayAccmCnt++;
    pstInputPort->u32LastStaticDelayTotalMS = pstInputPort->u32StaticDelayAccmMS;
    pstInputPort->u32LastStaticDelayTotalCnt = pstInputPort->u32StaticDelayAccmCnt;
    if(pstInputPort->u32StaticDelayAccmCnt >= 100)
    {
       pstInputPort->u32StaticDelayAccmCnt = 0;
       pstInputPort->u32StaticDelayAccmMS = 0;
    }

    memset(pstDelayWork, 0x33, sizeof(*pstDelayWork));
    kfree(pstDelayWork);
}

static void _MI_SYS_IMPL_Dup_LowLatencyMode_BufrefIntoInputBindQueue(MI_SYS_InputPort_t *pstInputPort, MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_BufRef_t *pstNewBufRef = NULL;
    MI_SYS_InputPort_DelayWork_t *pstDelayTask;
    struct timeval tv;

    BUG_ON(!pstInputPort ||pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
    BUG_ON(!pstInputPort->pstChannel ||pstInputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
    BUG_ON(!pstBufRef);

    if(_MI_SYS_IMPL_IsInputPortEnabled(pstInputPort) && pstInputPort->bEnableLowLatencyReceiveMode)
    {
        pstNewBufRef = mi_sys_dup_bufref(pstBufRef, _MI_SYS_IMPL_InputPortOnBufRefRelFunc, pstInputPort);
        if(!pstNewBufRef)
        {
           DBG_ERR("Dup BufRef Faild\n");
           return;
        }
        if(pstInputPort->u32LowLatencyDelayMs == 0)
        {
             mi_sys_add_to_queue_tail(pstNewBufRef,  &pstInputPort->stBindInputBufQueue);
             DBG_INFO("Add To Warking Queue Done, Dup pstNewBufRef:%p, pstBufAllocation:%p\n", pstNewBufRef, pstNewBufRef->pstBufAllocation);
             WAKE_UP_QUEUE_IF_NECESSARY(pstInputPort->pstChannel->pstModDev->inputWaitqueue);
        }
        else
        {
             //add to delay queue and start delay process task
             int iScheduledDelayTaskCnt;
             pstDelayTask = kmalloc(sizeof(*pstDelayTask), GFP_KERNEL);
             if(!pstDelayTask)
             {
                DBG_ERR("Failed to alloc delaytask struct in %s\n", __FUNCTION__);
                mi_sys_release_bufref(pstNewBufRef);
                return;
             }
             pstDelayTask->pstBufRef = pstNewBufRef;
             pstDelayTask->pstInputPort = pstInputPort;
             INIT_DELAYED_WORK(&pstDelayTask->stDelayWork, _MI_SYS_IMPL_DelayLowLatencyWorkFunc);

             iScheduledDelayTaskCnt = atomic_inc_return(&pstInputPort->scheduledDelayTaskCnt);
             BUG_ON(iScheduledDelayTaskCnt<=0);
             do_gettimeofday(&tv);
             pstDelayTask->u64TaskStartUs = tv.tv_sec*1000*1000+tv.tv_usec;
             schedule_delayed_work(&pstDelayTask->stDelayWork, msecs_to_jiffies(pstInputPort->u32LowLatencyDelayMs));
        }
    }
}

MI_S32 MI_SYS_IMPL_OutputBuf_Pre_Process_Notify(MI_SYS_BufInfo_t *buf)
{
    MI_SYS_BufRef_t *pstBufRef = container_of(buf, MI_SYS_BufRef_t, bufinfo);
    MI_SYS_OutputPort_t *pstOutputPort = NULL;
    MI_SYS_InputPort_t *pstCurInputPort = NULL;
    struct list_head *pos;

    if(pstBufRef->onRelCB != _MI_SYS_IMPL_OutputPortOnBufRefRelFunc)
    {
         BUG();
    }

    pstOutputPort = (MI_SYS_OutputPort_t*)pstBufRef->pCBData;

    BUG_ON(!pstOutputPort);
    BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
    BUG_ON(atomic_read(&pstBufRef->pstBufAllocation->ref_cnt) !=1);

    //mi_sys_chk_in_queue(pstBufRef, &pstOutputPort->stWorkingQueue);

    down(&pstOutputPort->stBindedInputListSemlock);
    list_for_each(pos, &pstOutputPort->stBindPeerInputPortList)
    {
        pstCurInputPort = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
        BUG_ON(pstCurInputPort->pstBindPeerOutputPort != pstOutputPort);

        if(_MI_SYS_IMPL_RcAcceptBuf(pstCurInputPort))
        {
            _MI_SYS_IMPL_Dup_LowLatencyMode_BufrefIntoInputBindQueue(pstCurInputPort, pstBufRef);
        }
        //skip step rc  since finish buf wiil to this
        //_MI_SYS_IMPL_StepRc(pstCurInputPort);
    }
    up(&pstOutputPort->stBindedInputListSemlock);
    if(pstBufRef->bPreNotified)
    {
        DBG_ERR("should not PreNotify same buffer for more than once!!!\n");
        MI_SYS_BUG();
    }
    pstBufRef->bPreNotified = TRUE;

    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_RewindBuf(MI_SYS_BufInfo_t *buf)
{
    MI_SYS_BufRef_t *pstBufRef = container_of(buf, MI_SYS_BufRef_t, bufinfo);

    DBG_INFO("pstBufRef:%p, pstBufAllocation:%p\n", pstBufRef, pstBufRef->pstBufAllocation);
    if(pstBufRef->onRelCB == _MI_SYS_IMPL_OutputPortOnBufRefRelFunc)
    {
        return _MI_SYS_IMPL_OutputPortRewindBuf(pstBufRef);
    }
    else if(pstBufRef->onRelCB == _MI_SYS_IMPL_InputPortOnBufRefRelFunc)
    {
        return _MI_SYS_IMPL_InputPortRewindBuf(pstBufRef);
    }
    else
    {
        MI_SYS_BUG();
    }

    DBG_ERR("Rewind Buffer Faild\n");
    return MI_ERR_SYS_ILLEGAL_PARAM;
}

static int _MI_SYS_IMPL_InputDataAvailable(MI_SYS_DRV_HANDLE miSysDrvHandle)
{
    MI_U32 u32ChannelIdx , u32PortId;
    MI_SYS_InputPort_t *pstInputPort;
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t *)miSysDrvHandle;

    if(!pstModDev || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__)
    {
        DBG_ERR("_MI_SYS_IMPL_InputDataAvailable pstModDev = NULL|| pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__ \n");
        return 0;
    }

    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    MI_SYS_BUG_ON(pstModDev->u32InputPortNum>MI_SYS_MAX_INPUT_PORT_CNT);

    //check all channels
    for(u32ChannelIdx = 0 ; u32ChannelIdx < pstModDev->u32DevChnNum;u32ChannelIdx++)
    {
        //Get a buf from each input port
        for(u32PortId = 0 ; u32PortId < pstModDev->u32InputPortNum ; u32PortId ++)
        {
            pstInputPort = pstModDev->astChannels[u32ChannelIdx].pastInputPorts[u32PortId];
            MI_SYS_BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
            if(!_MI_SYS_IMPL_IsInputPortEnabled(pstInputPort))
                continue;
            if(pstInputPort->stUsrInjectBufQueue.queue_buf_count || pstInputPort->stBindInputBufQueue.queue_buf_count)
                return 1;
        }
    }
    return 0;
}
int MI_SYS_IMPL_WaitOnInputTaskAvailable(MI_SYS_DRV_HANDLE miSysDrvHandle , MI_S32 s32TimeOutMs)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)(miSysDrvHandle);
    MI_S32 ret = -1;

    if(!pstModDev || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__)
    {
        DBG_ERR("MI_SYS_IMPL_WaitOnInputTaskAvailable pstModDev = NULL|| pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__ \n");
        return MI_ERR_SYS_FAILED;
    }
    if(s32TimeOutMs < 0)
        s32TimeOutMs = 20;
    ret = wait_event_timeout(pstModDev->inputWaitqueue, _MI_SYS_IMPL_InputDataAvailable(miSysDrvHandle), msecs_to_jiffies(s32TimeOutMs));
    if(ret == 0)
    {
        return MI_ERR_SYS_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, mi_sys_ChnBufInfo_t *pstChnBufInfo)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)(miSysDrvHandle);
    MI_SYS_Channel_t *pstChannel;
    MI_SYS_InputPort_t *pstInputPort = NULL;
    MI_SYS_OutputPort_t *pstOutputPort = NULL;
    int channelIdx = 0 ;
    int portId = 0;
    MI_SYS_BUG_ON(!pstModDev || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);

    MI_SYS_BUG_ON(pstChnBufInfo == NULL);
    MI_SYS_BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);

    pstChannel = &pstModDev->astChannels[u32ChnId];
    MI_SYS_BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    memset(pstChnBufInfo, 0, sizeof(*pstChnBufInfo));

    for(channelIdx = 0 ; channelIdx < pstModDev->u32DevChnNum ; channelIdx ++)
    {
        for(portId = 0 ; portId <  pstModDev->u32InputPortNum ; portId ++)
        {
            pstInputPort = pstChannel->pastInputPorts[portId];
            pstChnBufInfo->au16InputPortBindConnectBufPendingCnt[portId] = pstInputPort->stBindInputBufQueue.queue_buf_count;
            pstChnBufInfo->au16InputPortBufHoldByDrv[portId] = pstInputPort->stWorkingQueue.queue_buf_count;
            pstChnBufInfo->au16InputPortUserBufPendingCnt[portId] = pstInputPort->stUsrInjectBufQueue.queue_buf_count;
        }
        pstChnBufInfo->u32InputPortNum = pstModDev->u32InputPortNum;

        for(portId = 0 ; portId < pstModDev->u32OutputPortNum ; portId ++)
        {
            pstOutputPort = pstChannel->pastOutputPorts[portId];
            pstChnBufInfo->au16OutputPortBufInUsrFIFONum[portId] = pstOutputPort->u32UsrFifoCount;
            pstChnBufInfo->au16OutputPortBufUsrLockedNum[portId] = atomic_read(&pstOutputPort->usrLockedBufCnt);
            pstChnBufInfo->au16OutputPortBufHoldByDrv[portId] = pstOutputPort->stWorkingQueue.queue_buf_count;
            pstChnBufInfo->au16OutputPortBufTotalInUsedNum[portId] = atomic_read(&pstOutputPort->totalOutputPortInUsedBuf);
        }
        pstChnBufInfo->u32OutputPortNum = pstModDev->u32OutputPortNum;
    }
    return MI_SUCCESS;
}

//first parameter u8MMAHeapName can be NULL.
MI_S32 MI_SYS_IMPL_MmaAlloc(MI_U8 *u8MMAHeapName, MI_U32 u32blkSize ,MI_PHY *phyAddr)
{
    MI_SYS_BufferAllocation_t * tmp = NULL;
    MI_SYS_BufConf_t pstBufConfig;
    //first parameter u8MMAHeapName can be NULL.
    MI_SYS_BUG_ON(!u32blkSize);
    MI_SYS_BUG_ON(!phyAddr);
    pstBufConfig.eBufType = E_MI_SYS_BUFDATA_RAW;
    pstBufConfig.stRawCfg.u32Size = u32blkSize;
    tmp= mi_sys_alloc_from_mma_allocators( u8MMAHeapName, &pstBufConfig);//first parameter u8MMAHeapName can be NULL.
    DBG_INFO("\n");
    if(tmp)
    {
        MI_SYS_BUG_ON(tmp->stBufInfo.eBufType != E_MI_SYS_BUFDATA_RAW );
        MI_SYS_BUG_ON(atomic_read(&tmp->ref_cnt) != 0);

        *phyAddr = tmp->stBufInfo.stRawData.phyAddr ;
        return MI_SUCCESS;
    }
    else
    {
        DBG_ERR("fail\n");
        return MI_ERR_SYS_NOMEM;
    }
}

MI_S32 MI_SYS_IMPL_MmaFree(MI_PHY phyAddr)
{
    MI_SYS_BufferAllocation_t * tmp = NULL;
    tmp = mi_sys_mma_find_allocation_by_pa( phyAddr);

    if(tmp)
    {
        MI_SYS_BUG_ON(atomic_read(&tmp->ref_cnt) != 0);
        MI_SYS_BUG_ON(tmp->ops.OnRelease == NULL);
        tmp->ops.OnRelease(tmp);//for mma ,OnRelease is free

        return MI_SUCCESS;
    }
    else
    {
        DBG_ERR("find_allocation_by_pa fail phyAddr=0x%llx\n",phyAddr);
        return MI_ERR_SYS_ILLEGAL_PARAM;
    }
}

void _MI_SYS_IMPL_ChnInnerBufRefRelFunc(MI_SYS_BufRef_t *pstBufRef, void *pCBData)
{
    return;
}

MI_SYS_BufInfo_t* MI_SYS_IMPL_InnerAllocBufFromVbPool(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32blkSize, MI_PHY *phyAddr)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
    MI_SYS_MOD_DEV_t *pstModDevCh = NULL;
    MI_SYS_Channel_t *pstChannel;
    MI_SYS_BufferAllocation_t *pBufAllocation = NULL;
    MI_SYS_BufRef_t *pstBufRef = NULL;

    MI_SYS_BufConf_t pstBufConfig;

    MI_SYS_BUG_ON(!u32blkSize);
    pstBufConfig.eBufType = E_MI_SYS_BUFDATA_RAW;
    pstBufConfig.stRawCfg.u32Size = u32blkSize;

    DBG_INFO("u32ChnId:%d, u32DevChnNum:%d, u32blkSize:0x%0x\n", u32ChnId, pstModDev->u32DevChnNum, u32blkSize);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);

    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

    pstChannel = &pstModDev->astChannels[u32ChnId];
    pstModDevCh = pstChannel->pstModDev;
    MI_SYS_BUG_ON(!pstChannel);
    MI_SYS_BUG_ON(!pstModDevCh);

    pBufAllocation = mi_sys_alloc_from_allocator_collection(&pstModDevCh->stAllocatorCollection, &pstBufConfig);
    if(!pBufAllocation)
    {
        pBufAllocation = mi_sys_alloc_from_allocator_collection(&g_stGlobalVBPOOLAllocatorCollection,&pstBufConfig);
    }

    if(!pBufAllocation)
    {
        DBG_WRN("Can't Find VBPOOL Buf Allocation Error\n");
        return NULL;
    }

    pstBufRef = mi_sys_create_bufref(pBufAllocation, &pstBufConfig ,_MI_SYS_IMPL_ChnInnerBufRefRelFunc, pstChannel);
    if(pstBufRef == NULL)
    {
        pBufAllocation->ops.OnRelease(pBufAllocation);
        DBG_ERR("Create VBPOOL BufRef Faild\n");
        return NULL;
    }

    MI_SYS_BUG_ON(atomic_read(&pstBufRef->pstBufAllocation->ref_cnt) !=1);

    if(pstBufRef)
    {
        *phyAddr = pstBufRef->bufinfo.stRawData.phyAddr;
        return &(pstBufRef->bufinfo);
    }

    DBG_ERR("Vb Pool Get Buffer Faild\n");
    return NULL;
}


MI_S32 MI_SYS_IMPL_FreeInnerVbPool(MI_SYS_BufInfo_t *stBufInfo)
{
    MI_SYS_BufRef_t *pstBufRef = container_of(stBufInfo, MI_SYS_BufRef_t, bufinfo);

    DBG_INFO("MI_SYS_IMPL_FreeInnerVbPool pstBufAllocation:%p\n", pstBufRef->pstBufAllocation);

    MI_SYS_BUG_ON(!pstBufRef);
    MI_SYS_BUG_ON(pstBufRef->onRelCB != _MI_SYS_IMPL_ChnInnerBufRefRelFunc);
    MI_SYS_BUG_ON(atomic_read(&pstBufRef->pstBufAllocation->ref_cnt) !=1);
    mi_sys_release_bufref(pstBufRef);

    return MI_SUCCESS;
}

void * MI_SYS_IMPL_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size , MI_BOOL bCache)
{
    struct vm_struct *area;
    unsigned long pfn, count;
    unsigned long addr;
    unsigned long long cpu_bus_addr;
    int err;
    pgprot_t pgprot;
    MI_SYS_BUG_ON(!u32Size);

    if (bCache)
        pgprot = PAGE_KERNEL;
    else
        pgprot = pgprot_writecombine(PAGE_KERNEL);

    cpu_bus_addr = mi_sys_Miu2Cpu_BusAddr(u64PhyAddr);

    pfn = __phys_to_pfn(cpu_bus_addr);
    count = PAGE_ALIGN(u32Size) / PAGE_SIZE;

    if(!mi_sys_check_page_range_mapable(pfn, count))
    {
        //invalid parameter
        DBG_ERR("mi_sys_check_page_range_mapable failed \n");
        MI_SYS_BUG();
        return NULL;
    }
    area = get_vm_area_caller((count << PAGE_SHIFT), VM_MAP, __builtin_return_address(0));
    if(!area)
    {
        DBG_ERR("get_vm_area_caller failed\n");
        return NULL;
    }

    area->phys_addr = cpu_bus_addr;
    addr = (unsigned long)area->addr;

    err = ioremap_page_range(addr, addr + count*PAGE_SIZE,  cpu_bus_addr, pgprot);
    if(err)
    {
        vunmap((void *)addr);
        return NULL;
    }
    return (void*)addr;
}

void MI_SYS_IMPL_UnVmap(void *pVirtAddr)
{
/*
    struct vm_struct *area;
    area = find_vm_area(pVirtAddr);
    MI_SYS_BUG_ON(!area);
    //flush and invalid line cacahe of once used user space VA area to avoid random garbage on next owner of this phy mem
    mi_sys_buf_mgr_flush_inv_cache(area->addr, area->size);
*/
    vunmap(pVirtAddr);
}


#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 mi_sys_vbpool_allocator_proc_dump_attr(MI_SYS_DEBUG_HANDLE_t  handle,void *private)
{
    struct MI_SYS_Allocator_Collection_s *curr_stAllocatorCollection;
    struct list_head *pos;
    struct list_head *pos_2;
    mi_sys_Allocator_t * pstAllocator;
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    MI_SYS_MOD_DEV_t *pstModDev;
    struct MI_SYS_Allocator_Collection_s *pstAllocatorCollection =NULL;

    curr_stAllocatorCollection = (struct MI_SYS_Allocator_Collection_s *)private;

    handle.OnPrintOut(handle,"-------start of vbpool  dump attr--------\n");

    if(curr_stAllocatorCollection == &g_stGlobalVBPOOLAllocatorCollection)
    {
        handle.OnPrintOut(handle,"global vb pool info:\n");
        down(&g_stGlobalVBPOOLAllocatorCollection.semlock);
        handle.OnPrintOut(handle,"collection_size 0x%x\n",g_stGlobalVBPOOLAllocatorCollection.collection_size);
        list_for_each(pos, &g_stGlobalVBPOOLAllocatorCollection.list)
        {
            pstAllocatorRef = container_of(pos, MI_SYS_AllocatorRef_t, list);
            pstAllocator = pstAllocatorRef->pstAllocator;
            _vbpool_deal_with_mi_sys_Allocator_t(handle,pstAllocator);
        }
        handle.OnPrintOut(handle,"\n\n\n");
        up(&g_stGlobalVBPOOLAllocatorCollection.semlock);
    }
    else
    {
        MI_BOOL findAllocatorCollection = false;
        down(&mi_sys_global_dev_list_semlock);
        list_for_each(pos, &mi_sys_global_dev_list)
        {
            pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
            MI_SYS_BUG_ON(!pstModDev);
            pstAllocatorCollection = (struct MI_SYS_Allocator_Collection_s *)(&(pstModDev->stAllocatorCollection));
            if(pstAllocatorCollection == curr_stAllocatorCollection)
            {
                down(&pstAllocatorCollection->semlock);
                handle.OnPrintOut(handle,"[eModuleId 0x%x      u32DevId 0x%x] vb pool info:\n",pstModDev->eModuleId,pstModDev->u32DevId);
                handle.OnPrintOut(handle,"collection_size 0x%x\n",pstAllocatorCollection->collection_size);
                list_for_each(pos_2, &pstAllocatorCollection->list)
                {
                    pstAllocatorRef = container_of(pos_2, MI_SYS_AllocatorRef_t, list);
                    pstAllocator = pstAllocatorRef->pstAllocator;
                    _vbpool_deal_with_mi_sys_Allocator_t(handle,pstAllocator);
                }
                handle.OnPrintOut(handle,"\n\n\n");
                up(&pstAllocatorCollection->semlock);
                findAllocatorCollection = true;
                break;//found,so break.
            }
        }
        up(&mi_sys_global_dev_list_semlock);
        BUG_ON(false == findAllocatorCollection);
    }

    handle.OnPrintOut(handle,"-------end of vbpool  dump attr--------\n");

    return MI_SUCCESS;
}

MI_S32 mi_sys_vbpool_allocator_proc_exec_cmd(MI_SYS_DEBUG_HANDLE_t  handle,allocator_echo_cmd_info_t *cmd_info,void *private)
{
    struct list_head *pos;
    struct MI_SYS_Allocator_Collection_s *pstAllocatorCollection = (struct MI_SYS_Allocator_Collection_s *)private;
    mi_sys_Allocator_t * stsysAllocator;
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    MI_U32 size;
    MI_U32 count;
    MI_S32 ret;
    MI_U32 total_size;
    char mod_dev_name[256]={0};
    MI_SYS_MOD_DEV_t *pstModDev;
    struct MI_SYS_Allocator_Collection_s *tmp_pstAllocatorCollection;
    u8 mod_name[10];
    MI_BOOL found_offset = false;

    MI_SYS_BUG_ON(!pstAllocatorCollection);
    if(pstAllocatorCollection == &g_stGlobalVBPOOLAllocatorCollection)
    {
        memcpy(mod_dev_name,"global",sizeof("global"));
    }
    else
    {
        MI_BOOL find_dev = false;
        down(&mi_sys_global_dev_list_semlock);
        list_for_each(pos, &mi_sys_global_dev_list)
        {
            pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
            MI_SYS_BUG_ON(!pstModDev);
            {
                tmp_pstAllocatorCollection = (struct MI_SYS_Allocator_Collection_s *)(&(pstModDev->stAllocatorCollection));
                if(pstAllocatorCollection == tmp_pstAllocatorCollection)
                {
                    _MI_SYS_IMPL_ModuleIdToPrefixName(pstModDev->eModuleId,(char *)mod_name);
                    find_dev = true;
                    sprintf(mod_dev_name,"%s%d",mod_name,pstModDev->u32DevId);
                    break;
                }
            }
        }
        up(&mi_sys_global_dev_list_semlock);
        if(false == find_dev)
        {
            DBG_ERR("error,fail,not found mod and dev!!!\n");
            return E_MI_ERR_FAILED;
        }
    }

    if(cmd_info->dir_name[cmd_info->dir_size -1] == '/')
        sprintf(cmd_info->dir_name+cmd_info->dir_size,"vb_pool__%s__%u__%u.bin",mod_dev_name,cmd_info->offset,cmd_info->length);
    else
        sprintf(cmd_info->dir_name+cmd_info->dir_size,"/vb_pool__%s__%u__%u.bin",mod_dev_name,cmd_info->offset,cmd_info->length);

    printk("dir_and_file_name  is %s\n",cmd_info->dir_name);

    down(&pstAllocatorCollection->semlock);
    total_size = 0;
    found_offset = false;
    list_for_each(pos, &pstAllocatorCollection->list)
    {
        pstAllocatorRef = container_of(pos, MI_SYS_AllocatorRef_t, list);
        stsysAllocator = pstAllocatorRef->pstAllocator;
        ret = mi_sys_vbpool_allocator_get_info(stsysAllocator, &size ,&count);
        BUG_ON(ret != MI_SUCCESS);
        if(cmd_info->offset >=total_size && cmd_info->offset <= total_size + size * count)
        {
            if(cmd_info->offset + cmd_info->length <= total_size + size * count)
            {
                mi_sys_vbpool_allocator_dump_data((char *)(&cmd_info->dir_name[0]),stsysAllocator,cmd_info->offset - total_size, cmd_info->length);
                BUG_ON(ret != MI_SUCCESS);
                found_offset = true;
                break;//done
            }
            else
            {
                DBG_ERR("fail ,current only support dump data in same vb allocator ,not support in different pool of the pools!!!\n");
                DBG_ERR("this allocator [offset,length]=[0x%x,0x%x] ,but wanted [offset,length]=[0x%x,0x%x]\n",total_size,size,cmd_info->offset,cmd_info->length);
                up(&pstAllocatorCollection->semlock);
                return E_MI_ERR_ILLEGAL_PARAM;
            }
        }
        else
        {
            total_size += size*count;
        }
    }

    up(&pstAllocatorCollection->semlock);
    if(false == found_offset)
    {
        DBG_ERR("fail,error,not found the offset %u in this pstAllocatorCollection!!!!\n",cmd_info->offset);
        return E_MI_ERR_ILLEGAL_PARAM;
    }
    else
        return MI_SUCCESS;
}

#endif


static MI_S32 _MI_SYS_IMPL_ConfPubPools(struct MI_SYS_Allocator_Collection_s *stAllocatorCollection,MI_VB_PoolListConf_t  stPoolListConf)
{
    int i=0;
    struct list_head *pos ,*q;
    mi_sys_Allocator_t * stsysAllocator;
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    DBG_INFO("%s:%d stAllocatorCollection=%p\n",__FUNCTION__,__LINE__,stAllocatorCollection);
    MI_SYS_BUG_ON(!stAllocatorCollection);
    if(stPoolListConf.u32PoolListCnt > MI_VB_POOL_LIST_MAX_CNT)
    {
        DBG_ERR("%s %d u32PoolListCnt=0x%x  to large!\n",__FUNCTION__,__LINE__,stPoolListConf.u32PoolListCnt);
        return MI_ERR_SYS_ILLEGAL_PARAM;
    }
    if(0 == stPoolListConf.u32PoolListCnt)
    {
        DBG_ERR("%s %d u32PoolListCnt is 0,do nothing!!!\n",__FUNCTION__,__LINE__);
        return MI_ERR_SYS_ILLEGAL_PARAM;
    }
    DBG_INFO("%s:%d \n",__FUNCTION__,__LINE__);
    for(i=0;i<stPoolListConf.u32PoolListCnt;i++)
    {
        if((stPoolListConf.stPoolConf[i].u32BlkCnt == 0 )
                ||(stPoolListConf.stPoolConf[i].u32BlkSize == 0 ))
        {
            DBG_ERR(" INVALID_PARAM i=%d  u32BlkCnt=0x%x  u32BlkSize=0x%x  !!!\n",i,stPoolListConf.stPoolConf[i].u32BlkCnt,stPoolListConf.stPoolConf[i].u32BlkSize);
            return MI_ERR_SYS_ILLEGAL_PARAM;
        }
    }

    DBG_INFO("%s:%d \n",__FUNCTION__,__LINE__);
    down(&stAllocatorCollection->semlock);
    if(!list_empty(&stAllocatorCollection->list))//if not empty ,means busy.
    {
        up(&stAllocatorCollection->semlock);
        DBG_ERR(" fail\n");
        return MI_ERR_SYS_BUSY;
    }
    for(i=0;i<stPoolListConf.u32PoolListCnt;i++)
    {
        DBG_INFO("%s:%d  \n",__FUNCTION__,__LINE__);
        stsysAllocator = mi_sys_vbpool_allocator_create((char *)stPoolListConf.stPoolConf[i].u8MMAHeapName , stPoolListConf.stPoolConf[i].u32BlkSize, stPoolListConf.stPoolConf[i].u32BlkCnt);
        DBG_INFO("%s:%d \n",__FUNCTION__,__LINE__);
        if( !stsysAllocator)
        {
            DBG_ERR("%s:%d \n",__FUNCTION__,__LINE__);
            up(&stAllocatorCollection->semlock);
            goto release_already_allocated_Allocator;
        }
        DBG_INFO("%s:%d \n",__FUNCTION__,__LINE__);
        stsysAllocator->ops->OnRef(stsysAllocator);
        up(&stAllocatorCollection->semlock);
        mi_sys_attach_allocator_to_collection(stsysAllocator,stAllocatorCollection);
        stsysAllocator->ops->OnUnref(stsysAllocator);
        down(&stAllocatorCollection->semlock);

        DBG_INFO("%s:%d   stsysAllocator=%p\n",__FUNCTION__,__LINE__,stsysAllocator);
    }
    up(&stAllocatorCollection->semlock);
    DBG_INFO("%s:%d   \n",__FUNCTION__,__LINE__);
    return MI_SUCCESS;

release_already_allocated_Allocator:
    down(&stAllocatorCollection->semlock);
    DBG_INFO("%s:%d \n",__FUNCTION__,__LINE__);
    list_for_each_safe(pos,q, &stAllocatorCollection->list)
    {
        DBG_INFO("%s:%d \n",__FUNCTION__,__LINE__);
        pstAllocatorRef = container_of(pos, MI_SYS_AllocatorRef_t, list);
        stsysAllocator = pstAllocatorRef->pstAllocator;
        DBG_INFO("%s:%d \n",__FUNCTION__,__LINE__);

        //do not delete this code annotation :
        //the following means deatatch allocator,but here not directly call mi_sys_detach_allocator_from_collection.
        list_del(&pstAllocatorRef->list);
        stAllocatorCollection->collection_size--;
        MI_SYS_BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->OnUnref);
        pstAllocatorRef->pstAllocator->ops->OnUnref(pstAllocatorRef->pstAllocator);
        kfree(pstAllocatorRef);

        DBG_INFO("%s:%d \n",__FUNCTION__,__LINE__);
    }
    MI_SYS_BUG_ON(stAllocatorCollection->collection_size);
    up(&stAllocatorCollection->semlock);
    DBG_ERR(" fail  i=%d\n",i);
    return MI_ERR_SYS_FAILED;
}

static MI_S32 _MI_SYS_IMPL_RelPubPools(struct MI_SYS_Allocator_Collection_s *stAllocatorCollection)
{
    struct list_head *pos ,*q;
    mi_sys_Allocator_t * stsysAllocator;
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    MI_SYS_BUG_ON(!stAllocatorCollection);

    down(&stAllocatorCollection->semlock);
    if(list_empty(&stAllocatorCollection->list))
    {
        DBG_INFO("warning ,  stAllocatorCollection already empty!!!\n");
        up(&stAllocatorCollection->semlock);
        return MI_SUCCESS;
    }

    list_for_each_safe(pos,q, &stAllocatorCollection->list)
    {
        pstAllocatorRef = container_of(pos, MI_SYS_AllocatorRef_t, list);
        stsysAllocator = pstAllocatorRef->pstAllocator;

        //do not delete this code annotation :
        //the following means deatatch allocator,but here not directly call mi_sys_detach_allocator_from_collection.
        list_del(&pstAllocatorRef->list);
        stAllocatorCollection->collection_size--;
        MI_SYS_BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->OnUnref);
        pstAllocatorRef->pstAllocator->ops->OnUnref(pstAllocatorRef->pstAllocator);
        kfree(pstAllocatorRef);
    }
    MI_SYS_BUG_ON(stAllocatorCollection->collection_size);
    up(&stAllocatorCollection->semlock);

    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_ConfDevPubPools(MI_ModuleId_e   eModule, MI_U32 u32DevId , MI_VB_PoolListConf_t  stPoolListConf)
{
    MI_SYS_MOD_DEV_t *pstModDev;
    struct list_head *pos;
    struct MI_SYS_Allocator_Collection_s *pstAllocatorCollection =NULL;
    MI_S32 ret= MI_SUCCESS;
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_AllocatorProcfsOps_t Ops;
    MI_U8 prefix_name[10];
    MI_U8 name_buf[16];
#endif
    down(&mi_sys_global_dev_list_semlock);
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);
        if((eModule== pstModDev->eModuleId)
            &&(u32DevId == pstModDev->u32DevId))
        {
            pstAllocatorCollection = (struct MI_SYS_Allocator_Collection_s *)(&(pstModDev->stAllocatorCollection));
            break;
        }
    }
    if(pstAllocatorCollection)
    {
        ret= _MI_SYS_IMPL_ConfPubPools(pstAllocatorCollection,stPoolListConf);

#ifdef MI_SYS_PROC_FS_DEBUG
        if(ret == MI_SUCCESS)
        {
            memset(&Ops, 0 , sizeof(Ops));
            Ops.OnDumpAllocatorAttr = mi_sys_vbpool_allocator_proc_dump_attr;
            Ops.OnAllocatorExecCmd = mi_sys_vbpool_allocator_proc_exec_cmd;
            _MI_SYS_IMPL_ModuleIdToPrefixName(pstModDev->eModuleId,(char *)prefix_name);
            snprintf(name_buf, sizeof(name_buf), "vb_pool_%s%d",prefix_name,pstModDev->u32DevId);
            ret = mi_sys_allocator_proc_create(name_buf,&Ops,(void *)pstAllocatorCollection);
            MI_SYS_BUG_ON(ret != MI_SUCCESS);
        }
        else
        {
            DBG_ERR("_MI_SYS_IMPL_ConfPubPools fail!\n");
        }
#endif

    }
    else
    {
        DBG_ERR("not found Allocator Collection!\n");
        ret = MI_ERR_SYS_INVALID_DEVID;
    }

    up(&mi_sys_global_dev_list_semlock);

    return ret;
}
MI_S32 MI_SYS_IMPL_RelDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId)
{
    MI_SYS_MOD_DEV_t *pstModDev;
    struct list_head *pos;
    struct MI_SYS_Allocator_Collection_s *stAllocatorCollection =NULL;
    MI_S32 ret= MI_SUCCESS;
#ifdef MI_SYS_PROC_FS_DEBUG
    MI_U8 prefix_name[15];
    MI_U8 name_buf[MAX_ENTRY_NAME_LEN];
#endif
    down(&mi_sys_global_dev_list_semlock);
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);
        if((eModule== pstModDev->eModuleId )
            &&(u32DevId == pstModDev->u32DevId))
        {
            stAllocatorCollection = (struct MI_SYS_Allocator_Collection_s *)(&(pstModDev->stAllocatorCollection));
            break;
        }
    }
    if(stAllocatorCollection)
    {
        ret= _MI_SYS_IMPL_RelPubPools(stAllocatorCollection);
        #ifdef MI_SYS_PROC_FS_DEBUG
        _MI_SYS_IMPL_ModuleIdToPrefixName(pstModDev->eModuleId,(char *)prefix_name);
        snprintf(name_buf, sizeof(name_buf), "vb_pool_%s%d",prefix_name,pstModDev->u32DevId);
        mi_sys_allocator_proc_remove_allocator(name_buf);
        #endif
    }
    else
    {
        ret = MI_ERR_SYS_UNEXIST;
    }

    up(&mi_sys_global_dev_list_semlock);

    return ret;
}
MI_S32 MI_SYS_IMPL_ConfGloPubPools(MI_VB_PoolListConf_t  stPoolListConf)
{
    //confg to g_stGlobalVBPOOLAllocatorCollection
    MI_S32 ret;
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_AllocatorProcfsOps_t Ops;
#endif

    ret = _MI_SYS_IMPL_ConfPubPools(&g_stGlobalVBPOOLAllocatorCollection,stPoolListConf);

#ifdef MI_SYS_PROC_FS_DEBUG
    if(ret == MI_SUCCESS)
    {
        memset(&Ops, 0 , sizeof(Ops));
        Ops.OnDumpAllocatorAttr = mi_sys_vbpool_allocator_proc_dump_attr;
        Ops.OnAllocatorExecCmd = mi_sys_vbpool_allocator_proc_exec_cmd;
        ret = mi_sys_allocator_proc_create("vb_pool_global",&Ops,(void *)&g_stGlobalVBPOOLAllocatorCollection);
        MI_SYS_BUG_ON(ret != MI_SUCCESS);
    }
    else
    {
        DBG_INFO("mma_heap_name is NULL,do nothing about proc fs!\n");
    }
#endif

    return ret;
}
MI_S32 MI_SYS_IMPL_RelGloPubPools(void)
{
    int ret;
    ret = _MI_SYS_IMPL_RelPubPools(&g_stGlobalVBPOOLAllocatorCollection);
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_allocator_proc_remove_allocator("vb_pool_global");
#endif
    return ret;
}


MI_S32 MI_SYS_IMPL_EnableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;

    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >= pstModDev->u32OutputPortNum);

    pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId]->bPortEnable = TRUE;

    pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId]->u64UsrGetFifoBufQueueAddCnt = 0;
    pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId]->u64UsrGetFifoBufQueueDiscardCnt = 0;

    DBG_INFO("eModuleId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d\n", pstModDev->eModuleId, pstModDev->u32DevId, u32ChnId, u32PortId);

    return MI_SUCCESS;
}
MI_S32 MI_SYS_IMPL_DisableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    MI_SYS_BufRef_t *pstBufRef;
    MI_SYS_OutputPort_t *pstOutputPort;

    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;

    MI_SYS_BUG_ON(!pstModDev
          || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >= pstModDev->u32OutputPortNum);

    pstOutputPort = pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
    MI_SYS_BUG_ON(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
    if(!pstOutputPort->bPortEnable)
        return MI_SUCCESS;
    pstOutputPort->bPortEnable = FALSE;

    pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stUsrGetFifoBufQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stUsrGetFifoBufQueue);
    }

    pstOutputPort->u64UsrGetFifoBufQueueAddCnt = 0;
    pstOutputPort->u64UsrGetFifoBufQueueDiscardCnt = 0;

    pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stDrvBkRefFifoQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stDrvBkRefFifoQueue);
    }

    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_SetOutputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf)
{
    MI_SYS_OutputPort_t *pstOutputPort;
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;

    MI_SYS_BUG_ON(!pstModDev
          || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >= pstModDev->u32OutputPortNum);

    pstOutputPort = pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
    MI_SYS_BUG_ON(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
    _mi_sys_mergeBufExtraRequirement(&pstOutputPort->stOutputBufExtraRequirements,pstBufExtraConf);
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_SetInputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf)
{
    MI_SYS_InputPort_t *pstInputPort;
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;

    MI_SYS_BUG_ON(!pstModDev
        || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >= pstModDev->u32InputPortNum);

    pstInputPort = pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
    MI_SYS_BUG_ON(!pstInputPort || pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
    _mi_sys_mergeBufExtraRequirement(&pstInputPort->stInputBufExtraConf,pstBufExtraConf);
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_EnableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t *)miSysDrvHandle;

    MI_SYS_BUG_ON(!pstModDev
          || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >= pstModDev->u32InputPortNum);

    DBG_INFO("u32ChnId:%d, u32PortId:%d\n", u32ChnId, u32PortId);

    pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId]->bPortEnable = TRUE;
    //pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId]->bEnableLowLatencyReceiveMode = FALSE;

    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_SetInputPortSidebandMsg(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_U64 u64SidebandMsg)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t *)miSysDrvHandle;

    MI_SYS_BUG_ON(!pstModDev
          || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >= pstModDev->u32InputPortNum);

    DBG_INFO("u32ChnId:%d, u32PortId:%d\n", u32ChnId, u32PortId);

    pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId]->u64SidebandMsg = u64SidebandMsg;

    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_EnableLowlatencyPreProcessMode(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_BOOL bEnable, MI_U32 delayMs)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t *)miSysDrvHandle;

    BUG_ON(!pstModDev
          || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);
    BUG_ON(u32PortId >= pstModDev->u32InputPortNum);

    DBG_INFO("u32ChnId:%d, u32PortId:%d\n", u32ChnId, u32PortId);

    pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId]->bEnableLowLatencyReceiveMode = bEnable;
    pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId]->u32LowLatencyDelayMs = delayMs;

    if(bEnable && delayMs>30)
    {
         DBG_ERR("MOD:%d, DEV:%d, u32ChnId:%d, u32PortId:%d, lowlatencyMode delay to large(%dms)\n", pstModDev->eModuleId,
                            pstModDev->u32DevId,  u32ChnId, u32PortId, delayMs);
    }
    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_DisableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    MI_SYS_BufRef_t *pstBufRef;
    MI_SYS_InputPort_t *pstInputPort;

    int iScheduledDelayTaskCnt;
    MI_U32 u32LoopCnt=0;
    int last_jiffies;

    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;

    MI_SYS_BUG_ON(!pstModDev
        || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32PortId >= pstModDev->u32InputPortNum);

    pstInputPort = pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
    MI_SYS_BUG_ON(!pstInputPort || pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
    if(!pstInputPort->bPortEnable)
    {
        return MI_SUCCESS;
    }
    pstInputPort->bPortEnable = FALSE;

    pstInputPort->u32LastStaticDelayTotalMS = 0;
    pstInputPort->u32LastStaticDelayTotalCnt = 0;
    pstInputPort->u32StaticDelayAccmMS = 0;
    pstInputPort->u32StaticDelayAccmCnt=0;

    last_jiffies = jiffies;
    //wait all inputport delay low latency task done
    while((iScheduledDelayTaskCnt = atomic_read(&pstInputPort->scheduledDelayTaskCnt)))
    {
        BUG_ON(iScheduledDelayTaskCnt < 0);
        msleep(1);
        u32LoopCnt++;

        if(u32LoopCnt%1000 == 0)
        {
           DBG_ERR(" waiting delay input low latency task for more than %ums\n", jiffies_to_msecs(jiffies-last_jiffies));
        }
    }

    pstBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stBindInputBufQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stBindInputBufQueue);
    }

    pstBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stUsrInjectBufQueue);
    while(pstBufRef)
    {
        mi_sys_release_bufref(pstBufRef);
        pstBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stUsrInjectBufQueue);
    }

    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_EnableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
    DBG_INFO("%p\n", pstModDev);

    DBG_INFO("%p, 0x%x, %d, u32ChnId:%d\n", pstModDev, pstModDev->u32MagicNumber, pstModDev->u32DevChnNum, u32ChnId);

    MI_SYS_BUG_ON(!pstModDev
        || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

    MI_SYS_BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);

    pstModDev->astChannels[u32ChnId].bChnEnable= TRUE;

    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_DisableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId)
{
    MI_BOOL bEnable;
    MI_U32 u32PortIdx;
    MI_SYS_Channel_t *pstChannel;

    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;

    MI_SYS_BUG_ON(!pstModDev
          || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
    MI_SYS_BUG_ON(pstModDev->u32DevChnNum > MI_SYS_MAX_DEV_CHN_CNT);
    MI_SYS_BUG_ON(u32ChnId >= pstModDev->u32DevChnNum);

    pstChannel = &pstModDev->astChannels[u32ChnId];

    MI_SYS_BUG_ON(!pstChannel || pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    if(!pstChannel->bChnEnable)
        return MI_SUCCESS;
    pstChannel->bChnEnable = FALSE;

    for(u32PortIdx = 0 ; u32PortIdx < pstModDev->u32OutputPortNum ; u32PortIdx ++)
    {
        bEnable = pstChannel->pastOutputPorts[u32PortIdx]->bPortEnable;
        MI_SYS_IMPL_DisableOutputPort(miSysDrvHandle , u32ChnId , u32PortIdx);
        pstChannel->pastOutputPorts[u32PortIdx]->bPortEnable = bEnable;
    }
    for(u32PortIdx = 0 ; u32PortIdx < pstModDev->u32InputPortNum ; u32PortIdx ++)
    {
        bEnable = pstChannel->pastInputPorts[u32PortIdx]->bPortEnable;
        MI_SYS_IMPL_DisableInputPort(miSysDrvHandle , u32ChnId , u32PortIdx);
        pstChannel->pastInputPorts[u32PortIdx]->bPortEnable = bEnable;
    }

    return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32InputportId, mi_sys_Allocator_t *pstUserAllocator)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
    MI_SYS_InputPort_t *pstInputPort;
    MI_SYS_Channel_t *pstChannel;

    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

    MI_SYS_BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32InputportId >=pstModDev->u32InputPortNum);

    pstChannel = &pstModDev->astChannels[u32ChnId];

    MI_SYS_BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    pstInputPort = pstChannel->pastInputPorts[u32InputportId];
    MI_SYS_BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

    //_sys_Allocator_t *pstTmpAllocator = pstInputPort->pstCusBufAllocator;

    if(pstUserAllocator)
        pstUserAllocator->ops->OnRef(pstUserAllocator);

    if(pstInputPort->pstCusBufAllocator)
        pstInputPort->pstCusBufAllocator->ops->OnUnref(pstInputPort->pstCusBufAllocator);

    pstInputPort->pstCusBufAllocator = pstUserAllocator;

    return MI_SUCCESS;
}
MI_S32 MI_SYS_IMPL_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32InputportId, mi_sys_Allocator_t *pstUserAllocator)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
    MI_SYS_OutputPort_t *pstPeerOutputPort;
    MI_SYS_InputPort_t *pstInputPort;
    MI_SYS_Channel_t *pstChannel;

    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

    MI_SYS_BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(u32InputportId >=pstModDev->u32InputPortNum);

    pstChannel = &pstModDev->astChannels[u32ChnId];

    MI_SYS_BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    pstInputPort = pstChannel->pastInputPorts[u32InputportId];
    MI_SYS_BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

    pstPeerOutputPort = pstInputPort->pstBindPeerOutputPort;

    if(pstPeerOutputPort)
    {
        MI_SYS_BUG_ON(list_empty(&pstInputPort->stBindRelationShipList));

        MI_SYS_BUG_ON(pstPeerOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
        _MI_SYS_IMPL_CheckInputOutputBindCoherence(pstInputPort, pstPeerOutputPort);

        if(pstUserAllocator)
            pstUserAllocator->ops->OnRef(pstUserAllocator);

        if(pstPeerOutputPort->pstCusBufAllocator)
            pstPeerOutputPort->pstCusBufAllocator->ops->OnUnref(pstPeerOutputPort->pstCusBufAllocator);

        pstPeerOutputPort->pstCusBufAllocator = pstUserAllocator;

        return MI_SUCCESS;
    }
    else
    {
        MI_SYS_BUG_ON(!list_empty(&pstInputPort->stBindRelationShipList));
        return MI_ERR_SYS_ILLEGAL_PARAM;
    }

}


static MI_U32 u32VdecVbPoolFlag =0;
MI_S32 MI_SYS_IMPL_Set_VDEC_VBPool_Flag(MI_U32 VdecVbPoolFlag)
{
    u32VdecVbPoolFlag = VdecVbPoolFlag;
    return MI_SUCCESS;
}

MI_U32 MI_SYS_IMPL_Get_VDEC_VBPool_Flag(void)
{
    return u32VdecVbPoolFlag;
}
///////////////////////////POLL FD START
#define MI_SYS_MTTEST (0)
#if MI_SYS_MTTEST
///MT TEST START
MI_SYS_OutputPort_t stTest1;
MI_SYS_OutputPort_t stTest2;
struct task_struct *ptskTsPollTask = NULL;
wait_queue_head_t stPutWaitQueueHead;

void _MI_SYS_IMPL_SetPollState(MI_COMMON_PollFlag_e ePollFlag, MI_SYS_OutputPort_t *pstOutputPort);

static int _MI_SYS_IMPL_MTTask(void *pUsrData)
{
    while (1)
    {
        interruptible_sleep_on_timeout(&stPutWaitQueueHead, 5000);
        DBG_INFO("LOOP IN\n");
        static int test = 0;
        if (stTest1.pstPollFile)
        {
            if (test%2)
            {
                _MI_SYS_IMPL_SetPollState(&stTest1);
            }
            else
            {
                _MI_SYS_IMPL_SetPollState(&stTest1);
            }
        }

        if (stTest2.pstPollFile)
        {
            if (test%2)
            {
                _MI_SYS_IMPL_SetPollState(E_MI_COMMON_POLL_NOT_READY, &stTest2);
            }
            else
            {
                _MI_SYS_IMPL_SetPollState(E_MI_COMMON_FRAME_READY_FOR_READ, &stTest2);
            }
        }
        test++;
    }

    return 0;
}

void Init(void)
{
    stTest1.stMTtestChnPort.eModId = E_MI_MODULE_ID_VENC;
    stTest1.stMTtestChnPort.u32DevId = 0;
    stTest1.stMTtestChnPort.u32ChnId = 1;
    stTest1.stMTtestChnPort.u32PortId = 1;

    stTest2.stMTtestChnPort.eModId = E_MI_MODULE_ID_VENC;
    stTest2.stMTtestChnPort.u32DevId = 0;
    stTest2.stMTtestChnPort.u32ChnId = 2;
    stTest2.stMTtestChnPort.u32PortId = 1;

    init_waitqueue_head(&stPutWaitQueueHead);
    ptskTsPollTask = kthread_create(_MI_SYS_IMPL_MTTask, NULL, "poll_mt");
    wake_up_process(ptskTsPollTask);
}

MI_SYS_OutputPort_t *_MI_SYS_GetOutputPort(MI_SYS_ChnPort_t *pstChnPort)
{
    if (stTest1.stMTtestChnPort.eModId == pstChnPort->eModId &&
        stTest1.stMTtestChnPort.u32ChnId == pstChnPort->u32ChnId &&
        stTest1.stMTtestChnPort.u32DevId == pstChnPort->u32DevId &&
        stTest1.stMTtestChnPort.u32PortId == pstChnPort->u32PortId)
    {
        return &stTest1;
    }
    else if (stTest2.stMTtestChnPort.eModId == pstChnPort->eModId &&
        stTest2.stMTtestChnPort.u32ChnId == pstChnPort->u32ChnId &&
        stTest2.stMTtestChnPort.u32DevId == pstChnPort->u32DevId &&
        stTest2.stMTtestChnPort.u32PortId == pstChnPort->u32PortId)
    {
        return &stTest2;
    }
    return NULL;
}
#endif
///MT TEST END

static void _MI_SYS_IMPL_SetPollState(MI_SYS_OutputPort_t *pstOutputPort)
{
    if (pstOutputPort && pstOutputPort->pstPollFile)
    {
        down(&(pstOutputPort->stPollSemlock));
        if (pstOutputPort->pstPollFile)
        {
            MI_COMMON_WakeUpClient(pstOutputPort->pstPollFile);
        }
        up(&(pstOutputPort->stPollSemlock));
#if MI_SYS_MTTEST
         if (stTest1.stMTtestChnPort.eModId == pstOutputPort->stMTtestChnPort.eModId &&
             stTest1.stMTtestChnPort.u32ChnId == pstOutputPort->stMTtestChnPort.u32ChnId &&
             stTest1.stMTtestChnPort.u32DevId == pstOutputPort->stMTtestChnPort.u32DevId &&
             stTest1.stMTtestChnPort.u32PortId == pstOutputPort->stMTtestChnPort.u32PortId)
        {
            DBG_INFO("Set Test1:%d\n", pstOutputPort->ePollFlag);
        }
        else if (stTest2.stMTtestChnPort.eModId == pstOutputPort->stMTtestChnPort.eModId &&
            stTest2.stMTtestChnPort.u32ChnId == pstOutputPort->stMTtestChnPort.u32ChnId &&
            stTest2.stMTtestChnPort.u32DevId == pstOutputPort->stMTtestChnPort.u32DevId &&
            stTest2.stMTtestChnPort.u32PortId == pstOutputPort->stMTtestChnPort.u32PortId)
        {
            DBG_INFO("Set Test2:%d\n", pstOutputPort->ePollFlag);
        }
#endif
    }
}

static MI_COMMON_PollFlag_e _MI_SYS_IMPL_PollFdState(MI_COMMON_PollFile_t *c)
{
    MI_COMMON_PollFlag_e ePollFlag = E_MI_COMMON_POLL_NOT_READY;
    MI_SYS_OutputPort_t *pstOutputPort = (MI_SYS_OutputPort_t *)(c->private);
    if (c && pstOutputPort && pstOutputPort->pstPollFile)
    {
        down(&(pstOutputPort->stPollSemlock));
        if (pstOutputPort->pstPollFile)
        {
            if(pstOutputPort->stUsrGetFifoBufQueue.queue_buf_count)
                ePollFlag = E_MI_COMMON_FRAME_READY_FOR_READ;
            else
                ePollFlag = E_MI_COMMON_POLL_NOT_READY;
        }
        up(&(pstOutputPort->stPollSemlock));
#if MI_SYS_MTTEST
        if (stTest1.stMTtestChnPort.eModId == pstOutputPort->stMTtestChnPort.eModId &&
            stTest1.stMTtestChnPort.u32ChnId == pstOutputPort->stMTtestChnPort.u32ChnId &&
            stTest1.stMTtestChnPort.u32DevId == pstOutputPort->stMTtestChnPort.u32DevId &&
            stTest1.stMTtestChnPort.u32PortId == pstOutputPort->stMTtestChnPort.u32PortId)
        {
            DBG_INFO("Get Test1:%d\n", ePollFlag);
        }
        else if (stTest2.stMTtestChnPort.eModId == pstOutputPort->stMTtestChnPort.eModId &&
            stTest2.stMTtestChnPort.u32ChnId == pstOutputPort->stMTtestChnPort.u32ChnId &&
            stTest2.stMTtestChnPort.u32DevId == pstOutputPort->stMTtestChnPort.u32DevId &&
            stTest2.stMTtestChnPort.u32PortId == pstOutputPort->stMTtestChnPort.u32PortId)
        {
            DBG_INFO("Get Test2:%d\n", ePollFlag);
        }
#endif
    }
    return ePollFlag;
}

static void _MI_SYS_IMPL_CloseFd(MI_COMMON_PollFile_t *c)
{
    MI_SYS_OutputPort_t *pstOutputPort;
    MI_SYS_BUG_ON(!c);
    pstOutputPort = (MI_SYS_OutputPort_t *)(c->private);
    if (pstOutputPort)
    {
        down(&(pstOutputPort->stPollSemlock));
        if (pstOutputPort->pstPollFile)
        {
            pstOutputPort->pstPollFile = NULL;
        }
        up(&(pstOutputPort->stPollSemlock));

    }
}

MI_S32 MI_SYS_IMPL_GetFd(MI_SYS_ChnPort_t *pstChnPort, MI_S32 *s32Fd)
{
    MI_SYS_OutputPort_t *pstOutputPort = NULL;

    MI_SYS_BUG_ON(!s32Fd);
    MI_SYS_BUG_ON(!pstChnPort);

    DBG_INFO("eModId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d\n",
        pstChnPort->eModId,
        pstChnPort->u32DevId,
        pstChnPort->u32ChnId,
        pstChnPort->u32PortId);
#if MI_SYS_MTTEST
    pstOutputPort = _MI_SYS_GetOutputPort(pstChnPort);
#else
    pstOutputPort = _MI_SYS_IMPL_GetOutputPortInfo(pstChnPort);
#endif
    if (pstOutputPort)
    {
        down(&(pstOutputPort->stPollSemlock));
        if (NULL == pstOutputPort->pstPollFile)
        {
            ///Only Open Output Port Fd Need Init Poll State
           // pstOutputPort->ePollFlag = E_MI_COMMON_POLL_NOT_READY;
        }

        MI_COMMON_GetPollFd(&(pstOutputPort->pstPollFile), _MI_SYS_IMPL_PollFdState, _MI_SYS_IMPL_CloseFd, s32Fd);
        pstOutputPort->pstPollFile->private = pstOutputPort;
        up(&(pstOutputPort->stPollSemlock));
        DBG_INFO("pstPollFile:%p\n", pstOutputPort->pstPollFile);
        return MI_SUCCESS;
    }
    else
        return MI_ERR_SYS_ILLEGAL_PARAM;
}
///////////////////////////POLL FD END

 MI_PHY MI_SYS_IMPL_Cpu2Miu_BusAddr(MI_PHY cpu_addr)
{
    if(cpu_addr >= (MI_PHY)ARM_MIU2_BUS_BASE)
    {
        return (MI_PHY)(cpu_addr - ARM_MIU2_BUS_BASE+ARM_MIU2_BASE_ADDR);
    }
    else if(cpu_addr >= (MI_PHY)ARM_MIU1_BUS_BASE)
    {
        return  (MI_PHY)(cpu_addr - ARM_MIU1_BUS_BASE+ARM_MIU1_BASE_ADDR);
    }
    else if(cpu_addr >= (MI_PHY)ARM_MIU0_BUS_BASE)
    {
        return (MI_PHY)(cpu_addr - ARM_MIU0_BUS_BASE+ARM_MIU0_BASE_ADDR);
    }
    else
    {
        DBG_ERR("cpu_addr=0x%llx\n",cpu_addr);
        DBG_ERR("ARM_MIU2_BUS_BASE=0x%llx  ARM_MIU2_BASE_ADDR=0x%llx \n",(MI_PHY)ARM_MIU2_BUS_BASE,(MI_PHY)ARM_MIU2_BASE_ADDR);
        DBG_ERR("ARM_MIU1_BUS_BASE=0x%llx  ARM_MIU1_BASE_ADDR=0x%llx   \n",(MI_PHY)ARM_MIU1_BUS_BASE,(MI_PHY)ARM_MIU1_BASE_ADDR);
        DBG_ERR("ARM_MIU0_BUS_BASE=0x%llx  ARM_MIU0_BASE_ADDR=0x%llx    \n",(MI_PHY)ARM_MIU0_BUS_BASE,(MI_PHY)ARM_MIU0_BASE_ADDR);
        MI_SYS_BUG();
    }
    return (MI_PHY)-1ULL;
}

 unsigned long long MI_SYS_IMPL_Miu2Cpu_BusAddr(MI_PHY miu_phy_addr)
{
    if(miu_phy_addr >= (MI_PHY)ARM_MIU2_BASE_ADDR)
    {
        return (MI_PHY)(miu_phy_addr + ARM_MIU2_BUS_BASE-ARM_MIU2_BASE_ADDR);
    }
    else if(miu_phy_addr >= (MI_PHY)ARM_MIU1_BASE_ADDR)
    {
        return  (MI_PHY)(miu_phy_addr + ARM_MIU1_BUS_BASE-ARM_MIU1_BASE_ADDR);
    }
    else if(miu_phy_addr >= (MI_PHY)ARM_MIU0_BASE_ADDR)
    {
        return (MI_PHY)(miu_phy_addr + ARM_MIU0_BUS_BASE-ARM_MIU0_BASE_ADDR);
    }
    else
    {
        DBG_ERR("miu_phy_addr=0x%llx\n");
        MI_SYS_BUG();
    }
    return (MI_PHY)-1ULL;
}

 MI_SYS_BufferAllocation_t* MI_SYS_IMPL_AllocMMALowLevelBufAllocation(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_SYS_BufConf_t *pstBufConfig)
{
    MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;

    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

    MI_SYS_BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
    MI_SYS_BUG_ON(!pstBufConfig);

    return mi_sys_alloc_from_mma_allocators(pstModDev->astChannels[u32ChnId].pu8MMAHeapName, pstBufConfig);
}
