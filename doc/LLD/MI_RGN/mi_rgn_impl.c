//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/lish.h>

#include "mi_rgn_datatype.h"
#include "mi_rgn_impl.h"
#include "mi_rgn_internal.h"

#define MI_RGN_MAX_GOP_GWIN_NUM 2
#define MI_RGN_MEMALLOC(x) kmalloc(x, GFP_ATOMIC)
#define MI_RGN_MEMFREE(x) kfree(x)

#define MI_RGN_HANDLE_CHECK(PARA, ERRCODE) do{  \
                                                if(PARA > MI_RGN_MAX_HANDLE) \
                                                { \
                                                    DBG_EXIT_ERR("Handle num out of range.\n"); \
                                                    {ERRCODE}; \
                                                }   \
                                             }while(0); 
#define MI_RGN_PTR_CHECK(PTR,ERRCODE) do {  \
                                            if(!PTR) \ 
                                            { \
                                                DBG_EXIT_ERR("parameter pointer is null.[%s,%d]\n",__func__,__line__); \
                                                {ERRCODE}; \
                                            }   \
                                      }while (0);

#define MI_RGN_RANGE_CHECK(PARA,MIN,MAX,ERRCODE) do{    \
                                                       if(PARA < MIN || PARA > MAX) \
                                                       { \
                                                           DBG_EXIT_ERR("parameter is out of range.[%s,%d]", __func__,__line__\n); \
                                                           {ERRCODE}; \
                                                       }    \
                                                   }while(0);
#define MI_RGN_WIDTH_ALIGN(WIDTH, ERRCODE) do{  \
                                                if (WIDTH % 2)   \
                                                {   \
                                                    DBG_EXIT_ERR("Width ALIGN false!\n");   \
                                                    {ERRCODE}; \
                                                }  \
                                            }

#define MI_RGN_MAP_PIXFORMAT_TO_MHAL(mhal, mi) do{  \
                                                    switch (mi) \
                                                    {   \
                                                        case E_MI_RGN_PIXEL_FORMAT_RGB_1555:    \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_RGB_1555;    \
                                                        }   \
                                                        break;  \
                                                        case E_MI_RGN_PIXEL_FORMAT_RGB_4444:    \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_RGB_4444;    \
                                                        }   \
                                                        break;  \
                                                        case E_MI_RGN_PIXEL_FORMAT_RGB_I2:  \
                                                        {  \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_RGB_I2;  \
                                                        }   \
                                                        break;  \
                                                        case E_MI_RGN_PIXEL_FORMAT_RGB_I4:  \
                                                        {   \
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_RGB_I4;  \
                                                        }   \
                                                        break;  \
                                                        default:    \
                                                        {
                                                            mhal = E_MHAL_RGN_PIXEL_FORMAT_RGB_MAX; \
                                                        }   \
                                                        break;  \
                                                    }   \
                                               }while (0);
#define MI_RGN_MAP_GOPID_TO_MHAL(mhalGopId, miPortId, miModeId) /*To do*/
#define MI_RGN_MAP_GWINID_TO_MHAL(mhalGwinId, miGwinId) /*To do*/
#define MI_RGN_MAP_COVERTYPE_TO_MHAL(mhalCoverType, miPortId, miModeId)  /*To do*/

typedef struct MI_RGN_DoubleBufferCanvasInfo_s
{
    MI_RGN_CanvasInfo_t stFront;
    MI_RGN_CanvasInfo_t stBackground;
    MI_RGN_Point_t stPoint;
}MI_RGN_DoubleBufferCanvasInfo_t;
typedef struct MI_RGN_Portdata_s
{	
	struct list_head chnPortList;
    MI_U16 u16CoverCnt;
    MI_U16 u16OverlayCnt;
    MI_BOOL bOneBufferMode;
    MI_RGN_DoubleBufferCanvasInfo_t stDoubleBuffer[MI_RGN_MAX_GOP_GWIN_NUM];
}MI_RGN_Portdata_t;

typedef struct MI_RGN_ChndataVpe_s
{
	MI_RGN_Portdata_t stPortData[MI_RGN_VPE_PORT_MAXNUM];
}MI_RGN_VpeChndata_t;

typedef struct MI_RGN_ChndataDivp_s
{
	MI_RGN_Portdata_t stPortData[MI_RGN_DIVP_PORT_MAXNUM];
}MI_RGN_DivpChndata_t;

typedef struct MI_RGN_HandleData_s
{
	struct list_head handleList;  
    MI_U16 u16ListCnt;
}MI_RGN_HandleListHeadData_t;

typedef struct MI_RGN_HandleListHead_s
{
    MI_BOOL bInitFlag;
    MI_BOOL bFreshHalInit;

    MI_RGN_HandleListHeadData_t stHandleData;
	MI_RGN_DivpChndata_t stDivpChn[MI_RGN_VPE_MAX_CH_NUM];
    MI_RGN_VpeChndata_t stVpeChn[MI_RGN_DIVP_MAX_CH_NUM];
}MI_RGN_HandleListHead_t;

typedef struct MI_RGN_ChPortListData_s
{
    list_head chnPortList;
    list_head handleToChPortList;

    MI_RGN_HandleListData_t *pstAttachedListData;
    MI_RGN_DoubleBufferCanvasInfo_t *pstLinkedDouleBuffer;
    MI_BOOL bFreshToFront;

    MI_RGN_ChnPort_t stAttachedChnPort;
    MI_RGN_ChnPortParam_t stAttachedChnPortPara;
}MI_RGN_ChPortListData_t;

typedef struct MI_RGN_HandleListData_s
{
	list_head handleList;
    list_head handleToChPortList;
    MI_S32 hHandle;
	MI_RGN_Attr_t stRegAttr;
	MI_RGN_CanvasInfo_t stCanvasInfo;
}MI_RGN_HandleListData_t;

typedef struct MI_RGN_BufferDelWorkListData_s
{
    list_head bufDelWorkList;
    MI_RGN_CanvasInfo_t stBuffer;
    MI_U32 u32Fence;
    //time stamp
}MI_RGN_BufferDelWorkListData_t;

DECLARE_MUTEX(RGN_List_Sem);
LIST_HEAD(_gstBufDelWorkList);
static struct MI_RGN_HandleListHead_t _stRegionlist;
static MI_RGN_PaletteTable_t _stPaletteTable;

static MI_BOOL _MI_RGN_IMPL_CheckCanvasInfoEmpty(MI_RGN_CanvasInfo_t *pstBuffer)
{
    /*Empty : return TRUE else return FALSE*/

    return FALSE;
}

static MI_S32 _MI_RGN_IMPL_MiSysDelBuffer(MI_RGN_CanvasInfo_t *pstBuffer)
{
    /*Use mi sys to do delete by the addr*/
    /*Fill the zero value*/
}
static MI_S32 _MI_RGN_IMPL_MiSysAllocBuffer(MI_RGN_CanvasInfo_t *pstBufferCfg)
{
}

MI_BOOL _MI_RGN_IMPL_CheckOsdOverLap(const MI_RGN_Portdata_t *pstChPortListHead, const MI_RGN_Point_t *pstPoint, const MI_RGN_Size_t *pstSize)
{
    /*Check all osd region if overlap*/
    return TRUE;
}
static MI_S32 _MI_RGN_IMPL_DelChPortListData(MI_RGN_Portdata_t * pstChnPortHead, MI_RGN_ChPortListData_t *pstChPortListData)
{
    MI_RGN_PTR_CHECK(pstChnPortHead, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChPortListData, return MI_ERR_RGN_NULL_PTR;);

    list_del(&pstChPortListData->chnPortList);
    switch (pstChPortListData->pstAttachedListData.stRegAttr.eType)
    {
        case E_MI_RGN_OSD:
        {
            pstChnPortHead->u16OverlayCnt--;
        }
        break;
        case E_MI_RGN_COVER:
        {
            pstChnPortHead->u16CoverCnt--;
        }
        break;
        default:
        {
            BUG();
        }
        break;
    }
    return MI_NOTICE_RGN_SUCCESS;
}


