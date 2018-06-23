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


#ifndef _DISP_IOCTL_H_
#define _DISP_IOCTL_H_
#include <linux/ioctl.h>
#include "mi_disp.h"

typedef enum
{
    E_MI_DISP_CMD_ENABLE,
    E_MI_DISP_CMD_DISABLE,
    E_MI_DISP_CMD_SET_PUB_ATTR,
    E_MI_DISP_CMD_GET_PUB_ATTR,
    E_MI_DISP_CMD_DEVICE_ATTACH,
    E_MI_DISP_CMD_DEVICE_DETACH,
    E_MI_DISP_CMD_ENABLE_VIDEO_LAYER,
    E_MI_DISP_CMD_DISABLE_VIDEO_LAYER,
    E_MI_DISP_CMD_SET_VIDEO_LAYER_ATTR,
    E_MI_DISP_CMD_GET_VIDEO_LAYER_ATTR,
    E_MI_DISP_CMD_BIND_VIDEO_LAYER,
    E_MI_DISP_CMD_UN_BIND_VIDEO_LAYER,

    E_MI_DISP_CMD_GET_VIDEO_LAYER_PRIORITY,
    E_MI_DISP_CMD_SET_PLAY_TOLERATION,
    E_MI_DISP_CMD_GET_PLAY_TOLERATION,
    E_MI_DISP_CMD_GET_OUTPUTTIMING,
    E_MI_DISP_CMD_GET_SCREEN_FRAME,
    E_MI_DISP_CMD_RELEASE_SCREEN_FRAME,
    E_MI_DISP_CMD_SET_VIDEO_LAYER_ATTR_BEGIN,
    E_MI_DISP_CMD_SET_VIDEO_LAYER_ATTR_END,
    E_MI_DISP_CMD_SET_INPUT_PORT_ATTR,
    E_MI_DISP_CMD_GET_INPUT_PORT_ATTR,
    E_MI_DISP_CMD_ENABLE_INPUT_PORT,
    E_MI_DISP_CMD_DISABLE_INPUT_PORT,
    E_MI_DISP_CMD_SET_INPUT_PORT_DISP_POS,
    E_MI_DISP_CMD_GET_INPUT_PORT_DISP_POS,
    E_MI_DISP_CMD_PAUSE_INPUT_PORT,
    E_MI_DISP_CMD_RESUME_INPUT_PORT,
    E_MI_DISP_CMD_STEP_INPUT_PORT,
    E_MI_DISP_CMD_SHOW_INPUT_PORT,
    E_MI_DISP_CMD_HIDE_INPUT_PORT,
    E_MI_DISP_CMD_SET_INPUT_PORT_SYNC_MODE,
    E_MI_DISP_CMD_QUERY_INPUT_PORT_STAT,
    E_MI_DISP_CMD_SET_ZOOM_WINDOW,
    E_MI_DISP_CMD_GET_VGA_PARAM,
    E_MI_DISP_CMD_SET_VGA_PARAM,
    E_MI_DISP_CMD_GET_HDMI_PARAM,
    E_MI_DISP_CMD_SET_HDMI_PARAM,
    E_MI_DISP_CMD_GET_CVBS_PARAM,
    E_MI_DISP_CMD_SET_CVBS_PARAM,
    E_MI_DISP_CMD_CLEAR_INPORT_BUFFER,
    E_MI_DISP_CMD_MAX,
} MI_DISP_Cmd_e;


typedef struct MI_DISP_SetPubAttr_s
{
    MI_DISP_DEV DispDev;
    MI_DISP_PubAttr_t stPubAttr;
} MI_DISP_SetPubAttr_t;

typedef struct MI_DISP_GetPubAttr_s
{
    MI_DISP_DEV DispDev;
    MI_DISP_PubAttr_t stPubAttr;
} MI_DISP_GetPubAttr_t;

typedef struct MI_DISP_DeviceAttach_s
{
    MI_DISP_DEV DispSrcDev;
    MI_DISP_DEV DispDstDev;
} MI_DISP_DeviceAttach_t;

typedef struct MI_DISP_DeviceDetach_s
{
    MI_DISP_DEV DispSrcDev;
    MI_DISP_DEV DispDstDev;
} MI_DISP_DeviceDetach_t;

typedef struct MI_DISP_SetVideoLayerCompressAttr_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_CompressAttr_t stCompressAttr;
} MI_DISP_SetVideoLayerCompressAttr_t;

