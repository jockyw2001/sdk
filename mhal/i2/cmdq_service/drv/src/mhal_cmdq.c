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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#if 0
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/list.h>
#endif

#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#include "cam_os_util_list.h"
#include "drv_cmdq_os.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "drv_cmdq.h"
#include "hal_cmdq_dbg.h"
#include "hal_cmdq_util.h"
#include "cmdq_proc.h"

#if defined(HAL_K6_SIMULATE) || defined(HAL_I3_SIMULATE)
extern void MDrvCmdqAllocateBuffer(void *pPriv);
extern void MDrvCmdqReleaseBuffer(void *pPriv);
#endif

#if defined(HAL_I2_SIMULATE) || defined(CAM_OS_RTK)
extern void MDrvCmdqAllocateBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo);
extern void MDrvCmdqReleaseBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo);
#endif

typedef struct MHAL_CMDQ_AlloBufItem_s
{
    MS_PHYADDR                  phyCmdqBufItemPaddr;
    MS_PHYADDR                  phyCmdqBufItemVaddr;
    MS_U32                      u32CmdqBufSize;
    MS_U32                      bUsed;
    struct CamOsListHead_t      sItemListHead;
    MHAL_CMDQ_Id_e              eCmdqBufItemID;
} MHAL_CMDQ_AlloBufItem_t;

typedef struct MHAL_CMDQ_AllocateBufferHead_s
{
    MS_PHYADDR                  phyCmdqBufPhyAddr;
    MS_PHYADDR                  phyCmdqBufVirAddr;
    MS_U32                      u32CmdqBufSize;
    MS_U32                      u32CmdqBufAvailSize;
    MS_PHYADDR                  phyMloadBufPhyAddr;
    MS_PHYADDR                  phyMloadBufVirAddr;
    MS_U32                      u32MloadBufSize;
    MS_U32                      u32ListAllocCnt;
    struct CamOsListHead_t      sMemAllocListHead;
    CamOsMemCache_t             sMemAllocCache;
    CamOsMutex_t                tBufMutex;
} MHAL_CMDQ_AllocateBufferHead_t;

MHAL_CMDQ_AllocateBufferHead_t  gCmdBufferHead;

MS_S32  MHAL_CMDQ_GetNextMloadRingBufWritePtr(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_PHYADDR* phyWritePtr);
MS_S32  MHAL_CMDQ_UpdateMloadRingBufReadPtr(MHAL_CMDQ_CmdqInterface_t* pCmdinf, MS_PHYADDR phyReadPtr);
MS_S32  MHAL_CMDQ_MloadCopyBuf(MHAL_CMDQ_CmdqInterface_t *pCmdinf, void * MloadBuf, MS_U32 u32Size, MS_U16 u16Alignment, MS_PHYADDR *phyRetAddr);
MS_S32  MHAL_CMDQ_CheckBufAvailable(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32CmdqNum);
MS_S32  MHAL_CMDQ_WriteDummyRegCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16 u16Value);
MS_S32  MHAL_CMDQ_ReadDummyRegCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16* u32RegVal);
MS_S32  MHAL_CMDQ_WriteRegCmdqMask(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value, MS_U16 u16WriteMask);
MS_U32  MHAL_CMDQ_WriteRegCmdqMaskMulti(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_MultiCmdBufMask_t *u32MultiCmdBufMask, MS_U16 u16Size);
MS_S32  MHAL_CMDQ_WriteRegCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value);
MS_U32  MHAL_CMDQ_WriteRegCmdqMulti(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_MultiCmdBuf_t *u32MultiCmdBuf, MS_U16 u16Size);
MS_S32  MHAL_CMDQ_CmdqPollRegBits(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value,  MS_U16 u16WriteMask, MS_BOOL bPollEq);
MS_S32  MHAL_CMDQ_CmdqAddWaitEventCmd(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_EventId_e eEvent);
MS_S32  MHAL_CMDQ_CmdqAbortBuffer(MHAL_CMDQ_CmdqInterface_t *pCmdinf);
MS_S32  MHAL_CMDQ_CmdqResetEngine(MHAL_CMDQ_CmdqInterface_t *pCmdinf);
MS_S32  MHAL_CMDQ_ReadStatusCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32* u32StatVal);
MS_S32  MHAL_CMDQ_KickOffCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf);
MS_S32  MHAL_CMDQ_IsCmdqEmptyIdle(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_BOOL* bIdleVal);
MS_S32  MHAL_CMDQ_ClearTriggerEvent(MHAL_CMDQ_CmdqInterface_t *pCmdinf);
MS_S32  MHAL_CMDQ_KickOffCmdqByTag(MHAL_CMDQ_CmdqInterface_t *pCmdinf,MS_U16 u16Tag);
MS_S32  MHAL_CMDQ_CmdDelay(MHAL_CMDQ_CmdqInterface_t *pCmdinf,MS_U32 uTimeNs);
MS_U32  MHAL_CMDQ_GetCurrentCmdqNumber(MHAL_CMDQ_CmdqInterface_t *pCmdinf);
MS_S32  MHAL_CMDQ_CmdqPollRegBits_ByTime(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value,  MS_U16 u16WriteMask, MS_BOOL bPollEq,MS_U32 uTimeNs);

static unsigned int   gCmdqInitDone = 0;
static unsigned int   gCmdqDeinitDone = 0;
static unsigned int   gCmdqBufferInitDone = 0;

MHAL_CMDQ_CmdqInterface_t   gCmdqOps[E_MHAL_CMDQ_ID_MAX];

#if DRVCMD_NO_KO_MODULE
MHAL_CMDQ_Mmap_Info_t gCmdqTestMmapInfoNoKO;
#endif
static unsigned int         gInitBufMutexDone = 0;
CamOsMutex_t                gInitBufMutex;

#if !defined(HAL_K6_SIMULATE)&&!defined(HAL_I3_SIMULATE)
//=============================================================================
// Description:
// _MHAL_CMDQ_CheckToReleaseAllBuffer
// Author:
// Kevin Cheng
// Input:
// Output:
//=============================================================================
void _MHAL_CMDQ_PrintBufferCnt(void)
{
    MS_U32 u32Cnt = 0;
    MHAL_CMDQ_AlloBufItem_t *pCmdqItemPos, *pCmdqItemNxt;
    CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(pCmdqItemPos, pCmdqItemNxt, &gCmdBufferHead.sMemAllocListHead, sItemListHead)
    {
        CMDQ_LOG("alloc buff ctx (0x%x-0x%x-0x%x-0x%x-0x%x-0x%x)\n",(u32)pCmdqItemPos,(u32)pCmdqItemPos->bUsed
               ,(u32)pCmdqItemPos->eCmdqBufItemID,(u32)pCmdqItemPos->phyCmdqBufItemPaddr
               ,(u32)pCmdqItemPos->phyCmdqBufItemVaddr,(u32)pCmdqItemPos->u32CmdqBufSize);
        u32Cnt++;
    }
    CMDQ_LOG("buffer count =%d\n",u32Cnt);
}

