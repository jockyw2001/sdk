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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   vdec_api.c
/// @brief vdec module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "mi_syscall.h"
#include "mi_print.h"
#include <string.h>

#include "mi_hdmi.h"
#include "hdmi_ioctl.h"
#include <pthread.h>
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
MI_MODULE_DEFINE(hdmi)

#define ExecFunc(_func_, _ret_) \
    if (_func_ != _ret_)\
    {\
        DBG_ERR("[%s][%d]exec function failed\n", __FILE__, __LINE__);\
        return 1;\
    }\
    else\
    {\
        DBG_INFO("[%s][%d]exec function pass\n", __FILE__, __LINE__);\
    }

#define HDMI_API_ISVALID_POINT(X)  \
    {   \
        if( X == NULL)  \
        {   \
            DBG_ERR("MI_ERR_INVALID_PARAMETER!\n");  \
            return MI_ERR_HDMI_INVALID_PARAM;   \
        }   \
    }   \

#define MI_HDMI_BLOCK_SIZE 128 /* Size per block */

#define MI_HDMI_XVYCC601            0x01
#define MI_HDMI_XVYCC709            0x02

#define MI_HDMI_STANDARD_TIMING_SIZE 12
//Aspect ratio
#define MI_HDMI_ASPECT_4       1  // 4:3
#define MI_HDMI_ASPECT_16      2  // 16:9
#define MI_HDMI_ASPECT_4or16   3  // 16:9
#define MI_HDMI_ASPECT_16_10   4  // 16:10
#define MI_HDMI_ASPECT_5_4     5  // 5:4

#define MI_HDMI_EXT_BLOCK_VER_TAG 0x2
#define MI_HDMI_EXT_BLOCK_REVERSION 0x3

//Data block flag
#define MI_HDMI_AUDIO_DATA_BLOCK    0x01
#define MI_HDMI_VIDEO_DATA_BLOCK    0x02
#define MI_HDMI_VENDOR_DATA_BLOCK   0x03
#define MI_HDMI_SPEAKER_DATA_BLOCK  0x04
#define MI_HDMI_VESA_DTC_DATA_BLOCK 0x05
#define MI_HDMI_USE_EXT_DATA_BLOCK  0x07
#define MI_HDMI_DATA_BLOCK_LENGTH   0x1F
#define MI_HDMI_DATA_BLOCK_TAG_CODE 0xE0
#define MI_HDMI_AUDIO_FORMAT_CODE   0x78
#define MI_HDMI_AUDIO_MAX_CHANNEL   0x07

#define MI_HDMI_VIDEO_CAPABILITY_DATA_BLOCK      0x00
#define MI_HDMI_VENDOR_SPECIFIC_VIDEO_DATA_BLOCK 0x01
#define MI_HDMI_RESERVED_VESA_DISPLAY_DEVICE     0x02
#define MI_HDMI_RESERVED_VESA_VIDEO_DATA_BLOCK   0x03
#define MI_HDMI_RESERVED_HDMI_VIDEO_DATA_BLOCK   0x04
#define MI_HDMI_COLORIMETRY_DATA_BLOCK           0x05
#define MI_HDMI_CEA_MISCELLANENOUS_AUDIO_FIELDS  0x10
#define MI_HDMI_VENDOR_SPECIFIC_AUDIO_DATA_BLOCK 0x11
#define MI_HDMI_RESERVED_HDMI_AUDIO_DATA_BLOCK   0x12

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct MI_HDMI_BlockZero_s
{
    MI_U8 au8Header[8];
    MI_U8 au8ManufactureId[2]; //Manufacture
    MI_U8 au8ProductCode[2];
    MI_U8 au8SerialNumber[4];
    MI_U8 u8ManufactureWeek;
    MI_U8 u8ManufactureYear;
    MI_U8 u8Version;
    MI_U8 u8Revision;
    MI_U8 u8Input;
    MI_U8 u8MaxWidthCm;
    MI_U8 u8MaxHeightCm;
    MI_U8 u8Gamma;
    MI_U8 u8Features;
    MI_U8 u8RedGreenLow;
    MI_U8 u8BlackWhiteLow;
    MI_U8 u8RedX;
    MI_U8 u8RedY;
    MI_U8 u8GreenX;
    MI_U8 u8GreenY;
    MI_U8 u8BlueX;
    MI_U8 u8BlueY;
    MI_U8 u8WhiteX;
    MI_U8 u8WhiteY;
    MI_U8 au8EstablishTiming[3]; //35 36 37
    MI_U8 au8StdTiming[16]; //38-53
    MI_U8 au8DetailedTiming[36]; //54-89£¬36Byte
    MI_U8 au8MonitorDescr[36]; //90-125£¬36Byte
    MI_U8 u8ExtBlocks; //126
    MI_U8 u8CheckSum; //127
} MI_HDMI_BlockZero_t;

typedef struct MI_HDMI_OptimumResolution_s
{
    MI_U32 u32Horizontal;
    MI_U32 u32Vertical;
    MI_U32 u32NativeFieldRate;
}MI_HDMI_OptimumResolution_t;

//Explain: H-horizontal,V-vertical
typedef struct MI_HDMI_DetailedTiming_s
{
    MI_U8 au8PixelClk[2];
    MI_U8 u8HActive;
    MI_U8 u8HBlank;
    MI_U8 u8HActiveBlank;
    MI_U8 u8VActive;
    MI_U8 u8VBlank;
    MI_U8 u8VActiveBlank;
    MI_U8 u8HSyncOffset;
    MI_U8 u8HSyncPulseWidth;
    MI_U8 u8VsOffsetPulseWidth;
    MI_U8 u8HsOffsetVsOffset;
    MI_U8 u8HImageSize;
    MI_U8 u8VImageSize;
    MI_U8 u8HVImageSize;
    MI_U8 u8HBorder;
    MI_U8 u8VBorder;
    MI_U8 u8Flags;
} MI_HDMI_DetailedTiming_t;

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------

