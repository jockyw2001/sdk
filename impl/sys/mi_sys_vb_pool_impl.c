#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>

#include "mi_print.h"
#include "mi_sys.h"
#include "mi_sys_vb_pool_impl.h"
#include "mi_sys_debug.h"

#define  MI_SYS_VBPOOL_MAX_IDR_SIZE  0x1000

MI_SYS_BufferAllocation_t *mi_sys_vbpool_allocator_alloc(mi_sys_Allocator_t *pstAllocator,
                                       MI_SYS_BufConf_t *pstBufConfig);
int mi_sys_vbpool_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *pstBufConfig);
int mi_sys_vbpool_allocator_on_release(struct mi_sys_Allocator_s *pstAllocator);
void mi_sys_vbpool_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation);
void *mi_sys_vbpool_allocation_map_user(MI_SYS_BufferAllocation_t *pstAllocation);
void mi_sys_vbpool_allocation_unmap_user( MI_SYS_BufferAllocation_t *pstAllocation);
extern MI_SYS_BufferAllocation_t *mi_sys_alloc_from_mma_allocators( const char *mma_heap_name, MI_SYS_BufConf_t *pstBufConfig);
void *mi_sys_vbpool_allocation_vmap_kernel(MI_SYS_BufferAllocation_t *pstAllocation);
void mi_sys_vbpool_allocation_vunmap_kernel( MI_SYS_BufferAllocation_t *pstAllocation);
struct kmem_cache *mi_sys_vbpool_allocation_cachep;

buf_allocator_ops_t vbpool_allocator_ops =
{
    .OnRef = generic_allocator_on_ref,
    .OnUnref = generic_allocator_on_unref,
    .OnRelease = mi_sys_vbpool_allocator_on_release,
    .alloc = mi_sys_vbpool_allocator_alloc,
    .suit_bufconfig = mi_sys_vbpool_allocator_suit_bufconfig
};

buf_allocation_ops_t vbpool_allocation_ops =
{
    .OnRef = generic_allocation_on_ref,
    .OnUnref = generic_allocation_on_unref,
    .OnRelease = mi_sys_vbpool_allocation_on_release,
    .map_user = mi_sys_vbpool_allocation_map_user,
    .unmap_user = mi_sys_vbpool_allocation_unmap_user,
    .vmap_kern =  mi_sys_vbpool_allocation_vmap_kernel,
    .vunmap_kern = mi_sys_vbpool_allocation_vunmap_kernel
};

