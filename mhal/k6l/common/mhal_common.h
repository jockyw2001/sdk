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
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
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
/// @file   MsTypes.h
/// @brief  MStar General Data Types
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MHAL_COMMON_H_
#define _MHAL_COMMON_H_

#include "MsTypes.h"

//-------------------------------------------------------------------------------------------------
//  System Data Type
//-------------------------------------------------------------------------------------------------




//-------------------------------------------------------------------------------------------------
//  Software Data Type
//-------------------------------------------------------------------------------------------------

typedef MS_U16                      MHAL_AUDIO_DEV;



//-------------------------------------------------------------------------------------------------
// MHAL Interface Return  Value Define
//-------------------------------------------------------------------------------------------------


#define MHAL_SUCCESS    (0)
#define MHAL_FAILURE    (-1)
#define MHAL_ERR_ID  (0x80000000L + 0x20000000L)


/******************************************************************************
|----------------------------------------------------------------|
| 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
|----------------------------------------------------------------|
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/

#define MHAL_DEF_ERR( module, level, errid) \
    ((MS_S32)( (MHAL_ERR_ID) | ((module) << 16 ) | ((level)<<13) | (errid) ))

typedef enum
{
    E_MHAL_ERR_LEVEL_INFO,       /* informational                                */
    E_MHAL_ERR_LEVEL_WARNING,    /* warning conditions                           */
    E_MHAL_ERR_LEVEL_ERROR,      /* error conditions                             */
    E_MHAL_ERR_LEVEL_BUTT
} MHAL_ErrLevel_e;

typedef enum
{
    E_MHAL_ERR_INVALID_DEVID = 1, /* invlalid device ID                           */
    E_MHAL_ERR_INVALID_CHNID = 2, /* invlalid channel ID                          */
    E_MHAL_ERR_ILLEGAL_PARAM = 3, /* at lease one parameter is illagal
                               * eg, an illegal enumeration value             */
    E_MHAL_ERR_EXIST         = 4, /* resource exists                              */
    E_MHAL_ERR_UNEXIST       = 5, /* resource unexists                            */
    E_MHAL_ERR_NULL_PTR      = 6, /* using a NULL point                           */
    E_MHAL_ERR_NOT_CONFIG    = 7, /* try to enable or initialize system, device
                              ** or channel, before configing attribute       */
    E_MHAL_ERR_NOT_SUPPORT   = 8, /* operation or type is not supported by NOW    */
    E_MHAL_ERR_NOT_PERM      = 9, /* operation is not permitted
                              ** eg, try to change static attribute           */
    E_MHAL_ERR_NOMEM         = 12,/* failure caused by malloc memory              */
    E_MHAL_ERR_NOBUF         = 13,/* failure caused by malloc buffer              */
    E_MHAL_ERR_BUF_EMPTY     = 14,/* no data in buffer                            */
    E_MHAL_ERR_BUF_FULL      = 15,/* no buffer for new data                       */
    E_MHAL_ERR_SYS_NOTREADY  = 16,/* System is not ready,maybe not initialed or
                              ** loaded. Returning the error code when opening
                              ** a device file failed.                        */
    E_MHAL_ERR_BADADDR       = 17,/* bad address,
                              ** eg. used for copy_from_user & copy_to_user   */
    E_MHAL_ERR_BUSY          = 18,/* resource is busy,
                              ** eg. destroy a venc chn without unregister it */
    E_MHAL_ERR_BUTT          = 63,/* maxium code, private error code of all modules
                              ** must be greater than it                      */
} MHAL_ErrCode_e;

/// data type 64bit physical address
typedef unsigned long long                       PHY;        // 8 bytes
typedef enum
{
    E_MHAL_PIXEL_FRAME_YUV422_YUYV = 0,
    E_MHAL_PIXEL_FRAME_ARGB8888,
    E_MHAL_PIXEL_FRAME_ABGR8888,

    E_MHAL_PIXEL_FRAME_RGB565,
    E_MHAL_PIXEL_FRAME_ARGB1555,
    E_MHAL_PIXEL_FRAME_I2,
    E_MHAL_PIXEL_FRAME_I4,
    E_MHAL_PIXEL_FRAME_I8,

    E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422,
    E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,
    // mstar mdwin/mgwin
    E_MHAL_PIXEL_FRAME_YUV_MST_420,

    //vdec mstar private video format
    E_MHAL_PIXEL_FRAME_YC420_MSTTILE1_H264,
    E_MHAL_PIXEL_FRAME_YC420_MSTTILE2_H265,
    E_MHAL_PIXEL_FRAME_YC420_MSTTILE3_H265,

    E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GB,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GB,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GB,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_GB,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GB,

    E_MHAL_PIXEL_FRAME_FORMAT_MAX,
} MHalPixelFormat_e;

typedef enum
{
    E_MHAL_RGB_BAYER_RG = 0,
    E_MHAL_RGB_BAYER_GR = 1,
    E_MHAL_RGB_BAYER_GB = 2,
    E_MHAL_RGB_BAYER_BG = 3
} MHalRGBBayerID_e;

typedef enum
{
    E_MHAL_COMPRESS_MODE_NONE,//no compress
    E_MHAL_COMPRESS_MODE_SEG,//compress unit is 256 bytes as a segment
    E_MHAL_COMPRESS_MODE_LINE,//compress unit is the whole line
    E_MHAL_COMPRESS_MODE_FRAME,//compress unit is the whole frame
    E_MHAL_COMPRESS_MODE_BUTT, //number
} MHalPixelCompressMode_e;

typedef enum
{
    E_MHAL_FRAME_SCAN_MODE_PROGRESSIVE = 0x0,  // progessive.
    E_MHAL_FRAME_SCAN_MODE_INTERLACE   = 0x1,  // interlace.
    E_MHAL_FRAME_SCAN_MODE_MAX,
} MHalFrameScanMode_e;

typedef enum
{
    E_MHAL_FIELDTYPE_NONE,        //< no field.
    E_MHAL_FIELDTYPE_TOP,           //< Top field only.
    E_MHAL_FIELDTYPE_BOTTOM,    //< Bottom field only.
    E_MHAL_FIELDTYPE_BOTH,        //< Both fields.
    E_MHAL_FIELDTYPE_NUM
} MHalFieldType_e;

typedef struct MHalWindowRect_s
{
    MS_U16 u16X;
    MS_U16 u16Y;
    MS_U16 u16Width;
    MS_U16 u16Height;
} MHalWindowRect_t;



#endif // _MS_TYPES_H_
