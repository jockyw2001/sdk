
//#include <linux/platform_device.h>
//#include <linux/of.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include "mhal_venc.h"
#include "mhal_mhe.h"
#include "cam_os_wrapper.h"
#include "NV12_cif_frames.h"
#include "NV12_cif_frames_qp32_golden.h"
#include "NV12_cif_frames_qp50_golden.h"
#ifdef SUPPORT_CMDQ_SERVICE
#include "mhal_cmdq.h"
#endif
#include "file_access.h"
#include "md5.h"

#include <ms_platform.h>

MODULE_LICENSE("GPL");

#define YUV_WIDTH   1920
#define YUV_HEIGHT  1088
#define YUV_FRAME_SIZE  (YUV_WIDTH*YUV_HEIGHT*3/2)


#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

#define FRAME_NUM 5
#define OUTBUFSIZE YUV_FRAME_SIZE

#define QP 32

#define MD5_CHECK  1

static unsigned int thread0_exit = 0;
static unsigned int thread1_exit = 0;

CamOsTsem_t stDev0FrameDone;
CamOsTsem_t stDev1FrameDone;

#ifdef SUPPORT_CMDQ_SERVICE
MHAL_CMDQ_CmdqInterface_t *stCmdQInf;
MHAL_CMDQ_CmdqInterface_t *stCmdQInf_1;
#endif

void* gMhalMheDev0 = NULL;
void* gMhalMheDev1 = NULL;
void* gMhalMheInst0 = NULL;
void* gMhalMheInst1 = NULL;

static unsigned char Md5Golden0[16] = {0xf4,0x8e,0x52,0x97,0xb1,0x36,0xa0,0xc6,0xcd,0xe9,0x89,0xc7,0xdd,0x69,0x0d,0x14};
static unsigned char Md5Golden1[16] = {0xff,0x00,0x39,0xc1,0xb5,0x9d,0x0c,0x86,0x85,0x11,0xbc,0x18,0x47,0xd5,0x68,0xf4};

static unsigned int fixedgolden = 1;
module_param(fixedgolden, uint, S_IRUGO | S_IWUSR);

static unsigned int clock = 5;  //sinle core multi-instance, dual core single instance
module_param(clock, uint, S_IRUGO | S_IWUSR);

static unsigned int test_mode = 2;  //sinle core multi-instance, dual core single instance
module_param(test_mode, uint, S_IRUGO | S_IWUSR);

static unsigned int rc_mode = 0;  //0: FixedQP, 1:CBR, 2:VBR
module_param(rc_mode, uint, S_IRUGO | S_IWUSR);

static unsigned int burnin_mode = 1;
module_param(burnin_mode, uint, S_IRUGO | S_IWUSR);

static unsigned int core_id = 0;
module_param(core_id, uint, S_IRUGO | S_IWUSR);

static unsigned int loop_run = 3;
module_param(loop_run, uint, S_IRUGO | S_IWUSR);

static unsigned int cmdq_en = 0;
module_param(cmdq_en, uint, S_IRUGO | S_IWUSR);

static unsigned int multi_stream = 0;
module_param(multi_stream, uint, S_IRUGO | S_IWUSR);

static unsigned int encode_framed = 300;//sizeof(gYUV)/YUV_FRAME_SIZE;
module_param(encode_framed, uint, S_IRUGO | S_IWUSR);

static unsigned int data_compare = 0;
module_param(data_compare, uint, S_IRUGO | S_IWUSR);

static unsigned int disp_fps = 0;
module_param(disp_fps, uint, S_IRUGO | S_IWUSR);

static unsigned int save_file = 0;
module_param(save_file, uint, S_IRUGO | S_IWUSR);

static unsigned int debug_yuvfile = 0;
module_param(debug_yuvfile, uint, S_IRUGO | S_IWUSR);

#define TIMEVAL_US_DIFF(start, end)     ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec))

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
    for(i = 0; i < num; i++) {
        if(i % 16 == 0)
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
        if(a[i] != b[i])
            return -1;
    }
    return 0;
}

static irqreturn_t _MheUtIsr(int irq, void* priv)
{
    //CamOsPrintf("> _MheUtIsr\n");
    MHAL_MHE_IsrProc(priv);

    CamOsTsemUp(&stDev0FrameDone);

    return IRQ_HANDLED;
}

static irqreturn_t _MheUtIsr_1(int irq, void* priv)
{
    //CamOsPrintf("> _MheUtIsr\n");
    MHAL_MHE_IsrProc(priv);

    CamOsTsemUp(&stDev1FrameDone);

    return IRQ_HANDLED;
}


static int _CamOsThreadTestEntry0(void *pUserdata)
{

    MHAL_VENC_ParamInt_t param;
    MHAL_VENC_Resoluton_t ResCtl;
    MHAL_VENC_RcInfo_t RcCtl;
    MHAL_VENC_InternalBuf_t stMheIntrBuf;
    MHAL_VENC_InOutBuf_t Inst0InOutBuf;
    MHAL_VENC_EncResult_t Inst0EncRet;
    void* pYuvBufVitr = NULL;
    void* pYuvBufPhys = NULL;
    void* pYuvBufMiu = NULL;
    void* pInst0BsBufVitr = NULL;
    void* pInst0BsBufPhys = NULL;
    void* pInst0BsBufMiu = NULL;
    void *pInst0MheInternalBuf1Virt = NULL;
    void *pInst0MheInternalBuf1Phys = NULL;
    void *pInst0MheInternalBuf1Miu = NULL;
    void *pInst0MheInternalBuf2Virt = NULL;
    void *pInst0MheInternalBuf2Phys = NULL;
    void *pInst0MheInternalBuf2Miu = NULL;
    u32 u32Inst0TotalLen = 0;
    u32 u32Inst1TotalLen = 0;
    MHAL_VENC_InOutBuf_t Inst1InOutBuf;
    MHAL_VENC_EncResult_t Inst1EncRet;
    void* pInst1BsBufVitr = NULL;
    void* pInst1BsBufPhys = NULL;
    void* pInst1BsBufMiu = NULL;
    void *pInst1MheInternalBuf1Virt = NULL;
    void *pInst1MheInternalBuf1Phys = NULL;
    void *pInst1MheInternalBuf1Miu = NULL;
    void *pInst1MheInternalBuf2Virt = NULL;
    void *pInst1MheInternalBuf2Phys = NULL;
    void *pInst1MheInternalBuf2Miu = NULL;
    int frame_cnt;
    int run_frame = sizeof(gYUV) / YUV_FRAME_SIZE;
    int MheIrq = 0;
#ifdef SUPPORT_CMDQ_SERVICE
    MHAL_CMDQ_BufDescript_t stCmdqBufDesp;
#endif
    int rec_frame_cnt;
    int total_frame = 0;
    struct timeval stTvStart, stTvEnd;
    struct file *fp;
    char szFn[64];
    int i;
    int md5_compare;
    int loop_run_cnt = loop_run;
    struct file *outfp;
    void* pYuvBufVitrBak = NULL;
    void* pYuvBufPhysBak = NULL;
    void* pYuvBufMiuBak = NULL;
    u16 nRegVal;
    CamOsRet_e eRet;
    //CamOsTimer_t tTimer0;
    //unsigned long nTimerMs;

#if MD5_CHECK==1
    MD5_CTX stMd5 = {0};
    unsigned char u8Md5Result[16] = {0};
    unsigned char u8Md5Golden[16] = {0};

    MD5_CTX stMd5YUV = {0};
    unsigned char u8Md5ResultYUV[16] = {0};
    unsigned char u8Md5GoldenYUV[16] = {0};
#endif

    CamOsPrintf("> %s \n", __FUNCTION__);

    if(fixedgolden == 1)
    {
        memcpy(u8Md5Golden,Md5Golden0,16);
        printk(KERN_WARNING"MHE0 Golden MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
               u8Md5Golden[0], u8Md5Golden[1], u8Md5Golden[2], u8Md5Golden[3],
               u8Md5Golden[4], u8Md5Golden[5], u8Md5Golden[6], u8Md5Golden[7],
               u8Md5Golden[8], u8Md5Golden[9], u8Md5Golden[10], u8Md5Golden[11],
               u8Md5Golden[12], u8Md5Golden[13], u8Md5Golden[14], u8Md5Golden[15]);
    }


    //if(encode_framed < sizeof(gYUV)/YUV_FRAME_SIZE)
    run_frame = encode_framed;

    rec_frame_cnt = run_frame;

    CamOsPrintf("[%s] encode_framed :  %d\n", __FUNCTION__, run_frame);

    CamOsTsemInit(&stDev0FrameDone, 0);

    if(MHAL_MHE_CreateDevice(0, &gMhalMheDev0))
    {
        CamOsPrintf("[KUT] MHAL_MHE_CreateDevice core 0 Fail\n");
        return 0;
    }

    nRegVal = *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2)));
    if(clock == 5)
        nRegVal = (nRegVal & 0x00FF) | 0x1800;      // 576MHz
    else if(clock == 4)
        nRegVal = (nRegVal & 0x00FF) | 0x0400;      // 480MHz
    else if(clock == 3)
        nRegVal = (nRegVal & 0x00FF) | 0x0C00;      // 384MHz
    else if(clock == 2)
        nRegVal = (nRegVal & 0x00FF) | 0x1000;      // 288MHz
    else if(clock == 1)
        nRegVal = (nRegVal & 0x00FF) | 0x1400;      // 192MHz
    else if(clock == 0)
        nRegVal = (nRegVal & 0x00FF) | 0x0800;      // 432MHz
    *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2))) = nRegVal;

    setHeader(&param, sizeof(param));
    if(MHAL_MHE_GetDevConfig(gMhalMheDev0, E_MHAL_VENC_HW_IRQ_NUM, &param))
    {
        CamOsPrintf("[KUT] MHAL_MHE_GetDevConfig Fail\n");
        return 0;
    }

    MheIrq = param.u32Val;
    CamOsPrintf("[KUT] MHE core 0 IRQ: %d\n", MheIrq);
    if(0 != request_irq(MheIrq, _MheUtIsr, IRQF_SHARED, "_MheUtIsr", gMhalMheDev0))
    {
        CamOsPrintf("[KUT] request_irq core 0 (%d) Fail\n", param.u32Val);
        return 0;
    }

    setHeader(&param, sizeof(param));
    if(MHAL_MHE_GetDevConfig(gMhalMheDev0, E_MHAL_VENC_HW_CMDQ_BUF_LEN, &param))
    {
        CamOsPrintf("[KUT] MHAL_MHE_GetDevConfig Fail\n");
        return 0;
    }

