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
 *  @file drv_scl_vip_ioctl.h
 *  @brief VIP Driver IOCTL interface
 */

/**
 * \defgroup sclvip_group  SCLVIP driver
 * \note
 *
 * sysfs Node: /sys/devices/platform/mvip1.0/bypass
 *
 * sysfs R/W mode: W
 *
 * sysfs Usage & Description: W:echo 1 open bypass ,echo 0 close.
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/ckPQ
 *
 * sysfs R/W mode: W
 *
 * sysfs Usage & Description: echo 1 PQ already setting check , echo 2 Auto test PQ
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/ckCMDQ
 *
 * sysfs R/W mode: W
 *
 * sysfs Usage & Description: echo 1 CMDQ already setting check , echo 2 Auto test CMDQ
 *
 * @{
 */
#ifndef __DRV_SCL_VIP_IOCTL_H__
#define __DRV_SCL_VIP_IOCTL_H__

//=============================================================================
// Includs
//=============================================================================


//=============================================================================
// IOCTRL defines
//=============================================================================
#define IOCTL_SCLVIP_SET_PEAKING_CONFIG_NR                        (1)  ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_PEAKING_CONFIG
#define IOCTL_SCLVIP_SET_DLC_HISTOGRAM_CONFIG_NR                  (2)  ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_DLC_HISTOGRAM_CONFIG
#define IOCTL_SCLVIP_GET_DLC_HISTOGRAM_REPORT_NR                  (3)  ///< The IOCTL NR definition for IOCTL_SCLVIP_GET_DLC_HISTOGRAM_REPORT
#define IOCTL_SCLVIP_SET_DLC_CONFIG_NR                            (4)  ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_DLC_CONFIG
#define IOCTL_SCLVIP_SET_LCE_CONFIG_NR                            (6)  ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_LCE_CONFIG
#define IOCTL_SCLVIP_SET_UVC_CONFIG_NR                            (7)  ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_UVC_CONFIG
#define IOCTL_SCLVIP_SET_IHC_CONFIG_NR                            (8)  ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_IHC_CONFIG
#define IOCTL_SCLVIP_SET_ICE_CONFIG_NR                            (9)  ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_ICE_CONFIG
#define IOCTL_SCLVIP_SET_IHC_ICE_ADP_Y_CONFIG_NR                  (10) ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_IHC_ICE_ADP_Y_CONFIG
#define IOCTL_SCLVIP_SET_IBC_CONFIG_NR                            (11) ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_IBC_CONFIG
#define IOCTL_SCLVIP_SET_FCC_CONFIG_NR                            (12) ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_FCC_CONFIG
#define IOCTL_SCLVIP_SET_VIP_CONFIG_NR                            (13) ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_VIP_CONFIG
#define IOCTL_SCLVIP_SET_ACK_CONFIG_NR                            (19) ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_ACK_CONFIG
#define IOCTL_SCLVIP_SET_NLM_CONFIG_NR                            (20) ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_NLM_CONFIG
#define IOCTL_SCLVIP_AIP_CONFIG_NR                                (24) ///< The IOCTL NR definition for IOCTL_SCLVIP_AIP_CONFIG
#define IOCTL_SCLVIP_AIP_SRAM_CONFIG_NR                           (25) ///< The IOCTL NR definition for IOCTL_SCLVIP_AIP_SRAM_CONFIG
#define IOCTL_SCLVIP_SET_MCNR_CONFIG_NR                           (26) ///< The IOCTL NR definition for IOCTL_SCLVIP_SET_MCNR_CONFIG
#define IOCLT_SCLVIP_GET_VERSION_CONFIG_NR                        (27) ///< The IOCTL NR definition for IOCLT_SCLVIP_GET_VERSION_CONFIG_NR
#define IOCTL_SCLVIP_GET_PRIVATE_ID_CONFIG_NR                     (29) ///< The IOCTL NR for vip driver
#define IOCTL_SCLVIP_CREATE_INST_CONFIG_NR                        (30) ///< The IOCTL NR for vip driver
#define IOCTL_SCLVIP_DESTROY_INST_CONFIG_NR                       (31) ///< The IOCTL NR for vip driver
#define IOCTL_SCLVIP_SET_ROI_CONFIG_NR                            (32) ///< The IOCTL NR for vip driver
#define IOCTL_SCLVIP_SET_MASK_ONOFF_NR                            (33) ///< The IOCTL NR for vip driver
#define IOCTL_SCLVIP_GET_ROI_CONFIG_NR                            (34) ///< The IOCTL NR for vip driver
#define IOCTL_SCLVIP_GET_NR_HISTGRAM_NR                           (35) ///< The IOCTL NR for vip driver
#define IOCTL_SCLVIP_FILP_INST_NR                                 (36) ///< The IOCTL NR for vip driver
#define IOCTL_SCLVIP_REQUEST_MEM_NR                               (37) ///< The IOCTL NR for vip driver
#define IOCTL_SCLVIP_MAX_NR                                       (38) ///< The Max IOCTL NR for vip driver



