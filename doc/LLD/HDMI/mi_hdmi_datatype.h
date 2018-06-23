//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>

#ifndef _MI_HDMI_DATATYPE_H_
#define _MI_HDMI_DATATYPE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "mi_common.h"

/*-------------------------------------------------------------------------------------------------
 * Defines
 ------------------------------------------------------------------------------------------------*/
#define MI_HDMI_MAX_ACAP_CNT 16
#define MI_HDMI_MAX_AUDIO_SAMPLE_RATE_CNT 10

#define BLOCK0_INDEX 0
#define BLOCK1_INDEX 1
#define BLOCK_SIZE 128 /* Size per block */

/*-------------------------------------------------------------------------------------------------
 * Structures
 ------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_MI_HDMI_ID_0 = 0,
    E_MI_HDMI_ID_MAX
} MI_HDMI_DeviceId_e;

typedef enum
{
    E_MI_HDMI_EVENT_HOTPLUG = 0,
    E_MI_HDMI_EVENT_NO_PLUG,
    E_MI_HDMI_EVENT_MAX
} MI_HDMI_EventType_e;

typedef enum
{
    E_MI_HDMI_TIMING_480_60I         = 0,
    E_MI_HDMI_TIMING_480_60P         = 1,
    E_MI_HDMI_TIMING_576_50I         = 2,
    E_MI_HDMI_TIMING_576_50P         = 3,
    E_MI_HDMI_TIMING_720_50P         = 4,
    E_MI_HDMI_TIMING_720_60P         = 5,
    E_MI_HDMI_TIMING_1080_50I        = 6,
    E_MI_HDMI_TIMING_1080_50P        = 7,
    E_MI_HDMI_TIMING_1080_60I        = 8,
    E_MI_HDMI_TIMING_1080_60P        = 9,
    E_MI_HDMI_TIMING_1080_30P        = 10,
    E_MI_HDMI_TIMING_1080_25P        = 11,
    E_MI_HDMI_TIMING_1080_24P        = 12,
    E_MI_HDMI_TIMING_4K2K_30P        = 13,
    E_MI_HDMI_TIMING_1440_50P        = 14,
    E_MI_HDMI_TIMING_1440_60P        = 15,
    E_MI_HDMI_TIMING_1440_24P        = 16,
    E_MI_HDMI_TIMING_1440_30P        = 17,
    E_MI_HDMI_TIMING_1470_50P        = 18,
    E_MI_HDMI_TIMING_1470_60P        = 19,
    E_MI_HDMI_TIMING_1470_24P        = 20,
    E_MI_HDMI_TIMING_1470_30P        = 21,
    E_MI_HDMI_TIMING_1920x2205_24P   = 22,
    E_MI_HDMI_TIMING_1920x2205_30P   = 23,
    E_MI_HDMI_TIMING_4K2K_25P        = 24,
    E_MI_HDMI_TIMING_4K1K_60P        = 25,
    E_MI_HDMI_TIMING_4K2K_60P        = 26,
    E_MI_HDMI_TIMING_4K2K_24P        = 27,
    E_MI_HDMI_TIMING_4K2K_50P        = 28,
    E_MI_HDMI_TIMING_2205_24P        = 29,
    E_MI_HDMI_TIMING_4K1K_120P       = 30,
    E_MI_HDMI_TIMING_4096x2160_24P   = 31,
    E_MI_HDMI_TIMING_4096x2160_25P   = 32,
    E_MI_HDMI_TIMING_4096x2160_30P   = 33,
    E_MI_HDMI_TIMING_4096x2160_50P   = 34,
    E_MI_HDMI_TIMING_4096x2160_60P   = 35,
    E_MI_HDMI_TIMING_1024x768_60P    = 36,
    E_MI_HDMI_TIMING_1280x1024_60P   = 37,
    E_MI_HDMI_TIMING_1440x900_60P    = 38,
    E_MI_HDMI_TIMING_1600x1200_60P   = 39,
    E_MI_HDMI_TIMING_MAX,
} MI_HDMI_TimingType_e;

typedef enum
{
    E_MI_HDMI_COLOR_TYPE_RGB444 = 0,
    E_MI_HDMI_COLOR_TYPE_YCBCR422,
    E_MI_HDMI_COLOR_TYPE_YCBCR444,
    E_MI_HDMI_COLOR_TYPE_YCBCR420,
    E_MI_HDMI_COLOR_TYPE_MAX
} MI_HDMI_ColorType_e;

typedef enum
{
    E_MI_HDMI_VIDEO_HDMI = 0,
    E_MI_HDMI_VIDEO_HDMI_HDCP,
    E_MI_HDMI_VIDEO_DVI,
    E_MI_HDMI_VIDEO_DVI_HDCP,
    E_MI_HDMI_VIDEO_MAX,
} MI_HDMI_OutputMode_e;

typedef enum
{
    E_MI_HDMI_DEEP_COLOR_24BIT = 0x00,
    E_MI_HDMI_DEEP_COLOR_30BIT,
    E_MI_HDMI_DEEP_COLOR_36BIT,
    E_MI_HDMI_DEEP_COLOR_48BIT,
    E_MI_HDMI_DEEP_COLOR_MAX,
    E_MI_HDMI_DEEP_COLOR_OFF   = 0xFE,
}MI_HDMI_DeepColor_e;

typedef enum
{
    E_MI_HDMI_AUDIO_SAMPLERATE_UNKNOWN      = 0,
    E_MI_HDMI_AUDIO_SAMPLERATE_32K          = 1,
    E_MI_HDMI_AUDIO_SAMPLERATE_44K          = 2,
    E_MI_HDMI_AUDIO_SAMPLERATE_48K          = 3,
    E_MI_HDMI_AUDIO_SAMPLERATE_88K          = 4,
    E_MI_HDMI_AUDIO_SAMPLERATE_96K          = 5,
    E_MI_HDMI_AUDIO_SAMPLERATE_176K         = 6,
    E_MI_HDMI_AUDIO_SAMPLERATE_192K         = 7,
    E_MI_HDMI_AUDIO_SAMPLERATE_MAX,
} MI_HDMI_SampleRate_e;

typedef enum
{
    E_MI_HDMI_BIT_DEPTH_UNKNOWN =0,
    E_MI_HDMI_BIT_DEPTH_8    = 8,
    E_MI_HDMI_BIT_DEPTH_16  = 16,
    E_MI_HDMI_BIT_DEPTH_18  = 18,
    E_MI_HDMI_BIT_DEPTH_20  = 20,
    E_MI_HDMI_BIT_DEPTH_24  = 24,
    E_MI_HDMI_BIT_DEPTH_32  = 32,
    E_MI_HDMI_BIT_DEPTH_MAX
} MI_HDMI_BitDepth_e;

typedef enum
{
    E_MI_HDMI_ACODE_PCM =0,
    E_MI_HDMI_ACODE_NON_PCM,
    E_MI_HDMI_ACODE_MAX
} MI_HDMI_AudioCodeType_e;

typedef enum
{
    E_MI_INFOFRAME_TYPE_AVI = 0,
    E_MI_INFOFRAME_TYPE_SPD,
    E_MI_INFOFRAME_TYPE_AUDIO,
    E_MI_INFOFRAME_TYPE_MPEG,
    E_MI_INFOFRAME_TYPE_VENDORSPEC,
    E_MI_INFOFRAME_TYPE_MAX
} MI_HDMI_InfoFrameType_e;

typedef enum
{
    E_MI_HDMI_AUDIO_CODING_REFER_STREAM_HEAD = 0,
    E_MI_HDMI_AUDIO_CODING_PCM,
    E_MI_HDMI_AUDIO_CODING_AC3,
    E_MI_HDMI_AUDIO_CODING_MPEG1,
    E_MI_HDMI_AUDIO_CODING_MP3,
    E_MI_HDMI_AUDIO_CODING_MPEG2,
    E_MI_HDMI_AUDIO_CODING_AAC,
    E_MI_HDMI_AUDIO_CODING_DTS,
    E_MI_HDMI_AUDIO_CODING_DDPLUS,
    E_MI_HDMI_AUDIO_CODING_MLP,
    E_MI_HDMI_AUDIO_CODING_WMA,
    E_MI_HDMI_AUDIO_CODING_MAX
} MI_HDMI_AudioCodingType_e;

typedef enum
{
   E_MI_HDMI_BAR_INFO_NOT_VALID = 0,             /**< Bar Data not valid */
   E_MI_HDMI_BAR_INFO_V,                         /**< Vertical bar data valid */
   E_MI_HDMI_BAR_INFO_H,                         /**< Horizental bar data valid */
   E_MI_HDMI_BAR_INFO_VH                         /**< Horizental and Vertical bar data valid */
} MI_HDMI_BarInfo_e;

