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
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/math64.h>
#include <linux/idr.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/completion.h>
#include <linux/math64.h>

#include "mi_print.h"
#include "mi_sys.h"
#include "mi_ldc_impl.h"
#include "mi_ldc.h"
//#include "mhal_ldc.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_gfx.h"


#define COPY_DATA_BY_GFX        1
#define MI_LDC_MAX_QUEUE_NUM   3

#define MI_LDC_CHECK_POINTER(pPtr)  \
    if(NULL == pPtr)  \
    {   \
        DBG_ERR("Invalid parameter! NULL pointer.\n");   \
        return MI_ERR_LDC_NULL_PTR;   \
    }

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
#define SEMA_INIT(pSem) init_MUTEX(pSem)
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
#define SEMA_INIT(pSem) sema_init(pSem, 1)
#endif

#define MI_INITIAL_BUF_QUEUE(queue) do{ \
            INIT_LIST_HEAD(&queue.bufList);  \
            queue.u32BufCnt = 0;   \
            SEMA_INIT(&queue.semQueue);    \
} while(0);


#define MI_DEINITIAL_BUF_QUEUE(queue) do{ \
            INIT_LIST_HEAD(queue.bufList);  \
            queue.u32BufCnt = 0;   \
} while(0);

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
static MI_U64 _MI_LDC_StatSpendTime(struct timespec *pstTimeStart)
{
    struct timespec stCurTime;
    memset(&stCurTime, 0, sizeof(stCurTime));
    do_posix_clock_monotonic_gettime(&stCurTime);

    return (stCurTime.tv_sec - pstTimeStart->tv_sec)*1000*1000 + (stCurTime.tv_nsec - pstTimeStart->tv_nsec)/1000;
}

static MI_U32 _MI_LDC_SetStatFpsDepth(MI_LDC_CalcFps_t *pstCalcPts, MI_U32 u32Depth)
{
    BUG_ON(u32Depth == 0 || u32Depth > 100);

    if (pstCalcPts->pu64StatSet)
    {
        kfree(pstCalcPts->pu64StatSet);
    }

    pstCalcPts->u32StatDepth = u32Depth;
    pstCalcPts->pu64StatSet = (MI_U64*)kmalloc(sizeof(MI_U64)*u32Depth, GFP_ATOMIC);
    memset(pstCalcPts->pu64StatSet, 0, sizeof(MI_U64)*u32Depth);

    return MI_SUCCESS;
}

void _MI_LDC_InitialDebugInfo(MI_LDC_DebugInfo_t *pstDebugInfo)
{
    memset(&pstDebugInfo->stCalcHalTime, 0, sizeof(MI_LDC_CalcSpendTime_t));
    pstDebugInfo->stCalcHalTime.u64MinSpendTime = 0xffffffff;

    memset(&pstDebugInfo->stBufCnt, 0, sizeof(MI_LDC_BufOptCnt_t));

    memset(&pstDebugInfo->stCalcInputFps, 0, sizeof(MI_LDC_CalcFps_t));
    _MI_LDC_SetStatFpsDepth(&pstDebugInfo->stCalcInputFps, 20);

    memset(&pstDebugInfo->stCalcOutputFps, 0, sizeof(MI_LDC_CalcFps_t));
    _MI_LDC_SetStatFpsDepth(&pstDebugInfo->stCalcOutputFps, 20);

    memset(&pstDebugInfo->stInputPortInfo, 0, sizeof(MI_LDC_StatPortInfo_t));
    memset(&pstDebugInfo->stOutputPortInfo, 0, sizeof(MI_LDC_StatPortInfo_t));
}

void _MI_LDC_DeinitDebugInfo(MI_LDC_DebugInfo_t *pstDebugInfo)
{
    if (pstDebugInfo->stCalcInputFps.pu64StatSet)
    {
        kfree(pstDebugInfo->stCalcInputFps.pu64StatSet);
    }

    if (pstDebugInfo->stCalcOutputFps.pu64StatSet)
    {
        kfree(pstDebugInfo->stCalcOutputFps.pu64StatSet);
    }

    memset(pstDebugInfo, 0, sizeof(MI_LDC_DebugInfo_t));
}

void _MI_LDC_CalcTimeSpend(MI_LDC_CalcSpendTime_t *pstCalcSpendTime)
{
    pstCalcSpendTime->u64CurSpendTime = _MI_LDC_StatSpendTime(&pstCalcSpendTime->stStartTmSpec);
    if (pstCalcSpendTime->u64MaxSpendTime < pstCalcSpendTime->u64CurSpendTime)
        pstCalcSpendTime->u64MaxSpendTime = pstCalcSpendTime->u64CurSpendTime;

    if (pstCalcSpendTime->u64MinSpendTime > pstCalcSpendTime->u64CurSpendTime)
        pstCalcSpendTime->u64MinSpendTime = pstCalcSpendTime->u64CurSpendTime;

    pstCalcSpendTime->u64TotalTime += pstCalcSpendTime->u64CurSpendTime;

//    DBG_ERR("CurTime=%llu\n", pstCalcSpendTime->u64CurSpendTime);
}

void _MI_LDC_StatPts(MI_LDC_CalcFps_t *pstCalcFps, MI_U32 u32FrameCnt, MI_U64 u64CurPts)
{
    BUG_ON(!u32FrameCnt || !pstCalcFps);

    if (1 == u32FrameCnt)
    {
        pstCalcFps->u32CurIndex = 0;
        pstCalcFps->u64PrePts = u64CurPts;
        pstCalcFps->u64LastPts = u64CurPts;
        pstCalcFps->u32SampleCnt = 0;
    }
    else
    {
        pstCalcFps->u64PrePts = pstCalcFps->u64LastPts;
        pstCalcFps->u64LastPts = u64CurPts;

        if (pstCalcFps->u32StatDepth > u32FrameCnt - 1)
        {
            pstCalcFps->u32SampleCnt = u32FrameCnt - 1;
            pstCalcFps->u32CurIndex = pstCalcFps->u32SampleCnt - 1;

        }
        else
        {
            pstCalcFps->u32SampleCnt = pstCalcFps->u32StatDepth;
            pstCalcFps->u32CurIndex = (u32FrameCnt - 1) % (pstCalcFps->u32StatDepth+1) - 1;
        }

        *(pstCalcFps->pu64StatSet+pstCalcFps->u32CurIndex) = pstCalcFps->u64LastPts - pstCalcFps->u64PrePts;
    }
}

MI_U32 _MI_LDC_ClacFps(MI_LDC_CalcFps_t *pstCalcFps)
{
    MI_U32 i = 0;
    MI_U64 u64AvgPtsInterVal = 0;
    MI_U64 u64TotalPtsInterVal = 0;
    MI_U32 u32Fps = 0;  // x10


    BUG_ON(!pstCalcFps);

    for (i = 0; i < pstCalcFps->u32SampleCnt; i++)
    {
        u64TotalPtsInterVal += *(pstCalcFps->pu64StatSet+i);
    }

    if (pstCalcFps->u32SampleCnt)
        u64AvgPtsInterVal = div64_u64(u64TotalPtsInterVal, pstCalcFps->u32SampleCnt);

    if (u64AvgPtsInterVal)
        u32Fps = (MI_U32)div64_u64(1000*1000*10, u64AvgPtsInterVal);
    else
        u32Fps = 0;

    return u32Fps;
}
#endif


static MI_LDC_Module_t _stWarpModule;
extern MI_S32 mi_gfx_PendingDone(MI_U16 u16TargetFence);


irqreturn_t  _MI_LDC_ISR_Proc(s32 irq, void* data)
{
    MHAL_WARP_DEV_HANDLE hHandle = (MHAL_WARP_DEV_HANDLE)data;
    MHAL_WARP_ISR_STATE_E eState;

    eState = MHAL_WARP_IsrProc(hHandle);
    switch(eState)
    {
        case MHAL_WARP_ISR_STATE_DONE:
            return IRQ_HANDLED;

        default:
            return IRQ_NONE;
    }

    return IRQ_NONE;
}