/*
mma_heap_name :alloc from which heap,if set NULL,means no care in which mma heap
*/
mi_sys_Allocator_t *mi_sys_vbpool_allocator_create(const char *mma_heap_name, unsigned long size, unsigned long count)
{
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    MI_SYS_BufferAllocation_t *pst_mma_allocation;
    mi_sys_vbpool_allocation_t *vbpool_allocation;
    int size_aligned = ALIGN_UP(size, PAGE_SIZE);
    unsigned long long cpu_bus_addr;
    int allocated_count = 0;
    MI_SYS_BufConf_t buf_config;

    struct sg_table *sg_table;
    int ret;
    struct scatterlist *sg;
    MI_SYS_INFO("%s : %d\n",__FUNCTION__,__LINE__);
    MI_SYS_BUG_ON(size==0);
    MI_SYS_BUG_ON(count==0);

    pst_vbpool_allocator = (mi_sys_vbpool_allocator_t*)kmalloc(sizeof(mi_sys_vbpool_allocator_t),GFP_KERNEL);
    if(pst_vbpool_allocator  == NULL)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

    generic_allocator_init(&pst_vbpool_allocator->stdAllocator, &vbpool_allocator_ops);
    pst_vbpool_allocator->u32MagicNumber = __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__;
    sema_init(&pst_vbpool_allocator->semlock, 1);

    INIT_LIST_HEAD(&pst_vbpool_allocator->list_of_all_allocations);
    INIT_LIST_HEAD(&pst_vbpool_allocator->list_of_free_allocations);

    pst_vbpool_allocator->size = size_aligned;
    pst_vbpool_allocator->count = count;

    buf_config.eBufType = E_MI_SYS_BUFDATA_RAW;
    buf_config.u64TargetPts = 0;
    buf_config.stRawCfg.u32Size = size_aligned;

    sg_table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
    if (!sg_table)
    {
        DBG_ERR(" fail\n");
        kfree(pst_vbpool_allocator);
        return NULL;
    }

    ret = sg_alloc_table(sg_table, count, GFP_KERNEL);
    if (unlikely(ret))
    {
        kfree(pst_vbpool_allocator);
        kfree(sg_table);
        DBG_ERR(" fail\n");
        return NULL;
    }

    pst_vbpool_allocator->sg_table = sg_table;
    sg=sg_table->sgl;
    MI_SYS_INFO("%s : %d\n",__FUNCTION__,__LINE__);
    allocated_count = 0;
    while(allocated_count < count)
    {
        //MI_SYS_INFO("%s : %d   allocated_count=0x%x  count=0x%lx\n",__FUNCTION__,__LINE__,allocated_count,count);
        pst_mma_allocation = mi_sys_alloc_from_mma_allocators(mma_heap_name, &buf_config);
        if(!pst_mma_allocation)
        {
            MI_SYS_INFO("%s : %d\n",__FUNCTION__,__LINE__);
            goto failed_n_free;
        }
        pst_mma_allocation->ops.OnRef(pst_mma_allocation);
        vbpool_allocation = (mi_sys_vbpool_allocation_t*)kmem_cache_alloc(mi_sys_vbpool_allocation_cachep, GFP_KERNEL);
        if(!vbpool_allocation)
        {
            pst_mma_allocation->ops.OnUnref(pst_mma_allocation);
            DBG_ERR(" fail\n");
            goto failed_n_free;
        }
        vbpool_allocation->offset_in_vb_pool = allocated_count*size_aligned;
        vbpool_allocation->pst_mma_allocation = pst_mma_allocation;
        vbpool_allocation->pstParentAllocator = pst_vbpool_allocator;
        generic_allocation_init(&vbpool_allocation->stdBufAllocation, &vbpool_allocation_ops);
        list_add_tail(&vbpool_allocation->list_in_all,   &pst_vbpool_allocator->list_of_all_allocations);
        list_add_tail(&vbpool_allocation->list_in_free,   &pst_vbpool_allocator->list_of_free_allocations);

        cpu_bus_addr = mi_sys_Miu2Cpu_BusAddr(vbpool_allocation->pst_mma_allocation->stBufInfo.stRawData.phyAddr);
        sg_set_page(sg, pfn_to_page(__phys_to_pfn(cpu_bus_addr)), PAGE_ALIGN(vbpool_allocation->pst_mma_allocation->stBufInfo.stRawData.u32BufSize), 0);
        sg = sg_next(sg);
        allocated_count ++;
    }
    MI_SYS_INFO("%s : %d\n",__FUNCTION__,__LINE__);
    pst_vbpool_allocator->kern_map_ptr = NULL;
    return &pst_vbpool_allocator->stdAllocator;

failed_n_free:
    MI_SYS_INFO("%s : %d\n",__FUNCTION__,__LINE__);
    while(!list_empty( &pst_vbpool_allocator->list_of_all_allocations))
    {
        vbpool_allocation = container_of(pst_vbpool_allocator->list_of_all_allocations.next, mi_sys_vbpool_allocation_t, list_in_all);
        vbpool_allocation->pst_mma_allocation->ops.OnUnref(vbpool_allocation->pst_mma_allocation);
        list_del(&vbpool_allocation->list_in_all);
        memset(vbpool_allocation, 0x37, sizeof(*vbpool_allocation));
        kmem_cache_free(mi_sys_vbpool_allocation_cachep, vbpool_allocation);
    }
    MI_SYS_INFO("%s : %d\n",__FUNCTION__,__LINE__);
    sg_free_table(pst_vbpool_allocator->sg_table);
    kfree(pst_vbpool_allocator->sg_table);
    memset(pst_vbpool_allocator, 0x38, sizeof(*pst_vbpool_allocator));
    kfree(pst_vbpool_allocator);
    DBG_ERR(" fail\n");
    return NULL;
}

