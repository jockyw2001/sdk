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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   mi_vdec_impl.c
/// @brief vdec module impl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
///#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>

#include "mi_print.h"
#include "mi_vdec_impl.h"
#include "mi_vdec_internal.h"
#include "mi_sys_internal.h"

#include "vdec_dummy.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MI_VDEC_MAX_CHN_NUM (33)
#define MI_VDEC_CPU_MEM_SIZE (3*1024*1024)
#define MI_VDEC_ES_MEM_SIZE (1*1024*1024)
#define MI_VDEC_FRAME_MEM_SIZE (18*1024*1024)
#define MI_VDEC_MEMALLOC(x) kmalloc(x, GFP_ATOMIC)
#define MI_VDEC_MEMFREE(x) kfree(x)

///TODO:ENABLE
#define DOWN(x) if(0){down(x);}
#define UP(x) if(0){up(x);}

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------
#define MI_VDEC_ChkChnCreate(VdecChn, eRet) \
    do\
    {\
        if (FALSE == _stResMgr.astChnInfo[VdecChn].bCreate)\
        {\
            return eRet;\
        }\
    }while(0)

#define MI_VDEC_ChkChnStart(VdecChn, eRet) \
    do\
    {\
        if (FALSE == _stResMgr.astChnInfo[VdecChn].bStart)\
        {\
            if (MI_SUCCESS == eRet)\
            {\
                DBG_WRN("Chn(%d) Not Start\n", VdecChn);\
            }\
            else\
            {\
                DBG_EXIT_ERR("Chn(%d) Not Start\n", VdecChn);\
            }\
            return eRet;\
        }\
    }while(0)

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

typedef struct MI_VDEC_RegDevPriData_s
{
    MI_U32 u32Version;
} MI_VDEC_RegDevPriData_t;

typedef struct MI_VDEC_MemBufInfo_s
{
    MI_PHY phyAddr;
    void *pVirAddr;
    MI_U32 u32BufSize;
} MI_VDEC_MemBufInfo_t;

typedef struct
{
    MI_VDEC_MemBufInfo_t stCpuMemInfo;
    MI_VDEC_MemBufInfo_t stEsMemInfo;
    MI_VDEC_MemBufInfo_t stFrameMemInfo;
    MI_BOOL bCreate;
    MI_BOOL bStart;

    MI_VDEC_ChnAttr_t stChnAttr;
    MI_VDEC_ChnParam_t stChnParam;
    MI_VDEC_DisplayMode_e eDisplayMode;
    VDEC_StreamId stVDECStreamId;
    MI_U8 u8EnableMfcodec;
} MI_VDEC_ChnInfo_t;

typedef struct mi_vdec_BufInfo_s
{
    MI_U8 *pu8Addr;
    MI_PHY phyAddr;
    MI_U32 u32BufSize;
    MI_U64 u64Pts;
    MI_BOOL bEndOfStream;

    MI_BOOL bPictureStart;
    MI_BOOL bBrokenByUs;
} mi_vdec_BufInfo_t;

typedef struct MI_VDEC_ResMgr_s
{
    MI_BOOL bInitFlag;
    MI_BOOL bInjTaskRun;
    MI_BOOL bPutFrmTaskRun;
    MI_VDEC_ChnInfo_t astChnInfo[MI_VDEC_MAX_CHN_NUM];
    struct task_struct *ptskInjTask;
    struct task_struct *ptskPutFrmTask;
    wait_queue_head_t stInjWaitQueueHead;
    wait_queue_head_t stPutWaitQueueHead;
    MI_SYS_DRV_HANDLE hVdecDev;

    struct semaphore semChnLock[MI_VDEC_MAX_CHN_NUM];
} MI_VDEC_ResMgr_t;
//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------
static MI_VDEC_ResMgr_t _stResMgr;


//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
static MI_S32 _MI_VDEC_IMPL_GetFrmAlignOffset(MI_U8 *pBuf, MI_U32 u32BufSize, MI_VDEC_CodecType_e eCodecType, MI_U32 *pu32OffLastFrm)
{
    MI_S32 s32FrmAlign =  E_MI_ERR_FAILED;
    *pu32OffLastFrm = u32BufSize;

    if (eCodecType == E_MI_VDEC_CODEC_TYPE_H264)
    {
        s32FrmAlign = MI_SUCCESS;
    }
    else if (eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        s32FrmAlign = MI_SUCCESS;
    }
    else
    {
        s32FrmAlign = MI_SUCCESS;
    }

    return s32FrmAlign;
}

