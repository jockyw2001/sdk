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

#ifndef _HAL_DIVP_DATATYPE_H_
#define _HAL_DIVP_DATATYPE_H_
#pragma pack(push)
#pragma pack(4)

#define HAL_DIVP_FENCE (0x1235FEUL)

typedef s32 (*PfnAlloc)(u8 *u8MMAHeapName, u32 u32Size , PHY *phyAddr);
typedef s32 (*PfnFree)(PHY phyAddr);

typedef enum
{
    HAL_DIVP_Device0 = 0,         ///< DIP window
    HAL_DIVP_Device1 = 1,
    HAL_DIVP_Device2 = 2,
    HAL_DIVP_Device_MAX          /// The max support window
}HalDivpDeviceId_e;

typedef enum
{
    HAL_DIVP_ATTR_TNR = 0,
    HAL_DIVP_ATTR_DI,
    HAL_DIVP_ATTR_ROTATE,
    HAL_DIVP_ATTR_CROP,
    HAL_DIVP_ATTR_MIRROR,
} HalDivpAttrType_e;

typedef enum
{
    HAL_DIVP_Display0 = 0,         ///< DIP window
    HAL_DIVP_Display1 = 1,
    HAL_DIVP_Display_MAX          /// The max support window
}HalDivpDisplayId_e;

typedef enum
{
    HAL_DIVP_CAP_STAGE_INPUT = 0,
    HAL_DIVP_CAP_STAGE_OUTPUT = 1,
    HAL_DIVP_CAP_STAGE_OUTPUT_WITH_OSD = 2,
    HAL_DIVP_CAP_STAGE_MAX = 3,
}HalDivpCapStage_e;

typedef enum
{
    HAL_DIVP_TILE_MODE_NONE     = 0x00,
    HAL_DIVP_TILE_MODE_16x16    = 0x01,
    HAL_DIVP_TILE_MODE_16x32    = 0x02,
    HAL_DIVP_TILE_MODE_32x16    = 0x03,
    HAL_DIVP_TILE_MODE_32x32    = 0x04,
    HAL_DIVP_TILE_MODE_MAX,
}HalDivpTileMode_e;

typedef enum
{
    HAL_DIVP_SCAN_MODE_PROGRESSIVE = 0x00,
    HAL_DIVP_SCAN_MODE_INTERLACE,
    HAL_DIVP_SCAN_MODE_MAX,
}HalDivpScanMode_e;

typedef enum
{
    HAL_DIVP_PIXEL_FORMAT_YUV422_YUYV,//(0)
    HAL_DIVP_PIXEL_FORMAT_ARGB8888,//(1)
    HAL_DIVP_PIXEL_FORMAT_ABGR8888,//(2)
    HAL_DIVP_PIXEL_FORMAT_RGB565,//(3)
    HAL_DIVP_PIXEL_FORMAT_ARGB1555,//(4)
    HAL_DIVP_PIXEL_FORMAT_I2,//(5)
    HAL_DIVP_PIXEL_FORMAT_I4,//(6)
    HAL_DIVP_PIXEL_FORMAT_I8,//(7)
    HAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_422,//(8)
    HAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420,//(9)
    HAL_DIVP_PIXEL_FORMAT_YUV_MST_420,//(10)

    //vdec mstar private video format
    HAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264,//(11)
    HAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265,//(12)
    HAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265,//(13)
    HAL_DIVP_PIXEL_FORMAT_MAX,//(14)
}HalDivpPixelFormat_e;

typedef enum
{
    HAL_DIVP_ROTATE_NONE, //Rotate 0 degrees
    HAL_DIVP_ROTATE_90, //Rotate 90 degrees
    HAL_DIVP_ROTATE_180, //Rotate 180 degrees
    HAL_DIVP_ROTATE_270, //Rotate 270 degrees
    HAL_DIVP_ROTATE_NUM,
}HalDivpRotate_e;

typedef enum
{
    HAL_DIVP_FIELD_TYPE_NONE,        //< no field.
    HAL_DIVP_FIELD_TYPE_TOP,           //< Top field only.
    HAL_DIVP_FIELD_TYPE_BOTTOM,    //< Bottom field only.
    HAL_DIVP_FIELD_TYPE_BOTH,        //< Both fields.
    HAL_DIVP_FIELD_TYPE_NUM
}HalDivpFieldType_e;

