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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   apiXC_ACE.h
/// @brief  ACE API layer Interface
/// @author MStar Semiconductor Inc.
/// CL 227231+:In box case, there is no need to do YUV2RGB transform.
///////////////////////////////////////////////////////////////////////////////////////////////////

/*! \defgroup Video Video modules

 *  \defgroup XC_BE XC_BE modules
 *  \ingroup Video

	ACE is used for	\n
	1. control the picture quality,include brightness, contrast, CSC ,etc.\n
	2. MWE function.\n
	For more information,see \link ACE ACE interface (apiXC_ACE.h) \endlink

 *  \defgroup ACE ACE interface (apiXC_ACE.h)
 *  \ingroup XC_BE

    ACE is used for

    1.control the picture quality,include brightness, contrast, CSC ,etc.
    2.MWE function.

 *  \defgroup ACE_INIT ACE init control
 *  \ingroup ACE

 *! \defgroup ACE_FEATURE ACE feature operation
 *  \ingroup ACE

 *! \defgroup ACE_INFO ACE Infomation pool
 *  \ingroup ACE

 *! \defgroup ACE_ToBeModified ACE api to be modified
 *  \ingroup ACE

 *! \defgroup ACE_ToBeRemove ACE api to be removed
 *  \ingroup ACE
 */

#ifndef _API_XC_ACE_H_
#define _API_XC_ACE_H_

#include "MsDevice.h"
#include "MsCommon.h"
#include "UFO.h"

#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
#define MSIF_ACE_LIB_CODE               {'A','C','E','_'}
#define MSIF_ACE_LIBVER                 {'0','0'}
#define MSIF_ACE_BUILDNUM               {'5','9'}
#define MSIF_ACE_CHANGELIST             {'0','0','6','2','1','0','5','3'}
#define XC_ACE_API_VERSION              /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    MSIF_ACE_LIB_CODE,                  /* IP__                                             */  \
    MSIF_ACE_LIBVER,                    /* 0.0 ~ Z.Z                                        */  \
    MSIF_ACE_BUILDNUM,                  /* 00 ~ 99                                          */  \
    MSIF_ACE_CHANGELIST,                /* CL#                                              */  \
    MSIF_OS

#if defined(UFO_PUBLIC_HEADER_500_3)
#define XC_ACE_HDR_VERSION  1
#else
#define XC_ACE_HDR_VERSION  3
#endif

/*!
   The color information setting.
*/
typedef enum
{
    /// Version
    E_XC_ACE_INFO_TYPE_VERSION,
    /// Contrast
    E_XC_ACE_INFO_TYPE_CONTRAST,
    /// R Gain
    E_XC_ACE_INFO_TYPE_R_GAIN,
    /// G Gain
    E_XC_ACE_INFO_TYPE_G_GAIN,
    /// B Gain
    E_XC_ACE_INFO_TYPE_B_GAIN,
    /// Saturation
    E_XC_ACE_INFO_TYPE_SATURATION,
    /// Hue
    E_XC_ACE_INFO_TYPE_HUE,
    /// Color correction XY R
    E_XC_ACE_INFO_TYPE_COLOR_CORRECTION_XY_R,
    /// Color correction XY G
    E_XC_ACE_INFO_TYPE_COLOR_CORRECTION_XY_G,
    /// Color correction XY B
    E_XC_ACE_INFO_TYPE_COLOR_CORRECTION_XY_B,
    /// Color correction Offset R
    E_XC_ACE_INFO_TYPE_COLOR_CORRECTION_OFFSET_R,
    /// Color correction Offset G
    E_XC_ACE_INFO_TYPE_COLOR_CORRECTION_OFFSET_G,
    /// Color correction Offset B
    E_XC_ACE_INFO_TYPE_COLOR_CORRECTION_OFFSET_B,
    /// R Brightness
    E_XC_ACE_INFO_TYPE_BRIGHTNESS_R,
    /// G Brightness
    E_XC_ACE_INFO_TYPE_BRIGHTNESS_G,
    /// B Brightness
    E_XC_ACE_INFO_TYPE_BRIGHTNESS_B,

    /// for C51 CPU only, other type CPU will always return 0
    E_XC_ACE_INFO_TYPE_COMPILER_VERSION,
    /// for C51 CPU only, will return the YUV to RGB matrix
    E_XC_ACE_INFO_TYPE_YUV_TO_RGB_MATRIX_SEL,

    /// IHC related
    // Users can define color, inorder to match previous settings,
    // R,G,B,C,M,Y,F can also mapping USER_COLOR0 ~ 6
    E_XC_ACE_INFO_TYPE_IHC_COLOR_R,
    E_XC_ACE_INFO_TYPE_IHC_COLOR_G,
    E_XC_ACE_INFO_TYPE_IHC_COLOR_B,
    E_XC_ACE_INFO_TYPE_IHC_COLOR_C,
    E_XC_ACE_INFO_TYPE_IHC_COLOR_M,
    E_XC_ACE_INFO_TYPE_IHC_COLOR_Y,
    E_XC_ACE_INFO_TYPE_IHC_COLOR_F,

    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR1 = E_XC_ACE_INFO_TYPE_IHC_COLOR_R,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR2 = E_XC_ACE_INFO_TYPE_IHC_COLOR_G,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR3 = E_XC_ACE_INFO_TYPE_IHC_COLOR_B,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR4 = E_XC_ACE_INFO_TYPE_IHC_COLOR_C,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR5 = E_XC_ACE_INFO_TYPE_IHC_COLOR_M,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR6 = E_XC_ACE_INFO_TYPE_IHC_COLOR_Y,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR7 = E_XC_ACE_INFO_TYPE_IHC_COLOR_F,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR8,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR9,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR10,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR11,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR12,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR13,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR14,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR15,
    E_XC_ACE_INFO_TYPE_IHC_USER_COLOR0,

    /// ICC related
    // Users can define color, inorder to match previous settings,
    // R,G,B,C,M,Y,F can also mapping USER_COLOR0 ~ 6
    E_XC_ACE_INFO_TYPE_ICC_COLOR_R,
    E_XC_ACE_INFO_TYPE_ICC_COLOR_G,
    E_XC_ACE_INFO_TYPE_ICC_COLOR_B,
    E_XC_ACE_INFO_TYPE_ICC_COLOR_C,
    E_XC_ACE_INFO_TYPE_ICC_COLOR_M,
    E_XC_ACE_INFO_TYPE_ICC_COLOR_Y,
    E_XC_ACE_INFO_TYPE_ICC_COLOR_F,

    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR1 = E_XC_ACE_INFO_TYPE_ICC_COLOR_R,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR2 = E_XC_ACE_INFO_TYPE_ICC_COLOR_G,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR3 = E_XC_ACE_INFO_TYPE_ICC_COLOR_B,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR4 = E_XC_ACE_INFO_TYPE_ICC_COLOR_C,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR5 = E_XC_ACE_INFO_TYPE_ICC_COLOR_M,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR6 = E_XC_ACE_INFO_TYPE_ICC_COLOR_Y,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR7 = E_XC_ACE_INFO_TYPE_ICC_COLOR_F,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR8,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR9,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR10,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR11,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR12,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR13,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR14,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR15,
    E_XC_ACE_INFO_TYPE_ICC_USER_COLOR0,

    /// IBC related
    // Users can define color, inorder to match previous settings,
    // R,G,B,C,M,Y,F can also mapping USER_COLOR0 ~ 6
    E_XC_ACE_INFO_TYPE_IBC_COLOR_R,
    E_XC_ACE_INFO_TYPE_IBC_COLOR_G,
    E_XC_ACE_INFO_TYPE_IBC_COLOR_B,
    E_XC_ACE_INFO_TYPE_IBC_COLOR_C,
    E_XC_ACE_INFO_TYPE_IBC_COLOR_M,
    E_XC_ACE_INFO_TYPE_IBC_COLOR_Y,
    E_XC_ACE_INFO_TYPE_IBC_COLOR_F,

    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR1 = E_XC_ACE_INFO_TYPE_IBC_COLOR_R,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR2 = E_XC_ACE_INFO_TYPE_IBC_COLOR_G,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR3 = E_XC_ACE_INFO_TYPE_IBC_COLOR_B,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR4 = E_XC_ACE_INFO_TYPE_IBC_COLOR_C,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR5 = E_XC_ACE_INFO_TYPE_IBC_COLOR_M,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR6 = E_XC_ACE_INFO_TYPE_IBC_COLOR_Y,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR7 = E_XC_ACE_INFO_TYPE_IBC_COLOR_F,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR8,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR9,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR10,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR11,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR12,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR13,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR14,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR15,
    E_XC_ACE_INFO_TYPE_IBC_USER_COLOR0,
    //max,min contrast
    E_XC_ACE_INFO_TYPE_CONTRAST_MIN_VALUE,
    E_XC_ACE_INFO_TYPE_CONTRAST_MAX_VALUE,
    /// Dummy
    E_XC_ACE_INFO_TYPE_DUMMY
} E_XC_ACE_INFOTYPE;

