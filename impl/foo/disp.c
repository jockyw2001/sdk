#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include "mi_print.h"
#include "mi_sys_internal.h"
#include "mi_sys_proc_fs_internal.h"


MI_SYS_DRV_HANDLE hDispHandle;

#define DISP_CHN_NUM (16)
#define DISP_INPUT_PORT_NUM (16)
#define DISP_OUTPUT_PORT_NUM (0)

static int work_thread_disp(void *data)
{
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    MI_BOOL bFind = FALSE;
    while(1)
    {
        int i = 0;
        if(!bFind)
        {
            schedule();
            mi_sys_WaitOnInputTaskAvailable(hDispHandle, 10);
        }
        bFind = FALSE;
        for(i = 0 ; i <  DISP_INPUT_PORT_NUM ; i ++)
        {
            pstBufInfo = mi_sys_GetInputPortBuf(hDispHandle,0, i ,0);
            if(!pstBufInfo)
            {
                DBG_INFO("get out put buf fail(Module: DISP Chn:%d Port:%d)\n",0,i);
                continue;
            }

            mi_sys_FinishBuf(pstBufInfo);
            bFind = TRUE;
        }
    }
    return 0;
}


MI_S32 _MI_SYS_DEMO_DispBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("DispBindInput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("DispBindInput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return 0;
}

MI_S32 _MI_SYS_DEMO_DispUnBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("DispUnBindInput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("DispUnBindInput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return 0;
}


MI_S32 _MI_SYS_DispRegistDev(MI_SYS_DRV_HANDLE *pHandle)
{

    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevOps;
    #ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    #endif
    void *pUsrData = NULL;
    memset(&stModDevInfo, 0 , sizeof(stModDevInfo));
    memset(&stModDevOps, 0 , sizeof(stModDevOps));

    stModDevInfo.eModuleId = E_MI_MODULE_ID_DISP;
    stModDevInfo.u32DevId = 0;
    stModDevInfo.u32DevChnNum = DISP_CHN_NUM;
    stModDevInfo.u32InputPortNum = DISP_INPUT_PORT_NUM;
    stModDevInfo.u32OutputPortNum = DISP_OUTPUT_PORT_NUM;

    stModDevOps.OnBindInputPort = _MI_SYS_DEMO_DispBindInput;
    stModDevOps.OnUnBindInputPort = _MI_SYS_DEMO_DispUnBindInput;
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



void disp_init(void)
{
    MI_U32 u32Chn = 0 , u32Port = 0;

    MI_PRINT("%s\n", __func__);

    _MI_SYS_DispRegistDev(&hDispHandle);


    for(u32Chn = 0 ; u32Chn < DISP_CHN_NUM ; u32Chn ++)
    {
        for(u32Port = 0 ; u32Port < DISP_INPUT_PORT_NUM ; u32Port ++)
        {
            mi_sys_EnableInputPort(hDispHandle,u32Chn,u32Port);
        }
        mi_sys_EnableChannel(hDispHandle ,u32Chn);
    }


    kthread_run(work_thread_disp,  NULL, "disp_work0");

}

