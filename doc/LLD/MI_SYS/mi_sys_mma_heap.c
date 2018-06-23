#include "mi_sys_mma_miu_protect.h"


DEFINE_SEMAPHORE(g_mma_list_semlock);
LIST_HEAD(g_mma_heap_list);

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
    .OnRef = generic_allocation_on_ref;
    .OnUnref = generic_allocation_on_unref,
    .OnRelease = mi_sys_mma_allocation_on_release,
    .map_user = mi_sys_mma_allocation_map_user,
    .unmap_user = mi_sys_mma_allocation_unmap_user,
        .vmap_kern =  generic_allocation_on_map_kern,
    .vunmap_kern = generic_allocation_on_unmap_kern
};

int glob_miu_kranges_init_done = 0;
unsigned char *g_kernel_protect_client_id = NULL;

mi_sys_mma_allocator_t *mi_sys_mma_allocator_create(const char *mma_heap_name, unsigned long long base_addr, unsigned long length)
{
    mi_sys_mma_allocator_t *pst_mma_allocator;
    //char *heap_name;
    //int name_length;
    int ret;

    BUG_ON(base_addr & ~PAGE_MASK);
    BUG_ON(length & ~PAGE_MASK);

    BUG_ON(!length);

    pst_mma_allocator = (mi_sys_mma_allocator_t*)kmalloc(sizeof(mi_sys_mma_allocator_t));
    if(pst_mma_allocator  == NULL)
    {
        kfree(tmp);
        return NULL;
    }

    generic_allocator_init(&pst_mma_allocator->stdAllocator, &mma_allocator_ops);
    pst_mma_allocator->u32MagicNumber = __MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__;

    pst_mma_allocator->heap_base_addr = base_addr;
    pst_mma_allocator->length = length;

    if(mma_heap_name)
    {
          strncpy(pst_mma_allocator->heap_name, mma_heap_name, MI_SYS_MMA_HEAP_NAME_MAX_LEN);
          pst_mma_allocator->heap_name[MI_SYS_MMA_HEAP_NAME_MAX_LEN-1] = 0;
    }
    else
    {
           pst_mma_allocator->heap_name[0] = 0;
    }
    
    sema_init(&pst_mma_allocator->semlock, 1);   
    
    ret = mi_sys_init_chuck(&pst_mma_allocator->chunk_mgr, length);//start from offset 0 in mma heap
    if(ret != MI_SUCCESS)
    {
        kfree(pst_mma_allocator);
        kfree(tmp);
        return NULL;
    }

    tmp->pst_mma_allocator = pst_mma_allocator;
    list_add(&tmp->list,&mma_heap_list);


    //about miu protect
    if(false == glob_miu_kranges_init_done)
    {
        if(FALSE == MDrv_MIU_Init())
        {
                 list_del(&tmp->list);
                 kmem_cache_free(mi_sys_mma_allocation_cachep,pst_mma_allocator->chunk_mgr);
                 kfree(pst_mma_allocator);
                    
                 kfree(tmp);
                 return NULL;
        }
        g_kernel_protect_client_id = MDrv_MIU_GetDefaultClientID_KernelProtect();

       /* to set kernel_protect for each lxmem */
       init_glob_miu_kranges();

        glob_miu_kranges_init_done = true;
    }

    ret = deleteKRange(base_addr,  length);
    BUG_ON(ret != MI_SUCCESS);
    
    return pst_mma_allocator;
 
}


