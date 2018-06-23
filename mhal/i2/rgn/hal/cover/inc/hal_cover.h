////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

/**
* @file     hal_scl_util.h
* @version
* @Platform I3
* @brief    This file defines the HAL SCL utility interface
*
*/

#ifndef __HAL_COVER_H__
#define __HAL_COVER_H__

//=============================================================================
// Defines
//=============================================================================
#include "mhal_common.h"
#include "mhal_cmdq.h"
#define  bool   unsigned char

//=============================================================================
// enum
//=============================================================================
typedef enum
{
    E_HAL_ISPSCL1_COVER_ID_00     = 0x00,
    E_HAL_ISPSCL4_COVER_ID_01     = 0x01,
    E_HAL_DIP_COVER_ID_02   = 0x02,
} HalCoverIdType_e;

typedef enum
{
    E_HAL_COVER_BWIN_ID_0 = 0,
    E_HAL_COVER_BWIN_ID_1 = 1,
    E_HAL_COVER_BWIN_ID_2 = 2,
    E_HAL_COVER_BWIN_ID_3 = 3,
    E_HAL_COVER_BWIN_ID_NUM = 4,
} HalCoverWinIdType_e;

typedef enum
{
    E_HAL_COVER_CMDQ_VPE_ID_0 = 0,
    E_HAL_COVER_CMDQ_DIVP_ID_1 = 1,
    E_HAL_COVER_CMDQ_ID_NUM = 2,
}HalCoverCmdqIdType_e;

//=============================================================================
// struct
//=============================================================================
typedef struct
{
    u16 u16X;
    u16 u16Y;
    u16 u16Width;
    u16 u16Height;
} HalCoverWindowType_t;

typedef struct
{
    u8 u8R;
    u8 u8G;
    u8 u8Bh;
} HalCoverColorType_t;

//=============================================================================

//=============================================================================
#ifndef __HAL_COVER_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE void HalCoverInitCmdq(void);
INTERFACE void HalCoverSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,HalCoverCmdqIdType_e eHalCmdqId);
INTERFACE void HalCoverSetWindowSize(HalCoverIdType_e eCoverId, HalCoverWinIdType_e eWinId, HalCoverWindowType_t tWinCfg);
INTERFACE void HalCoverSetColor(HalCoverIdType_e eCoverId, HalCoverWinIdType_e eWinId, u8 u8R, u8 u8G, u8 u8B);
INTERFACE void HalCoverSetEnableWin(HalCoverIdType_e eCoverId, HalCoverWinIdType_e eWinId, bool bEn);

#undef INTERFACE
#endif /* __HAL_COVER_H__ */