/*!
    The convert method of YUV to RGB
*/
typedef enum
{
    /// YUV to RGB matrix - Use Std CSCM for SDTV
    E_XC_ACE_YUV_TO_RGB_MATRIX_SDTV,
    /// YUV to RGB matrix - Use Std CSCM for HDTV
    E_XC_ACE_YUV_TO_RGB_MATRIX_HDTV,
    /// Use user define CSCM, specified by tUserYVUtoRGBMatrix
    E_XC_ACE_YUV_TO_RGB_MATRIX_USER,
} E_XC_ACE_YUVTORGBTYPE;

/*!
   The Effect Settings of Multi Window
*/
typedef enum
{
    /// off
    E_XC_ACE_MWE_MODE_OFF,
    /// H split, reference window at right side,default
    E_XC_ACE_MWE_MODE_H_SPLIT,
    /// Move
    E_XC_ACE_MWE_MODE_MOVE,
    /// Zoom
    E_XC_ACE_MWE_MODE_ZOOM,
    /// H Scan
    E_XC_ACE_MWE_MODE_H_SCAN,
    /// H split, reference window at left side
    E_XC_ACE_MWE_MODE_H_SPLIT_LEFT,
    /// The number of Scaler ACE MWE functoin
    E_XC_ACE_MWE_MODE_NUMS,
} E_XC_ACE_MWE_FUNC;

/*!
   The Result of XC_ACE function call.
*/
typedef enum
{
    /// fail
    E_XC_ACE_FAIL = 0,
    /// ok
    E_XC_ACE_OK = 1,
    /// get base address failed when initialize panel driver
    E_XC_ACE_GET_BASEADDR_FAIL,
    /// obtain mutex timeout when calling this function
    E_XC_ACE_OBTAIN_RESOURCE_FAIL,
} E_XC_ACE_RESULT;


typedef enum
{
    E_XC_ACE_MWE_MIRROR_NORMAL,
    E_XC_ACE_MWE_MIRROR_H_ONLY,
    E_XC_ACE_MWE_MIRROR_V_ONLY,
    E_XC_ACE_MWE_MIRROR_HV,
    E_XC_ACE_MWE_MIRROR_MAX,
}E_XC_ACE_MWE_MIRROR_TYPE;

/*!
   obosolte!! use XC_ACE_color_temp_ex2 instead
*/
typedef struct DLL_PACKED
{
    /// red color offset
    MS_U8 cRedOffset;
    /// green color offset
    MS_U8 cGreenOffset;
    /// blue color offset
    MS_U8 cBlueOffset;

    /// red color
    MS_U8 cRedColor;
    /// green color
    MS_U8 cGreenColor;
    /// blue color
    MS_U8 cBlueColor;

    /// scale 100 value of red color
    MS_U8 cRedScaleValue;
    /// scale 100 value of green color
    MS_U8 cGreenScaleValue;
    /// scale 100 value of blue color
    MS_U8 cBlueScaleValue;
} XC_ACE_color_temp;

/*!
   obosolte!! use XC_ACE_color_temp_ex2 instead
*/
typedef struct DLL_PACKED
{
    /// red color offset
    MS_U16 cRedOffset;
    /// green color offset
    MS_U16 cGreenOffset;
    /// blue color offset
    MS_U16 cBlueOffset;

    /// red color
    MS_U8 cRedColor;
    /// green color
    MS_U8 cGreenColor;
    /// blue color
    MS_U8 cBlueColor;

    /// scale 100 value of red color
    MS_U8 cRedScaleValue;
    /// scale 100 value of green color
    MS_U8 cGreenScaleValue;
    /// scale 100 value of blue color
    MS_U8 cBlueScaleValue;
} XC_ACE_color_temp_ex;

/*!
   The color temp settings ex2.
*/
typedef struct DLL_PACKED
{
    /// red color offset
    MS_U16 cRedOffset;
    /// green color offset
    MS_U16 cGreenOffset;
    /// blue color offset
    MS_U16 cBlueOffset;

    /// red color
    MS_U16 cRedColor;
    /// green color
    MS_U16 cGreenColor;
    /// blue color
    MS_U16 cBlueColor;

    /// scale 100 value of red color
    MS_U16 cRedScaleValue;
    /// scale 100 value of green color
    MS_U16 cGreenScaleValue;
    /// scale 100 value of blue color
    MS_U16 cBlueScaleValue;
} XC_ACE_color_temp_ex2;

//----------------------------
// XC ACE initialize
//----------------------------
/*!
   The ACE initialze DATA.
*/
typedef struct __attribute__((packed))
{
    MS_BOOL    eWindow;                  ///< initial eWindow

    MS_S16*    S16ColorCorrectionMatrix; ///< initial Color Correction Matrix
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#if !defined (__aarch64__)
    MS_U32 u32AlignmentDummy0;
#endif
#endif
    MS_S16*    S16RGB;                   ///< initial RGB
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#if !defined (__aarch64__)
    MS_U32 u32AlignmentDummy1;
#endif
#endif
    MS_U16     u16MWEHstart;             ///< initial Horizontal start


    MS_U16     u16MWEVstart;             ///< initial Vertical start

    MS_U16     u16MWEWidth;              ///< initial MWE Width

    MS_U16     u16MWEHeight;             ///< initial MWE Height

    MS_U16     u16MWE_Disp_Hstart;       ///< initial MWE Display Horizontal start

    MS_U16     u16MWE_Disp_Vstart;       ///< initial MWE Display Vertical start

    MS_U16     u16MWE_Disp_Width;        ///< initial MWE Display width

    MS_U16     u16MWE_Disp_Height;       ///< initial MWE Display height

    MS_BOOL    bMWE_Enable;              ///< Enable or not
} XC_ACE_InitData;

/*!
   The XC_ACE library infomation.
*/
typedef struct
{

} XC_ACE_ApiInfo;

