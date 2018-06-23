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
#include <linux/string.h>
///#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/list.h>
#include "mi_print.h"
#include "mi_sys_internal.h"
#include "mhal_common.h"
#include "mi_vpe.h"
#include "mi_vpe_impl_internal.h"
//#define DBG_ROI(fmt, args...) DBG_INFO(fmt, ##args)
#define DBG_ROI(fmt, args...)
// MHAL ROI use 12bits
// MI to User use 8 bits
#define CONVERT_12BITS_TO_8BITS(data) (((data) + (1 << 3)) >> 4)

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------
// ROI Defination:
//   Crop Window
//    *----------------------------------------*
//    |                                        |
//    |   AccX[0]AccY[0]   AccX[1]AccY[1]      |
//    |   *---------------------*              |
//    |   |    ROI region       |              |
//    |   |                     |              |
//    |   *---------------------*              |
//    |   AccX[3]AccY[3]   AccX[2]AccY[2]      |
//    *----------------------------------------*
//--------------------------------------------------
static inline void _mi_vpe_impl_RoiCalcRegionToHal(const MI_SYS_WindowRect_t *pstCropInfo, const MI_SYS_WindowRect_t *pstRegion, MHalVpeIqWdrRoiConfig_t *pstHalRoi)
{
    pstHalRoi->u16RoiAccX[0] = pstRegion->u16X;
    pstHalRoi->u16RoiAccY[0] = pstRegion->u16Y;

    pstHalRoi->u16RoiAccX[1] = pstRegion->u16X + pstRegion->u16Width;
    pstHalRoi->u16RoiAccY[1] = pstRegion->u16Y;


    pstHalRoi->u16RoiAccX[3] = pstRegion->u16X;
    pstHalRoi->u16RoiAccY[3] = pstRegion->u16Y + pstRegion->u16Height;// * pstCropInfo->u16Width);

    pstHalRoi->u16RoiAccX[2] = pstRegion->u16X + pstRegion->u16Width;
    pstHalRoi->u16RoiAccY[2] = pstRegion->u16Y + pstRegion->u16Height;// * pstCropInfo->u16Width);
    DBG_INFO("%dx%d@(%d, %d)--> {(%d, %d), (%d, %d), (%d, %d), (%d, %d)}.\n",
        pstRegion->u16Width, pstRegion->u16Height, pstRegion->u16X, pstRegion->u16Y,
        pstHalRoi->u16RoiAccX[0], pstHalRoi->u16RoiAccY[0],
        pstHalRoi->u16RoiAccX[1], pstHalRoi->u16RoiAccY[1],
        pstHalRoi->u16RoiAccX[2], pstHalRoi->u16RoiAccY[2],
        pstHalRoi->u16RoiAccX[3], pstHalRoi->u16RoiAccY[3]
    );

}

MI_BOOL mi_vpe_impl_RoiGetTask(mi_sys_ChnTaskInfo_t *pstTask, mi_vpe_DevInfo_t *pstDevInfo, ROI_Task_t **ppstRoiTask, struct list_head *pListHead, struct semaphore *pSemphore)
{
    MI_BOOL bRet = FALSE;
    struct list_head *pos, *n;
    ROI_Task_t *pstRoi = NULL;
    if ((pstDevInfo->eRoiStatus == E_MI_VPE_ROI_STATUS_IDLE)
        || (pstDevInfo->eRoiStatus == E_MI_VPE_ROI_STATUS_UPDATED)
        )
    {
        up(pSemphore);
        list_for_each_safe(pos, n, pListHead)
        {
            pstRoi = container_of(pos, ROI_Task_t, list);
            if ((pstRoi->eRoiStatus == E_MI_VPE_ROI_STATUS_NEED_UPDATE)
                && (pstRoi->pstChnnInfo->VpeCh == pstTask->u32ChnId))
            {
                pstRoi->eRoiStatus   = E_MI_VPE_ROI_STATUS_RUNNING;
                pstDevInfo->eRoiStatus = E_MI_VPE_ROI_STATUS_RUNNING;
                *ppstRoiTask = pstRoi;
                DBG_ROI("Get ROI task: %p.\n", pstRoi);
                bRet = TRUE;
                break;
            }
        }
        down(pSemphore);
    }
    return bRet;
}

