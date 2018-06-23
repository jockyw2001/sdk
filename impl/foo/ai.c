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

#define Ai_CHN_NUM (16)
#define Ai_INPUT_NUM (0)
#define Ai_OUTPUT_NUM (1)

MI_U8 u8Buffer[1024] = {0};

MI_SYS_DRV_HANDLE hAiHandle;

static DECLARE_WAIT_QUEUE_HEAD(wait_queue);

static int work_thread_Ai(void *data)
{
    MI_SYS_BufConf_t stBufConfig;
    MI_SYS_BufInfo_t *pstBufInfo;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    MI_S32 ret = -1;
    int u32Chn = 0 , u32Port = 0;

    while(1)
    {
        if(schedule_timeout_interruptible(20) > 0){
            //MI_PRINT("Ai isr called\n");
        }

        memset(&stBufConfig , 0 , sizeof(stBufConfig));
        stBufConfig.eBufType = E_MI_SYS_BUFDATA_RAW;
        stBufConfig.u64TargetPts = 0xf88889;
        stBufConfig.stRawCfg.u32Size = 1024;
        stBufConfig.u32Flags = MI_SYS_MAP_VA;


        for(u32Chn = 0 ; u32Chn < Ai_CHN_NUM ; u32Chn ++)
        {

            pstBufInfo = mi_sys_GetOutputPortBuf(hAiHandle,u32Chn,u32Port,&stBufConfig,&bBlockedByRateCtrl);
            if(!pstBufInfo)
            {
                DBG_INFO("get out put buf fail(Module: Ai Chn:%d Port:%d)\n",u32Chn,u32Port);
                continue;
            }
            MI_SYS_BUG_ON(pstBufInfo->eBufType  != stBufConfig.eBufType);

            sprintf(u8Buffer , "Ai Demo Test Raw data(ChnId :%d  PortId : %d)",
                u32Chn , u32Port);

            pstBufInfo->stRawData.u32ContentSize = strlen(u8Buffer);
            memcpy(pstBufInfo->stRawData.pVirAddr, u8Buffer , pstBufInfo->stRawData.u32ContentSize);
            pstBufInfo->bEndOfStream = 0;
            ret = mi_sys_FinishBuf(pstBufInfo);
            if(ret != MI_SUCCESS)
            {
                printk("finish buf fail (Module: Ai Chn:%d Port:%d)\n",u32Chn,u32Port);
            }
        }

    }
    return 0;
}

MI_S32 _MI_SYS_DEMO_AiBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("AiBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("AiBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_AiUnBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("AiUnBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("AiUnBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_AiRegistDev(MI_SYS_DRV_HANDLE *pHandle)
{
    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevOps;
    #ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    #endif
    void *pUsrData = NULL;
    memset(&stModDevInfo, 0 , sizeof(stModDevInfo));
    memset(&stModDevOps, 0 , sizeof(stModDevOps));

    stModDevInfo.eModuleId = E_MI_MODULE_ID_AI;
    stModDevInfo.u32DevId = 0;
    stModDevInfo.u32DevChnNum = Ai_CHN_NUM;
    stModDevInfo.u32InputPortNum = Ai_INPUT_NUM;
    stModDevInfo.u32OutputPortNum = Ai_OUTPUT_NUM;

    stModDevOps.OnBindInputPort = NULL;
    stModDevOps.OnBindOutputPort = _MI_SYS_DEMO_AiBindOutput;
    stModDevOps.OnUnBindInputPort = NULL;
    stModDevOps.OnUnBindOutputPort = _MI_SYS_DEMO_AiUnBindOutput;
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

void ai_init(void)
{
    MI_U32 u32Chn = 0;

    MI_PRINT("%s\n", __func__);
    _MI_SYS_AiRegistDev(&hAiHandle);


    for(u32Chn = 0 ; u32Chn < Ai_CHN_NUM ; u32Chn ++)
    {
        mi_sys_EnableOutputPort(hAiHandle,u32Chn,0);
        mi_sys_EnableChannel(hAiHandle ,u32Chn);
    }


    kthread_run(work_thread_Ai, NULL, "Ai");
}