static MI_S32 _MI_RGN_IMPL_AddChPortListDataByLayer(MI_RGN_Portdata_t * pstChnPortHead, MI_RGN_ChPortListData_t *pstChPortListData)
{
    MI_RGN_ChPortListData_t *pos;

    /*If layer is conflict so return errorcode*/
    MI_RGN_PTR_CHECK(pstChnPortHead, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChPortListData, return MI_ERR_RGN_NULL_PTR;);

    switch (pstChPortListData->pstAttachedListData->stRegAttr.eType)
    {
        case E_MI_RGN_OSD:
        {
            pstChnPortHead->u16OverlayCnt++;
            list_add_tail(&pstChPortListData->chnPortList, &pstChnPortHead->chnPortList);
        }
        break;
        case E_MI_RGN_COVER:
        {
            if (!list_empty(pstChnPortHead->chnPortList))
            {
                list_add_tail(&pstChPortListData->chnPortList, &pstChnPortHead->chnPortList);
                pstChnPortHead->u16CoverCnt++;
            }
            else
            {
                list_for_each_entry(pos, &pstChPortListData->chnPortList, chnPortList)
                {
                    if (pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Layer == pos->stAttachedChnPortPara.stCoverPara.u32Layer)
                    {
                        return MI_ERR_RGN_ILLEGAL_PARAM;
                    }
                    (pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Layer > pos->stAttachedChnPortPara.stCoverPara.u32Layer)? continue;    \
                        list_add(&pstChPortListData->chnPortList, pos->chnPortList.prev);
                    pstChnPortHead->u16CoverCnt++;
                }
            }
        }
        break;
        default:
        {
            BUG();
        }
        break;
    }
    (pstChnPortHead->u16OverlayCnt <= MI_RGN_MAX_GOP_GWIN_NUM)?  \
        (pstChnPortHead->bOneBufferMode = FALSE):    \
        (pstChnPortHead->bOneBufferMode = TRUE);

    /*List sort by layer */
    return MI_NOTICE_RGN_SUCCESS;
}

static MI_S32 _MI_RGN_IMPL_DoesHandleListAttached(MI_RGN_HandleListData_t *pstHandleListData, MI_RGN_ChPortListData_t *pstChPortListData)
{
    MI_RGN_ChPortListData_t *pos;

    MI_RGN_PTR_CHECK(pstHandleListData, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChPortListData, return MI_ERR_RGN_NULL_PTR;);

    list_for_each_entry(pos, &pstHandleListData->handleToChPortList, handleToChPortList)
    {
        if (pos == pstChPortListData)
        {
            return 0;
        }
    }
    return 1;
}
static MI_S32 _MI_RGN_IMPL_DoesHandleListExist(MI_RGN_HANDLE hHandle)
{
    MI_RGN_HandleListData_t *pstLd = NULL;

	MI_RGN_HANDLE_CHECK(hHandle, return MI_ERR_RGN_INVALID_HANDLE);

    list_for_each_entry_safe(pstLd, &_stRegionlist.stHandleData.handleList, handleList);
    {
        if (hHandle == pstLd->hHandle)
        {
            return 1; //Handle list exist.
        }
    }
    return 0;
}

static MI_S32 _MI_RGN_IMPL_ClearDoubleBuffer(const MI_RGN_Portdata_t * pstChnPortHead)
{
    MI_U8 i = 0;
    MI_RGN_PTR_CHECK(pstChnPortHead, return MI_ERR_RGN_NULL_PTR;);

    for(i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
    {
        if (!_MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortHead->stDoubleBuffer[i].stBackground))
        {
            if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_EnqueueBufDelWork(&pstChnPortHead->stDoubleBuffer[i].stBackground))
            {
                DBG_EXIT_ERR("MiSys del work buffer error!\n");
                return  MI_ERR_RGN_NOMEM;
            }
        }
        if (!_MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortHead->stDoubleBuffer[i].stFront))
        {
            if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_EnqueueBufDelWork(&pstChnPortHead->stDoubleBuffer[i].stFront))
            {
                DBG_EXIT_ERR("MiSys del work buffer error!\n");
                return  MI_ERR_RGN_NOMEM;
            }
        }
    }

    return MI_NOTICE_RGN_SUCCESS;
}
static MI_S32 _MI_RGN_IMPL_DestroyDoubleBuffer(const MI_RGN_Portdata_t * pstChnPortHead, const MI_RGN_ChPortListData_t *pstChnPortListData)
{
    MI_U8 i = 0;
    MI_RGN_PTR_CHECK(pstChnPortHead, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChnPortListData, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChnPortListData->pstLinkedDouleBuffer, return MI_ERR_RGN_NULL_PTR;);

    if (pstChnPortHead->bOneBufferMode == FALSE)
    {
        if (pstChnPortHead->stDoubleBuffer[i].stBackground.PhyAddr != &pstChnPortHead->stDoubleBuffer[i].stFront.PhyAddr)
        {
            DBG_EXIT_ERR("MiSys del buffer error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
        if (!_MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortListData->pstLinkedDouleBuffer->stBackground))
        {
            if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_EnqueueBufDelWork(&pstChnPortListData->pstLinkedDouleBuffer->stBackground))
            {
                DBG_EXIT_ERR("MiSys del work buffer error!\n");
                return  MI_ERR_RGN_NOMEM;
            }
            memset(&pstChnPortListData->pstLinkedDouleBuffer->stFront);
        }
    }
    else
    {
        if (!_MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortHead->stDoubleBuffer[0].stBackground)&&   \
            !_MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortHead->stDoubleBuffer[0].stFront) &&   \
            pstChnPortHead->stDoubleBuffer[0].stBackground.PhyAddr != pstChnPortHead->stDoubleBuffer[0].stFront.PhyAddr)
        {
            if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_EnqueueBufDelWork(&_MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortHead->stDoubleBuffer[0].stBackground))
            {
                DBG_EXIT_ERR("MiSys del work buffer error!\n");
                return  MI_ERR_RGN_NOMEM;
            }
            if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_EnqueueBufDelWork(&_MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortHead->stDoubleBuffer[0].stFront))
            {
                DBG_EXIT_ERR("MiSys del work buffer error!\n");
                return  MI_ERR_RGN_NOMEM;
            }
        }

    }
    pstChnPortListData->bFreshToFront = TRUE;

    return MI_NOTICE_RGN_SUCCESS;
}