MI_BOOL mi_vpe_impl_RoiProcessTaskStart(mi_sys_ChnTaskInfo_t *pstTask, mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo, MI_SYS_FrameData_t *pstBuffFrame, ROI_Task_t *pstRoiTask)
{
    // ROI region set by MI API: MI_VPE_ATOM_GetChannelRegionLuma
    MHalVpeIqWdrRoiHist_t stCfg;
    MHalVpeIqWdrRoiConfig_t stIqRoiCfg[ROI_WINDOW_MAX];
    MI_SYS_WindowRect_t *pstRegion = NULL;
    int i = 0;
    DBG_ROI("Start ROI task: %p.\n", pstTask);

    MI_SYS_BUG_ON(pstRoiTask == NULL);
    MI_SYS_BUG_ON(pstRoiTask->u32MagicNumber != __MI_VPE_ROI_MAGIC__);
    MI_SYS_BUG_ON(pstRoiTask->pstRegion == NULL);
    stCfg.bEn = TRUE;
    stCfg.u8WinCount = (MI_U8)(pstRoiTask->pstRegion->u32RegionNum & 0xff);
    stCfg.enPipeSrc  = E_MHAL_IQ_ROISRC_BEFORE_WDR;

    pstRegion = pstRoiTask->pstRegion->pstWinRect;
    // ROI base on Crop Window
    for (i = 0; i < sizeof(stIqRoiCfg)/sizeof(stIqRoiCfg[0]); i++)
    {
       if (i < pstRoiTask->pstRegion->u32RegionNum)
       {
           _mi_vpe_impl_RoiCalcRegionToHal(&pstChnnInfo->stRealCrop, pstRegion, &stIqRoiCfg[i]);
           stIqRoiCfg[i].bEnSkip = FALSE;
       }
       else
       {
           stIqRoiCfg[i].bEnSkip = TRUE;
       }
       memcpy(&stCfg.stRoiCfg[i] , &stIqRoiCfg[i], sizeof(stIqRoiCfg[i]));
       pstRegion++;
    }
    MHalVpeIqSetWdrRoiHist(pstChnnInfo->pIqCtx, &stCfg);
#ifdef CONFIG_64BIT
    pstTask->u64Reserved1 = (MI_U64)pstRoiTask;
#else
    pstTask->u64Reserved1 = ((MI_U32)pstRoiTask)& 0xffffffffffffffff;
#endif
    DBG_ROI("pstChTask: %p u32Reserved1: %x.\n", pstTask, pstTask->u32Reserved1);
    pstTask->u32Reserved1 |= MI_VPE_TASK_ROI_UPDATE;
    DBG_ROI("pstChTask: %p u32Reserved1: %x.\n", pstTask, pstTask->u32Reserved1);
    // register release for ROI
    // TODO: tommy: system on need mask by MHAL
    return MHalVpeIqSetWdrRoiMask(pstChnnInfo->pIqCtx, FALSE, pstDevInfo->pstCmdMloadInfo);
}

void mi_vpe_impl_RoiProcessTaskEnd(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo)
{
    DBG_ROI("Process ROI task.\n");
    // Mask ROI register update.
    MHalVpeIqSetWdrRoiMask(pstChnnInfo->pIqCtx, TRUE, pstDevInfo->pstCmdMloadInfo);
}

void mi_vpe_impl_RoiProcessFinish(mi_vpe_DevInfo_t * pstDevInfo, mi_vpe_ChannelInfo_t * pstChnnInfo, mi_sys_ChnTaskInfo_t *pstTask)
{
    MHalVpeIqWdrRoiReport_t stRoi;

    DBG_ROI("Finish ROI task.\n");
    memset(&stRoi, 0, sizeof(stRoi));
    if (TRUE == MHalVpeIqGetWdrRoiHist(pstChnnInfo->pIqCtx, &stRoi))
    {
        // 1. for user API need check region max support in HW.
        // 2. HW caps: put in HAL.
        // Notify ROI Data already
        //pstDevInfo->stRoiInfo.eRoiStatus = E_MI_VPE_ROI_STATUS_UPDATED;
        //WAKE_UP_QUEUE_IF_NECESSARY(roi_task->vpe_roi_waitqueue);

#ifdef CONFIG_64BIT
        ROI_Task_t *pstRoiTask = (ROI_Task_t *)pstTask->u64Reserved1;
#else
        ROI_Task_t *pstRoiTask = (ROI_Task_t *)((MI_U32)(pstTask->u64Reserved1 & 0xffffffff));
#endif
        MI_SYS_BUG_ON(pstRoiTask == NULL);
        MI_SYS_BUG_ON(pstRoiTask->u32MagicNumber != __MI_VPE_ROI_MAGIC__);

        pstRoiTask->u32LumaData[0] = CONVERT_12BITS_TO_8BITS(stRoi.u32Y[0]);
        pstRoiTask->u32LumaData[1] = CONVERT_12BITS_TO_8BITS(stRoi.u32Y[1]);
        pstRoiTask->u32LumaData[2] = CONVERT_12BITS_TO_8BITS(stRoi.u32Y[2]);
        pstRoiTask->u32LumaData[3] = CONVERT_12BITS_TO_8BITS(stRoi.u32Y[3]);
        pstRoiTask->eRoiStatus = E_MI_VPE_ROI_STATUS_UPDATED;
        WAKE_UP_QUEUE_IF_NECESSARY(pstRoiTask->queue);
        pstDevInfo->eRoiStatus = E_MI_VPE_ROI_STATUS_UPDATED;
    }
}
