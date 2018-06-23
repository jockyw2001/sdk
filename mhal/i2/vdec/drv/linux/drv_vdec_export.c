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
#include "apiVDEC_EX.h"

MODULE_LICENSE("GPL");

EXPORT_SYMBOL(MApi_VDEC_EX_GetLibVer);
EXPORT_SYMBOL(MApi_VDEC_EX_GetInfo);
EXPORT_SYMBOL(MApi_VDEC_EX_GetStatus);
EXPORT_SYMBOL(MApi_VDEC_EX_EnableTurboMode);
EXPORT_SYMBOL(MApi_VDEC_EX_CheckCaps);
EXPORT_SYMBOL(MApi_VDEC_EX_SetSingleDecode);
EXPORT_SYMBOL(MApi_VDEC_EX_Rst);
EXPORT_SYMBOL(MApi_VDEC_EX_SetFrcMode);
EXPORT_SYMBOL(MApi_VDEC_EX_SetDynScalingParams);

EXPORT_SYMBOL(MApi_VDEC_EX_GetFreeStream);
EXPORT_SYMBOL(MApi_VDEC_EX_GetConfig);
EXPORT_SYMBOL(MApi_VDEC_EX_Init);
EXPORT_SYMBOL(MApi_VDEC_EX_Exit);
EXPORT_SYMBOL(MApi_VDEC_EX_CheckDispInfoRdy);
EXPORT_SYMBOL(MApi_VDEC_EX_SetDbgLevel);
EXPORT_SYMBOL(MApi_VDEC_EX_Play);
EXPORT_SYMBOL(MApi_VDEC_EX_Pause);
EXPORT_SYMBOL(MApi_VDEC_EX_Resume);
EXPORT_SYMBOL(MApi_VDEC_EX_StepDisp);
EXPORT_SYMBOL(MApi_VDEC_EX_IsStepDispDone);
EXPORT_SYMBOL(MApi_VDEC_EX_StepDecode);
EXPORT_SYMBOL(MApi_VDEC_EX_IsStepDecodeDone);
EXPORT_SYMBOL(MApi_VDEC_EX_SetTrickMode);
EXPORT_SYMBOL(MApi_VDEC_EX_PushDecQ);
EXPORT_SYMBOL(MApi_VDEC_EX_Flush);
EXPORT_SYMBOL(MApi_VDEC_EX_EnableLastFrameShow);
EXPORT_SYMBOL(MApi_VDEC_EX_SetSpeed);
EXPORT_SYMBOL(MApi_VDEC_EX_SetFreezeDisp);
EXPORT_SYMBOL(MApi_VDEC_EX_SetBlueScreen);
EXPORT_SYMBOL(MApi_VDEC_EX_ResetPTS);
EXPORT_SYMBOL(MApi_VDEC_EX_AVSyncOn);
EXPORT_SYMBOL(MApi_VDEC_EX_SetAVSyncFreerunThreshold);
EXPORT_SYMBOL(MApi_VDEC_EX_GetDispInfo);
EXPORT_SYMBOL(MApi_VDEC_EX_IsAVSyncOn);
EXPORT_SYMBOL(MApi_VDEC_EX_IsWithValidStream);
EXPORT_SYMBOL(MApi_VDEC_EX_IsDispFinish);
EXPORT_SYMBOL(MApi_VDEC_EX_IsFrameRdy);
EXPORT_SYMBOL(MApi_VDEC_EX_IsIFrameFound);
EXPORT_SYMBOL(MApi_VDEC_EX_IsSeqChg);
EXPORT_SYMBOL(MApi_VDEC_EX_GetActiveSrcMode);
EXPORT_SYMBOL(MApi_VDEC_EX_IsReachSync);
EXPORT_SYMBOL(MApi_VDEC_EX_IsStartSync);
EXPORT_SYMBOL(MApi_VDEC_EX_IsFreerun);
EXPORT_SYMBOL(MApi_VDEC_EX_IsWithLowDelay);
EXPORT_SYMBOL(MApi_VDEC_EX_IsAllBufferEmpty);
EXPORT_SYMBOL(MApi_VDEC_EX_GetExtDispInfo);
EXPORT_SYMBOL(MApi_VDEC_EX_GetDecFrameInfo);
EXPORT_SYMBOL(MApi_VDEC_EX_GetDispFrameInfo);
EXPORT_SYMBOL(MApi_VDEC_EX_GetDecTimeCode);
EXPORT_SYMBOL(MApi_VDEC_EX_GetDispTimeCode);
EXPORT_SYMBOL(MApi_VDEC_EX_SetEvent);
EXPORT_SYMBOL(MApi_VDEC_EX_UnsetEvent);
EXPORT_SYMBOL(MApi_VDEC_EX_SetEvent_MultiCallback);
EXPORT_SYMBOL(MApi_VDEC_EX_UnsetEvent_MultiCallback);
EXPORT_SYMBOL(MApi_VDEC_EX_GetEventInfo);
EXPORT_SYMBOL(MApi_VDEC_EX_FireDecCmd);
EXPORT_SYMBOL(MApi_VDEC_EX_SeekToPTS);
EXPORT_SYMBOL(MApi_VDEC_EX_SkipToPTS);
EXPORT_SYMBOL(MApi_VDEC_EX_DisableDeblocking);
EXPORT_SYMBOL(MApi_VDEC_EX_DisableQuarterPixel);
EXPORT_SYMBOL(MApi_VDEC_EX_SetBalanceBW);

