#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/hashtable.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <asm/atomic.h>
#include <asm/string.h>

#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_sys.h"
#include "mi_sys_internal.h"

#include "sub_buf_allocator.h"


static struct kmem_cache *subbufhead_cache;
static struct kmem_cache *bufhead_cache;
static struct idr sba_idr;

static inline sba_subbufhead_t * _get_empty_subbufhead(void)
{
    sba_subbufhead_t * subhead;
    subhead=kmem_cache_alloc(subbufhead_cache,GFP_KERNEL);
    return subhead;
}
static inline void _put_subbufhead(
    sba_subbufhead_t *subhead)
{
    kmem_cache_free(subbufhead_cache,subhead);
}

static inline sba_subbufallocinfo_t * _allc_info_of_id(
        sba_buf_queue_t *bufq,
        int sub_buf_id)
{
    struct list_head *pos;
    sba_subbufallocinfo_t *allcinfo;
    if(sub_buf_id<0)
        return NULL;
    list_for_each(pos, &(bufq->subbufinfo_list)) {
        allcinfo=list_entry(pos, sba_subbufallocinfo_t, node);
        if(allcinfo->sub_buf_id == sub_buf_id)
            return allcinfo;
    }
    return NULL;
}
static inline void _vdisp_allocstep(
    sba_buf_queue_t *bufq,
    sba_subbufallocinfo_t *allcinfo)
{
    if(!allcinfo->cur_alloc)
        return;
try_next:
    if(allcinfo->cur_alloc->node.next==&(bufq->buf_queue)){
        allcinfo->cur_alloc=NULL;
        list_add_tail(&(allcinfo->pending_list_node),&(bufq->subbufinfo_pending_list)); //will handle in push big buf
    }else{
        allcinfo->cur_alloc=list_entry(allcinfo->cur_alloc->node.next, sba_bufhead_t,  node);
        if(allcinfo->cur_alloc->flags & SBA_BUFHEAD_FLAG_SKIP){
            goto try_next;
        }
    }
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
static inline void _sba_skipalloc(
    sba_buf_queue_t *bufq,
    sba_bufhead_t *buf)
{
    struct list_head *pos;
    sba_subbufallocinfo_t *allcinfo;
    list_for_each(pos, &(bufq->subbufinfo_list)) {
        allcinfo=list_entry(pos, sba_subbufallocinfo_t, node);
        if(allcinfo->cur_alloc==buf){
            _vdisp_allocstep(bufq, allcinfo);
        }
    }
}
static inline void _sba_releaseallcinfo(
    sba_subbufallocinfo_t *allcinfo)
{
    list_del(&allcinfo->pending_list_node);
    list_del(&allcinfo->node);
    kfree(allcinfo);
}
static inline void _sba_unregisterallcinfo(
    sba_subbufallocinfo_t *allcinfo)
{
    allcinfo->sub_buf_id=-1;
    allcinfo->cur_alloc=NULL;
    if(list_empty(&allcinfo->allcinfo_subbuf_list)){
        //该allocinfo上已经没有分配的子buf，则直接销毁
        _sba_releaseallcinfo(allcinfo);
    }else {
        //该allocinfo上还有已经分配的子buf，需要延迟销毁
        //在该allocinfo上已经分配的子buf全部释放时会销毁它
        allcinfo->bunregisted=1;
        list_del(&allcinfo->pending_list_node);
        INIT_LIST_HEAD(&allcinfo->pending_list_node);
    }
}
static inline void _sba_checkreleaseallcinfo(
    sba_subbufallocinfo_t *allcinfo)
{
    if(allcinfo->bunregisted && list_empty(&(allcinfo->allcinfo_subbuf_list))){
        _sba_releaseallcinfo(allcinfo);
    }
}

unsigned long sba_subbufvalidwidth(sba_subbufhead_t *subbuf)
{
    unsigned long x0,w0,w1;
    x0=sba_subbufx(subbuf);
    w0=sba_subbufwidth(subbuf);
    w1=subbuf->buf->width;
    if(x0>=w1){
        return 0;
    }else if((x0+w0)>w1){
        return w1-x0;
    }else{
        return w0;
    }
}
unsigned long sba_subbufvalidheight(sba_subbufhead_t *subbuf)
{
    unsigned long y0,h0,h1;
    y0=sba_subbufy(subbuf);
    h0=sba_subbufheight(subbuf);
    h1=subbuf->buf->height;
    if(y0>=h1){
        return 0;
    }else if((y0+h0)>h1){
        return h1-y0;
    }else{
        return h0;
    }
}

int sba_init(void)
{
    subbufhead_cache=kmem_cache_create("subbufhead-cache",
                                                            sizeof(sba_subbufhead_t),0,SLAB_HWCACHE_ALIGN,NULL);
    if(!subbufhead_cache)
        goto fail_return;
    bufhead_cache=kmem_cache_create("bufhead-cache",
                                                            sizeof(sba_bufhead_t),0,SLAB_HWCACHE_ALIGN,NULL);
    if(!bufhead_cache)
        goto fail_destroysubbufcache;
    idr_init(&sba_idr);
    return 0;
fail_destroysubbufcache:
    kmem_cache_destroy(subbufhead_cache);
fail_return:
    return -1;
}

int sba_deinit(void)
{
    kmem_cache_destroy(subbufhead_cache);
    kmem_cache_destroy(bufhead_cache);
    idr_destroy(&sba_idr);
    return 0;
}

int sba_bufqinit(
    sba_buf_queue_t *bufq)
{
    INIT_LIST_HEAD(&bufq->buf_queue);
    INIT_LIST_HEAD(&bufq->subbufinfo_list);
    INIT_LIST_HEAD(&bufq->subbufinfo_pending_list);
    mutex_init(&bufq->mtx);
    bufq->benable=0;
    bufq->count=0;
    bufq->next_buf_id=1;
    return 0;
}
int sba_bufqdestroy(
    sba_buf_queue_t *bufq)
{
    MI_SYS_BUG_ON(!list_empty(&bufq->buf_queue));
    MI_SYS_BUG_ON(!list_empty(&bufq->subbufinfo_list));
    MI_SYS_BUG_ON(!list_empty(&bufq->subbufinfo_pending_list));
    MI_SYS_BUG_ON(bufq->count>0);
    mutex_destroy(&bufq->mtx);
    return 0;
}
#define RESYNC_TOLERANCE 1000000
sba_subbufhead_t* sba_alloc(
    sba_buf_queue_t *bufq,
    int sub_buf_id,
    uint64_t u64pts,
    unsigned long flag)
{
    sba_subbufallocinfo_t *allcinfo;
    sba_subbufhead_t *subbufhead=NULL;

    mutex_lock(&bufq->mtx);
    if(!bufq->benable)
        goto alloc_exit;
    //获取allocinfo
    allcinfo=_allc_info_of_id(bufq,sub_buf_id);
    if(!allcinfo || !(allcinfo->cur_alloc) || !(allcinfo->enabled))
        goto alloc_exit;
    //如果是freerun则不比较pts，直接分配
    if(flag & VDISP_SUBBUF_ALLOC_FLAG_FREERUN)
        goto try_alloc;
    if(INT_MAX==allcinfo->ptsadjust){
        //第一次分配，设置pts差值，并分配
        allcinfo->ptsadjust=allcinfo->cur_alloc->u64pts-u64pts;
    }else{
        //比较pts
        int cmpval=_vdisp_cmp_pts(u64pts+allcinfo->ptsadjust,
                                                  allcinfo->cur_alloc->u64pts,
                                                  allcinfo->cur_alloc->pts_tolerance);
        //input pts小于output pts
        if(0>cmpval){
            //pts差值太大，需要重新调整pts差值并分配
            if(allcinfo->unsync_cnt>30 ||
                (allcinfo->cur_alloc->u64pts-(u64pts+allcinfo->ptsadjust))>RESYNC_TOLERANCE){
                allcinfo->ptsadjust=allcinfo->cur_alloc->u64pts-u64pts;
            }else{
                allcinfo->unsync_cnt ++;
                goto alloc_exit;//忽略当前分配需求
            }
        }else if(0<cmpval){
            //input pts大于output pts，需要重新调整pts差值并分配
            allcinfo->ptsadjust=allcinfo->cur_alloc->u64pts-u64pts;
        }
    }
    allcinfo->unsync_cnt=0;
try_alloc:
    //进行分配
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
    list_add_tail(&(subbufhead->allcinfo_subbuf_node),&(allcinfo->allcinfo_subbuf_list));
    //分配指针后移
    _vdisp_allocstep(bufq, allcinfo);
    allcinfo->alloccounter ++;
    if(allcinfo->alloccounter==0)allcinfo->alloccounter ++;
alloc_exit:
    mutex_unlock(&bufq->mtx);
    return subbufhead;
}
void sba_free(
    sba_buf_queue_t *bufq,
    sba_subbufhead_t* subbufhead)
{
    sba_subbufallocinfo_t *allcinfo=subbufhead->suballcinfo;
    mutex_lock(&bufq->mtx);
    list_del(&(subbufhead->node));
    list_del(&(subbufhead->allcinfo_subbuf_node));
    _sba_checkreleaseallcinfo(allcinfo);
    _put_subbufhead(subbufhead);
    mutex_unlock(&bufq->mtx);
}
int sba_queuebuf(
    sba_buf_queue_t *bufq,
    sba_bufhead_t *buf)
{
    struct list_head *pos, *n;
    sba_subbufallocinfo_t *allcinfo;
    int ret=-1;
    mutex_lock(&bufq->mtx);
    buf->flags=0;
    buf->buf_id=bufq->next_buf_id++;
    list_add_tail(&buf->node,&bufq->buf_queue);
    INIT_LIST_HEAD(&buf->subbuf_list);
    list_for_each_safe(pos, n, &(bufq->subbufinfo_pending_list)) {
        allcinfo=list_entry(pos, sba_subbufallocinfo_t, pending_list_node);
        allcinfo->cur_alloc=buf;
        list_del(&allcinfo->pending_list_node);
        INIT_LIST_HEAD(&allcinfo->pending_list_node);
    }
    bufq->count++;
    ret=0;
    mutex_unlock(&bufq->mtx);
    return ret;
}

//one whole buf only can be dequeued when there's no subbufhead associate with it
sba_bufhead_t *sba_dequeuebuf(
    sba_buf_queue_t *bufq)
{
    sba_bufhead_t *buf=NULL;
    mutex_lock(&bufq->mtx);
     if(list_empty(&bufq->buf_queue))
        goto exit;
    buf=list_first_entry(&bufq->buf_queue, sba_bufhead_t, node);
    if(!list_empty(&buf->subbuf_list)){ //if has sub buf
        buf=NULL;
        goto exit;
    }
    //将所有指向该大buf的分配指针后移
    _sba_skipalloc(bufq, buf);
    list_del(&buf->node);
    INIT_LIST_HEAD(&buf->node);
    bufq->count--;
exit:
    mutex_unlock(&bufq->mtx);
    return buf;
}

int sba_registersubbuf(
    sba_buf_queue_t *bufq,
    unsigned long x, unsigned long y,
    unsigned long width, unsigned long height,
    int old_subbufid, void *priv)
{
    int id=-1;
    sba_subbufallocinfo_t *allcinfo=NULL, *oldallcinfo=NULL;
    if(width<=0 || height<=0)
        return -1;
    mutex_lock(&bufq->mtx);
    //是否需要替换已有的allocinfo
    if(old_subbufid>=0){
        oldallcinfo=_allc_info_of_id(bufq, old_subbufid);
        if(!oldallcinfo)
            goto exit;
    }
    //分配新的allocinfo
    allcinfo=kmalloc(sizeof(sba_subbufallocinfo_t),GFP_KERNEL);
    if(!allcinfo)
        goto exit;
    id=idr_alloc(&sba_idr, NULL, 1, 0, GFP_KERNEL);
    if(id<0)
        goto fail_freeallcinfo;
    memset(allcinfo, 0, sizeof(sba_subbufallocinfo_t));
    allcinfo->x=x;
    allcinfo->y=y;
    allcinfo->width=width;
    allcinfo->height=height;
    allcinfo->sub_buf_id=id;
    INIT_LIST_HEAD(&allcinfo->allcinfo_subbuf_list);
    INIT_LIST_HEAD(&allcinfo->pending_list_node);
    list_add_tail(&allcinfo->node,&(bufq->subbufinfo_list));
    allcinfo->priv=priv;
    if(oldallcinfo){
        //需要继承已有的allocinfo的分配指针
        allcinfo->enabled = oldallcinfo->enabled;
        allcinfo->cur_alloc=oldallcinfo->cur_alloc;
        allcinfo->ptsadjust=oldallcinfo->ptsadjust;
        oldallcinfo->cur_alloc=NULL;
        if(!allcinfo->cur_alloc){
            list_add_tail(&allcinfo->pending_list_node, &(bufq->subbufinfo_pending_list));
        }
    }else{
        allcinfo->ptsadjust=INT_MAX;
        if(list_empty(&bufq->buf_queue)){
            list_add_tail(&allcinfo->pending_list_node, &(bufq->subbufinfo_pending_list));
        }else{
            sba_bufhead_t *bufhead=list_first_entry(&bufq->buf_queue, sba_bufhead_t, node);
            allcinfo->cur_alloc=bufhead;
            if(bufhead->flags & SBA_BUFHEAD_FLAG_SKIP){
                _vdisp_allocstep(bufq, allcinfo);
            }
        }
    }
    if(oldallcinfo){
        //取消注册被替换的allocinfo
        idr_remove(&sba_idr,old_subbufid);
        _sba_unregisterallcinfo(oldallcinfo);
    }
    goto exit;
fail_freeallcinfo:
    kfree(allcinfo);
exit:
    mutex_unlock(&bufq->mtx);
    return id;
}

void sba_unregistersubbuf(
    sba_buf_queue_t *bufq,
    int sub_buf_id)
{
    sba_subbufallocinfo_t *allcinfo;
    mutex_lock(&bufq->mtx);
    allcinfo=_allc_info_of_id(bufq, sub_buf_id);
    if(!allcinfo)
        goto exit;
    idr_remove(&sba_idr,sub_buf_id);
    _sba_unregisterallcinfo(allcinfo);
exit:
    mutex_unlock(&bufq->mtx);
}

int sba_enable(
    sba_buf_queue_t *bufq,
    int sub_buf_id, int enable)
{
    sba_subbufallocinfo_t *allcinfo;
    int ret=-1;
    mutex_lock(&bufq->mtx);
    if(sub_buf_id<0){
        ret=bufq->benable;
        bufq->benable=!!enable;
        goto exit;
    }
    allcinfo=_allc_info_of_id(bufq, sub_buf_id);
    if(!allcinfo)
        goto exit;
    ret=allcinfo->enabled;
    allcinfo->enabled=!!enable;
exit:
    mutex_unlock(&bufq->mtx);
    return ret;
}

sba_bufhead_t *sba_firstbuf(
    sba_buf_queue_t *bufq)
{
    sba_bufhead_t *buf=NULL;
    mutex_lock(&bufq->mtx);
     if(list_empty(&bufq->buf_queue))
        goto exit;
    buf=list_first_entry(&bufq->buf_queue, sba_bufhead_t, node);
exit:
    mutex_unlock(&bufq->mtx);
    return buf;
}
void sba_skipallocfromfirst(
    sba_buf_queue_t *bufq)
{
    sba_bufhead_t *buf=NULL;
    mutex_lock(&bufq->mtx);
    if(list_empty(&bufq->buf_queue))
        goto exit;
    buf=list_first_entry(&bufq->buf_queue, sba_bufhead_t, node);
    buf->flags |= SBA_BUFHEAD_FLAG_SKIP;
    _sba_skipalloc(bufq, buf);
exit:
    mutex_unlock(&bufq->mtx);
}

sba_subbufhead_t *sba_findsubbuf(
    sba_buf_queue_t *bufq,
    sba_bufhead_t *buf,
    void *allcinfo_priv)
{
    struct list_head *pos;
    sba_subbufallocinfo_t *allcinfo;
    sba_subbufhead_t * subbuf, *ret=NULL;
    if(buf==NULL){
        DBG_ERR("Invalid operation!Try to find subbuf in a NULL buf!!\n");
        return NULL;
    }
    mutex_lock(&bufq->mtx);
    list_for_each(pos, &(buf->subbuf_list)) {
        subbuf=list_entry(pos, sba_subbufhead_t, node);
        allcinfo=subbuf->suballcinfo;
        if(allcinfo_priv==allcinfo->priv){
            ret=subbuf;
            goto exit;
        }
    }
exit:
    mutex_unlock(&bufq->mtx);
    return ret;
}

sba_bufhead_t * sba_newbufhead(void)
{
    sba_bufhead_t * bufhead;
    bufhead=kmem_cache_alloc(bufhead_cache, GFP_KERNEL);
    return bufhead;
}
void sba_freebufhead(
    sba_bufhead_t * bufhead)
{
    kmem_cache_free(bufhead_cache,bufhead);
}

unsigned long sba_getnextbufid(
    sba_buf_queue_t *bufq,
    int subbufid)
{
    unsigned long id=(unsigned long)(-1);
    sba_subbufallocinfo_t *allcinfo=NULL;
    mutex_lock(&bufq->mtx);
    if(subbufid>=0){
        allcinfo=_allc_info_of_id(bufq, subbufid);
        if(!allcinfo)
            goto exit;
    }
    if(allcinfo && allcinfo->cur_alloc)
        id=allcinfo->cur_alloc->buf_id;
    else
        id=bufq->next_buf_id;
exit:
    mutex_unlock(&bufq->mtx);
    return id;
}