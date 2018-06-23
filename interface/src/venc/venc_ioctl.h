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


#ifndef _VENC_IOCTL_H_
#define _VENC_IOCTL_H_
#include <linux/ioctl.h>

#include "mi_venc.h"

typedef enum
{
    E_MI_VENC_CMD_SET_MOD_PARAM,
    E_MI_VENC_CMD_GET_MOD_PARAM,
    E_MI_VENC_CMD_CREATE_CHN,
    E_MI_VENC_CMD_DESTROY_CHN,
    E_MI_VENC_CMD_RESET_CHN,
    E_MI_VENC_CMD_START_RECV_PIC,
    E_MI_VENC_CMD_START_RECV_PIC_EX,
    E_MI_VENC_CMD_STOP_RECV_PIC,
    E_MI_VENC_CMD_QUERY,
    E_MI_VENC_CMD_SET_CHN_ATTR,
    E_MI_VENC_CMD_GET_CHN_ATTR,
    E_MI_VENC_CMD_GET_STREAM,
    E_MI_VENC_CMD_RELEASE_STREAM,
    E_MI_VENC_CMD_INSERT_USER_DATA,
    E_MI_VENC_CMD_SET_MAX_STREAM_CNT,
    E_MI_VENC_CMD_GET_MAX_STREAM_CNT,
    E_MI_VENC_CMD_REQUEST_IDR,
    E_MI_VENC_CMD_ENABLE_IDR,
    E_MI_VENC_CMD_SET_H264_IDR_PIC_ID,
    E_MI_VENC_CMD_GET_H264_IDR_PIC_ID,
    E_MI_VENC_CMD_GET_FD,
    E_MI_VENC_CMD_CLOSE_FD,
    E_MI_VENC_CMD_SET_ROI_CFG,
    E_MI_VENC_CMD_GET_ROI_CFG,
    E_MI_VENC_CMD_SET_ROI_BG_FRAME_RATE,
    E_MI_VENC_CMD_GET_ROI_BG_FRAME_RATE,
    E_MI_VENC_CMD_SET_H264_SLICE_SPLIT,
    E_MI_VENC_CMD_GET_H264_SLICE_SPLIT,
    E_MI_VENC_CMD_SET_H264_INTER_PRED,
    E_MI_VENC_CMD_GET_H264_INTER_PRED,
    E_MI_VENC_CMD_SET_H264_INTRA_PRED,
    E_MI_VENC_CMD_GET_H264_INTRA_PRED,
    E_MI_VENC_CMD_SET_H264_TRANS,
    E_MI_VENC_CMD_GET_H264_TRANS,
    E_MI_VENC_CMD_SET_H264_ENTROPY,
    E_MI_VENC_CMD_GET_H264_ENTROPY,
    E_MI_VENC_CMD_SET_H264_DBLK,
    E_MI_VENC_CMD_GET_H264_DBLK,
    E_MI_VENC_CMD_SET_H264_VUI,
    E_MI_VENC_CMD_GET_H264_VUI,
    E_MI_VENC_CMD_SET_H265_SLICE_SPLIT,
    E_MI_VENC_CMD_GET_H265_SLICE_SPLIT,
    E_MI_VENC_CMD_SET_H265_INTER_PRED,
    E_MI_VENC_CMD_GET_H265_INTER_PRED,
    E_MI_VENC_CMD_SET_H265_INTRA_PRED,
    E_MI_VENC_CMD_GET_H265_INTRA_PRED,
    E_MI_VENC_CMD_SET_H265_TRANS,
    E_MI_VENC_CMD_GET_H265_TRANS,
    E_MI_VENC_CMD_SET_H265_DBLK,
    E_MI_VENC_CMD_GET_H265_DBLK,
    E_MI_VENC_CMD_SET_H265_VUI,
    E_MI_VENC_CMD_GET_H265_VUI,
    E_MI_VENC_CMD_SET_JPEG_PARAM,
    E_MI_VENC_CMD_GET_JPEG_PARAM,
    E_MI_VENC_CMD_SET_RC_PARAM,
    E_MI_VENC_CMD_GET_RC_PARAM,
    E_MI_VENC_CMD_SET_REF_PARAM,
    E_MI_VENC_CMD_GET_REF_PARAM,
    E_MI_VENC_CMD_GET_CROP,
    E_MI_VENC_CMD_SET_CROP,
    E_MI_VENC_CMD_SET_FRAME_LOST_STRATEGY,
    E_MI_VENC_CMD_GET_FRAME_LOST_STRATEGY,
    E_MI_VENC_CMD_SET_SUPER_FRAME_CFG,
    E_MI_VENC_CMD_GET_SUPER_FRAME_CFG,
    E_MI_VENC_CMD_SET_RC_PRIORITY,
    E_MI_VENC_CMD_GET_RC_PRIORITY,
    E_MI_VENC_CMD_GET_CHN_DEVID,
    E_MI_VENC_CMD_MAX,
} MI_VENC_Cmd_e;


