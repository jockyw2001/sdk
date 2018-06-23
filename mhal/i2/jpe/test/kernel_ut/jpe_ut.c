
#include <linux/module.h>
#include <linux/interrupt.h>
#include "mhal_venc.h"
#include "mhal_jpe.h"
#include "cam_os_wrapper.h"
#include "NV12_cif_frames.h"
#include "file_access.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

#define YUV_WIDTH   352
#define YUV_HEIGHT  288
#define YUV_FRAME_SIZE  (YUV_WIDTH*YUV_HEIGHT*3/2)

#define ENCODE_MULTI_STREAM

#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

static unsigned int partial_out = 0;
module_param(partial_out, uint, S_IRUGO|S_IWUSR);

CamOsTsem_t stDev0FrameDone;

void* gMhalJpeDev0 = NULL;
void* gMhalJpeInst0 = NULL;
#ifdef ENCODE_MULTI_STREAM
void* gMhalJpeInst1 = NULL;
#endif

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

    CamOsPrintf("%s\nOffset(h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n"
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

void JpeNotifyCallbackFunc(unsigned long nBufAddr, unsigned long nOffset, unsigned long nNotifySize, unsigned short bFrameDone)
{
    CamOsPrintf("%s: nBufAddr = 0x%x, nOffset = 0x%x, nNotifySize = 0x%x, bFrameDone = %u\n", __FUNCTION__, nBufAddr, nOffset, nNotifySize, bFrameDone);
}

static irqreturn_t _JpeUtIsr(int irq, void* priv)
{
    CamOsPrintf("_JpeUtIsr\n");
    if (MHAL_JPE_IsrProc(priv) == 0)
        CamOsTsemUp(&stDev0FrameDone);

    return IRQ_HANDLED;
}

static int
JpeProbe(
//    struct platform_device* pdev)
void)
{
    MHAL_VENC_ParamInt_t param;
    MHAL_VENC_Resoluton_t ResCtl;
    MHAL_VENC_RcInfo_t RcCtl;
    MHAL_VENC_InternalBuf_t stJpeIntrBuf;
    MHAL_VENC_InOutBuf_t Inst0InOutBuf;
    MHAL_VENC_EncResult_t Inst0EncRet;
    void* pYuvBufVitr = NULL;
    void* pYuvBufPhys = NULL;
    void* pYuvBufMiu = NULL;
    void* pInst0BsBufVitr = NULL;
    void* pInst0BsBufPhys = NULL;
    void* pInst0BsBufMiu = NULL;
    u32 u32Inst0TotalLen = 0;
#ifdef ENCODE_MULTI_STREAM
    MHAL_VENC_InOutBuf_t Inst1InOutBuf;
    MHAL_VENC_EncResult_t Inst1EncRet;
    void* pInst1BsBufVitr = NULL;
    void* pInst1BsBufPhys = NULL;
    void* pInst1BsBufMiu = NULL;
    u32 u32Inst1TotalLen = 0;
#endif
    MHAL_VENC_PartialNotify_t PartialNotify;
    int frame_cnt;
    int JpeIrq = 0;
    struct file *fd;
    char szFileName[64];

    CamOsTsemInit(&stDev0FrameDone, 0);

    if (MHAL_JPE_CreateDevice(0, &gMhalJpeDev0))
    {
        CamOsPrintf("[KUT] MHAL_JPE_CreateDevice Fail\n");
        return 0;
    }

    setHeader(&param, sizeof(param));
    if (MHAL_JPE_GetDevConfig(gMhalJpeDev0, E_MHAL_VENC_HW_IRQ_NUM, &param))
    {
        CamOsPrintf("[KUT] MHAL_JPE_GetDevConfig Fail\n");
        return 0;
    }

    JpeIrq = param.u32Val;
    CamOsPrintf("[KUT] irq %d\n", JpeIrq);
    if (0 != request_irq(JpeIrq, _JpeUtIsr, IRQF_SHARED, "_JpeUtIsr", gMhalJpeDev0))
    {
        CamOsPrintf("[KUT] request_irq(%d) Fail\n", param.u32Val);
        return 0;
    }

    setHeader(&param, sizeof(param));
    if (MHAL_JPE_GetDevConfig(gMhalJpeDev0, E_MHAL_VENC_HW_CMDQ_BUF_LEN, &param))
    {
        CamOsPrintf("[KUT] MHAL_JPE_GetDevConfig Fail\n");
        return 0;
    }


    if (MHAL_JPE_CreateInstance(gMhalJpeDev0, &gMhalJpeInst0))
    {
        CamOsPrintf("[KUT] MHAL_JPE_CreateInstance Fail\n");
        return 0;
    }


    /* Set JPE parameter */
    memset(&ResCtl, 0, sizeof(ResCtl));
    setHeader(&ResCtl, sizeof(ResCtl));
    ResCtl.u32Width = YUV_WIDTH;
    ResCtl.u32Height = YUV_HEIGHT;
    ResCtl.eFmt = E_MHAL_VENC_FMT_NV12;
    MHAL_JPE_SetParam(gMhalJpeInst0, E_MHAL_VENC_JPEG_RESOLUTION, &ResCtl);


    memset(&ResCtl, 0, sizeof(ResCtl));
    setHeader(&ResCtl, sizeof(ResCtl));
    MHAL_JPE_GetParam(gMhalJpeInst0, E_MHAL_VENC_JPEG_RESOLUTION, &ResCtl);
    CamOsPrintf("[KUT] resolution %dx%d\n", ResCtl.u32Width, ResCtl.u32Height);


    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    MHAL_JPE_GetParam(gMhalJpeInst0, E_MHAL_VENC_JPEG_RC, &RcCtl);
    RcCtl.stAttrMJPGRc.u32Qfactor = 90;
    MHAL_JPE_SetParam(gMhalJpeInst0, E_MHAL_VENC_JPEG_RC, &RcCtl);


    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    MHAL_JPE_GetParam(gMhalJpeInst0, E_MHAL_VENC_JPEG_RC, &RcCtl);
    CamOsPrintf("[KUT] RC %d\n", RcCtl.stAttrMJPGRc.u32Qfactor);

    if (partial_out)
    {
        memset(&PartialNotify, 0, sizeof(PartialNotify));
        setHeader(&PartialNotify, sizeof(PartialNotify));
        PartialNotify.u32NotifySize = partial_out;
        PartialNotify.notifyFunc = JpeNotifyCallbackFunc;
        MHAL_JPE_SetParam(gMhalJpeInst0, E_MHAL_VENC_PARTIAL_NOTIFY, &PartialNotify);


        memset(&PartialNotify, 0, sizeof(PartialNotify));
        setHeader(&PartialNotify, sizeof(PartialNotify));
        MHAL_JPE_GetParam(gMhalJpeInst0, E_MHAL_VENC_PARTIAL_NOTIFY, &PartialNotify);
        CamOsPrintf("[KUT] partial notify size %u\n", PartialNotify.u32NotifySize);
    }


    memset(&stJpeIntrBuf, 0, sizeof(MHAL_VENC_InternalBuf_t));
    setHeader(&stJpeIntrBuf, sizeof(stJpeIntrBuf));
    MHAL_JPE_QueryBufSize(gMhalJpeInst0, &stJpeIntrBuf);
    CamOsPrintf("[KUT] internal buffer size %d %d\n", stJpeIntrBuf.u32IntrAlBufSize, stJpeIntrBuf.u32IntrRefBufSize);
    stJpeIntrBuf.pu8IntrAlVirBuf = NULL;
    stJpeIntrBuf.phyIntrAlPhyBuf = NULL;
    stJpeIntrBuf.phyIntrRefPhyBuf = NULL;
    stJpeIntrBuf.u32IntrAlBufSize = 0;
    stJpeIntrBuf.u32IntrRefBufSize = 0;
    MHAL_JPE_SetParam(gMhalJpeInst0, E_MHAL_VENC_IDX_STREAM_ON, &stJpeIntrBuf);


#ifdef ENCODE_MULTI_STREAM
    if (MHAL_JPE_CreateInstance(gMhalJpeDev0, &gMhalJpeInst1))
    {
        CamOsPrintf("[KUT] MHAL_JPE_CreateInstance Fail\n");
        return 0;
    }


    /* Set JPE parameter */
    memset(&ResCtl, 0, sizeof(ResCtl));
    setHeader(&ResCtl, sizeof(ResCtl));
    ResCtl.u32Width = YUV_WIDTH;
    ResCtl.u32Height = YUV_HEIGHT;
    ResCtl.eFmt = E_MHAL_VENC_FMT_NV12;
    MHAL_JPE_SetParam(gMhalJpeInst1, E_MHAL_VENC_JPEG_RESOLUTION, &ResCtl);


    memset(&ResCtl, 0, sizeof(ResCtl));
    setHeader(&ResCtl, sizeof(ResCtl));
    MHAL_JPE_GetParam(gMhalJpeInst1, E_MHAL_VENC_JPEG_RESOLUTION, &ResCtl);
    CamOsPrintf("[KUT] resolution %dx%d\n", ResCtl.u32Width, ResCtl.u32Height);


    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    MHAL_JPE_GetParam(gMhalJpeInst1, E_MHAL_VENC_JPEG_RC, &RcCtl);
    RcCtl.stAttrMJPGRc.u32Qfactor = 70;
    MHAL_JPE_SetParam(gMhalJpeInst1, E_MHAL_VENC_JPEG_RC, &RcCtl);


    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    MHAL_JPE_GetParam(gMhalJpeInst1, E_MHAL_VENC_JPEG_RC, &RcCtl);
    CamOsPrintf("[KUT] RC %d\n", RcCtl.stAttrMJPGRc.u32Qfactor);

    if (partial_out)
    {
        memset(&PartialNotify, 0, sizeof(PartialNotify));
        setHeader(&PartialNotify, sizeof(PartialNotify));
        PartialNotify.u32NotifySize = partial_out;
        PartialNotify.notifyFunc = JpeNotifyCallbackFunc;
        MHAL_JPE_SetParam(gMhalJpeInst1, E_MHAL_VENC_PARTIAL_NOTIFY, &PartialNotify);


        memset(&PartialNotify, 0, sizeof(PartialNotify));
        setHeader(&PartialNotify, sizeof(PartialNotify));
        MHAL_JPE_GetParam(gMhalJpeInst1, E_MHAL_VENC_PARTIAL_NOTIFY, &PartialNotify);
        CamOsPrintf("[KUT] partial notify size %u\n", PartialNotify.u32NotifySize);
    }


    memset(&stJpeIntrBuf, 0, sizeof(MHAL_VENC_InternalBuf_t));
    setHeader(&stJpeIntrBuf, sizeof(stJpeIntrBuf));
    MHAL_JPE_QueryBufSize(gMhalJpeInst1, &stJpeIntrBuf);
    CamOsPrintf("[KUT] internal buffer size %d %d\n", stJpeIntrBuf.u32IntrAlBufSize, stJpeIntrBuf.u32IntrRefBufSize);
    stJpeIntrBuf.pu8IntrAlVirBuf = NULL;
    stJpeIntrBuf.phyIntrAlPhyBuf = NULL;
    stJpeIntrBuf.phyIntrRefPhyBuf = NULL;
    stJpeIntrBuf.u32IntrAlBufSize = 0;
    stJpeIntrBuf.u32IntrRefBufSize = 0;
    MHAL_JPE_SetParam(gMhalJpeInst1, E_MHAL_VENC_IDX_STREAM_ON, &stJpeIntrBuf);
#endif


    CamOsDirectMemAlloc("JPEIBUFF", YUV_FRAME_SIZE, &pYuvBufVitr, &pYuvBufPhys, &pYuvBufMiu);
    CamOsDirectMemAlloc("JPEOBUFF0", YUV_FRAME_SIZE, &pInst0BsBufVitr, &pInst0BsBufPhys, &pInst0BsBufMiu);
    Inst0InOutBuf.pu32RegBase0 = NULL;
    Inst0InOutBuf.pu32RegBase1 = NULL;
    Inst0InOutBuf.pCmdQ = NULL;
    Inst0InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(u32)pYuvBufMiu;
    Inst0InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
    Inst0InOutBuf.phyInputYUVBuf2 = (MS_PHYADDR)(u32)pYuvBufMiu + YUV_WIDTH * YUV_HEIGHT;
    Inst0InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;
    Inst0InOutBuf.phyOutputBuf = (MS_PHYADDR)(u32)pInst0BsBufMiu;
    Inst0InOutBuf.virtOutputBuf = (MS_PHYADDR)(u32)pInst0BsBufVitr;
    Inst0InOutBuf.u32OutputBufSize = YUV_FRAME_SIZE;
    Inst0InOutBuf.pFlushCacheCb = NULL;


#ifdef ENCODE_MULTI_STREAM
    CamOsDirectMemAlloc("JPEOBUFF1", YUV_FRAME_SIZE, &pInst1BsBufVitr, &pInst1BsBufPhys, &pInst1BsBufMiu);
    Inst1InOutBuf.pu32RegBase0 = NULL;
    Inst1InOutBuf.pu32RegBase1 = NULL;
    Inst1InOutBuf.pCmdQ = NULL;
    Inst1InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(u32)pYuvBufMiu;
    Inst1InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
    Inst1InOutBuf.phyInputYUVBuf2 = (MS_PHYADDR)(u32)pYuvBufMiu + YUV_WIDTH * YUV_HEIGHT;
    Inst1InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;
    Inst1InOutBuf.phyOutputBuf = (MS_PHYADDR)(u32)pInst1BsBufMiu;
    Inst1InOutBuf.virtOutputBuf = (MS_PHYADDR)(u32)pInst1BsBufVitr;
    Inst1InOutBuf.u32OutputBufSize = YUV_FRAME_SIZE;
    Inst1InOutBuf.pFlushCacheCb = NULL;
#endif


    for (frame_cnt=0; frame_cnt < (sizeof(gYUV)/YUV_FRAME_SIZE); frame_cnt++)
    {
        memcpy(pYuvBufVitr, gYUV+frame_cnt*YUV_FRAME_SIZE, YUV_FRAME_SIZE);

#if (NON_BLOCKING_MODE == 0)
        MHAL_JPE_EncodeOneFrame(gMhalJpeInst0, &Inst0InOutBuf);
    #ifdef ENCODE_MULTI_STREAM
        //CamOsMsSleep(1000);
        MHAL_JPE_EncodeOneFrame(gMhalJpeInst1, &Inst1InOutBuf);
    #endif

        CamOsTsemDown(&stDev0FrameDone);    // wait encode frame done
    #ifdef ENCODE_MULTI_STREAM
        CamOsTsemDown(&stDev0FrameDone);    // wait encode frame done
    #endif
        //CamOsMsSleep(1000);
        MHAL_JPE_EncodeFrameDone(gMhalJpeInst0, &Inst0EncRet);
    #ifdef ENCODE_MULTI_STREAM
        MHAL_JPE_EncodeFrameDone(gMhalJpeInst1, &Inst1EncRet);
    #endif
#else
        MHAL_JPE_EncodeOneFrame(gMhalJpeInst0, &Inst0InOutBuf);
        CamOsTsemDown(&stDev0FrameDone);    // wait encode frame done
        MHAL_JPE_EncodeFrameDone(gMhalJpeInst0, &Inst0EncRet);

    #ifdef ENCODE_MULTI_STREAM
        MHAL_JPE_EncodeOneFrame(gMhalJpeInst1, &Inst1InOutBuf);
        CamOsTsemDown(&stDev0FrameDone);    // wait encode frame done
        MHAL_JPE_EncodeFrameDone(gMhalJpeInst1, &Inst1EncRet);
    #endif
#endif

        CamOsPrintf("frame:%d, size:%d \n", frame_cnt, Inst0EncRet.u32OutputBufUsed);
        //print_hex("bitstream", pInst0BsBufVitr, 128);
        /*if(Inst0EncRet.u32OutputBufUsed && _CompareArrays((u8 *)(gH264_qp45 + u32Inst0TotalLen), (u8 *)pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed) == 0)
            CamOsPrintf("Inst0 Encode Result Compare OK!!!\n");
        else
            CamOsPrintf("Inst0 Encode Result Compare Fail!!!\n");*/

        CamOsSnprintf(szFileName, sizeof(szFileName), "/tmp/kernel_ut_ch%d_frm%d.jpg", 0, frame_cnt);
        fd = OpenFile(szFileName, O_RDONLY | O_CREAT | O_TRUNC, 0666);
        if(fd >= 0)
        {
            WriteFile(fd, pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed);
            CloseFile(fd);
        }

        u32Inst0TotalLen += Inst0EncRet.u32OutputBufUsed;
#ifdef ENCODE_MULTI_STREAM
        CamOsPrintf("frame:%d, size:%d \n", frame_cnt, Inst1EncRet.u32OutputBufUsed);
        //print_hex("bitstream", pInst1BsBufVitr, Inst1EncRet.u32OutputBufUsed);
        /*if(Inst1EncRet.u32OutputBufUsed && _CompareArrays((u8 *)(gH264_qp25 + u32Inst1TotalLen), (u8 *)pInst1BsBufVitr, Inst1EncRet.u32OutputBufUsed) == 0)
            CamOsPrintf("Inst1 Encode Result Compare OK!!!\n");
        else
            CamOsPrintf("Inst1 Encode Result Compare Fail!!!\n");*/

        CamOsSnprintf(szFileName, sizeof(szFileName), "/tmp/kernel_ut_ch%d_frm%d.jpg", 1, frame_cnt);
        fd = OpenFile(szFileName, O_RDONLY | O_CREAT | O_TRUNC, 0666);
        if(fd >= 0)
        {
            WriteFile(fd, pInst1BsBufVitr, Inst1EncRet.u32OutputBufUsed);
            CloseFile(fd);
        }

        u32Inst1TotalLen += Inst1EncRet.u32OutputBufUsed;
#endif
    }


    if (MHAL_JPE_DestroyInstance(gMhalJpeInst0))
    {
        CamOsPrintf("[KUT] MHAL_JPE_DestroyInstance Fail\n");
        return 0;
    }
#ifdef ENCODE_MULTI_STREAM
    if (MHAL_JPE_DestroyInstance(gMhalJpeInst1))
    {
        CamOsPrintf("[KUT] MHAL_JPE_DestroyInstance Fail\n");
        return 0;
    }
#endif

    free_irq(JpeIrq, gMhalJpeDev0);

    if (MHAL_JPE_DestroyDevice(gMhalJpeDev0))
    {
        CamOsPrintf("[KUT] MHAL_JPE_DestroyDevice Fail\n");
        return 0;
    }

    CamOsDirectMemRelease(pYuvBufVitr, YUV_FRAME_SIZE);
    CamOsDirectMemRelease(pInst0BsBufVitr, YUV_FRAME_SIZE);
#ifdef ENCODE_MULTI_STREAM
    CamOsDirectMemRelease(pInst1BsBufVitr, YUV_FRAME_SIZE);
#endif

    return 0;
}

static int
JpeRemove(
//    struct platform_device* pdev)
    void)
{
    return 0;
}

/*static const struct of_device_id JpeMatchTables[] = {
	{ .compatible = "mstar,jpe" },
	{},
};

static struct platform_driver JpePdrv = {
    .probe = JpeProbe,
    .remove = JpeRemove,
    .driver = {
        .name = "mjpe",
        .owner = THIS_MODULE,
		.of_match_table = of_match_ptr(JpeMatchTables),
    }
};*/

static int  __init mjpe_init(void)
{
    //return platform_driver_register(&JpePdrv);
    JpeProbe();
    return 0;
}

static void __exit mjpe_exit(void)
{
    //platform_driver_unregister(&JpePdrv);
    JpeRemove();
}

module_init(mjpe_init);
module_exit(mjpe_exit);
