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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/irqreturn.h>
#include <linux/delay.h>
#include <linux/math64.h>


#include "mi_common_datatype.h"
#include "mi_common.h"
#include "mi_print.h"
#include "mi_sys.h"
#include "mi_sys_internal.h"
#include "mi_vdec_internal.h"
#include "mi_divp_datatype.h"
#include "mi_divp_datatype_internal.h"
#include "mi_divp_impl.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "mhal_divp_datatype.h"
#include "mhal_divp.h"
#include "mi_syscfg_datatype.h"
#include "mi_syscfg.h"
#include "mi_sys_sideband_msg.h"
#include "mi_sys_proc_fs_internal.h"

#if defined(CONFIG_MSTAR_CHIP_I2)&&(CONFIG_MSTAR_CHIP_I2==1)
#define DIVP_SUPPORT_RGN (1)
#endif

#define DIVP_RGN_OVERLAP_TEST (0)

#if defined(DIVP_SUPPORT_RGN)&&(DIVP_SUPPORT_RGN==1)
#include "../rgn/mi_rgn_internal.h"
#endif

#if defined(SUPPORT_DIVP_USE_GE_SCALING_UP)&&(SUPPORT_DIVP_USE_GE_SCALING_UP==1)
#include "mi_gfx.h"
#endif

#define PRINT_TIME(fmt, args...)      ({do{printk("\n [MI DIVP Time]:%s[%d]: ", __FUNCTION__,__LINE__);printk(fmt, ##args);}while(0);})
#define DIVP_TASK_DUMMY_REG (0x123CF0)//BK123C_78[15~0]
#define MI_DIVP_CHN_NULL (0xFF)
#define DIVP_CHN_MIN (0x00)
#define DIVP_CHN_MAX (MI_DIVP_CHN_MAX_NUM)
#define DIVP_CHN_INPUT_PORT_ID (0)
#define DIVP_CHN_OUTPUT_PORT_ID (0)
#define DIVP_CROP_ALIGNMENT (2)
#define ENABLE_DEBUG_LOCK (FALSE)
#define ENABLE_VDEC_DEBUG (FALSE)

/////////////////////////////////////////////////////debug fence start/////////////////////////////////////////////////////
///1 enable ENABLE_DEBUG_FENCE
#define ENABLE_DEBUG_FENCE (FALSE)
///2 need add variables as members to mi_sys_ChnTaskInfo_t
//  MI_U64 u64RecieveTask;//divp recieve task
//  MI_PHY phyRecieveAddress;//divp recieve buffer address
//  MI_U64 u64ProcessTask;//divp process task
//  MI_PHY phyProcessAddress;//divp recieve buffer address
//  MI_U64 u64FinishTask;//divp finish task
//  MI_PHY phyFinishAddress;//divp recieve buffer address
/////////////////////////////////////////////////////debug fence end/////////////////////////////////////////////////////

/////////////////////////////////////////////////////debug performance start/////////////////////////////////////////////////////
///1 enable ENABLE_DEBUG_PERFORMANCE
#define ENABLE_DEBUG_PERFORMANCE (FALSE)
#if defined(ENABLE_DEBUG_PERFORMANCE) && (ENABLE_DEBUG_PERFORMANCE == 1)
#define DIVP_RECORD_TASK_NUM (1000)
#define DIVP_PERF_TIME(pu64Time) do {\
        struct timespec sttime;\
        memset(&sttime, 0, sizeof(sttime));\
        do_posix_clock_monotonic_gettime(&sttime);\
         *(pu64Time) = ((MI_U64)sttime.tv_sec) * 1000000ULL + (sttime.tv_nsec / 1000);\
    } while(0)
#else
#define DIVP_PERF_TIME(pu64Time)
#endif
///2 need add variables as members to mi_sys_ChnTaskInfo_t
//  MI_U64 u64DivpGotTask;//divp recieve task
//  MI_U64 u64DivpTaskId;//divp recieve task number
//  MI_U64 u64DivpGotCmdqBuffer;//CMDQ got buffer:begin to process task
//  MI_U64 u64DivpProcessTaskStart;//divp start to process task
//  MI_U64 u64DivpProcessTaskEnd;//divp process task end(create all commands)
//  MI_U64 u64DivpCmdqKickOff;//kickof CMDQ
//  MI_U64 u64DivpIsrComming;//ISR comming time
//  MI_U64 u64DivpIsrId;//ISR number
//  MI_U64 u64DivpFinishTask;//divp finish task
/////////////////////////////////////////////////////debug performance end/////////////////////////////////////////////////////

#define MI_DIVP_FENCE_MAX (0xFFF)
#define MI_DIVP_INPUT_WIDTH_ALIGNMENT_YUV422 (16)
#define MI_DIVP_INPUT_WIDTH_ALIGNMENT_YUV420 (32)
#define MI_DIVP_TIMING_WIDTH_2K (1920)
#define MI_DIVP_TIMING_WIDTH_4K (3840)
#define MI_DIVP_TIMING_HEIGHT_2K (2160)
#define MI_DIVP_TIMING_HEIGHT_1K (1080)
#define MI_DIVP_OUTPUT_WIDTH_MIN (128)
#define MI_DIVP_OUTPUT_HEIGHT_MIN (64)
#define MI_DIVP_OUTPUT_WIDTH_MAX (4096)
#define MI_DIVP_OUTPUT_HEIGHT_MAX (4096)
#define MI_DIVP_SW_CROP_START_ALIGNMENT (64)//crop window start (x,y) alignment
#define MI_DIVP_SW_CROP_WIDTH_ALIGNMENT (32)//crop window width alignment
#define MI_DIVP_SW_CROP_HEIGHT_ALIGNMENT (32)//crop window height alignment
#define MI_DIVP_MFDEC_WIDTH_ALIGNMENT (32)//MFDEC width alignment
#define MI_DIVP_MFDEC_STARTX_ALIGNMENT (32)//MFDEC StartX alignment
#define MI_DIVP_ZOOM_RATION_DEN 0x80000UL //so we support 8K resulotion calculation where U32 overflow
#define MI_DIVP_OUTPUT_WIDTH_ALIGNMENT (32/2)//output width 32bytes/bytes per pixel
#define INVALID_ADDRESS (0x0000)
#define MI_DIVP_SCL_CLOCK (432) // divp scaler colock
#define MI_DIVP_SCL_RATIO_INPUT (3)
#define MI_DIVP_SCL_RATIO_OUTPUT (2)// out_width/In_width < 2/3  ==> 2Pmode
#define MI_DIVP_CORRIDOR_AREA_RATIO (2) // CORRIDOR mode input area > output_area > 2 ==> 2P mode

#if defined(SUPPORT_DIVP_USE_GE_SCALING_UP)&&(SUPPORT_DIVP_USE_GE_SCALING_UP==1)
#define DIVP_GE_SCALING_META_MAGIC (0x44505343ULL<<32)
#define CHECK_DIVP_GE_SCL_MAGIC(val)  do{MI_SYS_BUG_ON(((val)&0xFFFFFFFF00000000ULL)!=DIVP_GE_SCALING_META_MAGIC);}while(0)
#endif

#define MI_DIVP_CHECK_CHN(DivpChn)  \
    if(DivpChn < DIVP_CHN_MIN || DivpChn > DIVP_CHN_MAX)  \
    {   \
        DBG_ERR("DivpChn is invalid! DivpChn = %u.\n", DivpChn);   \
        return MI_DIVP_ERR_INVALID_CHNID;   \
    }

#define MI_DIVP_CHECK_POINTER(pPtr)  \
    if(NULL == pPtr)  \
    {   \
        DBG_ERR("Invalid parameter! NULL pointer.\n");   \
        return MI_DIVP_ERR_NULL_PTR;   \
    }

#define MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DivpChn)  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("channel %d global vatiable locked.~~ ++\n", DivpChn);   \
        }\
        down(&gChnCtxSem[DivpChn]);  \
    }

#define MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DivpChn)  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("channel %d global vatiable unlocked.~~ ++\n", DivpChn);   \
        }\
        up(&gChnCtxSem[DivpChn]);  \
    }

#define MI_DIVP_LOCK_WORKING_CAPTURE_TASK_LIST()  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("working capture task locked.~~ @@\n");   \
        }\
        down(&divp_working_capture_task_list_sem);  \
    }

#define MI_DIVP_UNLOCK_WORKING_CAPTURE_TASK_LIST()  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("working capture task unlocked.~~ @@\n");   \
        }\
        up(&divp_working_capture_task_list_sem);  \
    }

#define MI_DIVP_LOCK_TODO_CAPTURE_TASK_LIST()  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("todo capture task locked.~~ ##\n");   \
        }\
        down(&divp_todo_capture_task_list_sem);  \
    }

#define MI_DIVP_UNLOCK_TODO_CAPTURE_TASK_LIST()  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("todo capture task unlocked.~~ ##\n");   \
        }\
        up(&divp_todo_capture_task_list_sem);  \
    }

#define MI_DIVP_LOCK_WORKING_COMMON_TASK_LIST()  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("working common task locked.~~ $$\n");   \
        }\
        down(&divp_working_common_task_list_sem);  \
    }

#define MI_DIVP_UNLOCK_WORKING_COMMON_TASK_LIST()  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("working common task unlocked.~~ $$\n");   \
        }\
        up(&divp_working_common_task_list_sem);  \
    }


#define MI_DIVP_LOCK_TODO_COMMON_TASK_LIST()  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("todo common task locked.~~ %%%\n");   \
        }\
        down(&divp_todo_common_task_list_sem);  \
    }

#define MI_DIVP_UNLOCK_TODO_COMMON_TASK_LIST()  \
    {   \
        if(ENABLE_DEBUG_LOCK) \
        {\
            DBG_INFO("todo common task unlocked.~~ %%%\n");   \
        }\
        up(&divp_todo_common_task_list_sem);  \
    }

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static MI_SYS_DRV_HANDLE gDivpDevHdl = NULL;
static MI_BOOL _gbInited = FALSE;
static MI_U64 _gu64InitTimes = 0;

#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
#define PRINTF_PROC(fmt, args...)  {do{MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_RED"[MI DIVP PROCFS]:" fmt ASCII_COLOR_END, ##args);}while(0);}

static mi_divp_ProcessOneChnl_t gstProcessOneChnl;
static MI_BOOL gbStopOneChnl[MI_DIVP_CHN_MAX_NUM];
#endif

//save channel's settings, channel number is 33(APP 32 + 1 private for capture)
static mi_divp_ChnContex_t gstDivpChnCtx[MI_DIVP_CHN_MAX_NUM + 1];
static MI_U32 gu32ChnNum = 0;
static MI_U32 gu32DevNum = 0;
static struct task_struct * gpstWorkThread = NULL;
static struct task_struct * gpstIsrThread = NULL;
static struct semaphore gChnCtxSem[MI_DIVP_CHN_MAX_NUM + 1];
static MI_BOOL gbResetAllAttr[MI_DIVP_CHN_MAX_NUM + 1] = {FALSE};
#if ENABLE_DEBUG_FENCE
static MI_U64 _gau64ReceiveFrmNum[32] = {0};
static MI_U64 _gau64ProcessFrmNum[32] = {0};
static MI_U64 _gau64FinishFrmNum[32] = {0};
#endif
#if ENABLE_DEBUG_PERFORMANCE
static MI_U64 _gu64TaskNum = 0;
static MI_U64 _gu64IsrNum = 0;
static MI_U64 _gu64IsrCommingTime = 0;
static MI_U64 _gu64StartTime = 0;
static MI_U64 _gu64EndTime = 0;
static char _gchTraceInfo[DIVP_RECORD_TASK_NUM][180];
static MI_U64 _gu64TraceCnt = 0;
#endif
DECLARE_WAIT_QUEUE_HEAD(divp_isr_waitqueue);
LIST_HEAD(divp_todo_common_task_list);//process DRAM data
LIST_HEAD(divp_working_common_task_list);
LIST_HEAD(divp_todo_capture_task_list);//caputre Display timing
LIST_HEAD(divp_working_capture_task_list);
DEFINE_SEMAPHORE(divp_todo_common_task_list_sem);
DEFINE_SEMAPHORE(divp_working_common_task_list_sem);
DEFINE_SEMAPHORE(divp_todo_capture_task_list_sem);
DEFINE_SEMAPHORE(divp_working_capture_task_list_sem);

EXPORT_SYMBOL(mi_divp_Init);
EXPORT_SYMBOL(mi_divp_DeInit);
EXPORT_SYMBOL(mi_divp_CaptureTiming);

MI_S32 mi_divp_dump_FrameInfo(MHAL_DIVP_InputInfo_t* iInfo,  MHAL_DIVP_OutPutInfo_t* oInfo)
{
    BUG_ON((!iInfo) && (!oInfo));

    if (iInfo) {
        DBG_INFO("DivpInputInfo: u64BufAddr[0](0x%llx) u64BufAddr[1](0x%llx) \
                u64BufAddr[2](0x%llx) u16InputWidth(%d) u16InputHeight(%d) \
                u16Stride[0](%d) u16Stride[1](%d) u16Stride[2](%d)\n",
        iInfo->u64BufAddr[0], iInfo->u64BufAddr[1],
        iInfo->u64BufAddr[2], iInfo->u16InputWidth,
        iInfo->u16InputHeight, iInfo->u16Stride[0],
        iInfo->u16Stride[1], iInfo->u16Stride[2]);
    }

    if (oInfo) {
        DBG_INFO("DivpOutputInfo: u64BufAddr[0](0x%llx) u64BufAddr[1](0x%llx) \
                u64BufAddr[2](0x%llx) u16OutputWidth(%d) u16OutputHeight(%d) \
                u16Stride[0](%d) u16Stride[1](%d) u16Stride[2](%d)\n",
        oInfo->u64BufAddr[0], oInfo->u64BufAddr[1],
        oInfo->u64BufAddr[2], oInfo->u16OutputWidth,
        oInfo->u16OutputHeight, oInfo->u16Stride[0],
        oInfo->u16Stride[1], oInfo->u16Stride[2]);
    }

    return MI_SUCCESS;
}


static void inline _mi_divp_ClearTodoCaptureTask(void)
{
    struct list_head *pos, *n;
    mi_divp_CaptureInfo_t* pstCaptureTask = NULL;

    MI_DIVP_LOCK_TODO_CAPTURE_TASK_LIST();
    if(!list_empty_careful(&divp_todo_capture_task_list))
    {
        list_for_each_safe(pos, n, &divp_todo_capture_task_list)
        {
            pstCaptureTask = (mi_divp_CaptureInfo_t*)container_of(pos, mi_divp_CaptureInfo_t, capture_list);
            pstCaptureTask->bRetVal = FALSE;
            pstCaptureTask->pfnDispCallBack(pstCaptureTask);
        }
    }
    MI_DIVP_UNLOCK_TODO_CAPTURE_TASK_LIST()
}

static void inline _mi_divp_ClearWorkingCaptureTask(void)
{
    struct list_head *pos, *n;
    mi_divp_CaptureInfo_t* pstCaptureTask = NULL;

    MI_DIVP_LOCK_WORKING_CAPTURE_TASK_LIST();
    if(!list_empty_careful(&divp_working_capture_task_list))
    {
        list_for_each_safe(pos, n, &divp_working_capture_task_list)
        {
            pstCaptureTask = (mi_divp_CaptureInfo_t*)container_of(pos, mi_divp_CaptureInfo_t, capture_list);
            pstCaptureTask->bRetVal = FALSE;
            pstCaptureTask->pfnDispCallBack(&pstCaptureTask);
        }
    }
    MI_DIVP_UNLOCK_WORKING_CAPTURE_TASK_LIST();
}

static void inline _mi_divp_ClearTodoCommonTask(void)
{
    struct list_head *pos, *n;
    mi_sys_ChnTaskInfo_t* pstTask = NULL;

    MI_DIVP_LOCK_TODO_COMMON_TASK_LIST();
    if(!list_empty_careful(&divp_todo_common_task_list))
    {
        list_for_each_safe(pos, n, &divp_todo_common_task_list)
        {
            pstTask = (mi_sys_ChnTaskInfo_t*)container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
            mi_sys_RewindTask(pstTask);
        }
    }
    MI_DIVP_UNLOCK_TODO_COMMON_TASK_LIST();
}

static void inline _mi_divp_ClearWorkingCommonTask(void)
{
    struct list_head *pos, *n;
    mi_sys_ChnTaskInfo_t* pstTask = NULL;

    MI_DIVP_LOCK_WORKING_COMMON_TASK_LIST();
    if(!list_empty_careful(&divp_working_common_task_list))
    {
        list_for_each_safe(pos, n, &divp_working_common_task_list)
        {
            pstTask = (mi_sys_ChnTaskInfo_t*)container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
            mi_sys_RewindTask(pstTask);
        }
    }
    MI_DIVP_UNLOCK_WORKING_COMMON_TASK_LIST();
}

static MI_BOOL inline _mi_divp_IsFenceLE(MI_U16 u16Fence1, MI_U16 u16Fence2)
{
    MI_BOOL bRet = FALSE;
    if(((u16Fence1 <= u16Fence2) && (u16Fence2 - u16Fence1 < MI_DIVP_FENCE_MAX / 2))
        || ((u16Fence1 > u16Fence2) && (u16Fence2 + (MI_DIVP_FENCE_MAX - u16Fence1) < MI_DIVP_FENCE_MAX /2)))
    {
        bRet = TRUE;
    }
    return bRet;
}

static MI_S32 _mi_divp_CheckAttrValue(MI_DIVP_ChnAttr_t* pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if(((E_MI_SYS_ROTATE_180 == pstAttr->eRotateType)///DIVP only support rotate 90 and 270 degrees
            || (E_MI_SYS_ROTATE_NUM == pstAttr->eRotateType))
        || ((pstAttr->u32MaxHeight < pstAttr->stCropRect.u16Height)//crop need smaller than max size
            || (pstAttr->u32MaxWidth < pstAttr->stCropRect.u16Width))
        || ((MI_DIVP_OUTPUT_HEIGHT_MAX < pstAttr->u32MaxHeight)///Max height 2160.
            || (MI_DIVP_OUTPUT_WIDTH_MAX < pstAttr->u32MaxWidth)))///Max width 4096.
    {
        DBG_ERR(" Can't change channel static attribute. max(W,H) = (%u,%u) \n eRotateType = %u, crop window (W,H) = (%u, %u) \n",
            pstAttr->u32MaxWidth, pstAttr->u32MaxHeight, pstAttr->eRotateType, pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }

    return s32Ret;
}

static MI_BOOL _mi_divp_ChnBusy(MI_DIVP_CHN DivpChn)
{
    MI_BOOL bRet = FALSE;
    struct list_head *pos;
    mi_sys_ChnTaskInfo_t* pstChnTask = NULL;

    ///1 check capture task
    if(DIVP_CHN_MAX == DivpChn)
    {
        MI_DIVP_LOCK_TODO_CAPTURE_TASK_LIST();
        if(!list_empty_careful(&divp_todo_capture_task_list))
        {
            bRet = TRUE;
        }
        MI_DIVP_UNLOCK_TODO_CAPTURE_TASK_LIST();

        MI_DIVP_LOCK_WORKING_CAPTURE_TASK_LIST();
        if(!list_empty_careful(&divp_working_capture_task_list))
        {
            bRet = TRUE;
        }
        MI_DIVP_UNLOCK_WORKING_CAPTURE_TASK_LIST();
    }

    ///2 check todo common task
    MI_DIVP_LOCK_TODO_COMMON_TASK_LIST();
    if(!list_empty_careful(&divp_todo_common_task_list))
    {
        list_for_each(pos, &divp_todo_common_task_list)
        {
            pstChnTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
            if(pstChnTask->u32ChnId == DivpChn)
            {
                bRet = TRUE;
                break;
            }
        }
    }
    MI_DIVP_UNLOCK_TODO_COMMON_TASK_LIST();

    ///3 check working common task
    MI_DIVP_LOCK_WORKING_COMMON_TASK_LIST();
    if(!list_empty_careful(&divp_working_common_task_list))
    {
        list_for_each(pos, &divp_working_common_task_list)
        {
            pstChnTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
            if(pstChnTask->u32ChnId == DivpChn)
            {
                bRet = TRUE;
                break;
            }
        }
    }
    MI_DIVP_UNLOCK_WORKING_COMMON_TASK_LIST();

    return bRet;
}

static MHAL_DIVP_DisplayId_e _mi_divp_TransDispDeviceIdToDivpHal(mi_disp_Dev_e eMiDispId)
{
    MHAL_DIVP_DisplayId_e eHalDispId = E_MHAL_DIVP_Display_MAX;

    switch(eMiDispId)
    {
        case MI_DISP_DEV_0:
             eHalDispId = E_MHAL_DIVP_Display0;
             break;

        case MI_DISP_DEV_1:
             eHalDispId = E_MHAL_DIVP_Display1;
             break;

        default:
             eHalDispId = E_MHAL_DIVP_Display_MAX;
             break;
    }

    return eHalDispId;
}

static MHAL_DIVP_CapStage_e _mi_divp_TransMiCaptureStagrToHal(mi_divp_CapStage_e stMiCapStage)
{
    MHAL_DIVP_CapStage_e eHalCapStage = E_MHAL_DIVP_CAP_STAGE_MAX;

    switch(stMiCapStage)
    {
        case E_MI_DIVP_CAP_STAGE_INPUT:
             eHalCapStage = E_MHAL_DIVP_CAP_STAGE_INPUT;
             break;

        case E_MI_DIVP_CAP_STAGE_OUTPUT:
             eHalCapStage = E_MHAL_DIVP_CAP_STAGE_OUTPUT;
             break;

        case E_MI_DIVP_CAP_STAGE_OUTPUT_WITH_OSD:
             eHalCapStage = E_MHAL_DIVP_CAP_STAGE_OUTPUT_WITH_OSD;
             break;

        default:
             eHalCapStage = E_MHAL_DIVP_CAP_STAGE_MAX;
             break;
    }

    return eHalCapStage;
}


static MI_BOOL _mi_divp_CheckIPModeChange(MI_DIVP_CHN DivpChn, MHAL_DIVP_InputInfo_t* pstDivpInputInfo)
{
    MI_BOOL bRet = TRUE;
    if(E_MHAL_DIVP_SCAN_MODE_PROGRESSIVE == pstDivpInputInfo->eScanMode)//input P ome
    {
        if((E_MI_DIVP_DI_TYPE_3D == gstDivpChnCtx[DivpChn].stChnAttrPre.eDiType)
            || (E_MI_DIVP_DI_TYPE_2D == gstDivpChnCtx[DivpChn].stChnAttrPre.eDiType)) ///I to P change
        {
            gstDivpChnCtx[DivpChn].stChnAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
            gstDivpChnCtx[DivpChn].bIPchg = TRUE;
            gstDivpChnCtx[DivpChn].bChnAttrChg = TRUE;
        }
        else
        {
            gstDivpChnCtx[DivpChn].bIPchg = FALSE;
        }
    }
    else//input I mode
    {
        if((E_MI_DIVP_DI_TYPE_OFF == gstDivpChnCtx[DivpChn].stChnAttrPre.eDiType) ///P to I change
            || gstDivpChnCtx[DivpChn].bInputChange)//input size change
        {
            if((E_MI_DIVP_DI_TYPE_3D == gstDivpChnCtx[DivpChn].stChnAttrOrg.eDiType)
                || (E_MI_DIVP_DI_TYPE_2D == gstDivpChnCtx[DivpChn].stChnAttrOrg.eDiType))
            {
                //restore DI setting
                gstDivpChnCtx[DivpChn].stChnAttr.eDiType = gstDivpChnCtx[DivpChn].stChnAttrOrg.eDiType;
            }
            else
            {
                gstDivpChnCtx[DivpChn].stChnAttr.eDiType = E_MI_DIVP_DI_TYPE_3D;//P to I
            }
            gstDivpChnCtx[DivpChn].bChnAttrChg = TRUE;

            //reset filed infomation save in channel contex
            gstDivpChnCtx[DivpChn].eaFieldType[0] = E_MI_SYS_FIELDTYPE_NONE;
            gstDivpChnCtx[DivpChn].eaFieldType[1] = E_MI_SYS_FIELDTYPE_NONE;

            if(E_MHAL_DIVP_FIELD_TYPE_TOP == pstDivpInputInfo->stDiSettings.eFieldType)
            {
                pstDivpInputInfo->stDiSettings.eFieldOrderType = E_MHAL_DIVP_FIELD_ORDER_TYPE_TOP;
            }
            else if(E_MHAL_DIVP_FIELD_TYPE_BOTTOM == pstDivpInputInfo->stDiSettings.eFieldType)
            {
                pstDivpInputInfo->stDiSettings.eFieldOrderType = E_MHAL_DIVP_FIELD_ORDER_TYPE_BOTTOM;
            }
            else if(E_MHAL_DIVP_FIELD_TYPE_BOTH == pstDivpInputInfo->stDiSettings.eFieldType)
            {
                //fix me: need get value from input source.
            }
        }
    }

    DBG_INFO("bIPchg = %d.\n", gstDivpChnCtx[DivpChn].bIPchg);
    return bRet;
}

static MHAL_DIVP_DiType_e  _mi_divp_TransMiDiTypeToHal(MI_DIVP_DiType_e eMiDiType)
{
    MHAL_DIVP_DiType_e eHalDiType = E_MHAL_DIVP_DI_TYPE_OFF;

    switch(eMiDiType)
    {
        case E_MI_DIVP_DI_TYPE_OFF:
            eHalDiType = E_MHAL_DIVP_DI_TYPE_OFF;
            break;

        case E_MI_DIVP_DI_TYPE_2D:
            eHalDiType = E_MHAL_DIVP_DI_TYPE_2D;
            break;

        case E_MI_DIVP_DI_TYPE_3D:
            eHalDiType = E_MHAL_DIVP_DI_TYPE_3D;
            break;

        case E_MI_DIVP_DI_TYPE_NUM:
            eHalDiType = E_MHAL_DIVP_DI_TYPE_NUM;
            break;

        default:
            DBG_ERR(" invalid di type = %u.\n", eMiDiType);
            eHalDiType = E_MHAL_DIVP_DI_TYPE_OFF;
            break;
    }

    return eHalDiType;
}

static MHAL_DIVP_Rotate_e  _mi_divp_TransSysRotateToDivpHal(MI_SYS_Rotate_e eSysRotateType)
{
    MHAL_DIVP_Rotate_e eHalRotate = E_MHAL_DIVP_ROTATE_NONE;

    switch(eSysRotateType)
    {
        case E_MI_SYS_ROTATE_NONE:
            eHalRotate = E_MHAL_DIVP_ROTATE_NONE;
            break;

        case E_MI_SYS_ROTATE_90:
            eHalRotate = E_MHAL_DIVP_ROTATE_90;
            break;

        case E_MI_SYS_ROTATE_180:
            eHalRotate = E_MHAL_DIVP_ROTATE_180;
            break;

        case E_MI_SYS_ROTATE_270:
            eHalRotate = E_MHAL_DIVP_ROTATE_270;
            break;

        case E_MI_SYS_ROTATE_NUM:
            eHalRotate = E_MHAL_DIVP_ROTATE_NUM;
            break;

        default:
            DBG_ERR(" invalid rotate type = %u.\n", eSysRotateType);
            eHalRotate = E_MHAL_DIVP_ROTATE_NONE;
            break;
    }

    return eHalRotate;
}

static MHAL_DIVP_TnrLevel_e  _mi_divp_TransMiTnrLevelToHal(MI_DIVP_TnrLevel_e eMiTnrLevel)
{
    MHAL_DIVP_TnrLevel_e eHalTnrLvl = E_MHAL_DIVP_TNR_LEVEL_OFF;

    switch(eMiTnrLevel)
    {
        case E_MI_DIVP_TNR_LEVEL_OFF:
            eHalTnrLvl = E_MHAL_DIVP_DI_TYPE_OFF;
            break;

        case E_MI_DIVP_TNR_LEVEL_LOW:
            eHalTnrLvl = E_MHAL_DIVP_TNR_LEVEL_LOW;
            break;

        case E_MI_DIVP_TNR_LEVEL_MIDDLE:
            eHalTnrLvl = E_MHAL_DIVP_TNR_LEVEL_MIDDLE;
            break;

        case E_MI_DIVP_TNR_LEVEL_HIGH:
            eHalTnrLvl = E_MHAL_DIVP_TNR_LEVEL_NUM;
            break;

        default:
            DBG_ERR(" invalid Tnr level = %u.\n", eMiTnrLevel);
            eHalTnrLvl = E_MHAL_DIVP_TNR_LEVEL_OFF;
            break;
    }

    return eHalTnrLvl;
}

static MI_S32 _mi_divp_TransSysWindowToDivpHal(MI_SYS_WindowRect_t* pstWinRect, MHAL_DIVP_Window_t* pstHalWin)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_DIVP_CHECK_POINTER(pstWinRect)
    MI_DIVP_CHECK_POINTER(pstHalWin)

    pstHalWin->u16X = pstWinRect->u16X;
    pstHalWin->u16Y = pstWinRect->u16Y;
    pstHalWin->u16Width = pstWinRect->u16Width;
    pstHalWin->u16Height = pstWinRect->u16Height;

    return s32Ret;
}

