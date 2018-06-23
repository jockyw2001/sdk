
//#include <linux/platform_device.h>
//#include <linux/of.h>
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


MODULE_LICENSE("GPL");

//#define ENCODE_H264
#define ENCODE_H265

#if defined(ENCODE_H264)
    #define IP_NAME     "MFE"
    #define MHAL_VENC_CTRL_ID_RESOLUTION    E_MHAL_VENC_264_RESOLUTION
    #define MHAL_VENC_CTRL_ID_RC            E_MHAL_VENC_264_RC
    #define MHAL_VENC_CTRL_ID_ENTROPY       E_MHAL_VENC_264_ENTROPY
    #define MHAL_VENC_CTRL_ID_USER_DATA     E_MHAL_VENC_264_USER_DATA

    #define MHAL_VENC_CreateDevice          MHAL_MFE_CreateDevice
    #define MHAL_VENC_DestroyDevice         MHAL_MFE_DestroyDevice
    #define MHAL_VENC_GetDevConfig          MHAL_MFE_GetDevConfig
    #define MHAL_VENC_CreateInstance        MHAL_MFE_CreateInstance
    #define MHAL_VENC_DestroyInstance       MHAL_MFE_DestroyInstance
    #define MHAL_VENC_SetParam              MHAL_MFE_SetParam
    #define MHAL_VENC_GetParam              MHAL_MFE_GetParam
    #define MHAL_VENC_EncodeOneFrame        MHAL_MFE_EncodeOneFrame
    #define MHAL_VENC_EncodeFrameDone       MHAL_MFE_EncodeFrameDone
    #define MHAL_VENC_QueryBufSize          MHAL_MFE_QueryBufSize
    #define MHAL_VENC_IsrProc               MHAL_MFE_IsrProc

    #define E_MHAL_CMDQ_ID                  E_MHAL_CMDQ_ID_H264_VENC0
#elif defined(ENCODE_H265)
    #define IP_NAME     "MHE"
    #define MHAL_VENC_CTRL_ID_RESOLUTION    E_MHAL_VENC_265_RESOLUTION
    #define MHAL_VENC_CTRL_ID_RC            E_MHAL_VENC_265_RC
    #define MHAL_VENC_CTRL_ID_USER_DATA     E_MHAL_VENC_265_USER_DATA

    #define MHAL_VENC_CreateDevice          MHAL_MHE_CreateDevice
    #define MHAL_VENC_DestroyDevice         MHAL_MHE_DestroyDevice
    #define MHAL_VENC_GetDevConfig          MHAL_MHE_GetDevConfig
    #define MHAL_VENC_CreateInstance        MHAL_MHE_CreateInstance
    #define MHAL_VENC_DestroyInstance       MHAL_MHE_DestroyInstance
    #define MHAL_VENC_SetParam              MHAL_MHE_SetParam
    #define MHAL_VENC_GetParam              MHAL_MHE_GetParam
    #define MHAL_VENC_EncodeOneFrame        MHAL_MHE_EncodeOneFrame
    #define MHAL_VENC_EncodeFrameDone       MHAL_MHE_EncodeFrameDone
    #define MHAL_VENC_QueryBufSize          MHAL_MHE_QueryBufSize
    #define MHAL_VENC_IsrProc               MHAL_MHE_IsrProc

    #define E_MHAL_CMDQ_ID                  E_MHAL_CMDQ_ID_H265_VENC0
#endif

#define YUV_WIDTH   1920
#define YUV_HEIGHT  1088
#define YUV_FRAME_SIZE  (YUV_WIDTH*YUV_HEIGHT*3/2)
#define CROP_WIDTH 0//1024
#define CROP_HEIGHT 0// 768
#define OFFSET_X 0 //512
#define OFFSET_Y 0 //160

#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

#define FRAME_NUM 5
#define OUTBUFSIZE YUV_FRAME_SIZE

#define QP 20

#define MD5_CHECK  1


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


static unsigned int core_id = 0;
module_param(core_id, uint, S_IRUGO|S_IWUSR);

static unsigned int loop_run = 0;
module_param(loop_run, uint, S_IRUGO|S_IWUSR);

static unsigned int cmdq_en = 0;
module_param(cmdq_en, uint, S_IRUGO|S_IWUSR);

static unsigned int multi_stream = 0;
module_param(multi_stream, uint, S_IRUGO|S_IWUSR);

