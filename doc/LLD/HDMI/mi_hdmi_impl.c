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
#include <linux/string.h>
#include <linux/time.h>
//#include <linux/delay.h>

#include "mi_common.h"
#include "mi_print.h"
#include "mi_hdmi_impl.h"
#include "hdmi_dummy.h" //dummy driver header

//-------------------------------------------------------------------------------------------------
// Local Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MI_HDMI_ERR_INVALID_DEVID = 0x10008001,
    E_MI_HDMI_ERR_DRV_RET_FAIL,
    E_MI_HDMI_ERR_NOT_INIT,
    E_MI_HDMI_ERR_UNSUPPORT_TIMING,
    E_MI_HDMI_ERR_UNSUPPORT_COLORTYPE,
    E_MI_HDMI_ERR_UNSUPPORT_COLORDEPTH,
    E_MI_HDMI_ERR_UNSUPPORT_OUTPUTMODE,
    E_MI_HDMI_ERR_UNSUPPORT_ACODETYPE,
    E_MI_HDMI_ERR_UNSUPPORT_AFREQ,
    E_MI_HDMI_ERR_EDID_HEADER_ERR,
    E_MI_HDMI_ERR_MAX
} MI_HDMI_ErrCode_e;

//-------------------------------------------------------------------------------------------------
// Local Defines
//-------------------------------------------------------------------------------------------------
#define TRUE (1)
#define FALSE (0)

#define HDMI_EDID_CHECK_TIMEOUT 100
#define HDMI_EDID_CHECK_DELAY   10

#define HDMI_IMPL_ISVALID_POINT(X)  \
    {   \
        if( X == NULL)  \
        {   \
            DBG_ERR("MI_ERR_INVALID_PARAMETER!\n");  \
            return E_MI_ERR_ILLEGAL_PARAM;   \
        }   \
    }   \

/* HDMI Module ErrorCode */
#define MI_ERR_HDMI_INVALID_DEVID MI_DEF_ERR(E_MI_MODULE_ID_HDMI, E_MI_ERR_LEVEL_ERROR, E_MI_HDMI_ERR_INVALID_DEVID)
#define MI_ERR_HDMI_DRV_RET_FAIL MI_DEF_ERR(E_MI_MODULE_ID_HDMI, E_MI_ERR_LEVEL_ERROR, E_MI_HDMI_ERR_DRV_RET_FAIL)
#define MI_ERR_HDMI_NOT_INIT MI_DEF_ERR(E_MI_MODULE_ID_HDMI, E_MI_ERR_LEVEL_ERROR, E_MI_HDMI_ERR_NOT_INIT)
#define MI_ERR_HDMI_UNSUPPORT_TIMING MI_DEF_ERR(E_MI_MODULE_ID_HDMI, E_MI_ERR_LEVEL_ERROR, E_MI_HDMI_ERR_UNSUPPORT_TIMING)
#define MI_ERR_HDMI_UNSUPPORT_COLORTYPE MI_DEF_ERR(E_MI_MODULE_ID_HDMI, E_MI_ERR_LEVEL_ERROR, E_MI_HDMI_ERR_UNSUPPORT_COLORTYPE)
#define MI_ERR_HDMI_UNSUPPORT_ACODETYPE MI_DEF_ERR(E_MI_MODULE_ID_HDMI, E_MI_ERR_LEVEL_ERROR, E_MI_HDMI_ERR_UNSUPPORT_ACODETYPE)
#define MI_ERR_HDMI_UNSUPPORT_AFREQ MI_DEF_ERR(E_MI_MODULE_ID_HDMI, E_MI_ERR_LEVEL_ERROR, E_MI_HDMI_ERR_UNSUPPORT_AFREQ)
#define MI_ERR_HDMI_EDID_HEADER_ERR MI_DEF_ERR(E_MI_MODULE_ID_HDMI, E_MI_ERR_LEVEL_ERROR, E_MI_HDMI_ERR_EDID_HEADER_ERR)

/* SPD(Source Product Description) infoframe data byte */
typedef enum
{
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_UNKNOWN,              ///<Unknown
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_DIGITAL_STB,          ///<Digital STB
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_DVD_PLAYER,           ///<DVD player
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_D_VHS,                ///<D-VHS
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_HDD_VIDEO_RECORDER,   ///<HDD VideoRecorder
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_DVC,                  ///<Digital Video Camera (DVC)
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_DSC,                  ///<Digital Still Camera (DSC)
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_VIDEO_CD,             ///<Video CD
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_GAME,                 ///<Game
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_PC_GENERAL,           ///<PC general
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_BULE_RAY_DISC,        ///<Blu-Ray Disc(BD)
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_SUPER_AUDIO_CD,       ///<Super Audio CD
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_HD_DVD,               ///<HD DVD
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_PMP,                  ///<Portable Multimedia Player (PMP)
    E_DEVICE_HDMITX_SPD_SOURCE_INFO_MAX,
} E_MI_HDMI_SpdSourceInfo_e;

//-------------------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------------------

/*-------------------------------------------------------------------------------------------------
 * Local Structures
 ------------------------------------------------------------------------------------------------*/
/* User: Hdmi Module*/
typedef struct MI_HDMI_InerAttr_s
{
    HDMITX_OUTPUT_MODE eHdmiTxMode; //Utopia Enum
    HDMITX_VIDEO_COLOR_FORMAT eHdmiTxInColor;
    HDMITX_VIDEO_COLOR_FORMAT eHdmiTxOutColor;
    HDMITX_VIDEO_COLORDEPTH_VAL eHdmiTxColorDepth;
    HDMITX_AUDIO_FREQUENCY eAudioFreq;
    HDMITX_AUDIO_CHANNEL_COUNT eAudioChCnt;
    HDMITX_AUDIO_CODING_TYPE eAudioCodeType;
    HDMITX_AUDIO_SOURCE_FORMAT eAudioSrcFmt;
    HDMITX_VIDEO_TIMING eHdmiVideoTiming;
} MI_HDMI_InerAttr_t;

/* Hdmi Module Resource Manage Structure */
typedef struct _MI_HDMI_ResMgr_s
{
    MI_BOOL bInitFlag;
    MI_BOOL bHdmiTxRuning;
    MI_BOOL bAvMute;
    MI_BOOL bPowerOn;
    MI_U32  u32swVerion;
    MI_HDMI_Attr_t stAttr;
    MI_HDMI_InerAttr_t stInerAttr;
    MI_HDMI_AviInfoFrameVer_t stAviInfoFrame;
    MI_HDMI_AudInfoFrameVer_t stAudInfoFrame;
    MI_HDMI_SpdInfoFrame_t stSpdInfoFrame;
} _MI_HDMI_ResMgr_t;

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------
static _MI_HDMI_ResMgr_t _stHdmiMgr = {0};
static MI_U8 _u8HdmiGpioPin = 16; //hardware pad

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
static MI_U32 _GetTime(void) {
    struct timeval stTimeVal;
    MI_U32 u32Ms;
    do_gettimeofday(&stTimeVal);
    u32Ms = (stTimeVal.tv_sec * 1000) + (stTimeVal.tv_usec / 1000);
    if(0 == u32Ms)
    {
        u32Ms = 1;
    }

    return u32Ms;
}

