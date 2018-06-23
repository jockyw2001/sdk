#ifndef _MI_SYS_BUFMGR_H_
#define _MI_SYS_BUFMGR_H_

#define __MI_SYS_ALLOCATOR_MAGIC_NUM__ 0x4D414C43
#define __MI_SYS_COLLECTION_MAGIC_NUM__ 0x4D434F4C


#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))

struct MI_SYS_BufferAllocation_s;
typedef struct buf_allocation_ops_s
{
    void (*OnRef)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*OnUnref)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*OnRelease)(struct MI_SYS_BufferAllocation_s *thiz);
    void *(*map_user)(struct MI_SYS_BufferAllocation_s *thiz);
    void  (*unmap_user)(struct MI_SYS_BufferAllocation_s *thiz);
    void *(vmap_kern)(struct MI_SYS_BufferAllocation_s *thiz);
    void *(vunmap_kern)(struct MI_SYS_BufferAllocation_s *thiz);
}buf_allocation_ops_t;

typedef struct MI_SYS_BufferAllocation_s
{
   atomic_t  ref_cnt;
   buf_allocation_ops_t *ops;
   void *va_in_kern;//could be NULL
   MI_PHY u64PhyAddr;
   unsigned long u32Length;
}MI_SYS_BufferAllocation_t;

typedef void (*OnBufRefRelFunc)(struct MI_SYS_BufRef_s *pstBufRef, void *pCBData);

typedef struct MI_SYS_BufRef_s
{
    struct list_head list;
    struct MI_SYS_BufferAllocation_t *pstBufAllocation;
    OnBufRefRelFunc onRelCB;
    void *pCBData;
    MI_SYS_BufInfo_t bufinfo;
}MI_SYS_BufRef_t;

typedef struct MI_SYS_BufferQueue_s
{
    struct semaphore semlock;
    struct list_head list;
    int  queue_buf_count;
}MI_SYS_BufferQueue_t;

 

struct mi_sys_Allocator_s;
typedef struct buf_allocator_ops_s
{
    MI_S32 (*OnRef)(struct mi_sys_Allocator_s *thiz);
    MI_S32 (*OnUnref)(struct mi_sys_Allocator_s *thiz);
    MI_S32 (*OnRelease)(struct mi_sys_Allocator_s *thiz);
    MI_SYS_BufferAllocation_t *(*alloc)(mi_sys_Allocator_t *pstAllocator, MI_Buf_Config *stBufConfig);
    int (*suit_bufconfig)(mi_sys_Allocator_t *pstAllocator, MI_Buf_Config *pstBufConfig);
}buf_allocator_ops_t;

typedef struct mi_sys_Allocator_s
{
    atomic_t  ref_cnt;
    unsigned int u32MagicNumber;
    buf_allocator_ops_t *ops;
}mi_sys_Allocator_t;


typedef struct MI_SYS_AllocatorRef_s
{
    struct list_head list;
    struct mi_sys_Allocator_t *pstAllocator;
}MI_SYS_AllocatorRef_t;

typedef struct MI_SYS_Allocator_Collection_s
{
    unsigned int u32MagicNumber;
    struct semaphore semlock;
    struct list_head list;
    int  collection_size;
}MI_SYS_Allocator_Collection_t;


typedef void * MI_SYS_USER_MAP_CTX;


static inline void generic_allocation_on_ref(struct MI_SYS_BufferAllocation_s *pstBufAllocation)
{
     int ref_cnt;
     BUG_ON(!pstBufAllocation);
     BUG_ON(!pstBufAllocation->ops || !pstBufAllocation->ops->OnRef != generic_buf_on_ref);

     ref_cnt = atomic_inc_return(&pstBufAllocation->ref_cnt);

     BUG_ON(ref_cnt<=0);    
}


