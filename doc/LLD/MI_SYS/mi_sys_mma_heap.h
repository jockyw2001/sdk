#ifndef _MI_SYS_MMA_HEAP_H_
#define _MI_SYS_MMA_HEAP_H_
#define __MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__ 0x484D4D41
#define  MI_SYS_MMA_HEAP_NAME_MAX_LEN 256
    
struct kmem_cache *mi_sys_mma_allocation_cachep;


struct mi_sys_mma_allocator_s;
typedef struct mi_sys_mma_buf_allocation_s
{
   unsigned long   offset_in_heap;
   unsigned long   length;
   mi_sys_mma_allocator_s *pstMMAAllocator;
   void *user_map_ptr;//mapped_va in user space
   MI_SYS_USER_MAP_CTX user_map_ctx;
   MI_SYS_BufferAllocation_t stdBufAllocation;
}mi_sys_mma_buf_allocation_t;

typedef struct mi_sys_mma_allocator_s
{
   struct list_head list;//list in global g_mma_allocator list
    unsigned int u32MagicNumber;
    unsigned long length;
    unsigned long long heap_base_addr;
    mi_sys_chunk_mgr_t chunk_mgr;
    mi_sys_Allocator_t stdAllocator;
    char heap_name[MI_SYS_MMA_HEAP_NAME_MAX_LEN];
}mi_sys_mma_allocator_t;

MI_SYS_BufferAllocation_t *mi_sys_alloc_from_mma_allocators( const char *mma_heap_name, MI_SYS_BufConf_t *pstBufConfig);

#endif