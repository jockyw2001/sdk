#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/hashtable.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/highmem.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <asm/atomic.h>
#include <asm/string.h>
#include <asm/memory.h>
#include <asm/highmem.h>
#include <asm/cacheflush.h>
#include <asm/div64.h>

#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_sys.h"
#include "mi_sys_internal.h"

#include "mi_vdisp.h"
#include "sub_buf_allocator.h"
#include "mi_vdisp_impl.h"
#include "vdisp_custom_allocator.h"
#include "region_substract.h"
#include "mi_gfx_datatype.h"
#include "mi_gfx.h"
#include "mi_sys_proc_fs_internal.h"

#define DBG_PRINT printk
#define vdisp_devid(d) (d-&_vdisp_module.dev[0])

//#define ENABLE_VDISP_TRACE_PERF (1)
#if defined(ENABLE_VDISP_TRACE_PERF) && (ENABLE_VDISP_TRACE_PERF == 1)
#define VDISP_PERF_TIME(pu64Time) do {\
        struct timespec sttime;\
        memset(&sttime, 0, sizeof(sttime));\
        do_posix_clock_monotonic_gettime(&sttime);\
         *(pu64Time) = ((MI_U64)sttime.tv_sec) * 1000000ULL + (sttime.tv_nsec / 1000);\
    } while(0)
#else
#define VDISP_PERF_TIME(pu64Time)
#endif

#define MAX_FENCE(X, Y, Z) (X>Y?(X>Z?X:Z):(Y>Z?Y:Z))

static MI_U16 _vdisp_copy_buf_gfx(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect)
{
    MI_U16 u16Fence = 0;
    MI_GFX_Opt_t stOpt;
    MI_GFX_Opt_t *pstBlitOpt = &stOpt;
    //DBG_INFO("Src surface = {.phyAddr: %llx, .eColorFmt: %d, .u32Width: %u, .u32Height: %u, .u32Stride: %u}.\n",
    //    pstSrc->phyAddr, pstSrc->eColorFmt, pstSrc->u32Width, pstSrc->u32Height, pstSrc->u32Stride);
    //DBG_INFO("Dest surface = {.phyAddr: %llx, .eColorFmt: %d, .u32Width: %u, .u32Height: %u, .u32Stride: %u}.\n",
    //    pstDst->phyAddr, pstDst->eColorFmt, pstDst->u32Width, pstDst->u32Height, pstDst->u32Stride);
    memset(pstBlitOpt, 0, sizeof(*pstBlitOpt));

    pstBlitOpt->bEnGfxRop = FALSE;
    pstBlitOpt->eRopCode = E_MI_GFX_ROP_NONE;
    pstBlitOpt->eSrcDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    pstBlitOpt->eDstDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    pstBlitOpt->eMirror = E_MI_GFX_MIRROR_NONE;
    pstBlitOpt->eRotate = E_MI_GFX_ROTATE_0;
    pstBlitOpt->eSrcYuvFmt = 0;
    pstBlitOpt->eDstYuvFmt = 0;
    pstBlitOpt->stClipRect.s32Xpos = 0;
    pstBlitOpt->stClipRect.s32Ypos = 0;
    pstBlitOpt->stClipRect.u32Width  = 0;
    pstBlitOpt->stClipRect.u32Height = 0;
    MI_GFX_BitBlit(pstSrc, pstSrcRect, pstDst, pstDstRect, pstBlitOpt, &u16Fence);
    return u16Fence;
}
void vdisp_finish_copy(MI_U16 u16Fence)
{
    MI_GFX_WaitAllDone(FALSE, u16Fence);
}

MI_U16 vdisp_copy_buf(vdisp_copyinfo_t *copyinfo)
{
    int i;
    MI_U16 u16Fence = 0;
    vdisp_copyinfo_plane_t *plane;
    MI_GFX_Surface_t srcSurf, dstSurf;
    for(i=0;i<copyinfo->plane_num;i++){
        plane=&copyinfo->plane[i];
        srcSurf.eColorFmt=E_MI_GFX_FMT_I8;
        srcSurf.phyAddr=plane->src_paddr;
        srcSurf.u32Width=plane->width;
        srcSurf.u32Height=plane->height;
        srcSurf.u32Stride=plane->src_stride;

        dstSurf.eColorFmt=E_MI_GFX_FMT_I8;
        dstSurf.phyAddr=plane->dst_paddr;
        dstSurf.u32Width=plane->width;
        dstSurf.u32Height=plane->height;
        dstSurf.u32Stride=plane->dst_stride;

        u16Fence=_vdisp_copy_buf_gfx(&srcSurf, NULL, &dstSurf,NULL);
    }
    return u16Fence;
}
MI_U16 _vdisp_clear_buf_rect(MI_SYS_BufInfo_t * buf,
    MI_GFX_Rect_t *dstRect, MI_U32 u32ColorVal)
{
    MI_U16 u16Fence=0;
    MI_GFX_Surface_t dstSurf;
    MI_GFX_Rect_t rect,*prect;
    MI_SYS_FrameData_t *frame=&buf->stFrameData;
    MI_SYS_PixelFormat_e ePixelFormat=frame->ePixelFormat;
    MI_U32 tmp;
    if(ePixelFormat==E_MI_SYS_PIXEL_FRAME_YUV422_YUYV){
        dstSurf.eColorFmt=E_MI_GFX_FMT_YUV422;
        dstSurf.phyAddr=frame->phyAddr[0];
        dstSurf.u32Width=frame->u16Width;
        dstSurf.u32Height=frame->u16Height;
        dstSurf.u32Stride=frame->u32Stride[0];
        prect=NULL;
        if(dstRect){
            rect.s32Xpos=dstRect->s32Xpos;
            rect.s32Ypos=dstRect->s32Ypos;
            rect.u32Width=dstRect->u32Width;
            rect.u32Height=dstRect->u32Height;
            prect=&rect;
        }
        MI_GFX_QuickFill(&dstSurf, prect, u32ColorVal, &u16Fence);
    }else if(ePixelFormat==E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420){
        dstSurf.eColorFmt=E_MI_GFX_FMT_I8;
        dstSurf.phyAddr=frame->phyAddr[0];
        dstSurf.u32Width=frame->u16Width ;
        dstSurf.u32Height=frame->u16Height;
        dstSurf.u32Stride=frame->u32Stride[0];
        prect=NULL;
        if(dstRect){
            rect.s32Xpos=dstRect->s32Xpos ;
            rect.s32Ypos=dstRect->s32Ypos;
            rect.u32Width=dstRect->u32Width ;
            rect.u32Height=dstRect->u32Height;
            prect=&rect;
        }
        MI_GFX_QuickFill(&dstSurf, prect, (u32ColorVal>>8)&0xFF/*Y*/, &u16Fence);
        dstSurf.eColorFmt=E_MI_GFX_FMT_RGB565;
        dstSurf.phyAddr=frame->phyAddr[1];
        dstSurf.u32Width=frame->u16Width/2;
        dstSurf.u32Height=frame->u16Height/2;
        dstSurf.u32Stride=frame->u32Stride[1];
        prect=NULL;
        if(dstRect){
            rect.s32Xpos=dstRect->s32Xpos/2;
            rect.s32Ypos=dstRect->s32Ypos/2;
            rect.u32Width=dstRect->u32Width/2 ;
            rect.u32Height=dstRect->u32Height/2;
            prect=&rect;
        }
        tmp=((u32ColorVal&0xFF0000)>>5)|((u32ColorVal&0xFF)<<3);
        u32ColorVal=(u32ColorVal&0xF80000)|((tmp<<2)&0xFC00)|(tmp&0xF8);
        MI_GFX_QuickFill(&dstSurf, prect,
            u32ColorVal,
            &u16Fence);
    }else{
        MI_SYS_BUG();
    }
    return u16Fence;
}
void vdisp_rgn_add_rect(PCLIPRGN rgn,
    int x, int y, int w, int h)
{
    RECT rect;
    rect.left=x;
    rect.top=y;
    rect.right=x+w;
    rect.bottom=y+h;
    AddClipRect(rgn, &rect);
}
MI_U16 vdisp_clear_buf(MI_SYS_BufInfo_t * buf,
    PCLIPRGN reserve_rgn, MI_U32 u32ColorVal)
{

    MI_U16 u16Fence=0;
    MI_GFX_Rect_t dstRect;
    CLIPRGN rgnD, rgnM;
    RECT rect, *prect;
    int donecnt=0;
    if(IsEmptyClipRgn(reserve_rgn)){
        u16Fence=_vdisp_clear_buf_rect(buf, NULL, u32ColorVal);
        return u16Fence;
    }
    InitClipRgn(&rgnD, NULL);
    InitClipRgn(&rgnM, NULL);
    rect.left=0;
    rect.top=0;
    rect.right=rect.left+buf->stFrameData.u16Width;
    rect.bottom=rect.top+buf->stFrameData.u16Height;
    AddClipRect(&rgnM, &rect);

    SubtractRegion(&rgnD, &rgnM, reserve_rgn);
    region_for_each_rect(prect, &rgnD){
        dstRect.s32Xpos=prect->left;
        dstRect.s32Ypos=prect->top;
        dstRect.u32Width=prect->right-prect->left;
        dstRect.u32Height=prect->bottom-prect->top;
        u16Fence=_vdisp_clear_buf_rect(buf, &dstRect, u32ColorVal);
        donecnt++;
    }
    EmptyClipRgn(&rgnD);
    EmptyClipRgn(&rgnM);
    return u16Fence;
}

static vdisp_module_t _vdisp_module;
static DEFINE_SEMAPHORE(module_sem);

static int _vdisp_init_inputports_allocator(
    vdisp_device_t *dev)
{
    vdisp_allocator_t *allocator;
    int i=0;
    int ret=-1;
    for(i=0;i<VDISP_MAX_INPUTPORT_NUM;i++){
        allocator=vdisp_allcator_create(VDISP_PORT_INPUT,
                           &dev->inputport[i].inputport);
        if(!allocator){
            DBG_ERR("create allocator fail\n");
            goto unset_allocator;
        }
        if(mi_sys_SetInputPortUsrAllocator(dev->devhandle, 0,
                 i, &allocator->sys_allocator)<0){
            DBG_ERR("set inputport allocator fail\n");
            allocator->sys_allocator.ops->OnRelease(&allocator->sys_allocator);
            goto unset_allocator;
        }
    }
    ret=0;
    goto exit;
unset_allocator:
    for(i=0;i<VDISP_MAX_INPUTPORT_NUM;i++){
        mi_sys_SetInputPortUsrAllocator(dev->devhandle, 0,i, NULL);
    }
exit:
    return ret;
}
static int _vdisp_deinit_inputports_allocator(
    vdisp_device_t *dev)
{
    int i=0;
    vdisp_inputport_t *inputport;
    for(i=0;i<VDISP_MAX_INPUTPORT_NUM;i++){
        inputport=&dev->inputport[i];
        mi_sys_SetInputPortUsrAllocator(dev->devhandle, 0,i, NULL);
        if(inputport->sub_buf_id>=0){
            sba_unregistersubbuf(&dev->bufqueue,inputport->sub_buf_id);
            inputport->sub_buf_id=-1;
        }
    }
    return 0;
}

