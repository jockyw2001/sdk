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
// (!¡±MStar Confidential Information!¡L) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
//==============================================================================
#ifndef _DRV_SCLIRQ_H
#define _DRV_SCLIRQ_H


//-------------------------------------------------------------------------------------------------
//  Defines & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_SCLIRQ_M2MEVENT_CLEAR      = 0x00000000, // no event
    E_SCLIRQ_M2MEVENT_ACTIVE     = 0x00000001, // SC3FRM act
    E_SCLIRQ_M2MEVENT_ACTIVEN    = 0x00000002, // SC3FRM act_N
    E_SCLIRQ_M2MEVENT_END        = 0x00000003,
    E_SCLIRQ_M2MEVENT_DONE       = 0x00000004, // SC3FRM done
    E_SCLIRQ_M2MEVENT_IRQ        = 0x00000008, // any one ISR need to handle
} SCLIRQM2MEvent;
typedef enum
{
	E_DRV_SCL_IRQ_HIST_PORT0   = 0,
    E_DRV_SCL_IRQ_HIST_PORT1   ,
    E_DRV_SCL_IRQ_HIST_PORT2   ,
    E_DRV_SCL_IRQ_HIST_PORT3   ,
    E_DRV_SCL_IRQ_HIST_MAX   ,
} SCLIRQPortHistType;

typedef enum
{
	E_DRV_SCL_IRQ_OFF   = 0,
    E_DRV_SCL_IRQ_OFFM2M = 0x1000,
    E_DRV_SCL_IRQ_OFFROT = 0x2000,
    E_DRV_SCL_IRQ_PORT0 = 0x1,
    E_DRV_SCL_IRQ_PORT1 = 0x2,
    E_DRV_SCL_IRQ_PORT2 = 0x4,
    E_DRV_SCL_IRQ_PORT3 = 0x8,
    E_DRV_SCL_IRQ_ONLYDONEINT_MODE = 0x10,
    E_DRV_SCL_IRQ_MULTIINT_MODE = 0x20,
    E_DRV_SCL_IRQ_FRAME_MODE = 0x40,
    E_DRV_SCL_IRQ_REALTIME_MODE = 0x80,
    E_DRV_SCL_IRQ_M2M_MODE = 0x100,
    E_DRV_SCL_IRQ_LDC_MODE = 0x200,
    E_DRV_SCL_IRQ_ROT_MODE = 0x400,
    E_DRV_SCL_IRQ_PORT_ROTWDMA = 0x800,
    E_DRV_SCL_IRQ_3PORT_MODE = (E_DRV_SCL_IRQ_PORT0|E_DRV_SCL_IRQ_PORT1|E_DRV_SCL_IRQ_PORT3),
    E_DRV_SCL_IRQ_4PORT_MODE = (E_DRV_SCL_IRQ_PORT0|E_DRV_SCL_IRQ_PORT1|E_DRV_SCL_IRQ_PORT2|E_DRV_SCL_IRQ_PORT3),
	E_DRV_SCL_IRQ_DVR_FRAMEBUF_4PORT = (E_DRV_SCL_IRQ_4PORT_MODE|E_DRV_SCL_IRQ_ONLYDONEINT_MODE|E_DRV_SCL_IRQ_FRAME_MODE),
    E_DRV_SCL_IRQ_DVR_FRAMEBUF_3PORT = (E_DRV_SCL_IRQ_3PORT_MODE|E_DRV_SCL_IRQ_ONLYDONEINT_MODE|E_DRV_SCL_IRQ_FRAME_MODE),
    E_DRV_SCL_IRQ_CAM_FRAMEBUF_4PORT = (E_DRV_SCL_IRQ_4PORT_MODE|E_DRV_SCL_IRQ_MULTIINT_MODE|E_DRV_SCL_IRQ_FRAME_MODE),
	E_DRV_SCL_IRQ_CAM_FRAMEBUF_3PORT = (E_DRV_SCL_IRQ_3PORT_MODE|E_DRV_SCL_IRQ_MULTIINT_MODE|E_DRV_SCL_IRQ_FRAME_MODE),
    E_DRV_SCL_IRQ_CAM_REALTIME_4PORT = (E_DRV_SCL_IRQ_4PORT_MODE|E_DRV_SCL_IRQ_MULTIINT_MODE|E_DRV_SCL_IRQ_REALTIME_MODE),
    E_DRV_SCL_IRQ_CAM_REALTIME_3PORT = (E_DRV_SCL_IRQ_3PORT_MODE|E_DRV_SCL_IRQ_MULTIINT_MODE|E_DRV_SCL_IRQ_REALTIME_MODE),
    E_DRV_SCL_IRQ_PORT2_M2M = (E_DRV_SCL_IRQ_PORT2|E_DRV_SCL_IRQ_M2M_MODE),
    E_DRV_SCL_IRQ_ROT_WDMA = (E_DRV_SCL_IRQ_ROT_MODE|E_DRV_SCL_IRQ_PORT_ROTWDMA),
}DrvSclIrqEnableMode_e;
typedef enum
{
    E_DRV_SCL_IRQ_AFF = 0x1,
    E_DRV_SCL_IRQ_VSYNC = 0x2,
    E_DRV_SCL_IRQ_ISPDONE = 0x4,
    E_DRV_SCL_IRQ_SCDONE = 0x8,
    E_DRV_SCL_IRQ_ERR = (E_DRV_SCL_IRQ_VSYNC|E_DRV_SCL_IRQ_SCDONE),
    E_DRV_SCL_IRQ_PROCESSCNT = 0x10,
    E_DRV_SCL_IRQ_ADDPORT0QUEUE = 0x20,
    E_DRV_SCL_IRQ_ADDPORT1QUEUE = 0x40,
    E_DRV_SCL_IRQ_ADDPORT2QUEUE = 0x80,
    E_DRV_SCL_IRQ_ADDPORT3QUEUE = 0x100,
    E_DRV_SCL_IRQ_PROCESSLATE = 0x200,
    E_DRV_SCL_IRQ_PROCESSDROP = 0x400,
    E_DRV_SCL_IRQ_PROCESSDIFF = 0x800,
    E_DRV_SCL_IRQ_FERR = 0x1000,
}DrvSclIrqScIntsType_e;

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    u32 u32RiuBase;
    u32 u32IRQNUM;
    u32 u32CMDQIRQNUM;
    void *pvCtx;
}DrvSclIrqInitConfig_t;

