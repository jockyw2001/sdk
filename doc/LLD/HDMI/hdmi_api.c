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

#include "mi_hdmi.h"
#include "hdmi_ioctl.h"
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
MI_MODULE_DEFINE(hdmi)

#define ExecFunc(_func_, _ret_) \
    if (_func_ != _ret_)\
    {\
        DBG_ERR("[%d]exec function failed\n", __LINE__);\
        return 1;\
    }\
    else\
    {\
        DBG_INFO("(%d)exec function pass\n", __LINE__);\
    }

#define HDMI_API_ISVALID_POINT(X)  \
    {   \
        if( X == NULL)  \
        {   \
            DBG_ERR("MI_ERR_INVALID_PARAMETER!\n");  \
            return E_MI_ERR_ILLEGAL_PARAM;   \
        }   \
    }   \

#define  XVYCC601            0x01
#define  XVYCC709            0x02

#define STANDARD_TIMING_SIZE 12
//                  Aspect ratio
#define ASPECT_4       1  // 4:3
#define ASPECT_16      2  // 16:9
#define ASPECT_4or16   3  // 16:9
#define ASPECT_16_10   4  // 16:10
#define ASPECT_5_4     5  // 5:4

#define EXT_BLOCK_VER_TAG 0x2
#define EXT_BLOCK_REVERSION 0x3

//Data block flag
#define  AUDIO_DATA_BLOCK    0x01
#define  VIDEO_DATA_BLOCK    0x02
#define  VENDOR_DATA_BLOCK   0x03
#define  SPEAKER_DATA_BLOCK  0x04
#define  VESA_DTC_DATA_BLOCK 0x05
#define  USE_EXT_DATA_BLOCK  0x07
#define  DATA_BLOCK_LENGTH   0x1F
#define  DATA_BLOCK_TAG_CODE 0xE0
#define  AUDIO_FORMAT_CODE   0x78
#define  AUDIO_MAX_CHANNEL   0x07

