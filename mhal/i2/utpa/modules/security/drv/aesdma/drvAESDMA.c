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
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// file    drvAESDMA.c
// @brief  AESDMA Driver
// @author MStar Semiconductor,Inc.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(MSOS_TYPE_LINUX_KERNEL)
#include <stdio.h>
#include <string.h>
#else
#include <linux/string.h>
#include <linux/slab.h>
#endif
#include "MsCommon.h"
#include "MsIRQ.h"
#include "MsOS.h"
#include "MsVersion.h"
#include "drvSYS.h"

#include "drvMMIO.h"
#include "halAESDMA.h"
#include "drvAESDMA.h"
#include "utopia.h"
#include "utopia_dapi.h"
#include "drvAESDMA_private.h"
#include "ULog.h"

#include "drvSEM.h"
#include "drvCMDQ.h"
#include "drvCIPHER.h"
#include "drvCIPHER_v2.h"
#include "halSEM.h"

//--------------------------------------------------------------------------------------------------
//  Driver Compiler Option
//--------------------------------------------------------------------------------------------------
#if !defined(MSOS_TYPE_NOS)
#define AESDMAIntEnable()                       MsOS_EnableInterrupt(E_INT_IRQ_WADR_ERR);
#define AESDMAIntDisable()                      MsOS_DisableInterrupt(E_INT_IRQ_WADR_ERR);
#define AESDMAIntAttach()                       MsOS_AttachInterrupt(E_INT_IRQ_WADR_ERR, (InterruptCb )_AESDMA_Isr)
#define AESDMAIntDetach()                       MsOS_DetachInterrupt (E_INT_IRQ_WADR_ERR)
#endif //#if !defined(MSOS_TYPE_NOS)
#define AESDMAPA2KSEG1(addr)                    ((void *)(((MS_U32)addr) | 0xa0000000)) //physical -> unchched


//--------------------------------------------------------------------------------------------------
//  Local Defines
//--------------------------------------------------------------------------------------------------
#if !defined(MSOS_TYPE_NOS)
#define AESDMA_TASK_STACK_SIZE 4096UL
#endif //#if !defined(MSOS_TYPE_NOS)

#define TAG_AESDMA "AESDMA"

static MS_PHY phyTmpFileInAddr = 0;
static MS_PHY phyTmpFileOutAddr = 0;
static MS_U32 u32TmpFileInNum = 0;

static MS_U32 u32ProtEng0 = 0;

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
static DrvAESDMA_MDB_AESDMA sDrvAESDMA_MDB_AESDMA;
static DrvAESDMA_MDB_SHA sDrvAESDMA_MDB_SHA;
static DrvAESDMA_MDB_RSA sDrvAESDMA_MDB_RSA;
#endif
//--------------------------------------------------------------------------------------------------
//  Global Variable
//--------------------------------------------------------------------------------------------------
//#ifndef MSOS_TYPE_LINUX
static DrvAESDMA_Event                  _AESDMAEvtNotify;
static P_DrvAESDMA_EvtCallback          _AESDMACallback;
//#endif // #ifndef MSOS_TYPE_LINUX

#if !defined(MSOS_TYPE_NOS)
static void*                            _pAESDMATaskStack;
static MS_U8                            _u8AESDMA_StackBuffer[AESDMA_TASK_STACK_SIZE];
static MS_S32                           _s32AESDMATaskId = -1;
static MS_U32                           _u32LastErr;
#endif //#if !defined(MSOS_TYPE_NOS)

static MS_PHY                         _u32MIU0_Addr, _u32MIU1_Addr , _u32MIU2_Addr, _u32MIU_num;
static DrvAESDMA_DbgLevel               _u32AESDMADBGLevel;
static MS_BOOL                          _bExitIsrLoop;

static MSIF_Version _drv_aesdma_version = {
    .DDI = { AESDMA_DRV_VERSION, },
};

#if (AESDMA_UTOPIA20)
void* pInstantAesdma = NULL;
void* pAttributeAesdma = NULL;
#endif

//--------------------------------------------------------------------------------------------------
//  Internal Variable
//--------------------------------------------------------------------------------------------------
#if !defined(MSOS_TYPE_NOS)
#if AESDMS_SECURED_BANK_EN
static MS_S32                           _s32AESDMAMutexId = -1;
#endif
static MS_S32                           _s32AESDMAEventId = -1;
#endif //#if !defined(MSOS_TYPE_NOS)
static MS_U8                            _u8IsInit = FALSE;
#ifndef MSOS_TYPE_NUTTX
static MS_BOOL                          bSecureModeEnable = FALSE;
#endif

#if !defined(MSOS_TYPE_NOS)
#define _AESDMA_SetEvent(flag)          MsOS_SetEvent(_s32AESDMAEventId, (flag))
#define _AESDMA_GetEvent(events)        MsOS_WaitEvent(_s32AESDMAEventId, AESDMA_STATES_GROUP, &events, E_OR_CLEAR, MSOS_WAIT_FOREVER)
#endif //#if !defined(MSOS_TYPE_NOS)

//--------------------------------------------------------------------------------------------------
//  Debug Function
//--------------------------------------------------------------------------------------------------
#define DEBUG_AES_DMA(debug_level, x)     do { if (_u32AESDMADBGLevel >= (debug_level)) (x); } while(0)

//--------------------------------------------------------------------------------------------------
//  Internal Function
//--------------------------------------------------------------------------------------------------
static MS_BOOL _AESDMA_Chk_MIU(MS_PHY *phy64InAddr, MS_U32 u32InLen, MS_PHY *phy64OutSAddr, MS_PHY *phyOutEAddr)
{

    if (_u32MIU_num == 3)
    {
        if ((_u32MIU1_Addr > *phy64InAddr) & (_u32MIU1_Addr > *phy64InAddr+u32InLen) & (_u32MIU1_Addr > *phy64OutSAddr) & (_u32MIU1_Addr > *phyOutEAddr))
        {
            AESDMA_Set_MIU_Path(FALSE,FALSE);
            AESDMA_Set_MIU2_Path(FALSE,FALSE); // miu0->miu0
            return TRUE;
        }

        if ((_u32MIU1_Addr > *phy64InAddr) & (_u32MIU1_Addr > *phy64InAddr+u32InLen) & (_u32MIU1_Addr < *phy64OutSAddr) & (_u32MIU1_Addr < *phyOutEAddr) & (_u32MIU2_Addr > *phy64OutSAddr) & (_u32MIU2_Addr > *phyOutEAddr))
        {
            *phy64OutSAddr = *phy64OutSAddr - _u32MIU1_Addr;
            *phyOutEAddr = *phyOutEAddr - _u32MIU1_Addr;
            AESDMA_Set_MIU_Path(FALSE,TRUE);
            AESDMA_Set_MIU2_Path(FALSE,FALSE); // miu0->miu1
            return TRUE;
        }

        if ((_u32MIU1_Addr > *phy64InAddr) & (_u32MIU1_Addr > *phy64InAddr+u32InLen) & (_u32MIU2_Addr <= *phy64OutSAddr) & (_u32MIU2_Addr <= *phyOutEAddr))
        {
            *phy64OutSAddr = *phy64OutSAddr - _u32MIU2_Addr;
            *phyOutEAddr = *phyOutEAddr - _u32MIU2_Addr;
            AESDMA_Set_MIU_Path(FALSE,FALSE);
            AESDMA_Set_MIU2_Path(FALSE,TRUE); // miu0->miu2
            return TRUE;
        }

        if ((_u32MIU1_Addr <= *phy64InAddr) & (_u32MIU1_Addr <= *phy64InAddr+u32InLen) & (_u32MIU2_Addr > *phy64InAddr) & (_u32MIU2_Addr > *phy64InAddr+u32InLen) & (_u32MIU1_Addr > *phy64OutSAddr) & (_u32MIU1_Addr > *phyOutEAddr))
        {
            *phy64InAddr = *phy64InAddr - _u32MIU1_Addr;
            AESDMA_Set_MIU_Path(TRUE,FALSE);
            AESDMA_Set_MIU2_Path(FALSE,FALSE); // miu1->miu0
            return TRUE;
        }

        if ((_u32MIU1_Addr <= *phy64InAddr) & (_u32MIU1_Addr <= *phy64InAddr+u32InLen) & (_u32MIU2_Addr > *phy64InAddr) & (_u32MIU2_Addr > *phy64InAddr+u32InLen) & (_u32MIU1_Addr <= *phy64OutSAddr) & (_u32MIU1_Addr <= *phyOutEAddr) & (_u32MIU2_Addr > *phy64OutSAddr) & (_u32MIU2_Addr > *phyOutEAddr))
        {
            *phy64InAddr = *phy64InAddr - _u32MIU1_Addr;
            *phy64OutSAddr = *phy64OutSAddr - _u32MIU1_Addr;
            *phyOutEAddr = *phyOutEAddr - _u32MIU1_Addr;
            AESDMA_Set_MIU_Path(TRUE,TRUE);
            AESDMA_Set_MIU2_Path(FALSE,FALSE); // miu1->miu1
            return TRUE;
        }

        if ((_u32MIU1_Addr <= *phy64InAddr) & (_u32MIU1_Addr <= *phy64InAddr+u32InLen) & (_u32MIU2_Addr > *phy64InAddr) & (_u32MIU2_Addr > *phy64InAddr+u32InLen) & (_u32MIU2_Addr <= *phy64OutSAddr) & (_u32MIU2_Addr <= *phyOutEAddr))
        {
            *phy64InAddr = *phy64InAddr - _u32MIU1_Addr;
            *phy64OutSAddr = *phy64OutSAddr - _u32MIU2_Addr;
            *phyOutEAddr = *phyOutEAddr - _u32MIU2_Addr;
            AESDMA_Set_MIU_Path(TRUE,FALSE);
            AESDMA_Set_MIU2_Path(FALSE,TRUE); // miu1->miu2
            return TRUE;
        }

        if ((_u32MIU2_Addr <= *phy64InAddr) & (_u32MIU2_Addr <= *phy64InAddr+u32InLen) & (_u32MIU1_Addr > *phy64OutSAddr) & (_u32MIU1_Addr > *phyOutEAddr))
        {
            *phy64InAddr = *phy64InAddr - _u32MIU2_Addr;
            AESDMA_Set_MIU_Path(FALSE,FALSE);
            AESDMA_Set_MIU2_Path(TRUE,FALSE); // miu2->miu0
            return TRUE;
        }

        if ((_u32MIU2_Addr <= *phy64InAddr) & (_u32MIU2_Addr <= *phy64InAddr+u32InLen) & (_u32MIU1_Addr <= *phy64OutSAddr) & (_u32MIU1_Addr <= *phyOutEAddr) & (_u32MIU2_Addr > *phy64OutSAddr) & (_u32MIU2_Addr > *phyOutEAddr))
        {
            *phy64InAddr = *phy64InAddr - _u32MIU2_Addr;
            *phy64OutSAddr = *phy64OutSAddr - _u32MIU1_Addr;
            *phyOutEAddr = *phyOutEAddr - _u32MIU1_Addr;
            AESDMA_Set_MIU_Path(FALSE,TRUE);
            AESDMA_Set_MIU2_Path(TRUE,FALSE); // miu2->miu1
            return TRUE;
        }

        if ((_u32MIU2_Addr <= *phy64InAddr) & (_u32MIU2_Addr <= *phy64InAddr+u32InLen) & (_u32MIU2_Addr <= *phy64OutSAddr) & (_u32MIU2_Addr <= *phyOutEAddr))
        {
            *phy64InAddr = *phy64InAddr - _u32MIU2_Addr;
            *phy64OutSAddr = *phy64OutSAddr - _u32MIU2_Addr;
            *phyOutEAddr = *phyOutEAddr - _u32MIU2_Addr;
            AESDMA_Set_MIU_Path(FALSE,FALSE);
            AESDMA_Set_MIU2_Path(TRUE,TRUE); // miu2->miu2
            return TRUE;
        }
        return FALSE;
    }
    else if (_u32MIU_num == 2)
    {
        if ((_u32MIU1_Addr > *phy64InAddr) & (_u32MIU1_Addr > *phy64InAddr+u32InLen) & (_u32MIU1_Addr > *phy64OutSAddr) & (_u32MIU1_Addr > *phyOutEAddr))
        {
            AESDMA_Set_MIU_Path(FALSE,FALSE); // miu0->miu0
            return TRUE;
        }

        if ((_u32MIU1_Addr < *phy64InAddr) & (_u32MIU1_Addr < *phy64InAddr+u32InLen) & (_u32MIU1_Addr < *phy64OutSAddr) & (_u32MIU1_Addr < *phyOutEAddr))
        {
            *phy64InAddr = *phy64InAddr - _u32MIU1_Addr;
            *phy64OutSAddr = *phy64OutSAddr - _u32MIU1_Addr;
            *phyOutEAddr = *phyOutEAddr - _u32MIU1_Addr;
            AESDMA_Set_MIU_Path(TRUE,TRUE); // miu1->miu1
            return TRUE;
        }

        if ((_u32MIU1_Addr > *phy64InAddr) & (_u32MIU1_Addr > *phy64InAddr+u32InLen) & (_u32MIU1_Addr <= *phy64OutSAddr) & (_u32MIU1_Addr <= *phyOutEAddr))
        {
            *phy64OutSAddr = *phy64OutSAddr - _u32MIU1_Addr;
            *phyOutEAddr = *phyOutEAddr - _u32MIU1_Addr;
            AESDMA_Set_MIU_Path(FALSE,TRUE); // miu0->miu1
            return TRUE;
        }

        if ((_u32MIU1_Addr <= *phy64InAddr) & (_u32MIU1_Addr <= *phy64InAddr+u32InLen) & (_u32MIU1_Addr > *phy64OutSAddr) & (_u32MIU1_Addr > *phyOutEAddr))
        {
            *phy64InAddr = *phy64InAddr - _u32MIU1_Addr;
            AESDMA_Set_MIU_Path(TRUE,FALSE); // miu1->miu0
            return TRUE;
        }
        return FALSE;
    }
    else
    {
        if ((_u32MIU0_Addr < *phy64InAddr) & (_u32MIU0_Addr < *phy64InAddr+u32InLen) & (_u32MIU0_Addr < *phy64OutSAddr) & (_u32MIU0_Addr < *phyOutEAddr))
        {
            AESDMA_Set_MIU_Path(FALSE,FALSE); // miu0->miu0

        }
        return TRUE;
    }
}

