
#define  MI_SYS_VBPOOL_MAX_IDR_SIZE  0x1000
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
    .OnRef = generic_allocation_on_ref;
    .OnUnref = generic_allocation_on_unref,
    .OnRelease = mi_sys_vbpool_allocation_on_release,
    .map_user = mi_sys_vbpool_allocation_map_user,
    .unmap_user = mi_sys_vbpool_allocation_unmap_user,
    .vmap_kern =  generic_allocation_on_map_kern,
    .vunmap_kern = generic_allocation_on_unmap_kern
};
mi_sys_Allocator_t *mi_sys_vbpool_allocator_create(const char *mma_heap_name, unsigned long size, unsigned long count)
{
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    MI_SYS_BufferAllocation_t *mma_allocation;
    mi_sys_vbpool_allocation_t *vbpool_allocation;
    int size_aligned = ALIGN_UP(size, PAGE_SIZE);
    
    int allocated_count = 0;
    MI_SYS_BufConf_t buf_config;
    struct list_head *pos;

    struct sg_table *sg_table;
    int ret;
    struct scatterlist *sg;

    BUG_ON(size==0);
    BUG_ON(count==0);

    pst_vbpool_allocator = (mi_sys_vbpool_allocator_t*)kmalloc(sizeof(mi_sys_vbpool_allocator_t));
    if(pst_vbpool_allocator  == NULL)
           return NULL;

    generic_allocator_init(&pst_vbpool_allocator->stdAllocator, &vbpool_allocator_ops);
    pst_vbpool_allocator->u32MagicNumber = __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__;
    sema_init(&pst_vbpool_allocator->semlock, 1);

    INIT_LIST_HEAD(&pst_vbpool_allocator->list_of_all_allocations);
    INIT_LIST_HEAD(&pst_vbpool_allocator->list_of_free_allocations);

    pst_vbpool_allocator->size = size;
    pst_vbpool_allocator->count = count;

    buf_config.eBufType = E_MI_SYS_BUFDATA_RAW;
    buf_config.u64TargetPts = 0;
    buf_config.stRawCfg.u32Size = size_aligned;

    ret = sg_alloc_table(sg_table, count, GFP_KERNEL);
    if (unlikely(ret))
        return NULL;


    pst_vbpool_allocator->sg_table = sg_table;
    sg=sg_table->sgl;

    while(allocated_count < count)
    {
         mma_allocation = mi_sys_alloc_from_mma_allocators(mma_heap_name, &buf_config, NULL);
         if(!mma_allocation)
              goto failed_n_free;
         mma_allocation->ops->OnRef(mma_allocation);
         vbpool_allocation = (mi_sys_vbpool_allocation_t*)kmem_cache_alloc(mi_sys_vbpool_allocation_cachep, GFP_KERNEL);
         if(!vbpool_allocation)
         {
              mma_allocation->ops->OnUnref(mma_allocation);
               goto failed_n_free;
         }
         vbpool_allocation->offset_in_vb_pool = allocated_count*size_aligned;
         vbpool_allocation->mma_allocation = mma_allocation;
         vbpool_allocation->pstParentAllocator = pst_vbpool_allocator;
         generic_allocation_init(&vbpool_allocation->stdBufAllocation, &vbpool_allocation_ops,
                                                         mma_allocation->u64PhyAddr, mma_allocation->u32Length, mma_allocation->va_in_kern);
         list_add_tail(&vbpool_allocation->list_in_all,   &pst_vbpool_allocator->list_of_all_allocations);
         list_add_tail(&vbpool_allocation->list_in_free,   &pst_vbpool_allocator->list_of_free_allocations);


         sg_set_page(sg, pfn_to_page(__phys_to_pfn(vbpool_allocation->mma_allocation->u64PhyAddr)), PAGE_ALIGN(vbpool_allocation->mma_allocation->u32Length), 0);
         sg = sg_next(sg);
         
    }
    
    return &pst_vbpool_allocator->stdAllocator;
    
failed_n_free:
     
    while(!list_empty( &pst_vbpool_allocator->list_of_all_allocations))
    {
         vbpool_allocation = container(pst_vbpool_allocator->list_of_all_allocations.next, mi_sys_vbpool_allocation_t, list_in_all);
         vbpool_allocation->mma_allocation->ops->OnUnref(vbpool_allocation->mma_allocation);
         list_del(&vbpool_allocation->list_in_all);
         memset(vbpool_allocation, 0xE8, sizeof(*vbpool_allocation));
         kmem_cache_free(mi_sys_vbpool_allocation_cachep, vbpool_allocation);
    }
    memset(pst_vbpool_allocator, 0xE9, sizeof(*pst_vbpool_allocator));
    sg_free_tagble(pst_vbpool_allocator->sg_table);
    kfree(pst_vbpool_allocator);
    
    return NULL;
 
}

