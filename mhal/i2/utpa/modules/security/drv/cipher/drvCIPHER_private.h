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

#ifndef _DRVCIPHER_PRIV_H_
#define _DRVCIPHER_PRIV_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    E_CIPHER_RESOURCE,
} eCipherResourceId;

typedef struct _CIPHER_RESOURCE_PRIVATE
{
    MS_BOOL       bInited_Drv;
    MS_U16        u16ThreadID;
    SYS_TEEINFO   stTeeInfo;
    MS_BOOL       bEnableTEE;
} CIPHER_RESOURCE_PRIVATE;


typedef struct _CIPHER_INSTANT_PRIVATE
{

} CIPHER_INSTANT_PRIVATE;

typedef struct DLL_PACKED _CIPHER_TEE_DMA_CFG
{
    MS_U32 u32MainAlgo;
    MS_U32 u32SubAlgo;
    MS_U32 u32ResAlgo;
    MS_U32 u32SBAlgo;
    MS_U32 u32KeySrc;
    MS_U8  u8KeyIdx;
    MS_U8  u8KeyLen;
    MS_U8  u8IVLen;
    MS_BOOL bDecrypt;
    MS_U32 u32KeyData;
    MS_U32 u32IVData;
    MS_U64 u64InputAddr;
    MS_U32 u32InputSize;
    MS_U32 u32InputMemoryType;
    MS_U64 u64OutputAddr;
    MS_U32 u32OutputSize;
    MS_U32 u32OutputMemoryType;
    MS_U32 u32CAVid;
} CIPHER_TEE_DMA_CFG;

typedef struct DLL_PACKED _CIPHER_TEE_HASH_CFG
{
    MS_U32 u32Algo;
    MS_U32 u32CAVid;
    MS_U64 u64InputAddr;
    MS_U32 u32InputSize;
    MS_U32 u32InputMemoryType;
    MS_U32 u32Digest_Buf;
    MS_U32 u32Digest_Buf_Size;
} CIPHER_TEE_HASH_CFG;

typedef struct DLL_PACKED _CIPHER_TEE_HMAC_CFG
{
    MS_U32 u32Algo;
    MS_U32 u32CAVid;
    MS_U64 u64InputAddr;
    MS_U32 u32InputSize;
    MS_U32 u32InputMemoryType;
    MS_U32 u32Digest_Buf;
    MS_U32 u32Digest_Buf_Size;
    MS_U32 u32KeySrc;
    MS_U8  u8KeyIdx;
    MS_U8  u8KeyLen;
    MS_U16 u16Reserved;
    MS_U32 u32KeyData;
}CIPHER_TEE_HMAC_CFG;


typedef enum {
    E_TEE_CMD_CIPHER_DMACalc = 0x10,
    E_TEE_CMD_CIPHER_HASH,
    E_TEE_CMD_CIPHER_HMAC,
    E_TEE_CMD_CIPHER_IsDMADone,
    E_TEE_CMD_CIPHER_IsHASHDone,
} CIPHER_TEE_CMD;


// TEE Function
MS_BOOL _MDrv_CIPHER_TEE_Open(void);
MS_BOOL _MDrv_CIPHER_TEE_Close(void);
MS_BOOL _MDrv_CIPHER_TEE_Send_CMD(CIPHER_TEE_CMD eCMD,void* para,MS_U32 size);
DRV_CIPHER_RET _MDrv_CIPHER_TEE_DMACalc(DRV_CIPHER_DMACFG stCfg, MS_U32 *pu32CmdId);
DRV_CIPHER_RET _MDrv_CIPHER_TEE_HASH(DRV_CIPHER_HASHCFG stCfg, MS_U32 *pu32CmdId);
DRV_CIPHER_RET _MDrv_CIPHER_TEE_HMAC(DRV_CIPHER_HMACCFG stCfg, MS_U32 *pu32CmdId);
MS_BOOL _MDrv_CIPHER_TEE_IsDMADone(MS_U32 u32CmdId, MS_U32 *pu32Exception);
MS_BOOL _MDrv_CIPHER_TEE_IsHASHDone(MS_U32 u32CmdId, MS_U32 *pu32Exception);
MS_BOOL _MDrv_CIPHER_IsSupportTEE(void);


MS_BOOL _MDrv_CIPHER_IsDMADone(MS_U32 u32CmdId, MS_U32 *pu32Exception);
MS_BOOL _MDrv_CIPHER_IsHASHDone(MS_U32 u32CmdId, MS_U32 *pu32Exception);

DRV_CIPHER_RET _MDrv_CIPHER_Init(void);
DRV_CIPHER_RET _MDrv_CIPHER_InitBySWFlag(MS_BOOL bSWInit);
DRV_CIPHER_RET _MDrv_CIPHER_ResetKey(MS_U32 u32CAVid, MS_U32 u32KeyIdx);
DRV_CIPHER_RET _MDrv_CIPHER_Reset(void);
DRV_CIPHER_RET _MDrv_CIPHER_SetDbgLevel(CIPHER_DBGMSG_LEVEL eDBGMsgLevel);
DRV_CIPHER_RET _MDrv_CIPHER_DMACalc(DRV_CIPHER_DMACFG stCfg, MS_U32 *pu32CmdId);
DRV_CIPHER_RET _MDrv_CIPHER_HASH(DRV_CIPHER_HASHCFG stCfg, MS_U32 *pu32CmdId);

#ifdef __cplusplus
}
#endif
#endif // _DRVCIPHER_PRIV_H_
