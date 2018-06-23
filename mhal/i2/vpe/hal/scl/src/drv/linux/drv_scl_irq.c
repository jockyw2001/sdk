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
#define TIMESTAMPBUFCNT 100
#define TIMESTAMPBUFSIZE (sizeof(u64)*TIMESTAMPBUFCNT)
#define IN_TASKLET_ALREADY(u64flag) (((u64)u64flag & TASKLET_ALREADY)!= 0)
#define freerunID               5
#define CRITICAL_SECTION_TIME 80
#define CHANGE_CRITICAL_SECTION_TIMING_RANGE 3000 // (ns)
#define CHANGE_CRITICAL_SECTION_TIMING_COUNT 60 // (ns)
#define CRITICAL_SECTION_TIMING_MIN 500 // (ns)
#define _Is_SCLDMA_RingMode(enClientType)            (pgstScldmaInfo->enBuffMode[(enClientType)] == E_DRV_SCLDMA_BUF_MD_RING)
#define _Is_SCLDMA_SingleMode(enClientType)         (pgstScldmaInfo->enBuffMode[(enClientType)] == E_DRV_SCLDMA_BUF_MD_SINGLE)
#define _Is_SCLDMA_SWRingMode(enClientType)         (pgstScldmaInfo->enBuffMode[(enClientType)] == E_DRV_SCLDMA_BUF_MD_SWRING)
#define DRV_SCLIRQ_MUTEX_LOCK_ISR()        DrvSclOsObtainMutexIrq(*_pSCLIRQ_SCLDMA_Mutex)
#define DRV_SCLIRQ_MUTEX_UNLOCK_ISR()        DrvSclOsReleaseMutexIrq(*_pSCLIRQ_SCLDMA_Mutex)
#define DRV_SCLIRQ_MUTEX_LOCK()             DrvSclOsObtainMutex(_SCLIRQ_Mutex , SCLOS_WAIT_FOREVER)
#define DRV_SCLIRQ_MUTEX_UNLOCK()             DrvSclOsReleaseMutex(_SCLIRQ_Mutex)

#define FPSREFCNT (1000)
#define FPSSEC (1000000)*10
#define _IsHvspEnable() ((HalSclIrqGetRegVal(REG_SCL0_06_L)&0xF00)>>8)
#define _IsPort0Enable() ((HalSclIrqGetRegVal(REG_SCL_DMA0_01_L)&0x8000))
#define _IsPort1Enable() ((HalSclIrqGetRegVal(REG_SCL_DMA0_04_L)&0x8000))
#define _IsPort2Enable() ((HalSclIrqGetRegVal(REG_SCL_DMA1_01_L)&0x8000))
#define _IsPort3Enable() ((!(HalSclIrqGetRegVal(REG_MDWIN2_72_L)&0x1)))
#define _IsPortDone(enType) ((HalSclIrqGetRegVal(REG_SCL1_2E_L)&(ISPORT0_USEDUMMY<<enType)))
#define _IsPort0Done() ((HalSclIrqGetRegVal(REG_SCL1_2E_L)&ISPORT0_USEDUMMY))
#define _IsPort1Done() ((HalSclIrqGetRegVal(REG_SCL1_2E_L)&ISPORT1_USEDUMMY))
#define _IsPort2Done() ((HalSclIrqGetRegVal(REG_SCL1_2E_L)&ISPORT2_USEDUMMY))
#define _IsPort3Done() (((HalSclIrqGetRegVal(REG_SCL1_2E_L)&ISPORT3_USEDUMMY)))
#define _GetPort0ActiveBuf() ((u32)(HalSclIrqGetRegVal(REG_SCL_DMA0_09_L)<<16|HalSclIrqGetRegVal(REG_SCL_DMA0_08_L))*8)
#define _GetPort1ActiveBuf() ((u32)(HalSclIrqGetRegVal(REG_SCL_DMA0_51_L)<<16|HalSclIrqGetRegVal(REG_SCL_DMA0_50_L))*8)
#define _GetPort2ActiveBuf() ((u32)(HalSclIrqGetRegVal(REG_SCL_DMA1_09_L)<<16|HalSclIrqGetRegVal(REG_SCL_DMA1_08_L))*8)
#define _GetPort3ActiveBuf() ((u32)(HalSclIrqGetRegVal(REG_MDWIN1_04_L)<<16|HalSclIrqGetRegVal(REG_MDWIN1_03_L))<<5)
#define _GetProcessActCnt() ((HalSclIrqGetRegVal(ISSCPROCESS_DUMMYReg)&ISSCPROCESS_DUMMY))

#define Print_TimeStamp_Parser(x)           (x==E_DRV_SCLIRQ_TimeStamp_Vsync   ? "Vsync" : \
                                        x==E_DRV_SCLIRQ_TimeStamp_SCDone   ? "SCDone" : \
                                        x==E_DRV_SCLIRQ_TimeStamp_ISPDone   ? "ISPDone" : \
                                        x==E_DRV_SCLIRQ_TimeStamp_Process   ? "Process" : \
                                        x==E_DRV_SCLIRQ_TimeStamp_Late ? "Process Late" : \
                                        x==E_DRV_SCLIRQ_TimeStamp_Drop ? "Process Drop" : \
                                        x==E_DRV_SCLIRQ_TimeStamp_SCDoneWhenLate ? "SCDoneWhenLate" : \
                                        x==E_DRV_SCLIRQ_TimeStamp_ProcessWhenLate ? "ProcessWhenLate" : \
                                                               "UNKNOWN")


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvSclDmaInfoType_t  *pgstScldmaInfo;
DrvSclIrqEnableMode_e        _geIrqEnableMode = E_DRV_SCL_IRQ_OFF;
DrvSclIrqEnableMode_e        _geM2MIrqEnableMode = E_DRV_SCL_IRQ_OFF;
s32  _s32M2MEventId = 0;

//keep or use for isr
s32 _SCLIRQ_Mutex = -1;

s32* _pSCLIRQ_SCLDMA_Mutex = NULL;
bool _bSCLIRQ_Suspend = 0;
//only ISR
bool gbEachDMAEnable;
DrvSclIrqScIntsType_t gstSCInts;
u64 gu64ActTime[E_DRV_SCLIRQ_Clk_MAX];
DrvSclIrqTimeStampConfig_t gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_NUM];
u32 gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_MAX];
u32 gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_MAX];
u8 gbDMADoneEarlyISP;
bool gbPtgen = 0;
u64 gu64FrameDoneInt = 0;
DrvSclIrqScIntsFpsType_t gstFps;
DrvSclIrqScIntsFpsType_t gstProcessFps;