MI_U32 _MI_LDC_Trigger_Callback(MHAL_WARP_INST_HANDLE instance, void *usrData)
{
    MI_LDC_Channel_t *pstChnCtx = (MI_LDC_Channel_t*)usrData;
    MI_LDC_Device_t *pstDev = NULL;
    MI_U64 u64Pts = 0;

    DBG_INFO("callback is called\n");
    pstDev = &(_stWarpModule.stDev[pstChnCtx->u32DevId]);

    if (pstChnCtx->stInputPort.pstInBufInfo)
    {
        u64Pts = pstChnCtx->stInputPort.pstInBufInfo->u64Pts;
        mi_sys_FinishBuf(pstChnCtx->stInputPort.pstInBufInfo);
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
        pstChnCtx->stDebugInfo.stBufCnt.u32FinishInputCnt++;
#endif
    }
    if (pstChnCtx->stOutputPort.pstOutBufInfo)
    {
        pstChnCtx->stOutputPort.pstOutBufInfo->u64Pts = u64Pts;
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
        pstChnCtx->stDebugInfo.stBufCnt.u32FinishOutputCnt++;

        // calc output fps
        _MI_LDC_StatPts(&pstChnCtx->stDebugInfo.stCalcOutputFps, pstChnCtx->stDebugInfo.stBufCnt.u32FinishOutputCnt
                         , pstChnCtx->stOutputPort.pstOutBufInfo->u64Pts);
        pstChnCtx->stDebugInfo.stOutputPortInfo.eFormat = pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.ePixelFormat;
        pstChnCtx->stDebugInfo.stOutputPortInfo.u16Width = pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.u16Width;
        pstChnCtx->stDebugInfo.stOutputPortInfo.u16Height = pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.u16Height;
#endif
        mi_sys_FinishBuf(pstChnCtx->stOutputPort.pstOutBufInfo);

    }

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
    pstChnCtx->stDebugInfo.stBufCnt.u32FinishTriggerCnt++;
    _MI_LDC_CalcTimeSpend(&pstChnCtx->stDebugInfo.stCalcHalTime);
#endif

    MHAL_WARP_ReadyForNext(instance);
    complete(&pstChnCtx->stChnCompletion);

    return 1;
}
static MI_S32 _MI_LDC_WorkThread(void *pData)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_SYS_BufInfo_t *pstInBufInfo = NULL;
    MI_SYS_BufInfo_t *pstOutBufInfo = NULL;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    MI_SYS_BufConf_t stBufConf;
    MI_LDC_Channel_t *pstChnCtx = (MI_LDC_Channel_t*)pData;
    MHAL_WARP_CONFIG *pConfig = &pstChnCtx->stWarpConfig;
    MHAL_WARP_INSTANCE_STATE_E eState;
#if 0
    MI_U8 i = 0;
    MI_U16 j = 0;
    MI_U8 *pAddrFrom = NULL;
    MI_U8 *pAddrTo = NULL;
    MI_U8 *pAddrBaseFrom = NULL;
    MI_U8 *pAddrBaseTo = NULL;
#endif
    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
    pstDev = &(_stWarpModule.stDev[pstChnCtx->u32DevId]);


    while(!kthread_should_stop())
    {
        DBG_INFO("step 1..waiton input, pstDev %p, devhandle %p\n", pstDev, pstDev->hDevHandle);
        if(mi_sys_WaitOnInputTaskAvailable(pstDev->hDevHandle, 50) != MI_SUCCESS)
        {
            continue;
        }
        mutex_lock(&pstDev->mtx);
        DBG_INFO("step 2..get input channel %d\n", pstChnCtx->u32ChnId);
        pstInBufInfo = mi_sys_GetInputPortBuf(pstDev->hDevHandle, pstChnCtx->u32ChnId, 0, 0);
        if (!pstInBufInfo)
        {
            DBG_WRN("Get inputport buf err\n");
            mutex_unlock(&pstDev->mtx);
            msleep(1);
            continue;
        }
        pstChnCtx->stInputPort.pstInBufInfo = pstInBufInfo;

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
        pstChnCtx->stDebugInfo.stBufCnt.u32GetInputCnt++;

        // calc input fps
        _MI_LDC_StatPts(&pstChnCtx->stDebugInfo.stCalcInputFps, pstChnCtx->stDebugInfo.stBufCnt.u32GetInputCnt, pstInBufInfo->u64Pts);
        pstChnCtx->stDebugInfo.stInputPortInfo.eFormat = pstInBufInfo->stFrameData.ePixelFormat;
        pstChnCtx->stDebugInfo.stInputPortInfo.u16Width = pstInBufInfo->stFrameData.u16Width;
        pstChnCtx->stDebugInfo.stInputPortInfo.u16Height = pstInBufInfo->stFrameData.u16Height;
#endif

        DBG_INFO("step 3..get output\n");
        stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
        stBufConf.stFrameCfg.eFormat = pstInBufInfo->stFrameData.ePixelFormat;
        stBufConf.stFrameCfg.u16Width = pstInBufInfo->stFrameData.u16Width;
        stBufConf.stFrameCfg.u16Height = pstInBufInfo->stFrameData.u16Height;
        stBufConf.stFrameCfg.eFrameScanMode = pstInBufInfo->stFrameData.eFrameScanMode;

        pstOutBufInfo = mi_sys_GetOutputPortBuf(pstDev->hDevHandle, pstChnCtx->u32ChnId, 0, &stBufConf, &bBlockedByRateCtrl);
        if(!pstOutBufInfo)
        {
            DBG_INFO("Warp block by rate control which is not my falut\n");
            mi_sys_FinishBuf(pstInBufInfo);
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
            pstChnCtx->stDebugInfo.stBufCnt.u32FinishInputCnt++;
#endif
            mutex_unlock(&pstDev->mtx);
            continue;
        }
        pstChnCtx->stOutputPort.pstOutBufInfo = pstOutBufInfo;

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
        pstChnCtx->stDebugInfo.stBufCnt.u32GetOutputCnt++;
#endif

        // warp process
        pConfig->input_image.width = pstInBufInfo->stFrameData.u16Width;
        pConfig->input_image.height = pstInBufInfo->stFrameData.u16Height;
        pConfig->output_image.width = pstOutBufInfo->stFrameData.u16Width;
        pConfig->output_image.height = pstOutBufInfo->stFrameData.u16Height;
        switch (pstInBufInfo->stFrameData.ePixelFormat)
        {
            case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            {
                pConfig->input_image.format = MHAL_WARP_IMAGE_FORMAT_YUV420;
                pConfig->input_data.num_planes = 2;
                pConfig->input_data.data[0] = pstInBufInfo->stFrameData.phyAddr[0];
                pConfig->input_data.data[1] = pstInBufInfo->stFrameData.phyAddr[1];
            }
            break;
            case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            {
                pConfig->input_image.format = MHAL_WARP_IMAGE_FORMAT_YUV422;
                pConfig->input_data.num_planes = 1;
                pConfig->input_data.data[0] = pstInBufInfo->stFrameData.phyAddr[0];
            }
            break;
            default:
                mi_sys_FinishBuf(pstInBufInfo);
                mi_sys_RewindBuf(pstOutBufInfo);
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
                pstChnCtx->stDebugInfo.stBufCnt.u32FinishInputCnt++;
                pstChnCtx->stDebugInfo.stBufCnt.u32RewindOutputCnt++;
#endif
                mutex_unlock(&pstDev->mtx);
                DBG_ERR("Input buffer format error!\n");
                continue;
        }
        switch (pstOutBufInfo->stFrameData.ePixelFormat)
        {
            case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            {
                pConfig->output_image.format = MHAL_WARP_IMAGE_FORMAT_YUV420;
                pConfig->output_data.num_planes = 2;
                pConfig->output_data.data[0] = pstOutBufInfo->stFrameData.phyAddr[0];
                pConfig->output_data.data[1] = pstOutBufInfo->stFrameData.phyAddr[1];
            }
            break;
            case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            {
                pConfig->output_image.format = MHAL_WARP_IMAGE_FORMAT_YUV422;
                pConfig->output_data.num_planes = 1;
                pConfig->output_data.data[0] = pstOutBufInfo->stFrameData.phyAddr[0];
            }
            break;
            default:
                mi_sys_FinishBuf(pstInBufInfo);
                mi_sys_RewindBuf(pstOutBufInfo);
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
                pstChnCtx->stDebugInfo.stBufCnt.u32FinishInputCnt++;
                pstChnCtx->stDebugInfo.stBufCnt.u32RewindOutputCnt++;
#endif
                mutex_unlock(&pstDev->mtx);
                DBG_ERR("Out buffer format error!\n");
                continue;
        }
        pstChnCtx->stOutputPort.pstOutBufInfo->u64Pts = pstChnCtx->stInputPort.pstInBufInfo->u64Pts;
        // proce a image
#if 0
        for (i = 0;
            i < ((pstChnCtx->stInputPort.pstInBufInfo->stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)?2:1);
            i++)
        {
            pAddrBaseFrom = pAddrFrom = mi_sys_Vmap(pstChnCtx->stInputPort.pstInBufInfo->stFrameData.phyAddr[i],
            pstChnCtx->stInputPort.pstInBufInfo->stFrameData.u32Stride[i] * pstChnCtx->stInputPort.pstInBufInfo->stFrameData.u16Height,
            FALSE);
            pAddrBaseTo = pAddrTo = mi_sys_Vmap(pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.phyAddr[i],
            pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.u32Stride[i] * pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.u16Height,
            FALSE);
            DBG_INFO("PHY addr is %llx\n", pstChnCtx->stInputPort.pstInBufInfo->stFrameData.phyAddr[i]);
            DBG_INFO("Input buf width %d height %d, stride %d\n", pstChnCtx->stInputPort.pstInBufInfo->stFrameData.u16Width,
                pstChnCtx->stInputPort.pstInBufInfo->stFrameData.u16Height, pstChnCtx->stInputPort.pstInBufInfo->stFrameData.u32Stride[i]);
            DBG_INFO("Output buf width %d height %d stride %d\n", pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.u16Width,
                pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.u16Height, pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.u32Stride[i]);
            for (j = 0; j < pstChnCtx->stInputPort.pstInBufInfo->stFrameData.u16Height; j++)
            {
                memcpy(pAddrTo, pAddrFrom, pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.u32Stride[i]);
                pAddrTo += pstChnCtx->stOutputPort.pstOutBufInfo->stFrameData.u32Stride[i];
                pAddrFrom += pstChnCtx->stInputPort.pstInBufInfo->stFrameData.u32Stride[i];
            }
            mi_sys_UnVmap(pAddrBaseFrom);
            mi_sys_UnVmap(pAddrBaseTo);
        }
        if (pstChnCtx->stInputPort.pstInBufInfo)
        {
            if(mi_sys_FinishBuf(pstChnCtx->stInputPort.pstInBufInfo) != MI_SUCCESS)
            {
                DBG_ERR("finish input buf fail \n");
            }
        }
        if (pstChnCtx->stOutputPort.pstOutBufInfo)
        {
            if(mi_sys_FinishBuf(pstChnCtx->stOutputPort.pstOutBufInfo) != MI_SUCCESS)
            {
                DBG_ERR("finish output buf fail \n");
            }
        }
#else
        eState = MHAL_WARP_CheckState(pstChnCtx->hInstHandle);
        if (MHAL_WARP_INSTANCE_STATE_READY != eState)
        {
            DBG_ERR("MHAL_WARP_CheckState Error, state is %d\n", eState);
            mi_sys_FinishBuf(pstInBufInfo);
            mi_sys_RewindBuf(pstOutBufInfo);
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
            pstChnCtx->stDebugInfo.stBufCnt.u32FinishInputCnt++;
            pstChnCtx->stDebugInfo.stBufCnt.u32RewindOutputCnt++;
#endif
            mutex_unlock(&pstDev->mtx);
            continue;
        }

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
        pstChnCtx->stDebugInfo.stBufCnt.u32TryTriggerCnt++;
        memset(&pstChnCtx->stDebugInfo.stCalcHalTime.stStartTmSpec, 0, sizeof(struct timespec));
        do_posix_clock_monotonic_gettime(&pstChnCtx->stDebugInfo.stCalcHalTime.stStartTmSpec);
#endif

        if (MHAL_SUCCESS != MHAL_WARP_Trigger(pstChnCtx->hInstHandle, pConfig, (MHAL_WARP_CALLBACK)_MI_LDC_Trigger_Callback, pstChnCtx))
        {
            DBG_ERR("MHAL_WARP_Trigger Error\n");
            mi_sys_FinishBuf(pstInBufInfo);
            mi_sys_RewindBuf(pstOutBufInfo);
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
            pstChnCtx->stDebugInfo.stBufCnt.u32FinishInputCnt++;
            pstChnCtx->stDebugInfo.stBufCnt.u32RewindOutputCnt++;
#endif
            mutex_unlock(&pstDev->mtx);
            continue;
        }
#endif
        mutex_unlock(&pstDev->mtx);
        while (!wait_for_completion_timeout(&pstChnCtx->stChnCompletion, msecs_to_jiffies(400)))
        {
            DBG_ERR("wait completion timeout\n");
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
            // dump tigger info
            DBG_INFO("Trigger time: avg=%llu max=%llu min=%llu cur=%llu\n", pstChnCtx->stDebugInfo.stCalcHalTime.u64TotalTime/pstChnCtx->stDebugInfo.stBufCnt.u32TryTriggerCnt
                    , pstChnCtx->stDebugInfo.stCalcHalTime.u64MaxSpendTime, pstChnCtx->stDebugInfo.stCalcHalTime.u64MinSpendTime
                    , _MI_LDC_StatSpendTime(&pstChnCtx->stDebugInfo.stCalcHalTime.stStartTmSpec));
#endif
        }

//        wait_for_completion(&pstChnCtx->stChnCompletion);
    }

    return MI_LDC_OK;
}