static MI_SYS_PixelFormat_e _mi_divp_TransDivpHalPixelFormatToSys(MHAL_DIVP_PixelFormat_e eHalPxlFmt)
{
    MI_SYS_PixelFormat_e eSysPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_MAX;

    switch(eHalPxlFmt)
    {
        case E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_ARGB8888:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_ARGB8888;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_ABGR8888:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_ABGR8888;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_RGB565:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_RGB565;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_ARGB1555:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_ARGB1555;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_I2:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_I2;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_I4:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_I4;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_I8:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_I8;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_422:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_YUV_MST_420://mstar private format for Vdeec
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE1_H264;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE2_H265;
             break;

        case E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265:
             eSysPxlFmt = E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE3_H265;
             break;

        default:
             eSysPxlFmt = E_MHAL_DIVP_SCAN_MODE_MAX;
             break;
    }

    return eSysPxlFmt;
}

static MHAL_DIVP_PixelFormat_e _mi_divp_TransSysPixelFormatToDivpHal(MI_SYS_PixelFormat_e ePixelFormat)
{
    MHAL_DIVP_PixelFormat_e eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_MAX;

    switch(ePixelFormat)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV;
             break;

        case E_MI_SYS_PIXEL_FRAME_ARGB8888:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_ARGB8888;
             break;

        case E_MI_SYS_PIXEL_FRAME_ABGR8888:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_ABGR8888;
             break;

        case E_MI_SYS_PIXEL_FRAME_RGB565:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_RGB565;
             break;

        case E_MI_SYS_PIXEL_FRAME_ARGB1555:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_ARGB1555;
             break;

        case E_MI_SYS_PIXEL_FRAME_I2:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_I2;
             break;

        case E_MI_SYS_PIXEL_FRAME_I4:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_I4;
             break;

        case E_MI_SYS_PIXEL_FRAME_I8:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_I8;
             break;

        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_422;
             break;

        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
             break;

        case E_MI_SYS_PIXEL_FRAME_YUV_MST_420://mstar private format for Vdeec
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_YUV_MST_420;
             break;

        case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE1_H264:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264;
             break;

        case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE2_H265:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265;
             break;

        case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE3_H265:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265;
             break;

        default:
             eHalPxlFmt = E_MHAL_DIVP_PIXEL_FORMAT_MAX;
             break;
    }

    return eHalPxlFmt;
}

static MHAL_DIVP_TileMode_e _mi_divp_TransSysTileModeToDivpHal(MI_SYS_FrameTileMode_e eTileMode)
{
    MHAL_DIVP_TileMode_e eHalTileMode = E_MHAL_DIVP_TILE_MODE_MAX;

    switch(eTileMode)
    {
        case E_MI_SYS_FRAME_TILE_MODE_NONE:
             eHalTileMode = E_MHAL_DIVP_TILE_MODE_NONE;
             break;

        case E_MI_SYS_FRAME_TILE_MODE_16x16: // tile mode 16x16
             eHalTileMode = E_MHAL_DIVP_TILE_MODE_16x16;
             break;

        case E_MI_SYS_FRAME_TILE_MODE_16x32: // tile mode 16x32
             eHalTileMode = E_MHAL_DIVP_TILE_MODE_16x32;
             break;

        case E_MI_SYS_FRAME_TILE_MODE_32x16:// tile mode 32x16
             eHalTileMode = E_MHAL_DIVP_TILE_MODE_32x16;
             break;

        case E_MI_SYS_FRAME_TILE_MODE_32x32:// tile mode 32x32
             eHalTileMode = E_MHAL_DIVP_TILE_MODE_32x32;
             break;

        default:
             eHalTileMode = E_MHAL_DIVP_TILE_MODE_MAX;
             break;
    }

    return eHalTileMode;
}

static MHAL_DIVP_ScanMode_e _mi_divp_TransSysScanModeToDivpHal(MI_SYS_FieldType_e eFieldType, MI_SYS_FrameScanMode_e eScanMode)
{
    MHAL_DIVP_ScanMode_e eHalScanmode = E_MHAL_DIVP_SCAN_MODE_MAX;

    switch(eScanMode)
    {
        case E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE://P mode
            eHalScanmode = E_MHAL_DIVP_SCAN_MODE_PROGRESSIVE;
            break;

        case E_MI_SYS_FRAME_SCAN_MODE_INTERLACE:/// I mode
            if(E_MI_SYS_FIELDTYPE_BOTH == eFieldType)
            {
                eHalScanmode = E_MHAL_DIVP_SCAN_MODE_INTERLACE_FRAME;
            }
            else
            {
                eHalScanmode = E_MHAL_DIVP_SCAN_MODE_INTERLACE_FIELD;
            }
            break;

        default:
            eHalScanmode = E_MHAL_DIVP_SCAN_MODE_MAX;
            break;
    }

    return eHalScanmode;
}


static MHAL_DIVP_FieldType_e _mi_divp_TransSysFieldTypeToDivpHal(MI_SYS_FieldType_e eFieldType)
{
    MHAL_DIVP_FieldType_e eHalFieldType = E_MHAL_DIVP_FIELD_TYPE_NONE;

    switch(eFieldType)
    {
        case E_MI_SYS_FIELDTYPE_NONE://< no field.
             eHalFieldType = E_MHAL_DIVP_FIELD_TYPE_NONE;
             break;

        case E_MI_SYS_FIELDTYPE_TOP://< Top field only.
             eHalFieldType = E_MHAL_DIVP_FIELD_TYPE_TOP;
             break;

        case E_MI_SYS_FIELDTYPE_BOTTOM://< Bottom field only.
             eHalFieldType = E_MHAL_DIVP_FIELD_TYPE_BOTTOM;
             break;

        case E_MI_SYS_FIELDTYPE_BOTH://< Both fields.
             eHalFieldType = E_MHAL_DIVP_FIELD_TYPE_BOTH;
             break;

        default:
             eHalFieldType = E_MHAL_DIVP_FIELD_TYPE_NUM;
             break;
    }

    return eHalFieldType;
}

static void _mi_divp_TransVdecMfdecInfoToDivpHal(mi_vdec_DbInfo_t *pstDbInfo, MHAL_DIVP_MFdecInfo_t* pstHalMfdecInfo)
{
    pstHalMfdecInfo->bDbEnable = pstDbInfo->bDbEnable;           // Decoder Buffer Enable
    pstHalMfdecInfo->u8DbSelect = pstDbInfo->u8DbSelect;          // Decoder Buffer select
    pstHalMfdecInfo->stMirror.bHMirror = pstDbInfo->bHMirror;
    pstHalMfdecInfo->stMirror.bVMirror = pstDbInfo->bVMirror;
    pstHalMfdecInfo->bUncompressMode = pstDbInfo->bUncompressMode;
    pstHalMfdecInfo->bBypassCodecMode = pstDbInfo->bBypassCodecMode;
    pstHalMfdecInfo->u16StartX = ALIGN_DOWN(pstDbInfo->u16StartX,MI_DIVP_MFDEC_STARTX_ALIGNMENT);
    pstHalMfdecInfo->u16StartY = pstDbInfo->u16StartY;
    pstHalMfdecInfo->u16Width= ALIGN_UP(pstDbInfo->u16HSize, MI_DIVP_MFDEC_WIDTH_ALIGNMENT);
    pstHalMfdecInfo->u16Height = pstDbInfo->u16VSize;
    pstHalMfdecInfo->u64DbBaseAddr = pstDbInfo->phyDbBase;          // Decoder Buffer base addr
    pstHalMfdecInfo->u16DbPitch = pstDbInfo->u16DbPitch;         // Decoder Buffer pitch
    pstHalMfdecInfo->u8DbMiuSel = pstDbInfo->u8DbMiuSel;         // Decoder Buffer Miu select
    pstHalMfdecInfo->u64LbAddr = pstDbInfo->phyLbAddr;          // Lookaside buffer addr
    pstHalMfdecInfo->u8LbSize = pstDbInfo->u8LbSize;           // Lookaside buffer size
    pstHalMfdecInfo->u8LbTableId = pstDbInfo->u8LbTableId;        // Lookaside buffer table Id

    DBG_INFO("bDbEnable = %u, u8DbSelect = %u, phyDbBase = 0x%llx, u16DbPitch = %u, u8DbMiuSel = %u. phyLbAddr = 0x%llx .\n",
        pstDbInfo->bDbEnable, pstDbInfo->u8DbSelect, pstDbInfo->phyDbBase, pstDbInfo->u16DbPitch, pstDbInfo->u8DbMiuSel, pstDbInfo->phyLbAddr);

    // Decoder Buffer mode
    if(E_MI_VDEC_DB_MODE_H264_H265 == pstDbInfo->eDbMode)
    {
        pstHalMfdecInfo->eDbMode = E_MHAL_DIVP_DB_MODE_H264_H265;
    }
    else if(E_MI_VDEC_DB_MODE_VP9 == pstDbInfo->eDbMode)
    {
        pstHalMfdecInfo->eDbMode = E_MHAL_DIVP_DB_MODE_VP9;
    }
    else
    {
        pstHalMfdecInfo->eDbMode = E_MHAL_DIVP_DB_MODE_MAX;
    }
}

static void _mi_divp_TransMiCaptureInfoToHal(mi_divp_CaptureInfo_t* pstMiCapInfo, MHAL_DIVP_CaptureInfo_t* pstHalCapInfo)
{
    pstHalCapInfo->u64BufAddr[0] = pstMiCapInfo->phyBufAddr[0];
    pstHalCapInfo->u64BufAddr[1] = pstMiCapInfo->phyBufAddr[1];
    pstHalCapInfo->u64BufAddr[2] = pstMiCapInfo->phyBufAddr[2];
    pstHalCapInfo->u32BufSize = pstMiCapInfo->u32BufSize;
    pstHalCapInfo->u16Width = pstMiCapInfo->u16Width;
    pstHalCapInfo->u16Height = pstMiCapInfo->u16Height;
    pstHalCapInfo->u16Stride[0] = pstMiCapInfo->u16Stride[0];
    pstHalCapInfo->u16Stride[1] = pstMiCapInfo->u16Stride[1];
    pstHalCapInfo->u16Stride[2] = pstMiCapInfo->u16Stride[2];
    pstHalCapInfo->stMirror.bHMirror = (bool)(pstMiCapInfo->stMirror.bHMirror);
    pstHalCapInfo->stMirror.bVMirror = (bool)(pstMiCapInfo->stMirror.bVMirror);

    pstHalCapInfo->eInputPxlFmt = _mi_divp_TransSysPixelFormatToDivpHal(pstMiCapInfo->eInputPxlFmt);
    pstHalCapInfo->eOutputPxlFmt = _mi_divp_TransSysPixelFormatToDivpHal(pstMiCapInfo->eOutputPxlFmt);
    pstHalCapInfo->eDispId = _mi_divp_TransDispDeviceIdToDivpHal(pstMiCapInfo->eDispId);
    pstHalCapInfo->eCapStage = _mi_divp_TransMiCaptureStagrToHal(pstMiCapInfo->eCapStage);
    pstHalCapInfo->eRotate = _mi_divp_TransSysRotateToDivpHal(pstMiCapInfo->eRotate);
    _mi_divp_TransSysWindowToDivpHal(&pstMiCapInfo->stCropWin, &pstHalCapInfo->stCropWin);

    DBG_INFO("u32BufAddr = 0x%llx, u32BufSize = 0x%x, u16Width = %u, u16Height = %u\n",
        pstHalCapInfo->u64BufAddr[0], pstHalCapInfo->u32BufSize, pstHalCapInfo->u16Width, pstHalCapInfo->u16Height);
    DBG_INFO("u16Stride[0] = %u, u16Stride[1] = %u, u16Stride[2] = %u, eDispId = %d, eCapStage = %d\n",
        pstHalCapInfo->u16Stride[0], pstHalCapInfo->u16Stride[1],pstHalCapInfo->u16Stride[2],pstHalCapInfo->eDispId, pstHalCapInfo->eCapStage);
    DBG_INFO("eInputPxlFmt = %d, eOutputPxlFmt = %d, eRotateType = %d, bHMirror = %d, bVMirror = %d\n",
        pstHalCapInfo->eInputPxlFmt, pstHalCapInfo->eOutputPxlFmt, pstHalCapInfo->eRotate, pstHalCapInfo->stMirror.bHMirror, pstHalCapInfo->stMirror.bVMirror);
    DBG_INFO("crop(x, y, w, h) = (%u, %u, %u, %u). \n", pstHalCapInfo->stCropWin.u16X, pstHalCapInfo->stCropWin.u16Y, pstHalCapInfo->stCropWin.u16Width, pstHalCapInfo->stCropWin.u16Height);

}

static void _mi_divp_checkPrivateChnAttr(mi_divp_CaptureInfo_t* pstMiCapInfo)
{
    if(pstMiCapInfo->eRotate != gstDivpChnCtx[DIVP_CHN_MAX].stChnAttrPre.eRotateType)
    {
        gstDivpChnCtx[DIVP_CHN_MAX].stChnAttr.eRotateType = pstMiCapInfo->eRotate;
        gstDivpChnCtx[DIVP_CHN_MAX].bChnAttrChg = TRUE;
    }

    if((pstMiCapInfo->stCropWin.u16X != gstDivpChnCtx[DIVP_CHN_MAX].stChnAttrPre.stCropRect.u16X)
        || (pstMiCapInfo->stCropWin.u16Y != gstDivpChnCtx[DIVP_CHN_MAX].stChnAttrPre.stCropRect.u16Y)
        || (pstMiCapInfo->stCropWin.u16Width != gstDivpChnCtx[DIVP_CHN_MAX].stChnAttrPre.stCropRect.u16Width)
        || (pstMiCapInfo->stCropWin.u16Height != gstDivpChnCtx[DIVP_CHN_MAX].stChnAttrPre.stCropRect.u16Height))
    {
        gstDivpChnCtx[DIVP_CHN_MAX].stChnAttr.stCropRect.u16X = pstMiCapInfo->stCropWin.u16X;
        gstDivpChnCtx[DIVP_CHN_MAX].stChnAttr.stCropRect.u16Y = pstMiCapInfo->stCropWin.u16Y;
        gstDivpChnCtx[DIVP_CHN_MAX].stChnAttr.stCropRect.u16Width = pstMiCapInfo->stCropWin.u16Width;
        gstDivpChnCtx[DIVP_CHN_MAX].stChnAttr.stCropRect.u16Height = pstMiCapInfo->stCropWin.u16Height;
        gstDivpChnCtx[DIVP_CHN_MAX].bChnAttrChg = TRUE;
    }

    if((pstMiCapInfo->stMirror.bHMirror != gstDivpChnCtx[DIVP_CHN_MAX].stChnAttrPre.bHorMirror)
        || (pstMiCapInfo->stMirror.bVMirror != gstDivpChnCtx[DIVP_CHN_MAX].stChnAttrPre.bVerMirror))
    {
        gstDivpChnCtx[DIVP_CHN_MAX].stChnAttr.bHorMirror = pstMiCapInfo->stMirror.bHMirror ;
        gstDivpChnCtx[DIVP_CHN_MAX].stChnAttr.bVerMirror = pstMiCapInfo->stMirror.bVMirror;
        gstDivpChnCtx[DIVP_CHN_MAX].bChnAttrChg = TRUE;
    }
}

static void _mi_divp_CheckInputWidthAlignment(MHAL_DIVP_InputInfo_t* pstDivpInputInfo)
{
    //input width and stride need align according to MIU_BUS_WIDTH
    MI_U16 u16InputWidth = 0;

    if(E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV == pstDivpInputInfo->ePxlFmt)
    {
        //align to 16 pixel, each pixel occupy 2 bytes
        if((pstDivpInputInfo->u16InputWidth % MI_DIVP_INPUT_WIDTH_ALIGNMENT_YUV422))
        {
            u16InputWidth = ALIGN_UP(pstDivpInputInfo->u16InputWidth, MI_DIVP_INPUT_WIDTH_ALIGNMENT_YUV422);
            if(u16InputWidth * 2 <= pstDivpInputInfo->u16Stride[0])//*2 means each pixels occupy 2 bytes
            {
                DBG_INFO("attention: input width %u align to %u.\n",pstDivpInputInfo->u16InputWidth, u16InputWidth);
                pstDivpInputInfo->u16InputWidth = u16InputWidth;
            }
        }
    }
    else if((E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420 == pstDivpInputInfo->ePxlFmt)
        || (E_MHAL_DIVP_PIXEL_FORMAT_YUV_MST_420 == pstDivpInputInfo->ePxlFmt)
        || (E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264 == pstDivpInputInfo->ePxlFmt)
        || (E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265 == pstDivpInputInfo->ePxlFmt)
        || (E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265 == pstDivpInputInfo->ePxlFmt))
    {
        //Y and YV use different address, need to align to 32 pixels
        if((pstDivpInputInfo->u16InputWidth % MI_DIVP_INPUT_WIDTH_ALIGNMENT_YUV420))
        {
            u16InputWidth = ALIGN_UP(pstDivpInputInfo->u16InputWidth, MI_DIVP_INPUT_WIDTH_ALIGNMENT_YUV420);
            if(u16InputWidth <= pstDivpInputInfo->u16Stride[0])
            {
                DBG_INFO("attention: input width %u align to %u.\n",pstDivpInputInfo->u16InputWidth, u16InputWidth);
                pstDivpInputInfo->u16InputWidth = u16InputWidth;
            }
        }
    }
}

static MI_BOOL _mi_divp_SetChnAttr(MI_DIVP_CHN DivpChn, MI_U16 u16InputWidth, MI_U16 u16InputHeight, MHAL_CMDQ_CmdqInterface_t* pstCmdInf)
{
    MI_BOOL bRet = FALSE;
    MI_BOOL bNeedResetNextTime = FALSE;
    MHAL_DIVP_DiType_e eHalDiType = E_MHAL_DIVP_DI_TYPE_NUM;
    MHAL_DIVP_TnrLevel_e eHalTnrLvl = E_MHAL_DIVP_TNR_LEVEL_NUM;
    MHAL_DIVP_Rotate_e  eHalRotate = E_MHAL_DIVP_ROTATE_NUM;
    MHAL_DIVP_Mirror_t stMirror;
    MI_DIVP_ChnAttr_t* pstAttr = NULL;
    memset(&stMirror, 0, sizeof(MHAL_DIVP_Mirror_t));

    pstAttr = &gstDivpChnCtx[DivpChn].stChnAttr;
    DBG_INFO("current attr: DivpChn = %d, bHorMirror = %d, bVerMirror = %d, eDiType = %d, eRotateType = %d, eTnrLevel = %d, \n  u32MaxWidth = %u, u32MaxHeight = %u, crop(x, y, w, h) = (%u, %u, %u, %u). \n",
        DivpChn, pstAttr->bHorMirror, pstAttr->bVerMirror, pstAttr->eDiType, pstAttr->eRotateType, pstAttr->eTnrLevel, pstAttr->u32MaxWidth,
        pstAttr->u32MaxHeight, pstAttr->stCropRect.u16X, pstAttr->stCropRect.u16Y, pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);

    pstAttr = &gstDivpChnCtx[DivpChn].stChnAttrPre;
    DBG_INFO("pre attr: DivpChn = %d, bHorMirror = %d, bVerMirror = %d, eDiType = %d, eRotateType = %d, eTnrLevel = %d, \n  u32MaxWidth = %u, u32MaxHeight = %u, crop(x, y, w, h) = (%u, %u, %u, %u). \n",
        DivpChn, pstAttr->bHorMirror, pstAttr->bVerMirror, pstAttr->eDiType, pstAttr->eRotateType, pstAttr->eTnrLevel, pstAttr->u32MaxWidth,
        pstAttr->u32MaxHeight, pstAttr->stCropRect.u16X, pstAttr->stCropRect.u16Y, pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);


    pstAttr = &gstDivpChnCtx[DivpChn].stChnAttrOrg;
    DBG_INFO("orignal attr: DivpChn = %d, bHorMirror = %d, bVerMirror = %d, eDiType = %d, eRotateType = %d, eTnrLevel = %d, \n  u32MaxWidth = %u, u32MaxHeight = %u, crop(x, y, w, h) = (%u, %u, %u, %u). \n",
        DivpChn, pstAttr->bHorMirror, pstAttr->bVerMirror, pstAttr->eDiType, pstAttr->eRotateType, pstAttr->eTnrLevel, pstAttr->u32MaxWidth,
        pstAttr->u32MaxHeight, pstAttr->stCropRect.u16X, pstAttr->stCropRect.u16Y, pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);

    if(gstDivpChnCtx[DivpChn].bChnAttrChg || gbResetAllAttr[DivpChn])
    {
        if((gstDivpChnCtx[DivpChn].stChnAttr.eDiType != gstDivpChnCtx[DivpChn].stChnAttrPre.eDiType) || gbResetAllAttr[DivpChn])
        {
            eHalDiType = _mi_divp_TransMiDiTypeToHal(gstDivpChnCtx[DivpChn].stChnAttr.eDiType);
            if(MHAL_SUCCESS == MHAL_DIVP_SetAttr(gstDivpChnCtx[DivpChn].pHalCtx, E_MHAL_DIVP_ATTR_DI, &eHalDiType, pstCmdInf))
            {
                gstDivpChnCtx[DivpChn].stChnAttrPre.eDiType = gstDivpChnCtx[DivpChn].stChnAttr.eDiType;
                bRet = TRUE;
            }
            else
            {
                bNeedResetNextTime = TRUE;
                DBG_ERR(" setAttr fail!attr type = E_MHAL_DIVP_ATTR_DI, attr value = %u\n",
                    gstDivpChnCtx[DivpChn].stChnAttr.eDiType);
            }
        }

        if((gstDivpChnCtx[DivpChn].stChnAttr.eTnrLevel != gstDivpChnCtx[DivpChn].stChnAttrPre.eTnrLevel) || gbResetAllAttr[DivpChn])
        {
            eHalTnrLvl = _mi_divp_TransMiTnrLevelToHal(gstDivpChnCtx[DivpChn].stChnAttr.eTnrLevel);
            if(MHAL_SUCCESS == MHAL_DIVP_SetAttr(gstDivpChnCtx[DivpChn].pHalCtx, E_MHAL_DIVP_ATTR_TNR, &eHalTnrLvl, pstCmdInf))
            {
                gstDivpChnCtx[DivpChn].stChnAttrPre.eTnrLevel = gstDivpChnCtx[DivpChn].stChnAttr.eTnrLevel;
                bRet = TRUE;
            }
            else
            {
                bNeedResetNextTime = TRUE;
                DBG_ERR(" setAttr fail!attr type = E_MHAL_DIVP_ATTR_TNR, attr value = %u\n",
                    gstDivpChnCtx[DivpChn].stChnAttr.eTnrLevel);
            }
        }

        if((u16InputWidth <= MI_DIVP_TIMING_WIDTH_2K)//rotate support max size 2K2K
            && (u16InputHeight <= MI_DIVP_TIMING_HEIGHT_2K))
        {
            eHalRotate = _mi_divp_TransSysRotateToDivpHal(gstDivpChnCtx[DivpChn].stChnAttr.eRotateType);
            if((gstDivpChnCtx[DivpChn].stChnAttr.eRotateType != gstDivpChnCtx[DivpChn].stChnAttrPre.eRotateType) || gbResetAllAttr[DivpChn])
            {
                if(MHAL_SUCCESS == MHAL_DIVP_SetAttr(gstDivpChnCtx[DivpChn].pHalCtx, E_MHAL_DIVP_ATTR_ROTATE, &eHalRotate, pstCmdInf))
                {
                    gstDivpChnCtx[DivpChn].stChnAttrPre.eRotateType = gstDivpChnCtx[DivpChn].stChnAttr.eRotateType;
                    bRet = TRUE;
                }
                else
                {
                    bNeedResetNextTime = TRUE;
                    DBG_ERR(" setAttr fail!attr type = E_MHAL_DIVP_ATTR_ROTATE, attr value = %u\n", gstDivpChnCtx[DivpChn].stChnAttr.eRotateType);
                }
            }
        }

        if((gstDivpChnCtx[DivpChn].stChnAttr.bHorMirror != gstDivpChnCtx[DivpChn].stChnAttrPre.bHorMirror)
            || (gstDivpChnCtx[DivpChn].stChnAttr.bVerMirror != gstDivpChnCtx[DivpChn].stChnAttrPre.bVerMirror)
            || gbResetAllAttr[DivpChn])
        {
            stMirror.bHMirror = gstDivpChnCtx[DivpChn].stChnAttr.bHorMirror;
            stMirror.bVMirror = gstDivpChnCtx[DivpChn].stChnAttr.bVerMirror;
            if(MHAL_SUCCESS == MHAL_DIVP_SetAttr(gstDivpChnCtx[DivpChn].pHalCtx, E_MHAL_DIVP_ATTR_MIRROR, &stMirror, pstCmdInf))
            {
                gstDivpChnCtx[DivpChn].stChnAttrPre.bHorMirror = gstDivpChnCtx[DivpChn].stChnAttr.bHorMirror;
                gstDivpChnCtx[DivpChn].stChnAttrPre.bVerMirror = gstDivpChnCtx[DivpChn].stChnAttr.bVerMirror;
                bRet = TRUE;
            }
            else
            {
                bNeedResetNextTime = TRUE;
                DBG_ERR(" setAttr fail!attr type = E_MHAL_DIVP_ATTR_MIRROR, Mirror(H, V) = (%u, %u)\n",
                    gstDivpChnCtx[DivpChn].stChnAttr.bHorMirror, gstDivpChnCtx[DivpChn].stChnAttr.bVerMirror);
            }
        }

        if((gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X != gstDivpChnCtx[DivpChn].stChnAttrPre.stCropRect.u16X)
            || (gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y != gstDivpChnCtx[DivpChn].stChnAttrPre.stCropRect.u16Y)
            || (gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width != gstDivpChnCtx[DivpChn].stChnAttrPre.stCropRect.u16Width)
            || (gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height != gstDivpChnCtx[DivpChn].stChnAttrPre.stCropRect.u16Height)
            || gbResetAllAttr[DivpChn])
        {
            if(MHAL_SUCCESS == MHAL_DIVP_SetAttr(gstDivpChnCtx[DivpChn].pHalCtx, E_MHAL_DIVP_ATTR_CROP,
                                                           &gstDivpChnCtx[DivpChn].stChnAttr.stCropRect, pstCmdInf))
            {
                memcpy(&gstDivpChnCtx[DivpChn].stChnAttrPre.stCropRect, &gstDivpChnCtx[DivpChn].stChnAttr.stCropRect, sizeof(MI_SYS_WindowRect_t));
                bRet = TRUE;
            }
            else
            {
                bNeedResetNextTime = TRUE;
                DBG_ERR(" setAttr fail!attr type = E_MHAL_DIVP_ATTR_CROP, crop(x, y, W, H) = (%u, %u, %u, %u)\n",
                    gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y,
                    gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height);
            }
        }
    }

    if(bNeedResetNextTime)
    {
        gstDivpChnCtx[DivpChn].bChnAttrChg = TRUE;
    }
    else
    {
        gstDivpChnCtx[DivpChn].bChnAttrChg = FALSE;
    }

    //reset crop window info: crop window maybe change by sideband message.
    gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X = gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16X;
    gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y = gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Y;
    gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width = gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Width;
    gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height = gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Height;

    //reset variable of updating all attribute for next frame.
    gbResetAllAttr[DivpChn] = FALSE;

    return bRet;
}

