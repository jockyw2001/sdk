#include <mi_vdisp.h>
#include "sub_buf_allocator.h"

#define INT_MAX		((int)(~0U>>1))
#define VDISP_INVALID_PTR ((unsigned long)(-1))
#define VDISP_BUFQUEUE_DEPTH 3


typedef enum
{
    VDISP_PORT_INPUT,
    VDISP_PORT_OUTPUT
}vdisp_port_type_e;

typedef enum
{
    VDISP_INPUTPORT_UNINIT,
    VDISP_INPUTPORT_INIT,
    VDISP_INPUTPORT_ENABLED,
    VDISP_INPUTPORT_DISABLED,
}vdisp_inputport_status_e;

typedef enum
{
    VDISP_DEVICE_UNINIT,
    VDISP_DEVICE_INIT,
    VDISP_DEVICE_START,
    VDISP_DEVICE_STOP,
}vdisp_device_status_e;

typedef enum
{
    VDISP_WORKER_WAKE_VSYNC=0,
    VDISP_WORKER_PAUSE,
    VDISP_WORKER_PAUSED,
}vdisp_worker_wake_event_e;

typedef struct
{
    MI_VDISP_InputPortAttr_t attr;
    vdisp_inputport_status_e status;
    MI_SYS_ChnPort_t inputport;
    int battr_changed;
    int bbind;
    int sub_buf_id;
    MI_SYS_ChnPort_t bindport;
}vdisp_inputport_t;

typedef struct
{
    int binited;
    MI_VDISP_OutputPortAttr_t attr;
    MI_SYS_ChnPort_t outputport;
    int bbind; //is bound
    MI_SYS_ChnPort_t bindport;
}vdisp_outputport_t;

typedef struct
{
    vdisp_inputport_t inputport[VDISP_MAX_INPUTPORT_NUM];
    vdisp_outputport_t outputport; //only one output port
    struct task_struct *work_thread;
    unsigned long wakeevent;
    subbufallc_subbuf_queue_t bufqueue;
    subbufallc_bufhead_t *prev_buf;
    vdisp_device_status_e status;
    MI_SYS_DRV_HANDLE devhandle;
    atomic_t allocator_cnt;
}vdisp_device_t;

typedef struct
{
    int binited;
    struct semaphore sem;
    vdisp_device_t dev[VDISP_MAX_DEVICE_NUM];
}vdisp_module_t;

typedef struct
{
    void *src_vaddr;
    phys_addr_t src_paddr;
    MI_U32 src_stride;
    void *dst_vaddr;
    phys_addr_t dst_paddr;
    MI_U32 dst_stride;
    MI_U32 width;
    MI_U32 height;
    MI_SYS_BufInfo_t* finish_buf;
}vdisp_copyinfo_t;

typedef struct
{
    mi_sys_Allocator_t sys_allocator;
    vdisp_port_type_e porttype;
    MI_SYS_ChnPort_t *port; //point to vdisp_outputport_t.outputport or vdisp_inputport_t.inputport
    struct mutex mtx;
    int breleased;
    int allocation_cnt;
}vdisp_allocator_t;

typedef struct
{
    MI_SYS_BufferAllocation_t sys_allocation;
    vdisp_allocator_t *allocator;
    subbufallc_subbufhead_t* subbufhd;
}vdisp_allocation_t;


static vdisp_module_t _vdisp_module;
static struct kmem_cache *allocation_memcache=NULL;

MI_RESULT vdisp_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    vdisp_device_t *dev=(vdisp_device_t *)pUsrData;
    vdisp_allocator_t *allocator;
    vdisp_inputport_t *inputport;
    int ret=-1;
    if(pstChnCurPort->eModId!=E_MI_SYS_MODULE_ID_VDISP
       || pstChnCurPort->s32DevId>=VDISP_MAX_DEVICE_NUM
       || pstChnCurPort->s32ChnId>=1
       || pstChnCurPort->s32PortId>=VDISP_MAX_INPUTPORT_NUM)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    inputport=&dev->inputport[pstChnCurPort->s32PortId];
    if(dev->status==VDISP_DEVICE_UNINIT) // bind even when device is working
        goto exit;
    BUG_ON(dev->inputport[pstChnCurPort->s32PortId].bbind);
    if(dev->inputport[pstChnCurPort->s32PortId].status!=VDISP_INPUTPORT_UNINIT){
        BUG_ON(inputport->sub_buf_id>=0);
        inputport->sub_buf_id=subbufallc_registersubbuf(&dev->bufqueue,
            inputport->attr.u32OutX, inputport->attr.u32OutY,
            inputport->attr.u32OutWidth, inputport->attr.u32OutHeight,
            inputport->sub_buf_id, inputport);
        if(inputport->sub_buf_id<0)
            goto exit;
    }
    allocator=vdisp_allcator_create(VDISP_PORT_INPUT,
                       &dev->inputport[pstChnCurPort->s32PortId].inputport);
    if(!allocator)
        goto unreg_subbuf;
    if(0>mi_sys_SetPeerOutputPortCusAllocator(dev->devhandle, 0,
             pstChnCurPort->s32PortId, &allocator->sys_allocator))
        goto free_allocator;
    dev->inputport[pstChnCurPort->s32PortId].bindport=*pstChnPeerPort;
    dev->inputport[pstChnCurPort->s32PortId].bbind=1;
    atomic_inc(&dev->allocator_cnt);
    ret=0;
    goto exit;
