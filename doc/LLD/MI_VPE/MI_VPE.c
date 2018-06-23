DECLARE_WAIT_QUEUE_HEAD(vpe_isr_waitqueue);
LIST_HEAD(VPE_todo_task_list);
LIST_HEAD(VPE_working_task_list);
DECLARE_MUTEX(VPE_working_list_sem);

static inline is_fence_LE(MI_U16 fence1, MI_U16 fence2)
{
   if(fence1<=fence2)
       return 1;
   if(fence2+(0xFFFF-fence1) <0x7FFFF)
        return 1;
   return 0;
}

void VPE_ISR_Proc_Thread(void *data)
{
    cmd_mload_interface *cmdinf = (cmd_mload_interface*)data;

     while(1)
     {
          interruptible_sleep_on_timeout(&vpe_isr_queue, 10);

       while(!list_empty(&VPE_working_task_list));
          {
               mi_sys_ChnTaskInfo_t *pstChnTask;
         int loop_cnt = 0;
            pstChnTask = container_of(&VPE_working_task_list.next., MI_SYS_ChnTaskInfo_t, cur_list);

        if(!is_fence_LE(READ_REG(dump_reg_VPE_FENCE), pstChnTask->u32Reserved0))
        {
             if(!cmdinf->is_cmdq_empty_idle(cmdinf->ctx))
             {
                   up(&VPE_working_list_sem);
                  break;
             }
            if(!is_fence_LE(READ_REG(dump_reg_VPE_FENCE), pstChnTask->u32Reserved0))
            {
                 dmsg("invalid fence %04x, %04x!\n", READ_REG(dump_reg_VPE_FENCE), pstChnTask->u32Reserved0);
                 BUG();
             }
        }
        
        down(&VPE_working_list_sem);
        list_del(&pstChnTask->cur_list);
        up(&VPE_working_list_sem);

        cmdinf->update_mload_ringbuf_read_ptr(cmdinf, pstChnTask->u64Reserved0);

        if(XC_HAL_CHECK_TASK_SUCCESS(pstChnTask))
        {
             mi_sys_FinishTaskBuf(pstChnTask);
        }
        else
        {
             mi_sys_CancelTaskBuf(pstChnTask);
        }
           }   
     }
   
}

void VPE_ISR(void *data)
{
    wake_up(&vpe_isr_queue);
}


typedef struct 
{
   int totalAddedTask;
}VPE_Iterator_WorkInfo;

MI_TaskIteratorCBAction MI_VPE_TaskIteratorCallBK(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData)
{
    int i;
    int valid_output_port_cnt = 0;
    VPE_Iterator_WorkInfo *workInfo = (VPE_Iterator_WorkInfo *)pUsrData;


     if(mi_sys_PrepareTaskOutputBuf(pstTaskInfo) != MI_SUCCESS)
         return MI_SYS_ITERATOR_SKIP_CONTINUTE;

     for( i=0;i<pstTaskInfo->u32OutputPortNum; i++)
     {
        BUG_ON(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i] && pstTaskInfo->astOutputPortBufInfo[i]);

        if(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i])
            valid_output_port_cnt++;
         else if(pstTaskInfo->astOutputPortBufInfo[i])
             valid_output_port_cnt++;
     }

    //check if leak of output buf
     if(pstTaskInfo->u32OutputPortNum && valid_output_port_cnt==0)
         return MI_SYS_ITERATOR_SKIP_CONTINUTE;


     list_add_tail(&pstTaskInfo->cur_list,&VPE_todo_task_list);
     
     //we at most process 32 batches at one time
     if(++workInfo->totalAddedTask >= 32)
         return MI_SYS_ITERATOR_ACCEPT_STOP;
     else
          return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
}

void MI_VPE_SortHandleChnListOrder(LIST_HEAD list)
{
    ///sort handle channel  order
}


