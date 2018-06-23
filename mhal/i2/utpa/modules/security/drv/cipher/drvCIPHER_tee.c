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
/// file   drvCIPHER_tee.c
/// @brief  CryptoDMA (CIPHER) Driver Interface
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

#include "drvCIPHER_v2.h"
#include "drvCIPHER_private.h"
#include "tee_client_api.h"

#define CIPHER_TA_MAIN_VERSION 0x00000001
#define CIPHER_TA_SUB_VERSION  0x00000001

#define MSTAR_INTERNAL_CIPHER_UUID  {0x2c4e35d4, 0x1b50, 0x438c, {0xb4, 0x2b, 0x79, 0x7f, 0x3d, 0x5d, 0xcb, 0x4a}}
#define SYS_TEEC_OPERATION_INITIALIZER { 0 }
TEEC_Context mstar_cipher_teec_ctx;
TEEC_Session cipher_session = { 0 };
TEEC_UUID cipher_uuid = MSTAR_INTERNAL_CIPHER_UUID;
TEEC_Operation cipher_op = SYS_TEEC_OPERATION_INITIALIZER;
static const char optee_cipher[] = "opteearmtz00";
char *_cipher_device = (char *)optee_cipher;

extern MS_U32 MDrv_SYS_TEEC_InitializeContext(const char *name, TEEC_Context *context);
extern MS_U32 MDrv_SYS_TEEC_Open(TEEC_Context *context, TEEC_Session *session, const TEEC_UUID *destination, MS_U32 connection_method, const void *connection_data, TEEC_Operation *operation, MS_U32 *error_origin);
extern MS_U32 MDrv_SYS_TEEC_InvokeCmd(TEEC_Session *session, MS_U32 cmd_id, TEEC_Operation *operation, MS_U32 *error_origin);
extern void MDrv_SYS_TEEC_Close(TEEC_Session *session);
extern void MDrv_SYS_TEEC_FinalizeContext(TEEC_Context *context);

MS_BOOL _MDrv_CIPHER_TEE_Open(void)
{
#if defined (MSOS_TYPE_LINUX)
    MS_U32 ret_orig = 0;
    MS_U32 u32Ret = 0;
    memset(&cipher_op, 0, sizeof(TEEC_Operation));

    cipher_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    u32Ret = MDrv_SYS_TEEC_InitializeContext(_cipher_device, &mstar_cipher_teec_ctx);
    if ( u32Ret != TEEC_SUCCESS)
    {
        printf("%s %d: Init Context failed, u32Ret=%08x\n",__func__,__LINE__, u32Ret);
        MDrv_SYS_TEEC_FinalizeContext(&mstar_cipher_teec_ctx);
        return FALSE;
    }

    u32Ret = MDrv_SYS_TEEC_Open(&mstar_cipher_teec_ctx, &cipher_session, &cipher_uuid, TEEC_LOGIN_PUBLIC, NULL, &cipher_op, &ret_orig);
    if (u32Ret != TEEC_SUCCESS)
    {
        printf("%s %d: TEEC Open session failed, ret_orig=%08x u32Ret=%08x \n",__func__,__LINE__, ret_orig, u32Ret);
        MDrv_SYS_TEEC_Close(&cipher_session);
        MDrv_SYS_TEEC_FinalizeContext(&mstar_cipher_teec_ctx);
        return FALSE;
    }

    if(cipher_op.params[0].value.a  != CIPHER_TA_MAIN_VERSION)
    {
        printf("\033[31m CIPHER TA Version(v%d.%d) is not match (v%d.%d)\n\033[m \n",
            cipher_op.params[0].value.a,
            cipher_op.params[0].value.b,
            CIPHER_TA_MAIN_VERSION,
            CIPHER_TA_SUB_VERSION);

        MDrv_SYS_TEEC_Close(&cipher_session);
        MDrv_SYS_TEEC_FinalizeContext(&mstar_cipher_teec_ctx);
        return FALSE;
    }

    printf("CIPHER TA Version(v%d.%d)\n",
        cipher_op.params[0].value.a,
        cipher_op.params[0].value.b);

    return TRUE;
#else
    return FALSE;
#endif
}