#if !defined(MSOS_TYPE_NOS)
static void _AESDMA_Isr(void)
{
    MS_U32                 u32AESDMAStatus;

    // Direct register access
    u32AESDMAStatus = AESDMA_Get_AESDMA_Status();

    if (HAS_FLAG(u32AESDMAStatus, AESDMA_STATES_GROUP))
    {
        _AESDMA_SetEvent(HAS_FLAG(u32AESDMAStatus, AESDMA_STATES_GROUP)); // Trigger task
    }
    AESDMA_Clear_Int();
    AESDMA_Enable_Int();
    AESDMAIntEnable();
}

static void _AESDMA_Isr_Proc(void)
{
    MS_U32              u32Events;

    while ( _bExitIsrLoop == FALSE )
    {
        _AESDMA_GetEvent(u32Events);
        if (u32Events & AESDMA_STATES_GROUP)
        {
            if (HAS_FLAG(u32Events, AESDMA_STATES_GROUP) && _AESDMAEvtNotify && _AESDMACallback)
            {
                switch (u32Events & AESDMA_STATES_GROUP)
                {
                    case AESDMA_DMA_DONE:
                        if (HAS_FLAG(_AESDMAEvtNotify, E_DRVAESDMA_EVENT_DMA_DONE))
                        {
                            MsOS_ReadMemory();
                            _AESDMACallback(E_DRVAESDMA_EVENT_DMA_DONE);
                        }
                        break;
                    case AESDMA_DMA_PAUSE:
                        if (HAS_FLAG(_AESDMAEvtNotify, E_DRVAESDMA_EVENT_DMA_PAUSE))
                        {
                            _AESDMACallback(E_DRVAESDMA_EVENT_DMA_PAUSE);
                        }
                        break;
                   default:
                        break;
                }
            }
        }
    } // Task while loop
}
#endif //#if !defined(MSOS_TYPE_NOS)

MS_U32 _AESDMA_MSBF2Native(const MS_U8 u8Bytes[4])
{
    return (u8Bytes[0] << 24) | (u8Bytes[1] << 16) | (u8Bytes[2] << 8) | u8Bytes[3];
}

//--------------------------------------------------------------------------------------------------
//  Global Function
//--------------------------------------------------------------------------------------------------
#ifndef MSOS_TYPE_NUTTX
DRVAESDMA_RESULT MDrv_AESDMA_SetSecurityInfo(DrvAESDMA_SecureInfo *pSecureInfo)
{
    bSecureModeEnable = pSecureInfo->bSecureModeEnable;
    return DRVAESDMA_OK;
}
#endif

DRVAESDMA_RESULT _MDrv_AESDMA_Init(MS_PHY phy64miu0addr , MS_PHY phy64miu1addr , MS_U32 u32miunum)
{
    MS_VIRT u32Bank, u32IRBank;
    MS_PHY u32BankSize;  // Non-PM bank
    MS_PHY u32IrBankSize;  // PM bank

    _bExitIsrLoop  = FALSE;

    if(_u8IsInit == FALSE)
    {
        if (FALSE == MDrv_MMIO_GetBASE(&u32Bank, &u32BankSize, MS_MODULE_BDMA))
        {
            MS_DEBUG_MSG(ULOGE(TAG_AESDMA, "MDrv_MMIO_GetBASE (NonPM base)fail\n"));
            MS_ASSERT(0);
        }

        if (FALSE == MDrv_MMIO_GetBASE(&u32IRBank, &u32IrBankSize, MS_MODULE_IR))
        {
            MS_DEBUG_MSG(ULOGE(TAG_AESDMA, "MDrv_MMIO_GetBASE (PM base)fail\n"));
            MS_ASSERT(0);
        }
        _u8IsInit = TRUE;
        HAL_AESDMA_SetBank(u32Bank, u32IRBank);
        AESDMA_Enable_Clk();
        _u32MIU_num = u32miunum;
        _u32MIU0_Addr = 0;
        if(_u32MIU_num == 2)
        {
            _u32MIU1_Addr = AESDMA_GetMIU1Base();
        }

        if(_u32MIU_num == 3)
        {
            _u32MIU1_Addr = AESDMA_GetMIU1Base();
            _u32MIU2_Addr = AESDMA_GetMIU2Base();
        }

        // ++ Release reset that asserted in the DC off
        // *(volatile MS_U32*)(0xbf800000+(0x2380<<2)) |= BIT15;
        // -- Release reset that asserted in the DC off

        #ifndef MSOS_TYPE_NUTTX
        if(bSecureModeEnable == FALSE)
        {
            AESDMA_Reset();
            AESDMA_MultEng_NormalReset(E_DRVAESDMA_ENG1);
        }
        #else
        AESDMA_Reset();
        AESDMA_MultEng_NormalReset(E_DRVAESDMA_ENG1);
        #endif

#if !defined(MSOS_TYPE_NOS)
        _AESDMAEvtNotify = E_DRVAESDMA_EVENT_DATA_INIT;
        _AESDMACallback = NULL;

        AESDMA_HW_Patch();

        AESDMAIntAttach();
        AESDMAIntEnable();

        AESDMA_Enable_Int();

#if AESDMS_SECURED_BANK_EN
        //for AESDMA SEMEPHONE
        if ( _s32AESDMAMutexId < 0)
            _s32AESDMAMutexId = MsOS_CreateMutex(E_MSOS_FIFO, (char*)"AESDMA Mutex", MSOS_PROCESS_SHARED);
#endif

        if (_s32AESDMAEventId < 0)
            _s32AESDMAEventId = MsOS_CreateEventGroup("AESDMA_Event");

        if (_s32AESDMAEventId < 0)
        {
#if AESDMS_SECURED_BANK_EN
            MsOS_DeleteMutex(_s32AESDMAMutexId);
#endif
            _u32LastErr = DRVAESDMA_FAIL;
            return _u32LastErr;
        }

        _pAESDMATaskStack = _u8AESDMA_StackBuffer;
        if (_s32AESDMATaskId < 0)
            _s32AESDMATaskId = MsOS_CreateTask((TaskEntry)_AESDMA_Isr_Proc, NULL, E_TASK_PRI_MEDIUM, TRUE, _pAESDMATaskStack, AESDMA_TASK_STACK_SIZE, "AESDMA_ISR_Task");

        if (_s32AESDMATaskId < 0)
        {
            MsOS_DeleteEventGroup(_s32AESDMAEventId);
#if AESDMS_SECURED_BANK_EN
            MsOS_DeleteMutex(_s32AESDMAMutexId);
#endif
            _u32LastErr = DRVAESDMA_FAIL;
            return _u32LastErr;
        }
#endif //#if !defined(MSOS_TYPE_NOS)
    }
    else
    {
        AESDMA_Enable_Clk();
        #ifndef MSOS_TYPE_NUTTX
        if(bSecureModeEnable == FALSE)
        {
            AESDMA_Reset();
            AESDMA_MultEng_NormalReset(E_DRVAESDMA_ENG1);
        }
        #else
        AESDMA_Reset();
        AESDMA_MultEng_NormalReset(E_DRVAESDMA_ENG1);
        #endif

        _u8IsInit = TRUE;
    }

#if AESDMS_SECURED_BANK_EN
    AESDMA_Set_SecureBankEnable (TRUE);
#else
    AESDMA_Set_SecureBankEnable (FALSE);
#endif

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_Init(MS_PHY u32miu0addr , MS_PHY u32miu1addr , MS_U32 u32miunum)
{
#if (AESDMA_UTOPIA20)
    AESDMA_INIT sAesdmaInit;

    sAesdmaInit.u32miu0addr = u32miu0addr;
    sAesdmaInit.u32miu1addr = u32miu1addr;
    sAesdmaInit.u32miunum = u32miunum;

#ifndef CONFIG_MBOOT
    if (pInstantAesdma != NULL) // prevent memory leak, and avoid strange issue in mboot after using same instance for a long time
    {
        UtopiaClose(pInstantAesdma);
        pInstantAesdma = NULL;
    }
#endif

    if(UtopiaOpen(MODULE_AESDMA, &pInstantAesdma, 0, pAttributeAesdma) !=  UTOPIA_STATUS_SUCCESS)
    {
        pInstantAesdma = NULL;
        return DRVAESDMA_FAIL;
    }

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_Init,(void*)&sAesdmaInit) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_AESDMA_Init(u32miu0addr, u32miu1addr, u32miunum);
#endif
}
//--------------------------------------------------------------------------------------------------
/// Set key to the AESDMA
/// @param pu8Key \b IN: The key is represented as vectors of bytes with the MSB first
/// @param u32Len \b IN: The byte length of the pu8Key. 16 for AES/TDES and 8 for DES
/// @return DRVAESDMA_OK - Success
/// @return DRVAESDMA_INVALID_PARAM - Invalid key length
//--------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT MDrv_AESDMA_SetKey_Ex(const MS_U8 *pu8Key, MS_U32 u32Len)
{
    MS_U32 u32Tmp[4] = {0};

    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if(pu8Key == NULL)
    {
        AESDMA_Sel_Key(TRUE); // key from KL
        return DRVAESDMA_OK;
    }

    // key from cipherkey
    if(u32Len == 16)
    {
        u32Tmp[3] = _AESDMA_MSBF2Native(pu8Key);
        u32Tmp[2] = _AESDMA_MSBF2Native(pu8Key + 4);
        u32Tmp[1] = _AESDMA_MSBF2Native(pu8Key + 8);
        u32Tmp[0] = _AESDMA_MSBF2Native(pu8Key + 12);
    }
    else if(u32Len == 8)
    {
        u32Tmp[3] = _AESDMA_MSBF2Native(pu8Key);
        u32Tmp[2] = _AESDMA_MSBF2Native(pu8Key + 4);
    }
    else
    {
        return DRVAESDMA_INVALID_PARAM;
    }

    AESDMA_Set_CipherKey(u32Tmp);
    AESDMA_Sel_Key(FALSE);
    return DRVAESDMA_OK;
}