/*!
   The XC_ACE status infomation.
*/
typedef struct DLL_PACKED
{
    /// Scaler ACE API status
    XC_ACE_InitData ACEinit;
} XC_ACE_ApiStatus;


/*!
   The XC_ACE DNR parameters settings.
*/
typedef enum
{
    /// NR ONOFF
    E_ACE_DNR_NR_ONOFF,
    /// SNR
    E_ACE_DNR_SNR,
    /// Spike NR 0
    E_ACE_DNR_SPIKE_NR_0,
    /// Spike NR 1
    E_ACE_DNR_SPIKE_NR_1,
    /// Gray ground gain
    E_ACE_DNR_GRAY_GROUND_GAIN,
    /// Gray ground EN
    E_ACE_DNR_GRAY_GROUND_EN,
    /// Peaking bank coring
    E_ACE_DNR_PEAKING_BANK_CORING,
    /// Guassin SNR threshold
    E_ACE_DNR_GUASSIN_SNR_THRESHOLD,
    /// Motion
    E_ACE_DNR_MOTION,
    /// Peaking coring threshold
    E_ACE_DNR_PEAKING_CORING_THRESHOLD,
    /// Sharpness adjust
    E_ACE_DNR_SHARPNESS_ADJUST,
    /// NM V
    E_ACE_DNR_NM_V,
    /// GNR 0
    E_ACE_DNR_GNR_0,
    /// GNR 1
    E_ACE_DNR_GNR_1,
    /// CP
    E_ACE_DNR_CP,
    /// DP
    E_ACE_DNR_DP,
    /// NM H 0
    E_ACE_DNR_NM_H_0,
    /// HM H 1
    E_ACE_DNR_NM_H_1,
    /// SC coring
    E_ACE_DNR_SC_CORING,
    /// NM
    E_ACE_DNR_SNR_NM,
}XC_ACE_DNR_Param;

typedef enum
{
    // Users can define color, inorder to match previous settings,
    // R,G,B,C,M,Y,F can also mapping USER_COLOR0 ~ 6

    E_ACE_IHC_COLOR_R,
    E_ACE_IHC_COLOR_G,
    E_ACE_IHC_COLOR_B,
    E_ACE_IHC_COLOR_C,
    E_ACE_IHC_COLOR_M,
    E_ACE_IHC_COLOR_Y,
    E_ACE_IHC_COLOR_F,

    E_ACE_IHC_USER_COLOR1 = E_ACE_IHC_COLOR_R,
    E_ACE_IHC_USER_COLOR2 = E_ACE_IHC_COLOR_G,
    E_ACE_IHC_USER_COLOR3 = E_ACE_IHC_COLOR_B,
    E_ACE_IHC_USER_COLOR4 = E_ACE_IHC_COLOR_C,
    E_ACE_IHC_USER_COLOR5 = E_ACE_IHC_COLOR_M,
    E_ACE_IHC_USER_COLOR6 = E_ACE_IHC_COLOR_Y,
    E_ACE_IHC_USER_COLOR7 = E_ACE_IHC_COLOR_F,
    E_ACE_IHC_USER_COLOR8,
    E_ACE_IHC_USER_COLOR9,
    E_ACE_IHC_USER_COLOR10,
    E_ACE_IHC_USER_COLOR11,
    E_ACE_IHC_USER_COLOR12,
    E_ACE_IHC_USER_COLOR13,
    E_ACE_IHC_USER_COLOR14,
    E_ACE_IHC_USER_COLOR15,
    E_ACE_IHC_USER_COLOR0,
    E_ACE_IHC_COLOR_MAX,
}XC_ACE_IHC_COLOR_TYPE;

typedef enum
{
    // Users can define color, inorder to match previous settings,
    // R,G,B,C,M,Y,F can also mapping USER_COLOR0 ~ 6

    E_ACE_ICC_COLOR_R,
    E_ACE_ICC_COLOR_G,
    E_ACE_ICC_COLOR_B,
    E_ACE_ICC_COLOR_C,
    E_ACE_ICC_COLOR_M,
    E_ACE_ICC_COLOR_Y,
    E_ACE_ICC_COLOR_F,

    E_ACE_ICC_USER_COLOR1 = E_ACE_ICC_COLOR_R,
    E_ACE_ICC_USER_COLOR2 = E_ACE_ICC_COLOR_G,
    E_ACE_ICC_USER_COLOR3 = E_ACE_ICC_COLOR_B,
    E_ACE_ICC_USER_COLOR4 = E_ACE_ICC_COLOR_C,
    E_ACE_ICC_USER_COLOR5 = E_ACE_ICC_COLOR_M,
    E_ACE_ICC_USER_COLOR6 = E_ACE_ICC_COLOR_Y,
    E_ACE_ICC_USER_COLOR7 = E_ACE_ICC_COLOR_F,
    E_ACE_ICC_USER_COLOR8,
    E_ACE_ICC_USER_COLOR9,
    E_ACE_ICC_USER_COLOR10,
    E_ACE_ICC_USER_COLOR11,
    E_ACE_ICC_USER_COLOR12,
    E_ACE_ICC_USER_COLOR13,
    E_ACE_ICC_USER_COLOR14,
    E_ACE_ICC_USER_COLOR15,
    E_ACE_ICC_USER_COLOR0,
    E_ACE_ICC_COLOR_MAX,
}XC_ACE_ICC_COLOR_TYPE;

typedef enum
{
    // Users can define color, inorder to match previous settings,
    // R,G,B,C,M,Y,F can also mapping USER_COLOR0 ~ 6

    E_ACE_IBC_COLOR_R,
    E_ACE_IBC_COLOR_G,
    E_ACE_IBC_COLOR_B,
    E_ACE_IBC_COLOR_C,
    E_ACE_IBC_COLOR_M,
    E_ACE_IBC_COLOR_Y,
    E_ACE_IBC_COLOR_F,

    E_ACE_IBC_USER_COLOR1 = E_ACE_IBC_COLOR_R,
    E_ACE_IBC_USER_COLOR2 = E_ACE_IBC_COLOR_G,
    E_ACE_IBC_USER_COLOR3 = E_ACE_IBC_COLOR_B,
    E_ACE_IBC_USER_COLOR4 = E_ACE_IBC_COLOR_C,
    E_ACE_IBC_USER_COLOR5 = E_ACE_IBC_COLOR_M,
    E_ACE_IBC_USER_COLOR6 = E_ACE_IBC_COLOR_Y,
    E_ACE_IBC_USER_COLOR7 = E_ACE_IBC_COLOR_F,
    E_ACE_IBC_USER_COLOR8,
    E_ACE_IBC_USER_COLOR9,
    E_ACE_IBC_USER_COLOR10,
    E_ACE_IBC_USER_COLOR11,
    E_ACE_IBC_USER_COLOR12,
    E_ACE_IBC_USER_COLOR13,
    E_ACE_IBC_USER_COLOR14,
    E_ACE_IBC_USER_COLOR15,
    E_ACE_IBC_USER_COLOR0,
    E_ACE_IBC_COLOR_MAX,
}XC_ACE_IBC_COLOR_TYPE;

#define ACE_INFOEX_MODE_POST_RGB_GAIN           0x1
#define ACE_INFOEX_MODE_POST_RGB_OFFSET         0x2
typedef struct DLL_PACKED
{
    MS_U32 u32Mode;
    MS_U32 u32Reserved;
}XC_ACE_InfoEx;

