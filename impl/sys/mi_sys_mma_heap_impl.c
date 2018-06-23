#include "mi_sys_mma_miu_protect_impl.h"
#include <linux/types.h>
#include "mi_sys_internal.h"
#include "mi_sys_buf_mgr.h"
#include "mi_sys_mma_heap_impl.h"
#include <linux/slab.h>
#include "mi_sys_chunk_impl.h"
#include <linux/scatterlist.h>
#include "mi_sys_debug.h"

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

#ifdef MI_SYS_PROC_FS_DEBUG
#include "drv_proc_ext_k.h"
#endif

DEFINE_SEMAPHORE(g_mma_list_semlock);
LIST_HEAD(g_mma_heap_list);
struct kmem_cache *mi_sys_mma_allocation_cachep;

MI_S32 mi_sys_mma_allocator_on_release(mi_sys_Allocator_t *pstAllocator);
MI_SYS_BufferAllocation_t *mi_sys_mma_allocator_alloc(mi_sys_Allocator_t *pstAllocator,
                                            MI_SYS_BufConf_t *pstBufConfig );
int mi_sys_mma_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *pstBufConfig);
void mi_sys_mma_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation);
void *mi_sys_mma_allocation_map_user(MI_SYS_BufferAllocation_t *pstAllocation);
void mi_sys_mma_allocation_unmap_user(MI_SYS_BufferAllocation_t *pstAllocation);
void *mi_sys_mma_allocation_vmap_kern(MI_SYS_BufferAllocation_t *pstAllocation);
void mi_sys_mma_allocation_vunmap_kern( MI_SYS_BufferAllocation_t *pstAllocation);
int mi_sys_buf_mgr_vmap_kern(struct sg_table *sg_table,void **pp_kern_map_ptr);

buf_allocator_ops_t mma_allocator_ops =
{
    .OnRef = generic_allocator_on_ref,
    .OnUnref = generic_allocator_on_unref,
    .OnRelease = mi_sys_mma_allocator_on_release,
    .alloc = mi_sys_mma_allocator_alloc,
    .suit_bufconfig = mi_sys_mma_allocator_suit_bufconfig
};

buf_allocation_ops_t mma_allocation_ops =
{
    .OnRef = generic_allocation_on_ref,
    .OnUnref = generic_allocation_on_unref,
    .OnRelease = mi_sys_mma_allocation_on_release,
    .map_user = mi_sys_mma_allocation_map_user,
    .unmap_user = mi_sys_mma_allocation_unmap_user,
    .vmap_kern =  mi_sys_mma_allocation_vmap_kern,
    .vunmap_kern = mi_sys_mma_allocation_vunmap_kern
};

unsigned short *g_kernel_protect_client_id = NULL;

