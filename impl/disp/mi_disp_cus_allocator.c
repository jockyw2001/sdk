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
#include <linux/version.h>
#include <linux/kernel.h>

#include <linux/string.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>

#include "mi_print.h"
#include "mi_common_datatype.h"
#include "mi_sys_internal.h"

#include "mi_disp.h"
#include "mi_disp_datatype.h"
#include "mi_disp_datatype_internal.h"
#include "mi_disp_impl.h"
#include "mi_disp_cus_allocator.h"

#if DISP_ENABLE_CUS_ALLOCATOR

#define __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__  0x44414C43
#define __MI_DISP_CUS_ALLOCATION_MAGIC_NUM__  0x44414C4E
#define MI_DISP_MAX_LAYER_FULL_SCREEN_BUF_CNT 5
#if (defined SUPPORT_DISP_ALIGN_UP_OFFSET32 ) && (SUPPORT_DISP_ALIGN_UP_OFFSET32 == 0)
#define MI_DISP_ALIGN_UP_OFFSET32 0
#else
#define MI_DISP_ALIGN_UP_OFFSET32 1
#endif
//!!!Attention: define buffer stride alignment, if you change this value, peleas change MVOP_STRIDE_ALIGNMENT in mhal_disp.c(K6 and K6L).
#define MI_BUFFER_STRIDE_ALIGNMENT (96)

struct mi_disp_layer_buf_mgr_s;
typedef struct mi_disp_inputport_allocator_s
{
    unsigned int u32MagicNumber;//must be __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__
    MI_DISP_VidWinRect_t stWinRect; //win rect on layer buf
    struct list_head list_of_free_allocations; //list of all free allocations
    struct list_head list_of_all_allocations;
    struct semaphore semlock;
    struct  mi_disp_layer_buf_mgr_s *pst_disp_lay_buf_mgr; //reference to parent lay buf mgr
    mi_sys_Allocator_t stdAllocator;

}mi_disp_inputport_allocator_t;


typedef struct mi_disp_inputport_allocation_s
{
    MI_U32 u32MagicNumber;//must be __MI_DISP_CUS_ALLOCATION_MAGIC_NUM__
    //list in input port cus_allocator
    struct list_head list_in_allocator_freelist;
    struct list_head list_in_allocator_alllist;
    unsigned long flags;//for coherence check purpose
    //reference to output buf where this allocation's physical mem come from
    mi_disp_inputport_allocator_t *pst_parent_allocator;
    MI_SYS_BufferAllocation_t *pstParentMMAAllocation; //for user/kern va mapping support
    int bBufStaled;// app has changed the window attribute

    MI_SYS_BufferAllocation_t stdAllocation;
}mi_disp_inputport_allocation_t;

typedef struct mi_disp_layer_buf_mgr_s
{
   MI_BOOL bInited;
   MI_U32 u32BufCnt;
   MI_U32 u32Width;
   MI_U32 u32Height;
   MI_U32 u32Stride;
   MI_SYS_BufferAllocation_t *pst_layer_mma_allocations[MI_DISP_MAX_LAYER_FULL_SCREEN_BUF_CNT];
   mi_disp_inputport_allocator_t stLayerInputPortAllocators[MI_DISP_INPUTPORT_MAX];
}mi_disp_layer_buf_mgr_t;

static mi_disp_layer_buf_mgr_t g_disp_layer_buf_mgr_array[MI_DISP_LAYER_MAX] =
{
    {
         .bInited = FALSE
    },
    {
         .bInited = FALSE
    }
};

static DEFINE_SEMAPHORE(g_disp_layer_buf_mgr_semlock);

static inline MI_U32 _mi_disp_win_offset_in_layer_buf(mi_disp_inputport_allocator_t *pstInputPortCusAllocator)
{
#if MI_DISP_ALIGN_UP_OFFSET32
    MI_U32 u32Offset = 0;
#endif
    MI_SYS_BUG_ON(!pstInputPortCusAllocator->pst_disp_lay_buf_mgr);
    //Currenctly, lay buffer manager only resolve YUY2 capable buf. But we can treat it as MST YUV420 buf
    MI_SYS_BUG_ON(pstInputPortCusAllocator->stWinRect.u16X >= pstInputPortCusAllocator->pst_disp_lay_buf_mgr->u32Width
       || pstInputPortCusAllocator->stWinRect.u16Y >= pstInputPortCusAllocator->pst_disp_lay_buf_mgr->u32Height);
#if MI_DISP_ALIGN_UP_OFFSET32
    u32Offset = pstInputPortCusAllocator->stWinRect.u16X*2;
    // X need 32 alignment
    u32Offset = ALIGN_UP(u32Offset, 32);
    u32Offset += pstInputPortCusAllocator->stWinRect.u16Y*pstInputPortCusAllocator->pst_disp_lay_buf_mgr->u32Stride;
    return u32Offset;
#else
    return pstInputPortCusAllocator->stWinRect.u16Y*pstInputPortCusAllocator->pst_disp_lay_buf_mgr->u32Stride+pstInputPortCusAllocator->stWinRect.u16X*2;
#endif 
}