MI_VPE_Process_TASK(MI_SYS_ChnTaskInfo_t *pstTask, cmd_menuload_interface *cmdinf, MS_U16 fence)
{
      ISP_process(pstTask, cmdinf);
      isp_XC_process(pstTask, cmdinf);
      MDWIN_process(pstTask, cmdinf);
      OSD_process(pstTask,cmdinf);
      cmdinf->cmdq_add_wait_event_cmd(cmdinf, CMDQ_EVENT_ISP_SC_ISR);
}
void VPEWorkThread(void *data)
{

    MS_U16 fence = 0;
    cmd_mload_interface *cmdinf = (cmd_mload_interface*)data;// get_sys_cmdq_service(CMDQ_ID_VPE);
    ///have cmd queue
    while(1)
    {

         VPE_Iterator_WorkInfo workinfo;
      list_head pos,n;
         workinfo.totalAddedTask = 0;

      mi_sys_DevTaskIterator(E_MI_SYS_MODULE_ID_VPE, 0, MI_VPE_TaskIteratorCallBK, &workinfo);
      if(list_empty(&VPE_todo_task_list))
      {
            schedule();
            mi_sys_WaitOnInputTaskAvailable(E_MI_SYS_MODULE_ID_VPE, 0, 100);
            continue;        
       }
    
          list_for_each_entry_safe(&pos, &n, &VPE_todo_task_list)
          {
               mi_sys_ChnTaskInfo_t *pstChnTask;
         MI_PHY menuload_buf_ring_beg_addr;
         MI_PHY menuload_buf_ring_end_addr;
         int loop_cnt = 0;
         
            pstChnTask = container_of(&pos, mi_sys_ChnTaskInfo_t, cur_list);

               while(!cmdinf->ensure_cmdbuf_available(cmdinf, 0x400, 0x1000))
               {
                   interruptible_sleep_on_timeout(&vpe_isr_queue, 2);
                     loop_cnt++;
                    if(loop_cnt>1000)
                    BUG();//engine hang
               }
        
            MI_VPE_Process_TASK(pstChnTask, cmdinf);
         menuload_buf_ring_end_addr = cmdinf->get_menuload_ringbuf_write_ptr(cmdinf);
         cmdinf->write_reg_cmdq_mask(cmdinf,dump_reg_VPE_FENCE, fence, 0x0);
         cmdinf->kick_off_cmdq(cmdinf);
            pstChnTask->u32Reserved0 = fence++;
         pstChnTask->u64Reserved0 =   cmdinf->get_next_mload_ringbuf_write_ptr(cmdinf, 0x1000);;
         list_del(pstChnTask->cur_list);
         
         down(&VPE_working_list_sem);
         list_add_tail(&pstChnTask->cur_list, &VPE_working_task_list);
         up(&VPE_working_list_sem);
        
           }
        }
           
}

MI_RESULT VPEOnBindChnOutputCallback(MI_SYS_ChnInputPortCfgInfo_t *pstChnPortCfgInfo)
{
    if (UNSUPPORT(pstChnPortCfgInfo->eFormat) || UNSUPPORT(pstChnPortCfgInfo->u32Stride * pstChnPortCfgInfo->u32Height) || UNSUPPORT(xxxxx))
    {
        printf("can't binder cannel, capacity limited, unspport xxxx\n");
        return MI_ERR_FAILED;
    }

    return MI_OK;
}

MI_RESULT VPEOnUnBindChnOutputCallback(void)
{
    if (PORT_BUSY)
    {
        printf("unbind fail\n");
        return MI_ERR_FAILED;
    }

    return MI_OK;
}

void MI_VPE_Init()
{
    mi_sys_ModuleBindOps_t stVPEPOps;
    stVPEPOps.OnBindChn = VPEOnBindChnOutputCallback;
    stVPEPOps.OnUnBindChn = VPEOnUnBindChnOutputCallback;
    mi_sys_ModuleInfo_t stModInfo;
    memset(&stModInfo, 0x0, sizeof(MI_SYS_ModuleInfo_t));
    stModInfo.u32ModuleId = E_MI_SYS_MODULE_ID_VPE;
    stModInfo.u32DevId = 0;
    stModInfo.u32DevChnNum = 64;
    stModInfo.u32InputPortNum = 1;
    stModInfo.u32OutputPortNum = 4;
    
    mi_sys_RegisterDev(&stModInfo, &stVPEPOps);
    
     cmd_mload_interface *cmdinf = get_sys_cmdq_service(CMDQ_ID_VPE, MI_TRUE);

     kthread_create(VPEWorkThread, cmdinf);
     kthread_create(VPE_ISR_Proc_Thread, cmdinf);
     request_irq(VPE_ISR_IDX, VPE_ISR, NULL);
}

