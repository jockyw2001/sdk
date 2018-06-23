////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2017 MStar Semiconductor, Inc.
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

#ifndef ___DRV_JPE_DEV_H__
#define ___DRV_JPE_DEV_H__

#include "drv_jpe_io_st_kernel.h"

int JpeDevRegister(JpeDev_t*, JpeCtx_t*);
int JpeDevUnregister(JpeDev_t*, JpeCtx_t*);
int JpeDevPowerOn(JpeDev_t*, int);
int JpeDevPushJob(JpeDev_t*, JpeCtx_t*);
int JpeDevLockAndFire(JpeDev_t*, JpeCtx_t*);
int JpeDevUnlockAfterEncDone(JpeDev_t*, JpeCtx_t*);
int JpeDevIsrFnx(JpeDev_t*);
int JpeDevGetEngStatus(JpeDev_t* pDev, int*, u32*, u32*);
int JpeDevAddOutBuff(JpeDev_t* pDev, u32 phyOutAddr, u32 PhyOutSize);
int JpeDevEncodeCancel(JpeDev_t* pDev);

int JpeClkInit(void);
int JpeDevClkRate(JpeDev_t*, int);
int JpeDevClkOn(JpeDev_t*, int);
int JpeDevClkInit(JpeDev_t*, u32 uDevId);
int JpeDevClkDeinit(JpeDev_t*);
int JpeDevGetResourceMem(u32 uDevId, void**, int*);
int JpeDevGetResourceIrq(u32 uDevId, int*);
JPE_IOC_RET_STATUS_e _DevPowserOn(JpeDev_t* pDev, int bOn);

#endif // ___DRV_JPE_DEV_H__