#define  VIDEO_CAPABILITY_DATA_BLOCK      0x00
#define  VENDOR_SPECIFIC_VIDEO_DATA_BLOCK 0x01
#define  RESERVED_VESA_DISPLAY_DEVICE     0x02
#define  RESERVED_VESA_VIDEO_DATA_BLOCK   0x03
#define  RESERVED_HDMI_VIDEO_DATA_BLOCK   0x04
#define  COLORIMETRY_DATA_BLOCK           0x05
#define  CEA_MISCELLANENOUS_AUDIO_FIELDS  0x10
#define  VENDOR_SPECIFIC_AUDIO_DATA_BLOCK 0x11
#define  RESERVED_HDMI_AUDIO_DATA_BLOCK   0x12

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct MI_HDMI_BlockZero_s
{
    MI_U8 ua8Header[8];
    MI_U8 au8MfgId[2]; //Manufacture
    MI_U8 au8ProdCode[2];
    MI_U8 au8SerialNumber[4];
    MI_U8 u8MfgWeek;
    MI_U8 u8MfgYear;
    MI_U8 u8Version;
    MI_U8 u8Revision;
    MI_U8 u8Input;
    MI_U8 u8WidthCm;
    MI_U8 u8HeightCm;
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
    MI_U8 au8EstTiming[3];
    MI_U8 au8StdTiming[16];
    MI_U8 au8DetailedTiming[4];
} MI_HDMI_BlockZero_t;

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

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------
//static pthread_t _HdmiTid = 0; //Need Colin Support
static MI_BOOL bThreadRunning = 0;
static MI_HDMI_EventCallBack gEventCallback = NULL;
static MI_HDMI_SinkInfo_t _stSinkInfo = {0};

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
void * mi_hdmi_process(void * args)
{
    MI_S32 s32RetVal = E_MI_ERR_FAILED;
    MI_BOOL bLastStatus, bCurStatus;
    MI_HDMI_Attr_t stAttr;
    MI_HDMI_EventType_e eEventType = E_MI_HDMI_EVENT_MAX;

    bLastStatus = bCurStatus = FALSE;
    while (bThreadRunning)
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
                if (gEventCallback)
                {
                    gEventCallback(E_MI_HDMI_ID_0, eEventType, NULL, NULL);
                }
            }
        }
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
            return E_MI_ERR_FAILED;
        }
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_ParseVendorInfo(MI_HDMI_BlockZero_t *pstData)
{
    MI_U16 u16Index, u16Data;
    MI_HDMI_SinkInfo_t *pSinkInfo = &_stSinkInfo;

    HDMI_API_ISVALID_POINT(pstData);

    u16Data = (pstData->au8MfgId[0] << 8) | (pstData->au8MfgId[1]);

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
            return E_MI_ERR_FAILED;
        }
    }

    pSinkInfo->u32IdProductCode = (pstData->au8ProdCode[1] << 8) | pstData->au8ProdCode[0];
    pSinkInfo->u32IdSerialNumber = (pstData->au8SerialNumber[3] << 24) | (pstData->au8SerialNumber[2] << 16) |
        (pstData->au8SerialNumber[1] << 8) | (pstData->au8SerialNumber[0]);
    pSinkInfo->u32WeekOfManufacture = pstData->u8MfgWeek;
    pSinkInfo->u32YearOfManufacture = pstData->u8MfgYear + 1990;

    DBG_INFO("Edid info mfg name[%s]\n", pSinkInfo->au8IdManufactureName);
    DBG_INFO("Edid info code:%d\n", pSinkInfo->u32IdProductCode);
    DBG_INFO("Edid info serial:%d\n", pSinkInfo->u32IdSerialNumber);
    DBG_INFO("Edid info year:%d,week:%d\n", pSinkInfo->u32WeekOfManufacture, pSinkInfo->u32YearOfManufacture);

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_ParseEstablishTiming(MI_U8 *pu8Data)
{
    HDMI_API_ISVALID_POINT(pu8Data);

    if(pu8Data[0] & 0x20)
    {
        DBG_INFO("640 x 480 @ 60Hz\n");
        _stSinkInfo.abVideoFmtSupported[E_MI_HDMI_TIMING_480_60P] = TRUE;
    }
    if(pu8Data[0] & 0x01)
    {
        //_stSinkInfo.abVideoFmtSupported[800*600]
        DBG_INFO("800 x 400 @ 60Hz\n");
    }
    if(pu8Data[1] & 0x08)
    {
        _stSinkInfo.abVideoFmtSupported[E_MI_HDMI_TIMING_1024x768_60P] = TRUE;
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

    for(u8Index = 0; u8Index < STANDARD_TIMING_SIZE; u8Index += 2)
    {
        if((0x01 == pu8Data[u8Index]) && (0x01 == pu8Data[u8Index + 1]))
        {
            DBG_INFO("Mode %d wasn't defined! \n", (int)pu8Data[u8Index]);
        }
        else
        {
            u32Hor = (pu8Data[u8Index]+31) * 8;
            DBG_INFO(" Hor Act pixels %d \n", u32Hor);
            u8TmpVal = pu8Data[u8Index + 1] & 0xC0;
            if (0x00 == u8TmpVal)
            {
                DBG_INFO("Aspect ratio:16:10\n");
                u32AspectRatio = ASPECT_16_10;
                u32Ver = u32Hor * 10/16;
            }
            else if (0x40 == u8TmpVal)
            {
                DBG_INFO("Aspect ratio:4:3\n");
                u32AspectRatio = ASPECT_4;
                u32Ver = u32Hor * 3/4;
            }
            else if (0x80 == u8TmpVal)
            {
                DBG_INFO("Aspect ratio:5:4\n");
                u32AspectRatio = ASPECT_5_4;
                u32Ver = u32Hor * 4/5;
            }
            else //0xc0
            {
                DBG_INFO("Aspect ratio:16:9\n");
                u32AspectRatio = ASPECT_16;
                u32Ver = u32Hor * 9/16;
            }
            u32Freq = ((pu8Data[u8Index + 1]) & 0x3F) + 60;
            DBG_INFO(" Refresh rate %d Hz \n", u32Freq);
        }
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_ParseYcbcrSupport(MI_U8 *pu8Data)
{
    MI_HDMI_SinkInfo_t *pSinkInfo = &_stSinkInfo;
    HDMI_API_ISVALID_POINT(pu8Data);
    pSinkInfo->bSupportYCbCr222 = (pu8Data[0] & 0X10) ? TRUE : FALSE;
    pSinkInfo->bSupportYCbCr444 = (pu8Data[0] & 0X20) ? TRUE : FALSE;

    return MI_SUCCESS;
}

/* Audio Data Block */
static MI_S32 _MI_HDMI_ParseAudioDataBlock(MI_U8 *pu8Data, MI_U8 u8Len)
{
    //no impl
    return MI_SUCCESS;
}

/* Video Data Block */
static MI_S32 _MI_HDMI_ParseVideoDataBlock(MI_U8 *pu8Data, MI_U8 u8Len)
{
    //no impl
    return MI_SUCCESS;
}

/* Vendor Specific Data Block */
static MI_S32 _MI_HDMI_ParseVendorSpecDataBlock(MI_U8 *pu8Data, MI_U8 u8Len)
{
    MI_HDMI_SinkInfo_t *pSinkInfo = &_stSinkInfo;

    DBG_INFO("IEERegId:0x%02x, 0x%02x, 0x%02x\n", pu8Data[0], pu8Data[1], pu8Data[2]);
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
        return E_MI_ERR_FAILED;
    }

    if(u8Len < 4)
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
static MI_S32 _MI_HDMI_ParseSpeakerDataBlock(MI_U8 *pu8Data, MI_U8 u8Len)
{
    MI_HDMI_SinkInfo_t *pSinkInfo = &_stSinkInfo;

    DBG_INFO("Speaker Data[0]:0x%x Data[0]:0x%x\n", pu8Data[0], pu8Data[1]);
    if ((0 != pu8Data[0]) || (0 != pu8Data[1]))
    {
        pSinkInfo->u8Speaker = TRUE;
    }

    return MI_SUCCESS;
}

/* Extern Data Block */
static MI_S32 _MI_HDMI_ParseExtDataBlock(MI_U8 *pu8Data, MI_U8 u8Len)
{
    MI_HDMI_SinkInfo_t *pSinkInfo = &_stSinkInfo;

    switch(pu8Data[0])
    {
        case VIDEO_CAPABILITY_DATA_BLOCK:
            DBG_INFO("Video Capability Data Block\n");
            break;
        case VENDOR_SPECIFIC_VIDEO_DATA_BLOCK:
            DBG_INFO("vendor specific data block\n");
            break;
        case RESERVED_VESA_DISPLAY_DEVICE:
            DBG_INFO("reserved vesa display device\n");
            break;
        case RESERVED_VESA_VIDEO_DATA_BLOCK:
            DBG_INFO("reserved vesa video data block\n");
            break;
        case RESERVED_HDMI_VIDEO_DATA_BLOCK:
            DBG_INFO("reserved hdmi video data block\n");
            break;
        case COLORIMETRY_DATA_BLOCK:
            if(XVYCC601 & pu8Data[1])
            {
                pSinkInfo->bSupportxvYcc601 = TRUE;
            }
            if(XVYCC709 & pu8Data[1])
            {
                pSinkInfo->bSupportxvYcc709 = TRUE;
            }
            DBG_INFO("Colorimetry:0x%02x\n", pu8Data[1]);
            break;
        case CEA_MISCELLANENOUS_AUDIO_FIELDS:
            DBG_INFO("CEA miscellanenous audio data fileds\n");
            break;
        case VENDOR_SPECIFIC_AUDIO_DATA_BLOCK:
            DBG_INFO("vendor specific audio data block\n");
            break;
        case RESERVED_HDMI_AUDIO_DATA_BLOCK:
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
    MI_S32 s32Ret = -1;

    memset(&_stSinkInfo, 0, sizeof(MI_HDMI_SinkInfo_t));
    MI_HDMI_BlockZero_t * pstBlockZero = (MI_HDMI_BlockZero_t *)pu8Data;

    // Prase block0 EDID
    ExecFunc(_MI_HDMI_CheckHeader(pu8Data), MI_SUCCESS); // check hdmi spec header
    _stSinkInfo.u8Version = pstBlockZero->u8Version;
    _stSinkInfo.u8Revision = pstBlockZero->u8Revision;
    _stSinkInfo.u8EdidExternBlockNum = (pu8Data[0x7E] > 3) ? 3 : pu8Data[0x7E];
    ExecFunc(_MI_HDMI_ParseVendorInfo(pstBlockZero), MI_SUCCESS);
    ExecFunc(_MI_HDMI_ParseEstablishTiming(pstBlockZero->au8EstTiming), MI_SUCCESS);
    ExecFunc(_MI_HDMI_ParseStdTiming(pstBlockZero->au8StdTiming), MI_SUCCESS);

    // Prase block1 EDID
    if (EXT_BLOCK_VER_TAG != pu8Data[BLOCK_SIZE])
    {
        DBG_ERR("Extern block version err!\n");
        return E_MI_ERR_FAILED;
    }
    if (pu8Data[BLOCK_SIZE + 1] < EXT_BLOCK_REVERSION)
    {
        DBG_ERR("Extern block version err!\n");
        return E_MI_ERR_FAILED;
    }
    u8Length = pu8Data[BLOCK_SIZE + 2];
    DBG_INFO("data block length:0x%x\n", u8Length);
    if (0 == u8Length)
    {
        DBG_ERR("No detailed timing data!\n");
        return E_MI_ERR_FAILED;
    }

    ExecFunc(_MI_HDMI_ParseYcbcrSupport(&pu8Data[BLOCK_SIZE + 3]), MI_SUCCESS);
    if(u8Length <= 4)
    {
        DBG_ERR("No reserved provided! len:%d\n", u8Length);
        return E_MI_ERR_FAILED;
    }
    u8Offset = 4 + BLOCK_SIZE; //block1
    u8Length += BLOCK_SIZE;

    while(u8Offset < u8Length)
    {
        u8Len = pu8Data[u8Offset] & DATA_BLOCK_LENGTH; //Data block len

        switch((pu8Data[u8Offset] & DATA_BLOCK_TAG_CODE) >> 5) //ext data type
        {
            case AUDIO_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseAudioDataBlock(&pu8Data[u8Offset + 1], u8Len), MI_SUCCESS);
                break;
            case VIDEO_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseVideoDataBlock(&pu8Data[u8Offset + 1], u8Len), MI_SUCCESS);
                break;
            case VENDOR_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseVendorSpecDataBlock(&pu8Data[u8Offset + 1], u8Len), MI_SUCCESS);
                break;
            case SPEAKER_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseSpeakerDataBlock(&pu8Data[u8Offset + 1], u8Len), MI_SUCCESS);
                break;
            case VESA_DTC_DATA_BLOCK:
                DBG_INFO("VESA_DTC parase\n");
                break;
            case USE_EXT_DATA_BLOCK:
                ExecFunc(_MI_HDMI_ParseExtDataBlock(&pu8Data[u8Offset + 1], u8Len), MI_SUCCESS);
                break;
             default:
                DBG_INFO("resvered block tag code define");
                break;
        }
        u8Offset += u8Len + 1;
    }

    memcpy(pstSinkInfo, &_stSinkInfo, sizeof(MI_HDMI_SinkInfo_t));

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_Init(MI_HDMI_InitParam_t *pstInitParam)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL_VOID(MI_HDMI_INIT);

    if (MI_SUCCESS == s32Ret)
    {
        //if (0 != _HdmiTid) //Need Colin Support
        {
        //    DBG_INFO("Thread alrady running tid (%d)\n", _HdmiTid);
        //    return;
        }
        if (NULL != pstInitParam->pfnHdmiEventCallback)
        {
            gEventCallback = pstInitParam->pfnHdmiEventCallback;
        }
        //pthread_attr_t attr;
        //pthread_attr_init(&attr);
        //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        bThreadRunning = TRUE;
        //pthread_create(&_hdmi_tid, &attr, mi_hdmi_process, NULL);
        //pthread_attr_destroy(&attr);
    }

    return s32Ret;
}