//As _MI_SYS_IMPL_AllocBufDefaultPolicy show,
//mma allocator will not attach to collection.
mi_sys_mma_allocator_t *mi_sys_mma_allocator_create(const char *mma_heap_name, unsigned long long cpu_bus_addr, unsigned long length)
{
    mi_sys_mma_allocator_t *pst_mma_allocator;
    mi_sys_mma_allocator_t *tmp_pst_mma_allocator;
    //char *heap_name;
    //int name_length;
    int ret;
    struct list_head *pos;
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_AllocatorProcfsOps_t Ops;
#endif
    MI_SYS_INFO("%s:%d\n",__FUNCTION__,__LINE__);
    MI_SYS_BUG_ON(cpu_bus_addr & ~PAGE_MASK);
    MI_SYS_BUG_ON(length & ~PAGE_MASK);
    MI_SYS_BUG_ON(!length);

    down(&g_mma_list_semlock);
    list_for_each(pos, &g_mma_heap_list)
    {
        tmp_pst_mma_allocator = list_entry(pos, mi_sys_mma_allocator_t, list);
        if((tmp_pst_mma_allocator->heap_name[0] != 0)
            &&( mma_heap_name != NULL))//if name is NULL,no care whether same
        {
            if((0 == strncmp(tmp_pst_mma_allocator->heap_name,mma_heap_name,strlen(mma_heap_name)))
                &&(tmp_pst_mma_allocator->heap_base_cpu_bus_addr == cpu_bus_addr)
                &&(tmp_pst_mma_allocator->length == length))//already exist this mma heap
            {
                    DBG_ERR("mma heap already exist! mma_heap_name=%s   cpu_bus_addr=0x%llx   length=0x%lx\n",mma_heap_name,cpu_bus_addr,length);
                    up(&g_mma_list_semlock);
                    return NULL;
            }
        }
    }
    up(&g_mma_list_semlock);

    pst_mma_allocator = (mi_sys_mma_allocator_t*)kmalloc(sizeof(mi_sys_mma_allocator_t),GFP_KERNEL);
    if(pst_mma_allocator  == NULL)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }
    generic_allocator_init(&pst_mma_allocator->stdAllocator, &mma_allocator_ops);
    pst_mma_allocator->u32MagicNumber = __MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__;
    pst_mma_allocator->heap_base_cpu_bus_addr = cpu_bus_addr;
    pst_mma_allocator->length = length;
    sema_init(&pst_mma_allocator->semlock, 1);
    MI_SYS_INFO("%s:%d\n",__FUNCTION__,__LINE__);
    if(mma_heap_name)
    {
        strncpy(pst_mma_allocator->heap_name, mma_heap_name, MI_SYS_MMA_HEAP_NAME_MAX_LEN);
        pst_mma_allocator->heap_name[MI_SYS_MMA_HEAP_NAME_MAX_LEN-1] = 0;
    }
    else
    {
        pst_mma_allocator->heap_name[0] = 0;
    }
    sema_init(&g_mma_list_semlock, 1);
    ret = mi_sys_init_chunk(&pst_mma_allocator->chunk_mgr, length);//start from offset 0 in mma heap
    if(ret != MI_SUCCESS)
    {
        kfree(pst_mma_allocator);
        DBG_ERR("mi_sys_init_chunk fail\n");
        return NULL;
    }
    pst_mma_allocator->stdAllocator.ops->OnRef(&pst_mma_allocator->stdAllocator);
    down(&g_mma_list_semlock);
    list_add_tail(&pst_mma_allocator->list,&g_mma_heap_list);//list_add will add to HEAD, in accordance with mma_config,here use list_add_tail
    up(&g_mma_list_semlock);
    ret = deleteKRange(cpu_bus_addr,  length);
    MI_SYS_BUG_ON(ret != MI_SUCCESS);

#ifdef MI_SYS_PROC_FS_DEBUG
    if(mma_heap_name)
    {
        memset(&Ops, 0 , sizeof(Ops));
        Ops.OnDumpAllocatorAttr = mi_sys_mma_allocator_proc_dump_attr;
        Ops.OnAllocatorExecCmd = mi_sys_mma_allocator_proc_exec_cmd;
        ret = mi_sys_allocator_proc_create((MI_U8 *)mma_heap_name,&Ops,(void *)pst_mma_allocator);
        MI_SYS_BUG_ON(ret != MI_SUCCESS);
    }
    else
    {
        DBG_INFO("mma_heap_name is NULL,do nothing about proc fs!\n");
    }
#endif

    return pst_mma_allocator;
}

#define MI_SYS_MMA_HEAP_PHY_HASH_BITS 8
static DEFINE_HASHTABLE(g_mi_sys_mma_heap_phy_hash, MI_SYS_MMA_HEAP_PHY_HASH_BITS);
static DEFINE_SEMAPHORE(g_mi_sys_mma_heap_phy_hash_semalock);

static inline void mi_sys_mma_allocator_free_allocation(mi_sys_mma_allocator_t *pst_mma_allocator, mi_sys_mma_buf_allocation_t *pst_mma_allocation)
{
    int ret;
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    MI_SYS_BUG_ON(!pst_mma_allocator);
    MI_SYS_BUG_ON(!pst_mma_allocation);
    MI_SYS_BUG_ON(pst_mma_allocator->u32MagicNumber != __MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    ret = mi_sys_buf_mgr_user_unmap(pst_mma_allocation);
    if(ret != MI_SUCCESS)
    {
        MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);

        //if not have user va, will user unmap fail and that is a normal case.
    }
    down(&pst_mma_allocator->semlock);
    MI_SYS_INFO("%s %d   pst_mma_allocation->offset_in_heap=0x%lx\n",__FUNCTION__,__LINE__,pst_mma_allocation->offset_in_heap);
    mi_sys_free_chunk(&pst_mma_allocator->chunk_mgr, pst_mma_allocation->offset_in_heap);
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    up(&pst_mma_allocator->semlock);

    down(&g_mi_sys_mma_heap_phy_hash_semalock);
    hash_del(&pst_mma_allocation->hentry);
    up(&g_mi_sys_mma_heap_phy_hash_semalock);

    if(pst_mma_allocation->kern_vmap_ptr)
    {
        MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
        mi_sys_buf_mgr_vunmap_kern(pst_mma_allocation->kern_vmap_ptr);
        MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    }

    //release reference count for allocator
    memset(pst_mma_allocation, 0x35, sizeof(*pst_mma_allocation));
    kmem_cache_free(mi_sys_mma_allocation_cachep, pst_mma_allocation);
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    pst_mma_allocator->stdAllocator.ops->OnUnref(&pst_mma_allocator->stdAllocator);
}

