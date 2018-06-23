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

#if defined(__linux__)
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>

#include "drv_jpe_enc.h"
#include "cam_os_wrapper.h"
#include <poll.h>

#elif defined(__I_SW__)
#include "cam_os_wrapper.h"
#include "drv_jpe_io.h"
#include "drv_jpe_io_st.h"
#include "drv_jpe_module.h"
#include "drv_jpe_enc.h"

#endif

#define JPE_STRINGIFY(x) #x
#define JPE_TOSTRING(x) JPE_STRINGIFY(x)


#if 0
static void _DumpCapabilities(JpeCaps_t *pCaps)
{
    if(pCaps)
    {
        CamOsPrintf("u32JpeId = %d\n",               pCaps->u32JpeId);
        CamOsPrintf("nRefYLogAddrAlign[0] = %d\n",   pCaps->nRefYLogAddrAlign[0]);
        CamOsPrintf("nRefYLogAddrAlign[1] = %d\n",   pCaps->nRefYLogAddrAlign[1]);
        CamOsPrintf("nRefCLogAddrAlign[0] = %d\n",   pCaps->nRefCLogAddrAlign[0]);
        CamOsPrintf("nRefCLogAddrAlign[1] = %d\n",   pCaps->nRefCLogAddrAlign[1]);
        CamOsPrintf("nOutBufSLogAddrAlign = %d\n",   pCaps->nOutBufSLogAddrAlign);
        CamOsPrintf("nSclHandShakeSupport = %d\n",   pCaps->nSclHandShakeSupport);
        CamOsPrintf("nCodecSupport = %d\n",          pCaps->nCodecSupport);

        if(JPE_IBUF_FRAME_MODE == pCaps->nBufferModeSupport)
        {
            CamOsPrintf("nBufferModeSupport = %s\n",     JPE_TOSTRING(JPE_IBUF_FRAME_MODE));
        }
        else
        {
            CamOsPrintf("nBufferModeSupport = %s\n",     JPE_TOSTRING(JPE_IBUF_ROW_MODE));
        }
    }
}

static void _DumpBitInfo(JpeBitstreamInfo_t *pBitInfo)
{
    if(pBitInfo)
    {
        CamOsPrintf("pBitInfo nAddr=%p, nOrigSize=%d nOutputSize=%d\n",   pBitInfo->nAddr, pBitInfo->nOrigSize, pBitInfo->nOutputSize);
        switch(pBitInfo->eState)
        {
            case JPE_IDLE_STATE:
                CamOsPrintf("eState=%d %s\n", pBitInfo->eState, JPE_TOSTRING(JPE_IDLE_STATE));
                break;
            case JPE_BUSY_STATE:
                CamOsPrintf("eState=%d %s\n", pBitInfo->eState, JPE_TOSTRING(JPE_BUSY_STATE));
                break;
            case JPE_FRAME_DONE_STATE:
                CamOsPrintf("eState=%d %s\n", pBitInfo->eState, JPE_TOSTRING(JPE_FRAME_DONE_STATE));
                break;
            case JPE_OUTBUF_FULL_STATE:
                CamOsPrintf("eState=%d %s\n", pBitInfo->eState, JPE_TOSTRING(JPE_OUTBUF_FULL_STATE));
                break;
            case JPE_INBUF_FULL_STATE:
                CamOsPrintf("eState=%d %s\n", pBitInfo->eState, JPE_TOSTRING(JPE_INBUF_FULL_STATE));
                break;
        }
    }
}
#endif

