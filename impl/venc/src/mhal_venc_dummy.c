//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2006 - 2015 MStar Semiconductor, Inc. All rights reserved.
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
// Copyright (c) 2006-2015 MStar Semiconductor, Inc.
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

//#include "mi_common_datatype.h"
//#include "MsTypes.h"
//TODO remove this later. Now it's for vmap
#include "mi_sys_internal.h"

#include "mhal_venc.h"
#include <linux/version.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30) //i2
#include "mhal_mfe.h"
#include "mhal_mhe.h"
#include "mhal_jpe.h"
#endif
#include "mi_venc_internal.h"

#include "mi_print.h"
#include <linux/string.h>


#define UNUSED(x) (x) = (x)

#define MHAL_VENC_IS_PARAM_CORRECT_VER(pstParam, u32Ver)\
    ((pstParam->stVerCtl.u32Size == sizeof(*pstParam)) && \
            (pstParam->stVerCtl.u32Version == u32Ver))

#define MAX_DUMMY_INSTANCE (16) //should == MI_VENC_MAX_CHN_NUM_PER_MODULE

#if 0
__inline MS_S32 MHAL_VENC_CheckParamIntVer(MHAL_VENC_Version_t *pParam, MS_U32 u32Size, MS_U32 u32Ver)
{
    if ()
}
#endif

typedef struct MHAL_VENC_DummyInstance_s
{
    MS_U32 u32LastSize;
    MS_U32 u32FrameId;
    MS_BOOL bUsed;
} MHAL_VENC_DummyInstance_t;

typedef struct MHAL_VENC_Dummy_Res_s {
    MHAL_VENC_DummyInstance_t astInst[MAX_DUMMY_INSTANCE];
} MHAL_VENC_Dummy_Dev_t;

static MHAL_VENC_Dummy_Dev_t _stDev[E_MI_VENC_DEV_MAX];

//returns MHAL_ErrCode_e
//MS_S32 MHAL_VENC_DummyCreateDevice( void *pOsDev, void** ppBase,
//        int *pSize, MHAL_VENC_DEV_HANDLE *phDev)
MS_S32 MHAL_VENC_DummyCreateDevice(MS_U32 u32CoreId, MHAL_VENC_DEV_HANDLE *phDev)
{
    MHAL_VENC_Dummy_Dev_t *pstDev;
    //UNUSED(ppBase);
    //UNUSED(pOsDev);
    //UNUSED(pSize);
    //DBG_INFO("This is dummy\n");
    if (phDev == NULL)
    {
        return -1;//E_MHAL_ERR_NULL_PTR;
    }
    //Every device uses dummy for now, otherwise multiple CreateDevices are needed.
    pstDev = _stDev + E_MI_VENC_DEV_DUMMY;
    *phDev = (MHAL_VENC_DEV_HANDLE)pstDev;
    if (pstDev)
    {
        int i;
        for (i = 0; i < MAX_DUMMY_INSTANCE; ++i) {
            pstDev->astInst[i].bUsed = FALSE;
        }
    }
    return 0;
}

MS_S32 MHAL_VENC_DummyGetDevConfig(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_IDX type, MHAL_VENC_Param_t* pstParam)
{
    MHAL_VENC_ParamInt_t *pstParamInt;
    if (hDev == NULL) {
        return -1;
    }

    switch (type) {
    case E_MHAL_VENC_HW_IRQ_NUM: //MHAL_VENC_PARAM_Int_t
        pstParamInt = (MHAL_VENC_ParamInt_t *)pstParam;
        if (!MHAL_VENC_IS_PARAM_CORRECT_VER(pstParamInt, MHAL_VENC_ParamInt_t_ver)) {
            return -2;
        }
        pstParamInt->u32Val = MHAL_VENC_DUMMY_IRQ;
        break;
    case E_MHAL_VENC_HW_CMDQ_BUF_LEN: //MHAL_VENC_PARAM_Int_t
        pstParamInt = (MHAL_VENC_ParamInt_t *)pstParam;
        if (!MHAL_VENC_IS_PARAM_CORRECT_VER(pstParamInt, MHAL_VENC_ParamInt_t_ver)) {
            return -2;
        }
        pstParamInt->u32Val = 0;
        break;
    case E_MHAL_VENC_HW_RESOLUTION_ALIGN:
        break;
    default:
        return -3;//not support
        break;
    }
    return 0;
}

