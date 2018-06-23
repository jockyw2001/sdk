#include "mi_sys_buf_mgr.h"
#include <linux/slab.h>
#include "mi_sys_impl.h"
#include "mi_sys_debug.h"
#include "mi_print.h"
#include <linux/fs.h>
#include <linux/mman.h>
#include <asm/cacheflush.h>
#include <asm/outercache.h>
#include <linux/hashtable.h>
#include "mi_print.h"
#include <linux/anon_inodes.h>
#include <linux/mman.h>
#include <linux/file.h>
#include <linux/hardirq.h>

#include <linux/version.h>
#include <linux/kernel.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
    #include <../../mstar2/drv/mma_heap/mdrv_mma_heap.h>
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    #include <mdrv_mma_heap.h>
    #include "ms_platform.h"
#else
    #error not support this kernel version
#endif
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/pgtable.h>
#include <asm/page.h>



#include "mi_sys_debug.h"

#include "mi_sys_trace.h"

#define  _MI_SYS_GENERAL_ALLOCATION_MAGIC_NUM_ 0x414C4C43

extern struct kmem_cache *mi_sys_bufref_cachep;
atomic_t _gBufRefcount = ATOMIC_INIT(0);
atomic_t _gBufcount = ATOMIC_INIT(0);

void generic_allocation_on_ref(struct MI_SYS_BufferAllocation_s *pstBufAllocation)
{
    int ref_cnt;
    MI_SYS_BUG_ON(!pstBufAllocation);
    MI_SYS_BUG_ON(pstBufAllocation->ops.OnRef != generic_allocation_on_ref);
    MI_SYS_BUG_ON(pstBufAllocation->u32MagicNumber != _MI_SYS_GENERAL_ALLOCATION_MAGIC_NUM_);
    ref_cnt = atomic_inc_return(&pstBufAllocation->ref_cnt);

    MI_SYS_BUG_ON(ref_cnt<=0);
}
EXPORT_SYMBOL(generic_allocation_on_ref);

void generic_allocation_on_unref(struct MI_SYS_BufferAllocation_s *pstBufAllocation)
{
    int ref_cnt;
    MI_SYS_BUG_ON(!pstBufAllocation);
    MI_SYS_BUG_ON(pstBufAllocation->ops.OnUnref != generic_allocation_on_unref);
    MI_SYS_BUG_ON(pstBufAllocation->u32MagicNumber != _MI_SYS_GENERAL_ALLOCATION_MAGIC_NUM_);

    ref_cnt = atomic_dec_return(&pstBufAllocation->ref_cnt);

    MI_SYS_BUG_ON(ref_cnt<0);
    if(ref_cnt ==0)
    {
        pstBufAllocation->ops.OnRelease(pstBufAllocation);
    }
}
EXPORT_SYMBOL(generic_allocation_on_unref);

//use this function to replace mi_sys_buf_mgr_init_std_allocation function
void generic_allocation_init(MI_SYS_BufferAllocation_t *pstBufAllocation,buf_allocation_ops_t *ops)
{
    MI_SYS_BUG_ON(!pstBufAllocation);
    MI_SYS_BUG_ON(!ops);
    MI_SYS_BUG_ON(!ops->OnRef || !ops->OnUnref ||!ops->OnRelease);

    pstBufAllocation->u32MagicNumber = _MI_SYS_GENERAL_ALLOCATION_MAGIC_NUM_;
    pstBufAllocation->ops = *ops;
    atomic_set(&(pstBufAllocation->ref_cnt), 0);
}
EXPORT_SYMBOL(generic_allocation_init);

MI_S32 generic_allocator_on_ref(struct mi_sys_Allocator_s *pstAllocator)
{
    int ref_cnt;
    MI_SYS_BUG_ON(!pstAllocator);
    MI_SYS_BUG_ON(!pstAllocator->ops || pstAllocator->ops->OnRef != generic_allocator_on_ref);

    ref_cnt = atomic_inc_return(&pstAllocator->ref_cnt);

    MI_SYS_BUG_ON(ref_cnt<=0);
    return MI_SUCCESS;
}
EXPORT_SYMBOL(generic_allocator_on_ref);

MI_S32 generic_allocator_on_unref(struct mi_sys_Allocator_s *pstAllocator)
{
    int ref_cnt;
    MI_SYS_BUG_ON(!pstAllocator);
    MI_SYS_BUG_ON(!pstAllocator->ops || (pstAllocator->ops->OnUnref != generic_allocator_on_unref));

    ref_cnt = atomic_dec_return(&pstAllocator->ref_cnt);

    MI_SYS_BUG_ON(ref_cnt<0);
    if(ref_cnt ==0)
        pstAllocator->ops->OnRelease(pstAllocator);
    return MI_SUCCESS;
}
EXPORT_SYMBOL(generic_allocator_on_unref);
void generic_allocator_init(mi_sys_Allocator_t *pstAllocator, buf_allocator_ops_t *ops)
{
    MI_SYS_BUG_ON(!pstAllocator);

    MI_SYS_BUG_ON(!ops || !ops->OnRef || !ops->OnUnref || !ops->OnRelease ||
                  !ops->alloc ||!ops->suit_bufconfig);

    pstAllocator->u32MagicNumber = __MI_SYS_ALLOCATOR_MAGIC_NUM__;
    INIT_LIST_HEAD(&pstAllocator->list);
    atomic_set(&(pstAllocator->ref_cnt), 0);
    pstAllocator->ops = ops;
}
EXPORT_SYMBOL(generic_allocator_init);
/*
input:
pstBufConfig
pstFrameDataInfo : only Frame need this parameter,meta and raw data no need.
output:
pstBufAllocation
*/
/*
this function will be used by user,not be used by other internal VB POOL functions.
*/
void mi_sys_buf_mgr_fill_bufinfo(MI_SYS_BufInfo_t *pstBufInfo,
                                 MI_SYS_BufConf_t *pstBufConfig, MI_SYS_FrameSpecial_t *pstFrameDataInfo, MI_PHY  phyaddr)
{
    MI_SYS_BUG_ON(!pstBufInfo);
    MI_SYS_BUG_ON(!pstBufConfig);
    if(E_MI_SYS_BUFDATA_FRAME == pstBufConfig->eBufType)
    {
        MI_SYS_BUG_ON(!pstFrameDataInfo);
    }
    else
    {
        //do nothing,if buf not frame,no care pstFrameDataInfo,and it can be NULL.
    }

    memset(pstBufInfo, 0, sizeof(*pstBufInfo));

    pstBufInfo->eBufType = pstBufConfig->eBufType;
    pstBufInfo->u64Pts = pstBufConfig->u64TargetPts;
    pstBufInfo->bEndOfStream = false;//to be discuss
    pstBufInfo->bUsrBuf = false;//to be discuss
    //pstBufInfo->u64SidebandMsg = 0;

    if(E_MI_SYS_BUFDATA_RAW == pstBufConfig->eBufType)
    {
        pstBufInfo->stRawData.u32BufSize = pstBufConfig->stRawCfg.u32Size;
        pstBufInfo->stRawData.phyAddr = phyaddr;
        //to be discuss:how about other parameters?
    }
    else if(E_MI_SYS_BUFDATA_FRAME == pstBufConfig->eBufType)
    {
        int i;
        pstBufInfo->stFrameData.eTileMode =   E_MI_SYS_FRAME_TILE_MODE_NONE;//set a default value
        pstBufInfo->stFrameData.ePixelFormat = pstBufConfig->stFrameCfg.eFormat;
        pstBufInfo->stFrameData.eCompressMode =   E_MI_SYS_COMPRESS_MODE_NONE;//set a default value
        pstBufInfo->stFrameData.eFrameScanMode = pstBufConfig->stFrameCfg.eFrameScanMode;
        pstBufInfo->stFrameData.eFieldType  =  E_MI_SYS_FIELDTYPE_NONE;//set a default value
#if 0//may future add stWindowRect
        pstBufInfo->stFrameData.stWindowRect.u16X = 0;
        pstBufInfo->stFrameData.stWindowRect.u16Y = 0;
        pstBufInfo->stFrameData.stWindowRect.u16Width = pstBufConfig->stFrameCfg.u16Width;
        pstBufInfo->stFrameData.stWindowRect.u16Height = pstBufConfig->stFrameCfg.u16Height;
#endif
        pstBufInfo->stFrameData.u16Width = pstBufConfig->stFrameCfg.u16Width;
        pstBufInfo->stFrameData.u16Height = pstBufConfig->stFrameCfg.u16Height;

        for( i=0; i<3 && pstFrameDataInfo->u32Stride[i]; i++)
        {
            pstBufInfo->stFrameData.phyAddr[i] =  phyaddr+pstFrameDataInfo->phy_offset[i];
            pstBufInfo->stFrameData.u32Stride[i] =  pstFrameDataInfo->u32Stride[i];
        }
        pstBufInfo->stFrameData.u32BufSize = pstFrameDataInfo->u32BufSize;

    }
    else if(E_MI_SYS_BUFDATA_META == pstBufConfig->eBufType)
    {
        //pstBufConfig->stMetaCfg.eBufAllocMode;//to be discuss :how to deal with eBufAllocMode?
        pstBufInfo->stMetaData.u32Size = pstBufConfig->stMetaCfg.u32Size;
        pstBufInfo->stMetaData.phyAddr = phyaddr;
    }
    else
    {
        MI_SYS_BUG();
    }
    return;

}