//--------------------------------------------------------------------------------------------------
// MWE Load Visual Effect Table related
//--------------------------------------------------------------------------------------------------
#define ACE_TABINFO_VERSION                0

#define ACE_TABINFO_REG_ADDR_SIZE          (2)
#define ACE_TABINFO_REG_MASK_SIZE          (1)
#define ACE_TABINFO_REG_DATA_SIZE          (2)

typedef struct DLL_PACKED
{
    MS_U8 *pTable;
    MS_U8 u8TabCols;
    MS_U8 u8TabRows;
    MS_U8 u8TabIdx;
    MS_U32 u32ACE_TabInfo_version; //Version of current structure, including the content of pTable
}XC_ACE_TAB_Info;
//--------------------------------------------------------------------------------------------------

//------------------------------
// Weave Type: used for 3D
//------------------------------
typedef enum
{
    E_ACE_WEAVETYPE_NONE        = 0x00,
    E_ACE_WEAVETYPE_H          = 0x01,
    E_ACE_WEAVETYPE_V          = 0x02,
    E_ACE_WEAVETYPE_DUALVIEW    = 0x04,
    E_ACE_WEAVETYPE_NUM,
}XC_ACE_WEAVETYPE;

typedef struct __attribute__((packed))
{
    MS_U8      u8ColorPrimaries;
    MS_U8      u8TransferCharacteristics;
    MS_U8      u8MatrixCoefficients;
} XC_ACE_HDRMetadataMpegVUI;

///HDR Code////
typedef struct __attribute__((packed))
{
    MS_U16 u16Smin; // 0.10
    MS_U16 u16Smed; // 0.10
    MS_U16 u16Smax; // 0.10
    MS_U16 u16Tmin; // 0.10
    MS_U16 u16Tmed; // 0.10
    MS_U16 u16Tmax; // 0.10
    MS_U16 u16MidSourceOffset;
    MS_U16 u16MidTargetOffset;
    MS_U16 u16MidSourceRatio;
    MS_U16 u16MidTargetRatio;
} XC_ACE_HDRToneMappingData;

typedef struct __attribute__((packed))
{
    MS_U16 u16tRx; // target Rx
    MS_U16 u16tRy; // target Ry
    MS_U16 u16tGx; // target Gx
    MS_U16 u16tGy; // target Gy
    MS_U16 u16tBx; // target Bx
    MS_U16 u16tBy; // target By
    MS_U16 u16tWx; // target Wx
    MS_U16 u16tWy; // target Wy
} XC_ACE_HDRGamutMappingData;

typedef struct __attribute__((packed))
{
    MS_U8      u8EOTF; // 0:SDR gamma, 1:HDR gamma, 2:SMPTE ST2084, 3:Future EOTF, 4-7:Reserved
    MS_U16     u16Rx; // display primaries Rx
    MS_U16     u16Ry; // display primaries Ry
    MS_U16     u16Gx; // display primaries Gx
    MS_U16     u16Gy; // display primaries Gy
    MS_U16     u16Bx; // display primaries Bx
    MS_U16     u16By; // display primaries By
    MS_U16     u16Wx; // display primaries Wx
    MS_U16     u16Wy; // display primaries Wy
    MS_U16     u16Lmax; // max display mastering luminance
    MS_U16     u16Lmin; // min display mastering luminance
    MS_U16     u16MaxCLL; // maximum content light level
    MS_U16     u16MaxFALL; // maximum frame-average light level
} XC_ACE_HDRMetadataHdmiTxInfoFrame;

typedef struct  __attribute__((packed))
{
    MS_U8 PixelFormat;              // Pixel Format
    MS_U8 Colorimetry;              // Color imetry
    MS_U8 ExtendedColorimetry;      // Extended Color imetry
    MS_U8 RgbQuantizationRange;     // Rgb Quantization Range
    MS_U8 YccQuantizationRange;     // Ycc Quantization Range
    MS_U8 StaticMetadataDescriptorID; //Static Metadata Descriptor ID
} XC_ACE_HDRHdmiTxAviInfoFrame;

/*!
 *  Initial  HDR   Settings
 */
typedef struct __attribute__((packed))
{
    /// HDR Version Info
    MS_U16 u16HDRVerInfo;
    /// HDR init Length
    MS_U16 u16HDRInitLength;
    /// HDR Enable
    MS_BOOL bHDREnable;
    /// HDR Function Select
    MS_U16 u16HDRFunctionSelect;
    /// HDR Metadata Mpeg VUI
    XC_ACE_HDRMetadataMpegVUI ACE_HDRMetadataMpegVUI;
    //HDR Tone Mapping Data
    XC_ACE_HDRToneMappingData ACE_HDRToneMappingData;
    //HDR Gamut Mapping Data
    XC_ACE_HDRGamutMappingData ACE_HDRGamutMappingData;
    //HDR Metadata Hdmi Tx Info Frame
    XC_ACE_HDRMetadataHdmiTxInfoFrame ACE_HDRMetadataHdmiTxInfoFrame;
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
    //HDR Hdmi Tx Avi Info Frame
    XC_ACE_HDRHdmiTxAviInfoFrame ACE_HDRHdmiTxAviInfoFrame;
#endif
} XC_ACE_HDRinit;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
#define XC_ACE_BYPASS_COLOR_GAIN  128 //necessary for HW color tempture adjustment,
                                      //used after T2, i.e: T3/T4/T7/T8/Janus..

/********************************************************************************/
/*                   msAPI_ACE.c                   */
/********************************************************************************/

//-------------------------------------------------------------------------------------------------
/// ACE Initiation
/// @ingroup ACE_INIT
/// @param  pstXC_ACE_InitData                  \b IN: @ref XC_ACE_InitData
/// @param  u32InitDataLen                      \b IN: The Length of pstXC_ACE_InitData.
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_Init(XC_ACE_InitData *pstXC_ACE_InitData, MS_U32 u32InitDataLen);


//-------------------------------------------------------------------------------------------------
/// ACE Exit
/// @ingroup ACE_INIT
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_Exit(void);

//-------------------------------------------------------------------------------------------------
/// ACE DMS
/// @ingroup ACE_FEATURE
/// @param  eWindow                 \b IN: Indicates the window where the ACE function applies to.
/// @param  bisATV                  \b IN: Whether the active input source is or not ATV.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_DMS(MS_BOOL eWindow, MS_BOOL bisATV);

//-------------------------------------------------------------------------------------------------
/// Set PC YUV to RGB
/// @ingroup ACE_FEATURE
/// @param  eWindow                  \b IN: Indicates the window where the ACE function applies to.
/// @param  bEnable                  \b IN: Enable or Disalbe the function.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_SetPCYUV2RGB(MS_BOOL eWindow, MS_BOOL bEnable);

//-------------------------------------------------------------------------------------------------
/// Select YUV to RGB Matrix, if the ucMatrix type is ACE_YUV_TO_RGB_MATRIX_USER, then apply the
/// psUserYUVtoRGBMatrix supplied by user.
/// @ingroup ACE_FEATURE
/// @param  eWindow                       \b IN: Indicates the window where the ACE function applies to.
/// @param  ucMatrix                      \b IN: @ref E_ACE_YUVTORGBInfoType.
/// @param  psUserYUVtoRGBMatrix          \b IN: User-Defined color transformed matrix.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_SelectYUVtoRGBMatrix(MS_BOOL eWindow, MS_U8 ucMatrix, MS_S16* psUserYUVtoRGBMatrix);

