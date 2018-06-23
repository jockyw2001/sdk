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

#ifndef __MI_DATA_TYPE_H__ //
#define __MI_DATA_TYPE_H__ //


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
//#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <limits.h>

#ifdef __cplusplus
extern "C"
{
#endif

//include "mi_common_debug.h"

typedef char                    S8;
typedef short                   S16;
typedef int                     S32;

typedef unsigned char           U8;
typedef unsigned short          U16;
typedef unsigned int            U32;
typedef unsigned long long      U64;
typedef long long               S64;

typedef float                   F32;
/*
typedef enum
{
    FALSE = 0,
    TRUE  = 1,
} BOOL;
*/

#ifndef VOID
#define VOID    void
#endif

#ifndef NULL
#define NULL    0L
#endif

#define ABS(x)  ((x) < 0 ? -(x) : (x))
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX((x), MIN((y), (z)))
#define SGN(x)       (((x) >= 0) ? 1 : -1)

#define WRAPER_CHECK_FUNCPTR(PTR) \
    if(NULL == PTR){LOG_ERROR(DBG_MODULE_WRAPER,"not support %s now.\n",__func__);return -1;}
#define WRAPER_CHECK_FUNCPTR_NORETURN(PTR) \
        if(NULL == PTR){LOG_ERROR(DBG_MODULE_WRAPER,"not support %s now.\n",__func__);return;}

#define MI_RETURN(ERRORCODE) \
    switch((int)ERRORCODE)  \
    {\
        case OMX_ErrorNone:         \
            return MI_RET_SUCCESS;\
        case OMX_ErrorTimeout:\
        case MI_RET_TIMEOUT:\
            return MI_RET_TIMEOUT;\
        case OMX_ErrorBadParameter:\
        case MI_RET_BAD_PARAMETER:\
            return MI_RET_BAD_PARAMETER;\
        case OMX_ErrorNotReady:\
        case MI_RET_NOT_READY:\
            return MI_RET_NOT_READY;\
        case MI_RET_BUSY:\
            return MI_RET_BUSY;\
        case MI_RET_DEVICE_NOT_EXIST:\
            return MI_RET_DEVICE_NOT_EXIST;\
        case OMX_ErrorComponentNotFound:\
        case MI_RET_RESOURCE_LIMITED:\
            return MI_RET_RESOURCE_LIMITED;\
        case OMX_ErrorUndefined:\
        case MI_RET_UNDERFINED:\
            return MI_RET_UNDERFINED;\
        case MI_RET_FAIL:\
        default:\
            return MI_RET_FAIL;\
    }


#if 0
typedef enum _MI_RET_E
{
    MI_RET_FAIL                             = -1,
    MI_RET_SUCCESS                          = 0x00000000,
    MI_RET_BAD_PARAMETER                    = 0x00000001,
    MI_RET_NOT_CONFIG                       = 0x00000002,
    MI_RET_NOT_SUPPORT                      = 0x00000003,
    MI_RET_MEMORY_OUT                       = 0x00000004,
    MI_RET_OVERFLOW                         = 0x00000005,
    MI_RET_NOT_READY                        = 0x00000006,
    MI_RET_BUSY                             = 0x00000007,
    MI_RET_TIMEOUT                          = 0x00000008,
    MI_RET_INVALID_STATE                    = 0x00000009,
    MI_RET_HARDWARE_ERROR                   = 0x0000000A,
    MI_RET_VERSION_MISMATCH                 = 0x0000000B,
    MI_RET_RESOURCE_LIMITED                 = 0x0000000C,
    MI_RET_DEVICE_EXIST                     = 0x0000000D,
    MI_RET_DEVICE_NOT_EXIST                 = 0x0000000E,
    MI_RET_CHANNEL_EXIST                    = 0x0000000F,
    MI_RET_CHANNEL_NOT_EXIST                = 0x00000010,
    MI_RET_SAT_X_FAIL                       = 0X10000000,//sat ����
    MI_RET_UNDERFINED                       = 0x00000011,

    /*MI_MVE*/
    MI_MVE_RET_INIT_ERROR                   = 0x10000101,   /*MVE init error*/
    MI_MVE_RET_IC_CHECK_ERROR               = 0x10000102,   /*MVE platform check error*/
    MI_MVE_RET_INVALID_HANDLE               = 0x10000103,   /*Invalid MVE handle*/
    MI_MVE_RET_INVALID_PARAMETER            = 0x10000104,   /*Invalid MVE parameter*/
    MI_MVE_RET_INVALID_ADDRESS              = 0x10000105,   /*Invalid buffer address*/
    MI_MVE_RET_NOT_SUPPORT                  = 0x10000106,   /*Unsupported feature of MVE*/
    MI_MVE_RET_HARDWARE_TIMEOUT             = 0x10000107,   /*MVE hardware timeout*/
    MI_MVE_RET_HARDWARE_ERROR               = 0x10000108,   /*MVE hardware error*/
    MI_MVE_RET_HARDWARE_BUSY                = 0x10000109,   /*MVE hardware busy*/
    MI_MVE_RET_INVALID_BUFFER_NAME          = 0x1000010A,   /*Invalid buffer name when allocating MVE buffer*/
    MI_MVE_RET_MALLOC_ERROR                 = 0x1000010B,   /*Allocate MVE buffer error*/

    /*MI_BCRY*/
    MI_BCRY_RET_INIT_ERROR                  = 0x10000201,   /*BCRY init error*/
    MI_BCRY_RET_IC_CHECK_ERROR              = 0x10000202,   /*BCRY platform check error*/
    MI_BCRY_RET_INVALID_HANDLE              = 0x10000203,   /*Invalid BCRY handle*/
    MI_BCRY_RET_INVALID_SAMPLERATE          = 0x10000204,   /*Invalid Sample rate of BCRY*/

    /*MI_VG*/
    MI_VG_RET_INIT_ERROR                    = 0x10000301,   /*VG init error*/
    MI_VG_RET_IC_CHECK_ERROR                = 0x10000302,   /*VG platform check error*/
    MI_VG_RET_INVALID_HANDLE                = 0x10000303,   /*Invalid VG handle*/
    MI_VG_RET_INVALID_USER_INFO_POINTER     = 0x10000304,   /*Invalid user information pointer*/
    MI_VG_RET_INVALID_ENVIRONMENT_STATE     = 0x10000305,   /*Invalid environment state*/
    MI_VG_RET_INVALID_ENVIRONMENT_POINTER   = 0x10000306,   /*Invalid environment pointer*/
    MI_VG_RET_INVALID_LINE_NUMBER           = 0x10000307,   /*Invalid line number*/
    MI_VG_RET_INVALID_LINE_POINTER          = 0x10000308,   /*Invalid line pointer*/
    MI_VG_RET_INVALID_COORDINATE_POINTER    = 0x10000309,   /*Invalid coordinate pointer*/
    MI_VG_RET_INVALID_FIRST_LINE_INFO       = 0x1000030A,   /*Invalid first line information*/
    MI_VG_RET_INVALID_SECOND_LINE_INFO      = 0x1000030B,   /*Invalid second line information*/
    MI_VG_RET_INVALID_THRESHOLD             = 0x1000030C,   /*Invalid object threshold*/
    MI_VG_RET_INVALID_THRESHOLD_POINTER     = 0x1000030D,   /*Invalid threshold pointer*/
    MI_VG_RET_INVALID_INPUT_POINTER         = 0x1000030E,   /*Invalid input pointer*/
    MI_VG_RET_OPERATE_ERROR                 = 0x1000030F,   /*VG operate error*/
    MI_VG_RET_INVALID_ALARM_POINTER         = 0x10000310,   /*Invalid alarm pointer*/
    MI_VG_RET_INVALID_DEBUG_POINTER         = 0x10000311,   /*Invalid debug pointer*/

    /*MI_MD*/
    MI_MD_RET_INIT_ERROR                    = 0x10000401,   /*MD init error*/
    MI_MD_RET_IC_CHECK_ERROR                = 0x10000402,   /*MD platform check error*/
    MI_MD_RET_INVALID_HANDLE                = 0x10000403,   /*Invalid MD handle*/
    MI_MD_RET_INVALID_PARAMETER             = 0x10000404,   /*Invalid MD parameter*/
    MI_MD_RET_MALLOC_ERROR                  = 0x10000405,   /*Allocate MD workiung buffer error*/
    MI_MD_RET_REGION_INIT_ERROR             = 0x10000406,   /*Multi region initial error*/

    /*MI_OD*/
    MI_OD_RET_INIT_ERROR                    = 0x10000501,   /*OD init error*/
    MI_OD_RET_IC_CHECK_ERROR                = 0x10000502,   /*OD platform check error*/
    MI_OD_RET_INVALID_HANDLE                = 0x10000503,   /*Invalid OD handle*/
    MI_OD_RET_INVALID_PARAMETER             = 0x10000504,   /*Invalid OD parameter*/
    MI_OD_RET_INVALID_WINDOW                = 0x10000505,   /*Invalid window*/
    MI_OD_RET_INVALID_COLOR_TYPE            = 0x10000506,   /*Invalid color tpye*/

    /*MI_FDFR*/
    MI_FDFR_RET_INIT_ERROR                  = 0x10000601,   /*FDFR init error*/
    MI_FDFR_RET_IC_CHECK_ERROR              = 0x10000602,   /*FDFR platform check error*/
    MI_FDFR_RET_INVALID_HANDLE              = 0x10000603,   /*Invalid FDFR handle*/
    MI_FDFR_RET_INVALID_PARAMETER           = 0x10000604,   /*Invalid FDFR parameter*/
    MI_FDFR_RET_FD_ENABLE_ERROR             = 0x10000605,   /*FD enable error*/
    MI_FDFR_RET_FR_ENABLE_ERROR             = 0x10000606,   /*FR enable error*/
    MI_FDFR_RET_FR_GET_FEATURE_DATA_ERROR   = 0x10000607,   /*FR get feature data error*/
    MI_FDFR_RET_FR_SET_FEATURE_DATA_ERROR   = 0x10000608,   /*FR set feature data error*/
    MI_FDFR_RET_IN_IMAGE_ERROR              = 0x10000609   /*Input image error*/
} MI_RET;
#endif

#define MALLOC(s) malloc(s)
#define FREEIF(m) if(m!=0){free(m);m=NULL;}

/**
 * point
 */
#if 0
typedef struct _Point_t
{
    S32 x;
    S32 y;
} Point_t;

/**
 * size_t
 */
typedef struct _Size_t
{
    U32 width;
    U32 height;
} Size_t;

/**
 * rect
 */
typedef struct _Rect_t
{
    S32 x;
    S32 y;
    U32 width;
    U32 height;
} Rect_t;

typedef struct _YUVColor_t
{
    U8  y;
    U8  u;
    U8  v;
    U8  transparent;
} YUVColor_t;
/**
 * rotate
 */
typedef enum _Rotate_e
{
    ROTATE_0    = 0,
    ROTATE_90   = 1,
    ROTATE_180  = 2,
    ROTATE_270  = 3,
} Rotate_e;

/**
 * color
 */
typedef struct _Color_t
{
    U8  a;
    U8  r;
    U8  g;
    U8  b;
} Color_t;



typedef S32 VI_CHN;
typedef S32 VENC_CHN;

/**
*   channel type
*/
typedef enum _ChnType_e
{
    AI_CHANNEL = 0,
    AO_CHANNEL,
    AENC_CHANNEL,
    ADEC_CHANNEL,

    VI_CHANNEL,
    VENC_CHANNEL,

    CHANNEL_NUM,
} ChnType_e;

/**
 * channel ����
 */
/*
typedef struct _Achn_t
{
Chn_t chn;
}AChn_t;
*/

typedef enum
{
    TUNNEL_MODE = 0,
    USER_MODE,
    SHARE_MODE,
    INVALID_MODE
} ChannelMode_e;

typedef struct _Chn_t
{
    ChnType_e chnType;
    U32 deviceID;
    U32 chnID;
} Chn_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /*__MI_DATA_TYPE_H__ */