static inline void mi_sys_vbpool_allocator_free(mi_sys_vbpool_allocator_t *pst_vbpool_allocator, mi_sys_vbpool_allocation_t *pst_vbpool_allocation)
{
    MI_SYS_BUG_ON(!pst_vbpool_allocator);
    MI_SYS_BUG_ON(!pst_vbpool_allocation);
    MI_SYS_BUG_ON(pst_vbpool_allocation->pstParentAllocator != pst_vbpool_allocator);
    down(&pst_vbpool_allocator->semlock);
    list_add_tail(&pst_vbpool_allocation->list_in_free, &pst_vbpool_allocator->list_of_free_allocations);
    up(&pst_vbpool_allocator->semlock);

    //release reference to vbpool allocation reference
    pst_vbpool_allocator->stdAllocator.ops->OnUnref(&pst_vbpool_allocator->stdAllocator);
}

MI_SYS_BufferAllocation_t *mi_sys_vbpool_allocator_alloc(mi_sys_Allocator_t *pstAllocator,
                                       MI_SYS_BufConf_t *pstBufConfig)
{
    mi_sys_vbpool_allocation_t *vbpool_allocation;
    unsigned long size_to_alloc;
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    MI_SYS_FrameSpecial_t  stFrameDataInfo;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufConfig);

    pst_vbpool_allocator = container_of(pstAllocator, mi_sys_vbpool_allocator_t, stdAllocator);
    MI_SYS_BUG_ON(pst_vbpool_allocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);

    size_to_alloc = mi_sys_buf_mgr_get_size(pstBufConfig,&stFrameDataInfo);

    if(size_to_alloc > pst_vbpool_allocator->size)
    {
        DBG_ERR(" fail   size_to_alloc=0x%lx   pst_vbpool_allocator->size=0x%lx\n",size_to_alloc,pst_vbpool_allocator->size);
        return NULL;
    }

    down(&pst_vbpool_allocator->semlock);
    if(!list_empty(&pst_vbpool_allocator->list_of_free_allocations))
    {
        vbpool_allocation = container_of(pst_vbpool_allocator->list_of_free_allocations.next, mi_sys_vbpool_allocation_t,list_in_free);
        MI_SYS_BUG_ON(atomic_read(&vbpool_allocation->stdBufAllocation.ref_cnt));
        MI_SYS_BUG_ON(vbpool_allocation->pst_mma_allocation==NULL ||
              vbpool_allocation->pstParentAllocator!=pst_vbpool_allocator);
        MI_SYS_BUG_ON( vbpool_allocation->stdBufAllocation.ops.map_user != vbpool_allocation_ops.map_user);
        list_del(&vbpool_allocation->list_in_free);
        INIT_LIST_HEAD(&vbpool_allocation->list_in_free);
        up(&pst_vbpool_allocator->semlock);
        MI_SYS_BUG_ON( vbpool_allocation->pst_mma_allocation->stBufInfo.eBufType != E_MI_SYS_BUFDATA_RAW);
        MI_SYS_BUG_ON( vbpool_allocation->pst_mma_allocation->stBufInfo.stRawData.u32BufSize <pst_vbpool_allocator->size);
        mi_sys_buf_mgr_fill_bufinfo(&vbpool_allocation->stdBufAllocation.stBufInfo,
                                             pstBufConfig, &stFrameDataInfo, vbpool_allocation->pst_mma_allocation->stBufInfo.stRawData.phyAddr);

        if(pstBufConfig->stFrameCfg.stFrameBufExtraConf.bClearPadding && \
            vbpool_allocation->stdBufAllocation.stBufInfo.eBufType == E_MI_SYS_BUFDATA_FRAME)
        {
            _mi_clear_framebuf_pading_area(&vbpool_allocation->stdBufAllocation.stBufInfo.stFrameData, &pstBufConfig->stFrameCfg);
        }

        //add reference to vbpool allocator for allocator lifecycle management
        pstAllocator->ops->OnRef(pstAllocator);
        return (MI_SYS_BufferAllocation_t *)(&(vbpool_allocation->stdBufAllocation));
    }
    up(&pst_vbpool_allocator->semlock);
    DBG_ERR(" fail\n");
    return NULL;
}

