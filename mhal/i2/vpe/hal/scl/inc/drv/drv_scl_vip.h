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
#ifndef _DRV_VIP_H
#define _DRV_VIP_H
//=============================================================================
// Defines
//=============================================================================
//
#define DRV_SCLVIP_ROI_WINDOW_MAX 4
#define DRV_SCLVIP_WDR_HIST_BUFFER 4
#define VIP_DLC_HISTOGRAM_SECTION_NUM   7
#define VIP_DLC_HISTOGRAM_REPORT_NUM    8
#define VIP_NLM_ENTRY_NUM 1104
#define VIP_NLM_AUTODOWNLOAD_BASE_UNIT 16
#define VIP_NLM_AUTODOWNLOAD_CLIENT 9
#define DRV_VIP_MUTEX_LOCK()            //DrvSclOsObtainMutex(_VIP_Mutex,SCLOS_WAIT_FOREVER)
#define DRV_VIP_MUTEX_UNLOCK()          //DrvSclOsReleaseMutex(_VIP_Mutex)
//
//
typedef enum
{
    E_DRV_SCLVIP_ACK_CONFIG           = 0x1,      ///< ACK
    E_DRV_SCLVIP_IBC_CONFIG           = 0x2,      ///< IBC
    E_DRV_SCLVIP_IHCICC_CONFIG        = 0x4,      ///< ICCIHC
    E_DRV_SCLVIP_ICC_CONFIG           = 0x8,      ///< ICE
    E_DRV_SCLVIP_IHC_CONFIG           = 0x10,     ///< IHC
    E_DRV_SCLVIP_FCC_CONFIG           = 0x20,     ///< FCC
    E_DRV_SCLVIP_UVC_CONFIG           = 0x40,     ///< UVC
    E_DRV_SCLVIP_DLC_HISTOGRAM_CONFIG = 0x80,    ///< HIST
    E_DRV_SCLVIP_DLC_CONFIG           = 0x100,     ///< DLC
    E_DRV_SCLVIP_LCE_CONFIG           = 0x200,    ///< LCE
    E_DRV_SCLVIP_PEAKING_CONFIG       = 0x400,    ///< PK
    E_DRV_SCLVIP_NLM_CONFIG           = 0x800,    ///< NLM
    E_DRV_SCLVIP_CONFIG               = 0x10000,  ///< 19 bit to control 19 IOCTL
    E_DRV_SCLVIP_MCNR_CONFIG          = 0x20000,  ///< MCNR
}E_DRV_SCLVIP_CONFIG_TYPE;
typedef enum
{
    E_DRV_SCLVIP_AIP_SRAM_GAMMA_Y = 0, ///< gamma y
    E_DRV_SCLVIP_AIP_SRAM_GAMMA_U, ///< gamma u
    E_DRV_SCLVIP_AIP_SRAM_GAMMA_V, ///< gamma v
    E_DRV_SCLVIP_AIP_SRAM_GM10to12_R, ///< gamma R
    E_DRV_SCLVIP_AIP_SRAM_GM10to12_G, ///< gamma G
    E_DRV_SCLVIP_AIP_SRAM_GM10to12_B, ///< gamma B
    E_DRV_SCLVIP_AIP_SRAM_GM12to10_R, ///< gamma R
    E_DRV_SCLVIP_AIP_SRAM_GM12to10_G, ///< gamma G
    E_DRV_SCLVIP_AIP_SRAM_GM12to10_B, ///< gamma B
    E_DRV_SCLVIP_AIP_SRAM_WDR, ///< wdr
    E_DRV_SCLVIP_AIP_SRAM_WDR_TBL, ///< wdr
    E_DRV_SCLVIP_AIP_SRAM_NUM, ///< wdr
}DrvSclVipSramType_e;
typedef enum
{
    E_DRV_SCLVIP_ROISRC_BEFORE_WDR,      //
    E_DRV_SCLVIP_ROISRC_AFTER_WDR,      //
    E_DRV_SCLVIP_ROISRC_WDR,        //
    E_DRV_SCLVIP_ROISRC_TYPE,        //
}DrvSclVipWdrRoiSrcType_e;
typedef enum
{
    E_DRV_SCLVIP_MASK_WDR,      //
    E_DRV_SCLVIP_MASK_NR,      //
    E_DRV_SCLVIP_MASK_TYPE,      //
}DrvSclVipMaskType_e;
typedef enum
{
    E_DRV_SCLVIP_UCMCE_OFF,      //
    E_DRV_SCLVIP_UCMCE_8,      //
    E_DRV_SCLVIP_UCMCE_6,      //
    E_DRV_SCLVIP_UCMCE_TYPE,      //
}DrvSclVipUCMCEType_e;
typedef enum
{
    E_DRV_SCLVIP_AIP_YEE = 0,           ///< yee
    E_DRV_SCLVIP_AIP_YEE_AC_LUT,        ///< yee ac lut//1
    E_DRV_SCLVIP_AIP_MXNR,              ///< mxnr
    E_DRV_SCLVIP_AIP_UVADJ,             ///< uvadj
    E_DRV_SCLVIP_AIP_XNR,               ///< xnr
    E_DRV_SCLVIP_AIP_PFC,               ///< pfc
    E_DRV_SCLVIP_AIP_YCUVM,             ///< ycuvm
    E_DRV_SCLVIP_AIP_COLORTRAN,         ///< ct
    E_DRV_SCLVIP_AIP_GAMMA,             ///< gamma
    E_DRV_SCLVIP_AIP_422TO444,          ///< 422to444 //10
    E_DRV_SCLVIP_AIP_YUVTORGB,          ///< yuv2rgb
    E_DRV_SCLVIP_AIP_GM10TO12,          ///< 10 to 12
    E_DRV_SCLVIP_AIP_CCM,               ///< ccm
    E_DRV_SCLVIP_AIP_HSV,               ///< hsv
    E_DRV_SCLVIP_AIP_GM12TO10,          ///< gm12to10//15
    E_DRV_SCLVIP_AIP_WDR,               ///< wdr
    E_DRV_SCLVIP_AIP_VD,                ///< vd
    E_DRV_SCLVIP_AIP_ARBSHP,            ///< arbshp
    E_DRV_SCLVIP_AIP_RGBTOYUV,          ///< rgb2yuv
    E_DRV_SCLVIP_AIP_444TO422,          ///< 4442422
    E_DRV_SCLVIP_AIP_FORVPECFG,         ///< For Vpe Cfg
    E_DRV_SCLVIP_AIP_FORVPEONOFF,       ///< For Vpe OnoOff
    E_DRV_SCLVIP_AIP_FORVPEPROCESS,     ///< For Vpe OnoOff
    E_DRV_SCLVIP_AIP_FORVPEPROCESSNE,   ///< For Vpe OnoOff
    E_DRV_SCLVIP_AIP_NUM,               ///< Num
}DrvSclVipAipType_e;

