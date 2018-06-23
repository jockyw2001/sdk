#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>

#include "mi_print.h"
#include "mi_sys_internal.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_gfx_datatype.h"
#include "mi_gfx_internal.h"


#define DIVP_CHN_NUM (32)
#define DIVP_INPUT_NUM (1)
#define DIVP_OUTPUT_NUM (1)

MI_SYS_DRV_HANDLE hDivpHandle;

extern MI_BOOL _ScalingByGe(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect);

static int work_thread_Divp(void *data)
{
    MI_SYS_BufConf_t stBufConfig;
    MI_SYS_BufInfo_t *pstBufInfo = NULL , *pstOutBufInfo = NULL;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    MI_BOOL bFind = FALSE;
    MI_S32 ret = -1;
    MI_U32 u32Flags = MI_SYS_MAP_VA;
    int u32Chn = 0 , u32Port = 0;

    MI_GFX_Surface_t stSrc;
    MI_GFX_Rect_t stSrcRect;
    MI_GFX_Surface_t stDst;
    MI_GFX_Rect_t stDstRect;
    while(1)
    {
        if(!bFind)
        {
            schedule();
            mi_sys_WaitOnInputTaskAvailable(hDivpHandle, 20);
        }
        bFind = FALSE;

        memset(&stBufConfig , 0 , sizeof(stBufConfig));
        stBufConfig.eBufType = E_MI_SYS_BUFDATA_FRAME;
        stBufConfig.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        stBufConfig.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        stBufConfig.stFrameCfg.u16Width = 720;
        stBufConfig.stFrameCfg.u16Height = 576;
        stBufConfig.u32Flags = MI_SYS_MAP_VA;

        for(u32Chn = 0 ; u32Chn < DIVP_CHN_NUM ; u32Chn ++)
        {

            pstBufInfo = mi_sys_GetInputPortBuf(hDivpHandle,u32Chn,u32Port,u32Flags);
            if(!pstBufInfo)
            {
                DBG_INFO("get input buf fail(Module: Divp Chn:%d Port:%d)\n",u32Chn,u32Port);
                continue;
            }
            MI_SYS_BUG_ON(pstBufInfo->eBufType != E_MI_SYS_BUFDATA_FRAME);
            pstOutBufInfo = mi_sys_GetOutputPortBuf(hDivpHandle,u32Chn,u32Port,&stBufConfig,&bBlockedByRateCtrl);
            if(!pstOutBufInfo)
            {
                if(bBlockedByRateCtrl)
                {
                    mi_sys_FinishBuf(pstBufInfo);
                }
                else
                {
                    mi_sys_RewindBuf(pstBufInfo);
                }
                bFind = TRUE;
                continue;
            }

            stSrcRect.s32Xpos = 0;
            stSrcRect.u32Height = (MI_U32)(pstBufInfo->stFrameData.u16Height);
            stSrcRect.s32Ypos = 0;
            stSrcRect.u32Width = (MI_U32)(pstBufInfo->stFrameData.u16Width);

            stSrc.eColorFmt = E_MI_GFX_FMT_YUV422;
            stSrc.phyAddr = pstBufInfo->stFrameData.phyAddr[0];
            stSrc.u32Height = stSrcRect.u32Height;
            stSrc.u32Width = stSrcRect.u32Width;
            stSrc.u32Stride = pstBufInfo->stFrameData.u32Stride[0];

            memset(&stDst, 0, sizeof(stDst));
            stDst.eColorFmt = E_MI_GFX_FMT_YUV422;
            stDst.phyAddr   = pstOutBufInfo->stFrameData.phyAddr[0];
            stDst.u32Stride   = pstOutBufInfo->stFrameData.u32Stride[0];
            stDst.u32Height = pstOutBufInfo->stFrameData.u16Height;
            stDst.u32Width  = pstOutBufInfo->stFrameData.u16Width;

            memset(&stDstRect, 0, sizeof(stDstRect));
            stDstRect.s32Xpos = 0;
            stDstRect.u32Width = stDst.u32Width;
            stDstRect.s32Ypos = 0;
            stDstRect.u32Height = stDst.u32Height;
            pstOutBufInfo->bEndOfStream = pstBufInfo->bEndOfStream;
            _ScalingByGe(&stSrc, &stSrcRect, &stDst, &stDstRect);

            ret = mi_sys_FinishBuf(pstBufInfo);
            if(ret != MI_SUCCESS)
            {
                printk("finish input buf fail (Module: divp Chn:%d Port:%d)\n",u32Chn,u32Port);
            }

            ret = mi_sys_FinishBuf(pstOutBufInfo);
            if(ret != MI_SUCCESS)
            {
                printk("finish output buf fail (Module: divp Chn:%d Port:%d)\n",u32Chn,u32Port);
            }

            bFind = TRUE;
        }

    }
    return 0;
}

MI_S32 _MI_SYS_DEMO_DivpBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("DivpBindInputput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("DivpBindInputput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_DivpUnBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("DivpUnBindInputput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("DivpUnBindInputput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_DivpBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("DivpBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("DivpBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_DivpUnBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("DivpUnBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("DivpnBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DivpRegistDev(MI_SYS_DRV_HANDLE *pHandle)
{
    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevOps;
    #ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    #endif
    void *pUsrData = NULL;
    memset(&stModDevInfo, 0 , sizeof(stModDevInfo));
    memset(&stModDevOps, 0 , sizeof(stModDevOps));

    stModDevInfo.eModuleId = E_MI_MODULE_ID_DIVP;
    stModDevInfo.u32DevId = 0;
    stModDevInfo.u32DevChnNum = DIVP_CHN_NUM;
    stModDevInfo.u32InputPortNum = DIVP_INPUT_NUM;
    stModDevInfo.u32OutputPortNum = DIVP_OUTPUT_NUM;

    stModDevOps.OnBindInputPort = _MI_SYS_DEMO_DivpBindInput;
    stModDevOps.OnBindOutputPort = _MI_SYS_DEMO_DivpBindOutput;
    stModDevOps.OnUnBindInputPort = _MI_SYS_DEMO_DivpUnBindInput;
    stModDevOps.OnUnBindOutputPort = _MI_SYS_DEMO_DivpUnBindOutput;
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

void divp_init(void)
{
    MI_U32 u32Chn = 0;

    MI_PRINT("%s\n", __func__);
    _MI_SYS_DivpRegistDev(&hDivpHandle);


    for(u32Chn = 0 ; u32Chn < DIVP_CHN_NUM ; u32Chn ++)
    {
        mi_sys_EnableInputPort(hDivpHandle,u32Chn,0);
        mi_sys_EnableOutputPort(hDivpHandle,u32Chn,0);
        mi_sys_EnableChannel(hDivpHandle ,u32Chn);
    }

    kthread_run(work_thread_Divp, NULL, "divp");
}