typedef struct MI_DISP_GetVideoLayerCompressAttr_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_CompressAttr_t stCompressAttr;
} MI_DISP_GetVideoLayerCompressAttr_t;

typedef struct MI_DISP_SetVideoLayerAttr_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
} MI_DISP_SetVideoLayerAttr_t;

typedef struct MI_DISP_GetVideoLayerAttr_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
} MI_DISP_GetVideoLayerAttr_t;

typedef struct MI_DISP_BindVideoLayer_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_DEV DispDev;
} MI_DISP_BindVideoLayer_t;

typedef struct MI_DISP_UnBindVideoLayer_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_DEV DispDev;
} MI_DISP_UnBindVideoLayer_t;

typedef struct MI_DISP_SetVideoLayerPriority_s
{
    MI_DISP_LAYER DispLayer;
    MI_U32 u32Priority;
} MI_DISP_SetVideoLayerPriority_t;

typedef struct MI_DISP_GetVideoLayerPriority_s
{
    MI_DISP_LAYER DispLayer;
    MI_U32 u32Priority;
} MI_DISP_GetVideoLayerPriority_t;

typedef struct MI_DISP_SetPlayToleration_s
{
    MI_DISP_LAYER DispLayer;
    MI_U32 u32Toleration;
} MI_DISP_SetPlayToleration_t;

typedef struct MI_DISP_GetPlayToleration_s
{
    MI_DISP_LAYER DispLayer;
    MI_U32 u32Toleration;
} MI_DISP_GetPlayToleration_t;

typedef struct MI_DISP_GetOutputTiming_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_OutputTiming_e eOutputTiming;
    MI_DISP_SyncInfo_t       stSyncInfo;          /* Information about VO interface timings */
} MI_DISP_GetOutputTiming_t;

typedef struct MI_DISP_GetScreenFrame_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_VideoFrame_t stVFrame;
    //MI_U32  u32MilliSec;
} MI_DISP_GetScreenFrame_t;

typedef struct MI_DISP_ReleaseScreenFrame_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_VideoFrame_t stVFrame;
} MI_DISP_ReleaseScreenFrame_t;

typedef struct MI_DISP_SetInputPortAttr_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
    MI_DISP_InputPortAttr_t stInputPortAttr;
} MI_DISP_SetInputPortAttr_t;

typedef struct MI_DISP_GetInputPortAttr_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
    MI_DISP_InputPortAttr_t stInputPortAttr;
} MI_DISP_GetInputPortAttr_t;

typedef struct MI_DISP_EnableInputPort_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
} MI_DISP_EnableInputPort_t;

typedef struct MI_DISP_DisableInputPort_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
} MI_DISP_DisableInputPort_t;

typedef struct MI_DISP_SetInputPortDispPos_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
    MI_DISP_Position_t stDispPos;
} MI_DISP_SetInputPortDispPos_t;

typedef struct MI_DISP_GetInputPortDispPos_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
    MI_DISP_Position_t stDispPos;
} MI_DISP_GetInputPortDispPos_t;

typedef struct MI_DISP_PauseInputPort_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
} MI_DISP_PauseInputPort_t;

typedef struct MI_DISP_ResumeInputPort_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
} MI_DISP_ResumeInputPort_t;

typedef struct MI_DISP_StepInputPort_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
} MI_DISP_StepInputPort_t;

typedef struct MI_DISP_ShowInputPort_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
} MI_DISP_ShowInputPort_t;

typedef struct MI_DISP_HideInputPort_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
} MI_DISP_HideInputPort_t;

typedef struct MI_DISP_FrameFlip_s
{
    MI_PHY phyAddr;
	MI_U32 u32Width;
	MI_U32 u32Height;
} MI_DISP_FrameFlip_t;


typedef struct MI_DISP_SetInputPortSyncMode_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
    MI_DISP_SyncMode_e eMode;
} MI_DISP_SetInputPortSyncMode_t;

typedef struct MI_DISP_QueryInputPortStat_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
    MI_DISP_QueryChannelStatus_t stStatus;
} MI_DISP_QueryInputPortStat_t;

typedef struct MI_DISP_SetZoomInWindow_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
    MI_DISP_VidWinRect_t stCropWin;
} MI_DISP_SetZoomInWindow_t;

typedef struct MI_DISP_GetVgaParam_s
{
    MI_DISP_DEV DispDev;
    MI_DISP_VgaParam_t stVgaParam;
} MI_DISP_GetVgaParam_t;