MI_S32 _MI_LDC_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort, void *pUsrData)
{
    MI_LDC_Device_t *pstDev;
    MI_S32 s32Ret;

    pstDev = (MI_LDC_Device_t *)pUsrData;
    s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_MODID(pstChnCurPort->eModId) || !MI_LDC_VALID_DEVID(pstChnCurPort->u32DevId)
       || !MI_LDC_VALID_CHNID(pstChnCurPort->u32ChnId) || !MI_LDC_VALID_OUTPUTPORTID(pstChnCurPort->u32PortId))
        return MI_ERR_LDC_FAIL;

    if(!_stWarpModule.bInited)
        goto exit;

    mutex_lock(&pstDev->mtx);
    //if(pstDev->eStatus == E_MI_LDC_DEVICE_UNINIT || pstDev->eStatus == E_MI_LDC_DEVICE_START)
    if(pstDev->eStatus == E_MI_LDC_DEVICE_UNINIT)
    {
        DBG_ERR("Bind outport failed............\n");
        goto exit_device;
    }

    pstDev->stChannel[pstChnCurPort->u32ChnId].stOutputPort.stBindPort = *pstChnPeerPort;
    pstDev->stChannel[pstChnCurPort->u32ChnId].stOutputPort.bBind = 1;
    s32Ret = MI_LDC_OK;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:

    return s32Ret;
}

MI_S32 _MI_LDC_OnUnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    MI_LDC_Device_t *pstDev = (MI_LDC_Device_t *)pUsrData;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;
    if(!MI_LDC_VALID_MODID(pstChnCurPort->eModId) || !MI_LDC_VALID_DEVID(pstChnCurPort->u32DevId)
       || !MI_LDC_VALID_CHNID(pstChnCurPort->u32ChnId) || !MI_LDC_VALID_OUTPUTPORTID(pstChnCurPort->u32PortId))
        return MI_ERR_LDC_FAIL;

    if(!_stWarpModule.bInited)
        goto exit;

    mutex_lock(&pstDev->mtx);
    if(pstDev->eStatus == E_MI_LDC_DEVICE_UNINIT || pstDev->eStatus == E_MI_LDC_DEVICE_START)
        goto exit_device;

    pstDev->stChannel[pstChnCurPort->u32ChnId].stOutputPort.bBind = 0;
    s32Ret = MI_LDC_OK ;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:

    return s32Ret;
}

MI_S32 _MI_LDC_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    MI_LDC_Device_t *pstDev = (MI_LDC_Device_t *)pUsrData;
    MI_LDC_Inputport_t *pstInputPort;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_MODID(pstChnCurPort->eModId) || !MI_LDC_VALID_DEVID(pstChnCurPort->u32DevId)
       || !MI_LDC_VALID_CHNID(pstChnCurPort->u32ChnId) || !MI_LDC_VALID_INPUTPORTID(pstChnCurPort->u32PortId))
    {
        DBG_ERR("Invalid cur port\n");
        DBG_ERR("mod: %d dev: %d chn: %d port: %d\n", pstChnCurPort->eModId, pstChnCurPort->u32DevId,
                 pstChnCurPort->u32ChnId, pstChnCurPort->u32PortId);
        return MI_ERR_LDC_FAIL;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        goto exit;
    }
    mutex_lock(&pstDev->mtx);

    pstInputPort = &pstDev->stChannel[pstChnCurPort->u32ChnId].stInputPort;
    if(pstDev->eStatus == E_MI_LDC_DEVICE_UNINIT)
    {
        DBG_ERR("Device not open\n");
        goto exit_device;
    }

    MI_SYS_BUG_ON(pstDev->stChannel[pstChnCurPort->u32ChnId].stInputPort.bBind);
    pstDev->stChannel[pstChnCurPort->u32ChnId].stInputPort.stBindPort = *pstChnPeerPort;
    pstDev->stChannel[pstChnCurPort->u32ChnId].stInputPort.bBind = 1;

    s32Ret = MI_LDC_OK;
    goto exit_device;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:

    return s32Ret;
}

MI_S32 _MI_LDC_OnUnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    MI_LDC_Device_t *pstDev = (MI_LDC_Device_t *)pUsrData;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;
    MI_LDC_Inputport_t *pstInputPort;

    if(!MI_LDC_VALID_MODID(pstChnCurPort->eModId) || !MI_LDC_VALID_DEVID(pstChnCurPort->u32DevId)
       || !MI_LDC_VALID_CHNID(pstChnCurPort->u32ChnId) || !MI_LDC_VALID_INPUTPORTID(pstChnCurPort->u32PortId))
    {
        DBG_ERR("Invalid cur port\n");
        return MI_ERR_LDC_FAIL;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        goto exit;
    }

    mutex_lock(&pstDev->mtx);
    pstInputPort = &pstDev->stChannel[pstChnCurPort->u32ChnId].stInputPort;
    if(pstDev->eStatus == E_MI_LDC_DEVICE_UNINIT)
    {
        DBG_ERR("Device not open\n");
        goto exit_device;
    }

    if(!pstInputPort->bBind)
    {
        DBG_ERR("Input port not bound\n");
        goto exit_device;
    }

    memset(&pstDev->stChannel[pstChnCurPort->u32ChnId].stInputPort.stBindPort, 0, sizeof(MI_SYS_ChnPort_t));
    pstDev->stChannel[pstChnCurPort->u32ChnId].stInputPort.bBind = 0;
    s32Ret = MI_LDC_OK;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:

    return s32Ret;
}


