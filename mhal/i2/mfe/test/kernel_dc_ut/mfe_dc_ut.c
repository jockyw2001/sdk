
#include <linux/module.h>
#include <linux/interrupt.h>
#include "mhal_venc.h"
//#include "mhal_mfe.h"
#include "mhal_mfe_dc.h"
#include "cam_os_wrapper.h"
#include "NV12_qvga_10frame.h"
#include "H264_qvga_10frame_qp25.h"
#include "H264_qvga_10frame_qp45.h"
#include "file_access.h"

MODULE_LICENSE("GPL");

#define YUV_WIDTH   320
#define YUV_HEIGHT  240
#define YUV_FRAME_SIZE  (YUV_WIDTH*YUV_HEIGHT*3/2)

#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

CamOsTsem_t stDev0FrameDone;

void* gMhalMfeDev0 = NULL;
void* gMhalMfeInst0 = NULL;

void setHeader(VOID* header, MS_U32 size) {
    MHAL_VENC_Version_t* ver = (MHAL_VENC_Version_t*)header;
    ver->u32Size = size;

    ver->s.u8VersionMajor = SPECVERSIONMAJOR;
    ver->s.u8VersionMinor = SPECVERSIONMINOR;
    ver->s.u8Revision = SPECREVISION;
    ver->s.u8Step = SPECSTEP;
}

void print_hex(char* title, void* buf, int num) {
        int i;
        char *data = (char *) buf;

        CamOsPrintf(
                        "%s\nOffset(h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n"
                        "----------------------------------------------------------",
                        title);
        for (i = 0; i < num; i++) {
                if (i % 16 == 0)
                    CamOsPrintf("\n%08X   ", i);
                CamOsPrintf("%02X ", data[i]);
        }
        CamOsPrintf("\n");
}

int _CompareArrays(u8* a, u8* b, int len)
{
    int i = 0;
    for(i = 0; i < len; i++)
    {
        if (a[i] != b[i])
            return -1;
    }
    return 0;
}

void _MfeEncCb(MHAL_VENC_EncResult_t *pstEncRet)
{
    CamOsPrintf("_MfeEncCb %d\n", pstEncRet->u32OutputBufUsed);
    CamOsTsemUp(&stDev0FrameDone);
}