//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
void DrvSclIrqSetSclIsrOpenMode(DrvSclIrqEnableMode_e emode)
{
	_geIrqEnableMode = emode;
}
s32 DrvSclIrqGetIRQM2MEventID(void)
{
    return _s32M2MEventId;
}
void DrvSclIrqSetM2MIsrOpenMode(DrvSclIrqEnableMode_e emode)
{
	_geM2MIrqEnableMode = emode;
	if(emode && !_s32M2MEventId)
	{
        _s32M2MEventId      = DrvSclOsCreateEventGroup("SC3_Event");
        DrvSclOsSetEvent(DrvSclIrqGetIRQM2MEventID(), E_SCLIRQ_M2MEVENT_ACTIVEN);
    }
    else
    {
        if(_s32M2MEventId)
        {
            DrvSclOsDeleteEventGroup(_s32M2MEventId);
            _s32M2MEventId = 0;
        }
    }
}
DrvSclIrqEnableMode_e DrvSclIrqGetSclIsrOpenMode(void)
{
	return _geIrqEnableMode;
}
void DrvSclIrqSetDmaOff(s32 s32Handler,bool bEnImi)
{
    MDrvSclCtxSetLockConfig(s32Handler,E_MDRV_SCL_CTX_ID_SC_ALL);
    HalSclDmaSetDmaOff(gbPtgen,bEnImi);
    MDrvSclCtxSetUnlockConfig(s32Handler,E_MDRV_SCL_CTX_ID_SC_ALL);
}
void _DrvSclIrqSetIsBlankingRegion(bool bBk)
{
	/*write 0x15262f dummy register*/
	if(bBk)
	{
		HalSclIrqSetReg(ISSW_IDLEDUMMYReg, ISSW_IDLEDUMMY, ISSW_IDLEDUMMY);
	}
	else
	{
		HalSclIrqSetReg(ISSW_IDLEDUMMYReg, 0x0, ISSW_IDLEDUMMY);
	}
}
bool DrvSclIrqGetIsBlankingRegion(void)
{
	if(_geIrqEnableMode & E_DRV_SCL_IRQ_MULTIINT_MODE)	{

		if(HalSclIrqGetRegVal(ISSW_IDLEDUMMYReg)&ISSW_IDLEDUMMY)
			return 1;
		else
			return 0;
	}
	else
		return 1;
}
void DrvSclIrqSetCMDQIdle(bool bEn)
{
    HalSclIrqSetReg(ISCMDQ_IDLEDUMMYReg,bEn ? ISCMDQ_IDLEDUMMY : 0,ISCMDQ_IDLEDUMMY);
}
bool DrvSclIrqGetIsCMDQIdle(void)
{
    bool bRet = 0;
    u8 u8ActProcessCnt;
    if(HalSclIrqGetRegVal(ISCMDQ_IDLEDUMMYReg)&ISCMDQ_IDLEDUMMY)
    {
        if(HalSclIrqGetRegVal(ISCMDQ_IDLEDUMMYReg)&ISCMDQ_IDLEDUMMY)
        {
            u8ActProcessCnt = _GetProcessActCnt();
            if((u8ActProcessCnt == gstSCInts.u8ProcessCnt) && (HalSclIrqGetRegVal(ISCMDQ_IDLEDUMMYReg)&ISCMDQ_IDLEDUMMY))
            {
                bRet = 1;
                SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_ELSE),
                "[VPESCL]%s Is CMDQ Idle and Frame Active %hhx @:%llu\n",
                    __FUNCTION__,gstSCInts.u8ProcessCnt,((u64)DrvSclOsGetSystemTimeStamp()));
            }
        }
    }
    return bRet;
}