static inline int _mi_sys_vb_pool_score(unsigned long redundant_size)
{
    const unsigned long max_waste_sze = 1024*1024*4;
    MI_SYS_BUG_ON(redundant_size < 0);

    if(max_waste_sze <= redundant_size)
    {
        return (INT_MIN+1);// Did better than do nothing.
    }
    if(redundant_size ==0)
        return 4096;

    return 4096-(redundant_size*256/max_waste_sze)*16;
}

int mi_sys_vbpool_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *pstBufConfig)
{
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    unsigned long size_to_alloc;
    MI_SYS_FrameSpecial_t  stFrameDataInfo;
    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufConfig);

    pst_vbpool_allocator = container_of(pstAllocator, mi_sys_vbpool_allocator_t, stdAllocator);

    MI_SYS_BUG_ON(pst_vbpool_allocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);

    size_to_alloc = mi_sys_buf_mgr_get_size(pstBufConfig,&stFrameDataInfo);

    if(pstBufConfig->eBufType == E_MI_SYS_BUFDATA_META)
         return INT_MIN;//means VB POOL refuse BUFDATA_META

    if(pst_vbpool_allocator->size < size_to_alloc)
    {
        return INT_MIN;
    }

   // down(&pst_vbpool_allocator->semlock);
    if(list_empty(&pst_vbpool_allocator->list_of_free_allocations))
    {
        DBG_INFO("empty free_allocations  allocator_size=0x%lx   size_to_alloc=0x%lx\n",pst_vbpool_allocator->size,size_to_alloc);

        //schedule_timeout_interruptible(msecs_to_jiffies(10));//wait for a moment

        //if still not have free allocations,return the min score
        if(list_empty(&pst_vbpool_allocator->list_of_free_allocations))
        {
            DBG_INFO("after wait ,allocator still not have free allocations,allocator=%p,allocator_size=0x%lx,size_to_alloc=0x%lx\n"
                                                                      ,pst_vbpool_allocator ,pst_vbpool_allocator->size,size_to_alloc);
            return INT_MIN;
        }
    }
    //up(&pst_vbpool_allocator->semlock);

    return _mi_sys_vb_pool_score(pst_vbpool_allocator->size - size_to_alloc);
}

int mi_sys_vbpool_allocator_on_release(struct mi_sys_Allocator_s *pstAllocator)
{
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    mi_sys_vbpool_allocation_t *pst_vbpool_allocation;
    int ret ;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);

    MI_SYS_BUG_ON(atomic_read(&pstAllocator->ref_cnt));

    pst_vbpool_allocator = container_of(pstAllocator, mi_sys_vbpool_allocator_t, stdAllocator);

    MI_SYS_BUG_ON(pst_vbpool_allocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);

    //return MI_SUCCESS means user unmap success,
    //return other value means may not have user va,
    //so here no care return value of  mi_sys_buf_mgr_user_unmap().
    ret = mi_sys_buf_mgr_user_unmap(pst_vbpool_allocator);
    if(pst_vbpool_allocator->kern_map_ptr)
        mi_sys_buf_mgr_vunmap_kern(pst_vbpool_allocator->kern_map_ptr);
    pst_vbpool_allocator->kern_map_ptr = NULL;

    while(!list_empty( &pst_vbpool_allocator->list_of_all_allocations))
    {
        pst_vbpool_allocation = container_of(pst_vbpool_allocator->list_of_all_allocations.next, mi_sys_vbpool_allocation_t, list_in_all);
        MI_SYS_BUG_ON(pst_vbpool_allocation->pstParentAllocator != pst_vbpool_allocator);
        MI_SYS_BUG_ON(!pst_vbpool_allocation->pst_mma_allocation);
        MI_SYS_BUG_ON(list_empty(&pst_vbpool_allocation->list_in_free));
        pst_vbpool_allocation->pst_mma_allocation->ops.OnUnref(pst_vbpool_allocation->pst_mma_allocation);

        list_del(&pst_vbpool_allocation->list_in_all);
        list_del(&pst_vbpool_allocation->list_in_free);

        memset(pst_vbpool_allocation, 0x39, sizeof(*pst_vbpool_allocation));
        kmem_cache_free(mi_sys_vbpool_allocation_cachep, pst_vbpool_allocation);
    }

    MI_SYS_BUG_ON(!list_empty(&pst_vbpool_allocator->list_of_free_allocations));
    sg_free_table(pst_vbpool_allocator->sg_table);
    kfree(pst_vbpool_allocator->sg_table);
    memset(pst_vbpool_allocator, 0x40, sizeof(*pst_vbpool_allocator));
    kfree(pst_vbpool_allocator);
    return MI_SUCCESS;
}

