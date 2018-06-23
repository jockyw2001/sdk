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

#ifndef _MI_VDISP_DATATYPE_H_
#define _MI_VDISP_DATATYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VDISP_MAX_DEVICE_NUM 4
#define VDISP_MAX_CHN_NUM 1
#define VDISP_MAX_INPUTPORT_NUM 32
#define VDISP_MAX_OVERLAYINPUTPORT_NUM 1
#define VDISP_OVERLAYINPUTPORTID VDISP_MAX_INPUTPORT_NUM
#define VDISP_MAX_OUTPUTPORT_NUM 1

typedef MI_S32 MI_VDISP_DEV;
typedef MI_S32 MI_VDISP_PORT;

typedef struct MI_VDISP_OutputPortAttr_s
{
    MI_U32 u32BgColor;          /* Background color of a output port, in YUV format. [23:16]:v, [15:8]:y, [7:0]:u*/
    MI_SYS_PixelFormat_e ePixelFormat; /* pixel format of a output port */
    MI_U64 u64pts; /* current PTS */
    MI_U32 u32FrmRate; /* the frame rate of output port */
    MI_U32 u32Width; /* the frame width of a output port */
    MI_U32 u32Height; /* the frame height of a output port */
} MI_VDISP_OutputPortAttr_t;

typedef struct MI_VDISP_InputPortAttr_s
{
    MI_U32 u32OutX; /* the output frame X position of this input port */
    MI_U32 u32OutY; /* the output frame Y position of this input port */
    MI_U32 u32OutWidth; /* the output frame width of this input port */
    MI_U32 u32OutHeight; /* the output frame height of this input port */
    MI_S32 s32IsFreeRun; /* is this port free run */
} MI_VDISP_InputPortAttr_t;

typedef enum
{
    E_MI_VDISP_ERR_DEV_OPENED=MI_VDISP_INITIAL_ERROR_CODE,
    E_MI_VDISP_ERR_DEV_NOT_OPEN,
    E_MI_VDISP_ERR_DEV_NOT_CLOSE,
    E_MI_VDISP_ERR_PORT_NOT_UNBIND,
}MI_VDISP_ErrCode_e;

#define MI_VDISP_ERR_FAIL (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_FAILED))
#define MI_VDISP_ERR_INVALID_DEVID (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID))
#define MI_VDISP_ERR_ILLEGAL_PARAM (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM))
#define MI_VDISP_ERR_NOT_SUPPORT (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT))
#define MI_VDISP_ERR_MOD_INITED (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INITED))
#define MI_VDISP_ERR_MOD_NOT_INIT (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_INIT))
#define MI_VDISP_ERR_DEV_OPENED (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_VDISP_ERR_DEV_OPENED))
#define MI_VDISP_ERR_DEV_NOT_OPEN (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_VDISP_ERR_DEV_NOT_OPEN))
#define MI_VDISP_ERR_DEV_NOT_STOP (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DEV_NOT_STOPED))
#define MI_VDISP_ERR_DEV_NOT_CLOSE (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_VDISP_ERR_DEV_NOT_CLOSE))
#define MI_VDISP_ERR_NOT_CONFIG (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG))
#define MI_VDISP_ERR_PORT_NOT_DISABLE (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_DISABLE))
#define MI_VDISP_ERR_PORT_NOT_UNBIND (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_VDISP_ERR_PORT_NOT_UNBIND))

#ifdef __cplusplus
}
#endif

#endif///_MI_VDISP_H_
