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

#include "hal_jpe_ios.h"
#include "hal_jpe_ops.h"
#include "drv_jpe_io.h"
#include "drv_jpe_io_st_kernel.h"
#include "_drv_jpe_dev.h"

#if defined(__linux__)
#include <asm/uaccess.h>
#else

static unsigned long copy_from_user(void *to, const void *from, unsigned long n)
{
    memcpy(to, from, n);
    return 0;
}
static unsigned long copy_to_user (void * to, const void * from, unsigned long n)
{
    memcpy(to, from, n);
    return 0;
}

#endif

//=============================================================================
// Description:
//     IOCTL handler for JPE_IOC_SET_OUTBUF
// Author:
//      Albert.Liao.
// Input:
//      pCtx: JPEG user context
//      arg: a pointer of JpeBufInfo_t from user-space
// Output:
//      JPE_IOC_RET_SUCCESS: Success
//      JPE_IOC_RET_FAIL: Failure
//=============================================================================
JPE_IOC_RET_STATUS_e _JpeCtxSetOutputBuffer(JpeCtx_t* pCtxIn, JpeBufInfo_t* pOutBuf)
{
    JPE_IOC_RET_STATUS_e eStatus = JPE_IOC_RET_SUCCESS;
    JpeCtx_t* pCtx = pCtxIn;
    JpeCfg_t* pJpeCfg = &pCtx->tJpeCfg;

    if(!pJpeCfg)
    {
        return JPE_IOC_RET_FAIL;
    }

    CamOsTsemDown(&pCtx->m_stream);

    /* Is a legal buffer? */
    if(pOutBuf->nAddr == 0 || pOutBuf->nSize == 0)
    {
        JPE_MSG(JPE_MSG_ERR, "The buffer is not legal\n");
        eStatus = JPE_IOC_RET_BAD_OUTBUF;
        CamOsTsemUp(&pCtx->m_stream);

        goto RETURN;
    }

    /* Set output buffer */
    pCtx->tEncOutBuf.nAddr = pOutBuf->nAddr;
    pCtx->tEncOutBuf.nOrigSize = pOutBuf->nSize;

    pJpeCfg->OutBuf.nAddr = pOutBuf->nAddr;
    pJpeCfg->OutBuf.nSize = pOutBuf->nSize;

    // The actual configuration will be applied when encode

    CamOsTsemUp(&pCtx->m_stream);

RETURN:
    return eStatus;
}


//=============================================================================
// Description:
//     Release JPEG user context structure
// Author:
//      Albert.Liao.
// Input:
//      pCtxIn: JPEG user context
// Output:
//      void
//=============================================================================
void _JpeCtxRelease(void* pCtxIn)
{
    JpeCtx_t* pCtx = pCtxIn;
    JpeOpsCB_t* pOpsCB = pCtx->p_handle;

    CamOsTsemDown(&pCtx->m_stream);

    pCtx->i_state = JPE_CTX_STATE_NULL;
    if(pOpsCB)
    {
//        if(pOpsCB->release)
//            pOpsCB->release(pOpsCB);
        CamOsMemRelease(pOpsCB);
        pCtx->p_handle = NULL;
    }

    CamOsTsemUp(&pCtx->m_stream);

    CamOsTsemDeinit(&pCtx->m_stream);
    CamOsMemRelease(pCtx->p_usrdt);
    CamOsMemRelease(pCtx);
}


//=============================================================================
// Description:
//     IOCTL handler for JPE_IOC_INIT
//     User uses this command to reset & init JPE encoder. User has to call this function again
//     before starting to encode another frame.
// Author:
//      Albert.Liao.
// Input:
//      pCtx: JPEG user context
//      arg: a pointer of JpeCfg_t from user-space
// Output:
//      JPE_IOC_RET_SUCCESS: Success
//      JPE_IOC_RET_FAIL: Failure
//=============================================================================
JPE_IOC_RET_STATUS_e _JpeCtxIoctlInit(JpeCtx_t* pCtxIn, JpeCfg_t* pJpeCfg)
{
    int nRet;
    JPE_IOC_RET_STATUS_e eStatus = JPE_IOC_RET_SUCCESS;
    JpeCtx_t* pCtx = pCtxIn;
    JpeOpsCB_t* pOpsCB = pCtx->p_handle;

    CamOsTsemDown(&pCtx->m_stream);

    pCtx->i_state = JPE_CTX_STATE_IDLE;
    nRet = pOpsCB->init(pOpsCB, pJpeCfg);
    if(JPE_IOC_RET_SUCCESS == nRet)
    {
        memcpy(&pCtxIn->tJpeCfg, pJpeCfg, sizeof(JpeCfg_t));
    }
    pCtx->tEncOutBuf.nAddr = pJpeCfg->OutBuf.nAddr;
    pCtx->tEncOutBuf.nOrigSize = pJpeCfg->OutBuf.nSize;
    eStatus = nRet;

    CamOsTsemUp(&pCtx->m_stream);

    return eStatus;
}


