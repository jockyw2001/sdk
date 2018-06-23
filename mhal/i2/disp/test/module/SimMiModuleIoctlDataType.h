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

#ifndef __SIM_MIMODULE_IOCTL_DATA_TYPE_H__
#define __SIM_MIMODULE_IOCTL_DATA_TYPE_H__

//=============================================================================
// Includs
//=============================================================================
typedef enum
{
    E_SIMMI_MODULE_DEVICE_ID_0,
    E_SIMMI_MODULE_DEVICE_ID_1,
    E_SIMMI_MODULE_DEVICE_ID_NUM,
}SimMiModuleDeviceId_e;


typedef enum
{
    E_SIMMI_MODULE_VIDEOLAYER_ID_0,
    E_SIMMI_MODULE_VIDEOLAYER_ID_1,
    E_SIMMI_MODULE_VIDEOLAYER_ID_NUM,
}SimMiModuleVideoLayerId_e;

typedef enum
{
    E_SIMMI_MODULE_OUTPUT_HDMI = 0x01,
    E_SIMMI_MODULE_OUTPUT_CVBS = 0x02,
    E_SIMMI_MODULE_OUTPUT_VGA  = 0x04,
    E_SIMMI_MODULE_OUTPUT_LCD  = 0x08,
    E_SIMMI_MODULE_OUTPUT_NUM,
}SimMiModuleOuputInterface_e;


typedef enum
{
    E_SIMMI_MODULE_PNL_TIMING_MIN = 0,

    E_SIMMI_MODULE_PNL_TIMING_SEC32_LE32A_FULLHD = E_SIMMI_MODULE_PNL_TIMING_MIN,
    // For Normal LVDS panel
    E_SIMMI_MODULE_PNL_TIMING_SXGA          = 1,
    E_SIMMI_MODULE_PNL_TIMING_WXGA          = 2,
    E_SIMMI_MODULE_PNL_TIMING_WXGA_PLUS     = 3,
    E_SIMMI_MODULE_PNL_TIMING_WSXGA         = 4,
    E_SIMMI_MODULE_PNL_TIMING_FULL_HD       = 5,

    // For DAC/HDMI Tx output
    E_SIMMI_MODULE_PNL_TIMING_480I       = 6,
    E_SIMMI_MODULE_PNL_TIMING_480P       = 7,
    E_SIMMI_MODULE_PNL_TIMING_576I       = 8,
    E_SIMMI_MODULE_PNL_TIMING_576P       = 9,
    E_SIMMI_MODULE_PNL_TIMING_720P_50    = 10,
    E_SIMMI_MODULE_PNL_TIMING_720P_60    = 11,
    E_SIMMI_MODULE_PNL_TIMING_1080I_50   = 12,
    E_SIMMI_MODULE_PNL_TIMING_1080I_60   = 13,
    E_SIMMI_MODULE_PNL_TIMING_1080P_24   = 14,
    E_SIMMI_MODULE_PNL_TIMING_1080P_25   = 15,
    E_SIMMI_MODULE_PNL_TIMING_1080P_30   = 16,
    E_SIMMI_MODULE_PNL_TIMING_1080P_50   = 17,
    E_SIMMI_MODULE_PNL_TIMING_1080P_60   = 18,
    E_SIMMI_MODULE_PNL_TIMING_640X480P   = 19,
    E_SIMMI_MODULE_PNL_TIMING_1920X2205P_24 = 20,
    E_SIMMI_MODULE_PNL_TIMING_1280X1470P_50 = 21,
    E_SIMMI_MODULE_PNL_TIMING_1280X1470P_60 = 22,
    E_SIMMI_MODULE_PNL_TIMING_3840X2160P_24 = 23,
    E_SIMMI_MODULE_PNL_TIMING_3840X2160P_25 = 24,
    E_SIMMI_MODULE_PNL_TIMING_3840X2160P_30 = 25,
    E_SIMMI_MODULE_PNL_TIMING_3840X2160P_50 = 26,
    E_SIMMI_MODULE_PNL_TIMING_3840X2160P_60 = 27,
    E_SIMMI_MODULE_PNL_TIMING_4096X2160P_24 = 28,
    E_SIMMI_MODULE_PNL_TIMING_4096X2160P_25 = 29,
    E_SIMMI_MODULE_PNL_TIMING_4096X2160P_30 = 30,
    E_SIMMI_MODULE_PNL_TIMING_4096X2160P_50 = 31,
    E_SIMMI_MODULE_PNL_TIMING_4096X2160P_60 = 32,
    E_SIMMI_MODULE_PNL_TIMING_1600X1200P_60 = 33,
    E_SIMMI_MODULE_PNL_TIMING_1440X900P_60  = 34,
    E_SIMMI_MODULE_PNL_TIMING_1280X1024P_60 = 35,
    E_SIMMI_MODULE_PNL_TIMING_1024X768P_60  = 36,
    E_SIMMI_MODULE_PNL_TIMING_720P_24       = 37,
    E_SIMMI_MODULE_PNL_TIMING_720P_25       = 38,
    E_SIMMI_MODULE_PNL_TIMING_720P_30       = 39,
    E_SIMMI_MODULE_PNL_TIMING_800X600P_60   = 40,
    E_SIMMI_MODULE_PNL_TIMING_1280X800P_60  = 41,
    E_SIMMI_MODULE_PNL_TIMING_1366X768P_60  = 42,
    E_SIMMI_MODULE_PNL_TIMING_1680X1050P_60 = 43,
    E_SIMMI_MODULE_PNL_TIMING_1920X1200P_60 = 44,
    E_SIMMI_MODULE_PNL_TIMING__WUXGA        = 45,
    E_SIMMI_MODULE_PNL_TIMING_MAX_NUM       = 46,
}SimMiModulePnlTiming_e;