bool DrvSclIrqInitVariable(void)
{
    _bSCLIRQ_Suspend    = 0;
    gbDMADoneEarlyISP   = 0;
    gbEachDMAEnable     = 0;
    gbPtgen = 0;
    gu64ActTime[E_DRV_SCLIRQ_CLK_FRMDONE] = 0;
    gu64FrameDoneInt = SCLIRQ_MSK_SC3_ENG_FRM_END|SCLIRQ_MSK_SC1_ENG_FRM_END;
    DrvSclOsMemset(&gstSCInts,0x0,sizeof(DrvSclIrqScIntsType_t));
    DrvSclOsMemset(&gstFps,0x0,sizeof(DrvSclIrqScIntsFpsType_t));
    DrvSclOsMemset(&gstProcessFps,0x0,sizeof(DrvSclIrqScIntsFpsType_t));
    DrvSclOsMemset(gu32ProcessDiffTime,0x0,sizeof(u32)*E_DRV_SCLIRQ_Process_Hist_MAX);
    DrvSclOsMemset(gu32ProcessDiffDoneTime,0x0,sizeof(u32)*E_DRV_SCLIRQ_Process_Hist_MAX);
    _DrvSclIrqSetIsBlankingRegion(1);

    return TRUE;
}
void DrvSclIrqSetPTGenStatus(bool bPTGen)
{
    DRV_SCLIRQ_MUTEX_LOCK();
    gbPtgen = bPTGen;
    DRV_SCLIRQ_MUTEX_UNLOCK();
}
void DrvSclIrqSetFrameDoneInt(u64 u64FrameDoneInt)
{
    gu64FrameDoneInt = u64FrameDoneInt;
}
DrvSclIrqScIntsType_e DrvSclIrqM2MIsrHandler(u64 u64Time)
{
    u64 u64Flag = 0;
    DrvSclIrqScIntsType_e enRet = 0;
    u64 u64OpenInt = (SCLIRQ_MSK_SC3_DMA_W_ACTIVE|SCLIRQ_MSK_SC3_DMA_W_ACTIVE_N);
    u64Flag = HalSclIrqGetFlag(E_DRV_SCLIRQ_SCTOP_1,u64OpenInt);
    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_1,u64Flag, 1);
    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_1,u64Flag, 0);
    if(u64Flag & SCLIRQ_MSK_SC3_DMA_W_ACTIVE_N)
    {
        enRet |= E_DRV_SCL_IRQ_SCDONE;
        DrvSclIrqSetM2MInts(E_DRV_SCL_IRQ_SCDONE,u64Time);
        HalSclIrqSetReg(ISM2MDone_DUMMYReg, ISM2MDone_DUMMY, ISM2MDone_DUMMY);
        DrvSclOsSetEvent(DrvSclIrqGetIRQM2MEventID(), E_SCLIRQ_M2MEVENT_ACTIVEN);
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_SCDONE), "[VPESCL]%s Done!!@:%llu\n",
            __FUNCTION__,u64Time);
    }
    if(u64Flag & SCLIRQ_MSK_SC3_DMA_W_ACTIVE)
    {
        enRet |= E_DRV_SCL_IRQ_VSYNC;
        DrvSclIrqSetM2MInts(E_DRV_SCL_IRQ_VSYNC,u64Time);
        HalSclIrqSetReg(ISM2MDone_DUMMYReg, 0, ISM2MDone_DUMMY);
        DrvSclOsClearEvent(DrvSclIrqGetIRQM2MEventID(), E_SCLIRQ_M2MEVENT_ACTIVEN);
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_VSYNC), "[VPESCL]%s Active!!@:%llu\n",
            __FUNCTION__,u64Time);
    }
    return enRet;
}
DrvSclIrqScIntsType_e DrvSclIrqIsrHandler(u64 u64Time)
{
    DrvSclIrqScIntsType_e enRet = 0;
	static u8 nSwDoneFlag = 0;
	static u8 u8HvspEnFlag = 0;
	static u8 u8DVFlag = 0;
	bool bhvspen;
	u16 u16val;
    u64 u64Flag = 0;
    u64 u64RealFlag = 0;
    u64 u64MskFlag = 0;
    u64 u64ClrFlag = 0;
    u64 u64OpenInt =
    (gu64FrameDoneInt|
    SCLIRQ_MSK_SC_IN_FRM_END |
    SCLIRQ_MSK_VSYNC_IDCLK |
    SCLIRQ_MSK_SC4_HVSP_FINISH);
    u64Flag = HalSclIrqGetFlag(E_DRV_SCLIRQ_SCTOP_0,u64OpenInt);
    u64MskFlag = HalSclIrqGetMask(E_DRV_SCLIRQ_SCTOP_0,u64OpenInt);
    u64RealFlag = (u64Flag & ~(u64MskFlag));
    bhvspen = _IsHvspEnable();
    // if MDwin only ,
    if(u64Flag & SCLIRQ_MSK_SC4_HVSP_FINISH)
    {
        if(u64RealFlag & SCLIRQ_MSK_SC4_HVSP_FINISH)
        {
            u64ClrFlag |= SCLIRQ_MSK_SC4_HVSP_FINISH;
            enRet |= E_DRV_SCL_IRQ_SCDONE;
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_SCDONE,u64Time);
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_SCDONE), "[VPESCL]%s SC4 Done @:%llu\n",
                __FUNCTION__,u64Time);
        }
    }
    if(u64Flag & gu64FrameDoneInt)
    {
        DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_SCDONE,u64Time);
        u64ClrFlag |= gu64FrameDoneInt;
        if(u64RealFlag & gu64FrameDoneInt)
        {
            enRet |= E_DRV_SCL_IRQ_SCDONE;
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_SCDONE), "[VPESCL]%s SC Done @:%llu\n",
                __FUNCTION__,u64Time);
        }
    }
    if(u64Flag & SCLIRQ_MSK_SC_IN_FRM_END)
    {
        DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ISPDONE,u64Time);
        u64ClrFlag |= SCLIRQ_MSK_SC_IN_FRM_END;
        if(u64RealFlag & SCLIRQ_MSK_SC_IN_FRM_END)
        {
            if(!gbEachDMAEnable && (_geIrqEnableMode & E_DRV_SCL_IRQ_REALTIME_MODE))
            {
                HalSclIrqSetReg(REG_SCL0_01_L, 1, 0xFFFF);
                HalSclIrqSetReg(REG_SCL0_01_L, 0, 0xFFFF);
                SCL_DBGERR("[VPESCL]%s SW Reset @:%llu\n",__FUNCTION__,u64Time);
            }
            enRet |= E_DRV_SCL_IRQ_ISPDONE;
            gbEachDMAEnable = 1;
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_SCDONE), "[VPESCL]%s ISP Done @:%llu\n",
                __FUNCTION__,u64Time);
        }
    }
    if(u64Flag & SCLIRQ_MSK_VSYNC_IDCLK)
    {
        DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_VSYNC,u64Time);
        u64ClrFlag |= SCLIRQ_MSK_VSYNC_IDCLK;
        if(u64RealFlag & SCLIRQ_MSK_VSYNC_IDCLK)
        {
            enRet |= E_DRV_SCL_IRQ_VSYNC;
            if(enRet&E_DRV_SCL_IRQ_ISPDONE)
            {
                enRet &= ~(E_DRV_SCL_IRQ_ISPDONE);
            }
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_VSYNC), "[VPESCL]%s Vsync @:%llu\n",
                __FUNCTION__,u64Time);
        }
    }
    if(u64Flag & SCLIRQ_MSK_AFF_FULL && !(u64Flag & SCLIRQ_MSK_VSYNC_IDCLK))
    {
        DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_AFF,u64Time);
        SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_AFF), "[VPESCL]%s AFF Flag @:%llu\n",
            __FUNCTION__,u64Time);
    }
    else if(u64Flag & SCLIRQ_MSK_AFF_FULL)
    {
        u64ClrFlag |= SCLIRQ_MSK_AFF_FULL;
    }
	if((enRet & E_DRV_SCL_IRQ_ERR) ==E_DRV_SCL_IRQ_ERR)
	{
        if(HalSclIrqGetRegVal(ISDV_DUMMYReg)&ISDVuse_DUMMY)
        {
            u8DVFlag = 1;
        }
        else
        {
            if( (_geIrqEnableMode &E_DRV_SCL_IRQ_REALTIME_MODE))
            {
                enRet = E_DRV_SCL_IRQ_VSYNC;
            }
            else
            {
                enRet = E_DRV_SCL_IRQ_SCDONE;
            }
            u8DVFlag = 0;
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_FERR,u64Time);
            //SCL_DBGERR("[VPESCL]%s fake double vsync!!@:%llu %llu\n",  __FUNCTION__,u64Time,(DrvSclOsGetSystemTimeStamp()));
            //SCL_DBGERR("[VPESCL]Cmdq flag:%hx\n",HalSclIrqGetRegVal(0x15253E));
            //SCL_DBGERR("[VPESCL]Crop %hx %hx\n",HalSclIrqGetRegVal(0x1527E0),HalSclIrqGetRegVal(0x1527E2));
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_ELSE), "[VPESCL]%s fake double vsync!!@:%llu %llu\n",  __FUNCTION__,u64Time,(DrvSclOsGetSystemTimeStamp()));
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_ELSE), "[VPESCL]Cmdq flag:%hx\n",HalSclIrqGetRegVal(REG_SCL0_1F_L));
            SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&(EN_DBGMG_SCLIRQLEVELSI_ELSE), "[VPESCL]Crop %hx %hx\n",HalSclIrqGetRegVal(REG_SCL2_70_L),HalSclIrqGetRegVal(REG_SCL2_71_L));
        }
	}
	if(((enRet & E_DRV_SCL_IRQ_ERR) ==E_DRV_SCL_IRQ_ERR)&& u8DVFlag &&(_geIrqEnableMode &E_DRV_SCL_IRQ_REALTIME_MODE))
	{
        SCL_DBGERR( "[VPESCL]%s double vsync!!@:%llu %llu\n",  __FUNCTION__,u64Time,(DrvSclOsGetSystemTimeStamp()));
        SCL_DBGERR( "[VPESCL]Cmdq flag:%hx\n",HalSclIrqGetRegVal(REG_SCL0_1F_L));
        SCL_DBGERR( "[VPESCL]Crop %hx %hx\n",HalSclIrqGetRegVal(REG_SCL2_70_L),HalSclIrqGetRegVal(REG_SCL2_71_L));
        nSwDoneFlag = 0;
        u8DVFlag = 1;
        u8HvspEnFlag = 0;
        _DrvSclIrqSetIsBlankingRegion(0);
        //ToDo:
        u16val = HalSclIrqGetRegVal(REG_SCL0_1E_L);
        HalSclIrqSetReg(REG_SCL0_1E_L, ~u16val, ~u16val);
        HalSclIrqSetReg(REG_SCL0_1E_L, 0, ~u16val);
        HalSclIrqSetReg(REG_CMDQ_54_L, 0xFFFF, 0xFFFF);// clear CMDQ trigger bus
        DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ERR,u64Time);
    }
	else if(enRet&E_DRV_SCL_IRQ_SCDONE)
	{
		_DrvSclIrqSetIsBlankingRegion(1);
		nSwDoneFlag = 1;
	}
	else if(enRet&E_DRV_SCL_IRQ_ISPDONE)
	{
	    if(u8DVFlag)
        {
            u8DVFlag = 0;
        }
        else if(_IsHvspEnable()==0 && !u8HvspEnFlag)
        {
	        _DrvSclIrqSetIsBlankingRegion(1);
        }
	}
	else
	{
		/*Because wait done event close dma , the next frame done can't be trigger*/
		/*We need to set blanking as 1 , let next frame cant be work*/
		if(enRet&E_DRV_SCL_IRQ_VSYNC)
		{
		    if(gbEachDMAEnable)
		    {
                _DrvSclIrqSetIsBlankingRegion(0);
            }
			if(!nSwDoneFlag && (gbPtgen))
			{
				_DrvSclIrqSetIsBlankingRegion(1);
			}
			nSwDoneFlag = 0;
			u8HvspEnFlag = _IsHvspEnable();
		}
	}
    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,u64ClrFlag, 1);
    //to avoid clear not reset.
    HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_0,u64ClrFlag | ~(u64MskFlag)| SCLIRQ_MSK_MLOAD_DONE, 0);
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVELSI_ELSE,
    "[SCLVPE]port0 en:%hx port1 en:%hx port2 en:%hx port3 en:%hx hvsp en:%hx\nport0 addr:%lx port1 addr:%lx port2 addr:%lx port3 addr:%lx \ncmdummy:%hx Cmdq flag:%hx swdummy:%hx pdummy:%hx pqdummy:%hx\n",
        _IsPort0Enable(),_IsPort1Enable(),
        _IsPort2Enable(),_IsPort3Enable(),bhvspen,
        _GetPort0ActiveBuf(),
        _GetPort1ActiveBuf(),
        _GetPort2ActiveBuf(),
        _GetPort3ActiveBuf(),
        HalSclIrqGetRegVal(ISCMDQ_IDLEDUMMYReg),HalSclIrqGetRegVal(REG_SCL0_1F_L),HalSclIrqGetRegVal(ISSW_IDLEDUMMYReg),HalSclIrqGetRegVal(ISSCPROCESS_DUMMYReg),HalSclIrqGetRegVal(ISINPROCESSQ_DUMMYReg));
        return enRet;
}
void _DrvSclIrqSetHwStatusIntsbyPort(SCLIRQPortHistType enType)
{
    gstSCInts.u8QueueRCnt[enType]++;
    gstSCInts.u32DoneCount[enType]++;
    gstSCInts.u8Act[enType] = 0;
    HalSclIrqSetReg(REG_SCL1_2E_L, 0, (ISPORT0_USEDUMMY<<enType));
}
void _DrvSclIrqSetFpsStatusIntsbyPort(DrvSclIrqScIntsFpsType_t *pCfg,u64 u64Time,SCLIRQPortHistType enType)
{
    u32 u32Temp;
    pCfg->u32DoneCount[enType]++;
    if(pCfg->u32DoneCount[enType]>=FPSREFCNT)
    {
        u32Temp = (((u32)u64Time - pCfg->u32OriTime[enType])/pCfg->u32DoneCount[enType]);
        pCfg->u32DoneCount[enType] = 0;
        if(u32Temp)
        pCfg->u32Fps[enType] = FPSSEC/u32Temp;
        pCfg->u32OriTime[enType] = (u32)u64Time;
    }
}
u32 _DrvSclIrqGetActiveBuf(SCLIRQPortHistType enType)
{
    switch(enType)
    {
        case E_DRV_SCL_IRQ_HIST_PORT0:
            return _GetPort0ActiveBuf();
        case E_DRV_SCL_IRQ_HIST_PORT1:
            return _GetPort1ActiveBuf();
        case E_DRV_SCL_IRQ_HIST_PORT2:
            return _GetPort2ActiveBuf();
        case E_DRV_SCL_IRQ_HIST_PORT3:
            return _GetPort3ActiveBuf();
        default:
        SCLOS_BUG();
    }
}
void _DrvSclIrqGetHwStatusInts(u64 u64Time)
{
    SCLIRQPortHistType enType;
    for(enType=E_DRV_SCL_IRQ_HIST_PORT0;enType<E_DRV_SCL_IRQ_HIST_MAX;enType++)
    {
        if(_IsPortDone(enType))
        {
            _DrvSclIrqSetHwStatusIntsbyPort(enType);
            _DrvSclIrqSetFpsStatusIntsbyPort(&gstFps,u64Time,enType);
        }
        gstSCInts.u32ActiveAddr[enType] = _DrvSclIrqGetActiveBuf(enType);
    }
    gstSCInts.u8ProcessActCnt = _GetProcessActCnt();
}
u32 DrvSclIrqGetProcessHist(DrvSclIrqProcessHistType_e enType)
{
    return gu32ProcessDiffTime[enType];
}
u32 DrvSclIrqGetProcessDiffDoneHist(DrvSclIrqProcessHistType_e enType)
{
    return gu32ProcessDiffDoneTime[enType];
}
void DrvSclIrqSetProcessHist(u32 u32Time)
{
    if(u32Time < 10000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_10]++;
    }
    else if(u32Time < 20000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_20]++;
    }
    else if(u32Time < 30000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_30]++;
    }
    else if(u32Time < 40000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_40]++;
    }
    else if(u32Time < 50000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_50]++;
    }
    else if(u32Time < 60000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_60]++;
    }
    else if(u32Time < 70000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_70]++;
    }
    else if(u32Time < 80000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_80]++;
    }
    else if(u32Time < 90000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_90]++;
    }
    else if(u32Time < 100000)
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_100]++;
    }
    else
    {
        gu32ProcessDiffTime[E_DRV_SCLIRQ_Process_Hist_100UP]++;
    }
}
void DrvSclIrqSetProcessDiffDoneHist(u32 u32Time)
{
    if(u32Time < 10000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_10]++;
    }
    else if(u32Time < 20000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_20]++;
    }
    else if(u32Time < 30000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_30]++;
    }
    else if(u32Time < 40000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_40]++;
    }
    else if(u32Time < 50000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_50]++;
    }
    else if(u32Time < 60000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_60]++;
    }
    else if(u32Time < 70000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_70]++;
    }
    else if(u32Time < 80000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_80]++;
    }
    else if(u32Time < 90000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_90]++;
    }
    else if(u32Time < 100000)
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_100]++;
    }
    else
    {
        gu32ProcessDiffDoneTime[E_DRV_SCLIRQ_Process_Hist_100UP]++;
    }
}
void DrvSclIrqResetSclHistInts(void)
{
    gstSCInts.u32AffCount = 0;
    gstSCInts.u32SCLMainDoneCount = 0;
    gstSCInts.u32DoneCount[E_DRV_SCL_IRQ_HIST_PORT0] = 0;
    gstSCInts.u32DoneCount[E_DRV_SCL_IRQ_HIST_PORT1] = 0;
    gstSCInts.u32DoneCount[E_DRV_SCL_IRQ_HIST_PORT2] = 0;
    gstSCInts.u32DoneCount[E_DRV_SCL_IRQ_HIST_PORT3] = 0;
    gstSCInts.u32ISPDoneCount = 0;
    gstSCInts.u32ISPInCount = 0;
    gstSCInts.u16ErrorCount = 0;
    gstSCInts.u8ProcessLateCnt = 0;
    gstSCInts.u8ProcessDropCnt = 0;
    DrvSclOsMemset(gu32ProcessDiffTime,0x0,sizeof(u32)*E_DRV_SCLIRQ_Process_Hist_MAX);
    DrvSclOsMemset(gu32ProcessDiffDoneTime,0x0,sizeof(u32)*E_DRV_SCLIRQ_Process_Hist_MAX);
}

