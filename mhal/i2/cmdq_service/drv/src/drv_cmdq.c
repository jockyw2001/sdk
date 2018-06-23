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
#endif
#include "cam_os_wrapper.h"
#ifdef HAL_K6_SIMULATE
#include <linux/mman.h>
#include <asm/mach/map.h>
#include <chip_setup.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#include <linux/io.h>
#include <linux/module.h>
#endif

#if defined(HAL_I3_SIMULATE) || defined(HAL_I2_SIMULATE)
#include <linux/mm.h>
#include "ms_platform.h"
#include "ms_msys.h"
#endif
#if !defined(CAM_OS_RTK)
#include "ms_platform.h"
#endif
#include "drv_cmdq_os.h"
#include "drv_cmdq_irq.h"
#include "mhal_common.h"
#include "hal_cmdq_dbg.h"
#include "mhal_cmdq.h"
#include "hal_cmdq_util.h"
#include "drv_cmdq.h"
#include "hal_cmdq.h"

#define    WAIT_SLEEP_TIMEOUT        (3*1000)
#define    WAIT_SLEEP_TIME_CNT       150
#define    SLEEP_POLL_TIME           10 //10MS

u32 gCmdqDbgLvl = CMDQ_DBG_NONE_TYPE;
static int  _gInitPrivData = 0 ;
#if !defined(HAL_I2_SIMULATE_NO_HW)
static unsigned int _gPollTimeOut   = 0x000000FF;
static unsigned int _gPollTimeRatio = 0x0000000F;
#endif
CmdqInterfacePriv_t gDrvCmdPriv[EN_CMDQ_TYPE_MAX];

s32 MDrvCmdqWriteDummyCmdForRiu(void* pPriv, u16 nValue);
void MDrvCmdDbgPause(DrvCmdqIPSupport_e eIpNum);
void MDrvCmdDbgContinue(DrvCmdqIPSupport_e eIpNum);
//void MdrvCmdqWorkQueueFunc(struct work_struct *wq);
s32 MDrvCmdqKickOfftCommandByRiu(void* pPriv);

#ifdef HAL_K6_SIMULATE

u32 gDrvCmdqIrqNumber[NUMBER_OF_CMDQ_HW] =
{
    CMDQ_IP0_IRQ_NUM
};

#elif defined(HAL_I3_SIMULATE)
u32 gDrvCmdqIrqNumber[NUMBER_OF_CMDQ_HW] =
{
    CMDQ_IP0_IRQ_NUM
};

#else

u32 gDrvCmdqIrqNumber[NUMBER_OF_CMDQ_HW] =
{

    CMDQ_IP0_IRQ_NUM,
    CMDQ_IP1_IRQ_NUM,
    CMDQ_IP2_IRQ_NUM,
    CMDQ_IP3_IRQ_NUM,
    CMDQ_IP4_IRQ_NUM
};

CmdqPollTimeRegInfo_t gDrvCmdqPollTimeReg[NUMBER_OF_CMDQ_HW] =
{
    {0x123500UL,0x15265C, /*0x15262e*/0},
    {0x123c00UL,0x1349BE, /*0x13495F*/0},
    {0x123d00UL,0x171DF0, /*0x171D78*/0},
    {0x123e00UL,0x173CF0, /*0x173C78*/0},
    {0x122f00UL,0x1112FC,  /*0x11127E*/0}
};
#if DRVCMDQ_ENABLE_DUMMY_IRQ

u32 gDrvCmdqHWDummyIrqNumber[NUMBER_OF_CMDQ_HW] =
{
    CMDQ_IP0_DUMMY_IRQ_NUM,
    CMDQ_IP1_DUMMY_IRQ_NUM,
    CMDQ_IP2_DUMMY_IRQ_NUM,
    CMDQ_IP3_DUMMY_IRQ_NUM,
    CMDQ_IP4_DUMMY_IRQ_NUM
};

#endif

#endif

#if defined(HAL_K6_SIMULATE)

#define HAL_MIU0_BASE               0x00000000UL
#if defined(__AEONR2__)
#define HAL_MIU1_BASE               0x40000000UL // 1512MB
#else
#define HAL_MIU1_BASE               0x80000000UL // 1512MB
#define HAL_MIU2_BASE               0xC0000000UL //
#endif

/* BUS ADDR */
#define HAL_MIU0_BUS_BASE           0x20000000UL // MIU0 Low 256MB
#define HAL_MIU1_BUS_BASE           0x60000000UL // MIU1 Low 256MB
#define HAL_MIU2_BUS_BASE           0xE0000000UL // MIU2 Low 256MB

bool MdrvCmdqDcacheFlush(u32 pAddrVirt, u32 tSize)
{
    Chip_Flush_Cache_Range(pAddrVirt, tSize);

    return true;
}
static u32 map_kdriver_mem(u32 u32BusStart, u32 u32MapSize, bool bNonCache)
{
    u32 VirtAddr = 0;

    if(pfn_valid(__phys_to_pfn(u32BusStart & PAGE_MASK)))
    {
        /* the __va will return the cached kernel_memory mapping !! This is due to the kernel memory is mapped as cached, and never be remapped @3.10.86(but 3.10.40 will be remaped to non-cached after allocated by cma)
         * So, we need to remap the non-cached request !! The UnMapping will also need a corresponding change.
         *
         *
         * If 3.10.86 having remap for cma, then here and UnMapping may need to change again
         */
        if(bNonCache)
        {
            int err;
            unsigned long addr;
            struct vm_struct *area;

            area = get_vm_area_caller(u32MapSize, VM_IOREMAP, "TVOS");
            if(!area)
            {
                dump_stack();
                return 0;
            }
            addr = (unsigned long)area->addr;
#if defined (__aarch64__)
            err = ioremap_page_range(addr, addr + u32MapSize, u32BusStart, __pgprot(PROT_DEVICE_nGnRnE));
#else
            err = ioremap_page_range(addr, addr + u32MapSize, u32BusStart, MT_DEVICE);
#endif
            if(err)
            {
                dump_stack();
                return 0;
            }
            VirtAddr = addr;
        }
        else
            VirtAddr = (u32)__va(u32BusStart);
    }
    else
    {
        if(bNonCache)
            VirtAddr = (u32)ioremap(u32BusStart, u32MapSize);
        else
            VirtAddr = (u32)ioremap_cached(u32BusStart, u32MapSize);
    }

    return VirtAddr;
}

u32 HAL_MsOS_MPool_PA2BA(u32 u32PhyAddr)
{
    u32 u32BusAddr = 0x0;

    // ba = pa + offset
    if((u32PhyAddr >= HAL_MIU0_BASE) && (u32PhyAddr < HAL_MIU1_BASE))	// MIU0
        u32BusAddr = u32PhyAddr - HAL_MIU0_BASE + HAL_MIU0_BUS_BASE;
    else																// MIU1
        u32BusAddr = u32PhyAddr - HAL_MIU1_BASE + HAL_MIU1_BUS_BASE;

    return u32BusAddr;
}
#elif defined(HAL_I3_SIMULATE)
#define HAL_MIU1_BASE               0x60000000 // 1512MB
#else
#define HAL_MIU1_BASE               0x60000000 // 1512MB
#endif

bool _MdrvCmdqDcacheFlush(u32 pAddrVirt, u32 tSize)
{
#if defined(HAL_K6_SIMULATE)
    Chip_Flush_Cache_Range(pAddrVirt, tSize);
#elif defined(HAL_I3_SIMULATE)
    if(pAddrVirt | tSize) {}
    Chip_Flush_MIU_Pipe();
#elif defined(CAM_OS_RTK)
    if(pAddrVirt | tSize) {}
#else
    if(pAddrVirt | tSize) {}
#if 0/*remove it mi allocate nocache memory*/
    Chip_Flush_Cache_Range(pAddrVirt, tSize);
#endif
#endif
    return true;
}

CmdqInterfacePriv_t * _MdrvGetCmdqPrivData(u32 nDrvIpId, DrvCcmdqId_e eCmdId)
{
    int i = 0;

    for(i = 0; i < EN_CMDQ_TYPE_MAX; i++)
    {
        if(gDrvCmdPriv[i].bUsed && gDrvCmdPriv[i].CmdqIpId == eCmdId)
        {
            CMDQ_ERR("%s private data have allocated(%d)\n", __func__, eCmdId);
            return NULL;
        }
    }


    for(i = 0; i < EN_CMDQ_TYPE_MAX; i++)
    {
        if(!gDrvCmdPriv[i].bUsed && gDrvCmdPriv[i].nCmdqDrvIpId == nDrvIpId)
        {
            gDrvCmdPriv[i].CmdqIpId = eCmdId;
            gDrvCmdPriv[i].bUsed = 1;
            CamOsMutexInit(&gDrvCmdPriv[i].tCmdqMutex);
            CamOsMutexInit(&gDrvCmdPriv[i].tMloadMutex);
            return &gDrvCmdPriv[i];
        }
    }

    return NULL;
}

int  _MdrvGetCmdqTrigEventBit(void *pPriv, DRV_CMDQ_EVENT_ID nTrigEvn)
{
    int i;
    CmdqInterfacePriv_t *ptCmdPriv = (CmdqInterfacePriv_t *)pPriv;

    for(i = 0; i < DVRCMDQ_TRIG_EVENT_NUM; i++)
    {
        if(nTrigEvn == ptCmdPriv->tDrvCmdTrigEvt[i].eTrigEvnt)
            return ptCmdPriv->tDrvCmdTrigEvt[i].nBit;

    }

    CMDQ_ERR("%s no support cmdq event(0x%x-0x%x)\n", __func__, nTrigEvn, ptCmdPriv->nCmdqDrvIpId);
    return -1;
}

