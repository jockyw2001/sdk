#include "mi_sys_chunk_impl.h"
#include <asm/page.h>
#include "mi_sys_debug.h"
#include <linux/slab.h>
#include "mi_sys_impl.h"

#ifdef MI_SYS_PROC_FS_DEBUG
void dump_chunk_mgr(MI_SYS_DEBUG_HANDLE_t  handle, mi_sys_chunk_mgr_t *pst_chunk_mgr)
{
    mi_sys_chunk_t *chunk;
    down(&pst_chunk_mgr->semlock);
    handle.OnPrintOut(handle,"chunk_mgr info:\n");
    handle.OnPrintOut(handle,"%20s%20s%20s\n"
                              ,"offset","length","avail");
    handle.OnPrintOut(handle,"%20lx%20lx%20lx\n",pst_chunk_mgr->offset,pst_chunk_mgr->length,pst_chunk_mgr->avail);

    chunk = pst_chunk_mgr->chunks;
    handle.OnPrintOut(handle,"\neach chunk info:\n");
    handle.OnPrintOut(handle,"%20s%20s%20s%20s\n",
                             "offset","length","used_flag","task_name");
    while (chunk)
    {
        handle.OnPrintOut(handle,"%20lx%20lx%20x%20s\n",chunk->offset, chunk->length,chunk->used
                                                                         ,(chunk->used == 1)?chunk->task_comm:"NA");
        chunk = chunk->next;
    }
    up(&pst_chunk_mgr->semlock);
}
#endif

struct kmem_cache *g_mi_sys_chunk_cachep;

//#define ALLOC_FROM_HIGH_TO_LOW

int mi_sys_init_chunk(mi_sys_chunk_mgr_t  *pst_chunk_mgr, unsigned long   size)
{
    mi_sys_chunk_t *chunk;
    sema_init(&pst_chunk_mgr->semlock, 1);
    down(&pst_chunk_mgr->semlock);

    MI_SYS_ASSERT((size & ~PAGE_MASK)==0);

    chunk = kmem_cache_alloc(g_mi_sys_chunk_cachep, GFP_KERNEL);
    if(!chunk)
    {
        up(&pst_chunk_mgr->semlock);
        DBG_ERR(" fail\n");
        return MI_ERR_SYS_NOMEM;
    }

    pst_chunk_mgr->chunks = chunk;
    pst_chunk_mgr->offset = 0;
    pst_chunk_mgr->length =size;
    pst_chunk_mgr->avail = pst_chunk_mgr->length - pst_chunk_mgr->offset;

    MI_SYS_MAGIC_SET(pst_chunk_mgr,mi_sys_chunk_mgr_t);

    chunk->offset =pst_chunk_mgr->offset;
    chunk->length = pst_chunk_mgr->avail;
    chunk->used = 0;

    MI_SYS_MAGIC_SET(chunk,mi_sys_chunk_t);

    chunk->next = NULL;
    chunk->prev = NULL;

    up(&pst_chunk_mgr->semlock);
    return MI_SUCCESS;
}
void mi_sys_deinit_chunk(mi_sys_chunk_mgr_t  *pst_chunk_mgr)
{
    mi_sys_chunk_t *chunk;
    void *next;
    down(&pst_chunk_mgr->semlock);

    //we should only have one chunk here and which must be in free state
    MI_SYS_BUG_ON(pst_chunk_mgr->avail != pst_chunk_mgr->length- pst_chunk_mgr->offset);
    MI_SYS_BUG_ON(!pst_chunk_mgr->chunks);
    MI_SYS_BUG_ON(pst_chunk_mgr->chunks->used);
    MI_SYS_BUG_ON(pst_chunk_mgr->chunks->next);
    MI_SYS_BUG_ON(pst_chunk_mgr->chunks->offset!=pst_chunk_mgr->offset);
    MI_SYS_BUG_ON(pst_chunk_mgr->chunks->length!=pst_chunk_mgr->avail);

    MI_SYS_MAGIC_CHECK(pst_chunk_mgr,mi_sys_chunk_mgr_t);
    chunk = pst_chunk_mgr->chunks;
    while(chunk)
    {
        next=chunk->next;
        MI_SYS_MAGIC_CLEAR( chunk );
        kmem_cache_free(g_mi_sys_chunk_cachep,chunk);
        chunk = next;
    }

    MI_SYS_MAGIC_CLEAR( pst_chunk_mgr );

    up(&pst_chunk_mgr->semlock);
}

