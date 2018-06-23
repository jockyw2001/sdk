#if !defined(MSOS_TYPE_LINUX_KERNEL)
#include "stdio.h"
#include <string.h>
#else
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <../../mstar2/drv/mma_heap/mdrv_mma_heap.h>
#endif

#include "MsCommon.h"
#include "MsOS.h"
#include "drvSYS.h"
#include "apiXC.h"

#include "apiXC_DWIN.h"
#include "apiPNL.h"

//#include "cmdq_service.h"
#include "mhal_cmdq.h"
#include "mhal_divp_datatype.h"
#include "mhal_divp.h"
#include "mhal_common_cmdq.h"

//Dip bus alignment value for base or width
#define DIP_BUS_ALIGNMENT_FACTOR  32
#define MAX_CHANNEL_NUM 33//32+1
#define DI_BUFFER_CNT 4
#define DIP_IRQ_NUM (349)
// complie flag
#define DIP_CMDQ 0//mhal dip not use cmdq default
#define DIP_SUPPORT_3DDI 1

EXPORT_SYMBOL(MHAL_DIVP_Init);
EXPORT_SYMBOL(MHAL_DIVP_DeInit);
EXPORT_SYMBOL(MHAL_DIVP_CreateInstance);
EXPORT_SYMBOL(MHAL_DIVP_DestroyInstance);
EXPORT_SYMBOL(MHAL_DIVP_ProcessDramData);
EXPORT_SYMBOL(MHAL_DIVP_GetIrqNum);
EXPORT_SYMBOL(MHAL_DIVP_EnableFrameDoneIsr);
EXPORT_SYMBOL(MHAL_DIVP_CleanFrameDoneIsr);
EXPORT_SYMBOL(MHAL_DIVP_SetAttr);
EXPORT_SYMBOL(MHAL_DIVP_CaptureTiming);
EXPORT_SYMBOL(MHAL_DIVP_GetChipCapbility);
EXPORT_SYMBOL(MHAL_DIVP_WaitCmdQFrameDone);

static u32 g_u32LogLevel = BIT(0);//01; //0xFF;//
#define LOG_TAG "[divp]"
#define LOG printk
#define FUN_ENTRY() {if(g_u32LogLevel & BIT(4)) LOG("%s:%d"LOG_TAG" entry >>>>>.\n", __FUNCTION__, __LINE__);}
#define FUN_EXIT() {if(g_u32LogLevel & BIT(4)) LOG("%s:%d"LOG_TAG" exit  <<<<<.\n", __FUNCTION__, __LINE__);}
#define LOG_DBG(x,...) {if(g_u32LogLevel & BIT(3)) LOG("%s:%d"LOG_TAG""x"\n", __FUNCTION__, __LINE__,  ##__VA_ARGS__);}
#define LOG_WARN(x,...) {if(g_u32LogLevel & BIT(2)) LOG("%s:%d"LOG_TAG" "x"",__FUNCTION__, __LINE__,  ##__VA_ARGS__);}
#define LOG_INFO(x,...) {if(g_u32LogLevel & BIT(1)) LOG("%s:%d"LOG_TAG" "x"",__FUNCTION__, __LINE__,  ##__VA_ARGS__);}
#define LOG_ERR(x,...) do{if(g_u32LogLevel & BIT(0)) LOG("%s:%d"LOG_TAG" error:"x"",__FUNCTION__, __LINE__,  ##__VA_ARGS__);}while(0)

