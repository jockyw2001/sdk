#ifndef _MI_SYS_MMA_HEAP_H_
#define _MI_SYS_MMA_HEAP_H_

#include "mi_sys.h"
#include "mi_sys_buf_mgr.h"
#include "mi_sys_chunk_impl.h"
#include <linux/hashtable.h>
#include "mi_print.h"
#include <linux/debugfs.h>

#define __MI_SYS_MMA_ALLOCATOR_MAGIC_NUM__ 0x484D4D41
#define  MI_SYS_MMA_HEAP_NAME_MAX_LEN 256

struct mi_sys_mma_allocator_s;
typedef struct mi_sys_mma_buf_allocation_s
{
    unsigned long   offset_in_heap;
    unsigned long   length;
    struct mi_sys_mma_allocator_s *pstMMAAllocator;
    MI_SYS_BufferAllocation_t stdBufAllocation;
    struct hlist_node   hentry;
    char  *kern_vmap_ptr;
    unsigned char padding[8];
}mi_sys_mma_buf_allocation_t;

typedef struct mi_sys_mma_allocator_s
{
    struct list_head list;//list in global g_mma_allocator list
    unsigned int u32MagicNumber;
    unsigned long length;
    unsigned long long heap_base_cpu_bus_addr;
    mi_sys_chunk_mgr_t chunk_mgr;
    mi_sys_Allocator_t stdAllocator;
    char heap_name[MI_SYS_MMA_HEAP_NAME_MAX_LEN];
    struct semaphore semlock;
}mi_sys_mma_allocator_t;

MI_SYS_BufferAllocation_t *mi_sys_alloc_from_mma_allocators( const char *mma_heap_name, MI_SYS_BufConf_t *pstBufConfig);
MI_SYS_BufferAllocation_t* mi_sys_mma_find_allocation_by_pa(    unsigned long  phy_addr);
mi_sys_mma_allocator_t *mi_sys_mma_allocator_create(const char *mma_heap_name, unsigned long long cpu_bus_addr, unsigned long length);

#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 mi_sys_mma_allocator_proc_dump_attr(MI_SYS_DEBUG_HANDLE_t  handle,void *private);
MI_S32 mi_sys_mma_allocator_proc_exec_cmd(MI_SYS_DEBUG_HANDLE_t  handle,allocator_echo_cmd_info_t *cmd_info,void *private);
int mma_heap_info_open(struct inode *inode, struct file *file);
MI_U32 debugfs_deal_with_mma_heap_info(struct dentry *debugfs_dir_mma_heap);
MI_U32 debugfs_deal_with_mma_heap_info_finish(void);
#endif

#endif
