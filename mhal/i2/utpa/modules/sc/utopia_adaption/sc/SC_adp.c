//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2014 - 2016 MStar Semiconductor, Inc. All rights reserved.
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
// Copyright (c) 2014-2016 MStar Semiconductor, Inc.
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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file  SC_adp.c
/// @brief  SC adaption API
/// @author MStar Semiconductor,Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/compat.h>

#include "drvSC_v2.h"
#include "SC_adp.h"

#include "utopia.h"
#include "utopia_adp.h"


UADP_SDT_0_DEF(SC_GENERIC_NULL);

//MS_U8
UADP_SDT_NAMEn_DEF(SC_GENERIC_MS_U8_ADP,0);

//MS_U16
UADP_SDT_NAMEn_DEF(SC_GENERIC_MS_U16_ADP,0);

//MS_U32
UADP_SDT_NAMEn_DEF(SC_GENERIC_MS_U32_ADP,0);

//SC_Caps
UADP_SDT_NAMEn_DEF(SC_CAPS_ADP,0);

//SC_Status
UADP_SDT_NAMEn_DEF(SC_STATUS_ADP,0);

//SC_GENERIC_PARAM
UADP_SDT_NAMEn_DEF(SC_GENERIC_PARAM_ADP,0);

//SC_Param
UADP_SDT_NAMEn_DEF(SC_PARAM_ADP,1);

//SC_Info
UADP_SDT_NAMEn_DEF(SC_INFO_ADP,2);


UADP_SDT_NAMEn_DEF(SC_OPEN_PARAM_ADP,2);
UADP_SDT_NAMEn_DEF(SC_RESET_PARAM_ADP,1);
UADP_SDT_NAMEn_DEF(SC_GETATR_PARAM_ADP,4);
UADP_SDT_NAMEn_DEF(SC_CONFIG_PARAM_ADP,1);
UADP_SDT_NAMEn_DEF(SC_SEND_PARAM_ADP,1);
UADP_SDT_NAMEn_DEF(SC_RECV_PARAM_ADP,2);
UADP_SDT_NAMEn_DEF(SC_GETCAPS_PARAM_ADP,1);
UADP_SDT_NAMEn_DEF(SC_SETPPS_PARAM_ADP,0);
UADP_SDT_NAMEn_DEF(SC_GETSTATUS_PARAM_ADP,1);
UADP_SDT_NAMEn_DEF(SC_SETDBGLEVEL_PARAM_ADP,0);
UADP_SDT_NAMEn_DEF(SC_RAWEXCHANGE_PARAM_ADP,4);
UADP_SDT_NAMEn_DEF(SC_CARDVOLTAGE_CONFIG_PARAM_ADP,0);
UADP_SDT_NAMEn_DEF(SC_KDRVKERNELMODEPOLL_PARAM_ADP,1);
UADP_SDT_NAMEn_DEF(SC_RESET_ATR_PARAM_ADP,5);
UADP_SDT_NAMEn_DEF(SC_T0_SENDRECV_PARAM_ADP,3);
UADP_SDT_NAMEn_DEF(SC_T1_SENDRECV_PARAM_ADP,4);
UADP_SDT_NAMEn_DEF(SC_T14_SENDRECV_PARAM_ADP,3);
UADP_SDT_NAMEn_DEF(SC_GETINFO_PARAM_ADP,1);
UADP_SDT_NAMEn_DEF(SC_SETGUARDTIME_PARAM_ADP,0);
UADP_SDT_NAMEn_DEF(SC_SETPOWERSTATE_PARAM_ADP,0);


