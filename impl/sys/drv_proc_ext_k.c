#include "drv_proc_ext_k.h"

#ifdef MI_SYS_PROC_FS_DEBUG

extern struct workqueue_struct *mi_sys_debug_workqueue;
extern struct idr idr_delay_worker;

static  DRV_PROC_INTFPARAM  *procIntfParam = NULL;
MI_S32 _MI_SYS_IMPL_Drv_Proc_RegisterParam(DRV_PROC_INTFPARAM *param)
{
    if (NULL == param)
    {
        return E_MI_ERR_FAILED;
    }
    if( (param->addDevfun == NULL) ||
        (param->rmvDevfun == NULL))
    {
        return E_MI_ERR_FAILED;
    }
    procIntfParam = param;
    return MI_SUCCESS;
}

void _MI_SYS_IMPL_Drv_Proc_UnRegisterParam(void)
{
    procIntfParam = NULL;
    return;
}

DRV_PROC_ITEM_S* _MI_SYS_IMPL_Drv_Proc_AddDev(MI_U8 *entry_name ,DRV_PROC_EX_S* pfnOpt, void * data,struct proc_dir_entry *proc_dir_entry)
{
    if(procIntfParam){
        if(procIntfParam->addDevfun){
            return procIntfParam->addDevfun(entry_name, pfnOpt, data,proc_dir_entry);
        }
    }
    return NULL;
}

void _MI_SYS_IMPL_Drv_Proc_RemoveDev(char *entry_name)
{
    if(procIntfParam){
        if(procIntfParam->rmvDevfun){
            procIntfParam->rmvDevfun(entry_name);
        }
    }
    return;
}

void _MI_SYS_IMPL_ModuleIdToPrefixName(MI_ModuleId_e eModuleId , char *prefix_name)
{
    switch(eModuleId)
    {
        case E_MI_MODULE_ID_IVE:
            strcpy(prefix_name,"mi_ive");
            break;
        case E_MI_MODULE_ID_VDF:
            strcpy(prefix_name,"mi_vdf");
            break;
        case E_MI_MODULE_ID_VENC:
            strcpy(prefix_name,"mi_venc");
            break;
        case E_MI_MODULE_ID_RGN:
            strcpy(prefix_name,"mi_rgn");
            break;
        case E_MI_MODULE_ID_AI:
            strcpy(prefix_name,"mi_ai");
            break;
        case E_MI_MODULE_ID_AO:
            strcpy(prefix_name,"mi_ao");
            break;
        case E_MI_MODULE_ID_VIF:
            strcpy(prefix_name,"mi_vif");
            break;
        case E_MI_MODULE_ID_VPE:
            strcpy(prefix_name,"mi_vpe");
            break;
        case E_MI_MODULE_ID_VDEC:
            strcpy(prefix_name,"mi_vdec");
            break;
        case E_MI_MODULE_ID_SYS:
            strcpy(prefix_name,"mi_sys");
            break;
         case E_MI_MODULE_ID_FB:
            strcpy(prefix_name,"mi_fb");
            break;
         case E_MI_MODULE_ID_HDMI:
            strcpy(prefix_name,"mi_hdmi");
            break;
         case E_MI_MODULE_ID_DIVP:
            strcpy(prefix_name,"mi_divp");
            break;
         case E_MI_MODULE_ID_GFX:
            strcpy(prefix_name,"mi_gfx");
            break;
         case E_MI_MODULE_ID_VDISP:
            strcpy(prefix_name,"mi_vdisp");
            break;
        case E_MI_MODULE_ID_DISP:
            strcpy(prefix_name,"mi_disp");
            break;
        case E_MI_MODULE_ID_OS:
            strcpy(prefix_name,"mi_os");
            break;
        case E_MI_MODULE_ID_IAE:
            strcpy(prefix_name,"mi_iae");
            break;
        case E_MI_MODULE_ID_MD:
            strcpy(prefix_name,"mi_md");
            break;
        case E_MI_MODULE_ID_OD:
            strcpy(prefix_name,"mi_od");
            break;
        case E_MI_MODULE_ID_SHADOW:
            strcpy(prefix_name,"mi_shadow");
            break;
        case E_MI_MODULE_ID_WARP:
            strcpy(prefix_name,"mi_warp");
            break;
        case E_MI_MODULE_ID_UAC:
            strcpy(prefix_name,"mi_uac");
            break;
        case E_MI_MODULE_ID_LDC:
            strcpy(prefix_name,"mi_ldc");
            break;
        case E_MI_MODULE_ID_SD:
            strcpy(prefix_name,"mi_sd");
            break;
        case E_MI_MODULE_ID_PANEL:
            strcpy(prefix_name,"mi_panel");
            break;
        default:
            DBG_ERR("fail,error!!!  eModuleId is %d ,bigger than E_MI_MODULE_ID_MAX=%d\n",eModuleId,E_MI_MODULE_ID_MAX);
            MI_SYS_BUG_ON(1);
            break;

    }
    return;
}

static MI_S32 _MI_SYS_IMPL_Common_Help(struct seq_file* q)
{
    DRV_PROC_ITEM_S* pstProcItem;
    MI_SYS_DEBUG_HANDLE_t  handle;
    char module_name[20];


    handle.file = (void *)q;
    handle.OnPrintOut = OnPrintOut_linux_vprintk;
    pstProcItem = q->private;
    _MI_SYS_IMPL_ModuleIdToPrefixName(pstProcItem->pstModDev->eModuleId , module_name);
    handle.OnPrintOut(handle,"-------------------------------%s start----------------------------------\n",__FUNCTION__);
    handle.OnPrintOut(handle,"entry_name=%s  eModuleId=0x%x  DevId=0x%x   InputPortNum=0x%x   OutputPortNum=0x%x\n",pstProcItem->entry_name,pstProcItem->pstModDev->eModuleId
                                                ,pstProcItem->pstModDev->u32DevId,pstProcItem->pstModDev->u32InputPortNum,pstProcItem->pstModDev->u32OutputPortNum);
    handle.OnPrintOut(handle,"use     cat /proc/mi_modules/%s/%s     to get attr supported by MI_SYS and this module\n",module_name,pstProcItem->entry_name);
    handle.OnPrintOut(handle,"use     echo help > /proc/mi_modules/%s/%s     to get help\n",module_name,pstProcItem->entry_name);
    handle.OnPrintOut(handle,"use     echo dump_buffer parameter1 parameter2 parameter3 parameter4  parameter5 parameter6 > /proc/mi_modules/%s/%s     to dump buffer by MI_SYS\n",module_name,pstProcItem->entry_name);
    handle.OnPrintOut(handle,"                         parameter1  chn_id: means channel id.\n");
    handle.OnPrintOut(handle,"                         parameter2  port_type: value is \"iport\" or \"oport\".\n");
    handle.OnPrintOut(handle,"                         parameter3  port_id: means port id of this channel.\n");
    handle.OnPrintOut(handle,"                         parameter4  Queue_name:for input port ,only can be \"UsrInject\" or  \"BindInput\";for output port ,only support \"UsrGetFifo\".\n");
    handle.OnPrintOut(handle,"                         parameter5  path:result file is stored in which dir,should be absolute path.\n");
    handle.OnPrintOut(handle,"                         parameter6  end_method:When will end dump process.only support \"bufnum=xxx\" ,or \"time=xxx\"(here unit is ms) or \"start/end\" pair.\n");
    handle.OnPrintOut(handle,"-------------------------------%s end----------------------------------\n",__FUNCTION__);
    return MI_SUCCESS;
}

//for echo
MI_S32  OnPrintOut_linux_vprintk(MI_SYS_DEBUG_HANDLE_t  handle, char *data,...)
{
    va_list arg_ptr;
    va_start(arg_ptr, data);
    vprintk(data,arg_ptr);
    va_end(arg_ptr);
    return MI_SUCCESS;
}

//for cat
MI_S32  OnPrintOut_linux_seq_write(MI_SYS_DEBUG_HANDLE_t  handle, char *data,...)
{
    struct seq_file *seq;
    va_list arg_ptr;

    seq = (struct seq_file *)handle.file;
    va_start(arg_ptr, data);
    seq_vprintf(seq,data,arg_ptr);
    va_end(arg_ptr);

    return MI_SUCCESS;
}


ssize_t MI_SYS_BufQueueStatus_IMPL_ProcWrite( struct file * file,  const char __user * buf,
                     size_t count, loff_t *ppos)
{
    DBG_WRN("%s not support !!!do nothing!!!\n",__FUNCTION__);

    //return input parameter count
    return count;
}

extern struct list_head mi_sys_global_dev_list;
extern struct semaphore mi_sys_global_dev_list_semlock;