#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_LDC_PROCFS_DEBUG == 1)
#define MI_LDC_GET_FORMAT(fmt, strFmt) do { \
                            memset(strFmt, 0, sizeof(strFmt));  \
                            if (E_MI_SYS_PIXEL_FRAME_YUV422_YUYV == fmt)    \
                            {   \
                                strcpy(strFmt, "YUV422_YUYV");  \
                            }   \
                            else if (E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == fmt) \
                            {   \
                                strcpy(strFmt, "YUV420_SP");    \
                            }   \
                            else \
                            {   \
                                strcpy(strFmt, "Unknow");   \
                            }   \
} while(0);

MI_S32 _MI_LDC_OnDumpstDevAttr(MI_SYS_DEBUG_HANDLE_t handle,MI_U32 u32DevId,void *pUsrData)
{
    MI_U32 i = 0;
//    MI_U32 j = 0;
    MI_S8 szFormat[16];
    MI_LDC_Device_t *pstDev = NULL;
    pstDev = &(_stWarpModule.stDev[u32DevId]);

    mutex_lock(&pstDev->mtx);
    handle.OnPrintOut(handle, "\n----------------------------------- Dump Warp Dev%d InputPort Info -----------------------------------\n", u32DevId);
    handle.OnPrintOut(handle, "%7s%12s%7s%7s%12s%15s%15s%12s%12s\n", "ChnId", "Format", "Width", "Height", "GetInput", "FinishInput", "RewindInput", "Fps", "StatDepth");
    for (i = 0; i < MI_LDC_MAX_CHN_NUM; i++)
    {
        if (pstDev->stChannel[i].eChnStatus != E_MI_LDC_CHN_UNINIT)
        {
            MI_LDC_GET_FORMAT(pstDev->stChannel[i].stDebugInfo.stInputPortInfo.eFormat, szFormat);
            handle.OnPrintOut(handle, "%7d%12s%7d%7d%12d%15d%15d%10d.%1d%12d\n", i, szFormat, pstDev->stChannel[i].stDebugInfo.stInputPortInfo.u16Width
                              , pstDev->stChannel[i].stDebugInfo.stInputPortInfo.u16Height, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32GetInputCnt
                              , pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishInputCnt, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32RewindInputCnt
                              , _MI_LDC_ClacFps(&pstDev->stChannel[i].stDebugInfo.stCalcInputFps)/10, _MI_LDC_ClacFps(&pstDev->stChannel[i].stDebugInfo.stCalcInputFps)%10
                              , pstDev->stChannel[i].stDebugInfo.stCalcInputFps.u32StatDepth);
        }
    }
    handle.OnPrintOut(handle, "\n--------------------------------- End Dump Warp Dev%d InputPort Info ---------------------------------\n", u32DevId);

    handle.OnPrintOut(handle, "\n----------------------------------- Dump Warp Dev%d OutputPort Info -----------------------------------\n", u32DevId);
    handle.OnPrintOut(handle, "%7s%12s%7s%7s%12s%15s%15s%12s%12s\n", "ChnId", "Format", "Width", "Height", "GetOutput", "FinishOutput", "RewindOutput", "Fps", "StatDepth");
    for (i = 0; i < MI_LDC_MAX_CHN_NUM; i++)
    {
        if (pstDev->stChannel[i].eChnStatus != E_MI_LDC_CHN_UNINIT)
        {
            MI_LDC_GET_FORMAT(pstDev->stChannel[i].stDebugInfo.stOutputPortInfo.eFormat, szFormat);
            handle.OnPrintOut(handle, "%7d%12s%7d%7d%12d%15d%15d%10d.%1d%12d\n", i, szFormat, pstDev->stChannel[i].stDebugInfo.stOutputPortInfo.u16Width
                              , pstDev->stChannel[i].stDebugInfo.stOutputPortInfo.u16Height, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32GetOutputCnt
                              , pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishOutputCnt, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32RewindOutputCnt
                              , _MI_LDC_ClacFps(&pstDev->stChannel[i].stDebugInfo.stCalcOutputFps)/10, _MI_LDC_ClacFps(&pstDev->stChannel[i].stDebugInfo.stCalcOutputFps)%10
                              , pstDev->stChannel[i].stDebugInfo.stCalcOutputFps.u32StatDepth);
        }
    }
    handle.OnPrintOut(handle, "\n--------------------------------- End Dump Warp Dev%d OutputPort Info ---------------------------------\n", u32DevId);

    handle.OnPrintOut(handle, "\n-------------------------------- Dump Warp Dev%d Hal Info --------------------------------\n", u32DevId);
    handle.OnPrintOut(handle, "%7s%12s%12s%12s%15s%15s\n", "ChnId", "AvgTime(us)", "MaxTime(us)", "MinTime(us)", "TotalTrigger", "FinishTrigger");
    for (i = 0; i < MI_LDC_MAX_CHN_NUM; i++)
    {
        if (pstDev->stChannel[i].eChnStatus != E_MI_LDC_CHN_UNINIT)
        {
            if (!pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishTriggerCnt)
            {
                handle.OnPrintOut(handle, "%7d%12lld%12lld%12lld%15d%15d\n", i, 0, 0, 0, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32TryTriggerCnt, 0);
            }
            else
            {
                handle.OnPrintOut(handle, "%7d%12llu%12llu%12llu%15d%15d\n", i, div64_u64(pstDev->stChannel[i].stDebugInfo.stCalcHalTime.u64TotalTime, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishTriggerCnt)
                              , pstDev->stChannel[i].stDebugInfo.stCalcHalTime.u64MaxSpendTime, pstDev->stChannel[i].stDebugInfo.stCalcHalTime.u64MinSpendTime
                              , pstDev->stChannel[i].stDebugInfo.stBufCnt.u32TryTriggerCnt, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishTriggerCnt);
//                handle.OnPrintOut(handle, "\nRecent pts interval(us):\n");
//                for (j = 0; j < pstDev->stChannel[i].stDebugInfo.stCalcOutputFps.u32SampleCnt; j ++)
//                {
//                    handle.OnPrintOut(handle, "%2d:%8llu\n", j, *(pstDev->stChannel[i].stDebugInfo.stCalcOutputFps.pu64StatSet+j));
//                }
            }

        }
    }
    handle.OnPrintOut(handle, "\n------------------------------ End Dump Warp Dev%d Hal Info ------------------------------\n", u32DevId);
    mutex_unlock(&pstDev->mtx);

    return MI_SUCCESS;
}

/*
N.B. use handle.OnWriteOut to print
*/
MI_S32 _MI_LDC_OnDumpChannelAttr(MI_SYS_DEBUG_HANDLE_t handle,MI_U32 u32DevId,void *pUsrData)
{
    MI_LDC_Device_t *pstDev = NULL;
    pstDev = &(_stWarpModule.stDev[u32DevId]);

    return MI_SUCCESS;
}

/*
N.B. use handle.OnWriteOut to print
*/
MI_S32 _MI_LDC_OnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t handle,MI_U32 u32DevId,void *pUsrData)
{
    MI_LDC_Device_t *pstDev = NULL;
    pstDev = &(_stWarpModule.stDev[u32DevId]);

    return MI_SUCCESS;
}

MI_S32 _MI_LDC_OnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t handle,MI_U32 u32DevId, void *pUsrData)
{
    MI_LDC_Device_t *pstDev = NULL;
    pstDev = &(_stWarpModule.stDev[u32DevId]);

    return MI_SUCCESS;
}

