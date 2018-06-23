#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/hashtable.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <asm/atomic.h>
#include <asm/string.h>

#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_sys.h"
#include "mi_sys_internal.h"

#include "mi_vdisp.h"
#include "sub_buf_allocator.h"
#include "mi_vdisp_impl.h"
#include "vdisp_custom_allocator.h"
#include "vdisp_customer_allocator_usermap.h"


static struct kmem_cache *allocation_memcache=NULL;
static DEFINE_MUTEX(allocation_hashtbl_mtx);
static DEFINE_HASHTABLE(allocation_hashtbl, 8);

inline static void _vdisp_add_allocation(vdisp_allocation_t *allocation)
{
    mutex_lock(&allocation_hashtbl_mtx);
    hash_add(allocation_hashtbl, &allocation->node, allocation->phys[0]);
    mutex_unlock(&allocation_hashtbl_mtx);
}
inline static void _vdisp_remove_allocation(vdisp_allocation_t *allocation)
{
    mutex_lock(&allocation_hashtbl_mtx);
    hash_del(&allocation->node);
    mutex_unlock(&allocation_hashtbl_mtx);
}
inline static vdisp_allocation_t * _vdisp_find_allocation(phys_addr_t *phys, int cnt)
{
    vdisp_allocation_t *allocation=NULL;
    if(cnt<=0)return NULL;
    mutex_lock(&allocation_hashtbl_mtx);
    hash_for_each_possible(allocation_hashtbl, allocation, node, phys[0]){
        if(allocation->phys[0]!=phys[0])
            continue;
        if(cnt>=2 && allocation->phys[1]!=phys[1])
            continue;
        if(cnt>=3 && allocation->phys[2]!=phys[2])
            continue;
        goto exit;
    }
exit:
    mutex_unlock(&allocation_hashtbl_mtx);
    return allocation;
}

static void vdisp_allcator_release(vdisp_allocator_t* allocator)
{
    vdisp_device_t *dev=MI_VDISP_IMPL_GetDevice(allocator->port->u32DevId);
#if VDISP_CUSALLOCATOR_DBG
    vdisp_dbg_rmv_allocator(allocator);
#endif
    atomic_dec(&dev->allocator_cnt);
    mutex_unlock(&allocator->mtx);
    mutex_destroy(&allocator->mtx);
    kfree(allocator);
}
#if 0
static void vdisp_bufallocation_OnRef(struct MI_SYS_BufferAllocation_s *thiz)
{
    generic_allocation_on_ref(thiz);
}
static void vdisp_bufallocation_OnUnref(struct MI_SYS_BufferAllocation_s *thiz)
{
    generic_allocation_on_unref(thiz);
}
#endif
static void vdisp_bufallocation_OnRelease(struct MI_SYS_BufferAllocation_s *thiz)
{
    vdisp_allocation_t *allocation=container_of(thiz, vdisp_allocation_t, sys_allocation);
    vdisp_allocator_t *allocator=allocation->allocator;
    vdisp_device_t *dev=NULL;
    vdisp_inputport_t *inputport=NULL;
    mutex_lock(&allocator->mtx);
    inputport=container_of(allocator->port, vdisp_inputport_t, inputport);
    _vdisp_remove_allocation(allocation);
#if VDISP_CUSALLOCATOR_DBG
    vdisp_dbg_rmv_allocation(allocation);
#endif
    dev=MI_VDISP_IMPL_GetDevice(inputport->inputport.u32DevId);
    sba_free(&dev->bufqueue, allocation->subbufhd);
    kmem_cache_free(allocation_memcache, allocation);
    allocator->allocation_cnt--;
    if(allocator->allocation_cnt==0 && allocator->breleased)
        vdisp_allcator_release(allocator);
    else{
        mutex_unlock(&allocator->mtx);
    }
}
static void *vdisp_bufallocation_map_user(struct MI_SYS_BufferAllocation_s *thiz)
{
    vdisp_allocation_t *allocation=container_of(thiz, vdisp_allocation_t, sys_allocation);
    vdisp_allocator_t *allocator=allocation->allocator;
    void *va=NULL;
    int total_len=0;
    MI_SYS_BufInfo_t *bufinfo;
    mutex_lock(&allocator->mtx);
    bufinfo=MI_VDISP_IMPL_GetBufInfoFromBufhead(allocation->subbufhd->buf);
    BUG_ON(!bufinfo);
    total_len=MI_VDISP_IMPL_GetBufsizeFromBufinfo(bufinfo);
    BUG_ON(total_len == 0);
    va=vdisp_map_usr(bufinfo->stFrameData.phyAddr[0], total_len);
    if(va){
        va=((char*)(va))+((MI_PHY)(allocation->phys[0])-bufinfo->stFrameData.phyAddr[0]);
    }
    mutex_unlock(&allocator->mtx);
    return va;
}
static void  vdisp_bufallocation_unmap_user(struct MI_SYS_BufferAllocation_s *thiz)
{
    vdisp_allocation_t *allocation=container_of(thiz, vdisp_allocation_t, sys_allocation);
    vdisp_allocator_t *allocator=allocation->allocator;
    MI_SYS_BufInfo_t *bufinfo;
    int ret=-1;
    mutex_lock(&allocator->mtx);
    bufinfo=MI_VDISP_IMPL_GetBufInfoFromBufhead(allocation->subbufhd->buf);
    BUG_ON(!bufinfo);
    ret=vdisp_unmap_usr(bufinfo->stFrameData.phyAddr[0]);
    mutex_unlock(&allocator->mtx);
}
static void *vdisp_bufallocation_vmap_kern(struct MI_SYS_BufferAllocation_s *thiz)
{
    return NULL;
}
static void vdisp_bufallocation_vunmap_kern(struct MI_SYS_BufferAllocation_s *thiz)
{
}