typedef enum
{
    E_HDMITX_VIC_640x480P_60_4_3         = 1,
    E_HDMITX_VIC_720x480P_60_4_3         = 2,
    E_HDMITX_VIC_720x480P_60_16_9        = 3,
    E_HDMITX_VIC_1280x720P_60_16_9       = 4,
    E_HDMITX_VIC_1920x1080I_60_16_9      = 5,
    E_HDMITX_VIC_720x480I_60_4_3         = 6,
    E_HDMITX_VIC_720x480I_60_16_9        = 7,
    E_HDMITX_VIC_720x240P_60_4_3         = 8,
    E_HDMITX_VIC_720x240P_60_16_9        = 9,
    E_HDMITX_VIC_2880x480I_60_4_3        = 10,
    E_HDMITX_VIC_2880x480I_60_16_9       = 11,
    E_HDMITX_VIC_2880x240P_60_4_3        = 12,
    E_HDMITX_VIC_2880x240P_60_16_9       = 13,
    E_HDMITX_VIC_1440x480P_60_4_3        = 14,
    E_HDMITX_VIC_1440x480P_60_16_9       = 15,
    E_HDMITX_VIC_1920x1080P_60_16_9      = 16,
    E_HDMITX_VIC_720x576P_50_4_3         = 17,
    E_HDMITX_VIC_720x576P_50_16_9        = 18,
    E_HDMITX_VIC_1280x720P_50_16_9       = 19,
    E_HDMITX_VIC_1920x1080I_50_16_9      = 20,
    E_HDMITX_VIC_720x576I_50_4_3         = 21,
    E_HDMITX_VIC_720x576I_50_16_9        = 22,
    E_HDMITX_VIC_720x288P_50_4_3         = 23,
    E_HDMITX_VIC_720x288P_50_16_9        = 24,
    E_HDMITX_VIC_2880x576I_50_4_3        = 25,
    E_HDMITX_VIC_2880x576I_50_16_9       = 26,
    E_HDMITX_VIC_2880x288P_50_4_3        = 27,
    E_HDMITX_VIC_2880x288P_50_16_9       = 28,
    E_HDMITX_VIC_1440x576P_50_4_3        = 29,
    E_HDMITX_VIC_1440x576P_50_16_9       = 30,
    E_HDMITX_VIC_1920x1080P_50_16_9      = 31,
    E_HDMITX_VIC_1920x1080P_24_16_9      = 32,
    E_HDMITX_VIC_1920x1080P_25_16_9      = 33,
    E_HDMITX_VIC_1920x1080P_30_16_9      = 34,
    E_HDMITX_VIC_2880x480P_60_4_3        = 35,
    E_HDMITX_VIC_2880x480P_60_16_9       = 36,
    E_HDMITX_VIC_2880x576P_50_4_3        = 37,
    E_HDMITX_VIC_2880x576P_50_16_9       = 38,
    E_HDMITX_VIC_1280x720P_30_16_9       = 62,
    E_HDMITX_VIC_3840x2160P_30_16_9      = 95,
    E_HDMITX_VIC_3840x2160P_50_16_9      = 96,
    E_HDMITX_VIC_3840x2160P_60_16_9      = 97,
    E_HDMITX_VIC_4096x2160p_30_256_135   = 100,
    E_HDMITX_VIC_4096x2160p_50_256_135   = 101,
    E_HDMITX_VIC_4096x2160p_60_256_135   = 102,
    E_HDMITX_VIC_MAX,
} MI_HDMITX_VIC_TimingType_e;
/*
typedef enum
{
    E_HDMITX_VIC_720x480p_60_4_3       = 2,
    E_HDMITX_VIC_1280x720p_60_16_9     = 4,
    E_HDMITX_VIC_1920x1080p_60_16_9    = 16,
    E_HDMITX_VIC_720x576p_50_4_3       = 17,
    E_HDMITX_VIC_3840x2160p_50_16_9    = 96,
    E_HDMITX_VIC_3840x2160p_60_16_9    = 97,
    E_HDMITX_VIC_4096x2160p_50_256_135   = 101,
    E_HDMITX_VIC_4096x2160p_60_256_135   = 102,
} MI_HDMITX_VIC_TimingType_e;*/
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------
//static pthread_t _HdmiTid = 0; //Need Colin Support
static MI_BOOL _bThreadRunning = 0;
static MI_HDMI_EventCallBack _gEventCallback = NULL;

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
static void * mi_hdmi_process(void * args)
{
    MI_S32 s32RetVal = MI_SUCCESS;
    MI_BOOL bLastStatus, bCurStatus;
    MI_HDMI_Attr_t stAttr;
    MI_HDMI_EventType_e eEventType = E_MI_HDMI_EVENT_MAX;

    MI_HDMI_GetAttr(E_MI_HDMI_ID_0, &stAttr); //get default hotplug status
    bLastStatus = stAttr.bConnect;
    bCurStatus = bLastStatus;

    while (_bThreadRunning)
    {
        memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
        s32RetVal = MI_HDMI_GetAttr(E_MI_HDMI_ID_0, &stAttr); //get hotplug status
        if (MI_SUCCESS == s32RetVal)
        {
            bCurStatus = stAttr.bConnect;
            if (bLastStatus != bCurStatus)
            {
                if (TRUE == bCurStatus)
                {
                    eEventType = E_MI_HDMI_EVENT_HOTPLUG;
                }
                else
                {
                    eEventType = E_MI_HDMI_EVENT_NO_PLUG;
                }
                if (_gEventCallback)
                {
                    _gEventCallback(E_MI_HDMI_ID_0, eEventType, NULL, NULL);
                }
            }
            bLastStatus = bCurStatus;
        }
        usleep(500*1000); //check once per half second
    }

    return NULL;
}

