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

#ifndef __JPE_DRV_IO_ST_H__
#define __JPE_DRV_IO_ST_H__

#include "cam_os_wrapper.h"

#if defined(__linux__)
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
#endif // #if defined(__linux__)

// NOTE: JpeCfg_t and JpeEncOutbuf_t should sync with the definition in hal_jpe_ops.h
#ifndef JPE_IN_BUF_MODE_E
typedef enum
{
    JPE_IBUF_ROW_MODE   = 0x0,
    JPE_IBUF_FRAME_MODE = 0x1
} JpeInBufMode_e;
#define JPE_IN_BUF_MODE_E
#endif

#ifndef JPE_RAW_FORMAT_E
typedef enum
{
    JPE_RAW_YUYV = 0x0,
    JPE_RAW_YVYU = 0x1,
    JPE_RAW_NV12 = 0x3,
    JPE_RAW_NV21 = 0x4,
} JpeRawFormat_e;
#define JPE_RAW_FORMAT_E
#endif

#ifndef JPE_CODEC_FORMAT_E
typedef enum
{
    JPE_CODEC_JPEG          = 0x1,
    JPE_CODEC_H263I         = 0x2,  //! obsolete
    JPE_CODEC_ENCODE_DCT    = 0x4,  //! obsolete
} JpeCodecFormat_e;
#define JPE_CODEC_FORMAT_E
#endif

#ifndef JPE_STATE_E
typedef enum
{
    JPE_IDLE_STATE          = 0,
    JPE_BUSY_STATE          = 1,
    JPE_FRAME_DONE_STATE    = 2,
    JPE_OUTBUF_FULL_STATE   = 3,
    JPE_INBUF_FULL_STATE    = 4
} JpeState_e;
#define JPE_STATE_E
#endif

#ifndef JPE_IOC_RET_STATUS_E
typedef enum
{
    JPE_IOC_RET_SUCCESS             = 0,
    JPE_IOC_RET_BAD_QTABLE          = 1,
    JPE_IOC_RET_BAD_QP              = 2,
    JPE_IOC_RET_BAD_BITSOFFSET      = 3,
    JPE_IOC_RET_BAD_BANKNUM         = 4,
    JPE_IOC_RET_BAD_INBUF           = 5,
    JPE_IOC_RET_BAD_OUTBUF          = 6,
    JPE_IOC_RET_BAD_NULLPTR         = 7,
    JPE_IOC_RET_BAD_BANKCNT         = 8,
    JPE_IOC_RET_BAD_LASTZZ          = 9,
    JPE_IOC_RET_UNKOWN_COMMAND      = 10,
    JPE_IOC_RET_BAD_VIRTUAL_MEM     = 11,
    JPE_IOC_RET_NEED_DRIVER_INIT    = 12,
    JPE_IOC_RET_FMT_NOT_SUPPORT     = 13,
    JPE_IOC_RET_HW_IS_RUNNING       = 14,
    JPE_IOC_RET_FAIL                = 15
} JPE_IOC_RET_STATUS_e;
#define JPE_IOC_RET_STATUS_E
#endif

#ifndef JPE_COLOR_PLAN_E
typedef enum
{
    JPE_COLOR_PLAN_LUMA     = 0,
    JPE_COLOR_PLAN_CHROMA   = 1,
    JPE_COLOR_PLAN_MAX      = 2
} JPE_COLOR_PLAN_e;
#define JPE_COLOR_PLAN_E
#endif

#ifndef JPE_CAPS_T
typedef struct
{
    u32 u32JpeId;
    u32 nRefYLogAddrAlign[2];
    u32 nRefCLogAddrAlign[2];
    u32 nOutBufSLogAddrAlign;
    u8 nSclHandShakeSupport;
    u8 nCodecSupport;
    u8 nBufferModeSupport;
} JpeCaps_t, *pJpeCaps;
#define JPE_CAPS_T
#endif

#ifndef JPE_BITSTREAM_INFO_T
typedef struct
{
    unsigned long nAddr;
    unsigned long nOrigSize;
    unsigned long nOutputSize;
    JpeState_e eState;
} JpeBitstreamInfo_t, *pJpeBitstreamInfo;
#define JPE_BITSTREAM_INFO_T
#endif

#ifndef JPE_BUF_INFO_T
typedef struct
{
    unsigned long nAddr;
    unsigned long nSize;
} JpeBufInfo_t;
#define JPE_BUF_INFO_T
#endif

#ifndef JPE_CFG_T
typedef struct
{
    JpeInBufMode_e   eInBufMode;
    JpeRawFormat_e   eRawFormat;
    JpeCodecFormat_e eCodecFormat;

    u32 nWidth;
    u32 nHeight;
    u32 nCropW ;
    u32 nCropH;
    u32 nCropOffsetX;
    u32 nCropOffsetY;
    u16 YQTable[64];
    u16 CQTable[64];
    u16 nQScale;
    u16 nQFactor;

    JpeBufInfo_t InBuf[JPE_COLOR_PLAN_MAX];
    JpeBufInfo_t OutBuf;

    u32 nJpeOutBitOffset;
    u32 nHeaderLen;
    u32 nPartialNotifySize;
    void (*notifyFunc)(unsigned long nBufAddr, unsigned long nOffset, unsigned long nNotifySize, unsigned short bFrameDone);
} JpeCfg_t;
#define JPE_CFG_T
#endif


#ifndef JPE_ENC_OUTBUF_T
typedef struct JpeEncOutbuf_t
{
    unsigned long   nAddr;
    unsigned long   nOrigSize;              // Original buffer Size
    unsigned long   nOutputSize;            // Output Size
    JpeState_e      eState;
} JpeEncOutbuf_t;
#define JPE_ENC_OUTBUF_T
#endif


/* NOTE: remove below definition when move this file to  linux-3.18\drivers\mstar\include\ */
#if defined(__I_SW__) || defined(__KERNEL__)
#include "drv_jpe_io_st_kernel.h"
#endif

#endif // __JPE_DRV_IO_ST_H__