void DrvSclIrqSetSclIntsTimeStamp(DrvSclIrqTimeStampType_e enType,u64 u64Time)
{
    u64 * u64val;
    if(gstTimeStampBuffer[enType].bLock)
    {
        return;
    }
    if(gstTimeStampBuffer[enType].memVirAddr==NULL)
    {
        return;
    }
    u64val = (u64 *)(u32)gstTimeStampBuffer[enType].memVirAddr;
    *(u64val+gstTimeStampBuffer[enType].u32WPoint) = u64Time;
    gstTimeStampBuffer[enType].u32WPoint++;
    if(gstTimeStampBuffer[enType].u32WPoint>=TIMESTAMPBUFCNT)
    {
        gstTimeStampBuffer[enType].u32WPoint = 0;
    }
    if(gstTimeStampBuffer[enType].u32WPoint == gstTimeStampBuffer[enType].u32RPoint)
    {
        gstTimeStampBuffer[enType].bRing = 1;
    }
}
void _DrvSclIrqSetbPrintflag(u8 u8Flag,bool *bPrint,bool bEn)
{
    if(gstTimeStampBuffer[0].memVirAddr==NULL)
    {
        return;
    }
    if(u8Flag & E_DRV_SCLIRQ_TimeStamp_Flag_Vsync)
    {
        bPrint[E_DRV_SCLIRQ_TimeStamp_Vsync] = 1;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].bLock = bEn;
    }
    if(u8Flag & E_DRV_SCLIRQ_TimeStamp_Flag_ISPDone)
    {
        bPrint[E_DRV_SCLIRQ_TimeStamp_ISPDone] = 1;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_ISPDone].bLock = bEn;
    }
    if(u8Flag & E_DRV_SCLIRQ_TimeStamp_Flag_SCDone)
    {
        bPrint[E_DRV_SCLIRQ_TimeStamp_SCDone] = 1;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_SCDone].bLock = bEn;
    }
    if(u8Flag & E_DRV_SCLIRQ_TimeStamp_Flag_Process)
    {
        bPrint[E_DRV_SCLIRQ_TimeStamp_Process] = 1;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Process].bLock = bEn;
    }
    if(u8Flag & E_DRV_SCLIRQ_TimeStamp_Flag_Late)
    {
        bPrint[E_DRV_SCLIRQ_TimeStamp_Late] = 1;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Late].bLock = bEn;
    }
    if(u8Flag & E_DRV_SCLIRQ_TimeStamp_Flag_Drop)
    {
        bPrint[E_DRV_SCLIRQ_TimeStamp_Drop] = 1;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Drop].bLock = bEn;
    }
    if(u8Flag & E_DRV_SCLIRQ_TimeStamp_Flag_SCDoneWhenLate)
    {
        bPrint[E_DRV_SCLIRQ_TimeStamp_SCDoneWhenLate] = 1;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_SCDoneWhenLate].bLock = bEn;
    }
    if(u8Flag & E_DRV_SCLIRQ_TimeStamp_Flag_ProcessWhenLate)
    {
        bPrint[E_DRV_SCLIRQ_TimeStamp_ProcessWhenLate] = 1;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_ProcessWhenLate].bLock = bEn;
    }
}
void _DrvSclIrqTimeStampAddlist
    (DrvSclIrqTimeStampLinklistHead_t *stNew,DrvSclIrqTimeStampLinklistHead_t *stPrev)
{
    DrvSclIrqTimeStampLinklistHead_t *NextEntry;
    NextEntry = stPrev->next;
    stPrev->next = stNew;
    stNew->prev = stPrev;
    stNew->next = NextEntry;
    NextEntry->prev = stNew;
}
void _DrvSclIrqTimeStampDellist
    (DrvSclIrqTimeStampLinklistHead_t *stDel,DrvSclIrqTimeStampLinklistHead_t *stPrev)
{
    stPrev->next = stDel->next;
    stDel->next->prev = stPrev;
    stDel->prev = NULL;
    stDel->next = NULL;
}
DrvSclIrqTimeStampLinklistHead_t* _DrvSclIrqTimeStampCreateEntry
    (DrvSclIrqTimeStampType_e enType)
{
    DrvSclIrqTimeStampLinklistHead_t *entry;
    u64 * u64val;
    u32 u32Point;
    if(gstTimeStampBuffer[enType].memVirAddr==NULL)
    {
        return NULL;
    }
    if(gstTimeStampBuffer[enType].bRing)
    {
        gstTimeStampBuffer[enType].bRing = 0;
        gstTimeStampBuffer[enType].u32RPoint = gstTimeStampBuffer[enType].u32WPoint;
    }
    else
    {
        if(gstTimeStampBuffer[enType].u32WPoint == gstTimeStampBuffer[enType].u32RPoint)
        {
            return NULL;
        }
    }
    u32Point = gstTimeStampBuffer[enType].u32RPoint;
    u64val = (u64 *)(u32)gstTimeStampBuffer[enType].memVirAddr;
    entry = (DrvSclIrqTimeStampLinklistHead_t *)
    DrvSclOsMemalloc(sizeof(DrvSclIrqTimeStampLinklistHead_t),GFP_KERNEL);
    entry->enType = enType;
    entry->u64Val = *(u64val+u32Point);
    gstTimeStampBuffer[enType].u32RPoint++;
    if(gstTimeStampBuffer[enType].u32RPoint>=TIMESTAMPBUFCNT)
    {
        gstTimeStampBuffer[enType].u32RPoint = 0;
    }
    return entry;
}
void _DrvSclIrqTimeStampDestroyEntry(DrvSclIrqTimeStampLinklistHead_t *stEntry)
{
    DrvSclOsMemFree(stEntry);
}
DrvSclIrqTimeStampLinklistHead_t * _DrvSclIrqTimeStampSearchPreEntry
    (DrvSclIrqTimeStampLinklistHead_t *pstEntry,DrvSclIrqTimeStampLinklistHead_t *pstLinkTail)
{
    DrvSclIrqTimeStampLinklistHead_t *pstPreEntry;
    pstPreEntry = pstLinkTail;
    do
    {
        pstPreEntry = pstPreEntry->prev;
        if(pstPreEntry->enType==E_DRV_SCLIRQ_TimeStamp_NUM)
        {
            break;
        }
        else if(pstPreEntry->u64Val<=pstEntry->u64Val)
        {
            break;
        }
    }
    while(pstPreEntry->prev!=NULL);
    return pstPreEntry;
}
void _DrvSclIrqTimeStampCreateAndAddEntry
(DrvSclIrqTimeStampType_e enType,DrvSclIrqTimeStampLinklistHead_t *stLinkTail)
{
    DrvSclIrqTimeStampLinklistHead_t *stEntry = NULL;
    DrvSclIrqTimeStampLinklistHead_t *stPreEntry;
    if(enType!=E_DRV_SCLIRQ_TimeStamp_NUM)
    {
        stEntry = _DrvSclIrqTimeStampCreateEntry(enType);
    }
    if(stEntry)
    {
        stPreEntry = _DrvSclIrqTimeStampSearchPreEntry(stEntry,stLinkTail);
        _DrvSclIrqTimeStampAddlist(stEntry,stPreEntry);
    }

}
void _DrvSclIrqPrintSclTimeStampListInit
(bool *bPrint,DrvSclIrqTimeStampLinklistHead_t *stLinkTail)
{
    DrvSclIrqTimeStampType_e enType;
    for(enType=E_DRV_SCLIRQ_TimeStamp_Vsync;enType<E_DRV_SCLIRQ_TimeStamp_NUM;enType++)
    {
        if(bPrint[enType])
        {
            _DrvSclIrqTimeStampCreateAndAddEntry(enType,stLinkTail);
        }
    }
}
DrvSclIrqTimeStampType_e _DrvSclIrqPrintSclTimeStampHead(DrvSclIrqTimeStampLinklistHead_t *stLinkHead)
{
    DrvSclIrqTimeStampType_e enType;
    DrvSclIrqTimeStampLinklistHead_t *pstEntry;
    u64 u64Val;
    pstEntry = stLinkHead->next;
    enType = pstEntry->enType;
    if(enType != E_DRV_SCLIRQ_TimeStamp_NUM)
    {
        u64Val = pstEntry->u64Val;
        _DrvSclIrqTimeStampDellist(pstEntry,stLinkHead);
        _DrvSclIrqTimeStampDestroyEntry(pstEntry);
        SCL_DBGERR("[Drvsclirq]%s @:%llu\n",Print_TimeStamp_Parser(enType),u64Val);
    }
    return enType;
}
void _DrvSclIrqPrintSclTimeStampListProcess
(bool *bPrint,DrvSclIrqTimeStampLinklistHead_t *stLinkHead,DrvSclIrqTimeStampLinklistHead_t *stLinkTail)
{
    DrvSclIrqTimeStampType_e enType;
    do{
    //print smaller and return the type to relink.
    enType = _DrvSclIrqPrintSclTimeStampHead(stLinkHead);
    //relink
    _DrvSclIrqTimeStampCreateAndAddEntry(enType,stLinkTail);
    }
    while(enType!=E_DRV_SCLIRQ_TimeStamp_NUM);
}

