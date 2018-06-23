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
#define DRV_SCL_IRQ_C

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"

#include "hal_scl_util.h"
#include "hal_scl_reg.h"
#include "drv_scl_irq_st.h"
#include "drv_scl_irq.h"
#include "drv_scl_dbg.h"
#include "drv_scl_dma_st.h" //add later
#include "hal_scl_dma.h" //add later
#include "drv_scl_dma.h" //add later

#include "drv_scl_hvsp_st.h"
#include "drv_scl_hvsp.h"
#include "hal_scl_irq.h"
#include "drv_scl_vip_m_st.h"
#include "drv_scl_vip_m.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_SCLIRQ_DBG(x)
#define DRV_SCLIRQ_DBG_H(x)
#define DRV_SCLIRQ_ERR(x)      x
#define TASKLET_ALREADY 0x8000000000000000
#define IN_TASKLET_ALREADY(u64flag) (((u64)u64flag & TASKLET_ALREADY)!= 0)
#define freerunID               5
#define CRITICAL_SECTION_TIME 80
#define CHANGE_CRITICAL_SECTION_TIMING_RANGE 3000 // (ns)
#define CHANGE_CRITICAL_SECTION_TIMING_COUNT 60 // (ns)
#define CRITICAL_SECTION_TIMING_MIN 500 // (ns)
#define _Is_SCLDMA_RingMode(enClientType)            (pgstScldmaInfo->enBuffMode[(enClientType)] == E_DRV_SCLDMA_BUF_MD_RING)
#define _Is_SCLDMA_SingleMode(enClientType)         (pgstScldmaInfo->enBuffMode[(enClientType)] == E_DRV_SCLDMA_BUF_MD_SINGLE)
#define _Is_SCLDMA_SWRingMode(enClientType)         (pgstScldmaInfo->enBuffMode[(enClientType)] == E_DRV_SCLDMA_BUF_MD_SWRING)
#define _IsFrmIN_Vsync()                                  (gbFRMInFlag & (E_DRV_SCLIRQ_FRM_IN_COUNT_NOW ))
#define _IsFrmIN()                                  (gbFRMInFlag & (E_DRV_SCLIRQ_FRM_IN_COUNT_NOW << SCL_DELAYFRAME))
#define _IsFrmIN_Last()                                   (gbFRMInFlag & (E_DRV_SCLIRQ_FRM_IN_COUNT_NOW<< (SCL_DELAYFRAME+1) ))
#define _IsFrmDrop()                                  (gstDropFrameCount)
#define DRV_SCLIRQ_MUTEX_LOCK_ISR()        DrvSclOsObtainMutexIrq(*_pSCLIRQ_SCLDMA_Mutex)
#define DRV_SCLIRQ_MUTEX_UNLOCK_ISR()        DrvSclOsReleaseMutexIrq(*_pSCLIRQ_SCLDMA_Mutex)
#define DRV_SCLIRQ_MUTEX_LOCK()             DrvSclOsObtainMutex(_SCLIRQ_Mutex , SCLOS_WAIT_FOREVER)
#define DRV_SCLIRQ_MUTEX_UNLOCK()             DrvSclOsReleaseMutex(_SCLIRQ_Mutex)
#define _IsFlagType(u8client,u8type)                     (pgstScldmaInfo->bDmaflag[(u8client)] & (u8type))
#define _SetANDGetFlagType(u8client,u8Set,u8get)         (pgstScldmaInfo->bDmaflag[(u8client)]=((pgstScldmaInfo->bDmaflag[(u8client)]&(u8get))|(u8Set)))
#define _SetFlagType(u8client,u8type)                    (pgstScldmaInfo->bDmaflag[(u8client)] |= (u8type))
#define _ReSetFlagType(u8client,u8type)                  (pgstScldmaInfo->bDmaflag[(u8client)] &= ~(u8type))
#define _GetIdxType(u8client,u8type)                     (pgstScldmaInfo->bDMAidx[(u8client)] & (u8type))
#define _SetANDGetIdxType(u8client,u8Set,u8get)         (pgstScldmaInfo->bDMAidx[(u8client)]=((pgstScldmaInfo->bDMAidx[(u8client)]&(u8get))|(u8Set)))
#define _SetIdxType(u8client,u8type)                     (pgstScldmaInfo->bDMAidx[(u8client)] |= (u8type))
#define _ReSetIdxType(u8client,u8type)                   (pgstScldmaInfo->bDMAidx[(u8client)] &= ~(u8type))
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvSclDmaInfoType_t  *pgstScldmaInfo;
MDrvSclCtxSclIrqGlobalSet_t *gstGlobalSclIrqSet;

//keep or use for isr
s32 _SCLIRQ_Mutex = -1;

s32* _pSCLIRQ_SCLDMA_Mutex = NULL;
u32 gu32Time;
bool _bSCLIRQ_Suspend = 0;
DrvSclDmaThreadConfig_t gstThreadCfg;
s32  _s32FRMENDEventId;
s32  _s32SYNCEventId;
s32  _s32SC3EventId;
DrvSclIrqDazaEventType_e genDAZAEvent;
//only ISR
bool gbEachDMAEnable;
volatile bool gbInBlanking;
DrvSclIrqTxEventType_e gsclirqstate;
DrvSclIrqSc3EventType_e gscl3irqstate;
bool gbFRMInFlag;
bool gbBypassDNR;
u8 gstDropFrameCount;
bool gbChangeTimingCount = 0;
DrvSclIrqScIntsType_t gstSCInts;
u32 gu32ActTime[E_DRV_SCLIRQ_Clk_MAX];
u32 gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_MAX];
u8 gbDMADoneEarlyISP;
DrvSclIrqEnableMode_e        _geIrqEnableMode = E_DRV_SCL_IRQ_OFF;

#if ENABLE_ACTIVEID_ISR
void _DrvSclIrqSetDazaQueueWork(DrvSclIrqDazaEventType_e enEvent);
#endif

//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
void _DrvSclIrqSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstGlobalSclIrqSet = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stSclIrqCfg);
    pgstScldmaInfo = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stScldmaInfo);
}

#if (ENABLE_CMDQ_ISR)
void _DrvSclIrqCmdqStateTrig(void)
{
    DrvSclOsSetEvent(_s32SYNCEventId, E_DRV_SCLIRQ_EVENT_CMDQGOING);
}
void _DrvSclIrqCmdqStateDone(void)
{
    u32 u32Events;
    bool bLDC = 0;
    u32Events = DrvSclOsGetEvent(_s32SYNCEventId);
    if(u32Events & E_DRV_SCLIRQ_EVENT_BRESETDNR)
    {
        SCL_DBGERR("[DRVSCLIRQ]%s:!!!E_SCLIRQ_DAZA_BRESETDNR\n",__FUNCTION__);
        HalSclIrqSetReg(REG_SCL0_03_L, 0, BIT12); //LDC path close
        HalSclIrqSetDNRBypass(1);
        gbBypassDNR = 1;
        DrvSclOsSetSclFrameDelay(0);
        DrvSclOsClearEvent(_s32SYNCEventId,(E_DRV_SCLIRQ_EVENT_BRESETDNR));
    }
    if(u32Events & E_DRV_SCLIRQ_EVENT_SETDELAY)
    {
        bLDC = ((HalSclIrqGetRegVal(REG_SCL0_03_L)&BIT12)? 1 : 0);
        SCL_DBG(SCL_DBG_LV_DRVCMDQ()&EN_DBGMG_CMDQEVEL_ISR,
            "[DRVSCLIRQ]%s:!!!E_SCLIRQ_EVENT_SETDELAY %hhd\n",__FUNCTION__,bLDC);
        DrvSclOsSetSclFrameDelay(bLDC);
        DrvSclOsClearEvent(_s32SYNCEventId,(E_DRV_SCLIRQ_EVENT_SETDELAY));
    }
    DrvSclOsSetEvent(_s32SYNCEventId, E_DRV_SCLIRQ_EVENT_CMDQDONE);
    DrvSclCmdqSetLPoint(E_DRV_SCLCMDQ_TYPE_IP0);
    DrvSclOsQueueWork(1,gstThreadCfg.s32Taskid[E_DRV_SCLIRQ_SCTASKID_DAZA],
        gstThreadCfg.s32HandlerId[E_DRV_SCLIRQ_SCTASK_WORKQUEUECMDQ],0);
}
void _DrvSclIrqCmdqIsr(void)
{
    u32 u32IrqFlag;
    u32 u32Time;
    u32IrqFlag = (u32)DrvSclCmdqGetFinalIrq(E_DRV_SCLCMDQ_TYPE_IP0);
    DrvSclCmdqClearIrqByFlag(E_DRV_SCLCMDQ_TYPE_IP0,u32IrqFlag);
    u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
    if(u32IrqFlag & CMDQ_IRQ_STATE_TRIG)
    {
        _DrvSclIrqCmdqStateTrig();
    }
    if(u32IrqFlag & CMDQ_IRQ_STATE_DONE)
    {
        _DrvSclIrqCmdqStateDone();
    }
    SCL_DBG(SCL_DBG_LV_DRVCMDQ()&EN_DBGMG_CMDQEVEL_ISR, "[CMDQ]u32IrqFlag:%lx  %lu\n",u32IrqFlag,u32Time);
}
#endif
#if I2_DVR
#else
void _DrvSclIrqSclFrameEndConnectToIsp(void)
{
    scl_ve_isr();
}
void _DrvSclIrqSclFrameStartConnectToIsp(void)
{
    scl_vs_isr();
}
#endif
#if ENABLE_ACTIVEID_ISR

