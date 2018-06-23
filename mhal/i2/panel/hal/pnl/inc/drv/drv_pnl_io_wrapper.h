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
#ifndef __DRV_SCL_IO_WRAPPER_H__
#define __DRV_SCL_IO_WRAPPER_H__

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_PNL_HANDLER_PRE_FIX         0x12380000
#define DRV_PNL_HANDLER_PRE_MASK        0xFFFF0000
#define DRV_PNL_HANDLER_INSTANCE_NUM    (1)
#define DRV_PNL_HANDLER_MAX             (1 * DRV_PNL_HANDLER_INSTANCE_NUM)


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    void *pCtx;
}DrvPnlIoCtxConfig_t;


typedef struct
{
    s32 s32Handle;
    DrvPnlIoCtxConfig_t stCtxCfg;
} DrvPnlHandleConfig_t;

typedef struct
{
    u32 u32StructSize;
    u32 *pVersion;
    u32 u32VersionSize;
} DrvPnlVersionChkConfig_t;


//------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifndef __DRV_PNL_IO_WRAPPER_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE bool              _DrvPnlIoInit(void);
INTERFACE s32               _DrvPnlIoOpen(void);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoClose(s32 s32Handler);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoSetTimingConfig(s32 s32Handler, DrvPnlIoTimingConfig_t *pstIoTimingCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoSetSscConfig(s32 s32Handler, DrvPnlIoSscConfig_t *pstIoSscCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoSetTestPatternConfig(s32 s32Hander, DrvPnlIoTestPatternConfig_t *pIoTestPatCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoSetTestTgenConfig(s32 s32Handler, DrvPnlIoTestTgenConfig_t *pIoTestTgenCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoSetSignalCtrlConfig(s32 s32Handler, DrvPnlIoSignalCtrlConfig_t *pIoSignalCtrlCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoSetPowerConfig(s32 s32Handler, DrvPnlIoPowerConfig_t *pIoPowerCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoSetBackLightConfig(s32 s32Handler, DrvPnlIoBackLightConfig_t *pIoBackLightCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoGetPowerConfig(s32 s32Handler, DrvPnlIoPowerConfig_t *pIoPowerCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoGetBackLightConfig(s32 s32Handler, DrvPnlIoBackLightConfig_t *pIoBackLightCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoSetDrvCurrentConfig(s32 s32Handler, DrvPnlIoDrvCurrentConfig_t *pIoDrvCurrentCfg);
INTERFACE DrvPnlIoErrType_e _DrvPnlIoSetDbgLevelConfig(s32 s32Handler, DrvPnlIoDbgLevelConfig_t *pIoDbgLvCfg);


#undef INTERFACE

#endif