static MI_BOOL _mi_divp_GetDiSettings(MI_DIVP_CHN DivpChn, MHAL_DIVP_InputInfo_t* pstDivpInputInfo)
{
    MI_BOOL bRet = TRUE;
    MI_DIVP_ChnAttr_t* pstAttr = NULL;

    if(E_MHAL_DIVP_FIELD_TYPE_BOTH == pstDivpInputInfo->stDiSettings.eFieldType)
    {
        //fix me: need get value from input source.
    }

    if(E_MI_DIVP_DI_TYPE_3D == gstDivpChnCtx[DivpChn].stChnAttr.eDiType)
    {
        if(memcmp(&gstDivpChnCtx[DivpChn].stChnAttr, &gstDivpChnCtx[DivpChn].stChnAttrPre, sizeof(MI_DIVP_ChnAttr_t)))//attribute change
        {
            pstAttr = &gstDivpChnCtx[DivpChn].stChnAttr;
            DBG_INFO("current attr: DivpChn = %d, bHorMirror = %d, bVerMirror = %d, eDiType = %d, eRotateType = %d, eTnrLevel = %d, \n  u32MaxWidth = %u, u32MaxHeight = %u, crop(x, y, w, h) = (%u, %u, %u, %u). \n",
                DivpChn, pstAttr->bHorMirror, pstAttr->bVerMirror, pstAttr->eDiType, pstAttr->eRotateType, pstAttr->eTnrLevel, pstAttr->u32MaxWidth,
                pstAttr->u32MaxHeight, pstAttr->stCropRect.u16X, pstAttr->stCropRect.u16Y, pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);

            pstAttr = &gstDivpChnCtx[DivpChn].stChnAttrPre;
            DBG_INFO("pre attr: DivpChn = %d, bHorMirror = %d, bVerMirror = %d, eDiType = %d, eRotateType = %d, eTnrLevel = %d, \n  u32MaxWidth = %u, u32MaxHeight = %u, crop(x, y, w, h) = (%u, %u, %u, %u). \n",
                DivpChn, pstAttr->bHorMirror, pstAttr->bVerMirror, pstAttr->eDiType, pstAttr->eRotateType, pstAttr->eTnrLevel, pstAttr->u32MaxWidth,
                pstAttr->u32MaxHeight, pstAttr->stCropRect.u16X, pstAttr->stCropRect.u16Y, pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);
            pstDivpInputInfo->stDiSettings.eDiMode = E_MHAL_DIVP_DI_MODE_BOB;
            gstDivpChnCtx[DivpChn].eaFieldType[0] = E_MI_SYS_FIELDTYPE_NONE;
            gstDivpChnCtx[DivpChn].eaFieldType[1] = E_MI_SYS_FIELDTYPE_NONE;
        }
        else
        {
            if(((E_MHAL_DIVP_FIELD_TYPE_TOP == _mi_divp_TransSysFieldTypeToDivpHal(gstDivpChnCtx[DivpChn].eaFieldType[0]))
                    && (E_MHAL_DIVP_FIELD_TYPE_BOTTOM == _mi_divp_TransSysFieldTypeToDivpHal(gstDivpChnCtx[DivpChn].eaFieldType[1]))
                    && (E_MHAL_DIVP_FIELD_TYPE_TOP == pstDivpInputInfo->stDiSettings.eFieldType))
                || ((E_MHAL_DIVP_FIELD_TYPE_BOTTOM == _mi_divp_TransSysFieldTypeToDivpHal(gstDivpChnCtx[DivpChn].eaFieldType[0]))
                    && (E_MHAL_DIVP_FIELD_TYPE_TOP == _mi_divp_TransSysFieldTypeToDivpHal(gstDivpChnCtx[DivpChn].eaFieldType[1]))
                    && (E_MHAL_DIVP_FIELD_TYPE_BOTTOM == pstDivpInputInfo->stDiSettings.eFieldType)))
            {
                pstDivpInputInfo->stDiSettings.eDiMode = E_MHAL_DIVP_DI_MODE_WAVE;
            }
            else
            {
                pstDivpInputInfo->stDiSettings.eDiMode = E_MHAL_DIVP_DI_MODE_BOB;
            }
        }
    }
    else if(E_MI_DIVP_DI_TYPE_2D == gstDivpChnCtx[DivpChn].stChnAttr.eDiType)
    {
        pstDivpInputInfo->stDiSettings.eDiMode = E_MHAL_DIVP_DI_MODE_EODI;
    }
    else//DI off
    {
        pstDivpInputInfo->stDiSettings.eDiMode = E_MHAL_DIVP_DI_MODE_MAX;
    }

    //save field type
    gstDivpChnCtx[DivpChn].eaFieldType[0] = gstDivpChnCtx[DivpChn].eaFieldType[1];
    gstDivpChnCtx[DivpChn].eaFieldType[1] = pstDivpInputInfo->stDiSettings.eFieldType;

    return bRet;
}

static void _MI_DIVP_ProcessTaskSideBandMsg(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_U64 u64SideBandMsg = 0;
    MI_SYS_PixelFormat_e ePixFmt = E_MI_SYS_PIXEL_FRAME_FORMAT_MAX;
    MI_SYS_BufInfo_t *pstOutputPortBufInfo= NULL;
    MI_U32  u32XZoomInRatio = 0;
    MI_U32  u32YZoomInRatio = 0;
    MI_U32  u32WidthZoomInRatio = 0;
    MI_U32  u32HeightZoomInRatio = 0;
    MI_SYS_WindowRect_t* pstCropRect = &(gstDivpChnCtx[pstChnTask->u32ChnId].stChnAttr.stCropRect);
    MI_U64 u64Type = 0;

    u64SideBandMsg = pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->u64SidebandMsg;
    pstOutputPortBufInfo = pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID];
    u64Type = MI_SYS_GET_SIDEBAND_MSG_TYPE(u64SideBandMsg);

    switch(MI_SYS_GET_SIDEBAND_MSG_TYPE(u64SideBandMsg))
    {
       case MI_SYS_SIDEBAND_MSG_TYPE_PREFER_CROP_RECT:
            MI_SYS_GET_PREFER_CROP_MSG_DAT(u64SideBandMsg, u32XZoomInRatio,
                                      u32YZoomInRatio, u32WidthZoomInRatio, u32HeightZoomInRatio, ePixFmt);

            if((u32XZoomInRatio +  u32WidthZoomInRatio > pstOutputPortBufInfo->stFrameData.u16Width)
                   || (u32YZoomInRatio + u32HeightZoomInRatio > pstOutputPortBufInfo->stFrameData.u16Height)
                   || (0 == u32WidthZoomInRatio)
                   || (0 == u32HeightZoomInRatio)
                   || (ePixFmt != E_MI_SYS_PIXEL_FRAME_YUV422_YUYV && ePixFmt != E_MI_SYS_PIXEL_FRAME_YUV_MST_420)
                   || (ePixFmt == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV && pstOutputPortBufInfo->stFrameData.ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV422_YUYV))
            {
                DBG_ERR("Invalid divp sideband Msg{crop:x%u.y%u.w%u.h%u,fmt:%u} from chn%u, output buf{size:w%u.h%u, fmt:%u}\n",
                    u32XZoomInRatio, u32YZoomInRatio, u32WidthZoomInRatio, u32HeightZoomInRatio,ePixFmt, pstChnTask->u32ChnId,
                    pstOutputPortBufInfo->stFrameData.u16Width, pstOutputPortBufInfo->stFrameData.u16Height, pstOutputPortBufInfo->stFrameData.ePixelFormat);
            }
            else
            {
                DBG_INFO("divp sideband Msg{crop:x%u.y%u.w%u.h%u,fmt:%u} from chn%u, output buf{size:w%u.h%u, fmt:%u}\n",
                    u32XZoomInRatio, u32YZoomInRatio, u32WidthZoomInRatio, u32HeightZoomInRatio,ePixFmt, pstChnTask->u32ChnId,
                    pstOutputPortBufInfo->stFrameData.u16Width, pstOutputPortBufInfo->stFrameData.u16Height, pstOutputPortBufInfo->stFrameData.ePixelFormat);

                //standnize crop
                MI_SYS_BUG_ON((pstOutputPortBufInfo->eBufType != E_MI_SYS_BUFDATA_FRAME)
                 || (pstOutputPortBufInfo->stFrameData.u16Width == 0) || (pstOutputPortBufInfo->stFrameData.u16Height == 0));

                u32XZoomInRatio = (u32XZoomInRatio * MI_DIVP_ZOOM_RATION_DEN) / pstOutputPortBufInfo->stFrameData.u16Width;
                u32YZoomInRatio = (u32YZoomInRatio * MI_DIVP_ZOOM_RATION_DEN) / pstOutputPortBufInfo->stFrameData.u16Height;
                u32WidthZoomInRatio = (u32WidthZoomInRatio * MI_DIVP_ZOOM_RATION_DEN) / pstOutputPortBufInfo->stFrameData.u16Width;
                u32HeightZoomInRatio = (u32HeightZoomInRatio * MI_DIVP_ZOOM_RATION_DEN) / pstOutputPortBufInfo->stFrameData.u16Height;

                DBG_INFO("crop src :x%u.y%u.w%u.h%u.\n",
                    pstCropRect->u16X, pstCropRect->u16Y, pstCropRect->u16Width, pstCropRect->u16Height);
                //map output size to input size
                pstCropRect->u16X += (MI_U16)((MI_U32)u32XZoomInRatio * pstCropRect->u16Width / MI_DIVP_ZOOM_RATION_DEN);
                pstCropRect->u16Y += (MI_U16)((MI_U32)u32YZoomInRatio * pstCropRect->u16Height / MI_DIVP_ZOOM_RATION_DEN);
                pstCropRect->u16Width = (MI_U16)((MI_U32)u32WidthZoomInRatio * pstCropRect->u16Width / MI_DIVP_ZOOM_RATION_DEN);
                pstCropRect->u16Height = (MI_U16)((MI_U32)u32HeightZoomInRatio * pstCropRect->u16Height / MI_DIVP_ZOOM_RATION_DEN);

                pstCropRect->u16X = ALIGN_DOWN(pstCropRect->u16X, DIVP_CROP_ALIGNMENT);
                pstCropRect->u16Y = ALIGN_DOWN(pstCropRect->u16Y, DIVP_CROP_ALIGNMENT);
                pstCropRect->u16Width = ALIGN_DOWN(pstCropRect->u16Width, DIVP_CROP_ALIGNMENT);
                pstCropRect->u16Height = ALIGN_DOWN(pstCropRect->u16Height, DIVP_CROP_ALIGNMENT);
                DBG_INFO("crop dst :x%u.y%u.w%u.h%u.\n",
                    pstCropRect->u16X, pstCropRect->u16Y, pstCropRect->u16Width, pstCropRect->u16Height);

                //force switch target format from YUYV to MST YUV420 to save BW since DISP don't need to use GE sclaing anymore..
                //Do we have better solution?
                pstOutputPortBufInfo->stFrameData.ePixelFormat = ePixFmt;

                MI_SYS_ACK_SIDEBAND_MSG(pstOutputPortBufInfo->u64SidebandMsg);
            }
            break;
        case MI_SYS_SIDEBAND_MSG_TYPE_NULL:
            if (pstOutputPortBufInfo->stFrameData.ePixelFormat != gstDivpChnCtx[pstChnTask->u32ChnId].stOutputPortAttr.ePixelFormat)
            {
                pstOutputPortBufInfo->stFrameData.ePixelFormat = gstDivpChnCtx[pstChnTask->u32ChnId].stOutputPortAttr.ePixelFormat;
            }
            break;
       default:
            break;
    }
}

static MI_S32 _mi_divp_SwCrop(MI_DIVP_CHN DivpChn, MI_BOOL bCompressMode, MHAL_DIVP_InputInfo_t* pstDivpInputInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MHAL_DIVP_Window_t stCapWin;
    MI_U16 u16StartX = 0, u16StartY = 0, u16EndX = 0, u16EndY = 0;
    memset(&stCapWin, 0, sizeof(MHAL_DIVP_Window_t));
    MI_DIVP_CHECK_POINTER(pstDivpInputInfo);

    DBG_INFO("DivpChn = %d, bCompressMode = %u, bChnAttrChg = %u.\n", DivpChn, bCompressMode, gstDivpChnCtx[DivpChn].bChnAttrChg);
    DBG_INFO("current crop window (%u, %u, %u, %u)\n", gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height);
    DBG_INFO("original crop window (%u, %u, %u, %u)\n", gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16X,
        gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Y, gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Width,
        gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Height);

     if((gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X ==0)
         && (gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y ==0)
         && (gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width ==0)
         && (gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height ==0)
         )
    {
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width = pstDivpInputInfo->u16InputWidth;
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height = pstDivpInputInfo->u16InputHeight;
    }
         
    //if SW crop is enabled, DIPR's capture window's H_start and V_start should be align to 64 pixels, H_size and V_size shoud be align to 32 pixel.
    if((gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X >= MI_DIVP_SW_CROP_START_ALIGNMENT)
        || (gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y >= MI_DIVP_SW_CROP_START_ALIGNMENT)
        || (pstDivpInputInfo->u16InputWidth - gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X
                - gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width >= MI_DIVP_SW_CROP_WIDTH_ALIGNMENT)
        || (pstDivpInputInfo->u16InputHeight - gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y
                - gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height >= MI_DIVP_SW_CROP_HEIGHT_ALIGNMENT))
    {
        ///1 calculate capture window according to crop window
        u16StartX = ALIGN_DOWN(gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X, MI_DIVP_SW_CROP_START_ALIGNMENT);
        u16StartY = ALIGN_DOWN(gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y, MI_DIVP_SW_CROP_START_ALIGNMENT);
        u16EndX = ALIGN_UP(gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X + gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, MI_DIVP_SW_CROP_WIDTH_ALIGNMENT);
        u16EndY = ALIGN_UP(gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y + gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height, MI_DIVP_SW_CROP_HEIGHT_ALIGNMENT);
        DBG_INFO("(u16StartX, u16StartY, u16EndX, u16EndY) =  (%u, %u, %u, %u)\n", u16StartX, u16StartY, u16EndX, u16EndY);

        stCapWin.u16X = u16StartX;
        stCapWin.u16Y = u16StartY;
        stCapWin.u16Width = MIN((pstDivpInputInfo->u16InputWidth - u16StartX), (u16EndX - u16StartX));
        stCapWin.u16Height = MIN((pstDivpInputInfo->u16InputHeight - u16StartY), (u16EndY - u16StartY));

        DBG_INFO("SW capture window (%u, %u, %u, %u)\n", stCapWin.u16X, stCapWin.u16Y, stCapWin.u16Width, stCapWin.u16Height);

        if(bCompressMode)
        {
            pstDivpInputInfo->stMfdecInfo.u16StartX +=ALIGN_DOWN(stCapWin.u16X,MI_DIVP_MFDEC_STARTX_ALIGNMENT);
            pstDivpInputInfo->stMfdecInfo.u16StartY += stCapWin.u16Y;
            pstDivpInputInfo->stMfdecInfo.u16Height = stCapWin.u16Height;
            pstDivpInputInfo->stMfdecInfo.u16Width = ALIGN_UP(stCapWin.u16Width,MI_DIVP_MFDEC_WIDTH_ALIGNMENT);
        }
        else
        {
            //the algorithm of calculating the base address is provided by HW RD tommy.wang
            //*** attention please: Unit of address in formula is MIU_BUS_WIDTH. Unit of dipr_line_offset is pixel. ***
            //phyaddr = MIU_BUS_WIDTH * capture_window_address
            if(E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV == pstDivpInputInfo->ePxlFmt)
            {
                //16 is  pixel number in each MIU_BUS_WIDTH when MIU_BUS_WIDTH is 32 bytes.
                //Capture window base address = original base address + v_start_point * dipr_line_offset/16 + (h_start_point/16)
                pstDivpInputInfo->u64BufAddr[0] = pstDivpInputInfo->u64BufAddr[0]
                                                                + stCapWin.u16Y * pstDivpInputInfo->u16Stride[0] + stCapWin.u16X * 2;
            }
            else if(E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420 == pstDivpInputInfo->ePxlFmt)
            {
                //Capture window Y base address = original base address + v_start_point * dipr_line_offset/32 + (h_start_point/32)
                //Capture window C base address = original base address + (v_start_point/2) * dipr_line_offset/32 + (h_start_point/32)
                pstDivpInputInfo->u64BufAddr[0] = pstDivpInputInfo->u64BufAddr[0]
                                                                + stCapWin.u16Y * pstDivpInputInfo->u16Stride[0]+ stCapWin.u16X;
                pstDivpInputInfo->u64BufAddr[1] = pstDivpInputInfo->u64BufAddr[1]
                                                                + stCapWin.u16Y / 2 * pstDivpInputInfo->u16Stride[1]+ stCapWin.u16X;
            }
            else if((E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264 == pstDivpInputInfo->ePxlFmt)
                || (E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265 == pstDivpInputInfo->ePxlFmt)
                || (E_MHAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265 == pstDivpInputInfo->ePxlFmt))
            {
                if(E_MHAL_DIVP_TILE_MODE_16x32 == pstDivpInputInfo->eTileMode)
                {
                    //Capture window Y base address = original base address + v_start_point * dipr_line_offset/32 + h_start_point
                    //Capture window C base address = original base address + (v_start_point/2) * dipr_line_offset/32 + h_start_point
                    pstDivpInputInfo->u64BufAddr[0] = pstDivpInputInfo->u64BufAddr[0]
                                                                    + stCapWin.u16Y * pstDivpInputInfo->u16Stride[0]+ stCapWin.u16X * 32;
                    pstDivpInputInfo->u64BufAddr[1] = pstDivpInputInfo->u64BufAddr[1]
                                                                    + stCapWin.u16Y / 2 * pstDivpInputInfo->u16Stride[1]+ stCapWin.u16X * 32;
                }
                else if(E_MHAL_DIVP_TILE_MODE_32x16 == pstDivpInputInfo->eTileMode)
                {
                    //Capture window Y base address = original base address + v_start_point * dipr_line_offset/32 + h_start_point/2
                    //Capture window C base address = original base address + (v_start_point/2) * dipr_line_offset/32 + h_start_point/2
                    pstDivpInputInfo->u64BufAddr[0] = pstDivpInputInfo->u64BufAddr[0]
                                                                    + stCapWin.u16Y * pstDivpInputInfo->u16Stride[0]+ stCapWin.u16X * 16;
                    pstDivpInputInfo->u64BufAddr[1] = pstDivpInputInfo->u64BufAddr[1]
                                                                    + stCapWin.u16Y / 2 * pstDivpInputInfo->u16Stride[1]+ stCapWin.u16X * 16;
                }
                else if(E_MHAL_DIVP_TILE_MODE_32x32 == pstDivpInputInfo->eTileMode)
                {
                    //Capture window Y base address = original base address + v_start_point * dipr_line_offset/32 + h_start_point
                    //Capture window C base address = original base address + (v_start_point/2) * dipr_line_offset/32 + h_start_point
                    pstDivpInputInfo->u64BufAddr[0] = pstDivpInputInfo->u64BufAddr[0]
                                                                    + stCapWin.u16Y * pstDivpInputInfo->u16Stride[0]+ stCapWin.u16X * 32;
                    pstDivpInputInfo->u64BufAddr[1] = pstDivpInputInfo->u64BufAddr[1]
                                                                    + stCapWin.u16Y / 2 * pstDivpInputInfo->u16Stride[1]+ stCapWin.u16X * 32;
                }
                else
                {
                    DBG_ERR("unsupport SW crop. eTileMode = %u.\n", pstDivpInputInfo->eTileMode);
                }
            }
            pstDivpInputInfo->u16InputWidth = stCapWin.u16Width;
            pstDivpInputInfo->u16InputHeight = stCapWin.u16Height;
        }

        if((gstDivpChnCtx[DivpChn].stChnAttrPre.stCropRect.u16X != gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X + stCapWin.u16X)
            || (gstDivpChnCtx[DivpChn].stChnAttrPre.stCropRect.u16Y != gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y + stCapWin.u16Y))
        {
            gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X -= stCapWin.u16X;
            gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y -= stCapWin.u16Y;
            gstDivpChnCtx[DivpChn].bChnAttrChg = TRUE;
        }
    }

    pstDivpInputInfo->stCropWin.u16X = gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X;
    pstDivpInputInfo->stCropWin.u16Y = gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y;
    pstDivpInputInfo->stCropWin.u16Width = gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width;
    pstDivpInputInfo->stCropWin.u16Height = gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height;

    DBG_INFO("s32Ret = 0x%x, bCompressMode = %u, bChnAttrChg = %u.\n", s32Ret, bCompressMode, gstDivpChnCtx[DivpChn].bChnAttrChg);
    DBG_INFO("current crop window (%u, %u, %u, %u).\n", gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height);

    return s32Ret;
}

