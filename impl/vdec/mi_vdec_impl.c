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
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/math64.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/kernel.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
#include <mstar/mstar_chip.h>
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
#include <mstar_chip.h>
#else
#error not support this kernel version
#endif

#include "mi_print.h"
#include "mi_vdec_impl.h"
#include "mi_vdec_internal.h"
#include "mi_sys_internal.h"
#include "mi_common_internal.h"
#include "mi_syscfg.h"

#include "UFO.h"
#include "MsOS.h"
#include "MsTypes.h"
#include "apiVDEC_EX.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_sys_internal.h"
#include "mi_sys_proc_fs_internal.h"
#include "apiJPEG.h"
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
static atomic_t stInjWaitQueueHead_red_flag = ATOMIC_INIT(0);
static atomic_t stPutWaitQueueHead_red_flag = ATOMIC_INIT(0);

#if (defined CONFIG_MSTAR_CHIP_I2) && (CONFIG_MSTAR_CHIP_I2 == 1)
#define MI_VDEC_MAX_CHN_NUM (33)
#elif (defined CONFIG_MSTAR_CHIP_K6) && (CONFIG_MSTAR_CHIP_K6 == 1)
#define MI_VDEC_MAX_CHN_NUM (26)
#elif (defined CONFIG_MSTAR_CHIP_K6LITE) && (CONFIG_MSTAR_CHIP_K6LITE == 1)
#define MI_VDEC_MAX_CHN_NUM (17)
#else
#define MI_VDEC_MAX_CHN_NUM (33)
#endif

#define MI_VDEC_ES_MEM_SIZE (1*1024*1024)
#define MI_VMALLOC vmalloc
#define MI_VFREE vfree
#define DOWN(x) down(x)
#define UP(x) up(x)
#define FRAMING_BUF_MAX_SIZE (512 * 1024)
#define MI_VDEC_PAGE_SIZE (0x1000)
#define MI_VDEC_IMAGE_PITCH_ALIGN_SIZE (32)
#define MI_VDEC_SUPPORT_MAX_PROGRESSICE_JPEG_WIDTH 1920
#define MI_VDEC_SUPPORT_MAX_PROGRESSICE_JPEG_HEIGHT 1088
#define MI_VDEC_MAX_USED_MIU 3
static MI_SYS_BufInfo_t* pVdecStBufInfo[MI_VDEC_MAX_CHN_NUM] = {NULL};

#ifndef MIN
#define MIN(_a_, _b_)               ((_a_) < (_b_) ? (_a_) : (_b_))
#endif
#ifndef MAX
#define MAX(_a_, _b_)               ((_a_) > (_b_) ? (_a_) : (_b_))
#endif

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
#define PRINTF_PROC(fmt, args...)  {do{MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_RED"[MI VDEC PROCFS]:" fmt ASCII_COLOR_END, ##args);}while(0);}
#endif

#if (defined SUPPORT_MsOS_MPool_Add_PA2VARange) && (SUPPORT_MsOS_MPool_Add_PA2VARange == 0)
#define NOSUPPORT_MsOS_MPool_Add_PA2VARange   1
#else
#define NOSUPPORT_MsOS_MPool_Add_PA2VARange   0
#endif
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

#define MI_VDEC_ChkChnIdx(VdecChn, eRet) \
    do\
    {\
        if (VdecChn >= MI_VDEC_MAX_CHN_NUM)\
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

#define MI_VDEC_ChkDumpParamsVarNum(_x_)\
    do\
    {\
        if (v < _x_)\
        {\
            return -EINVAL;\
        }\
    } while (0)

typedef enum
{
  E_MI_CHIP_MIU_0 = 0,
  E_MI_CHIP_MIU_1,
  E_MI_CHIP_MIU_2,
  E_MI_CHIP_MIU_3,
  E_MI_CHIP_MIU_NUM,
} MI_CHIP_MIU_ID;


#define MI_VDEC_PHYADDR_TO_MIU_OFFSET(phy_addr) \
    (phy_addr >= ARM_MIU2_BASE_ADDR)?((MI_PHY)(phy_addr - ARM_MIU2_BASE_ADDR)):((phy_addr >= (MI_PHY)ARM_MIU1_BASE_ADDR)?((MI_PHY)(phy_addr - ARM_MIU1_BASE_ADDR)):((MI_PHY)(phy_addr - ARM_MIU0_BASE_ADDR)))

#define MI_VDEC_MIU_OFFSET_TO_PHYADDR(MiuSel, Offset, phy_addr) \
    (MiuSel == E_MI_CHIP_MIU_0)?((MI_PHY)(phy_addr = Offset)):((MiuSel == E_MI_CHIP_MIU_1)?(MI_PHY)(phy_addr = (Offset + ARM_MIU1_BASE_ADDR)):(MI_PHY)(phy_addr = (Offset + ARM_MIU2_BASE_ADDR)))

#define MI_VDEC_DoMsOsMPoolMapping(_x_)\
    do\
    {\
        MI_VDEC_MemBufInfo_t stMmapInfo;\
        _MI_VDEC_IMPL_GetMmapInfo(_x_, &stMmapInfo);\
        if(!MsOS_MPool_Mapping_Dynamic(stMmapInfo.u8Miu, MI_VDEC_PHYADDR_TO_MIU_OFFSET(stMmapInfo.phyAddr), stMmapInfo.u32BufSize, TRUE))\
        {\
            DBG_ERR("MsOS_MPool_Mapping_Dynamic "_x_" fail!\n");\
            if(MsOS_MPool_Mapping_Dynamic(stMmapInfo.u8Miu, MI_VDEC_PHYADDR_TO_MIU_OFFSET(stMmapInfo.phyAddr), stMmapInfo.u32BufSize, FALSE))\
            {\
                DBG_ERR("MsOS_MPool_Mapping_Dynamic "_x_" fail!\n");\
            }\
        }\
    } while (0)

#define MI_VDEC_UnSupportOperation(VdecChn)\
    do\
    {\
        if(_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_JPEG)\
        {\
            DBG_WRN("JPEG Chn(%d) UnSupport Current Operation, Return\n", VdecChn);\
            return E_MI_ERR_NOT_SUPPORT;\
        }\
    } while (0)

#define _ALIGN_BYTE_(x, y) ((x+y-1)&(~(y-1)))

#define MI_VDEC_JPEG_BITSTREAM_PROGRESSIVE_BUFFER(width, height, bufsize)\
    (width*height*2*3) +( width*height*2*3/64)+(64*1024+128)+(8*1024)+bufsize

#define MI_VDEC_JPEG_BITSTREAM_BASELINE_BUFFER(bufsize)\
    (64*1024+128)+(8*1024)+bufsize+0x3FC000

#define MI_VDEC_MAX_JPEG_BITSTREAM_BUFFER(width, height, bufsize)\
    (MI_VDEC_JPEG_BITSTREAM_PROGRESSIVE_BUFFER(width, height, bufsize) > MI_VDEC_JPEG_BITSTREAM_BASELINE_BUFFER(bufsize))?MI_VDEC_JPEG_BITSTREAM_PROGRESSIVE_BUFFER(width, height, bufsize):MI_VDEC_JPEG_BITSTREAM_BASELINE_BUFFER(bufsize)


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
typedef struct MI_VDEC_ChkFrmPts_s
{
    MI_BOOL bChkFrmPts;

    MI_U64  u64Pts;
    MI_U32  u32TotalFrmCnt;
} MI_VDEC_ChkFrmPts_t;

typedef struct MI_VDEC_StatPtsIntvl_s
{
    MI_BOOL bStatTime;           // switch of Stat Time interval

    MI_U32  u32TotalFrmCnt;     //Total frame count

    MI_U64  u64PreTime;    		//Previous Time
    MI_U64  u64CurTime;			//Current Time

    MI_U64  u64MaxTimeIntvl; 	//max of Time interval
    MI_U64  u64MinTimeIntvl;   	//min of Time interval
    MI_U64  u64SumTimeIntvl;   	//sum of Time interval
    MI_U64  u64AvgTimeIntvl;   	//average of Time interval
} MI_VDEC_StatPtsIntvl_t;

typedef struct MI_VDEC_StatFrameTimer_s
{
    struct timer_list Timer;
    MI_BOOL bCreate;
}MI_VDEC_StatFrameTimer_t;

typedef struct MI_VDEC_StatPts_s
{
    MI_BOOL bStat;           // switch of Stat all chn pts interval
    MI_U16  u16InChnCnt;
    MI_U16  u16OutChnCnt;

    MI_VDEC_StatPtsIntvl_t stInPtsIntvl[MI_VDEC_MAX_CHN_NUM];
    MI_VDEC_StatPtsIntvl_t stOutPtsIntvl[MI_VDEC_MAX_CHN_NUM];
} MI_VDEC_StatPts_t;

typedef struct MI_VDEC_Output_Info_s
{
    // stat output frmrate
    MS_U32 u32Timeout;
    MI_U16 u16DecCnt;
    MI_U16 u16FrmRate;
} MI_VDEC_ChnOutput_Info_t;

typedef struct MI_VDEC_ProcInfo_s
{
    MI_BOOL bRlsFrmDirect; //release frame buffer directly

    MI_VDEC_ChnOutput_Info_t stChnOutputInfo[MI_VDEC_MAX_CHN_NUM];
    struct semaphore stProcChnLock[MI_VDEC_MAX_CHN_NUM];

} MI_VDEC_ProcInfo_t;

#endif


typedef struct MI_VDEC_RegDevPriData_s
{
    MI_U32 u32Version;
} MI_VDEC_RegDevPriData_t;

typedef struct MI_VDEC_MemBufInfo_s
{
    MI_PHY phyAddr;
    void *pVirAddr;
    MI_U32 u32BufSize;
    MI_U8 u8Miu;
} MI_VDEC_MemBufInfo_t;

typedef struct MI_VDEC_DumpParams_s
{
    MI_U8 au8DumpPath[256];
    MI_BOOL bDumpBS[MI_VDEC_MAX_CHN_NUM];
    MI_BOOL bDumpFB[MI_VDEC_MAX_CHN_NUM];
    MI_BOOL bMappingBS[MI_VDEC_MAX_CHN_NUM];
    MI_BOOL bMappingFB[MI_VDEC_MAX_CHN_NUM];
    MI_U32 u32DisableMFDec;

    MI_BOOL bChkRlsFrmStatus;
    MI_BOOL bChkFrmAddrLegal;
    MI_U32 au32GetFrmCnt[MI_VDEC_MAX_CHN_NUM];
    MI_U32 au32RlsFrmCnt[MI_VDEC_MAX_CHN_NUM];

    MI_BOOL abChkMetaData[MI_VDEC_MAX_CHN_NUM];

    MI_U32 u32FwDbgLv;
} MI_VDEC_DumpParams_t;

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
    MI_U8 u8DisableDeblocking;

    MI_U64 u64JpegDecErr;
    MI_U64 u64JpegDecSuc;
    MI_U64 u64JpegDecBytes;

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
    MI_BOOL bPutFrmJpegTaskRun;
    MI_VDEC_ChnInfo_t astChnInfo[MI_VDEC_MAX_CHN_NUM];
    struct task_struct *ptskInjTask;
    struct task_struct *ptskPutFrmTask;
    struct task_struct *ptskPutJpegFrmTask;
    wait_queue_head_t stInjWaitQueueHead;
    wait_queue_head_t stPutWaitQueueHead;
    wait_queue_head_t stPutWaitJpegQueueHead;
    MI_SYS_DRV_HANDLE hVdecDev;
    MI_VDEC_MemBufInfo_t stCpuMem;
    MI_VDEC_MemBufInfo_t stLxMem;
    MI_VDEC_MemBufInfo_t stDvXcShmMem;
    MI_VDEC_MemBufInfo_t stJpegInternalMem;
    atomic_long_t u64FastChnId[MI_VDEC_MAX_CHN_NUM];
    mi_vdec_DispFrame_t stDispFrm[MI_VDEC_MAX_CHN_NUM];

    struct semaphore semChnLock[MI_VDEC_MAX_CHN_NUM];
    struct semaphore semExitChnLock[MI_VDEC_MAX_CHN_NUM];
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
static MI_VDEC_DumpParams_t _stDumpParams;
extern bool mma_config_exist_max_offset_to_curr_lx_mem;

static MI_U8 *_au8MmaHeapName[MI_VDEC_MAX_USED_MIU] = {"mma_heap_name0", "mma_heap_name1", "mma_heap_name2"};
static MI_U8 *_au8MmaHeapName_vdec[MI_VDEC_MAX_USED_MIU] = {"mma_heap_low_memory", "mma_heap_low_memory", "mma_heap_low_memory"};

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
static MI_VDEC_StatPts_t _stStatPts;
static MI_VDEC_StatFrameTimer_t _stStatInFrmTimer;
static MI_VDEC_StatFrameTimer_t _stStatOutFrmTimer;
static MI_VDEC_ChkFrmPts_t _stInFrmPts[MI_VDEC_MAX_CHN_NUM];
static MI_VDEC_ChkFrmPts_t _stOutFrmPts[MI_VDEC_MAX_CHN_NUM];
static MI_VDEC_ProcInfo_t _stVdecProcInfo;
#endif

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
void _MI_VDEC_StatInputTimeIntvl(MI_VDEC_CHN VdecChn, mi_vdec_BufInfo_t *pstBufInfo);
void _MI_VDEC_StatOutputInfo(MI_VDEC_CHN VdecChn, MI_SYS_BufInfo_t *pstBufInfo);
#endif
void _MI_VDEC_IMPL_DebugDumpFrameBuffer(MI_U32 u32Chn, MI_SYS_BufInfo_t *pstBufInfo);
void _MI_VDEC_IMPL_DebugDumpJpegFrmBuf(MI_U32 u32Chn, mi_vdec_DispFrame_t *pstDispFrm);
void _MI_VDEC_IMPL_DebugDumpAppPushEs(MI_U32 u32Chn, MI_U8 *pu8EsData, MI_U32 u32Len, MI_BOOL bReset);
void _MI_VDEC_IMPL_DebugDumpJpegDecImage(MI_U8 *pYuv422Data, MI_U32 width, MI_U32 height, MI_U32 pitch, int chn);
void _MI_VDEC_IMPL_DebugDumpBitStreamBuffer(MI_U32 u32Chn);
void _MI_VDEC_IMPL_PoolMapping(void);
void _MI_VDEC_IMPL_MMABufMapping(MI_VDEC_CHN VdecChn, MI_BOOL bMapping);

static MI_S32 _MI_VDEC_IMPL_QueryFreeBuffer(MI_VDEC_CHN VdecChn, mi_vdec_BufInfo_t *pstBufInfo)
{
    MI_U32 u32AvailSize = 0;
    MI_PHY phyAddr = 0;
    MI_U32 u32Vacany = 0;

    u32Vacany = MApi_VDEC_EX_GetESBuffVacancy(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), NULL);
    if (u32Vacany >= pstBufInfo->u32BufSize)
    {
        if (E_VDEC_EX_OK == MApi_VDEC_EX_GetESBuff(
            &(_stResMgr.astChnInfo[VdecChn].stVDECStreamId),
            (MS_U32)pstBufInfo->u32BufSize,
            (MS_U32 *)&u32AvailSize,
            (MS_PHY *)&phyAddr))
        {
            pstBufInfo->phyAddr = phyAddr;
            return MI_SUCCESS;
        }
    }

    return E_MI_ERR_FAILED;
}

static MI_S32 _MI_VDEC_IMPL_WriteBufferComplete(MI_VDEC_CHN VdecChn, mi_vdec_BufInfo_t *pstBufInfo)
{
    VDEC_EX_DecCmd stDecCmd;
    memset(&stDecCmd, 0, sizeof(VDEC_EX_DecCmd));
    stDecCmd.u32StAddr = pstBufInfo->phyAddr;
    stDecCmd.u32Size = pstBufInfo->u32BufSize;
    ///TODO: u32ID_H u32ID_L
    stDecCmd.u32ID_H = ((pstBufInfo->u64Pts >> 32)& 0xFFFFFFFF);
    stDecCmd.u32ID_L = (pstBufInfo->u64Pts & 0xFFFFFFFF);
    //stDecCmd.u32Timestamp = (MS_U32)div64_u64(pstBufInfo->u64Pts, 1000);
    stDecCmd.u32Timestamp = (MS_U32)pstBufInfo->u64Pts;

    if (E_VDEC_EX_OK !=
        MApi_VDEC_EX_PushDecQ(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), (VDEC_EX_DecCmd*)(void*)&stDecCmd))
    {
        DBG_ERR("PushDecQ Error\n");
    }

    if (_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType != E_MI_VDEC_CODEC_TYPE_JPEG)
    {
        if (E_VDEC_EX_OK !=
            MApi_VDEC_EX_FireDecCmd(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId)))
        {
            DBG_ERR("FireDecCmd Error\n");
        }
    }

    return MI_SUCCESS;
}

///get frame from driver
static MI_S32 _MI_VDEC_IMPL_GetFrame(MI_VDEC_CHN VdecChn, mi_vdec_DispFrame_t *pstGetDispFrm)
{
    VDEC_EX_DispFrame* pstNextDispFrm;
    MI_BOOL b10Bit;
    VDEC_EX_FrameInfoExt_v6 stFrmInfoExt_v6;
    VDEC_EX_DispInfo stDispInfo;
    MI_PHY phyDbOffset;

    if(MApi_VDEC_EX_GetNextDispFrame(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), &pstNextDispFrm) == E_VDEC_EX_OK)
    {
        pstGetDispFrm->stFrmInfo.phyLumaAddr   = pstNextDispFrm->stFrmInfo.u32LumaAddr;
        pstGetDispFrm->stFrmInfo.phyChromaAddr = pstNextDispFrm->stFrmInfo.u32ChromaAddr;
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
        if (_stDumpParams.bChkFrmAddrLegal)
        {
            DBG_WRN("Chn(%d)m get:luma_0x%llx, chm_0x%llx\n", VdecChn, pstGetDispFrm->stFrmInfo.phyLumaAddr, pstGetDispFrm->stFrmInfo.phyChromaAddr);
        }
    }
    else
    {
        return E_MI_ERR_FAILED;
    }

    atomic_set(&(pstGetDispFrm->u64FastChnId), atomic_read(&(_stResMgr.u64FastChnId[VdecChn])));
    if (_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H264)
    {
        pstGetDispFrm->ePixelFrm = E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE1_H264;
    }
    else if (_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        pstGetDispFrm->ePixelFrm = E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE2_H265;
    }
    else if (_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_JPEG)
    {
        pstGetDispFrm->ePixelFrm = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    }
    else
    {
        pstGetDispFrm->ePixelFrm = E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE1_H264;
        DBG_WRN("Unknow Type, Force Set To H264\n");
    }

    memset(&stDispInfo, 0, sizeof(stDispInfo));
    if(E_VDEC_EX_OK == MApi_VDEC_EX_GetDispInfo(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), &stDispInfo))
    {
        pstGetDispFrm->stDbInfo.u16StartX = stDispInfo.u16CropLeft;
        pstGetDispFrm->stDbInfo.u16StartY = stDispInfo.u16CropTop;

        ///display info
        pstGetDispFrm->stDispInfo.u16CropLeft = stDispInfo.u16CropLeft;
        pstGetDispFrm->stDispInfo.u16CropRight = stDispInfo.u16CropRight;
        pstGetDispFrm->stDispInfo.u16CropTop = stDispInfo.u16CropTop;
        pstGetDispFrm->stDispInfo.u16CropBottom = stDispInfo.u16CropBottom;
        pstGetDispFrm->stDispInfo.u16HorSize = stDispInfo.u16HorSize;
        pstGetDispFrm->stDispInfo.u16Pitch = stDispInfo.u16Pitch;
        pstGetDispFrm->stDispInfo.u16VerSize = stDispInfo.u16VerSize;
        pstGetDispFrm->stDispInfo.u8Interlace = stDispInfo.u8Interlace;
        pstGetDispFrm->stDispInfo.u32FrameRate = stDispInfo.u32FrameRate;
    }
    else
    {
        return E_MI_ERR_FAILED;
    }

    // Jpeg not support b10Bit
    if(E_MI_VDEC_CODEC_TYPE_JPEG != _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType)
    {
        if(b10Bit == TRUE)
        {
            VDEC_EX_FrameInfoEX stFrmInfo;

            if (pstGetDispFrm->ePixelFrm == E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE2_H265)
            {
                pstGetDispFrm->ePixelFrm = E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE3_H265;
            }
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
            pstGetDispFrm->stDbInfo.u16DbPitch = (MI_U16)((u32MFCodecInfo >> 16) & 0xFF);
            pstGetDispFrm->stDbInfo.u8DbMiuSel = (MI_U8)((u32MFCodecInfo >> 24) & 0x0F);
            phyDbOffset = (MI_PHY)stFrmInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32LumaMFCbitlen;
            MI_VDEC_MIU_OFFSET_TO_PHYADDR(pstGetDispFrm->stDbInfo.u8DbMiuSel,phyDbOffset,pstGetDispFrm->stDbInfo.phyDbBase);
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

        // this value not right
        //_stResMgr.astChnInfo[VdecChn].u8EnableMfcodec = pstGetDispFrm->stDbInfo.bUncompressMode;
    }
    else
    {
        if (_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_JPEG)
        {
            return MI_SUCCESS;
        }

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
    stDispFrm.stFrmInfo.u32LumaAddr     = (MS_PHY)pstRlsDispFrm->stFrmInfo.phyLumaAddr;
    stDispFrm.stFrmInfo.u32ChromaAddr   = (MS_PHY)pstRlsDispFrm->stFrmInfo.phyChromaAddr;
    stDispFrm.stFrmInfo.u32TimeStamp    = (MS_U32)pstRlsDispFrm->stFrmInfo.u32TimeStamp;
    stDispFrm.stFrmInfo.u32ID_L         = (MS_U32)pstRlsDispFrm->stFrmInfo.u32IdL;
    stDispFrm.stFrmInfo.u32ID_H         = (MS_U32)pstRlsDispFrm->stFrmInfo.u32IdH;
    stDispFrm.stFrmInfo.u16Pitch        = (MS_U16)pstRlsDispFrm->stFrmInfo.u16Pitch;
    stDispFrm.stFrmInfo.u16Width        = (MS_U16)pstRlsDispFrm->stFrmInfo.u16Width;
    stDispFrm.stFrmInfo.u16Height       = (MS_U16)pstRlsDispFrm->stFrmInfo.u16Height;
    stDispFrm.stFrmInfo.eFrameType      = (VDEC_EX_FrameType)pstRlsDispFrm->stFrmInfo.eFrameType;
    stDispFrm.stFrmInfo.eFieldType      = (VDEC_EX_FieldType)pstRlsDispFrm->stFrmInfo.eFieldType;
    stDispFrm.u32PriData                = (MS_U32)pstRlsDispFrm->u32PriData;
    stDispFrm.u32Idx                    = (MS_U32)pstRlsDispFrm->u32Idx;
    if (_stDumpParams.bChkFrmAddrLegal)
    {
        DBG_WRN("Chn(%d)m rls:luma_0x%llx, chm_0x%llx\n", VdecChn, pstRlsDispFrm->stFrmInfo.phyLumaAddr, pstRlsDispFrm->stFrmInfo.phyChromaAddr);
    }
    if(E_VDEC_EX_OK == MApi_VDEC_EX_ReleaseFrame(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), &stDispFrm))
    {
        return MI_SUCCESS;
    }

    return E_MI_ERR_FAILED;
}