static inline void __disp_cus_allocator_check_coherence(mi_disp_inputport_allocator_t *pst_parent_allocator, mi_disp_inputport_allocation_t *pst_inport_allocation)
{
   //this function musb be called in pst_parent_allocator semlock locked state
   static unsigned long lst_flags = 0;
   mi_disp_inputport_allocation_t *pst_inport_tmp_allocation;

   ++lst_flags;

   list_for_each_entry(pst_inport_tmp_allocation, &pst_parent_allocator->list_of_all_allocations, list_in_allocator_alllist)
   {
          //mark that all previous allocation to be out of date
           pst_inport_tmp_allocation->flags =lst_flags;
   }


   list_for_each_entry(pst_inport_tmp_allocation, &pst_parent_allocator->list_of_free_allocations, list_in_allocator_freelist)
   {
          //mark that all previous allocation to be out of date
           MI_SYS_BUG_ON(pst_inport_tmp_allocation->flags !=lst_flags);
           pst_inport_tmp_allocation->flags = lst_flags+1;
   }

   if(pst_inport_allocation)
    {
        if(list_empty(&pst_inport_allocation->list_in_allocator_freelist))
        {
             MI_SYS_BUG_ON(pst_inport_allocation->flags != lst_flags);
        }
        else
        {
             MI_SYS_BUG_ON(pst_inport_allocation->flags != lst_flags+1);
        }
    }
    lst_flags++;
}

static int mi_disp_inputport_allocator_on_release(struct mi_sys_Allocator_s *pstAllocator)
{
     mi_disp_inputport_allocator_t *pst_inport_allocator;
     mi_disp_inputport_allocation_t   *pst_inport_allocation, *n;

     pst_inport_allocator =  container_of(pstAllocator, mi_disp_inputport_allocator_t,  stdAllocator);
     MI_SYS_BUG_ON(pst_inport_allocator->u32MagicNumber != __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__);

     down(&pst_inport_allocator->semlock);
     //free all free bufs
     __disp_cus_allocator_check_coherence(pst_inport_allocator, NULL);

     list_for_each_entry(pst_inport_allocation, &pst_inport_allocator->list_of_all_allocations, list_in_allocator_alllist)
     {
          //mark that all previous allocation to be out of date
           pst_inport_allocation->bBufStaled =1;
     }

     list_for_each_entry_safe(pst_inport_allocation, n, &pst_inport_allocator->list_of_free_allocations, list_in_allocator_freelist)
     {
           //free all allocation which already  in free list
           MI_SYS_BUG_ON(pst_inport_allocation->bBufStaled  == 0);
           list_del(&pst_inport_allocation->list_in_allocator_freelist);
           list_del(&pst_inport_allocation->list_in_allocator_alllist);
           MI_SYS_BUG_ON(!pst_inport_allocation->pstParentMMAAllocation);
           pst_inport_allocation->pstParentMMAAllocation->ops.OnUnref(pst_inport_allocation->pstParentMMAAllocation);

           memset(pst_inport_allocation, 0xD1, sizeof(*pst_inport_allocation));
           kfree(pst_inport_allocation);
     }

     MI_SYS_BUG_ON(!list_empty(&pst_inport_allocator->list_of_all_allocations));

     up(&pst_inport_allocator->semlock);

     return MI_SYS_SUCCESS;
}
static MI_SYS_BufferAllocation_t *mi_disp_inputport_allocator_on_alloc(mi_sys_Allocator_t *pstAllocator,
                                       MI_SYS_BufConf_t *pstBufConfig)
{

    mi_disp_inputport_allocator_t *pst_inport_allocator;
    mi_disp_inputport_allocation_t *pst_inport_allocation, *n;


     MI_SYS_BUG_ON(!pstAllocator || !pstBufConfig);

     pst_inport_allocator =  container_of(pstAllocator, mi_disp_inputport_allocator_t,  stdAllocator);

     MI_SYS_BUG_ON(pst_inport_allocator->u32MagicNumber != __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__);

     down(&pst_inport_allocator->semlock);

     list_for_each_entry_safe(pst_inport_allocation, n, &pst_inport_allocator->list_of_free_allocations, list_in_allocator_freelist)
     {
           MI_SYS_BUG_ON(pst_inport_allocation->u32MagicNumber != __MI_DISP_CUS_ALLOCATION_MAGIC_NUM__);
           list_del(&pst_inport_allocation->list_in_allocator_freelist);
           INIT_LIST_HEAD(&pst_inport_allocation->list_in_allocator_freelist);

           if(pst_inport_allocation->bBufStaled)
            {
                pst_inport_allocation->stdAllocation.ops.OnRelease(&pst_inport_allocation->stdAllocation);
                continue;
            }
           //pst_inport_allocation->pst_parent_allocator = pst_inport_allocator;
           pstAllocator->ops->OnRef(pstAllocator);
           up(&pst_inport_allocator->semlock);

           pst_inport_allocation->stdAllocation.stBufInfo.u64Pts = pstBufConfig->u64TargetPts;
           //DBG_INFO("Allocator Request: %d x %d return: %d x %d.\n", pstBufConfig->stFrameCfg.u16Width, pstBufConfig->stFrameCfg.u16Height, pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.u16Width, pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.u16Height);
           return &pst_inport_allocation->stdAllocation;
     }


     up(&pst_inport_allocator->semlock);

     return NULL;

}