unreg_subbuf:
    subbufallc_unregistersubbuf(&dev->bufqueue, inputport->sub_buf_id);
    inputport->sub_buf_id=-1;
free_allocator:
    kree(allocator);
exit:
    up(&_vdisp_module.sem);
    return ret;
}
MI_RESULT vdisp_OnUnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    vdisp_device_t *dev=(vdisp_device_t *)pUsrData;
    int ret=-1;
    vdisp_inputport_t *inputport;
    if(pstChnCurPort->eModId!=E_MI_SYS_MODULE_ID_VDISP
       || pstChnCurPort->s32DevId>=VDISP_MAX_DEVICE_NUM
       || pstChnCurPort->s32ChnId>=1
       || pstChnCurPort->s32PortId>=VDISP_MAX_INPUTPORT_NUM)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    inputport=&dev->inputport[pstChnCurPort->s32PortId];
    if(dev->status==VDISP_DEVICE_UNINIT) // unbind even when device is working
        goto exit;
    if(!inputport->bbind)
        goto exit;
    if(0>mi_sys_SetPeerOutputPortCusAllocator(dev->devhandle, 0,
             pstChnCurPort->s32PortId, NULL))
        goto exit;
    if(inputport->status != VDISP_INPUTPORT_UNINIT){
        BUG_ON(inputport->sub_buf_id<0);
        subbufallc_unregistersubbuf(&dev->bufqueue,inputport->sub_buf_id);
        inputport->sub_buf_id=-1;
    }

    memset(&dev->inputport[pstChnCurPort->s32PortId].bindport, 0, sizeof(MI_SYS_ChnPort_t));
    dev->inputport[pstChnCurPort->s32PortId].bbind=0;
    ret=0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}

MI_RESULT vdisp_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    vdisp_device_t *dev=(vdisp_device_t *)pUsrData;
    int ret=-1;
    if(pstChnCurPort->eModId!=E_MI_SYS_MODULE_ID_VDISP
       || pstChnCurPort->s32DevId>=VDISP_MAX_DEVICE_NUM
       || pstChnCurPort->s32ChnId>=1
       || pstChnCurPort->s32PortId>0)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    if(dev->status==VDISP_DEVICE_UNINIT || dev->status==VDISP_DEVICE_START)
        goto exit;
    dev->outputport.bbind=1;
    ret=0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}
MI_RESULT vdisp_OnUnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    vdisp_device_t *dev=(vdisp_device_t *)pUsrData;
    int ret=-1;
    if(pstChnCurPort->eModId!=E_MI_SYS_MODULE_ID_VDISP
       || pstChnCurPort->s32DevId>=VDISP_MAX_DEVICE_NUM
       || pstChnCurPort->s32ChnId>=1
       || pstChnCurPort->s32PortId>0)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    if(dev->status==VDISP_DEVICE_UNINIT || dev->status==VDISP_DEVICE_START)
        goto exit;
    dev->outputport.bbind=0;
    ret=0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}


static int vdisp_check_inout_compatible(MI_VDISP_InputPortAttr_t *pstInputPortAttr,
    MI_VDISP_OutputPortAttr_t *pstOutputPortAttr)
{
    if(pstInputPortAttr->u32OutWidth<=0 || pstInputPortAttr->u32OutHeight<=0
        || pstInputPortAttr->u32OutX>=pstOutputPortAttr->u32Width
        || pstInputPortAttr->u32OutY>=pstOutputPortAttr->u32Height
        || (pstInputPortAttr->u32OutX+pstInputPortAttr->u32OutWidth)>pstOutputPortAttr->u32Width
        || (pstInputPortAttr->u32OutY+pstOutputPortAttr->u32Height)>pstOutputPortAttr->u32Height)
        return 0;
    return 1;
}

