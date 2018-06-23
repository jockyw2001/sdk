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

#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
///#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/math64.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include "mi_print.h"
#include "mhal_common.h"
#include "mi_sd_impl.h"
#include "mi_sd.h"
#include "mi_sd_impl_internal.h"
#include "mi_sys_sideband_msg.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_vif_internal.h"

#include "../sys/mi_sys_buf_mgr.h"

#define PORTID E_MHAL_SCL_OUTPUT_PORT2
#define USE_CMDQ 0
#define SD_ZOOM_RATION_DEN 0x80000UL //so we support 8K resulotion calculation where U32 overflow
#define LOCK_CHNN(pstChnnInfo)   down(&pstChnnInfo->stChnnMutex)
#define UNLOCK_CHNN(pstChnnInfo) up(&pstChnnInfo->stChnnMutex)

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define SW_STUB_TEST 1
#define SD_ADJUST_THREAD_PRIORITY 0
#define SD_TASK_PERF_DBG          0

#define MI_SD_MAX_INPUTPORT_NUM             (1)
#define SD_WORK_THREAD_WAIT                 (2)
// TODO: Need Dummy register
#define MI_SD_FENCE_REGISTER                (0x12345678)
#define FHD_SIZE                             ((1920*1088))
#ifndef SW_STUB_TEST
#define MI_SD_FRAME_PER_BURST_CMDQ          (1)
#define SD_PROC_WAIT                        (1)
#else
#define MI_SD_FRAME_PER_BURST_CMDQ          (1)
#define SD_PROC_WAIT                        (1)
#endif
//#define WITHOUT_IRQ
#define MIU_BURST_BITS        (256)
#define YUV422_PIXEL_ALIGN    (2)
#define YUV422_BYTE_PER_PIXEL (2)
#define YUV420_PIXEL_ALIGN    (2)
#define YUV420_BYTE_PER_PIXEL (1)
#define REPEAT_MAX_NUMBER (2)

#ifndef MI_SUCCESS
#define MI_SUCCESS (0)
#endif
#ifndef MI_SYS_ERR_BUSY
#define MI_SYS_ERR_BUSY (1)
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#endif
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, alignment) (( (val)/(alignment))*(alignment))
#endif
#define SD_CMDQ_BUFF_SIZE_MAX (0x80000)
#define SD_CMDQ_BUFF_ALIGN    (64)

#define SD_MLOAD_BUFF_SIZE_MAX (0x100000)
#define SD_MLOAD_BUFF_ALIGN    (64)


//#define DBG_ERR(fmt, args...)  do {printk("[SD  ERR]: %s [%d]: ", __FUNCTION__, __LINE__);printk(fmt, ##args);}while(0)
//#define DBG_INFO(fmt, args...) do {printk("[SD INFO]: %s [%d]: ", __FUNCTION__, __LINE__);printk(fmt, ##args);}while(0)

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

#define MI_SD_CHECK_CHNN_SUPPORTED(SDCh)   (((SDCh) >= 0) && ((SDCh) < (MI_SD_MAX_CHANNEL_NUM)))
#define MI_SD_CHECK_PORT_SUPPORTED(PortNum) (((PortNum) >= 0) && ((PortNum) < (MI_SD_MAX_PORT_NUM)))
#define MI_SD_CHECK_CHNN_CREATED(SDCh)       (MI_SD_CHECK_CHNN_SUPPORTED((SDCh)) && (_gSDDevInfo.stChnnInfo[SDCh].bCreated == TRUE))
#define GET_SD_CHNN_PTR(SDCh)              (&_gSDDevInfo.stChnnInfo[(SDCh)])
#define GET_SD_PORT_PTR(SDCh, SDPort)     (&_gSDDevInfo.stChnnInfo[(SDCh)].stOutPortInfo[(SDPort)])
#define GET_SD_DEV_PTR()                    (&_gSDDevInfo)

#define  MI_SD_CHNN_STATIC_ATTR_CHANGED(pstOldAttr, pstNewAttr) \
        (((pstOldAttr)->u16MaxW != (pstNewAttr)->u16MaxW)\
            || ((pstOldAttr)->u16MaxH != (pstNewAttr)->u16MaxH)\
        )


#define GET_SD_BUFF_FRAME_FROM_TASK(pstChnTask) (&(pstChnTask)->astInputPortBufInfo[0]->stFrameData)

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------

static mi_SD_DevInfo_t _gSDDevInfo = {
    .u32MagicNumber= __MI_SD_DEV_MAGIC__,
    .bInited = FALSE,
    .hDevSysHandle = MI_HANDLE_NULL,
    .uSDIrqNum    = 0,
    .u32ChannelCreatedNum = 0,
};
// TODO: Tommy macro change to static inline function
#define MI_SD_CHNN_ATTR_OUT_OF_CAPS(pstDevInfo, pstSDChAttr) (!(!(((pstSDChAttr)->u16MaxW > MI_SD_CHANNEL_MAX_WIDTH) && ((pstSDChAttr)->u16MaxW * (pstSDChAttr)->u16MaxH) > (MI_SD_CHANNEL_MAX_WIDTH *MI_SD_CHANNEL_MAX_HEIGHT))\
            && (((pstDevInfo)->u64TotalCreatedArea + ((pstSDChAttr)->u16MaxW * (pstSDChAttr)->u16MaxH) <= 8*FHD_SIZE))\
            && (((pstDevInfo)->u32ChannelCreatedNum +  1) < MI_SD_MAX_CHANNEL_NUM)\
            ))

DECLARE_WAIT_QUEUE_HEAD(SD_isr_waitqueue);

LIST_HEAD(SD_todo_task_list);
LIST_HEAD(SD_working_task_list);
DEFINE_SEMAPHORE(SD_working_list_sem);

static MS_S32 _MI_SD_IMPL_MiSysAlloc(MS_U8 *pu8Name, MS_U32 u32Size, MS_PHYADDR * phyAddr)
{
    MI_SYS_BUG_ON(NULL == phyAddr);

    return (MS_S32)mi_sys_MMA_Alloc((MI_U8*)pu8Name, (MI_U32)u32Size, (MI_PHY *)phyAddr);
}
static MS_S32 _MI_SD_IMPL_MiSysFree (MS_PHYADDR phyAddr)
{
    return (MS_S32)mi_sys_MMA_Free((MI_PHY)phyAddr);
}

static MS_S32 _MI_SD_IMPL_MiSysFlashCache(void *pVirtAddr, MS_U32 u32Size)
{
    MI_SYS_BUG_ON(NULL == pVirtAddr);

    return (MS_S32)mi_sys_VFlushInvCache(pVirtAddr, (MI_U32)u32Size);
}

static void* _MI_SD_IMPL_MiSysMap(MS_PHYADDR u64PhyAddr, MS_U32 u32Size , MS_BOOL bCache)
{
    return mi_sys_Vmap((MI_PHY)u64PhyAddr, (MI_U32)u32Size , (MI_BOOL)bCache);
}

static void _MI_SD_IMPL_MiSysUnMap(void *pVirtAddr)
{
    MI_SYS_BUG_ON(NULL == pVirtAddr);
    return mi_sys_UnVmap(pVirtAddr);
}

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
// TODO: Tommy: Think about channel semaphore between  work thread and  user API event.
MI_S32 MI_SD_IMPL_CreateChannel(MI_SD_CHANNEL SDCh, MI_SD_ChannelAttr_t *pstSDChAttr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    mi_SD_ChannelInfo_t *pstChnnInfo = NULL;
    DBG_ENTER("chn%d, max(%dx%d), crop(%dx%d)\n", SDCh, pstSDChAttr->u16MaxW, pstSDChAttr->u16MaxH,
        pstSDChAttr->stCropRect.u16X, pstSDChAttr->stCropRect.u16Y, pstSDChAttr->stCropRect.u16Width, pstSDChAttr->stCropRect.u16Height);

    if (MI_SD_CHECK_CHNN_SUPPORTED(SDCh))
    {
        mi_SD_DevInfo_t *pstDevInfo = GET_SD_DEV_PTR();
        // Create channel
        MHalAllocPhyMem_t stAlloc;
        // SCL handle
        MHalVpeSclWinSize_t stMaxWin;
        MHalVpeSclInputSizeConfig_t stSclInputCfg;
        MHalVpeSclCreate_t stSdSclCreat;

        memset(&stSdSclCreat, 0, sizeof(MHalVpeSclCreate_t));
        pstChnnInfo = GET_SD_CHNN_PTR(SDCh);

        // Check created ?
        if (pstChnnInfo->bCreated == TRUE)
        {
            s32Ret = MI_ERR_SD_EXIST;
            DBG_EXIT_ERR("Channel id: %d already created.\n", SDCh);
            return s32Ret;
        }
        else if (MI_SD_CHNN_ATTR_OUT_OF_CAPS(pstDevInfo, pstSDChAttr))
        {
            DBG_WRN("Channel id: %d MaxW: %d MaxH: %d out of hardware Caps !!!\n", SDCh, pstSDChAttr->u16MaxW, pstSDChAttr->u16MaxH);
        }

         if ((pstSDChAttr->stCropRect.u16X != pstChnnInfo->stCropWin.u16X)
            || (pstSDChAttr->stCropRect.u16Y != pstChnnInfo->stCropWin.u16Y)
            || (pstSDChAttr->stCropRect.u16Width !=  pstChnnInfo->stCropWin.u16Width)
            || (pstSDChAttr->stCropRect.u16Height!=  pstChnnInfo->stCropWin.u16Height)
            )
        {

            pstChnnInfo->stCropWin.u16X = pstSDChAttr->stCropRect.u16X;
            pstChnnInfo->stCropWin.u16Y = pstSDChAttr->stCropRect.u16Y;
            pstChnnInfo->stCropWin.u16Width  = pstSDChAttr->stCropRect.u16Width;
            pstChnnInfo->stCropWin.u16Height = pstSDChAttr->stCropRect.u16Height;
            s32Ret = MI_SD_OK;
        }

        memset(pstChnnInfo, 0, sizeof(*pstChnnInfo));
        atomic_set(&pstChnnInfo->stAtomTask, 0);
        stAlloc.alloc = _MI_SD_IMPL_MiSysAlloc;
        stAlloc.free  = _MI_SD_IMPL_MiSysFree;
        stAlloc.map   = _MI_SD_IMPL_MiSysMap;
        stAlloc.unmap = _MI_SD_IMPL_MiSysUnMap;
        stAlloc.flush_cache = _MI_SD_IMPL_MiSysFlashCache;
        stMaxWin.u16Width = pstChnnInfo->stChnnAttr.u16MaxW = pstSDChAttr->u16MaxW;
        stMaxWin.u16Height= pstChnnInfo->stChnnAttr.u16MaxH = pstSDChAttr->u16MaxH;

        //eRunMode
        stSdSclCreat.enSclInstance = E_MHAL_VPE_SCL_SUB;
        stSdSclCreat.enTopIn = E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE;
        stSdSclCreat.enInputPortMode = E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME;
        stSdSclCreat.enOutmode = E_MHAL_SCL_OUTPUT_MODE_PORT2_M2M_MODE;
        stSdSclCreat.stMaxWin.u16Width = stMaxWin.u16Width;
        stSdSclCreat.stMaxWin.u16Height = stMaxWin.u16Height;
        if (FALSE == MHalVpeCreateSclInstance(&stAlloc, &stSdSclCreat, &pstChnnInfo->pSclCtx))
        {
            DBG_EXIT_ERR("Channel id: %d MHalSDCreateSclInstance Fail.\n", SDCh);
            goto error_create_scl;
        }

        // Init input width/height
        // Cap Window
        pstChnnInfo->u64PhyAddrOffset[0] = 0;
        pstChnnInfo->u64PhyAddrOffset[1] = 0;
        pstChnnInfo->u64PhyAddrOffset[2] = 0;
        memset(&pstChnnInfo->stSrcWin, 0, sizeof(pstChnnInfo->stSrcWin));
        memset(&pstChnnInfo->stCropWin, 0, sizeof(pstChnnInfo->stCropWin));
        memset(&pstChnnInfo->stRealCrop, 0, sizeof(pstChnnInfo->stRealCrop));

        {
            memset(&pstChnnInfo->stPeerInputPortInfo, 0, sizeof(pstChnnInfo->stPeerInputPortInfo));
            //memset(&pstChnnInfo->u32LumaData, 0, sizeof(pstChnnInfo->u32LumaData));
            memset(&pstChnnInfo->stOutPortInfo, 0, sizeof(pstChnnInfo->stOutPortInfo));
            pstDevInfo->u32ChannelCreatedNum++;
            pstChnnInfo->bCreated = TRUE;
            pstChnnInfo->SDCh    = SDCh;
            pstDevInfo->u64TotalCreatedArea += pstSDChAttr->u16MaxW * pstSDChAttr->u16MaxH;
            INIT_LIST_HEAD(&pstChnnInfo->list);

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            init_MUTEX(&pstChnnInfo->stChnnMutex);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            sema_init(&pstChnnInfo->stChnnMutex, 1);
#endif
            memset(&stSclInputCfg, 0, sizeof(stSclInputCfg));
            MHalVpeSclInputConfig(pstChnnInfo->pSclCtx, &stSclInputCfg);
            s32Ret = MI_SD_OK;
            DBG_EXIT_OK();
        }

        if(pstDevInfo->u32ChannelCreatedNum == 1
            && TRUE == pstDevInfo->bSupportIrq)
        {
            MHalVpeIrqEnableMode_e eIrqMode;
            eIrqMode = E_MHAL_SCL_IRQ_PORT2_M2M;
            // Enable SD IRQ
            MHalVpeSclEnableIrq(eIrqMode);
            pstDevInfo->bEnbaleIrq = TRUE;
            pstDevInfo->eIrqEnableMode = E_MHAL_SCL_IRQ_PORT2_M2M;
        }

    }
    else
    {
        s32Ret = MI_ERR_SD_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", SDCh);
    }

    return s32Ret;

error_create_scl:
    s32Ret = MI_ERR_SD_ILLEGAL_PARAM;
    DBG_EXIT_ERR("Channel id: %d illegal parameter.\n", SDCh);
    return s32Ret;
}