/*
LinkList array is
Head = SMALLER..=..BIGGER = Tail
*/
void DrvSclIrqPrintSclHistInts(u8 u8Flag)
{
    bool bPrint[E_DRV_SCLIRQ_TimeStamp_NUM];
    DrvSclIrqTimeStampLinklistHead_t stLinkHead;
    DrvSclIrqTimeStampLinklistHead_t stLinkTail;
    stLinkHead.next = &stLinkTail;
    stLinkHead.prev = NULL;
    stLinkHead.u64Val = 0;
    stLinkHead.enType = E_DRV_SCLIRQ_TimeStamp_NUM;
    stLinkTail.next = NULL;
    stLinkTail.prev = &stLinkHead;
    stLinkTail.u64Val = 0xFFFFFFFFFFFFFFFF;
    stLinkTail.enType = E_DRV_SCLIRQ_TimeStamp_NUM;
    DrvSclOsMemset(bPrint,0x0,sizeof(bPrint));
    _DrvSclIrqSetbPrintflag(u8Flag,bPrint,1);
    _DrvSclIrqPrintSclTimeStampListInit(bPrint,&stLinkTail);
    _DrvSclIrqPrintSclTimeStampListProcess(bPrint,&stLinkHead,&stLinkTail);
    _DrvSclIrqSetbPrintflag(u8Flag,bPrint,0);
}
void DrvSclIrqSetM2MInts(DrvSclIrqScIntsType_e enType,u64 u64Time)
{
    switch(enType)
    {
        case E_DRV_SCL_IRQ_SCDONE:
            gstSCInts.u32M2MDoneCount++;
            if(gu64ActTime[E_DRV_SCLIRQ_CLK_SC3FRMACT])
            {
                gstSCInts.u32M2MActiveTime = u64Time -gu64ActTime[E_DRV_SCLIRQ_CLK_SC3FRMACT];
                gu64ActTime[E_DRV_SCLIRQ_CLK_SC3FRMACT] = 0;
            }
            if(_geM2MIrqEnableMode&E_DRV_SCL_IRQ_PORT2)
            {
                gstSCInts.u8QueueRCnt[E_DRV_SCL_IRQ_HIST_PORT2]++;
                gstSCInts.u32DoneCount[E_DRV_SCL_IRQ_HIST_PORT2]++;
                gstSCInts.u8Act[E_DRV_SCL_IRQ_HIST_PORT0] = 0;
            }
            break;
        case E_DRV_SCL_IRQ_VSYNC:
            gu64ActTime[E_DRV_SCLIRQ_CLK_SC3FRMACT] = u64Time;
            if(_geM2MIrqEnableMode&E_DRV_SCL_IRQ_PORT2)
            {
                gstSCInts.u8Act[E_DRV_SCL_IRQ_HIST_PORT2] = 1;
                gstSCInts.u32ActiveAddr[E_DRV_SCL_IRQ_HIST_PORT2] = _GetPort2ActiveBuf();
            }
            break;
        default:
            break;
    }
}
u8 DrvSclIrqGetSclProcessCnt(void)
{
    return gstSCInts.u8ProcessCnt;
}
void DrvSclIrqGetFps(SCLIRQPortHistType enType,bool bHW,u32 *u32Fps,u32 *u32fFps)
{
    DrvSclIrqScIntsFpsType_t *pstFps;
    pstFps = bHW ? &gstFps : &gstProcessFps;
    *u32Fps = (pstFps->u32Fps[enType]/10);
    *u32fFps = (pstFps->u32Fps[enType]%10);
}

