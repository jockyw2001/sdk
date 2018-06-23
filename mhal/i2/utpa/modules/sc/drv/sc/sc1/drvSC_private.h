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
////////////////////////////////////////////////////////////////////////////////

#ifndef _DRVSC_PRIV_H_
#define _DRVSC_PRIV_H_

////////////////////////////////////////////////////////////////////////////////
/// @file drvSC_private.h
/// @author MStar Semiconductor Inc.
/// @brief Smart Card control driver
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

#include "MsTypes.h"
#include "drvSC.h"

typedef SC_Result (*IOCTL_SC_INIT)(MS_U8);
typedef SC_Result (*IOCTL_SC_EXIT)(MS_U8);
typedef SC_Result (*IOCTL_SC_OPEN)(MS_U8, MS_U8, SC_Param*, P_SC_Callback);
typedef SC_Result (*IOCTL_SC_ACTIVATE)(MS_U8);
typedef SC_Result (*IOCTL_SC_DEACTIVATE)(MS_U8);
typedef SC_Result (*IOCTL_SC_CLOSE)(MS_U8);
typedef SC_Result (*IOCTL_SC_RESET)(MS_U8, SC_Param*);
typedef SC_Result (*IOCTL_SC_CLEARSTATE)(MS_U8);
typedef SC_Result (*IOCTL_SC_GETATR)(MS_U8, MS_U32, MS_U8*, MS_U16*, MS_U8*, MS_U16*);
typedef SC_Result (*IOCTL_SC_CONFIG)(MS_U8, SC_Param*);
typedef SC_Result (*IOCTL_SC_SEND)(MS_U8, MS_U8*, MS_U16, MS_U32);
typedef SC_Result (*IOCTL_SC_RECV)(MS_U8, MS_U8*, MS_U16*, MS_U32);
typedef SC_Result (*IOCTL_SC_GETCAPS)(SC_Caps*);
typedef SC_Result (*IOCTL_SC_SETPPS)(MS_U8, MS_U8, MS_U8, MS_U8);
typedef SC_Result (*IOCTL_SC_PPS)(MS_U8);
typedef SC_Result (*IOCTL_SC_GETSTATUS)(MS_U8, SC_Status*);
typedef void (*IOCTL_SC_SETDBGLEVEL)(SC_DbgLv);
typedef SC_Result (*IOCTL_SC_RAWEXCHANGE)(MS_U8, MS_U8*, MS_U16*, MS_U8*, MS_U16*);
typedef MS_BOOL (*IOCTL_SC_CARDVOLTAGE_CONFIG)(MS_U8, SC_VoltageCtrl);
typedef SC_Result (*IOCTL_SC_RESET_ATR)(MS_U8, SC_Param*, MS_U8*, MS_U16*, MS_U8*, MS_U16*);
typedef SC_Result (*IOCTL_SC_T0_SENDRECV)(MS_U8, MS_U8*, MS_U16, MS_U8*, MS_U16*);
typedef SC_Result (*IOCTL_SC_T1_SENDRECV)(MS_U8, MS_U8*, MS_U16*, MS_U8*, MS_U16*);
typedef SC_Result (*IOCTL_SC_T14_SENDRECV)(MS_U8, MS_U8*, MS_U16, MS_U8*, MS_U16*);
typedef SC_Result (*IOCTL_SC_GETINFO)(MS_U8, SC_Info*);
typedef SC_Result (*IOCTL_SC_POWEROFF)(void);
typedef SC_Result (*IOCTL_SC_SETGUARDTIME)(MS_U8, MS_U8);
typedef MS_U32 (*IOCTL_SC_SETPOWERSTATE)(EN_POWER_MODE);

#if defined(MSOS_TYPE_LINUX_KERNEL)
typedef SC_Result (*IOCTL_SC_KDRVKERNELMODEPOLL)(MS_U8, MS_U32*, MS_U32);
#endif

typedef struct _SC_RESOURCE_PRIVATE
{
    MS_U32 Dummy;
}SC_RESOURCE_PRIVATE;

typedef struct _SC_INSTANT_PRIVATE
{
    IOCTL_SC_INIT               fpSCInit;
    IOCTL_SC_EXIT               fpSCExit;
    IOCTL_SC_OPEN               fpSCOpen;
    IOCTL_SC_ACTIVATE           fpSCActivate;
    IOCTL_SC_DEACTIVATE         fpSCDeactivate;
    IOCTL_SC_CLOSE              fpSCClose;
    IOCTL_SC_RESET              fpSCReset;
    IOCTL_SC_CLEARSTATE         fpSCClearState;
    IOCTL_SC_GETATR             fpSCGetATR;
    IOCTL_SC_CONFIG             fpSCConfig;
    IOCTL_SC_SEND               fpSCSend;
    IOCTL_SC_RECV               fpSCRecv;
    IOCTL_SC_GETCAPS            fpSCGetCaps;
    IOCTL_SC_SETPPS             fpSCSetPPS;
    IOCTL_SC_PPS                fpSCPPS;
    IOCTL_SC_GETSTATUS          fpSCGetStatus;
    IOCTL_SC_SETDBGLEVEL        fpSCSetDbgLevel;
    IOCTL_SC_RAWEXCHANGE        fpSCRawExchange;
    IOCTL_SC_CARDVOLTAGE_CONFIG fpSCCardVoltage_Config;
    IOCTL_SC_RESET_ATR          fpSCReset_ATR;
    IOCTL_SC_T0_SENDRECV        fpSCT0_SendRecv;
    IOCTL_SC_T1_SENDRECV        fpSCT1_SendRecv;
    IOCTL_SC_T14_SENDRECV       fpSCT14_SendRecv;
    IOCTL_SC_GETINFO            fpSCGetInfo;
    IOCTL_SC_POWEROFF           fpSCPowerOff;
    IOCTL_SC_SETGUARDTIME       fpSCSetGuardTime;
    IOCTL_SC_SETPOWERSTATE      fpSCSetPowerState;
#if defined(MSOS_TYPE_LINUX_KERNEL)
    IOCTL_SC_KDRVKERNELMODEPOLL fpSCKDrvKernelModePoll;
#endif
}SC_INSTANT_PRIVATE;