//--------------------------------------------------------------------------------------------------
/// Set IV to the AESDMA
/// @param pu8IV \b IN: The IV is represented as vectors of bytes with the MSB first
/// @param u32Len \b IN: The byte length of the pu8IV. 16 for AES and 8 for TDES/DES
/// @return DRVAESDMA_OK - Success
/// @return DRVAESDMA_INVALID_PARAM - Invalid IV length
//--------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT MDrv_AESDMA_SetIV_Ex(const MS_U8 *pu8IV, MS_U32 u32Len)
{
    MS_U32 u32Tmp[4] = {0};

    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if(pu8IV == NULL)
    {
        return DRVAESDMA_OK;
    }

    if(u32Len == 16)
    {
        u32Tmp[3] = _AESDMA_MSBF2Native(pu8IV);
        u32Tmp[2] = _AESDMA_MSBF2Native(pu8IV + 4);
        u32Tmp[1] = _AESDMA_MSBF2Native(pu8IV + 8);
        u32Tmp[0] = _AESDMA_MSBF2Native(pu8IV + 12);
    }
    else if(u32Len == 8)
    {
        u32Tmp[3] = _AESDMA_MSBF2Native(pu8IV);
        u32Tmp[2] = _AESDMA_MSBF2Native(pu8IV + 4);
    }
    else
    {
        return DRVAESDMA_INVALID_PARAM;
    }

    AESDMA_Set_InitVector(u32Tmp);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetKey(MS_U32 *pCipherKey)
{
    if(u32ProtEng0 == 1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if (pCipherKey != NULL)
    {
        AESDMA_Set_CipherKey(pCipherKey);
        AESDMA_Sel_Key(FALSE); // key from cipherkey
    }
    else
    {
        AESDMA_Sel_Key(TRUE); // key from KL
    }

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetKeySel(MS_U32 *pCipherKey, DrvAESDMA_KEY_TYPE stKeyType)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Parser_Enable_Two_Keys(TRUE);
    switch (stKeyType)
    {
    case E_DRVAESDMA_ODD_KEY:
        AESDMA_Set_CipherOddKey(pCipherKey);
        break;

    case E_DRVAESDMA_EVEN_KEY:
        AESDMA_Set_CipherEvenKey(pCipherKey);
        break;

    default:
        break;
    }

    return DRVAESDMA_OK;
}

//--------------------------------------------------------------------------------------------------
/// Set Odd/Even Key to the AESDMA
/// @param pCipherKey \b IN: The key is represented as vectors of 4-bytes
/// @param stKeyType \b IN: The key type of input key.
/// @return DRVAESDMA_OK - Success
//--------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT MDrv_AESDMA_SetKeySel(MS_U32 *pCipherKey, DrvAESDMA_KEY_TYPE stKeyType)
{
#ifndef MSOS_TYPE_NUTTX
    if(bSecureModeEnable)
    {
        return DRVAESDMA_OK;
    }
    else
    {
        AESDMA_Parser_Enable_Two_Keys(TRUE);
        return _MDrv_AESDMA_SetKeySel(pCipherKey, stKeyType);
    }
#else
    AESDMA_Parser_Enable_Two_Keys(TRUE);
    return _MDrv_AESDMA_SetKeySel(pCipherKey, stKeyType);
#endif
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetOddIV(MS_U32 *pInitVector)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if (pInitVector != NULL)
    {
        AESDMA_Set_OddInitVector(pInitVector);
        return DRVAESDMA_OK;
    }
    else
    {
        return DRVAESDMA_INVALID_PARAM;
    }
}

DRVAESDMA_RESULT MDrv_AESDMA_SetOddIV(MS_U32 *pInitVector)
{
#ifndef MSOS_TYPE_NUTTX
    if(bSecureModeEnable)
    {
        return DRVAESDMA_OK;
    }
    else
    {
        if (NULL == pInstantAesdma)
            return DRVAESDMA_FAIL;

        if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_SetOddIV, (void*)pInitVector) != UTOPIA_STATUS_SUCCESS)
        {
            return DRVAESDMA_FAIL;
        }
        return DRVAESDMA_OK;
    }
#else
    return _MDrv_AESDMA_SetOddIV(pInitVector);
#endif
}


DRVAESDMA_RESULT _MDrv_AESDMA_SetSecureKey(void)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Sel_SecureKey();
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetIV(MS_U32 *pInitVector)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if (pInitVector != NULL)
    {
        AESDMA_Set_InitVector(pInitVector);
        return DRVAESDMA_OK;
    }
    else
    {
        return DRVAESDMA_INVALID_PARAM;
    }
}

DRVAESDMA_RESULT MDrv_AESDMA_QueryCipherMode(DrvAESDMA_CipherMode eMode)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    return AESDMA_QueryCipherMode(eMode);
}

DRVAESDMA_RESULT _MDrv_AESDMA_SelEng(DrvAESDMA_CipherMode eMode, MS_BOOL bDescrypt)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    switch (eMode)
    {
        case E_DRVAESDMA_CIPHER_CTR:
        case E_DRVAESDMA_CIPHER_DES_CTR:
        case E_DRVAESDMA_CIPHER_TDES_CTR:
	        AESDMA_Set_CIPHER_ENG((AESDMA_CipherMode)eMode, FALSE);
            break;
        case E_DRVAESDMA_CIPHER_ECB:
        case E_DRVAESDMA_CIPHER_CBC:
        case E_DRVAESDMA_CIPHER_DES_ECB:
        case E_DRVAESDMA_CIPHER_DES_CBC:
        case E_DRVAESDMA_CIPHER_TDES_ECB:
        case E_DRVAESDMA_CIPHER_TDES_CBC:
        case E_DRVAESDMA_CIPHER_CTS_CBC:
        case E_DRVAESDMA_CIPHER_CTS_ECB:
        case E_DRVAESDMA_CIPHER_DES_CTS_CBC:
        case E_DRVAESDMA_CIPHER_DES_CTS_ECB:
        case E_DRVAESDMA_CIPHER_TDES_CTS_CBC:
        case E_DRVAESDMA_CIPHER_TDES_CTS_ECB:
            AESDMA_Set_CIPHER_ENG((AESDMA_CipherMode)eMode, bDescrypt);
            break;
        default:
            break;
    }
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetPS(MS_U32 u32PTN, MS_U32 u32Mask, MS_BOOL bPSin_Enable, MS_BOOL bPSout_Enable)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Set_PS_PTN(u32PTN);
    AESDMA_Set_PS_Mask(u32Mask);
    AESDMA_Set_PS_ENG(bPSin_Enable, bPSout_Enable);
    AESDMA_Enable_Int();

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_PSRelease(void)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Set_PS_Release();
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetFileInOut(MS_PHY phy64FileinAddr, MS_U32 u32FileInNum, MS_PHY phy64FileOutSAddr, MS_PHY phy64FileOutEAddr)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    // check miu addr
    if (!_AESDMA_Chk_MIU(&phy64FileinAddr, u32FileInNum, &phy64FileOutSAddr, &phy64FileOutEAddr))
    {
        return DRVAESDMA_MIU_ADDR_ERROR;
    }

    if ((phy64FileOutSAddr == 0) | (phy64FileOutEAddr == 0))
    {
        AESDMA_Set_FileinDesc(phy64FileinAddr , u32FileInNum);
    }
    else
    {
        AESDMA_Set_FileinDesc(phy64FileinAddr, u32FileInNum);
        AESDMA_Set_FileoutDesc(phy64FileOutSAddr, phy64FileOutEAddr);
    }

    phyTmpFileInAddr = phy64FileinAddr;
    u32TmpFileInNum = u32FileInNum;
    phyTmpFileOutAddr = phy64FileOutSAddr;

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_Start(MS_BOOL bStart)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if (!MsOS_Dcache_Flush(MsOS_PA2KSEG0(phyTmpFileInAddr), u32TmpFileInNum))
        MsOS_Dcache_Flush(MsOS_PA2KSEG0(phyTmpFileInAddr) & ~((MS_VIRT)0x0F), u32TmpFileInNum+32);
    if (!MsOS_Dcache_Flush(MsOS_PA2KSEG0(phyTmpFileOutAddr), u32TmpFileInNum))
        MsOS_Dcache_Flush(MsOS_PA2KSEG0(phyTmpFileOutAddr) & ~((MS_VIRT)0x0F), u32TmpFileInNum+32);

    MsOS_FlushMemory();
    AESDMA_Start(bStart);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_NormalReset(void)
{
#ifndef MSOS_TYPE_NUTTX
    if(bSecureModeEnable)
    {
        return DRVAESDMA_OK;
    }
    else
    {
        AESDMA_NormalReset();
        AESDMA_HW_Patch();
        AESDMA_Enable_Int();
        AESDMA_Parser_Enable_HWParser(FALSE);
        return DRVAESDMA_OK;
    }
#else
    AESDMA_NormalReset();
    AESDMA_HW_Patch();
    AESDMA_Enable_Int();
    AESDMA_Parser_Enable_HWParser(FALSE);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT _MDrv_AESDMA_Reset(void)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Reset();
    AESDMA_HW_Patch();
    AESDMA_Enable_Int();
    AESDMA_Parser_Enable_HWParser(FALSE);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_GetStatus(MS_U32 *u32RetVal)
{
    *u32RetVal = AESDMA_Get_AESDMA_Status();

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_IsFinished(MS_U32 *u32RetVal)
{
    if(AESDMA_Get_AESDMA_IsFinished())
    {
        *u32RetVal = DRVAESDMA_OK;
        MsOS_ReadMemory();
        return DRVAESDMA_OK;
    }
    else
    {
        *u32RetVal = DRVAESDMA_FAIL;
        return DRVAESDMA_FAIL;
    }
}

MS_U32 _MDrv_AESDMA_GetPSMatchedByteCNT(MS_U32 *u32RetVal)
{
	MS_U32 u32Val = AESDMA_Get_PS_MatchedBytecnt();
	*u32RetVal = u32Val;
    return u32Val;
}

MS_U32 _MDrv_AESDMA_GetPSMatchedPTN(void)
{
    return AESDMA_Get_PS_MatchedPTN();
}

DRVAESDMA_RESULT _MDrv_AESDMA_Notify(DrvAESDMA_Event eEvents, P_DrvAESDMA_EvtCallback pfCallback)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if (pfCallback)
    {
        AESDMA_Enable_Int();
    }
    else
    {
        AESDMA_Disable_Int();
    }

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_SetDbgLevel(DrvAESDMA_DbgLevel DbgLevel)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    _u32AESDMADBGLevel = DbgLevel;
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_GetLibVer(const MSIF_Version **ppVersion)
{
    // No mutex check, it can be called before Init
    if (!ppVersion)
    {
        return DRVAESDMA_FAIL;
    }

    *ppVersion = &_drv_aesdma_version;

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_Rand(MS_U32 *u32PABuf, MS_U32 u32Size)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    MS_U32 Index;
    ptrdiff_t n = (ptrdiff_t)u32PABuf;
    MS_U8 *u8VABuf = (MS_U8 *)MsOS_PA2KSEG1((MS_PHY)n);
    MS_U8 data;

    if( u32Size > 0 )
        *u8VABuf = AESDMA_Rand();
    else
        return DRVAESDMA_FAIL;

    for( Index = 1; Index < u32Size; Index++ )
    {
        while( (data = AESDMA_Rand()) == *(u8VABuf + Index -1));
        *(u8VABuf + Index) = data;
    }
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_Get_Rand_Num(MS_U8 *pu8Num)
{

    *pu8Num = AESDMA_Rand();

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_Set_Clk(MS_BOOL benable)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if(benable)
    {
        AESDMA_Enable_Clk();
    }
    else
    {
        AESDMA_Disable_Clk();
    }

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_SHA_Calculate(DrvAESDMA_SHAMode eMode, MS_PHY phy64PAInBuf, MS_U32 u32Size, MS_PHY phy64PAOutBuf)
{
    MS_U32 u32OutOffset = 0;
    MS_U32 u32timeout_count = 0;
    MS_PHY u64PAOutBufE;

    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;
    switch (eMode)
    {
    case E_AESDMA_SHA1:
        u32OutOffset = 20; //Out 160-bit
        break;

    case E_AESDMA_SHA256:
        u32OutOffset = 32; //Out 256-bit
        break;

    default:
        return DRVAESDMA_INVALID_PARAM;
    }
#if 1
    // check miu addr
    u64PAOutBufE = (phy64PAOutBuf + u32OutOffset);
    if (!_AESDMA_Chk_MIU(&phy64PAInBuf, u32Size, &phy64PAOutBuf, &u64PAOutBufE))
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "@@@DRVAESDMA_MIU_ADDR_ERROR\n"));
        return DRVAESDMA_MIU_ADDR_ERROR;
    }
#endif
    // Clear all SHA setting(clear the last outputs)
    SHA_Reset();
    // Error handler for 16-byte alignment limitation
    if(phy64PAInBuf & WORD_MASK)
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "Input Address should be 16-byte alignment\n"));
        return DRVAESDMA_FAIL;
    }

    if( SHA_Calculate(eMode, phy64PAInBuf, u32Size, phy64PAOutBuf, _u32MIU1_Addr) != TRUE)
        return DRVAESDMA_FAIL;

    MsOS_DelayTaskUs(40);

    MsOS_FlushMemory();

    MsOS_DelayTaskUs(200);

    SHA_Start(1);

    while ((SHA_Get_Status() & SHARNG_CTRL_SHA_READY) == 0)
    {
        MsOS_DelayTaskUs(10);
        u32timeout_count++;
#if (SHA_WAIT==1)
        //500ms timeout
        if(u32timeout_count>SHA_TIMEOUT_VALUE)
        {
            SHA_Reset();
            return DRVAESDMA_FAIL;
        }
#endif
    }

    SHA_Out(MsOS_PA2KSEG1(phy64PAOutBuf));
    while ((SHA_Get_Status() & SHARNG_CTRL_SHA_BUSY) != 0)
        ;
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_SHA_CalculateManual(DrvAESDMA_HASHCFG stCfg, DrvAESDMA_HASH_STAGE eStage, MS_U32 u32DoneBytes, MS_U8 *pu8SetIV)
{

    //not implement yet
    return DRVAESDMA_OK;
}

#ifdef MOBF_ENABLE
//-------------------------------------------------------------------------------------------------
/// MOBF BDMA without any Encrypt (sw patch)
///

/// @return AESDMA_Result
/// @note Must be called after MDrv_AESDMA_SelEng(E_DRVAESDMA_CIPHER_DES_CTR,1)
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT MDrv_MOBF_DmaOnly(void)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    MOBF_DisableTDES();
    return DRVAESDMA_OK;
}

//-------------------------------------------------------------------------------------------------
/// Set MOBF encrypt (from AESDMA to MIU)
/// @param  u32Key                \b IN: Encrypt Key
/// @param  bEnable                \b IN: Encrypt Enable/disable

/// @return AESDMA_Result
/// @note
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT MDrv_MOBF_Encrypt(MS_U32 u32Key, MS_BOOL bEnable)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    MOBF_WriteKey(u32Key);
    MOBF_WriteEnable(bEnable);

    return DRVAESDMA_OK;
}