EXPORT_SYMBOL(MApi_VDEC_EX_GetActiveFormat);
EXPORT_SYMBOL(MApi_VDEC_EX_GetColourPrimaries);
EXPORT_SYMBOL(MApi_VDEC_EX_GetFwVersion);
EXPORT_SYMBOL(MApi_VDEC_EX_GetGOPCnt);
EXPORT_SYMBOL(MApi_VDEC_EX_GetESWritePtr);
EXPORT_SYMBOL(MApi_VDEC_EX_GetESReadPtr);
EXPORT_SYMBOL(MApi_VDEC_EX_GetPTS);
EXPORT_SYMBOL(MApi_VDEC_EX_GetNextPTS);
EXPORT_SYMBOL(MApi_VDEC_EX_GetVideoPtsStcDelta);
EXPORT_SYMBOL(MApi_VDEC_EX_GetVideoFirstPts);
EXPORT_SYMBOL(MApi_VDEC_EX_GetErrCode);
EXPORT_SYMBOL(MApi_VDEC_EX_GetErrCnt);
EXPORT_SYMBOL(MApi_VDEC_EX_GetBitsRate);
EXPORT_SYMBOL(MApi_VDEC_EX_GetFrameCnt);
EXPORT_SYMBOL(MApi_VDEC_EX_GetSkipCnt);
EXPORT_SYMBOL(MApi_VDEC_EX_GetDropCnt);
EXPORT_SYMBOL(MApi_VDEC_EX_GetDispCnt);

EXPORT_SYMBOL(MApi_VDEC_EX_GetDecQVacancy);
EXPORT_SYMBOL(MApi_VDEC_EX_IsAlive);
EXPORT_SYMBOL(MApi_VDEC_EX_SetControl);
EXPORT_SYMBOL(MApi_VDEC_EX_GetControl);
EXPORT_SYMBOL(MApi_VDEC_EX_PreSetControl);
EXPORT_SYMBOL(MApi_VDEC_EX_SetBlockDisplay);
EXPORT_SYMBOL(MApi_VDEC_EX_EnableESBuffMalloc);
EXPORT_SYMBOL(MApi_VDEC_EX_GetESBuffVacancy);
EXPORT_SYMBOL(MApi_VDEC_EX_GetESBuff);
EXPORT_SYMBOL(MApi_VDEC_EX_GetNextDispFrame);
EXPORT_SYMBOL(MApi_VDEC_EX_DisplayFrame);
EXPORT_SYMBOL(MApi_VDEC_EX_ReleaseFrame);
EXPORT_SYMBOL(MApi_VDEC_EX_GetActiveCodecType);

EXPORT_SYMBOL(MApi_VDEC_EX_GetHwMemSize);