typedef struct MI_VENC_CreateChn_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ChnAttr_t stAttr;
} MI_VENC_CreateChn_t;

typedef struct MI_VENC_StartRecvPicEx_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_RecvPicParam_t stRecvParam;
} MI_VENC_StartRecvPicEx_t;

typedef struct MI_VENC_Query_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ChnStat_t stStat;
} MI_VENC_Query_t;

typedef struct MI_VENC_SetChnAttr_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ChnAttr_t stAttr;
} MI_VENC_SetChnAttr_t;

typedef struct MI_VENC_GetChnAttr_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ChnAttr_t stAttr;
} MI_VENC_GetChnAttr_t;

typedef struct MI_VENC_GetStream_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_Stream_t stStream;
    MI_S32 s32MilliSec;
} MI_VENC_GetStream_t;

typedef struct MI_VENC_ReleaseStream_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_Stream_t stStream;
} MI_VENC_ReleaseStream_t;

typedef struct MI_VENC_InsertUserData_s
{
    MI_VENC_CHN VeChn;
    MI_U8 *u8Data;
    MI_U32 u32Len;
} MI_VENC_InsertUserData_t;

typedef struct MI_VENC_SetMaxStreamCnt_s
{
    MI_VENC_CHN VeChn;
    MI_U32 u32MaxStrmCnt;
} MI_VENC_SetMaxStreamCnt_t;

typedef struct MI_VENC_GetMaxStreamCnt_s
{
    MI_VENC_CHN VeChn;
    MI_U32 u32MaxStrmCnt;
} MI_VENC_GetMaxStreamCnt_t;

typedef struct MI_VENC_RequestIdr_s
{
    MI_VENC_CHN VeChn;
    MI_BOOL bInstant;
} MI_VENC_RequestIdr_t;

typedef struct MI_VENC_EnableIdr_s
{
    MI_VENC_CHN VeChn;
    MI_BOOL bEnableIdr;
} MI_VENC_EnableIdr_t;

typedef struct MI_VENC_SetH264IdrPicId_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_H264IdrPicIdCfg_t stH264eIdrPicIdCfg;
} MI_VENC_SetH264IdrPicId_t;

typedef struct MI_VENC_GetH264IdrPicId_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_H264IdrPicIdCfg_t stH264eIdrPicIdCfg ;
} MI_VENC_GetH264IdrPicId_t;

typedef struct MI_VENC_GetFd_s
{
    MI_VENC_CHN VeChn;
    MI_S32 s32Fd;
} MI_VENC_GetFd_t, MI_VENC_CloseFd_t;

typedef struct MI_VENC_SetRoiCfg_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_RoiCfg_t stVencRoiCfg;
} MI_VENC_SetRoiCfg_t;

typedef struct MI_VENC_GetRoiCfg_s
{
    MI_VENC_CHN VeChn;
    MI_U32 u32Index;
    MI_VENC_RoiCfg_t stVencRoiCfg;
} MI_VENC_GetRoiCfg_t;

typedef struct MI_VENC_SetRoiBgFrameRate_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_RoiBgFrameRate_t pstRoiBgFrmRate;
} MI_VENC_SetRoiBgFrameRate_t;

typedef struct MI_VENC_GetRoiBgFrameRate_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_RoiBgFrameRate_t stRoiBgFrmRate;
} MI_VENC_GetRoiBgFrameRate_t;

typedef struct MI_VENC_SetH264SliceSplit_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264SliceSplit_t stSliceSplit;
} MI_VENC_SetH264SliceSplit_t;

typedef struct MI_VENC_GetH264SliceSplit_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264SliceSplit_t stSliceSplit;
} MI_VENC_GetH264SliceSplit_t;

typedef struct MI_VENC_SetH264InterPred_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264InterPred_t stH264InterPred;
} MI_VENC_SetH264InterPred_t;

