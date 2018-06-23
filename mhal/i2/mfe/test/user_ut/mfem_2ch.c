
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <mdrv_msys_io.h>
#include <mdrv_verchk.h>
#include <mdrv_msys_io_st.h>
#include <mdrv_rqct_io.h>
#include <mdrv_mfe_io.h>
//#include <mhal_mfe_st.h>
#include <mhal_venc.h>
#include "mhal_ut_wrapper.h"
#include "mfe_utility.h"
#include "cam_os_wrapper.h"

#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define CLP(a,b,x)  (MIN(MAX(x,a),b))

#define CBR_UPPER_QP        48
#define CBR_LOWER_QP        12

typedef MSYS_DMEM_INFO msys_dmem;
typedef MSYS_ADDR_TRANSLATION_INFO addr_info;

#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

void *pMfeInternalBuf1VirtCh1 = NULL;
void *pMfeInternalBuf1PhysCh1 = NULL;
void *pMfeInternalBuf1MiuCh1 = NULL;
void *pMfeInternalBuf2VirtCh1 = NULL;
void *pMfeInternalBuf2PhysCh1 = NULL;
void *pMfeInternalBuf2MiuCh1 = NULL;
MHAL_VENC_InternalBuf_t stMfeIntrBufCh1;
void *pMfeInternalBuf1VirtCh2 = NULL;
void *pMfeInternalBuf1PhysCh2 = NULL;
void *pMfeInternalBuf1MiuCh2 = NULL;
void *pMfeInternalBuf2VirtCh2 = NULL;
void *pMfeInternalBuf2PhysCh2 = NULL;
void *pMfeInternalBuf2MiuCh2 = NULL;
MHAL_VENC_InternalBuf_t stMfeIntrBufCh2;

void setHeader(VOID* header, MS_U32 size) {
    MHAL_VENC_Version_t* ver = (MHAL_VENC_Version_t*)header;
    ver->u32Size = size;

    ver->s.u8VersionMajor = SPECVERSIONMAJOR;
    ver->s.u8VersionMinor = SPECVERSIONMINOR;
    ver->s.u8Revision = SPECREVISION;
    ver->s.u8Step = SPECSTEP;
}

