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
#ifndef __HAL_PNL_H__
#define __HAL_PNL_H__


//-------------------------------------------------------------------------------------------------
//  Defines & ENUM
//-------------------------------------------------------------------------------------------------
#define     HAL_PNL_MOD_CH_MAX   5


#define     HAL_PNL_MIPI_DSI_FLAG_DELAY                         0xFE
#define     HAL_PNL_MIPI_DSI_FLAG_END_OF_TABLE                  0xFF   // END OF REGISTERS MARKER


#define     HAL_PNL_MIPI_DSI_DCS_MAXIMUM_RETURN_PACKET_SIZE	    0x37
#define     HAL_PNL_MIPI_DSI_DCS_SHORT_PACKET_ID_0              0x05
#define     HAL_PNL_MIPI_DSI_DCS_SHORT_PACKET_ID_1              0x15
#define     HAL_PNL_MIPI_DSI_DCS_LONG_WRITE_PACKET_ID	        0x39
#define     HAL_PNL_MIPI_DSI_DCS_READ_PACKET_ID                 0x06

#define     HAL_PNL_MIPI_DSI_GERNERIC_SHORT_PACKET_ID_0         0x03
#define     HAL_PNL_MIPI_DSI_GERNERIC_SHORT_PACKET_ID_1         0x13
#define     HAL_PNL_MIPI_DSI_GERNERIC_SHORT_PACKET_ID_2         0x23
#define     HAL_PNL_MIPI_DSI_GERNERIC_LONG_WRITE_PACKET_ID	    0x29
#define     HAL_PNL_MIPI_DSI_GERNERIC_READ_0_PARAM_PACKET_ID    0x04
#define     HAL_PNL_MIPI_DSI_GERNERIC_READ_1_PARAM_PACKET_ID    0x14
#define     HAL_PNL_MIPI_DSI_GERNERIC_READ_2_PARAM_PACKET_ID    0x24


typedef enum
{
    E_HAL_PNL_LINK_LVDS     = 0,
    E_HAL_PNL_LINK_MIPI_DSI = 1,
    E_HAL_PNL_LINK_TTL      = 2,
    E_HAL_PNL_LINK_VBY1     = 3,
    E_HAL_PNL_LINK_MAX,
}HalPnlLinkType_e;

typedef enum
{
    E_HAL_PNL_MOD_TI_BIT_10 = 0x00,
    E_HAL_PNL_MOD_TI_BIT_8  = 0x02,
    E_HAL_PNL_MOD_TI_BIT_6  = 0x03,
}HalPnlModTiBitMode_e;

typedef enum
{
    E_HAL_PNL_MOD_CH_SWAP_0 = 0x00,
    E_HAL_PNL_MOD_CH_SWAP_1 = 0x01,
    E_HAL_PNL_MOD_CH_SWAP_2 = 0x02,
    E_HAL_PNL_MOD_CH_SWAP_3 = 0x03,
    E_HAL_PNL_MOD_CH_SWAP_4 = 0x04,
}HalPnlModChannelSwapType_e;


typedef enum
{
    E_HAL_PNL_MIPI_DSI_RGB565         = 0,
    E_HAL_PNL_MIPI_DSI_RGB666         = 1,
    E_HAL_PNL_MIPI_DSI_LOOSELY_RGB666 = 2,
    E_HAL_PNL_MIPI_DSI_RGB888         = 3,
}HalPnlMipiDsiFormat_e;

typedef enum
{
    E_HAL_PNL_MIPI_DSI_CMD_MODE   = 0,
    E_HAL_PNL_MIPI_DSI_SYNC_PULSE = 1,
    E_HAL_PNL_MIPI_DSI_SYNC_EVENT = 2,
    E_HAL_PNL_MIPI_DSI_BURST_MODE = 3,
}HalPnlMipiDsiCtrlMode_e;