void _MdrvInitCmdqTrigEvent(CmdqInterfacePriv_t *ptCmdPriv)
{
    switch(ptCmdPriv->nCmdqDrvIpId)
    {
        case EN_CMDQ_TYPE_IP0:
            ptCmdPriv->tDrvCmdTrigEvt[0].eTrigEvnt = DRVCMDQEVE_ISP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[0].nBit      = 0;
            ptCmdPriv->tDrvCmdTrigEvt[1].eTrigEvnt = DRVCMDQEVE_SC_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[1].nBit      = 1;
            ptCmdPriv->tDrvCmdTrigEvt[2].eTrigEvnt = DRVCMDQEVE_SC_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[2].nBit      = 2;
            ptCmdPriv->tDrvCmdTrigEvt[3].eTrigEvnt = DRVCMDQEVE_GOP_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[3].nBit      = 3;
            ptCmdPriv->tDrvCmdTrigEvt[4].eTrigEvnt = DRVCMDQEVE_GOP_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[4].nBit      = 4;
            ptCmdPriv->tDrvCmdTrigEvt[5].eTrigEvnt = DRVCMDQEVE_GOP_TRIG4;
            ptCmdPriv->tDrvCmdTrigEvt[5].nBit      = 5;
            ptCmdPriv->tDrvCmdTrigEvt[6].eTrigEvnt = DRVCMDQEVE_DIP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[6].nBit      = 6;
            ptCmdPriv->tDrvCmdTrigEvt[7].eTrigEvnt = DRVCMDQEVE_CORE0_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[7].nBit      = 7;
            ptCmdPriv->tDrvCmdTrigEvt[8].eTrigEvnt = DRVCMDQEVE_CORE1_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[8].nBit      = 8;
            ptCmdPriv->tDrvCmdTrigEvt[9].eTrigEvnt  = DRVCMDQEVE_CORE0_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[9].nBit       = 9;
            ptCmdPriv->tDrvCmdTrigEvt[10].eTrigEvnt = DRVCMDQEVE_CORE1_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[10].nBit      = 10;
            ptCmdPriv->tDrvCmdTrigEvt[11].eTrigEvnt = DRVCMDQEVE_GE_CMDQ_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[11].nBit      = 11;
            ptCmdPriv->tDrvCmdTrigEvt[12].eTrigEvnt = DRVCMDQEVE_LDC_CMDQ_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[12].nBit      = 12;
            ptCmdPriv->tDrvCmdTrigEvt[13].eTrigEvnt = DRVCMDQEVE_REG_DUMMY_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[13].nBit      = 13;
            ptCmdPriv->tDrvCmdTrigEvt[14].eTrigEvnt = DRVCMDQEVE_S0_MGW_FIRE;
            ptCmdPriv->tDrvCmdTrigEvt[14].nBit      = 14;
            ptCmdPriv->tDrvCmdTrigEvt[15].eTrigEvnt = DRVCMDQEVE_S0_MDW_W_DONE;
            ptCmdPriv->tDrvCmdTrigEvt[15].nBit      = 15;
            break;
#if  !defined(HAL_K6_SIMULATE) && !defined(HAL_I3_SIMULATE)
        case EN_CMDQ_TYPE_IP1:
            ptCmdPriv->tDrvCmdTrigEvt[0].eTrigEvnt = DRVCMDQEVE_ISP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[0].nBit      = 0;
            ptCmdPriv->tDrvCmdTrigEvt[1].eTrigEvnt = DRVCMDQEVE_SC_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[1].nBit      = 1;
            ptCmdPriv->tDrvCmdTrigEvt[2].eTrigEvnt = DRVCMDQEVE_SC_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[2].nBit      = 2;
            ptCmdPriv->tDrvCmdTrigEvt[3].eTrigEvnt = DRVCMDQEVE_GOP_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[3].nBit      = 3;
            ptCmdPriv->tDrvCmdTrigEvt[4].eTrigEvnt = DRVCMDQEVE_GOP_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[4].nBit      = 4;
            ptCmdPriv->tDrvCmdTrigEvt[5].eTrigEvnt = DRVCMDQEVE_GOP_TRIG4;
            ptCmdPriv->tDrvCmdTrigEvt[5].nBit      = 5;
            ptCmdPriv->tDrvCmdTrigEvt[6].eTrigEvnt = DRVCMDQEVE_DIP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[6].nBit      = 6;
            ptCmdPriv->tDrvCmdTrigEvt[7].eTrigEvnt = DRVCMDQEVE_CORE0_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[7].nBit      = 7;
            ptCmdPriv->tDrvCmdTrigEvt[8].eTrigEvnt = DRVCMDQEVE_CORE1_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[8].nBit      = 8;
            ptCmdPriv->tDrvCmdTrigEvt[9].eTrigEvnt  = DRVCMDQEVE_CORE0_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[9].nBit       = 9;
            ptCmdPriv->tDrvCmdTrigEvt[10].eTrigEvnt = DRVCMDQEVE_CORE1_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[10].nBit      = 10;
            ptCmdPriv->tDrvCmdTrigEvt[11].eTrigEvnt = DRVCMDQEVE_REG_DUMMY_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[11].nBit      = 11;
            ptCmdPriv->tDrvCmdTrigEvt[12].eTrigEvnt = DRVCMDQEVE_S1_MGW_FIRE;
            ptCmdPriv->tDrvCmdTrigEvt[12].nBit      = 12;
            ptCmdPriv->tDrvCmdTrigEvt[13].eTrigEvnt = DRVCMDQEVE_S1_MDW_W_DONE;
            ptCmdPriv->tDrvCmdTrigEvt[13].nBit      = 13;
            ptCmdPriv->tDrvCmdTrigEvt[14].eTrigEvnt = DRVCMDQEVE_DMAGEN_TRIGGER0;
            ptCmdPriv->tDrvCmdTrigEvt[14].nBit      = 14;
            ptCmdPriv->tDrvCmdTrigEvt[15].eTrigEvnt = DRVCMDQEVE_DMAGEN_TRIGGER1;
            ptCmdPriv->tDrvCmdTrigEvt[15].nBit      = 15;
            break;
        case EN_CMDQ_TYPE_IP2:
            ptCmdPriv->tDrvCmdTrigEvt[0].eTrigEvnt = DRVCMDQEVE_ISP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[0].nBit      = 0;
            ptCmdPriv->tDrvCmdTrigEvt[1].eTrigEvnt = DRVCMDQEVE_SC_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[1].nBit      = 1;
            ptCmdPriv->tDrvCmdTrigEvt[2].eTrigEvnt = DRVCMDQEVE_SC_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[2].nBit      = 2;
            ptCmdPriv->tDrvCmdTrigEvt[3].eTrigEvnt = DRVCMDQEVE_GOP_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[3].nBit      = 3;
            ptCmdPriv->tDrvCmdTrigEvt[4].eTrigEvnt = DRVCMDQEVE_GOP_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[4].nBit      = 4;
            ptCmdPriv->tDrvCmdTrigEvt[5].eTrigEvnt = DRVCMDQEVE_GOP_TRIG4;
            ptCmdPriv->tDrvCmdTrigEvt[5].nBit      = 5;
            ptCmdPriv->tDrvCmdTrigEvt[6].eTrigEvnt = DRVCMDQEVE_DIP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[6].nBit      = 6;
            ptCmdPriv->tDrvCmdTrigEvt[7].eTrigEvnt = DRVCMDQEVE_CORE0_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[7].nBit      = 7;
            ptCmdPriv->tDrvCmdTrigEvt[8].eTrigEvnt = DRVCMDQEVE_CORE1_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[8].nBit      = 8;
            ptCmdPriv->tDrvCmdTrigEvt[9].eTrigEvnt  = DRVCMDQEVE_CORE0_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[9].nBit       = 9;
            ptCmdPriv->tDrvCmdTrigEvt[10].eTrigEvnt = DRVCMDQEVE_CORE1_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[10].nBit      = 10;
            ptCmdPriv->tDrvCmdTrigEvt[11].eTrigEvnt = DRVCMDQEVE_GE_CMDQ_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[11].nBit      = 11;
            ptCmdPriv->tDrvCmdTrigEvt[12].eTrigEvnt = DRVCMDQEVE_LDC_CMDQ_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[12].nBit      = 12;
            ptCmdPriv->tDrvCmdTrigEvt[13].eTrigEvnt = DRVCMDQEVE_IVE_CMDQ_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[13].nBit      = 13;
            ptCmdPriv->tDrvCmdTrigEvt[14].eTrigEvnt = DRVCMDQEVE_BDMA_TRIGGER0;
            ptCmdPriv->tDrvCmdTrigEvt[14].nBit      = 14;
            ptCmdPriv->tDrvCmdTrigEvt[15].eTrigEvnt = DRVCMDQEVE_BDMA_TRIGGER1;
            ptCmdPriv->tDrvCmdTrigEvt[15].nBit      = 15;
            break;
        case EN_CMDQ_TYPE_IP3:
            ptCmdPriv->tDrvCmdTrigEvt[0].eTrigEvnt = DRVCMDQEVE_ISP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[0].nBit      = 0;
            ptCmdPriv->tDrvCmdTrigEvt[1].eTrigEvnt = DRVCMDQEVE_SC_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[1].nBit      = 1;
            ptCmdPriv->tDrvCmdTrigEvt[2].eTrigEvnt = DRVCMDQEVE_SC_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[2].nBit      = 2;
            ptCmdPriv->tDrvCmdTrigEvt[3].eTrigEvnt = DRVCMDQEVE_GOP_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[3].nBit      = 3;
            ptCmdPriv->tDrvCmdTrigEvt[4].eTrigEvnt = DRVCMDQEVE_GOP_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[4].nBit      = 4;
            ptCmdPriv->tDrvCmdTrigEvt[5].eTrigEvnt = DRVCMDQEVE_GOP_TRIG4;
            ptCmdPriv->tDrvCmdTrigEvt[5].nBit      = 5;
            ptCmdPriv->tDrvCmdTrigEvt[6].eTrigEvnt = DRVCMDQEVE_DIP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[6].nBit      = 6;
            ptCmdPriv->tDrvCmdTrigEvt[7].eTrigEvnt = DRVCMDQEVE_CORE0_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[7].nBit      = 7;
            ptCmdPriv->tDrvCmdTrigEvt[8].eTrigEvnt = DRVCMDQEVE_CORE1_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[8].nBit      = 8;
            ptCmdPriv->tDrvCmdTrigEvt[9].eTrigEvnt  = DRVCMDQEVE_CORE0_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[9].nBit       = 9;
            ptCmdPriv->tDrvCmdTrigEvt[10].eTrigEvnt = DRVCMDQEVE_CORE1_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[10].nBit      = 10;
            ptCmdPriv->tDrvCmdTrigEvt[11].eTrigEvnt = DRVCMDQEVE_REG_DUMMY_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[11].nBit      = 11;
            ptCmdPriv->tDrvCmdTrigEvt[12].eTrigEvnt = DRVCMDQEVE_S1_MGW_FIRE;
            ptCmdPriv->tDrvCmdTrigEvt[12].nBit      = 12;
            ptCmdPriv->tDrvCmdTrigEvt[13].eTrigEvnt = DRVCMDQEVE_S1_MDW_W_DONE;
            ptCmdPriv->tDrvCmdTrigEvt[13].nBit      = 13;
            ptCmdPriv->tDrvCmdTrigEvt[14].eTrigEvnt = DRVCMDQEVE_DMAGEN_TRIGGER0;
            ptCmdPriv->tDrvCmdTrigEvt[14].nBit      = 14;
            ptCmdPriv->tDrvCmdTrigEvt[15].eTrigEvnt = DRVCMDQEVE_DMAGEN_TRIGGER1;
            ptCmdPriv->tDrvCmdTrigEvt[15].nBit      = 15;
            break;
        case EN_CMDQ_TYPE_IP4:
            ptCmdPriv->tDrvCmdTrigEvt[0].eTrigEvnt = DRVCMDQEVE_ISP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[0].nBit      = 0;
            ptCmdPriv->tDrvCmdTrigEvt[1].eTrigEvnt = DRVCMDQEVE_SC_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[1].nBit      = 1;
            ptCmdPriv->tDrvCmdTrigEvt[2].eTrigEvnt = DRVCMDQEVE_SC_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[2].nBit      = 2;
            ptCmdPriv->tDrvCmdTrigEvt[3].eTrigEvnt = DRVCMDQEVE_GOP_TRIG013;
            ptCmdPriv->tDrvCmdTrigEvt[3].nBit      = 3;
            ptCmdPriv->tDrvCmdTrigEvt[4].eTrigEvnt = DRVCMDQEVE_GOP_TRIG2;
            ptCmdPriv->tDrvCmdTrigEvt[4].nBit      = 4;
            ptCmdPriv->tDrvCmdTrigEvt[5].eTrigEvnt = DRVCMDQEVE_GOP_TRIG4;
            ptCmdPriv->tDrvCmdTrigEvt[5].nBit      = 5;
            ptCmdPriv->tDrvCmdTrigEvt[6].eTrigEvnt = DRVCMDQEVE_DIP_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[6].nBit      = 6;
            ptCmdPriv->tDrvCmdTrigEvt[7].eTrigEvnt = DRVCMDQEVE_CORE0_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[7].nBit      = 7;
            ptCmdPriv->tDrvCmdTrigEvt[8].eTrigEvnt = DRVCMDQEVE_CORE1_MFE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[8].nBit      = 8;
            ptCmdPriv->tDrvCmdTrigEvt[9].eTrigEvnt  = DRVCMDQEVE_CORE0_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[9].nBit       = 9;
            ptCmdPriv->tDrvCmdTrigEvt[10].eTrigEvnt = DRVCMDQEVE_CORE1_MHE_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[10].nBit      = 10;
            ptCmdPriv->tDrvCmdTrigEvt[11].eTrigEvnt = DRVCMDQEVE_REG_DUMMY_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[11].nBit      = 11;
            ptCmdPriv->tDrvCmdTrigEvt[12].eTrigEvnt = DRVCMDQEVE_S0_MGW_FIRE;
            ptCmdPriv->tDrvCmdTrigEvt[12].nBit      = 12;
            ptCmdPriv->tDrvCmdTrigEvt[13].eTrigEvnt = DRVCMDQEVE_IVE_CMDQ_TRIG;
            ptCmdPriv->tDrvCmdTrigEvt[13].nBit      = 13;
            ptCmdPriv->tDrvCmdTrigEvt[14].eTrigEvnt = DRVCMDQEVE_BDMA_TRIGGER0;
            ptCmdPriv->tDrvCmdTrigEvt[14].nBit      = 14;
            ptCmdPriv->tDrvCmdTrigEvt[15].eTrigEvnt = DRVCMDQEVE_S0_MDW_W_DONE;
            ptCmdPriv->tDrvCmdTrigEvt[15].nBit      = 15;
            break;
#endif
        default:
            CMDQ_ERR("%s no support DRVIPID(%d)\n", __func__, ptCmdPriv->nCmdqDrvIpId);
            break;
    }
}

void _MdrvDeInitCmdqPrivData(CmdqInterfacePriv_t *pPrivData)
{
    pPrivData->bUsed = 0;
    pPrivData->bFirstFireDone  = 0;
#if DRVCMDQ_ENABLE_IRQ
    DrvCmdqIrqStatusDeInit(pPrivData->nCmdqDrvIpId,pPrivData->nCmdqIrq,(void*)pPrivData);
#endif

#if DRVCMDQ_ENABLE_DUMMY_IRQ
    DrvCmdqIrqHwDummyDeInit(pPrivData->nCmdqDrvIpId,gDrvCmdqHWDummyIrqNumber[pPrivData->nCmdqDrvIpId],(void*)pPrivData);
#endif

#if !defined(HAL_I2_SIMULATE_NO_HW)
    HAL_CMDQ_Stop(pPrivData->nCmdqDrvIpId);
#endif

    CamOsMutexDestroy(&pPrivData->tCmdqMutex);
    CamOsMutexDestroy(&pPrivData->tMloadMutex);
    //CamOsTsemDeinit(&pPrivData->tWaitQue);
    //flush_workqueue(pPrivData->pQueuwork);
    memset(pPrivData->nForbiddenTag,0x0,sizeof(u16)*DRVCMD_FORBIDDENTAG_NUM);
}

void _MdrvInitCmdqPrivData(void)
{
    int i = 0;
    memset(&gDrvCmdPriv[0], 0x0, sizeof(CmdqInterfacePriv_t)*EN_CMDQ_TYPE_MAX);

    for(i = 0; i < EN_CMDQ_TYPE_MAX; i++)
    {
        gDrvCmdPriv[i].bUsed = 0;
        gDrvCmdPriv[i].bFirstFireDone = 0;
        gDrvCmdPriv[i].nCmdqDrvIpId = i;
#if defined(USE_DIRECTMODE)
        gDrvCmdPriv[i].nCmdMode     = DIRECT_MODE;
#else
        gDrvCmdPriv[i].nCmdMode     = RING_BUFFER_MODE;
#endif
        gDrvCmdPriv[i].nCmdqRiuBase = CMDQ_RIU_BASE;
        gDrvCmdPriv[i].nCmdqIrq     = gDrvCmdqIrqNumber[i];
        //CamOsMutexInit(&gDrvCmdPriv[i].tCmdqMutex);
        //CamOsMutexInit(&gDrvCmdPriv[i].tMloadMutex);
        //CamOsTsemInit(&gDrvCmdPriv[i].tWaitQue, 0);

#if DRVCMDQ_ENABLE_IRQ
        //CamOsTsemInit(&gDrvCmdPriv[i].tIrqWaitSem, 0);
#endif
        memset(gDrvCmdPriv[i].nForbiddenTag,0x0,sizeof(u16)*DRVCMD_FORBIDDENTAG_NUM);
    }

    for(i = 0; i < EN_CMDQ_TYPE_MAX; i++)
    {
        _MdrvInitCmdqTrigEvent(&gDrvCmdPriv[i]);
    }
}

s32 _MDrvCmdqSetBuffer(DrvCmdqIPSupport_e eIpNum, u32 StartAddr, u32 EndAddr)
{
    if(eIpNum >= EN_CMDQ_TYPE_MAX)
    {
        CMDQ_ERR("%s IPid is ill(%d)\n", __func__, eIpNum);
        return -1;
    }

    HAL_CMDQ_Set_Start_Pointer(eIpNum, StartAddr);
    HAL_CMDQ_Set_End_Pointer(eIpNum, EndAddr);
    HAL_CMDQ_Write_Pointer(eIpNum, StartAddr);

    CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "set cmd%d buffer from 0x%08x to 0x%08x\n", eIpNum, StartAddr, EndAddr);

    return 0;
}

u32 _MDrvCmdqTransPhyBuf2VirBuf(CmdqInterfacePriv_t* pPrivData, u32 PhyBufAddr)
{
    u32 nOffSetSize, nVirBufAddr;

    if(pPrivData  == NULL)
    {
        CMDQ_ERR("%s priv data error\n", __func__);
        return 0;
    }

    if(PhyBufAddr < pPrivData->tCmdqBuf.BufPhy)
    {
        CMDQ_ERR("%s phy addr is ill(0x%08x-0x%08x)\n", __func__, PhyBufAddr, pPrivData->tCmdqBuf.BufPhy);
        return 0;
    }

    nOffSetSize = PhyBufAddr - pPrivData->tCmdqBuf.BufPhy;
    nVirBufAddr = pPrivData->tCmdqBuf.BufVir + nOffSetSize;
    return nVirBufAddr;
}
u32 _MDrvCmdqTransMloadPhyBuf2MloadVirBuf(CmdqInterfacePriv_t* pPrivData, u32 PhyBufAddr)
{
    u32 nOffSetSize, nVirBufAddr;

    if(pPrivData  == NULL)
    {
        CMDQ_ERR("%s priv data error\n", __func__);
        return 0;
    }

    if(PhyBufAddr < pPrivData->tMloadBuf.BufPhy)
    {
        CMDQ_ERR("%s phy addr is ill(0x%08x-0x%08x)\n", __func__, PhyBufAddr, pPrivData->tMloadBuf.BufPhy);
        return 0;
    }

    nOffSetSize = PhyBufAddr - pPrivData->tMloadBuf.BufPhy;
    nVirBufAddr = pPrivData->tMloadBuf.BufVir + nOffSetSize;
    return nVirBufAddr;
}
u32 _MDrvCmdqTransVirBuf2PhyBuf(CmdqInterfacePriv_t* pPrivData, u32  VirBufAddr)
{
    u32 nOffSetSize, nPhyBufAddr;

    if(pPrivData  == NULL)
    {
        CMDQ_ERR("%s priv data error\n", __func__);
        return 0;
    }

    if(VirBufAddr < pPrivData->tCmdqBuf.BufVir)
    {
        CMDQ_ERR("%s vir addr is ill(0x%08x-0x%08x)\n", __func__, VirBufAddr, pPrivData->tCmdqBuf.BufVir);
        return 0;
    }

    nOffSetSize = VirBufAddr - pPrivData->tCmdqBuf.BufVir;
    nPhyBufAddr = pPrivData->tCmdqBuf.BufPhy + nOffSetSize;
    return nPhyBufAddr;
}

