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

/*************************************************
*
* Copyright (c) 2006-2015 MStar Semiconductor, Inc.
* All rights reserved.
*
**************************************************
* File name: mi_common_debug.h
* Author:     mingkun.zhang@mstarsemi.com
* Version:    Initial Draft
* Date:       2015/4/15
* Description: MI COMMON DEBUG HEAD FILE
*
*
*
* History:
*
*    1. Date:        2015/4/15
*     Author:        mingkun.zhang@mstarsemi.com
*    Modification:  Created file
*
*************************************************/

#ifndef __MI_COMMON_DEBUG_H__
#define __MI_COMMON_DEBUG_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Debug level
 */
#define DBG_LEV_NO_OUTPUT  0
#define DBG_LEV_DEBUG      0x01
#define DBG_LEV_INFO       0x02
#define DBG_LEV_WARNING    0x04
#define DBG_LEV_ERROR      0x08
#define DBG_LEV_FATAL      0x10

/**
 * All debug level
 */
#define DBG_LEV_ALL        DBG_LEV_DEBUG


/**
 * Debug module
 */
#define DBG_MODULE_AENC     0x000100
#define DBG_MODULE_ADEC     0x000200
#define DBG_MODULE_AI       0x000400
#define DBG_MODULE_AO       0x000800
#define DBG_MODULE_VI       0x001000
#define DBG_MODULE_VENC     0x002000
#define DBG_MODULE_ISP      0x004000
#define DBG_MODULE_OSD      0x008000
#define DBG_MODULE_SYS      0x010000
#define DBG_MODULE_IE       0x020000
#define DBG_MODULE_SAMPLE   0x040000
#define DBG_MODULE_UT       0x080000
#define DBG_MODULE_WRAPER   0x100000
#define DBG_MODULE_OMX      0x200000
#define DBG_MODULE_BIND       0x400000
#define DBG_MODULE_OS       0x800000
/**
 * All debug module
 */
#define DBG_MODULE_ALL     0xFFFF00


/**
 * Config DEBUG_MODULE and DEBUG_LEVEL to set output message.
 * Debug module setting
 */
#define MI_DEBUG_MODULE DBG_MODULE_ALL

/**
 * Debug level setting
 */
#define MI_DEBUG_LEVEL DBG_LEV_INFO

#if 1
extern unsigned int g_uDbgLogLevel;
extern unsigned int g_uDbgLogModule;


#define LOG_DEBUG(module, fmt, args...) do { if ((g_uDbgLogModule & (module)) && (g_uDbgLogLevel <= DBG_LEV_DEBUG)){fprintf(stderr, "mi-debug:\t" fmt, ##args);} } while (0)
#define LOG_INFO(module, fmt, args...) do { if ((g_uDbgLogModule & (module)) && (g_uDbgLogLevel <= DBG_LEV_INFO)){fprintf(stderr, "mi-info:\t" fmt, ##args);} } while (0)
#define LOG_WARNING(module, fmt, args...) do { if ((g_uDbgLogModule & (module)) && (g_uDbgLogLevel <= DBG_LEV_WARNING)){fprintf(stderr, "mi-warning:\t" fmt, ##args);} } while (0)
#define LOG_ERROR(module, fmt, args...) do { if ((g_uDbgLogModule & (module)) && (g_uDbgLogLevel <= DBG_LEV_ERROR)){fprintf(stderr, "mi-error:\t" fmt, ##args);} } while (0)
#define LOG_FATAL(module, fmt, args...) do { if ((g_uDbgLogModule & (module)) && (g_uDbgLogLevel <= DBG_LEV_FATAL)){fprintf(stderr, "mi-fatal:\t" fmt, ##args);} } while (0)


#else

#define LOG_DEBUG(module, fmt, args...) {}
#define LOG_INFO(module, fmt, args...) {}
#define LOG_WARNING(module, fmt, args...) {}
#define LOG_ERROR(module, fmt, args...) {}
#define LOG_FATAL(module, fmt, args...) {}

#endif


#ifdef __cplusplus
}
#endif

#endif //__MI_COMMON_DEBUG_H__