static MI_S32 _MI_HDMI_CheckHeader(MI_U8 *pu8Data)
{
    MI_U32 u32Index;
    const MI_U8 au8BlockZeroHeader[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};

    HDMI_API_ISVALID_POINT(pu8Data);

    for (u32Index = 0; u32Index < 8; u32Index ++)
    {
        if(pu8Data[u32Index] != au8BlockZeroHeader[u32Index])
        {
            DBG_ERR("Index:%d, 0x%02x\n",u32Index, pu8Data[u32Index]);
            return MI_ERR_HDMI_EDID_HEADER_ERR;
        }
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_ParseVendorInfo(MI_HDMI_BlockZero_t *pstData, MI_HDMI_SinkInfo_t *pSinkInfo)
{
    MI_U16 u16Index, u16Data;

    HDMI_API_ISVALID_POINT(pstData);

    u16Data = (pstData->au8ManufactureId[0] << 8) | (pstData->au8ManufactureId[1]);

    for(u16Index = 0; u16Index < 3; u16Index++)
    {
        pSinkInfo->au8IdManufactureName[2 - u16Index] = ((u16Data & (0x1F << (5 * u16Index))) >> (5 * u16Index));
        if((0 < pSinkInfo->au8IdManufactureName[2 - u16Index])&&\
            (27 > pSinkInfo->au8IdManufactureName[2 - u16Index]))
        {
            pSinkInfo->au8IdManufactureName[2 - u16Index] += 'A' - 1;
        }
        else
        {
            DBG_INFO("Can't parse manufacture name\n");
            return MI_ERR_HDMI_EDID_DATA_ERR;
        }
    }

    pSinkInfo->u32IdProductCode = (pstData->au8ProductCode[1] << 8) | pstData->au8ProductCode[0];
    pSinkInfo->u32IdSerialNumber = (pstData->au8SerialNumber[3] << 24) | (pstData->au8SerialNumber[2] << 16) |
        (pstData->au8SerialNumber[1] << 8) | (pstData->au8SerialNumber[0]);
    pSinkInfo->u32WeekOfManufacture = pstData->u8ManufactureWeek;
    pSinkInfo->u32YearOfManufacture = pstData->u8ManufactureYear + 1990;

    DBG_INFO("Edid info mfg name[%s]\n", pSinkInfo->au8IdManufactureName);
    DBG_INFO("Edid info code:%d\n", pSinkInfo->u32IdProductCode);
    DBG_INFO("Edid info serial:%d\n", pSinkInfo->u32IdSerialNumber);
    DBG_INFO("Edid info year:%d,week:%d\n", pSinkInfo->u32WeekOfManufacture, pSinkInfo->u32YearOfManufacture);

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_ParseEstablishTiming(MI_U8 *pu8Data, MI_HDMI_SinkInfo_t *pSinkInfo)
{
    HDMI_API_ISVALID_POINT(pu8Data);

    if(pu8Data[0] & 0x20)
    {
        DBG_INFO("640 x 480 @ 60Hz\n");
        pSinkInfo->abVideoFmtSupported[E_MI_HDMI_TIMING_480_60P] = TRUE;
    }
    if(pu8Data[0] & 0x01)
    {
        //pSinkInfo->abVideoFmtSupported[800*600]
        DBG_INFO("800 x 600 @ 60Hz\n");
    }
    if(pu8Data[1] & 0x08)
    {
        pSinkInfo->abVideoFmtSupported[E_MI_HDMI_TIMING_1024x768_60P] = TRUE;
        DBG_INFO("1024 x 768 @ 60Hz\n");
    }
    if((!pu8Data[0]) && (!pu8Data[1]) && (!pu8Data[2]))
    {
        DBG_INFO("No established video modes\n");
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_ParseStdTiming(MI_U8 *pu8Data)
{
    MI_U8 u8TmpVal, u8Index;
    MI_U32 u32Hor, u32Ver, u32AspectRatio, u32Freq;

    for(u8Index = 0; u8Index < MI_HDMI_STANDARD_TIMING_SIZE; u8Index += 2)
    {
        if((0x01 == pu8Data[u8Index]) && (0x01 == pu8Data[u8Index + 1]))
        {
            DBG_INFO("Mode %d wasn't defined! \n", (int)pu8Data[u8Index]);
        }
        else
        {
            u32Hor = (pu8Data[u8Index] + 31) * 8;
            DBG_INFO(" Hor Act pixels %d \n", u32Hor);
            u8TmpVal = pu8Data[u8Index + 1] & 0xC0;
            if (0x00 == u8TmpVal)
            {
                DBG_INFO("Aspect ratio:16:10\n");
                u32AspectRatio = MI_HDMI_ASPECT_16_10;
                u32Ver = u32Hor * 10/16;
            }
            else if (0x40 == u8TmpVal)
            {
                DBG_INFO("Aspect ratio:4:3\n");
                u32AspectRatio = MI_HDMI_ASPECT_4;
                u32Ver = u32Hor * 3/4;
            }
            else if (0x80 == u8TmpVal)
            {
                DBG_INFO("Aspect ratio:5:4\n");
                u32AspectRatio = MI_HDMI_ASPECT_5_4;
                u32Ver = u32Hor * 4/5;
            }
            else //0xc0
            {
                DBG_INFO("Aspect ratio:16:9\n");
                u32AspectRatio = MI_HDMI_ASPECT_16;
                u32Ver = u32Hor * 9/16;
            }
            u32Freq = ((pu8Data[u8Index + 1]) & 0x3F) + 60;
            DBG_INFO(" Refresh rate %d Hz \n", u32Freq);
        }
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_ParseYcbcrSupport(MI_U8 *pu8Data, MI_HDMI_SinkInfo_t *pSinkInfo)
{
    HDMI_API_ISVALID_POINT(pu8Data);
    pSinkInfo->bSupportYCbCr422 = (pu8Data[0] & 0X10) ? TRUE : FALSE;
    pSinkInfo->bSupportYCbCr444 = (pu8Data[0] & 0X20) ? TRUE : FALSE;

    return MI_SUCCESS;
}

/* Audio Data Block */
static MI_S32 _MI_HDMI_ParseAudioDataBlock(MI_U8 *pu8Data, MI_U8 u8Len, MI_HDMI_SinkInfo_t *pSinkInfo)
{
	HDMI_API_ISVALID_POINT(pu8Data);
    MI_U16 u8Length = 0, u8Offset = 0;
    MI_U16 i;
    u8Length = u8Offset + u8Len;
    for(i = 0; i < u8Length; i = i+3)
    {
        switch((pu8Data[i] & MI_HDMI_AUDIO_FORMAT_CODE) >> 3) //MI_HDMI_AUDIO_FORMAT_CODE
        {
           case E_MI_HDMI_AUDIO_CODING_PCM:
               pSinkInfo->abAudioFmtSupported[0] = true;//9D
               pSinkInfo->u32MaxPcmChannels = pu8Data[i] & 0x07;//9D
               //pSinkInfo->bSupportPcmSampleSizes16Bit =  (pu8Data[i+2] & 0x01);//9F(157)
               //pSinkInfo->bSupportPcmSampleSizes20Bit =  (pu8Data[i+2] & 0x02) >> 1 ;
               //pSinkInfo->bSupportPcmSampleSizes24Bit =  (pu8Data[i+2] & 0x03) >> 2 ;
               DBG_INFO("0 output E_MI_HDMI_AUDIO_CODING_PCM");
               break;
           case E_MI_HDMI_AUDIO_CODING_AC3:
               pSinkInfo->abAudioFmtSupported[1] = true;
               DBG_INFO("1 output E_MI_HDMI_AUDIO_CODING_AC3");
               break;
           case E_MI_HDMI_AUDIO_CODING_MPEG1:
               pSinkInfo->abAudioFmtSupported[2] = true;
               DBG_INFO("2 output E_MI_HDMI_AUDIO_CODING_MPEG1");
               break;
           case E_MI_HDMI_AUDIO_CODING_MP3:
               pSinkInfo->abAudioFmtSupported[3] = true;
               DBG_INFO("3 output E_MI_HDMI_AUDIO_CODING_MP3");
               break;
           case E_MI_HDMI_AUDIO_CODING_MPEG2:
               pSinkInfo->abAudioFmtSupported[4] = true;
               DBG_INFO("4 output E_MI_HDMI_AUDIO_CODING_MPEG2");
               break;
           case E_MI_HDMI_AUDIO_CODING_AAC:
               pSinkInfo->abAudioFmtSupported[5] = true;
               DBG_INFO("5 output E_MI_HDMI_AUDIO_CODING_AAC");
               break;
           case E_MI_HDMI_AUDIO_CODING_DTS:
               pSinkInfo->abAudioFmtSupported[6] = true;
               DBG_INFO("6 output E_MI_HDMI_AUDIO_CODING_DTS");
               break;
           default:
                DBG_INFO("resvered block tag code define");
                break;
        }

        if((pu8Data[i+1] & 0x01))
        {
           pSinkInfo->au32AudioSampleRateSupported[E_MI_HDMI_AUDIO_SAMPLERATE_32K] = 32000;//9E
        }
        else if((pu8Data[i+1] & 0x02))
        {
           pSinkInfo->au32AudioSampleRateSupported[E_MI_HDMI_AUDIO_SAMPLERATE_44K] = 44100;
        }
        else if((pu8Data[i+1] & 0x04))
        {
           pSinkInfo->au32AudioSampleRateSupported[E_MI_HDMI_AUDIO_SAMPLERATE_48K] = 48000;
        }
        else if((pu8Data[i+1] & 0x08))
        {
           pSinkInfo->au32AudioSampleRateSupported[E_MI_HDMI_AUDIO_SAMPLERATE_88K] = 88200;
        }
        else if((pu8Data[i+1] & 0x10))
        {
           pSinkInfo->au32AudioSampleRateSupported[E_MI_HDMI_AUDIO_SAMPLERATE_96K] = 96000;
        }
        else if((pu8Data[i+1] & 0x20))
        {
           pSinkInfo->au32AudioSampleRateSupported[E_MI_HDMI_AUDIO_SAMPLERATE_176K] = 176400;
        }
        else if((pu8Data[i+1] & 0x40))
        {
           pSinkInfo->au32AudioSampleRateSupported[E_MI_HDMI_AUDIO_SAMPLERATE_192K] = 192000;
        }
     }

    return MI_SUCCESS;
}

/* Video Data Block */
static MI_S32 _MI_HDMI_ParseVideoDataBlock(MI_U8 *pu8Data, MI_U8 u8Len, MI_HDMI_SinkInfo_t *pSinkInfo,MI_U8 u16Dtd)
{
    MI_U16 i = 0, j = 0, k = 0;
    MI_U16 u16MaxVicSupportNum = 107;

    MI_U32 au32Horizontal[u16MaxVicSupportNum];
    MI_U32 au32Vertical[u16MaxVicSupportNum];
    MI_U16 au16P_I[u16MaxVicSupportNum];

    MI_U32 au32SelectH[u16MaxVicSupportNum];
    MI_U32 au32SelectV[u16MaxVicSupportNum];
    MI_U16 au16SelectP_I[u16MaxVicSupportNum];

    MI_U16 u16offset = 0;
    MI_U8 au8NativeTiming[u16MaxVicSupportNum];

    MI_U32 au32NativeH[u16MaxVicSupportNum];
    MI_U32 au32NativeV[u16MaxVicSupportNum];
    MI_U16 au32NativeP_I[u16MaxVicSupportNum];
    MI_U32 au32NativeAspectRatio[u16MaxVicSupportNum];
    MI_U32 au32NativeFieldRate[u16MaxVicSupportNum];
    MI_HDMI_OptimumResolution_t u32OptimumResolution;

    MI_U16 au16NativeVideoFmtSupported[u16MaxVicSupportNum];
    //get video data
    MI_U16 u16StartDtd = u16Dtd;//relative u16offset to bolock1 Detailed Timing 3 //0x8c
	MI_U16 u16AbsoluteDtd = u16StartDtd + 5 + MI_HDMI_BLOCK_SIZE;//absolute u16offset
    for(i = 0 ; i <(256 - u16AbsoluteDtd)/18; i = i++)
    {
        //get au32Horizontal
        au32SelectH[i] = (MI_U32)((((pu8Data[u16StartDtd+4+u16offset] & 0xF0) >> 4) <<8 ) | pu8Data[u16StartDtd+2+u16offset]);
        DBG_INFO("\033[1;33m ##au32Horizontal>>hight8bit[%d]\n",au32SelectH[i]);
        switch(au32SelectH[i])
        {
            case 640:
                au32Horizontal[i] = 640;
                break;
            case 720:
                au32Horizontal[i] = 720;
                break;
            case 1280:
                au32Horizontal[i] = 1280;
                break;
            case 1440:
                au32Horizontal[i] = 1440;
                break;
            case 1920:
                au32Horizontal[i] = 1920;
                break;
            case 2880:
                au32Horizontal[i] = 2880;
                break;
            case 3840:
                au32Horizontal[i] = 3840;
                break;
            case 4096:
                au32Horizontal[i] = 4096;
                break;
            default:
                au32Horizontal[i] = 0;
                break;
        }
        //get au32Vertical
        au32SelectV[i] = (MI_U32)((((pu8Data[u16StartDtd+7+u16offset] & 0xF0) >> 4) <<8) |  pu8Data[u16StartDtd+5+u16offset]);
        DBG_INFO("\033[1;33m ##au32Vertical[%d]\n",au32SelectV[i]);
        switch(au32SelectV[i])
        {
            case 240:
                au32Vertical[i] = 240;
                break;
            case 288:
                au32Vertical[i] = 288;
                break;
            case 480:
                au32Vertical[i] = 480;
                break;
            case 576:
                au32Vertical[i] = 576;
                break;
            case 720:
                au32Vertical[i] = 720;
                break;
            case 1080:
                au32Vertical[i] = 1080;
                break;
            case 2160:
                au32Vertical[i] = 2160;
                break;
            default:
                au32Vertical[i] = 0;
                break;
        }
        //get scanmode
        au16SelectP_I[i] = (MI_U16)(((pu8Data[u16StartDtd+17+u16offset] & 0x80) >>7));
        if(au16SelectP_I[i])
        {
            au16P_I[i] = 1;
        }
        else
        {
            au16P_I[i] = 0;
        }
        u16offset += 18;
    }
    //au8NativeTiming
    for(i = 0; i < u8Len; i++)
    {
        if((pu8Data[i] & 0x80))
        {
            au8NativeTiming[i] = (MI_U16)(pu8Data[i] & 0x7F);//au8NativeTiming ( 4 ¡¢19)
            if(E_HDMITX_VIC_640x480P_60_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_640x480P_60_4_3;
                au32NativeH[i] = 640;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------640x480P_60_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x480P_60_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x480P_60_4_3;
                au32NativeH[i] = 720;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------720x480P_60_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x480P_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x480P_60_16_9;
                au32NativeH[i] = 720;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
               DBG_INFO("\033[1;33m ------720x480P_60_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1280x720P_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_1280x720P_60_16_9;
                au32NativeH[i] = 1280;
                au32NativeV[i] = 720;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------1280x720P_60_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1920x1080I_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_1920x1080I_60_16_9;
                au32NativeH[i] = 1920;
                au32NativeV[i] = 1080;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------1920x1080I_60_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x480I_60_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x480I_60_4_3;
                au32NativeH[i] = 720;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------720x480I_60_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x480I_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x480I_60_16_9;
                au32NativeH[i] = 720;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------720x480I_60_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x240P_60_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x240P_60_4_3;
                au32NativeH[i] = 720;
                au32NativeV[i] = 240;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------720x240P_60_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x240P_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x240P_60_16_9;
                au32NativeH[i] = 720;
                au32NativeV[i] = 240;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------720x240P_60_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x480I_60_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_2880x480I_60_4_3;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------2880x480I_60_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x480I_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_2880x480I_60_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------2880x480I_60_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x240P_60_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_2880x240P_60_4_3;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 240;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------2880x240P_60_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x240P_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_2880x240P_60_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 240;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------2880x240P_60_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1440x480P_60_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_1440x480P_60_4_3;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------1440x480P_60_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1440x480P_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_1440x480P_60_16_9;
                au32NativeH[i] = 1440;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------1440x480P_60_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1920x1080P_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_1920x1080P_60_16_9;
                au32NativeH[i] = 1920;
                au32NativeV[i] = 1080;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------1920x1080P_60_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x576P_50_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x576P_50_4_3;
                au32NativeH[i] = 720;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------720x576P_50_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x576P_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x576P_50_16_9;
                au32NativeH[i] = 720;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------720x576P_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1280x720P_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_1280x720P_50_16_9;
                au32NativeH[i] = 1280;
                au32NativeV[i] = 720;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------1280x720P_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1920x1080I_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_1920x1080I_50_16_9;
                au32NativeH[i] = 1920;
                au32NativeV[i] = 1080;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------1920x1080I_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x576I_50_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x576I_50_4_3;
                au32NativeH[i] = 720;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------720x576I_50_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x576I_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x576I_50_16_9;
                au32NativeH[i] = 720;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------720x576I_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x288P_50_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_4_3;
                au32NativeH[i] = 720;
                au32NativeV[i] = 288;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------720x288P_50_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_720x288P_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 720;
                au32NativeV[i] = 288;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------720x288P_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x576I_50_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------2880x576I_50_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x576I_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 1;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------2880x576I_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x288P_50_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 288;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------2880x288P_50_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x288P_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 288;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------2880x288P_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1440x576P_50_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 1440;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------1440x576P_50_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1440x576P_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 1440;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------2880x576I_50_16_9(%d) \033[0m \n",i);
            }else if(E_HDMITX_VIC_1920x1080P_50_16_9  == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 1920;
                au32NativeV[i] = 1080;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------1920x1080P_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1920x1080P_24_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 1920;
                au32NativeV[i] = 1080;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 24;
                DBG_INFO("\033[1;33m ------1920x1080P_24_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1920x1080P_25_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 1920;
                au32NativeV[i] = 1080;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 25;
                DBG_INFO("\033[1;33m ------2880x576I_25_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1920x1080P_30_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 1920;
                au32NativeV[i] = 1080;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 30;
                DBG_INFO("\033[1;33m ------2880x576I_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x480P_60_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------2880x480P_60_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x480P_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 480;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------2880x480P_60_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x576P_50_4_3 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_4;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------2880x576P_50_4_3(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_2880x576P_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 2880;
                au32NativeV[i] = 576;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------2880x576P_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_1280x720P_30_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 1280;
                au32NativeV[i] = 720;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 30;
                DBG_INFO("\033[1;33m ------1280x720P_30_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_3840x2160P_30_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 3840;
                au32NativeV[i] = 2160;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 30;
                DBG_INFO("\033[1;33m ------3840x2160P_30_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_3840x2160P_50_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 3840;
                au32NativeV[i] = 2160;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 50;
                DBG_INFO("\033[1;33m ------3840x2160P_50_16_9(%d) \033[0m \n",i);
            }
            else if(E_HDMITX_VIC_3840x2160P_60_16_9 == au8NativeTiming[i])
            {
                au16NativeVideoFmtSupported[i] = E_HDMITX_VIC_720x288P_50_16_9;
                au32NativeH[i] = 3840;
                au32NativeV[i] = 2160;
                au32NativeP_I[i] = 0;
                au32NativeAspectRatio[i] = MI_HDMI_ASPECT_16;
                au32NativeFieldRate[i] = 60;
                DBG_INFO("\033[1;33m ------3840x2160P_60_16_9(%d) \033[0m \n",i);
            }
            else
            {
                DBG_INFO("no exit native timing\n");
            }
            i++;
        }
        else
        {
            au32NativeH[i] = 0;
            au32NativeV[i] = 0;
            au32NativeP_I[i] = 0;
        }
    }
    for(i = 0; i < ((256 - u16AbsoluteDtd)/18); i++)
    {
        for(j = 0; j < u8Len; j++)
        {
            if((au32NativeH[j] ==  au32Horizontal[i] ) && (au32NativeV[j] ==  au32Vertical[i] ) && (au32NativeP_I[j] == au16P_I[i])
                && (au32NativeH[j] != 0) && (au32NativeV[j] != 0) && (au32Horizontal[i] != 0) && (au32Vertical[i] != 0)
                && (au32NativeAspectRatio[j] == 2) && (au32NativeFieldRate[j] == 60))
            {
                u32OptimumResolution.u32Horizontal = au32NativeH[j];
                u32OptimumResolution.u32Vertical = au32NativeV[j];
                u32OptimumResolution.u32NativeFieldRate = au32NativeFieldRate[j];
            }
        }
    }
    for(k = 0; k < u8Len; k++)
    {
         if((u32OptimumResolution.u32Horizontal == au32NativeH[k]) && (u32OptimumResolution.u32Vertical == au32NativeV[k])
              && (u32OptimumResolution.u32NativeFieldRate == au32NativeFieldRate[k]))
         {
            pSinkInfo->eNativeTimingType = au16NativeVideoFmtSupported[k];
            DBG_INFO("\031[1;31m ##>>>>>pSinkInfo->eNativeTimingType:(%d) <<< \031[0m \n",pSinkInfo->eNativeTimingType);
         }
    }
    return MI_SUCCESS;
}

/* Vendor Specific Data Block */
static MI_S32 _MI_HDMI_ParseVendorSpecDataBlock(MI_U8 *pu8Data, MI_U8 u8Len, MI_HDMI_SinkInfo_t *pSinkInfo)
{
    pSinkInfo->au8IeeRegId[0] = pu8Data[0];
    pSinkInfo->au8IeeRegId[1] = pu8Data[1];
    pSinkInfo->au8IeeRegId[2] = pu8Data[2];
    if ((0x03 == pu8Data[0]) && (0x0c == pu8Data[1]) && (0x00 == pu8Data[2]))
    {
        DBG_INFO("This is HDMI Device\n");
        pSinkInfo->bSupportHdmi = TRUE;
    }
    else
    {
        pSinkInfo->bSupportHdmi = FALSE;
        DBG_INFO("This is DVI Device, we don't parse it\n");
        return MI_ERR_HDMI_EDID_DATA_ERR;
    }

    if (u8Len < 4)
    {
        DBG_INFO("len:%d\n", u8Len);
        return MI_SUCCESS;
    }
    pSinkInfo->u8PhyAddr_A = (pu8Data[3] & 0xF0) >> 4;
    pSinkInfo->u8PhyAddr_B = (pu8Data[3] & 0x0F);
    pSinkInfo->u8PhyAddr_C = (pu8Data[4] & 0xF0) >> 4;
    pSinkInfo->u8PhyAddr_D = (pu8Data[4] & 0x0F);

    if(u8Len < 6)
    {
        DBG_INFO("len:%d\n", u8Len);
        return MI_SUCCESS;
    }
    pSinkInfo->bSupportDviDual = (pu8Data[5] & 0x01);
    pSinkInfo->bSupportAi = (pu8Data[5] & 0x80) >> 7;
    pSinkInfo->bSupportDeepColorYcbcr444 = (pu8Data[5] & 0x08) >> 3;
    pSinkInfo->bSupportDeepColor30Bit = (pu8Data[5] & 0x10) >> 4;
    pSinkInfo->bSupportDeepColor36Bit = (pu8Data[5] & 0x20) >> 5;
    pSinkInfo->bSupportDeepColor48Bit = (pu8Data[5] & 0x40) >> 6;

    if(u8Len < 7)
    {
        DBG_INFO("len:%d\n", u8Len);
        return MI_SUCCESS;
    }
    pSinkInfo->u8MaxTmdsClock = (pu8Data[6] & 0xff) * 5;

    if(u8Len < 8)
    {
        DBG_INFO("len:%d\n", u8Len);
        return MI_SUCCESS;
    }
    DBG_INFO("support 3d:%d\n", (pu8Data[7] & 0x20) >> 5);
    DBG_INFO("CNC:%d\n", (pu8Data[7] & 0x0F));

    pSinkInfo->bLatencyFieldsPresent = (pu8Data[7] & 0x80) >> 7;
    pSinkInfo->bILatencyFieldsPresent = (pu8Data[7] & 0x40) >> 6;
    pSinkInfo->bHdmiVideoPresent = (pu8Data[7] & 0x20) >> 5;
    if(u8Len < 9)
    {
        DBG_INFO("len:%d\n", u8Len);
        return MI_SUCCESS;
    }
    pSinkInfo->u8VideoLatency = (pu8Data[8] & 0xff);
    pSinkInfo->u8AudioLatency = (pu8Data[9] & 0xff);
    pSinkInfo->u8InterlacedVideoLatency = (pu8Data[10] & 0xff);
    pSinkInfo->u8InterlacedVideoLatency = (pu8Data[11] & 0xff);

    return MI_SUCCESS;
}

/* Vendor Speaker Data Block */
static MI_S32 _MI_HDMI_ParseSpeakerDataBlock(MI_U8 *pu8Data, MI_U8 u8Len, MI_HDMI_SinkInfo_t *pSinkInfo)
{
    DBG_INFO("Speaker Data[0]:0x%x Data[0]:0x%x\n", pu8Data[0], pu8Data[1]);
    if ((0 != pu8Data[0]) || (0 != pu8Data[1]))
    {
        pSinkInfo->u8Speaker = TRUE;
    }

    return MI_SUCCESS;
}

/* Extern Data Block */
static MI_S32 _MI_HDMI_ParseExtDataBlock(MI_U8 *pu8Data, MI_U8 u8Len, MI_HDMI_SinkInfo_t *pSinkInfo)
{
    switch(pu8Data[0])
    {
        case MI_HDMI_VIDEO_CAPABILITY_DATA_BLOCK:
            DBG_INFO("Video Capability Data Block\n");
            break;
        case MI_HDMI_VENDOR_SPECIFIC_VIDEO_DATA_BLOCK:
            DBG_INFO("vendor specific data block\n");
            break;
        case MI_HDMI_RESERVED_VESA_DISPLAY_DEVICE:
            DBG_INFO("reserved vesa display device\n");
            break;
        case MI_HDMI_RESERVED_VESA_VIDEO_DATA_BLOCK:
            DBG_INFO("reserved vesa video data block\n");
            break;
        case MI_HDMI_RESERVED_HDMI_VIDEO_DATA_BLOCK:
            DBG_INFO("reserved hdmi video data block\n");
            break;
        case MI_HDMI_COLORIMETRY_DATA_BLOCK:
            if(MI_HDMI_XVYCC601 & pu8Data[1])
            {
                pSinkInfo->bSupportxvYcc601 = TRUE;
            }
            if(MI_HDMI_XVYCC709 & pu8Data[1])
            {
                pSinkInfo->bSupportxvYcc709 = TRUE;
            }
            DBG_INFO("Colorimetry:0x%02x\n", pu8Data[1]);
            break;
        case MI_HDMI_CEA_MISCELLANENOUS_AUDIO_FIELDS:
            DBG_INFO("CEA miscellanenous audio data fileds\n");
            break;
        case MI_HDMI_VENDOR_SPECIFIC_AUDIO_DATA_BLOCK:
            DBG_INFO("vendor specific audio data block\n");
            break;
        case MI_HDMI_RESERVED_HDMI_AUDIO_DATA_BLOCK:
            DBG_INFO("reserved hdmi audio data block\n");
            break;
        default:
            break;
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_PraseEdid(MI_U8 *pu8Data, MI_HDMI_SinkInfo_t *pstSinkInfo)
{
    MI_U8 u8Length = 0, u8Offset = 0, u8Len = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U16 u8DTD=0;

    MI_HDMI_BlockZero_t * pstBlockZero = (MI_HDMI_BlockZero_t *)pu8Data;
    MI_HDMI_SinkInfo_t stSinkInfo;
    memset(&stSinkInfo, 0, sizeof(MI_HDMI_SinkInfo_t));
    // Prase block0 EDID
    s32Ret = _MI_HDMI_CheckHeader(pu8Data); // check hdmi spec header
    if (MI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }
    stSinkInfo.u8Version = pstBlockZero->u8Version;
    stSinkInfo.u8Revision = pstBlockZero->u8Revision;
    stSinkInfo.u8EdidExternBlockNum = (pu8Data[0x7E] > 3) ? 3 : pu8Data[0x7E];
    ExecFunc(_MI_HDMI_ParseVendorInfo(pstBlockZero, &stSinkInfo), MI_SUCCESS);
    ExecFunc(_MI_HDMI_ParseEstablishTiming(pstBlockZero->au8EstablishTiming, &stSinkInfo), MI_SUCCESS);
    ExecFunc(_MI_HDMI_ParseStdTiming(pstBlockZero->au8StdTiming), MI_SUCCESS);

    // Prase block1 EDID
    if (MI_HDMI_EXT_BLOCK_VER_TAG != pu8Data[MI_HDMI_BLOCK_SIZE])
    {
        DBG_ERR("Extern block version err!\n");
        return MI_ERR_HDMI_EDID_DATA_ERR;
    }
    if (pu8Data[MI_HDMI_BLOCK_SIZE + 1] < MI_HDMI_EXT_BLOCK_REVERSION)
    {
        DBG_ERR("Extern block version err!\n");
        return MI_ERR_HDMI_EDID_DATA_ERR;
    }
    u8Length = pu8Data[MI_HDMI_BLOCK_SIZE + 2];
    DBG_INFO("data block length:0x%x\n", u8Length);
    if (0 == u8Length)
    {
        DBG_ERR("No detailed timing data!\n");
        return MI_ERR_HDMI_EDID_DATA_ERR;
    }

    ExecFunc(_MI_HDMI_ParseYcbcrSupport(&pu8Data[MI_HDMI_BLOCK_SIZE + 3], &stSinkInfo), MI_SUCCESS);
    if(u8Length <= 4)
    {
        DBG_ERR("No reserved provided! len:%d\n", u8Length);
        return MI_ERR_HDMI_EDID_DATA_ERR;
    }
    u8DTD = pu8Data[MI_HDMI_BLOCK_SIZE + 2] - 5;//relative Detailed Timing 3 u16offset
    u8Offset = 4 + MI_HDMI_BLOCK_SIZE; //block1 u16offset
    u8Length = u8Length + MI_HDMI_BLOCK_SIZE;

    while (u8Offset < u8Length)
    {
        u8Len = pu8Data[u8Offset] & MI_HDMI_DATA_BLOCK_LENGTH; //Data block len

        switch((pu8Data[u8Offset] & MI_HDMI_DATA_BLOCK_TAG_CODE) >> 5) //ext data type
        {
            case MI_HDMI_AUDIO_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseAudioDataBlock(&pu8Data[u8Offset + 1], u8Len, &stSinkInfo), MI_SUCCESS);
                break;
            case MI_HDMI_VIDEO_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseVideoDataBlock(&pu8Data[u8Offset + 1], u8Len, &stSinkInfo, u8DTD), MI_SUCCESS);
                break;
            case MI_HDMI_VENDOR_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseVendorSpecDataBlock(&pu8Data[u8Offset + 1], u8Len, &stSinkInfo), MI_SUCCESS);
                break;
            case MI_HDMI_SPEAKER_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseSpeakerDataBlock(&pu8Data[u8Offset + 1], u8Len, &stSinkInfo), MI_SUCCESS);
                break;
            case MI_HDMI_VESA_DTC_DATA_BLOCK:
                DBG_INFO("VESA_DTC parase\n");
                break;
            case MI_HDMI_USE_EXT_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseExtDataBlock(&pu8Data[u8Offset + 1], u8Len, &stSinkInfo), MI_SUCCESS);
                break;
             default:
                DBG_INFO("resvered block tag code define");
                break;
        }
        u8Offset += u8Len + 1;
    }

    memcpy(pstSinkInfo, &stSinkInfo, sizeof(MI_HDMI_SinkInfo_t));

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_Init(MI_HDMI_InitParam_t *pstInitParam)
{
    pthread_t _hdmi_tid;
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL_VOID(MI_HDMI_INIT);
    if (MI_SUCCESS == s32Ret)
    {
        if (_bThreadRunning)
        {
            DBG_INFO("Thread alrady running...\n");
            return;
        }
        if (NULL != pstInitParam->pfnHdmiEventCallback)
        {
            _gEventCallback = pstInitParam->pfnHdmiEventCallback;
        }
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        _bThreadRunning = TRUE;
        pthread_create(&_hdmi_tid, &attr, mi_hdmi_process, NULL);
        pthread_attr_destroy(&attr);
    }

    return s32Ret;
}

MI_S32 MI_HDMI_DeInit(void)
{
    MI_S32 s32Ret = MI_SUCCESS;
    _bThreadRunning = 0;
    //pthread_join(_HdmiTid, NULL);
    //_HdmiTid = 0;
    _gEventCallback = NULL;
    s32Ret = MI_SYSCALL_VOID(MI_HDMI_DEINIT);

    return s32Ret;
}

MI_S32 MI_HDMI_Open(MI_HDMI_DeviceId_e eHdmi)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_HDMI_OPEN, &eHdmi);

    return s32Ret;
}

MI_S32 MI_HDMI_Close(MI_HDMI_DeviceId_e eHdmi)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_HDMI_CLOSE, &eHdmi);

    return s32Ret;
}

MI_S32 MI_HDMI_SetAttr(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_Attr_t *pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_SetAttr_t stSetAttr;
    stSetAttr.eHdmi = eHdmi;
    memcpy(&stSetAttr.stAttr, pstAttr, sizeof(MI_HDMI_Attr_t));
    s32Ret = MI_SYSCALL(MI_HDMI_SET_ATTR, &stSetAttr);

    return s32Ret;
}

MI_S32 MI_HDMI_GetAttr(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_Attr_t *pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_GetAttr_t stGetAttr;
    if (NULL == pstAttr)
    {
        DBG_ERR("[%s][%d]MI_HDMI_GetAttr param invaild!\n", __FILE__, __LINE__);
        return MI_ERR_HDMI_INVALID_PARAM;
    }
    stGetAttr.eHdmi = eHdmi;
    s32Ret = MI_SYSCALL(MI_HDMI_GET_ATTR, &stGetAttr);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstAttr, &stGetAttr.stAttr, sizeof(MI_HDMI_Attr_t));
    }

    return s32Ret;
}

MI_S32 MI_HDMI_Start(MI_HDMI_DeviceId_e eHdmi)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_HDMI_START, &eHdmi);

    return s32Ret;
}

MI_S32 MI_HDMI_Stop(MI_HDMI_DeviceId_e eHdmi)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = MI_SYSCALL(MI_HDMI_STOP, &eHdmi);

    return s32Ret;
}

MI_S32 MI_HDMI_ForceGetEdid(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_Edid_t *pstEdidData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_FroceGetEdid_t stGetEdid;
    if (NULL == pstEdidData)
    {
        DBG_ERR("[%s][%d]MI_HDMI_ForceGetEdid param invaild!\n", __FILE__, __LINE__);
        return MI_ERR_HDMI_INVALID_PARAM;
    }
    stGetEdid.eHdmi = eHdmi;
    s32Ret = MI_SYSCALL(MI_HDMI_FROCE_GET_EDID, &stGetEdid);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstEdidData, &stGetEdid.stEdidData, sizeof(MI_HDMI_Edid_t));
    }
    else
    {
        return MI_ERR_HDMI_EDID_DATA_ERR;
    }

    return s32Ret;
}

MI_S32 MI_HDMI_GetSinkInfo(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_SinkInfo_t *pstSinkInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_SinkInfo_t stSinkInfo;
    MI_HDMI_FroceGetEdid_t stGetEdid;
    MI_HDMI_Edid_t stEdidData;
    if (NULL == pstSinkInfo)
    {
        DBG_ERR("[%s][%d]MI_HDMI_GetSinkInfo param invaild!\n", __FILE__, __LINE__);
        return MI_ERR_HDMI_INVALID_PARAM;
    }
    stGetEdid.eHdmi = eHdmi;
    s32Ret = MI_SYSCALL(MI_HDMI_FROCE_GET_EDID, &stGetEdid);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(&stEdidData, &stGetEdid.stEdidData, sizeof(MI_HDMI_Edid_t));
        if ((stEdidData.bEdidValid) && (stEdidData.u32Edidlength > 0))
        {
            s32Ret = _MI_HDMI_PraseEdid(stEdidData.au8Edid, &stSinkInfo);
            if (MI_SUCCESS == s32Ret)
            {
                DBG_INFO("_MI_HDMI_PraseEdid.OK..\n");
                memcpy(pstSinkInfo, &stSinkInfo, sizeof(MI_HDMI_SinkInfo_t));
            }
            else
            {
                DBG_ERR("[%s][%d]Prase Edid fail\n", __FILE__, __LINE__);
                return MI_ERR_HDMI_EDID_PRASE_ERR;
            }
        }
    }
    else
    {
        DBG_ERR("[%s][%d]Get Edid fail!\n", __FILE__, __LINE__);
        return MI_ERR_HDMI_EDID_PRASE_ERR;
    }

    return s32Ret;
}