s32 DrvSclIrqGetIrqSYNCEventID(void)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclIrqSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    return _s32SYNCEventId;
}

void _DrvSclIrqSetIsBlankingRegion(bool bBk)
{
    gbInBlanking = bBk;
}
bool DrvSclIrqGetIsBlankingRegion(void)
{
    bool bRet = 0;
    u32 u32Time;
    u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
    //critical section
    if(gbInBlanking && gu32ActTime[E_DRV_SCLIRQ_CLK_FRMDONE] && gstSCInts.u32ISPDoneCount&&
        ((u32Time - gu32ActTime[E_DRV_SCLIRQ_CLK_FRMDONE])>((gstSCInts.u32ISPBlankingTime)-CRITICAL_SECTION_TIME)))
    {
        bRet = 0;
        if((u32Time - gu32ActTime[E_DRV_SCLIRQ_CLK_FRMDONE])>((gstSCInts.u32ISPBlankingTime)*2))
        {
            // error
            bRet = gbInBlanking;
        }
        else
        {
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()==(EN_DBGMG_SCLIRQLEVEL_ELSE),
                "[SCLIRQ]Blanking Critical Section (%ld ,%ld)\n",
                (u32Time - gu32ActTime[E_DRV_SCLIRQ_CLK_FRMDONE]),((gstSCInts.u32ISPBlankingTime)-CRITICAL_SECTION_TIME));
        }
    }
    else
    {
        bRet = gbInBlanking;
    }
    return bRet;
}
DrvSclIrqTxEventType_e _DrvSclIrqGetISTEventFlag(DrvSclDmaClientType_e enClient)
{
    switch(enClient)
    {
        case E_DRV_SCLDMA_1_FRM_W:
            return E_DRV_SCLIRQ_EVENT_ISTSC1FRM;
        case E_DRV_SCLDMA_1_SNP_W:
            return E_DRV_SCLIRQ_EVENT_ISTSC1SNP;
        case E_DRV_SCLDMA_2_FRM_W:
            return E_DRV_SCLIRQ_EVENT_ISTSC2FRM;
        case E_DRV_SCLDMA_2_FRM2_W:
            return E_DRV_SCLIRQ_EVENT_ISTSC2FRM2;
        default :
            return 0;
    }
}
void _DrvSclIrqSetSingleModeDmaInfo(DrvSclDmaClientType_e enclient, u32 u32Events)
{
    if ((u32Events & (SCLIRQ_ISTEVENT_BASE << enclient)) && _Is_SCLDMA_SingleMode(enclient))
    {
        DRV_SCLIRQ_MUTEX_LOCK_ISR();
        pgstScldmaInfo->bDMAOnOff[enclient] = 0;
        DrvSclDmaSetFrameResolution(enclient);
        DRV_SCLIRQ_MUTEX_UNLOCK_ISR();
    }
}
void _DrvSclIrqSetSingleModeDmaInfoNoLock(DrvSclDmaClientType_e enclient, u32 u32Events)
{
    if ((u32Events & (SCLIRQ_ISTEVENT_BASE << enclient)) && _Is_SCLDMA_SingleMode(enclient))
    {
        pgstScldmaInfo->bDMAOnOff[enclient] = 0;
        DrvSclDmaSetFrameResolution(enclient);
    }
}
static void _DrvSclIrqRingModeActiveWithoutDoublebuffer(DrvSclDmaClientType_e enClient)
{
    u8 u8RPoint = 0;
    u32 u32Time;
    u16  u16RealIdx;
    u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
    //u8RPoint = (_GetIdxType(enClient,E_DRV_SCLDMA_ACTIVE_BUFFER_OMX))>>4;
    if(_IsFlagType(enClient,E_DRV_SCLDMA_FLAG_ACTIVE))
    {
        _SetFlagType(enClient,E_DRV_SCLDMA_FLAG_FRMIGNORE);
        SCL_DBGERR("[DRVSCLIRQ]%d!!!!!!!!!!!!double active\n",enClient);
    }
    else
    {
        _SetANDGetFlagType(enClient,E_DRV_SCLDMA_FLAG_ACTIVE,(~E_DRV_SCLDMA_FLAG_FRMDONE));
    }
    u8RPoint = (_GetIdxType(enClient,E_DRV_SCLDMA_ACTIVE_BUFFER_OMX))>>4;
    if(!_IsFlagType(enClient,E_DRV_SCLDMA_FLAG_DMAOFF))
    {
        u16RealIdx = HalSclDmaGetRWIdx(enClient);
        if(u16RealIdx == 0 && _GetIdxType(enClient,E_DRV_SCLDMA_ACTIVE_BUFFER_SCL) != pgstScldmaInfo->bMaxid[enClient]&&
            !(_IsFlagType(enClient,E_DRV_SCLDMA_FLAG_BLANKING)))
        {
            _SetFlagType(enClient,E_DRV_SCLDMA_FLAG_FRMIGNORE);
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&Get_DBGMG_SCLIRQclient(enClient,0),
                "[DRVSCLDMA]%d wodb ISR ignore\n"
            ,enClient);
        }
    }
    else
    {
        u16RealIdx = _GetIdxType(enClient,E_DRV_SCLDMA_ACTIVE_BUFFER_SCL);
    }
    DrvSclDmaSetFrameResolution(enClient);
    _SetANDGetIdxType(enClient,u16RealIdx,E_DRV_SCLDMA_ACTIVE_BUFFER_OMX);
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&Get_DBGMG_SCLIRQclient(enClient,0),
        "[DRVSCLDMA]%d wodb ISR R_P=%hhx RealIdx=%hx flag:%hx @:%lu\n"
    ,enClient,u8RPoint,u16RealIdx,pgstScldmaInfo->bDmaflag[enClient],u32Time);
}
static bool _DrvSclIrqRingModeActive_NWithoutDoublebuffer(DrvSclDmaClientType_e enClient)
{
    u8 u8RPoint = 0;
    u16  u16RealIdx;
    u8RPoint = (_GetIdxType(enClient,E_DRV_SCLDMA_ACTIVE_BUFFER_OMX))>>4;
    u16RealIdx = _GetIdxType(enClient,E_DRV_SCLDMA_ACTIVE_BUFFER_SCL);
    if((!_IsFlagType(enClient,E_DRV_SCLDMA_FLAG_FRMIN)))
    {
        _SetFlagType(enClient,E_DRV_SCLDMA_FLAG_FRMIGNORE);
    }
    if(!_IsFlagType(enClient,E_DRV_SCLDMA_FLAG_FRMIGNORE))
    {
        if(u8RPoint==u16RealIdx && !_IsFlagType(enClient,E_DRV_SCLDMA_FLAG_BLANKING))//5 is debug freerun
        {
            //trig off
            if(u8RPoint != freerunID )
            {
                DrvSclDmaSetSWReTrigCount(enClient,1);
                DrvSclDmaSetISRHandlerDmaOff(enClient,0);
                _SetANDGetFlagType
                    (enClient,E_DRV_SCLDMA_FLAG_FRMDONE|E_DRV_SCLDMA_FLAG_EVERDMAON|E_DRV_SCLDMA_FLAG_DMAOFF,(~E_DRV_SCLDMA_FLAG_ACTIVE));
                _SetANDGetIdxType(enClient,(u16RealIdx),(E_DRV_SCLDMA_ACTIVE_BUFFER_OMX));
            }
            else
            {
                _SetANDGetIdxType(enClient,(u16RealIdx),(E_DRV_SCLDMA_ACTIVE_BUFFER_OMX));
                _SetANDGetFlagType(enClient,E_DRV_SCLDMA_FLAG_FRMDONE,(~(E_DRV_SCLDMA_FLAG_DMAOFF|E_DRV_SCLDMA_FLAG_ACTIVE)));
            }
        }
        else
        {
            _SetANDGetFlagType(enClient,E_DRV_SCLDMA_FLAG_FRMDONE,(~E_DRV_SCLDMA_FLAG_ACTIVE));
        }
        if(_IsFlagType(enClient,E_DRV_SCLDMA_FLAG_DMAOFF))
        {
            pgstScldmaInfo->bDMAOnOff[enClient] = 0;
        }
        if(_IsFlagType(enClient,E_DRV_SCLDMA_FLAG_BLANKING))
        {
            _ReSetFlagType(enClient,E_DRV_SCLDMA_FLAG_BLANKING);
        }
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&Get_DBGMG_SCLIRQclient(enClient,1), "[DRVSCLDMA]%d ISR actN wodb flag:%hx idx:%hhx @:%lu\n"
        ,enClient,pgstScldmaInfo->bDmaflag[enClient],pgstScldmaInfo->bDMAidx[enClient],((u32)DrvSclOsGetSystemTimeStamp()));
        return 1;
    }
    else
    {
        _ReSetFlagType(enClient,E_DRV_SCLDMA_FLAG_FRMIGNORE);
        if(u8RPoint != freerunID )
        {
            DrvSclDmaSetDMAIgnoreCount(enClient,1);
            DrvSclDmaSetISRHandlerDmaOff(enClient,0);
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&Get_DBGMG_SCLIRQclient(enClient,1),
                "[DRVSCLDMA]%d ISR actN wodb ignore flag:%hx\n"
            ,enClient,pgstScldmaInfo->bDmaflag[(enClient)]);
            _SetANDGetFlagType
                (enClient,E_DRV_SCLDMA_FLAG_FRMDONE|E_DRV_SCLDMA_FLAG_EVERDMAON|E_DRV_SCLDMA_FLAG_DMAOFF,(~(E_DRV_SCLDMA_FLAG_ACTIVE)));
        }
        if(_IsFlagType(enClient,E_DRV_SCLDMA_FLAG_DMAOFF))
        {
            pgstScldmaInfo->bDMAOnOff[enClient] = 0;
        }
        _ReSetFlagType(enClient,E_DRV_SCLDMA_FLAG_FRMIN);
        DrvSclOsSetEvent(_s32FRMENDEventId, (0x1 <<enClient));
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&Get_DBGMG_SCLIRQclient(enClient,1),
            "[DRVSCLDMA]%d ISR actN wodb ignore DMA off:%hhx flag:%hx@:%lu\n"
        ,enClient,pgstScldmaInfo->bDMAOnOff[enClient],pgstScldmaInfo->bDmaflag[(enClient)],((u32)DrvSclOsGetSystemTimeStamp()));
        return 0;
    }
    //sclprintf("ACTIVE_N:%lu\n",u32Time);
}
static void _DrvSclIrqSWRingModeActive(DrvSclDmaClientType_e enclient)
{
    u32 u32Time;
    u16  u16RealIdx;
    u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
    if(_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_BLANKING))
    {
        u16RealIdx = 0;
    }
    else
    {
        u16RealIdx = DrvSclDmaGetActiveBufferIdx(enclient);
    }
    if(_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_ACTIVE))
    {
        _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIGNORE);
        SCL_DBGERR("[DRVSCLIRQ]!!!!!!!!!!!!double active\n");
    }
    else
    {
        _SetANDGetFlagType(enclient,E_DRV_SCLDMA_FLAG_ACTIVE,(~(E_DRV_SCLDMA_FLAG_FRMDONE|E_DRV_SCLDMA_FLAG_FRMIGNORE)));
    }
    _SetANDGetIdxType(enclient,u16RealIdx,E_DRV_SCLDMA_ACTIVE_BUFFER_OMX);
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&Get_DBGMG_SCLIRQclient(enclient,0), "[DRVSCLDMA]%d ISR SWRING flag:%hx ACT:%hhd@:%lu\n"
    ,enclient,pgstScldmaInfo->bDmaflag[enclient],u16RealIdx,u32Time);
}
static bool _DrvSclIrqSWRingModeActive_N(DrvSclDmaClientType_e enclient)
{
    DrvSclDmaFrameBufferConfig_t stTarget;
    u64 u64Time;
    bool Ret = 1;
    DrvSclOsMemset(&stTarget,0,sizeof(DrvSclDmaFrameBufferConfig_t));
    u64Time = ((u64)DrvSclOsGetSystemTimeStamp());
    if(u64Time){}

    if(!_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIN)|| _IsFlagType(enclient,E_DRV_SCLDMA_FLAG_DROP)
        || _IsFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIGNORE))
    {
        Ret = 0;
        _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_ACTIVE);
        if( _IsFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIGNORE))
        {
            DrvSclDmaSetDMAIgnoreCount(enclient,1);
            DrvSclDmaSetISRHandlerDmaOff(enclient,0);
            _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_EVERDMAON);
        }
        else
        {
            _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIGNORE);
        }
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&Get_DBGMG_SCLIRQclient(enclient,1),
            "[DRVSCLDMA]%d ISR SWRING ignore flag:%hx idx:%hhd\n"
        ,enclient,pgstScldmaInfo->bDmaflag[enclient],pgstScldmaInfo->bDMAidx[enclient]);
    }
    else
    {
        _SetANDGetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMDONE,(~E_DRV_SCLDMA_FLAG_ACTIVE));
        _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_ACTIVE);
        stTarget.u8FrameAddrIdx = _GetIdxType(enclient,E_DRV_SCLDMA_ACTIVE_BUFFER_SCL);
        stTarget.u32FrameAddr   = pgstScldmaInfo->u32Base_Y[enclient][stTarget.u8FrameAddrIdx];
        stTarget.u64FRMDoneTime = DrvSclDmaGetSclFrameDoneTime(enclient,0);
        stTarget.u16FrameWidth   = pgstScldmaInfo->u16FrameWidth[enclient];
        stTarget.u16FrameHeight  = pgstScldmaInfo->u16FrameHeight[enclient];
        if(DrvSclDmaMakeSureNextActiveId(enclient))
        {
            if(DrvSclDmaBufferEnQueue(enclient,&stTarget))
            {
                DrvSclDmaChangeBufferIdx(enclient);
            }
        }
        if(_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_BLANKING))
        {
            _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_BLANKING);
        }
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&Get_DBGMG_SCLIRQclient(enclient,1), "[DRVSCLDMA]%d ISR SWRING actN flag:%hx idx:%hhd\n"
        ,enclient,pgstScldmaInfo->bDmaflag[enclient],pgstScldmaInfo->bDMAidx[enclient]);
    }
    return Ret;
}
static void _DrvSclIrqSingleModeActiveWithoutDoublebuffer(DrvSclDmaClientType_e enclient)
{
    u32 u32Time;
    if(_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_ACTIVE))
    {
        _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIGNORE);
        SCL_DBGERR("[DRVSCLIRQ]!!!!!!! double active\n");
    }
    else
    {
        _SetANDGetFlagType(enclient,E_DRV_SCLDMA_FLAG_ACTIVE,(~(E_DRV_SCLDMA_FLAG_FRMDONE|E_DRV_SCLDMA_FLAG_FRMIGNORE)));
    }
    u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_SC1SINGLE, "[DRVSCLIRQ]singlewodb act:%lu\n",u32Time);
}
static bool _DrvSclIrqSingleModeActive_NWithoutDoublebuffer(DrvSclDmaClientType_e enclient)
{
    u32 u32Time;
    bool Ret = 1;
    u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
    _SetANDGetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMDONE,(~E_DRV_SCLDMA_FLAG_ACTIVE));
    if(enclient< E_DRV_SCLDMA_3_FRM_R)
    {
        if(!_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIN) || _IsFlagType(enclient,E_DRV_SCLDMA_FLAG_DROP)
            || _IsFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIGNORE))
        {
            Ret = 0;
            if( _IsFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIGNORE))
            {
                DrvSclDmaSetDMAIgnoreCount(enclient,1);
                DrvSclDmaSetISRHandlerDmaOff(enclient,0);
                _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_EVERDMAON);
            }
            else
            {
                _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIGNORE);
            }
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_SC1SINGLE, "[DRVSCLIRQ]singlewodb ignore actN:%lu\n",u32Time);
        }
        else
        {
            _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_DMAOFF);
            //Drv_SCLDMA_SetISRHandlerDMAOff(enclient,0);
            _DrvSclIrqSetSingleModeDmaInfoNoLock(enclient,(SCLIRQ_ISTEVENT_BASE << enclient));
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_SC1SINGLE, "[DRVSCLIRQ]singlewodb actN:%lu\n",u32Time);
        }
    }
    return Ret;
}
static void _DrvSclIrqSC3SingleMode_Active(void)
{
    u32 u32Time;
    u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_SC3SINGLE, "[DRVSCLIRQ]SC3 single act @:%lu\n",u32Time);
    _SetANDGetFlagType(E_DRV_SCLDMA_3_FRM_W,E_DRV_SCLDMA_FLAG_ACTIVE,(~E_DRV_SCLDMA_FLAG_FRMDONE));
    _SetANDGetFlagType(E_DRV_SCLDMA_3_FRM_R,E_DRV_SCLDMA_FLAG_ACTIVE,(~E_DRV_SCLDMA_FLAG_FRMDONE));
}
static void _DrvSclIrqSC3SingleMode_Active_N(void)
{
    u32 u32Time;
    u32 u32Events = 0;
    u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
    DrvSclDmaSetISRHandlerDmaOff(E_DRV_SCLDMA_3_FRM_W,0);
    u32Events = DrvSclOsGetEvent(_s32M2MEventId);//get now ir
    _SetANDGetFlagType(E_DRV_SCLDMA_3_FRM_W,(E_DRV_SCLDMA_FLAG_DMAOFF|E_DRV_SCLDMA_FLAG_FRMDONE),
        ~(E_DRV_SCLDMA_FLAG_EVERDMAON|E_DRV_SCLDMA_FLAG_ACTIVE));
    _SetANDGetFlagType(E_DRV_SCLDMA_3_FRM_R,(E_DRV_SCLDMA_FLAG_DMAOFF|E_DRV_SCLDMA_FLAG_FRMDONE),
    ~(E_DRV_SCLDMA_FLAG_EVERDMAON|E_DRV_SCLDMA_FLAG_ACTIVE));
    _DrvSclIrqSetSingleModeDmaInfoNoLock(E_DRV_SCLDMA_3_FRM_W,(SCLIRQ_ISTEVENT_BASE << E_DRV_SCLDMA_3_FRM_W));
    _DrvSclIrqSetSingleModeDmaInfoNoLock(E_DRV_SCLDMA_3_FRM_R,(SCLIRQ_ISTEVENT_BASE << E_DRV_SCLDMA_3_FRM_R));
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_SC3SINGLE, "[DRVSCLIRQ]SC3 single actN @:%lu\n",u32Time);
    DrvSclOsClearEvent(_s32M2MEventId,u32Events &(E_DRV_SCLIRQ_SC3EVENT_ISTCLEAR));
    DrvSclOsSetEvent(_s32M2MEventId, (E_DRV_SCLIRQ_SC3EVENT_DONE |E_DRV_SCLIRQ_SC3EVENT_HVSPST));
}
static void _DrvSclIrqSetFrameInFlag(DrvSclDmaClientType_e enclient)
{
    if(_IsFrmDrop())
    {
        _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_DROP);
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_ELSE,
            "[DRVSCLIRQ]%dIsFrmDrop @:%lu\n",enclient,((u32)DrvSclOsGetSystemTimeStamp()));
    }
    else
    {
        _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_DROP);
    }
    if(_IsFrmIN())
    {
        _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIN);
    }
    else if(!DrvSclOsGetSclFrameDelay())
    {
        u16 u16Crop;
        u16 u16Crop2;
        u16Crop = HalSclIrqGetRegVal(REG_SCL2_50_L);
        u16Crop &= 0x1;
        u16Crop2 = HalSclIrqGetRegVal(REG_SCL2_58_L);
        u16Crop2 &= 0x1;
        if((u16Crop || u16Crop2)&&_IsFrmIN_Last())
        {
            _SetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIN);
        }
        else
        {
            _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIN);
        }
    }
    else
    {
        _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_FRMIN);
    }
}
void _DrvSclIrqFrmDoneEventHandler(void)
{
    DrvSclIrqTxEventType_e enEvent = gsclirqstate;
    if(enEvent)
    {
        enEvent |= (E_DRV_SCLIRQ_EVENT_IRQ|E_DRV_SCLIRQ_EVENT_HVSPST);
        DrvSclOsSetEvent(_s32FRMENDEventId, enEvent);
        gsclirqstate &= (~enEvent);
        gsclirqstate |= E_DRV_SCLIRQ_EVENT_SET;
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVEL_ELSE),
            "%s @:%lu\n",__FUNCTION__,gu32Time);
    }
}
void _DrvSclIrqCmdqBlankingHandler(void)
{
    if(DrvSclOsGetEvent(_s32SYNCEventId)&E_DRV_SCLIRQ_EVENT_CMDQFIRE )
    {
        DrvSclCmdqBeTrigger(E_DRV_SCLCMDQ_TYPE_IP0,1);
        DrvSclOsClearEvent(DrvSclIrqGetIrqSYNCEventID(),(E_DRV_SCLIRQ_EVENT_CMDQFIRE));
        SCL_DBG(SCL_DBG_LV_DRVCMDQ()&(EN_DBGMG_CMDQEVEL_ISR),
            "%s @:%lu\n",__FUNCTION__,gu32Time);
    }
}
void _DrvSclIrqSetFrmInWhenDMADoneEarlyIsp(bool bEn)
{
    if(bEn)
    {
        _SetFlagType(E_DRV_SCLDMA_1_FRM_W,E_DRV_SCLDMA_FLAG_FRMIN);
        _SetFlagType(E_DRV_SCLDMA_1_SNP_W,E_DRV_SCLDMA_FLAG_FRMIN);
        _SetFlagType(E_DRV_SCLDMA_2_FRM_W,E_DRV_SCLDMA_FLAG_FRMIN);
        _SetFlagType(E_DRV_SCLDMA_2_FRM2_W,E_DRV_SCLDMA_FLAG_FRMIN);
    }
    else
    {
        _ReSetFlagType(E_DRV_SCLDMA_1_FRM_W,E_DRV_SCLDMA_FLAG_FRMIN);
        _ReSetFlagType(E_DRV_SCLDMA_1_SNP_W,E_DRV_SCLDMA_FLAG_FRMIN);
        _ReSetFlagType(E_DRV_SCLDMA_2_FRM_W,E_DRV_SCLDMA_FLAG_FRMIN);
        _ReSetFlagType(E_DRV_SCLDMA_2_FRM2_W,E_DRV_SCLDMA_FLAG_FRMIN);
    }
}
void _Drv_SCLIRQ_FrmEndhandler(void)
{
    u8 idx;
    u8 loop = 0;
    if(!gbEachDMAEnable)
    {
        gbEachDMAEnable = 1;
    }
    else if(gsclirqstate)
    {
    }
    else
    {
        for(idx = 0;idx<E_DRV_SCLDMA_CLIENT_NUM;idx++)
        {
            if(pgstScldmaInfo->bDMAOnOff[idx])
            {
                loop = 1;
                break;
            }
        }
        if(!loop)
        {
            gbEachDMAEnable = 0;
        }
    }
    DrvSclOsSetEvent(_s32SYNCEventId, E_DRV_SCLIRQ_EVENT_FRMENDSYNC);
    DrvSclOsClearEvent(DrvSclIrqGetIrqSYNCEventID(),(E_DRV_SCLIRQ_EVENT_LDCSYNC));
    //TODO:not open this time
    if((!gbDMADoneEarlyISP)||(SCL_DELAYFRAME) || (gstGlobalSclIrqSet->gbPtgenMode))
    {
        _DrvSclIrqFrmDoneEventHandler();
    }
}
void _DrvSclIrqSetDmaOnOffWithoutDoubleBuffer(DrvSclDmaClientType_e enclient)
{
    if((_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_DMAOFF))&&_Is_SCLDMA_SingleMode(enclient))
    {
        _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_DMAOFF);

        DrvSclDmaSetISRHandlerDmaOff(enclient,0);
        pgstScldmaInfo->bDMAOnOff[enclient] = 0;
    }
    else if((_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_EVERDMAON))&&
         (_Is_SCLDMA_SingleMode(enclient)||_Is_SCLDMA_SWRingMode(enclient)))
    {
        _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_EVERDMAON);
        DrvSclDmaSetISRHandlerDmaOff(enclient,1);
    }
    if ((_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_NEXT_OFF)))
    {
        _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_NEXT_OFF);
        DrvSclDmaSetISRHandlerDmaOff(enclient,0);
        pgstScldmaInfo->bDMAOnOff[enclient] = 0;
    }
    if ((_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_NEXT_ON))&&(!_IsFlagType(enclient,E_DRV_SCLDMA_FLAG_DMAFORCEOFF)))
    {
        _ReSetFlagType(enclient,E_DRV_SCLDMA_FLAG_NEXT_ON);
        DrvSclDmaSetISRHandlerDmaOff(enclient,1);
        pgstScldmaInfo->bDMAOnOff[enclient] = 1;
    }
}
void _DrvSclIrqSetDmaOnOffWithoutDoubleBufferHandler(void)
{
    _DrvSclIrqSetDmaOnOffWithoutDoubleBuffer(E_DRV_SCLDMA_1_FRM_W);
    _DrvSclIrqSetDmaOnOffWithoutDoubleBuffer(E_DRV_SCLDMA_1_SNP_W);
    _DrvSclIrqSetDmaOnOffWithoutDoubleBuffer(E_DRV_SCLDMA_2_FRM_W);
    //for HW bug
    _DrvSclIrqSetDmaOnOffWithoutDoubleBuffer(E_DRV_SCLDMA_2_FRM2_W);
}
void _DrvSclIrqReduceDropFrameCount(void)
{
    if(gstDropFrameCount)
    {
        gstDropFrameCount--;
    }
}
void _DrvSclIrqSetDMADoneEarlyISP(bool bDMADoneEarlyISP)
{
    // LDC patch open,crop open,
    gbDMADoneEarlyISP  = bDMADoneEarlyISP;
}
static void _DrvSclIrqSetRealTimeFrmEndHandler(u32 u32Time)
{
    _DrvSclIrqSetIsBlankingRegion(1);
    if(DrvSclOsGetEvent(_s32SYNCEventId)&E_DRV_SCLIRQ_EVENT_ISPFRMEND)
    {
        _DrvSclIrqSetDMADoneEarlyISP(0);
    }
    else
    {
        _DrvSclIrqSetDMADoneEarlyISP(1);
    }

    _DrvSclIrqSetDmaOnOffWithoutDoubleBufferHandler();
    if((!gbDMADoneEarlyISP) || (gstGlobalSclIrqSet->gbPtgenMode))
    {
        _DrvSclIrqCmdqBlankingHandler();
    }
}
static void _DrvSclIrqSetFrmEndInterruptStatus(u32 u32Time)
{
    DrvSclDmaSetIspFrameCount();
    _Drv_SCLIRQ_FrmEndhandler();
    _DrvSclIrqReduceDropFrameCount();
    _DrvSclIrqSclFrameEndConnectToIsp();
}

