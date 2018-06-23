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


#define Vdec_CHN_NUM (16)
#define Vdec_INPUT_NUM (1)
#define Vdec_OUTPUT_NUM (1)

MI_SYS_DRV_HANDLE hVdecHandle;

static int work_thread_Vdec(void *data)
{
    MI_SYS_BufConf_t stBufConfig;
    MI_SYS_BufInfo_t *pstBufInfo = NULL , *pstOutBufInfo = NULL;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    MI_BOOL bFind = FALSE;
    MI_S32 ret = -1;
    int n = 0;
    MI_U32 u32Flags = MI_SYS_MAP_VA;
    int u32Chn = 0 , u32Port = 0;
    FILE_HANDLE fp[Vdec_CHN_NUM] = {NULL};
    while(1)
    {
        if(!bFind)
        {
            schedule();
            mi_sys_WaitOnInputTaskAvailable(hVdecHandle, 20);
        }
        bFind = FALSE;

        memset(&stBufConfig , 0 , sizeof(stBufConfig));
        stBufConfig.eBufType = E_MI_SYS_BUFDATA_FRAME;
        stBufConfig.u64TargetPts = 0xf5678;
        stBufConfig.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        stBufConfig.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        stBufConfig.stFrameCfg.u16Width = 1920;
        stBufConfig.stFrameCfg.u16Height = 1080;

        stBufConfig.u32Flags = MI_SYS_MAP_VA;

        for(u32Chn = 0  ; u32Chn < Vdec_CHN_NUM ; u32Chn ++)
        {
            pstBufInfo = mi_sys_GetInputPortBuf(hVdecHandle,u32Chn,u32Port,u32Flags);
            if(!pstBufInfo)
            {
                DBG_INFO("get input buf fail(Module: Vdec Chn:%d Port:%d)\n",u32Chn,u32Port);
                continue;
            }

            pstOutBufInfo = mi_sys_GetOutputPortBuf(hVdecHandle,u32Chn,u32Port,&stBufConfig,&bBlockedByRateCtrl);
            if(!pstOutBufInfo)
            {
                if(bBlockedByRateCtrl)
                    mi_sys_FinishBuf(pstBufInfo);
                else
                    mi_sys_RewindBuf(pstBufInfo);
                bFind = TRUE;
                continue;
            }

            if(!fp[u32Chn])
            {
                fp[u32Chn] = open_yuv_file("/mnt/test/1920_1080_yuv422.yuv",O_RDONLY);
            }

            n = read_yuv_file(fp[u32Chn], pstOutBufInfo->stFrameData);
            if(n < 0)
            {
                close_yuv_file(fp[u32Chn]);
                fp[u32Chn] = NULL;
                mi_sys_RewindBuf(pstBufInfo);
                mi_sys_RewindBuf(pstOutBufInfo);
                bFind = TRUE;
                continue;
            }
            pstOutBufInfo->bEndOfStream = pstBufInfo->bEndOfStream;
            ret = mi_sys_FinishBuf(pstBufInfo);
            if(ret != MI_SUCCESS)
            {
                printk("finish input buf fail (Module: Vdec Chn:%d Port:%d)\n",u32Chn,u32Port);
            }

            ret = mi_sys_FinishBuf(pstOutBufInfo);
            if(ret != MI_SUCCESS)
            {
                printk("finish output buf fail (Module: Vdec Chn:%d Port:%d)\n",u32Chn,u32Port);
            }

            bFind = TRUE;
        }

    }
    return 0;
}

MI_S32 _MI_SYS_DEMO_VdecBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VdecBindInputput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VdecBindInputput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_VdecUnBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VdecUnBindInputput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VdecUnBindInputput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_VdecBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VdecBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VdecBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_VdecUnBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VdecUnBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VdecnBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_VdecRegistDev(MI_SYS_DRV_HANDLE *pHandle)
{
    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevOps;
    #ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    #endif
    void *pUsrData = NULL;
    memset(&stModDevInfo, 0 , sizeof(stModDevInfo));
    memset(&stModDevOps, 0 , sizeof(stModDevOps));

    stModDevInfo.eModuleId = E_MI_MODULE_ID_VDEC;
    stModDevInfo.u32DevId = 0;
    stModDevInfo.u32DevChnNum = Vdec_CHN_NUM;
    stModDevInfo.u32InputPortNum = Vdec_INPUT_NUM;
    stModDevInfo.u32OutputPortNum = Vdec_OUTPUT_NUM;

    stModDevOps.OnBindInputPort = _MI_SYS_DEMO_VdecBindInput;
    stModDevOps.OnBindOutputPort = _MI_SYS_DEMO_VdecBindOutput;
    stModDevOps.OnUnBindInputPort = _MI_SYS_DEMO_VdecUnBindInput;
    stModDevOps.OnUnBindOutputPort = _MI_SYS_DEMO_VdecUnBindOutput;
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

void vdec_init(void)
{
    MI_U32 u32Chn = 0;

    MI_PRINT("%s\n", __func__);
    _MI_SYS_VdecRegistDev(&hVdecHandle);

    for(u32Chn = 0 ; u32Chn < Vdec_CHN_NUM ; u32Chn ++)
    {
        mi_sys_EnableInputPort(hVdecHandle,u32Chn,0);
        mi_sys_EnableOutputPort(hVdecHandle,u32Chn,0);
        mi_sys_EnableChannel(hVdecHandle ,u32Chn);
    }

    kthread_run(work_thread_Vdec, NULL, "Vdec");
}