u32 _MDrvCmdqTransVirMloadBuf2PhyMloadBuf(CmdqInterfacePriv_t* pPrivData, u32  VirBufAddr)
{
    u32 nOffSetSize, nPhyBufAddr;

    if(pPrivData  == NULL)
    {
        CMDQ_ERR("%s priv data error\n", __func__);
        return 0;
    }

    if(VirBufAddr < pPrivData->tMloadBuf.BufVir)
    {
        CMDQ_ERR("mload vir addr is ill(0x%08x-0x%08x)\n", VirBufAddr, pPrivData->tMloadBuf.BufVir);
        return 0;
    }

    nOffSetSize = VirBufAddr - pPrivData->tMloadBuf.BufVir;
    nPhyBufAddr = pPrivData->tMloadBuf.BufPhy + nOffSetSize;
    return nPhyBufAddr;
}

u32 _MdrvCmdTransToDrvIpId(DrvCcmdqId_e eCmdId)
{
#if defined(HAL_K6_SIMULATE) || defined(HAL_I3_SIMULATE)
    switch(eCmdId)
    {
        case DRVCMDQ_ID_VPE:
        case DRVCMDQ_ID_DIVP:
        case DRVCMDQ_ID_H265_VENC0:
        case DRVCMDQ_ID_H265_VENC1:
        case DRVCMDQ_ID_H264_VENC0:
        default:
            return EN_CMDQ_TYPE_IP0;
    }
#else
    switch(eCmdId)
    {
        case DRVCMDQ_ID_VPE:
            return EN_CMDQ_TYPE_IP0;
            break;
        case DRVCMDQ_ID_DIVP:
            return EN_CMDQ_TYPE_IP1;
            break;
        case DRVCMDQ_ID_H265_VENC0:
            return EN_CMDQ_TYPE_IP2;
            break;
        case DRVCMDQ_ID_H265_VENC1:
            return EN_CMDQ_TYPE_IP3;
            break;
        case DRVCMDQ_ID_H264_VENC0:
            return EN_CMDQ_TYPE_IP4;
            break;
        default:
            break;
    }
#endif
    CMDQ_ERR("_MdrvCmdTransToDrvIpId ERROR CMDID(%d)\n",eCmdId);
    return EN_CMDQ_TYPE_IP0;
    //panic("_MdrvCmdTransToDrvIpId ERROR CMDID(%d)\n", eCmdId);
}

u32 _MDrvCmdqCheckMloadBufferAvail(void* pPriv, u32 nBufSize, u16 align)
{
    u32 s, e, r, w;
    u32 checkbufsz = 0;
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    s = (u32)pPrivData->tMloadInfo.pBufStart;
    e = (u32)pPrivData->tMloadInfo.pBufEnd;
    w = (u32)pPrivData->tMloadInfo.pBufWrite;
    r = (u32)pPrivData->tMloadInfo.pBufReadHw;

    checkbufsz = CMDQ_DO_SIZE_ALIGN(nBufSize, align);

    //@NOTE need to handle out of range and rdptr reseted value.
    if(r > e || r < s)
    {
        CMDQ_ERR("readp out of buffer range.(0x%x-0x%x-0x%x)\n", s, e, r);
        //return _ctx[hnd].TOTAL_BUFFER_SIZE;
        r = s;
        return 0;
    }

    if(w > e || w < s)
    {
        CMDQ_ERR("writep out of buffer range.(0x%x-0x%x-0x%x)\n", s, e, w);
        //return _ctx[hnd].TOTAL_BUFFER_SIZE;
        w = s;
        return 0;
    }

    if(w == r)
    {
        pPrivData->tMloadInfo.pBufWrite =  pPrivData->tMloadInfo.pBufStart;
        pPrivData->tMloadInfo.pBufReadHw  =  pPrivData->tMloadInfo.pBufStart;
        return pPrivData->tMloadBuf.nBufSize;
    }
    else
    {
        if(w > r)
        {
            if((e - w) >= checkbufsz)
            {
                return (e - w);
            }
            else
            {
                if((r - s) >= checkbufsz)
                {
                    pPrivData->tMloadInfo.pBufWrite =  pPrivData->tMloadInfo.pBufStart;
                    return (r - s);
                }
                else
                    return 0;
            }
        }
        else
        {
            if((r - w) >= checkbufsz)
                return (r - w);
            else
                return 0;
        }

    }

    return 0;
}

u32 _MDrvCmdqCopyMloadBuf(void* pPriv, void * mload_buf, u32 nBufSize, u16 u16Alignment)
{
    u32 phystar;
    u32 checkbufsz = 0;
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    checkbufsz = CMDQ_DO_SIZE_ALIGN(nBufSize, u16Alignment);

    phystar = (u32)pPrivData->tMloadInfo.pBufWrite;
    memcpy((void*)pPrivData->tMloadInfo.pBufWrite, mload_buf, checkbufsz);
    _MdrvCmdqDcacheFlush((u32)pPrivData->tMloadInfo.pBufWrite, checkbufsz);
    pPrivData->tMloadInfo.pBufWrite = (u8*)((u32)pPrivData->tMloadInfo.pBufWrite + checkbufsz);
    phystar = _MDrvCmdqTransVirMloadBuf2PhyMloadBuf(pPrivData, phystar);

    return phystar;
}

u32 _MDrvCmdqGetCmdqMiuNumber(void* pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    if(pPrivData->tCmdqBuf.BufPhy >= HAL_MIU1_BASE)
        return EN_CMDQ_MIU_1;

    return EN_CMDQ_MIU_0;
}



u32 _MDrvCmdqCheckBufferAvail(void* pPriv, u32 nCmdqNum)
{
#if defined(USE_DIRECTMODE)
    u32 s, e,  w;
    //u32 checkbuf = 0;
    CmdqInterfacePriv_t * pPrivData = NULL;
    u32        nRetVal = 0;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        return nCmdqNum;
    }

    s = (u32)pPrivData->tCmdqInfo.pBufStart;
    e = (u32)pPrivData->tCmdqInfo.pBufEnd;
    w = (u32)pPrivData->tCmdqInfo.pBufWrite;

    nRetVal = (nCmdqNum <= ((e - w) / DVRCMDQ_COMMAND_SIZE)) ? ((e - w) / DVRCMDQ_COMMAND_SIZE) : 0;
    return nRetVal;
#else
    u32 s, e, r, w;
    //u32 checkbuf = 0;
    CmdqInterfacePriv_t * pPrivData = NULL;
    u32        nRetVal = 0;

    _CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_DRV_CHECK2);

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_DRV_CHECK2);

        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_DRV_CHECK2);

        return nCmdqNum;
    }

    s = (u32)pPrivData->tCmdqInfo.pBufStart;
    e = (u32)pPrivData->tCmdqInfo.pBufEnd;
    w = (u32)pPrivData->tCmdqInfo.pBufWrite;

    if(pPrivData->bFirstFireDone)
    {
        _CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_CHECK_BUF);
        r = HAL_CMDQ_Read_Pointer(pPrivData->nCmdqDrvIpId);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_CHECK_BUF);
        pPrivData->tCmdqInfo.pBufReadHw = (u8*)r;
        r = r << DRVCMD_BUFFER_SHIFT;
        //r = _MDrvCmdqTransPhyBuf2VirBuf(pPrivData, r);
        CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "%s ReadHw=0x%08x\n", __func__, r);
    }
    else
        r = s;

    if(nCmdqNum % DVRCMDQ_COMMAND_ALIGN)
        nCmdqNum += 1;

    //@NOTE need to handle out of range and rdptr reseted value.
    if(r > e || r < s)
    {
        CMDQ_ERR("[CMDQ]check buf rdptr out of buffer range.(0x%x-0x%x-0x%x-0x%x)\n", s, e, w,r);
        //return _ctx[hnd].TOTAL_BUFFER_SIZE;
        r = s;
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_DRV_CHECK2);

        return 0;
    }

    if(pPrivData->nCmdMode == RING_BUFFER_MODE)
    {
        //@NOTE: actually, rdptr will equal to wrptr.
        if(w == r)
        {
            //@NOTE: because if we fill buffer full, that rd=wr will not trigger.

            nRetVal = (nCmdqNum <= (pPrivData->tCmdqInfo.nTotalCmdCount)) ? (pPrivData->tCmdqInfo.nTotalCmdCount) : 0;
        }
        else if(r > w)
        {
            //|S|......W_________R......|E|
            nRetVal = (nCmdqNum <= ((r - w) / DVRCMDQ_COMMAND_SIZE)) ? ((r - w) / DVRCMDQ_COMMAND_SIZE) : 0;
        }
        else if(w > r)
        {
            //|S|______R.........W______|E|
            nRetVal = (nCmdqNum <= ((((r - s) + (e - w)) / DVRCMDQ_COMMAND_SIZE) + 1)) ? ((((r - s) + (e - w)) / DVRCMDQ_COMMAND_SIZE) + 1) : 0;
        }
        else
        {
            CMDQ_ERR("Internal cmdq error(0x%08x-0x%08x)!\n", w, r);
            //panic("Internal cmdq error(0x%x-0x%x)!\n",w,r);
        }
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_DRV_CHECK2);

        return nRetVal;
    }
    else
    {
        CMDQ_ERR("We only support RING_BUFFER_MODE ONLY!\n");
        //panic("We only support RING_BUFFER_MODE ONLY!\n");
    }
    //CMDQ_ERR("[CMDQ]ptr out of buffer range.(0x%x-0x%x-0x%x-0x%x)\n", s, e, w,r);
    _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_DRV_CHECK2);
    return 0;
#endif
}
u32 _MdrvCmdAddForbiddenTag(void* pPriv,u16 nTagValue)
{
    u32 i;
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }
    pPrivData = (CmdqInterfacePriv_t*)pPriv;
    for(i = 0;i<DRVCMD_FORBIDDENTAG_NUM;i++)
    {
        if(pPrivData->nForbiddenTag[i] == 0)
        {
            pPrivData->nForbiddenTag[i] = nTagValue;
            break;
        }
    }
    return 0;
}
u32 _MdrvCmdCheckForbiddenTag(void* pPriv,u16 nTagValue)
{
    u32 i;
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }
    pPrivData = (CmdqInterfacePriv_t*)pPriv;
    for(i = 0;i<DRVCMD_FORBIDDENTAG_NUM;i++)
    {
        if(nTagValue == pPrivData->nForbiddenTag[i])
            return 1;
        if(pPrivData->nForbiddenTag[i] == 0)
            break;
    }
    return 0;
}

s32 _MdrvCmdCalcuTimeoutVlaue(u32 nTimens,u32 *pTimeBase,u32 *pTimeCount)
{
    u32 i,min=0xffffffff;
    u32 nClk,bfound = 0;
    u32 nTotalNs;
    u32 nTempTimeDist;
    u32 nTempTimeCount,nBestTimeCount,nBestTimeBase;

    nClk = (1000*10)/DRVCMDQ_MIU_CLK;/*ns base , miu is 333 mhz*/

    nTotalNs =((nTimens*10)/nClk);

    if(nTotalNs == 0)
        nTotalNs = 1;

    if(nTotalNs <=0xfffff)
    {
        nBestTimeCount = nTotalNs;
        nBestTimeBase  = 1;
        bfound = 1;
    }
    else
    {
        for(i=0x1;i<=0xff;i++)
        {
            nTempTimeCount = nTotalNs / i;

            if(nTempTimeCount > 0xfffff)
                continue;

            if((nTempTimeCount*i) >= nTotalNs)
                nTempTimeDist = (nTempTimeCount*i) - nTotalNs;
            else
                nTempTimeDist = nTotalNs - (nTempTimeCount*i);

            if(min > nTempTimeDist)
            {
                nBestTimeCount = nTempTimeCount;
                nBestTimeBase = i;
                min = nTempTimeDist;
                bfound = 1;
            }
        }
    }

    if(!bfound)
    {
        nBestTimeCount = 0xfffff;
        nBestTimeBase  = 0xff;
    }

    *pTimeCount = (nBestTimeCount&0xfffff);
    *pTimeBase  = (nBestTimeBase&0xff);
    return 0;
}

s32 _MdrvCmdCalcuPollTimeVlaue(u32 nTimens,u32 *pTimeWait,u32 *pTimeRiato)
{
    u32 i,min=0xffffffff;
    u32 nClk,bfound = 0;
    u32 nTotalNs;
    u32 nTempTimeDist;
    u32 nTempTimeCount,nBestTimeCount,nBestTimeBase;

    nClk = (1000*10)/DRVCMDQ_MIU_CLK;/*ns base , miu is 333 mhz*/

    nTotalNs =((nTimens*10)/nClk);

    if(nTotalNs == 0)
        nTotalNs = 1;


    for(i=0xff;i>=0x1;i--)
    {
        nTempTimeCount = nTotalNs / (i+1);

        if(nTempTimeCount > 0xffff)
            continue;

        if((nTempTimeCount*i) >= nTotalNs)
            nTempTimeDist = (nTempTimeCount*(i+1)) - nTotalNs;
        else
            nTempTimeDist = nTotalNs - (nTempTimeCount*(i+1));

        if(min > nTempTimeDist)
        {
            nBestTimeCount = nTempTimeCount;
            nBestTimeBase = i;
            min = nTempTimeDist;
            bfound = 1;
        }
    }


    if(!bfound)
    {
        nBestTimeCount = 0xffff;
        nBestTimeBase  = 0xff;
    }

    *pTimeWait = (nBestTimeCount&0xffff);
    *pTimeRiato  = (nBestTimeBase&0xff);
    return 0;
}

u32 _MdrvCmdqCheckIsAbortTag(void* pPriv,u16 nTagValue)
{
#if defined(HAL_I3_SIMULATE) ||  defined(HAL_K6_SIMULATE)
    return 0;
#else
    CmdqInterfacePriv_t * pPrivData = NULL;
    u16                   nRegValue = 0;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }
    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    switch(pPrivData->nCmdqDrvIpId)
    {
        case EN_CMDQ_TYPE_IP0:
            if(nTagValue == 0)
                break;

            if(_MdrvCmdCheckForbiddenTag(pPriv,nTagValue))
                return 1;

            //READ vpe 0x152565 dummy register
            nRegValue =  R2BYTE(pPrivData->nCmdqRiuBase, 0x1525CA);

            if(nTagValue == nRegValue)
            {
                _MdrvCmdAddForbiddenTag(pPriv,nTagValue);
                return 1;
            }

            break;
        case EN_CMDQ_TYPE_IP1:
            if(nTagValue == 0)
                break;
            if(_MdrvCmdCheckForbiddenTag(pPriv,nTagValue))
                return 1;
            //READ dip 0x13347d dummy register
            nRegValue =  R2BYTE(pPrivData->nCmdqRiuBase, 0x1334FA);
            if(nTagValue == nRegValue)
            {
                _MdrvCmdAddForbiddenTag(pPriv,nTagValue);
                return 1;
            }
            break;
        case EN_CMDQ_TYPE_IP2:
        case EN_CMDQ_TYPE_IP3:
        case EN_CMDQ_TYPE_IP4:
        default:
            return 0;
    }
    return 0;
