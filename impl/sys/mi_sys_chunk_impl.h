#ifndef _MI_SYS_CHUNK_IMPL_H_
#define _MI_SYS_CHUNK_IMPL_H_

#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include "mi_print.h"
#define MI_SYS_CHUCK_ALLOC_FAILED ULLONG_MAX

typedef struct mi_sys_chunk_s
{
    int magic;
    //struct list_head free_list; //in freelist
    unsigned long   offset;
    unsigned long   length;
    int used;
    char task_comm[TASK_COMM_LEN];

    struct mi_sys_chunk_s *prev;
    struct mi_sys_chunk_s *next;
    unsigned char padding[24];
}mi_sys_chunk_t;

typedef struct mi_sys_chunk_mgr_s
{
    int magic;
    mi_sys_chunk_t *chunks;
    unsigned long offset;//manager's offset
    unsigned long   length;//manager's length
    unsigned long avail;

    struct semaphore semlock;
    //struct list_head free_list_head; //in freelist
}mi_sys_chunk_mgr_t;

int mi_sys_init_chunk(mi_sys_chunk_mgr_t  *pst_chunk_mgr, unsigned long   size);
void mi_sys_deinit_chunk(mi_sys_chunk_mgr_t  *pst_chunk_mgr);
int mi_sys_alloc_chunk(mi_sys_chunk_mgr_t  *pst_chunk_mgr, unsigned long   size_to_alloc,unsigned long  *offset_in_heap);
void mi_sys_free_chunk(mi_sys_chunk_mgr_t  *pst_chunk_mgr,  unsigned long offset_in_heap);

#ifdef MI_SYS_PROC_FS_DEBUG
void dump_chunk_mgr(MI_SYS_DEBUG_HANDLE_t  handle, mi_sys_chunk_mgr_t *chunk_mgr);
#endif

#endif
