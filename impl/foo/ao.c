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

MI_SYS_DRV_HANDLE hAoHandle;

#define Ao_CHN_NUM (1)
#define Ao_INPUT_PORT_NUM (1)
#define Ao_OUTPUT_PORT_NUM (0)


static int work_thread_Ao(void *data)
{
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    MI_BOOL bFind = FALSE;
    while(1)
    {
        int i = 0;
        if(!bFind)
        {
            schedule();
            mi_sys_WaitOnInputTaskAvailable(hAoHandle, 20);
        }
        bFind = FALSE;

        pstBufInfo = mi_sys_GetInputPortBuf(hAoHandle,0, i ,0);
        if(!pstBufInfo)
        {
            DBG_INFO("get out put buf fail(Module: Ao Chn:%d Port:%d)\n",0,i);
            continue;
        }

        mi_sys_FinishBuf(pstBufInfo);
        bFind = TRUE;

    }
    return 0;
}


MI_S32 _MI_SYS_DEMO_AoBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("AoBindInput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("AoBindInput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_AoUnBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("AoUnBindInput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("AoUnBindInput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}


MI_S32 _MI_SYS_AoRegistDev(MI_SYS_DRV_HANDLE *pHandle)
{

    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevOps;
    #ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    #endif
    void *pUsrData = NULL;
    memset(&stModDevInfo, 0 , sizeof(stModDevInfo));
    memset(&stModDevOps, 0 , sizeof(stModDevOps));

    stModDevInfo.eModuleId = E_MI_MODULE_ID_AO;
    stModDevInfo.u32DevId = 0;
    stModDevInfo.u32DevChnNum = Ao_CHN_NUM;
    stModDevInfo.u32InputPortNum = Ao_INPUT_PORT_NUM;
    stModDevInfo.u32OutputPortNum = Ao_OUTPUT_PORT_NUM;

    stModDevOps.OnBindInputPort = _MI_SYS_DEMO_AoBindInput;
    stModDevOps.OnUnBindInputPort = _MI_SYS_DEMO_AoUnBindInput;
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



void ao_init(void)
{
    MI_PRINT("%s\n", __func__);

    _MI_SYS_AoRegistDev(&hAoHandle);

    mi_sys_EnableInputPort(hAoHandle,0,0);
    mi_sys_EnableChannel(hAoHandle,0);

    kthread_run(work_thread_Ao,  NULL, "Ao_work0");

}