static int vdisp_check_inout_compatible(MI_VDISP_InputPortAttr_t *pstInputPortAttr,
    MI_VDISP_OutputPortAttr_t *pstOutputPortAttr)
{
    if(pstInputPortAttr->u32OutWidth<=0 || pstInputPortAttr->u32OutHeight<=0
        || pstInputPortAttr->u32OutX>=pstOutputPortAttr->u32Width
        || pstInputPortAttr->u32OutY>=pstOutputPortAttr->u32Height
        || (pstInputPortAttr->u32OutX+pstInputPortAttr->u32OutWidth)>pstOutputPortAttr->u32Width
        || (pstInputPortAttr->u32OutY+pstInputPortAttr->u32OutHeight)>pstOutputPortAttr->u32Height)
        return 0;
    return 1;
}

static int vdisp_check_inputportattr(
    vdisp_device_t *dev,
    MI_VDISP_PORT PortId,
    MI_VDISP_InputPortAttr_t *attr)
{
    int i,j;
    MI_VDISP_InputPortAttr_t *attrtmp;
    for(i=0;i<VDISP_MAX_INPUTPORT_NUM; i++){
        if(i==PortId ||
           dev->inputport[i].status!=VDISP_INPUTPORT_ENABLED)
            continue;
        attrtmp=&dev->inputport[i].attr;
        if(attrtmp->u32OutX<(attr->u32OutX+attr->u32OutWidth)
           && (attrtmp->u32OutX+attrtmp->u32OutWidth)>attr->u32OutX
           && attrtmp->u32OutY<(attr->u32OutY+attr->u32OutHeight)
           && (attrtmp->u32OutY+attrtmp->u32OutHeight)>attr->u32OutY){
            for(j=0;j<VDISP_MAX_INPUTPORT_NUM; j++) {
                DBG_ERR("[PortId %d] Attr illegal :port %d disbale: %d :[%d, %d,%d,%d]\n",
                PortId, j, dev->inputport[j].status,
                dev->inputport[j].attr.u32OutX, dev->inputport[j].attr.u32OutY,
                dev->inputport[j].attr.u32OutWidth, dev->inputport[j].attr.u32OutHeight
                );
             }
            DBG_ERR("PortId :%d is same as others Portid\n", PortId);
            return 0;
        }
    }
    return 1;
}

static unsigned long vdisp_pixel_offset_yuv422_yuyv(struct sba_plane_s *plane,unsigned long x, unsigned long y)
{
    MI_SYS_BUG_ON(x&1);
    return y*plane->stride+x*2;
}
static unsigned long vdisp_pixel_offset_yuv420_y(struct sba_plane_s *plane,unsigned long x, unsigned long y)
{
    MI_SYS_BUG_ON(x&1);
    MI_SYS_BUG_ON(y&1);
    return y*plane->stride+x;
}
static unsigned long vdisp_pixel_offset_yuv420_uv(struct sba_plane_s *plane,unsigned long x, unsigned long y)
{
    MI_SYS_BUG_ON(x&1);
    MI_SYS_BUG_ON(y&1);
    return (y/2)*plane->stride+x;
}

static void vdisp_addtocopy_prev(vdisp_copyinfo_t *copyinfo, vdisp_inputport_t *inputport,
    sba_bufhead_t *curbuf, sba_bufhead_t *prev_buf, MI_SYS_PixelFormat_e ePixelFmt)
{
    MI_U32 u32Offset;
    vdisp_copyinfo_plane_t *copyplane;
    int cnt=prev_buf->planenum,i;
    memset(copyinfo, 0, sizeof(vdisp_copyinfo_t));
    copyinfo->finish_buf=NULL;
    copyinfo->plane_num=prev_buf->planenum;
    for(i=0;i<cnt;i++){
        sba_plane_t *bufplane=&prev_buf->bufplane[i];
        copyplane=&copyinfo->plane[i];
        u32Offset=bufplane->pixel_offset(bufplane, inputport->attr.u32OutX, inputport->attr.u32OutY);
        copyplane->src_paddr=bufplane->paddr+u32Offset;
        if(bufplane->vaddr)
            copyplane->src_vaddr=bufplane->vaddr+u32Offset;
        else
            copyplane->src_vaddr=NULL;
        copyplane->src_stride=bufplane->stride;

        u32Offset=curbuf->bufplane[i].pixel_offset(&curbuf->bufplane[i], inputport->attr.u32OutX, inputport->attr.u32OutY);
        copyplane->dst_paddr=curbuf->bufplane[i].paddr+u32Offset;
        if(curbuf->bufplane[i].vaddr)
            copyplane->dst_vaddr=curbuf->bufplane[i].vaddr+u32Offset;
        else
            copyplane->dst_vaddr=NULL;
        copyplane->dst_stride=curbuf->bufplane[i].stride;
    }
    if(ePixelFmt==E_MI_SYS_PIXEL_FRAME_YUV422_YUYV){
        MI_SYS_BUG_ON(copyinfo->plane_num!=1);
        copyinfo->plane[0].width=inputport->attr.u32OutWidth*2;
        copyinfo->plane[0].height=inputport->attr.u32OutHeight;
    }else if(ePixelFmt==E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420){
        MI_SYS_BUG_ON(copyinfo->plane_num!=2);
        copyinfo->plane[0].width=inputport->attr.u32OutWidth;
        copyinfo->plane[0].height=inputport->attr.u32OutHeight;
        copyinfo->plane[1].width=inputport->attr.u32OutWidth;
        copyinfo->plane[1].height=inputport->attr.u32OutHeight/2;
    }else{
        MI_SYS_BUG();
    }
}

static void vdisp_addtocopy(
    vdisp_copyinfo_t *copyinfo,
    vdisp_inputport_t *inputport,
    sba_bufhead_t *curbuf,
    MI_SYS_BufInfo_t * srcbuf)
{
    MI_SYS_PixelFormat_e ePixelFmt=srcbuf->stFrameData.ePixelFormat;
    vdisp_copyinfo_plane_t *copyplane;
    MI_U32 u32Offset;
    MI_U32 u32OutWidth, u32OutHeight;
    int i;
    u32OutWidth = inputport->attr.u32OutWidth;
    u32OutHeight= inputport->attr.u32OutHeight;
    if(srcbuf->stFrameData.u16Width<u32OutWidth)
        u32OutWidth = srcbuf->stFrameData.u16Width;
    if(srcbuf->stFrameData.u16Height<u32OutHeight)
        u32OutHeight = srcbuf->stFrameData.u16Height;
    memset(copyinfo, 0, sizeof(vdisp_copyinfo_t));
    copyinfo->finish_buf=NULL;
    if(ePixelFmt==E_MI_SYS_PIXEL_FRAME_YUV422_YUYV){
        copyinfo->plane_num=1;
        copyinfo->plane[0].width=u32OutWidth*2;
        copyinfo->plane[0].height=u32OutHeight;
    }else if(ePixelFmt==E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420){
        copyinfo->plane_num=2;
        copyinfo->plane[0].width=u32OutWidth;
        copyinfo->plane[0].height=u32OutHeight;
        copyinfo->plane[1].width=u32OutWidth;
        copyinfo->plane[1].height=u32OutHeight/2;
    }else{
        MI_SYS_BUG();
    }
    for(i=0;i<copyinfo->plane_num;i++){
        copyplane=&copyinfo->plane[i];
        copyplane->src_paddr=srcbuf->stFrameData.phyAddr[i];
        copyplane->src_vaddr=srcbuf->stFrameData.pVirAddr[i];
        copyplane->src_stride=srcbuf->stFrameData.u32Stride[i];

        u32Offset=curbuf->bufplane[i].pixel_offset(&curbuf->bufplane[i], inputport->attr.u32OutX, inputport->attr.u32OutY);
        copyplane->dst_paddr=curbuf->bufplane[i].paddr+u32Offset;
        if(curbuf->bufplane[i].vaddr)
            copyplane->dst_vaddr=curbuf->bufplane[i].vaddr+u32Offset;
        else
            copyplane->dst_vaddr=NULL;
        copyplane->dst_stride=curbuf->bufplane[i].stride;
    }
}

#if VDISP_SUPPORT_OVERLAYINPUTPORT
static int _vdisp_enable_overlay(
            vdisp_device_t *dev,
            vdisp_inputport_t *inputport)
{
    vdisp_overlayinfo_t *overlayinfo=&dev->overlayinfo[VDISP_PORTID_TO_OVERLAYIDX(inputport->inputport.u32PortId)];
    overlayinfo->overlayinputport=inputport;
    return 0;
}
static int _vdisp_disable_overlay(
            vdisp_device_t *dev,
            vdisp_inputport_t *inputport)
{
    vdisp_overlayinfo_t *overlayinfo=&dev->overlayinfo[VDISP_PORTID_TO_OVERLAYIDX(inputport->inputport.u32PortId)];
    MI_SYS_BUG_ON(inputport!=overlayinfo->overlayinputport);
    overlayinfo->overlayinputport=NULL;
    if(overlayinfo->overlaybufinfo){
        mi_sys_FinishBuf(overlayinfo->overlaybufinfo);
        overlayinfo->overlaybufinfo=NULL;
    }
    return 0;
}
static int _vdisp_try_update_overlay(
            vdisp_device_t *dev)
{
    MI_SYS_BufInfo_t *bufinfo;
    vdisp_overlayinfo_t *overlayinfo;
    int i;
    for(i=0;i<VDISP_MAX_OVERLAYINPUTPORT_NUM;i++){
        overlayinfo=&dev->overlayinfo[i];
        if(!overlayinfo->overlayinputport)
            continue;
        bufinfo=mi_sys_GetInputPortBuf(dev->devhandle, 0,
                overlayinfo->overlayinputport->inputport.u32PortId, 0);
        if(!bufinfo)
            continue;
        if(overlayinfo->overlaybufinfo){
            mi_sys_FinishBuf(overlayinfo->overlaybufinfo);
        }
        overlayinfo->overlaybufinfo=bufinfo;
    }
    return 0;
}
static MI_U16 _vdisp_blend_overlay(
                vdisp_device_t *dev,
                sba_bufhead_t *curbuf,
                PCLIPRGN reserve_rgn)
{
    vdisp_overlayinfo_t *overlay;
    vdisp_copyinfo_t copyinfo;
    MI_U16 u16Fence=0;
    int i, donecnt=0;
    for(i=0;i<VDISP_MAX_OVERLAYINPUTPORT_NUM;i++){
        overlay=&dev->overlayinfo[i];
        if(!overlay->overlayinputport
           || !overlay->overlaybufinfo)
            continue;
        vdisp_addtocopy(&copyinfo, overlay->overlayinputport,
                        curbuf, overlay->overlaybufinfo);
        u16Fence=vdisp_copy_buf(&copyinfo);
        vdisp_rgn_add_rect(reserve_rgn,
                            overlay->overlayinputport->attr.u32OutX,
                            overlay->overlayinputport->attr.u32OutY,
                            overlay->overlayinputport->attr.u32OutWidth,
                            overlay->overlayinputport->attr.u32OutHeight);
        donecnt++;
    }
    return u16Fence;
}
#endif

