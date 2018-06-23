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

/**
 *  @file drv_scl_hvsp_io_st.h
 *  @brief PNL Driver struct parameter interface
 */

/**
* \ingroup PNL_group
* @{
*/

#ifndef __DRV_PNL_IO_ST_H__
#define __DRV_PNL_IO_ST_H__

//=============================================================================
// Defines
//=============================================================================
#define DRV_PNL_VERSION                        0x0100


#define DRV_PNL_IO_SIGNAL_CTRL_CH_MAX          5

//=============================================================================
// enum
//=============================================================================

///@cond
/**
* The error type of SclHvsp
*/
typedef enum
{
    E_DRV_PNL_IO_ERR_OK    =  0, ///< No Error
    E_DRV_PNL_IO_ERR_FAULT = -1, ///< Fault
    E_DRV_PNL_IO_ERR_INVAL = -2, ///< Invalid value
    E_DRV_PNL_IO_ERR_MULTI = -3, ///< MultiInstance Fault
}DrvPnlIoErrType_e;
/// @endcond

///@cond
/**
* The panel type of PNL
*/
typedef enum
{
    E_DRV_PNL_IO_LINK_NONE     = 0,
    E_DRV_PNL_IO_LINK_LVDS     = 1,
    E_DRV_PNL_IO_LINK_MIPI_DSI = 2,
    E_DRV_PNL_IO_LINK_TTL      = 3,
    E_DRV_PNL_IO_LINK_VBY1     = 4,
    E_DRV_PNL_IO_LINK_MAX,
}DrvPnlIoLinkType_e;
/// @endcond


typedef enum
{
    E_DRV_PNL_IO_MIPI_DSI_RGB565,
    E_DRV_PNL_IO_MIPI_DSI_RGB666,
    E_DRV_PNL_IO_MIPI_DSI_LOOSELY_RGB666,
    E_DRV_PNL_IO_MIPI_DSI_RGB888,
}DrvPnlIoMipiDsiFormat_e;

typedef enum
{
    E_DRV_PNL_IO_MIPI_DSI_CMD_MODE   = 0,
    E_DRV_PNL_IO_MIPI_DSI_SYNC_PULSE = 1,
    E_DRV_PNL_IO_MIPI_DSI_SYNC_EVENT = 2,
    E_DRV_PNL_IO_MIPI_DSI_BURST_MODE = 3,
}DrvPnlIoMipiDsiCtrlMode_e;


typedef enum
{
    E_DRV_PNL_IO_MIPI_DSI_LANE_NONE = 0,
    E_DRV_PNL_IO_MIPI_DSI_LANE_1    = 1,
    E_DRV_PNL_IO_MIPI_DSI_LANE_2    = 2,
    E_DRV_PNL_IO_MIPI_DSI_LANE_3    = 3,
    E_DRV_PNL_IO_MIPI_DSI_LANE_4    = 4,
}DrvPnlIoMipiDsiLaneNum_e;

typedef enum
{
    E_DRV_PNL_IO_TI_BIT_10 = 0x00,
    E_DRV_PNL_IO_TI_BIT_8  = 0x02,
    E_DRV_PNL_IO_TI_BIT_6  = 0x03,
}DrvPnlIoTiBitMode_e;


typedef enum
{
    E_DRV_PNL_IO_CH_SWAP_0 = 0x00,
    E_DRV_PNL_IO_CH_SWAP_1 = 0x01,
    E_DRV_PNL_IO_CH_SWAP_2 = 0x02,
    E_DRV_PNL_IO_CH_SWAP_3 = 0x03,
    E_DRV_PNL_IO_CH_SWAP_4 = 0x04,
}DrvPnlIoChannelSwapType_e;


//=============================================================================
// struct
//=============================================================================
/**
*  The Version of PNL
*/
typedef struct
{
    u32   VerChk_Version ; ///< structure version
    u32   u32Version;      ///< version
    u32   VerChk_Size;     ///< structure size for version checking
} __attribute__ ((__packed__))DrvPnlIoVersionConfig_t;



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

    DrvPnlIoMipiDsiLaneNum_e enLaneNum;
    DrvPnlIoMipiDsiFormat_e enformat;
    DrvPnlIoMipiDsiCtrlMode_e enCtrl;

    u8   *pu8CmdBuf;
    u32  u32CmdBufSize;
}__attribute__ ((__packed__)) DrvPnlIoMipiDsiConfig_t;