typedef enum
{
    E_SIMMI_MODULE_OUTPUT_TIMING_NTSC,
    E_SIMMI_MODULE_OUTPUT_TIMING_PAL,

    E_SIMMI_MODULE_OUTPUT_TIMING_480P,
    E_SIMMI_MODULE_OUTPUT_TIMING_576P,
    E_SIMMI_MODULE_OUTPUT_TIMING_720P50,
    E_SIMMI_MODULE_OUTPUT_TIMING_720P60,
    E_SIMMI_MODULE_OUTPUT_TIMING_1080P24,
    E_SIMMI_MODULE_OUTPUT_TIMING_1080P25,
    E_SIMMI_MODULE_OUTPUT_TIMING_1080P30,
    E_SIMMI_MODULE_OUTPUT_TIMING_1080P50,
    E_SIMMI_MODULE_OUTPUT_TIMING_1080P60,
    E_SIMMI_MODULE_OUTPUT_TIMING_4K2K_30P,

    E_SIMMI_MODULE_OUTPUT_TIMING_640x480_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_800x600_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_1024x768_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_1280x1024_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_1366x768_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_1440x900_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_1280x800_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_1680x1050_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_1600x1200_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_1920x1200_60,
    E_SIMMI_MODULE_OUTPUT_TIMING_USER,
    E_SIMMI_MODULE_OUTPUT_TIMING_NUM,
}SimMiModuleOuptTiming_e;

typedef enum
{
    E_SIMMI_MODULE_PIXEL_FORMAT_ARGB8888,
    E_SIMMI_MODULE_PIXEL_FORMAT_YUV422,
    E_SIMMI_MODULE_PIXEL_FORMAT_YUV420,
    E_SIMMI_MODULE_PIXEL_FORMAT_RGB565,
    E_SIMMI_MODULE_PIXEL_FORMAT_NUM,
}SimMiModulePixelFormat_e;

typedef enum
{
    E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1,
    E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_2,
    E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_NUM,
}SimMiModuleOutputInerfaceId_e;

typedef enum
{
    E_SIMMI_MODULE_DEBUG_LEVEL_XC,
    E_SIMMI_MODULE_DEBUG_LEVEL_HDMITX,
    E_SIMMI_MODULE_DEBUG_LEVEL_MHAL_DISP,
    E_SIMMI_MODULE_DEBUG_LEVEL_VE,
    E_SIMMI_MODULE_DEBUG_LEVEL_MGWIN,
    E_SIMMI_MODULE_DEBUG_LEVEL_MISC,
    E_SIMMI_MODULE_DEBUG_LEVEL_TEST_PATTERN,
}SimMiModuleDebugType_e;

typedef enum
{
    E_SIMMI_MODULE_HDMITX_COLOR_RGB444,
    E_SIMMI_MODULE_HDMITX_COLOR_YUV444,
    E_SIMMI_MODULE_HDMITX_COLOR_YUV422,
    E_SIMMI_MODULE_HDMITX_COLOR_NUM,
}SimMiModuleHdmitxColorType_e;