void _DrvSclIrqHandlerFrmIn(bool bEn)
{
    if(!gstGlobalSclIrqSet->gbPtgenMode)
    {
        gbFRMInFlag = (bool)((gbFRMInFlag)|bEn);
    }
}
void _DrvSclIrqHandlerFrmInShift(void)
{
    if(!gstGlobalSclIrqSet->gbPtgenMode)
    {
        gbFRMInFlag = (bool)((gbFRMInFlag << 1));
    }
}
void _DrvSclIrqIsAffFullContinually(bool bAffFull)
{
    static u8 u8framecount = 0;
    DrvSclIrqInterruptEnable(SCLIRQ_AFF_FULL);
    if(!(bAffFull))
    {
        u8framecount = 0;
    }
    else
    {
        u8framecount++;
    }
    if((u8framecount)>2)
    {
        _DrvSclIrqSetDazaQueueWork(E_DRV_SCLIRQ_DAZA_BRESETFCLK);
        if(gstDropFrameCount <= 1)
        {
            gstDropFrameCount = 1;
        }
        u8framecount = 0;
    }
}
void _DrvSclIrqAffFullHandler(bool bAffFull )
{
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVEL_ELSE),
        "SC AFF FULL %lu @:%lu\n",gstSCInts.u32AffCount,gu32Time);
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVEL_ELSE),
        "CROP H:%hd V:%hd \n",HalSclIrqGetRegVal(REG_SCL2_70_L),HalSclIrqGetRegVal(REG_SCL2_71_L));
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVEL_ELSE),
        "CROP2 H:%hd V:%hd \n",HalSclIrqGetRegVal(REG_SCL2_72_L),HalSclIrqGetRegVal(REG_SCL2_73_L));

}
void _DrvSclIrqFrameInEndHandler(void)
{
    if(DrvSclOsGetEvent(_s32SYNCEventId)&E_DRV_SCLIRQ_EVENT_FRMENDSYNC)
    {
        _DrvSclIrqSetDMADoneEarlyISP(1);
    }
    else
    {
        _DrvSclIrqSetDMADoneEarlyISP(0);
    }
    DrvSclDmaSetIspFrameCount();
    _DrvSclIrqHandlerFrmIn(1);
    if(gbBypassDNR)
    {
        HalSclIrqSetDNRBypass(0);
        gbBypassDNR = 0;
    }
    if(gbDMADoneEarlyISP)
    {
        if(!SCL_DELAYFRAME)
        {
            _DrvSclIrqSetFrmInWhenDMADoneEarlyIsp(1);
            //TODO:not open this time
            _DrvSclIrqFrmDoneEventHandler();
        }
        _DrvSclIrqCmdqBlankingHandler();
    }
    DrvSclOsSetEvent(_s32SYNCEventId, E_DRV_SCLIRQ_EVENT_ISPFRMEND);
}
void _DrvSclIrqIDCLKVsyncInRealTimeHandler(void)
{
    if(gbEachDMAEnable)
    {
        _DrvSclIrqSetIsBlankingRegion(0);
    }
    //disable dnr for ISP not perfect
    if(!_IsFrmIN_Vsync())
    {
        HalSclIrqSetDNRBypass(1);
        gbBypassDNR = 1;
    }
}
void _DrvSclIrqLDCVsyncInRealTimeHandler(void)
{
    u8 idx = 0;
    u8 loop = 0;
    if(gbEachDMAEnable)
    {
        if(!(DrvSclOsGetEvent(DrvSclIrqGetIrqSYNCEventID())&(E_DRV_SCLIRQ_EVENT_FRMENDSYNC)))
        {
            //no frame done
            for(idx = 0;idx<E_DRV_SCLDMA_CLIENT_NUM;idx++)
            {
                if(pgstScldmaInfo->bDMAOnOff[idx])
                {
                    loop = 1;
                    break;
                }
            }
            if(!loop)
            {
                gbEachDMAEnable = 0;
                _DrvSclIrqSetIsBlankingRegion(1);
            }
        }
    }
    else
    {
        // for issue reset rstz_ctrl_clk_miu_sc_dnr cause to overwrite 0x0
        //HalSclIrqSetReg(REG_SCL_DNR1_24_L, 0, BIT6);
        HalSclIrqSetReg(REG_SCL_DNR1_60_L, 0, BIT0);
    }
}
void _DrvSclIrqLDCVsyncInHandler(void)
{
    _DrvSclIrqHandlerFrmInShift();
    DrvSclOsSetEvent(DrvSclIrqGetIrqSYNCEventID(), E_DRV_SCLIRQ_EVENT_LDCSYNC);
    DrvSclOsClearEvent(DrvSclIrqGetIrqSYNCEventID(),(E_DRV_SCLIRQ_EVENT_FRMENDSYNC|E_DRV_SCLIRQ_EVENT_ISPFRMEND));
    if(gsclirqstate &E_DRV_SCLIRQ_EVENT_SET)
    {
        gsclirqstate = E_DRV_SCLIRQ_EVENT_CLEAR;
    }
    else if(gsclirqstate)
    {
        gstSCInts.u16ErrorCount++;
        if(gbDMADoneEarlyISP)
        {
            if(!SCL_DELAYFRAME)
            {
                SCL_DBGERR("[LDC]:ISP_IN lost:%lu\n",gstSCInts.u16ErrorCount);
                _DrvSclIrqSetFrmInWhenDMADoneEarlyIsp(0);
                //TODO:not open this time
                _DrvSclIrqFrmDoneEventHandler();
                if(gsclirqstate &E_DRV_SCLIRQ_EVENT_SET)
                {
                    gsclirqstate = E_DRV_SCLIRQ_EVENT_CLEAR;
                }
            }
        }
    }
    _DrvSclIrqSclFrameStartConnectToIsp();
}


