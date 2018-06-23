
struct kmem_cache *mi_sys_meta_allocation_cachep;

buf_allocator_ops_t meta_allocator_ops = 
{
  .OnRef = generic_allocator_on_ref,
  .OnUnref = generic_allocator_on_unref,
  .OnRelease = mi_sys_meta_allocator_on_release,
  .alloc = mi_sys_meta_allocator_alloc,
  .suit_bufconfig = mi_sys_meta_allocator_suit_bufconfig;
};

buf_allocation_ops_t meta_allocation_ops = 
{
    .OnRef = generic_allocation_on_ref;
    .OnUnref = generic_allocation_on_unref,
    .OnRelease = mi_sys_meta_allocation_on_release,
    .map_user = mi_sys_meta_allocation_map_user,
    .unmap_user = mi_sys_meta_allocation_unmap_user,
    .vmap_kern =  mi_sys_meta_allocation_on_map_kern,//do not use generic realize,use it self's which do nothing
    .vunmap_kern = mi_sys_meta_allocation_on_unmap_kern    //do not use generic realize,use it self's which do nothing
};


//to be discuss:no need any input for this func ?
mi_sys_meta_allocator_t *mi_sys_meta_allocator_create(/*unsigned long length*/)
{
    int ret;
    mi_sys_meta_allocator_t *pst_meta_allocator;
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    struct sg_table *sg_table;
    struct scatterlist *sg;
    int allocated_page_count = 0;
    unsigned long offset_in_pool;
    
    pst_meta_allocator = (mi_sys_meta_allocator_t*)kmalloc(sizeof(mi_sys_meta_allocator_t));
    if(pst_meta_allocator  == NULL)
    {
           return NULL;
    }

    generic_allocator_init(&pst_meta_allocator->stdAllocator, &meta_allocator_ops);
    pst_meta_allocator->u32MagicNumber = __MI_SYS_META_ALLOCATOR_MAGIC_NUM__;

    INIT_LIST_HEAD(&pst_meta_allocator->list_of_free_allocations);
    sema_init(&pst_meta_allocator->semlock, 1);


    ret = sg_alloc_table(sg_table, MI_SYS_META_DATA_PAGE_COUNT, GFP_KERNEL);
    if (unlikely(ret))
        return NULL;


    pst_meta_allocator->sg_table = sg_table;
    sg=sg_table->sgl;

    while(allocated_page_count < MI_SYS_META_DATA_PAGE_COUNT)
    {
         struct page *page = alloc_page(GFP_KERNEL);
         int i;
         if(!page)
              goto failed_n_free;
         pst_meta_allocator->pages[allocated_page_count++] = page;
         sg_set_page(sg, page, PAGE_SIZE, 0);
         sg = sg_next(sg);
         for( i=0; i<PAGE_SIZE/MI_SYS_META_DATA_SIZE_MAX; i++)
         {
             pst_meta_allocation = (mi_sys_meta_buf_allocation_t*)kmem_cache_alloc(mi_sys_meta_allocation_cachep, GFP_KERNEL);
             if(!pst_meta_allocation)
                goto failed_n_free;
             pst_meta_allocation->length = MI_SYS_META_DATA_SIZE_MAX;
             pst_meta_allocation->phy_addr = __pfn_to_phys(__page_to_pfn(page));
             pst_meta_allocation->pstmetaAllocator = pst_meta_allocator;
             pst_meta_allocation->va_in_kern = (void*)((char*)page_address(page)+MI_SYS_META_DATA_SIZE_MAX*i);
             pst_meta_allocation->offset_in_pool = offset_in_pool;
             offset_in_pool+=MI_SYS_META_DATA_SIZE_MAX;
             //generic_allocation_init(&pst_meta_allocation->stdBufAllocation, &meta_allocation_ops);
             list_add_tail(pst_meta_allocation->list_in_all_list, &pst_meta_allocator->list_of_all_allocations);
             list_add_tail(pst_meta_allocation->list_in_free_list, &pst_meta_allocator->list_of_free_allocations);
         }
    }
        

    return &pst_meta_allocator->stdAllocator;
    
failed_n_free:

    while(allocated_page_count)
    {
        free_page(pst_meta_allocator->pages[--allocated_page_count]);
    }
    while(!list_empty( &pst_meta_allocator->list_of_all_allocations))
    {
         pst_meta_allocation  = container(pst_meta_allocator->list_of_all_allocations.next, mi_sys_meta_buf_allocation_t, list_in_all_list);
         kmem_cache_free(mi_sys_meta_allocation_cachep, pst_meta_allocation);
    }
    sg_free_tagble(pst_meta_allocator->sg_table);
    kfree(pst_meta_allocator);

    
    return NULL;
 
}

