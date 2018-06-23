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


#define VIF_CHN_NUM (16)
#define VIF_INPUT_NUM (0)
#define VIF_OUTPUT_NUM (2)

MI_SYS_DRV_HANDLE hHandle;
static DECLARE_WAIT_QUEUE_HEAD(wait_queue);

static int work_thread_vif(void *data)
{
    MI_SYS_BufConf_t stBufConfig[2];
    MI_SYS_BufInfo_t *pstBufInfo;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    MI_S32 ret = -1;
    MI_U32 u32Chn = 0 , u32Port = 0;
    int n =0;
    FILE_HANDLE fp[VIF_CHN_NUM][VIF_OUTPUT_NUM];
    while(1)
    {
        if(schedule_timeout_interruptible(20) > 0){
            //MI_PRINT("vif isr called\n");
        }

        stBufConfig[0].eBufType = E_MI_SYS_BUFDATA_FRAME;
        stBufConfig[0].u64TargetPts = 0xffff;
        stBufConfig[0].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        stBufConfig[0].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        stBufConfig[0].stFrameCfg.u16Width = 1920;
        stBufConfig[0].stFrameCfg.u16Height = 1080;
        stBufConfig[0].u32Flags = MI_SYS_MAP_VA;

        stBufConfig[1].eBufType = E_MI_SYS_BUFDATA_FRAME;
        stBufConfig[1].u64TargetPts = 0xffff;
        stBufConfig[1].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        stBufConfig[1].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        stBufConfig[1].stFrameCfg.u16Width = 960;
        stBufConfig[1].stFrameCfg.u16Height = 540;
        stBufConfig[1].u32Flags = MI_SYS_MAP_VA;

        for(u32Chn = 0  ; u32Chn < VIF_CHN_NUM ; u32Chn ++)
        {
            for(u32Port = 0 ; u32Port < VIF_OUTPUT_NUM ; u32Port ++)
            {
                pstBufInfo = mi_sys_GetOutputPortBuf(hHandle,u32Chn,u32Port,&stBufConfig[u32Port],&bBlockedByRateCtrl);
                if(!pstBufInfo)
                {
                    DBG_INFO("get out put buf fail(Module: Vif Chn:%d Port:%d)\n",u32Chn,u32Port);
                    continue;
                }
                if(!fp[u32Chn][u32Port])
                {
                    if(u32Port == 0)
                        fp[u32Chn][u32Port] = open_yuv_file("/mnt/test/1920_1080_yuv422.yuv",O_RDONLY);
                    else
                        fp[u32Chn][u32Port] = open_yuv_file("/mnt/test/960_540_yuv422.yuv",O_RDONLY);
                }
                n = read_yuv_file(fp[u32Chn][u32Port], pstBufInfo->stFrameData);

                if(n < 0)
                {
                    close_yuv_file(fp[u32Chn][u32Port]);
                    fp[u32Chn][u32Port] = NULL;
                    mi_sys_RewindBuf(pstBufInfo);
                    continue;
                }

                pstBufInfo->bEndOfStream = 0;
                ret = mi_sys_FinishBuf(pstBufInfo);
                if(ret != MI_SUCCESS)
                {
                    printk("finish buf fail (Module: Vif Chn:%d Port:%d)\n",u32Chn,u32Port);
                }
            }
        }
    }
    return 0;
}

MI_S32 _MI_SYS_DEMO_VifBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VifBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VifBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return 0;
}

MI_S32 _MI_SYS_DEMO_VifUnBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VifUnBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VifUnBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return 0;

}

MI_S32 _MI_SYS_VifRegistDev(MI_SYS_DRV_HANDLE *pHandle)
{
    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevOps;
    void *pUsrData = NULL;
    #ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    #endif
    memset(&stModDevInfo, 0 , sizeof(stModDevInfo));
    memset(&stModDevOps, 0 , sizeof(stModDevOps));

    stModDevInfo.eModuleId = E_MI_MODULE_ID_VIF;
    stModDevInfo.u32DevId = 0;
    stModDevInfo.u32DevChnNum = VIF_CHN_NUM;
    stModDevInfo.u32InputPortNum = VIF_INPUT_NUM;
    stModDevInfo.u32OutputPortNum = VIF_OUTPUT_NUM;

    stModDevOps.OnBindInputPort = NULL;
    stModDevOps.OnBindOutputPort = _MI_SYS_DEMO_VifBindOutput;
    stModDevOps.OnUnBindInputPort = NULL;
    stModDevOps.OnUnBindOutputPort = _MI_SYS_DEMO_VifUnBindOutput;
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

void vif_init(void)
{
    MI_U32 u32Chn = 0;

    MI_PRINT("%s\n", __func__);
    _MI_SYS_VifRegistDev(&hHandle);

    for(u32Chn = 0 ; u32Chn < VIF_CHN_NUM ; u32Chn ++)
    {
        mi_sys_EnableOutputPort(hHandle,u32Chn,0);
        mi_sys_EnableOutputPort(hHandle,u32Chn,1);
        mi_sys_EnableChannel(hHandle ,u32Chn);
    }

    kthread_run(work_thread_vif, NULL, "vif");
}


