#ifndef _MI_SYS_META_H_
#define _MI_SYS_META_H_
#define __MI_SYS_META_ALLOCATOR_MAGIC_NUM__ 0x4D455441
//#define  MI_SYS_MMA_HEAP_NAME_MAX_LEN 256

    

struct mi_sys_meta_allocator_s;
typedef struct mi_sys_meta_buf_allocation_s
{
   struct list_head list_in_free_list;
   struct list_head list_in_all_list;
   phys_addr_t  phy_addr;
   void * va_in_kern;//kernel space va
   unsigned long   length;
   unsigned long offset_in_pool;
   mi_sys_meta_allocator_s *pstmetaAllocator;
   MI_SYS_BufferAllocation_t stdBufAllocation;
}mi_sys_meta_buf_allocation_t;


#define MI_SYS_META_DATA_SIZE_MAX 256
#define MI_SYS_META_DATA_COUNT_QUOTA 256
#define MI_SYS_META_DATA_PAGE_COUNT (ALIGN_UP(MI_SYS_META_DATA_SIZE_MAX*MI_SYS_META_DATA_COUNT_QUOTA,PAGE_SIZE)/PAGE_SIZE)


typedef struct mi_sys_meta_allocator_s
{
    unsigned long u32MagicNumber;
    struct semaphore semlock;
    struct list_head list_of_free_allocations;
    struct list_head list_of_all_allocations;
    
    void *user_map_ptr;//mapped_va in user space
    MI_SYS_USER_MAP_CTX user_map_ctx;  
    struct page  *pages[MI_SYS_META_DATA_PAGE_COUNT];
    
    mi_sys_Allocator_t stdAllocator;
     struct sg_table *sg_table;
}mi_sys_meta_allocator_t;


#endif