static MI_S32 _MI_VDEC_IMPL_RleaseFrameCallBack(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_BufInfo_t *pstBufInfo)
{
    mi_vdec_DispFrame_t *pstDispFrm = NULL;

    MI_SYS_BUG_ON(!pstChnCurryPort);
    MI_VDEC_ChkChnIdx(pstChnCurryPort->u32ChnId, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(pstChnCurryPort->u32ChnId, E_MI_ERR_UNEXIST);
    DOWN(&_stResMgr.semChnLock[pstChnCurryPort->u32ChnId]);
    if ((_stResMgr.astChnInfo[pstChnCurryPort->u32ChnId].stChnAttr.eCodecType) == E_MI_VDEC_CODEC_TYPE_JPEG)
    {
        UP(&_stResMgr.semChnLock[pstChnCurryPort->u32ChnId]);
        return MI_SUCCESS;
    }

    MI_SYS_BUG_ON(!pstBufInfo);
    MI_SYS_BUG_ON(pstChnCurryPort->eModId != E_MI_MODULE_ID_VDEC);
    MI_SYS_BUG_ON(pstChnCurryPort->u32ChnId >= MI_VDEC_MAX_CHN_NUM);
    MI_SYS_BUG_ON(pstChnCurryPort->u32PortId != 0);
    MI_SYS_BUG_ON(pstBufInfo->eBufType != E_MI_SYS_BUFDATA_META);

    pstDispFrm = pstBufInfo->stMetaData.pVirAddr;
    MI_SYS_BUG_ON(!pstDispFrm);

    if (_stDumpParams.bChkRlsFrmStatus)
    {
        _stDumpParams.au32RlsFrmCnt[pstChnCurryPort->u32ChnId]++;
    }

    if ( atomic_read(&(_stResMgr.u64FastChnId[pstChnCurryPort->u32ChnId])) == atomic_read(&(pstDispFrm->u64FastChnId)) )
    {
        _MI_VDEC_IMPL_RlsFrame(pstChnCurryPort->u32ChnId, pstDispFrm);
    }
    else
    {
        DBG_WRN("Rlease Old Chn(%d) Frame Buffer\n", pstChnCurryPort->u32ChnId);
    }
    UP(&_stResMgr.semChnLock[pstChnCurryPort->u32ChnId]);

    return MI_SUCCESS;
}

MI_S32 _MI_VDEC_IMPL_InjectBuffer(MI_VDEC_CHN VdecChn, MI_SYS_BufInfo_t *pstBufInfo)
{
    mi_vdec_BufInfo_t stBufInfo;
    MI_U32 u32Offset = 0;
    MI_U8 *pu8VirAddr = NULL;
    memset(&stBufInfo, 0x0, sizeof(mi_vdec_BufInfo_t));
    stBufInfo.u32BufSize = pstBufInfo->stRawData.u32ContentSize;
    if (pstBufInfo->eBufType != E_MI_SYS_BUFDATA_RAW)
    {
        DBG_ERR("Chn(%d) pstBufInfo->eBufType != E_MI_SYS_BUFDATA_RAW\n", VdecChn);
        return MI_SUCCESS;
    }

    if (pstBufInfo->stRawData.u32ContentSize > pstBufInfo->stRawData.u32BufSize)
    {
        DBG_ERR("Chn(%d) pstBufInfo->stRawData.u32ContentSize(%d) > pstBufInfo->stRawData.u32BufSize(%d)\n", VdecChn,
            pstBufInfo->stRawData.u32ContentSize, pstBufInfo->stRawData.u32BufSize);
        return MI_SUCCESS;
    }

    if (stBufInfo.u32BufSize >= _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize)
    {
        DBG_ERR("Chn(%d) Out Of Es Buffer Limited, Drop Es. Push Data Size:%d, Es Buffer Size:%d\n", VdecChn,
            stBufInfo.u32BufSize, _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize);
        return MI_SUCCESS;
    }


    if (MI_SUCCESS != _MI_VDEC_IMPL_QueryFreeBuffer(VdecChn, &stBufInfo))
    {
        DBG_INFO("Chn(%d) Query Free Buffer Faild\n", VdecChn);
        return E_MI_ERR_FAILED;
    }

    u32Offset = stBufInfo.phyAddr - _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr;
    pu8VirAddr = (MI_U8 *)(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr) + u32Offset;

#if (0)
    pu8VirAddr = (MI_U8 *)MsOS_MPool_PA2KSEG1(stBufInfo.phyAddr);
#endif

    stBufInfo.u32BufSize = pstBufInfo->stRawData.u32ContentSize;
    memcpy(pu8VirAddr, pstBufInfo->stRawData.pVirAddr, stBufInfo.u32BufSize);

    stBufInfo.u64Pts = pstBufInfo->u64Pts;
    stBufInfo.bEndOfStream = pstBufInfo->bEndOfStream;
    if (MI_SUCCESS != _MI_VDEC_IMPL_WriteBufferComplete(VdecChn, &stBufInfo))
    {
        DBG_ERR("Chn(%d) Push Buffer Faild\n", VdecChn);
        return E_MI_ERR_FAILED;
    }

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
    _MI_VDEC_StatInputTimeIntvl(VdecChn, &stBufInfo);
#endif

    if (_stDumpParams.bDumpBS[VdecChn])
    {
        DBG_WRN("Chn(%d) Dump Es Buffer, Size:%d, (0x%02x 0x%02x 0x%02x 0x%02x 0x%02x)\n",
            VdecChn, stBufInfo.u32BufSize,
            pu8VirAddr[0], pu8VirAddr[1], pu8VirAddr[2], pu8VirAddr[3], pu8VirAddr[4]);
        _MI_VDEC_IMPL_DebugDumpAppPushEs(VdecChn, pstBufInfo->stRawData.pVirAddr, stBufInfo.u32BufSize, FALSE);
    }

    return MI_SUCCESS;
}

static int _MI_VDEC_IMPL_InjectTask(void *pUsrData)
{
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    mi_vdec_BufInfo_t stBufInfo;

    DBG_INFO("Create Inject Task Done\n");
    memset(&stBufInfo, 0x0, sizeof(mi_vdec_BufInfo_t));
    while (_stResMgr.bInjTaskRun && !kthread_should_stop())
    {
        MI_U16 u16Chn = 0;
        MI_BOOL bPushEsBufFaild = TRUE;

        ///wait input port buffer
        if(mi_sys_WaitOnInputTaskAvailable(_stResMgr.hVdecDev, 100) != MI_SUCCESS)
        {
            continue;
        }
        for (u16Chn = 0; u16Chn < MI_VDEC_MAX_CHN_NUM; ++u16Chn)
        {
            DOWN(&_stResMgr.semChnLock[u16Chn]);
            if (FALSE == _stResMgr.astChnInfo[u16Chn].bStart)
            {
                ///current channel haven't start, but have buffer, need to remove
                UP(&_stResMgr.semChnLock[u16Chn]);
                continue;
            }

#if 0
            if (E_MI_VDEC_CODEC_TYPE_JPEG == _stResMgr.astChnInfo[u16Chn].stChnAttr.eCodecType)
            {
                UP(&_stResMgr.semChnLock[u16Chn]);
                continue;
            }
#endif
            DBG_INFO("Chn(%d) meta(%d) Dec Status -> Dec:%d Err:%d Drop:%d Skip:%d\n",
                                u16Chn, sizeof(mi_vdec_DispFrame_t),
                                MApi_VDEC_EX_GetFrameCnt(&(_stResMgr.astChnInfo[u16Chn].stVDECStreamId)),
                                MApi_VDEC_EX_GetErrCnt(&(_stResMgr.astChnInfo[u16Chn].stVDECStreamId)),
                                MApi_VDEC_EX_GetDropCnt(&(_stResMgr.astChnInfo[u16Chn].stVDECStreamId)),
                                MApi_VDEC_EX_GetSkipCnt(&(_stResMgr.astChnInfo[u16Chn].stVDECStreamId)));
            pstBufInfo = mi_sys_GetInputPortBuf(_stResMgr.hVdecDev, u16Chn, 0, MI_SYS_MAP_VA);

            if (_stDumpParams.bChkRlsFrmStatus)
            {
                DBG_WRN("Chn(%d) UnRelease Frame Count:%d\n", u16Chn, _stDumpParams.au32GetFrmCnt[u16Chn] - _stDumpParams.au32RlsFrmCnt[u16Chn]);
            }

            if (pstBufInfo == NULL)
            {
                ///current channel haven't es stream
                UP(&_stResMgr.semChnLock[u16Chn]);
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
            UP(&_stResMgr.semChnLock[u16Chn]);
        }

        if (bPushEsBufFaild)///refine wait condition
        {
            ///have input buffer, but can't push buffer to any channel
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            interruptible_sleep_on_timeout(&_stResMgr.stInjWaitQueueHead, 2);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            wait_event_interruptible_timeout(_stResMgr.stInjWaitQueueHead, atomic_read(&stInjWaitQueueHead_red_flag) > 0, msecs_to_jiffies(2));
#endif
            ///wake_up_interruptible
            atomic_dec(&stInjWaitQueueHead_red_flag);
        }
        else
        {
            atomic_inc(&stPutWaitQueueHead_red_flag);
            wake_up_interruptible(&_stResMgr.stPutWaitQueueHead);
        }
    }

    return 0;
}

static int _MI_VDEC_IMPL_PutFrmTask(void *pUsrData)
{
    MI_SYS_BufConf_t stBufCfg;
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    mi_vdec_DispFrame_t *pstDispFrm = NULL;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    MI_BOOL abRlsFrame[MI_VDEC_MAX_CHN_NUM];

    DBG_INFO("Create Put Frame Task Done\n");
    memset(&stBufCfg, 0x0, sizeof(MI_SYS_BufConf_t));
    memset(abRlsFrame, TRUE, sizeof(MI_BOOL)*MI_VDEC_MAX_CHN_NUM);
    stBufCfg.eBufType = E_MI_SYS_BUFDATA_META;
    stBufCfg.u64TargetPts = MI_SYS_INVALID_PTS;
    stBufCfg.stMetaCfg.u32Size = sizeof(mi_vdec_DispFrame_t);
    stBufCfg.u32Flags = MI_SYS_MAP_VA;

    /*
    No need set MI_SYS_MAP_CPU_WRITE flag.
    Since the data is meta data here.
    */
    if (stBufCfg.stMetaCfg.u32Size > 256)
    {
        DBG_ERR("stBufCfg.stMetaCfg.u32Size (%d) > MI_SYS_META_DATA_SIZE_MAX (%d)\n",
            stBufCfg.stMetaCfg.u32Size, 256);
        MI_SYS_BUG();
    }

    while (_stResMgr.bPutFrmTaskRun)
    {
        MI_U16 u16Chn = 0;
        MI_BOOL bPutFrm = FALSE;
        for (u16Chn = 0; u16Chn < MI_VDEC_MAX_CHN_NUM; ++u16Chn)
        {
            DOWN(&(_stResMgr.semExitChnLock[u16Chn]));
            if ((FALSE == _stResMgr.astChnInfo[u16Chn].bStart)
                || (_stResMgr.astChnInfo[u16Chn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_JPEG))
            {
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }

            // stat frame rate
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
            if(jiffies > _stVdecProcInfo.stChnOutputInfo[u16Chn].u32Timeout)
            {
                _stVdecProcInfo.stChnOutputInfo[u16Chn].u16FrmRate = _stVdecProcInfo.stChnOutputInfo[u16Chn].u16DecCnt;
                _stVdecProcInfo.stChnOutputInfo[u16Chn].u16DecCnt = 0;
                _stVdecProcInfo.stChnOutputInfo[u16Chn].u32Timeout = jiffies + 1*HZ;
            }
#endif

            /// check if get frame already but not release
            if(TRUE == abRlsFrame[u16Chn])
            {
                memset(&_stResMgr.stDispFrm[u16Chn], 0, sizeof(mi_vdec_DispFrame_t));
                if (MI_SUCCESS != _MI_VDEC_IMPL_GetFrame(u16Chn, &_stResMgr.stDispFrm[u16Chn]))
                {
                    //DBG_INFO("VdecChn(%d) get frame failed!!\n", u16Chn);
                    schedule();
                    atomic_set(&(_stResMgr.stDispFrm[u16Chn].u64FastChnId), 0xFFFFFFFFFFFFFFFF);
                    UP(&(_stResMgr.semExitChnLock[u16Chn]));
                    continue;
                }
                else
                {
                    abRlsFrame[u16Chn] = FALSE;
                    // debug info
                    DBG_INFO("VdecChn(%d) get frame success!!\n", u16Chn);
                }
            }

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
            /// release frame buffer directly, not get output buffer
            if(_stVdecProcInfo.bRlsFrmDirect)
            {
                _stVdecProcInfo.stChnOutputInfo[u16Chn].u16DecCnt += 1;
                _MI_VDEC_IMPL_RlsFrame(u16Chn, &_stResMgr.stDispFrm[u16Chn]);
                abRlsFrame[u16Chn] = TRUE;
                bPutFrm = TRUE;
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }
#endif

            pstBufInfo = NULL;
            pstBufInfo = mi_sys_GetOutputPortBuf(_stResMgr.hVdecDev, u16Chn, 0, &stBufCfg ,&bBlockedByRateCtrl);
            if (pstBufInfo == NULL)
            {
                ///no output buffer
                if (E_MI_VDEC_DISPLAY_MODE_PREVIEW == _stResMgr.astChnInfo[u16Chn].eDisplayMode)
                {
                    ///maybe blocked by others, preview need to drop frame
                    _MI_VDEC_IMPL_RlsFrame(u16Chn, &_stResMgr.stDispFrm[u16Chn]);
                    abRlsFrame[u16Chn] = TRUE;
                    bPutFrm = TRUE;
                }
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }
            else
            {
                // debug info
                DBG_INFO("VdecChn(%d) get ouput port buffer success!!\n", u16Chn);
            }

            if (pstBufInfo->eBufType != E_MI_SYS_BUFDATA_META) {
                DBG_ERR("stBufCfg eBufType= (%d) u64TargetPts= (%d) stMetaCfg.u32Size=(%d) u32Flags=(%d)\n",
                stBufCfg.eBufType, stBufCfg.u64TargetPts, stBufCfg.stMetaCfg.u32Size, stBufCfg.u32Flags);

                DBG_ERR("pstBufInfo eBufType= (%d) u64TargetPts= (%d) \n",
                pstBufInfo->eBufType, pstBufInfo->u64Pts);

                MI_SYS_BUG_ON(pstBufInfo->eBufType != E_MI_SYS_BUFDATA_META);
            }

            pstDispFrm = pstBufInfo->stMetaData.pVirAddr;
            MI_SYS_BUG_ON(!pstDispFrm);

            if (pstBufInfo->stMetaData.u32Size < sizeof(mi_vdec_DispFrame_t))
            {
                DBG_WRN("pstBufInfo->stMetaData.u32Size(%d) < sizeof(mi_vdec_DispFrame_t)\n",
                pstBufInfo->stMetaData.u32Size);
                atomic_set(&(pstDispFrm->u64FastChnId), 0xFFFFFFFFFFFFFFFF);
                mi_sys_RewindBuf(pstBufInfo);
                pstDispFrm = NULL;
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }

            memcpy(pstDispFrm, &_stResMgr.stDispFrm[u16Chn], sizeof(mi_vdec_DispFrame_t));
            ///Check Luma And Chroma Addr
            if (((pstDispFrm->stFrmInfo.phyLumaAddr + pstDispFrm->stFrmInfo.u16Width * pstDispFrm->stFrmInfo.u16Height)
                      > (_stResMgr.astChnInfo[u16Chn].stFrameMemInfo.phyAddr + _stResMgr.astChnInfo[u16Chn].stFrameMemInfo.u32BufSize))
                 || ((pstDispFrm->stFrmInfo.phyChromaAddr + (pstDispFrm->stFrmInfo.u16Width * pstDispFrm->stFrmInfo.u16Height >> 1))
                      > (_stResMgr.astChnInfo[u16Chn].stFrameMemInfo.phyAddr + _stResMgr.astChnInfo[u16Chn].stFrameMemInfo.u32BufSize))
                )
            {
                atomic_set(&(pstDispFrm->u64FastChnId), 0xFFFFFFFFFFFFFFFF);
                mi_sys_RewindBuf(pstBufInfo);
                DBG_ERR("L/C Address Out Of Bound\n");
                _MI_VDEC_IMPL_RlsFrame(u16Chn, pstDispFrm);
                abRlsFrame[u16Chn] = TRUE;
                pstDispFrm = NULL;
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }

            /// Check PTS And Abandon Frame
            if(-2 == pstDispFrm->stFrmInfo.u32TimeStamp)
            {
                atomic_set(&(pstDispFrm->u64FastChnId), 0xFFFFFFFFFFFFFFFF);
                mi_sys_RewindBuf(pstBufInfo);
                DBG_INFO("VdecChn(%d) will Abandon Frame if PTS is %d\n", u16Chn, pstDispFrm->stFrmInfo.u32TimeStamp);
                _MI_VDEC_IMPL_RlsFrame(u16Chn, pstDispFrm);
                abRlsFrame[u16Chn] = TRUE;
                pstDispFrm = NULL;
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }

            pstBufInfo->stMetaData.eDataFromModule = E_MI_MODULE_ID_VDEC;
            pstBufInfo->stMetaData.u32Size = sizeof(mi_vdec_DispFrame_t);
            pstBufInfo->u64Pts = (MI_U64)(pstDispFrm->stFrmInfo.u32TimeStamp) * 1000ULL;

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
            _MI_VDEC_StatOutputInfo(u16Chn, pstBufInfo);
#endif

            if (_stDumpParams.abChkMetaData[u16Chn])
            {
                MI_U32 i = 0;
                MI_U32 u32CheckSum = 0;
                MI_U8 *p = NULL;
                DBG_INFO("Chn(%d), Bufino(bEndOfStream:%d, bUsrBuf:%d, eBufType:%d, Pts:%lld)\n",
                    u16Chn,
                    pstBufInfo->bEndOfStream,
                    pstBufInfo->bUsrBuf,
                    pstBufInfo->eBufType,
                    pstBufInfo->u64Pts);

                DBG_INFO("Chn(%d), eDataFromModule:%d, phyAddr:0x%llx, pVirAddr:0x%p, u32Size:%d\n",
                    u16Chn,
                    pstBufInfo->stMetaData.eDataFromModule,
                    pstBufInfo->stMetaData.phyAddr,
                    pstBufInfo->stMetaData.pVirAddr,
                    pstBufInfo->stMetaData.u32Size);
                p = (MI_U8 *)(pstBufInfo->stMetaData.pVirAddr);
                for (i = 0; i < pstBufInfo->stMetaData.u32Size; ++i)
                {
                    u32CheckSum += p[i];
                }

                DBG_INFO("Chn(%d), pts(%lld) u32CheckSum:%d\n", u16Chn, pstBufInfo->u64Pts, u32CheckSum);
            }

            if (_stDumpParams.bDumpFB[u16Chn])
            {
                if(!_stDumpParams.bMappingFB[u16Chn])
                {
                    _MI_VDEC_IMPL_MMABufMapping(u16Chn, TRUE);
                }
                _MI_VDEC_IMPL_DebugDumpFrameBuffer(u16Chn, pstBufInfo);
            }

            mi_sys_FinishBuf(pstBufInfo);

            if (_stDumpParams.bChkRlsFrmStatus)
            {
                _stDumpParams.au32GetFrmCnt[u16Chn]++;
            }

            pstDispFrm = NULL;
            abRlsFrame[u16Chn] = TRUE;
            bPutFrm = TRUE;
            UP(&(_stResMgr.semExitChnLock[u16Chn]));
        }

        if (bPutFrm)
        {
            atomic_inc(&stInjWaitQueueHead_red_flag);
            wake_up_interruptible(&_stResMgr.stInjWaitQueueHead);
        }
        else
        {
            ///sleep
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            interruptible_sleep_on_timeout(&_stResMgr.stPutWaitQueueHead, 2);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            wait_event_interruptible_timeout(_stResMgr.stPutWaitQueueHead, atomic_read(&stPutWaitQueueHead_red_flag) > 0, msecs_to_jiffies(2));
#endif
            atomic_dec(&stPutWaitQueueHead_red_flag);
        }
    }
    return MI_SUCCESS;
}

static int _MI_VDEC_IMPL_PutJpegFrmTask(void *pUsrData)
{
    MI_SYS_BufConf_t  stBufCfg;
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    mi_vdec_DispFrame_t stDispFrm;
    MI_BOOL bBlockedByRateCtrl = FALSE;

    DBG_INFO("Create Put Jpeg Frame Task Done\n");
    memset(&stBufCfg, 0x0, sizeof(MI_SYS_BufConf_t));
    stBufCfg.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufCfg.u64TargetPts = MI_SYS_INVALID_PTS;

    while (_stResMgr.bPutFrmJpegTaskRun)
    {
        MI_U16  u16Chn = 0;
        MI_BOOL bPutFrm = FALSE;
        for (u16Chn = 0; u16Chn < MI_VDEC_MAX_CHN_NUM; ++u16Chn)
        {
            DOWN(&(_stResMgr.semExitChnLock[u16Chn]));
            if ((FALSE == _stResMgr.astChnInfo[u16Chn].bStart)
                || (E_MI_VDEC_CODEC_TYPE_JPEG != _stResMgr.astChnInfo[u16Chn].stChnAttr.eCodecType))
            {
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }

            // stat frame rate
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
            if(jiffies > _stVdecProcInfo.stChnOutputInfo[u16Chn].u32Timeout)
            {
                _stVdecProcInfo.stChnOutputInfo[u16Chn].u16FrmRate = _stVdecProcInfo.stChnOutputInfo[u16Chn].u16DecCnt;
                _stVdecProcInfo.stChnOutputInfo[u16Chn].u16DecCnt = 0;
                _stVdecProcInfo.stChnOutputInfo[u16Chn].u32Timeout = jiffies + 1*HZ;
            }
#endif

            stBufCfg.stFrameCfg.eFormat        = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            stBufCfg.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
            stBufCfg.stFrameCfg.u16Height      = _ALIGN_BYTE_(_stResMgr.astChnInfo[u16Chn].stChnAttr.u32PicHeight, MI_VDEC_IMAGE_PITCH_ALIGN_SIZE);
            stBufCfg.stFrameCfg.u16Width       = _ALIGN_BYTE_(_stResMgr.astChnInfo[u16Chn].stChnAttr.u32PicWidth, MI_VDEC_IMAGE_PITCH_ALIGN_SIZE);

            pstBufInfo = NULL;
            pstBufInfo = mi_sys_GetOutputPortBuf(_stResMgr.hVdecDev, u16Chn, 0, &stBufCfg, &bBlockedByRateCtrl);
            if (NULL == pstBufInfo)
            {
                ///no output buffer
                if (E_MI_VDEC_DISPLAY_MODE_PREVIEW == _stResMgr.astChnInfo[u16Chn].eDisplayMode)
                {
                    ///maybe blocked by others, preview need to drop frame
                    mi_vdec_DispFrame_t stDispFrmTmp;
                    memset(&stDispFrmTmp, 0x0, sizeof(mi_vdec_DispFrame_t));
                    if (MI_SUCCESS == _MI_VDEC_IMPL_GetFrame(u16Chn, &stDispFrmTmp))
                    {
                        _MI_VDEC_IMPL_RlsFrame(u16Chn, &stDispFrmTmp);
                        bPutFrm = TRUE;
                    }
                }
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }
            else
            {
                // debug info
                DBG_INFO("VdecChn(%d) get ouput port buffer success!!\n", u16Chn);
            }

            //check BufType
            if (E_MI_SYS_BUFDATA_FRAME != pstBufInfo->eBufType)
            {
                DBG_ERR("Input BufCfg eBufType= (%d) u64TargetPts= (%d)\n", stBufCfg.eBufType, stBufCfg.u64TargetPts);
                DBG_ERR("Get BufInfo eBufType= (%d) u64TargetPts= (%d) \n", pstBufInfo->eBufType, pstBufInfo->u64Pts);

                MI_SYS_BUG_ON(E_MI_SYS_BUFDATA_FRAME != pstBufInfo->eBufType);
            }

            // check buffer u16Width and u16Height
            if ((pstBufInfo->stFrameData.u16Height < stBufCfg.stFrameCfg.u16Height)
                || (pstBufInfo->stFrameData.u16Width < stBufCfg.stFrameCfg.u16Width))
            {
                DBG_ERR("get buffer u16Height(%d) < config u16Height(%d) or "
                    "u16Width(%d) < config u16Width(%d).\n",
                    stDispFrm.stFrmInfo.u16Height, stBufCfg.stFrameCfg.u16Height,
                    stDispFrm.stFrmInfo.u16Width, stBufCfg.stFrameCfg.u16Width);

                atomic_set(&(stDispFrm.u64FastChnId), 0xFFFFFFFFFFFFFFFF);
                _MI_VDEC_IMPL_RlsFrame(u16Chn, &stDispFrm);
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }

            memset(&stDispFrm, 0, sizeof(mi_vdec_DispFrame_t));
            if (MI_SUCCESS != _MI_VDEC_IMPL_GetFrame(u16Chn, &stDispFrm))
            {
                //DBG_INFO("VdecChn(%d) get frame failed!!\n", u16Chn);
                atomic_set(&(stDispFrm.u64FastChnId), 0xFFFFFFFFFFFFFFFF);
                mi_sys_RewindBuf(pstBufInfo);
                UP(&(_stResMgr.semExitChnLock[u16Chn]));
                continue;
            }
            else
            {
                // debug info
                DBG_INFO("VdecChn(%d) get frame success!!\n", u16Chn);
            }

            // dump frame
            if (_stDumpParams.bDumpFB[u16Chn])
            {
                _MI_VDEC_IMPL_DebugDumpJpegFrmBuf(u16Chn, &stDispFrm);
            }

            pstBufInfo->u64Pts = (MI_U64)(stDispFrm.stFrmInfo.u32TimeStamp) * 1000ULL;
            pstBufInfo->stFrameData.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            pstBufInfo->stFrameData.u16Width     = stDispFrm.stFrmInfo.u16Width;
            pstBufInfo->stFrameData.u16Height    = stDispFrm.stFrmInfo.u16Height;
            pstBufInfo->stFrameData.u32Stride[0] = stDispFrm.stFrmInfo.u16Pitch * 2;
            pstBufInfo->stFrameData.phyAddr[0]   = stDispFrm.stFrmInfo.phyLumaAddr;
            _MI_VDEC_IMPL_RlsFrame(u16Chn, &stDispFrm);

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
            _MI_VDEC_StatOutputInfo(u16Chn, pstBufInfo);
#endif

            ///TODO: save output info

            mi_sys_FinishBuf(pstBufInfo);

            if (_stDumpParams.bChkRlsFrmStatus)
            {
                _stDumpParams.au32GetFrmCnt[u16Chn]++;
            }

            bPutFrm = TRUE;
            UP(&(_stResMgr.semExitChnLock[u16Chn]));
        }

        if (bPutFrm)
        {
            atomic_inc(&stInjWaitQueueHead_red_flag);
            wake_up_interruptible(&_stResMgr.stInjWaitQueueHead);
        }
        else
        {
            ///sleep
#if LINUX_VERSION_CODE == KERNEL_VERSION(3, 10, 40)
            interruptible_sleep_on_timeout(&_stResMgr.stPutWaitQueueHead, 2);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3, 18, 30)
            wait_event_interruptible_timeout(_stResMgr.stPutWaitQueueHead, atomic_read(&stPutWaitQueueHead_red_flag) > 0, msecs_to_jiffies(2));
#endif
            atomic_dec(&stPutWaitQueueHead_red_flag);
        }
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_IMPL_StartChn(MI_VDEC_CHN VdecChn)
{
    VDEC_EX_DecModCfg stDecModCfg;
    VDEC_EX_INPUT_TSP eInputTSP = E_VDEC_EX_INPUT_TSP_NONE;
    VDEC_EX_DynmcDispPath stDynmcDispPath;
    VDEC_EX_DISPLAY_MODE eDispMode = E_VDEC_EX_DISPLAY_MODE_MCU;
    VDEC_EX_MFCodec_mode eMFCodecMode = E_VDEC_EX_MFCODEC_FORCE_ENABLE;
    VDEC_EX_InitParam stInitParams;
    VDEC_EX_CodecType eVdecCodecType = E_VDEC_EX_CODEC_TYPE_NONE;
#if NOSUPPORT_MsOS_MPool_Add_PA2VARange
    VDEC_EX_BufferInfo stBitstreamBufInfo;
    VDEC_EX_BufferInfo stFrameBufInfo;
#endif
    VDEC_EX_BufferInfo stDvXcShmBufInfo;

    if (FALSE == _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec)
    {
        eMFCodecMode = E_VDEC_EX_MFCODEC_FORCE_DISABLE;
        DBG_WRN("Disable MFDec Mode, %d\n", eMFCodecMode);
    }

    if (_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H264)
    {
        eVdecCodecType = E_VDEC_EX_CODEC_TYPE_H264;
    }
    else if (_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        eVdecCodecType = E_VDEC_EX_CODEC_TYPE_HEVC;
    }
    else if (_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_JPEG)
    {
        eVdecCodecType = E_VDEC_EX_CODEC_TYPE_MJPEG;
    }
    else
    {
        DBG_EXIT_ERR("UnSupport Codec Type:%d\n", _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType);
        return E_MI_ERR_NOT_SUPPORT;
    }

    if (E_VDEC_EX_OK != MApi_VDEC_EX_GetFreeStream(&_stResMgr.astChnInfo[VdecChn].stVDECStreamId,
        sizeof(VDEC_StreamId), E_VDEC_EX_N_STREAM, eVdecCodecType))
    {
        DBG_EXIT_ERR("Get FreeStream Fail:%d %d\n", _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType, eVdecCodecType);
        return E_MI_ERR_BUSY;
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
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_MFCODEC_MODE, (MS_U32)eMFCodecMode);

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
        stInitParams.VideoInfo.u32FrameRate = 30;
        stInitParams.VideoInfo.u32FrameRateBase = 30;
    }
    else
    {
        MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        DBG_EXIT_ERR("Vdec Chn(%d) UnSupport Codec(%d) Failed\n", VdecChn, _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType);
        return E_MI_ERR_NOT_SUPPORT;
    }

    stInitParams.VideoInfo.eSrcMode = E_VDEC_EX_SRC_MODE_FILE;
    stInitParams.VideoInfo.eTimeStampType = E_VDEC_EX_TIME_STAMP_PTS;

    stInitParams.EnableDynaScale = FALSE;
    stInitParams.bDisableDropErrFrame = TRUE;
    stInitParams.bRepeatLastField = TRUE;
#if (defined CONFIG_MSTAR_CHIP_I2) && (CONFIG_MSTAR_CHIP_I2 == 1)
    stInitParams.bDisableErrConceal = FALSE;
#else //k6 or k6l need disable
    stInitParams.bDisableErrConceal = TRUE;
#endif
    ///cpu buffer
    stInitParams.SysConfig.u32CodeBufAddr = _stResMgr.astChnInfo[VdecChn].stCpuMemInfo.phyAddr;
    stInitParams.SysConfig.u32CodeBufSize = _stResMgr.astChnInfo[VdecChn].stCpuMemInfo.u32BufSize;
    stInitParams.SysConfig.u32FWBinaryAddr = stInitParams.SysConfig.u32CodeBufAddr;
    stInitParams.SysConfig.u32FWBinarySize = stInitParams.SysConfig.u32CodeBufSize;

    ///framebuffer
    stInitParams.SysConfig.u32FrameBufAddr = _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr;
    stInitParams.SysConfig.u32FrameBufSize = _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize;

#if NOSUPPORT_MsOS_MPool_Add_PA2VARange
    ///set all frame buffer base address and size
    ///1. vdec fw base address offset vdec_cpu address
    ///2. frame buffer must behind vdec_cpu
    /* VDEC_EX_BufferInfo stFrameBufInfo; */
    memset(&stFrameBufInfo, 0, sizeof(VDEC_EX_BufferInfo));
    stFrameBufInfo.eType     = E_VDEC_EX_BUFFERTYPE_FB_MAIN;
    stFrameBufInfo.phyAddr = _stResMgr.stLxMem.phyAddr;
    stFrameBufInfo.szSize = _stResMgr.stLxMem.u32BufSize;
    stFrameBufInfo.u32Config = 0;
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SET_BUFFER_INFO, (MS_VIRT)&stFrameBufInfo);
#endif
    ///bitstream buffer
    stInitParams.SysConfig.u32BitstreamBufAddr = _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr;
    stInitParams.SysConfig.u32BitstreamBufSize = _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize;
    stInitParams.SysConfig.u32DrvProcBufAddr = stInitParams.SysConfig.u32BitstreamBufAddr;
#if (defined CONFIG_MSTAR_CHIP_I2) && (CONFIG_MSTAR_CHIP_I2 == 1)
    if (E_MI_VDEC_CODEC_TYPE_JPEG == _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType)
    {
        stInitParams.SysConfig.u32DrvProcBufSize = 0x3FC000;
    }
    else
    {
        stInitParams.SysConfig.u32DrvProcBufSize = 0xA000;
    }
    if (stInitParams.SysConfig.u32DrvProcBufSize % 4096)
    {
        stInitParams.SysConfig.u32DrvProcBufSize
            +=(4096 - (stInitParams.SysConfig.u32DrvProcBufSize % 4096));
    }
#else
    stInitParams.SysConfig.u32DrvProcBufSize = 0;
#endif

#if NOSUPPORT_MsOS_MPool_Add_PA2VARange
    ///set all bitstream buffer base address and size
    ///1. vdec fw base address offset vdec_cpu address
    ///2. bitstream buffer must behind vdec_cpu
    /* VDEC_EX_BufferInfo stBitstreamBufInfo; */
    memset(&stBitstreamBufInfo, 0, sizeof(VDEC_EX_BufferInfo));
    stBitstreamBufInfo.eType     = E_VDEC_EX_BUFFERTYPE_BS_MAIN;
    stBitstreamBufInfo.phyAddr = _stResMgr.stLxMem.phyAddr;
    stBitstreamBufInfo.szSize = _stResMgr.stLxMem.u32BufSize;
    stBitstreamBufInfo.u32Config = 0;
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SET_BUFFER_INFO, (MS_VIRT)&stBitstreamBufInfo);
#endif

    ///set dv xc shm info
    memset(&stDvXcShmBufInfo, 0, sizeof(VDEC_EX_BufferInfo));
    stDvXcShmBufInfo.eType     = E_VDEC_EX_BUFFERTYPE_HDRSHM_MAIN;
    stDvXcShmBufInfo.phyAddr = _stResMgr.stDvXcShmMem.phyAddr;
    stDvXcShmBufInfo.szSize = _stResMgr.stDvXcShmMem.u32BufSize;
    stDvXcShmBufInfo.u32Config = 0;
    MApi_VDEC_EX_PreSetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SET_BUFFER_INFO, (MS_VIRT)&stDvXcShmBufInfo);

    DBG_INFO("Chn(%d) CPU:(0x%llx, 0x%x) Framebuffer:(0x%llx, 0x%x) Bitstream:(0x%llx, 0x%x) MS_PHY:%d\n",
        VdecChn,
        stInitParams.SysConfig.u32CodeBufAddr,
        stInitParams.SysConfig.u32CodeBufSize,
        stInitParams.SysConfig.u32FrameBufAddr,
        stInitParams.SysConfig.u32FrameBufSize,
        stInitParams.SysConfig.u32BitstreamBufAddr,
        stInitParams.SysConfig.u32BitstreamBufSize, sizeof(MS_PHY));

    stInitParams.SysConfig.eDbgMsgLevel = (VDEC_EX_DbgLevel)_stDumpParams.u32FwDbgLv;
    if (E_VDEC_EX_OK != MApi_VDEC_EX_Init(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), &stInitParams))
    {
        MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        DBG_EXIT_ERR("Vdec Chn(%d) Init Failed\n", VdecChn);
        return E_MI_ERR_BUSY;
    }

    MApi_VDEC_EX_SetBlockDisplay(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), TRUE);
    MApi_VDEC_EX_EnableESBuffMalloc(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), TRUE);
    MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SET_DISP_OUTSIDE_CTRL_MODE, TRUE);
    MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SET_DISP_FINISH_MODE, TRUE);
    MApi_VDEC_EX_DisableDeblocking(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), FALSE);
    MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_AVC_SUPPORT_REF_NUM_OVER_MAX_DPB_SIZE, TRUE);
    ///set decoder order output
    MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER, TRUE);

