#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
///#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <linux/time.h>

#include <linux/version.h>
#include <linux/kernel.h>
#include "mhal_common.h"
#include "mhal_vpe.h"


//#define VPE_FAKE_TEST_USE_GE
#ifdef VPE_FAKE_TEST_USE_GE
#include "apiGFX.h"
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif
#define VPE_IRQ_NUM     (104)
#define VPE_CHANNEL_MAX (64)
#define VPE_PORT_MAX    (4)

#define DBG_ERR(fmt, args...)  do {printk("[MHAL  ERR]: %s [%d]: ", __FUNCTION__, __LINE__);printk(fmt, ##args);}while(0)
//#define DBG_INFO(fmt, args...) do {printk("[MHAL INFO]: %s [%d]: ", __FUNCTION__, __LINE__);printk(fmt, ##args);}while(0)
#define DBG_INFO(fmt, args...)

typedef struct {
    struct list_head node;
    MS_U32 u32ChnnId;
}MHalVpeIq_t;

typedef struct {
    struct list_head node;
    MS_U32 u32ChnnId;
    MHalVpeSclOutputSizeConfig_t astDispWin[4];
    MHalVpeSclCropConfig_t stCrop;
    MHalVpeSclInputSizeConfig_t stSrc;
}MHalVpeScl_t;


typedef struct {
    struct list_head node;
    MS_U32 u32ChnnId;
}MHalVpeIsp_t;

typedef struct
{
    MS_U64 u64BitMap;
    struct semaphore stMutex;
    struct list_head active_list;
    MS_U32 u32Cnt;
} VpeManager;

#define VPE_Manager_AddMember(pMgr, pCtx)\
                do {\
                    down(&(pMgr)->stMutex);\
                    if (list_empty(&(pMgr)->active_list))\
                    {\
                        (pCtx)->u32ChnnId = 0;\
                    }\
                    else\
                    {\
                        int i;\
                        for (i = 0; i < 64; i++)\
                        {\
                            if (0 == ((pMgr)->u64BitMap && (1 << i)))\
                            {\
                                (pCtx)->u32ChnnId = i;\
                                break;\
                            }\
                        }\
                    }\
                    (pMgr)->u32Cnt++;\
                    list_add(&(pCtx)->node, &(pMgr)->active_list);\
                    up(&(pMgr)->stMutex);\
                }while(0)

#define VPE_Manager_DelMember(pMgr, pCtx)\
                do {\
                    down(&(pMgr)->stMutex);\
                    list_del(&(pCtx)->node);\
                    (pMgr)->u64BitMap &= ~(1 << (pCtx)->u32ChnnId);\
                    (pMgr)->u32Cnt--;\
                    up(&(pMgr)->stMutex);\
                    kfree((pCtx));\
                } while(0)


static VpeManager _gIqMgr = {
    .u64BitMap = 0,
    .u32Cnt = 0,
};

static VpeManager _gSclMgr = {
    .u64BitMap = 0,
    .u32Cnt = 0,
};

static VpeManager _gIspMgr = {
    .u64BitMap = 0,
    .u32Cnt = 0,
};

static void VPE_Manager_Init(VpeManager *pMgr)
{
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
    init_MUTEX(&pMgr->stMutex);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
                sema_init(&pMgr->stMutex, 1);
#endif

    pMgr->u64BitMap = 0;
    INIT_LIST_HEAD(&pMgr->active_list);
}

static inline MS_BOOL VPE_Manager_CheckFull(VpeManager *pMgr)
{
    if ((pMgr->u32Cnt + 1) == VPE_CHANNEL_MAX)
    {
        return TRUE;
    }
    return FALSE;
}

static inline MS_BOOL VPE_Manager_CheckEmpty(VpeManager *pMgr)
{
    if (pMgr->u32Cnt == 0)
    {
        return TRUE;
    }
    return FALSE;
}