static sba_bufhead_t *vdisp_get_outputbuf(vdisp_device_t *device,uint64_t u64pts)
{
    sba_bufhead_t * bufhd;
    MI_SYS_BufConf_t bufconf;
    MI_SYS_BufInfo_t * buf;
    MI_BOOL bBlocked;
    MI_SYS_FrameData_t *dataFrame;
    bufhd=sba_newbufhead();
    if(!bufhd){
        return NULL;
    }
    memset(bufhd, 0, sizeof(sba_bufhead_t));
    memset(&bufconf, 0, sizeof(bufconf));
    bufconf.eBufType=E_MI_SYS_BUFDATA_FRAME;
    bufconf.u64TargetPts=u64pts;
    bufconf.stFrameCfg.eFormat=device->outputport.attr.ePixelFormat;
    bufconf.stFrameCfg.eFrameScanMode=E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    bufconf.stFrameCfg.u16Width=device->outputport.attr.u32Width;
    bufconf.stFrameCfg.u16Height=device->outputport.attr.u32Height;
    buf=mi_sys_GetOutputPortBuf(device->devhandle, 0, 0, &bufconf, &bBlocked);
    if(!buf){
        goto fail_freebufhd;
    }
    if(!MI_VDISP_IMPL_IsSupBufInfo(buf)){
        goto finish_buf;
    }
    dataFrame=&buf->stFrameData;
    MI_SYS_BUG_ON(dataFrame->eTileMode!=E_MI_SYS_FRAME_TILE_MODE_NONE);
    MI_SYS_BUG_ON(dataFrame->eCompressMode!=E_MI_SYS_COMPRESS_MODE_NONE);
    MI_SYS_BUG_ON(dataFrame->eFrameScanMode!=E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE);
    MI_SYS_BUG_ON(dataFrame->eFieldType!=E_MI_SYS_FIELDTYPE_NONE);
    MI_VDISP_IMPL_InitBufHeadFromBufInfo(bufhd, buf, u64pts, 500000/*device->outputport.interval/2*/);
    return bufhd;
finish_buf:
    mi_sys_FinishBuf(buf);
fail_freebufhd:
    sba_freebufhead(bufhd);
    return NULL;
}

inline static MI_U64 vdisp_get_cur_stc(void)
{
    struct timespec sttime;
    MI_U64 u64Value;

    memset(&sttime, 0, sizeof(sttime));
    do_posix_clock_monotonic_gettime(&sttime);
    u64Value = timespec_to_ns(&sttime);
    do_div(u64Value,1000);
    return u64Value;
}
#if 0
inline static uint64_t vdisp_get_and_step_pts(vdisp_device_t *device)
{
    uint64_t u64pts;
    u64pts=device->outputport.attr.u64pts;
    device->outputport.attr.u64pts += device->outputport.interval;
    return u64pts;
}
#endif
static sba_bufhead_t *vdisp_getnextoutputbuf(vdisp_device_t *device, int step_pts)
{
    uint64_t u64pts;
    sba_bufhead_t *buf=NULL;
    mutex_lock(&device->mtx);
    u64pts=device->outputport.attr.u64pts;
    buf=vdisp_get_outputbuf(device, u64pts);
    if(buf || step_pts)
        device->outputport.attr.u64pts += device->outputport.interval;
    mutex_unlock(&device->mtx);
    return buf;
}

static void vdisp_fill_bufq(vdisp_device_t *device, int bwait)
{
    int i=0;
    sba_bufhead_t *buf=NULL;
#if VDISP_IMPL_DBG
    if(bwait)DBG_INFO("Dev %d: start fill bufq\n", vdisp_devid(device));
#endif
    i=sba_bufcnt((&device->bufqueue));
    while(i<VDISP_BUFQUEUE_DEPTH){
        buf=vdisp_getnextoutputbuf(device, i==0);
        if(!buf){
            if(bwait)
                continue;
            else
                break;
        }
        sba_queuebuf(&device->bufqueue, buf);
        i++;
    }
#if VDISP_IMPL_DBG
    if(bwait)DBG_INFO("Dev %d: end fill bufq\n", vdisp_devid(device));
#endif
}

static void vdisp_ondevstart(vdisp_device_t *device)
{
#if VDISP_IMPL_DBG
    DBG_INFO("VDISP DEVICE start...\n");
#endif
    vdisp_fill_bufq(device, 0);
    device->u64next_stc=vdisp_get_cur_stc();
#if VDISP_IMPL_DBG
    DBG_INFO("VDISP DEVICE start success\n");
#endif
}
static void vdisp_ondevstop(vdisp_device_t *device)
{
    //nothing to do now
#if VDISP_IMPL_DBG
    DBG_INFO("VDISP DEVICE stop...\n");
#endif
}
static void vdisp_ondevclose(vdisp_device_t *device)
{
    sba_bufhead_t *buf=NULL;
#if VDISP_IMPL_DBG
    DBG_INFO("VDISP DEVICE close...\n");
#endif
    while(sba_bufcnt((&device->bufqueue))>0){
        buf=sba_dequeuebuf(&device->bufqueue);
        if(buf){
            mi_sys_RewindBuf((MI_SYS_BufInfo_t *)(buf->priv));
            sba_freebufhead(buf);
        }else{
            msleep(1);
        }
    }
    if(device->prev_buf){
        sba_freebufhead(device->prev_buf);
        device->prev_buf=NULL;
    }
#if VDISP_IMPL_DBG
    DBG_INFO("VDISP DEVICE close success...\n");
#endif
}

static int vdisp_check_paused(vdisp_device_t *device, int *pre_paused)
{
    int paused=0;
    set_current_state(TASK_UNINTERRUPTIBLE);
    //要求进入暂停状态
    if(test_and_clear_bit(VDISP_WORKER_PAUSE, &device->wakeevent)){
        set_bit(VDISP_WORKER_PAUSED, &device->wakeevent);
        smp_mb();
    }
    //当前是暂停状态
    if(test_bit(VDISP_WORKER_PAUSED, &device->wakeevent)){
        if(!(*pre_paused)){
            //线程由运行进入暂停
            vdisp_ondevstop(device);
            (*pre_paused)=1;
        }
        schedule();
        paused=1;
    }else{
        if((*pre_paused)){
            //线程由暂停进入运行
            vdisp_ondevstart(device);
            (*pre_paused)=0;
        }
    }
    set_current_state(TASK_RUNNING);
    return paused;
}
void _vdisp_dump_bufq_status(vdisp_device_t *device,
        vdisp_allocation_t * allocation)
{
    vdisp_inputport_t *inputport;
    sba_subbufhead_t *cursub;
    MI_SYS_BufInfo_t *tmpbufinfo[10];
    sba_bufhead_t *tmpbuf;
    int ind=0,cnt=0;
    MI_U32 u32PortId=0, u32EndPortId=VDISP_MAX_INPUTPORT_NUM;

    if(allocation){
        cursub = allocation->subbufhd;
        if(!cursub || !(cursub->buf)
           || !(inputport = (vdisp_inputport_t *)(cursub->suballcinfo->priv))){
            DBG_PRINT("VDISP error: subbuf head was wrong!\n");
            goto dump_bufq;
        }
        u32PortId = inputport->inputport.u32PortId;
        u32EndPortId = u32PortId+1;
        DBG_PRINT("VDISP input port[%u] get buf: [%p, %lu], refcnt=%d\n", u32PortId,
                    cursub->buf, cursub->buf->buf_id,
                    allocation->sys_allocation.ref_cnt.counter);
    }
dump_bufq:
    while(u32PortId < u32EndPortId){
        cnt=0;
        while(cnt<10 && (tmpbufinfo[cnt]=mi_sys_GetInputPortBuf(device->devhandle, 0, u32PortId, 0))){
            vdisp_allocation_t *tmpallocation=vdisp_bufinfo2allocation(tmpbufinfo[cnt]);
            if(!tmpallocation || !vdisp_IsCorrectAllocation(tmpallocation)){
                DBG_PRINT("VDISP bindbufq[%u][%d]: error: not vdisp buffer\n",u32PortId,cnt);
            }else if(!(tmpallocation->subbufhd) || !(tmpallocation->subbufhd->buf)){
                DBG_PRINT("VDISP error: VDISP bindbufq[%u][%d] subbuf head was wrong!\n",
                    u32PortId,cnt);
            }else{
                DBG_PRINT("VDISP bindbufq[%u][%d]: [%p, %lu], refcnt=%d\n",u32PortId,cnt,
                    tmpallocation->subbufhd->buf, tmpallocation->subbufhd->buf->buf_id,
                    tmpallocation->sys_allocation.ref_cnt.counter);
            }
            cnt++;
        }
        for(;cnt>0;cnt--){
            mi_sys_RewindBuf(tmpbufinfo[cnt-1]);
        }
        u32PortId++;
    }
    ind=0;
    mutex_lock(&device->bufqueue.mtx);
    list_for_each_entry(tmpbuf, &(device->bufqueue.buf_queue), node) {
        DBG_PRINT("VDISP vdispbufq[%d]: [%p, %lu]\n",ind,
            tmpbuf, tmpbuf->buf_id);
        ind++;
        list_for_each_entry(cursub, &(tmpbuf->subbuf_list), node) {
            if(!(cursub->buf) || !(inputport = (vdisp_inputport_t *)(cursub->suballcinfo->priv))){
                DBG_PRINT("VDISP error: subbuf head was wrong!\n");
            }else{
                DBG_PRINT("VDISP input port[%u] subbuf: [%p, %lu]\n", inputport->inputport.u32PortId,
                    cursub->buf, cursub->buf->buf_id);
            }
        }
    }
    mutex_unlock(&device->bufqueue.mtx);
}
#define _VDISP_FINISH_PORT(i) \
        if(waitarray[i]){        \
            waitarray[i]=NULL;   \
            need_wait_cnt--;     \
        }
