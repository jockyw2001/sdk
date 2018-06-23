

/*
input:
pstBufAllocation:
pstBufConfig:
output:
bufinfo:
*/
/*
this function will be used by user,not be used by other internal VB POOL functions.
*/
void mi_sys_buf_mgr_fill_bufinfo(MI_SYS_BufferAllocation_t *pstBufAllocation,MI_SYS_BufConf_t *pstBufConfig, MI_SYS_BufInfo_t *bufinfo)
{

    bufinfo->eBufType = pstBufConfig->eBufType;
    bufinfo->u64Pts = pstBufConfig->u64TargetPts;
    bufinfo->bEndOfStream = false;//to be discuss
    bufinfo->bUserBuf = false;//to be discuss

    if(E_MI_SYS_BUFDATA_RAW == bufinfo->eBufType)
    {
        bufinfo->stRawData.u32BufSize = pstBufConfig->stRawCfg.u32Size;
        bufinfo->stRawData.u64PhyAddr = pstBufAllocation->u64PhyAddr;
        //to be discuss:how about other parameters?
    }
    else if(E_MI_SYS_BUFDATA_FRAME == bufinfo->eBufType)
    {
        bufinfo->stFrameData.eTileMode =   E_MI_SYS_FRAME_TILE_MODE_NONE;//set a default value 
        bufinfo->stFrameData.ePixelFormat = pstBufConfig->stFrameCfg.eFormat;
        bufinfo->stFrameData.eCompressMode =   E_MI_SYS_COMPRESS_MODE_NONE;//set a default value 
        bufinfo->stFrameData.eFrameScanMode = pstBufConfig->stFrameCfg.eFrameScanMode;
        bufinfo->stFrameData.eFieldType  =  E_MI_SYS_FIELDTYPE_TOP;//set a default value 
        bufinfo->stFrameData.stWindowRect.u16X = 0;
        bufinfo->stFrameData.stWindowRect.u16Y = 0;
        bufinfo->stFrameData.stWindowRect.u16Width = pstBufConfig->stFrameCfg.u16Width;
        bufinfo->stFrameData.stWindowRect.u16Height = pstBufConfig->stFrameCfg.u16Height;
        bufinfo->stFrameData.u16Width = pstBufConfig->stFrameCfg.u16Width;
        bufinfo->stFrameData.u16Height = pstBufConfig->stFrameCfg.u16Height;
        
        //to be discuss:how to???
        /*
            void* pVirAddr[3];
    MI_PHY u64PhyAddr[3];
    MI_U32 u32Stride[3];
    
        */
        //bufinfo->stFrameData.pVirAddr
        //
        //
        //here only need care u64PhyAddr[0],no need care u64PhyAddr[1] and u64PhyAddr[2]
        bufinfo->stFrameData.u64PhyAddr[0]= pstBufAllocation->u64PhyAddr;
        
    }
    else if(E_MI_SYS_BUFDATA_META == bufinfo->eBufType)
    {
        //pstBufConfig->stMetaCfg.eBufAllocMode;//to be discuss :how to deal with eBufAllocMode?
        bufinfo->stMetaData.u32Size = pstBufConfig->stMetaCfg.u32Size;
        bufinfo->stRawData.u64PhyAddr = pstBufAllocation->u64PhyAddr;
     }
     else
     {
         BUG();
    }
     return;

}