MI_S32 MI_HDMI_DeInit(void)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    bThreadRunning = 0;
    //pthread_cancel(_HdmiTid);
    //pthread_join(_HdmiTid, NULL);
    //_HdmiTid = 0;
    gEventCallback = NULL;
    s32Ret = MI_SYSCALL_VOID(MI_HDMI_DEINIT);

    return s32Ret;
}

MI_S32 MI_HDMI_Open(MI_HDMI_DeviceId_e eHdmi)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_IoctlEdid_t stEdid;
    stEdid.eHdmi = eHdmi;
    s32Ret = MI_SYSCALL(MI_HDMI_OPEN, &stEdid);

    return s32Ret;
}

MI_S32 MI_HDMI_Close(MI_HDMI_DeviceId_e eHdmi)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_IoctlEdid_t stEdid;
    stEdid.eHdmi = eHdmi;
    s32Ret = MI_SYSCALL(MI_HDMI_CLOSE, &stEdid);

    return s32Ret;
}

MI_S32 MI_HDMI_SetAttr(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_Attr_t *pstAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_SetAttr_t stSetAttr;
    stSetAttr.eHdmi = eHdmi;
    memcpy(&stSetAttr.stAttr, pstAttr, sizeof(MI_HDMI_Attr_t));
    s32Ret = MI_SYSCALL(MI_HDMI_SET_ATTR, &stSetAttr);

    return s32Ret;
}