static MI_S32 _MI_VDEC_IMPL_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_IMPL_OnUnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_IMPL_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_IMPL_OnUnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_IMPL_QueryFreeBuffer(MI_VDEC_CHN VdecChn, mi_vdec_BufInfo_t *pstBufInfo)
{
    MI_U32 u32AvailSize = 0;
    MI_PHY phyAddr = 0;
    MI_U32 u32Vacany = 0;

    u32Vacany = MApi_VDEC_EX_GetESBuffVacancy(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), NULL);
    if (u32Vacany < pstBufInfo->u32BufSize)
    {
        return E_MI_ERR_FAILED;
    }

    if (E_VDEC_EX_OK == MApi_VDEC_EX_GetESBuff(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), (MS_U32)pstBufInfo->u32BufSize, (MS_U32 *)&u32AvailSize, (MS_PHY *)&phyAddr))
    {
        ///TODO:map phy address to virtual address
        pstBufInfo->pu8Addr = (MI_U8 *)phyAddr;
        pstBufInfo->phyAddr = phyAddr;
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_IMPL_WriteBufferComplete(MI_VDEC_CHN VdecChn, mi_vdec_BufInfo_t *pstBufInfo)
{
    VDEC_EX_DecCmd stDecCmd;
    memset(&stDecCmd, 0, sizeof(VDEC_EX_DecCmd));
    stDecCmd.u32StAddr = pstBufInfo->phyAddr;
    stDecCmd.u32Size = pstBufInfo->u32BufSize;
    stDecCmd.u32ID_H = ((pstBufInfo->u64Pts >> 32)& 0xFFFFFFFF);
    stDecCmd.u32ID_L = (pstBufInfo->u64Pts & 0xFFFFFFFF);
    stDecCmd.u32Timestamp = (pstBufInfo->u64Pts & 0xFFFFFFFF);

    MApi_VDEC_EX_PushDecQ(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), (VDEC_EX_DecCmd*)(void*)&stDecCmd);
    MApi_VDEC_EX_FireDecCmd(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
    return MI_SUCCESS;
}

///get frame from driver
static MI_S32 _MI_VDEC_IMPL_GetFrame(MI_VDEC_CHN VdecChn, mi_vdec_DispFrame_t *pstGetDispFrm)
{
    VDEC_EX_DispFrame* pstNextDispFrm;
    MI_BOOL b10Bit;
    VDEC_EX_FrameInfoExt_v6 stFrmInfoExt_v6;
    VDEC_EX_DispInfo stDispInfo;

    if(MApi_VDEC_EX_GetNextDispFrame(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), &pstNextDispFrm) == E_VDEC_EX_OK)
    {
        pstGetDispFrm->stFrmInfo.u32LumaAddr   = pstNextDispFrm->stFrmInfo.u32LumaAddr;
        pstGetDispFrm->stFrmInfo.u32ChromaAddr = pstNextDispFrm->stFrmInfo.u32ChromaAddr;
        pstGetDispFrm->stFrmInfo.u32TimeStamp  = pstNextDispFrm->stFrmInfo.u32TimeStamp;
        pstGetDispFrm->stFrmInfo.u32IdL        = pstNextDispFrm->stFrmInfo.u32ID_L;
        pstGetDispFrm->stFrmInfo.u32IdH        = pstNextDispFrm->stFrmInfo.u32ID_H;
        pstGetDispFrm->stFrmInfo.u16Pitch      = pstNextDispFrm->stFrmInfo.u16Pitch;
        pstGetDispFrm->stFrmInfo.u16Width      = pstNextDispFrm->stFrmInfo.u16Width;
        pstGetDispFrm->stFrmInfo.u16Height     = pstNextDispFrm->stFrmInfo.u16Height;
        pstGetDispFrm->stFrmInfo.eFrameType    = (mi_vdec_FrameType_e)(pstNextDispFrm->stFrmInfo.eFrameType);
        pstGetDispFrm->stFrmInfo.eFieldType    = (MI_SYS_FieldType_e)(pstNextDispFrm->stFrmInfo.eFieldType);

        pstGetDispFrm->u32PriData              = pstNextDispFrm->u32PriData;
        pstGetDispFrm->u32Idx                  = pstNextDispFrm->u32Idx;
        pstGetDispFrm->stFrmInfoExt.eFrameScanMode  = (MI_SYS_FrameScanMode_e)((pstNextDispFrm->stFrmInfo.u32ID_L >> 19) & 0x03);
        b10Bit = (pstNextDispFrm->stFrmInfo.u32ID_L >> 21) & 0x01;
    }
    else
    {
        return E_MI_ERR_FAILED;
    }

    if(b10Bit == TRUE)
    {
        VDEC_EX_FrameInfoEX stFrmInfo;
        memset(&stFrmInfo, 0, sizeof(stFrmInfo));
        if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_GET_FRAME_INFO_EX, (MS_U32*)&stFrmInfo))
        {
            pstGetDispFrm->stFrmInfoExt.phyLumaAddr2bit = stFrmInfo.u32LumaAddr_2bit;
            pstGetDispFrm->stFrmInfoExt.phyChromaAddr2bit = stFrmInfo.u32ChromaAddr_2bit;
            pstGetDispFrm->stFrmInfoExt.u8LumaBitDepth = stFrmInfo.u8LumaBitdepth;
            pstGetDispFrm->stFrmInfoExt.u8ChromaBitDepth = stFrmInfo.u8ChromaBitdepth;
            pstGetDispFrm->stFrmInfoExt.u16Pitch2bit = stFrmInfo.u16Pitch_2bit;
        }
        else
        {
            return E_MI_ERR_FAILED;
        }
    }

    // check support MFDEC or not
    if(sizeof(VDEC_EX_FrameInfoExt_v6) <= 1)
    {
        return MI_SUCCESS;
    }

    memset(&stFrmInfoExt_v6, 0, sizeof(stFrmInfoExt_v6));
    stFrmInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.stVerCtl.u32version = 6;
    stFrmInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.stVerCtl.u32size = sizeof(VDEC_EX_FrameInfoExt_v6);
    if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT, (MS_U32*)&stFrmInfoExt_v6))
    {
        MS_U32 u32MFCodecInfo = stFrmInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32MFCodecInfo;
        mi_vdec_MfCodecVersion_e eMfDecVersion = (mi_vdec_MfCodecVersion_e)(u32MFCodecInfo & 0xff);
        if((eMfDecVersion != E_MI_VDEC_MFCODEC_UNSUPPORT) && (eMfDecVersion != E_MI_VDEC_MFCODEC_DISABLE))
        {
            pstGetDispFrm->stDbInfo.bDbEnable = TRUE;
            pstGetDispFrm->stDbInfo.bBypassCodecMode = FALSE;
            pstGetDispFrm->stDbInfo.bUncompressMode = (u32MFCodecInfo >> 28) & 0x1;
            pstGetDispFrm->stDbInfo.u8DbSelect = (MI_U8)((u32MFCodecInfo >> 8) & 0x1);
            pstGetDispFrm->stDbInfo.eDbMode = (mi_vdec_DbMode_e)((u32MFCodecInfo >> 29) & 0x1);
            pstGetDispFrm->stDbInfo.u16HSize = stFrmInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.sFrameInfo.u16Width;
            pstGetDispFrm->stDbInfo.u16VSize = stFrmInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.sFrameInfo.u16Height;
            pstGetDispFrm->stDbInfo.phyDbBase = (MI_PHY)stFrmInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32LumaMFCbitlen;
            pstGetDispFrm->stDbInfo.u16DbPitch = (MI_U16)((u32MFCodecInfo >> 16) & 0xFF);
            pstGetDispFrm->stDbInfo.u8DbMiuSel = (MI_U8)((u32MFCodecInfo >> 24) & 0x0F);
            pstGetDispFrm->stDbInfo.phyLbAddr = (MI_PHY)stFrmInfoExt_v6.u32HTLBEntriesAddr;
            pstGetDispFrm->stDbInfo.u8LbSize = stFrmInfoExt_v6.u8HTLBEntriesSize;
            pstGetDispFrm->stDbInfo.u8LbTableId = stFrmInfoExt_v6.u8HTLBTableId;
        }
        else
        {
            pstGetDispFrm->stDbInfo.bDbEnable = FALSE;
            pstGetDispFrm->stDbInfo.bBypassCodecMode = TRUE;
        }
        pstGetDispFrm->stFrmInfoExt.eFrameTileMode = (MI_SYS_FrameTileMode_e)(stFrmInfoExt_v6.eTileMode);
        _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec = pstGetDispFrm->stDbInfo.bUncompressMode;
    }
    else
    {
        return E_MI_ERR_FAILED;
    }

    memset(&stDispInfo, 0, sizeof(stDispInfo));
    if(E_VDEC_EX_OK == MApi_VDEC_EX_GetDispInfo(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), &stDispInfo))
    {
        pstGetDispFrm->stDbInfo.u16StartX = stDispInfo.u16CropLeft;
        pstGetDispFrm->stDbInfo.u16StartY = stDispInfo.u16CropTop;
    }
    else
    {
        return E_MI_ERR_FAILED;
    }

    return MI_SUCCESS;
}