static MI_S32 _mi_divp_GetInputInfoFromMetaData(MI_DIVP_CHN DivpChn, mi_vdec_DispFrame_t* pstVdecFrameInfo, MHAL_DIVP_InputInfo_t* pstDivpInputInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;

    pstDivpInputInfo->u64BufAddr[0] = pstVdecFrameInfo->stFrmInfo.phyLumaAddr;
    pstDivpInputInfo->u64BufAddr[1] = pstVdecFrameInfo->stFrmInfo.phyChromaAddr;
    pstDivpInputInfo->u64BufAddr[2] = INVALID_ADDRESS;
    pstDivpInputInfo->u16InputWidth = pstVdecFrameInfo->stFrmInfo.u16Width;
    pstDivpInputInfo->u16InputHeight = pstVdecFrameInfo->stFrmInfo.u16Height;
    pstDivpInputInfo->u16Stride[0] = pstVdecFrameInfo->stFrmInfo.u16Pitch;
    pstDivpInputInfo->u16Stride[1] = pstVdecFrameInfo->stFrmInfo.u16Pitch;
    pstDivpInputInfo->u16Stride[2] = pstVdecFrameInfo->stFrmInfo.u16Pitch;
    pstDivpInputInfo->u32BufSize = pstDivpInputInfo->u16InputHeight * pstVdecFrameInfo->stFrmInfo.u16Pitch * 3 / 2;

    pstDivpInputInfo->ePxlFmt = _mi_divp_TransSysPixelFormatToDivpHal(pstVdecFrameInfo->ePixelFrm);
    pstDivpInputInfo->eTileMode = _mi_divp_TransSysTileModeToDivpHal(pstVdecFrameInfo->stFrmInfoExt.eFrameTileMode);
    pstDivpInputInfo->eScanMode = _mi_divp_TransSysScanModeToDivpHal(pstVdecFrameInfo->stFrmInfo.eFieldType, pstVdecFrameInfo->stFrmInfoExt.eFrameScanMode);
    pstDivpInputInfo->stDiSettings.eFieldType = _mi_divp_TransSysFieldTypeToDivpHal(pstVdecFrameInfo->stFrmInfo.eFieldType);

    _mi_divp_TransVdecMfdecInfoToDivpHal(&pstVdecFrameInfo->stDbInfo, &pstDivpInputInfo->stMfdecInfo);///MFdec info

    DBG_INFO("eFieldType = %u. ePxlFmt = %u, eScanMode = %u, eTileMode = %u.\n",
        pstDivpInputInfo->stDiSettings.eFieldType, pstDivpInputInfo->ePxlFmt, pstDivpInputInfo->eScanMode, pstDivpInputInfo->eTileMode);
    DBG_INFO("u16InputWidth = %d. u16InputHeight = %u, u32BufSize = 0x%x, phyLumaAddr = 0x%llx, phyChromaAddr = 0x%llx.\n",
        pstDivpInputInfo->u16InputWidth, pstDivpInputInfo->u16InputHeight,  pstDivpInputInfo->u32BufSize,
        pstVdecFrameInfo->stFrmInfo.phyLumaAddr, pstVdecFrameInfo->stFrmInfo.phyChromaAddr);
    DBG_INFO("u64BufAddr[2] = 0x%llx, u16Stride[0] = %u, u16Stride[1] = %u, u16Stride[2] = %u, eDiMode = %u\n",
        pstDivpInputInfo->u64BufAddr[2], pstDivpInputInfo->u16Stride[0],pstDivpInputInfo->u16Stride[1],
        pstDivpInputInfo->u16Stride[2], pstDivpInputInfo->stDiSettings.eDiMode);

    ///check crop info
    DBG_INFO("source crop(x, y, w, h) = (%u, %u, %u, %u). \n",
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height);

    if((gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X > pstDivpInputInfo->u16InputWidth - pstVdecFrameInfo->stDispInfo.u16CropRight)
        || (gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y > pstDivpInputInfo->u16InputHeight - pstVdecFrameInfo->stDispInfo.u16CropBottom))
    {
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
        DBG_ERR("Crop window's start is out of input frame. crop(x, y, w, h) = (%u, %u, %u, %u). input frame size (w, h) = (%u, %u).\n",
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height,
        pstDivpInputInfo->u16InputWidth, pstDivpInputInfo->u16InputHeight);
    }
    else
    {
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X += pstVdecFrameInfo->stDispInfo.u16CropLeft;
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y += pstVdecFrameInfo->stDispInfo.u16CropTop;
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width = MIN(gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Width,
            pstDivpInputInfo->u16InputWidth - pstVdecFrameInfo->stDispInfo.u16CropRight - gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X);
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height = MIN(gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Height,
            pstDivpInputInfo->u16InputHeight - pstVdecFrameInfo->stDispInfo.u16CropBottom - gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y);
    }

    DBG_INFO("s32Ret = 0x%x, dest crop(x, y, w, h) = (%u, %u, %u, %u). \n", s32Ret,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height);

    mi_divp_dump_FrameInfo(pstDivpInputInfo, NULL);

    return s32Ret;
}

static MI_S32 _mi_divp_GetInputInfoFromFrameData(MI_DIVP_CHN DivpChn, MI_SYS_FrameData_t* pstFrameData, MHAL_DIVP_InputInfo_t* pstDivpInputInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;

    pstDivpInputInfo->u64BufAddr[0] = pstFrameData->phyAddr[0];
    pstDivpInputInfo->u64BufAddr[1] = pstFrameData->phyAddr[1];
    pstDivpInputInfo->u64BufAddr[2] = pstFrameData->phyAddr[2];

    pstDivpInputInfo->u16InputWidth = pstFrameData->u16Width;
    pstDivpInputInfo->u16InputHeight = pstFrameData->u16Height;
    pstDivpInputInfo->u16Stride[0] = pstFrameData->u32Stride[0];
    pstDivpInputInfo->u16Stride[1] = pstFrameData->u32Stride[1];
    pstDivpInputInfo->u16Stride[2] = pstFrameData->u32Stride[2];

    pstDivpInputInfo->ePxlFmt = _mi_divp_TransSysPixelFormatToDivpHal(pstFrameData->ePixelFormat);
    if((E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420 == pstDivpInputInfo->ePxlFmt)
            || (E_MHAL_DIVP_PIXEL_FORMAT_YUV_MST_420 == pstDivpInputInfo->ePxlFmt))
    {
        pstDivpInputInfo->u32BufSize = pstDivpInputInfo->u16InputHeight * pstDivpInputInfo->u16Stride[0] * 3 / 2;
    }
    else if(E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_422 == pstDivpInputInfo->ePxlFmt)
    {
        pstDivpInputInfo->u32BufSize = pstDivpInputInfo->u16InputHeight * pstDivpInputInfo->u16Stride[0] * 2;
    }
    else
    {
        pstDivpInputInfo->u32BufSize = pstDivpInputInfo->u16InputHeight * pstDivpInputInfo->u16Stride[0];
    }

    pstDivpInputInfo->eTileMode = _mi_divp_TransSysTileModeToDivpHal(pstFrameData->eTileMode);
    pstDivpInputInfo->eScanMode = _mi_divp_TransSysScanModeToDivpHal(pstFrameData->eFieldType, pstFrameData->eFrameScanMode);
    pstDivpInputInfo->stDiSettings.eFieldType = _mi_divp_TransSysFieldTypeToDivpHal(pstFrameData->eFieldType);

    DBG_INFO("eFieldType = %d. ePxlFmt = %u, eScanMode = %u, eTileMode = %d.\n",
        pstDivpInputInfo->stDiSettings.eFieldType, pstDivpInputInfo->ePxlFmt, pstDivpInputInfo->eScanMode, pstDivpInputInfo->eTileMode);
    DBG_INFO("u16InputWidth = %d. u16InputHeight = %u, u32BufSize = 0x%x, u64BufAddr[0] = 0x%llx, u64BufAddr[1] = 0x%llx.\n",
        pstDivpInputInfo->u16InputWidth, pstDivpInputInfo->u16InputHeight,  pstDivpInputInfo->u32BufSize,
        pstDivpInputInfo->u64BufAddr[0], pstDivpInputInfo->u64BufAddr[1]);
    DBG_INFO("u64BufAddr[2] = 0x%llx, u16Stride[0] = %u, u16Stride[1] = %u, u16Stride[2] = %u, eDiMode = %u\n",
        pstDivpInputInfo->u64BufAddr[2], pstDivpInputInfo->u16Stride[0],pstDivpInputInfo->u16Stride[1],
        pstDivpInputInfo->u16Stride[2], pstDivpInputInfo->stDiSettings.eDiMode);

    ///check crop info
    DBG_INFO("source crop(x, y, w, h) = (%u, %u, %u, %u). \n",
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height);

    if((gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X > pstDivpInputInfo->u16InputWidth)
        || (gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y > pstDivpInputInfo->u16InputHeight))
    {
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
        DBG_ERR("Crop window's start is out of input frame. crop(x, y, w, h) = (%u, %u, %u, %u). input frame size (w, h) = (%u, %u).\n",
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height,
        pstDivpInputInfo->u16InputWidth, pstDivpInputInfo->u16InputHeight);
    }
    else
    {
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width = MIN(gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Width,
            pstDivpInputInfo->u16InputWidth - gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X);
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height = MIN(gstDivpChnCtx[DivpChn].stChnAttrOrg.stCropRect.u16Height,
            pstDivpInputInfo->u16InputHeight - gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y);
    }

    DBG_INFO("s32Ret = 0x%x, dest crop(x, y, w, h) = (%u, %u, %u, %u). \n", s32Ret,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16X, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Y,
        gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, gstDivpChnCtx[DivpChn].stChnAttr.stCropRect.u16Height);

    return s32Ret;
}

static MI_S32 _mi_divp_GetInputInfo(mi_sys_ChnTaskInfo_t* pstTask, MHAL_DIVP_InputInfo_t* pstDivpInputInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;
    mi_vdec_DispFrame_t* pstVdecFrameInfo = NULL;
    MI_BOOL bCompressMode = FALSE;
    MI_DIVP_CHECK_POINTER(pstTask)
    MI_DIVP_CHECK_POINTER(pstDivpInputInfo)
    DBG_INFO("eBufType = %d.\n", pstTask->astInputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType);

    ///1 PTS
    pstDivpInputInfo->u64Pts = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->u64Pts;

    ///2 get info from input source
    if(E_MI_SYS_BUFDATA_FRAME == pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->eBufType)///VIF input
    {
        if((E_MI_SYS_COMPRESS_MODE_NONE != pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->stFrameData.eCompressMode)
            && (E_MI_SYS_COMPRESS_MODE_BUTT!= pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->stFrameData.eCompressMode))
        {
            bCompressMode = TRUE;
        }

        DBG_INFO(" eFieldType = %u, u64Pts = 0x%llx.\n", pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->stFrameData.eFieldType,
            pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->u64Pts);
        s32Ret = _mi_divp_GetInputInfoFromFrameData(pstTask->u32ChnId,
                                          &pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->stFrameData, pstDivpInputInfo);
#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
        {
            mi_divp_ChnContex_t *pstChnInfo = &gstDivpChnCtx[pstTask->u32ChnId];
            if(pstChnInfo->bCheckFrameID == TRUE)
            {
                PRINTF_PROC("ChnID %d, PTS = %lld\n",
                pstTask->u32ChnId, pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->u64Pts);
            }
        }
#endif
    }
    else if(E_MI_SYS_BUFDATA_META == pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->eBufType)//Vdec input
    {
        if(E_MI_MODULE_ID_VDEC == pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->stMetaData.eDataFromModule)
        {
            pstVdecFrameInfo = (mi_vdec_DispFrame_t*)pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->stMetaData.pVirAddr;

            if(pstVdecFrameInfo->stDbInfo.bDbEnable)
            {
                bCompressMode = TRUE;
            }
            pstDivpInputInfo->u32BufSize = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->stMetaData.u32Size;
            s32Ret = _mi_divp_GetInputInfoFromMetaData(pstTask->u32ChnId, pstVdecFrameInfo, pstDivpInputInfo);

#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
{
            mi_divp_ChnContex_t *pstChnInfo = &gstDivpChnCtx[pstTask->u32ChnId];
            if(pstChnInfo->bCheckFrameID == TRUE)
            {
                PRINTF_PROC("ChnID %d, H = %u, L = %u, PTS = %lld\n",
                pstTask->u32ChnId,
                pstVdecFrameInfo->stFrmInfo.u32IdH,pstVdecFrameInfo->stFrmInfo.u32IdL,
                    pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->u64Pts);
            }
}
#endif
            pstVdecFrameInfo = NULL;
        }
    }
    else if(E_MI_SYS_BUFDATA_RAW == pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->eBufType)
    {
        DBG_ERR("Invalid buffer data. buffer type is E_MI_SYS_BUFDATA_RAW!\n");
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
        BUG_ON(E_MI_SYS_BUFDATA_RAW == pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->eBufType);
    }

    ///3 save input info
    if((gstDivpChnCtx[pstTask->u32ChnId].stInputInfoPre.u16Width != pstDivpInputInfo->u16InputWidth)
        ||(gstDivpChnCtx[pstTask->u32ChnId].stInputInfoPre.u16Height != pstDivpInputInfo->u16InputHeight)
        ||(gstDivpChnCtx[pstTask->u32ChnId].stInputInfoPre.ePxlFmt != _mi_divp_TransDivpHalPixelFormatToSys(pstDivpInputInfo->ePxlFmt)))
    {
        gstDivpChnCtx[pstTask->u32ChnId].stInputInfoPre.u16Width = pstDivpInputInfo->u16InputWidth;
        gstDivpChnCtx[pstTask->u32ChnId].stInputInfoPre.u16Height = pstDivpInputInfo->u16InputHeight;
        gstDivpChnCtx[pstTask->u32ChnId].stInputInfoPre.ePxlFmt = pstDivpInputInfo->ePxlFmt;
        gstDivpChnCtx[pstTask->u32ChnId].bInputChange = TRUE;
        DBG_INFO("bInputChange = %u.\n", gstDivpChnCtx[pstTask->u32ChnId].bInputChange);
    }
    else
    {
        gstDivpChnCtx[pstTask->u32ChnId].bInputChange = FALSE;
    }

    if(MI_SUCCESS == s32Ret)
    {
        ///3 check I/P mode change and set DI parameters
        _mi_divp_CheckIPModeChange(pstTask->u32ChnId, pstDivpInputInfo);

        ///4 check sideband message and deal with sw crop
        _MI_DIVP_ProcessTaskSideBandMsg(pstTask);
        _mi_divp_SwCrop(pstTask->u32ChnId, bCompressMode, pstDivpInputInfo);

        ///5 check DI settings
        _mi_divp_GetDiSettings(pstTask->u32ChnId, pstDivpInputInfo);
    }

    pstDivpInputInfo->stCropWin.u16X = gstDivpChnCtx[pstTask->u32ChnId].stChnAttr.stCropRect.u16X;
    pstDivpInputInfo->stCropWin.u16Y = gstDivpChnCtx[pstTask->u32ChnId].stChnAttr.stCropRect.u16Y;
    pstDivpInputInfo->stCropWin.u16Width = gstDivpChnCtx[pstTask->u32ChnId].stChnAttr.stCropRect.u16Width;
    pstDivpInputInfo->stCropWin.u16Height = gstDivpChnCtx[pstTask->u32ChnId].stChnAttr.stCropRect.u16Height;

    _mi_divp_CheckInputWidthAlignment(pstDivpInputInfo);

    DBG_INFO("eFieldType = %d. ePxlFmt = %u, eScanMode = %u, eTileMode = %d. eDiType = %u, eFieldOrderType = %u.\n",
        pstDivpInputInfo->stDiSettings.eFieldType, pstDivpInputInfo->ePxlFmt, pstDivpInputInfo->eScanMode, pstDivpInputInfo->eTileMode,
        gstDivpChnCtx[pstTask->u32ChnId].stChnAttr.eDiType, pstDivpInputInfo->stDiSettings.eFieldOrderType);
    DBG_INFO("u16InputWidth = %d. u16InputHeight = %u, u32BufSize = 0x%x, u64BufAddr[0] = 0x%llx, u64BufAddr[1] = 0x%llx.\n",
        pstDivpInputInfo->u16InputWidth, pstDivpInputInfo->u16InputHeight,  pstDivpInputInfo->u32BufSize,
        pstDivpInputInfo->u64BufAddr[0], pstDivpInputInfo->u64BufAddr[1]);
    DBG_INFO("u64BufAddr[2] = 0x%llx, u16Stride[0] = %u, u16Stride[1] = %u, u16Stride[2] = %u, eDiMode = %u\n",
        pstDivpInputInfo->u64BufAddr[2], pstDivpInputInfo->u16Stride[0],pstDivpInputInfo->u16Stride[1],
        pstDivpInputInfo->u16Stride[2], pstDivpInputInfo->stDiSettings.eDiMode);

    DBG_INFO("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

static MI_S32 _mi_divp_GetOutputInfo(mi_sys_ChnTaskInfo_t* pstTask, MHAL_DIVP_OutPutInfo_t* pstDivpOutputInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_CHECK_POINTER(pstDivpOutputInfo)
    MI_DIVP_CHECK_POINTER(pstTask)
    DBG_INFO("eBufType = %d.\n", pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType);

    if(E_MI_SYS_BUFDATA_FRAME == pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType)///VIF input
    {
        pstDivpOutputInfo->u64BufAddr[0] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.phyAddr[0];
        pstDivpOutputInfo->u64BufAddr[1] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.phyAddr[1];
        pstDivpOutputInfo->u64BufAddr[2] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.phyAddr[2];
        pstDivpOutputInfo->u16OutputWidth = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u16Width;
        pstDivpOutputInfo->u16OutputHeight = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u16Height;
        pstDivpOutputInfo->u16Stride[0] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u32Stride[0];
        pstDivpOutputInfo->u16Stride[1] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u32Stride[1];
        pstDivpOutputInfo->u16Stride[2] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u32Stride[2];

        DBG_INFO("chn port_out_pxl_fmt = %u. buf_cfg_pxl_fmt = %u. buf_pxl_fmt = %u.\n", gstDivpChnCtx[pstTask->u32ChnId].stOutputPortAttr.ePixelFormat,
            pstTask->astOutputPortPerfBufConfig[DIVP_CHN_OUTPUT_PORT_ID].stFrameCfg.eFormat,
            pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.ePixelFormat);
        pstDivpOutputInfo->ePxlFmt = _mi_divp_TransSysPixelFormatToDivpHal(pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.ePixelFormat);
        if((E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420 == pstDivpOutputInfo->ePxlFmt)
            || (E_MHAL_DIVP_PIXEL_FORMAT_YUV_MST_420 == pstDivpOutputInfo->ePxlFmt))
        {
            pstDivpOutputInfo->u32BufSize = pstDivpOutputInfo->u16OutputHeight * pstDivpOutputInfo->u16Stride[0] * 3 / 2;
        }
        else if(E_MHAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_422 == pstDivpOutputInfo->ePxlFmt)
        {
            pstDivpOutputInfo->u32BufSize = pstDivpOutputInfo->u16OutputHeight * pstDivpOutputInfo->u16Stride[0] * 2;
        }
        else
        {
            pstDivpOutputInfo->u32BufSize = pstDivpOutputInfo->u16OutputHeight * pstDivpOutputInfo->u16Stride[0];
        }

        ///save output info
        if((gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.u16Width != pstDivpOutputInfo->u16OutputWidth)
            ||(gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.u16Height != pstDivpOutputInfo->u16OutputHeight)
            ||(gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.ePxlFmt != _mi_divp_TransDivpHalPixelFormatToSys(pstDivpOutputInfo->ePxlFmt)))
        {
            gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.u16Width = pstDivpOutputInfo->u16OutputWidth;
            gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.u16Height = pstDivpOutputInfo->u16OutputHeight;
            gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.ePxlFmt = pstDivpOutputInfo->ePxlFmt;
            gstDivpChnCtx[pstTask->u32ChnId].bOutputChange = TRUE;
            DBG_INFO("bOutputChange = %u.\n", gstDivpChnCtx[pstTask->u32ChnId].bOutputChange);

            if((gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.u16Width != gstDivpChnCtx[pstTask->u32ChnId].stOutputPortAttr.u32Width)
                        || (gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.u16Height != gstDivpChnCtx[pstTask->u32ChnId].stOutputPortAttr.u32Height))
            {
                DBG_WRN("User Set Output W=%d, H=%d OutputBuffer Info W=%d, H=%d\n",gstDivpChnCtx[pstTask->u32ChnId].stOutputPortAttr.u32Width,gstDivpChnCtx[pstTask->u32ChnId].stOutputPortAttr.u32Height,
                    gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.u16Width, gstDivpChnCtx[pstTask->u32ChnId].stOutputInfoPre.u16Height);
            }

        }
        else
        {
            gstDivpChnCtx[pstTask->u32ChnId].bOutputChange = FALSE;
        }
    }
    else if((E_MI_SYS_BUFDATA_RAW == pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType)
        || (E_MI_SYS_BUFDATA_META == pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType))//Vdec input
    {
        DBG_ERR(" DIVP can't support  output customized or raw data. buftype = %u.\n",
            pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType);

        s32Ret = MI_DIVP_ERR_CHN_NOT_SUPPORT;

        BUG_ON((E_MI_SYS_BUFDATA_RAW == pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType)
            || (E_MI_SYS_BUFDATA_META == pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType));
    }

    DBG_INFO("ePxlFmt = %u, u16OutputWidth = %u, u16OutputHeight = %u, u32BufSize = 0x%x, \n",
        pstDivpOutputInfo->ePxlFmt, pstDivpOutputInfo->u16OutputWidth, pstDivpOutputInfo->u16OutputHeight, pstDivpOutputInfo->u32BufSize);
    DBG_INFO("u64BufAddr[0] = 0x%llx, u64BufAddr[1] = 0x%llx, u64BufAddr[2] = 0x%llx, u16Stride[0] = %u, u16Stride[1] = %u, u16Stride[2] = %u,\n\n",
        pstDivpOutputInfo->u64BufAddr[0], pstDivpOutputInfo->u64BufAddr[1], pstDivpOutputInfo->u64BufAddr[2], pstDivpOutputInfo->u16Stride[0],pstDivpOutputInfo->u16Stride[1],pstDivpOutputInfo->u16Stride[2]);

    mi_divp_dump_FrameInfo(NULL, pstDivpOutputInfo);

    DBG_INFO("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

mi_sys_TaskIteratorCBAction_e mi_divp_TaskIteratorCallBack(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData)
{
    mi_sys_TaskIteratorCBAction_e eRet = MI_SYS_ITERATOR_SKIP_CONTINUTE;
    mi_divp_IteratorWorkInfo_t *workInfo = (mi_divp_IteratorWorkInfo_t *)pUsrData;

    MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(pstTaskInfo->u32ChnId);
    if(E_MI_DIVP_CHN_STARTED == gstDivpChnCtx[pstTaskInfo->u32ChnId].eStatus)
    {
        //set buffer info
        pstTaskInfo->astOutputPortPerfBufConfig[DIVP_CHN_OUTPUT_PORT_ID].eBufType = E_MI_SYS_BUFDATA_FRAME;
        pstTaskInfo->astOutputPortPerfBufConfig[DIVP_CHN_OUTPUT_PORT_ID].stFrameCfg.u16Width =
                                                             gstDivpChnCtx[pstTaskInfo->u32ChnId].stOutputPortAttr.u32Width;
        pstTaskInfo->astOutputPortPerfBufConfig[DIVP_CHN_OUTPUT_PORT_ID].stFrameCfg.u16Height =
                                                             gstDivpChnCtx[pstTaskInfo->u32ChnId].stOutputPortAttr.u32Height;
        pstTaskInfo->astOutputPortPerfBufConfig[DIVP_CHN_OUTPUT_PORT_ID].stFrameCfg.eFormat=
                                                             gstDivpChnCtx[pstTaskInfo->u32ChnId].stOutputPortAttr.ePixelFormat;

        if((gstDivpChnCtx[pstTaskInfo->u32ChnId].stChnAttr.eRotateType == E_MI_SYS_ROTATE_90)
            ||(gstDivpChnCtx[pstTaskInfo->u32ChnId].stChnAttr.eRotateType == E_MI_SYS_ROTATE_270))
        {
            if(pstTaskInfo->astInputPortBufInfo[0]->eBufType == E_MI_SYS_BUFDATA_FRAME)
            {
                MI_U16 *pu16width = &(pstTaskInfo->astInputPortBufInfo[0]->stFrameData.u16Width);
                MI_U16 *pu16height = &(pstTaskInfo->astInputPortBufInfo[0]->stFrameData.u16Height);

                *pu16width = ALIGN_UP(*pu16width, 64);
                *pu16height = ALIGN_UP(*pu16height, 64);
            }
        }
            
#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
            if(gbStopOneChnl[pstTaskInfo->u32ChnId] == TRUE)
            {
                MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(pstTaskInfo->u32ChnId);
                mi_sys_FinishAndReleaseTask(pstTaskInfo);
                return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
            }

            if(gstProcessOneChnl.bEnable == TRUE)
            {
                if(gstProcessOneChnl.u32ChnId != pstTaskInfo->u32ChnId)
                {
                    MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(pstTaskInfo->u32ChnId);
                    mi_sys_FinishAndReleaseTask(pstTaskInfo);
                    return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
                }
                else if(gstProcessOneChnl.u16AddtaskCnt == 1)
                {
                    MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(pstTaskInfo->u32ChnId);
                    mi_sys_FinishAndReleaseTask(pstTaskInfo);
                    return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
                }
            }

            gstDivpChnCtx[pstTaskInfo->u32ChnId].u64GetBufferCnt ++;
#endif

        if(mi_sys_PrepareTaskOutputBuf(pstTaskInfo) == MI_SUCCESS)
        {
            //check if leak of output buf
            if(NULL != pstTaskInfo->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID])
            {
                //pstTaskInfo->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->u64SidebandMsg = 0x20005A0500078028;///640x360 yuv422
                MI_DIVP_LOCK_TODO_COMMON_TASK_LIST();
#if ENABLE_DEBUG_FENCE
                _gau64ReceiveFrmNum[pstTaskInfo->u32ChnId]++;
                pstTaskInfo->u64RecieveTask = _gau64ReceiveFrmNum[pstTaskInfo->u32ChnId];
                pstTaskInfo->phyRecieveAddress = pstTaskInfo->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.phyAddr[0];
#endif

#if ENABLE_DEBUG_PERFORMANCE
                _gu64TaskNum ++;
                pstTaskInfo->u64DivpTaskId = _gu64TaskNum;
                DIVP_PERF_TIME(&pstTaskInfo->u64DivpGotTask);
                if(1 == _gu64TaskNum)
                {
                    DIVP_PERF_TIME(&_gu64StartTime);
                }
#endif
                list_add_tail(&pstTaskInfo->listChnTask, &divp_todo_common_task_list);
#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
                if(gstProcessOneChnl.bEnable == TRUE
                    && gstProcessOneChnl.u32ChnId == pstTaskInfo->u32ChnId)
                {
                    gstProcessOneChnl.u16AddtaskCnt ++;
                }

                gstDivpChnCtx[pstTaskInfo->u32ChnId].u64PrepareOutBufCnt ++;
#endif

                MI_DIVP_UNLOCK_TODO_COMMON_TASK_LIST();
                DBG_INFO("add divp_todo_common_task! \n");

                //we at most process 32 batches at one time
                if(++workInfo->totalAddedTask >= 32)
                {
                    eRet = MI_SYS_ITERATOR_ACCEPT_STOP;
                }
                else
                {
                    eRet = MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
                }
            }
            else
            {
                if(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[DIVP_CHN_OUTPUT_PORT_ID])
                {
                    MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(pstTaskInfo->u32ChnId);
                    mi_sys_FinishAndReleaseTask(pstTaskInfo);
                    return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
                }
                else
                {
                    eRet = MI_SYS_ITERATOR_SKIP_CONTINUTE;
                    //DBG_ERR("pstTaskInfo->astOutputPortBufInfo == NULL. \n");
                }
            }
        }
        else
        {
            //DBG_ERR("mi_sys_PrepareTaskOutputBuf failed. \n");
            mi_sys_RewindTask(pstTaskInfo);
        }
    }
    else if(E_MI_DIVP_CHN_STARTED > gstDivpChnCtx[pstTaskInfo->u32ChnId].eStatus)
    {
        eRet = MI_SYS_ITERATOR_SKIP_CONTINUTE;
    }
    MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(pstTaskInfo->u32ChnId);

    return eRet;
}

static MI_S32 _mi_divp_delay_free_task_with_refresh_support(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32ChnId = pstChnTask->u32ChnId;
    MI_DIVP_CHECK_POINTER(pstChnTask)
    BUG_ON(u32ChnId>=MI_DIVP_CHN_MAX_NUM);

    DBG_INFO("pstChnTask = %p .\n", pstChnTask);
    MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(pstChnTask->u32ChnId);
    if(gstDivpChnCtx[pstChnTask->u32ChnId].pstLastDelayFreeTask)
    {
        mi_sys_FinishAndReleaseTask(gstDivpChnCtx[pstChnTask->u32ChnId].pstLastDelayFreeTask);
        gstDivpChnCtx[pstChnTask->u32ChnId].pstLastDelayFreeTask = NULL;
    }
    MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(pstChnTask->u32ChnId);

    //flush &release output buf  in advance
    if(pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID])
    {
        DBG_INFO("mi_sys_FinishBuf OutputPortBufInfo = %p .\n", pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]);
        s32Ret = mi_sys_FinishBuf(pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]);
        if(s32Ret == MI_SUCCESS)
        {
            pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID] = NULL;
        }
        else
        {
            DBG_ERR("fail to finish output buffer.\n");
            MI_SYS_BUG();
        }
    }
    gstDivpChnCtx[pstChnTask->u32ChnId].pstLastDelayFreeTask = pstChnTask;

    return s32Ret;
}