MI_S32 MI_HDMI_SetAvMute(MI_HDMI_DeviceId_e eHdmi, MI_BOOL bAvMute)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_SetAvMute_t stAvMute;
    stAvMute.eHdmi = eHdmi;
    stAvMute.bAvMute = bAvMute;
    s32Ret = MI_SYSCALL(MI_HDMI_SET_AV_MUTE, &stAvMute);

    return s32Ret;
}

MI_S32 MI_HDMI_SetInfoFrame(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_InfoFrame_t *pstInfoFrame)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_SetInfoFrame_t stInfoFrame;
    stInfoFrame.eHdmi = eHdmi;

    memcpy(&stInfoFrame.stInfoFrame, pstInfoFrame, sizeof(MI_HDMI_InfoFrame_t));
    s32Ret = MI_SYSCALL(MI_HDMI_SET_INFO_FRAME, &stInfoFrame);

    return s32Ret;
}

MI_S32 MI_HDMI_GetInfoFrame(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_InfoFrameType_e eInfoFrameType,
    MI_HDMI_InfoFrame_t *pstInfoFrame)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_GetInfoFrame_t stInfoFrame;
    if (NULL == pstInfoFrame)
    {
        DBG_ERR("[%s][%d]MI_HDMI_GetInfoFrame param invaild!\n", __FILE__, __LINE__);
        return MI_ERR_HDMI_INVALID_PARAM;
    }
    stInfoFrame.eHdmi = eHdmi;
    stInfoFrame.eInfoFrameType = eInfoFrameType;
    s32Ret = MI_SYSCALL(MI_HDMI_GET_INFO_FRAME, &stInfoFrame);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstInfoFrame, &stInfoFrame.stInfoFrame, sizeof(MI_HDMI_InfoFrame_t));
    }

    return s32Ret;
}