//release frame
static MI_S32 _MI_VDEC_IMPL_RlsFrame(MI_VDEC_CHN VdecChn, mi_vdec_DispFrame_t *pstRlsDispFrm)
{
    VDEC_EX_DispFrame stDispFrm;
    if(NULL == pstRlsDispFrm)
    {
        return E_MI_ERR_FAILED;
    }

    memset(&stDispFrm, 0x0, sizeof(VDEC_EX_DispFrame));
    stDispFrm.stFrmInfo.u32LumaAddr     = (MS_PHY)pstRlsDispFrm->stFrmInfo.u32LumaAddr;
    stDispFrm.stFrmInfo.u32ChromaAddr   = (MS_PHY)pstRlsDispFrm->stFrmInfo.u32ChromaAddr;
    stDispFrm.stFrmInfo.u32TimeStamp    = (MS_U32)pstRlsDispFrm->stFrmInfo.u32TimeStamp;
    stDispFrm.stFrmInfo.u32ID_L          = (MS_U32)pstRlsDispFrm->stFrmInfo.u32IdL;
    stDispFrm.stFrmInfo.u32ID_H          = (MS_U32)pstRlsDispFrm->stFrmInfo.u32IdH;
    stDispFrm.stFrmInfo.u16Pitch        = (MS_U16)pstRlsDispFrm->stFrmInfo.u16Pitch;
    stDispFrm.stFrmInfo.u16Width        = (MS_U16)pstRlsDispFrm->stFrmInfo.u16Width;
    stDispFrm.stFrmInfo.u16Height       = (MS_U16)pstRlsDispFrm->stFrmInfo.u16Height;
    stDispFrm.stFrmInfo.eFrameType      = (VDEC_EX_FrameType)pstRlsDispFrm->stFrmInfo.eFrameType;
    stDispFrm.stFrmInfo.eFieldType      = (VDEC_EX_FieldType)pstRlsDispFrm->stFrmInfo.eFieldType;
    stDispFrm.u32PriData                = (MS_U32)pstRlsDispFrm->u32PriData;
    stDispFrm.u32Idx                    = (MS_U32)pstRlsDispFrm->u32Idx;
    if(E_VDEC_EX_OK == MApi_VDEC_EX_ReleaseFrame(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), &stDispFrm))
    {
        return MI_SUCCESS;
    }

    return E_MI_ERR_FAILED;
}

static MI_S32 _MI_VDEC_IMPL_RleaseFrameCallBack(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_BufInfo_t *pstBufInfo)
{
    mi_vdec_DispFrame_t *pstDispFrm = pstBufInfo->stMetaData.pVirAddr;

    if (NULL == pstDispFrm)
    {
        while (1)
        {
            DBG_ERR("To Panic: NULL == pstDispFrm\n");
        }
    }

    ///TODO: Case:Release Old Channel Frame
    ///TODO:Case:UnBind Not CallBack Release Frame
    _MI_VDEC_IMPL_RlsFrame(pstChnCurryPort->s32ChnId, pstDispFrm);
    return MI_SUCCESS;
}

MI_S32 _MI_VDEC_IMPL_InjectBuffer(MI_VDEC_CHN VdecChn, MI_SYS_BufInfo_t *pstBufInfo)
{
    mi_vdec_BufInfo_t stBufInfo;

    ///TODO:Frame Aligned
    memset(&stBufInfo, 0x0, sizeof(mi_vdec_BufInfo_t));
    stBufInfo.u32BufSize = pstBufInfo->stRawData.u32ContenttSize;
    ///TODO:ENABLE CHECK
    if (0)///(stBufInfo.u32BufSize >= _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize)
    {
        DBG_ERR("Out Of Es Buffer Limited, Drop Es. Push Data Size:%d, Es Buffer Size:%d\n", stBufInfo.u32BufSize, _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize);
        return MI_SUCCESS;
    }

    if (MI_SUCCESS != _MI_VDEC_IMPL_QueryFreeBuffer(VdecChn, &stBufInfo))
    {
        return E_MI_ERR_FAILED;
    }

    stBufInfo.u32BufSize = pstBufInfo->stRawData.u32ContenttSize;
    memcpy(stBufInfo.pu8Addr, pstBufInfo->stRawData.pVirAddr, stBufInfo.u32BufSize);
    stBufInfo.u64Pts = pstBufInfo->u64Pts;
    stBufInfo.bEndOfStream = pstBufInfo->bEndOfStream;
    if (MI_SUCCESS != _MI_VDEC_IMPL_WriteBufferComplete(VdecChn, &stBufInfo))
    {
        return E_MI_ERR_FAILED;
    }
    return MI_SUCCESS;
}