MS_BOOL MHalVpeCreateIqInstance(const MHalAllocPhyMem_t *pstAlloc ,const MHalVpeSclWinSize_t *pstMaxWin, void **pCtx)
{
    MS_BOOL bRet     = FALSE;
    MHalVpeIq_t *pIq = NULL;
    VpeManager  *pMgr= &_gIqMgr;

    if ((NULL == pCtx) || (NULL == pstAlloc))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    if (VPE_Manager_CheckEmpty(pMgr) == TRUE)
    {
        VPE_Manager_Init(pMgr);
    }
    else if (VPE_Manager_CheckFull(pMgr) == TRUE)
    {
        DBG_ERR("channel is full !!!\n");
        return bRet;
    }

    pIq = kmalloc(sizeof(*pIq), GFP_KERNEL);
    if (pIq != NULL)
    {
        VPE_Manager_AddMember(pMgr, pIq);
        *pCtx = pIq;
        DBG_INFO("sucess pCtx: %p Chnn: %d.\n", *pCtx, pIq->u32ChnnId);
    }
    else
    {
        DBG_ERR("Fail to alloc Ctx.\n");
    }

    bRet = TRUE;
    return bRet;
}

MS_BOOL MHalVpeDestroyIqInstance(void *pCtx)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIq_t *pIq = (MHalVpeIq_t *)pCtx;
    VpeManager  *pMgr= &_gIqMgr;

    if (NULL == pCtx)
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    DBG_INFO("sucess pCtx: %p Chnn: %d.\n", pCtx, pIq->u32ChnnId);
    VPE_Manager_DelMember(pMgr, pIq);
    bRet = TRUE;
    return bRet;
}

#define testCtx(pCtx) DBG_INFO("%p -> Chnn: %d.\n", (pCtx), (pCtx)->u32ChnnId)

MS_BOOL MHalVpeIqProcess(void *pCtx, const MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIq_t *pIq = (MHalVpeIq_t *)pCtx;
    if ((NULL == pIq) || (NULL == pstCmdQInfo))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }
    // test
    testCtx(pIq);
    DBG_INFO("sucess pCtx: %p pstCmdQInfo: %p.\n", pCtx, pstCmdQInfo);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeIqDbgLevel(void *p)
{
    MS_BOOL bRet = FALSE;
    if (NULL == p)
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test

    DBG_INFO("sucess p: %p.\n", p);

    bRet = TRUE;
    return bRet;
}

MS_BOOL MHalVpeIqConfig(void *pCtx, const MHalVpeIqConfig_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIq_t *pIq = (MHalVpeIq_t *)pCtx;

    if ((NULL == pIq) ||(pCfg == NULL))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pIq);

    DBG_INFO("sucess pCtx: %p pCfg: %p.\n", pCtx, pCfg);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeIqOnOff(void *pCtx, const MHalVpeIqOnOff_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIq_t *pIq = (MHalVpeIq_t *)pCtx;

    if ((NULL == pIq) || (pCfg == NULL))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pIq);

    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeIqGetWdrRoiHist(void *pCtx, MHalVpeIqWdrRoiReport_t * pstRoiReport)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIq_t *pIq = (MHalVpeIq_t *)pCtx;
    int i = 0;
    if ((NULL == pIq) || (NULL == pstRoiReport))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pIq);
    for (i = 0; i < ROI_WINDOW_MAX; i++)
    {
        pstRoiReport->u32Y[i] = ((i + 1) * 10);
    }

    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeIqSetWdrRoiHist(void *pCtx, const MHalVpeIqWdrRoiHist_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIq_t *pIq = (MHalVpeIq_t *)pCtx;

    if ((NULL == pIq) || (NULL == pCfg))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }
    // test
    testCtx(pIq);
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}

// Register write via cmdQ
MS_BOOL MHalVpeIqSetWdrRoiMask(void *pCtx,const MS_BOOL bEnMask, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIq_t *pIq = (MHalVpeIq_t *)pCtx;

    if ((NULL == pIq) || (NULL == pstCmdQInfo))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pIq);
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
// Register write via cmdQ
MS_BOOL MHalVpeIqSetDnrTblMask(void *pCtx,const MS_BOOL bEnMask, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIq_t *pIq = (MHalVpeIq_t *)pCtx;

    if ((NULL == pIq) || (NULL == pstCmdQInfo))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pIq);
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}

//ROI Buffer