static unsigned int encode_framed = 10;//sizeof(gYUV)/YUV_FRAME_SIZE;
module_param(encode_framed, uint, S_IRUGO|S_IWUSR);

static unsigned int data_compare = 0;
module_param(data_compare, uint, S_IRUGO|S_IWUSR);

static unsigned int user_data = 0;
module_param(user_data, uint, S_IRUGO|S_IWUSR);

//LTR
static unsigned int ltr_en = 0;
module_param(ltr_en, uint, S_IRUGO|S_IWUSR);
static unsigned int ltr_mode = 1;   //mode1=1, mode3=3
module_param(ltr_mode, uint, S_IRUGO|S_IWUSR);
static unsigned int ltr_period = 5;
module_param(ltr_period, uint, S_IRUGO|S_IWUSR);

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

static irqreturn_t _MheUtIsr(int irq, void* priv)
{
    //CamOsPrintf("> _MheUtIsr\n");
    MHAL_MHE_IsrProc(priv);

    CamOsTsemUp(&stDev0FrameDone);

    return IRQ_HANDLED;
}

static int
MheProbe(
//    struct platform_device* pdev)
void)
{

    //s32 *pnArg = (s32 *)pUserdata;
    //MHAL_ErrCode_e err;
    MHAL_VENC_ParamInt_t param;
    MHAL_VENC_Resoluton_t ResCtl;
    MHAL_VENC_CropCfg_t CropCtl;
    MHAL_VENC_ParamRef_t RefCtl;
    MHAL_VENC_RcInfo_t RcCtl;
    MHAL_VENC_EnableIdr_t EnableIdrCtl;
    MHAL_VENC_UserData_t UserData;
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
    int run_frame = sizeof(gYUV)/YUV_FRAME_SIZE;
    int MheIrq = 0;
#ifdef SUPPORT_CMDQ_SERVICE
    MHAL_CMDQ_BufDescript_t stCmdqBufDesp;
#endif
    int rec_frame_cnt;
    int total_frame = 0;
    struct timeval stTvStart, stTvEnd;
    struct file *fp;
    char szFn[64];
    struct file *fd0;
    struct file *fd1;
    char szFileName[64];
    char UserDataBuf[1024];


    CamOsPrintf("mhe_kernel_ut.ko :  %s - %s\n", __DATE__, __TIME__);

    //if(encode_framed < sizeof(gYUV)/YUV_FRAME_SIZE)
        run_frame = encode_framed;

    rec_frame_cnt = run_frame;

    CamOsPrintf("encode_framed :  %d\n", run_frame);

    CamOsTsemInit(&stDev0FrameDone, 0);

    if (core_id > 1)
    {
        CamOsPrintf("[KUT] MHE not support core%d\n", core_id);
        return 0;
    }

    if (MHAL_MHE_CreateDevice(core_id, &gMhalMheDev0))
    {
        CamOsPrintf("[KUT] MHAL_MHE_CreateDevice Fail\n");
        return 0;
    }

    setHeader(&param, sizeof(param));
    if (MHAL_MHE_GetDevConfig(gMhalMheDev0, E_MHAL_VENC_HW_IRQ_NUM, &param))
    {
        CamOsPrintf("[KUT] MHAL_MHE_GetDevConfig Fail\n");
        return 0;
    }

    MheIrq = param.u32Val;
    CamOsPrintf("[KUT] irq %d\n", MheIrq);
    if (0 != request_irq(MheIrq, _MheUtIsr, IRQF_SHARED, "_MheUtIsr", gMhalMheDev0))
    {
        CamOsPrintf("[KUT] request_irq(%d) Fail\n", param.u32Val);
        return 0;
    }

    setHeader(&param, sizeof(param));
    if (MHAL_MHE_GetDevConfig(gMhalMheDev0, E_MHAL_VENC_HW_CMDQ_BUF_LEN, &param))
    {
        CamOsPrintf("[KUT] MHAL_MHE_GetDevConfig Fail\n");
        return 0;
    }

#ifdef SUPPORT_CMDQ_SERVICE
    if (cmdq_en)
    {
        stCmdqBufDesp.u32CmdqBufSize = param.u32Val;
        stCmdqBufDesp.u32CmdqBufSizeAlign = 16;
        stCmdqBufDesp.u32MloadBufSize = 0;
        stCmdqBufDesp.u16MloadBufSizeAlign = 16;
        stCmdQInf = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC0, &stCmdqBufDesp, FALSE);
        CamOsPrintf("Call MHAL_CMDQ_GetSysCmdqService: 0x%08X 0x%08X\n", stCmdQInf, stCmdQInf->MHAL_CMDQ_CheckBufAvailable);
    }