MI_S32 MI_SD_IMPL_DestroyChannel(MI_SD_CHANNEL SDCh)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    DBG_ENTER("chn%d\n", SDCh);

    if (MI_SD_CHECK_CHNN_CREATED(SDCh))
    {
        mi_SD_ChannelInfo_t *pstChnnInfo = GET_SD_CHNN_PTR(SDCh);
        mi_SD_DevInfo_t *pstDevInfo = GET_SD_DEV_PTR();

        // TODO: tommy: 1. check whether disable channel.
        // API set destroy, work thread check status and drop task.
        // Depend on channel

        // Wait Running Task Finish
        while (atomic_read(&pstChnnInfo->stAtomTask) != 0)
        {
            msleep(1000/HZ);
        }

        pstDevInfo->u32ChannelCreatedNum--;
        pstDevInfo->u64TotalCreatedArea -= pstChnnInfo->stChnnAttr.u16MaxW * pstChnnInfo->stChnnAttr.u16MaxH;
        pstChnnInfo->bCreated = FALSE;
        pstChnnInfo->eStatus  = E_MI_SD_CHANNEL_STATUS_DESTROYED;
        MHalVpeDestroySclInstance(pstChnnInfo->pSclCtx);
        s32Ret = MI_SD_OK;

        if(pstDevInfo->u32ChannelCreatedNum == 0
            && TRUE == pstDevInfo->bSupportIrq)
        {
            MHalVpeIrqEnableMode_e eIrqMode = E_MHAL_SCL_IRQ_OFFM2M;

            // Enable SD IRQ
            MHalVpeSclEnableIrq(eIrqMode);
            pstDevInfo->bEnbaleIrq = FALSE;
        }

        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_SD_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", SDCh);
    }

    return s32Ret;
}

MI_S32 MI_SD_IMPL_SetChannelAttr(MI_SD_CHANNEL SDCh, MI_SD_ChannelAttr_t *pstSDChAttr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    DBG_ENTER("chn%d, max(%dx%d), crop(%dx%d)\n", SDCh, pstSDChAttr->u16MaxW, pstSDChAttr->u16MaxH,
           pstSDChAttr->stCropRect.u16X, pstSDChAttr->stCropRect.u16Y, pstSDChAttr->stCropRect.u16Width, pstSDChAttr->stCropRect.u16Height);
// TODO: tommy: Add channel semaphore
    if (MI_SD_CHECK_CHNN_CREATED(SDCh))
    {
        mi_SD_ChannelInfo_t *pstChnnInfo = GET_SD_CHNN_PTR(SDCh);

        LOCK_CHNN(pstChnnInfo);
        if ((pstSDChAttr->stCropRect.u16X != pstChnnInfo->stCropWin.u16X)
            || (pstSDChAttr->stCropRect.u16Y != pstChnnInfo->stCropWin.u16Y)
            || (pstSDChAttr->stCropRect.u16Width !=  pstChnnInfo->stCropWin.u16Width)
            || (pstSDChAttr->stCropRect.u16Height!=  pstChnnInfo->stCropWin.u16Height)
            )
        {

            pstChnnInfo->stCropWin.u16X = pstSDChAttr->stCropRect.u16X;
            pstChnnInfo->stCropWin.u16Y = pstSDChAttr->stCropRect.u16Y;
            pstChnnInfo->stCropWin.u16Width  = pstSDChAttr->stCropRect.u16Width;
            pstChnnInfo->stCropWin.u16Height = pstSDChAttr->stCropRect.u16Height;

            s32Ret = MI_SD_OK;
        }
        else
        {
            s32Ret = MI_SD_OK;
            DBG_ERR("crop not change\n");
            DBG_EXIT_OK();
        }

        UNLOCK_CHNN(pstChnnInfo);
    }
    else
    {
        s32Ret = MI_ERR_SD_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", SDCh);
    }

    return s32Ret;
}

MI_S32 MI_SD_IMPL_GetChannelAttr(MI_SD_CHANNEL SDCh, MI_SD_ChannelAttr_t *pstSDChAttr)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    DBG_ENTER("chn%d\n", SDCh);
    // No need add semaphore.
    if (MI_SD_CHECK_CHNN_CREATED(SDCh))
    {
        memcpy(pstSDChAttr, &_gSDDevInfo.stChnnInfo[SDCh].stChnnAttr, sizeof(*pstSDChAttr));
        s32Ret = MI_SD_OK;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_SD_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", SDCh);
    }

    return s32Ret;
}

MI_S32 MI_SD_IMPL_StartChannel(MI_SD_CHANNEL SDCh)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    DBG_ENTER("chn%d\n", SDCh);

    if (MI_SD_CHECK_CHNN_CREATED(SDCh))
    {
        mi_SD_ChannelInfo_t *pstChnnInfo = GET_SD_CHNN_PTR(SDCh);
        mi_SD_DevInfo_t     *pstDevInfo = GET_SD_DEV_PTR();
        LOCK_CHNN(pstChnnInfo);
       if(pstChnnInfo->eStatus == E_MI_SD_CHANNEL_STATUS_START)
        {
           MI_SYS_BUG_ON(list_empty(&pstChnnInfo->list));
           UNLOCK_CHNN(pstChnnInfo);
           return s32Ret;//
        }
        pstChnnInfo->eStatus = E_MI_SD_CHANNEL_STATUS_START;

        UNLOCK_CHNN(pstChnnInfo);

        mi_sys_EnableInputPort(pstDevInfo->hDevSysHandle, SDCh, 0);


       mi_sys_EnableOutputPort(pstDevInfo->hDevSysHandle, SDCh, 0); // portID < portNUM
        mi_sys_EnableChannel(pstDevInfo->hDevSysHandle, SDCh);
        s32Ret = MI_SD_OK;

        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_SD_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", SDCh);
    }

    DBG_EXIT_OK();

    return s32Ret;
}

MI_S32 MI_SD_IMPL_StopChannel(MI_SD_CHANNEL SDCh)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    DBG_ENTER("chn%d\n", SDCh);

    if (MI_SD_CHECK_CHNN_CREATED(SDCh))
    {
        mi_SD_ChannelInfo_t *pstChnnInfo = GET_SD_CHNN_PTR(SDCh);
        mi_SD_DevInfo_t     *pstDevInfo = GET_SD_DEV_PTR();

        LOCK_CHNN(pstChnnInfo);
        if(pstChnnInfo->eStatus != E_MI_SD_CHANNEL_STATUS_START)
        {
           MI_SYS_BUG_ON(!list_empty(&pstChnnInfo->list));
           UNLOCK_CHNN(pstChnnInfo);
           return s32Ret;//
        }

        pstChnnInfo->eStatus = E_MI_SD_CHANNEL_STATUS_STOP;

        UNLOCK_CHNN(pstChnnInfo);

        mi_sys_DisableChannel(pstDevInfo->hDevSysHandle, SDCh);
        mi_sys_DisableOutputPort(pstDevInfo->hDevSysHandle, SDCh, 0);
        mi_sys_DisableInputPort(pstDevInfo->hDevSysHandle, SDCh, 0);
        s32Ret = MI_SD_OK;

        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_SD_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", SDCh);
    }

    return s32Ret;
}