MS_U32 SC_adp_Init(FUtopiaIOctl *pIoctl)
{
    UADP_SDT_NAME0(SC_GENERIC_MS_U8_ADP, MS_U8);
    UADP_SDT_NAME0(SC_GENERIC_MS_U16_ADP, MS_U16);
    UADP_SDT_NAME0(SC_GENERIC_MS_U32_ADP, MS_U32);
    UADP_SDT_NAME0(SC_CAPS_ADP, SC_Caps);
    UADP_SDT_NAME0(SC_STATUS_ADP, SC_Status);
    UADP_SDT_NAME0(SC_GENERIC_PARAM_ADP, SC_GENERIC_PARAM);

    UADP_SDT_NAME1(SC_PARAM_ADP, SC_Param, UADP_SDT_AT, pfOCPControl, SC_GENERIC_NULL);

    UADP_SDT_NAME2(SC_INFO_ADP, SC_Info, UADP_SDT_AT, pfEn5V, SC_GENERIC_NULL, UADP_SDT_AT, pfNotify, SC_GENERIC_NULL);

    UADP_SDT_NAME2(SC_OPEN_PARAM_ADP, SC_OPEN_PARAM, UADP_SDT_P2N, pParam, SC_PARAM_ADP, UADP_SDT_AT, pfSmartNotify, SC_GENERIC_NULL);

    UADP_SDT_NAME1(SC_RESET_PARAM_ADP, SC_RESET_PARAM, UADP_SDT_P2N, pParam, SC_PARAM_ADP);

    UADP_SDT_NAME4(SC_GETATR_PARAM_ADP, SC_GETATR_PARAM,
                    UADP_SDT_AT, pu8Atr, SC_GENERIC_NULL,
                    UADP_SDT_P2N, pu16AtrLen, SC_GENERIC_MS_U16_ADP,
                    UADP_SDT_AT, pu8His, SC_GENERIC_NULL,
                    UADP_SDT_P2N, pu16HisLen, SC_GENERIC_MS_U16_ADP);

    UADP_SDT_NAME1(SC_CONFIG_PARAM_ADP, SC_CONFIG_PARAM, UADP_SDT_P2N, pParam, SC_PARAM_ADP);

    UADP_SDT_NAME1(SC_SEND_PARAM_ADP, SC_SEND_PARAM, UADP_SDT_AT, pu8SendData, SC_GENERIC_MS_U8_ADP);

    UADP_SDT_NAME2(SC_RECV_PARAM_ADP, SC_RECV_PARAM, UADP_SDT_AT, pu8ReadData, SC_GENERIC_MS_U8_ADP, UADP_SDT_P2N, pu16ReadDataLen, SC_GENERIC_MS_U16_ADP);

    UADP_SDT_NAME1(SC_GETCAPS_PARAM_ADP, SC_GETCAPS_PARAM, UADP_SDT_P2N, pstCaps, SC_CAPS_ADP);

    UADP_SDT_NAME0(SC_SETPPS_PARAM_ADP, SC_SETPPS_PARAM);

    UADP_SDT_NAME1(SC_GETSTATUS_PARAM_ADP, SC_GETSTATUS_PARAM, UADP_SDT_P2N, pstStatus, SC_STATUS_ADP);

    UADP_SDT_NAME0(SC_SETDBGLEVEL_PARAM_ADP, SC_SETDBGLEVEL_PARAM);

    UADP_SDT_NAME4(SC_RAWEXCHANGE_PARAM_ADP, SC_RAWEXCHANGE_PARAM,
                    UADP_SDT_AT, pu8SendData, SC_GENERIC_MS_U8_ADP,
                    UADP_SDT_P2N, u16SendDataLen, SC_GENERIC_MS_U16_ADP,
                    UADP_SDT_AT, pu8ReadData, SC_GENERIC_MS_U8_ADP,
                    UADP_SDT_P2N, u16ReadDataLen, SC_GENERIC_MS_U16_ADP);

    UADP_SDT_NAME0(SC_CARDVOLTAGE_CONFIG_PARAM_ADP, SC_CARDVOLTAGE_CONFIG_PARAM);

    UADP_SDT_NAME1(SC_KDRVKERNELMODEPOLL_PARAM_ADP, SC_KDRVKERNELMODEPOLL_PARAM, UADP_SDT_P2N, pu32Event, SC_GENERIC_MS_U32_ADP);

    UADP_SDT_NAME5(SC_RESET_ATR_PARAM_ADP, SC_RESET_ATR_PARAM,
                    UADP_SDT_P2N, pParam, SC_PARAM_ADP,
                    UADP_SDT_AT, pu8Atr, SC_GENERIC_NULL,
                    UADP_SDT_P2N, pu16AtrLen, SC_GENERIC_MS_U16_ADP,
                    UADP_SDT_AT, pu8His, SC_GENERIC_NULL,
                    UADP_SDT_P2N, pu16HisLen, SC_GENERIC_MS_U16_ADP);

    UADP_SDT_NAME3(SC_T0_SENDRECV_PARAM_ADP, SC_T0_SENDRECV_PARAM,
                    UADP_SDT_AT, pu8SendData, SC_GENERIC_NULL,
                    UADP_SDT_AT, pu8RecvData, SC_GENERIC_NULL,
                    UADP_SDT_P2N, pu16RecvLen, SC_GENERIC_MS_U16_ADP);

    UADP_SDT_NAME4(SC_T1_SENDRECV_PARAM_ADP, SC_T1_SENDRECV_PARAM,
                    UADP_SDT_AT, pu8SendData, SC_GENERIC_NULL,
                    UADP_SDT_P2N, u16SendDataLen, SC_GENERIC_MS_U16_ADP,
                    UADP_SDT_AT, pu8ReadData, SC_GENERIC_NULL,
                    UADP_SDT_P2N, u16ReadDataLen, SC_GENERIC_MS_U16_ADP);

    UADP_SDT_NAME3(SC_T14_SENDRECV_PARAM_ADP, SC_T14_SENDRECV_PARAM,
                    UADP_SDT_AT, pu8SendData, SC_GENERIC_NULL,
                    UADP_SDT_AT, pu8RecvData, SC_GENERIC_NULL,
                    UADP_SDT_P2N, pu16RecvLen, SC_GENERIC_MS_U16_ADP);

    UADP_SDT_NAME1(SC_GETINFO_PARAM_ADP, SC_GETINFO_PARAM, UADP_SDT_P2N, pstInfo, SC_INFO_ADP);

    UADP_SDT_NAME0(SC_SETGUARDTIME_PARAM_ADP, SC_SETGUARDTIME_PARAM);

    UADP_SDT_NAME0(SC_SETPOWERSTATE_PARAM_ADP, EN_POWER_MODE);

    *pIoctl= (FUtopiaIOctl)SC_adp_Ioctl;

    return 0;
}