static int _MI_VDEC_IMPL_InjectTask(void *pUsrData)
{
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    MI_U32 u32PreFrmSize = 0;
    MI_U8 *pu8PreFrmBuf = NULL;
    mi_vdec_BufInfo_t stBufInfo;

    DBG_INFO("Create Inject Task Done\n");
    memset(&stBufInfo, 0x0, sizeof(mi_vdec_BufInfo_t));
    while (_stResMgr.bInjTaskRun)
    {
        MI_U16 u16Chn = 0;
        MI_BOOL bPushEsBufFaild = TRUE;
        
        ///wait input port buffer
        mi_sys_WaitOnInputTaskAvailable(_stResMgr.hVdecDev, 100);
        for (u16Chn = 0; u16Chn < MI_VDEC_MAX_CHN_NUM; ++u16Chn)
        {
            if (FALSE == _stResMgr.astChnInfo[u16Chn].bStart)
            {
                ///current channel haven't start, but have buffer, need to remove
                continue;
            }

            pstBufInfo = mi_sys_GetInputPortBuf(_stResMgr.hVdecDev, u16Chn, 0);
            if (pstBufInfo == NULL)
            {
                ///current channel haven't es stream
                continue;
            }
            
            if (MI_SUCCESS == _MI_VDEC_IMPL_InjectBuffer(u16Chn, pstBufInfo))
            {
                mi_sys_FinishBuf(pstBufInfo);
                bPushEsBufFaild = FALSE;
            }
            else
            {
                mi_sys_RewindBuf(pstBufInfo);
            }
        }

        if (bPushEsBufFaild)
        {
            ///have input buffer, but can't push buffer to any channel
            interruptible_sleep_on_timeout(&_stResMgr.stInjWaitQueueHead, 10);
            ///wake_up_interruptible
        }
        else
        {
            wake_up_interruptible(&_stResMgr.stPutWaitQueueHead);
        }
    }

    pu8PreFrmBuf = NULL;
    return 0;
}

static int _MI_VDEC_IMPL_PutFrmTask(void *pUsrData)
{

    MI_SYS_BufConf_t stBufCfg;
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    mi_vdec_DispFrame_t *pstDispFrm = NULL;

    DBG_INFO("Create Put Frame Task Done\n");
    memset(&stBufCfg, 0x0, sizeof(MI_SYS_BufConf_t));
    stBufCfg.eBufType = E_MI_SYS_BUFDATA_META;
    stBufCfg.u64TargetPts = MI_SYS_INVALID_PTS;
    stBufCfg.stMetaCfg.eBufAllocMode = E_MI_SYS_BUFALLOC_MODE_KMALLOC;
    stBufCfg.stMetaCfg.u32Size = sizeof(mi_vdec_DispFrame_t);

    while (_stResMgr.bPutFrmTaskRun)
    {
        MI_U16 u16Chn = 0;
        MI_BOOL bPutFrm = FALSE;
        for (u16Chn = 0; u16Chn < MI_VDEC_MAX_CHN_NUM; ++u16Chn)
        {
            if (FALSE == _stResMgr.astChnInfo[u16Chn].bStart)
            {
                ///current channel haven't start, but have buffer, need to remove
                continue;
            }

            pstBufInfo = mi_sys_GetOutputPortBuf(_stResMgr.hVdecDev, u16Chn, 0, &stBufCfg);
            if (pstBufInfo == NULL)
            {
                ///no output buffer
                if (E_MI_VDEC_DISPLAY_MODE_PREVIEW == _stResMgr.astChnInfo[u16Chn].eDisplayMode)
                {
                    ///maybe blocked by others, preview need to drop frame
                    mi_vdec_DispFrame_t stDispFrm;
                    memset(&stDispFrm, 0x0, sizeof(mi_vdec_DispFrame_t));
                    if (MI_SUCCESS != _MI_VDEC_IMPL_GetFrame(u16Chn, &stDispFrm))
                    {
                        _MI_VDEC_IMPL_RlsFrame(u16Chn, &stDispFrm);
                        bPutFrm = TRUE;
                    }
                }
                continue;
            }

            if (pstBufInfo->stMetaData.u32Size < sizeof(mi_vdec_DispFrame_t))
            {
                ///ERROR
                mi_sys_RewindBuf(pstBufInfo);
                continue;
            }

            pstDispFrm = pstBufInfo->stMetaData.pVirAddr;
            if (MI_SUCCESS != _MI_VDEC_IMPL_GetFrame(u16Chn, pstDispFrm))
            {
                mi_sys_RewindBuf(pstBufInfo);
                continue;
            }

            pstBufInfo->stMetaData.eDataFromModule = E_MI_SYS_MODULE_ID_VDEC;
            pstBufInfo->stMetaData.u32Size = sizeof(mi_vdec_DispFrame_t);
            pstBufInfo->u64Pts = pstDispFrm->stFrmInfo.u32TimeStamp;

            mi_sys_FinishBuf(pstBufInfo);
            bPutFrm = TRUE;
        }

        if (bPutFrm)
        {
            wake_up_interruptible(&_stResMgr.stInjWaitQueueHead);
        }
        else
        {
            ///sleep
            interruptible_sleep_on_timeout(&_stResMgr.stPutWaitQueueHead, 10);
        }
    }
    return MI_SUCCESS;
}

static void _MI_VDEC_IMPL_InitVar(void)
{
    MI_U16 u16Chn = 0;
    memset(&_stResMgr, 0x0, sizeof(MI_VDEC_ResMgr_t));
    
    init_waitqueue_head(&_stResMgr.stInjWaitQueueHead);
    init_waitqueue_head(&_stResMgr.stPutWaitQueueHead);

    for (u16Chn = 0; u16Chn < MI_VDEC_MAX_CHN_NUM; ++u16Chn)
    {
        sema_init(&(_stResMgr.semChnLock[u16Chn]), 1);
    }
}