MS_S32 MHAL_VENC_DummyDestroyDevice(MHAL_VENC_DEV_HANDLE hDev)
{
    if (hDev == NULL) {
        return -1;
    }

    return 0;
}

MS_S32 MHAL_VENC_DummyCreateInstance(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_INST_HANDLE *phInst)
{
    MHAL_VENC_Dummy_Dev_t *pstDev = (MHAL_VENC_Dummy_Dev_t *)hDev;
    int i;

    if (hDev == NULL || phInst == NULL)
        return -1;

    for (i = 0; i < MAX_DUMMY_INSTANCE; ++i) {
        if (pstDev->astInst[i].bUsed == FALSE)
        { //found available
            *phInst = &pstDev->astInst[i];
            pstDev->astInst[i].u32FrameId = 0;
            pstDev->astInst[i].u32LastSize = 0;
            pstDev->astInst[i].bUsed = TRUE;
            return 0;
        }
    }
    return -3;
}

MS_S32 MHAL_VENC_DummyDestroyInstance(MHAL_VENC_INST_HANDLE hInst)
{
    MHAL_VENC_DummyInstance_t *pstInst = (MHAL_VENC_DummyInstance_t*)hInst;
    if (hInst == NULL)
        return -1;

    pstInst->bUsed = FALSE;
    return 0;
}

MS_S32 MHAL_VENC_DummyQueryBufSize(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InternalBuf_t *pstSize)
{
    if (hInst == NULL || pstSize == NULL)
        return -1;

    //any small size
    pstSize->u32IntrAlBufSize = 100;
    pstSize->u32IntrRefBufSize = 200;
    return 0;
}

MS_S32 MHAL_VENC_DummySetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX type, MHAL_VENC_Param_t* pstParam)
{
    //MHAL_VENC_ParamInt_t *pstParamInt;
    if (hInst == NULL) {
        return -1;
    }

    switch (type) {
    //these are skipped and accepted
    case E_MHAL_VENC_265_RESOLUTION:
    case E_MHAL_VENC_265_RC:
    case E_MHAL_VENC_264_RESOLUTION:
    case E_MHAL_VENC_264_RC:
    case E_MHAL_VENC_JPEG_RESOLUTION:
    break;
    case E_MHAL_VENC_IDX_STREAM_ON: //VencInternalBuf, CFG
        {
            MHAL_VENC_InternalBuf_t *pstInternalBuf;
            pstInternalBuf = (MHAL_VENC_InternalBuf_t *) pstParam;
            if (!MHAL_VENC_IS_PARAM_CORRECT_VER(pstInternalBuf, MHAL_VENC_InternalBuf_t_ver)) {
                return -2;
            }
        }
        break;
    case E_MHAL_VENC_IDX_STREAM_OFF: //RT
        //does not need pstParam
        break;
    default:
        return -3;//not support
        break;
    }
    return 0;
}

MS_S32 MHAL_VENC_DummyGetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX type, MHAL_VENC_Param_t* pstParam)
{
    //MHAL_VENC_ParamInt_t *pstParamInt;
    if (hInst == NULL) {
        return -1;
    }

    switch (type) {
    case E_MHAL_VENC_264_ROI: //VencInternalBuf, CFG
        {
            MHAL_VENC_RoiCfg_t *pstRoiCfg;
            pstRoiCfg = (MHAL_VENC_RoiCfg_t *) pstParam;
            if (!MHAL_VENC_IS_PARAM_CORRECT_VER(pstRoiCfg, MHAL_VENC_RoiCfg_t_ver)) {
                return -2;
            }
            pstRoiCfg->stAttrRoiArea.bEnable = FALSE; //dummy return value
        }
        break;
    case E_MHAL_VENC_IDX_STREAM_OFF: //RT
        //does not need pstParam
        break;
    default:
        return -3;//not support
        break;
    }
    return 0;
}