//get from high to low
//here in split,ensure c->used is 0,
//and after split,set newchunk->used be 1 which means later will be used.
static mi_sys_chunk_t *
split_chunk(/* mi_sys_chunk_mgr_t *manager, */mi_sys_chunk_t *chunk, int length )
{
    mi_sys_chunk_t *newchunk;

    MI_SYS_MAGIC_CHECK( chunk, mi_sys_chunk_t );
    MI_SYS_ASSERT(chunk->used == 0);

    if (chunk->length == length)          /* does not need be splitted */
        return chunk;

    MI_SYS_ASSERT(chunk->length> length);
    newchunk = (mi_sys_chunk_t*)kmem_cache_alloc(g_mi_sys_chunk_cachep, GFP_KERNEL);
    if (!newchunk)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

#ifdef ALLOC_FROM_HIGH_TO_LOW
    /* calculate offsets and lengths of resulting chunks */
    newchunk->offset = chunk->offset + chunk->length - length;
    newchunk->length = length;

    newchunk->used = 0;

    chunk->length -= newchunk->length;
#else
    newchunk->offset = chunk->offset +  length;
    newchunk->length = chunk->length - length;

    newchunk->used = 0;

    chunk->length = length;
#endif

    /* insert newchunk after chunk c */
    newchunk->prev = chunk;
    newchunk->next = chunk->next;
    if (chunk->next)
        chunk->next->prev = newchunk;
    chunk->next = newchunk;

    MI_SYS_MAGIC_SET( newchunk, mi_sys_chunk_t );
#ifdef ALLOC_FROM_HIGH_TO_LOW
    return newchunk;
#else
    return chunk;//return chunk,not return newchunk
#endif
}

/** internal functions NOT locking*/
static mi_sys_chunk_t *
occupy_chunk( mi_sys_chunk_mgr_t *pst_chunk_mgr, mi_sys_chunk_t *chunk, int length)
{
    MI_SYS_MAGIC_CHECK( pst_chunk_mgr, mi_sys_chunk_mgr_t );
    MI_SYS_MAGIC_CHECK( chunk, mi_sys_chunk_t );
    MI_SYS_ASSERT(chunk->used == 0);

    chunk = split_chunk(/* pst_chunk_mgr, */chunk, length );
    if (!chunk)
        return NULL;

    MI_SYS_BUG_ON(chunk->used);
    chunk->used = 1;
    memcpy(chunk->task_comm,current->comm,sizeof(current->comm));;
    pst_chunk_mgr->avail -= length;

    return chunk;
}