static int mi_disp_inputport_allocator_suit_bufconfig(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufConf_t *stBufConfig)
{


    mi_disp_inputport_allocator_t *pst_inport_allocator;


     MI_SYS_BUG_ON(!pstAllocator || !stBufConfig);

     pst_inport_allocator =  container_of(pstAllocator, mi_disp_inputport_allocator_t,  stdAllocator);

     MI_SYS_BUG_ON(pst_inport_allocator->u32MagicNumber != __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__);

     MI_SYS_BUG_ON(!stBufConfig);

     if(stBufConfig->eBufType  != E_MI_SYS_BUFDATA_FRAME)
     {
         MI_SYS_BUG();
         return INT_MIN;
     }

     return INT_MAX;

}


static buf_allocator_ops_t mi_disp_inputport_allocator_ops =
{
  .OnRef = generic_allocator_on_ref,
  .OnUnref = generic_allocator_on_unref,
  .OnRelease = mi_disp_inputport_allocator_on_release,
  .alloc = mi_disp_inputport_allocator_on_alloc,
  .suit_bufconfig = mi_disp_inputport_allocator_suit_bufconfig
};


static void *mi_disp_inputport_allocation_map_user(struct MI_SYS_BufferAllocation_s *thiz)
{
     MI_U32 offset;
     mi_disp_inputport_allocation_t *pst_inport_allocation;
     mi_disp_inputport_allocator_t  *pst_inport_cus_allocator;
     void *user_va;

     MI_SYS_BUG_ON(!thiz);

     pst_inport_allocation = container_of(thiz, mi_disp_inputport_allocation_t, stdAllocation);
     MI_SYS_BUG_ON(pst_inport_allocation->u32MagicNumber != __MI_DISP_CUS_ALLOCATION_MAGIC_NUM__);

     MI_SYS_BUG_ON(!pst_inport_allocation->pstParentMMAAllocation);

     pst_inport_cus_allocator = pst_inport_allocation->pst_parent_allocator;
     MI_SYS_BUG_ON(pst_inport_cus_allocator->u32MagicNumber != __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__);

     offset = _mi_disp_win_offset_in_layer_buf(pst_inport_cus_allocator);

     user_va = pst_inport_allocation->pstParentMMAAllocation->ops.map_user(pst_inport_allocation->pstParentMMAAllocation);
     if(user_va == NULL)
         return NULL;

     return (void*)((char*)user_va+offset);
}
static void  mi_disp_inputport_allocation_unmap_user(struct MI_SYS_BufferAllocation_s *thiz)
{
     mi_disp_inputport_allocation_t *pst_inport_allocation;

     MI_SYS_BUG_ON(!thiz);

     pst_inport_allocation = container_of(thiz, mi_disp_inputport_allocation_t, stdAllocation);
     MI_SYS_BUG_ON(pst_inport_allocation->u32MagicNumber != __MI_DISP_CUS_ALLOCATION_MAGIC_NUM__);
     MI_SYS_BUG_ON(!pst_inport_allocation->pstParentMMAAllocation);

     pst_inport_allocation->pstParentMMAAllocation->ops.unmap_user(pst_inport_allocation->pstParentMMAAllocation);
}
static void *mi_disp_inputport_allocation_vmap_kern(struct MI_SYS_BufferAllocation_s *thiz)
{
     MI_U32 offset;
     mi_disp_inputport_allocation_t *pst_inport_allocation;
     mi_disp_inputport_allocator_t  *pstCusAllocator;
     void *kern_va;

     MI_SYS_BUG_ON(!thiz);

     pst_inport_allocation = container_of(thiz, mi_disp_inputport_allocation_t, stdAllocation);
     MI_SYS_BUG_ON(pst_inport_allocation->u32MagicNumber != __MI_DISP_CUS_ALLOCATION_MAGIC_NUM__);

     MI_SYS_BUG_ON(!pst_inport_allocation->pstParentMMAAllocation);

     pstCusAllocator = pst_inport_allocation->pst_parent_allocator;
     MI_SYS_BUG_ON(pstCusAllocator->u32MagicNumber != __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__);

     offset = _mi_disp_win_offset_in_layer_buf(pstCusAllocator);

     kern_va = pst_inport_allocation->pstParentMMAAllocation->ops.vmap_kern(pst_inport_allocation->pstParentMMAAllocation);
     if(kern_va == NULL)
         return NULL;

     return (void*)((char*)kern_va+offset);
}
static void  mi_disp_inputport_allocation_vunmap_kern(struct MI_SYS_BufferAllocation_s *thiz)
{
     mi_disp_inputport_allocation_t *pst_inport_allocation;
     MI_SYS_BUG_ON(!thiz);

     pst_inport_allocation = container_of(thiz, mi_disp_inputport_allocation_t, stdAllocation);
     MI_SYS_BUG_ON(pst_inport_allocation->u32MagicNumber != __MI_DISP_CUS_ALLOCATION_MAGIC_NUM__);
     MI_SYS_BUG_ON(!pst_inport_allocation->pstParentMMAAllocation);

     pst_inport_allocation->pstParentMMAAllocation->ops.vunmap_kern(pst_inport_allocation->pstParentMMAAllocation);
}