#endif
}

#ifdef HAL_K6_SIMULATE
void MDrvCmdqAllocateBuffer(void *pPriv)
{
    u32 k6_cmdq_phy_addr = 0x0668B000;
    u32 k6_cmdq_size     = 0x1000;
    u32 k6_mload_phy_addr = 0x0668C000;//dummy15
    u32 k6_mload_size     = 0x1000;
    u32 k6baseaddr ;
    void * pVirAddr;

    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        BUG();
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;
    k6baseaddr = HAL_MsOS_MPool_PA2BA(k6_cmdq_phy_addr);
    pVirAddr = (void*) map_kdriver_mem(k6baseaddr, k6_cmdq_size, false);

    pPrivData->tCmdqBuf.BufPhy = k6_cmdq_phy_addr;
    pPrivData->tCmdqBuf.BufVir = (u32)pVirAddr;
    pPrivData->tCmdqBuf.nBufSize = k6_cmdq_size;

    k6baseaddr = HAL_MsOS_MPool_PA2BA(k6_mload_phy_addr);
    pVirAddr = (void*) map_kdriver_mem(k6baseaddr, k6_cmdq_size, false);

    pPrivData->tMloadBuf.BufPhy = k6_mload_phy_addr;
    pPrivData->tMloadBuf.BufVir = (u32)pVirAddr;
    pPrivData->tMloadBuf.nBufSize = k6_mload_size;
}
void MDrvCmdqReleaseBuffer(void *pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        BUG();
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    if(pPrivData->tCmdqBuf.BufVir)
        iounmap((void*)pPrivData->tCmdqBuf.BufVir);

    if(pPrivData->tMloadBuf.BufVir)
        iounmap((void*)pPrivData->tMloadBuf.BufVir);

}
#elif defined(HAL_I3_SIMULATE)

static void* alloc_dmem(const char* name, unsigned int size, dma_addr_t *addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    if(0 != msys_request_dmem(&dmem))
    {
        return NULL;
    }
    *addr = dmem.phys;
    return (void *)((uintptr_t)dmem.kvirt);
}
static void free_dmem(const char* name, unsigned int size, void *virt, dma_addr_t addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    dmem.kvirt  = (unsigned long long)((uintptr_t)virt);
    dmem.phys   = (unsigned long long)((uintptr_t)addr);
    msys_release_dmem(&dmem);
}

void MDrvCmdqAllocateBuffer(void *pPriv)
{
    void *sg_vip_cmdq_vir_addr ;
    dma_addr_t  sg_vip_cmdq_bus_addr;

    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        BUG();
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    if(!(sg_vip_cmdq_vir_addr = alloc_dmem("DRVCMDQ",
                                           PAGE_ALIGN(0x1000),
                                           &sg_vip_cmdq_bus_addr)))
    {
        CMDQ_ERR("%s: unable to allocate screen memory\n", __FUNCTION__);
        return ;
    }
    CMDQ_LOG("[CMDQ]: CMDQ: Phy:%x  Vir:%x\n", sg_vip_cmdq_bus_addr, (u32)sg_vip_cmdq_vir_addr);

    pPrivData->tCmdqBuf.BufPhy = Chip_Phys_to_MIU(sg_vip_cmdq_bus_addr);
    pPrivData->tCmdqBuf.BufVir = (u32)sg_vip_cmdq_vir_addr;
    pPrivData->tCmdqBuf.nBufSize = 0x1000;
    memset(sg_vip_cmdq_vir_addr, 0x0, 0x1000);


    if(!(sg_vip_cmdq_vir_addr = alloc_dmem("CMDMENU",
                                           PAGE_ALIGN(0x1000),
                                           &sg_vip_cmdq_bus_addr)))
    {
        CMDQ_ERR("%s: unable to allocate screen memory\n", __FUNCTION__);
        return ;
    }
    CMDQ_LOG("[CMDQ]: MENULOAD: Phy:%x  Vir:%x\n", sg_vip_cmdq_bus_addr, (u32)sg_vip_cmdq_vir_addr);

    pPrivData->tMloadBuf.BufPhy = Chip_Phys_to_MIU(sg_vip_cmdq_bus_addr);
    pPrivData->tMloadBuf.BufVir = (u32)sg_vip_cmdq_vir_addr;
    pPrivData->tMloadBuf.nBufSize = 0x1000;
    memset(sg_vip_cmdq_vir_addr, 0x0, 0x1000);

}
void MDrvCmdqReleaseBuffer(void *pPriv)
{

    CmdqInterfacePriv_t * pPrivData = NULL;
    dma_addr_t  sg_vip_cmdq_bus_addr;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        BUG();
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;
    sg_vip_cmdq_bus_addr = Chip_MIU_to_Phys(pPrivData->tCmdqBuf.BufPhy);
    free_dmem("DRVCMDQ", pPrivData->tCmdqBuf.nBufSize, (void*)pPrivData->tCmdqBuf.BufVir, sg_vip_cmdq_bus_addr);
}
#elif defined(HAL_I2_SIMULATE)
static void* alloc_dmem(const char* name, unsigned int size, dma_addr_t *addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    if(0 != msys_request_dmem(&dmem))
    {
        return NULL;
    }
    *addr = dmem.phys;
    return (void *)((uintptr_t)dmem.kvirt);
}
static void free_dmem(const char* name, unsigned int size, void *virt, dma_addr_t addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    dmem.kvirt  = (unsigned long long)((uintptr_t)virt);
    dmem.phys   = (unsigned long long)((uintptr_t)addr);
    msys_release_dmem(&dmem);
}

void MDrvCmdqAllocateBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo)
{
    void *sg_vip_cmdq_vir_addr ;
    dma_addr_t  sg_vip_cmdq_bus_addr;

    if(!(sg_vip_cmdq_vir_addr = alloc_dmem("DRVCMDQ",
                                           DRVCMDQ_CMDQBUFFER_SIZE,
                                           &sg_vip_cmdq_bus_addr)))
    {
        CMDQ_ERR("%s: unable to allocate screen memory\n", __FUNCTION__);
        return ;
    }
    CMDQ_LOG("[CMDQ]: CMDQ: Phy:%x  Vir:%x\n", sg_vip_cmdq_bus_addr, (u32)sg_vip_cmdq_vir_addr);

    pCmdqMmapInfo->u32CmdqMmapPhyAddr = Chip_Phys_to_MIU(sg_vip_cmdq_bus_addr);
    pCmdqMmapInfo->u32CmdqMmapVirAddr = (u32)sg_vip_cmdq_vir_addr;
    pCmdqMmapInfo->u32CmdqMmapSize    = DRVCMDQ_CMDQBUFFER_SIZE;

    memset(sg_vip_cmdq_vir_addr, 0x0, DRVCMDQ_CMDQBUFFER_SIZE);


    if(!(sg_vip_cmdq_vir_addr = alloc_dmem("CMDMENU",
                                           DRVCMDQ_MLOADBUFFER_SIZE,
                                           &sg_vip_cmdq_bus_addr)))
    {
        CMDQ_ERR("%s: unable to allocate screen memory\n", __FUNCTION__);
        return ;
    }
    CMDQ_LOG("[CMDQ]: MENULOAD: Phy:%x  Vir:%x\n", sg_vip_cmdq_bus_addr, (u32)sg_vip_cmdq_vir_addr);

    pCmdqMmapInfo->u32MloadMmapPhyAddr = Chip_Phys_to_MIU(sg_vip_cmdq_bus_addr);
    pCmdqMmapInfo->u32MloadMmapVirAddr = (u32)sg_vip_cmdq_vir_addr;
    pCmdqMmapInfo->u32MloadMmapSize    =  DRVCMDQ_MLOADBUFFER_SIZE;

    memset(sg_vip_cmdq_vir_addr, 0x0, DRVCMDQ_MLOADBUFFER_SIZE);

}

void MDrvCmdqReleaseBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo)
{
    dma_addr_t  sg_vip_cmdq_bus_addr;

    sg_vip_cmdq_bus_addr = Chip_MIU_to_Phys(pCmdqMmapInfo->u32CmdqMmapPhyAddr);
    free_dmem("DRVCMDQ", pCmdqMmapInfo->u32CmdqMmapSize, (void*)pCmdqMmapInfo->u32CmdqMmapVirAddr, sg_vip_cmdq_bus_addr);

    sg_vip_cmdq_bus_addr = Chip_MIU_to_Phys(pCmdqMmapInfo->u32MloadMmapPhyAddr);
    free_dmem("CMDMENU",pCmdqMmapInfo->u32MloadMmapSize,(void*)pCmdqMmapInfo->u32MloadMmapVirAddr, sg_vip_cmdq_bus_addr);
}
#elif defined(CAM_OS_RTK)
void MDrvCmdqAllocateBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo)
{
    CamOsRet_e eRet;
    void *pVirtPtr = NULL;
    void *pMiuAddr = NULL;
    void *pPhysAddr = NULL;

    eRet = CamOsDirectMemAlloc("CMDMEM", DRVCMDQ_CMDQBUFFER_SIZE, &pVirtPtr, &pPhysAddr,&pMiuAddr);
    if(eRet!=CAM_OS_OK)
    {
        CMDQ_ERR("[CMDQ]can't allocate cmdq memory\n");
        return ;
    }

    pCmdqMmapInfo->u32CmdqMmapPhyAddr = (u32)pPhysAddr;
    pCmdqMmapInfo->u32CmdqMmapVirAddr = (u32)pVirtPtr;
    pCmdqMmapInfo->u32CmdqMmapSize    = DRVCMDQ_CMDQBUFFER_SIZE;

    memset(pVirtPtr, 0x0, DRVCMDQ_CMDQBUFFER_SIZE);

    eRet = CamOsDirectMemAlloc("MLOADMEM", DRVCMDQ_MLOADBUFFER_SIZE, &pVirtPtr, &pPhysAddr, &pMiuAddr);
    if(eRet!=CAM_OS_OK)
    {
        CMDQ_ERR("[CMDQ]can't allocate MLOAD memory\n");
        return ;
    }
    pCmdqMmapInfo->u32MloadMmapPhyAddr = (u32)pPhysAddr;
    pCmdqMmapInfo->u32MloadMmapVirAddr = (u32)pVirtPtr;
    pCmdqMmapInfo->u32MloadMmapSize    =  DRVCMDQ_MLOADBUFFER_SIZE;

    memset(pVirtPtr, 0x0, DRVCMDQ_MLOADBUFFER_SIZE);

}

void MDrvCmdqReleaseBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo)
{
    CamOsDirectMemRelease((void*)pCmdqMmapInfo->u32CmdqMmapVirAddr,pCmdqMmapInfo->u32CmdqMmapSize);
    CamOsDirectMemRelease((void*)pCmdqMmapInfo->u32MloadMmapVirAddr,pCmdqMmapInfo->u32MloadMmapSize);

}

#else

#endif

s32 _MDrvCmdqInsertOneCommand(void* pPriv, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h)
{
    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;
    u8     *pVirBufwrite;

    CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, " cmdq(%d) cmd=0x%02X -> |L| %02x %02x %02x %02x %02x %02x %02x %02x |H|\n", pPrivData->CmdqIpId, ((h & 0xF0) >> 4), a, b, c, d, e, f, g, h);

    if((u32)pPrivData->tCmdqInfo.pBufWrite >= ((u32)pPrivData->tCmdqInfo.pBufEnd))
    {

        CMDQ_DBG(CMDQ_DBG_LVL1_TYPE, " cmdq write >= cmdq end(0x%08x-0x%08x)\n", (u32)pPrivData->tCmdqInfo.pBufWrite
                 , (u32)pPrivData->tCmdqInfo.pBufEnd);
        pPrivData->tCmdqInfo.pBufWrite = (u8*)pPrivData->tCmdqInfo.pBufStart ;
    }

    pVirBufwrite = (u8*)_MDrvCmdqTransPhyBuf2VirBuf(pPrivData, (u32)pPrivData->tCmdqInfo.pBufWrite);
    pVirBufwrite[0] = a;
    pVirBufwrite[1] = b;
    pVirBufwrite[2] = c;
    pVirBufwrite[3] = d;
    pVirBufwrite[4] = e;
    pVirBufwrite[5] = f;
    pVirBufwrite[6] = g;
    pVirBufwrite[7] = h;

    //CMDQ_LOG("vir addr=0x%x , value(0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x)\n"
    //        ,(u32)pVirBufwrite ,(u32)pVirBufwrite[0],(u32)pVirBufwrite[1],pVirBufwrite[2],pVirBufwrite[3]
    //        ,pVirBufwrite[4],pVirBufwrite[5],pVirBufwrite[6],pVirBufwrite[7]);
    pPrivData->tCmdqInfo.pBufWrite = (u8*)((u32)pPrivData->tCmdqInfo.pBufWrite + DVRCMDQ_COMMAND_SIZE);
    pPrivData->tCmdqInfo.nCurCmdCount++;
    return 0;
}

s32 _MDrvCmdqInsertOneWriteCommand(void* pPriv, u32 reg_addr, u16 value,  u16 write_mask)
{
    u32  u1, u2, u3;
    u32  DDR_1, DDR_2, DDR_3, DDR_4, DDR_5, DDR_6, DDR_7, DDR_8;
    u16  nWMask = ~write_mask;
    u16  u16Bank;
    u8   u8addr;

    //_CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_SLEEP);

    u16Bank = (u16)((reg_addr >> 8) & 0xFFFF);
    u8addr = (u8)((reg_addr & 0xFF));
    u1 = ((((u32)u16Bank << 4) * 8) | ((u32)u8addr / 2));
    u2 = value;
    u3 = nWMask;

    DDR_1 = DRVCMDQ_CMDTYPE_WRITE;    //command type
    DDR_2 = u1 >> 16;                 //target RIU
    DDR_3 = (u1 >> 8 & (0xff));       //target RIU
    DDR_4 = (u1) & (0xff);            //target RIU
    DDR_5 = u2 >> 8;                  //write value
    DDR_6 = u2 & (0xff);              //write value
    DDR_7 = u3 >> 8;                  //write mask
    DDR_8 = u3 & (0xff);              //write mask

    //_CDMQ_PROFILE_LOW(_CMDQ_PROFILE_SLEEP);

    return _MDrvCmdqInsertOneCommand(pPriv, DDR_8, DDR_7, DDR_6, DDR_5, DDR_4, DDR_3, DDR_2, DDR_1);
}

#if 0
#if DRVCMDQ_ENABLE_DUMMY_IRQ && !defined(CAM_OS_RTK)

irqreturn_t  _MDrvCmdqHwDummyIsr(int eIntNum, void* dev_id)
{
    CmdqInterfacePriv_t  *pPrivData;

    pPrivData = (CmdqInterfacePriv_t*) dev_id;
    HAL_CMDQ_Clear_HwDummy_Register(pPrivData->nCmdqDrvIpId);
    CMDQ_LOG("HwDummyIsr cmdq(%d)\n",pPrivData->nCmdqDrvIpId);
    return IRQ_HANDLED;
}
#else
void  _MDrvCmdqHwDummyIsr(int eIntNum, void* dev_id)
{
    CmdqInterfacePriv_t  *pPrivData;

    pPrivData = (CmdqInterfacePriv_t*) DrvCmdqOsGetIntPrivData(eIntNum);
    if(pPrivData == NULL)
    {
        CMDQ_ERR("HwDummyIsr cmdq IRQ(%d) Unknow PrivData\n",eIntNum);
        return;
    }
    HAL_CMDQ_Clear_HwDummy_Register(pPrivData->nCmdqDrvIpId);
    CMDQ_LOG("HwDummyIsr cmdq(%d)\n",pPrivData->nCmdqDrvIpId);
}
#endif

