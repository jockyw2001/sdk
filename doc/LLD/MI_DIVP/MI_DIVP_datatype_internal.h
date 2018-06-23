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

#ifndef _MI_DIVP_DATATYPE_INTERNAL_H_
#define _MI_DIVP_DATATYPE_INTERNAL_H_
#pragma pack(push)
#pragma pack(4)

#define MI_DIVP_CHN_NAME_LENTH (12)
#define MI_DIVP_CHN_INPUT_PORT_NUM (1)
#define MI_DIVP_CHN_OUTPUT_PORT_NUM (1)


typedef void (*DispCaptureCB)(void);

typedef enum
{
    E_MI_DIVP_Device0 = 0,         ///< DIP window
    E_MI_DIVP_Device1 = 1,
    E_MI_DIVP_Device2 = 2,
    E_MI_DIVP_Device_MAX          /// The max support window
} mi_divp_DeviceId_e;

typedef enum
{
    E_MI_DIVP_CAP_STAGE_INPUT = 0,
    E_MI_DIVP_CAP_STAGE_OUTPUT = 1,
    E_MI_DIVP_CAP_STAGE_OUTPUT_WITH_OSD = 2,
    E_MI_DIVP_CAP_STAGE_MAX = 3,
} mi_divp_CapStage_e;

typedef enum
{
    E_MI_DIVP_TILE_MODE_NONE     = 0x00,
    E_MI_DIVP_TILE_MODE_16x16    = 0x01,
    E_MI_DIVP_TILE_MODE_16x32    = 0x02,
    E_MI_DIVP_TILE_MODE_32x16    = 0x03,
    E_MI_DIVP_TILE_MODE_32x32    = 0x04,
    E_MI_DIVP_TILE_MODE_MAX,
} mi_divp_TileMode_e;

typedef struct mi_divp_Mirror_s
{
    MI_BOOL bHMirror;
    MI_BOOL bVMirror;
} mi_divp_Mirror_t;

typedef enum
{
    E_MI_DIVP_CHN_INITED,
    E_MI_DIVP_CHN_CREATED,
    E_MI_DIVP_CHN_STARTED,
    E_MI_DIVP_CHN_STOPED,
    E_MI_DIVP_CHN_DISTROYED,
} mi_divp_ChnStatus_e;

typedef struct mi_divp_CaptureInfo_s
{
    struct list_head capture_list;//used in divp mi
    MI_U32 u32Fence;//used in divp mi, init value is 0.
    MI_PHY u32BufAddr[3];//output buffer address
    MI_PHY u32BufSize;//output buffer size
    MI_U16 u16Width;//output frame width
    MI_U16 u16Height;//output frame height
    MI_U16 u16Stride[3];//output(unit: byte)
    MI_SYS_PixelFormat_e ePxlFmt;//output frame pixel format
    mi_disp_Dev_e eDispId;//capture which display
    mi_divp_CapStage_e eCapStage;//capture point in display
    MI_SYS_Rotate_e eRotate;//whether need DIVP to rotate the frame
    mi_divp_Mirror_t stMirror;//whether need DIVP to do mirror
    MI_SYS_WindowRect_t stCropWin;//crop window size
} mi_divp_CaptureInfo_t;

typedef struct mi_divp_FrameInfo_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
    MI_SYS_PixelFormat_e ePxlFmt;
}mi_divp_FrameInfo_t;

typedef struct mi_divp_ChnContex_s
{
    mi_divp_DeviceId_e eDeviceId;
    DIVP_CHN u32ChnId;
    MI_U32 u32OverlayMask;
    MI_BOOL bEnableCusCrop;
    mi_divp_ChnStatus_e eStatus;
    MI_DIVP_ChnAttrParams_t stChnAttr;
    MI_DIVP_ChnAttrParams_t stChnAttrPre;//channel attribute of last.
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_ChnPort_t stInputPort;
    MI_SYS_ChnPort_t stOutputPort;
    mi_divp_FrameInfo_t stInputInfoPre;//input infomation of last frame
    mi_divp_FrameInfo_t stOutputInfoPre;//output infomation of last frame
    void* pHalCtx;
}mi_divp_ChnContex_t;

typedef struct mi_divp_IteratorWorkInfo_s
{
   int totalAddedTask;
}mi_divp_IteratorWorkInfo_t;

#pragma pack(pop)
#endif