MI_SYS_BufferAllocation_t* mi_sys_mma_find_allocation_by_pa(    unsigned long  phy_addr)
{
    mi_sys_mma_buf_allocation_t *pst_mma_allocation;
    unsigned long long cpu_bus_addr = mi_sys_Miu2Cpu_BusAddr(phy_addr);

    down(&g_mi_sys_mma_heap_phy_hash_semalock);
    hash_for_each_possible(g_mi_sys_mma_heap_phy_hash, pst_mma_allocation, hentry, (unsigned long)phy_addr)//here use miu bus addr
    {
        if(pst_mma_allocation->offset_in_heap+pst_mma_allocation->pstMMAAllocator->heap_base_cpu_bus_addr
                                      == cpu_bus_addr)//here use cpu bus addr
        {
            up(&g_mi_sys_mma_heap_phy_hash_semalock);
            //return pst_mma_allocation;
            return &pst_mma_allocation->stdBufAllocation;
        }
    }
    up(&g_mi_sys_mma_heap_phy_hash_semalock);
    DBG_ERR(" fail\n");
    return NULL;
}

MI_SYS_BufferAllocation_t *mi_sys_mma_allocator_alloc(mi_sys_Allocator_t *pstAllocator,
                                       MI_SYS_BufConf_t *pstBufConfig)
{
    mi_sys_mma_buf_allocation_t *pst_mma_allocation;
    mi_sys_mma_allocator_t *pst_mma_allocator;
    unsigned long size_to_alloc;
    unsigned long  long phy_addr;
    unsigned long  offset_in_heap;
    MI_SYS_FrameSpecial_t  stFrameDataInfo;
    int ret;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);

    MI_SYS_BUG_ON(!pstBufConfig);

    size_to_alloc = mi_sys_buf_mgr_get_size(pstBufConfig, &stFrameDataInfo);
    MI_SYS_BUG_ON(0 == size_to_alloc);

    size_to_alloc =ALIGN_UP(size_to_alloc, PAGE_SIZE);

    pst_mma_allocator = container_of(pstAllocator, mi_sys_mma_allocator_t,  stdAllocator);
    down(&pst_mma_allocator->semlock);
    MI_SYS_BUG_ON(pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);

    MI_SYS_INFO("%s %d   size_to_alloc=0x%lx\n",__FUNCTION__,__LINE__,size_to_alloc);
    ret =  mi_sys_alloc_chunk(&pst_mma_allocator->chunk_mgr,  size_to_alloc,&offset_in_heap);
    MI_SYS_INFO("%s %d  offset_in_heap=0x%lx \n",__FUNCTION__,__LINE__,offset_in_heap);
    if(ret != MI_SUCCESS)
    {
        up(&pst_mma_allocator->semlock);
        DBG_ERR(" fail\n");
        return NULL;
    }

    phy_addr = mi_sys_Cpu2Miu_BusAddr(pst_mma_allocator ->heap_base_cpu_bus_addr + offset_in_heap);

    MI_SYS_INFO("%s %d  heap_base_addr=0x%llx ,  phy_addr=0x%llx\n",__FUNCTION__,__LINE__,pst_mma_allocator ->heap_base_cpu_bus_addr,phy_addr);

    pst_mma_allocation = kmem_cache_alloc(mi_sys_mma_allocation_cachep, GFP_KERNEL);
    if(!pst_mma_allocation)
    {
        mi_sys_free_chunk(&pst_mma_allocator->chunk_mgr,  offset_in_heap);
        {
            up(&pst_mma_allocator->semlock);
            DBG_ERR(" fail\n");
            return NULL;
        }
    }

    pst_mma_allocation->offset_in_heap = offset_in_heap;
    pst_mma_allocation->length = size_to_alloc;
    pst_mma_allocation->pstMMAAllocator = pst_mma_allocator ;
    pst_mma_allocation->kern_vmap_ptr = NULL;
    generic_allocation_init(&pst_mma_allocation->stdBufAllocation, &mma_allocation_ops);

    mi_sys_buf_mgr_fill_bufinfo(&pst_mma_allocation->stdBufAllocation.stBufInfo, pstBufConfig, &stFrameDataInfo,phy_addr);
    up(&pst_mma_allocator->semlock);

    down(&g_mi_sys_mma_heap_phy_hash_semalock);
    hash_add(g_mi_sys_mma_heap_phy_hash, &pst_mma_allocation->hentry, (unsigned long long)phy_addr);
    up(&g_mi_sys_mma_heap_phy_hash_semalock);

	if(pstBufConfig->stFrameCfg.stFrameBufExtraConf.bClearPadding && \
        pst_mma_allocation->stdBufAllocation.stBufInfo.eBufType == E_MI_SYS_BUFDATA_FRAME)
    {
        _mi_clear_framebuf_pading_area(&pst_mma_allocation->stdBufAllocation.stBufInfo.stFrameData, &pstBufConfig->stFrameCfg);
    }

    MI_SYS_INFO("%s %d  ,  phy_addr=0x%llx\n",__FUNCTION__,__LINE__,phy_addr);
    //add reference count for allocator
    pstAllocator->ops->OnRef(pstAllocator);
    return &pst_mma_allocation->stdBufAllocation;
}