void _DrvSclIrqDmaActiveHandler(DrvSclDmaClientType_e enclient)
{
    if(_Is_SCLDMA_RingMode(enclient))
    {
        _DrvSclIrqRingModeActiveWithoutDoublebuffer(enclient);
    }
    else if(_Is_SCLDMA_SWRingMode(enclient))
    {
        _DrvSclIrqSWRingModeActive(enclient);
    }
    else if(enclient ==E_DRV_SCLDMA_3_FRM_W &&_Is_SCLDMA_SingleMode(E_DRV_SCLDMA_3_FRM_W))
    {
        _DrvSclIrqSC3SingleMode_Active();
    }
    else if(_Is_SCLDMA_SingleMode(enclient))
    {
        _DrvSclIrqSingleModeActiveWithoutDoublebuffer(enclient);
    }
    DrvSclDmaSetSclFrameDoneTime(enclient ,DrvSclOsGetSystemTimeStamp());
}
bool _DrvSclIrqDmaActiveNHandler(DrvSclDmaClientType_e enclient)
{
    bool Ret=1;
    if(enclient < E_DRV_SCLDMA_3_FRM_R)
    {
        _DrvSclIrqSetFrameInFlag(enclient);
    }
    if(_Is_SCLDMA_RingMode(enclient))
    {
        Ret = _DrvSclIrqRingModeActive_NWithoutDoublebuffer(enclient);
    }
    else if(_Is_SCLDMA_SWRingMode(enclient))
    {
        _DrvSclIrqSWRingModeActive_N(enclient);
    }
    else if(enclient ==E_DRV_SCLDMA_3_FRM_W && _Is_SCLDMA_SingleMode(E_DRV_SCLDMA_3_FRM_W))
    {
        _DrvSclIrqSC3SingleMode_Active_N();
    }
    else if(_Is_SCLDMA_SingleMode(enclient))
    {
        Ret = _DrvSclIrqSingleModeActive_NWithoutDoublebuffer(enclient);
    }
    return Ret;
}
void _DrvSclIrqSetHvspHSram(DrvSclHvspIdType_e enHVSP_ID,u32 u32EventFlag)
{
    //u32 u32Events;
    if(DrvSclIrqGetIsBlankingRegion())
    {
        //DrvSclHvspSetSCIQHSRAM(enHVSP_ID);
        DrvSclOsClearEventIRQ(_s32SYNCEventId,(u32EventFlag));
    }
    else
    {
        SCL_DBG(SCL_DBG_LV_DRVCMDQ()&EN_DBGMG_CMDQEVEL_ISR, "[DRVSCLIRQ]%s:SRAMSETH wating\n",__FUNCTION__);
        //DrvSclHvspSetSCIQHSRAM(enHVSP_ID);
        DrvSclOsClearEventIRQ(_s32SYNCEventId,(u32EventFlag));
    }
}
void _DrvSclIrqSetHvspVSram(DrvSclHvspIdType_e enHVSP_ID,u32 u32EventFlag)
{
    //u32 u32Events;
    if(DrvSclIrqGetIsBlankingRegion())
    {
        //DrvSclHvspSetSCIQVSRAM(enHVSP_ID);
        DrvSclOsClearEventIRQ(_s32SYNCEventId,(u32EventFlag));
    }
    else
    {
        SCL_DBG(SCL_DBG_LV_DRVCMDQ()&EN_DBGMG_CMDQEVEL_ISR, "[DRVSCLIRQ]%s:SRAMSETV wating\n",__FUNCTION__);
        DrvSclOsClearEventIRQ(_s32SYNCEventId,(u32EventFlag));
    }
}
static int SCLIRQ_CMDQIST(void *arg)
{
    u32 u32Events;
    u32Events = DrvSclOsGetEvent(_s32SYNCEventId);
    SCL_DBG(SCL_DBG_LV_DRVCMDQ()&EN_DBGMG_CMDQEVEL_ISR, "[DRVSCLIRQ]%s:going\n",__FUNCTION__);
   return 0;

}