static void mi_sys_buf_mgr_check_bufinfo(MI_SYS_BufferAllocation_t *pstBufAllocation,MI_SYS_BufConf_t *pstBufConfig, MI_SYS_BufInfo_t *bufinfo)
{

    BUG_ON(bufinfo->eBufType != pstBufConfig->eBufType);
    BUG_ON(bufinfo->u64Pts != pstBufConfig->u64TargetPts);
    BUG_ON(bufinfo->bEndOfStream != false);//to be discuss
    BUG_ON(bufinfo->bUserBuf != false);//to be discuss

    if(E_MI_SYS_BUFDATA_RAW == bufinfo->eBufType)
    {
        BUG_ON(bufinfo->stRawData.u32BufSize != pstBufConfig->stRawCfg.u32Size);
        BUG_ON(bufinfo->stRawData.u64PhyAddr != pstBufAllocation->u64PhyAddr);
        //to be discuss:how about other parameters?
    }
    else if(E_MI_SYS_BUFDATA_FRAME == bufinfo->eBufType)
    {
       BUG_ON (bufinfo->stFrameData.eTileMode !=   E_MI_SYS_FRAME_TILE_MODE_NONE);//set a default value 
        BUG_ON(bufinfo->stFrameData.ePixelFormat != pstBufConfig->stFrameCfg.eFormat);
       BUG_ON (bufinfo->stFrameData.eCompressMode !=   E_MI_SYS_COMPRESS_MODE_NONE);//set a default value 
        BUG_ON(bufinfo->stFrameData.eFrameScanMode != pstBufConfig->stFrameCfg.eFrameScanMode);
        BUG_ON(bufinfo->stFrameData.eFieldType  !=  E_MI_SYS_FIELDTYPE_TOP);//set a default value 
        BUG_ON(bufinfo->stFrameData.stWindowRect.u16X != 0);
        BUG_ON(bufinfo->stFrameData.stWindowRect.u16Y != 0);
        BUG_ON(bufinfo->stFrameData.stWindowRect.u16Width != pstBufConfig->stFrameCfg.u16Width);
        BUG_ON(bufinfo->stFrameData.stWindowRect.u16Height != pstBufConfig->stFrameCfg.u16Height);
       BUG_ON (bufinfo->stFrameData.u16Width != pstBufConfig->stFrameCfg.u16Width);
        BUG_ON(bufinfo->stFrameData.u16Height != pstBufConfig->stFrameCfg.u16Height);
        
        //to be discuss:how to???
        /*
            void* pVirAddr[3];
    MI_PHY u64PhyAddr[3];
    MI_U32 u32Stride[3];
    
        */
        //bufinfo->stFrameData.pVirAddr
        //
        //
        //here only need care u64PhyAddr[0],no need care u64PhyAddr[1] and u64PhyAddr[2]
        BUG_ON(bufinfo->stFrameData.u64PhyAddr[0] != pstBufAllocation->u64PhyAddr);
        
    }
    else if(E_MI_SYS_BUFDATA_META == bufinfo->eBufType)
    {
        //pstBufConfig->stMetaCfg.eBufAllocMode;//to be discuss :how to deal with eBufAllocMode?
        BUG_ON(bufinfo->stMetaData.u32Size != pstBufConfig->stMetaCfg.u32Size);
        BUG_ON(bufinfo->stRawData.u64PhyAddr != pstBufAllocation->u64PhyAddr);
     }
     else
     {
         BUG();
    }
     return;

}

MI_SYS_BufRef_t *mi_sys_create_bufref(MI_SYS_BufferAllocation_t *pstBufAllocation, 
                                                   MI_SYS_BufConf_t *pstBufConfig, OnBufRefRelFunc onRelCB, void *pCBData)
{
    MI_SYS_BufRef_t *pstBufRef;
    
    BUG_ON(!pstBufAllocation);
    BUG_ON(!pstBufConfig);

    pstBufRef = kmem_cache_alloc(mi_sys_bufref_cachep, GFP_KERNEL);
    if(!pstBufRef)
        return NULL;

//before check,should already filled by user though fill function
    mi_sys_buf_mgr_check_bufinfo(pstBufAllocation,pstBufConfig, &pstBufRef->bufinfo);

    
    INIT_LIST_HEAD(&pstBufRef->list);
    pstBufRef->onRelCB = onRelCB;
    pstBufRef->pCBData = pCBData;
    pstBufRef->pstBufAllocation = pstBufAllocation;
    pstBufAllocation->ops->OnRef(pstBufAllocation);
    return pstBufRef;
}
void mi_sys_release_bufref(MI_SYS_BufRef_t *pstBufRef)
{
     BUG_ON(!list_empty(&pstBufRef->list));

     if(pstBufRef->onRelCB)
          pstBufRef->onRelCB(pstBufRef, pstBufRef->pCBData);
     if(pstBufRef->pstBufAllocation)
     {
          BUG_ON(mi_sys_vbpool_check_bufinfo_coherence(pstBufRef->pstBufAllocation, &pstBufRef->bufinfo));
          pstBufRef->pstBufAllocation->ops->OnUnref();
     }
     //for debug purpose
     memset(pstBufRef, 0xE1, sizeof(*pstBufRef));
     kmem_cache_free(mi_sys_bufref_cachep, pstBufRef);
     return;
}
MI_SYS_BufRef_t *mi_sys_dup_bufref(MI_SYS_BufRef_t *pstBufRef, OnBufRefRelFunc onRelCB, void *pCBData)
{
    MI_SYS_BufRef_t *pstDuppedBufRef;
    BUG_ON(NULL == pstBufRef);
    BUG_ON(NULL == pstBufRef->pstBufAllocation);
    BUG_ON(mi_sys_vbpool_check_bufinfo_coherence(pstBufRef->pstBufAllocation, &pstBufRef->bufinfo));

    pstDuppedBufRef = kmem_cache_alloc(mi_sys_bufref_cachep, GFP_KERNEL);
    if(!pstDuppedBufRef)
        return NULL;
    memcpy(pstDuppedBufRef,  pstBufRef, sizeof(*pstBufRef));
    INIT_LIST_HEAD(&pstDuppedBufRef->list);
    pstDuppedBufRef->onRelCB = onRelCB;
    pstDuppedBufRef->pCBData = pCBData;
    pstDuppedBufRef->pstBufAllocation->OnRef();
    return pstDuppedBufRef;
}