extern MI_S32 mi_gfx_PendingDone(MI_U16 u16TargetFence);

static void _mi_divp_DebugGetBufCnt(mi_divp_ChnContex_t *pstChnContex)
{
    pstChnContex->u64FinishOutBufCnt ++;
    pstChnContex->u32FrameCntpers ++;
    if(jiffies - pstChnContex->u32FrameCntJiffies > HZ)
    {
        pstChnContex->u8PortFps = pstChnContex->u32FrameCntpers;
        pstChnContex->u32FrameCntpers = 0;
        pstChnContex->u32FrameCntJiffies = jiffies;
    }
}


int mi_divp_IsrProcessThread(void* data)
{
    MI_U16 u16Fence = 0;
    MI_U64 u64MiniSec = 0;
    struct timespec stTaskRunningTime;
    MHAL_CMDQ_CmdqInterface_t *pstCmdInf = (MHAL_CMDQ_CmdqInterface_t*)data;
#if defined(SUPPORT_DIVP_USE_GE_SCALING_UP)&&(SUPPORT_DIVP_USE_GE_SCALING_UP==1)
    LIST_HEAD(divp_isr_thread_post_ge_scalingup_task_list);
    MI_GFX_Open();
#endif
    /////////////////////////////////// sys channel task reserved variables useage ///////////////////////////////////
    //mi_sys_ChnTaskInfo_t.u32Reserved0 ==> DIVP task fence
    //mi_sys_ChnTaskInfo_t.u32Reserved1 ==> Not used
    //mi_sys_ChnTaskInfo_t.u64Reserved0 ==> RGN task fence
    //mi_sys_ChnTaskInfo_t.u64Reserved1 ==> task start time : seconds
    //mi_sys_ChnTaskInfo_t.u64Reserved2 ==> task start time : Nanoseconds
    //mi_sys_ChnTaskInfo_t.u64Reserved3 ==> task has been created script : seconds
    //mi_sys_ChnTaskInfo_t.u64Reserved4 ==> task has been created script : Nanoseconds
    //mi_sys_ChnTaskInfo_t.u64Reserved5 ==> Not used
    /////////////////////////////////// sys channel task reserved variables useage ///////////////////////////////////

    DBG_INFO(" mi_divp_IsrProcessThread working!\n");
    while(1)
    {
        struct list_head* pos, *n;

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
        interruptible_sleep_on_timeout(&divp_isr_waitqueue, msecs_to_jiffies(1));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
        wait_event_interruptible_timeout(divp_isr_waitqueue, FALSE, msecs_to_jiffies(1));
#endif

        pstCmdInf->MHAL_CMDQ_ReadDummyRegCmdq(pstCmdInf, &u16Fence);

        if(kthread_should_stop())
        {
            DBG_INFO(" mi_divp_IsrProcessThread stoped!\n");
            _mi_divp_ClearWorkingCommonTask();
            _mi_divp_ClearWorkingCaptureTask();
            return -1;
        }

        ///1 callback to display after capture screen done.
        MI_DIVP_LOCK_WORKING_CAPTURE_TASK_LIST();
        if(!list_empty_careful(&divp_working_capture_task_list))
        {
            mi_divp_CaptureInfo_t* pstCapInfo = NULL;
            list_for_each_safe(pos, n, &divp_working_capture_task_list)
            {
                pstCapInfo = container_of(pos, mi_divp_CaptureInfo_t, capture_list);
                if(_mi_divp_IsFenceLE(pstCapInfo->u32Fence, u16Fence))
                {
                    list_del(&pstCapInfo->capture_list);
                    //call back to display
                    pstCapInfo->pfnDispCallBack(pstCapInfo);
                }
#if 0
                else
                {
                    MI_U64 u64MiniSec = 0;
                    struct timespec stTaskRunningTime;
                    memset(&stTaskRunningTime, 0, sizeof(stTaskRunningTime));
                    do_posix_clock_monotonic_gettime(&stTaskRunningTime);

                    if(stTaskRunningTime.tv_sec >= pstCapInfo->u64Reserved0)
                    {
                        u64MiniSec = (stTaskRunningTime.tv_sec - pstCapInfo->u64Reserved0) * 1000
                               + stTaskRunningTime.tv_nsec / 1000 - pstCapInfo->u64Reserved1 / 1000;
                        if(u64MiniSec > 300)
                        {
                            DBG_ERR("capture screen cost more than %llu MS.\n", u64MiniSec);
                        }
                    }
                }
#endif
            }
        }
        MI_DIVP_UNLOCK_WORKING_CAPTURE_TASK_LIST();

        ///2 notify MI_SYS finish buffer
        MI_DIVP_LOCK_WORKING_COMMON_TASK_LIST();
        if(!list_empty_careful(&divp_working_common_task_list))
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            MI_BOOL bNeedReset = FALSE;
            list_for_each_safe(pos, n, &divp_working_common_task_list)
            {
                pstChnTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
                DBG_INFO(" u16Fence = %d. pstChnTask->u32Reserved0 = %u.\n", u16Fence, pstChnTask->u32Reserved0);
                if(_mi_divp_IsFenceLE(pstChnTask->u32Reserved0, u16Fence))
                {
                    list_del(&pstChnTask->listChnTask);
#if ENABLE_DEBUG_FENCE
                    _gau64FinishFrmNum[pstChnTask->u32ChnId]++;
                    pstChnTask->u64FinishTask = _gau64FinishFrmNum[pstChnTask->u32ChnId];
                    pstChnTask->phyFinishAddress = pstChnTask->astOutputPortBufInfo[0]->stFrameData.phyAddr[0];
                    printk("\n 1 CYS %u, %llu, %llx, %llu, %llx, %llu, %llx, %u, %u.\n",
                        pstChnTask->u32ChnId, pstChnTask->u64RecieveTask, pstChnTask->phyRecieveAddress,
                        pstChnTask->u64ProcessTask,  pstChnTask->phyProcessAddress,
                        pstChnTask->u64FinishTask,  pstChnTask->phyFinishAddress,
                        pstChnTask->u32Reserved0, u16Fence);
                    if((pstChnTask->u64ProcessTask != pstChnTask->u64FinishTask)
                        ||(pstChnTask->u64ProcessTask != pstChnTask->u64RecieveTask))
                    {
                        DBG_ERR("DIVP FENCE ERROR\n");
                    }
#endif

#if ENABLE_DEBUG_PERFORMANCE
                    pstChnTask->u64DivpIsrId = _gu64IsrNum;
                    pstChnTask->u64DivpIsrComming = _gu64IsrCommingTime;

                    DIVP_PERF_TIME(&pstChnTask->u64DivpFinishTask);
                    if((pstChnTask->u64DivpFinishTask - pstChnTask->u64DivpGotTask) > 20000)///xx ms
                    {
                        sprintf(_gchTraceInfo[_gu64TraceCnt], "\n TaskId: %llu, IsrId: %llu, totoal: %llu, start: %llu, CMDQ buf: %llu, create cmds: %llu, kickoff: %llu, IsrComming: %llu, finish: %llu.\n",
                            pstChnTask->u64DivpTaskId, pstChnTask->u64DivpIsrId,
                            (pstChnTask->u64DivpFinishTask - pstChnTask->u64DivpGotTask),//total
                            (pstChnTask->u64DivpProcessTaskStart - pstChnTask->u64DivpGotTask),//recieve task to start
                            (pstChnTask->u64DivpGotCmdqBuffer - pstChnTask->u64DivpProcessTaskStart),//get cmdq buffer
                            (pstChnTask->u64DivpCmdqKickOff - pstChnTask->u64DivpGotCmdqBuffer),//process task:create commands
                            (pstChnTask->u64DivpProcessTaskEnd - pstChnTask->u64DivpCmdqKickOff),//kick off cmdq
                            (pstChnTask->u64DivpIsrComming - pstChnTask->u64DivpProcessTaskEnd),//ISR comming
                            (pstChnTask->u64DivpFinishTask - pstChnTask->u64DivpIsrComming));
                        _gu64TraceCnt++;
                        if (_gu64TraceCnt >= DIVP_RECORD_TASK_NUM)
                        {
                            int i = 0;
                            DIVP_PERF_TIME(&_gu64EndTime);
                            DBG_ERR("total time: %llu ms, task number = %llu, average time of task: %llu us.\n",
                                (_gu64EndTime - _gu64StartTime)/1000, pstChnTask->u64DivpTaskId, (_gu64EndTime - _gu64StartTime)/pstChnTask->u64DivpTaskId);
                            for (i = 0; i < _gu64TraceCnt; i++)
                            {
                                printk("%s.\n", _gchTraceInfo[i]);
                            }
                            _gu64TraceCnt = 0;
                        }
                    }
#endif

#if defined(DIVP_SUPPORT_RGN)&&(DIVP_SUPPORT_RGN==1)
                    //notify region to deal with cover and OSD
                    if(MI_RGN_OK != mi_rgn_NotifyFenceDone(pstChnTask->u64Reserved0))
                    {
                        DBG_ERR(" mi_rgn_NotifyFenceDone failed. u64Fence = %llu.\n", pstChnTask->u64Reserved0);
                    }

#endif

#if defined (DIVP_RGN_OVERLAP_TEST) && (DIVP_RGN_OVERLAP_TEST == 1)
                    // rgn overlap test
                    mi_rgn_OverlayProcess(&pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData,
                                          E_MI_GFX_FMT_YUV422, pstChnTask->u32ChnId, DIVP_CHN_OUTPUT_PORT_ID, 1);
#endif
                    memset(&stTaskRunningTime, 0, sizeof(stTaskRunningTime));
                    do_posix_clock_monotonic_gettime(&stTaskRunningTime);
                    u64MiniSec = (stTaskRunningTime.tv_sec - pstChnTask->u64Reserved1) * 1000000
                           + stTaskRunningTime.tv_nsec / 1000 - div_u64(pstChnTask->u64Reserved2, 1000);
                    pstChnTask->u64Reserved5 =( (MI_U64)stTaskRunningTime.tv_sec)<<32|stTaskRunningTime.tv_nsec;

#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
{
                    MI_U32 u32ChnId = pstChnTask->u32ChnId;
                    if((-1 - gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskSumTime) <  u64MiniSec)
                    {
                        gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskSumTime = 0;
                        gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskMaxTime = 0;
                        gstDivpChnCtx[u32ChnId].u64ReleaseTaskCnt = 0;
                    }
                    else
                    {
                        if(u64MiniSec > gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskMaxTime)
                            gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskMaxTime = u64MiniSec;

                        gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskSumTime += u64MiniSec;
                        gstDivpChnCtx[u32ChnId].u64ReleaseTaskCnt ++;
                    }

}
#endif

#if defined(SUPPORT_DIVP_USE_GE_SCALING_UP)&&(SUPPORT_DIVP_USE_GE_SCALING_UP==1)
                   {
                        MI_U32 offsetx=0, offsety=0;
                        MI_GFX_Surface_t stSrc;
                        MI_GFX_Surface_t stDst;
                        MI_GFX_Rect_t stSrcRect;
                        MI_GFX_Rect_t stDstRect;
                        MI_U16 u16GEFence;
                        MI_S32 ret;
                        static MI_S32 s32UclibcBug = 0;
                        if (0 == s32UclibcBug)
                        {
                            printk("@@@@@SUPPORT_DIVP_USE_GE_SCALING_UP==1@@@@@\n");
                            s32UclibcBug = 1;
                        }
                       CHECK_DIVP_GE_SCL_MAGIC(pstChnTask->u64Reserved6);
                       offsetx = ((MI_U32)pstChnTask->u64Reserved6)&0xFFFF;
                       offsety = ((MI_U32)pstChnTask->u64Reserved6>>16)&0xFFFF;

                       if(offsetx|| offsety)
                       {
                            //K6L GE post scaling up patch for YUYV output buffer format
                            MI_SYS_BUG_ON(E_MI_SYS_BUFDATA_FRAME != pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType);
                            MI_SYS_BUG_ON(E_MI_SYS_PIXEL_FRAME_YUV422_YUYV != pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.ePixelFormat);
                            MI_SYS_BUG_ON(offsetx >= pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u16Width);
                            MI_SYS_BUG_ON(offsety >= pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u16Height);
                            memset(&stSrc, 0, sizeof(MI_GFX_Surface_t));
                            memset(&stSrcRect, 0, sizeof(MI_GFX_Rect_t));
                            memset(&stDstRect, 0, sizeof(MI_GFX_Rect_t));

                            stDst.eColorFmt = E_MI_GFX_FMT_YUV422;
                            stDst.phyAddr = pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.phyAddr[0];
                            stDst.u32Height = pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u16Height;
                            stDst.u32Width = pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u16Width;
                            stDst.u32Stride = pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->stFrameData.u32Stride[0];
                            memcpy(&stSrc, &stDst, sizeof(stSrc));

                            stSrc.phyAddr+= offsety*stSrc.u32Stride;
                            stSrc.u32Height -= offsety;

                            stSrcRect.s32Xpos = offsetx;
                            stSrcRect.s32Ypos = 0;
                            stSrcRect.u32Width = stSrc.u32Width-offsetx;
                            stSrcRect.u32Height = stSrc.u32Height;

                            stDstRect.s32Xpos = 0;
                            stDstRect.s32Ypos = 0;
                            stDstRect.u32Width = stDst.u32Width;
                            stDstRect.u32Height = stDst.u32Height;

                            //pre releasing input port buf for saving buf cache & latency
                            BUG_ON(!pstChnTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]);
                            ret = mi_sys_FinishBuf(pstChnTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]);
                            pstChnTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID] = NULL;
                            BUG_ON(ret != MI_SUCCESS);

                            if(MI_SUCCESS != MI_GFX_BitBlit(&stSrc, &stSrcRect, &stDst, &stDstRect, NULL, &u16GEFence))
                            {
                                  DBG_ERR("[%s %d] MI_GFX_BitBlit fail!!!\n", __FUNCTION__, __LINE__);
                            }
                            //delay free output buf due to post GE scaling up requirements
                            pstChnTask->u64Reserved6 = u16GEFence | ((MI_U64)jiffies)<<32;
                            list_add_tail(&pstChnTask->listChnTask, &divp_isr_thread_post_ge_scalingup_task_list);
                        }
                        else
                        {
                             // NO GE scaling up requirement, just finish task buf immediately
                             #if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
                                gstDivpChnCtx[pstChnTask->u32ChnId].u64FinishOutBufCnt ++;
                             #endif
                            _mi_divp_delay_free_task_with_refresh_support(pstChnTask);
                        }// if(offsetx|| offsety)
                   }
#else
                    //finish buffer
                   _mi_divp_delay_free_task_with_refresh_support(pstChnTask);
#endif

#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
                   _mi_divp_DebugGetBufCnt(&gstDivpChnCtx[pstChnTask->u32ChnId]);
#endif
                }
#if 0
                else
                {
                    MI_U64 u64MiniSec = 0;
                    struct timespec stTaskRunningTime;
                    memset(&stTaskRunningTime, 0, sizeof(stTaskRunningTime));
                    do_posix_clock_monotonic_gettime(&stTaskRunningTime);

                    if(stTaskRunningTime.tv_sec >= pstChnTask->u64Reserved1)
                    {
                        u64MiniSec = (stTaskRunningTime.tv_sec - pstChnTask->u64Reserved1) * 1000
                               + stTaskRunningTime.tv_nsec / 1000000 - pstChnTask->u64Reserved2 / 1000000;
                    }

                    if(u64MiniSec > 3000)
                    {
                        DBG_ERR("Task running more than 300ms. u64MiniSec = %llu \n", u64MiniSec);

                        ///rewind task, cmdq and dip may be hung.
                        list_del(&pstChnTask->listChnTask);
                        pstChnTask->u64Reserved1 = 0;
                        pstChnTask->u64Reserved2 = 0;
                        pstChnTask->u64Reserved3 = 0;
                        pstChnTask->u64Reserved4 = 0;
                        MI_DIVP_LOCK_TODO_COMMON_TASK_LIST();
                        DBG_ERR("add divp_todo_common_task! \n");
                        //list_add(&pstChnTask->listChnTask, &divp_todo_common_task_list);
                        MI_DIVP_UNLOCK_TODO_COMMON_TASK_LIST();

                        //reset cmdq and dip
                        bNeedReset = TRUE;
                    }
                }
#endif
            }

            if(bNeedReset)
            {
                //reset cmdq and dip HW // CYS
                bNeedReset = FALSE;
            }
        }
        MI_DIVP_UNLOCK_WORKING_COMMON_TASK_LIST();

#if defined(SUPPORT_DIVP_USE_GE_SCALING_UP)&&(SUPPORT_DIVP_USE_GE_SCALING_UP==1)
        //check if GE post scaling up task is done
        //list_for_each(pos, &divp_isr_thread_post_ge_scalingup_task_list)

        list_for_each_safe(pos, n, &divp_isr_thread_post_ge_scalingup_task_list)
        {
            MI_U16 u16GEFence;
            MI_U32 curJiffies = jiffies;
            MI_U32 u32diff;
            mi_sys_ChnTaskInfo_t *pstChnTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
            u16GEFence = (MI_U16)(pstChnTask->u64Reserved6 & 0xffffUL);
            if(mi_gfx_PendingDone(u16GEFence) == MI_SUCCESS)
            {
                #if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
                    gstDivpChnCtx[pstChnTask->u32ChnId].u64FinishOutBufCnt ++;
                #endif
                list_del(&pstChnTask->listChnTask);
                mi_sys_FinishAndReleaseTask(pstChnTask);
            }
            else
            {
                if(curJiffies> (pstChnTask->u64Reserved6>>32))
                    u32diff = curJiffies- (pstChnTask->u64Reserved6>>32);
                else
                    u32diff = (0xFFFFFFFF-curJiffies)+(pstChnTask->u64Reserved6>>32)+1;
                if( u32diff>HZ/2)
                {
                    DBG_ERR("DIVP GE Post Scalingup Task doesn't finished in %d ms, force finish.\n", u32diff*1000/HZ);
                    list_del(&pstChnTask->listChnTask);
                    mi_sys_FinishAndReleaseTask(pstChnTask);
                }
                break;
            }
        }
#endif
    }
#if defined(SUPPORT_DIVP_USE_GE_SCALING_UP)&&(SUPPORT_DIVP_USE_GE_SCALING_UP==1)
    MI_GFX_Close();
#endif
    return 0;
}

static MI_U8 _mi_divp_SetInput2Pmode(void)
{
    MI_U8 u8ChannelId = 0;
    MI_U64 u64TotalArea = 0;
    MI_U16 u16fps =0;

    for(u8ChannelId =0; u8ChannelId < MI_DIVP_CHN_MAX_NUM; u8ChannelId++)
    {
        if(gstDivpChnCtx[u8ChannelId].eStatus == E_MI_DIVP_CHN_STARTED)
        {
            u64TotalArea += gstDivpChnCtx[u8ChannelId].stInputInfoPre.u16Width * gstDivpChnCtx[u8ChannelId].stInputInfoPre.u16Height;
        }
    }

    if(u64TotalArea == 0)
    {
        DBG_INFO("total area %llu\n",u64TotalArea);
        return MI_SUCCESS;
    }

    u16fps = MI_DIVP_SCL_CLOCK*1024*1024 /u64TotalArea;
    for(u8ChannelId =0; u8ChannelId < MI_DIVP_CHN_MAX_NUM; u8ChannelId++)
    {
        if((gstDivpChnCtx[u8ChannelId].eStatus == E_MI_DIVP_CHN_STARTED) && (gstDivpChnCtx[u8ChannelId].bProcSet2Pmode == FALSE))
        {
            if(u16fps < 40)
            {
                MI_U16 u16InWidth = gstDivpChnCtx[u8ChannelId].stInputInfoPre.u16Width;
                MI_U16 u16InHeight = gstDivpChnCtx[u8ChannelId].stInputInfoPre.u16Height;
                MI_U16 u16OutWidth = gstDivpChnCtx[u8ChannelId].stOutputInfoPre.u16Width;
                MI_U16 u16OutHeight = gstDivpChnCtx[u8ChannelId].stOutputInfoPre.u16Height;
                if(u16OutWidth ==0 || u16OutHeight == 0)
                {
                    DBG_INFO("output widthxheight %d x %d\n", u16OutWidth, u16OutHeight);
                    continue;
                }
                
                if(u16InWidth > u16InHeight)// not corridor mode
                {
                    MI_U16 u16SclStatu = (u16InWidth*MI_DIVP_SCL_RATIO_OUTPUT)/(u16OutWidth*MI_DIVP_SCL_RATIO_INPUT);
                    if(u16SclStatu > 0)
                    {
                        gstDivpChnCtx[u8ChannelId].b2Pmode = TRUE;
                    }
                    else
                    {
                        gstDivpChnCtx[u8ChannelId].b2Pmode = FALSE;
                    }
                }
                else // corridor mode
                {
                    if((u16InWidth*u16InHeight)/(u16OutWidth *u16OutHeight) >= MI_DIVP_CORRIDOR_AREA_RATIO)
                    {
                        gstDivpChnCtx[u8ChannelId].b2Pmode = TRUE;
                    }
                    else
                    {
                        gstDivpChnCtx[u8ChannelId].b2Pmode = FALSE;
                    }
                }
            }
            else
            {
                gstDivpChnCtx[u8ChannelId].b2Pmode = FALSE;
            }
        }
    }

    return MI_SUCCESS;
}

MI_S32 mi_divp_ProcessDramDataTask(MI_U16* pu16Fence, mi_divp_IteratorWorkInfo_t* pworkinfo, MHAL_CMDQ_CmdqInterface_t* pstCmdInf)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_BOOL bNeedDouble = FALSE;//if channel attribute change, pixel format or input/output size change, send the save frame twice to avoid show garbage.

    struct list_head* pos,*n;
    MHAL_DIVP_InputInfo_t stDivpInputInfo;
    MHAL_DIVP_OutPutInfo_t stDivpOutputInfo;
    MI_SYS_RawData_t stRawData;
    MI_SYS_FrameData_t stFrameData;
    MI_SYS_MetaData_t stMetaData;
    MHAL_DIVP_InputInfo_t stDivpInputInfo0;//used to save interlace top field info
    MHAL_DIVP_OutPutInfo_t stDivpOutputInfo0;//used to save output frame info

#if defined(DIVP_SUPPORT_RGN)  && (DIVP_SUPPORT_RGN == 1)
    mi_rgn_ProcessCmdInfo_t stRgnCmd;
    memset(&stRgnCmd, 0, sizeof(stRgnCmd));
