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
#ifndef __HAL_SCL_HVSP_H__
#define __HAL_SCL_HVSP_H__


//-------------------------------------------------------------------------------------------------
//  Defines & ENUM
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_SCLHVSP_FILTER_MODE_BYPASS,
    E_HAL_SCLHVSP_FILTER_MODE_BILINEAR,
    E_HAL_SCLHVSP_FILTER_MODE_SRAM_0,
    E_HAL_SCLHVSP_FILTER_MODE_SRAM_1,
}HalSclHvspFilterMode_e;

typedef enum
{
    E_HAL_SCLHVSP_SRAM_SEL_0,
    E_HAL_SCLHVSP_SRAM_SEL_1,
}HalSclHvspSramSelType_e;

typedef enum
{
    E_HALSCLHVSP_FILTER_SRAM_SEL_1,
    E_HALSCLHVSP_FILTER_SRAM_SEL_2,
    E_HALSCLHVSP_FILTER_SRAM_SEL_3,
    E_HALSCLHVSP_FILTER_SRAM_SEL_4,
}HalSclHvspFilterSramSelType_e;

typedef enum
{
    E_HALSCLHVSP_CLKATTR_ISP=0x10,
    E_HALSCLHVSP_CLKATTR_BT656=0x20,
    E_HALSCLHVSP_CLKATTR_FORCEMODE=0x40,
}HalSclHvspClkAttrType_e;
typedef enum
{
    E_HAL_SCLVIP_VTRACK_ENABLE_ON,
    E_HAL_SCLVIP_VTRACK_ENABLE_OFF,
    E_HAL_SCLVIP_VTRACK_ENABLE_DEBUG,
}HalSclHvspVtrackEnableType_e;

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    bool bEn;
    u16  u16In_hsize;
    u16  u16In_vsize;
    u16  u16Hst;
    u16  u16Hsize;
    u16  u16Vst;
    u16  u16Vsize;
}HalSclHvspCropInfo_t;
typedef enum
{
    E_HAL_SCLHVSP_SRAM_DUMP_ICC,
    E_HAL_SCLHVSP_SRAM_DUMP_IHC,
    E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V,
    E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_1,
    E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_2,
    E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_3,
    E_HAL_SCLHVSP_SRAM_DUMP_WDR,
    E_HAL_SCLHVSP_SRAM_DUMP_GAMMA_Y,
    E_HAL_SCLHVSP_SRAM_DUMP_GAMMA_U,
    E_HAL_SCLHVSP_SRAM_DUMP_GAMMA_V,
    E_HAL_SCLHVSP_SRAM_DUMP_GM10to12_R,
    E_HAL_SCLHVSP_SRAM_DUMP_GM10to12_G,
    E_HAL_SCLHVSP_SRAM_DUMP_GM10to12_B,
    E_HAL_SCLHVSP_SRAM_DUMP_GM12to10_R,
    E_HAL_SCLHVSP_SRAM_DUMP_GM12to10_G,
    E_HAL_SCLHVSP_SRAM_DUMP_GM12to10_B,
    E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H,
    E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_1,
    E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_2,
    E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_3,
    E_HAL_SCLHVSP_SRAM_DUMP_NUM,
}HalSclHvspSramDumpType_e;
typedef enum
{
    E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H = 0,
    E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_V,
    E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H_1,
    E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_V_1,
    E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H_2,
    E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_V_2,
    E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H_3,
    E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_V_3,
    E_HAL_SCLHVSP_SRAM_MLOAD_NUM,
}HalSclHvspSramMloadType_e;
typedef enum
{
    E_HAL_SCLUCM_OFF = 0,
    E_HAL_SCLUCM_CE8_ON = 0x1,
    E_HAL_SCLUCM_CE6_ON = 0x2,
    E_HAL_SCLUCM_YONLY_ON = 0x4,
}HalSclUcmOnOffType_e;