MI_S32 MI_SD_IMPL_SetOutputPortAttr(MI_SD_CHANNEL SDCh, MI_SD_OuputPortAttr_t *pstSDMode)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_PORT SDPort =0;
    DBG_ENTER("chn%d, size(%dx%d), pixel %d, compress %d\n",SDCh, pstSDMode->u16Width,pstSDMode->u16Height,pstSDMode->ePixelFormat,pstSDMode->eCompressMode);

    if (MI_SD_CHECK_CHNN_CREATED(SDCh))
    {
        if (MI_SD_CHECK_PORT_SUPPORTED(SDPort))
        {
            mi_SD_ChannelInfo_t  *pstChnnInfo    = GET_SD_CHNN_PTR(SDCh);
            mi_SD_OutPortInfo_t *pstOutPortInfo = GET_SD_PORT_PTR(SDCh, SDPort);

            LOCK_CHNN(pstChnnInfo);
            s32Ret = MI_SD_OK;
#if 0
            if ((pstSDMode->u16Width != pstOutPortInfo->stPortMode.u16Width)
                || (pstSDMode->u16Height!= pstOutPortInfo->stPortMode.u16Height))
        #endif
            {
                MHalVpeSclOutputSizeConfig_t stOutput;
                // Need check as foo ?
                // memset(&stOutput, 0, sizeof(stOutput));
                stOutput.enOutPort = (MHalVpeDmaPort_e)PORTID;
                stOutput.u16Width  = pstSDMode->u16Width;
                stOutput.u16Height = pstSDMode->u16Height;
                if (TRUE == MHalVpeSclPortSizeConfig(pstChnnInfo->pSclCtx, &stOutput))
                {
                    pstOutPortInfo->stPortMode.u16Width = pstSDMode->u16Width;
                    pstOutPortInfo->stPortMode.u16Height= pstSDMode->u16Height;

                    DBG_EXIT_OK();
                }
                else
                {
                    s32Ret = MI_ERR_SD_ILLEGAL_PARAM;
                    DBG_EXIT_ERR("Ch: %d port %d set output size fail.\n", SDCh, SDPort);
                }
            }

            // Tommy: Check Caps: Pixel format per frame change in ISP ????
            if (1)//(s32Ret == MI_SD_OK)
            {

                #if 0
                if ((pstOutPortInfo->stPortMode.ePixelFormat != pstSDMode->ePixelFormat)
                    ||(pstOutPortInfo->stPortMode.eCompressMode != pstSDMode->eCompressMode))
                    #endif
                if (1)
                {
                    MHalVpeSclDmaConfig_t stDmaCfg;
                    memset(&stDmaCfg, 0, sizeof(stDmaCfg));
                    stDmaCfg.enPort = (MHalVpeDmaPort_e)PORTID;
                    stDmaCfg.enCompress  = E_MHAL_COMPRESS_MODE_NONE;
                    stDmaCfg.enFormat = pstSDMode->ePixelFormat;
                    stDmaCfg.enCompress  = pstSDMode->eCompressMode;

                    if (TRUE == MHalVpeSclDmaPortConfig(pstChnnInfo->pSclCtx, &stDmaCfg))
                    {
                        pstOutPortInfo->stPortMode.ePixelFormat = pstSDMode->ePixelFormat;
                        pstOutPortInfo->stPortMode.eCompressMode= pstSDMode->eCompressMode;
                        //pstOutPortInfo->eRealOutputPixelFormat  = pstSDMode->ePixelFormat;
                        DBG_EXIT_OK();
                    }
                    else
                    {
                        s32Ret = MI_ERR_SD_ILLEGAL_PARAM;
                        DBG_EXIT_ERR("Ch: %d port %d set output size fail.\n", SDCh, SDPort);
                    }
                }

                pstOutPortInfo->stPortMode = *pstSDMode;
            }

            UNLOCK_CHNN(pstChnnInfo);
        }
        else
        {
            s32Ret = MI_ERR_SD_INVALID_PORTID;
            DBG_EXIT_ERR("Invalid port id: %d.\n", SDPort);
        }
    }
    else
    {
        s32Ret = MI_ERR_SD_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", SDCh);
    }

    return s32Ret;
}

MI_S32 MI_SD_IMPL_GetOutputPortAttr(MI_SD_CHANNEL SDCh, MI_SD_OuputPortAttr_t *pstSDMode)
{
    MI_S32 s32Ret = MI_ERR_SD_BUSY;
    MI_SD_PORT SDPort =0;
    DBG_ENTER("chn%d\n", SDCh);

    if (MI_SD_CHECK_CHNN_CREATED(SDCh))
    {
        if (MI_SD_CHECK_PORT_SUPPORTED(SDPort))
        {
            mi_SD_OutPortInfo_t *pstOutPortInfo = GET_SD_PORT_PTR(SDCh, SDPort);
            *pstSDMode = pstOutPortInfo->stPortMode;
            s32Ret = MI_SD_OK;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_SD_INVALID_PORTID;
            DBG_EXIT_ERR("Invalid port id: %d.\n", SDPort);
        }
    }
    else
    {
        s32Ret = MI_ERR_SD_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", SDCh);
    }

    return s32Ret;
}