//-------------------------------------------------------------------------------------------------
//  Defines & Structure
//-------------------------------------------------------------------------------------------------
// IOCTL_VIP_SET_DNR_CONFIG
typedef struct
{
    bool bEn;
    u8 u8framecnt;
}DrvSclVipFcConfig_t;
typedef struct
{
    u32 u32RiuBase;
    void *pvCtx;
}DrvSclVipInitConfig_t;

typedef struct
{
    u32  u32baseadr;
    u16  u16depth;
    u16  u16reqlen;
    bool bCLientEn;
    bool btrigContinue;
    u16  u16iniaddr;
    u32  u32viradr;
}DrvSclVipNlmSramConfig_t;


typedef struct
{
    s32 s32Y[DRV_SCLVIP_ROI_WINDOW_MAX];
    s32 s32R[DRV_SCLVIP_ROI_WINDOW_MAX];
    s32 s32G[DRV_SCLVIP_ROI_WINDOW_MAX];
    s32 s32B[DRV_SCLVIP_ROI_WINDOW_MAX];
    s32 s32YCnt[DRV_SCLVIP_ROI_WINDOW_MAX];
}DrvSclVipWdrRoiReport_t;
typedef struct
{
    bool bEnSkip;
    u16 u16RoiAccX[4];
    u16 u16RoiAccY[4];
}DrvSclVipWdrRoiConfig_t;
typedef struct
{
    bool bEn;
    u8 u8WinCount;
    DrvSclVipWdrRoiSrcType_e enPipeSrc;
    DrvSclVipWdrRoiConfig_t stRoiCfg[DRV_SCLVIP_ROI_WINDOW_MAX];
    u32 u32BaseAddr[DRV_SCLVIP_ROI_WINDOW_MAX];
}DrvSclVipWdrRoiHist_t;
typedef struct
{
    bool bYCMRead;      // DNR IP can Read from Buffer (if can read then DNR availability)
    bool bYCMWrite;     // DNR IP can Write to Buffer
    u32  u32YCBaseAddr;// YC Buffer phycal address(unit:Byte)
    DrvSclVipUCMCEType_e enCeType;
    u32 u32MemSize;
    void *pvCtx;
}DrvSclVipIpmConfig_t;
typedef struct
{
    bool bOnOff;                     ///< Base Address of AIP SRAM setting
    DrvSclVipMaskType_e enMaskType; ///< Mask type
}DrvSclVipSetMaskOnOff_t;