#if DRVCMDQ_ENABLE_IRQ && !defined(CAM_OS_RTK)

irqreturn_t  _MDrvCmdqIsr(int eIntNum, void* dev_id)
{
    u32 u32IrqFlag;
    //u32 u32Time;
    CmdqInterfacePriv_t  *pPrivData;

    pPrivData = (CmdqInterfacePriv_t*) dev_id;

    //u32IrqFlag = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    u32IrqFlag = HAL_CMDQ_Read_FinalIrq_Status(pPrivData->nCmdqDrvIpId);
    HAL_CMDQ_Clear_IRQByFlag(pPrivData->nCmdqDrvIpId, 0xFFFF);

    //CMDQ_LOG("pPrivData->irq dev(%d) irq(%d)\n",pPrivData->CmdqIpId,pPrivData->nCmdqIrq);
    if(u32IrqFlag & DRVCMDQ_IRQ_STATE_DONE)
    {
        CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is done irq status(0x%x)\n",pPrivData->CmdqIpId
                ,pPrivData->nCmdqIrq,u32IrqFlag);
        //CamOsTsemSignal(&pPrivData->tIrqWaitSem);
    }
    if(u32IrqFlag & DRVCMDQ_ERROR_STATUS)
    {
        CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is error irq status(0x%x)\n",pPrivData->CmdqIpId
                ,pPrivData->nCmdqIrq,u32IrqFlag);
        MDrvCmdqPrintfCrashCommand((void*)pPrivData);
    }
    return IRQ_HANDLED;
}
#else
void _MDrvCmdqIsr(int eIntNum, void* dev_id)
{
    u32 u32IrqFlag;
    //u32 u32Time;
    CmdqInterfacePriv_t  *pPrivData;

    pPrivData = (CmdqInterfacePriv_t*) DrvCmdqOsGetIntPrivData(eIntNum);
    if(pPrivData == NULL)
    {
        CMDQ_ERR("_MDrvCmdqIsr cmdq IRQ(%d) Unknow PrivData\n",eIntNum);
        return;
    }


    //u32IrqFlag = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    u32IrqFlag = HAL_CMDQ_Read_FinalIrq_Status(pPrivData->nCmdqDrvIpId);
    HAL_CMDQ_Clear_IRQByFlag(pPrivData->nCmdqDrvIpId, 0xFFFF);

    //CMDQ_LOG("pPrivData->irq dev(%d) irq(%d)\n",pPrivData->CmdqIpId,pPrivData->nCmdqIrq);
    if(u32IrqFlag & DRVCMDQ_IRQ_STATE_DONE)
    {
        CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is done irq status(0x%x)\n",pPrivData->CmdqIpId
                ,pPrivData->nCmdqIrq,u32IrqFlag);
        //CamOsTsemSignal(&pPrivData->tIrqWaitSem);
    }
    if(u32IrqFlag & DRVCMDQ_ERROR_STATUS)
    {
        CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is error irq status(0x%x)\n",pPrivData->CmdqIpId
                ,pPrivData->nCmdqIrq,u32IrqFlag);
        MDrvCmdqPrintfCrashCommand((void*)pPrivData);
    }

}

#endif
#endif
DRVCMDQ_RESULT MDrvCMDQSetTimerRatio(DrvCmdqIPSupport_e eIpNum, u32 time, u32 ratio)
{
    HAL_CMDQ_Set_Timer(eIpNum, time);
    HAL_CMDQ_Set_Ratio(eIpNum, ratio);
    return DRVCMDQ_OK;
}

void MDrvCmdqSetBufInfo(CmdqInterfacePriv_t* pPrivData)
{
    u32 nStartAddr, nEndaddr;
    if(pPrivData  == NULL)
    {
        CMDQ_ERR("%s priv data error\n", __func__);
        return ;
    }

    if(!pPrivData->tCmdqBuf.nBufSize)
    {
        CMDQ_ERR("%s priv CMDQ buffer empty\n", __func__);
        return ;
    }

    pPrivData->tCmdqInfo.pBufStart           = (u8*)(pPrivData->tCmdqBuf.BufPhy);
    pPrivData->tCmdqInfo.pBufEnd             = (u8*)((pPrivData->tCmdqBuf.BufPhy + pPrivData->tCmdqBuf.nBufSize) - DVRCMDQ_COMMAND_SIZE_ALIGN);
    pPrivData->tCmdqInfo.pBufWrite           = (u8*)(pPrivData->tCmdqBuf.BufPhy);
    pPrivData->tCmdqInfo.pBufFireWriteHw     = (u8*) 0;
    pPrivData->tCmdqInfo.pBufReadHw          = (u8*) 0;
    pPrivData->tCmdqInfo.nTotalCmdCount      = (pPrivData->tCmdqInfo.pBufEnd - pPrivData->tCmdqInfo.pBufStart) / DVRCMDQ_COMMAND_SIZE;

    if(pPrivData->tMloadBuf.nBufSize)
    {
        pPrivData->tMloadInfo.pBufStart = (u8*)pPrivData->tMloadBuf.BufVir;
        pPrivData->tMloadInfo.pBufEnd   = (u8*)(pPrivData->tMloadBuf.BufVir + pPrivData->tMloadBuf.nBufSize);
        pPrivData->tMloadInfo.pBufWrite = (u8*)pPrivData->tMloadBuf.BufVir;
        pPrivData->tMloadInfo.pBufReadHw  = (u8*)pPrivData->tMloadBuf.BufVir;
    }

    nStartAddr = ((u32)pPrivData->tCmdqInfo.pBufStart) >> DRVCMD_BUFFER_SHIFT;
    nEndaddr   = ((u32)pPrivData->tCmdqInfo.pBufEnd)  >> DRVCMD_BUFFER_SHIFT;
#if !defined(HAL_I2_SIMULATE_NO_HW)
    _MDrvCmdqSetBuffer(pPrivData->nCmdqDrvIpId, nStartAddr, nEndaddr);
#endif
}

CmdqInterfacePriv_t* MDrvGetCmdqPrivateDataByCmdqId(DrvCcmdqId_e eCmdId)
{
    int   i;
    u32                   nDrvIpId;

    nDrvIpId = _MdrvCmdTransToDrvIpId(eCmdId);

    for(i = 0; i < EN_CMDQ_TYPE_MAX; i++)
    {
        if(gDrvCmdPriv[i].bUsed && gDrvCmdPriv[i].nCmdqDrvIpId == nDrvIpId)
        {
            return &gDrvCmdPriv[i];
        }
    }
    return NULL;
}

void MDrvRelesaeCmdqService(CmdqInterfacePriv_t *pPrivData)
{
    _MdrvDeInitCmdqPrivData(pPrivData);
}

CmdqInterfacePriv_t* MDrvCmdqInit(DrvCcmdqId_e eCmdId)
{
    CmdqInterfacePriv_t * pPrivData = NULL;
    u32                   nDrvIpId;
#if !defined(HAL_I2_SIMULATE_NO_HW)
    u32                   nMiuNum;
#endif
#if DRVCMDQ_ENABLE_IRQ
    //char                  aName[12];
#endif
    if(!_gInitPrivData)
    {
        _gInitPrivData = 1;
        _MdrvInitCmdqPrivData();
    }

    nDrvIpId = _MdrvCmdTransToDrvIpId(eCmdId);

    if((pPrivData = _MdrvGetCmdqPrivData(nDrvIpId, eCmdId)) == NULL)
    {
        CMDQ_ERR("%s priv data error(%d-%d)\n", __func__, eCmdId, nDrvIpId);
        return NULL;
    }
#if !defined(HAL_I2_SIMULATE_NO_HW)
    HAL_CMDQ_SetBank(pPrivData->nCmdqDrvIpId, pPrivData->nCmdqRiuBase);
    nMiuNum = _MDrvCmdqGetCmdqMiuNumber((void*)pPrivData);
    HAL_CMDQ_Set_MIU_SELECT(pPrivData->nCmdqDrvIpId, nMiuNum);
    HAL_CMDQ_Enable(pPrivData->nCmdqDrvIpId);
    //HAL_CMDQ_Reset(pPrivData->nCmdqDrvIpId);
    MDrvCmdDbgPause(pPrivData->nCmdqDrvIpId);
    MDrvCmdDbgContinue(pPrivData->nCmdqDrvIpId);

    if(false == HAL_CMDQ_Set_Mode(pPrivData->nCmdqDrvIpId, pPrivData->nCmdMode))
    {
        CMDQ_ERR("\033[35mCMDQ Set Mode FAIL!!\033[m\n");
        return NULL;
    }

    /*set trigger bus as sample enable*/
    HAL_CMDQ_Enable_TriggerBusSample(pPrivData->nCmdqDrvIpId);

    /* the polling_timer is for re-checking polling value, if the time_interval is too small, cmdq will polling RIU frequently, so that RIU will very busy */
    MDrvCMDQSetTimerRatio(pPrivData->nCmdqDrvIpId, _gPollTimeOut, _gPollTimeRatio);        // settime / ratio, total wait time is (wait_time * (ratio + 1)) => ( FF * (F + 1)/ 216MHz ) = sec
    HAL_CMDQ_Reset_Soft_Interrupt(pPrivData->nCmdqDrvIpId);
    // 0 is enable,enable cmdq status and timeout status
    HAL_CMDQ_SetISRMSK(pPrivData->nCmdqDrvIpId, 0xEAFF);/*only write,wait, poll neq*/
#if DRVCMDQ_ENABLE_IRQ
    DrvCmdqIrqStatusInit(pPrivData->nCmdqDrvIpId,pPrivData->nCmdqIrq,(void*)pPrivData);
#else

#endif
#if DRVCMDQ_ENABLE_DUMMY_IRQ
    DrvCmdqIrqHwDummyInit(pPrivData->nCmdqDrvIpId,gDrvCmdqHWDummyIrqNumber[pPrivData->nCmdqDrvIpId],(void*)pPrivData);
#endif
#endif

    return pPrivData;
}
u32 MDrvCmdqGetCurrentCmdqNumber(void* pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }
    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    return pPrivData->tCmdqInfo.nCurCmdCount;
}
u32 MDrvCmdqGetAvailCmdqNumber(void* pPriv)
{
#if defined(USE_DIRECTMODE)
    u32 s, e, w;

    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    s = (u32)pPrivData->tCmdqInfo.pBufStart;
    e = (u32)pPrivData->tCmdqInfo.pBufEnd;
    w = (u32)pPrivData->tCmdqInfo.pBufWrite;

    return ((e - w) / DVRCMDQ_COMMAND_SIZE);
#else
    u32 s, e, r, w;

    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    s = (u32)pPrivData->tCmdqInfo.pBufStart;
    e = (u32)pPrivData->tCmdqInfo.pBufEnd;
    w = (u32)pPrivData->tCmdqInfo.pBufWrite;
    //r = HAL_CMDQ_Read_Pointer(pPrivData->nCmdqDrvIpId);
    //r = _MDrvCmdqTransPhyBuf2VirBuf(pPrivData, r);
    if(pPrivData->bFirstFireDone)
    {
        r = HAL_CMDQ_Read_Pointer(pPrivData->nCmdqDrvIpId);
        pPrivData->tCmdqInfo.pBufReadHw = (u8*)r;
        r = r << DRVCMD_BUFFER_SHIFT;
        //r = _MDrvCmdqTransPhyBuf2VirBuf(pPrivData, r);
        CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "%s ReadHw=0x%08x\n", __func__, r);

    }
    else
        r = s ;


    if(r > e || r < s)
    {
        CMDQ_ERR("rdptr out of buffer range.(0x%x-0x%x-0x%x)\n", s, e, r);
        //return _ctx[hnd].TOTAL_BUFFER_SIZE;
        r = s;
        return 0;
    }

    if(pPrivData->nCmdMode == RING_BUFFER_MODE)
    {
        //@NOTE: actually, rdptr will equal to wrptr.
        if(w == r)
        {
            //@NOTE: because if we fill buffer full, that rd=wr will not trigger.

            return (pPrivData->tCmdqInfo.nTotalCmdCount) ;
        }
        else if(r > w)
        {
            //|S|......W_________R......|E|
            return ((r - w) / DVRCMDQ_COMMAND_SIZE);
        }
        else if(w > r)
        {
            //|S|______R.........W______|E|
            return ((((r - s) + (e - w)) / DVRCMDQ_COMMAND_SIZE) + 1);
        }
        else
        {
            CMDQ_ERR("Internal cmdq error(0x%08x-0x%08x)!\n", w, r);
            //panic("Internal cmdq error(0x%x-0x%x)!\n",w,r);
        }
        return 0;
    }
    else
    {
        CMDQ_ERR("We only support RING_BUFFER_MODE ONLY!\n");
        //panic("We only support RING_BUFFER_MODE ONLY!\n");
    }
    return 0;
#endif
}

u32 MDrvCmdqCheckBufferAvail(void* pPriv, u32 nCmdqNum)
{
    u32        nBufWaitCnt = 0;
    u16        nRetVal;
    CmdqInterfacePriv_t * pPrivData = NULL;

    _CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_DRV_CHECK);

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_DRV_CHECK);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    if(pPrivData->bForceRIU)
    {
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_DRV_CHECK);
        return nCmdqNum;
    }

    if((nRetVal = _MDrvCmdqCheckBufferAvail(pPriv, nCmdqNum)) == 0)
    {
        do
        {
            //CMDQ_ERR("%s Wait cmdq buffer TimeOut(%d)\n", __func__, nRetVal);
            _CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_SLEEP);
            DrvCmdqOsDelayTask(SLEEP_POLL_TIME);
            _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_SLEEP);

            nBufWaitCnt++;
            if(nBufWaitCnt > WAIT_SLEEP_TIME_CNT)
            {
                nRetVal = 0;
                CMDQ_ERR("%s Wait cmdq buffer TimeOut(%d)\n", __func__, nCmdqNum);

                break;
            }
        }
        while((nRetVal = _MDrvCmdqCheckBufferAvail(pPriv, nCmdqNum)) == 0);

    }

    _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_DRV_CHECK);

    return nRetVal;
}

