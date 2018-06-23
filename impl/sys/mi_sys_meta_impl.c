#include "mi_sys_meta_impl.h"
#include <linux/list.h>
#include "mi_sys_debug.h"
#include <linux/fs.h>
#include <asm/uaccess.h>

struct kmem_cache *mi_sys_meta_allocation_cachep;

MI_S32 mi_sys_meta_allocator_on_release(struct mi_sys_Allocator_s *pstAllocator);
MI_SYS_BufferAllocation_t *mi_sys_meta_allocator_alloc(mi_sys_Allocator_t *pstAllocator,  MI_SYS_BufConf_t *pstBufConfig);
int mi_sys_meta_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *pstBufConfig);
void mi_sys_meta_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation);
void *mi_sys_meta_allocation_map_user(MI_SYS_BufferAllocation_t *pstAllocation);
void mi_sys_meta_allocation_unmap_user(MI_SYS_BufferAllocation_t *pstAllocation);
void *mi_sys_meta_allocation_on_map_kern(MI_SYS_BufferAllocation_t *pstAllocation);
void mi_sys_meta_allocation_on_unmap_kern(MI_SYS_BufferAllocation_t *pstAllocation);

buf_allocator_ops_t meta_allocator_ops =
{
    .OnRef = generic_allocator_on_ref,
    .OnUnref = generic_allocator_on_unref,
    .OnRelease = mi_sys_meta_allocator_on_release,
    .alloc = mi_sys_meta_allocator_alloc,
    .suit_bufconfig = mi_sys_meta_allocator_suit_bufconfig
};

buf_allocation_ops_t meta_allocation_ops =
{
    .OnRef = generic_allocation_on_ref,
    .OnUnref = generic_allocation_on_unref,
    .OnRelease = mi_sys_meta_allocation_on_release,
    .map_user = mi_sys_meta_allocation_map_user,
    .unmap_user = mi_sys_meta_allocation_unmap_user,
    .vmap_kern =  mi_sys_meta_allocation_on_map_kern,//do not use generic realize,use it self's which do nothing
    .vunmap_kern = mi_sys_meta_allocation_on_unmap_kern    //do not use generic realize,use it self's which do nothing
};

#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 mi_sys_meta_allocator_proc_dump_attr(MI_SYS_DEBUG_HANDLE_t  handle,void *private)
{
    mi_sys_meta_allocator_t *pst_meta_allocator;
    mi_sys_Allocator_t *pstAllocator;
    struct list_head *pos;
    struct list_head *pos_2;
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    unsigned int total_free_allocation = 0;

    pst_meta_allocator = (mi_sys_meta_allocator_t *)private;

    handle.OnPrintOut(handle,"===================  start meta_info  ================================\n");
    pstAllocator = mi_sys_get_global_meta_allocator();
    pst_meta_allocator = container_of(pstAllocator, mi_sys_meta_allocator_t,  stdAllocator);

    down(&pst_meta_allocator->semlock);
    total_free_allocation = 0;
    list_for_each(pos_2,&pst_meta_allocator->list_of_free_allocations)
    {
        pst_meta_allocation = container_of(pos_2, mi_sys_meta_buf_allocation_t, list_in_free_list);
        MI_SYS_BUG_ON(!pst_meta_allocation);
        total_free_allocation++;
    }
    handle.OnPrintOut(handle,"basic info:\n");
    //some pages may part used,part freed. So calc total free data size,not calc total free page count.
    handle.OnPrintOut(handle,"total page count:%d        each meta data size:%d    total_count_with_metadatasize  %d    total_free_count_with_metadatasize:%u\n"
                                      ,(int)MI_SYS_META_DATA_PAGE_COUNT,(int)MI_SYS_META_DATA_SIZE_MAX
                                      ,(int)(MI_SYS_META_DATA_PAGE_COUNT*PAGE_SIZE/MI_SYS_META_DATA_SIZE_MAX),total_free_allocation);
    up(&pst_meta_allocator->semlock);

    handle.OnPrintOut(handle,"meta info :\n");
    handle.OnPrintOut(handle,"ref_cnt=%d  \n",atomic_read(&pstAllocator->ref_cnt));
    down(&pst_meta_allocator->semlock);
    handle.OnPrintOut(handle,"each allocation info:\n");
    handle.OnPrintOut(handle,"offset_in_pool   length    phy_addr       va_in_kern        real_used_flag\n");
    list_for_each(pos,&pst_meta_allocator->list_of_all_allocations)
    {
        pst_meta_allocation = container_of(pos, mi_sys_meta_buf_allocation_t, list_in_all_list);
        handle.OnPrintOut(handle,"0x%lx        0x%lx         0x%llx      %p             ",pst_meta_allocation->offset_in_pool,pst_meta_allocation->length,pst_meta_allocation->phy_addr,pst_meta_allocation->va_in_kern);
        if(list_empty(&pst_meta_allocation->list_in_free_list))
        {
            handle.OnPrintOut(handle,"1\n");//used
        }
        else
        {
            handle.OnPrintOut(handle,"0\n");//not used.
        }
    }
    up(&pst_meta_allocator->semlock);
    handle.OnPrintOut(handle,"===================  end meta_info  ================================\n");
    return MI_SUCCESS;

}