//-------------------------------------------------------------------------------------------------
/// Set MOBF decrypt (from MIU to AESDMA)
/// @param  u32Key                \b IN: Decrypt Key
/// @param  bEnable                \b IN: Decrypt Enable/disable

/// @return AESDMA_Result
/// @note
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT MDrv_MOBF_Decrypt(MS_U32 u32Key, MS_BOOL bEnable)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    MOBF_ReadKey(u32Key);
    MOBF_ReadEnable(bEnable);

    return DRVAESDMA_OK;
}

//-------------------------------------------------------------------------------------------------
/// Enable MOBF One Way
/// @return AESDMA_Result
/// @note disable read mobf key
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT MDrv_MOBF_OneWay(void)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    MOBF_OneWay();
    return DRVAESDMA_OK;
}

#endif

//-------------------------------------------------------------------------------------------------
/// Mask Scrmb bit(for decrypt using)
/// @param  bEnable                \b IN: Mask Scrmb Enable/disable
/// @return AESDMA_Result
/// @note mask Scrmb bit
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT _MDrv_AESDMA_Parser_MaskScrmb(MS_BOOL bEnable)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Parser_Mask_Scrmb(bEnable);
    return DRVAESDMA_OK;
}

//-------------------------------------------------------------------------------------------------
/// Set Scrmb Pattern
/// @param  ePattern                \b IN: input Scrmb pattern(10/11)
/// @return AESDMA_Result
/// @note set Scrmb Pattern
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT _MDrv_AESDMA_Parser_SetScrmbPattern(DrvAESDMA_ScrmbPattern ePattern)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Parser_Set_ScrmbPattern(ePattern);
    return DRVAESDMA_OK;
}

//-------------------------------------------------------------------------------------------------
/// Set Scrmb Pattern
/// @param  ePattern                \b IN: input Scrmb pattern(10/11)
/// @return AESDMA_Result
/// @note set Scrmb Pattern
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT _MDrv_AESDMA_Parser_SetAddedScrmbPattern(DrvAESDMA_ScrmbPattern ePattern)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Parser_Set_AddedScrmbPattern(ePattern);
    return DRVAESDMA_OK;
}

//-------------------------------------------------------------------------------------------------
/// Query Pid Count
/// @return Pid Count
/// @note query pid count
//-------------------------------------------------------------------------------------------------
MS_U8 _MDrv_AESDMA_Parser_QueryPidCount(MS_U8* retCount)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    *retCount = AESDMA_Parser_Query_PidCount();
    return *retCount;
}

//-------------------------------------------------------------------------------------------------
/// Set Pid
/// @param  u8Index                 \b IN: Pid Index(0/1)
/// @param  u16Pid                   \b IN: Pid Value(max value is 0x1FFF)
/// @return AESDMA_Result
/// @note set Pid
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT _MDrv_AESDMA_Parser_SetPid(MS_U8 u8Index, MS_U16 u16Pid)
{
    MS_U8 u8PidCount;
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    // check Pid value
    if (u16Pid > AESDMA_PARSER_PID_MAX)
    {
        return DRVAESDMA_INVALID_PARAM;
    }

    // check Pid count
    u8PidCount = AESDMA_Parser_Query_PidCount();
    if (u8Index >= u8PidCount)
    {
        return DRVAESDMA_INVALID_PARAM;
    }

    AESDMA_Parser_Set_Pid(u8Index,u16Pid);
    return DRVAESDMA_OK;
}

//-------------------------------------------------------------------------------------------------
/// Bypass Pid
/// @param  bEnable                \b IN: Bypass Pid Enable/disable
/// @return AESDMA_Result
/// @note bypass Pid
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT _MDrv_AESDMA_Parser_BypassPid(MS_BOOL bEnable)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Parser_Bypass_Pid(bEnable);
    return DRVAESDMA_OK;
}

//-------------------------------------------------------------------------------------------------
/// Init Parser Encrypt
/// @param  eMode                \b IN: input parser mode
/// @return AESDMA_Result
/// @note init parser encrypt
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT _MDrv_AESDMA_Parser_Encrypt(DrvAESDMA_ParserMode eMode)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Parser_Set_Mode((AESDMA_ParserMode)eMode);
    switch (eMode)
    {
    case E_DRVAESDMA_PARSER_TS_PKT192:
    case E_DRVAESDMA_PARSER_TS_PKT192_CLEAR:
    case E_DRVAESDMA_PARSER_TS_PKT188:
    case E_DRVAESDMA_PARSER_TS_PKT188_CLEAR:
        AESDMA_Parser_Insert_Scrmb(TRUE);
        AESDMA_Parser_Remove_Scrmb(FALSE);
        break;

    case E_DRVAESDMA_PARSER_HDCP20_PKT192:
    case E_DRVAESDMA_PARSER_HDCP20_PKT192_CLEAR:
    case E_DRVAESDMA_PARSER_HDCP20_PKT188:
    case E_DRVAESDMA_PARSER_HDCP20_PKT188_CLEAR:
        AESDMA_Parser_Insert_Scrmb(FALSE);
        AESDMA_Parser_Remove_Scrmb(FALSE);
        break;
    }

    AESDMA_Parser_Enable_HWParser(TRUE);
    return DRVAESDMA_OK;
}

//-------------------------------------------------------------------------------------------------
/// Init Parser Decrypt
/// @param  eMode                \b IN: input parser mode
/// @return AESDMA_Result
/// @note init parser decrypt
//-------------------------------------------------------------------------------------------------
DRVAESDMA_RESULT _MDrv_AESDMA_Parser_Decrypt(DrvAESDMA_ParserMode eMode)
{
    if(u32ProtEng0 == 1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_Parser_Set_Mode((AESDMA_ParserMode)eMode);

    switch (eMode)
    {
    case E_DRVAESDMA_PARSER_TS_PKT192:
    case E_DRVAESDMA_PARSER_TS_PKT192_CLEAR:
    case E_DRVAESDMA_PARSER_TS_PKT188:
    case E_DRVAESDMA_PARSER_TS_PKT188_CLEAR:
        AESDMA_Parser_Insert_Scrmb(FALSE);
        AESDMA_Parser_Remove_Scrmb(TRUE);
        break;

    case E_DRVAESDMA_PARSER_HDCP20_PKT192:
    case E_DRVAESDMA_PARSER_HDCP20_PKT192_CLEAR:
    case E_DRVAESDMA_PARSER_HDCP20_PKT188:
    case E_DRVAESDMA_PARSER_HDCP20_PKT188_CLEAR:
        AESDMA_Parser_Insert_Scrmb(FALSE);
        AESDMA_Parser_Remove_Scrmb(FALSE);
        break;
    }


    AESDMA_Parser_Enable_HWParser(TRUE);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_RSA_Calculate(DrvAESDMA_RSASig *pstSign, DrvAESDMA_RSAKey *pstKey, DrvAESDMA_RSAMode eMode)
{
    MS_U32 u32KeyLen = 0;
    MS_U8  u8Public = 0;
    MS_U32 *pu32Sig = (MS_U32 *)((void*)pstSign);
    MS_U32 *pu32Key = (MS_U32 *)((void*)pstKey);


    if(u32ProtEng0 == 1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

	RSA_Reset();
	RSA_Ind32Ctrl(1);//ind32_ctrl=0xE0

    switch (eMode)
    {
    case E_DRVAESDMA_RSA1024_PUBLIC:
        {
            u32KeyLen = 128;
            u8Public = 1;
        }
        break;
    case E_DRVAESDMA_RSA1024_PRIVATE:
        {
            u32KeyLen = 128;
            u8Public = 0;
        }
        break;
    case E_DRVAESDMA_RSA2048_PUBLIC:
        {
            u32KeyLen = 256;
            u8Public = 1;
        }
        break;
    case E_DRVAESDMA_RSA2048_PRIVATE:
        {
            u32KeyLen = 256;
            u8Public = 0;
        }
        break;
    case E_DRVAESDMA_RSA256_PUBLIC:
        {
            u32KeyLen = 32;
            u8Public = 1;
        }
        break;
    case E_DRVAESDMA_RSA256_PRIVATE:
        {
            u32KeyLen = 32;
            u8Public = 0;
        }
        break;
    default:
        return DRVAESDMA_INVALID_PARAM;
    }

	RSA_LoadSram((pu32Sig+(RSA_INPUT_SIZE/RSA_UNIT_SIZE)-1),E_RSA_ADDRESS_A);
    RSA_LoadSram((pu32Key+(RSA_INPUT_SIZE/RSA_UNIT_SIZE)-1),E_RSA_ADDRESS_N);
    RSA_LoadSram((pu32Key+(RSA_INPUT_SIZE/RSA_UNIT_SIZE)),E_RSA_ADDRESS_E);

    RSA_SetKeyLength((u32KeyLen/RSA_UNIT_SIZE)-1);
    RSA_SetKeyType(0,u8Public); //sw key, public key

	RSA_ExponetialStart();

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_RSA_Calculate_Hw_Key(DrvAESDMA_RSASig *pstSign, DrvAESDMA_RSAHwKeyMode eMode)
{
    return DRVAESDMA_FAIL;
}

DRVAESDMA_RESULT _MDrv_RSA_IsFinished(void)
{
    if(!RSA_Get_RSA_IsFinished())
    {
        return DRVAESDMA_OK;
    }
    else
    {
        return DRVAESDMA_FAIL;
    }
}

DRVAESDMA_RESULT _MDrv_RSA_Output(DrvAESDMA_RSAMode eMode, DrvAESDMA_RSAOut *pstRSAOut)
{
    MS_U32 i = 0;
    MS_U32 u32KeyLen = 0;

    MS_U32 *pu32RSAOut = (MS_U32 *)((void*)pstRSAOut);

    if(u32ProtEng0 == 1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    RSA_Ind32Ctrl(0);//ind32_ctrl=0xC0

    switch (eMode)
    {
    case E_DRVAESDMA_RSA1024_PUBLIC:
    case E_DRVAESDMA_RSA1024_PRIVATE:
        u32KeyLen = 128;
        break;
    case E_DRVAESDMA_RSA2048_PUBLIC:
    case E_DRVAESDMA_RSA2048_PRIVATE:
        u32KeyLen = 256;
        break;
    case E_DRVAESDMA_RSA256_PUBLIC:
    case E_DRVAESDMA_RSA256_PRIVATE:
        u32KeyLen = 32;
        break;
    default:
        return DRVAESDMA_INVALID_PARAM;
    }

	for( i = 0; i<(u32KeyLen/RSA_UNIT_SIZE); i++)
	{
		RSA_SetFileOutAddr(i);
		RSA_FileOutStart();
		*(pu32RSAOut+i) = RSA_FileOut();
	}

    //clear the used key whenever calculating is finished
    RSA_Reset();

    return DRVAESDMA_OK;
}

MS_U32 MDrv_AESDMA_SetPowerState(EN_POWER_MODE u16PowerState)
{
    static EN_POWER_MODE u16PreAESDMAPowerState = E_POWER_MECHANICAL;
    MS_U32 u32Return = UTOPIA_STATUS_FAIL;

    if (u16PowerState == E_POWER_SUSPEND)
    {
        u16PreAESDMAPowerState = u16PowerState;
	_u8IsInit = FALSE;
        u32Return = UTOPIA_STATUS_SUCCESS;//SUSPEND_OK;

        AESDMA_POWER_SUSPEND();

#if !defined(MSOS_TYPE_NOS)
        AESDMAIntDisable();
        AESDMAIntDetach();
#endif //#if !defined(MSOS_TYPE_NOS)
        _bExitIsrLoop = TRUE;
    }
    else if (u16PowerState == E_POWER_RESUME)
    {
        if (u16PreAESDMAPowerState == E_POWER_SUSPEND)
        {
            MDrv_AESDMA_Init(0, 0x20000000, 2);

            AESDMA_POWER_RESUME();

            u16PreAESDMAPowerState = u16PowerState;
            u32Return = UTOPIA_STATUS_SUCCESS;//RESUME_OK;
        }
        else
        {
            ULOGE(TAG_AESDMA, "[%s,%5d]It is not suspended yet. We shouldn't resume\n",__FUNCTION__,__LINE__);
            u32Return = UTOPIA_STATUS_FAIL;//SUSPEND_FAILED;
        }
    }
    else
    {
        ULOGE(TAG_AESDMA, "[%s,%5d]Do Nothing: %d\n",__FUNCTION__,__LINE__,u16PowerState);
        u32Return = UTOPIA_STATUS_FAIL;
    }

    return u32Return;// for success
}

//reserved for secure protect function
/*
DRVAESDMA_RESULT MDrv_AESDMA_SecureSetKey(MS_U32 *pCipherKey)
{
    if (pCipherKey != NULL)
    {
        AESDMA_Secure_SetCipherKey(pCipherKey);
        return DRVAESDMA_OK;
    }
    else
    {
        return DRVAESDMA_INVALID_PARAM;
    }
}

DRVAESDMA_RESULT MDrv_AESDMA_SecureProtect(MS_BOOL bEnable)
{
    AESDMA_SecureProtect(bEnable);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_SecureSetIV(MS_U32 *pInitVector)
{
    if (pInitVector != NULL)
    {
        AESDMA_Secure_SetInitVector(pInitVector);
        return DRVAESDMA_OK;
    }
    else
    {
        return DRVAESDMA_INVALID_PARAM;
    }
}

DRVAESDMA_RESULT MDrv_AESDMA_SetFileOutEnable(MS_BOOL bEnable)
{
    AESDMA_Set_FileOutEnable(bEnable);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_SecureStart(MS_BOOL bStart)
{
    MsOS_FlushMemory();
    AESDMA_Secure_Start(bStart);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_SecureIsFinished(void)
{
    if(AESDMA_Get_AESDMA_SecureIsFinished())
    {
        MsOS_ReadMemory();
        return DRVAESDMA_OK;
    }
    else
    {
        return DRVAESDMA_FAIL;
    }
}
*/

DRVAESDMA_RESULT _MDrv_AESDMA_IsSecretKeyInNormalBank(MS_U8 *retVal)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;
    *retVal = AESDMA_IsSecretKeyInNormalBank ();
    if (*retVal == 0)
    {
        return DRVAESDMA_FAIL;
    }
    else
    {
        return DRVAESDMA_OK;
    }
}

DRVAESDMA_RESULT MDrv_HDCP_ProcessCipher(MS_U8 u8Idx, MS_U8* pu8Riv, MS_U8 *pu8ContentKey)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    HDCP_ProcessCipher(u8Idx, pu8ContentKey, pu8Riv);

    return DRVAESDMA_OK;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function           \b Name: MDrv_HDCP_GetHdcpCipherState
/// @brief \b Function           \b Description: Check the capability of HDCP 2.2
/// @param None                  \b IN : u8Idx
/// @param None                  \b OUT : pu8State
/// @param DRVAESDMA_RESULT      \b RET : DRVAESDMA_OK
/// @param None                  \b GLOBAL :
////////////////////////////////////////////////////////////////////////////////
DRVAESDMA_RESULT MDrv_HDCP_GetHdcpCipherState(MS_U8 u8Idx, MS_U8 *pu8State)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    HDCP_GetHdcpCipherState(u8Idx, pu8State);

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetKeyIndex (MS_U32 u32Index, MS_BOOL bIsOddKey)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_SetKeyIndex (u32Index, bIsOddKey);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_EnableTwoKeys(MS_BOOL bEnable)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

	AESDMA_Parser_Enable_Two_Keys(bEnable);
	return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetOddKey(MS_U32 *pInitVector)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if (pInitVector != NULL)
    {
    	AESDMA_Set_CipherOddKey(pInitVector);
        return DRVAESDMA_OK;
    }
    else
    {
        return DRVAESDMA_INVALID_PARAM;
    }
}

DRVAESDMA_RESULT MDrv_HDMI_GetM0(MS_U32 u32InputPortType, MS_U8 *pu8Data)
{
    if (pu8Data == (MS_U8 *)NULL)
    {
        return DRVAESDMA_FAIL;
    }
    else
    {
        HAL_AESDMA_HDMI_GetM0(u32InputPortType, pu8Data);
        return DRVAESDMA_OK;
    }
}

DRVAESDMA_RESULT MDrv_Set_Kslot(MS_U32 u32InputPortType, MS_U8 *pu8Data)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    if (pu8Data == (MS_U8 *)NULL)
    {
        return DRVAESDMA_FAIL;
    }
    else
    {
        HAL_AESDMA_HDMI_GetM0(u32InputPortType, pu8Data);
        return DRVAESDMA_OK;
    }
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetAesCtr64 (MS_BOOL bCtr64En)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_SET_CTR64(bCtr64En);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT _MDrv_AESDMA_SetSecuredKeyIndex (MS_U8 u8KeyIdx)
{
    if(u32ProtEng0 == 0x1)  //Lock by API Calc
        return DRVAESDMA_FAIL;

    AESDMA_SetSecureKeyIdx (u8KeyIdx);
    return DRVAESDMA_OK;
}

#ifndef UTOPIAXP_REMOVE_WRAPPER
DRVAESDMA_RESULT MDrv_AESDMA_SetKey(MS_U32 *pCipherKey)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;
    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_SetKey,(void*)pCipherKey) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_AESDMA_SetKey(pCipherKey);
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_SetSecureKey(void)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;
    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_SetSecureKey,NULL) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    AESDMA_Sel_SecureKey();
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_SetIV(MS_U32 *pInitVector)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_SetIV,(void*)pInitVector) != UTOPIA_STATUS_SUCCESS)
    {
        MS_DEBUG_MSG(printf("Ioctl MDrv_AESDMA_SetIV fail\n"));
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_AESDMA_SetIV(pInitVector);
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_SelEng(DrvAESDMA_CipherMode eMode, MS_BOOL bDescrypt)
{
#if (AESDMA_UTOPIA20)
    AESDMA_SEL_ENG SelEngParam;
    SelEngParam.eMode= eMode;
    SelEngParam.bDescrypt= bDescrypt;
    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_SelEng,(void*)&SelEngParam) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_AESDMA_SelEng(eMode,bDescrypt);
#endif
    }