static MI_S32 _MI_RGN_IMPL_CreateDoubleBuffer(const MI_RGN_Portdata_t * pstChnPortHead, const MI_RGN_ChPortListData_t *pstChnPortListData)
{
    MI_U8 i = 0;
    MI_RGN_PTR_CHECK(pstChnPortHead, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChnPortListData, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChnPortListData->pstLinkedDouleBuffer, return MI_ERR_RGN_NULL_PTR;);

    if (pstChnPortHead->bOneBufferMode == FALSE)
    {
        for(i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
        {
            if (_MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortHead->stDoubleBuffer[i].stBackground) && _MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortHead->stDoubleBuffer[i].stFront))
            {
                memcpy(&pstChnPortHead->stDoubleBuffer[i].stBackground, &pstChnPortListData->pstAttachedListData->stCanvasInfo, sizeof(MI_RGN_CanvasInfo_t));
                if (-1 == _MI_RGN_IMPL_MiSysAllocBuffer(&pstChnPortHead->stDoubleBuffer[i].stFront))
                {
                    DBG_EXIT_ERR("MiSys alloc buffer error!\n");
                    return  MI_ERR_RGN_NOMEM;
                }
                if (_MI_RGN_IMPL_BlitBuffer(&pstChnPortHead->stDoubleBuffer[i].stFront, &pstChnPortHead->stDoubleBuffer[i].stBackground, 0, 0))
                {
                    DBG_EXIT_ERR("Buffer blit error!\n");
                    return  MI_ERR_RGN_NOMEM;
                }
                pstChnPortHead->stDoubleBuffer.stPoint.u32X = pstChnPortListData->stAttachedChnPortPara.stPoint.u32X;
                pstChnPortHead->stDoubleBuffer.stPoint.u32Y = pstChnPortListData->stAttachedChnPortPara.stPoint.u32Y;
                pstChnPortListData->pstLinkedDouleBuffer = &pstChnPortHead->stDoubleBuffer[i];
                break;
            }
            if (i == MI_RGN_MAX_GOP_GWIN_NUM)
            {
                DBG_EXIT_ERR("MiSys alloc buffer error!\n");
                return  MI_ERR_RGN_NOMEM;
            }
        }
    }
    else
    {

        if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_ClearDoubleBuffer(pstChnPortHead))
        {
            DBG_EXIT_ERR("MiSys clear buffer error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
        /*New*/
        if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_MakeListBufferTogether(&pstChnPortHead->stDoubleBuffer[0].stBackground, &pstChnPortHead->stDoubleBuffer.stPoint, pstChnPortHead))
        {
            DBG_EXIT_ERR("_MI_RGN_IMPL_MakeListBufferTogether buffer error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
        memcpy(&pstChnPortHead->stDoubleBuffer[0].stFront, &pstChnPortHead->stDoubleBuffer[0].stBackground, sizeof(MI_RGN_CanvasInfo_t));
        if (-1 == _MI_RGN_IMPL_MiSysAllocBuffer(&pstChnPortHead->stDoubleBuffer[0].stFront))
        {
            DBG_EXIT_ERR("MiSys alloc buffer error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
        if (_MI_RGN_IMPL_BlitBuffer(&pstChnPortHead->stDoubleBuffer[0].stFront, &pstChnPortHead->stDoubleBuffer[0].stBackground, 0, 0))
        {
            DBG_EXIT_ERR("Buffer blit error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
        pstChnPortListData->pstLinkedDouleBuffer = &pstChnPortHead->stDoubleBuffer[0];
    }

    return MI_NOTICE_RGN_SUCCESS;
}
static MI_S32 _MI_RGN_IMPL_DoUpdataCanvas(const MI_RGN_Portdata_t * pstChnPortHead, const MI_RGN_ChPortListData_t *pstChnPortListData)
{
    MI_RGN_ChPortListData_t *pstChnPortListData = NULL;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;

    MI_RGN_PTR_CHECK(pstChnPortHead, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChnPortListData, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChnPortListData->pstLinkedDouleBuffer, return MI_ERR_RGN_NULL_PTR;);

    if (pstChnPortListHead->bOneBufferMode)
    {
        if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_ClearDoubleBuffer(pstChnPortListHead))
        {
            DBG_EXIT_ERR("MiSys clear buffer error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
        /*New*/
        if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_MakeListBufferTogether(&pstChnPortListHead->stDoubleBuffer[0].stBackground, &pstChnPortListHead->stPoint, pstChnPortHead))
        {
            DBG_EXIT_ERR("_MI_RGN_IMPL_MakeListBufferTogether buffer error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
        memcpy(&pstChnPortListHead->stDoubleBuffer[0].stFront, &pstChnPortListHead->stDoubleBuffer[0].stBackground, sizeof(MI_RGN_CanvasInfo_t));
        if (-1 == _MI_RGN_IMPL_MiSysAllocBuffer(&pstChnPortListHead->stDoubleBuffer[0].stFront))
        {
            DBG_EXIT_ERR("MiSys alloc buffer error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
        if (_MI_RGN_IMPL_BlitBuffer(&pstChnPortListHead->stDoubleBuffer[0].stFront, &pstChnPortListHead->stDoubleBuffer[0].stBackground, 0, 0))
        {
            DBG_EXIT_ERR("Buffer blit error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
    }
    else
    {
        MI_RGN_PTR_CHECK(pstChnPortListData->pstLinkedDouleBuffer, return MI_ERR_RGN_NULL_PTR);
        if (pstChnPortListData->pstLinkedDouleBuffer->stBackground.PhyAddr != pstChnPortListData->pstLinkedDouleBuffer->stFront.PhyAddr)
        {
            DBG_EXIT_ERR("Front buffer error!\n");
            return  MI_ERR_RGN_NOMEM;
        }
        if (!_MI_RGN_IMPL_CheckCanvasInfoEmpty(pstChnPortListData->pstLinkedDouleBuffer->stBackground))
        {
            if (_MI_RGN_IMPL_BlitBuffer(pstChnPortListData->pstLinkedDouleBuffer->stBackground, &pstLd->stCanvasInfo, 0, 0))
            {
                DBG_EXIT_ERR("Buffer blit error!\n");
                return  MI_ERR_RGN_NOMEM;
            }
        }
    }

    return MI_NOTICE_RGN_SUCCESS;

}
static MI_U32 _MI_RGN_IMPL_CalPixStride(MI_RGN_PixelFormat_e ePixelFmt, const MI_RGN_Size_t *pstSize)
{
    /*Calculate stride by pix format & w/h*/
    /*align = 128/8/bitperpixel
     *E_HAL_REG_PIXEL_FORMAT_RGB_1555 , =>128/8/2= 8 pixel  align
     *E_HAL_REG_PIXEL_FORMAT_RGB_4444,  =>128/8/2= 8 align
     *E_HAL_REG_PIXEL_FORMAT_RGB_I2,       =>128/8/1= 16 pixel  align
     *E_HAL_REG_PIXEL_FORMAT_RGB_I4        =>128/8/1 =16 pixel  align
     *For exsample :
     *Color Format : I2
     *Width=1080, Height=800
     *Stride is by 16 bit align which caculated as 1088.
     */
}

static MI_U32 _MI_RGN_IMPL_CalBufSize(MI_RGN_PixelFormat_e ePixelFmt, const MI_RGN_Size_t *pstSize)
{
    /*E_HAL_REG_PIXEL_FORMAT_RGB_1555  = 2byte perpixel*/
    /*E_HAL_REG_PIXEL_FORMAT_RGB_4444  = 2byte perpixel*/
    /*E_HAL_REG_PIXEL_FORMAT_RGB_I2  = 2bit perpixel*/
    /*E_HAL_REG_PIXEL_FORMAT_RGB_I4  = 4bit perpixel*/

    /*Calculate stride by pix format & w/h*/
    /*Memery = Stride x Weight x byteperpixel*/
}
static MI_S32 _MI_RGN_IMPL_BlitBuffer(MI_RGN_CanvasInfo_t *pstCurBuffer, const MI_RGN_CanvasInfo_t* pstRequireBufInfo, MI_S32 s32X, MI_S32 s32Y)
{
    /*If the two buffer match the size and phy/vir addr must be available*/
    /*Copy buffer data from pstRequireBufInfo to pstCurBuffer*/

    /*Use gfx api*/
}
/*pstBuffer : Combine buffer output*/
static MI_S32 _MI_RGN_IMPL_MakeListBufferTogether(MI_RGN_CanvasInfo_t *pstBuffer, const MI_RGN_Point_t *pstPoint, const MI_RGN_Portdata_t *pstChPortListHead)
{
    /*need check bUpdatetoVancas & bShow & Layer */
    return MI_NOTICE_RGN_SUCCESS;
}

static MI_S32 _MI_RGN_IMPL_EnqueueBufDelWork(MI_RGN_CanvasInfo_t *pBuffer)
{
    MI_RGN_BufferDelWorkListData_t *pstBufDelWorkList;

    MI_RGN_PTR_CHECK(pBuffer, return MI_ERR_RGN_NULL_PTR;);

    pstBufDelWorkList = MI_RGN_MEMALLOC(sizeof(MI_RGN_BufferDelWorkListData_t));
    MI_RGN_PTR_CHECK(pstBufDelWorkList, return MI_ERR_RGN_NOMEM);
    memcpy(&pstBufDelWorkList->stBuffer, pBuffer, sizeof(MI_RGN_CanvasInfo_t));
    memset(pBuffer, 0, sizeof(MI_RGN_CanvasInfo_t));
    pBuffer->VirtAddr = NULL;
    pBuffer->PhyAddr = NULL;
    list_add_tail(&pstBufDelWorkList->bufDelWorkList, &_gstBufDelWorkList);

    return MI_NOTICE_RGN_SUCCESS;
}

static MI_S32 _MI_RGN_IMPL_DequeueBufDelWork(void)
{
    MI_RGN_BufferDelWorkListData_t *pstBufWorkList, *pstBufWorkListN;

    list_for_each_entry_safe(pstBufWorkList, pstBufWorkListN, _gstBufDelWorkList, bufDelWorkList)
    {
        if (-1 == _MI_RGN_IMPL_MiSysDelBuffer(&pstBufWorkList->stBuffer))
        {
            DBG_EXIT_ERR("MiSys del buffer error!\n");
            return MI_ERR_RGN_BADADDR;
        }
        list_del(&pstBufWorkList->bufDelWorkList);
        MI_RGN_MEMFREE(pstBufWorkList);
    }

    return MI_NOTICE_RGN_SUCCESS;
}

static MI_S32 _MI_RGN_IMPL_UpdateChPortListData(const MI_RGN_Portdata_t *pstChnPortHead, const MI_RGN_ChnPort_t *pstChnPort, const MI_RGN_Size_t *pstDispWindowSize, MI_BOOL bEn, const cmd_mload_interface* pstCmdInf)
{
    /*After do _MI_RGN_LoadChPortListData enalbe hal config and change to the other buffer(double buffer case)*/
    MI_U8 i = 0;
    MI_S32 s32ErrorCode;
    MI_RGN_ChPortListData_t *pstChPortListData = NULL;
    MHAL_RGN_GopIdType_e eGopId;
    MHAL_RGN_GopGwinIdType_e eGwinId;
    MHAL_RGN_ConverType_e eCoverType;
    MHAL_RGN_CoverLayer_e eLayer = E_DRV_COVER_LAYER_0;
    MHAL_RGN_PixelFormat_e eFormat;
    MI_S32 s32ErrorCode;

    /*1. Do cover case*/
    MI_RGN_PTR_CHECK(pstChnPortHead, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstDispWindowSize, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstCmdInf, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChnPort, return MI_ERR_RGN_NULL_PTR;);
    
    if (_stRegionlist.bFreshHalInit == FALSE)
    {
        MHAL_RGN_CoverInit();
        MHAL_RGN_GopInit(&_stPaletteTable, pstCmdInf);
        _stRegionlist.bFreshHalInit = TRUE;
    }
    list_for_each_entry(pstChPortListData, pstChnPortHead, chnPortList)
    {
        if (E_MI_RGN_COVER == pstChPortListData->pstAttachedListData->stRegAttr.eType)
        {
            if (bEn && pstChPortListData->bFreshToFront)
            {
                if (pstChPortListData->stAttachedChnPortPara.bShow)
                {
                    MI_RGN_MAP_COVERTYPE_TO_MHAL(eCoverType, pstChnPort->s32OutputPortId, pstChnPort->eModId);
                    MHAL_RGN_CoverDisable(eCoverType, eLayer, pstCmdInf);
                    MHAL_RGN_CoverSetBaseWindow(eCoverType, (MS_U32)pstDispWindowSize->u32Width, (MS_U32)pstDispWindowSize->u32Height, pstCmdInf);
                    MHAL_RGN_CoverSetSize(eCoverType, eLayer, (MS_U32)pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Width, (MS_U32)pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Height, pstCmdInf);
                    MHAL_RGN_CoverSetColor(eCoverType, eLayer, (MS_U32)pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Color, pstCmdInf);
                    MHAL_RGN_CoverSetPos(eCoverType, eLayer, (MS_U32)pstChPortListData->stAttachedChnPortPara.stPoint.u32X, (MS_U32)pstChPortListData->stAttachedChnPortPara.stPoint.u32Y, pstCmdInf);
                    MHAL_RGN_CoverEnable(eCoverType, eLayer, pstCmdInf);
                    DBG_INFO("Cover id: %d layer: %d Height: %d width: %d x: %d y: %d Color: R %d G %d B %d\n", (int)eCoverType, (int)eLayer,   \
                        (int)pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Height, (int)(pstChPortListData->stAttachedChnPortPara.stCoverPara.stSize.u32Height),   \
                        pstChPortListData->stAttachedChnPortPara.stPoint.u32X, (int)(pstChPortListData->stAttachedChnPortPara.stPoint.u32Y),    \
                        (int)(pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Color & 0xF00) >> 8,      \
                        (int)(pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Color & 0xF0) >> 4 ,     \
                        (int)(pstChPortListData->stAttachedChnPortPara.stCoverPara.u32Color & 0xF));
                }
                else
                {
                    MI_RGN_MAP_COVERTYPE_TO_MHAL(eCoverType, pstChnPort->s32OutputPortId, pstChnPort->eModId);
                    MHAL_RGN_CoverDisable(eCoverType, eLayer, pstCmdInf);
                }
                pstChPortListData->bFreshToFront == FALSE;
            }
            eLayer++;
        }
    }
    MI_RGN_MAP_GOPID_TO_MHAL(eGopId, pstChnPort->s32OutputPortId, pstChnPort->eModId);
    for (i = 0; i < MI_RGN_MAX_GOP_GWIN_NUM; i++)
    {
        if (bEn && pstChPortListData->bFreshToFront)
        {
            if (!_MI_RGN_IMPL_CheckCanvasInfoEmpty(&pstChnPortHead->stDoubleBuffer[i]) && pstChPortListData->stAttachedChnPortPara.bShow)
            {
                MI_RGN_MAP_GWINID_TO_MHAL(eGwinId, i);
                MI_RGN_MAP_PIXFORMAT_TO_MHAL(eFormat, pstChPortListData->pstAttachedListData->stCanvasInfo.ePixelFmt);
                MHAL_RGN_GopGwinDisable(eGopId, eGwinId, pstCmdInf);
                MHAL_RGN_GopSetBaseWindow(eGopId, (MS_U32)pstDispWindowSize->u32Width, (MS_U32)pstDispWindowSize->u32Height, pstCmdInf);
                MHAL_RGN_GopGwinSetPixelFormat(eGopId, eGwinId, eFormat, pstCmdInf);
                MHAL_RGN_GopGwinSetPos(eGopId, eGwinId, (MS_U32)pstChnPortHead->stDoubleBuffer[i]->stPoint.u32X,    \
                    (MS_U32)pstChnPortHead->stDoubleBuffer[i]->stPoint.u32Y, pstCmdInf);
                MHAL_RGN_GopGwinSetSize(eGopId, eGwinId, (MS_U32)pstChnPortHead->stDoubleBuffer[i].stFront.stSize.u32Width,     \
                    (MS_U32)pstChnPortHead->stDoubleBuffer[i].stFront.stSize.u32Height, (MS_U32)pstChnPortHead->stDoubleBuffer[i].stFront.u32Stride, pstCmdInf);
                MHAL_RGN_GopGwinSetBuffer(eGopId, eGwinId,  (MS_U32)pstChnPortHead->stDoubleBuffer[i].stFront.PhyAddr, pstCmdInf);
                MHAL_RGN_GopGwinEnable(eGopId, eGwinId, pstCmdInf);
                DBG_INFO("Enable gwin: Gop id %d GwinId %d Format %d W %d H %d\n", (int)eGopId, (int)eGwinId, (int)eFormat, (int)pstDispWindowSize->u32Height, (int)pstDispWindowSize->u32Width);
           }
           else
           {
                MI_RGN_MAP_GWINID_TO_MHAL(eGwinId, i);
                MHAL_RGN_GopGwinDisable(eGopId, eGwinId, pstCmdInf);
           }           
           pstChPortListData->bFreshToFront == FALSE;
       }
    }
    
ERROR_CODE:
    return s32ErrorCode;
}
static MI_S32 _MI_RGN_IMPL_CheckRgnOsdAttr(MI_RGN_OsdInitParam_t *pstOsdInitPara)
{
	MI_RGN_RANGE_CHECK(pstOsdInitPara->stSize.u32Height, MI_RGN_MIN_WIDTH, MI_RGN_MAX_WIDTH, return MI_ERR_RGN_ILLEGAL_PARAM);
	MI_RGN_RANGE_CHECK(pstOsdInitPara->stSize.u32Width, MI_RGN_MIN_HEIGHT, MI_RGN_MAX_HEIGHT, return MI_ERR_RGN_ILLEGAL_PARAM);
    MI_RGN_WIDTH_ALIGN(pstOsdInitPara->stSize.u32Width, return MI_ERR_RGN_ILLEGAL_PARAM);

    /*To do need check ePixelFmt align.*/

    return MI_NOTICE_RGN_SUCCESS;
}
static MI_S32 _MI_RGN_IMPL_CheckCanvasBufAttr(MI_RGN_CanvasInfo_t *pstCanvasInfo)
{
	MI_RGN_RANGE_CHECK(pstCanvasInfo->stSize.u32Height, MI_RGN_MIN_WIDTH, MI_RGN_MAX_WIDTH, return MI_ERR_RGN_ILLEGAL_PARAM);
	MI_RGN_RANGE_CHECK(pstCanvasInfo->stSize.u32Width, MI_RGN_MIN_HEIGHT, MI_RGN_MAX_HEIGHT, return MI_ERR_RGN_ILLEGAL_PARAM);
    MI_RGN_WIDTH_ALIGN(pstCanvasInfo->stSize.u32Width, return MI_ERR_RGN_ILLEGAL_PARAM);

    /*To do need check ePixelFmt align.*/

    return MI_NOTICE_RGN_SUCCESS;
}
static MI_S32 _MI_RGN_IMPL_GetHandleListData(MI_S32 hHandle, MI_RGN_HandleListData_t**ppHandleListData)
{
    MI_RGN_HandleListData_t *pstLd = NULL;

    MI_RGN_PTR_CHECK(ppHandleListData, return MI_ERR_RGN_NULL_PTR;);

    list_for_each_entry(pstLd, &_stRegionlist.stHandleData.handleList, handleList);
    {
        if (hHandle == pstLd->hHandle)
        {
            *ppHandleListData = pstLd;
        }
    }
    if (&pstLd->handleList == &_stRegionlist.stHandleData.handleList)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        return MI_ERR_RGN_INVALID_HANDLE;
    }

    return MI_NOTICE_RGN_SUCCESS;
}
static MI_S32 _MI_RGN_IMPL_GetChnPortListHead(const MI_RGN_ChnPort_t* pstChnPort, MI_RGN_Portdata_t **ppstChnPortListHead)
{

    MI_RGN_PTR_CHECK(pstChnPort, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(ppstChnPortListHead, return MI_ERR_RGN_NULL_PTR;);

    switch (pstChnPort->eModId)
    {
        case E_MI_RGN_VPE:
        {
            MI_RGN_RANGE_CHECK(pstChnPort->s32ChnId, 0, MI_RGN_VPE_MAX_CH_NUM - 1, return MI_ERR_RGN_ILLEGAL_PARAM);
            MI_RGN_RANGE_CHECK(pstChnPort->s32OutputPortId, 0, MI_RGN_VPE_MAX_CH_NUM - 1, return MI_ERR_RGN_ILLEGAL_PARAM);
            *ppstChnPortListHead = &_stRegionlist.stVpeChn[pstChnPort->s32ChnId].stPortData[pstChnPort->s32OutputPortId];
        }
        break;
        case E_MI_RGN_DIVP:
        {
            MI_RGN_RANGE_CHECK(pstChnPort->s32ChnId, 0, MI_RGN_DIVP_MAX_CH_NUM - 1, return MI_ERR_RGN_ILLEGAL_PARAM);
            MI_RGN_RANGE_CHECK(pstChnPort->s32OutputPortId, 0, MI_RGN_DIVP_MAX_CH_NUM - 1, return MI_ERR_RGN_ILLEGAL_PARAM);
            *ppstChnPortListHead = &_stRegionlist.stDivpChn[pstChnPort->s32ChnId].stPortData[pstChnPort->s32OutputPortId];
        }
        break;
        default:
        {
            return MI_ERR_RGN_ILLEGAL_PARAM;
        }
        break;
    }

    return MI_NOTICE_RGN_SUCCESS;
}

MI_S32 MI_RGN_IMPL_Init(const MI_RGN_PaletteTable_t *pstPaletteTable)
{
    MI_U8 i = 0, j = 0, k = 0;

	if (_stRegionlist.bInitFlag == TRUE)
	{
        return MI_ERR_RGN_BUSY;
	}
    memset(&_stRegionlist, 0, sizeof(MI_RGN_HandleListHead_t));
    for ( i = 0; i < MI_RGN_VPE_MAX_CH_NUM; i++)
    {
        for (; j < MI_RGN_VPE_PORT_MAXNUM; j++)
        { 
            INIT_LIST_HEAD(&_stRegionlist.stVpeChn[i].stPortData[j].chnPortList);
            for (; k < MI_RGN_MAX_GOP_GWIN_NUM, k++)
            {                
                memset(&_stRegionlist.stVpeChn[i].stPortData[j].stDoubleBuffer[k], 0, sizeof(MI_RGN_DoubleBufferCanvasInfo_t));
            }
        }      
    }
    for (i = 0; i < MI_RGN_DIVP_MAX_CH_NUM; i++)
    {
        for (j = 0; j < MI_RGN_DIVP_PORT_MAXNUM; j++)
        { 
            INIT_LIST_HEAD(&_stRegionlist.stDivpChn[i].stPortData[j].chnPortList);
            for (k = 0; k < MI_RGN_MAX_GOP_GWIN_NUM, k++)
            {                
                memset(&_stRegionlist.stDivpChn[i].stPortData[j].stDoubleBuffer[k], 0, sizeof(MI_RGN_DoubleBufferCanvasInfo_t));
            }
        } 
    }
    INIT_LIST_HEAD(&_stRegionlist.stHandleData.handleList);
    _stRegionlist.bInitFlag = TRUE;

    memcpy(&_stPaletteTable, pstPaletteTable, sizeof(MI_RGN_PaletteTable_t));
    return MI_NOTICE_RGN_SUCCESS;
}
MI_S32 MI_RGN_IMPL_DeInit()
{
	if (_stRegionlist.bInitFlag == FALSE)
	{
        return MI_ERR_RGN_BUSY;
	}

    return MI_NOTICE_RGN_SUCCESS;
}

MI_S32 MI_RGN_IMPL_Create(MI_RGN_HANDLE hHandle, const MI_RGN_Attr_t *pstRegion)
{
	/*1.var define*/
    MI_RGN_HandleListData_t *pstLd = NULL, *pstLdN = NULL;
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;

    DBG_ENTER();
	MI_RGN_PTR_CHECK(pstRegion, return MI_ERR_RGN_NULL_PTR);
    MI_RGN_HANDLE_CHECK(hHandle, return MI_ERR_RGN_INVALID_HANDLE);

    /*Tommy: Module init*/

	/*4.normal code*/

    down(&RGN_List_Sem);
    if (_MI_RGN_IMPL_DoesHandleListExist(hHandle))
    {
        DBG_EXIT_ERR("Handle is exist.\n");        
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    if (_stRegionlist.stHandleData.u16ListCnt == MI_RGN_MAX_HANDLE)
    {
        DBG_EXIT_ERR("Handle num is max\n");
        s32ErrorCode = MI_ERR_RGN_NOMEM;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }

    /*Apply list node buffer & fill the data*/
    /*Tommy kmalloc*/
	pstLd = MI_RGN_MEMALLOC(sizeof(MI_RGN_HandleListData_t);
    MI_RGN_PTR_CHECK(pstLd, s32ErrorCode = MI_ERR_RGN_NOMEM;    \
                                up(&RGN_List_Sem);  \
                                goto ERROR_CODE;);
	memset(pstLd, 0, sizeof(MI_RGN_HandleListData_t));
    INIT_LIST_HEAD(&pstLd->handleToChPortList);
	pstLd->hHandle = hHandle;

    switch (pstRegion->eType)
    {
        case E_MI_RGN_OSD:
        {
            /***************
                Apply buffer from mi_sys
            ***************/
            
            s32ErrorCode = _MI_RGN_IMPL_CheckRgnOsdAttr(&pstRegion->stOsdInitParam);
            if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
            {
                DBG_EXIT_ERR("ePixelFmt error!\n");
                s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
                MI_RGN_MEMFREE(pstLd);
                up(&RGN_List_Sem);
                goto ERROR_CODE;
            }
            pstLd->stCanvasInfo.ePixelFmt = pstRegion->stOsdInitParam.ePixelFmt;
            pstLd->stCanvasInfo.stSize.u32Height = pstRegion->stOsdInitParam.stSize.u32Height;
            pstLd->stCanvasInfo.stSize.u32Width = pstRegion->stOsdInitParam.stSize.u32Width;
            pstLd->stCanvasInfo.u32Stride = _MI_RGN_IMPL_CalPixStride(pstRegion->stOsdInitParam.ePixelFmt, &pstRegion->stOsdInitParam.stSize);
            if (-1 == _MI_RGN_IMPL_MiSysAllocBuffer(&pstLd->stCanvasInfo))
            {
                DBG_EXIT_ERR("MiSys alloc buffer error!\n");
                s32ErrorCode = MI_ERR_RGN_NOMEM;
                MI_RGN_MEMFREE(pstLd);
                up(&RGN_List_Sem);
                goto ERROR_CODE;
            }

        }
        break;
        case E_MI_RGN_COVER:
        {
            /***************
            todo:
                do cover case.
            ***************/
        }
        break;
        default:
        {
            DBG_EXIT_ERR("Region type error!\n");
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            MI_RGN_MEMFREE(pstLd);
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
        break;
    }
	memcpy(&pstLd->stRegAttr,pstRegion,sizeof(MI_RGN_Attr_t));
	list_add_tail(&pstLd->handleList,&_stRegionlist.stHandleData.list);
    _stRegionlist.stHandleData.u16ListCnt++;
    up(&RGN_List_Sem);

    DBG_EXIT_OK();
	return MI_NOTICE_RGN_SUCCESS;

ERROR_CODE:
    return s32ErrorCode;
}

MI_S32 MI_RGN_IMPL_Destroy (MI_RGN_HANDLE hHandle)
{
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_ChPortListData_t *pstCpld = NULL, *pstCpldN = NULL;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;

    DBG_ENTER();

    /*1. Find each handle list if not return false*/
    down(&RGN_List_Sem);
    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (MI_NOTICE_RGN_SUCCESS == s32ErrorCode)
    {
        /*2. If find the handle then look up for every chPortData and find their's head*/
        list_for_each_entry_safe(pstCpld, pstCpldN, &pstLd->handleToChPortList, handleToChPortList)
        {       
        
            /*3. List del*/ 
            /*4. Delete handleToChPortList*/
            list_del(&pstCpld->handleToChPortList);                
            list_del(&pstCpld->chnPortList);
            s32ErrorCode = _MI_RGN_IMPL_GetChnPortListHead(&pstCpld->stAttachedChnPort, &pstChnPortListHead);
            if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
            {
                DBG_EXIT_ERR("Get ch port List fail.\n");
                up(&RGN_List_Sem);
                goto ERROR_CODE;
            }
            if (_MI_RGN_IMPL_DestroyDoubleBuffer(pstChnPortListHead, pstCpld) != MI_NOTICE_RGN_SUCCESS)
            {
                DBG_EXIT_ERR("Do del front buffer error!.\n");
                s32ErrorCode = MI_ERR_RGN_NOT_PERM;
                up(&RGN_List_Sem);
                goto ERROR_CODE;
            }
            if (_MI_RGN_IMPL_DelChPortListData(pstChnPortListHead, pstCpld) != MI_NOTICE_RGN_SUCCESS)
            {
                DBG_EXIT_ERR("Do del ch port list error!.\n");
                s32ErrorCode = MI_ERR_RGN_NOT_PERM;
                up(&RGN_List_Sem);
                goto ERROR_CODE;
            }
            MI_RGN_MEMFREE(pstCpld);
        }
        /*5. return region buffer*/
        switch (pstLd->stRegAttr.eType)
        {
            case E_MI_RGN_OSD:
            {
                /***************
                    Delete region buffer from mi_sys
                ***************/
                if (-1 == _MI_RGN_IMPL_MiSysDelBuffer(&pstLd->stCanvasInfo))
                {
                    DBG_EXIT_ERR("MiSys del buffer error!\n");
                    s32ErrorCode = MI_ERR_RGN_BADADDR;            
                    up(&RGN_List_Sem);
                    goto ERROR_CODE;
                }
            }
            break;
            case E_MI_RGN_COVER:
            {
                /***************
                todo:
                    do cover destroy case.
                ***************/
            }
            break;
            default:
            {
                BUG();
            }
            break;
        }
        _stRegionlist.stHandleData.u16ListCnt--;
        MI_RGN_MEMFREE(pstLd);
    }
    else
    {
        DBG_EXIT_ERR("Handle not found.\n");
        up(&RGN_List_Sem);
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        goto ERROR_CODE;
    }
    up(&RGN_List_Sem);
    return MI_NOTICE_RGN_SUCCESS;

ERROR_CODE:
    return s32ErrorCode;
}

MI_S32 MI_RGN_IMPL_GetAttr(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion)
{
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;
    MI_RGN_HandleListData_t *pstLd = NULL, *pstLdN = NULL;

    DBG_ENTER();

    /*1. Check handle avaliable. Check para.*/
	MI_RGN_PTR_CHECK(pstRegion, return MI_ERR_RGN_NULL_PTR);

    down(&RGN_List_Sem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (MI_NOTICE_RGN_SUCCESS == s32ErrorCode)
    {
        memcpy(pstRegion, &pstLd->stRegAttr);
    }
    else
    {
        DBG_EXIT_ERR("Handle not found.\n");
        up(&RGN_List_Sem);
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        goto ERROR_CODE;
    }
    up(&RGN_List_Sem);
    
    
    return MI_NOTICE_RGN_SUCCESS;
        
ERROR_CODE:
    return s32ErrorCode;
}
MI_S32 MI_RGN_IMPL_SetBitMap(MI_RGN_HANDLE hHandle, const MI_RGN_Bitmap_t *pstBitmap)
{
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;
    MI_RGN_CanvasInfo_t stCanvasInfo;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_ChPortListData_t *pstChnPortListData = NULL;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;

    DBG_ENTER();
    /*1. Check handle avaliable. Check para.*/
	MI_RGN_PTR_CHECK(pstBitmap, return MI_ERR_RGN_NULL_PTR);
    MI_RGN_PTR_CHECK(pstBitmap->pData, return MI_ERR_RGN_NULL_PTR);

    down(&RGN_List_Sem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (MI_NOTICE_RGN_SUCCESS == s32ErrorCode)
    {
        if (pstLd->stRegAttr.eType != E_MI_RGN_OSD)
        {
            DBG_EXIT_ERR("Reg type error.\n");
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
        if (pstLd->stRegAttr.stOsdInitParam.ePixelFmt ! = stCanvasInfo.ePixelFmt)
        {
            DBG_EXIT_ERR("Pix fmt not match.\n");
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
        stCanvasInfo.ePixelFmt = pstBitmap->ePixelFormat;
        stCanvasInfo.stSize.u32Height = pstBitmap->stSize.u32Height;
        stCanvasInfo.stSize.u32Width = pstBitmap->stSize.u32Width;
        s32ErrorCode = _MI_RGN_IMPL_CheckCanvasBufAttr(&stCanvasInfo);
        if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
        {
            DBG_EXIT_ERR("ePixelFmt error!\n");
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
        copy_from_user(&stCanvasInfo.VirtAddr, &pstBitmap->pData, _MI_RGN_IMPL_CalBufSize(stCanvasInfo.ePixelFmt, &stCanvasInfo.stSize));
        /*Copy buffer from user*/
        s32ErrorCode = _MI_RGN_IMPL_BlitBuffer(&pstLd->stCanvasInfo, &stCanvasInfo, 0, 0);
        {
            DBG_EXIT_ERR("Blit buf error.\n");
            s32ErrorCode = MI_ERR_RGN_BADADDR;
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
        list_for_each_entry(pstChnPortListData, pstLd->handleToChPortList, handleToChPortList)
        {
            s32ErrorCode = _MI_RGN_IMPL_GetChnPortListHead(&pstChnPortListData->stAttachedChnPort, &pstChnPortListHead);
            if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
            {
                DBG_EXIT_ERR("chn port list not found.\n");
                s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
                up(&RGN_List_Sem);
                goto ERROR_CODE;
            }
            if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_DoUpdataCanvas(pstChnPortListHead, pstChnPortListData))
            {
                DBG_EXIT_ERR("Do update canvas error!\n");
                s32ErrorCode = MI_ERR_RGN_BADADDR;
                up(&RGN_List_Sem);
                goto ERROR_CODE;
            }
        }
        pstChnPortListData->bFreshToFront = TRUE;
    }

    up(&RGN_List_Sem);
    return MI_NOTICE_RGN_SUCCESS;

ERROR_CODE:

    return s32ErrorCode;
}
MI_S32 MI_RGN_IMPL_AttachToChn(MI_RGN_HANDLE hHandle, const MI_RGN_ChnPort_t* pstChnPort, const MI_RGN_ChnPortParam_t *pstChnAttr)
{
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_ChPortListData_t *pstChnPortListData = NULL;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;

    DBG_ENTER();
    /*1. Check handle avaliable. Check para.*/

    down(&RGN_List_Sem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    s32ErrorCode = _MI_RGN_IMPL_GetChnPortListHead(pstChnPort, &pstChnPortListHead);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("chn port list not found.\n");
        s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }

    if (_MI_RGN_IMPL_DoesHandleListAttached(pstLd, pstChnPortListData))
    {
        DBG_EXIT_ERR("CH/Port had been attached.\n");
        s32ErrorCode = MI_ERR_RGN_BUSY;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
     /*Create a chPortList data */
    if (pstLd->stRegAttr.eType == E_MI_RGN_OSD)
    {
        s32ErrorCode = _MI_RGN_IMPL_CheckOsdOverLap(pstChnPortListHead, &pstChnAttr->stPoint, &pstLd->stRegAttr.stOsdInitParam.stSize);        
        if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
        {
            DBG_EXIT_ERR("Osd is overlap.\n");
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
    }
    pstChnPortListData = MI_RGN_MEMALLOC(sizeof(MI_RGN_ChPortListData_t))
    MI_RGN_PTR_CHECK(pstChnPortListData, s32ErrorCode = MI_ERR_RGN_NOMEM;    \
                                    up(&RGN_List_Sem);  \
                                    goto ERROR_CODE;);
    memset(pstChnPortListData, 0, sizeof(MI_RGN_ChPortListData_t));
    /*Para init*/
    pstChnPortListData->pstAttachedListData = pstLd;
    memcpy(&pstChnPortListData->stAttachedChnPort, pstChnPort, sizeof(MI_RGN_ChnPort_t));
    memcpy(&pstChnPortListData->stAttachedChnPortPara, pstChnAttr, sizeof(MI_RGN_ChnPortParam_t));
    if (_MI_RGN_IMPL_AddChPortListDataByLayer(pstChnPortListHead, pstChnPortListData) != MI_NOTICE_RGN_SUCCESS)
    {
        DBG_EXIT_ERR("Do add ch port list error! Please check your layer config.\n");
        MI_RGN_MEMFREE(pstChnPortListData);
        up(&RGN_List_Sem);
        s32ErrorCode = MI_ERR_RGN_NOT_PERM;
        goto ERROR_CODE;
    }
    list_add_tail(&pstChnPortListData->handleToChPortList, &pstLd->handleToChPortList);
    if (pstLd->stRegAttr.eType == E_MI_RGN_OSD)
    {
        s32ErrorCode = _MI_RGN_IMPL_CreateDoubleBuffer(pstChnPortListHead, pstChnPortListData);
        if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
        {
            DBG_EXIT_ERR("Do create double buffer error!.\n");
            list_del(&pstChnPortListData->handleToChPortList);
            if (_MI_RGN_IMPL_DelChPortListData(pstChnPortListHead, pstChnPortListData) != MI_NOTICE_RGN_SUCCESS)
            {
                DBG_EXIT_ERR("Do del ch port list error!.\n");
                s32ErrorCode = MI_ERR_RGN_NOT_PERM;
            }
            MI_RGN_MEMFREE(pstChnPortListData);
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
    }
    up(&RGN_List_Sem);
    return MI_NOTICE_RGN_SUCCESS;

ERROR_CODE:
    return s32ErrorCode;
}
MI_S32 MI_RGN_IMPL_DetachFromChn(MI_RGN_HANDLE hHandle, const MI_RGN_ChnPort_t *pstChnPort)
{

    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_ChPortListData_t *pstChnPortListData = NULL;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;

    DBG_ENTER();
    /*1. Check handle avaliable. Check para.*/

    down(&RGN_List_Sem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    s32ErrorCode = _MI_RGN_IMPL_GetChnPortListHead(pstChnPort, &pstChnPortListHead);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("chn port list not found.\n");
        s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }

    if (!_MI_RGN_IMPL_DoesHandleListAttached(pstLd, pstChnPortListData))
    {
        DBG_EXIT_ERR("pstChnPort and handle not matched.\n");
        s32ErrorCode = MI_ERR_RGN_BUSY;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    if (pstLd->stRegAttr.eType == E_MI_RGN_OSD)
    {
        if (_MI_RGN_IMPL_DestroyDoubleBuffer(pstChnPortListHead, pstChnPortListData) != MI_NOTICE_RGN_SUCCESS)
        {
            DBG_EXIT_ERR("Do del front buffer error!.\n");
            s32ErrorCode = MI_ERR_RGN_NOT_PERM;
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
    }
    if (_MI_RGN_IMPL_DelChPortListData(pstChnPortListHead, pstChnPortListData) != MI_NOTICE_RGN_SUCCESS)
    {
        DBG_EXIT_ERR("Do del ch port list error!.\n");
        s32ErrorCode = MI_ERR_RGN_NOT_PERM;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    list_del(&pstChnPortListData->handleToChPortList);
    MI_RGN_MEMFREE(pstChnPortListData);
    up(&RGN_List_Sem);
    return MI_NOTICE_RGN_SUCCESS;

ERROR_CODE:
    return s32ErrorCode;

}
MI_S32 MI_RGN_IMPL_SetDisplayAttr(MI_RGN_HANDLE hHandle, const MI_RGN_ChnPort_t *pstChnPort, const MI_RGN_ChnPortParam_t *pstChnPortAttr)
{

    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_ChPortListData_t *pstChnPortListData = NULL;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;

    DBG_ENTER();
    /*1. Check handle avaliable. Check para.*/

    down(&RGN_List_Sem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    s32ErrorCode = _MI_RGN_IMPL_GetChnPortListHead(pstChnPort, &pstChnPortListHead);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("chn port list not found.\n");
        s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    if (!_MI_RGN_IMPL_DoesHandleListAttached(pstLd, pstChnPortListData))
    {
        DBG_EXIT_ERR("pstChnPort and handle not matched.\n");
        s32ErrorCode = MI_ERR_RGN_BUSY;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }

    memcpy(&pstChnPortListData.stAttachedChnPortPara, pstChnPortAttr, sizeof(MI_RGN_ChnPortParam_t));
    pstChnPortListData->bFreshToFront = TRUE;

    up(&RGN_List_Sem);
    return MI_NOTICE_RGN_SUCCESS;

ERROR_CODE:
    return s32ErrorCode;

}


MI_S32 MI_RGN_IMPL_GetDisplayAttr(MI_RGN_HANDLE hHandle, const MI_RGN_ChnPort_t *pstChnPort, MI_RGN_ChnPortParam_t *pstChnPortAttr)
{

    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_ChPortListData_t *pstChnPortListData = NULL;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;

    DBG_ENTER();
    /*1. Check handle avaliable. Check para.*/

    down(&RGN_List_Sem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    s32ErrorCode = _MI_RGN_IMPL_GetChnPortListHead(pstChnPort, &pstChnPortListHead);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("chn port list not found.\n");
        s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    memcpy(pstChnPortAttr, &pstChnPortListData->stAttachedChnPortPara, sizeof(MI_RGN_ChnPortParam_t));
    up(&RGN_List_Sem);
    return MI_NOTICE_RGN_SUCCESS;

ERROR_CODE:
    return s32ErrorCode;

}
MI_S32 MI_RGN_IMPL_GetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t* pstCanvasInfo)
{
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;
    MI_RGN_HandleListData_t *pstLd = NULL;

    DBG_ENTER();
    /*1. Check handle avaliable. Check para.*/
	MI_RGN_PTR_CHECK(pstCanvasInfo, return MI_ERR_RGN_NULL_PTR);

    down(&RGN_List_Sem);
    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    memcpy(pstCanvasInfo, &pstLd->stCanvasInfo);
    /*Tommy kernel to user by misys /Buffer overwrite*/
    copy_to_user(&pstCanvasInfo->VirtAddr, &pstLd->stCanvasInfo.VirtAddr, _MI_RGN_IMPL_CalBufSize(pstLd->stCanvasInfo.ePixelFmt, &pstLd->stCanvasInfo.stSize));

    up(&RGN_List_Sem);
    return MI_NOTICE_RGN_SUCCESS;

ERROR_CODE:
    return s32ErrorCode;

}
MI_S32 MI_RGN_IMPL_UpdateCanvas(MI_RGN_HANDLE hHandle)
{
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_ChPortListData_t *pstChnPortListData = NULL;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;

    DBG_ENTER();
    /*1. Check handle avaliable. Check para.*/

    down(&RGN_List_Sem);
    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&RGN_List_Sem);
        goto ERROR_CODE;
    }
    list_for_each_entry(pstChnPortListData, pstLd->handleToChPortList, handleToChPortList)
    {
        s32ErrorCode = _MI_RGN_IMPL_GetChnPortListHead(&pstChnPortListData->stAttachedChnPort, &pstChnPortListHead);
        if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
        {
            DBG_EXIT_ERR("chn port list not found.\n");
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
        if (MI_NOTICE_RGN_SUCCESS != _MI_RGN_IMPL_DoUpdataCanvas(pstChnPortListHead, pstChnPortListData))
        {
            DBG_EXIT_ERR("Do update canvas error!\n");
            s32ErrorCode = MI_ERR_RGN_BADADDR;
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
    }
    pstChnPortListData->bFreshToFront = TRUE;

    up(&RGN_List_Sem);
    return MI_NOTICE_RGN_SUCCESS;

ERROR_CODE:
    return s32ErrorCode;

}
MI_S32 mi_rgn_NotifyFenceDone(MI_U64 u64SWFence)
{
    return MI_NOTICE_RGN_SUCCESS;
}

MI_S32 mi_rgn_DivpProcess(mi_rgn_DivpCmdInfo_t CmdInfo, cmd_mload_interface *cmdinf, MI_U64 *pu64Fence)
{
    MI_U8 i = 0;
    MI_RGN_Size_t stDispWindowSize;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;
    MI_RGN_ChnPort_t stChnPort;
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;

    DBG_ENTER();

    down(&RGN_List_Sem);
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));

    for (i = E_MI_RGN_DIVP_PORT0; i < MI_RGN_DIVP_PORT_MAXNUM; i++)
    {
        stDispWindowSize.u32Height = CmdInfo.stDivpPort[i].u32Height;
        stDispWindowSize.u32Width = CmdInfo.stDivpPort[i].u32Width;
        stChnPort.s32OutputPortId = i;
        stChnPort.s32ChnId = CmdInfo.u32chnID;
        stChnPort.eModId = E_MI_RGN_DIVP;
        s32ErrorCode = _MI_RGN_IMPL_GetChnPortListHead(&stChnPort, &pstChnPortListHead);
        if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
        {
            DBG_EXIT_ERR("chn port list not found.\n");
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
        _MI_RGN_IMPL_UpdateChPortListData(pstChnPortListHead, &stChnPort,  &stDispWindowSize, CmdInfo.stDivpPort[i].bEnable, cmdinf);
    }
    up(&RGN_List_Sem);
ERROR_CODE:
    return s32ErrorCode;
}

MI_S32 mi_rgn_VpeProcess(mi_rgn_VpeCmdInfo_t CmdInfo, cmd_mload_interface *cmdinf, MI_U64 *pu64Fence)
{
    MI_U8 i = 0;
    MI_RGN_Size_t stDispWindowSize;
    MI_RGN_Portdata_t *pstChnPortListHead = NULL;
    MI_RGN_ChnPort_t stChnPort;
    MI_S32 s32ErrorCode = MI_NOTICE_RGN_SUCCESS;

    DBG_ENTER();

    down(&RGN_List_Sem);
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));

    for (i = E_MI_RGN_DIVP_PORT0; i < MI_RGN_DIVP_PORT_MAXNUM; i++)
    {
        stDispWindowSize.u32Height = CmdInfo.stVpePort[i].u32Height;
        stDispWindowSize.u32Width = CmdInfo.stVpePort[i].u32Width;
        stChnPort.s32OutputPortId = i;
        stChnPort.s32ChnId = CmdInfo.u32chnID;
        stChnPort.eModId = E_MI_RGN_VPE;
        s32ErrorCode = _MI_RGN_IMPL_GetChnPortListHead(&stChnPort, &pstChnPortListHead);
        if (MI_NOTICE_RGN_SUCCESS != s32ErrorCode)
        {
            DBG_EXIT_ERR("chn port list not found.\n");
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            up(&RGN_List_Sem);
            goto ERROR_CODE;
        }
        _MI_RGN_IMPL_UpdateChPortListData(pstChnPortListHead, &stChnPort,  &stDispWindowSize, CmdInfo.stVpePort[i].u32OverlayMask, CmdInfo.stVpePort[i].bEnable, cmdinf);
    }
    up(&RGN_List_Sem);

ERROR_CODE:
    return s32ErrorCode;
}

