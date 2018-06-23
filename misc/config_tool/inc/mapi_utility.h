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
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
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

#ifndef __MAPI_UTILITY_H__
#define __MAPI_UTILITY_H__


// headers of standard C libs
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include "mi_common.h"
// headers of the same layer's
#include "iniparser.h"

// headers of underlying layer's

/// the helper utility class for ini file parsing and reading
        //-------------------------------------------------------------------------------------------------
        /// constructor
        /// @param s    \b IN: config path
        /// @return  None
        //-------------------------------------------------------------------------------------------------
        dictionary *mapi_config_helper(const char* s);
        //-------------------------------------------------------------------------------------------------
        /// Get U32 value from ini
        /// @param s          \b IN: ini attribute name
        /// @param  pu32Num         \b OUT: U32 value get from ini
        /// @param  notfound         \b IN: *pu32Num will be notfound if read error or not found
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL   mapi_config_helper_GetU32(dictionary *ini, const char* s, MI_U32 *pu32Num, MI_U32 notfound);
        //-------------------------------------------------------------------------------------------------
        /// Get BOOL value from ini
        /// @param s          \b IN: ini attribute name
        /// @param  pu8Bool         \b OUT: BOOL value get from ini
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL    mapi_config_helper_GetBOOL(dictionary *ini, const char* s, MI_BOOL *pu8Bool);
        //-------------------------------------------------------------------------------------------------
        /// Get char string from ini, the string is created by strdup, so user must free it if no more use
        /// @param s          \b IN: ini attribute name
        /// @param  ppu8String         \b OUT:  get char string from ini
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL  mapi_config_helper_GetString(dictionary *ini, const char* s, MI_U8 **ppu8String);
        //-------------------------------------------------------------------------------------------------
        /// Get Each Items in Array from ini
        /// @param s          \b IN: ini attribute name
        /// @param  thelist         \b OUT: list<string> that contain each row in the array.
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL  mapi_config_helper_GetArrayItems(dictionary *ini, const char* s,const char **arr, int *len);
        //-------------------------------------------------------------------------------------------------
        /// Get U8 value from ini
        /// @param s          \b IN: ini attribute name
        /// @param  u8Num         \b OUT: U8 value get from ini
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL mapi_config_helper_GetU8(dictionary *ini, const char* s, MI_U8 *u8Num);
        //-------------------------------------------------------------------------------------------------
        /// Get 1-Dimensional value from ini, the array must malloc by user then this API will fill the value in it
        /// @param s          \b IN: ini attribute name
        /// @param  pu8Array         \b OUT: u8 Array get from ini
        /// @param  size         \b OUT: Array Length
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL mapi_config_helper_Get1DArray(dictionary *ini, const char* s, MI_U8 * pu8Array,int size);
        //-------------------------------------------------------------------------------------------------
        /// Get 2-Dimensional value from ini, the array must malloc by user then this API will fill the value in it
        /// @param s          \b IN: ini attribute name
        /// @param  ppu8Array         \b OUT: u8 2-D Array get from ini
        /// @param dimension1          \b IN: dimension1 rows of array (user must indicate explictly number for lower level to malloc)
        /// @param dimension2          \b IN: dimension2 column of array (user must indicate explictly number for lower level to malloc )
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL mapi_config_helper_Get2DArray(dictionary *ini, const char* s, MI_U8 ** ppu8Array,int dimension1, int dimension2);
#endif
