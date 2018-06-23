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
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
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
/// file   drvDSCMB_tee.c
/// @brief  Descrambler (DSCMB) Driver Interface
/// @author MStar Semiconductor,Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MsCommon.h"
#include "MsVersion.h"
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#include <linux/limits.h>
#else
#include <string.h>
#include <limits.h>
#endif
#include "drvSYS.h"
#include "drvMMIO.h"

#include "drvDSCMB_v2.h"
#include "drvDSCMB_private.h"
#include "tee_client_api.h"

#define DSCMB_TA_MAIN_VERSION 0x00000001
#define DSCMB_TA_SUB_VERSION  0x00000001

#define MSTAR_INTERNAL_DSCMB_UUID {0x41ddc1ac, 0x5b84, 0x4584,{0x93, 0x55, 0x37, 0xf4, 0x3a, 0x23, 0x37, 0xd4} }
#define SYS_TEEC_OPERATION_INITIALIZER { 0 }
TEEC_Context mstar_dscmb_teec_ctx;
TEEC_Session dscmb_session = { 0 };
TEEC_UUID dscmb_uuid = MSTAR_INTERNAL_DSCMB_UUID;
TEEC_Operation dscmb_op = SYS_TEEC_OPERATION_INITIALIZER;
static const char optee_dscmb[] = "opteearmtz00";
char *_dscmb_device = (char *)optee_dscmb;

extern MS_U32 MDrv_SYS_TEEC_InitializeContext(const char *name, TEEC_Context *context);
extern MS_U32 MDrv_SYS_TEEC_Open(TEEC_Context *context, TEEC_Session *session, const TEEC_UUID *destination, MS_U32 connection_method, const void *connection_data, TEEC_Operation *operation, MS_U32 *error_origin);
extern MS_U32 MDrv_SYS_TEEC_InvokeCmd(TEEC_Session *session, MS_U32 cmd_id, TEEC_Operation *operation, MS_U32 *error_origin);
extern void MDrv_SYS_TEEC_Close(TEEC_Session *session);
extern void MDrv_SYS_TEEC_FinalizeContext(TEEC_Context *context);

MS_BOOL _MDrv_DSCMB2_TEE_Open(void)
{
#if defined (MSOS_TYPE_LINUX)
    MS_U32 ret_orig = 0;
    MS_U32 u32Ret = 0;

    memset(&dscmb_op, 0, sizeof(TEEC_Operation));
    dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    u32Ret = MDrv_SYS_TEEC_InitializeContext(_dscmb_device, &mstar_dscmb_teec_ctx);
    if (u32Ret != TEEC_SUCCESS)
    {
        printf("%s %d: Init Context failed, u32Ret=%08x\n",__func__,__LINE__, u32Ret);
        MDrv_SYS_TEEC_FinalizeContext(&mstar_dscmb_teec_ctx);
        return FALSE;
    }

    u32Ret = MDrv_SYS_TEEC_Open(&mstar_dscmb_teec_ctx, &dscmb_session, &dscmb_uuid, TEEC_LOGIN_PUBLIC, NULL, &dscmb_op, &ret_orig);
    if (u32Ret != TEEC_SUCCESS)
    {
        printf("%s %d: TEEC Open session failed, ret_orig=%08x u32Ret=%08x \n",__func__,__LINE__, ret_orig, u32Ret);
        MDrv_SYS_TEEC_Close(&dscmb_session);
        MDrv_SYS_TEEC_FinalizeContext(&mstar_dscmb_teec_ctx);
        return FALSE;
    }

    if(dscmb_op.params[0].value.a  != DSCMB_TA_MAIN_VERSION)
    {
        printf("\033[31m DSCMB TA Version(v%d.%d) is not match (v%d.%d)\n\033[m \n",
            dscmb_op.params[0].value.a,
            dscmb_op.params[0].value.b,
            DSCMB_TA_MAIN_VERSION,
            DSCMB_TA_SUB_VERSION);

        MDrv_SYS_TEEC_Close(&dscmb_session);
        MDrv_SYS_TEEC_FinalizeContext(&mstar_dscmb_teec_ctx);
        return FALSE;
    }

    printf("DSCMB TA Version(v%d.%d)\n",
        dscmb_op.params[0].value.a,
        dscmb_op.params[0].value.b);

    return TRUE;
#else
    return FALSE;
#endif
}

