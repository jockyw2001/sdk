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
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
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
#define __DRV_SCL_CTX_M_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"

#include "drv_scl_verchk.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"
#include "hal_utility.h"
#if I2_DVR_TEST_ISR
#include "drv_scl_irq.h"
#endif
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define MDRV_SCL_CTX_NAME_LEN   16
#if 0

#define DRV_CTX_LOCK_MUTEX(x)    \
    SCL_ERR("+++ [MUTEX_LOCK][%s]_1_[%d] \n", __FUNCTION__, __LINE__); \
    DrvSclOsObtainMutex(x, SCLOS_WAIT_FOREVER); \
    SCL_ERR("+++ [MUTEX_LOCK][%s]_2_[%d] \n", __FUNCTION__, __LINE__);

#define DRV_CTX_UNLOCK_MUTEX(x)  \
    SCL_ERR("--- [MUTEX_LOCK][%s]   [%d] \n", __FUNCTION__, __LINE__); \
    DrvSclOsReleaseMutex(x);


#define DRV_CTX_LOCK_SEM(x)      \
    SCL_ERR("+++ [SEM_LOCK][%s]_1_[%d] \n", __FUNCTION__, __LINE__); \
    DrvSclOsObtainTsem(x); \
    SCL_ERR("+++ [SEM_LOCK][%s]_2_[%d] \n", __FUNCTION__, __LINE__);

#define DRV_CTX_UNLOCK_SEM(x)    \
    SCL_ERR("--- [SEM_LOCK][%s]   [%d] \n", __FUNCTION__, __LINE__); \
    DrvSclOsReleaseTsem(x)

#else
#define DRV_CTX_LOCK_MUTEX(x)    //DrvSclOsObtainMutexIrq(x)
#define DRV_CTX_UNLOCK_MUTEX(x)  //DrvSclOsReleaseMutexIrq(x)


#define DRV_CTX_LOCK_SEM(x)      DrvSclOsObtainTsem(x)
#define DRV_CTX_UNLOCK_SEM(x)    DrvSclOsReleaseTsem(x)

#endif

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
MDrvSclCtxConfig_t gstSclCtxTbl[E_MDRV_SCL_CTX_ID_NUM * MDRV_SCL_CTX_INSTANT_MAX];
MDrvSclCtxLockConfig_t gstSclCtxLockTbl;
MDrvSclCtxConfig_t gstGeneralCtx;
MDrvSclCtxCmdqConfig_t *pgstConfigCtx = NULL;
MDrvSclCtxCmdqConfig_t *pgstProcessCtx;
static bool bInit = 0;
MDrvSclCtxIdType_e genCtxIdType = E_MDRV_SCL_CTX_ID_SC_ALL;
// for real Hw setting
u8 gu8MloadBufid[E_HAL_SCLHVSP_SRAM_MLOAD_NUM] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void MDrvSclCtxKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq)
{
    HalUtilityKeepCmdqFunction(pstCmdq);
}
void MDrvSclCtxKeepM2MCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq)
{
    HalUtilityKeepM2MCmdqFunction(pstCmdq);
}
void MDrvSclCtxSetDefaultCtxId(MDrvSclCtxIdType_e enType)
{
    genCtxIdType = enType;
}
void MDrvSclCtxResetWdrTblCnt(void)
{
    pgstConfigCtx->u32SRAMTblCnt[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL] = 0;
}
MDrvSclCtxIdType_e MDrvSclCtxGetDefaultCtxId(void)
{
    return genCtxIdType;
}