typedef enum
{
    E_HAL_PNL_MIPI_DSI_LANE_NONE = 0,
    E_HAL_PNL_MIPI_DSI_LANE_1    = 1,
    E_HAL_PNL_MIPI_DSI_LANE_2    = 2,
    E_HAL_PNL_MIPI_DSI_LANE_3    = 3,
    E_HAL_PNL_MIPI_DSI_LANE_4    = 4,
}HalPnlMipiDsiLaneMode_e;


//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    HalPnlLinkType_e enLinkType;
    HalPnlMipiDsiFormat_e enMipiDsiFmt;
    HalPnlMipiDsiLaneMode_e enMipiDsiLane;
    u16 u16Htotal;
    u16 u16Vtotal;
    u16 u16Hactive;
    u16 u16Vactive;
    u16 u16Fps;
}HalPnlLpllConfig_t;

typedef struct
{
    bool bEn;
    u16  u16R;
    u16  u16G;
    u16  u16B;
}HalPnlModTestPatternConfig_t;

typedef struct
{
    u16  u16Htt;
    u16  u16Hpw;
    u16  u16Hbp;
    u16  u16Hactive;
    u16  u16Hstart;
    u16  u16Vtt;
    u16  u16Vpw;
    u16  u16Vbp;
    u16  u16Vactive;
    u16  u16Vstart;
}HalPnlTimingConfig_t;

typedef struct
{
    bool bEn;
    HalPnlTimingConfig_t stTimingCfg;
}HalPnlModTgenConfig_t;

typedef struct
{
    bool bEn;
    u16  u16Step;
    u16  u16Span;
}HalPnlLpllSscConfig_t;



typedef struct
{
    bool bTiMode;
    HalPnlModTiBitMode_e enTiBitMode;
    HalPnlModChannelSwapType_e enCh[HAL_PNL_MOD_CH_MAX];
    bool bChPolarity;
    bool bHsyncInvert;
    bool bVsyncInvert;
    bool bDeInvert;
}HalPnlModConfig_t;

typedef struct
{
    bool bEn;
}HalPnlPowerConfig_t;

typedef struct
{
    bool bEn;
    u16  u16Level;
}HalPnlBackLightConfig_t;

typedef struct
{
    u16 u16Val;
}HalPnlDrvCurrentConfig_t;


typedef struct
{
    u8      u8Type    : 2;
    u8      u8Bta     : 1;
    u8      u8Hs      : 1;
    u8      u8Cl      : 1;
    u8      u8Te      : 1;
    u8      u8Rsv     : 1;
    u8      u8Rpt     : 1;
} HalPnlMipiDsiCmdqConfig_t;

//Type0 Used for DSI short packet read/write command
typedef struct
{
    u8 u8Confg;
    u8 u8DataId;
    u8 u8Data0;
    u8 u8Data1;
} HalPnlMipiDsiT0Ins_t;


//Type2 Used for DSI generic long packet write command
typedef struct
{
    u8  u8Confg      :8;
    u8  u8DataId     :8;
    u16 u16Wc        :16;
    //u8 *pu8data;
} HalPnlMipiDsiT2Ins_t;

//Type3 Used for DSI frame buffer read command (short packet)
typedef struct
{
    u8 u8Confg       : 8;
    u8 u8DataId      : 8;
    u8 u8MemStart0   : 8;
    u8 u8MemStart1   : 8;
} HalPnlMipiDsiT3Ins_t;



typedef struct
{
    u8 byte0;
    u8 byte1;
    u8 byte2;
    u8 byte3;
} HalPnlMipiDsiCmdq_t;

typedef struct
{
    HalPnlMipiDsiCmdq_t data[32];
} HalPnlMipiDsiCmdqRegs_t;

typedef struct
{
    u8 u8Lpx;
    u8 u8HsPrpr;
    u8 u8HsZero;
    u8 u8HsTrail;
} HalPnlMipiDsiPhyTimCon0Reg_t;


typedef struct
{
    u8 u8TaGo;
    u8 u8TaSure;
    u8 u8TaGet;
    u8 u8DaHsExit;
} HalPnlMipiDsiPhyTimCon1Reg_t;


typedef struct
{
    u8 u8ContDet;
    u8 RSV8;
    u8 u8ClkZero;
    u8 u8ClkTrail;
} HalPnlMipiDsiPhyTimCon2Reg_t;