static buf_allocation_ops_t vdisp_bufallcops=
{
    .OnRef=generic_allocation_on_ref,//vdisp_bufallocation_OnRef,
    .OnUnref=generic_allocation_on_unref,//vdisp_bufallocation_OnUnref,
    .OnRelease=vdisp_bufallocation_OnRelease,
    .map_user=vdisp_bufallocation_map_user,
    .unmap_user=vdisp_bufallocation_unmap_user,
    .vmap_kern=vdisp_bufallocation_vmap_kern,
    .vunmap_kern=vdisp_bufallocation_vunmap_kern,
};
#if 0
static MI_S32 vdisp_allocator_OnRef(struct mi_sys_Allocator_s *thiz)
{
    generic_allocator_on_ref(thiz);
    return 0;
}
static MI_S32 vdisp_allocator_OnUnref(struct mi_sys_Allocator_s *thiz)
{
    generic_allocator_on_unref(thiz);
    return 0;
}
#endif
static MI_S32 vdisp_allocator_OnRelease(struct mi_sys_Allocator_s *thiz)
{
    vdisp_allocator_t *allocator=container_of(thiz, vdisp_allocator_t, sys_allocator);
    mutex_lock(&allocator->mtx);
    allocator->breleased=1;
    if(allocator->allocation_cnt==0)
    {
        vdisp_allcator_release(allocator);
        //mutex_unlock(&allocator->mtx); //vdisp_allcator_release already destory mutex
    }
    else{
        mutex_unlock(&allocator->mtx);
    }
    return 0;
}
static MI_SYS_BufferAllocation_t *vdisp_allocator_alloc(mi_sys_Allocator_t *pstAllocator,
                                   MI_SYS_BufConf_t *stBufConfig )
{
    vdisp_allocator_t *allocator=container_of(pstAllocator, vdisp_allocator_t, sys_allocator);
    vdisp_allocation_t *allocation=NULL;
    MI_SYS_BufferAllocation_t *ret=NULL;
    vdisp_device_t *dev=NULL;
    vdisp_inputport_t *inputport=NULL;
    sba_subbufhead_t* subbufhd=NULL;
    MI_SYS_BufInfo_t *bufinfo;
    mutex_lock(&allocator->mtx);
    if(allocator->breleased)
        goto exit;
    inputport=container_of(allocator->port, vdisp_inputport_t, inputport);
    dev=MI_VDISP_IMPL_GetDevice(inputport->inputport.u32DevId);
    if(inputport->sub_buf_id<0){
        goto exit;
    }
    allocation=kmem_cache_alloc(allocation_memcache, GFP_KERNEL);
    if(!allocation)
        goto exit;
    subbufhd=sba_alloc(&dev->bufqueue, inputport->sub_buf_id,
        stBufConfig->u64TargetPts, inputport->attr.s32IsFreeRun?VDISP_SUBBUF_ALLOC_FLAG_FREERUN:0);
    if(!subbufhd)
        goto free_allocation;
    allocator->allocation_cnt++;
    allocation->subbufhd=subbufhd;
    allocation->allocator=allocator;
    allocation->magic=VDISP_CUSALLOCATION_MAGIC;
    generic_allocation_init(&allocation->sys_allocation, &vdisp_bufallcops);
    bufinfo=&allocation->sys_allocation.stBufInfo;
    MI_VDISP_IMPL_InitBufInfoFromSubBuf(bufinfo, subbufhd, stBufConfig->u64TargetPts);
    allocation->phys[0]=bufinfo->stFrameData.phyAddr[0];
    allocation->phys[1]=bufinfo->stFrameData.phyAddr[1];
    allocation->phys[2]=bufinfo->stFrameData.phyAddr[2];
    _vdisp_add_allocation(allocation);
    ret = &allocation->sys_allocation;
#if VDISP_CUSALLOCATOR_DBG
    vdisp_dbg_add_allocation(allocation);
#endif
    goto exit;
free_allocation:
    kmem_cache_free(allocation_memcache,allocation);
exit:
    mutex_unlock(&allocator->mtx);
    return ret;
}


static int vdisp_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *pstBufConfig)
{
    return INT_MAX;
}