#if USE_CMDQ
static MI_BOOL _MI_SD_IsFenceL(MI_U16 u16Fence1, MI_U16 u16Fence2)
{
    if(u16Fence1<u16Fence2)
    {
        if((u16Fence1 + (0xffff-u16Fence2)) < 0x7FFF)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

static MI_U16 _MI_SD_ReadFence(MHAL_CMDQ_CmdqInterface_t *cmdinf)
{
    MI_U16 u16Value = 0;
    cmdinf->MHAL_CMDQ_ReadDummyRegCmdq(cmdinf, &u16Value);
    return u16Value;
}
#endif

static void _MI_SD_DebugGetBufCnt(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_U8 u8PortId = 0;

    for(u8PortId = 0; u8PortId <MI_SD_MAX_PORT_NUM; u8PortId++)
    {
        if(pstChnTask->astOutputPortBufInfo[u8PortId])
        {
            mi_SD_OutPortInfo_t *pstPortInfo = NULL;
            pstPortInfo = GET_SD_PORT_PTR(pstChnTask->u32ChnId, u8PortId);
            pstPortInfo->u64FinishOutputBufferCnt++;
            pstPortInfo->u32FrameCntpers ++;
            if(jiffies - pstPortInfo->u32FrameCntJiffies > HZ)
            {
                pstPortInfo->u8PortFps = pstPortInfo->u32FrameCntpers;
                pstPortInfo->u32FrameCntpers = 0;
                pstPortInfo->u32FrameCntJiffies = jiffies;
            }
        }
    }
}

static void _mi_SD_DebugWriteFile(MI_U8 * pu8Name, MI_U8 *pu8Buf, MI_U32 u32Len, MI_U32 u32Offset)
{
    struct file *fp = NULL;
    mm_segment_t fs;

    fp =filp_open(pu8Name, O_RDWR | O_CREAT, 0777);
    if (IS_ERR(fp))
    {
        DBG_ERR("Open File Faild  PTR_ERR_fp=%ld\n",PTR_ERR(fp));
        return;
    }

    fs =get_fs();
    set_fs(KERNEL_DS);
    fp->f_op->llseek(fp, u32Offset, SEEK_SET);

    if(fp->f_op->write(fp, pu8Buf, u32Len, &(fp->f_pos)) != u32Len)
    {
        DBG_ERR("fwrite %s failed\n", pu8Name);
    }
    else
    {
        DBG_WRN("dump file(%s) v1 ok ..............[len:%d]\n", pu8Name, u32Len);
    }

    set_fs(fs);
    filp_close(fp,NULL);
}


static void _MI_SD_WriteTaskFile(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_U8 u8PortId =0;
    static MI_U8 au8FileName[128];
    void *pY = NULL, *pUV = NULL;
    mi_SD_OutPortInfo_t *pstPortInfo =GET_SD_PORT_PTR(pstChnTask->u32ChnId, 0);

    if(pstChnTask->astOutputPortBufInfo[u8PortId] != NULL)
    {
         MI_U16 u16Width = pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.u16Width;
         MI_U16 u16Height = pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.u16Height;

        if(pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
        {
            MI_U32 u32Size = u16Width*u16Height*2;
            pY = mi_sys_Vmap(pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.phyAddr[0], u32Size, FALSE);
            if(pY == NULL)
                DBG_ERR("PY MMAP fail\n");

            sprintf(au8FileName, "/mnt/sd_outport%d_%dx%d_yuv422.yuv",u8PortId, u16Width, u16Height);
            _mi_SD_DebugWriteFile(au8FileName, pY, u32Size, pstPortInfo->u32OutOffset);
            pstPortInfo->u32OutOffset += u32Size;

            mi_sys_UnVmap(pY);
        }

        if(pstChnTask->astInputPortBufInfo[0]->stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
        {
            MI_U32 u32YSize = pstChnTask->astInputPortBufInfo[0]->stFrameData.u16Width*pstChnTask->astInputPortBufInfo[0]->stFrameData.u16Height;
            //MI_U32 u32UVSize = u16Width*u16Height/2;

             pY = mi_sys_Vmap(pstChnTask->astInputPortBufInfo[0]->stFrameData.phyAddr[0], u32YSize, FALSE);
             //pUV = mi_sys_Vmap(pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.phyAddr[1], u32UVSize, FALSE);
            if(pY  == NULL)
                DBG_ERR("MMAP fail PY %p, PUV %p\n", pY, pUV);

            sprintf(au8FileName, "/mnt/sd_Inport%d_%dx%d_yuv422sp.yuv",u8PortId, u16Width, u16Height);
            _mi_SD_DebugWriteFile(au8FileName, pY, u32YSize, pstPortInfo->u32InOffset);
            pstPortInfo->u32InOffset += u32YSize;
            //_mi_vpe_DebugWriteFile(au8FileName, pUV, u32UVSize, u32YSize);
             mi_sys_UnVmap(pY);
            // mi_sys_UnVmap(pUV);
        }

        if(pstChnTask->astInputPortBufInfo[0]->stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
        {
            MI_U32 u32YSize = pstChnTask->astInputPortBufInfo[0]->stFrameData.u16Width*pstChnTask->astInputPortBufInfo[0]->stFrameData.u16Height;
            MI_U32 u32UVSize = u16Width*u16Height/2;

             pY = mi_sys_Vmap(pstChnTask->astInputPortBufInfo[u8PortId]->stFrameData.phyAddr[0], u32YSize, FALSE);
             pUV = mi_sys_Vmap(pstChnTask->astInputPortBufInfo[u8PortId]->stFrameData.phyAddr[1], u32UVSize, FALSE);
            if(pY && pUV  == NULL)
                DBG_ERR("MMAP fail PY %p, PUV %p\n", pY, pUV);

            sprintf(au8FileName, "/mnt/sd_port%d_%dx%d_yuv420sp.yuv",u8PortId, u16Width, u16Height);
            _mi_SD_DebugWriteFile(au8FileName, pY, u32YSize, pstPortInfo->u32InOffset);
            pstPortInfo->u32InOffset += u32YSize;
            _mi_SD_DebugWriteFile(au8FileName, pUV, u32UVSize,  pstPortInfo->u32InOffset);
             pstPortInfo->u32InOffset += u32UVSize;
             mi_sys_UnVmap(pY);
             mi_sys_UnVmap(pUV);
        }
    }
}



static int _MI_SD_IsrProcThread(void *data)
{
    mi_SD_DevInfo_t *pstDevInfo  = (mi_SD_DevInfo_t *)data;
    #if USE_CMDQ
    MHAL_CMDQ_CmdqInterface_t *cmdinf = pstDevInfo->pstCmdMloadInfo;
    #endif
    MI_S32 s32Ret = 0;
    struct list_head* pos, *n;
    MI_BOOL bTaskHasNoOutToDram = FALSE;
    mi_sys_ChnTaskInfo_t *pstLastPreprocessNotifyTask = NULL;
    while (!kthread_should_stop())
    {
        //DBG_INFO("Proc get data.\n");
        if (pstDevInfo->u32TaskNoToDramCnt != 0)
        {
            bTaskHasNoOutToDram = TRUE;
        }

        if (bTaskHasNoOutToDram == TRUE)
        {
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            interruptible_sleep_on_timeout(&SD_isr_waitqueue, msecs_to_jiffies(1));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            wait_event_interruptible_timeout(SD_isr_waitqueue, atomic_read(&pstDevInfo->wqRedFlag) > 0, msecs_to_jiffies(1));
#endif
            atomic_dec(&pstDevInfo->wqRedFlag);
        }
        else
        {
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            interruptible_sleep_on_timeout(&SD_isr_waitqueue, msecs_to_jiffies(SD_PROC_WAIT));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            wait_event_interruptible_timeout(SD_isr_waitqueue, atomic_read(&pstDevInfo->wqRedFlag) > 0, msecs_to_jiffies(SD_PROC_WAIT));
#endif
            atomic_dec(&pstDevInfo->wqRedFlag);
        }

        down(&SD_working_list_sem);
        if (list_empty(&SD_working_task_list))
        {
            up(&SD_working_list_sem);
            continue;
        }

        list_for_each_safe(pos, n, &SD_working_task_list)
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            mi_SD_ChannelInfo_t *pSDChnInfo = NULL;
            pstChnTask = container_of(SD_working_task_list.next, mi_sys_ChnTaskInfo_t, listChnTask);
            pSDChnInfo = GET_SD_CHNN_PTR(pstChnTask->u32ChnId);
            if (pstChnTask == NULL)
            {
                DBG_ERR("pstChnTask %p\n", pstChnTask);
                break;
            }
            DBG_INFO(" got data.\n");

            //the current should be the ongoing task or done task
            if(pstLastPreprocessNotifyTask != pstChnTask)
            {
                int i;
                MI_S32 ret=0;

                for(i=0;i<MI_SYS_MAX_OUTPUT_PORT_CNT; i++)
                {
                   if(!pstChnTask->astOutputPortBufInfo[i])
                       continue;

                   ret = mi_sys_NotifyPreProcessBuf(pstChnTask->astOutputPortBufInfo[i]);
                   if(ret != MI_SUCCESS)
                       MI_SYS_BUG();
                }
                pstLastPreprocessNotifyTask = pstChnTask;
            }

            // Task has not been finished yet.
            #if USE_CMDQ
            if(_MI_SD_IsFenceL(pstDevInfo->u16CmdqFence, pstChnTask->u32Reserved0) == TRUE)
            {
                MS_BOOL bIdleVal = FALSE;
                cmdinf->MHAL_CMDQ_IsCmdqEmptyIdle(cmdinf, &bIdleVal);
                if(bIdleVal == FALSE) //cmdQ is Running
                {
                    //DBG_INFO("invalid fence %04x, %04x!\n", _MI_SD_ReadFence(cmdinf), pstChnTask->u32Reserved0);
                    break;
                }
                else // cmdQ is stop
                {
                    if(_MI_SD_IsFenceL(pstDevInfo->u16CmdqFence, pstChnTask->u32Reserved0) == TRUE)
                    {
                        //DBG_ERR("invalid fence %04x, %04x!\n", pstDevInfo->u16CmdqFence, pstChnTask->u32Reserved0);
                       // MI_SYS_BUG();
                    }
                    else
                    {
                        break;
                    }
                }
            }
            #endif

            DBG_INFO("Finish data.\n");
            // Task already finished.
            // Remove task from working list
            list_del(&pstChnTask->listChnTask);
            pstLastPreprocessNotifyTask = NULL;

            if (pstChnTask->u32Reserved1 & MI_SD_TASK_NO_DRAM_OUTPUT)
            {
                pstDevInfo->u32TaskNoToDramCnt--;
            }

            MI_SYS_BUG_ON(atomic_read(&pstDevInfo->stChnnInfo[pstChnTask->u32ChnId].stAtomTask) == 0);
            atomic_dec_return(&pstDevInfo->stChnnInfo[pstChnTask->u32ChnId].stAtomTask);

            _MI_SD_DebugGetBufCnt(pstChnTask);

            if(pSDChnInfo->u16DumpTaskFileNum > 0)
            {
                _MI_SD_WriteTaskFile(pstChnTask);
                pSDChnInfo->u16DumpTaskFileNum --;
            }

            s32Ret = mi_sys_FinishAndReleaseTask(pstChnTask);

            DBG_INFO("s32Ret = %d.\n", s32Ret);
            DBG_INFO("Release data.\n");
        }
        up(&SD_working_list_sem);
    }
        // release working task list
    down(&SD_working_list_sem);
    while(!list_empty(&SD_working_task_list))
    {
        mi_sys_ChnTaskInfo_t *pstChnTask;
        pstChnTask = container_of(SD_working_task_list.next, mi_sys_ChnTaskInfo_t, listChnTask);
        if (pstChnTask != NULL)
        {
            list_del(&pstChnTask->listChnTask);
            mi_sys_RewindTask(pstChnTask);
        }
    }
    up(&SD_working_list_sem);
    return 0;
}

static irqreturn_t _MI_SD_Isr(int irq, void *data)
{
    MHalVpeIrqStatus_e eIrqStatus;
    mi_SD_DevInfo_t *pstDevInfo = (mi_SD_DevInfo_t *)data;

    memset(&eIrqStatus, 0, sizeof(eIrqStatus));

    eIrqStatus = MHalVpeSclCheckIrq(pstDevInfo->eIrqEnableMode);
    if (eIrqStatus > E_MHAL_SCL_IRQ_NONE)
    {
        MHalVpeSclClearIrq(pstDevInfo->eIrqEnableMode);
        #if USE_CMDQ
        pstDevInfo->u16CmdqFence = _MI_SD_ReadFence(pstDevInfo->pstCmdMloadInfo);
        #endif

        atomic_inc(&pstDevInfo->wqRedFlag);
        WAKE_UP_QUEUE_IF_NECESSARY(SD_isr_waitqueue);
    }

    return IRQ_HANDLED;
}

static mi_sys_TaskIteratorCBAction_e _MI_SD_TaskIteratorCallBK(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData)
{
    int valid_output_port_cnt = 0;
    mi_SD_IteratorWorkInfo_t *workInfo = (mi_SD_IteratorWorkInfo_t *)pUsrData;
    mi_SD_ChannelInfo_t *pstChnnInfo = GET_SD_CHNN_PTR(pstTaskInfo->u32ChnId);
    //mi_SD_DevInfo_t *pstDevInfo = GET_SD_DEV_PTR();


    LOCK_CHNN(pstChnnInfo);
    DBG_INFO("Chnn: %d got data.\n",  pstTaskInfo->u32ChnId);

    pstChnnInfo->u64GetInputBufferCnt += 1;

    // Check Channel stop or created ??
    if ((pstChnnInfo->bCreated == FALSE) ||
        (pstChnnInfo->eStatus != E_MI_SD_CHANNEL_STATUS_START)
        )
    {
        // Drop can not process input buffer
        mi_sys_FinishAndReleaseTask(pstTaskInfo);
        DBG_EXIT_ERR("Ch %d is not create or Ch %d is stop. Drop frame directly.\n",
                        pstTaskInfo->u32ChnId, pstTaskInfo->u32ChnId);
        UNLOCK_CHNN(pstChnnInfo);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }

    #if 0
    if (pstTaskInfo->astInputPortBufInfo[0]->eBufType != E_MI_SYS_BUFDATA_FRAME)
    {
         // Drop can not process input buffer
        DBG_EXIT_ERR("Ch %d is not support buffer Framedate  %d .\n",
            pstTaskInfo->u32ChnId, pstTaskInfo->astInputPortBufInfo[0]->eBufType);
        mi_sys_FinishAndReleaseTask(pstTaskInfo);

        UNLOCK_CHNN(pstChnnInfo);
        MI_SYS_BUG();
        //return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }
    #endif

        pstTaskInfo->astOutputPortPerfBufConfig[0].u64TargetPts = pstTaskInfo->astInputPortBufInfo[0]->u64Pts;
        pstTaskInfo->astOutputPortPerfBufConfig[0].eBufType     = E_MI_SYS_BUFDATA_FRAME;
        pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.eFormat = pstChnnInfo->stOutPortInfo[0].stPortMode.ePixelFormat;
        pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.eFrameScanMode =  pstTaskInfo->astInputPortBufInfo[0]->stFrameData.eFrameScanMode;
        pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.u16Width = pstChnnInfo->stOutPortInfo[0].stPortMode.u16Width;
        pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.u16Height= pstChnnInfo->stOutPortInfo[0].stPortMode.u16Height;

        DBG_INFO("[%d]->{ u64TargetPts: 0x%llx eBufType: %d, eFormat = %d, eFrameScanMode: %d u16Width = %d, u16Height: %d.\n",
            0,
            pstTaskInfo->astOutputPortPerfBufConfig[0].u64TargetPts, pstTaskInfo->astOutputPortPerfBufConfig[0].eBufType,
            pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.eFormat, pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.eFrameScanMode, pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.u16Width,
            pstTaskInfo->astOutputPortPerfBufConfig[0].stFrameCfg.u16Height
        );

    UNLOCK_CHNN(pstChnnInfo);

    if(mi_sys_PrepareTaskOutputBuf(pstTaskInfo) != MI_SUCCESS)
    {
        DBG_EXIT_ERR("Ch %d mi_sys_PrepareTaskOutputBuf failed.\n",
                        pstTaskInfo->u32ChnId);
        return MI_SYS_ITERATOR_SKIP_CONTINUTE;
    }

    {
        // Incase SYS mask output as disable according FRC, port output buffer must be NULL.
        MI_SYS_BUG_ON(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[0] && pstTaskInfo->astOutputPortBufInfo[0]);

         if(pstTaskInfo->astOutputPortBufInfo[0])
        {
            valid_output_port_cnt++;
            pstChnnInfo->stOutPortInfo[0].u64GetOutputBufferCnt ++;
        }
        else
        {
            if(pstChnnInfo->eStatus == E_MI_SD_CHANNEL_STATUS_START)
            {
                DBG_INFO("channel start not get buffer\n");
                pstChnnInfo->stOutPortInfo[0].u64GetOutputBuffFailCnt ++;
            }
        }
    }

    //check if lack of output buf
    if(valid_output_port_cnt==0)
    {
#ifdef  MI_SYS_SERIOUS_ERR_MAY_MULTI_TIMES_SHOW
                DBG_EXIT_ERR("Ch %d valid_output_port_cnt %d.\n",
                        pstTaskInfo->u32ChnId, valid_output_port_cnt);
#endif
        return MI_SYS_ITERATOR_SKIP_CONTINUTE;
    }

    //TODO Just for Test
    pstTaskInfo->u32Reserved0 = 0;
    pstTaskInfo->u32Reserved1 = 0;
    pstTaskInfo->u64Reserved0 = 0;
    pstTaskInfo->u64Reserved1 = 0;

    atomic_inc(&pstChnnInfo->stAtomTask);

    list_add_tail(&pstTaskInfo->listChnTask, &SD_todo_task_list);

    //we at most process 32 batches at one time
    if(++workInfo->totalAddedTask >= MI_SD_FRAME_PER_BURST_CMDQ)
    {
        DBG_INFO("Chnn: %d workInfo->totalAddedTask: %d stop.\n",  pstTaskInfo->u32ChnId, workInfo->totalAddedTask);
        return MI_SYS_ITERATOR_ACCEPT_STOP;
    }
    else
    {
        DBG_INFO("Chnn: %d workInfo->totalAddedTask: %d.\n",  pstTaskInfo->u32ChnId, workInfo->totalAddedTask);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }
}


// TODO: implement by region ownner
//extern MI_S32 SD_osd_process(MI_REG_CmdInfo_t CmdInfo, MHAL_CMDQ_CmdqInterface_t *cmdinf);

static void _MI_SD_ProcessTask(mi_sys_ChnTaskInfo_t *pstTask, mi_SD_DevInfo_t *pstDevInfo, mi_SD_InBuffInfo_t *pstInBuffInfo)
{
    int i = 0;
    mi_SD_ChannelInfo_t *pstChnnInfo = &pstDevInfo->stChnnInfo[pstTask->u32ChnId];
    MHalVpeSclOutputBufferConfig_t stSDOutputBuffer;
    MHalVpeWaitDoneType_e eDoneType = E_MHAL_VPE_WAITDONE_ERR;
    mi_SD_OutPortInfo_t *pstPortInfo = GET_SD_PORT_PTR(pstTask->u32ChnId, 0);

    memset(&stSDOutputBuffer, 0, sizeof(stSDOutputBuffer));

    // Init reserved info
    //pstTask->u32Reserved1 = 0;
    // TODO: Tommy: Think about output Buffer ALL NULL
    // Port enable + port no buffer
    if ((pstChnnInfo->eStatus == E_MI_SD_CHANNEL_STATUS_START) && (NULL != pstTask->astOutputPortBufInfo[0]))
    {
        if (pstPortInfo->eRealOutputPixelFormat != pstTask->astOutputPortBufInfo[i]->stFrameData.ePixelFormat)
        {
            MHalVpeSclDmaConfig_t stDmaCfg;
            memset(&stDmaCfg, 0, sizeof(stDmaCfg));
            stDmaCfg.enPort = (MHalVpeDmaPort_e)PORTID;
            stDmaCfg.enCompress= pstPortInfo->stPortMode.eCompressMode;
            stDmaCfg.enFormat=pstTask->astOutputPortBufInfo[i]->stFrameData.ePixelFormat;
            if (TRUE == MHalVpeSclDmaPortConfig(pstChnnInfo->pSclCtx, &stDmaCfg))
            {
                pstPortInfo->eRealOutputPixelFormat = stDmaCfg.enFormat;
            }
            else
            {
                DBG_ERR("Ch: %d port %d set output size fail.\n", pstTask->u32ChnId, PORTID);
            }
        }
    }

    // Update SCL OutputBuffer
    // TODO: Tommy: Think about output Buffer ALL NULL
    // Port enable + port no buffer
    if ((pstChnnInfo->eStatus == E_MI_SD_CHANNEL_STATUS_START) && (NULL != pstTask->astOutputPortBufInfo[0]))
    {
        // Tommy: need add check output buffer --> disp window change ??
        if ((pstTask->astOutputPortBufInfo[0]->stFrameData.u16Width != pstPortInfo->stPortMode.u16Width)
            || (pstTask->astOutputPortBufInfo[0]->stFrameData.u16Height != pstPortInfo->stPortMode.u16Height)
            )
        {
            MHalVpeSclOutputSizeConfig_t stOutputSize;

            pstPortInfo->stPortMode.u16Width = pstTask->astOutputPortBufInfo[0]->stFrameData.u16Width;
            pstPortInfo->stPortMode.u16Height = pstTask->astOutputPortBufInfo[0]->stFrameData.u16Height;

            memset(&stOutputSize, 0, sizeof(stOutputSize));
            stOutputSize.enOutPort = (MHalVpeDmaPort_e)(PORTID);
            stOutputSize.u16Width  = pstPortInfo->stPortMode.u16Width;
            stOutputSize.u16Height = pstPortInfo->stPortMode.u16Height;
            MHalVpeSclPortSizeConfig(pstChnnInfo->pSclCtx, &stOutputSize);
        }

        // Enable
        stSDOutputBuffer.stCfg[PORTID].bEn = TRUE;
        // Address

        // portID is 0 or 2 ???????
        stSDOutputBuffer.stCfg[PORTID].stBufferInfo.u32Stride[0] = pstTask->astOutputPortBufInfo[0]->stFrameData.u32Stride[0];
        stSDOutputBuffer.stCfg[PORTID].stBufferInfo.u32Stride[1] = pstTask->astOutputPortBufInfo[0]->stFrameData.u32Stride[1];
        stSDOutputBuffer.stCfg[PORTID].stBufferInfo.u32Stride[2] = pstTask->astOutputPortBufInfo[0]->stFrameData.u32Stride[2];
        stSDOutputBuffer.stCfg[PORTID].stBufferInfo.u64PhyAddr[0] = pstTask->astOutputPortBufInfo[0]->stFrameData.phyAddr[0];
        stSDOutputBuffer.stCfg[PORTID].stBufferInfo.u64PhyAddr[1] = pstTask->astOutputPortBufInfo[0]->stFrameData.phyAddr[1];
        stSDOutputBuffer.stCfg[PORTID].stBufferInfo.u64PhyAddr[2] = pstTask->astOutputPortBufInfo[0]->stFrameData.phyAddr[2];
    }
    else
    {
        stSDOutputBuffer.stCfg[PORTID].bEn = FALSE;
    }


    stSDOutputBuffer.stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u32Stride[0] = pstInBuffInfo->u32Stride[0];
    stSDOutputBuffer.stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u32Stride[1] = pstInBuffInfo->u32Stride[1];
    stSDOutputBuffer.stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u32Stride[2] = pstInBuffInfo->u32Stride[2];
    stSDOutputBuffer.stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u64PhyAddr[0] = pstInBuffInfo->phyAddr[0];
    stSDOutputBuffer.stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u64PhyAddr[1] = pstInBuffInfo->phyAddr[1];
    stSDOutputBuffer.stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u64PhyAddr[2] = pstInBuffInfo->phyAddr[2];
    stSDOutputBuffer.stCfg[E_MHAL_M2M_INPUT_PORT].bEn = TRUE;

    MHalVpeSclProcess(pstChnnInfo->pSclCtx, pstDevInfo->pstCmdMloadInfo, &stSDOutputBuffer);

    // Wait MDWIN Done

   eDoneType = E_MHAL_VPE_WAITDONE_DMAONLY;
   MHalVpeSclSetWaitDone(pstChnnInfo->pSclCtx, pstDevInfo->pstCmdMloadInfo, eDoneType);
}

static MI_BOOL _MI_SD_CheckInputChanged(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_BOOL bRet = FALSE;
    mi_SD_ChannelInfo_t *pstChnInfo = GET_SD_CHNN_PTR(pstChnTask->u32ChnId);
    mi_vif_InputInfo_t *pstMetaDataInfo = NULL;
    MI_SYS_FrameData_t *pstBuffFrame = NULL;
    mi_SD_InBuffInfo_t stInBuffInfo;

    memset(&stInBuffInfo, 0, sizeof(stInBuffInfo));

    if(pstChnTask->astInputPortBufInfo[0]->eBufType == E_MI_SYS_BUFDATA_META)
    {
        pstMetaDataInfo = (mi_vif_InputInfo_t *)pstChnTask->astInputPortBufInfo[0]->stMetaData.pVirAddr;
        stInBuffInfo.u16Width = pstMetaDataInfo->u16Width;
        stInBuffInfo.u16Height = pstMetaDataInfo->u16Height;
    }
    else if(pstChnTask->astInputPortBufInfo[0]->eBufType == E_MI_SYS_BUFDATA_FRAME)
    {
        pstBuffFrame = GET_SD_BUFF_FRAME_FROM_TASK(pstChnTask);
        stInBuffInfo.u16Width = pstBuffFrame->u16Width;
        stInBuffInfo.u16Height = pstBuffFrame->u16Height;
        stInBuffInfo.eCompressMode = pstBuffFrame->eCompressMode;
        stInBuffInfo.ePixelFormat = pstBuffFrame->ePixelFormat;
    }

    // Check source window changed ?
    DBG_INFO("pstChnInfo->stSrcWin.u16Width %d X pstBuffFrame->u16Width: %d.\n", pstChnInfo->stSrcWin.u16Width , stInBuffInfo.u16Width);
    DBG_INFO("pstChnInfo->stSrcWin.u16Height %d X pstBuffFrame->u16Height: %d.\n", pstChnInfo->stSrcWin.u16Height , stInBuffInfo.u16Height);

    if ((pstChnInfo->stSrcWin.u16Width == stInBuffInfo.u16Width)
        && (pstChnInfo->stSrcWin.u16Height == stInBuffInfo.u16Height)
        &&(pstChnInfo->stSrcWin.eCompressMode == stInBuffInfo.eCompressMode)
        &&(pstChnInfo->eInPixFmt == stInBuffInfo.ePixelFormat)
        )
    {
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
        pstChnInfo->stSrcWin.u16Width = stInBuffInfo.u16Width;
        pstChnInfo->stSrcWin.u16Height= stInBuffInfo.u16Height;
        pstChnInfo->stSrcWin.eCompressMode= stInBuffInfo.eCompressMode;
        pstChnInfo->eInPixFmt = stInBuffInfo.ePixelFormat;
    }

    return bRet;
}
// TODO:  Tommy: Add PerChannel semaphore for global var.

static MI_BOOL _MI_SD_CheckCropChanged(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_BOOL bRet = FALSE;
    MI_SYS_WindowRect_t stRealCrop = {0,0,0,0};
    mi_SD_ChannelInfo_t *pstChnInfo = GET_SD_CHNN_PTR(pstChnTask->u32ChnId);

    // Calculator real cropWindow
    if ((pstChnInfo->stCropWin.u16X + pstChnInfo->stCropWin.u16Width) > pstChnInfo->stSrcWin.u16Width)
    {
        bRet = TRUE;
        stRealCrop.u16X     = 0;
        stRealCrop.u16Width = pstChnInfo->stSrcWin.u16Width;
    }
    else
    {
        bRet = FALSE;
        stRealCrop.u16X     = pstChnInfo->stCropWin.u16X;
        stRealCrop.u16Width = pstChnInfo->stCropWin.u16Width;

    }
    if ((pstChnInfo->stCropWin.u16Y + pstChnInfo->stCropWin.u16Height) > pstChnInfo->stSrcWin.u16Height)
    {
        bRet = TRUE;
        stRealCrop.u16Y     = 0;
        stRealCrop.u16Height = pstChnInfo->stSrcWin.u16Height;
    }
    else
    {
        bRet = FALSE;
        stRealCrop.u16Y     = pstChnInfo->stCropWin.u16Y;
        stRealCrop.u16Height = pstChnInfo->stCropWin.u16Height;
    }

    // Change Crop window Changed?
    if (((stRealCrop.u16X != pstChnInfo->stRealCrop.u16X)
        || (stRealCrop.u16Width != pstChnInfo->stRealCrop.u16Width)
        || (stRealCrop.u16Y != pstChnInfo->stRealCrop.u16Y)
        || (stRealCrop.u16Height != pstChnInfo->stRealCrop.u16Height)
        ))
    {
         pstChnInfo->stRealCrop.u16X = stRealCrop.u16X;
         pstChnInfo->stRealCrop.u16Width = stRealCrop.u16Width;
         pstChnInfo->stRealCrop.u16Y = stRealCrop.u16Y;
         pstChnInfo->stRealCrop.u16Height = stRealCrop.u16Height;

        bRet = TRUE;
    }

    return bRet;
}

static MI_BOOL _MI_SD_CalcCropInfo(mi_SD_ChannelInfo_t *pstChnInfo, MI_SYS_WindowRect_t *pstHwCrop, MI_SYS_WindowRect_t *pstSwCrop)
{
    MI_BOOL bRet = TRUE;
    int left_top_x, left_top_y, right_bottom_x, right_bottom_y;
    MI_U32 u32PixelAlign = 0, u32PixelPerBytes = 0;
    switch (pstChnInfo->eInPixFmt)
    {
    case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
        u32PixelAlign    = YUV422_PIXEL_ALIGN;
        u32PixelPerBytes = YUV422_BYTE_PER_PIXEL;
        break;
    case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        u32PixelAlign    = YUV420_PIXEL_ALIGN;
        u32PixelPerBytes = YUV420_BYTE_PER_PIXEL;
        break;
    default:
        bRet = FALSE;
    }

    if (bRet == TRUE)
    {
        left_top_y = ALIGN_DOWN(pstChnInfo->stRealCrop.u16Y, u32PixelAlign);
        left_top_x = ALIGN_DOWN(pstChnInfo->stRealCrop.u16X, (MIU_BURST_BITS/(u32PixelPerBytes*8)));
        right_bottom_y = ALIGN_UP(pstChnInfo->stRealCrop.u16Y + pstChnInfo->stRealCrop.u16Height, u32PixelAlign);
        right_bottom_x = ALIGN_UP(pstChnInfo->stRealCrop.u16X + pstChnInfo->stRealCrop.u16Width, (MIU_BURST_BITS/(u32PixelPerBytes*8)));

        pstSwCrop->u16X = left_top_x;
        pstSwCrop->u16Width = right_bottom_x-left_top_x;
        pstSwCrop->u16Y = left_top_y;
        pstSwCrop->u16Height = right_bottom_y-left_top_y;

        if (pstChnInfo->stSrcWin.u16Width < pstSwCrop->u16Width)
        {
            pstSwCrop->u16Width = pstChnInfo->stSrcWin.u16Width;
        }

        if (pstChnInfo->stSrcWin.u16Height < pstSwCrop->u16Height)
        {
            pstSwCrop->u16Height = pstChnInfo->stSrcWin.u16Height;
        }

        pstHwCrop->u16X = pstChnInfo->stRealCrop.u16X-left_top_x;
        pstHwCrop->u16Y = pstChnInfo->stRealCrop.u16Y-left_top_y;

        pstHwCrop->u16Width = pstChnInfo->stRealCrop.u16Width;
        pstHwCrop->u16Height = pstChnInfo->stRealCrop.u16Height;
        // HW crop position and size need 2 align
        pstHwCrop->u16Y = ALIGN_UP(pstHwCrop->u16Y, 2);
        pstHwCrop->u16Height = ALIGN_DOWN(pstHwCrop->u16Height, 2);

        pstHwCrop->u16X = ALIGN_UP(pstHwCrop->u16Y, 2);
        pstHwCrop->u16Width = ALIGN_DOWN(pstHwCrop->u16Width, 2);
    }
    else
    {
        DBG_WRN("UnSupport pixel format: %d.\n", pstChnInfo->eInPixFmt);
    }
    DBG_INFO("swCrop: {x: %u, y: %u, width: %u, height: %u}.\n", pstSwCrop->u16X, pstSwCrop->u16Y, pstSwCrop->u16Width, pstSwCrop->u16Height);
    DBG_INFO("hwCrop: {x: %u, y: %u, width: %u, height: %u}.\n", pstHwCrop->u16X, pstHwCrop->u16Y, pstHwCrop->u16Width, pstHwCrop->u16Height);

    return bRet;
}

static MI_BOOL _MI_SD_UpdateCropAddress(mi_sys_ChnTaskInfo_t *pstChnTask, MI_SYS_WindowRect_t *pstSwCrop)
{
    MI_BOOL bRet = FALSE;
    MI_PHY u64PhyAddr = 0;
    if (pstChnTask != NULL)
    {
        mi_SD_ChannelInfo_t *pstChnInfo = GET_SD_CHNN_PTR(pstChnTask->u32ChnId);
        MI_SYS_FrameData_t *pstBuffFrame = GET_SD_BUFF_FRAME_FROM_TASK(pstChnTask);
        switch (pstBuffFrame->ePixelFormat)
        {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
        {
            MI_U32 u32BytePerPixel = 2;
            u64PhyAddr =  (pstSwCrop->u16Y * pstBuffFrame->u32Stride[0]);
            u64PhyAddr += pstSwCrop->u16X * u32BytePerPixel;
            pstChnInfo->u64PhyAddrOffset[0] = u64PhyAddr;
            pstChnInfo->u64PhyAddrOffset[1] = 0;
            pstChnInfo->u64PhyAddrOffset[2] = 0;

        }
        break;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        {
            MI_U32 u32LumaBytePerPixel = 1;
            u64PhyAddr =  (pstSwCrop->u16Y * pstBuffFrame->u32Stride[0]);
            u64PhyAddr += pstSwCrop->u16X * u32LumaBytePerPixel;
            pstChnInfo->u64PhyAddrOffset[0] = u64PhyAddr;

            u64PhyAddr =   (pstSwCrop->u16Y/2 * pstBuffFrame->u32Stride[0]);
            u64PhyAddr +=  pstSwCrop->u16X; // TODO: tommy: Check UV whether need /2 ?
            pstChnInfo->u64PhyAddrOffset[1] = u64PhyAddr;
            pstChnInfo->u64PhyAddrOffset[2] = 0;
        }
        break;
        default:
            bRet = FALSE;
        }
    }

    return bRet;
}
#if defined(SD_TASK_PERF_DBG) && (SD_TASK_PERF_DBG == 1)
static MI_U64 _gGotDataTime = 0;
#endif


int SDWorkThread(void *data)
{
    MI_U16 fence = 0;
    mi_SD_DevInfo_t *pstDevInfo = (mi_SD_DevInfo_t *)data;
    #if USE_CMDQ
    MHAL_CMDQ_CmdqInterface_t *cmdinf = pstDevInfo->pstCmdMloadInfo;
    #endif
#if defined(SD_TASK_PERF_DBG) && (SD_TASK_PERF_DBG == 1)
    MI_U64 u64CurrentTime = 0;
#endif
    while (!kthread_should_stop())
    {
        mi_SD_IteratorWorkInfo_t workinfo;
        struct list_head *pos = NULL, *n = NULL;
        workinfo.totalAddedTask = 0;
        DBG_INFO("Start get data.\n");

        mi_sys_DevTaskIterator(pstDevInfo->hDevSysHandle, _MI_SD_TaskIteratorCallBK, &workinfo);
        if(list_empty(&SD_todo_task_list))
        {
            schedule_timeout_interruptible(1);
            // Tommy: Need description for this API behavior: eg: input buffer will return immediately.
            mi_sys_WaitOnInputTaskAvailable(pstDevInfo->hDevSysHandle, 50);
            continue;
        }
        DBG_INFO(" got data.\n");

        list_for_each_safe(pos, n, &SD_todo_task_list)
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            #if USE_CMDQ
            int loop_cnt = 0;
            #endif
            int repeatNum = 1;
            int i = 0;
            MHalVpeSclInputSizeConfig_t stCfg;
            MI_SYS_FrameData_t *pstBuffFrame = NULL;
            mi_SD_ChannelInfo_t *pstChInfo = NULL;
            mi_SD_InBuffInfo_t stInBuffInfo;

            memset(&stInBuffInfo, 0, sizeof(stInBuffInfo));

            pstChnTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
            pstChInfo = GET_SD_CHNN_PTR(pstChnTask->u32ChnId);

            pstBuffFrame = GET_SD_BUFF_FRAME_FROM_TASK(pstChnTask);

            LOCK_CHNN(pstChInfo);

            //2.Check input size change
            //bHaveSideBandMsg = _MI_SD_ProcessTaskSideBandMsg(pstChnTask);
            if ((_MI_SD_CheckInputChanged(pstChnTask)== TRUE))
            {
                // Set SCL input
                // Tommy: Think about repeat: input size change /sw crop
                // Need check with HAL wheather add Hal API for get NR reference Buffer.
                repeatNum = REPEAT_MAX_NUMBER;
                memset(&stCfg, 0, sizeof(stCfg));

                {
                    stCfg.u16Height = pstChInfo->stSrcWin.u16Height;
                    stCfg.u16Width  = pstChInfo->stSrcWin.u16Width;
                }
                stCfg.eCompressMode = pstChInfo->eCompressMode;
                stCfg.ePixelFormat  = pstChInfo->eInPixFmt;
                DBG_INFO("%s()@line %d: MHalVPESclInputConfig: %d x %d.\n", __func__, __LINE__, stCfg.u16Width, stCfg.u16Height);
                MHalVpeSclInputConfig(pstChInfo->pSclCtx, &stCfg);
            }

            // 2. check sideband crop msg
            //_MI_VPE_ProcessTaskSideBandMsg(pstChnTask);

            // 3. sw crop
            // Check Crop
            if((_MI_SD_CheckCropChanged(pstChnTask) == TRUE))
            {
                MI_SYS_WindowRect_t stHwCrop;
                MI_SYS_WindowRect_t stSwCrop;
                MHalVpeSclCropConfig_t stHwCropCfg;

                memset(&stHwCrop, 0, sizeof(stHwCrop));
                memset(&stSwCrop, 0, sizeof(stSwCrop));
                _MI_SD_CalcCropInfo(pstChInfo, &stHwCrop, &stSwCrop);
                _MI_SD_UpdateCropAddress(pstChnTask, &stSwCrop);

                // Set SCL input <-- SW crop
                repeatNum = REPEAT_MAX_NUMBER;
                memset(&stCfg, 0, sizeof(stCfg));
                {
                    stCfg.u16Height = stSwCrop.u16Height;
                    stCfg.u16Width  = stSwCrop.u16Width;
                }
                stCfg.eCompressMode = pstChInfo->eCompressMode;
                stCfg.ePixelFormat  = pstChInfo->eInPixFmt;
                DBG_INFO("%s()@line %d: MHalVpeSclInputConfig: %d x %d.\n", __func__, __LINE__, stCfg.u16Width, stCfg.u16Height);

                MHalVpeSclInputConfig(pstChInfo->pSclCtx, &stCfg);

                // Set Crop info <- HW crop
                memset(&stHwCropCfg, 0, sizeof(stHwCropCfg));
                stHwCropCfg.bCropEn = TRUE;

                stHwCropCfg.stCropWin.u16X = stHwCrop.u16X;
                stHwCropCfg.stCropWin.u16Y = stHwCrop.u16Y;
                stHwCropCfg.stCropWin.u16Width = stHwCrop.u16Width;
                stHwCropCfg.stCropWin.u16Height = stHwCrop.u16Height;
                MHalVpeSclCropConfig(pstChInfo->pSclCtx, &stHwCropCfg);
            }

            for (i = 0; i < repeatNum; i++)
            {
                // TODO: user cmdq service api
                // cmdq service need add check menuload buffer size valid.
                #if USE_CMDQ
                while(!cmdinf->MHAL_CMDQ_CheckBufAvailable(cmdinf, 0x1000))
                {
                    msleep(SD_WORK_THREAD_WAIT*1000/HZ);

                    loop_cnt++;
                    if(loop_cnt>1000)
                        MI_SYS_BUG();//engine hang
                }
                #endif

                if(pstChnTask->astInputPortBufInfo[0]->eBufType == E_MI_SYS_BUFDATA_FRAME)
                {
                    stInBuffInfo.phyAddr[0] = pstBuffFrame->phyAddr[0];
                    stInBuffInfo.phyAddr[1] = pstBuffFrame->phyAddr[1];
                    stInBuffInfo.phyAddr[2] = pstBuffFrame->phyAddr[2];
                    stInBuffInfo.u32Stride[0] = pstBuffFrame->u32Stride[0];
                    stInBuffInfo.u32Stride[1] = pstBuffFrame->u32Stride[1];
                    stInBuffInfo.u32Stride[2] = pstBuffFrame->u32Stride[2];
                }

#if 0
                if(pstDevInfo->eRunningMode != E_MI_SD_RUNNING_MODE_FRAMEBUF_DVR_MODE)
                {
                    mi_sys_LowLatencyDataInfo_t *pstLowLatencyData = (mi_sys_LowLatencyDataInfo_t *)pstBuffFrame->pPrivdata;
                    stInBuffInfo.u64MetaInfo = pstLowLatencyData->u64MetaInfo;
                }

#endif
                _MI_SD_ProcessTask(pstChnTask, pstDevInfo, &stInBuffInfo);
                #if USE_CMDQ
                cmdinf->MHAL_CMDQ_WriteDummyRegCmdq(cmdinf, fence);
                cmdinf->MHAL_CMDQ_KickOffCmdq(cmdinf);
                #endif

                pstChnTask->u32Reserved0 = fence++;
                //cmdinf->MHAL_CMDQ_GetNextMloadRingBufWritePtr(cmdinf, (MS_PHYADDR *)&pstChnTask->u64Reserved0);
            }
            UNLOCK_CHNN(pstChInfo);
            DBG_INFO("Add to working litst.\n");

            list_del(&pstChnTask->listChnTask);
            down(&SD_working_list_sem);
            list_add_tail(&pstChnTask->listChnTask, &SD_working_task_list);
            up(&SD_working_list_sem);

            DBG_INFO("Add to working litst Done.\n");
        }
        DBG_INFO("Todo task list empty.\n");
    }
    return 0;
}


static MI_S32 _MI_SD_OnBindChnnInputputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_SD_DevInfo_t *pstDevInfo = (mi_SD_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_SD_DEV_MAGIC__);
    if ((pstChnCurryPort->eModId == E_MI_MODULE_ID_SD)    // Check Callback information ok ?
        && MI_SD_CHECK_CHNN_SUPPORTED(pstChnCurryPort->u32ChnId) // Check support Channel
#if 0
        && ((pstChnPeerPort->eModId == E_MI_MODULE_ID_VIF) // Check supported input module ?
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_DIVP)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_SD))
#endif
        )
    {
        pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stPeerInputPortInfo = *pstChnPeerPort;
        return MI_SUCCESS;
    }
    else
    {
        return MI_SYS_ERR_BUSY;
    }
}

static MI_S32 _MI_SD_OnUnBindChnnInputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_SD_DevInfo_t *pstDevInfo = (mi_SD_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_SD_DEV_MAGIC__);
    if ((pstChnCurryPort->eModId == E_MI_MODULE_ID_SD)         // Check Callback information ok ?
        && MI_SD_CHECK_CHNN_SUPPORTED(pstChnCurryPort->u32ChnId)   // Check support Channel
#if 0
    && ((pstChnPeerPort->eModId == E_MI_MODULE_ID_VIF)      // Check supported output module
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_DIVP)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_SD))
#endif
        )
    {
        // Need check Chnn Busy ???
        // if(list_empty(&SD_todo_task_list))
        // ... ...
        memset(&pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stPeerInputPortInfo, 0, sizeof(pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stPeerInputPortInfo));
        return MI_SUCCESS;
    }
    else
    {
        return MI_SYS_ERR_BUSY;
    }
}

