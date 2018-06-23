#ifndef _MI_RGN_DATATYPE_INTERNAL_
#define _MI_RGN_DATATYPE_INTERNAL_

#include "mhal_common.h"
#include "mhal_cmdq.h"

#include "mi_rgn_datatype.h"

typedef MI_VIRT MI_RGN_WIN_HANDLER;

#define MI_RGN_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MI_RGN_MIN(x, y) (((x) < (y)) ? (x) : (y))

#define MI_RGN_HANDLE_CHECK(PARA, ERRCODE) do{  \
                                                if(PARA >= MI_RGN_MAX_HANDLE) \
                                                { \
                                                    DBG_EXIT_ERR("Handle num out of range.\n"); \
                                                    {ERRCODE;}; \
                                                }   \
                                             }while(0);
#define MI_RGN_PTR_CHECK(PTR,ERRCODE) do {  \
                                            if(!PTR) \
                                            { \
                                                DBG_EXIT_ERR("parameter pointer is null.[%s,%d]\n", __FUNCTION__, __LINE__); \
                                                {ERRCODE;}; \
                                            }   \
                                      }while (0);

#define MI_RGN_INIT_CHECK(INIT,ERRCODE) do {  \
                                             if(INIT == FALSE) \
                                             { \
                                                  DBG_EXIT_ERR("RGN did not init\n", __FUNCTION__, __LINE__); \
                                                  {ERRCODE;}; \
                                             }   \
                                         }while (0);


#define MI_RGN_RANGE_CHECK(PARA,MIN,MAX,ERRCODE) do{    \
                                                       if(PARA < MIN || PARA > MAX) \
                                                       { \
                                                           DBG_EXIT_ERR(" parameter is out of range.[%s,%d][MIN %d MAX %d PARA %d]\n", __FUNCTION__, __LINE__, MIN, MAX, PARA); \
                                                           {ERRCODE;}; \
                                                       }    \
                                                   }while(0);
#define MI_RGN_ALIGN_CHECK(WIDTH, ALIGN, ERRCODE) do{  \
                                                if (WIDTH % ALIGN)   \
                                                {   \
                                                    DBG_EXIT_ERR("Check ALIGN false!\n");   \
                                                    {ERRCODE;}; \
                                                }  \
                                            }while(0);


#define MI_RGN_MEMALLOC(addr, size)    do{    \
                                            addr = kmalloc(size, GFP_ATOMIC);    \
                                             mi_rgn_MemAllocCnt(addr, size, 1);     \
                                        }while(0);
#define MI_RGN_MEMFREE(addr)    do{    \
                                    mi_rgn_MemFreeCnt(addr, 1);    \
                                    kfree(addr);    \
                                  }while(0);
#define MI_RGN_BITS_PER_PIXEL(bits, pix, format) do{  \
                                                  switch (format) \
                                                  {   \
                                                      case E_MI_RGN_PIXEL_FORMAT_ARGB1555:    \
                                                      case E_MI_RGN_PIXEL_FORMAT_ARGB4444:    \
                                                      case E_MI_RGN_PIXEL_FORMAT_RGB565:    \
                                                      {   \
                                                          bits = pix * 16;    \
                                                      }   \
                                                      break;  \
                                                      case E_MI_RGN_PIXEL_FORMAT_I2:  \
                                                      {  \
                                                          bits = pix * 2;  \
                                                      }   \
                                                      break;  \
                                                      case E_MI_RGN_PIXEL_FORMAT_I4:  \
                                                      {   \
                                                           bits = pix * 4;  \
                                                      }   \
                                                      break;  \
                                                      case E_MI_RGN_PIXEL_FORMAT_I8:  \
                                                      {   \
                                                           bits = pix * 8;  \
                                                      }   \
                                                      break;  \
                                                      case E_MI_RGN_PIXEL_FORMAT_ARGB8888:  \
                                                      {   \
                                                           bits = pix * 32;  \
                                                      }   \
                                                      break;  \
                                                      default:    \
                                                      {   \
                                                          bits = 0;  \
                                                          DBG_ERR("Err format!!\n"); \
                                                          MI_SYS_BUG_ON(TRUE);    \
                                                      }   \
                                                      break;  \
                                                  }   \
               }while (0);
#define MI_RGN_BITS_TO_PIXEL(pix, bits, format) do{  \
                                                  switch (format) \
                                                  {   \
                                                      case E_MI_RGN_PIXEL_FORMAT_ARGB1555:    \
                                                      case E_MI_RGN_PIXEL_FORMAT_RGB565:    \
                                                      case E_MI_RGN_PIXEL_FORMAT_ARGB4444:  \
                                                      {   \
                                                          pix = bits/16;    \
                                                      }   \
                                                      break;  \
                                                      case E_MI_RGN_PIXEL_FORMAT_I2:  \
                                                      {  \
                                                          pix = bits / 2;  \
                                                      }   \
                                                      break;  \
                                                      case E_MI_RGN_PIXEL_FORMAT_I4:  \
                                                      {   \
                                                           pix = bits / 4;  \
                                                      }   \
                                                      break;  \
                                                      case E_MI_RGN_PIXEL_FORMAT_I8:  \
                                                      {   \
                                                          pix = bits / 8;  \
                                                      }   \
                                                      break;  \
                                                      case E_MI_RGN_PIXEL_FORMAT_ARGB8888:  \
                                                      {   \
                                                          pix = bits / 32;  \
                                                      }   \
                                                      break;  \
                                                      default:    \
                                                      {   \
                                                          pix = 0;  \
                                                          DBG_ERR("Err format!!\n"); \
                                                          MI_SYS_BUG_ON(TRUE);    \
                                                      }   \
                                                      break;  \
                                                  }   \
               }while (0);

