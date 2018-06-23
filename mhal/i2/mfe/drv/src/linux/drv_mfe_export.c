////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2017 MStar Semiconductor, Inc.
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

///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_export.c
/// @brief     Cam OS Export Symbol Source File for Linux Kernel Space
///            Only Include This File in Linux Kernel
///////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include "mhal_mfe.h"
#include "mhal_mfe_dc.h"

MODULE_LICENSE("GPL");

EXPORT_SYMBOL(MHAL_MFE_Init);
EXPORT_SYMBOL(MHAL_MFE_CreateDevice);
EXPORT_SYMBOL(MHAL_MFE_DestroyDevice);
EXPORT_SYMBOL(MHAL_MFE_GetDevConfig);
EXPORT_SYMBOL(MHAL_MFE_CreateInstance);
EXPORT_SYMBOL(MHAL_MFE_DestroyInstance);
EXPORT_SYMBOL(MHAL_MFE_SetParam);
EXPORT_SYMBOL(MHAL_MFE_GetParam);
EXPORT_SYMBOL(MHAL_MFE_EncodeOneFrame);
EXPORT_SYMBOL(MHAL_MFE_EncodeFrameDone);
EXPORT_SYMBOL(MHAL_MFE_QueryBufSize);
EXPORT_SYMBOL(MHAL_MFE_IsrProc);

EXPORT_SYMBOL(MHAL_MFE_DC_CreateDevice);
EXPORT_SYMBOL(MHAL_MFE_DC_DestroyDevice);
EXPORT_SYMBOL(MHAL_MFE_DC_CreateInstance);
EXPORT_SYMBOL(MHAL_MFE_DC_DestroyInstance);
EXPORT_SYMBOL(MHAL_MFE_DC_SetParam);
EXPORT_SYMBOL(MHAL_MFE_DC_GetParam);
EXPORT_SYMBOL(MHAL_MFE_DC_EncodeFrame);
EXPORT_SYMBOL(MHAL_MFE_DC_EncodeFrameDone);
EXPORT_SYMBOL(MHAL_MFE_DC_QueryBufSize);

unsigned int mfe_clock = 0;  //clock setting
module_param(mfe_clock, uint, S_IRUGO | S_IWUSR);