static void mi_disp_inputport_allocation_on_release(struct MI_SYS_BufferAllocation_s *thiz)
{
     mi_disp_inputport_allocation_t *pst_inport_allocation;
     mi_sys_Allocator_t *pstParentStdAllocator;

     MI_SYS_BUG_ON(!thiz);

     pst_inport_allocation =  container_of(thiz, mi_disp_inputport_allocation_t,  stdAllocation);
     DBG_INFO("Release: pst_inport_allocation[%p].\n", pst_inport_allocation);
     MI_SYS_BUG_ON(!pst_inport_allocation->pst_parent_allocator);

     MI_SYS_BUG_ON(pst_inport_allocation->pst_parent_allocator->u32MagicNumber != __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__);

     MI_SYS_BUG_ON(list_empty(&pst_inport_allocation->list_in_allocator_alllist));
     MI_SYS_BUG_ON(!list_empty(&pst_inport_allocation->list_in_allocator_freelist));

     pstParentStdAllocator = &pst_inport_allocation->pst_parent_allocator->stdAllocator;

     if(pst_inport_allocation->bBufStaled)
     {
          down(&pst_inport_allocation->pst_parent_allocator->semlock);
          __disp_cus_allocator_check_coherence(pst_inport_allocation->pst_parent_allocator, pst_inport_allocation);

          list_del(&pst_inport_allocation->list_in_allocator_alllist);
          up(&pst_inport_allocation->pst_parent_allocator->semlock);

          MI_SYS_BUG_ON(!pst_inport_allocation->pstParentMMAAllocation);
          pst_inport_allocation->pstParentMMAAllocation->ops.OnUnref(pst_inport_allocation->pstParentMMAAllocation);

          memset(pst_inport_allocation, 0xD1, sizeof(*pst_inport_allocation));
          kfree(pst_inport_allocation);
     }
     else
     {
         down(&pst_inport_allocation->pst_parent_allocator->semlock);
          __disp_cus_allocator_check_coherence(pst_inport_allocation->pst_parent_allocator, pst_inport_allocation);
         list_add(&pst_inport_allocation->list_in_allocator_freelist, &pst_inport_allocation->pst_parent_allocator->list_of_free_allocations);
         up(&pst_inport_allocation->pst_parent_allocator->semlock);

     }
     //unreference to parent allocator
     pstParentStdAllocator->ops->OnUnref(pstParentStdAllocator);
}
static buf_allocation_ops_t mi_disp_inputport_allocation_ops=
{
    .OnRef=generic_allocation_on_ref,
    .OnUnref=generic_allocation_on_unref,
    .OnRelease=mi_disp_inputport_allocation_on_release,
    .map_user=mi_disp_inputport_allocation_map_user,
    .unmap_user=mi_disp_inputport_allocation_unmap_user,
    .vmap_kern=mi_disp_inputport_allocation_vmap_kern,
    .vunmap_kern=mi_disp_inputport_allocation_vunmap_kern
};

static void mi_disp_allcator_init(mi_disp_inputport_allocator_t *pstVDispInportAllocator, mi_disp_layer_buf_mgr_t *pst_disp_lay_buf_mgr)
{
   pstVDispInportAllocator->u32MagicNumber = __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__;

   INIT_LIST_HEAD(&pstVDispInportAllocator->list_of_all_allocations);
   INIT_LIST_HEAD(&pstVDispInportAllocator->list_of_free_allocations);
   sema_init(&pstVDispInportAllocator->semlock, 1);
   pstVDispInportAllocator->stWinRect.u16X = 0;
   pstVDispInportAllocator->stWinRect.u16Y = 0;
   pstVDispInportAllocator->stWinRect.u16Width = 0;
   pstVDispInportAllocator->stWinRect.u16Height = 0;
   pstVDispInportAllocator->pst_disp_lay_buf_mgr = pst_disp_lay_buf_mgr;
   generic_allocator_init(&pstVDispInportAllocator->stdAllocator, &mi_disp_inputport_allocator_ops);
}