#ifdef SUPPORT_CMDQ_SERVICE
    if(cmdq_en)
    {
        stCmdqBufDesp.u32CmdqBufSize = param.u32Val;
        CamOsPrintf("0 stCmdqBufDesp.u32CmdqBufSize = %d\n", stCmdqBufDesp.u32CmdqBufSize);
        stCmdqBufDesp.u32CmdqBufSizeAlign = 16;
        stCmdqBufDesp.u32MloadBufSize = 0;
        stCmdqBufDesp.u16MloadBufSizeAlign = 16;
        stCmdQInf = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC0, &stCmdqBufDesp, FALSE);
        CamOsPrintf("Call MHAL_CMDQ_GetSysCmdqService: 0x%08X 0x%08X\n", stCmdQInf, stCmdQInf->MHAL_CMDQ_CheckBufAvailable);
    }
#endif

    if(MHAL_MHE_CreateInstance(gMhalMheDev0, &gMhalMheInst0))
    {
        CamOsPrintf("[KUT] MHAL_MHE_CreateInstance Fail\n");
        return 0;
    }

    /* Set MHE parameter */
    memset(&ResCtl, 0, sizeof(ResCtl));
    setHeader(&ResCtl, sizeof(ResCtl));
    ResCtl.u32Width = YUV_WIDTH;
    ResCtl.u32Height = YUV_HEIGHT;
    ResCtl.eFmt = E_MHAL_VENC_FMT_NV12;
    MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_265_RESOLUTION, &ResCtl);

    memset(&ResCtl, 0, sizeof(ResCtl));
    setHeader(&ResCtl, sizeof(ResCtl));
    MHAL_MHE_GetParam(gMhalMheInst0, E_MHAL_VENC_265_RESOLUTION, &ResCtl);
    CamOsPrintf("[KUT] resolution %dx%d\n", ResCtl.u32Width, ResCtl.u32Height);

    if(rc_mode == 1)
    {
        memset(&RcCtl, 0, sizeof(RcCtl));
        setHeader(&RcCtl, sizeof(RcCtl));
        RcCtl.eRcMode = E_MHAL_VENC_RC_MODE_H265CBR;
        RcCtl.stAttrH265Cbr.u32SrcFrmRateNum = 30;
        RcCtl.stAttrH265Cbr.u32SrcFrmRateDen = 1;
        RcCtl.stAttrH265Cbr.u32Gop = 29;
        RcCtl.stAttrH265Cbr.u32BitRate = 500000;
        MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_265_RC, &RcCtl);

        memset(&RcCtl, 0, sizeof(RcCtl));
        setHeader(&RcCtl, sizeof(RcCtl));
        MHAL_MHE_GetParam(gMhalMheInst0, E_MHAL_VENC_265_RC, &RcCtl);
        CamOsPrintf("[KUT] RC %d %d/%d %d\n", RcCtl.stAttrH265Cbr.u32Gop, RcCtl.stAttrH265Cbr.u32SrcFrmRateNum, RcCtl.stAttrH265Cbr.u32SrcFrmRateDen, RcCtl.stAttrH265Cbr.u32BitRate);
    }
    else
    {
        memset(&RcCtl, 0, sizeof(RcCtl));
        setHeader(&RcCtl, sizeof(RcCtl));
        RcCtl.eRcMode = E_MHAL_VENC_RC_MODE_H265FIXQP;
        RcCtl.stAttrH265FixQp.u32SrcFrmRateNum = 30;
        RcCtl.stAttrH265FixQp.u32SrcFrmRateDen = 1;
        RcCtl.stAttrH265FixQp.u32Gop = 29;
        RcCtl.stAttrH265FixQp.u32IQp = QP;
        RcCtl.stAttrH265FixQp.u32PQp = QP;
        MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_265_RC, &RcCtl);

        memset(&RcCtl, 0, sizeof(RcCtl));
        setHeader(&RcCtl, sizeof(RcCtl));
        MHAL_MHE_GetParam(gMhalMheInst0, E_MHAL_VENC_265_RC, &RcCtl);
        CamOsPrintf("[KUT] RC %d %d %d\n", RcCtl.stAttrH265FixQp.u32Gop, RcCtl.stAttrH265FixQp.u32IQp, RcCtl.stAttrH265FixQp.u32PQp);
    }

    memset(&stMheIntrBuf, 0, sizeof(MHAL_VENC_InternalBuf_t));
    setHeader(&stMheIntrBuf, sizeof(stMheIntrBuf));
    MHAL_MHE_QueryBufSize(gMhalMheInst0, &stMheIntrBuf);
    CamOsPrintf("[KUT] internal buffer size %d %d\n", stMheIntrBuf.u32IntrAlBufSize, stMheIntrBuf.u32IntrRefBufSize);

    /* Allocate MHE internal buffer and assign to driver */
    CamOsDirectMemAlloc("MHE0ALBUF", stMheIntrBuf.u32IntrAlBufSize, &pInst0MheInternalBuf1Virt, &pInst0MheInternalBuf1Phys, &pInst0MheInternalBuf1Miu);
    CamOsDirectMemAlloc("MHE0REFBUF", stMheIntrBuf.u32IntrRefBufSize, &pInst0MheInternalBuf2Virt, &pInst0MheInternalBuf2Phys, &pInst0MheInternalBuf2Miu);


    if(multi_stream && cmdq_en)
    {
        if(MHAL_MHE_CreateInstance(gMhalMheDev0, &gMhalMheInst1))
        {
            CamOsPrintf("[KUT] MHAL_MHE_CreateInstance Fail\n");
            return 0;
        }

        /* Set MHE parameter */
        memset(&ResCtl, 0, sizeof(ResCtl));
        setHeader(&ResCtl, sizeof(ResCtl));
        ResCtl.u32Width = YUV_WIDTH;
        ResCtl.u32Height = YUV_HEIGHT;
        ResCtl.eFmt = E_MHAL_VENC_FMT_NV12;
        MHAL_MHE_SetParam(gMhalMheInst1, E_MHAL_VENC_265_RESOLUTION, &ResCtl);

        memset(&ResCtl, 0, sizeof(ResCtl));
        setHeader(&ResCtl, sizeof(ResCtl));
        MHAL_MHE_GetParam(gMhalMheInst1, E_MHAL_VENC_265_RESOLUTION, &ResCtl);
        CamOsPrintf("[KUT] resolution %dx%d\n", ResCtl.u32Width, ResCtl.u32Height);

        memset(&RcCtl, 0, sizeof(RcCtl));
        setHeader(&RcCtl, sizeof(RcCtl));
        RcCtl.eRcMode = E_MHAL_VENC_RC_MODE_H265FIXQP;
        RcCtl.stAttrH265FixQp.u32SrcFrmRateNum = 30;
        RcCtl.stAttrH265FixQp.u32SrcFrmRateDen = 1;
        RcCtl.stAttrH265FixQp.u32Gop = 29;
        RcCtl.stAttrH265FixQp.u32IQp = 50;
        RcCtl.stAttrH265FixQp.u32PQp = 50;
        MHAL_MHE_SetParam(gMhalMheInst1, E_MHAL_VENC_265_RC, &RcCtl);

        memset(&RcCtl, 0, sizeof(RcCtl));
        setHeader(&RcCtl, sizeof(RcCtl));
        MHAL_MHE_GetParam(gMhalMheInst1, E_MHAL_VENC_265_RC, &RcCtl);
        CamOsPrintf("[KUT] RC %d %d %d\n", RcCtl.stAttrH265FixQp.u32Gop, RcCtl.stAttrH265FixQp.u32IQp, RcCtl.stAttrH265FixQp.u32PQp);

        memset(&stMheIntrBuf, 0, sizeof(MHAL_VENC_InternalBuf_t));
        setHeader(&stMheIntrBuf, sizeof(stMheIntrBuf));
        MHAL_MHE_QueryBufSize(gMhalMheInst1, &stMheIntrBuf);
        CamOsPrintf("[KUT] internal buffer size %d %d\n", stMheIntrBuf.u32IntrAlBufSize, stMheIntrBuf.u32IntrRefBufSize);


        /* Allocate MHE internal buffer and assign to driver */
        CamOsDirectMemAlloc("MHEINTRALBUF1", stMheIntrBuf.u32IntrAlBufSize, &pInst1MheInternalBuf1Virt, &pInst1MheInternalBuf1Phys, &pInst1MheInternalBuf1Miu);
        CamOsDirectMemAlloc("MHEINTRREFBUF1", stMheIntrBuf.u32IntrRefBufSize, &pInst1MheInternalBuf2Virt, &pInst1MheInternalBuf2Phys, &pInst1MheInternalBuf2Miu);
        stMheIntrBuf.pu8IntrAlVirBuf = (MS_U8 *)pInst1MheInternalBuf1Virt;
        stMheIntrBuf.phyIntrAlPhyBuf = (MS_PHYADDR)(u32)pInst1MheInternalBuf1Miu;
        stMheIntrBuf.phyIntrRefPhyBuf = (MS_PHYADDR)(u32)pInst1MheInternalBuf2Miu;
        MHAL_MHE_SetParam(gMhalMheInst1, E_MHAL_VENC_IDX_STREAM_ON, &stMheIntrBuf);
    }

    CamOsDirectMemAlloc("MHE0IBUFF", YUV_FRAME_SIZE * FRAME_NUM, &pYuvBufVitr, &pYuvBufPhys, &pYuvBufMiu);
    CamOsDirectMemAlloc("MHE0OBUFF", YUV_FRAME_SIZE, &pInst0BsBufVitr, &pInst0BsBufPhys, &pInst0BsBufMiu);
    Inst0InOutBuf.pu32RegBase0 = NULL;
    Inst0InOutBuf.pu32RegBase1 = NULL;

    CamOsDirectMemAlloc("MHE0Backup", YUV_FRAME_SIZE * FRAME_NUM, &pYuvBufVitrBak, &pYuvBufPhysBak, &pYuvBufMiuBak);