static MI_S32 _MI_VDEC_IMPL_StartChn(MI_VDEC_CHN VdecChn)
{
    VDEC_EX_DecModCfg stDecModCfg;
    VDEC_EX_INPUT_TSP eInputTSP = E_VDEC_EX_INPUT_TSP_NONE;
    VDEC_EX_DynmcDispPath stDynmcDispPath;
    VDEC_EX_DISPLAY_MODE eDispMode = E_VDEC_EX_DISPLAY_MODE_MCU;
    VDEC_EX_MFCodec_mode eMFCodecMode = E_VDEC_EX_MFCODEC_FORCE_DISABLE;
    VDEC_EX_InitParam stInitParams;

    if (E_VDEC_EX_OK != MApi_VDEC_EX_GetFreeStream(&_stResMgr.astChnInfo[VdecChn].stVDECStreamId,
        sizeof(VDEC_StreamId), E_VDEC_EX_N_STREAM, _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType))
    {
        DBG_EXIT_ERR("Get FreeStream Fail:%d\n", _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType);
        return E_MI_ERR_FAILED;
    }

    memset(&stDecModCfg, 0, sizeof(VDEC_EX_DecModCfg));
    stDecModCfg.eDecMod = E_VDEC_EX_DEC_MODE_DUAL_INDIE;
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SET_DECODE_MODE, (MS_U32)&stDecModCfg);
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_CONNECT_INPUT_TSP, eInputTSP);

    memset(&stDynmcDispPath, 0x0, sizeof(VDEC_EX_DynmcDispPath));
    stDynmcDispPath.bConnect  = TRUE;
    stDynmcDispPath.eMvopPath = E_VDEC_EX_DISPLAY_PATH_NONE;
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_CONNECT_DISPLAY_PATH, (MS_U32)&stDynmcDispPath);
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SET_DISPLAY_MODE, (MS_U32)eDispMode);
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_BITSTREAMBUFFER_MONOPOLY, TRUE);
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_FRAMEBUFFER_MONOPOLY, TRUE);
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_DYNAMIC_CMA_MODE, FALSE);
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_MFCODEC_MODE, (MS_U32)&eMFCodecMode);
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_DYNAMIC_CMA_MODE, FALSE);

    memset(&stInitParams, 0x0, sizeof(VDEC_EX_InitParam));
    if (E_MI_VDEC_CODEC_TYPE_H264 == _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType)
    {
        stInitParams.eCodecType = E_VDEC_EX_CODEC_TYPE_H264;
    }
    else if (E_MI_VDEC_CODEC_TYPE_H265 == _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType)
    {
        stInitParams.eCodecType = E_VDEC_EX_CODEC_TYPE_HEVC;
    }
    else if (E_MI_VDEC_CODEC_TYPE_JPEG == _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType)
    {
        stInitParams.eCodecType = E_VDEC_EX_CODEC_TYPE_MJPEG;
    }
    else
    {
        MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        DBG_EXIT_ERR("Vdec Chn(%d) UnSupport Codec(%d) Failed\n", VdecChn, _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType);
        return E_MI_ERR_FAILED;
    }

    stInitParams.VideoInfo.eSrcMode = E_VDEC_EX_SRC_MODE_FILE;
    stInitParams.VideoInfo.eTimeStampType = E_VDEC_EX_TIME_STAMP_PTS;

    stInitParams.EnableDynaScale = FALSE;
    stInitParams.bDisableDropErrFrame = TRUE;
    stInitParams.bDisableErrConceal = FALSE;
    stInitParams.bRepeatLastField = TRUE;

    ///cpu buffer
    stInitParams.SysConfig.u32CodeBufAddr = _stResMgr.astChnInfo[VdecChn].stCpuMemInfo.phyAddr;
    stInitParams.SysConfig.u32CodeBufSize = _stResMgr.astChnInfo[VdecChn].stCpuMemInfo.u32BufSize;
    stInitParams.SysConfig.u32FWBinaryAddr = stInitParams.SysConfig.u32CodeBufAddr;
    stInitParams.SysConfig.u32FWBinarySize = stInitParams.SysConfig.u32CodeBufSize;

    ///framebuffer
    stInitParams.SysConfig.u32FrameBufAddr = _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr;
    stInitParams.SysConfig.u32FrameBufSize = _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize;

    ///bitstream buffer
    stInitParams.SysConfig.u32BitstreamBufAddr = _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr;
    stInitParams.SysConfig.u32BitstreamBufSize = _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize;
    stInitParams.SysConfig.u32DrvProcBufAddr = stInitParams.SysConfig.u32BitstreamBufAddr;
    stInitParams.SysConfig.u32DrvProcBufSize = stInitParams.SysConfig.u32BitstreamBufSize;

    stInitParams.SysConfig.eDbgMsgLevel = E_VDEC_EX_DBG_LEVEL_NONE;
    if (E_VDEC_EX_OK != MApi_VDEC_EX_Init(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), &stInitParams))
    {
        MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        DBG_EXIT_ERR("Vdec Chn(%d) Init Failed\n", VdecChn);
        return E_MI_ERR_FAILED;
    }

    MApi_VDEC_EX_SetBlockDisplay(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), TRUE);
    MApi_VDEC_EX_EnableESBuffMalloc(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), TRUE);
    MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SET_DISP_OUTSIDE_CTRL_MODE, TRUE);
    MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SET_DISP_FINISH_MODE, TRUE);
    MApi_VDEC_EX_DisableDeblocking(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), TRUE);
    MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_AVC_SUPPORT_REF_NUM_OVER_MAX_DPB_SIZE, TRUE);
    ///set decoder order output
    MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER, TRUE);

    MApi_VDEC_EX_AVSyncOn(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), FALSE, 180, 40);
    if (E_VDEC_EX_OK != MApi_VDEC_EX_Play(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId)))
    {
        MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        DBG_EXIT_ERR("Vdec Chn(%d) Play Failed\n", VdecChn);
        return E_MI_ERR_FAILED;
    }

    _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec = (MI_U8)(-1);
    return MI_SUCCESS;
}

static MI_VDEC_ErrCode_e _MI_VDEC_TransDrvErrCode2MiType(MI_U32 u32ErrCode)
{
    MI_VDEC_ErrCode_e eErrCode = E_MI_VDEC_ERR_CODE_MAX;
    switch (u32ErrCode)
    {
        case 1:
            eErrCode = E_MI_VDEC_ERR_CODE_MAX;
            break;
        default:
            eErrCode = E_MI_VDEC_ERR_CODE_MAX;
    }

    return eErrCode;
}