static int mi_disp_cus_allcator_staled_all_previous_allocation(mi_disp_inputport_allocator_t *pst_inport_allocator)
{
     mi_disp_inputport_allocation_t *pst_inport_allocation, *n;
     int ret = MI_SYS_SUCCESS;

     MI_SYS_BUG_ON(pst_inport_allocator->u32MagicNumber != __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__);
     down(&pst_inport_allocator->semlock);
     //free all un used bufs
      __disp_cus_allocator_check_coherence(pst_inport_allocator, NULL);
     list_for_each_entry(pst_inport_allocation, &pst_inport_allocator->list_of_all_allocations, list_in_allocator_alllist)
     {
          //mark that all previous allocation to be out of date
           pst_inport_allocation->bBufStaled =1;
     }
     list_for_each_entry_safe(pst_inport_allocation, n, &pst_inport_allocator->list_of_free_allocations, list_in_allocator_freelist)
     {
           //free all allocation which already  in free list
           MI_SYS_BUG_ON(pst_inport_allocation->bBufStaled  == 0);
           list_del(&pst_inport_allocation->list_in_allocator_freelist);
           list_del(&pst_inport_allocation->list_in_allocator_alllist);

           MI_SYS_BUG_ON(!pst_inport_allocation->pstParentMMAAllocation);
           pst_inport_allocation->pstParentMMAAllocation->ops.OnUnref(pst_inport_allocation->pstParentMMAAllocation);

           memset(pst_inport_allocation, 0xD1, sizeof(*pst_inport_allocation));
           kfree(pst_inport_allocation);
     }
     up(&pst_inport_allocator->semlock);
     return ret;
}
//add new allocation into cus allocator (pixel width will alway set to 2)
static int mi_disp_cus_allcator_add_new_allocation(mi_disp_inputport_allocator_t *pst_inport_allocator,
                                     MI_PHY phyAddr,  MI_U32 u32Width, MI_U32 u32Height, MI_U32 u32Stride,
                                     MI_SYS_PixelFormat_e  ePixelFormat,
                                     MI_SYS_BufferAllocation_t *pstParentMMAAllocation)
{
     mi_disp_inputport_allocation_t *pst_inport_allocation;

     MI_SYS_BUG_ON(pst_inport_allocator->u32MagicNumber != __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__);


     MI_SYS_BUG_ON(!u32Width || !u32Height ||u32Stride<u32Width*2);


     pst_inport_allocation = (mi_disp_inputport_allocation_t*)kmalloc(sizeof(*pst_inport_allocation), GFP_KERNEL);
     if(!pst_inport_allocation)
          return MI_ERR_DISP_NO_MEM;

     memset(pst_inport_allocation, 0, sizeof(*pst_inport_allocation));

     INIT_LIST_HEAD(&pst_inport_allocation->list_in_allocator_alllist);
     INIT_LIST_HEAD(&pst_inport_allocation->list_in_allocator_freelist);

     pst_inport_allocation->pst_parent_allocator =  pst_inport_allocator;
     pst_inport_allocation->bBufStaled = 0;
     pst_inport_allocation->flags = 0;
     generic_allocation_init(&pst_inport_allocation->stdAllocation, &mi_disp_inputport_allocation_ops);

     pst_inport_allocation->stdAllocation.stBufInfo.eBufType = E_MI_SYS_BUFDATA_FRAME;
     pst_inport_allocation->stdAllocation.stBufInfo.u64Pts = -1ULL;
     pst_inport_allocation->stdAllocation.stBufInfo.bEndOfStream = 0;
     pst_inport_allocation->stdAllocation.stBufInfo.bUsrBuf = 0;
     pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
     pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.ePixelFormat = ePixelFormat;
     pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
     pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
     pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
     pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.u16Width = u32Width;
     pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.u16Height = u32Height;
     pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.phyAddr[0] = phyAddr;
     pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.u32Stride[0] = u32Stride;
     if (E_MI_SYS_PIXEL_FRAME_YUV422_YUYV == ePixelFormat)
     {
         pst_inport_allocation->stdAllocation.stBufInfo.stFrameData.u32BufSize = u32Width * u32Height * 2;
     }
     pst_inport_allocation->pstParentMMAAllocation = pstParentMMAAllocation;
     pst_inport_allocation->u32MagicNumber = __MI_DISP_CUS_ALLOCATION_MAGIC_NUM__;
     pstParentMMAAllocation->ops.OnRef(pstParentMMAAllocation);

     down(&pst_inport_allocator->semlock);
     list_add_tail(&pst_inport_allocation->list_in_allocator_alllist, &pst_inport_allocator->list_of_all_allocations);
     list_add_tail(&pst_inport_allocation->list_in_allocator_freelist, &pst_inport_allocator->list_of_free_allocations);
     up(&pst_inport_allocator->semlock);

     return MI_SYS_SUCCESS;

}

static void _mi_disp_init_layer_cus_allocator(mi_disp_layer_buf_mgr_t *pstDispLayBufMgr)
{
   MI_U32 index;
   if(pstDispLayBufMgr->bInited)
      return;
   memset(pstDispLayBufMgr, 0, sizeof(*pstDispLayBufMgr));
   for( index=0; index<MI_DISP_INPUTPORT_MAX; index++)
       mi_disp_allcator_init(&pstDispLayBufMgr->stLayerInputPortAllocators[index], pstDispLayBufMgr);
    pstDispLayBufMgr->bInited = TRUE;
}