//ISP
MS_BOOL MHalVpeCreateIspInstance(const MHalAllocPhyMem_t *pstAlloc ,void **pCtx)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIsp_t *pIsp = NULL;
    VpeManager  *pMgr= &_gIspMgr;

    if ((NULL == pCtx) || (NULL == pstAlloc))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    if (VPE_Manager_CheckEmpty(pMgr) == TRUE)
    {
        VPE_Manager_Init(pMgr);
    }
    else if (VPE_Manager_CheckFull(pMgr) == TRUE)
    {
        DBG_ERR("channel is full !!!\n");
        return bRet;
    }

    pIsp = kmalloc(sizeof(*pIsp), GFP_KERNEL);
    if (pIsp != NULL)
    {
        VPE_Manager_AddMember(pMgr, pIsp);
        *pCtx = pIsp;
        DBG_INFO("sucess pCtx: %p Chnn: %d.\n", *pCtx, pIsp->u32ChnnId);
    }
    else
    {
        DBG_ERR("Fail to alloc Ctx.\n");
    }

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeDestroyIspInstance(void *pCtx)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIsp_t *pIsp = (MHalVpeIsp_t *)pCtx;
    VpeManager  *pMgr= &_gIspMgr;

    if (NULL == pCtx)
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    DBG_INFO("sucess pCtx: %p Chnn: %d.\n", pCtx, pIsp->u32ChnnId);
    VPE_Manager_DelMember(pMgr, pIsp);

    bRet = TRUE;
    return bRet;
}

#ifdef VPE_FAKE_TEST_USE_GE

static MHalVpeSclOutputBufferConfig_t _gSclVideo;
static MHalVpeScl_t * _gpVpeScl = NULL;
typedef struct MI_GFX_Surface_s
{
    MS_PHYADDR phyAddr;
    GFX_Buffer_Format eColorFmt;
    MS_U32 u32Width;
    MS_U32 u32Height;
    MS_U32 u32Stride;
} MI_GFX_Surface_t;
typedef struct MI_GFX_Rect_s
{
    MS_S32 s32Xpos;
    MS_S32 s32Ypos;
    MS_U32 u32Width;
    MS_U32 u32Height;
} MI_GFX_Rect_t;