static inline void mi_sys_vbpool_allocator_free(mi_sys_vbpool_allocator_t *pst_vbpool_allocator, mi_sys_vbpool_allocation_t *vbpool_allocation)
{

    BUG_ON(vbpool_allocation->pstParentAllocator != pst_vbpool_allocator);
    down(&pst_vbpool_allocator->semlock);
    list_add_tail(&vbpool_allocation->list_in_free, pst_vbpool_allocator&vbpool_allocation->list_in_free);
    up(&pst_vbpool_allocator->semlock);
    

}

MI_SYS_BufferAllocation_t *mi_sys_vbpool_allocator_alloc(mi_sys_Allocator_t *pstAllocator,
                                       MI_SYS_BufConf_t *stBufConfig)
{

     mi_sys_vbpool_allocation_t *vbpool_allocation;
     unsigned long size_to_alloc;
     mi_sys_vbpool_allocator_t *pst_vbpool_allocator;


     BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber!=__MI_SYS_ALLOCATOR_MAGIC_NUM__);
     BUG_ON(!stBufConfig);
 
     
     pst_vbpool_allocator = container_of(pstAllocator, mi_sys_vbpool_allocator_t, stdAllocator);
     BUG_ON(pst_vbpool_allocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);
        
     size_to_alloc = mi_sys_buf_mgr_get_size(stBufConfig);

     if(size_to_alloc > pst_vbpool_allocator->size)
          return NULL;

     down(&pst_vbpool_allocator->semlock);
     if(!list_empty(pst_vbpool_allocator->list_of_free_allocations))
      {
           vbpool_allocation = container_of(pst_vbpool_allocator->list_of_free_allocations.next, mi_sys_vbpool_allocation_t,list_in_free));
           BUG_ON(atomic_read(&vbpool_allocation->stdBufAllocation.ref_cnt));
           BUG_ON(vbpool_allocation->mma_allocation==NULL ||
              vbpool_allocation->pstParentAllocator!=pst_vbpool_allocator);
           BUG_ON( vbpool_allocation->ops != &vbpool_allocation_ops);
           list_del(&vbpool_allocation->list_in_free);
           INIT_LIST_HEAD(&vbpool_allocation->list_in_free);
           up(&pst_vbpool_allocator->semlock);
           return vbpool_allocation->stdBufAllocation;
      }
     up(&pst_vbpool_allocator->semlock);
     return NULL;
     
}
int mi_sys_vbpool_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *stBufConfig)
{

    int ret = 4096;
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    mi_sys_vbpool_allocation_t *vbpool_allocation;
    unsigned long size_to_alloc;
    unsigned long power2;
    BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);
    BUG_ON(!stBufConfig);
    
    pst_vbpool_allocator = container_of(pstAllocator, mi_sys_vbpool_allocator_t, stdAllocator);

    BUG_ON(pst_vbpool_allocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);

    size_to_alloc = mi_sys_buf_mgr_get_size(stBufConfig);
    
    if(stBufConfig->buf_type == E_MI_SYS_BUFDATA_META)
         return INT_MIN;

    if(pst_vbpool_allocator->size < size_to_alloc)
         return INT_MIN;

     size_to_alloc = size_to_alloc-pst_vbpool_allocator->size;

     if(size_to_alloc == 0)
           return ret;

     if(size_to_alloc<=1024*256UL)
     {  
          return 4096-size_to_alloc/256;
     }
     else if(size_to_alloc<=1024*1024UL)
     {
          return 3072-(size_to_alloc-1024*256UL)/768;
     }
     else if(size_to_alloc<=2048*1024UL)
     {
        return 2048-(size_to_alloc-1024*1024UL)/1024;
         
     }
     else if(size_to_alloc<=4096*1024UL)
     {
        return 1024-(size_to_alloc-2048*1024UL)/2048;
         
     }
     else
        return 0;   
}