int MI_SYS_BufQueueStatus_IMPL_ProcRead(struct seq_file *s, void *pArg)
{
    MI_SYS_MOD_DEV_t *pstModDev;
    struct list_head *pos;
    MI_SYS_DEBUG_HANDLE_t  handle;
    MI_U32 u32ChnId;
    MI_U32 u32PortId;
    MI_SYS_InputPort_t *pstInputPort;
    MI_SYS_OutputPort_t *pstOutputPort;
    handle.file = (void *)s;
    handle.OnPrintOut =  OnPrintOut_linux_seq_write;

    down(&mi_sys_global_dev_list_semlock);

    handle.OnPrintOut(handle,"dump Queues in input port only for enabled port:\n");
    handle.OnPrintOut(handle,"%8s%8s%7s%12s%16s%17s%13s%14s%14s%15s\n"
                             ,"ModId","DevId","ChnId","InPortId","UsrInjectQ_cnt","UsrInjectQ_size"
                             ,"BindInQ_cnt","BindInQ_size","WorkingQ_cnt","WorkingQ_size");
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);

        for(u32ChnId=0;u32ChnId<pstModDev->u32DevChnNum;u32ChnId++)
        {
            for(u32PortId=0;u32PortId<pstModDev->u32InputPortNum;u32PortId++)
            {
                pstInputPort = pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
                MI_SYS_BUG_ON(!pstInputPort);
                if(pstInputPort->bPortEnable)
                {
                    handle.OnPrintOut(handle,"%8d%8d",pstModDev->eModuleId,pstModDev->u32DevId);
                    handle.OnPrintOut(handle,"%7d%12d",u32ChnId,u32PortId);
                    handle.OnPrintOut(handle,"%16d%17lld"
                                     ,pstInputPort->stUsrInjectBufQueue.queue_buf_count
                                     ,mi_sys_calc_buf_size_in_queue(&pstInputPort->stUsrInjectBufQueue));

                    handle.OnPrintOut(handle,"%13d%14lld"
                                     ,pstInputPort->stBindInputBufQueue.queue_buf_count
                                     ,mi_sys_calc_buf_size_in_queue(&pstInputPort->stBindInputBufQueue));

                    handle.OnPrintOut(handle,"%14d%15lld\n"
                                     ,pstInputPort->stWorkingQueue.queue_buf_count
                                     ,mi_sys_calc_buf_size_in_queue(&pstInputPort->stWorkingQueue));
                 }
            }
        }
    }
    handle.OnPrintOut(handle,"dump Queues in output port only for enabled port:\n");
    handle.OnPrintOut(handle,"%8s%8s%7s%12s%20s%20s%17s%17s%14s%15s\n"
                              ,"ModId","DevId","ChnId","OutPortId"
                              ,"DrvBkRefFifoQ_cnt","DrvBkRefFifoQ_size","UsrGetFifoQ_cnt","UsrGetFifoQ_size"
                              ,"WorkingQ_cnt","WorkingQ_size");
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);

        for(u32ChnId=0;u32ChnId<pstModDev->u32DevChnNum;u32ChnId++)
        {
            for(u32PortId=0;u32PortId<pstModDev->u32OutputPortNum;u32PortId++)
            {
                pstOutputPort = pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
                MI_SYS_BUG_ON(!pstOutputPort);
                if(pstOutputPort->bPortEnable)
                {
                    handle.OnPrintOut(handle,"%8d%8d",pstModDev->eModuleId,pstModDev->u32DevId);
                    handle.OnPrintOut(handle,"%7d%12d",u32ChnId,u32PortId);

                    handle.OnPrintOut(handle,"%20d%20lld"
                                     ,pstOutputPort->stDrvBkRefFifoQueue.queue_buf_count
                                     ,mi_sys_calc_buf_size_in_queue(&pstOutputPort->stDrvBkRefFifoQueue));
                    handle.OnPrintOut(handle,"%17d%17lld"
                                     ,pstOutputPort->stUsrGetFifoBufQueue.queue_buf_count
                                     ,mi_sys_calc_buf_size_in_queue(&pstOutputPort->stUsrGetFifoBufQueue));
                    handle.OnPrintOut(handle,"%14d%15lld\n"
                                     ,pstOutputPort->stWorkingQueue.queue_buf_count
                                     ,mi_sys_calc_buf_size_in_queue(&pstOutputPort->stWorkingQueue));
                }
            }
        }
    }
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    handle.OnPrintOut(handle,"\n\ndump Queues LL in Input port only for enabled port:\n");
    handle.OnPrintOut(handle,"%s%4s%4s%5s%9s%13s%13s%13s%13s%13s%13s%13s%13s%13s%13s%13s\n"
                             ,"MId","DId","CId","IPId","UsrEnTD","UsrEnLD","UsrDeTD","UsrDeLD"
                             ,"BindEnTD","BindEnLD","BindDeTD","BindDeLD"
                             ,"WorkEnTD","WorkEnLD","WorkDeTD","WorkDeLD");
    list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);

        for(u32ChnId=0;u32ChnId<pstModDev->u32DevChnNum;u32ChnId++)
        {
            for(u32PortId=0;u32PortId<pstModDev->u32InputPortNum;u32PortId++)
            {
                pstInputPort = pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
                MI_SYS_BUG_ON(!pstInputPort);
                if(pstInputPort->bPortEnable)
                {
                    handle.OnPrintOut(handle,"%3d%3d",pstModDev->eModuleId,pstModDev->u32DevId);
                    handle.OnPrintOut(handle,"%3d%3d",u32ChnId,u32PortId);
                    handle.OnPrintOut(handle,"%13lld%13lld%13lld%13lld"
                                     ,pstInputPort->stUsrInjectBufQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstInputPort->stUsrInjectBufQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS
                                     ,pstInputPort->stUsrInjectBufQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstInputPort->stUsrInjectBufQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS);

                    handle.OnPrintOut(handle,"%13lld%13lld%13lld%13lld"
                                     ,pstInputPort->stBindInputBufQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstInputPort->stBindInputBufQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS
                                     ,pstInputPort->stBindInputBufQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstInputPort->stBindInputBufQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS);

                    handle.OnPrintOut(handle,"%13lld%13lld%13lld%13lld\n"
                                     ,pstInputPort->stWorkingQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstInputPort->stWorkingQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS
                                     ,pstInputPort->stWorkingQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstInputPort->stWorkingQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS);
                 }
            }
        }
    }

    handle.OnPrintOut(handle,"dump Queues LL in Output port only for enabled port:\n");
    handle.OnPrintOut(handle,"%s%4s%4s%5s%9s%13s%13s%13s%13s%13s%13s%13s%13s%13s%13s%13s\n"
                             ,"MId","DId","CId","OPId","DrvEnTD","DrvEnLD","DrvDeTD","DrvDeLD"
                             ,"UsrEnTD","UsrEnLD","UsrDeTD","UsrDeLD"
                             ,"WorkEnTD","WorkEnLD","WorkDeTD","WorkDeLD");

     list_for_each(pos, &mi_sys_global_dev_list)
    {
        pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
        MI_SYS_BUG_ON(!pstModDev);

        for(u32ChnId=0;u32ChnId<pstModDev->u32DevChnNum;u32ChnId++)
        {
            for(u32PortId=0;u32PortId<pstModDev->u32OutputPortNum;u32PortId++)
            {
                pstOutputPort = pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
                MI_SYS_BUG_ON(!pstOutputPort);
                if(pstOutputPort->bPortEnable)
                {
                    handle.OnPrintOut(handle,"%3d%3d",pstModDev->eModuleId,pstModDev->u32DevId);
                    handle.OnPrintOut(handle,"%3d%3d",u32ChnId,u32PortId);

                    handle.OnPrintOut(handle,"%13lld%13lld%13lld%13lld"
                                     ,pstOutputPort->stDrvBkRefFifoQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstOutputPort->stDrvBkRefFifoQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS
                                     ,pstOutputPort->stDrvBkRefFifoQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstOutputPort->stDrvBkRefFifoQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS);

                    handle.OnPrintOut(handle,"%13lld%13lld%13lld%13lld"
                                     ,pstOutputPort->stUsrGetFifoBufQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstOutputPort->stUsrGetFifoBufQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS
                                     ,pstOutputPort->stUsrGetFifoBufQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstOutputPort->stUsrGetFifoBufQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS);

                    handle.OnPrintOut(handle,"%13lld%13lld%13lld%13lld\n"
                                     ,pstOutputPort->stWorkingQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstOutputPort->stWorkingQueue.stEnqueueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS
                                     ,pstOutputPort->stWorkingQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalDelayInUS,pstOutputPort->stWorkingQueue.stDequeueBufObjectTrackStaticInfo.u64AccumTotalLastDelayInUS);
                }
            }
        }
    }
 #endif
    up(&mi_sys_global_dev_list_semlock);

    return 0;
}

static void _mi_sys_dump_dev_common_info(struct seq_file* q)
{
    DRV_PROC_ITEM_S* pstProcItem;
    MI_SYS_DEBUG_HANDLE_t  handle;
    MI_U32 u32ChnId;
    MI_U32 enabled_chnNum;

    pstProcItem = q->private;
    handle.file = (void *)q;
    handle.OnPrintOut =  OnPrintOut_linux_seq_write;
    handle.OnPrintOut(handle,"\n-----------------------------------------Common info for %s-----------------------------------------\n",
            pstProcItem->entry_name);

    enabled_chnNum = 0;
    for(u32ChnId = 0  ; u32ChnId < pstProcItem->pstModDev->u32DevChnNum ; u32ChnId ++)
    {
        if(pstProcItem->pstModDev->astChannels[u32ChnId].bChnEnable)
        {
            enabled_chnNum++;
        }
    }

    handle.OnPrintOut(handle,"%8s%10s%11s%12s%13s\n"
                              ,"ChnNum","EnChnNum","InPortNum","OutPortNum","CollectSize");

    handle.OnPrintOut(handle,"%8d%10d%11d%12d%13d\n"
                               ,pstProcItem->pstModDev->u32DevChnNum
                               ,enabled_chnNum
                               ,pstProcItem->pstModDev->u32InputPortNum
                               ,pstProcItem->pstModDev->u32OutputPortNum,pstProcItem->pstModDev->stAllocatorCollection.collection_size);
}

static void _mi_sys_dump_chn_common_info(struct seq_file* q)
{
    DRV_PROC_ITEM_S* pstProcItem;
    MI_SYS_DEBUG_HANDLE_t  handle;
    MI_U32 u32PortId;
    MI_U32 u32EnabledNum;
    MI_U32 u32ChnId;
    pstProcItem = q->private;
    handle.file = (void *)q;
    handle.OnPrintOut =  OnPrintOut_linux_seq_write;

    handle.OnPrintOut(handle,"\n------------------------------Common info for %s only dump enabled chn------------------------------\n",
        pstProcItem->entry_name);

    handle.OnPrintOut(handle,"%7s%10s%11s%13s\n"
                              ,"ChnId","EnInPNum","EnOutPNum","MMAHeapName");

    for(u32ChnId=0;u32ChnId<pstProcItem->pstModDev->u32DevChnNum;u32ChnId++)
    {
        if(pstProcItem->pstModDev->astChannels[u32ChnId].bChnEnable)
        {
            handle.OnPrintOut(handle,"%7d",u32ChnId);

            u32EnabledNum = 0;
            for(u32PortId = 0  ; u32PortId < pstProcItem->pstModDev->u32InputPortNum ; u32PortId ++)
            {
                MI_SYS_InputPort_t *pstInputPort = pstProcItem->pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
                if(pstInputPort && pstInputPort->bPortEnable)
                {
                    u32EnabledNum++;
                }
            }
            handle.OnPrintOut(handle,"%8d",u32EnabledNum);

            u32EnabledNum = 0;
            for(u32PortId = 0  ; u32PortId < pstProcItem->pstModDev->u32OutputPortNum ; u32PortId ++)
            {
                MI_SYS_OutputPort_t *pstOutputPort = pstProcItem->pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
                if(pstOutputPort && pstOutputPort->bPortEnable)
                {
                    u32EnabledNum++;
                }
            }
            handle.OnPrintOut(handle,"%11d",u32EnabledNum);

            handle.OnPrintOut(handle,"%13s\n",pstProcItem->pstModDev->astChannels[u32ChnId].pu8MMAHeapName);
        }
    }
}