#if 0
static MI_BOOL _ScalingByGe(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect)
{
    MI_U16 u16Fence = 0xFF;
    MI_GFX_Opt_t stOpt;
    MI_GFX_Opt_t *pstBlitOpt = &stOpt;
    memset(pstBlitOpt, 0, sizeof(*pstBlitOpt));
    MI_GFX_Open();
    DBG_INFO("Start bit blit.\n");

    DBG_INFO("Src : Rect = {%d, %d, %d, %d}.\n", pstSrcRect->s32Xpos, pstSrcRect->s32Ypos, pstSrcRect->u32Width, pstSrcRect->u32Height);
    DBG_INFO("Src surface = {.phyAddr: %llx, .eColorFmt: %d, .u32Width: %u, .u32Height: %u, .u32Stride: %u}.\n",
        pstSrc->phyAddr, pstSrc->eColorFmt, pstSrc->u32Width, pstSrc->u32Height, pstSrc->u32Stride);

    DBG_INFO("Dest: Rect = {%d, %d, %d, %d}.\n", pstDstRect->s32Xpos, pstDstRect->s32Ypos, pstDstRect->u32Width, pstDstRect->u32Height);
    DBG_INFO("Dest surface = {.phyAddr: %llx, .eColorFmt: %d, .u32Width: %u, .u32Height: %u, .u32Stride: %u}.\n",
        pstDst->phyAddr, pstDst->eColorFmt, pstDst->u32Width, pstDst->u32Height, pstDst->u32Stride);
    DBG_INFO("Flip: Src: 0x%llx Dest: 0x%llx.\n", pstSrc->phyAddr,pstDst->phyAddr);

    pstBlitOpt->bEnGfxRop = FALSE;
    pstBlitOpt->eRopCode = E_MI_GFX_ROP_NONE;
    pstBlitOpt->eSrcDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    pstBlitOpt->eDstDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    pstBlitOpt->eMirror = E_MI_GFX_MIRROR_NONE;
    pstBlitOpt->eRotate = E_MI_GFX_ROTATE_0;
    pstBlitOpt->eSrcYuvFmt = E_MI_GFX_YUV_YVYU;
    pstBlitOpt->eDstYuvFmt = E_MI_GFX_YUV_YVYU;
    pstBlitOpt->stClipRect.s32Xpos = 0;
    pstBlitOpt->stClipRect.s32Ypos = 0;
    pstBlitOpt->stClipRect.u32Width  = 0;
    pstBlitOpt->stClipRect.u32Height = 0;

    MI_GFX_BitBlit(pstSrc, pstSrcRect, pstDst, pstDstRect, pstBlitOpt, &u16Fence);
    DBG_INFO("Bit blit done.\n");

    DBG_INFO("Start wait fence: 0x%x.\n", u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    DBG_INFO("Wait done.\n");

    MI_GFX_Close();
    return TRUE;
}
#else
static MS_BOOL _gGeInited = FALSE;
static MS_BOOL _ScalingByGe(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect)
{

#if 1
    GFX_Config gfx_config;
    MS_U32 u32GfxScale = GFXDRAW_FLAG_DEFAULT;
    GFX_BufferInfo stSrcbuf, stDstbuf;
    GFX_DrawRect stBitbltInfo;
    GFX_Point stP0, stP1;
    MS_BOOL bMirrorH =FALSE;
    MS_BOOL bMirrorV =FALSE;
    MS_U16 u16TagId;

    gfx_config.u8Miu = 0;
    gfx_config.u8Dither = FALSE;
    gfx_config.u32VCmdQSize = 0;
    gfx_config.u32VCmdQAddr = 0;
    gfx_config.bIsHK = 1;
    gfx_config.bIsCompt = FALSE;
    if (_gGeInited == FALSE)
    {
        _gGeInited = TRUE;
        MApi_GFX_Init(&gfx_config);
    }
    //do scaling
    MApi_GFX_SetDC_CSC_FMT(GFX_YUV_RGB2YUV_PC, GFX_YUV_OUT_255, GFX_YUV_IN_255, GFX_YUV_YUYV,  GFX_YUV_YUYV);
    //MApi_GFX_SetDC_CSC_FMT(GFX_YUV_RGB2YUV_PC, GFX_YUV_OUT_PC, GFX_YUV_IN_255, GFX_YUV_YUYV,  GFX_YUV_YUYV);
    //MApi_GFX_SetAlpha(FALSE, COEF_ONE, ABL_FROM_ASRC, 0xFF);

    memset(&stSrcbuf, 0, sizeof(GFX_BufferInfo));
    memset(&stDstbuf, 0, sizeof(GFX_BufferInfo));

    stSrcbuf.u32Addr = pstSrc->phyAddr;//u2Addr;//_stNVRBlitFrameParam.u32SrcAddr;
    stSrcbuf.u32Width =  pstSrc->u32Width;//pstTravelInfo->stDIPRProperty.u16Width;//_stNVRBlitFrameParam.u32SrcWidth;
    stSrcbuf.u32Height =  pstSrc->u32Height;//pstTravelInfo->stDIPRProperty.u16Height;//_stNVRBlitFrameParam.u32SrcHeight;
    stSrcbuf.u32Pitch = pstSrc->u32Stride;//pstTravelInfo->u16MemoryPitch;//3840;//u32TgtSrcPitch;//0;
    stSrcbuf.u32ColorFmt = GFX_FMT_YUV422;

    stDstbuf.u32Addr = pstDst->phyAddr;//pstTravelInfo->u32MemoryPhyAddress;//_stNVRBlitFrameParam.u32DstAddr;
    stDstbuf.u32Width = pstDst->u32Width;//pstTravelInfo->u16TravelWidth;//_stNVRBlitFrameParam.u32DstWidth;
    stDstbuf.u32Height = pstDst->u32Height;//pstTravelInfo->u16TravelHeight;//_stNVRBlitFrameParam.u32DstHeight;
    stDstbuf.u32Pitch = pstDst->u32Stride;//pstTravelInfo->u16MemoryPitch;//3840;//u32TgtDstPitch;//0;
    stDstbuf.u32ColorFmt = GFX_FMT_YUV422;

    DBG_INFO("lsy CAP OSD, %#llx,%d:%d,%d --> %#llx,%d:%d,%d;\n",
        stSrcbuf.u32Addr, stSrcbuf.u32Width, stSrcbuf.u32Height, stSrcbuf.u32Pitch,
        stDstbuf.u32Addr, stDstbuf.u32Width, stDstbuf.u32Height, stDstbuf.u32Pitch);

    MApi_GFX_BeginDraw();
    MApi_GFX_EnableDFBBlending(FALSE);
    MApi_GFX_EnableAlphaBlending(FALSE);
    //MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ASRC);

    if ((stSrcbuf.u32Width == stDstbuf.u32Width) && (stSrcbuf.u32Height == stDstbuf.u32Height))
    {
        u32GfxScale = GFXDRAW_FLAG_DEFAULT;
    }
    else
    {
        u32GfxScale = GFXDRAW_FLAG_SCALE;
    }

    stP0.x = 0;
    stP0.y = 0;
    stP1.x = stDstbuf.u32Width;
    stP1.y = stDstbuf.u32Height;
    MApi_GFX_SetClip(&stP0, &stP1);

    if (MApi_GFX_SetSrcBufferInfo(&stSrcbuf, 0) != GFX_SUCCESS)
    {
        DBG_ERR("[DipDataHandleThread]GFX set SrcBuffer failed\n");
        return FALSE;
    }

    if (MApi_GFX_SetDstBufferInfo(&stDstbuf, 0) != GFX_SUCCESS)
    {
        DBG_ERR("[DipDataHandleThread]GFX set DetBuffer failed\n");
        return FALSE;
    }

    stBitbltInfo.srcblk.x = pstSrcRect->s32Xpos;
    stBitbltInfo.srcblk.y = pstSrcRect->s32Ypos;
    stBitbltInfo.srcblk.width = pstSrcRect->u32Width;
    stBitbltInfo.srcblk.height = pstSrcRect->u32Height;

    stBitbltInfo.dstblk.x = pstDstRect->s32Xpos;
    stBitbltInfo.dstblk.y = pstDstRect->s32Ypos;
    stBitbltInfo.dstblk.width = pstDstRect->u32Width;
    stBitbltInfo.dstblk.height = pstDstRect->u32Height;

    if (bMirrorH)
    {
        stBitbltInfo.srcblk.x += stBitbltInfo.srcblk.width - 1;
    }

    if (bMirrorV)
    {
        stBitbltInfo.srcblk.y += stBitbltInfo.srcblk.height - 1;
    }

    MApi_GFX_SetMirror(bMirrorH, bMirrorV);
    if (MApi_GFX_BitBlt(&stBitbltInfo, u32GfxScale) != GFX_SUCCESS)
    {
        DBG_ERR("[DipDataHandleThread]GFX BitBlt failed\n");
        return FALSE;
    }
    u16TagId = MApi_GFX_SetNextTAGID();
    MApi_GFX_WaitForTAGID(u16TagId );
#if 0
    if (MApi_GFX_FlushQueue() != GFX_SUCCESS)
    {
        DBG_ERR("[DipDataHandleThread]GFX FlushQueue failed\n");
        return FALSE;
    }
#endif
    MApi_GFX_EndDraw();

    //MApi_GE_Exit();
    #endif
    return TRUE;
}
#endif
#endif

MS_BOOL MHalVpeIspProcess(void *pCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeIspVideoInfo_t *pstVidInfo)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIsp_t *pIsp = (MHalVpeIsp_t *)pCtx;
#ifdef VPE_FAKE_TEST_USE_GE
    MI_GFX_Surface_t stSrc;
    MI_GFX_Rect_t stSrcRect;
    MI_GFX_Surface_t stDst;
    MI_GFX_Rect_t stDstRect;
    int i = 0;
    MHalVpeScl_t * pScl = _gpVpeScl;
    MHalVpeSclOutputBufferConfig_t *pBuffer = &_gSclVideo;
    MI_GFX_Rect_t *pstSrcRect = NULL;
    //struct timespec stTime1, stTime2;
#endif
    if ((NULL == pIsp) || (NULL == pstVidInfo))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pIsp);