void mi_sys_vbpool_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation)
{
    mi_sys_vbpool_allocation_t *pst_vbpool_allocation;
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;

    MI_SYS_BUG_ON(!pst_allocation || pst_allocation->ops.map_user != vbpool_allocation_ops.map_user);

    pst_vbpool_allocation = container_of(pst_allocation, mi_sys_vbpool_allocation_t, stdBufAllocation);
    pst_vbpool_allocator = pst_vbpool_allocation->pstParentAllocator;
    MI_SYS_BUG_ON(!pst_vbpool_allocation->pstParentAllocator);
    MI_SYS_BUG_ON(atomic_read(&pst_allocation->ref_cnt));
    MI_SYS_BUG_ON(pst_vbpool_allocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);
    mi_sys_vbpool_allocator_free(pst_vbpool_allocation->pstParentAllocator, pst_vbpool_allocation);
}

static inline void *mi_sys_vbpool_allocator_map_usr(mi_sys_vbpool_allocator_t*pst_vbpool_allocator,  mi_sys_vbpool_allocation_t*pst_vbpool_allocation)
{
    int ret;
    void *user_map_ptr;
    MI_SYS_BUG_ON(!pst_vbpool_allocator);
    MI_SYS_BUG_ON(!pst_vbpool_allocation);
    ret = mi_sys_buf_mgr_user_map(pst_vbpool_allocator->sg_table, &user_map_ptr, pst_vbpool_allocator,0);
    if(ret != MI_SUCCESS)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

    return pst_vbpool_allocation->offset_in_vb_pool+user_map_ptr;
}

static inline void *mi_sys_vbpool_allocator_vmap_kernel(mi_sys_vbpool_allocator_t*pst_vbpool_allocator,  mi_sys_vbpool_allocation_t*pst_vbpool_allocation)
{
    int ret;
    void *kern_map_ptr;
    MI_SYS_BUG_ON(!pst_vbpool_allocator);
    MI_SYS_BUG_ON(!pst_vbpool_allocation);
    if(pst_vbpool_allocator->kern_map_ptr)
        return pst_vbpool_allocator->kern_map_ptr+pst_vbpool_allocation->offset_in_vb_pool;

    ret = mi_sys_buf_mgr_vmap_kern(pst_vbpool_allocator->sg_table, &kern_map_ptr);
    if(ret != MI_SUCCESS)
    {
        MI_SYS_BUG_ON(kern_map_ptr);
        DBG_ERR(" fail\n");
        return NULL;
    }
    MI_SYS_BUG_ON(!kern_map_ptr);
    pst_vbpool_allocator->kern_map_ptr = kern_map_ptr;
    return pst_vbpool_allocator->kern_map_ptr+pst_vbpool_allocation->offset_in_vb_pool;
}

static inline void mi_sys_vbpool_allocator_unmap_usr(mi_sys_vbpool_allocator_t*pst_vbpool_allocator)
{
    int ret;
    MI_SYS_BUG_ON(!pst_vbpool_allocator);
    MI_SYS_BUG_ON(pst_vbpool_allocator->u32MagicNumber!=__MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);

    ret = mi_sys_buf_mgr_user_unmap(pst_vbpool_allocator);
    if(ret != MI_SUCCESS)
    {
        //if mi_sys_buf_mgr_user_unmap not success,do nothing.
        DBG_WRN("fail\n");
    }
}

