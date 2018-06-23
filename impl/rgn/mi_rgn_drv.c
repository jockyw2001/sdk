#include <linux/string.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <linux/semaphore.h>
#include <linux/list_sort.h>

#include "mhal_rgn.h"

#include "mi_common.h"
#include "mi_sys_internal.h"
#if (INTERFACE_GFX == 1)
#include "mi_gfx.h"
#endif
#include "mi_print.h"
#include "mi_rgn_internal.h"
#include "mi_rgn_datatype_internal.h"
#include "mi_rgn_drv.h"

#define MI_RGN_MAX_GOP_GWIN_NUM 2
#define MI_RGN_OSD_MAX_NUM 4
#define MI_RGN_COVER_MAX_NUM 4

#define MI_RGN_OVERLAY_MIN_WIDTH 2
#define MI_RGN_OVERLAY_MAX_WIDTH 3840
#define MI_RGN_OVERLAY_MIN_HEIGHT 2
#define MI_RGN_OVERLAY_MAX_HEIGHT 2160
#define MI_RGN_COVER_MIN_WIDTH 1
#define MI_RGN_COVER_MAX_WIDTH 8192
#define MI_RGN_COVER_MIN_HEIGHT 1
#define MI_RGN_COVER_MAX_HEIGHT 8192

#define MI_RGN_VPE_MAX_CH_NUM 64
#define MI_RGN_DIVP_MAX_CH_NUM 64
#define MI_RGN_OSD_WIDTH_ALIGN 2
#define MI_RGN_OSD_I2_WIDTH_ALIGN 4

#define MI_RGN_ENABLE_COLOR_KEY 1
#define MI_RGN_INIT_ALPHA_VALUE 0xFFFF
#define MI_RGN_COLOR_KEY_VALUE 0x2323

#define MI_RGN_BUF_DEL_WORK_TIMEOUT 500 //ms

#define MI_RGN_MAP_PIXFORMAT_TO_MI(mi, mhal) do{  \
                                                    switch (mhal) \
                                                    {   \
                                                        case E_MHAL_RGN_PIXEL_FORMAT_ARGB1555:    \
                                                        {   \
                                                            mi = E_MI_RGN_PIXEL_FORMAT_ARGB1555;    \
                                                        }   \
                                                        break;  \
                                                        case E_MHAL_RGN_PIXEL_FORMAT_ARGB4444:    \
                                                        {   \
                                                            mi = E_MI_RGN_PIXEL_FORMAT_ARGB4444;    \
                                                        }   \
                                                        break;  \
                                                        case E_MHAL_RGN_PIXEL_FORMAT_I2:  \
                                                        {  \
                                                            mi = E_MI_RGN_PIXEL_FORMAT_I2;  \
                                                        }   \
                                                        break;  \
                                                        case E_MHAL_RGN_PIXEL_FORMAT_I4:  \
                                                        {   \
                                                            mi = E_MI_RGN_PIXEL_FORMAT_I4;  \
                                                        }   \
                                                        break;  \
                                                        case E_MHAL_RGN_PIXEL_FORMAT_I8:  \
                                                        {   \
                                                            mi = E_MI_RGN_PIXEL_FORMAT_I8;  \
                                                        }   \
                                                        break;  \
                                                        case E_MHAL_RGN_PIXEL_FORMAT_RGB565:  \
                                                        {   \
                                                            mi = E_MI_RGN_PIXEL_FORMAT_RGB565;  \
                                                        }   \
                                                        break;  \
                                                        case E_MHAL_RGN_PIXEL_FORMAT_ARGB8888:  \
                                                        {   \
                                                            mi = E_MI_RGN_PIXEL_FORMAT_ARGB8888;  \
                                                        }   \
                                                        break;  \
                                                        default:    \
                                                        {   \
                                                            mhal = E_MI_RGN_PIXEL_FORMAT_MAX; \
                                                        }   \
                                                        break;  \
                                                    }   \
                                               }while (0);


#define MI_RGN_MAP_PIXFORMAT_TO_MHAL(mhal, mi) do{  \
                                                    switch (mi) \
                                                    {   \
                                                        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:    \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_ARGB1555;    \
                                                        }   \
                                                        break;  \
                                                        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:    \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_ARGB4444;    \
                                                        }   \
                                                        break;  \
                                                        case E_MI_RGN_PIXEL_FORMAT_I2:  \
                                                        {  \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_I2;  \
                                                        }   \
                                                        break;  \
                                                        case E_MI_RGN_PIXEL_FORMAT_I4:  \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_I4;  \
                                                        }   \
                                                        break;  \
                                                        case E_MI_RGN_PIXEL_FORMAT_I8:  \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_I8;  \
                                                        }   \
                                                        break;  \
                                                        case E_MI_RGN_PIXEL_FORMAT_RGB565:  \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_RGB565;  \
                                                        }   \
                                                        break;  \
                                                        case E_MI_RGN_PIXEL_FORMAT_ARGB8888:  \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_ARGB8888;  \
                                                        }   \
                                                        break;  \
                                                        default:    \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_MAX; \
                                                        }   \
                                                        break;  \
                                                    }   \
                                               }while (0);
#define MI_RGN_MAP_PIXFORMAT_TO_GFX(gfx, mi) do{  \
                                                       switch (mi) \
                                                       {   \
                                                           case E_MI_RGN_PIXEL_FORMAT_ARGB1555:    \
                                                           {   \
                                                               gfx = E_MI_GFX_FMT_ARGB1555;    \
                                                           }   \
                                                           break;  \
                                                           case E_MI_RGN_PIXEL_FORMAT_ARGB4444:    \
                                                           {   \
                                                               gfx = E_MI_GFX_FMT_ARGB4444;    \
                                                           }   \
                                                           break;  \
                                                           case E_MI_RGN_PIXEL_FORMAT_I2:  \
                                                           {  \
                                                               gfx = E_MI_GFX_FMT_I8;  \
                                                           }   \
                                                           break;  \
                                                           case E_MI_RGN_PIXEL_FORMAT_I4:  \
                                                           {   \
                                                               gfx = E_MI_GFX_FMT_I8;  \
                                                           }   \
                                                           break;  \
                                                           case E_MI_RGN_PIXEL_FORMAT_I8:  \
                                                           {   \
                                                               gfx = E_MI_GFX_FMT_I8;  \
                                                           }   \
                                                           break;  \
                                                           case E_MI_RGN_PIXEL_FORMAT_RGB565:  \
                                                           {   \
                                                               gfx = E_MI_GFX_FMT_RGB565;  \
                                                           }   \
                                                           break;  \
                                                           case E_MI_RGN_PIXEL_FORMAT_ARGB8888:  \
                                                           {   \
                                                               gfx = E_MI_GFX_FMT_ARGB8888;  \
                                                           }   \
                                                           break;  \
                                                           default:    \
                                                           {   \
                                                               gfx = E_MI_GFX_FMT_MAX; \
                                                           }   \
                                                           break;  \
                                                       }   \
                                                                                                       }while (0);

#define MI_RGN_MAP_GOPID_TO_MHAL(mhalGopId, miPortId, miModeId) do{ \
                                                                    if (E_MI_RGN_MODID_DIVP == miModeId) \
                                                                    { \
                                                                        switch (miPortId) \
                                                                        { \
                                                                            case 0: \
                                                                                mhalGopId = E_MHAL_GOP_DIVP_PORT0; \
                                                                                break; \
                                                                            default: \
                                                                                mhalGopId = E_MHAL_GOP_TYPE_MAX; \
                                                                                break; \
                                                                        } \
                                                                    } \
                                                                    else if (E_MI_RGN_MODID_VPE == miModeId) \
                                                                    { \
                                                                        switch (miPortId) \
                                                                        { \
                                                                            case 0: \
                                                                                mhalGopId = E_MHAL_GOP_VPE_PORT0; \
                                                                                break; \
                                                                            case 1: \
                                                                                mhalGopId = E_MHAL_GOP_VPE_PORT1; \
                                                                                break; \
                                                                            case 2: \
                                                                                mhalGopId = E_MHAL_GOP_VPE_PORT2; \
                                                                                break; \
                                                                            case 3: \
                                                                                mhalGopId = E_MHAL_GOP_VPE_PORT3; \
                                                                                break; \
                                                                            default: \
                                                                                mhalGopId = E_MHAL_GOP_TYPE_MAX; \
                                                                                break; \
                                                                        } \
                                                                    } \
                                                                }while(0);

#define MI_RGN_MAP_GWINID_TO_MHAL(mhalGwinId, miGwinId)  do{ \
                                                                switch(miGwinId) \
                                                                { \
                                                                    case 0:\
                                                                        mhalGwinId = E_MHAL_GOP_GWIN_ID_0; \
                                                                        break; \
                                                                    case 1: \
                                                                        mhalGwinId = E_MHAL_GOP_GWIN_ID_1; \
                                                                        break; \
                                                                    default: \
                                                                        mhalGwinId = E_MHAL_GOP_GWIN_ID_MAX; \
                                                                        break; \
                                                                } \
                                                            }while(0);

#define MI_RGN_MAP_COVERTYPE_TO_MHAL(mhalCoverType, miPortId, miModeId) do{ \
                                                                            if (E_MI_RGN_MODID_DIVP == miModeId) \
                                                                            { \
                                                                                switch (miPortId) \
                                                                                { \
                                                                                    case 0: \
                                                                                        mhalCoverType = E_MHAL_COVER_DIVP_PORT0; \
                                                                                        break; \
                                                                                    default: \
                                                                                        mhalCoverType = E_MHAL_COVER_TYPE_MAX; \
                                                                                        break; \
                                                                                } \
                                                                            } \
                                                                            else if (E_MI_RGN_MODID_VPE == miModeId) \
                                                                            { \
                                                                                switch (miPortId) \
                                                                                { \
                                                                                    case 0: \
                                                                                        mhalCoverType = E_MHAL_COVER_VPE_PORT0; \
                                                                                        break; \
                                                                                    case 1: \
                                                                                        mhalCoverType = E_MHAL_COVER_VPE_PORT1; \
                                                                                        break; \
                                                                                    case 2: \
                                                                                        mhalCoverType = E_MHAL_COVER_VPE_PORT2; \
                                                                                        break; \
                                                                                    case 3: \
                                                                                        mhalCoverType = E_MHAL_COVER_VPE_PORT3; \
                                                                                        break; \
                                                                                    default: \
                                                                                        mhalCoverType = E_MHAL_COVER_TYPE_MAX; \
                                                                                        break; \
                                                                                } \
                                                                            } \
                                                                        }while(0);

 typedef enum
{
    E_MI_RGN_BUF_WORKTYPE_FILL,
    E_MI_RGN_BUF_WORKTYPE_BLIT,
    E_MI_RGN_BUF_WORKTYPE_NULL
}MI_RGN_BufWorkType_e;

typedef struct MI_RGN_FrontBufferInfo_s
{
    MI_U8 u8Idx;
    MI_U8 u8OsdCnt;
    MI_RGN_CanvasInfo_t stFront;
    MI_RGN_Point_t stPoint;
    MI_BOOL bShow;
    MI_U64 u64MagicNum;
}MI_RGN_FrontBufferInfo_t;

typedef struct MI_RGN_ChPorHeadData_s
{
    struct list_head chPortAttachedOverlayList;
    struct list_head chPortAttachedCoverList;

    MI_U16 u16CoverCnt;
    MI_U16 u16OverlayCnt;
    MI_RGN_Size_t stBaseWindow;
    MI_RGN_FrontBufferInfo_t astFrontBufferInfo[MI_RGN_MAX_GOP_GWIN_NUM];
}MI_RGN_ChPorHeadData_t;

typedef struct MI_RGN_ChPortHead_s
{
    MI_RGN_ChPorHeadData_t astVpeChPortHeadData[MI_RGN_VPE_MAX_CH_NUM][MI_RGN_VPE_PORT_MAXNUM];
    MI_RGN_ChPorHeadData_t astDivpChPortHeadData[MI_RGN_DIVP_MAX_CH_NUM][MI_RGN_DIVP_PORT_MAXNUM];
}MI_RGN_ChPortHead_t;

typedef struct MI_RGN_ChPortListData_s
{
    struct list_head chPortList;

    MI_RGN_FrontBufferInfo_t *pstLinkedFrontBuffer;

    struct list_head chPortListTmp;

    MI_RGN_ChnPort_t stAttachedChnPort;
    MI_RGN_ChnPortParam_t stAttachedChnPortPara;

    MI_RGN_Attr_t stRgnAttr;
    MI_RGN_CanvasInfo_t stBack;
}MI_RGN_ChPortListData_t;

typedef struct MI_RGN_BufWorkListHead_s
{
    struct list_head bufDelWorkList;
    struct workqueue_struct *pStBufOptWorkQueue;
    MI_U64 u64Fence;
}MI_RGN_BufWorkListHead_t;

typedef struct MI_RGN_TemiBufWorkData_s
{
    MI_U64 u64Fence;
    struct work_struct stTemiDelWork;
 }MI_RGN_TemiBufWorkData_t;

typedef struct MI_RGN_BufferDelWorkData_s
{
    struct delayed_work bufDelTimeOutWork;
    struct list_head bufDelWorkList;
    MI_RGN_CanvasInfo_t stBuffer;
    MI_U64 u64Fence;
 }MI_RGN_BufferDelWorkListData_t;
#if MI_RGN_BUF_WQ
typedef struct MI_RGN_BufferBlitWorkData_s
{
    MI_RGN_CanvasInfo_t stCanvasInfoFrom;
    MI_RGN_Point_t stPoint;
}MI_RGN_BufferBlitWorkData_t;

typedef struct MI_RGN_BufferFillWorkData_s
{
    MI_RGN_Size_t stRectSize;
    MI_RGN_Point_t stRectPos;
    MI_U32 u32ColorVal;
}MI_RGN_BufferFillWorkData_t;
typedef struct MI_RGN_BufferWorkData_s
{
    MI_RGN_BufWorkType_e eWorkType;
    MI_RGN_FrontBufferInfo_t *pstFrontBuffer;
    MI_U64 u64MagicNum;//Decide the frontbuffer is avaliable or not.
    struct work_struct bufWork;
    union
    {
        MI_RGN_BufferBlitWorkData_t stBlitWork;
        MI_RGN_BufferFillWorkData_t stFillWork;
    };
}MI_RGN_BufferWorkData_t;
#endif
static MI_RGN_ChPortHead_t _gstChPortHead;
DEFINE_SEMAPHORE(_gstRgnDrvWmSem);
DEFINE_SEMAPHORE(_gstRgnDrvbufDelWorkListSem);
static MI_RGN_PaletteTable_t _stPaletteTable;
static MI_RGN_BufWorkListHead_t _stBufWorkListHead;
#if MI_RGN_ENABLE_COLOR_KEY
static MI_BOOL _gbConfigColor[E_MHAL_GOP_TYPE_MAX][E_MI_RGN_PIXEL_FORMAT_MAX];
#endif
extern void mi_rgn_MemAllocCnt(void *pAddr, MI_U32 u32Size, MI_BOOL bMode);
extern void mi_rgn_MemFreeCnt(void *pAddr, MI_BOOL bMode);

static MI_U32 _mi_rgn_drv_calpixstride(MI_RGN_PixelFormat_e ePixelFmt,  MI_RGN_Size_t *pstSize)
{
    /*Calculate stride by pix format & w/h*/
    /*align = 128/8/byteperpixel
     *E_HAL_REG_PIXEL_FORMAT_RGB_1555 , =>128/8/2= 8 pixel  align
     *E_HAL_REG_PIXEL_FORMAT_RGB_4444,  =>128/8/2= 8 align
     *E_HAL_REG_PIXEL_FORMAT_RGB_I2,       =>128/8*4= 64 pixel  align
     *E_HAL_REG_PIXEL_FORMAT_RGB_I4        =>128/8*2 =32 pixel  align
     *For exsample :
     *Color Format : I2
     *Width=1080, Height=800
     *Stride is by 16 bytes align which caculated as 1088.
     */
    MI_U32 u32PixStrideBytes = 0;
    MI_U32 u32RetBytes = 0;
    MHAL_RGN_GopPixelFormat_e eMhalPixFmt;

    MI_RGN_PTR_CHECK(pstSize, MI_SYS_BUG_ON(TRUE););

    MI_RGN_MAP_PIXFORMAT_TO_MHAL(eMhalPixFmt, ePixelFmt);
    MI_SYS_BUG_ON(eMhalPixFmt == E_MHAL_RGN_PIXEL_FORMAT_MAX);
    u32PixStrideBytes = (MI_U32)MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT, eMhalPixFmt);


    switch (ePixelFmt)
    {
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
            u32RetBytes = ALIGN_UP(pstSize->u32Width * 2, u32PixStrideBytes);
            break;
        case E_MI_RGN_PIXEL_FORMAT_I2:
            u32RetBytes = ALIGN_UP(pstSize->u32Width / 4, u32PixStrideBytes);
            break;
        case E_MI_RGN_PIXEL_FORMAT_I4:
            u32RetBytes = ALIGN_UP(pstSize->u32Width / 2, u32PixStrideBytes);
            break;
        case E_MI_RGN_PIXEL_FORMAT_I8:
            u32RetBytes = ALIGN_UP(pstSize->u32Width, u32PixStrideBytes);
            break;
        case E_MI_RGN_PIXEL_FORMAT_RGB565:
            u32RetBytes = ALIGN_UP(pstSize->u32Width * 2, u32PixStrideBytes);
            break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB8888:
            u32RetBytes = ALIGN_UP(pstSize->u32Width * 4, u32PixStrideBytes);
            break;
        default:
            MI_SYS_BUG();
    }

    return u32RetBytes;
}

static MI_S32 _mi_rgn_drv_get_capability(mi_rgn_DrvCapability_t *pstCap)
{
    MI_U8 i = 0;
    MHAL_RGN_GopPixelFormat_e eMhalPixFmt = E_MHAL_RGN_PIXEL_FORMAT_MAX;

    MI_SYS_BUG_ON(!pstCap);
    for (i = 0; i < E_MI_RGN_PIXEL_FORMAT_MAX; i++)
    {
        MI_RGN_MAP_PIXFORMAT_TO_MHAL(eMhalPixFmt, i);
        pstCap->bSupportFormat[i] = (MI_BOOL)MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT, eMhalPixFmt);
    }
    pstCap->u16OsdMaxWidth = MI_RGN_OVERLAY_MAX_WIDTH;
    pstCap->u16OsdMinWidth = MI_RGN_OVERLAY_MIN_WIDTH;
    pstCap->u16OsdMaxHeight = MI_RGN_OVERLAY_MAX_HEIGHT;
    pstCap->u16OsdMinHeight = MI_RGN_OVERLAY_MIN_HEIGHT;
    pstCap->u16CoverMaxHeight = MI_RGN_COVER_MAX_HEIGHT;
    pstCap->u16CoverMinHeight = MI_RGN_COVER_MIN_HEIGHT;
    pstCap->u16CoverMaxWidth = MI_RGN_COVER_MAX_WIDTH;
    pstCap->u16CoverMinWidth = MI_RGN_COVER_MIN_WIDTH;
    pstCap->u8AttachedCoverCnt = MI_RGN_COVER_MAX_NUM;
    pstCap->u8AttachedOsdCnt = MI_RGN_OSD_MAX_NUM;
    pstCap->u8VpeMaxChannelNum = MI_RGN_VPE_MAX_CH_NUM;
    pstCap->u8VpeMaxPortNum = MI_RGN_VPE_PORT_MAXNUM;
    pstCap->u8DivpMaxChannelNum = MI_RGN_DIVP_MAX_CH_NUM;
    pstCap->u8DivpMaxPortNum = MI_RGN_DIVP_PORT_MAXNUM;
    pstCap->u8OsdHwModeCnt = (MI_U32)MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_HW_GWIN_CNT, 0);
    pstCap->u8OsdWidthAlignment = MI_RGN_OSD_WIDTH_ALIGN;
    pstCap->u8OsdI2WidthAlignment = MI_RGN_OSD_I2_WIDTH_ALIGN;
    return MI_RGN_OK;
}