static MI_S32 _MI_SD_OnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_SD_DevInfo_t *pstDevInfo = (mi_SD_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_SD_DEV_MAGIC__);
    if ((pstChnCurryPort->eModId == E_MI_MODULE_ID_SD)         // Check Callback information ok ?
        && MI_SD_CHECK_CHNN_SUPPORTED(pstChnCurryPort->u32ChnId)   // Check support Channel
        && MI_SD_CHECK_PORT_SUPPORTED(pstChnCurryPort->u32PortId)  // Check support Port Id
#if 0
    && ((pstChnPeerPort->eModId == E_MI_MODULE_ID_DIVP)     // supported input module
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VENC)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VDISP)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_DISP))
#endif
        )
    {
        pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stOutPortInfo[pstChnCurryPort->u32PortId].stPeerOutputPortInfo = *pstChnPeerPort;
        return MI_SUCCESS;
    }
    else
    {
        return MI_SYS_ERR_BUSY;
    }
}

static MI_S32 _MI_SD_OnUnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_SD_DevInfo_t *pstDevInfo = (mi_SD_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_SD_DEV_MAGIC__);
    if ((pstChnCurryPort->eModId == E_MI_MODULE_ID_SD)         // Check Callback information ok ?
        && MI_SD_CHECK_CHNN_SUPPORTED(pstChnCurryPort->u32ChnId)   // Check support Channel
        && MI_SD_CHECK_PORT_SUPPORTED(pstChnCurryPort->u32PortId)  // Check support Port Id
#if 0 // No use
    && ((pstChnPeerPort->eModId == E_MI_MODULE_ID_DIVP)     // supported input module
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VENC)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VDISP)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_DISP))
#endif
        )
    {
        memset (&pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stOutPortInfo[pstChnCurryPort->u32PortId].stPeerOutputPortInfo, 0,
            sizeof(pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stOutPortInfo[pstChnCurryPort->u32PortId].stPeerOutputPortInfo));
        return MI_SUCCESS;
    }
    else
    {
        return MI_SYS_ERR_BUSY;
    }

    // Check Busy ??
    // if(list_empty(&SD_working_task_list))
 }

