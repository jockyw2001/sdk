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

//! @file mdrv_rqct_io.h
//! @author MStar Semiconductor Inc.
//! @brief RQCT Driver IOCTL User's Interface.
//! \defgroup rqct_group RQ controller
//! @{
#ifndef _MDRV_PMBR_IO_H_
#define _MDRV_PMBR_IO_H_

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////
#include "mdrv_pmbr_st.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#if defined(__I_SW__)
//! Use to set pmbr during streaming.
#define IOCTL_PMBR_S_CONF   14
//! Use to get pmbr during streaming.
#define IOCTL_PMBR_G_CONF   15
#else
//! Magic Number of RQCT.
#define MAGIC_PMBR          ('p')
//! Use to Query version number of user interface.
#define IOCTL_PMBR_VERSION  _IOWR(MAGIC_PMBR, 0,unsigned int)
//! Use to set pmbr-control during streaming.
#define IOCTL_PMBR_S_CONF   _IOWR(MAGIC_PMBR, 1,pmbr_conf)
//! Use to get pmbr-control during streaming.
#define IOCTL_PMBR_G_CONF   _IOWR(MAGIC_PMBR, 2,pmbr_conf)
#endif
#endif //_MDRV_PMBR_IO_H_
//! @}