typedef struct
{
    u32   VerChk_Version ;              ///< structure version
    DrvPnlIoLinkType_e  enLinkType;     ///< panel type
    u16 u16Htt;                         ///< Horizontal Total
    u16 u16Hpw;                         ///< Horizontal Sync Width
    u16 u16Hbp;                         ///< Horizontal back porch
    u16 u16Hactive;                     ///< Horizontal  Width
    u16 u16Hstart;                      ///< Horizontal DE Start
    u16 u16Vtt;                         ///< Vertical Total
    u16 u16Vpw;                         ///< Vertical Sync Width
    u16 u16Vbp;                         ///< Verttical back porch
    u16 u16Vactive;                     ///< Vertical Width
    u16 u16Vstart;                      ///< Vertical DE Start
    u16 u16Fps;                         ///< Frame Rate
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;                                  ///< structure size for version checking
}__attribute__ ((__packed__)) DrvPnlIoTimingConfig_t;


typedef struct
{
    u32   VerChk_Version ;                              ///< structure version
    DrvPnlIoLinkType_e  enLinkType;                     ///< panel type
    bool  bEn;                                          ///< Enable SSC
    u16   u16Step;                                      ///< Step of SSC
    u16   u16Span;                                      ///< Span of SSC
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;                                  ///< structure size for version checking
}__attribute__ ((__packed__)) DrvPnlIoSscConfig_t;

typedef struct
{
    u32   VerChk_Version ;                              ///< structure version
    DrvPnlIoLinkType_e  enLinkType;                     ///< panel type
    bool  bEn;                                          ///< Enable Test Pattern
    u16   u16R;                                         ///< color R
    u16   u16G;                                         ///< color G
    u16   u16B;                                         ///< color B
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;                                  ///< structure size for version checking
}__attribute__ ((__packed__)) DrvPnlIoTestPatternConfig_t;


typedef struct
{
    u32   VerChk_Version ;              ///< structure version
    DrvPnlIoLinkType_e  enLinkType;     ///< panel type
    bool  bEn;                          ///< Enable TGen
    u16 u16Htt;                         ///< Horizontal Total
    u16 u16Hpw;                         ///< Horizontal Sync Width
    u16 u16Hbp;                         ///< Horizontal back porch
    u16 u16Hactive;                     ///< Horizontal  Width
    u16 u16Vtt;                         ///< Vertical Total
    u16 u16Vpw;                         ///< Vertical Sync Width
    u16 u16Vbp;                         ///< Verttical back porch
    u16 u16Vactive;                     ///< Vertical Width
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;                                  ///< structure size for version checking
}__attribute__ ((__packed__)) DrvPnlIoTestTgenConfig_t;

typedef struct
{
    bool bTiMode;
    DrvPnlIoTiBitMode_e enTiBitMode;
    DrvPnlIoChannelSwapType_e enCh[DRV_PNL_IO_SIGNAL_CTRL_CH_MAX];
    bool bChPolarity;
    bool bHsyncInvert;
    bool bVsyncInvert;
    bool bDeInvert;
}__attribute__ ((__packed__))  DrvPnlIoModConfig_t;

typedef struct
{
    u32   VerChk_Version ;                              ///< structure version
    DrvPnlIoLinkType_e enLinkType;
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    DrvPnlIoModConfig_t stModCfg;
    DrvPnlIoMipiDsiConfig_t stMipiDsiCfg;
    u32   VerChk_Size;
}__attribute__ ((__packed__)) DrvPnlIoSignalCtrlConfig_t;

typedef struct
{
    u32   VerChk_Version ;                              ///< structure version
    DrvPnlIoLinkType_e enLinkType;
    bool bEn;
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;
}__attribute__ ((__packed__)) DrvPnlIoPowerConfig_t;

typedef struct
{
    u32   VerChk_Version ;                              ///< structure version
    DrvPnlIoLinkType_e enLinkType;
    bool bEn;
    u16  u16Level;
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;
}__attribute__ ((__packed__)) DrvPnlIoBackLightConfig_t;


typedef struct
{
    u32   VerChk_Version ;                              ///< structure version
    DrvPnlIoLinkType_e enLinkType;
    u16  u16Val;
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;
}__attribute__ ((__packed__))  DrvPnlIoDrvCurrentConfig_t;


typedef struct
{
    u32   VerChk_Version ;                              ///< structure version
    u32   u32Level;
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;
}__attribute__ ((__packed__))  DrvPnlIoDbgLevelConfig_t;
//=============================================================================

#endif //
/** @} */ // end of PNL_group
