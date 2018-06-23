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
#include "MsCommon.h"
#include "apiJPEG.h"

MODULE_LICENSE("GPL");

EXPORT_SYMBOL(MApi_JPEG_EnableOJPD);
EXPORT_SYMBOL(MApi_JPEG_Init_UsingOJPD);
EXPORT_SYMBOL(MApi_JPEG_Init);
EXPORT_SYMBOL(MApi_JPEG_DecodeHdr);
EXPORT_SYMBOL(MApi_JPEG_Decode);
EXPORT_SYMBOL(MApi_JPEG_Exit);
EXPORT_SYMBOL(MApi_JPEG_GetErrorCode);
EXPORT_SYMBOL(MApi_JPEG_GetJPDEventFlag);
EXPORT_SYMBOL(MApi_JPEG_SetJPDEventFlag);

EXPORT_SYMBOL(MApi_JPEG_Rst);
EXPORT_SYMBOL(MApi_JPEG_PowerOn);
EXPORT_SYMBOL(MApi_JPEG_PowerOff);
EXPORT_SYMBOL(MApi_JPEG_GetCurVidx);
EXPORT_SYMBOL(MApi_JPEG_IsProgressive);
EXPORT_SYMBOL(MApi_JPEG_ThumbnailFound);
EXPORT_SYMBOL(MApi_JPEG_GetWidth);
EXPORT_SYMBOL(MApi_JPEG_GetHeight);
EXPORT_SYMBOL(MApi_JPEG_GetOriginalWidth);
EXPORT_SYMBOL(MApi_JPEG_GetOriginalHeight);
EXPORT_SYMBOL(MApi_JPEG_GetNonAlignmentWidth);
EXPORT_SYMBOL(MApi_JPEG_GetNonAlignmentHeight);

EXPORT_SYMBOL(MApi_JPEG_GetAlignedPitch);
EXPORT_SYMBOL(MApi_JPEG_GetAlignedPitch_H);
EXPORT_SYMBOL(MApi_JPEG_GetAlignedWidth);
EXPORT_SYMBOL(MApi_JPEG_GetAlignedHeight);
EXPORT_SYMBOL(MApi_JPEG_GetScaleDownFactor);
EXPORT_SYMBOL(MApi_JPEG_SetMaxDecodeResolution);
EXPORT_SYMBOL(MApi_JPEG_SetProMaxDecodeResolution);
EXPORT_SYMBOL(MApi_JPEG_SetMRBufferValid);
EXPORT_SYMBOL(MApi_JPEG_UpdateReadInfo);
EXPORT_SYMBOL(MApi_JPEG_ProcessEOF);
EXPORT_SYMBOL(MApi_JPEG_SetErrCode);
EXPORT_SYMBOL(MApi_JPEG_SetDbgLevel);
EXPORT_SYMBOL(MApi_JPEG_GetDbgLevel);
EXPORT_SYMBOL(MApi_JPEG_GetInfo);
EXPORT_SYMBOL(MApi_JPEG_GetStatus);
EXPORT_SYMBOL(MApi_JPEG_GetLibVer);
EXPORT_SYMBOL(MApi_JPEG_HdlVidxChk);


#if SUPPORT_EXIF_EXTRA_INFO
EXPORT_SYMBOL(MApi_JPEG_GetEXIFManufacturer);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFModel);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFFlash);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFISOSpeedRatings);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFShutterSpeedValue);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFApertureValue);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFExposureBiasValue);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFFocalLength);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFImageWidth);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFImageHeight);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFExposureTime);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFFNumber);
#endif

EXPORT_SYMBOL(MApi_JPEG_GetBuffLoadType);
EXPORT_SYMBOL(MApi_JPEG_EnableISR);
EXPORT_SYMBOL(MApi_JPEG_DisableISR);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFDateTime);
EXPORT_SYMBOL(MApi_JPEG_GetEXIFOrientation);
EXPORT_SYMBOL(MApi_JPEG_GetControl);
EXPORT_SYMBOL(MApi_JPEG_DisableAddressConvert);
EXPORT_SYMBOL(MApi_JPEG_GetFreeMemory);
EXPORT_SYMBOL(MApi_JPEG_GetDataOffset);
EXPORT_SYMBOL(MApi_JPEG_GetSOFOffset);
EXPORT_SYMBOL(MApi_JPEG_SetNJPDInstance);
EXPORT_SYMBOL(MApi_JPEG_SetMHEG5);

#if SUPPORT_MPO_FORMAT
EXPORT_SYMBOL(MApi_JPEG_IsMPOFormat);
EXPORT_SYMBOL(MApi_JPEG_GetMPOIndex);
EXPORT_SYMBOL(MApi_JPEG_GetMPOAttr);
EXPORT_SYMBOL(MApi_JPEG_DumpMPO);
EXPORT_SYMBOL(MApi_JPEG_SetMPOBuffer);
EXPORT_SYMBOL(MApi_JPEG_SetMPOMaxDecodeResolution);
EXPORT_SYMBOL(MApi_JPEG_SetMPOProMaxDecodeResolution);
#endif

EXPORT_SYMBOL(MApi_JPEG_SetVerificationMode);
EXPORT_SYMBOL(MApi_JPEG_GetVerificationMode);
EXPORT_SYMBOL(MApi_NJPD_Debug);
EXPORT_SYMBOL(MApi_JPEG_IsNJPD);
