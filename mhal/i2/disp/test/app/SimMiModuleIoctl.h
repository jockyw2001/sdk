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

/**
 *  @file drv_scl_dma_ioctl.h
 *  @brief SCLDMA Driver IOCTL interface
 */


/**
 * \defgroup scldma_group  SCLDMA driver
 * \note
 *
 * sysfs Node: /sys/devices/platform/mscldma1.0/ckfrm
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:check trig off count and dma line count W: if 0 reset scldma status ,if echo 1 all reset
 *
 * sysfs Node: /sys/devices/platform/mscldma1.0/cksnp
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:check trig off count and dma line count ,W: if 0 reset scldma status ,if echo 1 all reset
 *
 * sysfs Node: /sys/devices/platform/mscldma2.0/ckfrm
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:check trig off count and dma line count W: if 0 reset scldma status ,if echo 1 all reset
 *
 * sysfs Node: /sys/devices/platform/mscldma3.0/ckfrmR
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:check trig off count and dma line count W: if 0 reset scldma status ,if echo 1 all reset
 *
 * sysfs Node: /sys/devices/platform/mscldma3.0/ckfrmW
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:check trig off count and dma line count W: if 0 reset scldma status ,if echo 1 all reset
 *
 * @{
 */

#ifndef __SIM_MIMODULE_IOCTL_H__
#define __SIM_MIMODULE_IOCTL_H__

//=============================================================================
// Includs
//=============================================================================


//=============================================================================
// IOCTRL defines
//=============================================================================
#define IOCTL_SIMMI_DISP_CONFIG_NR                              (0)
#define IOCTL_SIMMI_SET_DEBUG_LEVEL_CONFIG_NR                   (1)
#define IOCTL_SIMMI_SET_HDMITX_CONFIG_NR                        (2)
#define IOCTL_SIMMI_SET_PICTURE_CONFIG_NR                       (3)
#define IOCTL_SIMMI_MAX_NR                                      (4)

// use 'm' as magic number
#define IOCTL_SIMMI_MAGIC                                       ('2')   ///< The Type definition of IOCTL for scldma driver

#define IOCTL_SIMMI_DISP_CONFIG                                 _IO(IOCTL_SIMMI_MAGIC, IOCTL_SIMMI_DISP_CONFIG_NR)
#define IOCTL_SIMMI_SET_DEBUG_LEVEL_CONFIG                      _IO(IOCTL_SIMMI_MAGIC, IOCTL_SIMMI_SET_DEBUG_LEVEL_CONFIG_NR)
#define IOCTL_SIMMI_SET_HDMITX_CONFIG                           _IO(IOCTL_SIMMI_MAGIC, IOCTL_SIMMI_SET_HDMITX_CONFIG_NR)
#define IOCTL_SIMMI_SET_PICTURE_CONFIG                          _IO(IOCTL_SIMMI_MAGIC, IOCTL_SIMMI_SET_PICTURE_CONFIG_NR)

#endif //

/** @} */ // end of scldma_group