#if 0
void dump_tread( int *pdata)
{

     while(!kthread_should_stop())
     {
        struct task_struct *g, *p;
        msleep(60000);//30 secs
          rcu_read_lock();
         do_each_thread(g, p)
         {
            show_stack(p,NULL);
         } while_each_thread(g, p);
         rcu_read_unlock();
    }

}
#endif

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
static MI_S32 _MI_SD_ProcOnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId,void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_SD_NOT_SUPPORT;
    mi_SD_DevInfo_t     *pstDev = GET_SD_DEV_PTR();

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    if (u32DevId > 0)
    {
        return s32Ret;
    }
    handle.OnPrintOut(handle, "\r\n-------------------------- start dump DEV info --------------------------------\n");
    handle.OnPrintOut(handle,"%7s%12s%9s%12s\n","DevID", "SupportIRQ", "IRQ_num", "IRQ_Enable");
    handle.OnPrintOut(handle,"%7d",u32DevId);
    handle.OnPrintOut(handle, "%12d", pstDev->bSupportIrq);
    handle.OnPrintOut(handle, "%9d",  pstDev->uSDIrqNum);
    handle.OnPrintOut(handle, "%12d\n", pstDev->bEnbaleIrq);
    handle.OnPrintOut(handle, "-------------------------- End dump dev %d info --------------------------------\n", u32DevId);

    s32Ret = MI_SD_OK;
    return s32Ret;
}