static void _mi_sys_dump_inputport_common_info(struct seq_file* q)
{
    DRV_PROC_ITEM_S* pstProcItem;
    MI_SYS_DEBUG_HANDLE_t  handle;
    MI_SYS_InputPort_t *pstInputPort;
    MI_U32 u32ChnId;
    MI_U32 u32PortId;
    char bind_module_name[20];

    pstProcItem = q->private;
    handle.file = (void *)q;
    handle.OnPrintOut =  OnPrintOut_linux_seq_write;

    handle.OnPrintOut(handle,"\n-------------------------Input port common info for %s  only dump enabled port---------------------\n",
        pstProcItem->entry_name);

    handle.OnPrintOut(handle,"%7s%8s%12s%12s%16s%16s%17s%13s%14s%14s%15s%20s\n"
                             ,"ChnId","PortId","SrcFrmrate","DstFrmrate","user_buf_quota","UsrInjectQ_cnt","UsrInjectQ_size"
                             ,"BindInQ_cnt","BindInQ_size","WorkingQ_cnt","WorkingQ_size","usrLockedInjectCnt");

    for(u32ChnId=0;u32ChnId<pstProcItem->pstModDev->u32DevChnNum;u32ChnId++)
    {
        for(u32PortId=0;u32PortId<pstProcItem->pstModDev->u32InputPortNum;u32PortId++)
        {
            pstInputPort = pstProcItem->pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
            MI_SYS_BUG_ON(!pstInputPort);
            if(pstInputPort->bPortEnable)
            {
                handle.OnPrintOut(handle,"%7d%8d",u32ChnId,u32PortId);

                handle.OnPrintOut(handle,"%12d%12d",pstInputPort->u32SrcFrmrate,pstInputPort->u32DstFrmrate);

                handle.OnPrintOut(handle,"%16d",MI_SYS_USR_INJECT_BUF_CNT_QUOTA_DEFAULT);

                handle.OnPrintOut(handle,"%16d%17lld"
                                     ,pstInputPort->stUsrInjectBufQueue.queue_buf_count
                                     ,mi_sys_calc_buf_size_in_queue(&pstInputPort->stUsrInjectBufQueue));

                handle.OnPrintOut(handle,"%13d%14lld"
                                     ,pstInputPort->stBindInputBufQueue.queue_buf_count
                                     ,mi_sys_calc_buf_size_in_queue(&pstInputPort->stBindInputBufQueue));

                handle.OnPrintOut(handle,"%14d%15lld"
                                     ,pstInputPort->stWorkingQueue.queue_buf_count
                                     ,mi_sys_calc_buf_size_in_queue(&pstInputPort->stWorkingQueue));

                handle.OnPrintOut(handle,"%20d\n"
                            ,atomic_read(&pstInputPort->usrLockedInjectBufCnt));
            }
        }
    }

    handle.OnPrintOut(handle,"\n%7s%8s%16s%18s%12s%13s\n"
                              ,"ChnId","PortId","bind_module_id","bind_module_name","bind_ChnId","bind_PortId");

    for(u32ChnId=0;u32ChnId<pstProcItem->pstModDev->u32DevChnNum;u32ChnId++)
    {
        for(u32PortId=0;u32PortId<pstProcItem->pstModDev->u32InputPortNum;u32PortId++)
        {
            pstInputPort = pstProcItem->pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
            MI_SYS_BUG_ON(!pstInputPort);
            if(pstInputPort->bPortEnable)
            {
                if(!pstInputPort->pstBindPeerOutputPort)
                {
                    //do nothing
                    //handle.OnPrintOut(handle,"do not have BindPeerOutputPort\n");
                }
                else
                {
                    _MI_SYS_IMPL_ModuleIdToPrefixName(pstInputPort->pstBindPeerOutputPort->pstChannel->pstModDev->eModuleId,bind_module_name);
                        handle.OnPrintOut(handle,"%7d%8d%16d%18s%12d%13d\n",u32ChnId,u32PortId
                        ,pstInputPort->pstBindPeerOutputPort->pstChannel->pstModDev->eModuleId
                        ,bind_module_name,pstInputPort->pstBindPeerOutputPort->pstChannel->u32ChannelId
                        ,pstInputPort->pstBindPeerOutputPort->u32PortId);
                }
            }
            else
            {
                //do nothing
            }
        }
    }

    handle.OnPrintOut(handle,"\n%7s%8s%20s%25s%25s\n"
                                 ,"ChnId","PortId","LowLatencyDelayMs","scheduledDelayTaskCnt"
                                 ,"LastStaticDelayAveMS");

    for(u32ChnId=0;u32ChnId<pstProcItem->pstModDev->u32DevChnNum;u32ChnId++)
    {
        for(u32PortId=0;u32PortId<pstProcItem->pstModDev->u32InputPortNum;u32PortId++)
        {
            pstInputPort = pstProcItem->pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
            MI_SYS_BUG_ON(!pstInputPort);
            if(pstInputPort->bPortEnable)
            {
                if(pstInputPort->bEnableLowLatencyReceiveMode == TRUE)
                {
                    handle.OnPrintOut(handle,"%7d%8d",u32ChnId,u32PortId);

                    handle.OnPrintOut(handle,"%20d%25d",pstInputPort->u32LowLatencyDelayMs,atomic_read(&pstInputPort->scheduledDelayTaskCnt));
                    handle.OnPrintOut(handle,"%25d\n",pstInputPort->u32LastStaticDelayTotalMS/pstInputPort->u32LastStaticDelayTotalCnt);
                }
            }
        }
    }

}

static void _mi_sys_dump_outputport_common_info(struct seq_file* q)
{
    DRV_PROC_ITEM_S* pstProcItem;
    MI_SYS_DEBUG_HANDLE_t  handle;
    MI_SYS_OutputPort_t *pstOutputPort;
    MI_SYS_InputPort_t *pstCur = NULL;
    char bind_module_name[20];
    struct list_head *pos;
    MI_U32 u32ChnId;
    MI_U32 u32PortId;

    pstProcItem = q->private;
    handle.file = (void *)q;
    handle.OnPrintOut =  OnPrintOut_linux_seq_write;

    handle.OnPrintOut(handle,"\n----------------------Output port common info for %s  only for enabled port---------------------\n",pstProcItem->entry_name);

    handle.OnPrintOut(handle,"%7s%8s%9s%13s%14s%20s%20s%20s\n"
                              ,"ChnId","PortId","usrDepth","BufCntQuota","usrLockedCnt","totalOutPortInUsed"
                              ,"DrvBkRefFifoQ_cnt","DrvBkRefFifoQ_size");

    for(u32ChnId=0;u32ChnId<pstProcItem->pstModDev->u32DevChnNum;u32ChnId++)
    {
        for(u32PortId=0;u32PortId<pstProcItem->pstModDev->u32OutputPortNum;u32PortId++)
        {
            pstOutputPort = pstProcItem->pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
            MI_SYS_BUG_ON(!pstOutputPort);
            if(pstOutputPort->bPortEnable)
            {
                handle.OnPrintOut(handle,"%7d%8d%9d%13d",u32ChnId,u32PortId,pstOutputPort->u32UsrFifoCount, pstOutputPort->u32OutputPortBufCntQuota);

                handle.OnPrintOut(handle,"%14d%20d"
                                                    ,atomic_read(&pstOutputPort->usrLockedBufCnt)
                                                    ,atomic_read(&pstOutputPort->totalOutputPortInUsedBuf));

                handle.OnPrintOut(handle,"%20d%20lld\n"
                                 ,pstOutputPort->stDrvBkRefFifoQueue.queue_buf_count
                                 ,mi_sys_calc_buf_size_in_queue(&pstOutputPort->stDrvBkRefFifoQueue));
            }
        }
    }
    handle.OnPrintOut(handle,"%7s%8s%17s%17s%20s%24s%14s%15s\n"
                                  ,"ChnId","PortId","UsrGetFifoQ_cnt","UsrGetFifoQ_size"
                                  ,"UsrGetFifoQ_seqnum","UsrGetFifoQ_discardnum"
                                  ,"WorkingQ_cnt","WorkingQ_size");
    for(u32ChnId=0;u32ChnId<pstProcItem->pstModDev->u32DevChnNum;u32ChnId++)
    {
        for(u32PortId=0;u32PortId<pstProcItem->pstModDev->u32OutputPortNum;u32PortId++)
        {
            pstOutputPort = pstProcItem->pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
            MI_SYS_BUG_ON(!pstOutputPort);
            if(pstOutputPort->bPortEnable)
            {
                handle.OnPrintOut(handle,"%7d%8d",u32ChnId,u32PortId);
                handle.OnPrintOut(handle,"%17d%17lld"
                                 ,pstOutputPort->stUsrGetFifoBufQueue.queue_buf_count
                                 ,mi_sys_calc_buf_size_in_queue(&pstOutputPort->stUsrGetFifoBufQueue));
                handle.OnPrintOut(handle,"%20lld%24lld"
                                 ,pstOutputPort->u64UsrGetFifoBufQueueAddCnt
                                 ,pstOutputPort->u64UsrGetFifoBufQueueDiscardCnt);

                handle.OnPrintOut(handle,"%14d%15lld\n"
                                 ,pstOutputPort->stWorkingQueue.queue_buf_count
                                 ,mi_sys_calc_buf_size_in_queue(&pstOutputPort->stWorkingQueue));
            }
        }
    }

    handle.OnPrintOut(handle,"\n----------------------------------------BindPeerInputPortList---------------------------------------\n");
    handle.OnPrintOut(handle,"%7s%8s%16s%18s%12s%13s\n"
                             ,"ChnId","PortId","bind_module_id","bind_module_name","bind_ChnId","bind_PortId");

    for(u32ChnId=0;u32ChnId<pstProcItem->pstModDev->u32DevChnNum;u32ChnId++)
    {
        for(u32PortId=0;u32PortId<pstProcItem->pstModDev->u32OutputPortNum;u32PortId++)
        {
            pstOutputPort = pstProcItem->pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
            MI_SYS_BUG_ON(!pstOutputPort);
            if(pstOutputPort->bPortEnable)
            {
                down(&pstOutputPort->stBindedInputListSemlock);
                list_for_each(pos, &pstOutputPort->stBindPeerInputPortList)
                {
                    pstCur = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
                    MI_SYS_BUG_ON(pstCur->pstBindPeerOutputPort != pstOutputPort);
                    _MI_SYS_IMPL_ModuleIdToPrefixName(pstCur->pstChannel->pstModDev->eModuleId,bind_module_name);
                    handle.OnPrintOut(handle,"%7d%8d%16d%18s%12d%13d\n",u32ChnId,u32PortId,pstCur->pstChannel->pstModDev->eModuleId,bind_module_name,pstCur->pstChannel->u32ChannelId,pstCur->u32PortId);
                }
                up(&pstOutputPort->stBindedInputListSemlock);

            }
        }
    }
}


static MI_S32 _MI_SYS_IMPL_ParseStringsStart(MI_U8 *pStr, MI_U8 **argv)
{
    MI_U8 *pStr1 = NULL;
    MI_U8 *pStr2 = NULL;
    MI_U8 u8CmdSize = 0;
    MI_U8 i = 0;

    MI_SYS_BUG_ON(!pStr);
    MI_SYS_BUG_ON(!argv);

    pStr1 = pStr;
    while (1)
    {
        while (*pStr1 == ' ' && *pStr1 != 0)
        {
            pStr1++;
        }
        pStr2 = pStr1;
        while (*pStr2 != ' ' && *pStr2 != 0 && *pStr2 != '\t' && *pStr2 != '\r' && *pStr2 != '\n')
        {
            pStr2++;
        }
        u8CmdSize = pStr2 - pStr1;
        if (u8CmdSize != 0)
        {
            argv[i] = kmalloc(u8CmdSize + 1, GFP_KERNEL);
            MI_SYS_BUG_ON(!argv[i]);
            strncpy(argv[i], pStr1, u8CmdSize);
            argv[i][u8CmdSize] = 0;
            i++;
            pStr1 = pStr2;
        }
        else
        {
            break;
        }
    }

    return i;
}
static MI_S32 _MI_SYS_IMPL_ParseStringsEnd(MI_U8 argc, MI_U8 **argv)
{
    MI_U8 i = 0;

    MI_SYS_BUG_ON(!argv);
    MI_SYS_BUG_ON(!argc);

    for(i = 0; i < argc; i++)
    {
        MI_SYS_BUG_ON(!argv[i]);
        kfree(argv[i]);
    }

    return MI_SUCCESS;
}
void _MI_SYS_IMPL_CommonRegCmd(MI_U8 *u8Cmd, MI_U8 u8MaxPara,
                    MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t,MI_U32,MI_U8,MI_U8 **,void *), MI_SYS_MOD_DEV_t *pstModDev)
{
    MI_U8 aszBuf[16];
    MI_U8 prefix_name[10];

    MI_SYS_BUG_ON(!fpExecCmd);
    MI_SYS_BUG_ON(!u8Cmd);
    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->eModuleId >= E_MI_MODULE_ID_MAX);

    memset(aszBuf, 0, sizeof(aszBuf));
    memset(prefix_name, 0, sizeof(prefix_name));
    _MI_SYS_IMPL_ModuleIdToPrefixName(pstModDev->eModuleId,(char *)prefix_name);
    snprintf(aszBuf, sizeof(aszBuf), "%s%d",prefix_name,pstModDev->u32DevId);

    if(procIntfParam){
        if(procIntfParam->addDevCmdfun){
            procIntfParam->addDevCmdfun(u8Cmd, u8MaxPara, fpExecCmd, aszBuf);
        }
    }
}
void _MI_SYS_IMPL_CommonClearCmd(MI_SYS_MOD_DEV_t *pstModDev)
{
    MI_U8 aszBuf[16];
    MI_U8 prefix_name[10];

    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON(pstModDev->eModuleId >= E_MI_MODULE_ID_MAX);

    memset(aszBuf, 0, sizeof(aszBuf));
    memset(prefix_name, 0, sizeof(prefix_name));
    _MI_SYS_IMPL_ModuleIdToPrefixName(pstModDev->eModuleId,(char *)prefix_name);
    snprintf(aszBuf, sizeof(aszBuf), "%s%d",prefix_name,pstModDev->u32DevId);

    if(procIntfParam){
        if(procIntfParam->addDevfun){
            procIntfParam->clearDevCmdfun(aszBuf);
        }
    }

    return;
}

