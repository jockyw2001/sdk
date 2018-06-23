#ifndef _MI_SYS_MMA_HEAP_H_
#define _MI_SYS_MMA_HEAP_H_
struct mi_sys_vbpool_allocator_t;

#define __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__ 0x5642504C


typedef struct mi_sys_vbpool_allocation_s
{
    struct list_head list_in_all;
    struct list_head list_in_free;
    struct mi_sys_vbpool_allocator_t *pstParentAllocator;
    MI_S32 offset_in_vb_pool;
    MI_SYS_BufferAllocation_t *mma_allocation;
    MI_SYS_BufferAllocation_t stdBufAllocation;
}mi_sys_vbpool_allocation_t;

typedef struct mi_sys_vbpool_allocator_s
{
    unsigned int u32MagicNumber;
    int vb_pool_id;
    struct list_head list_of_all_allocations;//list in global mma_allocator list
    struct list_head list_of_free_allocations;//list in global mma_allocator list
    unsigned long size;
    unsigned long size_aligned;
    unsigned long count;
    struct sg_table *sg_table;
    struct semaphore semlock;
   void *user_map_ptr;//mapped_va in user space
   MI_SYS_USER_MAP_CTX user_map_ctx;
   mi_sys_Allocator_t stdAllocator;
}mi_sys_vbpool_allocator_t;
#endif