DRVAESDMA_RESULT MDrv_AESDMA_SetPS(MS_U32 u32PTN, MS_U32 u32Mask, MS_BOOL bPSin_Enable, MS_BOOL bPSout_Enable)
{
#if (AESDMA_UTOPIA20)
    AESDMA_SET_PS SetPsParam;
    SetPsParam.u32PTN = u32PTN;
    SetPsParam.u32Mask = u32Mask;
    SetPsParam.bPSin_Enable = bPSin_Enable;
    SetPsParam.bPSout_Enable = bPSout_Enable;
    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_SetPS,(void*)&SetPsParam) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    AESDMA_Set_PS_PTN(u32PTN);
    AESDMA_Set_PS_Mask(u32Mask);
    AESDMA_Set_PS_ENG(bPSin_Enable, bPSout_Enable);
    AESDMA_Enable_Int();
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_PSRelease(void)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;
    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_PSRelease,NULL) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    AESDMA_Set_PS_Release();
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_SetFileInOut(MS_PHY u32FileinAddr, MS_U32 u32FileInNum, MS_PHY u32FileOutSAddr, MS_PHY u32FileOutEAddr)
{
#if (AESDMA_UTOPIA20)
    AESDMA_FILE_INOUT FileInOutParam;
    FileInOutParam.u32FileinAddr= u32FileinAddr;
    FileInOutParam.u32FileInNum= u32FileInNum;
    FileInOutParam.u32FileOutSAddr= u32FileOutSAddr;
    FileInOutParam.u32FileOutEAddr= u32FileOutEAddr;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_SetFileInOut,(void*)&FileInOutParam) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_AESDMA_SetFileInOut(u32FileinAddr,u32FileInNum,u32FileOutSAddr,u32FileOutEAddr);
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Start(MS_BOOL bStart)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_Start,(void*)&bStart) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_AESDMA_Start(bStart);
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Reset(void)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_Reset,NULL) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }

    return DRVAESDMA_OK;
#else
    AESDMA_Reset();
    AESDMA_HW_Patch();
    AESDMA_Enable_Int();
    AESDMA_Parser_Enable_HWParser(FALSE);
    return DRVAESDMA_OK;
#endif
}

MS_U32 MDrv_AESDMA_GetStatus(void)
{
#if (AESDMA_UTOPIA20)
    MS_U32 u32RetVal = 0;
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;
    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_GetStatus,(void*)&u32RetVal) != UTOPIA_STATUS_SUCCESS)
    {
        return 0;
    }
    return u32RetVal;
#else
    return AESDMA_Get_AESDMA_Status();
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_IsFinished(void)
{
#if (AESDMA_UTOPIA20)
    MS_U32 u32TimeOut = AES_MAX_TIMEOUT_VALUE;
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_IsFinished, (void*)&u32TimeOut) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_AESDMA_IsFinished();
#endif
}

MS_U32 MDrv_AESDMA_GetPSMatchedByteCNT(void)
{
#if (AESDMA_UTOPIA20)
    MS_U32 u32Count = 0x000;

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_GetPSMatchedByteCNT, (void*)&u32Count) != UTOPIA_STATUS_SUCCESS)
    {
        return 0;
    }

    return u32Count;
#else
    return AESDMA_Get_PS_MatchedBytecnt();
#endif
}

MS_U32 MDrv_AESDMA_GetPSMatchedPTN(void)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    return UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_GetPSMatchedPTN,NULL);
#else
    return AESDMA_Get_PS_MatchedPTN();
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Notify(DrvAESDMA_Event eEvents, P_DrvAESDMA_EvtCallback pfCallback)
{
#if (AESDMA_UTOPIA20)
    AESDMA_NOTIFY stNotifyParam;

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    stNotifyParam.eEvents = eEvents;
    stNotifyParam.pfCallback = pfCallback;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_Notify, (void*)&stNotifyParam) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }

    _AESDMAEvtNotify = eEvents;
    _AESDMACallback = pfCallback;

    return DRVAESDMA_OK;
#else
    if (pfCallback)
    {
        AESDMA_Enable_Int();
    }
    else
    {
        AESDMA_Disable_Int();
    }

    _AESDMAEvtNotify = eEvents;
    _AESDMACallback = pfCallback;

    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Rand(MS_U32 *u32PABuf, MS_U32 u32Size)
{
#if (AESDMA_UTOPIA20)

    AESDMA_RAND stAesdmaRand;
    stAesdmaRand.u32PABuf = u32PABuf;
    stAesdmaRand.u32Size = u32Size;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_Rand, (void*)&stAesdmaRand) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;

#else
    MS_U32 Index;
    MS_U8 *u8VABuf = (MS_U8 *)MsOS_PA2KSEG1((MS_U32)u32PABuf);

    for( Index = 0; Index < u32Size; Index++ )
    {
        *(u8VABuf + Index) = AESDMA_Rand();
    }
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Set_Clk(MS_BOOL benable)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;
    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_SetClk,(void*)&benable) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    if(benable)
    {
        AESDMA_Enable_Clk();
    }
    else
    {
        AESDMA_Disable_Clk();
    }
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_SHA_Calculate(DrvAESDMA_SHAMode eMode, MS_PHY u32PAInBuf, MS_U32 u32Size, MS_PHY u32PAOutBuf)
{
#if (AESDMA_UTOPIA20)
    SHA_CALCULATE ShaCalParam;
    ShaCalParam.eMode= eMode;
    ShaCalParam.u32PAInBuf= u32PAInBuf;
    ShaCalParam.u32Size= u32Size;
    ShaCalParam.u32PAOutBuf= u32PAOutBuf;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_SHA_Calculate,(void*)&ShaCalParam) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_SHA_Calculate(eMode,u32PAInBuf,u32Size,u32PAOutBuf, NULL);
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Parser_MaskScrmb(MS_BOOL bEnable)
{
#if (AESDMA_UTOPIA20)

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_PARSER_MASKSCRMB,(void*)&bEnable) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;

#else
    AESDMA_Parser_Mask_Scrmb(bEnable);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Parser_SetScrmbPattern(DrvAESDMA_ScrmbPattern ePattern)
{
#if (AESDMA_UTOPIA20)

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_PARSER_SETSCRMBPATTERN,(void*)&ePattern) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;

#else
    AESDMA_Parser_Set_ScrmbPattern(ePattern);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Parser_SetAddedScrmbPattern(DrvAESDMA_ScrmbPattern ePattern)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_PARSER_SETADDEDSCRMBPATTERN,(void*)&ePattern) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;

#else
    AESDMA_Parser_Set_AddedScrmbPattern(ePattern);
    return DRVAESDMA_OK;
#endif
}

MS_U8 MDrv_AESDMA_Parser_QueryPidCount(void)
{
#if (AESDMA_UTOPIA20)

    MS_U8 mCount = 0;
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_PARSER_QUERYPIDCOUNT,(void*)&mCount) != UTOPIA_STATUS_SUCCESS)
    {
        return mCount;
    }
    return mCount;

#else
    return AESDMA_Parser_Query_PidCount();
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Parser_SetPid(MS_U8 u8Index, MS_U16 u16Pid)
{
#if (AESDMA_UTOPIA20)
    AESDMA_PARSER_PID_CONFIGURATIONS mConfig;

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    mConfig.u8PidIndex = u8Index;
    mConfig.u16Pid = u16Pid;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_PARSER_SETPID,(void*)&mConfig) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;

#else
    MS_U8 u8PidCount;
    // check Pid value
    if (u16Pid > AESDMA_PARSER_PID_MAX)
    {
        return DRVAESDMA_INVALID_PARAM;
    }

    // check Pid count
    u8PidCount = AESDMA_Parser_Query_PidCount();
    if (u8Index >= u8PidCount)
    {
        return DRVAESDMA_INVALID_PARAM;
    }

    AESDMA_Parser_Set_Pid(u8Index,u16Pid);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Parser_BypassPid(MS_BOOL bEnable)
{
#if (AESDMA_UTOPIA20)

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_PARSER_BYPASSPID,(void*)&bEnable) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;