static MI_S32 _MI_SYS_IMPL_OnModExecHelp(MI_SYS_DEBUG_HANDLE_t handle, MI_U32  u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    struct seq_file* q = (struct seq_file *)handle.file;
    DRV_PROC_ITEM_S* pstProcItem = q->private;
    mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps;
    pstModuleProcfsOps = (mi_sys_ModuleDevProcfsOps_t *)(&pstProcItem->Ops);

    if (MI_SUCCESS != _MI_SYS_IMPL_Common_Help(q))
    {
        return E_MI_ERR_BUSY;
    }
    if(pstModuleProcfsOps->OnHelp)
    {
        return pstModuleProcfsOps->OnHelp(handle, u32DevId, pUsrData);
    }

    return MI_SUCCESS;
}

static __inline__ int str2val(char *str,unsigned int str_len ,unsigned int *data)
{
    unsigned int i, d, dat, weight;
    dat = 0;
    if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        i = 2;
        weight = 16;
    }
    else
    {
        i = 0;
        weight = 10;
    }

    for(; i < str_len; i++)
    {
        if (str[i] < 0x20)
        {
            break;
        }
        else if (weight == 16 && str[i] >= 'a' && str[i] <= 'f')
        {
            d = str[i] - 'a' + 10;
        }
        else if (weight == 16 && str[i] >= 'A' && str[i] <= 'F')
        {
            d = str[i] - 'A' + 10;
        }
        else if (str[i] >= '0' && str[i] <= '9')
        {
            d = str[i] - '0';
        }
        else
        {
            return -1;
        }

        dat = dat * weight + d;
    }

    *data = dat;

    return 0;
}

static MI_S32 _MI_SYS_IMPL_DumpBufferParCheck(MI_SYS_DEBUG_HANDLE_t handle, MI_U32  u32DevId, MI_U8 argc, MI_U8 **argv, MI_SYS_MOD_DEV_t *pstModDev)
{
    MI_U32 u32ChnId=0;
    MI_U32 u32PortId=0;
    MI_U8 module_name[20];
    int i=0;
    //handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
    _MI_SYS_IMPL_ModuleIdToPrefixName(pstModDev->eModuleId,module_name);
    //handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
    if((argc -1 ) != PROC_DUMP_BUFFER_COMMAND_PARAMETER_NUM)
    {
        handle.OnPrintOut(handle,"support and only support %d parameters!\n",PROC_DUMP_BUFFER_COMMAND_PARAMETER_NUM);
        handle.OnPrintOut(handle,"such as:\n");
        handle.OnPrintOut(handle,"echo dump_buffer channel_id port_type port_id Queue_name path stop_method > /proc/mi_modules/%s/%s%d\n",module_name,module_name,u32DevId);
        return E_MI_ERR_ILLEGAL_PARAM;
    }
//    handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
    for(i = 1;i <= PROC_DUMP_BUFFER_COMMAND_PARAMETER_NUM;i++)
    {
        handle.OnPrintOut(handle,"argv[%d]=%s \n",i,argv[i]);
    }
    //handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
    str2val(argv[1],strlen(argv[1]),&u32ChnId);
    //handle.OnPrintOut(handle,"u32ChnId = %u\n",u32ChnId);
    if(u32ChnId >= pstModDev->u32DevChnNum)
    {
        handle.OnPrintOut(handle,"u32ChnId %u should less than %u !!!\n",u32ChnId,pstModDev->u32DevChnNum);
        return E_MI_ERR_INVALID_CHNID;
    }

    if(0 == strncmp("iport",argv[2],strlen("iport")))
    {
        str2val(argv[3],strlen(argv[3]),&u32PortId);
        if(u32PortId >= pstModDev->u32InputPortNum)
        {
            handle.OnPrintOut(handle,"%s:%d  u32PortId %u  is larger than u32InputPortNum  %u\n",__FUNCTION__,__LINE__,u32PortId,pstModDev->u32InputPortNum);
            return E_MI_ERR_FAILED;
        }
        if((0 != strncmp("UsrInject",argv[4],strlen("UsrInject")))
            &&(0 != strncmp("BindInput",argv[4],strlen("BindInput"))))
            {
                handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
                return E_MI_ERR_ILLEGAL_PARAM;
            }
    }
    else if(0 == strncmp("oport",argv[2],strlen("oport")))
    {
        str2val(argv[3],strlen(argv[3]),&u32PortId);
        if(u32PortId >= pstModDev->u32OutputPortNum)
        {
            handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
            return E_MI_ERR_FAILED;
        }

        if((0 != strncmp("UsrGetFifo",argv[4],strlen("UsrGetFifo"))))
        {
            handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
            return E_MI_ERR_ILLEGAL_PARAM;
        }
    }
    else
    {
        handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
        return E_MI_ERR_ILLEGAL_PARAM;
    }

    if((0 != strncmp("bufnum=",argv[6],strlen("bufnum=")))
        &&(0 != strncmp("time=",argv[6],strlen("time=")))
        &&(0 != strncmp("start",argv[6],strlen("start")))
        &&(0 != strncmp("end",argv[6],strlen("end"))))
    {
        handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
        return E_MI_ERR_ILLEGAL_PARAM;
    }


    return MI_SUCCESS;
}