//=============================================================================
// Description:
//     IOCTL handler for JPE_IOC_GET_CAPS
//     This command provide the capabilities of JPE
// Author:
//      Albert.Liao.
// Input:
//      pCtx: JPEG user context
//      arg: a pointer of JpeCaps_t from user-space
// Output:
//      JPE_IOC_RET_SUCCESS: Success
//      JPE_IOC_RET_FAIL: Failure
//=============================================================================
JPE_IOC_RET_STATUS_e _JpeCtxGetCapbilities(JpeCtx_t* pCtxIn, JpeCaps_t* pCaps)
{
    JPE_IOC_RET_STATUS_e eStatus = JPE_IOC_RET_SUCCESS;
    JpeCtx_t* pCtx = pCtxIn;
    JpeOpsCB_t *pOpsCB = pCtx->p_handle;

    memset(pCaps, 0, sizeof(JpeCaps_t));

    CamOsTsemDown(&pCtx->m_stream);

    /* Get capability */

    eStatus = pOpsCB->getCaps(pOpsCB, pCaps);
    if(eStatus != JPE_IOC_RET_SUCCESS)
    {
        CamOsTsemUp(&pCtx->m_stream);
        return eStatus;
    }

    CamOsTsemUp(&pCtx->m_stream);

    return eStatus;
}


//=============================================================================
// Description:
//     IOCTL handler for JPE_IOC_GETBITS
//     User uses this command to get the processed (encoded) buffer
// Author:
//      Albert.Liao.
// Input:
//      pCtx: JPEG user context
//      arg: a pointer of JpeBitstreamInfo_t from user-space
// Output:
//      JPE_IOC_RET_SUCCESS: Success
//      JPE_IOC_RET_FAIL: Failure
//=============================================================================
JPE_IOC_RET_STATUS_e _JpeCtxGetBitInfo(JpeCtx_t* pCtxIn, JpeBitstreamInfo_t* pStreamInfo)
{
    JPE_IOC_RET_STATUS_e eStatus = JPE_IOC_RET_SUCCESS;
    JpeCtx_t* pCtx = pCtxIn;

    memset(pStreamInfo, 0, sizeof(JpeBitstreamInfo_t));

    CamOsTsemDown(&pCtx->m_stream);

    if(pCtx->tEncOutBuf.eState == JPE_FRAME_DONE_STATE && pCtx->tEncOutBuf.nAddr && pCtx->tEncOutBuf.nOutputSize)
    {
        pStreamInfo->nAddr = pCtx->tEncOutBuf.nAddr;
        pStreamInfo->nOrigSize = pCtx->tEncOutBuf.nOrigSize;
        pStreamInfo->nOutputSize = pCtx->tEncOutBuf.nOutputSize;
        pStreamInfo->eState = pCtx->tEncOutBuf.eState;
    }
    else
    {
        JPE_MSG(JPE_MSG_ERR, "ERROR!! tEncOutBuf.addr:0x%x, tEncOutBuf.size:%ld\n", \
                (unsigned int)pCtx->tEncOutBuf.nAddr, pCtx->tEncOutBuf.nOutputSize);

        switch(pCtx->tEncOutBuf.eState)
        {
            case JPE_IDLE_STATE:
                JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_IDLE_STATE));
                break;
            case JPE_BUSY_STATE:
                JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_BUSY_STATE));
                break;
            case JPE_FRAME_DONE_STATE:
                JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_FRAME_DONE_STATE));
                break;
            case JPE_OUTBUF_FULL_STATE:
                JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_OUTBUF_FULL_STATE));
                break;
            case JPE_INBUF_FULL_STATE:
                JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_INBUF_FULL_STATE));
                break;
        }

        CamOsTsemUp(&pCtx->m_stream);

        return JPE_IOC_RET_HW_IS_RUNNING;
    }

    CamOsTsemUp(&pCtx->m_stream);

    return eStatus;
}