MI_S32 MI_HDMI_GetAttr(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_Attr_t *pstAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_GetAttr_t stGetAttr;
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
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_IoctlEdid_t stEdid;
    stEdid.eHdmi = eHdmi;
    s32Ret = MI_SYSCALL(MI_HDMI_START, &stEdid);

    return s32Ret;
}

MI_S32 MI_HDMI_Stop(MI_HDMI_DeviceId_e eHdmi)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_IoctlEdid_t stEdid;
    stEdid.eHdmi = eHdmi;
    s32Ret = MI_SYSCALL(MI_HDMI_STOP, &stEdid);

    return s32Ret;
}

MI_S32 MI_HDMI_ForceGetEdid(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_Edid_t *pstEdidData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_FroceGetEdid_t stGetEdid;
    stGetEdid.eHdmi = eHdmi;
    s32Ret = MI_SYSCALL(MI_HDMI_FROCE_GET_EDID, &stGetEdid);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstEdidData, &stGetEdid.stEdidData, sizeof(MI_HDMI_Edid_t));
    }

    return s32Ret;
}

MI_S32 MI_HDMI_GetSinkInfo(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_SinkInfo_t *pstSinkInfo)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_FroceGetEdid_t stGetEdid;
    MI_HDMI_Edid_t stEdidData;
    stGetEdid.eHdmi = eHdmi;
    s32Ret = MI_SYSCALL(MI_HDMI_FROCE_GET_EDID, &stGetEdid);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(&stEdidData, &stGetEdid.stEdidData, sizeof(MI_HDMI_Edid_t));
        if ((stEdidData.bEdidValid) && (stEdidData.u32Edidlength > 0))
        {
            s32Ret = _MI_HDMI_PraseEdid(stEdidData.au8Edid + BLOCK_SIZE, pstSinkInfo);
        }
    }

    return s32Ret;
}