static MI_U32 _DiffFromNow(MI_U32 u32Time) {
    MI_U32 u32Now;
    struct timeval stTimeVal;
    do_gettimeofday(&stTimeVal);
    u32Now = (stTimeVal.tv_sec * 1000) + (stTimeVal.tv_usec / 1000);
    if(u32Now >= u32Time)
    {
        return u32Now - u32Time;
    }

    return (0xFFFFFFFF - u32Time) + u32Now;
}

static MI_BOOL _MI_HDMI_IsReceiverSupportYPbPr(void)
{
    MI_U8 au8BlockData[BLOCK_SIZE] = {0};

    if(MApi_HDMITx_GetEDIDData(au8BlockData, BLOCK1_INDEX) == FALSE)
    {
        DBG_ERR("HDMITX %s[%d] Get EDID fail \n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if((au8BlockData[0x03] & 0x30) == 0x00)
    {
        DBG_ERR("HDMITX %s[%d] Rx Not Support YCbCr \n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    else
    {
        DBG_ERR("HDMITX %s[%d] Rx Support YUV444 or YUV422 \n", __FUNCTION__, __LINE__);
        return TRUE;
    }

    return FALSE;
}

static void _MI_HDMI_ChkColorFormatForSpecTiming(MI_HDMI_TimingType_e eTimingType)
{
    MI_U8 au8BlockData[128] = {0};

    HDMITX_VIDEO_COLOR_FORMAT eHdmiTxInColor = HDMITX_VIDEO_COLOR_RGB444;
    HDMITX_VIDEO_COLOR_FORMAT eHdmiTxOutColor = HDMITX_VIDEO_COLOR_RGB444;

    MI_U32 u32StartTime = _GetTime();

    while(TRUE != MApi_HDMITx_GetEDIDData(au8BlockData, BLOCK1_INDEX))
    {
        if(_DiffFromNow(u32StartTime) > HDMI_EDID_CHECK_TIMEOUT)
        {
            if(MApi_HDMITx_EdidChecking())
            {
                MApi_HDMITx_GetEDIDData(au8BlockData, BLOCK1_INDEX);
                break;
            }
            else
            {
                DBG_ERR("HDMITX %s[%d] EDID checking failed!!\n", __FUNCTION__, __LINE__);
                return;
            }
        }
        //mdelay(HDMI_EDID_CHECK_DELAY*1000); // 10ms
    }

    if(eTimingType == E_MI_HDMI_TIMING_4K2K_60P)
    {
        HDMITX_EDID_COLOR_FORMAT ColorFmt = HDMITX_EDID_Color_YCbCr_444;
        MApi_HDMITx_GetColorFormatFromEDID(HDMITX_RES_3840x2160p_60Hz, &ColorFmt);
        if(ColorFmt != HDMITX_EDID_Color_YCbCr_420)
        {
            // support 4k2k@60Hz YUV444
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV444;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV444;
        }
        else
        {
            // only support 4k2k@60Hz YUV420
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV420;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV420;
        }
    }
    else if(eTimingType == E_MI_HDMI_TIMING_4K2K_50P)
    {
        HDMITX_EDID_COLOR_FORMAT ColorFmt = HDMITX_EDID_Color_YCbCr_444;
        MApi_HDMITx_GetColorFormatFromEDID(HDMITX_RES_3840x2160p_50Hz, &ColorFmt);
        if(ColorFmt != HDMITX_EDID_Color_YCbCr_420)
        {
            // support 4k2k@50Hz YUV444
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV444;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV444;
        }
        else
        {
            // only support 4k2k@50Hz YUV420
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV420;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV420;
        }
    }
    else if(eTimingType == E_MI_HDMI_TIMING_4K2K_30P)
    {
        HDMITX_EDID_COLOR_FORMAT ColorFmt = HDMITX_EDID_Color_YCbCr_444;
        MApi_HDMITx_GetColorFormatFromEDID(HDMITX_RES_3840x2160p_30Hz, &ColorFmt);
        if(ColorFmt != HDMITX_EDID_Color_YCbCr_420)
        {
            // support 4k2k@30Hz YUV444
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV444;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV444;
        }
        else
        {
            // only support 4k2k@30Hz YUV420
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV420;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV420;
        }
    }
    else if(eTimingType == E_MI_HDMI_TIMING_4096x2160_60P)
    {
        HDMITX_EDID_COLOR_FORMAT ColorFmt = HDMITX_EDID_Color_YCbCr_444;
        MApi_HDMITx_GetColorFormatFromEDID(HDMITX_RES_4096x2160p_60Hz, &ColorFmt);
        if(ColorFmt != HDMITX_EDID_Color_YCbCr_420)
        {
            // support 4096x2160@60Hz YUV444
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV444;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV444;
        }
        else
        {
            // only support 4096x2160@60Hz YUV420
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV420;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV420;
        }
    }
    else if(eTimingType == E_MI_HDMI_TIMING_4096x2160_50P)
    {
        HDMITX_EDID_COLOR_FORMAT ColorFmt = HDMITX_EDID_Color_YCbCr_444;
        MApi_HDMITx_GetColorFormatFromEDID(HDMITX_RES_4096x2160p_50Hz, &ColorFmt);
        if(ColorFmt != HDMITX_EDID_Color_YCbCr_420)
        {
            // support 4096x2160@50Hz YUV444
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV444;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV444;
        }
        else
        {
            // only support 4096x2160@50Hz YUV420
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV420;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV420;
        }
    }
    else if(eTimingType == E_MI_HDMI_TIMING_4096x2160_30P)
    {
        HDMITX_EDID_COLOR_FORMAT ColorFmt = HDMITX_EDID_Color_YCbCr_444;
        MApi_HDMITx_GetColorFormatFromEDID(HDMITX_RES_4096x2160p_30Hz, &ColorFmt);
        if(ColorFmt != HDMITX_EDID_Color_YCbCr_420)
        {
            // support 4096x2160@30Hz YUV444
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV444;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV444;
        }
        else
        {
            // only support 4096x2160@30Hz YUV420
            eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV420;
            eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV420;
        }
    }
    else if(_MI_HDMI_IsReceiverSupportYPbPr() == TRUE)
    {
        eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV444;
        eHdmiTxOutColor = HDMITX_VIDEO_COLOR_YUV444;
    }
    else
    {
        eHdmiTxInColor = HDMITX_VIDEO_COLOR_YUV444;
        eHdmiTxOutColor = HDMITX_VIDEO_COLOR_RGB444;
    }
    _stHdmiMgr.stInerAttr.eHdmiTxInColor = eHdmiTxInColor;
    _stHdmiMgr.stInerAttr.eHdmiTxOutColor = eHdmiTxOutColor;

    return;
}

static MI_S32 _MI_HDMI_SetTiming(MI_HDMI_TimingType_e eTimingType)
{
    HDMITX_VIDEO_TIMING eHdmiVideoTiming = HDMITX_RES_1920x1080p_60Hz;
    HDMITX_OUTPUT_MODE eHDMITxMode = _stHdmiMgr.stInerAttr.eHdmiTxMode;
    HDMITX_VIDEO_COLORDEPTH_VAL eHDMITxColorDepth = _stHdmiMgr.stInerAttr.eHdmiTxColorDepth;

    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    //HDMITX_ANALOG_TUNING stHDMITxTun;

    _MI_HDMI_ChkColorFormatForSpecTiming(eTimingType); //IMPL COLOR FORMAT

    DBG_INFO("HDMITX %s[%d] MI_HDMI_TimingType_e(enTiming = %d) \n", __FUNCTION__, __LINE__, eTimingType);
    switch(eTimingType)
    {
        case E_MI_HDMI_TIMING_480_60I:
            eHdmiVideoTiming = HDMITX_RES_720x480i;
            break;
        case E_MI_HDMI_TIMING_480_60P:
            eHdmiVideoTiming = HDMITX_RES_720x480p;
            break;
        case E_MI_HDMI_TIMING_576_50I:
            eHdmiVideoTiming = HDMITX_RES_720x576i;
            break;
        case E_MI_HDMI_TIMING_576_50P:
            eHdmiVideoTiming = HDMITX_RES_720x576p;
            break;
        case E_MI_HDMI_TIMING_720_50P:
            eHdmiVideoTiming = HDMITX_RES_1280x720p_50Hz;
            break;
        case E_MI_HDMI_TIMING_720_60P:
            eHdmiVideoTiming = HDMITX_RES_1280x720p_60Hz;
            break;
        case E_MI_HDMI_TIMING_1080_50I:
            eHdmiVideoTiming = HDMITX_RES_1920x1080i_50Hz;
            break;
        case E_MI_HDMI_TIMING_1080_50P:
            eHdmiVideoTiming = HDMITX_RES_1920x1080p_50Hz;
            break;
        case E_MI_HDMI_TIMING_1080_60I:
            eHdmiVideoTiming = HDMITX_RES_1920x1080i_60Hz;
            break;
        case E_MI_HDMI_TIMING_1080_60P:
            eHdmiVideoTiming = HDMITX_RES_1920x1080p_60Hz;
            break;
        case E_MI_HDMI_TIMING_1080_30P:
            eHdmiVideoTiming = HDMITX_RES_1920x1080p_30Hz;
            break;
        case E_MI_HDMI_TIMING_1080_25P:
            eHdmiVideoTiming = HDMITX_RES_1920x1080p_25Hz;
            break;
        case E_MI_HDMI_TIMING_1080_24P:
            eHdmiVideoTiming = HDMITX_RES_1920x1080p_24Hz;
            break;
        case E_MI_HDMI_TIMING_4K2K_24P:
            eHdmiVideoTiming = HDMITX_RES_3840x2160p_24Hz;
            break;
        case E_MI_HDMI_TIMING_4K2K_25P:
            eHdmiVideoTiming = HDMITX_RES_3840x2160p_25Hz;
            break;
        case E_MI_HDMI_TIMING_4K2K_30P:
            eHdmiVideoTiming = HDMITX_RES_3840x2160p_30Hz;
            break;
        case E_MI_HDMI_TIMING_4K2K_50P:
            eHdmiVideoTiming = HDMITX_RES_3840x2160p_50Hz;
            break;
        case E_MI_HDMI_TIMING_4K2K_60P:
            eHdmiVideoTiming = HDMITX_RES_3840x2160p_60Hz;
            break;
        case E_MI_HDMI_TIMING_4096x2160_24P:
            eHdmiVideoTiming = HDMITX_RES_4096x2160p_24Hz;
            break;
        case E_MI_HDMI_TIMING_4096x2160_25P:
            eHdmiVideoTiming = HDMITX_RES_4096x2160p_25Hz;
            break;
        case E_MI_HDMI_TIMING_4096x2160_30P:
            eHdmiVideoTiming = HDMITX_RES_4096x2160p_30Hz;
            break;
        case E_MI_HDMI_TIMING_4096x2160_50P:
            eHdmiVideoTiming = HDMITX_RES_4096x2160p_50Hz;
            break;
        case E_MI_HDMI_TIMING_4096x2160_60P:
            eHdmiVideoTiming = HDMITX_RES_4096x2160p_60Hz;
            break;
        case E_MI_HDMI_TIMING_1024x768_60P:
            eHdmiVideoTiming = HDMITX_RES_1024x768p_60Hz;
            break;
        case E_MI_HDMI_TIMING_1280x1024_60P:
            eHdmiVideoTiming = HDMITX_RES_1280x1024p_60Hz;
            break;
        case E_MI_HDMI_TIMING_1440x900_60P:
            eHdmiVideoTiming = HDMITX_RES_1440x900p_60Hz;
            break;
        case E_MI_HDMI_TIMING_1600x1200_60P:
            eHdmiVideoTiming = HDMITX_RES_1600x1200p_60Hz;
            break;
        default:
            DBG_ERR("HDMITX %s[%d] Wrong Mode of MI_HDMI_TimingType_e (0x%x)\n", __FUNCTION__, __LINE__, eTimingType);
            return E_MI_HDMI_ERR_UNSUPPORT_TIMING;
    }

    MApi_HDMITx_SetHDMITxMode_CD(eHDMITxMode, eHDMITxColorDepth);
    MApi_HDMITx_SetColorFormat(_stHdmiMgr.stInerAttr.eHdmiTxInColor, _stHdmiMgr.stInerAttr.eHdmiTxOutColor);

    //MApi_HDMITx_AnalogTuning(&stHDMITxTun);

    DBG_INFO("HDMITX %s[%d] eHdmiVideoTiming = %d\n", __FUNCTION__, __LINE__, eHdmiVideoTiming);
    MApi_HDMITx_SetVideoOutputTiming(eHdmiVideoTiming);
    MApi_HDMITx_Exhibit();

    // VS infoframe
    if((E_MI_HDMI_TIMING_4K2K_24P == eTimingType) || (E_MI_HDMI_TIMING_4K2K_25P == eTimingType)
        ||(E_MI_HDMI_TIMING_4K2K_30P == eTimingType) || (E_MI_HDMI_TIMING_4K2K_60P == eTimingType))
    {
        MApi_HDMITx_PKT_User_Define(HDMITX_VS_INFOFRAME, FALSE, HDMITX_CYCLIC_PACKET, 0x0);
        // VS_FCNT=0x0, send one vender packet per VS_FCNT+1 frames

        switch(eTimingType)
        {
            case E_MI_HDMI_TIMING_4K2K_24P:
                MApi_HDMITx_Set_VS_InfoFrame(HDMITX_VIDEO_VS_4k_2k, HDMITx_VIDEO_3D_Not_in_Use, HDMITX_VIDEO_4k2k_24Hz);
                break;
            case E_MI_HDMI_TIMING_4K2K_25P:
                MApi_HDMITx_Set_VS_InfoFrame(HDMITX_VIDEO_VS_4k_2k, HDMITx_VIDEO_3D_Not_in_Use, HDMITX_VIDEO_4k2k_25Hz);
                break;
            case E_MI_HDMI_TIMING_4K2K_30P:
                MApi_HDMITx_Set_VS_InfoFrame(HDMITX_VIDEO_VS_4k_2k, HDMITx_VIDEO_3D_Not_in_Use, HDMITX_VIDEO_4k2k_30Hz);
                break;
            case E_MI_HDMI_TIMING_4096x2160_24P:
                MApi_HDMITx_Set_VS_InfoFrame(HDMITX_VIDEO_VS_4k_2k, HDMITx_VIDEO_3D_Not_in_Use, HDMITx_VIDEO_4k2k_24Hz_SMPTE);
                break;
            default:
                DBG_WRN("HDMITX %s[%d] Invalid 4K2K output mode\n", __FUNCTION__, __LINE__);
                MApi_HDMITx_Set_VS_InfoFrame(HDMITX_VIDEO_VS_No_Addition, HDMITx_VIDEO_3D_Not_in_Use, HDMITx_VIDEO_4k2k_Reserved);
                break;
        }
    }
    else if((E_MI_HDMI_TIMING_2205_24P != eTimingType) && (E_MI_HDMI_TIMING_1920x2205_24P != eTimingType)) // Not 4K and Not 3D
    {
        MApi_HDMITx_PKT_User_Define(HDMITX_VS_INFOFRAME, TRUE, HDMITX_STOP_PACKET, 0xFF);
        MApi_HDMITx_Set_VS_InfoFrame(HDMITX_VIDEO_VS_No_Addition, HDMITx_VIDEO_3D_Not_in_Use, HDMITx_VIDEO_4k2k_Reserved);
    }
    _stHdmiMgr.bHdmiTxRuning = TRUE;

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_SetColorType(MI_HDMI_ColorType_e eColorType)
{
    HDMITX_VIDEO_COLOR_FORMAT eOutColor = HDMITX_VIDEO_COLOR_RGB444;

    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    switch(eColorType)
    {
        case E_MI_HDMI_COLOR_TYPE_RGB444:
            eOutColor = HDMITX_VIDEO_COLOR_RGB444;
            break;
        case E_MI_HDMI_COLOR_TYPE_YCBCR422:
            eOutColor = HDMITX_VIDEO_COLOR_YUV422;
            break;
        case E_MI_HDMI_COLOR_TYPE_YCBCR444:
            eOutColor = HDMITX_VIDEO_COLOR_YUV444;
            break;
        case E_MI_HDMI_COLOR_TYPE_YCBCR420:
            eOutColor = HDMITX_VIDEO_COLOR_YUV420;
            break;
        default:
            DBG_WRN("HDMITX %s[%d] Wrong Mode of MI_HDMI_ColorType_e (%d) \n", __FUNCTION__, __LINE__, eColorType);
            return E_MI_HDMI_ERR_UNSUPPORT_COLORTYPE;
    }

    if(_stHdmiMgr.stInerAttr.eHdmiTxOutColor == eOutColor)
    {
        DBG_INFO("HDMITX %s[%d] Set the same color format setting, skip it!! \n", __FUNCTION__, __LINE__);
        return MI_SUCCESS;
    }
    _stHdmiMgr.stInerAttr.eHdmiTxOutColor = eOutColor;

    if((HDMITX_DVI == _stHdmiMgr.stInerAttr.eHdmiTxMode) || (HDMITX_DVI_HDCP == _stHdmiMgr.stInerAttr.eHdmiTxMode))
    {
        _stHdmiMgr.stInerAttr.eHdmiTxOutColor = HDMITX_VIDEO_COLOR_RGB444;
    }

    if(MApi_HDMITx_ForceHDMIOutputColorFormat(1, _stHdmiMgr.stInerAttr.eHdmiTxOutColor) == FALSE)
    {
        DBG_ERR("HDMITX %s[%d] MApi_HDMITx_ForceHDMIOutputColorFormat!! \n", __FUNCTION__, __LINE__);
        return MI_ERR_HDMI_DRV_RET_FAIL;
    }
    else
    {
        MApi_HDMITx_SetAVMUTE(TRUE);
        //mdelay(100*1000);
        MApi_HDMITx_SetColorFormat(_stHdmiMgr.stInerAttr.eHdmiTxInColor, _stHdmiMgr.stInerAttr.eHdmiTxOutColor);
        //mdelay(100*1000);
        MApi_HDMITx_SetAVMUTE(FALSE);
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_SetColorDepth(MI_HDMI_DeepColor_e eColorDepth)
{
    HDMITX_VIDEO_COLORDEPTH_VAL eColorDepthVal = HDMITX_VIDEO_CD_NoID;

    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }
    DBG_INFO("DEVICE_HDMITX %s[%d] enColorDepth = %d \n", __FUNCTION__,__LINE__, eColorDepth);

    switch(eColorDepth)
    {
        case E_MI_HDMI_DEEP_COLOR_24BIT:
            eColorDepthVal = HDMITX_VIDEO_CD_24Bits;
            break;
        case E_MI_HDMI_DEEP_COLOR_30BIT:
            eColorDepthVal = HDMITX_VIDEO_CD_30Bits;
            break;
        case E_MI_HDMI_DEEP_COLOR_36BIT:
            eColorDepthVal = HDMITX_VIDEO_CD_36Bits;
            break;
        case E_MI_HDMI_DEEP_COLOR_48BIT:
            eColorDepthVal = HDMITX_VIDEO_CD_48Bits;
            break;
        case E_MI_HDMI_DEEP_COLOR_OFF:
            eColorDepthVal = HDMITX_VIDEO_CD_NoID;
            break;
        default:
            DBG_WRN("HDMITX %s[%d] Wrong Mode of MI_HDMI_DeepColor_e \n", __FUNCTION__,__LINE__);
            return E_MI_HDMI_ERR_UNSUPPORT_COLORDEPTH;
    }
    if(_stHdmiMgr.stInerAttr.eHdmiTxColorDepth == eColorDepthVal)
    {
        DBG_WRN("HDMITX %s[%d] Set the same color depth setting, skip it!! \n", __FUNCTION__,__LINE__);
        return MI_SUCCESS;
    }
    _stHdmiMgr.stInerAttr.eHdmiTxColorDepth = eColorDepthVal;
    if(_stHdmiMgr.bHdmiTxRuning)
    {
        //_MI_HDMI_SetTiming(_stHdmiMgr.stAttr.stVideoAttr.eVideoFmt);
        MApi_HDMITx_SetHDMITxMode_CD(_stHdmiMgr.stInerAttr.eHdmiTxMode, _stHdmiMgr.stInerAttr.eHdmiTxColorDepth);
        MApi_HDMITx_Exhibit();
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_SetOutputMode(MI_HDMI_OutputMode_e eOutputMode)
{
    MI_BOOL bIsSupportHDMIMode = FALSE;
    MI_U32 u32StartTime = _GetTime();
    HDMITX_OUTPUT_MODE HDMITX_Mode = HDMITX_HDMI;
    HDMITX_VIDEO_COLOR_FORMAT HDMITx_OutColor = HDMITX_VIDEO_COLOR_RGB444;

    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    DBG_INFO("HDMITX %s[%d] enOutputMode = %d \n", __FUNCTION__,__LINE__, eOutputMode);

    if ((E_MI_HDMI_VIDEO_DVI == eOutputMode) || (E_MI_HDMI_VIDEO_HDMI == eOutputMode))
    {
        MApi_HDMITx_HDCP_StartAuth(FALSE);
    }

    switch (eOutputMode)
    {
        case E_MI_HDMI_VIDEO_HDMI:
            HDMITX_Mode = HDMITX_HDMI;
            MApi_HDMITx_SetHDCPOnOff(FALSE);
            break;
        case E_MI_HDMI_VIDEO_HDMI_HDCP:
            HDMITX_Mode = HDMITX_HDMI_HDCP;
            MApi_HDMITx_SetHDCPOnOff(TRUE);
            break;
        case E_MI_HDMI_VIDEO_DVI:
            HDMITX_Mode = HDMITX_DVI;
            MApi_HDMITx_SetHDCPOnOff(FALSE);
            HDMITx_OutColor = HDMITX_VIDEO_COLOR_RGB444;
            break;
        case E_MI_HDMI_VIDEO_DVI_HDCP:
            HDMITX_Mode = HDMITX_DVI_HDCP;
            MApi_HDMITx_SetHDCPOnOff(TRUE);
            HDMITx_OutColor = HDMITX_VIDEO_COLOR_RGB444;
            break;
        default:
            DBG_WRN("HDMITX %s[%d] Wrong Mode of MI_HDMI_OutputMode_e \n", __FUNCTION__,__LINE__);
            return E_MI_HDMI_ERR_UNSUPPORT_OUTPUTMODE;
    }

    while(TRUE != MApi_HDMITx_EDID_HDMISupport(&bIsSupportHDMIMode))
    {
        if(_DiffFromNow(u32StartTime) > 100) // timeout: 100ms
        {
            if(MApi_HDMITx_EdidChecking())
            {
                MApi_HDMITx_EDID_HDMISupport(&bIsSupportHDMIMode);
            }
            else
            {
                DBG_ERR("HDMITX %s[%d] EDID checking failed!!\n", __FUNCTION__, __LINE__);
            }
            break;
        }
        //mdelay(10*1000); // 10ms
    }

    if(bIsSupportHDMIMode)
    {
        if (HDMITX_DVI == HDMITX_Mode)
        {
            HDMITX_Mode = HDMITX_HDMI;
        }
        else if(HDMITX_DVI_HDCP == HDMITX_Mode)
        {
            HDMITX_Mode = HDMITX_HDMI_HDCP;
        }

        if (TRUE == _MI_HDMI_IsReceiverSupportYPbPr())
        {
            HDMITx_OutColor = HDMITX_VIDEO_COLOR_YUV444;
        }
        else
        {
            HDMITx_OutColor = HDMITX_VIDEO_COLOR_RGB444;
        }
    }
    else
    {
        if (HDMITX_HDMI == HDMITX_Mode)
        {
            HDMITX_Mode = HDMITX_DVI;
        }
        else if (HDMITX_HDMI_HDCP == HDMITX_Mode)
        {
            HDMITX_Mode = HDMITX_DVI_HDCP;
        }
        HDMITx_OutColor = HDMITX_VIDEO_COLOR_RGB444;
    }
    DBG_INFO("HDMITX %s[%d] eHdmiTxMode = %s, eHdmiTxOutColor = %s\n", __FUNCTION__, __LINE__,
        HDMITX_Mode, HDMITx_OutColor);
    _stHdmiMgr.stInerAttr.eHdmiTxMode = HDMITX_Mode;
    _stHdmiMgr.stInerAttr.eHdmiTxOutColor = HDMITx_OutColor;

    if(FALSE == _stHdmiMgr.bHdmiTxRuning)
    {
        MApi_HDMITx_SetHDMITxMode(_stHdmiMgr.stInerAttr.eHdmiTxMode);

        if((HDMITX_DVI_HDCP == HDMITX_Mode)||(HDMITX_HDMI_HDCP == HDMITX_Mode))
        {
            MApi_HDMITx_HDCP_StartAuth(TRUE);
        }
        MApi_HDMITx_SetColorFormat(_stHdmiMgr.stInerAttr.eHdmiTxInColor, _stHdmiMgr.stInerAttr.eHdmiTxOutColor);
    }
    else
    {
        MI_HDMI_IMPL_Close(E_MI_HDMI_ID_0);
        //mdelay(200000); //delay for Rocket reset
        MI_HDMI_IMPL_Open(E_MI_HDMI_ID_0);
        MApi_HDMITx_SetAudioOnOff(TRUE);
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_HDMI_SetAudioConfiguration(MI_HDMI_AudioAttr_t *stAudioAttr)
{
    HDMITX_AUDIO_FREQUENCY eAudioFreq = HDMITX_AUDIO_FREQ_NO_SIG;
    HDMITX_AUDIO_CHANNEL_COUNT eAudioChCnt = 2;
    HDMITX_AUDIO_CODING_TYPE eAudioCodeType = HDMITX_AUDIO_PCM;
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    DBG_INFO("HDMITX %s[%d] Frequency:%d, ChannelCount:%d, CodingType:%d \n", __FUNCTION__,__LINE__,
        stAudioAttr->eSampleRate, stAudioAttr->bIsMultiChannel, stAudioAttr->eCodeType);

    switch (stAudioAttr->eSampleRate)
    {
        case E_MI_HDMI_AUDIO_SAMPLERATE_UNKNOWN:
            eAudioFreq = HDMITX_AUDIO_FREQ_NO_SIG;
            break;
        case E_MI_HDMI_AUDIO_SAMPLERATE_32K:
            eAudioFreq = HDMITX_AUDIO_32K;
            break;
        case E_MI_HDMI_AUDIO_SAMPLERATE_44K:
            eAudioFreq = HDMITX_AUDIO_44K;
            break;
        case E_MI_HDMI_AUDIO_SAMPLERATE_48K:
            eAudioFreq = HDMITX_AUDIO_48K;
            break;
        case E_MI_HDMI_AUDIO_SAMPLERATE_88K:
            eAudioFreq = HDMITX_AUDIO_88K;
            break;
        case E_MI_HDMI_AUDIO_SAMPLERATE_96K:
            eAudioFreq = HDMITX_AUDIO_96K;
            break;
        case E_MI_HDMI_AUDIO_SAMPLERATE_176K:
            eAudioFreq = HDMITX_AUDIO_176K;
            break;
        case E_MI_HDMI_AUDIO_SAMPLERATE_192K:
            eAudioFreq = HDMITX_AUDIO_192K;
            break;
        case E_MI_HDMI_AUDIO_SAMPLERATE_MAX:
        default:
            DBG_WRN("HDMITX %s[%d] Wrong Mode of MI_HDMI_SampleRate_e \n", __FUNCTION__,__LINE__);
            return MI_ERR_HDMI_UNSUPPORT_AFREQ;
    }

    switch (stAudioAttr->bIsMultiChannel)
    {
        case 1:
            eAudioChCnt = HDMITX_AUDIO_CH_8;
            break;
        case 0:
        default:
            eAudioChCnt = HDMITX_AUDIO_CH_2;
            break;
    }

    switch (stAudioAttr->eCodeType)
    {
        case E_MI_HDMI_ACODE_PCM:
            eAudioCodeType = HDMITX_AUDIO_PCM;
            break;
        case E_MI_HDMI_ACODE_NON_PCM:
            eAudioCodeType = HDMITX_AUDIO_NONPCM;
            break;
        default:
            DBG_WRN("HDMITX %s[%d] Wrong Mode of MI_HDMI_AudioCodeType_e \n", __FUNCTION__,__LINE__);
            return MI_ERR_HDMI_UNSUPPORT_ACODETYPE;
    }

    if(_stHdmiMgr.bHdmiTxRuning)
    {
        MApi_HDMITx_SetAudioConfiguration(eAudioFreq, eAudioChCnt, eAudioCodeType);
        _stHdmiMgr.stInerAttr.eAudioChCnt = eAudioChCnt;
        _stHdmiMgr.stInerAttr.eAudioCodeType = eAudioCodeType;
        _stHdmiMgr.stInerAttr.eAudioFreq = eAudioFreq;
    }

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_Init()
{
    if (_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module has been initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }

    _stHdmiMgr.stInerAttr.eAudioChCnt = HDMITX_AUDIO_CH_2;
    _stHdmiMgr.stInerAttr.eAudioCodeType = HDMITX_AUDIO_PCM;
    _stHdmiMgr.stInerAttr.eAudioFreq = HDMITX_AUDIO_48K;
    _stHdmiMgr.stInerAttr.eAudioSrcFmt = HDMITX_AUDIO_FORMAT_PCM;
    _stHdmiMgr.stInerAttr.eHdmiTxColorDepth = HDMITX_VIDEO_CD_NoID;
    _stHdmiMgr.stInerAttr.eHdmiTxInColor = HDMITX_VIDEO_COLOR_RGB444;
    _stHdmiMgr.stInerAttr.eHdmiTxOutColor = HDMITX_VIDEO_COLOR_RGB444;
    _stHdmiMgr.stInerAttr.eHdmiTxMode = HDMITX_HDMI;
    _stHdmiMgr.stInerAttr.eHdmiVideoTiming = HDMITX_RES_1920x1080p_60Hz;

    _stHdmiMgr.stAttr.stVideoAttr.bEnableVideo = TRUE;
    _stHdmiMgr.stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
    _stHdmiMgr.stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_RGB444;
    _stHdmiMgr.stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_VIDEO_HDMI;
    _stHdmiMgr.stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_OFF;
    _stHdmiMgr.stAttr.stAudioAttr.bEnableAudio = TRUE;
    _stHdmiMgr.stAttr.stAudioAttr.bIsMultiChannel = 0;
    _stHdmiMgr.stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_UNKNOWN;
    _stHdmiMgr.stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
    _stHdmiMgr.stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;

    _stHdmiMgr.stAttr.stEnInfoFrame.bEnableAudInfoFrame = FALSE;
    _stHdmiMgr.stAttr.stEnInfoFrame.bEnableAviInfoFrame = FALSE;
    _stHdmiMgr.stAttr.stEnInfoFrame.bEnableMpegInfoFrame = FALSE;
    _stHdmiMgr.stAttr.stEnInfoFrame.bEnableSpdInfoFrame = FALSE;

    _stHdmiMgr.bHdmiTxRuning = FALSE;
    _stHdmiMgr.bPowerOn = FALSE;
    _stHdmiMgr.bAvMute  = FALSE;

    MApi_HDMITx_SetHPDGpioPin(_u8HdmiGpioPin);
    MApi_HDMITx_SetDbgLevel(HDMITX_DRIVER_DBG | HDMITX_DRIVER_DBG_HDCP);

    if (FALSE == MApi_HDMITx_Init())
    {
        DBG_ERR("DEVICE_HDMITX %s[%d] MApi_HDMITx_Init Fail \n", __FUNCTION__, __LINE__);
        return MI_ERR_HDMI_DRV_RET_FAIL;
    }
    _stHdmiMgr.bInitFlag = TRUE;

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_DeInit()
{
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }
    if (FALSE == MApi_HDMITx_Exit())
    {
        DBG_ERR("DEVICE_HDMITX %s[%d] MApi_HDMITx_Exit Fail \n", __FUNCTION__, __LINE__);
        return MI_ERR_HDMI_DRV_RET_FAIL;
    }

    _stHdmiMgr.bInitFlag = FALSE;
    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_Open(MI_HDMI_DeviceId_e eHdmi)
{
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    //MApi_HDMITx_UnHDCPRxControl(E_UNHDCPRX_BLUE_SCREEN); /* Unused HDCP */
    //MApi_HDMITx_HDCPRxFailControl(E_HDCPRXFail_BLUE_SCREEN);

    MApi_HDMITx_TurnOnOff(TRUE);

    MApi_HDMITx_SetHDMITxMode(_stHdmiMgr.stInerAttr.eHdmiTxMode);
    MApi_HDMITx_SetHDMITxMode_CD(_stHdmiMgr.stInerAttr.eHdmiTxMode, _stHdmiMgr.stInerAttr.eHdmiTxColorDepth);
    MApi_HDMITx_SetColorFormat(_stHdmiMgr.stInerAttr.eHdmiTxInColor, _stHdmiMgr.stInerAttr.eHdmiTxOutColor);
    MApi_HDMITx_SetVideoOutputTiming(_stHdmiMgr.stInerAttr.eHdmiVideoTiming);
    MApi_HDMITx_SetAudioConfiguration(_stHdmiMgr.stInerAttr.eAudioFreq, _stHdmiMgr.stInerAttr.eAudioChCnt,
        _stHdmiMgr.stInerAttr.eAudioCodeType);

    MApi_HDMITx_SetVideoOnOff(TRUE);
    MApi_HDMITx_SetAudioOnOff(FALSE);

    MApi_HDMITx_Exhibit();
    //mdelay(1000*1000);

    _stHdmiMgr.bHdmiTxRuning = TRUE;

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_Close(MI_HDMI_DeviceId_e eHdmi)
{
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    if (FALSE == _stHdmiMgr.bHdmiTxRuning)
    {
        DBG_INFO("HDMITX %s[%d] Already DisConnect \n", __FUNCTION__, __LINE__);
    }
    else
    {
        DBG_INFO("HDMITX %s[%d] DisConnect \n", __FUNCTION__, __LINE__);
        MApi_HDMITx_TurnOnOff(FALSE);
        MApi_HDMITx_SetTMDSOnOff(FALSE);
        MApi_HDMITx_SetVideoOnOff(FALSE);
        MApi_HDMITx_SetAudioOnOff(FALSE);
        _stHdmiMgr.bHdmiTxRuning = FALSE;
    }

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_SetAttr(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_Attr_t *pstAttr)
{
    MI_S32 s32retVal = -1;

    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    HDMI_IMPL_ISVALID_POINT(pstAttr);

    memcpy(&_stHdmiMgr.stAttr, pstAttr, sizeof(MI_HDMI_Attr_t));
    if (eHdmi < E_MI_HDMI_ID_MAX)
    {
        if (TRUE == pstAttr->stVideoAttr.bEnableVideo) // process video attr
        {
            if (pstAttr->stVideoAttr.eColorType < E_MI_HDMI_COLOR_TYPE_MAX)
            {
                s32retVal = _MI_HDMI_SetColorType(pstAttr->stVideoAttr.eColorType);
                if (MI_SUCCESS != s32retVal)
                {
                    DBG_ERR("%s: _MI_HDMI_SetColorType fail (0x%x)!\n", __FUNCTION__, s32retVal);
                    return MI_ERR_HDMI_DRV_RET_FAIL;
                }
                _stHdmiMgr.stAttr.stVideoAttr.eColorType = pstAttr->stVideoAttr.eColorType;
            }
            if (pstAttr->stVideoAttr.eDeepColorMode < E_MI_HDMI_DEEP_COLOR_MAX)
            {
                s32retVal = _MI_HDMI_SetColorDepth(pstAttr->stVideoAttr.eDeepColorMode);
                if (MI_SUCCESS != s32retVal)
                {
                    DBG_ERR("%s: _MI_HDMI_SetColorDepth fail (0x%x)!\n", __FUNCTION__, s32retVal);
                    return MI_ERR_HDMI_DRV_RET_FAIL;
                }
                _stHdmiMgr.stAttr.stVideoAttr.eDeepColorMode = pstAttr->stVideoAttr.eDeepColorMode;
            }
            if (pstAttr->stVideoAttr.eOutputMode < E_MI_HDMI_VIDEO_MAX)
            {
                s32retVal = _MI_HDMI_SetOutputMode(pstAttr->stVideoAttr.eOutputMode);
                if (MI_SUCCESS != s32retVal)
                {
                    DBG_ERR("%s: _MI_HDMI_SetOutputMode fail (0x%x)!\n", __FUNCTION__, s32retVal);
                    return MI_ERR_HDMI_DRV_RET_FAIL;
                }
                _stHdmiMgr.stAttr.stVideoAttr.eOutputMode = pstAttr->stVideoAttr.eOutputMode;
            }
            if (pstAttr->stVideoAttr.eTimingType < E_MI_HDMI_TIMING_MAX)
            {
                s32retVal = _MI_HDMI_SetTiming(pstAttr->stVideoAttr.eTimingType);
                if (MI_SUCCESS != s32retVal)
                {
                    DBG_ERR("%s: _MI_HDMI_SetTiming fail (0x%x)!\n", __FUNCTION__, s32retVal);
                    return MI_ERR_HDMI_DRV_RET_FAIL;
                }
                _stHdmiMgr.stAttr.stVideoAttr.eTimingType = pstAttr->stVideoAttr.eTimingType;
            }
            memcpy(&_stHdmiMgr.stAttr.stEnInfoFrame, &pstAttr->stEnInfoFrame, sizeof(MI_HDMI_EnInfoFrame_t));
        }

        if (TRUE == pstAttr->stAudioAttr.bEnableAudio) // process audio attr
        {
            s32retVal = _MI_HDMI_SetAudioConfiguration(&pstAttr->stAudioAttr);
            if (MI_SUCCESS != s32retVal)
            {
                DBG_ERR("%s: _MI_HDMI_SetAudioConfiguration fail (0x%x)!\n", __FUNCTION__, s32retVal);
                return MI_ERR_HDMI_DRV_RET_FAIL;
            }
            memcpy(&_stHdmiMgr.stAttr.stAudioAttr, &pstAttr->stAudioAttr, sizeof(MI_HDMI_AudioAttr_t));
        }
    }

    return s32retVal;
}

MI_S32 MI_HDMI_IMPL_GetAttr(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_Attr_t *pstAttr)
{
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    HDMI_IMPL_ISVALID_POINT(pstAttr);

    if (eHdmi < E_MI_HDMI_ID_MAX)
    {
        _stHdmiMgr.stAttr.bConnect = MApi_HDMITx_GetRxStatus(); //get HPD PIN level
        memcpy(pstAttr, &_stHdmiMgr.stAttr, sizeof(MI_HDMI_Attr_t));
    }

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_Start(MI_HDMI_DeviceId_e eHdmi)
{
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    if ((FALSE == _stHdmiMgr.bPowerOn) && (eHdmi < E_MI_HDMI_ID_MAX))
    {
        MApi_HDMITx_DisableTMDSCtrl(FALSE);
        MApi_HDMITx_SetTMDSOnOff(TRUE);
        MApi_HDMITx_Exhibit();
        _stHdmiMgr.bPowerOn = TRUE;
    }

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_Stop(MI_HDMI_DeviceId_e eHdmi)
{
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    if ((TRUE == _stHdmiMgr.bPowerOn) && (eHdmi < E_MI_HDMI_ID_MAX))
    {
        MApi_HDMITx_SetAVMUTE(TRUE);
        //Add delay time for avoiding garbage before AV mute, HDMITx driver send AV mute packet per 2 frames.
        //mdelay(100*1000); // MApi_XC_WaitOutputVSync
        MApi_HDMITx_SetTMDSOnOff(FALSE);
        MApi_HDMITx_DisableTMDSCtrl(TRUE);
        _stHdmiMgr.bPowerOn = FALSE;
    }

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_SetAvMute(MI_HDMI_DeviceId_e eHdmi, MI_BOOL bAvMute)
{
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    if (TRUE == bAvMute) //enable av mute
    {
        MApi_HDMITx_SetAVMUTE(TRUE);
        MApi_HDMITx_SetVideoOnOff(FALSE);
        MApi_HDMITx_SetAudioOnOff(FALSE);
        //Add delay time for avoiding garbage before AV mute, HDMITx driver send AV mute packet per 2 frames.
        //mdelay(100*1000); // MApi_XC_WaitOutputVSync
        MApi_HDMITx_SetTMDSOnOff(FALSE);
        _stHdmiMgr.bAvMute = TRUE;
    }
    else
    {
        MApi_HDMITx_SetAVMUTE(FALSE);
        MApi_HDMITx_SetVideoOnOff(TRUE);
        MApi_HDMITx_SetAudioOnOff(TRUE);
        MApi_HDMITx_SetTMDSOnOff(TRUE);
        MApi_HDMITx_Exhibit();
        _stHdmiMgr.bAvMute = FALSE;
    }

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_ForceGetEdid(MI_HDMI_DeviceId_e  eHdmi, MI_HDMI_Edid_t *pstEdidData)
{
    MI_BOOL retVal = FALSE;
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    retVal = MApi_HDMITx_GetEDIDData(pstEdidData->au8Edid, BLOCK0_INDEX); //read block0
    if (FALSE == retVal)
    {
        DBG_ERR("%s: MApi_HDMITx_GetEDIDData fail (0x%x)!\n", __FUNCTION__, retVal);
        return MI_ERR_HDMI_DRV_RET_FAIL;
    }
    retVal = FALSE;
    retVal = MApi_HDMITx_GetEDIDData(pstEdidData->au8Edid + BLOCK_SIZE, BLOCK1_INDEX); //read block1
    if (FALSE == retVal)
    {
        DBG_ERR("%s: MApi_HDMITx_GetEDIDData fail (0x%x)!\n", __FUNCTION__, retVal);
        return MI_ERR_HDMI_DRV_RET_FAIL;
    }
    pstEdidData->bEdidValid = TRUE;
    pstEdidData->u32Edidlength = BLOCK_SIZE * 2;

    return retVal;
}

MI_S32 MI_HDMI_IMPL_SetInfoFrame(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_InfoFrame_t *pstInfoFrame)
{
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    HDMI_IMPL_ISVALID_POINT(pstInfoFrame);
    switch (pstInfoFrame->eInfoFrameType)
    {
        case E_MI_INFOFRAME_TYPE_SPD:
        {
            MI_U8 au8SpdInfoData[25] = {0};

            if(TRUE == _stHdmiMgr.stAttr.stEnInfoFrame.bEnableSpdInfoFrame)
            {
                memset(au8SpdInfoData, 0x00, sizeof(au8SpdInfoData));
                memcpy(au8SpdInfoData, pstInfoFrame->unInforUnit.stSpdInfoFrame.au8VendorName, 8);
                memcpy(au8SpdInfoData+8, pstInfoFrame->unInforUnit.stSpdInfoFrame.au8ProductDescription, 16);
                au8SpdInfoData[sizeof(au8SpdInfoData)-1] = E_DEVICE_HDMITX_SPD_SOURCE_INFO_DIGITAL_STB; //Default
                MApi_HDMITx_PKT_User_Define(HDMITX_SPD_INFOFRAME, TRUE, HDMITX_CYCLIC_PACKET, 1);
                // send 1 packet for every (fcnt + 1) frame, fcnt=1
                if(TRUE != MApi_HDMITx_PKT_Content_Define(HDMITX_SPD_INFOFRAME, au8SpdInfoData, sizeof(au8SpdInfoData)))
                {
                    DBG_ERR("HDMITX %s[%d] MApi_HDMITx_PKT_Content_Define fail !!\n", __FUNCTION__, __LINE__);
                    return MI_ERR_HDMI_DRV_RET_FAIL;
                }
                memcpy(&_stHdmiMgr.stSpdInfoFrame, &pstInfoFrame->unInforUnit.stSpdInfoFrame, sizeof(MI_HDMI_SpdInfoFrame_t));
            }
            else
            {
                MApi_HDMITx_PKT_User_Define(HDMITX_SPD_INFOFRAME, FALSE, HDMITX_CYCLIC_PACKET, 1);
                memset(&_stHdmiMgr.stSpdInfoFrame, 0x00, sizeof(MI_HDMI_SpdInfoFrame_t));
                DBG_INFO("HDMITX %s[%d] Disable SPD info frame \n", __FUNCTION__, __LINE__);
            }
            break;
        }
        case E_MI_INFOFRAME_TYPE_AVI:
            break;
        case E_MI_INFOFRAME_TYPE_AUDIO:
            break;
        case E_MI_INFOFRAME_TYPE_MPEG:
            break;
        case E_MI_INFOFRAME_TYPE_VENDORSPEC:
            break;
        default:
            DBG_ERR("HDMITX %s[%d] Unsupported info frame type !!\n", __FUNCTION__, __LINE__);
            return E_MI_ERR_FAILED;
    }

    return MI_SUCCESS;
}

MI_S32 MI_HDMI_IMPL_GetInfoFrame(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_InfoFrameType_e eInfoFrameType,
    MI_HDMI_InfoFrame_t *pstInfoFrame)
{
    if (!_stHdmiMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return E_MI_HDMI_ERR_NOT_INIT;
    }

    HDMI_IMPL_ISVALID_POINT(pstInfoFrame);
    switch (eInfoFrameType)
    {
        case E_MI_INFOFRAME_TYPE_SPD:
        {
            memcpy(&pstInfoFrame->unInforUnit.stSpdInfoFrame, &_stHdmiMgr.stSpdInfoFrame, sizeof(MI_HDMI_SpdInfoFrame_t));
            break;
        }
        case E_MI_INFOFRAME_TYPE_AVI:
            break;
        case E_MI_INFOFRAME_TYPE_AUDIO:
            break;
        case E_MI_INFOFRAME_TYPE_MPEG:
            break;
        case E_MI_INFOFRAME_TYPE_VENDORSPEC:
            break;
        default:
            DBG_ERR("HDMITX %s[%d] Unsupported info frame type !!\n", __FUNCTION__, __LINE__);
            return E_MI_ERR_FAILED;
    }

    return MI_SUCCESS;
}