#ifdef SUPPORT_CMDQ_SERVICE
    if(cmdq_en)
    {
        Inst0InOutBuf.pCmdQ = stCmdQInf;
    }
    else
    {
        Inst0InOutBuf.pCmdQ = NULL;
    }
#else
    Inst0InOutBuf.pCmdQ = NULL;
#endif

    Inst0InOutBuf.bRequestI = 0;
    Inst0InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(u32)pYuvBufMiu;
    Inst0InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
    Inst0InOutBuf.phyInputYUVBuf2 = Inst0InOutBuf.phyInputYUVBuf1 + Inst0InOutBuf.u32InputYUVBuf1Size;
    Inst0InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;
    Inst0InOutBuf.phyOutputBuf = (MS_PHYADDR)(u32)pInst0BsBufMiu;
    Inst0InOutBuf.virtOutputBuf = (MS_PHYADDR)(u32)pInst0BsBufVitr;
    Inst0InOutBuf.u32OutputBufSize = OUTBUFSIZE;//YUV_FRAME_SIZE;
    Inst0InOutBuf.pFlushCacheCb = NULL;

    sprintf(szFn, "/vendor/%dNV", YUV_WIDTH);
    fp = OpenFile(szFn, O_RDONLY, 0);
    if(fp)
        CamOsPrintf("Open %s success\n", szFn);
    else
        CamOsPrintf("Open %s failed\n", szFn);
    ReadFile(fp, pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
    CloseFile(fp);

    Chip_Flush_Cache_Range((unsigned long)pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
    Chip_Flush_Memory();

    if(debug_yuvfile)
    {
        MD5_Init(&stMd5YUV);
        MD5_Update(&stMd5YUV, pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
        MD5_Final(u8Md5GoldenYUV, &stMd5YUV);
        CamOsPrintf("MHE0 YUV MD5 Golden: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                u8Md5GoldenYUV[0], u8Md5GoldenYUV[1], u8Md5GoldenYUV[2], u8Md5GoldenYUV[3],
                u8Md5GoldenYUV[4], u8Md5GoldenYUV[5], u8Md5GoldenYUV[6], u8Md5GoldenYUV[7],
                u8Md5GoldenYUV[8], u8Md5GoldenYUV[9], u8Md5GoldenYUV[10], u8Md5GoldenYUV[11],
                u8Md5GoldenYUV[12], u8Md5GoldenYUV[13], u8Md5GoldenYUV[14], u8Md5GoldenYUV[15]);
    }

    memcpy(pYuvBufVitrBak,pYuvBufVitr,YUV_FRAME_SIZE * FRAME_NUM);

    if(multi_stream && cmdq_en)
    {
        CamOsDirectMemAlloc("MHEOBUFF1", YUV_FRAME_SIZE, &pInst1BsBufVitr, &pInst1BsBufPhys, &pInst1BsBufMiu);
        Inst1InOutBuf.pu32RegBase0 = NULL;
        Inst1InOutBuf.pu32RegBase1 = NULL;
#ifdef SUPPORT_CMDQ_SERVICE
        if(cmdq_en)
        {
            Inst1InOutBuf.pCmdQ = stCmdQInf;
        }
        else
        {
            Inst1InOutBuf.pCmdQ = NULL;
        }
#else
        Inst1InOutBuf.pCmdQ = NULL;
#endif
        Inst1InOutBuf.bRequestI = 0;
        Inst1InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(u32)pYuvBufMiu;
        Inst1InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
        Inst1InOutBuf.phyInputYUVBuf2 = Inst1InOutBuf.phyInputYUVBuf1 + Inst1InOutBuf.u32InputYUVBuf1Size;
        Inst1InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;
        Inst1InOutBuf.phyOutputBuf = (MS_PHYADDR)(u32)pInst1BsBufMiu;
        Inst1InOutBuf.virtOutputBuf = (MS_PHYADDR)(u32)pInst1BsBufVitr;
        Inst1InOutBuf.u32OutputBufSize = YUV_FRAME_SIZE;
        Inst1InOutBuf.pFlushCacheCb = NULL;
    }

    while(1)
    {
        //initial value
        total_frame = 0;
        md5_compare = 1;
#if MD5_CHECK==1
        MD5_Init(&stMd5);
#endif

        memset(pInst0MheInternalBuf1Virt, 0, stMheIntrBuf.u32IntrAlBufSize);
        memset(pInst0MheInternalBuf2Virt, 0, stMheIntrBuf.u32IntrRefBufSize);

        if(save_file)
        {
            sprintf(szFn, "/tmp/MHE0_%dx%d_%d.h265", YUV_WIDTH, YUV_HEIGHT, loop_run_cnt);
            outfp = OpenFile(szFn, O_RDWR | O_CREAT | O_TRUNC, 0644);
            if(outfp)
                CamOsPrintf("Open %s success\n", szFn);
            else
                CamOsPrintf("Open %s failed\n", szFn);
        }

        /* Allocate MHE internal buffer and assign to driver */
        stMheIntrBuf.pu8IntrAlVirBuf = (MS_U8 *)pInst0MheInternalBuf1Virt;
        stMheIntrBuf.phyIntrAlPhyBuf = (MS_PHYADDR)(u32)pInst0MheInternalBuf1Miu;
        stMheIntrBuf.phyIntrRefPhyBuf = (MS_PHYADDR)(u32)pInst0MheInternalBuf2Miu;
        MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_IDX_STREAM_ON, &stMheIntrBuf);

        do_gettimeofday(&stTvStart);

        for(frame_cnt = 0; frame_cnt < run_frame; frame_cnt++)
        {
            //memcpy(pYuvBufVitr, gYUV + frame_cnt * YUV_FRAME_SIZE, YUV_FRAME_SIZE);
            Inst0InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(u32)pYuvBufMiu + YUV_FRAME_SIZE * (frame_cnt % FRAME_NUM);
            Inst0InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
            Inst0InOutBuf.phyInputYUVBuf2 = Inst0InOutBuf.phyInputYUVBuf1 + Inst0InOutBuf.u32InputYUVBuf1Size;
            Inst0InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;

            MHAL_MHE_EncodeOneFrame(gMhalMheInst0, &Inst0InOutBuf);

            if(multi_stream && cmdq_en)
            {
                MHAL_MHE_EncodeOneFrame(gMhalMheInst0, &Inst1InOutBuf);
            }

#ifdef SUPPORT_CMDQ_SERVICE
            if(cmdq_en)
            {
                stCmdQInf->MHAL_CMDQ_KickOffCmdq(stCmdQInf);
            }
#endif

            //CamOsTsemDown(&stDev0FrameDone);    // wait encode frame done
            eRet = CamOsTsemTimedDown(&stDev0FrameDone, 1000);
            if(eRet != CAM_OS_OK)
            {
                CamOsPrintf("MHE0 encode timeout!! %d\n", frame_cnt);
                break;
            }

            if(multi_stream && cmdq_en)
            {
                CamOsTsemTimedDown(&stDev0FrameDone, 1000);    // wait encode frame done
            }

            MHAL_MHE_EncodeFrameDone(gMhalMheInst0, &Inst0EncRet);
            if(multi_stream && cmdq_en)
            {
                MHAL_MHE_EncodeFrameDone(gMhalMheInst1, &Inst1EncRet);
            }

            //CamOsPrintf("MHE0: frame:%d, size:%d \n", frame_cnt, Inst0EncRet.u32OutputBufUsed);
            //print_hex("MHE0:bitstream", pInst0BsBufVitr, 100);
            if(save_file)
                WriteFile(outfp, pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed);
#if MD5_CHECK==1
            MD5_Update(&stMd5, pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed);
#endif
            total_frame++;
        }

        u32Inst0TotalLen = 0;
        if(multi_stream && cmdq_en)
        {
            u32Inst1TotalLen = 0;
        }

#if MD5_CHECK==1
        MD5_Final(u8Md5Result, &stMd5);
        printk(KERN_WARNING"MHE0 Result MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
               u8Md5Result[0], u8Md5Result[1], u8Md5Result[2], u8Md5Result[3],
               u8Md5Result[4], u8Md5Result[5], u8Md5Result[6], u8Md5Result[7],
               u8Md5Result[8], u8Md5Result[9], u8Md5Result[10], u8Md5Result[11],
               u8Md5Result[12], u8Md5Result[13], u8Md5Result[14], u8Md5Result[15]);

        if(u8Md5Golden[15] == 0 && u8Md5Golden[14] == 0 && u8Md5Golden[13] == 0)
        {
            for(i = 0; i < 16; i++)
                u8Md5Golden[i] = u8Md5Result[i];

            printk(KERN_WARNING"MHE0 Golden MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                   u8Md5Golden[0], u8Md5Golden[1], u8Md5Golden[2], u8Md5Golden[3],
                   u8Md5Golden[4], u8Md5Golden[5], u8Md5Golden[6], u8Md5Golden[7],
                   u8Md5Golden[8], u8Md5Golden[9], u8Md5Golden[10], u8Md5Golden[11],
                   u8Md5Golden[12], u8Md5Golden[13], u8Md5Golden[14], u8Md5Golden[15]);
        }

        for(i = 0; i < 16; i++)
        {
            if(u8Md5Result[i] != u8Md5Golden[i])
            {
                md5_compare = 0;
                CamOsPrintf("MHE0: MD5 compare err !!! Exit burn-in test\n");
                break;
            }
        }
#endif
        do_gettimeofday(&stTvEnd);

        if(disp_fps)
        {
            printk(KERN_WARNING"0 total time: %ldus, total frame: %d, fps %ld\n",
                   TIMEVAL_US_DIFF(stTvStart, stTvEnd),
                   total_frame,
                   (total_frame * 1000000) / TIMEVAL_US_DIFF(stTvStart, stTvEnd));
        }


        MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_IDX_STREAM_OFF, &stMheIntrBuf);

        if(md5_compare)
        {
            printk(KERN_WARNING"MHE0: MD5 compare success !!\n");
        }
        else
        {
            if(debug_yuvfile)
            {
                MD5_Init(&stMd5YUV);
                MD5_Update(&stMd5YUV, pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
                MD5_Final(u8Md5ResultYUV, &stMd5YUV);
                CamOsPrintf("MHE0 YUV MD5 Result: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                        u8Md5ResultYUV[0], u8Md5ResultYUV[1], u8Md5ResultYUV[2], u8Md5ResultYUV[3],
                        u8Md5ResultYUV[4], u8Md5ResultYUV[5], u8Md5ResultYUV[6], u8Md5ResultYUV[7],
                        u8Md5ResultYUV[8], u8Md5ResultYUV[9], u8Md5ResultYUV[10], u8Md5ResultYUV[11],
                        u8Md5ResultYUV[12], u8Md5ResultYUV[13], u8Md5ResultYUV[14], u8Md5ResultYUV[15]);
#if 1
                if(strncmp(u8Md5ResultYUV,u8Md5GoldenYUV,16))
                {

                    while(1)
                    {
                        CamOsPrintf("MHE0: Read input file again!\n");

                        memcpy(pYuvBufVitr, pYuvBufVitrBak, YUV_FRAME_SIZE * FRAME_NUM);

                        MD5_Init(&stMd5YUV);
                        MD5_Update(&stMd5YUV, pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
                        MD5_Final(u8Md5ResultYUV, &stMd5YUV);
                        CamOsPrintf("MHE0 Read YUV MD5 Result: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                                u8Md5ResultYUV[0], u8Md5ResultYUV[1], u8Md5ResultYUV[2], u8Md5ResultYUV[3],
                                u8Md5ResultYUV[4], u8Md5ResultYUV[5], u8Md5ResultYUV[6], u8Md5ResultYUV[7],
                                u8Md5ResultYUV[8], u8Md5ResultYUV[9], u8Md5ResultYUV[10], u8Md5ResultYUV[11],
                                u8Md5ResultYUV[12], u8Md5ResultYUV[13], u8Md5ResultYUV[14], u8Md5ResultYUV[15]);

                        if(!strncmp(u8Md5ResultYUV,u8Md5GoldenYUV,16))
                            break;

                    }

                    loop_run_cnt++; //re-compress again

                }
                else
#endif
                {
                    printk(KERN_WARNING"MHE0: MD5 compare fail !!\n");
                    break;
                }

            }
            else
            {
                printk(KERN_WARNING"MHE0: MD5 compare fail !!\n");
                break;
            }
        }

        if(save_file)
            CloseFile(outfp);

        if(!burnin_mode)
        {
            if(!loop_run_cnt)
                break;
            loop_run_cnt--;
        }
    }

    if(MHAL_MHE_DestroyInstance(gMhalMheInst0))
    {
        CamOsPrintf("[KUT] MHAL_MHE_DestroyInstance Fail\n");
        return 0;
    }
    if(multi_stream && cmdq_en)
    {
        if(MHAL_MHE_DestroyInstance(gMhalMheInst1))
        {
            CamOsPrintf("[KUT] MHAL_MHE_DestroyInstance Fail\n");
            return 0;
        }
    }

    CamOsDirectMemRelease(pYuvBufVitrBak, YUV_FRAME_SIZE * FRAME_NUM);
    CamOsDirectMemRelease(pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
    CamOsDirectMemRelease(pInst0BsBufVitr, YUV_FRAME_SIZE);
    CamOsDirectMemRelease(pInst0MheInternalBuf1Virt, stMheIntrBuf.u32IntrAlBufSize);
    CamOsDirectMemRelease(pInst0MheInternalBuf2Virt, stMheIntrBuf.u32IntrRefBufSize);

    if(multi_stream && cmdq_en)
    {
        CamOsDirectMemRelease(pInst1BsBufVitr, YUV_FRAME_SIZE);
        CamOsDirectMemRelease(pInst1MheInternalBuf1Virt, stMheIntrBuf.u32IntrAlBufSize);
        CamOsDirectMemRelease(pInst1MheInternalBuf2Virt, stMheIntrBuf.u32IntrRefBufSize);
    }


    //release irq
    free_irq(MheIrq, gMhalMheDev0);

    //release device
    if(MHAL_MHE_DestroyDevice(gMhalMheDev0))
    {
        CamOsPrintf("[KUT] MHAL_MHE_DestroyDevice Fail\n");
        return 0;
    }

    //release comdq server
#ifdef SUPPORT_CMDQ_SERVICE
    if(cmdq_en)
    {
        CamOsPrintf("Call MHAL_CMDQ_ReleaseSysCmdqService\n");
        MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC0);
    }
#endif

    return 0;
}

static int _CamOsThreadTestEntry1(void *pUserdata)
{
    MHAL_VENC_ParamInt_t param;
    MHAL_VENC_Resoluton_t ResCtl;
    MHAL_VENC_RcInfo_t RcCtl;
    MHAL_VENC_InternalBuf_t stMheIntrBuf;
    MHAL_VENC_InOutBuf_t Inst0InOutBuf;
    MHAL_VENC_EncResult_t Inst0EncRet;
    void* pYuvBufVitr = NULL;
    void* pYuvBufPhys = NULL;
    void* pYuvBufMiu = NULL;
    void* pInst0BsBufVitr = NULL;
    void* pInst0BsBufPhys = NULL;
    void* pInst0BsBufMiu = NULL;
    void *pInst0MheInternalBuf1Virt = NULL;
    void *pInst0MheInternalBuf1Phys = NULL;
    void *pInst0MheInternalBuf1Miu = NULL;
    void *pInst0MheInternalBuf2Virt = NULL;
    void *pInst0MheInternalBuf2Phys = NULL;
    void *pInst0MheInternalBuf2Miu = NULL;
    u32 u32Inst0TotalLen = 0;
    u32 u32Inst1TotalLen = 0;
    MHAL_VENC_InOutBuf_t Inst1InOutBuf;
    //MHAL_VENC_EncResult_t Inst1EncRet;
    void* pInst1BsBufVitr = NULL;
    void* pInst1BsBufPhys = NULL;
    void* pInst1BsBufMiu = NULL;
//    void *pInst1MheInternalBuf1Virt = NULL;
//    void *pInst1MheInternalBuf1Phys = NULL;
//    void *pInst1MheInternalBuf1Miu = NULL;
//    void *pInst1MheInternalBuf2Virt = NULL;
//    void *pInst1MheInternalBuf2Phys = NULL;
//    void *pInst1MheInternalBuf2Miu = NULL;
    int frame_cnt;
    int run_frame = sizeof(gYUV) / YUV_FRAME_SIZE;
    int MheIrq = 0;
#ifdef SUPPORT_CMDQ_SERVICE
    MHAL_CMDQ_BufDescript_t stCmdqBufDesp;
#endif
    int rec_frame_cnt;
    int total_frame = 0;
    struct timeval stTvStart, stTvEnd;
    struct file *fp;
    char szFn[64];
    int i;
    int md5_compare;
    int loop_run_cnt = loop_run;
    struct file *outfp;
    void* pYuvBufVitrBak = NULL;
    void* pYuvBufPhysBak = NULL;
    void* pYuvBufMiuBak = NULL;
    u16 nRegVal;
    CamOsRet_e eRet;

#if MD5_CHECK==1
    MD5_CTX stMd5 = {0};
    unsigned char u8Md5Result[16] = {0};
    unsigned char u8Md5Golden[16] = {0};

    MD5_CTX stMd5YUV = {0};
    unsigned char u8Md5ResultYUV[16] = {0};
    unsigned char u8Md5GoldenYUV[16] = {0};
#endif

    CamOsPrintf("> %s \n", __FUNCTION__);

    if(fixedgolden == 1)
    {
        memcpy(u8Md5Golden,Md5Golden1,16);

        printk(KERN_WARNING"MHE1 Golden MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
               u8Md5Golden[0], u8Md5Golden[1], u8Md5Golden[2], u8Md5Golden[3],
               u8Md5Golden[4], u8Md5Golden[5], u8Md5Golden[6], u8Md5Golden[7],
               u8Md5Golden[8], u8Md5Golden[9], u8Md5Golden[10], u8Md5Golden[11],
               u8Md5Golden[12], u8Md5Golden[13], u8Md5Golden[14], u8Md5Golden[15]);
    }

    //if(encode_framed < sizeof(gYUV)/YUV_FRAME_SIZE)
    run_frame = encode_framed;

    rec_frame_cnt = run_frame;

    CamOsPrintf("[%s] encode_framed :  %d\n", __FUNCTION__, run_frame);

    CamOsTsemInit(&stDev1FrameDone, 0);

    if(MHAL_MHE_CreateDevice(1, &gMhalMheDev1))
    {
        CamOsPrintf("[KUT] MHAL_MHE_CreateDevice core 1 Fail\n");
        return 0;
    }

    nRegVal = *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2)));
    if(clock == 5)
        nRegVal = (nRegVal & 0x00FF) | 0x1800;      // 576MHz
    else if(clock == 4)
        nRegVal = (nRegVal & 0x00FF) | 0x0400;      // 480MHz
    else if(clock == 3)
        nRegVal = (nRegVal & 0x00FF) | 0x0C00;      // 384MHz
    else if(clock == 2)
        nRegVal = (nRegVal & 0x00FF) | 0x1000;      // 288MHz
    else if(clock == 1)
        nRegVal = (nRegVal & 0x00FF) | 0x1400;      // 192MHz
    else if(clock == 0)
        nRegVal = (nRegVal & 0x00FF) | 0x0800;      // 432MHz
    *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2))) = nRegVal;

    setHeader(&param, sizeof(param));
    if(MHAL_MHE_GetDevConfig(gMhalMheDev1, E_MHAL_VENC_HW_IRQ_NUM, &param))
    {
        CamOsPrintf("[KUT] MHAL_MHE_GetDevConfig Fail\n");
        return 0;
    }

    MheIrq = param.u32Val;
    CamOsPrintf("[KUT] MHE core 1 IRQ: %d\n", MheIrq);
    if(0 != request_irq(MheIrq, _MheUtIsr_1, IRQF_SHARED, "_MheUtIsr_1", gMhalMheDev1))
    {
        CamOsPrintf("[KUT] request_irq core 1 (%d) Fail\n", param.u32Val);
        return 0;
    }

    setHeader(&param, sizeof(param));
    if(MHAL_MHE_GetDevConfig(gMhalMheDev1, E_MHAL_VENC_HW_CMDQ_BUF_LEN, &param))
    {
        CamOsPrintf("[KUT] MHAL_MHE_GetDevConfig Fail\n");
        return 0;
    }

