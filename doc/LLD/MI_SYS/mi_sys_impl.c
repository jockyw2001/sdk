//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   mi_vdec_impl.c
/// @brief vdec module impl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
///#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/types.h>


#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "mi_sys_buf_mgr.h"
#include "mi_sys_impl.h"
#include "mi_sys_internal.h"
#define E_MI_ERR_BUSY 4
#define E_MI_ERR_NOMEM 5
#define MI_SYS_INVALIDPARAM 6
#define E_MI_ERR_NOMEM 7
#define E_MI_ERR_NOBUF 8
#define MI_SYS_NO_HANDLE 9


extern void mi_sys_vbpool_fill_bufinfo(MI_SYS_BufferAllocation_t *pstBufAllocation, MI_SYS_BufInfo_t pstBufInfo);
extern int  mi_sys_vbpool_check_bufinfo_coherence(MI_SYS_BufferAllocation_t *pstBufAllocation, MI_SYS_BufInfo_t pstBufInfo);

struct idr g_mi_sys_buf_handle_idr;
struct kmem_cache *mi_sys_bufref_cachep;
struct kmem_cache *mi_sys_inputport_cachep;
struct kmem_cache *mi_sys_outputport_cachep;
struct kmem_chche *mi_sys_taskinfo_cachep;


LIST_HEAD(mi_sys_global_dev_list);
DEFINE_SPINLOCK(mi_sys_global_dev_list_lock);


//检查input port与output port是否是绑定关系
static inline MI_S32  _MI_SYS_IMPL_CheckInputOutputBindCoherence(struct MI_SYS_InputPort_s *pstInputPort, struct MI_SYS_OutputPort_s *pstOutputPort)
{
	MI_S32 find = 0;
	struct list_head *pos;
	struct MI_SYS_InputPort_s *pstInputportCur; 
	down(&pstOutputPort->stBindedInputListSemlock);
	list_for_each(pos , &pstOutputPort->stBindPeerInputPortList)
	{
	   pstInputportCur = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
	   if(pstInputportCur == pstInputPort)
	   {
	       find = 1;
	       BUG_ON(pstInputportCur->pstBindPeerOutputPort != pstOutputPort);
		   break;
	   }
	}
	up(&pstOutputPort->stBindedInputListSemlock);
	BUG_ON(find == 0);
	return find;
	
}

struct MI_SYS_InputPort_s *_MI_SYS_IMPL_GetInportPort(struct MI_SYS_ChnPort_s *pstChnPort)
{
	struct MI_SYS_MOD_DEV_s *pstModDev;
	struct MI_SYS_InputPort_s *pstInputport; 
	struct list_head *pos;

	list_for_each(pos, &mi_sys_global_dev_list)
	{	
		pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
		BUG_ON(!pstModDev);
		if(pstModDev->eModuleId == pstChnPort->eModId && pstModDev->u32DevId == pstChnPort->u32DevId)
		{
			break;
		}
	}
	BUG_ON(!pstInputport);
	
	return pstInputport;
}

//创建input 端口
struct MI_SYS_InputPort_s *_MI_SYS_IMPL_CreateInputPort(struct  MI_SYS_Channel_s *pstChannel, int u32PortIndex)
{

    struct MI_SYS_InputPort_s *pstInputPort;

    BUG_ON(!pstChannel);

    pstInputPort = (struct MI_SYS_InputPort_s*)kmem_cache_alloc(mi_sys_inputport_cachep, GFP_KERNEL);
    if(!pstInputPort)
        return NULL;
	
    BUG_ON(u32PortIndex >= pstChannel->pstModDev->u32InputPortNum);
    //初始化port内参数
    pstInputPort->pstChannel = pstChannel;
    pstInputPort->u32PortIndex = u32PortIndex;
    mi_sys_init_buf_queue(&pstInputPort->stUsrInjectBufQueue);
    mi_sys_init_buf_queue(&pstInputPort->stBindInputBufQueue);
    mi_sys_init_buf_queue(&pstInputPort->stWorkingQueue);
    pstInputPort->pstBindPeerOutputPort = NULL;
	pstInputPort->bPortEnable = TRUE;
    pstInputPort->u32MagicNumber = __MI_SYS_INPUTPORT_MAGIC_NUM__;
    pstInputPort->pstCusBufAllocator = NULL;
    
    return pstInputPort;
}

//销毁当前input 端口
MI_S32 _MI_SYS_IMPL_DestroyInputPort(struct  MI_SYS_InputPort_s *pstInputPort)
{
	MI_SYS_BufRef_t *pstBufRef;

	BUG_ON(!pstInputPort | !pstInputPort->pstChannel || !pstInputPort->pstChannel->pstModDev);
	BUG_ON(pstInputPort->pstChannel->pstModDev->u32InputPortNum <=pstInputPort->u32PortIndex);
	BUG_ON(pstInputPort->pstBindPeerOutputPort);

    //如果working queue还有数据在处理，直接返回busy
	if(pstInputPort->stWorkingQueue.queue_buf_count) 
	    return MI_SYS_ERR_BUSY;
	
	//把usr_inject_buf_queue与bind_input_buf_queue 中的buf release
	while(pstBufRef=mi_sys_remove_from_queue_head(&pstInputPort->stUsrInjectBufQueue))
	{
	    mi_sys_release_bufref(pstBufRef);
	}
	while(pstBufRef=mi_sys_remove_from_queue_head(&pstInputPort->stBindInputBufQueue))
	{
	    mi_sys_release_bufref(pstBufRef);
	}

	//for debug purpose
	memset(pstInputPort, 0xE2, sizeof(*pstInputPort));
	kmem_cache_free(mi_sys_inputport_cachep, pstInputPort);
	return MI_SUCCESS;
}


MI_S32 _MI_SYS_IMPL_DestroyOutputPort(struct  MI_SYS_OutputPort_s *pstOutputPort)
{
	MI_SYS_BufRef_t *pstBufRef;

	BUG_ON(!pstOutputPort | !pstOutputPort->pstChannel || !pstOutputPort->pstChannel->pstModDev);
	//BUG_ON(pstOutputPort->stBindPeerInputPortList);

    //如果working queue还有数据在处理，直接返回busy
	if(pstOutputPort->stWorkingQueue.queue_buf_count) 
	    return MI_SYS_ERR_BUSY;
	
	while(pstBufRef=mi_sys_remove_from_queue_head(&pstOutputPort->stUsrGetFifoBufQueue))
	{
	    mi_sys_release_bufref(pstBufRef);
	}
	
	//for debug purpose
	memset(pstOutputPort, 0xE2, sizeof(*pstOutputPort));
	kmem_cache_free(mi_sys_outputport_cachep, pstOutputPort);
	return MI_SUCCESS;
}


//获取当前input 端口绑定的output 端口
struct MI_SYS_OutputPort_s * _MI_SYS_IMPL_GetInputportPeer(struct MI_SYS_InputPort_s *pstInputPort)
{
     BUG_ON(!pstInputPort);
	 
     if(list_empty(&pstInputPort->stBindRelationShipList))
     {
         BUG_ON(pstInputPort->pstBindPeerOutputPort);
         return NULL;
     }

     BUG_ON(!pstInputPort->pstBindPeerOutputPort);
     
     _MI_SYS_IMPL_CheckInputOutputBindCoherence(pstInputPort, pstInputPort->pstBindPeerOutputPort);
     
     return pstInputPort->pstBindPeerOutputPort;
}


//绑定端口
MI_S32 _MI_SYS_IMPL_BindChannelPort(struct MI_SYS_InputPort_s *pstInputPort,  struct MI_SYS_OutputPort_s *pstOutputPort, 
                                            int u32SrcFrmrate,    int u32DstFrmrate)
{
     BUG_ON(!pstOutputPort || !pstInputPort);
	 BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
	 BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
	 
	 if(pstOutputPort->pstChannel == pstInputPort->pstChannel)
		  return E_MI_ERR_ILLEGAL_PARAM;


	 down(&pstOutputPort->stBindedInputListSemlock);
	 //判断input port是否被绑定
	 if(!list_empty(&pstInputPort->stBindRelationShipList))
	 {
		  BUG_ON(!pstInputPort->pstBindPeerOutputPort);
		  up(&pstOutputPort->stBindedInputListSemlock);
		  
		  return E_MI_ERR_BUSY;
	 }
	 BUG_ON(pstInputPort->pstBindPeerOutputPort);
	 //把input port的bind_relationship_list 结点加到bind_peer_inputport_list链表中
	 list_add(&pstInputPort->stBindRelationShipList, &pstOutputPort->stBindPeerInputPortList);
	 pstInputPort->u32SrcFrmrate= u32SrcFrmrate;
	 pstInputPort->u32DstFrmrate = u32DstFrmrate;
	 pstInputPort->u32CurRcParam= u32SrcFrmrate;
	 
	 up(&pstOutputPort->stBindedInputListSemlock);
	 if(pstInputPort->pstChannel->pstModDev->ops.OnBindInputPort)
	 	 pstInputPort->pstChannel->pstModDev->ops.OnBindInputPort( pstInputPort, pstOutputPort , pstInputPort->pstChannel->pstModDev->pUsrData);
	 if(pstOutputPort->pstChannel->pstModDev->ops.OnBindOutputPort)
	 	 pstOutputPort->pstChannel->pstModDev->ops.OnBindOutputPort( pstOutputPort,pstInputPort , pstOutputPort->pstChannel->pstModDev->pUsrData);

	 return MI_SUCCESS;
     
}