MI_S32 MI_VDEC_IMPL_Init(void)
{
    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevBindOps;
    MI_VDEC_RegDevPriData_t stRegDevPriData;

    DBG_ENTER();
    if (_stResMgr.bInitFlag)
    {
        DBG_WRN("Already Init\n");
        return MI_SUCCESS;
    }

    _MI_VDEC_IMPL_InitVar();
    memset(&stModDevInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    stModDevInfo.eModuleId = E_MI_SYS_MODULE_ID_VDEC;
    stModDevInfo.u32DevId = 0x0;
    stModDevInfo.u32DevChnNum = 33;
    stModDevInfo.u32InputPortNum = 1;
    stModDevInfo.u32OutputPortNum = 0;

    memset(&stModDevBindOps, 0x0, sizeof(mi_sys_ModuleDevBindOps_t));
    stModDevBindOps.OnBindInputPort = _MI_VDEC_IMPL_OnBindInputPort;
    stModDevBindOps.OnBindOutputPort = _MI_VDEC_IMPL_OnBindOutputPort;
    stModDevBindOps.OnUnBindInputPort = _MI_VDEC_IMPL_OnUnBindInputPort;
    stModDevBindOps.OnUnBindOutputPort = _MI_VDEC_IMPL_OnUnBindOutputPort;
    stModDevBindOps.OnOutputPortBufRelease = _MI_VDEC_IMPL_RleaseFrameCallBack;

    stRegDevPriData.u32Version = 1;
    _stResMgr.hVdecDev = mi_sys_RegisterDev(&stModDevInfo, &stModDevBindOps , &stRegDevPriData);

    _stResMgr.ptskInjTask = kthread_create(_MI_VDEC_IMPL_InjectTask, &_stResMgr, "mi_vdec_InjTask");
    _stResMgr.ptskPutFrmTask = kthread_create(_MI_VDEC_IMPL_PutFrmTask, &_stResMgr, "_MI_VDEC_IMPL_PutFrmTask");

    _stResMgr.bInjTaskRun = TRUE;
    wake_up_process(_stResMgr.ptskInjTask);

    _stResMgr.bPutFrmTaskRun = TRUE;
    wake_up_process(_stResMgr.ptskPutFrmTask);
   _stResMgr.bInitFlag = TRUE;

   DBG_EXIT_OK();
   return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_DeInit(void)
{
    DBG_INFO("Enter Vdec DeInit\n");
    if (FALSE == _stResMgr.bInitFlag)
    {
        DBG_ERR("Already Exit\n");
        return E_MI_ERR_FAILED;
    }

    if (_stResMgr.ptskInjTask)
    {
        _stResMgr.bInjTaskRun = FALSE;
        kthread_stop(_stResMgr.ptskInjTask);
        _stResMgr.ptskInjTask = NULL;
    }

    if (_stResMgr.ptskPutFrmTask)
    {
        _stResMgr.bPutFrmTaskRun = FALSE;
        kthread_stop(_stResMgr.ptskPutFrmTask);
        _stResMgr.ptskPutFrmTask = NULL;
    }

    mi_sys_UnRegisterDev(_stResMgr.hVdecDev);
    _stResMgr.bInitFlag = FALSE;
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_CreateChn(MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstChnAttr)
{
    VDEC_EX_CodecType eVdecCodecType = E_VDEC_EX_CODEC_TYPE_NONE;

    if (NULL == pstChnAttr)
    {
        DBG_EXIT_ERR("pstChnAttr is Null\n");
        return E_MI_ERR_FAILED;
    }

    DBG_INFO("VdecChn:%d, eCodecType:%d, eVideoMode:%d, u32RefFrameNum:%d, u32BufSize:%d, u32PicWidth:%d, u32PicHeight:%d, u32Priority:%d\n",
        VdecChn,
        pstChnAttr->eCodecType,
        pstChnAttr->eVideoMode,
        pstChnAttr->stVdecVideoAttr.u32RefFrameNum,
        pstChnAttr->u32BufSize,
        pstChnAttr->u32PicWidth,
        pstChnAttr->u32PicHeight,
        pstChnAttr->u32Priority);

    DOWN(&(_stResMgr.semChnLock[VdecChn]));
    if (_stResMgr.astChnInfo[VdecChn].bCreate)
    {
        DBG_WRN("Chn(%d) Already Create\n", VdecChn);
        UP(&(_stResMgr.semChnLock[VdecChn]));
        return MI_SUCCESS;
    }

    if (pstChnAttr->eCodecType == E_MI_VDEC_CODEC_TYPE_H264)
    {
        eVdecCodecType = E_VDEC_EX_CODEC_TYPE_H264;
    }
    else if (pstChnAttr->eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        eVdecCodecType = E_VDEC_EX_CODEC_TYPE_HEVC;
    }
    else
    {
        DBG_EXIT_ERR("UnSupport Codec Type:%d\n", pstChnAttr->eCodecType);
        UP(&(_stResMgr.semChnLock[VdecChn]));
        return E_MI_ERR_FAILED;
    }

    ///alloc cpu buffer
    _stResMgr.astChnInfo[VdecChn].stCpuMemInfo.u32BufSize = MI_VDEC_CPU_MEM_SIZE;
    if (MI_SUCCESS != mi_sys_MMA_Alloc(NULL,
        _stResMgr.astChnInfo[VdecChn].stCpuMemInfo.u32BufSize,
        &_stResMgr.astChnInfo[VdecChn].stCpuMemInfo.phyAddr))
    {
        goto _Exit;
    }
    ///alloc Es buffer
    if ((pstChnAttr->u32PicWidth >= 1920) || (pstChnAttr->u32PicHeight >= 1080))
    {
        if (MI_VDEC_ES_MEM_SIZE > pstChnAttr->u32BufSize)
        {
            pstChnAttr->u32BufSize = MI_VDEC_ES_MEM_SIZE;
        }
    }
    else
    {
        if ((MI_VDEC_ES_MEM_SIZE >> 1) > pstChnAttr->u32BufSize)
        {
            pstChnAttr->u32BufSize = (MI_VDEC_ES_MEM_SIZE >> 1);
        }
    }
    _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize = pstChnAttr->u32BufSize;
    if (MI_SUCCESS != mi_sys_MMA_Alloc(NULL,
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize,
        &_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr))
    {
        goto _Exit;
    }
    ///alloc frame buffer
    if (6 > pstChnAttr->stVdecVideoAttr.u32RefFrameNum)
    {
        pstChnAttr->stVdecVideoAttr.u32RefFrameNum = 6;
    }
    _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize = (pstChnAttr->u32PicWidth * pstChnAttr->u32PicHeight * 3 / 2) * pstChnAttr->stVdecVideoAttr.u32RefFrameNum;
    if (MI_SUCCESS != mi_sys_MMA_Alloc(NULL,
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize,
        &_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr)
        )
    {
        goto _Exit;
    }

    ///save channel attribite
    memcpy(&_stResMgr.astChnInfo[VdecChn].stChnAttr, pstChnAttr, sizeof(MI_VDEC_ChnAttr_t));
    _stResMgr.astChnInfo[VdecChn].bCreate = TRUE;

    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_EXIT_OK();
    return MI_SUCCESS;
_Exit:
    if (_stResMgr.astChnInfo[VdecChn].stCpuMemInfo.phyAddr)
    {
        mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stCpuMemInfo.phyAddr);
    }

    if (_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr)
    {
        mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr);
    }

    if (_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr)
    {
        mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr);
    }

    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_ERR("Create Chn Error\n");
    return E_MI_ERR_FAILED;
}