static int
MfeProbe(
//    struct platform_device* pdev)
void)
{
    MHAL_VENC_Resoluton_t ResCtl;
    MHAL_VENC_RcInfo_t RcCtl;
    MHAL_VENC_InternalBuf_t stMfeIntrBuf;
    MHAL_VENC_InOutBuf_t Inst0InOutBuf;
    MHAL_VENC_EncResult_t Inst0EncRet;
    void* pYuvBufVitr = NULL;
    void* pYuvBufPhys = NULL;
    void* pYuvBufMiu = NULL;
    void* pInst0BsBufVitr = NULL;
    void* pInst0BsBufPhys = NULL;
    void* pInst0BsBufMiu = NULL;
    void *pInst0MfeInternalBuf1Virt = NULL;
    void *pInst0MfeInternalBuf1Phys = NULL;
    void *pInst0MfeInternalBuf1Miu = NULL;
    void *pInst0MfeInternalBuf2Virt = NULL;
    void *pInst0MfeInternalBuf2Phys = NULL;
    void *pInst0MfeInternalBuf2Miu = NULL;
    u32 u32Inst0TotalLen = 0;
    int frame_cnt;
    struct file *fd;

    CamOsTsemInit(&stDev0FrameDone, 0);

    if (MHAL_MFE_DC_CreateDevice(&gMhalMfeDev0))
    {
        CamOsPrintf("[KUT] MHAL_MFE_CreateDevice Fail\n");
        return 0;
    }

    if (MHAL_MFE_DC_CreateInstance(gMhalMfeDev0, &gMhalMfeInst0))
    {
        CamOsPrintf("[KUT] MHAL_MFE_DC_CreateInstance Fail\n");
        return 0;
    }


    /* Set MFE parameter */
    memset(&ResCtl, 0, sizeof(ResCtl));
    setHeader(&ResCtl, sizeof(ResCtl));
    ResCtl.u32Width = YUV_WIDTH;
    ResCtl.u32Height = YUV_HEIGHT;
    ResCtl.eFmt = E_MHAL_VENC_FMT_NV12;
    MHAL_MFE_DC_SetParam(gMhalMfeInst0, E_MHAL_VENC_264_RESOLUTION, &ResCtl);


    memset(&ResCtl, 0, sizeof(ResCtl));
    setHeader(&ResCtl, sizeof(ResCtl));
    MHAL_MFE_DC_GetParam(gMhalMfeInst0, E_MHAL_VENC_264_RESOLUTION, &ResCtl);
    CamOsPrintf("[KUT] resolution %dx%d\n", ResCtl.u32Width, ResCtl.u32Height);


    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    RcCtl.eRcMode = E_MHAL_VENC_RC_MODE_H264FIXQP;
    RcCtl.stAttrH264FixQp.u32SrcFrmRateNum = 30;
    RcCtl.stAttrH264FixQp.u32SrcFrmRateDen = 1;
    RcCtl.stAttrH264FixQp.u32Gop = 10;
    RcCtl.stAttrH264FixQp.u32IQp = 45;
    RcCtl.stAttrH264FixQp.u32PQp = 45;
    MHAL_MFE_DC_SetParam(gMhalMfeInst0, E_MHAL_VENC_264_RC, &RcCtl);


    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    MHAL_MFE_DC_GetParam(gMhalMfeInst0, E_MHAL_VENC_264_RC, &RcCtl);
    CamOsPrintf("[KUT] RC %d %d %d\n", RcCtl.stAttrH264FixQp.u32Gop, RcCtl.stAttrH264FixQp.u32IQp, RcCtl.stAttrH264FixQp.u32PQp);


    memset(&stMfeIntrBuf, 0, sizeof(MHAL_VENC_InternalBuf_t));
    setHeader(&stMfeIntrBuf, sizeof(stMfeIntrBuf));
    MHAL_MFE_DC_QueryBufSize(gMhalMfeInst0, &stMfeIntrBuf);
    CamOsPrintf("[KUT] internal buffer size %d %d\n", stMfeIntrBuf.u32IntrAlBufSize, stMfeIntrBuf.u32IntrRefBufSize);


    /* Allocate MFE internal buffer and assign to driver */
    CamOsDirectMemAlloc("MFEINTRALBUF0", stMfeIntrBuf.u32IntrAlBufSize, &pInst0MfeInternalBuf1Virt, &pInst0MfeInternalBuf1Phys, &pInst0MfeInternalBuf1Miu);
    CamOsDirectMemAlloc("MFEINTRREFBUF0", stMfeIntrBuf.u32IntrRefBufSize, &pInst0MfeInternalBuf2Virt, &pInst0MfeInternalBuf2Phys, &pInst0MfeInternalBuf2Miu);
    stMfeIntrBuf.pu8IntrAlVirBuf = (MS_U8 *)pInst0MfeInternalBuf1Virt;
    stMfeIntrBuf.phyIntrAlPhyBuf = (MS_PHYADDR)(u32)pInst0MfeInternalBuf1Miu;
    stMfeIntrBuf.phyIntrRefPhyBuf = (MS_PHYADDR)(u32)pInst0MfeInternalBuf2Miu;
    MHAL_MFE_DC_SetParam(gMhalMfeInst0, E_MHAL_VENC_IDX_STREAM_ON, &stMfeIntrBuf);


    CamOsDirectMemAlloc("MFEIBUFF", YUV_FRAME_SIZE, &pYuvBufVitr, &pYuvBufPhys, &pYuvBufMiu);
    CamOsDirectMemAlloc("MFEOBUFF0", YUV_FRAME_SIZE, &pInst0BsBufVitr, &pInst0BsBufPhys, &pInst0BsBufMiu);
    Inst0InOutBuf.pu32RegBase0 = NULL;
    Inst0InOutBuf.pu32RegBase1 = NULL;
    Inst0InOutBuf.pCmdQ = NULL;
    Inst0InOutBuf.bRequestI = 0;
    Inst0InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(u32)pYuvBufMiu;
    Inst0InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
    Inst0InOutBuf.phyInputYUVBuf2 = Inst0InOutBuf.phyInputYUVBuf1 + Inst0InOutBuf.u32InputYUVBuf1Size;
    Inst0InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;
    Inst0InOutBuf.phyOutputBuf = (MS_PHYADDR)(u32)pInst0BsBufMiu;
    Inst0InOutBuf.virtOutputBuf = (MS_PHYADDR)(u32)pInst0BsBufVitr;
    Inst0InOutBuf.u32OutputBufSize = YUV_FRAME_SIZE;
    Inst0InOutBuf.pFlushCacheCb = NULL;


    fd = OpenFile("/tmp/dc.264", O_RDONLY | O_CREAT, 0);

    for (frame_cnt=0; frame_cnt < (sizeof(gYUV)/YUV_FRAME_SIZE); frame_cnt++)
    {
        memcpy(pYuvBufVitr, gYUV+frame_cnt*YUV_FRAME_SIZE, YUV_FRAME_SIZE);
        MHAL_MFE_DC_EncodeFrame(gMhalMfeInst0, &Inst0InOutBuf, _MfeEncCb);
        CamOsTsemDown(&stDev0FrameDone);    // wait encode frame done
        MHAL_MFE_DC_EncodeFrameDone(gMhalMfeInst0, &Inst0EncRet);

        CamOsPrintf("frame:%d, size:%d \n", frame_cnt, Inst0EncRet.u32OutputBufUsed);
        //print_hex("bitstream", pInst0BsBufVitr, 128);
        WriteFile(fd, pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed);
        if(Inst0EncRet.u32OutputBufUsed && _CompareArrays((u8 *)(gH264_qp45 + u32Inst0TotalLen), (u8 *)pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed) == 0)
            CamOsPrintf("Inst0 Encode Result Compare OK!!!\n");
        else
            CamOsPrintf("Inst0 Encode Result Compare Fail!!!\n");
        u32Inst0TotalLen += Inst0EncRet.u32OutputBufUsed;
    }

    CloseFile(fd);

    if (MHAL_MFE_DC_DestroyInstance(gMhalMfeInst0))
    {
        CamOsPrintf("[KUT] MHAL_MFE_DC_DestroyInstance Fail\n");
        return 0;
    }

    if (MHAL_MFE_DC_DestroyDevice(gMhalMfeDev0))
    {
        CamOsPrintf("[KUT] MHAL_MFE_DC_DestroyDevice Fail\n");
        return 0;
    }

    CamOsDirectMemRelease(pYuvBufVitr, YUV_FRAME_SIZE);
    CamOsDirectMemRelease(pInst0BsBufVitr, YUV_FRAME_SIZE);
    CamOsDirectMemRelease(pInst0MfeInternalBuf1Virt, stMfeIntrBuf.u32IntrAlBufSize);
    CamOsDirectMemRelease(pInst0MfeInternalBuf2Virt, stMfeIntrBuf.u32IntrRefBufSize);

    return 0;
}

static int
MfeRemove(
//    struct platform_device* pdev)
    void)
{
    return 0;
}

/*static const struct of_device_id MfeMatchTables[] = {
    { .compatible = "mstar,mfe" },
    {},
};

static struct platform_driver MfePdrv = {
    .probe = MfeProbe,
    .remove = MfeRemove,
    .driver = {
        .name = "mmfe",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(MfeMatchTables),
    }
};*/

static int  __init mmfe_init(void)
{
    //return platform_driver_register(&MfePdrv);
    MfeProbe();
    return 0;
}

static void __exit mmfe_exit(void)
{
    //platform_driver_unregister(&MfePdrv);
    MfeRemove();
}

module_init(mmfe_init);
module_exit(mmfe_exit);
