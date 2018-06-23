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
 *  @file drv_scl_ioctl.h
 *  @brief SCL Driver IOCTL interface
 */

/**
 * \defgroup sclgroup  SCL driver
 * \note
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/clk
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:print explain W:control clk by explain.
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/ckcrop
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:print vsync count,err times W: if 0 close ptgen ,if echo 1 open check crop ,echo 2 open check hvsp and dma count
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/ptgen
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:print explain W: if 0 close ptgen ,if echo 1 open static ptgen ,echo 2 open dynamic ptgen,echo 3 open scl time gen
 *
 * @{
 */

#ifndef __DRV_SCL_IOCTL_H__
#define __DRV_SCL_IOCTL_H__

//=============================================================================
// Includs
//=============================================================================


//=============================================================================
// IOCTRL defines
//=============================================================================

#define IOCTL_SCL_SET_IN_CONFIG_NR                  (0)   ///< The IOCTL NR definition for IOCTL_SCL_SET_IN_CONFIG
#define IOCTL_SCL_SET_OUT_CONFIG_NR                 (1)   ///< The IOCTL NR definition for IOCTL_SCL_SET_OUT_CONFIG
#define IOCTL_SCL_SET_SCALING_CONFIG_NR             (2)   ///< The IOCTL NR definition for IOCTL_SCL_SET_SCALING_CONFIG
#define IOCTL_SCL_REQ_MEM_CONFIG_NR                 (3)   ///< The IOCTL NR definition for IOCTL_SCL_REQ_MEM_CONFIG
#define IOCTL_SCL_SET_MISC_CONFIG_NR                (4)   ///< The IOCTL NR definition for IOCTL_SCL_SET_MISC_CONFIG
#define IOCTL_SCL_SET_POST_CROP_CONFIG_NR           (5)   ///< The IOCTL NR definition for IOCTL_SCL_SET_POST_CROP_CONFIG
#define IOCTL_SCL_GET_PRIVATE_ID_CONFIG_NR          (6)   ///< The IOCTL NR definition for IOCTL_SCL_GET_PRIVATE_ID_CONFIG
#define IOCTL_SCL_GET_INFORM_CONFIG_NR              (7)   ///< The IOCTL NR definition for IOCTL_SCL_GET_INFORM_CONFIG
#define IOCTL_SCL_RELEASE_MEM_CONFIG_NR             (8)   ///< The IOCTL NR definition for IOCTL_SCL_RELEASE_MEM_CONFIG
#define IOCTL_SCL_SET_FB_MANAGE_CONFIG_NR           (10)  ///< The IOCTL NR definition for IOCTL_SCL_SET_FB_MANAGE_CONFIG
#define IOCLT_SCL_GET_VERSION_CONFIG_NR             (13)  ///< The IOCTL NR definition for IOCLT_SCL_GET_VERSION_CONFIG
#define IOCTL_SCL_SET_ROTATE_CONFIG_NR              (14)  ///< The IOCTL NR definition for IOCTL_SCL_SET_ROTATE_CONFIG
#define IOCTL_SCL_SET_LOCK_CONFIG_NR                (15)  ///< The IOCTL NR definition for IOCTL_SCL_SET_LOCK_CONFIG
#define IOCTL_SCL_SET_UNLOCK_CONFIG_NR              (16)  ///< The IOCTL NR definition for IOCTL_SCL_SET_UNLOCK_CONFIG
#define IOCTL_SCL_SET_IN_BUFFER_CONFIG_NR           (17)  ///< The IOCTL NR definition for IOCTL_SCL_SET_IN_BUFFER_CONFIG
#define IOCTL_SCL_SET_IN_TRIGGER_CONFIG_NR          (18)  ///< The IOCTL NR definition for IOCTL_SCL_SET_IN_TRIGGER_CONFIG
#define IOCTL_SCL_SET_OUT_BUFFER_CONFIG_NR          (19)  ///< The IOCTL NR definition for IOCTL_SCL_SET_OUT_BUFFER_CONFIG
#define IOCTL_SCL_SET_OUT_TRIGGER_CONFIG_NR         (20)  ///< The IOCTL NR definition for IOCTL_SCL_SET_OUT_TRIGGER_CONFIG
#define IOCTL_SCL_CREATE_INST_CONFIG_NR             (23)  ///< The IOCTL NR definition for IOCTL_SCL_SET_LOCK_CONFIG
#define IOCTL_SCL_DESTROY_INST_CONFIG_NR            (24)  ///< The IOCTL NR definition for IOCTL_SCL_SET_UNLOCK_CONFIG
#define IOCTL_SCL_GET_INFORMATION_CONFIG_NR         (25) ///< The IOCTL NR definition for IOCTL_SCL_GET_INFORMATION_CONFIG
#define IOCTL_SCL_SET_EXTERNAL_INPUT_CONFIG_NR      (26) ///< The IOCTL NR definition for IOCTL_SCL_SET_EXTERNAL_INPUT_CONFIG
#define IOCTL_SCL_BUFFER_QUEUE_HANDLE_CONFIG_NR     (27) ///< The IOCTL NR definition for IOCTL_SCL_BUFFER_QUEUE_HANDLE_CONFIG
#define IOCTL_SCL_INST_PROCESS_CONFIG_NR            (28)  ///< The IOCTL NR definition for IOCTL_SCL_SET_LOCK_CONFIG
#define IOCTL_VPE_TEST_CASE_NR                      (29)  ///< The IOCTL NR definition for IOCTL_SCL_SET_LOCK_CONFIG
#define IOCTL_SCL_FILP_INST_NR                      (30)  ///< The IOCTL NR definition for IOCTL_SCL_SET_LOCK_CONFIG
#define IOCTL_SCLVIP_SET_VTRACK_CONFIG_NR           (31) ///< The IOCTL NR definition for IOCTL_SCLHVSP_SET_VTRACK_CONFIG
#define IOCTL_SCLVIP_SET_VTRACK_ONOFF_CONFIG_NR     (32) ///< The IOCTL NR definition for IOCTL_SCLHVSP_SET_VTRACK_ONOFF_CONFIG
#define IOCTL_SCL_MAX_NR                            (33)  ///< The Max IOCTL NR for SCL driver