void DrvSclIrqSetSclInts(DrvSclIrqScIntsType_e enType,u64 u64Time)
{
    u64 u64DiffTime = 0;
    _DrvSclIrqGetHwStatusInts((DrvSclOsGetSystemTimeStamp()));
    switch(enType)
    {
        case E_DRV_SCL_IRQ_AFF:
            gstSCInts.u32AffCount++;
            break;
        case E_DRV_SCL_IRQ_SCDONE:
            gstSCInts.u32SCLMainDoneCount++;
            gu64ActTime[E_DRV_SCLIRQ_CLK_FRMDONE] = u64Time;
            gu64ActTime[E_DRV_SCLIRQ_CLK_SC1FRMACT] = u64Time;
            if(gu64ActTime[E_DRV_SCLIRQ_CLK_FCLK_LDC])
            {
                u64DiffTime = u64Time -gu64ActTime[E_DRV_SCLIRQ_CLK_FCLK_LDC];
                gu64ActTime[E_DRV_SCLIRQ_CLK_FCLK_LDC] = 0;
                gstSCInts.u32SCLMainActiveTime = (u32)u64DiffTime;
            }
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_SCDone,u64Time);
            break;
        case E_DRV_SCL_IRQ_ISPDONE:
            gstSCInts.u32ISPDoneCount++;
            if(gu64ActTime[E_DRV_SCLIRQ_CLK_FCLK_LDC])
            {
                u64DiffTime = u64Time - gu64ActTime[E_DRV_SCLIRQ_CLK_FCLK_LDC];
                gstSCInts.u32ISPTime = (u32)u64DiffTime;
            }
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_ISPDone,u64Time);
            break;
        case E_DRV_SCL_IRQ_VSYNC:
            gstSCInts.u32ISPInCount++;
            gu64ActTime[E_DRV_SCLIRQ_CLK_FCLK_LDC] = u64Time;
            if(_IsPort0Enable()&&_IsHvspEnable())
            {
                gstSCInts.u8Act[E_DRV_SCL_IRQ_HIST_PORT0] = 1;
            }
            if(_IsPort1Enable()&&_IsHvspEnable())
            {
                gstSCInts.u8Act[E_DRV_SCL_IRQ_HIST_PORT1] = 1;
            }
            if(_IsPort2Enable()&&_IsHvspEnable())
            {
                gstSCInts.u8Act[E_DRV_SCL_IRQ_HIST_PORT2] = 1;
            }
            if(_IsPort3Enable()&&_IsHvspEnable())
            {
                gstSCInts.u8Act[E_DRV_SCL_IRQ_HIST_PORT3] = 1;
            }
            if(gu64ActTime[E_DRV_SCLIRQ_CLK_FRMDONE])
            {
                u64DiffTime = u64Time - gu64ActTime[E_DRV_SCLIRQ_CLK_FRMDONE];
                gu64ActTime[E_DRV_SCLIRQ_CLK_FRMDONE] = 0;
                gstSCInts.u32ISPBlankingTime = (u32)u64DiffTime;
            }
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_Vsync,u64Time);
            break;
        case E_DRV_SCL_IRQ_ERR:
            gstSCInts.u16ErrorCount++;
            break;
        case E_DRV_SCL_IRQ_FERR:
            gstSCInts.u16FakeErrorCount++;
            break;
        case E_DRV_SCL_IRQ_PROCESSCNT:
            HalSclIrqSetReg(ISINPROCESSQ_DUMMYReg, (u8)u64Time, ISINPROCESSQ_DUMMY);
            gstSCInts.u8ProcessCnt = (u8)u64Time;
            break;
        case E_DRV_SCL_IRQ_ADDPORT0QUEUE:
            gstSCInts.u8QueueWCnt[E_DRV_SCL_IRQ_HIST_PORT0] ++;
            _DrvSclIrqSetFpsStatusIntsbyPort(&gstProcessFps,u64Time,E_DRV_SCL_IRQ_HIST_PORT0);
            break;
        case E_DRV_SCL_IRQ_ADDPORT1QUEUE:
            gstSCInts.u8QueueWCnt[E_DRV_SCL_IRQ_HIST_PORT1] ++;
            _DrvSclIrqSetFpsStatusIntsbyPort(&gstProcessFps,u64Time,E_DRV_SCL_IRQ_HIST_PORT1);
            break;
        case E_DRV_SCL_IRQ_ADDPORT2QUEUE:
            gstSCInts.u8QueueWCnt[E_DRV_SCL_IRQ_HIST_PORT2] ++;
            _DrvSclIrqSetFpsStatusIntsbyPort(&gstProcessFps,u64Time,E_DRV_SCL_IRQ_HIST_PORT2);
            break;
        case E_DRV_SCL_IRQ_ADDPORT3QUEUE:
            gstSCInts.u8QueueWCnt[E_DRV_SCL_IRQ_HIST_PORT3] ++;
            _DrvSclIrqSetFpsStatusIntsbyPort(&gstProcessFps,u64Time,E_DRV_SCL_IRQ_HIST_PORT3);
            break;
        case E_DRV_SCL_IRQ_PROCESSLATE:
            gstSCInts.u8ProcessLateCnt ++;
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_Late,u64Time);
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_ProcessWhenLate,gu64ActTime[E_DRV_SCLIRQ_CLK_PROCESS]);
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_SCDoneWhenLate,gu64ActTime[E_DRV_SCLIRQ_CLK_SC1FRMACT]);
            break;
        case E_DRV_SCL_IRQ_PROCESSDROP:
            gstSCInts.u8ProcessDropCnt ++;
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_Drop,u64Time);
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_ProcessWhenLate,gu64ActTime[E_DRV_SCLIRQ_CLK_PROCESS]);
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_SCDoneWhenLate,gu64ActTime[E_DRV_SCLIRQ_CLK_SC1FRMACT]);
            break;
        case E_DRV_SCL_IRQ_PROCESSDIFF:
            if(gu64ActTime[E_DRV_SCLIRQ_CLK_PROCESS])
            {
                u64DiffTime = u64Time - gu64ActTime[E_DRV_SCLIRQ_CLK_PROCESS];
                gstSCInts.u32ProcessDiffTime = (u32)u64DiffTime;
            }
            gu64ActTime[E_DRV_SCLIRQ_CLK_PROCESS] = u64Time;
            DrvSclIrqSetProcessHist((u32)u64DiffTime);
            if(gu64ActTime[E_DRV_SCLIRQ_CLK_SC1FRMACT])
            {
                u64DiffTime = u64Time - gu64ActTime[E_DRV_SCLIRQ_CLK_SC1FRMACT];
                gstSCInts.u32ProcessDiffDoneTime = (u32)u64DiffTime;
                DrvSclIrqSetProcessDiffDoneHist((u32)u64DiffTime);
            }
            DrvSclIrqSetSclIntsTimeStamp(E_DRV_SCLIRQ_TimeStamp_Process,u64Time);
            break;
    }
}
DrvSclIrqScIntsType_t* DrvSclIrqGetSclInts(void)
{
    return &gstSCInts;
}
void DrvSclIrqTimeFreeStampBuf(void)
{
    if(gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].memVirAddr)
    {
        DrvSclOsMemFree((void *)gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].memVirAddr);
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].memVirAddr = 0;
    }
}
void DrvSclIrqTimePrepareStampBuf(void)
{
    DrvSclIrqTimeStampType_e enType;
    gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].memVirAddr =
        (DrvSclOsDmemBusType_t)DrvSclOsMemalloc(E_DRV_SCLIRQ_TimeStamp_NUM*TIMESTAMPBUFSIZE,GFP_KERNEL);
    if(gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].memVirAddr)
    {
        DrvSclOsMemset((void *)gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].memVirAddr,0x0,E_DRV_SCLIRQ_TimeStamp_NUM*TIMESTAMPBUFSIZE);
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].u32RPoint  = 0;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].u32WPoint  = 0;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].bLock  = 0;
        gstTimeStampBuffer[E_DRV_SCLIRQ_TimeStamp_Vsync].bRing  = 0;
        for(enType = E_DRV_SCLIRQ_TimeStamp_Vsync;enType<(E_DRV_SCLIRQ_TimeStamp_NUM-1);enType++)
        {
            gstTimeStampBuffer[enType+1].memVirAddr = gstTimeStampBuffer[enType].memVirAddr+ TIMESTAMPBUFSIZE;
            gstTimeStampBuffer[enType+1].u32RPoint  = 0;
            gstTimeStampBuffer[enType+1].u32WPoint  = 0;
            gstTimeStampBuffer[enType+1].bLock  = 0;
            gstTimeStampBuffer[enType+1].bRing  = 0;
        }
    }
}
bool DrvSclIrqInit(DrvSclIrqInitConfig_t *pCfg)
{
    char mutx_word[] = {"_SCLIRQ_Mutex"};

    u8 i;
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
    DrvSclIrqTimePrepareStampBuf();
    DrvSclIrqInitVariable();
    return TRUE;
}
bool DrvSclIrqSuspend(DrvSclIrqSuspendResumeConfig_t *pCfg)
{
    SCL_DBG(SCL_DBG_LV_DRVSCLIRQ()&EN_DBGMG_SCLIRQLEVEL_NORMAL, "%s(%d), bSuspend=%d\n", __FUNCTION__, __LINE__, _bSCLIRQ_Suspend);
    _bSCLIRQ_Suspend = 1;
    _DrvSclIrqSetIsBlankingRegion(1);
    return TRUE;
}