//-------------------------------------------------------------------------------------------------
/// Set color correction table
/// @ingroup ACE_FEATURE
/// @param  bScalerWin                      \b IN: Indicates the window where the ACE function
///                                                 applies to, 0: Main Window, 1: Sub Window.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_SetColorCorrectionTable( MS_BOOL bScalerWin);

//-------------------------------------------------------------------------------------------------
/// Set PCs RGB table
/// @ingroup ACE_FEATURE
/// @param  bScalerWin                      \b IN: Indicates the window where the ACE function
///                                                 applies to, 0: Main Window, 1: Sub Window.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_SetPCsRGBTable( MS_BOOL bScalerWin);

//-------------------------------------------------------------------------------------------------
/// Get color matrix
/// @ingroup ACE_FEATURE
/// @param  eWindow                      \b IN: Indicates the window where the ACE function applies to.
/// @param  pu16Matrix                   \b OUT: a MS_U16 matrix represents current color matrix
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_GetColorMatrix(MS_BOOL eWindow, MS_U16* pu16Matrix);

//-------------------------------------------------------------------------------------------------
/// Set color matrix
/// @ingroup ACE_FEATURE
/// @param  eWindow                      \b IN: Indicates the window where the ACE function applies to.
/// @param  pu16Matrix                   \b IN: The Matrix given to set the color transformation.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_SetColorMatrix( MS_BOOL eWindow, MS_U16* pu16Matrix);

//-------------------------------------------------------------------------------------------------
/// Set Bypass Color Matrix
/// @ingroup ACE_FEATURE
/// @param bEnable      \b IN:  Enable : Bypass Set Color Matrix
/// @return @ref E_XC_ACE_RESULT
//-------------------------------------------------------------------------------------------------
E_XC_ACE_RESULT MApi_XC_ACE_SetBypassColorMatrix(MS_BOOL bEnable );

//-------------------------------------------------------------------------------------------------
/// Set IHC value
/// @ingroup ACE_FEATURE
/// @param  bScalerWin                   \b IN: Indicates the window where the ACE function applies to.
/// @param  eIHC                         \b IN: Indicates the color to be set.
/// @param  u8Val                        \b IN: The value set to the color(0 ~ 127). (middle value is 64)
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_SetIHC(MS_BOOL bScalerWin, XC_ACE_IHC_COLOR_TYPE eIHC, MS_U8 u8Val);

//-------------------------------------------------------------------------------------------------
/// Set ICC value
/// @ingroup ACE_FEATURE
/// @param  bScalerWin                   \b IN: Indicates the window where the ACE function applies to.
/// @param  eICC                         \b IN: Indicates the color to be set.
/// @param  u8Val                        \b IN: The value set to the color(0 ~ 31). (middle value is 16)
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL SYMBOL_WEAK MApi_XC_ACE_SetICC(MS_BOOL bScalerWin, XC_ACE_ICC_COLOR_TYPE eICC, MS_U8 u8Val);

//-------------------------------------------------------------------------------------------------
/// Set IBC value
/// @ingroup ACE_FEATURE
/// @param  bScalerWin                   \b IN: Indicates the window where the ACE function applies to.
/// @param  eIBC                         \b IN: Indicates the color to be set.
/// @param  u8Val                        \b IN: The value set to the color(0 ~ 63). (middle value is 32)
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL SYMBOL_WEAK MApi_XC_ACE_SetIBC(MS_BOOL bScalerWin, XC_ACE_IBC_COLOR_TYPE eIBC, MS_U8 u8Val);

//-------------------------------------------------------------------------------------------------
/// Patch DTG color checker
/// @ingroup ACE_FEATURE
// @param  u8Mode                       \b IN: TBD
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_PatchDTGColorChecker( MS_U8 u8Mode);

//-------------------------------------------------------------------------------------------------
/// Get ACE information
/// @ingroup ACE_Tobemodified
/// @param  bWindow                      \b IN: Indicates the window where the ACE function
///                                              applies to, 0: Main Window, 1: Sub Window.
/// @param  eXCACEInfoType                \b IN: Specify the information type users interested in.
/// @return @ref MS_U16
//-------------------------------------------------------------------------------------------------
MS_U16  MApi_XC_ACE_GetACEInfo( MS_BOOL bWindow, E_XC_ACE_INFOTYPE eXCACEInfoType );

//-------------------------------------------------------------------------------------------------
/// Get ACE extra information
/// @ingroup ACE_Tobemodified
/// @param  bWindow                      \b IN: Indicates the window where the ACE function
///                                              applies to, 0: Main Window, 1: Sub Window.
/// @param  XC_ACE_InfoEx                \b OUT: Specify the extra information returned by driver.
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_GetACEInfoEx( MS_BOOL bWindow, XC_ACE_InfoEx *pInfoEx);

/********************************************************************************/
/*                   msAPI_ACE_Ext.c                   */
/********************************************************************************/
//-------------------------------------------------------------------------------------------------
/// Picture set contrast
/// @ingroup ACE_FEATURE
/// @param  eWindow                       \b IN: Indicates the window where the ACE function applies to.
/// @param  bUseYUVSpace                  \b IN: Use YUV format or RGB format.
/// @param  u8Contrast                    \b IN: Contrase value given by user.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_PicSetContrast( MS_BOOL eWindow, MS_BOOL bUseYUVSpace, MS_U8 u8Contrast);

//-------------------------------------------------------------------------------------------------
/// Picture set brightness
/// @ingroup ACE_Tobemodified
/// @param  u8Brightness_R                    \b IN: R value given by user.
/// @param  u8Brightness_G                    \b IN: G value given by user.
/// @param  u8Brightness_B                    \b IN: B value given by user.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_PicSetBrightness( MS_BOOL eWindow, MS_U8 u8Brightness_R, MS_U8 u8Brightness_G, MS_U8 u8Brightness_B);

//-------------------------------------------------------------------------------------------------
/// Picture set precise brightness
/// @ingroup ACE_Tobemodified
/// @param  u16Brightness_R                    \b IN: R value given by user.
/// @param  u16Brightness_G                    \b IN: G value given by user.
/// @param  u16Brightness_B                    \b IN: B value given by user.
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_PicSetBrightnessPrecise(MS_BOOL eWindow, MS_U16 u16Brightness_R, MS_U16 u16Brightness_G, MS_U16 u16Brightness_B);

//-------------------------------------------------------------------------------------------------
/// This function will set Pre Y Offset
/// @ingroup ACE_FEATURE
///@param eWindow           \b IN: Window type
///@param u8PreYOffset      \b IN: Pre Y Offset value
/// @return @ref E_XC_ACE_RESULT
//-------------------------------------------------------------------------------------------------
E_XC_ACE_RESULT MApi_XC_ACE_PicSetPreYOffset(MS_BOOL eWindow, MS_U8 u8PreYOffset);

//-------------------------------------------------------------------------------------------------
/// This function will get Pre Y Offset
/// @ingroup ACE_FEATURE
///@param eWindow           \b IN: Window type
/// @return @ref MS_U8
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_XC_ACE_PicGetPreYOffset(MS_BOOL eWindow);

//-------------------------------------------------------------------------------------------------
/// Picture set Hue
/// @ingroup ACE_FEATURE
/// @param  eWindow                  \b IN: Indicates the window where the ACE function applies to.
/// @param  bUseYUVSpace             \b IN: Use YUV format or RGB format.
/// @param  u8Hue                    \b IN: Hue value given by user.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_PicSetHue( MS_BOOL eWindow, MS_BOOL bUseYUVSpace, MS_U8 u8Hue );

