////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011 MStar Semiconductor, Inc.
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


//=============================================================================
#ifndef __DRV_PNL_H__
#define __DRV_PNL_H__

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define DRV_PNL_INSTANT_MAX             1
#define DRV_PNL_SIGNAL_CTRL_CH_MAX      5
//-------------------------------------------------------------------------------------------------
//  enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_DRV_PNL_LINK_NONE     = 0,
    E_DRV_PNL_LINK_LVDS     = 1,
    E_DRV_PNL_LINK_MIPI_DSI = 2,
    E_DRV_PNL_LINK_TTL      = 3,
    E_DRV_PNL_LINK_VBY1     = 4,
    E_DRV_PNL_LINK_MAX,
}DrvPnlLinkType_e;
/// @endcond


typedef enum
{
    E_DRV_PNL_MIPI_DSI_RGB565,
    E_DRV_PNL_MIPI_DSI_RGB666,
    E_DRV_PNL_MIPI_DSI_LOOSELY_RGB666,
    E_DRV_PNL_MIPI_DSI_RGB888,
}DrvPnlMipiDsiFormat_e;

typedef enum
{
    E_DRV_PNL_MIPI_DSI_CMD_MODE   = 0,
    E_DRV_PNL_MIPI_DSI_SYNC_PULSE = 1,
    E_DRV_PNL_MIPI_DSI_SYNC_EVENT = 2,
    E_DRV_PNL_MIPI_DSI_BURST_MODE = 3,
}DrvPnlMipiDsiCtrlMode_e;


typedef enum
{
    E_DRV_PNL_MIPI_DSI_LANE_NONE = 0,
    E_DRV_PNL_MIPI_DSI_LANE_1    = 1,
    E_DRV_PNL_MIPI_DSI_LANE_2    = 2,
    E_DRV_PNL_MIPI_DSI_LANE_3    = 3,
    E_DRV_PNL_MIPI_DSI_LANE_4    = 4,
}DrvPnlMipiDsiLaneNum_e;

typedef enum
{
    E_DRV_PNL_TI_BIT_10 = 0x00,
    E_DRV_PNL_TI_BIT_8  = 0x02,
    E_DRV_PNL_TI_BIT_6  = 0x03,
}DrvPnlTiBitMode_e;


typedef enum
{
    E_DRV_PNL_CH_SWAP_0 = 0x00,
    E_DRV_PNL_CH_SWAP_1 = 0x01,
    E_DRV_PNL_CH_SWAP_2 = 0x02,
    E_DRV_PNL_CH_SWAP_3 = 0x03,
    E_DRV_PNL_CH_SWAP_4 = 0x04,
}DrvPnlChannelSwapType_e;

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
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

    u16 u16Hactive;
    u16 u16Hpw;
    u16 u16Hbp;
    u16 u16Hfp;

    u16 u16Vactive;
    u16 u16Vpw;
    u16 u16Vbp;
    u16 u16Vfp;

    u16 u16Bllp;
    u16 u16Fps;

    DrvPnlMipiDsiLaneNum_e enLaneNum;
    DrvPnlMipiDsiFormat_e enformat;
    DrvPnlMipiDsiCtrlMode_e enCtrl;

    u8  *pu8CmdBuf;
    u32 u32CmdBufSize;
}DrvPnlMipiDsiConfig_t;

typedef struct
{
    DrvPnlLinkType_e enLinkType;              ///< panel type
    u16 u16Htt;
    u16 u16Hpw;
    u16 u16Hbp;
    u16 u16Hactive;
    u16 u16Hstart;
    u16 u16Vtt;
    u16 u16Vpw;
    u16 u16Vbp;
    u16 u16Vactive;
    u16 u16Vstart;
    u16 u16Fps;
}DrvPnlTimingConfig_t;

typedef struct
{
    DrvPnlLinkType_e enLinkType;
    bool bEn;
    u16  u16Step;
    u16  u16Span;
}DrvPnlSscConfig_t;


typedef struct
{
    DrvPnlLinkType_e enLinkType;
    bool bEn;
    u16  u16R;
    u16  u16G;
    u16  u16B;
}DrvPnlTestPatternConfig_t;

typedef struct
{
    DrvPnlLinkType_e enLinkType;
    bool bEn;
    u16 u16Htt;
    u16 u16Hpw;
    u16 u16Hbp;
    u16 u16Hactive;
    u16 u16Vtt;
    u16 u16Vpw;
    u16 u16Vbp;
    u16 u16Vactive;
}DrvPnlTestTgenConfig_t;

typedef struct
{
    bool bTiMode;
    DrvPnlTiBitMode_e enTiBitMode;
    DrvPnlChannelSwapType_e enCh[DRV_PNL_SIGNAL_CTRL_CH_MAX];
    bool bChPolarity;
    bool bHsyncInvert;
    bool bVsyncInvert;
    bool bDeInvert;
}DrvPnlModConfig_t;

typedef struct
{
    DrvPnlLinkType_e enLinkType;
    DrvPnlModConfig_t stModCfg;
    DrvPnlMipiDsiConfig_t stMipiDsiCfg;
}DrvPnlSignalCtrlConfig_t;

typedef struct
{
    DrvPnlLinkType_e enLinkType;
    bool bEn;
    u16  u16Level;
}DrvPnlBackLightConfig_t;

typedef struct
{
    DrvPnlLinkType_e enLinkType;
    bool bEn;
}DrvPnlPowerConfig_t;

typedef struct
{
    DrvPnlLinkType_e enLinkType;
    u16 u16Val;
}DrvPnlDrvCurrentConfig_t;
//------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifndef __DRV_PNL_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE bool DrvPnlInit(void);
INTERFACE void *DrvPnlCtxAllocate(void);
INTERFACE bool DrvPnlCtxFree(void *);
INTERFACE bool DrvPnlSetTimingConfig(void *pCtx, DrvPnlTimingConfig_t *pParamCfg);
INTERFACE bool DrvPnlSetSignalCtrlConfig(void *pCtx, DrvPnlSignalCtrlConfig_t *pSignalCtrlCfg);
INTERFACE bool DrvPnlSetSscConfig(void *pCtx, DrvPnlSscConfig_t *pSscCfg);
INTERFACE bool DrvPnlSetTestPatternConfig(void *pCtx, DrvPnlTestPatternConfig_t *pTestPatCfg);
INTERFACE bool DrvPnlSetTestTgenConfig(void *pCtx, DrvPnlTestTgenConfig_t *pTgenCfg);
INTERFACE bool DrvPnlSetBackLightConfig(void *pCtx,  DrvPnlBackLightConfig_t *pBackLightCfg);
INTERFACE bool DrvPnlSetPowerConfig(void *pCtx, DrvPnlPowerConfig_t *pPowerCfg);
INTERFACE bool DrvPnlGetBackLightConfig(void *pCtx,  DrvPnlBackLightConfig_t *pBackLightCfg);
INTERFACE bool DrvPnlGetPowerConfig(void *pCtx, DrvPnlPowerConfig_t *pPowerCfg);
INTERFACE bool DrvPnlSetDrvCurrentConfig(void *pCtx, DrvPnlDrvCurrentConfig_t *pDrvCurrentCfg);
INTERFACE bool DrvPnlSetDbgLevelConfig(void *pCtx, u32 u32Level);

#undef INTERFACE

#endif