static MI_S32 _MI_SD_ProcOnDumpChannelAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId,void *pUsrData)
{
    MI_U32 u32ChnId = 0;//TODO: while all channel

    MI_S32 s32Ret = MI_ERR_SD_NOT_SUPPORT;
    mi_SD_ChannelInfo_t *pstChnInfo = NULL;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    handle.OnPrintOut(handle, "\r\n-------------------------- start dump CHN info --------------------------------\n");
    handle.OnPrintOut(handle,"%7s%8s%8s%8s%7s%7s%7s%7s%9s%15s%14s\n", "ChnId", "status", "InputW","InputH", "CropX","CropY","CropW","CropH");
    for(u32ChnId = 0; u32ChnId < MI_SD_MAX_CHANNEL_NUM; u32ChnId++)
    {
        pstChnInfo = GET_SD_CHNN_PTR(u32ChnId);
        if (pstChnInfo->bCreated == TRUE)
        {
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%8d", pstChnInfo->eStatus);
            handle.OnPrintOut(handle,"%8d", pstChnInfo->stSrcWin.u16Width);
            handle.OnPrintOut(handle,"%8d", pstChnInfo->stSrcWin.u16Height);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stCropWin.u16X);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stCropWin.u16Y);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stCropWin.u16Width);
            handle.OnPrintOut(handle,"%7d\n", pstChnInfo->stCropWin.u16Height);
        }
    }

    handle.OnPrintOut(handle,"%7s%10s%15s%10s%10s\n","ChnId","InBufCnt","InBufTodoCnt","GetToK.O", "K.OToRel");
    for(u32ChnId = 0; u32ChnId < MI_SD_MAX_CHANNEL_NUM; u32ChnId++)
    {
        pstChnInfo = GET_SD_CHNN_PTR(u32ChnId);
        if (pstChnInfo->bCreated == TRUE)
        {
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%10llu", pstChnInfo->u64GetInputBufferCnt);
            handle.OnPrintOut(handle,"%15llu", pstChnInfo->u64GetInputBufferTodoCnt);
            handle.OnPrintOut(handle,"\n");
            //pstChnInfo->u64GetInputBufferCnt = 0;
        }
    }
    handle.OnPrintOut(handle, "-------------------------- End dump CHN info -------------------------------\n");

    s32Ret = MI_SD_OK;
    return s32Ret;
}


static MI_S32 _MI_SD_ProcOnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_S32 s32Ret = MI_SD_OK;

    // Add input port implement here
    return s32Ret;
}

static MI_S32 _MI_SD_ProcOnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_SD_NOT_SUPPORT;
    MI_U32 u32ChnId = 0, u32OutPortId = 0;//TODO: while all channel
    mi_SD_OutPortInfo_t *pstPortInfo = NULL;
    mi_SD_ChannelInfo_t *pstChnInfo = NULL;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    handle.OnPrintOut(handle, "\r\n-------------------------- start dump OUTPUT PORT info -----------------------\n");
    handle.OnPrintOut(handle, "%7s%8s%7s%9s%9s%10s%15s%13s%18s%5s\n","ChnId", "PortId", "Pixel", "OutputW", "OutputH", "Compress","GetOputBufCnt","FailOBufCnt","FinishOputBufCnt","fps");
    for(u32ChnId = 0; u32ChnId < MI_SD_MAX_CHANNEL_NUM; u32ChnId++)
    {
        for(u32OutPortId = 0; u32OutPortId < MI_SD_MAX_PORT_NUM; u32OutPortId++)
        {
            pstChnInfo = GET_SD_CHNN_PTR(u32ChnId);
            if (pstChnInfo->bCreated == TRUE)
            {
                pstPortInfo = GET_SD_PORT_PTR(u32ChnId, u32OutPortId);
                handle.OnPrintOut(handle, "%7d", u32ChnId);
                handle.OnPrintOut(handle, "%8d", u32OutPortId);
                handle.OnPrintOut(handle, "%7d", pstPortInfo->stPortMode.ePixelFormat);
                handle.OnPrintOut(handle, "%9d", pstPortInfo->stPortMode.u16Width);
                handle.OnPrintOut(handle, "%9d", pstPortInfo->stPortMode.u16Height);
                handle.OnPrintOut(handle, "%10d", pstPortInfo->stPortMode.eCompressMode);
                handle.OnPrintOut(handle, "%15llu", pstPortInfo->u64GetOutputBufferCnt);
                handle.OnPrintOut(handle, "%13llu", pstPortInfo->u64GetOutputBuffFailCnt);
                handle.OnPrintOut(handle, "%18llu", pstPortInfo->u64FinishOutputBufferCnt);
                handle.OnPrintOut(handle, "%5d\n", pstPortInfo->u8PortFps);
                //pstPortInfo->u64GetOutputBufferCnt = 0;
                //pstPortInfo->u64FinishOutputBufferCnt = 0;
            }
        }
    }
    handle.OnPrintOut(handle, "-------------------------- End dump OUTPUT PORT info -------------------------\n");

    s32Ret = MI_SD_OK;

    return s32Ret;
}