s32 MDrvCmdqWriteCommandMask(void* pPriv, u32 reg_addr, u16 value,  u16 write_mask)
{
    u32  u1, u2, u3;
    u16  nCmdcnt;
    u32  DDR_1, DDR_2, DDR_3, DDR_4, DDR_5, DDR_6, DDR_7, DDR_8;
    u16  nWMask = 0;
    u16  u16Bank;
    u8   u8addr;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    _CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_WriteCMD);

    if(pPrivData->bForceRIU)
    {
        W2BYTEMSK(pPrivData->nCmdqRiuBase, reg_addr, value, write_mask);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

        return 0;
    }

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    nWMask = ~write_mask;

    if((nCmdcnt = MDrvCmdqCheckBufferAvail(pPriv, 1)) <= 0)
    {
        CMDQ_ERR("%s cmdq buffer isn't available(%d)\n", __func__, nCmdcnt);
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

        return -1;
    }
    u16Bank = (u16)((reg_addr >> 8) & 0xFFFF);
    u8addr = (u8)((reg_addr & 0xFF));
    u1 = ((((u32)u16Bank << 4) * 8) | ((u32)u8addr / 2));

    //u1 = reg_addr;
    u2 = value;
    u3 = nWMask;
    DDR_1 = DRVCMDQ_CMDTYPE_WRITE;    //command type
    DDR_2 = u1 >> 16;                 //target RIU
    DDR_3 = (u1 >> 8 & (0xff));       //target RIU
    DDR_4 = (u1) & (0xff);            //target RIU
    DDR_5 = u2 >> 8;                  //write value
    DDR_6 = u2 & (0xff);              //write value
    DDR_7 = u3 >> 8;                  //write mask
    DDR_8 = u3 & (0xff);              //write mask

    _MDrvCmdqInsertOneCommand(pPriv, DDR_8, DDR_7, DDR_6, DDR_5, DDR_4, DDR_3, DDR_2, DDR_1);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

    return 0;
}
u32 MDrvCmdqWriteCommandMaskMulti(void* pPriv, void *u32MultiCmdBufMask, u16 u16Size)
{
    MHAL_CMDQ_MultiCmdBufMask_t *_u32MultiCmdBuf = (MHAL_CMDQ_MultiCmdBufMask_t *)u32MultiCmdBufMask;
    u32  idx;
    u16  nCmdcnt,nCmdcntRemain,nCmdcntThis;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    _CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_WriteCMD);

    if(pPrivData->bForceRIU)
    {
        for(idx=0;idx<u16Size;idx++)
        {
            W2BYTEMSK(pPrivData->nCmdqRiuBase, _u32MultiCmdBuf[idx].u32RegAddr, _u32MultiCmdBuf[idx].u16RegValue, _u32MultiCmdBuf[idx].u16Mask);
        }
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

        return 0;
    }

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    nCmdcntRemain = u16Size;

    do {
        /* TODO: !!! IMPORTANT !!!
         * In order to prevent screw up the points, we have to ensure that:
         * 1) Reserve at least 2 cmds buffer size for kick. (Since it will auto align)
         * 2) NEVER let SW move the pointers into r == w...
         */
        nCmdcnt = _MDrvCmdqCheckBufferAvail(pPriv, 1);
        nCmdcnt -= (nCmdcnt >= (DVRCMDQ_COMMAND_ALIGN*2)) ? (DVRCMDQ_COMMAND_ALIGN*2) : nCmdcnt;
        if(nCmdcnt >= nCmdcntRemain)
        {
            nCmdcntThis = nCmdcntRemain;
        } else if(nCmdcnt != 0) {
            nCmdcntThis = nCmdcnt;
        } else {
            CMDQ_ERR("%s cmdq buffer isn't available(%d)\n", __func__, nCmdcnt);
            CamOsMutexUnlock(&pPrivData->tCmdqMutex);
            _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);
            return u16Size - nCmdcntRemain;
        }

        for(idx=0;idx<nCmdcntThis;idx++)
        {
            _MDrvCmdqInsertOneWriteCommand(pPriv, _u32MultiCmdBuf[(u16Size-nCmdcntRemain)+idx].u32RegAddr, (u16)_u32MultiCmdBuf[(u16Size-nCmdcntRemain)+idx].u16RegValue, (u16)_u32MultiCmdBuf[(u16Size-nCmdcntRemain)+idx].u16Mask);
        }

        nCmdcntRemain -= nCmdcntThis;

    } while(nCmdcntRemain > 0);

    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

    return (s32)u16Size;
}
s32 MDrvCmdqWriteCommand(void* pPriv, u32 reg_addr, u16 value)
{
    u32  u1, u2;
    u16  nCmdcnt;
    u32  DDR_1, DDR_2, DDR_3, DDR_4, DDR_5, DDR_6, DDR_7, DDR_8;
    u16  u16Bank;
    u8   u8addr;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    _CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_WriteCMD);

    if(pPrivData->bForceRIU)
    {
        W2BYTE(pPrivData->nCmdqRiuBase, reg_addr, value);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

        return 0;
    }

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    if((nCmdcnt = MDrvCmdqCheckBufferAvail(pPriv, 1)) <= 0)
    {
        CMDQ_ERR("%s cmdq buffer isn't available(%d)\n", __func__, nCmdcnt);
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

        return -1;
    }



    u16Bank = (u16)((reg_addr >> 8) & 0xFFFF);
    u8addr = (u8)((reg_addr & 0xFF));

    u1 = ((((u32)u16Bank << 4) * 8) | ((u32)u8addr / 2));
    //u1 = reg_addr ;//_MDrv_CMDQ_GetCmdReg(reg_addr);
    u2 = value;

    DDR_1 = DRVCMDQ_CMDTYPE_WRITE ;  //command type
    DDR_2 = u1 >> 16;                 //target RIU
    DDR_3 = (u1 >> 8 & (0xff));       //target RIU
    DDR_4 = (u1) & (0xff);            //target RIU
    DDR_5 = u2 >> 8;                  //write value
    DDR_6 = u2 & (0xff);              //write value
    DDR_7 = 0x00;                     //write mask
    DDR_8 = 0x00;                     //write mask

    //_MDrvCmdqInsertOneCommand(pPriv, 0xFF,0xFF,0,0,0,0,0,0);
    _MDrvCmdqInsertOneCommand(pPriv, DDR_8, DDR_7, DDR_6, DDR_5, DDR_4, DDR_3, DDR_2, DDR_1);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

    return 0;
}
u32 MDrvCmdqWriteCommandMulti(void* pPriv, void *u32MultiCmdBuf, u16 u16Size)
{
    MHAL_CMDQ_MultiCmdBuf_t *_u32MultiCmdBuf = (MHAL_CMDQ_MultiCmdBuf_t *)u32MultiCmdBuf;
    u32  idx;
    u16  nCmdcnt,nCmdcntRemain,nCmdcntThis;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    _CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_WriteCMD);

    if(pPrivData->bForceRIU)
    {
        for(idx=0;idx<u16Size;idx++)
        {
            W2BYTE(pPrivData->nCmdqRiuBase, _u32MultiCmdBuf[idx].u32RegAddr, _u32MultiCmdBuf[idx].u32RegValue);
        }
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

        return 0;
    }

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    nCmdcntRemain = u16Size;

    do {
        /* TODO: !!! IMPORTANT !!!
         * In order to prevent screw up the points, we have to ensure that:
         * 1) Reserve at least 2 cmds buffer size for kick. (Since it will auto align)
         * 2) NEVER let SW move the pointers into r == w...
         */
        nCmdcnt = _MDrvCmdqCheckBufferAvail(pPriv, 1);
        nCmdcnt -= (nCmdcnt >= (DVRCMDQ_COMMAND_ALIGN*2)) ? (DVRCMDQ_COMMAND_ALIGN*2) : nCmdcnt;
        if(nCmdcnt >= nCmdcntRemain)
        {
            nCmdcntThis = nCmdcntRemain;
        } else if(nCmdcnt != 0) {
            nCmdcntThis = nCmdcnt;
        } else {
            CMDQ_ERR("%s cmdq buffer isn't available(%d)\n", __func__, nCmdcnt);
            CamOsMutexUnlock(&pPrivData->tCmdqMutex);
            _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);
            return u16Size - nCmdcntRemain;
        }

        for(idx=0;idx<nCmdcntThis;idx++)
        {
            _MDrvCmdqInsertOneWriteCommand(pPriv, _u32MultiCmdBuf[(u16Size-nCmdcntRemain)+idx].u32RegAddr, (u16)_u32MultiCmdBuf[(u16Size-nCmdcntRemain)+idx].u32RegValue, CDMQ_MASK_DEFAULT);
        }

        nCmdcntRemain -= nCmdcntThis;

    } while(nCmdcntRemain > 0);

    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_WriteCMD);

    return (s32)u16Size;
}
s32 MDrvCmdqClearTriggerEvent(void* pPriv , u16 u16Event)
{
    u32  u1, u2, u3, nClrTriAdr;
    u16  nCmdcnt, i;
    u32  DDR_1, DDR_2, DDR_3, DDR_4, DDR_5, DDR_6, DDR_7, DDR_8;
    u16  u16Bank;
    u8   u8addr;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    if(pPrivData->bForceRIU)
    {
        return 0;
    }

    CamOsMutexLock(&pPrivData->tCmdqMutex);

    if((nCmdcnt = MDrvCmdqCheckBufferAvail(pPriv, DRVCMD_TRIGGE_BUS_CLR_CNT)) <= 0)
    {
        CMDQ_ERR("%s cmdq buffer isn't available(%d-%d)\n", __func__, nCmdcnt,DRVCMD_TRIGGE_BUS_CLR_CNT);
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        return -1;
    }


    nClrTriAdr = HAL_CMDQ_Get_TriggerClr_Register_RiuAddr(pPrivData->nCmdqDrvIpId);

    u16Bank = (u16)((nClrTriAdr >> 8) & 0xFFFF);
    u8addr = (u8)((nClrTriAdr & 0xFF));

    u1 = ((((u32)u16Bank << 4) * 8) | ((u32)u8addr / 2));
    u2 = u16Event;
    u3 = 0x0000;

    DDR_1 = DRVCMDQ_CMDTYPE_WRITE;    //command type
    DDR_2 = u1 >> 16;                 //target RIU
    DDR_3 = (u1 >> 8 & (0xff));       //target RIU
    DDR_4 = (u1) & (0xff);            //target RIU
    DDR_5 = u2 >> 8;                  //write value
    DDR_6 = u2 & (0xff);              //write value
    DDR_7 = u3 >> 8;                  //write mask
    DDR_8 = u3 & (0xff);              //write mask

    //CMDQ_LOG("[CMDQ] cmdq(%d) cmd=0x%02X -> |L| %02x %02x %02x %02x %02x %02x %02x %02x |H|\n", pPrivData->CmdqIpId, ((DDR_1 & 0xF0) >> 4),
    //         DDR_8,DDR_7,DDR_6,DDR_5,DDR_4,DDR_3,DDR_2, DDR_1);

    for(i = 0 ; i < DRVCMD_TRIGGE_BUS_CLR_CNT; i++)
    {
        _MDrvCmdqInsertOneCommand(pPriv, DDR_8, DDR_7, DDR_6, DDR_5, DDR_4, DDR_3, DDR_2, DDR_1);
    }

    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    return 0;
}

void MDrvCmdqPrintfCrashCommand(void* pPriv)
{
    u32 Command_15_0_bit, Command_31_16_bit, Command_55_32_bit, Command_63_56_bit;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;
    Command_15_0_bit = HAL_CMDQ_Error_Command(pPrivData->nCmdqDrvIpId, CMDQ_CRASH_15_0_BIT);
    Command_31_16_bit = HAL_CMDQ_Error_Command(pPrivData->nCmdqDrvIpId, CMDQ_CRASH_31_16_BIT);
    Command_55_32_bit = HAL_CMDQ_Error_Command(pPrivData->nCmdqDrvIpId, CMDQ_CRASH_55_32_BIT);
    Command_63_56_bit = HAL_CMDQ_Error_Command(pPrivData->nCmdqDrvIpId, CMDQ_CRASH_63_56_BIT);

    CMDQ_LOG("[CMDQ]\033[35mCommand _bits: %04x, %04x, %04x, %04x \033[m\n"
           , Command_63_56_bit, Command_55_32_bit, Command_31_16_bit, Command_15_0_bit);
}
s32 MDrvCmdqPollEqCommandMask(void* pPriv, u32 reg_addr, u16 value, u16 write_mask, bool bPollEq)
{
    u32  u1, u2, u3;
    u32  DDR_1, DDR_2, DDR_3, DDR_4, DDR_5, DDR_6, DDR_7, DDR_8;
    u16  nWMask = 0, nCmdcnt;
    u16  u16Bank;
    u8   u8addr;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        return 0;
    }

    CamOsMutexLock(&pPrivData->tCmdqMutex);

    if((nCmdcnt = MDrvCmdqCheckBufferAvail(pPriv, 1)) <= 0)
    {
        CMDQ_ERR("%s cmdq buffer isn't available(%d)\n", __func__, nCmdcnt);
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        return -1;
    }
    nWMask = ~write_mask;

    u16Bank = (u16)((reg_addr >> 8) & 0xFFFF);
    u8addr = (u8)((reg_addr & 0xFF));

    u1 = ((((u32)u16Bank << 4) * 8) | ((u32)u8addr / 2));
    //u1 = reg_addr;
    u2 = value;
    u3 = nWMask;

    if(bPollEq)
        DDR_1 = DRVCMDQ_CMDTYPE_POLLEQ;    //command type
    else
        DDR_1 = DRVCMDQ_CMDTYPE_POLLNEQ;    //command type

    DDR_2 = u1 >> 16;                 //target RIU
    DDR_3 = (u1 >> 8 & (0xff));       //target RIU
    DDR_4 = (u1) & (0xff);            //target RIU
    DDR_5 = u2 >> 8;                  //write value
    DDR_6 = u2 & (0xff);              //write value
    DDR_7 = u3 >> 8;                  //write mask
    DDR_8 = u3 & (0xff);              //write mask

    _MDrvCmdqInsertOneCommand(pPriv, DDR_8, DDR_7, DDR_6, DDR_5, DDR_4, DDR_3, DDR_2, DDR_1);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    return 0;

}