// use 'm' as magic number
#define IOCTL_SCL_MAGIC                       ('1')///< The Type definition of IOCTL for hvsp driver
/**
* Setup input configuration of SCL. The structure of input parameter is DrvSclHvspIoInputConfig_t.
*/
#define IOCTL_SCLHVSP_SET_IN_CONFIG             _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_IN_CONFIG_NR)
/**
* Setup output configuration of SCL. The structure of input parameter is DrvSclHvspIoOutputConfig_t.
*/
#define IOCTL_SCLHVSP_SET_OUT_CONFIG            _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_OUT_CONFIG_NR)
/**
* Setup scaling configuration, of SlcHvsp. The structure of input parameter is DrvSclHvspIoScalingConfig_t.
*/
#define IOCTL_SCLHVSP_SET_SCALING_CONFIG         _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_SCALING_CONFIG_NR)
/**
* Allocate Memory for SCL. The structure of input parameter is DrvSclHvspIoReqMemConfig_t.
*/
#define IOCTL_SCLHVSP_REQ_MEM_CONFIG             _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_REQ_MEM_CONFIG_NR)
/**
* Access the register of SCL. The structure of input parameter is DrvSclHvspIoMiscConfig_t.
*/
#define IOCTL_SCLHVSP_SET_MISC_CONFIG            _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_MISC_CONFIG_NR)
/**
* Setup post-crop configuration of SCL. The structure of input parameter is DrvSclHvspIoPostCropConfig_t.
*/
#define IOCTL_SCLHVSP_SET_POST_CROP_CONFIG       _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_POST_CROP_CONFIG_NR)
/**
* Get the private id of multiinst. The structure of output parameter is DrvSclHvspIoPrivateIdConfig_t.
*/
#define IOCTL_SCLHVSP_GET_PRIVATE_ID_CONFIG      _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_GET_PRIVATE_ID_CONFIG_NR)
/**
* Get the setting information of SCL. The structure of output parameter is DrvSclHvspIoScInformConfig_t.
*/
#define IOCTL_SCLHVSP_GET_INFORM_CONFIG          _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_GET_INFORM_CONFIG_NR)
/**
* Release allocated memory of SlcHvsp. There is no parameter
*/
#define IOCTL_SCLHVSP_RELEASE_MEM_CONFIG         _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_RELEASE_MEM_CONFIG_NR)
/**
* Get the version of SCL. The structure of output parameter is DrvSclHvspIoVersionConfig_t.
*/
#define IOCTL__GET_VERSION_CONFIG            _IO(IOCTL_SCL_MAGIC,  IOCLT_SCL_GET_VERSION_CONFIG_NR)
/**
* Setup the rotate configuration. The structure of input parameter is DrvSclHvspIoRotateConfig_t
*/
#define IOCTL_SCLHVSP_SET_ROTATE_CONFIG          _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_ROTATE_CONFIG_NR)
/**
* Setup the Lock configuration. The structure of input parameter is DrvSclHvspIoLockConfig_t
*/
#define IOCTL_SCLHVSP_SET_LOCK_CONFIG           _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_LOCK_CONFIG_NR)
/**
* Setup the UnLock configuration. The structure of input parameter is DrvSclHvspIoLockConfig_t
*/
#define IOCTL_SCLHVSP_SET_UNLOCK_CONFIG         _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_UNLOCK_CONFIG_NR)
/**
* Setup VTRACK configuration. The structure of input parameter is DrvSclHvspIoVtrackConfig_t.
*/
#define IOCTL_SCLHVSP_SET_VTRACK_CONFIG                            _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_VTRACK_CONFIG_NR)
/**
* Setup VTACK OnOff configuration. The structure of input parameter is DrvSclHvspIoVtrackOnOffConfig_t.
*/
#define IOCTL_SCLHVSP_SET_VTRACK_ONOFF_CONFIG                      _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_VTRACK_ONOFF_CONFIG_NR)
/**
* Setup parameters of in buffer configurate ,buffer address, number,dma mode,color mode and resolution, The structure of parameter is DrvSclDmaIoBufferConfig_t.
*/
#define IOCTL_SCLDMA_SET_IN_BUFFER_CONFIG           _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_IN_BUFFER_CONFIG_NR)