MI_S32 _MI_LDC_OnHelp(MI_SYS_DEBUG_HANDLE_t  handle,MI_U32  u32DevId,void *pUsrData)
{
    MI_LDC_Device_t *pstDev = NULL;
    pstDev = &(_stWarpModule.stDev[u32DevId]);

    return MI_SUCCESS;
}
static MI_S32 _MI_LDC_IMPL_DumpInfoToFile(const MI_U8 *pszFileName, const MI_U8 *pszBuffer, MI_U32 u32Length)
{
    struct file *pFile = NULL;
    mm_segment_t stSeg;
    loff_t pos = 0;
    ssize_t writeBytes = 0;
    pFile = filp_open(pszFileName, O_RDWR | O_CREAT,0644);

    if (IS_ERR(pFile))
    {
        printk("Create file:%s error.\n", pszFileName);
        return -1;
    }

    stSeg = get_fs();
    set_fs(KERNEL_DS);
    writeBytes = vfs_write(pFile, (char*)pszBuffer, u32Length, &pos);
    filp_close(pFile, NULL);
    set_fs(stSeg);
    //printk("Write %d bytes to %s.\n", writeBytes, pszFileName);

    return (MI_S32)writeBytes;
}
static MI_S32 _MI_LDC_IMPL_DumpBufToFile(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_U8 *pFileName = NULL;
    MI_LDC_Device_t *pstDev = NULL;
    char *ps8After = NULL;
    MI_PHY phyTableAddr = 0;
    MI_U8 *pu8ConfAddr = NULL;
    MI_U32 u32Size = 0;
    MI_U8 u8Choice = simple_strtoul(argv[2], &ps8After, 10);
    MI_U8 u8Channel = simple_strtoul(argv[1], &ps8After, 10);

    if (MI_LDC_MAX_CHN_NUM <= u8Channel)
    {
        handle.OnPrintOut(handle, "Error Channel id!!!\n");
        return MI_SUCCESS;
    }
    pFileName = kmalloc(256, GFP_ATOMIC);
    MI_SYS_BUG_ON(!pFileName);
    memset(pFileName, 0, 256);
    pstDev = &(_stWarpModule.stDev[u32DevId]);

    mutex_lock(&pstDev->mtx);
    if (u8Choice == 0)
    {
        sprintf(pFileName, "%s/Warp_BbTable.bin", argv[3]);
        phyTableAddr = pstDev->stChannel[u8Channel].stWarpConfig.bb_table.table;
        u32Size = pstDev->stChannel[u8Channel].stWarpConfig.bb_table.size;
        pu8ConfAddr = mi_sys_Vmap(phyTableAddr, u32Size, FALSE);
        if (NULL == pu8ConfAddr)
        {
            DBG_EXIT_ERR("Sys map error!\n");
            mutex_unlock(&pstDev->mtx);
            kfree(pFileName);
            return MI_SUCCESS;
        }
        _MI_LDC_IMPL_DumpInfoToFile(pFileName, pu8ConfAddr, u32Size);
        mi_sys_UnVmap(pu8ConfAddr);
        handle.OnPrintOut(handle, "dump warp bb table to %s size %d\n", pFileName, u32Size);
    }
    else if (u8Choice == 1)
    {
        phyTableAddr = pstDev->stChannel[u8Channel].stWarpConfig.disp_table.table;
        u32Size = pstDev->stChannel[u8Channel].stWarpConfig.disp_table.size;
        switch (pstDev->stChannel[u8Channel].stWarpConfig.disp_table.format)
        {
            case MHAL_WARP_MAP_FORMAT_ABSOLUTE:
            {
                sprintf(pFileName, "%s/Disp_absolute.bin", argv[3]);
            }
            break;
            case MHAL_WARP_MAP_FORMAT_RELATIVE:
            {
                sprintf(pFileName, "%s/Disp_relative.bin", argv[3]);
            }
            break;
            default:
                DBG_EXIT_ERR("Format error!\n");
                mutex_unlock(&pstDev->mtx);
                kfree(pFileName);
                return MI_SUCCESS;
        }
        pu8ConfAddr = mi_sys_Vmap(phyTableAddr, u32Size, FALSE);
        if (NULL == pu8ConfAddr)
        {
            DBG_EXIT_ERR("Sys map error!\n");
            mutex_unlock(&pstDev->mtx);
            kfree(pFileName);
            return MI_SUCCESS;
        }
        _MI_LDC_IMPL_DumpInfoToFile(pFileName, pu8ConfAddr, u32Size);
        mi_sys_UnVmap(pu8ConfAddr);
        handle.OnPrintOut(handle, "dump warp disp table to %s size %d\n", pFileName, u32Size);
    }
    mutex_unlock(&pstDev->mtx);
    kfree(pFileName);

    return MI_SUCCESS;
}

static MI_S32 _MI_LDC_IMPL_DumpFps(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_U32 u32InputFps, u32OutputFps;
    MI_U32 i = 0;

    pstDev = &(_stWarpModule.stDev[u32DevId]);
    mutex_lock(&pstDev->mtx);
    for (i = 0; i < MI_LDC_MAX_CHN_NUM; i++)
    {
        if (pstDev->stChannel[i].eChnStatus != E_MI_LDC_CHN_UNINIT)
        {
            u32InputFps = _MI_LDC_ClacFps(&pstDev->stChannel[i].stDebugInfo.stCalcInputFps);
            u32OutputFps = _MI_LDC_ClacFps(&pstDev->stChannel[i].stDebugInfo.stCalcOutputFps);

            handle.OnPrintOut(handle, "devId=%d, chnId=%d, Inputport fps=%d.%1d\n", u32DevId, i, u32InputFps/10, u32InputFps%10);
            handle.OnPrintOut(handle, "devId=%d, chnId=%d, Outputport fps=%d.%1d\n", u32DevId, i, u32OutputFps/10, u32OutputFps%10);
        }
    }
    mutex_unlock(&pstDev->mtx);

    return MI_SUCCESS;
}

static MI_S32 _MI_LDC_IMPL_DumpFrameCnt(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_U32 i = 0;

    pstDev = &(_stWarpModule.stDev[u32DevId]);
    mutex_lock(&pstDev->mtx);
    for (i = 0; i < MI_LDC_MAX_CHN_NUM; i++)
    {
        if (pstDev->stChannel[i].eChnStatus != E_MI_LDC_CHN_UNINIT)
        {
            handle.OnPrintOut(handle, "devId=%d, chnId=%d, getInput=%d finishInput=%d, rewindInput=%d, getOutput=%d, finishOutput=%d, rewindOutput=%d, tryTrigger=%d, finishTrigger=%d\n"
                              , u32DevId, i, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32GetInputCnt, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishInputCnt
                              , pstDev->stChannel[i].stDebugInfo.stBufCnt.u32RewindInputCnt, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32GetOutputCnt
                              , pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishOutputCnt, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32RewindOutputCnt
                              , pstDev->stChannel[i].stDebugInfo.stBufCnt.u32TryTriggerCnt, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishTriggerCnt);
        }
    }
    mutex_unlock(&pstDev->mtx);

    return MI_SUCCESS;
}

static MI_S32 _MI_LDC_IMPL_DumpHalTimeConsume(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_U64 u64MaxTime, u64MinTime, u64AvgTime;
    MI_U32 i = 0;

    pstDev = &(_stWarpModule.stDev[u32DevId]);
    mutex_lock(&pstDev->mtx);
    for (i = 0; i < MI_LDC_MAX_CHN_NUM; i++)
    {
        if (pstDev->stChannel[i].eChnStatus != E_MI_LDC_CHN_UNINIT)
        {
            u64MaxTime = pstDev->stChannel[i].stDebugInfo.stCalcHalTime.u64MaxSpendTime;
            u64MinTime = pstDev->stChannel[i].stDebugInfo.stCalcHalTime.u64MinSpendTime;
            u64AvgTime = pstDev->stChannel[i].stDebugInfo.stCalcHalTime.u64TotalTime
                         / pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishTriggerCnt;
            handle.OnPrintOut(handle, "devId=%d, chnId=%d, tryTrigger=%d finishTrigger=%d, avgTime=%llu, maxTime=%llu, minTime=%llu\n", u32DevId
                              , i, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32TryTriggerCnt, pstDev->stChannel[i].stDebugInfo.stBufCnt.u32FinishTriggerCnt
                              , u64AvgTime, u64MaxTime, u64MinTime);
        }
    }
    mutex_unlock(&pstDev->mtx);

    return MI_SUCCESS;
}

static MI_S32 _MI_LDC_IMPL_DumpSetStatDepth(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_LDC_Device_t *pstDev = NULL;
    char *ps8After = NULL;
    MI_U32 i = 0;
    MI_U32 u32Depth = simple_strtoul(argv[1], &ps8After, 10);

    pstDev = &(_stWarpModule.stDev[u32DevId]);
    mutex_lock(&pstDev->mtx);
    for (i = 0; i < MI_LDC_MAX_CHN_NUM; i++)
    {
        if (pstDev->stChannel[i].eChnStatus != E_MI_LDC_CHN_UNINIT)
        {
            _MI_LDC_SetStatFpsDepth(&pstDev->stChannel[i].stDebugInfo.stCalcInputFps, u32Depth);
            handle.OnPrintOut(handle, "set warp inputport stat depth to %d, devId=%d, chnId=%d\n", u32Depth, u32DevId, i);

            _MI_LDC_SetStatFpsDepth(&pstDev->stChannel[i].stDebugInfo.stCalcOutputFps, u32Depth);
            handle.OnPrintOut(handle, "set warp outputport stat depth to %d, devId=%d, chnId=%d\n", u32Depth, u32DevId, i);
        }
    }
    mutex_unlock(&pstDev->mtx);

    return MI_SUCCESS;
}


#endif