#endif

    memset(&stDivpInputInfo, 0, sizeof(stDivpInputInfo));
    memset(&stDivpOutputInfo, 0, sizeof(stDivpOutputInfo));
    memset(&stRawData, 0, sizeof(stRawData));
    memset(&stFrameData, 0, sizeof(stFrameData));
    memset(&stMetaData, 0, sizeof(MI_SYS_MetaData_t));
    memset(&stDivpInputInfo0, 0, sizeof(stDivpInputInfo0));
    memset(&stDivpOutputInfo0, 0, sizeof(stDivpOutputInfo0));


    /////////////////////////////////// sys channel task reserved variables useage ///////////////////////////////////
    //mi_sys_ChnTaskInfo_t.u32Reserved0 ==> DIVP task fence
    //mi_sys_ChnTaskInfo_t.u32Reserved1 ==> Not used
    //mi_sys_ChnTaskInfo_t.u64Reserved0 ==> RGN task fence
    //mi_sys_ChnTaskInfo_t.u64Reserved1 ==> task start time : seconds
    //mi_sys_ChnTaskInfo_t.u64Reserved2 ==> task start time : Nanoseconds
    //mi_sys_ChnTaskInfo_t.u64Reserved3 ==> task has been created script : seconds
    //mi_sys_ChnTaskInfo_t.u64Reserved4 ==> task has been created script : Nanoseconds
    //mi_sys_ChnTaskInfo_t.u64Reserved5 ==> Not used
    /////////////////////////////////// sys channel task reserved variables useage ///////////////////////////////////

    ///1 check input parameter
    MI_DIVP_CHECK_POINTER(pu16Fence)
    MI_DIVP_CHECK_POINTER(pstCmdInf)

    list_for_each_safe(pos, n, &divp_todo_common_task_list)
    {
        mi_sys_ChnTaskInfo_t *pstChnTask;
        MI_U32 u32LoopCnt = 0;

        pstChnTask = (mi_sys_ChnTaskInfo_t *)container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
        if (gstDivpChnCtx[pstChnTask->u32ChnId].eStatus == E_MI_DIVP_CHN_DISTROYED) {
            DBG_ERR("gstDivpChnCtx[pstChnTask->u32ChnId].eStatus == E_MI_DIVP_CHN_DISTROYED\n");
            continue;
        }

        DIVP_PERF_TIME(&pstChnTask->u64DivpProcessTaskStart);

        while(256 > pstCmdInf->MHAL_CMDQ_CheckBufAvailable(pstCmdInf, 256))//0x800 = 256 commands * 8 bytes/command
        {
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            interruptible_sleep_on_timeout(&divp_isr_waitqueue, msecs_to_jiffies(2));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            wait_event_interruptible_timeout(divp_isr_waitqueue, FALSE, msecs_to_jiffies(2));
#endif
            u32LoopCnt++;
            if(u32LoopCnt>1000)
            {
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
                DBG_ERR("HW engine hung.\n");
                if(!list_empty_careful(&divp_working_common_task_list));
                {
                    mi_sys_ChnTaskInfo_t *pstChnTask;
                    list_for_each_safe(pos, n, &divp_working_common_task_list)
                    {
                        pstChnTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
                        list_del(&pstChnTask->listChnTask);

                        ///rewind task, cmdq and dip may be hung.
                        pstChnTask->u64Reserved1 = 0;
                        pstChnTask->u64Reserved2 = 0;
                        pstChnTask->u64Reserved3 = 0;
                        pstChnTask->u64Reserved4 = 0;
                        MI_DIVP_LOCK_TODO_COMMON_TASK_LIST();
                        DBG_INFO("add divp_todo_common_task! \n");
                        list_add(&pstChnTask->listChnTask, &divp_todo_common_task_list);
                        MI_DIVP_UNLOCK_TODO_COMMON_TASK_LIST();
                    }
                }

                //reset cmdq and dip HW // CYS
#endif
                break;
            }
        }

        DIVP_PERF_TIME(&pstChnTask->u64DivpGotCmdqBuffer);
        MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(pstChnTask->u32ChnId);
        ///2 prepare channel output buffer and get input/output buffer infomation
        s32Ret = _mi_divp_GetInputInfo(pstChnTask, &stDivpInputInfo);
        if(MI_SUCCESS == s32Ret)
        {
            s32Ret = _mi_divp_GetOutputInfo(pstChnTask, &stDivpOutputInfo);
        }

        ///3 deal with dram data task.
        if(MI_SUCCESS == s32Ret)
        {
            ///3.0 clean CMDQ trigger bus signal and check CMDQ status.
            pstCmdInf->MHAL_CMDQ_ClearTriggerEvent(pstCmdInf);

#if defined(DIVP_SUPPORT_RGN)  && (DIVP_SUPPORT_RGN == 1)
            ///3.1 deal with cover and OSD.
            //call region HAL layer channel ID and module ID ,output size,
            //if the channel id is 32, close OSD and cover for private channel.
            stRgnCmd.u32chnID = pstChnTask->u32ChnId;
            stRgnCmd.stDivpPort[DIVP_CHN_OUTPUT_PORT_ID].bEnable = TRUE;
            stRgnCmd.stDivpPort[DIVP_CHN_OUTPUT_PORT_ID].u32Width = gstDivpChnCtx[pstChnTask->u32ChnId].stOutputInfoPre.u16Width;
            stRgnCmd.stDivpPort[DIVP_CHN_OUTPUT_PORT_ID].u32Height = gstDivpChnCtx[pstChnTask->u32ChnId].stOutputInfoPre.u16Height;

            if(MI_RGN_OK != mi_rgn_DivpProcess(&stRgnCmd, pstCmdInf, &(pstChnTask->u64Reserved0)))
            {
                DBG_ERR("mi_rgn_DivpProcess failed.\n");
            }
#endif

            ///3.2 when the channel shows the first frame , output config or attribute changes, if TNR is on, send the same frame twice
            ///to avoid show garbage. when the channel showing the first frame, we all so think all channel attribute have beenchanged.
            if((gstDivpChnCtx[pstChnTask->u32ChnId].bInputChange
                    || gstDivpChnCtx[pstChnTask->u32ChnId].bOutputChange
                    || gstDivpChnCtx[pstChnTask->u32ChnId].bChnAttrChg)
                && ((E_MI_DIVP_TNR_LEVEL_OFF < gstDivpChnCtx[pstChnTask->u32ChnId].stChnAttr.eTnrLevel)
                    && (E_MI_DIVP_TNR_LEVEL_NUM > gstDivpChnCtx[pstChnTask->u32ChnId].stChnAttr.eTnrLevel))
                && (stDivpInputInfo.eScanMode == E_MHAL_DIVP_SCAN_MODE_PROGRESSIVE))
            {
                bNeedDouble = TRUE;
            }

            ///3.3 set channel attr
            _mi_divp_SetChnAttr(pstChnTask->u32ChnId, stDivpInputInfo.u16InputWidth, stDivpInputInfo.u16InputHeight, pstCmdInf);

#if defined(SUPPORT_DIVP_USE_GE_SCALING_UP)&&(SUPPORT_DIVP_USE_GE_SCALING_UP==1)
         if((stDivpOutputInfo.u16OutputWidth> stDivpInputInfo.stCropWin.u16Width
            ||stDivpOutputInfo.u16OutputHeight> stDivpInputInfo.stCropWin.u16Height) &&
                      (stDivpOutputInfo.ePxlFmt==E_MHAL_DIVP_PIXEL_FORMAT_YUV422_YUYV))
           {
                    MI_U32 offsetx=0, offsety=0;
                    if(stDivpOutputInfo.u16OutputHeight>  stDivpInputInfo.stCropWin.u16Height)
                    {
                        offsety = stDivpOutputInfo.u16OutputHeight- stDivpInputInfo.stCropWin.u16Height;
                        stDivpOutputInfo.u64BufAddr[0] += offsety * stDivpOutputInfo.u16Stride[0];
                        stDivpOutputInfo.u16OutputHeight -=  offsety;
                    }
                    if(stDivpOutputInfo.u16OutputWidth> stDivpInputInfo.stCropWin.u16Width)
                    {
                        offsetx = stDivpOutputInfo.u16OutputWidth-stDivpInputInfo.stCropWin.u16Width;
                        offsetx = ALIGN_UP(offsetx,2);
                        stDivpOutputInfo.u64BufAddr[0] += offsetx*2;
                        stDivpOutputInfo.u16OutputWidth -= offsetx;
                    }
                    pstChnTask->u64Reserved6 = offsetx|offsety<<16UL|DIVP_GE_SCALING_META_MAGIC;
           }
           else
           {
                    pstChnTask->u64Reserved6 = DIVP_GE_SCALING_META_MAGIC;
           }
#endif
            ///3.4 if output is connected to display, output frame is send to MDWIN buffer, or else send to output buffer.
            //if channel attribute change, channel attribute, pixel format or input/output size change, send the save frame twice to avoid show garbage.
            _mi_divp_SetInput2Pmode();
            
            stDivpInputInfo.b2P_Enable = gstDivpChnCtx[pstChnTask->u32ChnId].b2Pmode;
            s32Ret = MHAL_DIVP_ProcessDramData(gstDivpChnCtx[pstChnTask->u32ChnId].pHalCtx, &stDivpInputInfo, &stDivpOutputInfo, pstCmdInf);
            if(bNeedDouble && (MHAL_SUCCESS == s32Ret))
            {
                s32Ret = MHAL_DIVP_ProcessDramData(gstDivpChnCtx[pstChnTask->u32ChnId].pHalCtx, &stDivpInputInfo, &stDivpOutputInfo, pstCmdInf);
                bNeedDouble = FALSE;
            }

            ///3.5 transfer PTS
            pstChnTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->u64Pts = pstChnTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->u64Pts;
        }
        else
        {
            DBG_ERR("Fail to get input/output information.");
        }
        MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(pstChnTask->u32ChnId);

        ///4 remove task from todo list
        list_del(&pstChnTask->listChnTask);
        pworkinfo->totalAddedTask --;

        if(MI_SUCCESS == s32Ret)
        {
            *pu16Fence += 1;
            if(0xFFF < *pu16Fence)
            {
                *pu16Fence = 1;
            }
            pstChnTask->u32Reserved0 = *pu16Fence;
            if(E_MHAL_DIVP_PIXEL_FORMAT_YUV_MST_420 == stDivpOutputInfo.ePxlFmt)
            {
                MHAL_DIVP_WaitCmdQFrameDone(pstCmdInf, E_MHAL_CMDQEVE_S1_MDW_W_DONE);
                DBG_INFO("CMDQ event = E_MHAL_CMDQEVE_S1_MDW_W_DONE");
            }
            else
            {
                MHAL_DIVP_WaitCmdQFrameDone(pstCmdInf, E_MHAL_CMDQEVE_DIP_TRIG);
                DBG_INFO("CMDQ event = E_MHAL_CMDQEVE_DIP_TRIG");
            }

            pstCmdInf->MHAL_CMDQ_WriteDummyRegCmdq(pstCmdInf, *pu16Fence);
            DIVP_PERF_TIME(&pstChnTask->u64DivpCmdqKickOff);
            pstCmdInf->MHAL_CMDQ_KickOffCmdq(pstCmdInf);
            DBG_INFO(" add task to divp_working_common_task_list! *pu16Fence = %u. pu16Fence = %p.\n",*pu16Fence, pu16Fence);

            MI_DIVP_LOCK_WORKING_COMMON_TASK_LIST();
#if ENABLE_DEBUG_FENCE
            _gau64ProcessFrmNum[pstChnTask->u32ChnId]++;
            pstChnTask->u64ProcessTask = _gau64ProcessFrmNum[pstChnTask->u32ChnId];
            pstChnTask->phyProcessAddress = pstChnTask->astOutputPortBufInfo[0]->stFrameData.phyAddr[0];
#endif

            DIVP_PERF_TIME(&pstChnTask->u64DivpProcessTaskEnd);

            list_add_tail(&pstChnTask->listChnTask, &divp_working_common_task_list);
            MI_DIVP_UNLOCK_WORKING_COMMON_TASK_LIST();

            //printk("@@@@ stTaskEndTime.tv_sec:0x%lx,stTaskEndTime.tv_nsec:0x%lx\n",stTaskEndTime.tv_sec,stTaskEndTime.tv_nsec);
            //PRINT_TIME("##  ==>  task running time : %lu ms\n\n", (stTaskEndTime.tv_sec - stTaskStartTime.tv_sec) * 1000 + (stTaskEndTime.tv_nsec - stTaskStartTime.tv_nsec) / 1000000);
        }
        else
        {
            DBG_ERR("Fail to process DRAM data! bNeedDouble = %d.\n", bNeedDouble);
            ///drop added commands
            pstCmdInf->MHAL_CMDQ_CmdqAbortBuffer(pstCmdInf);
            list_add(&pstChnTask->listChnTask, &divp_todo_common_task_list);
            //update all attribute for next frame.
            gbResetAllAttr[pstChnTask->u32ChnId] = TRUE;
            DIVP_PERF_TIME(&pstChnTask->u64DivpProcessTaskEnd);

        }
    }

    return s32Ret;
}

MI_S32 mi_divp_ProcessCaptureCreenTask(MI_U16* pu16Fence, MHAL_CMDQ_CmdqInterface_t* pstCmdInf)
{
    MI_S32 s32Ret = MI_SUCCESS;
    mi_divp_CaptureInfo_t* pstCapInfo;
    MHAL_DIVP_CaptureInfo_t stHalCapInfo;
    MI_U32 u32LoopCnt = 0;
    struct list_head* pos, *n;

    list_for_each_safe(pos, n, &divp_todo_capture_task_list)
    {
        u32LoopCnt = 0;
        memset(&stHalCapInfo, 0, sizeof(MHAL_DIVP_CaptureInfo_t));

        pstCapInfo = (mi_divp_CaptureInfo_t*)container_of(pos, mi_divp_CaptureInfo_t, capture_list);
        while(256 > pstCmdInf->MHAL_CMDQ_CheckBufAvailable(pstCmdInf, 256))//0x800 = 256 commands * 8 bytes/command
        {
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            interruptible_sleep_on_timeout(&divp_isr_waitqueue, msecs_to_jiffies(2));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            wait_event_interruptible_timeout(divp_isr_waitqueue, FALSE, msecs_to_jiffies(2));//wait timeout wakeup
#endif
#if 0
            u32LoopCnt++;
            if(u32LoopCnt>1000)
            {
                MI_SYS_BUG();//engine hang
                break;
            }
#endif
        }

        //process capture screen task
        _mi_divp_checkPrivateChnAttr(pstCapInfo);
        _mi_divp_SetChnAttr(DIVP_CHN_MAX, pstCapInfo->stCropWin.u16Width, pstCapInfo->stCropWin.u16Height, pstCmdInf);

        _mi_divp_TransMiCaptureInfoToHal(pstCapInfo, &stHalCapInfo);
        if(MHAL_SUCCESS == MHAL_DIVP_CaptureTiming(gstDivpChnCtx[MI_DIVP_CHN_MAX_NUM].pHalCtx, &stHalCapInfo, pstCmdInf))
        {
            pstCapInfo->bRetVal = TRUE;
        }
        else
        {
            pstCapInfo->bRetVal = FALSE;
            s32Ret = MI_DIVP_ERR_FAILED;
        }

        //record fence and remove task in todo list
        list_del(&pstCapInfo->capture_list);

        if(MI_SUCCESS == s32Ret)
        {
            *pu16Fence += 1;
            if(0xFFF < *pu16Fence)
            {
                *pu16Fence = 1;
            }
            pstCapInfo->u32Fence = *pu16Fence;
            MHAL_DIVP_WaitCmdQFrameDone(pstCmdInf, E_MHAL_CMDQEVE_DIP_TRIG);
            pstCmdInf->MHAL_CMDQ_WriteDummyRegCmdq(pstCmdInf, *pu16Fence);
            pstCmdInf->MHAL_CMDQ_KickOffCmdq(pstCmdInf);

            //add task info to working list
            MI_DIVP_LOCK_WORKING_CAPTURE_TASK_LIST();
            DBG_INFO("u32Fence :%d \n",pstCapInfo->u32Fence);
            DBG_INFO("add divp_working_capture_task_list! \n");
            list_add_tail(&pstCapInfo->capture_list, &divp_working_capture_task_list);
            MI_DIVP_UNLOCK_WORKING_CAPTURE_TASK_LIST();
        }
        else
        {
            ///drop added commands
            pstCmdInf->MHAL_CMDQ_CmdqAbortBuffer(pstCmdInf);
            pstCapInfo->pfnDispCallBack(pstCapInfo);
            //update all attribute for next frame.
            gbResetAllAttr[MI_DIVP_CHN_MAX_NUM] = TRUE;
            DBG_ERR("Fail to process capture display screen task. pstCapInfo->u32Fence = %d\n", pstCapInfo->u32Fence);
        }
    }

    return s32Ret;
}

int mi_divp_WorkThread(void *data)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U16 u16Fence = 0;
    MHAL_CMDQ_CmdqInterface_t *pstCmdInf = (MHAL_CMDQ_CmdqInterface_t*)data;

    mi_divp_IteratorWorkInfo_t workinfo;
    workinfo.totalAddedTask = 0;

    DBG_INFO(" mi_divp_WorkThread working! pstCmdInf = %p.\n",pstCmdInf);
    ///have cmd queue
    while(1)
    {
        if(kthread_should_stop())
        {
            DBG_INFO(" mi_divp_WorkThread stoped!\n");
            _mi_divp_ClearTodoCaptureTask();
            _mi_divp_ClearTodoCommonTask();
            return -1;
        }

        ///1 deal with capture screen task
        MI_DIVP_LOCK_TODO_CAPTURE_TASK_LIST();
        if(!list_empty_careful(&divp_todo_capture_task_list))
        {
            DBG_INFO(" u16Fence = %u. &u16Fence = %p.\n", u16Fence, &u16Fence);
            mi_divp_ProcessCaptureCreenTask(&u16Fence, pstCmdInf);
        }
        MI_DIVP_UNLOCK_TODO_CAPTURE_TASK_LIST();

        ///2 process common task
        mi_sys_DevTaskIterator(gDivpDevHdl, mi_divp_TaskIteratorCallBack, &workinfo);

        MI_DIVP_LOCK_TODO_COMMON_TASK_LIST();
        if(list_empty_careful(&divp_todo_common_task_list))
        {
            MI_DIVP_UNLOCK_TODO_COMMON_TASK_LIST();
            msleep(4);
            mi_sys_WaitOnInputTaskAvailable(gDivpDevHdl, 100);
            continue;
        }
        else
        {
            DBG_INFO(" u16Fence = %u. &u16Fence = %p.\n", u16Fence, &u16Fence);
            s32Ret = mi_divp_ProcessDramDataTask(&u16Fence, &workinfo, pstCmdInf);
        }
        MI_DIVP_UNLOCK_TODO_COMMON_TASK_LIST();
    }

    return s32Ret;
}

static irqreturn_t mi_divp_Isr(int nIRQ_ID, void *data)
{
    ///clean ISR
    MHAL_DIVP_CleanFrameDoneIsr();

    WAKE_UP_QUEUE_IF_NECESSARY(divp_isr_waitqueue);

#if ENABLE_DEBUG_PERFORMANCE
    _gu64IsrNum ++;
    DIVP_PERF_TIME(&_gu64IsrCommingTime);
#endif
    return IRQ_HANDLED;
}

MI_S32 mi_divp_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    //pstChnCurryPort: port in my module. pstChnPeerPort is the port other module. pUsrData is reserved.
    MI_S32 s32Ret = MI_SUCCESS;

    ///1 check input parameter
    MI_DIVP_CHECK_POINTER(pstChnCurryPort)
    MI_DIVP_CHECK_POINTER(pstChnPeerPort)
    DBG_INFO("Src : eModId = %u, u32DevId = %u, u32ChnId = %u, u32PortId = %u.", pstChnCurryPort->eModId,
    pstChnCurryPort->u32DevId, pstChnCurryPort->u32ChnId, pstChnCurryPort->u32PortId);
    DBG_INFO("Dst : eModId = %u, u32DevId = %u, u32ChnId = %u, u32PortId = %u.", pstChnPeerPort->eModId,
    pstChnPeerPort->u32DevId, pstChnPeerPort->u32ChnId, pstChnPeerPort->u32PortId);

    ///2 save output port infomation in channel context.
    if(E_MI_MODULE_ID_DIVP == pstChnCurryPort->eModId)
    {
        if((E_MI_DIVP_Device0 == pstChnCurryPort->u32DevId)
            && (DIVP_CHN_OUTPUT_PORT_ID == pstChnCurryPort->u32PortId))
        {
            memcpy(&gstDivpChnCtx[pstChnCurryPort->u32ChnId].stOutputPort, pstChnPeerPort, sizeof(MI_SYS_ChnPort_t));
        }
        else
        {
            DBG_ERR(" Invalid device or port ID. u32DevId = %d, u32PortId = %d\n", pstChnCurryPort->u32DevId, pstChnCurryPort->u32PortId);
            s32Ret = MI_DIVP_ERR_INVALID_PARAM;
        }
    }
    else
    {
        DBG_ERR(" Invalid module ID. eModId = %d.\n", pstChnCurryPort->eModId);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }

    return s32Ret;
}

MI_S32 mi_divp_OnUnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    //pstChnCurryPort: port in my module. pstChnPeerPort is the port other module. pUsrData is reserved.
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32Cnt = 0;

    ///1 check input parameter
    MI_DIVP_CHECK_POINTER(pstChnCurryPort)
    MI_DIVP_CHECK_POINTER(pstChnPeerPort)

    if(E_MI_MODULE_ID_DIVP == pstChnCurryPort->eModId)
    {
        ///2 check channel status
        if(E_MI_DIVP_CHN_STARTED == gstDivpChnCtx[pstChnCurryPort->u32ChnId].eStatus)
        {
            while(_mi_divp_ChnBusy(pstChnCurryPort->u32ChnId) && (s32Cnt < 100))
            {
                schedule_timeout_interruptible(msecs_to_jiffies(10));//sleep 10ms
                s32Cnt ++;
            }

            if(s32Cnt == 100)
            {
                DBG_ERR(" Unbind output timeout====. u32ChnId = %d.\n", pstChnCurryPort->u32ChnId);
                s32Ret = MI_DIVP_ERR_CHN_BUSY;
            }
        }

        ///3 reset output port infomation and stop status in channel context.
        if((MI_SUCCESS == s32Ret) && E_MI_MODULE_ID_DIVP == pstChnCurryPort->eModId)
        {
            if((E_MI_DIVP_Device0 == pstChnCurryPort->u32DevId)
                && (DIVP_CHN_OUTPUT_PORT_ID == pstChnCurryPort->u32PortId))
            {
                gstDivpChnCtx[pstChnCurryPort->u32ChnId].stOutputPort.eModId = E_MI_MODULE_ID_MAX;
                gstDivpChnCtx[pstChnCurryPort->u32ChnId].stOutputPort.u32DevId = 0xFF;
                gstDivpChnCtx[pstChnCurryPort->u32ChnId].stOutputPort.u32PortId = 0xFF;
                gstDivpChnCtx[pstChnCurryPort->u32ChnId].stOutputPort.u32ChnId = MI_DIVP_CHN_NULL;
            }
            else
            {
                DBG_ERR(" Invalid device or port ID. u32DevId = %d, u32PortId = %d\n", pstChnCurryPort->u32DevId, pstChnCurryPort->u32PortId);
                s32Ret = MI_DIVP_ERR_INVALID_PARAM;
            }
        }
    }
    else
    {
        DBG_ERR(" Invalid module ID. eModId = %d.\n", pstChnCurryPort->eModId);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }
    return s32Ret;
}

MI_S32 mi_divp_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    //pstChnCurryPort: port in my module. pstChnPeerPort is the port other module. pUsrData is reserved.
    MI_S32 s32Ret = MI_SUCCESS;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(pstChnCurryPort->u32ChnId)
    MI_DIVP_CHECK_POINTER(pstChnCurryPort)
    MI_DIVP_CHECK_POINTER(pstChnPeerPort)

    ///2 save input port infomation in channel context.
    if(E_MI_MODULE_ID_DIVP == pstChnCurryPort->eModId)
    {
        if((E_MI_DIVP_Device0 == pstChnCurryPort->u32DevId)
            && (DIVP_CHN_INPUT_PORT_ID == pstChnCurryPort->u32PortId))
        {
            memcpy(&gstDivpChnCtx[pstChnCurryPort->u32ChnId].stInputPort, pstChnPeerPort, sizeof(MI_SYS_ChnPort_t));
        }
        else
        {
            DBG_ERR(" Invalid device or port ID. u32DevId = %d, u32PortId = %d\n", pstChnCurryPort->u32DevId, pstChnCurryPort->u32PortId);
            s32Ret = MI_DIVP_ERR_INVALID_PARAM;
        }
    }
    else
    {
        DBG_ERR(" Invalid module ID. eModId = %d.\n", pstChnCurryPort->eModId);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }

    return MI_SUCCESS;
}

MI_S32 mi_divp_OnUnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    //pstChnCurryPort: port in my module. pstChnPeerPort is the port other module. pUsrData is reserved.
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32Cnt = 0;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(pstChnCurryPort->u32ChnId)
    MI_DIVP_CHECK_POINTER(pstChnCurryPort)
    MI_DIVP_CHECK_POINTER(pstChnPeerPort)

    if((E_MI_MODULE_ID_DIVP == pstChnCurryPort->eModId)
        && (E_MI_DIVP_Device0 == pstChnCurryPort->u32DevId)
        && (DIVP_CHN_OUTPUT_PORT_ID == pstChnCurryPort->u32PortId))
    {
        ///2 check channel status
        if(E_MI_DIVP_CHN_STARTED == gstDivpChnCtx[pstChnCurryPort->u32ChnId].eStatus)
        {
            while(_mi_divp_ChnBusy(pstChnCurryPort->u32ChnId) && (s32Cnt < 100))
            {
                schedule_timeout_interruptible(msecs_to_jiffies(10));//sleep 10ms
                s32Cnt ++;
            }

            if(s32Cnt == 100)
            {
                DBG_ERR(" Unbind input timeout====. u32ChnId = %d.\n", pstChnCurryPort->u32ChnId);
                s32Ret = MI_DIVP_ERR_CHN_BUSY;
            }
        }

        ///3 reset output port infomation and stop status in channel context.
        if(MI_SUCCESS == s32Ret)
        {
            if((E_MI_DIVP_Device0 == pstChnCurryPort->u32DevId)
                && (DIVP_CHN_OUTPUT_PORT_ID == pstChnCurryPort->u32PortId))
            {
#if (0)
                gstDivpChnCtx[pstChnCurryPort->u32ChnId].stInputPort.eModId = E_MI_MODULE_ID_MAX;
                gstDivpChnCtx[pstChnCurryPort->u32ChnId].stInputPort.u32DevId = 0xFF;
                gstDivpChnCtx[pstChnCurryPort->u32ChnId].stInputPort.u32PortId = 0xFF;
                gstDivpChnCtx[pstChnCurryPort->u32ChnId].stInputPort.u32ChnId = MI_DIVP_CHN_NULL;

                gstDivpChnCtx[pstChnCurryPort->u32ChnId].eStatus = E_MI_DIVP_CHN_STOPED;
#endif
            }
            else
            {
                DBG_ERR(" Invalid device or port ID. u32DevId = %d, u32PortId = %d\n", pstChnCurryPort->u32DevId, pstChnCurryPort->u32PortId);
                s32Ret = MI_DIVP_ERR_INVALID_PARAM;
            }
        }
    }
    else
    {
        DBG_ERR(" Invalid module ID. eModId = %d, u32DevId = %d, u32PortId = %d.\n",
            pstChnCurryPort->eModId, pstChnCurryPort->u32DevId, pstChnCurryPort->u32PortId);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }

    return s32Ret;
}