#if 0
static void _DumpConfig(JpeCfg_t *pJpeCfg)
{
    if(pJpeCfg)
    {
        switch(pJpeCfg->eInBufMode)
        {
            case JPE_IBUF_ROW_MODE:
                CamOsPrintf("eInBufMode=%s\n", JPE_TOSTRING(JPE_IBUF_ROW_MODE));
                break;
            case JPE_IBUF_FRAME_MODE:
                CamOsPrintf("eInBufMode=%s\n", JPE_TOSTRING(JPE_IBUF_FRAME_MODE));
                break;
        }

        switch(pJpeCfg->eRawFormat)
        {
            case JPE_RAW_YUYV:
                CamOsPrintf("eRawFormat=%s\n", JPE_TOSTRING(JPE_RAW_YUYV));
                break;
            case JPE_RAW_YVYU:
                CamOsPrintf("eRawFormat=%s\n", JPE_TOSTRING(JPE_RAW_YVYU));
                break;
            case JPE_RAW_NV12:
                CamOsPrintf("eRawFormat=%s\n", JPE_TOSTRING(JPE_RAW_NV12));
                break;
            case JPE_RAW_NV21:
                CamOsPrintf("eRawFormat=%s\n", JPE_TOSTRING(JPE_RAW_NV21));
                break;
        }

        CamOsPrintf("eCodecFormat = %d\n",       pJpeCfg->eCodecFormat);
        CamOsPrintf("nWidth, nHeight = %d,%d \n", pJpeCfg->nWidth, pJpeCfg->nHeight);
        CamOsPrintf("nQScale = %d \n",           pJpeCfg->nQScale);
        CamOsPrintf("InBuf[0].nAddr = 0x%x\n",     (int)pJpeCfg->InBuf[0].nAddr);
        CamOsPrintf("InBuf[0].nSize = 0x%x\n",     (int)pJpeCfg->InBuf[0].nSize);
        CamOsPrintf("InBuf[1].nAddr = 0x%x\n",     (int)pJpeCfg->InBuf[1].nAddr);
        CamOsPrintf("InBuf[1].nSize = 0x%x\n",     (int)pJpeCfg->InBuf[1].nSize);
        CamOsPrintf("OutBuf.nAddr = 0x%x\n",       (int)pJpeCfg->OutBuf.nAddr);
        CamOsPrintf("OutBuf.nSize = 0x%x\n",       (int)pJpeCfg->OutBuf.nSize);
    }
}
#endif


// input/output buffer should be allocated and assigned before invoke encoder
#if defined(__linux__)
void JpeEncode(JpeParam_t* pParam, int nDev)
{
    /* validate params */
    int nRet = JPE_IOC_RET_SUCCESS;
    JpeCaps_t tCaps;
    JpeBufInfo_t tOutBuf;
    JpeBitstreamInfo_t tBitInfo;
    JpeCfg_t *pJpeCfg = &pParam->tJpeCfg;

#if 0
    CamOsPrintf("Input Buffer Address : 0x%08x \n", (unsigned int)pJpeCfg->InBuf[JPE_COLOR_PLAN_LUMA].nAddr);
    CamOsPrintf("Output Buffer Address : 0x%08x \n", (unsigned int)pJpeCfg->OutBuf.nAddr);
    CamOsPrintf("Current JPEG Config\n");
    _DumpConfig(pJpeCfg);
#endif

    do
    {
        if(nDev <= 0)
        {
            CamOsPrintf("invalid nDev\n");
            return;
        }

        tOutBuf.nAddr = pJpeCfg->OutBuf.nAddr;
        tOutBuf.nSize = pJpeCfg->OutBuf.nSize;

        do
        {
            nRet = ioctl(nDev, JPE_IOC_GET_CAPS, &tCaps);
            if(nRet != JPE_IOC_RET_SUCCESS)
            {
                CamOsPrintf("> JPE_IOC_GET_CAPS Fail , err %d\n", nRet);
                break;
            }

            nRet = ioctl(nDev, JPE_IOC_INIT, pJpeCfg);
            if(nRet != JPE_IOC_RET_SUCCESS)
            {
                CamOsPrintf("> JPE_IOC_INIT Fail , err %d\n", nRet);
                break;
            }

#if JPE_DRIVER_VER == 2
            {
                unsigned long nClkSelect = 0;
                nClkSelect = pParam->nClkSelect;
                nRet = ioctl(nDev, JPE_IOC_SET_CLOCKRATE, nClkSelect);
                if(nRet != JPE_IOC_RET_SUCCESS)
                {
                    CamOsPrintf("> JPE_IOC_SET_CLOCKRATE Fail , err %d\n", nRet);
                    break;
                }
           }
#endif

            /* assign a output buffer */
            nRet = ioctl(nDev, JPE_IOC_SET_OUTBUF, &tOutBuf);
            if(nRet != JPE_IOC_RET_SUCCESS)
            {
                CamOsPrintf("> JPE_IOC_SET_OUTBUF Fail , err %d\n", nRet);
                break;
            }

            /* HW would continue to encode automatically if previous status is JPE_OUTBUF_FULL_STATE
             * and JPE_IOC_SET_OUTBUF is set. So JPE_IOC_ENCODE_FRAME is set in the beginning for frame
             */
            /* compress one frame */
            nRet = ioctl(nDev, JPE_IOC_ENCODE_FRAME, NULL);
            if(nRet != JPE_IOC_RET_SUCCESS)
            {
                CamOsPrintf("> JPE_IOC_ENCODE_FRAME Fail , err %d\n", nRet);
                break;
            }

//TODO: Remove poll operation when new JPE driver is applied
#if JPE_DRIVER_VER == 1
POLL_AGAIN:
            {
                int nMaxPollCount = 0;
                struct pollfd pfd;

                pfd.fd = nDev;
                pfd.events = POLLIN | POLLRDNORM;
                nRet = poll(&pfd, 1, 20); // wait 20ms second.
                if(nRet == -1)
                {
                    CamOsPrintf("Polling error!!\n");
                }
                else if(nRet == 0)
                {
                    usleep(5000); // 5ms
                    if(nMaxPollCount<3)
                    {
                        nMaxPollCount++;
                        goto POLL_AGAIN;
                    }
                    CamOsPrintf("Polling timeout!! Already try %d times\n", nMaxPollCount);
                }
                else if(pfd.revents & (POLLIN | POLLRDNORM))
                {
                    CamOsPrintf("Data Generated!!\n");
                }
                else
                {
                    CamOsPrintf("Unexpected result!!\n");
                }
            }
#endif

            nRet = ioctl(nDev, JPE_IOC_GETBITS, &tBitInfo);
            if (nRet)
            {
                CamOsPrintf("%s: JPE_IOC_GETBITS error(%d)\n", __func__, nRet);
                break;
            }

#if 0
            CamOsPrintf(" tBitInfo is \n");
            _DumpBitInfo(&tBitInfo);
            CamOsPrintf(" Encode END\n");
#endif
            pParam->nEncodeSize = tBitInfo.nOutputSize;

        }
        while(0);

        close(nDev);
    }
    while(0);
}
#endif