#endif

    if (MHAL_MHE_CreateInstance(gMhalMheDev0, &gMhalMheInst0))
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

    memset(&CropCtl, 0, sizeof(CropCtl));
    setHeader(&CropCtl, sizeof(CropCtl));
    CropCtl.stRect.u32W = CROP_WIDTH;
    CropCtl.stRect.u32H = CROP_HEIGHT;
    CropCtl.stRect.u32X = OFFSET_X;
    CropCtl.stRect.u32Y = OFFSET_Y;
    MHAL_VENC_SetParam(gMhalMheInst0, E_MHAL_VENC_265_CROP, &CropCtl);

    memset(&CropCtl, 0, sizeof(CropCtl));
    setHeader(&CropCtl, sizeof(CropCtl));
    MHAL_VENC_GetParam(gMhalMheInst0, E_MHAL_VENC_265_CROP, &CropCtl);
    CamOsPrintf("[KUT] Crop_x=%d , Crop_y=%d, Crop_w=%d, Crop_h=%d\n", CropCtl.stRect.u32X,CropCtl.stRect.u32Y,CropCtl.stRect.u32W,CropCtl.stRect.u32H);

    if(ltr_en)
    {
        memset(&RefCtl, 0, sizeof(RefCtl));
        setHeader(&RefCtl, sizeof(RefCtl));
        RefCtl.u32Base = 5;
        RefCtl.u32Enhance = ltr_period;
        RefCtl.bEnablePred = ltr_mode;
        MHAL_VENC_SetParam(gMhalMheInst0, E_MHAL_VENC_265_REF, &RefCtl);

        memset(&RefCtl, 0, sizeof(RefCtl));
        setHeader(&RefCtl, sizeof(RefCtl));
        MHAL_VENC_GetParam(gMhalMheInst0, E_MHAL_VENC_265_REF, &RefCtl);
        CamOsPrintf("[KUT] u32Base=%d , u32Enhance=%d, bEnablePred=%d\n", RefCtl.u32Base, RefCtl.u32Enhance, RefCtl.bEnablePred);
    }

#if 1
    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    RcCtl.eRcMode = E_MHAL_VENC_RC_MODE_H265FIXQP;
    RcCtl.stAttrH265FixQp.u32SrcFrmRateNum = 30;
    RcCtl.stAttrH265FixQp.u32SrcFrmRateDen = 1;
    RcCtl.stAttrH265FixQp.u32Gop = 29;
    RcCtl.stAttrH265FixQp.u32IQp = 32;
    RcCtl.stAttrH265FixQp.u32PQp = 32;
    MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_265_RC, &RcCtl);

    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    MHAL_MHE_GetParam(gMhalMheInst0, E_MHAL_VENC_265_RC, &RcCtl);
    CamOsPrintf("[KUT] RC %d %d %d\n", RcCtl.stAttrH265FixQp.u32Gop, RcCtl.stAttrH265FixQp.u32IQp, RcCtl.stAttrH265FixQp.u32PQp);
#else
    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    RcCtl.eRcMode = E_MHAL_VENC_RC_MODE_H265CBR;
    RcCtl.stAttrH265Cbr.u32SrcFrmRateNum = 30;
    RcCtl.stAttrH265Cbr.u32SrcFrmRateDen = 1;
    RcCtl.stAttrH265Cbr.u32Gop = 30;
    RcCtl.stAttrH265Cbr.u32BitRate = 500000;
    MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_265_RC, &RcCtl);

    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    MHAL_MHE_GetParam(gMhalMheInst0, E_MHAL_VENC_265_RC, &RcCtl);
    CamOsPrintf("[KUT] RC %d %d/%d %d\n", RcCtl.stAttrH265Cbr.u32Gop, RcCtl.stAttrH265Cbr.u32SrcFrmRateNum, RcCtl.stAttrH265Cbr.u32SrcFrmRateDen, RcCtl.stAttrH265Cbr.u32BitRate);