//each buf will be dumpped into different file.
MI_S32  dump_buf_data(MI_U32 index,struct queue_dump_buf_cmd_info * dump_buf_cmd_info,MI_SYS_BufRef_t *pstBufRef)
{
    mm_segment_t old_fs;
    struct file *fp;
    MI_PHY phyAddr;
    MI_U32  u32BufSize = 0;
    char path_file_name_buf[256];
    char tmp_buf[100];
    MI_S32 current_name_length=0;
    struct sg_table *sg_table;
    int ret;
    unsigned long long start_addr;
    void *kern_vmap_ptr;

    if(TRUE == dump_buf_cmd_info->force_stop)
    {
        return MI_SUCCESS;//force stop ,do nothing, return success.
    }

    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

    current_name_length = 0;
    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

    strncpy(path_file_name_buf,dump_buf_cmd_info->path,strlen(dump_buf_cmd_info->path));
    current_name_length += strlen(dump_buf_cmd_info->path);
    if(path_file_name_buf[current_name_length-1] != '/')
    {
        path_file_name_buf[current_name_length] = '/';
        current_name_length++;
    }
    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

    sprintf(tmp_buf,"%s__%u__%u__%s__%u__%s",dump_buf_cmd_info->module_name,dump_buf_cmd_info->u32DevId,dump_buf_cmd_info->u32ChnId
                                                  ,(dump_buf_cmd_info->port_type == E_MI_MODULE_DUMP_BUFFER_INPUT_PORT)?"iport":"oport"
                                                  ,dump_buf_cmd_info->port_id
                                                  ,(dump_buf_cmd_info->Queue_name == E_MI_MODULE_DUMP_BUFFER_Queue_UsrInject)?"UsrInject":
                                                  ((dump_buf_cmd_info->Queue_name == E_MI_MODULE_DUMP_BUFFER_Queue_BindInput)?"BindInput":"GetFifo")
                                                 );
    strncpy(path_file_name_buf+current_name_length,tmp_buf,strlen(tmp_buf));
    current_name_length += strlen(tmp_buf);
    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

    if(E_MI_SYS_BUFDATA_RAW == pstBufRef->bufinfo.eBufType)
    {
        //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
        phyAddr = pstBufRef->bufinfo.stRawData.phyAddr;
        u32BufSize = pstBufRef->bufinfo.stRawData.u32BufSize;
        sprintf(tmp_buf,"%s__%u__%u__%llu.bin","RAW",index,u32BufSize,pstBufRef->bufinfo.u64Pts);//future change u64Pts into ms
    }
    else if(E_MI_SYS_BUFDATA_META == pstBufRef->bufinfo.eBufType)
    {
        //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
        phyAddr = pstBufRef->bufinfo.stMetaData.phyAddr;
        u32BufSize = pstBufRef->bufinfo.stMetaData.u32Size;
        sprintf(tmp_buf,"%s__%u__%u__%llu.bin","META",index,u32BufSize,pstBufRef->bufinfo.u64Pts);//future change u64Pts into ms
    }
    else if(E_MI_SYS_BUFDATA_FRAME == pstBufRef->bufinfo.eBufType)
    {
        //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
        phyAddr = pstBufRef->bufinfo.stFrameData.phyAddr[0];
        u32BufSize = pstBufRef->bufinfo.stFrameData.u32BufSize;
        sprintf(tmp_buf,"%s__%u__%hu__%hu__%llu.bin","FRAME",index,pstBufRef->bufinfo.stFrameData.u16Width,pstBufRef->bufinfo.stFrameData.u16Height,pstBufRef->bufinfo.u64Pts);//future change u64Pts into ms
    }
    else
    {
        //DBG_INFO("bufinfo.eBufType = %d\n",pstBufRef->bufinfo.eBufType);
        MI_SYS_BUG();
    }

    strncpy(path_file_name_buf+current_name_length,tmp_buf,strlen(tmp_buf));
    current_name_length += strlen(tmp_buf);

    path_file_name_buf[current_name_length]='\0';
    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

    //DEFAULT printk following log
    DBG_WRN("%s:%d  path_file_name_buf=%s\n",__FUNCTION__,__LINE__,path_file_name_buf);

    if(!u32BufSize)
    {
        DBG_ERR("dump fail, the u32BufSize = %d\n", u32BufSize);
        return E_MI_ERR_FAILED;
    }

    start_addr = mi_sys_Miu2Cpu_BusAddr(phyAddr);

    if(E_MI_SYS_BUFDATA_META != pstBufRef->bufinfo.eBufType)
    {

        sg_table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
        if (!sg_table) {
            DBG_ERR(" fail\n");
            return E_MI_ERR_FAILED;
        }

        ret = sg_alloc_table(sg_table, 1, GFP_KERNEL);
        if (unlikely(ret))
        {
            DBG_ERR(" fail\n");
            kfree(sg_table);
            return E_MI_ERR_FAILED;
        }

        MI_SYS_BUG_ON(start_addr&~PAGE_MASK);
        //MI_SYS_BUG_ON(u32BufSize&~PAGE_MASK);

        sg_set_page(sg_table->sgl, pfn_to_page(__phys_to_pfn(start_addr)), PAGE_ALIGN(u32BufSize), 0);
        //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

        ret = mi_sys_buf_mgr_vmap_kern(sg_table, &kern_vmap_ptr);
        sg_free_table(sg_table);
        kfree(sg_table);
        if(ret != MI_SUCCESS)
        {
            DBG_ERR(" fail\n");
            return ret;
        }
    }
    else
    {
        DBG_INFO("start_addr=0x%llx   page offset 0x%llx\n",start_addr,start_addr%PAGE_SIZE);
        MI_SYS_BUG_ON(0 != ((start_addr%PAGE_SIZE)%MI_SYS_META_DATA_SIZE_MAX));
        kern_vmap_ptr = (char*)page_address(pfn_to_page(__phys_to_pfn(start_addr))) + (start_addr%PAGE_SIZE);
    }

    MI_SYS_BUG_ON(!kern_vmap_ptr);
    DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open((char *)(path_file_name_buf),O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if(IS_ERR(fp))
    {
        DBG_ERR("filp_open fail   PTR_ERR_fp = %d\n",PTR_ERR(fp));//here use PTR_ERR(fp) to show errno
        set_fs(old_fs);
        return E_MI_ERR_FAILED;
    }
    ret = vfs_write(fp, (char *)kern_vmap_ptr, u32BufSize, &fp->f_pos);

    if(E_MI_SYS_BUFDATA_META != pstBufRef->bufinfo.eBufType)
    {
        mi_sys_buf_mgr_vunmap_kern(kern_vmap_ptr);
    }
    else
    {
        //do nothing
    }

    filp_close(fp, NULL);

    set_fs(old_fs);

    if(ret != u32BufSize)
    {
        DBG_ERR(" fail\n");
        return E_MI_ERR_FAILED;
    }
    else
    {
        return MI_SUCCESS;
    }
}

MI_S32 Dump_Buffer_by_Queue(struct queue_dump_buf_cmd_info * dump_buf_cmd_info,MI_SYS_BufferQueue_t*pstQueue)
{
#define DUMP_TIMEOUT    500

    MI_SYS_BufRef_t *pstBufRef;
    struct list_head *pos;
    MI_S32 ret;

    //DBG_INFO("[debug][%s:%d]    pstQueue=%p\n",__FUNCTION__,__LINE__,pstQueue);

    MI_SYS_BUG_ON(!pstQueue);
    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
    //we don't allow queue operations in interrupt context
    MI_SYS_BUG_ON(in_interrupt());
    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

    if(-1 != dump_buf_cmd_info->bufnum)
    {
        MI_S32 need_bufnum = dump_buf_cmd_info->bufnum;
        /* exit while if not buffer data in 5 secs! */
        MI_S32 timeout_cnt = DUMP_TIMEOUT;
        //DBG_INFO("[debug][%s:%d]   need_bufnum=%d   dump_buf_cmd_info->bufnum=%d\n",__FUNCTION__,__LINE__,need_bufnum,dump_buf_cmd_info->bufnum);
        while((need_bufnum > 0) && (TRUE != dump_buf_cmd_info->force_stop))
        {
            DBG_INFO("[debug][%s:%d]   need_bufnum=%d    dump_buf_cmd_info->bufnum=%d\n",__FUNCTION__,__LINE__,need_bufnum,dump_buf_cmd_info->bufnum);
            mutex_lock(&pstQueue->mtx);
            //DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
            list_for_each(pos,&pstQueue->list)
            {
                //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
                pstBufRef = container_of(pos, MI_SYS_BufRef_t, list);
                if(FALSE == pstBufRef->is_dumped)
                {
                    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
                    ret = dump_buf_data(dump_buf_cmd_info->bufnum - need_bufnum,dump_buf_cmd_info,pstBufRef);
                    if(ret != MI_SUCCESS)
                    {
                        mutex_unlock(&pstQueue->mtx);
                        return ret;
                    }
                    pstBufRef->is_dumped = TRUE;
                    need_bufnum--;
                    if(need_bufnum <= 0 || (TRUE == dump_buf_cmd_info->force_stop))
                    {
                        DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
                        mutex_unlock(&pstQueue->mtx);
                        return MI_SUCCESS;
                    }
                }
            }
            pstQueue->dump_buffer_wq_cond = FALSE;
            mutex_unlock(&pstQueue->mtx);
            if (timeout_cnt--)
            {
                wait_event_timeout(pstQueue->dump_buffer_wq, pstQueue->dump_buffer_wq_cond, msecs_to_jiffies(2));
            }
            else
            {
                need_bufnum--;
                DBG_WRN("dump buf timeout, buffer queue is null! left bufnum=%d\n", need_bufnum);
                timeout_cnt = DUMP_TIMEOUT;
                if (!need_bufnum)
                {
                    return MI_SUCCESS;
                }
            }
        }
        return MI_SUCCESS;
    }
    else if(-1 != dump_buf_cmd_info->time)
    {
        struct timeval tv_A,tv_B;
        long interval_ms;
        int index=0;
        //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

        do_gettimeofday(&tv_A);

        do{
            //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
            mutex_lock(&pstQueue->mtx);
            list_for_each(pos,&pstQueue->list)
            {
               pstBufRef = container_of(pos, MI_SYS_BufRef_t, list);
               if(FALSE == pstBufRef->is_dumped)
               {
                   //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
                   ret = dump_buf_data(index,dump_buf_cmd_info,pstBufRef);
                   if(ret != MI_SUCCESS)
                   {
                       mutex_unlock(&pstQueue->mtx);
                       return ret;
                   }
                   pstBufRef->is_dumped = TRUE;
                   //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
                   index++;

                   do_gettimeofday(&tv_B);
                   interval_ms = (tv_B.tv_sec*1000 + tv_B.tv_usec/1000) -(tv_A.tv_sec*1000 + tv_A.tv_usec/1000);
                   if(interval_ms >= dump_buf_cmd_info->time || (TRUE == dump_buf_cmd_info->force_stop))
                   {
                       mutex_unlock(&pstQueue->mtx);
                       return MI_SUCCESS;
                   }
                }
            }
            pstQueue->dump_buffer_wq_cond = FALSE;
            mutex_unlock(&pstQueue->mtx);
            wait_event_timeout(pstQueue->dump_buffer_wq, pstQueue->dump_buffer_wq_cond, msecs_to_jiffies(2));

            do_gettimeofday(&tv_B);
            interval_ms = (tv_B.tv_sec*1000 + tv_B.tv_usec/1000) -(tv_A.tv_sec*1000 + tv_A.tv_usec/1000);
            //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

        }while((interval_ms < dump_buf_cmd_info->time) &&(TRUE != dump_buf_cmd_info->force_stop));
        //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
        return MI_SUCCESS;
    }
    else if(E_MI_MODULE_DUMP_BUFFER_START == dump_buf_cmd_info->start_end)
    {
        MI_U32 index = 0;
        while(TRUE != dump_buf_cmd_info->force_stop)
        {
            DBG_INFO("[debug][%s:%d]    dump_buf_cmd_info->bufnum=%d\n",__FUNCTION__,__LINE__,dump_buf_cmd_info->bufnum);
            mutex_lock(&pstQueue->mtx);
            list_for_each(pos,&pstQueue->list)
            {
               pstBufRef = container_of(pos, MI_SYS_BufRef_t, list);
               if(FALSE == pstBufRef->is_dumped)
               {
                   ret = dump_buf_data(index,dump_buf_cmd_info,pstBufRef);
                   if(ret != MI_SUCCESS)
                   {
                       mutex_unlock(&pstQueue->mtx);
                       return ret;
                   }
                   pstBufRef->is_dumped = TRUE;
                   index++;
                   if(TRUE == dump_buf_cmd_info->force_stop)
                   {
                       DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
                       mutex_unlock(&pstQueue->mtx);
                       return MI_SUCCESS;
                   }
               }
            }
            pstQueue->dump_buffer_wq_cond = FALSE;
            mutex_unlock(&pstQueue->mtx);
            wait_event_timeout(pstQueue->dump_buffer_wq, pstQueue->dump_buffer_wq_cond, msecs_to_jiffies(2));
        }
        return MI_SUCCESS;
    }
    else if(E_MI_MODULE_DUMP_BUFFER_END == dump_buf_cmd_info->start_end)
    {
        MI_SYS_BUG();
        return E_MI_ERR_FAILED;
    }
    else
    {
        MI_SYS_BUG();
        return E_MI_ERR_FAILED;
    }
}

static void dump_Queue_buf_worker(struct work_struct *work)
{
    struct queue_dump_buf_cmd_info * dump_buf_cmd_info;
    MI_SYS_MOD_DEV_t *pstModDev;
    MI_S32 ret;
    dump_buf_cmd_info = container_of(work, struct queue_dump_buf_cmd_info,
    		     queue_dump_buf_data_wq.work);
    DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);

    //DBG_INFO("[debug][%s:%d] dump_buf_cmd_info->bufnum=%d\n",__FUNCTION__,__LINE__,dump_buf_cmd_info->bufnum);

    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
    pstModDev = (MI_SYS_MOD_DEV_t *)dump_buf_cmd_info->pstModDev;
    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
    MI_SYS_BUG_ON(!pstModDev);

    //DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);

    //DBG_INFO("[debug][%s:%d]  module_name=%s\n",__FUNCTION__,__LINE__,dump_buf_cmd_info->module_name);

    if(E_MI_MODULE_DUMP_BUFFER_INPUT_PORT == dump_buf_cmd_info->port_type)
    {
        if(E_MI_MODULE_DUMP_BUFFER_Queue_UsrInject == dump_buf_cmd_info->Queue_name)
        {
            ret = Dump_Buffer_by_Queue(dump_buf_cmd_info,&pstModDev->astChannels[dump_buf_cmd_info->u32ChnId].pastInputPorts[dump_buf_cmd_info->port_id]->stUsrInjectBufQueue);
        }
        else if(E_MI_MODULE_DUMP_BUFFER_Queue_BindInput == dump_buf_cmd_info->Queue_name)
        {
            DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
            ret = Dump_Buffer_by_Queue(dump_buf_cmd_info,&pstModDev->astChannels[dump_buf_cmd_info->u32ChnId].pastInputPorts[dump_buf_cmd_info->port_id]->stBindInputBufQueue);
            DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
        }
        else
        {DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
            MI_SYS_BUG();
            ret = E_MI_ERR_FAILED;
            goto END;
        }
    }
    else if(E_MI_MODULE_DUMP_BUFFER_OUTPUT_PORT == dump_buf_cmd_info->port_type)
    {
         if(E_MI_MODULE_DUMP_BUFFER_Queue_GetFifo == dump_buf_cmd_info->Queue_name)

         {
             ret = Dump_Buffer_by_Queue(dump_buf_cmd_info,&pstModDev->astChannels[dump_buf_cmd_info->u32ChnId].pastOutputPorts[dump_buf_cmd_info->port_id]->stUsrGetFifoBufQueue);
         }
         else
         {
             MI_SYS_BUG();
             ret = E_MI_ERR_FAILED;
             goto END;
         }
    }
    else
    {
         DBG_INFO("[debug][%s:%d]\n",__FUNCTION__,__LINE__);
         MI_SYS_BUG();
         ret = E_MI_ERR_FAILED;
         goto END;
    }

END:


    if(ret != MI_SUCCESS)
    {
        DBG_ERR("failed\n");
    }

    DBG_WRN("cancel_delayed_work-->\n");
    if (!cancel_delayed_work(&dump_buf_cmd_info->queue_dump_buf_data_wq))
    {
        DBG_WRN("cancel_delayed_work ...\n");
    }
    //flush_workqueue(mi_sys_debug_workqueue);

    DBG_WRN("cancel_delayed_work--<\n");

    DBG_WRN("idr_remove-->\n");
    idr_remove(&idr_delay_worker,dump_buf_cmd_info->idr_num);
    kfree(dump_buf_cmd_info);//done,so kfree this

    DBG_WRN("idr_remove--<\n");

    return ;
}