#if (defined SUPPORT_VDEC_MULTI_RES) && (SUPPORT_VDEC_MULTI_RES == 1)
    ///support multi-4k for h26x
    MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SUPPORT_MULTI_ULTRA_RES, TRUE);
#endif

    MApi_VDEC_EX_AVSyncOn(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), FALSE, 180, 40);
    if (E_VDEC_EX_OK != MApi_VDEC_EX_Play(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId)))
    {
        MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        DBG_EXIT_ERR("Vdec Chn(%d) Play Failed\n", VdecChn);
        return E_MI_ERR_FAILED;
    }

    //_stResMgr.astChnInfo[VdecChn].u8EnableMfcodec = TRUE;
    _stResMgr.astChnInfo[VdecChn].u8DisableDeblocking = FALSE;

    DBG_INFO("Start Chn(%d) u32Id(%d) Done\n", VdecChn, _stResMgr.astChnInfo[VdecChn].stVDECStreamId.u32Id);
    return MI_SUCCESS;
}

static MI_VDEC_ErrCode_e _MI_VDEC_TransDrvErrCode2MiType(MI_U32 u32ErrCode)
{
    MI_VDEC_ErrCode_e eErrCode = E_MI_VDEC_ERR_CODE_UNKNOW;
    switch (u32ErrCode)
    {
        case E_VDEC_EX_ERR_CODE_ILLEGAL_ACCESS:
            eErrCode = E_MI_VDEC_ERR_CODE_ILLEGAL_ACCESS;
            break;
        case E_VDEC_EX_ERR_CODE_NOT_SUPPORT:
        case E_VDEC_EX_ERR_CODE_FRMRATE_NOT_SUPPORT:
            eErrCode = E_MI_VDEC_ERR_CODE_FRMRATE_UNSUPPORT;
            break;
        case E_VDEC_EX_HVD_ERR_CODE_HW_DEC_TIMEOUT:
            eErrCode = E_MI_VDEC_ERR_CODE_DEC_TIMEOUT;
            break;
        case E_VDEC_EX_HVD_ERR_CODE_OUT_OF_MEMORY:
        case E_VDEC_EX_HVD_ERR_CODE_AVC_RES:
            eErrCode = E_MI_VDEC_ERR_CODE_OUT_OF_MEMORY;
            break;
        case E_VDEC_EX_HVD_ERR_CODE_UNKNOW_ERR:
        case E_VDEC_EX_HVD_ERR_CODE_OUT_OF_SPEC:
        case E_VDEC_EX_HVD_ERR_CODE_HW_BREAK_DOWN:
        case E_VDEC_EX_HVD_ERR_CODE_UNKNOWN_CODEC:
            eErrCode = E_MI_VDEC_ERR_CODE_CODEC_TYPE_UNSUPPORT;
            break;
        case E_VDEC_EX_HVD_ERR_CODE_AVC_SPS_BROKEN:
        case E_VDEC_EX_HVD_ERR_CODE_AVC_SPS_NOT_IN_SPEC:
        case E_VDEC_EX_HVD_ERR_CODE_AVC_SPS_NOT_ENOUGH_FRM:
            eErrCode = E_MI_VDEC_ERR_CODE_ERR_SPS_UNSUPPORT;
            break;
        case E_VDEC_EX_HVD_ERR_CODE_AVC_PPS_BROKEN:
            eErrCode = E_MI_VDEC_ERR_CODE_ERR_PPS_UNSUPPORT;
            break;
        case E_VDEC_EX_HVD_ERR_CODE_AVC_REF_LIST:
        case E_VDEC_EX_HVD_ERR_CODE_AVC_NO_REF:
            eErrCode = E_MI_VDEC_ERR_CODE_REF_LIST_ERR;
            break;
        default:
            eErrCode = E_MI_VDEC_ERR_CODE_UNKNOW;
    }

    return eErrCode;
}

MI_U32 _MI_VDEC_IMPL_GetMmapInfo(MI_U8 *pu8MmapName, MI_VDEC_MemBufInfo_t *pstMmapInfo)
{
    MI_SYSCFG_MmapInfo_t *pstMmap = NULL;
    if (!MI_SYSCFG_GetMmapInfo(pu8MmapName, (const MI_SYSCFG_MmapInfo_t **)&pstMmap))
    {
        DBG_ERR("Get Mmap(%s) Error\n", pu8MmapName);
        return E_MI_ERR_FAILED;
    }

    pstMmapInfo->phyAddr = pstMmap->u32Addr;
    pstMmapInfo->u32BufSize = pstMmap->u32Size;
    pstMmapInfo->u8Miu = (MI_U8)pstMmap->u8MiuNo;
    DBG_INFO("Addr:0x%llx, Size:0x%x, Miu:%d\n", pstMmapInfo->phyAddr, pstMmapInfo->u32BufSize, pstMmapInfo->u8Miu);
    pstMmapInfo->pVirAddr = NULL;
    return MI_SUCCESS;
}

MI_U32 _MI_VDEC_IMPL_PutCpuBuffer(MI_VDEC_MemBufInfo_t *pstCpuMemInfo)
{
    if (pstCpuMemInfo->pVirAddr)
    {
#if !NOSUPPORT_MsOS_MPool_Add_PA2VARange
        MsOS_MPool_Remove_PA2VARange(
            pstCpuMemInfo->phyAddr,
            (MS_U32)pstCpuMemInfo->pVirAddr,
            _ALIGN_BYTE_(pstCpuMemInfo->u32BufSize, MI_VDEC_PAGE_SIZE),
            TRUE);
#endif
        mi_sys_UnVmap(pstCpuMemInfo->pVirAddr);
    }
    return MI_SUCCESS;
}

static void _MI_VDEC_IMPL_InitVar(void)
{
    MI_U16 u16Chn = 0;

    memset(&_stResMgr, 0x0, sizeof(MI_VDEC_ResMgr_t));
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
    memset(&_stVdecProcInfo, 0x0, sizeof(MI_VDEC_ProcInfo_t));
#endif

    init_waitqueue_head(&_stResMgr.stInjWaitQueueHead);
    init_waitqueue_head(&_stResMgr.stPutWaitQueueHead);
    init_waitqueue_head(&_stResMgr.stPutWaitJpegQueueHead);

    for (u16Chn = 0; u16Chn < MI_VDEC_MAX_CHN_NUM; ++u16Chn)
    {
        sema_init(&(_stResMgr.semChnLock[u16Chn]), 1);
        sema_init(&(_stResMgr.semExitChnLock[u16Chn]), 1);
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
        sema_init(&_stVdecProcInfo.stProcChnLock[u16Chn], 1);
#endif
    }

    _MI_VDEC_IMPL_PoolMapping();
}