typedef enum
{
    E_MI_HDMI_SCAN_INFO_NO_DATA      = 0,        /**< No Scan information*/
    E_MI_HDMI_SCAN_INFO_OVERSCANNED  = 1,        /**< Scan information, Overscanned (for television) */
    E_MI_HDMI_SCAN_INFO_UNDERSCANNED = 2,        /**< Scan information, Underscanned (for computer) */
    E_MI_HDMI_SCAN_INFO_FUTURE
} MI_HDMI_ScanInfo_e;

typedef enum
{
    E_MI_HDMI_PICTURE_NON_UNIFORM_SCALING = 0,   /**< No Known, non-uniform picture scaling  */
    E_MI_HDMI_PICTURE_SCALING_H,                 /**< Picture has been scaled horizentally */
    E_MI_HDMI_PICTURE_SCALING_V,                 /**< Picture has been scaled Vertically */
    E_MI_HDMI_PICTURE_SCALING_HV                 /**< Picture has been scaled Horizentally and Vertically   */
 } MI_HDMI_PictureScaling_e;

typedef enum
{
    E_MI_HDMI_COLORIMETRY_NO_DATA = 0,
    E_MI_HDMI_COLORIMETRY_ITU601,
    E_MI_HDMI_COLORIMETRY_ITU709,
    E_MI_HDMI_COLORIMETRY_EXTENDED,
    E_MI_HDMI_COLORIMETRY_XVYCC_601,
    E_MI_HDMI_COLORIMETRY_XVYCC_709
} MI_HDMI_ColorSpace_e;