MI_S32 MI_HDMI_SetAvMute(MI_HDMI_DeviceId_e eHdmi, MI_BOOL bAvMute)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_SetAvMute_t stAvMute;
    stAvMute.eHdmi = eHdmi;
    stAvMute.bAvMute = bAvMute;
    s32Ret = MI_SYSCALL(MI_HDMI_SET_AV_MUTE, &stAvMute);

    return s32Ret;
}

MI_S32 MI_HDMI_SetInfoFrame(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_InfoFrame_t *pstInfoFrame)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_SetInfoFrame_t stInfoFrame;
    stInfoFrame.eHdmi = eHdmi;

    memcpy(&stInfoFrame.stInfoFrame, pstInfoFrame, sizeof(MI_HDMI_InfoFrame_t));
    s32Ret = MI_SYSCALL(MI_HDMI_SET_INFO_FRAME, &stInfoFrame);

    return s32Ret;
}

MI_S32 MI_HDMI_GetInfoFrame(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_InfoFrameType_e eInfoFrameType,
    MI_HDMI_InfoFrame_t *pstInfoFrame)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_HDMI_GetInfoFrame_t stInfoFrame;
    stInfoFrame.eHdmi = eHdmi;
    stInfoFrame.eInfoFrameType = eInfoFrameType;
    s32Ret = MI_SYSCALL(MI_HDMI_GET_INFO_FRAME, &stInfoFrame);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstInfoFrame, &stInfoFrame.stInfoFrame, sizeof(MI_HDMI_InfoFrame_t));
    }

    return s32Ret;
}