static MS_S32 _MHAL_VENC_DummyCheckParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pInOutBuf)
{
    if (hInst == NULL || pInOutBuf == NULL)
        return -1;


    if (pInOutBuf->pCmdQ)
        return -3; //not support command Q for this encoder

    return 0;
}

/**
 *
 * @param[in] pInOutBuf For dummy encoder because CPU write the bit stream. MHAL_VENC_InOutBuf_t will be used
 *  as the following
    MS_PHYADDR phyInputYUVBuf1;       //input PHY
    MS_U32     u32InputYUVBuf1Size;   //input size
    MS_PHYADDR phyInputYUVBuf2;       //input VA
    MS_U32     u32InputYUVBuf2Size;
    MS_PHYADDR phyInputYUVBuf3;       //output VA
    MS_U32     u32InputYUVBuf3Size;
    MS_PHYADDR phyOutputBuf;          //output PHY
    MS_U32     u32OutputBufSize;      //output size
 * @param[out] pIn
 * @param[out] pOut
 * @return <0  error
 * @retval 0 OK
 * @retval 1 OK and mapped
 */
static MS_S32 _MHAL_VENC_DummyGetInOutAddr(MHAL_VENC_InOutBuf_t* pInOutBuf, void **pIn, void **pOut)
{
    MS_BOOL bMap = FALSE;
    if (pInOutBuf == NULL)
        return -14;

    //DBG_WRN("input %llX\n", pInOutBuf->phyInputYUVBuf2);
    //DBG_WRN("input %X\n", (MS_U32)pInOutBuf->phyInputYUVBuf2);
    *pIn = (void*)(MS_U32)(pInOutBuf->phyInputYUVBuf2);
    *pOut = (void*)(MS_U32)(pInOutBuf->phyInputYUVBuf3);
#if 0
    if (*pIn == NULL)
        return -15;
    if (*pOut == NULL)
        return -16;
#else
    if ((*pIn == NULL) || (*pOut == NULL))
    {
        bMap = TRUE;
        *pIn = mi_sys_Vmap(pInOutBuf->phyInputYUVBuf1, 128, FALSE);
        if (*pIn == NULL)
        {
            return -4;
        }
        *pOut = mi_sys_Vmap(pInOutBuf->phyOutputBuf, 128, FALSE);
        if (*pOut == NULL)
        {
            mi_sys_UnVmap(*pIn);
            return -6;
        }
    }
#endif

    return bMap;
}



MS_S32 MHAL_VENC_DummyEncodeOneFrame(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pInOutBuf)
{
    MS_S32 s32Ret = -100;
    char *szYuv;
    char *szEs;
    char *szEsTmp;
    MHAL_VENC_DummyInstance_t *pstInst = (MHAL_VENC_DummyInstance_t*)hInst;
    const char* szPrefix = "dumy";
    size_t nLen;

    DBG_ENTER();

    s32Ret = _MHAL_VENC_DummyCheckParam(hInst, pInOutBuf);
    if (s32Ret != 0)
        return s32Ret;

    s32Ret = _MHAL_VENC_DummyGetInOutAddr(pInOutBuf, (void *)&szYuv, (void *)&szEs);
    if (s32Ret < 0)
        return s32Ret;

    nLen = strlen(szPrefix);
    strcpy(szEs, szPrefix);
    szEsTmp = szEs + nLen;
    szEsTmp[0] = ((pstInst->u32FrameId % 100) / 10) + '0';
    szEsTmp[1] = (pstInst->u32FrameId % 10) + '0';
    szEsTmp[2] = '\0';
    pstInst->u32LastSize = (MS_U32)(3/*u32FrameId*/ + nLen);

    if (s32Ret == 1) //mapped
    {
        mi_sys_UnVmap(szYuv);
        mi_sys_UnVmap(szEs);
        DBG_EXIT_OK();
    }
    s32Ret = MS_VENC_DUMMY_IRQ;
    //no string.h
    //szEsTmp = szEs + snprintf(szEs, 128, "ENC.FRM: %d", _u32DummyInstance);
    return s32Ret;
}