MI_S32 mi_divp_Alloc(MI_U8 *pu8MMAHeapName, MI_U32 u32Size , MI_U64 *pu64Addr)
{
    MI_S32 s32Ret = mi_sys_MMA_Alloc(pu8MMAHeapName, u32Size , pu64Addr);

    return s32Ret;
}

MI_S32 mi_divp_Free(MI_U64 u64Addr)
{
    MI_S32 s32Ret = mi_sys_MMA_Free(u64Addr);
    return s32Ret;
}

static MI_S32 _mi_divp_CreatePrivateChannel(MHAL_CMDQ_CmdqInterface_t* ptCmdInf)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_ChnAttr_t stChnAttr;
    MHAL_DIVP_Mirror_t stHalMirror;
    memset(&stChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));

    //private channel default atrribute
    stChnAttr.bHorMirror = FALSE;
    stChnAttr.bVerMirror = FALSE;
    stChnAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stChnAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stChnAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stChnAttr.u32MaxWidth = MI_DIVP_TIMING_WIDTH_4K;
    stChnAttr.u32MaxHeight = MI_DIVP_TIMING_HEIGHT_2K;
    stChnAttr.stCropRect.u16X = 0;
    stChnAttr.stCropRect.u16Y = 0;
    stChnAttr.stCropRect.u16Width = MI_DIVP_TIMING_WIDTH_4K;
    stChnAttr.stCropRect.u16Height = MI_DIVP_TIMING_HEIGHT_2K;

    memset(&stHalMirror, 0, sizeof(MHAL_DIVP_Mirror_t));
    stHalMirror.bHMirror = stChnAttr.bHorMirror;
    stHalMirror.bVMirror = stChnAttr.bVerMirror;

    if(MHAL_SUCCESS == MHAL_DIVP_CreateInstance(E_MHAL_DIVP_Device0, stChnAttr.u32MaxWidth, stChnAttr.u32MaxHeight,
                                    mi_divp_Alloc, mi_divp_Free, MI_DIVP_CHN_MAX_NUM, &gstDivpChnCtx[MI_DIVP_CHN_MAX_NUM].pHalCtx))
    {
        MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DIVP_CHN_MAX);
        //create channel ID and save channel attribute
        gstDivpChnCtx[DIVP_CHN_MAX].u32ChnId = DIVP_CHN_MAX;
        gstDivpChnCtx[DIVP_CHN_MAX].eStatus = E_MI_DIVP_CHN_STARTED;

        //save attribute
        memcpy(&gstDivpChnCtx[DIVP_CHN_MAX].stChnAttr, &stChnAttr, sizeof(MI_DIVP_ChnAttr_t));
        memcpy(&gstDivpChnCtx[DIVP_CHN_MAX].stChnAttrOrg, &stChnAttr, sizeof(MI_DIVP_ChnAttr_t));
        gstDivpChnCtx[DIVP_CHN_MAX].bChnAttrChg = TRUE;
        MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DIVP_CHN_MAX);
    }
    else
    {
        DBG_ERR(" Fail to create instance in HAL layer.\n");
        s32Ret = MI_DIVP_ERR_FAILED;
    }

    return s32Ret;
}

#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
/*
N.B. use handle.OnWriteOut to print
*/
MI_S32 mi_divp_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle,MI_U32 u32DevId,void *pUsrData)
{
    DBG_INFO("u32DevId=%u\n",u32DevId);
    handle.OnPrintOut(handle,"in  mi_divp_OnDumpDevAttr\n");
    ///TODO:
    ///dump module_dev private info
    handle.OnPrintOut(handle,"\r\n-------------------------- Start dump DEV  --------------------------------\n");
    handle.OnPrintOut(handle,"\n");
    handle.OnPrintOut(handle, "-------------------------- End dump DEV  -----------------------------------\n");
    return MI_SUCCESS;
}

/*
N.B. use handle.OnWriteOut to print
*/
MI_S32 mi_divp_OnDumpChannelAttr(MI_SYS_DEBUG_HANDLE_t handle,MI_U32 u32DevId,void *pUsrData)
{
    MI_U32 u32ChnId = 0;//TODO:while all channel
    MI_U64 u64MeanTime = 0;

    DBG_INFO("u32DevId=%u   u32ChnId=%u\n",u32DevId,u32ChnId);
    ///TODO:
    ///dump  info base u32ChnId
    handle.OnPrintOut(handle,"\r\n-------------------------- start dump CHN info --------------------------------\n");
    handle.OnPrintOut(handle,"%7s%8s%9s%17s%12s%12s%14s%13s\n","ChnId", "Status", "AttrChg", "bSendFrameTwice","FieldType0", "FieldType1","TaskMeanTime","TaskMaxTime");

    for(u32ChnId = 0; u32ChnId < (MI_DIVP_CHN_MAX_NUM +1); u32ChnId++)
    {
        if(gstDivpChnCtx[u32ChnId].eStatus > E_MI_DIVP_CHN_INITED)
        {
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%8d", gstDivpChnCtx[u32ChnId].eStatus);
            handle.OnPrintOut(handle,"%9d", gstDivpChnCtx[u32ChnId].bChnAttrChg);
            handle.OnPrintOut(handle,"%17d", gstDivpChnCtx[u32ChnId].bSendFrameTwice);
            handle.OnPrintOut(handle,"%12d", gstDivpChnCtx[u32ChnId].eaFieldType[0]);
            handle.OnPrintOut(handle,"%12d", gstDivpChnCtx[u32ChnId].eaFieldType[1]);

            if(gstDivpChnCtx[u32ChnId].u64ReleaseTaskCnt != 0)
                u64MeanTime = div64_u64(gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskSumTime, gstDivpChnCtx[u32ChnId].u64ReleaseTaskCnt);

            handle.OnPrintOut(handle,"%12lluus", u64MeanTime);
            handle.OnPrintOut(handle,"%11lluus\n", gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskMaxTime);
            //gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskSumTime =0;
            //gstDivpChnCtx[u32ChnId].u64GetToReleaseTaskMaxTime =0;
            //gstDivpChnCtx[u32ChnId].u64ReleaseTaskCnt = 0;
        }
    }

    handle.OnPrintOut(handle,"\r\n%19s%6s%6s%7s%7s%7s%7s%10s%10s%9s%13s%11s\n","ChnID", "MaxW", "MaxH", "CropX", "CropY", "CropW", "CropH","bHMirror", "bVMirror","eDiType","eRotateType","eTnrLevel");
    for(u32ChnId = 0; u32ChnId < (MI_DIVP_CHN_MAX_NUM +1); u32ChnId++)
    {
        if(gstDivpChnCtx[u32ChnId].eStatus > E_MI_DIVP_CHN_INITED)
        {

            handle.OnPrintOut(handle,"%12s","ChnAttr");
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%6d", gstDivpChnCtx[u32ChnId].stChnAttr.u32MaxWidth);
            handle.OnPrintOut(handle,"%6d", gstDivpChnCtx[u32ChnId].stChnAttr.u32MaxHeight);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttr.stCropRect.u16X);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttr.stCropRect.u16Y);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttr.stCropRect.u16Width);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttr.stCropRect.u16Height);
            handle.OnPrintOut(handle,"%10d", gstDivpChnCtx[u32ChnId].stChnAttr.bHorMirror);
            handle.OnPrintOut(handle,"%10d", gstDivpChnCtx[u32ChnId].stChnAttr.bVerMirror);
            handle.OnPrintOut(handle,"%9d", gstDivpChnCtx[u32ChnId].stChnAttr.eDiType);
            handle.OnPrintOut(handle,"%13d", gstDivpChnCtx[u32ChnId].stChnAttr.eRotateType);
            handle.OnPrintOut(handle,"%11d\n", gstDivpChnCtx[u32ChnId].stChnAttr.eTnrLevel);

            handle.OnPrintOut(handle,"%12s","ChnAttrPre");
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%6d", gstDivpChnCtx[u32ChnId].stChnAttrPre.u32MaxWidth);
            handle.OnPrintOut(handle,"%6d", gstDivpChnCtx[u32ChnId].stChnAttrPre.u32MaxHeight);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttrPre.stCropRect.u16X);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttrPre.stCropRect.u16Y);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttrPre.stCropRect.u16Width);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttrPre.stCropRect.u16Height);
            handle.OnPrintOut(handle,"%10d", gstDivpChnCtx[u32ChnId].stChnAttrPre.bHorMirror);
            handle.OnPrintOut(handle,"%10d", gstDivpChnCtx[u32ChnId].stChnAttrPre.bVerMirror);
            handle.OnPrintOut(handle,"%9d", gstDivpChnCtx[u32ChnId].stChnAttrPre.eDiType);
            handle.OnPrintOut(handle,"%13d", gstDivpChnCtx[u32ChnId].stChnAttrPre.eRotateType);
            handle.OnPrintOut(handle,"%11d\n", gstDivpChnCtx[u32ChnId].stChnAttrPre.eTnrLevel);

            handle.OnPrintOut(handle,"%12s","ChnAttrOrg");
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%6d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.u32MaxWidth);
            handle.OnPrintOut(handle,"%6d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.u32MaxHeight);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.stCropRect.u16X);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.stCropRect.u16Y);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.stCropRect.u16Width);
            handle.OnPrintOut(handle,"%7d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.stCropRect.u16Height);
            handle.OnPrintOut(handle,"%10d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.bHorMirror);
            handle.OnPrintOut(handle,"%10d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.bVerMirror);
            handle.OnPrintOut(handle,"%9d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.eDiType);
            handle.OnPrintOut(handle,"%13d", gstDivpChnCtx[u32ChnId].stChnAttrOrg.eRotateType);
            handle.OnPrintOut(handle,"%11d\r\n", gstDivpChnCtx[u32ChnId].stChnAttrOrg.eTnrLevel);

        }
    }
    handle.OnPrintOut(handle, "-------------------------- end dump CHN info --------------------------------\n");

    return MI_SUCCESS;
}

/*
N.B. use handle.OnWriteOut to print
*/
MI_S32 mi_divp_OnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t handle,MI_U32 u32DevId,void *pUsrData)
{
    MI_U32 u32ChnId = 0;//TODO:channel ID
    DBG_INFO("u32DevId=%u\n",u32DevId);
    ///TODO:
    ///dump  info base u32InPortId
    handle.OnPrintOut(handle,"\r\n-------------------------- start dump INPUT PORT info --------------------------------\n");
    handle.OnPrintOut(handle,"%7s%10s%8s%7s%8s%7s%12s%8s\n","ChnId","InputChg","bIPChg", "Width","Height","Pixel","GetBuffCnt","2Pmode");

    for(u32ChnId = 0; u32ChnId < (MI_DIVP_CHN_MAX_NUM +1); u32ChnId++)
    {
        if(gstDivpChnCtx[u32ChnId].eStatus > E_MI_DIVP_CHN_INITED)
        {
            handle.OnPrintOut(handle, "%7d", u32ChnId);
            handle.OnPrintOut(handle, "%10d", gstDivpChnCtx[u32ChnId].bInputChange);
            handle.OnPrintOut(handle, "%8d", gstDivpChnCtx[u32ChnId].bIPchg);
            handle.OnPrintOut(handle, "%7d", gstDivpChnCtx[u32ChnId].stInputInfoPre.u16Width);
            handle.OnPrintOut(handle, "%8d", gstDivpChnCtx[u32ChnId].stInputInfoPre.u16Height);
            handle.OnPrintOut(handle, "%7d", gstDivpChnCtx[u32ChnId].stInputInfoPre.ePxlFmt);
            handle.OnPrintOut(handle, "%12llu", gstDivpChnCtx[u32ChnId].u64GetBufferCnt);
            handle.OnPrintOut(handle, "%8d\n", gstDivpChnCtx[u32ChnId].b2Pmode);
            //gstDivpChnCtx[u32ChnId].u64GetBufferCnt = 0;
        }
    }

    handle.OnPrintOut(handle,"-------------------------- end dump INPUT PORT info --------------------------------\n");
    return MI_SUCCESS;
}

/*
N.B. use handle.OnWriteOut to print
*/
MI_S32 mi_divp_OnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t handle,MI_U32 u32DevId, void *pUsrData)
{
    MI_U32 u32ChnId = 0;//TODO:channel ID
    DBG_INFO("u32DevId=%u \n",u32DevId);
    ///TODO:
    ///dump  info base u32OutPortId
    handle.OnPrintOut(handle,"\r\n-------------------------- start dump OUTPUT PORT info --------------------------------\n");
    handle.OnPrintOut(handle,"%7s%11s%7s%8s%7s%10s%10s%11s%10s%8s%11s%5s\n","ChnId","OutputChg","Width","Height","Pixel","CompMode","PreWidth","PreHeight","PrePixel","GetCnt","FinishCnt","fps");
    for(u32ChnId = 0; u32ChnId < (MI_DIVP_CHN_MAX_NUM +1); u32ChnId++)
    {
        if(gstDivpChnCtx[u32ChnId].eStatus > E_MI_DIVP_CHN_INITED)
        {
            handle.OnPrintOut(handle, "%7d", u32ChnId);
            handle.OnPrintOut(handle, "%11d", gstDivpChnCtx[u32ChnId].bOutputChange);
            handle.OnPrintOut(handle, "%7d", gstDivpChnCtx[u32ChnId].stOutputPortAttr.u32Width);
            handle.OnPrintOut(handle, "%8d", gstDivpChnCtx[u32ChnId].stOutputPortAttr.u32Height);
            handle.OnPrintOut(handle, "%7d", gstDivpChnCtx[u32ChnId].stOutputPortAttr.ePixelFormat);
            handle.OnPrintOut(handle, "%10d", gstDivpChnCtx[u32ChnId].stOutputPortAttr.eCompMode);

            handle.OnPrintOut(handle, "%10d", gstDivpChnCtx[u32ChnId].stOutputInfoPre.u16Width);
            handle.OnPrintOut(handle, "%11d", gstDivpChnCtx[u32ChnId].stOutputInfoPre.u16Height);
            handle.OnPrintOut(handle, "%10d", gstDivpChnCtx[u32ChnId].stOutputInfoPre.ePxlFmt);
            handle.OnPrintOut(handle, "%8llu", gstDivpChnCtx[u32ChnId].u64PrepareOutBufCnt);
            handle.OnPrintOut(handle, "%11llu", gstDivpChnCtx[u32ChnId].u64FinishOutBufCnt);
            handle.OnPrintOut(handle, "%5d\n", gstDivpChnCtx[u32ChnId].u8PortFps);
            //gstDivpChnCtx[u32ChnId].u64PrepareOutBufCnt = 0;
            //gstDivpChnCtx[u32ChnId].u64FinishOutBufCnt = 0;
        }
    }
    handle.OnPrintOut(handle,"-------------------------- end dump OUTPUT PORT info --------------------------------\n");

    return MI_SUCCESS;
}

/*
N.B. use handle.OnWriteOut to print
*/

MI_S32 mi_divp_CheckFrameID(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_DIVP_INITIAL_ERROR_CODE;
    MI_DIVP_CHN DivpChn;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 2)
    {
        DivpChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(gstDivpChnCtx[DivpChn].eStatus == E_MI_DIVP_CHN_STARTED)
        {
            if (strcmp(argv[2], "ON") == 0 && gstDivpChnCtx[DivpChn].bCheckFrameID == FALSE)
            {
               gstDivpChnCtx[DivpChn].bCheckFrameID = TRUE;
            }
            else if (strcmp(argv[2], "OFF") == 0 && gstDivpChnCtx[DivpChn].bCheckFrameID == TRUE)
            {
                gstDivpChnCtx[DivpChn].bCheckFrameID = FALSE;
            }
            else
            {
                handle.OnPrintOut(handle, "argv[2] is error[ON, OFF].\n");
                return MI_DIVP_INITIAL_ERROR_CODE;
            }
        }
        else
        {
            handle.OnPrintOut(handle, "channelID[%d] is error not Enable.\n", DivpChn);
            return MI_DIVP_INITIAL_ERROR_CODE;
        }

        handle.OnPrintOut(handle,"ChanID %d bCheckFrameID %d \r\n", DivpChn,gstDivpChnCtx[DivpChn].bCheckFrameID);
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "DIVP CHECK FRAMEID CHANNEL ID; [ON, OFF];.\n");
        s32Ret = MI_DIVP_INITIAL_ERROR_CODE;
    }

    return s32Ret;
}

MI_S32 mi_divp_StopOneChnl(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_DIVP_INITIAL_ERROR_CODE;
    MI_DIVP_CHN DivpChn;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 2)
    {
        DivpChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(gstDivpChnCtx[DivpChn].eStatus != E_MI_DIVP_CHN_STARTED)
        {
            handle.OnPrintOut(handle, "channelID[%d] is error not Enable.\n", DivpChn);
            return MI_DIVP_INITIAL_ERROR_CODE;
        }

        if (strcmp(argv[2], "ON") == 0)
        {
            gbStopOneChnl[DivpChn] = TRUE;
        }
        else if (strcmp(argv[2], "OFF") == 0)
        {
            gbStopOneChnl[DivpChn] = FALSE;
        }
        else
        {
            handle.OnPrintOut(handle, "argv[2] is error[ON, OFF].\n", DivpChn);
            return MI_DIVP_INITIAL_ERROR_CODE;
        }

        handle.OnPrintOut(handle,"ChanID %d bStop %d \r\n", DivpChn, gbStopOneChnl[DivpChn]);
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "DIVP STOP ONE CHANNEL CHANNEL ID; [ON, OFF];\n");
        s32Ret = MI_DIVP_INITIAL_ERROR_CODE;
    }

    return s32Ret;
}

MI_S32 mi_divp_ProcessOneChnl(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_DIVP_INITIAL_ERROR_CODE;
    MI_DIVP_CHN DivpChn;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 2)
    {
        DivpChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(gstDivpChnCtx[DivpChn].eStatus != E_MI_DIVP_CHN_STARTED)
        {
            handle.OnPrintOut(handle, "channelID[%d] is error not Enable.\n", DivpChn);
            return MI_DIVP_INITIAL_ERROR_CODE;
        }
        else
        {
            gstProcessOneChnl.u32ChnId = DivpChn;
        }

        if (strcmp(argv[2], "ON") == 0)
        {
            gstProcessOneChnl.bEnable = TRUE;
            gstProcessOneChnl.u16AddtaskCnt = 0;
        }
        else if (strcmp(argv[2], "OFF") == 0)
        {
            gstProcessOneChnl.bEnable = FALSE;
        }
        else
        {
            handle.OnPrintOut(handle, "argv[2] is error[ON, OFF].\n", DivpChn);
            return MI_DIVP_INITIAL_ERROR_CODE;
        }

        handle.OnPrintOut(handle,"ChanID %d bEnable %d \r\n", DivpChn, gstProcessOneChnl.bEnable);
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "DIVP DISPOS ONE CHANNEL CHANNEL ID; [ON, OFF];\n");
        s32Ret = MI_DIVP_INITIAL_ERROR_CODE;
    }

    return s32Ret;
}

MI_S32 mi_divp_PorcSetChnl2Pmode(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_DIVP_INITIAL_ERROR_CODE;
    MI_DIVP_CHN DivpChn;
    MI_U8 u8Pmode;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 3)
    {
        DivpChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(gstDivpChnCtx[DivpChn].eStatus != E_MI_DIVP_CHN_STARTED)
        {
            handle.OnPrintOut(handle, "channelID[%d] is error start.\n", DivpChn);
            return MI_DIVP_INITIAL_ERROR_CODE;
        }

        if (strcmp(argv[2], "ON") == 0)
        {
            gstDivpChnCtx[DivpChn].bProcSet2Pmode = TRUE;
        }
        else if (strcmp(argv[2], "OFF") == 0)
        {
            gstDivpChnCtx[DivpChn].bProcSet2Pmode = FALSE;
        }
        else
        {
            handle.OnPrintOut(handle, "argv[2] is error[ON, OFF].\n", DivpChn);
            return MI_DIVP_INITIAL_ERROR_CODE;
        }

        u8Pmode = (MI_U8)simple_strtoul(argv[3], NULL, 10);
        if(u8Pmode == 0 || u8Pmode ==1)
        {
            gstDivpChnCtx[DivpChn].b2Pmode = u8Pmode;
        }
        else
        {
            handle.OnPrintOut(handle,"1 enable 2pmode, 0 disable 2pmode, user set %s\n", argv[1]);
            return MI_DIVP_INITIAL_ERROR_CODE;
        }
        
        handle.OnPrintOut(handle,"ChanID %d, %s, set2Pmode %d\r\n", DivpChn, argv[2], gstDivpChnCtx[DivpChn].b2Pmode);
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "DIVP SetChannel 2pmode CHANNEL ID; [ON, OFF];\n");
        s32Ret = MI_DIVP_INITIAL_ERROR_CODE;
    }

    return s32Ret;
}

/*
N.B. use handle.OnWriteOut to print
*/
MI_S32 mi_divp_OnHelp(MI_SYS_DEBUG_HANDLE_t  handle,MI_U32  u32DevId,void *pUsrData)
{
    DBG_INFO("\n");
    ///TODO:
    //show mod's private help
    return MI_SUCCESS;
}
#endif

MI_S32 mi_divp_Init(void)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32Index = 0;
    MHAL_CMDQ_CmdqInterface_t *pstCmdInf = NULL;
    mi_sys_ModuleDevBindOps_t stDivpOp;
    mi_sys_ModuleDevInfo_t stModInfo;
    MHAL_CMDQ_BufDescript_t stBuffwrInfo;
    MI_U32 u32IrqId = 0;
    MI_S32 s32IrqRet = 0;
    MS_U32 u32DivpCaps = 0;

#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
#endif

    stDivpOp.OnBindInputPort = mi_divp_OnBindInputPort;
    stDivpOp.OnUnBindInputPort = mi_divp_OnUnBindInputPort;
    stDivpOp.OnBindOutputPort = mi_divp_OnBindOutputPort;
    stDivpOp.OnUnBindOutputPort = mi_divp_OnUnBindOutputPort;
    stDivpOp.OnOutputPortBufRelease = NULL;

    memset(&stModInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId = E_MI_MODULE_ID_DIVP;
    stModInfo.u32DevId = E_MI_DIVP_Device0;
    stModInfo.u32DevChnNum = MI_DIVP_CHN_MAX_NUM + 1;
    stModInfo.u32InputPortNum = MI_DIVP_CHN_INPUT_PORT_NUM;
    stModInfo.u32OutputPortNum = MI_DIVP_CHN_OUTPUT_PORT_NUM;

    if(_gbInited)
    {
        _gu64InitTimes ++;
        DBG_INFO("DIVP has been inited. _gu64InitTimes = %llu. \n", _gu64InitTimes);
        return MI_SUCCESS;
    }

    memset(&stBuffwrInfo, 0, sizeof(stBuffwrInfo));
    stBuffwrInfo.u32CmdqBufSize = 4096 *8;
    stBuffwrInfo.u32CmdqBufSizeAlign = 32;//256bit allgin
    stBuffwrInfo.u32MloadBufSize = 0;
    stBuffwrInfo.u16MloadBufSizeAlign = 1;
    pstCmdInf = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_DIVP, &stBuffwrInfo, FALSE);
    if(NULL == pstCmdInf)
    {
        DBG_ERR("NULL == pstCmdInf.\n");
    }
    else
    {
        //reset fence in dummy register
        pstCmdInf->MHAL_CMDQ_WriteDummyRegCmdq(pstCmdInf, 0x0000);
    }

    ///1 init DIVP HW and globle variable
    for(u32Index = 0; u32Index < E_MHAL_DIVP_Device_MAX; u32Index ++)
    {
        if((MHAL_SUCCESS == MHAL_DIVP_GetChipCapbility((MHAL_DIVP_DeviceId_e)u32Index, &u32DivpCaps))
            && (u32DivpCaps & 0x01))//bit0 = 1 means divp device exist.
        {
            if(MHAL_SUCCESS != MHAL_DIVP_Init((MHAL_DIVP_DeviceId_e)u32Index))
            {
                s32Ret = MI_DIVP_ERR_FAILED;
                DBG_ERR("deinit channel %u fail! \n", u32Index);
            }
            else
            {
                gu32DevNum ++;
            }
        }
        else
        {
            DBG_ERR("u32DivpCaps = 0x%x, u32Index = %u. \n", u32DivpCaps, u32Index);
        }
    }

    memset(gstDivpChnCtx, 0, sizeof(gstDivpChnCtx));
    for(u32Index = 0; u32Index <= MI_DIVP_CHN_MAX_NUM; u32Index ++)
    {
        gstDivpChnCtx[u32Index].u32ChnId = MI_DIVP_CHN_NULL;
        gstDivpChnCtx[u32Index].stChnAttrPre.bHorMirror = FALSE;
        gstDivpChnCtx[u32Index].stChnAttrPre.bVerMirror = FALSE;
        gstDivpChnCtx[u32Index].stChnAttrPre.eDiType = E_MI_DIVP_DI_TYPE_NUM;//only inited
        gstDivpChnCtx[u32Index].stChnAttrPre.eRotateType = E_MI_SYS_ROTATE_NUM;
        gstDivpChnCtx[u32Index].stChnAttrPre.eTnrLevel = E_MI_DIVP_TNR_LEVEL_NUM;

        //init semaphore
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
        init_MUTEX(&gChnCtxSem[u32Index]);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
        sema_init(&(gChnCtxSem[u32Index]), 1);
#endif
    }

