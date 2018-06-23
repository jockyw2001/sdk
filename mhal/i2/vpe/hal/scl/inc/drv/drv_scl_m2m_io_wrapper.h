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
#ifndef _DRV_SCL_M2M_IO_WRAPPER_H__
#define _DRV_SCL_M2M_IO_WRAPPER_H__

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_M2M_HANDLER_PRE_FIX          SCLM2M_HANDLER_PRE_FIX
#define DRV_M2M_HANDLER_PRE_MASK         0xFFFF0000
#define DRV_M2M_HANDLER_DEV_MASK         0xF000
#define DRV_M2M_HANDLER_INSTANCE_NUM     16
#define DRV_M2M_HANDLER_MAX              (DRV_M2M_HANDLER_INSTANCE_NUM)
#define IsMDrvM2MIdType_1(x)          (x == E_MDRV_SCLDMA_ID_1)
#define IsMDrvM2MIdType_2(x)          (x == E_MDRV_SCLDMA_ID_2)
#define IsMDrvM2MIdType_3(x)          (x == E_MDRV_SCLDMA_ID_3)
#define IsMDrvM2MIdType_4(x)          (x == E_MDRV_SCLDMA_ID_4)
#define IsMDrvM2MIdType_Max(x)        (x == E_MDRV_SCLDMA_ID_NUM)

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    bool bWaitQueue;
    DrvSclOsPollWaitConfig_t stPollWaitCfg;
    pDrvSclDmaIoPollCb pfnCb;
    u8 u8pollval;
    u8 u8retval;
}DrvM2MIoWrapperPollConfig_t;


//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifndef __DRV_SCL_DMA_IO_WRAPPER_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE bool                 _DrvM2MIoInit(void);
INTERFACE bool                  _DrvM2MIoDeInit(void);
INTERFACE s32                  _DrvM2MIoOpen(DrvM2MIoIdType_e enM2MId);
INTERFACE DrvM2MIoErrType_e _DrvM2MIoRelease(s32 s32Handler);
INTERFACE DrvM2MIoErrType_e _DrvM2MIoPoll(s32 s32Handler ,DrvM2MIoWrapperPollConfig_t *pstCfg);
INTERFACE DrvM2MIoErrType_e _DrvM2MIoSuspend(s32 s32Handler);
INTERFACE DrvM2MIoErrType_e _DrvM2MIoResume(s32 s32Handler);
INTERFACE DrvM2MIoErrType_e _DrvM2MIoSetM2MConfig(s32 s32Handler, DrvM2MIoConfig_t *pstIoInCfg);
INTERFACE DrvM2MIoErrType_e _DrvM2MIoCreateInstConfig(s32 s32Handler, DrvM2MIoLockConfig_t *pstIoInCfg);
INTERFACE DrvM2MIoErrType_e _DrvM2MIoDestroyInstConfig(s32 s32Handler, DrvM2MIoLockConfig_t *pstIoInCfg);
INTERFACE DrvM2MIoErrType_e _DrvM2MIoInstProcess(s32 s32Handler, DrvM2MIoProcessConfig_t *pstIoInCfg);
INTERFACE void _DrvM2MIoKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq);
INTERFACE DrvM2MIoErrType_e _DrvM2MIoInstFlip(s32 s32Handler);

#undef INTERFACE

#endif