s32 MDrvCmdqPollEqCommandMaskByTime(void* pPriv, u32 reg_addr, u16 value, u16 write_mask, bool bPollEq,u32 nTimeNs)
{
    u32  u1, u2, u3;
    u32  DDR_1, DDR_2, DDR_3, DDR_4, DDR_5, DDR_6, DDR_7, DDR_8;
    u16  nWMask = 0, nCmdcnt;
    u16  u16Bank;
    u8   u8addr;
	u32  nTimeWait;
    u32  nTimeRaito;
    u32  nRegValue;
    u32  nSetPollTimeValue = 0;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        return 0;
    }

	/*calcu timout value (MIU is 333mhz)*/
    /*1.set poll jump enable and time value(MIU is 333mhz)*/
    _MdrvCmdCalcuPollTimeVlaue(nTimeNs,&nTimeWait,&nTimeRaito);
    nSetPollTimeValue = (nTimeWait & 0xffff);
    nRegValue  = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nCmdqCtlReg;
    nRegValue |= (0x4a*2);
    MDrvCmdqWriteCommandMask(pPriv,nRegValue,nSetPollTimeValue,0xffff);

    nSetPollTimeValue = (nTimeRaito & 0xff);
    nRegValue  = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nCmdqCtlReg;
    nRegValue |= (0x4b*2);
    MDrvCmdqWriteCommandMask(pPriv,nRegValue,nSetPollTimeValue,0xff);
	//CMDQ_ERR("%s polltime 0x%x,0x%x \n", __func__, nTimeWait,nTimeRaito);
    CamOsMutexLock(&pPrivData->tCmdqMutex);

    if((nCmdcnt = MDrvCmdqCheckBufferAvail(pPriv, 1)) <= 0)
    {
        CMDQ_ERR("%s cmdq buffer isn't available(%d)\n", __func__, nCmdcnt);
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        return -1;
    }
    nWMask = ~write_mask;

    u16Bank = (u16)((reg_addr >> 8) & 0xFFFF);
    u8addr = (u8)((reg_addr & 0xFF));

    u1 = ((((u32)u16Bank << 4) * 8) | ((u32)u8addr / 2));
    //u1 = reg_addr;
    u2 = value;
    u3 = nWMask;

    if(bPollEq)
        DDR_1 = DRVCMDQ_CMDTYPE_POLLEQ;    //command type
    else
        DDR_1 = DRVCMDQ_CMDTYPE_POLLNEQ;    //command type

    DDR_2 = u1 >> 16;                 //target RIU
    DDR_3 = (u1 >> 8 & (0xff));       //target RIU
    DDR_4 = (u1) & (0xff);            //target RIU
    DDR_5 = u2 >> 8;                  //write value
    DDR_6 = u2 & (0xff);              //write value
    DDR_7 = u3 >> 8;                  //write mask
    DDR_8 = u3 & (0xff);              //write mask

    _MDrvCmdqInsertOneCommand(pPriv, DDR_8, DDR_7, DDR_6, DDR_5, DDR_4, DDR_3, DDR_2, DDR_1);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);


	/*calcu timout value (MIU is 333mhz)*/
	/*1.set poll jump enable and time value(MIU is 333mhz)*/
	nSetPollTimeValue = _gPollTimeOut;
	nRegValue  = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nCmdqCtlReg;
	nRegValue |= (0x4a*2);
	MDrvCmdqWriteCommandMask(pPriv,nRegValue,nSetPollTimeValue,0xffff);

	nSetPollTimeValue = _gPollTimeRatio;
	nRegValue  = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nCmdqCtlReg;
	nRegValue |= (0x4b*2);
	MDrvCmdqWriteCommandMask(pPriv,nRegValue,nSetPollTimeValue,0xff);
    return 0;

}

s32 MDrvCmdqWaitCommand(void* pPriv,  DRV_CMDQ_EVENT_ID nTrigEvnt)
{
    u32  DDR_1, DDR_2, DDR_3, DDR_4, DDR_5, DDR_6, DDR_7, DDR_8;
    u16  nMask = 0;
    u32  nBitMask = 0, nCmdcnt,i,nBit;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    if(pPrivData->bForceRIU)
    {
        CMDQ_LOG("wait cmd no support RIU mode\n");
        return 0;
    }

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    if((nCmdcnt = MDrvCmdqCheckBufferAvail(pPriv, 1)) <= 0)
    {
        CMDQ_ERR("%s cmdq buffer isn't available(%d)\n", __func__, nCmdcnt);
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        return -1;
    }

    for(i = 0 ; i < DRVCMDQEVE_MAX ; i++)
    {
        if(nTrigEvnt & (0x1 << i))
        {
            if((nBit = _MdrvGetCmdqTrigEventBit(pPriv, (0x1 << i))) < 0)
            {
                CMDQ_ERR("%s cmdq event isn't support (0x%x)\n", __func__,nTrigEvnt);
                CamOsMutexUnlock(&pPrivData->tCmdqMutex);
                return -1;
            }
            //CMDQ_LOG("[CMDQ]Get event(0x%x)bit(0x%x)\n",nTrigEvnt,nBit);
            nBitMask |= (0x1 << nBit);
        }
    }


    nMask = (~nBitMask) & 0xffff;

    DDR_1 = DRVCMDQ_CMDTYPE_WAIT;    //command type
    DDR_2 = 0;                       //target RIU
    DDR_3 = 0;                       //target RIU
    DDR_4 = 0;                       //target RIU
    DDR_5 = 0;                       //write value
    DDR_6 = 0;                       //write value
    DDR_7 = nMask >> 8;              //write mask
    DDR_8 = nMask & (0xff);          //write mask

    _MDrvCmdqInsertOneCommand(pPriv, DDR_8, DDR_7, DDR_6, DDR_5, DDR_4, DDR_3, DDR_2, DDR_1);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);

    /*if wait done , clean this trigger event*/
    //CMDQ_LOG("[CMDQ]Get event(0x%x)bitmask(0x%x)\n",nTrigEvnt,nBitMask);
    MDrvCmdqClearTriggerEvent(pPriv,nBitMask);
#if 0
    if(pPrivData->bForceRIU)
    {
        CamOsMutexLock(&pPrivData->tCmdqMutex);

        //CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        MDrvCmdqWriteDummyCmdForRiu(pPriv, (pPrivData->nCmdqDrvIpId + 1));
        if(MDrvCmdqKickOfftCommandByRiu(pPriv) < 0)
        {
            if((HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId)&DRVCMDQ_ERROR_STATUS))
                CMDQ_ERR("wait command irq status err(0x%x)\n", HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId));
            else
                CMDQ_ERR("wait command no resource(0x%x)\n", HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId));
        }
        queue_work(pPrivData->pQueuwork, &pPrivData->tWorkTask);
        eRet = CamOsTsemTimedDown(&pPrivData->tWaitQue,  WAIT_SLEEP_TIMEOUT);
        if(eRet == CAM_OS_TIMEOUT)
            CMDQ_ERR("wait_event_timeout TIMEOUT occured\n");
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    }
#endif
    return 0;

}

s32 MDrvCmdqDummyCommand(void* pPriv)
{
    u32  DDR_1, DDR_2, DDR_3, DDR_4, DDR_5, DDR_6, DDR_7, DDR_8;
    u16  nCmdcnt;
    //int  nBitMask;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    if((nCmdcnt = MDrvCmdqCheckBufferAvail(pPriv, 1)) <= 0)
    {
        CMDQ_ERR("%s cmdq buffer isn't available(%d)\n", __func__, nCmdcnt);
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        return -1;
    }

    DDR_1 = DRVCMDQ_CMDTYPE_NULL;    //command type
    DDR_2 = 0;                       //target RIU
    DDR_3 = 0;                       //target RIU
    DDR_4 = 0;                       //target RIU
    DDR_5 = 0;                       //write value
    DDR_6 = 0;                       //write value
    DDR_7 = 0xff;                    //write mask
    DDR_8 = 0xff;                    //write mask

    _MDrvCmdqInsertOneCommand(pPriv, DDR_8, DDR_7, DDR_6, DDR_5, DDR_4, DDR_3, DDR_2, DDR_1);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    return 0;

}

s32 MDrvCmdqKickOfftCommandByRiu(void* pPriv)
{
    u32 nTmpBufWrite;
    u32 nTmpBufNum;
    u16 nIrqStatus = 0;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;
#if 0
    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        return 0;
    }
#endif
    //CamOsMutexLock(&pPrivData->tCmdqMutex);

    if(pPrivData->tCmdqInfo.pBufWrite > pPrivData->tCmdqInfo.pBufEnd)
    {
        CMDQ_DBG(CMDQ_DBG_LVL1_TYPE, "kickoff cmd%d buffer write(0x%08x) end(0x%08x)\n", pPrivData->nCmdqDrvIpId
                 , (u32) pPrivData->tCmdqInfo.pBufWrite, (u32)pPrivData->tCmdqInfo.pBufEnd);
        nTmpBufWrite = (u32)pPrivData->tCmdqInfo.pBufEnd;
    }
    else
        nTmpBufWrite = (u32)pPrivData->tCmdqInfo.pBufWrite;

    if(nTmpBufWrite % DVRCMDQ_COMMAND_SIZE_ALIGN)
    {
        if((nTmpBufNum = MDrvCmdqGetAvailCmdqNumber(pPriv)) == 0)
        {
            CMDQ_ERR("%s no more cmdq(%d)\n", __func__, nTmpBufNum);
            //CamOsMutexUnlock(&pPrivData->tCmdqMutex);
            return -1;
        }

        _MDrvCmdqInsertOneCommand(pPriv, 0xff, 0xff, 0, 0, 0, 0, 0, 0);
        nTmpBufWrite = (u32)pPrivData->tCmdqInfo.pBufWrite;
    }

    nIrqStatus = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    if((nIrqStatus & DRVCMDQ_ERROR_STATUS))
    {
        CMDQ_ERR("%s cmdq(%d) error (0x%08x)\n", __func__, pPrivData->nCmdqDrvIpId, nIrqStatus);
        //CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        return -1;
    }
    pPrivData->tCmdqInfo.pBufLastFireWriteHw = pPrivData->tCmdqInfo.pBufFireWriteHw;
    pPrivData->tCmdqInfo.pBufFireWriteHw = (u8*)(((u32)nTmpBufWrite) >> DRVCMD_BUFFER_SHIFT);

    nTmpBufNum = pPrivData->tCmdqInfo.nCurCmdCount;
    pPrivData->tCmdqInfo.nCurCmdCount = 0;
    pPrivData->bFirstFireDone = 1;

    _MdrvCmdqDcacheFlush(pPrivData->tCmdqBuf.BufVir, pPrivData->tCmdqBuf.nBufSize);

    HAL_CMDQ_Write_Pointer(pPrivData->nCmdqDrvIpId, (u32)pPrivData->tCmdqInfo.pBufFireWriteHw);
    HAL_CMDQ_Start(pPrivData->nCmdqDrvIpId);

    //CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    return nTmpBufNum;
}

s32 MDrvCmdqKickOfftCommandByTag(void* pPriv,u16 nTagValue)
{
    u32 nTmpBufWrite;
    u32 nTmpBufNum;
    u16 nIrqStatus = 0;

    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        return 0;
    }

    if(_MdrvCmdqCheckIsAbortTag(pPriv,nTagValue))
    {
        MDrvAbortCmdqCommand(pPriv);
        return 0;
    }

    CamOsMutexLock(&pPrivData->tCmdqMutex);

    if(pPrivData->tCmdqInfo.pBufWrite > pPrivData->tCmdqInfo.pBufEnd)
    {
        CMDQ_DBG(CMDQ_DBG_LVL1_TYPE, "kickoff cmd%d buffer write(0x%08x) end(0x%08x)\n", pPrivData->nCmdqDrvIpId
                 , (u32) pPrivData->tCmdqInfo.pBufWrite, (u32)pPrivData->tCmdqInfo.pBufEnd);
        nTmpBufWrite = (u32)pPrivData->tCmdqInfo.pBufEnd;
    }
    else
        nTmpBufWrite = (u32)pPrivData->tCmdqInfo.pBufWrite;

    if(nTmpBufWrite % DVRCMDQ_COMMAND_SIZE_ALIGN)
    {
        if((nTmpBufNum = MDrvCmdqGetAvailCmdqNumber(pPriv)) == 0)
        {
            CMDQ_ERR("%s no more cmdq(%d)\n", __func__, nTmpBufNum);
            CamOsMutexUnlock(&pPrivData->tCmdqMutex);
            return -1;
        }

        _MDrvCmdqInsertOneCommand(pPriv, 0xff, 0xff, 0, 0, 0, 0, 0, 0);
        nTmpBufWrite = (u32)pPrivData->tCmdqInfo.pBufWrite;
    }

    nIrqStatus = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    if((nIrqStatus & DRVCMDQ_ERROR_STATUS))
    {
        CMDQ_ERR("%s cmdq(%d) error (0x%08x)\n", __func__, pPrivData->nCmdqDrvIpId, nIrqStatus);
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        return -1;
    }
    pPrivData->tCmdqInfo.pBufLastFireWriteHw = pPrivData->tCmdqInfo.pBufFireWriteHw;
    pPrivData->tCmdqInfo.pBufFireWriteHw = (u8*)(((u32)nTmpBufWrite) >> DRVCMD_BUFFER_SHIFT);

    nTmpBufNum = pPrivData->tCmdqInfo.nCurCmdCount;
    pPrivData->tCmdqInfo.nCurCmdCount = 0;
    pPrivData->bFirstFireDone = 1;

    _MdrvCmdqDcacheFlush(pPrivData->tCmdqBuf.BufVir, pPrivData->tCmdqBuf.nBufSize);

    HAL_CMDQ_Write_Pointer(pPrivData->nCmdqDrvIpId, (u32)pPrivData->tCmdqInfo.pBufFireWriteHw);
    HAL_CMDQ_Start(pPrivData->nCmdqDrvIpId);

    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    return nTmpBufNum;

}

s32 MDrvCmdqKickOfftCommand(void* pPriv)
{
    u32 nTmpBufWrite;
    u32 nTmpBufNum;
    u16 nIrqStatus = 0;
#if defined(USE_DIRECTMODE)
    u32 nCmdCtlReg;
#endif
    CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)pPriv;

    _CDMQ_PROFILE_HIGH(_CMDQ_PROFILE_KICK);

    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_KICK);

        return 0;
    }

#if defined(USE_DIRECTMODE)
    nCmdCtlReg = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nCmdqCtlReg | (0x03*2);
    MDrvCmdqWriteCommandMask(pPriv,nCmdCtlReg,0x2,0xffff);
#endif

    CamOsMutexLock(&pPrivData->tCmdqMutex);

    if(pPrivData->tCmdqInfo.pBufWrite > pPrivData->tCmdqInfo.pBufEnd)
    {
        CMDQ_DBG(CMDQ_DBG_LVL1_TYPE, "kickoff cmd%d buffer write(0x%08x) end(0x%08x)\n", pPrivData->nCmdqDrvIpId
                 , (u32) pPrivData->tCmdqInfo.pBufWrite, (u32)pPrivData->tCmdqInfo.pBufEnd);
        nTmpBufWrite = (u32)pPrivData->tCmdqInfo.pBufEnd;
    }
    else
        nTmpBufWrite = (u32)pPrivData->tCmdqInfo.pBufWrite;

    if(nTmpBufWrite % DVRCMDQ_COMMAND_SIZE_ALIGN)
    {
        if((nTmpBufNum = MDrvCmdqGetAvailCmdqNumber(pPriv)) == 0)
        {
            CMDQ_ERR("%s no more cmdq(%d)\n", __func__, nTmpBufNum);
            CamOsMutexUnlock(&pPrivData->tCmdqMutex);
            _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_KICK);

            return -1;
        }

        _MDrvCmdqInsertOneCommand(pPriv, 0xff, 0xff, 0, 0, 0, 0, 0, 0);
        nTmpBufWrite = (u32)pPrivData->tCmdqInfo.pBufWrite;
    }

    nIrqStatus = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    if((nIrqStatus & DRVCMDQ_ERROR_STATUS))
    {
        CMDQ_ERR("%s cmdq(%d) error (0x%08x)\n", __func__, pPrivData->nCmdqDrvIpId, nIrqStatus);
        CamOsMutexUnlock(&pPrivData->tCmdqMutex);
        _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_KICK);

        return -1;
    }
    pPrivData->tCmdqInfo.pBufLastFireWriteHw = pPrivData->tCmdqInfo.pBufFireWriteHw;
    pPrivData->tCmdqInfo.pBufFireWriteHw = (u8*)(((u32)nTmpBufWrite) >> DRVCMD_BUFFER_SHIFT);

    nTmpBufNum = pPrivData->tCmdqInfo.nCurCmdCount;
    pPrivData->tCmdqInfo.nCurCmdCount = 0;
    pPrivData->bFirstFireDone = 1;

    _MdrvCmdqDcacheFlush(pPrivData->tCmdqBuf.BufVir, pPrivData->tCmdqBuf.nBufSize);
