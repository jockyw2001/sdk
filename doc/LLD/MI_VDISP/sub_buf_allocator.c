#include "sub_buf_allocator.h"

static struct kmem_cache *subbufhead_cache;
static struct kmem_cache *bufhead_cache;
static struct idr subbuf_idr;
static spinlock_t subufallc_lock;

static inline subbufallc_subbufhead_t * _get_empty_subbufhead(void)
{
    subbufallc_subbufhead_t * subhead;
    subhead=kmem_cache_alloc(subbufhead_cache,GFP_KERNEL);
    return subhead;
}
static inline void _put_subbufhead(
    subbufallc_subbufhead_t *subhead)
{
    kmem_cache_free(subbufhead_cache,subhead);
}

static inline subbufallc_subbufallocinfo_t * _allc_info_of_id(
        subbufallc_subbuf_queue_t *bufq,
        int sub_buf_id)
{
    struct list_head *pos;
    subbufallc_subbufallocinfo_t *allcinfo;
    if(sub_buf_id<0)
        return NULL;
    list_for_each(pos, &(bufq->subbufinfo_list)) {
        allcinfo=list_entry(pos, struct subbufallc_subbufallocinfo_t, node);
        if(allcinfo->sub_buf_id == sub_buf_id)
            return allcinfo;
    }
    return NULL;
}
static inline void _vdisp_allocstep(
    subbufallc_subbuf_queue_t *subbufq,
    subbufallc_subbufallocinfo_t *allcinfo)
{
    if(!allcinfo->cur_alloc)
        return;
    if(allcinfo->cur_alloc->node.next==&(subbufq->buf_queue)){
        allcinfo->cur_alloc=NULL;
        list_add_tail(&(allcinfo->outbuf_list),&(subbufq->subbufinfo_out_list)); //will handle in push big buf
    }else
        allcinfo->cur_alloc=list_entry(allcinfo->cur_alloc->node.next, struct subbufallc_bufhead_t,  node);
}

static inline int _vdisp_cmp_pts(
    uint64_t pts,
    uint64_t dstpts,
    uint64_t threshold)
{
    if(pts>=dstpts){
        if((pts-dstpts)<=threshold)
            return 0;
        else
            return 1;
    }else{
        if((dstpts-pts)<=threshold)
            return 0;
        else
            return -1;
    }
}
static inline void _subbufallc_skipalloc(
    subbufallc_subbuf_queue_t *subbufq,
    subbufallc_bufhead_t *buf)
{
    struct list_head *pos;
    subbufallc_subbufallocinfo_t *allcinfo;
    list_for_each(pos, &(subbufq->subbufinfo_list)) {
        allcinfo=list_entry(pos, struct subbufallc_subbufallocinfo_t, node);
        if(allcinfo->cur_alloc==buf){
            _vdisp_allocstep(subbufq, allcinfo);
        }
    }
}
static inline void _subbufallc_releaseallcinfo(
    subbufallc_subbufallocinfo_t *allcinfo)
{
    list_del(&allcinfo->bufout_list);
    list_del(&allcinfo->node);
    kfree(allcinfo);
}
static inline void _subbufallc_unregisterallcinfo(
    subbufallc_subbufallocinfo_t *allcinfo)
{
    allcinfo->sub_buf_id=-1;
    allcinfo->cur_alloc=NULL;
    if(list_empty(&allcinfo->samesub_list)){
        _subbufallc_releaseallcinfo(allcinfo);
    }else {
        allcinfo->bunregisted=1;
        list_del(&allcinfo->bufout_list);
    }
}
static inline void _subbufallc_checkreleaseallcinfo(
    subbufallc_subbufallocinfo_t *allcinfo)
{
    if(allcinfo->bunregisted && list_empty(&(allcinfo->samesub_list))){
        _subbufallc_releaseallcinfo(allcinfo);
    }
}

