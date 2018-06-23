struct kmem_cache *g_mi_sys_chunk_cachep;

int mi_sys_init_chuck(mi_sys_chunk_mgr_t  *pst_chunk_mgr, unsigned long   size)
{
    mi_sys_chunk_t *chunk;
    sema_init(&pst_chunk_mgr->semlock, 1);
    down(&pst_chunk_mgr->semlock);

    MI_SYS_ASSERT((size & ~PAGE_MASK)==0);
    
    chuck = kmem_cache_alloc(g_mi_sys_chunk_cachep, GFP_KERNEL);
    if(!chuck)
    {
       up(&pst_chunk_mgr->semlock);
       return MI_ERR_NO_MEM;
    }

    pst_chunk_mgr->chunks = chunk;
    pst_chunk_mgr->offset = 0;
    pst_chunk_mgr->length =size;
    pst_chunk_mgr->avail = pst_chunk_mgr->length - pst_chunk_mgr->offset;

    MI_SYS_MAGIC_SET(pst_chunk_mgr,mi_sys_chunk_mgr_t);

    chunk->offset_in_heap =pst_chunk_mgr->offset;
    chunk->length = pst_chunk_mgr->avail;
    chunk->used = 0;

    MI_SYS_MAGIC_SET(chunk,mi_sys_chunk_t);
    
    up(&pst_chunk_mgr->semlock);
    return MI_SUCCESS;
}
void mi_sys_deinit_chuck(mi_sys_chunk_mgr_t  *pst_chunk_mgr)
{
    mi_sys_chunk_t *chunk;
    void *next;
    down(&pst_chunk_mgr->semlock);

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



static mi_sys_chunk_t *
split_chunk( mi_sys_chunk_mgr_t *manager, mi_sys_chunk_t *c, int length )
{
     mi_sys_chunk_t *newchunk;

     MI_SYS_MAGIC_CHECK( c, mi_sys_chunk_t );

     if (c->length == length)          /* does not need be splitted */
          return c;

     MI_SYS_ASSERT(c->length> length);
     newchunk = (mi_sys_chunk_t*)kmem_cache_alloc(g_mi_sys_chunk_cachep, GFP_KERNEL);
     if (!newchunk) {
          return NULL;
     }

     /* calculate offsets and lengths of resulting chunks */
     newchunk->offset = c->offset + c->length - length;
     newchunk->length = length;

     newchunk->used = c->used;//used flag is same with c
     
     c->length -= newchunk->length;

     /* insert newchunk after chunk c */
     newchunk->prev = c;
     newchunk->next = c->next;
     if (c->next)
          c->next->prev = newchunk;
     c->next = newchunk;

     MI_SYS_MAGIC_SET( newchunk, mi_sys_chunk_t );

     return newchunk;
}

/** internal functions NOT locking*/
static mi_sys_chunk_t *
occupy_chunk( mi_sys_chunk_mgr_t *manager, mi_sys_chunk_t *chunk, int length )
{
     MI_SYS_MAGIC_CHECK( manager, mi_sys_chunk_mgr_t );
     MI_SYS_MAGIC_CHECK( chunk, mi_sys_chunk_t );


     chunk = split_chunk( manager, chunk, length );
     if (!chunk)
          return NULL;

     manager->avail -= length;

     return chunk;
}

/*
return value:
MI_SYS_CHUCK_ALLOC_FAILED:means alloc fail
ret_chunk->offset_in_heap :means success,and value is start offset in mma heap.
*/
int mi_sys_alloc_chuck(mi_sys_chunk_mgr_t  *pst_chunk_mgr, unsigned long   size_to_alloc,unsigned long  *offset_in_heap)
{
    mi_sys_chunk_t *c;
    mi_sys_chunk_t                 *ret_chunk;
    mi_sys_chunk_t *best_free = NULL;
    int ret;
    down(&pst_chunk_mgr->semlock);

    MI_SYS_ASSERT(offset_in_heap);

    MI_SYS_MAGIC_CHECK(pst_chunk_mgr,mi_sys_chunk_mgr_t);

    if(pst_chunk_mgr->avail < size_to_alloc)
    {
               MI_SYS_WARN("not enough memory to alloc");
               up(&pst_chunk_mgr->semlock);
               
               return MI_ERR_NO_MEM;
    }
    
    c = pst_chunk_mgr->chunks;
    MI_SYS_MAGIC_CHECK( c, mi_sys_chunk_t );

    while(c->next)
    {
             MI_SYS_ASSERT(c->offset+c->length==c->next->offset);
             c = c->next;
    }

     while (c) {
                MI_SYS_MAGIC_CHECK( c, mi_sys_chunk_t );

                if (!c->used && c->length >= size_to_alloc) {


                        /* found a nice place to chill */
                        if (!best_free  ||  best_free->length > c->length)
                        /* first found or better one? */
                                best_free = c;
                      //  if(best_free)
                          //  break;
                       if (c->length == size_to_alloc)
                            break;
                 }
                 c = c->prev;
     }

     /* if we not found a place */
     if(!best_free)
     {

         MI_SYS_INFO("\n====> the available memory is enough but can not find the free place to allocate memeory!!!!!!!!!\n");
         MI_SYS_INFO("\nthe request memory length is %d\n",size_to_alloc);
         MI_SYS_INFO("\nthe avail memory is %d\n",pst_chunk_mgr->avail);

     }
     
     
     
     if (best_free) 
     {
        ret_chunk = occupy_chunk( pst_chunk_mgr, best_free, size_to_alloc);
     }
     up(&pst_chunk_mgr->semlock);
     
     if(!ret_chunk)
        return MI_ERR_NO_MEM;

     *offset_in_heap = ret_chunk->offset_in_heap;

     return MI_SUCCESS;
}


void mi_sys_free_chuck(mi_sys_chunk_mgr_t  *pst_chunk_mgr,  unsigned long offset_in_heap)
{
    mi_sys_chunk_t                 *chunk;
    mi_sys_chunk_t                 *next_chunk;
    int   find_inside_chunk =0;
    down(&pst_chunk_mgr->semlock);
    MI_SYS_MAGIC_CHECK(pst_chunk_mgr,mi_sys_chunk_mgr_t);
    

    if((offset_in_heap < pst_chunk_mgr->offset) || (offset_in_heap >  pst_chunk_mgr->offset + pst_chunk_mgr->length -1))
    {
         MI_SYS_BUG();
    }

    chunk = pst_chunk_mgr->chunks;
    MI_SYS_MAGIC_CHECK(chunk,mi_sys_chunk_t);

    while(chunk)
    {
        if(chunk->used)
           && (chunk->offset_in_heap <= offset_in_heap  )
            &&(offset_in_heap < chunk->offset_in_heap +chunk->length))
        {

        
            MI_SYS_ASSERT(offset_in_heap  == chunk->offset_in_heap);
            
            find_inside_chunk =1;
            break;
        }
        else
        {
            chunk = chunk->next;
            MI_SYS_MAGIC_CHECK(chunk,mi_sys_chunk_t);
        }
        
    }

    //not found
    if(!find_inside_chunk)
    {
        MI_SYS_BUG();
        
        up(&pst_chunk_mgr->semlock);
        
        return;
    }


//found,free it.
    pst_chunk_mgr->avail += chunk->length;
    chunk->used =0;

//whether can merge with prev 
     if (chunk->prev  &&  (0==chunk->prev->used)) 
     {
          mi_sys_chunk_t *prev = chunk->prev;


          prev->length += chunk->length;

          prev->next = chunk->next;
          if (prev->next)
               prev->next->prev = prev;


          MI_SYS_MAGIC_CLEAR( chunk );
          kmem_cache_free(g_mi_sys_chunk_cachep,chunk);
          chunk = prev;
     }

     //whether can merge with next 
     if (chunk->next  &&  (false==chunk->next->used)) {
          mi_sys_chunk_t *next = chunk->next;


          chunk->length += next->length;

          chunk->next = next->next;
          if (chunk->next)
               chunk->next->prev = chunk;

          MI_SYS_MAGIC_CLEAR( next );
          
          kmem_cache_free(g_mi_sys_chunk_cachep,next);
     }

    up(&pst_chunk_mgr->semlock);
    return ;
}