#if defined(USE_DIRECTMODE)
    HAL_CMDQ_Set_End_Pointer(pPrivData->nCmdqDrvIpId, (u32)pPrivData->tCmdqInfo.pBufFireWriteHw);
    HAL_CMDQ_Start(pPrivData->nCmdqDrvIpId);
#else

    HAL_CMDQ_Write_Pointer(pPrivData->nCmdqDrvIpId, (u32)pPrivData->tCmdqInfo.pBufFireWriteHw);
    HAL_CMDQ_Start(pPrivData->nCmdqDrvIpId);
#endif
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    _CDMQ_PROFILE_LOW(_CMDQ_PROFILE_KICK);

    return nTmpBufNum;
}

s32 MDrvCmdqCmdDelay(void* pPriv,u32 nTimens)
{
    CmdqInterfacePriv_t * pPrivData = NULL;
    u32  nTimeBase;
    u32  nTimeCount;
    u32  nRegValue;
    u32  nSetJumpTimeValue = 0;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return -1;
    }

    pPrivData = (CmdqInterfacePriv_t *)pPriv;

    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        return 0;
    }
#if 0
    if(pPrivData->nCmdqDrvIpId >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s Cmdq DRV_ID is ill(%d)\n", __func__,pPrivData->nCmdqDrvIpId);
        return -1;
    }
#endif
    /*calcu timout value (MIU is 333mhz)*/
    /*1.set poll jump enable and time value(MIU is 333mhz)*/
    _MdrvCmdCalcuTimeoutVlaue(nTimens,&nTimeBase,&nTimeCount);
    nSetJumpTimeValue = (nTimeCount & 0xffff);
    nRegValue  = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nCmdqCtlReg;
    nRegValue |= (0x28*2);
    MDrvCmdqWriteCommand(pPriv,nRegValue,nSetJumpTimeValue);

    nSetJumpTimeValue = ((nTimeCount >> 16) & 0xf);
    nSetJumpTimeValue |= 0x80;
    nSetJumpTimeValue |= ((nTimeBase &0xff) << 8);
    nRegValue  = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nCmdqCtlReg;
    nRegValue |= (0x29*2);
    MDrvCmdqWriteCommand(pPriv,nRegValue,nSetJumpTimeValue);

    CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "[CMDQ]delay cmd%d (%u)ns (%u-%u)\n", pPrivData->nCmdqDrvIpId
             , (u32) nTimens,nTimeBase,nTimeCount);

    /*2.set poll register*/
    nRegValue = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nDummyReg;
    if(!gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nInitDone)
    {
        gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nInitDone = 1;
        W2BYTE(pPrivData->nCmdqRiuBase, nRegValue, 0x5a5a);
    }

    MDrvCmdqPollEqCommandMask(pPriv,nRegValue,0xffff,0xffff,1);
    /*3.restore poll time out value and no jump*/
    nSetJumpTimeValue = 0xffff;
    nRegValue  = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nCmdqCtlReg;
    nRegValue |= (0x28*2);
    MDrvCmdqWriteCommand(pPriv,nRegValue,nSetJumpTimeValue);

    nSetJumpTimeValue = 0xff0f;
    nRegValue  = gDrvCmdqPollTimeReg[pPrivData->nCmdqDrvIpId].nCmdqCtlReg;
    nRegValue |= (0x29*2);
    MDrvCmdqWriteCommand(pPriv,nRegValue,nSetJumpTimeValue);
    return 0;
}

bool MDrvCmdqIsIdle(void* pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;
    bool                  bRetV;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return false;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        return true;
    }

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    bRetV =  HAL_CMDQ_Read_Idle_Done(pPrivData->nCmdqDrvIpId);

    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    return bRetV;
}

u16 MDrvCmdqReadIrqStatus(void* pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;
    u16                   nRetValue;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return false;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;
    CamOsMutexLock(&pPrivData->tCmdqMutex);
    nRetValue =  HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);

    return nRetValue;
}

s32 MDrvCmdqResetEngine(void* pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;


    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return -1;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    pPrivData->tCmdqInfo.nCurCmdCount   = 0;
    pPrivData->tCmdqInfo.pBufWrite        = pPrivData->tCmdqInfo.pBufStart;
    pPrivData->tCmdqInfo.pBufReadHw       = NULL;
    pPrivData->tCmdqInfo.pBufFireWriteHw  = NULL;
    pPrivData->bFirstFireDone           = 0;
#if 1/*set 3 times for clear*/
    HAL_CMDQ_Clear_TiggerBusIrq(pPrivData->nCmdqDrvIpId, 0xffff);
    HAL_CMDQ_Clear_TiggerBusIrq(pPrivData->nCmdqDrvIpId, 0xffff);
    HAL_CMDQ_Clear_TiggerBusIrq(pPrivData->nCmdqDrvIpId, 0xffff);
#endif
    MDrvCmdDbgPause(pPrivData->nCmdqDrvIpId);
    MDrvCmdDbgContinue(pPrivData->nCmdqDrvIpId);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    return 0;
}

s32  MDrvUpdateMloadRingBufReadPtr(void* pPriv, u64 nReadPtr)
{
    u32                   u32TmpAdr;
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return -1;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;
    u32TmpAdr = (u32) nReadPtr;
    CamOsMutexLock(&pPrivData->tMloadMutex);
    if(u32TmpAdr < (u32) pPrivData->tMloadInfo.pBufStart || u32TmpAdr > (u32)pPrivData->tMloadInfo.pBufEnd)
    {
        CMDQ_ERR("Mload ReadPtr is ill(0x%08x-0x%08x-0x%08x)\n", u32TmpAdr, (u32)pPrivData->tMloadInfo.pBufStart
                 , (u32)pPrivData->tMloadInfo.pBufEnd);
        CamOsMutexUnlock(&pPrivData->tMloadMutex);
        return -1;
    }
    pPrivData->tMloadInfo.pBufReadHw = (u8*)u32TmpAdr;
    CamOsMutexUnlock(&pPrivData->tMloadMutex);
    return 0;
}

u64  MDrvGetMloadRingBufWritePtr(void* pPriv)
{
    u32   nTmpBufWr;
    u64   nTmpBufWr_1;
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    CamOsMutexLock(&pPrivData->tMloadMutex);
    nTmpBufWr = (u32)pPrivData->tMloadInfo.pBufWrite;
    nTmpBufWr_1 = (u64)nTmpBufWr;
    CamOsMutexUnlock(&pPrivData->tMloadMutex);

    return nTmpBufWr_1;
}

u64  MDrvCmdqMloadCopyBuf(void* pPriv, void * mload_buf, u32 size, u16 u16Alignment)
{
    u64 nPhyAddr;
    u32 nBufWaitCnt = 0, nRetVal = 0;
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    CamOsMutexLock(&pPrivData->tMloadMutex);

    if((nRetVal = _MDrvCmdqCheckMloadBufferAvail(pPriv, size, u16Alignment)) == 0)
    {
        do
        {
            DrvCmdqOsDelayTask(SLEEP_POLL_TIME);
            nBufWaitCnt++;
            if(nBufWaitCnt > WAIT_SLEEP_TIME_CNT)
            {
                nRetVal = 0;
                CMDQ_ERR("%s Wait menuload buffer TimeOut\n", __func__);
                break;
            }
        }
        while((nRetVal = _MDrvCmdqCheckMloadBufferAvail(pPriv, size, u16Alignment)) == 0);

    }

    if(nRetVal != 0)
    {

        nPhyAddr = _MDrvCmdqCopyMloadBuf(pPriv, mload_buf, size, u16Alignment);
        CamOsMutexUnlock(&pPrivData->tMloadMutex);
        return nPhyAddr;
    }

    CamOsMutexUnlock(&pPrivData->tMloadMutex);
    return 0;
}

s32 MDrvAbortCmdqCommand(void* pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return -1;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    if(pPrivData->bForceRIU)
    {
        CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "%s force mode", __func__);
        return 0;
    }
    CamOsMutexLock(&pPrivData->tCmdqMutex);
    pPrivData->tCmdqInfo.pBufWrite = (u8*)(((u32)pPrivData->tCmdqInfo.pBufFireWriteHw) << DRVCMD_BUFFER_SHIFT);
    pPrivData->tCmdqInfo.nCurCmdCount = 0;
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
    return 0;
}

s32 MDrvCmdqWriteDummyCmdForRiu(void* pPriv, u16 nValue)
{
    CmdqInterfacePriv_t * pPrivData = NULL;
    u32                   nDumyAdr;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return -1;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;


    nDumyAdr = HAL_CMDQ_Get_Dummy_Register_RiuAddr(pPrivData->nCmdqDrvIpId);
#if 0
    nDumyAdr = nDumyAdr >> 2;

    u16Bank =  nDumyAdr >> 16;
    u8addr  =  nDumyAdr & 0xff;

    nDumyAdr = (u16Bank + u8addr);
#endif

    if(MDrvCmdqWriteCommandMask(pPriv, nDumyAdr, nValue, 0xffff) < 0)
    {
        CMDQ_ERR("%s write cmdq dummy error\n", __func__);
        return -1;
    }

    return 0;
}

s32 MDrvCmdqWriteDummyRegister(void* pPriv, u16 nValue)
{
    CmdqInterfacePriv_t * pPrivData = NULL;
    u32                   nDumyAdr;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return -1;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    if(pPrivData->bForceRIU)
    {
        if(HAL_CMDQ_Write_Dummy_Register(pPrivData->nCmdqDrvIpId, nValue) < 0)
        {
            CMDQ_ERR("%s write dummy error\n", __func__);
            return -1;
        }
        return 0;
    }

    nDumyAdr = HAL_CMDQ_Get_Dummy_Register_RiuAddr(pPrivData->nCmdqDrvIpId);
    //CMDQ_LOG("nDumyAdr=0x%x\n", nDumyAdr);

    if(MDrvCmdqWriteCommandMask(pPriv, nDumyAdr, nValue, 0xffff) < 0)
    {
        CMDQ_ERR("%s write cmdq dummy error\n", __func__);
        return -1;
    }

    return 0;
}

u32 MDrvGetDummyRegisterRiuAddr(void* pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;
    u32                   nRetValue;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return -1;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    nRetValue =   HAL_CMDQ_Get_Dummy_Register_RiuAddr(pPrivData->nCmdqDrvIpId);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);

    return nRetValue;
}

u16 MDrvCmdqReadDummyRegister(void* pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;
    u16                   nRetValue;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    //CamOsMutexLock(&pPrivData->tCmdqMutex);
    nRetValue = (u16)HAL_CMDQ_Read_Dummy_Register(pPrivData->nCmdqDrvIpId);
    //CamOsMutexUnlock(&pPrivData->tCmdqMutex);

    return nRetValue;
}

//---------------------------------------------------------------------------
//  Debug function.
//---------------------------------------------------------------------------
void MDrvCmdDbgWriteDummy(void* pPriv, u32 value)
{
    CmdqInterfacePriv_t * pPrivData = NULL;

    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return;
    }

    pPrivData = (CmdqInterfacePriv_t*)pPriv;
    CamOsMutexLock(&pPrivData->tCmdqMutex);
    HAL_CMDQ_Write_Dummy_Register(pPrivData->nCmdqDrvIpId, value);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);
}

u32 MDrvCmdDbgReadDummy(void* pPriv)
{
    CmdqInterfacePriv_t * pPrivData = NULL;
    u32                   nRetValue;
    if(pPriv == NULL)
    {
        CMDQ_ERR("%s priv is NULL\n", __func__);
        return 0;
    }
    pPrivData = (CmdqInterfacePriv_t*)pPriv;

    CamOsMutexLock(&pPrivData->tCmdqMutex);
    nRetValue =  HAL_CMDQ_Read_Dummy_Register(pPrivData->nCmdqDrvIpId);
    CamOsMutexUnlock(&pPrivData->tCmdqMutex);

    return nRetValue;
}

void MDrvCmdDbgPause(DrvCmdqIPSupport_e eIpNum)
{
    HAL_CMDQ_Reset2(eIpNum, 0);
}

void MDrvCmdDbgContinue(DrvCmdqIPSupport_e eIpNum)
{
    HAL_CMDQ_Reset2(eIpNum, 1);
}

void MDrvCmdqDumpContext(void)
{
    int i;
    u32 s, e, r, w;

    CMDQ_LOG("-- Total %d CMDQ HW ----------------------------------------------------\n", NUMBER_OF_CMDQ_HW);
    for(i = 0; i < NUMBER_OF_CMDQ_HW; i++)
    {
        if(!gDrvCmdPriv[i].bUsed)
            continue;

        CMDQ_LOG(" ====================================================== CMDQ #%d ===============\n", i);
        CMDQ_LOG(" Mode: (%d-%d-%d)DRVID(%d-%d)\n",
                 gDrvCmdPriv[i].nCmdMode, gDrvCmdPriv[i].bForceRIU, gDrvCmdPriv[i].bUsed, gDrvCmdPriv[i].CmdqIpId, gDrvCmdPriv[i].nCmdqDrvIpId);

        CMDQ_LOG(" Basis: miu        size         phyadr -     viradr \n");
        CMDQ_LOG("        0x%8x      0x%8x        0x%08X - 0x%08X \n",
                 gDrvCmdPriv[i].nCmdqRiuBase, gDrvCmdPriv[i].tCmdqBuf.nBufSize
                 , gDrvCmdPriv[i].tCmdqBuf.BufPhy, gDrvCmdPriv[i].tCmdqBuf.BufVir);

        {

            s = (u32)gDrvCmdPriv[i].tCmdqInfo.pBufStart;
            e = (u32)gDrvCmdPriv[i].tCmdqInfo.pBufEnd;
            w = (u32)gDrvCmdPriv[i].tCmdqInfo.pBufWrite;
            r = HAL_CMDQ_Read_Pointer(i);
            CMDQ_LOG("s,e,w,r: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", s, e, w, r);
            CMDQ_LOG("fire_w,total_cnt,current_cnt: 0x%08x,%d,%d\n", (u32)gDrvCmdPriv[i].tCmdqInfo.pBufFireWriteHw
                     , gDrvCmdPriv[i].tCmdqInfo.nTotalCmdCount, gDrvCmdPriv[i].tCmdqInfo.nCurCmdCount);
        }

        CMDQ_LOG(" Mload Basis: miu        size         phyadr -     viradr \n");
        CMDQ_LOG("              0x%08x     0x%8x        0x%08X - 0x%08X  \n",
                 gDrvCmdPriv[i].nCmdqRiuBase, gDrvCmdPriv[i].tMloadBuf.nBufSize,
                 gDrvCmdPriv[i].tMloadBuf.BufPhy, gDrvCmdPriv[i].tMloadBuf.BufVir);

        {

            s = (u32)gDrvCmdPriv[i].tMloadInfo.pBufStart;
            e = (u32)gDrvCmdPriv[i].tMloadInfo.pBufEnd;
            w = (u32)gDrvCmdPriv[i].tMloadInfo.pBufWrite;
            r = (u32)gDrvCmdPriv[i].tMloadInfo.pBufReadHw;
            CMDQ_LOG("Mload s,e,w,r: 0x%8x, 0x%8x, 0x%8x, 0x%8x\n", s, e, w, r);

        }

        HAL_CMDQ_Print_All(i);
    }
    CMDQ_LOG("-- CMDQ Dump Context ----------------------------------------------------------\n");

}