//解绑端口
MI_S32 _MI_SYS_IMPL_UnBindChannelPort(struct MI_SYS_InputPort_s *pstInputPort, struct  MI_SYS_OutputPort_s *pstOutputPort)
{
	BUG_ON(!pstOutputPort || !pstInputPort);

	BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
	BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

	struct MI_SYS_InputPort_s *pstCur = NULL; 
	int find = 0;
	struct list_head *pos;
	//判断是否被判定
	if(!list_empty(&pstInputPort->stBindRelationShipList))
	{
		BUG_ON(!pstInputPort->pstBindPeerOutputPort);
		return E_MI_ERR_ILLEGAL_PARAM;
	}

	down(&pstOutputPort->stBindedInputListSemlock);
	//先从链表bind_peer_inputport_list 中打到当前需要解决的input port hanel
	list_for_each(pos, &pstOutputPort->stBindPeerInputPortList)
	{
		pstCur = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
		//判断当前的链接中的结点是不是都绑定在当前output port下的(只为检测错误)
		BUG_ON(pstCur->pstBindPeerOutputPort != pstOutputPort);
		if(pstCur == pstInputPort)
		{
			//如果找到当前结点，则删掉该结点
			pstInputPort->pstBindPeerOutputPort = NULL;
			list_del(&pstInputPort->stBindRelationShipList);
			INIT_LIST_HEAD(&pstInputPort->stBindRelationShipList);
			break;
		}
	}
	up(&pstOutputPort->stBindedInputListSemlock);

	if(pstCur == pstInputPort)
	{
		if(pstInputPort->pstChannel->pstModDev->ops.OnUnBindInputPort)
			pstInputPort->pstChannel->pstModDev->ops.OnUnBindInputPort(pstInputPort, pstOutputPort , pstInputPort->pstChannel->pstModDev->pUsrData);
		if(pstOutputPort->pstChannel->pstModDev->ops.OnUnBindOutputPort)
			pstOutputPort->pstChannel->pstModDev->ops.OnUnBindOutputPort(pstOutputPort,pstInputPort , pstOutputPort->pstChannel->pstModDev->pUsrData);
		
		return MI_SUCCESS;
	}

	return E_MI_ERR_ILLEGAL_PARAM;

}

//创建output 端口
struct MI_SYS_OutputPort_s *_MI_SYS_IMPL_CreateOutputPort(struct MI_SYS_Channel_s *pstChannel, int u32PortIndex)
{
    MI_SYS_OutputPort_t *pstOutputPort;

    BUG_ON(!pstChannel);

    pstOutputPort = (struct MI_SYS_InputPort_s*)kmem_cache_alloc(mi_sys_outputport_cachep, GFP_KERNEL);
    if(!pstOutputPort)
        return NULL;
    BUG_ON(u32PortIndex >= pstChannel->pstModDev->u32OutputPortNum);

    // 初始化MI_SYS_OutputPort_t
    pstOutputPort->pstChannel = pstChannel;
    mi_sys_init_buf_queue(&pstOutputPort->stUsrGetFifoBufQueue);
    mi_sys_init_buf_queue(&pstOutputPort->stWorkingQueue);
    sema_init(&pstOutputPort->stBindedInputListSemlock, 1);
    INIT_LIST_HEAD(&pstOutputPort->stBindPeerInputPortList);
    pstOutputPort->u32MagicNumber = __MI_SYS_OUTPORT_MAGIC_NUM__;
    atomic_set(&pstOutputPort->usrLockedBufCnt, 0);
	atomic_set(&pstOutputPort->totalOutputPortInUsedBuf , 0);
    pstOutputPort->pstCusBufAllocator = NULL;
	pstOutputPort->bPortEnable = TRUE;
	pstOutputPort->u32OutputPortBufCntQuota = MI_SYS_OUTPUT_BUF_CNT_QUOTA_DEFAULT;
    
    return pstOutputPort;
}

//
void _MI_SYS_IMPL_ReleaseOutputPort(struct  MI_SYS_OutputPort_s *pstOutputPort)
{
    MI_SYS_BufRef_t *pstBufRef;

    BUG_ON(!pstOutputPort);

	if(pstOutputPort->stWorkingQueue.queue_buf_count)
		return ;
   
    while(!list_empty(&pstOutputPort->stBindPeerInputPortList))
    {
		struct MI_SYS_InputPort_s *pstInputPort  = container_of(&pstOutputPort->stBindPeerInputPortList.next, struct MI_SYS_InputPort_s, stBindRelationShipList);
		_MI_SYS_IMPL_UnBindChannelPort(pstInputPort, pstOutputPort);
    }
    while(pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stUsrGetFifoBufQueue))
    {
		mi_sys_release_bufref(pstBufRef);
    }
    memset(pstOutputPort, 0xE3, sizeof(*pstOutputPort));
    kmem_cache_free(mi_sys_outputport_cachep, pstOutputPort);
}