//=============================================================================
// Description:
// _MHAL_CMDQ_CheckToReleaseAllBuffer
// Author:
// Kevin Cheng
// Input:
// Output:
//=============================================================================
void _MHAL_CMDQ_CheckToReleaseAllBuffer(void)
{
    MS_U32 u32Cnt = 0;
    MHAL_CMDQ_AlloBufItem_t *pCmdqItemPos, *pCmdqItemNxt,*pCmdqItemPosOnly;

    CamOsMutexLock(&gCmdBufferHead.tBufMutex);
    CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(pCmdqItemPos, pCmdqItemNxt, &gCmdBufferHead.sMemAllocListHead, sItemListHead)
    {
        pCmdqItemPosOnly = pCmdqItemPos;
        u32Cnt++;
    }
    CamOsMutexUnlock(&gCmdBufferHead.tBufMutex);
    /*only one*/
    if(u32Cnt == 1)
    {
        pCmdqItemPosOnly = CAM_OS_LIST_FIRST_ENTRY(&gCmdBufferHead.sMemAllocListHead, MHAL_CMDQ_AlloBufItem_t, sItemListHead);
        gCmdqInitDone = 0;
        gCmdqDeinitDone = 1;
        gCmdBufferHead.u32ListAllocCnt--;
        CamOsMemCacheFree(&gCmdBufferHead.sMemAllocCache, pCmdqItemPosOnly);
        CamOsMemCacheDestroy(&gCmdBufferHead.sMemAllocCache);
        CamOsMutexDestroy(&gCmdBufferHead.tBufMutex);
#if DRVCMD_NO_KO_MODULE
        MDrvCmdqReleaseBuffer(&gCmdqTestMmapInfoNoKO);
#endif
        CmdqProcDeInit();
        CMDQ_LOG("ReleaseAllBuffer count =%d\n",gCmdBufferHead.u32ListAllocCnt);

    }
}

//=============================================================================
// Description:
// _MHAL_CMDQ_SplitBufferArea
// Author:
// Kevin Cheng
// Input:
// Output:
//=============================================================================
MS_S32 _MHAL_CMDQ_SplitBufferArea(MHAL_CMDQ_AlloBufItem_t* ptBestCfg, MS_U32 u32AlloSz, MHAL_CMDQ_Id_e eCmdqId)
{
    MHAL_CMDQ_AlloBufItem_t* pAllocItem;
    MS_U32                   u32RemindSz;

    if(eCmdqId >= E_MHAL_CMDQ_ID_MAX || ptBestCfg == NULL)
    {
        CMDQ_ERR("%s param error (0x%x-%d)!!\n", __func__, (MS_U32)ptBestCfg, eCmdqId);
        return DRVCMDQ_INVALID_PARAM;
    }

    if(ptBestCfg->u32CmdqBufSize > u32AlloSz)
    {
        //_MHAL_CMDQ_PrintBufferCnt();
        pAllocItem = (MHAL_CMDQ_AlloBufItem_t*)CamOsMemCacheAlloc(&gCmdBufferHead.sMemAllocCache);

        if(pAllocItem == NULL)
        {
            CMDQ_ERR("%s No kmem_cache (%d)!!\n", __func__, eCmdqId);
            return DRVCMDQ_FAIL;
        }
        //CMDQ_LOG("pAllocItem=0x%x-0x%x\n",(u32)pAllocItem,(u32)ptBestCfg);
        u32RemindSz = ptBestCfg->u32CmdqBufSize - u32AlloSz;
        ptBestCfg->u32CmdqBufSize = u32AlloSz;
        ptBestCfg->bUsed          = 1;
        ptBestCfg->eCmdqBufItemID = eCmdqId;
        gCmdBufferHead.u32ListAllocCnt++;

        //memset(pAllocItem, 0x0, sizeof(MHAL_CMDQ_AlloBufItem_t));
        CAM_OS_INIT_LIST_HEAD(&pAllocItem->sItemListHead);

        pAllocItem->eCmdqBufItemID = E_MHAL_CMDQ_ID_MAX;
        pAllocItem->bUsed          = 0;
        pAllocItem->u32CmdqBufSize = u32RemindSz;

        pAllocItem->phyCmdqBufItemPaddr = (ptBestCfg->phyCmdqBufItemPaddr + u32AlloSz);
        pAllocItem->phyCmdqBufItemVaddr = (ptBestCfg->phyCmdqBufItemVaddr + u32AlloSz);

        //_MHAL_CMDQ_PrintBufferCnt();
        CAM_OS_LIST_ADD_TAIL(&pAllocItem->sItemListHead,&gCmdBufferHead.sMemAllocListHead);
        //_MHAL_CMDQ_PrintBufferCnt();
    }
    else
    {

        if(ptBestCfg->u32CmdqBufSize == u32AlloSz)
        {
            ptBestCfg->bUsed          = 1;
            return DRVCMDQ_OK;
        }
        else
        {
            CMDQ_ERR("%s remind buffer(%d) <= alloc sz (%d)!!\n", __func__, ptBestCfg->u32CmdqBufSize, u32AlloSz);
            return DRVCMDQ_FAIL;
        }

    }

    return DRVCMDQ_OK;
}
//=============================================================================
// Description:
// _MHAL_CMDQ_GetBestCmdqBuffer
// Author:
// Kevin Cheng
// Input:
// Output:
//=============================================================================
MHAL_CMDQ_AlloBufItem_t * _MHAL_CMDQ_GetBufferAreaByIpId(MHAL_CMDQ_Id_e eCmdqId)
{
    int bfound = 0;
    MHAL_CMDQ_AlloBufItem_t *pCmdqItemPos, *pCmdqItemNxt;

    CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(pCmdqItemPos, pCmdqItemNxt, &gCmdBufferHead.sMemAllocListHead, sItemListHead)
    {
        //CMDQ_LOG("alloc buf(0x%x-0x%x-0x%x)\n",pCmdqItemPos->bUsed,pCmdqItemPos->eCmdqBufItemID,eCmdqId);
        if(pCmdqItemPos->bUsed && pCmdqItemPos->eCmdqBufItemID == eCmdqId)
        {
            bfound = 1;
            break;
        }
    }

    if(!bfound)
    {
        CMDQ_ERR("%s CMDQ Buffer not found By ID(%d)!!\n", __func__, eCmdqId);
        return NULL;
    }

    return pCmdqItemPos;
}