typedef enum
{
    E_MI_HDMI_ASPECT_RATIO_UNKNOWN = 0,       /**< unknown aspect ratio */
    E_MI_HDMI_ASPECT_RATIO_4TO3,              /**< 4:3 */
    E_MI_HDMI_ASPECT_RATIO_16TO9,             /**< 16:9 */
    E_MI_HDMI_ASPECT_RATIO_SQUARE,            /**< square */
    E_MI_HDMI_ASPECT_RATIO_14TO9,             /**< 14:9 */
    E_MI_HDMI_ASPECT_RATIO_221TO1,            /**< 221:100 */
    E_MI_HDMI_ASPECT_RATIO_ZOME,              /**< default not support, use source's aspect ratio to display */
    E_MI_HDMI_ASPECT_RATIO_FULL,              /**< default not support, full screen display */
    E_MI_HDMI_ASPECT_RATIO_BUTT
} MI_HDMI_AspectRatio_e;

typedef enum
{
    E_MI_HDMI_RGB_QUANTIZATION_DEFAULT_RANGE = 0,    /**< Defaulr range, it depends on the video format */
    E_MI_HDMI_RGB_QUANTIZATION_LIMITED_RANGE,        /**< Limited quantization range of 220 levels when receiving a CE video format*/
    E_MI_HDMI_RGB_QUANTIZATION_FULL_RANGE            /**< Full quantization range of 256 levels when receiving an IT video format*/
} MI_HDMI_RgbQuanRage_e;

typedef enum
{
    E_MI_HDMI_YCC_QUANTIZATION_LIMITED_RANGE = 0,    /**< Limited quantization range of 220 levels when receiving a CE video format*/
    E_MI_HDMI_YCC_QUANTIZATION_FULL_RANGE            /**< Full quantization range of 256 levels when receiving an IT video format*/
} MI_HDMI_YccQuanRage_e;