#else
    AESDMA_Parser_Bypass_Pid(bEnable);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Parser_Encrypt(DrvAESDMA_ParserMode eMode)
{
#if (AESDMA_UTOPIA20)
    AESDMA_PARSER_CONFIGURATIONS stArgs;

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    stArgs.eParserMode = eMode;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_PARSER_ENCRYPT,(void*)&stArgs) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;

#else
    AESDMA_Parser_Set_Mode((AESDMA_ParserMode)eMode);
    switch (eMode)
    {
    case E_DRVAESDMA_PARSER_TS_PKT192:
    case E_DRVAESDMA_PARSER_TS_PKT192_CLEAR:
    case E_DRVAESDMA_PARSER_TS_PKT188:
    case E_DRVAESDMA_PARSER_TS_PKT188_CLEAR:
        AESDMA_Parser_Insert_Scrmb(TRUE);
        AESDMA_Parser_Remove_Scrmb(FALSE);
        break;

    case E_DRVAESDMA_PARSER_HDCP20_PKT192:
    case E_DRVAESDMA_PARSER_HDCP20_PKT192_CLEAR:
    case E_DRVAESDMA_PARSER_HDCP20_PKT188:
    case E_DRVAESDMA_PARSER_HDCP20_PKT188_CLEAR:
        AESDMA_Parser_Insert_Scrmb(FALSE);
        AESDMA_Parser_Remove_Scrmb(FALSE);
        break;
    }

    AESDMA_Parser_Enable_HWParser(TRUE);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_Parser_Decrypt(DrvAESDMA_ParserMode eMode)
{
#if (AESDMA_UTOPIA20)
    AESDMA_PARSER_CONFIGURATIONS stArgs;

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    stArgs.eParserMode = eMode;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_PARSER_DECRYPT,(void*)&stArgs) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;

#else
    AESDMA_Parser_Set_Mode((AESDMA_ParserMode)eMode);

    switch (eMode)
    {
    case E_DRVAESDMA_PARSER_TS_PKT192:
    case E_DRVAESDMA_PARSER_TS_PKT192_CLEAR:
    case E_DRVAESDMA_PARSER_TS_PKT188:
    case E_DRVAESDMA_PARSER_TS_PKT188_CLEAR:
        AESDMA_Parser_Insert_Scrmb(FALSE);
        AESDMA_Parser_Remove_Scrmb(TRUE);
        break;

    case E_DRVAESDMA_PARSER_HDCP20_PKT192:
    case E_DRVAESDMA_PARSER_HDCP20_PKT192_CLEAR:
    case E_DRVAESDMA_PARSER_HDCP20_PKT188:
    case E_DRVAESDMA_PARSER_HDCP20_PKT188_CLEAR:
        AESDMA_Parser_Insert_Scrmb(FALSE);
        AESDMA_Parser_Remove_Scrmb(FALSE);
        break;
    }


    AESDMA_Parser_Enable_HWParser(TRUE);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_RSA_Calculate(DrvAESDMA_RSASig *pstSign, DrvAESDMA_RSAKey *pstKey, DrvAESDMA_RSAMode eMode)
{
#if (AESDMA_UTOPIA20)
    RSA_CALCULATE stRsaCalParam;

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    stRsaCalParam.pstSign = pstSign;
    stRsaCalParam.pstKey = pstKey;
    stRsaCalParam.eMode= eMode;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_RSA_Calculate,(void*)&stRsaCalParam) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_RSA_Calculate(pstSign,pstKey,eMode);
#endif
}

DRVAESDMA_RESULT MDrv_RSA_IsFinished(void)
{
#if (AESDMA_UTOPIA20)
    MS_U32 u32Timeout = 0x100000;

    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_RSA_IsFinished,(void*)&u32Timeout) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_RSA_IsFinished();
#endif
}

DRVAESDMA_RESULT MDrv_RSA_Output(DrvAESDMA_RSAMode eMode, DrvAESDMA_RSAOut *pstRSAOut)
{
#if (AESDMA_UTOPIA20)
    RSA_OUTPUT RsaOutParam;
    RsaOutParam.eMode= eMode;
    RsaOutParam.pstRSAOut= pstRSAOut;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_RSA_Output,(void*)&RsaOutParam) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_RSA_Output(eMode,pstRSAOut);
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_IsSecretKeyInNormalBank(void)
{

#if (AESDMA_UTOPIA20)

    MS_U8 retVal = 0;
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_IS_SECRETKEY_IN_NORMAL_BLANK,(void*)&retVal) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }

    if (retVal == 0)
        return DRVAESDMA_FAIL;
    else
        return DRVAESDMA_OK;


#else
    if (AESDMA_IsSecretKeyInNormalBank() == 0)
    {
        return DRVAESDMA_FAIL;
    }
    else
    {
        return DRVAESDMA_OK;
    }
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_SetKeyIndex (MS_U32 u32Index, MS_BOOL bIsOddKey)
{
#if (AESDMA_UTOPIA20)
    AESDMA_KEY_INDEX KeyIndex;
    KeyIndex.bOddKeyEn = bIsOddKey;
    KeyIndex.u32KeyIndex = u32Index;
    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_Set_Key_Index, (void*)&KeyIndex) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    AESDMA_SetKeyIndex (u32Index, bIsOddKey);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_EnableTwoKeys(MS_BOOL bEnable)
{
#if (AESDMA_UTOPIA20)
    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_ENABLE_Two_Key, (void*)&bEnable) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    AESDMA_Parser_Enable_Two_Keys(bEnable);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_SetOddKey(MS_U32 *pInitVector)
{
#if (AESDMA_UTOPIA20)
    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_SetOddKey,(void*)pInitVector) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    if (pInitVector != NULL)
    {
        AESDMA_Set_CipherOddKey(pInitVector);
        return DRVAESDMA_OK;
    }
    else
    {
        return DRVAESDMA_INVALID_PARAM;
    }
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_ParserStart(MS_BOOL bStart)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma,MDrv_CMD_AESDMA_PARSER_Start,(void*)&bStart) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    return _MDrv_AESDMA_ParserStart(bStart);
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_SetAesCtr64 (MS_BOOL bCtr64En)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_Set_AES_CTR64_En, (void*)&bCtr64En) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    AESDMA_SET_CTR64(bCtr64En);
    return DRVAESDMA_OK;
#endif
}

DRVAESDMA_RESULT MDrv_AESDMA_SetSecuredKeyIndex (MS_U8 u8KeyIdx)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRVAESDMA_FAIL;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_Set_Efuse_Key_Index, (void*)&u8KeyIdx) != UTOPIA_STATUS_SUCCESS)
    {
        return DRVAESDMA_FAIL;
    }
    return DRVAESDMA_OK;
#else
    AESDMA_SetSecureKeyIdx (u8KeyIdx);
    return DRVAESDMA_OK;
#endif
}

MS_BOOL MDrv_AESDMA_GetConfig(PAESDMA_INIT sAesdmaInit)
{
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;
#if (AESDMA_UTOPIA20)
    if(NULL == pInstantAesdma)
    {
        if(UTOPIA_STATUS_FAIL == UtopiaOpen(MODULE_AESDMA, &pInstantAesdma, 0, NULL))
        {
            ULOGE("AESDMA", "%s, open MODULE_AESDMA failed\n", __FUNCTION__);
            return FALSE;
        }
    }
    u32Ret = UtopiaIoctl(pInstantAesdma, MDrv_CMD_AESDMA_GetConfig, (PAESDMA_INIT)sAesdmaInit);
    if(UTOPIA_STATUS_SUCCESS != u32Ret)
    {
        ULOGE("AESDMA", "[%s]ERROR : UtopiaIoctl return value 0x%x\n", __FUNCTION__, (unsigned int)u32Ret);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
#else
    return TRUE;
#endif
}
#endif

#if AESDMS_SECURED_BANK_EN
static DRV_CIPHER_RET _MDrv_CIPHER_DMACFG_Chk(DRV_CIPHER_DMACFG* pstCfg, MS_U32 *pu32CmdId)
{
    DRV_CIPHER_DMACFG stCfg;

    stCfg = *pstCfg;
    if(_u8IsInit == FALSE)
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "Error:AESDMA is not initialized!\n"));
        return DRV_CIPHER_FAIL;
    }

    ///Check data size
    if((stCfg.stInput.u32Size == 0) || (stCfg.stOutput.u32Size == 0))
        return DRV_CIPHER_BAD_PARAM;

    ///Check Key length
    if((stCfg.stKey.u8KeyLen > 16) || (stCfg.stKey.u8IVLen > 16))
        return DRV_CIPHER_BAD_PARAM;

    ///Check CmdId
    if(pu32CmdId == NULL)
        return DRV_CIPHER_BAD_PARAM;

    return DRV_CIPHER_OK;
}


static DRV_CIPHER_RET _MDrv_CIPHER_Cipher2AesdmaMode(DRV_CIPHER_DMACFG* pstCfg, DrvAESDMA_CipherMode* peMode)
{
    DRV_CIPHER_DMACFG stCfg;

    stCfg = *pstCfg;
    switch(stCfg.stAlgo.eMainAlgo)
    {
	case E_CIPHER_MAIN_AES:
            switch(stCfg.stAlgo.eResAlgo)
            {
		case E_CIPHER_RES_NONE:
		    switch(stCfg.stAlgo.eSubAlgo)
	            {
			case E_CIPHER_SUB_ECB:
			    *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_ECB;
			    break;
                        case E_CIPHER_SUB_CBC:
			    *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_CBC;
		            break;
                        case E_CIPHER_SUB_CTR:
			    *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_CTR;
			    break;
			default:
			    return DRV_CIPHER_BAD_PARAM;
		    }
		    break;
                case E_CIPHER_RES_CTS:
		    switch(stCfg.stAlgo.eSubAlgo)
		    {
		        case E_CIPHER_SUB_ECB:
			    *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_CTS_ECB;
			    break;
                        case E_CIPHER_SUB_CBC:
			    *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_CTS_CBC;
			    break;
			default:
                            return DRV_CIPHER_BAD_PARAM;
		    }
		    break;
	        default:
                    return DRV_CIPHER_BAD_PARAM;
	    }
	    break;
            case E_CIPHER_MAIN_DES:
	        switch(stCfg.stAlgo.eResAlgo)
		{
		    case E_CIPHER_RES_NONE:
		        switch(stCfg.stAlgo.eSubAlgo)
			{
                            case E_CIPHER_SUB_ECB:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_DES_ECB;
				break;
                            case E_CIPHER_SUB_CBC:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_DES_CBC;
				break;
                            case E_CIPHER_SUB_CTR:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_DES_CTR;
			        break;
			    default:
                                return DRV_CIPHER_BAD_PARAM;
			}
			break;
                    case E_CIPHER_RES_CTS:
			switch(stCfg.stAlgo.eSubAlgo)
			{
			    case E_CIPHER_SUB_ECB:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_DES_CTS_ECB;
				break;
                            case E_CIPHER_SUB_CBC:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_DES_CTS_CBC;
				break;
			    default:
                                return DRV_CIPHER_BAD_PARAM;
			}
			break;
	                default:
                            return DRV_CIPHER_BAD_PARAM;
	        }
		break;
            case E_CIPHER_MAIN_TDES:
		switch(stCfg.stAlgo.eResAlgo)
		{
		    case E_CIPHER_RES_NONE:
		        switch(stCfg.stAlgo.eSubAlgo)
			{
			    case E_CIPHER_SUB_ECB:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_TDES_ECB;
				break;
                            case E_CIPHER_SUB_CBC:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_TDES_CBC;
				break;
                            case E_CIPHER_SUB_CTR:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_TDES_CTR;
			        break;
			    default:
                                return DRV_CIPHER_BAD_PARAM;
			}
			break;
                    case E_CIPHER_RES_CTS:
			switch(stCfg.stAlgo.eSubAlgo)
			{
			    case E_CIPHER_SUB_ECB:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_TDES_CTS_ECB;
				break;
                            case E_CIPHER_SUB_CBC:
			        *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_TDES_CTS_CBC;
				break;
			    default:
                                return DRV_CIPHER_BAD_PARAM;
			}
			break;
	                default:
                            return DRV_CIPHER_BAD_PARAM;
		}
		break;
            case E_CIPHER_MAIN_NONE:
                *peMode = (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_NONE;
                break;
            default:
                return DRV_CIPHER_BAD_PARAM;
	}
    return DRV_CIPHER_OK;
}

static DRV_CIPHER_RET _MDrv_CIPHER_GetFreeEngine(eSemId SemId, DrvAESDMA_EngSel *peEngSel)
{
    MS_U16 u16ResID = 0;
    MS_U16* pu16ResId  = &u16ResID;
    MS_S16 s16SemId;
    DrvAESDMA_EngSel eEngSel;

    switch(SemId)
    {
        case E_SEM_AESDMA0:
            eEngSel = E_DRVAESDMA_ENG0;
            break;
        case E_SEM_AESDMA1:
            eEngSel = E_DRVAESDMA_ENG1;
            break;
        default:
            return DRV_CIPHER_FAIL;
    }

    s16SemId = HAL_SEM_GetSemId(SemId);
    if (MDrv_SEM_Get_ResourceID((MS_U8)s16SemId, pu16ResId) == TRUE)
    {
        if(*pu16ResId == 0)
	{
	    if(TRUE == MDrv_SEM_Lock(E_SEM_AESDMA1, SEM_WAIT_FOREVER))
	    {
		*peEngSel = eEngSel;
	        if (MDrv_SEM_Get_ResourceID((MS_U8)s16SemId, pu16ResId) == TRUE)
                    DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]Select Engine%d! The UserId = %u \n",__FUNCTION__,__LINE__,eEngSel,*pu16ResId));
                return DRV_CIPHER_OK;
            }
	}
	else
	{
	    DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]AESDMA ENGINE%d is used! The UserId = %u \n",__FUNCTION__,__LINE__,eEngSel,*pu16ResId));
	}
    }
    return DRV_CIPHER_FAIL;
}