//-------------------------------------------------------------------------------------------------
/// Skip Wait Vsync
/// @ingroup ACE_FEATURE
/// @param eWindow               \b IN: Enable
/// @param Skip wait Vsync      \b IN: Disable wait Vsync
/// @return @ref E_XC_ACE_RESULT
//-------------------------------------------------------------------------------------------------
E_XC_ACE_RESULT MApi_XC_ACE_SkipWaitVsync( MS_BOOL eWindow,MS_BOOL bIsSkipWaitVsyn);

//-------------------------------------------------------------------------------------------------
/// Picture set Saturation
/// @ingroup ACE_FEATURE
/// @param  eWindow                  \b IN: Indicates the window where the ACE function applies to.
/// @param  bUseYUVSpace             \b IN: Use YUV format or RGB format.
/// @param  u8Saturation             \b IN: Saturation value given by user.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_PicSetSaturation(MS_BOOL eWindow, MS_BOOL bUseYUVSpace, MS_U8 u8Saturation );

//-------------------------------------------------------------------------------------------------
/// Adjust sharpness
/// @ingroup ACE_FEATURE
/// @param  eWindow                  \b IN: Indicates the window where the ACE function applies to.
/// @param u8Sharpness                \b IN: sharpness value : 0~0x3f
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_PicSetSharpness(  MS_BOOL eWindow, MS_U8 u8Sharpness );

//-------------------------------------------------------------------------------------------------
/// Picture set color temp.
/// @ingroup ACE_ToBeModified
/// @param  eWindow                  \b IN: Indicates the window where the ACE function applies to.
/// @param  bUseYUVSpace             \b IN: Use YUV format or RGB format.
/// @param  pstColorTemp             \b IN: Color temp info given by user.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_PicSetColorTemp( MS_BOOL eWindow, MS_BOOL bUseYUVSpace, XC_ACE_color_temp* pstColorTemp);

//-------------------------------------------------------------------------------------------------
/// Picture set color temp ex.
/// @ingroup ACE_ToBeModified
/// @param  eWindow                  \b IN: Indicates the window where the ACE function applies to.
/// @param  bUseYUVSpace             \b IN: Use YUV format or RGB format.
/// @param  pstColorTemp             \b IN: Color temp info given by user.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_PicSetColorTempEx( MS_BOOL eWindow, MS_BOOL bUseYUVSpace, XC_ACE_color_temp_ex* pstColorTemp);

//-------------------------------------------------------------------------------------------------
/// Picture set post color temp
/// @ingroup ACE_ToBeModified
/// V02. Change the fields in XC_ACE_color_temp_ex structure to MS_U16
/// @param  eWindow                  \b IN: Indicates the window where the ACE function applies to.
/// @param  pstColorTemp             \b IN: Color temp info given by user.
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_PicSetPostColorTemp_V02(MS_BOOL eWindow, XC_ACE_color_temp_ex2* pstColorTemp);

#ifndef _API_XC_ACE_C_
// Projects without ENABLE_NEW_COLORTEMP_METHOD defined will set ENABLE_NEW_COLORTEMP_METHOD to 0
#ifndef ENABLE_NEW_COLORTEMP_METHOD
  #define ENABLE_NEW_COLORTEMP_METHOD 0
#endif

#ifndef ENABLE_PRECISE_RGBBRIGHTNESS
  #define ENABLE_PRECISE_RGBBRIGHTNESS 0
#endif

#if ENABLE_NEW_COLORTEMP_METHOD
    #if ENABLE_PRECISE_RGBBRIGHTNESS
        #define MApi_XC_ACE_PicSetColorTemp(x, y, z) MApi_XC_ACE_PicSetPostColorTemp2Ex(x, y, z)
    #else
        #define MApi_XC_ACE_PicSetColorTemp(x, y, z) MApi_XC_ACE_PicSetPostColorTemp2(x, y, z)
    #endif
#else
    #if ENABLE_PRECISE_RGBBRIGHTNESS
        #define MApi_XC_ACE_PicSetColorTemp(x, y, z) MApi_XC_ACE_PicSetColorTempEx(x, y, z)
    #endif
#endif

#define MApi_XC_ACE_PicSetBrightnessInVsync(bWin, x, y, z) {MApi_XC_WaitOutputVSync(2, 100, bWin); MApi_XC_ACE_PicSetBrightness(bWin, x, y, z);}
#define MApi_XC_ACE_PicSetBrightnessPreciseInVsync(bWin, x, y, z) {MApi_XC_WaitOutputVSync(2, 100, bWin); MApi_XC_ACE_PicSetBrightnessPrecise(bWin, x, y, z);}

#endif

//-------------------------------------------------------------------------------------------------
/// Set Flesh tone
/// @ingroup ACE_FEATURE
/// @param  eWindow                  \b IN: Indicates the window where the ACE function applies to.
/// @param  bEn                      \b IN: @ref MS_BOOL
/// @param  u8FleshTone              \b IN: Flesh Tone Value
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_SetFleshTone(MS_BOOL eWindow, MS_BOOL bEn, MS_U8 u8FleshTone);

//-------------------------------------------------------------------------------------------------
/// black adjsutment
/// @ingroup ACE_FEATURE
/// @param  eWindow                           \b IN: window type.
/// @param  u8BlackAdjust                     \b IN: adjust value given.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_SetBlackAdjust(MS_BOOL eWindow, MS_U8 u8BlackAdjust);

//-------------------------------------------------------------------------------------------------
/// Set the IHC sram
/// @ingroup ACE_FEATURE
/// @param  pBuf                            \b IN: sram data
/// @param  u16ByteSize                     \b IN: size of sram
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_Set_IHC_SRAM(MS_U16 *pBuf, MS_U16 u16ByteSize);

//-------------------------------------------------------------------------------------------------
/// Set the ICC sram
/// @ingroup ACE_FEATURE
/// @param  pBuf                            \b IN: sram data
/// @param  u16ByteSize                     \b IN: size of sram
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_Set_ICC_SRAM(MS_U16 *pBuf, MS_U16 u16ByteSize);

/********************************************************************************/
/*                  MWE function                  */
/********************************************************************************/

//-------------------------------------------------------------------------------------------------
/// Enable MWE
/// @ingroup ACE_FEATURE
/// @param  bEnable                     \b IN: @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_EnableMWE(MS_BOOL bEnable);

//-------------------------------------------------------------------------------------------------
/// MWE Clone Main Window Visual Effect
/// @ingroup ACE_FEATURE
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_MWECloneVisualEffect(void);

//-------------------------------------------------------------------------------------------------
/// MWE Apply Visual Effect From Reg Table
/// @ingroup ACE_FEATURE
/// @param  pMWETable                 \b IN: MWE table
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_MWESetRegTable(XC_ACE_TAB_Info *pMWETable);

//-------------------------------------------------------------------------------------------------
/// Flag for MWE Visual Effect Coming From Reg Table
/// @ingroup ACE_FEATURE
/// @param  bEnable                     \b IN: @ref MS_BOOL
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_SetMWELoadFromPQ(MS_BOOL bEnable);

//-------------------------------------------------------------------------------------------------
/// MWE status
/// @ingroup ACE_INFO
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_MWEStatus(void);