MHAL_ErrCode_e checkHeader(VOID* header, MS_U32 size)
{
    MHAL_ErrCode_e eError = 0;

    MHAL_VENC_Version_t* ver;

    if (header == NULL) {
        CamOsPrintf("In %s the header is null\n",__func__);
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    ver = (MHAL_VENC_Version_t*)header;

    if(ver->u32Size != size) {
        CamOsPrintf("In %s the header has a wrong size %i should be %i\n",__func__,ver->u32Size,(int)size);
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    if(ver->s.u8VersionMajor != SPECVERSIONMAJOR ||
            ver->s.u8VersionMinor != SPECVERSIONMINOR) {
        CamOsPrintf("The version does not match\n");
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    return eError;
}

int _SetMfeParameterByMhalCh1(Mfe_param* pParam)
{
    ERROR err = ErrorNone;
    char szDmemName[20];
    VencParamUT param;


    /* Set MFE parameter by iocal */
    memset(&param, 0, sizeof(VencParamUT));

    MHAL_VENC_Resoluton_t ResCtl;
    memset(&ResCtl, 0, sizeof(ResCtl));

    param.type = E_MHAL_VENC_264_RESOLUTION;
    setHeader(&ResCtl, sizeof(ResCtl));
    ResCtl.nWidth = 320;
    ResCtl.nHeight = 240;
    ResCtl.stFmt = E_MI_VENC_FMT_NV12;

    param.param = (void *)&ResCtl;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
    }

    MHAL_VENC_RcInfo_t RcCtl;
    memset(&RcCtl, 0, sizeof(RcCtl));

    param.type = E_MHAL_VENC_264_RC;
    setHeader(&RcCtl, sizeof(RcCtl));

    RcCtl.eeRcMode = E_MHAL_VENC_RC_MODE_H264FIXQP;
    RcCtl.stAttrH264FixQp.u32SrcFrmRateNum = 30;
    RcCtl.stAttrH264FixQp.u32SrcFrmRateDen = 1;
    RcCtl.stAttrH264FixQp.u32Gop = 30;
    RcCtl.stAttrH264FixQp.IQp = 25;
    RcCtl.stAttrH264FixQp.u32PQp = 25;

    param.param = (void *)&RcCtl;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_RT_RC Error = %x\n",__func__,err);
    }


    /* Qurey MFE internal buffer size from driver by iocal */
    memset(&stMfeIntrBufCh1, 0, sizeof(MHAL_VENC_InternalBuf_t));
    setHeader(&stMfeIntrBufCh1, sizeof(stMfeIntrBufCh1));

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_QUERY_BUFSIZE, &stMfeIntrBufCh1);
    if( err )
    {
        CamOsPrintf("%s IOCTL_MMFE_MHAL_QUERY_BUFSIZE Error = %x\n", __func__, err);
    }


    /* Allocate MFE internal buffer and assign to driver by iocal */
    memset(szDmemName, 0, sizeof(szDmemName));
    snprintf(szDmemName, sizeof(szDmemName), "MFE-INTRAL-CH1");
    CamOsDirectMemAlloc(szDmemName, stMfeIntrBufCh1.u32IntrAlBufSize, &pMfeInternalBuf1VirtCh1, &pMfeInternalBuf1PhysCh1, &pMfeInternalBuf1MiuCh1);
    //CamOsPrintf("[%s] querysize = %d, pUserPtr = %p, nPhysAddr = %p\n", szDmemName, vencbuf.IntrAlBufSize, pUserPtr, nPhysAddr);

    memset(szDmemName, 0, sizeof(szDmemName));
    snprintf(szDmemName, sizeof(szDmemName), "MFE-INTRREF-CH1");
    CamOsDirectMemAlloc(szDmemName, stMfeIntrBufCh1.u32IntrRefBufSize, &pMfeInternalBuf2VirtCh1, &pMfeInternalBuf2PhysCh1, &pMfeInternalBuf2MiuCh1);
    //CamOsPrintf("[%s] querysize = %d, pUserPtr2 = %p, nPhysAddr2 = %p\n", szDmemName, vencbuf.IntrRefBufSize, pUserPtr2, nPhysAddr2);

    stMfeIntrBufCh1.pu8IntrAlVirBuf = (MS_U32 *)pMfeInternalBuf1VirtCh1;
    stMfeIntrBufCh1.phyIntrAlPhyBuf = (MS_PHYADDR *)pMfeInternalBuf1MiuCh1;
    stMfeIntrBufCh1.phyIntrRefPhyBuf = (MS_PHYADDR *)pMfeInternalBuf2MiuCh1;

    VencParamUT querysizeparm;
    memset(&querysizeparm, 0, sizeof(VencParamUT));

    querysizeparm.type = E_MHAL_VENC_IDX_STREAM_ON;
    setHeader(&stMfeIntrBufCh1, sizeof(stMfeIntrBufCh1));
    querysizeparm.param = (void*)&stMfeIntrBufCh1;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &querysizeparm);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
    }

    return err;
}