static buf_allocator_ops_t vdisp_alloctorops=
{
    .OnRef=generic_allocator_on_ref,//vdisp_allocator_OnRef,
    .OnUnref=generic_allocator_on_unref,//vdisp_allocator_OnUnref,
    .OnRelease=vdisp_allocator_OnRelease,
    .alloc=vdisp_allocator_alloc,
    .suit_bufconfig=vdisp_allocator_suit_bufconfig,
};

int vdisp_allcator_init(void)
{
    if(allocation_memcache==NULL){
        allocation_memcache=kmem_cache_create("vdispallocation-cache",
                                                            sizeof(vdisp_allocation_t),0,SLAB_HWCACHE_ALIGN,NULL);
        hash_init(allocation_hashtbl);
        if(allocation_memcache==NULL)
            allocation_memcache=VDISP_INVALID_PTR;
    }
    return 0;
}
void vdisp_allcator_deinit(void)
{
    if(allocation_memcache
        && allocation_memcache!=VDISP_INVALID_PTR){
        kmem_cache_destroy(allocation_memcache);
        allocation_memcache=NULL;
    }
}
vdisp_allocator_t* vdisp_allcator_create(vdisp_port_type_e porttype,
    MI_SYS_ChnPort_t *port)
{
    vdisp_device_t *dev=MI_VDISP_IMPL_GetDevice(port->u32DevId);
    vdisp_allocator_t* allocator;
    if(allocation_memcache==VDISP_INVALID_PTR){
        return NULL;
    }
    allocator=kmalloc(sizeof(vdisp_allocator_t),GFP_KERNEL);
    if(!allocator)
        return NULL;
    allocator->porttype=porttype;
    allocator->port=port;
    mutex_init(&allocator->mtx);
    allocator->allocation_cnt=0;
    allocator->breleased=0;
    generic_allocator_init(&allocator->sys_allocator,&vdisp_alloctorops);
    atomic_inc(&dev->allocator_cnt);
#if VDISP_CUSALLOCATOR_DBG
    vdisp_dbg_add_allocator(allocator);
    INIT_LIST_HEAD(&allocator->dbg_allocation_list);
#endif
    return allocator;
}

vdisp_allocation_t * vdisp_bufinfo2allocation(MI_SYS_BufInfo_t *bufinfo)
{
    int cnt;
    vdisp_allocation_t *allocation;
    phys_addr_t phys[3];
    if(!MI_VDISP_IMPL_IsSupBufInfo(bufinfo))
        return NULL;
    cnt=MI_VDISP_IMPL_GetBufPlaneNum(bufinfo->stFrameData.ePixelFormat);
    phys[0]=bufinfo->stFrameData.phyAddr[0];
    phys[1]=bufinfo->stFrameData.phyAddr[1];
    phys[2]=bufinfo->stFrameData.phyAddr[2];
    allocation=_vdisp_find_allocation(phys, cnt);
    return allocation;
}

#if VDISP_CUSALLOCATOR_DBG
static DEFINE_MUTEX(_vdisp_cusalloc_mtx);
static LIST_HEAD(_vdisp_cusallc_list);
void vdisp_dbg_add_allocation(vdisp_allocation_t *allocation)
{
    mutex_lock(&_vdisp_cusalloc_mtx);
    list_add_tail(&allocation->dbg_node, &allocation->allocator->dbg_allocation_list);
    mutex_unlock(&_vdisp_cusalloc_mtx);
}
void vdisp_dbg_rmv_allocation(vdisp_allocation_t *allocation)
{
    mutex_lock(&_vdisp_cusalloc_mtx);
    list_del(&allocation->dbg_node);
    mutex_unlock(&_vdisp_cusalloc_mtx);
}
void vdisp_dbg_add_allocator(vdisp_allocator_t* allocator)
{
    mutex_lock(&_vdisp_cusalloc_mtx);
    list_add_tail(&allocator->dbg_node, &_vdisp_cusallc_list);
    mutex_unlock(&_vdisp_cusalloc_mtx);
}
void vdisp_dbg_rmv_allocator(vdisp_allocator_t* allocator)
{
    mutex_lock(&_vdisp_cusalloc_mtx);
    list_del(&allocator->dbg_node);
    mutex_unlock(&_vdisp_cusalloc_mtx);
}
void vdisp_dbg_dump_allocators(void)
{
    vdisp_allocator_t* allctor;
    mutex_lock(&_vdisp_cusalloc_mtx);
    list_for_each_entry(allctor, &_vdisp_cusallc_list, dbg_node){
        DBG_INFO("allocator[%u, %u]: release=%d, allocations=%d\n",
            allctor->port->u32DevId,allctor->port->u32PortId,
            allctor->breleased,allctor->allocation_cnt);
        if(allctor->allocation_cnt){
            vdisp_allocation_t *allocation;
            list_for_each_entry(allocation, &allctor->dbg_allocation_list, dbg_node){
                DBG_INFO("----allocations: phy=%x\n",
                    allocation->phys[0]);
            }
        }
    }
    mutex_unlock(&_vdisp_cusalloc_mtx);
}
#endif