#endif


    memset(&EnableIdrCtl, 0, sizeof(EnableIdrCtl));
    setHeader(&EnableIdrCtl, sizeof(EnableIdrCtl));
    EnableIdrCtl.bEnable = 1;
    MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_ENABLE_IDR, &EnableIdrCtl);


    memset(&EnableIdrCtl, 0, sizeof(EnableIdrCtl));
    setHeader(&EnableIdrCtl, sizeof(EnableIdrCtl));
    MHAL_MHE_GetParam(gMhalMheInst0, E_MHAL_VENC_ENABLE_IDR, &EnableIdrCtl);
    CamOsPrintf("[KUT] EnableIdr %d\n", EnableIdrCtl.bEnable);


    memset(&stMheIntrBuf, 0, sizeof(MHAL_VENC_InternalBuf_t));
    setHeader(&stMheIntrBuf, sizeof(stMheIntrBuf));
    MHAL_MHE_QueryBufSize(gMhalMheInst0, &stMheIntrBuf);
    CamOsPrintf("[KUT] internal buffer size %d %d\n", stMheIntrBuf.u32IntrAlBufSize, stMheIntrBuf.u32IntrRefBufSize);

    /* Allocate MHE internal buffer and assign to driver */
    CamOsDirectMemAlloc("MHEINTRALBUF0", stMheIntrBuf.u32IntrAlBufSize, &pInst0MheInternalBuf1Virt, &pInst0MheInternalBuf1Phys, &pInst0MheInternalBuf1Miu);
    CamOsDirectMemAlloc("MHEINTRREFBUF0", stMheIntrBuf.u32IntrRefBufSize, &pInst0MheInternalBuf2Virt, &pInst0MheInternalBuf2Phys, &pInst0MheInternalBuf2Miu);
    stMheIntrBuf.pu8IntrAlVirBuf = (MS_U8 *)pInst0MheInternalBuf1Virt;
    stMheIntrBuf.phyIntrAlPhyBuf = (MS_PHYADDR)(u32)pInst0MheInternalBuf1Miu;
    stMheIntrBuf.phyIntrRefPhyBuf = (MS_PHYADDR)(u32)pInst0MheInternalBuf2Miu;
    MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_IDX_STREAM_ON, &stMheIntrBuf);

    if (multi_stream && cmdq_en)
    {
        if (MHAL_MHE_CreateInstance(gMhalMheDev0, &gMhalMheInst1))
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


        memset(&EnableIdrCtl, 0, sizeof(EnableIdrCtl));
        setHeader(&EnableIdrCtl, sizeof(EnableIdrCtl));
        EnableIdrCtl.bEnable = 1;
        MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_ENABLE_IDR, &EnableIdrCtl);


        memset(&EnableIdrCtl, 0, sizeof(EnableIdrCtl));
        setHeader(&EnableIdrCtl, sizeof(EnableIdrCtl));
        MHAL_MHE_GetParam(gMhalMheInst0, E_MHAL_VENC_ENABLE_IDR, &EnableIdrCtl);
        CamOsPrintf("[KUT] EnableIdr %d\n", EnableIdrCtl.bEnable);


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

    CamOsDirectMemAlloc("MHEIBUFF", YUV_FRAME_SIZE, &pYuvBufVitr, &pYuvBufPhys, &pYuvBufMiu);
    CamOsDirectMemAlloc("MHEOBUFF0", YUV_FRAME_SIZE, &pInst0BsBufVitr, &pInst0BsBufPhys, &pInst0BsBufMiu);
    Inst0InOutBuf.pu32RegBase0 = NULL;
    Inst0InOutBuf.pu32RegBase1 = NULL;
#ifdef SUPPORT_CMDQ_SERVICE
    if (cmdq_en)
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
    Inst0InOutBuf.u32OutputBufSize = YUV_FRAME_SIZE;
    Inst0InOutBuf.pFlushCacheCb = NULL;

    sprintf(szFn, "/vendor/%dx%d.NV12", YUV_WIDTH,YUV_HEIGHT);
    fp = OpenFile(szFn, O_RDONLY | O_SYNC, 0644);
    if(fp)
        CamOsPrintf("Open %s success\n", szFn);
    else
        CamOsPrintf("Open %s failed\n", szFn);

