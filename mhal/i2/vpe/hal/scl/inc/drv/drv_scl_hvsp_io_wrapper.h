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
#ifndef _DRV_SCL_HVSP_IO_WRAPPER_H__
#define _DRV_SCL_HVSP_IO_WRAPPER_H__

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_SCLHVSP_HANDLER_PRE_FIX         SCLHVSP_HANDLER_PRE_FIX
#define DRV_SCLHVSP_HANDLER_PRE_MASK        0xFFFF0000
#define DRV_SCLHVSP_HANDLER_INSTANCE_NUM    (64)
#define DRV_SCLHVSP_HANDLER_MAX             (4 * DRV_SCLHVSP_HANDLER_INSTANCE_NUM)
#define DRV_SCLHVSP_HANDLER_DEV_MASK         0xF000

#define FHDWidth   1920
#define FHDHeight  1080
#define _3MWidth   2048
#define _3MHeight  1536


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    bool bWaitQueue;
    DrvSclOsPollWaitConfig_t stPollWaitCfg;
    PollCB *pCBFunc;
    u8 u8pollval;
    u8 u8retval;
}DrvSclHvspIoWrapperPollConfig_t;


//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifndef __DRV_SCL_HVSP_IO_WRAPPER_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE bool                  _DrvSclHvspIoInit(DrvSclHvspIoIdType_e enSclHvspId);
INTERFACE bool _DrvSclHvspIoDeInit(DrvSclHvspIoIdType_e enSclHvspId);
INTERFACE s32                   _DrvSclHvspIoOpen(DrvSclHvspIoIdType_e enSclHvspId);
INTERFACE DrvSclHvspIoErrType_e _DrvSclHvspIoRelease(s32 s32Handler);
INTERFACE DrvSclHvspIoErrType_e _DrvSclHvspIoSetInputConfig(s32 s32Handler, DrvSclHvspIoInputConfig_t *pstIoInCfg);
INTERFACE DrvSclHvspIoErrType_e _DrvSclHvspIoSetScalingConfig(s32 s32Handler, DrvSclHvspIoScalingConfig_t *pstIOSclCfg);
INTERFACE DrvSclHvspIoErrType_e _DrvSclHvspIoSetMiscConfig(s32 s32Handler, DrvSclHvspIoMiscConfig_t *pstIOMiscCfg);
INTERFACE DrvSclHvspIoErrType_e _DrvSclHvspIoGetInformConfig(s32 s32Handler, DrvSclHvspIoScInformConfig_t *pstIOInfoCfg);
INTERFACE DrvSclHvspIoErrType_e _DrvSclHvspIoSetRotateConfig(s32 s32Handler, DrvSclHvspIoRotateConfig_t *pstIoRotateCfg);
INTERFACE DrvSclHvspIoErrType_e _DrvSclHvspIoSetVtrackConfig(s32 s32Handler, DrvSclHvspIoVtrackConfig_t *pstCfg);
INTERFACE DrvSclHvspIoErrType_e _DrvSclHvspIoSetVtrackOnOffConfig(s32 s32Handler, DrvSclHvspIoVtrackOnOffConfig_t *pstCfg);

#undef INTERFACE

#endif