static inline int _vdisp_check_can_copy_prev(vdisp_device_t *device,
        vdisp_inputport_t *inputport, sba_bufhead_t *curbuf)
{
    //check buf sync
    if(inputport->sync_buf_id!=0 && (((long)(inputport->sync_buf_id))-((long)(curbuf->buf_id)))<0)
        inputport->sync_buf_id=0;
    if(inputport->sync_buf_id==0
        && device->prev_buf
        && device->prev_buf->ePixelFormat== curbuf->ePixelFormat){
        return 1;
    }
    return 0;
}
static void vdisp_try_process_outputbuf(vdisp_device_t *device, vdisp_copyinfo_t *copyarray)
{
    int i;
    vdisp_inputport_t *inputport;
    MI_SYS_BufInfo_t *bufinfo;
    int need_copy_cnt=0;
    int need_wait_cnt=0;
    vdisp_inputport_t *waitarray[VDISP_MAX_INPUTPORT_NUM];
    sba_bufhead_t *curbuf;
    MI_U16 u16Fence = 0, u16OverlayFence = 0, u16FillFence = 0;
    CLIPRGN reserve_rgn;
#if defined(ENABLE_VDISP_TRACE_PERF) && (ENABLE_VDISP_TRACE_PERF == 1)
    MI_U64 u64BeginProcess = 0;
    MI_U64 u64EndProcess = 0;
    VDISP_PERF_TIME(&u64BeginProcess);
#endif
#if VDISP_SUPPORT_OVERLAYINPUTPORT
    mutex_lock(&device->mtx);
    _vdisp_try_update_overlay(device);
    mutex_unlock(&device->mtx);
#endif
    if(0>((MI_S64)vdisp_get_cur_stc()-(MI_S64)(device->u64next_stc)))
        return;

    device->u64next_stc += device->outputport.interval;

    //0. 获取一张大buf并插入队列
    vdisp_fill_bufq(device, 0);

    //1. 取出要处理的第一张大buf
    curbuf=sba_firstbuf(&device->bufqueue);
    if(!curbuf){
#if VDISP_IMPL_DBG_PROCESS
        DBG_INFO("Dev %d: No output buffer to handle for pts %lld\n",
                vdisp_devid(device),
                device->u64next_stc - device->outputport.interval);
#endif
        goto process_fill_bufq;
    }
#if VDISP_IMPL_DBG_PROCESS
    DBG_INFO("Dev %d: begin handle for pts %lld\n",
            vdisp_devid(device),
            device->u64next_stc - device->outputport.interval);
#endif

    //2. 禁止从要处理的第一张大buf中再次分配子buf
    sba_skipallocfromfirst(&device->bufqueue);
    //3. 初始化要处理的input列表，第一次需要检查所有的input port
    for(need_wait_cnt=0;need_wait_cnt<VDISP_MAX_INPUTPORT_NUM;need_wait_cnt++){
        waitarray[need_wait_cnt]=&device->inputport[need_wait_cnt];
    }
    InitClipRgn(&reserve_rgn, NULL);

try_again:
    need_copy_cnt=0;
    mutex_lock(&device->mtx);
    //4. 遍历所有要处理的input port，对其进行处理
    for(i=0;i<VDISP_MAX_INPUTPORT_NUM && need_wait_cnt>0;i++){
        int bhassubbuf=0;
        vdisp_allocation_t * allocation;
        sba_subbufhead_t *cursub;
        inputport=waitarray[i];
        //4.1 该input port已经处理过
        if(!inputport)
            continue;

        //4.2 该input port尚未初始化，不需处理
        if(inputport->status==VDISP_INPUTPORT_UNINIT){
            _VDISP_FINISH_PORT(i);
            continue;
        }

        //4.3 从input port获取ready的buf
        bufinfo=mi_sys_GetInputPortBuf(device->devhandle, 0, i, 0);
        inputport->u64Try++;
        //4.4 检查该input port是否已经从当前大buf分配子buf
        if(sba_findsubbuf(&device->bufqueue, curbuf, inputport))
            bhassubbuf=1;

        //4.5 该input port没有从当前大buf分配过子buf，这次循环对其处理完毕，
        //     后续无需再次处理该input port
        if(!bhassubbuf){
            _VDISP_FINISH_PORT(i);
        }

        //4.6 没拿到buf，且未分配过子buf，需要copy前一张
        if(!bufinfo && !bhassubbuf){
            if(inputport->status != VDISP_INPUTPORT_ENABLED){
                continue;
            }
#if VDISP_IMPL_DBG_PROCESS
            DBG_INFO("Dev %d-inputport %d: do not get buf and not allocate sub buf\n",
                    vdisp_devid(device),i);
#endif
            //如果属性更改过，则不能copy
            if(_vdisp_check_can_copy_prev(device, inputport, curbuf)){
                vdisp_copyinfo_t *copyinfo=&copyarray[need_copy_cnt];
                vdisp_addtocopy_prev(copyinfo, inputport, curbuf, device->prev_buf, curbuf->ePixelFormat);
                need_copy_cnt++;
                vdisp_rgn_add_rect(&reserve_rgn, inputport->attr.u32OutX, inputport->attr.u32OutY,
                                              inputport->attr.u32OutWidth, inputport->attr.u32OutHeight);
            }
            continue;
        }

        //4.7 这次未ready，但分配过子buf，下次再来try
        if(!bufinfo && bhassubbuf){
#if VDISP_IMPL_DBG_PROCESS
            DBG_INFO("Dev %d-inputport %d: do not get buf but allocate sub buf\n",
                    vdisp_devid(device),i);
#endif
            continue;
        }

        //4.8 该input port是disable的，将拿到的buf直接finish，这次处理中后续不再检查它
        if(inputport->status != VDISP_INPUTPORT_ENABLED){
            //the allocation is disabled, we need to cost the bufs allocated before disable
            mi_sys_FinishBuf(bufinfo);
            _VDISP_FINISH_PORT(i);
            continue;
        }

        allocation=vdisp_bufinfo2allocation(bufinfo);
        //4.10 检查是不是从custom allocator分配出去的，如果不是则不处理
        //有时候在我们注册custom allocator之前，前端可能已经获取了一些buf，
        //这里需要识别出来并忽略它
        if(!allocation || !vdisp_IsCorrectAllocation(allocation)){
#if VDISP_IMPL_DBG_PROCESS
            DBG_INFO("Dev %d-inputport %d: get buf not from our allocator: allocation=%p, bufinfo=[%llx]\n",
                vdisp_devid(device),i,
                allocation, bufinfo->stFrameData.phyAddr[0]);
#endif
            mi_sys_FinishBuf(bufinfo);
            continue;
        }

        cursub=allocation->subbufhd;
        MI_SYS_BUG_ON(!(cursub->buf));
        //4.11 获取到的子buf不是当前大buf的
        //在我们处理当前大buf时，该input port从后面的大buf分配了子buf，
        //并且数据已经处理完毕，被我们获取到
        if(cursub->buf!=curbuf){
#if VDISP_IMPL_DBG_PROCESS
            DBG_INFO("Dev %d-inputport %d: get buf from our allocator but not current buf\n",
                    vdisp_devid(device),i);
#endif
            if(bhassubbuf){
                _vdisp_dump_bufq_status(device, allocation);
                _VDISP_FINISH_PORT(i);
            }
            //还回去，下次再处理
            mi_sys_RewindBuf(bufinfo);
            //如果属性更改过，则不能copy
            if(_vdisp_check_can_copy_prev(device, inputport, curbuf)){
                vdisp_copyinfo_t *copyinfo=&copyarray[need_copy_cnt];
                vdisp_addtocopy_prev(copyinfo, inputport, curbuf, device->prev_buf, curbuf->ePixelFormat);
                need_copy_cnt++;
                vdisp_rgn_add_rect(&reserve_rgn, inputport->attr.u32OutX, inputport->attr.u32OutY,
                                              inputport->attr.u32OutWidth, inputport->attr.u32OutHeight);
            }
            continue;
        }
        //4.12 该子buf是当前大buf的子buf，直接finish就好
        vdisp_rgn_add_rect(&reserve_rgn,
                                        sba_subbufx(allocation->subbufhd),
                                        sba_subbufy(allocation->subbufhd),
                                        sba_subbufwidth(allocation->subbufhd),
                                        sba_subbufheight(allocation->subbufhd));
        mi_sys_FinishBuf(bufinfo);
        inputport->u64RecvOk++;
        _VDISP_FINISH_PORT(i);
    }
    mutex_unlock(&device->mtx);
    //5. 在下一次检查剩余input，先复制
    for(i=0;i<need_copy_cnt;i++){
        vdisp_copyinfo_t *copyinfo=&copyarray[i];
        u16Fence=vdisp_copy_buf(copyinfo);
        //vdisp_finish_copy(u16Fence);
        if(copyinfo->finish_buf)
            mi_sys_FinishBuf(copyinfo->finish_buf);
    }
    //6.还有input port需要处理
    if(need_wait_cnt>0)
    {
        schedule_timeout_interruptible(1);
        goto try_again;
    }

    //7. 弹出当前大buf
    //this buffer may be kept by other input port
    {
        MI_U64 u64DeqStart=0;
        int bDeqChecked=0;
        while(!(curbuf=sba_dequeuebuf(&device->bufqueue))){
            MI_U64 u64CurTime;
            if(kthread_should_stop()){
#if VDISP_IMPL_DBG
                DBG_INFO("worker thread need to exit\n");
#endif
                EmptyClipRgn(&reserve_rgn);
                return;
            }
            if(!bDeqChecked){
                u64CurTime = vdisp_get_cur_stc();
                if(u64DeqStart==0)
                    u64DeqStart = u64CurTime;
                else if(((MI_S64)(u64DeqStart+3000000)-(MI_S64)u64CurTime)<0){
                    DBG_PRINT("VDISP deque buf overtime\n");
                    _vdisp_dump_bufq_status(device, NULL);
                    bDeqChecked=1;
                }
            }
            schedule_timeout_interruptible(1);
        }
    }
#if VDISP_SUPPORT_OVERLAYINPUTPORT
    mutex_lock(&device->mtx);
    u16OverlayFence = _vdisp_blend_overlay(device, curbuf, &reserve_rgn);
    mutex_unlock(&device->mtx);
#endif
    u16FillFence = vdisp_clear_buf((MI_SYS_BufInfo_t *)(curbuf->priv), &reserve_rgn, device->outputport.attr.u32BgColor);
    if (u16FillFence || u16OverlayFence || u16Fence)
    {
        vdisp_finish_copy(MAX_FENCE(u16Fence, u16OverlayFence, u16FillFence));
    }
    EmptyClipRgn(&reserve_rgn);

    //8. 释放前一张大buf的bufhead结构体
    if(device->prev_buf){
        sba_freebufhead(device->prev_buf);
        device->prev_buf = NULL;
    }
    device->prev_buf = curbuf;
    //9. 推出这张输出大buf
    mi_sys_FinishBuf((MI_SYS_BufInfo_t *)(curbuf->priv));
#if defined(ENABLE_VDISP_TRACE_PERF) && (ENABLE_VDISP_TRACE_PERF == 1)
    VDISP_PERF_TIME(&u64EndProcess);
    printk("Vdisp: process: %llu u16Fence(%d) u16OverlayFence(%d) u16FillFence(%d) MAX(%d).\n", u64EndProcess - u64BeginProcess,
        u16Fence, u16OverlayFence, u16FillFence, MAX_FENCE(u16Fence, u16OverlayFence, u16FillFence));
#endif
    device->outputport.u64SendOk++;
#if VDISP_IMPL_DBG_PROCESS
    DBG_INFO("Dev %d: end handle for pts %lld\n",
            vdisp_devid(device),
            device->u64next_stc - device->outputport.interval);
#endif

    //10. 获取一张大buf并插入队列
process_fill_bufq:
    vdisp_fill_bufq(device, 0);
}