//=============================================================================
// Description:
//     IOCTL handler for JPE_IOC_ENCODE_FRAME
//     This command is used to encode JPEG image accroding to JpeCfg_t
// Author:
//      Albert.Liao.
// Input:
//      pCtx: JPEG user context
//      arg: a pointer of JpeBitstreamInfo_t from user-space
// Output:
//      JPE_IOC_RET_SUCCESS: Success
//      JPE_IOC_RET_FAIL: Failure
//=============================================================================
JPE_IOC_RET_STATUS_e _JpeCtxEncodeFrame(JpeCtx_t* pCtxIn, JpeBitstreamInfo_t* pStreamInfo)
{
    JPE_IOC_RET_STATUS_e eStatus = JPE_IOC_RET_SUCCESS;

    JpeCtx_t* pCtx = pCtxIn;
    JpeDev_t* pDev = pCtx->p_device;
    JpeOpsCB_t* pOpsCB = pCtx->p_handle;
    JpeOps_t* pOps = pCtx->p_handle;
    JpeCfg_t* pJpeCfg = (JpeCfg_t *)&pCtxIn->tJpeCfg;

    memset(pStreamInfo, 0, sizeof(JpeBitstreamInfo_t));

    CamOsTsemDown(&pCtx->m_stream);

    do
    {
        if(JPE_CTX_STATE_IDLE == pCtx->i_state)
        {
            // If MIU address translation is need. It will be translate in HAL layer.
            pOps->nClkSelect = pCtxIn->nClkSelect;
            eStatus = pOpsCB->setConf(pOpsCB, pJpeCfg);
            if(JPE_IOC_RET_SUCCESS != eStatus)
            {
                JPE_MSG(JPE_MSG_ERR, "pOpsCB->set_conf err\n");
                break;
            }

            pCtx->i_state = JPE_CTX_STATE_BUSY;

            // When this function return, the raw data is already encoded as a JPEG image
            eStatus = JpeDevPushJob(pDev, pCtx);
            if(eStatus)
            {
                JPE_MSG(JPE_MSG_ERR, "jpeDevPushJob err\n");
                pCtx->i_state = JPE_CTX_STATE_IDLE;
                break;
            }

            pStreamInfo->nAddr = pCtx->tEncOutBuf.nAddr;
            pStreamInfo->nOrigSize = pCtx->tEncOutBuf.nOrigSize;
            pStreamInfo->nOutputSize = pCtx->tEncOutBuf.nOutputSize;
            pStreamInfo->eState = pCtx->tEncOutBuf.eState;

            pCtx->i_state = JPE_CTX_STATE_IDLE;
        }
    }
    while(0);

    CamOsTsemUp(&pCtx->m_stream);

    return eStatus;
}

//=============================================================================
// Description:
//     Non-blocked Jpe encode frame for MHAL layer
//     This command is used to encode JPEG image accroding to JpeCfg_t
// Author:
//      Ken.Chang
// Input:
//      pCtx: JPEG user context
//      arg: a pointer of JpeBitstreamInfo_t from user-space
// Output:
//      JPE_IOC_RET_SUCCESS: Success
//      JPE_IOC_RET_FAIL: Failure
//=============================================================================
JPE_IOC_RET_STATUS_e JpeCtxEncFireAndReturn(JpeCtx_t* pCtxIn)
{
    JPE_IOC_RET_STATUS_e eStatus = JPE_IOC_RET_SUCCESS;

    JpeCtx_t* pCtx = pCtxIn;
    JpeDev_t* pDev = pCtx->p_device;
    JpeOpsCB_t* pOpsCB = pCtx->p_handle;
    JpeOps_t* pOps = pCtx->p_handle;
    JpeCfg_t* pJpeCfg = (JpeCfg_t *)&pCtxIn->tJpeCfg;

    CamOsTsemDown(&pCtx->m_stream);

    do
    {
        if(JPE_CTX_STATE_IDLE == pCtx->i_state)
        {
            // If MIU address translation is need. It will be translate in HAL layer.
            pOps->nClkSelect = pCtxIn->nClkSelect;
            eStatus = pOpsCB->setConf(pOpsCB, pJpeCfg);
            if(JPE_IOC_RET_SUCCESS != eStatus)
            {
                JPE_MSG(JPE_MSG_ERR, "pOpsCB->set_conf err\n");
                break;
            }

            pCtx->i_state = JPE_CTX_STATE_BUSY;
            // When this function return, the raw data is already encoded as a JPEG image
            eStatus = JpeDevLockAndFire(pDev, pCtx);
            if(eStatus)
            {
                JPE_MSG(JPE_MSG_ERR, "JpeDevLockAndFire err\n");
                pCtx->i_state = JPE_CTX_STATE_IDLE;
                break;
            }

        }
    }
    while(0);

    CamOsTsemUp(&pCtx->m_stream);

    return eStatus;
}