typedef enum
{
    E_HAL_SCLHVSP_ISPCLK_123M = 0x0,
    E_HAL_SCLHVSP_ISPCLK_OFF = 0x1,
    E_HAL_SCLHVSP_ISPCLK_86M = 0x4,
    E_HAL_SCLHVSP_ISPCLK_72M = 0x8,
    E_HAL_SCLHVSP_ISPCLK_54M = 0xC,
    E_HAL_SCLHVSP_ISPCLK_144M = 0x10,
    E_HAL_SCLHVSP_ISPCLK_NUM,
}HalSclHvspIspClkType_e;
typedef struct
{
    DrvSclHvspIpMuxType_e genIpType[4];
    u16 gu16height[2];
    u16 gu16width[2];
    ST_HVSP_SIZE_CONFIG gstHalSrcSize;
    u8 u8MloadBufid[E_HAL_SCLHVSP_SRAM_MLOAD_NUM];
    void *pvMloadBuf[E_HAL_SCLHVSP_SRAM_MLOAD_NUM];
    bool bMloadchange[E_HAL_SCLHVSP_SRAM_MLOAD_NUM];
}MDrvSclCtxHalHvspGlobalSet_t;

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifndef HAL_HVSP_C
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE void HalSclHvspExit(void);
INTERFACE void HalSclHvspSetRiuBase(u32 u32RiuBase);
INTERFACE void HalSclHvspSetInputSrcSize(ST_HVSP_SIZE_CONFIG *stSrc);
INTERFACE void HalSclHvspSetCropConfig(DrvSclHvspCropIdType_e enID, HalSclHvspCropInfo_t *stCropInfo);
INTERFACE void HalSclHvspSetNlmRegionSize(HalSclHvspCropInfo_t *stCropInfo);
INTERFACE void HalSclHvspSetHwSc3InputMux(DrvSclHvspIpMuxType_e enIpType,void *pvCtx);
INTERFACE void HalSclHvspSetInputMuxType(DrvSclHvspIdType_e enID,DrvSclHvspIpMuxType_e enIpType);
INTERFACE u32 HalSclHvspGetInputSrcMux(DrvSclHvspIdType_e enID);
INTERFACE void HalSclHvspSetReset(void *pvCtx);
INTERFACE void HalSclHvspReSetHw(void *pvCtx);
INTERFACE void HalSclHvspSetUcmClk(void);
INTERFACE void HalSclHvspSetSWReset(bool bEn);
INTERFACE void HalSclHvspSetDNRReset(bool bEn);

// NLM
INTERFACE void HalSclHvspSetVipSize(u16 u16Width, u16 u16Height);
INTERFACE void HalSclHvspSetNlmEn(bool bEn);
INTERFACE void HalSclHvspSetNlmLineBufferSize(u16 u16Width, u16 u16Height);
//AIP
INTERFACE void HalSclHvspSetWdrGlobalSize(u16 u16Width, u16 u16Height);
INTERFACE void HalSclHvspSetWdrLocalSize(u16 u16Width, u16 u16Height);
INTERFACE void HalSclHvspSetMXnrSize(u16 u16Width, u16 u16Height);
INTERFACE void HalSclHvspSetUVadjSize(u16 u16Width, u16 u16Height);
INTERFACE void HalSclHvspSetXnrSize(u16 u16Width, u16 u16Height);