typedef struct MI_VENC_GetH264InterPred_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264InterPred_t stH264InterPred;
} MI_VENC_GetH264InterPred_t;

typedef struct MI_VENC_SetH264IntraPred_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264IntraPred_t stH264IntraPred;
} MI_VENC_SetH264IntraPred_t;

typedef struct MI_VENC_GetH264IntraPred_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264IntraPred_t stH264IntraPred;
} MI_VENC_GetH264IntraPred_t;

typedef struct MI_VENC_SetH264Trans_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264Trans_t stH264Trans;
} MI_VENC_SetH264Trans_t;

typedef struct MI_VENC_GetH264Trans_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264Trans_t stH264Trans;
} MI_VENC_GetH264Trans_t;

typedef struct MI_VENC_SetH264Entropy_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264Entropy_t stH264EntropyEnc;
} MI_VENC_SetH264Entropy_t;

typedef struct MI_VENC_GetH264Entropy_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264Entropy_t stH264EntropyEnc;
} MI_VENC_GetH264Entropy_t;

typedef struct MI_VENC_SetH264Dblk_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264Dblk_t stH264Dblk;
} MI_VENC_SetH264Dblk_t;

typedef struct MI_VENC_GetH264Dblk_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264Dblk_t stH264Dblk;
} MI_VENC_GetH264Dblk_t;

typedef struct MI_VENC_SetH264Vui_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264Vui_t stH264Vui;
} MI_VENC_SetH264Vui_t;

typedef struct MI_VENC_GetH264Vui_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH264Vui_t stH264Vui;
} MI_VENC_GetH264Vui_t;

typedef struct MI_VENC_SetH265SliceSplit_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265SliceSplit_t stSliceSplit;
} MI_VENC_SetH265SliceSplit_t;

typedef struct MI_VENC_GetH265SliceSplit_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265SliceSplit_t stSliceSplit;
} MI_VENC_GetH265SliceSplit_t;

typedef struct MI_VENC_SetH265InterPred_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265InterPred_t stH265InterPred;
} MI_VENC_SetH265InterPred_t;

typedef struct MI_VENC_GetH265InterPred_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265InterPred_t stH265InterPred;
} MI_VENC_GetH265InterPred_t;

typedef struct MI_VENC_SetH265IntraPred_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265IntraPred_t stH265IntraPred;
} MI_VENC_SetH265IntraPred_t;

typedef struct MI_VENC_GetH265IntraPred_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265IntraPred_t stH265IntraPred;
} MI_VENC_GetH265IntraPred_t;

typedef struct MI_VENC_SetH265Trans_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265Trans_t stH265Trans;
} MI_VENC_SetH265Trans_t;

typedef struct MI_VENC_GetH265Trans_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265Trans_t stH265Trans;
} MI_VENC_GetH265Trans_t;

typedef struct MI_VENC_SetH265Dblk_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265Dblk_t stH265Dblk;
} MI_VENC_SetH265Dblk_t;

typedef struct MI_VENC_GetH265Dblk_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265Dblk_t stH265Dblk;
} MI_VENC_GetH265Dblk_t;

typedef struct MI_VENC_SetH265Vui_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265Vui_t stH265Vui;
} MI_VENC_SetH265Vui_t;

typedef struct MI_VENC_GetH265Vui_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamH265Vui_t stH265Vui;
} MI_VENC_GetH265Vui_t;

typedef struct MI_VENC_SetJpegParam_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamJpeg_t stParamJpeg;
} MI_VENC_SetJpegParam_t;

typedef struct MI_VENC_GetJpegParam_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamJpeg_t stParamJpeg;
} MI_VENC_GetJpegParam_t;


typedef struct MI_VENC_SetRcParam_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_RcParam_t stRcParam;
} MI_VENC_SetRcParam_t;

typedef struct MI_VENC_GetRcParam_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_RcParam_t stRcParam;
} MI_VENC_GetRcParam_t;

typedef struct MI_VENC_SetRefParam_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamRef_t stRefParam;
} MI_VENC_SetRefParam_t;

typedef struct MI_VENC_GetRefParam_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamRef_t stRefParam;
} MI_VENC_GetRefParam_t;

typedef struct MI_VENC_SetCrop_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_CropCfg_t stCropCfg;
} MI_VENC_SetCrop_t;

typedef struct MI_VENC_GetCrop_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_CropCfg_t stCropCfg;
} MI_VENC_GetCrop_t;