#ifdef VPE_FAKE_TEST_USE_GE
    if (pBuffer != NULL)
    {
        memset(&stSrc, 0, sizeof(stSrc));
        stSrc.eColorFmt = GFX_FMT_YUV422;
        stSrc.phyAddr   = pstVidInfo->u64PhyAddr[0];
        stSrc.u32Stride = pstVidInfo->u32Stride[0];
        stSrc.u32Height = pScl->stSrc.u16Height;
        stSrc.u32Width  = pScl->stSrc.u16Width;

        memset(&stSrcRect, 0, sizeof(stSrcRect));
        pstSrcRect = &stSrcRect;
        if (pScl->stCrop.bCropEn == TRUE)
        {
            stSrcRect.s32Xpos  = pScl->stCrop.stCropWin.u16X;
            stSrcRect.u32Width = pScl->stCrop.stCropWin.u16Width;
            stSrcRect.s32Ypos  = pScl->stCrop.stCropWin.u16Y;
            stSrcRect.u32Height = pScl->stCrop.stCropWin.u16Height;

            DBG_INFO("Src : Rect = {%d, %d, %d, %d}.\n", pstSrcRect->s32Xpos, pstSrcRect->s32Ypos, pstSrcRect->u32Width, pstSrcRect->u32Height);
        }
        else
        {
            stSrcRect.s32Xpos = 0;
            stSrcRect.u32Width = stSrc.u32Width;
            stSrcRect.s32Ypos = 0;
            stSrcRect.u32Height = stSrc.u32Height;

            DBG_INFO("Src : Rect = {%d, %d, %d, %d}.\n", pstSrcRect->s32Xpos, pstSrcRect->s32Ypos, pstSrcRect->u32Width, pstSrcRect->u32Height);
        }

        for (i = 0; i < 4; i++)
        {
            if (pBuffer->stCfg[i].bEn == FALSE)
            {
                continue;
            }
            memset(&stDst, 0, sizeof(stDst));
            stDst.eColorFmt = GFX_FMT_YUV422;
            stDst.phyAddr   = pBuffer->stCfg[i].stBufferInfo.u64PhyAddr[0];
            stDst.u32Stride   = pBuffer->stCfg[i].stBufferInfo.u32Stride[0];
            stDst.u32Height = pScl->astDispWin[i].u16Height;
            stDst.u32Width  = pScl->astDispWin[i].u16Width;

            memset(&stDstRect, 0, sizeof(stDstRect));
            stDstRect.s32Xpos = 0;
            stDstRect.u32Width = stDst.u32Width;
            stDstRect.s32Ypos = 0;
            stDstRect.u32Height = stDst.u32Height;
            DBG_INFO("Port: %d use GE.\n", i);
            //memset(&stTime1, 0, sizeof(stTime1));
            //memset(&stTime2, 0, sizeof(stTime2));
            //do_posix_clock_monotonic_gettime(&stTime1);
            _ScalingByGe(&stSrc, &stSrcRect, &stDst, &stDstRect);
            //do_posix_clock_monotonic_gettime(&stTime2);
            //DBG_INFO("[MHAL_VPE] _ScalingByGe cost: %d us.\n", (stTime2.tv_sec * 1000 * 1000 + (stTime2.tv_nsec / 1000)) - (stTime1.tv_sec * 1000 * 1000 + (stTime1.tv_nsec / 1000)));
        }
    }
    else
    {
        DBG_ERR("Skip not input buffer case.\n");
    }