// IPM
INTERFACE void HalSclHvspSetIpmYCMReadEn(bool bEn);
INTERFACE void HalSclHvspSetIpmYCMWriteEn(bool bEn);
INTERFACE void HalSclHvspSetUcmYCBase1(u32 u32Base);
INTERFACE void HalSclHvspSetUcmYCBase2(u32 u32Base);
INTERFACE void HalSclHvspSetUCMConpress(HalSclUcmOnOffType_e enType);
INTERFACE void HalSclHvspSetIpmvSize(u16 u16Vsize);
INTERFACE void HalSclHvspSetIpmLineOffset(u16 u16Lineoffset);
INTERFACE void HalSclHvspSetIpmFetchNum(u16 u16FetchNum);
INTERFACE void HalSclHvspSetIpmReadReq(u8 u8Thrd, u8 u8Limit);
INTERFACE void HalSclHvspSetIpmWriteReq(u8 u8Thrd, u8 u8Limit);
INTERFACE void HalSclHvspSetIpmBufferNumber(u8 u8Num);
INTERFACE void HalSclHvspSetNeSampleStep(u16 u16Hsize,u16 u16Vsize);

// Scaling
INTERFACE void HalSclHvspSetScalingVeEn(DrvSclHvspIdType_e enID, bool ben);
INTERFACE void HalSclHvspSetScalingVeFactor(DrvSclHvspIdType_e enID, u32 u32Ratio);
INTERFACE void HalSclHvspSetScalingHoEn(DrvSclHvspIdType_e enID, bool ben);
INTERFACE void HalSclHvspSetScalingHoFacotr(DrvSclHvspIdType_e enID, u32 u32Ratio);
INTERFACE void HalSclHvspSetModeYHo(DrvSclHvspIdType_e enID, HalSclHvspFilterMode_e enFilterMode);
INTERFACE void HalSclHvspSetModeYVe(DrvSclHvspIdType_e enID, HalSclHvspFilterMode_e enFilterMode);
INTERFACE void HalSclHvspSetModeCHo(DrvSclHvspIdType_e enID, HalSclHvspFilterMode_e enFilterMode, HalSclHvspSramSelType_e enSramSel);
INTERFACE void HalSclHvspSetModeCVe(DrvSclHvspIdType_e enID, HalSclHvspFilterMode_e enFilterMode, HalSclHvspSramSelType_e enSramSel);
INTERFACE void HalSclHvspSetHspDithEn(DrvSclHvspIdType_e enID, bool bEn);
INTERFACE void HalSclHvspSetVspDithEn(DrvSclHvspIdType_e enID, bool bEn);
INTERFACE void HalSclHvspSetHspCoringEnC(DrvSclHvspIdType_e enID, bool bEn);
INTERFACE void HalSclHvspSetHspCoringEnY(DrvSclHvspIdType_e enID, bool bEn);
INTERFACE void HalSclHvspSetVspCoringEnC(DrvSclHvspIdType_e enID, bool bEn);
INTERFACE void HalSclHvspSetVspCoringEnY(DrvSclHvspIdType_e enID, bool bEn);
INTERFACE void HalSclHvspSetHspCoringThrdC(DrvSclHvspIdType_e enID, u16 u16Thread);
INTERFACE void HalSclHvspSetHspCoringThrdY(DrvSclHvspIdType_e enID, u16 u16Thread);
INTERFACE void HalSclHvspSetVspCoringThrdC(DrvSclHvspIdType_e enID, u16 u16Thread);
INTERFACE void HalSclHvspSetVspCoringThrdY(DrvSclHvspIdType_e enID, u16 u16Thread);
INTERFACE void HalSclHvspSetSramCoeff(DrvSclHvspIdType_e enID, HalSclHvspFilterSramSelType_e enSramSel, bool bC_SRAM, u8 *pData);
INTERFACE void HalSclHvspSetHVSPInputSize(DrvSclHvspIdType_e enID, u16 u16Width, u16 u16Height);
INTERFACE void HalSclHvspSetHVSPOutputSize(DrvSclHvspIdType_e enID, u16 u16Width, u16 u16Height);