static int vdisp_device_worker(void *data)
{
    vdisp_device_t *device=(vdisp_device_t *)data;
    int pre_paused=1;
    vdisp_copyinfo_t *copyarray=kmalloc(sizeof(vdisp_copyinfo_t)*VDISP_MAX_INPUTPORT_NUM,GFP_KERNEL);
    MI_SYS_BUG_ON(!copyarray);
    while(1){
        //检查是否要退出线程
        if(kthread_should_stop())
            break;
        //检查是否要暂停线程
        if(vdisp_check_paused(device, &pre_paused)){
            continue;
        }
        //检查是否时间到并处理
        vdisp_try_process_outputbuf(device, copyarray);
        schedule_timeout_interruptible(1);
    }
    vdisp_ondevclose(device);
    kfree(copyarray);
    return 0;
}


static MI_S32 vdisp_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    vdisp_device_t *dev=(vdisp_device_t *)pUsrData;
    vdisp_allocator_t *allocator;
    vdisp_inputport_t *inputport;
    int ret=-1;
#if VDISP_IMPL_DBG
    DBG_INFO("\n");
#endif
    if(!VDISP_VALID_MODID(pstChnCurPort->eModId)
       || !VDISP_VALID_DEVID(pstChnCurPort->u32DevId)
       || !VDISP_VALID_CHNID(pstChnCurPort->u32ChnId)
       || !VDISP_VALID_INPUTPORTID(pstChnCurPort->u32PortId)){
        DBG_ERR("Invalid cur port\n");
        return -1;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        goto exit;
    }
    mutex_lock(&dev->mtx);
    inputport=&dev->inputport[pstChnCurPort->u32PortId];
    if(dev->status==VDISP_DEVICE_UNINIT){
        DBG_ERR("Device not open\n");
        goto exit_device;
    }
    MI_SYS_BUG_ON(dev->inputport[pstChnCurPort->u32PortId].bbind);
    //overlay inputport use normal memory allocator
    if(!VDISP_IS_OVERLAYINPUTPORTID(pstChnCurPort->u32PortId)){
        allocator=vdisp_allcator_create(VDISP_PORT_INPUT,
                           &dev->inputport[pstChnCurPort->u32PortId].inputport);
        if(!allocator){
            DBG_ERR("create allocator fail\n");
            goto unreg_subbuf;
        }
        if(0>mi_sys_SetPeerOutputPortCusAllocator(dev->devhandle, 0,
                 pstChnCurPort->u32PortId, &allocator->sys_allocator)){
            DBG_ERR("Set custom allocator fail\n");
            goto free_allocator;
        }
    }
    dev->inputport[pstChnCurPort->u32PortId].bindport=*pstChnPeerPort;
    dev->inputport[pstChnCurPort->u32PortId].bbind=1;

    if(dev->inputport[pstChnCurPort->u32PortId].status==VDISP_INPUTPORT_ENABLED){
        sba_enable(&dev->bufqueue,inputport->sub_buf_id, 1);
    }
    ret=0;
    goto exit_device;
free_allocator:
    kfree(allocator);
unreg_subbuf:
    if(inputport->sub_buf_id>=0){
        sba_unregistersubbuf(&dev->bufqueue, inputport->sub_buf_id);
        inputport->sub_buf_id=-1;
    }
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
static MI_S32 vdisp_OnUnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    vdisp_device_t *dev=(vdisp_device_t *)pUsrData;
    int ret=-1;
    vdisp_inputport_t *inputport;
#if VDISP_IMPL_DBG
    DBG_INFO("\n");
#endif
    if(!VDISP_VALID_MODID(pstChnCurPort->eModId)
       || !VDISP_VALID_DEVID(pstChnCurPort->u32DevId)
       || !VDISP_VALID_CHNID(pstChnCurPort->u32ChnId)
       || !VDISP_VALID_INPUTPORTID(pstChnCurPort->u32PortId)){
        DBG_ERR("Invalid cur port\n");
        return -1;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        goto exit;
    }

    mutex_lock(&dev->mtx);
    inputport=&dev->inputport[pstChnCurPort->u32PortId];
    if(dev->status==VDISP_DEVICE_UNINIT){
        DBG_ERR("Device not open\n");
        goto exit_device;
    }
    if(!inputport->bbind){
        DBG_ERR("Input port not bound\n");
        goto exit_device;
    }
    if(!VDISP_IS_OVERLAYINPUTPORTID(pstChnCurPort->u32PortId)){
        if(0>mi_sys_SetPeerOutputPortCusAllocator(dev->devhandle, 0,
                 pstChnCurPort->u32PortId, NULL)){
            DBG_ERR("Set Null Custom allocator fail\n");
            goto exit_device;
        }
    }

    memset(&dev->inputport[pstChnCurPort->u32PortId].bindport, 0, sizeof(MI_SYS_ChnPort_t));
    dev->inputport[pstChnCurPort->u32PortId].bbind=0;
    ret=0;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

static MI_S32 vdisp_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    vdisp_device_t *dev=(vdisp_device_t *)pUsrData;
    int ret=-1;
    if(!VDISP_VALID_MODID(pstChnCurPort->eModId)
       || !VDISP_VALID_DEVID(pstChnCurPort->u32DevId)
       || !VDISP_VALID_CHNID(pstChnCurPort->u32ChnId)
       || !VDISP_VALID_OUTPUTPORTID(pstChnCurPort->u32PortId))
        return -1;
    down(&module_sem);
    if(!_vdisp_module.binited)
        goto exit;

    mutex_lock(&dev->mtx);
    if(dev->status==VDISP_DEVICE_UNINIT || dev->status==VDISP_DEVICE_START)
        goto exit_device;
    dev->outputport.bbind=1;
    ret=0;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
static MI_S32 vdisp_OnUnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    vdisp_device_t *dev=(vdisp_device_t *)pUsrData;
    int ret=-1;
    if(!VDISP_VALID_MODID(pstChnCurPort->eModId)
       || !VDISP_VALID_DEVID(pstChnCurPort->u32DevId)
       || !VDISP_VALID_CHNID(pstChnCurPort->u32ChnId)
       || !VDISP_VALID_OUTPUTPORTID(pstChnCurPort->u32PortId))
        return -1;
    down(&module_sem);
    if(!_vdisp_module.binited)
        goto exit;

    mutex_lock(&dev->mtx);
    if(dev->status==VDISP_DEVICE_UNINIT || dev->status==VDISP_DEVICE_START)
        goto exit_device;
    dev->outputport.bbind=0;
    ret=0;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

vdisp_device_t* MI_VDISP_IMPL_GetDevice(MI_VDISP_DEV DevId)
{
    return &_vdisp_module.dev[DevId];
}

void MI_VDISP_IMPL_InitBufInfoFromSubBuf(MI_SYS_BufInfo_t *bufinfo,
    sba_subbufhead_t* subbufhd,
    MI_U64 u64TargetPts)
{
    MI_SYS_FrameData_t *dataFrame, *bigDataFrame;
    MI_U32 u32Offset=0;
    int cnt;
    int i;
    sba_bufhead_t *buf=subbufhd->buf;
    MI_SYS_BufInfo_t *bigbufinfo=(MI_SYS_BufInfo_t *)buf->priv;

    dataFrame=&bufinfo->stFrameData;
    bigDataFrame=&bigbufinfo->stFrameData;
    memset(bufinfo, 0, sizeof(MI_SYS_BufInfo_t));
    cnt=buf->planenum;
    bufinfo->eBufType=E_MI_SYS_BUFDATA_FRAME;
    bufinfo->u64Pts=u64TargetPts;
    dataFrame->eTileMode=bigDataFrame->eTileMode;
    dataFrame->ePixelFormat=bigDataFrame->ePixelFormat;
    dataFrame->eCompressMode=bigDataFrame->eCompressMode;
    dataFrame->eFrameScanMode=bigDataFrame->eFrameScanMode;
    dataFrame->eFieldType=bigDataFrame->eFieldType;
    dataFrame->u16Width=sba_subbufvalidwidth(subbufhd);
    dataFrame->u16Height=sba_subbufvalidheight(subbufhd);

    for(i=0;i<cnt;i++){
        dataFrame->u32Stride[i]=subbufhd->buf->bufplane[i].stride;
        u32Offset=buf->bufplane[i].pixel_offset(&buf->bufplane[i], subbufhd->suballcinfo->x, subbufhd->suballcinfo->y);
        dataFrame->phyAddr[i]=buf->bufplane[i].paddr+u32Offset;
        if(subbufhd->buf->bufplane[i].vaddr)
            dataFrame->pVirAddr[i]=buf->bufplane[i].vaddr+u32Offset;
        else
            dataFrame->pVirAddr[i]=NULL;
    }

}