//=============================================================================
// Description:
//     Non-blocked Jpe encode frame done for MHAL layer
//     This command is used to get JPEG image accroding to JpeCfg_t
// Author:
//      Ken.Chang
// Input:
//      pCtx: JPEG user context
//      arg: a pointer of JpeBitstreamInfo_t from user-space
// Output:
//      JPE_IOC_RET_SUCCESS: Success
//      JPE_IOC_RET_FAIL: Failure
//=============================================================================
JPE_IOC_RET_STATUS_e JpeCtxEncPostProc(JpeCtx_t* pCtxIn, JpeBitstreamInfo_t* pStreamInfo)
{
    JPE_IOC_RET_STATUS_e eStatus = JPE_IOC_RET_SUCCESS;

    JpeCtx_t* pCtx = pCtxIn;
    JpeDev_t* pDev = pCtx->p_device;

    memset(pStreamInfo, 0, sizeof(JpeBitstreamInfo_t));

    CamOsTsemDown(&pCtx->m_stream);

    do
    {
        if(JPE_CTX_STATE_BUSY == pCtx->i_state)
        {
            JpeDevUnlockAfterEncDone(pDev, pCtx);
            pCtx->i_state = JPE_CTX_STATE_IDLE;
            if(pCtx->tEncOutBuf.eState == JPE_FRAME_DONE_STATE && pCtx->tEncOutBuf.nAddr && pCtx->tEncOutBuf.nOutputSize)
            {
                pStreamInfo->nAddr = pCtx->tEncOutBuf.nAddr;
                pStreamInfo->nOrigSize = pCtx->tEncOutBuf.nOrigSize;
                pStreamInfo->nOutputSize = pCtx->tEncOutBuf.nOutputSize;
                pStreamInfo->eState = pCtx->tEncOutBuf.eState;
            }
            else
            {
                JPE_MSG(JPE_MSG_ERR, "ERROR!! tEncOutBuf.addr:0x%x, tEncOutBuf.size:%ld\n", \
                        (unsigned int)pCtx->tEncOutBuf.nAddr, pCtx->tEncOutBuf.nOutputSize);

                switch(pCtx->tEncOutBuf.eState)
                {
                    case JPE_IDLE_STATE:
                        JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_IDLE_STATE));
                        break;
                    case JPE_BUSY_STATE:
                        JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_BUSY_STATE));
                        break;
                    case JPE_FRAME_DONE_STATE:
                        JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_FRAME_DONE_STATE));
                        break;
                    case JPE_OUTBUF_FULL_STATE:
                        JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_OUTBUF_FULL_STATE));
                        break;
                    case JPE_INBUF_FULL_STATE:
                        JPE_MSG(JPE_MSG_ERR, "eState=%s\n", JPE_TOSTRING(JPE_INBUF_FULL_STATE));
                        break;
                }

                CamOsTsemUp(&pCtx->m_stream);

                CamOsPrintf("%s:%d\n", __FUNCTION__, __LINE__);
                return JPE_IOC_RET_HW_IS_RUNNING;
            }
        }
    }
    while(0);

    CamOsTsemUp(&pCtx->m_stream);

    return eStatus;
}

//=============================================================================
// Description:
//      Create a new JPE user context.
//      User should use this context to do all JPEG encode related operations.
// Author:
//      Albert.Liao.
// Input:
//      pDev: JPEG hardware device handle
// Output:
//      pCtx: JPEG user context
//=============================================================================
JpeCtx_t* JpeCtxAcquire(JpeDev_t* pDev)
{
    JpeCtx_t*   pCtx = NULL;
    JpeOps_t*   pOps = NULL;
    JpeOpsCB_t* pOpsCB = NULL;

    int id = 0;
    pCtx = CamOsMemCalloc(1, sizeof(JpeCtx_t));
    if((pDev == NULL) || (pCtx == NULL))
    {
        JPE_MSG(JPE_MSG_ERR, "> CamOsMemCalloc Fail \n");
        if(pCtx)
        {
            CamOsMemRelease(pCtx);
            pCtx = NULL;
        }
        return pCtx;
    }

    pCtx->p_usrdt = NULL;

    do
    {
        pOpsCB = JpeOpsAcquire(id);
        if(pOpsCB == NULL)
        {
            JPE_MSG(JPE_MSG_ERR, "> JpeOpsAcquire Fail \n");
            break;
        }

        pOps = (JpeOps_t*)pOpsCB;
        pOps->pEncOutBuf    =   &pCtx->tEncOutBuf;
        pCtx->p_usrdt = CamOsMemCalloc(1, JPE_USER_DATA_SIZE);
        if(pCtx->p_usrdt == NULL)
        {
            JPE_MSG(JPE_MSG_ERR, "> CamOsMemCalloc Fail \n");
            break;
        }

        CamOsTsemInit(&pCtx->m_stream, 1);

        pCtx->i_state = JPE_CTX_STATE_NULL;
        pCtx->release = _JpeCtxRelease;
        pCtx->p_handle = pOpsCB;
        return pCtx;
    }
    while(0);

    if (pCtx->p_usrdt)
    {
        CamOsMemRelease(pCtx->p_usrdt);
        pCtx->p_usrdt = NULL;
    }
    if(pOpsCB)
    {
//        if(pOpsCB->release)
//            pOpsCB->release(pOpsCB);
        CamOsMemRelease(pOpsCB);
        pCtx->p_handle = NULL;
    }
    if(pCtx)
    {
        CamOsMemRelease(pCtx);
        pCtx = NULL;
    }
    return pCtx;
}