bool MDrvSclCtxInit(void)
{
    u8 i = 0, j;
    bool bRet = 1;
    u8 u8CtxLockName[E_MDRV_SCL_CTX_ID_NUM][MDRV_SCL_CTX_NAME_LEN] = {"SCLCTXLOCK1"};
    u8 u8CtxName[E_MDRV_SCL_CTX_ID_NUM][MDRV_SCL_CTX_NAME_LEN] = {"SCLCTX1"};

    if(bInit == 1)
    {
        return TRUE;
    }

    bInit = 1;

    //Lock Config
    DrvSclOsMemset(&gstSclCtxLockTbl, 0, sizeof(MDrvSclCtxLockConfig_t)*E_MDRV_SCL_CTX_ID_NUM);
    gu8MloadBufid[0] = 0x10;
    gu8MloadBufid[1] = 0x10;
    gu8MloadBufid[2] = 0x10;
    gu8MloadBufid[3] = 0x10;
    gu8MloadBufid[4] = 0x10;
    gu8MloadBufid[5] = 0x10;
    gu8MloadBufid[6] = 0x10;
    gu8MloadBufid[7] = 0x10;
    gstSclCtxLockTbl.bLock = FALSE;
    gstSclCtxLockTbl.s32Mutex = DrvSclOsCreateSpinlock(E_DRV_SCLOS_FIFO, u8CtxLockName[i], SCLOS_PROCESS_SHARED);
    gstSclCtxLockTbl.s32Sem = DrvSclOsTsemInit(1);
    gstSclCtxLockTbl.u8IdNum = 0;
    for(j=0; j<MDRV_SCL_CTX_CLIENT_ID_MAX; j++)
    {
        gstSclCtxLockTbl.s32Id[j] = -1;
    }

    if(gstSclCtxLockTbl.s32Mutex == -1 || gstSclCtxLockTbl.s32Sem == -1)
    {
        SCL_ERR("%s %d, Allocate Lock Config Fail, %d\n", __FUNCTION__, __LINE__, i);

        for(j=0; j<i; j++)
        {
            DrvSclOsDeleteMutex(gstSclCtxLockTbl.s32Mutex);
            DrvSclOsTsemDeinit(gstSclCtxLockTbl.s32Sem);
        }
        bRet = 0;
    }

    if(bRet == 0)
    {
        return FALSE;
    }


    //General Ctx allocate
    DrvSclOsMemset(&gstGeneralCtx, 0, sizeof(MDrvSclCtxConfig_t));
    gstGeneralCtx.u8IdNum = 0xFF;
    gstGeneralCtx.stCtx.u32TblCnt = 0;
    gstGeneralCtx.stCtx.enCtxId = E_MDRV_SCL_CTX_ID_SC_ALL;
    gstGeneralCtx.stCtx.pValueIdxTblVir = DrvSclOsMemalloc(MDRV_SCL_CTX_REG_BANK_NUM * MDRV_SCL_CTX_REG_BANK_SIZE,GFP_KERNEL);
    gstGeneralCtx.stCtx.pInquireTbl = DrvSclOsMemalloc(MDRV_SCL_CTX_CMDQ_BUFFER_SIZE,GFP_KERNEL);
    gstGeneralCtx.stCtx.pgstGlobalSet =  DrvSclOsMemalloc(sizeof(MDrvSclCtxGlobalSet_t),GFP_KERNEL);
    gstGeneralCtx.stCtx.pvMloadHandler = NULL;
    HalUtilityGetMloadHandler(&gstGeneralCtx.stCtx.pvMloadHandler);
    for(i=E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL;i<E_MDRV_SCL_CTX_CMDBUF_NUM;i++)
    {
        gstGeneralCtx.stCtx.pSRAMTbl[i] = DrvSclOsMemalloc(MDRV_SCL_CTX_WDROPR_BUFFER_SIZE,GFP_KERNEL);
        DrvSclOsMemset(gstGeneralCtx.stCtx.pSRAMTbl[i], 0, (MDRV_SCL_CTX_WDROPR_BUFFER_SIZE));
        gstGeneralCtx.stCtx.u32SRAMTblCnt[i] = 0;
    }
    if(gstGeneralCtx.stCtx.pValueIdxTblVir == NULL ||
        gstGeneralCtx.stCtx.pInquireTbl == NULL|| gstGeneralCtx.stCtx.pgstGlobalSet == NULL ||
        gstGeneralCtx.stCtx.pSRAMTbl[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL] == NULL )
    {
        SCL_ERR("%s %d, Allocate General Ctx Fail\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    else
    {
        DrvSclOsMemset(gstGeneralCtx.stCtx.pValueIdxTblVir, 0, (MDRV_SCL_CTX_REG_BANK_NUM * MDRV_SCL_CTX_REG_BANK_SIZE));
        DrvSclOsMemset(gstGeneralCtx.stCtx.pInquireTbl, 0, (MDRV_SCL_CTX_CMDQ_BUFFER_SIZE));
        DrvSclOsMemset(gstGeneralCtx.stCtx.pgstGlobalSet, 0, (sizeof(MDrvSclCtxGlobalSet_t)));
        for(i=E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL;i<E_MDRV_SCL_CTX_CMDBUF_NUM;i++)
        {
            DrvSclOsMemset(gstGeneralCtx.stCtx.pSRAMTbl[i], 0, (MDRV_SCL_CTX_WDROPR_BUFFER_SIZE));
        }
    }
    //utility
    HalUtilitySetDefaultBuffer(gstGeneralCtx.stCtx.pValueIdxTblVir,gstGeneralCtx.stCtx.pInquireTbl,&gstGeneralCtx.stCtx.u32TblCnt);
    HalUtilitySetDefaultWdrTblBuffer(gstGeneralCtx.stCtx.pSRAMTbl[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL],
        &gstGeneralCtx.stCtx.u32SRAMTblCnt[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL]);
    HalUtilityInit(gstSclCtxLockTbl.s32Mutex);

    // Instance Ctx
    DrvSclOsMemset(&gstSclCtxTbl, 0, sizeof(MDrvSclCtxCmdqConfig_t) * E_MDRV_SCL_CTX_ID_NUM * MDRV_SCL_CTX_INSTANT_MAX);
    for(i=0; i<E_MDRV_SCL_CTX_ID_NUM * MDRV_SCL_CTX_INSTANT_MAX; i++)
    {
        if(i<MDRV_SCL_CTX_INSTANT_MAX)
        {
            u8CtxName[0][7] = 48 + i;

        }
        else
        {
            u8CtxName[1][7] = 48 + i - MDRV_SCL_CTX_INSTANT_MAX;
        }
        gstSclCtxTbl[i].bUsed = 0;
        gstSclCtxTbl[i].u8IdNum = 0;
        for(j=0; j<MDRV_SCL_CTX_CLIENT_ID_MAX; j++)
        {
            gstSclCtxTbl[i].s32Id[j] = -1;
        }

    }
    if(bRet == 1)
    {
        pgstConfigCtx = &(gstGeneralCtx.stCtx);
    }
    return bRet;
}
bool MDrvSclCtxDeInit(void)
{
    u8 i, j;
    bool bRet = 1;

    if(bInit == 0)
    {
        return TRUE;
    }

    bInit = 0;
    HalUtilityDeInit();
    gu8MloadBufid[0] = 0x10;
    gu8MloadBufid[1] = 0x10;
    gu8MloadBufid[2] = 0x10;
    gu8MloadBufid[3] = 0x10;
    gu8MloadBufid[4] = 0x10;
    gu8MloadBufid[5] = 0x10;
    gu8MloadBufid[6] = 0x10;
    gu8MloadBufid[7] = 0x10;
    //Lock Config
    if(gstSclCtxLockTbl.s32Mutex != -1 || gstSclCtxLockTbl.s32Sem != -1)
    {
        DrvSclOsDeleteSpinlock(gstSclCtxLockTbl.s32Mutex);
        DrvSclOsTsemDeinit(gstSclCtxLockTbl.s32Sem);
        gstSclCtxLockTbl.s32Mutex = -1;
        gstSclCtxLockTbl.s32Sem = -1;
        gstSclCtxLockTbl.bLock = FALSE;
        gstSclCtxLockTbl.u8IdNum = 0;
        for(j=0; j<MDRV_SCL_CTX_CLIENT_ID_MAX; j++)
        {
            gstSclCtxLockTbl.s32Id[j] = 0;
        }
        bRet = 0;
    }

    //General Ctx allocate
    gstGeneralCtx.u8IdNum = 0;
    gstGeneralCtx.stCtx.u32TblCnt = 0;
    gstGeneralCtx.stCtx.enCtxId = E_MDRV_SCL_CTX_ID_SC_ALL;
    DrvSclOsMemFree(gstGeneralCtx.stCtx.pValueIdxTblVir);
    DrvSclOsMemFree(gstGeneralCtx.stCtx.pInquireTbl);
    DrvSclOsMemFree(gstGeneralCtx.stCtx.pgstGlobalSet);
    gstGeneralCtx.stCtx.pValueIdxTblVir = NULL;
    gstGeneralCtx.stCtx.pInquireTbl = NULL;
    gstGeneralCtx.stCtx.pgstGlobalSet =  NULL;
    for(i=E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL;i<E_MDRV_SCL_CTX_CMDBUF_NUM;i++)
    {
        DrvSclOsMemFree(gstGeneralCtx.stCtx.pSRAMTbl[i]);
        gstGeneralCtx.stCtx.pSRAMTbl[i] = NULL;
        gstGeneralCtx.stCtx.u32SRAMTblCnt[i]= 0;
    }

    // Instance Ctx
    for(i=0; i<E_MDRV_SCL_CTX_ID_NUM * MDRV_SCL_CTX_INSTANT_MAX; i++)
    {
        gstSclCtxTbl[i].bUsed = 0;
        gstSclCtxTbl[i].u8IdNum = 0;
        for(j=0; j<MDRV_SCL_CTX_CLIENT_ID_MAX; j++)
        {
            gstSclCtxTbl[i].s32Id[j] = 0;
        }
        gstSclCtxTbl[i].stCtx.u32TblCnt = 0;
        gstSclCtxTbl[i].stCtx.enCtxId = E_MDRV_SCL_CTX_ID_SC_ALL;
        for(j=E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL;j<E_MDRV_SCL_CTX_CMDBUF_NUM;j++)
        {
            DrvSclOsMemFree(gstSclCtxTbl[i].stCtx.pSRAMTbl[j]);
            gstSclCtxTbl[i].stCtx.pSRAMTbl[j] = NULL;
            gstSclCtxTbl[i].stCtx.u32SRAMTblCnt[j] = NULL;
        }

    }
    return bRet;
}
//for pass allocate
MDrvSclCtxConfig_t *MDrvSclCtxAllocate(MDrvSclCtxIdType_e enCtxId, MDrvSclCtxInstConfig_t *stCtxInst)
{
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    DrvSclosDevType_e enDevId = E_DRV_SCLOS_DEV_MAX;
    HalUtilityIdType_e enUtId;
    u16 i, start, end;
    u32 u32Size;
    u16 j;

    start = enCtxId * MDRV_SCL_CTX_INSTANT_MAX;
    end = start + MDRV_SCL_CTX_INSTANT_MAX;
    u32Size = (enCtxId==E_MDRV_SCL_CTX_ID_SC_ALL) ? MDRV_SCL_CTX_SCLALL_REG_BANK_NUM :
        (enCtxId==E_MDRV_SCL_CTX_ID_SC_VIP) ? MDRV_SCL_CTX_SCLVIP_REG_BANK_NUM :
            MDRV_SCL_CTX_REG_BANK_NUM;
    for(i=start; i<end ;i++)
    {
        if(gstSclCtxTbl[i].bUsed == FALSE)
        {
            pCtxCfg = &gstSclCtxTbl[i];
            gstSclCtxTbl[i].bUsed = TRUE;
            gstSclCtxTbl[i].stCtx.pvMloadHandler = NULL;
            gstSclCtxTbl[i].stCtx.u32TblCnt = 0;
            gstSclCtxTbl[i].stCtx.pValueIdxTblVir = DrvSclOsMemalloc(u32Size * MDRV_SCL_CTX_REG_BANK_SIZE,GFP_KERNEL);
            DrvSclOsMemset(gstSclCtxTbl[i].stCtx.pValueIdxTblVir, 0, (u32Size * MDRV_SCL_CTX_REG_BANK_SIZE));
            gstSclCtxTbl[i].stCtx.pInquireTbl = DrvSclOsMemalloc(MDRV_SCL_CTX_CMDQ_BUFFER_SIZE,GFP_KERNEL);
            DrvSclOsMemset(gstSclCtxTbl[i].stCtx.pInquireTbl, 0, (MDRV_SCL_CTX_CMDQ_BUFFER_SIZE));
            gstSclCtxTbl[i].stCtx.pgstGlobalSet =  DrvSclOsMemalloc(sizeof(MDrvSclCtxGlobalSet_t),GFP_KERNEL);
            DrvSclOsMemset(gstSclCtxTbl[i].stCtx.pgstGlobalSet, 0, (sizeof(MDrvSclCtxGlobalSet_t)));
            gstSclCtxTbl[i].stCtx.enCtxId = enCtxId;

            for(j=E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL;j<E_MDRV_SCL_CTX_CMDBUF_NUM;j++)
            {
                gstSclCtxTbl[i].stCtx.pSRAMTbl[j] = DrvSclOsMemalloc(MDRV_SCL_CTX_WDROPR_BUFFER_SIZE,GFP_KERNEL);
                gstSclCtxTbl[i].stCtx.u32SRAMTblCnt[j]= 0;
            }

            if(gstSclCtxTbl[i].stCtx.pValueIdxTblVir == NULL ||
               gstSclCtxTbl[i].stCtx.pInquireTbl == NULL ||
               gstSclCtxTbl[i].stCtx.pgstGlobalSet == NULL
                || gstSclCtxTbl[i].stCtx.pSRAMTbl[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL] == NULL)

               //gstSclCtxTbl[i].stCtx.pSRAMTbl[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL] == NULL)
            {
                if(gstSclCtxTbl[i].stCtx.pValueIdxTblVir)
                {
                    DrvSclOsMemFree(gstSclCtxTbl[i].stCtx.pValueIdxTblVir);
                    gstSclCtxTbl[i].stCtx.pValueIdxTblVir = NULL;
                }
                if(gstSclCtxTbl[i].stCtx.pInquireTbl)
                {
                    DrvSclOsMemFree(gstSclCtxTbl[i].stCtx.pInquireTbl);
                    gstSclCtxTbl[i].stCtx.pInquireTbl = NULL;
                }
                if(gstSclCtxTbl[i].stCtx.pgstGlobalSet)
                {
                    DrvSclOsMemFree(gstSclCtxTbl[i].stCtx.pgstGlobalSet);
                    gstSclCtxTbl[i].stCtx.pgstGlobalSet = NULL;
                }

                if(gstSclCtxTbl[i].stCtx.pSRAMTbl[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL])
                {
                    DrvSclOsMemFree(gstSclCtxTbl[i].stCtx.pSRAMTbl[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL]);
                    gstSclCtxTbl[i].stCtx.pSRAMTbl[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL] = NULL;
                }

                SCL_ERR("%s %d, Allocate ValueIdxTbl Fail %d \n", __FUNCTION__, __LINE__, i);
                return 0;
            }
            else
            {
                DrvSclOsMemset(gstSclCtxTbl[i].stCtx.pValueIdxTblVir, 0, (u32Size * MDRV_SCL_CTX_REG_BANK_SIZE));
                DrvSclOsMemset(gstSclCtxTbl[i].stCtx.pInquireTbl, 0, (MDRV_SCL_CTX_CMDQ_BUFFER_SIZE));
                DrvSclOsMemset(gstSclCtxTbl[i].stCtx.pgstGlobalSet, 0, (sizeof(MDrvSclCtxGlobalSet_t)));

                for(j=E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL;j<E_MDRV_SCL_CTX_CMDBUF_NUM;j++)
                {
                    DrvSclOsMemset(gstSclCtxTbl[i].stCtx.pSRAMTbl[j], 0, (MDRV_SCL_CTX_WDROPR_BUFFER_SIZE));
                }

            }
            break;//for for
        }
    }
    pCtxCfg->u8IdNum = stCtxInst->u8IdNum;
    pCtxCfg->stCtx.enCtxId = enCtxId;
    enUtId = (enCtxId==E_MDRV_SCL_CTX_ID_SC_ALL) ? E_HAL_SCL_UTI_ID_SC_ALL :
             (enCtxId==E_MDRV_SCL_CTX_ID_SC_VIP) ? E_HAL_SCL_UTI_ID_SC_VIP :
                E_HAL_SCL_UTI_ID_NUM;
    HalUtilityCreateInst(enUtId,pCtxCfg->stCtx.pValueIdxTblVir);
    SCL_DBG(SCL_DBG_LV_CTX_CONFIG()&EN_DBGMG_CTX_INIT, "[CTX]MDrvSclCtxAllocate @CtxTblId:%hu,Ctx@%lx Globla@%lx\n"
        ,i,(u32)&pCtxCfg->stCtx,(u32)pCtxCfg->stCtx.pgstGlobalSet);
    for(i=0; i<stCtxInst->u8IdNum; i++)
    {
        enDevId = MDrvSclOsGetDevByHandler(stCtxInst->ps32IdBuf[i]);
        pCtxCfg->s32Id[enDevId] = stCtxInst->ps32IdBuf[i];
    }
    SCL_DBG(SCL_DBG_LV_CTX_CONFIG()&EN_DBGMG_CTX_INIT, "[CTX]MDrvSclCtxAllocate @CtxId:%d ,enDevId:%d,@%lx NUm:%hhd\n"
        ,enCtxId,enDevId,(u32)pCtxCfg,pCtxCfg->u8IdNum);
    return pCtxCfg;
}

bool MDrvSclCtxFree(MDrvSclCtxConfig_t *pCtxCfg)
{
    u16 i;
    u32 u32Size;
    u32Size = (pCtxCfg->stCtx.enCtxId==E_MDRV_SCL_CTX_ID_SC_ALL) ? MDRV_SCL_CTX_SCLALL_REG_BANK_NUM :
        (pCtxCfg->stCtx.enCtxId==E_MDRV_SCL_CTX_ID_SC_VIP) ? MDRV_SCL_CTX_SCLVIP_REG_BANK_NUM :
            MDRV_SCL_CTX_REG_BANK_NUM;
    if(pCtxCfg == NULL)
    {
        return FALSE;
    }
    pCtxCfg->bUsed = 0;
    for(i=0; i<E_DRV_SCLOS_DEV_MAX; i++)
    {
        pCtxCfg->s32Id[i] = 0;
    }
    pCtxCfg->u8IdNum = 0;
    pCtxCfg->stCtx.u32TblCnt = 0;
    DrvSclOsMemset(pCtxCfg->stCtx.pValueIdxTblVir,0,(u32Size * MDRV_SCL_CTX_REG_BANK_SIZE));
    DrvSclOsMemset(pCtxCfg->stCtx.pInquireTbl,0,MDRV_SCL_CTX_CMDQ_BUFFER_SIZE);
    DrvSclOsMemset(pCtxCfg->stCtx.pgstGlobalSet,0,sizeof(sizeof(MDrvSclCtxGlobalSet_t)));
    pCtxCfg->stCtx.u32TblCnt = 0;
    pCtxCfg->stCtx.enCtxId = E_MDRV_SCL_CTX_ID_NUM;
    HalUtilityFreeMloadHandler(pCtxCfg->stCtx.pvMloadHandler);
    pCtxCfg->stCtx.pvMloadHandler = NULL;
    DrvSclOsMemFree(pCtxCfg->stCtx.pValueIdxTblVir);
    DrvSclOsMemFree(pCtxCfg->stCtx.pInquireTbl);
    DrvSclOsMemFree(pCtxCfg->stCtx.pgstGlobalSet);

    for(i=E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL;i<E_MDRV_SCL_CTX_CMDBUF_NUM;i++)
    {
        DrvSclOsMemset(pCtxCfg->stCtx.pSRAMTbl[i], 0, (MDRV_SCL_CTX_WDROPR_BUFFER_SIZE));
        pCtxCfg->stCtx.u32SRAMTblCnt[i]= 0;
        DrvSclOsMemFree(pCtxCfg->stCtx.pSRAMTbl[i]);
        pCtxCfg->stCtx.pSRAMTbl[i] = NULL;
    }

    pCtxCfg->stCtx.pValueIdxTblVir = NULL;
    pCtxCfg->stCtx.pInquireTbl = NULL;
    pCtxCfg->stCtx.pgstGlobalSet =  NULL;
    SCL_DBG(SCL_DBG_LV_CTX_CONFIG()&EN_DBGMG_CTX_INIT, "[CTX]MDrvSclCtxFree Ctx@%lx Globla@%lx @%lx\n"
        ,(u32)&pCtxCfg->stCtx,(u32)pCtxCfg->stCtx.pgstGlobalSet,(u32)pCtxCfg);
    return TRUE;
}
//for Flip Reg Tbl
bool MDrvSclCtxFireM2M(MDrvSclCtxCmdqConfig_t *pCtx)
{
    HalUtilityFilpRegFire(E_HAL_SCL_UTI_CMDQID_RSC);
    return TRUE;
}
bool MDrvSclCtxFire(MDrvSclCtxCmdqConfig_t *pCtx)
{
    HalUtilityFilpRegFire(E_HAL_SCL_UTI_CMDQID_SC_ALL);
    return TRUE;
}
void *_MDrvSclCtxGetMloadBuf(MDrvSclCtxMloadID_e enId,MDrvSclCtxCmdqConfig_t *pCtx)
{
    void *pvBuf;
    MDrvSclCtxHalHvspGlobalSet_t *pstGlobalHalHvspSet;
    MDrvSclCtxMvipGlobalSet_t *pstGlobalMVipSet;
    HalSclHvspSramMloadType_e enType;
    pstGlobalHalHvspSet = &(((MDrvSclCtxGlobalSet_t*)(pCtx->pgstGlobalSet))->stHalHvspCfg);
    pstGlobalMVipSet = &(((MDrvSclCtxGlobalSet_t*)(pCtx->pgstGlobalSet))->stMvipCfg);
    switch(enId)
    {
        case E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC1:
            enType = E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H;
            break;
        case E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC1:
            enType = E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_V;
            break;
        case E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC2:
            enType = E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H_1;
            break;
        case E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC2:
            enType = E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_V_1;
            break;
        case E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC3:
            enType = E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H_2;
            break;
        case E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC3:
            enType = E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_V_2;
            break;
        case E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC4:
            enType = E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H_3;
            break;
        case E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC4:
            enType = E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_V_3;
            break;
        case E_MDRV_SCL_CTX_MLOAD_ID_WDR:
            enType = E_HAL_SCLHVSP_SRAM_MLOAD_NUM;
#if defined(SCLOS_TYPE_MLOAD)
            pvBuf = (void *)pstGlobalMVipSet->u32VirWdrMloadBuf;
#else
            pvBuf = (void *)pstGlobalMVipSet->u32WdrMloadBuf;
#endif
            break;
        default:
            SCLOS_BUG();
    }
    if(enType != E_HAL_SCLHVSP_SRAM_MLOAD_NUM)
    {
        if(pstGlobalHalHvspSet->bMloadchange[enType] &&
        (gu8MloadBufid[enType] != pstGlobalHalHvspSet->u8MloadBufid[enType]))
        {
            gu8MloadBufid[enType] = pstGlobalHalHvspSet->u8MloadBufid[enType];
            pvBuf = pstGlobalHalHvspSet->pvMloadBuf[enType];
        }
        else
        {
            pvBuf = NULL;
        }
    }
    SCL_DBG(SCL_DBG_LV_CTX_CONFIG()&EN_DBGMG_CTX_HIGH,"[%s]return @:%lx\n",__FUNCTION__,(u32)pvBuf);
    return pvBuf;
}
bool MDrvSclCtxSetMload(MDrvSclCtxMloadID_e enId,MDrvSclCtxCmdqConfig_t *pCtx)
{
    void *pvBuf;
    bool bRet = 0;
    if(pCtx->pvMloadHandler==NULL)
    {
        HalUtilityGetMloadHandler(&pCtx->pvMloadHandler);
    }
    pvBuf = _MDrvSclCtxGetMloadBuf(enId,pCtx);
    if(pvBuf)
    {
        HalUtilitySetMloadTbl(enId,pCtx->pvMloadHandler,pvBuf);
        //SCL_DBGERR("[%s]%u\n",__FUNCTION__,enId);
        bRet = 1;
    }
    return bRet;
}
DrvSclHvspIpMuxType_e _DrvSclDmaGetInputMuxType(DrvSclHvspIdType_e enIpType);
bool MDrvSclCtxTriggerM2M(bool bFire,MDrvSclCtxCmdqConfig_t *pCtx)
{
    DrvSclHvspIpMuxType_e enIpType;
    if(bFire)
    {
        enIpType = (_DrvSclDmaGetInputMuxType(E_DRV_SCLHVSP_ID_3));
        if((enIpType==E_DRV_SCLHVSP_IP_MUX_LDC || enIpType==E_DRV_SCLHVSP_IP_MUX_RSC))
        {
            HalUtilityTriggerM2M(pCtx);
        }
    }
    return TRUE;
}
bool MDrvSclCtxFireMload(bool bFire,MDrvSclCtxCmdqConfig_t *pCtx)
{
    if(bFire)
    {
        HalUtilityMloadFire(pCtx->pvMloadHandler);
    }
    return TRUE;
}
bool MDrvSclCtxSetWaitMload(void)
{
    HalUtilitySetWaitMload();
    return TRUE;
}
// for context switch
bool MDrvSclCtxSwapConfigCtx(MDrvSclCtxConfig_t *pCtxCfg,MDrvSclCtxIdType_e enCtxId)
{
    HalUtilityIdType_e enUtId;
    DrvSclosDevType_e enDevId;
    if(pCtxCfg && pCtxCfg->s32Id[E_DRV_SCLOS_DEV_M2M] != -1)
    {
        enDevId = MDrvSclOsGetDevByHandler(pCtxCfg->s32Id[E_DRV_SCLOS_DEV_M2M]);
    }
    else
    {
        enDevId = E_DRV_SCLOS_DEV_MAX;
    }
    if(pCtxCfg)
    {
        pgstConfigCtx = &(pCtxCfg->stCtx);
        enUtId = (enCtxId==E_MDRV_SCL_CTX_ID_SC_ALL) ? E_HAL_SCL_UTI_ID_SC_ALL :
                 (enCtxId==E_MDRV_SCL_CTX_ID_SC_VIP) ? E_HAL_SCL_UTI_ID_SC_VIP :
                    E_HAL_SCL_UTI_ID_NUM;
        HalUtilitySetUtiId(enUtId);
        if(enDevId==E_DRV_SCLOS_DEV_M2M)
        {
            HalUtilitySwapCmdqPf(E_HAL_SCL_UTI_CMDQID_RSC);
        }
        else
        {
            HalUtilitySwapCmdqPf(0);
        }
        SCL_DBG(SCL_DBG_LV_CTX_CONFIG()&EN_DBGMG_CTX_HIGH, "[CTX]Swap Ctx success @CtxId:%d ,@%lx @%lx\n"
            ,enCtxId,(u32)pCtxCfg,(u32)pgstConfigCtx);
        return TRUE;
    }
    else
    {
        pgstConfigCtx = &(gstGeneralCtx.stCtx);
        HalUtilitySetUtiId(E_HAL_SCL_UTI_ID_SC_DEFAULT);
        HalUtilitySwapCmdqPf(0);
        SCL_DBG(SCL_DBG_LV_CTX_CONFIG()&EN_DBGMG_CTX_HIGH, "[CTX]Swap Ctx Default @CtxId:%d ,@%lx @%lx\n"
            ,enCtxId,(u32)pCtxCfg,(u32)pgstConfigCtx);
        return FALSE;
    }
}
bool MDrvSclCtxSwapProcessCtx(MDrvSclCtxConfig_t *pCtxCfg,MDrvSclCtxIdType_e enCtxId)
{
    if(pCtxCfg)
    {
        pgstProcessCtx = &(pCtxCfg->stCtx);
        return TRUE;
    }
    else
    {
        pgstProcessCtx = &(gstGeneralCtx.stCtx);
        return FALSE;
    }
}
MDrvSclCtxConfig_t *MDrvSclCtxGetCtx(u16 u16inst)
{
    return &(gstSclCtxTbl[u16inst]);
}
MDrvSclCtxConfig_t *MDrvSclCtxGetInstByHandler(s32 s32Handler,MDrvSclCtxIdType_e enCtxId)
{
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    DrvSclosDevType_e enDevId;
    u16 start, end , inst,success = 0;

    start = enCtxId * MDRV_SCL_CTX_INSTANT_MAX;
    end = start + MDRV_SCL_CTX_INSTANT_MAX;
    enDevId = MDrvSclOsGetDevByHandler(s32Handler);
    for(inst=start;inst<end;inst++)
    {
        if(gstSclCtxTbl[inst].bUsed)
        {
            if(gstSclCtxTbl[inst].s32Id[enDevId] == s32Handler)
            {
                success = 1;
                break;
            }
        }
    }
    if(success)
    {
        pCtxCfg = MDrvSclCtxGetCtx(inst);
        SCL_DBG(SCL_DBG_LV_CTX_CONFIG()&EN_DBGMG_CTX_HIGH, "[CTX]Get Ctx success @inst:%hu ,@%lx\n"
            ,inst,(u32)pCtxCfg);
    }
    else
    {
        pCtxCfg = NULL;
        SCL_DBG(SCL_DBG_LV_CTX_CONFIG()&EN_DBGMG_CTX_HIGH, "[CTX]Get Ctx Default @inst:%hu ,@%lx\n"
            ,inst,(u32)pCtxCfg);
    }
    return pCtxCfg;
}
MDrvSclCtxCmdqConfig_t *MDrvSclCtxGetProcessCtx(MDrvSclCtxIdType_e enCtxId)
{
    return pgstProcessCtx;
}

MDrvSclCtxCmdqConfig_t *MDrvSclCtxGetConfigCtx(MDrvSclCtxIdType_e enCtxId)
{
    return pgstConfigCtx;
}
MDrvSclCtxConfig_t *MDrvSclCtxGetDefaultCtx(void)
{
    return &gstGeneralCtx;
}

MDrvSclCtxLockConfig_t *MDrvSclCtxGetLockConfig(MDrvSclCtxIdType_e enCtxId)
{
    MDrvSclCtxLockConfig_t *pstSclCtxLockTbl;
    pstSclCtxLockTbl = &gstSclCtxLockTbl;
    return pstSclCtxLockTbl;
}
bool MDrvSclCtxIspAddNewFrame(MDrvSclCtxIspInterface_t *pstCfg)
{
    return 0;
}
bool _MDrvSclCtxSwapConfigRegTbl(MDrvSclCtxIdType_e enCtxId)
{
    void *pvCtx;
    pvCtx = pgstConfigCtx->pValueIdxTblVir;
    HalUtilitySwapActiveRegTbl(pvCtx);
    return TRUE;
}
bool _MDrvSclCtxSwapInquireTbl(MDrvSclCtxIdType_e enCtxId)
{
    void *pvCtx;
    pvCtx = pgstConfigCtx->pInquireTbl;
    HalUtilitySwapInquireTbl(pvCtx);
    return TRUE;
}
bool _MDrvSclCtxSwapTblCnt(MDrvSclCtxIdType_e enCtxId)
{
    u32 *pu32Cnt;
    pu32Cnt = &pgstConfigCtx->u32TblCnt;
    HalUtilitySwapTblCnt(pu32Cnt);
    return TRUE;
}
bool _MDrvSclCtxSwapWdrTbl(MDrvSclCtxIdType_e enCtxId)
{
    void *pvCtx;
    pvCtx = pgstConfigCtx->pSRAMTbl[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL];
    HalUtilitySwapWdrTbl(pvCtx);
    return TRUE;
}
bool _MDrvSclCtxSwapWdrTblCnt(MDrvSclCtxIdType_e enCtxId)
{
    u32 *pu32Cnt;
    pu32Cnt = &pgstConfigCtx->u32SRAMTblCnt[E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL];
    HalUtilitySwapWdrTblCnt(pu32Cnt);
    return TRUE;
}
void MDrvSclCtxDumpSetting(void)
{
    HalUtilityDumpSetting();
}
void MDrvSclCtxDumpRegSetting(u32 u32RegAddr,bool bAllbank)
{
    if(bAllbank)
    {
        HalUtilityDumpRegSettingAll(u32RegAddr);
    }
    else
    {
        SCL_ERR("Offset:%lx val:%hx\n",((u32RegAddr&0xFF)/2),HalUtilityR2BYTE(u32RegAddr));
    }
}
bool MDrvSclCtxSetLockConfig(s32 s32Handler,MDrvSclCtxIdType_e enCtxId)
{
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    MDrvSclCtxLockConfig_t *pstSclCtxLockTbl;
    pstSclCtxLockTbl = MDrvSclCtxGetLockConfig(enCtxId);
    DRV_CTX_LOCK_SEM(pstSclCtxLockTbl->s32Sem);

    DRV_CTX_LOCK_MUTEX(pstSclCtxLockTbl->s32Mutex);
    MDrvSclCtxSetDefaultCtxId(enCtxId);
    pstSclCtxLockTbl->bLock = 1;
    //context switch
    pCtxCfg = MDrvSclCtxGetInstByHandler(s32Handler,enCtxId);
    MDrvSclCtxSwapConfigCtx(pCtxCfg,enCtxId);
    _MDrvSclCtxSwapConfigRegTbl(enCtxId);
    _MDrvSclCtxSwapInquireTbl(enCtxId);
    _MDrvSclCtxSwapTblCnt(enCtxId);
    _MDrvSclCtxSwapWdrTbl(enCtxId);
    _MDrvSclCtxSwapWdrTblCnt(enCtxId);
    DRV_CTX_UNLOCK_MUTEX(pstSclCtxLockTbl->s32Mutex);

    return 0;

}

bool MDrvSclCtxSetUnlockConfig(s32 s32Handler,MDrvSclCtxIdType_e enCtxId)
{
    bool bRet = 0;
    MDrvSclCtxLockConfig_t *pstSclCtxLockTbl;
    pstSclCtxLockTbl = MDrvSclCtxGetLockConfig(enCtxId);

    DRV_CTX_LOCK_MUTEX(pstSclCtxLockTbl->s32Mutex);
    if(pstSclCtxLockTbl->bLock)
    {
        MDrvSclCtxSwapConfigCtx(NULL,enCtxId);//change to default
        _MDrvSclCtxSwapConfigRegTbl(enCtxId);
        _MDrvSclCtxSwapInquireTbl(enCtxId);
        _MDrvSclCtxSwapTblCnt(enCtxId);
        _MDrvSclCtxSwapWdrTbl(enCtxId);
        _MDrvSclCtxSwapWdrTblCnt(enCtxId);
        pstSclCtxLockTbl->bLock = 0;
        MDrvSclCtxSetDefaultCtxId(E_MDRV_SCL_CTX_ID_SC_ALL);
        DRV_CTX_UNLOCK_SEM(pstSclCtxLockTbl->s32Sem);
    }

    DRV_CTX_UNLOCK_MUTEX(pstSclCtxLockTbl->s32Mutex);

    return (bRet);
}