MI_SYS_BufRef_t *mi_sys_create_bufref(MI_SYS_BufferAllocation_t *pstBufAllocation,
                                      MI_SYS_BufConf_t *pstBufConfig, OnBufRefRelFunc onRelCB, void *pCBData)
{
    MI_SYS_BufRef_t *pstBufRef;

    MI_SYS_BUG_ON(!pstBufAllocation);
    MI_SYS_BUG_ON(!pstBufConfig);

    pstBufRef = kmem_cache_alloc(mi_sys_bufref_cachep, GFP_KERNEL);
    if(!pstBufRef)
    {
        DBG_ERR("Create BufRef Faild\n");
        return NULL;
    }

    memset(pstBufRef, 0x0, sizeof(MI_SYS_BufRef_t));
    INIT_LIST_HEAD(&pstBufRef->list);
    pstBufRef->onRelCB = onRelCB;
    pstBufRef->pCBData = pCBData;
    pstBufRef->pstBufAllocation = pstBufAllocation;
    pstBufRef->u32MagicNumber = __MI_SYS_BUF_REF_MAGIC_NUM__;
    pstBufRef->bPreNotified = FALSE;
    memcpy(&pstBufRef->bufinfo, &pstBufAllocation->stBufInfo, sizeof(pstBufRef->bufinfo));
    pstBufAllocation->ops.OnRef(pstBufAllocation);

    return pstBufRef;
}
void mi_sys_release_bufref(MI_SYS_BufRef_t *pstBufRef)
{
    MI_SYS_BUG_ON(!list_empty(&pstBufRef->list));
    MI_SYS_BUG_ON(pstBufRef->u32MagicNumber != __MI_SYS_BUF_REF_MAGIC_NUM__);

    if(pstBufRef->onRelCB)
        pstBufRef->onRelCB(pstBufRef, pstBufRef->pCBData);
    if(pstBufRef->pstBufAllocation)
    {
        pstBufRef->pstBufAllocation->ops.OnUnref(pstBufRef->pstBufAllocation);
    }
    //for debug purpose
    memset(pstBufRef, 0x21, sizeof(*pstBufRef));
    kmem_cache_free(mi_sys_bufref_cachep, pstBufRef);

    return;
}
MI_SYS_BufRef_t *mi_sys_dup_bufref(MI_SYS_BufRef_t *pstBufRef, OnBufRefRelFunc onRelCB, void *pCBData)
{
    MI_SYS_BufRef_t *pstDuppedBufRef = NULL;
    MI_SYS_BUG_ON(NULL == pstBufRef);
    MI_SYS_BUG_ON(NULL == pstBufRef->pstBufAllocation);

    // if "mi_sys_bufref_cachep" is null, the application maybe exception exit.
    BUG_ON(!mi_sys_bufref_cachep);

    pstDuppedBufRef = kmem_cache_alloc(mi_sys_bufref_cachep, GFP_KERNEL);
    if(!pstDuppedBufRef)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }
    memcpy(pstDuppedBufRef,  pstBufRef, sizeof(*pstBufRef));
    INIT_LIST_HEAD(&pstDuppedBufRef->list);

    pstDuppedBufRef->onRelCB = onRelCB;
    pstDuppedBufRef->pCBData = pCBData;
    pstDuppedBufRef->bPreNotified = FALSE;

    pstDuppedBufRef->pstBufAllocation->ops.OnRef(pstDuppedBufRef->pstBufAllocation);

    return pstDuppedBufRef;
}

void mi_sys_init_buf_queue(MI_SYS_BufferQueue_t*pstQueue, MI_ModuleId_e eModuleId, MI_SYS_QueueId_e eQueueId, unsigned char device_id, unsigned char channel_id, unsigned char port, unsigned char io)
{
    MI_SYS_BUG_ON(!pstQueue);
    INIT_LIST_HEAD(&pstQueue->list);
    pstQueue->queue_buf_count = 0;
    mutex_init(&pstQueue->mtx);
    pstQueue->module_id = eModuleId;
    pstQueue->queue_id = eQueueId;
    pstQueue->device_id = device_id;
    pstQueue->channel_id = channel_id;
    pstQueue->port = port;
    pstQueue->io = io;
    init_waitqueue_head(&pstQueue->dump_buffer_wq);
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    memset(&pstQueue->stEnqueueBufObjectTrackStaticInfo, 0, sizeof(pstQueue->stEnqueueBufObjectTrackStaticInfo));
    memset(&pstQueue->stDequeueBufObjectTrackStaticInfo, 0, sizeof(pstQueue->stDequeueBufObjectTrackStaticInfo));
#endif
}

void mi_sys_add_to_queue_head( MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue)
{
    MI_SYS_BUG_ON(!pstQueue|| !pstBufRef);
    //we don't allow queue operations in interrupt context
    MI_SYS_BUG_ON(in_interrupt());
    mutex_lock(&pstQueue->mtx);

    MI_SYS_BUG_ON(!list_empty(&pstBufRef->list));

    list_add(&pstBufRef->list, &pstQueue->list);
    pstBufRef->is_dumped = FALSE;
    pstQueue->dump_buffer_wq_cond = TRUE;
    pstQueue->queue_buf_count++;

    MI_SYS_BUG_ON(pstQueue->queue_buf_count<=0);
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    if(pstQueue->io)
    {
        //for output buf, accumulate module process time
        mi_sys_buf_mgr_accum_trackinfo(&pstQueue->stEnqueueBufObjectTrackStaticInfo, pstBufRef);
        //begin to track user fifo APP latency
        mi_sys_buf_mgr_update_trackinfo(pstBufRef);
    }
    else
    {
        switch(pstQueue->queue_id)
        {
            case MI_SYS_QUEUE_ID_BIND_INPUT:
            case MI_SYS_QUEUE_ID_USR_INJECT:
                //this is the rewind case, we only want to track the repending time
                //begin to track pending latency in this input queue
                mi_sys_buf_mgr_update_trackinfo(pstBufRef);
                break;
            default:
                break;
        }
    }
#endif
    mutex_unlock(&pstQueue->mtx);

    trace_mi_sys_bufqueue_add_head(pstBufRef, pstQueue);
}

void mi_sys_add_to_queue_tail( MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue)
{
    MI_SYS_BUG_ON(!pstQueue|| !pstBufRef);
    //we don't allow queue operations in interrupt context
    MI_SYS_BUG_ON(in_interrupt());
    mutex_lock(&pstQueue->mtx);

    MI_SYS_BUG_ON(!list_empty(&pstBufRef->list));
    list_add_tail(&pstBufRef->list, &pstQueue->list);
    pstBufRef->is_dumped = FALSE;
    pstQueue->dump_buffer_wq_cond = TRUE;
    pstQueue->queue_buf_count++;
    MI_SYS_BUG_ON(pstQueue->queue_buf_count<=0);
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    if(pstQueue->io)
    {
        switch(pstQueue->queue_id)
        {
            case MI_SYS_QUEUE_ID_USR_GET_FIFO:
                if(pstQueue->module_id == E_MI_MODULE_ID_VENC) //only venc has usr get case
                {
                    //static current Mi module processing latency
                    mi_sys_buf_mgr_accum_trackinfo(&pstQueue->stEnqueueBufObjectTrackStaticInfo, pstBufRef);
                    //begin to track pending latency in this user get fifo queue
                    mi_sys_buf_mgr_update_trackinfo(pstBufRef);
                }
                break;
            default:
                break;
        }
    }
    else
    {
        switch(pstQueue->queue_id)
        {
            case MI_SYS_QUEUE_ID_BIND_INPUT:
            case MI_SYS_QUEUE_ID_WORKING:
                //static previous Mi module processing latency
                mi_sys_buf_mgr_accum_trackinfo(&pstQueue->stEnqueueBufObjectTrackStaticInfo, pstBufRef);
                mi_sys_buf_mgr_update_trackinfo(pstBufRef); //aaron update for calc inputbind ~ outputbind time
                break;
            case MI_SYS_QUEUE_ID_USR_INJECT:
                //begin to track pending latency in this input queue
                mi_sys_buf_mgr_update_trackinfo(pstBufRef);
                break;
            default:
                break;
        }
    }
#endif
    mutex_unlock(&pstQueue->mtx);

    trace_mi_sys_bufqueue_add_tail(pstBufRef, pstQueue);
}

MI_SYS_BufRef_t* mi_sys_remove_from_queue_head(MI_SYS_BufferQueue_t*pstQueue)
{
    MI_SYS_BufRef_t *pstBufRef;

    MI_SYS_BUG_ON(!pstQueue);
    //we don't allow queue operations in interrupt context
    MI_SYS_BUG_ON(in_interrupt());
    mutex_lock(&pstQueue->mtx);

    if(list_empty(&pstQueue->list))
    {
        MI_SYS_BUG_ON(pstQueue->queue_buf_count != 0);
        pstBufRef = NULL;
    }
    else
    {
        pstBufRef = container_of(pstQueue->list.next, MI_SYS_BufRef_t, list);
        list_del(&pstBufRef->list);
        INIT_LIST_HEAD(&pstBufRef->list);
        MI_SYS_BUG_ON(pstQueue->queue_buf_count<=0);
        pstQueue->queue_buf_count--;
    }
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    if(pstQueue->io)
    {
        switch(pstQueue->queue_id)
        {
            case MI_SYS_QUEUE_ID_USR_GET_FIFO:
                if(pstQueue->module_id == E_MI_MODULE_ID_VENC) //only venc has usr get case
                {
                    //static the pending latency in user get fifo queue
                    mi_sys_buf_mgr_accum_trackinfo(&pstQueue->stDequeueBufObjectTrackStaticInfo, pstBufRef);
                }
                break;
            default:
                break;
        }
    }
    else
    {
        switch(pstQueue->queue_id)
        {
            case MI_SYS_QUEUE_ID_BIND_INPUT:
            case MI_SYS_QUEUE_ID_USR_INJECT:
                //static the pending time in this queue
                mi_sys_buf_mgr_accum_trackinfo(&pstQueue->stDequeueBufObjectTrackStaticInfo, pstBufRef);
                //reset last check point time
                mi_sys_buf_mgr_update_trackinfo(pstBufRef);
                break;
            default:
                break;
        }
    }
#endif
    mutex_unlock(&pstQueue->mtx);

    trace_mi_sys_bufqueue_rm_head(pstBufRef, pstQueue);
    return pstBufRef;
}