typedef enum
{
    E_SIMMI_MODULE_HDMITX_CD_8_BIT,
    E_SIMMI_MODULE_HDMITX_CD_10_BIT,
    E_SIMMI_MODULE_HDMITX_CD_12_BIT,
    E_SIMMI_MODULE_HDMITX_CD_16_BIT,
    E_SIMMI_MODULE_HDMITX_CD_NUM,
}SimMiModuleHdmitxColorDepthType_e;

typedef enum
{
    E_SIMMI_MODULE_MATRIX_BYPASS = 0,
    E_SIMMI_MODULE_MATRIX_BT601_TO_BT709,
    E_SIMMI_MODULE_MATRIX_BT709_TO_BT601,
    E_SIMMI_MODULE_MATRIX_BT601_TO_RGB_PC,
    E_SIMMI_MODULE_MATRIX_BT709_TO_RGB_PC,
    E_SIMMI_MODULE_MATRIX_RGB_TO_BT601_PC,
    E_SIMMI_MODULE_MATRIX_RGB_TO_BT709_PC,
    E_SIMMI_MODULE_MATRIX_MAX
}SimMiModulePictureCSCType_e;

typedef struct
{
    u16 u16Hpw;
    u16 u16Hbp;
    u16 u16Hactive;
    u16 u16Htt;
    u16 u16Vpw;
    u16 u16Vbp;
    u16 u16Vactive;
    u16 u16Vtt;
    u16 u16Fps;
}SimMiModuleUserTimingCfg_t;

typedef struct
{
    SimMiModuleDeviceId_e enDeviceId;
    SimMiModuleOuputInterface_e enInterface[E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_NUM];
    SimMiModuleOuptTiming_e enTiming[E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_NUM];
    SimMiModuleUserTimingCfg_t stUserTimingCfg[E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_NUM];
}SimMiModuleDeviceCfg_t;


typedef struct
{
    unsigned short            u16X;
    unsigned short            u16Y;
    unsigned short            u16Width;
    unsigned short            u16Height;
}SimMiModuleRectConfig_t;


typedef struct
{
    SimMiModuleVideoLayerId_e enVideoLayerId;
    SimMiModulePixelFormat_e  enPixelFormat;
    SimMiModuleRectConfig_t   stDispRect;
    unsigned short            u16Width;
    unsigned short            u16Height;
}SimMiModuleVideoLayerConfig_t;


typedef struct
{
    unsigned char             u8PortId;
    SimMiModulePixelFormat_e  enPixelFormat;
    unsigned short            u16X;
    unsigned short            u16Y;
    unsigned short            u16Width;
    unsigned short            u16Height;
    unsigned long             u32BaseAddr;
    unsigned long             u32Stride;
}SimMiModuleInputPortConfig_t;


typedef struct
{   u8 u8DeivceNum;
    SimMiModuleDeviceCfg_t stDeviceCfg[E_SIMMI_MODULE_DEVICE_ID_NUM];
    u8 u8VideoLayerNum;
    SimMiModuleVideoLayerConfig_t stVideoLayerCfg[E_SIMMI_MODULE_VIDEOLAYER_ID_NUM];
    unsigned short u16InputPortFlag[E_SIMMI_MODULE_VIDEOLAYER_ID_NUM];
    SimMiModulePixelFormat_e enPixelFormat[E_SIMMI_MODULE_VIDEOLAYER_ID_NUM];
    SimMiModuleInputPortConfig_t stInputPortCfg[E_SIMMI_MODULE_VIDEOLAYER_ID_NUM][16];
    bool bAttached;
    bool bMgwin32En;
    bool bHVSwap;
    u32 u32DemoTimeSec;
    bool bIsr;
    bool bLCD;
    u32  u32ImageBufferAddr;
    u32  u32ImageBufferSize;
}SimMiModuleDispConfig_t;

typedef struct
{
    SimMiModuleOuputInterface_e enInterface;
    SimMiModulePictureCSCType_e enCscType;
    u32 u32Contrast;
    u32 u32Hue;
    u32 u32Saturation;
    u32 u32Brightness;
    u32 u32Sharpness;
    u32 u32Gain;
}SimMiModuleDispPictureConfig_t;

typedef struct
{
    SimMiModuleDebugType_e enType;
    unsigned long u32Flag;
}SimMiModuleDispDebguConfig_t;

typedef struct
{
    SimMiModuleHdmitxColorType_e enInColorType;
    SimMiModuleHdmitxColorType_e enOutColorType;
    SimMiModuleHdmitxColorDepthType_e enColorDepth;
}SimMiModuleDispHdmitxConfig_t;

//=============================================================================
// IOCTRL defines
//=============================================================================

#endif //

/** @} */ // end of scldma_group