MI_S32 mi_sys_meta_allocator_proc_exec_cmd(MI_SYS_DEBUG_HANDLE_t  handle,allocator_echo_cmd_info_t *cmd_info,void *private)
{
    mi_sys_meta_allocator_t *pst_meta_allocator;
    struct list_head *pos;
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    unsigned long curr_offset;
    MI_U32 target_offset = cmd_info->offset;
    MI_U32 target_length = cmd_info->length;
    struct file *fp;
    mm_segment_t old_fs;
    MI_S32 ret;

    pst_meta_allocator = (mi_sys_meta_allocator_t *)private;

    if((cmd_info->offset >= MI_SYS_META_DATA_PAGE_COUNT*PAGE_SIZE)
        || (cmd_info->offset + cmd_info->length > MI_SYS_META_DATA_PAGE_COUNT*PAGE_SIZE))
    {
        DBG_ERR("invalid offset or length,offset=%u,length=%u   max_offset=%u\n",cmd_info->offset,cmd_info->length,MI_SYS_META_DATA_PAGE_COUNT*PAGE_SIZE);
        return E_MI_ERR_FAILED;
    }

    if(cmd_info->dir_name[cmd_info->dir_size -1] == '/')
        sprintf(cmd_info->dir_name+cmd_info->dir_size,"meta__%u__%u.bin",cmd_info->offset,cmd_info->length);
    else
        sprintf(cmd_info->dir_name+cmd_info->dir_size,"/meta__%u__%u.bin",cmd_info->offset,cmd_info->length);

    printk("dir_and_file_name  is %s\n",cmd_info->dir_name);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open((char *)(&cmd_info->dir_name[0]),O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if(IS_ERR(fp))
    {
        set_fs(old_fs);
        return E_MI_ERR_FAILED;
    }

    down(&pst_meta_allocator->semlock);
    curr_offset = 0;
    list_for_each(pos,&pst_meta_allocator->list_of_all_allocations)
    {
        pst_meta_allocation = container_of(pos, mi_sys_meta_buf_allocation_t, list_in_all_list);
        BUG_ON(curr_offset != pst_meta_allocation->offset_in_pool);
        if(target_offset >= pst_meta_allocation->offset_in_pool && target_offset <= pst_meta_allocation->offset_in_pool + pst_meta_allocation->length)
        {
           if(target_offset + target_length <= pst_meta_allocation->offset_in_pool + pst_meta_allocation->length)
           {
               //dump data in [target_offset,target_offset+target_length] of this meta allocator
               ret = vfs_write(fp,(char *)pst_meta_allocation->va_in_kern + (target_offset-pst_meta_allocation->offset_in_pool),target_length,&fp->f_pos);
               up(&pst_meta_allocator->semlock);
               set_fs(old_fs);

               if(ret != target_length)
               {
                   DBG_ERR("fail   ret=0x%x, length=0x%x\n",ret,target_length);
                   filp_close(fp, NULL);
                   return E_MI_ERR_FAILED;
               }
               else
               {
                   BUG_ON(cmd_info->length != fp->f_pos);
                   filp_close(fp, NULL);
                   return MI_SUCCESS;//finish ,return!
               }
           }
           else
           {
              //dump data in [target_offset,pst_meta_allocation->offset_in_pool + pst_meta_allocation->length-target_offset] of this meta allocator
              ret = vfs_write(fp,(char *)pst_meta_allocation->va_in_kern + (target_offset-pst_meta_allocation->offset_in_pool),pst_meta_allocation->offset_in_pool + pst_meta_allocation->length-target_offset,&fp->f_pos);

              if(ret != pst_meta_allocation->offset_in_pool + pst_meta_allocation->length-target_offset)
              {
                  DBG_ERR("fail   ret=0x%x, this length=0x%x\n",ret,pst_meta_allocation->offset_in_pool + pst_meta_allocation->length-target_offset);
                  up(&pst_meta_allocator->semlock);
                  set_fs(old_fs);
                  filp_close(fp, NULL);
                  return E_MI_ERR_FAILED;
              }

              //after dump,change target value.
              target_length -= pst_meta_allocation->offset_in_pool + pst_meta_allocation->length-target_offset;
              target_offset = pst_meta_allocation->offset_in_pool + pst_meta_allocation->length;
           }
        }

        curr_offset += pst_meta_allocation->length;
    }
    up(&pst_meta_allocator->semlock);
    set_fs(old_fs);
    filp_close(fp, NULL);

    DBG_ERR("error,fail,should not return from here\n");

    return E_MI_ERR_FAILED;


}

#endif

///allocator_create的时候实际的buffer就已经alloc好了
mi_sys_Allocator_t *mi_sys_meta_allocator_create(void)
{
    int ret;
    mi_sys_meta_allocator_t *pst_meta_allocator;
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    struct sg_table *sg_table;
    struct scatterlist *sg;
    int allocated_page_count = 0;
    unsigned long offset_in_pool;
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_AllocatorProcfsOps_t Ops;
#endif
    MI_SYS_INFO("%s:%d \n",__FUNCTION__,__LINE__);
    pst_meta_allocator = (mi_sys_meta_allocator_t*)kmalloc(sizeof(mi_sys_meta_allocator_t),GFP_KERNEL);
    if(pst_meta_allocator  == NULL)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

    generic_allocator_init(&pst_meta_allocator->stdAllocator, &meta_allocator_ops);
    pst_meta_allocator->u32MagicNumber = __MI_SYS_META_ALLOCATOR_MAGIC_NUM__;

    INIT_LIST_HEAD(&pst_meta_allocator->list_of_all_allocations);
    INIT_LIST_HEAD(&pst_meta_allocator->list_of_free_allocations);
    sema_init(&pst_meta_allocator->semlock, 1);

    sg_table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
    if (!sg_table) {
        DBG_ERR(" fail\n");
        kfree(pst_meta_allocator);
        return NULL;
    }

    ret = sg_alloc_table(sg_table, MI_SYS_META_DATA_PAGE_COUNT, GFP_KERNEL);
    if (unlikely(ret))
    {
        DBG_ERR(" fail\n");
        kfree(pst_meta_allocator);
        kfree(sg_table);
        return NULL;
    }

    pst_meta_allocator->sg_table = sg_table;
    sg=sg_table->sgl;

    allocated_page_count = 0;
    offset_in_pool = 0;
    while(allocated_page_count < MI_SYS_META_DATA_PAGE_COUNT)
    {
        struct page *page = alloc_page(GFP_KERNEL);
        int i;
        if(!page)
        {
            DBG_ERR(" fail\n");
            goto failed_n_free;
        }
        pst_meta_allocator->pages[allocated_page_count++] = page;
        sg_set_page(sg, page, PAGE_SIZE, 0);
        sg = sg_next(sg);
        for( i=0; i<PAGE_SIZE/MI_SYS_META_DATA_SIZE_MAX; i++)
        {
            pst_meta_allocation = (mi_sys_meta_buf_allocation_t*)kmem_cache_alloc(mi_sys_meta_allocation_cachep, GFP_KERNEL);
            if(!pst_meta_allocation)
            {
                DBG_ERR(" fail\n");
                goto failed_n_free;
            }
            pst_meta_allocation->length = MI_SYS_META_DATA_SIZE_MAX;
            pst_meta_allocation->phy_addr = mi_sys_Cpu2Miu_BusAddr(__pfn_to_phys(__page_to_pfn(page)))+MI_SYS_META_DATA_SIZE_MAX*i;
            pst_meta_allocation->pstmetaAllocator = pst_meta_allocator;
            pst_meta_allocation->va_in_kern = (void*)((char*)page_address(page)+MI_SYS_META_DATA_SIZE_MAX*i);
            pst_meta_allocation->offset_in_pool = offset_in_pool;
            offset_in_pool+=MI_SYS_META_DATA_SIZE_MAX;
            //generic_allocation_init(&pst_meta_allocation->stdBufAllocation, &meta_allocation_ops);
            list_add_tail(&pst_meta_allocation->list_in_all_list, &pst_meta_allocator->list_of_all_allocations);
            list_add_tail(&pst_meta_allocation->list_in_free_list, &pst_meta_allocator->list_of_free_allocations);
        }
    }

#ifdef MI_SYS_PROC_FS_DEBUG
    memset(&Ops, 0 , sizeof(Ops));
    Ops.OnDumpAllocatorAttr = mi_sys_meta_allocator_proc_dump_attr;
    Ops.OnAllocatorExecCmd = mi_sys_meta_allocator_proc_exec_cmd;
    ret = mi_sys_allocator_proc_create("meta",&Ops,(void *)pst_meta_allocator);
    MI_SYS_BUG_ON(ret != MI_SUCCESS);
#endif

    return &pst_meta_allocator->stdAllocator;

failed_n_free:

    while(allocated_page_count)
    {
        free_page((unsigned long)pst_meta_allocator->pages[--allocated_page_count]);
    }
    while(!list_empty( &pst_meta_allocator->list_of_all_allocations))
    {
        pst_meta_allocation  = container_of(pst_meta_allocator->list_of_all_allocations.next, mi_sys_meta_buf_allocation_t, list_in_all_list);
        list_del(&pst_meta_allocation->list_in_all_list);
        kmem_cache_free(mi_sys_meta_allocation_cachep, pst_meta_allocation);
    }
    sg_free_table(pst_meta_allocator->sg_table);
    kfree(pst_meta_allocator->sg_table);
    kfree(pst_meta_allocator);
    DBG_ERR(" fail\n");

    return NULL;
}

///把free的buffer 还给pst_meta_allocator，不真正的释放
static inline void mi_sys_meta_allocator_free(mi_sys_meta_allocator_t *pst_meta_allocator, mi_sys_meta_buf_allocation_t *pst_meta_allocation)
{
    MI_SYS_BUG_ON(!pst_meta_allocator);
    MI_SYS_BUG_ON(!pst_meta_allocation);
    MI_SYS_BUG_ON(pst_meta_allocator->u32MagicNumber != __MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
    down(&pst_meta_allocator->semlock);
    list_add_tail(&pst_meta_allocation->list_in_free_list, &pst_meta_allocator->list_of_free_allocations);
    up(&pst_meta_allocator->semlock);

    //release reference to allocator
    pst_meta_allocator->stdAllocator.ops->OnUnref(&pst_meta_allocator->stdAllocator);
}

///allocator_alloc时直接拿之前 allocator_create已经alloc好了的buffer
MI_SYS_BufferAllocation_t *mi_sys_meta_allocator_alloc(mi_sys_Allocator_t *pstAllocator,  MI_SYS_BufConf_t *pstBufConfig)
{
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    mi_sys_meta_allocator_t *pst_meta_allocator;
    MI_SYS_INFO("#s %d \n",__FUNCTION__,__LINE__);
    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufConfig);

    pst_meta_allocator = container_of(pstAllocator, mi_sys_meta_allocator_t,  stdAllocator);
    MI_SYS_BUG_ON(pst_meta_allocator->u32MagicNumber!=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);

    if(pstBufConfig->eBufType != E_MI_SYS_BUFDATA_META)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

    if(pstBufConfig->stMetaCfg.u32Size > MI_SYS_META_DATA_SIZE_MAX)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

    down(&pst_meta_allocator->semlock);

    if(list_empty(&pst_meta_allocator->list_of_free_allocations))
    {
        pst_meta_allocation = NULL;
    }
    else
    {
        pst_meta_allocation = container_of(pst_meta_allocator->list_of_free_allocations.next, mi_sys_meta_buf_allocation_t, list_in_free_list);
        list_del(&pst_meta_allocation->list_in_free_list);
        INIT_LIST_HEAD(&pst_meta_allocation->list_in_free_list);
    }
    up(&pst_meta_allocator->semlock);

    if(pst_meta_allocation == NULL )
    {
        DBG_ERR(" fail\n");
        return NULL;
    }
    MI_SYS_INFO("#s %d will init meta_allocation_ops\n",__FUNCTION__,__LINE__);
    generic_allocation_init(&pst_meta_allocation->stdBufAllocation, &meta_allocation_ops);

    mi_sys_buf_mgr_fill_bufinfo(&pst_meta_allocation->stdBufAllocation.stBufInfo, pstBufConfig, NULL,pst_meta_allocation->phy_addr);//for meta data,no care third parameter "MI_SYS_FrameSpecial_t *pstFrameDataInfo"
    MI_SYS_BUG_ON(atomic_read(&pst_meta_allocation->stdBufAllocation.ref_cnt) != 0);

    //add reference counter of allocator for pstAllocator lifecycle management
    pstAllocator->ops->OnRef(pstAllocator);

    return &pst_meta_allocation->stdBufAllocation;
}
int mi_sys_meta_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *pstBufConfig)
{
    mi_sys_meta_allocator_t *pst_meta_allocator;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);
    MI_SYS_BUG_ON(!pstBufConfig);

    pst_meta_allocator = container_of(pstAllocator, mi_sys_meta_allocator_t,  stdAllocator);

    MI_SYS_BUG_ON(pst_meta_allocator->u32MagicNumber!=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
    if(pstBufConfig->eBufType  != E_MI_SYS_BUFDATA_META)
        return INT_MIN;

    if(pstBufConfig->stMetaCfg.u32Size > MI_SYS_META_DATA_SIZE_MAX)
        return INT_MIN;

    return 10000;// meta > vb pool > mma
}