MS_BOOL _MDrv_CIPHER_TEE_Close(void)
{
#if defined (MSOS_TYPE_LINUX)
    MDrv_SYS_TEEC_Close(&cipher_session);
    MDrv_SYS_TEEC_FinalizeContext(&mstar_cipher_teec_ctx);
    return TRUE;
#else
    return FALSE;
#endif
}

MS_BOOL _MDrv_CIPHER_TEE_Send_CMD(CIPHER_TEE_CMD eCMD,void* para, MS_U32 size)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32Ret = 0;
#if defined (MSOS_TYPE_LINUX)
    MS_U32 ret_orig = 0;
    CIPHER_DMACALC*        pstDMACalc       = NULL;
    CIPHER_HASHCALC*       pstHashCalc      = NULL;
    CIPHER_HMACCALC*       pstHmacCalc    = NULL;
    CIPHER_ISDMADONE*      pstIsDMADone   = NULL;
    CIPHER_ISHASHDONE*     pstIsHASHDone  = NULL;

    CIPHER_TEE_DMA_CFG*        pstTEEDmaCfg   = NULL;
    CIPHER_TEE_HASH_CFG*       pstTEEHashCfg  = NULL;
    CIPHER_TEE_HMAC_CFG*       pstTEEHmacCfg  = NULL;

    MS_U8  u8TmpBuffer[1024];
    MS_U32 u32TmpCopyBufferLen = 0;

    memset(u8TmpBuffer, 0, 1024);
    memset(&cipher_op, 0, sizeof(TEEC_Operation));