static MI_BOOL _mi_rgn_drv_check_canvasinfo_empty( MI_RGN_CanvasInfo_t *pstBuffer)
{
    /*Empty : return TRUE else return FALSE*/
    if (pstBuffer->phyAddr == 0 &&    \
        pstBuffer->stSize.u32Height == 0 &&     \
        pstBuffer->stSize.u32Width == 0 &&      \
        pstBuffer->u32Stride == 0 &&    \
        pstBuffer->virtAddr == 0 &&     \
        pstBuffer->phyAddr == 0)
    {
        return TRUE;
    }

    return FALSE;
}
static MI_S32 _mi_rgn_drv_get_chport_list_head(MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChPorHeadData_t **ppstChPortListHead)
{

    MI_RGN_PTR_CHECK(pstChnPort, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(ppstChPortListHead, MI_SYS_BUG_ON(TRUE););

    *ppstChPortListHead = NULL;
    switch (pstChnPort->eModId)
    {
        case E_MI_RGN_MODID_VPE:
        {
            MI_RGN_RANGE_CHECK(pstChnPort->s32ChnId, 0, MI_RGN_VPE_MAX_CH_NUM - 1, return MI_ERR_RGN_ILLEGAL_PARAM;);
            MI_RGN_RANGE_CHECK(pstChnPort->s32OutputPortId, 0, MI_RGN_VPE_PORT_MAXNUM - 1, return MI_ERR_RGN_ILLEGAL_PARAM;);
            *ppstChPortListHead = &_gstChPortHead.astVpeChPortHeadData[pstChnPort->s32ChnId][pstChnPort->s32OutputPortId];
        }
        break;
        case E_MI_RGN_MODID_DIVP:

        {
            MI_RGN_RANGE_CHECK(pstChnPort->s32ChnId, 0, MI_RGN_DIVP_MAX_CH_NUM - 1, return MI_ERR_RGN_ILLEGAL_PARAM;);
            MI_RGN_RANGE_CHECK(pstChnPort->s32OutputPortId, 0, MI_RGN_DIVP_PORT_MAXNUM - 1, return MI_ERR_RGN_ILLEGAL_PARAM;);
            *ppstChPortListHead = &_gstChPortHead.astDivpChPortHeadData[pstChnPort->s32ChnId][pstChnPort->s32OutputPortId];
        }
        break;
        default:
        {
            return MI_ERR_RGN_ILLEGAL_PARAM;
        }
        break;
    }

    return MI_RGN_OK;
}
static MI_S32 _mi_rgn_drv_get_frontbufferinfo(MI_U8 idx, MI_RGN_ChnPort_t *pstChnPort, MI_RGN_CanvasInfo_t *pstFrontBufInfo, MI_RGN_Point_t *pstPoint, MI_BOOL *bShow)
{
    MI_RGN_ChPorHeadData_t *pstCplh = NULL;
    MI_S32 s32ErrorCode;

    MI_SYS_BUG_ON(!pstChnPort);
    MI_SYS_BUG_ON(!pstFrontBufInfo);
    MI_SYS_BUG_ON(!pstPoint);
    MI_SYS_BUG_ON(!bShow);
    MI_SYS_BUG_ON(idx > MI_RGN_MAX_GOP_GWIN_NUM);

    down(&_gstRgnDrvWmSem);
    s32ErrorCode = _mi_rgn_drv_get_chport_list_head(pstChnPort, &pstCplh);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("Get ch port List fail.\n");
        up(&_gstRgnDrvWmSem);
        return MI_ERR_RGN_ILLEGAL_PARAM;
    }
    memcpy(pstFrontBufInfo, &pstCplh->astFrontBufferInfo[idx].stFront, sizeof(MI_RGN_CanvasInfo_t));
    memcpy(pstPoint, &pstCplh->astFrontBufferInfo[idx].stPoint, sizeof(MI_RGN_Point_t));
    *bShow = pstCplh->astFrontBufferInfo[idx].bShow;

    up(&_gstRgnDrvWmSem);

    return MI_RGN_OK;
}
static MI_S32 _mi_rgn_drv_get_basewindow(MI_RGN_ChnPort_t *pstChnPort, MI_RGN_Size_t *pstBaseWin)
{
    MI_S32 s32ErrorCode;
    MI_RGN_ChPorHeadData_t *pstCplh = NULL;

    MI_SYS_BUG_ON(!pstChnPort);
    MI_SYS_BUG_ON(!pstBaseWin);

    memset(pstBaseWin, 0, sizeof(MI_RGN_Size_t));
    down(&_gstRgnDrvWmSem);
    s32ErrorCode = _mi_rgn_drv_get_chport_list_head(pstChnPort, &pstCplh);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("Get ch port List fail.\n");
        up(&_gstRgnDrvWmSem);
        return MI_ERR_RGN_ILLEGAL_PARAM;
    }
    memcpy(pstBaseWin, &pstCplh->stBaseWindow, sizeof(MI_RGN_Size_t));

    up(&_gstRgnDrvWmSem);

    return MI_RGN_OK;

}
#define MI_RGN_CHECK_OVERLAP(pointALT, pointARB, pointBLT, pointBRB) ((pointALT.u32X <= pointBRB.u32X) && (pointALT.u32Y <= pointBRB.u32Y))     \
                                                    && ((pointARB.u32X >= pointBLT.u32X) && (pointARB.u32Y >= pointBLT.u32Y))
