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
#define DRV_VIP_C

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"

#include "hal_scl_util.h"
#include "hal_scl_reg.h"
#include "drv_scl_vip.h"
#include "hal_scl_vip.h"
#include "drv_scl_hvsp_st.h"
#include "drv_scl_hvsp.h"
#include "hal_scl_hvsp.h"
#include "drv_scl_irq_st.h"
#include "drv_scl_irq.h"
#include "drv_scl_pq_define.h"
#include "drv_scl_pq_declare.h"
#include "drv_scl_pq.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_VIP_DBG 0
#define DRV_VIP_LDC_DMAP_align(x,align) ((x+align) & ~(align-1))
#define DMAPBLOCKUNIT       32
#define DMAPBLOCKUNITBYTE   4
#define DMAPBLOCKALIGN      4
#define DEFAULTLDCMD 0x1
#define SRAMNORMAL 0

#define DLCVariableSection 8
#define DLCCurveFitEnable 7
#define DLCCurveFitRGBEnable 13
#define DLCDitherEnable 5
#define DLCHistYRGBEnable 10
#define DLCStaticEnable 1
#define LDCFBRWDiff 4
#define LDCSWModeEnable 10
#define LDCAppointFBidx 2
#define FHD_Width   1920
#define FHD_Height  1080
#define DRV_VIP_MUTEX_LOCK()            //DrvSclOsObtainMutex(_VIP_Mutex,SCLOS_WAIT_FOREVER)
#define DRV_VIP_MUTEX_UNLOCK()          //DrvSclOsReleaseMutex(_VIP_Mutex)

#define DRV_SCLVIP_IO_NR_SAD_SHIFT 16
#define DRV_SCLVIP_IO_NR_SUM_NOISE_SHIFT 32
#define DRV_SCLVIP_IO_NR_CNT_NOISE_SHIFT 16
#define DRV_SCLVIP_NR_Y_SIZE 64
#define DRV_SCLVIP_NR_SIZE 128

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
//keep
s32 _VIP_Mutex = -1;
//-------------------------------------------------------------------------------------------------
//  Loacl Functions
//-------------------------------------------------------------------------------------------------
u16 _DrvSclVipGetDlcEnableSetting(ST_VIP_DLC_HISTOGRAM_CONFIG *stDLCCfg)
{
    u16 u16valueForReg1Eh04;
    u16valueForReg1Eh04 = ((u16)stDLCCfg->bVariable_Section<<DLCVariableSection)|
        ((u16)stDLCCfg->bcurve_fit_en<<DLCCurveFitEnable)|
        ((u16)stDLCCfg->bcurve_fit_rgb_en<<DLCCurveFitRGBEnable)|
        ((u16)stDLCCfg->bDLCdither_en<<DLCDitherEnable)|
        ((u16)stDLCCfg->bhis_y_rgb_mode_en<<DLCHistYRGBEnable)|
        ((u16)stDLCCfg->bstatic<<DLCStaticEnable);
    return u16valueForReg1Eh04;
}
void _DrvSclVipSetDlcHistRangeEachSection(ST_VIP_DLC_HISTOGRAM_CONFIG *stDLCCfg,bool bEn)
{
    u16 u16sec;
    for(u16sec=0;u16sec<VIP_DLC_HISTOGRAM_SECTION_NUM;u16sec++)
    {
        HalSclVipDlcHistSetRange(stDLCCfg->u8Histogram_Range[u16sec],u16sec+1);
    }
}
void _DrvSclVipSetDlcHistogramRiuConfig(ST_VIP_DLC_HISTOGRAM_CONFIG *stDLCCfg,u16 u16valueForReg1Eh04)
{
    HalSclVipDlcHistVarOnOff(u16valueForReg1Eh04);
    HalSclVipSetDlcstatMIU(stDLCCfg->bstat_MIU,stDLCCfg->u32StatBase[0],stDLCCfg->u32StatBase[1]);
    HalSclVipSetDlcShift(stDLCCfg->u8HistSft);
    HAlSclVipSetDlcMode(stDLCCfg->u8trig_ref_mode);
    HalSclVipSetDlcActWin(stDLCCfg->bRange,stDLCCfg->u16Vst,stDLCCfg->u16Hst,stDLCCfg->u16Vnd,stDLCCfg->u16Hnd);
    _DrvSclVipSetDlcHistRangeEachSection(stDLCCfg,0);
}
void _DrvSclVipGetDlcHistogramConfig(DrvSclVipDlcHistogramReport_t *stdlc)
{
    stdlc->u32PixelCount  = HalSclVipDlcGetPC();
    stdlc->u32PixelWeight = HalSclVipDlcGetPW();
    stdlc->u8Baseidx      = HalSclVipDlcGetBaseIdx();
    stdlc->u8MaxPixel     = HalSclVipDlcGetMaxP();
    stdlc->u8MinPixel     = HalSclVipDlcGetMinP();
}
void _DrvSclVipSetNlmSrambyAutodownLoad(DrvSclVipNlmSramConfig_t *stCfg)
{
    HalSclVipSetAutoDownloadAddr(stCfg->u32baseadr,stCfg->u16iniaddr,VIP_NLM_AUTODOWNLOAD_CLIENT);
    HalSclVipSetAutoDownloadReq(stCfg->u16depth,stCfg->u16reqlen,VIP_NLM_AUTODOWNLOAD_CLIENT);
    HalSclVipSetAutoDownload(stCfg->bCLientEn,stCfg->btrigContinue,VIP_NLM_AUTODOWNLOAD_CLIENT);
#if DRV_VIP_DBG
    u16 u16entry;
    for(u16entry=0;u16entry<VIP_NLM_ENTRY_NUM;u16entry++)
    {
        HalSclVipGetNlmSram(u16entry);
    }
#endif
}
void _DrvSclVipSetNlmSrambyCpu(DrvSclVipNlmSramConfig_t *stCfg)
{
    u16 u16entry;
    u32 u32value,u32addr;
    u32 *pu32Addr = NULL;
    for(u16entry = 0;u16entry<VIP_NLM_ENTRY_NUM;u16entry++)
    {
        u32addr  = stCfg->u32viradr + u16entry * VIP_NLM_AUTODOWNLOAD_BASE_UNIT ;// 1entry cost 16 byte(128 bit)
        pu32Addr = (u32 *)(u32addr);
        u32value = *pu32Addr;
        HalSclVipSetNlmSrambyCPU(u16entry,u32value);
#if DRV_VIP_DBG
        HalSclVipGetNlmSram(u16entry);
#endif
    }
}
HalSclVipSramDumpType_e _DrvSclVipGetSramType(DrvSclVipSramType_e enAIPType)
{
    HalSclVipSramDumpType_e enType;
    switch(enAIPType)
    {
        case E_DRV_SCLVIP_AIP_SRAM_WDR_TBL:
            enType = E_HAL_SCLVIP_SRAM_DUMP_WDR_TBL;
            break;
        default:
            enType = E_HAL_SCLVIP_SRAM_DUMP_NUM;
            break;
    }
    return enType;
}