/*
return value:
MI_SYS_CHUCK_ALLOC_FAILED:means alloc fail
ret_chunk->offset_in_heap :means success,and value is start offset in mma heap.
*/
int mi_sys_alloc_chunk(mi_sys_chunk_mgr_t  *pst_chunk_mgr, unsigned long   size_to_alloc,unsigned long  *offset_in_heap)
{
    mi_sys_chunk_t *chunk;
    mi_sys_chunk_t *ret_chunk;
    mi_sys_chunk_t *best_free = NULL;

#ifdef MI_SYS_PROC_FS_DEBUG
#if 0
    DBG_INFO("alloc dump_chunk_mgr:\n");
    dump_chunk_mgr(pst_chunk_mgr);
#endif
#endif

    down(&pst_chunk_mgr->semlock);
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    MI_SYS_ASSERT(offset_in_heap);

    MI_SYS_MAGIC_CHECK(pst_chunk_mgr,mi_sys_chunk_mgr_t);

    if(pst_chunk_mgr->avail < size_to_alloc)
    {
        MI_SYS_WARN("not enough memory to alloc");
        up(&pst_chunk_mgr->semlock);
        DBG_WRN("%s %d  fail,error avail=0x%lx  size_to_alloc=0x%lx\n",__FUNCTION__,__LINE__,pst_chunk_mgr->avail,size_to_alloc);
        *offset_in_heap = -1;

        DBG_WRN("You can enable following      #if 0     debug code to get more information about each chunk memory is used by which thread\n");
        DBG_WRN("And the follow debug info also can be get from cat /proc/mi_modules/mi_sys_mma/xxx command\n");
        //to avoid printk to much, default not enable following debug code.
        //the follow debug info also can be get from cat /proc/mi_modules/mi_sys_mma/xxx
        #if 0
        DBG_WRN("chunk_mgr info:\n");
        DBG_WRN("offset 0x%lx       length 0x%lx      avail 0x%lx\n",pst_chunk_mgr->offset,pst_chunk_mgr->length,pst_chunk_mgr->avail);

        chunk = pst_chunk_mgr->chunks;
        DBG_WRN("\neach chunk info:\n");
        DBG_WRN("offset    length          used_flag    task_name\n");
        while (chunk)
        {
            DBG_WRN("0x%lx        0x%lx       0x%x     %s\n",chunk->offset, chunk->length,chunk->used
                                                                                 ,(chunk->used == 1)?chunk->task_comm:"NA");
            chunk = chunk->next;
        }
        #endif

        return MI_ERR_SYS_NOMEM;
    }

    chunk = pst_chunk_mgr->chunks;
    MI_SYS_MAGIC_CHECK( chunk, mi_sys_chunk_t );
    //MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    while(chunk->next)
    {
        //MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
        MI_SYS_ASSERT(chunk->offset+chunk->length==chunk->next->offset);
        //MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
        chunk = chunk->next;
        //MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
        MI_SYS_MAGIC_CHECK( chunk, mi_sys_chunk_t );
        //MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    }
    //MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    while (chunk)
    {
        MI_SYS_MAGIC_CHECK( chunk, mi_sys_chunk_t );

        if (!chunk->used && chunk->length >= size_to_alloc)
        {
            /* found a nice place to chill */
            if (!best_free  ||  best_free->length > chunk->length)
            /* first found or better one? */
                best_free = chunk;

            if (chunk->length == size_to_alloc)
                break;
        }
        chunk = chunk->prev;
    }
    //MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    /* if we not found a place */
    if(!best_free)
    {

        MI_SYS_INFO("\n====> the available memory is enough but can not find the free place to allocate memeory!!!!!!!!!\n");
        MI_SYS_INFO("\nthe request memory length is %lu\n",size_to_alloc);
        MI_SYS_INFO("\nthe avail memory is %lu\n",pst_chunk_mgr->avail);
        ret_chunk = NULL;

    }
    else
    {
        //MI_SYS_INFO("%s %d  best_free->offset=0x%lx,best_free->length=0x%lx\n",__FUNCTION__,__LINE__,best_free->offset,best_free->length);
        ret_chunk = occupy_chunk( pst_chunk_mgr, best_free, size_to_alloc);
    }

    if(!ret_chunk)
    {
        //MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
        up(&pst_chunk_mgr->semlock);
        *offset_in_heap = -1;
        DBG_ERR("fail\n");
        return MI_ERR_SYS_NOMEM;
    }

    *offset_in_heap = ret_chunk->offset;
    ///MI_SYS_ERROR( "alloc chunk 0x%08x length0x%08x, %08x(%d)\n", *offset_in_heap, ret_chunk->length, ret_chunk, ret_chunk->used);

    MI_SYS_INFO("%s %d  ret_chunk->offset=0x%lx,ret_chunk->length=0x%lx \n",__FUNCTION__,__LINE__,ret_chunk->offset,ret_chunk->length);
    up(&pst_chunk_mgr->semlock);
    return MI_SUCCESS;
}

void mi_sys_free_chunk(mi_sys_chunk_mgr_t  *pst_chunk_mgr,  unsigned long offset_in_heap)
{
    mi_sys_chunk_t *chunk;

#ifdef MI_SYS_PROC_FS_DEBUG
#if 0
    DBG_INFO("free dump_chunk_mgr:\n");
    dump_chunk_mgr(pst_chunk_mgr);
#endif
#endif

    down(&pst_chunk_mgr->semlock);
    //MI_SYS_ERROR( "try free chunk 0x%08x\n", offset_in_heap);
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    MI_SYS_MAGIC_CHECK(pst_chunk_mgr,mi_sys_chunk_mgr_t);

    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    if((offset_in_heap < pst_chunk_mgr->offset) || (offset_in_heap >  pst_chunk_mgr->offset + pst_chunk_mgr->length -1))
    {
        DBG_ERR(" fail\n");
        MI_SYS_BUG();
    }

    chunk = pst_chunk_mgr->chunks;

    while(chunk)
    {
        MI_SYS_MAGIC_CHECK(chunk,mi_sys_chunk_t);
        //printk("chunk->used=%d   [0x%lx  0x%lx]    offset_in_heap=0x%lx\n",chunk->used,chunk->offset ,chunk->length,offset_in_heap);
        if(offset_in_heap>=chunk->offset+chunk->length)
        {
            chunk = chunk->next;
            continue;
        }
        if(offset_in_heap != chunk->offset)
        {
            DBG_ERR("double free  offset_in_heap=0x%lx , chunk(0x%lx, 0x%lx) used%d\n",
                    offset_in_heap, chunk->offset, chunk->length, chunk->used);
            MI_SYS_BUG();
            up(&pst_chunk_mgr->semlock);
            return;
        }
        if(!chunk->used)
        {
            DBG_ERR("double free  offset_in_heap=0x%lx , free chunk(0x%lx, 0x%lx)\n",
                    offset_in_heap, chunk->offset, chunk->length);
            MI_SYS_BUG();
            up(&pst_chunk_mgr->semlock);
            return;
        }
        break;
    }

    pst_chunk_mgr->avail += chunk->length;
    chunk->used =0;
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);

    //whether can merge with prev
    if (chunk->prev  &&  (0==chunk->prev->used))
    {
        mi_sys_chunk_t *prev = chunk->prev;
        MI_SYS_BUG_ON(prev->offset+prev->length !=chunk->offset);
        MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);

        prev->length += chunk->length;

        prev->next = chunk->next;
        if (prev->next)
            prev->next->prev = prev;


        MI_SYS_MAGIC_CLEAR( chunk );
        kmem_cache_free(g_mi_sys_chunk_cachep,chunk);
        chunk = prev;
        MI_SYS_INFO("%s %d   chunk->offset=0x%lx,chunk->length=0x%lx\n",__FUNCTION__,__LINE__,chunk->offset,chunk->length);
    }

    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);

    //whether can merge with next
    if (chunk->next  &&  (0==chunk->next->used)) {
        mi_sys_chunk_t *next = chunk->next;
        MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);

        MI_SYS_BUG_ON(chunk->offset+chunk->length !=next->offset);

        chunk->length += next->length;

        chunk->next = next->next;
        if (chunk->next)
            chunk->next->prev = chunk;

        MI_SYS_MAGIC_CLEAR( next );

        kmem_cache_free(g_mi_sys_chunk_cachep,next);
        MI_SYS_INFO("%s %d   chunk->offset=0x%lx,chunk->length=0x%lx\n",__FUNCTION__,__LINE__,chunk->offset,chunk->length);
    }

    up(&pst_chunk_mgr->semlock);
    MI_SYS_INFO("%s %d\n",__FUNCTION__,__LINE__);
    return;
}