typedef struct MI_VENC_SetFrameLostStrategy_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamFrameLost_t stFrmLostParam;
} MI_VENC_SetFrameLostStrategy_t;

typedef struct MI_VENC_GetFrameLostStrategy_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_ParamFrameLost_t stFrmLostParam;
} MI_VENC_GetFrameLostStrategy_t;

typedef struct MI_VENC_SetSuperFrameCfg_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_SuperFrameCfg_t stSuperFrmParam;
} MI_VENC_SetSuperFrameCfg_t;

typedef struct MI_VENC_GetSuperFrameCfg_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_SuperFrameCfg_t stSuperFrmParam;
} MI_VENC_GetSuperFrameCfg_t;

typedef struct MI_VENC_SetRcPriority_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_RcPriority_e eRcPriority;
} MI_VENC_SetRcPriority_t;

typedef struct MI_VENC_GetRcPriority_s
{
    MI_VENC_CHN VeChn;
    MI_VENC_RcPriority_e eRcPriority;
} MI_VENC_GetRcPriority_t;

typedef struct MI_VENC_GetChnDevid_s
{
    MI_VENC_CHN VeChn;
    MI_U32 u32DevId;
} MI_VENC_GetChnDevid_t;

#define MI_VENC_SET_MOD_PARAM _IOW('i', E_MI_VENC_CMD_SET_MOD_PARAM, MI_VENC_CHN)
#define MI_VENC_GET_MOD_PARAM _IOWR('i', E_MI_VENC_CMD_GET_MOD_PARAM, MI_VENC_CHN)
#define MI_VENC_CREATE_CHN _IOW('i', E_MI_VENC_CMD_CREATE_CHN, MI_VENC_CreateChn_t)
#define MI_VENC_DESTROY_CHN _IOW('i', E_MI_VENC_CMD_DESTROY_CHN, MI_VENC_CHN)
#define MI_VENC_RESET_CHN _IOW('i', E_MI_VENC_CMD_RESET_CHN, MI_VENC_CHN)
#define MI_VENC_START_RECV_PIC _IOW('i', E_MI_VENC_CMD_START_RECV_PIC, MI_VENC_CHN)
#define MI_VENC_START_RECV_PIC_EX _IOW('i', E_MI_VENC_CMD_START_RECV_PIC_EX, MI_VENC_StartRecvPicEx_t)
#define MI_VENC_STOP_RECV_PIC _IOW('i', E_MI_VENC_CMD_STOP_RECV_PIC, MI_VENC_CHN)
#define MI_VENC_QUERY _IOWR('i', E_MI_VENC_CMD_QUERY, MI_VENC_Query_t)
#define MI_VENC_SET_CHN_ATTR _IOW('i', E_MI_VENC_CMD_SET_CHN_ATTR, MI_VENC_SetChnAttr_t)
#define MI_VENC_GET_CHN_ATTR _IOWR('i', E_MI_VENC_CMD_GET_CHN_ATTR, MI_VENC_GetChnAttr_t)
#define MI_VENC_GET_STREAM _IOWR('i', E_MI_VENC_CMD_GET_STREAM, MI_VENC_GetStream_t)
#define MI_VENC_RELEASE_STREAM _IOW('i', E_MI_VENC_CMD_RELEASE_STREAM, MI_VENC_ReleaseStream_t)
#define MI_VENC_INSERT_USER_DATA _IOW('i', E_MI_VENC_CMD_INSERT_USER_DATA, MI_VENC_InsertUserData_t)
#define MI_VENC_SET_MAX_STREAM_CNT _IOW('i', E_MI_VENC_CMD_SET_MAX_STREAM_CNT, MI_VENC_SetMaxStreamCnt_t)
#define MI_VENC_GET_MAX_STREAM_CNT _IOWR('i', E_MI_VENC_CMD_GET_MAX_STREAM_CNT, MI_VENC_GetMaxStreamCnt_t)
#define MI_VENC_REQUEST_IDR _IOW('i', E_MI_VENC_CMD_REQUEST_IDR, MI_VENC_RequestIdr_t)
#define MI_VENC_ENABLE_IDR _IOW('i', E_MI_VENC_CMD_ENABLE_IDR, MI_VENC_EnableIdr_t)
#define MI_VENC_SET_H264_IDR_PIC_ID _IOW('i', E_MI_VENC_CMD_SET_H264_IDR_PIC_ID, MI_VENC_SetH264IdrPicId_t)
#define MI_VENC_GET_H264_IDR_PIC_ID _IOWR('i', E_MI_VENC_CMD_GET_H264_IDR_PIC_ID, MI_VENC_GetH264IdrPicId_t)
#define MI_VENC_GET_FD _IOWR('i', E_MI_VENC_CMD_GET_FD, MI_VENC_CHN)
#define MI_VENC_CLOSE_FD _IOW('i', E_MI_VENC_CMD_CLOSE_FD, MI_VENC_CHN)
#define MI_VENC_SET_ROI_CFG _IOW('i', E_MI_VENC_CMD_SET_ROI_CFG, MI_VENC_SetRoiCfg_t)
#define MI_VENC_GET_ROI_CFG _IOWR('i', E_MI_VENC_CMD_GET_ROI_CFG, MI_VENC_GetRoiCfg_t)
#define MI_VENC_SET_ROI_BG_FRAME_RATE _IOW('i', E_MI_VENC_CMD_SET_ROI_BG_FRAME_RATE, MI_VENC_SetRoiBgFrameRate_t)
#define MI_VENC_GET_ROI_BG_FRAME_RATE _IOWR('i', E_MI_VENC_CMD_GET_ROI_BG_FRAME_RATE, MI_VENC_GetRoiBgFrameRate_t)
#define MI_VENC_SET_H264_SLICE_SPLIT _IOW('i', E_MI_VENC_CMD_SET_H264_SLICE_SPLIT, MI_VENC_SetH264SliceSplit_t)
#define MI_VENC_GET_H264_SLICE_SPLIT _IOWR('i', E_MI_VENC_CMD_GET_H264_SLICE_SPLIT, MI_VENC_GetH264SliceSplit_t)
#define MI_VENC_SET_H264_INTER_PRED _IOW('i', E_MI_VENC_CMD_SET_H264_INTER_PRED, MI_VENC_SetH264InterPred_t)
#define MI_VENC_GET_H264_INTER_PRED _IOWR('i', E_MI_VENC_CMD_GET_H264_INTER_PRED, MI_VENC_GetH264InterPred_t)
#define MI_VENC_SET_H264_INTRA_PRED _IOW('i', E_MI_VENC_CMD_SET_H264_INTRA_PRED, MI_VENC_SetH264IntraPred_t)
#define MI_VENC_GET_H264_INTRA_PRED _IOWR('i', E_MI_VENC_CMD_GET_H264_INTRA_PRED, MI_VENC_GetH264IntraPred_t)
#define MI_VENC_SET_H264_TRANS _IOW('i', E_MI_VENC_CMD_SET_H264_TRANS, MI_VENC_SetH264Trans_t)
#define MI_VENC_GET_H264_TRANS _IOWR('i', E_MI_VENC_CMD_GET_H264_TRANS, MI_VENC_GetH264Trans_t)
#define MI_VENC_SET_H264_ENTROPY _IOW('i', E_MI_VENC_CMD_SET_H264_ENTROPY, MI_VENC_SetH264Entropy_t)
#define MI_VENC_GET_H264_ENTROPY _IOWR('i', E_MI_VENC_CMD_GET_H264_ENTROPY, MI_VENC_GetH264Entropy_t)
#define MI_VENC_SET_H264_DBLK _IOW('i', E_MI_VENC_CMD_SET_H264_DBLK, MI_VENC_SetH264Dblk_t)
#define MI_VENC_GET_H264_DBLK _IOWR('i', E_MI_VENC_CMD_GET_H264_DBLK, MI_VENC_GetH264Dblk_t)
#define MI_VENC_SET_H264_VUI _IOW('i', E_MI_VENC_CMD_SET_H264_VUI, MI_VENC_SetH264Vui_t)
#define MI_VENC_GET_H264_VUI _IOWR('i', E_MI_VENC_CMD_GET_H264_VUI, MI_VENC_GetH264Vui_t)
#define MI_VENC_SET_H265_SLICE_SPLIT _IOW('i', E_MI_VENC_CMD_SET_H265_SLICE_SPLIT, MI_VENC_SetH265SliceSplit_t)
#define MI_VENC_GET_H265_SLICE_SPLIT _IOWR('i', E_MI_VENC_CMD_GET_H265_SLICE_SPLIT, MI_VENC_GetH265SliceSplit_t)
#define MI_VENC_SET_H265_INTER_PRED _IOW('i', E_MI_VENC_CMD_SET_H265_INTER_PRED, MI_VENC_SetH265InterPred_t)
#define MI_VENC_GET_H265_INTER_PRED _IOWR('i', E_MI_VENC_CMD_GET_H265_INTER_PRED, MI_VENC_GetH265InterPred_t)
#define MI_VENC_SET_H265_INTRA_PRED _IOW('i', E_MI_VENC_CMD_SET_H265_INTRA_PRED, MI_VENC_SetH265IntraPred_t)
#define MI_VENC_GET_H265_INTRA_PRED _IOWR('i', E_MI_VENC_CMD_GET_H265_INTRA_PRED, MI_VENC_GetH265IntraPred_t)
#define MI_VENC_SET_H265_TRANS _IOW('i', E_MI_VENC_CMD_SET_H265_TRANS, MI_VENC_SetH265Trans_t)
#define MI_VENC_GET_H265_TRANS _IOWR('i', E_MI_VENC_CMD_GET_H265_TRANS, MI_VENC_GetH265Trans_t)
#define MI_VENC_SET_H265_DBLK _IOW('i', E_MI_VENC_CMD_SET_H265_DBLK, MI_VENC_SetH265Dblk_t)
#define MI_VENC_GET_H265_DBLK _IOWR('i', E_MI_VENC_CMD_GET_H265_DBLK, MI_VENC_GetH265Dblk_t)
#define MI_VENC_SET_H265_VUI _IOW('i', E_MI_VENC_CMD_SET_H265_VUI, MI_VENC_SetH265Vui_t)
#define MI_VENC_GET_H265_VUI _IOWR('i', E_MI_VENC_CMD_GET_H265_VUI, MI_VENC_GetH265Vui_t)
#define MI_VENC_SET_JPEG_PARAM _IOW('i', E_MI_VENC_CMD_SET_JPEG_PARAM, MI_VENC_SetJpegParam_t)
#define MI_VENC_GET_JPEG_PARAM _IOWR('i', E_MI_VENC_CMD_GET_JPEG_PARAM, MI_VENC_GetJpegParam_t)
#define MI_VENC_SET_RC_PARAM _IOW('i', E_MI_VENC_CMD_SET_RC_PARAM, MI_VENC_SetRcParam_t)
#define MI_VENC_GET_RC_PARAM _IOWR('i', E_MI_VENC_CMD_GET_RC_PARAM, MI_VENC_GetRcParam_t)
#define MI_VENC_SET_REF_PARAM _IOW('i', E_MI_VENC_CMD_SET_REF_PARAM, MI_VENC_SetRefParam_t)
#define MI_VENC_GET_REF_PARAM _IOWR('i', E_MI_VENC_CMD_GET_REF_PARAM, MI_VENC_GetRefParam_t)
#define MI_VENC_SET_CROP _IOW('i', E_MI_VENC_CMD_SET_CROP, MI_VENC_SetCrop_t)
#define MI_VENC_SET_CROP _IOW('i', E_MI_VENC_CMD_SET_CROP, MI_VENC_SetCrop_t)
#define MI_VENC_SET_FRAME_LOST_STRATEGY _IOW('i', E_MI_VENC_CMD_SET_FRAME_LOST_STRATEGY, MI_VENC_SetFrameLostStrategy_t)
#define MI_VENC_GET_FRAME_LOST_STRATEGY _IOWR('i', E_MI_VENC_CMD_GET_FRAME_LOST_STRATEGY, MI_VENC_GetFrameLostStrategy_t)
#define MI_VENC_SET_SUPER_FRAME_CFG _IOW('i', E_MI_VENC_CMD_SET_SUPER_FRAME_CFG, MI_VENC_SetSuperFrameCfg_t)
#define MI_VENC_GET_SUPER_FRAME_CFG _IOWR('i', E_MI_VENC_CMD_GET_SUPER_FRAME_CFG, MI_VENC_GetSuperFrameCfg_t)
#define MI_VENC_SET_RC_PRIORITY _IOW('i', E_MI_VENC_CMD_SET_RC_PRIORITY, MI_VENC_SetRcPriority_t)
#define MI_VENC_GET_RC_PRIORITY _IOWR('i', E_MI_VENC_CMD_GET_RC_PRIORITY, MI_VENC_GetRcPriority_t)

#define MI_VENC_GET_CHN_DEVID _IOWR('i', E_MI_VENC_CMD_GET_CHN_DEVID, MI_VENC_GetChnDevid_t)

#endif /// _VENC_IOCTL_H_
