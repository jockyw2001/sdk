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

#ifndef __DRV_MFE_CTX_H__
#define __DRV_MFE_CTX_H__

#include "drv_mfe_kernel.h"

#define _ALIGN_(b,a)    (((a)+(1<<(b))-1)&(~((1<<(b))-1)))

#define MBPIXELS_Y      256
#define MBPIXELS_C      (MBPIXELS_Y/2)
#define MB_GN_DATA      128 // For supporting CABAC, GN size for each MB is enlarged from 64 to 128 bytes.

mmfe_ctx* MfeCtxAcquire(mmfe_dev*);
long MfeCtxActions(mmfe_ctx*, unsigned int, void*);
int MfeCtxEncFireAndReturn(void* pctx, mmfe_buff* buff);
int MfeCtxEncPostProc(void* pctx, mmfe_buff* buff);
int MfeCtxGenCompressRegCmd(void* pctx, mmfe_buff* buff, void* cmd_buf, int* cmd_len);

#endif // __DRV_MFE_CTX_H__