#if defined(__I_SW__)
void JpeEncode(JpeParam_t* pParam, JpeDev_t* pDev)
{
    /* validate params */
    int nRet;
    JpeCaps_t tCaps;
    JpeBufInfo_t tOutBuf;
    JpeBitstreamInfo_t tBitInfo;
    JpeCfg_t *pJpeCfg = &pParam->tJpeCfg;
    JpeCtx_t* pCtx = NULL;

#if 0
    CamOsPrintf("Input Buffer Address : 0x%08x \n", (unsigned int)pJpeCfg->InBuf[JPE_COLOR_PLAN_LUMA].nAddr);
    CamOsPrintf("Output Buffer Address : 0x%08x \n", (unsigned int)pJpeCfg->OutBuf.nAddr);
    CamOsPrintf("Current JPEG Config\n");
    _DumpConfig(pJpeCfg);
#endif

    do
    {
        if(pDev == NULL)
        {
            CamOsPrintf("pDev == NULL\n");
            break;
        }

        pCtx = JpeOpen(pDev, pCtx);
        if(pCtx == NULL)
        {
            CamOsPrintf("pCtx == NULL\n");
            break;
        }

        pCtx->nClkSelect = pParam->nClkSelect;
        tOutBuf.nAddr = pJpeCfg->OutBuf.nAddr;
        tOutBuf.nSize = pJpeCfg->OutBuf.nSize;

        do
        {
            nRet = JpeCtxActions(pCtx, JPE_IOC_GET_CAPS, &tCaps);
            if(JPE_IOC_RET_SUCCESS != nRet)
            {
                CamOsPrintf("> JPE_IOC_GET_CAPS Fail , err %d\n", nRet);
                break;
            }

            nRet = JpeCtxActions(pCtx, JPE_IOC_INIT, pJpeCfg);
            if(JPE_IOC_RET_SUCCESS != nRet)
            {
                CamOsPrintf("> JPE_IOC_INIT Fail , err %d\n", nRet);
                //_DumpConfig(pJpeCfg);
                break;
            }

            nRet = JpeCtxActions(pCtx, JPE_IOC_SET_OUTBUF, &tOutBuf);
            if(JPE_IOC_RET_SUCCESS != nRet)
            {
                CamOsPrintf("> JPE_IOC_SET_OUTBUF Fail , err %d\n", nRet);
                break;
            }

            nRet = JpeCtxActions(pCtx, JPE_IOC_ENCODE_FRAME, &tBitInfo);
            if(JPE_IOC_RET_SUCCESS != nRet)
            {
                CamOsPrintf("> JPE_IOC_ENCODE_FRAME Fail , err %d\n", nRet);
                break;
            }

            // If user need get encode data in other function.
            if(JPE_IOC_RET_SUCCESS != JpeCtxActions(pCtx, JPE_IOC_GETBITS, &tBitInfo))
            {
                CamOsPrintf("> JPE_IOC_GETBITS Fail , err %d\n", nRet);
                break;
            }
#if 0
            CamOsPrintf(" tBitInfo is \n");
            _DumpBitInfo(&tBitInfo);
            CamOsPrintf(" Encode END\n");
#endif
            pParam->nEncodeSize = tBitInfo.nOutputSize;

        }
        while(0);
        JpeRelease(pDev, pCtx);
    }
    while(0);
}
#endif