//deal with
//echo force_stop_dump delay_worker_id > /proc/
ssize_t mi_dump_buffer_delay_worker_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    char force_stop_delay_worker_info[100];
    char buf_size = -1;//default value not be 0,because return 0 will cause loop.
    MI_U8* tmp_pcBuf = force_stop_delay_worker_info;
    unsigned int delay_worker_id = 0;
    struct queue_dump_buf_cmd_info *dump_buf_cmd_info;

    buf_size = min(count,(sizeof(force_stop_delay_worker_info)-1));
    if(copy_from_user(force_stop_delay_worker_info,user_buf,buf_size))
    {
        DBG_ERR("%s :%d \n",__FUNCTION__,__LINE__);
        return -EFAULT;
    }

    COMMON_STRING_SKIP_BLANK(tmp_pcBuf);

    if(0 == strncmp(tmp_pcBuf,"force_stop_dump ",strlen("force_stop_dump ")))//N.B. here have blank in end of "force_stop_dump "
    {
        tmp_pcBuf += strlen("force_stop_dump ");
        COMMON_STRING_SKIP_BLANK(tmp_pcBuf);
        str2val(tmp_pcBuf,strlen(tmp_pcBuf),&delay_worker_id);
        DBG_INFO("delay_worker_id is %u\n",delay_worker_id);
        dump_buf_cmd_info = idr_find(&idr_delay_worker,delay_worker_id);
        if(!dump_buf_cmd_info)
        {
            DBG_ERR("idr_find for %u   failed!\n",delay_worker_id);
            goto END;
        }
        dump_buf_cmd_info->force_stop = TRUE;//set force stop flag
    }
    else
    {
        DBG_ERR("not support cmd: %s\n",tmp_pcBuf);
        DBG_ERR("current only support:    echo force_stop_dump  delay_worker_id > /proc/mi_modules/mi_dump_buffer_delay_worker\n");
        DBG_ERR("and you can get  delay_worker_id and other delay_worker information from cat /proc/mi_modules/mi_dump_buffer_delay_worker\n");
        goto END;
    }
END:
    return buf_size;
}

int mi_dump_buffer_delay_worker_info(struct seq_file *m, void *v)
{
    int id;
    struct queue_dump_buf_cmd_info *dump_buf_cmd_info;
    seq_printf(m,"%20s%15s%15s%10s%10s%15s%10s%15s%30s%15s%20s\n","delay_worker_id","module_name","force_stop","dev_id","chn_id","port_type","port_id",
                      "Queue_name","stored_dir","dump_method","dump_method_value");

    rcu_read_lock();
    idr_for_each_entry(&idr_delay_worker, dump_buf_cmd_info, id)
    {
        MI_SYS_BUG_ON(dump_buf_cmd_info->idr_num != id);
        seq_printf(m,"%20d%15s%15d%10u%10u%15s%10u%15s%30s",id,dump_buf_cmd_info->module_name
                             ,dump_buf_cmd_info->force_stop
                             ,dump_buf_cmd_info->u32DevId,dump_buf_cmd_info->u32ChnId
                             ,dump_buf_cmd_info->port_type == E_MI_MODULE_DUMP_BUFFER_INPUT_PORT?"iport":"oport"
                             ,dump_buf_cmd_info->port_id
                             ,(dump_buf_cmd_info->Queue_name == E_MI_MODULE_DUMP_BUFFER_Queue_UsrInject)?"UsrInject":
                             ((dump_buf_cmd_info->Queue_name == E_MI_MODULE_DUMP_BUFFER_Queue_BindInput)?"BindInput":"GetFifo")
                             ,dump_buf_cmd_info->path);
        if(dump_buf_cmd_info->bufnum != -1)
        {
            seq_printf(m,"%15s%20d\n","bufnum",dump_buf_cmd_info->bufnum);
        }
        else if(dump_buf_cmd_info->time != -1)
        {
            seq_printf(m,"%15s%20d\n","time",dump_buf_cmd_info->time);
        }
        else if(dump_buf_cmd_info->start_end != E_MI_MODULE_DUMP_BUFFER_INVALID)
        {
            seq_printf(m,"%15s%20s\n","start_end",dump_buf_cmd_info->start_end == E_MI_MODULE_DUMP_BUFFER_START?"start":"end");
        }
        else
        {
            MI_SYS_BUG();
        }

    }
    rcu_read_unlock();

    return MI_SUCCESS;
}

int mi_dump_buffer_delay_worker_open(struct inode *inode, struct file *file)
{
    return single_open(file, mi_dump_buffer_delay_worker_info, PDE_DATA(inode));
}

MI_S32 deal_with_argc7_end(MI_SYS_MOD_DEV_t *pstModDev, MI_U32  u32DevId,MI_U8 **argv)
{
    int idr_id;
    struct queue_dump_buf_cmd_info *dump_buf_cmd_info;
    struct queue_dump_buf_cmd_info *tmp_dump_buf_cmd_info;

    MI_SYS_BUG_ON(0 != strncmp("end",argv[6],strlen("end")));

    dump_buf_cmd_info = kzalloc(sizeof(struct queue_dump_buf_cmd_info),GFP_KERNEL);
    if(!dump_buf_cmd_info)
    {
        return E_MI_ERR_NOMEM;
    }

    dump_buf_cmd_info->pstModDev = pstModDev;

    dump_buf_cmd_info->u32DevId = u32DevId;

    str2val(argv[1],strlen(argv[1]),&dump_buf_cmd_info->u32ChnId);

    if(0 == strncmp("iport",argv[2],strlen("iport")))
        dump_buf_cmd_info->port_type = E_MI_MODULE_DUMP_BUFFER_INPUT_PORT;//input port or output port.
    else
        dump_buf_cmd_info->port_type = E_MI_MODULE_DUMP_BUFFER_OUTPUT_PORT;//

    str2val(argv[3],strlen(argv[3]),&dump_buf_cmd_info->port_id) ;

    if(0 == strncmp("UsrInject",argv[4],strlen("UsrInject")))
    {
        dump_buf_cmd_info->Queue_name = E_MI_MODULE_DUMP_BUFFER_Queue_UsrInject;
    }
    else if(0 == strncmp("BindInput",argv[4],strlen("BindInput")))
    {
        dump_buf_cmd_info->Queue_name = E_MI_MODULE_DUMP_BUFFER_Queue_BindInput;
    }
    else
    {
        dump_buf_cmd_info->Queue_name = E_MI_MODULE_DUMP_BUFFER_Queue_GetFifo;
    }
    strncpy(dump_buf_cmd_info->path,argv[5],strlen(argv[5]));
    dump_buf_cmd_info->path[strlen(argv[5])] = '\0';

    DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
    rcu_read_lock();
    idr_for_each_entry(&idr_delay_worker, tmp_dump_buf_cmd_info, idr_id)
    {
        DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
        MI_SYS_BUG_ON(tmp_dump_buf_cmd_info->idr_num != idr_id);
        if(E_MI_MODULE_DUMP_BUFFER_START == tmp_dump_buf_cmd_info->start_end)
        {
            DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
            if((dump_buf_cmd_info->pstModDev == tmp_dump_buf_cmd_info->pstModDev)
                &&(dump_buf_cmd_info->u32DevId == tmp_dump_buf_cmd_info->u32DevId)
                &&(dump_buf_cmd_info->u32ChnId == tmp_dump_buf_cmd_info->u32ChnId)
                &&(dump_buf_cmd_info->port_type == tmp_dump_buf_cmd_info->port_type)
                &&(dump_buf_cmd_info->port_id == tmp_dump_buf_cmd_info->port_id)
                &&(dump_buf_cmd_info->Queue_name == tmp_dump_buf_cmd_info->Queue_name)
                &&(0 == strncmp(dump_buf_cmd_info->path, tmp_dump_buf_cmd_info->path
                                ,max(strlen(dump_buf_cmd_info->path),strlen(tmp_dump_buf_cmd_info->path)))))
            {
                DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
                tmp_dump_buf_cmd_info->force_stop = TRUE;//change it into force stop.
                break;
            }
        }
        DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
    }
    rcu_read_unlock();
    DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);

    kfree(dump_buf_cmd_info);//finish use,kfree it .
    return MI_SUCCESS;
}