// use 'm' as magic number
#define IOCTL_VIP_MAGIC                                        ('3') ///< The Type definition of IOCTL for vip driver
/**
* Setup Peaking configuration. The structure of input parameter is DrvSclVipIoPeakingConfig_t
*/
#define IOCTL_SCLVIP_SET_PEAKING_CONFIG                           _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_PEAKING_CONFIG_NR)
/**
* Setup DLC hsigotram configuration. The structure of input parameter is DrvSclVipIoDlcHistogramConfig_t.
*/
#define IOCTL_SCLVIP_SET_DLC_HISTOGRAM_CONFIG                     _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_DLC_HISTOGRAM_CONFIG_NR)
/**
* Get the report of DLC. The structure of ouput parameter is DrvSclVipIoDlcHistogramReport_t.
*/
#define IOCTL_SCLVIP_GET_DLC_HISTOGRAM_REPORT                     _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_GET_DLC_HISTOGRAM_REPORT_NR)
/**
* Setup DLC configuration. The structure of input parameter is DrvSclVipIoDlcConfig_t.
*/
#define IOCTL_SCLVIP_SET_DLC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_DLC_CONFIG_NR)
/**
* Setup LCE configuration. The structure of input parameter is DrvSclVipIoLceConfig_t.
*/
#define IOCTL_SCLVIP_SET_LCE_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_LCE_CONFIG_NR)
/**
* Setup UVC Cconfiguration. The structure of input parameter is DrvSclVipIoUvcConfig_t.
*/
#define IOCTL_SCLVIP_SET_UVC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_UVC_CONFIG_NR)
/**
* Setup IHC configuration. The structure of input parameter is DrvSclVipIoIhcConfig_t.
*/
#define IOCTL_SCLVIP_SET_IHC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_IHC_CONFIG_NR)
/**
* Setup ICE configuration. The structure of input parameter is DrvSclVipIoIccConfig_t.
*/
#define IOCTL_SCLVIP_SET_ICE_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_ICE_CONFIG_NR)
/**
* Setup IHCICC configuration. The structure of input parameter is DrvSclVipIoIhcIccConfig_t.
*/
#define IOCTL_SCLVIP_SET_IHC_ICE_ADP_Y_CONFIG                     _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_IHC_ICE_ADP_Y_CONFIG_NR)
/**
* Setup IBC configuration. The structure of input parameter is DrvSclVipIoIbcConfig_t.
*/
#define IOCTL_SCLVIP_SET_IBC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_IBC_CONFIG_NR)
/**
* Setup FCC configuration. The structure of input parameter is DrvSclVipIoFccConfig_t.
*/
#define IOCTL_SCLVIP_SET_FCC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_FCC_CONFIG_NR)
/**
* Setup ACK configuration. The structure of input parameter is DrvSclVipIoAckConfig_t.
*/
#define IOCTL_SCLVIP_SET_ACK_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_ACK_CONFIG_NR)
/**
* Setup NLM configuration. The structure of input parameter is DrvSclVipIoNlmConfig_t.
*/
#define IOCTL_SCLVIP_SET_NLM_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_NLM_CONFIG_NR)
/**
* Setup VIPLB configuration. The structure of input parameter is DrvSclVipIoConfig_t.
*/
#define IOCTL_SCLVIP_SET_VIP_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_VIP_CONFIG_NR)
/**
* Setup AIP configuration. The structure of input parameter is DrvSclVipIoAipConfig_t.
*/
#define IOCTL_VIP_SET_AIP_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_AIP_CONFIG_NR)
/**
* Setup AIP SRAM configuration. The structure of input parameter is DrvSclVipIoAipSramConfig_t.
*/
#define IOCTL_VIP_SET_AIP_SRAM_CONFIG                          _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_AIP_SRAM_CONFIG_NR)
/**
* Setup MCNR configuration. The structure of input parameter is DrvSclVipIoMcnrConfig_t.
*/
#define IOCTL_SCLVIP_SET_MCNR_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_MCNR_CONFIG_NR)

/**
* Get version of SCLVIP. The structure of output parameter is DrvSclVipIoGetVersionConfig_t.
*/
#define IOCTL_VIP_GET_VERSION_CONFIG                           _IO(IOCTL_VIP_MAGIC,  IOCLT_SCLVIP_GET_VERSION_CONFIG_NR)
/**
* Setup all vip Configuration. The structure of input parameter is DrvSclVipIoAllSetConfig_t.
*/
#define IOCTL_SCLVIP_GET_PRIVATE_ID_CONFIG                             _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_GET_PRIVATE_ID_CONFIG_NR)
#define IOCTL_SCLVIP_CREATE_INST_CONFIG                             _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_CREATE_INST_CONFIG_NR)
#define IOCTL_SCLVIP_DESTROY_INST_CONFIG                             _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_DESTROY_INST_CONFIG_NR)
#define IOCTL_SCLVIP_SET_ROI_CONFIG                             _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_ROI_CONFIG_NR)
#define IOCTL_SCLVIP_SET_MASK_ONOFF                             _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_SET_MASK_ONOFF_NR)
#define IOCTL_SCLVIP_GET_ROI_CONFIG                             _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_GET_ROI_CONFIG_NR)
#define IOCTL_SCLVIP_GET_NR_HISTGRAM                             _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_GET_NR_HISTGRAM_NR)
#define IOCTL_SCLVIP_FILP_INST                             _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_FILP_INST_NR)
#define IOCTL_SCLVIP_REGUEST_MEM                             _IO(IOCTL_VIP_MAGIC,  IOCTL_SCLVIP_REQUEST_MEM_NR)


#endif //
/** @} */ // end of sclvip_group