void SCLIRQ_DazaThread(void)
{
    while(1)
    {
        if(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP) && DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA))
        {
            break;
        }
        else
        {
            DrvSclOsTaskWait((u32)(0x1 <<(gstThreadCfg.s32Taskid[E_DRV_SCLIRQ_SCTASKID_DAZA] &0xFFFF)));
        }

    }
    while(1)
    {
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_ELSE, "[DRVSCLIRQ]%s:going \n",__FUNCTION__);
        if(DrvSclOsGetQueueExist((s32)gstThreadCfg.s32HandlerId[E_DRV_SCLIRQ_SCTASK_WORKQUEUEDAZA]) == TRUE)
        {
            if(DrvSclOsQueueWait(gstThreadCfg.s32HandlerId[E_DRV_SCLIRQ_SCTASK_WORKQUEUEDAZA]))
            {
                SCLIRQ_DazaIST();
            }
        }
        if(DrvSclOsGetQueueExist((s32)gstThreadCfg.s32HandlerId[E_DRV_SCLIRQ_SCTASK_WORKQUEUECMDQ]) == TRUE)
        {
            if(DrvSclOsQueueWait(gstThreadCfg.s32HandlerId[E_DRV_SCLIRQ_SCTASK_WORKQUEUECMDQ]))
            {
#if 0
                SCLIRQ_CMDQIST();
#endif
            }
        }
        DrvSclOsTaskWait((u32)(0x1 <<(gstThreadCfg.s32Taskid[E_DRV_SCLIRQ_SCTASKID_DAZA] &0xFFFF)));
    }
}