//echo dump_buffer channel_id port_type port_id Queue_name path stop_method > /proc/mi_modules/modulename/modulename_devid
//parameter1  channel_id: means channel id of this dev.
//parameter2  port_type: value is "iport" or "oport" .
//parameter3  port_id:
//parameter4  Queue_name:for input port ,only can be UsrInjectBufQueue or  BindInputBufQueue;for output port ,only support UsrGetFifoBufQueue.
//parameter5  path:result file is stored in which dir.
//parameter6  stop_method:"bufnum=xxx" ,or "time=xxx"(here unit is ms) or "start/stop" pair.
static MI_S32 _MI_SYS_IMPL_OnModExecDumpBuffer(MI_SYS_DEBUG_HANDLE_t handle, MI_U32  u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 ret;
    MI_U8 module_name[20];
    int idr_num;
    struct queue_dump_buf_cmd_info *dump_buf_cmd_info;
    struct seq_file* q;
    DRV_PROC_ITEM_S* pstProcItem;
    MI_SYS_MOD_DEV_t *pstModDev;
    struct queue_dump_buf_cmd_info *existed_dump_buf_cmd_info;
    int existed_id;

    q = (struct seq_file*)handle.file;
    MI_SYS_BUG_ON(!q);
    pstProcItem = (DRV_PROC_ITEM_S* )q->private;
    MI_SYS_BUG_ON(!pstProcItem);
    pstModDev = (MI_SYS_MOD_DEV_t *)pstProcItem->pstModDev;
    MI_SYS_BUG_ON(!pstModDev);
    //handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
    _MI_SYS_IMPL_ModuleIdToPrefixName(pstModDev->eModuleId,module_name);
    //handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);

    ret = _MI_SYS_IMPL_DumpBufferParCheck(handle,u32DevId,argc,argv,pstModDev);
    DBG_INFO("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
    if(ret != MI_SUCCESS)
    {
        DBG_ERR("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
        return ret;
    }

    if(0 == strncmp("end",argv[6],strlen("end")))
        return deal_with_argc7_end(pstModDev,u32DevId,argv);

    dump_buf_cmd_info = kzalloc(sizeof(struct queue_dump_buf_cmd_info),GFP_KERNEL);
    if(!dump_buf_cmd_info)
    {
        handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);
        return E_MI_ERR_NOMEM;
    }

    dump_buf_cmd_info->pstModDev = pstModDev;

    strncpy(dump_buf_cmd_info->module_name,module_name,strlen(module_name));
    dump_buf_cmd_info->module_name[strlen(module_name)] = '\0';
    dump_buf_cmd_info->u32DevId = u32DevId;

    str2val(argv[1],strlen(argv[1]),&dump_buf_cmd_info->u32ChnId);

    if(0 == strncmp("iport",argv[2],strlen("iport")))
        dump_buf_cmd_info->port_type = E_MI_MODULE_DUMP_BUFFER_INPUT_PORT;//input port or output port.
    else
        dump_buf_cmd_info->port_type = E_MI_MODULE_DUMP_BUFFER_OUTPUT_PORT;//

    str2val(argv[3],strlen(argv[3]),&dump_buf_cmd_info->port_id) ;

    if(0 == strncmp("UsrInject",argv[4],strlen("UsrInject")))
    {
        dump_buf_cmd_info->Queue_name = E_MI_MODULE_DUMP_BUFFER_Queue_UsrInject;
    }
    else if(0 == strncmp("BindInput",argv[4],strlen("BindInput")))
    {
        dump_buf_cmd_info->Queue_name = E_MI_MODULE_DUMP_BUFFER_Queue_BindInput;
    }
    else
    {
        dump_buf_cmd_info->Queue_name = E_MI_MODULE_DUMP_BUFFER_Queue_GetFifo;
    }
    strncpy(dump_buf_cmd_info->path,argv[5],strlen(argv[5]));
    dump_buf_cmd_info->path[strlen(argv[5])] = '\0';

    if(0 == strncmp("bufnum=",argv[6],strlen("bufnum=")))
    {
        str2val(argv[6]+strlen("bufnum="),strlen(argv[6]+strlen("bufnum=")),&dump_buf_cmd_info->bufnum);
        //handle.OnPrintOut(handle,"[debug][%s:%d] dump_buf_cmd_info->bufnum=%d\n",__FUNCTION__,__LINE__,dump_buf_cmd_info->bufnum);
    }
    else
    {
        dump_buf_cmd_info->bufnum = -1;//-1 means invalid
    }

    if(0 == strncmp("time=",argv[6],strlen("time=")))
    {
        str2val(argv[6]+strlen("time="),strlen(argv[6]+strlen("time=")),&dump_buf_cmd_info->time);
    }
    else
    {
        dump_buf_cmd_info->time = -1;//-1 means invalid
    }

    if(0 == strncmp("start",argv[6],strlen("start")))
    {
        dump_buf_cmd_info->start_end = E_MI_MODULE_DUMP_BUFFER_START;
    }
    else if(0 == strncmp("end",argv[6],strlen("end")))
    {
        dump_buf_cmd_info->start_end = E_MI_MODULE_DUMP_BUFFER_END;
    }
    else
        dump_buf_cmd_info->start_end = E_MI_MODULE_DUMP_BUFFER_INVALID;

    dump_buf_cmd_info->private = NULL;//other value that callback in work needed.

    //handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);

    //printk("%s dump_Queue_buf=%p\n",__FUNCTION__,dump_buf_cmd_info);

    //handle.OnPrintOut(handle,"[debug][%s:%d] dump_buf_cmd_info->bufnum=%d\n",__FUNCTION__,__LINE__,dump_buf_cmd_info->bufnum);


    rcu_read_lock();
    idr_for_each_entry(&idr_delay_worker, existed_dump_buf_cmd_info, existed_id)
    {
        MI_SYS_BUG_ON(existed_dump_buf_cmd_info->idr_num != existed_id);
        if((existed_dump_buf_cmd_info->pstModDev == dump_buf_cmd_info->pstModDev)
            &&(existed_dump_buf_cmd_info->u32DevId == dump_buf_cmd_info->u32DevId)
            &&(existed_dump_buf_cmd_info->u32ChnId == dump_buf_cmd_info->u32ChnId)
            &&(existed_dump_buf_cmd_info->port_type == dump_buf_cmd_info->port_type)
            &&(existed_dump_buf_cmd_info->port_id == dump_buf_cmd_info->port_id)
            &&(existed_dump_buf_cmd_info->Queue_name == dump_buf_cmd_info->Queue_name)
            )
        {
            DBG_ERR("Targeted Queue already have a current doing work,so skip this request!\n");
            kfree(dump_buf_cmd_info);//skip ,so kfree this
            rcu_read_unlock();
            return E_MI_ERR_ILLEGAL_PARAM;
        }
    }
    rcu_read_unlock();


    idr_num = idr_alloc(&idr_delay_worker,dump_buf_cmd_info,0,0x0FFFFFFF, GFP_KERNEL);
    if(idr_num < 0)
    {
        DBG_ERR("[debug][%s:%d]  \n",__FUNCTION__,__LINE__);
        kfree(dump_buf_cmd_info);//if fail,should kfree
        return E_MI_ERR_FAILED;
    }
    dump_buf_cmd_info->idr_num = idr_num;
    dump_buf_cmd_info->force_stop = FALSE;

    INIT_DELAYED_WORK(&dump_buf_cmd_info->queue_dump_buf_data_wq,
                  dump_Queue_buf_worker);
    DBG_WRN("queue_delayed_work --> \n");

    queue_delayed_work(mi_sys_debug_workqueue, &dump_buf_cmd_info->queue_dump_buf_data_wq, msecs_to_jiffies(3000));

    DBG_WRN("queue_delayed_work --< \n");

    //handle.OnPrintOut(handle,"%s:%d\n",__FUNCTION__,__LINE__);

    return MI_SUCCESS;
}


MI_S32 _MI_SYS_IMPL_Common_ReadProc(struct seq_file* q, void* v)
{
    DRV_PROC_ITEM_S* pstProcItem;
    MI_S32 ret;
    mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps;
    MI_SYS_DEBUG_HANDLE_t  handle;

    pstProcItem = q->private;
    pstModuleProcfsOps = (mi_sys_ModuleDevProcfsOps_t *)(&pstProcItem->Ops);
    #ifdef DEBUG_COMMON_Proc
    DBG_INFO("pstProcItem info %p  %p %p  %p %p\n",pstModuleProcfsOps
                    ,pstModuleProcfsOps->OnDumpDevAttr,pstModuleProcfsOps->OnDumpChannelAttr
                    ,pstModuleProcfsOps->OnDumpInputPortAttr,pstModuleProcfsOps->OnDumpOutPortAttr);
    #endif

    switch (pstProcItem->pstModDev->eModuleId)
    {
        case E_MI_MODULE_ID_VDF:
        case E_MI_MODULE_ID_GFX:
        case E_MI_MODULE_ID_HDMI:
        case E_MI_MODULE_ID_RGN:
        case E_MI_MODULE_ID_FB:
            break;//Not need dump common info
        default:
            {
                _mi_sys_dump_dev_common_info(q);

                _mi_sys_dump_chn_common_info(q);

                _mi_sys_dump_inputport_common_info(q);

                _mi_sys_dump_outputport_common_info(q);
            }
            break;
    }
    handle.file = (void *)q;
    handle.OnPrintOut = OnPrintOut_linux_seq_write;
    DBG_INFO("\n");

    ///TODO:
    if(pstModuleProcfsOps->OnDumpDevAttr)
    {
        ret = pstModuleProcfsOps->OnDumpDevAttr(handle,pstProcItem->pstModDev->u32DevId,pstProcItem->pstModDev->pUsrData);
        if(ret != MI_SUCCESS)
        {
            goto FAIL;
        }
    }
    else
    {
        //seq_printf(q,"%s not register OnDumpDevAttr!!!!\n",pstProcItem->entry_name);//default not print
    }

    if(pstModuleProcfsOps->OnDumpChannelAttr)
    {
        ret = pstModuleProcfsOps->OnDumpChannelAttr(handle,pstProcItem->pstModDev->u32DevId,pstProcItem->pstModDev->pUsrData);
        if(ret != MI_SUCCESS)
        {
            goto FAIL;
        }
    }
    else
    {
        //seq_printf(q,"%s not register OnDumpChannelAttr!!!!\n",pstProcItem->entry_name);//default not print
    }

    if(pstModuleProcfsOps->OnDumpInputPortAttr)
    {
        ret = pstModuleProcfsOps->OnDumpInputPortAttr(handle,pstProcItem->pstModDev->u32DevId,pstProcItem->pstModDev->pUsrData);
                    if(ret != MI_SUCCESS)
                    {
                        goto FAIL;
                    }
    }
    else
    {
        //seq_printf(q,"%s not register OnDumpInputPortAttr!!!!\n",pstProcItem->entry_name);//default not print
    }

    if(pstModuleProcfsOps->OnDumpOutPortAttr)
    {
        ret = pstModuleProcfsOps->OnDumpOutPortAttr(handle,pstProcItem->pstModDev->u32DevId,pstProcItem->pstModDev->pUsrData);
        if(ret != MI_SUCCESS)
        {
            goto FAIL;
        }
    }
    else
    {
        //seq_printf(q,"%s not register OnDumpChannelAttr!!!!\n",pstProcItem->entry_name);//default not print
    }

    return MI_SUCCESS;

FAIL:
    _MI_SYS_IMPL_Common_Help(q);
    if(pstModuleProcfsOps->OnHelp)
    {
        pstModuleProcfsOps->OnHelp(handle,pstProcItem->pstModDev->u32DevId,pstProcItem->pstModDev->pUsrData);
    }
    seq_printf(q,"debug code in %s:%d  ret=0x%x\n",__FUNCTION__,__LINE__,ret);
    return ret;
}

MI_S32 _MI_SYS_IMPL_Common_WriteProc(struct file* file, const char __user* buf, size_t count, loff_t* ppos)
{

    struct seq_file* q = file->private_data;
    DRV_PROC_ITEM_S* pstProcItem = q->private;
    mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps;
    MI_SYS_DEBUG_HANDLE_t  handle;
    MI_U8 u8StringCount;
    DRV_PROC_EXEC_CMD_t stExecCmd;
    MI_U8 *pu8Argv[20];
    MI_U8 aszBuf[16];
    MI_U8 prefix_name[10];
    MI_U8 szBuf[256] = {0};

    pstModuleProcfsOps = (mi_sys_ModuleDevProcfsOps_t *)(&pstProcItem->Ops);
    #ifdef DEBUG_COMMON_Proc
    printk("pstProcItem info %p  %p %p	%p %p\n",pstModuleProcfsOps
                  ,pstModuleProcfsOps->OnDumpDevAttr,pstModuleProcfsOps->OnDumpChannelAttr
                  ,pstModuleProcfsOps->OnDumpInputPortAttr,pstModuleProcfsOps->OnDumpOutPortAttr);
    #endif

    //DBG_INFO("\n");
    printk("printk in %s\n",__FUNCTION__);

    handle.file = (void *)q;
    handle.OnPrintOut = OnPrintOut_linux_vprintk;

    if (copy_from_user(szBuf, buf, count < sizeof(szBuf) - 1 ? count : sizeof(szBuf) - 1))
    {
        //DBG_INFO("\n");
        printk("printk in %s:%d\n",__FUNCTION__,__LINE__);
        return E_MI_ERR_FAILED;
    }
    u8StringCount  =_MI_SYS_IMPL_ParseStringsStart(szBuf, pu8Argv);
    if (u8StringCount)
    {
        memset(&stExecCmd, 0, sizeof(DRV_PROC_EXEC_CMD_t));
        if(procIntfParam)
        {
            if(procIntfParam->findDevCmdfun)
            {
                memset(aszBuf, 0, sizeof(aszBuf));
                memset(prefix_name, 0, sizeof(prefix_name));
                _MI_SYS_IMPL_ModuleIdToPrefixName(pstProcItem->pstModDev->eModuleId,(char *)prefix_name);
                snprintf(aszBuf, sizeof(aszBuf), "%s%d",prefix_name,pstProcItem->pstModDev->u32DevId);
                procIntfParam->findDevCmdfun(pu8Argv[0], aszBuf, &stExecCmd);
                if (stExecCmd.fpExecCmd)
                {
                    if (u8StringCount - 1 != stExecCmd.u8MaxPara)
                    {
                        DBG_ERR("Command %s para count is not matched!\n", stExecCmd.pu8CmdName);
                    }
                    else
                    {
                        if (MI_SUCCESS != stExecCmd.fpExecCmd(handle, pstProcItem->pstModDev->u32DevId, u8StringCount, pu8Argv, pstProcItem->pstModDev->pUsrData))
                        {
                            goto SAVE_CMD_FAULT;
                        }
                    }
                }
                else
                {
                    DBG_ERR("Not parsed command %s\n", pu8Argv[0]);
                }
            }
        }

    }
    _MI_SYS_IMPL_ParseStringsEnd(u8StringCount, pu8Argv);

    return count;

SAVE_CMD_FAULT:
    //DBG_INFO("\n");
    _MI_SYS_IMPL_Common_Help(q);
    if(pstModuleProcfsOps->OnHelp)
    {
        pstModuleProcfsOps->OnHelp(handle,pstProcItem->pstModDev->u32DevId,pstProcItem->pstModDev->pUsrData);
    }
    printk("printk in %s:%d\n",__FUNCTION__,__LINE__);
    return E_MI_ERR_FAILED;
}

COMMON_PROC_PARAM_S s_stCommonProc =
{
    .pfnReadProc      = _MI_SYS_IMPL_Common_ReadProc,
    .pfnWriteProc      = _MI_SYS_IMPL_Common_WriteProc,
};

void _MI_SYS_IMPL_Common_RegProc(MI_SYS_MOD_DEV_t *pstModDev,mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps,struct proc_dir_entry *proc_dir_entry)
{
    u8 aszBuf[16];
    u8 prefix_name[10];
    DRV_PROC_EX_S pfnOpt;
    DRV_PROC_ITEM_S* pProcItem = NULL;
    #ifdef DEBUG_COMMON_Proc
    mi_sys_ModuleDevProcfsOps_t * tmp_pstModuleProcfsOps;
    #endif

    //allow pstModuleProcfsOps be NULL,but not allow pstModDev be NULL
    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON( pstModDev->eModuleId >= E_MI_MODULE_ID_MAX);

    memset(aszBuf, 0, sizeof(aszBuf));
    memset(prefix_name, 0, sizeof(prefix_name));
    _MI_SYS_IMPL_ModuleIdToPrefixName(pstModDev->eModuleId,(char *)prefix_name);

    /* Create proc */
    snprintf(aszBuf, sizeof(aszBuf), "%s%d",prefix_name,pstModDev->u32DevId);
    DBG_INFO("pstModuleProcfsOps=%p\n",pstModuleProcfsOps);
    /* Set functions */

    pfnOpt.fnRead = s_stCommonProc.pfnReadProc;
    pfnOpt.fnWrite= s_stCommonProc.pfnWriteProc;
    pProcItem = _MI_SYS_IMPL_Drv_Proc_AddDev(aszBuf, &pfnOpt ,pstModuleProcfsOps,proc_dir_entry);
    if (!pProcItem)
    {
        printk("%s:%d  \n",__FUNCTION__,__LINE__);
        return;
    }
    pProcItem->pstModDev = pstModDev;
    _MI_SYS_IMPL_CommonRegCmd("help", 0, _MI_SYS_IMPL_OnModExecHelp, pstModDev);
    _MI_SYS_IMPL_CommonRegCmd("dump_buffer", PROC_DUMP_BUFFER_COMMAND_PARAMETER_NUM, _MI_SYS_IMPL_OnModExecDumpBuffer, pstModDev);

    if(pProcItem)
    {
        #ifdef DEBUG_COMMON_Proc
        printk("before pstModuleProcfsOps  %p  %p %p %p %p\n",pstModuleProcfsOps
                                                                ,pstModuleProcfsOps->OnDumpDevAttr,pstModuleProcfsOps->OnDumpChannelAttr
                                                                ,pstModuleProcfsOps->OnDumpInputPortAttr,pstModuleProcfsOps->OnDumpOutPortAttr);
        tmp_pstModuleProcfsOps = (mi_sys_ModuleDevProcfsOps_t *)(&pProcItem->data);
        printk("%s:%d  will check pstModuleProcfsOps\n",__FUNCTION__,__LINE__);
        printk("%s:%d  in check pstModuleProcfsOps\n",__FUNCTION__,__LINE__);
        printk("after pstModuleProcfsOps	%p	%p %p %p %p\n",pstModuleProcfsOps
                                                                ,pstModuleProcfsOps->OnDumpDevAttr,pstModuleProcfsOps->OnDumpChannelAttr
                                                                ,pstModuleProcfsOps->OnDumpInputPortAttr,pstModuleProcfsOps->OnDumpOutPortAttr);
        printk("after tmp_pstModuleProcfsOps	%p	%p %p %p %p\n",tmp_pstModuleProcfsOps
                                                                ,tmp_pstModuleProcfsOps->OnDumpDevAttr,tmp_pstModuleProcfsOps->OnDumpChannelAttr
                                                                ,tmp_pstModuleProcfsOps->OnDumpInputPortAttr,tmp_pstModuleProcfsOps->OnDumpOutPortAttr);

        if(tmp_pstModuleProcfsOps->OnDumpDevAttr)
        {
            tmp_pstModuleProcfsOps->OnDumpDevAttr(pstModDev->u32DevId);
        }
        else
        {
            printk("%s not register  OnDumpDevAttr\n",(char *)aszBuf);
        }
        #endif
    }
    return;
}

void _MI_SYS_IMPL_Common_UnRegProc(MI_SYS_MOD_DEV_t *pstModDev)
{
    MI_U8 aszBuf[16];
    u8 prefix_name[10];
    MI_SYS_BUG_ON(!pstModDev);
    MI_SYS_BUG_ON( pstModDev->eModuleId >= E_MI_MODULE_ID_MAX);

    memset(aszBuf, 0, sizeof(aszBuf));
    memset(prefix_name, 0, sizeof(prefix_name));
    _MI_SYS_IMPL_ModuleIdToPrefixName(pstModDev->eModuleId,(char *)prefix_name);
    snprintf(aszBuf, sizeof(aszBuf), "%s%d",prefix_name,pstModDev->u32DevId);
    _MI_SYS_IMPL_Drv_Proc_RemoveDev(aszBuf);
}

MI_S32 _MI_SYS_IMPL_Allocator_ReadProc(struct seq_file* q, void* v)
{
    ALLOCATOR_PROC_ITEM_S *item = q->private;
    MI_SYS_DEBUG_HANDLE_t  handle;
    MI_SYS_BUG_ON(item->read != _MI_SYS_IMPL_Allocator_ReadProc);
    handle.file = (void *)q;
    handle.OnPrintOut = OnPrintOut_linux_seq_write;
    if(item->Ops.OnDumpAllocatorAttr)
    {
        item->Ops.OnDumpAllocatorAttr(handle,item->allocator_private_data);
    }
    else
    {
        MI_SYS_BUG_ON(1);
    }

    return MI_SUCCESS;
}

void Allocator_WriteProc_Help(MI_SYS_DEBUG_HANDLE_t  handle)
{
    handle.OnPrintOut(handle,"Allocator echo command only support help command and the below dump data command:\n");
    handle.OnPrintOut(handle,"  echo path offset length > /proc/mi_modules/mi_modulename/mi_modulenamedeviceid  \n");
    handle.OnPrintOut(handle,"       path is absolute path,not include file name.\n");
    handle.OnPrintOut(handle,"            offset is logical offset in this allocator that dump data start.\n");
    handle.OnPrintOut(handle,"                   length means how much data will be dumpped.\n");
    handle.OnPrintOut(handle,"       modulename like vpe,disp,divp ; deviceid like 0,1,2 .\n");
}
MI_S32 _MI_SYS_IMPL_Allocator_WriteProc(struct file* file, const char __user* user_buf, size_t count, loff_t* ppos)
{
    char szBuf[250];
    MI_U8* pcBuf = szBuf;
    size_t buf_copied_size = -1;//default value not be 0,because return 0 will cause loop.
    struct seq_file* q = file->private_data;
    ALLOCATOR_PROC_ITEM_S* item = q->private;
    MI_SYS_DEBUG_HANDLE_t  handle;

    MI_SYS_BUG_ON(item->write != _MI_SYS_IMPL_Allocator_WriteProc);
    buf_copied_size = min(count, (sizeof(szBuf)-1));
    if(copy_from_user(szBuf, user_buf, buf_copied_size))
    {
        DBG_ERR("%s :%d \n",__FUNCTION__,__LINE__);
        return -EFAULT;
    }

    COMMON_STRING_SKIP_BLANK(pcBuf);

    handle.file = (void *)q;
    handle.OnPrintOut = OnPrintOut_linux_vprintk;
    if(item->Ops.OnAllocatorExecCmd)
    {
        MI_U8* tmp_pcBuf = pcBuf;
        MI_S32 ret;
        MI_U8 *tmp_buf_2;
        MI_U32 tmp_buf_2_len;
        allocator_echo_cmd_info_t *cmd_info;

        cmd_info = (allocator_echo_cmd_info_t *)kmalloc(sizeof(allocator_echo_cmd_info_t),GFP_KERNEL);
        if(!cmd_info)
        {
            printk("%s:%d  kmalloc fail\n",__FUNCTION__,__LINE__);
            return E_MI_ERR_FAILED;
        }
        DBG_INFO("tmp_pcBuf  is %s\n",tmp_pcBuf);
        COMMON_STRING_SKIP_BLANK(tmp_pcBuf);

        if(0 == strncmp(tmp_pcBuf,"help",strlen("help")))
        {
            Allocator_WriteProc_Help(handle);
            return buf_copied_size;
        }

        DBG_INFO("curr tmp_pcBuf  is %s\n",tmp_pcBuf);
        cmd_info->dir_size = 0;
        while (tmp_pcBuf[0] != ' ' && tmp_pcBuf[0] != '\0')
        {
            (tmp_pcBuf)++;
            cmd_info->dir_size++;
        }

        if(0 == cmd_info->dir_size)
        {
            printk("dir is invalid! you can \"echo help >\" for help \n");
            kfree(cmd_info);
            goto EXIT;
        }
        memcpy(cmd_info->dir_name ,pcBuf,cmd_info->dir_size);//get dir name.
        DBG_INFO("dir_name  is %s\n",cmd_info->dir_name);

        COMMON_STRING_SKIP_BLANK(tmp_pcBuf);
        DBG_INFO("curr tmp_pcBuf  is %s\n",tmp_pcBuf);
        tmp_buf_2 = tmp_pcBuf;
        tmp_buf_2_len = 0;
        while (tmp_pcBuf[0] != ' ' && tmp_pcBuf[0] != '\0')
        {
            (tmp_pcBuf)++;
            tmp_buf_2_len++;
        }
        if(0 == tmp_buf_2_len)
        {
            printk("offset is invalid  you can \"echo help >\" for help \n");
            kfree(cmd_info);
            goto EXIT;
        }
        ret = str2val(tmp_buf_2,tmp_buf_2_len, &cmd_info->offset);
        if(ret != 0)
        {
            printk("get offset fail  you can \"echo help >\" for help \n");
            kfree(cmd_info);
            goto EXIT;
        }
        DBG_INFO("offset is %u\n",cmd_info->offset);
        COMMON_STRING_SKIP_NON_BLANK(tmp_pcBuf);
        COMMON_STRING_SKIP_BLANK(tmp_pcBuf);
        DBG_INFO("curr tmp_pcBuf  is %s\n",tmp_pcBuf);
        tmp_buf_2 = tmp_pcBuf;
        tmp_buf_2_len = 0;
        while (tmp_pcBuf[0] != ' ' && tmp_pcBuf[0] != '\0')
        {
            (tmp_pcBuf)++;
            tmp_buf_2_len++;
        }
        if(0 == tmp_buf_2_len)
        {
            printk("length is invalid   you can \"echo help >\" for help \n");
            kfree(cmd_info);
            goto EXIT;
        }
        ret = str2val(tmp_buf_2,tmp_buf_2_len, &cmd_info->length);
        if(ret != 0)
        {
            printk("get length fail   you can \"echo help >\" for help \n");
            kfree(cmd_info);
            goto EXIT;
        }
        DBG_INFO("length is %u\n",cmd_info->length);
        item->Ops.OnAllocatorExecCmd(handle,cmd_info,item->allocator_private_data);
        kfree(cmd_info);
    }
    else
    {
        MI_SYS_BUG_ON(1);
    }

EXIT:

    return buf_copied_size;
}


COMMON_PROC_PARAM_S s_stAllocatorProc =
{
    .pfnReadProc      = _MI_SYS_IMPL_Allocator_ReadProc,
    .pfnWriteProc      = _MI_SYS_IMPL_Allocator_WriteProc,

};


#endif