MS_BOOL _MDrv_DSCMB2_TEE_Close(void)
{
#if defined (MSOS_TYPE_LINUX)
    MDrv_SYS_TEEC_Close(&dscmb_session);
    MDrv_SYS_TEEC_FinalizeContext(&mstar_dscmb_teec_ctx);
    return TRUE;
#else
    return FALSE;
#endif
}

MS_BOOL _MDrv_DSCMB2_TEE_Send_CMD(DSCMB_TEE_CMD eCMD,void* para, MS_U32 size)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32Ret = 0;
#if defined (MSOS_TYPE_LINUX)
    MS_U32 ret_orig = 0;
    DSCMB_FLTALLOC*      pstFltAlloc = NULL;
    DSCMB_FLTFREE*       pstFltFree = NULL;
    DSCMB_CONNECTFLT*    pstConnectFlt = NULL;
    DSCMB_DISCONNECTFLT* pstDisconnectFlt = NULL;
    DSCMB_DEFAULT_CAVID* pstDefaultCAVid = NULL;
    DSCMB_TYPESET*       pstTypeSet = NULL;
    DSCMB_FLTKEYSET*     pstFltKeySet = NULL;
    DSCMB_FLTKEYRESET*   pstFltKeyReset = NULL;
    DSCMB_FLTIVSET*      pstFltIVSet = NULL;
    DSCMB_ENGALGO*       pstEngAlgo = NULL;
    DSCMB_ENGKEYFSCB*    pstEngKeyFSCB = NULL;
    DSCMB_ENGKEY*        pstEngKey = NULL;
    DSCMB_ENGRESETKEY*   pstEngResetKey = NULL;
    DSCMB_ENGSWITCH*     pstEngSwitch = NULL;
    DSCMB_ENGFSCB*       pstEngFSCB = NULL;
    DSCMB_ENGIV_EX*      pstEngIV_Ex = NULL;
    DSCMB_ENGENABLEKEY*  pstEngEnableKey = NULL;
    DSCMB_HDCP2_SETRIV_EX*  pstHdcp2SetRiv_Ex = NULL;
    DSCMB_MULTI2_SETROUND*  pstMulti2SetRound = NULL;
    DSCMB_MULTI2_SETSYSKEY* pstMulti2SetSysKey = NULL;
    DSCMB_CAPVR_FLOWSET* pstCapvrFlowset = NULL;
    DSCMB_DUALPATH*      pstDualPath = NULL;
    DSCMB_KL_ATOMICEXEC* pstKLAtomicExec = NULL;
    DSCMB_KL_ETSI*       pstKLETSI       = NULL;
    DSCMB_KLCfg_All*     pstKLCfgAll     = NULL;

    DSCMB_TEE_KL_CFG*    pstTEEKLCfg = NULL;

    MS_U8  u8TmpBuffer[1024];
    MS_U32 u32TmpCopyBufferLen = 0;

    memset(u8TmpBuffer, 0, 1024);
    memset(&dscmb_op, 0, sizeof(TEEC_Operation));

    //deal with parameter input
    switch(eCMD)
    {
        case E_TEE_CMD_DSCMB_FltAlloc:
            pstFltAlloc = (DSCMB_FLTALLOC *)para;
            dscmb_op.params[0].value.a = pstFltAlloc->u32EngId;
            dscmb_op.params[0].value.b = pstFltAlloc->eFltType;
            dscmb_op.params[1].value.a = pstFltAlloc->u32DscmbId;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_FltFree:
            pstFltFree = (DSCMB_FLTFREE *)para;
            dscmb_op.params[0].value.a = pstFltFree->u32EngId;
            dscmb_op.params[0].value.b = pstFltFree->u32DscmbId;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_FltConnectFltId:
            pstConnectFlt = (DSCMB_CONNECTFLT *)para;
            dscmb_op.params[0].value.a = pstConnectFlt->u32EngId;
            dscmb_op.params[0].value.b = pstConnectFlt->u32DscmbId;
            dscmb_op.params[1].value.a = pstConnectFlt->u32DmxFltId;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_FltDisconnectFltId:
            pstDisconnectFlt = (DSCMB_DISCONNECTFLT *)para;
            dscmb_op.params[0].value.a = pstDisconnectFlt->u32EngId;
            dscmb_op.params[0].value.b = pstDisconnectFlt->u32DscmbId;
            dscmb_op.params[1].value.a = pstDisconnectFlt->u32DmxFltId;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_FltTypeSet:
            pstTypeSet = (DSCMB_TYPESET *)para;
            dscmb_op.params[0].value.a = pstTypeSet->u32EngId;
            dscmb_op.params[0].value.b = pstTypeSet->u32DscmbId;
            dscmb_op.params[1].value.a = pstTypeSet->eType;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_FltKeySet:
            pstFltKeySet = (DSCMB_FLTKEYSET *)para;
            dscmb_op.params[0].value.a = pstFltKeySet->u32EngId;
            dscmb_op.params[0].value.b = pstFltKeySet->u32DscmbId;
            dscmb_op.params[1].value.a = pstFltKeySet->eKeyType;
            dscmb_op.params[2].tmpref.buffer  = pstFltKeySet->pu8Key;
            dscmb_op.params[2].tmpref.size  = DSCMB_KEY_LENGTH;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_FltKeyReset:
            pstFltKeyReset = (DSCMB_FLTKEYRESET *)para;
            dscmb_op.params[0].value.a = pstFltKeyReset->u32EngId;
            dscmb_op.params[0].value.b = pstFltKeyReset->u32DscmbId;
            dscmb_op.params[1].value.a = pstFltKeyReset->eKeyType;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_FltIVSet:
            pstFltIVSet = (DSCMB_FLTIVSET *)para;
            dscmb_op.params[0].value.a = pstFltIVSet->u32EngId;
            dscmb_op.params[0].value.b = pstFltIVSet->u32DscmbId;
            dscmb_op.params[1].value.a = pstFltIVSet->eKeyType;
            dscmb_op.params[2].tmpref.buffer = pstFltIVSet->pu8IV;
            dscmb_op.params[2].tmpref.size = DSCMB_IV_LENGTH;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_EngSetAlgo:
            pstEngAlgo = (DSCMB_ENGALGO *)para;
            dscmb_op.params[0].value.a = pstEngAlgo->u32EngId;
            dscmb_op.params[0].value.b = pstEngAlgo->u32DscmbId;
            dscmb_op.params[1].value.a = pstEngAlgo->eEngType;
            dscmb_op.params[1].value.b = pstEngAlgo->stConfig.eMainAlgo;
            dscmb_op.params[2].value.a = pstEngAlgo->stConfig.eSubAlgo;
            dscmb_op.params[2].value.b = pstEngAlgo->stConfig.eSubAlgo;
            dscmb_op.params[3].value.a = pstEngAlgo->stConfig.eResAlgo;
            dscmb_op.params[3].value.b = pstEngAlgo->stConfig.bDecrypt;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_VALUE_INPUT);
            break;

        case E_TEE_CMD_DSCMB_EngSetKey:
            pstEngKey = (DSCMB_ENGKEY *)para;
            dscmb_op.params[0].value.a = pstEngKey->u32EngId;
            dscmb_op.params[0].value.b = pstEngKey->u32DscmbId;
            dscmb_op.params[1].value.a = pstEngKey->eEngType;
            dscmb_op.params[1].value.b = pstEngKey->eKeyType;
            dscmb_op.params[2].tmpref.buffer = pstEngKey->pu8Key;
            dscmb_op.params[2].tmpref.size = DSCMB_KEY_LENGTH;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_EngResetKey:
            pstEngResetKey = (DSCMB_ENGRESETKEY *)para;
            dscmb_op.params[0].value.a = pstEngResetKey->u32EngId;
            dscmb_op.params[0].value.b = pstEngResetKey->u32DscmbId;
            dscmb_op.params[1].value.a = pstEngResetKey->eEngType;
            dscmb_op.params[1].value.b = pstEngResetKey->eKeyType;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_EngSetSwitch:
            pstEngSwitch = (DSCMB_ENGSWITCH *)para;
            dscmb_op.params[0].value.a = pstEngSwitch->u32EngId;
            dscmb_op.params[0].value.b = pstEngSwitch->u32DscmbId;
            dscmb_op.params[1].value.a = pstEngSwitch->eUppSwitch;
            dscmb_op.params[1].value.b = pstEngSwitch->eLowSwitch;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_EngSetFSCB:
            pstEngFSCB = (DSCMB_ENGFSCB *)para;
            dscmb_op.params[0].value.a = pstEngFSCB->u32EngId;
            dscmb_op.params[0].value.b = pstEngFSCB->u32DscmbId;
            dscmb_op.params[1].value.a = pstEngFSCB->eForceSCB;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_EngSetKeyFSCB:
            pstEngKeyFSCB = (DSCMB_ENGKEYFSCB *)para;
            dscmb_op.params[0].value.a = pstEngKeyFSCB->u32EngId;
            dscmb_op.params[0].value.b = pstEngKeyFSCB->u32DscmbId;
            dscmb_op.params[1].value.a = pstEngKeyFSCB->eEngType;
            dscmb_op.params[1].value.b = pstEngKeyFSCB->eKeyType;
            dscmb_op.params[2].value.a = pstEngKeyFSCB->eForceSCB;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_EngSetIV_Ex:
            pstEngIV_Ex = (DSCMB_ENGIV_EX *)para;
            dscmb_op.params[0].value.a = pstEngIV_Ex->u32EngId;
            dscmb_op.params[0].value.b = pstEngIV_Ex->u32DscmbId;
            dscmb_op.params[1].value.a = pstEngIV_Ex->eEngType;
            dscmb_op.params[1].value.b = pstEngIV_Ex->eKeyType;
            dscmb_op.params[2].tmpref.buffer = pstEngIV_Ex->pu8IV;
            dscmb_op.params[2].tmpref.size = DSCMB_IV_LENGTH;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_EngSetRIV:
            pstHdcp2SetRiv_Ex = (DSCMB_HDCP2_SETRIV_EX *)para;
            dscmb_op.params[0].value.a = pstHdcp2SetRiv_Ex->u32EngId;
            dscmb_op.params[0].value.b = pstHdcp2SetRiv_Ex->u32DscmbId;
            dscmb_op.params[1].tmpref.buffer = pstHdcp2SetRiv_Ex->pu8Riv;
            dscmb_op.params[1].tmpref.size = DSCMB_HDCP2_RIV_LENGTH;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_EngEnableKey:
            pstEngEnableKey = (DSCMB_ENGENABLEKEY *)para;
            dscmb_op.params[0].value.a = pstEngEnableKey->u32EngId;
            dscmb_op.params[0].value.b = pstEngEnableKey->u32DscmbId;
            dscmb_op.params[1].value.a = pstEngEnableKey->eEngType;
            dscmb_op.params[1].value.b = pstEngEnableKey->eKeyType;
            dscmb_op.params[2].value.a = pstEngEnableKey->bEnable;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_SetDefaultCAVid:
            pstDefaultCAVid = (DSCMB_DEFAULT_CAVID *)para;
            dscmb_op.params[0].value.a = pstDefaultCAVid->u32EngId;
            dscmb_op.params[0].value.b = pstDefaultCAVid->u32CAVid;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_Multi2_SetRound:
            pstMulti2SetRound = (DSCMB_MULTI2_SETROUND *)para;
            dscmb_op.params[0].value.a = pstMulti2SetRound->u32EngId;
            dscmb_op.params[0].value.b = pstMulti2SetRound->u32Round;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_Multi2_SetSysKey:
            pstMulti2SetSysKey = (DSCMB_MULTI2_SETSYSKEY *)para;
            dscmb_op.params[0].value.a = pstMulti2SetSysKey->u32EngId;
            dscmb_op.params[1].tmpref.buffer = pstMulti2SetSysKey->pu8SysKey;
            dscmb_op.params[1].tmpref.size   = DSCMB_MULTI2_SYSKEY_LENGTH;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);
            break;

