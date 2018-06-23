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
#ifndef __DRV_SCL_VIP_IO_H__
#define __DRV_SCL_VIP_IO_H__
#include "drv_scl_vip_io_st.h"
#include "drv_scl_hvsp_io_st.h"

s32 DrvSclVipIoOpen(DrvSclVipIoIdType_e enSlcVipId);

DrvSclVipIoErrType_e DrvSclVipIoRelease(s32 s32Handler);

DrvSclVipIoErrType_e DrvSclVipIoSetPeakingConfig(s32 s32Handler, DrvSclVipIoPeakingConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoGetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramReport_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetDlcConfig(s32 s32Handler,DrvSclVipIoDlcConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetLceConfig(s32 s32Handler,DrvSclVipIoLceConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetUvcConfig(s32 s32Handler, DrvSclVipIoUvcConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetIhcConfig(s32 s32Handler,DrvSclVipIoIhcConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetIccConfig(s32 s32Handler, DrvSclVipIoIccConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetIhcIceAdpYConfig(s32 s32Handler,DrvSclVipIoIhcIccConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetIbcConfig(s32 s32Handler,DrvSclVipIoIbcConfig_t *pstCfg);
DrvSclVipIoErrType_e DrvSclVipIoSetFccConfig(s32 s32Handler,DrvSclVipIoFccConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetNlmConfig(s32 s32Handler,DrvSclVipIoNlmConfig_t *pstIoCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetAckConfig(s32 s32Handler, DrvSclVipIoAckConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetAipConfig(s32 s32Handler, DrvSclVipIoAipConfig_t *pstIoConfig);

DrvSclVipIoErrType_e DrvSclVipIoSetAipSramConfig(s32 s32Handler, DrvSclVipIoAipSramConfig_t *pstIoCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetVipConfig(s32 s32Handler,DrvSclVipIoConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetWdrRoiHistConfig(s32 s32Handler,DrvSclVipIoWdrRoiHist_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoGetWdrRoiHistConfig(s32 s32Handler,DrvSclVipIoWdrRoiReport_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetWdrRoiMask(s32 s32Handler,DrvSclVipIoSetMaskOnOff_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoSetMcnrConfig(s32 s32Handler, DrvSclVipIoMcnrConfig_t *pstIoCfg);

DrvSclVipIoErrType_e DrvSclVipIoReqmemConfig(s32 s32Handler, DrvSclVipIoReqMemConfig_t *pstIoCfg);

DrvSclVipIoErrType_e DrvSclVipIoGetVersion(s32 s32Handler,DrvSclVipIoVersionConfig_t *pstCfg);

DrvSclVipIoErrType_e DrvSclVipIoGetNRHistogram(s32 s32Handler, DrvSclVipIoNrHist_t *pstIOCfg);

DrvSclVipIoErrType_e DrvSclVipIoGetPrivateIdConfig(s32 s32Handler, DrvSclVipIoPrivateIdConfig_t *pstIOCfg);

DrvSclVipIoErrType_e DrvSclVipIoctlInstFlip(s32 s32Handler);
#endif