#define LOG_INFO_DIPR(x, ...) {if(g_u32LogLevel & BIT(5)) LOG(x, ##__VA_ARGS__);}
#define CHECK_APIXC_RET(x) \
    if(E_APIXC_RET_OK != x) {\
        LOG_ERR(" "#x"return error.\n");\
        return FALSE;\
    }
#define CHECK_BOOL_RET(x) \
    if(TRUE != x) {\
        LOG_ERR(""#x"return error.\n");\
        return FALSE;\
    }
#define CHECK_UTOPIA_RETURN(x) do{}while(0)
#define CHECK_ASSERT(x)\
    do{\
        if (!(x)) {\
            LOG_ERR("["#x"] is assert fail.\n");\
        }\
    }while(0)

typedef struct _DivpChannelInfo DivpChannelInfo;

typedef struct
{
    u16 u16StructSize;
    MS_U8 u8ChannelId;
    SCALER_DIP_WIN enDipWin;
    bool bIsInit;
    bool bUSeCmdq;
    bool bDipPollMode;
    bool bNeedScalingUp;
    bool bIsDiEnable;
    EN_XC_DIP_DATA_FMT eOutputFmt;
    MHAL_DIVP_DiType_e eDiType;
    ST_XC_DIP_CHIPCAPS stChipCaps;

    //di buffer info


    ST_DIP_DI_SETTING st3DDISetting;
    ST_XC_DIPR_PROPERTY_EX stDiprInfo;
    XC_SETWIN_INFO stDipWindowIfo;
    ST_XC_DIP_WINPROPERTY stDipwInfo;

    PfnAlloc pfAlloc;
    PfnFree pfFree;
    MHAL_CMDQ_CmdqInterface_t *pCmdqOps;
    DivpChannelInfo *pstChnInfo;
} DivpPrivateData;

struct _DivpChannelInfo
{
    MS_U16 u16StructSize;
    MS_U8 u8ChannelId;
    MHAL_DIVP_DeviceId_e eDevId;
    MS_U16 u16MaxWidth;
    MS_U16 u16MaxHeight;

    MS_U32 u32DIStartAddr;// = RES_DMSDI.u32DIPDIAddress_pa;
    MS_U32 u32DIEndAddr;//= RES_DMSDI.u32DIPDIAddress_pa + u32DIPDISize;
    MS_U32 u32DISize;// = u32DIPDISize;
    MS_U8 u8DIBufCnt;// = 4; // 4 DI temp buffer

    //attrbuite parameters
    MHAL_DIVP_DiType_e eDiType;
    MHAL_DIVP_Rotate_e eRotate;
    MHAL_DIVP_Window_t stCropWin;
    MHAL_DIVP_Mirror_t stMirror;
    MHAL_DIVP_TnrLevel_e eTnrLvl;
    void* pCtx;
};
static ST_DIP_DI_NWAY_BUF_INFO stDINwayBuf[MAX_CHANNEL_NUM];

static DivpPrivateData g_stDipRes[MAX_DIP_WINDOW] = {{0}, {0},};
static bool g_bIsInit[MAX_DIP_WINDOW] ={FALSE, FALSE, FALSE};
static DivpChannelInfo g_astDivpChnInfo[MAX_CHANNEL_NUM] = {{0},};

//mma variable
extern struct MMA_BootArgs_Config mma_config[MAX_MMA_AREAS];
extern int mstar_driver_boot_mma_buffer_num ;

#if 0
static InterruptCb pAPI_DipIntCb;
static MS_BOOL bAPI_DipIsr = FALSE;
static void _DIP_CallBack(InterruptNum eIntNum)
{
    if (MApi_XC_DIP_GetIntStatus(DIP_WINDOW) > 0)
    {
        LOG_INFO("dip interrupt, 1get a irq signal.\n");
        // clear int status
        MApi_XC_DIP_ClearInt(0xFFFF, DIP_WINDOW);
    } else {
        LOG_INFO("dip interrupt, 1get a irq signal, but not ready.\n");
    }

    MsOS_EnableInterrupt(E_INT_IRQ_DIPW);
}
#endif
EN_XC_DIP_TILE_BLOCK _DipTileModeTransform(MHAL_DIVP_TileMode_e eHalTile)
{
    EN_XC_DIP_TILE_BLOCK eDipTile = DIP_TILE_BLOCK_R_NONE;
    switch (eHalTile) {
        case E_MHAL_DIVP_TILE_MODE_NONE:
            eDipTile = DIP_TILE_BLOCK_R_NONE;
            break;
        case E_MHAL_DIVP_TILE_MODE_16x16:
            eDipTile = DIP_TILE_BLOCK_R_NONE;
            break;
        case E_MHAL_DIVP_TILE_MODE_16x32:
            eDipTile = DIP_TILE_BLOCK_R_16_32;
            break;
        case E_MHAL_DIVP_TILE_MODE_32x16:
            eDipTile = DIP_TILE_BLOCK_R_32_16;
            break;
        case E_MHAL_DIVP_TILE_MODE_32x32:
            eDipTile = DIP_TILE_BLOCK_R_32_32;
            break;
        default:
            eDipTile = DIP_TILE_BLOCK_R_NONE;
            break;
    }
    return eDipTile;
}
EN_MFDEC_TILE_MODE _DipMFDecTileModeTransform(MHAL_DIVP_TileMode_e eHalTile)
{
    EN_MFDEC_TILE_MODE eDipTile = E_DIP_MFEDC_TILE_16_32;
    switch (eHalTile) {
        case E_MHAL_DIVP_TILE_MODE_16x32:
            eDipTile = E_DIP_MFEDC_TILE_16_32;
            break;
        case E_MHAL_DIVP_TILE_MODE_32x16:
            eDipTile = E_DIP_MFEDC_TILE_32_16;
            break;
        case E_MHAL_DIVP_TILE_MODE_32x32:
            eDipTile = E_DIP_MFEDC_TILE_32_32;
            break;
        default:
            eDipTile = E_DIP_MFEDC_TILE_16_32;
            break;
    }
    return eDipTile;
}

EN_XC_DIP_DATA_FMT _DipDataFormatTransform(MHAL_DIVP_PixelFormat_e eHalFmt)
{
    EN_XC_DIP_DATA_FMT eDipFmt = DIP_DATA_FMT_YUV422;
    switch (eHalFmt) {
        case E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV:
            eDipFmt = DIP_DATA_FMT_YUV422;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_ARGB8888:
            eDipFmt = DIP_DATA_FMT_ARGB8888;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_ABGR8888:
            eDipFmt = DIP_DATA_FMT_ARGB8888;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264:
            eDipFmt = DIP_DATA_FMT_YUV420;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265:
            eDipFmt = DIP_DATA_FMT_YUV420_H265;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265:
            eDipFmt = DIP_DATA_FMT_YUV420_H265_10BITS;
            break;
        default:
            eDipFmt = DIP_DATA_FMT_YUV422;
            break;
    }
    return eDipFmt;
}

u16 _DipGetSizeOf2Pixel(MHAL_DIVP_PixelFormat_e eHalFmt)
{
    u16 u16BytesOf2Pixel = 4;
    switch (eHalFmt) {
        case E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV:
            u16BytesOf2Pixel = 4;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_ARGB8888:
            u16BytesOf2Pixel = 8;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_ABGR8888:
            u16BytesOf2Pixel = 8;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264:
            u16BytesOf2Pixel = 2;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265:
            u16BytesOf2Pixel = 2;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265:
            u16BytesOf2Pixel = 2;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
            u16BytesOf2Pixel = 2;
            break;
        default:
            u16BytesOf2Pixel = 4;
            break;

    }
    return u16BytesOf2Pixel;
}

EN_DIP_DI_FIELD _DipGetDiFieldType(MHAL_DIVP_DiType_e eDiType, MHAL_DIVP_ScanMode_e eScanType, MHAL_DIVP_FieldType_e eFieldType)
{
    EN_DIP_DI_FIELD eDIField = E_DIP_3DDI_TOP_BOTH_FIELD_STAGGER;
    MS_BOOL rt = TRUE;
    if(eDiType == E_MHAL_DIVP_DI_TYPE_3D)
    {
        if(eScanType == E_MHAL_DIVP_SCAN_MODE_INTERLACE_FRAME)
        {
            if(eFieldType == E_MHAL_DIVP_FIELD_TYPE_TOP)
            {LOG_DBG("E_MHAL_DIVP_DI_TYPE_3DDI>>>E_DIP_3DDI_TOP_BOTH_FIELD_STAGGER.\n");
                eDIField = E_DIP_3DDI_TOP_BOTH_FIELD_STAGGER;
            }
            else if(eFieldType == E_MHAL_DIVP_FIELD_TYPE_BOTTOM)
            {LOG_DBG("E_MHAL_DIVP_DI_TYPE_3DDI>>>E_DIP_3DDI_BOTTOM_BOTH_FIELD_STAGGER.\n");
                eDIField = E_DIP_3DDI_BOTTOM_BOTH_FIELD_STAGGER;
            }
            else
            {
                rt = FALSE;
            }
        }
        else if(eScanType == E_MHAL_DIVP_SCAN_MODE_INTERLACE_FIELD)
        {
            if(eFieldType == E_MHAL_DIVP_FIELD_TYPE_TOP)
            {
                eDIField = E_DIP_3DDI_TOP_SINGLE_FIELD;
            }
            else if(eFieldType == E_MHAL_DIVP_FIELD_TYPE_BOTTOM)
            {
                eDIField = E_DIP_3DDI_BOTTOM_SINGLE_FIELD;
            }
            else
            {
                rt = FALSE;
            }

        }
        //return eDIField;
    }
    else if(eDiType == E_MHAL_DIVP_DI_TYPE_2D)
    {
        if(eScanType == E_MHAL_DIVP_SCAN_MODE_INTERLACE_FRAME)
        {
            if(eFieldType == E_MHAL_DIVP_FIELD_TYPE_TOP)
            {
                eDIField = E_DIP_MED_DI_TOP_BOTH_FIELD_STAGGER;
            }
            else if(eFieldType == E_MHAL_DIVP_FIELD_TYPE_BOTTOM)
            {
                eDIField = E_DIP_MED_DI_BOTTOM_BOTH_FIELD_STAGGER;
            }
            else
            {
                rt = FALSE;
            }

        }
        else if(eScanType == E_MHAL_DIVP_SCAN_MODE_INTERLACE_FIELD)
        {
            if(eFieldType == E_MHAL_DIVP_FIELD_TYPE_TOP)
            {
                eDIField = E_DIP_MED_DI_TOP_BOTH_FIELD_SEPARATE;
            }
            else if(eFieldType == E_MHAL_DIVP_FIELD_TYPE_BOTTOM)
            {
                eDIField = E_DIP_MED_DI_BOTTOM_BOTH_FIELD_SEPARATE;
            }
            else
            {
                rt = FALSE;
            }

        }
        //return eDIField;
    }
    else
    {
        rt = FALSE;
    }

    if (rt == FALSE)
    {
        LOG_ERR("not support:eDiType=%d,eScanType=%d,eFieldType=%d.\n", eDiType, eScanType, eFieldType);
    }
    return eDIField;
}
bool _DipPrintLog(MHAL_DIVP_DeviceId_e eDevId)
{
    SCALER_DIP_WIN enDipWin = (SCALER_DIP_WIN)eDevId;
    LOG_INFO("Timeout: SetDIPRProperty input Fmt=%d,w,h=%u,%u pitch=%u buf(%#llx,%#llx)\n",
        g_stDipRes[enDipWin].stDiprInfo.stDIPRProperty.enDataFmt,
        g_stDipRes[enDipWin].stDiprInfo.stDIPRProperty.u16Width,
        g_stDipRes[enDipWin].stDiprInfo.stDIPRProperty.u16Height,
        g_stDipRes[enDipWin].stDiprInfo.stDIPRProperty.u16Pitch,
        g_stDipRes[enDipWin].stDiprInfo.stDIPRProperty.u32YBufAddr,
        g_stDipRes[enDipWin].stDiprInfo.stDIPRProperty.u32CBufAddr);

    LOG_INFO("DIPSetWindow[%u] hvsp(%u), Fmt(%u), CapWin(%u, %u, %u, %u), Src(%u, %u), Dst(%u, %u)\n",
        g_stDipRes[enDipWin].enDipWin,
        g_stDipRes[enDipWin].bNeedScalingUp,
        g_stDipRes[enDipWin].eOutputFmt,//eDIPOutputFormat,
        g_stDipRes[enDipWin].stDipWindowIfo.stCapWin.x,
        g_stDipRes[enDipWin].stDipWindowIfo.stCapWin.y,
        g_stDipRes[enDipWin].stDipWindowIfo.stCapWin.width,
        g_stDipRes[enDipWin].stDipWindowIfo.stCapWin.height,
        g_stDipRes[enDipWin].stDipWindowIfo.u16PreHCusScalingSrc,
        g_stDipRes[enDipWin].stDipWindowIfo.u16PreVCusScalingSrc,
        g_stDipRes[enDipWin].stDipWindowIfo.u16PreHCusScalingDst,
        g_stDipRes[enDipWin].stDipWindowIfo.u16PreVCusScalingDst);

    LOG_INFO("DIPWinProperty Src=%u,w,h=%u,%u p=%u cnt=%u,buf(%#llx, %#llx)\n",
        g_stDipRes[enDipWin].stDipwInfo.enSource,
        g_stDipRes[enDipWin].stDipwInfo.u16Width,
        g_stDipRes[enDipWin].stDipwInfo.u16Height,
        g_stDipRes[enDipWin].stDipwInfo.u16Pitch,
        g_stDipRes[enDipWin].stDipwInfo.u8BufCnt,
        g_stDipRes[enDipWin].stDipwInfo.u32BufStart,
        g_stDipRes[enDipWin].stDipwInfo.u32BufEnd);

    if(g_stDipRes[enDipWin].stDiprInfo.stDIPR_MFDecInfo.bMFDec_Enable == TRUE)
    {
        ST_DIP_MFDEC_INFO *pstDiprDecInfo = &g_stDipRes[enDipWin].stDiprInfo.stDIPR_MFDecInfo;
        //ST_DIP_MFDEC_INFO &stDiprDecInfo __attribute__((__unused__)) = stDIPRPropertyEx[enDipWin].stDIPR_MFDecInfo;
        LOG_INFO("MFDec: version(%d,%d),enable=%d,select=%d,mirror=(%d:%d),tile=%d,uncompress=%d,bypass=%u,mode=%d\n"
            "Fb(%#llx,%#llx,%d) window(%d,%d,%d,%d) Bitlen(%#llx,%d) HTLB(%d,%d)(%#llx,%d,%d)(%p,%p)\n"
            ,
            pstDiprDecInfo->stMFDec_VerCtl.u32version,
            pstDiprDecInfo->stMFDec_VerCtl.u32size,
            pstDiprDecInfo->bMFDec_Enable,
            pstDiprDecInfo->u8MFDec_Select,
            pstDiprDecInfo->bHMirror,
            pstDiprDecInfo->bVMirror,
            pstDiprDecInfo->enMFDec_tile_mode,
            pstDiprDecInfo->bUncompress_mode,
            pstDiprDecInfo->bBypass_codec_mode,
            pstDiprDecInfo->en_MFDecVP9_mode,
            (MS_U64)pstDiprDecInfo->phyLuma_FB_Base,
            (MS_U64)pstDiprDecInfo->phyChroma_FB_Base,
            pstDiprDecInfo->u16FB_Pitch,
            pstDiprDecInfo->u16StartX,
            pstDiprDecInfo->u16StartY,
            pstDiprDecInfo->u16HSize,
            pstDiprDecInfo->u16VSize,
            (MS_U64)pstDiprDecInfo->phyBitlen_Base,
            pstDiprDecInfo->u16Bitlen_Pitch,
            pstDiprDecInfo->stMFDec_HTLB_Info.stMFDec_HTLB_VerCtl.u32version,
            pstDiprDecInfo->stMFDec_HTLB_Info.stMFDec_HTLB_VerCtl.u32size,
            pstDiprDecInfo->stMFDec_HTLB_Info.u32HTLBEntriesAddr,
            pstDiprDecInfo->stMFDec_HTLB_Info.u8HTLBEntriesSize,
            pstDiprDecInfo->stMFDec_HTLB_Info.u8HTLBTableId,
            pstDiprDecInfo->stMFDec_HTLB_Info.pHTLBInfo,
            pstDiprDecInfo->pMFDecInfo);
    }

    if(1) //3d di info
    {
        ST_DIP_DI_SETTING *pstDipr3DDiSetting = &g_stDipRes[enDipWin].st3DDISetting;
        LOG_INFO("3DDI: diEnable=%u, initDi=%u, initAddr=%#llx,initSize=%d,nIndex=%u,eField=%u,"
            "BotBuff(%#llx,%#llx,%#llx,%#llx),bEnBob=%u,bFresh=%u,BufCnt=%u.\n",
        pstDipr3DDiSetting->bEnableDI,
        pstDipr3DDiSetting->bInitDI,
        (MS_U64)pstDipr3DDiSetting->stInitInfo.phyDIInitAddr,
        pstDipr3DDiSetting->stInitInfo.u32DIInitSize,
        pstDipr3DDiSetting->u8NwayIndex,
        pstDipr3DDiSetting->enDI_Field,
        (MS_U64)pstDipr3DDiSetting->phyBotYBufAddr,
        (MS_U64)pstDipr3DDiSetting->phyBotCBufAddr,
        (MS_U64)pstDipr3DDiSetting->phyBotYBufAddr10Bits,
        (MS_U64)pstDipr3DDiSetting->phyBotCBufAddr10Bits,
        pstDipr3DDiSetting->bEnableBob,
        pstDipr3DDiSetting->bEnaNwayInfoRefresh,
        pstDipr3DDiSetting->u8NwayBufCnt);
    }

    return TRUE;
}


bool _DipAllocateMem(DivpChannelInfo *pstDivpChnInfo, MS_U64 *pAddr, MS_U32 AddrSize)
{
    PfnAlloc pfMemAlloc = NULL;
    CHECK_ASSERT(pstDivpChnInfo != NULL);
    //allocate memory
    pfMemAlloc = g_stDipRes[pstDivpChnInfo->eDevId].pfAlloc;//fixme  g_stDipRes[0]
    CHECK_ASSERT(pfMemAlloc != NULL);

    pfMemAlloc(mma_config[0].name, AddrSize, pAddr);

    return TRUE;
}

bool _DipFreeMem(DivpChannelInfo *pstDivpChnInfo, MS_U64 u64Addr)
{
    MS_U8 u8ChannelId = 0;
    PfnFree pfMemFree = NULL;
    CHECK_ASSERT(pstDivpChnInfo != NULL);
    u8ChannelId = pstDivpChnInfo->u8ChannelId;
    pfMemFree = g_stDipRes[pstDivpChnInfo->eDevId].pfFree;//fixme  g_stDipRes[0]
    CHECK_ASSERT(pfMemFree != NULL);

    pfMemFree((MS_U64)u64Addr);

    return TRUE;
}

MS_BOOL DipDiInit(MHAL_DIVP_DeviceId_e eDevId, DivpChannelInfo *pstDivpChnInfo, MS_BOOL bEnableDi)
{
    SCALER_DIP_WIN enDipWin = (SCALER_DIP_WIN)eDevId;
    MS_U8 u8ChannelId = pstDivpChnInfo->u8ChannelId;
    MS_U32 u32DipDiSize =  0;//1920 * 2 * 1080 * DI_BUFFER_CNT; //DI_BUFFER_CNT = 4 buffer
    ST_XC_DIPR_PROPERTY_EX DIPRWinPropertytoShow_EX;
    ST_DIP_DI_SETTING DIP3DDISetting;
    ST_DIP_DI_NWAY_BUF_INFO stDINwayBuf[MAX_CHANNEL_NUM];
    MS_U64 u64DipDiAddr = NULL;
    memset(&DIPRWinPropertytoShow_EX, 0, sizeof(ST_XC_DIPR_PROPERTY_EX));
    memset(&DIP3DDISetting, 0, sizeof(ST_DIP_DI_SETTING));
    LOG_DBG("DipDiInit, eDevId=%u, u8ChannelId=%u, bEnableDi=%u.\n", eDevId, u8ChannelId, bEnableDi);
    if(bEnableDi == FALSE)
    {
        DIP3DDISetting.bEnableDI = FALSE;
    }
    else
    {
        u32DipDiSize = pstDivpChnInfo->u16MaxWidth * 2 * pstDivpChnInfo->u16MaxHeight;//1920 * 2 * 1080 * DI_BUFFER_CNT; //DI_BUFFER_CNT = 4 buffer
        CHECK_BOOL_RET(_DipAllocateMem(pstDivpChnInfo, &u64DipDiAddr, u32DipDiSize));
        LOG_DBG("DipDiInit 3ddi u64DipDiAddr=%#llx.\n", u64DipDiAddr);

        //dip di init
        DIP3DDISetting.bEnableDI = bEnableDi;//TRUE;
        DIP3DDISetting.bInitDI = TRUE;// init di flag
        DIP3DDISetting.stInitInfo.phyDIInitAddr = (MS_PHY)(u64DipDiAddr); //only use one times, so use the same buferr with di temp buffer(stDINwayBuf)
        DIP3DDISetting.stInitInfo.u32DIInitSize = u32DipDiSize;
        DIP3DDISetting.u8NwayIndex = u8ChannelId;// channel id
        DIP3DDISetting.enDI_Field = E_DIP_3DDI_TOP_SINGLE_FIELD;//E_DIP_3DDI_TOP_BOTH_FIELD_STAGGER;
        DIP3DDISetting.bEnaNwayInfoRefresh = TRUE;
        DIP3DDISetting.u8NwayBufCnt = 1;
        DIP3DDISetting.bEnableBob = TRUE;
        stDINwayBuf[u8ChannelId].phyDI_BufAddr = (MS_PHY)u64DipDiAddr; //RES_DMSDI.u32DIPDIAddress_pa;
        stDINwayBuf[u8ChannelId].u32DI_BufSize = u32DipDiSize;
        stDINwayBuf[u8ChannelId].u8DIBufCnt = DI_BUFFER_CNT;
        DIP3DDISetting.stDINwayBufInfo = (void*)stDINwayBuf;
    }
    DIPRWinPropertytoShow_EX.stDIPR3DDISetting.stDipr3DDI_VerCtl.u32version=1;
    DIPRWinPropertytoShow_EX.stDIPR3DDISetting.pDIPR3DDISetting = (void*)&DIP3DDISetting;

    MApi_XC_DIP_SetDIPRProperty_EX(&DIPRWinPropertytoShow_EX, enDipWin);

    //DivpChannelInfo *pstDivpChnInfo = &g_astDivpChnInfo[u8ChannelId];
    pstDivpChnInfo->u32DIStartAddr = (MS_U32)u64DipDiAddr;
    pstDivpChnInfo->u32DIEndAddr= (MS_U32)(u64DipDiAddr + u32DipDiSize);
    pstDivpChnInfo->u32DISize = u32DipDiSize;
    pstDivpChnInfo->u8DIBufCnt = DI_BUFFER_CNT; // 4 DI temp buffer

    return TRUE;
}

// dip init, only run one times
bool DipInit(MHAL_DIVP_DeviceId_e eDevId)
{
    SCALER_DIP_WIN enDipWin = (SCALER_DIP_WIN)eDevId;
    XC_INITDATA stXC_InitData;
    ST_XC_DIP_CHIPCAPS stDivpCaps;
    if(g_bIsInit[enDipWin])
    {
        LOG_INFO("dip[%d] has already init.\n", enDipWin);
        return TRUE;
    }
    //do system init
    MsOS_Init();
    MsOS_MPool_Init();
    MDrv_SYS_GlobalInit();

    memset(&stXC_InitData, 0, sizeof(XC_INITDATA));
    MApi_XC_Init(&stXC_InitData, 0);

    //get dip resource.
    CHECK_UTOPIA_RETURN(MApi_XC_DIP_GetResource(enDipWin));

    //do dip init
    g_bIsInit[enDipWin] = TRUE;
    memset(&g_stDipRes[enDipWin], 0, sizeof(DivpPrivateData));
    memset(&stDivpCaps, 0, sizeof(stDivpCaps));
    //memset(g_u8ChannelId, 0, MAX_CHANNEL_NUM *sizeof(MS_U8));

    MApi_XC_DIP_InitByDIP(enDipWin);

    //dip init config
    MApi_XC_DIP_SetOutputDataFmt(DIP_DATA_FMT_ARGB8888, enDipWin);
    MApi_XC_DIP_SetAlpha(0xFF, enDipWin);
    MApi_XC_DIP_SetMirror(FALSE, FALSE, enDipWin);
    MApi_XC_DIP_FrameRateCtrl(TRUE, 1/*u16DIPFrcRatio*/, 1, enDipWin); // 1/Ratio of input

    // unmask interrupt, enable intrrupt
    CHECK_APIXC_RET(MApi_XC_DIP_EnaInt(0xFFFF, TRUE, enDipWin));

    // clear int status
    MApi_XC_DIP_ClearInt(0xFFFF, enDipWin);

    //
    //MsOS_AttachInterrupt(E_INT_IRQ_DIPW,_DIP_CallBack);
    //MsOS_EnableInterrupt(E_INT_IRQ_DIPW);

    //DipDiInit(0,0,TRUE);
    //enable dip
    //MApi_XC_DIP_Ena(MAPI_FALSE, enDipWin);
    #if DIP_CMDQ
    if(g_stDipRes[enDipWin].pCmdqOps == NULL)
    {
        MHAL_CMDQ_BufDescript_t stBuffwrInfo = {0};
        stBuffwrInfo.u32CmdqBufSize = 4096 *8;
        stBuffwrInfo.u32CmdqBufSizeAlign = 32;//256bit allgin
        stBuffwrInfo.u32MloadBufSize = 0;
        stBuffwrInfo.u16MloadBufSizeAlign = 1;
        g_stDipRes[enDipWin].pCmdqOps = NULL;//MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_DIVP, &stBuffwrInfo, FALSE);
        LOG_INFO("cmdq pCmdqOps=0x%p.\n", g_stDipRes[enDipWin].pCmdqOps);
    }
    #else
    g_stDipRes[enDipWin].pCmdqOps = NULL;
    g_stDipRes[enDipWin].bUSeCmdq = FALSE;
    #endif

    MApi_XC_GetChipCaps(E_XC_DIP_CHIP_CAPS, (MS_U32*)&stDivpCaps, sizeof(stDivpCaps));
    memcpy(&g_stDipRes[enDipWin].stChipCaps, &stDivpCaps, sizeof(stDivpCaps));

    return TRUE;
}

// dip deinit, only run one times
bool DipDeInit(MHAL_DIVP_DeviceId_e eDevId)
{
    SCALER_DIP_WIN enDipWin = (SCALER_DIP_WIN)eDevId;
    FUN_ENTRY();
#if DIP_CMDQ
    if(g_stDipRes[enDipWin].pCmdqOps != NULL)
    {
        MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_DIVP);
        LOG_DBG("cmdq pCmdqOps=0x%p.\n", g_stDipRes[enDipWin].pCmdqOps);
        MApi_XC_DIP_CMDQ_SetOperations(g_stDipRes[enDipWin].pCmdqOps, FALSE, enDipWin);
    }
    g_stDipRes[enDipWin].bUSeCmdq = FALSE;
    g_stDipRes[enDipWin].pCmdqOps = NULL;
#else
    g_stDipRes[enDipWin].pCmdqOps = NULL;
    g_stDipRes[enDipWin].bUSeCmdq = FALSE;
#endif

    // mask interrupt, disable intrrupt
    //MApi_XC_DIP_EnaInt(0xFFFF, FALSE, enDipWin);
    // clear int status
    MApi_XC_DIP_ClearInt(0xFFFF, enDipWin);

    //MApi_XC_DIP_InterruptDetach(enDipWin);//de-attach before dip off
    //MApi_XC_DIP_Ena(MAPI_FALSE, enDipWin);

    //MsOS_DisableInterrupt(E_INT_IRQ_DIPW);
    //MsOS_DetachInterrupt(E_INT_IRQ_DIPW); // do not deattach, because deattach will take 100 ms to wait pthread join

    CHECK_UTOPIA_RETURN(MApi_XC_DIP_ReleaseResource(enDipWin));
    //clear private data
    g_bIsInit[enDipWin] = FALSE;
    memset(&g_stDipRes[enDipWin], 0, sizeof(DivpPrivateData));
    FUN_EXIT();
    return TRUE;
}



//-------------------------------------------------------------------------------------------------
/// SDK interface:Capture once from OP2/VOP
/// @param                 \b ST_CAPTURE_ONCE_INFO
/// @return                 \b  MAPI_BOOL
//-------------------------------------------------------------------------------------------------
//bool DipCapture (ST_CAPTURE_ONCE_INFO *pstCapOnceInfo)
bool DipCaptureTiming (MHAL_DIVP_CaptureInfo_t *pstCapOnceInfo, MHAL_CMDQ_CmdqInterface_t* pstCmdInf, DivpChannelInfo *pstDivpChnInfo)
{
    SCALER_DIP_WIN enDipWin = (SCALER_DIP_WIN)pstDivpChnInfo->eDevId;//MAX_DIP_WINDOW;
    const MHAL_DIVP_CaptureInfo_t *pstTempCapInfo = pstCapOnceInfo;
    XC_SETWIN_INFO stXC_SetWin_Info;
    ST_XC_DIP_WINPROPERTY stDIPWinProperty;
    //ST_XC_DIP_CHIPCAPS stDipChipCaps ={(SCALER_DIP_WIN)(0), 0};
    MS_PNL_DST_DispInfo dstDispInfo;
    EN_XC_DIP_DATA_FMT enTempFmt = DIP_DATA_FMT_MAX;
    u32 u32OPWidth = 0, u32OPHeight = 0;
    bool bMirrorH = FALSE;
    bool bMirrorV = FALSE;
    bool bSwapRGB = FALSE;
    EN_XC_DIP_OP_CAPTURE enOpCapture = E_XC_DIP_VOP2;
    MirrorMode_t enVideoMirror;
    #define DIP_RETRY_TIMES 15
    u16 u16IntStatus = 0;
    u16 u16Index = 1;

    XC_INITDATA XC_InitData;
    memset((void*)&XC_InitData, 0, sizeof(XC_INITDATA));
    if(pstCmdInf != NULL)
    {
        g_stDipRes[enDipWin].bUSeCmdq = TRUE;
        g_stDipRes[enDipWin].pCmdqOps = pstCmdInf;
        LOG_DBG("use cmdq--------------------.\n");
    }

    if(g_stDipRes[enDipWin].bUSeCmdq && g_stDipRes[enDipWin].pCmdqOps != NULL)
    {
        LOG_DBG("MApi_XC_DIP_CMDQ_SetOperations enable cmdq_srv.\n");
        MApi_XC_DIP_CMDQ_SetOperations(g_stDipRes[enDipWin].pCmdqOps, TRUE, enDipWin);
    }

    //ST_CAPTURE_ONCE_INFO stCaptureOnceInfo;
    //memset((void*)&stCaptureOnceInfo, 0, sizeof(ST_CAPTURE_ONCE_INFO));
    //memcpy(&stCaptureOnceInfo, pstCapOnceInfo, sizeof(ST_CAPTURE_ONCE_INFO));
    //enDipWin = (SCALER_DIP_WIN)(eDevId);
    MApi_XC_Init(&XC_InitData, 0);

    memset(&dstDispInfo, 0, sizeof(MS_PNL_DST_DispInfo));
    if(TRUE == MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo)))
    {
        u32OPWidth  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
        u32OPHeight = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;
    }
    else
    {
        LOG_ERR("Get OP Width/Height Failed!\n");
        return FALSE;
    }
    #if 0
    for (int i = 0; i < MAX_DIP_WINDOW ; i++) {
        enDipWin = (SCALER_DIP_WIN)i;
        if ((E_APIXC_RET_OK == MApi_XC_DIP_QueryResource(enDipWin)) && (E_APIXC_RET_OK == MApi_XC_DIP_GetResource(enDipWin)))
        {
            stDipChipCaps.eWindow = enDipWin;
            if (E_APIXC_RET_OK != MApi_XC_GetChipCaps(E_XC_DIP_CHIP_CAPS, (MS_U32 *)(&stDipChipCaps), sizeof(ST_XC_DIP_CHIPCAPS)))
            {
                DIPCDBGMSG("MApi_XC_GetChipCaps return fail for DIP[%d]\n", enDipWin);
            }

            if ((((u32OPWidth > stCaptureOnceInfo.u16OutputWidth) || (u32OPHeight > stCaptureOnceInfo.u16OutputHeight))
                && (MS_BOOL)(stDipChipCaps.u32DipChipCaps & DIP_CAP_SCALING_DOWN))
                || ((u32OPWidth == stCaptureOnceInfo.u16OutputWidth) && (u32OPHeight == stCaptureOnceInfo.u16OutputWidth)))
            {
                DIPCDBGMSG("get DIP[%u] support scaling down or no scaling!\n", enDipWin);
                break;
            }
            else
            {
                DIPCDBGMSG("DIP[%u] doesn't support this capture requirements!\n", enDipWin);
                MApi_XC_DIP_ReleaseResource(enDipWin);
            }
        }
        else
        {
            DIPCDBGMSG("Can't get DIP[%u] resource for capture once!\n", enDipWin);
        }
    }
    #endif
    if (enDipWin == MAX_DIP_WINDOW)
    {
        LOG_ERR("CaptureOnce can't get resource!!!\n");
        return FALSE;
    }

    if(E_APIXC_RET_OK != MApi_XC_DIP_InitByDIP(enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_InitByDIP Failed!!!\n");
        return FALSE;
    }


    switch (pstCapOnceInfo->eOutputPxlFmt) {
        case E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV:
            enTempFmt = DIP_DATA_FMT_YUV422;
            break;
        case E_MHAL_DIVP_PIXEL_FORMAT_ARGB8888:
            enTempFmt = DIP_DATA_FMT_ARGB8888;
            break;
        default :
            LOG_ERR("unknow format(%d).\n", pstCapOnceInfo->eOutputPxlFmt);
            enTempFmt = DIP_DATA_FMT_YUV422;
    }
    if (DIP_DATA_FMT_MAX == enTempFmt)
    {
        LOG_ERR("Unsupport Color Format!!!\n");
        return FALSE;
    }

    if(E_APIXC_RET_OK != MApi_XC_DIP_SetOutputDataFmt(enTempFmt, enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_SetOutputDataFmt Failed!!!\n");
        return FALSE;
    }

    if (DIP_DATA_FMT_ARGB8888 == enTempFmt)
    {
        if(E_APIXC_RET_OK != MApi_XC_DIP_SetAlpha(0xFF, enDipWin))
        {
            LOG_ERR("MApi_XC_DIP_SetAlpha Failed!!!\n");
            return FALSE;
        }
    }

    if(E_APIXC_RET_OK != MApi_XC_DIP_FrameRateCtrl(TRUE, 1, 1, enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_FrameRateCtrl Failed!!!\n");
        return FALSE;
    }

    enOpCapture =
        (pstTempCapInfo->eCapStage == E_MHAL_DIVP_CAP_STAGE_OUTPUT_WITH_OSD)? E_XC_DIP_OP2 : E_XC_DIP_VOP2;
    if(E_APIXC_RET_OK != MApi_XC_DIP_SetOutputCapture(TRUE, enOpCapture, enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_SetOutputCapture Failed!!!\n");
        return FALSE;
    }

    if (pstTempCapInfo->eInputPxlFmt == E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV)//(mapi_video::GetDisplayOPColorFormat() == E_OP_COLOR_FORMAT_YUV)
    {
        if ((DIP_DATA_FMT_YUV422 == enTempFmt)
            || (DIP_DATA_FMT_YUV420 == enTempFmt))
        {
            MApi_XC_DIP_EnableY2R(FALSE, enDipWin);
        }
        else
        {
            MApi_XC_DIP_EnableY2R(TRUE, enDipWin);
        }
        MApi_XC_DIP_EnableR2Y(FALSE, enDipWin);//OP is YUV, so disenable R2Y
    }
    else
    {
        if ((DIP_DATA_FMT_YUV422 == enTempFmt)
            || (DIP_DATA_FMT_YUV420 == enTempFmt))
        {
            MApi_XC_DIP_EnableR2Y(TRUE, enDipWin);
        }
        else
        {
            MApi_XC_DIP_EnableR2Y(FALSE, enDipWin);
        }
        MApi_XC_DIP_EnableY2R(FALSE, enDipWin);//OP is RGB, so disenable Y2R
    }

    if (bSwapRGB)
    {
        MApi_XC_DIP_SwapRGB(TRUE, E_XC_DIP_RGB_SWAPTO_BGR, enDipWin);
    }
    else
    {
        MApi_XC_DIP_SwapRGB(FALSE, E_XC_DIP_RGB_SWAPTO_BGR, enDipWin);
    }

    if (E_APIXC_RET_OK != MApi_XC_DIP_SetSourceScanType(DIP_SCAN_MODE_PROGRESSIVE, enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_SetSourceScanType Failed!!!\n");
        return FALSE;
    }

    if (E_APIXC_RET_OK != MApi_XC_DIP_EnaInterlaceWrite(FALSE, enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_EnaInterlaceWrite Failed!!!\n");
        return FALSE;
    }

    memset(&stXC_SetWin_Info, 0, sizeof(XC_SETWIN_INFO));
    stXC_SetWin_Info.enInputSourceType = INPUT_SOURCE_TV;
    stXC_SetWin_Info.stCapWin.x = 0;
    stXC_SetWin_Info.stCapWin.y = 0;
    stXC_SetWin_Info.stCapWin.width = u32OPWidth;
    stXC_SetWin_Info.stCapWin.height = u32OPHeight;
    stXC_SetWin_Info.bPreHCusScaling = TRUE;
    stXC_SetWin_Info.u16PreHCusScalingSrc = u32OPWidth;
    stXC_SetWin_Info.u16PreHCusScalingDst = pstTempCapInfo->u16Width;//stCaptureOnceInfo.u16OutputWidth;
    stXC_SetWin_Info.bPreVCusScaling = TRUE;
    stXC_SetWin_Info.u16PreVCusScalingSrc = u32OPHeight;
    stXC_SetWin_Info.u16PreVCusScalingDst = pstTempCapInfo->u16Height;//stCaptureOnceInfo.u16OutputHeight;
    stXC_SetWin_Info.bInterlace = FALSE;

    LOG_INFO("SetWindow---enInputSourceType=%u\n", stXC_SetWin_Info.enInputSourceType);
    LOG_INFO("SetWindow---SetDIPWin x,y,w,h=%u,%u,%u,%u\n", stXC_SetWin_Info.stCapWin.x, stXC_SetWin_Info.stCapWin.y,
                        stXC_SetWin_Info.stCapWin.width, stXC_SetWin_Info.stCapWin.height);
    LOG_INFO("SetWindow---stDispWin x,y,w,h=%u,%u,%u,%u\n",  stXC_SetWin_Info.stDispWin.x, stXC_SetWin_Info.stDispWin.y,
                        stXC_SetWin_Info.stDispWin.width, stXC_SetWin_Info.stDispWin.height);
    LOG_INFO("SetWindow---stCropWin x,y,w,h=%u,%u,%u,%u\n", stXC_SetWin_Info.stCropWin.x, stXC_SetWin_Info.stCropWin.y,
                        stXC_SetWin_Info.stCropWin.width, stXC_SetWin_Info.stCropWin.height);
    LOG_INFO("SetWindow---bInterlace, bHDuplicate, u16InputVFreq,u16InputVTotal, u16DefaultHtotal, u8DefaultPhase=%u,%u,%u,%u,%u,%u\n", stXC_SetWin_Info.bInterlace, stXC_SetWin_Info.bHDuplicate,
                        stXC_SetWin_Info.u16InputVFreq, stXC_SetWin_Info.u16InputVTotal, stXC_SetWin_Info.u16DefaultHtotal, stXC_SetWin_Info.u8DefaultPhase);
    LOG_INFO("SetWindow---bHCusScaling[%u] src,dst =%u,%u\n",  stXC_SetWin_Info.bHCusScaling, stXC_SetWin_Info.u16HCusScalingSrc, stXC_SetWin_Info.u16HCusScalingDst);
    LOG_INFO("SetWindow---bVCusScaling[%u] src,dst =%u,%u\n",  stXC_SetWin_Info.bVCusScaling, stXC_SetWin_Info.u16VCusScalingSrc, stXC_SetWin_Info.u16VCusScalingDst);
    LOG_INFO("SetWindow---bDisplayNineLattice=%u\n", stXC_SetWin_Info.bDisplayNineLattice);
    LOG_INFO("SetWindow---PreScalingH[%u] src,dst =%u,%u\n", stXC_SetWin_Info.bPreHCusScaling, stXC_SetWin_Info.u16PreHCusScalingSrc, stXC_SetWin_Info.u16PreHCusScalingDst);
    LOG_INFO("SetWindow---PreScalingV[%u] src,dst =%u,%u\n",stXC_SetWin_Info.bPreVCusScaling, stXC_SetWin_Info.u16PreVCusScalingSrc, stXC_SetWin_Info.u16PreVCusScalingDst);
    LOG_INFO("SetWindow---u16DefaultPhase=%u\n", stXC_SetWin_Info.u16DefaultPhase);

    if (E_APIXC_RET_OK != MApi_XC_DIP_SetWindow(&stXC_SetWin_Info, sizeof(XC_SETWIN_INFO), enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_SetWindow Failed!!!\n");
        return FALSE;
    }

    enVideoMirror = MApi_XC_GetMirrorModeTypeEx(MAIN_WINDOW);
    if (MIRROR_MAX == enVideoMirror)
    {
        LOG_ERR("MApi_XC_GetMirrorModeTypeExFailed!\n");
        return FALSE;
    }

    switch (enVideoMirror)
    {
        case MIRROR_H_ONLY:
            bMirrorH = TRUE;
            bMirrorV = FALSE;
            break;
        case MIRROR_V_ONLY:
            bMirrorH = FALSE;
            bMirrorV = TRUE;
            break;
        case MIRROR_HV:
            bMirrorH = TRUE;
            bMirrorV = TRUE;
            break;
        default:
            bMirrorH = FALSE;
            bMirrorV = FALSE;
            break;
    }

    if ((TRUE == bMirrorH) || (TRUE == bMirrorH))
    {
        if (E_APIXC_RET_FAIL == MApi_XC_DIP_SetMirror(bMirrorH, bMirrorV, enDipWin))
        {
            LOG_ERR("MApi_XC_DIP_SetMirror Failed!!!\n");
            return FALSE;
        }
    }

    memset(&stDIPWinProperty,0,sizeof(ST_XC_DIP_WINPROPERTY));
    stDIPWinProperty.u8BufCnt = 1;
    stDIPWinProperty.u16Width = pstTempCapInfo->u16Width;//stCaptureOnceInfo.u16OutputWidth;
    stDIPWinProperty.u16Height = pstTempCapInfo->u16Height;//stCaptureOnceInfo.u16OutputHeight;
    stDIPWinProperty.u32BufStart = (MS_U32)pstTempCapInfo->u64BufAddr[0];//stCaptureOnceInfo.u32BufPhyAddress;
    stDIPWinProperty.u32BufEnd = (MS_U32)(pstTempCapInfo->u64BufAddr[0] + pstTempCapInfo->u32BufSize);//stCaptureOnceInfo.u32BufPhyAddress + stCaptureOnceInfo.u32MemorySize;
    if (0 == pstTempCapInfo->u16Stride[0])
    {
        stDIPWinProperty.u16Pitch = stDIPWinProperty.u16Width * _DipGetSizeOf2Pixel(pstTempCapInfo->eOutputPxlFmt)/2;
    }
    else
    {
        stDIPWinProperty.u16Pitch = pstTempCapInfo->u16Stride[0];
    }
    stDIPWinProperty.enSource = SCALER_DIP_SOURCE_TYPE_OP_CAPTURE;

    LOG_INFO("SetDIPWinProperty---output w,h=%u,%u, u32BufStart, u32BufEnd = 0x%x,0x%x, enSource= %u\n",
        stDIPWinProperty.u16Width,  stDIPWinProperty.u16Height, (u32)stDIPWinProperty.u32BufStart,
        (u32)stDIPWinProperty.u32BufEnd, stDIPWinProperty.enSource);

    if(E_APIXC_RET_OK != MApi_XC_DIP_SetDIPWinProperty(&stDIPWinProperty, enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_SetDIPWinProperty Failed!!!\n");
        return FALSE;
    }

    if (E_APIXC_RET_OK != MApi_XC_DIP_ClearInt(BIT(0), enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_ClearInt fail!!");
        return FALSE;
    }

    if (E_APIXC_RET_OK != MApi_XC_DIP_EnaInt((MS_U16)0xFF, TRUE, enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_EnaInt fail!!");
        return FALSE;
    }

    if(E_APIXC_RET_OK != MApi_XC_DIP_CapOneFrameFast(enDipWin))
    {
        LOG_ERR("MApi_XC_DIP_CapOneFrameFast fail!!");
        return FALSE;
    }
    LOG_INFO("CapOneFrameFast config end!!\n");

    if(g_stDipRes[enDipWin].bUSeCmdq && g_stDipRes[enDipWin].pCmdqOps != NULL)
    {
        LOG_DBG("MApi_XC_DIP_CMDQ_SetOperations disable cmdq_srv.\n");
        MApi_XC_DIP_CMDQ_SetOperations(g_stDipRes[enDipWin].pCmdqOps, FALSE, enDipWin);
    }
    else
    {
        // no cmdq
        while (u16IntStatus == 0 && u16Index < DIP_RETRY_TIMES)
        {
            mdelay(4);
            u16IntStatus = MApi_XC_DIP_GetIntStatus(enDipWin);
            LOG_INFO("[capture screen]: delay %d ms, CaptureDone u16IntStatus %d \n",8*u16Index,u16IntStatus);
            if (u16IntStatus != 0)
            {
                break;
            }
            u16Index ++;
        }

        if(DIP_RETRY_TIMES == u16Index)
        {
            LOG_ERR("Wait for RawData time out!!!");
            return FALSE;
        }
    }
    return TRUE;
}

bool DipWaitWritebufferDone(MHAL_DIVP_DeviceId_e eDevId)
{
    bool bRet = TRUE;
    SCALER_DIP_WIN enDipWin = (SCALER_DIP_WIN)eDevId;
    MS_U16 u16IntStatus = 0;
    MS_U16 u16RetryTimes = 0;
    while(1)
    {
        u16IntStatus = MApi_XC_DIP_GetIntStatus(enDipWin);
        if(u16IntStatus != 0)
        {
            //MAPI_PRINTF("utopia cap %lld.\n", systemTime(CLOCK_BOOTTIME));
            LOG_INFO("get a dip status success, u16IntStatus=%d.\n", u16IntStatus);
            bRet = TRUE;
            break;
        }
        else
        {
            udelay(100);
        }
        u16RetryTimes++ ;
        if(u16RetryTimes%5000 == 0)//wait 500ms
        {
            LOG_INFO("[DIPC] wait MApi_XC_DIP_GetIntStatus timeout.\n");
        }
        else if(u16RetryTimes > 5000 * 5)
        {
            //do not print 'timeout' all the time
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}

bool DipUpdateFrameBufferIndex(DivpChannelInfo *pstDivpCtx, MHAL_DIVP_InputInfo_t* pstDivpInputInfo, MHAL_DIVP_OutPutInfo_t* pstDivpOutputInfo, MS_U8 u8ChannelId, bool bLastFrame, MHAL_DIVP_DeviceId_e eDevId)
{
    SCALER_DIP_WIN enDipWin = (SCALER_DIP_WIN)(eDevId);//MAX_DIP_WINDOW;
    bool bIsNeedScalingUp = FALSE;
    bool bRet = TRUE;
    //static MS_U8 u8count = 0;
    FUN_ENTRY();
    if((pstDivpOutputInfo->u16OutputWidth > pstDivpInputInfo->u16InputWidth)
        || (pstDivpOutputInfo->u16OutputHeight> pstDivpInputInfo->u16InputHeight))
    {
        if((g_stDipRes[enDipWin].stChipCaps.u32DipChipCaps & DIP_CAP_SCALING_UP) == DIP_CAP_SCALING_UP)
        {
            bIsNeedScalingUp = TRUE;
        } else {
            LOG_ERR(" UpdateFrameBuffer parameters is error, not support hvsp.\n");

            LOG_ERR("DivpInputInfo: u64BufAddr[0](%lld) u64BufAddr[1](%lld) \
                    u64BufAddr[2](%lld) u16InputWidth(%d) u16InputHeight(%d) \
                    u16Stride[0](%d) u16Stride[1](%d) u16Stride[2](%d)\n",
            pstDivpInputInfo->u64BufAddr[0], pstDivpInputInfo->u64BufAddr[1],
            pstDivpInputInfo->u64BufAddr[2], pstDivpInputInfo->u16InputWidth,
            pstDivpInputInfo->u16InputHeight, pstDivpInputInfo->u16Stride[0],
            pstDivpInputInfo->u16Stride[1], pstDivpInputInfo->u16Stride[2]);

            LOG_ERR("DivpOutputInfo: u64BufAddr[0](%lld) u64BufAddr[1](%lld) \
                    u64BufAddr[2](%lld) u16OutputWidth(%d) u16OutputHeight(%d) \
                    u16Stride[0](%d) u16Stride[1](%d) u16Stride[2](%d)\n",
            pstDivpOutputInfo->u64BufAddr[0], pstDivpOutputInfo->u64BufAddr[1],
            pstDivpOutputInfo->u64BufAddr[2], pstDivpOutputInfo->u16OutputWidth,
            pstDivpOutputInfo->u16OutputHeight, pstDivpOutputInfo->u16Stride[0],
            pstDivpOutputInfo->u16Stride[1], pstDivpOutputInfo->u16Stride[2]);

            return FALSE;
        }
    }
    g_stDipRes[enDipWin].bNeedScalingUp = bIsNeedScalingUp;
    //set params
    EN_XC_DIP_DATA_FMT eDipOutputFormat = DIP_DATA_FMT_YUV422;
    ST_DIP_DI_SETTING *pstDipr3DDiSetting = &g_stDipRes[enDipWin].st3DDISetting;
    ST_XC_DIPR_PROPERTY *pstDiprInfo = &g_stDipRes[enDipWin].stDiprInfo.stDIPRProperty;
    XC_SETWIN_INFO *pstDipWindowInfo = &g_stDipRes[enDipWin].stDipWindowIfo;
    ST_XC_DIP_WINPROPERTY *pstDipwInfo = &g_stDipRes[enDipWin].stDipwInfo;

    EN_XC_DIP_TILE_BLOCK enTileMode = DIP_TILE_BLOCK_R_16_32;
    EN_MFDEC_TILE_MODE enMFDecTileMode = E_DIP_MFEDC_TILE_16_32;

    {
        //DIPR input win setting
        u16 dipr_u8BytePer2Pixel = _DipGetSizeOf2Pixel(pstDivpInputInfo->ePxlFmt);
        //u16 diprDefaultPitch = 0;
        u16 u16DiprAllignPixels = 0;
        pstDiprInfo->enDataFmt = _DipDataFormatTransform(pstDivpInputInfo->ePxlFmt);

        //diprDefaultPitch = ((pstDivpInputInfo->u16InputWidth) * dipr_u8BytePer2Pixel)/2;
        pstDiprInfo->u16Width = pstDivpInputInfo->u16InputWidth;
        pstDiprInfo->u16Height = pstDivpInputInfo->u16InputHeight;
        pstDiprInfo->u16Pitch = MAX((pstDivpInputInfo->u16InputWidth * dipr_u8BytePer2Pixel /2), pstDivpInputInfo->u16Stride[0]);
        pstDiprInfo->u32YBufAddr = pstDivpInputInfo->u64BufAddr[0];
        pstDiprInfo->u32CBufAddr = pstDivpInputInfo->u64BufAddr[1];
        pstDiprInfo->u32YBufAddr10Bits = 0;
        pstDiprInfo->u32CBufAddr10Bits = 0;
        // Fix for input buffer not match dipr allign rule.
        if (pstDivpInputInfo->ePxlFmt == E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264
            || pstDivpInputInfo->ePxlFmt == E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265
            || pstDivpInputInfo->ePxlFmt == E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265)
        {
            u16DiprAllignPixels = 32; //allign 32 pixels

        } else {
            const int iDipAllignSizes = 32; //allign 32 bytes
            u16DiprAllignPixels = (iDipAllignSizes * 2) / dipr_u8BytePer2Pixel;
        }
        if ((pstDiprInfo->u16Width % u16DiprAllignPixels) != 0)
        {
            // do allignment
            u16 u16Width = (pstDiprInfo->u16Width & (~(u16DiprAllignPixels - 1)))
                + u16DiprAllignPixels;
            if (u16Width <= ((pstDiprInfo->u16Pitch * 2) / dipr_u8BytePer2Pixel))
            {
                pstDiprInfo->u16Width = u16Width;
            }
        }
        enTileMode = _DipTileModeTransform(pstDivpInputInfo->eTileMode);
        enMFDecTileMode = _DipMFDecTileModeTransform(pstDivpInputInfo->eTileMode);

        if (pstDivpInputInfo->stMfdecInfo.bDbEnable == FALSE
            #if DIP_SUPPORT_3DDI
            && pstDivpInputInfo->eScanMode == E_MHAL_DIVP_SCAN_MODE_PROGRESSIVE
            #endif
            )
        {
            LOG_INFO("@@@@@@@@@@@@@@@@@@@@@u16Pitch = [%d] @@@@@@@@@@@@@@.\n", pstDiprInfo->u16Pitch);
            CHECK_APIXC_RET(MApi_XC_DIP_SetDIPRProperty(pstDiprInfo, enDipWin));
        } else {
            ST_DIP_MFDEC_INFO *pstDiprDecInfo = &g_stDipRes[enDipWin].stDiprInfo.stDIPR_MFDecInfo;
            ST_DIP_DIPR_3DDI_SETTING *pstDipr3DDiInfo = &g_stDipRes[enDipWin].stDiprInfo.stDIPR3DDISetting;
            MHAL_DIVP_MFdecInfo_t *pstDecBufInfo = &pstDivpInputInfo->stMfdecInfo;

            g_stDipRes[enDipWin].stDiprInfo.stDIPR_VerCtl.u32version = 0;
            g_stDipRes[enDipWin].stDiprInfo.stDIPR_VerCtl.u32size = sizeof(ST_XC_DIPR_PROPERTY_EX);
            g_stDipRes[enDipWin].stDiprInfo.pDIPRSetting = NULL;

            // set dipr window(ST_XC_DIPR_PROPERTY)
            // do nothing

            // set mfdec info
            if (pstDivpInputInfo->stMfdecInfo.bDbEnable == TRUE)
            {
                pstDiprDecInfo->stMFDec_VerCtl.u32version = 0;
                pstDiprDecInfo->stMFDec_VerCtl.u32size = sizeof(ST_DIP_MFDEC_INFO);
                pstDiprDecInfo->bMFDec_Enable = pstDecBufInfo->bDbEnable;
                pstDiprDecInfo->u8MFDec_Select = pstDecBufInfo->u8DbSelect;
                pstDiprDecInfo->bHMirror = pstDecBufInfo->stMirror.bHMirror;
                pstDiprDecInfo->bVMirror = pstDecBufInfo->stMirror.bVMirror;
                pstDiprDecInfo->enMFDec_tile_mode = enMFDecTileMode;
                pstDiprDecInfo->bUncompress_mode = pstDecBufInfo->bUncompressMode;
                pstDiprDecInfo->bBypass_codec_mode = pstDecBufInfo->bBypassCodecMode;
                switch (pstDecBufInfo->eDbMode)
                {
                    case E_MHAL_DIVP_DB_MODE_H264_H265:
                        pstDiprDecInfo->en_MFDecVP9_mode = E_XC_DIP_H26X_MODE;
                        break;
                    case E_MHAL_DIVP_DB_MODE_VP9:
                        pstDiprDecInfo->en_MFDecVP9_mode = E_XC_DIP_VP9_MODE;
                        break;
                    default :
                        LOG_ERR("DIPC Error en_MFDecVP9_mode[%d] not support.\n",
                            pstDecBufInfo->bDbEnable);
                        return FALSE;
                }

                pstDiprDecInfo->phyLuma_FB_Base = pstDiprInfo->u32YBufAddr;
                pstDiprDecInfo->phyChroma_FB_Base = pstDiprInfo->u32CBufAddr;
                pstDiprDecInfo->u16FB_Pitch = pstDiprInfo->u16Pitch;
                pstDiprDecInfo->u16StartX = pstDecBufInfo->u16StartX;
                pstDiprDecInfo->u16StartY = pstDecBufInfo->u16StartY;
                pstDiprDecInfo->u16HSize = pstDecBufInfo->u16Width;
                pstDiprDecInfo->u16VSize = pstDecBufInfo->u16Height;
                pstDiprDecInfo->phyBitlen_Base = pstDecBufInfo->u64DbBaseAddr;
                pstDiprDecInfo->u16Bitlen_Pitch = pstDecBufInfo->u16DbPitch;

                pstDiprDecInfo->stMFDec_HTLB_Info.stMFDec_HTLB_VerCtl.u32version = 0;
                pstDiprDecInfo->stMFDec_HTLB_Info.stMFDec_HTLB_VerCtl.u32size =
                    sizeof(ST_DIP_MFDEC_HTLB_INFO);
                pstDiprDecInfo->stMFDec_HTLB_Info.u32HTLBEntriesAddr =
                    pstDecBufInfo->u64LbAddr;
                pstDiprDecInfo->stMFDec_HTLB_Info.u8HTLBEntriesSize =
                    pstDecBufInfo->u8LbSize;
                pstDiprDecInfo->stMFDec_HTLB_Info.u8HTLBTableId =
                    pstDecBufInfo->u8LbTableId;
                pstDiprDecInfo->stMFDec_HTLB_Info.pHTLBInfo = NULL;
                pstDiprDecInfo->pMFDecInfo = NULL;
            }

            // set 3d di info
            #if DIP_SUPPORT_3DDI
            DivpChannelInfo *pstDivpChnInfo = &g_astDivpChnInfo[u8ChannelId];
            MHAL_DIVP_ScanMode_e eScanType = pstDivpInputInfo->eScanMode;//
            MHAL_DIVP_DiType_e eDiType = pstDivpChnInfo->eDiType;//pstDivpInputInfo->stDiSettings.eDiType;//g_stDipRes[enDipWin].eDiType;
            //MHAL_DIVP_DiType_e eDiType = pstDivpInputInfo->stDiSettings.eDiType;//g_stDipRes[enDipWin].eDiType;
            MHAL_DIVP_DiMode_e eDiMode = pstDivpInputInfo->stDiSettings.eDiMode;
            MHAL_DIVP_FieldType_e eFieldType = pstDivpInputInfo->stDiSettings.eFieldType;//

            LOG_DBG("E_MHAL_DIVP_DI_TYPE_MED eScanType=%u, eDiType=%u,  eDiMode=%u,eFieldType=%u.\n",
                 eScanType, eDiType, eDiMode, eFieldType);

            // 3ddi init
            if (eScanType == E_MHAL_DIVP_SCAN_MODE_PROGRESSIVE )
            {
                if(g_stDipRes[enDipWin].bIsDiEnable == TRUE)
                {
                    DipDiInit(eDevId, pstDivpChnInfo, FALSE);
                    g_stDipRes[enDipWin].bIsDiEnable = FALSE;
                }
            }
            else
            {
                if(g_stDipRes[enDipWin].bIsDiEnable == FALSE)
                {
                    DipDiInit(eDevId, pstDivpChnInfo, TRUE);
                    g_stDipRes[enDipWin].bIsDiEnable = TRUE;
                }
            }
            if (eDiType == E_MHAL_DIVP_DI_TYPE_3D)
            {
                LOG_DBG("E_MHAL_DIVP_DI_TYPE_3DDI>>>.\n");
                pstDipr3DDiSetting->bEnableDI = TRUE;
                pstDipr3DDiSetting->bInitDI = FALSE;
                pstDipr3DDiSetting->u8NwayIndex = u8ChannelId;//channel id -->u32Window;
                pstDipr3DDiSetting->enDI_Field = _DipGetDiFieldType(eDiType, eScanType, eFieldType);//_DMS_DIP_GetDIPDiDataFMT(u32Window, RES_DMSDI._stXC_DIPDIInfo[u32Window][u16BufferID].eScanType, RES_DMSDI._stXC_DIPDIInfo[u32Window][u16BufferID].eFieldType);
                LOG_DBG("E_MHAL_DIVP_DI_TYPE_3DDI>>> enDI_Field=%u.\n", pstDipr3DDiSetting->enDI_Field);

                pstDipr3DDiSetting->bEnaNwayInfoRefresh = FALSE;
                pstDipr3DDiSetting->u8NwayBufCnt = 1; // Fix me
                pstDipr3DDiSetting->bEnableBob = TRUE;// Fix me
                #if 0
                if(u8count < 2) //the first && the second field need run bob mode
                {LOG_DBG("E_MHAL_DIVP_DI_TYPE_3DDI>>> bEnableBob.\n");
                    pstDipr3DDiSetting->bEnableBob = TRUE;// Fix me
                    u8count ++;
                }
                else //run nonbob mode
                {LOG_DBG("E_MHAL_DIVP_DI_TYPE_3DDI>>> no bob.\n");
                    pstDipr3DDiSetting->bEnableBob = FALSE;// Fix me
                }
                #else
                if (E_MHAL_DIVP_DI_MODE_BOB == eDiMode)
                {
                    pstDipr3DDiSetting->bEnableBob = TRUE;
                }
                else
                {
                    pstDipr3DDiSetting->bEnableBob = FALSE;// Fix me
                }

                #endif
                stDINwayBuf[0].phyDI_BufAddr = pstDivpChnInfo->u32DIStartAddr;
                stDINwayBuf[0].u32DI_BufSize = pstDivpChnInfo->u32DISize;
                stDINwayBuf[0].u8DIBufCnt = pstDivpChnInfo->u8DIBufCnt;
                pstDipr3DDiSetting->stDINwayBufInfo = (void*)stDINwayBuf;
                LOG_DBG("E_MHAL_DIVP_DI_TYPE_3DDI>>> phyDI_BufAddr=%#llx.\n",stDINwayBuf[0].phyDI_BufAddr);

                // interlace mode, height *1/2
                //pstDiprInfo->u16Height = pstDiprInfo->u16Height>>1;

                pstDipr3DDiInfo->stDipr3DDI_VerCtl.u32version = 1;
                pstDipr3DDiInfo->stDipr3DDI_VerCtl.u32size = sizeof(ST_DIP_DI_SETTING);
                pstDipr3DDiInfo->pDIPR3DDISetting = pstDipr3DDiSetting;

            }
            else if (eDiType == E_MHAL_DIVP_DI_TYPE_2D)//2.5D di
            {
                pstDipr3DDiSetting->bEnableDI = TRUE;
                pstDipr3DDiSetting->enDI_Field = _DipGetDiFieldType(eDiType, eScanType, eFieldType);//_DMS_DIP_GetDIPDiDataFMT(u32Window, RES_DMS._stXC_DIPPushInfo[u32Window][u16BufferID].eScanType, RES_DMS._stXC_DIPPushInfo[u32Window][u16BufferID].eFieldType);
                if(eScanType == E_MHAL_DIVP_SCAN_MODE_INTERLACE_FIELD)//(_DMS_Is_SourceFieldBaseInterlace(RES_DMS._stXC_DIPPushInfo[u32Window][u16BufferID].eScanType))
                {//field base di
                    pstDipr3DDiSetting->phyBotYBufAddr = 0;//RES_DMS._stXC_DIPPushInfo[u32Window][u16BufferID].u32SrcLumaAddrI;
                    pstDipr3DDiSetting->phyBotCBufAddr = 0;//RES_DMS._stXC_DIPPushInfo[u32Window][u16BufferID].u32SrcChromaAddrI;
                    pstDipr3DDiSetting->phyBotYBufAddr10Bits = 0;//RES_DMS._stXC_DIPPushInfo[u32Window][u16BufferID].u32SrcLumaAddrI;
                    pstDipr3DDiSetting->phyBotCBufAddr10Bits = 0;//RES_DMS._stXC_DIPPushInfo[u32Window][u16BufferID].u32SrcChromaAddrI;
                }
                else
                {//frame base di
                    pstDipr3DDiSetting->phyBotYBufAddr = pstDiprInfo->u32YBufAddr;//DIPRWinPropertytoShow_EX.stDIPRProperty.u32YBufAddr;
                    pstDipr3DDiSetting->phyBotCBufAddr = pstDiprInfo->u32CBufAddr;//DIPRWinPropertytoShow_EX.stDIPRProperty.u32CBufAddr;
                    pstDipr3DDiSetting->phyBotYBufAddr10Bits = pstDiprInfo->u32YBufAddr10Bits;//DIPRWinPropertytoShow_EX.stDIPRProperty.u32YBufAddr;
                    pstDipr3DDiSetting->phyBotCBufAddr10Bits = pstDiprInfo->u32CBufAddr10Bits;//DIPRWinPropertytoShow_EX.stDIPRProperty.u32CBufAddr;
                }

                pstDipr3DDiInfo->stDipr3DDI_VerCtl.u32version = 1;
                pstDipr3DDiInfo->stDipr3DDI_VerCtl.u32size = sizeof(ST_DIP_DI_SETTING);
                pstDipr3DDiInfo->pDIPR3DDISetting = pstDipr3DDiSetting;
            }
            else
            {
                pstDipr3DDiInfo->stDipr3DDI_VerCtl.u32version = 0;
                pstDipr3DDiInfo->stDipr3DDI_VerCtl.u32size = 0;//sizeof(ST_DIP_DI_SETTING);
                pstDipr3DDiInfo->pDIPR3DDISetting = 0;//pstDipr3DDiSetting;
            }

            #else
            pstDipr3DDiInfo->stDipr3DDI_VerCtl.u32version = 0;
            pstDipr3DDiInfo->stDipr3DDI_VerCtl.u32size = 0;
            pstDipr3DDiInfo->pDIPR3DDISetting = NULL;
            #endif

            CHECK_APIXC_RET(MApi_XC_DIP_SetDIPRProperty_EX(&g_stDipRes[enDipWin].stDiprInfo, enDipWin));
        }
    }

     {
        //set output memory formate
        eDipOutputFormat = _DipDataFormatTransform(pstDivpOutputInfo->ePxlFmt);
        switch(pstDivpOutputInfo->ePxlFmt)
        {
            case E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV:
                eDipOutputFormat = DIP_DATA_FMT_YUV422;
                MApi_XC_DIP_EnableY2R(FALSE, enDipWin);
                break;
            case E_MHAL_DIVP_PIXEL_FORMAT_ARGB8888:
                eDipOutputFormat = DIP_DATA_FMT_ARGB8888;
                MApi_XC_DIP_EnableY2R(TRUE, enDipWin);
                MApi_XC_DIP_SetAlpha(0xFF, enDipWin);
                break;
            case E_MHAL_DIVP_PIXEL_FORMAT_ABGR8888:
                eDipOutputFormat = DIP_DATA_FMT_ARGB8888;
                MApi_XC_DIP_EnableY2R(TRUE, enDipWin);
                MApi_XC_DIP_SetAlpha(0xFF, enDipWin);
                break;
            case E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
                eDipOutputFormat = DIP_DATA_FMT_YUV420;
                MApi_XC_DIP_EnableY2R(FALSE, enDipWin);
                break;
            default:
                LOG_ERR("[DIPC] pstTravelInfo not support.\n");
                return FALSE;
        }
        CHECK_APIXC_RET(MApi_XC_DIP_SetOutputDataFmt(eDipOutputFormat, enDipWin));
        g_stDipRes[enDipWin].eOutputFmt = eDipOutputFormat;
    }

    {
        //Capture window
        //input and output setting
        pstDipWindowInfo->stCapWin.x = pstDivpInputInfo->stCropWin.u16X;//pstDivpInputInfo->stCropWin.u16X;//0;
        pstDipWindowInfo->stCapWin.y = pstDivpInputInfo->stCropWin.u16Y;//0;
        pstDipWindowInfo->stCapWin.width = pstDivpInputInfo->stCropWin.u16Width;
        pstDipWindowInfo->stCapWin.height = pstDivpInputInfo->stCropWin.u16Height;
        pstDipWindowInfo->bPreHCusScaling = TRUE;
        pstDipWindowInfo->u16PreHCusScalingSrc = pstDipWindowInfo->stCapWin.width;
        pstDipWindowInfo->u16PreHCusScalingDst = pstDivpOutputInfo->u16OutputWidth;//stTempTravelInfo.u16TravelWidth;
        pstDipWindowInfo->bPreVCusScaling = TRUE;
        pstDipWindowInfo->u16PreVCusScalingSrc = pstDipWindowInfo->stCapWin.height;
        pstDipWindowInfo->u16PreVCusScalingDst = pstDivpOutputInfo->u16OutputHeight;//stTempTravelInfo.u16TravelHeight;
        CHECK_APIXC_RET(MApi_XC_DIP_SetWindow(pstDipWindowInfo, sizeof(XC_SETWIN_INFO), enDipWin));
    }

    if(bIsNeedScalingUp)
    {
        CHECK_APIXC_RET(MApi_XC_DIP_SetHVSP(TRUE, enDipWin));
    }

    {
        //DIPW win setting
        //output setting
        //DivpChannelInfo *pstDivpCtx = (DivpChannelInfo *)pCtx;
        pstDipwInfo->enSource = SCALER_DIP_SOURCE_TYPE_DRAM;
        pstDipwInfo->u16Width = pstDivpOutputInfo->u16OutputWidth;
        pstDipwInfo->u16Height = pstDivpOutputInfo->u16OutputHeight;
        pstDipwInfo->u16Pitch = pstDivpOutputInfo->u16Stride[0];//stTempTravelInfo.u16MemoryPitch;
        pstDipwInfo->u8BufCnt = 1;
        pstDipwInfo->u32BufStart = pstDivpOutputInfo->u64BufAddr[0];//stTempTravelInfo.u32MemoryPhyAddress;
        pstDipwInfo->u32BufEnd = pstDivpOutputInfo->u64BufAddr[0] + pstDivpOutputInfo->u32BufSize;
        if(pstDivpOutputInfo->u32BufSize <
            (u32)(pstDipwInfo->u16Pitch * pstDipwInfo->u16Height))
        {
            LOG_ERR("[DIPC] UpdateFrameBuffer u32MemorySize[%#x] is not support.\n",
                pstDivpOutputInfo->u32BufSize);
            return FALSE;
        }
        CHECK_APIXC_RET(MApi_XC_DIP_SetDIPWinProperty(pstDipwInfo, enDipWin));

    }

    if((enTileMode != DIP_TILE_BLOCK_R_NONE) &&
        (pstDivpInputInfo->ePxlFmt == E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264
        || pstDivpInputInfo->ePxlFmt == E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265
        || pstDivpInputInfo->ePxlFmt == E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265))
    {
        CHECK_APIXC_RET(MApi_XC_DIP_Set420TileBlock(enTileMode, enDipWin));
    }

    if (g_stDipRes[enDipWin].bUSeCmdq) {
        //dip cmdq flow
        if (bLastFrame)
        {
            // unmask interrupt, enable intrrupt
            CHECK_APIXC_RET(MApi_XC_DIP_EnaInt(0xFFFF, TRUE, enDipWin));
        }

        CHECK_APIXC_RET(MApi_XC_DIP_CapOneFrameFast(enDipWin));

        if(!bLastFrame) {
            if(bIsNeedScalingUp) {
                CHECK_APIXC_RET(MApi_XC_DIP_SetHVSP(FALSE, enDipWin));
            }
        }
    } else {

        CHECK_APIXC_RET(MApi_XC_DIP_CapOneFrameFast(enDipWin));

        //wait dip write buffer done
        bRet = DipWaitWritebufferDone(eDevId);
        //if(bRet != TRUE)
        {
            _DipPrintLog(eDevId);
        }

        if(bIsNeedScalingUp)
        {
            CHECK_APIXC_RET(MApi_XC_DIP_SetHVSP(FALSE, enDipWin));
        }

        CHECK_APIXC_RET(MApi_XC_DIP_ClearInt(0xFF, enDipWin));
    }

    return bRet;
}

bool DipUpdateFrameBuffer(DivpChannelInfo *pstDivpCtx, MHAL_DIVP_InputInfo_t* pstDivpInputInfo, MHAL_DIVP_OutPutInfo_t* pstDivpOutputInfo, MHAL_CMDQ_CmdqInterface_t* pstCmdInf, MHAL_DIVP_DeviceId_e eDevId)
{
    bool bRet = TRUE;
    SCALER_DIP_WIN enDipWin = (SCALER_DIP_WIN)eDevId;
    MHAL_CMDQ_CmdqInterface_t *pCmdqOps = NULL;
    FUN_ENTRY();

    if(pstCmdInf != NULL)
    {
        g_stDipRes[enDipWin].bUSeCmdq = TRUE;
        g_stDipRes[enDipWin].pCmdqOps = pstCmdInf;
    }

    if(g_stDipRes[enDipWin].bUSeCmdq && g_stDipRes[enDipWin].pCmdqOps != NULL)
    {
        LOG_DBG("MApi_XC_DIP_CMDQ_SetOperations enable cmdq_srv.\n");
        MApi_XC_DIP_CMDQ_SetOperations(g_stDipRes[enDipWin].pCmdqOps, TRUE, enDipWin);
    }

    DipUpdateFrameBufferIndex(pstDivpCtx, pstDivpInputInfo, pstDivpOutputInfo, 0, TRUE/*bLastFrame*/, eDevId);

    #if DIP_CMDQ
    if(g_stDipRes[enDipWin].bUSeCmdq && g_stDipRes[enDipWin].pCmdqOps)
    {
        pCmdqOps = g_stDipRes[enDipWin].pCmdqOps;
        pCmdqOps->MHAL_CMDQ_CmdqAddWaitEventCmd(pCmdqOps, E_MHAL_CMDQEVE_DIP_TRIG);
        //pCmdqOps->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqOps, 0x911);
        pCmdqOps->MHAL_CMDQ_KickOffCmdq(pCmdqOps);
    }
    #endif

    if(g_stDipRes[enDipWin].bUSeCmdq && g_stDipRes[enDipWin].pCmdqOps != NULL)
    {
        LOG_DBG("MApi_XC_DIP_CMDQ_SetOperations disable cmdq_srv.\n");
        MApi_XC_DIP_CMDQ_SetOperations(g_stDipRes[enDipWin].pCmdqOps, FALSE, enDipWin);
    }

    #if DIP_CMDQ
    //wait dip write buffer done
    bRet = DipWaitWritebufferDone(eDevId);
    //if(bRet != TRUE)
    {
        _DipPrintLog(eDevId);
    }
    #endif

    FUN_EXIT();
    return bRet;
}

bool DipGEScaling(void)
{
#if 0
    GFX_Config gfx_config;
    gfx_config.u8Miu = 0;
    gfx_config.u8Dither = FALSE;
    gfx_config.u32VCmdQSize = 0;
    gfx_config.u32VCmdQAddr = 0;
    gfx_config.bIsHK = 1;
    gfx_config.bIsCompt = FALSE;
    MApi_GFX_Init(&gfx_config);

    //do scaling
    GFX_BufferInfo stSrcbuf, stDstbuf;
    stSrcbuf.u32Addr = 0;
    stSrcbuf.u32Width =  0;
    stSrcbuf.u32Height =  0;
    stSrcbuf.u32Pitch = 0;
    stSrcbuf.u32ColorFmt = GFX_FMT_YUV422;

    stDstbuf.u32Addr = 0;
    stDstbuf.u32Height = 0;
    stDstbuf.u32Pitch = 0;
    stDstbuf.u32Pitch = 0;
    stDstbuf.u32ColorFmt = GFX_FMT_YUV422;

    MS_U32 u32GfxScale = GFXDRAW_FLAG_DEFAULT;
    GFX_DrawRect stBitbltInfo;
    GFX_Point stP0, stP1;

    MApi_GFX_SetDC_CSC_FMT(GFX_YUV_RGB2YUV_PC, GFX_YUV_OUT_255, GFX_YUV_IN_255, GFX_YUV_YUYV,  GFX_YUV_YUYV);
    //MApi_GFX_SetDC_CSC_FMT(GFX_YUV_RGB2YUV_PC, GFX_YUV_OUT_PC, GFX_YUV_IN_255, GFX_YUV_YUYV,  GFX_YUV_YUYV);
    MApi_GFX_SetAlpha(FALSE, COEF_ONE, ABL_FROM_ASRC, 0xFF);

    GFX_BufferInfo stSrcbuf, stDstbuf;
    memset(&stSrcbuf, 0, sizeof(GFX_BufferInfo));
    memset(&stDstbuf, 0, sizeof(GFX_BufferInfo));

    stSrcbuf.u32Addr = 0;//u2Addr;//_stNVRBlitFrameParam.u32SrcAddr;
    stSrcbuf.u32Width =  0;//pstTravelInfo->stDIPRProperty.u16Width;//_stNVRBlitFrameParam.u32SrcWidth;
    stSrcbuf.u32Height =  0;//pstTravelInfo->stDIPRProperty.u16Height;//_stNVRBlitFrameParam.u32SrcHeight;
    stSrcbuf.u32Pitch = 0;//pstTravelInfo->u16MemoryPitch;//3840;//u32TgtSrcPitch;//0;
    stSrcbuf.u32ColorFmt = GFX_FMT_YUV422;

    stDstbuf.u32Addr = 0;//pstTravelInfo->u32MemoryPhyAddress;//_stNVRBlitFrameParam.u32DstAddr;
    stDstbuf.u32Width = 0;//pstTravelInfo->u16TravelWidth;//_stNVRBlitFrameParam.u32DstWidth;
    stDstbuf.u32Height = 0;//pstTravelInfo->u16TravelHeight;//_stNVRBlitFrameParam.u32DstHeight;
    stDstbuf.u32Pitch = 0;//pstTravelInfo->u16MemoryPitch;//3840;//u32TgtDstPitch;//0;
    stDstbuf.u32ColorFmt = GFX_FMT_YUV422;

    LOG_INFO("lsy CAP OSD, %#llx,%d:%d,%d --> %#llx,%d:%d,%d;\n",
        stSrcbuf.u32Addr, stSrcbuf.u32Width, stSrcbuf.u32Height, stSrcbuf.u32Pitch,
        stDstbuf.u32Addr, stDstbuf.u32Width, stDstbuf.u32Height, stDstbuf.u32Pitch);

    MApi_GFX_BeginDraw();
    MApi_GFX_EnableDFBBlending(FALSE);
    MApi_GFX_EnableAlphaBlending(FALSE);
    MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ASRC);

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
        LOG_ERR("[DipDataHandleThread]GFX set SrcBuffer failed\n");
        return FALSE;
    }

    if (MApi_GFX_SetDstBufferInfo(&stDstbuf, 0) != GFX_SUCCESS)
    {
        LOG_ERR("[DipDataHandleThread]GFX set DetBuffer failed\n");
        return FALSE;
    }

    stBitbltInfo.srcblk.x = 0;
    stBitbltInfo.srcblk.y = 0;
    stBitbltInfo.srcblk.width = stSrcbuf.u32Width;
    stBitbltInfo.srcblk.height = stSrcbuf.u32Height;

    stBitbltInfo.dstblk.x = 0;
    stBitbltInfo.dstblk.y = 0;
    stBitbltInfo.dstblk.width = stDstbuf.u32Width;
    stBitbltInfo.dstblk.height = stDstbuf.u32Height;

    MS_BOOL bMirrorH =FALSE;
    MS_BOOL bMirrorV =FALSE;
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
        LOG_ERR("[DipDataHandleThread]GFX BitBlt failed\n");
        return FALSE;
    }
    //MI_U16 u16TagId = MApi_GFX_SetNextTAGID();
    //MApi_GFX_WaitForTAGID(u16TagId );

    if (MApi_GFX_FlushQueue() != GFX_SUCCESS)
    {
        LOG_ERR("[DipDataHandleThread]GFX FlushQueue failed\n");
        return FALSE;
    }
    MApi_GFX_EndDraw();

    MApi_GE_Exit();
    #endif
    return TRUE;
}
MS_S32 MHAL_DIVP_Init(MHAL_DIVP_DeviceId_e eDevId)
{
    MS_S32 rt = MHAL_SUCCESS;
    if(eDevId >= E_MHAL_DIVP_Device_MAX)
    {
        LOG_ERR("eDevId(%d) is not support.\n", eDevId);
        return MHAL_FAILURE;
    }
    rt = DipInit(eDevId)  == TRUE? MHAL_SUCCESS: MHAL_FAILURE;
    return rt ;
}
MS_S32 MHAL_DIVP_DeInit(MHAL_DIVP_DeviceId_e eDevId)
{
    MS_S32 rt = MHAL_SUCCESS;
    if(eDevId >= E_MHAL_DIVP_Device_MAX)
    {
        LOG_ERR("eDevId(%d) is not support.\n", eDevId);
        return MHAL_FAILURE;
    }

    rt = DipDeInit(eDevId)  == TRUE? MHAL_SUCCESS: MHAL_FAILURE;
    return rt ;
}

MS_S32 MHAL_DIVP_GetChipCapbility(MHAL_DIVP_DeviceId_e eDevId, MS_U32* pu32Caps)
{
    MS_S32 s32Ret = MHAL_FAILURE;
    ST_XC_DIP_CHIPCAPS stDivpCaps;
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_INITDATA stXC_InitData;
    memset(&stXC_InitData, 0, sizeof(XC_INITDATA));
    memset(&stDivpCaps, 0, sizeof(stDivpCaps));

    if((NULL == pu32Caps) || (eDevId >= E_MHAL_DIVP_Device_MAX))
    {
        LOG_ERR("invalid parameter : pu32Caps = %p, eDevId = %u .\n", pu32Caps, eDevId);
        return s32Ret;
    }

    *pu32Caps = 0;
    stDivpCaps.eWindow = (SCALER_DIP_WIN)eDevId;
    MApi_XC_Init(&stXC_InitData, 0);
    eRet = MApi_XC_GetChipCaps(E_XC_DIP_CHIP_CAPS, (MS_U32*)&stDivpCaps, sizeof(stDivpCaps));
    if(E_APIXC_RET_OK == eRet)
    {
        *pu32Caps = stDivpCaps.u32DipChipCaps;
        s32Ret = MHAL_SUCCESS;
    }
    else
    {
        LOG_ERR("eRet = %u, stDivpCaps.u32DipChipCaps = %u .\n", eRet, stDivpCaps.u32DipChipCaps);
    }

    return s32Ret;
}

// create channel contexct
MS_S32 MHAL_DIVP_CreateInstance(MHAL_DIVP_DeviceId_e eDevId, MS_U16 u16MaxWidth,
    MS_U16 u16MaxHeight, PfnAlloc pfAlloc, PfnFree pfFree, MS_U8 u8ChannelId, void** ppCtx)
{
    MS_S32 rt = MHAL_SUCCESS;
    SCALER_DIP_WIN enDipWin = (SCALER_DIP_WIN)eDevId;
    if(eDevId >= E_MHAL_DIVP_Device_MAX
        || pfAlloc == NULL
        || pfFree == NULL
        || u8ChannelId >= MAX_CHANNEL_NUM)
    {
        LOG_ERR("eDevId(%d) pfAlloc(%p) pfFree(%p) u8ChannelId(%u)is not support.\n", eDevId, pfAlloc, pfFree, u8ChannelId);
        return MHAL_FAILURE;
    }

    g_stDipRes[enDipWin].pfAlloc = pfAlloc;
    g_stDipRes[enDipWin].pfFree = pfFree;

    memset(&g_astDivpChnInfo[u8ChannelId], 0, sizeof(DivpChannelInfo));
    //3ddi &&tnr, dip need buffer
    g_astDivpChnInfo[u8ChannelId].u16StructSize = sizeof(DivpChannelInfo);
    g_astDivpChnInfo[u8ChannelId].u8ChannelId = u8ChannelId;
    g_astDivpChnInfo[u8ChannelId].eDevId = eDevId;
    g_astDivpChnInfo[u8ChannelId].u16MaxWidth = u16MaxWidth;
    g_astDivpChnInfo[u8ChannelId].u16MaxHeight = u16MaxHeight;
    g_astDivpChnInfo[u8ChannelId].pCtx = (void *)&g_astDivpChnInfo[u8ChannelId];

    * ppCtx = g_astDivpChnInfo[u8ChannelId].pCtx;
    LOG_DBG("*pCtx = %p.\n", g_astDivpChnInfo[u8ChannelId].pCtx);

    return rt;
}


// destory channel contexct
MS_S32 MHAL_DIVP_DestroyInstance(void* pCtx)
{
    MS_S32 rt = MHAL_SUCCESS;
    if (pCtx == NULL)
    {
        LOG_ERR("pCtx(%p) is not support.\n", pCtx);
        return MHAL_FAILURE;
    }


    DivpChannelInfo *pstDivpChnInfo = (DivpChannelInfo *)pCtx;
    if(pCtx != pstDivpChnInfo->pCtx)
    {
        LOG_ERR("this is not a valid pCtx(0x%p),pstDivpCtx(0x%p)", pCtx, pstDivpChnInfo->pCtx);
        return MHAL_FAILURE;
    }

    if (pstDivpChnInfo->u32DIStartAddr)
    {
        //free di buffer
        _DipFreeMem(pstDivpChnInfo, pstDivpChnInfo->u32DIStartAddr);
    }
    memset(pstDivpChnInfo, 0, sizeof(DivpChannelInfo));


    return rt;
}

//the api for dip capture from xc op2/vop
MS_S32 MHAL_DIVP_CaptureTiming(void* pCtx, MHAL_DIVP_CaptureInfo_t* ptCaptureInfo,
    MHAL_CMDQ_CmdqInterface_t* ptCmdInf)
{
    bool rt = MHAL_SUCCESS;
    CHECK_ASSERT(pCtx != NULL);
    CHECK_ASSERT(ptCaptureInfo != NULL);
    CHECK_ASSERT(ptCmdInf != NULL);
    if (pCtx == NULL
        || ptCaptureInfo == NULL)
    {
        LOG_ERR("pCtx(%p) is not support.\n", pCtx);
        return MHAL_FAILURE;
    }

    DipCaptureTiming(ptCaptureInfo, ptCmdInf, (DivpChannelInfo *)pCtx);
    return rt;
}

//the api for dip capture from dram, output to dram
MS_S32 MHAL_DIVP_ProcessDramData(void* pCtx, MHAL_DIVP_InputInfo_t* ptDivpInputInfo,
    MHAL_DIVP_OutPutInfo_t* ptDivpOutputInfo, MHAL_CMDQ_CmdqInterface_t* pstCmdInf)
{
    DivpChannelInfo *pstDivpChnInfo = (DivpChannelInfo *)pCtx;
    CHECK_ASSERT(pCtx != NULL);
    CHECK_ASSERT(ptDivpInputInfo != NULL);
    CHECK_ASSERT(ptDivpOutputInfo != NULL);
    //CHECK_ASSERT(pstCmdInf != NULL);

    DipUpdateFrameBuffer(pstDivpChnInfo, ptDivpInputInfo, ptDivpOutputInfo, pstCmdInf, pstDivpChnInfo->eDevId);
    return MHAL_SUCCESS;
}

MS_S32 MHAL_DIVP_SetAttr(void* pCtx, MHAL_DIVP_AttrType_e eAttrType, const void* pAttrParam,
    MHAL_CMDQ_CmdqInterface_t* ptCmdInf)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MS_U8 u8ChannelId = 0;
    DivpChannelInfo *pstDivpChnInfo = (DivpChannelInfo *)pCtx;
    MHAL_DIVP_Window_t *pstCropWin = NULL;
    CHECK_ASSERT(pCtx);
    CHECK_ASSERT(pAttrParam);
    CHECK_ASSERT(ptCmdInf);
    if(pCtx != pstDivpChnInfo->pCtx)
    {
        LOG_ERR("this is not a valid pCtx(0x%p),pstDivpCtx(0x%p)", pCtx, pstDivpChnInfo->pCtx);
        return MHAL_FAILURE;
    }
    if(pstDivpChnInfo->u8ChannelId > MAX_CHANNEL_NUM)
    {
        LOG_ERR("a invalid u8ChannelId(%d).\n", pstDivpChnInfo->u8ChannelId);
        return MHAL_FAILURE;
    }
    u8ChannelId = pstDivpChnInfo->u8ChannelId;
    switch (eAttrType)
    {
        case E_MHAL_DIVP_ATTR_DI:
            g_astDivpChnInfo[u8ChannelId].eDiType = *((MHAL_DIVP_DiType_e*)pAttrParam);
            break;
        case E_MHAL_DIVP_ATTR_ROTATE:
            g_astDivpChnInfo[u8ChannelId].eRotate = *((MHAL_DIVP_Rotate_e *)pAttrParam);
            break;
        case E_MHAL_DIVP_ATTR_CROP:
            pstCropWin = (MHAL_DIVP_Window_t *)pAttrParam;
            CHECK_ASSERT(pstCropWin != NULL);
            g_astDivpChnInfo[u8ChannelId].stCropWin.u16X = pstCropWin->u16X;
            g_astDivpChnInfo[u8ChannelId].stCropWin.u16Y = pstCropWin->u16Y;
            g_astDivpChnInfo[u8ChannelId].stCropWin.u16Width = pstCropWin->u16Width;
            g_astDivpChnInfo[u8ChannelId].stCropWin.u16Height = pstCropWin->u16Height;
            break;
        case E_MHAL_DIVP_ATTR_MIRROR:
            //MHAL_DIVP_Mirror_t *pstMirror = (MHAL_DIVP_Mirror_t *)pAttrParam;
            g_astDivpChnInfo[u8ChannelId].stMirror.bHMirror= ((MHAL_DIVP_Mirror_t *)pAttrParam)->bHMirror;
            g_astDivpChnInfo[u8ChannelId].stMirror.bVMirror= ((MHAL_DIVP_Mirror_t *)pAttrParam)->bVMirror;
            break;
        case E_MHAL_DIVP_ATTR_TNR:
            g_astDivpChnInfo[u8ChannelId].eTnrLvl = *((MHAL_DIVP_TnrLevel_e *)pAttrParam);
            break;
        default:
            LOG_ERR("unknow eAttrType=%d.\n", eAttrType);
            return MHAL_FAILURE;
    }
    return s32Ret;
}

MS_S32 MHAL_DIVP_EnableFrameDoneIsr(MS_BOOL bEnable)
{
    //clear interrupt status
    if(bEnable)
    {
        MsOS_EnableInterrupt(E_INT_IRQ_DIPW);
    }
    else
    {
        MsOS_DisableInterrupt(E_INT_IRQ_DIPW);
    }

    return MHAL_SUCCESS;
}

MS_S32 MHAL_DIVP_CleanFrameDoneIsr(void)
{
    LOG_DBG("get a dip irq isr--------------->.\n");
    MApi_XC_DIP_ClearInt(0xFFFF, DIP_WINDOW);
    MHAL_DIVP_EnableFrameDoneIsr(TRUE);
    return MHAL_SUCCESS;
}

MS_S32 MHAL_DIVP_GetIrqNum(MS_U32 *pu32IrqNum)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    *pu32IrqNum = DIP_IRQ_NUM;
    return s32Ret;
}

//------------------------------------------------------------------------------
/// @brief wait cmdq frame done..
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_DIVP_WaitCmdQFrameDone(MHAL_CMDQ_CmdqInterface_t* pstCmdQInfo, MHAL_CMDQ_EventId_e eEventId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    if((eEventId == E_MHAL_CMDQEVE_DIP_TRIG) || (eEventId == E_MHAL_CMDQEVE_S1_MDW_W_DONE))
    {
        pstCmdQInfo->MHAL_CMDQ_CmdqAddWaitEventCmd(pstCmdQInfo, eEventId);
    }
    else
    {
        s32Ret = MHAL_FAILURE;
        LOG_ERR("Unkonwn CmdQ event ID!!!!\n");
    }
    pstCmdQInfo->MHAL_CMDQ_WriteRegCmdqMask(pstCmdQInfo, 0x1235A8, 0x40, 0x40);
    pstCmdQInfo->MHAL_CMDQ_WriteRegCmdqMask(pstCmdQInfo, 0x1235A8, 0x40, 0x40);
    pstCmdQInfo->MHAL_CMDQ_WriteRegCmdqMask(pstCmdQInfo, 0x1235A8, 0x40, 0x40);
    pstCmdQInfo->MHAL_CMDQ_WriteRegCmdqMask(pstCmdQInfo, 0x1235A8, 0x40, 0x40);
    //MDrv_XC_DIVP_CMDQ_ClearIntr(pstCmdQInfo, 0x01, DIP_WINDOW);

    return s32Ret;
}