int _SetMfeParameterByMhalCh2(Mfe_param* pParam)
{
    ERROR err = ErrorNone;
    char szDmemName[20];
    VencParamUT param;


    /* Set MFE parameter by iocal */
    memset(&param, 0, sizeof(VencParamUT));

    MHAL_VENC_Resoluton_t ResCtl;
    memset(&ResCtl, 0, sizeof(ResCtl));

    param.type = E_MHAL_VENC_264_RESOLUTION;
    setHeader(&ResCtl, sizeof(ResCtl));
    ResCtl.nWidth = 320;
    ResCtl.nHeight = 240;
    ResCtl.stFmt = E_MI_VENC_FMT_NV12;

    param.param = (void *)&ResCtl;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
    }

    MHAL_VENC_RcInfo_t RcCtl;
    memset(&RcCtl, 0, sizeof(RcCtl));

    param.type = E_MHAL_VENC_264_RC;
    setHeader(&RcCtl, sizeof(RcCtl));

    RcCtl.eRcMode = E_MHAL_VENC_RC_MODE_H264FIXQP;
    RcCtl.stAttrH264FixQp.u32SrcFrmRateNum = 30;
    RcCtl.stAttrH264FixQp.u32SrcFrmRateDen = 1;
    RcCtl.stAttrH264FixQp.u32Gop = 30;
    RcCtl.stAttrH264FixQp.IQp = 35;
    RcCtl.stAttrH264FixQp.u32PQp = 35;

    param.param = (void *)&RcCtl;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_RT_RC Error = %x\n",__func__,err);
    }


    /* Qurey MFE internal buffer size from driver by iocal */
    memset(&stMfeIntrBufCh2, 0, sizeof(MHAL_VENC_InternalBuf_t));
    setHeader(&stMfeIntrBufCh2, sizeof(stMfeIntrBufCh2));

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_QUERY_BUFSIZE, &stMfeIntrBufCh2);
    if( err )
    {
        CamOsPrintf("%s IOCTL_MMFE_MHAL_QUERY_BUFSIZE Error = %x\n", __func__, err);
    }


    /* Allocate MFE internal buffer and assign to driver by iocal */
    memset(szDmemName, 0, sizeof(szDmemName));
    snprintf(szDmemName, sizeof(szDmemName), "MFE-INTRAL-CH2");
    CamOsDirectMemAlloc(szDmemName, stMfeIntrBufCh2.u32IntrAlBufSize, &pMfeInternalBuf1VirtCh2, &pMfeInternalBuf1PhysCh2, &pMfeInternalBuf1MiuCh2);
    //CamOsPrintf("[%s] querysize = %d, pUserPtr = %p, nPhysAddr = %p\n", szDmemName, vencbuf.IntrAlBufSize, pUserPtr, nPhysAddr);

    memset(szDmemName, 0, sizeof(szDmemName));
    snprintf(szDmemName, sizeof(szDmemName), "MFE-INTRREF-CH2");
    CamOsDirectMemAlloc(szDmemName, stMfeIntrBufCh2.u32IntrRefBufSize, &pMfeInternalBuf2VirtCh2, &pMfeInternalBuf2PhysCh2, &pMfeInternalBuf2MiuCh2);
    //CamOsPrintf("[%s] querysize = %d, pUserPtr2 = %p, nPhysAddr2 = %p\n", szDmemName, vencbuf.IntrRefBufSize, pUserPtr2, nPhysAddr2);

    stMfeIntrBufCh2.pu8IntrAlVirBuf = (MS_U32 *)pMfeInternalBuf1VirtCh2;
    stMfeIntrBufCh2.phyIntrAlPhyBuf = (MS_PHYADDR *)pMfeInternalBuf1MiuCh2;
    stMfeIntrBufCh2.phyIntrRefPhyBuf = (MS_PHYADDR *)pMfeInternalBuf2MiuCh2;

    VencParamUT querysizeparm;
    memset(&querysizeparm, 0, sizeof(VencParamUT));

    querysizeparm.type = E_MHAL_VENC_IDX_STREAM_ON;
    setHeader(&stMfeIntrBufCh2, sizeof(stMfeIntrBufCh2));
    querysizeparm.param = (void*)&stMfeIntrBufCh2;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &querysizeparm);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
    }

    return err;
}

int _GetMfeParameterByMhal(Mfe_param* pParam)
{
    ERROR err = ErrorNone;

    VencParamUT param;
    memset(&param, 0, sizeof(VencParamUT));

    MHAL_VENC_Resoluton_t ResCtl;
    memset(&ResCtl, 0, sizeof(ResCtl));

    CamOsPrintf("======[ %s ]======\n",__func__);


    param.type = E_MHAL_VENC_264_RESOLUTION;
    setHeader(&ResCtl, sizeof(ResCtl));
    param.param = (void *)&ResCtl;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_GET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Get E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
    }

    CamOsPrintf("[%s %d] ResCtl.nWidth = %d \n", __func__, __LINE__,ResCtl.nWidth);
    CamOsPrintf("[%s %d] ResCtl.nHeight = %d \n", __func__, __LINE__, ResCtl.nHeight);
    CamOsPrintf("[%s %d] ResCtl.stFmt = %d \n",__func__, __LINE__, ResCtl.stFmt);

    MHAL_VENC_RcInfo_t RcCtl;
    memset(&RcCtl, 0, sizeof(RcCtl));

    param.type = E_MHAL_VENC_264_RC;
    setHeader(&RcCtl, sizeof(RcCtl));

    param.param = (void *)&RcCtl;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_GET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_RT_RC Error = %x\n",__func__,err);
    }

    CamOsPrintf("[%s %d] RcCtl.eRcMode = %d \n", __func__, __LINE__,RcCtl.eRcMode);
    CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.u32SrcFrmRateNum = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.u32SrcFrmRateNum);
    CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.u32SrcFrmRateDen = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.u32SrcFrmRateDen);
    CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.u32Gop = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.u32Gop);
    CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.IQp = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.IQp);
    CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.u32PQp = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.u32PQp);

    return err;
}