DRV_CIPHER_RET MDrv_CIPHER_DMACalc(DRV_CIPHER_DMACFG stCfg, MS_U32 *pu32CmdId)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRV_CIPHER_FAIL;

    CIPHER_DMACALC sCipherDMACalc;
    sCipherDMACalc.stCfg = stCfg;
    sCipherDMACalc.pu32CmdId = pu32CmdId;
    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_CIPHER_DMACALC, (void*)&sCipherDMACalc) != UTOPIA_STATUS_SUCCESS)
	{
        return DRV_CIPHER_FAIL;
    }
    return DRV_CIPHER_OK;
#else
    return _MDrv_CIPHER_DMACalc(stCfg, pu32CmdId);
#endif
}

DLL_PUBLIC DRV_CIPHER_RET _MDrv_CIPHER_DMACalc(DRV_CIPHER_DMACFG stCfg, MS_U32 *pu32CmdId)
{
    DRV_CIPHER_RET cipher_ret = DRV_CIPHER_FAIL;

    DRV_CIPHER_DMACFG* pstCfg;
    DrvAESDMA_CipherMode eMode = E_DRVAESDMA_CIPHER_NONE;
    DrvAESDMA_EngSel eEngSel;
    MS_PHY  phyOutSAaddr;

    pstCfg = &stCfg;
    //Check parameters
    cipher_ret = _MDrv_CIPHER_DMACFG_Chk(pstCfg, pu32CmdId);
    if( cipher_ret != DRV_CIPHER_OK )
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "Error:BAD_PARAM! \n"));
        return cipher_ret;
    }

    if(MDrv_SEM_Init() != TRUE)
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "Error:SEM is not initial! \n"));
        return DRV_CIPHER_FAIL;
    }
    MsOS_FlushMemory();

    ///choose which ENGINE
    //==============Obtain MUTEX==============
    if( _s32AESDMAMutexId < 0)
        return DRV_CIPHER_FAIL;
    MsOS_ObtainMutex(_s32AESDMAMutexId, MSOS_WAIT_FOREVER);

    if(_MDrv_CIPHER_GetFreeEngine(E_SEM_AESDMA1, &eEngSel) == DRV_CIPHER_OK)
        goto DMA_DATA;
    if(_MDrv_CIPHER_GetFreeEngine(E_SEM_AESDMA0, &eEngSel) == DRV_CIPHER_OK)
        goto DMA_DATA;

    MsOS_ReleaseMutex(_s32AESDMAMutexId);
    return DRV_CIPHER_FAIL;

DMA_DATA:
	///MODE
    AESDMA_MultEng_Reset(eEngSel);
    if( _MDrv_CIPHER_Cipher2AesdmaMode(pstCfg, &eMode) != DRV_CIPHER_OK )
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]Error:No Mode%d \n",__FUNCTION__,__LINE__,eMode));
        goto DMA_FINISH;
    }

    AESDMA_MultEng_Set_CIPHER_ENG(eMode, stCfg.bDecrypt, eEngSel);
    DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]eMode = %u stCfg.bDecrypt = %d \n",__FUNCTION__,__LINE__,eMode,stCfg.bDecrypt));
	///KEY
    if(AESDMA_MultEng_SetKey_Ex(stCfg.stKey.pu8KeyData, (MS_U32) stCfg.stKey.u8KeyLen, eEngSel) != DRVAESDMA_OK)
    {
	DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:SetKey \n",__FUNCTION__,__LINE__));
	goto DMA_FINISH;
    }
	///IV
    if(AESDMA_MultEng_SetIV_Ex(stCfg.stKey.pu8IVData, (MS_U32) stCfg.stKey.u8IVLen, eEngSel) != DRVAESDMA_OK)
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:SetIV  \n",__FUNCTION__,__LINE__));
        goto DMA_FINISH;
    }
	///ADDR
    phyOutSAaddr = (MS_PHY)(stCfg.stOutput.u32Addr+stCfg.stOutput.u32Size);
    if (!_AESDMA_Chk_MIU(&stCfg.stInput.u32Addr, stCfg.stInput.u32Size, &stCfg.stOutput.u32Addr, &phyOutSAaddr))
    {
       DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:_AESDMA_Chk_MIU  \n",__FUNCTION__,__LINE__));
	goto DMA_FINISH;
    }
    if(AESDMA_MultEng_SetFileInOut(stCfg.stInput.u32Addr, stCfg.stInput.u32Size, stCfg.stOutput.u32Addr, phyOutSAaddr, eEngSel) == DRVAESDMA_OK)
    {
	phyTmpFileInAddr = stCfg.stInput.u32Addr;
        u32TmpFileInNum = stCfg.stInput.u32Size;
        phyTmpFileOutAddr = stCfg.stOutput.u32Addr;
    }
    else
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:SetFileInOut \n",__FUNCTION__,__LINE__));
        goto DMA_FINISH;
    }
	///START
    if (!MsOS_Dcache_Flush(MsOS_PA2KSEG0(phyTmpFileInAddr), u32TmpFileInNum))
        MsOS_Dcache_Flush(MsOS_PA2KSEG0(phyTmpFileInAddr) & ~((MS_VIRT)0x0F), u32TmpFileInNum+32);
    if (!MsOS_Dcache_Flush(MsOS_PA2KSEG0(phyTmpFileOutAddr), u32TmpFileInNum))
        MsOS_Dcache_Flush(MsOS_PA2KSEG0(phyTmpFileOutAddr) & ~((MS_VIRT)0x0F), u32TmpFileInNum+32);
    MsOS_FlushMemory();
    if (AESDMA_MultEng_Start(TRUE, eEngSel) != DRVAESDMA_OK)
    {
	DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "ERROR: DMA Fail!\n"));
	goto DMA_FINISH;
    }

    *pu32CmdId = (MS_U32) eEngSel;
    //==============Realse MUTEX==============
    MsOS_ReleaseMutex(_s32AESDMAMutexId);
    return DRV_CIPHER_OK;

DMA_FINISH:
    if(eEngSel == E_DRVAESDMA_ENG0)
        MDrv_SEM_Unlock(E_SEM_AESDMA0);
    else
        MDrv_SEM_Unlock(E_SEM_AESDMA1);
    DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]Unlock SEM%d in DMACalc!\n",__FUNCTION__,__LINE__,eEngSel));

    MsOS_ReleaseMutex(_s32AESDMAMutexId);
    return DRV_CIPHER_FAIL;
}

MS_BOOL MDrv_CIPHER_IsDMADone(MS_U32 u32CmdId, MS_U32 *pu32Exception)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRV_CIPHER_FAIL;

    CIPHER_ISDMADONE sCipherIsDMADone;
    sCipherIsDMADone.u32CmdId = u32CmdId;
    sCipherIsDMADone.pu32Exception = pu32Exception;
    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_CIPHER_ISDMADONE, (void*)&sCipherIsDMADone) != UTOPIA_STATUS_SUCCESS)
    {
        return DRV_CIPHER_FAIL;
    }
    return DRV_CIPHER_OK;
#else
    return _MDrv_CIPHER_IsDMADone(stCfg, pu32Exception);
#endif
}

MS_BOOL _MDrv_CIPHER_IsDMADone(MS_U32 u32CmdId, MS_U32 *pu32Exception)
{
    eSemId SemId;

    switch(u32CmdId)
    {
        case E_DRVAESDMA_ENG0:
	case E_CMDQ_DRVAESDMA_ENG0:
            SemId = E_SEM_AESDMA0;
            break;
        case E_DRVAESDMA_ENG1:
        case E_CMDQ_DRVAESDMA_ENG1:
            SemId = E_SEM_AESDMA1;
            break;
        default:
            return FALSE;
    }

    switch(u32CmdId)
    {
	case E_DRVAESDMA_ENG0:
                u32ProtEng0 = 0; //unlock NEW API
	case E_DRVAESDMA_ENG1:
            if(AESDMA_MultEng_IsFinished(u32CmdId) == DRVAESDMA_OK)
            {
                if(MDrv_SEM_Unlock(SemId) == TRUE)
                {
                    DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]Unlok AESDMA ENGINE%d \n",__FUNCTION__,__LINE__,u32CmdId));
                    break;
                }
                else
                    DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:Unlok AESDMA ENGINE%d FALSE! \n",__FUNCTION__,__LINE__,u32CmdId));
            }
            else
            {
                DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]AESDMA ENGINE%d NOTDONE! \n",__FUNCTION__,__LINE__,u32CmdId));
            }
            return FALSE;
	case E_CMDQ_DRVAESDMA_ENG0:
	case E_CMDQ_DRVAESDMA_ENG1:
            if(AESDMA_MultEng_CMDQGetDummyReg() == TRUE)
            {
//Close SEM
#if 0
                if(MDrv_SEM_Unlock(SemId) == TRUE)
                {
                    DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]Unlok AESDMA ENGINE%d \n",__FUNCTION__,__LINE__,u32CmdId-2));
                    break;
                }
                else
                    DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:Unlok AESDMA ENGINE%d FALSE! \n",__FUNCTION__,__LINE__,u32CmdId-2));
#endif
                AESDMA_MultEng_CleanDummyReg();
                break;
            }
            else
            {
                DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]CMDQ AESDMA ENGINE%d NOTDONE! \n",__FUNCTION__,__LINE__,u32CmdId-2));
            }
            return FALSE;
        default:
            return FALSE;
    }

    return TRUE;
}

DRV_CIPHER_RET MDrv_CIPHER_DMACalcList(DRV_CIPHER_DMACFG* pstCfg, MS_U32 u32NumOfCmds, MS_U32 *pu32CmdId, MS_PHY BufferAddr, MS_PHY BufferLen,  MS_U32 CMDQ_MIU_SELECT)
{
#if (AESDMA_UTOPIA20)
    if (NULL == pInstantAesdma)
        return DRV_CIPHER_FAIL;
    CIPHER_DMACALCLIST sCipherDMACalcList;
    sCipherDMACalcList.pstCfg = pstCfg;
    sCipherDMACalcList.u32NumOfCmds = u32NumOfCmds;
    sCipherDMACalcList.pu32CmdId = pu32CmdId;
    sCipherDMACalcList.BufferAddr = BufferAddr;
    sCipherDMACalcList.BufferLen = BufferLen;
    sCipherDMACalcList.CMDQ_MIU_SELECT = CMDQ_MIU_SELECT;

    if(UtopiaIoctl(pInstantAesdma, MDrv_CMD_CIPHER_DMACALCLIST, (void*)&sCipherDMACalcList) != UTOPIA_STATUS_SUCCESS)
    {
        return DRV_CIPHER_FAIL;
    }
    return DRV_CIPHER_OK;
#else
    return _MDrv_CIPHER_DMACalcList(pstCfg, u32NumOfCmds, pu32CmdId, BufferAddr, BufferLen, CMDQ_MIU_SELECT);
#endif
}

DRV_CIPHER_RET _MDrv_CIPHER_DMACalcList(DRV_CIPHER_DMACFG* pstCfg, MS_U32 u32NumOfCmds, MS_U32 *pu32CmdId, MS_PHY BufferAddr, MS_PHY BufferLen,  MS_U32 CMDQ_MIU_SELECT)
{
    DRV_CIPHER_RET cipher_ret = DRV_CIPHER_FAIL;
    MS_U32  idx;
    MS_PHY  phyOutSAaddr;

    DRV_CIPHER_DMACFG stCfg;
    DrvAESDMA_CipherMode eMode = E_DRVAESDMA_CIPHER_NONE;
    DrvAESDMA_EngSel eEngSel;

    if (_u8IsInit == FALSE)
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "Error:AESDMA is not initialized!\n"));
        return DRV_CIPHER_FAIL;
    }
//Close SEM
#if 0
    if(MDrv_SEM_Init() != TRUE)
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "Error:SEM is not initial! \n"));
        return DRV_CIPHER_FAIL;
    }
#endif
	//==============Obtain MUTEX==============
    if( _s32AESDMAMutexId < 0)
        return DRV_CIPHER_FAIL;
    MsOS_ObtainMutex(_s32AESDMAMutexId, MSOS_WAIT_FOREVER);
    //Close SEM
#if 0
	///ENGINE
	//always choose ENG1
    if(_MDrv_CIPHER_GetFreeEngine(E_SEM_AESDMA1, &eEngSel) == DRV_CIPHER_OK)
        goto alcList_DATA;
    MsOS_ReleaseMutex(_s32AESDMAMutexId);
    return DRV_CIPHER_FAIL;