typedef enum
{
    E_MI_HDMI_CONTNET_GRAPHIC = 0,               /**< Graphics type*/
    E_MI_HDMI_CONTNET_PHOTO,                     /**< Photo type*/
    E_MI_HDMI_CONTNET_CINEMA,                    /**< Cinema type*/
    E_MI_HDMI_CONTNET_GAME                       /**< Game type*/
} MI_HDMI_ContentType_e;

typedef MI_S32 (* MI_HDMI_EventCallBack)(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e event, void *pEventParam, void *pUsrParam);

typedef struct MI_HDMI_InitParam_s
{
    MI_HDMI_EventCallBack pfnHdmiEventCallback;
    void *pCallBackArgs;
} MI_HDMI_InitParam_t;

typedef struct MI_HDMI_VideoAttr_s
{
    MI_BOOL bEnableVideo;
    MI_HDMI_TimingType_e eTimingType;
    MI_HDMI_OutputMode_e eOutputMode;
    MI_HDMI_ColorType_e eColorType;
    MI_HDMI_DeepColor_e eDeepColorMode;
} MI_HDMI_VideoAttr_t;

typedef struct MI_HDMI_AudioAttr_s
{
    MI_BOOL bEnableAudio;
    MI_BOOL bIsMultiChannel;// 0->2channel 1->8channel
    MI_HDMI_SampleRate_e eSampleRate;
    MI_HDMI_BitDepth_e eBitDepth;
    MI_HDMI_AudioCodeType_e eCodeType;
} MI_HDMI_AudioAttr_t;

typedef struct MI_HDMI_EnInfoFrame_s
{
    MI_BOOL bEnableAviInfoFrame;
    MI_BOOL bEnableAudInfoFrame;
    MI_BOOL bEnableSpdInfoFrame;
    MI_BOOL bEnableMpegInfoFrame;
} MI_HDMI_EnInfoFrame_t;

typedef struct MI_HDMI_Attr_s
{
    MI_BOOL bConnect;
    MI_HDMI_VideoAttr_t stVideoAttr;
    MI_HDMI_AudioAttr_t stAudioAttr;
    MI_HDMI_EnInfoFrame_t stEnInfoFrame;
} MI_HDMI_Attr_t;

typedef struct MI_HDMI_Edid_s
{
    MI_BOOL bEdidValid;
    MI_U32 u32Edidlength;
    MI_U8 au8Edid[512]; /* EDID buffer */
} MI_HDMI_Edid_t;

typedef struct MI_HDMI_Sink_Info_s
{
    MI_BOOL bConnected;
    MI_BOOL bSupportHdmi;
    MI_HDMI_TimingType_e eNativeTimingType;
    MI_BOOL abVideoFmtSupported[E_MI_HDMI_TIMING_MAX];
    MI_BOOL bSupportYCbCr444;
    MI_BOOL bSupportYCbCr222;
    MI_BOOL bSupportYCbCr;
    MI_BOOL bSupportxvYcc601;
    MI_BOOL bSupportxvYcc709;
    MI_U8   u8MdBit;
    MI_BOOL abAudioFmtSupported[MI_HDMI_MAX_ACAP_CNT];
    MI_U32  au32AudioSampleRateSupported[MI_HDMI_MAX_AUDIO_SAMPLE_RATE_CNT];
    MI_U32  u32MaxPcmChannels;
    MI_U8   u8Speaker;
    MI_U8   au8IdManufactureName[4];
    MI_U32  u32IdProductCode;
    MI_U32  u32IdSerialNumber;
    MI_U32  u32WeekOfManufacture;
    MI_U32  u32YearOfManufacture;
    MI_U8 u8Version;
    MI_U8 u8Revision;
    MI_U8 u8EdidExternBlockNum;
    MI_U8 au8IeeRegId[3];
    MI_U8 u8PhyAddr_A;
    MI_U8 u8PhyAddr_B;
    MI_U8 u8PhyAddr_C;
    MI_U8 u8PhyAddr_D;
    MI_BOOL bSupportDviDual;
    MI_BOOL bSupportDeepColorYcbcr444;
    MI_BOOL bSupportDeepColor30Bit;
    MI_BOOL bSupportDeepColor36Bit;
    MI_BOOL bSupportDeepColor48Bit;
    MI_BOOL bSupportAi;
    MI_U32  u8MaxTmdsClock;
    MI_BOOL bILatencyFieldsPresent;
    MI_BOOL bLatencyFieldsPresent;
    MI_BOOL bHdmiVideoPresent;
    MI_U8 u8VideoLatency;
    MI_U8 u8AudioLatency;
    MI_U8 u8InterlacedVideoLatency;
    MI_U8 u8InterlacedAudioLatency;
} MI_HDMI_SinkInfo_t;