//=============================================================================
// Description:
// _MHAL_CMDQ_GetBestCmdqBuffer
// Author:
// Kevin Cheng
// Input:
// Output:
//=============================================================================
MHAL_CMDQ_AlloBufItem_t * _MHAL_CMDQ_GetBestCmdqBuffer(MHAL_CMDQ_Id_e eCmdqId, MS_U32 u32AllocSz)
{
    int                     bfound = 0;
    MHAL_CMDQ_AlloBufItem_t *pCmdqItemPos, *pCmdqItemNxt;
    MHAL_CMDQ_AlloBufItem_t *pBestCfg = NULL;

    CamOsMutexLock(&gCmdBufferHead.tBufMutex);
    CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(pCmdqItemPos, pCmdqItemNxt, &gCmdBufferHead.sMemAllocListHead, sItemListHead)
    {
        if(pCmdqItemPos->bUsed == 0 && u32AllocSz <= pCmdqItemPos->u32CmdqBufSize
                && (pBestCfg == NULL || (pBestCfg->u32CmdqBufSize > pCmdqItemPos->u32CmdqBufSize)))
        {
            pBestCfg = pCmdqItemPos;
            bfound = 1;
            if(pBestCfg->u32CmdqBufSize == u32AllocSz)
                break;
        }
    }
    if(!bfound)
        goto CMDQ_MM_NOT_FOUND;
    if(_MHAL_CMDQ_SplitBufferArea(pBestCfg, u32AllocSz, eCmdqId) != DRVCMDQ_OK)
    {
        CMDQ_ERR("MHAL_CMDQ_SplitBufferArea fail\n");
        pBestCfg->bUsed = 0;
        goto CMDQ_MM_NOT_FOUND;
    }
    //_MHAL_CMDQ_PrintBufferCnt();
    CMDQ_LOG("[CMDQ]Get best memory(0x%x-0x%x-0x%x-0x%x-0x%x)\n",(u32)pBestCfg->phyCmdqBufItemPaddr
            ,(u32)pBestCfg->phyCmdqBufItemVaddr,(u32)pBestCfg->u32CmdqBufSize
            ,(u32)pBestCfg->eCmdqBufItemID,(u32)pBestCfg->bUsed);
    CamOsMutexUnlock(&gCmdBufferHead.tBufMutex);
    return pBestCfg;

CMDQ_MM_NOT_FOUND:
    CamOsMutexUnlock(&gCmdBufferHead.tBufMutex);
    CMDQ_ERR("%s CMDQ Buffer not found(%d-%d)!!\n", __func__, eCmdqId, u32AllocSz);
    return NULL;
}
//=============================================================================
// Description:
// _MHAL_CMDQ_InitCmdqBuffer
// Author:
// Kevin Cheng
// Input:
// Output:
//=============================================================================
void _MHAL_CMDQ_InitCmdqBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo)
{
    MHAL_CMDQ_AlloBufItem_t  * pAllocBuf;

    memset(&gCmdBufferHead, 0x0, sizeof(MHAL_CMDQ_AllocateBufferHead_t));
    CAM_OS_INIT_LIST_HEAD(&gCmdBufferHead.sMemAllocListHead);
    CamOsMemCacheCreate(&gCmdBufferHead.sMemAllocCache,"cmdq_pool", sizeof(MHAL_CMDQ_AlloBufItem_t), 0);
    /*This place to get command queue buffer sz& and menuload sz& address*/
    /*TBD here*/
    gCmdBufferHead.phyCmdqBufPhyAddr  = pCmdqMmapInfo->u32CmdqMmapPhyAddr;
    gCmdBufferHead.phyCmdqBufVirAddr  = pCmdqMmapInfo->u32CmdqMmapVirAddr ;
    gCmdBufferHead.u32CmdqBufSize     = pCmdqMmapInfo->u32CmdqMmapSize ;

    gCmdBufferHead.phyMloadBufPhyAddr = pCmdqMmapInfo->u32MloadMmapPhyAddr;
    gCmdBufferHead.phyMloadBufVirAddr = pCmdqMmapInfo->u32MloadMmapVirAddr ;
    gCmdBufferHead.u32MloadBufSize    = pCmdqMmapInfo->u32MloadMmapSize;

    pAllocBuf = (MHAL_CMDQ_AlloBufItem_t*)CamOsMemCacheAlloc(&gCmdBufferHead.sMemAllocCache);
    if(pAllocBuf == NULL)
    {
         CMDQ_ERR("%s No kmem_cache!!\n", __func__);
         return;
    }
    gCmdBufferHead.u32ListAllocCnt++;

    pAllocBuf->bUsed               = 0;
    pAllocBuf->eCmdqBufItemID      = E_MHAL_CMDQ_ID_MAX;
    pAllocBuf->phyCmdqBufItemPaddr = gCmdBufferHead.phyCmdqBufPhyAddr;
    pAllocBuf->phyCmdqBufItemVaddr = gCmdBufferHead.phyCmdqBufVirAddr;
    pAllocBuf->u32CmdqBufSize      = gCmdBufferHead.u32CmdqBufSize;
    CAM_OS_INIT_LIST_HEAD(&pAllocBuf->sItemListHead);
    CamOsMutexInit(&gCmdBufferHead.tBufMutex);
    CAM_OS_LIST_ADD_TAIL(&pAllocBuf->sItemListHead,&gCmdBufferHead.sMemAllocListHead);
    CMDQ_LOG("[CMDQ]init buffer cmdq(0x%x-0x%x-0x%x)\n",(MS_U32)gCmdBufferHead.phyCmdqBufPhyAddr
              ,(MS_U32)gCmdBufferHead.phyCmdqBufVirAddr,(MS_U32)gCmdBufferHead.u32CmdqBufSize);
    CMDQ_LOG("[CMDQ]init buffer mload(0x%x-0x%x-0x%x)\n",(MS_U32)gCmdBufferHead.phyMloadBufPhyAddr
              ,(MS_U32)gCmdBufferHead.phyMloadBufVirAddr,(MS_U32)gCmdBufferHead.u32MloadBufSize);

}
#endif
//=============================================================================
// Description:
// Trans cmdq service id to drv id
// Author:
// Kevin Cheng
// Input:
// MHAL_CMDQ_Id_e cmdqId
// Output:
// DrvCcmdqId_e : drv id
//=============================================================================
DrvCcmdqId_e _MHAL_CMDQ_CmdqServiceTransToDrvCmdqId(MHAL_CMDQ_Id_e cmdqId)
{
    switch(cmdqId)
    {
        case E_MHAL_CMDQ_ID_VPE:
            return DRVCMDQ_ID_VPE;
        case E_MHAL_CMDQ_ID_DIVP:
            return DRVCMDQ_ID_DIVP;
        case E_MHAL_CMDQ_ID_H265_VENC0:
            return DRVCMDQ_ID_H265_VENC0;
        case E_MHAL_CMDQ_ID_H265_VENC1:
            return DRVCMDQ_ID_H265_VENC1;
        case E_MHAL_CMDQ_ID_H264_VENC0:
            return DRVCMDQ_ID_H264_VENC0;
        default:
            break;
    }

    CMDQ_ERR("Can't translate cmdqId(%d)\n", cmdqId);
    return DRVCMDQ_ID_VPE;
}
//=============================================================================
// Description:
// MHAL_CMDQ_GetSysCmdqService
// Author:
// Kevin Cheng
// Input:
// MI_CMDQ_ID : cmdq id
// MS_BOOL : true is RIU , false is ringbuffer
// Output:
// cmd_mload_interface POINT  : success
// NULL : fail
//=============================================================================
MS_S32 _MHAL_CMDQ_CmdqServiceAllocBuf(CmdqInterfacePriv_t *pCmdqPriv, MHAL_CMDQ_BufDescript_t *pCmdqBufDesp)
{
#if defined(HAL_K6_SIMULATE)||defined(HAL_I3_SIMULATE)
    MDrvCmdqAllocateBuffer((void*)pCmdqPriv);
    return 0;
#else
    MHAL_CMDQ_AlloBufItem_t * pAllocBufItem;
    MS_U32                    nTmpBufSize;

    if(pCmdqBufDesp->u32CmdqBufSizeAlign == 0)
       pCmdqBufDesp->u32CmdqBufSizeAlign = DVRCMDQ_COMMAND_SIZE_ALIGN;

    nTmpBufSize = CMDQ_DO_SIZE_ALIGN(pCmdqBufDesp->u32CmdqBufSize, pCmdqBufDesp->u32CmdqBufSizeAlign);
    if(!nTmpBufSize)
    {
        CMDQ_ERR("[CMDQ]%d Allocate buffer zero \n"
                      ,(u32)pCmdqPriv->nCmdqDrvIpId);
        return -1;
    }

    pAllocBufItem = _MHAL_CMDQ_GetBestCmdqBuffer(pCmdqPriv->CmdqIpId, nTmpBufSize);
    if(!pAllocBufItem)
    {
        CMDQ_ERR("[CMDQ]%d can't get best buffer\n"
                      ,(u32)pCmdqPriv->nCmdqDrvIpId);
        return -1;
    }

    pCmdqPriv->tCmdqBuf.BufPhy    = (u32)pAllocBufItem->phyCmdqBufItemPaddr;
    pCmdqPriv->tCmdqBuf.BufVir    = (u32)pAllocBufItem->phyCmdqBufItemVaddr;
    pCmdqPriv->tCmdqBuf.nBufSize  =  nTmpBufSize;

    if(pCmdqBufDesp->u32MloadBufSize != 0)
    {
        if(pCmdqBufDesp->u32MloadBufSize > gCmdBufferHead.u32MloadBufSize)
        {
            CMDQ_ERR("Allocate mload buffer(0x%x) bigger than default(0x%x)\n"
                      ,(u32)pCmdqBufDesp->u32MloadBufSize,(u32)gCmdBufferHead.u32MloadBufSize);
            return -1;
        }

        pCmdqPriv->tMloadBuf.BufPhy    = (u32)gCmdBufferHead.phyMloadBufPhyAddr;
        pCmdqPriv->tMloadBuf.BufVir    = (u32)gCmdBufferHead.phyMloadBufVirAddr;
        pCmdqPriv->tMloadBuf.nBufSize  =  gCmdBufferHead.u32MloadBufSize;
    }
    return 0;
#if 0
    u32        nTmpBufSize;
    MS_U64     pu64PhyAddr;
    VOID *  pVirAddr;

    if(pCmdqBufDesp->uCmdqBufSize != 0)
    {
        nTmpBufSize = CMDQ_DO_SIZE_ALIGN(pCmdqBufDesp->uCmdqBufSize, pCmdqBufDesp->uCmdqBufSizeAlign);

        if(MI_MMA_Alloc(DRVCMDQ_NAME, nTmpBufSize, &pu64PhyAddr) < 0)
        {
            //CMDQ_ERR("allocate cmdq Buffer fail\n");
            panic("allocate cmdq Buffer fail\n");
        }

        if((pVirAddr = MI_SYS_Mmap(pu64PhyAddr, nTmpBufSize, 0)) == 0)
        {
            //CMDQ_ERR("mmap vir fail(0x%x)\n",(u32)pu64PhyAddr);
            panic("mmap vir fail(0x%x)\n");
        }
        pCmdqPriv->tCmdqBuf.BufPhy    = (u8*)((u32)pu64PhyAddr);
        pCmdqPriv->tCmdqBuf.BufVir    = (u8*)((u32)pVirAddr);
        pCmdqPriv->tCmdqBuf.nBufSize  =  nTmpBufSize;
    }

    if(pCmdqBufDesp->uMloadBufSize != 0)
    {
        nTmpBufSize = CMDQ_DO_SIZE_ALIGN(pCmdqBufDesp->uMloadBufSize, pCmdqBufDesp->uMloadBufSizeAlign);

        if(MI_MMA_Alloc(DRVCMDQ_NAME, nTmpBufSize, &pu64PhyAddr) < 0)
        {
            //CMDQ_ERR("allocate mload Buffer fail\n");
            panic("allocate mload Buffer fail\n");
        }

        if((pVirAddr = MI_SYS_Mmap(pu64PhyAddr, nTmpBufSize, 0)) == 0)
        {
            //CMDQ_ERR("%s mmap mload vir fail(0x%x)\n",(u32)pu64PhyAddr);
            panic("mmap mload vir fail(0x%x)\n", (u32)pu64PhyAddr);
        }
        pCmdqPriv->tMloadBuf.BufPhy    = (u8*)((u32)pu64PhyAddr);
        pCmdqPriv->tMloadBuf.BufVir    = (u8*)((u32)pVirAddr);
        pCmdqPriv->tMloadBuf.nBufSize  =  nTmpBufSize;
    }
#endif
    return 0;
#endif
}
//=============================================================================
// Description:
// _MHAL_CMDQ_ReleaseBufferArea
// Author:
// Kevin Cheng
// Input:
// MI_CMDQ_ID : cmdq id
// MS_BOOL : true is RIU , false is ringbuffer
// Output:
// cmd_mload_interface POINT  : success
// NULL : fail
//=============================================================================
MS_S32 _MHAL_CMDQ_ReleaseBufferArea(CmdqInterfacePriv_t *pCmdqPriv)
{
    int bfound = 0;
    MHAL_CMDQ_AlloBufItem_t *pCmdqItemPos, *pCmdqItemNxt;
    MHAL_CMDQ_AlloBufItem_t *pMergeCfg = NULL, *pPreCfg = NULL, *pNxtCfg = NULL;

    CamOsMutexLock(&gCmdBufferHead.tBufMutex);
    pMergeCfg = _MHAL_CMDQ_GetBufferAreaByIpId(pCmdqPriv->CmdqIpId);

    if(pMergeCfg == NULL)
    {
        CamOsMutexUnlock(&gCmdBufferHead.tBufMutex);
        CMDQ_ERR("%s err!!!(%d)!!\n", __func__, pCmdqPriv->CmdqIpId);
        return DRVCMDQ_FAIL;
    }

    CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(pCmdqItemPos, pCmdqItemNxt, &gCmdBufferHead.sMemAllocListHead, sItemListHead)
    {

        if(pCmdqItemPos->bUsed == 0 && ((u32)pCmdqItemPos->phyCmdqBufItemPaddr + pCmdqItemPos->u32CmdqBufSize) == (u32)pMergeCfg->phyCmdqBufItemPaddr)
        {
            pPreCfg = pCmdqItemPos;
        }

        if(pCmdqItemPos->bUsed == 0  && ((u32)pMergeCfg->phyCmdqBufItemPaddr + pMergeCfg->u32CmdqBufSize) == (u32)pCmdqItemPos->phyCmdqBufItemPaddr)
        {
            pNxtCfg = pCmdqItemPos;
        }

        if(pCmdqItemPos == pMergeCfg)
            bfound = 1;
    }

    if(!bfound)
    {
        CamOsMutexUnlock(&gCmdBufferHead.tBufMutex);
        CMDQ_ERR("%s err not found!!!(%d)!!\n", __func__, pCmdqPriv->CmdqIpId);
        return DRVCMDQ_FAIL;
    }


    if(pPreCfg)
    {
        pPreCfg->u32CmdqBufSize += pMergeCfg->u32CmdqBufSize;
        CAM_OS_LIST_DEL(&pMergeCfg->sItemListHead);
        gCmdBufferHead.u32ListAllocCnt--;

        CamOsMemCacheFree(&gCmdBufferHead.sMemAllocCache, pMergeCfg);

        if(pNxtCfg)
        {
            pPreCfg->u32CmdqBufSize += pNxtCfg->u32CmdqBufSize;
            CAM_OS_LIST_DEL(&pNxtCfg->sItemListHead);
            gCmdBufferHead.u32ListAllocCnt--;
            CamOsMemCacheFree(&gCmdBufferHead.sMemAllocCache, pNxtCfg);
        }
    }
    else
    {
        if(pNxtCfg)
        {
            pMergeCfg->u32CmdqBufSize += pNxtCfg->u32CmdqBufSize;
            CAM_OS_LIST_DEL(&pNxtCfg->sItemListHead);
            gCmdBufferHead.u32ListAllocCnt--;
            pMergeCfg->bUsed = 0;
            CamOsMemCacheFree(&gCmdBufferHead.sMemAllocCache, pNxtCfg);
        }
        else
        {
            pMergeCfg->bUsed = 0;
        }
    }
    CamOsMutexUnlock(&gCmdBufferHead.tBufMutex);
    /**/
    _MHAL_CMDQ_CheckToReleaseAllBuffer();

    return DRVCMDQ_OK;
}
//=============================================================================
// Description:
// _MHAL_CMDQ_ReleaseBufferArea
// Author:
// Kevin Cheng
// Input:
// MI_CMDQ_ID : cmdq id
// MS_BOOL : true is RIU , false is ringbuffer
// Output:
// cmd_mload_interface POINT  : success
// NULL : fail
//=============================================================================
void _MHAL_CMDQ_CmdqServiceReleaseBuf(CmdqInterfacePriv_t *pCmdqPriv)
{
#if defined(HAL_K6_SIMULATE)||defined(HAL_I3_SIMULATE)
    MDrvCmdqReleaseBuffer((void*)pCmdqPriv);
#else
    _MHAL_CMDQ_ReleaseBufferArea(pCmdqPriv);
#if 0
    if(pCmdqPriv->tCmdqBuf.nBufSize != 0)
    {
        MI_SYS_Munmap(pCmdqPriv->tCmdqBuf.BufVir, pCmdqPriv->tCmdqBuf.nBufSize);
        MI_MMA_Free(pCmdqPriv->tCmdqBuf.BufPhy);
    }

    if(pCmdqPriv->tMloadBuf.nBufSize != 0)
    {
        MI_SYS_Munmap(pCmdqPriv->tMloadBuf.BufVir, pCmdqPriv->tMloadBuf.nBufSize);
        MI_MMA_Free(pCmdqPriv->tMloadBuf.BufPhy);
    }
#endif
#endif
}
//=============================================================================
// Description:
// Init CMDQ mmap info
// Author:
// Kevin Cheng
// Input:
// MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo
// Output:
// MHAL_SUCCESS : is ok
// MHAL_FAILURE : is failed
//=============================================================================
MS_S32 MHAL_CMDQ_InitCmdqMmapInfo(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo)
{
#if !defined(HAL_K6_SIMULATE)&&!defined(HAL_I3_SIMULATE)
    if(!gCmdqBufferInitDone)
    {
         gCmdqBufferInitDone =1;
         _MHAL_CMDQ_InitCmdqBuffer(pCmdqMmapInfo);
    }
#endif
    return MHAL_SUCCESS;
}