void mi_sys_init_buf_queue(MI_SYS_BufferQueue_t*pstQueue)
{
   BUG_ON(!pstQueue);
   INIT_LIST_HEAD(&pstQueue->list);
   pstQueue->queue_buf_count = 0;
   sema_init(&pstQueue->semlock, 1);
}
void mi_sys_add_to_queue_head( MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue)
{
      
      BUG_ON(!pstQueue|| !pstBufRef);
      down(&pstQueue->semlock);
        
      BUG_ON(!list_empty(&pstBufRef->list));
      list_add(&pstBufRef->list, &pstQueue->list);
      pstQueue->queue_buf_count++;
      BUG_ON(pstQueue->queue_buf_count<=0);

      up(&pstQueue->semlock);
       
}

void mi_sys_add_to_queue_tail( MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue)
{
        
      BUG_ON(!pstQueue|| !pstBufRef);
      down(&pstQueue->semlock);
      
      BUG_ON(!list_empty(&pstBufRef->list));
      list_add_tail(&pstBufRef->list, &pstQueue->list);
      pstQueue->queue_buf_count++;
      BUG_ON(pstQueue->queue_buf_count<=0);

      up(&pstQueue->semlock);
       
}

MI_SYS_BufRef_t* mi_sys_remove_from_queue_head(MI_SYS_BufferQueue_t*pstQueue)
{
      MI_SYS_BufRef_t *pstBufRef;
      
      BUG_ON(!pstQueue);
      
      down(&pstQueue->semlock);
      
      if(list_empty(&pstQueue->list))
      {
           BUG_ON(pstQueue->queue_buf_count != 0);
           pstBufRef = NULL;
      }
      else
      {
            pstBufRef = container_of(pstQueue->list->next, MI_SYS_BufRef_t, list);
            list_del(&pstBufRef->list);
            INIT_LIST_HEAD(&pstBufRef->list);
            BUG_ON(pstQueue->queue_buf_count<=0);
            pstQueue->queue_buf_count--;
      }
      
      up(&pstQueue->semlock);
      
      return pstBufRef;
       
}

MI_SYS_BufRef_t* mi_sys_remove_from_queue_tail(MI_SYS_BufferQueue_t*pstQueue)
{
      MI_SYS_BufRef_t *pstBufRef;
      
      BUG_ON(!pstQueue);
      down(&pstQueue->semlock);

      if(list_empty(&pstQueue->list))
      {
           BUG_ON(pstQueue->queue_buf_count != 0);
           pstBufRef = NULL;
      }
      else
      {
            pstBufRef = container_of(pstQueue->list->prev, MI_SYS_BufRef_t, list);
            list_del(&pstBufRef->list);
            INIT_LIST_HEAD(&pstBufRef->list);
            BUG_ON(pstQueue->queue_buf_count<=0);
            pstQueue->queue_buf_count--;
      }
      up(&pstQueue->semlock);
      return pstBufRef;
       
}

