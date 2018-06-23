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

#ifndef _MDRV_VDEC_IO_H_
#define _MDRV_VDEC_IO_H_

#include <asm/types.h>
#include <linux/kernel.h>

#if 0
#define IOCPARM_MASK    0x7f            /* parameters must be < 128 bytes */
#define IOC_VDEC_VOID    0x20000000      /* no parameters */
#define IOC_VDEC_OUT     0x40000000      /* copy out parameters */
#define IOC_VDEC_IN      0x80000000      /* copy in parameters */
#define IOC_VDEC_INOUT   (IOC_VDEC_IN|IOC_VDEC_OUT)

#define _IO_VDEC(x,y)    (IOC_VDEC_VOID|((x)<<8)|(y))
#define _IOR_VDEC(x,y,t) (IOC_VDEC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#define _IOW_VDEC(x,y,t) (IOC_VDEC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#else
#define _IO_VDEC        _IO
#define _IOR_VDEC       _IOR
#define _IOW_VDEC       _IOW
#endif

//VDEC IOC COMMANDS

#define VDEC_IOC_MAGIC                            'D'
// Initialize *pJpeInfo
#define IOCTL_CMD_VDEC_EX_GET_FREE_STREAM       _IO_VDEC(VDEC_IOC_MAGIC, 0)
// Set up VDEC RIU and fire VDEC
#define IOCTL_CMD_VDEC_EX_INIT                     _IO_VDEC(VDEC_IOC_MAGIC, 1)
// Get output buffer status
#define IOCTL_CMD_VDEC_EX_SET_CONTROL              _IO_VDEC(VDEC_IOC_MAGIC, 2)
//
#define IOCTL_CMD_VDEC_EX_GET_CONTROL              _IO_VDEC(VDEC_IOC_MAGIC, 3)
//
#define IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL          _IO_VDEC(VDEC_IOC_MAGIC, 4)
//
#define IOCTL_CMD_VDEC_EX_POST_SET_CONTROL         _IO_VDEC(VDEC_IOC_MAGIC, 5)
//
#define IOCTL_CMD_VDEC_EX_DOMMAP                   _IO_VDEC(VDEC_IOC_MAGIC, 6)
//
#define IOCTL_CMD_VDEC_EX_DOUNMAP                  _IO_VDEC(VDEC_IOC_MAGIC, 7)
//
#define IOCTL_CMD_VDEC_JPDSetVerificationMode      _IO_VDEC(VDEC_IOC_MAGIC, 8)
//
#define IOCTL_CMD_VDEC_SET_R2EVDCLK                _IO_VDEC(VDEC_IOC_MAGIC, 9)

#endif // _MDRV_VDEC_IO_H_