//=============================================================================
// Description:
// MHAL_CMDQ_GetSysCmdqService
// Author:
// Kevin Cheng
// Input:
// MI_CMDQ_ID : cmdq id
// MS_BOOL : true is RIU , false is ringbuffer
// Output:
// cmd_mload_interface POINT  : success
// NULL : fail
//=============================================================================
MHAL_CMDQ_CmdqInterface_t *MHAL_CMDQ_GetSysCmdqService(MHAL_CMDQ_Id_e eCmdqId, MHAL_CMDQ_BufDescript_t *pCmdqBufDesp, MS_BOOL bForceRIU)
{
    int i;
    DrvCcmdqId_e         nDrvID ;
    CmdqInterfacePriv_t *pCmdqPriv;
#if !DRVCMD_NO_KO_MODULE
    MHAL_CMDQ_Mmap_Info_t tCmdqMmapInfo;
#endif

#if !defined(HAL_I2_SIMULATE) && !defined(CAM_OS_RTK)
    if(!gCmdqBufferInitDone)
    {
        CMDQ_ERR("%s No Init CMDQ MMap (%d)\n", __func__, eCmdqId);
        return NULL;
    }
#endif

    if(eCmdqId >= E_MHAL_CMDQ_ID_MAX)
    {
        CMDQ_ERR("%s priv data error(%d)\n", __func__, eCmdqId);
        return NULL;
    }

    if(!gInitBufMutexDone)
    {
        gInitBufMutexDone = 1;
        CamOsMutexInit(&gInitBufMutex);
    }
    CamOsMutexLock(&gInitBufMutex);
    if(!gCmdqInitDone)
    {
        gCmdqInitDone = 1;

        for(i = 0; i < E_MHAL_CMDQ_ID_MAX; i++)
        {
            gCmdqOps[i].MHAL_CMDQ_GetNextMloadRingBufWritePtr    = MHAL_CMDQ_GetNextMloadRingBufWritePtr;
            gCmdqOps[i].MHAL_CMDQ_UpdateMloadRingBufReadPtr      = MHAL_CMDQ_UpdateMloadRingBufReadPtr;
            gCmdqOps[i].MHAL_CMDQ_MloadCopyBuf                   = MHAL_CMDQ_MloadCopyBuf;
            gCmdqOps[i].MHAL_CMDQ_CheckBufAvailable              = MHAL_CMDQ_CheckBufAvailable;
            gCmdqOps[i].MHAL_CMDQ_WriteRegCmdqMask               = MHAL_CMDQ_WriteRegCmdqMask;
            gCmdqOps[i].MHAL_CMDQ_WriteRegCmdqMaskMulti          = MHAL_CMDQ_WriteRegCmdqMaskMulti;
            gCmdqOps[i].MHAL_CMDQ_WriteRegCmdq                   = MHAL_CMDQ_WriteRegCmdq;
            gCmdqOps[i].MHAL_CMDQ_WriteRegCmdqMulti              = MHAL_CMDQ_WriteRegCmdqMulti;
            gCmdqOps[i].MHAL_CMDQ_CmdqPollRegBits                = MHAL_CMDQ_CmdqPollRegBits;
            gCmdqOps[i].MHAL_CMDQ_CmdqAddWaitEventCmd            = MHAL_CMDQ_CmdqAddWaitEventCmd;
            gCmdqOps[i].MHAL_CMDQ_WriteDummyRegCmdq              = MHAL_CMDQ_WriteDummyRegCmdq;
            gCmdqOps[i].MHAL_CMDQ_ReadDummyRegCmdq               = MHAL_CMDQ_ReadDummyRegCmdq;
            gCmdqOps[i].MHAL_CMDQ_KickOffCmdq                    = MHAL_CMDQ_KickOffCmdq;
            gCmdqOps[i].MHAL_CMDQ_IsCmdqEmptyIdle                = MHAL_CMDQ_IsCmdqEmptyIdle;
            gCmdqOps[i].MHAL_CMDQ_CmdqAbortBuffer                = MHAL_CMDQ_CmdqAbortBuffer;
            gCmdqOps[i].MHAL_CMDQ_ReadStatusCmdq                 = MHAL_CMDQ_ReadStatusCmdq;
            gCmdqOps[i].MHAL_CMDQ_ClearTriggerEvent              = MHAL_CMDQ_ClearTriggerEvent;
            gCmdqOps[i].MHAL_CMDQ_CmdqResetEngine                = MHAL_CMDQ_CmdqResetEngine;
            gCmdqOps[i].MHAL_CMDQ_KickOffCmdqByTag               = MHAL_CMDQ_KickOffCmdqByTag;
            gCmdqOps[i].MHAL_CMDQ_CmdDelay                       = MHAL_CMDQ_CmdDelay;
            gCmdqOps[i].MHAL_CMDQ_GetCurrentCmdqNumber           = MHAL_CMDQ_GetCurrentCmdqNumber;
			gCmdqOps[i].MHAL_CMDQ_CmdqPollRegBits_ByTime         = MHAL_CMDQ_CmdqPollRegBits_ByTime;
            gCmdqOps[i].pCtx                                     = NULL;
        }

#if DRVCMD_NO_KO_MODULE
        memset(&gCmdqTestMmapInfoNoKO,0x0,sizeof(MHAL_CMDQ_Mmap_Info_t));
        MDrvCmdqAllocateBuffer(&gCmdqTestMmapInfoNoKO);
        gCmdqBufferInitDone = 0;
        MHAL_CMDQ_InitCmdqMmapInfo(&gCmdqTestMmapInfoNoKO);
#else
        if(gCmdqDeinitDone)
        {
            tCmdqMmapInfo.u32CmdqMmapPhyAddr     = gCmdBufferHead.phyCmdqBufPhyAddr ;
            tCmdqMmapInfo.u32CmdqMmapVirAddr     = gCmdBufferHead.phyCmdqBufVirAddr ;
            tCmdqMmapInfo.u32CmdqMmapSize        = gCmdBufferHead.u32CmdqBufSize ;

            tCmdqMmapInfo.u32MloadMmapPhyAddr    = gCmdBufferHead.phyMloadBufPhyAddr ;
            tCmdqMmapInfo.u32MloadMmapVirAddr    = gCmdBufferHead.phyMloadBufVirAddr ;
            tCmdqMmapInfo.u32MloadMmapSize       = gCmdBufferHead.u32MloadBufSize;
            gCmdqBufferInitDone = 0;
            CMDQ_LOG("[CMDQ]Reinit buffer cmdq(0x%x-0x%x-0x%x)\n",(MS_U32)gCmdBufferHead.phyCmdqBufPhyAddr
              ,(MS_U32)gCmdBufferHead.phyCmdqBufVirAddr,(MS_U32)gCmdBufferHead.u32CmdqBufSize);
            MHAL_CMDQ_InitCmdqMmapInfo(&tCmdqMmapInfo);
            gCmdqDeinitDone = 0;
        }
#endif
        CmdqProcInit();
    }

    if(gCmdqOps[eCmdqId].pCtx == NULL)
    {
        nDrvID = _MHAL_CMDQ_CmdqServiceTransToDrvCmdqId(eCmdqId);
        pCmdqPriv = MDrvCmdqInit(nDrvID);

        if(pCmdqPriv == NULL)
        {
            CMDQ_ERR("%s (%d) FAIL\n",__func__,nDrvID);
            //panic("MDrvCmdqInit FAIL\n");
        }

        pCmdqPriv->bForceRIU = bForceRIU;
        if(pCmdqBufDesp->u32CmdqBufSize == 0 || pCmdqBufDesp->u32CmdqBufSizeAlign ==0)
        {
            CMDQ_ERR("CMDQID(%d) buf sz(%d),alignSz(%d) err\n",eCmdqId
                      ,pCmdqBufDesp->u32CmdqBufSize,pCmdqBufDesp->u32CmdqBufSizeAlign);
            CamOsMutexUnlock(&gInitBufMutex);
            return NULL;
        }

        if(pCmdqBufDesp->u32MloadBufSize > 0 && eCmdqId != E_MHAL_CMDQ_ID_VPE)
        {
             CMDQ_ERR("Mload buffer only support VPE(%d)\n",eCmdqId);
             pCmdqBufDesp->u32MloadBufSize = 0;
        }
        /*allocate buffer , TBD*/
        if(_MHAL_CMDQ_CmdqServiceAllocBuf(pCmdqPriv, pCmdqBufDesp)<0)
        {
            CMDQ_ERR("%s CmdqServiceAllocBuf fail(%d)\n", __func__, eCmdqId);
            CamOsMutexUnlock(&gInitBufMutex);
            return NULL;
        }
        /**/
        MDrvCmdqSetBufInfo(pCmdqPriv);
        gCmdqOps[eCmdqId].pCtx = (void*)pCmdqPriv;
    }
    else
    {
        CMDQ_ERR("%s double allocate(%d)\n", __func__, eCmdqId);
        CamOsMutexUnlock(&gInitBufMutex);
        return NULL;
    }
    //CMDQ_ERR("[CMDQ]INTF =%p ctx=%p\n", &gCmdqOps[eCmdqId], gCmdqOps[eCmdqId].pCtx);
    CamOsMutexUnlock(&gInitBufMutex);
    return &gCmdqOps[eCmdqId];
}
//=============================================================================
// Description:
// MHAL_CMDQ_ReleaseSysCmdqService
// Author:
// Kevin Cheng
// Input:
// MI_CMDQ_ID cmdqId
// Output:
// void
//=============================================================================
void MHAL_CMDQ_ReleaseSysCmdqService(MHAL_CMDQ_Id_e eCmdqId)
{
    CmdqInterfacePriv_t *pCmdqPriv;
    DrvCcmdqId_e         nDrvID ;

    if(eCmdqId >= E_MHAL_CMDQ_ID_MAX)
    {
        CMDQ_ERR("%s priv data error(%d)\n", __func__, eCmdqId);
        return ;
    }
    CamOsMutexLock(&gInitBufMutex);
    nDrvID = _MHAL_CMDQ_CmdqServiceTransToDrvCmdqId(eCmdqId);
    pCmdqPriv = MDrvGetCmdqPrivateDataByCmdqId(nDrvID);

    if(pCmdqPriv == NULL)
    {
        CMDQ_ERR("%s get priv data fail(%d)\n",__func__,eCmdqId);
        //panic("%s get priv data fail(%d)\n", __func__, eCmdqId);
    }
    _MHAL_CMDQ_CmdqServiceReleaseBuf(pCmdqPriv);
    /*release allocate buffer*/
    MDrvRelesaeCmdqService(pCmdqPriv);
    gCmdqOps[eCmdqId].pCtx = NULL;
    CMDQ_LOG("[CMDQ]Release CMDQ(%d) service\n",eCmdqId);
    CamOsMutexUnlock(&gInitBufMutex);
}
//=============================================================================
// Description:
// To get menuload buffer write point
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_GetNextMloadRingBufWritePtr(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_PHYADDR* phyWritePtr)
{
    MS_PHYADDR phyCmdAddr;

    if(!(phyCmdAddr = MDrvGetMloadRingBufWritePtr(pCmdinf->pCtx)))
        return MHAL_FAILURE;

    *phyWritePtr = phyCmdAddr;
    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// To update menuload buffer read point
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32 MHAL_CMDQ_UpdateMloadRingBufReadPtr(MHAL_CMDQ_CmdqInterface_t* pCmdinf, MS_PHYADDR phyReadPtr)
{
    if(MDrvUpdateMloadRingBufReadPtr(pCmdinf->pCtx, phyReadPtr) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_MloadCopyBuf
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_MloadCopyBuf(MHAL_CMDQ_CmdqInterface_t *pCmdinf, void * MloadBuf, MS_U32 u32Size, MS_U16 u16Alignment, MS_PHYADDR *phyRetAddr)
{
    u32 pPhyAddr;

    if((pPhyAddr = MDrvCmdqMloadCopyBuf(pCmdinf->pCtx, MloadBuf, u32Size, u16Alignment)) == 0)
    {
        *phyRetAddr = pPhyAddr;
        return MHAL_FAILURE;
    }

    *phyRetAddr = pPhyAddr;
    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_CheckBufAvailable
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_CheckBufAvailable(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32CmdqNum)
{
    return MDrvCmdqCheckBufferAvail(pCmdinf->pCtx, u32CmdqNum);
}
//=============================================================================
// Description:
// MHAL_CMDQ_WriteRegCmdqMask
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32 MHAL_CMDQ_WriteRegCmdqMask(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value, MS_U16 u16WriteMask)
{
    if(MDrvCmdqWriteCommandMask(pCmdinf->pCtx, u32RegAddr, u16Value, u16WriteMask) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_WriteRegCmdqMaskMulti
// Author:
// Joe Su
// Input:
// MHAL_CMDQ_CmdqInterface_t *pCmdinf
// MHAL_CMDQ_MultiCmdBuf_t *u32MultiCmdBufMask
// MS_U16 u16Size
// Output:
// Actual size already write to CMDQ buffer
//=============================================================================
MS_U32  MHAL_CMDQ_WriteRegCmdqMaskMulti(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_MultiCmdBufMask_t *u32MultiCmdBufMask, MS_U16 u16Size)
{
    return MDrvCmdqWriteCommandMaskMulti(pCmdinf->pCtx, (void *)u32MultiCmdBufMask, u16Size);
}
//=============================================================================
// Description:
// MHAL_CMDQ_WriteRegCmdq
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_WriteRegCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value)
{
    if(MDrvCmdqWriteCommand(pCmdinf->pCtx, u32RegAddr, u16Value) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_WriteRegCmdqMulti
// Author:
// Joe Su
// Input:
// MHAL_CMDQ_CmdqInterface_t *pCmdinf
// MHAL_CMDQ_MultiCmdBuf_t *u32MultiCmdBuf
// MS_U16 u16Size
// Output:
// Actual size already write to CMDQ buffer
//=============================================================================
MS_U32  MHAL_CMDQ_WriteRegCmdqMulti(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_MultiCmdBuf_t *u32MultiCmdBuf, MS_U16 u16Size)
{
    return MDrvCmdqWriteCommandMulti(pCmdinf->pCtx, (void *)u32MultiCmdBuf, u16Size);
}

//=============================================================================
// Description:
// MHAL_CMDQ_CmdqPollRegBits
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_CmdqPollRegBits(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value,  MS_U16 u16WriteMask, MS_BOOL bPollEq)
{
    if(MDrvCmdqPollEqCommandMask(pCmdinf->pCtx, u32RegAddr, u16Value, u16WriteMask, bPollEq) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_CmdqAddWaitEventCmd
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_CmdqAddWaitEventCmd(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_EventId_e eEvent)
{
    if(MDrvCmdqWaitCommand(pCmdinf->pCtx, eEvent) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_KickOffCmdq
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_KickOffCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    MS_S32 s32TmpVal;
    if((s32TmpVal = MDrvCmdqKickOfftCommand(pCmdinf->pCtx)) < 0)
        return MHAL_FAILURE;

    return s32TmpVal;
}
//=============================================================================
// Description:
// MHAL_CMDQ_KickOffCmdqByTag
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_KickOffCmdqByTag(MHAL_CMDQ_CmdqInterface_t *pCmdinf,MS_U16 u16Tag)
{
    MS_S32 s32TmpVal;
    if((s32TmpVal = MDrvCmdqKickOfftCommandByTag(pCmdinf->pCtx,u16Tag)) < 0)
        return MHAL_FAILURE;

    return s32TmpVal;
}
MS_U32 MHAL_CMDQ_GetCurrentCmdqNumber(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    return MDrvCmdqGetCurrentCmdqNumber(pCmdinf->pCtx);
}

//=============================================================================
// Description:
// MHAL_CMDQ_IsCmdqEmptyIdle
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
// wirte point address : success
// NULL : fail
//=============================================================================
MS_S32  MHAL_CMDQ_IsCmdqEmptyIdle(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_BOOL* bIdleVal)
{
    if(MDrvCmdqIsIdle(pCmdinf->pCtx))
        *bIdleVal = TRUE;
    else
        *bIdleVal = FALSE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_WriteDummyRegCmdq
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_WriteDummyRegCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16 u16Value)
{
    if(MDrvCmdqWriteDummyRegister(pCmdinf->pCtx, u16Value) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_ReadDummyRegCmdq
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_ReadDummyRegCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16* u16RegVal)
{
    *u16RegVal = MDrvCmdqReadDummyRegister(pCmdinf->pCtx);
    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_CmdqAbortBuffer
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_CmdqAbortBuffer(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    if(MDrvAbortCmdqCommand(pCmdinf->pCtx) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_ReadStatusCmdq
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_ReadStatusCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 *u32StatVal)
{
    *u32StatVal = MDrvCmdqReadIrqStatus(pCmdinf->pCtx);
    return MHAL_SUCCESS;
}

//=============================================================================
// Description:
// MHAL_CMDQ_CmdqResetEngine
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_CmdqResetEngine(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    if(MDrvCmdqResetEngine(pCmdinf->pCtx) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_ClearTriggerEvent
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_ClearTriggerEvent(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    /*Clear All*/
    if(MDrvCmdqClearTriggerEvent(pCmdinf->pCtx,0xffff) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
// Description:
// MHAL_CMDQ_ClearTriggerEvent
// Author:
// Kevin Cheng
// Input:
// cmd_mload_interface *cmdinf
// Output:
//=============================================================================
MS_S32  MHAL_CMDQ_CmdDelay(MHAL_CMDQ_CmdqInterface_t *pCmdinf,MS_U32 uTimeNs)
{
    if(MDrvCmdqCmdDelay(pCmdinf->pCtx,uTimeNs) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
//=============================================================================
/// @brief add poll command with Mask
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @param[in] MS_U32 u32RegAddr
/// @param[in] MS_U16 u16Value
/// @param[in] MS_U16 u16WriteMask
/// @param[in] MS_BOOL bPollEq : true is poll eq command , false is poll neq command
/// @param[in] MS_U32 uTimeNs : poll time (ns)
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//=============================================================================
MS_S32  MHAL_CMDQ_CmdqPollRegBits_ByTime(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value,  MS_U16 u16WriteMask, MS_BOOL bPollEq,MS_U32 uTimeNs)
{
	if(MDrvCmdqPollEqCommandMaskByTime(pCmdinf->pCtx,u32RegAddr,u16Value,u16WriteMask,bPollEq,uTimeNs) < 0)
        return MHAL_FAILURE;

    return MHAL_SUCCESS;
}
#if !defined(CAM_OS_RTK)
EXPORT_SYMBOL(MHAL_CMDQ_InitCmdqMmapInfo);
EXPORT_SYMBOL(MHAL_CMDQ_GetSysCmdqService);
EXPORT_SYMBOL(MHAL_CMDQ_ReleaseSysCmdqService);
#endif