typedef struct MI_HDMIAviInforFrameVer2_s
{
    MI_HDMI_TimingType_e eTimingType;
    MI_HDMI_OutputMode_e eOutputMode;
    MI_BOOL bActiveInfoPresent;
    MI_HDMI_BarInfo_e eBarInfo;
    MI_HDMI_ScanInfo_e eScanInfo;
    MI_HDMI_ColorSpace_e eColorimetry;
    MI_HDMI_AspectRatio_e eAspectRatio;
    MI_HDMI_AspectRatio_e eActiveAspectRatio;
    MI_HDMI_RgbQuanRage_e eRgbQuantization;
    MI_BOOL bIsItContent;
    MI_U32  u32PixelRepetition;
    MI_HDMI_ContentType_e eContentType;
    MI_HDMI_YccQuanRage_e eYccQuantization;
    MI_U32 u32LineNEndofTopBar;
    MI_U32 u32LineNStartofBotBar;
    MI_U32 u32PixelNEndofLeftBar;
    MI_U32 u32PixelNStartofRightBar;
} MI_HDMI_AviInfoFrameVer_t;

typedef struct MI_HDMI_AudInfoFrameVer1_s
{
    MI_U32 u32ChannelCount;
    MI_HDMI_AudioCodingType_e eAudioCodingType;
    MI_U32 u32SampleSize;
    MI_U32 u32SamplingFrequency;
    MI_U32 u32ChannelAlloc;
    MI_U32 u32LevelShift;
    MI_BOOL bDownMixInhibit;
} MI_HDMI_AudInfoFrameVer_t;

typedef struct MI_HDMI_SpdInfoFrame_s
{
    MI_U8 au8VendorName[8];
    MI_U8 au8ProductDescription[16];
} MI_HDMI_SpdInfoFrame_t;

typedef struct MI_HDMI_MpegSourceInfoFrame_s
{
    MI_U32  u32MpegBitRate;
    MI_BOOL bIsFieldRepeated;
} MI_HDMI_MpegSourceInfoFrame_t;

typedef struct MI_HDMI_VendorSpecInfoFrame_s
{
    MI_U32 u32RegistrationId;
} MI_HDMI_VendorSpecInfoFrame_t;

typedef union
{
    MI_HDMI_AviInfoFrameVer_t     stAviInfoFrame;
    MI_HDMI_AudInfoFrameVer_t     stAudInfoFrame;
    MI_HDMI_SpdInfoFrame_t        stSpdInfoFrame;
    MI_HDMI_MpegSourceInfoFrame_t stMpegSourceInfoFrame;
    MI_HDMI_VendorSpecInfoFrame_t stVendorSpecInfoFrame;
} MI_HDMI_InfoFrameUnit_u;

typedef struct MI_HDMI_InfoFrame_s
{
    MI_HDMI_InfoFrameType_e eInfoFrameType; /* InfoFrame type */
    MI_HDMI_InfoFrameUnit_u unInforUnit;    /* InfoFrame date */
}MI_HDMI_InfoFrame_t;

#ifdef __cplusplus
}
#endif
#endif //_MI_HDMI_DATATYPE_H_
