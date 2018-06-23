#ifndef _MI_SYS_CHUNK__H_
#define _MI_SYS_CHUNK__H_

#define MI_SYS_CHUCK_ALLOC_FAILED ULLONG_MAX

typedef struct mi_sys_chunk_s
{
   int magic;
   //struct list_head free_list; //in freelist
   unsigned long   offset_in_heap;
   unsigned long   length;
   int used;

   mi_sys_chunk_t *prev;
   mi_sys_chunk_t *next;
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

int mi_sys_init_chuck(mi_sys_chunk_mgr_t  *pst_chunk_mgr, unsigned long   size);
void mi_sys_deinit_chuck(mi_sys_chunk_mgr_t  *pst_chunk_mgr);
int mi_sys_alloc_chuck(mi_sys_chunk_mgr_t  *pst_chunk_mgr, unsigned long   size_to_alloc,unsigned long  *offset_in_heap);
void mi_sys_free_chuck(mi_sys_chunk_mgr_t  *pst_chunk_mgr,  unsigned long offset_in_heap);
#endif