MS_S32 MHAL_VENC_DummyMfeEncodeOneFrame(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pInOutBuf)
{
    MS_S32 s32Ret = -100;
    char *szYuv;
    char *szEs;
    char *szEsTmp;
    MHAL_VENC_DummyInstance_t *pstInst = (MHAL_VENC_DummyInstance_t*)hInst;
    const char* szPrefix = "h264";
    size_t nLen;

    DBG_ENTER();

    s32Ret = _MHAL_VENC_DummyCheckParam(hInst, pInOutBuf);
    if (s32Ret != 0)
        return s32Ret;

    s32Ret = _MHAL_VENC_DummyGetInOutAddr(pInOutBuf, (void *)&szYuv, (void *)&szEs);
    if (s32Ret < 0)
        return s32Ret;

    nLen = strlen(szPrefix);
    strcpy(szEs, szPrefix);
    szEsTmp = szEs + nLen;
    szEsTmp[0] = ((pstInst->u32FrameId % 100) / 10) + '0';
    szEsTmp[1] = (pstInst->u32FrameId % 10) + '0';
    szEsTmp[2] = '\0';
    pstInst->u32LastSize = (MS_U32)(3/*u32FrameId*/ + nLen);

    if (s32Ret == 1) //mapped
    {
        mi_sys_UnVmap(szYuv);
        mi_sys_UnVmap(szEs);
    }
    s32Ret = 0;
    //no string.h
    //szEsTmp = szEs + snprintf(szEs, 128, "ENC.FRM: %d", _u32DummyInstance);
    return s32Ret;
}

MS_S32 MHAL_VENC_DummyMheEncodeOneFrame(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pInOutBuf)
{
    MS_S32 s32Ret = -100;
    char *szYuv;
    char *szEs;
    char *szEsTmp;
    MHAL_VENC_DummyInstance_t *pstInst = (MHAL_VENC_DummyInstance_t*)hInst;
    const char* szPrefix = "h265";
    size_t nLen;

    DBG_ENTER();

    s32Ret = _MHAL_VENC_DummyCheckParam(hInst, pInOutBuf);
    if (s32Ret != 0)
        return s32Ret;

    s32Ret = _MHAL_VENC_DummyGetInOutAddr(pInOutBuf, (void *)&szYuv, (void *)&szEs);
    if (s32Ret < 0)
        return s32Ret;

    nLen = strlen(szPrefix);
    strcpy(szEs, szPrefix);
    szEsTmp = szEs + nLen;
    szEsTmp[0] = ((pstInst->u32FrameId % 100) / 10) + '0';
    szEsTmp[1] = (pstInst->u32FrameId % 10) + '0';
    szEsTmp[2] = '\0';
    pstInst->u32LastSize = (MS_U32)(3/*u32FrameId*/ + nLen);

    if (s32Ret == 1) //mapped
    {
        mi_sys_UnVmap(szYuv);
        mi_sys_UnVmap(szEs);
    }
    s32Ret = 0;
    //no string.h
    //szEsTmp = szEs + snprintf(szEs, 128, "ENC.FRM: %d", _u32DummyInstance);
    return s32Ret;
}

MS_S32 MHAL_VENC_DummyJpegEncodeOneFrame(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pInOutBuf)
{
    MS_S32 s32Ret = -100;
    char *szYuv;
    char *szEs;
    char *szEsTmp;
    MHAL_VENC_DummyInstance_t *pstInst = (MHAL_VENC_DummyInstance_t*)hInst;
    const char* szPrefix = "jpeg";
    size_t nLen;

    DBG_ENTER();

    s32Ret = _MHAL_VENC_DummyCheckParam(hInst, pInOutBuf);
    if (s32Ret != 0)
        return s32Ret;

    s32Ret = _MHAL_VENC_DummyGetInOutAddr(pInOutBuf, (void *)&szYuv, (void *)&szEs);
    if (s32Ret < 0)
        return s32Ret;

    nLen = strlen(szPrefix);
    strcpy(szEs, szPrefix);
    szEsTmp = szEs + nLen;
    szEsTmp[0] = ((pstInst->u32FrameId % 100) / 10) + '0';
    szEsTmp[1] = (pstInst->u32FrameId % 10) + '0';
    szEsTmp[2] = '\0';
    pstInst->u32LastSize = (MS_U32)(3/*u32FrameId*/ + nLen);

    if (s32Ret == 1) //mapped
    {
        mi_sys_UnVmap(szYuv);
        mi_sys_UnVmap(szEs);
    }
    s32Ret = 0;
    //no string.h
    //szEsTmp = szEs + snprintf(szEs, 128, "ENC.FRM: %d", _u32DummyInstance);
    return s32Ret;
}