void _MI_VDEC_IMPL_MMABufMapping(MI_VDEC_CHN VdecChn, MI_BOOL bMapping)
{
    ///MS_PA2KSEG1 == MsOS_MPool_PA2KSEG1 == mi_sys_Vmap(FALSE), MsOS_MPool_Add_PA2VARange(TRUE)
    ///MS_PA2KSEG0 == MsOS_MPool_PA2KSEG0 == mi_sys_Vmap(TRUE), MsOS_MPool_Add_PA2VARange(FALSE)
    ///enale mma es buffer memory to msos mmap

    if (bMapping)
    {
        ///mmap
        if (!_stDumpParams.bMappingBS[VdecChn])
        {
            _stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr = mi_sys_Vmap(
                _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                FALSE);

            MsOS_MPool_Add_PA2VARange(
                _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr,
                (MS_U32)_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                TRUE);

            _stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr =
                (void *)MsOS_MPool_PA2KSEG1(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr);
            _stDumpParams.bMappingBS[VdecChn] = TRUE;
        }

        ///enale mma framebuffer memory to msos mmap
        if (!_stDumpParams.bMappingFB[VdecChn])
        {
            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr = mi_sys_Vmap(
                (MS_U32)_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                FALSE);

            MsOS_MPool_Add_PA2VARange(
                _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr,
                (MS_U32)_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                TRUE);

            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr =
                (void *)MsOS_MPool_PA2KSEG1(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr);
            _stDumpParams.bMappingFB[VdecChn] = TRUE;
        }
    }
    else
    {
        ///unmmap
        if (_stDumpParams.bMappingBS[VdecChn])
        {
            MsOS_MPool_Remove_PA2VARange(
                _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr,
                (MS_U32)_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                TRUE);

            mi_sys_UnVmap(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr);
            _stDumpParams.bMappingBS[VdecChn] = FALSE;
        }

        if (_stDumpParams.bMappingFB[VdecChn])
        {
            MsOS_MPool_Remove_PA2VARange(
                _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr,
                (MS_U32)_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                TRUE);

            mi_sys_UnVmap(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr);
            _stDumpParams.bMappingFB[VdecChn] = FALSE;
        }
    }
}

static void _MI_VDEC_IMPL_HWMemCfg(MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstChnAttr)
{
    if(!pstChnAttr)
    {
        DBG_ERR("Vdec chn(%d) pstChnAttr is NULL\n", VdecChn);
    }

    /*
     * if enable MFDec, hw buffer need add
     * MFDec, default (k6&k6l-enable, i2:disable)
     */

    /* #######< K6LITE hw buffer config >#######
     * no matter mfdec enable or disable
     * h264 & jpeg: 1M
     * h265: 2M (if reso <= 720x576, buffer = 1M)
     */

    /* #######< K6 hw buffer config >#######
     *
     * mfdec disable
     * h264 & jpeg: 1M
     * h265: 2M (if reso <= 720x576, buffer = 1M)
     *
     * mfdec enable
     * h264:
     *      = 720p    hw buffer: 4.5M
     *    720p~1080p  hw buffer: 5M
     *      >  1080p  hw buffer: 12M
     * h265:
     *      = 720p    hw buffer: 5.5M
     *    720p~1080p  hw buffer: 6M
     *      >  1080p  hw buffer: 13M
     */

    /* #######< I2 hw buffer config >#######
     * no matter mfdec enable or disable
     * h264: 1M
     * h265: GetHwMemSize and set
     */

#if (defined CONFIG_MSTAR_CHIP_I2) && (CONFIG_MSTAR_CHIP_I2 == 1)
    if(E_MI_VDEC_CODEC_TYPE_JPEG == pstChnAttr->eCodecType)
    {
        _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec = FALSE;
    }
    else if(E_MI_VDEC_CODEC_TYPE_H264 == pstChnAttr->eCodecType)
    {
        _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec = TRUE;
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize += 0x100000;
    }
    else
    {
        VDEC_EX_Result eRet = E_VDEC_EX_FAIL;
        VDEC_EX_HW_MEM_CFG stHwMemCfg;

        memset(&stHwMemCfg, 0x0, sizeof(VDEC_EX_HW_MEM_CFG));
        stHwMemCfg.eCodecType = E_VDEC_EX_CODEC_TYPE_HEVC;
        stHwMemCfg.u16FrmNum = 3 + pstChnAttr->stVdecVideoAttr.u32RefFrameNum;
        stHwMemCfg.u16Width  = pstChnAttr->u32PicWidth;
        stHwMemCfg.u16Height = pstChnAttr->u32PicHeight;

        //enable MFDec
        stHwMemCfg.u16EnableMfcodec = 1;
        _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec = TRUE;

        eRet = MApi_VDEC_EX_GetHwMemSize(&stHwMemCfg);
        if(E_VDEC_EX_OK != eRet)
        {
            DBG_ERR("H265 MFDEC GetHwMemSize failed, VdecChn: %d\n", VdecChn);
        }
        //DBG_INFO("@@@@@@@@@@ u32HwMemSize: %x\n", stHwMemCfg.u32HwMemSize);
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize += stHwMemCfg.u32HwMemSize;
    }
#else
    ///hw memory 1M
    _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize += 0x100000;
    if (pstChnAttr->eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        ///H265, hw memory 1M or 2M
        if(pstChnAttr->u32PicWidth * pstChnAttr->u32PicHeight > 720 * 576)
        {
            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize += 0x100000;
        }
    }
    _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec = FALSE; //

    //if mfdec enable
    if(FALSE == _stDumpParams.u32DisableMFDec)
    {
    #if (defined CONFIG_MSTAR_CHIP_K6) && (CONFIG_MSTAR_CHIP_K6 == 1)
        _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec = TRUE;
        if(pstChnAttr->u32PicWidth * pstChnAttr->u32PicHeight > 1920 * 1080)
        {
            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize += 0xB00000;
        }
        else if(pstChnAttr->u32PicWidth * pstChnAttr->u32PicHeight > 1280 * 720)
        {
            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize += 0x400000;
        }
        else if(pstChnAttr->u32PicWidth * pstChnAttr->u32PicHeight == 1280 * 720)
        {
            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize += 0x380000;
        }
    #endif
    }
#endif
}

static MI_S32 _MI_VDEC_DisableMFDec(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **args, void *pUsrData)
{
    MI_S32 s32Ret = -EINVAL;

    if(argc < 2)
    {
        handle.OnPrintOut(handle, "_MI_VDEC_DisableMFDec error!!!, argc < 2\n");
        handle.OnPrintOut(handle, "Usge:\n");
        handle.OnPrintOut(handle, "DisableMFDec MFE_Enable \r\n");
    }

    kstrtou32(args[1], 0, &_stDumpParams.u32DisableMFDec);

    handle.OnPrintOut(handle, "Set %s MFDec Now\n", _stDumpParams.u32DisableMFDec ? "Disable" : "Enable");
    s32Ret = 0;

    return s32Ret;
}

static MI_S32 _MI_VDEC_DumpBS(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **args, void *pUsrData)
{
    MI_S32 s32Ret = 0;
    MI_U32 VdecChn = 0;

    if(argc < 3)
    {
        handle.OnPrintOut(handle, "DumpBS error!!!, argc < 3\n");
        handle.OnPrintOut(handle, "Usge:\n");
        handle.OnPrintOut(handle, "DumpBS dst_path channel_nums \r\n");
    }

    kstrtou32(args[2], 0, &VdecChn);
    if (VdecChn >= MI_VDEC_MAX_CHN_NUM)
    {
        handle.OnPrintOut(handle, "Dbg Chn(%d) Error\n", VdecChn);
        return -1;
    }

    if(!strncasecmp(args[1], "off", strlen("off")))
    {
        _stDumpParams.bDumpBS[VdecChn] = FALSE;
        handle.OnPrintOut(handle, "Open Dump Chn(%d) BS Buffer finished\n", VdecChn);
    }
    else
    {
        strcpy(_stDumpParams.au8DumpPath, args[1]);
        _stDumpParams.bDumpBS[VdecChn] = TRUE;
        handle.OnPrintOut(handle, "Open Dump Chn(%d) BS Buffer Done, Dump File Path:%s\n", VdecChn, _stDumpParams.au8DumpPath);
    }

    return s32Ret;
}


static MI_S32 _MI_VDEC_DumpFB(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **args, void *pUsrData)
{
    MI_S32 s32Ret = 0;
    MI_U32 VdecChn = 0;

    if(argc < 3)
    {
        handle.OnPrintOut(handle, "DumpFB error!!!, argc < 3\n");
        handle.OnPrintOut(handle, "Usge:\n");
        handle.OnPrintOut(handle, "DumpFB dst_path channel_nums \r\n");
    }

    kstrtou32(args[2], 0, &VdecChn);
    if (VdecChn >= MI_VDEC_MAX_CHN_NUM)
    {
        handle.OnPrintOut(handle, "Dbg Chn(%d) Error\n", VdecChn);
        return -1;
    }

    strcpy(_stDumpParams.au8DumpPath, args[1]);

    _stDumpParams.bDumpFB[VdecChn] = TRUE;
    handle.OnPrintOut(handle, "Open Dump Chn(%d) FB Buffer Done, Dump File Path:%s\n", VdecChn, _stDumpParams.au8DumpPath);

    return s32Ret;
}


static MI_S32 _MI_VDEC_DumpCurBS(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **args, void *pUsrData)
{
    MI_S32 s32Ret = 0;
    MI_U32 VdecChn = 0;

    if(argc < 3)
    {
        handle.OnPrintOut(handle, "DumpCurBS error!!!, argc < 2\n");
        handle.OnPrintOut(handle, "Usge:\n");
        printk("DumpCurBS channel_num \r\n");
    }

    kstrtou32(args[2], 0, &VdecChn);
    if (VdecChn >= MI_VDEC_MAX_CHN_NUM)
    {
        handle.OnPrintOut(handle, "Dbg Chn(%d) Error\n", VdecChn);
        return -1;
    }

    strcpy(_stDumpParams.au8DumpPath, args[1]);

    handle.OnPrintOut(handle, "Dump Chn(%d) ES Data Now\n", VdecChn);
    _MI_VDEC_IMPL_DebugDumpBitStreamBuffer(VdecChn);

    return s32Ret;
}

#if 0
static MI_S32 _MI_VDEC_SetDebugParams(const char **args, MI_S32 v)
{
    MI_S32 s32Ret = -EINVAL;

    MI_VDEC_ChkDumpParamsVarNum(1);
    if (0 == strcmp(args[0], "DisableMFDec"))
    {
        MI_VDEC_ChkDumpParamsVarNum(2);
        kstrtou32(args[1], 0, &_stDumpParams.u32DisableMFDec);

        DBG_WRN("Set %s MFDec Now\n", _stDumpParams.u32DisableMFDec ? "Disable" : "Enable");
        s32Ret = 0;
    }
    else
    if (0 == strcmp(args[0], "ChkFrmAddrLegal"))
    {
        _stDumpParams.bChkFrmAddrLegal = TRUE;
        DBG_WRN("Open Check Frame Address Legal Done\n");
        s32Ret = 0;
    }
    else
    if (0 == strcmp(args[0], "DumpBS"))
    {
        MI_U32 VdecChn = 0;
        MI_S32 i = 0;

        MI_VDEC_ChkDumpParamsVarNum(2);
        strcpy(_stDumpParams.au8DumpPath, args[1]);
        for (i = 2; i < MI_VDEC_MAX_CHN_NUM; ++i)
        {
            if (v < (i + 1))
            {
                break;
            }
            kstrtou32(args[i], 0, &VdecChn);
            if (VdecChn >= MI_VDEC_MAX_CHN_NUM)
            {
                DBG_ERR("Dbg Chn(%d) Error\n", VdecChn);
                return s32Ret;
            }
            _stDumpParams.bDumpBS[VdecChn] = TRUE;
            DBG_WRN("Open Dump Chn(%d) BS Buffer Done, Dump File Path:%s\n", VdecChn, _stDumpParams.au8DumpPath);
        }
        s32Ret = 0;
    }
    else
    if (0 == strcmp(args[0], "DumpFB"))
    {
        MI_U32 VdecChn = 0;
        MI_S32 i = 0;

        MI_VDEC_ChkDumpParamsVarNum(2);
        strcpy(_stDumpParams.au8DumpPath, args[1]);
        for (i = 2; i < MI_VDEC_MAX_CHN_NUM; ++i)
        {
            if (v < (i + 1))
            {
                break;
            }
            kstrtou32(args[i], 0, &VdecChn);
            if (VdecChn >= MI_VDEC_MAX_CHN_NUM)
            {
                DBG_ERR("Dbg Chn(%d) Error\n", VdecChn);
                return s32Ret;
            }
            _stDumpParams.bDumpFB[VdecChn] = TRUE;
            DBG_WRN("Open Dump Chn(%d) FB Buffer Done, Dump File Path:%s\n", VdecChn, _stDumpParams.au8DumpPath);
        }
        s32Ret = 0;
    }
    else
    if (0 == strcmp(args[0], "DumpCurBS"))
    {
        MI_U32 VdecChn = 0;
        MI_VDEC_ChkDumpParamsVarNum(2);
        kstrtou32(args[1], 0, &VdecChn);
        if (VdecChn >= MI_VDEC_MAX_CHN_NUM)
        {
            DBG_ERR("Dbg Chn(%d) Error\n", VdecChn);
            return s32Ret;
        }
        DBG_WRN("Dump Chn(%d) ES Data Now\n", VdecChn);
        _MI_VDEC_IMPL_DebugDumpBitStreamBuffer(VdecChn);
        s32Ret = 0;
    }
    else
    if (0 == strcmp(args[0], "DumpCurFB"))
    {
        MI_U32 VdecChn = 0;
        MI_VDEC_ChkDumpParamsVarNum(2);
        kstrtou32(args[1], 0, &VdecChn);
        if (VdecChn >= MI_VDEC_MAX_CHN_NUM)
        {
            DBG_ERR("Dbg Chn(%d) Error\n", VdecChn);
            return s32Ret;
        }
        DBG_WRN("Dump Chn(%d) FB Data Now\n", VdecChn);
        s32Ret = 0;
    }
    else
    if (0 == strcmp(args[0], "ChkRlsFrmStatus"))
    {
        MI_U32 u32ChkRlsFrmStatus = FALSE;
        MI_VDEC_ChkDumpParamsVarNum(2);
        kstrtou32(args[1], 0, &u32ChkRlsFrmStatus);
        _stDumpParams.bChkRlsFrmStatus = (MI_BOOL)u32ChkRlsFrmStatus;
        memset(_stDumpParams.au32GetFrmCnt, 0x0, sizeof(_stDumpParams.au32GetFrmCnt));
        memset(_stDumpParams.au32RlsFrmCnt, 0x0, sizeof(_stDumpParams.au32RlsFrmCnt));
        DBG_WRN("Open Check Release Frame Info Done\n");
        s32Ret = 0;
    }
    else
    if (0 == strcmp(args[0], "ChkMetaData"))
    {
        MI_U32 VdecChn = 0;
        MI_VDEC_ChkDumpParamsVarNum(2);
        kstrtou32(args[1], 0, &VdecChn);
        if (VdecChn >= MI_VDEC_MAX_CHN_NUM)
        {
            DBG_ERR("Dbg Chn(%d) Error\n", VdecChn);
            return s32Ret;
        }

        _stDumpParams.abChkMetaData[VdecChn] = TRUE;
        DBG_WRN("Open Chn(%d) Check Meta Data Sum Done\n", VdecChn);
        s32Ret = 0;
    }
    else
    if (0 == strcmp(args[0], "SetFwDbgLv"))
    {
        MI_VDEC_ChkDumpParamsVarNum(2);
        kstrtou32(args[1], 0, &_stDumpParams.u32FwDbgLv);
        DBG_WRN("Set Current Fw Debug Lv:%d\n", _stDumpParams.u32FwDbgLv);
        s32Ret = 0;
    }
    else
    if (0 == strcmp(args[0], "SetJpdDbgLv"))
    {
        MI_VDEC_ChkDumpParamsVarNum(2);
        kstrtou32(args[1], 0, &_stDumpParams.u32JpdDbgLv);
        DBG_WRN("Set Current JPG Debug Lv:%d\n", _stDumpParams.u32FwDbgLv);
        s32Ret = 0;
    }

    return s32Ret;
}
#endif

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
static MI_S32 _MI_VDEC_ProcOnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_ProcOnDumpChannelAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_VDEC_CHN VdecChn = 0;
    MI_VDEC_ChnAttr_t     *pstChnAttr   = NULL;
    MI_VDEC_ChnParam_t    *pstChnParam  = NULL;
    MI_VDEC_ChnStat_t      stChnStat;
    MI_U32 ErrCnt  = 0;
    MI_U32 DropCnt = 0;
    MI_U32 SkipCnt = 0;

    MI_U8 aCodecType[16] = "";
    MI_U8 aVideoMode[16] = "";
    MI_U8 aJpegFormat[16] = "";
    MI_U8 aRefFrameNum[16] = "";
    MI_U8 aDecMode[16] = "";
    MI_U8 aOutputOrder[16] = "";
    MI_U8 aVideoFormat[16] = "";
    MI_U8 aDisplayMode[16] = "";
    MI_U8 aEnMfcodec[16] = "";
    MI_U8 aDisableDblk[16] = "";
    MI_U8 abChnStart[16] = "";

    memset(&stChnStat, 0x0, sizeof(MI_VDEC_ChnStat_t));

    handle.OnPrintOut(handle, "\n==================================Private Vdec%u Info====================================\n", u32DevId);
    handle.OnPrintOut(handle, "------------------------------------CHN ATTR Info---------------------------------------\n");
    handle.OnPrintOut(handle, "%5s  %9s  %5s  %6s  %8s  %8s  %9s  %10s  %9s\n",\
        "ChnID","CodecType","Width","Height","BufSize","Priority","VideoMode","JpegFormat","RefFrmNum");

    for(VdecChn = 0; VdecChn < MI_VDEC_MAX_CHN_NUM; VdecChn++)
    {
        if(TRUE != _stResMgr.astChnInfo[VdecChn].bCreate)
        {
            continue;
        }
        pstChnAttr = &(_stResMgr.astChnInfo[VdecChn].stChnAttr);

        switch(pstChnAttr->eCodecType)
        {
            case E_MI_VDEC_CODEC_TYPE_H264:
                strncpy(aCodecType, "H264", sizeof("H264"));
                break;
            case E_MI_VDEC_CODEC_TYPE_H265:
                strncpy(aCodecType, "H265", sizeof("H265"));
                break;
            case E_MI_VDEC_CODEC_TYPE_JPEG:
                strncpy(aCodecType, "JPEG", sizeof("JPEG"));
                break;
            default:
                snprintf(aCodecType, 15, "%d", pstChnAttr->eCodecType);
        }

        if(E_MI_VDEC_VIDEO_MODE_STREAM == pstChnAttr->eVideoMode)
        {
            strncpy(aVideoMode, "STREAM", sizeof("STREAM"));
        }
        else if(E_MI_VDEC_VIDEO_MODE_FRAME == pstChnAttr->eVideoMode)
        {
            strncpy(aVideoMode, "FRAME", sizeof("FRAME"));
        }
        else
        {
            snprintf(aVideoMode, 15, "%d", pstChnAttr->eVideoMode);
        }

        if(E_MI_VDEC_CODEC_TYPE_JPEG == pstChnAttr->eCodecType)
        {
            switch(pstChnAttr->stVdecJpegAttr.eJpegFormat)
            {
                case E_MI_VDEC_JPEG_FORMAT_YCBCR400:
                    strncpy(aJpegFormat, "YCbCr400", sizeof("YCbCr400"));
                    break;
                case E_MI_VDEC_JPEG_FORMAT_YCBCR420:
                    strncpy(aJpegFormat, "YCbCr420", sizeof("YCbCr420"));
                    break;
                case E_MI_VDEC_JPEG_FORMAT_YCBCR422:
                    strncpy(aJpegFormat, "YCbCr422", sizeof("YCbCr422"));
                    break;
                case E_MI_VDEC_JPEG_FORMAT_YCBCR444:
                    strncpy(aJpegFormat, "YCbCr444", sizeof("YCbCr444"));
                    break;
                default:
                    snprintf(aJpegFormat, 15, "%d", pstChnAttr->stVdecJpegAttr.eJpegFormat);
            }
            strncpy(aRefFrameNum, "NA", sizeof("NA"));
        }
        else
        {
            strncpy(aJpegFormat, "NA", sizeof("NA"));
            snprintf(aRefFrameNum, 15, "%d", pstChnAttr->stVdecVideoAttr.u32RefFrameNum);
        }

        handle.OnPrintOut(handle, "%5u  %9s  %5u  %6u  %8u  %8u  %9s  %10s  %9s\n",
            VdecChn, aCodecType, pstChnAttr->u32PicWidth, pstChnAttr->u32PicHeight, pstChnAttr->u32BufSize,
            pstChnAttr->u32Priority, aVideoMode, aJpegFormat, aRefFrameNum);
    }


    handle.OnPrintOut(handle, "\n------------------------------------CHN PARAM Info--------------------------------------\n");
    handle.OnPrintOut(handle, "%5s  %7s  %11s  %11s  %11s  %13s  %17s\n", \
        "ChnID","DecMode","OutputOrder","VideoFormat","DisplayMode","EnableMfcodec","DisableDeblocking");

    for(VdecChn = 0; VdecChn < MI_VDEC_MAX_CHN_NUM; VdecChn++)
    {
        if(TRUE != _stResMgr.astChnInfo[VdecChn].bCreate)
        {
            continue;
        }
        pstChnParam = &(_stResMgr.astChnInfo[VdecChn].stChnParam);
        if(E_MI_VDEC_CODEC_TYPE_JPEG != pstChnAttr->eCodecType)
        {
            switch(pstChnParam->eDecMode)
            {
                case E_MI_VDEC_DECODE_MODE_ALL:
                    strncpy(aDecMode, "ALL", sizeof("ALL"));
                    break;
                case E_MI_VDEC_DECODE_MODE_I:
                    strncpy(aDecMode, "I", sizeof("I"));
                    break;
                case E_MI_VDEC_DECODE_MODE_IP:
                    strncpy(aDecMode, "IP", sizeof("IP"));
                    break;
                default:
                    snprintf(aDecMode, 15, "%d", pstChnParam->eDecMode);
            }
        }
        else
        {
            strncpy(aDecMode, "NA", sizeof("NA"));
        }

        if(E_MI_VDEC_CODEC_TYPE_JPEG != pstChnAttr->eCodecType)
        {
            if(E_MI_VDEC_OUTPUT_ORDER_DISPLAY == pstChnParam->eOutputOrder)
            {
                strncpy(aOutputOrder, "DISPLAY", sizeof("DISPLAY"));
            }
            else if(E_MI_VDEC_OUTPUT_ORDER_DECODE == pstChnParam->eOutputOrder)
            {
                strncpy(aOutputOrder, "DECODE", sizeof("DECODE"));
            }
            else
            {
                snprintf(aOutputOrder, 15, "%d", pstChnParam->eOutputOrder);
            }
        }
        else
        {
            strncpy(aOutputOrder, "NA", sizeof("NA"));
        }

        if(E_MI_VDEC_CODEC_TYPE_JPEG != pstChnAttr->eCodecType)
        {
            if(E_MI_VDEC_VIDEO_FORMAT_TILE == pstChnParam->eVideoFormat)
            {
                strncpy(aVideoFormat, "TILE", sizeof("TILE"));
            }
            else if(E_MI_VDEC_VIDEO_FORMAT_REDUCE == pstChnParam->eVideoFormat)
            {
                strncpy(aVideoFormat, "REDUCE", sizeof("REDUCE"));
            }
            else
            {
                snprintf(aVideoFormat, 15, "%d", pstChnParam->eVideoFormat);
            }
        }
        else
        {
            strncpy(aVideoFormat, "NA", sizeof("NA"));
        }

        if(E_MI_VDEC_DISPLAY_MODE_PREVIEW == _stResMgr.astChnInfo[VdecChn].eDisplayMode)
        {
            strncpy(aDisplayMode, "PREVIEW", sizeof("PREVIEW"));
        }
        else if(E_MI_VDEC_DISPLAY_MODE_PLAYBACK == _stResMgr.astChnInfo[VdecChn].eDisplayMode)
        {
            strncpy(aDisplayMode, "PLAYBACK", sizeof("PLAYBACK"));
        }
        else
        {
            snprintf(aDisplayMode, 15, "%d", _stResMgr.astChnInfo[VdecChn].eDisplayMode);
        }

        if(E_MI_VDEC_CODEC_TYPE_JPEG != pstChnAttr->eCodecType)
        {
            if(1 == _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec)
            {
                strncpy(aEnMfcodec, "Y", sizeof("Y"));
            }
            else if(0 == _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec)
            {
                strncpy(aEnMfcodec, "N", sizeof("N"));
            }
            else
            {
                snprintf(aEnMfcodec, 15, "%d", _stResMgr.astChnInfo[VdecChn].u8EnableMfcodec);
            }
        }
        else
        {
            strncpy(aEnMfcodec, "NA", sizeof("NA"));
        }

        if(1 == _stResMgr.astChnInfo[VdecChn].u8DisableDeblocking)
        {
            strncpy(aDisableDblk, "Y", sizeof("Y"));
        }
        else if(0 == _stResMgr.astChnInfo[VdecChn].u8DisableDeblocking)
        {
            strncpy(aDisableDblk, "N", sizeof("N"));
        }
        else
        {
            snprintf(aDisableDblk, 15, "%d", _stResMgr.astChnInfo[VdecChn].u8DisableDeblocking);
        }

        handle.OnPrintOut(handle, "%5u  %7s  %11s  %11s  %11s  %13s  %17s\n",\
            VdecChn, aDecMode, aOutputOrder, aVideoFormat, aDisplayMode, aEnMfcodec, aDisableDblk);
    }

    handle.OnPrintOut(handle, "\n------------------------------------CHN STATE-------------------------------------------\n");
    handle.OnPrintOut(handle, "%5s  %6s  %9s  %13s  %11s  %10s  %11s  %10s\n",\
        "ChnID","bStart","CodecType","LeftStrmBytes","LeftStrmFrm","LeftPic","RecvStrmFrm","DecStrmFrm");

    for(VdecChn = 0; VdecChn < MI_VDEC_MAX_CHN_NUM; VdecChn++)
    {
        if(TRUE != _stResMgr.astChnInfo[VdecChn].bCreate)
        {
            continue;
        }

        DOWN(&(_stVdecProcInfo.stProcChnLock[VdecChn]));
        if(TRUE == _stResMgr.astChnInfo[VdecChn].bStart)
        {
            if(MI_SUCCESS != MI_VDEC_IMPL_GetChnStat(VdecChn, &stChnStat))
            {
                //DBG_ERR("VdecChn(%u) Get Chn Stat failed\n", u32ChnId);
                // if getChnStat failed, then memset
                memset(&stChnStat, 0x0, sizeof(MI_VDEC_ChnStat_t));
            }
        }
        UP(&(_stVdecProcInfo.stProcChnLock[VdecChn]));

        if(1 == stChnStat.bChnStart)
        {
            strncpy(abChnStart, "Y", sizeof("Y"));
        }
        else if(0 == stChnStat.bChnStart)
        {
            strncpy(abChnStart, "N", sizeof("N"));
        }
        else
        {
            snprintf(abChnStart, 15, "%d", stChnStat.bChnStart);
        }

        switch(stChnStat.eCodecType)
        {
            case E_MI_VDEC_CODEC_TYPE_H264:
                strncpy(aCodecType, "H264", sizeof("H264"));
                break;
            case E_MI_VDEC_CODEC_TYPE_H265:
                strncpy(aCodecType, "H265", sizeof("H265"));
                break;
            case E_MI_VDEC_CODEC_TYPE_JPEG:
                strncpy(aCodecType, "JPEG", sizeof("JPEG"));
                break;
            default:
                snprintf(aCodecType, 15, "%d", stChnStat.eCodecType);
        }

        handle.OnPrintOut(handle, "%5u  %6s  %9s  %13u  %11u  %10u  %11u  %10u\n",\
            VdecChn, abChnStart, aCodecType, stChnStat.u32LeftStreamBytes, \
            stChnStat.u32LeftStreamFrames, stChnStat.u32LeftPics, stChnStat.u32RecvStreamFrames, stChnStat.u32DecodeStreamFrames);
    }

    handle.OnPrintOut(handle, "\n%5s  %10s  %10s  %10s\n", "ChnID","ErrCnt","DropCnt","SkipCnt");
    for(VdecChn = 0; VdecChn < MI_VDEC_MAX_CHN_NUM; VdecChn++)
    {
        if(TRUE != _stResMgr.astChnInfo[VdecChn].bCreate)
        {
            continue;
        }

        ErrCnt = MApi_VDEC_EX_GetErrCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        DropCnt = MApi_VDEC_EX_GetDropCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        SkipCnt = MApi_VDEC_EX_GetSkipCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));

        handle.OnPrintOut(handle, "%5u  %10u  %10u  %10u\n", VdecChn, ErrCnt, DropCnt, SkipCnt);
    }

    handle.OnPrintOut(handle, "\n------------------------------------CHN STATE2------------------------------------------\n");
    handle.OnPrintOut(handle, "%5s  %7s\n", "ChnID","FrmRate");
    for(VdecChn = 0; VdecChn < MI_VDEC_MAX_CHN_NUM; VdecChn++)
    {
        if(TRUE != _stResMgr.astChnInfo[VdecChn].bCreate)
        {
            continue;
        }

        handle.OnPrintOut(handle, "%5u  %7u\n", VdecChn, _stVdecProcInfo.stChnOutputInfo[VdecChn].u16FrmRate);
    }

	return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_ProcOnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    ///TODO:
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_ProcOnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    ///TODO:
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_ProcOnHelp(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    ///TODO:
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_DebugSetDecMode(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    char *ps8After = NULL;
    MI_VDEC_CHN VdecChn = 0;
    MI_VDEC_DecodeMode_e eDecMode = E_MI_VDEC_DECODE_MODE_MAX;

    if(3 > argc)
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    VdecChn = (MI_VDEC_CHN)simple_strtoul(argv[1], &ps8After, 0);
    // check input param
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);
    MI_VDEC_UnSupportOperation(VdecChn);

    eDecMode = (MI_VDEC_DecodeMode_e)simple_strtoul(argv[2], &ps8After, 0);
    if(E_MI_VDEC_DECODE_MODE_MAX <= eDecMode)
    {
        handle.OnPrintOut(handle, "eDecMode[%d] Set error\n", eDecMode);
        return E_MI_ERR_FAILED;
    }
    if (eDecMode != _stResMgr.astChnInfo[VdecChn].stChnParam.eDecMode)
    {
        VDEC_EX_TrickDec eTrickDec = E_VDEC_EX_TRICK_DEC_ALL;
        if (eDecMode == E_MI_VDEC_DECODE_MODE_ALL)
        {
            eTrickDec = E_VDEC_EX_TRICK_DEC_ALL;
        }
        else if (eDecMode == E_MI_VDEC_DECODE_MODE_I)
        {
            eTrickDec = E_VDEC_EX_TRICK_DEC_I;
        }
        else if (eDecMode == E_MI_VDEC_DECODE_MODE_IP)
        {
            eTrickDec = E_VDEC_EX_TRICK_DEC_IP;
        }

        MApi_VDEC_EX_SetTrickMode(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), eTrickDec);
        _stResMgr.astChnInfo[VdecChn].stChnParam.eDecMode = eDecMode;
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_DebugSetOutputOrder(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    char *ps8After = NULL;
    MI_VDEC_CHN VdecChn = 0;
    MI_VDEC_OutputOrder_e eOutputOrder = E_MI_VDEC_OUTPUT_ORDER_MAX;

    if(3 > argc)
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    VdecChn = (MI_VDEC_CHN)simple_strtoul(argv[1], &ps8After, 0);
    // check input param
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);
    MI_VDEC_UnSupportOperation(VdecChn);

    eOutputOrder = (MI_VDEC_OutputOrder_e)simple_strtoul(argv[2], &ps8After, 0);
    if(E_MI_VDEC_OUTPUT_ORDER_MAX <= eOutputOrder)
    {
        handle.OnPrintOut(handle, "eOutputOrder[%d] Set error\n", eOutputOrder);
        return E_MI_ERR_FAILED;
    }
    if (eOutputOrder != _stResMgr.astChnInfo[VdecChn].stChnParam.eOutputOrder)
    {
        MI_BOOL bDecOrder = TRUE;
        if (eOutputOrder == E_MI_VDEC_OUTPUT_ORDER_DISPLAY)
        {
            bDecOrder = FALSE;
        }

        MApi_VDEC_EX_SetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER, bDecOrder);
        _stResMgr.astChnInfo[VdecChn].stChnParam.eOutputOrder = eOutputOrder;
    }

	return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_DebugSetVideoFormat(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    // TODO:
    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_DebugSetDisplayMode(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    char *ps8After = NULL;
    MI_VDEC_CHN VdecChn = 0;
    MI_VDEC_DisplayMode_e eDisplayMode = E_MI_VDEC_DISPLAY_MODE_MAX;

    if(3 > argc)
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    VdecChn = (MI_VDEC_CHN)simple_strtoul(argv[1], &ps8After, 0);
    // check input param
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);

    eDisplayMode = (MI_VDEC_DisplayMode_e)simple_strtoul(argv[2], &ps8After, 0);
    if(E_MI_VDEC_DISPLAY_MODE_MAX <= eDisplayMode)
    {
        handle.OnPrintOut(handle, "eDisplayMode[%d] Set error\n", eDisplayMode);
        return E_MI_ERR_FAILED;
    }
    _stResMgr.astChnInfo[VdecChn].eDisplayMode = eDisplayMode;

    return MI_SUCCESS;
}

