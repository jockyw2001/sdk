void Disp_Dev_ISR_Thread(void *data)
{
     MI_DISP_DEV_Param_t *pstDisp_Dev_Param = (MI_DISP_DEV_Param_t*)data;

      MS_U16 last_read_index[VIF_PORT_NUM];

      memset(last_read_index, 0, sizeof(last_read_index));
      
     
      int i;
      
     while(1)
     {
          interruptible_sleep_on_timeout(&pstDisp_Dev_Param->stWaitQueueHead, 10);
          if(!pstDisp_Dev_Param->bDISPEnabled)
               continue;

           down(&pstDisp_Dev_Param->stBindedVideoLayer[i]->stDispLayerPendingQueueMutex);
          
       for( i=0;i<MI_DISP_VIDEO_LAYER_MAX;i++)
       {
           int j;
           if(!pstDisp_Dev_Param->stBindedVideoLayer[i])
               continue;
              if(!DISP_MGWIN_HAL_DoubleBuffer_Fired(i))
                     continue;
              
              for(j=0;j<MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX;j++)
        {
            if(pstDisp_Dev_Param->stBindedVideoLayer[i]->pstCurrentFiredBufInfo[i])
            {
                if(pstDisp_Dev_Param->stBindedVideoLayer[i]->pstOnScreenBufInfo[j] != pstDisp_Dev_Param->pstCurrentFiredBufInfo[i])
                {
                            mi_sys_FinishBuf(pstDisp_Dev_Param->stBindedVideoLayer[i]->pstOnScreenBufInfo[j]);
                            pstDisp_Dev_Param->stBindedVideoLayer[i]->pstOnScreenBufInfo[j] = pstDisp_Dev_Param->pstCurrentFiredBufInfo[i];
                }
                       pstDisp_Dev_Param->pstCurrentFiredBufInfo[i] = NULL;
            }
                  if(!pstDisp_Dev_Param->stBindedVideoLayer[i]->bPortEnabled[j] && pstDisp_Dev_Param->stBindedVideoLayer[i]->pstOnScreenBufInfo[j])
                  {
                        mi_sys_FinishBuf(pstDisp_Dev_Param->stBindedVideoLayer[i]->pstOnScreenBufInfo[j]);
                        pstDisp_Dev_Param->stBindedVideoLayer[i]->pstOnScreenBufInfo[j] = NULL;
                  }

            while(!list_empty(&pstDisp_Dev_Param->stBindedVideoLayer[i]->port_pending_buf_queue));
            {
                 list_head *next_bufinfo;
                       MI_SYS_BufInfo_t *nextBufInfo;
                    down(&pstDisp_Dev_Param->stBindedVideoLayer[i]->stDispLayerPendingQueueMutex);
                       next_bufinfo = pstDisp_Dev_Param->stBindedVideoLayer[i]->port_pending_buf_queue.next;
                       list_del(next_bufinfo);
                       up(&pstDisp_Dev_Param->stBindedVideoLayer[i]->stDispLayerPendingQueueMutex);
                       nextBufInfo = container_of(nextBufInfo, MI_SYS_BufInfo_t, list);
                       if(MI_DISP_SuitableDispWin(nextBufInfo))
                        {
                              MI_DISP_DoubleBufFlipDispWin(nextBufInfo);
                              break;
                       }
                       mi_sys_FinishBuf(nextBufInfo);                          
             }
        }

       }
         }
   
}
void Disp_Dev_Work_Thread(void *data)
{

     MI_DISP_DEV_Param_t *pstDisp_Dev_Param = (MI_DISP_DEV_Param_t*)data;
      int i;
      
     while(1)
     {
      
         if(!pstDisp_Dev_Param->bDISPEnabled)
         {
                schedule_timeout_interruptible(10);
              continue;
         }

         mi_sys_WaitOnInputTaskAvailable(E_MI_SYS_MODULE_ID_DISP, pstDisp_Dev_Param->u32DevId, 100);
         
       for( i=0;i<MI_DISP_VIDEO_LAYER_MAX;i++)
       {
           int j;

              for(j=0;j<MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX;j++)
        {
            while(1)
            {
                MI_SYS_ChnPort_t stChnPort;
                memset(&stChnPort , 0 , sizeof(MI_SYS_ChnPort_t));
                stChnPort.eModId = E_MI_SYS_MODULE_ID_DISP;
                stChnPort.s32DevId = 0;
                stChnPort.s32PortId = 1;
                 MI_SYS_BufInfo_t *bufinfo = mi_sys_GetInputPortBuf(&stChnPort);
                 if(!bufinfo)
                   break;
                       if(!pstDisp_Dev_Param->stBindedVideoLayer[i])
                       {
                          //add log err cnt into status info
                          mi_sys_FinishBuf(bufinfo);
                          continue;
                       }
                       down(&pstDisp_Dev_Param->stBindedVideoLayer[i]->stDispLayerPendingQueueMutex);
                       list_add_tail(&bufinfo->list, pstDisp_Dev_Param->stBindedVideoLayer[i]->port_pending_buf_queue); 
                       up(&pstDisp_Dev_Param->stBindedVideoLayer[i]->stDispLayerPendingQueueMutex);
             }
        }

       }
          schedule();
         }
   
}
void MI_DISP_ISR(void *data)
{

    MI_DISP_DEV_Param_t *pstDisp_Dev_Param = (MI_DISP_DEV_Param_t*)data;

    if(pstDisp_Dev_Param->bDISPEnabled)
        wake_up(&pstDisp_Dev_Param->stWaitQueueHead);
};

