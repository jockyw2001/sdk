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
#ifndef _MI_VDEC_INTERNAL_H_
#define _MI_VDEC_INTERNAL_H_
#include "mi_vdec.h"
#include "mi_sys_datatype.h"

typedef enum
{
    E_MI_VDEC_DB_MODE_H264_H265   = 0x00,
    E_MI_VDEC_DB_MODE_VP9         = 0x01,
    E_MI_VDEC_DB_MODE_MAX
} mi_vdec_DbMode_e; // Decoder Buffer Mode

typedef struct mi_vdec_DbInfo_s
{
    MI_BOOL bDbEnable;           // Decoder Buffer Enable
    MI_U8   u8DbSelect;          // Decoder Buffer select
    MI_BOOL bHMirror;
    MI_BOOL bVMirror;
    MI_BOOL bUncompressMode;
    MI_BOOL bBypassCodecMode;
    mi_vdec_DbMode_e eDbMode;        // Decoder Buffer mode
    MI_U16 u16StartX;
    MI_U16 u16StartY;
    MI_U16 u16HSize;
    MI_U16 u16VSize;
    MI_PHY phyDbBase;          // Decoder Buffer base addr
    MI_U16 u16DbPitch;         // Decoder Buffer pitch
    MI_U8  u8DbMiuSel;         // Decoder Buffer Miu select
    MI_PHY phyLbAddr;          // Lookaside buffer addr
    MI_U8  u8LbSize;           // Lookaside buffer size
    MI_U8  u8LbTableId;        // Lookaside buffer table Id
} mi_vdec_DbInfo_t; // Decoder Buffer Info

typedef enum
{
    E_MI_VDEC_FRAME_TYPE_I = 0,
    E_MI_VDEC_FRAME_TYPE_P,
    E_MI_VDEC_FRAME_TYPE_B,
    E_MI_VDEC_FRAME_TYPE_OTHER,
    E_MI_VDEC_FRAME_TYPE_NUM
} mi_vdec_FrameType_e;

typedef struct
{
    MI_U32 u32LumaAddr;                           // frame buffer base + the start offset of current displayed luma data. Unit: byte.
    MI_U32 u32ChromaAddr;                       // frame buffer base + the start offset of current displayed chroma data. Unit: byte.
    MI_U32 u32TimeStamp;                         // Time stamp(DTS, PTS) of current displayed frame. Unit: ms (todo: 90khz)
    MI_U32 u32IdL;                                    // low part of ID number
    MI_U32 u32IdH;                                   // high part of ID number
    MI_U16 u16Pitch;                                   // pitch
    MI_U16 u16Width;                                  // width
    MI_U16 u16Height;                                 // hight
    mi_vdec_FrameType_e eFrameType;    //< Frame type: I, P, B frame
    MI_SYS_FieldType_e eFieldType;         //< Field type: Top, Bottom, Both
} mi_vdec_FrameInfo_t;

typedef struct mi_vdec_FrameInfoExt_s
{
    MI_PHY phyLumaAddr2bit;                   // physical address of Luma 2bit buffer
    MI_PHY phyChromaAddr2bit;                 // physical address of Chroma 2bit buffer
    MI_U8  u8LumaBitDepth;                    // Luma Frame bitdepth, support 8~10bits now
    MI_U8  u8ChromaBitDepth;                  // Chroma Frame bitdepth, support 8~10bits now
    MI_U16 u16Pitch2bit;                      // pitch of 2bits frame buffer
    MI_SYS_FrameTileMode_e eFrameTileMode;  // Frame tile mode
    MI_SYS_FrameScanMode_e eFrameScanMode;  // Frame scan mode
} mi_vdec_FrameInfoExt_t;

typedef struct mi_vdec_DispFrame_s
{
    mi_vdec_FrameInfo_t stFrmInfo;    //< frame information
    MI_U32 u32PriData;                         //< firmware private data
    MI_U32 u32Idx;                               //< index used by apiVDEC to manage VDEC_DispQ[][]
    mi_vdec_FrameInfoExt_t stFrmInfoExt;        // Frame Info Extend
    mi_vdec_DbInfo_t stDbInfo;
    MI_U64 u64FastChnId;
} mi_vdec_DispFrame_t;

typedef enum
{
    E_MI_VDEC_MFCODEC_UNSUPPORT = 0x00,
    E_MI_VDEC_MFCODEC_10 = 0x01,           // MFDEC version 1.0
    E_MI_VDEC_MFCODEC_20 = 0x05,           // MFDEC version 2.0
    E_MI_VDEC_MFCODEC_25 = 0x06,           // MFDEC version 2.5
    E_MI_VDEC_MFCODEC_30 = 0x07,           // MFDEC version 3.0
    E_MI_VDEC_MFCODEC_DISABLE   = 0xFF,
} mi_vdec_MfCodecVersion_e;

#endif /* _MI_VDEC_INTERNAL_H_ */