MI_S32 MI_VDEC_IMPL_DestroyChn(MI_VDEC_CHN VdecChn)
{
    DBG_INFO("Destroy Chn(%d)\n", VdecChn);
    MI_VDEC_ChkChnCreate(VdecChn, MI_SUCCESS);

    DOWN(&(_stResMgr.semChnLock[VdecChn]));
    if (_stResMgr.astChnInfo[VdecChn].bStart)
    {
        MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        _stResMgr.astChnInfo[VdecChn].bStart = FALSE;
    }

    mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stCpuMemInfo.phyAddr);
    mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr);
    mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr);
    memset(&_stResMgr.astChnInfo[VdecChn], 0x0, sizeof(MI_VDEC_ChnInfo_t));
    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_GetChnAttr(MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstChnAttr)
{
    DBG_INFO("Get Chn(%d) Attr:%p\n", VdecChn, pstChnAttr);
    MI_VDEC_ChkChnCreate(VdecChn, E_MI_ERR_FAILED);
    memcpy(pstChnAttr, &_stResMgr.astChnInfo[VdecChn].stChnAttr, sizeof(MI_VDEC_ChnAttr_t));
    DBG_INFO("(%d) eCodecType:%d, eVideoMode:%d, u32RefFrameNum:%d, u32BufSize:%d, u32PicWidth:%d, u32PicHeight:%d, u32Priority:%d\n",
        VdecChn,
        pstChnAttr->eCodecType,
        pstChnAttr->eVideoMode,
        pstChnAttr->stVdecVideoAttr.u32RefFrameNum,
        pstChnAttr->u32BufSize,
        pstChnAttr->u32PicWidth,
        pstChnAttr->u32PicHeight,
        pstChnAttr->u32Priority);
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_StartChn(MI_VDEC_CHN VdecChn)
{
    DBG_INFO("Start Chn(%d)\n", VdecChn);
    MI_VDEC_ChkChnCreate(VdecChn, E_MI_ERR_FAILED);
    DOWN(&(_stResMgr.semChnLock[VdecChn]));
    if (_stResMgr.astChnInfo[VdecChn].bStart)
    {
        ///already start
        UP(&(_stResMgr.semChnLock[VdecChn]));
        DBG_WRN("Chn(%d) Already Start\n", VdecChn);
        return MI_SUCCESS;
    }

    _stResMgr.astChnInfo[VdecChn].bStart = (MI_SUCCESS == _MI_VDEC_IMPL_StartChn(VdecChn));
    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_StopChn(MI_VDEC_CHN VdecChn)
{
    DBG_INFO("Stop Chn(%d)\n", VdecChn);
    MI_VDEC_ChkChnStart(VdecChn, MI_SUCCESS);
    DOWN(&(_stResMgr.semChnLock[VdecChn]));
    MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
    _stResMgr.astChnInfo[VdecChn].bStart = FALSE;
    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_GetChnStat(MI_VDEC_CHN VdecChn, MI_VDEC_ChnStat_t *pstChnStat)
{
    MI_U32 u32DecodeStreamFrames = 0;
    MI_U32 u32ErrCode = 0;

    DBG_INFO("Get Chn(%d) Stat:%p\n", VdecChn, pstChnStat);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_FAILED);

    ///software calculater channel info
    ///mi_vdec_SwCalcChnInfo(pstChnStat);

    ///get channel info from driver, frame count and error code
    u32DecodeStreamFrames = MApi_VDEC_EX_GetFrameCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId))
                                 + MApi_VDEC_EX_GetErrCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId))
                                 + MApi_VDEC_EX_GetDropCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId))
                                 + MApi_VDEC_EX_GetSkipCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));

    u32ErrCode = MApi_VDEC_EX_GetErrCode(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
    pstChnStat->eErrCode = _MI_VDEC_TransDrvErrCode2MiType(u32ErrCode);
    DBG_INFO("(Exit Ok)Get Chn(%d) Stat, bChnStart:%d, eCodecType:%d, eErrCode:%d, u32DecodeStreamFrames:%d, u32LeftPics:%d, "
            "u32LeftStreamBytes:%d, u32LeftStreamFrames:%d, u32RecvStreamFrames:%d\n",
        VdecChn,
        pstChnStat->bChnStart,
        pstChnStat->eCodecType,
        pstChnStat->eErrCode,
        pstChnStat->u32DecodeStreamFrames,
        pstChnStat->u32LeftPics,
        pstChnStat->u32LeftStreamBytes,
        pstChnStat->u32LeftStreamFrames,
        pstChnStat->u32RecvStreamFrames);
    return MI_SUCCESS;
}

static void _MI_VDEC_IMPL_ClearInjectBuf(MI_VDEC_CHN VdecChn)
{
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    _stResMgr.astChnInfo[VdecChn].bStart = FALSE;
    ///TODO:ENBALE
    while (0)//(TRUE)
    {
        pstBufInfo = mi_sys_GetInputPortBuf(_stResMgr.hVdecDev, VdecChn, 0);
        if (pstBufInfo == NULL)
        {
            break;
        }
        mi_sys_FinishBuf(pstBufInfo);
    }
}

