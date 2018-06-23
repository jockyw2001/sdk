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
/// @file      drv_jpe_export.c
/// @brief     JPE Export Symbol Source File for Linux Kernel Space
///            Only Include This File in Linux Kernel
///////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include "mhal_jpe.h"

MODULE_LICENSE("GPL");

EXPORT_SYMBOL(MHAL_JPE_Init);
EXPORT_SYMBOL(MHAL_JPE_CreateDevice);
EXPORT_SYMBOL(MHAL_JPE_DestroyDevice);
EXPORT_SYMBOL(MHAL_JPE_GetDevConfig);
EXPORT_SYMBOL(MHAL_JPE_CreateInstance);
EXPORT_SYMBOL(MHAL_JPE_DestroyInstance);
EXPORT_SYMBOL(MHAL_JPE_SetParam);
EXPORT_SYMBOL(MHAL_JPE_GetParam);
EXPORT_SYMBOL(MHAL_JPE_EncodeOneFrame);
EXPORT_SYMBOL(MHAL_JPE_EncodeFrameDone);
EXPORT_SYMBOL(MHAL_JPE_QueryBufSize);
EXPORT_SYMBOL(MHAL_JPE_IsrProc);
EXPORT_SYMBOL(MHAL_JPE_GetEngStatus);
EXPORT_SYMBOL(MHAL_JPE_EncodeAddOutBuff);
EXPORT_SYMBOL(MHAL_JPE_EncodeCancel);

unsigned int jpe_clock = 0;  //clock setting
module_param(jpe_clock, uint, S_IRUGO | S_IWUSR);