MS_U32 SC_adp_Ioctl(void* pInstanceTmp, MS_U32 u32Cmd, void* const pArgs)
{
    MS_U32 u32Ret = 0;
    char argBuffer[32];

    switch(u32Cmd)
    {
        case E_MDRV_CMD_SC_Init:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GENERIC_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_Exit:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GENERIC_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_Open:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_OPEN_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_Activate:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GENERIC_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_Deactivate:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GENERIC_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_Close:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GENERIC_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_Reset:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_RESET_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_ClearState:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GENERIC_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_GetATR:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GETATR_PARAM_ADP, spt_SC_GETATR_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_Conig:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_CONFIG_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_Send:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_SEND_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_Recv:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_RECV_PARAM_ADP, spt_SC_RECV_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_GetCaps:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,NULL, spt_SC_GETCAPS_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_SetPPS:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_SETPPS_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_PPS:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GENERIC_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_GETSTATUS:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GETSTATUS_PARAM_ADP, spt_SC_GETSTATUS_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_SETDBGLEVEL:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_SETDBGLEVEL_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_RAWEXCHANGE:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_RAWEXCHANGE_PARAM_ADP, spt_SC_RAWEXCHANGE_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_CARDVOLTAGE_CONFIG:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_CARDVOLTAGE_CONFIG_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_KDRVKERNELMODEPOLL:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_KDRVKERNELMODEPOLL_PARAM_ADP, spt_SC_KDRVKERNELMODEPOLL_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_RESET_ATR:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_RESET_ATR_PARAM_ADP, spt_SC_RESET_ATR_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_T0_SENDRECV:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_T0_SENDRECV_PARAM_ADP, spt_SC_T0_SENDRECV_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_T1_SENDRECV:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_T1_SENDRECV_PARAM_ADP, spt_SC_T1_SENDRECV_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_T14_SENDRECV:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_T14_SENDRECV_PARAM_ADP, spt_SC_T14_SENDRECV_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_GETINFO:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_GETINFO_PARAM_ADP, spt_SC_GETINFO_PARAM_ADP, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_POWEROFF:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,NULL, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_SETGUARDTIME:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_SETGUARDTIME_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        case E_MDRV_CMD_SC_SETPOWERSTATE:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SC_SETPOWERSTATE_PARAM_ADP, NULL, argBuffer, sizeof(argBuffer));
            break;

        default:
            break;
    }

    return u32Ret;
}