MI_SYS_BufRef_t* mi_sys_remove_from_queue_tail(MI_SYS_BufferQueue_t*pstQueue)
{
    MI_SYS_BufRef_t *pstBufRef;

    MI_SYS_BUG_ON(!pstQueue);
    //we don't allow queue operations in interrupt context
    MI_SYS_BUG_ON(in_interrupt());
    mutex_lock(&pstQueue->mtx);

    if(list_empty(&pstQueue->list))
    {
        MI_SYS_BUG_ON(pstQueue->queue_buf_count != 0);
        pstBufRef = NULL;
    }
    else
    {
        pstBufRef = container_of(pstQueue->list.prev, MI_SYS_BufRef_t, list);
        list_del(&pstBufRef->list);
        INIT_LIST_HEAD(&pstBufRef->list);
        MI_SYS_BUG_ON(pstQueue->queue_buf_count<=0);
        pstQueue->queue_buf_count--;
    }
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    if(pstQueue->io)
    {
        switch(pstQueue->queue_id)
        {
            case MI_SYS_QUEUE_ID_USR_GET_FIFO:
                //static the pending latency in user get fifo queue
                mi_sys_buf_mgr_accum_trackinfo(&pstQueue->stDequeueBufObjectTrackStaticInfo, pstBufRef);
                break;
            default:
                break;
        }
    }
    else
    {
        switch(pstQueue->queue_id)
        {
            case MI_SYS_QUEUE_ID_BIND_INPUT:
            case MI_SYS_QUEUE_ID_USR_INJECT:
                //static the pending time in this queue
                mi_sys_buf_mgr_accum_trackinfo(&pstQueue->stDequeueBufObjectTrackStaticInfo, pstBufRef);
                //reset last check point time
                mi_sys_buf_mgr_update_trackinfo(pstBufRef);
                break;
            default:
                break;
        }
    }
#endif
    mutex_unlock(&pstQueue->mtx);

    trace_mi_sys_bufqueue_rm_tail(pstBufRef, pstQueue);
    return pstBufRef;
}

void mi_sys_remove_from_queue(MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue)
{
    int found = 0;
    struct list_head *pos;
    MI_SYS_BUG_ON(!pstQueue);
    MI_SYS_BUG_ON(!pstBufRef);
    //we don't allow queue operations in interrupt context
    MI_SYS_BUG_ON(in_interrupt());
    mutex_lock(&pstQueue->mtx);

    list_for_each(pos, &(pstQueue->list))
    {
        if(pos == &pstBufRef->list)
        {
            found = 1;
            break;
        }
    }
    MI_SYS_BUG_ON(!found);
    MI_SYS_BUG_ON(pstQueue->queue_buf_count<=0);
    pstQueue->queue_buf_count --;
    list_del(&pstBufRef->list);
    //LIST_HEAD_INIT(&pstBufRef->list);
    INIT_LIST_HEAD(&pstBufRef->list);
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    if(pstQueue->io)
    {
        switch(pstQueue->queue_id)
        {
            case MI_SYS_QUEUE_ID_USR_GET_FIFO:
            case MI_SYS_QUEUE_ID_WORKING:
                //static the pending latency in user get fifo queue
                mi_sys_buf_mgr_accum_trackinfo(&pstQueue->stDequeueBufObjectTrackStaticInfo, pstBufRef);
                mi_sys_buf_mgr_update_trackinfo(pstBufRef);
                break;
            default:
                break;
        }
    }
    else
    {
        switch(pstQueue->queue_id)
        {
            case MI_SYS_QUEUE_ID_BIND_INPUT:
            case MI_SYS_QUEUE_ID_USR_INJECT:
                //static the pending time in this queue
                mi_sys_buf_mgr_accum_trackinfo(&pstQueue->stDequeueBufObjectTrackStaticInfo, pstBufRef);
                //reset last check point time
                mi_sys_buf_mgr_update_trackinfo(pstBufRef);
                break;
            default:
                break;
        }
    }
#endif
    mutex_unlock(&pstQueue->mtx);

    trace_mi_sys_bufqueue_rm_tail(pstBufRef, pstQueue);
}



#ifdef MI_SYS_PROC_FS_DEBUG

MI_U64 mi_sys_calc_buf_size_in_queue(MI_SYS_BufferQueue_t*pstQueue)
{
    struct list_head *pos;
    MI_SYS_BufRef_t *pstBufRef;
    MI_U64 total_buf_size_in_queue;
    MI_SYS_BUG_ON(!pstQueue);
    //we don't allow queue operations in interrupt context
    MI_SYS_BUG_ON(in_interrupt());
    mutex_lock(&pstQueue->mtx);
    total_buf_size_in_queue = 0;
    list_for_each(pos, &(pstQueue->list))
    {
        pstBufRef = container_of(pos, MI_SYS_BufRef_t, list);
        if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_RAW)
        {
            total_buf_size_in_queue += pstBufRef->bufinfo.stRawData.u32BufSize;
        }
        else if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_META)
        {
            total_buf_size_in_queue += pstBufRef->bufinfo.stMetaData.u32Size;
        }
        else if(pstBufRef->bufinfo.eBufType == E_MI_SYS_BUFDATA_FRAME)
        {
            total_buf_size_in_queue += pstBufRef->bufinfo.stFrameData.u32BufSize;
        }
        else
        {
            MI_SYS_BUG();
        }
    }
    MI_SYS_BUG_ON(pstQueue->queue_buf_count<0);
    mutex_unlock(&pstQueue->mtx);
    return total_buf_size_in_queue;
}
#endif

MI_S32 mi_sys_attach_allocator_to_collection(mi_sys_Allocator_t *pstAllocator, MI_SYS_Allocator_Collection_t *pstAllocatorCollection)
{
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    MI_SYS_AllocatorRef_t *tmp_pstAllocatorRef;
    struct list_head *pos;
    int count = 0;

    MI_SYS_BUG_ON(!pstAllocator);
    MI_SYS_BUG_ON(!pstAllocatorCollection);
    MI_SYS_BUG_ON(pstAllocatorCollection->u32MagicNumber != __MI_SYS_COLLECTION_MAGIC_NUM__);
    pstAllocatorRef = (MI_SYS_AllocatorRef_t*)kmalloc(sizeof(*pstAllocatorRef),GFP_KERNEL);
    if(!pstAllocatorRef)
    {
        DBG_ERR(" fail\n");
        return MI_ERR_SYS_NOMEM;
    }

    pstAllocatorRef->pstAllocator = pstAllocator;
    MI_SYS_BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->OnRef);
    pstAllocatorRef->pstAllocator->ops->OnRef(pstAllocatorRef->pstAllocator);
    down(&pstAllocatorCollection->semlock);
    list_for_each(pos, &pstAllocatorCollection->list)
    {
        tmp_pstAllocatorRef = container_of(pos, MI_SYS_AllocatorRef_t, list);
        MI_SYS_BUG_ON(tmp_pstAllocatorRef->pstAllocator==NULL ||
                      tmp_pstAllocatorRef->pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);
        count++;
        MI_SYS_BUG_ON(pstAllocatorCollection->collection_size < count);
        if(tmp_pstAllocatorRef->pstAllocator == pstAllocator)
        {
            up(&pstAllocatorCollection->semlock);
            MI_SYS_BUG_ON(!tmp_pstAllocatorRef->pstAllocator->ops || !tmp_pstAllocatorRef->pstAllocator->ops->OnUnref);
            tmp_pstAllocatorRef->pstAllocator->ops->OnUnref(tmp_pstAllocatorRef->pstAllocator);
            kfree(pstAllocatorRef);
            return MI_SUCCESS;
        }
    }
    MI_SYS_BUG_ON(count != pstAllocatorCollection->collection_size);
    list_add_tail(&pstAllocatorRef->list, &pstAllocatorCollection->list);
    pstAllocatorCollection->collection_size++;
    up(&pstAllocatorCollection->semlock);
    return MI_SUCCESS;
}

MI_S32 mi_sys_detach_allocator_from_collection(mi_sys_Allocator_t *pstAllocator, MI_SYS_Allocator_Collection_t *pstAllocatorCollection)
{
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    struct list_head *pos,*q;
    int count = 0;
    MI_SYS_INFO("%s:%d \n",__FUNCTION__,__LINE__);
    MI_SYS_BUG_ON(!pstAllocator);
    MI_SYS_BUG_ON(!pstAllocatorCollection);
    MI_SYS_BUG_ON(pstAllocatorCollection->u32MagicNumber != __MI_SYS_COLLECTION_MAGIC_NUM__);

    down(&pstAllocatorCollection->semlock);
    list_for_each_safe(pos, q,&pstAllocatorCollection->list)
    {
        pstAllocatorRef = container_of(pos, MI_SYS_AllocatorRef_t, list);
        MI_SYS_BUG_ON(pstAllocatorRef->pstAllocator==NULL ||
                      pstAllocatorRef->pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);
        count++;
        MI_SYS_BUG_ON(pstAllocatorCollection->collection_size < count);

        if(pstAllocatorRef->pstAllocator == pstAllocator)
        {
            list_del(&pstAllocatorRef->list);
            pstAllocatorCollection->collection_size--;
            MI_SYS_INFO("pstAllocatorCollection=%p    pstAllocatorCollection->collection_size=%d \n",pstAllocatorCollection,pstAllocatorCollection->collection_size);
            up(&pstAllocatorCollection->semlock);

            MI_SYS_BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->OnUnref);
            pstAllocatorRef->pstAllocator->ops->OnUnref(pstAllocatorRef->pstAllocator);
            kfree(pstAllocatorRef);
            MI_SYS_INFO("%s:%d \n",__FUNCTION__,__LINE__);
            return MI_SUCCESS;
        }
    }
    MI_SYS_BUG_ON(count != pstAllocatorCollection->collection_size);
    up(&pstAllocatorCollection->semlock);
    DBG_ERR("fail \n");
    return MI_ERR_SYS_UNEXIST;//later refine it to other errno
}