void _MI_VDEC_StatInputTimeIntvl(MI_VDEC_CHN VdecChn, mi_vdec_BufInfo_t *pstBufInfo)
{
    struct timespec sttime;

    memset(&sttime, 0, sizeof(sttime));

    if(_stInFrmPts[VdecChn].bChkFrmPts)
    {
        _stInFrmPts[VdecChn].u64Pts = pstBufInfo->u64Pts;
        _stInFrmPts[VdecChn].u32TotalFrmCnt += 1;

        PRINTF_PROC("Input ChnID: %d, TotalFrmCnt: %u, PTS: %llu\n",\
            VdecChn, _stInFrmPts[VdecChn].u32TotalFrmCnt, _stInFrmPts[VdecChn].u64Pts);
    }

    if(_stStatPts.stInPtsIntvl[VdecChn].bStatTime)
    {
        _stStatPts.stInPtsIntvl[VdecChn].u64PreTime = _stStatPts.stInPtsIntvl[VdecChn].u64CurTime;

        do_posix_clock_monotonic_gettime(&sttime);
        _stStatPts.stInPtsIntvl[VdecChn].u64CurTime = ((MI_U64)sttime.tv_sec) * 1000000ULL + div64_u64(sttime.tv_nsec, 1000);

        if(0 != _stStatPts.stInPtsIntvl[VdecChn].u64PreTime)
        {
            _stStatPts.stInPtsIntvl[VdecChn].u64MinTimeIntvl = \
                MIN(_stStatPts.stInPtsIntvl[VdecChn].u64MinTimeIntvl, _stStatPts.stInPtsIntvl[VdecChn].u64CurTime - _stStatPts.stInPtsIntvl[VdecChn].u64PreTime);
            _stStatPts.stInPtsIntvl[VdecChn].u64MaxTimeIntvl = \
                MAX(_stStatPts.stInPtsIntvl[VdecChn].u64MaxTimeIntvl, _stStatPts.stInPtsIntvl[VdecChn].u64CurTime - _stStatPts.stInPtsIntvl[VdecChn].u64PreTime);

            _stStatPts.stInPtsIntvl[VdecChn].u64SumTimeIntvl += _stStatPts.stInPtsIntvl[VdecChn].u64CurTime - _stStatPts.stInPtsIntvl[VdecChn].u64PreTime;
            _stStatPts.stInPtsIntvl[VdecChn].u32TotalFrmCnt += 1;

            _stStatPts.stInPtsIntvl[VdecChn].u64AvgTimeIntvl = \
            div64_u64(_stStatPts.stInPtsIntvl[VdecChn].u64SumTimeIntvl, _stStatPts.stInPtsIntvl[VdecChn].u32TotalFrmCnt);
        }
    }
}

void _MI_VDEC_StatOutputInfo(MI_VDEC_CHN VdecChn, MI_SYS_BufInfo_t *pstBufInfo)
{
    struct timespec sttime;

    memset(&sttime, 0, sizeof(sttime));

    if(_stOutFrmPts[VdecChn].bChkFrmPts)
    {
        _stOutFrmPts[VdecChn].u64Pts = pstBufInfo->u64Pts;
        _stOutFrmPts[VdecChn].u32TotalFrmCnt += 1;

        PRINTF_PROC("Output ChnID: %d, TotalFrmCnt: %u, PTS: %llu\n",\
            VdecChn, _stOutFrmPts[VdecChn].u32TotalFrmCnt, _stOutFrmPts[VdecChn].u64Pts);
    }

    // stat frmrate
    _stVdecProcInfo.stChnOutputInfo[VdecChn].u16DecCnt += 1;

    if(_stStatPts.stOutPtsIntvl[VdecChn].bStatTime)
    {
        _stStatPts.stOutPtsIntvl[VdecChn].u64PreTime = _stStatPts.stOutPtsIntvl[VdecChn].u64CurTime;

        do_posix_clock_monotonic_gettime(&sttime);
        _stStatPts.stOutPtsIntvl[VdecChn].u64CurTime = ((MI_U64)sttime.tv_sec) * 1000000ULL + div64_u64(sttime.tv_nsec, 1000);

        if(0 != _stStatPts.stOutPtsIntvl[VdecChn].u64PreTime)
        {
            _stStatPts.stOutPtsIntvl[VdecChn].u64MinTimeIntvl = \
                MIN(_stStatPts.stOutPtsIntvl[VdecChn].u64MinTimeIntvl, _stStatPts.stOutPtsIntvl[VdecChn].u64CurTime - _stStatPts.stOutPtsIntvl[VdecChn].u64PreTime);
            _stStatPts.stOutPtsIntvl[VdecChn].u64MaxTimeIntvl = \
                MAX(_stStatPts.stOutPtsIntvl[VdecChn].u64MaxTimeIntvl, _stStatPts.stOutPtsIntvl[VdecChn].u64CurTime - _stStatPts.stOutPtsIntvl[VdecChn].u64PreTime);

            _stStatPts.stOutPtsIntvl[VdecChn].u64SumTimeIntvl += _stStatPts.stOutPtsIntvl[VdecChn].u64CurTime - _stStatPts.stOutPtsIntvl[VdecChn].u64PreTime;
            _stStatPts.stOutPtsIntvl[VdecChn].u32TotalFrmCnt += 1;

            _stStatPts.stOutPtsIntvl[VdecChn].u64AvgTimeIntvl = \
            div64_u64(_stStatPts.stOutPtsIntvl[VdecChn].u64SumTimeIntvl, _stStatPts.stOutPtsIntvl[VdecChn].u32TotalFrmCnt);
        }
    }

}

void _MI_VDEC_PrintInputTimeIntvl(unsigned long arg)
{
    MI_VDEC_CHN VdecChn = 0;

    for(VdecChn = 0; VdecChn < MI_VDEC_MAX_CHN_NUM; VdecChn ++)
    {
        if(_stResMgr.astChnInfo[VdecChn].bStart && _stStatPts.stInPtsIntvl[VdecChn].bStatTime)
        {
            PRINTF_PROC("Input ChnID: %d, CurTime: %llu, TotalFrmCnt: %u, AvgTimeIntvl: %llu, MaxTimeIntvl: %llu, MinTimeIntvl: %llu\n", \
                VdecChn, _stStatPts.stInPtsIntvl[VdecChn].u64CurTime, _stStatPts.stInPtsIntvl[VdecChn].u32TotalFrmCnt,\
                _stStatPts.stInPtsIntvl[VdecChn].u64AvgTimeIntvl, _stStatPts.stInPtsIntvl[VdecChn].u64MaxTimeIntvl, \
                _stStatPts.stInPtsIntvl[VdecChn].u64MinTimeIntvl);
        }

        _stStatPts.stInPtsIntvl[VdecChn].u64PreTime      = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u64CurTime      = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u32TotalFrmCnt  = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u64MaxTimeIntvl = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u64MinTimeIntvl = -1;
        _stStatPts.stInPtsIntvl[VdecChn].u64SumTimeIntvl = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u64AvgTimeIntvl = 0;
    }

    mod_timer(&_stStatInFrmTimer.Timer, jiffies+HZ);//set 1s timeout
}

void _MI_VDEC_PrintOutputTimeIntvl(unsigned long arg)
{
    MI_VDEC_CHN VdecChn = 0;

    for(VdecChn = 0; VdecChn < MI_VDEC_MAX_CHN_NUM; VdecChn ++)
    {
        if(_stResMgr.astChnInfo[VdecChn].bStart && _stStatPts.stOutPtsIntvl[VdecChn].bStatTime)
        {
            PRINTF_PROC("Output ChnID: %d, CurTime: %llu, TotalFrmCnt: %u, AvgTimeIntvl: %llu, MaxTimeIntvl: %llu, MinTimeIntvl: %llu\n", \
                VdecChn, _stStatPts.stOutPtsIntvl[VdecChn].u64CurTime, _stStatPts.stOutPtsIntvl[VdecChn].u32TotalFrmCnt,\
                _stStatPts.stOutPtsIntvl[VdecChn].u64AvgTimeIntvl, _stStatPts.stOutPtsIntvl[VdecChn].u64MaxTimeIntvl, \
                _stStatPts.stOutPtsIntvl[VdecChn].u64MinTimeIntvl);
        }

        _stStatPts.stOutPtsIntvl[VdecChn].u64PreTime      = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u64CurTime      = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u32TotalFrmCnt  = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u64MaxTimeIntvl = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u64MinTimeIntvl = -1;
        _stStatPts.stOutPtsIntvl[VdecChn].u64SumTimeIntvl = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u64AvgTimeIntvl = 0;
    }

    mod_timer(&_stStatOutFrmTimer.Timer, jiffies+HZ);//set 1s timeout
}