///TODO:how to deal with ret???
static inline void mi_sys_meta_allocator_free(mi_sys_meta_allocator_t *pst_meta_allocator, mi_sys_meta_buf_allocation_t *pst_meta_allocation)
{

    int ret;
    BUG_ON(pst_meta_allocator->u32MagicNumber != __MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
    list_add_head(pst_meta_allocation->list_in_free_list, &pst_meta_allocator->list_of_free_allocations);
}

MI_SYS_BufferAllocation_t *mi_sys_meta_allocator_alloc(mi_sys_Allocator_t *pstAllocator,  MI_SYS_BufConf_t *stBufConfig)
{

     mi_sys_meta_buf_allocation_t *pst_meta_allocation;
     mi_sys_meta_allocator_t *pst_meta_allocator;
     unsigned long size_to_alloc;
     
     unsigned long  offset_in_heap;
     void *virt_addr;
      phys_addr_t  phy_addr;
     
     BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);
     BUG_ON(!stBufConfig);

     pst_meta_allocator = container_of(pstAllocator, mi_sys_meta_allocator_t,  stdAllocator);
     BUG_ON(pst_meta_allocator->u32MagicNumber!=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
     
     if(stBufConfig->buf_type != E_MI_SYS_BUFDATA_META)
        return NULL;

     if(stBufConfig.stMetaCfg.u32Size > MI_SYS_META_DATA_SIZE_MAX)
        return NULL;

     down(&pst_meta_allocator->semlock)

     if(list_empty(&pst_meta_allocator->list_of_free_allocations)
      { 
         pst_meta_allocation = NULL;
      }
     else
     {
        pst_meta_allocation = container(pst_meta_allocator->list_of_all_allocations.next, mi_sys_meta_buf_allocation_t, list_in_all_list);
        list_del(&pst_meta_allocation->list_in_free_list);
        INIT_LIST_HEAD(&pst_meta_allocation->list_in_free_list);
     }
     up(&pst_meta_allocator->semlock)
   
     generic_allocation_init(&pst_meta_allocation->stdBufAllocation, &meta_allocation_ops,
                                                                                                                     pst_meta_allocation->phy_addr, stBufConfig.stMetaCfg.u32Size,
                                                                                                                       pst_meta_allocation->va_in_kern);
     return &pst_meta_allocation->stdBufAllocation;
     
}
int mi_sys_meta_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *stBufConfig)
{

     mi_sys_meta_allocator_t *pst_meta_allocator;
     //unsigned long size_to_alloc;
     unsigned long  phy_addr;
     
     BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);

     BUG_ON(!stBufConfig);

     pst_meta_allocator = container_of(pstAllocator, mi_sys_meta_allocator_t,  stdAllocator);

     BUG_ON(pst_meta_allocator->u32MagicNumber!=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
    
     if(stBufConfig->buf_type  != E_MI_SYS_BUFDATA_META)
         return INT_MIN;

    if(stBufConfig.stMetaCfg.u32Size > MI_SYS_META_DATA_SIZE_MAX)
        return INT_MIN;

     return 10000;
   
}

int mi_sys_meta_allocator_on_release(mi_sys_meta_allocator_t *pst_meta_allocator)
{
    int ret;
    mi_sys_meta_buf_allocation_t *pst_meta_allocation;
    int allocated_page_count = MI_SYS_META_DATA_PAGE_COUNT;
     
    BUG_ON(!pst_meta_allocator || pst_meta_allocator->u32MagicNumber != __MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
   
    down(&pst_meta_allocator->semlock);   

    BUG_ON(atomic_read(&pst_meta_allocator->stdAllocator.ref_cnt));

    while(allocated_page_count)
    {
        free_page(pst_meta_allocator->pages[--allocated_page_count]);
    }
    while(!list_empty( &pst_meta_allocator->list_of_all_allocations))
    {
         pst_meta_allocation  = container(pst_meta_allocator->list_of_all_allocations.next, mi_sys_meta_buf_allocation_t, list_in_all_list);
         kmem_cache_free(mi_sys_meta_allocation_cachep, pst_meta_allocation);
    }
    sg_free_tagble(pst_meta_allocator->sg_table);
    up(&pst_meta_allocator->semlock);   
    memset(pst_meta_allocator, 0xC1, sizeof(*pst_meta_allocator));

    kfree(pst_meta_allocator);
    return MI_SUCCESS;
}
void mi_sys_meta_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation)
{
     mi_sys_meta_buf_allocation_t *pst_meta_allocation;
     mi_sys_meta_allocator_t *pst_meta_allocator;
     
     BUG_ON(!pst_allocation);
     BUG_ON(atomic_read(pst_allocation->ref_cnt));

     pst_meta_allocation = container_of(pst_allocation, mi_sys_meta_buf_allocation_t, stdBufAllocation);

     pst_meta_allocator = pst_meta_allocation->pstmetaAllocator;

     BUG_ON(!pst_meta_allocator || pst_meta_allocator->u32MagicNumber!=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);

     mi_sys_meta_allocator_free(pst_meta_allocator, pst_meta_allocation);
}
static inline void *mi_sys_meta_allocator_map_usr(mi_sys_meta_allocator_t*pst_meta_allocator,  mi_sys_meta_buf_allocation_t*pst_meta_allocation)
{
    struct sg_table *sg_table;
    int ret;
    unsigned long long start_addr;
    unsigned long   length;

     if(pst_meta_allocator->user_map_ptr)
     {
           //only support one user map ctx at this time
           BUG_ON(pst_meta_allocator->user_map_ctx != mi_sys_buf_get_cur_user_map_ctx())
           return (void*)((char*)pst_meta_allocator->user_map_ptr+pst_meta_allocation->offset_in_pool);
     }

    ret = sg_alloc_table(sg_table, 1, GFP_KERNEL);
    if (unlikely(ret))
        return NULL;

    ret = mi_sys_buf_mgr_user_map(pst_meta_allocator->sg_table, &pst_meta_allocator->user_map_ptr, &pst_meta_allocator->user_map_ctx);
    if(ret != MI_SUCCESS)
    {
        return NULL;
    }
    
    return (void*)((char*)pst_meta_allocator->user_map_ptr+pst_meta_allocation->offset_in_pool);
}
static inline void mi_sys_meta_allocator_unmap_usr(mi_sys_meta_allocator_t*pst_meta_allocator,  mi_sys_meta_buf_allocation_t*pst_meta_allocation)
{
    unsigned long size_to_alloc;
    unsigned long  phy_addr;
    int ret;

    BUG_ON(pst_meta_allocator->u32MagicNumber!=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
    //do nothing
}
void *mi_sys_meta_allocation_map_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
   mi_sys_meta_buf_allocation_t *pst_meta_allocation;
   BUG_ON(!pstAllocation);
   pst_meta_allocation = container_of(pstAllocation, mi_sys_meta_buf_allocation_t, stdBufAllocation);
   BUG_ON(!pst_meta_allocation->pstMMAAllocator || pst_meta_allocation->pstmetaAllocator->u32MagicNumber !=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
   return mi_sys_meta_allocator_map_usr(pst_meta_allocation->pstmetaAllocator, pst_meta_allocation);
}
void mi_sys_meta_allocation_unmap_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
   mi_sys_meta_buf_allocation_t *pst_meta_allocation;
   BUG_ON(!pstAllocation);
   pst_meta_allocation = container_of(pstAllocation, mi_sys_meta_buf_allocation_t, stdBufAllocation);
   BUG_ON(!pst_meta_allocation->pstmetaAllocator || pst_meta_allocation->pstmetaAllocator->u32MagicNumber !=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
   mi_sys_meta_allocator_unmap_usr(pst_meta_allocation->pstmetaAllocator, pst_meta_allocation);
}

void *mi_sys_meta_allocation_on_map_kern(MI_SYS_BufferAllocation_t *pstAllocation)
{
   mi_sys_meta_buf_allocation_t *pst_meta_allocation;
   BUG_ON(!pstAllocation);
   pst_meta_allocation = container_of(pstAllocation, mi_sys_meta_buf_allocation_t, stdBufAllocation);
   BUG_ON(!pst_meta_allocation->pstMMAAllocator || pst_meta_allocation->pstmetaAllocator->u32MagicNumber !=__MI_SYS_META_ALLOCATOR_MAGIC_NUM__);
   return pst_meta_allocation->va_in_kern;
}
void mi_sys_meta_allocation_on_unmap_kern(MI_SYS_BufferAllocation_t *pstAllocation)
{
    BUG_ON(!pstAllocation);

   //do nothing.
}