void mi_sys_remove_from_queue(MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue)
{
      int found = 0;
      struct list_head *pos;
       MI_SYS_BufRef_t *pstCur;
      BUG_ON(!pstQueue);
      BUG_ON(!pstBufRef);
      down(&pstQueue->semlock);

      list_for_each(pos, pstQueue->list)
      {
           if(pos == &pstBufRef->list)
           {
              found = 1;
              break;
           }
      }
      BUG_ON(!found);
      list_del(&pstBufRef->list);
      LIST_HEAD_INIT(&pstBufRef->list);
      up((&pstQueue->semlock);

}
MI_S32 mi_sys_attach_allocator_to_collection(mi_sys_Allocator_t *pstAllocator, MI_SYS_Allocator_Collection_t *pstAllocatorCollection)
{

    mi_sys_Allocator_t *pstAllocator;
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    struct list_head *pos;
    int count = 0;
    
    BUG_ON(!pstAllocatorCollection);
    BUG_ON(pstAllocatorCollection->u32MagicNumber != __MI_SYS_COLLECTION_MAGIC_NUM__);

    pstAllocatorRef = (MI_SYS_AllocatorRef_t*)kmalloc(sizeof(*pstAllocatorRef));

    if(!pstAllocatorRef)
        return MI_ERR_NO_MEM;

        
    pstAllocatorRef->pstAllocator = pstAllocator;
    BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->OnRef);
    pstAllocatorRef->pstAllocator->ops->OnRef(pstAllocatorRef->pstAllocator);
    
    down(&pstAllocatorCollection->semlock);
    list_for_each(pos, &pstAllocatorCollection->list)
    {
         pstAllocatorRef = container_of(pos, MI_SYS_AllocatorRef_t, list);
         BUG_ON(pstAllocatorRef->pstAllocator==NULL ||
                         pstAllocatorRef->pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);
         count++;
         BUG_ON(pstAllocatorCollection->collection_size < count);
         
         if(pstAllocatorRef->pstAllocator == pstAllocator)
         {
              up(&pstAllocatorCollection->semlock);
              BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->OnUnref);
              pstAllocatorRef->pstAllocator->ops->OnUnref(pstAllocatorRef->pstAllocator);
              kfree(pstAllocatorRef);
              return MI_SUCCESS;
         }
    }

    BUG_ON(count != pstAllocatorCollection->collection_size);
    list_add_tail(&pstAllocatorRef->list, &pstAllocatorCollection->list);
    pstAllocatorCollection->collection_size++;
    up(&pstAllocatorCollection->semlock);
    return MI_SUCCESS;
    
}
MI_S32 mi_sys_detach_allocator_from_collection(mi_sys_Allocator_t *pstAllocator, MI_SYS_Allocator_Collection_t *pstAllocatorCollection)
{
    mi_sys_Allocator_t *pstAllocator;
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    struct list_head *pos;
    int count = 0;
    
    BUG_ON(!pstAllocatorCollection);
    BUG_ON(pstAllocatorCollection->u32MagicNumber != __MI_SYS_COLLECTION_MAGIC_NUM__);


    down(&pstAllocatorCollection->semlock);
    list_for_each(pos, &pstAllocatorCollection->list)
    {
         pstAllocatorRef = container_of(pos, mi_sys_Allocator_t, list);
         BUG_ON(pstAllocatorRef->pstAllocator==NULL ||
                         pstAllocatorRef->pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);
         count++;
         BUG_ON(pstAllocatorCollection->collection_size < count);
         
         if(pstAllocatorRef->pstAllocator == pstAllocator)
         {
              list_del(&pstAllocatorRef->list);
              pstAllocatorCollection->collection_size--;
              up(&pstAllocatorCollection->semlock);

              BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->OnUnref);
              pstAllocatorRef->pstAllocator->ops->OnUnref(pstAllocatorRef->pstAllocator);
              kfree(pstAllocatorRef);
              
              return MI_SUCCESS;
         }
    }
    BUG_ON(count != pstAllocatorCollection->collection_size);
    up(&pstAllocatorCollection->semlock);
    return MI_ERR_NO_ITEM;
}
void mi_sys_init_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection)
{
    BUG_ON(!pstAllocatorCollection);
    INIT_LIST_HEAD(&pstAllocatorCollection->list);
    pstAllocatorCollection->collection_size = 0;
    pstAllocatorCollection->u32MagicNumber == __MI_SYS_COLLECTION_MAGIC_NUM__;
    sema_init(&pstAllocatorCollection->semlock, 1);   
}
void mi_sys_deinit_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection)
{
    mi_sys_Allocator_t *pstAllocator;
    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    
    BUG_ON(!pstAllocatorCollection);
    BUG_ON(pstAllocatorCollection->u32MagicNumber != __MI_SYS_COLLECTION_MAGIC_NUM__);

    while(!list_empty(&pstAllocatorCollection->list))
    {
         down(&pstAllocatorCollection->semlock);
         if(list_empty(&pstAllocatorCollection->list))
         {
              BUG_ON( pstAllocatorCollection->collection_size);
              up(&pstAllocatorCollection->semlock);
              return;
         }
         pstAllocatorRef = container_of(pstAllocatorCollection->list.next, MI_SYS_AllocatorRef_t, list);
         BUG_ON(pstAllocatorRef->pstAllocator==NULL ||
                         pstAllocatorRef->pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);

         BUG_ON( pstAllocatorCollection->collection_size<1);
         list_del(&pstAllocatorRef->list);
         pstAllocatorCollection->collection_size--;
         up(&pstAllocatorCollection->semlock);

         BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->OnUnref);
         pstAllocatorRef->pstAllocator->ops->OnUnref(pstAllocatorRef->pstAllocator);
         kfree(pstAllocatorRef);
    }
  
}
MI_SYS_BufferAllocation_t * mi_sys_alloc_from_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection, MI_Buf_Config *pstBufConfig , MI_SYS_ChnPort_t *pstChnPort)
{


    MI_SYS_AllocatorRef_t *pstAllocatorRef;
    struct list_head *pos;
    int count = 0;
    int suit_best = INT_MIN;
    mi_sys_Allocator_t *pstAllocator_suit_best = NULL;
    int suit_cur;
     MI_SYS_BufferAllocation_t *allocation;
        
    BUG_ON(!pstAllocatorCollection);
    BUG_ON(pstAllocatorCollection->u32MagicNumber != __MI_SYS_COLLECTION_MAGIC_NUM__);
    BUG_ON(!stBufConfig);


    down(&pstAllocatorCollection->semlock);
    list_for_each(pos, &pstAllocatorCollection->list)
    {
         pstAllocatorRef = container_of(pos, mi_sys_Allocator_t, list);
         BUG_ON(pstAllocatorRef->pstAllocator==NULL ||
                         pstAllocatorRef->pstAllocator->u32MagicNumber != __MI_SYS_ALLOCATOR_MAGIC_NUM__);
         count++;
         BUG_ON(pstAllocatorCollection->collection_size < count);
         BUG_ON(!pstAllocatorRef->pstAllocator->ops || !pstAllocatorRef->pstAllocator->ops->suit_bufconfig);
         suit_cur = pstAllocatorRef->pstAllocator->ops->suit_bufconfig(pstAllocatorRef->pstAllocator, pstBufConfig , pstChnPort)
         if(suit_cur > suit_best)
         {
              if(pstAllocator_suit_best)
                  pstAllocator_suit_best->ops->OnUnref(pstAllocator_suit_best);
              pstAllocator_suit_best = pstAllocatorRef->pstAllocator;
              pstAllocator_suit_best->ops->OnRef(pstAllocator_suit_best);
         }
    }
    BUG_ON(count != pstAllocatorCollection->collection_size);
    up(&pstAllocatorCollection->semlock);

    if(pstAllocator_suit_best)
    {
        allocation =  pstAllocator_suit_best->ops->alloc(pstAllocator_suit_best,  pstBufConfig , pstChnPort);
        pstAllocator_suit_best->ops->OnUnref(pstAllocator_suit_best);
        return allocation;
    }
          
    return NULL;
    
}