static MI_S32 _MI_VDEC_DebugStatInputTimeIntvl(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    char *ps8After = NULL;
    MI_VDEC_CHN VdecChn = 0;

    if(3 > argc)
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    VdecChn = (MI_VDEC_CHN)simple_strtoul(argv[1], &ps8After, 0);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);

    if(!strncasecmp(argv[2], "on", strlen("on")))
    {
        _stStatPts.stInPtsIntvl[VdecChn].bStatTime	= TRUE;

        _stStatPts.stInPtsIntvl[VdecChn].u64PreTime      = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u64CurTime      = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u32TotalFrmCnt  = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u64MaxTimeIntvl = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u64MinTimeIntvl = -1;
        _stStatPts.stInPtsIntvl[VdecChn].u64SumTimeIntvl = 0;
        _stStatPts.stInPtsIntvl[VdecChn].u64AvgTimeIntvl = 0;

        _stStatPts.u16InChnCnt ++;
    }
    else if(!strncasecmp(argv[2], "off", strlen("off")))
    {
        _stStatPts.stInPtsIntvl[VdecChn].bStatTime = FALSE;

        _stStatPts.u16InChnCnt --;
        memset(&_stStatPts.stInPtsIntvl[VdecChn], 0, sizeof(MI_VDEC_StatPtsIntvl_t));
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    if(0 < _stStatPts.u16InChnCnt && FALSE == _stStatInFrmTimer.bCreate)
    {
        init_timer(&_stStatInFrmTimer.Timer);
        _stStatInFrmTimer.Timer.function = _MI_VDEC_PrintInputTimeIntvl;
        add_timer(&_stStatInFrmTimer.Timer);
        mod_timer(&_stStatInFrmTimer.Timer, jiffies+HZ);//set 1s timeout

        _stStatInFrmTimer.bCreate = TRUE;
    }

    if(0 == _stStatPts.u16InChnCnt && TRUE == _stStatInFrmTimer.bCreate)
    {
        del_timer(&_stStatInFrmTimer.Timer);
        _stStatInFrmTimer.bCreate = FALSE;
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_DebugStatOutputTimeIntvl(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    char *ps8After = NULL;
    MI_VDEC_CHN VdecChn = 0;

    if(3 > argc)
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    VdecChn = (MI_VDEC_CHN)simple_strtoul(argv[1], &ps8After, 0);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);

    if(!strncasecmp(argv[2], "on", strlen("on")))
    {
        _stStatPts.stOutPtsIntvl[VdecChn].bStatTime	= TRUE;

        _stStatPts.stOutPtsIntvl[VdecChn].u64PreTime      = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u64CurTime      = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u32TotalFrmCnt  = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u64MaxTimeIntvl = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u64MinTimeIntvl = -1;
        _stStatPts.stOutPtsIntvl[VdecChn].u64SumTimeIntvl = 0;
        _stStatPts.stOutPtsIntvl[VdecChn].u64AvgTimeIntvl = 0;

        _stStatPts.u16OutChnCnt ++;
    }
    else if(!strncasecmp(argv[2], "off", strlen("off")))
    {
        _stStatPts.stOutPtsIntvl[VdecChn].bStatTime	= FALSE;

        _stStatPts.u16OutChnCnt --;

        memset(&_stStatPts.stOutPtsIntvl[VdecChn], 0, sizeof(MI_VDEC_StatPtsIntvl_t));
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    if(0 < _stStatPts.u16OutChnCnt && FALSE == _stStatOutFrmTimer.bCreate)
    {
        init_timer(&_stStatOutFrmTimer.Timer);
        _stStatOutFrmTimer.Timer.function = _MI_VDEC_PrintOutputTimeIntvl;
        add_timer(&_stStatOutFrmTimer.Timer);
        mod_timer(&_stStatOutFrmTimer.Timer, jiffies+HZ);//set 1s timeout

        _stStatOutFrmTimer.bCreate = TRUE;
    }

    if(0 == _stStatPts.u16OutChnCnt && TRUE == _stStatOutFrmTimer.bCreate)
    {
        del_timer(&_stStatOutFrmTimer.Timer);
        _stStatOutFrmTimer.bCreate = FALSE;
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_ChkInputFrmPts(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    char *ps8After = NULL;
    MI_VDEC_CHN VdecChn = 0;

    if(3 > argc)
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    VdecChn = (MI_VDEC_CHN)simple_strtoul(argv[1], &ps8After, 0);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);

    if(!strncasecmp(argv[2], "on", strlen("on")))
    {
        memset(&_stInFrmPts[VdecChn], 0, sizeof(MI_VDEC_ChkFrmPts_t));
        _stInFrmPts[VdecChn].bChkFrmPts = TRUE;
    }
    else if(!strncasecmp(argv[2], "off", strlen("off")))
    {
        _stInFrmPts[VdecChn].bChkFrmPts = FALSE;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_ChkOutputFrmPts(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    char *ps8After = NULL;
    MI_VDEC_CHN VdecChn = 0;

    if(3 > argc)
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    VdecChn = (MI_VDEC_CHN)simple_strtoul(argv[1], &ps8After, 0);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);

    if(!strncasecmp(argv[2], "on", strlen("on")))
    {
        memset(&_stOutFrmPts[VdecChn], 0, sizeof(MI_VDEC_ChkFrmPts_t));
        _stOutFrmPts[VdecChn].bChkFrmPts = TRUE;
    }
    else if(!strncasecmp(argv[2], "off", strlen("off")))
    {
        _stOutFrmPts[VdecChn].bChkFrmPts = FALSE;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_SetDbgLevel(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    char *ps8After = NULL;
    MI_VDEC_CHN VdecChn = 0;
    VDEC_EX_Result eRet = E_VDEC_EX_FAIL;
    VDEC_EX_DbgLevel eDbgLevel = E_VDEC_EX_DBG_LEVEL_NONE;

    if(2 > argc)
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    _stDumpParams.u32FwDbgLv = simple_strtoul(argv[1], &ps8After, 0);
    eDbgLevel = (VDEC_EX_DbgLevel)_stDumpParams.u32FwDbgLv;

    for (VdecChn = 0; VdecChn < MI_VDEC_MAX_CHN_NUM; ++VdecChn)
    {
        if (FALSE == _stResMgr.astChnInfo[VdecChn].bStart)
        {
            continue;
        }

        eRet = MApi_VDEC_EX_SetDbgLevel(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), eDbgLevel);
        if(E_VDEC_EX_OK != eRet)
        {
            DBG_ERR("Vdec chn(%d) SetDbgLevel failed!\n", VdecChn);
            return E_MI_ERR_FAILED;
        }
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VDEC_RlsFrmDirect(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    if(2 > argc)
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    if(!strncasecmp(argv[1], "on", strlen("on")))
    {
        _stVdecProcInfo.bRlsFrmDirect = TRUE;
    }
    else if(!strncasecmp(argv[1], "off", strlen("off")))
    {
        _stVdecProcInfo.bRlsFrmDirect = FALSE;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command: %s \n", argv[0]);
        return E_MI_ERR_FAILED;
    }

    return MI_SUCCESS;
}

#endif

void MI_VDEC_IMPL_Insmod(void)
{
    memset(&_stDumpParams, 0x0, sizeof(MI_VDEC_DumpParams_t));
    strcpy(_stDumpParams.au8DumpPath, "/mnt");
    _stDumpParams.u32FwDbgLv = E_VDEC_EX_DBG_LEVEL_ERR;
    _stDumpParams.bChkFrmAddrLegal = FALSE;

#if (defined CONFIG_MSTAR_CHIP_I2) && (CONFIG_MSTAR_CHIP_I2 == 1)
    _stDumpParams.u32DisableMFDec = FALSE;
#else // K6 & K6L disable mfdec
    _stDumpParams.u32DisableMFDec = TRUE;
#endif
    //MI_COMMON_AddDebugFile("DebugFile", _MI_VDEC_SetDebugParams, NULL);//change this into /proc/mi_modules/VDECX/
}

MI_S32 MI_VDEC_IMPL_Init(void)
{
    mi_sys_ModuleDevInfo_t stModDevInfo;
    mi_sys_ModuleDevBindOps_t stModDevBindOps;
    MI_VDEC_RegDevPriData_t stRegDevPriData;
    MI_VDEC_MemBufInfo_t stMemBufInfo;
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
#endif

    DBG_ENTER();
    if (_stResMgr.bInitFlag)
    {
        DBG_WRN("Already Init\n");
        return MI_SUCCESS;
    }

    _MI_VDEC_IMPL_InitVar();

    if (MI_SUCCESS != _MI_VDEC_IMPL_GetMmapInfo("E_MMAP_ID_VDEC_CPU", &_stResMgr.stCpuMem))
    {
        DBG_ERR("Can't Get Cpu Buffer\n");
        return E_MI_ERR_NOMEM;
    }
#if !NOSUPPORT_MsOS_MPool_Add_PA2VARange
    else
    {
        _stResMgr.stCpuMem.pVirAddr = mi_sys_Vmap(
            (MS_U32)_stResMgr.stCpuMem.phyAddr,
            _ALIGN_BYTE_(_stResMgr.stCpuMem.u32BufSize, MI_VDEC_PAGE_SIZE),
            FALSE);
        MsOS_MPool_Add_PA2VARange(
            _stResMgr.stCpuMem.phyAddr,
            (MS_U32)_stResMgr.stCpuMem.pVirAddr,
            _ALIGN_BYTE_(_stResMgr.stCpuMem.u32BufSize, MI_VDEC_PAGE_SIZE),
            TRUE);
    }
#endif
    MI_SYS_BUG_ON(_stResMgr.stCpuMem.u8Miu >= MI_VDEC_MAX_USED_MIU);
    memset(&stMemBufInfo, 0, sizeof(MI_VDEC_MemBufInfo_t));
    if (MI_SUCCESS == _MI_VDEC_IMPL_GetMmapInfo("E_LX_MEM", &stMemBufInfo))
    {
        if (_stResMgr.stCpuMem.u8Miu == stMemBufInfo.u8Miu)
        {
            DBG_INFO("Vdec Match miu bus %d\n", stMemBufInfo.u8Miu);
            goto GET_LX_FINISH;
        }
    }
    memset(&stMemBufInfo, 0, sizeof(MI_VDEC_MemBufInfo_t));
    if (MI_SUCCESS == _MI_VDEC_IMPL_GetMmapInfo("E_LX_MEM2", &stMemBufInfo))
    {
        if (_stResMgr.stCpuMem.u8Miu == stMemBufInfo.u8Miu)
        {
            DBG_INFO("Vdec Match miu bus %d\n", stMemBufInfo.u8Miu);
            goto GET_LX_FINISH;
        }
    }
    memset(&stMemBufInfo, 0, sizeof(MI_VDEC_MemBufInfo_t));
    if (MI_SUCCESS == _MI_VDEC_IMPL_GetMmapInfo("E_LX_MEM3", &stMemBufInfo))
    {
        if (_stResMgr.stCpuMem.u8Miu == stMemBufInfo.u8Miu)
        {
            DBG_INFO("Vdec Match miu bus %d\n", stMemBufInfo.u8Miu);
            goto GET_LX_FINISH;
        }
    }
    DBG_ERR("Can't Get Lx Buffer\n");
    return E_MI_ERR_NOMEM;

GET_LX_FINISH:
    memcpy(&_stResMgr.stLxMem, &stMemBufInfo, sizeof(MI_VDEC_MemBufInfo_t));

    _stResMgr.stDvXcShmMem.u32BufSize = 4*1024;
    if (MI_SUCCESS != mi_sys_MMA_Alloc(
        (mma_config_exist_max_offset_to_curr_lx_mem == FALSE)?(_au8MmaHeapName[_stResMgr.stCpuMem.u8Miu]):(_au8MmaHeapName_vdec[_stResMgr.stCpuMem.u8Miu]),
        _stResMgr.stDvXcShmMem.u32BufSize,
        &_stResMgr.stDvXcShmMem.phyAddr)
    )
    {
        DBG_ERR("Get DV XC SHM Buffer Error\n");
        return E_MI_ERR_NOMEM;
    }

    _stResMgr.stJpegInternalMem.u32BufSize = 400*1024;
    if (MI_SUCCESS != mi_sys_MMA_Alloc(
        (mma_config_exist_max_offset_to_curr_lx_mem == FALSE)?(_au8MmaHeapName[_stResMgr.stCpuMem.u8Miu]):(_au8MmaHeapName_vdec[_stResMgr.stCpuMem.u8Miu]),
        _stResMgr.stJpegInternalMem.u32BufSize,
        &_stResMgr.stJpegInternalMem.phyAddr)
    )
    {
        DBG_ERR("Get Jpeg Internal Mem Buffer Error\n");
        return E_MI_ERR_NOMEM;
    }
    else
    {
        _stResMgr.stJpegInternalMem.pVirAddr = mi_sys_Vmap(
            (MS_U32)_stResMgr.stJpegInternalMem.phyAddr,
            _ALIGN_BYTE_(_stResMgr.stJpegInternalMem.u32BufSize, MI_VDEC_PAGE_SIZE),
            FALSE);

        MsOS_MPool_Add_PA2VARange(
            _stResMgr.stJpegInternalMem.phyAddr,
            (MS_U32)_stResMgr.stJpegInternalMem.pVirAddr,
            _ALIGN_BYTE_(_stResMgr.stJpegInternalMem.u32BufSize, MI_VDEC_PAGE_SIZE),
            TRUE);
    }

    memset(&stModDevInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    stModDevInfo.eModuleId = E_MI_MODULE_ID_VDEC;
    stModDevInfo.u32DevId = 0x0;
    stModDevInfo.u32DevChnNum = MI_VDEC_MAX_CHN_NUM;
    stModDevInfo.u32InputPortNum = 1;
    stModDevInfo.u32OutputPortNum = 1;

    memset(&stModDevBindOps, 0x0, sizeof(mi_sys_ModuleDevBindOps_t));
    stModDevBindOps.OnOutputPortBufRelease = _MI_VDEC_IMPL_RleaseFrameCallBack;

    stRegDevPriData.u32Version = 1;
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
    pstModuleProcfsOps.OnDumpDevAttr = _MI_VDEC_ProcOnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = _MI_VDEC_ProcOnDumpChannelAttr;
    pstModuleProcfsOps.OnDumpInputPortAttr = _MI_VDEC_ProcOnDumpInputPortAttr;
    pstModuleProcfsOps.OnDumpOutPortAttr = _MI_VDEC_ProcOnDumpOutPortAttr;
    pstModuleProcfsOps.OnHelp = _MI_VDEC_ProcOnHelp;
#else
    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif
    _stResMgr.hVdecDev = mi_sys_RegisterDev(&stModDevInfo, &stModDevBindOps , &stRegDevPriData
#ifdef MI_SYS_PROC_FS_DEBUG
                                               , &pstModuleProcfsOps
                                               ,MI_COMMON_GetSelfDir
#endif
                                            );

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
    memset(&_stStatPts, 0, sizeof(MI_VDEC_StatPts_t));
    mi_sys_RegistCommand("setDecMode",          2, _MI_VDEC_DebugSetDecMode,            _stResMgr.hVdecDev);
    mi_sys_RegistCommand("setOutputOrder",      2, _MI_VDEC_DebugSetOutputOrder,        _stResMgr.hVdecDev);
    mi_sys_RegistCommand("setVideoFormat",      2, _MI_VDEC_DebugSetVideoFormat,        _stResMgr.hVdecDev);
    mi_sys_RegistCommand("setDisplayMode",      2, _MI_VDEC_DebugSetDisplayMode,        _stResMgr.hVdecDev);
    mi_sys_RegistCommand("statInputTimeIntvl",  2, _MI_VDEC_DebugStatInputTimeIntvl,    _stResMgr.hVdecDev);
    mi_sys_RegistCommand("statOutputTimeIntvl", 2, _MI_VDEC_DebugStatOutputTimeIntvl,   _stResMgr.hVdecDev);
    mi_sys_RegistCommand("ChkInputFrmPts",      2, _MI_VDEC_ChkInputFrmPts,             _stResMgr.hVdecDev);
    mi_sys_RegistCommand("ChkOutputFrmPts",     2, _MI_VDEC_ChkOutputFrmPts,            _stResMgr.hVdecDev);
    mi_sys_RegistCommand("SetDbgLevel",         1, _MI_VDEC_SetDbgLevel,                _stResMgr.hVdecDev);
    mi_sys_RegistCommand("rlsfrmdirect",        1, _MI_VDEC_RlsFrmDirect,               _stResMgr.hVdecDev);
#endif

    mi_sys_RegistCommand("DisableMFDec", 1, _MI_VDEC_DisableMFDec, _stResMgr.hVdecDev);
    mi_sys_RegistCommand("DumpBS", 2, _MI_VDEC_DumpBS, _stResMgr.hVdecDev);
    mi_sys_RegistCommand("DumpFB", 2, _MI_VDEC_DumpFB, _stResMgr.hVdecDev);
    mi_sys_RegistCommand("DumpCurBS", 2, _MI_VDEC_DumpCurBS, _stResMgr.hVdecDev);

    MI_SYS_BUG_ON(!_stResMgr.hVdecDev);
    _stResMgr.ptskInjTask = kthread_create(_MI_VDEC_IMPL_InjectTask, &_stResMgr, "_MI_VDEC_IMPL_InjectTask");
    _stResMgr.ptskPutFrmTask = kthread_create(_MI_VDEC_IMPL_PutFrmTask, &_stResMgr, "_MI_VDEC_IMPL_PutFrmTask");
    _stResMgr.ptskPutJpegFrmTask = kthread_create(_MI_VDEC_IMPL_PutJpegFrmTask, &_stResMgr, "_MI_VDEC_IMPL_PutJpegFrmTask");

    MI_SYS_BUG_ON(IS_ERR(_stResMgr.ptskInjTask));
    MI_SYS_BUG_ON(IS_ERR(_stResMgr.ptskPutFrmTask));
    MI_SYS_BUG_ON(IS_ERR(_stResMgr.ptskPutJpegFrmTask));

    _stResMgr.bInjTaskRun = TRUE;
    wake_up_process(_stResMgr.ptskInjTask);

    _stResMgr.bPutFrmTaskRun = TRUE;
    wake_up_process(_stResMgr.ptskPutFrmTask);

    _stResMgr.bPutFrmJpegTaskRun = TRUE;
    wake_up_process(_stResMgr.ptskPutJpegFrmTask);

   _stResMgr.bInitFlag = TRUE;
   DBG_EXIT_OK();
   return MI_SUCCESS;
}

void _MI_VDEC_IMPL_DestroyAllChn(void)
{
    MI_U8 i = 0;
    for (i = 0; i < MI_VDEC_MAX_CHN_NUM; ++i)
    {
        MI_VDEC_IMPL_DestroyChn(i);
    }
}

MI_S32 MI_VDEC_IMPL_DeInit(void)
{
    DBG_INFO("Enter Vdec DeInit\n");

    if (FALSE == _stResMgr.bInitFlag)
    {
        DBG_WRN("Already Exit\n");
        return MI_SUCCESS;
    }

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
    if(_stStatInFrmTimer.bCreate)
    {
        del_timer(&_stStatInFrmTimer.Timer);
        _stStatInFrmTimer.bCreate = FALSE;
    }

    if(_stStatOutFrmTimer.bCreate)
    {
        del_timer(&_stStatOutFrmTimer.Timer);
        _stStatOutFrmTimer.bCreate = FALSE;
    }
#endif

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

    if (_stResMgr.ptskPutJpegFrmTask)
    {
        _stResMgr.bPutFrmJpegTaskRun = FALSE;
        kthread_stop(_stResMgr.ptskPutJpegFrmTask);
        _stResMgr.ptskPutJpegFrmTask = NULL;
    }

    _MI_VDEC_IMPL_DestroyAllChn();
    mi_sys_UnRegisterDev(_stResMgr.hVdecDev);
    _MI_VDEC_IMPL_PutCpuBuffer(&_stResMgr.stCpuMem);
    mi_sys_MMA_Free(_stResMgr.stDvXcShmMem.phyAddr);

    MsOS_MPool_Remove_PA2VARange(
        _stResMgr.stJpegInternalMem.phyAddr,
        (MS_U32)_stResMgr.stJpegInternalMem.pVirAddr,
        _ALIGN_BYTE_(_stResMgr.stJpegInternalMem.u32BufSize, MI_VDEC_PAGE_SIZE),
        TRUE);
    mi_sys_UnVmap(_stResMgr.stJpegInternalMem.pVirAddr);
    mi_sys_MMA_Free(_stResMgr.stJpegInternalMem.phyAddr);
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
        return E_MI_ERR_ILLEGAL_PARAM;
    }

    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
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
    DOWN(&(_stResMgr.semExitChnLock[VdecChn]));
    if (_stResMgr.astChnInfo[VdecChn].bCreate)
    {
        DBG_WRN("Chn(%d) Already Create\n", VdecChn);
        UP(&(_stResMgr.semExitChnLock[VdecChn]));
        UP(&(_stResMgr.semChnLock[VdecChn]));
        return E_MI_ERR_EXIST;
    }

    if (pstChnAttr->eCodecType == E_MI_VDEC_CODEC_TYPE_H264)
    {
        eVdecCodecType = E_VDEC_EX_CODEC_TYPE_H264;
    }
    else if (pstChnAttr->eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        eVdecCodecType = E_VDEC_EX_CODEC_TYPE_HEVC;
    }
    else if (pstChnAttr->eCodecType == E_MI_VDEC_CODEC_TYPE_JPEG)
    {
        eVdecCodecType = E_VDEC_EX_CODEC_TYPE_MJPEG;
    }
    else
    {
        DBG_EXIT_ERR("UnSupport Codec Type:%d\n", pstChnAttr->eCodecType);
        UP(&(_stResMgr.semExitChnLock[VdecChn]));
        UP(&(_stResMgr.semChnLock[VdecChn]));
        return E_MI_ERR_ILLEGAL_PARAM;
    }

    /*********************************************************
       alloc cpu buffer
     *********************************************************/
    _stResMgr.astChnInfo[VdecChn].stCpuMemInfo = _stResMgr.stCpuMem;

    /*********************************************************
       alloc Es buffer
     *********************************************************/
    if ((pstChnAttr->u32PicWidth >= 1920) || (pstChnAttr->u32PicHeight >= 1080))
    {
        if (MI_VDEC_ES_MEM_SIZE > pstChnAttr->u32BufSize)
        {
            pstChnAttr->u32BufSize = MI_VDEC_ES_MEM_SIZE;
        }
    }
    else if ((pstChnAttr->u32PicWidth > 720) || (pstChnAttr->u32PicHeight > 576))
    {
        if ((MI_VDEC_ES_MEM_SIZE >> 1) > pstChnAttr->u32BufSize)
        {
            pstChnAttr->u32BufSize = (MI_VDEC_ES_MEM_SIZE >> 1);
        }
    }
    else
    {
        if ((MI_VDEC_ES_MEM_SIZE >> 2) > pstChnAttr->u32BufSize)
        {
            pstChnAttr->u32BufSize = (MI_VDEC_ES_MEM_SIZE >> 2);
        }
    }

    memset(&_stResMgr.astChnInfo[VdecChn].stEsMemInfo, 0x0,
        sizeof(_stResMgr.astChnInfo[VdecChn].stEsMemInfo));
    _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize = pstChnAttr->u32BufSize;

    //Jpeg Progressive Mode:
    //Bitstream Buffer Size = (width*height*2*3)(AC)+( width*height*2*3/64)(DC)+(64*1024+128)(EXIF)+(8*1024)+(max bitstream size)
    //Jpeg Baseline Mode:
    //Bitstream Buffer Size = 64*1024+128)(EXIF)+(8*1024)+(max bitstream size)+)+(drv proc buffer size)
    if (E_MI_VDEC_CODEC_TYPE_JPEG == pstChnAttr->eCodecType)
    {
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize =
            MI_VDEC_MAX_JPEG_BITSTREAM_BUFFER(
            _ALIGN_BYTE_(pstChnAttr->u32PicWidth, MI_VDEC_IMAGE_PITCH_ALIGN_SIZE),
            _ALIGN_BYTE_(pstChnAttr->u32PicHeight, MI_VDEC_IMAGE_PITCH_ALIGN_SIZE),
            pstChnAttr->u32BufSize);
    }

    if (_stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize % 4096)
    {
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize
            +=(4096 - (_stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize % 4096));
    }

    ///need set "mma_heap_vdec" to bootargs and miu must match cpu memory miu
    ///bitstream buffer miu == cpu buffer miu == frame buffer miu
    if (MI_SUCCESS != mi_sys_MMA_Alloc(
        (mma_config_exist_max_offset_to_curr_lx_mem == FALSE)?(_au8MmaHeapName[_stResMgr.stCpuMem.u8Miu]):(_au8MmaHeapName_vdec[_stResMgr.stCpuMem.u8Miu]),
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize,
        &_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr))
    {
        DBG_ERR("Get Bitstream Buffer Error\n");
        goto _Exit;
    }

#if NOSUPPORT_MsOS_MPool_Add_PA2VARange
    _stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr = mi_sys_Vmap(
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr,
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize,
        FALSE);
#else
    else
    {
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr = mi_sys_Vmap(
            _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr,
            _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize,
            FALSE);

        MsOS_MPool_Add_PA2VARange(
                _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr,
                (MS_U32)_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                TRUE);
     }
#endif

    if (!(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr))
    {
        DBG_ERR("Map Bitstream Buffer Error\n");
        goto _Exit;
    }

    /*********************************************************
       alloc frame buffer(frame buffer = 3 + refFrame buffer)
     *********************************************************/
    if (1 > pstChnAttr->stVdecVideoAttr.u32RefFrameNum)
    {
        pstChnAttr->stVdecVideoAttr.u32RefFrameNum = 1;
    }

    memset(&_stResMgr.astChnInfo[VdecChn].stFrameMemInfo, 0x0,
        sizeof(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo));
    _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize =
        ((_ALIGN_BYTE_(pstChnAttr->u32PicWidth, MI_VDEC_IMAGE_PITCH_ALIGN_SIZE)
        * _ALIGN_BYTE_(pstChnAttr->u32PicHeight, MI_VDEC_IMAGE_PITCH_ALIGN_SIZE) * 3) >> 1)
        * (3 + pstChnAttr->stVdecVideoAttr.u32RefFrameNum);

    ///Jpeg Frame Buffer Size = 8MB(collect buffer size)+(width*height*2*2)
    if (E_MI_VDEC_CODEC_TYPE_JPEG == pstChnAttr->eCodecType)
    {
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize =
            (_ALIGN_BYTE_(pstChnAttr->u32PicWidth, MI_VDEC_IMAGE_PITCH_ALIGN_SIZE)
            * _ALIGN_BYTE_(pstChnAttr->u32PicHeight, MI_VDEC_IMAGE_PITCH_ALIGN_SIZE) * 2 * 2) + 0x800000;

#if (defined CONFIG_MSTAR_CHIP_I2) && (CONFIG_MSTAR_CHIP_I2 == 1)
        if(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize < 0x400000UL*2 + 640UL * 480UL * 2 * 8UL)
        {
            //DBG_EXIT_ERR("JPEG Frame buf not enough, it should not be less than %u\n", 0x400000UL*2 + 640UL * 480UL * 2 * 8UL);
            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize = 0x400000UL*2 + 640UL * 480UL * 2 * 8UL;
        }
#endif
    }

    ///align with 0x20000 (h265 4K at least align with 128k)
    if (_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize % 0x20000)
    {
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize
            +=(0x20000 - (_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize % 0x20000));
    }

    /*********************************************************
        hw memory config
     *********************************************************/
    _MI_VDEC_IMPL_HWMemCfg(VdecChn, pstChnAttr);

#if (1)
    if(mi_sys_Get_Vdec_VBPool_UseStatus()) //vdec framebuffer usb vbpool
    {
        pVdecStBufInfo[VdecChn] = mi_sys_InnerAllocBufFromVbPool(
            _stResMgr.hVdecDev,
            VdecChn,
            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize,
            &_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr);

        if (NULL == pVdecStBufInfo[VdecChn])
        {
            DBG_ERR("Vdec VbPool Get Frame Buffer Error!!! size:%d\n", _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize);
            goto _Exit;
        }
    }
    else if (MI_SUCCESS != mi_sys_MMA_Alloc(
        (mma_config_exist_max_offset_to_curr_lx_mem == FALSE)?(_au8MmaHeapName[_stResMgr.stCpuMem.u8Miu]):(_au8MmaHeapName_vdec[_stResMgr.stCpuMem.u8Miu]),
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize,
        &_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr)
        )
    {
        DBG_ERR("Get Frame Buffer Error\n");
        goto _Exit;
    }

    if(pstChnAttr->eCodecType == E_MI_VDEC_CODEC_TYPE_JPEG)
    {
        if (!_stDumpParams.bMappingFB[VdecChn])
        {
            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr = mi_sys_Vmap(
                (MS_U32)_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                FALSE);

            MsOS_MPool_Add_PA2VARange(
                _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr,
                (MS_U32)_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                TRUE);

            _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr =
                (void *)MsOS_MPool_PA2KSEG1(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr);
            _stDumpParams.bMappingFB[VdecChn] = TRUE;
        }
    }

    //if (_stDumpParams.bDumpFB[VdecChn])
    //{
    //    _MI_VDEC_IMPL_MMABufMapping(VdecChn, TRUE);
    //}
#else
    if (MI_SUCCESS != _MI_VDEC_IMPL_GetMmapInfo("E_MMAP_ID_VDEC_FRAMEBUFFER", &_stResMgr.astChnInfo[VdecChn].stFrameMemInfo))
    {
        DBG_ERR("Can't Get FB Buffer\n");
        return E_MI_ERR_NOMEM;
    }

    _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize = (pstChnAttr->u32PicWidth * pstChnAttr->u32PicHeight * 3 / 2) * (3 + pstChnAttr->stVdecVideoAttr.u32RefFrameNum);
    ///ALIGN 1M
    if (_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize % 0x100000)
    {
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize
            +=(0x100000 - (_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize % 0x100000));
    }

    ///hw memory 1M
    _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize += 0x100000;
    if (pstChnAttr->eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        ///hw memory 2M
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize += 0x100000;
    }
    _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr = _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr + (_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize * VdecChn);
    _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr = (void *)MsOS_MPool_PA2KSEG1(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr);
#endif

    DBG_INFO("Chn(%d), Es Buffer(0x%llx, %p, 0x%x), FrameBuffer(0x%llx, %p, 0x%x)\n",
        VdecChn,
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr,
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr,
        _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize,
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr,
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr,
        _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize);

    ///save channel attribite
    memcpy(&_stResMgr.astChnInfo[VdecChn].stChnAttr, pstChnAttr, sizeof(MI_VDEC_ChnAttr_t));
    _stResMgr.astChnInfo[VdecChn].bCreate = TRUE;

    UP(&(_stResMgr.semExitChnLock[VdecChn]));
    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_EXIT_OK();
    return MI_SUCCESS;

_Exit:

    _MI_VDEC_IMPL_MMABufMapping(VdecChn, FALSE);

    if (_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr)
    {
        mi_sys_UnVmap(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr);
        mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr);
    }

    if(mi_sys_Get_Vdec_VBPool_UseStatus()) //vdec framebuffer usb vbpool
    {
        if(NULL != pVdecStBufInfo[VdecChn])
        {
            mi_sys_FreeInnerVbPool(pVdecStBufInfo[VdecChn]);
            pVdecStBufInfo[VdecChn] = NULL;
        }
    }
    else if (_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr)
    {
        mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr);
    }

    UP(&(_stResMgr.semExitChnLock[VdecChn]));
    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_ERR("Create Chn(%d) Error\n", VdecChn);
    return E_MI_ERR_NOMEM;
}

MI_S32 MI_VDEC_IMPL_DestroyChn(MI_VDEC_CHN VdecChn)
{
    DBG_INFO("Destroy Chn(%d)\n", VdecChn);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnCreate(VdecChn, E_MI_ERR_UNEXIST);

    DOWN(&(_stResMgr.semChnLock[VdecChn]));
    DOWN(&(_stResMgr.semExitChnLock[VdecChn]));
    if (_stResMgr.astChnInfo[VdecChn].bCreate)
    {
#if NOSUPPORT_MsOS_MPool_Add_PA2VARange
        if (_stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType != E_MI_VDEC_CODEC_TYPE_JPEG)
        {
            MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));

            if (_stDumpParams.bMappingFB[VdecChn])
            {
                MsOS_MPool_Remove_PA2VARange(
                    _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr,
                    (MS_U32)_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr,
                    _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                    TRUE);

                mi_sys_UnVmap(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr);
                _stDumpParams.bMappingFB[VdecChn] = FALSE;
            }

            if (_stDumpParams.bMappingBS[VdecChn])
            {
                MsOS_MPool_Remove_PA2VARange(
                    _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr,
                    (MS_U32)_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr,
                    _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                    TRUE);
                _stDumpParams.bMappingBS[VdecChn] = FALSE;
            }

            mi_sys_UnVmap(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr);
            mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr);
            if(mi_sys_Get_Vdec_VBPool_UseStatus())
            {
                if(NULL != pVdecStBufInfo[VdecChn])
                {
                    mi_sys_FreeInnerVbPool(pVdecStBufInfo[VdecChn]);
                    pVdecStBufInfo[VdecChn] = NULL;
                }
            }
            else
            {
                mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr);
            }
        }
        _stResMgr.astChnInfo[VdecChn].bCreate = FALSE;
#else
        MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
        if (_stDumpParams.bMappingFB[VdecChn])
        {
            MsOS_MPool_Remove_PA2VARange(
                _stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr,
                (MS_U32)_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr,
                _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
                TRUE);

            mi_sys_UnVmap(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.pVirAddr);

            _stDumpParams.bMappingFB[VdecChn] = FALSE;
        }

        if(mi_sys_Get_Vdec_VBPool_UseStatus())
        {
            if(NULL != pVdecStBufInfo[VdecChn])
            {
                mi_sys_FreeInnerVbPool(pVdecStBufInfo[VdecChn]);
                pVdecStBufInfo[VdecChn] = NULL;
            }
        }
        else
        {
            mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stFrameMemInfo.phyAddr);
        }

        MsOS_MPool_Remove_PA2VARange(
            _stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr,
            (MS_U32)_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr,
            _ALIGN_BYTE_(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize, MI_VDEC_PAGE_SIZE),
            TRUE);

        mi_sys_UnVmap(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.pVirAddr);
        mi_sys_MMA_Free(_stResMgr.astChnInfo[VdecChn].stEsMemInfo.phyAddr);
        _stDumpParams.bMappingBS[VdecChn] = FALSE;
        _stResMgr.astChnInfo[VdecChn].bCreate = FALSE;
#endif
    }

    memset(&_stResMgr.astChnInfo[VdecChn], 0x0, sizeof(MI_VDEC_ChnInfo_t));
    UP(&(_stResMgr.semExitChnLock[VdecChn]));
    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_GetChnAttr(MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstChnAttr)
{
    DBG_INFO("Get Chn(%d) Attr:%p\n", VdecChn, pstChnAttr);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnCreate(VdecChn, E_MI_ERR_UNEXIST);
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
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DBG_INFO("Start Chn(%d)\n", VdecChn);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnCreate(VdecChn, E_MI_ERR_UNEXIST);
    DOWN(&(_stResMgr.semChnLock[VdecChn]));
    DOWN(&(_stResMgr.semExitChnLock[VdecChn]));
    if (_stResMgr.astChnInfo[VdecChn].bStart)
    {
        ///already start
        UP(&(_stResMgr.semExitChnLock[VdecChn]));
        UP(&(_stResMgr.semChnLock[VdecChn]));
        DBG_WRN("Chn(%d) Already Start\n", VdecChn);
        return E_MI_ERR_EXIST;
    }

    s32Ret = _MI_VDEC_IMPL_StartChn(VdecChn);

    if (MI_SUCCESS != s32Ret)
    {
        ///already start
        UP(&(_stResMgr.semExitChnLock[VdecChn]));
        UP(&(_stResMgr.semChnLock[VdecChn]));
        DBG_WRN("Chn(%d) Start Faild\n", VdecChn);
        _stResMgr.astChnInfo[VdecChn].bStart = FALSE;
        return s32Ret;
    }

    mi_sys_EnableChannel(_stResMgr.hVdecDev, VdecChn);
    mi_sys_EnableOutputPort(_stResMgr.hVdecDev, VdecChn, 0);
    mi_sys_EnableInputPort(_stResMgr.hVdecDev, VdecChn, 0);
    atomic_inc(&(_stResMgr.u64FastChnId[VdecChn]));
    _stResMgr.astChnInfo[VdecChn].bStart = TRUE;
    UP(&(_stResMgr.semExitChnLock[VdecChn]));
    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_StopChn(MI_VDEC_CHN VdecChn)
{
    DBG_INFO("Stop Chn(%d)\n", VdecChn);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);

    DOWN(&(_stResMgr.semChnLock[VdecChn]));
    DOWN(&(_stResMgr.semExitChnLock[VdecChn]));
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
    DOWN(&(_stVdecProcInfo.stProcChnLock[VdecChn]));
#endif
    MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
    _stResMgr.astChnInfo[VdecChn].bStart = FALSE;
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VDEC_PROCFS_DEBUG == 1)
    UP(&(_stVdecProcInfo.stProcChnLock[VdecChn]));
#endif
    atomic_inc(&(_stResMgr.u64FastChnId[VdecChn]));
    mi_sys_DisableChannel(_stResMgr.hVdecDev, VdecChn);
    UP(&(_stResMgr.semExitChnLock[VdecChn]));
    UP(&(_stResMgr.semChnLock[VdecChn]));
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_GetChnStat(MI_VDEC_CHN VdecChn, MI_VDEC_ChnStat_t *pstChnStat)
{
    MI_U32 u32DecodeStreamFrames = 0;
    MI_U32 u32ErrCode = 0;
    MI_U32 u32LeftStreamBytes;
    MI_U32 u32LeftStreamFrames;

    DBG_INFO("Get Chn(%d) Stat:%p\n", VdecChn, pstChnStat);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnCreate(VdecChn, E_MI_ERR_UNEXIST);

    pstChnStat->bChnStart = _stResMgr.astChnInfo[VdecChn].bStart;
    pstChnStat->eCodecType = _stResMgr.astChnInfo[VdecChn].stChnAttr.eCodecType;

    u32LeftStreamBytes = MApi_VDEC_EX_GetESBuffVacancy(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), NULL);
    pstChnStat->u32LeftStreamBytes = _stResMgr.astChnInfo[VdecChn].stEsMemInfo.u32BufSize - u32LeftStreamBytes;

    u32LeftStreamFrames = MApi_VDEC_EX_GetDecQVacancy(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
    if(E_MI_VDEC_CODEC_TYPE_JPEG != pstChnStat->eCodecType)
    {
        pstChnStat->u32LeftStreamFrames = 254 - u32LeftStreamFrames;
    }
    else
    {
        pstChnStat->u32LeftStreamFrames = 1 - u32LeftStreamFrames;
    }

    pstChnStat->u32LeftPics = MApi_VDEC_EX_GetFrameCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId))
                - MApi_VDEC_EX_GetDispCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));
    ///get channel info from driver, frame count and error code
    u32DecodeStreamFrames = MApi_VDEC_EX_GetFrameCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId))
                                 + MApi_VDEC_EX_GetErrCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId))
                                 + MApi_VDEC_EX_GetDropCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId))
                                 + MApi_VDEC_EX_GetSkipCnt(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));

    pstChnStat->u32DecodeStreamFrames = u32DecodeStreamFrames;
    pstChnStat->u32RecvStreamFrames = u32DecodeStreamFrames + pstChnStat->u32LeftStreamFrames;
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

