#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include "cmdq.h"
#include "mi_common.h"
#include "mi_print.h"
#include "mi_sys_internal.h"
#include "mi_gfx_datatype.h"
#include "mi_gfx.h"
#include "mi_sys_proc_fs_internal.h"

#define MI_VPE_FRAME_PER_BURST_CMDQ (1)
#define VPE_INPUT_PORT_NUM  (1)
#define VPE_OUTPUT_PORT_NUM (4)
#define VPE_CHN_NUM         (64)

//#define MI_BY_GFX

static DECLARE_WAIT_QUEUE_HEAD(wait_queue);
static LIST_HEAD(todo_task);
MI_SYS_DRV_HANDLE hVpeHandle;

static atomic_t gcount;;


MI_BOOL _ScalingByGe(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect)
{
#ifdef MI_BY_GFX
    MI_U16 u16Fence = 0xFF;
    MI_GFX_Opt_t stOpt;
    MI_GFX_Opt_t *pstBlitOpt = &stOpt;
    memset(pstBlitOpt, 0, sizeof(*pstBlitOpt));
    MI_GFX_Open();
    DBG_INFO("Start bit blit.\n");

    DBG_INFO("Src : Rect = {%d, %d, %d, %d}.\n", pstSrcRect->s32Xpos, pstSrcRect->s32Ypos, pstSrcRect->u32Width, pstSrcRect->u32Height);
    DBG_INFO("Src surface = {.phyAddr: %llx, .eColorFmt: %d, .u32Width: %u, .u32Height: %u, .u32Stride: %u}.\n",
        pstSrc->phyAddr, pstSrc->eColorFmt, pstSrc->u32Width, pstSrc->u32Height, pstSrc->u32Stride);

    DBG_INFO("Dest: Rect = {%d, %d, %d, %d}.\n", pstDstRect->s32Xpos, pstDstRect->s32Ypos, pstDstRect->u32Width, pstDstRect->u32Height);
    DBG_INFO("Dest surface = {.phyAddr: %llx, .eColorFmt: %d, .u32Width: %u, .u32Height: %u, .u32Stride: %u}.\n",
        pstDst->phyAddr, pstDst->eColorFmt, pstDst->u32Width, pstDst->u32Height, pstDst->u32Stride);

    pstBlitOpt->bEnGfxRop = FALSE;
    pstBlitOpt->eRopCode = E_MI_GFX_ROP_NONE;
    pstBlitOpt->eSrcDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    pstBlitOpt->eDstDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    pstBlitOpt->eMirror = E_MI_GFX_MIRROR_NONE;
    pstBlitOpt->eRotate = E_MI_GFX_ROTATE_0;
    pstBlitOpt->eSrcYuvFmt = E_MI_GFX_YUV_YVYU;
    pstBlitOpt->eDstYuvFmt = E_MI_GFX_YUV_YVYU;
    pstBlitOpt->stClipRect.s32Xpos = 0;
    pstBlitOpt->stClipRect.s32Ypos = 0;
    pstBlitOpt->stClipRect.u32Width  = 0;
    pstBlitOpt->stClipRect.u32Height = 0;

    MI_GFX_BitBlit(pstSrc, pstSrcRect, pstDst, pstDstRect, pstBlitOpt, &u16Fence);
    DBG_INFO("Bit blit done.\n");

    DBG_INFO("Start wait fence: 0x%x.\n", u16Fence);
    //MI_GFX_IMPL_WaitAllDone(FALSE, u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    DBG_INFO("Wait done.\n");

    MI_GFX_Close();
#else
    MI_U32 u32SizeSrc = pstSrc->u32Stride * pstSrc->u32Height;
    MI_U32 u32SizeDst = pstDst->u32Stride * pstDst->u32Height;
    MI_U32 u32SizeMin = (u32SizeSrc<u32SizeDst)?u32SizeSrc:u32SizeDst;
    void *pVsrc = mi_sys_Vmap(pstSrc->phyAddr,u32SizeSrc,FALSE);
    void *pVdst = mi_sys_Vmap(pstDst->phyAddr,u32SizeDst,FALSE);
    memset(pVdst , 0 , u32SizeDst);
    memcpy(pVdst , pVsrc , u32SizeMin);
    mi_sys_UnVmap(pVsrc);
    mi_sys_UnVmap(pVdst);
#endif
    return TRUE;
}