void MI_VDISP_IMPL_InitBufHeadFromBufInfo(
    sba_bufhead_t * bufhd,
    MI_SYS_BufInfo_t *bufinfo,
    uint64_t u64pts,
    uint32_t pts_tolerance)
{
    int cnt=MI_VDISP_IMPL_GetBufPlaneNum(bufinfo->stFrameData.ePixelFormat);
    int i;
    bufhd->priv=bufinfo;
    bufhd->type=VIDSP_BUF_TYPE_NORMAL;
    bufhd->u64pts=u64pts;
    bufhd->planenum=cnt;
    bufhd->width=bufinfo->stFrameData.u16Width;
    bufhd->height=bufinfo->stFrameData.u16Height;
    bufhd->ePixelFormat=bufinfo->stFrameData.ePixelFormat;
    bufhd->pts_tolerance=pts_tolerance;
    for(i=0;i<cnt;i++){
        bufhd->bufplane[i].paddr=bufinfo->stFrameData.phyAddr[i];
        bufhd->bufplane[i].vaddr=bufinfo->stFrameData.pVirAddr[i];
        bufhd->bufplane[i].stride=bufinfo->stFrameData.u32Stride[i];
    }
    if(bufinfo->stFrameData.ePixelFormat==E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
        bufhd->bufplane[0].pixel_offset=vdisp_pixel_offset_yuv422_yuyv;
    else if(bufinfo->stFrameData.ePixelFormat==E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420){
        bufhd->bufplane[0].pixel_offset=vdisp_pixel_offset_yuv420_y;
        bufhd->bufplane[1].pixel_offset=vdisp_pixel_offset_yuv420_uv;
    }else
        MI_SYS_BUG();
}
MI_SYS_BufInfo_t * MI_VDISP_IMPL_GetBufInfoFromBufhead(sba_bufhead_t * bufhd)
{
    return (MI_SYS_BufInfo_t *)(bufhd->priv);
}

MI_S32 MI_VDISP_IMPL_Init(void)
{
    int i=0;
    int ret=-1;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Module Init\n");
#endif
    down(&module_sem);
    if(_vdisp_module.binited){
        DBG_ERR("VDISP IMPL: inited already\n");
        ret=MI_VDISP_ERR_MOD_INITED;
        goto exit;
    }
    if(0>sba_init()){
        DBG_ERR("VDISP IMPL: subbuf allocator init fail\n");
        ret=MI_VDISP_ERR_FAIL;
        goto exit;
    }
    vdisp_allcator_init();

    memset(&_vdisp_module,0,sizeof(_vdisp_module));
    for(i=0;i<VDISP_MAX_DEVICE_NUM;i++){
        mutex_init(&_vdisp_module.dev[i].mtx);
    }
    MI_GFX_Open();
    region_substract_init();
    _vdisp_module.binited=1;
    ret=MI_SUCCESS;
exit:
    up(&module_sem);
    return ret;
}
MI_S32 MI_VDISP_IMPL_Exit(void)
{
    int ret=-1;
    int i;
    vdisp_device_t *dev=NULL;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Module Exit\n");
#endif
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    for(i=0;i<VDISP_MAX_DEVICE_NUM;i++){
        dev=&(_vdisp_module.dev[i]);
        if(dev->status != VDISP_DEVICE_UNINIT){
            DBG_ERR("Device %d not closed\n", i);
            ret=MI_VDISP_ERR_DEV_NOT_CLOSE;
            goto exit;
        }
    }

    if(0>sba_deinit()){
        DBG_ERR("Sub buf allocator deinit fail\n");
        ret=MI_VDISP_ERR_FAIL;
        goto exit;
    }

    vdisp_allcator_deinit();

    for(i=0;i<VDISP_MAX_DEVICE_NUM;i++){
        mutex_destroy(&_vdisp_module.dev[i].mtx);
    }
    MI_GFX_Close();
    region_substract_deinit();
    _vdisp_module.binited=0;
    ret=MI_SUCCESS;
exit:
    up(&module_sem);
    return ret;
}

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VDISP_PROCFS_DEBUG ==1)
static const char *mi_vdisp_DevStatus2Str(vdisp_device_status_e status)
{
    switch (status)
    {
        case VDISP_DEVICE_UNINIT:
            return "UnInit";
        case VDISP_DEVICE_INIT:
            return "Init";
        case VDISP_DEVICE_START:
            return "Start";
        case VDISP_DEVICE_STOP:
            return "Stop";
        default:
            return "Unkown";
    }
}

static const char *mi_vdisp_InportStatus2Str(vdisp_inputport_status_e status)
{
    switch (status)
    {
        case VDISP_INPUTPORT_UNINIT:
            return "UnInit";
        case VDISP_INPUTPORT_INIT:
            return "Init";
        case VDISP_INPUTPORT_ENABLED:
            return "Enabled";
        case VDISP_INPUTPORT_DISABLED:
            return "Disabled";
        default:
            return "Unkown";
    }
}

MI_S32 mi_vdisp_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    vdisp_device_t *dev = NULL;
    dev = &(_vdisp_module.dev[u32DevId]);

    handle.OnPrintOut(handle, "\n");
    handle.OnPrintOut(handle, "======================================Private Vdisp%d Info ======================================\n", u32DevId);
    handle.OnPrintOut(handle, "%11s%10s%11s\n", "DevStatus", "AllocCnt", "WakeEvent");
    handle.OnPrintOut(handle, "%11s", mi_vdisp_DevStatus2Str(dev->status));
    handle.OnPrintOut(handle, "%10d", dev->allocator_cnt);
    handle.OnPrintOut(handle, "%11d\n", dev->wakeevent);

    return MI_SUCCESS;
}

MI_S32 mi_vdisp_OnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_S32 i = 0;
    vdisp_device_t *dev = NULL;
    vdisp_inputport_t *inputport = NULL;
    if (!VDISP_VALID_DEVID(u32DevId))
    {
        DBG_ERR("Invalid DevId=%d\n", u32DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }

    dev = &(_vdisp_module.dev[u32DevId]);

    handle.OnPrintOut(handle, "------------------------------------------------------- Input Port Info ------------------------------------------------\n");
    handle.OnPrintOut(handle, "%8s%12s%6s%6s%6s%6s%11s%15s%15s\n", "PortID", "PortStatus", "ChnX", "ChnY", "ChnW", "ChnH",
        "IsFreeRun", "TryOk", "RecvOk");
    for (i = 0; i < VDISP_TOTAL_INPUTPORT_NUM; i++)
    {
        inputport = &dev->inputport[i];
        handle.OnPrintOut(handle, "%8d", i);
        handle.OnPrintOut(handle, "%12s", mi_vdisp_InportStatus2Str(inputport->status));
        handle.OnPrintOut(handle, "%6d", inputport->attr.u32OutX);
        handle.OnPrintOut(handle, "%6d", inputport->attr.u32OutY);
        handle.OnPrintOut(handle, "%6d", inputport->attr.u32OutWidth);
        handle.OnPrintOut(handle, "%6d", inputport->attr.u32OutHeight);
        handle.OnPrintOut(handle, "%11d", inputport->attr.s32IsFreeRun);
        handle.OnPrintOut(handle, "%15llu", inputport->u64Try);
        handle.OnPrintOut(handle, "%15llu\n", inputport->u64RecvOk);
        //handle.OnPrintOut(handle, "%15d\n", inputport->sub_buf_id);
    }

    return MI_SUCCESS;
}

MI_S32 mi_vdisp_OnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    vdisp_device_t *dev = NULL;
    dev = &(_vdisp_module.dev[u32DevId]);
    handle.OnPrintOut(handle, "------------------------------------------------------ Output Port Info -------------------------------------------------\n");
    handle.OnPrintOut(handle, "%8s%13s%9s%10s%9s%7s%8s%15s\n", "Inited", "FrmInterval", "BgColor", "PixelFmt", "FrmRate", "Width", "Height", "SendOk");
    handle.OnPrintOut(handle, "%8d", dev->outputport.binited);
    handle.OnPrintOut(handle, "%13llu", dev->outputport.interval);
    handle.OnPrintOut(handle, "%9d", dev->outputport.attr.u32BgColor);
    handle.OnPrintOut(handle, "%10d", dev->outputport.attr.ePixelFormat);
    handle.OnPrintOut(handle, "%9d", dev->outputport.attr.u32FrmRate);
    handle.OnPrintOut(handle, "%7d", dev->outputport.attr.u32Width);
    handle.OnPrintOut(handle, "%8d", dev->outputport.attr.u32Height);
    handle.OnPrintOut(handle, "%15llu\n", dev->outputport.u64SendOk);

    return MI_SUCCESS;
}