#if 1
    CamOsSnprintf(szFileName, sizeof(szFileName), "/vendor/kernel_ut/%dx%d_ch%d_frm%d.h265", YUV_WIDTH, YUV_HEIGHT, 0, run_frame);
    fd0 = OpenFile(szFileName, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(IS_ERR(fd0))
    {
        CamOsPrintf("kernel open file fail, err %d\n", PTR_ERR(fd0));
        return 0;
    }
    else
    {
        CamOsPrintf("kernel open file success\n");
    }
#endif

    if (multi_stream && cmdq_en)
    {
        CamOsDirectMemAlloc("MHEOBUFF1", YUV_FRAME_SIZE, &pInst1BsBufVitr, &pInst1BsBufPhys, &pInst1BsBufMiu);
        Inst1InOutBuf.pu32RegBase0 = NULL;
        Inst1InOutBuf.pu32RegBase1 = NULL;
#ifdef SUPPORT_CMDQ_SERVICE
        if (cmdq_en)
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
        Inst0InOutBuf.u32InputYUVBuf1Size = YUV_WIDTH * YUV_HEIGHT;
        Inst0InOutBuf.phyInputYUVBuf2 = Inst0InOutBuf.phyInputYUVBuf1 + Inst0InOutBuf.u32InputYUVBuf1Size;
        Inst0InOutBuf.u32InputYUVBuf2Size = YUV_WIDTH * YUV_HEIGHT / 2;
        Inst1InOutBuf.phyOutputBuf = (MS_PHYADDR)(u32)pInst1BsBufMiu;
        Inst1InOutBuf.virtOutputBuf = (MS_PHYADDR)(u32)pInst1BsBufVitr;
        Inst1InOutBuf.u32OutputBufSize = YUV_FRAME_SIZE;
        Inst1InOutBuf.pFlushCacheCb = NULL;

        CamOsSnprintf(szFileName, sizeof(szFileName), "/vendor/kernel_ut/kernel_ut_ch%d_frm%d.h265", 1, run_frame);
        fd1 = OpenFile(szFileName, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if(IS_ERR(fd1))
        {
            CamOsPrintf("kernel open file fail, err %d\n", PTR_ERR(fd1));
            return 0;
        }
        else
        {
            CamOsPrintf("kernel open file success\n");
        }
    }

    do_gettimeofday(&stTvStart);

    while (1)
    {
        for(frame_cnt = 0; frame_cnt < run_frame; frame_cnt++)
        {
            //memcpy(pYuvBufVitr, gYUV + frame_cnt * YUV_FRAME_SIZE, YUV_FRAME_SIZE);
            ReadFile(fp, pYuvBufVitr,YUV_FRAME_SIZE);

            if (user_data)
            {
                memset(&UserData, 0, sizeof(UserData));
                setHeader(&UserData, sizeof(UserData));
                CamOsSnprintf(UserDataBuf, sizeof(UserDataBuf), "This is Frame %d", frame_cnt);
                UserData.pu8Data = UserDataBuf;
                UserData.u32Len = strlen(UserDataBuf);
                MHAL_MHE_SetParam(gMhalMheInst0, E_MHAL_VENC_265_USER_DATA, &UserData);
            }
            MHAL_MHE_EncodeOneFrame(gMhalMheInst0, &Inst0InOutBuf);

            if (multi_stream && cmdq_en)
            {
                if (user_data)
                {
                    memset(&UserData, 0, sizeof(UserData));
                    setHeader(&UserData, sizeof(UserData));
                    CamOsSnprintf(UserDataBuf, sizeof(UserDataBuf), "This is Frame %d", frame_cnt);
                    UserData.pu8Data = UserDataBuf;
                    UserData.u32Len = strlen(UserDataBuf);
                    MHAL_VENC_SetParam(gMhalMheInst1, MHAL_VENC_CTRL_ID_USER_DATA, &UserData);
                }
                MHAL_MHE_EncodeOneFrame(gMhalMheInst1, &Inst1InOutBuf);
            }

#ifdef SUPPORT_CMDQ_SERVICE
            if (cmdq_en)
            {
                stCmdQInf->MHAL_CMDQ_KickOffCmdq(stCmdQInf);
            }
#endif

            CamOsTsemDown(&stDev0FrameDone);    // wait encode frame done
            if (multi_stream && cmdq_en)
            {
                CamOsTsemDown(&stDev0FrameDone);    // wait encode frame done
            }

            MHAL_MHE_EncodeFrameDone(gMhalMheInst0, &Inst0EncRet);
            if (multi_stream && cmdq_en)
            {
                MHAL_MHE_EncodeFrameDone(gMhalMheInst1, &Inst1EncRet);
            }

#if 0
            if(fp >= 0)
            {
                WriteFile(fp, pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed);
            }
#endif

            CamOsPrintf("MHE0: frame:%d, size:%d \n", frame_cnt, Inst0EncRet.u32OutputBufUsed);

            WriteFile(fd0, pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed);

            //print_hex("MHE0:bitstream", pInst0BsBufVitr, 16);
//            if(data_compare)
//            {
//                if(Inst0EncRet.u32OutputBufUsed && _CompareArrays((u8 *)(gH265_qp32 + u32Inst0TotalLen), (u8 *)pInst0BsBufVitr, Inst0EncRet.u32OutputBufUsed) == 0)
//                    CamOsPrintf("Inst0 Encode Result Compare OK!!!\n");
//                else
//                    CamOsPrintf("Inst0 Encode Result Compare Fail!!!\n");
//                u32Inst0TotalLen += Inst0EncRet.u32OutputBufUsed;
//            }

            if (multi_stream && cmdq_en)
            {
                WriteFile(fd1, pInst1BsBufVitr, Inst1EncRet.u32OutputBufUsed);
//                CamOsPrintf("MHE1: frame:%d, size:%d \n", frame_cnt, Inst1EncRet.u32OutputBufUsed);
//
//                if(data_compare)
//                {
//                    print_hex("MHE1:bitstream", pInst1BsBufVitr, 20);
//                    if(Inst1EncRet.u32OutputBufUsed && _CompareArrays((u8 *)(gH265_qp50 + u32Inst1TotalLen), (u8 *)pInst0BsBufVitr, Inst1EncRet.u32OutputBufUsed) == 0)
//                        CamOsPrintf("Inst1 Encode Result Compare OK!!!\n");
//                    else
//                        CamOsPrintf("Inst1 Encode Result Compare Fail!!!\n");
//                    u32Inst1TotalLen += Inst1EncRet.u32OutputBufUsed;
//                }

            }

            total_frame++;
        }

        if(!IS_ERR(fd0))
        {
            CloseFile(fd0);
        }

        u32Inst0TotalLen = 0;
        if (multi_stream && cmdq_en)
        {
            u32Inst1TotalLen = 0;

            if(!IS_ERR(fd1))
            {
                CloseFile(fd1);
            }
        }

        if (!loop_run)
            break;

        loop_run--;
    }

    do_gettimeofday(&stTvEnd);

    CloseFile(fp);

    CamOsPrintf("total time: %dus, total frame: %d, fps %d\n",
                TIMEVAL_US_DIFF(stTvStart, stTvEnd),
                total_frame,
                (total_frame * 1000000)/TIMEVAL_US_DIFF(stTvStart, stTvEnd));

    if(MHAL_MHE_DestroyInstance(gMhalMheInst0))
    {
        CamOsPrintf("[KUT] MHAL_MHE_DestroyInstance Fail\n");
        return 0;
    }
    if (multi_stream && cmdq_en)
    {
        if(MHAL_MHE_DestroyInstance(gMhalMheInst1))
        {
            CamOsPrintf("[KUT] MHAL_MHE_DestroyInstance Fail\n");
            return 0;
        }
    }

    free_irq(MheIrq, gMhalMheDev0);

    if(MHAL_MHE_DestroyDevice(gMhalMheDev0))
    {
        CamOsPrintf("[KUT] MHAL_MHE_DestroyDevice Fail\n");
        return 0;
    }

    CamOsDirectMemRelease(pYuvBufVitr, YUV_FRAME_SIZE);
    CamOsDirectMemRelease(pInst0BsBufVitr, YUV_FRAME_SIZE);
    CamOsDirectMemRelease(pInst0MheInternalBuf1Virt, stMheIntrBuf.u32IntrAlBufSize);
    CamOsDirectMemRelease(pInst0MheInternalBuf2Virt, stMheIntrBuf.u32IntrRefBufSize);

    if (multi_stream && cmdq_en)
    {
        CamOsDirectMemRelease(pInst1BsBufVitr, YUV_FRAME_SIZE);
        CamOsDirectMemRelease(pInst1MheInternalBuf1Virt, stMheIntrBuf.u32IntrAlBufSize);
        CamOsDirectMemRelease(pInst1MheInternalBuf2Virt, stMheIntrBuf.u32IntrRefBufSize);
    }

#ifdef SUPPORT_CMDQ_SERVICE
    if (cmdq_en)
    {
        CamOsPrintf("Call MHAL_CMDQ_ReleaseSysCmdqService\n");
        MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC0);
    }
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
