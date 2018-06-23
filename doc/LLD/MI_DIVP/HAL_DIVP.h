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


//------------------------------------------------------------------------------
/// @brief Init DIVP Hardware
/// @param[in] eDevId: DIVP device ID.
/// @return TRUE: init HW success.
/// @return FALSE: init HW failed.
//------------------------------------------------------------------------------
bool HalDivpInit(HalDivpDeviceId_e eDevId);

//------------------------------------------------------------------------------
/// @brief deInit DIVP Hardware
/// @param[in] eDevId: DIVP device ID.
/// @return TRUE: deInit HW success.
/// @return FALSE: deInit HW failed.
//------------------------------------------------------------------------------
bool HalDivpDeInit(HalDivpDeviceId_e eDevId);

//------------------------------------------------------------------------------
/// @brief Open Video handler
/// @param[in] eDevId: DIVP device ID.
/// @param[in] pstAlloc: function pointer to allocate memory.
/// @param[in] pfFree: function pointer to free memory.
/// @param[in] u16MaxWidth: max width channel supportrd.
/// @param[in] u16MaxHeight: max height channel supportrd.
/// @param[out] pCtx: DIVP channel contex in hal and down layer.
/// @return TRUE: create DIVP channel inistance success.
/// @return FALSE: create DIVP channel inistance failed
//------------------------------------------------------------------------------
bool HalDivpCreateInstance(HalDivpDeviceId_e eDevId, u16 nMaxWidth,
                             u16 nMaxHeight, const PfnAlloc* pfAlloc, const PfnFree* pfFree, void** ppCtx);

//------------------------------------------------------------------------------
/// @brief destroy DIVP channel inistance
/// @param[in] pCtx: DIVP channel contex in hal and down layer.
/// @return TRUE: destroy inistance success.
/// @return FALSE: destroy inistance failed
//------------------------------------------------------------------------------
bool HalDivpDestroyInstance(void* pCtx);

//------------------------------------------------------------------------------
/// @attention: Disable TNR, 3D DI and crop when capture a picture.
/// @brief capture a picture from display VOP/OP2.
/// @param[in] pstCaptureInfo: config information of the picture captured by DIVP.
/// @param[in] pstCmdQInfo: a group of function pointer for controlling HW register. These functions
///    will deside write/read regster with RIU or Command queue.
/// @return TRUE: capture a picture success.
/// @return FALSE: capture a picture failed
//------------------------------------------------------------------------------
bool HalDivpCaptureTiming(HalDivpCaptureInfo_t* pstCaptureInfo,
                                                                                 cmd_mload_interface* pstCmdInf);

//------------------------------------------------------------------------------
/// @brief DIVP deal with a frame and than write to dram. The frame will be used by VPE...
/// @param[in] pCtx: DIVP channel contex in hal and down layer.
/// @param[in] pstDivpInputInfo: config information of the input frame.
/// @param[in] pstDivpOutputInfo: config information of the output frame.
/// @param[in] pstCmdQInfo: a group of function pointer for controlling HW register. These functions
///    will deside write/read regster with RIU or Command queue.
/// @return TRUE: DIVP deal with frame success.
/// @return FALSE: DIVP deal with frame failed
//------------------------------------------------------------------------------
bool HalDivpProcessDramData(void* pCtx, HalDivpInputInfo_t* pstDivpInputInfo,
                             HalDivpOutPutInfo_t* pstDivpOutputInfo, cmd_mload_interface* pstCmdInf);


//------------------------------------------------------------------------------
/// @brief set attribute of DIVP channel.
/// @param[in] pCtx: DIVP channel contex in hal and down layer.
/// @param[in] eAttrType: DIVP channel attribute type.
/// @param[in] pAttr: config information of DIVP channel.
///    if(HAL_DIVP_ATTR_TNR == eAttrType) ==> pAttr is HalDivpTnrLevel_e*.
///    if(HAL_DIVP_ATTR_DI == eAttrType) ==> pAttr is HalDivpDiType_e*.
///    if(HAL_DIVP_ATTR_ROTATE == eAttrType) ==> pAttr is HalDivpRotate_e*.
///    if(HAL_DIVP_ATTR_CROPWIN == eAttrType) ==> pAttr is HalDivpWindow_t*.
///    if(HAL_DIVP_ATTR_MIRROR == eAttrType) ==> pAttr is HalDivpMirror_t*.
/// @param[in] pstCmdQInfo: a group of function pointer for controlling HW register. These functions
///    will deside write/read regster with RIU or Command queue.
/// @return TRUE: DIVP deal with frame success.
/// @return FALSE: DIVP deal with frame failed
//------------------------------------------------------------------------------
bool HalDivpSetAttr(void* pCtx, HalDivpAttrType_e eAttrType, const void* pAttr,
                                                                                       cmd_mload_interface* pstCmdInf);

//------------------------------------------------------------------------------
/// @brief DIVP deal with a frame and than write to dram. The frame will be used by DISP.
/// @param[in] bEnable: control DIVP module's ISR of frame done. TURE: accept interrupt,
///    FALSE:ignore frame done ISR.
/// @return TRUE: DIVP deal with frame success.
/// @return FALSE: DIVP deal with frame failed
//------------------------------------------------------------------------------
bool HalDivpEnableFrameDoneIsr(MI_BOOL bEnable);

//------------------------------------------------------------------------------
/// @brief reset frame done interrupt..
/// @return TRUE: DIVP deal with frame success.
/// @return FALSE: DIVP deal with frame failed
//------------------------------------------------------------------------------
bool HalDivpCleanFrameDoneIsr(void);