MI_S32 mi_vdisp_DebugSetPortStatus(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8PortId = 0;
    MI_U8 u8Status = 0;

    if(argc < 3)
    {
        DBG_ERR("1.PortID 2.Status[0/1]\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    u8PortId = (MI_U8)simple_strtoul(argv[1], NULL, 10);
    u8Status = (MI_U8)simple_strtoul(argv[2], NULL, 10);
    handle.OnPrintOut(handle, "PortID(%d), Status(%d)\r\n", u8PortId, u8Status);

    if (1 == u8Status)
    {
        MI_VDISP_IMPL_EnableInputPort(u32DevId, u8PortId);
    }
    else if (0 == u8Status)
    {
        MI_VDISP_IMPL_DisableInputPort(u32DevId, u8PortId);
    }

    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 mi_vdisp_DebugSetPortFreeRun(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8PortId = 0;
    MI_U8 u8isFreeRun = 0;
    MI_VDISP_InputPortAttr_t stInputPortAttr;

    if(argc < 3)
    {
        DBG_ERR("1.PortID 2.isFreeRun[0/1]\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    u8PortId = (MI_U8)simple_strtoul(argv[1], NULL, 10);
    u8isFreeRun = (MI_U8)simple_strtoul(argv[2], NULL, 10);
    handle.OnPrintOut(handle, "PortID(%d), FreeRun(%d)\r\n", u8PortId, u8isFreeRun);

    if (MI_SUCCESS == MI_VDISP_IMPL_GetInputPortAttr(u32DevId, u8PortId, &stInputPortAttr))
    {
        stInputPortAttr.s32IsFreeRun = u8isFreeRun;
        if (MI_SUCCESS == MI_VDISP_IMPL_SetInputPortAttr(u32DevId, u8PortId, &stInputPortAttr))
        {
            DBG_INFO("Vdisp set input port attr dev(%d) port(%d)\n", u32DevId, u8PortId);
        }
    }

    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 mi_vdisp_DebugSetDevPause(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8Pause = 0;

    if(argc < 2)
    {
        DBG_ERR("1.Pause[0/1]\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    u8Pause = (MI_U8)simple_strtoul(argv[1], NULL, 10);
    handle.OnPrintOut(handle, "u32DevId(%d), Pause(%d)\r\n", u32DevId, u8Pause);

    if (1 == u8Pause)
    {
        MI_VDISP_IMPL_StartDev(u32DevId);
    }
    else if (0 == u8Pause)
    {
        MI_VDISP_IMPL_StopDev(u32DevId);
    }

    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

#endif //MI_SYS_PROC_FS_DEBUG

MI_S32 MI_VDISP_IMPL_OpenDevice(MI_VDISP_DEV DevId)
{
    vdisp_device_t *dev=NULL;
    mi_sys_ModuleDevInfo_t modinfo;
    mi_sys_ModuleDevBindOps_t bindops;
    int i;
    int ret=-1;
    #ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    #endif
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Open device %d\n", DevId);
#endif
    if(!VDISP_VALID_DEVID(DevId)){
        DBG_ERR("Invalid Device Id\n");
        return MI_VDISP_ERR_INVALID_DEVID;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(dev->status != VDISP_DEVICE_UNINIT){
        DBG_ERR("Device Opened already\n");
        ret=MI_VDISP_ERR_DEV_OPENED;
        goto exit_device;
    }

    dev->wakeevent=0;
    dev->prev_buf=NULL;
    dev->u64next_stc=0;
    atomic_set(&dev->allocator_cnt, 0);

    //input/output port
    for(i=0;i<VDISP_TOTAL_INPUTPORT_NUM;i++){
        dev->inputport[i].inputport.eModId=E_MI_MODULE_ID_VDISP;
        dev->inputport[i].inputport.u32DevId=DevId;
        dev->inputport[i].inputport.u32ChnId=0;
        dev->inputport[i].inputport.u32PortId=i;
        dev->inputport[i].sub_buf_id=-1;
        dev->inputport[i].u64Try = 0;
        dev->inputport[i].u64RecvOk = 0;
    }
    dev->outputport.outputport.eModId=E_MI_MODULE_ID_VDISP;
    dev->outputport.outputport.u32DevId=DevId;
    dev->outputport.outputport.u32ChnId=0;
    dev->outputport.outputport.u32PortId=0;
    dev->outputport.u64SendOk = 0;

    if(0>sba_bufqinit(&dev->bufqueue)){
        DBG_ERR("sub buf allocator bufq init fail\n");
        ret=MI_VDISP_ERR_FAIL;
        goto exit_device;
    }

    //create thread
    set_bit(VDISP_WORKER_PAUSE, &dev->wakeevent); //initial paused
    dev->work_thread=kthread_run(vdisp_device_worker, dev, "vdisp-dev%d",DevId);
    if(IS_ERR(dev->work_thread)){
        DBG_ERR("Create work thread fail\n");
        ret=MI_VDISP_ERR_FAIL;
        goto exit_device;
    }

    modinfo.eModuleId=E_MI_MODULE_ID_VDISP;
    modinfo.u32DevId=DevId;
    modinfo.u32DevChnNum=1;
    modinfo.u32InputPortNum=VDISP_TOTAL_INPUTPORT_NUM;
    modinfo.u32OutputPortNum=1;

    bindops.OnBindInputPort=vdisp_OnBindInputPort;
    bindops.OnBindOutputPort=vdisp_OnBindOutputPort;
    bindops.OnUnBindInputPort=vdisp_OnUnBindInputPort;
    bindops.OnUnBindOutputPort=vdisp_OnUnBindOutputPort;
    bindops.OnOutputPortBufRelease=NULL;

#ifdef MI_SYS_PROC_FS_DEBUG
    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
#if(MI_VDISP_PROCFS_DEBUG ==1)
    pstModuleProcfsOps.OnDumpDevAttr = mi_vdisp_OnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = mi_vdisp_OnDumpInputPortAttr;
    pstModuleProcfsOps.OnDumpOutPortAttr = mi_vdisp_OnDumpOutPortAttr;
    pstModuleProcfsOps.OnHelp = NULL;
#else
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif

#endif
    dev->devhandle=mi_sys_RegisterDev(&modinfo, &bindops, dev
                                        #ifdef MI_SYS_PROC_FS_DEBUG
                                        , &pstModuleProcfsOps
                                        ,MI_COMMON_GetSelfDir
                                        #endif
                                      );
    if(NULL==dev->devhandle){
        DBG_ERR("Register Module Device fail\n");
        ret=MI_VDISP_ERR_FAIL;
        goto fail_stopthread;
    }
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDISP_PROCFS_DEBUG == 1)
    mi_sys_RegistCommand("setportstatus", 2, mi_vdisp_DebugSetPortStatus, dev->devhandle);
    mi_sys_RegistCommand("setportfreerun", 2, mi_vdisp_DebugSetPortFreeRun, dev->devhandle);
    mi_sys_RegistCommand("setdevpause", 1, mi_vdisp_DebugSetDevPause, dev->devhandle);
#endif
    if(_vdisp_init_inputports_allocator(dev)<0){
        DBG_ERR("Set inputport allocator fail\n");
        ret=MI_VDISP_ERR_FAIL;
        goto unreigister_dev;
    }
    mutex_unlock(&dev->mtx);
#if VDISP_IMPL_DBG
    DBG_INFO("Waiting work thread pause\n");
#endif
    while(!test_bit(VDISP_WORKER_PAUSED, &dev->wakeevent)){
        wake_up_process(dev->work_thread);
        msleep(1);
    }
#if VDISP_IMPL_DBG
    DBG_INFO("Waiting work thread pause success\n");
#endif
    dev->status=VDISP_DEVICE_INIT;
    ret=MI_SUCCESS;
    goto exit;
unreigister_dev:
    mi_sys_UnRegisterDev(dev->devhandle);
fail_stopthread:
    kthread_stop(dev->work_thread);
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
MI_S32 MI_VDISP_IMPL_CloseDevice(MI_VDISP_DEV DevId)
{
    vdisp_device_t *dev=NULL;
    vdisp_inputport_t *inputport;
    int ret=-1;
    int i;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Close device %d\n", DevId);
#endif
    if(!VDISP_VALID_DEVID(DevId)){
        DBG_ERR("Invalid DevId=%d\n", DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not init\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(VDISP_DEVICE_INIT!=dev->status && VDISP_DEVICE_STOP!=dev->status){
        DBG_ERR("Device not stopped: %d\n", dev->status);
        ret=MI_VDISP_ERR_DEV_NOT_STOP;
        goto exit_device;
    }
    for(i=0;i<VDISP_TOTAL_INPUTPORT_NUM;i++){
        inputport=&dev->inputport[i];
        if(inputport->status == VDISP_INPUTPORT_ENABLED){
            DBG_ERR("Inputport %d is still enabled\n", i);
            ret=MI_VDISP_ERR_PORT_NOT_DISABLE;
            goto exit_device;
        }
        if(inputport->bbind) {
            DBG_ERR("Inputport %d is still bound\n", i);
            ret=MI_VDISP_ERR_PORT_NOT_UNBIND;
            goto exit_device;
        }
    }
    for(i=0;i<VDISP_TOTAL_INPUTPORT_NUM;i++){
        inputport=&dev->inputport[i];
        if(inputport->status != VDISP_INPUTPORT_UNINIT){
            inputport->status = VDISP_INPUTPORT_UNINIT;
        }
    }
    if(dev->work_thread){
#if VDISP_IMPL_DBG
        DBG_INFO("Stopping work thread\n");
#endif
        kthread_stop(dev->work_thread);
#if VDISP_IMPL_DBG
        DBG_INFO("Stopping work thread success\n");
#endif
    }
    MI_SYS_BUG_ON(dev->devhandle==NULL);
    _vdisp_deinit_inputports_allocator(dev);
    sba_bufqdestroy(&dev->bufqueue);
    mi_sys_UnRegisterDev(dev->devhandle);
#if VDISP_CUSALLOCATOR_DBG
    if(atomic_read(&dev->allocator_cnt)){
        DBG_ERR("allocator leaks: %d\n", atomic_read(&dev->allocator_cnt));
        vdisp_dbg_dump_allocators();
    }
#else
    MI_SYS_BUG_ON(atomic_read(&dev->allocator_cnt)!=0);
#endif
    dev->status=VDISP_DEVICE_UNINIT;
    ret = MI_SUCCESS;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

MI_S32 MI_VDISP_IMPL_SetOutputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_OutputPortAttr_t *pstOutputPortAttr)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    int i;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Set Outputport: dev=%d, port=%d, (cl=%08x, fmt=%d, pts=%lld, fr=%d, w=%d, h=%d)\n",
            DevId, PortId,
            pstOutputPortAttr->u32BgColor,
            pstOutputPortAttr->ePixelFormat,
            pstOutputPortAttr->u64pts,
            pstOutputPortAttr->u32FrmRate,
            pstOutputPortAttr->u32Width,
            pstOutputPortAttr->u32Height
            );
#endif
    if(!VDISP_VALID_DEVID(DevId)){
        DBG_ERR("Invalid DevId=%d\n", DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }
    if(!VDISP_VALID_OUTPUTPORTID(PortId)){
        DBG_ERR("Invalid PortId=%d\n", PortId);
        return MI_VDISP_ERR_ILLEGAL_PARAM;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(VDISP_DEVICE_UNINIT==dev->status) {
        DBG_ERR("device %d not open\n",DevId);
        ret=MI_VDISP_ERR_DEV_NOT_OPEN;
        goto exit_device;
    }

    for(i=0;i<VDISP_TOTAL_INPUTPORT_NUM;i++){
        if(dev->inputport[i].status!=VDISP_INPUTPORT_ENABLED)
            continue;
        if(!vdisp_check_inout_compatible(&dev->inputport[i].attr, pstOutputPortAttr)){
            DBG_ERR("Attr not compatible with inputport %d\n", i);
            ret=MI_VDISP_ERR_ILLEGAL_PARAM;
            goto exit_device;
        }
    }
    if(!MI_VDISP_IMPL_IsSupPixelFmt(pstOutputPortAttr->ePixelFormat)){
        DBG_ERR("Not supported pixel format\n");
        ret=MI_VDISP_ERR_NOT_SUPPORT;
        goto exit_device;
    }
    if(pstOutputPortAttr->u32Width%2){
        DBG_ERR("Output width is not 2 aligment\n");
        ret=MI_VDISP_ERR_ILLEGAL_PARAM;
        goto exit_device;
    }
    if(pstOutputPortAttr->ePixelFormat==E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420
        &&pstOutputPortAttr->u32Height%2==1){
        DBG_ERR("Output height is not 2 aligment\n");
        ret=MI_VDISP_ERR_ILLEGAL_PARAM;
        goto exit_device;
    }
    mi_sys_EnsureOutportBkRefFifoDepth(dev->devhandle, 0, 0, 1);
    dev->outputport.attr = *pstOutputPortAttr;
    dev->outputport.interval=pstOutputPortAttr->u32FrmRate?(1000000/pstOutputPortAttr->u32FrmRate):(1000000/30);
    dev->outputport.binited = 1;
    ret = MI_SUCCESS;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
MI_S32 MI_VDISP_IMPL_GetOutputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_OutputPortAttr_t *pstOutputPortAttr)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Get Outputport: dev=%d, port=%d\n",
            DevId, PortId);
#endif
    if(!VDISP_VALID_DEVID(DevId) ){
        DBG_ERR("Invalid DevId=%d\n", DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }
    if(!VDISP_VALID_OUTPUTPORTID(PortId)){
        DBG_ERR("Invalid PortId=%d\n", PortId);
        return MI_VDISP_ERR_ILLEGAL_PARAM;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(!dev->outputport.binited){
        DBG_ERR("Output port not inited\n");
        ret=MI_VDISP_ERR_NOT_CONFIG;
        goto exit_device;
    }
     *pstOutputPortAttr = dev->outputport.attr;
    ret = MI_SUCCESS;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

MI_S32 MI_VDISP_IMPL_SetInputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_InputPortAttr_t *pstInputPortAttr)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
    vdisp_inputport_t *inputport;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Set Inputport: dev=%d, port=%d, (x=%d, y=%d, w=%d, h=%d, frn=%d)\n",
            DevId, PortId,
            pstInputPortAttr->u32OutX,
            pstInputPortAttr->u32OutY,
            pstInputPortAttr->u32OutWidth,
            pstInputPortAttr->u32OutHeight,
            pstInputPortAttr->s32IsFreeRun
            );
#endif
    if(!VDISP_VALID_DEVID(DevId) ){
        DBG_ERR("Invalid DevId=%d\n", DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }
    if(!VDISP_VALID_INPUTPORTID(PortId)){
        DBG_ERR("Invalid PortId=%d\n", PortId);
        return MI_VDISP_ERR_ILLEGAL_PARAM;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    inputport=&dev->inputport[PortId];
    if(VDISP_DEVICE_UNINIT==dev->status){
        DBG_ERR("device %d not open\n",DevId);
        ret=MI_VDISP_ERR_DEV_NOT_OPEN;
        goto exit_device;
    }
    if(dev->outputport.binited){
        if(!vdisp_check_inout_compatible(pstInputPortAttr, &dev->outputport.attr)){
            DBG_ERR("Attr not compatible with outputport:[%d, %d,%d,%d],[%d,%d]\n",
                pstInputPortAttr->u32OutX, pstInputPortAttr->u32OutY,
                pstInputPortAttr->u32OutWidth, pstInputPortAttr->u32OutHeight,
                dev->outputport.attr.u32Width, dev->outputport.attr.u32Height
                );
            ret=MI_VDISP_ERR_ILLEGAL_PARAM;
            goto exit_device;
        }
    }
    if(!VDISP_IS_OVERLAYINPUTPORTID(PortId)){
        if(!vdisp_check_inputportattr(dev, PortId, pstInputPortAttr)){
            DBG_ERR("Attr illegal :[%d, %d,%d,%d]\n",
                pstInputPortAttr->u32OutX, pstInputPortAttr->u32OutY,
                pstInputPortAttr->u32OutWidth, pstInputPortAttr->u32OutHeight
                );
            ret=MI_VDISP_ERR_ILLEGAL_PARAM;
            goto exit_device;
        }
    }
    if((pstInputPortAttr->u32OutX%2) || (pstInputPortAttr->u32OutWidth%2)){
        DBG_ERR("Output x or w not 2 alignment:x=%d, w=%d\n",
            pstInputPortAttr->u32OutX, pstInputPortAttr->u32OutWidth);
        ret=MI_VDISP_ERR_ILLEGAL_PARAM;
        goto exit_device;
    }
    if((dev->outputport.attr.ePixelFormat==E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
        &&( (pstInputPortAttr->u32OutHeight%2==1)
              || (pstInputPortAttr->u32OutY%2==1))
      ){
        DBG_ERR("Output y or h not 2 alignment:y=%d, h=%d\n",
            pstInputPortAttr->u32OutY, pstInputPortAttr->u32OutHeight);
        ret=MI_VDISP_ERR_ILLEGAL_PARAM;
        goto exit_device;
    }
    inputport->attr = *pstInputPortAttr;
    //overlay inputport use normal memory allocator
    if(!VDISP_IS_OVERLAYINPUTPORTID(PortId)){
        inputport->sub_buf_id=sba_registersubbuf(&dev->bufqueue,
            inputport->attr.u32OutX, inputport->attr.u32OutY,
            inputport->attr.u32OutWidth, inputport->attr.u32OutHeight,
            inputport->sub_buf_id, inputport);
#if VDISP_IMPL_DBG
        DBG_INFO("register sub buf: %d\n", inputport->sub_buf_id);
#endif
        if(inputport->sub_buf_id<0){
            DBG_ERR("register sub buf fail\n");
            ret=MI_VDISP_ERR_FAIL;
            goto exit_device;
        }
        inputport->sync_buf_id=sba_getnextbufid(&dev->bufqueue,inputport->sub_buf_id);
    }

    if(inputport->status == VDISP_INPUTPORT_UNINIT){
        inputport->status = VDISP_INPUTPORT_INIT;
    }
    ret = MI_SUCCESS;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
MI_S32 MI_VDISP_IMPL_GetInputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_InputPortAttr_t *pstInputPortAttr)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Get Inputport: dev=%d, port=%d\n",
            DevId, PortId);
#endif
    if(!VDISP_VALID_DEVID(DevId) ){
        DBG_ERR("Invalid DevId=%d\n", DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }
    if(!VDISP_VALID_INPUTPORTID(PortId)){
        DBG_ERR("Invalid PortId=%d\n", PortId);
        return MI_VDISP_ERR_ILLEGAL_PARAM;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(VDISP_INPUTPORT_UNINIT==dev->inputport[PortId].status){
        DBG_ERR("Input port not inited\n");
        ret=MI_VDISP_ERR_NOT_CONFIG;
        goto exit_device;
    }
    *pstInputPortAttr = dev->inputport[PortId].attr;
    ret = MI_SUCCESS;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

MI_S32 MI_VDISP_IMPL_EnableInputPort(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Enable Inputport: dev=%d, port=%d\n",
            DevId, PortId);
#endif
    if(!VDISP_VALID_DEVID(DevId) ){
        DBG_ERR("Invalid DevId=%d\n", DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }
    if(!VDISP_VALID_INPUTPORTID(PortId)){
        DBG_ERR("Invalid PortId=%d\n", PortId);
        return MI_VDISP_ERR_ILLEGAL_PARAM;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(VDISP_INPUTPORT_UNINIT==dev->inputport[PortId].status){
        DBG_ERR("Input port not inited\n");
        ret=MI_VDISP_ERR_NOT_CONFIG;
        goto exit_device;
    }
    if(dev->outputport.binited){
        if(!vdisp_check_inout_compatible(&dev->inputport[PortId].attr, &dev->outputport.attr)){
            DBG_ERR("Input port attr not compatible with output port\n");
            ret=MI_VDISP_ERR_ILLEGAL_PARAM;
            goto exit_device;
        }
    }
    if(!VDISP_IS_OVERLAYINPUTPORTID(PortId)){
        if(!vdisp_check_inputportattr(dev, PortId, &dev->inputport[PortId].attr)){
            DBG_ERR("Attr illegal :[%d, %d,%d,%d]\n",
                dev->inputport[PortId].attr.u32OutX, dev->inputport[PortId].attr.u32OutY,
                dev->inputport[PortId].attr.u32OutWidth, dev->inputport[PortId].attr.u32OutHeight
                );
            ret=MI_VDISP_ERR_ILLEGAL_PARAM;
            goto exit_device;
        }
    }
#if VDISP_IMPL_DBG
    DBG_INFO("enable sub buf: %d\n", dev->inputport[PortId].sub_buf_id);
#endif
    if(dev->inputport[PortId].sub_buf_id>=0)
        sba_enable(&dev->bufqueue, dev->inputport[PortId].sub_buf_id, 1);
    mi_sys_EnableInputPort(dev->devhandle, 0, PortId);
    if(dev->inputport[PortId].status != VDISP_INPUTPORT_ENABLED){
#if VDISP_SUPPORT_OVERLAYINPUTPORT
        if(VDISP_IS_OVERLAYINPUTPORTID(PortId)){
            _vdisp_enable_overlay(dev, &dev->inputport[PortId]);
        }
#endif
        dev->inputport[PortId].status = VDISP_INPUTPORT_ENABLED;
    }
    ret = MI_SUCCESS;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
MI_S32 MI_VDISP_IMPL_DisableInputPort(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Disable Inputport: dev=%d, port=%d\n",
            DevId, PortId);
#endif
    if(!VDISP_VALID_DEVID(DevId) ){
        DBG_ERR("Invalid DevId=%d\n", DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }
    if(!VDISP_VALID_INPUTPORTID(PortId)){
        DBG_ERR("Invalid PortId=%d\n", PortId);
        return MI_VDISP_ERR_ILLEGAL_PARAM;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(VDISP_INPUTPORT_UNINIT==dev->inputport[PortId].status){
        DBG_ERR("Input port not inited\n");
        ret=MI_VDISP_ERR_NOT_CONFIG;
        goto exit_device;
    }
    mi_sys_DisableInputPort(dev->devhandle, 0, PortId);
    if(dev->inputport[PortId].sub_buf_id>=0){
        sba_enable(&dev->bufqueue, dev->inputport[PortId].sub_buf_id, 0);
    }
    if(dev->inputport[PortId].status == VDISP_INPUTPORT_ENABLED){
#if VDISP_SUPPORT_OVERLAYINPUTPORT
        if(VDISP_IS_OVERLAYINPUTPORTID(PortId)){
            _vdisp_disable_overlay(dev, &dev->inputport[PortId]);
        }
#endif
        dev->inputport[PortId].status = VDISP_INPUTPORT_DISABLED;
    }
    ret = MI_SUCCESS;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

MI_S32 MI_VDISP_IMPL_StartDev(MI_VDISP_DEV DevId)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Start device %d\n", DevId);
#endif
    if(!VDISP_VALID_DEVID(DevId) ){
        DBG_ERR("Invalid DevId=%d\n", DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }
    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(VDISP_DEVICE_UNINIT==dev->status || !dev->outputport.binited){
        DBG_ERR("Device not open or Outputport not inited\n");
        ret=MI_VDISP_ERR_NOT_CONFIG;
        goto exit_device;
    }
    if(VDISP_DEVICE_START != dev->status){
        sba_enable(&dev->bufqueue, -1, 1);
        clear_bit(VDISP_WORKER_PAUSED, &dev->wakeevent);
        wake_up_process(dev->work_thread);
        dev->status = VDISP_DEVICE_START;
    }
    mi_sys_EnableChannel(dev->devhandle,0);
    mi_sys_EnableOutputPort(dev->devhandle, 0, 0);
    ret = MI_SUCCESS;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
MI_S32 MI_VDISP_IMPL_StopDev(MI_VDISP_DEV DevId)
{
    vdisp_device_t *dev=NULL;
    int ret=-1;
#if VDISP_IMPL_DBG
    DBG_INFO("Vdisp Stop device %d\n", DevId);
#endif
    if(!VDISP_VALID_DEVID(DevId) ){
        DBG_ERR("Invalid DevId=%d\n", DevId);
        return MI_VDISP_ERR_INVALID_DEVID;
    }

    down(&module_sem);
    if(!_vdisp_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_VDISP_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_vdisp_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(VDISP_DEVICE_UNINIT==dev->status){
        DBG_ERR("Device not open\n");
        ret=MI_VDISP_ERR_DEV_NOT_OPEN;
        goto exit_device;
    }

    mi_sys_DisableOutputPort(dev->devhandle, 0, 0);
    mi_sys_DisableChannel(dev->devhandle,0);
    mutex_unlock(&dev->mtx);
    if(VDISP_DEVICE_START == dev->status){
        sba_enable(&dev->bufqueue, -1, 0);
        set_bit(VDISP_WORKER_PAUSE, &dev->wakeevent);
        smp_mb();
#if VDISP_IMPL_DBG
        DBG_INFO("Waiting work thread pause\n");
#endif
        while(!test_bit(VDISP_WORKER_PAUSED, &dev->wakeevent)){
            wake_up_process(dev->work_thread);
            msleep(1);
        }
#if VDISP_IMPL_DBG
        DBG_INFO("Waiting work thread pause success\n");
#endif
        dev->status = VDISP_DEVICE_STOP;
    }
    ret = MI_SUCCESS;
    goto exit;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
