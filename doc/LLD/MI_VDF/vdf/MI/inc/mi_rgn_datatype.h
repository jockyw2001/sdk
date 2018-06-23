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

#ifndef __MI_RGN_DATATYPE_
#define __MI_RGN_DATATYPE_

#define MI_RGN_OSD_MAX_NUM 4
#define MI_RGN_COVER_MAX_NUM 4
#define MI_RGN_MIN_WIDTH 0
#define MI_RGN_MAX_WIDTH 1920
#define MI_RGN_MIN_HEIGHT 0
#define MI_RGN_MAX_HEIGHT 1080
#define MI_RGN_VPE_MAX_CH_NUM 64
#define MI_RGN_VPE_MAX_PORT_NUM 64

#define MI_RGN_DIVP_MAX_CH_NUM 64
#define MI_RGN_MAX_HANDLE 1024
#define MI_RGN_HANDLE_NULL -1
#define MI_RGN_MAX_PALETTE_TABLE_NUM 16

#define MI_RGN_VPE_PORT_MAXNUM 4
#define MI_RGN_DIVP_PORT_MAXNUM 1


typedef enum
{
    E_MI_RGN_MODID_VPE = 0,
    E_MI_RGN_MODID_DIVP,
    E_MI_RGN_MODID_MAX
}MI_RGN_ModId_e;

typedef enum
{
    E_MI_RGN_TYPE_OSD = 0,
    E_MI_RGN_TYPE_COVER,
    E_MI_RGN_TYPE_MAX
} MI_RGN_Type_e;

typedef enum
{
    E_MI_RGN_PIXEL_FORMAT_RGB1555 = 0,
    E_MI_RGN_PIXEL_FORMAT_RGB4444,
    E_MI_RGN_PIXEL_FORMAT_RGBI2,
    E_MI_RGN_PIXEL_FORMAT_RGBI4,
    E_MI_RGN_PIXEL_FORMAT_MAX
} MI_RGN_PixelFormat_e;

typedef struct MI_RGN_ChnPort_s
{
    MI_RGN_ModId_e  eModId;
    MI_S32 s32DevId;
    MI_S32 s32ChnId;
    MI_S32 s32OutputPortId;
}MI_RGN_ChnPort_t;

typedef struct MI_RGN_Size_s
{
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_RGN_Size_t;

typedef struct MI_RGN_Bitmap_s
{
    MI_RGN_PixelFormat_e ePixelFormat;
    MI_RGN_Size_t stSize;
    void *pData;
} MI_RGN_Bitmap_t;

typedef struct MI_RGN_OsdInitParam_s
{
    MI_RGN_PixelFormat_e ePixelFmt;
    MI_RGN_Size_t stSize;
}MI_RGN_OsdInitParam_t;


typedef struct MI_RGN_Point_s
{
    MI_U32 u32X;
    MI_U32 u32Y;
}MI_RGN_Point_t;


typedef enum
{
    E_MI_RGN_LESSTHANLUM_THRESH = 0,
    E_MI_RGN_MORETHANLUM_THRESH
}MI_RGN_InvertColorMode_e;


typedef struct MI_RGN_OsdInvertColor_s
{
    MI_U32 u32LumThresh;
    MI_RGN_InvertColorMode_e  eChgMod;
    MI_BOOL bInvColEn;
}MI_RGN_OsdInvertColor_t;


typedef struct MI_RGN_Rect_s
{
    MI_S32 s32X;
    MI_S32 s32Y;
    MI_U32 u32Width;
    MI_U32 u32Height;
}MI_RGN_Rect_t;


typedef struct MI_RGN_CoverChnPortParam_s
{
    MI_U32 u32Layer;
    MI_RGN_Size_t stSize;
    MI_U32 u32Color;
}MI_RGN_CoverChnPortParam_t;


typedef struct MI_RGN_Attr_s
{
    MI_RGN_Type_e eType;
    MI_RGN_OsdInitParam_t stOsdInitParam;
}MI_RGN_Attr_t;

typedef struct MI_RGN_CoverChnParam_s
{
    MI_RGN_Rect_t  stRect;
    MI_U32 u32Color;
    MI_U32 u32Layer;
}MI_RGN_CoverChnParam_t;

typedef union
{
    MI_RGN_CoverChnPortParam_t stCoverChnPort;
} MI_RGN_ChnPortParamUnion_u;

typedef struct MI_RGN_ChnPortParam_s
{
    MI_BOOL bShow;
    MI_RGN_Point_t stPoint;
    MI_RGN_CoverChnPortParam_t stCoverPara;
} MI_RGN_ChnPortParam_t;


typedef struct MI_RGN_CanvasInfo_s
{
    MI_PHY phyAddr;
    MI_VIRT virtAddr;
    MI_RGN_Size_t stSize;
    MI_U32 u32Stride;
    MI_RGN_PixelFormat_e  ePixelFmt;
} MI_RGN_CanvasInfo_t;

typedef struct MI_RGN_PaletteElememt_s
{
    MI_U8 u8Alpha;
    MI_U8 u8Red;
    MI_U8 u8Green;
    MI_U8 u8Blue;
}MI_RGN_PaletteElememt_t;

typedef struct MI_RGN_PaletteTable_s
{
    MI_RGN_PaletteElememt_t astElememt[MI_RGN_MAX_PALETTE_TABLE_NUM];
}MI_RGN_PaletteTable_t;


#define MI_RGN_OK  MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_INFO, MI_SUCCESS)

/* PingPong buffer change when set attr, it needs to remap memory in mpi interface */
#define MI_NOTICE_RGN_BUFFER_CHANGE  MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_INFO, MI_SUCCESS)

/* invlalid handle */
#define MI_ERR_RGN_INVALID_HANDLE     MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
/* invlalid device ID */
#define MI_ERR_RGN_INVALID_DEVID     MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define MI_ERR_RGN_INVALID_CHNID     MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define MI_ERR_RGN_ILLEGAL_PARAM     MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
/* channel exists */
#define MI_ERR_RGN_EXIST             MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_EXIST)
/*UN exist*/
#define MI_ERR_RGN_UNEXIST           MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_UNEXIST)
/* using a NULL point */
#define MI_ERR_RGN_NULL_PTR          MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define MI_ERR_RGN_NOT_CONFIG        MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define MI_ERR_RGN_NOT_SUPPORT       MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT)
/* operation is not permitted ,eg, try to change stati attribute */
#define MI_ERR_RGN_NOT_PERM          MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define MI_ERR_RGN_NOMEM             MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM)
/* failure caused by malloc buffer */
#define MI_ERR_RGN_NOBUF             MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOBUF)
/* no data in buffer */
#define MI_ERR_RGN_BUF_EMPTY         MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_EMPTY)
/* no buffer for new data */
#define MI_ERR_RGN_BUF_FULL          MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_FULL)
/* bad address, eg. used for copy_from_user & copy_to_user */
#define MI_ERR_RGN_BADADDR           MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BADADDR)
/* resource is busy, eg. destroy a venc chn without unregistering it */
#define MI_ERR_RGN_BUSY              MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)

/* System is not ready,maybe not initialed or loaded.
 * Returning the error code when opening a device file failed.
 */
#define MI_ERR_RGN_NOTREADY          MI_DEF_ERR(E_MI_SYS_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SYS_NOTREADY)


#endif /* End of #ifndef __MI_RGN_DATATYPE_ */

