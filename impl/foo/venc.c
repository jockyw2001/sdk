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

#define MI_U32VALUE(pu8Data, index) (pu8Data[index]<<24)|(pu8Data[index+1]<<16)|(pu8Data[index+2]<<8)|(pu8Data[index+3])
#define Venc_CHN_NUM (64)
#define Venc_INPUT_NUM (1)
#define Venc_OUTPUT_NUM (1)
#define VESFILE_READER_BATCH (1024*128)

MI_SYS_DRV_HANDLE hVencHandle;

static int work_thread_Venc(void *data)
{
    MI_SYS_BufConf_t stBufConfig;
    MI_SYS_BufInfo_t *pstBufInfo = NULL , *pstOutBufInfo = NULL;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    MI_BOOL bFind = FALSE;
    MI_S32 ret = -1;
    mm_segment_t fs;
    MI_U8 au8Header[16] = {0};
    int n = 0;
    MI_U32 u32Pos = 0;
    MI_U32 u32Len = 0;
    MI_U32 u32FrameLen = 0;

    int u32Chn = 0 , u32Port = 0;
    struct file *fp = NULL;
    while(1)
    {
        if(!bFind)
        {
            schedule();
            mi_sys_WaitOnInputTaskAvailable(hVencHandle, 20);
        }
        bFind = FALSE;

        memset(&stBufConfig , 0 , sizeof(stBufConfig));
        stBufConfig.eBufType = E_MI_SYS_BUFDATA_RAW;
        stBufConfig.u64TargetPts = 0xf2345;
        stBufConfig.u32Flags = MI_SYS_MAP_VA;

        for(u32Chn = 0 ; u32Chn < Venc_CHN_NUM ; u32Chn ++)
        {

            pstBufInfo = mi_sys_GetInputPortBuf(hVencHandle,u32Chn,u32Port,0);
            if(!pstBufInfo)
            {
                DBG_INFO("get input buf fail(Module: Venc Chn:%d Port:%d)\n",u32Chn,u32Port);
                continue;
            }

            if(!fp)
            {
                fp = filp_open("/mnt/test/rawdata.avi",O_RDONLY,0644);
                if(IS_ERR(fp))
                {
                    printk("open file fail   PTR_ERR_fp=%ld\n",PTR_ERR(fp));
                    mi_sys_RewindBuf(pstBufInfo);
                    continue;
                }

            }

            fs = get_fs();
            set_fs(KERNEL_DS);
            memset(au8Header, 0, 16);
            u32Pos = fp->f_op->llseek(fp, 0L, SEEK_CUR);
            u32Len = fp->f_op->read(fp, au8Header, 16 ,&(fp->f_pos));
            if(u32Len <= 0)
            {
                fp->f_op->llseek(fp, 0L, SEEK_SET);
                mi_sys_RewindBuf(pstBufInfo);
                set_fs(fs);
                filp_close(fp, NULL);
                fp = NULL;
                bFind = TRUE;
                continue;
            }

            u32FrameLen = MI_U32VALUE(au8Header, 4);
            if(u32FrameLen > VESFILE_READER_BATCH)
            {
                fp->f_op->llseek(fp, 0L, SEEK_SET);
                mi_sys_RewindBuf(pstBufInfo);
                mi_sys_RewindBuf(pstOutBufInfo);
                continue;
            }
            stBufConfig.stRawCfg.u32Size = u32FrameLen;

            pstOutBufInfo = mi_sys_GetOutputPortBuf(hVencHandle,u32Chn,u32Port,&stBufConfig,&bBlockedByRateCtrl);
            if(!pstOutBufInfo)
            {
                if(bBlockedByRateCtrl)
                    mi_sys_FinishBuf(pstBufInfo);
                else
                    mi_sys_RewindBuf(pstBufInfo);
                bFind = TRUE;
                continue;
            }

            n =  fp->f_op->read(fp, (char *)pstOutBufInfo->stRawData.pVirAddr , pstOutBufInfo->stRawData.u32BufSize, &(fp->f_pos));
            if(n <= 0)
            {
                mi_sys_RewindBuf(pstBufInfo);
                mi_sys_RewindBuf(pstOutBufInfo);
                set_fs(fs);
                filp_close(fp, NULL);
                fp = NULL;
                bFind = TRUE;
                continue;
            }

            if(n < u32FrameLen)
            {
                filp_close(fp, NULL);
                fp = NULL;
                pstBufInfo->bEndOfStream = TRUE;
            }

            set_fs(fs);
            pstOutBufInfo->stRawData.u32ContentSize = n;
            pstOutBufInfo->bEndOfStream = pstBufInfo->bEndOfStream;
            ret = mi_sys_FinishBuf(pstBufInfo);
            if(ret != MI_SUCCESS)
            {
                printk("finish input buf fail (Module: Venc Chn:%d Port:%d)\n",u32Chn,u32Port);
            }

            ret = mi_sys_FinishBuf(pstOutBufInfo);
            if(ret != MI_SUCCESS)
            {
                printk("finish output buf fail (Module: Venc Chn:%d Port:%d)\n",u32Chn,u32Port);
            }
            bFind = TRUE;
        }

    }
    return 0;
}

MI_S32 _MI_SYS_DEMO_VencBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VencBindInputput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VencBindInputput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_VencUnBindInput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VencUnBindInputput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VencUnBindInputput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_VencBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VencBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VencBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_DEMO_VencUnBindOutput(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    printk("VencUnBindOutput pstChnCurryPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnCurryPort->eModId,pstChnCurryPort->u32ChnId,pstChnCurryPort->u32PortId);
    printk("VencnBindOutput pstChnPeerPort (ModuleId %d , ChnId %d , PortId %d )\n",pstChnPeerPort->eModId,pstChnPeerPort->u32ChnId,pstChnPeerPort->u32PortId);
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_VencRegistDev(MI_SYS_DRV_HANDLE *pHandle)
{
    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevOps;
    #ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    #endif
    void *pUsrData = NULL;
    memset(&stModDevInfo, 0 , sizeof(stModDevInfo));
    memset(&stModDevOps, 0 , sizeof(stModDevOps));

    stModDevInfo.eModuleId = E_MI_MODULE_ID_VENC;
    stModDevInfo.u32DevId = 0;
    stModDevInfo.u32DevChnNum = Venc_CHN_NUM;
    stModDevInfo.u32InputPortNum = Venc_INPUT_NUM;
    stModDevInfo.u32OutputPortNum = Venc_OUTPUT_NUM;

    stModDevOps.OnBindInputPort = _MI_SYS_DEMO_VencBindInput;
    stModDevOps.OnBindOutputPort = _MI_SYS_DEMO_VencBindOutput;
    stModDevOps.OnUnBindInputPort = _MI_SYS_DEMO_VencUnBindInput;
    stModDevOps.OnUnBindOutputPort = _MI_SYS_DEMO_VencUnBindOutput;
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

void venc_init(void)
{
    MI_U32 u32Chn = 0;

    MI_PRINT("%s\n", __func__);
    _MI_SYS_VencRegistDev(&hVencHandle);


    for(u32Chn = 0 ; u32Chn < Venc_CHN_NUM ; u32Chn ++)
    {
        mi_sys_EnableInputPort(hVencHandle,u32Chn,0);
        mi_sys_EnableOutputPort(hVencHandle,u32Chn,0);
        mi_sys_EnableChannel(hVencHandle ,u32Chn);
    }
    kthread_run(work_thread_Venc, NULL, "Venc");
}