static inline void mi_sys_mma_allocator_free_allocation(mi_sys_mma_allocator_t *pst_mma_allocator, mi_sys_mma_buf_allocation_t *pst_mma_allocation)
{
    int ret;
    
    BUG_ON(pst_mma_allocator->u32MagicNumber != __MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
    
    if(pst_mma_allocation->user_map_ptr)//to be discuss :free mma ,if mapped user va,will unmap????
    {
        ret = mi_sys_buf_mgr_user_unmap(pst_mma_allocation->user_map_ptr, &pst_mma_allocation->user_map_ctx,pst_mma_allocation->length);
        if(ret != MI_SUCCESS)
        {
            ///TODO:how to deal with ret???
        }
        pst_mma_allocation->user_map_ptr = NULL;
        pst_mma_allocation->user_map_ctx = NULL;
    }
    mi_sys_free_chuck(&pst_mma_allocator->chunk_mgr, pst_mma_allocation->offset_in_heap);

    
    memset(pst_mma_allocation, 0xE7, sizeof(*pst_mma_allocation));
    kmem_cache_free(mi_sys_mma_allocation_cachep, pst_mma_allocation);
}

MI_SYS_BufferAllocation_t *mi_sys_mma_allocator_alloc(mi_sys_Allocator_t *pstAllocator,
                                       MI_SYS_BufConf_t *stBufConfig , MI_SYS_ChnPort_t *stChnPort)
{

     mi_sys_mma_buf_allocation_t *pst_mma_allocation;
     mi_sys_mma_allocator_t *pst_mma_allocator;
     unsigned long size_to_alloc;
     unsigned long  phy_addr;
     unsigned long  offset_in_heap;
     
     BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);

     BUG_ON(!stBufConfig);


     size_to_alloc = mi_sys_buf_mgr_get_size(stBufConfig);
     

     pst_mma_allocator = container_of(pstAllocator, mi_sys_mma_allocator_t,  stdAllocator);

     BUG_ON(pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
     
     
     ret =  mi_sys_alloc_chuck(&pst_mma_allocator->chunk_mgr,  size_to_alloc,&offset_in_heap);
     if(ret != MI_SUCCESS)
        return NULL;
     
     phy_addr = pst_mma_allocator ->heap_base_addr + offset_in_heap;



    pst_mma_allocation = kmem_cache_alloc(mi_sys_mma_allocation_cachep, GFP_KERNEL);
    if(!pst_mma_allocation)
    {
         mi_sys_free_chuck(&pst_mma_allocator->chunk_mgr,  offset_in_heap);
        return NULL;
    }
    
     pst_mma_allocation->offset_in_heap = offset_in_heap;
     pst_mma_allocation->length = size_to_alloc;
     pst_mma_allocation->user_map_ctx = NULL;
     pst_mma_allocation->user_map_ptr = NULL;
   
     generic_allocation_init(&pst_mma_allocation->stdBufAllocation, &mma_allocation_ops,phy_addr,size_to_alloc, NULL);
     return &pst_mma_allocation->stdBufAllocation;
     
}
int mi_sys_mma_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *stBufConfig)
{

     mi_sys_mma_allocator_t *pst_mma_allocator;
     //unsigned long size_to_alloc;
     unsigned long  phy_addr;
     
     BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);

     BUG_ON(!stBufConfig);

     pst_mma_allocator = container_of(pstAllocator, mi_sys_mma_allocator_t,  stdAllocator);

     BUG_ON(pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
    
     if(stBufConfig->buf_type == E_MI_SYS_BUFDATA_META)
         return INT_MIN;
     
     return 100;
   
}