MI_S32 MI_VDEC_IMPL_ResetChn(MI_VDEC_CHN VdecChn)
{
    DBG_INFO("Reset Chn(%d)\n", VdecChn);
    MI_VDEC_ChkChnStart(VdecChn, MI_SUCCESS);
    DOWN(&(_stResMgr.semChnLock[VdecChn]));
    ///stop channel
    MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
    _MI_VDEC_IMPL_ClearInjectBuf(VdecChn);
    ///start channel
    _stResMgr.astChnInfo[VdecChn].bStart = (MI_SUCCESS == _MI_VDEC_IMPL_StartChn(VdecChn));
    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_SetChnParam(MI_VDEC_CHN VdecChn, MI_VDEC_ChnParam_t *pstChnParam)
{
    DBG_INFO("Set Chn(%d) Param:%p, eDecMode:%d, eOutputOrder:%d, eVideoFormat:%d, u32DecFramebufferNum:%d\n",
        VdecChn, pstChnParam,
        pstChnParam->eDecMode,
        pstChnParam->eOutputOrder,
        pstChnParam->eVideoFormat,
        pstChnParam->u32DecFramebufferNum);

    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_FAILED);
    if (pstChnParam->eDecMode != _stResMgr.astChnInfo[VdecChn].stChnParam.eDecMode)
    {
        VDEC_EX_TrickDec eTrickDec = E_VDEC_EX_TRICK_DEC_ALL;
        if (pstChnParam->eDecMode == E_MI_VDEC_DECODE_MODE_ALL)
        {
            eTrickDec = E_VDEC_EX_TRICK_DEC_ALL;
        }
        else if (pstChnParam->eDecMode == E_MI_VDEC_DECODE_MODE_I)
        {
            eTrickDec = E_VDEC_EX_TRICK_DEC_I;
        }
        else if (pstChnParam->eDecMode == E_MI_VDEC_DECODE_MODE_IP)
        {
            eTrickDec = E_VDEC_EX_TRICK_DEC_IP;
        }
        
        MApi_VDEC_EX_SetTrickMode(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), eTrickDec);
        _stResMgr.astChnInfo[VdecChn].stChnParam.eDecMode = pstChnParam->eDecMode;
    }

    if (pstChnParam->u32DecFramebufferNum != _stResMgr.astChnInfo[VdecChn].stChnParam.u32DecFramebufferNum)
    {
        ///TODO:set frame buffer number
        _stResMgr.astChnInfo[VdecChn].stChnParam.u32DecFramebufferNum = pstChnParam->u32DecFramebufferNum;
    }

    if (pstChnParam->eOutputOrder != _stResMgr.astChnInfo[VdecChn].stChnParam.eOutputOrder)
    {
        MI_BOOL bDecOrder = TRUE;
        if (pstChnParam->eOutputOrder == E_MI_VDEC_OUTPUT_ORDER_DISPLAY)
        {
            bDecOrder = FALSE;
        }

        MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER, bDecOrder);
        _stResMgr.astChnInfo[VdecChn].stChnParam.eOutputOrder = pstChnParam->eOutputOrder;
    }

    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_GetChnParam(MI_VDEC_CHN VdecChn, MI_VDEC_ChnParam_t *pstChnParam)
{
    DBG_INFO("Get Chn(%d) Param:%p\n", VdecChn, pstChnParam);

    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_FAILED);
    memcpy(pstChnParam, &_stResMgr.astChnInfo[VdecChn].stChnParam, sizeof(MI_VDEC_ChnParam_t));
    ///get tile
    if (_stResMgr.astChnInfo[VdecChn].u8EnableMfcodec == TRUE)
    {
        pstChnParam->eVideoFormat = E_MI_VDEC_VIDEO_FORMAT_TILE;
    }
    else if (_stResMgr.astChnInfo[VdecChn].u8EnableMfcodec == FALSE)
    {
        ///use mfcodec
        pstChnParam->eVideoFormat = E_MI_VDEC_VIDEO_FORMAT_REDUCE;
    }
    else
    {
        pstChnParam->eVideoFormat = E_MI_VDEC_VIDEO_FORMAT_MAX;
    }

    
    DBG_INFO("(Exit Ok) eDecMode:%d, eOutputOrder:%d, eVideoFormat:%d, u32DecFramebufferNum:%d\n",
        pstChnParam->eDecMode,
        pstChnParam->eOutputOrder,
        pstChnParam->eVideoFormat,
        pstChnParam->u32DecFramebufferNum);
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_GetUserData(MI_VDEC_CHN VdecChn, MI_VDEC_UserData_t *pstUserData, MI_S32 s32MilliSec)
{
    VDEC_EX_Frame_packing_SEI_EX stSEIData;

    DBG_INFO("Get Chn(%d) User Data, %p, timeout:%d\n", VdecChn, pstUserData, s32MilliSec);
    if (NULL == pstUserData->pu8Addr)
    {
        DBG_EXIT_ERR("pstUserData Is Null\n");
        return E_MI_ERR_FAILED;
    }

    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_FAILED);
    MApi_VDEC_EX_GetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_GET_FPA_SEI_EX, (MS_U32 *)&stSEIData);

    pstUserData->phyAddr = 0;
    if (stSEIData.u32DataSize > pstUserData->u32Len)
    {
        DBG_ERR("Out Of Buffer, Data Size:%ld, User Buffer Size:%d\n", stSEIData.u32DataSize, pstUserData->u32Len);
        return E_MI_ERR_FAILED;
    }
    memcpy(pstUserData->pu8Addr, (void *)stSEIData.u32DataBuff, stSEIData.u32DataSize);
    pstUserData->u32Len = stSEIData.u32DataSize;
    pstUserData->bValid = stSEIData.bValid;

    DBG_INFO("(Exit Ok) bValid:%d, u32Len:%d, Data[0]:0x%x\n", pstUserData->bValid, pstUserData->u32Len, pstUserData->pu8Addr[0]);
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_ReleaseUserData(MI_VDEC_CHN VdecChn, MI_VDEC_UserData_t *pstUserData)
{
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_SetDisplayMode(MI_VDEC_CHN VdecChn, MI_VDEC_DisplayMode_e eDisplayMode)
{
    DBG_INFO("Set Chn(%d) DisplayMode:%d\n", VdecChn, eDisplayMode);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_FAILED);

    _stResMgr.astChnInfo[VdecChn].eDisplayMode = eDisplayMode;
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_GetDisplayMode(MI_VDEC_CHN VdecChn, MI_VDEC_DisplayMode_e *peDisplayMode)
{
    DBG_INFO("Get Chn(%d) peDisplayMode:%p\n", VdecChn, peDisplayMode);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_FAILED);

    if (NULL == peDisplayMode)
    {
        DBG_EXIT_ERR("peDisplayMode Is Null\n");
        return E_MI_ERR_FAILED;
    }
    *peDisplayMode = _stResMgr.astChnInfo[VdecChn].eDisplayMode;
    DBG_INFO("(Exit Ok)Get Chn(%d) DisplayMode:%d\n", VdecChn, *peDisplayMode);
    return MI_SUCCESS;
}