//-------------------------------------------------------------------------------------------------
/// MWE Handle
/// @ingroup ACE_FEATURE
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_MWEHandle( void );

//-------------------------------------------------------------------------------------------------
/// MWE function selection
/// @ingroup ACE_FEATURE
/// @param  eWindow                 \b IN: Indicates the window where the ACE function applies to.
/// @param  mwe_func                     \b IN: @ref E_XC_ACE_MWE_FUNC
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_MWEFuncSel( MS_BOOL eWindow, E_XC_ACE_MWE_FUNC  mwe_func);

//-------------------------------------------------------------------------------------------------
/// MWE set display window
/// @ingroup ACE_FEATURE
/// @param  u16MWE_Disp_Hstart             \b IN: Horizotal position of start point. 0:means leftest position.
/// @param  u16MWE_Disp_Vstart             \b IN: Vertical position of start point, 0: mean up most position.
/// @param  u16MWE_Disp_Width              \b IN: Width of display window.
/// @param  u16MWE_Disp_Height             \b IN: Height of display winodw.
//-------------------------------------------------------------------------------------------------
void    MApi_XC_ACE_MWESetDispWin(MS_U16 u16MWE_Disp_Hstart, MS_U16 u16MWE_Disp_Vstart, MS_U16 u16MWE_Disp_Width, MS_U16 u16MWE_Disp_Height);

//-------------------------------------------------------------------------------------------------
/// 3D clone main and sub window's PQmap
/// @ingroup ACE_FEATURE
/// @param  enWeaveType             \b IN: Output WeaveType
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_3DClonePQMap(XC_ACE_WEAVETYPE enWeaveType);

/********************************************************************************/
/*                  DynamicNR function                                          */
/********************************************************************************/

//-------------------------------------------------------------------------------------------------
/// DNR Blending NR table
/// @ingroup ACE_ToBeModified
/// @param  pu8NewNR              \b IN: new NR target values.
/// @param  u8Weight              \b IN: Blending weighting.
/// @param  u8Step                \b IN: maxmium step size.
/// @param  pu16PreY0             \b IN: previous NR value 0.
/// @param  pu16PreY1             \b IN: previous NR value 1.
/// @param  pu8NRTBL              \b OUT: The new NR Table.
//-------------------------------------------------------------------------------------------------
void   MApi_XC_ACE_DNR_Blending_NRTbl(MS_U8 *pu8NewNR, MS_U8 u8Weight, MS_U8 u8Step, MS_U16 *pu16PreY0, MS_U16 *pu16PreY1, MS_U8 *pu8NRTBL);

//-------------------------------------------------------------------------------------------------
/// DNR Blending MISC
/// @ingroup ACE_ToBeModified
/// @param  u8NewItem              \b IN: New DNR blending target value.
/// @param  u16PreItem             \b IN: previous value.
/// @param  u8Weight               \b IN: Blending weighting.
/// @param  u8Step                 \b IN: Maxmium step size.
/// @return @ref MS_U16           \b return new DNR blending value.
//-------------------------------------------------------------------------------------------------
MS_U16 MApi_XC_ACE_DNR_Blending_MISC(MS_U8 u8NewItem, MS_U16 u16PreItem, MS_U8 u8Weight, MS_U8 u8Step);

//-------------------------------------------------------------------------------------------------
/// DNR Blending
/// @ingroup ACE_ToBeModified
/// @param  u8NewItem              \b IN: New DNR blending target value.
/// @param  u16PreItem             \b IN: previous value.
/// @param  u8Weight               \b IN: Blending weighting.
/// @return @ref MS_U8           \b return new DNR blending value.
//-------------------------------------------------------------------------------------------------
MS_U8  MApi_XC_ACE_DNR_Blending(MS_U8 u8NewItem, MS_U16 u16PreItem, MS_U8 u8Weight);

//-------------------------------------------------------------------------------------------------
/// DNR get precision shift
/// @ingroup ACE_FEATURE
/// @return @ref MS_U16           \b return shift value.
//-------------------------------------------------------------------------------------------------
MS_U16 MApi_XC_ACE_DNR_Get_PrecisionShift(void);

//-------------------------------------------------------------------------------------------------
/// DNR get motion weight
/// @ingroup ACE_FEATURE
/// @param  u8CurMotion                 \b IN: current motion value
/// @param  u8PreMotion                 \b IN: previous motion value
/// @param  u8CurMotionLvl              \b IN: current motion catalog-level
/// @param  u8PreMotionLvl              \b IN: previous motion catalog-level
/// @param  u8DeFlick_Thre              \b IN: de-flick threshold.
/// @param  u8DeFilick_Step             \b IN: de-flick step size.
/// @return @ref MS_U8                    \b Return motion weight
//-------------------------------------------------------------------------------------------------
MS_U8  MApi_XC_ACE_DNR_GetMotion_Weight(MS_U8 u8CurMotion, MS_U8 u8PreMotion, MS_U8 u8CurMotionLvl, MS_U8 u8PreMotionLvl, MS_U8 u8DeFlick_Thre, MS_U8 u8DeFilick_Step);

//-------------------------------------------------------------------------------------------------
/// DNR get Luma weight
/// @ingroup ACE_FEATURE
/// @param  u8CurAvgLuma                 \b IN: current avergae luma
/// @param  u8PreAvgLuam                 \b IN: previous average luma
/// @param  u8CurLumaLvl                 \b IN: current avergae luma catalog-level
/// @param  u8PreLumaLvl                 \b IN: previous avergae luma catalog-level
/// @param  u8DeFlick_Th                 \b IN: de-flick threshold.
/// @param  u8DeFlick_Step               \b IN: de-flick step size.
/// @return @ref MS_U8                    \b Return luma weight
//-------------------------------------------------------------------------------------------------
MS_U8  MApi_XC_ACE_DNR_GetLuma_Weight(MS_U8 u8CurAvgLuma, MS_U8 u8PreAvgLuam, MS_U8 u8CurLumaLvl, MS_U8 u8PreLumaLvl, MS_U8 u8DeFlick_Th,  MS_U8 u8DeFlick_Step);

//-------------------------------------------------------------------------------------------------
/// DNR get noise weight
/// @ingroup ACE_FEATURE
/// @param  u8CurNoise                 \b IN: current noise.
/// @param  u8PreNoise                 \b IN: previous noise.
/// @param  u8Range                    \b IN: noise threshold, if difference between PreNoise and
///                                          CurNoise > this value, then the stable status is down
///                                          -grade for 1 degree.
/// @param  u8DeFlick_Th              \b IN: de-flick threshold.
/// @param  u8DeFlick_Step            \b IN: de-flick step size.
/// @return @ref MS_U8                 \b Return  Noise weighting value.
//-------------------------------------------------------------------------------------------------
MS_U8  MApi_XC_ACE_DNR_GetNoise_Weight(MS_U8 u8CurNoise, MS_U8 u8PreNoise, MS_U8 u8Range, MS_U8 u8DeFlick_Th, MS_U8 u8DeFlick_Step);

//-------------------------------------------------------------------------------------------------
/// DNR get motion degree, lower one means its more like a still video.
/// @ingroup ACE_FEATURE
/// @return @ref MS_U8                    \b Return motion degree value.
//-------------------------------------------------------------------------------------------------
MS_U8  MApi_XC_ACE_DNR_GetMotion(void);

//-------------------------------------------------------------------------------------------------
/// DNR init motion
/// @ingroup ACE_INIT
//-------------------------------------------------------------------------------------------------
void   MApi_XC_ACE_DNR_Init_Motion(void);