bool DrvSclIrqResume(DrvSclIrqSuspendResumeConfig_t *pCfg)
{
    u16 i;
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
    if(_SCLIRQ_Mutex != -1)
    {
        DrvSclOsDeleteMutex(_SCLIRQ_Mutex);
        _SCLIRQ_Mutex = -1;
    }
    else
    {
        return FALSE;
    }
    DrvSclIrqTimeFreeStampBuf();
    DrvSclOsExit();
    return bRet;
}
bool DrvSclIrqM2MInterruptEnable(u16 u16IRQ)
{
    bool bRet = TRUE;
    if(u16IRQ < SCLIRQ_NUM)
    {
        HalSclIrqSetMask(E_DRV_SCLIRQ_SCTOP_1,((u64)1<<u16IRQ), 0);
        HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_1,((u64)1<<u16IRQ), 0);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

bool DrvSclIrqM2MDisable(u16 u16IRQ)
{
    bool bRet = TRUE;

    if(u16IRQ < SCLIRQ_NUM)
    {
        HalSclIrqSetMask(E_DRV_SCLIRQ_SCTOP_1,((u64)1<<u16IRQ), 1);
        HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_1,((u64)1<<u16IRQ), 1);
		HalSclIrqSetClear(E_DRV_SCLIRQ_SCTOP_1,((u64)1<<u16IRQ), 0);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

bool DrvSclIrqInterruptEnable(u16 u16IRQ)
{
    bool bRet = TRUE;
    if(u16IRQ < SCLIRQ_NUM)
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

    if(u16IRQ < SCLIRQ_NUM)
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

    if(u16IRQ < SCLIRQ_NUM)
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
    if(u16IRQ < SCLIRQ_NUM)
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
#undef DRV_SCL_IRQ_C
