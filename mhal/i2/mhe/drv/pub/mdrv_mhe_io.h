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

//! @file mdrv_mmhe_io.h
//! @author MStar Semiconductor Inc.
//! @brief MHE Driver IOCTL User's Interface.
//! \defgroup mmhe_group MHE driver
//! @{
#ifndef _MDRV_MMHE_IO_H_
#define _MDRV_MMHE_IO_H_

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////
#include "mdrv_mhe_st.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#if defined(__I_SW__)
//! Use to set parameters out of streaming.
#define IOCTL_MMHE_S_PARM       0
//! Use to get parameters any time.
#define IOCTL_MMHE_G_PARM       1
//! Use to transit the state to streaming-on.
#define IOCTL_MMHE_STREAMON     2
//! Use to transit the state to streaming-off.
#define IOCTL_MMHE_STREAMOFF    3
//! Use to set control during streaming.
#define IOCTL_MMHE_S_CTRL       4
//! Use to get control during streaming.
#define IOCTL_MMHE_G_CTRL       5
//! Use to encode a picture during streaming.
#define IOCTL_MMHE_S_PICT       6
//! Use to acquire the output bits of last coded picture.
#define IOCTL_MMHE_G_BITS       7
//! Use to encode a picture and acquire the output at the same time.
#define IOCTL_MMHE_ENCODE       8
//! Use to put user data.
#define IOCTL_MMHE_S_DATA       9

#else
//! Magic Number of MHE driver.
#define MAGIC_MMHE              ('m')
//! Use to Query version number of user interface.
#define IOCTL_MMHE_VERSION      _IOWR(MAGIC_MMHE, 0, unsigned int)
//! Use to set parameters out of streaming.
#define IOCTL_MMHE_S_PARM       _IOWR(MAGIC_MMHE, 1, mmhe_parm)
//! Use to get parameters any time.
#define IOCTL_MMHE_G_PARM       _IOWR(MAGIC_MMHE, 2, mmhe_parm)
//! Use to transit the state to streaming-on.
#define IOCTL_MMHE_STREAMON       _IO(MAGIC_MMHE, 3)
//! Use to transit the state to streaming-off.
#define IOCTL_MMHE_STREAMOFF      _IO(MAGIC_MMHE, 4)
//! Use to set control during streaming.
#define IOCTL_MMHE_S_CTRL       _IOWR(MAGIC_MMHE, 5, mmhe_ctrl)
//! Use to get control during streaming.
#define IOCTL_MMHE_G_CTRL       _IOWR(MAGIC_MMHE, 6, mmhe_ctrl)
//! Use to encode a picture during streaming.
#define IOCTL_MMHE_S_PICT       _IOWR(MAGIC_MMHE, 7, mmhe_buff)
//! Use to acquire the output bits of last coded picture.
#define IOCTL_MMHE_G_BITS       _IOWR(MAGIC_MMHE, 8, mmhe_buff)
//! Use to encode a picture and acquire the output at the same time.
#define IOCTL_MMHE_ENCODE       _IOWR(MAGIC_MMHE, 9, mmhe_buff[2])
//! Use to put user data.
#define IOCTL_MMHE_S_DATA       _IOWR(MAGIC_MMHE,10, mmhe_buff)
//! Use to set parameters and control (for MHAL API)
#define IOCTL_MMHE_MHAL_SET_PARAM           _IOWR(MAGIC_MMHE, 11, VencParamUT)
#define IOCTL_MMHE_MHAL_GET_PARAM           _IOWR(MAGIC_MMHE, 12, VencParamUT)
#define IOCTL_MMHE_MHAL_ENCODE_ONE_FRAME    _IOWR(MAGIC_MMHE, 13, MHAL_VENC_InOutBuf_t)
#define IOCTL_MMHE_MHAL_ENCODE_FRAME_DONE   _IOWR(MAGIC_MMHE, 14, MHAL_VENC_EncResult_t)
#define IOCTL_MMHE_MHAL_QUERY_BUFSIZE       _IOWR(MAGIC_MMHE, 15, MHAL_VENC_InternalBuf_t)
#define IOCTL_MMHE_MHAL_KICKOFF_CMDQ        _IO(MAGIC_MMHE, 16)

#endif

#endif//_MDRV_MMHE_IO_H_
//! @}