MI_S32 _MI_SYS_IMPL_InitChannel( MI_SYS_Channel_t *pstChannel, MI_SYS_MOD_DEV_t* pstModDev)
{
    MI_U32 u32PortId;
    
    BUG_ON(!pstChannel ||!pstModDev);
    BUG_ON(pstModDev->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
    BUG_ON(pstModDev->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);

    memset(pstChannel, 0, sizeof(*pstChannel));

    pstChannel->pstModDev = pstModDev;

    // 在channel 上创建input port
    for( u32PortId = 0 ; u32PortId < pstModDev->u32InputPortNum ; u32PortId++)
    {
		pstChannel->pastInputPorts[u32PortId] = _MI_SYS_IMPL_CreateInputPort( pstChannel,u32PortId);
		if(!pstChannel->pastInputPorts[u32PortId] )
		goto Failed_Rel;
    }
	
	// 在channel 上创建output port
    for( u32PortId = 0 ; u32PortId < pstModDev->u32OutputPortNum ; u32PortId ++)
    {
        pstChannel->pastOutputPorts[u32PortId] = _MI_SYS_IMPL_CreateOutputPort( pstChannel,  u32PortId);
        if(!pstChannel->pastOutputPorts[u32PortId] )
            goto Failed_Rel;
    }
    pstChannel->u32MagicNumber = __MI_SYS_CHN_MAGIC_NUM__;

    mi_sys_init_allocator_collection(&pstChannel->stAllocatorCollection);
    pstChannel->bChnEnable = TRUE;
    return MI_SUCCESS;

Failed_Rel:
    for( u32PortId = 0 ; u32PortId < pstModDev->u32InputPortNum ; u32PortId ++)
    {
        if(pstChannel->pastInputPorts[u32PortId])
             _MI_SYS_IMPL_DestroyInputPort(pstChannel->pastInputPorts[u32PortId]);
        else
            break;
    }
    for( u32PortId = 0;u32PortId < pstModDev->u32OutputPortNum ; u32PortId ++)
    {
        if( pstChannel->pastOutputPorts[u32PortId])
            _MI_SYS_IMPL_DestroyOutputPort(pstChannel->pastOutputPorts[u32PortId]);
        else
            break;
    }
    memset(pstChannel, 0xE3, sizeof(*pstChannel));
    return MI_ERR_NO_MEM;
}
void _MI_SYS_IMPL_DeinitChannel( MI_SYS_Channel_t *pstChannel)
{ 
	MI_U32 u32PortId;

	BUG_ON(!pstChannel);
	BUG_ON(!pstChannel->pstModDev);
	//销毁所有input port
	for( u32PortId=0 ; u32PortId < MI_SYS_MAX_INPUT_PORT_CNT ; u32PortId ++)
	{
		if(u32PortId<pstChannel->pstModDev->u32InputPortNum)
		{   
		     BUG_ON(!pstChannel->pastInputPorts[u32PortId]);
		     BUG_ON(pstChannel->pastInputPorts[u32PortId]->pstChannel != pstChannel);
		     _MI_SYS_IMPL_DestroyInputPort(pstChannel->pastInputPorts[u32PortId]);
		}
		else
		{
		    BUG_ON(pstChannel->pastInputPorts[u32PortId]);
		}
	}
	//销毁所有output port
	for( u32PortId = 0;u32PortId < MI_SYS_MAX_OUTPUT_PORT_CNT ; u32PortId ++)
	{
	    if(u32PortId<pstChannel->pstModDev->u32OutputPortNum)
	    {   
	         BUG_ON(!pstChannel->pastOutputPorts[u32PortId]);
	         BUG_ON(pstChannel->pastOutputPorts[u32PortId]->pstChannel != pstChannel);
	         _MI_SYS_IMPL_ReleaseOutputPort(pstChannel->pastOutputPorts[u32PortId]);
	    }
	    else
	    {
	         BUG_ON(pstChannel->pastOutputPorts[u32PortId]);
	    }
	}
	pstChannel->u32ChannelId = 0;
	pstChannel->bChnEnable = FALSE;
	mi_sys_deinit_allocator_collection(&pstChannel->stAllocatorCollection);
	memset(pstChannel, 0xE3, sizeof(*pstChannel));
}

void _MI_SYS_IMPL_DupBufrefIntoOutputFifoQueue(struct MI_SYS_OutputPort_s *pstOutputPort, struct MI_SYS_BufRef_s *pstBufRef)
{
	struct MI_SYS_BufRef_s *pstNewBufRef;
	int i_valid_fifo_cnt;

	BUG_ON(!pstOutputPort ||pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
	BUG_ON(!pstOutputPort->pstChannel ||pstOutputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
	BUG_ON(!pstBufRef);
	
	i_valid_fifo_cnt = (int)pstOutputPort->u32UsrFifoCount - pstOutputPort->stUsrGetFifoBufQueue.queue_buf_count;

	BUG_ON(i_valid_fifo_cnt< 0);


	if(i_valid_fifo_cnt && pstOutputPort->bPortEnable && pstOutputPort->pstChannel->bChnEnable)
	{ 
		pstNewBufRef = mi_sys_dup_bufref(pstBufRef, NULL, NULL);//mi_sys_dup_bufref(pstBufRef, mi_sys_output_UsrBuf_OnBufRefRelFunc, pstOutputPort);
		if(!pstNewBufRef)
		{
//			WARNING("mi_sys_dup_bufref_into_output_fifo_queue no mem!\n");
			return;
		}
		mi_sys_add_to_queue_tail(pstNewBufRef, &pstOutputPort->stUsrGetFifoBufQueue);
		while((int)pstOutputPort->stUsrGetFifoBufQueue.queue_buf_count>i_valid_fifo_cnt)
		{
			struct MI_SYS_BufRef_s *pstTmpBufRef = mi_sys_remove_from_queue_head(&pstOutputPort->stUsrGetFifoBufQueue);
			if(!pstTmpBufRef)
		     	BUG();
			mi_sys_release_bufref(pstTmpBufRef);   
		}
	}
	else
	{
	     while(1)
	      {
	           struct MI_SYS_BufRef_s *pstTmpBufRef = mi_sys_remove_from_queue_head(&pstOutputPort->stUsrGetFifoBufQueue);
	           if(!pstTmpBufRef)
	            break;
	          mi_sys_release_bufref(pstTmpBufRef);   
	      }
	}

}

void _MI_SYS_IMPL_DupBufrefIntoInputBindQueue(struct MI_SYS_InputPort_s *pstInputPort, MI_SYS_BufRef_t *pstBufRef)
{
	struct MI_SYS_BufRef_s *pstNewBufRef;   

	BUG_ON(!pstInputPort ||pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
	BUG_ON(!pstInputPort->pstChannel ||pstInputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
	BUG_ON(!pstBufRef);


	if(pstInputPort->bPortEnable && pstInputPort->pstChannel->bChnEnable)
	{ 

		pstNewBufRef = mi_sys_dup_bufref(pstBufRef, NULL, NULL);
		if(!pstNewBufRef)
		{
		 //  WARNING("mi_sys_dup_bufref_into_output_fifo_queue no mem!\n");
		   return;
		}
		mi_sys_add_to_queue_tail(pstNewBufRef,  &pstInputPort->stBindInputBufQueue);
	}
	else
	{
		while(1)
		{
			struct MI_SYS_BufRef_s *pstTmpBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stBindInputBufQueue);
			if(!pstTmpBufRef)
				break;
			mi_sys_release_bufref(pstTmpBufRef);   
		}
	}
}


MI_S32 _MI_SYS_IMPL_InputPortRewindBuf(struct MI_SYS_BufRef_s *pstBufRef)
{
	struct MI_SYS_InputPort_s *pstInputPort = (struct MI_SYS_InputPort_s*)pstBufRef->pCBData;

	BUG_ON(!pstInputPort ||pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
	BUG_ON(!pstInputPort->pstChannel ||pstInputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
	BUG_ON(!pstBufRef);

	if(pstInputPort->bPortEnable && pstInputPort->pstChannel->bChnEnable)
	{
		if(pstBufRef->bufinfo.bUsrBuf)
			mi_sys_add_to_queue_head(pstBufRef, &pstInputPort->stUsrInjectBufQueue);
		else
			mi_sys_add_to_queue_head(pstBufRef, &pstInputPort->stBindInputBufQueue);
	}
	else
	{
		mi_sys_release_bufref(pstBufRef);   
		while(1)
		{
			struct MI_SYS_BufRef_s *pstTmpBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stUsrInjectBufQueue);
			if(!pstTmpBufRef)
				break;
			mi_sys_release_bufref(pstTmpBufRef);   
		}
		while(1)
		{
			struct MI_SYS_BufRef_s *pstTmpBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stBindInputBufQueue);
			if(!pstTmpBufRef)
				break;
			mi_sys_release_bufref(pstTmpBufRef);   
		}
	}
	return MI_SUCCESS;
}
void _MI_SYS_IMPL_AddBufrefIntoInputUsrInjectQueue(struct MI_SYS_InputPort_s *pstInputPort, struct MI_SYS_BufRef_s *pstBufRef)
{
	BUG_ON(!pstInputPort ||pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
	BUG_ON(!pstInputPort->pstChannel ||pstInputPort->pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
	BUG_ON(!pstBufRef);

	if(pstInputPort->bPortEnable && pstInputPort->pstChannel->bChnEnable)
	{
	     mi_sys_add_to_queue_tail(pstBufRef, &pstInputPort->stUsrInjectBufQueue);
	}
	else
	{
		mi_sys_release_bufref(pstBufRef);   
		while(1)
		{
			MI_SYS_BufRef_t *pstTmpBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stUsrInjectBufQueue);
			if(!pstTmpBufRef)
				break;
			mi_sys_release_bufref(pstTmpBufRef);   
		}
	}
	return MI_SUCCESS;
}


MI_SYS_Allocator_Collection_t g_stGlobalVBPOOLAllocatorCollection;

void mi_sys_output_port_OnBufRefRelFunc(MI_SYS_BufRef_t *pstBufRef, void *pCBData)
{
}
void mi_sys_input_port_OnBufRefRelFunc(MI_SYS_BufRef_t *pstBufRef, void *pCBData)
{
}
static inline int _mi_sys_is_output_port_enabled(MI_SYS_OutputPort_t *pstOutputPort)
{
   return pstOutputPort->pstChannel->bChnEnable && pstOutputPort->bPortEnable;
}
static inline _mi_sys_is_input_port_enabled(struct MI_SYS_InputPort_s *pstInputPort)
{
   return pstInputPort->pstChannel->bChnEnable && pstInputPort->bPortEnable;
}

void mi_sys_output_UsrBuf_OnBufRefRelFunc(MI_SYS_BufRef_t *pstBufRef, void *pCBData)
{
	int val;

	MI_SYS_OutputPort_t *pstOutputPort = (MI_SYS_OutputPort_t*)pCBData;
	BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

	val = atomic_dec_return(&pstOutputPort->usrLockedBufCnt);

	BUG_ON(val<0);
}

//检测当前input port是否接收前端当前buf
int _MI_SYS_IMPL_RcAcceptBuf(struct MI_SYS_InputPort_s *pstCurInputPort)
{
	BUG_ON(!pstCurInputPort);
	BUG_ON(!pstCurInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

	if(!_mi_sys_is_input_port_enabled(pstCurInputPort))
		return 0;
	//如果当前input port与被绑定的output port的frc一样，则接收该buf
	if(pstCurInputPort->u32SrcFrmrate == pstCurInputPort->u32DstFrmrate)
		return 1;
	// 
	if(pstCurInputPort->u32CurRcParam>=pstCurInputPort->u32SrcFrmrate)
		return 1;
	return 0;
}
void _MI_SYS_IMPL_StepRc(struct MI_SYS_InputPort_s *pstCurInputPort)
{
    BUG_ON(!pstCurInputPort);

    if(pstCurInputPort->u32SrcFrmrate == pstCurInputPort->u32DstFrmrate)
         return;
    if(pstCurInputPort->u32CurRcParam >= pstCurInputPort->u32SrcFrmrate)
          pstCurInputPort->u32CurRcParam -= pstCurInputPort->u32SrcFrmrate;
    pstCurInputPort->u32CurRcParam+= pstCurInputPort->u32DstFrmrate;
}
MI_S32 _MI_SYS_IMPL_OutputPortFinishBuf(struct MI_SYS_BufRef_s *pstBufRef)
{
	struct MI_SYS_OutputPort_s *pstOutputPort = (struct MI_SYS_OutputPort_s*)pstBufRef->pCBData;
	struct MI_SYS_BufRef_s *pstNewBufRef;
	struct MI_SYS_InputPort_s *pstCurInputPort; 
	struct list_head *pos;   
	int i_valid_fifo_cnt;;

	BUG_ON(!pstOutputPort);

	mi_sys_remove_from_queue(pstBufRef, &pstOutputPort->stWorkingQueue);

    _MI_SYS_IMPL_DupBufrefIntoOutputFifoQueue(pstOutputPort, pstBufRef);


	down(&pstOutputPort->stBindedInputListSemlock);
	list_for_each(pos, &pstOutputPort->stBindPeerInputPortList)
	{
		pstCurInputPort = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
		BUG_ON(pstCurInputPort->pstBindPeerOutputPort != pstOutputPort);

		if(_MI_SYS_IMPL_RcAcceptBuf(pstCurInputPort))
		{
			 _MI_SYS_IMPL_DupBufrefIntoInputBindQueue(pstCurInputPort, pstBufRef);
		}

		_MI_SYS_IMPL_StepRc(pstCurInputPort);
	}
	up(&pstOutputPort->stBindedInputListSemlock);
	mi_sys_release_bufref(pstBufRef);
	return MI_SUCCESS;

}


MI_S32 _MI_SYS_IMPL_InputPortFinishBuf(MI_SYS_BufRef_t *pstBufRef)
{
     struct MI_SYS_InputPort_s *pstInputPort = (struct MI_SYS_InputPort_s*)pstBufRef->pCBData;
     BUG_ON(!pstInputPort);
	 
     mi_sys_remove_from_queue(pstBufRef, &pstInputPort->stWorkingQueue);
     mi_sys_release_bufref(pstBufRef);
     return MI_SUCCESS;

}

MI_S32 _MI_SYS_IMPL_OutputPortRewindBuf(MI_SYS_BufRef_t *pstBufRef)
{
     MI_SYS_OutputPort_t *pstOutputPort = (MI_SYS_OutputPort_t*)pstBufRef->pCBData;
     BUG_ON(!pstOutputPort);

     mi_sys_remove_from_queue(pstBufRef, &pstOutputPort->stWorkingQueue);
     mi_sys_release_bufref(pstBufRef);
     return MI_SUCCESS;
}




void _MI_SYS_OnAllocationFree_CB(struct MI_SYS_BufferAllocation_s *allocation, void *pUsrdata)
{
    struct MI_SYS_OutputPort_s *pstOutputPort = (struct MI_SYS_OutputPort_s *)pUsrdata;
    MI_SYS_ChnPort_t stChnPort;
	int bufInUsedCnt;
	MI_SYS_BufRef_t *pstBufRef;
	BUG_ON(!pstOutputPort);
	BUG_ON(pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
	
	BUG_ON(!pstOutputPort->pstChannel ||
		         pstOutputPort->pstChannel->u32MagicNumber!= __MI_SYS_CHN_MAGIC_NUM__);
	
	BUG_ON(!pstOutputPort->pstChannel->pstModDev ||
		         pstOutputPort->pstChannel->pstModDev->u32MagicNumber!= __MI_SYS_MAGIC_NUM__);

	bufInUsedCnt = atomic_dec_return(&pstOutputPort->totalOutputPortInUsedBuf);
    BUG_ON(bufInUsedCnt<0);

	if(pstOutputPort->pstChannel->pstModDev->ops.OnOutputPortBufRelease)
	{
	    stChnPort.eModId = pstOutputPort->pstChannel->pstModDev->eModuleId;
		stChnPort.u32DevId = pstOutputPort->pstChannel->pstModDev->u32DevId;
		stChnPort.u32ChnId = pstOutputPort->pstChannel->u32ChannelId;
		stChnPort.u32PortId = pstOutputPort-pstOutputPort->pstChannel->pastOutputPorts[0];
		BUG_ON(stChnPort.u32PortId<0 ||stChnPort.u32PortId>=MI_SYS_MAX_OUTPUT_PORT_CNT);
		pstBufRef = container_of(allocation, MI_SYS_BufRef_t , pstBufAllocation);
		pstOutputPort->pstChannel->pstModDev->ops.OnOutputPortBufRelease(&stChnPort , &pstBufRef->bufinfo);
	}
	
}

MI_SYS_BufRef_t *_MI_SYS_IMPL_AllocBufDefaultPolicy(struct MI_SYS_Channel_s *pstChannel, struct MI_SYS_BufConf_s *pstBufConfig,
                                                                                             struct MI_SYS_ChnPort_s *pstChnPort, OnBufRefRelFunc onRelCB, void *pCBData)
{
#if 0
	MI_SYS_BufferAllocation_t *pBufAllocation;
	MI_SYS_BufRef_t *pstBufRef;
	MI_SYS_MOD_DEV_t *pstModDev;

	BUG_ON(!pstChannel || pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
	BUG_ON(!pstBufConfig || !pstChnPort);

	pBufAllocation = mi_sys_alloc_from_allocator_collection(&pstChannel->stAllocatorCollection, pstBufConfig, pstChnPort);
	if(!pBufAllocation)
	{
		pstModDev = pstChannel->pstModDev;
		BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
		pBufAllocation = mi_sys_alloc_from_allocator_collection(&pstModDev->stAllocatorCollection, pstBufConfig, pstChnPort);
	}
	if(!pBufAllocation)
	{
		pBufAllocation = mi_sys_alloc_from_allocator_collection(&g_stGlobalVBPOOLAllocatorCollection, pstBufConfig, pstChnPort);
	}   
	if(!pBufAllocation)
	{
		pBufAllocation = mi_sys_alloc_from_mma_allocators(NULL, pstBufConfig, pstChnPort);
	}
	if(!pBufAllocation)
		return NULL;
	pstBufRef = mi_sys_create_bufref(pBufAllocation, onRelCB, pCBData);
	if(pstBufRef == NULL)
	{
		pBufAllocation->ops->OnRelease(pBufAllocation);
		return NULL;
	}
	return pstBufRef;
	#endif
	return NULL;
} 



struct MI_SYS_BufInfo_s *_MI_SYS_IMPL_UserGetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct MI_SYS_BufConf_s *pstBufConfig)
{

	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
	MI_SYS_Channel_t *pstChannel;
	struct MI_SYS_InputPort_s *pstInputPort;
	MI_SYS_BufferAllocation_t *pBufAllocation;
	MI_SYS_ChnPort_t stChnPort;
	MI_SYS_BufRef_t *pstBufRef;

	BUG_ON(!pstModDev);
	BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
	BUG_ON(!pstBufConfig);

	BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
	BUG_ON(u32PortId >=pstModDev->u32OutputPortNum);

	pstChannel = &pstModDev->astChannels[u32ChnId];

	BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

	pstInputPort = pstChannel->pastInputPorts[u32PortId];
	BUG_ON(!pstInputPort || pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

	if(!pstChannel->bChnEnable || !pstInputPort->bPortEnable)
		return NULL;

	stChnPort.eModId = pstModDev->eModuleId;
	stChnPort.u32DevId = pstModDev->u32DevId;
	stChnPort.u32ChnId = u32ChnId;
	stChnPort.u32PortId = u32PortId;

	if( pstInputPort->pstCusBufAllocator)
	{
		pBufAllocation = pstInputPort->pstCusBufAllocator->ops->alloc(pstInputPort->pstCusBufAllocator, pstBufConfig, &stChnPort);
		if(!pBufAllocation)
			return NULL;
		pstBufRef = mi_sys_create_bufref(pBufAllocation, pstBufConfig ,mi_sys_input_port_OnBufRefRelFunc,pstInputPort);
		if(pstBufRef == NULL)
		{
			pstInputPort->pstCusBufAllocator->ops->OnRelease(pstInputPort->pstCusBufAllocator);
			return NULL;
		}
		return &pstBufRef->bufinfo;
	   
	}

	pstBufRef = _MI_SYS_IMPL_AllocBufDefaultPolicy(pstChannel, pstBufConfig, &stChnPort, mi_sys_input_port_OnBufRefRelFunc, pstInputPort);

	if(pstBufRef)
	{
		mi_sys_add_to_queue_tail(pstBufRef, &pstInputPort->stWorkingQueue);
		return &pstBufRef->bufinfo;
	}
	return NULL;
}

MI_SYS_BufRef_t *_MI_SYS_IMPL_UserGetOutputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId)
{

	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
	MI_SYS_Channel_t *pstChannel;
	MI_SYS_OutputPort_t *pstOutputPort;
	MI_SYS_BufRef_t *pstBufRef;
	
	BUG_ON(!pstModDev);
	BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

	BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
	BUG_ON(u32PortId >=pstModDev->u32OutputPortNum);

	pstChannel = &pstModDev->astChannels[u32ChnId];

	BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);
	
	pstOutputPort = pstChannel->pastOutputPorts[u32PortId];
	BUG_ON(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);
	if(!pstChannel->bChnEnable || !pstOutputPort->bPortEnable)
		return NULL;
	pstBufRef = mi_sys_remove_from_queue_head(&pstOutputPort->stUsrGetFifoBufQueue);
	if(pstBufRef)
	{
	     BUG_ON(pstBufRef->onRelCB);
		 
	     pstBufRef->onRelCB = mi_sys_output_UsrBuf_OnBufRefRelFunc;
		 pstBufRef->pCBData = pstOutputPort;
		 atomic_inc(pstOutputPort->u32UsrFifoCount);
	}
	return pstBufRef;

}


struct MI_SYS_InputPort_s *_MI_SYS_IMPL_GetInputPortInfo(MI_SYS_ChnPort_t *pstChnPort)
{
	MI_SYS_MOD_DEV_t *pstModDev;
	struct list_head *pos;

	list_for_each(pos, &mi_sys_global_dev_list)
	{	
		pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
		BUG_ON(!pstModDev);
		BUG_ON(pstModDev->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
		BUG_ON(pstModDev->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);
		BUG_ON(pstModDev->u32InputPortNum ==0 && pstModDev->u32OutputPortNum ==0);
		BUG_ON(pstModDev->u32DevChnNum ==0);
		BUG_ON(pstModDev->u32DevChnNum > MI_SYS_MAX_DEV_CHN_CNT);

		if(pstModDev->eModuleId == pstChnPort->eModId && pstModDev->u32DevId == pstChnPort->u32DevId)
		{
			return pstModDev->astChannels[pstChnPort->u32ChnId].pastInputPorts[pstChnPort->u32PortId];
		}
	}
	return NULL;
}

MI_SYS_OutputPort_t *_MI_SYS_IMPL_GetOutputPortInfo(MI_SYS_ChnPort_t *pstChanPort)
{

	MI_SYS_MOD_DEV_t *pstModDev;
	struct list_head *pos;
	
	list_for_each(pos, &mi_sys_global_dev_list)
	{	
		pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
		BUG_ON(!pstModDev);
		BUG_ON(pstModDev->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
		BUG_ON(pstModDev->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);
		BUG_ON(pstModDev->u32InputPortNum ==0 && pstModDev->u32OutputPortNum ==0);
		BUG_ON(pstModDev->u32DevChnNum ==0);
		BUG_ON(pstModDev->u32DevChnNum > MI_SYS_MAX_DEV_CHN_CNT);
		
		if(pstModDev->eModuleId == pstChanPort->eModId && pstModDev->u32DevId == pstChanPort->u32DevId)
		{
			return pstModDev->astChannels[pstChanPort->u32ChnId].pastOutputPorts[pstChanPort->u32PortId];
		}
	}
	return NULL;
}





//////////////////////////////////////////////
////////////public api//////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

MI_S32 MI_SYS_IMPL_Init(void)
{
	idr_init(&g_mi_sys_buf_handle_idr);
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_Exit()
{
	idr_destroy(&g_mi_sys_buf_handle_idr);
	return MI_SUCCESS;
}



MI_S32 MI_SYS_IMPL_BindChnPort(struct MI_SYS_ChnPort_s *pstSrcInputPort, struct MI_SYS_ChnPort_s *pstDstOutputPort ,MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate)
{
	struct MI_SYS_InputPort_s *pstInputPort = _MI_SYS_IMPL_GetInputPortInfo(pstSrcInputPort);
	if(!pstInputPort)
	{
		BUG();
	}
	MI_SYS_OutputPort_t *pstOutputPort = _MI_SYS_IMPL_GetInputPortInfo(pstDstOutputPort);
	if(!pstOutputPort)
	{
		BUG();
	}
	return _MI_SYS_IMPL_BindChannelPort(pstInputPort , pstOutputPort , u32SrcFrmrate , u32DstFrmrate);
}

MI_S32 MI_SYS_IMPL_UnBindChnPort(struct MI_SYS_ChnPort_s *pstSrcInputPort, struct MI_SYS_ChnPort_s *pstDstOutputPort)
{
	struct MI_SYS_InputPort_s *pstInputPort = _MI_SYS_IMPL_GetInputPortInfo(pstSrcInputPort);
	if(!pstInputPort)
	{
		BUG();
	}
	MI_SYS_OutputPort_t *pstOutputPort = _MI_SYS_IMPL_GetInputPortInfo(pstDstOutputPort);
	if(!pstOutputPort)
	{
		BUG();
	}
	return _MI_SYS_IMPL_UnBindChannelPort(pstInputPort , pstOutputPort);

}



MI_SYS_DRV_HANDLE  MI_SYS_IMPL_RegisterDev(struct mi_sys_ModuleDevInfo_s *pstMouleInfo, struct mi_sys_ModuleDevBindOps_s *pstModuleBindOps , void *pUsrDatas)
{
	int size;
    struct list_head *pos;
    MI_SYS_MOD_DEV_t *pListCurDev;
    MI_SYS_MOD_DEV_t *pstModDev;
    unsigned long flags;
    MI_U32 i;
    //检查参数
    BUG_ON(!pstMouleInfo);
    BUG_ON(pstMouleInfo->u32InputPortNum > MI_SYS_MAX_INPUT_PORT_CNT);
    BUG_ON(pstMouleInfo->u32OutputPortNum > MI_SYS_MAX_OUTPUT_PORT_CNT);
    BUG_ON(pstMouleInfo->u32InputPortNum ==0 && pstMouleInfo->u32OutputPortNum ==0);
    BUG_ON(pstMouleInfo->u32DevChnNum ==0);
    BUG_ON(pstMouleInfo->u32DevChnNum > MI_SYS_MAX_DEV_CHN_CNT);
    
    size = sizeof(MI_SYS_MOD_DEV_t)+sizeof(MI_SYS_Channel_t)*pstMouleInfo->u32DevChnNum;

    pstModDev = kmalloc(size , GFP_KERNEL);

    if(!pstModDev)
        return NULL;

    pstModDev->u32InputPortNum =pstMouleInfo->u32InputPortNum;
    pstModDev->u32OutputPortNum =pstMouleInfo->u32OutputPortNum;
    pstModDev->u32DevChnNum =pstMouleInfo->u32DevChnNum;
    pstModDev->u32DevId =pstMouleInfo->u32DevId;
    pstModDev->eModuleId =pstMouleInfo->eModuleId;
    pstModDev->u32MagicNumber = __MI_SYS_MAGIC_NUM__;
	
    if(pstModuleBindOps)
       memcpy(&pstModDev->ops , pstModuleBindOps, sizeof(pstModDev->ops));
    else
       memset(&pstModDev->ops , 0, sizeof(pstModDev->ops));  
    pstModDev->pUsrData = pUsrDatas;

    //创建channel
    for( i=0; i<pstModDev->u32DevChnNum; i++)
    {
    	 pstModDev->astChannels[i].u32ChannelId = i;
         if(MI_SUCCESS != _MI_SYS_IMPL_InitChannel(&pstModDev->astChannels[i], pstModDev))
              break;
    }

    //如果channel创建失败，直接打印错误信息
    if( i != pstModDev->u32DevChnNum)
    {
          BUG();
    }
    
    mi_sys_init_allocator_collection(&pstModDev->stAllocatorCollection);
    
    spin_lock_irqsave(&mi_sys_global_dev_list_lock, flags);
    //检查当前设备是否已注册过
    list_for_each(pos, &mi_sys_global_dev_list)
    {
          if(pListCurDev->eModuleId == pstModDev->eModuleId &&
                  pListCurDev->u32DevId == pstModDev->u32DevId)
                  BUG();
    }
    list_add_tail(&pstModDev->listModDev, &mi_sys_global_dev_list);
    spin_unlock_irqrestore(&mi_sys_global_dev_list_lock, flags);
	return (MI_SYS_DRV_HANDLE)pstModDev;
}



MI_S32 MI_SYS_IMPL_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle)
{
	MI_U32 i;
	unsigned long flags;
	struct list_head *pos;
	int find = 0;
	
	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;

	BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

	spin_lock_irqsave(&mi_sys_global_dev_list_lock, flags);

	list_for_each(pos, &mi_sys_global_dev_list)
	{
		if(pos == &pstModDev->listModDev)
		{
			find = 1;
			break;
		}
	}
	BUG_ON(!find);
	list_del(&pstModDev->listModDev);
	spin_unlock_irqrestore(&mi_sys_global_dev_list_lock, flags);    

	for( i=0; i<pstModDev->u32DevChnNum; i++)
	{
		_MI_SYS_IMPL_DeinitChannel(&pstModDev->astChannels[i]);
	}
	mi_sys_deinit_allocator_collection(&pstModDev->stAllocatorCollection);
	//for bug check purpose
	memset(pstModDev, 0xE4, sizeof(MI_SYS_MOD_DEV_t));

	kfree(pstModDev);

	return MI_SUCCESS;

}

MI_S32 MI_SYS_IMPL_GetFd(MI_SYS_ChnPort_t *pstChnPort)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_CloseFd(MI_SYS_ChnPort_t *pstChnPort)
{
	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_SetChnOutPortDepth(struct MI_SYS_ChnPort_s *pstChnPort , MI_U32 u32UserFrameDepth , MI_U32 u32BufQueueDepth)
{
	BUG_ON(!pstChnPort);
	struct MI_SYS_OutputPort_s *pstOutputPort = _MI_SYS_IMPL_GetOutputPortInfo(pstChnPort);
	pstOutputPort->u32UsrFifoCount= u32UserFrameDepth;
	pstOutputPort->u32OutputPortBufCntQuota = u32BufQueueDepth;
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_UserOutputPortPutBuf (MI_SYS_BUF_HANDLE handle)
{
    MI_SYS_BufHandleIdrData_t *pstHandleData;
	struct MI_SYS_InputPort_s *pstInputPort = NULL;

	pstHandleData = idr_find(&g_mi_sys_buf_handle_idr, handle);
	if(!pstHandleData)
		return E_MI_ERR_ILLEGAL_PARAM;
	if(pstHandleData->eBufType != E_MI_SYS_IDR_BUF_TYPE_OUTPUT_PORT)
		 return E_MI_ERR_ILLEGAL_PARAM;

	mi_sys_release_bufref(pstHandleData->pstBufRef);
	memset(pstHandleData,0xAA, sizeof(*pstHandleData));
    kfree(pstHandleData);
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_UserGetOutputPortBuf(struct MI_SYS_ChnPort_s *pstChnPort, struct MI_SYS_BufInfo_s *pstBufInfo, MI_SYS_BUF_HANDLE *pBufHandle)
{
	struct MI_SYS_BufHandleIdrData_s *pstHandleData;
	struct MI_SYS_BufRef_s *pstBufRef;
	struct MI_SYS_MOD_DEV_s *pstModDev;
	struct list_head *pos;

	pstHandleData = (struct MI_SYS_BufHandleIdrData_s*)kmalloc(sizeof(MI_SYS_BufHandleIdrData_t) ,GFP_KERNEL);
	if(pstHandleData == NULL)
		return E_MI_ERR_NOMEM;

	list_for_each(pos, &mi_sys_global_dev_list)
	{	
		pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
		if(pstModDev->eModuleId == pstChnPort->eModId && pstModDev->u32DevId == pstChnPort->u32DevId)
		{
			break;
		}
	}
	BUG_ON(!pstModDev);
	
	pstBufRef = _MI_SYS_IMPL_UserGetOutputPortBuf((MI_SYS_DRV_HANDLE)pstModDev, pstChnPort->u32ChnId, pstChnPort->u32PortId);
	if(!pstBufRef)
	{
		kfree(pstHandleData);
		return E_MI_ERR_NOBUF;	
	}
	pstHandleData->eBufType = E_MI_SYS_IDR_BUF_TYPE_OUTPUT_PORT;
	pstHandleData->pstBufRef = pstBufRef;
	memcpy(&pstHandleData->stChnPort , pstChnPort , sizeof(MI_SYS_ChnPort_t));
	pstBufInfo = &pstBufRef->bufinfo;
	if(idr_get_new(&g_mi_sys_buf_handle_idr, pstHandleData, pBufHandle)<0)
	{
		kfree(pstHandleData);
		mi_sys_release_bufref(pstBufRef);
		return MI_SYS_NO_HANDLE;
	}
	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_OutputBufToInputBufQueue (MI_SYS_BUF_HANDLE handle , struct MI_SYS_ChnPort_s *pstChnPort)
{
    MI_SYS_BufHandleIdrData_t *pstHandleData;
	struct list_head *pos;
	struct MI_SYS_MOD_DEV_s *pstModDev;
	struct MI_SYS_InputPort_s *pstInputPort;
	struct MI_SYS_BufRef_s *pstTmp;
	pstHandleData = idr_find(&g_mi_sys_buf_handle_idr, handle);
	if(!pstHandleData)
		return E_MI_ERR_ILLEGAL_PARAM;
	if(pstHandleData->eBufType!= E_MI_SYS_IDR_BUF_TYPE_OUTPUT_PORT)
		return E_MI_ERR_ILLEGAL_PARAM;
	
	pstTmp = mi_sys_dup_bufref(pstHandleData->pstBufRef, NULL,NULL);
	if(!pstTmp)
		return E_MI_ERR_NOMEM;
	
	idr_remove(&g_mi_sys_buf_handle_idr, handle);
	
	// get input port
	list_for_each(pos, &mi_sys_global_dev_list)
	{	
		 pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
		 BUG_ON(!pstModDev);
		 if(pstModDev->eModuleId == pstChnPort->eModId && pstChnPort->u32DevId == pstModDev->u32DevId)
		 {
		 	pstInputPort = pstModDev->astChannels[pstHandleData->stChnPort.u32ChnId].pastInputPorts[pstHandleData->stChnPort.u32PortId];
			break;
		 }
	}
	BUG_ON(!pstInputPort);

	mi_sys_add_to_queue_tail(&pstInputPort->stUsrInjectBufQueue,
	            pstHandleData->pstBufRef);
	
	wake_up(&pstInputPort->pstChannel->pstModDev->inputWaitqueue);
	mi_sys_release_bufref(pstHandleData->pstBufRef);
	memset(pstHandleData,0xAA, sizeof(*pstHandleData));
    kfree(pstHandleData);
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_UserInputPortPutBuf (MI_SYS_BUF_HANDLE handle)
{
    MI_SYS_BufHandleIdrData_t *pstHandleData;
	struct list_head *pos;
	struct MI_SYS_MOD_DEV_s *pstModDev;
	struct MI_SYS_InputPort_s *pstInputPort;
	pstHandleData = idr_find(&g_mi_sys_buf_handle_idr, handle);
	if(!pstHandleData)
		return E_MI_ERR_ILLEGAL_PARAM;
	if(pstHandleData->eBufType!= E_MI_SYS_IDR_BUF_TYPE_INPUT_PORT)
		return E_MI_ERR_ILLEGAL_PARAM;

	list_for_each(pos, &mi_sys_global_dev_list)
	{	
		 pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
		 BUG_ON(!pstModDev);
		 if(pstModDev->eModuleId == pstHandleData->stChnPort.eModId && pstHandleData->stChnPort.u32DevId == pstModDev->u32DevId)
		 {
		 	pstInputPort = pstModDev->astChannels[pstHandleData->stChnPort.u32ChnId].pastInputPorts[pstHandleData->stChnPort.u32PortId];
			break;
		 }
	}
	BUG_ON(!pstHandleData);

	idr_remove(&g_mi_sys_buf_handle_idr, handle);
	
	_MI_SYS_IMPL_AddBufrefIntoInputUsrInjectQueue(pstInputPort , pstHandleData->pstBufRef);

	wake_up(&pstInputPort->pstChannel->pstModDev->inputWaitqueue);
	
	memset(pstHandleData,0xAA, sizeof(*pstHandleData));
    kfree(pstHandleData);
	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_UserGetInputPortBuf(struct MI_SYS_ChnPort_s *pstChnPort, struct MI_SYS_BufConf_s *pstBufConfig , struct MI_SYS_BufInfo_s *pstBufInfo ,MI_SYS_BUF_HANDLE *pBufHandle)
{
	struct MI_SYS_BufHandleIdrData_s *pstHandleData;
	struct MI_SYS_BufRef_s *pstBufRef;
	struct MI_SYS_MOD_DEV_s *pstModDev;
    struct list_head *pos;

	pstHandleData = (MI_SYS_BufHandleIdrData_t*)kmalloc(sizeof(MI_SYS_BufHandleIdrData_t) ,GFP_KERNEL);
	if(pstHandleData == NULL)
		return E_MI_ERR_NOMEM;

	list_for_each(pos, &mi_sys_global_dev_list)
	{	
		 pstModDev = container_of(pos, MI_SYS_MOD_DEV_t, listModDev);
		 if(pstModDev->eModuleId == pstChnPort->eModId && pstModDev->u32DevId == pstChnPort->u32DevId)
		 {
			break;
		 }
	}
	BUG_ON(!pstModDev);
		
	pstBufRef = _MI_SYS_IMPL_UserGetInputPortBuf((MI_SYS_DRV_HANDLE)pstModDev, pstChnPort->u32ChnId, pstChnPort->u32PortId, pstBufConfig);
	if(!pstBufRef)
	{
		kfree(pstHandleData);
		return E_MI_ERR_NOMEM;
	}
	pstHandleData->eBufType= E_MI_SYS_IDR_BUF_TYPE_INPUT_PORT;
	pstHandleData->pstBufRef = pstBufRef;
	pstHandleData->stChnPort = *pstChnPort;
	pstBufInfo = &pstBufRef->bufinfo;
	if(idr_get_new(&g_mi_sys_buf_handle_idr, pstHandleData, pBufHandle)<0)
	{
		kfree(pstHandleData);
		mi_sys_release_bufref(pstBufRef );
		return MI_SYS_NO_HANDLE;
	}
	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_SetChnOutputFrcCtrl(struct MI_SYS_ChnPort_s *pstChnPort, MI_U32 u32FrcCtrlNumerator , MI_U32 u32FrcCtrlDenominator)
{
	MI_SYS_OutputPort_t *pstOutPort = _MI_SYS_IMPL_GetOutputPortInfo(pstChnPort);
	struct MI_SYS_InputPort_s *pstInputPort;
	struct list_head *pos;
	down(&pstOutPort->stBindedInputListSemlock);
	list_for_each(pos, &pstOutPort->stBindPeerInputPortList)
	{
		pstInputPort = container_of(pos, struct MI_SYS_InputPort_s,  stBindRelationShipList);
		BUG_ON(pstInputPort->pstBindPeerOutputPort != pstOutPort);
		pstInputPort->u32DstFrmrate = u32FrcCtrlNumerator;
		pstInputPort->u32SrcFrmrate = u32FrcCtrlDenominator;
	}
	up(&pstOutPort->stBindedInputListSemlock);
	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_GetBindbyDest (struct MI_SYS_ChnPort_s *pstDstChnPort, struct MI_SYS_ChnPort_s *pstSrcChnPort)
{

	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_GetVersion (struct MI_SYS_Version_s *pstVersion)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_GetCurPts (MI_U64 *pu64Pts)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_InitPtsBase (MI_U64 u64PtsBase)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_SyncPts (MI_U64 u64Pts)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_Mmap(MI_PHY phyAddr, MI_U32 u32Size , void *pVirtualAddress)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_Munmap(void *pVirtualAddress, MI_U32 u32Size)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_SetReg (MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_GetReg (MI_U32 u32RegAddr, MI_U16 *pu16Value)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_SetChnPortMMAConf (struct MI_SYS_ChnPort_s *pstChnPort,MI_U8 *pu8MMAHeapName)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_GetChnPortMMAConf (struct MI_SYS_ChnPort_s *pstChnPort,void  *data, MI_U32 u32Length)
{
	return MI_SUCCESS;
}




//////////////////////////////////////////////
////////////////internal API////////////////////

MI_S32 MI_SYS_IMPL_DevTaskIterator(MI_SYS_DRV_HANDLE miSysDrvHandle, mi_sys_TaskIteratorCallBack pfCallBack,void *pUsrData)
{
	struct mi_sys_ChnTaskInfo_s *pstTask;
    MI_U32 u32ChannelIdx = 0 , u32PortId = 0;
	struct MI_SYS_InputPort_s *pstInputPort = NULL;
	struct MI_SYS_BufRef_s* stBufRef = NULL;
	struct MI_SYS_MOD_DEV_s *pstModDev = (MI_SYS_MOD_DEV_t *)miSysDrvHandle;
    struct list_head to_free_list;
	struct list_head *pos;
	DECLARE_BITMAP(chn_mask, MI_SYS_MAX_DEV_CHN_CNT);
    int accept_one;
	MI_S32 ret;
	INIT_LIST_HEAD(&to_free_list);
	BUG_ON(!pstModDev
		 || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
	BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
SCAN_AGAIN:  
	
	accept_one = 0;
	pstTask = NULL;

	//check all channels
	for(u32ChannelIdx = 0 ; u32ChannelIdx < pstModDev->u32DevChnNum;u32ChannelIdx++)
	{
	    int has_input_data = 0;
		//skip the current channel?
		if(test_bit(u32ChannelIdx , chn_mask))
	   	    continue;
	    if(!pstTask)
		   pstTask = (mi_sys_ChnTaskInfo_t *)kmem_cache_alloc(mi_sys_taskinfo_cachep, GFP_KERNEL);
		if(!pstTask)
	    {
	        goto free_rewind_list;
		}
		
		memset(pstTask, 0, sizeof(*pstTask));
		//Get a buf from each input port
		for(u32PortId = 0 ; u32PortId < pstModDev->u32InputPortNum ; u32PortId ++)
		{
		    pstInputPort = pstModDev->astChannels[u32ChannelIdx].pastInputPorts[u32PortId];

			pstTask->astInputPortBufInfo[u32PortId] = MI_SYS_IMPL_GetInputPortBuf(miSysDrvHandle, u32ChannelIdx,u32PortId);
			if(pstTask->astInputPortBufInfo[u32PortId])
				 has_input_data = 1;
		}
		if(has_input_data == 0)
			continue;
		
		ret = pfCallBack(pstTask , pUsrData);

		switch((mi_sys_TaskIteratorCBAction_e)ret)
		{
			case MI_SYS_ITERATOR_ACCEPT_CONTINUTE:
				accept_one = 1;
				break;
			case MI_SYS_ITERATOR_SKIP_CONTINUTE: //当前channel buf ,module不需要，从下个channle查询	
                __set_bit(u32ChannelIdx , chn_mask);
				list_add_tail(&pstTask->listChnTask, &to_free_list);
				break;
			case MI_SYS_ITERATOR_ACCEPT_STOP:
				goto free_rewind_list;
				return MI_SUCCESS;
			case MI_SYS_ITERATOR_SKIP_STOP:
				list_add_tail(&pstTask->listChnTask, &to_free_list);
				goto free_rewind_list;
			default:
					BUG();
		}
	}

   if(accept_one)
   	 goto SCAN_AGAIN;
free_rewind_list:

	list_for_each(pos,&to_free_list)
	{
	    mi_sys_ChnTaskInfo_t *pTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
		list_del(&pTask->listChnTask);
		mi_sys_RewindTask(pstTask);
	}
	return MI_SUCCESS;
}


MI_SYS_BufInfo_t *MI_SYS_IMPL_GetOutputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, 
                                            MI_U32 u32PortId, struct MI_SYS_BufConf_s *pstBufConfig, MI_BOOL *bBlockedByRateCtrl)
{

	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
	MI_SYS_Channel_t *pstChannel;
	MI_SYS_OutputPort_t *pstOutputPort;
	int i_valid_fifo_cnt;
	struct list_head *pos;
	MI_SYS_BufferAllocation_t *pBufAllocation;
	MI_SYS_ChnPort_t stChnPort;
	MI_SYS_BufRef_t *pstBufRef;


	BUG_ON(!pstModDev);
	BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
	BUG_ON(!pstBufConfig);

	BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
	BUG_ON(u32PortId >=pstModDev->u32OutputPortNum);

	pstChannel = &pstModDev->astChannels[u32ChnId];

	BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

	pstOutputPort = pstChannel->pastOutputPorts[u32PortId];
	BUG_ON(!pstOutputPort || pstOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

	//Check whether the channel or port is enabled
	if(!pstChannel->bChnEnable || !pstOutputPort->bPortEnable)
	{
		return NULL;
	}
	i_valid_fifo_cnt = (int)pstOutputPort->u32UsrFifoCount - atomic_read(&pstOutputPort->usrLockedBufCnt);

	BUG_ON(i_valid_fifo_cnt < 0);
	BUG_ON(pstOutputPort->u32OutputPortBufCntQuota == 0);

	if(atomic_read(&pstOutputPort->totalOutputPortInUsedBuf) >= pstOutputPort->u32OutputPortBufCntQuota)
	{
		return NULL;
	}

	if(i_valid_fifo_cnt)
		goto alloc_buf;

	down(&pstOutputPort->stBindedInputListSemlock);
	list_for_each(pos, &pstOutputPort->stBindPeerInputPortList)
	{ 
		struct MI_SYS_InputPort_s *pstCurInputPort = container_of(pos, struct MI_SYS_InputPort_s, stBindRelationShipList);
		BUG_ON(pstCurInputPort->pstBindPeerOutputPort != pstOutputPort);

		if(_MI_SYS_IMPL_RcAcceptBuf(pstCurInputPort))
		{ 
		    up(&pstOutputPort->stBindedInputListSemlock);
		    goto alloc_buf;
		}
	}
	up(&pstOutputPort->stBindedInputListSemlock); 
	if(bBlockedByRateCtrl)
	  *bBlockedByRateCtrl = TRUE;
	return NULL;

alloc_buf:
	if(bBlockedByRateCtrl)
	  *bBlockedByRateCtrl = FALSE;

	stChnPort.eModId = pstModDev->eModuleId;
	stChnPort.u32DevId = pstModDev->u32DevId;
	stChnPort.u32ChnId = u32ChnId;
	stChnPort.u32PortId = u32PortId;

	if( pstOutputPort->pstCusBufAllocator)
	{
	   pBufAllocation = pstOutputPort->pstCusBufAllocator->ops->alloc(pstOutputPort->pstCusBufAllocator, pstBufConfig, &stChnPort);
	   if(!pBufAllocation)
	        return NULL;
	   pstBufRef = mi_sys_create_bufref(pBufAllocation, pstBufConfig ,mi_sys_output_port_OnBufRefRelFunc, pstOutputPort);
	   if(pstBufRef == NULL)
	    {
	        pstOutputPort->pstCusBufAllocator->ops->OnRelease(pstOutputPort->pstCusBufAllocator);
	        return NULL;
	    }
	   mi_sys_add_to_queue_tail(pstBufRef, &pstOutputPort->stWorkingQueue);
	   BUG_ON(pstBufRef->pstBufAllocation->OnFreeNotifyCB);
	   pstBufRef->pstBufAllocation->OnFreeNotifyCB = _MI_SYS_OnAllocationFree_CB;
	   atomic_inc(&pstOutputPort->totalOutputPortInUsedBuf);
	   return &pstBufRef->bufinfo;
	   
	}
	//if(pstModDev->ops.OnOutputPortBufRelease)
	//	pstBufRef =  _MI_SYS_IMPL_AllocBufDefaultPolicy(pstChannel, pstBufConfig, &stChnPort, pstModDev->ops.OnOutputPortBufRelease, pstOutputPort);
	//else
	pstBufRef =  _MI_SYS_IMPL_AllocBufDefaultPolicy(pstChannel, pstBufConfig, &stChnPort, mi_sys_output_port_OnBufRefRelFunc, pstOutputPort);
	if(pstBufRef)
	    mi_sys_add_to_queue_tail(pstBufRef, &pstOutputPort->stWorkingQueue);

	if(pstBufRef)
	{
		BUG_ON(pstBufRef->pstBufAllocation->OnFreeNotifyCB);
		pstBufRef->pstBufAllocation->OnFreeNotifyCB = _MI_SYS_OnAllocationFree_CB;
		 atomic_inc(&pstOutputPort->totalOutputPortInUsedBuf);
		return &pstBufRef->bufinfo;
	}
	return NULL;
}


struct MI_SYS_BufInfo_s *MI_SYS_IMPL_GetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, 
                                            MI_U32 u32PortId)
{

	struct MI_SYS_MOD_DEV_s *pstModDev = (struct MI_SYS_MOD_DEV_s *)miSysDrvHandle;
	struct MI_SYS_Channel_s *pstChannel = NULL;
	struct MI_SYS_InputPort_s *pstInputPort = NULL;
	struct MI_SYS_BufRef_s *pstBufRef = NULL;
	struct MI_SYS_BufferQueue_s *pstBufferQue = NULL;

	BUG_ON(!pstModDev);
	BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

	BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
	BUG_ON(u32PortId >=pstModDev->u32InputPortNum);

	pstChannel = &pstModDev->astChannels[u32ChnId];

	BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

	pstInputPort = pstChannel->pastOutputPorts[u32PortId];
	BUG_ON(!pstInputPort || pstInputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);

	if(!pstChannel->bChnEnable || !pstInputPort->bPortEnable)
	{
		return NULL;
	}
	if(pstInputPort->stBindInputBufQueue.queue_buf_count > 0)
	{
		pstBufferQue = &pstInputPort->stBindInputBufQueue;
	}
	else if(pstInputPort->stUsrInjectBufQueue.queue_buf_count > 0)
	{
		pstBufferQue = &pstInputPort->stUsrInjectBufQueue;
	}
	else
	{
		pstBufferQue = NULL;
	}

	if(pstBufferQue)
	{
		pstBufRef = mi_sys_remove_from_queue_head(pstBufferQue);
		if(!pstBufRef)
			BUG();
		mi_sys_add_to_queue_tail(pstBufRef , &pstInputPort->stWorkingQueue);
	}

	return &pstBufRef->bufinfo;
}


MI_S32 MI_SYS_IMPL_PrepareTaskOutputBuf(struct mi_sys_ChnTaskInfo_s *pstTask)
{
	struct MI_SYS_MOD_DEV_s *pstModDev = (struct MI_SYS_MOD_DEV_s *)(pstTask->miSysDrvHandle);
	struct MI_SYS_InputPort_s *pstInputPort = NULL;
	struct MI_SYS_OutputPort_s *pstOutputPort = NULL;
	struct MI_SYS_BufInfo_s *pstBufInfo = NULL;
	struct MI_SYS_Channel_s *pstChannel = &pstModDev->astChannels[pstTask->u32ChnId];
	MI_BOOL bBlockedByRateCtrl = FALSE;
	int portid = 0;
	
	BUG_ON(!pstModDev
		 || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
	BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);

	for( portid=0;portid<pstModDev->u32OutputPortNum;portid++)
	{
		pstOutputPort = pstChannel->pastOutputPorts[portid];

		BUG_ON(pstTask->astOutputPortBufInfo[portid]);
		
        if(!mi_sys_buf_mgr_check_bufinfo(NULL, &pstTask->astOutputPortPerfBufConfig[portid]))
		       continue;
		
	    pstTask->astOutputPortBufInfo[portid]  = MI_SYS_IMPL_GetOutputPortBuf(pstTask->miSysDrvHandle, pstTask->u32ChnId, 
		portid, &pstTask->astOutputPortPerfBufConfig[portid], &bBlockedByRateCtrl);	
		pstTask->astOutputPortBufInfo[portid] = pstBufInfo;
		pstTask->bOutputPortMaskedByFrmrateCtrl[portid] = bBlockedByRateCtrl;
	}
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_FinishBuf(MI_SYS_BufInfo_t *buf)
{
    MI_SYS_BufRef_t *pstBufRef = container_of(buf, MI_SYS_BufRef_t, bufinfo);
    if(pstBufRef->onRelCB == mi_sys_output_port_OnBufRefRelFunc)
    {
         return _MI_SYS_IMPL_OutputPortFinishBuf(pstBufRef);
    }
     else if(pstBufRef->onRelCB == mi_sys_input_port_OnBufRefRelFunc)
    {
         return _MI_SYS_IMPL_InputPortFinishBuf(pstBufRef);
    }   
    else
         BUG();

    return MI_SYS_INVALIDPARAM;
}

MI_S32 MI_SYS_IMPL_RewindBuf(struct MI_SYS_BufInfo_s*buf)
{
    MI_SYS_BufRef_t *pstBufRef = container_of(buf, MI_SYS_BufRef_t, bufinfo);
    if(pstBufRef->onRelCB == mi_sys_output_port_OnBufRefRelFunc)
    {
         return _MI_SYS_IMPL_OutputPortRewindBuf(pstBufRef);
    }
     else if(pstBufRef->onRelCB == mi_sys_input_port_OnBufRefRelFunc)
    {
         return _MI_SYS_IMPL_InputPortRewindBuf(pstBufRef);
    }   
    else
         BUG();

    return MI_SYS_INVALIDPARAM;
}



MI_S32 MI_SYS_IMPL_FinishAndReleaseTask(struct mi_sys_ChnTaskInfo_s *pstTask)
{
	int i = 0;
	MI_S32 ret = 0;
    for (i = 0 ; i < MI_SYS_MAX_INPUT_PORT_CNT ; i ++)
	{
	    if(!pstTask->astInputPortBufInfo[i])
			continue;

		ret = MI_SYS_IMPL_FinishAndReleaseBuf(pstTask->astInputPortBufInfo[i]);
		if(ret != MI_SUCCESS)
			BUG();
	}
	for (i = 0 ; i < MI_SYS_MAX_OUTPUT_PORT_CNT ; i ++)
	{
		if(!pstTask->astOutputPortBufInfo[i])
			continue;
		ret = MI_SYS_IMPL_FinishAndReleaseBuf(pstTask->astOutputPortBufInfo[i]);
	    if(ret != MI_SUCCESS)
			BUG();
    }
	
	memset(pstTask, 0xA8, sizeof(*pstTask));
	
	kmem_cache_free(mi_sys_taskinfo_cachep,pstTask);

	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_RewindTask(struct mi_sys_ChnTaskInfo_s *pstTask)
{
	int i = 0;
	MI_S32 ret = 0;

    for (i = 0 ; i < MI_SYS_MAX_INPUT_PORT_CNT ; i ++)
	{
	    if(!pstTask->astInputPortBufInfo[i])
			continue;
		ret = MI_SYS_IMPL_RewindBuf(pstTask->astInputPortBufInfo[i]);
		if(ret != MI_SUCCESS)
			BUG();
	}
	for (i = 0 ; i < MI_SYS_MAX_OUTPUT_PORT_CNT ; i ++)
	{
		if(!pstTask->astOutputPortBufInfo[i])
			continue;
		ret = MI_SYS_IMPL_RewindBuf(pstTask->astOutputPortBufInfo[i]);
	    if(ret != MI_SUCCESS)
			BUG();
    }

	memset(pstTask, 0xA7, sizeof(*pstTask));
	kmem_cache_free(mi_sys_taskinfo_cachep,pstTask);

	return MI_SUCCESS;
}





MI_S32 MI_SYS_IMPL_WaitOnInputTaskAvailable(MI_SYS_DRV_HANDLE miSysDrvHandle , MI_S32 u32TimeOutMs)
{

	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)(miSysDrvHandle);

	BUG_ON(!pstModDev || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

	long __retval = u32TimeOutMs * HZ/1000;
	DEFINE_WAIT(wait);
	while (__retval>0) 
	{
		prepare_to_wait(&pstModDev->inputWaitqueue, &wait, TASK_INTERRUPTIBLE);
		if (signal_pending(current))
		{
			  finish_wait(&pstModDev->inputWaitqueue, &wait);
			  return -EINTR;
		}
		__retval = schedule_timeout_interruptible(u32TimeOutMs*HZ/1000);
		finish_wait(&pstModDev->inputWaitqueue, &wait);
	}
	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, struct mi_sys_ChnBufInfo_s *pstChnBufInfo)
{
	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)(miSysDrvHandle);
	MI_SYS_Channel_t *pstChannel;
	struct MI_SYS_InputPort_s *pstInputPort = NULL;
	MI_SYS_OutputPort_t *pstOutputPort = NULL;
	BUG_ON(!pstModDev
		 || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
	BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);

	int channelIdx = 0 ;
	int portId = 0;

    BUG_ON(pstChnBufInfo);
	BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
	
	pstChannel = &pstModDev->astChannels[u32ChnId];
	BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

    memset(pstChnBufInfo, 0, sizeof(*pstChnBufInfo));
	
	for(channelIdx = 0 ; channelIdx < pstModDev->u32DevChnNum ; channelIdx ++)
	{
		for(portId = 0 ; portId <  pstModDev->u32InputPortNum ; portId ++)
		{
			pstInputPort = pstChannel->pastInputPorts[portId];
			pstChnBufInfo->au32InputPortBindConnectBufPendingCnt[portId] = pstInputPort->stBindInputBufQueue.queue_buf_count;
			pstChnBufInfo->au32InputPortBufHoldByDrv[portId] = pstInputPort->stWorkingQueue.queue_buf_count;
			pstChnBufInfo->au32InputPortUserBufPendingCnt[portId] = pstInputPort->stUsrInjectBufQueue.queue_buf_count;
		}
		pstChnBufInfo->u32InputPortNum = pstModDev->u32InputPortNum;

		for(portId = 0 ; portId < pstModDev->u32DevChnNum ; portId ++)
		{
			pstOutputPort = pstChannel->pastOutputPorts[portId];
			pstChnBufInfo->au32OutputPortBufInUsrFIFONum[portId] = pstOutputPort->u32UsrFifoCount;
			pstChnBufInfo->au32OutputPortBufUsrLockedNum[portId] = atomic_read(&pstOutputPort->usrLockedBufCnt);
			pstChnBufInfo->au32OutputPortBufHoldByDrv[portId] = pstOutputPort->stWorkingQueue.queue_buf_count;
			pstChnBufInfo->au32OutputPortBufTotalInUsedNum[portId] = atomic_read(&pstOutputPort->totalOutputPortInUsedBuf);
		}
		pstChnBufInfo->u32OutputPortNum = pstModDev->u32OutputPortNum;
	}
	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U32 u32blkSize ,MI_PHY *phyAddr)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_MMA_Free(MI_PHY phyAddr)
{
	return MI_SUCCESS;

}

void * MI_SYS_IMPL_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size , MI_BOOL bCache)
{
    return NULL;
}

MI_S32 MI_SYS_IMPL_UnVmap(void *pVirtAddr)
{
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_EnableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
	pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId]->bPortEnable = TRUE;

	return MI_SUCCESS;
}
MI_S32 MI_SYS_IMPL_DisableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
	MI_SYS_OutputPort_t *pstOutputPort;
	MI_SYS_BufRef_t *pstBufRef;
	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
	pstOutputPort = pstModDev->astChannels[u32ChnId].pastOutputPorts[u32PortId];
	pstOutputPort->bPortEnable = FALSE;
    struct list_head *pos;

	while(pstBufRef = mi_sys_remove_from_queue_tail(&pstOutputPort->stUsrGetFifoBufQueue))
	{
		mi_sys_release_bufref(pstBufRef);
	}
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_EnableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t *)miSysDrvHandle;
	pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId]->bPortEnable = TRUE;

	return MI_SUCCESS;
}
MI_S32 MI_SYS_IMPL_DisableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
	MI_SYS_BufRef_t *pstBufRef;
	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
	struct MI_SYS_InputPort_s *pstInputPort;
	pstInputPort = pstModDev->astChannels[u32ChnId].pastInputPorts[u32PortId];
	pstInputPort->bPortEnable = FALSE;
    while(pstBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stBindInputBufQueue));
    {
		mi_sys_release_bufref(pstBufRef);
    }
	while(pstBufRef = mi_sys_remove_from_queue_tail(&pstInputPort->stUsrInjectBufQueue));
    {
		mi_sys_release_bufref(pstBufRef);
    }

	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_EnableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId)
{
	int portId = 0;

	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
	
	pstModDev->astChannels[u32ChnId].bChnEnable= TRUE;
	
	return MI_SUCCESS;
}

MI_S32 MI_SYS_IMPL_DisableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId)
{
	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
	MI_BOOL bEnable;
	int portId = 0;

	BUG_ON(!pstModDev
		 || pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);
	BUG_ON(pstModDev->u32DevChnNum>MI_SYS_MAX_DEV_CHN_CNT);
	
	pstModDev->astChannels[u32ChnId].bChnEnable= FALSE;
	for(portId = 0 ; portId < pstModDev->u32OutputPortNum ; portId ++)
	{
		bEnable = pstModDev->astChannels[u32ChnId].pastOutputPorts[portId]->bPortEnable;
		MI_SYS_IMPL_DisableOutputPort(miSysDrvHandle , u32ChnId , portId);
		pstModDev->astChannels[u32ChnId].pastOutputPorts[portId]->bPortEnable = TRUE;
	}
	for(portId = 0 ; portId < pstModDev->u32InputPortNum ; portId ++)
	{
		bEnable = pstModDev->astChannels[u32ChnId].pastOutputPorts[portId]->bPortEnable;
		MI_SYS_IMPL_DisableInputPort(miSysDrvHandle , u32ChnId , portId);
		pstModDev->astChannels[u32ChnId].pastOutputPorts[portId]->bPortEnable = TRUE;
	}
	return MI_SUCCESS;
}


MI_S32 MI_SYS_IMPL_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32InputportId, struct mi_sys_Allocator_s *pstUserAllocator)
{
     MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
     struct MI_SYS_InputPort_s *pstInputPort;
     MI_SYS_Channel_t *pstChannel;


     BUG_ON(!pstModDev);
     BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

     BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
     BUG_ON(u32InputportId >=pstModDev->u32InputPortNum);

     pstChannel = &pstModDev->astChannels[u32ChnId];

     BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

     pstInputPort = pstChannel->pastInputPorts[u32InputportId];
     BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

     mi_sys_Allocator_t *pstTmpAllocator = pstInputPort->pstCusBufAllocator;



     if(pstUserAllocator)
         pstUserAllocator->ops->OnRef(pstInputPort->pstCusBufAllocator);
     
     if(pstInputPort->pstCusBufAllocator)
         pstInputPort->pstCusBufAllocator->ops->OnUnref(pstInputPort->pstCusBufAllocator);

      pstInputPort->pstCusBufAllocator = pstUserAllocator;
     
     return MI_SUCCESS;

}
MI_S32 MI_SYS_IMPL_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32InputportId, struct mi_sys_Allocator_s *pstUserAllocator)
{

	MI_SYS_MOD_DEV_t *pstModDev = (MI_SYS_MOD_DEV_t*)miSysDrvHandle;
	MI_SYS_OutputPort_t *pstPeerOutputPort;
	struct MI_SYS_InputPort_s *pstInputPort;
	MI_SYS_Channel_t *pstChannel;


	BUG_ON(!pstModDev);
	BUG_ON(pstModDev->u32MagicNumber != __MI_SYS_MAGIC_NUM__);

	BUG_ON(u32ChnId >=pstModDev->u32DevChnNum);
	BUG_ON(u32InputportId >=pstModDev->u32InputPortNum);

	pstChannel = &pstModDev->astChannels[u32ChnId];

	BUG_ON(pstChannel->u32MagicNumber != __MI_SYS_CHN_MAGIC_NUM__);

	pstInputPort = pstChannel->pastInputPorts[u32InputportId];
	BUG_ON(pstInputPort->u32MagicNumber != __MI_SYS_INPUTPORT_MAGIC_NUM__);

	pstPeerOutputPort = pstInputPort->pstBindPeerOutputPort;

	if(pstPeerOutputPort)
	{
		BUG_ON(list_empty(&pstInputPort->stBindRelationShipList));

		BUG_ON(pstPeerOutputPort->u32MagicNumber != __MI_SYS_OUTPORT_MAGIC_NUM__);
		_MI_SYS_IMPL_CheckInputOutputBindCoherence(pstInputPort, pstPeerOutputPort);


		if(pstUserAllocator)
			pstUserAllocator->ops->OnRef(pstInputPort->pstCusBufAllocator);


		if(pstPeerOutputPort->pstCusBufAllocator)
			pstPeerOutputPort->pstCusBufAllocator->ops->OnUnref(pstInputPort->pstCusBufAllocator);

		pstPeerOutputPort->pstCusBufAllocator = pstUserAllocator;

		return MI_SUCCESS;
	}
	else
	{
		BUG_ON(!list_empty(&pstInputPort->stBindRelationShipList));
		return E_MI_ERR_ILLEGAL_PARAM;
	}
     
}