static mi_sys_TaskIteratorCBAction_e _MI_VPE_TaskIteratorCallBK(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData)
{
    //for 0
    pstTaskInfo->astOutputPortPerfBufConfig[0].eBufType = E_MI_SYS_BUFDATA_FRAME;
    pstTaskInfo->astOutputPortPerfBufConfig[0].u64TargetPts = 0x1ffff;
    pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.u16Width = 1920;
    pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.u16Height = 1080;


    //for 1
    pstTaskInfo->astOutputPortPerfBufConfig[1].eBufType = E_MI_SYS_BUFDATA_FRAME;
    pstTaskInfo->astOutputPortPerfBufConfig[1].u64TargetPts = 0x1ffff;
    pstTaskInfo->astOutputPortPerfBufConfig[1].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    pstTaskInfo->astOutputPortPerfBufConfig[1].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    pstTaskInfo->astOutputPortPerfBufConfig[1].stFrameCfg.u16Width = 1280;
    pstTaskInfo->astOutputPortPerfBufConfig[1].stFrameCfg.u16Height = 720;


    //for 2
    pstTaskInfo->astOutputPortPerfBufConfig[2].eBufType = E_MI_SYS_BUFDATA_FRAME;
    pstTaskInfo->astOutputPortPerfBufConfig[2].u64TargetPts = 0x1ffff;
    pstTaskInfo->astOutputPortPerfBufConfig[2].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    pstTaskInfo->astOutputPortPerfBufConfig[2].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    pstTaskInfo->astOutputPortPerfBufConfig[2].stFrameCfg.u16Width = 352;
    pstTaskInfo->astOutputPortPerfBufConfig[2].stFrameCfg.u16Height = 288;

    //for 3
    pstTaskInfo->astOutputPortPerfBufConfig[3].eBufType = E_MI_SYS_BUFDATA_FRAME;
    pstTaskInfo->astOutputPortPerfBufConfig[3].u64TargetPts = 0x1ffff;
    pstTaskInfo->astOutputPortPerfBufConfig[3].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    pstTaskInfo->astOutputPortPerfBufConfig[3].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    pstTaskInfo->astOutputPortPerfBufConfig[3].stFrameCfg.u16Width = 960;
    pstTaskInfo->astOutputPortPerfBufConfig[3].stFrameCfg.u16Height = 540;

    mi_sys_PrepareTaskOutputBuf(pstTaskInfo);

    list_add_tail(&pstTaskInfo->listChnTask, &todo_task);
    if(atomic_inc_return(&gcount) >= MI_VPE_FRAME_PER_BURST_CMDQ)
    {
        return MI_SYS_ITERATOR_ACCEPT_STOP;
    }
    return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
}



static int work_thread_vpe(void *data)
{
    MI_GFX_Surface_t stSrc;
    MI_GFX_Rect_t stSrcRect;
    MI_GFX_Surface_t stDst;
    MI_GFX_Rect_t stDstRect;
    int i ;
    struct list_head *pos, *n;
    mi_sys_ChnTaskInfo_t *pstChnTask;
    MI_U32 u32Size = 0;

    while(1)
    {

        mi_sys_DevTaskIterator(hVpeHandle ,_MI_VPE_TaskIteratorCallBK , NULL);
        if(list_empty(&todo_task) || atomic_read(&gcount) < MI_VPE_FRAME_PER_BURST_CMDQ)
        {
            schedule();
            mi_sys_WaitOnInputTaskAvailable(hVpeHandle, 10);
            continue;
        }
        atomic_set(&gcount , 0);

        list_for_each_safe(pos,n,&todo_task)
        {
            pstChnTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
            list_del(&pstChnTask->listChnTask);
            INIT_LIST_HEAD(&pstChnTask->listChnTask);


            u32Size = (MI_U32)(pstChnTask->astInputPortBufInfo[0]->stFrameData.u16Height) * pstChnTask->astInputPortBufInfo[0]->stFrameData.u32Stride[0];

            if(pstChnTask->astInputPortBufInfo[0])
            {
                stSrcRect.s32Xpos = 0;
                stSrcRect.u32Height = (MI_U32)(pstChnTask->astInputPortBufInfo[0]->stFrameData.u16Height);
                stSrcRect.s32Ypos = 0;
                stSrcRect.u32Width = (MI_U32)(pstChnTask->astInputPortBufInfo[0]->stFrameData.u16Width);

                stSrc.eColorFmt = E_MI_GFX_FMT_YUV422;
                stSrc.phyAddr = pstChnTask->astInputPortBufInfo[0]->stFrameData.phyAddr[0];
                stSrc.u32Height = stSrcRect.u32Height;
                stSrc.u32Width = stSrcRect.u32Width;
                stSrc.u32Stride = pstChnTask->astInputPortBufInfo[0]->stFrameData.u32Stride[0];

                for(i = 0 ; i < VPE_OUTPUT_PORT_NUM ; i ++)
                {
                    if(pstChnTask->astOutputPortBufInfo[i])
                    {
                        memset(&stDst, 0, sizeof(stDst));
                        stDst.eColorFmt = E_MI_GFX_FMT_YUV422;
                        stDst.phyAddr   = pstChnTask->astOutputPortBufInfo[i]->stFrameData.phyAddr[0];
                        stDst.u32Stride   = pstChnTask->astOutputPortBufInfo[i]->stFrameData.u32Stride[0];
                        stDst.u32Height = pstChnTask->astOutputPortBufInfo[i]->stFrameData.u16Height;
                        stDst.u32Width  = pstChnTask->astOutputPortBufInfo[i]->stFrameData.u16Width;

                        memset(&stDstRect, 0, sizeof(stDstRect));
                        stDstRect.s32Xpos = 0;
                        stDstRect.u32Width = stDst.u32Width;
                        stDstRect.s32Ypos = 0;
                        stDstRect.u32Height = stDst.u32Height;
                        pstChnTask->astOutputPortBufInfo[i]->bEndOfStream = pstChnTask->astInputPortBufInfo[0]->bEndOfStream;
                        _ScalingByGe(&stSrc, &stSrcRect, &stDst, &stDstRect);
                    }
                }
            }
            else
                MI_SYS_BUG();
            mi_sys_FinishAndReleaseTask(pstChnTask);
        }
    }
    return 0;
}