int mi_sys_mma_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *pstBufConfig)
{
    mi_sys_mma_allocator_t *pst_mma_allocator;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);

    MI_SYS_BUG_ON(!pstBufConfig);

    pst_mma_allocator = container_of(pstAllocator, mi_sys_mma_allocator_t,  stdAllocator);

    MI_SYS_BUG_ON(pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);

    if(pstBufConfig->eBufType == E_MI_SYS_BUFDATA_META)
        return INT_MIN;

    return 100;//MMA max score should less than vb pool.
}

MI_S32 mi_sys_mma_allocator_on_release(mi_sys_Allocator_t *pstAllocator)
{
    int ret;
    mi_sys_mma_allocator_t *pst_mma_allocator;
    mi_sys_mma_allocator_t *tmp_pst_mma_allocator;
    int find_allocator_in_heap_list = 0;
    struct list_head *pos, *q;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);

    pst_mma_allocator = container_of(pstAllocator, mi_sys_mma_allocator_t,  stdAllocator);

    MI_SYS_BUG_ON(pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
    MI_SYS_BUG_ON(atomic_read(&pst_mma_allocator->stdAllocator.ref_cnt));
    MI_SYS_BUG_ON(list_empty(&pst_mma_allocator->list));///must be in g_mma_heap_list

    down(&g_mma_list_semlock);
    list_for_each_safe(pos, q, &g_mma_heap_list)
    {
        tmp_pst_mma_allocator = list_entry(pos, mi_sys_mma_allocator_t, list);
        if(tmp_pst_mma_allocator == pst_mma_allocator)
        {
            list_del(&tmp_pst_mma_allocator->list);
            INIT_LIST_HEAD(&tmp_pst_mma_allocator->list);
            find_allocator_in_heap_list = 1;
            break;
        }
    }

    MI_SYS_BUG_ON(!find_allocator_in_heap_list );
    up(&g_mma_list_semlock);

    down(&pst_mma_allocator->semlock);

#ifdef MI_SYS_PROC_FS_DEBUG
    if(pst_mma_allocator->heap_name[0] != 0)
        mi_sys_allocator_proc_remove_allocator((MI_U8 *)&pst_mma_allocator->heap_name);
#endif

    mi_sys_deinit_chunk(&pst_mma_allocator->chunk_mgr);

    up(&pst_mma_allocator->semlock);

    ret = addKRange(pst_mma_allocator->heap_base_cpu_bus_addr,  pst_mma_allocator->length);
    MI_SYS_BUG_ON(ret != MI_SUCCESS);

    memset(pst_mma_allocator, 0x36, sizeof(*pst_mma_allocator));
    kfree(pst_mma_allocator);
    return MI_SUCCESS;
}
void mi_sys_mma_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation)
{
    mi_sys_mma_buf_allocation_t *pst_mma_allocation;
    mi_sys_mma_allocator_t *pst_mma_allocator;
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    MI_SYS_BUG_ON(!pst_allocation);
    MI_SYS_BUG_ON(atomic_read(&pst_allocation->ref_cnt));

    pst_mma_allocation = container_of(pst_allocation, mi_sys_mma_buf_allocation_t, stdBufAllocation);

    pst_mma_allocator = pst_mma_allocation->pstMMAAllocator;

    MI_SYS_BUG_ON(!pst_mma_allocator || pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    mi_sys_mma_allocator_free_allocation(pst_mma_allocator, pst_mma_allocation);
}
static inline void *mi_sys_mma_allocator_map_usr(mi_sys_mma_allocator_t*pst_mma_allocator,  mi_sys_mma_buf_allocation_t*pst_mma_allocation)
{
    struct sg_table *sg_table;
    int ret;
    unsigned long long cpu_bus_addr;
    unsigned long   length;
    void *user_map_ptr;

    MI_SYS_BUG_ON(!pst_mma_allocator);
    MI_SYS_BUG_ON(!pst_mma_allocation);
    sg_table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
    if (!sg_table)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

    ret = sg_alloc_table(sg_table, 1, GFP_KERNEL);
    if (unlikely(ret))
    {
        DBG_ERR(" fail\n");
        kfree(sg_table);
        return NULL;
    }

    cpu_bus_addr = pst_mma_allocator->heap_base_cpu_bus_addr+pst_mma_allocation->offset_in_heap;
    length = pst_mma_allocation->length;
    MI_SYS_BUG_ON(cpu_bus_addr&~PAGE_MASK || length&~PAGE_MASK);
    sg_set_page(sg_table->sgl, pfn_to_page(__phys_to_pfn(cpu_bus_addr)), PAGE_ALIGN(length), 0);

    ret = mi_sys_buf_mgr_user_map(sg_table, &user_map_ptr,pst_mma_allocation,0);
    if(ret != MI_SUCCESS)
    {
        DBG_WRN("fail,error\n");
        return NULL;
    }
    return user_map_ptr;
}
static inline void mi_sys_mma_allocator_unmap_usr(mi_sys_mma_allocator_t*pst_mma_allocator,  mi_sys_mma_buf_allocation_t*pst_mma_allocation)
{
    MI_SYS_BUG_ON(!pst_mma_allocator);
    MI_SYS_BUG_ON(!pst_mma_allocation);
    MI_SYS_BUG_ON(pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
    mi_sys_buf_mgr_user_unmap(pst_mma_allocation);
}
void *mi_sys_mma_allocation_map_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
    mi_sys_mma_buf_allocation_t *pst_mma_allocation;
    MI_SYS_BUG_ON(!pstAllocation);
    pst_mma_allocation = container_of(pstAllocation, mi_sys_mma_buf_allocation_t, stdBufAllocation);
    MI_SYS_BUG_ON(!pst_mma_allocation->pstMMAAllocator ||pst_mma_allocation->pstMMAAllocator->u32MagicNumber !=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
    return mi_sys_mma_allocator_map_usr(pst_mma_allocation->pstMMAAllocator, pst_mma_allocation);
}
void mi_sys_mma_allocation_unmap_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
    mi_sys_mma_buf_allocation_t *pst_mma_allocation;
    MI_SYS_BUG_ON(!pstAllocation);
    pst_mma_allocation = container_of(pstAllocation, mi_sys_mma_buf_allocation_t, stdBufAllocation);
    MI_SYS_BUG_ON(!pst_mma_allocation->pstMMAAllocator ||pst_mma_allocation->pstMMAAllocator->u32MagicNumber !=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
    mi_sys_mma_allocator_unmap_usr(pst_mma_allocation->pstMMAAllocator, pst_mma_allocation);
}

/*
mma_heap_name :alloc from which heap,if set NULL,means no care in which mma heap
*/
MI_SYS_BufferAllocation_t *mi_sys_alloc_from_mma_allocators( const char *mma_heap_name, MI_SYS_BufConf_t *pstBufConfig)
{
    MI_SYS_BufferAllocation_t *pst_mma_allocation;
    mi_sys_mma_allocator_t *pst_mma_allocator;
    struct list_head *pos, *q;
    MI_BOOL check_exist = FALSE;
    //MI_SYS_BUG_ON(!mma_heap_name);
    MI_SYS_BUG_ON(!pstBufConfig);

    //validity check for parameter mma_heap_name.
    if(mma_heap_name != NULL && mma_heap_name[0] != 0)
    {
        down(&g_mma_list_semlock);
        list_for_each_safe(pos, q, &g_mma_heap_list)
        {
            pst_mma_allocator = list_entry(pos, mi_sys_mma_allocator_t, list);
            if(0==strncmp(pst_mma_allocator->heap_name,mma_heap_name,max(strlen(mma_heap_name),strlen(pst_mma_allocator->heap_name))))
            {
                check_exist = TRUE;
                break;
            }
        }
        up(&g_mma_list_semlock);
        if(FALSE == check_exist)
        {
            DBG_ERR("Invalid mma_heap_name %s ,  please input real mma heap name or NULL !!!!\n",mma_heap_name);
            DBG_ERR("You can get real mma heap name from cat /proc/cmdline .\n");
            return NULL;
        }
    }

    down(&g_mma_list_semlock);
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    list_for_each_safe(pos, q, &g_mma_heap_list)
    {
        MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
        pst_mma_allocator = list_entry(pos, mi_sys_mma_allocator_t, list);
        if(!mma_heap_name||mma_heap_name[0]==0)//no  cache alloc in which mma heap
        {
            MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
            pst_mma_allocation =mi_sys_mma_allocator_alloc(&pst_mma_allocator->stdAllocator, pstBufConfig);

            if(!pst_mma_allocation)
            {
                continue;
            }
            else
            {
                up(&g_mma_list_semlock);
                MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
                return pst_mma_allocation;//success
            }
        }
        else
        {
            MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
            if(0==strncmp(pst_mma_allocator->heap_name,mma_heap_name,max(strlen(mma_heap_name),strlen(pst_mma_allocator->heap_name))))
            {
                MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
                pst_mma_allocation =mi_sys_mma_allocator_alloc(&pst_mma_allocator->stdAllocator, pstBufConfig);
                up(&g_mma_list_semlock);
                MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
                return pst_mma_allocation;
            }
            MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
        }
    }
    up(&g_mma_list_semlock);
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    return NULL;
}

static inline void *mi_sys_mma_allocator_vmap_kern(mi_sys_mma_allocator_t*pst_mma_allocator,  mi_sys_mma_buf_allocation_t*pst_mma_allocation)
{
    struct sg_table *sg_table;
    int ret;
    unsigned long long start_addr;
    unsigned long   length;
    void *kern_vmap_ptr;

    MI_SYS_BUG_ON(!pst_mma_allocator);
    MI_SYS_BUG_ON(!pst_mma_allocation);
    if(pst_mma_allocation->kern_vmap_ptr)
        return pst_mma_allocation->kern_vmap_ptr;

    sg_table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
    if (!sg_table) {
        DBG_ERR(" fail\n");
        return NULL;
    }

    ret = sg_alloc_table(sg_table, 1, GFP_KERNEL);
    if (unlikely(ret))
    {
        DBG_ERR(" fail\n");
        kfree(sg_table);
        return NULL;
    }

    start_addr = pst_mma_allocator->heap_base_cpu_bus_addr+pst_mma_allocation->offset_in_heap;
    length = pst_mma_allocation->length;

    MI_SYS_BUG_ON(start_addr&~PAGE_MASK || length&~PAGE_MASK);

    sg_set_page(sg_table->sgl, pfn_to_page(__phys_to_pfn(start_addr)), PAGE_ALIGN(length), 0);

    ret = mi_sys_buf_mgr_vmap_kern(sg_table, &kern_vmap_ptr);
    sg_free_table(sg_table);
    kfree(sg_table);
    if(ret != MI_SUCCESS)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }
    pst_mma_allocation->kern_vmap_ptr = kern_vmap_ptr;

    MI_SYS_BUG_ON(!kern_vmap_ptr);

    return kern_vmap_ptr;
}

void *mi_sys_mma_allocation_vmap_kern(MI_SYS_BufferAllocation_t *pstAllocation)
{
    mi_sys_mma_buf_allocation_t *pst_mma_allocation;
    MI_SYS_BUG_ON(!pstAllocation);
    pst_mma_allocation = container_of(pstAllocation, mi_sys_mma_buf_allocation_t, stdBufAllocation);
    MI_SYS_BUG_ON(!pst_mma_allocation->pstMMAAllocator ||pst_mma_allocation->pstMMAAllocator->u32MagicNumber !=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
    return mi_sys_mma_allocator_vmap_kern(pst_mma_allocation->pstMMAAllocator , pst_mma_allocation);
}
void mi_sys_mma_allocation_vunmap_kern( MI_SYS_BufferAllocation_t *pstAllocation)
{
    MI_SYS_BUG_ON(!pstAllocation);
    MI_SYS_BUG_ON(pstAllocation->ops.vunmap_kern != mma_allocation_ops.vunmap_kern);
    //do nothing
}

#ifdef MI_SYS_PROC_FS_DEBUG

static inline void *mi_sys_mma_allocator_vmap_kern_offset_length(mi_sys_mma_allocator_t*pst_mma_allocator,  MI_U32 offset, MI_U32 length)
{
    struct sg_table *sg_table;
    int ret;
    unsigned long long start_addr;
    void *kern_vmap_ptr;

    MI_SYS_BUG_ON(!pst_mma_allocator);

    sg_table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
    if (!sg_table) {
        DBG_ERR(" fail\n");
        return NULL;
    }

    ret = sg_alloc_table(sg_table, 1, GFP_KERNEL);
    if (unlikely(ret))
    {
        DBG_ERR(" fail\n");
        kfree(sg_table);
        return NULL;
    }

    start_addr = pst_mma_allocator->heap_base_cpu_bus_addr+offset;

    MI_SYS_BUG_ON(start_addr&~PAGE_MASK || length&~PAGE_MASK);

    sg_set_page(sg_table->sgl, pfn_to_page(__phys_to_pfn(start_addr)), PAGE_ALIGN(length), 0);

    ret = mi_sys_buf_mgr_vmap_kern(sg_table, &kern_vmap_ptr);
    sg_free_table(sg_table);
    kfree(sg_table);
    if(ret != MI_SUCCESS)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }
    MI_SYS_BUG_ON(!kern_vmap_ptr);

    return kern_vmap_ptr;
}

static inline int mi_sys_mma_allocator_vunmap_kern_offset_length(void *kern_vmap_ptr)

{
    return mi_sys_buf_mgr_vunmap_kern(kern_vmap_ptr);
}
MI_S32 mi_sys_mma_allocator_proc_dump_attr(MI_SYS_DEBUG_HANDLE_t  handle,void *private)
{
    mi_sys_mma_allocator_t *target_pst_mma_allocator = (mi_sys_mma_allocator_t *)private;
    mi_sys_mma_allocator_t * pst_mma_allocator =NULL;
    struct list_head *pos;
    //printk("in %s:%d \n",__FUNCTION__,__LINE__);

    down(&g_mma_list_semlock);
    handle.OnPrintOut(handle,"%30s%30s%30s%30s\n", "mma heap name", "heap_base_cpu_bus_addr"
                             ,"length","chunk_mgr_avail");
    list_for_each(pos, &g_mma_heap_list)
    {
        pst_mma_allocator = list_entry(pos, mi_sys_mma_allocator_t, list);
        MI_SYS_BUG_ON(!pst_mma_allocator);
        if(pst_mma_allocator == target_pst_mma_allocator)
        {
            //Base seq_file to seq_printf.
            //To printf ,do not use normal file and sprintf!!!
            handle.OnPrintOut(handle,"%30s%30llx%30lx%30lx\n", (char *)pst_mma_allocator->heap_name
                          , pst_mma_allocator->heap_base_cpu_bus_addr,pst_mma_allocator->length,pst_mma_allocator->chunk_mgr.avail);

            down(&pst_mma_allocator->semlock);
            dump_chunk_mgr(handle, &pst_mma_allocator->chunk_mgr);
            up(&pst_mma_allocator->semlock);

            break;//find ,so exit
        }
    }
    up(&g_mma_list_semlock);
    return MI_SUCCESS;
}


MI_S32 mi_sys_mma_allocator_proc_exec_cmd(MI_SYS_DEBUG_HANDLE_t  handle,allocator_echo_cmd_info_t *cmd_info,void *private)
{
    mi_sys_mma_allocator_t *pst_mma_allocator = (mi_sys_mma_allocator_t *)private;
    mm_segment_t old_fs;
    mi_sys_chunk_t *chunk;
    struct file *fp;
    unsigned long long chunk_cpu_bus_addr;
    MI_PHY chunk_miu_bus_addr;
    void *pVirAddr = NULL;
    MI_U32 target_offset = cmd_info->offset;
    MI_U32 target_length = cmd_info->length;
    MI_S32 ret = MI_SUCCESS;

    if((cmd_info->offset >= pst_mma_allocator->length)
        || (cmd_info->offset + cmd_info->length > pst_mma_allocator->length))
    {
        DBG_ERR("invalid offset or length,offset=%u,length=%u   max_offset=%u\n",cmd_info->offset,cmd_info->length,pst_mma_allocator->length);
        return E_MI_ERR_FAILED;
    }

    if(((pst_mma_allocator->heap_base_cpu_bus_addr+cmd_info->offset) &~PAGE_MASK)
        ||(cmd_info->length &~PAGE_MASK))
    {
        DBG_ERR("fail,error!Not allowed vmap kern,both start addr 0x%llx and length  %x should  be  0x%xBytes aligned!!!\n"
                  ,pst_mma_allocator->heap_base_cpu_bus_addr+cmd_info->offset
                  ,cmd_info->length
                  ,PAGE_SIZE);

        return E_MI_ERR_FAILED;
    }

    if(cmd_info->dir_name[cmd_info->dir_size -1] == '/')
        sprintf(cmd_info->dir_name+cmd_info->dir_size,"mma__%s__%u__%u.bin",pst_mma_allocator->heap_name,cmd_info->offset,cmd_info->length);
    else
        sprintf(cmd_info->dir_name+cmd_info->dir_size,"/mma__%s__%u__%u.bin",pst_mma_allocator->heap_name,cmd_info->offset,cmd_info->length);

    printk("%s %d  dir_and_file_name  is %s\n",__FUNCTION__,__LINE__,cmd_info->dir_name);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open((char *)(&cmd_info->dir_name[0]),O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if(IS_ERR(fp))
    {
        DBG_ERR("filp_open fail   PTR_ERR_fp = %d\n",PTR_ERR(fp));//here use PTR_ERR(fp) to show errno
        set_fs(old_fs);
        return E_MI_ERR_FAILED;
    }

    down(&pst_mma_allocator->semlock);
    down(&pst_mma_allocator->chunk_mgr.semlock);

    pVirAddr = mi_sys_mma_allocator_vmap_kern_offset_length(pst_mma_allocator,cmd_info->offset,cmd_info->length);
    if(!pVirAddr)
    {
        DBG_ERR("mi_sys_mma_allocator_vmap_kern_offset_length fail\n");
        up(&pst_mma_allocator->chunk_mgr.semlock);
        up(&pst_mma_allocator->semlock);
        filp_close(fp, NULL);
        set_fs(old_fs);

        return E_MI_ERR_FAILED;
    }

    chunk = pst_mma_allocator->chunk_mgr.chunks;

    while(chunk)
    {
        chunk_cpu_bus_addr = chunk->offset + pst_mma_allocator->heap_base_cpu_bus_addr;
        chunk_miu_bus_addr = mi_sys_Cpu2Miu_BusAddr(chunk_cpu_bus_addr);

        if(target_offset >= chunk->offset  &&
            (target_offset <= chunk->offset + chunk->length))
        {
            if(target_offset+target_length <= chunk->offset + chunk->length)
            {

                //dump data in [target_offset,target_offset + target_length] for this mma allocator.
                ret = vfs_write(fp, (char *)pVirAddr+(target_offset-cmd_info->offset), target_length, &fp->f_pos);

                mi_sys_mma_allocator_vunmap_kern_offset_length(pVirAddr);
                up(&pst_mma_allocator->chunk_mgr.semlock);
                up(&pst_mma_allocator->semlock);
                set_fs(old_fs);
                if(ret != target_length)
                {
                    DBG_ERR("vfs_write fail\n");
                    filp_close(fp, NULL);
                    return E_MI_ERR_FAILED;
                }
                else
                {
                    BUG_ON(cmd_info->length != fp->f_pos);
                    filp_close(fp, NULL);
                    return MI_SUCCESS;//done,so return.
                }
            }
            else
            {
                //dump data in [target_offset, chunk->length] for this mma allocator.
                ret = vfs_write(fp, (char *)pVirAddr+(target_offset-cmd_info->offset), chunk->offset +chunk->length - target_offset, &fp->f_pos);

                if(ret != chunk->offset +chunk->length - target_offset)
                {
                    DBG_ERR("vfs_write fail\n");
                    mi_sys_mma_allocator_vunmap_kern_offset_length(pVirAddr);
                    up(&pst_mma_allocator->chunk_mgr.semlock);
                    up(&pst_mma_allocator->semlock);
                    filp_close(fp, NULL);
                    set_fs(old_fs);
                    return E_MI_ERR_FAILED;
                }

                target_length -= chunk->offset +chunk->length - target_offset;
                target_offset = chunk->offset +chunk->length;
            }
        }

        chunk = chunk->next;

    }
    mi_sys_mma_allocator_vunmap_kern_offset_length(pVirAddr);
    up(&pst_mma_allocator->chunk_mgr.semlock);
    up(&pst_mma_allocator->semlock);
    filp_close(fp, NULL);
    set_fs(old_fs);
    DBG_ERR("error,fail,should not return from here\n");
    return E_MI_ERR_FAILED;
}

static int mma_heap_info_show(struct seq_file *m, void *v)
{
    int len = 0;
    mi_sys_mma_allocator_t * pmma_allocator =NULL;
    struct list_head *pos;

    down(&g_mma_list_semlock);
    list_for_each(pos, &g_mma_heap_list)
    {
        pmma_allocator = list_entry(pos, mi_sys_mma_allocator_t, list);
        MI_SYS_BUG_ON(!pmma_allocator);

        //Base seq_file to seq_printf.
        //To printf ,do not use normal file and sprintf!!!
        seq_printf(m, "mma heap name=%s   heap_base_cpu_bus_addr=0x%llx    length=0x%lx\n", (char *)pmma_allocator->heap_name
                          , pmma_allocator->heap_base_cpu_bus_addr,pmma_allocator->length);
    }
    up(&g_mma_list_semlock);

    return len;
}

int mma_heap_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, mma_heap_info_show, NULL);
}

#endif