#define MI_RGN_MODULE_TEST 0
#if (INTERFACE_GFX == 1)
#define MI_RGN_BLIT_BY_SW 0
#else
#define MI_RGN_BLIT_BY_SW 1
#endif
#define MI_RGN_BUF_WQ 1

#define MI_RGN_VPE_PORT_MAXNUM 4
#define MI_RGN_DIVP_PORT_MAXNUM 1

typedef struct mi_rgn_DrvCapability_s
{
    MI_BOOL bSupportFormat[E_MI_RGN_PIXEL_FORMAT_MAX];
    MI_U8 u8VpeMaxChannelNum;
    MI_U8 u8VpeMaxPortNum;
    MI_U8 u8DivpMaxChannelNum;
    MI_U8 u8DivpMaxPortNum;
    MI_U8 u8AttachedCoverCnt;
    MI_U8 u8AttachedOsdCnt;
    MI_U8 u8OsdHwModeCnt;
    MI_U8 u8OsdWidthAlignment;
    MI_U8 u8OsdI2WidthAlignment;
    MI_U16 u16CoverMinWidth;
    MI_U16 u16CoverMinHeight;
    MI_U16 u16CoverMaxWidth;
    MI_U16 u16CoverMaxHeight;
    MI_U16 u16OsdMinWidth;
    MI_U16 u16OsdMinHeight;
    MI_U16 u16OsdMaxWidth;
    MI_U16 u16OsdMaxHeight;
}mi_rgn_DrvCapability_t;

typedef struct mi_rgn_WindowManger_s
{
    MI_S32 (*fpSetPalette)(MI_RGN_PaletteTable_t *pstPaletteTable);
    MI_S32 (*fpGetPalette)(MI_RGN_PaletteTable_t *pstPaletteTable);
    MI_S32 (*fpGetCapability)(mi_rgn_DrvCapability_t *pstCap);
    MI_S32 (*fpCreate)(MI_RGN_WIN_HANDLER *pHandle, MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChnPortParam_t *pstChPort, MI_RGN_Attr_t *pstRgnAttr, MI_RGN_CanvasInfo_t *pstCanvasInfo);
    MI_S32 (*fpDestroy)(MI_RGN_WIN_HANDLER hHandle);
    MI_S32 (*fpSetAttr)(MI_RGN_WIN_HANDLER hHandle, MI_RGN_ChnPortParam_t *pstChnAttr);
    MI_S32 (*fpGetAttr)(MI_RGN_WIN_HANDLER hHandle, MI_RGN_ChnPortParam_t *pstChnAttr);
    MI_S32 (*fpGetChPort)(MI_RGN_WIN_HANDLER hHandle, MI_RGN_ChnPort_t *pstChPort);
    MI_S32 (*fpBlitOsdFrontBuffer)(MI_RGN_WIN_HANDLER hHandle, MI_RGN_CanvasInfo_t *pstCanvasInfo);
    MI_S32 (*fpGetFrontBuffer)(MI_U8 idx, MI_RGN_ChnPort_t *pstChnPort, MI_RGN_CanvasInfo_t *pstFrontBufInfo, MI_RGN_Point_t *pstPoint, MI_BOOL *bShow);
    MI_S32 (*fpGetBaseWindow)(MI_RGN_ChnPort_t *pstChnPort, MI_RGN_Size_t *pstBaseWin);

    MI_S32 (*fpUpdate)(MI_RGN_ChnPort_t *pstChnPort,  MI_RGN_Size_t *pstDispWindowSize, MI_BOOL bEn, MHAL_CMDQ_CmdqInterface_t* pstCmdInf, MI_U64 *pu64Fence);
    MI_S32 (*fpDone)(MI_U64 u64Fence);
}mi_rgn_WindowManger_t;
typedef struct MI_RGN_BufferManager_s
{
    MI_S32 (*fpBufferAlloc)(MI_RGN_CanvasInfo_t *pstCanvasInfo);
    void (*fpBufferDel)(MI_RGN_CanvasInfo_t *pstCanvasInfo);

    void (*fpOverlayBlit)(MI_U8 u8GfxColorFmtTo, MI_RGN_CanvasInfo_t *pstCanvasTo, MI_RGN_ChnPort_t *pstChnPort, MI_U8 u8Index);
}mi_rgn_BufferManager_t;


typedef struct mi_rgn_CmdPortAttr_s
{
    MI_BOOL bEnable;
    MI_U32 u32Width;
    MI_U32 u32Height;
}mi_rgn_CmdPortAttr_t;

typedef struct mi_rgn_ProcessCmdInfo_s
{
    MI_U32 u32chnID;
    union
    {
        mi_rgn_CmdPortAttr_t stVpePort[MI_RGN_VPE_PORT_MAXNUM];
        mi_rgn_CmdPortAttr_t stDivpPort[MI_RGN_DIVP_PORT_MAXNUM];
    };
}mi_rgn_ProcessCmdInfo_t;

#endif //_MI_RGN_DATATYPE_INTERNAL_