static void vdisp_on_vsync(unsigned long data)
{
    vdisp_device_t *device=(vdisp_device_t *)data;
    set_bit(VDISP_WORKER_WAKE_VSYNC, &device->wakeevent);
    wake_up_process(device->work_thread);
}
int vdisp_start_vsync(vdisp_device_t *device)
{
}
int vdisp_stop_vsync(vdisp_device_t *device)
{
}
MI_U32 vdisp_pixel_offset_yuv422_yuyv(MI_U32 x, MI_U32 y)
{
}
subbufallc_bufhead_t *vdisp_get_outputbuf(vdisp_device_t *device,uint64_t u64pts)
{
    subbufallc_bufhead_t * bufhd=subbufallc_newbufhead();
    MI_SYS_BufConf_t bufconf;
    MI_SYS_BufInfo_t * buf;
    MI_BOOL bBlocked;
    if(!bufhd)
        return NULL;
    bufconf.eBufType=E_MI_SYS_BUFDATA_FRAME;
    bufconf.u64TargetPts=u64pts;
    bufconf.stFrameCfg.eFormat=E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    bufconf.stFrameCfg.u16Width=device->outputport.attr.u32Width;
    bufconf.stFrameCfg.u16Height=device->outputport.attr.u32Height;
    buf=mi_sys_GetOutputPortBuf(device->devhandle, 0, 0, &bufconf, &bBlocked);
    if(!buf)
        goto fail_freebufhd;
    BUG_ON(buf->eBufType!=E_MI_SYS_BUFDATA_FRAME);
    BUG_ON(buf->stFrameData.ePixelFormat!=E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
    bufhd->priv=buf;
    bufhd->type=VIDSP_BUF_TYPE_NORMAL;
    bufhd->u64pts=u64pts;
    bufhd->planenum=1;
    bufhd->width=buf->stFrameData.u16Width;
    bufhd->height=buf->stFrameData.u16Height;
    bufhd->bufplane[0].paddr=buf->stFrameData.u64PhyAddr[0];
    bufhd->bufplane[0].vaddr=buf->stFrameData.pVirAddr[0];
    bufhd->bufplane[0].stride=buf->stFrameData.u32Stride[0];
    bufhd->bufplane[0].pixel_offset=vdisp_pixel_offset_yuv422_yuyv;
    return bufhd;
fail_freebufhd:
    subbufallc_freebufhead(bufhd);
    return NULL;
}

void vdisp_addtocopy_prev(vdisp_copyinfo_t *copyinfo, vdisp_inputport_t *inputport,
    subbufallc_bufhead_t *curbuf, subbufallc_bufhead_t *prev_buf)
{
    MI_U32 u32Offset;
    u32Offset=prev_buf->bufplane[0].pixel_offset(inputport->attr.u32OutX, inputport->attr.u32OutY);
    copyinfo->src_paddr=prev_buf->bufplane[0].paddr+u32Offset;
    if(prev_buf->bufplane[0].vaddr)
        copyinfo->src_vaddr=prev_buf->bufplane[0].vaddr+u32Offset;
    else
        copyinfo->src_vaddr=NULL;
    copyinfo->src_stride=prev_buf->bufplane[0].stride;
    copyinfo->width=inputport->attr.u32OutWidth;
    copyinfo->height=inputport->attr.u32OutHeight;
    copyinfo->finish_buf=NULL;
    u32Offset=curbuf->bufplane[0].pixel_offset(inputport->attr.u32OutX, inputport->attr.u32OutY);
    copyinfo->dst_paddr=curbuf->bufplane[0].paddr+u32Offset;
    if(curbuf->bufplane[0].vaddr)
        copyinfo->dst_vaddr=curbuf->bufplane[0].vaddr+u32Offset;
    else
        copyinfo->dst_vaddr=NULL;
    copyinfo->dst_stride=curbuf->bufplane[0].stride;
}
void vdisp_addtocopy_user(vdisp_copyinfo_t *copyinfo, vdisp_inputport_t *inputport,
    subbufallc_bufhead_t *curbuf, MI_SYS_BufInfo_t *bufinfo)
{
    MI_U32 u32Offset;
    copyinfo->src_paddr=bufinfo->stFrameData.u64PhyAddr[0];
    copyinfo->src_vaddr=bufinfo->stFrameData.pVirAddr[0];
    copyinfo->src_stride=bufinfo->stFrameData.u32Stride[0];
    copyinfo->width=bufinfo->stFrameData.u16Width;
    copyinfo->height=bufinfo->stFrameData.u16Height;
    copyinfo->finish_buf=bufinfo;
    u32Offset=curbuf->bufplane[0].pixel_offset(inputport->attr.u32OutX, inputport->attr.u32OutY);
    copyinfo->dst_paddr=curbuf->bufplane[0].paddr+u32Offset;
    if(curbuf->bufplane[0].vaddr)
        copyinfo->dst_vaddr=curbuf->bufplane[0].vaddr+u32Offset;
    else
        copyinfo->dst_vaddr=NULL;
    copyinfo->dst_stride=curbuf->bufplane[0].stride;
}

void vdisp_device_process(vdisp_device_t *device)
{
    int i;
    uint64_t u64pts;
    vdisp_inputport_t *inputport;
    vdisp_outputport_t *outputport;
    MI_SYS_BufInfo_t *bufinfo;
    int need_copy_cnt=0;
    vdisp_copyinfo_t copyarray[VDISP_MAX_INPUTPORT_NUM];
    int last_copy_cnt=0;
    vdisp_copyinfo_t lastcopyarray[VDISP_MAX_INPUTPORT_NUM];
    int need_wait_cnt=0;
    vdisp_inputport_t *waitarray[VDISP_MAX_INPUTPORT_NUM];
    subbufallc_bufhead_t *curbuf;
    subbufallc_skipallocfromfirst(&device->bufqueue);
    subbufallc_firstbuf(&device->bufqueue);
    for(need_wait_cnt=0;need_wait_cnt<VDISP_MAX_INPUTPORT_NUM;need_wait_cnt++){
        waitarray[need_wait_cnt]=&device->inputport[need_wait_cnt];
    }
try_again:
    need_copy_cnt=0;
    down(&_vdisp_module.sem);
    for(i=0;i<VDISP_MAX_INPUTPORT_NUM && need_wait_cnt>0;i++){
        int bhassubbuf=0;
        inputport=waitarray[i];
        if(!inputport){
            continue;
        }
        if(inputport->status==VDISP_INPUTPORT_UNINIT){
            waitarray[i]=NULL;
            need_wait_cnt--;
            continue;
        }

        if(subbufallc_findsubbuf(&device->bufqueue, curbuf, inputport))
            bhassubbuf=1;
        bufinfo=mi_sys_GetInputPortBuf(device->devhandle, 0, i);
        if(bufinfo){
            if(inputport->status!=VDISP_INPUTPORT_ENABLED){
                //the allocation is disabled, we need to cost the bufs allocated before disable
                mi_sys_FinishBuf(bufinfo);
                waitarray[i]=NULL;
                need_wait_cnt--;
                continue;
            }
            BUG_ON(bufinfo->eBufType!=E_MI_SYS_BUFDATA_FRAME);
            BUG_ON(bufinfo->stFrameData.ePixelFormat!=E_MI_SYS_PIXEL_FRAME_YUV422_YUYV);
            BUG_ON(bufinfo->stFrameData.eCompressMode!=E_MI_SYS_COMPRESS_MODE_NONE);
            BUG_ON(bufinfo->stFrameData.eFieldType!=E_MI_SYS_FIELDTYPE_NONE);
            BUG_ON(bufinfo->stFrameData.eTileMode!=E_MI_SYS_FRAME_TILE_MODE_NONE);
            if(bufinfo->bUsrBuf){
                if(bhassubbuf){ //both has our buf and user buf
                    vdisp_copyinfo_t *copyinfo=&lastcopyarray[last_copy_cnt];
                    vdisp_addtocopy_user(copyinfo, inputport, curbuf, bufinfo);
                    last_copy_cnt++;
                    //this port still need to handle
                }else{
                    vdisp_copyinfo_t *copyinfo=&copyarray[need_copy_cnt];
                    vdisp_addtocopy_user(copyinfo, inputport, curbuf, bufinfo);
                    need_copy_cnt++;
                    waitarray[i]=NULL;
                    need_wait_cnt--;
                }
            }else{
                //buf from our allocator
                vdisp_allocation_t * allocation=vdisp_bufinfo2allocation(bufinfo);
                subbufallc_subbufhead_t *cursub=allocation->subbufhd;
                if(cursub->buf!=curbuf){
                    BUG_ON(bhassubbuf); //if cur buf has subbuf, we should't get later subbuf
                    mi_sys_RewindBuf(bufinfo);
                    if(inputport->battr_changed){
                        //first time after attr changed, don't copy from prev
                        inputport->battr_changed=0;
                    }else if(device->prev_buf){
                        vdisp_copyinfo_t *copyinfo=&copyarray[need_copy_cnt];
                        vdisp_addtocopy_prev(copyinfo, inputport, curbuf, device->prev_buf);
                        need_copy_cnt++;
                    }
                }else{
                    mi_sys_FinishBuf(bufinfo);
                }
                waitarray[i]=NULL;
                need_wait_cnt--;
            }
        }else{
            if(!bhassubbuf){
                if(inputport->battr_changed){
                    //first time after attr changed, don't copy from prev
                    inputport->battr_changed=0;
                }else if(device->prev_buf){
                    vdisp_copyinfo_t *copyinfo=&copyarray[need_copy_cnt];
                    vdisp_addtocopy_prev(copyinfo, inputport, curbuf, device->prev_buf);
                    need_copy_cnt++;
                }
                waitarray[i]=NULL; //no need to wait
                need_wait_cnt--;
            }
        }
    }
    up(&_vdisp_module.sem);
    for(i=0;i<need_copy_cnt;i++){
        vdisp_copyinfo_t *copyinfo=&copyarray[i];
        //fix-me: call gfx to copy
        if(copyinfo->finish_buf)
            mi_sys_FinishBuf(copyinfo->finish_buf);
    }
    if(need_wait_cnt>0)
        goto try_again;

    for(i=0;i<last_copy_cnt;i++){
        vdisp_copyinfo_t *copyinfo=&lastcopyarray[i];
        //fix-me: call gfx to copy
        if(copyinfo->finish_buf)
            mi_sys_FinishBuf(copyinfo->finish_buf);
    }

    curbuf=subbufallc_dequeuebuf(&device->bufqueue);
    if(!curbuf)
        BUG();

    //we copy from previous output buf depending on that the previous output buf won't be free though we have finish it
    if(device->prev_buf){
        subbufallc_freebufhead(device->prev_buf);
    }
    device->prev_buf = curbuf;
    //this finish may cause the previous output buf be free
    mi_sys_FinishBuf((MI_SYS_BufInfo_t *)(curbuf->priv));

    //push one blank output buffer
    vdisp_fill_bufq(device, 0);
}

inline uint64_t get_and_step_pts(vdisp_device_t *device)
{
    uint64_t u64pts;
    u64pts=device->outputport.attr.u64pts;
    device->outputport.attr.u64pts += ; // how get the pts from framerate
    return u64pts;
}
subbufallc_bufhead_t *vdisp_getnextoutputbuf(vdisp_device_t *device)
{
    uint64_t u64pts;
    subbufallc_bufhead_t *buf=NULL;
    down(&_vdisp_module.sem);
    u64pts=get_and_step_pts(device);
    buf=vdisp_get_outputbuf(device, u64pts);
    up(&_vdisp_module.sem);
    return buf;
}
void vdisp_fill_bufq(vdisp_device_t *device, int bwait)
{
    int i=0;
    subbufallc_bufhead_t *buf=NULL;
    i=subbufallc_bufcnt(&device->bufqueue);
    while(i<VDISP_BUFQUEUE_DEPTH){
        buf=vdisp_getnextoutputbuf(device);
        if(!buf){
            if(bwait)
                continue;
            else
                break;
        }
        //fix-me: call gfx to clear output buffer
        subbufallc_queuebuf(&device->bufqueue, buf);
        i++;
    }
}

void vdisp_ondevstart(vdisp_device_t *device)
{
    vdisp_fill_bufq(device, 1);
}
void vdisp_ondevstop(vdisp_device_t *device)
{

}
void vdisp_ondevclose(vdisp_device_t *device)
{
    int i=0;
    subbufallc_bufhead_t *buf=NULL;
    while(subbufallc_bufcnt(&device->bufqueue)>0){
        buf=subbufallc_dequeuebuf(&device->bufqueue);
        if(buf){
            mi_sys_RewindBuf((MI_SYS_BufInfo_t *)(buf->priv));
            subbufallc_freebufhead(buf);
        }else{
            msleep(1);
        }
    }
    if(device->prev_buf){
        subbufallc_freebufhead(device->prev_buf);
        device->prev_buf=NULL;
    }
}

int vdisp_device_worker(void *data)
{
    vdisp_device_t *device=(vdisp_device_t *)data;
    int pre_paused=1;

    while(1){
        set_current_state(TASK_UNINTERRUPTIBLE);
        if(kthread_should_stop())
            break;
        if(test_and_clear_bit(VDISP_WORKER_PAUSE, &device->wakeevent)){
            if(!pre_paused){
                vdisp_ondevstop(device);
                pre_paused=1;
            }
            set_bit(VDISP_WORKER_PAUSED, &device->wakeevent);
            smp_mb();
        }
        if(test_bit(VDISP_WORKER_PAUSED, &device->wakeevent)){
            schedule();
            continue;
        }else{
            if(pre_paused){
                vdisp_ondevstart(device);
                pre_paused=0;
            }
        }
        if(test_and_clear_bit(VDISP_WORKER_WAKE_VSYNC, &device->wakeevent)){
            vdisp_device_process(device);
        }else{
            schedule();
        }
    }
    vdisp_ondevclose(device);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

inline vdisp_allocation_t * vdisp_bufinfo2allocation(MI_SYS_BufInfo_t *bufinfo)
{
    MI_SYS_BufferAllocation_t *sysallocation=container_of(bufinfo, MI_SYS_BufferAllocation_t, stBufInfo);
    vdisp_allocation_t *allocation=container_of(sysallocation, vdisp_allocation_t, sys_allocation);
    return allocation;
}

static buf_allocation_ops_t vdisp_bufallcops=
{
    .OnRef=vdisp_bufallocation_OnRef,
    .OnUnref=vdisp_bufallocation_OnUnref,
    .OnRelease=vdisp_bufallocation_OnRelease,
    .map_user=vdisp_bufallocation_map_user,
    .unmap_user=vdisp_bufallocation_unmap_user
};

void vdisp_bufallocation_OnRef(struct MI_SYS_BufferAllocation_s *thiz)
{
    generic_allocation_on_ref(thiz);
}
void vdisp_bufallocation_OnUnref(struct MI_SYS_BufferAllocation_s *thiz)
{
    generic_allocation_on_unref(thiz);
}
void vdisp_bufallocation_OnRelease(struct MI_SYS_BufferAllocation_s *thiz)
{
    vdisp_allocation_t *allocation=container_of(thiz, vdisp_allocation_t, sys_allocation);
    vdisp_allocator_t *allocator=allocation->allocator;
    vdisp_device_t *dev=NULL;
    vdisp_inputport_t *inputport=NULL;
    int needfree=0;
    mutex_lock(&allocator->mtx);
    inputport=container_of(allocator->port, vdisp_inputport_t, inputport);
    dev=&_vdisp_module.dev[inputport->inputport.s32DevId];
    subbufallc_free(&dev->bufqueue, allocation->subbufhd);
    kmem_cache_free(allocation_memcache, allocation);
    allocator->allocation_cnt--;
    if(allocator->allocation_cnt==0 && allocator->breleased)
        needfree=1;
    if(needfree){
        vdisp_allcator_release(allocator);
    }else{
        mutex_unlock(&allocator->mtx);
    }
}
void *vdisp_bufallocation_map_user(struct MI_SYS_BufferAllocation_s *thiz)
{
}
void  vdisp_bufallocation_unmap_user(struct MI_SYS_BufferAllocation_s *thiz)
{
}

static buf_allocator_ops_t vdisp_alloctorops=
{
    .OnRef=vdisp_allocator_OnRef,
    .OnUnref=vdisp_allocator_OnUnref,
    .OnRelease=vdisp_allocator_OnRelease,
    .alloc=vdisp_allocator_alloc,
    .suit_bufconfig=vdisp_allocator_suit_bufconfig
}
vdisp_allocator_t* vdisp_allcator_create(vdisp_port_type_e porttype,
    MI_SYS_ChnPort_t *port)
{
    vdisp_allocator_t* allocator;
    if(allocation_memcache==NULL){
        allocation_memcache=kmem_cache_create("vdispallocation-cache",
                                                            sizeof(vdisp_allocation_t),0,0,NULL);
        if(allocation_memcache==NULL)
            allocation_memcache=VDISP_INVALID_PTR;
    }
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
}
void vdisp_allcator_release(vdisp_allocator_t* allocator)
{
    vdisp_device_t *dev=&_vdisp_module.dev[allocator->port->s32DevId];
    atomic_dec(&dev->allocator_cnt);
    mutex_destroy(&allocator->mtx);
    kfree(allocator);
}
MI_S32 vdisp_allocator_OnRef(struct mi_sys_Allocator_s *thiz)
{
    return generic_allocator_on_ref(thiz);
}
MI_S32 vdisp_allocator_OnUnref(struct mi_sys_Allocator_s *thiz)
{
    return generic_allocator_on_unref(thiz);
}
MI_S32 vdisp_allocator_OnRelease(struct mi_sys_Allocator_s *thiz)
{
    vdisp_allocator_t *allocator=container_of(thiz, vdisp_allocation_t, allocator);
    int needfree=0;
    mutex_lock(&allocator->mtx);
    allocator->breleased=1;
    if(allocator->allocation_cnt==0)
        needfree=1;
    if(needfree){
        vdisp_allcator_release(allocator);
    }else{
        mutex_unlock(&allocator->mtx);
    }
}
MI_SYS_BufferAllocation_t *vdisp_allocator_alloc(mi_sys_Allocator_t *pstAllocator,
                                   MI_SYS_BufConf_t *stBufConfig , MI_SYS_ChnPort_t *stChnPort)
{
    vdisp_allocator_t *allocator=container_of(pstAllocator, vdisp_allocator_t, allocator);
    vdisp_allocation_t *allocation=NULL;
    MI_SYS_BufferAllocation_t *ret=NULL;
    vdisp_device_t *dev=NULL;
    vdisp_inputport_t *inputport=NULL;
    subbufallc_subbufhead_t* subbufhd=NULL;
    MI_SYS_BufInfo_t *bufinfo;
    MI_SYS_FrameData_t *dataFrame;
    MI_U32 u32Offset=0;
    mutex_lock(&allocator->mtx);
    if(allocator->breleased)
        goto exit;
    inputport=container_of(allocator->port, vdisp_inputport_t, inputport);
    dev=&_vdisp_module.dev[inputport->inputport.s32DevId];
    BUG_ON(inputport->sub_buf_id<0);
    allocation=kmem_cache_alloc(allocation_memcache, GFP_KERNEL);
    if(!allocation)
        goto exit;
    subbufhd=subbufallc_alloc(&dev->bufqueue, inputport->sub_buf_id,
        stBufConfig->u64TargetPts, inputport->attr.s32IsFreeRun?VDISP_SUBBUF_ALLOC_FLAG_FREERUN:0);
    if(!subbufhd)
        goto free_allocation;
    allocation->subbufhd=subbufhd;
    allocation->allocator=allocator;
    allocation->sys_allocation.ref_cnt=ATOMIC_INIT(1);
    allocation->sys_allocation.ops=&vdisp_bufallcops;

    BUG_ON(stBufConfig->eBufType!=E_MI_SYS_BUFDATA_FRAME);
    BUG_ON(stBufConfig->stFrameCfg.eFormat!=E_MI_SYS_PIXEL_FRAME_YUV422_YUYV); //review: support format yuv420 only
    bufinfo=&allocation->sys_allocation.stBufInfo;
    dataFrame=&bufinfo->stFrameData;
    memset(bufinfo, 0, sizeof(MI_SYS_BufInfo_t));
    bufinfo->eBufType=stBufConfig->eBufType;
    bufinfo->u64Pts=stBufConfig->u64TargetPts;
    dataFrame->eTileMode=E_MI_SYS_FRAME_TILE_MODE_NONE;
    dataFrame->ePixelFormat=stBufConfig->stFrameCfg.eFormat;
    dataFrame->eCompressMode=E_MI_SYS_COMPRESS_MODE_NONE;
    dataFrame->eFrameScanMode=E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    dataFrame->eFieldType=E_MI_SYS_FIELDTYPE_NONE;
    dataFrame->u16Width=subbufallc_subbufvalidwidth(subbufhd);
    dataFrame->u16Height=subbufallc_subbufvalidheight(subbufhd);
    dataFrame->u32Stride[0]=subbufhd->buf->bufplane[0].stride;
    u32Offset=subbufhd->buf->bufplane[0].pixel_offset(subbufhd->suballcinfo->x, subbufhd->suballcinfo->y);
    dataFrame->u64PhyAddr[0]=subbufhd->buf->bufplane[0].paddr+u32Offset;
    if(subbufhd->buf->bufplane[0].vaddr)
        dataFrame->pVirAddr[0]=subbufhd->buf->bufplane[0].vaddr+u32Offset; //tommy said this is user VA
    else
        dataFrame->pVirAddr[0]=NULL;
    dataFrame->stWindowRect.u16X=0;
    dataFrame->stWindowRect.u16Y=0;
    dataFrame->stWindowRect.u16Width=dataFrame->u16Width;
    dataFrame->stWindowRect.u16Height=dataFrame->u16Height;
    ret = &allocation->sys_allocation;
    goto exit;
free_allocation:
    kmem_cache_free(allocation_memcache,allocation);
exit:
    mutex_unlock(&allocator->mtx);
    return ret;
}


int vdisp_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_Buf_Config *pstBufConfig , MI_SYS_ChnPort_t *pstChnPort)
{
    return INT_MAX;
}

MI_S32 MI_VDISP_Init(void)
{
    if(_vdisp_module.binited)
        return -1;
    if(0>subbufallc_init())
        return -1;
    memset(&_vdisp_module,0,sizeof(_vdisp_module);
    init_MUTEX(&_vdisp_module.sem);
    _vdisp_module.binited=1;
    return 0;
}
MI_S32 MI_VDISP_Exit(void)
{
    int ret=-1;
    int i;
    vdisp_device_t *dev=NULL;
    if(!_vdisp_module.binited)
        return -1;
    if(0>subbufallc_deinit())
        return -1;
    down(&_vdisp_module.sem);
    for(i=0;i<;i++){
        dev=&(_vdisp_module.dev[i]);
        if(dev->status != VDISP_DEVICE_UNINIT)
            goto exit;
    }
    _vdisp_module.binited=0;
    ret=0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}

MI_S32 MI_VDISP_OpenDevice(MI_VDISP_DEV DevId)
{
    vdisp_device_t *dev=NULL;
    mi_sys_ModuleInfo_t modinfo;
    mi_sys_ModuleBindOps_t bindops;
    int i;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    if(dev->status != VDISP_DEVICE_UNINIT)
        goto fail_exit;
    memset(dev, 0, sizeof(*dev));

    dev->allocator_cnt=ATOMIC_INIT(0);

    //input/output port
    for(i=0;i<VDISP_MAX_INPUTPORT_NUM;i++){
        dev->inputport[i].inputport.eModId=E_MI_SYS_MODULE_ID_VDISP;
        dev->inputport[i].inputport.s32DevId=DevId;
        dev->inputport[i].inputport.s32ChnId=0;
        dev->inputport[i].inputport.s32PortId=i;
        dev->inputport[i].sub_buf_id=-1;
    }
    dev->outputport.outputport.eModId=E_MI_SYS_MODULE_ID_VDISP;
    dev->outputport.outputport.s32DevId=DevId;
    dev->outputport.outputport.s32ChnId=0;
    dev->outputport.outputport.s32PortId=0;

    if(0>subbufallc_subbufqinit(&dev->bufqueue))
        goto fail_exit;

    //create thread
    set_bit(VDISP_WORKER_PAUSE, &dev->wakeevent); //initial paused
    dev->work_thread=kthread_run(vdisp_device_worker, dev, "vdisp-dev%d",DevId);
    if(IS_ERR(dev->work_thread))
        goto fail_exit;

    modinfo.u32ModuleId=E_MI_SYS_MODULE_ID_VDISP;
    modinfo.u32DevId=DevId;
    modinfo.u32DevChnNum=1;
    modinfo.u32InputPortNum=VDISP_MAX_INPUTPORT_NUM;
    modinfo.u32OutputPortNum=1;

    bindops.OnBindInputPort=vdisp_OnBindInputPort;
    bindops.OnBindOutputPort=vdisp_OnUnBindInputPort;
    bindops.OnUnBindInputPort=vdisp_OnBindOutputPort;
    bindops.OnUnBindOutputPort=vdisp_OnUnBindOutputPort;
    dev->devhandle=mi_sys_RegisterDev(&modinfo, &bindops, dev);
    if(NULL==dev->devhandle)
        goto fail_stopthread;
    dev->status=VDISP_DEVICE_INIT;
    up(&_vdisp_module.sem);
    return 0;
fail_stopthread:
    kthread_stop(dev->work_thread);
fail_exit:
    up(&_vdisp_module.sem);
    return -1;
}
MI_S32 MI_VDISP_CloseDevice(MI_VDISP_DEV DevId)
{
    vdisp_device_t *dev=NULL;
    vdisp_inputport_t *inputport;
    int ret=-1;
    int i;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    if(VDISP_DEVICE_INIT!=dev->status && VDISP_DEVICE_STOP!=dev->status) //device is uninited or is running
        goto exit;
    for(i=0;i<VDISP_MAX_INPUTPORT_NUM;i++){
        inputport=&dev->inputport[i];
        if(inputport->status == VDISP_INPUTPORT_ENABLED) //input port still enabled
            goto exit;
        if(inputport->status != VDISP_INPUTPORT_UNINIT){
            if(inputport->bbind || inputport->sub_buf_id>=0) //input port still bond
                goto exit;
            inputport->status == VDISP_INPUTPORT_UNINIT;
        }
    }
    if(dev->work_thread){
        kthread_stop(dev->work_thread); //exit worker thread
    }
    BUG_ON(atomic_read(&dev->allocator_cnt)!=0);
    BUG_ON(dev->devhandle==NULL);
    subbufallc_subbufqdestroy(&dev->bufqueue);
    mi_sys_UnRegisterDev(dev->devhandle);
    dev->status=VDISP_DEVICE_UNINIT;
    ret = 0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}

MI_S32 MI_VDISP_SetOutputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_OutputPortAttr_t *pstOutputPortAttr)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    int i;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(PortId>0)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    if(VDISP_DEVICE_UNINIT==dev->status) //output port attr can be changed even is working
        goto exit;
    for(i=0;i<VDISP_MAX_INPUTPORT_NUM;i++){
        if(dev->inputport[i].status==VDISP_INPUTPORT_UNINIT
            || dev->inputport[i].status==VDISP_INPUTPORT_DISABLED)
            continue;
        if(!vdisp_check_inout_compatible(&dev->inputport[i].attr, pstOutputPortAttr))
            goto exit;
    }

    dev->outputport.attr = *pstOutputPortAttr;
    dev->outputport.binited = 1;
    ret = 0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}