void *mi_sys_vbpool_allocation_map_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
    mi_sys_vbpool_allocation_t *pst_vbpool_allocation;
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    MI_SYS_BUG_ON(!pstAllocation);
    MI_SYS_BUG_ON(pstAllocation->ops.map_user != vbpool_allocation_ops.map_user);
    pst_vbpool_allocation = container_of(pstAllocation, mi_sys_vbpool_allocation_t, stdBufAllocation);
    pst_vbpool_allocator = pst_vbpool_allocation->pstParentAllocator;
    MI_SYS_BUG_ON(!pst_vbpool_allocation->pstParentAllocator || pst_vbpool_allocation->pstParentAllocator->u32MagicNumber   !=__MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);
    return mi_sys_vbpool_allocator_map_usr(pst_vbpool_allocator, pst_vbpool_allocation);
}
void mi_sys_vbpool_allocation_unmap_user( MI_SYS_BufferAllocation_t *pstAllocation)
{
    MI_SYS_BUG_ON(!pstAllocation);
    MI_SYS_BUG_ON(pstAllocation->ops.map_user != vbpool_allocation_ops.map_user);
    //do nothing
}

void *mi_sys_vbpool_allocation_vmap_kernel(MI_SYS_BufferAllocation_t *pstAllocation)
{
    mi_sys_vbpool_allocation_t *pst_vbpool_allocation;
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    MI_SYS_BUG_ON(!pstAllocation);
    MI_SYS_BUG_ON(pstAllocation->ops.vmap_kern != vbpool_allocation_ops.vmap_kern);
    pst_vbpool_allocation = container_of(pstAllocation, mi_sys_vbpool_allocation_t, stdBufAllocation);
    pst_vbpool_allocator = pst_vbpool_allocation->pstParentAllocator;
    MI_SYS_BUG_ON(!pst_vbpool_allocation->pstParentAllocator || pst_vbpool_allocation->pstParentAllocator->u32MagicNumber !=__MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);
    return mi_sys_vbpool_allocator_vmap_kernel(pst_vbpool_allocator, pst_vbpool_allocation);
}
void mi_sys_vbpool_allocation_vunmap_kernel( MI_SYS_BufferAllocation_t *pstAllocation)
{
    MI_SYS_BUG_ON(!pstAllocation);
    MI_SYS_BUG_ON(pstAllocation->ops.vunmap_kern != vbpool_allocation_ops.vunmap_kern);
    //do nothing
}