void _DrvSclIrqIsDMAHangUp(u64 u64Flag)
{
    static bool bVsyncWithVend = 0;
    static bool bLDCEvenOpen = 0;
    static bool bLDCEvenOpenCount = 0;
    if(u64Flag &SCLIRQ_MSK_VSYNC_FCLK_LDC)
    {
        bVsyncWithVend ++;
    }
    else
    {
        bVsyncWithVend = 0;
        bLDCEvenOpenCount++;
    }
    if(bLDCEvenOpen && bLDCEvenOpenCount>100)
    {
        _DrvSclIrqSetDazaQueueWork(E_DRV_SCLIRQ_DAZA_BRESETLDCP);
        bLDCEvenOpen = 0;
        bLDCEvenOpenCount = 0;
    }
    if(bVsyncWithVend>3 && (HalSclIrqGetRegVal(REG_SCL0_03_L)&BIT12))
    {
        _DrvSclIrqSetDazaQueueWork(E_DRV_SCLIRQ_DAZA_BCLOSELDCP);
        bLDCEvenOpen = 1;
        bVsyncWithVend = 0;
        bLDCEvenOpenCount = 0;
    }
}
void _DrvSclIrqSetISPBlankingTime(u32 u32DiffTime)
{
    if(gstSCInts.u32ISPBlankingTime > u32DiffTime && (u32DiffTime > CRITICAL_SECTION_TIMING_MIN))
    {
        //for get most critical
        gstSCInts.u32ISPBlankingTime = u32DiffTime;
    }
    else if(gstSCInts.u32ISPBlankingTime == 0)
    {
        //for get inital
        gstSCInts.u32ISPBlankingTime = u32DiffTime;
    }
    else if (gstSCInts.u32ISPBlankingTime < u32DiffTime)
    {
        if(u32DiffTime - gstSCInts.u32ISPBlankingTime > CHANGE_CRITICAL_SECTION_TIMING_RANGE)
        {
            // for timing change
            gbChangeTimingCount ++;
        }
        else
        {
            gbChangeTimingCount = 0;
        }
        if(gbChangeTimingCount > CHANGE_CRITICAL_SECTION_TIMING_COUNT)
        {
            // for ensure timing change
            gstSCInts.u32ISPBlankingTime = u32DiffTime;
            gbChangeTimingCount = 0;
            SCL_DBGERR("[SCLIRQ]chang time :%lu\n",gstSCInts.u32ISPBlankingTime);
        }
    }
}
void _DrvSclIrqSetDazaQueueWork(DrvSclIrqDazaEventType_e enEvent)
{
    if(!(genDAZAEvent&enEvent))
    {
        genDAZAEvent |= enEvent;
        DrvSclOsQueueWork(1,gstThreadCfg.s32Taskid[E_DRV_SCLIRQ_SCTASKID_DAZA],
            gstThreadCfg.s32HandlerId[E_DRV_SCLIRQ_SCTASK_WORKQUEUEDAZA],0);
    }
}

void SCLIRQ_IST(void)
{
}
bool _Delete_SCLIRQ_IST(void)
{
    gstThreadCfg.flag = 0;
    DrvSclOsFlushWorkQueue(1,gstThreadCfg.s32Taskid[E_DRV_SCLIRQ_SCTASKID_DAZA]);
    DrvSclOsestroyWorkQueueTask(gstThreadCfg.s32Taskid[E_DRV_SCLIRQ_SCTASKID_DAZA]);
    return 0;
}
bool _Create_SCLIRQ_IST(void)
{
    char pName2[] = {"SCLDAZA_THREAD"};
    u8 bRet = 0;
    gstThreadCfg.s32Taskid[E_DRV_SCLIRQ_SCTASKID_DAZA] = DrvSclOsCreateWorkQueueTask(pName2);
    gstThreadCfg.s32HandlerId[E_DRV_SCLIRQ_SCTASK_WORKQUEUECMDQ] = DrvSclOsCreateWorkQueueEvent((void*)SCLIRQ_CMDQIST);
    gstThreadCfg.s32HandlerId[E_DRV_SCLIRQ_SCTASK_WORKQUEUEDAZA] = DrvSclOsCreateWorkQueueEvent((void*)SCLIRQ_DazaIST);
    return bRet;
}
#else
void _DrvSclIrqSetIsBlankingRegion(bool bBk)
{
}
bool DrvSclIrqGetIsBlankingRegion(void)
{
    return 1;
}


#endif //ENABLE_ACTIVEID_ISR

#if I2_DVR
#else