MI_S32 MI_VDISP_GetOutputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_OutputPortAttr_t *pstOutputPortAttr)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    int i;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(PortId>0)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    if(VDISP_DEVICE_UNINIT==dev->status || !dev->outputport.binited)
        goto exit;
     *pstOutputPortAttr = dev->outputport.attr;
    ret = 0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}

MI_S32 MI_VDISP_SetInputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_InputPortAttr_t *pstInputPortAttr)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    int i;
    vdisp_inputport_t *inputport;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(PortId>=VDISP_MAX_INPUTPORT_NUM)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    inputport=&dev->inputport[PortId];
    if(VDISP_DEVICE_UNINIT==dev->status) //input port attr can be changed even is working
        goto exit;
    if(dev->outputport.binited){
        if(!vdisp_check_inout_compatible(pstInputPortAttr, &dev->outputport.attr)) //review: need to check input port overlap
            goto exit;
    }
    inputport->attr = *pstInputPortAttr;
    if(inputport->bbind){
        inputport->sub_buf_id=subbufallc_registersubbuf(&dev->bufqueue,
            inputport->attr.u32OutX, inputport->attr.u32OutY,
            inputport->attr.u32OutWidth, inputport->attr.u32OutHeight,
            inputport->sub_buf_id, inputport);
        if(inputport->sub_buf_id<0)
            goto exit;
    }
    if(inputport->status == VDISP_INPUTPORT_UNINIT){
        inputport->status = VDISP_INPUTPORT_INIT;
    }
    inputport->battr_changed=1;
    ret = 0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}