#ifdef MI_SYS_PROC_FS_DEBUG
void _vbpool_deal_with_mi_sys_Allocator_t(MI_SYS_DEBUG_HANDLE_t  handle,mi_sys_Allocator_t * pstAllocator)
{
    mi_sys_vbpool_allocation_t *pst_vbpool_allocation;
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    struct list_head *pos;
    struct list_head *pos_2;
    unsigned long not_used_count = 0;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);

    pst_vbpool_allocator = container_of(pstAllocator, mi_sys_vbpool_allocator_t, stdAllocator);
    down(&pst_vbpool_allocator->semlock);
    handle.OnPrintOut(handle,"-------start of vbpool_allocator info--------\n");

    not_used_count = 0;
    list_for_each(pos_2,&pst_vbpool_allocator->list_of_free_allocations)
    {
        pst_vbpool_allocation = container_of(pos_2, mi_sys_vbpool_allocation_t, list_in_free);
        MI_SYS_BUG_ON(!pst_vbpool_allocation);
        not_used_count ++;
    }

    handle.OnPrintOut(handle,"\n\nvbpool_allocator info:\n");
    handle.OnPrintOut(handle,"%25s%25s%35s%15s\n","each_allocation_size","total_allocation_count"
                                ,"total_not_used_allocation_count","kern_map_ptr");
    handle.OnPrintOut(handle,"%25lx%25lx",pst_vbpool_allocator->size,pst_vbpool_allocator->count);

    handle.OnPrintOut(handle,"%35lx",not_used_count);

    handle.OnPrintOut(handle,"%15p\n",pst_vbpool_allocator->kern_map_ptr);

    handle.OnPrintOut(handle,"\nvbpool_allocation info:\n");
    list_for_each(pos,&pst_vbpool_allocator->list_of_all_allocations)
    {
        handle.OnPrintOut(handle,"\n\n%20s%15s%10s%15s%15s%10s%15s\n"
                                ,"offset_in_vb_pool","real_used_flag","u64Pts"
                                ,"u64SidebandMsg","bEndOfStream","bUsrBuf","eBufType");
        pst_vbpool_allocation = container_of(pos, mi_sys_vbpool_allocation_t, list_in_all);
        handle.OnPrintOut(handle,"%20x",pst_vbpool_allocation->offset_in_vb_pool);
        if(list_empty(&pst_vbpool_allocation->list_in_free))//used allocation
        {
            handle.OnPrintOut(handle,"%15s%10llx%15llx%15s%10s"
                                          ,"1",pst_vbpool_allocation->pst_mma_allocation->stBufInfo.u64Pts
                                          ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.u64SidebandMsg
                                          ,(pst_vbpool_allocation->pst_mma_allocation->stBufInfo.bEndOfStream== false) ? "false":"true"
                                          ,(pst_vbpool_allocation->pst_mma_allocation->stBufInfo.bUsrBuf== false) ? "false":"true");

            if(pst_vbpool_allocation->pst_mma_allocation->stBufInfo.eBufType == E_MI_SYS_BUFDATA_RAW)
            {
                handle.OnPrintOut(handle,"%15s\n","BUFDATA_RAW");//here use \n
                handle.OnPrintOut(handle,"pVirAddr  phyAddr  u32BufSize   u32ContentSize  bEndOfFrame\n");

                handle.OnPrintOut(handle,"%p  0x%llx   0x%x   0x%x  %s\n"
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stRawData.pVirAddr
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stRawData.phyAddr
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stRawData.u32BufSize
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stRawData.u32ContentSize
                                    ,(pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stRawData.bEndOfFrame == false) ? "false":"true");
            }
            else if(pst_vbpool_allocation->pst_mma_allocation->stBufInfo.eBufType == E_MI_SYS_BUFDATA_FRAME)
            {
                handle.OnPrintOut(handle,"%15s\n","BUFDATA_FRAME");//here use \n
                handle.OnPrintOut(handle,"eTileMode ePixelFormat  eCompressMode  eFrameScanMode  eFieldType  [u16Width u16Height]  [pVirAddr0 pVirAddr1 pVirAddr2]  [phyAddr0 phyAddr1 phyAddr2]  [u32Stride0 u32Stride1 u32Stride2]\n");
                handle.OnPrintOut(handle,"%d %d %d %d %d  [0x%04x 0x%04x]  [%p  %p  %p]  [0x%llx   0x%llx   0x%llx]  [0x%x  0x%x  0x%x]\n"
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.eTileMode
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.ePixelFormat
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.eCompressMode
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.eFrameScanMode
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.eFieldType
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.u16Width
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.u16Height
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.pVirAddr[0]
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.pVirAddr[1]
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.pVirAddr[2]
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.phyAddr[0]
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.phyAddr[1]
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.phyAddr[2]
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.u32Stride[0]
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.u32Stride[1]
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stFrameData.u32Stride[2]);
            }
            else if(pst_vbpool_allocation->pst_mma_allocation->stBufInfo.eBufType == E_MI_SYS_BUFDATA_META)
            {
                handle.OnPrintOut(handle,"%15s\n","BUFDATA_META");//here use \n
                handle.OnPrintOut(handle,"pVirAddr phyAddr  u32Size  u32ExtraData  eDataFromModule\n");
                handle.OnPrintOut(handle,"%p  0x%llx   0x%x   0x%x    %d\n"
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stMetaData.pVirAddr
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stMetaData.phyAddr
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stMetaData.u32Size
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stMetaData.u32ExtraData
                                    ,pst_vbpool_allocation->pst_mma_allocation->stBufInfo.stMetaData.eDataFromModule);
            }
            else
            {
                MI_SYS_BUG();
            }
        }
        else//not used allocation
        {
            handle.OnPrintOut(handle,"%15s%10s%15s%15s%10s%15s\n",
                                        "0","NA","NA","NA","NA","NA");
        }
    }

    handle.OnPrintOut(handle,"-------end of vbpool_allocator info--------\n\n");
    up(&pst_vbpool_allocator->semlock);
}