int mi_sys_vbpool_allocator_on_release(struct mi_sys_Allocator_s *pstAllocator)
{
    mi_sys_vbpool_allocator_t *pst_vbpool_allocator;
    mi_sys_vbpool_allocation_t *vbpool_allocation;
    BUG_ON(!pstAllocator || pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);

    BUG_ON(atomic_read(&pstAllocator->ref_cnt));

    pst_vbpool_allocator = container_of(pstAllocator, mi_sys_vbpool_allocator_t, stdAllocator);

    BUG_ON(pst_vbpool_allocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);
   
    while(!list_empty( &pst_vbpool_allocator->list_of_all_allocations))
    {
         mi_sys_vbpool_allocation_t *vbpool_allocation = container(pst_vbpool_allocator->list_of_all_allocations.next, mi_sys_vbpool_allocation_t, list_in_all);
         BUG_ON(vbpool_allocation->pstParentAllocator != pst_vbpool_allocator);
         BUG_ON(!vbpool_allocation->mma_allocation);
         BUG_ON(list_empty(&vbpool_allocation->list_in_free));
         vbpool_allocation->mma_allocation->ops->OnUnref(vbpool_allocation->mma_allocation);
         list_del(&vbpool_allocation->list_in_all);
         memset(vbpool_allocation, 0xE8, sizeof(*vbpool_allocation));
         kmem_cache_free(mi_sys_vbpool_allocation_cachep, vbpool_allocation);
    }  
    memset(pst_vbpool_allocator, 0xEA, sizeof(*mi_sys_vbpool_allocator_t));
    sg_free_tagble(pst_vbpool_allocator->sg_table);
    kfree(pst_vbpool_allocator);
    return MI_SUCCESS;
}
void mi_sys_vbpool_allocation_on_release(MI_SYS_BufferAllocation_t *pst_allocation)
{
    mi_sys_vbpool_allocation_t *vbpool_allocation;
    
    BUG_ON(!pst_allocation || pst_allocation->ops !=&vbpool_allocation_ops);

    vbpool_allocation = container_of(pst_allocation, mi_sys_vbpool_allocation_t, MI_SYS_BufferAllocation_t);
    BUG_ON(!vbpool_allocation->pstParentAllocator);
    BUG_ON(atomic_read(&pst_allocation->ref_cnt));
    BUG_ON(vbpool_allocation->pstParentAllocator->u32MagicNumber != __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);
    mi_sys_vbpool_allocator_free(vbpool_allocation->pstParentAllocator, vbpool_allocation);
   
}
static inline void *mi_sys_vbpool_allocator_map_usr(mi_sys_vbpool_allocator_t*pst_vbpool_allocator,  mi_sys_vbpool_allocation_t*pst_vbpool_allocation)
{
     int ret;
     if(pst_vbpool_allocator->user_map_ptr)
     {
           //only support one user map ctx at this time
           BUG_ON(pst_vbpool_allocator->user_map_ctx != mi_sys_buf_get_cur_user_map_ctx());
           return pst_vbpool_allocation->offset_in_vb_pool+pst_vbpool_allocator->user_map_ptr;
     }
     ret = mi_sys_buf_mgr_user_map(pst_vbpool_allocator->sg_table, 
                                                               &pst_vbpool_allocator->user_map_ptr, &pst_vbpool_allocator->user_map_ctx);
    if(ret != MI_SUCCESS)
    {
        return NULL;
    }

     return pst_vbpool_allocation->offset_in_vb_pool+pst_vbpool_allocator->user_map_ptr;
}

///TODO:how to deal with ret???
static inline void mi_sys_vbpool_allocator_unmap_usr(mi_sys_vbpool_allocator_t*pst_vbpool_allocator)
{
     unsigned long size_to_alloc;
     unsigned long  phy_addr;
     int ret;

     BUG_ON(pst_vbpool_allocator->u32MagicNumber!=__MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);

      if(pst_vbpool_allocator->user_map_ptr)
     {
           //only support one user map ctx at this time
             ret = mi_sys_buf_mgr_user_unmap(pst_vbpool_allocator->user_map_ptr, 
                                            pst_vbpool_allocator->user_map_ctx, pst_vbpool_allocator->size_aligned*pst_vbpool_allocator->count);
             if(ret != MI_SUCCESS)
             {
                 ///TODO:how to deal with ??
             }
             pst_vbpool_allocator->user_map_ptr = NULL;
             pst_vbpool_allocator->user_map_ctx = NULL;
     }
}
void *mi_sys_vbpool_allocation_map_user(MI_SYS_BufferAllocation_t *pstAllocation)
{
   mi_sys_vbpool_allocation_t *pst_vbpool_allocation;
   BUG_ON(!pstAllocation);
   BUG_ON(pstAllocation->ops != &vbpool_allocation_ops);
   pst_vbpool_allocation = container_of(pstAllocation, pst_vbpool_allocation, stdBufAllocation);
   BUG_ON(!pst_vbpool_allocation->pstMMAAllocator || pst_vbpool_allocation->pstMMAAllocator !=__MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__);
   return mi_sys_vbpool_allocator_map_usr(pst_vbpool_allocation->pstMMAAllocator, pst_vbpool_allocation);
}
void mi_sys_vbpool_allocation_unmap_user(struct MI_SYS_BufferAllocation_t *pstAllocation)
{
   BUG_ON(!pstAllocation);
   BUG_ON(pstAllocation->ops != &vbpool_allocation_ops);
   //do nothing
}