MI_S32 _MI_SD_ProcDumpFile(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_SD_NOT_SUPPORT;
    MI_SD_CHANNEL SDChn;
    MI_U16 u16DumpTaskFileNum = 0;
    mi_SD_ChannelInfo_t *pstChnInfo = NULL;

    mi_SD_DevInfo_t *pstDevInfo = (mi_SD_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_SD_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 2)
    {
        SDChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(MI_SD_CHECK_CHNN_SUPPORTED(SDChn) && MI_SD_CHECK_CHNN_CREATED(SDChn))
        {
            pstChnInfo = GET_SD_CHNN_PTR(SDChn);
        }
        else
        {
            handle.OnPrintOut(handle, "channelID[%d] is not Supported or not Create.\n", SDChn);
            return MI_ERR_SD_NOT_SUPPORT;
        }

        u16DumpTaskFileNum = (MI_U16)simple_strtoul(argv[2], NULL, 10);
        if ( u16DumpTaskFileNum > 0 && u16DumpTaskFileNum < 0xffff)
        {
            pstChnInfo->u16DumpTaskFileNum = u16DumpTaskFileNum;
        }
        else
        {
            handle.OnPrintOut(handle, "argv[2] is %d error[0~65535].\n", u16DumpTaskFileNum);
            return MI_ERR_SD_NOT_SUPPORT;
        }

        handle.OnPrintOut(handle,"ChanID %d u16DumpTaskFileNum %d \r\n", SDChn, u16DumpTaskFileNum);
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "VPE DumpTaskFile chnid  num;.\n");
        s32Ret = MI_ERR_SD_NOT_SUPPORT;
    }

    return s32Ret;
}

static MI_S32 _MI_SD_ProcOnHelp(MI_SYS_DEBUG_HANDLE_t  handle,MI_U32  u32DevId,void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_SD_NOT_SUPPORT;
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    handle.OnPrintOut(handle, "disable_cmdq [ON, OFF];              Enable/Disable CMDQ.\n");
    handle.OnPrintOut(handle, "disable_irq  [ON, OFF];              Enable/Disable IRQ.\n");

    s32Ret = MI_SD_OK;
    return s32Ret;
}

#endif

MI_S32 MI_SD_IMPL_Init(void)
{
    MI_S32 s32Ret = MI_ERR_SD_NULL_PTR;
    mi_sys_ModuleDevBindOps_t stSDPOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    #if USE_CMDQ
    MHAL_CMDQ_BufDescript_t stCmdqBufDesp;
    #endif
    MHalAllocPhyMem_t stAlloc;

#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
#endif

    mi_SD_DevInfo_t *pstDevInfo = GET_SD_DEV_PTR();
    struct sched_param param;
    memset(&param, 0, sizeof(param));

    if (pstDevInfo->bInited == TRUE)
    {
        DBG_ERR("already inited.\n");
        return MI_ERR_SD_EXIST;
    }
    DBG_INFO("Start Init.\n");

    //Get SD Cap
    MHalVpeGetCaps(&pstDevInfo->stCaps);

    // SD register to mi_sys
    memset(&stSDPOps, 0, sizeof(stSDPOps));
    stSDPOps.OnBindInputPort    = _MI_SD_OnBindChnnInputputCallback;
    stSDPOps.OnUnBindInputPort  = _MI_SD_OnUnBindChnnInputCallback;
    stSDPOps.OnBindOutputPort   = _MI_SD_OnBindChnnOutputCallback;
    stSDPOps.OnUnBindOutputPort = _MI_SD_OnUnBindChnnOutputCallback;
    stSDPOps.OnOutputPortBufRelease = NULL;
    memset(&stModInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId      = E_MI_MODULE_ID_SD;
    stModInfo.u32DevId         = 0;
    stModInfo.u32DevChnNum     = MI_SD_MAX_CHANNEL_NUM;
    stModInfo.u32InputPortNum  = MI_SD_MAX_INPUTPORT_NUM;
    stModInfo.u32OutputPortNum = MI_SD_MAX_PORT_NUM;

#ifdef MI_SYS_PROC_FS_DEBUG

    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
#if(MI_VPE_PROCFS_DEBUG == 1)
    pstModuleProcfsOps.OnDumpDevAttr = _MI_SD_ProcOnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = _MI_SD_ProcOnDumpChannelAttr;
    pstModuleProcfsOps.OnDumpInputPortAttr = _MI_SD_ProcOnDumpInputPortAttr;
    pstModuleProcfsOps.OnDumpOutPortAttr = _MI_SD_ProcOnDumpOutPortAttr;
    pstModuleProcfsOps.OnHelp = _MI_SD_ProcOnHelp;
#else
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif

#endif

    pstDevInfo->hDevSysHandle   = mi_sys_RegisterDev(&stModInfo, &stSDPOps, pstDevInfo
                                                   #ifdef MI_SYS_PROC_FS_DEBUG
                                                   , &pstModuleProcfsOps
                                                   ,MI_COMMON_GetSelfDir
                                                   #endif
                                                   );

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
     mi_sys_RegistCommand("writefile", 2, _MI_SD_ProcDumpFile, pstDevInfo->hDevSysHandle);
#endif

    if (pstDevInfo->hDevSysHandle == NULL)
    {
        DBG_ERR("Fail to register dev.\n");
        goto __register_sys_fail;
    }
    DBG_INFO("success to register dev.\n");

    // Get cmdQ service
    #if USE_CMDQ
    memset(&stCmdqBufDesp, 0, sizeof(stCmdqBufDesp));
    stCmdqBufDesp.u32CmdqBufSize       = SD_CMDQ_BUFF_SIZE_MAX;
    stCmdqBufDesp.u32CmdqBufSizeAlign  = SD_CMDQ_BUFF_ALIGN;
    stCmdqBufDesp.u32MloadBufSize      = 0;
    stCmdqBufDesp.u16MloadBufSizeAlign = 1;
    //if debug
    //pstDevInfo->pstCmdMloadInfo = get_sys_cmdq_service(CMDQ_ID_SD, cmdq_buffer_descript_t * pCmdqBufDesp, TRUE);
    //else
    pstDevInfo->pstCmdMloadInfo = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_DIVP, &stCmdqBufDesp, TRUE);

    if (pstDevInfo->pstCmdMloadInfo == NULL)
    {
        DBG_ERR("Fail to get cmd service.\n");
        goto __get_cmd_service_fail;
    }
    DBG_INFO("success to get cmd service.\n");
    #else
    pstDevInfo->pstCmdMloadInfo = NULL;
    #endif

    // SD MHAL init
    memset(&stAlloc, 0, sizeof(stAlloc));
    stAlloc.alloc = _MI_SD_IMPL_MiSysAlloc;
    stAlloc.free  = _MI_SD_IMPL_MiSysFree;
    stAlloc.map   = _MI_SD_IMPL_MiSysMap;
    stAlloc.unmap = _MI_SD_IMPL_MiSysUnMap;
    stAlloc.flush_cache = _MI_SD_IMPL_MiSysFlashCache;
    if (MHalVpeInit(&stAlloc, pstDevInfo->pstCmdMloadInfo) == FALSE)
    {
        goto __mhal_SD_init_fail;
    }

    // Get SD IRQ
    pstDevInfo->bEnbaleIrq = FALSE;
    if (MHalVpeSclGetIrqNum(&pstDevInfo->uSDIrqNum, E_MHAL_SCL_IRQ_PORT2_M2M) == FALSE)
    {
        DBG_WRN("Fail to get irq.\n");
        //return MI_ERR_SD_NOT_SUPPORT;
        pstDevInfo->bSupportIrq = FALSE;
    }
    else
    {
        pstDevInfo->bSupportIrq = TRUE;
        DBG_INFO("get irq: %d.\n", pstDevInfo->uSDIrqNum);
    }

    INIT_LIST_HEAD(&SD_todo_task_list);
    INIT_LIST_HEAD(&SD_working_task_list);

    atomic_set(&pstDevInfo->wqRedFlag, 0);

    // Create work thread
    pstDevInfo->pstWorkThread = kthread_create(SDWorkThread, pstDevInfo, "SD/WorkThread");
    if (IS_ERR(pstDevInfo->pstWorkThread))
    {
        DBG_ERR("Fail to create thread SD/WorkThread.\n");
        goto __create_work_thread_fail;
    }
    DBG_INFO("success to  create thread SD/WorkThread.\n");
    param.sched_priority = 99;
    sched_setscheduler(pstDevInfo->pstWorkThread, SCHED_RR, &param);

    // Create IRQ Bottom handler
    pstDevInfo->pstProcThread = kthread_create(_MI_SD_IsrProcThread, pstDevInfo, "SD/IsrProcThread");
    if (IS_ERR(pstDevInfo->pstProcThread))
    {
        DBG_ERR("Fail to create thread SD/IsrProcThread.\n");
        goto __create_proc_thread_fail;
    }
    DBG_INFO("success to create thread SD/IsrProcThread.\n");
#if defined(SD_ADJUST_THREAD_PRIORITY) && (SD_ADJUST_THREAD_PRIORITY == 1)
    param.sched_priority = 99;
    sched_setscheduler(pstDevInfo->pstProcThread, SCHED_RR, &param);
#endif
    //_gpstDbgThread = kthread_create(dump_tread, pstDevInfo, "SD/DbgThread");


    wake_up_process(pstDevInfo->pstWorkThread);
    wake_up_process(pstDevInfo->pstProcThread);
    //wake_up_process(_gpstDbgThread);

    if (TRUE == pstDevInfo->bSupportIrq)
    {
        // Register kernel IRQ
        if (0 > request_irq(pstDevInfo->uSDIrqNum, _MI_SD_Isr, IRQF_SHARED | IRQF_ONESHOT, "SD-IRQ", pstDevInfo))
        {
            DBG_ERR("Fail to request_irq: %d.\n", pstDevInfo->uSDIrqNum);
        }
        DBG_INFO("success to request_irq: %d.\n", pstDevInfo->uSDIrqNum);
        //Enable Irq In Creat first Channel
    }

    pstDevInfo->bInited = TRUE;
    s32Ret = MI_SD_OK;

    return s32Ret;

__create_proc_thread_fail:
    kthread_stop(pstDevInfo->pstProcThread);
__create_work_thread_fail:
    kthread_stop(pstDevInfo->pstWorkThread);
__mhal_SD_init_fail:
    MHalVpeDeInit();
 #if USE_CMDQ
__get_cmd_service_fail:
    MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_DIVP);
    pstDevInfo->pstCmdMloadInfo = NULL;
#endif
__register_sys_fail:
    mi_sys_UnRegisterDev(pstDevInfo->hDevSysHandle);
    pstDevInfo->hDevSysHandle = NULL;

    return s32Ret;
}

void MI_SD_IMPL_DeInit(void)
{
    MI_U8 u8SDChnlId =0;
    mi_SD_DevInfo_t *pstDevInfo = GET_SD_DEV_PTR();
    pstDevInfo->bInited = FALSE;

    while(pstDevInfo->u32ChannelCreatedNum>0 && u8SDChnlId < MI_SD_MAX_CHANNEL_NUM)
    {
        mi_SD_ChannelInfo_t *pstChnnInfo = GET_SD_CHNN_PTR(u8SDChnlId);

        if(pstChnnInfo->bCreated)
        {
            if(pstChnnInfo->eStatus == E_MI_SD_CHANNEL_STATUS_START)
                MI_SD_IMPL_StopChannel(u8SDChnlId);

            MI_SD_IMPL_DestroyChannel(u8SDChnlId);
        }

        u8SDChnlId ++;
    }

    if (pstDevInfo->bSupportIrq == TRUE)
    {
        free_irq(pstDevInfo->uSDIrqNum, pstDevInfo);
        pstDevInfo->bEnbaleIrq = FALSE;
    }
    kthread_stop(pstDevInfo->pstProcThread);
    kthread_stop(pstDevInfo->pstWorkThread);
    //kthread_stop(_gpstDbgThread);
    #if USE_CMDQ
    MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_DIVP);
    #endif
    pstDevInfo->pstCmdMloadInfo = NULL;
    MHalVpeDeInit();
    mi_sys_UnRegisterDev(pstDevInfo->hDevSysHandle);
    pstDevInfo->hDevSysHandle = NULL;
}