MI_S32 mi_sys_vbpool_allocator_dump_data(char *dir_and_file_name,struct mi_sys_Allocator_s *pstAllocator,MI_U32 offset_in_allocator,MI_U32 length)
{
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    mi_sys_vbpool_allocation_t *pst_vbpool_allocation;
    struct list_head *pos;
    MI_U32 current_offset;
    mm_segment_t old_fs;
    struct file *fp;
    MI_S32 ret;
    void *va = NULL;
    MI_U32 backup_length = length;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);

    pst_vbpool_allocator = container_of(pstAllocator, mi_sys_vbpool_allocator_t, stdAllocator);

    MI_SYS_BUG_ON(pst_vbpool_allocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(dir_and_file_name,O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if(IS_ERR(fp))
    {
        set_fs(old_fs);
        return E_MI_ERR_FAILED;
    }

    down(&pst_vbpool_allocator->semlock);
    current_offset = 0;
    list_for_each(pos,&pst_vbpool_allocator->list_of_all_allocations)
    {
        if(offset_in_allocator >= current_offset && offset_in_allocator <= current_offset + pst_vbpool_allocator->size)
        {
            pst_vbpool_allocation = container_of(pos, mi_sys_vbpool_allocation_t, list_in_all);
            BUG_ON(pst_vbpool_allocation->offset_in_vb_pool != current_offset);

            va = mi_sys_vbpool_allocator_map_usr(pst_vbpool_allocator,  pst_vbpool_allocation);
            if(!va)
            {
                up(&pst_vbpool_allocator->semlock);
                filp_close(fp, NULL);
                set_fs(old_fs);
                return E_MI_ERR_FAILED;
            }

            if(offset_in_allocator + length <= current_offset + pst_vbpool_allocator->size)
            {
                //dump data in [offset_in_allocator,offset_in_allocator+length] of this pst_vbpool_allocator
                ret = vfs_write(fp, (char *)va+(offset_in_allocator-current_offset), length, &fp->f_pos);
                mi_sys_vbpool_allocator_unmap_usr(pst_vbpool_allocator);
                up(&pst_vbpool_allocator->semlock);
                set_fs(old_fs);
                if(ret != length)
                {
                    DBG_ERR("fail   ret=0x%x, length=0x%x\n",ret,length);
                    filp_close(fp, NULL);
                    return E_MI_ERR_FAILED;
                }
                else
                {
                    BUG_ON(backup_length != fp->f_pos);
                    filp_close(fp, NULL);
                    return MI_SUCCESS;//finish ,return!
                }
            }
            else
            {
                //dump data in [offset_in_allocator,current_offset + pst_vbpool_allocator->size - offset_in_allocator]
                //of this pst_vbpool_allocator
                ret = vfs_write(fp, (char *)va+(offset_in_allocator-current_offset), current_offset + pst_vbpool_allocator->size - offset_in_allocator, &fp->f_pos);
                mi_sys_vbpool_allocator_unmap_usr(pst_vbpool_allocator);

                if(ret != current_offset + pst_vbpool_allocator->size - offset_in_allocator)
                {
                    DBG_ERR("fail   ret=0x%x, this length=0x%x\n",ret,current_offset + pst_vbpool_allocator->size - offset_in_allocator);
                    up(&pst_vbpool_allocator->semlock);
                    filp_close(fp, NULL);
                    set_fs(old_fs);
                    return E_MI_ERR_FAILED;
                }

                //after dump,change value
                length -= current_offset + pst_vbpool_allocator->size - offset_in_allocator;//change length
                offset_in_allocator = current_offset + pst_vbpool_allocator->size;//and then change offset_in_allocator
            }
        }

        current_offset += pst_vbpool_allocator->size;
    }
    up(&pst_vbpool_allocator->semlock);
    set_fs(old_fs);
    filp_close(fp, NULL);
    DBG_ERR("error,fail,should not return from here\n");

    return E_MI_ERR_FAILED;
}

MI_S32 mi_sys_vbpool_allocator_get_info(struct mi_sys_Allocator_s *pstAllocator,MI_U32 *size,MI_U32 *count)
{
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);

    pst_vbpool_allocator = container_of(pstAllocator, mi_sys_vbpool_allocator_t, stdAllocator);

    MI_SYS_BUG_ON(pst_vbpool_allocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);
    *size = pst_vbpool_allocator->size;
    *count = pst_vbpool_allocator->count;
    return MI_SUCCESS;
}
#endif

///

