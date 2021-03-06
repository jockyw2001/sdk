#ifndef MI_VENC_MHAL_MHE_H_
#define MI_VENC_MHAL_MHE_H_


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

#include "mhal_venc.h"

typedef enum
{
    E_MHAL_MHE_CORE0 = 0,
    E_MHAL_MHE_CORE1,
} MHAL_MHE_DeviceId_e;

//------------------------------------------------------------------------------
/// @brief MHE Hardware Initial
/// @param[in]  void.
/// @param[out] void.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_Init(void);

//------------------------------------------------------------------------------
/// @brief Get Device Handle
/// @param[in]  u32DevId: Device Core ID.
/// @param[out] phDev: Pointer to device handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_CreateDevice(MS_U32 u32DevId, MHAL_VENC_DEV_HANDLE *phDev);

//------------------------------------------------------------------------------
/// @brief Destroy Device Handle
/// @param[in]  hDev: Device handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_DestroyDevice(MHAL_VENC_DEV_HANDLE hDev);

//------------------------------------------------------------------------------
/// @brief Get Parameters of Device
/// @param[in]  hDev: Device handle.
/// @param[in]  eType: Parameter type.
/// @param[out] pstParam: Pointer to parameter.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_GetDevConfig(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam);

//------------------------------------------------------------------------------
/// @brief Get Instance Handle
/// @param[in]  hDev: Device handle.
/// @param[out] phInst: Pointer to instance handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_CreateInstance(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_INST_HANDLE *phInst);

//------------------------------------------------------------------------------
/// @brief Destroy Instance Handle
/// @param[in]  hInst: Pointer to instance handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_DestroyInstance(MHAL_VENC_INST_HANDLE hInst);

//------------------------------------------------------------------------------
/// @brief Set Parameters of Instance
/// @param[in]  hInst: Instance handle.
/// @param[in]  eType: Parameter type.
/// @param[in]  pstParam: Pointer to parameter.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_SetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam);

//------------------------------------------------------------------------------
/// @brief Get Parameters of Instance
/// @param[in]  hInst: Instance handle.
/// @param[in]  eType: Parameter type.
/// @param[out] pstParam: Pointer to parameter.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_GetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam);

//------------------------------------------------------------------------------
/// @brief Trigger Encoder
/// @param[in]  hInst: Instance handle.
/// @param[in]  pstInOutBuf: Pointer to buffer information.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_EncodeOneFrame(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pstInOutBuf);

//------------------------------------------------------------------------------
/// @brief Post Process after Encode Frame Done
/// @param[in]  hInst: Instance handle.
/// @param[out] pstEncRet: Pointer to encode result.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_EncodeFrameDone(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_EncResult_t* pstEncRet);

//------------------------------------------------------------------------------
/// @brief Query Internal Buffer Size Requirement of Encoder
/// @param[in]  hInst: Instance handle.
/// @param[out] pstParam: Pointer to internal buffer information.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_QueryBufSize(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InternalBuf_t *pstParam);

//------------------------------------------------------------------------------
/// @brief Device Interrupt Processing
/// @param[in]  hDev: Device handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_MHE_IsrProc(MHAL_VENC_DEV_HANDLE hDev);


#endif /* MI_VENC_MHAL_MHE_H_ */