static void _mi_disp_reflesh_cus_allocations(MI_DISP_LAYER layer, MI_U32 u32PortId)
{
     mi_disp_inputport_allocator_t *pstCusAllocator;
     MI_U32 index;

     MI_SYS_BUG_ON(layer<0 || layer>=MI_DISP_LAYER_MAX);
     MI_SYS_BUG_ON(u32PortId >= MI_DISP_INPUTPORT_MAX);


     pstCusAllocator = &g_disp_layer_buf_mgr_array[layer].stLayerInputPortAllocators[u32PortId];

     mi_disp_cus_allcator_staled_all_previous_allocation(pstCusAllocator);

     if(!g_disp_layer_buf_mgr_array[layer].bInited)
     {
          DBG_ERR("In %s [layer%d, port%u]  layer not inited.\n",  __FUNCTION__, layer, u32PortId);
          return;
     }

     if(pstCusAllocator->stWinRect.u16Width==0 || pstCusAllocator->stWinRect.u16Height==0 || (pstCusAllocator->stWinRect.u16X&1))
     {
          DBG_ERR("In %s [layer%d, port%u]  invalid param winrect(%u, %u, %u, %u)!!!\n",
             __FUNCTION__, layer, u32PortId,
             pstCusAllocator->stWinRect.u16X, pstCusAllocator->stWinRect.u16Y, pstCusAllocator->stWinRect.u16Width, pstCusAllocator->stWinRect.u16Height);
           return;
     }

     if(pstCusAllocator->stWinRect.u16X+pstCusAllocator->stWinRect.u16Width<=pstCusAllocator->stWinRect.u16X
        || pstCusAllocator->stWinRect.u16Y+pstCusAllocator->stWinRect.u16Height<=pstCusAllocator->stWinRect.u16Y
        || pstCusAllocator->stWinRect.u16X+pstCusAllocator->stWinRect.u16Width>g_disp_layer_buf_mgr_array[layer].u32Width
        || pstCusAllocator->stWinRect.u16Y+pstCusAllocator->stWinRect.u16Height>g_disp_layer_buf_mgr_array[layer].u32Height)
     {
           DBG_ERR("In %s [layer%d, port%u] invalid param winrect(%u, %u, %u, %u) layer buf size(%u, %u)!!!\n",
            __FUNCTION__, layer, u32PortId,
            pstCusAllocator->stWinRect.u16X, pstCusAllocator->stWinRect.u16Y, pstCusAllocator->stWinRect.u16Width, pstCusAllocator->stWinRect.u16Height,
            g_disp_layer_buf_mgr_array[layer].u32Width, g_disp_layer_buf_mgr_array[layer].u32Height);
           return;
     }

     for( index=0; index<g_disp_layer_buf_mgr_array[layer].u32BufCnt;  index++)
     {
        MI_SYS_BufferAllocation_t *pstMMAAllocation;

        pstMMAAllocation = g_disp_layer_buf_mgr_array[layer].pst_layer_mma_allocations[index];

        MI_SYS_BUG_ON(pstMMAAllocation->stBufInfo.eBufType != E_MI_SYS_BUFDATA_RAW);
        MI_SYS_BUG_ON(pstMMAAllocation->stBufInfo.stRawData.u32BufSize <g_disp_layer_buf_mgr_array[layer].u32Stride*g_disp_layer_buf_mgr_array[layer].u32Height);

        mi_disp_cus_allcator_add_new_allocation(pstCusAllocator,
            pstMMAAllocation->stBufInfo.stRawData.phyAddr+_mi_disp_win_offset_in_layer_buf(pstCusAllocator),
            pstCusAllocator->stWinRect.u16Width, pstCusAllocator->stWinRect.u16Height,  g_disp_layer_buf_mgr_array[layer].u32Stride,
            E_MI_SYS_PIXEL_FRAME_YUV422_YUYV, pstMMAAllocation);
     }
}

//#define trace_win(pstWin) printk("%s()@line %d: {%d, %d, %d, %d}.\n", __func__, __LINE__, (pstWin)->u16X, (pstWin)->u16Y, (pstWin)->u16Width, (pstWin)->u16Height)
#define trace_win(pstWin)
void mi_disp_layer_port_set_winrect(MI_DISP_LAYER layer, MI_U32 u32PortId, MI_DISP_VidWinRect_t *pstWinRect)
{

     static MI_DISP_VidWinRect_t zero_winrect = {0, 0, 0, 0};
     MI_SYS_BUG_ON(layer<0 || layer>=MI_DISP_LAYER_MAX);
     MI_SYS_BUG_ON(u32PortId >= MI_DISP_INPUTPORT_MAX);

     if(pstWinRect == NULL)
          pstWinRect = &zero_winrect;
     trace_win(pstWinRect);

     if(memcmp(&g_disp_layer_buf_mgr_array[layer].stLayerInputPortAllocators[u32PortId].stWinRect, pstWinRect, sizeof(*pstWinRect))==0)
           return;
     trace_win(pstWinRect);

     if(!g_disp_layer_buf_mgr_array[layer].bInited)
     {
          DBG_ERR("In %s [layer%d, port%u]  layer not inited.\n",  __FUNCTION__, layer, u32PortId);
          return;
     }
     trace_win(pstWinRect);

     down(&g_disp_layer_buf_mgr_semlock);
     if(pstWinRect->u16X+pstWinRect->u16Width<pstWinRect->u16X
        || pstWinRect->u16Y+pstWinRect->u16Height<pstWinRect->u16Y
        || pstWinRect->u16X+pstWinRect->u16Width>g_disp_layer_buf_mgr_array[layer].u32Width
        || pstWinRect->u16Y+pstWinRect->u16Height>g_disp_layer_buf_mgr_array[layer].u32Height)
    {
           DBG_ERR("In %s [layer%d, port%u] invalid param winrect(%u, %u, %u, %u) layer buf size(%u, %u), reset winrect to zero!!!\n",
            __FUNCTION__, layer, u32PortId,
            pstWinRect->u16X, pstWinRect->u16Y, pstWinRect->u16Width, pstWinRect->u16Height,
            g_disp_layer_buf_mgr_array[layer].u32Width, g_disp_layer_buf_mgr_array[layer].u32Height);
           memset(&g_disp_layer_buf_mgr_array[layer].stLayerInputPortAllocators[u32PortId].stWinRect, 0, sizeof(*pstWinRect));
    }
    else
    {
        memcpy(&g_disp_layer_buf_mgr_array[layer].stLayerInputPortAllocators[u32PortId].stWinRect, pstWinRect, sizeof(*pstWinRect));
    }

    trace_win(pstWinRect);
    trace_win(&g_disp_layer_buf_mgr_array[layer].stLayerInputPortAllocators[u32PortId].stWinRect);

     _mi_disp_reflesh_cus_allocations(layer, u32PortId);

    up(&g_disp_layer_buf_mgr_semlock);
}