typedef struct
{
    u8 u8ClkHsPrpr;
    u8 u8ClkHsPost;
    u8 u8ClkHsExit;
    u8 u8rsv24 : 8;
} HalPnlMipiDsiPhyTimCon3Reg_t;


typedef struct
{
    u8 u8HsTrail;
    u8 u8HsPrpr;
    u8 u8HsZero;
    u8 u8ClkHsPrpr;
    u8 u8ClkHsExit;
    u8 u8ClkTrail;
    u8 u8ClkZero;
    u8 u8ClkHsPost;
    u8 u8DaHsExit;
    u8 u8ContDet;

    u8 u8Lpx;
    u8 u8TaGet;
    u8 u8TaSure;
    u8 u8TaGo;

    u32 u32HActive;
    u32 u32HPW;
    u32 u32HBP;
    u32 u32HFP;
    u32 u32VActive;
    u32 u32VPW;
    u32 u32VBP;
    u32 u32VFP;
    u32 u32BLLP; // only for burst mode

    HalPnlMipiDsiLaneMode_e enLaneNum;
    HalPnlMipiDsiFormat_e enFormat;
    HalPnlMipiDsiCtrlMode_e enCtrl;
} HalPnlMipiDsiConfig_t;


//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifndef __HAL_PNL_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif


INTERFACE void HalPnlInitClk(void);
INTERFACE void HalPnlInitLvdsDphy(void);
INTERFACE void HalPnlSetLpllConfig(HalPnlLpllConfig_t *pLpllCfg);
INTERFACE void HalPnlSetModConfig(HalPnlModConfig_t *pModCfg);
INTERFACE void HalPnlSetModeTestPattern(HalPnlModTestPatternConfig_t *pTestPatCfg);
INTERFACE void HalPnlSetModTgenConfig(HalPnlModTgenConfig_t *pTgenCfg);
INTERFACE void HalPnlSetLpllSscConfig(HalPnlLpllSscConfig_t *pSscCfg);
INTERFACE void HalPnlSetTimingConfig(HalPnlTimingConfig_t *pTimingCfg);
INTERFACE void HalPnlSetBackLightConfig(HalPnlBackLightConfig_t *pBackLightCfg);
INTERFACE void HalPnlSetPowerConfig(HalPnlPowerConfig_t *pPowerCfg);
INTERFACE void HalPnlSetDrvCurrentConfig(HalPnlDrvCurrentConfig_t *pDrvCurCfg);

INTERFACE void HalPnlInitMipiDsiDphy(void);
INTERFACE void HalPnlSetMipiDsiPadOutSel(HalPnlMipiDsiLaneMode_e enLaneMode);
INTERFACE void HalPnlResetMipiDsi(void);
INTERFACE void HalPnlEnableMipiDsiClk(void);
INTERFACE void HalPnlDisableMipiDsiClk(void);
INTERFACE void HalPnlSetMipiDsiLaneNum(HalPnlMipiDsiLaneMode_e enLaneMode);
INTERFACE void HalPnlSetMipiDsiCtrlMode(HalPnlMipiDsiCtrlMode_e enCtrlMode);
INTERFACE bool HalPnlGetMipiDsiClkHsMode(void);
INTERFACE void HalPnlSetMpiDsiClkHsMode(bool bEn);
INTERFACE bool HalPnlSetMipiDsiShortPacket(u8 u8Count, u8 u8Cmd, u8 *pParamList);
INTERFACE bool HalPnlSetMipiDsiLongPacket(u8 u8Count, u8 u8Cmd, u8 *pu8ParamList);
INTERFACE bool HalPnlSetMipiDsiPhyTimConfig(HalPnlMipiDsiConfig_t *pstMipiDsiCfg);
INTERFACE bool HalPnlSetMipiDsiVdoTimingConfig(HalPnlMipiDsiConfig_t *pstMipiDsiCfg);
INTERFACE void HalPnlSetMipiDsiMisc(void);

#undef INTERFACE

#endif