void MI_DISP_Init()
{
    mi_sys_ModuleBindOps_t stDISPOps;
    stDISPOps.OnBindInputPort = DISPOnBindChnInputCallback;
    stDISPOps.OnBindOutputPort = DISPOnBindChnOutputCallback;
    stDISPOps.OnUnBindInputPort = DISPOnUnBindChnInputCallback;
    stDISPOps.OnUnBindOutputPort = DISPOnUnBindChnOutputCallback;
    mi_sys_ModuleInfo_t stModInfo;
    int i;

    MI_DISP_DEV_Param_t *pstDevice0Param, *pstDevice1Param;

    memset(&stModInfo, 0x0, sizeof(mi_sys_ModuleInfo_t));
    stModInfo.u32ModuleId = E_MI_SYS_MODULE_ID_VDEC;
    stModInfo.u32DevId = 0;
    stModInfo.u32DevChnNum = 1;
    stModInfo.u32InputPortNum = MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX*MI_DISP_VIDEO_LAYER_MAX;
    stModInfo.u32OutputPortNum = 0;

     stModInfo.u32DevId = 1;
     mi_sys_RegisterDev(&stModInfo, &stDISPOps);

     pstDevice0Param = kmalloc(sizeof(MI_DISP_DEV_Param_t));
     BUG_ON(pstDevice0Param == NULL);


     pstDevice1Param = kmalloc(sizeof(MI_DISP_DEV_Param_t));
     BUG_ON(pstDevice1Param == NULL);

     memset(pstDevice0Param, 0, sizeof(MI_DISP_DEV_Param_t));
     memset(pstDevice1Param, 0, sizeof(MI_DISP_DEV_Param_t));
      
     INIT_WAIT_QUEUE_HEAD(&pstDevice0Param->pstWaitQueueHead);
     pstDevice0Param->u32DevId = 0;
     kthread_create(Disp_Dev_Work_Thread, pstDevice0Param);
     kthread_create(Disp_Dev_ISR_Thread, pstDevice0Param);
     
     INIT_WAIT_QUEUE_HEAD(&pstDevice1Param->pstWaitQueueHead);
     pstDevice0Param->u32DevId = 1;
          kthread_create(Disp_Dev_Work_Thread, pstDevice0Param);
     kthread_create(Disp_Dev_ISR_Thread, pstDevice1Param);
     
     request_irq(DISP_DEV0_ISR_IDX, MI_DISP_ISR, pstDevice0Param);
     request_irq(DISP_DEV1_ISR_IDX, MI_DISP_ISR, pstDevice1Param);

}