CalcList_DATA:
    if(eEngSel == E_DRVAESDMA_ENG0)
        eEngSel = E_CMDQ_DRVAESDMA_ENG0;
    else if(eEngSel == E_DRVAESDMA_ENG1)
        eEngSel = E_CMDQ_DRVAESDMA_ENG1;
    *pu32CmdId = (MS_U32) eEngSel;
    DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]eEngSel change to CMQENG%u \n",__FUNCTION__,__LINE__,eEngSel));
#endif
    *pu32CmdId = (MS_U32) E_CMDQ_DRVAESDMA_ENG1;
     eEngSel = E_CMDQ_DRVAESDMA_ENG1;
	////////////////////////////////////////////////////////
    if( AESDMA_MultEng_CMDQInit(eEngSel, BufferAddr, BufferLen, CMDQ_MIU_SELECT) != TRUE )
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]AESDMA_MultEng_CMDQInit FALSE \n",__FUNCTION__,__LINE__));
        MsOS_ReleaseMutex(_s32AESDMAMutexId);
        return DRV_CIPHER_FAIL;
    }
	///set data
    for(idx = 0; idx < u32NumOfCmds; idx++)
    {
        stCfg = pstCfg[idx];
        ///MODE
        if( _MDrv_CIPHER_Cipher2AesdmaMode(&pstCfg[idx], &eMode) != DRV_CIPHER_OK )
        {
            DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]Error:No Mode%d \n",__FUNCTION__,__LINE__,eMode));
            cipher_ret = DRV_CIPHER_BAD_PARAM;
            goto NOT_CHSENG;
        }

        AESDMA_MultEng_Set_CIPHER_ENG(eMode, stCfg.bDecrypt, eEngSel);
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]eMode = %u stCfg.bDecrypt = %d \n",__FUNCTION__,__LINE__,eMode, stCfg.bDecrypt));
        if(eMode == (DrvAESDMA_CipherMode)E_AESDMA_CIPHER_NONE)
            goto DMA_ADDR;
	    ///KEY
        if(AESDMA_MultEng_SetKey_Ex(stCfg.stKey.pu8KeyData, (MS_U32) stCfg.stKey.u8KeyLen, eEngSel) != DRVAESDMA_OK)
        {
            DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:SetKey \n",__FUNCTION__,__LINE__));
            cipher_ret = DRV_CIPHER_BAD_PARAM;
            goto NOT_CHSENG;
        }
            ///IV
        if(AESDMA_MultEng_SetIV_Ex(stCfg.stKey.pu8IVData, (MS_U32) stCfg.stKey.u8IVLen, eEngSel) != DRVAESDMA_OK)
        {
            DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:SetIV  \n",__FUNCTION__,__LINE__));
            cipher_ret = DRV_CIPHER_BAD_PARAM;
            goto NOT_CHSENG;
        }
DMA_ADDR:
            ///ADDR
        phyOutSAaddr = (MS_PHY) (stCfg.stOutput.u32Addr+stCfg.stOutput.u32Size);
        if (!_AESDMA_Chk_MIU(&stCfg.stInput.u32Addr, stCfg.stInput.u32Size, &stCfg.stOutput.u32Addr, &phyOutSAaddr))
        {
           DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:_AESDMA_Chk_MIU  \n",__FUNCTION__,__LINE__));
            cipher_ret = DRV_CIPHER_BAD_PARAM;
            goto NOT_CHSENG;
        }
        if(AESDMA_MultEng_SetFileInOut(stCfg.stInput.u32Addr, stCfg.stInput.u32Size, stCfg.stOutput.u32Addr, phyOutSAaddr, eEngSel) == DRVAESDMA_OK)
        {
            phyTmpFileInAddr = stCfg.stInput.u32Addr;
            u32TmpFileInNum = stCfg.stInput.u32Size;
            phyTmpFileOutAddr = stCfg.stOutput.u32Addr;
        }
        else
        {
            DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:SetFileInOut \n",__FUNCTION__,__LINE__));
            cipher_ret = DRV_CIPHER_BAD_PARAM;
            goto NOT_CHSENG;
        }
            ///START
        if (AESDMA_MultEng_Start(TRUE, eEngSel) != DRVAESDMA_OK)
        {
            DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "ERROR: DMA Fail!\n"));
            cipher_ret = DRV_CIPHER_BAD_PARAM;
            goto NOT_CHSENG;
        }

        if( AESDMA_MultEng_CMDQWaitCommand() != TRUE )
            DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:AESDMA_MultEng_CMDQWaitCommand \n",__FUNCTION__,__LINE__));

NOT_CHSENG:
        if(cipher_ret == DRV_CIPHER_BAD_PARAM)
            DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]DRVAESDMA_INVALID_PARAM at %d AESDMA data! \n",__FUNCTION__,__LINE__, idx+1 ));

    }
    AESDMA_MultEng_CMDQDummyReg();

    //==============Realse MUTEX==============
    MsOS_ReleaseMutex(_s32AESDMAMutexId);
    if(  AESDMA_MultEng_CMDQEnd() != TRUE )
    {
        DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]ERROR:AESDMA_MultEng_CMDQEnd \n",__FUNCTION__,__LINE__));
//Close SEM
#if 0
        if(MDrv_SEM_Unlock(E_SEM_AESDMA1) == TRUE)
            DEBUG_AES_DMA(E_DRVAESDMA_DBG_L1, ULOGE(TAG_AESDMA, "[%s,%5d]Unlok AESDMA ENGINE1 \n",__FUNCTION__,__LINE__));
#endif
        return DRV_CIPHER_FAIL;
    }
    return DRV_CIPHER_OK;
}
#endif

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
static MS_BOOL _MDrv_AESDMA_GetAESDMAInfo(DrvAESDMA_MDB_AESDMA *psDrvAESDMA_MDB_AESDMA)
{
    return HAL_AESDMA_GetAESDMAInfo(psDrvAESDMA_MDB_AESDMA);
}

static MS_BOOL _MDrv_AESDMA_GetSHAInfo(DrvAESDMA_MDB_SHA *psDrvAESDMA_MDB_SHA)
{
    return HAL_AESDMA_GetSHAInfo(psDrvAESDMA_MDB_SHA);
}

static MS_BOOL _MDrv_AESDMA_GetRSAInfo(DrvAESDMA_MDB_RSA *psDrvAESDMA_MDB_RSA)
{
    return HAL_AESDMA_GetRSAInfo(psDrvAESDMA_MDB_RSA);
}

MS_BOOL MDrv_AESDMA_MdbCmdLine(MDBCMD_CMDLINE_PARAMETER *paraCmdLine, EN_AESDMA_MDBCMDLINE eMdbCmdLine)
{
    MS_U32 temp = 0x0;
    MS_U32 u32done = 0x0;
    MDrv_AESDMA_Init(0, 0x20000000, 2);
    switch(eMdbCmdLine)
    {
        case AESDMA_MDBCMDLINE_HELP:
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar AESDMA help---------\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"aesdma\tget aesdma information\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"sha\tget sha information\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"rsa\tget sha information\n");
            break;
        case AESDMA_MDBCMDLINE_AESDMA:
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar AESDMA Info---------\n");
            _MDrv_AESDMA_GetAESDMAInfo(&sDrvAESDMA_MDB_AESDMA);
            _MDrv_AESDMA_IsFinished(&u32done);
            MdbPrint(paraCmdLine->u64ReqHdl,"DONE:\t0x%x \n",temp);
            MdbPrint(paraCmdLine->u64ReqHdl,"Algorithm:\t0x%x \n",sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_Algorithm);
            MdbPrint(paraCmdLine->u64ReqHdl,"Input Address:\t0x%x \n",phyTmpFileInAddr);
            MdbPrint(paraCmdLine->u64ReqHdl,"Output Address:\t0x%x \n",phyTmpFileOutAddr);
            MdbPrint(paraCmdLine->u64ReqHdl,"End Output Address:\t0x%x \n",phyTmpFileOutAddr+(MS_PHY)u32TmpFileInNum);
            MdbPrint(paraCmdLine->u64ReqHdl,"Data Size:\t0x%x \n",u32TmpFileInNum);
            MdbPrint(paraCmdLine->u64ReqHdl,"Input data Location:\t0x%x \n",sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_Location[0]);
            MdbPrint(paraCmdLine->u64ReqHdl,"Output data Location:\t0x%x \n",sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_Location[1]);
            MdbPrint(paraCmdLine->u64ReqHdl,"KEY select:\t0x%x \n",sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_KeySelect);
            MdbPrint(paraCmdLine->u64ReqHdl,"KEY:\t0x%x %x %x %x\n",sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_Key[0], sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_Key[1], sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_Key[2], sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_Key[3]);
            MdbPrint(paraCmdLine->u64ReqHdl,"IV:\t0x%x %x %x %x\n",sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_IV[0], sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_IV[1], sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_IV[2], sDrvAESDMA_MDB_AESDMA.DBG_AESDMA_IV[3]);
            break;
        case AESDMA_MDBCMDLINE_SHA:
            _MDrv_AESDMA_GetSHAInfo(&sDrvAESDMA_MDB_SHA);
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar SHA Info---------\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"Algorithm:\t0x%x \n",sDrvAESDMA_MDB_SHA.DBG_SHA_Algorithm);
            MdbPrint(paraCmdLine->u64ReqHdl,"Bypass Mode:\t0x%x \n",sDrvAESDMA_MDB_SHA.DBG_SHA_BypassMode);
            MdbPrint(paraCmdLine->u64ReqHdl,"Input Address:\t0x%x \n",sDrvAESDMA_MDB_SHA.DBG_SHA_InputAddr);
            MdbPrint(paraCmdLine->u64ReqHdl,"Data Size:\t0x%x \n",sDrvAESDMA_MDB_SHA.DBG_SHA_Length);
            MdbPrint(paraCmdLine->u64ReqHdl,"Input data Location:\t0x%x \n",sDrvAESDMA_MDB_SHA.DBG_SHA_InputLocation);
            MdbPrint(paraCmdLine->u64ReqHdl,"SHA Status:\t0x%x \n",sDrvAESDMA_MDB_SHA.DBG_SHA_Status);
            MdbPrint(paraCmdLine->u64ReqHdl,"SHA Out:\t0x%x %x %x %x\n",sDrvAESDMA_MDB_SHA.DBG_SHA_Out[0], sDrvAESDMA_MDB_SHA.DBG_SHA_Out[1], sDrvAESDMA_MDB_SHA.DBG_SHA_Out[2], sDrvAESDMA_MDB_SHA.DBG_SHA_Out[3]);
            MdbPrint(paraCmdLine->u64ReqHdl,"SHA Out:\t0x%x %x %x %x\n",sDrvAESDMA_MDB_SHA.DBG_SHA_Out[4], sDrvAESDMA_MDB_SHA.DBG_SHA_Out[5], sDrvAESDMA_MDB_SHA.DBG_SHA_Out[6], sDrvAESDMA_MDB_SHA.DBG_SHA_Out[7]);
            break;
        case AESDMA_MDBCMDLINE_RSA:
            _MDrv_AESDMA_GetRSAInfo(&sDrvAESDMA_MDB_RSA);
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar RSA Info---------\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"Mode:\t0x%x \n",sDrvAESDMA_MDB_RSA.DBG_RSA_Mode);
            MdbPrint(paraCmdLine->u64ReqHdl,"Address Auto-increment Enable:\t0x%x \n",sDrvAESDMA_MDB_RSA.DBG_RSA_AutoIncre);
            MdbPrint(paraCmdLine->u64ReqHdl,"Auto-start Enable:\t0x%x \n",sDrvAESDMA_MDB_RSA.DBG_RSA_AutoStart);
            MdbPrint(paraCmdLine->u64ReqHdl,"Input Address:\t0x%x \n",sDrvAESDMA_MDB_RSA.DBG_RSA_InputAddr);
            MdbPrint(paraCmdLine->u64ReqHdl,"Write Data:\t0x%x %x\n",sDrvAESDMA_MDB_RSA.DBG_RSA_WriteData[1], sDrvAESDMA_MDB_RSA.DBG_RSA_WriteData[0]);
            MdbPrint(paraCmdLine->u64ReqHdl,"Read Data:\t0x%x %x\n",sDrvAESDMA_MDB_RSA.DBG_RSA_ReadData[1], sDrvAESDMA_MDB_RSA.DBG_RSA_ReadData[0]);
            MdbPrint(paraCmdLine->u64ReqHdl,"RSA Status:\t0x%x \n",sDrvAESDMA_MDB_RSA.DBG_RSA_Status);
            MdbPrint(paraCmdLine->u64ReqHdl,"KEY select:\t0x%x \n",sDrvAESDMA_MDB_RSA.DBG_RSA_KeySelect);
            MdbPrint(paraCmdLine->u64ReqHdl,"KEY Length:\t0x%x \n",sDrvAESDMA_MDB_RSA.DBG_RSA_KeyLength);
            break;
        default:
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar AESDMA---------\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"No have this cmd!\nPlease use 'echo help > /proc/utopia_mdb/miu' get cmdline!\n");
            return FALSE;
    }
    return TRUE;
}
#endif