void DrvSclIrqSetDropFrameFromCmdqDone(u8 u8Count)
{
    //u32 u32Events;
    //I3 patch
    DRV_SCLIRQ_MUTEX_LOCK_ISR();
    if(gstDropFrameCount<=u8Count)
    {
        gstDropFrameCount = u8Count;
    }
    DRV_SCLIRQ_MUTEX_UNLOCK_ISR();
}
void DrvSclIrqSetDropFrameFromIsp(u8 u8Count)
{
    SCL_ERR("[SCLIRQ]ISP drop Frame Count:%hhd \n",u8Count);
    DRV_SCLIRQ_MUTEX_LOCK_ISR();
    if(gstDropFrameCount<=u8Count)
    {
        gstDropFrameCount = u8Count;
    }
    DRV_SCLIRQ_MUTEX_UNLOCK_ISR();
}
void DrvSclIrqSetDazaQueueWorkISR(DrvSclIrqDazaEventType_e enEvent)
{
    if(!(genDAZAEvent&enEvent))
    {
        DRV_SCLIRQ_MUTEX_LOCK_ISR();
        genDAZAEvent |= enEvent;
        DRV_SCLIRQ_MUTEX_UNLOCK_ISR();
        DrvSclOsQueueWork(1,gstThreadCfg.s32Taskid[E_DRV_SCLIRQ_SCTASKID_DAZA],
            gstThreadCfg.s32HandlerId[E_DRV_SCLIRQ_SCTASK_WORKQUEUEDAZA],0);
    }
}
#endif
void  _DrvSclIrqDvrisrTest(int eIntNum, void* dev_id)
{
    u64 u64Flag;
    u64Flag = HalSclIrqGetFlag(E_DRV_SCLIRQ_SCTOP_0,0xFFFFFFFFFFFFFFFF);

    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,u64Flag, 1);
    gu32Time = ((u32)DrvSclOsGetSystemTimeStamp());
    /*
    if(u64Flag &SCLIRQ_MSK_SC1_ENG_FRM_END)
    {
        SCL_ERR("SCLIRQ_MSK_SC1_ENG_FRM_END @%lu\n",gu32Time);
    }
    if(u64Flag &SCLIRQ_MSK_SC1_FRM_W_ACTIVE_N)
    {
        SCL_ERR("SCLIRQ_MSK_SC1_FRM_W_ACTIVE_N @%lu\n",gu32Time);
    }
    if(u64Flag &SCLIRQ_MSK_SC2_FRM_W_ACTIVE_N)
    {
        SCL_ERR("SCLIRQ_MSK_SC2_FRM_W_ACTIVE_N @%lu\n",gu32Time);
    }
    if(u64Flag &SCLIRQ_MSK_SC3_DMA_W_ACTIVE_N)
    {
        SCL_ERR("SCLIRQ_MSK_SC3_DMA_W_ACTIVE_N @%lu\n",gu32Time);
    }
    if(u64Flag &SCLIRQ_MSK_SC1_SNP_W_ACTIVE_N)
    {
        SCL_ERR("SCLIRQ_MSK_SC1_SNP_W_ACTIVE_N @%lu\n",gu32Time);
    }
    if(u64Flag &SCLIRQ_MSK_SC2_FRM2_W_ACTIVE_N)
    {
        SCL_ERR("SCLIRQ_MSK_SC2_FRM2_W_ACTIVE_N @%lu\n",gu32Time);
    }
    if(u64Flag &(((u64)1)<<53))
    {
        SCL_ERR("matchv_int2_sc1_frm_w	@%lu line:%hu\n",gu32Time,HalSclDmaGetDMAOutputCount(E_DRV_SCLDMA_1_FRM_W));
    }
    if(u64Flag &(((u64)1)<<59))
    {
        SCL_ERR("matchv_int1_sc1_frm_w@%lu line:%hu \n",gu32Time,HalSclDmaGetDMAOutputCount(E_DRV_SCLDMA_1_FRM_W));
    }
    if(u64Flag &(((u64)1)<<52))
    {
        SCL_ERR("matchv_int2_sc1_snp_w	@%lu line:%hu\n",gu32Time,HalSclDmaGetDMAOutputCount(E_DRV_SCLDMA_1_SNP_W));
    }
    if(u64Flag &(((u64)1)<<58))
    {
        SCL_ERR("matchv_int1_sc1_snp_w @%lu line:%hu \n",gu32Time,HalSclDmaGetDMAOutputCount(E_DRV_SCLDMA_1_SNP_W));
    }
    if(u64Flag &(((u64)1)<<49))
    {
        SCL_ERR("matchv_int2_sc2_frm_w	@%lu line:%hu\n",gu32Time,HalSclDmaGetDMAOutputCount(E_DRV_SCLDMA_2_FRM_W));
    }
    if(u64Flag &(((u64)1)<<55))
    {
        SCL_ERR("matchv_int1_sc2_frm_w @%lu line:%hu \n",gu32Time,HalSclDmaGetDMAOutputCount(E_DRV_SCLDMA_2_FRM_W));
    }
    if(u64Flag &(((u64)1)<<50))
    {
        SCL_ERR("matchv_int2_sc2_frm2_w	@%lu line:%hu\n",gu32Time,HalSclDmaGetDMAOutputCount(E_DRV_SCLDMA_2_FRM2_W));
    }
    if(u64Flag &(((u64)1)<<56))
    {
        SCL_ERR("matchv_int1_sc2_frm2_w @%lu line:%hu \n",gu32Time,HalSclDmaGetDMAOutputCount(E_DRV_SCLDMA_2_FRM2_W));
    }*/
    if(u64Flag &SCLIRQ_MSK_SC3_ENG_FRM_END)
    {
        HalSclDmaSetOutputMatchline(E_DRV_SCLDMA_1_FRM_W,100,300);
        HalSclDmaSetOutputMatchline(E_DRV_SCLDMA_1_SNP_W,100,300);
        HalSclDmaSetOutputMatchline(E_DRV_SCLDMA_2_FRM_W,100,300);
        HalSclDmaSetOutputMatchline(E_DRV_SCLDMA_2_FRM2_W,100,300);
        DrvSclOsSetEvent(_s32FRMENDEventId, E_DRV_SCLIRQ_EVENT_FRMEND);
        //SCL_ERR("SCLIRQ_MSK_SC3_ENG_FRM_END @%lu\n",gu32Time);
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVEL_ELSE),"[FRMEND]flag:%llx@%lu\n",u64Flag,gu32Time);
    }
    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,u64Flag, 0);
}

bool DrvSclIrqInitVariable(void)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclIrqSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    gsclirqstate        = 0;
    gscl3irqstate       = 0;
    _bSCLIRQ_Suspend    = 0;
    gbFRMInFlag         = 0;
    gstGlobalSclIrqSet->gbPtgenMode         = 0;
    gbDMADoneEarlyISP   = 0;
    genDAZAEvent        = 0;
    gbEachDMAEnable     = 0;
    gbBypassDNR = 0;
    gstDropFrameCount = 0;
    gu32ActTime[E_DRV_SCLIRQ_CLK_FRMDONE] = 0;
    DrvSclOsMemset(gu32ProcessDiffTime,0x0,sizeof(u32)*E_DRV_SCLIRQ_Process_Hist_MAX);
    DrvSclOsMemset(&gstSCInts,0x0,sizeof(DrvSclIrqScIntsType_t));
    _DrvSclIrqSetIsBlankingRegion(1);

    DrvSclOsClearEventIRQ(_s32SYNCEventId,0xFFFFFFFF);
    DrvSclOsClearEventIRQ(_s32FRMENDEventId,0xFFFFFFFF);
    DrvSclOsClearEventIRQ(_s32M2MEventId,0xFFFFFFFF);
    return TRUE;
}
void DrvSclIrqSetPTGenStatus(bool bPTGen)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclIrqSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    DRV_SCLIRQ_MUTEX_LOCK();
    gstGlobalSclIrqSet->gbPtgenMode  = bPTGen;
    if(bPTGen)
    {
        gbFRMInFlag = 0xF;
    }
    DRV_SCLIRQ_MUTEX_UNLOCK();
}
DrvSclIrqScIntsType_t* DrvSclIrqGetSclInts(void)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclIrqSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    return &gstSCInts;
}