///each page will free_page
MI_S32 mi_sys_meta_allocator_on_release(struct mi_sys_Allocator_s *pstAllocator)
{
    mi_sys_meta_allocator_t *pst_meta_allocator;
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    int allocated_page_count;

    MI_SYS_BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);
    pst_meta_allocator = container_of(pstAllocator, mi_sys_meta_allocator_t,  stdAllocator);
    MI_SYS_BUG_ON(!pst_meta_allocator || pst_meta_allocator->u32MagicNumber != __MI_SYS_META_ALLOCATOR_MAGIC_NUM__);

    down(&pst_meta_allocator->semlock);

    MI_SYS_BUG_ON(atomic_read(&pst_meta_allocator->stdAllocator.ref_cnt));

    mi_sys_buf_mgr_user_unmap(pst_meta_allocator);

    while(!list_empty( &pst_meta_allocator->list_of_all_allocations))
    {
        pst_meta_allocation  = container_of(pst_meta_allocator->list_of_all_allocations.next, mi_sys_meta_buf_allocation_t, list_in_all_list);

        MI_SYS_BUG_ON(list_empty(&pst_meta_allocation->list_in_free_list));

        list_del(&pst_meta_allocation->list_in_all_list);
        list_del(&pst_meta_allocation->list_in_free_list);

        memset(pst_meta_allocation, 0x33, sizeof(*pst_meta_allocation));
        kmem_cache_free(mi_sys_meta_allocation_cachep, pst_meta_allocation);
    }
    MI_SYS_BUG_ON(!list_empty( &pst_meta_allocator->list_of_free_allocations));

    allocated_page_count = MI_SYS_META_DATA_PAGE_COUNT;
    while(allocated_page_count)
    {
        free_page((unsigned long)pst_meta_allocator->pages[--allocated_page_count]);
    }

    sg_free_table(pst_meta_allocator->sg_table);
    kfree(pst_meta_allocator->sg_table);
    up(&pst_meta_allocator->semlock);
    memset(pst_meta_allocator, 0x34, sizeof(*pst_meta_allocator));

    kfree(pst_meta_allocator);
    return MI_SUCCESS;
}
void mi_sys_meta_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation)
{
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    mi_sys_meta_allocator_t *pst_meta_allocator;

    MI_SYS_BUG_ON(!pst_allocation);
    MI_SYS_BUG_ON(atomic_read(&pst_allocation->ref_cnt));

    pst_meta_allocation = container_of(pst_allocation, mi_sys_meta_buf_allocation_t, stdBufAllocation);

    pst_meta_allocator = pst_meta_allocation->pstmetaAllocator;

    MI_SYS_BUG_ON(!pst_meta_allocator || pst_meta_allocator->u32MagicNumber!=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);

    mi_sys_meta_allocator_free(pst_meta_allocator, pst_meta_allocation);
}
static inline void *mi_sys_meta_allocator_map_usr(mi_sys_meta_allocator_t*pst_meta_allocator,  mi_sys_meta_buf_allocation_t*pst_meta_allocation)
{
    int ret;
    void *user_map_ptr;
    MI_SYS_BUG_ON(!pst_meta_allocator);
    MI_SYS_BUG_ON(!pst_meta_allocation);

    down(&pst_meta_allocator->semlock);
    ret = mi_sys_buf_mgr_user_map(pst_meta_allocator->sg_table, &user_map_ptr, pst_meta_allocator,1);
    up(&pst_meta_allocator->semlock);
    if(ret != MI_SUCCESS)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

    return (void*)((char *)user_map_ptr+pst_meta_allocation->offset_in_pool);
}
static inline void mi_sys_meta_allocator_unmap_usr(mi_sys_meta_allocator_t*pst_meta_allocator,  mi_sys_meta_buf_allocation_t*pst_meta_allocation)
{
    MI_SYS_BUG_ON(!pst_meta_allocator);
    MI_SYS_BUG_ON(!pst_meta_allocation);
    MI_SYS_BUG_ON(pst_meta_allocator->u32MagicNumber!=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
    //do nothing
}
void *mi_sys_meta_allocation_map_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    MI_SYS_BUG_ON(!pstAllocation);
    pst_meta_allocation = container_of(pstAllocation, mi_sys_meta_buf_allocation_t, stdBufAllocation);
    MI_SYS_BUG_ON(!pst_meta_allocation->pstmetaAllocator || pst_meta_allocation->pstmetaAllocator->u32MagicNumber !=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
    return mi_sys_meta_allocator_map_usr(pst_meta_allocation->pstmetaAllocator, pst_meta_allocation);
}
void mi_sys_meta_allocation_unmap_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    MI_SYS_BUG_ON(!pstAllocation);
    pst_meta_allocation = container_of(pstAllocation, mi_sys_meta_buf_allocation_t, stdBufAllocation);
    MI_SYS_BUG_ON(!pst_meta_allocation->pstmetaAllocator || pst_meta_allocation->pstmetaAllocator->u32MagicNumber !=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
    mi_sys_meta_allocator_unmap_usr(pst_meta_allocation->pstmetaAllocator, pst_meta_allocation);
}

void *mi_sys_meta_allocation_on_map_kern(MI_SYS_BufferAllocation_t *pstAllocation)
{
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    MI_SYS_BUG_ON(!pstAllocation);
    pst_meta_allocation = container_of(pstAllocation, mi_sys_meta_buf_allocation_t, stdBufAllocation);
    MI_SYS_BUG_ON(!pst_meta_allocation->pstmetaAllocator || pst_meta_allocation->pstmetaAllocator->u32MagicNumber !=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
    return pst_meta_allocation->va_in_kern;
}

void mi_sys_meta_allocation_on_unmap_kern(MI_SYS_BufferAllocation_t *pstAllocation)
{
    MI_SYS_BUG_ON(!pstAllocation);

    //do nothing.
}

mi_sys_Allocator_t *mi_sys_get_global_meta_allocator(void)
{
    static mi_sys_Allocator_t *g_metadata_allocator = NULL;//this is a static variate ,will only set value once.
    MI_SYS_INFO("%s:%d \n",__FUNCTION__,__LINE__);

    if(g_metadata_allocator)
    {
        MI_SYS_INFO("%s:%d \n",__FUNCTION__,__LINE__);
        return g_metadata_allocator;//if static variate already been set before ,directly return it.
    }

    g_metadata_allocator = mi_sys_meta_allocator_create();
    if(g_metadata_allocator)
    {
        g_metadata_allocator->ops->OnRef(g_metadata_allocator);
    }
    else
    {
        MI_SYS_INFO("%s:%d \n",__FUNCTION__,__LINE__);
        MI_SYS_BUG();
    }

    MI_SYS_INFO("%s:%d \n",__FUNCTION__,__LINE__);
    return g_metadata_allocator;
}