//-------------------------------------------------------------------------------------------------
/// DNR init luma
/// @ingroup ACE_INIT
//-------------------------------------------------------------------------------------------------
void   MApi_XC_ACE_DNR_Init_Luma(void);

//-------------------------------------------------------------------------------------------------
/// DNR init noise
/// @ingroup ACE_INIT
//-------------------------------------------------------------------------------------------------
void   MApi_XC_ACE_DNR_Init_Noise(void);

//-------------------------------------------------------------------------------------------------
/// Set parameter of a DNR item specified by user.
/// @ingroup ACE_FEATURE
/// @param  eWindow                 \b IN: Indicates the window where the ACE function applies to.
/// @param  eParam                 \b IN: @ref XC_ACE_DNR_Param
/// @param  u16val                 \b IN: Setting value of DNR item.
//-------------------------------------------------------------------------------------------------
void   MApi_XC_ACE_DNR_SetParam(MS_BOOL eWindow, XC_ACE_DNR_Param eParam, MS_U16 u16val);

//-------------------------------------------------------------------------------------------------
/// Set Set HDR Init
/// @ingroup ACE_FEATURE
// @param  pstACE_HDRInitData             \b IN: HDR Init Info
//-------------------------------------------------------------------------------------------------
MS_BOOL SYMBOL_WEAK MApi_XC_ACE_SetHDRInit(XC_ACE_HDRinit *pstACE_HDRInitData);

//-------------------------------------------------------------------------------------------------
/// Set Contrast by Gamma Table
// @param  bEnable             \b IN: enable contrast by gamma table or not
// @param  u8Contrast             \b IN: contrast value
// @param  u8BaseValue             \b IN: base value for the slope of luma mapping table, defualt value is 128
//-------------------------------------------------------------------------------------------------
void SYMBOL_WEAK MApi_XC_ACE_SetContrastByGammaTable(MS_BOOL bScalerWin, MS_BOOL bEnable, MS_U8 u8Contrast, MS_U8 u8BaseValue);

//-------------------------------------------------------------------------------------------------
/// Get parameter of a DNR item specified by user.
/// @ingroup ACE_FEATURE
/// @param  eWindow                 \b IN: Indicates the window where the ACE function applies to.
/// @param  eParam                 \b IN: @ref XC_ACE_DNR_Param
/// @return @ref MS_U8
//-------------------------------------------------------------------------------------------------
MS_U8  MApi_XC_ACE_DNR_GetParam(MS_BOOL eWindow, XC_ACE_DNR_Param eParam);

//-------------------------------------------------------------------------------------------------
/// Set NR table
/// @ingroup ACE_FEATURE
/// @param  pu8Tbl                 \b IN: NR table given by user.
//-------------------------------------------------------------------------------------------------
void   MApi_XC_ACE_DNR_SetNRTbl(MS_U8 *pu8Tbl);

// must have functions
//-------------------------------------------------------------------------------------------------
/// Get version (without Mutex protect)
/// @ingroup ACE_INFO
/// @param  ppVersion                 \b OUT: Version information of ACE lib.
/// @return @ref E_XC_ACE_RESULT
//-------------------------------------------------------------------------------------------------
E_XC_ACE_RESULT MApi_XC_ACE_GetLibVer(const MSIF_Version **ppVersion);

//-------------------------------------------------------------------------------------------------
/// Get info from driver (without Mutex protect)
/// @ingroup ACE_INFO
/// @return XC_ACE_ApiInfo
//-------------------------------------------------------------------------------------------------
const XC_ACE_ApiInfo * MApi_XC_ACE_GetInfo(void);

//-------------------------------------------------------------------------------------------------
/// Get panel current status
/// @ingroup ACE_INFO
/// @param  pDrvStatus                 \b IN: @ref XC_ACE_ApiStatus
/// @param  bWindow                    \b IN: Indicates the window where the ACE function
///                                           applies to, 0: Main Window, 1: Sub Window.
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_GetStatus(XC_ACE_ApiStatus *pDrvStatus, MS_BOOL bWindow);

//-------------------------------------------------------------------------------------------------
/// Set debug level (without Mutex protect)
/// @ingroup ACE_INFO
/// @param  u16DbgSwitch                 \b IN: debug switch value, use TRUE/FALSE to turn on/off.
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_ACE_SetDbgLevel(MS_U16 u16DbgSwitch);

//-------------------------------------------------------------------------------------------------
/// Set Color correction table
/// @ingroup ACE_FEATURE
/// @param  u16DbgSwitch                 \b IN: debug switch value, use TRUE/FALSE to turn on/off.
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_ColorCorrectionTable( MS_BOOL bScalerWin, MS_S16 *psColorCorrectionTable );

//-------------------------------------------------------------------------------------------------
/// Set Color Matrix
/// @ingroup ACE_FEATURE
/// @param  u16DbgSwitch                 \b IN: debug switch value, use TRUE/FALSE to turn on/off.
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_SetColorMatrixControl( MS_BOOL bScalerWin, MS_BOOL bEnable );

//-------------------------------------------------------------------------------------------------
/// Set RG channel range
/// @ingroup ACE_FEATURE
/// @param  u16DbgSwitch                 \b IN: debug switch value, use TRUE/FALSE to turn on/off.
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_SetRBChannelRange( MS_BOOL bScalerWin, MS_BOOL bRange );

//-------------------------------------------------------------------------------------------------
/// Ace power state control for fastboot
/// @ingroup ACE_INIT
/// @param  u16PowerState                 \b IN: power state
/// @return @ref MS_U32                    \b Return  result of power state control.
//-------------------------------------------------------------------------------------------------
MS_U32 MApi_XC_ACE_SetPowerState(EN_POWER_MODE enPowerState);

//////////////////////////////////////////////
// Below functions are obosolted ! Please do not use them if you do not use them yet.
//////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
///-obosolte!! use MApi_XC_ACE_PicSetPostColorTemp_V02 instead
/// @ingroup ACE_ToBeRemove
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_PicSetPostColorTemp(MS_BOOL eWindow, XC_ACE_color_temp* pstColorTemp);

//-------------------------------------------------------------------------------------------------
///-obosolte!! use MApi_XC_ACE_PicSetPostColorTemp_V02 instead
/// @ingroup ACE_ToBeRemove
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_PicSetPostColorTempEx(MS_BOOL eWindow, XC_ACE_color_temp_ex* pstColorTemp);

//-------------------------------------------------------------------------------------------------
///-obosolte!! use MApi_XC_ACE_PicSetPostColorTempWithBypassACESetting instead
/// @ingroup ACE_ToBeRemove
//-------------------------------------------------------------------------------------------------
void MApi_XC_ACE_PicSetPostColorTemp2(MS_BOOL eWindow, MS_BOOL bUseYUVSpace, XC_ACE_color_temp* pstColorTemp);

//*************************************************************************
///-obosolte!! Please bypass color temp by using MApi_XC_ACE_PicSetColorTemp and then set post color temp by using MApi_XC_ACE_PicSetPostColorTemp_V02
/// @ingroup ACE_ToBeRemove
//*************************************************************************
void MApi_XC_ACE_PicSetPostColorTemp2Ex(MS_BOOL eWindow, MS_BOOL bUseYUVSpace, XC_ACE_color_temp_ex* pstColorTemp);



#ifdef __cplusplus
}
#endif

#endif // _API_XC_ACE_H_