void mi_sys_init_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection)
{
    MI_SYS_BUG_ON(!pstAllocatorCollection);
    INIT_LIST_HEAD(&pstAllocatorCollection->list);
    pstAllocatorCollection->collection_size = 0;
    pstAllocatorCollection->u32MagicNumber = __MI_SYS_COLLECTION_MAGIC_NUM__;
    sema_init(&pstAllocatorCollection->semlock, 1);
}

MI_SYS_BufferAllocation_t * mi_sys_alloc_from_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection, MI_SYS_BufConf_t *pstBufConfig)
{
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    struct list_head *pos;
    int count = 0;
    int suit_best = INT_MIN;
    mi_sys_Allocator_t *pstAllocator_suit_best = NULL;
    int suit_cur;
    MI_SYS_BufferAllocation_t *allocation;

    MI_SYS_BUG_ON(!pstAllocatorCollection);
    MI_SYS_BUG_ON(pstAllocatorCollection->u32MagicNumber != __MI_SYS_COLLECTION_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufConfig);

    down(&pstAllocatorCollection->semlock);
    list_for_each(pos, &pstAllocatorCollection->list)
    {
        pstAllocatorRef = container_of(pos, MI_SYS_AllocatorRef_t, list);
        MI_SYS_BUG_ON(pstAllocatorRef->pstAllocator==NULL ||
                      pstAllocatorRef->pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);
        count++;
        MI_SYS_BUG_ON(pstAllocatorCollection->collection_size < count);
        MI_SYS_BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->suit_bufconfig);
        suit_cur = pstAllocatorRef->pstAllocator->ops->suit_bufconfig(pstAllocatorRef->pstAllocator, pstBufConfig /*, pstChnPort*/);
        if(suit_cur > suit_best)
        {
            if(pstAllocator_suit_best)
            {
                pstAllocator_suit_best->ops->OnUnref(pstAllocator_suit_best);
            }
            pstAllocator_suit_best = pstAllocatorRef->pstAllocator;
            pstAllocator_suit_best->ops->OnRef(pstAllocator_suit_best);
            suit_best = suit_cur;
        }
    }
    MI_SYS_BUG_ON(count != pstAllocatorCollection->collection_size);
    up(&pstAllocatorCollection->semlock);

    if(pstAllocator_suit_best)
    {
        allocation =  pstAllocator_suit_best->ops->alloc(pstAllocator_suit_best,  pstBufConfig);
        pstAllocator_suit_best->ops->OnUnref(pstAllocator_suit_best);
        return allocation;
    }

    //As function _MI_SYS_IMPL_AllocBufDefaultPolicy show,
    //goto here may not really error,so here not DBG_ERR.
    //DBG_ERR(" fail\n");
    return NULL;
}
void mi_sys_deinit_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection)
{
    MI_SYS_AllocatorRef_t *pstAllocatorRef;

    MI_SYS_BUG_ON(!pstAllocatorCollection);
    MI_SYS_BUG_ON(pstAllocatorCollection->u32MagicNumber != __MI_SYS_COLLECTION_MAGIC_NUM__);

    down(&pstAllocatorCollection->semlock);
    while(!list_empty(&pstAllocatorCollection->list))
    {
        MI_SYS_BUG_ON( pstAllocatorCollection->collection_size <= 0);
        pstAllocatorRef = container_of(pstAllocatorCollection->list.next, MI_SYS_AllocatorRef_t, list);
        MI_SYS_BUG_ON(pstAllocatorRef->pstAllocator==NULL ||
                      pstAllocatorRef->pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);

        list_del(&pstAllocatorRef->list);
        pstAllocatorCollection->collection_size--;

        MI_SYS_BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->OnUnref);
        pstAllocatorRef->pstAllocator->ops->OnUnref(pstAllocatorRef->pstAllocator);
        kfree(pstAllocatorRef);
    }
    MI_SYS_BUG_ON( pstAllocatorCollection->collection_size);
    up(&pstAllocatorCollection->semlock);
}