// PatTg
INTERFACE void HalSclHvspSetPatTgEn(bool bEn);
INTERFACE void HalSclHvspSetPatTgVsyncSt(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgVsyncEnd(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgVfdeSt(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgVfdeEnd(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgVdeSt(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgVdeEnd(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgVtt(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgHsyncSt(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgHsyncEnd(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgHfdeSt(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgHfdeEnd(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgHdeSt(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgHdeEnd(u16 u16Val);
INTERFACE void HalSclHvspSetPatTgHtt(u16 u16Val);
INTERFACE void HalSclHvspSetTestPatCfg(void);
// REG W
INTERFACE void HalSclHvspSetReg(u32 u32Reg, u8 u8Val, u8 u8Mask);
INTERFACE void HalSclHvspSetRegisterForceByInst(u32 u32Reg, u8 u8Val, u8 u8Mask);

// CMD buffer
INTERFACE void HalSclHvspSetCmdqTrigCfg(DrvSclHvspCmdTrigConfig_t stCmdTrigCfg);
INTERFACE void HalSclHvspGetCmdqTrigCfg(DrvSclHvspCmdTrigConfig_t *pCfg);
INTERFACE void HalSclHvspMloadSramBufferPrepare(void);
INTERFACE void HalSclHvspMloadSramBufferFree(void);
INTERFACE void HalSclHvspSramDumpbyMload(DrvSclHvspIdType_e enID,u32 u32Sram,bool bHer,bool bchange);
INTERFACE void HalSclHvspSetCmdqTrigFire(void);
INTERFACE void HalSclHvspSetUcmHWrwDiff(u16 u8Val);
INTERFACE void HalSclHvspSetUcmMemConfig(HalSclUcmOnOffType_e enType);
INTERFACE void HalSclHvspSetCoverBypass(DrvSclHvspIdType_e enID,bool bEn);
INTERFACE u16  HalSclHvspGetHvspOutputHeight(DrvSclHvspIdType_e enID);
INTERFACE u16  HalSclHvspGetScalingFunctionStatus(DrvSclHvspIdType_e enID);
INTERFACE u16  HalSclHvspGetHvspInputHeight(DrvSclHvspIdType_e enID);
INTERFACE u16  HalSclHvspGetHvspInputWidth(DrvSclHvspIdType_e enID);
INTERFACE u16  HalSclHvspGetCropX(void);
INTERFACE u16  HalSclHvspGetHvspOutputWidth(DrvSclHvspIdType_e enID);
INTERFACE u16  HalSclHvspGetCropY(void);
INTERFACE u16  HalSclHvspGetCrop2Yinfo(void);
INTERFACE u16  HalSclHvspGetCrop2Xinfo(void);
INTERFACE u16 HalSclHvspGetCrop1WidthCount(void);
INTERFACE u16 HalSclHvspGetCrop1HeightCount(void);
INTERFACE u16 HalSclHvspGetCrop1Width(void);
INTERFACE u16 HalSclHvspGetCrop1Height(void);
INTERFACE u16 HalSclHvspGetCrop2InputWidth(void);
INTERFACE u16 HalSclHvspGetCrop2InputHeight(void);
INTERFACE u16 HalSclHvspGetCrop2OutputWidth(void);
INTERFACE u16 HalSclHvspGetCrop2OutputHeight(void);
INTERFACE u16 HalSclHvspGetCrop2En(void);
INTERFACE u16 HalSclHvspGetCrop1En(void);
INTERFACE bool HalSclHvspSramDump(HalSclHvspSramDumpType_e endump,u32 u32Sram);
INTERFACE void HalSclHvspVtrackEnable(u8 u8FrameRate, HalSclHvspVtrackEnableType_e bEnable);
INTERFACE void HalSclHvspVtrackSetUserDefindedSetting(bool bUserDefinded, u8 *pu8Setting);
INTERFACE void HalSclHvspVtrackSetKey(bool bUserDefinded, u8 *pu8Setting);
INTERFACE void HalSclHvspVtrackSetPayloadData(u16 u16Timecode, u8 u8OperatorID);
INTERFACE void HalSclHvspVtrackSetUUID(void);

#undef INTERFACE

#endif