MI_S32 MI_VDISP_GetInputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_InputPortAttr_t *pstInputPortAttr)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    int i;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(PortId>=VDISP_MAX_INPUTPORT_NUM)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    if(VDISP_DEVICE_UNINIT==dev->status || VDISP_INPUTPORT_UNINIT==dev->inputport[PortId].status)
        goto exit;
    *pstInputPortAttr = dev->inputport[PortId].attr;
    ret = 0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}

MI_S32 MI_VDISP_EnableInputPort(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    int i;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(PortId>=VDISP_MAX_INPUTPORT_NUM)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    if(VDISP_DEVICE_UNINIT==dev->status || VDISP_INPUTPORT_UNINIT==dev->inputport[PortId].status)
        goto exit;
     if(dev->outputport.binited){
        if(!vdisp_check_inout_compatible(&dev->inputport[PortId].attr, &dev->outputport.attr))
            goto exit;
    }
    if(dev->inputport[PortId].sub_buf_id>=0)
        subbufallc_enable(&dev->bufqueue, dev->inputport[PortId].sub_buf_id, 1);
    if(dev->inputport[PortId].status != VDISP_INPUTPORT_ENABLED){
        dev->inputport[PortId].status = VDISP_INPUTPORT_ENABLED;
    }
    ret = 0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}