/*
this function will be used by user,not be used by other internal VB POOL functions.
*/
static unsigned long _mi_calc_sys_frame_size(MI_SYS_BufFrameConfig_t stFrameConfig
        ,MI_SYS_FrameSpecial_t  *pstFrameDataInfo)
{
    MI_U16 u16Width = stFrameConfig.u16Width;
    MI_U16 u16Height = stFrameConfig.u16Height;
    unsigned long size;

    MI_SYS_BUG_ON(!u16Width || !u16Height);
    memset(pstFrameDataInfo, 0, sizeof(*pstFrameDataInfo));

    //HAlignment:alighment in horizontal
    //VAlignment:alighment in vertical
    switch(stFrameConfig.eFormat)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            u16Width = ALIGN_UP(u16Width, 2);
            size = u16Width *  2;
            //size = ALIGN_UP(size, alignment);
            size = ALIGN_UP(size, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            size *=u16Height;
            break;
        case E_MI_SYS_PIXEL_FRAME_ARGB8888:
        case E_MI_SYS_PIXEL_FRAME_ABGR8888:
            size = u16Width *  4;
            //size = ALIGN_UP(size, alignment);
            size = ALIGN_UP(size, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            size *=u16Height;
            break;
        case E_MI_SYS_PIXEL_FRAME_RGB565:
        case E_MI_SYS_PIXEL_FRAME_ARGB1555:
            size = u16Width *  2;
            //size = ALIGN_UP(size, alignment);
            size = ALIGN_UP(size, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            size *=u16Height;
            break;
        case E_MI_SYS_PIXEL_FRAME_I2:
            size = (u16Width+3)/4;
            //size = ALIGN_UP(size, alignment);
            size = ALIGN_UP(size, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            size *=u16Height;
            break;
        case E_MI_SYS_PIXEL_FRAME_I4:
            size = (u16Width+1)/2;
            //size = ALIGN_UP(size, alignment);
            size = ALIGN_UP(size, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            size *=u16Height;
            break;
        case E_MI_SYS_PIXEL_FRAME_I8:
            //size = ALIGN_UP(u16Width, alignment);
            size = ALIGN_UP(u16Width, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            size *=u16Height;
            break;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422://:need two address
            //size = ALIGN_UP(u16Width, alignment);
            size = ALIGN_UP(u16Width, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            pstFrameDataInfo->u32Stride[1] = size;
            pstFrameDataInfo->phy_offset[1] = size*u16Height;
            size =size*u16Height*2;
            break;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420://:need two address
            u16Height = ALIGN_UP(u16Height, 2);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            //size = ALIGN_UP(u16Width, alignment);
            size = ALIGN_UP(u16Width, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            pstFrameDataInfo->u32Stride[1] = size;
            pstFrameDataInfo->phy_offset[1] = size*u16Height;
            size =size*u16Height*3/2;
            break;

        case  E_MI_SYS_PIXEL_FRAME_YUV_MST_420:
            //size = ALIGN_UP(u16Width*3/2, alignment);
            size = ALIGN_UP(u16Width*3/2, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            size =size*u16Height;
            break;


        //vdec mstar private video format
        case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE1_H264:
            u16Height = ALIGN_UP(u16Height, 16);//charles.wu said them self will alignment of H264   .
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            u16Width = ALIGN_UP(u16Width, 16);
            //size = ALIGN_UP(u16Width, alignment);
            size = ALIGN_UP(u16Width, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            //pstFrameDataInfo->u16Width = u16Height;
            pstFrameDataInfo->u32Stride[0] = size;
            pstFrameDataInfo->u32Stride[1] = size;
            pstFrameDataInfo->phy_offset[1] = size*u16Height;
            size =size*u16Height*3/2;//YC420_MSTTILE1_H264:  (8+4)/8=1.5,for code we use 3/2
            break;
        case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE2_H265://from ken.huang:H265 need 32 ALIGN_UP  u16Width & u16Height
            u16Width = ALIGN_UP(u16Width, 32);
            u16Height = ALIGN_UP(u16Height, 32);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            //size = ALIGN_UP(u16Width, alignment);
            size = ALIGN_UP(u16Width, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            pstFrameDataInfo->u32Stride[1] = size;
            pstFrameDataInfo->phy_offset[1] = size*u16Height;
            size =size*u16Height*3/2;//YC420_MSTTILE2_H265:  (8+4)/8=1.5,for code we use 3/2
            break;
        case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE3_H265:
            u16Width = ALIGN_UP(u16Width, 32);//from ken.huang:H265 need 32 ALIGN_UP  u16Width & u16Height
            u16Height = ALIGN_UP(u16Height, 32);
            u16Height = ALIGN_UP(u16Height, stFrameConfig.stFrameBufExtraConf.u16BufVAlignment);
            //size = ALIGN_UP(u16Width, alignment);
            size = ALIGN_UP(u16Width, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            pstFrameDataInfo->u32Stride[1] = size;
            pstFrameDataInfo->phy_offset[1] = size*u16Height;
            size =size*u16Height*3/2;
            break;

        //from andy.hsieh:for 8PP ,nowadays use width*height*2,may future change calculation method.
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_8BPP_RG:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_8BPP_GR:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_8BPP_GB:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_8BPP_BG:
            u16Width = ALIGN_UP(u16Width, 32);
            u16Height = ALIGN_UP(u16Height, 2);
            //size = ALIGN_UP(u16Width, alignment);
            size = ALIGN_UP(u16Width, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            size= size*u16Height*2;
            break;

        //from andy.hsieh:for 10/12/14/16 BPP,use width*height*2, stride=width*2.
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_GR:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_GB:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_BG:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_12BPP_RG:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_12BPP_GR:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_12BPP_GB:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_12BPP_BG:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_14BPP_RG:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_14BPP_GR:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_14BPP_GB:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_14BPP_BG:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_16BPP_RG:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_16BPP_GR:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_16BPP_GB:
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_16BPP_BG:
            u16Width = ALIGN_UP(u16Width, 32);
            u16Height = ALIGN_UP(u16Height, 2);
            //size = ALIGN_UP(u16Width, alignment);
            size = ALIGN_UP(u16Width * 2, stFrameConfig.stFrameBufExtraConf.u16BufHAlignment);
            pstFrameDataInfo->u32Stride[0] = size;
            size= size*u16Height;
            break;

        default:
            MI_SYS_BUG();
    }

    MI_SYS_BUG_ON(!size);
    pstFrameDataInfo->u32BufSize = size;
    return size;
}

unsigned long mi_sys_buf_mgr_get_size(MI_SYS_BufConf_t *pstBufConfig, MI_SYS_FrameSpecial_t  *pstFrameDataInfo)
{
    unsigned long size_to_alloc;

    MI_SYS_BUG_ON(!pstBufConfig);
    if(pstBufConfig->eBufType == E_MI_SYS_BUFDATA_RAW)
    {
        size_to_alloc = pstBufConfig->stRawCfg.u32Size;
    }
    else if(pstBufConfig->eBufType == E_MI_SYS_BUFDATA_FRAME)
    {
        size_to_alloc = _mi_calc_sys_frame_size(pstBufConfig->stFrameCfg, pstFrameDataInfo);
    }
    else if(pstBufConfig->eBufType == E_MI_SYS_BUFDATA_META)
    {
        size_to_alloc = pstBufConfig->stMetaCfg.u32Size;
    }
    else
    {
        MI_SYS_BUG();
    }

    MI_SYS_BUG_ON(!size_to_alloc);

    return size_to_alloc;
}

MI_SYS_USER_MAP_CTX mi_sys_buf_get_cur_user_map_ctx(void)
{
    return (MI_SYS_USER_MAP_CTX)current->mm;

}

typedef struct MI_SYS_MMAP_PARAM_s
{
    void *key;
    struct sg_table *sg_table;
    unsigned long content_map_offset;
    unsigned long content_map_len;
} MI_SYS_MMAP_PARAM_t;

#define MI_SYS_USER_MAP_HASH_BITS 8
static DEFINE_HASHTABLE(g_mi_sys_user_map_addr_hash,MI_SYS_USER_MAP_HASH_BITS);
static DEFINE_SEMAPHORE(g_mi_sys_user_map_hash_semphore);
#define MI_SYS_USR_MAP_REC_MAGIC 0x4D524543
typedef struct mi_sys_user_map_record_s
{
    struct hlist_node   hentry;
    MI_U32 u32MagicNumber;
    void *key;
    struct file *file;

    MI_BOOL bBoundChk;
    void *mapped_addr;//with boundary guard
    unsigned long mapped_length;//with boundary guard
    void *user_addr;
    unsigned long user_addr_len;
    MI_SYS_USER_MAP_CTX usr_process_ctx;

} mi_sys_user_map_record_t;
int mi_sys_user_mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{

    mi_sys_user_map_record_t *pUsrMapRecord;

    MI_SYS_BUG_ON(!vma);

    pUsrMapRecord =(mi_sys_user_map_record_t *)vma->vm_private_data;
    MI_SYS_BUG_ON(!pUsrMapRecord);
    MI_SYS_BUG_ON(pUsrMapRecord->u32MagicNumber != MI_SYS_USR_MAP_REC_MAGIC);

    printk(KERN_ERR "mi_sys_user_mmap_fault (at %p), rec\n", vmf->virtual_address);
    printk(KERN_ERR " valid range[%p~%p]\n",pUsrMapRecord->user_addr, (char*)pUsrMapRecord->user_addr+pUsrMapRecord->user_addr_len);
    printk(KERN_ERR " boundary range[%p~%p]\n", pUsrMapRecord->mapped_addr, (char*)pUsrMapRecord->mapped_addr+pUsrMapRecord->mapped_length);

    return VM_FAULT_SIGBUS;
}

static atomic_t g_mapped_cnt = ATOMIC_INIT(0);
static void mi_sys_user_mmap_close(struct vm_area_struct *vma)
{
    int val;
    mi_sys_user_map_record_t *pUsrMapRecord = vma->vm_private_data;

    MI_SYS_BUG_ON(!pUsrMapRecord);
    MI_SYS_BUG_ON(pUsrMapRecord->u32MagicNumber != MI_SYS_USR_MAP_REC_MAGIC);

    down(&g_mi_sys_user_map_hash_semphore);
    hlist_del_init(&pUsrMapRecord->hentry);
    up(&g_mi_sys_user_map_hash_semphore);

    MI_SYS_BUG_ON(vma->vm_start != (unsigned long)pUsrMapRecord->mapped_addr);
    MI_SYS_BUG_ON(vma->vm_end- vma->vm_start != pUsrMapRecord->mapped_length);
    memset(pUsrMapRecord, 0x24, sizeof(*pUsrMapRecord));
    kfree(pUsrMapRecord);

    val = atomic_dec_return(&g_mapped_cnt);
    MI_SYS_BUG_ON(val <0);
    // printk("vmap value return to %d\n", val);
}

static const struct vm_operations_struct mi_sys_usrmap_vm_ops =
{
    .fault      = mi_sys_user_mmap_fault,
    .close = mi_sys_user_mmap_close,
};

int mi_sys_userdev_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct scatterlist *sg;
    int i;
    int ret;
    MI_SYS_MMAP_PARAM_t *pstMmapParam;
    struct sg_table *sg_table;
    unsigned long addr;
    mi_sys_user_map_record_t *pUsrMapRecord;
    unsigned long mapped_len = 0;

    MI_SYS_BUG_ON(!file);
    MI_SYS_BUG_ON(!vma);
    pstMmapParam = (MI_SYS_MMAP_PARAM_t *)file->private_data;
    MI_SYS_BUG_ON(!pstMmapParam);

    sg_table = pstMmapParam->sg_table;
    MI_SYS_BUG_ON(!sg_table);

    down(&g_mi_sys_user_map_hash_semphore);

    hash_for_each_possible(g_mi_sys_user_map_addr_hash, pUsrMapRecord, hentry, (unsigned long)pstMmapParam->key)
    {

        if(pUsrMapRecord->key == pstMmapParam->key && pUsrMapRecord->usr_process_ctx == mi_sys_buf_get_cur_user_map_ctx())
        {
            MI_SYS_BUG();
        }
    }

    pUsrMapRecord = kmalloc(sizeof(*pUsrMapRecord),GFP_KERNEL);
    if(!pUsrMapRecord)
    {
        up(&g_mi_sys_user_map_hash_semphore);
        return -ENOSPC;
    }

    pUsrMapRecord->u32MagicNumber = MI_SYS_USR_MAP_REC_MAGIC;

    MI_SYS_BUG_ON(pstMmapParam->content_map_offset & ~PAGE_MASK);
    MI_SYS_BUG_ON(pstMmapParam->content_map_len==0 || (pstMmapParam->content_map_len & ~PAGE_MASK));
    MI_SYS_BUG_ON(vma->vm_pgoff != 0);
    MI_SYS_BUG_ON(vma->vm_start+pstMmapParam->content_map_offset >= vma->vm_end);
    MI_SYS_BUG_ON(vma->vm_start+pstMmapParam->content_map_offset+pstMmapParam->content_map_len > vma->vm_end);

    addr = vma->vm_start+pstMmapParam->content_map_offset;

    for_each_sg(sg_table->sgl, sg, sg_table->nents, i)
    {
        struct page *page = sg_page(sg);
        unsigned long remainder = vma->vm_end - addr;
        unsigned long len = sg->length;

        len = min(len, remainder);
        ret = remap_pfn_range(vma, addr, page_to_pfn(page), len,
                              vma->vm_page_prot);
        if (ret)
        {
            DBG_ERR("remap_pfn_range fail\n");
            memset(pUsrMapRecord, 0x22, sizeof(*pUsrMapRecord));
            kfree(pUsrMapRecord);
            return -ENOSPC;//for kernel do_mmap_pgoff  use,do not and no need change this errno to MI_ERR_SYS_xxx
        }
        addr += len;
        mapped_len += len;
        if (addr >= vma->vm_end)
        {
            break;
        }
    }

    MI_SYS_BUG_ON(mapped_len != pstMmapParam->content_map_len);
    //  printk("mi_sys_userdev_mmap %d\n", pstMmapParam->content_map_len);

    vma->vm_ops = &mi_sys_usrmap_vm_ops;
    vma->vm_flags = (vma->vm_flags | VM_DONTCOPY);
    vma->vm_private_data = (void*)pUsrMapRecord;

    pUsrMapRecord->mapped_addr = (void*)vma->vm_start;
    pUsrMapRecord->mapped_length = vma->vm_end-vma->vm_start;

    pUsrMapRecord->user_addr = (void*)(vma->vm_start+pstMmapParam->content_map_offset);
    pUsrMapRecord->user_addr_len = pstMmapParam->content_map_len;
    pUsrMapRecord->usr_process_ctx = mi_sys_buf_get_cur_user_map_ctx();
    pUsrMapRecord->key = pstMmapParam->key;
    atomic_inc(&g_mapped_cnt);

    hash_add(g_mi_sys_user_map_addr_hash, &pUsrMapRecord->hentry, (unsigned long)pUsrMapRecord->key);

    up(&g_mi_sys_user_map_hash_semphore);

    return 0;
}

static inline int _mi_sys_try_get_usr_map_addr(void *key, void **pp_user_map_ptr,  unsigned long  *pLen)
{
    mi_sys_user_map_record_t *pUsrMapRecord;

    down(&g_mi_sys_user_map_hash_semphore);

    hash_for_each_possible(g_mi_sys_user_map_addr_hash, pUsrMapRecord, hentry, (unsigned long)key)
    {
        MI_SYS_BUG_ON(pUsrMapRecord->u32MagicNumber != MI_SYS_USR_MAP_REC_MAGIC);
        if(pUsrMapRecord->key == key && pUsrMapRecord->usr_process_ctx == mi_sys_buf_get_cur_user_map_ctx())
        {
            *pp_user_map_ptr = pUsrMapRecord->user_addr;
            *pLen = pUsrMapRecord->user_addr_len;
            up(&g_mi_sys_user_map_hash_semphore);
            return MI_SUCCESS;
        }
    }
    *pp_user_map_ptr = NULL;
    up(&g_mi_sys_user_map_hash_semphore);

    return MI_ERR_SYS_UNEXIST;
}
static inline int _mi_sys_try_unmap_usr_map(void *key)
{
    mi_sys_user_map_record_t *pUsrMapRecord;
    int ret;

    down(&g_mi_sys_user_map_hash_semphore);
    hash_for_each_possible(g_mi_sys_user_map_addr_hash, pUsrMapRecord, hentry, (unsigned long)key)
    {
        MI_SYS_BUG_ON(pUsrMapRecord->u32MagicNumber != MI_SYS_USR_MAP_REC_MAGIC);
        if(pUsrMapRecord->key == key && pUsrMapRecord->usr_process_ctx == mi_sys_buf_get_cur_user_map_ctx())
        {
            up(&g_mi_sys_user_map_hash_semphore);

            //move to _MI_SYS_IMPL_UnMmapUserVirAddr, no need do here.
            //mi_sys_buf_mgr_flush_inv_cache(pUsrMapRecord->mapped_addr, pUsrMapRecord->mapped_length);

            down_write(&current->mm->mmap_sem);
            ret = do_munmap(mi_sys_buf_get_cur_user_map_ctx(), (unsigned long)pUsrMapRecord->mapped_addr, pUsrMapRecord->mapped_length);
            up_write(&current->mm->mmap_sem);
            MI_SYS_BUG_ON(ret);
            return MI_SUCCESS;
        }
    }
    up(&g_mi_sys_user_map_hash_semphore);
    return MI_ERR_SYS_UNEXIST;
}

static int mi_sys_userdev_release_file(struct inode *inode, struct file *filp)
{
    // TODO
//   printk("%s\n", __FUNCTION__);

    MI_SYS_MMAP_PARAM_t *pstMmapParam;
    struct sg_table *sg_table;


    pstMmapParam = (MI_SYS_MMAP_PARAM_t *)filp->private_data;
    MI_SYS_BUG_ON(!pstMmapParam);

    sg_table = pstMmapParam->sg_table;
    MI_SYS_BUG_ON(!sg_table);

    sg_free_table(sg_table);//change it into file_operations .release
    kfree(sg_table);//change it into file_operations .release

    kfree(pstMmapParam);//final free filp->private_data
    return 0;
}

/*static*/ struct file_operations mma_userdev_fops =
{
    .owner        = THIS_MODULE,
    .mmap        =  mi_sys_userdev_mmap,
    .release    = mi_sys_userdev_release_file,
};

extern struct MMA_BootArgs_Config mma_config[MAX_MMA_AREAS];
extern int mstar_driver_boot_mma_buffer_num ;

static inline int _is_in_mi_sys_mma_heap_range(unsigned long beg_pfn, unsigned long count)
{
    int i=0;
    MI_PHY beg_cpu_bus_addr;
    MI_PHY end_cpu_bus_addr;

    MI_SYS_BUG_ON(count ==0 || beg_pfn+count<=beg_pfn);
    MI_SYS_BUG_ON(mstar_driver_boot_mma_buffer_num > MAX_MMA_AREAS);

    beg_cpu_bus_addr = __pfn_to_phys(beg_pfn);
    end_cpu_bus_addr = __pfn_to_phys(beg_pfn+count);

    for(i=0; (i<mstar_driver_boot_mma_buffer_num) ; i++)
    {
        if(mma_config[i].size !=0)//inside for
        {
            if((beg_cpu_bus_addr>=mma_config[i].reserved_start &&
                    end_cpu_bus_addr<= mma_config[i].reserved_start + mma_config[i].size))
                return 1;//find pa in reserved area
        }
    }

    return 0;
}

int mi_sys_check_page_range_mapable(unsigned long pfn, long count)
{
    MI_SYS_BUG_ON(count <= 0);
    if(_is_in_mi_sys_mma_heap_range(pfn, count))
    {
        return 1;
    }
    while(count)
    {
        if(pfn_valid(pfn) && !_is_in_mi_sys_mma_heap_range(pfn, 1))
        {
            return 0;
        }
        count--;
        pfn++;
    }
    return 1;//mmap not in kernel,is also ok.
}

static int mi_sys_map_sg_table(struct sg_table *sg_table,void **pp_virt_addr, int bForceSkipPermitCheck, void *key)
{
    unsigned long populate;
    struct scatterlist *sg;
    int i=0;
    struct file *file;
    int total_len=0;
    MI_SYS_MMAP_PARAM_t *stMapParam;

    stMapParam = kmalloc(sizeof(MI_SYS_MMAP_PARAM_t), GFP_KERNEL);
    if (!stMapParam)
    {
        *pp_virt_addr = NULL;
        DBG_ERR("kmalloc fail\n");
        return MI_ERR_SYS_FAILED;
    }


    MI_SYS_BUG_ON(!sg_table);

    for_each_sg(sg_table->sgl, sg, sg_table->nents, i)
    {
        total_len += sg->length;
        if(!bForceSkipPermitCheck &&
                !mi_sys_check_page_range_mapable(page_to_pfn(sg_page(sg)), sg->length/PAGE_SIZE))
        {
            DBG_ERR("mi_sys_check_page_range_mapable failed\n");
            MI_SYS_BUG();
            return MI_ERR_SYS_NOT_PERM;
        }

    }

    MI_SYS_BUG_ON(total_len == 0);
    stMapParam->key = key;
    stMapParam->sg_table = sg_table;
    stMapParam->content_map_offset = 0;
    stMapParam->content_map_len = total_len;

    file = anon_inode_getfile("mi_sys_mma_usr_map_file", &mma_userdev_fops, stMapParam, O_RDWR);

    if (IS_ERR(file))
    {
        *pp_virt_addr = NULL;
        DBG_ERR("anon_inode_getfile fail\n");
        return MI_ERR_SYS_FAILED;
    }

    down_write(&current->mm->mmap_sem);

#ifdef MI_SYS_BOUNDARY_CHECK
    *pp_virt_addr = (void *)do_mmap_pgoff(file, 0, total_len+PAGE_SIZE*2, PROT_READ|PROT_WRITE, MAP_SHARED, 0, &populate);
#else
    *pp_virt_addr = (void *)do_mmap_pgoff(file, 0, total_len, PROT_READ|PROT_WRITE, MAP_SHARED, 0, &populate);
#endif
    //file->private_data->sg_table = NULL;

    up_write(&current->mm->mmap_sem);

    fput(file);

    if(IS_ERR_VALUE((unsigned long)(*pp_virt_addr)))
    {
        *pp_virt_addr = NULL;
        return MI_ERR_SYS_BUSY;
    }

    return MI_SUCCESS;
}

//MI_SYS_USER_MAP_CTX self is a pointer
int mi_sys_buf_mgr_user_map(struct sg_table *sg_table,void **pp_user_map_ptr, void *key, int bForceSkipPermitCheck)
{
    int ret;
    unsigned long total_len = 0;
    struct scatterlist *sg;
    int i;

    MI_SYS_BUG_ON(!sg_table);

    ret = _mi_sys_try_get_usr_map_addr(key, pp_user_map_ptr, &total_len);
    if(ret != MI_SUCCESS)
    {
        ret = mi_sys_map_sg_table( sg_table,  pp_user_map_ptr, bForceSkipPermitCheck, key);
        if(ret != MI_SUCCESS)
            return ret;
        ret = _mi_sys_try_get_usr_map_addr(key, pp_user_map_ptr, &total_len);
        // printk("mi_sys_map_sg_table totallen2.0 %d, %d\n", total_len, ret);
        MI_SYS_BUG_ON(ret != MI_SUCCESS);
        //printk(KERN_ERR "mi_sys_buf_mgr_user_map2 %d\n", total_len);
    }
    // else
    //  printk(KERN_ERR "mi_sys_buf_mgr_user_map1 %d\n", total_len);

    for_each_sg(sg_table->sgl, sg, sg_table->nents, i)
    {
        total_len -= sg->length;
    }
    //     printk(KERN_ERR "mi_sys_buf_mgr_user_map3 %d\n", total_len);

    MI_SYS_BUG_ON(total_len);
    return MI_SUCCESS;
}

int mi_sys_buf_mgr_user_unmap(void *key)
{
    int ret;
    unsigned long total_len;
    void *paddr;

    ret = _mi_sys_try_unmap_usr_map(key);
    if(ret == MI_SUCCESS)
    {

        MI_SYS_BUG_ON(MI_SUCCESS == _mi_sys_try_get_usr_map_addr(key, &paddr, &total_len));
    }

    return MI_SUCCESS;
}

int mi_sys_buf_mgr_vmap_kern(struct sg_table *sg_table,void **pp_kern_map_ptr)
{
    struct scatterlist *sg;
    int page_count=0;
    struct page **pages;
    int i, j,k;

    MI_SYS_BUG_ON(!sg_table);
    MI_SYS_BUG_ON(!pp_kern_map_ptr);
    *pp_kern_map_ptr = NULL;
    for_each_sg(sg_table->sgl, sg, sg_table->nents, i)
    {
        page_count += sg->length/PAGE_SIZE;
        //only support mapping MMA range
        if(!mi_sys_check_page_range_mapable(page_to_pfn(sg_page(sg)), sg->length/PAGE_SIZE))
        {
            DBG_ERR("mi_sys_check_page_range_mapable failed \n");
            MI_SYS_BUG();
            return MI_ERR_SYS_FAILED;
        }

    }
    MI_SYS_BUG_ON(!page_count);
    pages = vmalloc(sizeof(struct page*)*page_count);
    if(!pages)
    {
        DBG_ERR("vmalloc fail\n");
        return MI_ERR_SYS_NOBUF;
    }

    i=0;
    for_each_sg(sg_table->sgl, sg, sg_table->nents, k)
    {
        page_count = PAGE_ALIGN(sg->length) / PAGE_SIZE;
        for( j=0; j<page_count; j++)
            pages[i+j] =  sg_page(sg)+j;
        i+=page_count;
    }
    *pp_kern_map_ptr = vmap(pages, i, VM_MAP, PAGE_KERNEL);
    vfree(pages);
    if (*pp_kern_map_ptr == NULL)
    {
        DBG_ERR("vmap fail\n");
        return MI_ERR_SYS_FAILED;
    }

    return MI_SUCCESS;
}

int mi_sys_buf_mgr_vunmap_kern(void *va_in_kern)
{
    MI_SYS_BUG_ON(!va_in_kern);
    vunmap(va_in_kern);
    return MI_SUCCESS;
}

#ifdef CONFIG_OUTER_CACHE
static int flushcache_walk_pte_entry(pte_t *ptent, unsigned long addr,
                                     unsigned long next, struct mm_walk *walk)
{
    unsigned long pa;
    walk = walk;
    pa = (pte_val(*ptent)&PAGE_MASK) | (addr &~PAGE_MASK);
    outer_flush_range(pa, pa + (next - addr));
    return 0;
}
#endif

extern struct file_operations msys_api_mmap_fops;
extern struct file_operations mma_userdev_fops;
MI_S32 mi_sys_buf_mgr_flush_inv_cache(void *va, unsigned long len)
{
#ifdef CONFIG_OUTER_CACHE
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    unsigned long pa_start;
    unsigned long offset;
    struct mm_walk pagemap_walk =
    {
        .mm = current->active_mm,
        .pte_entry = flushcache_walk_pte_entry,
    };
#endif
#ifndef CONFIG_OUTER_CACHE
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
    extern void Chip_Flush_Miu_Pipe(void);
#endif
#endif

    if(len == 0)
    {
        DBG_WRN("len is 0,may give wrong parameter!");
        return MI_ERR_SYS_FAILED;
    }

    if((long unsigned)va >= PAGE_OFFSET)
    {
        //kernel space VA flush
        __cpuc_clean_dcache_area(va, len);
        //step 2-a:
        //for chip that def CONFIG_OUTER_CACHE,flush both L2 cache and L3 cache(miu pipe).
#ifdef CONFIG_OUTER_CACHE
        //for walk_page_range function,must page align down for first parameter,
        //and page align up for second parameter.
        walk_page_range(((unsigned long)va) & PAGE_MASK,PAGE_ALIGN((unsigned long)va+len),&pagemap_walk);
#endif
        return MI_SUCCESS;
    }
    else//for user space VA
    {
        //step1:flush L1 cache
        //if def CONFIG_OUTER_CACHE, means flush L1 cache;
        //if not def CONFIG_OUTER_CACHE,L1 and L2 both inside CPU, infact flush L1&L2 flush.
        dmac_flush_range(va, (void *)((unsigned long)va+len));

#ifdef CONFIG_OUTER_CACHE
        mm = current->active_mm;
        down_read(&mm->mmap_sem);
        vma = find_vma(mm, (unsigned long )va);

        if(!vma  || !vma->vm_file)
        {
            if(!vma)
                MI_SYS_WARN("Invalid VA for cache flush found!!![%p]\n", va);
            else
                MI_SYS_WARN("MI SYS doesn't support anonymous VA cache flush from user mode!!!\n");
            up_read(&mm->mmap_sem);
            return MI_ERR_SYS_FAILED;
        }

        if(vma->vm_file->f_op == &msys_api_mmap_fops)
        {
            /*case for
            first: MI_SYS_Mmap(xx,xx,xx,TRUE)
            and then: MI_SYS_FlushInvCache(xx,xx)
            and finally:  MI_SYS_Munmap(xx,xx)
            */
            mi_sys_mmap_data_t *pst_mmap_data;
            pst_mmap_data = vma->vm_file->private_data;

            offset = (unsigned long )va - vma->vm_start;
            len = min(len, pst_mmap_data->pages*PAGE_SIZE-offset);

            pa_start = __pfn_to_phys(pst_mmap_data->pfn)+offset;

            //step 2-a:
            //for chip that CONFIG_OUTER_CACHE,flush both L2 cache and L3 cache(miu pipe).
            outer_flush_range((phys_addr_t)pa_start, (phys_addr_t)(pa_start + len));
        }
        else if(vma->vm_file->f_op == &mma_userdev_fops)
        {
            /*case for
            first: MI_SYS_ChnInputPortGetBuf/MI_SYS_ChnOutputPortGetBuf
            and then: MI_SYS_FlushInvCache(xx,xx)
            and finally:  MI_SYS_ChnInputPortPutBuf/MI_SYS_ChnOutputPortPutBuf
            */

            MI_SYS_MMAP_PARAM_t *pstMmapParam;
            struct sg_table *sg_table;
            struct scatterlist *sg;
            int i=0;
            long flush_length;
            pstMmapParam = (MI_SYS_MMAP_PARAM_t *)vma->vm_file->private_data;
            MI_SYS_BUG_ON(!pstMmapParam);
            sg_table = pstMmapParam->sg_table;
            MI_SYS_BUG_ON(!sg_table);

            offset = (unsigned long )va - vma->vm_start;
            for_each_sg(sg_table->sgl, sg, sg_table->nents, i)
            {
                if(offset >= sg->length)
                {
                    offset -= sg->length;
                    continue;
                }
                if(len+offset > sg->length)
                    flush_length = sg->length-offset;
                else
                    flush_length = len;

                pa_start = page_to_phys(sg_page(sg))+offset;

                //step 2-a:
                //for chip that def CONFIG_OUTER_CACHE,flush both L2 cache and L3 cache(miu pipe).
                outer_flush_range(pa_start, pa_start+flush_length);

                offset = 0;
                len -= flush_length;

                if(!len)
                    break;
            }

        }
        else
        {
            MI_SYS_WARN("MI SYS doesn't support arbitrary VA cache flush from user mode!!![%p]\n", va);
            MI_SYS_WARN("current not support this vma->vm_file->f_op=%p,it may be vdisp_allocation_fops=%p or others???\n",vma->vm_file->f_op,&vdisp_allocation_fops);
        }
        up_read(&mm->mmap_sem);
#endif
    }

//step2-b:
//for chip that not def CONFIG_OUTER_CACHE,flush L3,means flush miu pipe.
#ifndef CONFIG_OUTER_CACHE
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
    Chip_Flush_Miu_Pipe();
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
#ifdef CONFIG_ARCH_INFINITY2
    Chip_Flush_MIU_Pipe();
#else
    //not know whether this chip use Chip_Flush_Miu_Pipe or Chip_Flush_MIU_Pipe yet,just give a #error
#error please confirm your chip flush pipe name
#endif

#else
#error not support this kernel version
#endif
#endif

    return MI_SUCCESS;
}

MI_S32 mi_sys_buf_map_kern_va(MI_SYS_BufferAllocation_t*pstBufAllocation, MI_SYS_BufInfo_t *pstBufInfo, MI_BOOL bFlushCache)
{
    MI_SYS_BUG_ON(!pstBufAllocation);
    MI_SYS_BUG_ON(!pstBufInfo);
    MI_SYS_INFO("   %s:%d  \n",__FUNCTION__,__LINE__);
    switch(pstBufInfo->eBufType)
    {
        case  E_MI_SYS_BUFDATA_RAW:
            MI_SYS_INFO("   %s:%d  \n",__FUNCTION__,__LINE__);
            if(pstBufInfo->stRawData.pVirAddr)
            {
                //va must be in kernel mapping space
                MI_SYS_INFO("   %s:%d   pstBufInfo->stRawData.pVirAddr=%p\n",__FUNCTION__,__LINE__,pstBufInfo->stRawData.pVirAddr);
                MI_SYS_BUG_ON((unsigned long )pstBufInfo->stRawData.pVirAddr<(unsigned long )PAGE_OFFSET);
                MI_SYS_INFO("   %s:%d  \n",__FUNCTION__,__LINE__);
                return MI_SUCCESS;
            }

            MI_SYS_INFO("   %s:%d  \n",__FUNCTION__,__LINE__);
            if(! pstBufAllocation->ops.vmap_kern )
            {
                MI_SYS_INFO("   %s:%d  pstBufAllocation->ops.vmap_kern is NULL\n",__FUNCTION__,__LINE__);
            }
            pstBufInfo->stRawData.pVirAddr = pstBufAllocation->ops.vmap_kern(pstBufAllocation);


            if (bFlushCache && (pstBufInfo->stRawData.u32ContentSize != 0))
            {
                MI_SYS_IMPL_FlushInvCache(pstBufInfo->stRawData.pVirAddr, pstBufInfo->stRawData.u32ContentSize);
            }

            MI_SYS_INFO("   %s:%d  \n",__FUNCTION__,__LINE__);
            return pstBufInfo->stRawData.pVirAddr?MI_SUCCESS:MI_ERR_SYS_FAILED;

        case E_MI_SYS_BUFDATA_META:
            if(pstBufInfo->stMetaData.pVirAddr)
            {
                //va must be in kernel mapping space
                MI_SYS_BUG_ON((unsigned long )pstBufInfo->stMetaData.pVirAddr<(unsigned long )PAGE_OFFSET);
                return MI_SUCCESS;
            }
            pstBufInfo->stMetaData.pVirAddr = pstBufAllocation->ops.vmap_kern(pstBufAllocation);

            //meta data
            //no flush cache.

            //meta data mapping must success
            MI_SYS_BUG_ON(! pstBufInfo->stMetaData.pVirAddr );
            return pstBufInfo->stMetaData.pVirAddr?MI_SUCCESS:MI_ERR_SYS_FAILED;
        case E_MI_SYS_BUFDATA_FRAME:
            DBG_ERR("doen't support map frame at this time because mapping frame will waste too much kernel va space\n");
            MI_SYS_BUG();//
            if(pstBufInfo->stFrameData.pVirAddr[0])
            {
                //va must be in kernel mapping space
                MI_SYS_BUG_ON((unsigned long)pstBufInfo->stFrameData.pVirAddr[0]<(unsigned long )PAGE_OFFSET);
                return MI_SUCCESS;
            }
            pstBufInfo->stFrameData.pVirAddr[0] = pstBufAllocation->ops.vmap_kern(pstBufAllocation);


            if(bFlushCache && pstBufInfo->stFrameData.u32BufSize)
                MI_SYS_IMPL_FlushInvCache(pstBufInfo->stFrameData.pVirAddr[0], pstBufInfo->stFrameData.u32BufSize);

            if( pstBufInfo->stFrameData.pVirAddr[0] == NULL)
                return MI_SUCCESS;
            if( pstBufInfo->stFrameData.phyAddr[1])
                pstBufInfo->stFrameData.pVirAddr[1]  = (char*)pstBufInfo->stFrameData.pVirAddr[0]
                                                       +(pstBufInfo->stFrameData.phyAddr[1]-pstBufInfo->stFrameData.phyAddr[0]);

            if( pstBufInfo->stFrameData.phyAddr[2])
                pstBufInfo->stFrameData.pVirAddr[2]  = (char*)pstBufInfo->stFrameData.pVirAddr[0]
                                                       +(pstBufInfo->stFrameData.phyAddr[2]-pstBufInfo->stFrameData.phyAddr[0]);
            return MI_SUCCESS;
        default:
            MI_SYS_BUG();
            {
                DBG_ERR("eBufType error\n");
                return MI_ERR_SYS_FAILED;
            }
    }
    MI_SYS_INFO("   %s:%d  \n",__FUNCTION__,__LINE__);
}

//MI_PHY cur_phyddr = 0;

static inline void _mi_sys_QuickClearBuf(MI_PHY u64PHYADDR, MI_U32 u32X, MI_U32 u32Y, MI_U32 u32Width, MI_U32 u32Height,
        MI_U32 u32Stride, MI_U8 color)
{
    void *pVirAddr = NULL;
    MI_PHY cur_phystart = 0;
    MI_PHY cur_phyend = 0;

    BUG_ON(!u64PHYADDR);
    // PAGE_SIZE align
    cur_phystart = ALIGN_DOWN((u64PHYADDR + (u32X * u32Y)), PAGE_SIZE);

    // PAGE_SIZE align
    cur_phyend =  ALIGN_UP((u64PHYADDR + (u32X * (u32Y + u32Height))), PAGE_SIZE);

    DBG_INFO("param:(%d, %d, %d, %d, %d)\n", u32X, u32Y, u32Width, u32Height, u32Stride);

    pVirAddr= MI_SYS_IMPL_Vmap(cur_phystart, (cur_phyend - cur_phystart), FALSE);
    BUG_ON(!pVirAddr);
    DBG_INFO("u64PHYADDR = %x, cur_phyend = %x, cur_phystart = %x\n", (MI_U32)u64PHYADDR, (MI_U32)cur_phyend, (MI_U32)cur_phystart);

    memset(pVirAddr, color, (cur_phyend - cur_phystart));    //black.
    DBG_INFO("PAGE_SIZE = %x\n", ((u32)cur_phyend - (u32)cur_phystart));
    mi_sys_buf_mgr_flush_inv_cache(pVirAddr, (cur_phyend - cur_phystart));
    MI_SYS_IMPL_UnVmap(pVirAddr);

}

static inline void _mi_sys_clear_padding(MI_PHY u64PHYADDR, MI_U32 u32XOffset, MI_U32 u32YOffset, MI_U32 u32XAlignment,
        MI_U32 u32YAlignment, MI_U32 u32Stride, MI_U32 height, MI_U8 color)
{
    MI_U32 u32XEnd;
    MI_U32 u32YEnd;

    DBG_INFO("param:(%d, %d, %d, %d, %d, %d)\n", u32XOffset, u32YOffset, u32XAlignment, u32YAlignment, u32Stride, height);

    if(u32XOffset<u32Stride)
    {
        u32XEnd = ALIGN_UP(u32XOffset, u32XAlignment);
        if(u32XEnd > u32Stride)
            u32XEnd = u32Stride;

        if(u32XEnd > u32XOffset)
        {
            _mi_sys_QuickClearBuf(u64PHYADDR, u32XOffset, u32YOffset, u32XEnd-u32XOffset, u32YOffset, u32Stride, color);
        }
    }
    if(u32YOffset < height)
    {
        u32YEnd = ALIGN_UP(u32YOffset, u32YAlignment);
        if(u32YEnd > height)
            u32YEnd = height;

        if(u32YEnd > u32YOffset)
        {
            _mi_sys_QuickClearBuf(u64PHYADDR, u32XOffset, u32YOffset, u32XOffset, u32YEnd-u32YOffset, u32Stride, color);
        }
    }
}

MI_U32 _mi_clear_framebuf_pading_area(MI_SYS_FrameData_t *pstFrameData, MI_SYS_BufFrameConfig_t *pstFrameCfg)
{
    MI_U16 u16Width = pstFrameData->u16Width;
    MI_U16 u16Height = pstFrameData->u16Height;
    MI_SYS_FrameBufExtraConfig_t *pstFrameBufExtraConf = &pstFrameCfg->stFrameBufExtraConf;
    MI_U16 u16MaxHeight = ALIGN_UP(u16Height, pstFrameBufExtraConf->u16BufVAlignment);

    MI_SYS_BUG_ON(!u16Width || !u16Height);
    MI_SYS_BUG_ON(!pstFrameBufExtraConf);

    if(pstFrameData->eTileMode != E_MI_SYS_FRAME_TILE_MODE_NONE)
        return MI_ERR_SYS_FAILED;

    //HAlignment:alighment in horizontal
    //VAlignment:alighment in vertical

    switch(pstFrameCfg->eFormat)
    {
        // TODO:fix me!
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            _mi_sys_clear_padding(pstFrameData->phyAddr[0], u16Width *  2, u16Height,
                                  pstFrameBufExtraConf->u16BufHAlignment, pstFrameBufExtraConf->u16BufVAlignment,
                                  pstFrameData->u32Stride[0], u16MaxHeight, 0);
            break;
        case E_MI_SYS_PIXEL_FRAME_ARGB8888:
        case E_MI_SYS_PIXEL_FRAME_ABGR8888:
            _mi_sys_clear_padding(pstFrameData->phyAddr[0], u16Width *  4, u16Height,
                                  pstFrameBufExtraConf->u16BufHAlignment, pstFrameBufExtraConf->u16BufVAlignment,
                                  pstFrameData->u32Stride[0], u16MaxHeight, 0);
            break;
        case E_MI_SYS_PIXEL_FRAME_RGB565:
        case E_MI_SYS_PIXEL_FRAME_ARGB1555:
            _mi_sys_clear_padding(pstFrameData->phyAddr[0], u16Width *  2, u16Height,
                                  pstFrameBufExtraConf->u16BufHAlignment, pstFrameBufExtraConf->u16BufVAlignment,
                                  pstFrameData->u32Stride[0], u16MaxHeight, 0);
            break;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422://:need two address
            _mi_sys_clear_padding(pstFrameData->phyAddr[0], u16Width, u16Height,
                                  pstFrameBufExtraConf->u16BufHAlignment, pstFrameBufExtraConf->u16BufVAlignment,
                                  pstFrameData->u32Stride[0], u16MaxHeight, 0);
            _mi_sys_clear_padding(pstFrameData->phyAddr[1], u16Width, u16Height,
                                  pstFrameBufExtraConf->u16BufHAlignment, pstFrameBufExtraConf->u16BufVAlignment,
                                  pstFrameData->u32Stride[1], u16MaxHeight, 128);
            break;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420://:need two address
            _mi_sys_clear_padding(pstFrameData->phyAddr[0], u16Width, u16Height,
                                  pstFrameBufExtraConf->u16BufHAlignment, pstFrameBufExtraConf->u16BufVAlignment,
                                  pstFrameData->u32Stride[0], u16MaxHeight, 0);
            _mi_sys_clear_padding(pstFrameData->phyAddr[1], u16Width, u16Height/2,
                                  pstFrameBufExtraConf->u16BufHAlignment, pstFrameBufExtraConf->u16BufVAlignment/2,
                                  pstFrameData->u32Stride[1], u16MaxHeight/2, 128);
            break;
        default:
            break;
    }
    return MI_SUCCESS;
}