MI_S32 MI_VDEC_IMPL_ResetChn(MI_VDEC_CHN VdecChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    DBG_INFO("Reset Chn(%d)\n", VdecChn);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);
    DOWN(&(_stResMgr.semChnLock[VdecChn]));
    DOWN(&(_stResMgr.semExitChnLock[VdecChn]));
    mi_sys_DisableChannel(_stResMgr.hVdecDev, VdecChn);

    ///stop channel
    MApi_VDEC_EX_Exit(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId));

    ///start channel
    s32Ret = _MI_VDEC_IMPL_StartChn(VdecChn);

    if (MI_SUCCESS == s32Ret)
    {
        _stResMgr.astChnInfo[VdecChn].bStart = TRUE;
        mi_sys_EnableChannel(_stResMgr.hVdecDev, VdecChn);
        DBG_EXIT_OK();
    }
    else
    {
        _stResMgr.astChnInfo[VdecChn].bStart = FALSE;
        DBG_ERR("Restart Faild\n");
    }
    UP(&(_stResMgr.semExitChnLock[VdecChn]));
    UP(&(_stResMgr.semChnLock[VdecChn]));
    return s32Ret;
}

MI_S32 MI_VDEC_IMPL_SetChnParam(MI_VDEC_CHN VdecChn, MI_VDEC_ChnParam_t *pstChnParam)
{
    DBG_INFO("Set Chn(%d) Param:%p, eDecMode:%d, eOutputOrder:%d, eVideoFormat:%d:%d\n",
        VdecChn, pstChnParam,
        pstChnParam->eDecMode,
        pstChnParam->eOutputOrder,
        pstChnParam->eVideoFormat);

    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);
    MI_VDEC_UnSupportOperation(VdecChn);
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

    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);
    memcpy(pstChnParam, &_stResMgr.astChnInfo[VdecChn].stChnParam, sizeof(MI_VDEC_ChnParam_t));
    ///get tile
    if (_stResMgr.astChnInfo[VdecChn].u8EnableMfcodec == TRUE)
    {
        ///use mfcodec
        pstChnParam->eVideoFormat = E_MI_VDEC_VIDEO_FORMAT_REDUCE;
    }
    else if (_stResMgr.astChnInfo[VdecChn].u8EnableMfcodec == FALSE)
    {
        pstChnParam->eVideoFormat = E_MI_VDEC_VIDEO_FORMAT_TILE;
    }
    else
    {
        pstChnParam->eVideoFormat = E_MI_VDEC_VIDEO_FORMAT_MAX;
    }


    DBG_INFO("(Exit Ok) eDecMode:%d, eOutputOrder:%d, eVideoFormat:%d\n",
        pstChnParam->eDecMode,
        pstChnParam->eOutputOrder,
        pstChnParam->eVideoFormat);
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_GetUserData(MI_VDEC_CHN VdecChn, MI_VDEC_UserData_t *pstUserData, MI_S32 s32MilliSec)
{
    VDEC_EX_Frame_packing_SEI_EX stSEIData;
    MI_U8 *pu8Buf = NULL;

    DBG_INFO("Get Chn(%d) User Data, %p, timeout:%d\n", VdecChn, pstUserData, s32MilliSec);
    if (NULL == pstUserData->pu8Addr)
    {
        DBG_EXIT_ERR("pstUserData Is Null\n");
        return E_MI_ERR_ILLEGAL_PARAM;
    }

    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);
    MI_VDEC_UnSupportOperation(VdecChn);
    memset(&stSEIData, 0x0, sizeof(VDEC_EX_Frame_packing_SEI_EX));
    stSEIData.stVerCtl.u32version = 0;
    stSEIData.stVerCtl.u32size = sizeof(VDEC_EX_Frame_packing_SEI_EX);

    MApi_VDEC_EX_GetControl(&(_stResMgr.astChnInfo[VdecChn].stVDECStreamId), E_VDEC_EX_USER_CMD_GET_FPA_SEI_EX, (MS_U32 *)&stSEIData);

    if(0 == stSEIData.u32DataSize)
    {
        //MI_PRINT("u32DataBuff : %x\n", stSEIData.u32DataBuff);
        MI_PRINT("Not found User Data\n");
		return MI_SUCCESS;
    }

    if (stSEIData.u32DataSize > pstUserData->u32Len)
    {
        DBG_ERR("Out Of Buffer, Data Size:%ld, User Buffer Size:%d\n", stSEIData.u32DataSize, pstUserData->u32Len);
        return E_MI_ERR_ILLEGAL_PARAM;
    }

    pu8Buf = mi_sys_Vmap(stSEIData.u32DataBuff, stSEIData.u32DataSize, FALSE);
    if (pu8Buf)
    {
        memcpy(pstUserData->pu8Addr, (void *)stSEIData.u32DataBuff, stSEIData.u32DataSize);
        pstUserData->u32Len = stSEIData.u32DataSize;
        pstUserData->bValid = stSEIData.bValid;
        mi_sys_UnVmap(pu8Buf);
    }
    else
    {
        memset(pstUserData, 0x0, sizeof(MI_VDEC_UserData_t));
        DBG_ERR("Mmap Faild\n");
        return E_MI_ERR_NOMEM;
    }

    DBG_INFO("(Exit Ok) bValid:%d, u32Len:%d, Data[0]:0x%x\n", pstUserData->bValid, pstUserData->u32Len, pstUserData->pu8Addr[0]);
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_ReleaseUserData(MI_VDEC_CHN VdecChn, MI_VDEC_UserData_t *pstUserData)
{
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_UnSupportOperation(VdecChn);
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_SetDisplayMode(MI_VDEC_CHN VdecChn, MI_VDEC_DisplayMode_e eDisplayMode)
{
    DBG_INFO("Set Chn(%d) DisplayMode:%d\n", VdecChn, eDisplayMode);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);

    _stResMgr.astChnInfo[VdecChn].eDisplayMode = eDisplayMode;
    DBG_EXIT_OK();
    return MI_SUCCESS;
}

MI_S32 MI_VDEC_IMPL_GetDisplayMode(MI_VDEC_CHN VdecChn, MI_VDEC_DisplayMode_e *peDisplayMode)
{
    DBG_INFO("Get Chn(%d) peDisplayMode:%p\n", VdecChn, peDisplayMode);
    MI_VDEC_ChkChnIdx(VdecChn, E_MI_ERR_INVALID_CHNID);
    MI_VDEC_ChkChnStart(VdecChn, E_MI_ERR_UNEXIST);

    if (NULL == peDisplayMode)
    {
        DBG_EXIT_ERR("peDisplayMode Is Null\n");
        return E_MI_ERR_ILLEGAL_PARAM;
    }
    *peDisplayMode = _stResMgr.astChnInfo[VdecChn].eDisplayMode;
    DBG_INFO("(Exit Ok)Get Chn(%d) DisplayMode:%d\n", VdecChn, *peDisplayMode);
    return MI_SUCCESS;
}