typedef struct MI_DISP_SetVgaParam_s
{
    MI_DISP_DEV DispDev;
    MI_DISP_VgaParam_t stVgaParam;
} MI_DISP_SetVgaParam_t;

typedef struct MI_DISP_GetHdmiParam_s
{
    MI_DISP_DEV DispDev;
    MI_DISP_HdmiParam_t stHdmiParam;
} MI_DISP_GetHdmiParam_t;

typedef struct MI_DISP_SetHdmiParam_s
{
    MI_DISP_DEV DispDev;
    MI_DISP_HdmiParam_t stHdmiParam;
} MI_DISP_SetHdmiParam_t;

typedef struct MI_DISP_GetCvbsParam_s
{
    MI_DISP_DEV DispDev;
    MI_DISP_CvbsParam_t stCvbsParam;
} MI_DISP_GetCvbsParam_t;

typedef struct MI_DISP_SetCvbsParam_s
{
    MI_DISP_DEV DispDev;
    MI_DISP_CvbsParam_t stCvbsParam;
} MI_DISP_SetCvbsParam_t;

typedef struct MI_DISP_ClearInportBuffParam_s
{
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
    MI_BOOL bClrAll;
} MI_DISP_ClearInportBuff_t;

#define MI_DISP_ENABLE _IOW('i', E_MI_DISP_CMD_ENABLE, MI_DISP_DEV)
#define MI_DISP_DISABLE _IOW('i', E_MI_DISP_CMD_DISABLE, MI_DISP_DEV)
#define MI_DISP_SET_PUB_ATTR _IOW('i', E_MI_DISP_CMD_SET_PUB_ATTR, MI_DISP_SetPubAttr_t)
#define MI_DISP_GET_PUB_ATTR _IOWR('i', E_MI_DISP_CMD_GET_PUB_ATTR, MI_DISP_GetPubAttr_t)
#define MI_DISP_DEVICE_ATTACH _IOW('i', E_MI_DISP_CMD_DEVICE_ATTACH, MI_DISP_DeviceAttach_t)
#define MI_DISP_DEVICE_DETACH _IOW('i', E_MI_DISP_CMD_DEVICE_DETACH, MI_DISP_DeviceDetach_t)
#define MI_DISP_ENABLE_VIDEO_LAYER _IOW('i', E_MI_DISP_CMD_ENABLE_VIDEO_LAYER, MI_DISP_LAYER)
#define MI_DISP_DISABLE_VIDEO_LAYER _IOW('i', E_MI_DISP_CMD_DISABLE_VIDEO_LAYER, MI_DISP_LAYER)
#define MI_DISP_SET_VIDEO_LAYER_ATTR _IOW('i', E_MI_DISP_CMD_SET_VIDEO_LAYER_ATTR, MI_DISP_SetVideoLayerAttr_t)
#define MI_DISP_GET_VIDEO_LAYER_ATTR _IOWR('i', E_MI_DISP_CMD_GET_VIDEO_LAYER_ATTR, MI_DISP_GetVideoLayerAttr_t)
#define MI_DISP_BIND_VIDEO_LAYER _IOW('i', E_MI_DISP_CMD_BIND_VIDEO_LAYER, MI_DISP_BindVideoLayer_t)
#define MI_DISP_UN_BIND_VIDEO_LAYER _IOW('i', E_MI_DISP_CMD_UN_BIND_VIDEO_LAYER, MI_DISP_UnBindVideoLayer_t)
#define MI_DISP_SET_PLAY_TOLERATION _IOW('i', E_MI_DISP_CMD_SET_PLAY_TOLERATION, MI_DISP_SetPlayToleration_t)
#define MI_DISP_GET_PLAY_TOLERATION _IOWR('i', E_MI_DISP_CMD_GET_PLAY_TOLERATION, MI_DISP_GetPlayToleration_t)
#define MI_DISP_GET_OUTPUTTIMING _IOWR('i', E_MI_DISP_CMD_GET_OUTPUTTIMING, MI_DISP_GetOutputTiming_t)
#define MI_DISP_GET_SCREEN_FRAME _IOWR('i', E_MI_DISP_CMD_GET_SCREEN_FRAME, MI_DISP_GetScreenFrame_t)
#define MI_DISP_RELEASE_SCREEN_FRAME _IOW('i', E_MI_DISP_CMD_RELEASE_SCREEN_FRAME, MI_DISP_ReleaseScreenFrame_t)
#define MI_DISP_SET_VIDEO_LAYER_ATTR_BEGIN _IOW('i', E_MI_DISP_CMD_SET_VIDEO_LAYER_ATTR_BEGIN, MI_DISP_LAYER)
#define MI_DISP_SET_VIDEO_LAYER_ATTR_END _IOW('i', E_MI_DISP_CMD_SET_VIDEO_LAYER_ATTR_END, MI_DISP_LAYER)
#define MI_DISP_SET_INPUT_PORT_ATTR _IOW('i', E_MI_DISP_CMD_SET_INPUT_PORT_ATTR, MI_DISP_SetInputPortAttr_t)
#define MI_DISP_GET_INPUT_PORT_ATTR _IOWR('i', E_MI_DISP_CMD_GET_INPUT_PORT_ATTR, MI_DISP_GetInputPortAttr_t)
#define MI_DISP_ENABLE_INPUT_PORT _IOW('i', E_MI_DISP_CMD_ENABLE_INPUT_PORT, MI_DISP_EnableInputPort_t)
#define MI_DISP_DISABLE_INPUT_PORT _IOW('i', E_MI_DISP_CMD_DISABLE_INPUT_PORT, MI_DISP_DisableInputPort_t)
#define MI_DISP_SET_INPUT_PORT_DISP_POS _IOW('i', E_MI_DISP_CMD_SET_INPUT_PORT_DISP_POS, MI_DISP_SetInputPortDispPos_t)
#define MI_DISP_GET_INPUT_PORT_DISP_POS _IOWR('i', E_MI_DISP_CMD_GET_INPUT_PORT_DISP_POS, MI_DISP_GetInputPortDispPos_t)
#define MI_DISP_PAUSE_INPUT_PORT _IOW('i', E_MI_DISP_CMD_PAUSE_INPUT_PORT, MI_DISP_PauseInputPort_t)
#define MI_DISP_RESUME_INPUT_PORT _IOW('i', E_MI_DISP_CMD_RESUME_INPUT_PORT, MI_DISP_ResumeInputPort_t)
#define MI_DISP_STEP_INPUT_PORT _IOW('i', E_MI_DISP_CMD_STEP_INPUT_PORT, MI_DISP_StepInputPort_t)
#define MI_DISP_SHOW_INPUT_PORT _IOW('i', E_MI_DISP_CMD_SHOW_INPUT_PORT, MI_DISP_ShowInputPort_t)
#define MI_DISP_HIDE_INPUT_PORT _IOW('i', E_MI_DISP_CMD_HIDE_INPUT_PORT, MI_DISP_HideInputPort_t)
#define MI_DISP_SET_INPUT_PORT_SYNC_MODE _IOW('i', E_MI_DISP_CMD_SET_INPUT_PORT_SYNC_MODE, MI_DISP_SetInputPortSyncMode_t)
#define MI_DISP_QUERY_INPUT_PORT_STAT _IOW('i', E_MI_DISP_CMD_QUERY_INPUT_PORT_STAT, MI_DISP_QueryInputPortStat_t)
#define MI_DISP_SET_ZOOM_WINDOW_STAT _IOW('i', E_MI_DISP_CMD_SET_ZOOM_WINDOW, MI_DISP_SetZoomInWindow_t)
#define MI_DISP_GET_VGA_PARAM _IOWR('i', E_MI_DISP_CMD_GET_VGA_PARAM, MI_DISP_GetVgaParam_t)
#define MI_DISP_SET_VGA_PARAM _IOW('i', E_MI_DISP_CMD_SET_VGA_PARAM, MI_DISP_SetVgaParam_t)
#define MI_DISP_GET_HDMI_PARAM _IOWR('i', E_MI_DISP_CMD_GET_HDMI_PARAM, MI_DISP_GetHdmiParam_t)
#define MI_DISP_SET_HDMI_PARAM _IOW('i', E_MI_DISP_CMD_SET_HDMI_PARAM, MI_DISP_SetHdmiParam_t)
#define MI_DISP_GET_CVBS_PARAM _IOWR('i', E_MI_DISP_CMD_GET_CVBS_PARAM, MI_DISP_GetCvbsParam_t)
#define MI_DISP_SET_CVBS_PARAM _IOW('i', E_MI_DISP_CMD_SET_CVBS_PARAM, MI_DISP_SetCvbsParam_t)
#define MI_DISP_CLEAR_INPORT_BUFFER _IOW('i', E_MI_DISP_CMD_CLEAR_INPORT_BUFFER, MI_DISP_ClearInportBuff_t)

#endif /// _DISP_IOCTL_H_