int subbufallc_init(void)
{
    subbufhead_cache=kmem_cache_create("subbufhead-cache",
                                                            sizeof(subbufallc_subbufhead_t),0,0,NULL);
    if(!subbufhead_cache)
        goto fail_return;
    bufhead_cache=kmem_cache_create("bufhead-cache",
                                                            sizeof(subbufallc_bufhead_t),0,0,NULL);
    if(bufhead_cache)
        goto fail_destroysubbufcache;
    idr_init(&subbuf_idr);
    spin_lock_init(&subufallc_lock);
    return 0;
fail_destroysubbufcache:
    kmem_cache_destroy(subbufhead_cache);
fail_return:
    return -1;
}

int subbufallc_deinit(void)
{
    kmem_cache_destroy(subbufhead_cache);
    kmem_cache_destroy(bufhead_cache);
    idr_destroy(&subbuf_idr);
    return 0;
}

int subbufallc_subbufqinit(
    subbufallc_subbuf_queue_t *subbufq)
{
    INIT_LIST_HEAD(&subbufq->buf_queue);
    INIT_LIST_HEAD(&subbufq->subbufinfo_list);
    INIT_LIST_HEAD(&subbufq->subbufinfo_out_list);
    spin_lock_init(&subbufq->lock);
    subbufq->benable=0;
    subbufq->count=0;
}
int subbufallc_subbufqdestroy(
    subbufallc_subbuf_queue_t *subbufq)
{
    BUG_ON(!list_empty(&subbufq->buf_queue));
    BUG_ON(!list_empty(&subbufq->subbufinfo_list));
    BUG_ON(!list_empty(&subbufq->subbufinfo_out_list));
    BUG_ON(subbufq->count>0);
    return 0;
}

subbufallc_subbufhead_t* subbufallc_alloc(
    subbufallc_subbuf_queue_t *subbufq,
    int sub_buf_id,
    uint64_t u64pts,
    unsigned long flag)
{
    subbufallc_subbufallocinfo_t *allcinfo;
    subbufallc_subbufhead_t *subbufhead=NULL;

    spin_lock(&subbufq->lock);
    if(!subbufq->benable)
        goto alloc_exit;
    allcinfo=_allc_info_of_id(subbufq,sub_buf_id);
    if(!allcinfo || !(allcinfo->cur_alloc) || !(allcinfo->enabled))
        goto alloc_exit;
    if(flag & VDISP_SUBBUF_ALLOC_FLAG_FREERUN)
        goto try_alloc;
    if(0==allcinfo->ptsadjust){
        allcinfo->ptsadjust=allcinfo->cur_alloc->u64pts-u64pts;
    }else{
        int cmpval=_vdisp_cmp_pts(u64pts+allcinfo->ptsadjust, allcinfo->cur_alloc->u64pts, );
        if(0>cmpval){
            if((allcinfo->cur_alloc->u64pts-(u64pts+allcinfo->ptsadjust))>){ //input is slower and over the threshold, readjust the pts
                allcinfo->ptsadjust=allcinfo->cur_alloc->u64pts-u64pts;
            }else
                goto alloc_exit;//input is slower, discard it
        }else if(0<cmpval){ //input is faster, readjust ptsadjustand alloc it
            allcinfo->ptsadjust=allcinfo->cur_alloc->u64pts-u64pts;
        }
    }
try_alloc:
    //alloc it
    subbufhead=_get_empty_subbufhead();
    if(!subbufhead)
        goto alloc_exit;
    subbufhead->buf=allcinfo->cur_alloc;
    subbufhead->suballcinfo=allcinfo;
    if(allcinfo->alloccounter==0)
        subbufhead->type=VIDSP_BUF_TYPE_SUB_FIRST;
    else
        subbufhead->type=VIDSP_BUF_TYPE_SUB;
    list_add_tail(&(subbufhead->node),&(allcinfo->cur_alloc->subbuf_list));
    list_add_tail(&(subbufhead->same_subbuf),&(allcinfo->samesub_list));
    //update allcinfo
    _vdisp_allocstep(subbufq, allcinfo);
    allcinfo->alloccounter ++;
    if(allcinfo->alloccounter==0)allcinfo->alloccounter ++;
alloc_exit:
    spin_unlock(&subbufq->lock);
    return subbufhead;
}
void subbufallc_free(
    subbufallc_subbuf_queue_t *subbufq,
    subbufallc_subbufhead_t* subbufhead)
{
    subbufallc_subbufallocinfo_t *allcinfo=subbufhead->suballcinfo;
    spin_lock(&subbufq->lock);
    list_del(&(subbufhead->node));
    list_del(&(subbufhead->same_subbuf));
    _vdisp_subbuf_allocator_checkrelease_allcinfo(allcinfo);
    _put_subbufhead(subbufhead);
    spin_unlock(&subbufq->lock);
}
int subbufallc_queuebuf(
    subbufallc_subbuf_queue_t *subbufq,
    subbufallc_bufhead_t *buf)
{
    struct list_head *pos;
    subbufallc_subbufallocinfo_t *allcinfo;
    int ret=-1;
    spin_lock(&subbufq->lock);
    list_add_tail(&buf->node,&subbufq->buf_queue);
    INIT_LIST_HEAD(&buf->subbuf_list);

    list_for_each_safe(pos, &(subbufq->subbufinfo_out_list)) {
        allcinfo=list_entry(pos, struct subbufallc_subbufallocinfo_t, bufout_list);
        allcinfo->cur_alloc=buf;
        list_del(&allcinfo->bufout_list);
    }
    subbufq->count++;
    ret=0;
    spin_unlock(&subbufq->lock);
    return ret;
}