void _MI_VDEC_IMPL_PoolMapping(void)
{
    if(!MsOS_Init())
    {
        DBG_ERR("%s: MsOS_Init fail\n",__FUNCTION__);
        MI_SYS_BUG();
    }

    if(!MsOS_MPool_Init())
    {
        DBG_ERR("%s: MsOS_MPool_Init fail\n",__FUNCTION__);
        MI_SYS_BUG();
    }
#if NOSUPPORT_MsOS_MPool_Add_PA2VARange
    MI_VDEC_DoMsOsMPoolMapping("E_MMAP_ID_VDEC_CPU");
#endif
    if (0)
    {
        MI_VDEC_DoMsOsMPoolMapping("E_MMAP_ID_VDEC_CPU");
        MI_VDEC_DoMsOsMPoolMapping("E_MMAP_ID_VDEC_BITSTREAM");
        MI_VDEC_DoMsOsMPoolMapping("E_MMAP_ID_VDEC_FRAMEBUFFER");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
////Debug Function Start
void _MI_VDEC_IMPL_DebugDumpJpegDecImage(MI_U8 *pYuv422Data, MI_U32 width, MI_U32 height, MI_U32 pitch, int chn)
{
    struct file *fp = NULL;
    mm_segment_t fs;
    char name[128];
    int length = width * height * 2;
    static int frmcnt[MI_VDEC_MAX_CHN_NUM] = {0};

    memset(name, 0x0, sizeof(name));
    sprintf(name, "%s/chn_%d_jpeg_dump_vdec[%d_%d_%d]_%d.yuv", _stDumpParams.au8DumpPath, chn, width, height, pitch, frmcnt[chn]++);
    fp =filp_open(name, O_RDWR | O_CREAT, 0777);
    if (IS_ERR(fp))
    {
        DBG_ERR("Open File Faild  PTR_ERR_fp=%ld\n",PTR_ERR(fp));
        return;
    }

    fs =get_fs();
    set_fs(KERNEL_DS);
    fp->f_op->llseek(fp, 0, SEEK_SET);

    if(fp->f_op->write(fp, pYuv422Data, length, &(fp->f_pos)) != length)
    {
        DBG_ERR("fwrite %s failed\n", name);
        goto _END;
    }

    DBG_WRN("dump file(%s) ok ..............[len:%d]\n", name, length);

_END:
    set_fs(fs);
    filp_close(fp,NULL);
}

MI_S32 _MI_VDEC_IMPL_DebugDetileYUV(void *pDstBuf, void *pSrcYBuf, void *pSrcUVBuf,
        MI_U32 u32Width, MI_U32 u32Pitch, MI_U32 u32Height, MI_SYS_FrameTileMode_e eMode)
{
    MI_U32 i = 0;
    MI_U32 j = 0;
    MI_U32 mfb_height;
    MI_U32 mfb_uv_height;
    MI_U32 mfetch_num_per_line;
    MI_U32 page_cnt = 0;
    MI_U32 row_st   = 0;
    MI_U32 col_st   = 0;
    MI_U32 owidth   = u32Width;
    MI_U8* pbuf     = (MI_U8 *)pSrcYBuf;
    MI_U8* pUDstBuf;
    MI_U8* pVDstBuf;
    MI_U32 shift = 0;
    MI_U32 endcol;
    MI_U32 fetch_size;
    MI_U32 DETILE_FETCH_SIZE = 16;
    MI_U32 DETILE_PAGE_SIZE  = 32;

    switch(eMode)
    {
        case E_MI_SYS_FRAME_TILE_MODE_16x16:
            DETILE_FETCH_SIZE = 16;
            DETILE_PAGE_SIZE  = 16;
            break;
        case E_MI_SYS_FRAME_TILE_MODE_16x32:
            DETILE_FETCH_SIZE = 16;
            DETILE_PAGE_SIZE  = 32;
            break;
        case E_MI_SYS_FRAME_TILE_MODE_32x16:
            DETILE_FETCH_SIZE = 32;
            DETILE_PAGE_SIZE  = 16;
            break;
        case E_MI_SYS_FRAME_TILE_MODE_32x32:
            DETILE_FETCH_SIZE = 32;
            DETILE_PAGE_SIZE  = 32;
            break;
        default:
            DETILE_FETCH_SIZE = 16;
            DETILE_PAGE_SIZE  = 32;
    }

    if (E_MI_SYS_FRAME_TILE_MODE_32x16 == eMode || E_MI_SYS_FRAME_TILE_MODE_16x16 == eMode)
    {
        mfb_height    = ((u32Height+15)>>4)<<4;
        mfb_uv_height = (((u32Height>>1)+15)>>4)<<4;
    }
    else
    {
        mfb_height    = ((u32Height+31)>>5)<<5;
        mfb_uv_height = (((u32Height>>1)+31)>>5)<<5;
    }
    mfetch_num_per_line = div64_u64(u32Pitch, DETILE_FETCH_SIZE);

    if (u32Width != u32Pitch)
    {
        div_u64_rem(u32Width, DETILE_FETCH_SIZE, &shift);
    }
    else
    {
        shift = DETILE_FETCH_SIZE;
    }
    endcol = div64_u64(owidth, DETILE_FETCH_SIZE) * DETILE_FETCH_SIZE;

    for (i = 0; i < mfb_height * mfetch_num_per_line; i++)
    {
        page_cnt = div64_u64(i, (DETILE_PAGE_SIZE * mfetch_num_per_line));
        div_u64_rem(i, DETILE_PAGE_SIZE, &row_st);
        row_st   = row_st + page_cnt * DETILE_PAGE_SIZE;
        div_u64_rem(div64_u64(i, DETILE_PAGE_SIZE) * DETILE_FETCH_SIZE, u32Pitch, &col_st);

        if (row_st < u32Height && col_st < u32Width)
        {
            if (col_st == endcol)
            {
                fetch_size = shift;
            }
            else
            {
                fetch_size = DETILE_FETCH_SIZE;
            }
            memcpy((MI_U8*)pDstBuf + (row_st * owidth + col_st), pbuf, fetch_size);
        }

        pbuf += DETILE_FETCH_SIZE;
    }

    pbuf     = (MI_U8 *)pSrcUVBuf;
    pUDstBuf = (MI_U8 *)pDstBuf + (owidth * u32Height);
    pVDstBuf = (MI_U8 *)pUDstBuf + ((owidth * u32Height) >> 2);
    u32Height = u32Height >> 1;
    owidth    = owidth >> 1;

    // UV
    for (i = 0; i < mfb_uv_height * mfetch_num_per_line; i++)
    {
        page_cnt = div64_u64(i, (DETILE_PAGE_SIZE * mfetch_num_per_line));
        div_u64_rem(i, DETILE_PAGE_SIZE, &row_st);
        row_st   = (row_st + page_cnt * DETILE_PAGE_SIZE);
        div_u64_rem(div64_u64(i, DETILE_PAGE_SIZE) * DETILE_FETCH_SIZE, u32Pitch, &col_st);

        if ((row_st < u32Height) && (col_st < u32Width))
        {
            if (col_st == endcol)
            {
                fetch_size = shift;
            }
            else
            {
                fetch_size = DETILE_FETCH_SIZE;
            }

            for (j = 0; j < fetch_size; j++)
            {
                if ((j & 0x1) == 0)
                    *(pUDstBuf + (row_st * owidth + ((col_st + j) >> 1))) = pbuf[j];
                else
                    *(pVDstBuf + (row_st * owidth + ((col_st + j) >> 1))) = pbuf[j];
            }
        }

        pbuf += DETILE_FETCH_SIZE;
    }

    return MI_SUCCESS;
}

static void _MI_VDEC_IMPL_DebugWriteFile(MI_U8 * pu8Name, MI_U8 *pu8Buf, MI_U32 u32Len)
{
    struct file *fp = NULL;
    mm_segment_t fs;

    fp =filp_open(pu8Name, O_RDWR | O_CREAT, 0777);
    if (IS_ERR(fp))
    {
        DBG_ERR("Open File Faild  PTR_ERR_fp=%ld\n",PTR_ERR(fp));
        return;
    }

    fs =get_fs();
    set_fs(KERNEL_DS);
    fp->f_op->llseek(fp, 0, SEEK_SET);

    if(fp->f_op->write(fp, pu8Buf, u32Len, &(fp->f_pos)) != u32Len)
    {
        DBG_ERR("fwrite %s failed\n", pu8Name);
    }
    else
    {
        DBG_WRN("dump file(%s) v1 ok ..............[len:%d]\n", pu8Name, u32Len);
    }

    set_fs(fs);
    filp_close(fp,NULL);
}

void _MI_VDEC_IMPL_DebugDumpFrameBuffer(MI_U32 u32Chn, MI_SYS_BufInfo_t *pstBufInfo)
{
    static MI_U32 u32GetFrm[MI_VDEC_MAX_CHN_NUM] = {0};
    mi_vdec_DispFrame_t *pstDispFrm = NULL;
    static MI_U8 au8FileName[128];
    MI_BOOL bDump = TRUE;
    MI_U32 u32Pitch = 0;

    pstDispFrm = (mi_vdec_DispFrame_t *)pstBufInfo->stMetaData.pVirAddr;
    u32Pitch = pstDispFrm->stFrmInfo.u16Pitch;
    DBG_WRN("Chn(%d) Y:0x%llx, UV:0x%llx, YSize:0x%x, UVSize:0x%x, (W:%d, H:%d, P:%d)\n",
        u32Chn,
        pstDispFrm->stFrmInfo.phyLumaAddr,
        pstDispFrm->stFrmInfo.phyChromaAddr,
        pstDispFrm->stFrmInfo.u16Width * pstDispFrm->stFrmInfo.u16Height,
        pstDispFrm->stFrmInfo.u16Width * pstDispFrm->stFrmInfo.u16Height >> 1,
        pstDispFrm->stFrmInfo.u16Width,
        pstDispFrm->stFrmInfo.u16Height,
        pstDispFrm->stFrmInfo.u16Width,
        u32Pitch);
    if (u32GetFrm[u32Chn] > 2)
    {
        DBG_INFO("Dump Return\n");
        u32GetFrm[u32Chn] = 0;
        _stDumpParams.bDumpFB[u32Chn] = FALSE;
        return;
    }

    memset(au8FileName, 0x0, sizeof(au8FileName));
    if (_stResMgr.astChnInfo[u32Chn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H264)
    {
        sprintf(au8FileName, "%s/chn_%d_%s_dump_vdec[%d_%d_%d]_%d.yuv", _stDumpParams.au8DumpPath, u32Chn, "h264",
            pstDispFrm->stFrmInfo.u16Width, pstDispFrm->stFrmInfo.u16Height, u32Pitch, u32GetFrm[u32Chn]);
    }
    else if (_stResMgr.astChnInfo[u32Chn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        sprintf(au8FileName, "%s/chn_%d_%s_dump_vdec[%d_%d_%d]_%d.yuv", _stDumpParams.au8DumpPath, u32Chn, "h265",
            pstDispFrm->stFrmInfo.u16Width, pstDispFrm->stFrmInfo.u16Height, u32Pitch, u32GetFrm[u32Chn]);
    }
    else
    {
        bDump = FALSE;
    }

    if (bDump)
    {
        MI_U32 u32Len = 0;
        MI_U8 *pu8Ybuf = NULL;
        MI_U8 *pu8UVbuf = NULL;
        MI_U8 *pu8BitlenBuf = NULL;
        char *pYuv = NULL;

        if (_stResMgr.astChnInfo[u32Chn].stFrameMemInfo.pVirAddr)
        {
            MI_S32 s32Offset = 0;
            s32Offset = pstDispFrm->stFrmInfo.phyLumaAddr - _stResMgr.astChnInfo[u32Chn].stFrameMemInfo.phyAddr;
            pu8Ybuf = (char *)(_stResMgr.astChnInfo[u32Chn].stFrameMemInfo.pVirAddr) + s32Offset;

            s32Offset = pstDispFrm->stFrmInfo.phyChromaAddr - _stResMgr.astChnInfo[u32Chn].stFrameMemInfo.phyAddr;
            pu8UVbuf = (char *)(_stResMgr.astChnInfo[u32Chn].stFrameMemInfo.pVirAddr) + s32Offset;

            if(_stResMgr.astChnInfo[u32Chn].u8EnableMfcodec)
            {
                pu8BitlenBuf = (MI_U8 *)MsOS_PA2KSEG1(pstDispFrm->stDbInfo.phyDbBase);
            }
        }
        else
        {
            DBG_WRN("chn(%d) frame buffer unmap ....\n", u32Chn);
            return;
        }

        u32Len = (pstDispFrm->stFrmInfo.u16Pitch * 3 >> 1) * pstDispFrm->stFrmInfo.u16Height;
        pYuv = (char *)MI_VMALLOC(u32Len);

        if (pYuv)
        {
            _MI_VDEC_IMPL_DebugDetileYUV(pYuv, pu8Ybuf, pu8UVbuf, pstDispFrm->stFrmInfo.u16Width, u32Pitch, pstDispFrm->stFrmInfo.u16Height, pstDispFrm->stFrmInfoExt.eFrameTileMode);
            _MI_VDEC_IMPL_DebugWriteFile(au8FileName, pYuv, u32Len);
            MI_VFREE(pYuv);
            pYuv = NULL;
        }

        sprintf(au8FileName, "%s/chn_%d_dump_vdec[%d_%d_%d]_%d_luma.yuv", _stDumpParams.au8DumpPath, u32Chn,
            pstDispFrm->stFrmInfo.u16Width, pstDispFrm->stFrmInfo.u16Height, u32Pitch, u32GetFrm[u32Chn]);
        _MI_VDEC_IMPL_DebugWriteFile(au8FileName, pu8Ybuf, pstDispFrm->stFrmInfo.u16Width * pstDispFrm->stFrmInfo.u16Height);

        sprintf(au8FileName, "%s/chn_%d_dump_vdec[%d_%d_%d]_%d_chroma.yuv", _stDumpParams.au8DumpPath, u32Chn,
            pstDispFrm->stFrmInfo.u16Width, pstDispFrm->stFrmInfo.u16Height, u32Pitch, u32GetFrm[u32Chn]);
        _MI_VDEC_IMPL_DebugWriteFile(au8FileName, pu8UVbuf, pstDispFrm->stFrmInfo.u16Width * pstDispFrm->stFrmInfo.u16Height >> 1);

        if(_stResMgr.astChnInfo[u32Chn].u8EnableMfcodec)
        {
            sprintf(au8FileName, "%s/chn_%d_dump_vdec[%d_%d_%d]_%d_mfdec_bitlen.yuv", _stDumpParams.au8DumpPath, u32Chn,
                pstDispFrm->stDbInfo.u16HSize, pstDispFrm->stDbInfo.u16VSize, pstDispFrm->stDbInfo.u16DbPitch, u32GetFrm[u32Chn]);
            _MI_VDEC_IMPL_DebugWriteFile(au8FileName, pu8BitlenBuf, _ALIGN_BYTE_(((pstDispFrm->stDbInfo.u16HSize + 63) / 64) * (((pstDispFrm->stDbInfo.u16VSize + 8) + 63) / 64) * 8 * 32, 512));
        }

        u32GetFrm[u32Chn]++;
    }
}

void _MI_VDEC_IMPL_DebugDumpJpegFrmBuf(MI_U32 u32Chn, mi_vdec_DispFrame_t *pstDispFrm)
{
    static MI_U8 u8DumpCount[MI_VDEC_MAX_CHN_NUM] = {0};
    MI_U32 u32Offset = 0;
    MI_U8 *pu8VdecBufAddr = NULL;

    if(u8DumpCount[u32Chn] > 2)
    {
        DBG_INFO("Dump Return\n");
        _stDumpParams.bDumpFB[u32Chn] = FALSE;
        u8DumpCount[u32Chn] = 0;
        return;
    }

    if (!pstDispFrm)
    {
        DBG_ERR("Chn(%d) pstDispFrm is Null %p\n", u32Chn, pstDispFrm);
        return;
    }

    u32Offset = pstDispFrm->stFrmInfo.phyLumaAddr - _stResMgr.astChnInfo[u32Chn].stFrameMemInfo.phyAddr;
    pu8VdecBufAddr = _stResMgr.astChnInfo[u32Chn].stFrameMemInfo.pVirAddr + u32Offset;

    DBG_INFO("phyLumaAddr[%llx] phyChromaAddr[%llx] u32Offset[%u]  stFrameMemInfo.pVirAddr[%p] pu8VdecBufAddr[%p]\n",
                pstDispFrm->stFrmInfo.phyLumaAddr,
                pstDispFrm->stFrmInfo.phyChromaAddr,
                u32Offset,
                _stResMgr.astChnInfo[u32Chn].stFrameMemInfo.pVirAddr,
                pu8VdecBufAddr);

    _MI_VDEC_IMPL_DebugDumpJpegDecImage(pu8VdecBufAddr, pstDispFrm->stFrmInfo.u16Width, pstDispFrm->stFrmInfo.u16Height, pstDispFrm->stFrmInfo.u16Pitch, u32Chn);
    u8DumpCount[u32Chn]++;
}

void _MI_VDEC_IMPL_DebugDumpAppPushEs(MI_U32 u32Chn, MI_U8 *pu8EsData, MI_U32 u32Len, MI_BOOL bReset)
{
    static struct file *fp[MI_VDEC_MAX_CHN_NUM] = { NULL };
    static mm_segment_t fs[MI_VDEC_MAX_CHN_NUM];
    int length = 0;

    if (!pu8EsData)
    {
        DBG_ERR("Chn(%d) pu8EsData Is Null %p\n", u32Chn, pu8EsData);
        return;
    }

    if (bReset || !fp[u32Chn])
    {
        static char name[128];
        memset(name, 0x0, sizeof(name));

        if (fp[u32Chn])
        {
            set_fs(fs[u32Chn]);
            filp_close(fp[u32Chn],NULL);
        }

        if (_stResMgr.astChnInfo[u32Chn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H264)
        {
            sprintf(name, "%s/chn_%d_%s_dump_vdec.es", _stDumpParams.au8DumpPath, u32Chn, "h264");
        }
        else if (_stResMgr.astChnInfo[u32Chn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
        {
            sprintf(name, "%s/chn_%d_%s_dump_vdec.es", _stDumpParams.au8DumpPath, u32Chn, "h265");
        }
        else if (_stResMgr.astChnInfo[u32Chn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_JPEG)
        {
            sprintf(name, "%s/chn_%d_%s_dump_vdec.es", _stDumpParams.au8DumpPath, u32Chn, "jpeg");
        }
        else
        {
            DBG_ERR("Error Code Type\n");
            return;
        }

        fp[u32Chn] =filp_open(name, O_RDWR | O_CREAT, 0777);
        if (IS_ERR(fp[u32Chn]))
        {
            DBG_ERR("Open File Faild  PTR_ERR_fp=%ld\n",PTR_ERR(fp[u32Chn]));
            fp[u32Chn] = NULL;
            return;
        }

        fs[u32Chn] = get_fs();
        set_fs(KERNEL_DS);
        ///N+1 m, all cpu address and size
        fp[u32Chn]->f_op->llseek(fp[u32Chn], 0, SEEK_SET);
    }

    length = fp[u32Chn]->f_op->write(fp[u32Chn], pu8EsData, u32Len, &(fp[u32Chn]->f_pos));
    if(u32Len != length)
    {
        DBG_ERR("Dump u32Chn(%d) Es Faild (%d %d)\n", u32Chn, u32Len, length);
    }

}

void _MI_VDEC_IMPL_DebugDumpBitStreamBuffer(MI_U32 u32Chn)
{
    static struct file *fp = NULL;
    static mm_segment_t fs;
    int length = 0;

    static char name[128];
    memset(name, 0x0, sizeof(name));

    if (FALSE == _stResMgr.astChnInfo[u32Chn].bStart)
    {
        DBG_ERR("UnCreate Chn(%d)\n", u32Chn);
        return;
    }

    if (fp)
    {
        set_fs(fs);
        filp_close(fp,NULL);
    }

    if (_stResMgr.astChnInfo[u32Chn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H264)
    {
        sprintf(name, "%s/chn_%d_%s_dump_now_vdec.es", _stDumpParams.au8DumpPath, u32Chn, "h264");
    }
    else if (_stResMgr.astChnInfo[u32Chn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        sprintf(name, "%s/chn_%d_%s_dump_now_vdec.es", _stDumpParams.au8DumpPath, u32Chn, "h265");
    }
    else if (_stResMgr.astChnInfo[u32Chn].stChnAttr.eCodecType == E_MI_VDEC_CODEC_TYPE_JPEG)
    {
        sprintf(name, "%s/chn_%d_%s_dump_now_vdec.es", _stDumpParams.au8DumpPath, u32Chn, "jpeg");
    }
    else
    {
        DBG_ERR("Error Code Type\n");
        return;
    }

    fp =filp_open(name, O_RDWR | O_CREAT, 0777);
    if (IS_ERR(fp))
    {
        DBG_ERR("Open File Faild  PTR_ERR_fp=%ld\n",PTR_ERR(fp));
        fp = NULL;
        return;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    ///N+1 m, all cpu address and size
    fp->f_op->llseek(fp, 0, SEEK_SET);

    length = fp->f_op->write(fp, _stResMgr.astChnInfo[u32Chn].stEsMemInfo.pVirAddr, _stResMgr.astChnInfo[u32Chn].stEsMemInfo.u32BufSize, &(fp->f_pos));
    if(_stResMgr.astChnInfo[u32Chn].stEsMemInfo.u32BufSize != length)
    {
        DBG_ERR("Dump u32Chn(%d) Es Faild (%d %d)\n", u32Chn, _stResMgr.astChnInfo[u32Chn].stEsMemInfo.u32BufSize, length);
    }
}
////Debug Function End
//////////////////////////////////////////////////////////////////////////////////////////////////