bool DrvSclIrqInit(DrvSclIrqInitConfig_t *pCfg)
{
    char mutx_word[] = {"_SCLIRQ_Mutex"};

    u8 i;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclIrqSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    if(_SCLIRQ_Mutex != -1)
    {
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_NORMAL, "[DRVSCLIRQ]%s(%d)::Already Done\n", __FUNCTION__, __LINE__);
        return TRUE;
    }

    if(DrvSclOsInit() == FALSE)
    {
        DRV_SCLIRQ_ERR(sclprintf("[DRVSCLIRQ]%s(%d)::DrvSclOsInit Fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }

    _SCLIRQ_Mutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, mutx_word, SCLOS_PROCESS_SHARED);
    _pSCLIRQ_SCLDMA_Mutex = DrvSclDmaGetDmaandIRQCommonMutex();
    if (_SCLIRQ_Mutex == -1)
    {
        DRV_SCLIRQ_ERR(sclprintf("[DRVSCLIRQ]%s(%d)::Create Mutex Flag Fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }
    HalSclIrqSetUtility();
    //HalSclIrqSetRiuBase(pCfg->u32RiuBase);

    // disable all interrupt first
    for(i=0; i<SCLIRQ_NUM; i++)
    {
        DrvSclIrqDisable(i);
        HalSclIrqSetMask(E_DRV_SCLIRQ_SCTOP_1, ((u64)1)<<i, 1);
        HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_1, ((u64)1)<<i, 1);
        HalSclIrqSetMask(E_DRV_SCLIRQ_SCTOP_2, ((u64)1)<<i, 1);
        HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_2, ((u64)1)<<i, 1);
    }
    for(i=0; i<16; i++)
    {
        DrvSclIrqCmdqDisable(i);
        HalSclIrqCmdqSetMask(E_DRV_SCLIRQ_SCTOP_CMDQ_1, ((u64)1)<<i, 1);
        HalSclIrqCmdqSetClear(E_DRV_SCLIRQ_SCTOP_CMDQ_1, ((u64)1)<<i, 1);
    }

    DrvSclIrqInitVariable();
    DrvSclOsMemset(&gstThreadCfg,0x0,sizeof(DrvSclDmaThreadConfig_t));

#if I2_DVR_TEST_ISR
    _s32FRMENDEventId   = DrvSclOsCreateEventGroup("FRMEND_Event");
    SCL_ERR("[DRVSCLIRQ]_s32FRMENDEventId:%ld\n",_s32FRMENDEventId);
    if(DrvSclOsAttachInterrupt(pCfg->u32IRQNUM, (InterruptCb)_DrvSclIrqDvrisrTest ,IRQF_DISABLED, "SCLINTR"))
    {
        DRV_SCLIRQ_ERR(sclprintf("[DRVSCLIRQ]%s(%d):: Request IRQ Fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }
    DrvSclOsDisableInterrupt(pCfg->u32IRQNUM);
    DrvSclOsEnableInterrupt(pCfg->u32IRQNUM);
#endif
#if (ENABLE_ACTIVEID_ISR)
    _s32FRMENDEventId   = DrvSclOsCreateEventGroup("FRMEND_Event");
    _s32SYNCEventId     = DrvSclOsCreateEventGroup("SYNC_Event");
    _s32M2MEventId      = DrvSclOsCreateEventGroup("SC3_Event");
    if(DrvSclOsAttachInterrupt(pCfg->u32IRQNUM, (InterruptCb)_DrvSclIrqisr ,0, "SCLINTR"))
    {
        DRV_SCLIRQ_ERR(sclprintf("[DRVSCLIRQ]%s(%d):: Request IRQ Fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }
    SCL_ERR("%s %d\n", __FUNCTION__, __LINE__);
    DrvSclOsDisableInterrupt(pCfg->u32IRQNUM);
    DrvSclOsEnableInterrupt(pCfg->u32IRQNUM);
    _Create_SCLIRQ_IST();
    SCL_ERR("%s %d\n", __FUNCTION__, __LINE__);
#endif
#if (ENABLE_CMDQ_ISR)
    DrvSclCmdqInitRIUBase(pCfg->u32RiuBase);
    if(DrvSclOsAttachInterrupt(pCfg->u32CMDQIRQNUM, (InterruptCb)_DrvSclIrqCmdqIsr , 0, "CMDQINTR"))
    {
        DRV_SCLIRQ_ERR(sclprintf("[DRVSCLIRQ]%s(%d):: Request IRQ Fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }
    DrvSclOsDisableInterrupt(pCfg->u32CMDQIRQNUM);
    DrvSclOsEnableInterrupt(pCfg->u32CMDQIRQNUM);
    DrvSclCmdqSetISRStatus(1);
#endif


    return TRUE;
}
bool DrvSclIrqSuspend(DrvSclIrqSuspendResumeConfig_t *pCfg)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclIrqSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_NORMAL, "%s(%d), bSuspend=%d\n", __FUNCTION__, __LINE__, _bSCLIRQ_Suspend);
    _bSCLIRQ_Suspend = 1;
    _DrvSclIrqSetIsBlankingRegion(1);
    return TRUE;
}

bool DrvSclIrqResume(DrvSclIrqSuspendResumeConfig_t *pCfg)
{
    u16 i;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclIrqSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_NORMAL, "%s(%d), bResume=%d\n", __FUNCTION__, __LINE__, _bSCLIRQ_Suspend);

    if(_bSCLIRQ_Suspend == 1)
    {
        // disable all interrupt first
        for(i=0; i<SCLIRQ_NUM; i++)
        {
            DrvSclIrqDisable(i);
            HalSclIrqSetMask(E_DRV_SCLIRQ_SCTOP_1,((u64)1)<<i, 1);
            HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_1,((u64)1)<<i, 1);
            HalSclIrqSetMask(E_DRV_SCLIRQ_SCTOP_2,((u64)1)<<i, 1);
            HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_2,((u64)1)<<i, 1);
        }
        _bSCLIRQ_Suspend = 0;
    }

    return TRUE;
}

bool DrvSclIrqExit(void)
{
    bool bRet = TRUE;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclIrqSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    if(_SCLIRQ_Mutex != -1)
    {
        DrvSclOsDeleteMutex(_SCLIRQ_Mutex);
        _SCLIRQ_Mutex = -1;
    }
    else
    {
        return FALSE;
    }

#if ENABLE_ACTIVEID_ISR
    u64 u64IRQ;
    u64IRQ = HalSclIrqGetFlag(E_DRV_SCLIRQ_SCTOP_0,0xFFFFFFFFFF);
    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,u64IRQ, 1);
    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,u64IRQ, 0);
    _Delete_SCLIRQ_IST();
    DrvSclOsDisableInterrupt(DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_SC0));
    DrvSclOsDetachInterrupt(DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_SC0));
#endif
#if (ENABLE_CMDQ_ISR)
    DrvSclOsDisableInterrupt(DrvSclOsGetIrqIDCMDQ(E_DRV_SCLOS_CMDQIRQ_CMDQ0));
    DrvSclOsDetachInterrupt(DrvSclOsGetIrqIDCMDQ(E_DRV_SCLOS_CMDQIRQ_CMDQ0));
    DrvSclCmdqSetISRStatus(0);
#endif
    DrvSclOsExit();
    return bRet;
}

bool DrvSclIrqInterruptEnable(u16 u16IRQ)
{
    bool bRet = TRUE;
    if(u16IRQ < 64)
    {
        HalSclIrqSetMask(E_DRV_SCLIRQ_SCTOP_0,((u64)1<<u16IRQ), 0);
        HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,((u64)1<<u16IRQ), 0);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

bool DrvSclIrqDisable(u16 u16IRQ)
{
    bool bRet = TRUE;

    if(u16IRQ < 64)
    {
        HalSclIrqSetMask(E_DRV_SCLIRQ_SCTOP_0,((u64)1<<u16IRQ), 1);
        HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,((u64)1<<u16IRQ), 1);
        HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,((u64)1<<u16IRQ), 0);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}
bool DrvSclIrqCmdqInterruptEnable(u16 u16IRQ)
{
    bool bRet = TRUE;
    if(u16IRQ)
    {
        HalSclIrqCmdqSetMask(E_DRV_SCLIRQ_SCTOP_CMDQ_0,((u16)1<<u16IRQ), 0);
        HalSclIrqCmdqSetClear(E_DRV_SCLIRQ_SCTOP_CMDQ_0,((u16)1<<u16IRQ), 0);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

bool DrvSclIrqCmdqDisable(u16 u16IRQ)
{
    bool bRet = TRUE;

    if(u16IRQ)
    {
        HalSclIrqCmdqSetMask(E_DRV_SCLIRQ_SCTOP_CMDQ_0,((u16)1<<u16IRQ), 1);
        HalSclIrqCmdqSetClear(E_DRV_SCLIRQ_SCTOP_CMDQ_0,((u16)1<<u16IRQ), 1);
        HalSclIrqCmdqSetClear(E_DRV_SCLIRQ_SCTOP_CMDQ_0,((u16)1<<u16IRQ), 0);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

bool DrvSclIrqGetFlag(u16 u16IRQ, u64 *pFlag)
{
    bool bRet = TRUE;
	u64  n64Mask = 0;

    if(u16IRQ < 48)
    {
        *pFlag = HalSclIrqGetFlag(E_DRV_SCLIRQ_SCTOP_0,((u64)1<<u16IRQ));
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

bool DrvSclIrqSetClear(u16 u16IRQ)
{
    bool bRet = TRUE;
    if(u16IRQ < 48)
    {
        //DRV_SCLIRQ_ERR(sclprintf("[DRVSCLIRQ]: IRQ clear %hx\n", u16IRQ));
        HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,(u64)1<<u16IRQ, 1);
        HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,(u64)1<<u16IRQ, 0);
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

bool DrvSclIrqGetMask(u64 u64IRQ, u64 *pFlag)
{
    bool bRet = TRUE;
    *pFlag = HalSclIrqGetMask(E_DRV_SCLIRQ_SCTOP_0,((u64)1<<u64IRQ));
    return bRet;
}

bool DrvSclIrqSetMask(u64 u64IRQ)
{
    bool bRet = TRUE;
    HalSclIrqSetMask(E_DRV_SCLIRQ_SCTOP_0,u64IRQ,1);
    return bRet;
}

bool DrvSclIrqGetFlags_Msk(u64 u64IrqMsk, u64 *pFlags)
{
    *pFlags = HalSclIrqGetFlag(E_DRV_SCLIRQ_SCTOP_0,u64IrqMsk);
    return TRUE;
}
bool DrvSclIrqSetClear_Msk(u64 u64IrqMsk)
{
    //DRV_SCLIRQ_ERR(sclprintf("[DRVSCLIRQ]: IRQ clear %hx\n", u16IRQ));
    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,u64IrqMsk, 1);
    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,u64IrqMsk, 0);
    return TRUE;
}
void DrvSclIrqSetOperMode(DrvSclIrqEnableMode_e emode)
{
	_geIrqEnableMode = emode;
}
#undef DRV_SCL_IRQ_C