//one whole buf only can be dequeued when there's no subbufhead associate with it
subbufallc_bufhead_t *subbufallc_dequeuebuf(
    subbufallc_subbuf_queue_t *subbufq)
{
    subbufallc_bufhead_t *buf=NULL;
    spin_lock(&subbufq->lock);
     if(list_empty(&subbufq->buf_queue))
        goto exit;
    buf=list_first_entry(&subbufq->buf_queue, subbufallc_bufhead_t, node);
    if(!list_empty(&buf->subbuf_list)){ //if has sub buf
        buf=NULL;
        goto exit;
    }

    _subbufallc_skipalloc(subbufq, buf);
    list_del(&buf->node);
    subbufq->count--;
exit:
    spin_unlock(&subbufq->lock);
    return buf;
}

int subbufallc_registersubbuf(
    subbufallc_subbuf_queue_t *subbufq,
    unsigned long x, unsigned long y,
    unsigned long width, unsigned long height,
    int old_subbufid, void *priv)
{
    int id=-1;
    subbufallc_subbufallocinfo_t *allcinfo=NULL, *oldallcinfo=NULL;
    if(width<=0 || height<=0)
        return -1;
skip_check:
    spin_lock(&subbufq->lock);
    if(old_subbufid>0){
        oldallcinfo=_allc_info_of_id(subbufq, old_subbufid);
        if(!oldallcinfo)
            goto exit;
    }
    allcinfo=kmalloc(sizeof(subbufallc_subbufallocinfo_t),GFP_KERNEL);
    if(!allcinfo)
        goto exit;
    id=idr_alloc(&_vdisp_module.subbuf_idr, NULL, 1, 0, GFP_KERNEL);
    if(id<0)
        goto fail_freeallcinfo;
    memset(allcinfo, 0, sizeof(subbufallc_subbufallocinfo_t));
    allcinfo->x=x;
    allcinfo->y=y;
    allcinfo->width=width;
    allcinfo->height=height;
    allcinfo->sub_buf_id=id;
    INIT_LIST_HEAD(&allcinfo->samesub_list);
    INIT_LIST_HEAD(&allcinfo->bufout_list);
    list_add_tail(&allcinfo->node,&(subbufq->subbufinfo_list));
    allcinfo->priv=priv;
    if(oldallcinfo){
        allcinfo->enabled = oldallcinfo->enabled;
        allcinfo->cur_alloc=oldallcinfo->cur_alloc;
        oldallcinfo->cur_alloc=NULL;
        if(!allcinfo->cur_alloc){
            list_add_tail(&allcinfo->bufout_list, &(subbufq->subbufinfo_out_list));
        }
    }else{
        if(list_empty(&subbufq->buf_queue)){
            list_add_tail(&allcinfo->bufout_list, &(subbufq->subbufinfo_out_list));
        }else{
            allcinfo->cur_alloc=list_first_entry(&subbufq->buf_queue, subbufallc_bufhead_t, node);
        }
    }
    if(oldallcinfo){
        idr_free(&_vdisp_module.subbuf_idr,old_subbufid);
        _subbufallc_unregisterallcinfo(oldallcinfo);
    }
    goto exit;
fail_freeallcinfo:
    kfree(allcinfo);
exit:
    spin_unlock(&subbufq->lock);
    return id;
}