MI_S32 MI_LDC_IMPL_Init(void)
{
    MI_S32 i;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;
    if(_stWarpModule.bInited)
    {
        DBG_ERR("WRAP IMPL: inited already\n");
        s32Ret = MI_ERR_LDC_MOD_INITED;
        goto exit;
    }

    memset(&_stWarpModule,0,sizeof(_stWarpModule));

    for(i=0; i < MI_LDC_MAX_DEVICE_NUM; i++)
    {
        mutex_init(&_stWarpModule.stDev[i].mtx);
    }

    _stWarpModule.bInited = TRUE;
    s32Ret = MI_LDC_OK;
exit:
    return s32Ret;
}
MI_S32 MI_LDC_IMPL_DeInit(void)
{
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;
    MI_S32 i;
    MI_LDC_Device_t *pstDev = NULL;

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    for(i=0; i < MI_LDC_MAX_DEVICE_NUM; i++)
    {
        pstDev = &(_stWarpModule.stDev[i]);
        if(pstDev->eStatus != E_MI_LDC_DEVICE_UNINIT)
        {
            DBG_ERR("Device %d not closed\n", i);
            s32Ret = MI_ERR_LDC_DEV_NOT_CLOSE;
            goto exit;
        }
    }

    for(i=0; i < MI_LDC_MAX_DEVICE_NUM; i++)
    {
        mutex_destroy(&_stWarpModule.stDev[i].mtx);
    }

    _stWarpModule.bInited = 0;
    s32Ret = MI_LDC_OK;

exit:

    return s32Ret;
}

MI_S32 MI_LDC_IMPL_CreateDevice(MI_LDC_DEV devId)
{
    MI_LDC_Device_t *pstDev = NULL;
    mi_sys_ModuleDevInfo_t stModInfo;
    mi_sys_ModuleDevBindOps_t stBindOps;
    MI_U32 u32IrqId = 0;
    MI_S32 s32IrqRet;
    MI_S32 i;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
#endif

    // check valid
    if(!MI_LDC_VALID_DEVID(devId))
    {
        DBG_ERR("Invalid Device Id\n");
        return MI_ERR_LDC_INVALID_DEVID;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);
    mutex_lock(&pstDev->mtx);
    pstDev->u32DeviceId = devId;
    if(pstDev->eStatus != E_MI_LDC_DEVICE_UNINIT)
    {
        DBG_ERR("Device Opened already\n");
        s32Ret = MI_ERR_LDC_DEV_OPENED;
        goto exit_device;
    }

    // initial dev, chn, inputport, outputport to uinit

    for (i = 0; i < MI_LDC_MAX_CHN_NUM; i++)
    {
        pstDev->stChannel[i].stInputPort.stInputPort.eModId = E_MI_MODULE_ID_LDC;
        pstDev->stChannel[i].stInputPort.stInputPort.u32DevId = devId;
        pstDev->stChannel[i].stInputPort.stInputPort.u32ChnId = i;
        pstDev->stChannel[i].stInputPort.stInputPort.u32PortId = 0;
        pstDev->stChannel[i].stInputPort.u64Try = 0;
        pstDev->stChannel[i].stInputPort.u64RecvOk = 0;

        pstDev->stChannel[i].stOutputPort.stOutputPort.eModId = E_MI_MODULE_ID_LDC;
        pstDev->stChannel[i].stOutputPort.stOutputPort.u32DevId = devId;
        pstDev->stChannel[i].stOutputPort.stOutputPort.u32ChnId = i;
        pstDev->stChannel[i].stOutputPort.stOutputPort.u32PortId = 0;
        pstDev->stChannel[i].stOutputPort.u64SendOk = 0;
        pstDev->stChannel[i].stOutputPort.bInited = FALSE;

        pstDev->stChannel[i].eChnStatus = E_MI_LDC_CHN_UNINIT;
        pstDev->stChannel[i].u32ChnId = i;
        pstDev->stChannel[i].hInstHandle = NULL;
    }

    stModInfo.eModuleId = E_MI_MODULE_ID_LDC;
    stModInfo.u32DevId = devId;
    stModInfo.u32DevChnNum = MI_LDC_MAX_CHN_NUM;
    stModInfo.u32InputPortNum = 1;
    stModInfo.u32OutputPortNum = 1;

    stBindOps.OnBindInputPort = _MI_LDC_OnBindInputPort;
    stBindOps.OnBindOutputPort = _MI_LDC_OnBindOutputPort;
    stBindOps.OnUnBindInputPort = _MI_LDC_OnUnBindInputPort;
    stBindOps.OnUnBindOutputPort = _MI_LDC_OnUnBindOutputPort;
    stBindOps.OnOutputPortBufRelease = NULL;

#ifdef MI_SYS_PROC_FS_DEBUG
    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
#if(MI_LDC_PROCFS_DEBUG ==1)
    pstModuleProcfsOps.OnDumpDevAttr = _MI_LDC_OnDumpstDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = _MI_LDC_OnDumpChannelAttr;
    pstModuleProcfsOps.OnDumpInputPortAttr = _MI_LDC_OnDumpInputPortAttr;
    pstModuleProcfsOps.OnDumpOutPortAttr = _MI_LDC_OnDumpOutPortAttr;
    pstModuleProcfsOps.OnHelp = _MI_LDC_OnHelp;

#else
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif

#endif

    pstDev->hDevHandle = mi_sys_RegisterDev(&stModInfo, &stBindOps, pstDev
                                        #ifdef MI_SYS_PROC_FS_DEBUG
                                        , &pstModuleProcfsOps
                                        ,MI_COMMON_GetSelfDir
                                        #endif
                                      );
    if(NULL == pstDev->hDevHandle)
    {
        DBG_ERR("Register Module Device fail\n");
        s32Ret = MI_ERR_LDC_FAIL;
        goto exit_device;
    }
#if (MI_LDC_PROCFS_DEBUG == 1)
    mi_sys_RegistCommand("dump_table2file", 3, _MI_LDC_IMPL_DumpBufToFile, pstDev->hDevHandle);
    mi_sys_RegistCommand("dump_fps", 0, _MI_LDC_IMPL_DumpFps, pstDev->hDevHandle);
    mi_sys_RegistCommand("dump_frameCnt", 0, _MI_LDC_IMPL_DumpFrameCnt, pstDev->hDevHandle);
    mi_sys_RegistCommand("dump_halTimeConsume", 0, _MI_LDC_IMPL_DumpHalTimeConsume, pstDev->hDevHandle);
    mi_sys_RegistCommand("dump_SetStatDepth", 1, _MI_LDC_IMPL_DumpSetStatDepth, pstDev->hDevHandle);
#endif
    // create mhal_warp
    if (MHAL_SUCCESS != MHAL_WARP_CreateDevice(devId, &pstDev->hMhalHandle) )
    {
        DBG_ERR("Create mhal_warp dev %d failed\n", devId);
        s32Ret = MI_ERR_LDC_FAIL;
        goto exit_device;
    }

    // enable irq
    u32IrqId = MHAL_WARP_GetIrqNum();
    DBG_INFO("u32IrqId = %d.\n", u32IrqId);
    s32IrqRet = request_irq(u32IrqId, _MI_LDC_ISR_Proc, IRQ_TYPE_LEVEL_HIGH, "mi_warp_isr", (void *)pstDev->hMhalHandle);
    if(0 != s32IrqRet)
    {
        DBG_ERR("request_irq failed. u32IrqId = %u, s32IrqRet = %d.\n\n ", u32IrqId, s32IrqRet);
    }
//    MHAL_WARP_EnableIsr(TRUE);

    pstDev->eStatus = E_MI_LDC_DEVICE_INIT;    // createDev uinit->init
    s32Ret = MI_LDC_OK;
    mutex_unlock(&pstDev->mtx);

    goto exit;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:
    return s32Ret;


}

MI_S32 MI_LDC_IMPL_DestroyDevice(MI_LDC_DEV devId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_LDC_Inputport_t *pstInputPort;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;
    MI_U32 u32IrqId = 0;
    MI_S32 i;

    // check valid
    if(!MI_LDC_VALID_DEVID(devId))
    {
        DBG_ERR("Invalid devId=%d\n", devId);
        return MI_ERR_LDC_INVALID_DEVID;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not init\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);
    mutex_lock(&pstDev->mtx);

    if(pstDev->eStatus == E_MI_LDC_DEVICE_UNINIT)
    {
        DBG_ERR("Device not opened\n");
        s32Ret = MI_ERR_LDC_DEV_NOT_OPEN;
        goto exit_device;
    }

    if(E_MI_LDC_DEVICE_START == pstDev->eStatus)
    {
        DBG_ERR("Device is working: %d\n", pstDev->eStatus);
        s32Ret = MI_ERR_LDC_DEV_NOT_STOP;
        goto exit_device;
    }

    // check if able to destroy dev
    for(i = 0; i < MI_LDC_MAX_CHN_NUM; i++)
    {
        if (pstDev->stChannel[i].eChnStatus != E_MI_LDC_CHN_UNINIT)
        {
            DBG_ERR("channel %d is still working\n", i);
            s32Ret = MI_ERR_LDC_CHN_NOT_CLOSE;
            goto exit_device;
        }

        pstInputPort = &pstDev->stChannel[i].stInputPort;

        if(pstInputPort->bBind)
        {
            DBG_ERR("Inputport is still bound\n");
            s32Ret = MI_ERR_LDC_PORT_NOT_UNBIND;
            goto exit_device;
        }

        pstDev->stChannel[i].stOutputPort.bInited = FALSE;
    }
    u32IrqId = MHAL_WARP_GetIrqNum();
    free_irq(u32IrqId, pstDev->hMhalHandle);
    if (MHAL_SUCCESS != MHAL_WARP_DestroyDevice(pstDev->hMhalHandle))
    {
        DBG_ERR("Mhal destroy dev %d failed\n", devId);
        s32Ret = MI_ERR_LDC_FAIL;
        goto exit_device;
    }

    MI_SYS_BUG_ON(pstDev->hDevHandle == NULL);
    mi_sys_UnRegisterDev(pstDev->hDevHandle);
    pstDev->eStatus = E_MI_LDC_DEVICE_UNINIT;

    s32Ret = MI_LDC_OK;
exit_device:
    mutex_unlock(&pstDev->mtx);
exit:
    return s32Ret;

}

