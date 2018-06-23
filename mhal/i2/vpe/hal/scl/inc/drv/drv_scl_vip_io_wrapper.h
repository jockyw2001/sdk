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

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_SCLVIP_HANDLER_PRE_FIX     SCLVIP_HANDLER_PRE_FIX
#define DRV_SCLVIP_HANDLER_PRE_MASK    0xFFFF0000
#define DRV_SCLVIP_HANDLER_INSTANCE_NUM    (64)
#define DRV_SCLVIP_HANDLER_MAX         (1 * DRV_SCLVIP_HANDLER_INSTANCE_NUM)
#define DRV_SCLVIP_HANDLER_DEV_MASK         0xF000



//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------




//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifndef __DRV_SCL_VIP_IO_WRAPPER_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE void                 _DrvSclVipIoKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq);
INTERFACE bool                 _DrvSclVipIoInit(void);
INTERFACE bool                  _DrvSclVipIoDeInit(void);
INTERFACE s32                  _DrvSclVipIoOpen(DrvSclVipIoIdType_e enSlcVipId);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoRelease(s32 s32Handler);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetPeakingConfig(s32 s32Handler, DrvSclVipIoPeakingConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoGetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramReport_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetDlcConfig(s32 s32Handler, DrvSclVipIoDlcConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetLceConfig(s32 s32Handler, DrvSclVipIoLceConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetUvcConfig(s32 s32Handler, DrvSclVipIoUvcConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetIhcConfig(s32 s32Handler, DrvSclVipIoIhcConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetIccConfig(s32 s32Handler, DrvSclVipIoIccConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetIhcIceAdpYConfig(s32 s32Handler, DrvSclVipIoIhcIccConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetIbcConfig(s32 s32Handler, DrvSclVipIoIbcConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetFccConfig(s32 s32Handler, DrvSclVipIoFccConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetNlmConfig(s32 s32Handler, DrvSclVipIoNlmConfig_t *pstIoCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetAckConfig(s32 s32Handler, DrvSclVipIoAckConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetMcnrConfig(s32 s32Handler, DrvSclVipIoMcnrConfig_t *pstIoCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetAipConfig(s32 s32Handler, DrvSclVipIoAipConfig_t *pstIoConfig);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetAipSramConfig(s32 s32Handler, DrvSclVipIoAipSramConfig_t *pstIoCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetVipConfig(s32 s32Handler, DrvSclVipIoConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoGetVersion(s32 s32Handler, DrvSclVipIoVersionConfig_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoCreateInstConfig(s32 s32Handler, DrvSclVipIoLockConfig_t *pstIoInCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoGetPrivateIdConfig(s32 s32Handler, DrvSclVipIoPrivateIdConfig_t *pstIOCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoDestroyInstConfig(s32 s32Handler, DrvSclVipIoLockConfig_t *pstIoInCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetRoiConfig(s32 s32Handler, DrvSclVipIoWdrRoiHist_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetMaskOnOff(s32 s32Handler, DrvSclVipIoSetMaskOnOff_t *pstCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoReqmemConfig(s32 s32Handler, DrvSclVipIoReqMemConfig_t*pstReqMemCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoGetWdrHistogram(s32 s32Handler, DrvSclVipIoWdrRoiReport_t *pstIOCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoGetNRHistogram(s32 s32Handler, DrvSclVipIoNrHist_t *pstIOCfg);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoReqWdrMloadBuffer(s32 s32Handler);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoFreeWdrMloadBuffer(s32 s32Handler);
INTERFACE DrvSclVipIoErrType_e _DrvSclVipIoSetFlip(s32 s32Handler);
INTERFACE bool _DrvSclVipIsOpend(s32 s32Handler);


#undef INTERFACE