void subbufallc_unregistersubbuf(
    subbufallc_subbuf_queue_t *subbufq,
    int sub_buf_id)
{
    subbufallc_subbufallocinfo_t *allcinfo;
    spin_lock(&subbufq->lock);
    allcinfo=_allc_info_of_id(subbufq, sub_buf_id);
    if(!allcinfo)
        goto exit;
    idr_free(&_vdisp_module.subbuf_idr,sub_buf_id);
    _subbufallc_unregisterallcinfo(allcinfo);
exit:
    spin_unlock(&subbufq->lock);
}

int subbufallc_enable(
    subbufallc_subbuf_queue_t *subbufq,
    int sub_buf_id, int enable)
{
    subbufallc_subbufallocinfo_t *allcinfo;
    int ret=-1;
    spin_lock(&subbufq->lock);
    if(sub_buf_id<0){
        ret=subbufq->benable;
        subbufq->benable=!!enable;
        goto exit;
    }
    allcinfo=_allc_info_of_id(subbufq, sub_buf_id);
    if(!allcinfo)
        goto exit;
    ret=allcinfo->enabled;
    allcinfo->enabled=!!enable;
exit:
    spin_unlock(&subbufq->lock);
    return ret;
}

subbufallc_bufhead_t *subbufallc_firstbuf(
    subbufallc_subbuf_queue_t *subbufq)
{
    subbufallc_bufhead_t *buf=NULL;
    spin_lock(&subbufq->lock);
     if(list_empty(&subbufq->buf_queue))
        goto exit;
    buf=list_first_entry(&subbufq->buf_queue, subbufallc_bufhead_t, node);
exit:
    spin_unlock(&subbufq->lock);
    return buf;
}
void subbufallc_skipallocfromfirst(
    subbufallc_subbuf_queue_t *subbufq)
{
    subbufallc_bufhead_t *buf=NULL;
    spin_lock(&subbufq->lock);
    if(list_empty(&subbufq->buf_queue))
        goto exit;
    buf=list_first_entry(&subbufq->buf_queue, subbufallc_bufhead_t, node);
    _subbufallc_skipalloc(subbufq, buf);
exit:
    spin_unlock(&subbufq->lock);
}

subbufallc_subbufhead_t *subbufallc_findsubbuf(
    subbufallc_subbuf_queue_t *subbufq,
    subbufallc_bufhead_t *buf,
    void *allcinfo_priv)
{
    struct list_head *pos;
    subbufallc_subbufallocinfo_t *allcinfo;
    subbufallc_subbufhead_t * subbuf, *ret=NULL;
    spin_lock(&subbufq->lock);
    list_for_each(pos, &(buf->subbuf_list)) {
        subbuf=list_entry(pos, subbufallc_subbufhead_t, node);
        allcinfo=subbuf->suballcinfo;
        if(allcinfo_priv==allcinfo->priv){
            ret=subbuf;
            goto exit;
        }
    }
exit:
    spin_unlock(&subbufq->lock);
    return NULL;
}

subbufallc_bufhead_t * subbufallc_newbufhead(void)
{
    subbufallc_bufhead_t * bufhead;
    bufhead=kmem_cache_alloc(bufhead_cache, GFP_KERNEL);
    return bufhead;
}
void subbufallc_freebufhead(
    subbufallc_bufhead_t * bufhead)
{
    kmem_cache_free(bufhead_cache,bufhead);
}