MI_S32 mi_disp_config_layer_canvas(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32DevChnn, MI_DISP_LAYER layer,  MI_U32 u32Width, MI_U32 u32Height, MI_U32 u32BufCnt)
{
   mi_disp_inputport_allocator_t *pstInportAllocator;
   MI_U32 u32CurIndex, k;
   MI_U32 u32Size, u32Stride;
   MI_SYS_BufConf_t buf_config;
   MI_SYS_BufferAllocation_t *pstMMAAllocation;

   if(layer >=  MI_DISP_LAYER_MAX)
       return MI_ERR_SYS_ILLEGAL_PARAM;

   if(u32BufCnt >= MI_DISP_MAX_LAYER_FULL_SCREEN_BUF_CNT)
   {
       DBG_ERR("Invalid u32BufCnt(%u) in %s, force to %u.\n", u32BufCnt, __FUNCTION__, MI_DISP_MAX_LAYER_FULL_SCREEN_BUF_CNT);
       u32BufCnt = MI_DISP_MAX_LAYER_FULL_SCREEN_BUF_CNT;
   }

   if(u32Width==0 || u32Height==0)
      u32BufCnt = 0;// release all

   if(g_disp_layer_buf_mgr_array[layer].bInited &&
       g_disp_layer_buf_mgr_array[layer].u32Width == u32Width && g_disp_layer_buf_mgr_array[layer].u32Height == u32Height
       && g_disp_layer_buf_mgr_array[layer].u32BufCnt == u32BufCnt)
       return MI_SUCCESS;

   down(&g_disp_layer_buf_mgr_semlock);

   _mi_disp_init_layer_cus_allocator(&g_disp_layer_buf_mgr_array[layer]);

    for( u32CurIndex=0; u32CurIndex<MI_DISP_INPUTPORT_MAX; u32CurIndex++)
    {
        pstInportAllocator = &g_disp_layer_buf_mgr_array[layer].stLayerInputPortAllocators[u32CurIndex];
        MI_SYS_BUG_ON(pstInportAllocator->u32MagicNumber != __MI_DISP_CUS_ALLOCATOR_MAGIC_NUM__);
        mi_disp_cus_allcator_staled_all_previous_allocation(pstInportAllocator);
    }
    while(g_disp_layer_buf_mgr_array[layer].u32BufCnt > u32BufCnt)
    {

        pstMMAAllocation = g_disp_layer_buf_mgr_array[layer].pst_layer_mma_allocations[g_disp_layer_buf_mgr_array[layer].u32BufCnt-1];
        g_disp_layer_buf_mgr_array[layer].pst_layer_mma_allocations[g_disp_layer_buf_mgr_array[layer].u32BufCnt-1] = NULL;

        MI_SYS_BUG_ON(pstMMAAllocation == NULL);
        pstMMAAllocation->ops.OnUnref(pstMMAAllocation);
        g_disp_layer_buf_mgr_array[layer].u32BufCnt--;
    }


     if((!((u32BufCnt == 0) && (u32Width == 0) && (u32Height == 0)))&& (u32BufCnt == 0 ||
            (g_disp_layer_buf_mgr_array[layer].u32Width >= u32Width && g_disp_layer_buf_mgr_array[layer].u32Height>=u32Height))
        )
    {
         goto skip_reallocating;
     }

    // free all previous MMA layer buf allocations
    while(g_disp_layer_buf_mgr_array[layer].u32BufCnt > 0)
    {
        MI_SYS_BufferAllocation_t *pstMMAAllocation;

        pstMMAAllocation = g_disp_layer_buf_mgr_array[layer].pst_layer_mma_allocations[g_disp_layer_buf_mgr_array[layer].u32BufCnt-1];
        g_disp_layer_buf_mgr_array[layer].pst_layer_mma_allocations[g_disp_layer_buf_mgr_array[layer].u32BufCnt-1] = NULL;

        MI_SYS_BUG_ON(pstMMAAllocation == NULL);
        pstMMAAllocation->ops.OnUnref(pstMMAAllocation);
        g_disp_layer_buf_mgr_array[layer].u32BufCnt--;
    }
     // YUV422 MTS420 share the same buffer
#if MI_DISP_ALIGN_UP_OFFSET32
     u32Stride = ALIGN_UP(u32Width*2 + 32, MI_BUFFER_STRIDE_ALIGNMENT);
     // Ask one line more to prevent disp input window flush memory over flow.
     // User flush Win1 will:
     // 1. start at Win1 baseAddress
     // 2. Size = Height * Stride
     // It will cause last line over Layer size
     // Solution: Layer size = (LayerHeight +1) * Stride
     //+ -------------------------------------------------+
     //|<----------------------Stride-------------------->|
     //|                                                  |
     //|                                                  |
     //+---------------------+        +-------------+     |
     //|                     |        |     Win1    |     |
     //|        Win0         |        |             |     |
     //|                     |        +-------------+     |
     //|                     |                            |
     //+---------------------+                            |
     //|                                                  |
     //|                                                  |
     //+ -------------------------------------------------+
     u32Size = u32Stride*(u32Height + 1);
#else
     u32Stride = ALIGN_UP(u32Width*2, MI_BUFFER_STRIDE_ALIGNMENT);
     u32Size = u32Stride*u32Height;
#endif

     buf_config.eBufType = E_MI_SYS_BUFDATA_RAW;
     buf_config.u64TargetPts = 0;
     buf_config.stRawCfg.u32Size = u32Size;

     for( g_disp_layer_buf_mgr_array[layer].u32BufCnt=0;
                 g_disp_layer_buf_mgr_array[layer].u32BufCnt<u32BufCnt; g_disp_layer_buf_mgr_array[layer].u32BufCnt++)
    {

          pstMMAAllocation = mi_sys_AllocMMALowLevelBufAllocation(miSysDrvHandle, u32DevChnn, &buf_config);
          if(pstMMAAllocation)
          {
             g_disp_layer_buf_mgr_array[layer].pst_layer_mma_allocations[g_disp_layer_buf_mgr_array[layer].u32BufCnt] = pstMMAAllocation;
             pstMMAAllocation->ops.OnRef(pstMMAAllocation);
          }
          else
          {
              DBG_ERR("Run out of MMA memory in %s\n", __FUNCTION__);
             break;
          }
    }

    if(g_disp_layer_buf_mgr_array[layer].u32BufCnt != u32BufCnt)
    {
         goto failed_n_free;
    }
    g_disp_layer_buf_mgr_array[layer].u32Width = u32Width;
    g_disp_layer_buf_mgr_array[layer].u32Height = u32Height;
    g_disp_layer_buf_mgr_array[layer].u32Stride= u32Stride;
    g_disp_layer_buf_mgr_array[layer].u32BufCnt = u32BufCnt;

skip_reallocating:
     for( u32CurIndex=0; u32CurIndex<MI_DISP_INPUTPORT_MAX; u32CurIndex++)
    {
        for( k=0; k< g_disp_layer_buf_mgr_array[layer].u32BufCnt; k++)
       {
          MI_SYS_BufferAllocation_t *pstMMAAllocation;

          pstMMAAllocation = g_disp_layer_buf_mgr_array[layer].pst_layer_mma_allocations[k];
          mi_disp_cus_allcator_add_new_allocation(&g_disp_layer_buf_mgr_array[layer].stLayerInputPortAllocators[u32CurIndex], pstMMAAllocation->stBufInfo.stRawData.phyAddr,
            g_disp_layer_buf_mgr_array[layer].u32Width,  g_disp_layer_buf_mgr_array[layer].u32Height,  g_disp_layer_buf_mgr_array[layer].u32Stride,
             E_MI_SYS_PIXEL_FRAME_YUV422_YUYV, pstMMAAllocation);
       }
    }
 //   for( u32CurIndex=0; u32CurIndex<MI_DISP_INPUTPORT_MAX; u32CurIndex++)
  //      _mi_disp_reflesh_cus_allocations(layer, u32CurIndex);
    up(&g_disp_layer_buf_mgr_semlock);
    return MI_SYS_SUCCESS;

failed_n_free:
    while(g_disp_layer_buf_mgr_array[layer].u32BufCnt > 0)
    {
        MI_SYS_BufferAllocation_t *pstMMAAllocation;
        pstMMAAllocation = g_disp_layer_buf_mgr_array[layer].pst_layer_mma_allocations[g_disp_layer_buf_mgr_array[layer].u32BufCnt-1];
        g_disp_layer_buf_mgr_array[layer].pst_layer_mma_allocations[g_disp_layer_buf_mgr_array[layer].u32BufCnt-1] = NULL;
        MI_SYS_BUG_ON(pstMMAAllocation == NULL);
        pstMMAAllocation->ops.OnUnref(pstMMAAllocation);
        g_disp_layer_buf_mgr_array[layer].u32BufCnt--;
    }
    g_disp_layer_buf_mgr_array[layer].u32Width = 0;
    g_disp_layer_buf_mgr_array[layer].u32Width = 0;
    g_disp_layer_buf_mgr_array[layer].u32Stride= 0;
    g_disp_layer_buf_mgr_array[layer].u32BufCnt = 0;

    for( u32CurIndex=0; u32CurIndex<MI_DISP_INPUTPORT_MAX; u32CurIndex++)
        _mi_disp_reflesh_cus_allocations(layer, u32CurIndex);

    up(&g_disp_layer_buf_mgr_semlock);
    return MI_ERR_DISP_NO_MEM;
}
mi_sys_Allocator_t *mi_disp_layer_get_cus_allocator(MI_DISP_LAYER layer, MI_U32 u32PortId)
{

     MI_SYS_BUG_ON(layer<0 || layer>=MI_DISP_LAYER_MAX);
     MI_SYS_BUG_ON(u32PortId >= MI_DISP_INPUTPORT_MAX);

     if( ! g_disp_layer_buf_mgr_array[layer].bInited
          || g_disp_layer_buf_mgr_array[layer].u32Width == 0)
    {
           DBG_ERR("In %s, layer buf not ready!!![inited%d, width%d]\n", __FUNCTION__,
            g_disp_layer_buf_mgr_array[layer].bInited, g_disp_layer_buf_mgr_array[layer].u32Width);
          return NULL;
    }

     return &g_disp_layer_buf_mgr_array[layer].stLayerInputPortAllocators[u32PortId].stdAllocator;
}

#endif //if DISP_ENABLE_CUS_ALLOCATOR