typedef struct
{
    bool bVariable_Section;//1E04
    bool bstatic; //1E04
    bool bcurve_fit_en;//1E04
    bool bhis_y_rgb_mode_en;//1E04
    bool bcurve_fit_rgb_en;//1E04
    bool bDLCdither_en;//1E04
    u8 u8Histogram_Range[VIP_DLC_HISTOGRAM_SECTION_NUM];
    bool bstat_MIU;
    bool bRange;
    u16 u16Vst;
    u16 u16Vnd;
    u16 u16Hst;
    u16 u16Hnd;
    u8 u8HistSft;
    u8 u8trig_ref_mode;
    u32 u32StatBase[2];
}ST_VIP_DLC_HISTOGRAM_CONFIG;

//IOCTL_SCLVIP_GET_DLC_HISTOGRAM_REPORT
typedef struct
{
    u32 u32Histogram[VIP_DLC_HISTOGRAM_REPORT_NUM];
    u32 u32PixelWeight;
    u32 u32PixelCount;
    u8 u8MinPixel;
    u8 u8MaxPixel;
    u8 u8Baseidx;
}DrvSclVipDlcHistogramReport_t;

//IOCTL_VIP_SET_FCC_ADP_YWIN_CONFIG


typedef struct
{
    DrvSclVipFcConfig_t stFCfg;
    u32 u32Addr;
    u16 u16Data;
    u16 u16Mask;
    u8 u8framecnt;
    bool bfire;
    bool bCnt;
}DrvSclVipCmdqConfig_t;
typedef struct
{
    void *gpvSRAMBuffer[E_DRV_SCLVIP_AIP_SRAM_NUM];
}MDrvSclCtxVipGlobalSet_t;


//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifndef DRV_VIP_C
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE void DrvSclVipGetDlcHistogramConfig(DrvSclVipDlcHistogramReport_t *stdlc);
INTERFACE bool DrvSclVipInit(DrvSclVipInitConfig_t *pCfg);
INTERFACE void DrvSclVipExit(void);
INTERFACE bool DrvSclVipSetDlcHistogramConfig(ST_VIP_DLC_HISTOGRAM_CONFIG *stDLCCfg,bool bEn);
INTERFACE u32  DrvSclVipGetDlcHistogramReport(u16 u16range);
INTERFACE void DrvSclVipHWInit(void);
INTERFACE bool DrvSclVipSetIPMConfig(DrvSclVipIpmConfig_t *stCfg);
INTERFACE void DrvSclVipHwReset(void);
INTERFACE void DrvSclVipSetWdrMloadConfig(bool bEn,u32 u32WdrBuf);
INTERFACE bool DrvSclVipGetWdrOnOff(void);
INTERFACE bool DrvSclVipGetNRHistogram(void *pvCfg);
INTERFACE bool DrvSclVipGetWdrHistogram(DrvSclVipWdrRoiReport_t *pvCfg);
INTERFACE bool DrvSclVipSetMaskOnOff(DrvSclVipSetMaskOnOff_t *pvCfg);
INTERFACE bool DrvSclVipSetRoiConfig(DrvSclVipWdrRoiHist_t *pvCfg);
INTERFACE bool DrvSclVipGetRoiConfig(u8 u8Win,DrvSclVipWdrRoiConfig_t *pvCfg);
INTERFACE void DrvSclVipSetWdrMloadBuffer(u32 u32Buffer);
INTERFACE void DrvSclVipSetNrHistOnOff(bool bEn);
INTERFACE void * DrvSclVipSetAipSramConfig(void * pvPQSetParameter, DrvSclVipSramType_e enAIPType);
INTERFACE void DrvSclVipDnrInit(void);
INTERFACE void DrvSclVipPkInit(void);
INTERFACE void DrvSclVipDlcInit(void);
INTERFACE void DrvSclVipUvcInit(void);
INTERFACE void DrvSclVipIceInit(void);
INTERFACE void DrvSclVipIhcInit(void);
INTERFACE void DrvSclVipFccInit(void);
INTERFACE void DrvSclVipLceInit(void);
INTERFACE bool DrvSclVipSetNlmSramConfig(DrvSclVipNlmSramConfig_t *stCfg);
INTERFACE void DrvSclVipSramDump(void);
INTERFACE u16 DrvSclVipGetBypassStatus(E_DRV_SCLVIP_CONFIG_TYPE enIPType);
INTERFACE u16 DrvSclVipGetAIPStatus(DrvSclVipAipType_e enIPType);
INTERFACE void DrvSclVipCheckVipSram(u32 u32Type);

#undef INTERFACE

#endif
