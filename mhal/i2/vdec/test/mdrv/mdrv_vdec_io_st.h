////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009-2010 MStar Semiconductor, Inc.
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

#ifndef _MDRV_VDEC_IO_ST_H_
#define _MDRV_VDEC_IO_ST_H_

#include <asm/types.h>
#include <linux/kernel.h>
#include "vdec_api.h"
#include "apiJPEG.h"

typedef struct
{
  VDEC_EX_GET_FREE_STREAM_PARAMS_t   tParam;
} IOCTL_PARAM_VDEC_GET_FREE_STREAM_t;

typedef struct
{
    VDEC_StreamId       tStreamId;
    VDEC_EX_InitParam   tParam;
} IOCTL_PARAM_VDEC_INIT_t;

typedef struct
{
    VDEC_StreamId               tStreamId;
    VDEC_EX_User_Cmd            eCmdId;
    VDEC_PRE_CONTROL_PARAMS_t   tParam;
} IOCTL_PARAM_VDEC_PRESET_CONTROL_t;

typedef struct
{
    VDEC_StreamId               tStreamId;
    VDEC_EX_User_Cmd            eCmdId;
    VDEC_SET_CONTROL_PARAMS_t   tParam;
} IOCTL_PARAM_VDEC_SET_CONTROL_t;

typedef struct
{
    VDEC_StreamId               tStreamId;
    VDEC_EX_User_Cmd            eCmdId;
    VDEC_GET_CONTROL_PARAMS_t   tParam;
} IOCTL_PARAM_VDEC_GET_CONTROL_t;

typedef struct
{
    MS_U8   u8MiuSel;
    MS_SIZE u32Offset;
    MS_SIZE u32MapSize;
    MS_SIZE u32KvirtAddr;
} IOCTL_PARAM_VDEC_MAPINFO_t;

typedef struct
{
    VDEC_UNMAPINFO_t            tInfo;
} IOCTL_PARAM_VDEC_UNMAPINFO_t;


//==================
typedef struct
{
  NJPEG_VerificationMode        eVerificationMode;
} IOCTL_PARAM_VDEC_JPDSetVerificationMode_t;


#endif // _MDRV_VDEC_IO_ST_H_