#endif

    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeIspDbgLevel(void *p)
{
    MS_BOOL bRet = FALSE;
    if (NULL == p)
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }


    // test no
    DBG_INFO("sucess p: %p.\n", p);

    bRet = TRUE;
    return bRet;
}


MS_BOOL MHalVpeIspRotationConfig(void *pCtx, const MHalVpeIspRotationConfig_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIsp_t *pIsp = (MHalVpeIsp_t *)pCtx;

    if ((NULL == pIsp) || (NULL == pCfg))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pIsp);
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeIspInputConfig(void *pCtx, const MHalVpeIspInputConfig_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeIsp_t *pIsp = (MHalVpeIsp_t *)pCtx;

    if ((NULL == pIsp) || (NULL == pCfg))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pIsp);
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}

// SCL
MS_BOOL MHalVpeCreateSclInstance(const MHalAllocPhyMem_t *pstAlloc, const MHalVpeSclWinSize_t *pstMaxWin, void **pCtx)
{
    MS_BOOL bRet = FALSE;
    MHalVpeScl_t *pScl = NULL;
    VpeManager  *pMgr= &_gSclMgr;

    if ((NULL == pstAlloc) || (NULL == pstMaxWin) || (pCtx == NULL))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    if (VPE_Manager_CheckEmpty(pMgr) == TRUE)
    {
        VPE_Manager_Init(pMgr);
    }
    else if (VPE_Manager_CheckFull(pMgr) == TRUE)
    {
        DBG_ERR("channel is full !!!\n");
        return bRet;
    }

    pScl = kmalloc(sizeof(*pScl), GFP_KERNEL);
    if (pScl != NULL)
    {
        VPE_Manager_AddMember(pMgr, pScl);
        *pCtx = pScl;
        DBG_INFO("sucess pCtx: %p Chnn: %d.\n", *pCtx, pScl->u32ChnnId);
    }
    else
    {
        DBG_ERR("Fail to alloc Ctx.\n");
    }

    // test
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeDestroySclInstance(void *pCtx)
{
    MS_BOOL bRet = FALSE;
    MHalVpeScl_t *pScl = (MHalVpeScl_t *)pCtx;
    VpeManager  *pMgr= &_gSclMgr;

    if (NULL == pCtx)
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    DBG_INFO("sucess pCtx: %p Chnn: %d.\n", pCtx, pScl->u32ChnnId);
    VPE_Manager_DelMember(pMgr, pScl);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHalVpeSclProcess(void *pCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeSclOutputBufferConfig_t *pBuffer)
{
    MS_BOOL bRet = FALSE;

    if ((NULL == pCtx) || (NULL == pBuffer))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
#ifdef VPE_FAKE_TEST_USE_GE
{
    MHalVpeScl_t *pScl = (MHalVpeScl_t *)pCtx;
    memcpy(&_gSclVideo, pBuffer, sizeof(*pBuffer));
    _gpVpeScl = pScl;
}
#endif

    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeSclDbgLevel(void *p)
{
    MS_BOOL bRet = FALSE;

    if (NULL == p)
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }


    // test no
    DBG_INFO("sucess p: %p.\n", p);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeSclCropConfig(void *pCtx, const MHalVpeSclCropConfig_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeScl_t *pScl = (MHalVpeScl_t *)pCtx;

    if ((NULL == pCtx) || (NULL == pCfg))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pScl);
    memcpy(&pScl->stCrop, pCfg, sizeof(*pCfg));
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeSclOutputDmaConfig(void *pCtx, const MHalVpeSclOutputDmaConfig_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeScl_t *pScl = (MHalVpeScl_t *)pCtx;

    if ((NULL == pScl) || (NULL == pCfg))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pScl);
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeSclInputConfig(void *pCtx, const MHalVpeSclInputSizeConfig_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeScl_t *pScl = (MHalVpeScl_t *)pCtx;

    if ((NULL == pScl) || (NULL == pCfg))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pScl);
    DBG_INFO("sucess pCtx: %p.\n", pCtx);
    memcpy(&pScl->stSrc, pCfg, sizeof(*pCfg));

    DBG_INFO("Chnn %u src_config: (%d %d) (%d %d).\n", pScl->u32ChnnId, pScl->stSrc.u16Width, pScl->stSrc.u16Height,
        pCfg->u16Width, pCfg->u16Height);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeSclOutputSizeConfig(void *pCtx, const MHalVpeSclOutputSizeConfig_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeScl_t *pScl = (MHalVpeScl_t *)pCtx;
    if ((NULL == pCtx) || (NULL == pCfg))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pScl);
    memcpy(&pScl->astDispWin[pCfg->enOutPort], pCfg, sizeof(*pCfg));
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeSclOutputMDWinConfig(void *pCtx, const MHalVpeSclOutputMDwinConfig_t *pCfg)
{
    MS_BOOL bRet = FALSE;
    MHalVpeScl_t *pScl = (MHalVpeScl_t *)pCtx;

    if ((NULL == pScl) || (NULL == pCfg))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    testCtx(pScl);
    DBG_INFO("sucess pCtx: %p.\n", pCtx);

    bRet = TRUE;
    return bRet;
}

// SCL vpe irq.
MS_BOOL MHalVpeSclGetIrqNum(unsigned int *pIrqNum)
{
    MS_BOOL bRet = FALSE;

    if (NULL == pIrqNum)
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }
    // test
    *pIrqNum = VPE_IRQ_NUM;

    DBG_INFO("sucess pIrqNum: %d.\n", *pIrqNum);

    bRet = TRUE;
    return bRet;
}
// RIU write register
MS_BOOL MHalVpeSclEnableIrq(MS_BOOL bOn)
{
    MS_BOOL bRet = FALSE;

    DBG_INFO("sucess bOn: %d.\n", bOn);
    // test
    // TODO IRQ ON/OFF
    bRet = TRUE;
    return bRet;
}
// RIU write register
MS_BOOL MHalVpeSclClearIrq(void)
{
    MS_BOOL bRet = FALSE;

    DBG_INFO("sucess.\n");

    bRet = TRUE;
    return bRet;
}
MS_BOOL MHalVpeSclCheckIrq(void)
{
    MS_BOOL bRet = FALSE;
    // test
    DBG_INFO("sucess.\n");

    bRet = TRUE;
    return bRet;
}

// SCL sw trigger irq by CMDQ or Sc
MS_BOOL MHalVpeSclSetSwTriggerIrq(MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    MS_BOOL bRet = FALSE;

    if (NULL == pstCmdQInfo)
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    // TODO: Run cmdQ
    DBG_INFO("sucess pstCmdQInfo: %p.\n", pstCmdQInfo);

    bRet = TRUE;
    return bRet;
}
// SCL in irq bottom: Read 3DNR register
MS_BOOL MHalVpeIqRead3DNRTbl(void *pIqCtx)
{
    MS_BOOL bRet = FALSE;

    if (NULL == pIqCtx)
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }
    // TODO Read 3DNR table
    // test

    DBG_INFO("sucess pIqCtx: %p. read 3D NR Table\n", pIqCtx);

    bRet = TRUE;
    return bRet;
}
// SCL polling MdwinDone
MS_BOOL MHalVpeSclSetWaitMdwinDone(void *pSclCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    MS_BOOL bRet = FALSE;

    if ((NULL == pSclCtx) || (pstCmdQInfo == NULL))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test

    DBG_INFO("sucess pSclCtx: %p.\n", pSclCtx);

    bRet = TRUE;
    return bRet;
}

MS_BOOL MHalVpeInit(const MHalAllocPhyMem_t *pstAlloc ,MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    MS_BOOL bRet = FALSE;

    if ((NULL == pstAlloc) || (pstCmdQInfo == NULL))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test

    DBG_INFO("sucess.\n");

    bRet = TRUE;
    return bRet;
}

MS_BOOL MHalVpeDeInit(void)
{
    DBG_INFO("sucess.\n");
    return TRUE;
}

MS_BOOL MHalVpeSclSetWaitDone(void *pSclCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, MHalVpeWaitDoneType_e enWait)
{
    MS_BOOL bRet = FALSE;

    if ((NULL == pSclCtx) || (pstCmdQInfo == NULL))
    {
        DBG_ERR("pointer Failed!!!\n");
        return bRet;
    }

    // test
    DBG_INFO("MHalVpeSclSetWaitDone wait %d", enWait);

    DBG_INFO("sucess.\n");

    bRet = TRUE;
    return bRet;
}

EXPORT_SYMBOL(MHalVpeIqSetWdrRoiMask);
EXPORT_SYMBOL(MHalVpeSclInputConfig);
EXPORT_SYMBOL(MHalVpeCreateIspInstance);
EXPORT_SYMBOL(MHalVpeSclCheckIrq);
EXPORT_SYMBOL(MHalVpeSclGetIrqNum);
EXPORT_SYMBOL(MHalVpeSclClearIrq);
EXPORT_SYMBOL(MHalVpeSclSetWaitMdwinDone);
EXPORT_SYMBOL(MHalVpeIspRotationConfig);
EXPORT_SYMBOL(MHalVpeIspInputConfig);
EXPORT_SYMBOL(MHalVpeIqGetWdrRoiHist);
EXPORT_SYMBOL(MHalVpeCreateSclInstance);
EXPORT_SYMBOL(MHalVpeSclEnableIrq);
EXPORT_SYMBOL(MHalVpeIqSetWdrRoiHist);
EXPORT_SYMBOL(MHalVpeIqSetDnrTblMask);
EXPORT_SYMBOL(MHalVpeSclOutputDmaConfig);
EXPORT_SYMBOL(MHalVpeSclCropConfig);
EXPORT_SYMBOL(MHalVpeIqRead3DNRTbl);
EXPORT_SYMBOL(MHalVpeSclProcess);
EXPORT_SYMBOL(MHalVpeIqConfig);
EXPORT_SYMBOL(MHalVpeIqProcess);
EXPORT_SYMBOL(MHalVpeSclOutputSizeConfig);
EXPORT_SYMBOL(MHalVpeDestroyIqInstance);
EXPORT_SYMBOL(MHalVpeIspProcess);
EXPORT_SYMBOL(MHalVpeCreateIqInstance);
EXPORT_SYMBOL(MHalVpeDestroyIspInstance);
EXPORT_SYMBOL(MHalVpeDestroySclInstance);
EXPORT_SYMBOL(MHalVpeIqOnOff);
EXPORT_SYMBOL(MHalVpeInit);
EXPORT_SYMBOL(MHalVpeDeInit);
EXPORT_SYMBOL(MHalVpeSclSetWaitDone);