MI_S32 MI_LDC_IMPL_StartDev(MI_LDC_DEV devId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_DEVID(devId))
    {
        DBG_ERR("Invalid devId=%d\n", devId);
        return MI_ERR_LDC_INVALID_DEVID;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);
    mutex_lock(&pstDev->mtx);
    if(E_MI_LDC_DEVICE_UNINIT == pstDev->eStatus)
    {
        DBG_ERR("Device not open\n");
        s32Ret = MI_ERR_LDC_DEV_NOT_OPEN;
        goto exit_device;
    }
    pstDev->eStatus = E_MI_LDC_DEVICE_START;
    s32Ret = MI_LDC_OK;
exit_device:
    mutex_unlock(&pstDev->mtx);
exit:
    return s32Ret;
}

MI_S32 MI_LDC_IMPL_StopDev(MI_LDC_DEV devId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_DEVID(devId))
    {
        DBG_ERR("Invalid devId=%d\n", devId);
        return MI_ERR_LDC_INVALID_DEVID;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);
    mutex_lock(&pstDev->mtx);
    if(E_MI_LDC_DEVICE_UNINIT == pstDev->eStatus)
    {
        DBG_ERR("Device not open\n");
        s32Ret = MI_ERR_LDC_DEV_NOT_OPEN;
        goto exit_device;
    }

    pstDev->eStatus = E_MI_LDC_DEVICE_STOP;
    s32Ret = MI_LDC_OK;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:
    return s32Ret;
}

// channel and input/output port disabled defaultly
MI_S32 MI_LDC_IMPL_CreateChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_DEVID(devId) || !MI_LDC_VALID_CHNID(chnId))
    {
        DBG_ERR("Invalid Param: devId=%d, Chn=%d\n", devId, chnId);
        return MI_ERR_LDC_ILLEGAL_PARAM;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);
    mutex_lock(&pstDev->mtx);
    if (pstDev->stChannel[chnId].eChnStatus != E_MI_LDC_CHN_UNINIT)
    {
        DBG_ERR("device %d chn %d has been Created\n", devId);
        s32Ret = MI_ERR_LDC_CHN_OPENED;
        goto exit_device;
    }

    if (MHAL_SUCCESS != MHAL_WARP_CreateInstance(pstDev->hMhalHandle, &pstDev->stChannel[chnId].hInstHandle))
    {
        DBG_ERR("device %d chn %d fail to create instance in HAL layer.\n", devId, chnId);
        s32Ret = MI_ERR_LDC_FAIL;
        goto exit_device;
    }

    pstDev->stChannel[chnId].u32DevId = devId;
    pstDev->stChannel[chnId].u32ChnId = chnId;
    pstDev->stChannel[chnId].eChnStatus = E_MI_LDC_CHN_INIT;
    pstDev->stChannel[chnId].stOutputPort.bInited = FALSE;

    pstDev->stChannel[chnId].stWarpConfig.op_mode = MHAL_WARP_OP_MODE_MAP;
    pstDev->stChannel[chnId].stWarpConfig.fill_value[0] = 0;
    pstDev->stChannel[chnId].stWarpConfig.fill_value[1] = 0x80;
    pstDev->stChannel[chnId].stWarpConfig.disp_table.resolution = MHAL_WARP_MAP_RESLOUTION_16X16;

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
    _MI_LDC_InitialDebugInfo(&pstDev->stChannel[chnId].stDebugInfo);
#endif

    init_completion(&pstDev->stChannel[chnId].stChnCompletion);

    // create chn workthread
    pstDev->stChannel[chnId].stWorkThread = kthread_run(_MI_LDC_WorkThread, &pstDev->stChannel[chnId], "WarpTest-Chn%d", chnId);
    if(IS_ERR(pstDev->stChannel[chnId].stWorkThread))
    {
        DBG_ERR("Create test thread fail\n");
        s32Ret = MI_ERR_LDC_FAIL;
        goto exit_device;
    }

    s32Ret = MI_LDC_OK;
    goto exit_device;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:
    return s32Ret;
}

MI_S32 MI_LDC_IMPL_DestroyChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;
    MHAL_WARP_CONFIG *pstWarpConfig = NULL;

    if(!MI_LDC_VALID_DEVID(devId) || !MI_LDC_VALID_CHNID(chnId))
    {
        DBG_ERR("Invalid Param: devId=%d, Chn=%d\n", devId, chnId);
        return MI_ERR_LDC_ILLEGAL_PARAM;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);
    mutex_lock(&pstDev->mtx);
    if(E_MI_LDC_CHN_UNINIT == pstDev->stChannel[chnId].eChnStatus)
    {
        DBG_ERR("device %d chn %d  chn not inited\n", devId, chnId);
        s32Ret = MI_ERR_LDC_CHN_NOT_OPEN;
        goto exit_device;
    }

    if (E_MI_LDC_CHN_ENABLED == pstDev->stChannel[chnId].eChnStatus)
    {
        DBG_ERR("device %d chn %d  is working\n", devId, chnId);
        s32Ret = MI_ERR_LDC_CHN_NOT_STOP;
        goto exit_device;
    }

    if (pstDev->stChannel[chnId].stOutputPort.bInited)
    {
        DBG_ERR("device %d chn %d outputport is working\n", devId, chnId);
        s32Ret = MI_ERR_LDC_PORT_NOT_DISABLE;
        goto exit_device;
    }

    // stop chn work thread
    if(pstDev->stChannel[chnId].stWorkThread)
    {
//        DBG_ERR("begin to stop chn thread\n");
        kthread_stop(pstDev->stChannel[chnId].stWorkThread);
//        DBG_ERR("stop chn thread finished\n");
        pstDev->stChannel[chnId].stWorkThread = NULL;
    }

    if (MHAL_SUCCESS != MHAL_WARP_DestroyInstance(pstDev->stChannel[chnId].hInstHandle))
    {
        DBG_ERR("device %d chn %d fail to destroy instance in HAL layer.\n", devId, chnId);
        s32Ret = MI_ERR_LDC_FAIL;
        goto exit_device;
    }

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_LDC_PROCFS_DEBUG == 1)
    if (pstDev->stChannel[chnId].eChnStatus != E_MI_LDC_CHN_UNINIT)
    {
        _MI_LDC_DeinitDebugInfo(&pstDev->stChannel[chnId].stDebugInfo);
    }
#endif

    pstDev->stChannel[chnId].hInstHandle = NULL;
    pstDev->stChannel[chnId].eChnStatus = E_MI_LDC_CHN_UNINIT;
    pstDev->stChannel[chnId].stOutputPort.bInited = FALSE;
    pstWarpConfig = &pstDev->stChannel[chnId].stWarpConfig;

    if (pstWarpConfig->bb_table.table)
    {
        mi_sys_MMA_Free(pstWarpConfig->bb_table.table);
        pstWarpConfig->bb_table.table = 0;
    }
    if (pstWarpConfig->disp_table.table)
    {
        mi_sys_MMA_Free(pstWarpConfig->disp_table.table);
        pstWarpConfig->disp_table.table = 0;
    }
    memset(pstWarpConfig, 0, sizeof(MHAL_WARP_CONFIG));
    s32Ret = MI_LDC_OK;

exit_device:
    mutex_unlock(&pstDev->mtx);

exit:

    return s32Ret;
}

MI_S32 MI_LDC_IMPL_EnableChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if (!MI_LDC_VALID_DEVID(devId) || !MI_LDC_VALID_CHNID(chnId))
    {
        DBG_ERR("Invalid Param: devId=%d, Chn=%d\n", devId, chnId);
        return MI_ERR_LDC_ILLEGAL_PARAM;
    }

    if (!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);

    DBG_INFO("pstDev %p\n", pstDev);

    mutex_lock(&pstDev->mtx);
    DBG_INFO("enter mutex\n");
    if (E_MI_LDC_CHN_UNINIT == pstDev->stChannel[chnId].eChnStatus)
    {
        DBG_ERR("device %d chn %d not inited\n", devId, chnId);
        s32Ret = MI_ERR_LDC_CHN_NOT_OPEN;
        goto exit_device;
    }

    DBG_INFO("chn states %d\n", pstDev->stChannel[chnId].eChnStatus);
    if (E_MI_LDC_CHN_ENABLED != pstDev->stChannel[chnId].eChnStatus)
    {
        mi_sys_EnableChannel(pstDev->hDevHandle, chnId);
        wake_up_process(pstDev->stChannel[chnId].stWorkThread);
        pstDev->stChannel[chnId].eChnStatus = E_MI_LDC_CHN_ENABLED;
    }
    s32Ret = MI_LDC_OK;