//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
void DrvSclVipExit(void)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    if(_VIP_Mutex != -1)
    {
        DrvSclOsDeleteMutex(_VIP_Mutex);
        _VIP_Mutex = -1;
    }
    HalSclVipExit();
}
bool DrvSclVipInit(DrvSclVipInitConfig_t *pCfg)
{
    char word[] = {"_VIP_Mutex"};
    void *pvCtx;
    if(_VIP_Mutex != -1)
    {
        SCL_DBG(SCL_DBG_LV_DRVVIP(), "[DRVVIP]%s already done\n", __FUNCTION__);
        return TRUE;
    }
    _VIP_Mutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, word, SCLOS_PROCESS_SHARED);

    if (_VIP_Mutex == -1)
    {
        SCL_DBG(SCL_DBG_LV_DRVVIP(), "[DRVVIP]%s create mutex fail\n", __FUNCTION__);
        return FALSE;
    }

    HalSclVipSetRiuBase(pCfg->u32RiuBase);
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    HalSclHvspSetIpmBufferNumber(DrvSclOsGetSclFrameBufferNum());
    HalSclHvspSetUcmHWrwDiff(1);
    HalSclHvspSetUcmMemConfig(E_HAL_SCLUCM_CE8_ON);
    HalSclHvspSetUcmClk();
    return TRUE;
}
bool DrvSclVipSetIPMConfig(DrvSclVipIpmConfig_t *stCfg)
{
    SCL_DBG(SCL_DBG_LV_DRVVIP()&EN_DBGMG_VIPLEVEL_NORMAL, "[DRVVIP]%s(%d): Base=%lx\n"
        , __FUNCTION__, __LINE__, stCfg->u32YCBaseAddr);
    HalSclHvspSetUcmYCBase1(stCfg->u32YCBaseAddr);
    if(DrvSclOsGetSclFrameBufferNum()== 2)
    {
        HalSclHvspSetUcmYCBase2(stCfg->u32YCBaseAddr+(stCfg->u32MemSize/2));
    }
    HalSclHvspSetUcmMemConfig((stCfg->enCeType==E_DRV_SCLVIP_UCMCE_8) ? E_HAL_SCLUCM_CE8_ON :
        (stCfg->enCeType==E_DRV_SCLVIP_UCMCE_6) ? E_HAL_SCLUCM_CE6_ON : E_HAL_SCLUCM_CE8_ON);
    HalSclHvspSetUCMConpress((stCfg->enCeType==E_DRV_SCLVIP_UCMCE_8) ? E_HAL_SCLUCM_CE8_ON :
        (stCfg->enCeType==E_DRV_SCLVIP_UCMCE_6) ? E_HAL_SCLUCM_CE6_ON : E_HAL_SCLUCM_CE8_ON);
    HalSclHvspSetIpmYCMWriteEn(stCfg->bYCMWrite);
    return TRUE;
}
bool DrvSclVipGetWdrHistogram(DrvSclVipWdrRoiReport_t *pvCfg)
{
    int idx;
    for(idx=0;idx<DRV_SCLVIP_ROI_WINDOW_MAX;idx++)
    {
        //SCL_ERR("[%s] %d\n",__FUNCTION__,idx);
        pvCfg->s32Y[idx] = HalSclVipGetWdrHistogram(E_HAL_SCLVIP_ROI_Y,idx);
        pvCfg->s32R[idx] = HalSclVipGetWdrHistogram(E_HAL_SCLVIP_ROI_R,idx);
        pvCfg->s32G[idx] = HalSclVipGetWdrHistogram(E_HAL_SCLVIP_ROI_G,idx);
        pvCfg->s32B[idx] = HalSclVipGetWdrHistogram(E_HAL_SCLVIP_ROI_B,idx);
        pvCfg->s32YCnt[idx] = HalSclVipGetWdrHistogram(E_HAL_SCLVIP_ROI_CNT,idx);
    }
    return 1;
}
bool DrvSclVipGetNRHistogram(void *pvCfg)
{
    // Y SAD 16
    // Y SUM 32
    // Y CNT 16
    // C SAD 16
    // C SUM 32
    // C CNT 16
    // dummy 10
    HalSclVipSetNRHistogramYCSel(E_HAL_SCLVIP_YCSel_Y);
    HalSclVipGetNRHistogram(pvCfg);
    HalSclVipSetNRHistogramYCSel(E_HAL_SCLVIP_YCSel_C);
    HalSclVipGetNRHistogram(pvCfg+(DRV_SCLVIP_NR_Y_SIZE));
    HalSclVipGetNRDummy(pvCfg+(DRV_SCLVIP_NR_SIZE));
    return 1;
}
bool DrvSclVipSetMaskOnOff(DrvSclVipSetMaskOnOff_t *pvCfg)
{
    switch(pvCfg->enMaskType)
    {
        case E_DRV_SCLVIP_MASK_WDR:
            HalSclVipSetWDRMaskOnOff(pvCfg->bOnOff);
            break;
        case E_DRV_SCLVIP_MASK_NR:
            DrvSclVipSetNrHistOnOff(1);
            HalSclVipSetNRMaskOnOff(pvCfg->bOnOff);
            break;
        default:
            SCL_ERR("[DRVSCLVIP]%s enMaskType not support\n",__FUNCTION__);
            break;
    }
    return 1;
}
void DrvSclVipSetWdrMloadBuffer(u32 u32Buffer)
{
    HalSclVipSetWdrMloadBuffer(u32Buffer);// bEn for control Mask
}
void DrvSclVipSetNrHistOnOff(bool bEn)
{
    HalSclVipSetNrHistOnOff(bEn);// bEn for control Mask
}
#if defined(SCLOS_TYPE_LINUX_DEBUG)
bool DrvSclVipGetRoiConfig(u8 u8Win,DrvSclVipWdrRoiConfig_t *pvCfg)
{
    HalSclVipGetRoiHistCfg(u8Win,pvCfg);
    return 0;
}
#endif
bool DrvSclVipSetRoiConfig(DrvSclVipWdrRoiHist_t *pvCfg)
{
    u16 idx;
    HalSclVipSetRoiHistSrc(pvCfg->enPipeSrc);
    for(idx=0;idx<DRV_SCLVIP_ROI_WINDOW_MAX;idx++)
    {
        if(idx<pvCfg->u8WinCount)
        {
            HalSclVipSetRoiHistCfg(idx,&pvCfg->stRoiCfg[idx]);
        }
        else
        {
            HalSclVipReSetRoiHistCfg(idx);
        }
        HalSclVipSetRoiHistBaseAddr(idx,pvCfg->u32BaseAddr[idx]);
    }
    HalSclVipSetRoiHistOnOff(pvCfg->bEn);
    return 1;
}
void DrvSclVipSetWdrMloadConfig(bool bEn,u32 u32WdrBuf)
{
    HalSclVipSetWdrMultiSensor(bEn);
}
bool DrvSclVipGetWdrOnOff(void)
{
    return HalSclVipGetWdrOnOff();
}
void DrvSclVipHwReset(void)
{
    HalSclVipHwReset();
}
void DrvSclVipHWInit(void)
{
    HalSclVipInitY2R();
    HalSclVipSetVpsSRAMEn(1);
    HalSclVipInitUCMReqLenTh();
    //ToDo
    DrvSclVipSramDump();
    HalSclVipInitNeDummy();
}
void * DrvSclVipSetAipSramConfig(void * pvPQSetParameter, DrvSclVipSramType_e enAIPType)
{
    HalSclVipSramDumpType_e enType;
    HalSclVipWdrTblType_e enWdrType;
    void * pvPQSetPara = NULL;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    enType =  _DrvSclVipGetSramType(enAIPType);
    if(enType == E_HAL_SCLVIP_SRAM_DUMP_WDR_TBL)
    {
        MDrvSclCtxResetWdrTblCnt();
        for(enWdrType = E_HAL_SCLVIP_WDR_TBL_NL;enWdrType<E_HAL_SCLVIP_WDR_TBL_NUM;enWdrType++)
        {
            HalSclVipSetWdrTbl(enWdrType,pvPQSetParameter);
        }
        pvPQSetPara = pvPQSetParameter;
    }
    else
    {
        SCL_ERR("%s not support Type :%d\n",__FUNCTION__,enAIPType);
    }
    return pvPQSetPara;
}
void DrvSclVipSramDump(void)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    HalSclVipSramDump(E_HAL_SCLVIP_SRAM_DUMP_IHC,SRAMNORMAL);
    HalSclVipSramDump(E_HAL_SCLVIP_SRAM_DUMP_ICC,SRAMNORMAL);
    DrvSclVipSetAipSramConfig(NULL,E_DRV_SCLVIP_AIP_SRAM_WDR_TBL);

}
bool DrvSclVipSetNlmSramConfig(DrvSclVipNlmSramConfig_t *stCfg)
{
    DrvSclOsWaitForCpuWriteToDMem();
    if(stCfg->bCLientEn)
    {
        _DrvSclVipSetNlmSrambyAutodownLoad(stCfg);
    }
    else
    {
        _DrvSclVipSetNlmSrambyCpu(stCfg);
    }
    return TRUE;
}
bool DrvSclVipSetDlcHistogramConfig(ST_VIP_DLC_HISTOGRAM_CONFIG *stDLCCfg,bool bEn)
{
    u16 u16valueForReg1Eh04;
    SCL_DBG(SCL_DBG_LV_DRVVIP(), "[DRVVIP]%s: ", __FUNCTION__);
    u16valueForReg1Eh04 = _DrvSclVipGetDlcEnableSetting(stDLCCfg);
    _DrvSclVipSetDlcHistogramRiuConfig(stDLCCfg,u16valueForReg1Eh04);
    SCL_DBG(SCL_DBG_LV_DRVVIP(),
    "[DRVVIP]u16valueForReg1Eh04:%hx ,stDLCCfg->u32StatBase[0]:%lx,stDLCCfg->u32StatBase[1]%lx,stDLCCfg->bstat_MIU:%hhd\n"
        ,u16valueForReg1Eh04,stDLCCfg->u32StatBase[0],stDLCCfg->u32StatBase[1],stDLCCfg->bstat_MIU);

    return TRUE;
}