#ifdef SUPPORT_CMDQ_SERVICE
    if(cmdq_en)
    {
        CamOsMsSleep(100);

        stCmdqBufDesp.u32CmdqBufSize = param.u32Val;
        CamOsPrintf("1 stCmdqBufDesp.u32CmdqBufSize = %d\n", stCmdqBufDesp.u32CmdqBufSize);
        stCmdqBufDesp.u32CmdqBufSizeAlign = 16;
        stCmdqBufDesp.u32MloadBufSize = 0;
        stCmdqBufDesp.u16MloadBufSizeAlign = 16;
        stCmdQInf_1 = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC1, &stCmdqBufDesp, FALSE);
        CamOsPrintf("Call MHAL_CMDQ_GetSysCmdqService: 0x%08X 0x%08X\n", stCmdQInf_1, stCmdQInf_1->MHAL_CMDQ_CheckBufAvailable);
    }
#endif

//LOOP_TEST:
    {

        if(MHAL_MHE_CreateInstance(gMhalMheDev1, &gMhalMheInst1))
        {
            CamOsPrintf("[KUT] MHAL_MHE_CreateInstance Fail\n");
            return 0;
        }

        /* Set MHE parameter */
        memset(&ResCtl, 0, sizeof(ResCtl));
        setHeader(&ResCtl, sizeof(ResCtl));
        ResCtl.u32Width = YUV_WIDTH;
        ResCtl.u32Height = YUV_HEIGHT;
        ResCtl.eFmt = E_MHAL_VENC_FMT_NV12;
        MHAL_MHE_SetParam(gMhalMheInst1, E_MHAL_VENC_265_RESOLUTION, &ResCtl);

        memset(&ResCtl, 0, sizeof(ResCtl));
        setHeader(&ResCtl, sizeof(ResCtl));
        MHAL_MHE_GetParam(gMhalMheInst1, E_MHAL_VENC_265_RESOLUTION, &ResCtl);
        CamOsPrintf("[KUT] resolution %dx%d\n", ResCtl.u32Width, ResCtl.u32Height);

        if(rc_mode == 1)
        {
            memset(&RcCtl, 0, sizeof(RcCtl));
            setHeader(&RcCtl, sizeof(RcCtl));
            RcCtl.eRcMode = E_MHAL_VENC_RC_MODE_H265CBR;
            RcCtl.stAttrH265Cbr.u32SrcFrmRateNum = 30;
            RcCtl.stAttrH265Cbr.u32SrcFrmRateDen = 1;
            RcCtl.stAttrH265Cbr.u32Gop = 29;
            RcCtl.stAttrH265Cbr.u32BitRate = 500000;
            MHAL_MHE_SetParam(gMhalMheInst1, E_MHAL_VENC_265_RC, &RcCtl);

            memset(&RcCtl, 0, sizeof(RcCtl));
            setHeader(&RcCtl, sizeof(RcCtl));
            MHAL_MHE_GetParam(gMhalMheInst1, E_MHAL_VENC_265_RC, &RcCtl);
            CamOsPrintf("[KUT] RC %d %d/%d %d\n", RcCtl.stAttrH265Cbr.u32Gop, RcCtl.stAttrH265Cbr.u32SrcFrmRateNum, RcCtl.stAttrH265Cbr.u32SrcFrmRateDen, RcCtl.stAttrH265Cbr.u32BitRate);
        }
        else
        {
            memset(&RcCtl, 0, sizeof(RcCtl));
            setHeader(&RcCtl, sizeof(RcCtl));
            RcCtl.eRcMode = E_MHAL_VENC_RC_MODE_H265FIXQP;
            RcCtl.stAttrH265FixQp.u32SrcFrmRateNum = 30;
            RcCtl.stAttrH265FixQp.u32SrcFrmRateDen = 1;
            RcCtl.stAttrH265FixQp.u32Gop = 29;
            RcCtl.stAttrH265FixQp.u32IQp = QP;
            RcCtl.stAttrH265FixQp.u32PQp = QP;
            MHAL_MHE_SetParam(gMhalMheInst1, E_MHAL_VENC_265_RC, &RcCtl);

            memset(&RcCtl, 0, sizeof(RcCtl));
            setHeader(&RcCtl, sizeof(RcCtl));
            MHAL_MHE_GetParam(gMhalMheInst1, E_MHAL_VENC_265_RC, &RcCtl);
            CamOsPrintf("[KUT] RC %d %d %d\n", RcCtl.stAttrH265FixQp.u32Gop, RcCtl.stAttrH265FixQp.u32IQp, RcCtl.stAttrH265FixQp.u32PQp);
        }

        memset(&stMheIntrBuf, 0, sizeof(MHAL_VENC_InternalBuf_t));
        setHeader(&stMheIntrBuf, sizeof(stMheIntrBuf));
        MHAL_MHE_QueryBufSize(gMhalMheInst1, &stMheIntrBuf);
        CamOsPrintf("[KUT] internal buffer size %d %d\n", stMheIntrBuf.u32IntrAlBufSize, stMheIntrBuf.u32IntrRefBufSize);

        CamOsDirectMemAlloc("MHE1IBUFF", YUV_FRAME_SIZE * FRAME_NUM, &pYuvBufVitr, &pYuvBufPhys, &pYuvBufMiu);
        CamOsDirectMemAlloc("MHE1OBUFF", YUV_FRAME_SIZE, &pInst0BsBufVitr, &pInst0BsBufPhys, &pInst0BsBufMiu);
        Inst0InOutBuf.pu32RegBase0 = NULL;
        Inst0InOutBuf.pu32RegBase1 = NULL;

        CamOsDirectMemAlloc("MHE1Backup", YUV_FRAME_SIZE * FRAME_NUM, &pYuvBufVitrBak, &pYuvBufPhysBak, &pYuvBufMiuBak);

#ifdef SUPPORT_CMDQ_SERVICE
        if(cmdq_en)
        {
            Inst0InOutBuf.pCmdQ = stCmdQInf_1;
        }
        else
        {
            Inst0InOutBuf.pCmdQ = NULL;
        }
#else
        Inst0InOutBuf.pCmdQ = NULL;
#endif
        Inst0InOutBuf.bRequestI = 0;
        Inst0InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(u32)pYuvBufMiu;
        Inst0InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
        Inst0InOutBuf.phyInputYUVBuf2 = Inst0InOutBuf.phyInputYUVBuf1 + Inst0InOutBuf.u32InputYUVBuf1Size;
        Inst0InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;
        Inst0InOutBuf.phyOutputBuf = (MS_PHYADDR)(u32)pInst0BsBufMiu;
        Inst0InOutBuf.virtOutputBuf = (MS_PHYADDR)(u32)pInst0BsBufVitr;
        Inst0InOutBuf.u32OutputBufSize = YUV_FRAME_SIZE;
        Inst0InOutBuf.pFlushCacheCb = NULL;

        sprintf(szFn, "/vendor/%dNV2", YUV_WIDTH);
        fp = OpenFile(szFn, O_RDONLY, 0);
        if(fp)
            CamOsPrintf("Open %s success\n", szFn);
        else
            CamOsPrintf("Open %s failed\n", szFn);
        ReadFile(fp, pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
        CloseFile(fp);

        Chip_Flush_Cache_Range((unsigned long)pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
        Chip_Flush_Memory();

        if(debug_yuvfile)
        {
            MD5_Init(&stMd5YUV);
            MD5_Update(&stMd5YUV, pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
            MD5_Final(u8Md5GoldenYUV, &stMd5YUV);
            CamOsPrintf("MHE1 YUV MD5 Golden: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                    u8Md5GoldenYUV[0], u8Md5GoldenYUV[1], u8Md5GoldenYUV[2], u8Md5GoldenYUV[3],
                    u8Md5GoldenYUV[4], u8Md5GoldenYUV[5], u8Md5GoldenYUV[6], u8Md5GoldenYUV[7],
                    u8Md5GoldenYUV[8], u8Md5GoldenYUV[9], u8Md5GoldenYUV[10], u8Md5GoldenYUV[11],
                    u8Md5GoldenYUV[12], u8Md5GoldenYUV[13], u8Md5GoldenYUV[14], u8Md5GoldenYUV[15]);
        }

        if(multi_stream && cmdq_en)
        {
            CamOsDirectMemAlloc("MHEOBUFF1_1", YUV_FRAME_SIZE, &pInst1BsBufVitr, &pInst1BsBufPhys, &pInst1BsBufMiu);
            Inst1InOutBuf.pu32RegBase0 = NULL;
            Inst1InOutBuf.pu32RegBase1 = NULL;
#ifdef SUPPORT_CMDQ_SERVICE
            if(cmdq_en)
            {

                Inst1InOutBuf.pCmdQ = stCmdQInf_1;
            }
            else
            {
                Inst1InOutBuf.pCmdQ = NULL;
            }
#else
            Inst1InOutBuf.pCmdQ = NULL;
#endif
            Inst1InOutBuf.bRequestI = 0;
            Inst1InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(u32)pYuvBufMiu;
            Inst1InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
            Inst1InOutBuf.phyInputYUVBuf2 = Inst1InOutBuf.phyInputYUVBuf1 + Inst1InOutBuf.u32InputYUVBuf1Size;
            Inst1InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;
            Inst1InOutBuf.phyOutputBuf = (MS_PHYADDR)(u32)pInst1BsBufMiu;
            Inst1InOutBuf.virtOutputBuf = (MS_PHYADDR)(u32)pInst1BsBufVitr;
            Inst1InOutBuf.u32OutputBufSize = YUV_FRAME_SIZE;
            Inst1InOutBuf.pFlushCacheCb = NULL;
        }

        /* Allocate MHE internal buffer and assign to driver */
        CamOsDirectMemAlloc("MHE1ALBUF", stMheIntrBuf.u32IntrAlBufSize, &pInst0MheInternalBuf1Virt, &pInst0MheInternalBuf1Phys, &pInst0MheInternalBuf1Miu);
        CamOsDirectMemAlloc("MHE1REFBUF", stMheIntrBuf.u32IntrRefBufSize, &pInst0MheInternalBuf2Virt, &pInst0MheInternalBuf2Phys, &pInst0MheInternalBuf2Miu);

        while(1)
        {
            //initial value
            total_frame = 0;
            md5_compare = 1;
#if MD5_CHECK==1
            MD5_Init(&stMd5);
#endif

            if(save_file)
            {
                sprintf(szFn, "/tmp/MHE1_%dx%d_%d.h265", YUV_WIDTH, YUV_HEIGHT, loop_run_cnt);
                outfp = OpenFile(szFn, O_RDWR | O_CREAT | O_TRUNC, 0644);
                if(outfp)
                    CamOsPrintf("Open %s success\n", szFn);
                else
                    CamOsPrintf("Open %s failed\n", szFn);
            }

            memset(pInst0MheInternalBuf1Virt, 0, stMheIntrBuf.u32IntrAlBufSize);
            memset(pInst0MheInternalBuf2Virt, 0, stMheIntrBuf.u32IntrRefBufSize);

            /* Allocate MHE internal buffer and assign to driver */
            stMheIntrBuf.pu8IntrAlVirBuf = (MS_U8 *)pInst0MheInternalBuf1Virt;
            stMheIntrBuf.phyIntrAlPhyBuf = (MS_PHYADDR)(u32)pInst0MheInternalBuf1Miu;
            stMheIntrBuf.phyIntrRefPhyBuf = (MS_PHYADDR)(u32)pInst0MheInternalBuf2Miu;
            MHAL_MHE_SetParam(gMhalMheInst1, E_MHAL_VENC_IDX_STREAM_ON, &stMheIntrBuf);

            do_gettimeofday(&stTvStart);

            for(frame_cnt = 0; frame_cnt < run_frame; frame_cnt++)
            {
                //memcpy(pYuvBufVitr, gYUV + frame_cnt * YUV_FRAME_SIZE, YUV_FRAME_SIZE);
                Inst0InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(u32)pYuvBufMiu + YUV_FRAME_SIZE * (frame_cnt % FRAME_NUM);
                Inst0InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
                Inst0InOutBuf.phyInputYUVBuf2 = Inst0InOutBuf.phyInputYUVBuf1 + Inst0InOutBuf.u32InputYUVBuf1Size;
                Inst0InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;

                MHAL_MHE_EncodeOneFrame(gMhalMheInst1, &Inst0InOutBuf);

#ifdef SUPPORT_CMDQ_SERVICE
                if(cmdq_en)
                {
                    stCmdQInf_1->MHAL_CMDQ_KickOffCmdq(stCmdQInf_1);
                }
#endif

                //CamOsTsemDown(&stDev1FrameDone);    // wait encode frame done
                eRet = CamOsTsemTimedDown(&stDev1FrameDone, 1000);
                if(eRet != CAM_OS_OK)
                {
                    CamOsPrintf("MHE1 encode timeout!! %d\n", frame_cnt);
                    break;
                }

                if(multi_stream && cmdq_en)
                {
                    CamOsTsemTimedDown(&stDev1FrameDone, 1000);    // wait encode frame done
                }

                MHAL_MHE_EncodeFrameDone(gMhalMheInst1, &Inst0EncRet);

                //CamOsPrintf("MHE1: frame:%d, size:%d \n", frame_cnt, Inst0EncRet.u32OutputBufUsed);
                if(save_file)
                    WriteFile(outfp, pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed);
#if MD5_CHECK==1
                MD5_Update(&stMd5, pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed);
#endif
                total_frame++;
            }

            u32Inst0TotalLen = 0;
            if(multi_stream && cmdq_en)
            {
                u32Inst1TotalLen = 0;
            }

#if MD5_CHECK==1
            MD5_Final(u8Md5Result, &stMd5);
            printk(KERN_WARNING"MHE1 Result MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                   u8Md5Result[0], u8Md5Result[1], u8Md5Result[2], u8Md5Result[3],
                   u8Md5Result[4], u8Md5Result[5], u8Md5Result[6], u8Md5Result[7],
                   u8Md5Result[8], u8Md5Result[9], u8Md5Result[10], u8Md5Result[11],
                   u8Md5Result[12], u8Md5Result[13], u8Md5Result[14], u8Md5Result[15]);

            if(u8Md5Golden[15] == 0 && u8Md5Golden[14] == 0 && u8Md5Golden[13] == 0)
            {
                for(i = 0; i < 16; i++)
                    u8Md5Golden[i] = u8Md5Result[i];

                printk(KERN_WARNING"MHE1 Golden MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                       u8Md5Golden[0], u8Md5Golden[1], u8Md5Golden[2], u8Md5Golden[3],
                       u8Md5Golden[4], u8Md5Golden[5], u8Md5Golden[6], u8Md5Golden[7],
                       u8Md5Golden[8], u8Md5Golden[9], u8Md5Golden[10], u8Md5Golden[11],
                       u8Md5Golden[12], u8Md5Golden[13], u8Md5Golden[14], u8Md5Golden[15]);
            }

            for(i = 0; i < 16; i++)
            {
                if(u8Md5Result[i] != u8Md5Golden[i])
                {
                    md5_compare = 0;
                    CamOsPrintf("MHE1: MD5 compare err !!! Exit burn-in test\n");
                    break;
                }
            }
#endif
            do_gettimeofday(&stTvEnd);

            if(disp_fps)
            {
                printk(KERN_WARNING"1 total time: %ldus, total frame: %d, fps %ld\n",
                       TIMEVAL_US_DIFF(stTvStart, stTvEnd),
                       total_frame,
                       (total_frame * 1000000) / TIMEVAL_US_DIFF(stTvStart, stTvEnd));
            }


            MHAL_MHE_SetParam(gMhalMheInst1, E_MHAL_VENC_IDX_STREAM_OFF, &stMheIntrBuf);

            if(md5_compare)
            {
                printk(KERN_WARNING"MHE1: MD5 compare success !!\n");
            }
            else
            {
                if(debug_yuvfile)
                {
#if 0
                    sprintf(szFn, "/vendor/VENC_burin_0222/MHE1_%dx%d_%d_%d.YUV", YUV_WIDTH, YUV_HEIGHT, loop_run_cnt, yuv_dump_cnt);
                    yuvfp = OpenFile(szFn, O_RDWR | O_CREAT | O_TRUNC, 0644);
                    if(yuvfp)
                        CamOsPrintf("Open %s success\n", szFn);
                    else
                        CamOsPrintf("Open %s failed\n", szFn);

                    yuv_dump_cnt++;

                    WriteFile(yuvfp, pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);

                    CloseFile(yuvfp);
#endif

                    MD5_Init(&stMd5YUV);
                    MD5_Update(&stMd5YUV, pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
                    MD5_Final(u8Md5ResultYUV, &stMd5YUV);
                    CamOsPrintf("MHE1 YUV MD5 Result: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                            u8Md5ResultYUV[0], u8Md5ResultYUV[1], u8Md5ResultYUV[2], u8Md5ResultYUV[3],
                            u8Md5ResultYUV[4], u8Md5ResultYUV[5], u8Md5ResultYUV[6], u8Md5ResultYUV[7],
                            u8Md5ResultYUV[8], u8Md5ResultYUV[9], u8Md5ResultYUV[10], u8Md5ResultYUV[11],
                            u8Md5ResultYUV[12], u8Md5ResultYUV[13], u8Md5ResultYUV[14], u8Md5ResultYUV[15]);

#if 1
                    if(strncmp(u8Md5ResultYUV,u8Md5GoldenYUV,16))
                    {
                        while(1)
                        {
                            CamOsPrintf("MHE1: Read input file again!\n");

                            memcpy(pYuvBufVitr, pYuvBufVitrBak, YUV_FRAME_SIZE * FRAME_NUM);

                            MD5_Init(&stMd5YUV);
                            MD5_Update(&stMd5YUV, pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
                            MD5_Final(u8Md5ResultYUV, &stMd5YUV);
                            CamOsPrintf("MHE1 Read YUV MD5 Result: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                                    u8Md5ResultYUV[0], u8Md5ResultYUV[1], u8Md5ResultYUV[2], u8Md5ResultYUV[3],
                                    u8Md5ResultYUV[4], u8Md5ResultYUV[5], u8Md5ResultYUV[6], u8Md5ResultYUV[7],
                                    u8Md5ResultYUV[8], u8Md5ResultYUV[9], u8Md5ResultYUV[10], u8Md5ResultYUV[11],
                                    u8Md5ResultYUV[12], u8Md5ResultYUV[13], u8Md5ResultYUV[14], u8Md5ResultYUV[15]);

                            if(!strncmp(u8Md5ResultYUV,u8Md5GoldenYUV,16))
                                break;
                        }

                        loop_run_cnt++; //re-compress again
                    }
                    else
#endif
                    {
                        printk(KERN_WARNING"MHE1: MD5 compare fail !!\n");
                        break;
                    }
                }
                else
                {
                    printk(KERN_WARNING"MHE1: MD5 compare fail !!\n");
                    break;
                }
            }

            if(save_file)
                CloseFile(outfp);

            if(!burnin_mode)
            {
                if(!loop_run_cnt)
                    break;
                loop_run_cnt--;
            }
        }

        if(MHAL_MHE_DestroyInstance(gMhalMheInst1))
        {
            CamOsPrintf("[KUT] MHAL_MHE_DestroyInstance Fail\n");
            return 0;
        }

        CamOsDirectMemRelease(pYuvBufVitrBak, YUV_FRAME_SIZE * FRAME_NUM);
        CamOsDirectMemRelease(pYuvBufVitr, YUV_FRAME_SIZE * FRAME_NUM);
        CamOsDirectMemRelease(pInst0BsBufVitr, YUV_FRAME_SIZE);
        CamOsDirectMemRelease(pInst0MheInternalBuf1Virt, stMheIntrBuf.u32IntrAlBufSize);
        CamOsDirectMemRelease(pInst0MheInternalBuf2Virt, stMheIntrBuf.u32IntrRefBufSize);

//    if(burnin_mode)
//    	goto LOOP_TEST;
    }

    free_irq(MheIrq, gMhalMheDev1);

    if(MHAL_MHE_DestroyDevice(gMhalMheDev1))
    {
        CamOsPrintf("[KUT] MHAL_MHE_DestroyDevice Fail\n");
        return 0;
    }

#ifdef SUPPORT_CMDQ_SERVICE
    if(cmdq_en)
    {
        CamOsPrintf("Call MHAL_CMDQ_ReleaseSysCmdqService\n");
        MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC1);
    }
#endif

    return 0;
}

static void _DualCoreTest(int test_mode)
{
    CamOsThread TaskHandle0, TaskHandle1;
    CamOsThreadAttrb_t tAttr = {0};
    s32 nTaskArg0 = 1000, nTaskArg1 = 1500;

    if(test_mode == 0 || test_mode == 2)
    {
        tAttr.nPriority = 50;
        tAttr.nStackSize = 0;
        CamOsThreadCreate(&TaskHandle0, &tAttr, (void *)_CamOsThreadTestEntry0, (void *)&nTaskArg0);
        CamOsMsSleep(500);
        //CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread get taskid: %d\n", __LINE__, TaskHandle0.eHandleObj);
        thread0_exit = 1;

        if(test_mode == 0)
            thread1_exit = 1;
    }

    if(test_mode == 1 || test_mode == 2)
    {
        tAttr.nPriority = 50;
        tAttr.nStackSize = 0;
        CamOsThreadCreate(&TaskHandle1, &tAttr, (void *)_CamOsThreadTestEntry1, (void *)&nTaskArg1);
        CamOsMsSleep(500);
        //CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread get taskid: %d\n", __LINE__, TaskHandle1.eHandleObj);
        thread1_exit = 1;

        if(test_mode == 1)
            thread0_exit = 1;
    }


#if 0
    if(test_mode == 0 || test_mode == 2)
        CamOsThreadStop(TaskHandle0);

    if(test_mode == 1 || test_mode == 2)
        CamOsThreadStop(TaskHandle1);
#endif
}

static int
MheProbe(
//    struct platform_device* pdev)
    void)
{
    if(burnin_mode)
        encode_framed = 1000;

    _DualCoreTest(test_mode);

#if 1
    while(1)
    {
        if(thread0_exit && thread1_exit)
        {
            break;
        }

        CamOsPrintf("(%d %d)\n", thread0_exit, thread1_exit);
        CamOsMsSleep(500);
    }

    CamOsPrintf("Exit MheProbe!\n");
#endif

    return 0;
}

static int
MheRemove(
//    struct platform_device* pdev)
    void)
{
    return 0;
}



static int  __init mmhe_init(void)
{
    //return platform_driver_register(&MhePdrv);
    MheProbe();
    return 0;
}

static void __exit mmhe_exit(void)
{
    //platform_driver_unregister(&MhePdrv);
    MheRemove();
}

module_init(mmhe_init);
module_exit(mmhe_exit);