void SCRegisterToUtopia(FUtopiaOpen ModuleType);
MS_U32 SCOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute);
MS_U32 SCClose(void* pInstance);
MS_U32 SCIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs);
//MS_U32 SCStr(MS_U32 u32PowerState, void* pModule);

SC_Result _MDrv_SC_Init(MS_U8 u8SCID);
SC_Result _MDrv_SC_Exit(MS_U8 u8SCID);
SC_Result _MDrv_SC_Open(MS_U8 u8SCID, MS_U8 u8Protocol, SC_Param* pParam, P_SC_Callback pfSmartNotify);
SC_Result _MDrv_SC_Activate(MS_U8 u8SCID);
SC_Result _MDrv_SC_Deactivate(MS_U8 u8SCID);
SC_Result _MDrv_SC_Close(MS_U8 u8SCID);
SC_Result _MDrv_SC_Reset(MS_U8 u8SCID, SC_Param* pParam);
SC_Result _MDrv_SC_ClearState(MS_U8 u8SCID);
SC_Result _MDrv_SC_GetATR(MS_U8 u8SCID, MS_U32 u32TimeOut,MS_U8 *pu8Atr, MS_U16 *pu16AtrLen, MS_U8 *pu8His, MS_U16 *pu16HisLen);
SC_Result _MDrv_SC_Config(MS_U8 u8SCID, SC_Param* pParam);
SC_Result _MDrv_SC_Send(MS_U8 u8SCID, MS_U8 *pu8SendData, MS_U16 u16SendDataLen, MS_U32 u32TimeoutMs);
SC_Result _MDrv_SC_Recv(MS_U8 u8SCID, MS_U8 *pu8ReadData, MS_U16 *pu16ReadDataLen, MS_U32 u32TimeoutMs);
SC_Result _MDrv_SC_GetCaps(SC_Caps *pCaps);
SC_Result _MDrv_SC_SetPPS(MS_U8 u8SCID, MS_U8 u8SCProtocol, MS_U8 u8Di,MS_U8 u8Fi);
SC_Result _MDrv_SC_PPS(MS_U8 u8SCID);
SC_Result _MDrv_SC_GetStatus(MS_U8 u8SCID, SC_Status *pStatus);
void _MDrv_SC_SetDbgLevel(SC_DbgLv eLevel);
SC_Result _MDrv_SC_RawExchange(MS_U8 u8SCID, MS_U8 *pu8SendData, MS_U16 *u16SendDataLen, MS_U8 *pu8ReadData, MS_U16 *u16ReadDataLen);
MS_BOOL _MDrv_SC_CardVoltage_Config(MS_U8 u8SCID, SC_VoltageCtrl eVoltage);
SC_Result _MDrv_SC_Reset_ATR(MS_U8 u8SCID, SC_Param* pParam, MS_U8 *pu8Atr, MS_U16 *pu16AtrLen, MS_U8 *pu8His, MS_U16 *pu16HisLen);
SC_Result _MDrv_SC_T0_SendRecv(MS_U8 u8SCID, MS_U8 *pu8SendData, MS_U16 u16SendLen, MS_U8 *pu8RecvData, MS_U16 *pu16RecvLen);
SC_Result _MDrv_SC_T1_SendRecv(MS_U8 u8SCID, MS_U8 *pu8SendData, MS_U16 *u16SendDataLen, MS_U8 *pu8ReadData, MS_U16 *u16ReadDataLen);
SC_Result _MDrv_SC_T14_SendRecv(MS_U8 u8SCID, MS_U8 *pu8SendData, MS_U16 u16SendLen, MS_U8 *pu8RecvData, MS_U16 *pu16RecvLen);
SC_Result _MDrv_SC_GetInfo(MS_U8 u8SCID, SC_Info *pstInfo);
SC_Result _MDrv_SC_PowerOff(void);
SC_Result _MDrv_SC_SetGuardTime(MS_U8 u8SCID, MS_U8 u8GuardTime);
MS_U32 _MDrv_SC_SetPowerState(EN_POWER_MODE u16PowerState);

#if defined(MSOS_TYPE_LINUX_KERNEL)
SC_Result _SC_KDrvKernelModePoll(MS_U8 u8SCID, MS_U32 *pu32Event, MS_U32 u32TimeoutMs);
#endif

#ifdef __cplusplus
}
#endif
#endif // _DRVSC_PRIV_H_
