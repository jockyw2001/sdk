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
#include "mhal_mhe.h"

MODULE_LICENSE("GPL");

EXPORT_SYMBOL(MHAL_MHE_Init);
EXPORT_SYMBOL(MHAL_MHE_CreateDevice);
EXPORT_SYMBOL(MHAL_MHE_DestroyDevice);
EXPORT_SYMBOL(MHAL_MHE_GetDevConfig);
EXPORT_SYMBOL(MHAL_MHE_CreateInstance);
EXPORT_SYMBOL(MHAL_MHE_DestroyInstance);
EXPORT_SYMBOL(MHAL_MHE_SetParam);
EXPORT_SYMBOL(MHAL_MHE_GetParam);
EXPORT_SYMBOL(MHAL_MHE_EncodeOneFrame);
EXPORT_SYMBOL(MHAL_MHE_EncodeFrameDone);
EXPORT_SYMBOL(MHAL_MHE_QueryBufSize);
EXPORT_SYMBOL(MHAL_MHE_IsrProc);

unsigned int mhe_clock = 0;  //clock setting
module_param(mhe_clock, uint, S_IRUGO | S_IWUSR);
