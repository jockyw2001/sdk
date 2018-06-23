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
#include <linux/semaphore.h>
#include <linux/types.h>
#include "mhal_common.h"
#include "mi_vpe_impl_internal.h"
#include "mhal_vpe.h"
#include "mi_print.h"
//#define DBG_3DNR(fmt, args...) DBG_INFO(fmt, ##args)
#define DBG_3DNR(fmt, args...)

#define MI_VPE_3DNR_MAX_WAITING_NUM          (32)

void mi_vpe_impl_3DnrUpdateSwitchChannel(mi_vpe_DevInfo_t *pstDevInfo, struct list_head *pListHead, struct semaphore *pSemphore)
{
    //MI_BOOL bFind = FALSE;
    mi_vpe_ChannelInfo_t *pstChInfo = NULL;
    //struct list_head *pos, *n;

    if ((E_MI_VPE_3DNR_STATUS_IDLE == pstDevInfo->st3DNRUpdate.eStatus)
        || (E_MI_VPE_3DNR_STATUS_UPDATED == pstDevInfo->st3DNRUpdate.eStatus)
        || ((E_MI_VPE_3DNR_STATUS_NEED_UPDATE == pstDevInfo->st3DNRUpdate.eStatus) // wait timeout
            && (pstDevInfo->st3DNRUpdate.u32WaitScriptNum < MI_VPE_3DNR_MAX_WAITING_NUM))
        )
    {
        down(pSemphore);
        #if 0
        list_for_each_safe(pos, n, pListHead)
        {
            pstChInfo = container_of(pos, mi_vpe_ChannelInfo_t, list);
            if (pstChInfo->VpeCh == pstDevInfo->st3DNRUpdate.VpeCh)
            {
                pstChInfo = container_of(n, mi_vpe_ChannelInfo_t, list)
                bFind = TRUE;
                list_del(&pstChInfo->list);
                list_add_tail(&pstChInfo->list, pListHead);
                break;
            }
        }
        #else
        if (list_empty(pListHead))
        {
            up(pSemphore);
            return;
        }

        pstChInfo = container_of(pListHead->next, mi_vpe_ChannelInfo_t, list);
        list_del(&pstChInfo->list);
        list_add_tail(&pstChInfo->list, pListHead);

        #endif
        up(pSemphore);
#if 0
        if (bFind == FALSE) // Can not find channel, just start from list head.
        {
            pstChInfo = container_of((pListHead->next), mi_vpe_ChannelInfo_t, list);
        }
#endif
        pstDevInfo->st3DNRUpdate.VpeCh = pstChInfo->VpeCh;
        pstDevInfo->st3DNRUpdate.eStatus = E_MI_VPE_3DNR_STATUS_NEED_UPDATE;
        pstDevInfo->st3DNRUpdate.u32WaitScriptNum = 0;
        DBG_3DNR("Got Channel %d to do 3DNR E_MI_VPE_3DNR_STATUS_NEED_UPDATE.\n", pstDevInfo->st3DNRUpdate.VpeCh);
    }
    else
    {
        DBG_3DNR("Got Channel %d to do 3DNR pstDevInfo->st3DNRUpdate.eStatus:%d.\n", pstDevInfo->st3DNRUpdate.VpeCh, pstDevInfo->st3DNRUpdate.eStatus);
    }

}

MI_BOOL mi_vpe_impl_3DnrUpdateProcessStart(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t * pstChnnInfo, mi_sys_ChnTaskInfo_t *pstTask)
{
    MI_BOOL bNeed3DnrUpdate = FALSE;
    if ((pstDevInfo->st3DNRUpdate.eStatus == E_MI_VPE_3DNR_STATUS_NEED_UPDATE) && (pstDevInfo->st3DNRUpdate.VpeCh == pstTask->u32ChnId))
    {
        pstDevInfo->st3DNRUpdate.eStatus = E_MI_VPE_3DNR_STATUS_RUNNING;
        bNeed3DnrUpdate = TRUE;
        // Register release for 3DNR

        DBG_3DNR("pstChTask: %p u32Reserved1: %x.\n", pstTask, pstTask->u32Reserved1);
        pstTask->u32Reserved1 |= MI_VPE_TASK_3DNR_UPDATE;
        DBG_3DNR("pstChTask: %p u32Reserved1: %x.\n", pstTask, pstTask->u32Reserved1);


        bNeed3DnrUpdate = MHalVpeIqSetDnrTblMask(pstChnnInfo->pIqCtx, FALSE, pstDevInfo->pstCmdMloadInfo);
        DBG_3DNR("Start Channel %d to do 3DNR E_MI_VPE_3DNR_STATUS_RUNNING disable mask.\n", pstDevInfo->st3DNRUpdate.VpeCh);
    }
    return bNeed3DnrUpdate;
}

void mi_vpe_impl_3DnrUpdateProcessEnd(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo)
{
    // Mask 3D NR register update.

    DBG_3DNR("Channel %d 3DNR Enable Mask.\n", pstDevInfo->st3DNRUpdate.VpeCh);
    MHalVpeIqSetDnrTblMask(pstChnnInfo->pIqCtx, TRUE, pstDevInfo->pstCmdMloadInfo);
}

void mi_vpe_impl_3DnrUpdateProcessFinish(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo)
{
    // HAL read 3D NR register
    MHalVpeIqRead3DNRTbl(pstChnnInfo->pIqCtx);
    // update 3D NR finish
    pstDevInfo->st3DNRUpdate.eStatus = E_MI_VPE_3DNR_STATUS_UPDATED;
    DBG_3DNR("Channel %d 3DNR read Tbl E_MI_VPE_3DNR_STATUS_UPDATED.\n", pstDevInfo->st3DNRUpdate.VpeCh);
}