exit_device:
    DBG_INFO("leave mutex\n");
    mutex_unlock(&pstDev->mtx);

exit:
    return s32Ret;
}

MI_S32 MI_LDC_IMPL_DisableChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_DEVID(devId) || !MI_LDC_VALID_CHNID(chnId))
    {
        DBG_ERR("Invalid Param: devId=%d, Chn=%d\n", devId, chnId);
        return MI_ERR_LDC_ILLEGAL_PARAM;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);

    mutex_lock(&pstDev->mtx);

    if (E_MI_LDC_CHN_UNINIT == pstDev->stChannel[chnId].eChnStatus)
    {
        DBG_ERR("device %d chn %d is not inited\n", devId, chnId);
        s32Ret = MI_ERR_LDC_CHN_NOT_OPEN;
        goto exit_device;
    }

    if (pstDev->stChannel[chnId].stOutputPort.bInited)
    {
        DBG_ERR("device %d chn %d outputport is working\n", devId, chnId);
        s32Ret = MI_ERR_LDC_PORT_NOT_DISABLE;
        goto exit_device;
    }

    if (E_MI_LDC_CHN_ENABLED == pstDev->stChannel[chnId].eChnStatus)
    {
        mi_sys_DisableChannel(pstDev->hDevHandle, chnId);
        pstDev->stChannel[chnId].eChnStatus = E_MI_LDC_CHN_DISABLED;
    }

    pstDev->stChannel[chnId].stOutputPort.bInited = FALSE;

    s32Ret = MI_LDC_OK;

exit_device:
    mutex_unlock(&pstDev->mtx);

exit:
    return s32Ret;
}

MI_S32 MI_LDC_IMPL_EnableInputPort(MI_LDC_DEV devId, MI_LDC_CHN chnId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_DEVID(devId) || !MI_LDC_VALID_CHNID(chnId))
    {
        DBG_ERR("Invalid Param: devId=%d, Chn=%d\n", devId, chnId);
        return MI_ERR_LDC_ILLEGAL_PARAM;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);

    mutex_lock(&pstDev->mtx);
    if (E_MI_LDC_CHN_ENABLED != pstDev->stChannel[chnId].eChnStatus)
    {
        DBG_ERR("Chn %d or inputport not inited\n", chnId);
        s32Ret = MI_ERR_LDC_CHN_NOT_OPEN;
        goto exit_device;
    }

    mi_sys_EnableInputPort(pstDev->hDevHandle, chnId, 0);

    s32Ret = MI_LDC_OK;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:
    return s32Ret;
}

MI_S32 MI_LDC_IMPL_DisableInputPort(MI_LDC_DEV devId, MI_LDC_CHN chnId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_DEVID(devId) || !MI_LDC_VALID_CHNID(chnId))
    {
        DBG_ERR("Invalid Param: devId=%d, Chn=%d\n", devId, chnId);
        return MI_ERR_LDC_ILLEGAL_PARAM;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);

    mutex_lock(&pstDev->mtx);
    if (E_MI_LDC_CHN_ENABLED != pstDev->stChannel[chnId].eChnStatus)
    {
        DBG_ERR("Input port not inited\n");
        s32Ret = MI_ERR_LDC_CHN_NOT_OPEN;
        goto exit_device;
    }

    mi_sys_DisableInputPort(pstDev->hDevHandle, chnId, 0);
    s32Ret = MI_LDC_OK;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:
    return s32Ret;
}

MI_S32 MI_LDC_IMPL_EnableOutputPort(MI_LDC_DEV devId, MI_LDC_CHN chnId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_DEVID(devId) || !MI_LDC_VALID_CHNID(chnId))
    {
        DBG_ERR("Invalid Param: devId=%d, Chn=%d\n", devId, chnId);
        return MI_ERR_LDC_ILLEGAL_PARAM;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);

    mutex_lock(&pstDev->mtx);
    if (E_MI_LDC_CHN_ENABLED != pstDev->stChannel[chnId].eChnStatus)
    {
        DBG_ERR("Chn %d or inputport not inited\n", chnId);
        s32Ret = MI_ERR_LDC_CHN_NOT_OPEN;
        goto exit_device;
    }

    mi_sys_EnableOutputPort(pstDev->hDevHandle, chnId, 0);
    if(!pstDev->stChannel[chnId].stOutputPort.bInited)
    {
        pstDev->stChannel[chnId].stOutputPort.bInited = TRUE;
    }
    s32Ret = MI_LDC_OK;

exit_device:
    mutex_unlock(&pstDev->mtx);
exit:
    return s32Ret;
}

MI_S32 MI_LDC_IMPL_DisableOutputPort(MI_LDC_DEV devId, MI_LDC_CHN chnId)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;

    if(!MI_LDC_VALID_DEVID(devId) || !MI_LDC_VALID_CHNID(chnId))
    {
        DBG_ERR("Invalid Param: devId=%d, Chn=%d\n", devId, chnId);
        return MI_ERR_LDC_ILLEGAL_PARAM;
    }

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;
        goto exit;
    }

    pstDev = &(_stWarpModule.stDev[devId]);

    mutex_lock(&pstDev->mtx);
    if (E_MI_LDC_CHN_ENABLED != pstDev->stChannel[chnId].eChnStatus)
    {
        DBG_ERR("Input port not inited\n");
        s32Ret = MI_ERR_LDC_CHN_NOT_OPEN;
        goto exit_device;
    }

    mi_sys_DisableOutputPort(pstDev->hDevHandle, chnId, 0);
    if(pstDev->stChannel[chnId].stOutputPort.bInited)
    {
        pstDev->stChannel[chnId].stOutputPort.bInited = FALSE;
    }

    s32Ret = MI_LDC_OK;
exit_device:
    mutex_unlock(&pstDev->mtx);
exit:
    return s32Ret;
}


MI_S32 MI_LDC_IMPL_SetTable(MI_LDC_DEV devId, MI_LDC_CHN chnId, void *pTableAddr, MI_U32 u32TableSize)
{
    MI_LDC_Device_t *pstDev = NULL;
    MI_S32 s32Ret = MI_ERR_LDC_FAIL;
    MHAL_WARP_CONFIG *pstMHalWarpCfg = NULL;
    MI_PHY phyTableAddr;
    MI_U8 *pu8Addr = NULL;

    if(!_stWarpModule.bInited)
    {
        DBG_ERR("Module not inited\n");
        s32Ret = MI_ERR_LDC_MOD_NOT_INIT;

        return s32Ret;
    }
    if(!MI_LDC_VALID_DEVID(devId) || !MI_LDC_VALID_CHNID(chnId))
    {
        DBG_ERR("Invalid Param: devId=%d, Chn=%d\n", devId, chnId);
        return MI_ERR_LDC_ILLEGAL_PARAM;
    }
    if (pTableAddr == NULL || u32TableSize == 0)
    {
        DBG_ERR("Table size of addr error!\n");

        return MI_ERR_LDC_ILLEGAL_PARAM;
    }

    pstDev = &(_stWarpModule.stDev[devId]);
    pstMHalWarpCfg = &pstDev->stChannel[chnId].stWarpConfig;
    mutex_lock(&pstDev->mtx);
    if (MI_SUCCESS != mi_sys_MMA_Alloc(NULL, u32TableSize, &phyTableAddr))
    {
        DBG_EXIT_ERR("Sys alloc error!\n");
        s32Ret = MI_ERR_LDC_NULL_PTR;
        mutex_unlock(&pstDev->mtx);

        return s32Ret;
    }
    pu8Addr = mi_sys_Vmap(phyTableAddr, u32TableSize, FALSE);
    if (NULL == pu8Addr)
    {
        DBG_EXIT_ERR("Sys map error!\n");
        s32Ret = MI_ERR_LDC_NULL_PTR;
        mi_sys_MMA_Free(phyTableAddr);
        mutex_unlock(&pstDev->mtx);

        return s32Ret;
    }
    copy_from_user((void *)pu8Addr, (void *)pTableAddr, u32TableSize);

    if (pstMHalWarpCfg->bb_table.table)
    {
        mi_sys_MMA_Free(pstMHalWarpCfg->bb_table.table);
    }
    pstMHalWarpCfg->bb_table.table = phyTableAddr;
    pstMHalWarpCfg->bb_table.size = u32TableSize;
    DBG_INFO("Bound Box table 0x%p size %d\n", pu8Addr, u32TableSize);
    mi_sys_UnVmap(pu8Addr);
    mutex_unlock(&pstDev->mtx);
    s32Ret = MI_LDC_OK;

    return s32Ret;

}