MI_S32 MI_VDISP_DisableInputPort(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    int i;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(PortId>=VDISP_MAX_INPUTPORT_NUM)
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    if(VDISP_DEVICE_UNINIT==dev->status || VDISP_INPUTPORT_UNINIT==dev->inputport[PortId].status)
        goto exit;
    if(dev->inputport[PortId].sub_buf_id>=0){
        subbufallc_enable(&dev->bufqueue, dev->inputport[PortId].sub_buf_id, 0);
    }
    if(dev->inputport[PortId].status == VDISP_INPUTPORT_ENABLED){
        dev->inputport[PortId].status = VDISP_INPUTPORT_DISABLED;
    }
    ret = 0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}

MI_S32 MI_VDISP_StartDev(MI_VDISP_DEV DevId)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    int i;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    if(VDISP_DEVICE_UNINIT==dev->status || !dev->outputport.binited)
        goto exit;
    if(VDISP_DEVICE_START != dev->status){
        //do start
        clear_bit(VDISP_WORKER_PAUSED, &dev->wakeevent);
        if(vdisp_start_vsync(dev)<0)
            goto exit;
        subbufallc_enable(&dev->bufqueue, -1, 1);
        dev->status = VDISP_DEVICE_START;
    }
    ret = 0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}
MI_S32 MI_VDISP_StopDev(MI_VDISP_DEV DevId)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    int i;
    if(DevId<0 || DevId>=VDISP_MAX_DEVICE_NUM )
        return -1;
    if(!_vdisp_module.binited)
        return -1;
    down(&_vdisp_module.sem);
    dev=&(_vdisp_module.dev[DevId]);
    if(VDISP_DEVICE_UNINIT==dev->status || !dev->outputport.binited)
        goto exit;

    if(VDISP_DEVICE_START == dev->status){
        //do stop
        if(vdisp_stop_vsync(dev)<0)
            goto exit;
        subbufallc_enable(&dev->bufqueue, -1, 0);
        set_bit(VDISP_WORKER_PAUSE, &dev->wakeevent);
        smp_mb();
        while(!test_bit(VDISP_WORKER_PAUSED, &dev->wakeevent)){
            wake_up_process(dev->work_thread);
            msleep(1);
        }
        dev->status = VDISP_DEVICE_STOP;
    }
    ret = 0;
exit:
    up(&_vdisp_module.sem);
    return ret;
}