u32 DrvSclVipGetDlcHistogramReport(u16 u16range)
{
    u32 u32value;
    u32value = HalSclVipDlcHistGetRange(u16range);
    SCL_DBG(SCL_DBG_LV_DRVVIP(), "[DRVVIP]%s Histogram_%hd:%lx \n", __FUNCTION__,u16range,u32value);
    return u32value;
}
void DrvSclVipGetDlcHistogramConfig(DrvSclVipDlcHistogramReport_t *stdlc)
{
    _DrvSclVipGetDlcHistogramConfig(stdlc);
    SCL_DBG(SCL_DBG_LV_DRVVIP(), "[DRVVIP]%s PixelCount:%lx,PixelWeight:%lx,Baseidx:%hhx \n"
        ,__FUNCTION__, stdlc->u32PixelCount,stdlc->u32PixelWeight,stdlc->u8Baseidx);
}
#if defined(SCLOS_TYPE_LINUX_DEBUG)
u16 DrvSclVipGetAIPStatus(DrvSclVipAipType_e enIPType)
{
    u16 u16Ret;
    switch(enIPType)
    {
        case E_DRV_SCLVIP_AIP_YEE:
            u16Ret = HalSclVipGetYeeOnOff();
            break;
        case E_DRV_SCLVIP_AIP_MXNR:
            u16Ret = HalSclVipGetMxnrOnOff();
            break;
        case E_DRV_SCLVIP_AIP_UVADJ:
            u16Ret = HalSclVipGetUvadjOnOff();
            break;
        case E_DRV_SCLVIP_AIP_XNR:
            u16Ret = HalSclVipGetXnrOnOff();
            break;
        case E_DRV_SCLVIP_AIP_PFC:
            u16Ret = HalSclVipGetPfcOnOff();
            break;
        case E_DRV_SCLVIP_AIP_YCUVM:
            u16Ret = HalSclVipGetYcuvmOnOff();
            break;
        case E_DRV_SCLVIP_AIP_COLORTRAN:
            u16Ret = HalSclVipGetCTOnOff();
            break;
        case E_DRV_SCLVIP_AIP_GAMMA:
            u16Ret = HalSclVipGetYUVGammaOnOff();
            break;
        case E_DRV_SCLVIP_AIP_YUVTORGB:
            u16Ret = HalSclVipGetY2ROnOff();
            break;
        case E_DRV_SCLVIP_AIP_RGBTOYUV:
            u16Ret = HalSclVipGetR2YOnOff();
            break;
        case E_DRV_SCLVIP_AIP_GM10TO12:
            u16Ret = HalSclVipGetGMA2COnOff();
            break;
        case E_DRV_SCLVIP_AIP_GM12TO10:
            u16Ret = HalSclVipGetGMC2AOnOff();
            break;
        case E_DRV_SCLVIP_AIP_HSV:
            u16Ret = HalSclVipGetHsvOnOff();
            break;
        case E_DRV_SCLVIP_AIP_CCM:
            u16Ret = HalSclVipGetCcmOnOff();
            break;
        case E_DRV_SCLVIP_AIP_WDR:
            u16Ret = HalSclVipGetWdrOnOff();
            break;
        default:
            u16Ret = 0;
            break;
    }
    return u16Ret;
}
u16 DrvSclVipGetBypassStatus(E_DRV_SCLVIP_CONFIG_TYPE enIPType)
{
    u16 u16Ret;
    switch(enIPType)
    {
        case E_DRV_SCLVIP_CONFIG:
            u16Ret = HalSclVipGetVipBypass();
            break;

        case E_DRV_SCLVIP_MCNR_CONFIG:
            u16Ret = HalSclVipGetMcnrBypass();
            break;

        case E_DRV_SCLVIP_NLM_CONFIG:
            u16Ret = HalSclVipGetNlmBypass();
            break;
        case E_DRV_SCLVIP_ACK_CONFIG:
            u16Ret = HalSclVipGetAckBypass();
            break;
        case E_DRV_SCLVIP_IBC_CONFIG:
            u16Ret = HalSclVipGetIbcBypass();
            break;
        case E_DRV_SCLVIP_ICC_CONFIG:
            u16Ret = HalSclVipGetIccBypass();
            break;
        case E_DRV_SCLVIP_IHC_CONFIG:
            u16Ret = HalSclVipGetIhcBypass();
            break;
        case E_DRV_SCLVIP_FCC_CONFIG:
            u16Ret = HalSclVipGetFccBypass();
            break;
        case E_DRV_SCLVIP_UVC_CONFIG:
            u16Ret = HalSclVipGetUvcBypass();
            break;
        case E_DRV_SCLVIP_DLC_CONFIG:
            u16Ret = HalSclVipGetDlcBypass();
            break;
        case E_DRV_SCLVIP_LCE_CONFIG:
            u16Ret = HalSclVipGetLceBypass();
            break;
        case E_DRV_SCLVIP_PEAKING_CONFIG:
            u16Ret = HalSclVipGetPkBypass();
            break;
        default:
            u16Ret = 0;
            break;
    }
    return u16Ret;
}
#endif
#undef DRV_VIP_C