typedef enum
{
    HAL_DIVP_DB_MODE_H264_H265   = 0x00,
    HAL_DIVP_DB_MODE_VP9         = 0x01,
    HAL_DIVP_DB_MODE_MAX,
}HalDivpDbMode_e; // Decoder Buffer Mode

typedef enum
{
    HAL_DIVP_DI_TYPE_OFF,//off
    HAL_DIVP_DI_TYPE_2D,///2.5D DI
    HAL_DIVP_DI_TYPE_3D,///3D DI
    HAL_DIVP_DI_TYPE_NUM,
}HalDivpDiType_e;

typedef enum
{
    HAL_DIVP_TNR_LEVEL_OFF,
    HAL_DIVP_TNR_LEVEL_LOW,
    HAL_DIVP_TNR_LEVEL_MIDDLE,
    HAL_DIVP_TNR_LEVEL_HIGH,
    HAL_DIVP_TNR_LEVEL_NUM,
}HalDivpTnrLevel_e;

typedef struct
{
    u16 nX;
    u16 nY;
    u16 nWidth;
    u16 nHeight;
}HalDivpWindow_t, *pHalDivpWindow_t;

typedef struct
{
    MI_PHY u32BufAddr[3];//output buffer address
    MI_PHY u32BufSize;//output buffer size
    u16 nWidth;//output frame width
    u16 nHeight;//output frame height
    u16 nStride[3];//pitch (unit: bytes)
    HalDivpPixelFormat_e ePxlFmt;//output frame pixel format
    HalDivpDisplayId_e eDispId;//whitch display will be captured the screen
    HalDivpCapStage_e stCapStage;//the postion captured the frame data
    HalDivpRotate_e eRota;//rotate angles
    HalDivpMirror_t stMirror;//whether need mirror
    HalDivpWindow_t stCropWin;//crop size
}HalDivpCaptureInfo_t, *pHalDivpCaptureInfo_t;

typedef struct
{
    bool bHMirror;
    bool bVMirror;
}HalDivpMirror_t, *pHalDivpMirror_t;

typedef struct
{
    MI_PHY u32BufAddr[3];
    u32 nBufSize;
    u16 nInputWidth;
    u16 nInputHeight;
    u16 nStride[3];
    u64 nPts;
    HalDivpPixelFormat_e ePxlFmt;
    HalDivpTileMode_e eTileMode;
    HalDivpScanMode_e eScanMode;
    HalDivpFieldType_e eFieldType;
    HalDivpMFdecInfo_t* pstMfdecInfo;///MFdec info
    void* pst3dDiInfo;///3D DI info
}HalDivpInputInfo_t, *pHalDivpInputInfo_t;

typedef struct
{
    MI_PHY u32BufAddr[3];
    MI_PHY u32BufSize;
    u16 nOutputWidth;
    u16 nOutputHeight;
    u16 nStride[3];//bytes
    HalDivpPixelFormat_e ePxlFmt;
}HalDivpOutPutInfo_t, *pHalDivpOutPutInfo_t;

typedef struct
{
    bool bDbEnable;// Decoder Buffer Enable
    u8   nDbSelect;// Decoder Buffer select
    HalDivpMirror_t stMirror;
    bool bUncompressMode;
    bool bBypassCodecMode;
    HalDivpDbMode_e eDbMode;// Decoder Buffer mode
    u16 nStartX;
    u16 nStartY;
    u16 nWidth;
    u16 nHeight;
    MI_PHY phyDbBase;// Decoder Buffer base addr
    u16 u16DbPitch;// Decoder Buffer pitch
    u8  nDbMiuSel;// Decoder Buffer Miu select
    MI_PHY phyLbAddr;// Lookaside buffer addr
    u8  nLbSize;// Lookaside buffer size
    u8  nLbTableId;// Lookaside buffer table Id
}HalDivpMFdecInfo_t, *pHalDivpMFdecInfo_t; // Decoder Buffer Info

#pragma pack(pop)
#endif