//	        case E_TEE_CMD_DSCMB_HDCP2_SetRiv:
//	            pstHdcp2SetRiv = (DSCMB_HDCP2_SETRIV *)para;
//	            dscmb_op.params[0].value.a = pstHdcp2SetRiv->u32EngId;
//	            dscmb_op.params[0].value.b = pstHdcp2SetRiv->pu8Riv;
//	            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
//	            break;

        case E_TEE_CMD_DSCMB_CAPVR_FlowSet:
            pstCapvrFlowset = (DSCMB_CAPVR_FLOWSET *)para;
            dscmb_op.params[0].value.a = pstCapvrFlowset->u32EngId;
            dscmb_op.params[0].value.b = pstCapvrFlowset->eCaMode;
            dscmb_op.params[1].value.a = pstCapvrFlowset->ePvrSrcTsif;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_DualPath_Enable:
        case E_TEE_CMD_DSCMB_DualPath_Disable:
            pstDualPath = (DSCMB_DUALPATH *)para;
            dscmb_op.params[0].value.a = pstDualPath->u32EngId;
            dscmb_op.params[0].value.b = pstDualPath->u32DmxFltId;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_DSCMB_KLadder_AtomicExec:
        case E_TEE_CMD_DSCMB_KLadder_ETSI:
            if(eCMD == E_TEE_CMD_DSCMB_KLadder_AtomicExec)
            {
                pstKLAtomicExec = (DSCMB_KL_ATOMICEXEC *)para;
                pstKLCfgAll = pstKLAtomicExec->KLCfg;
            }
            else
            {
                pstKLETSI = (DSCMB_KL_ETSI *)para;
                pstKLCfgAll = pstKLETSI->KLCfg;
            }

            pstTEEKLCfg     = (DSCMB_TEE_KL_CFG*)u8TmpBuffer;

            //Deal with DSCMB_KLCfg_All
            pstTEEKLCfg->u32Algo    = pstKLCfgAll->eAlgo;
            pstTEEKLCfg->u32Src     = pstKLCfgAll->eSrc;
            pstTEEKLCfg->u32Dst     = pstKLCfgAll->eDst;
            pstTEEKLCfg->u32Outsize = pstKLCfgAll->eOutsize;
            pstTEEKLCfg->u32KeyType = pstKLCfgAll->eKeyType;
            pstTEEKLCfg->u32Level   = pstKLCfgAll->u32Level;
            pstTEEKLCfg->u32EngID   = pstKLCfgAll->u32EngID;
            pstTEEKLCfg->u32DscID   = pstKLCfgAll->u32DscID;

            if(pstKLCfgAll->eSrc == E_DSCMB_KL_SRC_ACPU)
            {
                if(pstKLCfgAll->u8KeyACPU != NULL)
                {
                    pstTEEKLCfg->u32KeyACPU = 0x0000D001;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                pstTEEKLCfg->u32KeyACPU = 0x00000000;
            }

            if(pstKLCfgAll->pu8KeyKLIn != NULL)
            {
                pstTEEKLCfg->u32KeyKLIn = 0x0000D002;
            }
            else
            {
                return FALSE;
            }

            pstTEEKLCfg->bDecrypt   = pstKLCfgAll->bDecrypt;
            pstTEEKLCfg->bInverse   = pstKLCfgAll->bInverse;
            pstTEEKLCfg->u32KLSel   = pstKLCfgAll->eKLSel;
            pstTEEKLCfg->u32CAVid   = pstKLCfgAll->u32CAVid;
            pstTEEKLCfg->u16AppId   = pstKLCfgAll->stKDF.u16AppId;
            pstTEEKLCfg->u32KDFType = pstKLCfgAll->stKDF.eKDFType;
            pstTEEKLCfg->u32HWKeyId = pstKLCfgAll->stKDF.eHWKeyId;
            pstTEEKLCfg->u32FSCB    = pstKLCfgAll->eFSCB;

            u32TmpCopyBufferLen = sizeof(DSCMB_TEE_KL_CFG);
            if(pstKLCfgAll->u8KeyACPU != NULL)
            {
                memcpy(&u8TmpBuffer[u32TmpCopyBufferLen], pstKLCfgAll->u8KeyACPU, DSCMB_KEY_LENGTH);
                u32TmpCopyBufferLen += DSCMB_KEY_LENGTH;
            }

            if(pstKLCfgAll->pu8KeyKLIn != NULL)
            {
                if(pstKLCfgAll->u32Level <= E_DSCMB_KL_TYPE_LV5)
                {
                    memcpy(&u8TmpBuffer[u32TmpCopyBufferLen],
                        pstKLCfgAll->pu8KeyKLIn,
                        DSCMB_KEY_LENGTH *pstKLCfgAll->u32Level);
                    u32TmpCopyBufferLen += (DSCMB_KEY_LENGTH * pstKLCfgAll->u32Level);
                }
                else if(pstKLCfgAll->u32Level == E_DSCMB_KL_TYPE_S5)
                {
                    memcpy(&u8TmpBuffer[u32TmpCopyBufferLen],
                        pstKLCfgAll->pu8KeyKLIn,
                        DSCMB_KEY_LENGTH * 5);
                    u32TmpCopyBufferLen += (DSCMB_KEY_LENGTH * 5);
                }
                else if(pstKLCfgAll->u32Level == E_DSCMB_KL_TYPE_INF)
                {
                    memcpy(&u8TmpBuffer[u32TmpCopyBufferLen],
                        pstKLCfgAll->pu8KeyKLIn,
                        DSCMB_KEY_LENGTH);
                    u32TmpCopyBufferLen += DSCMB_KEY_LENGTH;
                }
            }

             // Deal with ACPU_Out
            u32TmpCopyBufferLen += DSCMB_KEY_LENGTH;

            if(eCMD == E_TEE_CMD_DSCMB_KLadder_ETSI)
            {
                // Deal with pu8Nonce
                memcpy(&u8TmpBuffer[u32TmpCopyBufferLen],
                    pstKLETSI->pu8Nonce,
                    DSCMB_KEY_LENGTH);
                u32TmpCopyBufferLen += DSCMB_KEY_LENGTH;

                // Deal with pu8Response
                u32TmpCopyBufferLen += DSCMB_KEY_LENGTH;
            }

            // Deal with  u32Status
            u32TmpCopyBufferLen += 4;

            dscmb_op.params[0].tmpref.buffer = u8TmpBuffer;
            dscmb_op.params[0].tmpref.size   = u32TmpCopyBufferLen;
            dscmb_op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
            break;

        default:
            MDrv_SYS_TEEC_Close(&dscmb_session);
            MDrv_SYS_TEEC_FinalizeContext(&mstar_dscmb_teec_ctx);
            return bRet;
    }

    //invoke a command
    u32Ret = MDrv_SYS_TEEC_InvokeCmd(&dscmb_session, (MS_U32)eCMD, &dscmb_op, &ret_orig);
    if (u32Ret != TEEC_SUCCESS)
    {
        printf("%s %d: TEEC Invoke command failed, error_origion=%x, u32Ret= %x\n",__func__,__LINE__, ret_orig, u32Ret);
    }
    else
    {
        bRet = TRUE;
    }

    //deal with parameter output
    switch(eCMD)
    {
        case E_TEE_CMD_DSCMB_FltAlloc:
            pstFltAlloc->u32DscmbId = dscmb_op.params[1].value.a;
            break;
        case E_TEE_CMD_DSCMB_KLadder_AtomicExec:
        case E_TEE_CMD_DSCMB_KLadder_ETSI:
            if(eCMD == E_TEE_CMD_DSCMB_KLadder_AtomicExec)
            {
                memcpy(pstKLAtomicExec->u32Status,
                       &((MS_U8*)dscmb_op.params[0].tmpref.buffer)[u32TmpCopyBufferLen - 4],
                       4);

                if(((pstKLCfgAll->eDst ==  E_DSCMB_KL_DST_ACPU) || (pstKLCfgAll->eDst ==  E_DSCMB_KL_DST_CPU_ACPU))
                    && (pstKLAtomicExec->ACPU_Out != NULL))
                {
                    memcpy(pstKLAtomicExec->ACPU_Out,
                           &((MS_U8*)dscmb_op.params[0].tmpref.buffer)[u32TmpCopyBufferLen - 4 - DSCMB_KEY_LENGTH],
                            DSCMB_KEY_LENGTH);
                }
            }
            else
            {
                memcpy(pstKLETSI->u32Status,
                       &((MS_U8*)dscmb_op.params[0].tmpref.buffer)[u32TmpCopyBufferLen - 4],
                       4);

                if(pstKLETSI->pu8Response != NULL )
                {
                    memcpy(pstKLETSI->pu8Response,
                           &((MS_U8*)dscmb_op.params[0].tmpref.buffer)[u32TmpCopyBufferLen - 4 - DSCMB_KEY_LENGTH],
                           DSCMB_KEY_LENGTH);
                }

                if(((pstKLCfgAll->eDst ==  E_DSCMB_KL_DST_ACPU) || (pstKLCfgAll->eDst ==  E_DSCMB_KL_DST_CPU_ACPU))
                    && (pstKLETSI->ACPU_Out != NULL))
                {
                    memcpy(pstKLETSI->ACPU_Out,
                           &((MS_U8*)dscmb_op.params[0].tmpref.buffer)[u32TmpCopyBufferLen - 4 - DSCMB_KEY_LENGTH*3],
                           DSCMB_KEY_LENGTH);
                }
            }



            break;
        default:
            ;
    }


#endif
    return bRet;
}
