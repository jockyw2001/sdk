/*
 * hal_mfe_device.c
 *
 *  Created on: Aug 14, 2017
 *      Author: giggs.huang
 */

#include "drv_mfe_kernel.h"
#include "drv_mfe_dc_kernel.h"
#include "mdrv_mfe_io.h"
#include "mdrv_mfe_st.h"
#include "mdrv_rqct_st.h"
#include "mdrv_rqct_io.h"
#include "drv_mfe_ctx.h"
#include "drv_mfe_dev.h"
#include "hal_mfe_api.h"
#include "mhal_mfe.h"
#include "mhal_venc.h"
#include "mhal_ut_wrapper.h"
#include "mhve_pmbr_cfg.h"
#include <linux/interrupt.h>

#define MFEDC_I3_TEST

#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

void _MfeDcSetHeader(VOID* header, MS_U32 size) {
    MHAL_VENC_Version_t* ver = (MHAL_VENC_Version_t*)header;
    ver->u32Size = size;

    ver->s.u8VersionMajor = SPECVERSIONMAJOR;
    ver->s.u8VersionMinor = SPECVERSIONMINOR;
    ver->s.u8Revision = SPECREVISION;
    ver->s.u8Step = SPECSTEP;
}

MHAL_ErrCode_e _MfeDcCheckHeader(VOID* pHeader, MS_U32 u32Size)
{
    MHAL_ErrCode_e eError = 0;

    MHAL_VENC_Version_t* ver;

    if (pHeader == NULL) {
        CamOsPrintf("In %s the header is null\n",__func__);
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    ver = (MHAL_VENC_Version_t*)pHeader;

    if(ver->u32Size != u32Size) {
        CamOsPrintf("In %s the header has a wrong size %i should be %i\n",__func__,ver->u32Size,u32Size);
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    if(ver->s.u8VersionMajor != SPECVERSIONMAJOR ||
            ver->s.u8VersionMinor != SPECVERSIONMINOR) {
        CamOsPrintf("The version does not match\n");
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    return eError;
}

static irqreturn_t _MfeDcDev0Isr(int irq, void* priv)
{
    mmfedc_dev *mdev = priv;
    MHAL_VENC_EncResult_t stEncRet;

    CamOsPrintf("_MfeDcDev0Isr\n");

    if (mdev && mdev->pDev[0])
        MHAL_MFE_IsrProc(mdev->pDev[0]);

    if (mdev && mdev->Dev0Cb)
    {
        stEncRet.eStatus            = 1;
        stEncRet.phyOutputBuf       = 1;
        stEncRet.u32OutputBufUsed   = 1;
        stEncRet.pUserPtr           = NULL;
        mdev->Dev0Cb(&stEncRet);
    }

    return IRQ_HANDLED;
}

#ifndef MFEDC_I3_TEST
static irqreturn_t _MfeDcDev1Isr(int irq, void* priv)
{
    mmfedc_dev *mdev = priv;
    MHAL_VENC_EncResult_t stEncRet;

    CamOsPrintf("_MfeDcDev1Isr\n");

    if (mdev && mdev->pDev[1])
        MHAL_MFE_IsrProc(mdev->pDev[1]);

    if (mdev && mdev->Dev1Cb)
    {
        stEncRet.u32OutputBufUsed = 999;
        mdev->Dev1Cb(&stEncRet);
    }

    return IRQ_HANDLED;
}
#endif

MS_S32 MHAL_MFE_DC_CreateDevice(MHAL_VENC_DEV_HANDLE *phDev)
{
    mmfedc_dev *mdev = NULL;
    MHAL_VENC_ParamInt_t stVEncInt;

    *phDev = NULL;

    do
    {
        if (!(mdev = CamOsMemCalloc(sizeof(mmfedc_dev), 1)))
            break;

        if (0 != MHAL_MFE_CreateDevice(0, &mdev->pDev[0]))
        {
            CamOsPrintf("MHAL_MFE_DC_CreateDevice create sub device0 fail\n");
            break;
        }
        else
        {
            memset(&stVEncInt, 0, sizeof(stVEncInt));
            _MfeDcSetHeader(&stVEncInt, sizeof(stVEncInt));
            if (MHAL_MFE_GetDevConfig(mdev->pDev[0], E_MHAL_VENC_HW_IRQ_NUM, &stVEncInt))
            {
                CamOsPrintf("MHAL_MFE_DC_CreateDevice get irq0 fail\n");
                break;
            }

            mdev->nIrq[0] = stVEncInt.u32Val;
            if (0 != request_irq(mdev->nIrq[0], _MfeDcDev0Isr, IRQF_SHARED, "_MfeDcDev0Isr", mdev))
            {
                CamOsPrintf("MHAL_MFE_DC_CreateDevice request_irq(%d) fail\n", stVEncInt.u32Val);
                return 0;
            }
        }

#ifndef MFEDC_I3_TEST
        if (0 != MHAL_MFE_CreateDevice(1, (void *)mdev->pDev[1]))
        {
            CamOsPrintf("MHAL_MFE_DC_CreateDevice create sub device1 fail\n");
            break;
        }
        else
        {
            memset(&stVEncInt, 0, sizeof(stVEncInt));
            _MfeDcSetHeader(&stVEncInt, sizeof(stVEncInt));
            if (MHAL_MFE_GetDevConfig(mdev->pDev[1], E_MHAL_VENC_HW_IRQ_NUM, &stVEncInt))
            {
                CamOsPrintf("MHAL_MFE_DC_CreateDevice get irq1 fail\n");
                break;
            }

            mdev->nIrq[1] = stVEncInt.u32Val;
            if (0 != request_irq(mdev->nIrq[1], _MfeDcDev1Isr, IRQF_SHARED, "_MfeDcDev1Isr", mdev))
            {
                CamOsPrintf("MHAL_MFE_DC_CreateDevice request_irq(%d) fail\n", stVEncInt.u32Val);
                return 0;
            }
        }
#endif

        *phDev = (MHAL_VENC_DEV_HANDLE)mdev;

        return 0;
    }
    while (0);

    if (mdev)
    {
        if (mdev->pDev[0])
            MHAL_MFE_DestroyDevice(mdev->pDev[0]);
        if (mdev->pDev[1])
            MHAL_MFE_DestroyDevice(mdev->pDev[1]);
        CamOsMemRelease(mdev);
    }

    return E_MHAL_ERR_ILLEGAL_PARAM;
}

MS_S32 MHAL_MFE_DC_DestroyDevice(MHAL_VENC_DEV_HANDLE hDev)
{
    MHAL_ErrCode_e eError = 0;
    mmfedc_dev *mdev = (mmfedc_dev *)hDev;

    if (mdev)
    {
        if (mdev->pDev[0])
        {
            free_irq(mdev->nIrq[0], mdev);
            MHAL_MFE_DestroyDevice(mdev->pDev[0]);
        }
        if (mdev->pDev[1])
        {
            free_irq(mdev->nIrq[1], mdev);
            MHAL_MFE_DestroyDevice(mdev->pDev[1]);
        }
        CamOsMemRelease(mdev);

        eError = 0;
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_MFE_DC_CreateInstance(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_INST_HANDLE *phInst)
{
    mmfedc_dev* mdev = (mmfedc_dev *)hDev;
    mmfedc_ctx* mctx;

    if (!mdev)
        return E_MHAL_ERR_NULL_PTR;

    do
    {
        if (!(mctx = CamOsMemCalloc(sizeof(mmfedc_ctx), 1)))
            break;

        if (mdev->pDev[0] && 0 != MHAL_MFE_CreateInstance(mdev->pDev[0], &mctx->pCtx[0]))
            break;

        if (mdev->pDev[1] && 0 != MHAL_MFE_CreateInstance(mdev->pDev[1], &mctx->pCtx[1]))
            break;

        mctx->pDcDev = mdev;

        *phInst = mctx;
        return 0;
    }
    while (0);

    if (mctx)
    {
        if (mctx->pCtx[0])
        {
            MHAL_MFE_SetParam(mctx->pCtx[0], E_MHAL_VENC_IDX_STREAM_OFF, NULL);
            MHAL_MFE_DestroyInstance(mctx->pCtx[0]);
        }

        if (mctx->pCtx[1])
        {
            MHAL_MFE_SetParam(mctx->pCtx[1], E_MHAL_VENC_IDX_STREAM_OFF, NULL);
            MHAL_MFE_DestroyInstance(mctx->pCtx[1]);
        }

        CamOsMemRelease(mctx);
    }

    return E_MHAL_ERR_ILLEGAL_PARAM;
}

MS_S32 MHAL_MFE_DC_DestroyInstance(MHAL_VENC_INST_HANDLE hInst)
{
    MHAL_ErrCode_e eError = 0;
    mmfedc_ctx* mctx = (mmfedc_ctx*)hInst;

    if (mctx)
    {
        if (mctx->pCtx[0])
        {
            MHAL_MFE_SetParam(mctx->pCtx[0], E_MHAL_VENC_IDX_STREAM_OFF, NULL);
            MHAL_MFE_DestroyInstance(mctx->pCtx[0]);
        }

        if (mctx->pCtx[1])
        {
            MHAL_MFE_SetParam(mctx->pCtx[1], E_MHAL_VENC_IDX_STREAM_OFF, NULL);
            MHAL_MFE_DestroyInstance(mctx->pCtx[1]);
        }

        CamOsMemRelease(mctx);
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_MFE_DC_SetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam)
{
    MHAL_ErrCode_e eError = 0;
    mmfedc_ctx* mctx = (mmfedc_ctx* )hInst;

    if (mctx)
    {
        if (mctx->pCtx[0] && 0 != (eError = MHAL_MFE_SetParam(mctx->pCtx[0], eType, pstParam)))
            return eError;
        if (mctx->pCtx[1] && 0 != (eError = MHAL_MFE_SetParam(mctx->pCtx[1], eType, pstParam)))
            return eError;
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_MFE_DC_GetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam)
{
    MHAL_ErrCode_e eError = 0;
    mmfedc_ctx* mctx = (mmfedc_ctx* )hInst;

    if (mctx)
    {
        if (mctx->pCtx[0] && 0 != (eError = MHAL_MFE_GetParam(mctx->pCtx[0], eType, pstParam)))
            return eError;
        if (mctx->pCtx[1] && 0 != (eError = MHAL_MFE_GetParam(mctx->pCtx[1], eType, pstParam)))
            return eError;
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_MFE_DC_EncodeFrame(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pstInOutBuf, void (*MfeEventHandler)(MHAL_VENC_EncResult_t *))
{
    MHAL_ErrCode_e eError = 0;
    mmfedc_ctx* mctx = (mmfedc_ctx*)hInst;

    if (mctx)
    {
        if (mctx->pCtx[0])
        {
            if (0 != (eError = MHAL_MFE_EncodeOneFrame(mctx->pCtx[0], pstInOutBuf)))
            {
                return eError;
            }
            else if (mctx->pDcDev)
            {
                mctx->pDcDev->Dev0Cb = MfeEventHandler;
            }
        }

        if (mctx->pCtx[1])
        {
            if (0 != (eError = MHAL_MFE_EncodeOneFrame(mctx->pCtx[1], pstInOutBuf)))
            {
                return eError;
            }
            else if (mctx->pDcDev)
            {
                mctx->pDcDev->Dev1Cb = MfeEventHandler;
            }
        }
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_MFE_DC_EncodeFrameDone(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_EncResult_t* pstEncRet)
{
    MHAL_ErrCode_e eError = 0;
    mmfedc_ctx* mctx = (mmfedc_ctx*)hInst;

    if (mctx)
    {
        if (mctx->pCtx[0] && 0 != (eError = MHAL_MFE_EncodeFrameDone(mctx->pCtx[0], pstEncRet)))
            return eError;
        if (mctx->pCtx[1] && 0 != (eError = MHAL_MFE_EncodeFrameDone(mctx->pCtx[1], pstEncRet)))
            return eError;
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_MFE_DC_QueryBufSize(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InternalBuf_t *pstParam)
{
    MHAL_ErrCode_e eError = 0;
    mmfedc_ctx* mctx = (mmfedc_ctx*)hInst;
    MHAL_VENC_InternalBuf_t stMfeIntrBuf0;
    MHAL_VENC_InternalBuf_t stMfeIntrBuf1;

    if (mctx)
    {
        memset(&stMfeIntrBuf0, 0, sizeof(MHAL_VENC_InternalBuf_t));
        _MfeDcSetHeader(&stMfeIntrBuf0, sizeof(MHAL_VENC_InternalBuf_t));
        if (mctx->pCtx[0] && 0 != (eError = MHAL_MFE_QueryBufSize(mctx->pCtx[0], &stMfeIntrBuf0)))
            return eError;

        memset(&stMfeIntrBuf1, 0, sizeof(MHAL_VENC_InternalBuf_t));
        _MfeDcSetHeader(&stMfeIntrBuf1, sizeof(MHAL_VENC_InternalBuf_t));
        if (mctx->pCtx[1] && 0 != (eError = MHAL_MFE_QueryBufSize(mctx->pCtx[1], &stMfeIntrBuf1)))
            return eError;
#ifdef MFEDC_I3_TEST
        pstParam->u32IntrAlBufSize = stMfeIntrBuf0.u32IntrAlBufSize * 2;
        pstParam->u32IntrRefBufSize = stMfeIntrBuf0.u32IntrRefBufSize * 2;
#else
        pstParam->u32IntrAlBufSize = stMfeIntrBuf0.u32IntrAlBufSize + stMfeIntrBuf1.u32IntrAlBufSize;
        pstParam->u32IntrRefBufSize = stMfeIntrBuf0.u32IntrRefBufSize + stMfeIntrBuf1.u32IntrRefBufSize;
#endif
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}