MS_S32 MHAL_VENC_DummyEncodeDone(MHAL_VENC_INST_HANDLE hInst,MHAL_VENC_EncResult_t* pEncRet)
{
    MS_S32 s32Ret = -100;
    MHAL_VENC_DummyInstance_t *pstInst = (MHAL_VENC_DummyInstance_t*)hInst;
    if (hInst == NULL || pEncRet == NULL)
        return -1;

    pEncRet->u32OutputBufUsed = pstInst->u32LastSize;
    pstInst->u32FrameId++; //simulate rate control and encoding sequence
    s32Ret = 0;

    return s32Ret;
}

MS_S32 MHAL_VENC_DummyIsrProc(MHAL_VENC_DEV_HANDLE hDev)
{
    return 0;
}

MHAL_VENC_Drv_t stDrvDummy = {
    .CreateDevice =    MHAL_VENC_DummyCreateDevice,
    .DestroyDevice =   MHAL_VENC_DummyDestroyDevice,
    .GetDevConfig =    MHAL_VENC_DummyGetDevConfig,
    .IsrProc      =    MHAL_VENC_DummyIsrProc,
    .CreateInstance =  MHAL_VENC_DummyCreateInstance,
    .DestroyInstance = MHAL_VENC_DummyDestroyInstance,
    .SetParam =        MHAL_VENC_DummySetParam,
    .GetParam =        MHAL_VENC_DummyGetParam,
    .EncodeOneFrame =  MHAL_VENC_DummyEncodeOneFrame,
    .EncodeDone =      MHAL_VENC_DummyEncodeDone,
    .QueryBufSize =    MHAL_VENC_DummyQueryBufSize
};

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30) && defined(CONFIG_ARCH_INFINITY2)//i2, i2-FPGA
MHAL_VENC_Drv_t stDrvMhe = {
    .CreateDevice =    MHAL_MHE_CreateDevice,
    .DestroyDevice =   MHAL_MHE_DestroyDevice,
    .GetDevConfig =    MHAL_MHE_GetDevConfig,
    .IsrProc      =    MHAL_MHE_IsrProc,
    .CreateInstance =  MHAL_MHE_CreateInstance,
    .DestroyInstance = MHAL_MHE_DestroyInstance,
    .SetParam =        MHAL_MHE_SetParam,
    .GetParam =        MHAL_MHE_GetParam,
    .EncodeOneFrame =  MHAL_MHE_EncodeOneFrame, ///< a difference against stDrvDummy
    .EncodeDone =      MHAL_MHE_EncodeFrameDone,
    .QueryBufSize =    MHAL_MHE_QueryBufSize
};
#else //k6l or i3
MHAL_VENC_Drv_t stDrvMhe = {
    .CreateDevice =    MHAL_VENC_DummyCreateDevice,
    .DestroyDevice =   MHAL_VENC_DummyDestroyDevice,
    .GetDevConfig =    MHAL_VENC_DummyGetDevConfig,
    .IsrProc      =    MHAL_VENC_DummyIsrProc,
    .CreateInstance =  MHAL_VENC_DummyCreateInstance,
    .DestroyInstance = MHAL_VENC_DummyDestroyInstance,
    .SetParam =        MHAL_VENC_DummySetParam,
    .GetParam =        MHAL_VENC_DummyGetParam,
    .EncodeOneFrame =  MHAL_VENC_DummyMheEncodeOneFrame, ///< a difference against stDrvDummy
    .EncodeDone =      MHAL_VENC_DummyEncodeDone,
    .QueryBufSize =    MHAL_VENC_DummyQueryBufSize
};
#endif

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30) //i2, i3, i2-FPGA
MHAL_VENC_Drv_t stDrvMfe = {
    .CreateDevice =    MHAL_MFE_CreateDevice,
    .DestroyDevice =   MHAL_MFE_DestroyDevice,
    .GetDevConfig =    MHAL_MFE_GetDevConfig,
    .IsrProc      =    MHAL_MFE_IsrProc,
    .CreateInstance =  MHAL_MFE_CreateInstance,
    .DestroyInstance = MHAL_MFE_DestroyInstance,
    .SetParam =        MHAL_MFE_SetParam,
    .GetParam =        MHAL_MFE_GetParam,
    .EncodeOneFrame =  MHAL_MFE_EncodeOneFrame, ///< a difference against stDrvDummy
    .EncodeDone =      MHAL_MFE_EncodeFrameDone,
    .QueryBufSize =    MHAL_MFE_QueryBufSize
};
#else //k6l
MHAL_VENC_Drv_t stDrvMfe = {
    .CreateDevice =    MHAL_VENC_DummyCreateDevice,
    .DestroyDevice =   MHAL_VENC_DummyDestroyDevice,
    .GetDevConfig =    MHAL_VENC_DummyGetDevConfig,
    .IsrProc      =    MHAL_VENC_DummyIsrProc,
    .CreateInstance =  MHAL_VENC_DummyCreateInstance,
    .DestroyInstance = MHAL_VENC_DummyDestroyInstance,
    .SetParam =        MHAL_VENC_DummySetParam,
    .GetParam =        MHAL_VENC_DummyGetParam,
    .EncodeOneFrame =  MHAL_VENC_DummyMfeEncodeOneFrame, ///< a difference against stDrvDummy
    .EncodeDone =      MHAL_VENC_DummyEncodeDone,
    .QueryBufSize =    MHAL_VENC_DummyQueryBufSize
};
#endif

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30) //i2, i3, i2-FPGA
MHAL_VENC_Drv_t stDrvJpe = {
    .CreateDevice =     MHAL_JPE_CreateDevice,
    .DestroyDevice =    MHAL_JPE_DestroyDevice,
    .GetDevConfig =     MHAL_JPE_GetDevConfig,
    .IsrProc      =     MHAL_JPE_IsrProc,
    .CreateInstance =   MHAL_JPE_CreateInstance,
    .DestroyInstance =  MHAL_JPE_DestroyInstance,
    .SetParam =         MHAL_JPE_SetParam,
    .GetParam =         MHAL_JPE_GetParam,
    .EncodeOneFrame =   MHAL_JPE_EncodeOneFrame,
    .EncodeDone =       MHAL_JPE_EncodeFrameDone,
    .EncodeCancel =     MHAL_JPE_EncodeCancel,
    .EncodeAddOutBuff = MHAL_JPE_EncodeAddOutBuff,
    .QueryBufSize =     MHAL_JPE_QueryBufSize,
    .GetEngStatus =     MHAL_JPE_GetEngStatus
};
#else //k6l
MHAL_VENC_Drv_t stDrvJpe = {
    .CreateDevice =    MHAL_VENC_DummyCreateDevice,
    .DestroyDevice =   MHAL_VENC_DummyDestroyDevice,
    .GetDevConfig =    MHAL_VENC_DummyGetDevConfig,
    .IsrProc      =    MHAL_VENC_DummyIsrProc,
    .CreateInstance =  MHAL_VENC_DummyCreateInstance,
    .DestroyInstance = MHAL_VENC_DummyDestroyInstance,
    .SetParam =        MHAL_VENC_DummySetParam,
    .GetParam =        MHAL_VENC_DummyGetParam,
    .EncodeOneFrame =  MHAL_VENC_DummyJpegEncodeOneFrame, ///< a difference against stDrvDummy
    .EncodeDone =      MHAL_VENC_DummyEncodeDone,
    .QueryBufSize =    MHAL_VENC_DummyQueryBufSize
};
#endif