typedef struct
{
    u32 u32IRQNUM;
    u32 u32CMDQIRQNUM;
}DrvSclIrqSuspendResumeConfig_t;
typedef struct
{
    u32 u32AffCount;
    u32 u32ISPInCount;
    u32 u32ISPDoneCount;
    u32 u32DoneCount[E_DRV_SCL_IRQ_HIST_MAX];
    u32 u32M2MDoneCount;
    u32 u32SCLMainDoneCount;
    u32 u32ActiveAddr[E_DRV_SCL_IRQ_HIST_MAX];
    u32 u32M2MActiveTime;
    u32 u32SCLMainActiveTime;
    u32 u32ISPTime;
    u32 u32ISPBlankingTime;
    u32 u32ProcessDiffTime;
    u32 u32ProcessDiffDoneTime;
    u16 u16ErrorCount;
    u16 u16FakeErrorCount;
    u8 u8QueueRCnt[E_DRV_SCL_IRQ_HIST_MAX];
    u8 u8Act[E_DRV_SCL_IRQ_HIST_MAX];
    u8 u8QueueWCnt[E_DRV_SCL_IRQ_HIST_MAX];
    u8 u8ProcessCnt;
    u8 u8ProcessActCnt;
    u8 u8ProcessLateCnt;
    u8 u8ProcessDropCnt;
}DrvSclIrqScIntsType_t;
typedef struct
{
    u32 u32DoneCount[E_DRV_SCL_IRQ_HIST_MAX];
    u32 u32OriTime[E_DRV_SCL_IRQ_HIST_MAX];
    u32 u32Fps[E_DRV_SCL_IRQ_HIST_MAX];
}DrvSclIrqScIntsFpsType_t;
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifndef DRV_SCLIRQ_C
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE void DrvSclIrqSetPTGenStatus(bool bPTGen);
INTERFACE void DrvSclIrqSetFrameDoneInt(u64 u64FrameDoneInt);
INTERFACE u8 DrvSclIrqGetCheckcropflag(void);
INTERFACE void DrvSclIrqSetSclInts(DrvSclIrqScIntsType_e enType,u64 u64Time);
INTERFACE void DrvSclIrqSetM2MInts(DrvSclIrqScIntsType_e enType,u64 u64Time);
INTERFACE u8 DrvSclIrqGetSclProcessCnt(void);
INTERFACE DrvSclIrqScIntsType_t* DrvSclIrqGetSclInts(void);
INTERFACE void DrvSclIrqSetDropFrameFromCmdqDone(u8 u8Count);
INTERFACE void DrvSclIrqSetDropFrameFromIsp(u8 u8Count);
INTERFACE bool DrvSclIrqInitVariable(void);
INTERFACE bool DrvSclIrqGetIsBlankingRegion(void);
INTERFACE bool DrvSclIrqInit(DrvSclIrqInitConfig_t *pCfg);
INTERFACE bool DrvSclIrqExit(void);
INTERFACE bool DrvSclIrqInterruptEnable(u16 u16IRQ);
INTERFACE bool DrvSclIrqM2MInterruptEnable(u16 u16IRQ);
INTERFACE bool DrvSclIrqDisable(u16 u16IRQ);
INTERFACE bool DrvSclIrqM2MDisable(u16 u16IRQ);
INTERFACE bool DrvSclIrqCmdqInterruptEnable(u16 u16IRQ);
INTERFACE bool DrvSclIrqCmdqDisable(u16 u16IRQ);
INTERFACE bool DrvSclIrqGetFlag(u16 u16IRQ, u64 *pFlag);
INTERFACE bool DrvSclIrqSetClear(u16 u16IRQ);
INTERFACE bool DrvSclIrqGetFlags_Msk(u64 u64IrqMsk, u64 *pFlags);
INTERFACE bool DrvSclIrqSetClear_Msk(u64 u64IrqMsk);
INTERFACE bool DrvSclIrqSetMask(u64 u64IRQ);
INTERFACE bool DrvSclIrqSuspend(DrvSclIrqSuspendResumeConfig_t *pCfg);
INTERFACE bool DrvSclIrqResume(DrvSclIrqSuspendResumeConfig_t *pCfg);
INTERFACE s32 DrvSclIrqGetIrqSYNCEventID(void);
INTERFACE void scl_ve_isr(void);
INTERFACE void scl_vs_isr(void);
INTERFACE void DrvSclIrqSetSclIsrOpenMode(DrvSclIrqEnableMode_e emode);
INTERFACE void DrvSclIrqSetM2MIsrOpenMode(DrvSclIrqEnableMode_e emode);
INTERFACE s32 DrvSclIrqGetIRQM2MEventID(void);
INTERFACE DrvSclIrqEnableMode_e DrvSclIrqGetSclIsrOpenMode(void);
INTERFACE void DrvSclIrqSetDmaOff(s32 s32Handler,bool bEnImi);
INTERFACE bool DrvSclIrqGetMask(u64 u64IRQ, u64 *pFlag);
INTERFACE DrvSclIrqScIntsType_e DrvSclIrqIsrHandler(u64 u64Time);
INTERFACE DrvSclIrqScIntsType_e DrvSclIrqM2MIsrHandler(u64 u64Time);
INTERFACE bool DrvSclIrqGetIsCMDQIdle(void);
INTERFACE void DrvSclIrqSetCMDQIdle(bool bEn);

#undef INTERFACE

#endif