static inline void generic_allocation_on_unref(struct MI_SYS_BufferAllocation_s *pstBufAllocation)
{
     int ref_cnt;
     BUG_ON(!pstBufAllocation);
     BUG_ON(!pstBufAllocation->ops || !pstBufAllocation->ops->OnUnref != generic_buf_on_unref);

     ref_cnt = atomic_dec_return(&pstBufAllocation->ref_cnt);

     BUG_ON(ref_cnt<0);    
     if(ref_cnt ==0)
     {
        if(pstBufAllocation->va_in_kern)
        {
            BUG_ON(&&!pstBufAllocation->vunmap_kern);
            pstBufAllocation->vunmap_kern(pstBufAllocation->viraddr_in_kernel);
         }
        pstBufAllocation->ops->OnRelease(pstBufAllocation);
      }
}
static inline void* generic_allocation_on_map_kern(struct MI_SYS_BufferAllocation_s *pstBufAllocation)
{
     int ref_cnt;
     BUG_ON(!pstBufAllocation);
     BUG_ON(!pstBufAllocation->ops || !pstBufAllocation->ops->vmap_kern != generic_allocation_on_map_kern);
     if(!pstBufAllocation->va_in_kern)
         pstBufAllocation->va_in_kern = vmap(pstBufAllocation->u64PhyAddr, pstBufAllocation->u32Length);
     return pstBufAllocation->va_in_kern;
}
static inline generic_allocation_init(MI_SYS_BufferAllocation_t *pstBufAllocation,buf_allocation_ops_t *ops,
                                                                                                                                unsigned long phyaddr,unsigned long length, void *va_in_kern)
{
   BUG_ON(!pstBufAllocation);
    
   BUG_ON(!pstBufAllocation || !ops);
   BUG_ON(!ops->OnRef || !ops->OnUnref ||!ops->OnRelease);

   pstBufAllocation->ops = ops;
   pstBufAllocation->ref_cnt = ATOMIC_INIT(0);
   pstBufAllocation->viraddr_in_kernel  = va_in_kern;
   pstBufAllocation->u64PhyAddr = phyaddr;
   pstBufAllocation->u32Length = length;
}
static inline void generic_allocation_on_unmap_kern(struct MI_SYS_BufferAllocation_s *pstBufAllocation)
{
     int ref_cnt;
     BUG_ON(!pstBufAllocation);
     BUG_ON(!pstBufAllocation->ops || !pstBufAllocation->ops->vunmap_kern != generic_allocation_on_unmap_kern);
     BUG_ON(!pstBufAllocation->va_in_kern);
     vunmap(pstBufAllocation->va_in_kern);
     pstBufAllocation->va_in_kern = NULL;
}

static inline void generic_allocator_on_ref(struct mi_sys_Allocator_s *pstAllocator)
{
     int ref_cnt;
     BUG_ON(!pstAllocator);
     BUG_ON(!pstAllocator->ops || !pstAllocator->ops->OnRef != generic_allocator_on_ref);

     ref_cnt = atomic_inc_return(&pstAllocator->ref_cnt);

     BUG_ON(ref_cnt<=0);    
}


static inline void generic_allocator_on_unref(struct mi_sys_Allocator_s *pstAllocator)
{
     int ref_cnt;
     BUG_ON(!pstAllocator);
     BUG_ON(!pstAllocator->ops || !pstAllocator->ops->OnUnref != generic_allocator_on_unref);

     ref_cnt = atomic_dec_return(&pstAllocator->ref_cnt);

     BUG_ON(ref_cnt<0);    
     if(ref_cnt ==0)
        pstAllocator->ops->OnRelease(pstAllocator);
}
static inline void generic_allocator_init(mi_sys_Allocator_t *pstAllocator, buf_allocator_ops_t *ops)
{
     BUG_ON(!pstAllocator);

     BUG_ON(!ops || !ops->OnRef || !ops->OnUnref || !ops->OnRelease ||
        !ops->alloc ||!ops->suit_bufconfig);


     pstAllocator->u32MagicNumber = __MI_SYS_ALLOCATOR_MAGIC_NUM__;
     INIT_LIST_HEAD(&pstAllocator->list);
     pstAllocator->ref_cnt = ATOMIC_INIT(0);
     pstAllocator->ops = ops;
}

MI_S32 mi_sys_attach_allocator_to_collection(mi_sys_Allocator_t *pstAllocator, MI_SYS_Allocator_Collection_t *pstAllocatorCollection);
MI_S32 mi_sys_detach_allocator_from_collection(mi_sys_Allocator_t *pstAllocator, MI_SYS_Allocator_Collection_t *pstAllocatorCollection);
void mi_sys_init_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection);
void mi_sys_deinit_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection);
MI_SYS_BufferAllocation_t * mi_sys_alloc_from_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection, MI_Buf_Config *pstBufConfig , MI_SYS_ChnPort_t *pstChnPort);
unsigned long mi_sys_buf_mgr_get_size(MI_Buf_Config *stBufConfig);
MI_SYS_USER_MAP_CTX *mi_sys_buf_get_cur_user_map_ctx();
unsigned int mi_sys_buf_mgr_user_map(struct sg_table *sg_table,void *user_map_ptr,MI_SYS_USER_MAP_CTX *user_map_ctx);
int mi_sys_buf_mgr_user_unmap(void *user_map_ptr,MI_SYS_USER_MAP_CTX *user_map_ctx,unsigned long total_len);
unsigned long Calc_Frame_Size_by_PixelFormat(MI_SYS_BufFrameConfig_t stFrameConfig,  unsigned long alignment);
void mi_sys_buf_mgr_fill_bufinfo(MI_SYS_BufferAllocation_t *pstBufAllocation,MI_SYS_BufConf_t *pstBufConfig, MI_SYS_BufInfo_t *bufinfo);
#endif