/*
this function will be used by user,not be used by other internal VB POOL functions.
*/
unsigned long _mi_calc_sys_frame_size(MI_SYS_BufFrameConfig_t stFrameConfig,  unsigned long alignment)
{
    MI_U16 u16Width = stFrameConfig.u16Width;
    MI_U16 u16Height = stFrameConfig.u16Height;
    unsigned long size;

    switch(stFrameConfig->eFormat)
    {
             case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
                 u16Width = ALIGN_UP(u16Width, 2);
                 size = u16Width *  2;
                 size = ALIGN_UP(size, alignment);
                 size *=u16Height; 
                break;
             case E_MI_SYS_PIXEL_FRAME_ARGB8888:
             case E_MI_SYS_PIXEL_FRAME_ABGR8888:
                 size = u16Width *  4;
                 size = ALIGN_UP(size, alignment);
                 size *=u16Height; 
                  break;
              case     E_MI_SYS_PIXEL_FRAME_RGB565:
              case E_MI_SYS_PIXEL_FRAME_ARGB1555: 
                 size = u16Width *  2;
                 size = ALIGN_UP(size, alignment);
                 size *=u16Height; 
                  break;
              case     E_MI_SYS_PIXEL_FRAME_I2:
                size = (u16Width+3)/4;
                 size = ALIGN_UP(size, alignment);
                 size *=u16Height; 
                  break;
              case E_MI_SYS_PIXEL_FRAME_I4:
                size = (u16Width+1)/2;
                 size = ALIGN_UP(size, alignment);
                 size *=u16Height;
                   break;
              case E_MI_SYS_PIXEL_FRAME_I8:
                 size = ALIGN_UP(u16Width, alignment);
                 size *=u16Height;
                    break;
              case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422:
                 size = ALIGN_UP(u16Width, alignment);
                 size =size*u16Height*2;
                     break;
               case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
                    u16Height = ALIGN_UP(u16Height, 2)
                    size = ALIGN_UP(u16Width, alignment);
                    size =size*u16Height*3/2;
                    break;

             case  E_MI_SYS_PIXEL_FRAME_YUV_MST_420:
                 u16Height = ALIGN_UP(u16Height, 2)
                 size = ALIGN_UP(u16Width, alignment);
                 size =size*u16Height*3/2;
                 break;
    
    
               //vdec mstar private video format
               //
              case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE1_H264:
                   u16Height = ALIGN_UP(u16Height, 16)
                   u16Width = ALIGN_UP(u16Width, 16)
                   size = ALIGN_UP(u16Width, alignment);
                   size =size*u16Height*3/2;//YC420_MSTTILE1_H264:  (8+4)/8=1.5,for code we use 3/2
                   break;
             case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE2_H265:
                   u16Width = ALIGN_UP(u16Width, 16)
                   u16Height = ALIGN_UP(u16Height, 16)
                   size = ALIGN_UP(u16Width, alignment);
                   size =size*u16Height*3/2;//YC420_MSTTILE2_H265:  (8+4)/8=1.5,for code we use 3/2
                   break;
              case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE3_H265:
                   u16Width = ALIGN_UP(u16Width, 16)
                   u16Height = ALIGN_UP(u16Height, 16)
                   size = ALIGN_UP(u16Width, alignment);
                   size =size*u16Height*2;//YC420_MSTTILE3_H265: ((8+2)+(4+2))/8=2,for code we use 2
                   break;

              default:
                    BUG();
    }

    return size;
}