/**
* Setup output buffer configuration, buffer address, number ,dma mode,color mode and resolution. The structure of parameter is DrvSclDmaIoBufferConfig_t.
*/
#define IOCTL_SCLDMA_SET_OUT_BUFFER_CONFIG          _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_SET_OUT_BUFFER_CONFIG_NR)

/**
* Peek queue information and set read flag. The structure of parameter is DrvSclDmaIoBufferQueueConfig_t.
*/
#define IOCTL_SCLDMA_BUFFER_QUEUE_HANDLE_CONFIG     _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_BUFFER_QUEUE_HANDLE_CONFIG_NR)
/**
* Get private id of multi-instant. The structure of parameter is DrvSclDmaIoPrivateIdConfig_t.
*/
#define IOCTL_SCLDMA_GET_PRIVATE_ID_CONFIG          _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_GET_PRIVATE_ID_CONFIG_NR)
/**
* Setup Lock Configuration of DMA. The structure of parameter is DrvSclDmaIoLockConfig_t.
*/
#define IOCTL_SCLDMA_FILP_INST                _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_FILP_INST_NR)

/**
* Setup Lock Configuration of DMA. The structure of parameter is DrvSclDmaIoLockConfig_t.
*/
#define IOCTL_SCLDMA_CREATE_INST_CONFIG                _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_CREATE_INST_CONFIG_NR)
/**
* Setup Lock Configuration of DMA. The structure of parameter is DrvSclDmaIoLockConfig_t.
*/
#define IOCTL_SCLDMA_INST_PROCESS_CONFIG                _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_INST_PROCESS_CONFIG_NR)

/**
* Setup Unlock configuration of DMA. The structure of parameter is DrvSclDmaIoLockConfig_t.
*/
#define IOCTL_SCLDMA_DESTROY_INST_CONFIG              _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_DESTROY_INST_CONFIG_NR)
/**
* Get version configuration of DMA.The structure of parameter is DrvSclDmaIoVersionConfig_t.
*/
#define IOCTL_SCLDMA_GET_VERSION_CONFIG            _IO(IOCTL_SCL_MAGIC,  IOCLT_SCL_GET_VERSION_CONFIG_NR)
/**
* Get informaion of DMA. The structure of parameter is DrvSclDmaIoGetInformationConfig_t.
*/
#define IOCTL_SCLDMA_GET_INFORMATION_CONFIG         _IO(IOCTL_SCL_MAGIC,  IOCTL_SCL_GET_INFORMATION_CONFIG_NR)
/**
* Set external input configuration of DMA. The structure of parameter is DrvSclDmaIoExternalInputConfig_t.
*/
#define IOCTL_VPE_TEST_CASE      _IO(IOCTL_SCL_MAGIC, IOCTL_VPE_TEST_CASE_NR)

#endif //
/** @} */ // end of sclgroup