#ifdef MI_SYS_PROC_FS_DEBUG

    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
#if (MI_DIVP_PROCFS_DEBUG == 1)
    pstModuleProcfsOps.OnDumpDevAttr = mi_divp_OnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = mi_divp_OnDumpChannelAttr;
    pstModuleProcfsOps.OnDumpInputPortAttr = mi_divp_OnDumpInputPortAttr;
    pstModuleProcfsOps.OnDumpOutPortAttr = mi_divp_OnDumpOutPortAttr;
    pstModuleProcfsOps.OnHelp = mi_divp_OnHelp;
    printk("DIVP pstModuleProcfsOps %p %p %p %p %p\n",&pstModuleProcfsOps,mi_divp_OnDumpDevAttr,mi_divp_OnDumpChannelAttr,mi_divp_OnDumpInputPortAttr,mi_divp_OnDumpOutPortAttr);
#else
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
#endif

#endif

    ///2 register DIVP module to system.
    gDivpDevHdl = mi_sys_RegisterDev(&stModInfo, &stDivpOp, NULL
                                      #ifdef MI_SYS_PROC_FS_DEBUG
                                      , &pstModuleProcfsOps
                                      ,MI_COMMON_GetSelfDir
                                      #endif
                                      );

#if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
    mi_sys_RegistCommand("checkframeid", 2, mi_divp_CheckFrameID, gDivpDevHdl);
    mi_sys_RegistCommand("processonechannel", 2, mi_divp_ProcessOneChnl, gDivpDevHdl);
    mi_sys_RegistCommand("stoponechannel", 2, mi_divp_StopOneChnl, gDivpDevHdl);
    mi_sys_RegistCommand("set2pmode", 3, mi_divp_PorcSetChnl2Pmode, gDivpDevHdl);
#endif

    DBG_INFO("gDivpDevHdl = %p.\n", gDivpDevHdl);

    ///3 create thread to deal with tasks of all channels.
    gpstWorkThread = kthread_create(mi_divp_WorkThread, pstCmdInf, "divp_work_thread");
    if (!IS_ERR(gpstWorkThread))
    {
        wake_up_process(gpstWorkThread);
    }
    else
    {
        DBG_ERR("create thread fail: mi_divp_WorkThread.\n");
    }

    ///4 create thread to recieve ISR and release unused buffer
    gpstIsrThread = kthread_create(mi_divp_IsrProcessThread, pstCmdInf, "divp_isr_thread");
    if (!IS_ERR(gpstWorkThread))
    {
        wake_up_process(gpstIsrThread);
    }
    else
    {
        DBG_ERR("create thread fail: mi_divp_IsrProcessThread\n");
    }

    MHAL_DIVP_GetIrqNum(&u32IrqId);
    DBG_INFO("u32IrqId = %d.\n", u32IrqId);
    s32IrqRet = request_irq(u32IrqId, mi_divp_Isr, NULL, "mi_divp_isr", NULL);
    if(0 != s32IrqRet)
    {
        DBG_ERR("request_irq failed. u32IrqId = %u, s32IrqRet = %d.\n\n ", u32IrqId, s32IrqRet);
    }
    MHAL_DIVP_EnableFrameDoneIsr(TRUE);

    ///5 create private channel for capture display creen and capture DRAM data.
    _mi_divp_CreatePrivateChannel(pstCmdInf);

    _gbInited = TRUE;
    _gu64InitTimes ++;
    DBG_INFO("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

MI_S32 mi_divp_DeInit(void)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32Index = 0;
    MI_U32 u32IrqId = 0;
    MS_U32 u32DivpCaps = 0;
    DBG_INFO("\n");

    if(!_gbInited)
    {
        DBG_INFO("DIVP is not initialized. \n", _gu64InitTimes);
        return s32Ret;
    }

    if(_gu64InitTimes > 1)
    {
        _gu64InitTimes --;
        DBG_INFO("Don't deinit DIVP HW. _gu64InitTimes = %llu. \n", _gu64InitTimes);
        return s32Ret;
    }

    ///destroy private channel for capture display creen and capture DRAM data.
    gstDivpChnCtx[MI_DIVP_CHN_MAX_NUM].eStatus = E_MI_DIVP_CHN_DISTROYED;
    gstDivpChnCtx[MI_DIVP_CHN_MAX_NUM].u32ChnId = MI_DIVP_CHN_NULL;

    //check channel status
    for(u32Index = 0; u32Index <= MI_DIVP_CHN_MAX_NUM; u32Index ++)
    {
        if(MI_DIVP_CHN_NULL != gstDivpChnCtx[u32Index].u32ChnId)
        {
            MI_IMPL_DIVP_StopChn(u32Index);
            s32Ret = MI_IMPL_DIVP_DestroyChn(u32Index);
            DBG_ERR(" Force destroying channel %u ---%s.\n", u32Index, s32Ret==MI_SUCCESS?"success":"failed");
        }
    }

  //  if(MI_SUCCESS == s32Ret)
    {
        MHAL_DIVP_EnableFrameDoneIsr(FALSE);

        MHAL_DIVP_GetIrqNum(&u32IrqId);
        free_irq(u32IrqId, NULL);

        kthread_stop(gpstWorkThread);
        kthread_stop(gpstIsrThread);

        mi_sys_UnRegisterDev(gDivpDevHdl);
        gDivpDevHdl = NULL;

        for(u32Index = 0; u32Index < E_MHAL_DIVP_Device_MAX; u32Index ++)
        {
            if((MHAL_SUCCESS == MHAL_DIVP_GetChipCapbility((MHAL_DIVP_DeviceId_e)u32Index, &u32DivpCaps))
                && (u32DivpCaps & 0x01))//bit0 = 1 means divp device exist.
            {
                if(MHAL_SUCCESS != MHAL_DIVP_DeInit((MHAL_DIVP_DeviceId_e)u32Index))
                {
                    s32Ret = MI_DIVP_ERR_FAILED;
                    DBG_ERR("deinit channel %u fail! \n", u32Index);
                }
            }
            else
            {
                DBG_ERR("u32DivpCaps = 0x%x, u32Index = %u. \n", u32DivpCaps, u32Index);
            }
        }
    }

    MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_DIVP);
    _gu64InitTimes --;
    _gbInited = FALSE;
    gu32DevNum = 0;
    gu32ChnNum = 0;
    DBG_INFO("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

MI_S32 mi_divp_CaptureTiming(mi_divp_CaptureInfo_t* pstCapInfo)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///1 check input parameter
    MI_DIVP_CHECK_POINTER(pstCapInfo)
    DBG_INFO("phyBufAddr[0] = 0x%llx, u32BufSize = 0x%x, u16Width = %u, u16Height = %u\n",
        pstCapInfo->phyBufAddr[0], pstCapInfo->u32BufSize, pstCapInfo->u16Width, pstCapInfo->u16Height);
    DBG_INFO("u16Stride[0] = %u, u16Stride[1] = %u, u16Stride[2] = %u, eDispId = %d, eCapStage = %d\n",
        pstCapInfo->u16Stride[0], pstCapInfo->u16Stride[1],pstCapInfo->u16Stride[2],pstCapInfo->eDispId, pstCapInfo->eCapStage);
    DBG_INFO("eInputPxlFmt = %d, eOutputPxlFmt = %d, eRotateType = %d, bHMirror = %d, bVMirror = %d\n",
        pstCapInfo->eInputPxlFmt, pstCapInfo->eOutputPxlFmt, pstCapInfo->eRotate, pstCapInfo->stMirror.bHMirror, pstCapInfo->stMirror.bVMirror);
    DBG_INFO("crop(x, y, w, h) = (%u, %u, %u, %u). \n", pstCapInfo->stCropWin.u16X, pstCapInfo->stCropWin.u16Y, pstCapInfo->stCropWin.u16Width, pstCapInfo->stCropWin.u16Height);

    ///2 add task to capture task list.
    #if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
    if(gstProcessOneChnl.bEnable == TRUE)
    {
        if(gstProcessOneChnl.u32ChnId == MI_DIVP_CHN_MAX_NUM + 1
            && gstProcessOneChnl.u16AddtaskCnt == 1)
                return s32Ret;
    }
    #endif

    MI_DIVP_LOCK_TODO_CAPTURE_TASK_LIST();
    list_add_tail(&pstCapInfo->capture_list, &divp_todo_capture_task_list);
    MI_DIVP_UNLOCK_TODO_CAPTURE_TASK_LIST();

    #if defined(MI_SYS_PROC_FS_DEBUG) &&(MI_DIVP_PROCFS_DEBUG == 1)
    if(gstProcessOneChnl.bEnable == TRUE
        &&gstProcessOneChnl.u32ChnId == MI_DIVP_CHN_MAX_NUM + 1)
            gstProcessOneChnl.u16AddtaskCnt ++;
    #endif

    DBG_INFO("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

//global APIs======================================================================================================
MI_S32 MI_IMPL_DIVP_RefreshChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    mi_sys_ChnTaskInfo_t *pstChnTask = NULL;
    DBG_ENTER("DivpChn = %u. \n",DivpChn);

    MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DivpChn);
    pstChnTask = gstDivpChnCtx[DivpChn].pstLastDelayFreeTask;
    gstDivpChnCtx[DivpChn].pstLastDelayFreeTask = NULL;
    MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DivpChn);

    if(!pstChnTask)
    {
        DBG_ERR("there is no task for this channel. DivpChn = %u. \n", DivpChn);
        s32Ret = MI_DIVP_ERR_NO_CONTENT;
        DBG_EXIT_ERR();
    }
    else
    {
        DBG_INFO("mi_sys_RewindTask: pstChnTask = %p. \n", pstChnTask);
        mi_sys_RewindTask(pstChnTask);
        DBG_EXIT_OK();
    }
    return s32Ret;
}

MI_S32 MI_IMPL_DIVP_CreateChn (MI_DIVP_CHN DivpChn, MI_DIVP_ChnAttr_t* pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MHAL_DIVP_DeviceId_e eDevId = E_MHAL_DIVP_Device0;
    MI_SYS_FrameBufExtraConfig_t  stInputAlignment;

    memset(&stInputAlignment, 0, sizeof(stInputAlignment));
    //check input parameter
    MI_DIVP_CHECK_POINTER(pstAttr);
    DBG_ENTER("DivpChn = %d, bHorMirror = %d, bVerMirror = %d, eDiType = %d, eRotateType = %d, eTnrLevel = %d, \n  u32MaxWidth = %u, u32MaxHeight = %u, crop(x, y, w, h) = (%u, %u, %u, %u). \n",
        DivpChn, pstAttr->bHorMirror, pstAttr->bVerMirror, pstAttr->eDiType, pstAttr->eRotateType, pstAttr->eTnrLevel, pstAttr->u32MaxWidth,
        pstAttr->u32MaxHeight, pstAttr->stCropRect.u16X, pstAttr->stCropRect.u16Y, pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);

    ///check HW resources
    if(gu32ChnNum >= MI_DIVP_CHN_MAX_NUM)
    {
        s32Ret = MI_DIVP_ERR_NO_RESOUCE;
        DBG_EXIT_ERR();
    }
    else if(MI_DIVP_CHN_NULL != gstDivpChnCtx[DivpChn].u32ChnId)
    {
        s32Ret = MI_DIVP_ERR_HAS_CREATED;
        DBG_EXIT_ERR();
    }
    else
    {
        s32Ret = _mi_divp_CheckAttrValue(pstAttr);
        if(MI_SUCCESS == s32Ret)
        {
            if(0 == (DivpChn % gu32DevNum))
            {
                eDevId = E_MHAL_DIVP_Device0;
            }
            else if(1 == (DivpChn % gu32DevNum))
            {
                eDevId = E_MHAL_DIVP_Device1;
            }
            else if(2 == (DivpChn % gu32DevNum))
            {
                eDevId = E_MHAL_DIVP_Device2;
            }

            if(MHAL_SUCCESS == MHAL_DIVP_CreateInstance(eDevId, pstAttr->u32MaxWidth, pstAttr->u32MaxHeight,
                                            mi_divp_Alloc, mi_divp_Free, DivpChn, &gstDivpChnCtx[DivpChn].pHalCtx))
            {
                MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DivpChn);
                //create channel ID and save channel attribute
                gstDivpChnCtx[DivpChn].u32ChnId = DivpChn;
                gstDivpChnCtx[DivpChn].eStatus = E_MI_DIVP_CHN_CREATED;

                memcpy(&gstDivpChnCtx[DivpChn].stChnAttr, pstAttr, sizeof(MI_DIVP_ChnAttr_t));
                memcpy(&gstDivpChnCtx[DivpChn].stChnAttrOrg, pstAttr, sizeof(MI_DIVP_ChnAttr_t));
                gstDivpChnCtx[DivpChn].stChnAttrPre.u32MaxWidth = pstAttr->u32MaxWidth;
                gstDivpChnCtx[DivpChn].stChnAttrPre.u32MaxHeight = pstAttr->u32MaxHeight;
                gstDivpChnCtx[DivpChn].bChnAttrChg = TRUE;
                gu32ChnNum ++;

                if((pstAttr->eRotateType == E_MI_SYS_ROTATE_90)||(pstAttr->eRotateType == E_MI_SYS_ROTATE_270))
                {
                    stInputAlignment.u16BufHAlignment = 64;
                    stInputAlignment.u16BufVAlignment = 64;
                    stInputAlignment.bClearPadding = FALSE;
                    if(MI_SUCCESS != mi_sys_SetInputPortBufExtConf(gDivpDevHdl, DivpChn, 0, &stInputAlignment))
                    {
                        DBG_ERR("Chnid %d Set Input port ext conf\n", DivpChn);
                    }
                }
                
                MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DivpChn);
                DBG_EXIT_OK();
            }
            else
            {
                DBG_EXIT_ERR(" Fail to create instance in HAL layer.\n");
                s32Ret = MI_DIVP_ERR_FAILED;
            }
        }
        else
        {
            DBG_EXIT_ERR(" Fail to create channel. invalid input parameter.\n");
        }
    }

    DBG_INFO("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

MI_S32 MI_IMPL_DIVP_DestroyChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32Cnt = 0;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn);
    DBG_ENTER("DivpChn = %d.\n", DivpChn);

    ///2 check channel status
    if (E_MI_DIVP_CHN_STARTED == gstDivpChnCtx[DivpChn].eStatus)
    {
        DBG_EXIT_ERR(" destroy channel fail. channel is not stoped. u32ChnId = %d.\n", DivpChn);
        s32Ret = MI_DIVP_ERR_CHN_NOT_STOPED;
    }
    else
    {
        while(_mi_divp_ChnBusy(DivpChn) && (s32Cnt < 100))
        {
            schedule_timeout_interruptible(msecs_to_jiffies(10));//sleep 10ms
            s32Cnt ++;
        }

        if(s32Cnt == 100)
        {
            s32Ret = MI_DIVP_ERR_CHN_BUSY;
            DBG_EXIT_ERR();
        }

        ///3 destroy channel instance in HAL layer.
        MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DivpChn);
        if((MI_SUCCESS == s32Ret) && (MHAL_SUCCESS == MHAL_DIVP_DestroyInstance(gstDivpChnCtx[DivpChn].pHalCtx)))
        {
            ///4 reset channel context
            memset(&gstDivpChnCtx[DivpChn], 0, sizeof(mi_divp_ChnContex_t));
            gstDivpChnCtx[DivpChn].u32ChnId = MI_DIVP_CHN_NULL;
            gstDivpChnCtx[DivpChn].eStatus = E_MI_DIVP_CHN_DISTROYED;
            gu32ChnNum --;
        }
        else
        {
            DBG_EXIT_ERR(" Fail to destroy channel. s32Ret = %d \n", s32Ret);
            s32Ret = MI_DIVP_ERR_FAILED;
        }

        if(gstDivpChnCtx[DivpChn].pstLastDelayFreeTask)
        {
            mi_sys_FinishAndReleaseTask(gstDivpChnCtx[DivpChn].pstLastDelayFreeTask);
            gstDivpChnCtx[DivpChn].pstLastDelayFreeTask = NULL;
        }
        MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DivpChn);
        DBG_EXIT_OK();
    }

    DBG_INFO("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

MI_S32 MI_IMPL_DIVP_SetChnAttr(MI_DIVP_CHN DivpChn, MI_DIVP_ChnAttr_t* pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn);
    MI_DIVP_CHECK_POINTER(pstAttr);

    DBG_ENTER("DivpChn = %d, bHorMirror = %d, bVerMirror = %d, eDiType = %d, eRotateType = %d, eTnrLevel = %d, \n  u32MaxWidth = %u, u32MaxHeight = %u, crop(x, y, w, h) = (%u, %u, %u, %u). \n",
        DivpChn, pstAttr->bHorMirror, pstAttr->bVerMirror, pstAttr->eDiType,
        pstAttr->eRotateType, pstAttr->eTnrLevel, pstAttr->u32MaxWidth, pstAttr->u32MaxHeight,
        pstAttr->stCropRect.u16X, pstAttr->stCropRect.u16Y, pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);

    ///2 check value of input parameter and save it.
    if(MI_SUCCESS == _mi_divp_CheckAttrValue(pstAttr))
    {
        MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DivpChn);
        if(memcmp(&gstDivpChnCtx[DivpChn].stChnAttr, pstAttr, sizeof(MI_DIVP_ChnAttr_t)))
        {
            gstDivpChnCtx[DivpChn].bChnAttrChg = TRUE;

            memcpy(&(gstDivpChnCtx[DivpChn].stChnAttr), pstAttr, sizeof(MI_DIVP_ChnAttr_t));
            memcpy(&gstDivpChnCtx[DivpChn].stChnAttrOrg, pstAttr, sizeof(MI_DIVP_ChnAttr_t));
        }
        MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DivpChn);
        DBG_EXIT_OK();
    }
    else
    {
        DBG_EXIT_ERR(" invalid channel attribute!!\n");
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }

    DBG_INFO("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

MI_S32 MI_IMPL_DIVP_GetChnAttr(MI_DIVP_CHN DivpChn, MI_DIVP_ChnAttr_t* pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn);
    MI_DIVP_CHECK_POINTER(pstAttr);
    DBG_ENTER("Chn%d\n", DivpChn);
    MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DivpChn);
    memcpy(pstAttr, &gstDivpChnCtx[DivpChn].stChnAttr, sizeof(MI_DIVP_ChnAttr_t));
    MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DivpChn);

    DBG_EXIT_OK("DivpChn = %d, bHorMirror = %d, bVerMirror = %d, eDiType = %d, eRotateType = %d, eTnrLevel = %d, \n  u32MaxWidth = %u, u32MaxHeight = %u, crop(x, y, w, h) = (%u, %u, %u, %u). s32Ret = 0x%x.\n",
        DivpChn, pstAttr->bHorMirror, pstAttr->bVerMirror, pstAttr->eDiType, pstAttr->eRotateType, pstAttr->eTnrLevel,
        pstAttr->u32MaxWidth, pstAttr->u32MaxHeight,  pstAttr->stCropRect.u16X,
        pstAttr->stCropRect.u16Y, pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height, s32Ret);
    return s32Ret;
}

MI_S32 MI_IMPL_DIVP_StartChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn);
    DBG_ENTER("DivpChn = %d.\n", DivpChn);

    gstDivpChnCtx[DivpChn].eStatus = E_MI_DIVP_CHN_STARTED;

    DBG_INFO("gDivpDevHdl = %p, DivpChn = %d.\n", gDivpDevHdl, DivpChn);
    s32Ret = mi_sys_EnableOutputPort(gDivpDevHdl, DivpChn, DIVP_CHN_OUTPUT_PORT_ID);
    s32Ret |= mi_sys_EnableInputPort(gDivpDevHdl, DivpChn, DIVP_CHN_INPUT_PORT_ID);
    s32Ret |= mi_sys_EnableChannel(gDivpDevHdl, DivpChn);
    DBG_EXIT_OK("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

MI_S32 MI_IMPL_DIVP_StopChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn);
    DBG_ENTER("DivpChn = %d.\n", DivpChn);

    mi_sys_DisableChannel(gDivpDevHdl, DivpChn);
    mi_sys_DisableOutputPort(gDivpDevHdl, DivpChn, DIVP_CHN_OUTPUT_PORT_ID);
    mi_sys_DisableInputPort(gDivpDevHdl, DivpChn, DIVP_CHN_INPUT_PORT_ID);

    if(gstDivpChnCtx[DivpChn].eStatus <= E_MI_DIVP_CHN_STARTED)
    {
        gstDivpChnCtx[DivpChn].eStatus = E_MI_DIVP_CHN_STOPED;
    }
    else
    {
        DBG_INFO("channel has been stoped. eStatus = %u.\n", gstDivpChnCtx[DivpChn].eStatus);
    }

    MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DivpChn);
    if(gstDivpChnCtx[DivpChn].pstLastDelayFreeTask)
    {
        mi_sys_FinishAndReleaseTask(gstDivpChnCtx[DivpChn].pstLastDelayFreeTask);
        gstDivpChnCtx[DivpChn].pstLastDelayFreeTask = NULL;
    }
    MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DivpChn);

    DBG_EXIT_OK("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

MI_S32 MI_IMPL_DIVP_SetOutputPortAttr(MI_DIVP_CHN DivpChn, MI_DIVP_OutputPortAttr_t * pstOutputPortAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn);
    MI_DIVP_CHECK_POINTER(pstOutputPortAttr);
    DBG_ENTER("DivpChn = %d. port Width = %u, Height = %u, pixelformat = %u, eCompressMode = %d.\n", DivpChn, pstOutputPortAttr->u32Width,
        pstOutputPortAttr->u32Height, pstOutputPortAttr->ePixelFormat, pstOutputPortAttr->eCompMode);

    ///2 check HW limitation of scaling and pixel format
    if((MI_DIVP_OUTPUT_WIDTH_MAX < pstOutputPortAttr->u32Width)//max output size 4096x2160
        || (MI_DIVP_OUTPUT_HEIGHT_MAX < pstOutputPortAttr->u32Height)
        || (MI_DIVP_OUTPUT_WIDTH_MIN > pstOutputPortAttr->u32Width)//mimun output size 128X64
        || (MI_DIVP_OUTPUT_HEIGHT_MIN > pstOutputPortAttr->u32Height)
        || ((E_MI_SYS_PIXEL_FRAME_YUV422_YUYV != pstOutputPortAttr->ePixelFormat)
            && (E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 != pstOutputPortAttr->ePixelFormat)
            && (E_MI_SYS_PIXEL_FRAME_YUV_MST_420 != pstOutputPortAttr->ePixelFormat))
        || (E_MI_SYS_COMPRESS_MODE_NONE != pstOutputPortAttr->eCompMode))
    {
        DBG_EXIT_ERR(" invalid output port attribute!!\n");
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }
    else
    {
        ///check width
        //pstOutputPortAttr->u32Width = (pstOutputPortAttr->u32Width + MI_DIVP_OUTPUT_WIDTH_ALIGNMENT - 1) & (~(MI_DIVP_OUTPUT_WIDTH_ALIGNMENT - 1));
        if((gstDivpChnCtx[DivpChn].stChnAttr.eRotateType == E_MI_SYS_ROTATE_90)
            ||(gstDivpChnCtx[DivpChn].stChnAttr.eRotateType == E_MI_SYS_ROTATE_270))
        {
            pstOutputPortAttr->u32Width = ALIGN_UP(pstOutputPortAttr->u32Width, 64);
            pstOutputPortAttr->u32Height = ALIGN_UP(pstOutputPortAttr->u32Height, 64);
        }
            
        ///3 save settings
        MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DivpChn);
        memcpy(&gstDivpChnCtx[DivpChn].stOutputPortAttr, pstOutputPortAttr, sizeof(MI_DIVP_OutputPortAttr_t));
        MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DivpChn);
    }

    DBG_EXIT_OK("DivpChn = %d. port Width = %u, Height = %u, pixelformat = %u, eCompressMode = %d.\n", DivpChn,
        gstDivpChnCtx[DivpChn].stOutputPortAttr.u32Width, gstDivpChnCtx[DivpChn].stOutputPortAttr.u32Height,
        gstDivpChnCtx[DivpChn].stOutputPortAttr.ePixelFormat, gstDivpChnCtx[DivpChn].stOutputPortAttr.eCompMode);
    DBG_EXIT_OK("s32Ret = 0x%x.\n", s32Ret);
    return s32Ret;
}

MI_S32 MI_IMPL_DIVP_GetOutputPortAttr(MI_DIVP_CHN DivpChn, MI_DIVP_OutputPortAttr_t * pstOutputPortAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn);
    MI_DIVP_CHECK_POINTER(pstOutputPortAttr);
    DBG_ENTER("chn%d\n", DivpChn);
    //copy settings
    MI_DIVP_LOCK_CHN_GLOBAL_VARIABLE(DivpChn);
    memcpy(pstOutputPortAttr, &gstDivpChnCtx[DivpChn].stOutputPortAttr, sizeof(MI_DIVP_OutputPortAttr_t));
    MI_DIVP_UNLOCK_CHN_GLOBAL_VARIABLE(DivpChn);

    DBG_EXIT_OK("DivpChn = %d. port Width = %u, Height = %u, pixelformat = %u, eCompressMode = %d, s32Ret = 0x%x\n", DivpChn, pstOutputPortAttr->u32Width,
        pstOutputPortAttr->u32Height, pstOutputPortAttr->ePixelFormat, pstOutputPortAttr->eCompMode, s32Ret);

    return s32Ret;
}