/*
this function just get size,
for UFDATA_FRAME,should set size first,and set process has been done by Calc_Frame_Size_by_PixelFormat.
*/
 unsigned long mi_sys_buf_mgr_get_size(MI_SYS_BufConf_t *stBufConfig)
{
    unsigned long size_to_alloc;
       MI_SYS_BufFrameConfig_t stFrameConfig;
       MI_SYS_BufRawConfig_t stRawConfig;
       MI_SYS_MetaDataConfig_t stCustomizedConfig;
       
    if(stBufConfig->buf_type == E_MI_SYS_BUFDATA_RAW)
    {
           stRawConfig =stBufConfig->Buf_Config;
           size_to_alloc = stRawConfig.u32Size;
    }
    else if(stBufConfig->buf_type == E_MI_SYS_BUFDATA_FRAME)
    {
           stFrameConfig = stBufConfig->Buf_Config;

           size_to_alloc = _mi_calc_sys_frame_size(stFrameConfig);


    }
    else if(stBufConfig->buf_type == E_MI_SYS_BUFDATA_META)
    {
           stCustomizedConfig = stBufConfig->Buf_Config;
           size_to_alloc = stCustomizedConfig.u32Size;
    }
    else
    {
           MI_SYS_BUG();
    }



    return size_to_alloc;
}


