#ifndef _MI_SYS_VB_POOL_IMPL_H_
#define _MI_SYS_VB_POOL_IMPL_H_
struct mi_sys_vbpool_allocator_t;
#include "mi_sys_buf_mgr.h"
#include "mi_sys_mma_heap_impl.h"

#define __MI_SYS_VBPOOL_ALLOCATOR_MAGIC_NUM__ 0x5642504C

typedef struct mi_sys_vbpool_allocation_s
{
    struct list_head list_in_all;
    struct list_head list_in_free;
    struct mi_sys_vbpool_allocator_s *pstParentAllocator;
    MI_S32 offset_in_vb_pool;
    MI_SYS_BufferAllocation_t *pst_mma_allocation;
    MI_SYS_BufferAllocation_t stdBufAllocation;
    unsigned char padding[8];
}mi_sys_vbpool_allocation_t;

typedef struct mi_sys_vbpool_allocator_s
{
    unsigned int u32MagicNumber;
    struct list_head list_of_all_allocations;//list in global mma_allocator list
    struct list_head list_of_free_allocations;//list in global mma_allocator list
    unsigned long size;
    unsigned long count;
    struct sg_table *sg_table;
    void *kern_map_ptr;
    struct semaphore semlock;
    mi_sys_Allocator_t stdAllocator;
}mi_sys_vbpool_allocator_t;

mi_sys_Allocator_t *mi_sys_vbpool_allocator_create(const char *mma_heap_name, unsigned long size, unsigned long count);

#ifdef MI_SYS_PROC_FS_DEBUG
void _vbpool_deal_with_mi_sys_Allocator_t(MI_SYS_DEBUG_HANDLE_t  handle,mi_sys_Allocator_t * pstAllocator);
MI_S32 mi_sys_vbpool_allocator_dump_data(char *dir_and_file_name,struct mi_sys_Allocator_s *pstAllocator,MI_U32 offset_in_allocator,MI_U32 length);
MI_S32 mi_sys_vbpool_allocator_get_info(struct mi_sys_Allocator_s *pstAllocator,MI_U32 *size,MI_U32 *count);
#endif

#endif