int _RunMfe(Mfe_param* pParamCh1, Mfe_param* pParamCh2)
{
    int ret;
    int mfefdCh1 = open("/dev/mmfe", O_RDWR);
    int mfefdCh2 = open("/dev/mmfe", O_RDWR);
    FILE * fi = NULL;
    FILE * foCh1 = NULL;
    FILE * foCh2 = NULL;
    int pictw, picth, picts, ysize, csize;
    void* pYuvBufVitr = NULL;
    void* pYuvBufPhys = NULL;
    void* pYuvBufMiu = NULL;
    void* pBsBufVitrCh1 = NULL;
    void* pBsBufPhysCh1 = NULL;
    void* pBsBufMiuCh1 = NULL;
    void* pBsBufVitrCh2 = NULL;
    void* pBsBufPhysCh2 = NULL;
    void* pBsBufMiuCh2 = NULL;
    int length = 0;
    int err = 0;
    int totalCh1 = 0;
    int totalCh2 = 0;
    mmfe_parm paramCh1;
    mmfe_parm paramCh2;
    int frame = 1;
    MHAL_VENC_InOutBuf_t InOutBufCh1;
    MHAL_VENC_EncResult_t EncRetCh1;
    MHAL_VENC_InOutBuf_t InOutBufCh2;
    MHAL_VENC_EncResult_t EncRetCh2;


    pParamCh1->mfefd = mfefdCh1;
    if(pParamCh1->mfefd == NULL)
    {
        CamOsPrintf("Get \"/dev/mmfe\" is NULL\n");
        return 0;
    }

    pParamCh2->mfefd = mfefdCh2;
    if(pParamCh2->mfefd == NULL)
    {
        CamOsPrintf("Get \"/dev/mmfe\" is NULL\n");
        return 0;
    }

    do
    {
        if (mfefdCh1 < 0)
            break;
        if (mfefdCh2 < 0)
            break;
        fi = fopen(pParamCh1->inputPath, "rb");
        foCh1 = fopen(pParamCh1->outputPath, "wb");
        foCh2 = fopen(pParamCh2->outputPath, "wb");
        pictw = pParamCh1->basic.w;
        picth = pParamCh1->basic.h;
        if(picth%16 != 0)
            picth = picth+ picth%16;
        if (!strncmp(pParamCh1->basic.pixfm, "NV12", 4))
        {
            ysize = pictw*picth;
            csize = ysize/2;
        }
        if (!strncmp(pParamCh1->basic.pixfm, "NV21", 4))
        {
            ysize = pictw*picth;
            csize = ysize/2;
        }
        if (!strncmp(pParamCh1->basic.pixfm, "YUYV", 4))
        {
            ysize = pictw*picth*2;
            csize = 0;
        }

        sleep(1);

        do
        {   /* now, check arguments */
            if (fi == NULL || foCh1 == NULL || foCh2 == NULL)
                break;
            if ((pictw%16) != 0 || (picth%8) != 0)
                break;

            picts = ysize+csize;
            length = pictw*picth;


            CamOsDirectMemAlloc("MFE-IBUFF", picts, &pYuvBufVitr, &pYuvBufPhys, &pYuvBufMiu);

            if(pYuvBufVitr == NULL)
            {
                CamOsPrintf("!!!!!!!!! pixels is NULL !!!!!!!!!!\n");
            }

            CamOsDirectMemAlloc("MFE-OBUFF-CH1", length, &pBsBufVitrCh1, &pBsBufPhysCh1, &pBsBufMiuCh1);
            CamOsDirectMemAlloc("MFE-OBUFF-CH2", length, &pBsBufVitrCh2, &pBsBufPhysCh2, &pBsBufMiuCh2);

            memset(pYuvBufVitr, 128, picts);


            _SetMfeParameterByMhalCh1(pParamCh1);
            _SetMfeParameterByMhalCh2(pParamCh2);


            CamOsPrintf("---------------------\n");
            sleep(1);

            //_GetMfeParameterByMhal(pParamCh1);
            //_GetMfeParameterByMhal(pParamCh2);

            while (length == fread(pYuvBufVitr, 1, length, fi))
            {
                fread(pYuvBufVitr+ysize, 1, length/2, fi);

                InOutBufCh1.pu32RegBase0 = NULL;
                InOutBufCh1.pu32RegBase1 = NULL;
                InOutBufCh1.pCmdQ = &InOutBufCh1;
                InOutBufCh1.bRequestI = 0;
                InOutBufCh1.phyInputYUVBuf1 = (MS_U32 *)(intptr_t)pYuvBufMiu;
                InOutBufCh1.u32InputYUVBuf1Size = ysize;
                InOutBufCh1.phyInputYUVBuf2 = InOutBufCh1.phyInputYUVBuf1 + InOutBufCh1.u32InputYUVBuf1Size;
                InOutBufCh1.u32InputYUVBuf2Size = csize;
                InOutBufCh1.phyOutputBuf = (MS_U32 *)(intptr_t)pBsBufMiuCh1;
                InOutBufCh1.u32OutputBufSize = length;
                InOutBufCh1.pFlushCacheCb = NULL;

                InOutBufCh2.pu32RegBase0 = NULL;
                InOutBufCh2.pu32RegBase1 = NULL;
                InOutBufCh2.pCmdQ = &InOutBufCh1;
                InOutBufCh2.bRequestI = 0;
                InOutBufCh2.phyInputYUVBuf1 = (MS_U32 *)(intptr_t)pYuvBufMiu;
                InOutBufCh2.u32InputYUVBuf1Size = ysize;
                InOutBufCh2.phyInputYUVBuf2 = InOutBufCh2.phyInputYUVBuf1 + InOutBufCh2.u32InputYUVBuf1Size;
                InOutBufCh2.u32InputYUVBuf2Size = csize;
                InOutBufCh2.phyOutputBuf = (MS_U32 *)(intptr_t)pBsBufMiuCh2;
                InOutBufCh2.u32OutputBufSize = length;
                InOutBufCh2.pFlushCacheCb = NULL;

                ret = ioctl(mfefdCh1, IOCTL_MMFE_MHAL_ENCODE_ONE_FRAME, &InOutBufCh1);
                if (ret)
                {
                    CamOsPrintf("!! IOCTL_MMFE_ENCODE_NONBLOCKING failed (%d) !!\n", ret);
                }

                usleep(20000);

                ret = ioctl(mfefdCh2, IOCTL_MMFE_MHAL_ENCODE_ONE_FRAME, &InOutBufCh2);
                if (ret)
                {
                    CamOsPrintf("!! IOCTL_MMFE_ENCODE_NONBLOCKING failed (%d) !!\n", ret);
                }

                ret = ioctl(mfefdCh1, IOCTL_MMFE_MHAL_KICKOFF_CMDQ, NULL);
                if (ret)
                {
                    CamOsPrintf("!! IOCTL_MMFE_MHAL_KICKOFF_CMDQ failed (%d) !!\n", ret);
                }

                usleep(40000);

                ret = ioctl(mfefdCh1, IOCTL_MMFE_MHAL_ENCODE_FRAME_DONE, &EncRetCh1);
                if (ret)
                {
                    CamOsPrintf("!! IOCTL_MMFE_ACQUIRE_NONBLOCKINGA failed (%d) !!\n", ret);
                }

                usleep(20000);

                ret = ioctl(mfefdCh2, IOCTL_MMFE_MHAL_ENCODE_FRAME_DONE, &EncRetCh2);
                if (ret)
                {
                    CamOsPrintf("!! IOCTL_MMFE_ACQUIRE_NONBLOCKINGA failed (%d) !!\n", ret);
                }

                fwrite(pBsBufVitrCh1, 1, EncRetCh1.u32OutputBufUsed, foCh1);
                fwrite(pBsBufVitrCh2, 1, EncRetCh2.u32OutputBufUsed, foCh2);

                totalCh1 += EncRetCh1.u32OutputBufUsed;
                CamOsPrintf("frame:%d, size:%d \n", frame, EncRetCh1.u32OutputBufUsed);
                totalCh2 += EncRetCh2.u32OutputBufUsed;
                CamOsPrintf("frame:%d, size:%d \n", frame, EncRetCh2.u32OutputBufUsed);
                frame++;
            }

            paramCh1.type = E_MHAL_VENC_IDX_STREAM_OFF;
            err = ioctl(mfefdCh1, IOCTL_MMFE_MHAL_SET_PARAM, &paramCh1);
            if( err )
            {
                CamOsPrintf("%s Set E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
            }

            CamOsPrintf("total-size:%8d\n",totalCh1);

            paramCh2.type = E_MHAL_VENC_IDX_STREAM_OFF;
            err = ioctl(mfefdCh2, IOCTL_MMFE_MHAL_SET_PARAM, &paramCh2);
            if( err )
            {
                CamOsPrintf("%s Set E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
            }

            CamOsPrintf("total-size:%8d\n",totalCh2);
        }
        while (0);

        if (fi) fclose(fi);
        if (foCh1) fclose(foCh1);
        if (foCh2) fclose(foCh2);

        CamOsDirectMemRelease(pYuvBufVitr, picts);
        CamOsDirectMemRelease(pBsBufVitrCh1, length);
        CamOsDirectMemRelease(pBsBufVitrCh2, length);
        CamOsDirectMemRelease(pMfeInternalBuf1VirtCh1, stMfeIntrBufCh1.u32IntrAlBufSize);
        CamOsDirectMemRelease(pMfeInternalBuf2VirtCh1, stMfeIntrBufCh1.u32IntrRefBufSize);
        CamOsDirectMemRelease(pMfeInternalBuf1VirtCh2, stMfeIntrBufCh2.u32IntrAlBufSize);
        CamOsDirectMemRelease(pMfeInternalBuf2VirtCh2, stMfeIntrBufCh2.u32IntrRefBufSize);
    }
    while (0);

    if (mfefdCh1 > 0)  close(mfefdCh1);
    if (mfefdCh2 > 0)  close(mfefdCh2);

    return 0;

}

int main(int argc, char** argv) {
    int cmd;
    Mfe_param* pMfeI1 = (Mfe_param*)malloc(sizeof(Mfe_param));
    Mfe_param* pMfeI2 = (Mfe_param*)malloc(sizeof(Mfe_param));
    memset(pMfeI1,0,sizeof(Mfe_param));
    memset(pMfeI2,0,sizeof(Mfe_param));

    ReadDefaultConfig("avc.cfg", pMfeI1);
    ReadDefaultConfig("avc.cfg", pMfeI2);

    DisplayMenuSetting();
    do{
        CamOsPrintf("==>");
        cmd = getchar();
        switch(cmd){
            case 'f':
                SetInOutPath(pMfeI1);
                DisplayMenuSetting();
                break;
            case 's':
                SetMFEParameter(pMfeI1);
                DisplayMenuSetting();
                break;
            case 'e':
                snprintf(pMfeI1->outputPath, sizeof(pMfeI1->outputPath), "QVGA_out_Ch1.h264");
                snprintf(pMfeI2->outputPath, sizeof(pMfeI2->outputPath), "QVGA_out_Ch2.h264");
                _RunMfe(pMfeI1, pMfeI2);
                DisplayMenuSetting();
                break;
            case 'r':
                ReadDefaultConfig("avc.cfg", pMfeI1);
                break;
            case 'p':
                ShowCurrentMFESetting(pMfeI1);
                break;
            case 'h':
                DisplayMenuSetting();
                break;
            default:
                break;
        }
    }while('q' != cmd );

    free(pMfeI1);
    free(pMfeI2);
    return 0;
}