static MI_BOOL _mi_rgn_drv_check_osdoverlap(MI_RGN_ChPorHeadData_t *pstChPortListHead, MI_RGN_ChPortListData_t *pstChPortListData)
{
    MI_RGN_ChPortListData_t *pos;
    MI_RGN_Point_t stPointALT, stPointARB, stPointBLT, stPointBRB;


    MI_RGN_PTR_CHECK(pstChPortListHead, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChPortListData, MI_SYS_BUG_ON(TRUE););

    list_for_each_entry(pos, &pstChPortListHead->chPortAttachedOverlayList, chPortList)
    {
        MI_SYS_BUG_ON(pos->stRgnAttr.eType != E_MI_RGN_TYPE_OSD);
        if (pos != pstChPortListData)
        {
            stPointALT.u32X = pstChPortListData->stAttachedChnPortPara.stPoint.u32X;
            stPointALT.u32Y = pstChPortListData->stAttachedChnPortPara.stPoint.u32Y;
            stPointARB.u32X = pstChPortListData->stAttachedChnPortPara.stPoint.u32X + pstChPortListData->stRgnAttr.stOsdInitParam.stSize.u32Width - 1;
            stPointARB.u32Y = pstChPortListData->stAttachedChnPortPara.stPoint.u32Y + pstChPortListData->stRgnAttr.stOsdInitParam.stSize.u32Height - 1;
            stPointBLT.u32X = pos->stAttachedChnPortPara.stPoint.u32X;
            stPointBLT.u32Y = pos->stAttachedChnPortPara.stPoint.u32Y;
            stPointBRB.u32X = pos->stAttachedChnPortPara.stPoint.u32X + pos->stRgnAttr.stOsdInitParam.stSize.u32Width - 1;
            stPointBRB.u32Y = pos->stAttachedChnPortPara.stPoint.u32Y + pos->stRgnAttr.stOsdInitParam.stSize.u32Height - 1;
            if (MI_RGN_CHECK_OVERLAP(stPointALT, stPointARB, stPointBLT, stPointBRB))
            {
                DBG_ERR("Overlap A:\nLeft Top x: %d y: %d A: Right Bottom x: %d y: %d\n", stPointALT.u32X, stPointALT.u32Y, stPointARB.u32X, stPointARB.u32Y);
                DBG_ERR("Overlap B:\nLeft Top x: %d y: %d B: Right Bottom x: %d y: %d\n", stPointBLT.u32X, stPointBLT.u32Y, stPointBRB.u32X, stPointBRB.u32Y);
                return TRUE;
            }
        }
    }

    return FALSE;
}
static MI_S32 _mi_rgn_drv_buf_fill(MI_RGN_CanvasInfo_t *pstSurface,  MI_RGN_Size_t *pstRectSize, MI_RGN_Point_t *pstRectPos, MI_U32 u32ColorVal)
{
    MI_S32 s32Error = MI_RGN_OK;
#if (MI_RGN_BLIT_BY_SW)
    MI_U16 i = 0, j = 0;
    MI_U8 *pAddrTo = NULL;
    MI_U32 u32CopyBytes = 0;
//Crop
    MI_U32 u32MinCopyWidth = 0;
    MI_U32 u32MinCopyHeight = 0;
//i2 i4 i8
    MI_U32 u32Bits;
    MI_U32 u32LeftGapBits = 0;
    MI_U32 u32RightGapBits = 0;
    MI_U8 u8LeftGapVal = 0;
    MI_U8 u8RightGapVal = 0;
    MI_U32 u32GapPixs = 0;
    MI_U8 u8ColorVal = 0;

//1555 4444 565
    MI_U16 u16Color = 0;
    MI_U16 *pu16AddrTo = NULL;
    MI_U32 u32CopyTwoBytes = 0;
//8888
    MI_U32 u32Color = 0;
    MI_U32 *pu32AddrTo = NULL;
    MI_U32 u32CopyFourBytes = 0;

#else
    MI_GFX_Surface_t stSurface;
    MI_GFX_Rect_t stRect;
    MI_U16 u16Fence;
#endif

    MI_SYS_BUG_ON(!pstSurface);
    MI_SYS_BUG_ON(!pstRectSize);
    MI_SYS_BUG_ON(!pstRectPos);
    MI_SYS_BUG_ON(!pstSurface->phyAddr);
    MI_SYS_BUG_ON(_mi_rgn_drv_check_canvasinfo_empty(pstSurface));
    if (pstRectSize->u32Width + pstRectPos->u32X > pstSurface->stSize.u32Width
        || pstRectSize->u32Height + pstRectPos->u32Y > pstSurface->stSize.u32Height)
    {
        DBG_INFO("Buf fill size over range!!!Surface w: %d h: %d fill pos x: %d y: %d size w: %d h: %d\n",
            pstSurface->stSize.u32Width, pstSurface->stSize.u32Height,
            pstRectPos->u32X, pstRectPos->u32Y,
            pstRectSize->u32Width, pstRectSize->u32Height);
        //MI_SYS_BUG_ON(TRUE);
    }

#if (MI_RGN_BLIT_BY_SW)
    if (pstRectPos->u32X < pstSurface->stSize.u32Width && pstRectPos->u32Y < pstSurface->stSize.u32Height)
    {
        pstSurface->virtAddr = (MI_VIRT)mi_sys_Vmap(pstSurface->phyAddr, pstSurface->u32Stride * pstSurface->stSize.u32Height, FALSE);
        if (0 == pstSurface->virtAddr)
        {
            DBG_EXIT_ERR("Sys map error!\n");
            return MI_ERR_RGN_BADADDR;
        }
        MI_RGN_BITS_PER_PIXEL(u32Bits, pstRectPos->u32X, pstSurface->ePixelFmt);
        u32LeftGapBits = ALIGN_UP(u32Bits, 8) - u32Bits;
        pAddrTo = (MI_U8 *)(pstSurface->virtAddr + (u32Bits + u32LeftGapBits)/8 + pstSurface->u32Stride * pstRectPos->u32Y);
        u32MinCopyWidth = (pstRectPos->u32X + pstRectSize->u32Width <= pstSurface->stSize.u32Width) ? pstRectSize->u32Width : (pstSurface->stSize.u32Width - pstRectPos->u32X);
        MI_RGN_BITS_PER_PIXEL(u32Bits, u32MinCopyWidth, pstSurface->ePixelFmt);
        u32Bits -= u32LeftGapBits;
        u32RightGapBits = u32Bits - ALIGN_DOWN(u32Bits, 8);
        u32CopyBytes =(u32Bits - u32RightGapBits)/8;
        u32MinCopyHeight = (pstRectPos->u32Y + pstRectSize->u32Height <= pstSurface->stSize.u32Height) ? pstRectSize->u32Height : (pstSurface->stSize.u32Height - pstRectPos->u32Y);
        switch (pstSurface->ePixelFmt)
        {
            case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
            {
                u16Color = ((u32ColorVal & 0xF8) >> 3) | ((u32ColorVal & 0xF800) >> 6) | ((u32ColorVal & 0xF80000) >> 9) | ((u32ColorVal & 0x80000000) >> 16);
                MI_SYS_BUG_ON(u32CopyBytes%2);
                u32CopyTwoBytes = u32CopyBytes/2;
                pu16AddrTo = (MI_U16 *)pAddrTo;
                for (i = 0; i < u32MinCopyHeight; i++)
                {
                    for (j = 0; j < u32CopyTwoBytes; j++)
                    {
                       pu16AddrTo[j] = u16Color;
                    }
                    pAddrTo += pstSurface->u32Stride;
                    pu16AddrTo = (MI_U16 *)pAddrTo;
                }
            }
            break;
            case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
            {
                u16Color = ((u32ColorVal & 0xF0) >> 4) | ((u32ColorVal & 0xF000) >> 8) | ((u32ColorVal & 0xF00000) >> 12) | ((u32ColorVal & 0xF0000000) >> 16);
                MI_SYS_BUG_ON(u32CopyBytes%2);
                u32CopyTwoBytes = u32CopyBytes/2;
                pu16AddrTo = (MI_U16 *)pAddrTo;
                for (i = 0; i < u32MinCopyHeight; i++)
                {
                    for (j = 0; j < u32CopyTwoBytes; j++)
                    {
                       pu16AddrTo[j] = u16Color;
                    }
                    pAddrTo += pstSurface->u32Stride;
                    pu16AddrTo = (MI_U16 *)pAddrTo;
                }
            }
            break;
            case E_MI_RGN_PIXEL_FORMAT_RGB565:
            {
                u16Color = ((u32ColorVal & 0xF8) >> 3) | ((u32ColorVal & 0xFC00) >> 5) | ((u32ColorVal & 0xF80000) >> 8);
                MI_SYS_BUG_ON(u32CopyBytes%2);
                u32CopyTwoBytes = u32CopyBytes/2;
                pu16AddrTo = (MI_U16 *)pAddrTo;
                for (i = 0; i < u32MinCopyHeight; i++)
                {
                    for (j = 0; j < u32CopyTwoBytes; j++)
                    {
                       pu16AddrTo[j] = u16Color;
                    }
                    pAddrTo += pstSurface->u32Stride;
                    pu16AddrTo = (MI_U16 *)pAddrTo;
                }
            }
            break;
            case E_MI_RGN_PIXEL_FORMAT_ARGB8888:
            {
                u32Color = u32ColorVal;
                MI_SYS_BUG_ON(u32CopyBytes%4);
                u32CopyFourBytes = u32CopyBytes/4;
                pu32AddrTo = (MI_U32 *)pAddrTo;
                for (i = 0; i < u32MinCopyHeight; i++)
                {
                    for (j = 0; j < u32CopyFourBytes; j++)
                    {
                       pu32AddrTo[j] = u32Color;
                    }
                    pAddrTo += pstSurface->u32Stride;
                    pu32AddrTo = (MI_U32 *)pAddrTo;
                }
            }
            break;
            case E_MI_RGN_PIXEL_FORMAT_I2:
            case E_MI_RGN_PIXEL_FORMAT_I4:
            case E_MI_RGN_PIXEL_FORMAT_I8:
            {
                if (u32LeftGapBits)
                {
                    MI_RGN_BITS_TO_PIXEL(u32GapPixs, u32LeftGapBits, pstSurface->ePixelFmt);
                    u8ColorVal = (MI_U8)u32ColorVal;
                    for (i = 0; i < u32GapPixs; i++)
                    {
                        u8ColorVal |= u8ColorVal << (i * (u32LeftGapBits/u32GapPixs));
                    }
                    u8LeftGapVal = (MI_U8)((u8ColorVal << (8 - u32LeftGapBits)) & 0xFF);
                }
                if (u32RightGapBits)
                {
                    MI_RGN_BITS_TO_PIXEL(u32GapPixs, u32RightGapBits, pstSurface->ePixelFmt);
                    u8ColorVal = (MI_U8)u32ColorVal;
                    for (i = 0; i < u32GapPixs; i++)
                    {
                        u8RightGapVal |= u8ColorVal << (i * (u32RightGapBits/u32GapPixs));
                    }
                }
                MI_RGN_BITS_TO_PIXEL(u32GapPixs, 8, pstSurface->ePixelFmt);
                u8ColorVal = (MI_U8)u32ColorVal;
                for (i = 0; i < u32GapPixs; i++)
                {
                    u8ColorVal |= u8ColorVal << (i * (16/u32GapPixs));
                }
                for (i = 0; i < u32MinCopyHeight; i++)
                {
                    if (u32LeftGapBits)
                    {
                        *(pAddrTo - 1) = u8LeftGapVal;
                    }
                    if (u32RightGapBits)
                    {
                        pAddrTo[u32CopyBytes] = u8LeftGapVal;
                    }
                    memset(pAddrTo, u8ColorVal, u32CopyBytes);
                    pAddrTo += pstSurface->u32Stride;
                }
            }
            break;
            default:
            {
                MI_SYS_BUG_ON(TRUE);
            }
        }
        mi_sys_UnVmap((void *)pstSurface->virtAddr);
        pstSurface->virtAddr = 0;
    }
#else

    MI_RGN_MAP_PIXFORMAT_TO_GFX(stSurface.eColorFmt, pstSurface->ePixelFmt);
    stSurface.phyAddr = pstSurface->phyAddr;
    stSurface.u32Height = pstSurface->stSize.u32Height;
    stSurface.u32Stride = pstSurface->u32Stride;
    stRect.s32Ypos = pstRectPos->u32Y;
    stRect.u32Height = pstRectSize->u32Height;

    switch (pstSurface->ePixelFmt)
    {
        case E_MI_RGN_PIXEL_FORMAT_I2:
        {
            MI_SYS_BUG_ON(pstRectPos->u32X % 4);
            MI_SYS_BUG_ON(pstRectSize->u32Width % 4);
            MI_SYS_BUG_ON(pstSurface->stSize.u32Width % 4);
            stRect.s32Xpos = pstRectPos->u32X/4;
            stRect.u32Width = pstRectSize->u32Width/4;
            stSurface.u32Width = pstSurface->stSize.u32Width/4;
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_I4:
        {
            MI_SYS_BUG_ON(pstRectPos->u32X % 2);
            MI_SYS_BUG_ON(pstRectSize->u32Width % 2);
            MI_SYS_BUG_ON(pstSurface->stSize.u32Width % 4);
            stRect.s32Xpos = pstRectPos->u32X/2;
            stRect.u32Width = pstRectSize->u32Width/2;
            stSurface.u32Width = pstSurface->stSize.u32Width/2;
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
        case E_MI_RGN_PIXEL_FORMAT_I8:
        case E_MI_RGN_PIXEL_FORMAT_RGB565:
        case E_MI_RGN_PIXEL_FORMAT_ARGB8888:
        {
            stRect.s32Xpos = pstRectPos->u32X;
            stRect.u32Width = pstRectSize->u32Width;
            stSurface.u32Width = pstSurface->stSize.u32Width;
        }
        break;
        default:
            MI_SYS_BUG();
            break;
    }
    DBG_INFO("GFX Surface w: %d h: %d format : %d fill pos x: %d y: %d size w: %d h: %d color %x\n",
        stSurface.u32Width, stSurface.u32Height, stSurface.eColorFmt,
        stRect.s32Xpos, stRect.s32Ypos,
        stRect.u32Width, stRect.u32Height, u32ColorVal);

    if (MI_SUCCESS != MI_GFX_QuickFill(&stSurface, &stRect, u32ColorVal, &u16Fence))
    {
        DBG_EXIT_ERR("Gfx clear buf error!\n");
        s32Error = MI_ERR_RGN_BADADDR;
        goto ERROR_CODE;
    }

    MI_GFX_WaitAllDone(FALSE, u16Fence);
ERROR_CODE:
#endif
    return s32Error;
}
static MI_S32 _mi_rgn_drv_blit_buffer(MI_RGN_CanvasInfo_t *pstCanvasInfoTo, MI_RGN_CanvasInfo_t *pstCanvasInfoFrom, MI_RGN_Point_t *pstPos)
{

#if  MI_RGN_BLIT_BY_SW
    MI_U8 *pAddrTo = NULL;
    MI_U8 *pAddrFrom = NULL;
    MI_U32 *pu32AddrTo = NULL;
    MI_U32 *pu32AddrFrom = NULL;

    MI_U16 i = 0;
    MI_U16 j = 0;
    MI_U32 u32Height;
    MI_U32 u32StrideByteFrom = 0;
    MI_U32 u32StrideByteTo = 0;
    MI_U32 u32CopyBytes = 0;
    MI_U32 u32Copy4Bytes = 0;
    MI_U32 u32Copy4BytesLeft = 0;
    MI_U32 u32CopyBits = 0;
    MI_U32 u32OffsetBits = 0;
    MI_U32 u32OffsetBytes = 0;
    MI_U32 u32BitsFrom = 0;
    MI_U32 u32BitsTo = 0;
    MI_U32 u32LeftGapBits = 0;
    MI_U32 u32RightGapBits = 0;
#else
    MI_GFX_Surface_t stSurfaceTo;
    MI_GFX_Rect_t stRectTo;
    MI_GFX_Surface_t stSurfaceFrom;
    MI_GFX_Rect_t stRectFrom;
    MI_U16 u16Fence;
    MI_GFX_Opt_t stOpt;

#endif
    MI_RGN_PTR_CHECK(pstCanvasInfoFrom, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstCanvasInfoTo, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstPos, MI_SYS_BUG_ON(TRUE););
    MI_SYS_BUG_ON(_mi_rgn_drv_check_canvasinfo_empty(pstCanvasInfoTo));
    MI_SYS_BUG_ON(_mi_rgn_drv_check_canvasinfo_empty(pstCanvasInfoFrom));

    DBG_INFO("Blit from size w %d h %d, stride : %d to size w %d h %d, start pos x: %d y: %d stride : %d (va %lx,pa %llx) to (va %lx,pa %llx)\n", pstCanvasInfoFrom->stSize.u32Width, pstCanvasInfoFrom->stSize.u32Height, pstCanvasInfoFrom->u32Stride,    \
        pstCanvasInfoTo->stSize.u32Width, pstCanvasInfoTo->stSize.u32Height, pstPos->u32X, pstPos->u32Y, pstCanvasInfoTo->u32Stride,    \
        pstCanvasInfoFrom->virtAddr, pstCanvasInfoFrom->phyAddr, pstCanvasInfoTo->virtAddr, pstCanvasInfoTo->phyAddr);

#if MI_RGN_BLIT_BY_SW
    if (pstPos->u32Y < pstCanvasInfoTo->stSize.u32Height && pstPos->u32X < pstCanvasInfoTo->stSize.u32Width)
    {
        pstCanvasInfoFrom->virtAddr = (MI_VIRT)mi_sys_Vmap(pstCanvasInfoFrom->phyAddr, pstCanvasInfoFrom->u32Stride * pstCanvasInfoFrom->stSize.u32Height, FALSE);
        if (0 == pstCanvasInfoFrom->virtAddr)
        {
            DBG_EXIT_ERR("Sys map error!\n");
            return MI_ERR_RGN_BADADDR;
        }
        pstCanvasInfoTo->virtAddr = (MI_VIRT)mi_sys_Vmap(pstCanvasInfoTo->phyAddr, pstCanvasInfoTo->u32Stride * pstCanvasInfoTo->stSize.u32Height, FALSE);
        if (0 == pstCanvasInfoTo->virtAddr)
        {
            DBG_EXIT_ERR("Sys map error!\n");
            return MI_ERR_RGN_BADADDR;
        }
        u32Height = (pstCanvasInfoFrom->stSize.u32Height + pstPos->u32Y <= pstCanvasInfoTo->stSize.u32Height)?(pstCanvasInfoFrom->stSize.u32Height):(pstCanvasInfoTo->stSize.u32Height - pstPos->u32Y);
        u32StrideByteFrom = pstCanvasInfoFrom->u32Stride;
        u32StrideByteTo = pstCanvasInfoTo->u32Stride;

        MI_RGN_BITS_PER_PIXEL(u32OffsetBits, pstPos->u32X, pstCanvasInfoTo->ePixelFmt);
        u32LeftGapBits = ALIGN_UP(u32OffsetBits, 8) - u32OffsetBits;
        u32OffsetBytes = (u32OffsetBits + u32LeftGapBits)/8;
        pAddrTo = (MI_U8 *)(pstCanvasInfoTo->virtAddr + (pstPos->u32Y) * u32StrideByteTo + u32OffsetBytes);
        pAddrFrom = (MI_U8 *)pstCanvasInfoFrom->virtAddr;

        MI_RGN_BITS_PER_PIXEL(u32BitsFrom, pstCanvasInfoFrom->stSize.u32Width, pstCanvasInfoFrom->ePixelFmt);
        MI_RGN_BITS_PER_PIXEL(u32BitsTo, pstCanvasInfoTo->stSize.u32Width, pstCanvasInfoTo->ePixelFmt);
        u32CopyBits = (u32OffsetBits + u32BitsFrom <= u32BitsTo)?(u32BitsFrom):(u32BitsTo - u32OffsetBits);
        u32RightGapBits = u32CopyBits - ALIGN_DOWN(u32CopyBits, 8);
        u32CopyBytes = (u32CopyBits - u32RightGapBits)/8;

        DBG_INFO("Left gap %d right gap %d\n", u32LeftGapBits, u32RightGapBits);
        if ((!u32LeftGapBits) && (!u32RightGapBits))
        {
            for (i = 0; i < u32Height; i++)
            {
                memcpy(pAddrTo, pAddrFrom, u32CopyBytes);
                /* pAddrTo[j] = C-An  */
                pAddrTo += u32StrideByteTo;
                pAddrFrom += u32StrideByteFrom;
            }
        }
        else
        {
            for (i = 0; i < u32Height; i++)
            {

                 pAddrTo = u32LeftGapBits?(pAddrTo - 1):(pAddrTo);
                /*|A1 B1| A2 B2|...|An Bn| C to  A1 |B1 A2| B2 A3|...|Bn An| C-An*/
                *pAddrTo |= *pAddrFrom << (8 - u32LeftGapBits); //left gap A done.
                pAddrTo = u32LeftGapBits?(pAddrTo + 1):(pAddrTo);
                pu32AddrTo = (MI_U32 *)pAddrTo;
                pu32AddrFrom = (MI_U32 *)pAddrFrom;
                u32Copy4Bytes = u32CopyBytes/4;
                for (j = 0; j < u32Copy4Bytes; j++)
                {
                    pu32AddrTo[j] = (pu32AddrFrom[j] >> u32LeftGapBits) | (pu32AddrFrom[j + 1] << (32 - u32LeftGapBits));
                }
                u32Copy4BytesLeft = ALIGN_DOWN(u32CopyBytes, 4);
                for (j = u32Copy4BytesLeft; j < u32CopyBytes; j++)
                {
                    /*pAddrTo[j] = B | A*/
                    pAddrTo[j] = (pAddrFrom[j] >> u32LeftGapBits) | (pAddrFrom[j + 1] << (8 - u32LeftGapBits));
                }
                /* pAddrTo[j] = C-An  */
                pAddrTo[j] = (u32RightGapBits > u32LeftGapBits)?(pAddrFrom[j] >> u32LeftGapBits):(pAddrTo[j]); //Right Gap C
                pAddrTo += u32StrideByteTo;
                pAddrFrom += u32StrideByteFrom;
            }
        }
        mi_sys_UnVmap((void *)pstCanvasInfoTo->virtAddr);
        pstCanvasInfoTo->virtAddr = 0;
        mi_sys_UnVmap((void *)pstCanvasInfoFrom->virtAddr);
        pstCanvasInfoFrom->virtAddr = 0;
    }
#else
    BUG_ON(pstCanvasInfoTo->ePixelFmt != pstCanvasInfoFrom->ePixelFmt);
    MI_RGN_MAP_PIXFORMAT_TO_GFX(stSurfaceTo.eColorFmt, pstCanvasInfoTo->ePixelFmt);
    stSurfaceTo.phyAddr = pstCanvasInfoTo->phyAddr;
    stSurfaceTo.u32Height = pstCanvasInfoTo->stSize.u32Height;
    stSurfaceTo.u32Stride = pstCanvasInfoTo->u32Stride;
    stRectTo.s32Ypos = pstPos->u32Y;
    stRectTo.u32Height = pstCanvasInfoFrom->stSize.u32Height;
    MI_RGN_MAP_PIXFORMAT_TO_GFX(stSurfaceFrom.eColorFmt, pstCanvasInfoFrom->ePixelFmt);
    stSurfaceFrom.phyAddr = pstCanvasInfoFrom->phyAddr;
    stSurfaceFrom.u32Height = pstCanvasInfoFrom->stSize.u32Height;
    stSurfaceFrom.u32Stride = pstCanvasInfoFrom->u32Stride;
    stRectFrom.s32Xpos = 0;
    stRectFrom.s32Ypos = 0;
    stRectFrom.u32Height = pstCanvasInfoFrom->stSize.u32Height;

    switch (pstCanvasInfoTo->ePixelFmt)
    {
        case E_MI_RGN_PIXEL_FORMAT_I2:
        {
            MI_SYS_BUG_ON(pstPos->u32X % 4);
            MI_SYS_BUG_ON(pstCanvasInfoTo->stSize.u32Width % 4);
            MI_SYS_BUG_ON(pstCanvasInfoFrom->stSize.u32Width % 4);
            stSurfaceTo.u32Width = pstCanvasInfoTo->stSize.u32Width/4;
            stRectTo.s32Xpos = pstPos->u32X/4;
            stRectTo.u32Width = pstCanvasInfoFrom->stSize.u32Width/4;
            stSurfaceFrom.u32Width = pstCanvasInfoFrom->stSize.u32Width/4;
            stRectFrom.u32Width = stSurfaceFrom.u32Width;
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_I4:
        {
            MI_SYS_BUG_ON(pstPos->u32X % 2);
            MI_SYS_BUG_ON(pstCanvasInfoTo->stSize.u32Width % 2);
            MI_SYS_BUG_ON(pstCanvasInfoFrom->stSize.u32Width % 2);
            stSurfaceTo.u32Width = pstCanvasInfoTo->stSize.u32Width/2;
            stRectTo.s32Xpos = pstPos->u32X/2;
            stRectTo.u32Width = pstCanvasInfoFrom->stSize.u32Width/2;
            stSurfaceFrom.u32Width = pstCanvasInfoFrom->stSize.u32Width/2;
            stRectFrom.u32Width = stSurfaceFrom.u32Width;
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
        case E_MI_RGN_PIXEL_FORMAT_I8:
        case E_MI_RGN_PIXEL_FORMAT_RGB565:
        case E_MI_RGN_PIXEL_FORMAT_ARGB8888:
        {
            MI_SYS_BUG_ON(pstPos->u32X % 2);
            MI_SYS_BUG_ON(pstCanvasInfoTo->stSize.u32Width % 2);
            MI_SYS_BUG_ON(pstCanvasInfoFrom->stSize.u32Width % 2);
            stSurfaceTo.u32Width = pstCanvasInfoTo->stSize.u32Width;
            stRectTo.s32Xpos = pstPos->u32X;
            stRectTo.u32Width = pstCanvasInfoFrom->stSize.u32Width;
            stSurfaceFrom.u32Width = pstCanvasInfoFrom->stSize.u32Width;
            stRectFrom.u32Width = stSurfaceFrom.u32Width;
        }
        break;
        default:
            MI_SYS_BUG();
            break;
    }


    memset(&stOpt, 0, sizeof(MI_GFX_Opt_t));
    stOpt.bEnGfxRop = FALSE;
    stOpt.eRopCode = E_MI_GFX_ROP_NONE;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    DBG_INFO("GFX Src addr %llx, w %d, h %d, stride %d, rect x %d, y %d, h %d, w %d, Dst addr %llx, h %d, w %d, stride %d, rect x %d, y %d, h %d, w %d\n", stSurfaceFrom.phyAddr, stSurfaceFrom.u32Width, stSurfaceFrom.u32Height, stSurfaceFrom.u32Stride, stRectFrom.s32Xpos, stRectFrom.s32Ypos, stRectFrom.u32Height, stRectFrom.u32Width,    \
        stSurfaceTo.phyAddr, stSurfaceTo.u32Height, stSurfaceTo.u32Width, stSurfaceTo.u32Stride, stRectTo.s32Xpos, stRectTo.s32Ypos, stRectTo.u32Height, stRectTo.u32Width);

    if (MI_SUCCESS != MI_GFX_BitBlit(&stSurfaceFrom, &stRectFrom, &stSurfaceTo, &stRectTo, &stOpt, &u16Fence))
    {
        DBG_EXIT_ERR("Gfx BitBlit error!\n");
        return MI_ERR_RGN_NOMEM;
    }

    MI_GFX_WaitAllDone(FALSE, u16Fence);

#endif
    return MI_RGN_OK;
}
static MI_S32 _mi_rgn_drv_fill_all_alpha_data_by_sw(MI_RGN_CanvasInfo_t *pstBufferCfg)
{
    MI_U32 u32Size;

    MI_RGN_PTR_CHECK(pstBufferCfg, MI_SYS_BUG_ON(TRUE););

    u32Size = pstBufferCfg->u32Stride * pstBufferCfg->stSize.u32Height;
    pstBufferCfg->virtAddr = (MI_VIRT)mi_sys_Vmap(pstBufferCfg->phyAddr, u32Size, FALSE);
    if (0 == pstBufferCfg->virtAddr)
    {
        DBG_EXIT_ERR("Sys map error!\n");
        return MI_ERR_RGN_NOMEM;
    }

    switch (pstBufferCfg->ePixelFmt)
    {
        case E_MI_RGN_PIXEL_FORMAT_I2:
        case E_MI_RGN_PIXEL_FORMAT_I4:
        case E_MI_RGN_PIXEL_FORMAT_I8:
        {
            memset((MI_U8 *)pstBufferCfg->virtAddr, 0, u32Size); //default 0 for color key
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
        case E_MI_RGN_PIXEL_FORMAT_ARGB8888:
        {
#if (MI_RGN_ENABLE_COLOR_KEY)
            memset((MI_U8 *)pstBufferCfg->virtAddr, MI_RGN_COLOR_KEY_VALUE & 0xFF, u32Size);
#else
            memset((MI_U8 *)pstBufferCfg->virtAddr, MI_RGN_INIT_ALPHA_VALUE & 0xFF, u32Size);
#endif
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_RGB565:
        {
#if (MI_RGN_ENABLE_COLOR_KEY)
            memset((MI_U8 *)pstBufferCfg->virtAddr, MI_RGN_COLOR_KEY_VALUE & 0xFF, u32Size);
#else
            DBG_ERR("RGB565 not support !!");
            MI_SYS_BUG();
#endif
        }
        break;
        default:
            MI_SYS_BUG();
        break;
    }
    mi_sys_UnVmap((void *)pstBufferCfg->virtAddr);
    pstBufferCfg->virtAddr = 0;

   return MI_RGN_OK;
}
static MI_S32 _mi_rgn_drv_misys_buf_alloc(MI_RGN_CanvasInfo_t *pstBufferCfg)
{
    MI_U32 u32Size = 0;
    MI_S32 s32Error = MI_RGN_OK;

    MI_RGN_PTR_CHECK(pstBufferCfg, MI_SYS_BUG_ON(TRUE););

    pstBufferCfg->u32Stride = _mi_rgn_drv_calpixstride(pstBufferCfg->ePixelFmt, &(pstBufferCfg->stSize));
    u32Size = pstBufferCfg->u32Stride * pstBufferCfg->stSize.u32Height;
    MI_SYS_BUG_ON(u32Size == 0);
    pstBufferCfg->virtAddr = (MI_VIRT)0;
    pstBufferCfg->phyAddr = (MI_PHY)0;
    if (MI_SUCCESS != mi_sys_MMA_Alloc(NULL, u32Size, &pstBufferCfg->phyAddr))
    {
        s32Error = MI_ERR_RGN_NOMEM;
        goto ALLOC_ERROR;
    }
    s32Error = _mi_rgn_drv_fill_all_alpha_data_by_sw(pstBufferCfg);
    if (MI_RGN_OK != s32Error)
        goto MAP_ERROR;

    mi_rgn_MemAllocCnt((void *)((MI_U32)pstBufferCfg->phyAddr), u32Size, 0);

    return MI_RGN_OK;

MAP_ERROR:
    DBG_EXIT_ERR("Alloc buf error!\n");
    mi_sys_MMA_Free(pstBufferCfg->phyAddr);
    pstBufferCfg->virtAddr = (MI_VIRT)0;
    pstBufferCfg->phyAddr = (MI_PHY)0;
ALLOC_ERROR:

    return s32Error;
}
static MI_S32 _mi_rgn_drv_misys_buf_del(MI_RGN_CanvasInfo_t *pstBuffer)
{
    /*Use mi sys to do delete by the addr*/
    /*Fill the zero value*/

    MI_RGN_PTR_CHECK(pstBuffer, MI_SYS_BUG_ON(TRUE););

    mi_rgn_MemFreeCnt((void *)((MI_U32)pstBuffer->phyAddr), 0);
    if (MI_SUCCESS != mi_sys_MMA_Free(pstBuffer->phyAddr))
    {
        return MI_ERR_RGN_BADADDR;
    }
    memset(pstBuffer, 0, sizeof(MI_RGN_CanvasInfo_t));

    return MI_RGN_OK;
}
static void _mi_rgn_drv_teminate_buf_work(struct work_struct * pstWork)
{
    MI_RGN_TemiBufWorkData_t *pstTemibufWorkList = NULL;
    MI_RGN_BufferDelWorkListData_t *pos = NULL;
    MI_RGN_BufferDelWorkListData_t *posN = NULL;

    MI_RGN_PTR_CHECK(pstWork, MI_SYS_BUG_ON(TRUE););
    pstTemibufWorkList = container_of(pstWork, MI_RGN_TemiBufWorkData_t, stTemiDelWork);

    flush_workqueue(_stBufWorkListHead.pStBufOptWorkQueue);
    down(&_gstRgnDrvbufDelWorkListSem);
    list_for_each_entry(pos, &_stBufWorkListHead.bufDelWorkList, bufDelWorkList)
    {
        if (pstTemibufWorkList->u64Fence >= pos->u64Fence)
        {
            break;
        }
    }
    while (&pos->bufDelWorkList != &_stBufWorkListHead.bufDelWorkList)
    {
        posN = list_first_entry(&pos->bufDelWorkList, MI_RGN_BufferDelWorkListData_t, bufDelWorkList);
        /*cancel_delayed_work return ture means delay work still pending and cancel it successfully*/
        if (cancel_delayed_work(&pos->bufDelTimeOutWork))
        {
            MI_SYS_BUG_ON(MI_RGN_OK != _mi_rgn_drv_misys_buf_del(&pos->stBuffer));
            list_del(&pos->bufDelWorkList);
            MI_RGN_MEMFREE(pos);
        }
        pos = posN;
    }
    up(&_gstRgnDrvbufDelWorkListSem);
    MI_RGN_MEMFREE(pstTemibufWorkList);

}
static void _mi_rgn_drv_dequeue_buf_del_work(struct work_struct * pstWork)
{
    MI_RGN_BufferDelWorkListData_t *pstbufDelWorkList = NULL;

    MI_RGN_PTR_CHECK(pstWork, MI_SYS_BUG_ON(TRUE););

    pstbufDelWorkList = container_of(pstWork, MI_RGN_BufferDelWorkListData_t, bufDelTimeOutWork.work);
    MI_RGN_PTR_CHECK(pstbufDelWorkList, MI_SYS_BUG_ON(TRUE););
    DBG_INFO("pa %llx va %lx\n", pstbufDelWorkList->stBuffer.phyAddr, pstbufDelWorkList->stBuffer.virtAddr);
    MI_SYS_BUG_ON(MI_RGN_OK != _mi_rgn_drv_misys_buf_del(&pstbufDelWorkList->stBuffer));
    down(&_gstRgnDrvbufDelWorkListSem);
    list_del(&pstbufDelWorkList->bufDelWorkList);
    up(&_gstRgnDrvbufDelWorkListSem);
    MI_RGN_MEMFREE(pstbufDelWorkList);
}
static void _mi_rgn_drv_buf_enqueue_del_work(MI_RGN_CanvasInfo_t *pBuffer)
{
    MI_RGN_BufferDelWorkListData_t *pstbufDelWorkList;

    MI_RGN_PTR_CHECK(pBuffer, MI_SYS_BUG_ON(TRUE););
    MI_SYS_BUG_ON(_mi_rgn_drv_check_canvasinfo_empty(pBuffer));
    MI_RGN_MEMALLOC(pstbufDelWorkList, sizeof(MI_RGN_BufferDelWorkListData_t));
    MI_SYS_BUG_ON(pstbufDelWorkList == NULL);
    memset(pstbufDelWorkList, 0, sizeof(MI_RGN_BufferDelWorkListData_t));
    memcpy(&pstbufDelWorkList->stBuffer, pBuffer, sizeof(MI_RGN_CanvasInfo_t));
    memset(pBuffer, 0, sizeof(MI_RGN_CanvasInfo_t));
    INIT_DELAYED_WORK(&pstbufDelWorkList->bufDelTimeOutWork, _mi_rgn_drv_dequeue_buf_del_work);
    pstbufDelWorkList->u64Fence = _stBufWorkListHead.u64Fence;
    DBG_INFO("pa %llx va %lx\n", pstbufDelWorkList->stBuffer.phyAddr, pstbufDelWorkList->stBuffer.virtAddr);
    queue_delayed_work(_stBufWorkListHead.pStBufOptWorkQueue, &pstbufDelWorkList->bufDelTimeOutWork, msecs_to_jiffies(MI_RGN_BUF_DEL_WORK_TIMEOUT));
    down(&_gstRgnDrvbufDelWorkListSem);
    list_add(&pstbufDelWorkList->bufDelWorkList, &_stBufWorkListHead.bufDelWorkList);
    up(&_gstRgnDrvbufDelWorkListSem);

    return;
}

static void _mi_rgn_drv_blit_overlay(MI_U8 u8GfxColorFmtTo, MI_RGN_CanvasInfo_t *pstCanvasTo, MI_RGN_ChnPort_t *pstChnPort, MI_U8 u8Index)
{
#if (INTERFACE_GFX == 1)
    MI_GFX_Surface_t stSurfaceTo;
    MI_GFX_Rect_t stRectTo;
    MI_GFX_Surface_t stSurfaceFrom;
    MI_GFX_Rect_t stRectFrom;
    MI_U16 u16Fence;
    MI_GFX_Opt_t stOpt;
    MI_RGN_CanvasInfo_t stCanvasFrom;
    MI_RGN_Point_t stPoint;
    MI_BOOL bShow = TRUE;
    memset(&stCanvasFrom, 0, sizeof(stCanvasFrom));
    memset(&stPoint, 0, sizeof(stPoint));

    _mi_rgn_drv_get_frontbufferinfo(u8Index, pstChnPort, &stCanvasFrom, &stPoint, &bShow);

    if (!stCanvasFrom.phyAddr)
        return;

    MI_SYS_BUG_ON(_mi_rgn_drv_check_canvasinfo_empty(&stCanvasFrom));
    MI_SYS_BUG_ON(_mi_rgn_drv_check_canvasinfo_empty(pstCanvasTo));

    MI_RGN_MAP_PIXFORMAT_TO_GFX(stSurfaceFrom.eColorFmt, stCanvasFrom.ePixelFmt);
    stSurfaceFrom.phyAddr = stCanvasFrom.phyAddr;
    stSurfaceFrom.u32Width = stCanvasFrom.stSize.u32Width;
    stSurfaceFrom.u32Height = stCanvasFrom.stSize.u32Height;
    stSurfaceFrom.u32Stride = stCanvasFrom.u32Stride;
    stRectFrom.s32Xpos = 0;
    stRectFrom.s32Ypos = 0;
    stRectFrom.u32Height = stCanvasFrom.stSize.u32Height;
    stRectFrom.u32Width = stSurfaceFrom.u32Width;

    stSurfaceTo.eColorFmt = (MI_GFX_ColorFmt_e)u8GfxColorFmtTo;
    stSurfaceTo.phyAddr = pstCanvasTo->phyAddr;
    stSurfaceTo.u32Width = pstCanvasTo->stSize.u32Width;
    stSurfaceTo.u32Height = pstCanvasTo->stSize.u32Height;
    stSurfaceTo.u32Stride = pstCanvasTo->u32Stride;
    stRectTo.s32Xpos = stPoint.u32X;
    stRectTo.s32Ypos = stPoint.u32Y;
    stRectTo.u32Height = stCanvasFrom.stSize.u32Height;
    stRectTo.u32Width = stCanvasFrom.stSize.u32Width;


    memset(&stOpt, 0, sizeof(MI_GFX_Opt_t));
    stOpt.bEnGfxRop = FALSE;
    stOpt.eRopCode = E_MI_GFX_ROP_NONE;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    DBG_INFO("GFX Src addr %llx, w %d, h %d, stride %d, rect x %d, y %d, h %d, w %d, Dst addr %llx, h %d, w %d, stride %d, rect x %d, y %d, h %d, w %d\n", stSurfaceFrom.phyAddr, stSurfaceFrom.u32Width, stSurfaceFrom.u32Height, stSurfaceFrom.u32Stride, stRectFrom.s32Xpos, stRectFrom.s32Ypos, stRectFrom.u32Height, stRectFrom.u32Width,    \
        stSurfaceTo.phyAddr, stSurfaceTo.u32Height, stSurfaceTo.u32Width, stSurfaceTo.u32Stride, stRectTo.s32Xpos, stRectTo.s32Ypos, stRectTo.u32Height, stRectTo.u32Width);

    if (MI_SUCCESS != MI_GFX_BitBlit(&stSurfaceFrom, &stRectFrom, &stSurfaceTo, &stRectTo, &stOpt, &u16Fence))
    {
        DBG_EXIT_ERR("Gfx BitBlit error!\n");
    }

    MI_GFX_WaitAllDone(FALSE, u16Fence);
#endif
}

#if MI_RGN_BUF_WQ
#define MI_RGN_DOUBLE_FRONT_BUFFER 0
#if (MI_RGN_DOUBLE_FRONT_BUFFER)
static MI_S32 _mi_rgn_drv_misys_buf_dup(MI_RGN_CanvasInfo_t *pstDupBuffer, MI_RGN_CanvasInfo_t *pstSrcBuffer)
{
    MI_U32 u32Size = 0;
    MI_S32 s32Error = MI_RGN_OK;

    MI_RGN_PTR_CHECK(pstDupBuffer, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstSrcBuffer, MI_SYS_BUG_ON(TRUE););

    memcpy(pstDupBuffer, pstSrcBuffer, sizeof(MI_RGN_CanvasInfo_t));
    u32Size = pstDupBuffer->u32Stride * pstDupBuffer->stSize.u32Height;
    if (MI_SUCCESS != mi_sys_MMA_Alloc(NULL, u32Size, &pstDupBuffer->phyAddr))
    {
        DBG_EXIT_ERR("alloc error!\n");
        s32Error = MI_ERR_RGN_NOMEM;
        goto ALLOC_ERROR;
    }
    pstDupBuffer->virtAddr = (MI_VIRT)mi_sys_Vmap(pstDupBuffer->phyAddr, u32Size, FALSE);
    if (NULL == pstDupBuffer->virtAddr)
    {
        DBG_EXIT_ERR("map error!\n");
        s32Error = MI_ERR_RGN_BADADDR;
        goto MAP_ERROR;
    }
    pstSrcBuffer->virtAddr = (MI_VIRT)mi_sys_Vmap(pstSrcBuffer->phyAddr, u32Size, FALSE);
    if (NULL == pstDupBuffer->virtAddr)
    {
        DBG_EXIT_ERR("map error!\n");
        s32Error = MI_ERR_RGN_BADADDR;
        goto MAP_ERROR;
    }
    memcpy((MI_U8 *)pstDupBuffer->virtAddr, (MI_U8 *)pstSrcBuffer->virtAddr, u32Size);
    mi_sys_UnVmap((void *)pstSrcBuffer->virtAddr);
    pstSrcBuffer->virtAddr = 0;
    mi_sys_UnVmap((void *)pstDupBuffer->virtAddr);
    pstDupBuffer->virtAddr = 0;
    mi_rgn_MemAllocCnt((void *)((MI_U32)pstDupBuffer->phyAddr), u32Size, 0);

    return MI_RGN_OK;

MAP_ERROR:
    memset(pstDupBuffer, 0, sizeof(MI_RGN_CanvasInfo_t));
    mi_sys_MMA_Free(pstDupBuffer->phyAddr);
    pstDupBuffer->virtAddr = (MI_VIRT)NULL;
    pstDupBuffer->phyAddr = (MI_PHY)NULL;
ALLOC_ERROR:

    return s32Error;
}
#endif
static void _mi_rgn_drv_dequeue_buf_blit_work(struct work_struct * pstWork)
{
    MI_RGN_BufferWorkData_t *pstbufWork = NULL;
    MI_RGN_CanvasInfo_t stFrontBuffer;
    MI_BOOL bDoBlit = FALSE;

    MI_RGN_PTR_CHECK(pstWork, MI_SYS_BUG_ON(TRUE););

    pstbufWork = container_of(pstWork, MI_RGN_BufferWorkData_t, bufWork);
    MI_RGN_PTR_CHECK(pstbufWork, MI_SYS_BUG_ON(TRUE););
    MI_SYS_BUG_ON(pstbufWork->eWorkType != E_MI_RGN_BUF_WORKTYPE_BLIT);

    /*If the magic number is different from backup, means the front buffer did enqueue buf del work, so no need to bilt/fill.*/
    /*Background buf is impossible be empty, because front buffer release earlier than background buffer.*/
    down(&_gstRgnDrvWmSem);
    MI_SYS_BUG_ON(!pstbufWork->pstFrontBuffer);
    if (pstbufWork->pstFrontBuffer->u64MagicNum == pstbufWork->u64MagicNum)
    {
        bDoBlit = TRUE;
        memcpy(&stFrontBuffer, &pstbufWork->pstFrontBuffer->stFront, sizeof(MI_RGN_CanvasInfo_t));
        /*Front buffer is impossile be empty because every front buffer opearation do magic number++*/
        MI_SYS_BUG_ON(_mi_rgn_drv_check_canvasinfo_empty(&stFrontBuffer));
    }
    up(&_gstRgnDrvWmSem);

    if (bDoBlit)
    {
#if (MI_RGN_DOUBLE_FRONT_BUFFER)
        MI_RGN_CanvasInfo_t stDoubleBuffer;

        if (MI_RGN_OK == _mi_rgn_drv_misys_buf_dup(&stDoubleBuffer, &stFrontBuffer))
        {
            _mi_rgn_drv_blit_buffer(&stDoubleBuffer, &pstbufWork->stBlitWork.stCanvasInfoFrom, &pstbufWork->stBlitWork.stPoint);
            down(&_gstRgnDrvWmSem);
            DBG_INFO("Magic num %lld bk %llx w %d h %d back %llx front %llx\n", pstbufWork->pstFrontBuffer->u64MagicNum, pstbufWork->u64MagicNum, pstbufWork->stBlitWork.stCanvasInfoFrom.stSize.u32Width, pstbufWork->stBlitWork.stCanvasInfoFrom.stSize.u32Height, pstbufWork->stBlitWork.stCanvasInfoFrom.phyAddr, pstbufWork->pstFrontBuffer->stFront.phyAddr);
            if (pstbufWork->pstFrontBuffer->u64MagicNum == pstbufWork->u64MagicNum)
            {
                _mi_rgn_drv_buf_enqueue_del_work(&pstbufWork->pstFrontBuffer->stFront);
                memcpy(&pstbufWork->pstFrontBuffer->stFront, &stDoubleBuffer, sizeof(MI_RGN_CanvasInfo_t));
            }
            else
            {
                _mi_rgn_drv_misys_buf_del(&stDoubleBuffer);
            }
            up(&_gstRgnDrvWmSem);
        }
        else
        {
            _mi_rgn_drv_blit_buffer(&stFrontBuffer, &pstbufWork->stBlitWork.stCanvasInfoFrom, &pstbufWork->stBlitWork.stPoint);
        }
#else
        _mi_rgn_drv_blit_buffer(&stFrontBuffer, &pstbufWork->stBlitWork.stCanvasInfoFrom, &pstbufWork->stBlitWork.stPoint);
#endif
    }


    MI_RGN_MEMFREE(pstbufWork);
}
static void _mi_rgn_drv_dequeue_buf_fill_work(struct work_struct * pstWork)
{
    MI_RGN_BufferWorkData_t *pstbufWork = NULL;
    MI_RGN_CanvasInfo_t stFrontBuffer;
    MI_BOOL bDoFill = FALSE;

    MI_RGN_PTR_CHECK(pstWork, MI_SYS_BUG_ON(TRUE););

    pstbufWork = container_of(pstWork, MI_RGN_BufferWorkData_t, bufWork);
    MI_RGN_PTR_CHECK(pstbufWork, MI_SYS_BUG_ON(TRUE););
    MI_SYS_BUG_ON(pstbufWork->eWorkType != E_MI_RGN_BUF_WORKTYPE_FILL);

    /*If the magic num is different from backup, means the front buffer did enqueue buf del work, so no need to bilt/fill.*/
    down(&_gstRgnDrvWmSem);
    MI_SYS_BUG_ON(!pstbufWork->pstFrontBuffer);
    if (pstbufWork->pstFrontBuffer->u64MagicNum == pstbufWork->u64MagicNum)
    {
        bDoFill = TRUE;
        memcpy(&stFrontBuffer, &pstbufWork->pstFrontBuffer->stFront, sizeof(MI_RGN_CanvasInfo_t));
        /*Front buffer is impossile be empty because every front buffer opearation do magic number++*/
        MI_SYS_BUG_ON(_mi_rgn_drv_check_canvasinfo_empty(&stFrontBuffer));
    }
    up(&_gstRgnDrvWmSem);
    if (bDoFill)
    {
#if (MI_RGN_DOUBLE_FRONT_BUFFER)
        MI_RGN_CanvasInfo_t stDoubleBuffer;

        if (MI_RGN_OK == _mi_rgn_drv_misys_buf_dup(&stDoubleBuffer, &stFrontBuffer))
        {
            _mi_rgn_drv_buf_fill(&stDoubleBuffer, &pstbufWork->stFillWork.stRectSize, &pstbufWork->stFillWork.stRectPos, pstbufWork->stFillWork.u32ColorVal);
            down(&_gstRgnDrvWmSem);
            DBG_INFO("Magic num %lld back %llx front w %d h %d %llx\n", pstbufWork->pstFrontBuffer->u64MagicNum, pstbufWork->u64MagicNum, pstbufWork->pstFrontBuffer->stFront.stSize.u32Width, pstbufWork->pstFrontBuffer->stFront.stSize.u32Height, pstbufWork->pstFrontBuffer->stFront.phyAddr);
            if (pstbufWork->pstFrontBuffer->u64MagicNum == pstbufWork->u64MagicNum)
            {
                _mi_rgn_drv_buf_enqueue_del_work(&pstbufWork->pstFrontBuffer->stFront);
                memcpy(&pstbufWork->pstFrontBuffer->stFront, &stDoubleBuffer, sizeof(MI_RGN_CanvasInfo_t));
            }
            else
            {
                _mi_rgn_drv_misys_buf_del(&stDoubleBuffer);
            }
            up(&_gstRgnDrvWmSem);
        }
        else
        {
            _mi_rgn_drv_buf_fill(&stFrontBuffer, &pstbufWork->stFillWork.stRectSize, &pstbufWork->stFillWork.stRectPos, pstbufWork->stFillWork.u32ColorVal);
        }
#else
        _mi_rgn_drv_buf_fill(&stFrontBuffer, &pstbufWork->stFillWork.stRectSize, &pstbufWork->stFillWork.stRectPos, pstbufWork->stFillWork.u32ColorVal);
#endif
    }

    MI_RGN_MEMFREE(pstbufWork);
}
static MI_S32 _mi_rgn_drv_enqueue_buf_blit_work(MI_RGN_FrontBufferInfo_t *pstFrontBuffer, MI_RGN_CanvasInfo_t *pstCanvasInfoFrom, MI_RGN_Point_t *pstPos)
{
    MI_RGN_BufferWorkData_t *pstbufWork;

    MI_SYS_BUG_ON(!pstFrontBuffer);
    MI_SYS_BUG_ON(!pstCanvasInfoFrom);
    MI_SYS_BUG_ON(!pstPos);

    MI_RGN_MEMALLOC(pstbufWork, sizeof(MI_RGN_BufferWorkData_t));
    MI_SYS_BUG_ON(!pstbufWork);
    pstbufWork->eWorkType = E_MI_RGN_BUF_WORKTYPE_BLIT;
    pstbufWork->pstFrontBuffer = pstFrontBuffer;
    pstbufWork->u64MagicNum = pstFrontBuffer->u64MagicNum;
    memcpy(&pstbufWork->stBlitWork.stCanvasInfoFrom, pstCanvasInfoFrom, sizeof(MI_RGN_CanvasInfo_t));
    memcpy(&pstbufWork->stBlitWork.stPoint, pstPos, sizeof(MI_RGN_Point_t));

    DBG_INFO("w %d h %d from addr %llx to addr %llx\n", pstCanvasInfoFrom->stSize.u32Width, pstCanvasInfoFrom->stSize.u32Height, pstCanvasInfoFrom->phyAddr, pstbufWork->pstFrontBuffer->stFront.phyAddr);
    INIT_WORK(&pstbufWork->bufWork, _mi_rgn_drv_dequeue_buf_blit_work);
    queue_work(_stBufWorkListHead.pStBufOptWorkQueue, &pstbufWork->bufWork);

    return MI_RGN_OK;
}
static MI_S32 _mi_rgn_drv_enqueue_buf_fill_work(MI_RGN_FrontBufferInfo_t *pstFrontBuffer,  MI_RGN_Size_t *pstRectSize, MI_RGN_Point_t *pstRectPos, MI_U32 u32ColorVal)
{
    MI_RGN_BufferWorkData_t *pstbufWork;

    MI_SYS_BUG_ON(!pstFrontBuffer);
    MI_SYS_BUG_ON(!pstRectSize);
    MI_SYS_BUG_ON(!pstRectPos);

    MI_RGN_MEMALLOC(pstbufWork, sizeof(MI_RGN_BufferWorkData_t));
    MI_SYS_BUG_ON(!pstbufWork);
    pstbufWork->eWorkType = E_MI_RGN_BUF_WORKTYPE_FILL;
    pstbufWork->stFillWork.u32ColorVal = u32ColorVal;
    pstbufWork->pstFrontBuffer = pstFrontBuffer;
    pstbufWork->u64MagicNum = pstFrontBuffer->u64MagicNum;
    memcpy(&pstbufWork->stFillWork.stRectSize, pstRectSize, sizeof(MI_RGN_Size_t));
    memcpy(&pstbufWork->stFillWork.stRectPos, pstRectPos, sizeof(MI_RGN_Point_t));

    DBG_INFO("Front w %d h %d from addr %llx\n", pstFrontBuffer->stFront.stSize.u32Width, pstFrontBuffer->stFront.stSize.u32Height, pstFrontBuffer->stFront.phyAddr);
    INIT_WORK(&pstbufWork->bufWork, _mi_rgn_drv_dequeue_buf_fill_work);
    queue_work(_stBufWorkListHead.pStBufOptWorkQueue, &pstbufWork->bufWork);

    return MI_RGN_OK;
}
#endif

static MI_S32 _mi_rgn_drv_buf_done(MI_U64 u64Fence)
{
    MI_RGN_TemiBufWorkData_t *pstTemibufWorkList = NULL;

    MI_RGN_MEMALLOC(pstTemibufWorkList, sizeof(MI_RGN_TemiBufWorkData_t));
    MI_SYS_BUG_ON(pstTemibufWorkList == NULL);
    memset(pstTemibufWorkList, 0, sizeof(MI_RGN_TemiBufWorkData_t));
    INIT_WORK(&pstTemibufWorkList->stTemiDelWork, _mi_rgn_drv_teminate_buf_work);
    pstTemibufWorkList->u64Fence = u64Fence;
    schedule_work(&pstTemibufWorkList->stTemiDelWork);

    return MI_RGN_OK;
}
static MI_S32 _mi_rgn_drv_buf_fill_alpha(MI_RGN_FrontBufferInfo_t *pstFrontBuffer,  MI_RGN_Size_t *pstRectSize, MI_RGN_Point_t *pstRectPos)
{
    MI_U32 u32Color = 0;

    MI_SYS_BUG_ON(!pstFrontBuffer);
    MI_SYS_BUG_ON(!pstRectSize);
    MI_SYS_BUG_ON(!pstRectPos);

    switch (pstFrontBuffer->stFront.ePixelFmt)
    {
        case E_MI_RGN_PIXEL_FORMAT_I2:
        case E_MI_RGN_PIXEL_FORMAT_I4:
        case E_MI_RGN_PIXEL_FORMAT_I8:
            {
                u32Color = 0;
            }
            break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
            {
#if (MI_RGN_ENABLE_COLOR_KEY)
                u32Color = ((MI_RGN_COLOR_KEY_VALUE & 0x8000) << 16)
                    | ((MI_RGN_COLOR_KEY_VALUE & 0x7C00) << 9)
                    | ((MI_RGN_COLOR_KEY_VALUE & 0x3E0) << 6)
                    | ((MI_RGN_COLOR_KEY_VALUE & 0x1F) << 3);
#else
                u32Color = MI_RGN_INIT_ALPHA_VALUE << 24;
#endif
            }
            break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
            {

#if (MI_RGN_ENABLE_COLOR_KEY)

                u32Color = ((MI_RGN_COLOR_KEY_VALUE & 0xF000) << 16)
                    | ((MI_RGN_COLOR_KEY_VALUE & 0xF00) << 12)
                    | ((MI_RGN_COLOR_KEY_VALUE & 0xF0) << 8)
                    | ((MI_RGN_COLOR_KEY_VALUE & 0xF) << 4);
#else
                u32Color = MI_RGN_INIT_ALPHA_VALUE << 24;
#endif
            }
            break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB8888:
            {

#if (MI_RGN_ENABLE_COLOR_KEY)
                u32Color = (MI_RGN_COLOR_KEY_VALUE  << 16) | MI_RGN_COLOR_KEY_VALUE;
#else
                u32Color = MI_RGN_INIT_ALPHA_VALUE << 16;
#endif
            }
            break;
        case E_MI_RGN_PIXEL_FORMAT_RGB565:
            {

#if (MI_RGN_ENABLE_COLOR_KEY)
                u32Color = ((MI_RGN_COLOR_KEY_VALUE & 0xF800) << 8)
                    | ((MI_RGN_COLOR_KEY_VALUE & 0x7E0) << 5)
                    | ((MI_RGN_COLOR_KEY_VALUE & 0x1F) << 3);
#else
                MI_SYS_BUG();//Not support.
#endif
            }
            break;
        default:
            MI_SYS_BUG();
            break;
    }
#if (MI_RGN_BUF_WQ)
    return _mi_rgn_drv_enqueue_buf_fill_work(pstFrontBuffer, pstRectSize, pstRectPos, u32Color);
#else
    return _mi_rgn_drv_buf_fill(&pstFrontBuffer->stFront, pstRectSize, pstRectPos, u32Color);
#endif
}

#define MI_RGN_CHECK_XPOS_OVERLAP(ALTX, ARTX, BLTX, BRTX) ((ALTX <= BRTX) && (ARTX >= BLTX))
#define MI_RGN_GET_DISTANCE(posA, posB) ((posA > posB) ? (posA - posB) : (posB - posA))
static MI_S32 _mi_rgn_drv_check_xpos(MI_U32 u32XposLt, MI_U32 u32XposRt, struct list_head *pTmpList, MI_BOOL *pbOverLap, MI_U16 *pu16Distance)
{
    MI_RGN_ChPortListData_t *pos;
    MI_U32 u32Distance = 0xFFFF;

    MI_RGN_PTR_CHECK(pTmpList, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pbOverLap, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pu16Distance, MI_SYS_BUG(););

    *pu16Distance = u32Distance;
    *pbOverLap = FALSE;
    list_for_each_entry(pos, pTmpList, chPortListTmp)
    {
        u32Distance = MI_RGN_GET_DISTANCE(u32XposLt, pos->stAttachedChnPortPara.stPoint.u32X);
        if (u32Distance < *pu16Distance)
        {
            *pu16Distance = u32Distance;
        }
        DBG_INFO("ALT %d ART %d BLT %d BRT %d DISTANCE %d\n", u32XposLt, u32XposRt, pos->stAttachedChnPortPara.stPoint.u32X, pos->stAttachedChnPortPara.stPoint.u32X + pos->stRgnAttr.stOsdInitParam.stSize.u32Width, u32Distance);
        *pbOverLap |= MI_RGN_CHECK_XPOS_OVERLAP(u32XposLt, u32XposRt,    \
                pos->stAttachedChnPortPara.stPoint.u32X,    \
                pos->stAttachedChnPortPara.stPoint.u32X + pos->stRgnAttr.stOsdInitParam.stSize.u32Width - 1);
        DBG_INFO("bOverLap %d\n", *pbOverLap);
    }

    return MI_RGN_OK;
}
#define MI_RGN_CHECK_YPOS_OVERLAP(ALTY, ALBY, BLTY, BLBY) ((ALTY <= BLBY) && (ALBY >= BLTY))
#define MI_RGN_GET_DISTANCE(posA, posB) ((posA > posB) ? (posA - posB) : (posB - posA))
static MI_S32 _mi_rgn_drv_check_ypos(MI_U32 u32YposLt, MI_U32 u32YposLb, struct list_head *pTmpList, MI_BOOL *pbOverLap, MI_U16 *pu16Distance)
{
    MI_RGN_ChPortListData_t *pos;
    MI_U32 u32Distance = 0xFFFF;

    MI_RGN_PTR_CHECK(pTmpList, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pbOverLap, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pu16Distance, MI_SYS_BUG(););

    *pu16Distance = u32Distance;
    *pbOverLap = FALSE;
    list_for_each_entry(pos, pTmpList, chPortListTmp)
    {
        u32Distance = MI_RGN_GET_DISTANCE(u32YposLt, pos->stAttachedChnPortPara.stPoint.u32Y);
        if (u32Distance < *pu16Distance)
        {
            *pu16Distance = u32Distance;
        }
        DBG_INFO("ALT %d ALB %d BLT %d BLB %d DISTANCE %d\n", u32YposLt, u32YposLb, pos->stAttachedChnPortPara.stPoint.u32Y, pos->stAttachedChnPortPara.stPoint.u32Y + pos->stRgnAttr.stOsdInitParam.stSize.u32Height, u32Distance);
        *pbOverLap |= MI_RGN_CHECK_XPOS_OVERLAP(u32YposLt, u32YposLb,    \
                pos->stAttachedChnPortPara.stPoint.u32Y,    \
                pos->stAttachedChnPortPara.stPoint.u32Y + pos->stRgnAttr.stOsdInitParam.stSize.u32Height - 1);
        DBG_INFO("bOverLap %d\n", *pbOverLap);
    }

    return MI_RGN_OK;
}

static MI_S32 _mi_rgn_drv_build_frontbuffer(MI_RGN_ChPorHeadData_t *pstChPortHeadData, MI_RGN_ChPortListData_t *pstChPortListData, struct list_head *pTmpList)
{
    MI_U8 i = 0;
    MI_RGN_FrontBufferInfo_t *pstRecordTheNearestOne = NULL;
    MI_RGN_FrontBufferInfo_t *pstRecordTheEmptyOne = NULL;
    MI_RGN_FrontBufferInfo_t *pstRecordTheOverLapOne = NULL;
    MI_RGN_ChPortListData_t *pos, *posN;
    MI_BOOL bXposOverLap = FALSE;
    MI_BOOL bYposOverLap = FALSE;
    MI_U32 u32Distance = 0xFFFF;
    MI_U32 u32CurDistance = 0xFFFF;
    MI_U16 u16XposDistance = 0xFFFF;
    MI_U16 u16YposDistance = 0xFFFF;

    MI_RGN_PTR_CHECK(pstChPortHeadData, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pstChPortListData, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pTmpList, MI_SYS_BUG(););

    /*Loop front buffer start.*/
    for ( i = 0; i < E_MHAL_GOP_GWIN_ID_MAX; i++)
    {
        if (list_empty(&pTmpList[i]) && pstRecordTheOverLapOne == NULL)
        {
            if (pstChPortListData->pstLinkedFrontBuffer)
            {
                /*Prefer to use the old frontbuffer*/
                if (pstChPortListData->pstLinkedFrontBuffer == &pstChPortHeadData->astFrontBufferInfo[i])
                {
                    pstRecordTheEmptyOne = &pstChPortHeadData->astFrontBufferInfo[i];
                }
            }
            if (!pstRecordTheEmptyOne)
            {
                pstRecordTheEmptyOne = &pstChPortHeadData->astFrontBufferInfo[i];
            }

        }
        else
        {
            _mi_rgn_drv_check_xpos(pstChPortListData->stAttachedChnPortPara.stPoint.u32X,
                pstChPortListData->stAttachedChnPortPara.stPoint.u32X + pstChPortListData->stRgnAttr.stOsdInitParam.stSize.u32Width,
                &pTmpList[i], &bXposOverLap, &u16XposDistance);
            _mi_rgn_drv_check_ypos(pstChPortListData->stAttachedChnPortPara.stPoint.u32Y,
                pstChPortListData->stAttachedChnPortPara.stPoint.u32Y + pstChPortListData->stRgnAttr.stOsdInitParam.stSize.u32Height,
                &pTmpList[i], &bYposOverLap, &u16YposDistance);
            /*Check ChnPort data hpos overlap*/
             //if (bXposOverLap && bYposOverLap) //Check x & y is to ignore x pos overlay limitation
            if (bXposOverLap)
            {
                /*Record the overlap frontbuffer*/
                if (pstRecordTheOverLapOne == NULL)
                {
                    pstRecordTheOverLapOne = &pstChPortHeadData->astFrontBufferInfo[i];
                    list_add_tail(&pstChPortListData->chPortListTmp, &pTmpList[i]);
                }
                else
                {
                    MI_SYS_BUG_ON(pstRecordTheOverLapOne == &pstChPortHeadData->astFrontBufferInfo[i]);
                    list_for_each_entry_safe(pos, posN, &pTmpList[i], chPortListTmp)
                    {
                        list_del(&pos->chPortListTmp);
                        list_add_tail(&pos->chPortListTmp, &pTmpList[pstRecordTheOverLapOne->u8Idx]);
                    }
                    pstRecordTheOverLapOne = &pstChPortHeadData->astFrontBufferInfo[i];
                }
            }
            else
            {
                /*Record the nearest distance from frontbuffer*/
                if (pstRecordTheNearestOne == NULL)
                {
                    u32Distance = u16XposDistance * u16XposDistance  + u16YposDistance * u16YposDistance;
                    pstRecordTheNearestOne = &pstChPortHeadData->astFrontBufferInfo[i];
                }
                else
                {
                    u32CurDistance = u16XposDistance * u16XposDistance  + u16YposDistance * u16YposDistance;
                    if (u32CurDistance < u32Distance)
                    {
                        u32Distance = u32CurDistance;
                        pstRecordTheNearestOne = &pstChPortHeadData->astFrontBufferInfo[i];
                    }
                }
            }
        }
    }
    if (pstRecordTheOverLapOne)
    {
        DBG_INFO("Reason hpos overlap !\n");

        return MI_RGN_OK;
    }
    if (pstRecordTheEmptyOne)
    {
        list_add_tail(&pstChPortListData->chPortListTmp, &pTmpList[pstRecordTheEmptyOne->u8Idx]);
        DBG_INFO("Reason Use empty buffer!\n");

        return MI_RGN_OK;
    }
    if (pstRecordTheNearestOne)
    {
        list_add_tail(&pstChPortListData->chPortListTmp, &pTmpList[pstRecordTheNearestOne->u8Idx]);
        DBG_INFO("Reason: Near region !\n");

        return MI_RGN_OK;
    }

    return MI_ERR_RGN_NOT_SUPPORT;
}
static MI_S32 _mi_rgn_drv_caculate_frontbuffer_size(struct list_head *pTmpList, MI_RGN_CanvasInfo_t *pstCanvasInfo, MI_RGN_Point_t *pstPos)
{
    MI_RGN_ChPortListData_t *pos = NULL;
    MI_BOOL bCheck = FALSE;
    MI_U32 u32MaxWidth = 0, u32MaxHeight = 0;
    MI_RGN_Point_t stPos;
    MI_RGN_PixelFormat_e eFormat = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
    MHAL_RGN_GopPixelFormat_e eMhalFormat = E_MHAL_RGN_PIXEL_FORMAT_MAX;

    MI_RGN_PTR_CHECK(pstCanvasInfo, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pstPos, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pTmpList, MI_SYS_BUG(););

    if (list_empty(pTmpList))
    {
        memset(pstCanvasInfo, 0, sizeof(MI_RGN_CanvasInfo_t));
        memset(pstPos, 0, sizeof(MI_RGN_Point_t));
        DBG_INFO("Empty front buffer!\n");

        return MI_RGN_OK;
    }
    memset(&stPos, 0, sizeof(MI_RGN_Point_t));
    list_for_each_entry(pos, pTmpList, chPortListTmp)
    {
        if (bCheck == FALSE)
        {
            u32MaxWidth = pos->stAttachedChnPortPara.stPoint.u32X + pos->stRgnAttr.stOsdInitParam.stSize.u32Width;
            u32MaxHeight = pos->stAttachedChnPortPara.stPoint.u32Y + pos->stRgnAttr.stOsdInitParam.stSize.u32Height;
            stPos.u32X = pos->stAttachedChnPortPara.stPoint.u32X;
            stPos.u32Y = pos->stAttachedChnPortPara.stPoint.u32Y;
            eFormat = pos->stRgnAttr.stOsdInitParam.ePixelFmt;
            bCheck = TRUE;
        }
        else
        {
            if (eFormat != pos->stRgnAttr.stOsdInitParam.ePixelFmt)
            {
                DBG_ERR("Only support one format!\n");
                return MI_ERR_RGN_NOT_SUPPORT;
            }//one buffer mode only support one format!
            u32MaxWidth = MI_RGN_MAX(pos->stAttachedChnPortPara.stPoint.u32X + pos->stRgnAttr.stOsdInitParam.stSize.u32Width, u32MaxWidth);
            u32MaxHeight = MI_RGN_MAX(pos->stAttachedChnPortPara.stPoint.u32Y + pos->stRgnAttr.stOsdInitParam.stSize.u32Height, u32MaxHeight);
            stPos.u32X = MI_RGN_MIN(pos->stAttachedChnPortPara.stPoint.u32X, stPos.u32X);
            stPos.u32Y = MI_RGN_MIN(pos->stAttachedChnPortPara.stPoint.u32Y, stPos.u32Y);
        }
    }
    u32MaxHeight = MI_RGN_MIN(u32MaxHeight, MI_RGN_OVERLAY_MAX_HEIGHT);
    u32MaxWidth = MI_RGN_MIN(u32MaxWidth, MI_RGN_OVERLAY_MAX_WIDTH);
    MI_RGN_MAP_PIXFORMAT_TO_MHAL(eMhalFormat, pstCanvasInfo->ePixelFmt);
    MI_SYS_BUG_ON(E_MHAL_RGN_PIXEL_FORMAT_MAX == eMhalFormat);
    pstPos->u32X = ALIGN_DOWN(stPos.u32X, (MI_U32)MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT, eMhalFormat)); //2 is start x pos aligment
    pstPos->u32Y = stPos.u32Y;
    u32MaxWidth += stPos.u32X - pstPos->u32X;
    pstCanvasInfo->stSize.u32Width = ALIGN_UP(u32MaxWidth - stPos.u32X, (MI_U32)MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT, eMhalFormat)); //2 is width aligment
    pstCanvasInfo->stSize.u32Height = u32MaxHeight - stPos.u32Y;
    pstCanvasInfo->ePixelFmt = eFormat;

    DBG_INFO("Get frontbuffer max width %d heitght %d start pos x: %d y: %d\n", pstCanvasInfo->stSize.u32Width, pstCanvasInfo->stSize.u32Height, stPos.u32X, stPos.u32Y);

    return MI_RGN_OK;
}
static MI_BOOL _mi_rgn_drv_check_frontbuffer_resize_flag(MI_RGN_FrontBufferInfo_t *pstFrontBuffer, MI_RGN_CanvasInfo_t *pstCanvasInfo)
{
    MI_RGN_PTR_CHECK(pstFrontBuffer, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstCanvasInfo, MI_SYS_BUG_ON(TRUE););


    if (pstCanvasInfo->stSize.u32Height == pstFrontBuffer->stFront.stSize.u32Height
        && pstCanvasInfo->stSize.u32Width == pstFrontBuffer->stFront.stSize.u32Width)
    {
        DBG_INFO("One buf mode no need to alloc buffer.!\n");
        DBG_INFO("w %d h %d!\n", pstCanvasInfo->stSize.u32Width, pstCanvasInfo->stSize.u32Height);
        DBG_INFO("front w %d h %d!\n", pstFrontBuffer->stFront.stSize.u32Width, pstFrontBuffer->stFront.stSize.u32Height);

        return FALSE;
    }
    DBG_INFO("Need to alloc buffer. w %d h %d!\n", pstCanvasInfo->stSize.u32Width, pstCanvasInfo->stSize.u32Height);

    return TRUE;
}
static MI_S32 _mi_rgn_drv_check_hw_alignment_limitation(
        struct list_head *pTmpListA, struct list_head *pTmpListB, 
        MI_RGN_CanvasInfo_t *pstCanvasInfoA, MI_RGN_Point_t *pstPointA, 
        MI_RGN_CanvasInfo_t *pstCanvasInfoB, MI_RGN_Point_t *pstPointB
    )
{
    MI_S32 s32Ret = MI_RGN_OK;
    MI_RGN_Point_t stPointALT, stPointARB, stPointBLT, stPointBRB;
    MI_BOOL bXposOverLap = FALSE, bYposOverLap = FALSE;

    MI_RGN_PTR_CHECK(pstCanvasInfoA, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pstPointA, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pstCanvasInfoB, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pstPointB, MI_SYS_BUG(););

    if (!list_empty(pTmpListA) && !list_empty(pTmpListB))
    {
        bXposOverLap = MI_RGN_CHECK_XPOS_OVERLAP(pstPointA->u32X, pstPointA->u32X + pstCanvasInfoA->stSize.u32Width - 1,
                                pstPointB->u32X, pstPointB->u32X + pstCanvasInfoB->stSize.u32Width - 1);
        bYposOverLap = MI_RGN_CHECK_YPOS_OVERLAP(pstPointA->u32Y, pstPointA->u32Y + pstCanvasInfoA->stSize.u32Height - 1,
                                pstPointB->u32Y, pstPointB->u32Y + pstCanvasInfoB->stSize.u32Height -  1);

        /*Check ChnPort data hpos overlap*/
        //if (bXposOverLap && bYposOverLap) //Check x & y is to ignore x pos overlay limitation
        if (bXposOverLap)
        {
            MI_RGN_ChPortListData_t *pos, *posN;

            MI_RGN_PTR_CHECK(pTmpListA, MI_SYS_BUG(););
            MI_RGN_PTR_CHECK(pTmpListB, MI_SYS_BUG(););

            DBG_INFO("Front buffer Overlap A:\nLeft Top x: %d y: %d A: Right Bottom x: %d y: %d\n", stPointALT.u32X, stPointALT.u32Y, stPointARB.u32X, stPointARB.u32Y);
            DBG_INFO("Front buffer Overlap B:\nLeft Top x: %d y: %d B: Right Bottom x: %d y: %d\n", stPointBLT.u32X, stPointBLT.u32Y, stPointBRB.u32X, stPointBRB.u32Y);
            list_for_each_entry_safe(pos, posN, pTmpListB, chPortListTmp)
            {
                list_del(&pos->chPortListTmp);
                list_add_tail(&pos->chPortListTmp, pTmpListA);
                s32Ret = _mi_rgn_drv_caculate_frontbuffer_size(pTmpListA, pstCanvasInfoA, pstPointA);
                if (s32Ret != MI_RGN_OK)
                {
                    return s32Ret;
                }

            }
        }
    }

    return MI_RGN_OK;
}
static MI_S32 _mi_rgn_drv_do_frontbuffer(MI_RGN_ChPorHeadData_t *pstChPortHeadData, MI_RGN_ChPortListData_t *pstChPortListData)
{
    MI_S32 s32Ret = MI_RGN_OK;
    MI_RGN_FrontBufferInfo_t *pstFrontBuffer;
    MI_U8 i = 0, j = 0;
    struct list_head stTmpChnPortListHead[MI_RGN_MAX_GOP_GWIN_NUM];
    MI_RGN_CanvasInfo_t stFrontBk[MI_RGN_MAX_GOP_GWIN_NUM];
    MI_RGN_Point_t stPointBk[MI_RGN_MAX_GOP_GWIN_NUM];
    MI_RGN_ChPortListData_t *pos = NULL;
    MI_RGN_ChPortListData_t *pOptListData = NULL;
    MI_RGN_Point_t stPos;
    MI_BOOL bDoBlit[MI_RGN_MAX_GOP_GWIN_NUM];
    DBG_ENTER();

    MI_RGN_PTR_CHECK(pstChPortHeadData, MI_SYS_BUG(););

    memset(&stPos, 0, sizeof(MI_RGN_Point_t));
    memset(stPointBk, 0, sizeof(MI_RGN_Point_t) * MI_RGN_MAX_GOP_GWIN_NUM);
    memset(stFrontBk, 0, sizeof(MI_RGN_CanvasInfo_t) * MI_RGN_MAX_GOP_GWIN_NUM);
    memset(bDoBlit,0, sizeof(MI_BOOL) * MI_RGN_MAX_GOP_GWIN_NUM);

    /*First build temp list*/
    for (i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
    {
        INIT_LIST_HEAD(&stTmpChnPortListHead[i]);
    }
    pOptListData = pstChPortListData;
    if (!pOptListData)
    {
        list_for_each_entry(pos, &pstChPortHeadData->chPortAttachedOverlayList, chPortList)
        {
            if (!pOptListData) /*for delete get first one*/
            {
                pOptListData = pos;
                continue;
            }
            list_add_tail(&pos->chPortListTmp, &stTmpChnPortListHead[pos->pstLinkedFrontBuffer->u8Idx]);
        }
    }
    else
    {
        list_for_each_entry(pos, &pstChPortHeadData->chPortAttachedOverlayList, chPortList)
        {
            if (pos == pOptListData)
                continue;
            list_add_tail(&pos->chPortListTmp, &stTmpChnPortListHead[pos->pstLinkedFrontBuffer->u8Idx]);
        }
    }

    /*Second start to build front buffer*/
    if (pOptListData)
    {
        MI_SYS_BUG_ON(_mi_rgn_drv_build_frontbuffer(pstChPortHeadData, pOptListData, stTmpChnPortListHead) != MI_RGN_OK);
    }
    for (i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
    {
        pstFrontBuffer = &pstChPortHeadData->astFrontBufferInfo[i];
        memcpy(&stFrontBk[i], &pstFrontBuffer->stFront, sizeof(MI_RGN_CanvasInfo_t));
        memcpy(&stPointBk[i], &pstFrontBuffer->stPoint, sizeof(MI_RGN_Point_t));
        s32Ret = _mi_rgn_drv_caculate_frontbuffer_size(&stTmpChnPortListHead[i], &stFrontBk[i], &stPointBk[i]);
        if (s32Ret != MI_RGN_OK)
        {
            goto ERROR_CODE;
        }
        for(j = i + 1; j < MI_RGN_MAX_GOP_GWIN_NUM; j++)
        {
            s32Ret = _mi_rgn_drv_caculate_frontbuffer_size(&stTmpChnPortListHead[j], &stFrontBk[j], &stPointBk[j]);
            if (s32Ret != MI_RGN_OK)
            {
                goto ERROR_CODE;
            }
            s32Ret = _mi_rgn_drv_check_hw_alignment_limitation(&stTmpChnPortListHead[i], &stTmpChnPortListHead[j], &stFrontBk[i], &stPointBk[i], &stFrontBk[j], &stPointBk[j]);
            if (s32Ret != MI_RGN_OK)
            {
                goto ERROR_CODE;
            }
        }
        if (_mi_rgn_drv_check_frontbuffer_resize_flag(pstFrontBuffer, &stFrontBk[i])
            &&  !_mi_rgn_drv_check_canvasinfo_empty(&stFrontBk[i]))
        {
            /*Creat a new buffer*/
            s32Ret = _mi_rgn_drv_misys_buf_alloc(&stFrontBk[i]);
            if (MI_RGN_OK != s32Ret)
            {
                memset(&stFrontBk[i], 0, sizeof(MI_RGN_CanvasInfo_t));
                DBG_EXIT_ERR("_mi_rgn_drv_misys_buf_alloc fail.\n");
                goto ERROR_CODE;
            }
        }
    }
#if 0
    for (i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
    {
        pstFrontBuffer = &pstChPortHeadData->astFrontBufferInfo[i];

        DBG_WRN("tmp buf w %d h %d x %d y %d pa %llx\n", stFrontBk[i].stSize.u32Width, stFrontBk[i].stSize.u32Width, stPointBk[i].u32X, stPointBk[i].u32Y, stFrontBk[i].phyAddr);
        DBG_WRN("front buf w %d h %d x %d y %d pa %llx\n", pstFrontBuffer->stFront.stSize.u32Width, pstFrontBuffer->stFront.stSize.u32Width, pstFrontBuffer->stPoint.u32X, pstFrontBuffer->stPoint.u32X, pstFrontBuffer->stFront.phyAddr);
    }
#endif
    /*Copy config data.*/
    for (i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
    {
        pstFrontBuffer = &pstChPortHeadData->astFrontBufferInfo[i];
        pstFrontBuffer->u8OsdCnt = 0;
        list_for_each_entry(pos, &stTmpChnPortListHead[i], chPortListTmp)
        {
            MI_SYS_BUG_ON(pos->stRgnAttr.eType != E_MI_RGN_TYPE_OSD);
            pos->pstLinkedFrontBuffer = pstFrontBuffer;
            pos->pstLinkedFrontBuffer->u8OsdCnt++;
        }
        if (memcmp(&stFrontBk[i], &pstFrontBuffer->stFront, sizeof(MI_RGN_CanvasInfo_t)) != 0)
        {
            /*Not equal! Front buffer change!*/
            if (!_mi_rgn_drv_check_canvasinfo_empty(&pstFrontBuffer->stFront))
            {
                _mi_rgn_drv_buf_enqueue_del_work(&pstFrontBuffer->stFront);
                pstFrontBuffer->stPoint.u32X = 0;
                pstFrontBuffer->stPoint.u32Y = 0;
                pstFrontBuffer->u64MagicNum++;
            }
            memcpy(&pstFrontBuffer->stFront, &stFrontBk[i], sizeof(MI_RGN_CanvasInfo_t));
            pstFrontBuffer->bShow = FALSE;
            bDoBlit[i] = TRUE;
        }
        else if (memcmp(&stPointBk, &pstFrontBuffer->stPoint, sizeof(MI_RGN_Size_t)) != 0
            && !_mi_rgn_drv_check_canvasinfo_empty(&pstFrontBuffer->stFront))
        {
            if (pstFrontBuffer->u8OsdCnt > 1) //patch for rare condition
            {
                _mi_rgn_drv_fill_all_alpha_data_by_sw(&pstFrontBuffer->stFront);
                pstFrontBuffer->u64MagicNum++; //Cancel bilt/fill operation before
                pstFrontBuffer->bShow = FALSE;
                bDoBlit[i] = TRUE;
            }
        }
        pstFrontBuffer->stPoint.u32X = stPointBk[i].u32X;
        pstFrontBuffer->stPoint.u32Y = stPointBk[i].u32Y;
        /*Get frontbuffer info after caculate.*/
    }
    /*Do bilt buffer on changed frontbuffer!.*/
    list_for_each_entry(pos, &pstChPortHeadData->chPortAttachedOverlayList, chPortList)
    {
        MI_SYS_BUG_ON(pos->stRgnAttr.eType != E_MI_RGN_TYPE_OSD);
        MI_SYS_BUG_ON(!pos->pstLinkedFrontBuffer);
        MI_SYS_BUG_ON(pos->pstLinkedFrontBuffer->u8Idx >= MI_RGN_MAX_GOP_GWIN_NUM);
        if (!pos->stAttachedChnPortPara.bShow)
        {
            /*Clear temp list*/
            list_del(&pos->chPortListTmp);
            continue;
        }
        if (bDoBlit[pos->pstLinkedFrontBuffer->u8Idx] || pOptListData == pos)
        {
            stPos.u32X = pos->stAttachedChnPortPara.stPoint.u32X - pos->pstLinkedFrontBuffer->stPoint.u32X;
            stPos.u32Y = pos->stAttachedChnPortPara.stPoint.u32Y - pos->pstLinkedFrontBuffer->stPoint.u32Y;
#if (MI_RGN_BUF_WQ)
            _mi_rgn_drv_enqueue_buf_blit_work(pos->pstLinkedFrontBuffer, &pos->stBack, &stPos);
#else
            _mi_rgn_drv_blit_buffer(&pos->pstLinkedFrontBuffer->stFront, &pos->stBack, &stPos);
#endif
            pos->pstLinkedFrontBuffer->bShow = TRUE;
        }
        /*Clear temp list*/
        list_del(&pos->chPortListTmp);
    }
    for (i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
    {
        MI_SYS_BUG_ON(!list_empty(&stTmpChnPortListHead[i]));
    }

    return MI_RGN_OK;

ERROR_CODE:

    /*Clear temp list*/
    for (i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
    {
        pstFrontBuffer = &pstChPortHeadData->astFrontBufferInfo[i];
        if (memcmp(&stFrontBk[i], &pstFrontBuffer->stFront, sizeof(MI_RGN_CanvasInfo_t)) != 0
            && !_mi_rgn_drv_check_canvasinfo_empty(&stFrontBk[i]))
        {
            _mi_rgn_drv_buf_enqueue_del_work(&stFrontBk[i]);
        }
        list_for_each_entry_safe(pOptListData, pos, &stTmpChnPortListHead[i], chPortListTmp)
        {
            list_del(&pOptListData->chPortListTmp);
        }
    }

    return s32Ret;
}
static MI_S32 _mi_rgn_drv_window_create_osd_front_buffer(MI_RGN_ChPorHeadData_t *pstChPortHeadData, MI_RGN_ChPortListData_t *pstChPortListData, MI_RGN_CanvasInfo_t *pstCanvasInfo)
{
    MI_S32 s32ErrorCode =  MI_RGN_OK;

    MI_RGN_PTR_CHECK(pstChPortHeadData, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChPortListData, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstCanvasInfo, MI_SYS_BUG_ON(TRUE););
    DBG_ENTER();

    MI_SYS_BUG_ON(pstChPortListData->pstLinkedFrontBuffer != NULL);


    s32ErrorCode = _mi_rgn_drv_get_chport_list_head(&pstChPortListData->stAttachedChnPort, &pstChPortHeadData);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("Get ch port List fail.\n");
        goto ERROR_CODE;
    }

    memcpy(&pstChPortListData->stBack, pstCanvasInfo, sizeof(MI_RGN_CanvasInfo_t));
    DBG_INFO("Back buf %llx\n", pstChPortListData->stBack.phyAddr);

    s32ErrorCode = _mi_rgn_drv_do_frontbuffer(pstChPortHeadData, pstChPortListData);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("_mi_rgn_drv_do_frontbuffer fail.\n");
        goto ERROR_CODE;
    }

ERROR_CODE:
    return s32ErrorCode;
}
static MI_S32 _mi_rgn_drv_window_destroy_osd_front_buffer(MI_RGN_ChPorHeadData_t *pstChPortHeadData, MI_RGN_ChPortListData_t *pstChPortListData)
{
    MI_S32 s32ErrorCode =  MI_RGN_OK;
    MI_RGN_Point_t stPoint;

    MI_RGN_PTR_CHECK(pstChPortHeadData, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChPortListData, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChPortListData->pstLinkedFrontBuffer, MI_SYS_BUG_ON(TRUE););

    DBG_ENTER();

    if (pstChPortListData->pstLinkedFrontBuffer->u8OsdCnt > 1)
    {
        stPoint.u32X = pstChPortListData->stAttachedChnPortPara.stPoint.u32X - pstChPortListData->pstLinkedFrontBuffer->stPoint.u32X;
        stPoint.u32Y = pstChPortListData->stAttachedChnPortPara.stPoint.u32Y - pstChPortListData->pstLinkedFrontBuffer->stPoint.u32Y;
        _mi_rgn_drv_buf_fill_alpha(pstChPortListData->pstLinkedFrontBuffer, &pstChPortListData->stRgnAttr.stOsdInitParam.stSize, &stPoint);
    }
    s32ErrorCode = _mi_rgn_drv_do_frontbuffer(pstChPortHeadData, NULL);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("_mi_rgn_drv_do_frontbuffer fail.\n");
        goto ERROR_CODE;
    }


ERROR_CODE:
    return s32ErrorCode;
}
static MI_S32 _mi_rgn_drv_window_blitosd_front_buffer(MI_RGN_WIN_HANDLER hHandle, MI_RGN_CanvasInfo_t *pstCanvasInfo)
{
    MI_RGN_ChPortListData_t *pstChPortListData = NULL;
    MI_S32 s32ErrorCode =  MI_RGN_OK;
    MI_RGN_ChPorHeadData_t *pstChPortHeadData = NULL;
    MI_RGN_Point_t stPos;

    pstChPortListData = (MI_RGN_ChPortListData_t *)hHandle;
    MI_RGN_PTR_CHECK(pstChPortListData, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChPortListData->pstLinkedFrontBuffer, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstCanvasInfo, MI_SYS_BUG_ON(TRUE););
    DBG_ENTER();
    down(&_gstRgnDrvWmSem);
    s32ErrorCode = _mi_rgn_drv_get_chport_list_head(&pstChPortListData->stAttachedChnPort, &pstChPortHeadData);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("Get ch port List fail.\n");
        up(&_gstRgnDrvWmSem);
        goto ERROR_CODE;
    }

    if (pstChPortListData->stAttachedChnPortPara.bShow)
    {
        stPos.u32X = pstChPortListData->stAttachedChnPortPara.stPoint.u32X - pstChPortListData->pstLinkedFrontBuffer->stPoint.u32X;
        stPos.u32Y = pstChPortListData->stAttachedChnPortPara.stPoint.u32Y - pstChPortListData->pstLinkedFrontBuffer->stPoint.u32Y;
#if (MI_RGN_BUF_WQ)
        s32ErrorCode= _mi_rgn_drv_enqueue_buf_blit_work(pstChPortListData->pstLinkedFrontBuffer, pstCanvasInfo, &stPos);
#else
        s32ErrorCode = _mi_rgn_drv_blit_buffer(&pstChPortListData->pstLinkedFrontBuffer->stFront, pstCanvasInfo, &stPos);
#endif
        if (MI_RGN_OK != s32ErrorCode)
        {
            DBG_EXIT_ERR("Buf blit fail.\n");
            up(&_gstRgnDrvWmSem);
            goto ERROR_CODE;
        }
    }

    up(&_gstRgnDrvWmSem);
ERROR_CODE:
    return s32ErrorCode;
}

static MI_S32 _mi_rgn_drv_window_create(MI_RGN_WIN_HANDLER *pHandle, MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChnPortParam_t *pstChnAttr, MI_RGN_Attr_t *pstRgnAttr, MI_RGN_CanvasInfo_t *pstCanvasInfo)
{
    MI_RGN_ChPortListData_t *pstChPortListData = NULL;
    MI_S32 s32ErrorCode =  MI_RGN_OK;
    MI_RGN_ChPorHeadData_t *pstChPortHeadData = NULL;
    MI_RGN_ChPortListData_t *pCplData = NULL;

    MI_RGN_PTR_CHECK(pHandle, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChnPort, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChnAttr, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstRgnAttr, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstCanvasInfo, MI_SYS_BUG_ON(TRUE););

    DBG_ENTER();

    *pHandle = 0;
    down(&_gstRgnDrvWmSem);
    s32ErrorCode = _mi_rgn_drv_get_chport_list_head(pstChnPort, &pstChPortHeadData);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("Get ch port List fail.\n");
        up(&_gstRgnDrvWmSem);
        goto ERROR_CODE;
    }
    MI_RGN_MEMALLOC(pstChPortListData, sizeof(MI_RGN_ChPortListData_t));
    MI_RGN_PTR_CHECK(pstChPortListData, s32ErrorCode = MI_ERR_RGN_NOMEM;  \
                                up(&_gstRgnDrvWmSem);    \
                                goto ERROR_CODE;);

    memset(pstChPortListData, 0, sizeof(MI_RGN_ChPortListData_t));
    memcpy(&pstChPortListData->stAttachedChnPort, pstChnPort, sizeof(MI_RGN_ChnPort_t));
    memcpy(&pstChPortListData->stAttachedChnPortPara, pstChnAttr, sizeof(MI_RGN_ChnPortParam_t));
    memcpy(&pstChPortListData->stRgnAttr, pstRgnAttr, sizeof(MI_RGN_Attr_t));

    *pHandle = (MI_RGN_WIN_HANDLER)pstChPortListData;
    switch (pstRgnAttr->eType)
    {
        case E_MI_RGN_TYPE_OSD:
        {
            if (pstChPortHeadData->u16OverlayCnt == MI_RGN_OSD_MAX_NUM)
            {
                s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
                MI_RGN_MEMFREE(pstChPortListData);
                DBG_EXIT_ERR("Osd is full.\n");
                up(&_gstRgnDrvWmSem);
                goto ERROR_CODE;
            }
            if (_mi_rgn_drv_check_osdoverlap(pstChPortHeadData, pstChPortListData))
            {
                s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
                MI_RGN_MEMFREE(pstChPortListData);
                DBG_EXIT_ERR("Osd is overlap.\n");
                up(&_gstRgnDrvWmSem);
                goto ERROR_CODE;
            }
            list_add_tail(&pstChPortListData->chPortList, &pstChPortHeadData->chPortAttachedOverlayList);
            pstChPortHeadData->u16OverlayCnt++;
            s32ErrorCode= _mi_rgn_drv_window_create_osd_front_buffer(pstChPortHeadData, pstChPortListData, pstCanvasInfo);
            if(MI_RGN_OK != s32ErrorCode)
            {
                pstChPortHeadData->u16OverlayCnt--;
                list_del(&pstChPortListData->chPortList);
                MI_RGN_MEMFREE(pstChPortListData);
                DBG_EXIT_ERR("Osd is overlap.\n");
                up(&_gstRgnDrvWmSem);
                goto ERROR_CODE;
            }
        }
        break;
        case E_MI_RGN_TYPE_COVER:
        {
            if (pstChPortHeadData->u16CoverCnt == MI_RGN_COVER_MAX_NUM)
            {
                s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
                MI_RGN_MEMFREE(pstChPortListData);
                DBG_EXIT_ERR("Cover is full.\n");
                up(&_gstRgnDrvWmSem);
                goto ERROR_CODE;
            }
            list_for_each_entry(pCplData, &pstChPortHeadData->chPortAttachedCoverList, chPortList)
            {
                if (pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Layer ==
                            pCplData->stAttachedChnPortPara.stCoverPara.u32Layer)
                {
                    s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
                    MI_RGN_MEMFREE(pstChPortListData);
                    DBG_EXIT_ERR("Wrong layer of cover.\n");
                    up(&_gstRgnDrvWmSem);
                    goto ERROR_CODE;
                }

                if (pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Layer <
                            pCplData->stAttachedChnPortPara.stCoverPara.u32Layer)
                {
                    break;
                }
            }
            list_add_tail(&pstChPortListData->chPortList, &pCplData->chPortList);
            pstChPortHeadData->u16CoverCnt++;
        }
        break;
        default:
        {
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            MI_RGN_MEMFREE(pstChPortListData);
            DBG_EXIT_ERR("Type error.\n");
            up(&_gstRgnDrvWmSem);
            goto ERROR_CODE;
        }
        break;
    }

    up(&_gstRgnDrvWmSem);

    return MI_RGN_OK;

ERROR_CODE:

    *pHandle = 0;
    return s32ErrorCode;

}
static MI_S32 _mi_rgn_drv_window_destroy(MI_RGN_WIN_HANDLER hHandle)
{
    MI_RGN_ChPortListData_t *pstChPortListData = NULL;
    MI_S32 s32ErrorCode =  MI_RGN_OK;
    MI_RGN_ChPorHeadData_t *pstChPortHeadData = NULL;

    pstChPortListData = (MI_RGN_ChPortListData_t *)hHandle;
    MI_RGN_PTR_CHECK(pstChPortListData, return MI_ERR_RGN_NULL_PTR;);
    DBG_ENTER();

    down(&_gstRgnDrvWmSem);
    s32ErrorCode = _mi_rgn_drv_get_chport_list_head(&pstChPortListData->stAttachedChnPort, &pstChPortHeadData);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("Get ch port List fail.\n");
        up(&_gstRgnDrvWmSem);
        goto ERROR_CODE;
    }
    DBG_INFO("Rgn type %d\n", pstChPortListData->stRgnAttr.eType);
    switch (pstChPortListData->stRgnAttr.eType)
    {
        case E_MI_RGN_TYPE_OSD:
        {
            pstChPortHeadData->u16OverlayCnt--;
            list_del(&pstChPortListData->chPortList);
            s32ErrorCode= _mi_rgn_drv_window_destroy_osd_front_buffer(pstChPortHeadData, pstChPortListData);
            if(MI_RGN_OK !=s32ErrorCode)
            {
                pstChPortHeadData->u16OverlayCnt++;
                list_add_tail(&pstChPortListData->chPortList, &pstChPortHeadData->chPortAttachedOverlayList);
                DBG_EXIT_ERR("window destroy front buffer error.\n");
                up(&_gstRgnDrvWmSem);
                goto ERROR_CODE;
             }

        }
        break;
        case E_MI_RGN_TYPE_COVER:
        {
            pstChPortHeadData->u16CoverCnt--;
            list_del(&pstChPortListData->chPortList);
        }
        break;
        default:
        {
            MI_SYS_BUG_ON(TRUE);
        }
    }
    MI_RGN_MEMFREE(pstChPortListData);
    up(&_gstRgnDrvWmSem);

    return MI_RGN_OK;

ERROR_CODE:
    return s32ErrorCode;
}
static MI_S32 _mi_rgn_drv_window_set_palette(MI_RGN_PaletteTable_t *pstPaletteTable)
{
    MI_U16 i = 0, j = 0;

    MI_RGN_PTR_CHECK(pstPaletteTable, return MI_ERR_RGN_NULL_PTR;);

    memcpy(&_stPaletteTable, pstPaletteTable, sizeof(MI_RGN_PaletteTable_t));
#if (MI_RGN_ENABLE_COLOR_KEY)
    _stPaletteTable.astElement[0].u8Alpha = 0;
    _stPaletteTable.astElement[0].u8Red = MI_RGN_COLOR_KEY_VALUE & 0xFF;
    _stPaletteTable.astElement[0].u8Green = MI_RGN_COLOR_KEY_VALUE & 0xFF;
    _stPaletteTable.astElement[0].u8Blue = MI_RGN_COLOR_KEY_VALUE & 0xFF;//Index 0 special for colorkey
#else
    _stPaletteTable.astElement[0].u8Alpha = MI_RGN_INIT_ALPHA_VALUE & 0xFF;
    _stPaletteTable.astElement[0].u8Red = 0;
    _stPaletteTable.astElement[0].u8Green = 0;
    _stPaletteTable.astElement[0].u8Blue = 0;
#endif

    for (i = E_MHAL_GOP_VPE_PORT0; i < E_MHAL_GOP_TYPE_MAX; i++)
    {
        for (j = 0; j < MI_RGN_MAX_PALETTE_TABLE_NUM; j++)
        {
            MHAL_RGN_GopSetPalette(i, _stPaletteTable.astElement[j].u8Alpha, _stPaletteTable.astElement[j].u8Red, _stPaletteTable.astElement[j].u8Green, _stPaletteTable.astElement[j].u8Blue, j);
        }
    }

    return MI_RGN_OK;
}
static MI_S32 _mi_rgn_drv_window_get_palette(MI_RGN_PaletteTable_t *pstPaletteTable)
{
    MI_RGN_PTR_CHECK(pstPaletteTable, return MI_ERR_RGN_NULL_PTR;);

    memcpy(pstPaletteTable, &_stPaletteTable, sizeof(MI_RGN_PaletteTable_t));

    return MI_RGN_OK;
}

static MI_S32 _mi_rgn_drv_window_setattr(MI_RGN_WIN_HANDLER hHandle, MI_RGN_ChnPortParam_t *pstChnAttr)
{
    MI_RGN_ChPortListData_t *pstChPortListData = NULL;
    MI_RGN_ChPorHeadData_t *pstChPortHeadData = NULL;
    MI_S32 s32ErrorCode =  MI_RGN_OK;
    MI_RGN_ChnPortParam_t stChPortParaBackUp;
    MI_RGN_Point_t stPoint;

    pstChPortListData = (MI_RGN_ChPortListData_t *)hHandle;
    MI_SYS_BUG_ON(!pstChPortListData);
    DBG_ENTER();

    down(&_gstRgnDrvWmSem);
    memcpy(&stChPortParaBackUp, &pstChPortListData->stAttachedChnPortPara, sizeof(MI_RGN_ChnPortParam_t));
    memcpy(&pstChPortListData->stAttachedChnPortPara, pstChnAttr, sizeof(MI_RGN_ChnPortParam_t));

    if (pstChPortListData->stRgnAttr.eType == E_MI_RGN_TYPE_COVER)
    {
        MI_RGN_ChPortListData_t *pstCpld = NULL;

        s32ErrorCode = _mi_rgn_drv_get_chport_list_head(&pstChPortListData->stAttachedChnPort, &pstChPortHeadData);
        if (MI_RGN_OK != s32ErrorCode)
        {
            DBG_EXIT_ERR("Get ch port List fail.\n");
            goto ERROR_CODE;
        }
        list_for_each_entry(pstCpld, &pstChPortHeadData->chPortAttachedCoverList, chPortList)
        {
            if (pstCpld == pstChPortListData)
                continue;
            if (pstCpld->stAttachedChnPortPara.stCoverPara.u32Layer == pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Layer)
            {
                s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
                DBG_EXIT_ERR("Set cover layer error. Layer must not be equal.\n");
                goto ERROR_CODE;
            }
            else if (pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Layer < pstCpld->stAttachedChnPortPara.stCoverPara.u32Layer)
            {
                break;
            }
        }
        list_del(&pstChPortListData->chPortList);
        list_add_tail(&pstChPortListData->chPortList, &pstCpld->chPortList);
    }
    else if (pstChPortListData->stRgnAttr.eType == E_MI_RGN_TYPE_OSD)
    {
        MI_SYS_BUG_ON(!pstChPortListData->pstLinkedFrontBuffer);
        s32ErrorCode = _mi_rgn_drv_get_chport_list_head(&pstChPortListData->stAttachedChnPort, &pstChPortHeadData);
        if (MI_RGN_OK != s32ErrorCode)
        {
            DBG_EXIT_ERR("Get ch port List fail.\n");
            goto ERROR_CODE;
        }
        if (_mi_rgn_drv_check_osdoverlap(pstChPortHeadData, pstChPortListData))
        {
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            DBG_EXIT_ERR("osd is overlap, \n");
            goto ERROR_CODE;
        }
        if (pstChPortListData->pstLinkedFrontBuffer->u8OsdCnt > 1)
        {
            stPoint.u32X = stChPortParaBackUp.stPoint.u32X - pstChPortListData->pstLinkedFrontBuffer->stPoint.u32X;
            stPoint.u32Y = stChPortParaBackUp.stPoint.u32Y - pstChPortListData->pstLinkedFrontBuffer->stPoint.u32Y;
            _mi_rgn_drv_buf_fill_alpha(pstChPortListData->pstLinkedFrontBuffer, &pstChPortListData->stRgnAttr.stOsdInitParam.stSize, &stPoint);
        }
        s32ErrorCode = _mi_rgn_drv_do_frontbuffer(pstChPortHeadData, pstChPortListData);
        if (MI_RGN_OK != s32ErrorCode)
        {
#if (MI_RGN_BUF_WQ)
            _mi_rgn_drv_enqueue_buf_blit_work(pstChPortListData->pstLinkedFrontBuffer, &pstChPortListData->stBack, &stPoint);
#else
            _mi_rgn_drv_blit_buffer(&pstChPortListData->pstLinkedFrontBuffer->stFront, &pstChPortListData->stBack, &stPoint);
#endif
            DBG_EXIT_ERR("_mi_rgn_drv_do_frontbuffer fail.\n");
            goto ERROR_CODE;
        }
    }
    else
    {
        MI_SYS_BUG();
    }

    up(&_gstRgnDrvWmSem);

    return MI_RGN_OK;
ERROR_CODE:
    memcpy(&pstChPortListData->stAttachedChnPortPara, &stChPortParaBackUp, sizeof(MI_RGN_ChnPortParam_t));
    up(&_gstRgnDrvWmSem);

    return s32ErrorCode;
}
static MI_S32 _mi_rgn_drv_window_getchport(MI_RGN_WIN_HANDLER hHandle, MI_RGN_ChnPort_t *pstChPort)
{
    MI_RGN_ChPortListData_t *pChPortListData = NULL;

    pChPortListData = (MI_RGN_ChPortListData_t *)hHandle;
    MI_RGN_PTR_CHECK(pChPortListData, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChPort, MI_SYS_BUG_ON(TRUE););

    down(&_gstRgnDrvWmSem);
    memcpy(pstChPort, &pChPortListData->stAttachedChnPort, sizeof(MI_RGN_ChnPort_t));
    up(&_gstRgnDrvWmSem);

    return MI_RGN_OK;
}
static MI_S32 _mi_rgn_drv_window_getattr(MI_RGN_WIN_HANDLER hHandle, MI_RGN_ChnPortParam_t *pstChnAttr)
{
    MI_RGN_ChPortListData_t *pstChPortListData = NULL;

    pstChPortListData = (MI_RGN_ChPortListData_t *)hHandle;
    MI_RGN_PTR_CHECK(pstChPortListData, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChnAttr, MI_SYS_BUG_ON(TRUE););
    DBG_ENTER();

    down(&_gstRgnDrvWmSem);
    memcpy(pstChnAttr, &pstChPortListData->stAttachedChnPortPara, sizeof(MI_RGN_ChnPortParam_t));
    up(&_gstRgnDrvWmSem);

    return MI_RGN_OK;
}

#if MI_RGN_MODULE_TEST

typedef struct MHAL_RGN_GopGwinBuf_s
{
    MS_U32 u32Width;
    MS_U32 u32Height;
    MS_U32 u32Stride;
    MS_PHYADDR phyAddr;
    MS_U32 virAddr;
    MHAL_RGN_GopPixelFormat_e ePixelFmt;
}MHAL_RGN_GopGwinBuf_t;

typedef MS_U32 (*fpRgnMhalFill)(MHAL_RGN_GopGwinBuf_t *pstBufTo, MS_U32 u32Width, MS_U32 u32Height, MS_U32 u32X, MS_U32 u32Y, MS_U32 u32Color);
typedef MS_U32 (*fpRgnMhalBlit)(MHAL_RGN_GopGwinBuf_t *pstBufTo, MHAL_RGN_GopGwinBuf_t* pstBufFrom, MS_U32 u32X, MS_U32 u32Y);
typedef MS_U32 (*fpRgnMhalBufOpt)(MHAL_RGN_GopGwinBuf_t *pstBuf);

extern MS_S32 MHAL_RGN_GwinUpdate( MHAL_RGN_GopType_e eGopId, MHAL_RGN_CoverType_e eCoverType,
                                     MS_S32 s32ChnId);
extern MS_S32 MHAL_RGN_RegistBlitAlloc(fpRgnMhalFill pfFill, fpRgnMhalBlit pfBlit, fpRgnMhalBufOpt fpBufAlloc, fpRgnMhalBufOpt fpBufDel);
extern MS_S32 MHAL_RGN_GopGwinSetBufferVirt(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_U32 u32VirAddr);

static MS_U32 _mi_rgn_mhal_alloc_buffer(MHAL_RGN_GopGwinBuf_t *pstBuf)
{
    MI_RGN_CanvasInfo_t stCanvasInfo;
    MI_RGN_PTR_CHECK(pstBuf, MI_SYS_BUG_ON(TRUE););

    MI_RGN_MAP_PIXFORMAT_TO_MI(stCanvasInfo.ePixelFmt, pstBuf->ePixelFmt);
    stCanvasInfo.stSize.u32Height = pstBuf->u32Height;
    stCanvasInfo.stSize.u32Width = pstBuf->u32Width;
    if (MI_RGN_OK != _mi_rgn_drv_misys_buf_alloc(&stCanvasInfo))
    {
        return MHAL_FAILURE;
    }
    pstBuf->phyAddr = (MS_PHYADDR)stCanvasInfo.phyAddr;
    pstBuf->virAddr = (MS_U32)stCanvasInfo.virtAddr;
    pstBuf->u32Stride = (MS_U32)stCanvasInfo.u32Stride;

    return MHAL_SUCCESS;
}
static MS_U32 _mi_rgn_mhal_del_buffer(MHAL_RGN_GopGwinBuf_t *pstBuf)
{
    MI_RGN_CanvasInfo_t stCanvasInfo;
    MI_RGN_PTR_CHECK(pstBuf, MI_SYS_BUG_ON(TRUE););

    MI_RGN_MAP_PIXFORMAT_TO_MI(stCanvasInfo.ePixelFmt, pstBuf->ePixelFmt);
    stCanvasInfo.stSize.u32Height = pstBuf->u32Height;
    stCanvasInfo.stSize.u32Width = pstBuf->u32Width;
    stCanvasInfo.phyAddr = pstBuf->phyAddr;
    stCanvasInfo.virtAddr = pstBuf->virAddr;
    stCanvasInfo.u32Stride = pstBuf->u32Stride;
    _mi_rgn_drv_buf_enqueue_del_work(&stCanvasInfo);

    return MHAL_SUCCESS;
}
static MS_U32 _mi_rgn_mhal_fill_buffer(MHAL_RGN_GopGwinBuf_t *pstBufTo, MS_U32 u32Width, MS_U32 u32Height, MS_U32 u32X, MS_U32 u32Y, MS_U32 u32Color)
{
    MI_RGN_CanvasInfo_t stTo;
    MI_RGN_Size_t stSize;
    MI_RGN_Point_t stPoint;

    MI_RGN_PTR_CHECK(pstBufTo, MI_SYS_BUG_ON(TRUE););

    MI_RGN_MAP_PIXFORMAT_TO_MI(stTo.ePixelFmt, pstBufTo->ePixelFmt);
    stTo.stSize.u32Height = pstBufTo->u32Height;
    stTo.stSize.u32Width = pstBufTo->u32Width;
    stTo.phyAddr = pstBufTo->phyAddr;
    stTo.virtAddr = pstBufTo->virAddr;
    stTo.u32Stride = pstBufTo->u32Stride;
    stSize.u32Height = u32Height;
    stSize.u32Width = u32Width;
    stPoint.u32X = u32X;
    stPoint.u32Y = u32Y;
    if (MI_RGN_OK != _mi_rgn_drv_buf_fill(&stTo, &stSize, &stPoint, u32Color))
    {
        return MHAL_FAILURE;
    }

    return MHAL_SUCCESS;
}
static MS_U32 _mi_rgn_mhal_blit_buffer(MHAL_RGN_GopGwinBuf_t *pstBufTo, MHAL_RGN_GopGwinBuf_t* pstBufFrom, MS_U32 u32X, MS_U32 u32Y)
{
    MI_RGN_CanvasInfo_t stTo, stFrom;
    MI_RGN_Point_t stPos;

    MI_RGN_PTR_CHECK(pstBufTo, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstBufFrom, MI_SYS_BUG_ON(TRUE););


    MI_RGN_MAP_PIXFORMAT_TO_MI(stTo.ePixelFmt, pstBufTo->ePixelFmt);
    stTo.phyAddr = pstBufTo->phyAddr;
    stTo.virtAddr = pstBufTo->virAddr;
    stTo.stSize.u32Height = pstBufTo->u32Height;
    stTo.stSize.u32Width = pstBufTo->u32Width;
    stTo.u32Stride = pstBufTo->u32Stride;
    stPos.u32X = u32X;
    stPos.u32Y = u32Y;

    MI_RGN_MAP_PIXFORMAT_TO_MI(stFrom.ePixelFmt, pstBufFrom->ePixelFmt);
    stFrom.phyAddr = pstBufFrom->phyAddr;
    stFrom.virtAddr = pstBufFrom->virAddr;
    stFrom.stSize.u32Height = pstBufFrom->u32Height;
    stFrom.stSize.u32Width = pstBufFrom->u32Width;
    stFrom.u32Stride = pstBufFrom->u32Stride;

    return (MS_U32)_mi_rgn_drv_blit_buffer(&stTo, &stFrom, &stPos);
}

#endif

#if MI_RGN_ENABLE_COLOR_KEY
static void _mi_rgn_drv_hal_config_color(MHAL_RGN_GopType_e eGopId, MI_RGN_PixelFormat_e eFormat)
{
    MI_U8 u8R, u8G, u8B;

    if (_gbConfigColor[eGopId][eFormat])
        return;

    switch (eFormat)
    {
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
        {
            u8R = (MI_RGN_COLOR_KEY_VALUE & 0x7C00) >> 10;
            u8G = (MI_RGN_COLOR_KEY_VALUE & 0x3E0) >> 5;
            u8B = MI_RGN_COLOR_KEY_VALUE & 0x1F;
            u8R = u8R << 3 | ((u8R & 0x1C) >> 2);
            u8G = u8G << 3 | ((u8G & 0x1C) >> 2);
            u8B = u8B << 3 | ((u8B & 0x1C) >> 2);
            MHAL_RGN_GopSetColorkey(eGopId, TRUE, u8R, u8G, u8B);
            memset(&_gbConfigColor[eGopId][0], 0, sizeof(MI_BOOL) * E_MI_RGN_PIXEL_FORMAT_MAX);
            _gbConfigColor[eGopId][eFormat] = TRUE;
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
        {
            u8R = (MI_RGN_COLOR_KEY_VALUE & 0xF00) >> 8;
            u8G = (MI_RGN_COLOR_KEY_VALUE & 0xF0) >> 4;
            u8B = MI_RGN_COLOR_KEY_VALUE & 0xF;
            u8R = (u8R << 4) | u8R;
            u8G = (u8G << 4) | u8G;
            u8B = (u8B << 4) | u8B;
            MHAL_RGN_GopSetColorkey(eGopId, TRUE, u8R, u8G, u8B);
            memset(&_gbConfigColor[eGopId][0], 0, sizeof(MI_BOOL) * E_MI_RGN_PIXEL_FORMAT_MAX);
            _gbConfigColor[eGopId][eFormat] = TRUE;
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB8888:
        {
            u8R = ((MI_RGN_COLOR_KEY_VALUE | (MI_RGN_COLOR_KEY_VALUE << 16)) & 0xFF0000) >> 16;
            u8G = ((MI_RGN_COLOR_KEY_VALUE | (MI_RGN_COLOR_KEY_VALUE << 16)) & 0xFF00) >> 8;
            u8B = (MI_RGN_COLOR_KEY_VALUE | (MI_RGN_COLOR_KEY_VALUE << 16)) & 0xFF;
            MHAL_RGN_GopSetColorkey(eGopId, TRUE, u8R, u8G, u8B);
            memset(&_gbConfigColor[eGopId][0], 0, sizeof(MI_BOOL) * E_MI_RGN_PIXEL_FORMAT_MAX);
            _gbConfigColor[eGopId][eFormat] = TRUE;
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_RGB565:
        {
            u8R = (MI_RGN_COLOR_KEY_VALUE & 0xF800) >> 11;
            u8G = (MI_RGN_COLOR_KEY_VALUE & 0x7E0) >> 5;
            u8B = MI_RGN_COLOR_KEY_VALUE & 0x1F;
            u8R = (u8R << 3) | ((u8R & 0x1C) >> 2);
            u8G = (u8G << 2) | ((u8G & 0x30) >> 4);
            u8B = (u8B << 3) | ((u8B & 0x1C) >> 2);
            MHAL_RGN_GopSetColorkey(eGopId, TRUE, u8R, u8G, u8B);
            memset(&_gbConfigColor[eGopId][0], 0, sizeof(MI_BOOL) * E_MI_RGN_PIXEL_FORMAT_MAX);
            _gbConfigColor[eGopId][eFormat] = TRUE;
        }
        break;
        case E_MI_RGN_PIXEL_FORMAT_I2:
        case E_MI_RGN_PIXEL_FORMAT_I4:
        case E_MI_RGN_PIXEL_FORMAT_I8:
        {
            u8R = _stPaletteTable.astElement[0].u8Red;
            u8G = _stPaletteTable.astElement[0].u8Green;
            u8B = _stPaletteTable.astElement[0].u8Blue;
            MHAL_RGN_GopSetColorkey(eGopId, TRUE, u8R, u8G, u8B);
            memset(&_gbConfigColor[eGopId][0], 0, sizeof(MI_BOOL) * E_MI_RGN_PIXEL_FORMAT_MAX);
            _gbConfigColor[eGopId][eFormat] = TRUE;
        }
        break;
        default:
            MI_SYS_BUG();
            break;
    }

    return;
}
#endif
#define MI_RGN_CUT_SIZE(src, pos, dst)  ((((src) + (pos)) > (dst)) ? ((dst) - (pos)) : (src))
static MI_S32 _mi_rgn_drv_window_update(MI_RGN_ChnPort_t *pstChnPort,  MI_RGN_Size_t *pstDispWindowSize, MI_BOOL bEn, MHAL_CMDQ_CmdqInterface_t* pstCmdInf, MI_U64 *pu64Fence)
{
    /*After do _MI_RGN_LoadChPortListData enalbe hal config and change to the other buffer(double buffer case)*/
    MI_U8 i = 0;
    MI_U32 u32CutWidth = 0;
    MI_U32 u32CutHeight = 0;
    MI_RGN_ChPortListData_t *pstChPortListData = NULL;
    MI_RGN_ChPorHeadData_t *pstChPortHeadData = NULL;
    MI_S32 s32ErrorCode =  MI_RGN_OK;
    MHAL_RGN_GopType_e eGopId = E_MHAL_GOP_VPE_PORT0;
    MHAL_RGN_GopGwinId_e eGwinId = E_MHAL_COVER_VPE_PORT0;
    MHAL_RGN_CoverType_e eCoverType = E_MHAL_COVER_TYPE_MAX;
    MHAL_RGN_CoverLayerId_e eLayer = E_MHAL_COVER_LAYER_0;
    MHAL_RGN_GopPixelFormat_e eFormat = E_MHAL_RGN_PIXEL_FORMAT_ARGB1555;

    /*1. Do cover case*/
    MI_RGN_PTR_CHECK(pstDispWindowSize, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstCmdInf, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pstChnPort, MI_SYS_BUG_ON(TRUE););
    MI_RGN_PTR_CHECK(pu64Fence, MI_SYS_BUG_ON(TRUE););

    //DBG_INFO("Process start --------------------------------------------------------\n");
    //DBG_INFO("Port %d, Channel %d, modId %d bEn %d\n", pstChnPort->s32OutputPortId, pstChnPort->s32ChnId, (MS_U8)pstChnPort->eModId, bEn);
    if (bEn)
    {
        down(&_gstRgnDrvWmSem);
        s32ErrorCode = _mi_rgn_drv_get_chport_list_head(pstChnPort, &pstChPortHeadData);
        if (MI_RGN_OK != s32ErrorCode)
        {
            DBG_EXIT_ERR("Get ch port List fail.\n");
            up(&_gstRgnDrvWmSem);
            goto ERROR_CODE;
        }
        MHAL_RGN_SetupCmdQ(pstCmdInf, (MHAL_RGN_CmdqType_e)pstChnPort->eModId);
        MI_RGN_MAP_COVERTYPE_TO_MHAL(eCoverType, pstChnPort->s32OutputPortId, pstChnPort->eModId);
        /*----------------------------- Set Cover Start -------------------------------------*/
        if (E_MHAL_COVER_TYPE_MAX != eCoverType)
        {
            for(eLayer = E_MHAL_COVER_LAYER_0; eLayer < E_MHAL_COVER_LAYER_MAX; eLayer++)
            {
                MHAL_RGN_CoverDisable(eCoverType, eLayer);
            }
            eLayer = E_MHAL_COVER_LAYER_0;
            list_for_each_entry(pstChPortListData, &pstChPortHeadData->chPortAttachedCoverList, chPortList)
            {
                if (pstChPortListData->stAttachedChnPortPara.bShow)
                {
                    MHAL_RGN_CoverSetWindow(eCoverType, eLayer,
                        (MS_U32)pstChPortListData->stAttachedChnPortPara.stPoint.u32X * pstDispWindowSize->u32Width / MI_RGN_COVER_MAX_WIDTH,
                        (MS_U32)pstChPortListData->stAttachedChnPortPara.stPoint.u32Y * pstDispWindowSize->u32Height / MI_RGN_COVER_MAX_HEIGHT,
                        (MS_U32)pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Width * pstDispWindowSize->u32Width / MI_RGN_COVER_MAX_WIDTH,
                        (MS_U32)pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Height * pstDispWindowSize->u32Height / MI_RGN_COVER_MAX_HEIGHT);
                    MHAL_RGN_CoverSetColor(eCoverType, eLayer, (MS_U32)pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Color);
                    MHAL_RGN_CoverEnable(eCoverType, eLayer);
                    /*DBG_INFO("coverType: %d layer: %d srcHeight: %d srcWidth: %d srcX: %d srcY: %d\n", (int)eCoverType, (int)eLayer,
                        (int)pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Height,
                        (int)pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Width,
                        (int)pstChPortListData->stAttachedChnPortPara.stPoint.u32X,
                        (int)pstChPortListData->stAttachedChnPortPara.stPoint.u32Y);
                    DBG_INFO("coverType: %d layer: %d Height: %d width: %d x: %d y: %d Color: R %d G %d B %d\n", (int)eCoverType, (int)eLayer,
                        (int)pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Height * pstDispWindowSize->u32Height / MI_RGN_COVER_MAX_HEIGHT,
                        (int)pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Width * pstDispWindowSize->u32Width / MI_RGN_COVER_MAX_WIDTH,
                        (int)pstChPortListData->stAttachedChnPortPara.stPoint.u32X * pstDispWindowSize->u32Width / MI_RGN_COVER_MAX_WIDTH,
                        (int)pstChPortListData->stAttachedChnPortPara.stPoint.u32Y * pstDispWindowSize->u32Height / MI_RGN_COVER_MAX_HEIGHT,
                        (int)(pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Color & 0xF00) >> 8,
                        (int)(pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Color & 0xF0) >> 4,
                        (int)(pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Color & 0xF));*/
                }
                eLayer++;
            }
            //DBG_INFO("coverType:%d layerCount: %d \n", (MS_U8)eCoverType, (MS_U8)eLayer);
        }
        /*----------------------------- Set Cover End -------------------------------------*/

        /*----------------------------- Set Gop Start -------------------------------------*/

        MI_RGN_MAP_GOPID_TO_MHAL(eGopId, pstChnPort->s32OutputPortId, pstChnPort->eModId);
        MHAL_RGN_GopSetBaseWindow(eGopId, (MS_U32)pstDispWindowSize->u32Width, (MS_U32)pstDispWindowSize->u32Height);
        pstChPortHeadData->stBaseWindow.u32Width = pstDispWindowSize->u32Width;
        pstChPortHeadData->stBaseWindow.u32Height = pstDispWindowSize->u32Height;
        for (i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
        {
            MI_RGN_MAP_GWINID_TO_MHAL(eGwinId, i);
            MHAL_RGN_GopGwinDisable(eGopId, eGwinId);
            if (pstChPortHeadData->astFrontBufferInfo[i].bShow
                && !_mi_rgn_drv_check_canvasinfo_empty(&pstChPortHeadData->astFrontBufferInfo[i].stFront)
                && (pstChPortHeadData->astFrontBufferInfo[i].stPoint.u32X < pstDispWindowSize->u32Width && pstChPortHeadData->astFrontBufferInfo[i].stPoint.u32Y < pstDispWindowSize->u32Height))
            {
#if MI_RGN_ENABLE_COLOR_KEY
                _mi_rgn_drv_hal_config_color(eGopId, pstChPortHeadData->astFrontBufferInfo[i].stFront.ePixelFmt);
#endif
                u32CutWidth = MI_RGN_CUT_SIZE(pstChPortHeadData->astFrontBufferInfo[i].stFront.stSize.u32Width, pstChPortHeadData->astFrontBufferInfo[i].stPoint.u32X, pstDispWindowSize->u32Width);
                u32CutHeight = MI_RGN_CUT_SIZE(pstChPortHeadData->astFrontBufferInfo[i].stFront.stSize.u32Height, pstChPortHeadData->astFrontBufferInfo[i].stPoint.u32Y, pstDispWindowSize->u32Height);
                MI_RGN_MAP_PIXFORMAT_TO_MHAL(eFormat, pstChPortHeadData->astFrontBufferInfo[i].stFront.ePixelFmt);
                MHAL_RGN_GopGwinSetPixelFormat(eGopId, eGwinId, eFormat);
                MHAL_RGN_GopGwinSetWindow(eGopId, eGwinId, (MS_U32)u32CutWidth, (MS_U32)u32CutHeight,
                (MS_U32)pstChPortHeadData->astFrontBufferInfo[i].stFront.u32Stride,
                (MS_U32)pstChPortHeadData->astFrontBufferInfo[i].stPoint.u32X,
                (MS_U32)pstChPortHeadData->astFrontBufferInfo[i].stPoint.u32Y);
#if MI_RGN_MODULE_TEST
                MHAL_RGN_GopGwinSetBufferVirt(eGopId, eGwinId, (MS_U32)pstChPortHeadData->astFrontBufferInfo[i].stFront.virtAddr);
#endif
                MHAL_RGN_GopGwinSetBuffer(eGopId, eGwinId, (MS_PHYADDR)pstChPortHeadData->astFrontBufferInfo[i].stFront.phyAddr);
                MHAL_RGN_GopGwinEnable(eGopId, eGwinId);
                /*DBG_INFO("dst w %d h %d src w %d h %d crop w %d h %d phyaddr 0x%llx\n", pstDispWindowSize->u32Width, pstDispWindowSize->u32Height,
                    pstChPortHeadData->astFrontBufferInfo[i].stFront.stSize.u32Width,
                    pstChPortHeadData->astFrontBufferInfo[i].stFront.stSize.u32Height,
                    u32CutWidth, u32CutHeight, pstChPortHeadData->astFrontBufferInfo[i].stFront.phyAddr);
                DBG_INFO("Enable gwin: Gop id %d GwinId %d Format %d W %d H %d X %d Y %d\n", eGopId, eGwinId, eFormat,
                    pstChPortHeadData->astFrontBufferInfo[i].stFront.stSize.u32Width,
                    pstChPortHeadData->astFrontBufferInfo[i].stFront.stSize.u32Height,
                    pstChPortHeadData->astFrontBufferInfo[i].stPoint.u32X,
                    pstChPortHeadData->astFrontBufferInfo[i].stPoint.u32Y);*/
            }
            else
            {
                /*This is sw patch to fix hw bug.*/
                MHAL_RGN_GopGwinSetWindow(eGopId, eGwinId, 0, 0, 0, 0, 0);
            }
        }
#if (MI_RGN_MODULE_TEST)
        MHAL_RGN_GwinUpdate(eGopId, eCoverType, pstChnPort->s32ChnId);
#endif
        /*----------------------------- Set Gop End -------------------------------------*/
        up(&_gstRgnDrvWmSem);
    }

    down(&_gstRgnDrvbufDelWorkListSem);
//    DBG_INFO("Process end fence %llx--------------------------------------------------------\n", _stBufWorkListHead.u64Fence);
    *pu64Fence = _stBufWorkListHead.u64Fence++;
    up(&_gstRgnDrvbufDelWorkListSem);

ERROR_CODE:
    return s32ErrorCode;
}

MI_U32 mi_rgn_drv_init(void)
{
    mi_rgn_BufferManager_t stBm;
    mi_rgn_WindowManger_t stWm;
    MI_U8 i = 0, j = 0, k = 0;

    memset(&_gstChPortHead, 0, sizeof(MI_RGN_ChPortHead_t));
    for (i = 0; i < MI_RGN_VPE_MAX_CH_NUM; i++)
    {
        for (j = 0; j < MI_RGN_VPE_PORT_MAXNUM; j++)
        {
            INIT_LIST_HEAD(&_gstChPortHead.astVpeChPortHeadData[i][j].chPortAttachedOverlayList);
            INIT_LIST_HEAD(&_gstChPortHead.astVpeChPortHeadData[i][j].chPortAttachedCoverList);
            for (k = 0 ; k < E_MHAL_GOP_GWIN_ID_MAX; k++)
            {
                _gstChPortHead.astVpeChPortHeadData[i][j].astFrontBufferInfo[k].u8Idx = k;
            }
        }
    }
    for (i = 0; i < MI_RGN_DIVP_MAX_CH_NUM; i++)
    {
        for (j = 0; j < MI_RGN_DIVP_PORT_MAXNUM; j++)
        {
            INIT_LIST_HEAD(&_gstChPortHead.astDivpChPortHeadData[i][j].chPortAttachedOverlayList);
            INIT_LIST_HEAD(&_gstChPortHead.astDivpChPortHeadData[i][j].chPortAttachedCoverList);
            for (k = 0 ; k < E_MHAL_GOP_GWIN_ID_MAX; k++)
            {
                _gstChPortHead.astDivpChPortHeadData[i][j].astFrontBufferInfo[k].u8Idx = k;
            }
        }
    }
    INIT_LIST_HEAD(&_stBufWorkListHead.bufDelWorkList);
    _stBufWorkListHead.pStBufOptWorkQueue = create_singlethread_workqueue("RGN buf del");
    if (_stBufWorkListHead.pStBufOptWorkQueue == NULL)
    {
        DBG_EXIT_ERR("create_singlethread_workqueue error.\n");
        return MI_ERR_RGN_NOT_SUPPORT;
    }
#if (MI_RGN_ENABLE_COLOR_KEY)
    memset(_gbConfigColor, 0, sizeof(MI_BOOL) * E_MHAL_GOP_TYPE_MAX * E_MI_RGN_PIXEL_FORMAT_MAX);
#endif
#if (MI_RGN_BLIT_BY_SW == 0)
    if (MI_SUCCESS != MI_GFX_Open())
    {
        DBG_EXIT_ERR("Gfx open error!\n");
        return MI_ERR_RGN_BADADDR;
    }
#endif
    MHAL_RGN_CoverInit();
    MHAL_RGN_GopInit();
    for (i = 0; i < E_MHAL_GOP_TYPE_MAX; i++)
    {
        for (j = 0 ; j < E_MHAL_GOP_GWIN_ID_MAX; j++)
        {
#if (MI_RGN_ENABLE_COLOR_KEY)
            MHAL_RGN_GopSetAlphaType(i, j, E_MHAL_GOP_GWIN_ALPHA_CONSTANT, 0xFF);
#else
            MHAL_RGN_GopSetAlphaType(i, j, E_MHAL_GOP_GWIN_ALPHA_PIXEL, 0);
            MHAL_RGN_GopSetArgb1555AlphaVal(i, j, E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1, MI_RGN_INIT_ALPHA_VALUE & 0xFF);
            MHAL_RGN_GopSetArgb1555AlphaVal(i, j, E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0, 0);
#endif
        }
    }

    stWm.fpCreate = _mi_rgn_drv_window_create;
    stWm.fpDestroy = _mi_rgn_drv_window_destroy;
    stWm.fpSetAttr = _mi_rgn_drv_window_setattr;
    stWm.fpGetAttr = _mi_rgn_drv_window_getattr;
    stWm.fpGetChPort = _mi_rgn_drv_window_getchport;
    stWm.fpBlitOsdFrontBuffer = _mi_rgn_drv_window_blitosd_front_buffer;
    stWm.fpUpdate = _mi_rgn_drv_window_update;
    stWm.fpSetPalette = _mi_rgn_drv_window_set_palette;
    stWm.fpGetPalette = _mi_rgn_drv_window_get_palette;
    stWm.fpDone = _mi_rgn_drv_buf_done;
    stWm.fpGetCapability = _mi_rgn_drv_get_capability;
    stWm.fpGetFrontBuffer = _mi_rgn_drv_get_frontbufferinfo;
    stWm.fpGetBaseWindow = _mi_rgn_drv_get_basewindow;

    stBm.fpBufferAlloc = _mi_rgn_drv_misys_buf_alloc;
    stBm.fpBufferDel = _mi_rgn_drv_buf_enqueue_del_work;

    stBm.fpOverlayBlit = _mi_rgn_drv_blit_overlay;

#if MI_RGN_MODULE_TEST
    MHAL_RGN_RegistBlitAlloc(_mi_rgn_mhal_fill_buffer, _mi_rgn_mhal_blit_buffer, _mi_rgn_mhal_alloc_buffer, _mi_rgn_mhal_del_buffer);
#endif

    return mi_rgn_RegistDriver(&stWm, &stBm);;
}
MI_U32 mi_rgn_drv_deinit(void)
{
    MI_RGN_BufferDelWorkListData_t *pstBl = NULL;
    MI_RGN_BufferDelWorkListData_t *pstBlN = NULL;

    mi_rgn_UnregistDriver();
    if (_stBufWorkListHead.pStBufOptWorkQueue)
    {
        /*In case of current work is pending*/
        flush_workqueue(_stBufWorkListHead.pStBufOptWorkQueue);
        down(&_gstRgnDrvbufDelWorkListSem);
        list_for_each_entry_safe(pstBl, pstBlN, &_stBufWorkListHead.bufDelWorkList, bufDelWorkList)
        {
            /*Del delwork timer*/
           if (cancel_delayed_work(&pstBl->bufDelTimeOutWork))
           {
               MI_SYS_BUG_ON(MI_RGN_OK != _mi_rgn_drv_misys_buf_del(&pstBl->stBuffer));
               list_del(&pstBl->bufDelWorkList);
               MI_RGN_MEMFREE(pstBl);
           }
        }
        up(&_gstRgnDrvbufDelWorkListSem);
        flush_workqueue(_stBufWorkListHead.pStBufOptWorkQueue);
        destroy_workqueue(_stBufWorkListHead.pStBufOptWorkQueue);
        _stBufWorkListHead.pStBufOptWorkQueue = NULL;
    }
#if (MI_RGN_BLIT_BY_SW == 0)
    MI_GFX_Close();
#endif
    memset(&_stBufWorkListHead, 0, sizeof(MI_RGN_BufWorkListHead_t));

    memset(&_gstChPortHead, 0, sizeof(MI_RGN_ChPortHead_t));

    return MI_RGN_OK;
}