//	    printf("=============[CIPHER]==================CMD=%x\n", eCMD);
    //deal with parameter input
    switch(eCMD)
    {
        case E_TEE_CMD_CIPHER_DMACalc:
        /*
                params[0]: TEEC_MEMREF_TEMP_INPUT     -  config CIPHER_TEE_DMA_CFG
                params[1]: TEEC_VALUE_OUTPUT             - status
                            *pstDMACalc->pu32CmdId = cipher_op.params[1].value.a
                              pstDMACalc->ret            = cipher_op.params[1].value.b
                params[2]: TEEC_MEMREF_TEMP_INPUT    - input data buffer
                params[3]: TEEC_MEMREF_TEMP_OUTPUT - output data buffer
            */
            pstDMACalc = (CIPHER_DMACALC *)para;
            pstTEEDmaCfg = (CIPHER_TEE_DMA_CFG* )u8TmpBuffer;
            u32TmpCopyBufferLen = sizeof(CIPHER_TEE_DMA_CFG);

            pstTEEDmaCfg->u32MainAlgo         = pstDMACalc->stCfg.stAlgo.eMainAlgo;
            pstTEEDmaCfg->u32SubAlgo          = pstDMACalc->stCfg.stAlgo.eSubAlgo;
            pstTEEDmaCfg->u32ResAlgo          = pstDMACalc->stCfg.stAlgo.eResAlgo;
            pstTEEDmaCfg->u32SBAlgo           = pstDMACalc->stCfg.stAlgo.eSBAlgo;
            pstTEEDmaCfg->u32KeySrc           = pstDMACalc->stCfg.stKey.eKeySrc;
            pstTEEDmaCfg->u8KeyIdx            = pstDMACalc->stCfg.stKey.u8KeyIdx;
            pstTEEDmaCfg->u8KeyLen            = pstDMACalc->stCfg.stKey.u8KeyLen;
            pstTEEDmaCfg->u8IVLen             = pstDMACalc->stCfg.stKey.u8IVLen;
            pstTEEDmaCfg->bDecrypt            = pstDMACalc->stCfg.bDecrypt;
            if(pstDMACalc->stCfg.stKey.pu8KeyData != NULL)
            {
                pstTEEDmaCfg->u32KeyData      = u32TmpCopyBufferLen;
                memcpy(&u8TmpBuffer[u32TmpCopyBufferLen],
                    pstDMACalc->stCfg.stKey.pu8KeyData,
                    pstDMACalc->stCfg.stKey.u8KeyLen);

                u32TmpCopyBufferLen += pstDMACalc->stCfg.stKey.u8KeyLen;
            }
            if(pstDMACalc->stCfg.stKey.pu8IVData != NULL)
            {
                pstTEEDmaCfg->u32IVData      = u32TmpCopyBufferLen;
                memcpy(&u8TmpBuffer[u32TmpCopyBufferLen],
                    pstDMACalc->stCfg.stKey.pu8IVData,
                    pstDMACalc->stCfg.stKey.u8IVLen);

                u32TmpCopyBufferLen += pstDMACalc->stCfg.stKey.u8IVLen;
            }

            pstTEEDmaCfg->u64InputAddr        = pstDMACalc->stCfg.stInput.u32Addr;
            pstTEEDmaCfg->u32InputSize        = pstDMACalc->stCfg.stInput.u32Size;
            pstTEEDmaCfg->u32InputMemoryType  = pstDMACalc->stCfg.stInput.eMemoryType;
            pstTEEDmaCfg->u64OutputAddr       = pstDMACalc->stCfg.stOutput.u32Addr;
            pstTEEDmaCfg->u32OutputSize       = pstDMACalc->stCfg.stOutput.u32Size;
            pstTEEDmaCfg->u32OutputMemoryType = pstDMACalc->stCfg.stOutput.eMemoryType;
            pstTEEDmaCfg->u32CAVid            = pstDMACalc->stCfg.u32CAVid;

            cipher_op.params[0].tmpref.buffer = u8TmpBuffer;
            cipher_op.params[0].tmpref.size   = u32TmpCopyBufferLen;

            cipher_op.params[2].tmpref.buffer = (void*)MsOS_PA2KSEG1((MS_PHY)pstDMACalc->stCfg.stInput.u32Addr);
            cipher_op.params[2].tmpref.size   = pstDMACalc->stCfg.stInput.u32Size;

            cipher_op.params[3].tmpref.buffer = (void*)MsOS_PA2KSEG1((MS_PHY)pstDMACalc->stCfg.stOutput.u32Addr);
            cipher_op.params[3].tmpref.size   = pstDMACalc->stCfg.stOutput.u32Size;
            cipher_op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_OUTPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT);

            break;

        case E_TEE_CMD_CIPHER_HASH:
        /*
                params[0]: TEEC_MEMREF_TEMP_INOUT     -  config CIPHER_TEE_HASH_CFG
                params[1]: TEEC_VALUE_OUTPUT             - status
                        *pstHashCalc->pu32CmdId = cipher_op.params[1].value.a
                        pstHashCalc->ret              = cipher_op.params[1].value.b
                params[2]: TEEC_MEMREF_TEMP_INPUT    - input data buffer
                params[3]: TEEC_NONE
            */
            pstHashCalc = (CIPHER_HASHCALC *)para;
            pstTEEHashCfg = (CIPHER_TEE_HASH_CFG* )u8TmpBuffer;
            u32TmpCopyBufferLen = sizeof(CIPHER_TEE_HASH_CFG);

            pstTEEHashCfg->u32Algo            = pstHashCalc->stCfg.eAlgo;
            pstTEEHashCfg->u64InputAddr       = pstHashCalc->stCfg.stInput.u32Addr;
            pstTEEHashCfg->u32InputSize       = pstHashCalc->stCfg.stInput.u32Size;
            pstTEEHashCfg->u32InputMemoryType = pstHashCalc->stCfg.stInput.eMemoryType;
            if(pstHashCalc->stCfg.pu8Digest_Buf != NULL)
            {
                pstTEEHashCfg->u32Digest_Buf  = u32TmpCopyBufferLen;
                u32TmpCopyBufferLen += pstHashCalc->stCfg.u32Digest_Buf_Size;
            }
            else
            {
                pstHashCalc->ret        =  DRV_CIPHER_BAD_PARAM;
                return FALSE;
            }

            pstTEEHashCfg->u32Digest_Buf_Size = pstHashCalc->stCfg.u32Digest_Buf_Size;
            pstTEEHashCfg->u32CAVid           = pstHashCalc->stCfg.u32CAVid;

            cipher_op.params[0].tmpref.buffer = u8TmpBuffer;
            cipher_op.params[0].tmpref.size   = u32TmpCopyBufferLen;

            cipher_op.params[2].tmpref.buffer = (void*)MsOS_PA2KSEG1((MS_PHY)pstHashCalc->stCfg.stInput.u32Addr);
            cipher_op.params[2].tmpref.size   = pstHashCalc->stCfg.stInput.u32Size;

            cipher_op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_VALUE_OUTPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE);
            break;

        case E_TEE_CMD_CIPHER_HMAC:
        /*
                params[0]: TEEC_MEMREF_TEMP_INOUT     -  config CIPHER_TEE_HMAC_CFG
                params[1]: TEEC_VALUE_OUTPUT              - status
                            *pstHmacCalc->pu32CmdId = cipher_op.params[1].value.a
                            pstHmacCalc->ret              = cipher_op.params[1].value.b
                params[2]: TEEC_MEMREF_TEMP_INPUT    - input data buffer
                params[3]: TEEC_NONE
            */
            pstHmacCalc = (CIPHER_HMACCALC *)para;
            pstTEEHmacCfg = (CIPHER_TEE_HMAC_CFG* )u8TmpBuffer;
            u32TmpCopyBufferLen = sizeof(CIPHER_TEE_HMAC_CFG);

            pstTEEHmacCfg->u32Algo            = pstHmacCalc->stCfg.eAlgo;
            pstTEEHmacCfg->u64InputAddr       = pstHmacCalc->stCfg.stInput.u32Addr;
            pstTEEHmacCfg->u32InputSize       = pstHmacCalc->stCfg.stInput.u32Size;
            pstTEEHmacCfg->u32InputMemoryType = pstHmacCalc->stCfg.stInput.eMemoryType;
            if(pstHmacCalc->stCfg.pu8Digest_Buf != NULL)
            {
                pstTEEHmacCfg->u32Digest_Buf  = u32TmpCopyBufferLen;
                u32TmpCopyBufferLen += pstHmacCalc->stCfg.u32Digest_Buf_Size;
            }
            else
            {
                pstHmacCalc->ret        =  DRV_CIPHER_BAD_PARAM;

                return FALSE;
            }

            pstTEEHmacCfg->u32Digest_Buf_Size = pstHmacCalc->stCfg.u32Digest_Buf_Size;
            pstTEEHmacCfg->u32CAVid           = pstHmacCalc->stCfg.u32CAVid;

            pstTEEHmacCfg->u32KeySrc          = pstHmacCalc->stCfg.stHMACKey.eKeySrc;
            pstTEEHmacCfg->u8KeyIdx           = pstHmacCalc->stCfg.stHMACKey.u8KeyIdx;
            pstTEEHmacCfg->u8KeyLen           = pstHmacCalc->stCfg.stHMACKey.u8KeyLen;

            if(pstHmacCalc->stCfg.stHMACKey.pu8KeyData != NULL)
            {
                pstTEEHmacCfg->u32KeyData      = u32TmpCopyBufferLen;
                memcpy(&u8TmpBuffer[u32TmpCopyBufferLen],
                    pstHmacCalc->stCfg.stHMACKey.pu8KeyData,
                    pstHmacCalc->stCfg.stHMACKey.u8KeyLen);

                u32TmpCopyBufferLen += pstHmacCalc->stCfg.stHMACKey.u8KeyLen;
            }

            cipher_op.params[0].tmpref.buffer = u8TmpBuffer;
            cipher_op.params[0].tmpref.size   = u32TmpCopyBufferLen;

            cipher_op.params[2].tmpref.buffer = (void*)MsOS_PA2KSEG1((MS_PHY)pstHmacCalc->stCfg.stInput.u32Addr);
            cipher_op.params[2].tmpref.size   = pstHmacCalc->stCfg.stInput.u32Size;

            cipher_op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_VALUE_OUTPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE);
            break;

        case E_TEE_CMD_CIPHER_IsDMADone:
        /*
                params[0]: TEEC_VALUE_INPUT     -
                         cipher_op.params[0].value.a   = pstIsDMADone->u32CmdId
                params[1]: TEEC_VALUE_OUTPUT  -
                        *pstIsDMADone->pu32Exception = cipher_op.params[1].value.a
                params[2]: TEEC_NONE
                params[3]: TEEC_NONE
            */
            pstIsDMADone = (CIPHER_ISDMADONE *)para;
            cipher_op.params[0].value.a   = pstIsDMADone->u32CmdId;
            cipher_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE);
            break;

        case E_TEE_CMD_CIPHER_IsHASHDone:
        /*
                params[0]: TEEC_VALUE_INPUT     -
                         cipher_op.params[0].value.a   = pstIsHASHDone->u32CmdId
                params[1]: TEEC_VALUE_OUTPUT  -
                        *pstIsHASHDone->pu32Exception = cipher_op.params[1].value.a
                params[2]: TEEC_NONE
                params[3]: TEEC_NONE
            */
            pstIsHASHDone = (CIPHER_ISHASHDONE *)para;
            cipher_op.params[0].value.a   = pstIsHASHDone->u32CmdId;
            cipher_op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE);
            break;

        default:
            MDrv_SYS_TEEC_Close(&cipher_session);
            MDrv_SYS_TEEC_FinalizeContext(&mstar_cipher_teec_ctx);
            return bRet;
    }

    //invoke a command
    u32Ret = MDrv_SYS_TEEC_InvokeCmd(&cipher_session, (MS_U32)eCMD, &cipher_op, &ret_orig);
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
        case E_TEE_CMD_CIPHER_DMACalc:
            *pstDMACalc->pu32CmdId = cipher_op.params[1].value.a;
            pstDMACalc->ret        = cipher_op.params[1].value.b;
            break;

        case E_TEE_CMD_CIPHER_HASH:
            *pstHashCalc->pu32CmdId = cipher_op.params[1].value.a;
            pstHashCalc->ret        = cipher_op.params[1].value.b;
            if(bRet == TRUE)
            {
                memcpy(pstHashCalc->stCfg.pu8Digest_Buf,
                    &((MS_U8*)cipher_op.params[0].tmpref.buffer)[pstTEEHashCfg->u32Digest_Buf],
                    pstHashCalc->stCfg.u32Digest_Buf_Size);
            }

            break;

        case E_TEE_CMD_CIPHER_HMAC:
            *pstHmacCalc->pu32CmdId = cipher_op.params[1].value.a;
            pstHmacCalc->ret        = cipher_op.params[1].value.b;

            if(bRet == TRUE)
            {
                memcpy(pstHmacCalc->stCfg.pu8Digest_Buf,
                    &((MS_U8*)cipher_op.params[0].tmpref.buffer)[pstTEEHmacCfg->u32Digest_Buf],
                    pstHmacCalc->stCfg.u32Digest_Buf_Size);
            }

            break;

        case E_TEE_CMD_CIPHER_IsDMADone:
            *pstIsDMADone->pu32Exception = cipher_op.params[1].value.a;
            break;

        case E_TEE_CMD_CIPHER_IsHASHDone:
            *pstIsHASHDone->pu32Exception = cipher_op.params[1].value.a;
            break;

        default:
            ;
    }

    if(u32Ret == TEEC_ERROR_TARGET_DEAD)
    {
        printf("Reload CIPHER TA\n");
        MDrv_SYS_TEEC_Close(&cipher_session);
        MDrv_SYS_TEEC_FinalizeContext(&mstar_cipher_teec_ctx);
        _MDrv_CIPHER_TEE_Open();
    }
#endif
    return bRet;
}