MI_S32 _MI_SYS_DEMO_VpeBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VpeBindInput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VpeBindInput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_VpeBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VpeBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VpeBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;

}

MI_S32 _MI_SYS_DEMO_VpeUnBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VpeUnBindInput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VpeUnBindInput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;

}

MI_S32 _MI_SYS_DEMO_VpeUnBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VpeUnBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VpeUnBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;

}

MI_S32 _MI_SYS_VpeRegistDev(MI_SYS_DRV_HANDLE *pHandle)
{

    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevOps;
    #ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    #endif
    void *pUsrData = NULL;
    memset(&stModDevInfo, 0 , sizeof(stModDevInfo));
    memset(&stModDevOps, 0 , sizeof(stModDevOps));

    stModDevInfo.eModuleId = E_MI_MODULE_ID_VPE;
    stModDevInfo.u32DevId = 0;
    stModDevInfo.u32DevChnNum = VPE_CHN_NUM;
    stModDevInfo.u32InputPortNum = VPE_INPUT_PORT_NUM;
    stModDevInfo.u32OutputPortNum = VPE_OUTPUT_PORT_NUM;

    stModDevOps.OnBindInputPort = _MI_SYS_DEMO_VpeBindInput;
    stModDevOps.OnBindOutputPort = _MI_SYS_DEMO_VpeBindOutput;
    stModDevOps.OnUnBindInputPort = _MI_SYS_DEMO_VpeUnBindInput;
    stModDevOps.OnUnBindOutputPort = _MI_SYS_DEMO_VpeUnBindOutput;
    stModDevOps.OnOutputPortBufRelease = NULL;

#ifdef MI_SYS_PROC_FS_DEBUG
    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif
    *pHandle = mi_sys_RegisterDev(&stModDevInfo,&stModDevOps,pUsrData
                                    #ifdef MI_SYS_PROC_FS_DEBUG
                                    , &pstModuleProcfsOps
                                    ,MI_COMMON_GetSelfDir
                                    #endif
                                 );

    return MI_SUCCESS;
}


void vpe_init(void)
{
    MI_U32 u32Chn = 0;

    MI_PRINT("%s\n", __func__);

    atomic_set(&gcount,0);

    _MI_SYS_VpeRegistDev(&hVpeHandle);


    for(u32Chn = 0 ; u32Chn < VPE_CHN_NUM ; u32Chn ++)
    {
        mi_sys_EnableInputPort(hVpeHandle,u32Chn,0);
        mi_sys_EnableOutputPort(hVpeHandle,u32Chn,0);
        mi_sys_EnableOutputPort(hVpeHandle,u32Chn,1);
        mi_sys_EnableOutputPort(hVpeHandle,u32Chn,2);
        mi_sys_EnableOutputPort(hVpeHandle,u32Chn,3);
        mi_sys_EnableChannel(hVpeHandle ,u32Chn);
    }

    kthread_run(work_thread_vpe, NULL, "vpe_work");
}