int mi_sys_mma_allocator_on_release(mi_sys_mma_allocator_t *pst_mma_allocator)
{
    int ret;
    mi_sys_mma_allocator_t *tmp_pst_mma_allocator;
    int find_allocator_in_heap_list = 0;

    BUG_ON(!pst_mma_allocator || pst_mma_allocator->u32MagicNumber != __MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
   
    down(&pst_mma_allocator->semlock);   

    BUG_ON(atomic_read(&pst_mma_allocator->stdAllocator.ref_cnt));

    BUG_ON(false == glob_miu_kranges_init_done);

    /*ret = */
    mi_sys_deinit_chuck(&pst_mma_allocator->chunk_mgr);

    /*if(ret != MI_SUCCESS)
    {
           up(&pst_mma_allocator->semlock);   
           return ret;
     }*/
     up(&pst_mma_allocator->semlock);   

    down(&g_mma_list_semlock);
    list_for_each_safe(pos, q, &g_mma_heap_list)
    {
        tmp_pst_mma_allocator = list_entry(pos, mi_sys_mma_allocator_t, list);
        if(tmp_pst_mma_allocator == pst_mma_allocator)
        {
             list_del(&tmp_pst_mma_allocator->list,&g_mma_heap_list);
             find_allocator_in_heap_list = 1;
             break;
        }
    }
    
    BUG_ON(!find_allocator_in_heap_list );
    up(&g_mma_list_semlock);
    

     ret = addKRange(pst_mma_allocator->heap_base_addr,  pst_mma_allocator->length);
     BUG_ON(ret != MI_SUCCESS);


      memset(pst_mma_allocator, 0xE6, sizeof(*pst_mma_allocator));    
      kfree(pst_mma_allocator);
     return MI_SUCCESS;
}
void mi_sys_mma_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation)
{
     mi_sys_mma_buf_allocation_t *pst_mma_allocation;
     mi_sys_mma_allocator_t *pst_mma_allocator;

     BUG_ON(!pst_allocation);
     BUG_ON(atomic_read(pst_allocation->ref_cnt));

     pst_mma_allocation = container_of(pst_allocation, mi_sys_mma_buf_allocation_t, stdBufAllocation);

     pst_mma_allocator = pst_mma_allocation->pstMMAAllocator;

     BUG_ON(!pst_mma_allocator || pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);

     mi_sys_mma_allocator_free_allocation(pst_mma_allocator, pst_mma_allocation); 
}
static inline void *mi_sys_mma_allocator_map_usr(mi_sys_mma_allocator_t*pst_mma_allocator,  mi_sys_mma_buf_allocation_t*pst_mma_allocation)
{
    struct sg_table *sg_table;
    int ret;
    unsigned long long start_addr;
    unsigned long   length;

     if(pst_mma_allocation->user_map_ptr)
     {
           //only support one user map ctx at this time
           BUG_ON(pst_mma_allocation->user_map_ctx != mi_sys_buf_get_cur_user_map_ctx())
           return pst_mma_allocation->user_map_ptr;
     }

    ret = sg_alloc_table(sg_table, 1, GFP_KERNEL);
    if (unlikely(ret))
        return NULL;

    start_addr = pst_mma_allocator->heap_base_addr+pst_mma_allocation->offset_in_heap;
    length = pst_mma_allocation->length;
    
    BUG_ON(start_addr&~PAGE_MASK || length&~PAGE_MASK);
    
    sg_set_page(sg_table->sgl, pfn_to_page(__phys_to_pfn(start_addr)), PAGE_ALIGN(length), 0);

    ret = mi_sys_buf_mgr_user_map(sg_table, &pst_mma_allocation->user_map_ptr, &pst_mma_allocation->user_map_ctx);
    if(ret != MI_SUCCESS)
    {
       sg_free_table(sg_table);
        return NULL;
    }
    return pst_mma_allocation->user_map_ptr;
}
static inline void mi_sys_mma_allocator_unmap_usr(mi_sys_mma_allocator_t*pst_mma_allocator,  mi_sys_mma_buf_allocation_t*pst_mma_allocation)
{
    unsigned long size_to_alloc;
    unsigned long  phy_addr;
    int ret;

    BUG_ON(pst_mma_allocator->u32MagicNumber!=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);

    if(pst_mma_allocation->user_map_ptr)
    {
             //only support one user map ctx at this time
             ret = mi_sys_buf_mgr_user_unmap(pst_mma_allocation->user_map_ptr, pst_mma_allocation->user_map_ctx,pst_mma_allocation->length);
             pst_mma_allocation->user_map_ptr = NULL;
             pst_mma_allocation->user_map_ctx = NULL;
    }
}
void *mi_sys_mma_allocation_map_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
   mi_sys_mma_buf_allocation_t *pst_mma_allocation;
   BUG_ON(!pstAllocation);
   pst_mma_allocation = container_of(pstAllocation, mi_sys_mma_buf_allocation_t, stdBufAllocation);
   BUG_ON(!pst_mma_allocation->pstMMAAllocator ||pst_mma_allocation->pstMMAAllocator->u32MagicNumber !=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
   return mi_sys_mma_allocator_map_usr(pst_mma_allocation->pstMMAAllocator, pst_mma_allocation);
}
void mi_sys_mma_allocation_unmap_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
   mi_sys_mma_buf_allocation_t *pst_mma_allocation;
   BUG_ON(!pstAllocation);
   pst_mma_allocation = container_of(pstAllocation, mi_sys_mma_buf_allocation_t, stdBufAllocation);
   BUG_ON(!pst_mma_allocation->pstMMAAllocator ||pst_mma_allocation->pstMMAAllocator->u32MagicNumber !=__MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__);
   mi_sys_mma_allocator_unmap_usr(pst_mma_allocation->pstMMAAllocator, pst_mma_allocation);
}

/*
mma_heap_name :alloc from which heap,if set NULL,means no care in which mma heap
*/
MI_SYS_BufferAllocation_t *mi_sys_alloc_from_mma_allocators( const char *mma_heap_name, MI_SYS_BufConf_t *pstBufConfig)
{
    MI_SYS_BufferAllocation_t *mma_allocation;
    mi_sys_mma_allocator_t *pst_mma_allocator;
    struct list_head *pos, *q;
    //BUG_ON(!mma_heap_name);
    BUG_ON(!pstBufConfig);

    //get pstAllocator with mma_heap_name
    pstAllocator = NULL;
    down(&g_mma_list_semlock);

    list_for_each_safe(pos, q, &g_mma_heap_list)
    {
        pst_mma_allocator = list_entry(pos, mi_sys_mma_allocator_t, list);
        if(!mma_heap_name)//no  cache alloc in which mma heap
        {

              mma_allocation =mi_sys_mma_allocator_alloc(&pst_mma_allocator->stdAllocator, pstBufConfig);

              if(!mma_allocation)
              {
                    continue;
               }
              else
                {
                       up(&g_mma_list_semlock);
                       return mma_allocation;//success
                }
        }
        else
        {
            if(0==strcpy(pst_mma_allocator->heap_name,mma_heap_name))
            {
            
                 mma_allocation =mi_sys_mma_allocator_alloc(&pst_mma_allocator->stdAllocator, pstBufConfig);
                 up(&g_mma_list_semlock);
                 return mma_allocation;
            }
        }
        
     }
     up(&g_mma_list_semlock);
     return NULL;
}

