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

#ifndef __DRV_MHE_DEV_H__
#define __DRV_MHE_DEV_H__

#include "drv_mhe_kernel.h"

int MheDevRegister(mmhe_dev*, mmhe_ctx*);
int MheDevUnregister(mmhe_dev*, mmhe_ctx*);
int MheDevPowerOn(mmhe_dev*, int);
int MheDevSuspend(mmhe_dev*);
int MheDevResume(mmhe_dev*);
int MheDevPushjob(mmhe_dev*, mmhe_ctx*);
int MheDevLockAndFire(mmhe_dev*, mmhe_ctx*);
int MheDevUnlockAfterEncDone(mmhe_dev*, mmhe_ctx*);
int MheDevGenEncCmd(mmhe_dev* mdev, mmhe_ctx* mctx, void* cmd_buf, int* cmd_len);
int MheDevIsrFnx(mmhe_dev*);

int MheClkInit(void);
int MheDevClkRate(mmhe_dev*, int);
int MheDevClkOn(mmhe_dev*, int);
int MheDevClkInit(mmhe_dev*, u32 nDevId);
int MheDevGetResourceMem(u32 nDevId, void**, int*);
int MheDevGetResourceIrq(u32 nDevId, int*);

#endif // __DRV_MHE_DEV_H__