MI_SYS_USER_MAP_CTX mi_sys_buf_get_cur_user_map_ctx()
{
    return (MI_SYS_USER_MAP_CTX)current->group_leader;

}

int mi_sys_userdev_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct sg_table *sg_table =  = file->private_data->sg_table ;//get [pa1,length1],...[pa_n,length_n] info from file private
    unsigned long addr = vma->vm_start;
    unsigned long offset = vma->vm_pgoff * PAGE_SIZE;
    struct scatterlist *sg;
    int i;
    int ret;

    if(!sg_table )
        return -EINVAL;

    for_each_sg(sg_table->sgl, sg, sg_table->nents, i) {
        struct page *page = sg_page(sg);
        unsigned long remainder = vma->vm_end - addr;
        unsigned long len = sg->length;

        if (offset >= sg->length) {
             offset -= sg->length;
             continue;
        } else if (offset) {
             page += offset / PAGE_SIZE;
             len = sg->length - offset;
             offset = 0;
        }
        len = min(len, remainder);
        ret = remap_pfn_range(vma, addr, page_to_pfn(page), len,
                                     vma->vm_page_prot);
        if (ret)
        {   
             return -ENOSPC;
        }
        addr += len;
        if (addr >= vma->vm_end)
        {
             return 0;
        }
    }

    return 0;

}

static struct file_operations mma_userdev_fops = {
    .owner        = THIS_MODULE,
    .open        = mi_sys_userdev_open,
    .release    = mi_sys__userdev_release,
    .unlocked_ioctl  = mi_sys__userdev_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl   = compat_mi_sys__userdev_ioctl,
#endif
    .mmap        =  mi_sys_userdev_mmap,
};


static int mi_sys_map_sg_table(struct file *file, struct sg_table *sg_table,virt_addr)
{
    unsigned long prot = PROT_READ|PROT_WRITE;
    unsigned long flags;
    unsigned long pgoff;
    unsigned long populate;
    struct scatterlist *sg;

    int i;
    int total_len=0;
         


    for_each_sg(sg_table->sgl, sg, sg_table->nents, i) {
            total_len += sg->length;
    }

    down_write(&current->mm->mmap_sem);
    
    file->private_data->sgl = sg_table;//set [pa1,length1],...[pa_n,length_n] info to file private
         
    virt_addr = do_mmap_pgoff(file, 0, total_len, prot, flags, pgoff, &populate);
    file->private_data->sgl = NULL;
         
    up_write(&current->mm->mmap_sem);
    
    sg_free_table(sg_table);
    
    if(IS_ERR_VALUE(virt_addr)) {
        up_write(&current->mm->mmap_sem);
        return MI_ERR_COMMON_ERR;
    }
    if (populate)
        mm_populate(virt_addr, populate);

    up_write(&current->mm->mmap_sem);
    return MI_SUCCESS;
}

unsigned int mi_sys_buf_mgr_user_map(struct sg_table *sg_table,void *user_map_ptr,MI_SYS_USER_MAP_CTX *user_map_ctx)
{
   int ret;
   
    BUG_ON(user_map_ctx != NULL);

    BUG_ON(!sg_table);

    //get file
    ///TODO:

    ret = mi_sys_map_sg_table(file, sg_table,user_map_ptr);
    if(ret == MI_SUCCESS)
    {
        *user_map_ctx = mi_sys_buf_get_cur_user_map_ctx();
    }
    else
        *user_map_ctx = NULL;

    return ret;
        

}



int mi_sys_buf_mgr_user_unmap(void *user_map_ptr,MI_SYS_USER_MAP_CTX *user_map_ctx,unsigned long total_len)
{

    int ret;

    MI_SYS_ASSERT(user_map_ctx == mi_sys_buf_get_cur_user_map_ctx());//who map,who un-map
         

    down_write(&current->mm->mmap_sem);
    ret = do_munmap(current->mm, user_map_ptr, total_len);
    if(ret != 0)
    {
        up_write(&current->mm->mmap_sem);
        return MI_ERR_COMMON_ERR;
    }
    *user_map_ctx = NULL;
    up_write(&current->mm->mmap_sem);
    return MI_SUCCESS;
}