//=============================================================================
// Description:
//      Handle all JPEG encode related operations.
// Author:
//      Albert.Liao.
// Input:
//      pCtx: JPEG user context
//      cmd: command number
//      arg: a pointer from user-space, this pointer may be the address of JpeBufInfo_t, JpeCfg_t, JpeCaps_t, JpeBitstreamInfo_t
// Output:
//      JPE_IOC_RET_SUCCESS: Success
//      JPE_IOC_RET_FAIL: Failure
//=============================================================================
JPE_IOC_RET_STATUS_e JpeCtxActions(JpeCtx_t* pCtx, unsigned int cmd, void* arg)
{
    JpeCfg_t tJpeCfg;
    JpeBufInfo_t tOutBuf;
    JpeBitstreamInfo_t tStreamInfo;
    JpeCaps_t tCaps;

    JPE_IOC_RET_STATUS_e eStatus = JPE_IOC_RET_SUCCESS;
    switch(cmd)
    {
        case JPE_IOC_INIT:
            if(arg && copy_from_user(&tJpeCfg, (void *)arg, sizeof(JpeCfg_t)))
            {
                eStatus = JPE_IOC_RET_FAIL;
            }
            eStatus = _JpeCtxIoctlInit(pCtx, &tJpeCfg);
            break;

        case JPE_IOC_SET_CLOCKRATE:
            {
                int nClkSelect = (int)(arg);

                // 0: 288MHz  1: 216MHz  2: 54MHz  3: 27MHz
                if((nClkSelect>=0) && (nClkSelect<4))
                {
                    pCtx->nClkSelect = nClkSelect;
                    eStatus = JPE_IOC_RET_SUCCESS;
                }
                else
                {
                    eStatus = JPE_IOC_RET_FAIL;
                }
            }
            break;

        case JPE_IOC_SET_OUTBUF:
            if(arg && copy_from_user(&tOutBuf, (const void *)arg, sizeof(JpeBufInfo_t)))
            {
                JPE_MSG(JPE_MSG_ERR, "The buffer is not legal\n");
                eStatus = JPE_IOC_RET_FAIL;
            }

            eStatus = _JpeCtxSetOutputBuffer(pCtx, &tOutBuf);
            break;

        case JPE_IOC_ENCODE_FRAME:
            eStatus = _JpeCtxEncodeFrame(pCtx, &tStreamInfo);

            if(arg && copy_to_user((void*)arg, (void*)&tStreamInfo, sizeof(JpeBitstreamInfo_t)))
            {
                eStatus = JPE_IOC_RET_FAIL;
            }
            break;

        case JPE_IOC_GETBITS:
            eStatus = _JpeCtxGetBitInfo(pCtx, &tStreamInfo);

            if(arg && copy_to_user((void *)arg, &tStreamInfo, sizeof(JpeBitstreamInfo_t)))
            {
                eStatus = JPE_IOC_RET_FAIL;
            }
            break;

        case JPE_IOC_GET_CAPS:
            eStatus = _JpeCtxGetCapbilities(pCtx, &tCaps);

            if(arg && copy_to_user((void *)arg, &tCaps, sizeof(JpeCaps_t)))
            {
                eStatus = JPE_IOC_RET_FAIL;
            }
            break;

        default:
            eStatus = JPE_IOC_RET_FAIL;
            break;
    }
    return eStatus;
}
