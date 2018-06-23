//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2006 - 2015 MStar Semiconductor, Inc. All rights reserved.
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
// Copyright (c) 2006-2015 MStar Semiconductor, Inc.
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

#ifndef _MI_VENC_INTERNAL_H_
#define _MI_VENC_INTERNAL_H_

//==== Porting ====

/* This is designed to be removed in the end.
 * Because the /project/ for CamOs K6L is not ready in Alkaid, temporarily
 * define this wrapper in VENC module to be Linux native or Cam OS wrappers.
 */
#define USE_CAM_OS (0)


#define VENC_FPGA (0)
#define VENC_BOARD (2)//2: i2, 3: i3, 6:k6l

//a dummy return value to produce IRQ
#define MS_VENC_DUMMY_IRQ (-101)


#ifdef VENC_PARAM
#define VENC_PARAM_AUTR 0644 //(S_IRUGO|S_IWUSR)
VENC_PARAM(reencode_dqf,     int, 4);  //delta Q factor while re-encode JPEG
VENC_PARAM(reencode_dqp,     int, 4);  //delta QP offset while re-encode H264/H265
VENC_PARAM(reencode_max_cnt, int, 4);  //max number of re-encoding
VENC_PARAM(discard_dqf,     int, 4);  //delta Q factor while re-encode JPEG
VENC_PARAM(discard_dqp,     int, 4);  //delta QP offset while re-encode H264/H265
VENC_PARAM(discard_max_cnt, int, 4);  //max number of re-encoding
VENC_PARAM(frm_lost_win, int, 6);  //Use this number of P frames as P frame lost threshold
#endif

//This enumeration is chip-dependent
typedef enum
{
    E_MI_VENC_DEV_MHE0, //support CMD_Q
    E_MI_VENC_DEV_MHE1, //support CMD_Q
    E_MI_VENC_DEV_MFE0, //support CMD_Q
    E_MI_VENC_DEV_MFE1, //MFE1 does not support CMD_Q
    E_MI_VENC_DEV_JPEG, //with CPU
#if CONNECT_DUMMY_HAL
    E_MI_VENC_DEV_DUMMY,//dummy HAL without CMD_Q support
#endif
    E_MI_VENC_DEV_MAX  //number of devices
} MI_VENC_Dev_e;

#if defined(__KERNEL__)
    #include <linux/irqreturn.h>
    typedef irqreturn_t MI_VENC_Irqreturn_t;
#else
    typedef int MI_VENC_Irqreturn_t;
#endif
typedef MI_VENC_Irqreturn_t (*MI_VENC_PFN_IRQ)(int irq, void* data);
MI_S32 MI_VENC_RequestIrq(MI_U32 u32IrqNum, MI_VENC_PFN_IRQ pfnIsr, char *szName, void* pUserData);
MI_S32 MI_VENC_FreeIrq(MI_U32 u32IrqNum, void* pUserData);

typedef struct MI_VENC_DevRes_s MI_VENC_DevRes_t;
typedef struct MI_VENC_ChnRes_s MI_VENC_ChnRes_t;
MS_S32 MHAL_VENC_InitDevice(MI_VENC_Dev_e eDevType);
MS_S32 MHAL_VENC_CreateDevice(MI_VENC_DevRes_t *pstDevRes, MI_VENC_Dev_e eDevType, void *pOsDev, void** ppBase,
        int *pSize, MHAL_VENC_DEV_HANDLE *phDev);
MS_S32 MHAL_VENC_DestroyDevice(MI_VENC_DevRes_t *pstDevRes);

MS_S32 MHAL_VENC_GetDevConfig(MI_VENC_DevRes_t *pstDevRes, MHAL_VENC_IDX eIdx, MHAL_VENC_Param_t* pstParam);
MS_S32 MHAL_VENC_CreateInstance(MI_VENC_DevRes_t *pstDevRes, MI_VENC_ChnRes_t *pstChnRes);
MS_S32 MHAL_VENC_DestroyInstance(MI_VENC_ChnRes_t *pstChnRes);
MS_S32 MHAL_VENC_QueryBufSize(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_InternalBuf_t *pstSize);
MS_S32 MHAL_VENC_EncDone(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_EncResult_t* pstEncRet);
MS_S32 MHAL_VENC_EncodeOneFrame(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_InOutBuf_t* pInOutBuf);
MS_S32 MHAL_VENC_EncodeCancel(MI_VENC_ChnRes_t *pstChnRes);
MS_S32 MHAL_VENC_IsrProc(MI_VENC_DevRes_t *pstDevRes);
MS_S32 MHAL_VENC_GetEngStatus(MI_VENC_DevRes_t *pstDevRes, MHAL_VENC_EncStatus_t *pstEncStatus);
MS_S32 MHAL_VENC_SetParam(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam);
MS_S32 MHAL_VENC_GetParam(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam);
#endif
