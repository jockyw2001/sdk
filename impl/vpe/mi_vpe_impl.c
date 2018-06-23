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
#include <linux/wait.h>
#include <linux/math64.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/module.h>

#include "mi_print.h"
#include "mhal_common.h"
#include "mi_vpe_impl.h"
#include "mi_vpe.h"
#include "mi_vpe_impl_internal.h"
#include "mi_sys_sideband_msg.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_vif_internal.h"

static int PortNum=4;
module_param(PortNum,int,S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(PortNum,"Cam VpePortnum=3, Dvr VPE portNum =4");

#define USBCAM_PORTNUM 3

#include "../sys/mi_sys_buf_mgr.h"
#define VPE_SUPPORT_RGN (1)
#if defined(VPE_SUPPORT_RGN)  && (VPE_SUPPORT_RGN == 1)
//#include "cmdq_service.h"
#include "../rgn/mi_rgn_internal.h"
#define VPE_GET_TASK_RGN_FENCE(pstTask) ((pstTask)->u64Reserved6)
#else
#define VPE_GET_TASK_RGN_FENCE(pstTask)
#endif

#define VPE_ZOOM_RATION_DEN 0x80000UL //so we support 8K resulotion calculation where U32 overflow

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define SW_STUB_TEST 1
#define ROTATION_BEFORE_SACLING    (1)
#define VPE_ADJUST_THREAD_PRIORITY 0
#define VPE_TASK_PERF_DBG          0

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
// u32Reserved0: Fence
// u32Reserved1: 3DNR
// u64Reserved0: Menuload Buffer
// u64Reserved1: ROI Task Info

#define VPE_GOT_TASK_TIME(pstTask)           ((pstTask)->u64Reserved2)
#define VPE_PROCESS_TASK_TIME(pstTask)       ((pstTask)->u64Reserved3)
#define VPE_KICKOFF_TASK_TIME(pstTask)       ((pstTask)->u64Reserved4)
#define VPE_RELEASE_TASK_TIME(pstTask)       ((pstTask)->u64Reserved5)

#define VPE_RELEASE_BUFCNTMAX  2000
#define VPE_GET_INBUFCNT_MAX   20000

#define VPE_PERF_TIME(pu64Time) do {\
    struct timeval stTime1;\
    memset(&stTime1, 0, sizeof(stTime1));\
    do_gettimeofday(&stTime1);\
    *(pu64Time) = (stTime1.tv_sec*1000000 + stTime1.tv_usec);\
} while(0)
#else
#define VPE_PERF_TIME(pu64Time)
#define VPE_GOT_TASK_TIME(pstTask)
#define VPE_PROCESS_TASK_TIME(pstTask)
#define VPE_KICKOFF_TASK_TIME(pstTask)
#define VPE_RELEASE_TASK_TIME(pstTask)

#define VPE_PERF_TIME(pu64Time)
#endif

#if defined(VPE_TASK_PERF_DBG) && (VPE_TASK_PERF_DBG == 1)
#define VPE_PERF_PRINT(fmt, args...) do {printk("[VPE_PERF]: %s [%d] ", __func__, __LINE__); printk(fmt, ##args);} while(0)
#else
#define VPE_PERF_PRINT(fmt, args...)
#endif

#define MI_VPE_MAX_INPUTPORT_NUM             (1)
#define VPE_WORK_THREAD_WAIT                 (2)
#define VPE_OUTPUT_PORT_FOR_MDWIN            (3)
// TODO: Need Dummy register
#define MI_VPE_FENCE_REGISTER                (0x12345678)
#define FHD_SIZE                             ((1920*1088))
#ifndef SW_STUB_TEST
#define MI_VPE_FRAME_PER_BURST_CMDQ          (1)
#define VPE_PROC_WAIT                        (1)
#else
#define MI_VPE_FRAME_PER_BURST_CMDQ          (1)
#define VPE_PROC_WAIT                        (1)
#endif
//#define WITHOUT_IRQ
#define MIU_BURST_BITS        (256)
#define YUV422_PIXEL_ALIGN    (2)
#define YUV422_BYTE_PER_PIXEL (2)
#define YUV420_PIXEL_ALIGN    (2)
#define YUV420_BYTE_PER_PIXEL (1)
#define REPEAT_MAX_NUMBER (2)

#ifndef MI_SUCCESS
#define MI_SUCCESS (0)
#endif
#ifndef MI_SYS_ERR_BUSY
#define MI_SYS_ERR_BUSY (1)
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#endif
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, alignment) (( (val)/(alignment))*(alignment))
#endif
#define VPE_CMDQ_BUFF_SIZE_MAX (0x80000)
#define VPE_CMDQ_BUFF_ALIGN    (64)

#define VPE_MLOAD_BUFF_SIZE_MAX (0x100000)
#define VPE_MLOAD_BUFF_ALIGN    (64)

#define ExecFunc(result)\
    if (result != TRUE)\
    {\
        DBG_ERR("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }\
    else\
    {\
        DBG_EXIT_OK("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

#define MI_VPE_CHECK_CHNN_SUPPORTED(VpeCh)   (((VpeCh) >= 0) && ((VpeCh) < (MI_VPE_MAX_CHANNEL_NUM)))
#define MI_VPE_CHECK_PORT_SUPPORTED(PortNum) (((PortNum) >= 0) && ((PortNum) < (MI_VPE_MAX_PORT_NUM)))
#define MI_VPE_CHECK_CHNN_CREATED(VpeCh)       (MI_VPE_CHECK_CHNN_SUPPORTED((VpeCh)) && (_gVpeDevInfo.stChnnInfo[VpeCh].bCreated == TRUE))
#define GET_VPE_CHNN_PTR(VpeCh)              (&_gVpeDevInfo.stChnnInfo[(VpeCh)])
#define GET_VPE_PORT_PTR(VpeCh, VpePort)     (&_gVpeDevInfo.stChnnInfo[(VpeCh)].stOutPortInfo[(VpePort)])
#define GET_VPE_DEV_PTR()                    (&_gVpeDevInfo)
#define  MI_VPE_CHNN_DYNAMIC_ATTR_CHANGED(pstOldAttr, pstNewAttr) \
        (((pstOldAttr)->bContrastEn != (pstNewAttr)->bContrastEn)\
            || ((pstOldAttr)->bEdgeEn != (pstNewAttr)->bEdgeEn)\
            || ((pstOldAttr)->bEsEn != (pstNewAttr)->bEsEn)\
            || ((pstOldAttr)->bNrEn != (pstNewAttr)->bNrEn)\
            || ((pstOldAttr)->bUvInvert != (pstNewAttr)->bUvInvert)\
            || ((pstOldAttr)->ePixFmt != (pstNewAttr)->ePixFmt)\
        )
#define  MI_VPE_CHNN_STATIC_ATTR_CHANGED(pstOldAttr, pstNewAttr) \
        (((pstOldAttr)->u16MaxW != (pstNewAttr)->u16MaxW)\
            || ((pstOldAttr)->u16MaxH != (pstNewAttr)->u16MaxH)\
        )

#if 0 // no need checck param change
#define MI_VPE_CHNN_PRAMS_CHANGED(pstOld, pstNew)\
        (((pstOld)->u8NrcSfStr != (pstNew)-> u8NrcSfStr) \
        || ((pstOld)->u8NrcTfStr != (pstNew)-> u8NrcTfStr) \
        || ((pstOld)->u8NrySfStr != (pstNew)-> u8NrySfStr) \
        || ((pstOld)->u8NryTfStr != (pstNew)-> u8NryTfStr) \
        || ((pstOld)->u8NryBlendMotionTh != (pstNew)-> u8NryBlendMotionTh) \
        || ((pstOld)->u8NryBlendStillTh != (pstNew)-> u8NryBlendStillTh) \
        || ((pstOld)->u8NryBlendMotionWei != (pstNew)-> u8NryBlendMotionWei) \
        || ((pstOld)->u8NryBlendOtherWei != (pstNew)-> u8NryBlendOtherWei) \
        || ((pstOld)->u8NryBlendStillWei != (pstNew)-> u8NryBlendStillWei) \
        || ((pstOld)->u8EdgeGain[0] != (pstNew)-> u8EdgeGain[0]) \
        || ((pstOld)->u8EdgeGain[1] != (pstNew)-> u8EdgeGain[1]) \
        || ((pstOld)->u8EdgeGain[2] != (pstNew)-> u8EdgeGain[2]) \
        || ((pstOld)->u8EdgeGain[3] != (pstNew)-> u8EdgeGain[3]) \
        || ((pstOld)->u8EdgeGain[4] != (pstNew)-> u8EdgeGain[4]) \
        || ((pstOld)->u8EdgeGain[5] != (pstNew)-> u8EdgeGain[5]) \
        || ((pstOld)->u8EdgeGain[6] != (pstNew)-> u8EdgeGain[6]) \
        || ((pstOld)->u8Contrast != (pstNew)-> u8Contrast)\
        )
#else
#define MI_VPE_CHNN_PRAMS_CHANGED(pstOld, pstNew) (TRUE)
#endif
#define GET_VPE_BUFF_FRAME_FROM_TASK(pstChnTask) (&(pstChnTask)->astInputPortBufInfo[0]->stFrameData)

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------

static mi_vpe_DevInfo_t _gVpeDevInfo = {
    .u32MagicNumber= __MI_VPE_DEV_MAGIC__,
    .bInited = FALSE,
    .hDevSysHandle = MI_HANDLE_NULL,
    .uVpeIrqNum    = 0,
    .u32ChannelCreatedNum = 0,
    .eRoiStatus    = E_MI_VPE_ROI_STATUS_IDLE,
    .st3DNRUpdate  = {
        .eStatus = E_MI_VPE_3DNR_STATUS_IDLE,
        .VpeCh    = 0,
    },
};
// TODO: Tommy macro change to static inline function
#define MI_VPE_CHNN_ATTR_OUT_OF_CAPS(pstDevInfo, pstVpeChAttr) (!(!(((pstVpeChAttr)->u16MaxW > MI_VPE_CHANNEL_MAX_WIDTH) && ((pstVpeChAttr)->u16MaxW * (pstVpeChAttr)->u16MaxH) > (MI_VPE_CHANNEL_MAX_WIDTH *MI_VPE_CHANNEL_MAX_HEIGHT))\
            && (((pstDevInfo)->u64TotalCreatedArea + ((pstVpeChAttr)->u16MaxW * (pstVpeChAttr)->u16MaxH) <= 8*FHD_SIZE))\
            && (((pstDevInfo)->u32ChannelCreatedNum +  1) < MI_VPE_MAX_CHANNEL_NUM)\
            ))

#define MI_VPE_CHECKDEVCHN_RUNMODE(eDevRunMode, eChnlRunMode) do {\
if(eDevRunMode == E_MI_VPE_RUNNING_MODE_INVALID)\
    eDevRunMode = eChnlRunMode;\
else if(eChnlRunMode != eChnlRunMode)\
{\
    DBG_ERR("Channel RunningMode can't be different.\n");\
    return MI_ERR_VPE_NOT_SUPPORT;\
}\
}while(0);

#define MI_VPE_CHECKDEV_SUPPORT_RUNMODE(nbRunMode)\
if(nbRunMode != TRUE)\
{\
    DBG_ERR("Dev not Support Run mode.\n");\
    return MI_ERR_VPE_NOT_SUPPORT;\
}\

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
#define PRINTF_PROC(fmt, args...)  {do{MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_RED"[MI VPE PROCFS]:" fmt ASCII_COLOR_END, ##args);}while(0);}
#endif

DECLARE_WAIT_QUEUE_HEAD(vpe_isr_waitqueue);
LIST_HEAD(VPE_roi_task_list);
DEFINE_SEMAPHORE(VPE_roi_task_list_sem);

LIST_HEAD(VPE_todo_task_list);
LIST_HEAD(VPE_working_task_list);
DEFINE_SEMAPHORE(VPE_working_list_sem);
LIST_HEAD(VPE_active_channel_list);
DEFINE_SEMAPHORE(VPE_active_channel_list_sem);
DEFINE_SEMAPHORE(VPE_Rotate_sem);

static MS_S32 _MI_VPE_IMPL_MiSysAlloc(MS_U8 *pu8Name, MS_U32 u32Size, MS_PHYADDR * phyAddr)
{
    MI_SYS_BUG_ON(NULL == phyAddr);

    return (MS_S32)mi_sys_MMA_Alloc((MI_U8*)pu8Name, (MI_U32)u32Size, (MI_PHY *)phyAddr);
}
static MS_S32 _MI_VPE_IMPL_MiSysFree (MS_PHYADDR phyAddr)
{
    return (MS_S32)mi_sys_MMA_Free((MI_PHY)phyAddr);
}

static MS_S32 _MI_VPE_IMPL_MiSysFlashCache(void *pVirtAddr, MS_U32 u32Size)
{
    MI_SYS_BUG_ON(NULL == pVirtAddr);

    return (MS_S32)mi_sys_VFlushInvCache(pVirtAddr, (MI_U32)u32Size);
}

static void* _MI_VPE_IMPL_MiSysMap(MS_PHYADDR u64PhyAddr, MS_U32 u32Size , MS_BOOL bCache)
{
    return mi_sys_Vmap((MI_PHY)u64PhyAddr, (MI_U32)u32Size , (MI_BOOL)bCache);
}

static void _MI_VPE_IMPL_MiSysUnMap(void *pVirtAddr)
{
    MI_SYS_BUG_ON(NULL == pVirtAddr);
    return mi_sys_UnVmap(pVirtAddr);
}

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
// TODO: Tommy: Think about channel semaphore between  work thread and  user API event.
MI_S32 mi_Vpe_HalGetMode(MHalVpeIrqEnableMode_e *peIrqMode, MHalVpeTopInputMode_e *peRunMode)
{
    mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();
    switch (pstDevInfo->eRunningMode)
    {
        case E_MI_VPE_RUNNING_MODE_REALTIME_MODE:
        {
            if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_3_PORTMODE)
            {
                *peIrqMode = E_MHAL_SCL_IRQ_CAM_REALTIME_3PORT;
            }
            else if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_4_PORTMODE)
            {
                *peIrqMode = E_MHAL_SCL_IRQ_CAM_REALTIME_4PORT;
            }
            *peRunMode = E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE;
            break;
        }
        case E_MI_VPE_RUNNING_MODE_FRAMEBUF_CAM_MODE:
        {
            if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_3_PORTMODE)
            {
                *peIrqMode = E_MHAL_SCL_IRQ_CAM_FRAMEBUF_3PORT;
            }
            else if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_4_PORTMODE)
            {
                *peIrqMode = E_MHAL_SCL_IRQ_CAM_FRAMEBUF_4PORT;
            }
            *peRunMode = E_MHAL_VPE_INPUT_MODE_FRAMEBUF_CAM_MODE;
            break;
        }
        case E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE:
        {
            if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_3_PORTMODE)
            {
                *peIrqMode = E_MHAL_SCL_IRQ_DVR_FRAMEBUF_3PORT;
            }
            else if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_4_PORTMODE)
            {
                *peIrqMode = E_MHAL_SCL_IRQ_DVR_FRAMEBUF_4PORT;
            }
            *peRunMode = E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE;
            break;
        }
        default:
            DBG_ERR("running mode %d err\n", pstDevInfo->eRunningMode);
    }

    return MI_SUCCESS;
}

MI_S32 MI_VPE_IMPL_CreateChannel(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelAttr_t *pstVpeChAttr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    mi_vpe_ChannelInfo_t *pstChnnInfo = NULL;
    DBG_ENTER("chnl%d, Max(%dx%d), RunMode %d, PqEn(%d, %d, %d, %d,%d)\n", VpeCh, pstVpeChAttr->u16MaxW, pstVpeChAttr->u16MaxH,
        pstVpeChAttr->eRunningMode, pstVpeChAttr->bNrEn, pstVpeChAttr->bEdgeEn, pstVpeChAttr->bEsEn, pstVpeChAttr->bContrastEn, pstVpeChAttr->bUvInvert);

    if (MI_VPE_CHECK_CHNN_SUPPORTED(VpeCh))
    {
        mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();
        // Create channel
        MHalAllocPhyMem_t stAlloc;
        // SCL handle
        MHalVpeSclWinSize_t stMaxWin;
        MHalVpeIqOnOff_t stCfg;
        MHalVpeIspInputConfig_t stIspCfg;
        MHalVpeSclRotationConfig_t stRotate;
        //Running mode
        MHalVpeTopInputMode_e eRunMode;
        MhalVpeIspCreate_t stISPCreat;
        MHalVpeIrqEnableMode_e eIrqMode;

        pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        switch (pstVpeChAttr->eRunningMode)
        {
            case E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE:
                {
                    MI_VPE_CHECKDEV_SUPPORT_RUNMODE(pstDevInfo->stCaps.nbFrmBufDvr);
                    MI_VPE_CHECKDEVCHN_RUNMODE(pstDevInfo->eRunningMode, pstVpeChAttr->eRunningMode);
                     pstDevInfo->bCameraUse = FALSE;
                }
                break;
            case E_MI_VPE_RUNNING_MODE_FRAMEBUF_CAM_MODE:
                {
                    MI_VPE_CHECKDEV_SUPPORT_RUNMODE(pstDevInfo->stCaps.nbFrmBufCam);
                    MI_VPE_CHECKDEVCHN_RUNMODE(pstDevInfo->eRunningMode, pstVpeChAttr->eRunningMode);
                    pstDevInfo->bCameraUse = FALSE;
                }
                break;
            case E_MI_VPE_RUNNING_MODE_REALTIME_MODE:
                {
                    MI_VPE_CHECKDEV_SUPPORT_RUNMODE(pstDevInfo->stCaps.nbRealTime);
                    MI_VPE_CHECKDEVCHN_RUNMODE(pstDevInfo->eRunningMode, pstVpeChAttr->eRunningMode);

                    if(pstDevInfo->u32ChannelCreatedNum >= MI_VPE_REALTIMEMODE_MAXCHNL_NUM)
                    {
                        DBG_ERR("Real Time Only Support %d ChannelNum.\n", MI_VPE_REALTIMEMODE_MAXCHNL_NUM);
                        return MI_ERR_VPE_NOT_SUPPORT;
                    }
                     pstDevInfo->bCameraUse = TRUE;
                }
                break;
            default:
                {
                    DBG_ERR("Running mode %d not support.\n",pstVpeChAttr->eRunningMode);
                    return MI_ERR_VPE_NOT_SUPPORT;
                }
        }

        // Check created ?
        if (pstChnnInfo->bCreated == TRUE)
        {
            s32Ret = MI_ERR_VPE_EXIST;
            DBG_ERR("Channel id: %d already created.\n", VpeCh);
            return s32Ret;
        }
        else if (MI_VPE_CHNN_ATTR_OUT_OF_CAPS(pstDevInfo, pstVpeChAttr))
        {
            DBG_WRN("Channel id: %d MaxW: %d MaxH: %d out of hardware Caps !!!\n", VpeCh, pstVpeChAttr->u16MaxW, pstVpeChAttr->u16MaxH);
        }

        if(pstDevInfo->bCameraUse)
        {
            init_waitqueue_head(&pstDevInfo->inputWaitqueue);
        }

        memset(pstChnnInfo, 0, sizeof(*pstChnnInfo));
        atomic_set(&pstChnnInfo->stAtomTask, 0);
        stAlloc.alloc = _MI_VPE_IMPL_MiSysAlloc;
        stAlloc.free  = _MI_VPE_IMPL_MiSysFree;
        stAlloc.map   = _MI_VPE_IMPL_MiSysMap;
        stAlloc.unmap = _MI_VPE_IMPL_MiSysUnMap;
        stAlloc.flush_cache = _MI_VPE_IMPL_MiSysFlashCache;
        stMaxWin.u16Width = pstChnnInfo->stChnnAttr.u16MaxW = pstVpeChAttr->u16MaxW;
        stMaxWin.u16Height= pstChnnInfo->stChnnAttr.u16MaxH = pstVpeChAttr->u16MaxH;
        pstChnnInfo->VpeCh = VpeCh;
        pstChnnInfo->stChnnAttr.eHDRType = pstVpeChAttr->eHDRType;
        pstChnnInfo->stChnnAttr.eRunningMode = pstVpeChAttr->eRunningMode;

        mi_Vpe_HalGetMode(&eIrqMode, &eRunMode);

        if(SUPPORT_SCL & pstDevInfo->eRunningMode)
        {
            MHalVpeSclCreate_t stSclCreat;

            memset(&stSclCreat, 0, sizeof(MHalVpeSclCreate_t));
            stSclCreat.enSclInstance = E_MHAL_VPE_SCL_MAIN;
            stSclCreat.enTopIn = eRunMode;
            if(pstChnnInfo->eRealRotationType == E_MI_SYS_ROTATE_NONE)
                stSclCreat.enInputPortMode = E_MHAL_SCL_MAIN_INPUT_MODE_REAL_TIME;// realtime is isp->scl not vif->vpe
            else
                stSclCreat.enInputPortMode = E_MHAL_SCL_MAIN_FRAME_ROTATION;

            if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_3_PORTMODE)
                stSclCreat.enOutmode = E_MHAL_SCL_OUTPUT_MODE_3PORT_2LEVEL_MODE;
            else if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_4_PORTMODE)
                stSclCreat.enOutmode = E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE;
            else
                DBG_ERR("SCL output port mode %d err\n", pstDevInfo->eSclPortMode);

            memcpy(&stSclCreat.stMaxWin, &stMaxWin, sizeof(MHalVpeSclWinSize_t));

            pstChnnInfo->eSclInPutMode = stSclCreat.enInputPortMode;
            pstChnnInfo->eSclOutputMode = stSclCreat.enOutmode;

            if (FALSE == MHalVpeCreateSclInstance(&stAlloc, &stSclCreat, &pstChnnInfo->pSclCtx))
            {
                DBG_ERR("Channel id: %d MHalVpeCreateSclInstance Fail.\n", VpeCh);
                goto error_create_scl;
            }
            // Init Rotation
            if(SUPPORT_ROATION & pstDevInfo->eRunningMode)
            {
                pstChnnInfo->eRotationType = E_MI_SYS_ROTATE_NONE;
                pstChnnInfo->eRealRotationType = E_MI_SYS_ROTATE_NONE;
                stRotate.enRotType = E_MHAL_SCL_ROTATION_Off;
                ExecFunc(MHalVpeSclRotationConfig(pstChnnInfo->pSclCtx, &stRotate));
            }
        }

        // IQ handle
        if(SUPPORT_IQ & pstDevInfo->eRunningMode)
        {
            if (FALSE == MHalVpeCreateIqInstance(&stAlloc, &stMaxWin, eRunMode, &pstChnnInfo->pIqCtx))
            {
                DBG_ERR("Channel id: %d MHalVpeCreateIqInstance Fail.\n", VpeCh);
                goto error_create_iq;
            }

            memset(&stCfg, 0, sizeof(stCfg));
            pstChnnInfo->stChnnAttr.bNrEn       = pstVpeChAttr->bNrEn;
            pstChnnInfo->stChnnAttr.bEdgeEn     = pstVpeChAttr->bEdgeEn;
            pstChnnInfo->stChnnAttr.bEsEn       = pstVpeChAttr->bEsEn;
            pstChnnInfo->stChnnAttr.bContrastEn = pstVpeChAttr->bContrastEn;
            pstChnnInfo->stChnnAttr.bUvInvert   = pstVpeChAttr->bUvInvert;

            stCfg.bNREn       = (bool)pstChnnInfo->stChnnAttr.bNrEn;
            stCfg.bEdgeEn     = (bool)pstChnnInfo->stChnnAttr.bEdgeEn;
            stCfg.bESEn       = (bool)pstChnnInfo->stChnnAttr.bEsEn;
            stCfg.bContrastEn = (bool)pstChnnInfo->stChnnAttr.bContrastEn;
            stCfg.bUVInvert   = (bool)pstChnnInfo->stChnnAttr.bUvInvert;
            ExecFunc(MHalVpeIqOnOff(pstChnnInfo->pIqCtx, &stCfg));

        }
        // ISP handle(SUPPORT_ISP)
        if(SUPPORT_ISP & pstDevInfo->eRunningMode)
        {
            stISPCreat.eInput = eRunMode;
            stISPCreat.eHDRMode = pstVpeChAttr->eHDRType;
            pstChnnInfo->eHDRType = pstVpeChAttr->eHDRType;
            if ( FALSE == MHalVpeCreateIspInstance(&stAlloc, &stISPCreat, &pstChnnInfo->pIspCtx))
            {
                DBG_ERR("Channel id: %d MHalVpeCreateIspInstance Fail.\n", VpeCh);
                goto error_create_isp;
            }

            // MHalVpeIspInputConfig
            memset(&stIspCfg, 0, sizeof(stIspCfg));
            // Init input compress mode
            stIspCfg.eCompressMode = pstChnnInfo->eCompressMode = E_MHAL_COMPRESS_MODE_NONE;
            stIspCfg.u32Width       = pstVpeChAttr->u16MaxW;
            stIspCfg.u32Height      = pstVpeChAttr->u16MaxH;
            // init input pixel format
            stIspCfg.ePixelFormat   = pstChnnInfo->stChnnAttr.ePixFmt = pstVpeChAttr->ePixFmt;
            ExecFunc(MHalVpeIspInputConfig(pstChnnInfo->pIspCtx, &stIspCfg));
       }

        // Init input width/height
        // Cap Window
        pstChnnInfo->u64PhyAddrOffset[0] = 0;
        pstChnnInfo->u64PhyAddrOffset[1] = 0;
        pstChnnInfo->u64PhyAddrOffset[2] = 0;
        memset(&pstChnnInfo->stSrcWin, 0, sizeof(pstChnnInfo->stSrcWin));
        memset(&pstChnnInfo->stCropWin, 0, sizeof(pstChnnInfo->stCropWin));
        memset(&pstChnnInfo->stRealCrop, 0, sizeof(pstChnnInfo->stRealCrop));

        // Init internal parameter
        pstChnnInfo->eRotationType = E_MI_SYS_ROTATE_NONE;
        memset(&pstChnnInfo->stPeerInputPortInfo, 0, sizeof(pstChnnInfo->stPeerInputPortInfo));
        //memset(&pstChnnInfo->u32LumaData, 0, sizeof(pstChnnInfo->u32LumaData));
        memset(&pstChnnInfo->stOutPortInfo, 0, sizeof(pstChnnInfo->stOutPortInfo));
        pstDevInfo->u32ChannelCreatedNum++;
        pstChnnInfo->bCreated = TRUE;
        pstChnnInfo->VpeCh    = VpeCh;
        pstDevInfo->u64TotalCreatedArea += pstVpeChAttr->u16MaxW * pstVpeChAttr->u16MaxH;
        INIT_LIST_HEAD(&pstChnnInfo->list);

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
        init_MUTEX(&pstChnnInfo->stChnnMutex);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
        sema_init(&pstChnnInfo->stChnnMutex, 1);
#endif

        if(pstDevInfo->u32ChannelCreatedNum == 1
            && TRUE == pstDevInfo->bSupportIrq)
        {
            // Enable VPE IRQ
            ExecFunc(MHalVpeSclEnableIrq(eIrqMode));
            pstDevInfo->bEnbaleIrq = TRUE;
            pstDevInfo->eIrqEnableMode = eIrqMode;
        }
         s32Ret = MI_SUCCESS;
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;

error_create_isp:
    ExecFunc(MHalVpeDestroyIqInstance(pstChnnInfo->pIqCtx));
error_create_iq:
    ExecFunc(MHalVpeDestroySclInstance(pstChnnInfo->pSclCtx));
error_create_scl:
    s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
    DBG_ERR("Channel id: %d illegal parameter.\n", VpeCh);
    return s32Ret;
}

MI_S32 MI_VPE_IMPL_DestroyChannel(MI_VPE_CHANNEL VpeCh)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MI_U8 u8PortId = 0;
    MI_U8 u8AtomCnt =0;
    DBG_ENTER("chnl%d", VpeCh);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();

        for(u8PortId =0; u8PortId <MI_VPE_MAX_PORT_NUM; u8PortId ++)
        {
            mi_vpe_OutPortInfo_t *pstOutPutPortPtr = GET_VPE_PORT_PTR(VpeCh, u8PortId);
            if(pstOutPutPortPtr->bEnable == TRUE)
                DBG_ERR("ChnId %d, PortId %d, not disable \n", VpeCh, u8PortId);
        }

        // TODO: tommy: 1. check whether disable channel.
        // API set destroy, work thread check status and drop task.
        // Depend on channel
        if(SUPPORT_SCL & pstDevInfo->eRunningMode)
        {
             ExecFunc(MHalVpeDestroySclInstance(pstChnnInfo->pSclCtx));
        }

        // Wait Running Task Finish
        while (atomic_read(&pstChnnInfo->stAtomTask) != 0)
        {
            if(u8AtomCnt ++ < 50)
            {
                DBG_WRN("ATOMtask %d\n", atomic_read(&pstChnnInfo->stAtomTask));
            }
            else
            {
                DBG_ERR("try 50, mhal not return buffer\n");
                break;
            }
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            interruptible_sleep_on_timeout(&vpe_isr_waitqueue, msecs_to_jiffies(1));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            wait_event_interruptible_timeout(vpe_isr_waitqueue, FALSE, msecs_to_jiffies(1));
#endif
        }

        //memset(pstChnnInfo, 0, sizeof(*pstChnnInfo));
        if(SUPPORT_ISP & pstDevInfo->eRunningMode)
        {
             ExecFunc(MHalVpeDestroyIspInstance(pstChnnInfo->pIspCtx));
        }

        if(SUPPORT_IQ & pstDevInfo->eRunningMode)
        {
             ExecFunc(MHalVpeDestroyIqInstance(pstChnnInfo->pIqCtx));
        }

        pstDevInfo->u32ChannelCreatedNum--;
        pstDevInfo->u64TotalCreatedArea -= pstChnnInfo->stChnnAttr.u16MaxW * pstChnnInfo->stChnnAttr.u16MaxH;
        pstChnnInfo->bCreated = FALSE;
        pstChnnInfo->eStatus  = E_MI_VPE_CHANNEL_STATUS_DESTROYED;

        if(pstDevInfo->u32ChannelCreatedNum == 0
            && TRUE == pstDevInfo->bSupportIrq)
        {
            MHalVpeIrqEnableMode_e eIrqMode = E_MHAL_SCL_IRQ_OFF;
            if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_3_PORTMODE)
                eIrqMode = E_MHAL_SCL_IRQ_OFFM2M;
            else if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_4_PORTMODE)
                eIrqMode = E_MHAL_SCL_IRQ_OFF;

            // Enable VPE IRQ
             ExecFunc(MHalVpeSclEnableIrq(eIrqMode));
             pstDevInfo->bEnbaleIrq = FALSE;
        }

        s32Ret = MI_VPE_OK;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_GetChannelAttr(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelAttr_t *pstVpeChAttr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl%d\n", VpeCh);
    // No need add semaphore.
    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        memcpy(pstVpeChAttr, &_gVpeDevInfo.stChnnInfo[VpeCh].stChnnAttr, sizeof(*pstVpeChAttr));
        s32Ret = MI_VPE_OK;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

#define LOCK_CHNN(pstChnnInfo)   down(&pstChnnInfo->stChnnMutex)
#define UNLOCK_CHNN(pstChnnInfo) up(&pstChnnInfo->stChnnMutex)

MI_S32 MI_VPE_IMPL_SetChannelAttr(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelAttr_t *pstVpeChAttr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl%d, Max(%dx%d), RunMode %d, PqEn(%d, %d, %d, %d,%d)\n", VpeCh, pstVpeChAttr->u16MaxW, pstVpeChAttr->u16MaxH,
           pstVpeChAttr->eRunningMode, pstVpeChAttr->bNrEn, pstVpeChAttr->bEdgeEn, pstVpeChAttr->bEsEn, pstVpeChAttr->bContrastEn, pstVpeChAttr->bUvInvert);
    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        LOCK_CHNN(pstChnnInfo);
        if (MI_VPE_CHNN_STATIC_ATTR_CHANGED(&pstChnnInfo->stChnnAttr, pstVpeChAttr))
        {
            s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
            DBG_ERR("Channel id: %d illegal params. Try to change static params(MaxW/MaxH).\n", VpeCh);
        }
        else // if (MI_VPE_CHNN_DYNAMIC_ATTR_CHANGED(&pstChnnInfo->stChnnAttr, pstVpeChAttr))
        {
            mi_vpe_DevInfo_t     *pstDevInfo = GET_VPE_DEV_PTR();

            if(SUPPORT_IQ & pstDevInfo->eRunningMode)
            {
                MHalVpeIqOnOff_t stCfg;
                memset(&stCfg, 0, sizeof(stCfg));
                stCfg.bContrastEn = pstVpeChAttr->bContrastEn;
                stCfg.bEdgeEn     = pstVpeChAttr->bEdgeEn;
                stCfg.bESEn       = pstVpeChAttr->bEsEn;
                stCfg.bNREn       = pstVpeChAttr->bNrEn;
                stCfg.bUVInvert   = pstVpeChAttr->bUvInvert;

                 ExecFunc(MHalVpeIqOnOff(pstChnnInfo->pIqCtx, &stCfg));
            }

            if(SUPPORT_ISP & pstDevInfo->eRunningMode)
            {
                MHalVpeIspInputConfig_t stIspCfg;
                memset(&stIspCfg, 0, sizeof(stIspCfg));
                stIspCfg.eCompressMode = pstChnnInfo->eCompressMode;
                stIspCfg.u32Width      = pstChnnInfo->stSrcWin.u16Width;
                stIspCfg.u32Height     = pstChnnInfo->stSrcWin.u16Height;
                stIspCfg.ePixelFormat  = pstChnnInfo->stChnnAttr.ePixFmt = pstVpeChAttr->ePixFmt;
                ExecFunc(MHalVpeIspInputConfig(pstChnnInfo->pIspCtx, &stIspCfg));
            }

            pstChnnInfo->stChnnAttr.bContrastEn = pstVpeChAttr->bContrastEn;
            pstChnnInfo->stChnnAttr.bEdgeEn     = pstVpeChAttr->bEdgeEn;
            pstChnnInfo->stChnnAttr.bEsEn       = pstVpeChAttr->bEsEn;
            pstChnnInfo->stChnnAttr.bNrEn       = pstVpeChAttr->bNrEn;
            pstChnnInfo->stChnnAttr.bUvInvert   = pstVpeChAttr->bUvInvert;

            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        UNLOCK_CHNN(pstChnnInfo);
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}


MI_S32 MI_VPE_IMPL_StartChannel(MI_VPE_CHANNEL VpeCh)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl%d\n", VpeCh);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        mi_vpe_DevInfo_t     *pstDevInfo = GET_VPE_DEV_PTR();
        LOCK_CHNN(pstChnnInfo);
       if(pstChnnInfo->eStatus == E_MI_VPE_CHANNEL_STATUS_START)
        {
           MI_SYS_BUG_ON(list_empty(&pstChnnInfo->list));
           UNLOCK_CHNN(pstChnnInfo);
           return s32Ret;//
        }
        pstChnnInfo->eStatus = E_MI_VPE_CHANNEL_STATUS_START;
        // Add channel to active list
        down(&VPE_active_channel_list_sem);
        list_add_tail(&pstChnnInfo->list, &VPE_active_channel_list);
        up(&VPE_active_channel_list_sem);
        UNLOCK_CHNN(pstChnnInfo);

        mi_sys_EnableInputPort(pstDevInfo->hDevSysHandle, VpeCh, 0);
        mi_sys_EnableChannel(pstDevInfo->hDevSysHandle, VpeCh);

        s32Ret = MI_VPE_OK;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_StopChannel(MI_VPE_CHANNEL VpeCh)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl%d\n", VpeCh);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        mi_vpe_DevInfo_t     *pstDevInfo = GET_VPE_DEV_PTR();

        LOCK_CHNN(pstChnnInfo);
        if(pstChnnInfo->eStatus != E_MI_VPE_CHANNEL_STATUS_START)
        {
           MI_SYS_BUG_ON(!list_empty(&pstChnnInfo->list));
           UNLOCK_CHNN(pstChnnInfo);
           return s32Ret;//
        }

        pstChnnInfo->eStatus = E_MI_VPE_CHANNEL_STATUS_STOP;

        // Remove channel from active list
        down(&VPE_active_channel_list_sem);
        list_del(&pstChnnInfo->list);
        INIT_LIST_HEAD(&pstChnnInfo->list);
        up(&VPE_active_channel_list_sem);
        UNLOCK_CHNN(pstChnnInfo);

        mi_sys_DisableChannel(pstDevInfo->hDevSysHandle, VpeCh);
        mi_sys_DisableInputPort(pstDevInfo->hDevSysHandle, VpeCh, 0);
        s32Ret = MI_VPE_OK;

        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_SetChannelParam(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelPara_t *pstVpeParam)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl%d, param(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, edge(%d,%d,%d,%d,%d,%d))\n",VpeCh,
        pstVpeParam->u8NrcSfStr, pstVpeParam->u8NrcTfStr, pstVpeParam->u8NrySfStr,pstVpeParam->u8NryTfStr, pstVpeParam->u8NryBlendMotionTh, pstVpeParam->u8NryBlendStillTh,
        pstVpeParam->u8NryBlendMotionWei, pstVpeParam->u8NryBlendOtherWei, pstVpeParam->u8NryBlendStillWei, pstVpeParam->u8Contrast,
        pstVpeParam->u8EdgeGain[0], pstVpeParam->u8EdgeGain[1],pstVpeParam->u8EdgeGain[2], pstVpeParam->u8EdgeGain[3],pstVpeParam->u8EdgeGain[4], pstVpeParam->u8EdgeGain[5]);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        LOCK_CHNN(pstChnnInfo);
        if (MI_VPE_CHNN_PRAMS_CHANGED(&pstChnnInfo->stChnnPara, pstVpeParam))
        {
            MHalVpeIqConfig_t stCfg;
            stCfg.u8Contrast = pstVpeParam->u8Contrast;
            stCfg.u8EdgeGain[0] = pstVpeParam->u8EdgeGain[0];
            stCfg.u8EdgeGain[1] = pstVpeParam->u8EdgeGain[1];
            stCfg.u8EdgeGain[2] = pstVpeParam->u8EdgeGain[2];
            stCfg.u8EdgeGain[3] = pstVpeParam->u8EdgeGain[3];
            stCfg.u8EdgeGain[4] = pstVpeParam->u8EdgeGain[4];
            stCfg.u8EdgeGain[5] = pstVpeParam->u8EdgeGain[5];
            stCfg.u8NRC_SF_STR  = pstVpeParam->u8NrcSfStr;
            stCfg.u8NRC_TF_STR  = pstVpeParam->u8NrcTfStr;
            stCfg.u8NRY_BLEND_MOTION_TH = pstVpeParam->u8NryBlendMotionTh;
            stCfg.u8NRY_BLEND_MOTION_WEI= pstVpeParam->u8NryBlendMotionWei;
            stCfg.u8NRY_BLEND_OTHER_WEI = pstVpeParam->u8NryBlendOtherWei;
            stCfg.u8NRY_BLEND_STILL_TH  = pstVpeParam->u8NryBlendStillTh;
            stCfg.u8NRY_BLEND_STILL_WEI = pstVpeParam->u8NryBlendStillWei;
            stCfg.u8NRY_SF_STR          = pstVpeParam->u8NrySfStr;
            stCfg.u8NRY_TF_STR          = pstVpeParam->u8NryTfStr;
            if (TRUE == MHalVpeIqConfig(pstChnnInfo->pIqCtx, &stCfg))
            {
                pstChnnInfo->stChnnPara.u8Contrast = pstVpeParam->u8Contrast;
                pstChnnInfo->stChnnPara.u8EdgeGain[0] = pstVpeParam->u8EdgeGain[0];
                pstChnnInfo->stChnnPara.u8EdgeGain[1] = pstVpeParam->u8EdgeGain[1];
                pstChnnInfo->stChnnPara.u8EdgeGain[2] = pstVpeParam->u8EdgeGain[2];
                pstChnnInfo->stChnnPara.u8EdgeGain[3] = pstVpeParam->u8EdgeGain[3];
                pstChnnInfo->stChnnPara.u8EdgeGain[4] = pstVpeParam->u8EdgeGain[4];
                pstChnnInfo->stChnnPara.u8EdgeGain[5] = pstVpeParam->u8EdgeGain[5];
                pstChnnInfo->stChnnPara.u8NrcSfStr = pstVpeParam->u8NrcSfStr;
                pstChnnInfo->stChnnPara.u8NrcTfStr = pstVpeParam->u8NrcTfStr;
                pstChnnInfo->stChnnPara.u8NryBlendMotionTh = pstVpeParam->u8NryBlendMotionTh;
                pstChnnInfo->stChnnPara.u8NryBlendMotionWei = pstVpeParam->u8NryBlendMotionWei;
                pstChnnInfo->stChnnPara.u8NryBlendOtherWei = pstVpeParam->u8NryBlendOtherWei;
                pstChnnInfo->stChnnPara.u8NryBlendStillTh = pstVpeParam->u8NryBlendStillTh;
                pstChnnInfo->stChnnPara.u8NryBlendStillWei = pstVpeParam->u8NryBlendStillWei;
                pstChnnInfo->stChnnPara.u8NrySfStr = pstVpeParam->u8NrySfStr;
                pstChnnInfo->stChnnPara.u8NryTfStr = pstVpeParam->u8NryTfStr;
                s32Ret = MI_VPE_OK;
                DBG_EXIT_OK();
            }
            else
            {
                s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
                DBG_ERR("Channel id: %d illegal params.\n", VpeCh);
            }

        }
        else
        {
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        UNLOCK_CHNN(pstChnnInfo);
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_GetChannelParam(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelPara_t *pstVpeParam)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl%d\n", VpeCh);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        memcpy(pstVpeParam, &pstChnnInfo->stChnnPara, sizeof(*pstVpeParam));
        s32Ret = MI_VPE_OK;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_SetChannelCrop(MI_VPE_CHANNEL VpeCh,  MI_SYS_WindowRect_t *pstCropInfo)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl%d, crop(%d, %d, %d, %d)\n", VpeCh, pstCropInfo->u16X, pstCropInfo->u16Y, pstCropInfo->u16Width, pstCropInfo->u16Height);
// TODO: tommy: Add channel semaphore
    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);

        LOCK_CHNN(pstChnnInfo);
        if ((pstCropInfo->u16X != pstChnnInfo->stCropWin.u16X)
            || (pstCropInfo->u16Y != pstChnnInfo->stCropWin.u16Y)
            || (pstCropInfo->u16Width !=  pstChnnInfo->stCropWin.u16Width)
            || (pstCropInfo->u16Height!=  pstChnnInfo->stCropWin.u16Height)
            )
        {

            pstChnnInfo->stCropWin.u16X = pstCropInfo->u16X;
            pstChnnInfo->stCropWin.u16Y = pstCropInfo->u16Y;
            pstChnnInfo->stCropWin.u16Width  = pstCropInfo->u16Width;
            pstChnnInfo->stCropWin.u16Height = pstCropInfo->u16Height;

            s32Ret = MI_VPE_OK;
        }
        else
        {
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }

        UNLOCK_CHNN(pstChnnInfo);
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_GetChannelCrop(MI_VPE_CHANNEL VpeCh,  MI_SYS_WindowRect_t *pstCropInfo)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl%d\n", VpeCh);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        memcpy(pstCropInfo, &pstChnnInfo->stCropWin, sizeof(*pstCropInfo));

        s32Ret = MI_VPE_OK;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}
#if 0
#define showReg(p, n) do {MI_SYS_WindowRect_t *pstReg = p+n;\
    DBG_ERR("Region %dx%d@(%d, %d).\n", (pstReg)->u16Width, (pstReg)->u16Height, (pstReg)->u16X, (pstReg)->u16Y);\
} while(0)
#else
#define showReg(p, n)
#endif
MI_S32 MI_VPE_IMPL_GetChannelRegionLuma(MI_VPE_CHANNEL VpeCh, MI_VPE_RegionInfo_t *pstRegionInfo, MI_U32 *pu32LumaData,MI_S32 s32MilliSec)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;

    ROI_Task_t stRoiTask;
    DBG_ENTER("chnl%d, Num %d\n", VpeCh, pstRegionInfo->u32RegionNum);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();
        MI_U32 u32RegionNum = (pstRegionInfo->u32RegionNum > ROI_WINDOW_MAX) ? ROI_WINDOW_MAX : pstRegionInfo->u32RegionNum;
        if((SUPPORT_DNR_ROI & pstDevInfo->eRunningMode) == 0)
        {
            DBG_ERR("Real Time Mode Not Support ROI\n");
            s32Ret = MI_ERR_VPE_NOT_SUPPORT;
            return s32Ret;
        }

        memset(&stRoiTask, 0, sizeof(stRoiTask));
        stRoiTask.u32MagicNumber = __MI_VPE_ROI_MAGIC__;
        stRoiTask.pstRegion   = pstRegionInfo;
        stRoiTask.eRoiStatus  = E_MI_VPE_ROI_STATUS_NEED_UPDATE;
        stRoiTask.pstChnnInfo = pstChnnInfo;
        init_waitqueue_head(&stRoiTask.queue);
        down(&VPE_roi_task_list_sem);
        list_add_tail(&stRoiTask.list, &VPE_roi_task_list);
        up(&VPE_roi_task_list_sem);
        DBG_INFO("%s()@line %d: pstRoiTask: %p pstRoiTask->pstRegion: %p s32MilliSec: %d\n", __func__, __LINE__, &stRoiTask, stRoiTask.pstRegion, s32MilliSec);
        showReg(pstRegionInfo->pstWinRect, 0);
        showReg(pstRegionInfo->pstWinRect, 1);
        showReg(pstRegionInfo->pstWinRect, 2);
        showReg(pstRegionInfo->pstWinRect, 3);

waiting_roi_running:
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
        interruptible_sleep_on_timeout(&stRoiTask.queue, msecs_to_jiffies(s32MilliSec));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
        wait_event_interruptible_timeout(stRoiTask.queue, (stRoiTask.eRoiStatus == E_MI_VPE_ROI_STATUS_UPDATED), msecs_to_jiffies(s32MilliSec));
#endif
        if (stRoiTask.eRoiStatus == E_MI_VPE_ROI_STATUS_RUNNING)
        {
            s32MilliSec = 1;
            goto waiting_roi_running;
        }
        else
        {
            down(&VPE_roi_task_list_sem);
            if (stRoiTask.eRoiStatus == E_MI_VPE_ROI_STATUS_RUNNING)
            {
                s32MilliSec = 1;
                up(&VPE_roi_task_list_sem);
                goto waiting_roi_running;
            }
            list_del(&stRoiTask.list);
            up(&VPE_roi_task_list_sem);

            if (stRoiTask.eRoiStatus == E_MI_VPE_ROI_STATUS_UPDATED)
            {
                // Copy ROI to user
                memcpy(pu32LumaData, &stRoiTask.u32LumaData, u32RegionNum*sizeof(MI_U32));
                s32Ret = MI_VPE_OK;
                DBG_EXIT_OK();
            }
            else
            {
                s32Ret = MI_ERR_VPE_BUSY;
                DBG_ERR("Channel %s is busy.\n", VpeCh);
            }
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

// TODO: tommy: Add in API description :  I2 just support  only one channel.
MI_S32 MI_VPE_IMPL_SetChannelRotation(MI_VPE_CHANNEL VpeCh,  MI_SYS_Rotate_e eType)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    MHalVpeSclRotationConfig_t stRotation;
    DBG_ENTER("chnl%d, RotType %d\n", VpeCh, eType);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        mi_vpe_DevInfo_t *pstVpeDevInfo = GET_VPE_DEV_PTR();
        LOCK_CHNN(pstChnnInfo);

        if((SUPPORT_ROATION & pstVpeDevInfo->eRunningMode) == 0)
        {
            s32Ret = MI_ERR_VPE_NOT_SUPPORT;
            DBG_ERR("Channel %d is Real Time Mode not support Roation: %d.\n", VpeCh, eType);
        }

        stRotation.enRotType = (MHalVpeSclRotationType_e)eType;
        if (TRUE == MHalVpeSclRotationConfig(pstChnnInfo->pSclCtx, &stRotation))
        {
            s32Ret = MI_VPE_OK;
            pstChnnInfo->eRotationType = eType;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_VPE_NOT_SUPPORT;
            DBG_ERR("Channel %d not support Roation: %d.\n", VpeCh, eType);
        }

        UNLOCK_CHNN(pstChnnInfo);
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_GetChannelRotation(MI_VPE_CHANNEL VpeCh,  MI_SYS_Rotate_e *pType)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl %d\n", VpeCh);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        *pType = pstChnnInfo->eRotationType;

        s32Ret = MI_VPE_OK;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_EnablePort(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl %d, port %d\n", VpeCh, VpePort);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        LOCK_CHNN(pstChnnInfo);
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);
            mi_vpe_DevInfo_t     *pstDevInfo     = GET_VPE_DEV_PTR();
            pstOutPortInfo->bEnable = TRUE;
            mi_sys_EnableOutputPort(pstDevInfo->hDevSysHandle, VpeCh, VpePort);
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_ERR("Invalid port id: %d.\n", VpePort);
        }
        UNLOCK_CHNN(pstChnnInfo);
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_DisablePort(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl %d, port %d\n", VpeCh, VpePort);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        LOCK_CHNN(pstChnnInfo);
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);
            mi_vpe_DevInfo_t     *pstDevInfo     = GET_VPE_DEV_PTR();
            pstOutPortInfo->bEnable = FALSE;
            mi_sys_DisableOutputPort(pstDevInfo->hDevSysHandle, VpeCh, VpePort);
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_ERR("Invalid port id: %d.\n", VpePort);
        }
        UNLOCK_CHNN(pstChnnInfo);
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_SetPortMode(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort, MI_VPE_PortMode_t *pstVpeMode)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    mi_vpe_DevInfo_t  *pstDevInfo    = GET_VPE_DEV_PTR();
    DBG_ENTER("chnl %d, port %d, size(%dx%d), pixel %d, compress %d\n", VpeCh, VpePort,
        pstVpeMode->u16Width, pstVpeMode->u16Height, pstVpeMode->ePixelFormat, pstVpeMode->eCompressMode);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_ChannelInfo_t  *pstChnnInfo    = GET_VPE_CHNN_PTR(VpeCh);
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);

            if((pstDevInfo->eSclPortMode == E_MI_VPE_SCL_3_PORTMODE)
                && (VpePort == E_MHAL_SCL_OUTPUT_PORT2))

            {
                DBG_ERR("3P0RT MODE for usb cam cant use port2\n");
                s32Ret =MI_ERR_VPE_ILLEGAL_PARAM;
                return s32Ret;
            }

            LOCK_CHNN(pstChnnInfo);
            s32Ret = MI_VPE_OK;

            if(SUPPORT_SCL & pstDevInfo->eRunningMode)
            {
                MHalVpeSclOutputSizeConfig_t stOutput;
                MHalVpeSclDmaConfig_t stDmaCfg;

                memset(&stOutput, 0, sizeof(stOutput));
                memset(&stDmaCfg, 0, sizeof(stDmaCfg));
                // Need check as foo ?
                // memset(&stOutput, 0, sizeof(stOutput));
                stOutput.enOutPort = (MHalVpeDmaPort_e)VpePort;
                stOutput.u16Width  = pstVpeMode->u16Width;
                stOutput.u16Height = pstVpeMode->u16Height;
                if (TRUE == MHalVpeSclPortSizeConfig(pstChnnInfo->pSclCtx, &stOutput))
                {
                    pstOutPortInfo->stPortMode.u16Width = pstVpeMode->u16Width;
                    pstOutPortInfo->stPortMode.u16Height= pstVpeMode->u16Height;

                    DBG_EXIT_OK();
                }
                else
                {
                    s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
                    DBG_ERR("Ch: %d port %d set output size fail.\n", VpeCh, VpePort);
                }

            // Tommy: Check Caps: Pixel format per frame change in ISP ????
                stDmaCfg.enPort = (MHalVpeDmaPort_e)VpePort;
                stDmaCfg.enFormat = pstVpeMode->ePixelFormat;
                stDmaCfg.enCompress  = pstVpeMode->eCompressMode;

                if (TRUE == MHalVpeSclDmaPortConfig(pstChnnInfo->pSclCtx, &stDmaCfg))
                {
                    pstOutPortInfo->stPortMode.ePixelFormat = pstVpeMode->ePixelFormat;
                    pstOutPortInfo->stPortMode.eCompressMode= pstVpeMode->eCompressMode;
                    //pstOutPortInfo->eRealOutputPixelFormat  = pstVpeMode->ePixelFormat;
                    DBG_EXIT_OK();
                }
                else
                {
                    s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
                    DBG_ERR("Ch: %d port %d set output size fail.\n", VpeCh, VpePort);
                }
                pstOutPortInfo->stPortMode = *pstVpeMode;
            }

            UNLOCK_CHNN(pstChnnInfo);
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_ERR("Invalid port id: %d.\n", VpePort);
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_IMPL_GetPortMode(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort, MI_VPE_PortMode_t *pstVpeMode)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER("chnl%d, port%d\n", VpeCh, VpePort);

    if (MI_VPE_CHECK_CHNN_CREATED(VpeCh))
    {
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);
            *pstVpeMode = pstOutPortInfo->stPortMode;
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_ERR("Invalid port id: %d.\n", VpePort);
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

static MI_BOOL _MI_VPE_IsFenceL(MI_U16 u16Fence1, MI_U16 u16Fence2)
{
   if(u16Fence1<u16Fence2)
  {
   if((u16Fence1 + (0xffff-u16Fence2)) < 0x7FFF)
   {
       return FALSE;
   }
  }
  else
  {
       return FALSE;
  }

  return TRUE;
}

static MI_U16 _MI_VPE_ReadFence(MHAL_CMDQ_CmdqInterface_t *cmdinf)
{
    MI_U16 u16Value = 0;
    cmdinf->MHAL_CMDQ_ReadDummyRegCmdq(cmdinf, &u16Value);
    return u16Value;
}

static void _mi_vpe_DebugWriteFile(MI_U8 * pu8Name, MI_U8 *pu8Buf, MI_U32 u32Len, MI_U32 u32Offset)
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
    fp->f_op->llseek(fp, u32Offset, SEEK_SET);

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

static void _MI_VPE_WriteTaskFile(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_U8 u8PortId =0;
    static MI_U8 au8FileName[128];
    void *pY = NULL, *pUV = NULL;
    mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();

    for(u8PortId=0; u8PortId <MI_VPE_MAX_PORT_NUM; u8PortId++)
    {
        if(pstChnTask->astOutputPortBufInfo[u8PortId] != NULL)
        {
             MI_U16 u16Width = pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.u16Width;
             MI_U16 u16Height = pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.u16Height;

            if(pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
            {
                MI_U32 u32Size = u16Width*u16Height*2;
                pY = mi_sys_Vmap(pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.phyAddr[0], u32Size, FALSE);
                if(pY == NULL)
                    DBG_ERR("PY MMAP fail\n");

                sprintf(au8FileName, "/mnt/vpe_port%d_%dx%d_yuv422_%llu.yuv",u8PortId, u16Width, u16Height, pstDevInfo->u64FramePts);
                _mi_vpe_DebugWriteFile(au8FileName, pY, u32Size, 0);
                mi_sys_UnVmap(pY);
            }

            if(pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
            {
                MI_U32 u32YSize = u16Width*u16Height;
                MI_U32 u32UVSize = u16Width*u16Height/2;

                 pY = mi_sys_Vmap(pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.phyAddr[0], u32YSize, FALSE);
                 pUV = mi_sys_Vmap(pstChnTask->astOutputPortBufInfo[u8PortId]->stFrameData.phyAddr[1], u32UVSize, FALSE);
                if(pY && pUV == NULL)
                    DBG_ERR("MMAP fail PY %p, PUV %p\n", pY, pUV);

                sprintf(au8FileName, "/mnt/vpe_port%d_%dx%d_yuv420_%llu.yuv",u8PortId, u16Width, u16Height, pstDevInfo->u64FramePts);
                _mi_vpe_DebugWriteFile(au8FileName, pY, u32YSize, 0);
                _mi_vpe_DebugWriteFile(au8FileName, pUV, u32UVSize, u32YSize);
                 mi_sys_UnVmap(pY);
                 mi_sys_UnVmap(pUV);
            }
        }
    }
}

static void _MI_VPE_DebugGetTaskTime(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_U64 u64Intervaltime;
    mi_vpe_ChannelInfo_t *pstChnnInfo;
    pstChnnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);

    u64Intervaltime = VPE_RELEASE_TASK_TIME(pstChnTask) - VPE_GOT_TASK_TIME(pstChnTask);
    if(u64Intervaltime > pstChnnInfo->u64GetToReleaseMaxTime)
        pstChnnInfo->u64GetToReleaseMaxTime = u64Intervaltime;

    pstChnnInfo->u64GetToReleaseSumTime += u64Intervaltime;
    pstChnnInfo->u64ReleaseBufcnt ++;

    pstChnnInfo->u64KickOffToReleaseSumTime += VPE_RELEASE_TASK_TIME(pstChnTask) - VPE_KICKOFF_TASK_TIME(pstChnTask);
    pstChnnInfo->u32KOToReleaseMeanTime = div64_u64(pstChnnInfo->u64KickOffToReleaseSumTime, pstChnnInfo->u64ReleaseBufcnt);
}


static void _MI_VPE_DebugGetBufCnt(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_U8 u8PortId = 0;

    for(u8PortId = 0; u8PortId <MI_VPE_MAX_PORT_NUM; u8PortId++)
    {
        if(pstChnTask->astOutputPortBufInfo[u8PortId])
        {
            mi_vpe_OutPortInfo_t *pstPortInfo = NULL;
            pstPortInfo = GET_VPE_PORT_PTR(pstChnTask->u32ChnId, u8PortId);
            pstPortInfo->u64FinishOutputBufferCnt++;
            pstPortInfo->u32FrameCntpers ++;
            if(jiffies - pstPortInfo->u32FrameCntJiffies > HZ)
            {
                pstPortInfo->u8PortFps = pstPortInfo->u32FrameCntpers;
                pstPortInfo->u32FrameCntpers = 0;
                pstPortInfo->u32FrameCntJiffies = jiffies;
            }
        }
    }
}

static int _MI_VPE_IsrProcThread(void *data)
{
    mi_vpe_DevInfo_t *pstDevInfo  = (mi_vpe_DevInfo_t *)data;
    MHAL_CMDQ_CmdqInterface_t *cmdinf = pstDevInfo->pstCmdMloadInfo;
    MI_S32 s32Ret = 0;
    struct list_head* pos, *n;
    MI_BOOL bTaskHasNoOutToDram = FALSE;
    mi_sys_ChnTaskInfo_t *pstLastPreprocessNotifyTask = NULL;
    MI_U16 cmdfence = 0;
    while (!kthread_should_stop())
    {
        //DBG_INFO("Proc get data.\n");
        if (pstDevInfo->u32TaskNoToDramCnt != 0)
        {
            bTaskHasNoOutToDram = TRUE;
        }

        if (bTaskHasNoOutToDram == TRUE)
        {
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            interruptible_sleep_on_timeout(&vpe_isr_waitqueue, msecs_to_jiffies(1));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            wait_event_interruptible_timeout(vpe_isr_waitqueue, FALSE, msecs_to_jiffies(1));
#endif
        }
        else
        {
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
            interruptible_sleep_on_timeout(&vpe_isr_waitqueue, msecs_to_jiffies(VPE_PROC_WAIT));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
            wait_event_interruptible_timeout(vpe_isr_waitqueue, FALSE, msecs_to_jiffies(VPE_PROC_WAIT));
#endif
        }

        down(&VPE_working_list_sem);
        if (list_empty(&VPE_working_task_list))
        {
            up(&VPE_working_list_sem);
            continue;
        }

        list_for_each_safe(pos, n, &VPE_working_task_list)
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            MI_BOOL bDroped = FALSE;
            mi_vpe_ChannelInfo_t *pstChnnInfo;

            pstChnTask = container_of(VPE_working_task_list.next, mi_sys_ChnTaskInfo_t, listChnTask);
            pstChnnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);

            if (pstChnTask == NULL)
            {
                DBG_ERR("pstChnTask %p\n", pstChnTask);
                break;
            }
            DBG_INFO(" got data.\n");


            // Task has not been finished yet.
            if(pstDevInfo->eRunningMode == E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
            {
                cmdfence = pstDevInfo->u16RTCmdqFence;
            }
            else
            {
                cmdfence = _MI_VPE_ReadFence(cmdinf);
            }

            if(_MI_VPE_IsFenceL(cmdfence, pstChnTask->u32Reserved0) == TRUE)
            {
                MS_BOOL bIdleVal = FALSE;
                cmdinf->MHAL_CMDQ_IsCmdqEmptyIdle(cmdinf, &bIdleVal);
                if(bIdleVal == FALSE) //cmdQ is Running
                {
                    //DBG_INFO("invalid fence %04x, %04x!\n", _MI_VPE_ReadFence(cmdinf), pstChnTask->u32Reserved0);

                    break;
                }
                else // cmdQ is stop
                {
                    if(_MI_VPE_IsFenceL(_MI_VPE_ReadFence(cmdinf), pstChnTask->u32Reserved0) == TRUE)
                    {
                        DBG_ERR("invalid fence %04x, %04x!\n", _MI_VPE_ReadFence(cmdinf), pstChnTask->u32Reserved0);
                        MI_SYS_BUG();
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if(pstDevInfo->bCameraUse)
            {
                WAKE_UP_QUEUE_IF_NECESSARY(pstDevInfo->inputWaitqueue);
            }

            DBG_INFO("Finish data.\n");
            // Task already finished.
            // Remove task from working list
            list_del(&pstChnTask->listChnTask);
            pstLastPreprocessNotifyTask = NULL;
            // Release menuload ring buffer
            cmdinf->MHAL_CMDQ_UpdateMloadRingBufReadPtr(cmdinf, pstChnTask->u64Reserved0);

            // Check Task is 3DNR upate
            if(pstDevInfo->eRunningMode & SUPPORT_DNR_ROI)
            {
                if (pstChnTask->u32Reserved1 & MI_VPE_TASK_3DNR_UPDATE)
                {
                    mi_vpe_impl_3DnrUpdateProcessFinish(pstDevInfo, &pstDevInfo->stChnnInfo[pstChnTask->u32ChnId]);
                }

                // Check Task is ROI update
                if (pstChnTask->u32Reserved1 & MI_VPE_TASK_ROI_UPDATE)
                {
                    mi_vpe_impl_RoiProcessFinish(pstDevInfo, &pstDevInfo->stChnnInfo[pstChnTask->u32ChnId], pstChnTask);
                }
                else
                {
                    //DBG_INFO("pstChTask: %p u32Reserved1: %x.\n", pstChnTask, pstChnTask->u32Reserved1);
                }
            }
#if defined(VPE_SUPPORT_RGN)  && (VPE_SUPPORT_RGN == 1)
            //notify region to deal with cover and OSD
            if(pstDevInfo->eRunningMode & SUPPORT_RGN)
            {
                if(MI_RGN_OK != mi_rgn_NotifyFenceDone(VPE_GET_TASK_RGN_FENCE(pstChnTask)))
                {
                    DBG_ERR(" mi_rgn_NotifyFenceDone failed. u64Fence = %u.\n", VPE_GET_TASK_RGN_FENCE(pstChnTask));
                }
            }
#endif

            if (pstChnTask->u32Reserved1 & MI_VPE_TASK_NO_DRAM_OUTPUT)
            {
                pstDevInfo->u32TaskNoToDramCnt--;
            }

            MI_SYS_BUG_ON(atomic_read(&pstDevInfo->stChnnInfo[pstChnTask->u32ChnId].stAtomTask) == 0);
            atomic_dec_return(&pstDevInfo->stChnnInfo[pstChnTask->u32ChnId].stAtomTask);

            VPE_PERF_TIME(&VPE_RELEASE_TASK_TIME(pstChnTask));

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
            _MI_VPE_DebugGetTaskTime(pstChnTask);
            _MI_VPE_DebugGetBufCnt(pstChnTask);
#endif

            if(E_MI_VPE_RUNNING_MODE_REALTIME_MODE == pstDevInfo->eRunningMode)
            {
               //config RealTimeMode PTS
                MI_U8 u8PortId =0;
                MI_U8 u8NodeId=0;
                MI_U32 u32TaskFrameFence = 0;
                u32TaskFrameFence = pstChnTask->u32Reserved0;
                for(u8PortId=0; u8PortId <MI_VPE_MAX_PORT_NUM; u8PortId++)
                {
                    mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(pstChnTask->u32ChnId, u8PortId);
                    if(TRUE == pstOutPortInfo->bEnable && pstChnTask->astOutputPortBufInfo[u8PortId] != NULL)
                    {
                       pstChnTask->astOutputPortBufInfo[u8PortId]->u64Pts = pstDevInfo->u64FramePts;
                    }
                }
            //drop frame
#if 1
                for(u8NodeId =0; u8NodeId<= MI_VPE_MAX_WORKINGLIST_NODE; u8NodeId++)
                {
                    if(OCCUPY == pstDevInfo->stDropFrameFence[u8NodeId].bStatus)
                    {
                        if(u32TaskFrameFence == pstDevInfo->stDropFrameFence[u8NodeId].u16DropFence)
                        {
                            if(pstChnnInfo->u16DumpTaskFileNum > 0)
                            {
                                _MI_VPE_WriteTaskFile(pstChnTask);
                                pstChnnInfo->u16DumpTaskFileNum -- ;
                            }

                            mi_sys_DropTask(pstChnTask);
                            pstDevInfo->stDropFrameFence[u8NodeId].bStatus = EMPTY;
                            bDroped = TRUE;
                            DBG_WRN("Drop Fence is %d\n",u32TaskFrameFence);
                        }
                        else if(u32TaskFrameFence > pstDevInfo->stDropFrameFence[u8NodeId].u16DropFence)
                        {
                            DBG_WRN("DROP Fence %d missed, maybe picture is bad \n", pstDevInfo->stDropFrameFence[u8NodeId].u16DropFence);
                            pstDevInfo->stDropFrameFence[u8NodeId].bStatus = EMPTY;
                            pstDevInfo->stDropFrameFence[u8NodeId].u16DropFence =0;
                        }
                    }

                }
            }
#endif

            // Finish task to mi_sys
            if(bDroped == FALSE)
            {
				MI_U8 u8PortId;
                 
                 for(u8PortId=0; u8PortId <MI_VPE_MAX_PORT_NUM; u8PortId++)
                {
                    mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(pstChnTask->u32ChnId, u8PortId);
                    if(TRUE == pstOutPortInfo->bEnable && pstChnTask->astOutputPortBufInfo[u8PortId] != NULL)
                    {
                       break;
                    }
                }
				//DBG_ERR("u8PortId: %d,bEnableLowLatencyReceiveMode: %d\n",u8PortId,pstDevInfo->stChnnInfo[pstChnTask->u32ChnId].stOutPortInfo[u8PortId].stPeerOutputPortInfo.bEnableLowLatencyReceiveMode);
				if(pstDevInfo->stChnnInfo[pstChnTask->u32ChnId].stOutPortInfo[u8PortId].stPeerOutputPortInfo.bEnableLowLatencyReceiveMode == TRUE)
					s32Ret = mi_sys_FinishAndReleaseTask_LL(pstChnTask);
				else
				{
					s32Ret = mi_sys_FinishAndReleaseTask(pstChnTask);
				}

            }

            DBG_INFO("s32Ret = %d.\n", s32Ret);

            DBG_INFO("Release data.\n");
        }
        up(&VPE_working_list_sem);
    }
        // release working task list
    down(&VPE_working_list_sem);
    while(!list_empty(&VPE_working_task_list))
    {
        mi_sys_ChnTaskInfo_t *pstChnTask;
        pstChnTask = container_of(VPE_working_task_list.next, mi_sys_ChnTaskInfo_t, listChnTask);
        if (pstChnTask != NULL)
        {
            list_del(&pstChnTask->listChnTask);
            mi_sys_RewindTask(pstChnTask);
        }
    }
    up(&VPE_working_list_sem);
    return 0;
}

static irqreturn_t _MI_VPE_Isr(int irq, void *data)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)data;
    MHalVpeIrqStatus_e eIrqStatus;
    MI_U8 u8NodeId=0;
    memset(&eIrqStatus, 0, sizeof(eIrqStatus));

    eIrqStatus = MHalVpeSclCheckIrq(pstDevInfo->eIrqEnableMode);
    if (eIrqStatus > E_MHAL_SCL_IRQ_NONE)
    {
        ExecFunc(MHalVpeSclClearIrq(pstDevInfo->eIrqEnableMode));
        if(pstDevInfo->eRunningMode == E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
        {
            struct timeval sttime;

            pstDevInfo->u16RTCmdqFence = _MI_VPE_ReadFence(pstDevInfo->pstCmdMloadInfo);
            //vsync and dma done happen same time
            if(eIrqStatus == (E_MHAL_SCL_IRQ_VSYNC | E_MHAL_SCL_IRQ_FRAMEDONE))
            {
                //DBG_ERR("DMA done with Frame Done same time happend\n");
                for(u8NodeId =0; u8NodeId< MI_VPE_MAX_WORKINGLIST_NODE; u8NodeId++)
                {
                    if(pstDevInfo->stDropFrameFence[u8NodeId].bStatus == EMPTY)
                    {
                        pstDevInfo->stDropFrameFence[u8NodeId].u16DropFence = pstDevInfo->u16RTCmdqFence;
                        pstDevInfo->stDropFrameFence[u8NodeId].bStatus = OCCUPY;
                        break;
                    }
                }
            }

            memset(&sttime, 0, sizeof(sttime));
            do_gettimeofday(&sttime);
            pstDevInfo->u64FramePts = (sttime.tv_sec*1000000 + sttime.tv_usec);
       }

        WAKE_UP_QUEUE_IF_NECESSARY(vpe_isr_waitqueue);
    }

    return IRQ_HANDLED;
}

static mi_sys_TaskIteratorCBAction_e _MI_VPE_TaskIteratorCallBK(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData)
{
    int i;
    int valid_output_port_cnt = 0;
    mi_vpe_IteratorWorkInfo_t *workInfo = (mi_vpe_IteratorWorkInfo_t *)pUsrData;
    mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(pstTaskInfo->u32ChnId);
    mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
    if(pstChnnInfo->u64GetInputBufferCnt < VPE_GET_INBUFCNT_MAX)
        pstChnnInfo->u64GetInputBufferCnt += 1;
    else
    {
        pstChnnInfo->u64GetInputBufferCnt =0;
        pstChnnInfo->u64GetInputBufferTodoCnt =0;
    }
#endif

    LOCK_CHNN(pstChnnInfo);
    DBG_INFO("Chnn: %d got data.\n",  pstTaskInfo->u32ChnId);

    // Check Channel stop or created ??
    if ((pstChnnInfo->bCreated == FALSE) ||
        (pstChnnInfo->eStatus != E_MI_VPE_CHANNEL_STATUS_START)
        )
    {
        // Drop can not process input buffer
        DBG_ERR("Ch %d is not create or Ch %d is stop. Drop frame directly.\n",
                pstTaskInfo->u32ChnId, pstTaskInfo->u32ChnId);
        mi_sys_FinishAndReleaseTask(pstTaskInfo);
        UNLOCK_CHNN(pstChnnInfo);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }

    //Real Time Mode only Support META data
    if(E_MI_VPE_RUNNING_MODE_REALTIME_MODE == pstDevInfo->eRunningMode
        && pstTaskInfo->astInputPortBufInfo[0]->eBufType != E_MI_SYS_BUFDATA_META)
    {
        DBG_ERR("Ch %d Real Time Mode but input Buffer Type is not MetaData.\n",
                        pstTaskInfo->u32ChnId);
        mi_sys_FinishAndReleaseTask(pstTaskInfo);
        UNLOCK_CHNN(pstChnnInfo);
        MI_SYS_BUG();
    }

    //not Real Time Mode Only Support Frame Data
    if (pstDevInfo->eRunningMode != E_MI_VPE_RUNNING_MODE_REALTIME_MODE
        && pstTaskInfo->astInputPortBufInfo[0]->eBufType != E_MI_SYS_BUFDATA_FRAME)
    {
         // Drop can not process input buffer
         DBG_ERR("Ch %d is not support buffer pixel format.\n",
                pstTaskInfo->u32ChnId);
        mi_sys_FinishAndReleaseTask(pstTaskInfo);

        UNLOCK_CHNN(pstChnnInfo);
        MI_SYS_BUG();
        //return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }

    for (i = 0; i < MI_VPE_MAX_PORT_NUM; i++)
    {
        pstTaskInfo->astOutputPortPerfBufConfig[i].u64TargetPts = pstTaskInfo->astInputPortBufInfo[0]->u64Pts;
        pstTaskInfo->astOutputPortPerfBufConfig[i].eBufType     = E_MI_SYS_BUFDATA_FRAME;
        pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.eFormat = pstChnnInfo->stOutPortInfo[i].stPortMode.ePixelFormat;
        pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.eFrameScanMode =  pstTaskInfo->astInputPortBufInfo[0]->stFrameData.eFrameScanMode;
        pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.u16Width = pstChnnInfo->stOutPortInfo[i].stPortMode.u16Width;
        pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.u16Height= pstChnnInfo->stOutPortInfo[i].stPortMode.u16Height;

        DBG_INFO("[%d]->{ u64TargetPts: 0x%llx eBufType: %d, eFormat = %d, eFrameScanMode: %d u16Width = %d, u16Height: %d.\n",
            i,
            pstTaskInfo->astOutputPortPerfBufConfig[i].u64TargetPts, pstTaskInfo->astOutputPortPerfBufConfig[i].eBufType,
            pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.eFormat, pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.eFrameScanMode, pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.u16Width,
            pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.u16Height
        );
    }
    UNLOCK_CHNN(pstChnnInfo);

    if(mi_sys_PrepareTaskOutputBuf(pstTaskInfo) != MI_SUCCESS)
    {
        DBG_ERR("Ch %d mi_sys_PrepareTaskOutputBuf failed.\n",
                        pstTaskInfo->u32ChnId);
        return MI_SYS_ITERATOR_SKIP_CONTINUTE;
    }

    for(i = 0; i < MI_VPE_MAX_PORT_NUM; i++)
    {
#if 0
        if (NULL != pstTaskInfo->astOutputPortBufInfo[i])
        printk("[VPE_IMPL]: %s [%d]: Flip: Get InputBuffer: 0x%llx OutputBuffer: 0x%llx.\n", __func__, __LINE__,
            pstTaskInfo->astInputPortBufInfo[0]->stFrameData.phyAddr[0], pstTaskInfo->astOutputPortBufInfo[i]->stFrameData.phyAddr[0]);
#endif
        // Incase SYS mask output as disable according FRC, port output buffer must be NULL.
        MI_SYS_BUG_ON(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i] && pstTaskInfo->astOutputPortBufInfo[i]);

         if(pstTaskInfo->astOutputPortBufInfo[i])
        {
            valid_output_port_cnt++;
            #if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
                pstChnnInfo->stOutPortInfo[i].u64GetOutputBufferCnt ++;
                pstChnnInfo->stOutPortInfo[i].u32OutbufStride = pstTaskInfo->astOutputPortBufInfo[i]->stFrameData.u32Stride[0];
                pstChnnInfo->u64InBuffStride = pstTaskInfo->astInputPortBufInfo[0]->stFrameData.u32Stride[0];
            #endif
        }
        else
        {
            if(pstChnnInfo->stOutPortInfo[i].bEnable == TRUE)
            {
                #if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
                    pstChnnInfo->stOutPortInfo[i].u64GetOutputBuffFailCnt += 1;
                #endif
            }
        }
    }

    //check if lack of output buf
    if(valid_output_port_cnt==0)
    {
#ifdef  MI_SYS_SERIOUS_ERR_MAY_MULTI_TIMES_SHOW
                DBG_ERR("Ch %d valid_output_port_cnt %d.\n",
                        pstTaskInfo->u32ChnId, valid_output_port_cnt);
#endif
        return MI_SYS_ITERATOR_SKIP_CONTINUTE;
    }

    //TODO Just for Test
    VPE_PERF_TIME(&VPE_GOT_TASK_TIME(pstTaskInfo));
    pstTaskInfo->u32Reserved0 = 0;
    pstTaskInfo->u32Reserved1 = 0;
    pstTaskInfo->u64Reserved0 = 0;
    pstTaskInfo->u64Reserved1 = 0;

    atomic_inc(&pstChnnInfo->stAtomTask);

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
    pstChnnInfo->u64GetInputBufferTodoCnt +=1;

    if(pstChnnInfo->bCheckFramePts == TRUE)
    {
        PRINTF_PROC("ChnID %d, PTS = %lld\n",
            pstTaskInfo->u32ChnId,pstTaskInfo->astInputPortBufInfo[0]->u64Pts);
    }
#endif
    list_add_tail(&pstTaskInfo->listChnTask, &VPE_todo_task_list);

    //we at most process 32 batches at one time
    if(++workInfo->totalAddedTask >= MI_VPE_FRAME_PER_BURST_CMDQ)
    {
        DBG_INFO("Chnn: %d workInfo->totalAddedTask: %d stop.\n",  pstTaskInfo->u32ChnId, workInfo->totalAddedTask);
        return MI_SYS_ITERATOR_ACCEPT_STOP;
    }
    else
    {
        DBG_INFO("Chnn: %d workInfo->totalAddedTask: %d.\n",  pstTaskInfo->u32ChnId, workInfo->totalAddedTask);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }
}

static MI_S32 _MI_VPE_SclAttrConfig(MI_U8 u8PortId, MI_SYS_FrameData_t *pstFrameData, mi_vpe_ChannelInfo_t *pstChnnInfo)
{
    mi_vpe_OutPortInfo_t *pstPortInfo = NULL;
    pstPortInfo =  &pstChnnInfo->stOutPortInfo[u8PortId];

    if (pstPortInfo->eRealOutputPixelFormat != pstFrameData->ePixelFormat)
    {
        MHalVpeSclDmaConfig_t stDmaCfg;
        memset(&stDmaCfg, 0, sizeof(stDmaCfg));
        stDmaCfg.enPort = (MHalVpeDmaPort_e)u8PortId;
        stDmaCfg.enCompress= pstPortInfo->stPortMode.eCompressMode;
        stDmaCfg.enFormat=pstFrameData->ePixelFormat;
        if (TRUE == MHalVpeSclDmaPortConfig(pstChnnInfo->pSclCtx, &stDmaCfg))
        {
            pstPortInfo->eRealOutputPixelFormat = stDmaCfg.enFormat;
        }
        else
        {
            DBG_ERR("Ch: %d port %d set output size fail.\n", pstChnnInfo->VpeCh, u8PortId);
        }
    }

    // Tommy: need add check output buffer --> disp window change ??
    if ((pstFrameData->u16Width != pstPortInfo->stPortMode.u16Width)
        || (pstFrameData->u16Height != pstPortInfo->stPortMode.u16Height)
        )
    {
        MHalVpeSclOutputSizeConfig_t stOutputSize;

        pstPortInfo->stPortMode.u16Width = pstFrameData->u16Width;
        pstPortInfo->stPortMode.u16Height = pstFrameData->u16Height;

        memset(&stOutputSize, 0, sizeof(stOutputSize));
        stOutputSize.enOutPort = (MHalVpeDmaPort_e)(u8PortId);
        stOutputSize.u16Width  = pstPortInfo->stPortMode.u16Width;
        stOutputSize.u16Height = pstPortInfo->stPortMode.u16Height;
        ExecFunc(MHalVpeSclPortSizeConfig(pstChnnInfo->pSclCtx, &stOutputSize));
    }
    return MI_SUCCESS;
}

// TODO: implement by region ownner
//extern MI_S32 VPE_osd_process(MI_REG_CmdInfo_t CmdInfo, MHAL_CMDQ_CmdqInterface_t *cmdinf);
static MI_S32 _MI_VPE_ProcessTask(mi_sys_ChnTaskInfo_t *pstTask, mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_InBuffInfo_t *pstInBuffInfo, MI_BOOL bSkipIQUpdate)
{
    MI_U8 u8PortId =0;
    MI_BOOL bNeedRoi = FALSE;
    MI_BOOL bNeed3DnrUpdate = FALSE;
    MI_BOOL bUseMDWIN = FALSE;
    MI_BOOL bUseDMA = FALSE;
    mi_vpe_ChannelInfo_t *pstChnnInfo = &pstDevInfo->stChnnInfo[pstTask->u32ChnId];
    MHalVpeSclOutputBufferConfig_t stVpeOutputBuffer;
    MHalVpeIspVideoInfo_t stIspVideoInfo;
    ROI_Task_t *pstRoiTask = NULL;
    MHalVpeWaitDoneType_e eDoneType = E_MHAL_VPE_WAITDONE_ERR;

#if defined(VPE_SUPPORT_RGN)  && (VPE_SUPPORT_RGN == 1)
    mi_rgn_ProcessCmdInfo_t stRgnCmd;
    memset(&stRgnCmd, 0, sizeof(stRgnCmd));
#endif
    memset(&stVpeOutputBuffer, 0, sizeof(stVpeOutputBuffer));
    // Update ISP input buffer address
    memset(&stIspVideoInfo, 0, sizeof(stIspVideoInfo));

    for (u8PortId = 0; u8PortId < MI_VPE_MAX_PORT_NUM; u8PortId++)
    {
        mi_vpe_OutPortInfo_t *pstPortInfo = NULL;
        pstPortInfo =  &pstChnnInfo->stOutPortInfo[u8PortId];

        // TODO: Tommy: Think about output Buffer ALL NULL
        // Port enable + port no buffer
        if ((pstPortInfo->bEnable == TRUE) && (NULL != pstTask->astOutputPortBufInfo[u8PortId]))
        {
            #if defined(VPE_SUPPORT_RGN)  && (VPE_SUPPORT_RGN == 1)
            stRgnCmd.u32chnID = pstTask->u32ChnId;
            stRgnCmd.stVpePort[u8PortId].bEnable = TRUE;
            stRgnCmd.stVpePort[u8PortId].u32Width = pstTask->astOutputPortBufInfo[u8PortId]->stFrameData.u16Width;
            stRgnCmd.stVpePort[u8PortId].u32Height = pstTask->astOutputPortBufInfo[u8PortId]->stFrameData.u16Height;
            #endif
            DBG_INFO("Ch: %d Port: %d eRealOutputPixelFormat: %d out buffer PixelFormat: %d.\n", pstTask->u32ChnId, u8PortId, pstPortInfo->eRealOutputPixelFormat,
                pstTask->astOutputPortBufInfo[u8PortId]->stFrameData.ePixelFormat);

            if(pstDevInfo->eRunningMode & SUPPORT_SCL)
                _MI_VPE_SclAttrConfig(u8PortId, &pstTask->astOutputPortBufInfo[u8PortId]->stFrameData, pstChnnInfo);

            // Port MDWIN enable
            if (u8PortId == VPE_OUTPUT_PORT_FOR_MDWIN)
            {
                bUseMDWIN = TRUE;
            }
            else
            {
                bUseDMA = TRUE;
            }

            // Address
            stVpeOutputBuffer.stCfg[u8PortId].stBufferInfo.u32Stride[0] = pstTask->astOutputPortBufInfo[u8PortId]->stFrameData.u32Stride[0];
            stVpeOutputBuffer.stCfg[u8PortId].stBufferInfo.u32Stride[1] = pstTask->astOutputPortBufInfo[u8PortId]->stFrameData.u32Stride[1];
            stVpeOutputBuffer.stCfg[u8PortId].stBufferInfo.u32Stride[2] = pstTask->astOutputPortBufInfo[u8PortId]->stFrameData.u32Stride[2];
            stVpeOutputBuffer.stCfg[u8PortId].stBufferInfo.u64PhyAddr[0] = pstTask->astOutputPortBufInfo[u8PortId]->stFrameData.phyAddr[0];
            stVpeOutputBuffer.stCfg[u8PortId].stBufferInfo.u64PhyAddr[1] = pstTask->astOutputPortBufInfo[u8PortId]->stFrameData.phyAddr[1];
            stVpeOutputBuffer.stCfg[u8PortId].stBufferInfo.u64PhyAddr[2] = pstTask->astOutputPortBufInfo[u8PortId]->stFrameData.phyAddr[2];

            // Enable
            stVpeOutputBuffer.stCfg[u8PortId].bEn = TRUE;

            #if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
            pstPortInfo->u64BufToHalCnt += 1;
            #endif

        }
        else
        {
            #if defined(VPE_SUPPORT_RGN)  && (VPE_SUPPORT_RGN == 1)
            stRgnCmd.stVpePort[u8PortId].bEnable = FALSE;
            #endif
            stVpeOutputBuffer.stCfg[u8PortId].bEn = FALSE;
        }
    }

#if defined(VPE_SUPPORT_RGN)  && (VPE_SUPPORT_RGN == 1)
    // OSD region process
    // TODO: Need add region OSD API.
    // VPE_osd_process(MI_REG_CmdInfo_t CmdInfo, MHAL_CMDQ_CmdqInterface_t * cmdinf);
    //call region HAL layer channel ID and module ID ,output size,
    //if the channel id is 32, close OSD and cover for private channel.
    if(pstDevInfo->eRunningMode & SUPPORT_RGN)
    {
        if(MI_RGN_OK != mi_rgn_VpeProcess(&stRgnCmd, pstDevInfo->pstCmdMloadInfo, &VPE_GET_TASK_RGN_FENCE(pstTask)))
        {
            DBG_ERR("mi_rgn_VpeProcess failed.\n");
            DBG_ERR("rgn Fence = %llx.\n", VPE_GET_TASK_RGN_FENCE(pstTask));
        }
    }
#endif
    // Check ROI
    if(pstDevInfo->eRunningMode & SUPPORT_DNR_ROI)
    {
        if (bSkipIQUpdate == FALSE)
        {
            bNeedRoi = mi_vpe_impl_RoiGetTask(pstTask, pstDevInfo, &pstRoiTask, &VPE_roi_task_list, &VPE_roi_task_list_sem);
            if (bNeedRoi == TRUE)
            {
                MI_SYS_BUG_ON(pstRoiTask == NULL);
                MI_SYS_BUG_ON(pstRoiTask->u32MagicNumber != __MI_VPE_ROI_MAGIC__);
                mi_vpe_impl_RoiProcessTaskStart(pstTask, pstDevInfo, pstChnnInfo, &pstTask->astOutputPortBufInfo[0]->stFrameData, pstRoiTask);
            }

            // Check 3DNR
            bNeed3DnrUpdate = mi_vpe_impl_3DnrUpdateProcessStart(pstDevInfo, pstChnnInfo, pstTask);
        }
    }

    // input buffer address
    stIspVideoInfo.u32Stride[0] = pstInBuffInfo->u32Stride[0];
    stIspVideoInfo.u32Stride[1] = pstInBuffInfo->u32Stride[1];
    stIspVideoInfo.u32Stride[2] = pstInBuffInfo->u32Stride[2];
    // Offset for sw crop in ISP for capture memory.
    stIspVideoInfo.u64PhyAddr[0] = pstInBuffInfo->phyAddr[0] + pstChnnInfo->u64PhyAddrOffset[0];
    stIspVideoInfo.u64PhyAddr[1] = pstInBuffInfo->phyAddr[1] + pstChnnInfo->u64PhyAddrOffset[1];
    stIspVideoInfo.u64PhyAddr[2] = pstInBuffInfo->phyAddr[2] + pstChnnInfo->u64PhyAddrOffset[2];

    stIspVideoInfo.u64MetaInfo = pstInBuffInfo->u64MetaInfo;

    VPE_PERF_TIME(&VPE_PROCESS_TASK_TIME(pstTask));
    if(pstDevInfo->eRunningMode & SUPPORT_SCL)
    {
        ExecFunc(MHalVpeSclProcess(pstChnnInfo->pSclCtx, pstDevInfo->pstCmdMloadInfo, &stVpeOutputBuffer));
    }

    if(pstDevInfo->eRunningMode & SUPPORT_IQ)
    {
        ExecFunc(MHalVpeIqProcess(pstChnnInfo->pIqCtx,   pstDevInfo->pstCmdMloadInfo));
    }

    if(pstDevInfo->eRunningMode & SUPPORT_ISP)
    {
        ExecFunc(MHalVpeIspProcess(pstChnnInfo->pIspCtx, pstDevInfo->pstCmdMloadInfo, &stIspVideoInfo));
    }

    // Wait MDWIN Done
    if ((bUseMDWIN == TRUE) || (bUseDMA == TRUE))
    {
        if ((bUseMDWIN == TRUE) && (bUseDMA == TRUE))
        {
            eDoneType = E_MHAL_VPE_WAITDONE_DMAANDMDWIN;
        }
        else if (bUseDMA == TRUE)
        {
            eDoneType = E_MHAL_VPE_WAITDONE_DMAONLY;
        }
        else if (bUseMDWIN == TRUE)
        {
            eDoneType = E_MHAL_VPE_WAITDONE_MDWINONLY;
        }

        if(pstDevInfo->eRunningMode & SUPPORT_SCL)
        {
            ExecFunc(MHalVpeSclSetWaitDone(pstChnnInfo->pSclCtx, pstDevInfo->pstCmdMloadInfo, eDoneType));
        }
    }
    else
    {
        //MI_SYS_BUG();
        pstTask->u32Reserved1 |= MI_VPE_TASK_NO_DRAM_OUTPUT;
        pstDevInfo->u32TaskNoToDramCnt++;
    }

    if(pstDevInfo->eRunningMode & SUPPORT_DNR_ROI)
    {
        if (bNeedRoi == TRUE)
        {
            mi_vpe_impl_RoiProcessTaskEnd(pstDevInfo, pstChnnInfo);
        }

        if (bNeed3DnrUpdate == TRUE)
        {
            mi_vpe_impl_3DnrUpdateProcessEnd(pstDevInfo, pstChnnInfo);
        }
    }

    return MI_SUCCESS;
}

static MI_BOOL _MI_VPE_CheckInputChanged(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_BOOL bRet = FALSE;
    mi_vpe_ChannelInfo_t *pstChnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);
    mi_vif_InputInfo_t *pstMetaDataInfo = NULL;
    MI_SYS_FrameData_t *pstBuffFrame = NULL;
    mi_vpe_InBuffInfo_t stInBuffInfo;

    memset(&stInBuffInfo, 0, sizeof(stInBuffInfo));

    if(pstChnTask->astInputPortBufInfo[0]->eBufType == E_MI_SYS_BUFDATA_META)
    {
        pstMetaDataInfo = (mi_vif_InputInfo_t *)pstChnTask->astInputPortBufInfo[0]->stMetaData.pVirAddr;
        stInBuffInfo.u16Width = pstMetaDataInfo->u16Width;
        stInBuffInfo.u16Height = pstMetaDataInfo->u16Height;
    }
    else if(pstChnTask->astInputPortBufInfo[0]->eBufType == E_MI_SYS_BUFDATA_FRAME)
    {
        pstBuffFrame = GET_VPE_BUFF_FRAME_FROM_TASK(pstChnTask);
        stInBuffInfo.u16Width = pstBuffFrame->u16Width;
        stInBuffInfo.u16Height = pstBuffFrame->u16Height;
        stInBuffInfo.eCompressMode = pstBuffFrame->eCompressMode;
        stInBuffInfo.ePixelFormat = pstBuffFrame->ePixelFormat;
    }
    // Check source window changed ?
    DBG_INFO("pstChnInfo->stSrcWin.u16Width %d X pstBuffFrame->u16Width: %d.\n", pstChnInfo->stSrcWin.u16Width , stInBuffInfo.u16Width);
    DBG_INFO("pstChnInfo->stSrcWin.u16Height %d X pstBuffFrame->u16Height: %d.\n", pstChnInfo->stSrcWin.u16Height , stInBuffInfo.u16Height);

    if ((pstChnInfo->stSrcWin.u16Width == stInBuffInfo.u16Width)
        && (pstChnInfo->stSrcWin.u16Height == stInBuffInfo.u16Height)
        &&(pstChnInfo->stSrcWin.eCompressMode == stInBuffInfo.eCompressMode)
        &&(pstChnInfo->stChnnAttr.ePixFmt == stInBuffInfo.ePixelFormat)
        )
    {
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
        pstChnInfo->stSrcWin.u16Width = stInBuffInfo.u16Width;
        pstChnInfo->stSrcWin.u16Height= stInBuffInfo.u16Height;
        pstChnInfo->stSrcWin.eCompressMode= stInBuffInfo.eCompressMode;
        pstChnInfo->stChnnAttr.ePixFmt = stInBuffInfo.ePixelFormat;
    }

    return bRet;
}

static MI_BOOL _MI_VPE_CheckRotationChanged(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_BOOL bRet = FALSE;
    mi_vpe_ChannelInfo_t *pstChnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);
    if (pstChnInfo->eRealRotationType != pstChnInfo->eRotationType)
    {
        bRet = TRUE;
        pstChnInfo->eRealRotationType = pstChnInfo->eRotationType;
        DBG_INFO("eRealRotationType: %d, eRotationType: %d.\n", pstChnInfo->eRealRotationType, pstChnInfo->eRotationType);
    }
    return bRet;
}

// TODO:  Tommy: Add PerChannel semaphore for global var.

static MI_BOOL _MI_VPE_CheckCropChanged(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_BOOL bRet = FALSE;
    MI_SYS_WindowRect_t stRealCrop = {0,0,0,0};
    mi_vpe_ChannelInfo_t *pstChnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);

    // Calculator real cropWindow
    if ((pstChnInfo->stCropWin.u16X + pstChnInfo->stCropWin.u16Width) > pstChnInfo->stSrcWin.u16Width)
    {
        bRet = TRUE;
        stRealCrop.u16X     = 0;
        stRealCrop.u16Width = pstChnInfo->stSrcWin.u16Width;
    }
    else
    {
        bRet = FALSE;
        stRealCrop.u16X     = pstChnInfo->stCropWin.u16X;
        stRealCrop.u16Width = pstChnInfo->stCropWin.u16Width;

    }
    if ((pstChnInfo->stCropWin.u16Y + pstChnInfo->stCropWin.u16Height) > pstChnInfo->stSrcWin.u16Height)
    {
        bRet = TRUE;
        stRealCrop.u16Y     = 0;
        stRealCrop.u16Height = pstChnInfo->stSrcWin.u16Height;
    }
    else
    {
        bRet = FALSE;
        stRealCrop.u16Y     = pstChnInfo->stCropWin.u16Y;
        stRealCrop.u16Height = pstChnInfo->stCropWin.u16Height;
    }

    //if we have sideband crop msg task
    if( pstChnInfo->w_zoom_in_ratio & pstChnInfo->h_zoom_in_ratio)
    {
        MI_SYS_BUG_ON(pstChnInfo->x_zoom_in_ratio+pstChnInfo->w_zoom_in_ratio > VPE_ZOOM_RATION_DEN);
        MI_SYS_BUG_ON(pstChnInfo->y_zoom_in_ratio+pstChnInfo->h_zoom_in_ratio > VPE_ZOOM_RATION_DEN);

        stRealCrop.u16X += (MI_U16)((MI_U32)pstChnInfo->x_zoom_in_ratio*stRealCrop.u16Width/VPE_ZOOM_RATION_DEN);
        stRealCrop.u16Y += (MI_U16)((MI_U32)pstChnInfo->y_zoom_in_ratio*stRealCrop.u16Height/VPE_ZOOM_RATION_DEN);
        stRealCrop.u16Width= (MI_U16)((MI_U32)pstChnInfo->w_zoom_in_ratio*stRealCrop.u16Width/VPE_ZOOM_RATION_DEN);
        stRealCrop.u16Height= (MI_U16)((MI_U32)pstChnInfo->h_zoom_in_ratio*stRealCrop.u16Height/VPE_ZOOM_RATION_DEN);
    }

    // Change Crop window Changed?
    if (((stRealCrop.u16X != pstChnInfo->stRealCrop.u16X)
        || (stRealCrop.u16Width != pstChnInfo->stRealCrop.u16Width)
        || (stRealCrop.u16Y != pstChnInfo->stRealCrop.u16Y)
        || (stRealCrop.u16Height != pstChnInfo->stRealCrop.u16Height)
        ))
    {
         pstChnInfo->stRealCrop.u16X = stRealCrop.u16X;
         pstChnInfo->stRealCrop.u16Width = stRealCrop.u16Width;
         pstChnInfo->stRealCrop.u16Y = stRealCrop.u16Y;
         pstChnInfo->stRealCrop.u16Height = stRealCrop.u16Height;

        bRet = TRUE;
    }

    return bRet;
}

static MI_BOOL _MI_VPE_CalcCropInfo(mi_vpe_ChannelInfo_t *pstChnInfo, MI_SYS_WindowRect_t *pstHwCrop, MI_SYS_WindowRect_t *pstSwCrop)
{
    MI_BOOL bRet = TRUE;
    int left_top_x, left_top_y, right_bottom_x, right_bottom_y;
    MI_U32 u32PixelAlign = 0, u32PixelPerBytes = 0;
    switch (pstChnInfo->stChnnAttr.ePixFmt)
    {
    case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
        u32PixelAlign    = YUV422_PIXEL_ALIGN;
        u32PixelPerBytes = YUV422_BYTE_PER_PIXEL;
        break;
    case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        u32PixelAlign    = YUV420_PIXEL_ALIGN;
        u32PixelPerBytes = YUV420_BYTE_PER_PIXEL;
        break;
    default:
        bRet = FALSE;
    }

    if (bRet == TRUE)
    {
        left_top_y = ALIGN_DOWN(pstChnInfo->stRealCrop.u16Y, u32PixelAlign);
        left_top_x = ALIGN_DOWN(pstChnInfo->stRealCrop.u16X, (MIU_BURST_BITS/(u32PixelPerBytes*8)));
        right_bottom_y = ALIGN_UP(pstChnInfo->stRealCrop.u16Y + pstChnInfo->stRealCrop.u16Height, u32PixelAlign);
        right_bottom_x = ALIGN_UP(pstChnInfo->stRealCrop.u16X + pstChnInfo->stRealCrop.u16Width, (MIU_BURST_BITS/(u32PixelPerBytes*8)));

        pstSwCrop->u16X = left_top_x;
        pstSwCrop->u16Width = right_bottom_x-left_top_x;
        pstSwCrop->u16Y = left_top_y;
        pstSwCrop->u16Height = right_bottom_y-left_top_y;

        if (pstChnInfo->stSrcWin.u16Width < pstSwCrop->u16Width)
        {
            pstSwCrop->u16Width = pstChnInfo->stSrcWin.u16Width;
        }

        if (pstChnInfo->stSrcWin.u16Height < pstSwCrop->u16Height)
        {
            pstSwCrop->u16Height = pstChnInfo->stSrcWin.u16Height;
        }

        pstHwCrop->u16X = pstChnInfo->stRealCrop.u16X-left_top_x;
        pstHwCrop->u16Y = pstChnInfo->stRealCrop.u16Y-left_top_y;

        pstHwCrop->u16Width = pstChnInfo->stRealCrop.u16Width;
        pstHwCrop->u16Height = pstChnInfo->stRealCrop.u16Height;
        // HW crop position and size need 2 align
        pstHwCrop->u16Y = ALIGN_UP(pstHwCrop->u16Y, 2);
        pstHwCrop->u16Height = ALIGN_DOWN(pstHwCrop->u16Height, 2);

        pstHwCrop->u16X = ALIGN_UP(pstHwCrop->u16Y, 2);
        pstHwCrop->u16Width = ALIGN_DOWN(pstHwCrop->u16Width, 2);
    }
    else
    {
        DBG_WRN("UnSupport pixel format: %d.\n", pstChnInfo->stChnnAttr.ePixFmt);
    }
    DBG_INFO("swCrop: {x: %u, y: %u, width: %u, height: %u}.\n", pstSwCrop->u16X, pstSwCrop->u16Y, pstSwCrop->u16Width, pstSwCrop->u16Height);
    DBG_INFO("hwCrop: {x: %u, y: %u, width: %u, height: %u}.\n", pstHwCrop->u16X, pstHwCrop->u16Y, pstHwCrop->u16Width, pstHwCrop->u16Height);

    return bRet;
}

static MI_BOOL _MI_VPE_UpdateCropAddress(mi_sys_ChnTaskInfo_t *pstChnTask, MI_SYS_WindowRect_t *pstSwCrop)
{
    MI_BOOL bRet = FALSE;
    MI_PHY u64PhyAddr = 0;
    if (pstChnTask != NULL)
    {
        mi_vpe_ChannelInfo_t *pstChnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);
        MI_SYS_FrameData_t *pstBuffFrame = GET_VPE_BUFF_FRAME_FROM_TASK(pstChnTask);
        switch (pstBuffFrame->ePixelFormat)
        {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
        {
            MI_U32 u32BytePerPixel = 2;
            u64PhyAddr =  (pstSwCrop->u16Y * pstBuffFrame->u32Stride[0]);
            u64PhyAddr += pstSwCrop->u16X * u32BytePerPixel;
            pstChnInfo->u64PhyAddrOffset[0] = u64PhyAddr;
            pstChnInfo->u64PhyAddrOffset[1] = 0;
            pstChnInfo->u64PhyAddrOffset[2] = 0;

        }
        break;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        {
            MI_U32 u32LumaBytePerPixel = 1;
            u64PhyAddr =  (pstSwCrop->u16Y * pstBuffFrame->u32Stride[0]);
            u64PhyAddr += pstSwCrop->u16X * u32LumaBytePerPixel;
            pstChnInfo->u64PhyAddrOffset[0] = u64PhyAddr;

            u64PhyAddr =   (pstSwCrop->u16Y/2 * pstBuffFrame->u32Stride[0]);
            u64PhyAddr +=  pstSwCrop->u16X; // TODO: tommy: Check UV whether need /2 ?
            pstChnInfo->u64PhyAddrOffset[1] = u64PhyAddr;
            pstChnInfo->u64PhyAddrOffset[2] = 0;
        }
        break;
        default:
            bRet = FALSE;
        }
    }

    return bRet;
}

static inline MI_BOOL _MI_VPE_ProcessTaskSideBandMsg(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_S32 i = 0;
    MI_S32 s32OutBufCnt = 0;
    MI_U64 u64SideBandMsg = 0;
    mi_vpe_ChannelInfo_t *pstChnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);
    MI_SYS_PixelFormat_e ePixFmt = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_S32 iOutPortIndex=-1;
    MI_SYS_BufInfo_t *pstOutputPortBufInfo= NULL;
    MI_BOOL bRet = FALSE;
    mi_vpe_OutPortInfo_t *pstPortInfo = NULL;

    for (i = 0; i < MI_VPE_MAX_PORT_NUM; i++)
    {
        MI_U32 u32PortId = i;
        pstPortInfo =  &pstChnInfo->stOutPortInfo[u32PortId];

        // TODO: Tommy: Think about output Buffer ALL NULL
        // Port enable + port no buffer
        if ((pstPortInfo->bEnable == TRUE) && (NULL != pstChnTask->astOutputPortBufInfo[i]))
        {
            s32OutBufCnt++;
            if(s32OutBufCnt > 1)
            {
                //VPE only support sideband msg in case of one output port mode
                u64SideBandMsg = 0;
                DBG_INFO("Ch: %d port %d SideBandMsg: 0x%llx.\n", u32PortId, i, u64SideBandMsg);

                break;
            }
            else
               u64SideBandMsg = pstChnTask->astOutputPortBufInfo[i]->u64SidebandMsg;
            DBG_INFO("Ch: %d port %d SideBandMsg: 0x%llx.\n", u32PortId, i, u64SideBandMsg);
            pstOutputPortBufInfo = pstChnTask->astOutputPortBufInfo[i];
            iOutPortIndex = i;
        }
    }

    switch(MI_SYS_GET_SIDEBAND_MSG_TYPE(u64SideBandMsg))
    {
       case MI_SYS_SIDEBAND_MSG_TYPE_PREFER_CROP_RECT:
            MI_SYS_GET_PREFER_CROP_MSG_DAT(u64SideBandMsg, pstChnInfo->x_zoom_in_ratio,
                                      pstChnInfo->y_zoom_in_ratio, pstChnInfo->w_zoom_in_ratio,
                                      pstChnInfo->h_zoom_in_ratio, ePixFmt);
#if 0 //DEBUG
            DBG_INFO("VPE sideband Msg: 0x%llx {crop:x%u.y%u.w%u.h%u,fmt:%u} from chn%u port%d, output buf{size:w%u.h%u, fmt:%u}\n",
                pstOutputPortBufInfo->u64SidebandMsg,
                pstChnInfo->x_zoom_in_ratio, pstChnInfo->y_zoom_in_ratio,
                pstChnInfo->w_zoom_in_ratio, pstChnInfo->h_zoom_in_ratio,
                ePixFmt, pstChnTask->u32ChnId, iOutPortIndex,
                pstOutputPortBufInfo->stFrameData.u16Width, pstOutputPortBufInfo->stFrameData.u16Height,
                pstOutputPortBufInfo->stFrameData.ePixelFormat);
#endif
            if(pstChnInfo->x_zoom_in_ratio+pstChnInfo->w_zoom_in_ratio > pstOutputPortBufInfo->stFrameData.u16Width
                   || pstChnInfo->y_zoom_in_ratio+pstChnInfo->h_zoom_in_ratio > pstOutputPortBufInfo->stFrameData.u16Height
                   ||pstChnInfo->w_zoom_in_ratio==0  || pstChnInfo->x_zoom_in_ratio+pstChnInfo->h_zoom_in_ratio==0
                   ||(ePixFmt!=E_MI_SYS_PIXEL_FRAME_YUV422_YUYV&&ePixFmt != E_MI_SYS_PIXEL_FRAME_YUV_MST_420)||
                   (ePixFmt==E_MI_SYS_PIXEL_FRAME_YUV422_YUYV && pstOutputPortBufInfo->stFrameData.ePixelFormat!=E_MI_SYS_PIXEL_FRAME_YUV422_YUYV))
            {
                DBG_ERR("Invalid vpe sideband Msg{crop:x%u.y%u.w%u.h%u,fmt:%u} from chn%u port%d, output buf{size:w%u.h%u, fmt:%u}\n",
                    pstChnInfo->x_zoom_in_ratio, pstChnInfo->y_zoom_in_ratio,
                    pstChnInfo->w_zoom_in_ratio, pstChnInfo->h_zoom_in_ratio,
                    ePixFmt, pstChnTask->u32ChnId, iOutPortIndex,
                    pstOutputPortBufInfo->stFrameData.u16Width, pstOutputPortBufInfo->stFrameData.u16Height,
                    pstOutputPortBufInfo->stFrameData.ePixelFormat);

                pstChnInfo->w_zoom_in_ratio = pstChnInfo->h_zoom_in_ratio = 0;
                break;
            }
            //standnize crop
            MI_SYS_BUG_ON(pstOutputPortBufInfo->eBufType !=E_MI_SYS_BUFDATA_FRAME
             || pstOutputPortBufInfo->stFrameData.u16Width==0 || pstOutputPortBufInfo->stFrameData.u16Height == 0);

            pstChnInfo->x_zoom_in_ratio = (pstChnInfo->x_zoom_in_ratio*VPE_ZOOM_RATION_DEN)/pstOutputPortBufInfo->stFrameData.u16Width;
            pstChnInfo->y_zoom_in_ratio= (pstChnInfo->y_zoom_in_ratio*VPE_ZOOM_RATION_DEN)/pstOutputPortBufInfo->stFrameData.u16Height;
            pstChnInfo->w_zoom_in_ratio = (pstChnInfo->w_zoom_in_ratio*VPE_ZOOM_RATION_DEN)/pstOutputPortBufInfo->stFrameData.u16Width;
            pstChnInfo->h_zoom_in_ratio = (pstChnInfo->h_zoom_in_ratio*VPE_ZOOM_RATION_DEN)/pstOutputPortBufInfo->stFrameData.u16Height;

            //force switch target format from YUYV to MST YUV420 to save BW since DISP don't need to use GE sclaing anymore..
            //Do we have better solution?
            DBG_INFO("CustomerAllocator: %d SideBand: %d.\n", pstOutputPortBufInfo->stFrameData.ePixelFormat, ePixFmt);
            pstOutputPortBufInfo->stFrameData.ePixelFormat = ePixFmt;

            MI_SYS_ACK_SIDEBAND_MSG(pstOutputPortBufInfo->u64SidebandMsg);
            pstPortInfo =  &pstChnInfo->stOutPortInfo[iOutPortIndex];
            if (pstPortInfo->u64SideBandMsg != pstOutputPortBufInfo->u64SidebandMsg)
            {
                pstPortInfo->u64SideBandMsg = pstOutputPortBufInfo->u64SidebandMsg;
                bRet = TRUE;
                DBG_INFO("SideBand Changed CustomerAllocator: PixelFormat %d SideBand Pixel: %d { %d, %d, %d, %d }.\n",
                        pstOutputPortBufInfo->stFrameData.ePixelFormat, ePixFmt, pstChnInfo->x_zoom_in_ratio, pstChnInfo->y_zoom_in_ratio, pstChnInfo->w_zoom_in_ratio, pstChnInfo->h_zoom_in_ratio);
            }
            else
            {
                DBG_INFO("CustomerAllocator: PixelFormat %d SideBand Pixel: %d.\n", pstOutputPortBufInfo->stFrameData.ePixelFormat, ePixFmt);
            }
            break;
            case MI_SYS_SIDEBAND_MSG_TYPE_NULL:
            for (i = 0; i < MI_VPE_MAX_PORT_NUM; i++)
            {
                pstPortInfo    = GET_VPE_PORT_PTR(pstChnTask->u32ChnId, i);
                pstOutputPortBufInfo = pstChnTask->astOutputPortBufInfo[i];
                if (pstOutputPortBufInfo != NULL)
                {
                    if (pstPortInfo->stPortMode.ePixelFormat != pstOutputPortBufInfo->stFrameData.ePixelFormat)
                    {
                        pstOutputPortBufInfo->stFrameData.ePixelFormat = pstPortInfo->stPortMode.ePixelFormat;
                    }

                    if (pstChnInfo->stOutPortInfo[i].u64SideBandMsg != u64SideBandMsg)
                    {
                        pstChnInfo->stOutPortInfo[i].u64SideBandMsg = u64SideBandMsg;
                        bRet = TRUE;
                        DBG_INFO("SideBand Changed CustomerAllocator: PixelFormat %d SideBand Pixel: %d { %d, %d, %d, %d }.\n",
                            pstOutputPortBufInfo->stFrameData.ePixelFormat, ePixFmt, pstChnInfo->x_zoom_in_ratio, pstChnInfo->y_zoom_in_ratio, pstChnInfo->w_zoom_in_ratio, pstChnInfo->h_zoom_in_ratio);
                    }
                    else
                    {
                        DBG_INFO("CustomerAllocator: PixelFormat %d SideBand Pixel: %d.\n", pstOutputPortBufInfo->stFrameData.ePixelFormat, ePixFmt);
                    }
                }
            }
            pstChnInfo->w_zoom_in_ratio = pstChnInfo->h_zoom_in_ratio = 0;
            break;
       default:
            pstChnInfo->w_zoom_in_ratio = pstChnInfo->h_zoom_in_ratio = 0;
            break;
    }

    return bRet;
}


static MI_BOOL _MI_VPE_CMDQTaskSlotAvaliable(MI_S32 s32CmdQTaskLimit)
{
    MI_S32 s32CurTaskCnt =0;
    MI_BOOL bRet = FALSE;
    struct list_head *pos = NULL, *n = NULL;

    if(s32CmdQTaskLimit <=0)
        return 1;

    down(&VPE_working_list_sem);
    list_for_each_safe(pos, n, &VPE_working_task_list)
    {
        s32CurTaskCnt++;
        if( s32CurTaskCnt >= s32CmdQTaskLimit)
            break;
    }
    up(&VPE_working_list_sem);

    if(s32CurTaskCnt < s32CmdQTaskLimit)
        bRet=TRUE;
    else
        bRet=FALSE;

    return bRet;

}

#include <linux/delay.h>
int cmdqstatus(void *data)
{
     mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)data;
     MHAL_CMDQ_CmdqInterface_t *cmdinf = pstDevInfo->pstCmdMloadInfo;
     MI_U32 u32Cnt =0;
     DBG_ERR("IN CMDQ THREAD \n");

     while(!kthread_should_stop())
     {
        MS_BOOL bIdleVal = FALSE;
        cmdinf->MHAL_CMDQ_IsCmdqEmptyIdle(cmdinf, &bIdleVal);
        if(bIdleVal == FALSE) //cmdQ is Running
        {
            u32Cnt++;
            if(u32Cnt == 500)
            {
                DBG_ERR("check cmdq 500 is busy\n");
                u32Cnt =0;
            }
        }
        else // cmdQ is stop
        {
            DBG_ERR("cmdq is idle status\n");
        }
        msleep(1);
     }

     return 0;
}
int VPEWorkThread(void *data)
{
    MI_U16 fence = 0;
    MI_U8 u8WaitIsrCnt =0;
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)data;
    MHAL_CMDQ_CmdqInterface_t *cmdinf = pstDevInfo->pstCmdMloadInfo;

    while (!kthread_should_stop())
    {
        mi_vpe_IteratorWorkInfo_t workinfo;
        struct list_head *pos = NULL, *n = NULL;
        workinfo.totalAddedTask = 0;
        DBG_INFO("Start get data.\n");

        if(pstDevInfo->bCameraUse)
        {
            MI_S32 s32CmdQTaskLimit =0;
            MI_U8 ret =0;
            if(pstDevInfo->eRunningMode == E_MI_VPE_RUNNING_MODE_REALTIME_MODE)
                s32CmdQTaskLimit = 2;
            else
                s32CmdQTaskLimit = -1;

            ret = wait_event_timeout(pstDevInfo->inputWaitqueue, _MI_VPE_CMDQTaskSlotAvaliable(s32CmdQTaskLimit), msecs_to_jiffies(200));
            if(ret == 0)
            {
                 if(u8WaitIsrCnt ++ < 30)
                  DBG_WRN("wait_event_timeout Fail cnt %d < 30\n", u8WaitIsrCnt);
            }
        }

        mi_sys_DevTaskIterator(pstDevInfo->hDevSysHandle, _MI_VPE_TaskIteratorCallBK, &workinfo);
        if(list_empty(&VPE_todo_task_list))
        {
            schedule_timeout_interruptible(1);
            // Tommy: Need description for this API behavior: eg: input buffer will return immediately.
            mi_sys_WaitOnInputTaskAvailable(pstDevInfo->hDevSysHandle, 50);
            continue;
        }
        DBG_INFO(" got data.\n");

        u8WaitIsrCnt = 0;

        // 3DNR Channel switch
        if(pstDevInfo->eRunningMode & SUPPORT_DNR_ROI)
            mi_vpe_impl_3DnrUpdateSwitchChannel(pstDevInfo, &VPE_active_channel_list, &VPE_active_channel_list_sem);

        list_for_each_safe(pos, n, &VPE_todo_task_list)
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            MI_BOOL bRotChanged = FALSE;
            MI_BOOL bHaveSideBandMsg = FALSE;
            int loop_cnt = 0;
            int repeatNum = 1;
            int i = 0;
            MI_BOOL bSkipIQUpdate = FALSE;
            MI_SYS_FrameData_t *pstBuffFrame = NULL;
            mi_vpe_ChannelInfo_t *pstChInfo = NULL;
            mi_vpe_InBuffInfo_t stInBuffInfo;


            mi_sys_ChnTaskInfo_t *pstLastPreprocessNotifyTask = NULL;

            memset(&stInBuffInfo, 0, sizeof(stInBuffInfo));

            pstChnTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
            pstChInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);

            pstBuffFrame = GET_VPE_BUFF_FRAME_FROM_TASK(pstChnTask);

            LOCK_CHNN(pstChInfo);

            // 1. Check roation change ??
            //only dvr mode support roation
            if(SUPPORT_ROATION & pstDevInfo->eRunningMode)
            {
                bRotChanged = _MI_VPE_CheckRotationChanged(pstChnTask);
                if (bRotChanged == TRUE)
                {
                    MHalVpeSclRotationConfig_t stRotation;
                    stRotation.enRotType = pstChInfo->eRealRotationType;
                    ExecFunc(MHalVpeSclRotationConfig(pstChInfo->pIspCtx, &stRotation));
                    DBG_INFO("Rotation Changed !!!.\n");
                }
            }

            //2.Check input size change
            bHaveSideBandMsg = _MI_VPE_ProcessTaskSideBandMsg(pstChnTask);
            if ((_MI_VPE_CheckInputChanged(pstChnTask)== TRUE) || (bRotChanged == TRUE) || (bHaveSideBandMsg == TRUE))
            {
                MHalVpeIspInputConfig_t stIspCfg;
                MHalVpeSclInputSizeConfig_t stSclCfg;

                // Set SCL input
                // Tommy: Think about repeat: input size change /sw crop
                // Need check with HAL wheather add Hal API for get NR reference Buffer.
                repeatNum = REPEAT_MAX_NUMBER;
                memset(&stSclCfg, 0, sizeof(stSclCfg));
                memset(&stIspCfg, 0, sizeof(stIspCfg));

#if defined(ROTATION_BEFORE_SACLING) && (ROTATION_BEFORE_SACLING == 1)
                if ((pstChInfo->eRotationType == E_MI_SYS_ROTATE_90)
                    || (pstChInfo->eRotationType == E_MI_SYS_ROTATE_270))
                {
                    stSclCfg.u16Height = pstChInfo->stSrcWin.u16Width;
                    stSclCfg.u16Width  = ALIGN_UP(pstChInfo->stSrcWin.u16Height, 32);

                    stIspCfg.u32Height = ALIGN_UP(pstChInfo->stSrcWin.u16Height, 32);
                    /**roation in isp input before ???**/
                    stIspCfg.u32Width  = pstChInfo->stSrcWin.u16Width;
                }
                else
#endif
                {
                    stSclCfg.u16Height = pstChInfo->stSrcWin.u16Height;
                    stSclCfg.u16Width  = pstChInfo->stSrcWin.u16Width;

                    stIspCfg.u32Height = pstChInfo->stSrcWin.u16Height;
                    stIspCfg.u32Width  = pstChInfo->stSrcWin.u16Width;
                }

                if(pstDevInfo->eRunningMode & SUPPORT_SCL)
                {
                    stSclCfg.eCompressMode = pstChInfo->eCompressMode;
                    stSclCfg.ePixelFormat  = pstChInfo->stChnnAttr.ePixFmt;
                    DBG_INFO("%s()@line %d: MHalVpeSclInputConfig: %d x %d.\n", __func__, __LINE__, stSclCfg.u16Width, stSclCfg.u16Height);
                    ExecFunc(MHalVpeSclInputConfig(pstChInfo->pSclCtx, &stSclCfg));
                }

                if(pstDevInfo->eRunningMode & SUPPORT_ISP)
                {
                    stIspCfg.eCompressMode = pstChInfo->eCompressMode;
                    stIspCfg.ePixelFormat  = pstChInfo->stChnnAttr.ePixFmt;
                    DBG_INFO("Input Changed ISPinput config %dx%d !!!.\n", stIspCfg.u32Width, stIspCfg.u32Height);
                    ExecFunc(MHalVpeIspInputConfig(pstChInfo->pIspCtx, &stIspCfg));
                }
            }

            // 2. check sideband crop msg
            //_MI_VPE_ProcessTaskSideBandMsg(pstChnTask);

            // 3. sw crop
            // Check Crop
            if ((_MI_VPE_CheckCropChanged(pstChnTask) == TRUE) || (bRotChanged == TRUE))
            {
                if(SUPPORT_SWCROP & pstDevInfo->eRunningMode)
                {
                    MI_SYS_WindowRect_t stHwCrop;
                    MI_SYS_WindowRect_t stSwCrop;
                    MHalVpeSclCropConfig_t stHwCropCfg;

                    MHalVpeIspInputConfig_t stIspCfg;
                    MHalVpeSclInputSizeConfig_t stSclCfg;

                    memset(&stHwCrop, 0, sizeof(stHwCrop));
                    memset(&stSwCrop, 0, sizeof(stSwCrop));
                    memset(&stSclCfg, 0, sizeof(stSclCfg));
                    memset(&stIspCfg, 0, sizeof(stIspCfg));
                    memset(&stHwCropCfg, 0, sizeof(stHwCropCfg));

                    _MI_VPE_CalcCropInfo(pstChInfo, &stHwCrop, &stSwCrop);
                    _MI_VPE_UpdateCropAddress(pstChnTask, &stSwCrop);

                    // Set SCL input <-- SW crop
                    repeatNum = REPEAT_MAX_NUMBER;
#if defined(ROTATION_BEFORE_SACLING) && (ROTATION_BEFORE_SACLING == 1)
                    if ((pstChInfo->eRotationType == E_MI_SYS_ROTATE_90)
                        || (pstChInfo->eRotationType == E_MI_SYS_ROTATE_270))
                    {
                        stSclCfg.u16Height = stSwCrop.u16Width;
                        stSclCfg.u16Width  = ALIGN_UP(stSwCrop.u16Height, 32);
                        
                        stIspCfg.u32Height = ALIGN_UP(stSwCrop.u16Height, 32);
                        stIspCfg.u32Width  = stSwCrop.u16Width;
                    }
                    else
#endif
                    {
                        stSclCfg.u16Height = stSwCrop.u16Height;
                        stSclCfg.u16Width  = stSwCrop.u16Width;

                        stIspCfg.u32Height = stSwCrop.u16Height;
                        stIspCfg.u32Width  = stSwCrop.u16Width;
                    }


                    if(pstDevInfo->eRunningMode & SUPPORT_ISP)
                    {
                        stIspCfg.eCompressMode = pstChInfo->eCompressMode;
                        stIspCfg.ePixelFormat  = pstChInfo->stChnnAttr.ePixFmt;
                        DBG_INFO("crop Changed ISPinput config %dx%d !!!.\n", stIspCfg.u32Width, stIspCfg.u32Height);
                        ExecFunc(MHalVpeIspInputConfig(pstChInfo->pIspCtx, &stIspCfg));
                    }
                    // Set Crop info <- HW crop
                    stHwCropCfg.bCropEn = TRUE;

#if defined(ROTATION_BEFORE_SACLING) && (ROTATION_BEFORE_SACLING == 1)
                    switch (pstChInfo->eRotationType)
                    {
                    case E_MI_SYS_ROTATE_90:
                        stHwCropCfg.stCropWin.u16X = stSwCrop.u16Height - (stHwCrop.u16Y + stHwCrop.u16Height);
                        stHwCropCfg.stCropWin.u16Y = stHwCrop.u16X;
                        stHwCropCfg.stCropWin.u16Width = stHwCrop.u16Height;
                        stHwCropCfg.stCropWin.u16Height = stHwCrop.u16Width;
                        break;
                    case E_MI_SYS_ROTATE_180:
                        stHwCropCfg.stCropWin.u16X = stSwCrop.u16Width- stHwCrop.u16X - stHwCrop.u16Width;
                        stHwCropCfg.stCropWin.u16Y = stSwCrop.u16Height - stHwCrop.u16Y - stHwCrop.u16Height;
                        stHwCropCfg.stCropWin.u16Width = stHwCrop.u16Width;
                        stHwCropCfg.stCropWin.u16Height = stHwCrop.u16Height;
                        break;
                    case E_MI_SYS_ROTATE_270:
                        stHwCropCfg.stCropWin.u16X = stHwCrop.u16Y;
                        stHwCropCfg.stCropWin.u16Y = stSwCrop.u16Width - stHwCrop.u16X - stHwCrop.u16Width;
                        stHwCropCfg.stCropWin.u16Width = stHwCrop.u16Height;
                        stHwCropCfg.stCropWin.u16Height = stHwCrop.u16Width;
                        break;
                    default:
                        stHwCropCfg.stCropWin.u16X = stHwCrop.u16X;
                        stHwCropCfg.stCropWin.u16Y = stHwCrop.u16Y;
                        stHwCropCfg.stCropWin.u16Width = stHwCrop.u16Width;
                        stHwCropCfg.stCropWin.u16Height = stHwCrop.u16Height;
                        break;
                    }
#else
                    stHwCropCfg.stCropWin.u16X = stHwCrop.u16X;
                    stHwCropCfg.stCropWin.u16Y = stHwCrop.u16Y;
                    stHwCropCfg.stCropWin.u16Width = stHwCrop.u16Width;
                    stHwCropCfg.stCropWin.u16Height = stHwCrop.u16Height;
#endif
                    if(pstDevInfo->eRunningMode & SUPPORT_SCL)
                    {
                        stSclCfg.eCompressMode = pstChInfo->eCompressMode;
                        stSclCfg.ePixelFormat  = pstChInfo->stChnnAttr.ePixFmt;
                        DBG_INFO("%s()@line %d: MHalVpeSclInputConfig: %d x %d.\n", __func__, __LINE__, stSclCfg.u16Width, stSclCfg.u16Height);
                        DBG_INFO("%s()@line %d: sclcropcfg:(%d,%d,%d,%d).\n", __func__, __LINE__, stHwCropCfg.stCropWin.u16X, stHwCropCfg.stCropWin.u16Y, stHwCropCfg.stCropWin.u16Width, stHwCropCfg.stCropWin.u16Height);
                        ExecFunc(MHalVpeSclInputConfig(pstChInfo->pSclCtx, &stSclCfg));
                        ExecFunc(MHalVpeSclCropConfig(pstChInfo->pSclCtx, &stHwCropCfg));
                    }
                }
                // Real Time Mode not support swCrop
                else if(pstDevInfo->eRunningMode & SUPPORT_SCL)
                {
                    MHalVpeSclCropConfig_t stSclCropCfg;
                    memset(&stSclCropCfg, 0, sizeof(stSclCropCfg));
                    stSclCropCfg.bCropEn = TRUE;
                    stSclCropCfg.stCropWin.u16X = ALIGN_DOWN(pstChInfo->stRealCrop.u16X, 2);
                    stSclCropCfg.stCropWin.u16Y = ALIGN_DOWN(pstChInfo->stRealCrop.u16Y, 2);
                    stSclCropCfg.stCropWin.u16Width = ALIGN_UP(pstChInfo->stRealCrop.u16Width, 16);
                    stSclCropCfg.stCropWin.u16Height = ALIGN_UP(pstChInfo->stRealCrop.u16Height,16);
                    ExecFunc(MHalVpeSclCropConfig(pstChInfo->pSclCtx, &stSclCropCfg));
                }
            }
            pstDevInfo->st3DNRUpdate.u32WaitScriptNum++;

            for (i = 0; i < repeatNum; i++)
            {
                if (repeatNum > 1)
                {
                    if (i == 0) // First need NR off and skip 3NR/ROI
                    {
                        bSkipIQUpdate = TRUE;
                    }
                    else if (i == (repeatNum -1)) // restore NR setting
                    {
                        bSkipIQUpdate = FALSE;
                    }
                }
                else
                {
                    bSkipIQUpdate = FALSE;
                }

                // TODO: user cmdq service api
                // cmdq service need add check menuload buffer size valid.
                while(!cmdinf->MHAL_CMDQ_CheckBufAvailable(cmdinf, 0x1000))
                {
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
                    interruptible_sleep_on_timeout(&vpe_isr_waitqueue, msecs_to_jiffies(VPE_WORK_THREAD_WAIT));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
                    wait_event_interruptible_timeout(vpe_isr_waitqueue, FALSE, msecs_to_jiffies(VPE_WORK_THREAD_WAIT));
#endif

                    loop_cnt++;
                    if(loop_cnt>1000)
                        MI_SYS_BUG();//engine hang
                }

                if(pstChnTask->astInputPortBufInfo[0]->eBufType == E_MI_SYS_BUFDATA_FRAME)
                {
                    stInBuffInfo.phyAddr[0] = pstBuffFrame->phyAddr[0];
                    stInBuffInfo.phyAddr[1] = pstBuffFrame->phyAddr[1];
                    stInBuffInfo.phyAddr[2] = pstBuffFrame->phyAddr[2];
                    stInBuffInfo.u32Stride[0] = pstBuffFrame->u32Stride[0];
                    stInBuffInfo.u32Stride[1] = pstBuffFrame->u32Stride[1];
                    stInBuffInfo.u32Stride[2] = pstBuffFrame->u32Stride[2];
                }


                _MI_VPE_ProcessTask(pstChnTask, pstDevInfo, &stInBuffInfo, bSkipIQUpdate);
                //the current should be the ongoing task or done task
                if(pstLastPreprocessNotifyTask != pstChnTask)
                {
                    int i;
                    MI_S32 ret=0;

                    for(i=0; i<MI_SYS_MAX_OUTPUT_PORT_CNT; i++)
                    {
                        if(!pstChnTask->astOutputPortBufInfo[i])
                            continue;

#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
                        if(pstChnTask->astOutputPortBufInfo[i] && pstChnTask->astInputPortBufInfo[i])
                        {
                            mi_sys_buf_mgr_sync_trackinfo(pstChnTask->astInputPortBufInfo[i],pstChnTask->astOutputPortBufInfo[i]);
                        }
#endif
                        ret = mi_sys_NotifyPreProcessBuf(pstChnTask->astOutputPortBufInfo[i]);
                        if(ret != MI_SUCCESS)
                            MI_SYS_BUG();

                    }
                    pstLastPreprocessNotifyTask = pstChnTask;
                }
                cmdinf->MHAL_CMDQ_WriteDummyRegCmdq(cmdinf, fence);
                cmdinf->MHAL_CMDQ_KickOffCmdq(cmdinf);

                VPE_PERF_TIME(&VPE_KICKOFF_TASK_TIME(pstChnTask));
                pstChInfo->u64GetToKickOffSumTime = VPE_KICKOFF_TASK_TIME(pstChnTask) - VPE_GOT_TASK_TIME(pstChnTask);

                pstChnTask->u32Reserved0 = fence++;
                cmdinf->MHAL_CMDQ_GetNextMloadRingBufWritePtr(cmdinf, (MS_PHYADDR *)&pstChnTask->u64Reserved0);
            }
            UNLOCK_CHNN(pstChInfo);
            DBG_INFO("Add to working litst.\n");

            list_del(&pstChnTask->listChnTask);
            down(&VPE_working_list_sem);
            list_add_tail(&pstChnTask->listChnTask, &VPE_working_task_list);
            up(&VPE_working_list_sem);

            DBG_INFO("Add to working litst Done.\n");
        }
        DBG_INFO("Todo task list empty.\n");
    }
    return 0;
}


static MI_S32 _MI_VPE_OnBindChnnInputputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    if ((pstChnCurryPort->eModId == E_MI_MODULE_ID_VPE)    // Check Callback information ok ?
        && MI_VPE_CHECK_CHNN_SUPPORTED(pstChnCurryPort->u32ChnId) // Check support Channel
#if 0
        && ((pstChnPeerPort->eModId == E_MI_MODULE_ID_VIF) // Check supported input module ?
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_DIVP)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VPE))
#endif
        )
    {
        pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stPeerInputPortInfo = *pstChnPeerPort;
        return MI_SUCCESS;
    }
    else
    {
        return MI_SYS_ERR_BUSY;
    }
}

static MI_S32 _MI_VPE_OnUnBindChnnInputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    if ((pstChnCurryPort->eModId == E_MI_MODULE_ID_VPE)         // Check Callback information ok ?
        && MI_VPE_CHECK_CHNN_SUPPORTED(pstChnCurryPort->u32ChnId)   // Check support Channel
#if 0
    && ((pstChnPeerPort->eModId == E_MI_MODULE_ID_VIF)      // Check supported output module
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_DIVP)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VPE))
#endif
        )
    {
        // Need check Chnn Busy ???
        // if(list_empty(&VPE_todo_task_list))
        // ... ...
        memset(&pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stPeerInputPortInfo, 0, sizeof(pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stPeerInputPortInfo));
        return MI_SUCCESS;
    }
    else
    {
        return MI_SYS_ERR_BUSY;
    }
}

static MI_S32 _MI_VPE_OnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    if ((pstChnCurryPort->eModId == E_MI_MODULE_ID_VPE)         // Check Callback information ok ?
        && MI_VPE_CHECK_CHNN_SUPPORTED(pstChnCurryPort->u32ChnId)   // Check support Channel
        && MI_VPE_CHECK_PORT_SUPPORTED(pstChnCurryPort->u32PortId)  // Check support Port Id
#if 0
    && ((pstChnPeerPort->eModId == E_MI_MODULE_ID_DIVP)     // supported input module
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VENC)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VDISP)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_DISP))
#endif
        )
    {
        pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stOutPortInfo[pstChnCurryPort->u32PortId].stPeerOutputPortInfo = *pstChnPeerPort;
        return MI_SUCCESS;
    }
    else
    {
        return MI_SYS_ERR_BUSY;
    }
}

static MI_S32 _MI_VPE_OnUnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    if ((pstChnCurryPort->eModId == E_MI_MODULE_ID_VPE)         // Check Callback information ok ?
        && MI_VPE_CHECK_CHNN_SUPPORTED(pstChnCurryPort->u32ChnId)   // Check support Channel
        && MI_VPE_CHECK_PORT_SUPPORTED(pstChnCurryPort->u32PortId)  // Check support Port Id
#if 0 // No use
    && ((pstChnPeerPort->eModId == E_MI_MODULE_ID_DIVP)     // supported input module
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VENC)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_VDISP)
            || (pstChnPeerPort->eModId == E_MI_MODULE_ID_DISP))
#endif
        )
    {
        memset (&pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stOutPortInfo[pstChnCurryPort->u32PortId].stPeerOutputPortInfo, 0,
            sizeof(pstDevInfo->stChnnInfo[pstChnCurryPort->u32ChnId].stOutPortInfo[pstChnCurryPort->u32PortId].stPeerOutputPortInfo));
        return MI_SUCCESS;
    }
    else
    {
        return MI_SYS_ERR_BUSY;
    }

    // Check Busy ??
    // if(list_empty(&VPE_working_task_list))
 }
#if 0
void dump_tread( int *pdata)
{

     while(!kthread_should_stop())
     {
        struct task_struct *g, *p;
        msleep(60000);//30 secs
          rcu_read_lock();
         do_each_thread(g, p)
         {
            show_stack(p,NULL);
         } while_each_thread(g, p);
         rcu_read_unlock();
    }

}
#endif

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
static MI_S32 _MI_VPE_ProcOnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId,void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    mi_vpe_DevInfo_t     *pstDev = GET_VPE_DEV_PTR();

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    if (u32DevId > 0)
    {
        return s32Ret;
    }
    handle.OnPrintOut(handle, "\r\n-------------------------- start dump DEV info --------------------------------\n");
    handle.OnPrintOut(handle,"%7s%13s%12s%12s%9s%12s%13s%12s%13s%12s\n","DevID","3DNR_Status", "ROI_Status", "SupportIRQ", "IRQ_num", "IRQ_Enable","DropStatus0","DropFence0","DropStatus1","DropFence1");

    handle.OnPrintOut(handle,"%7d",u32DevId);
    handle.OnPrintOut(handle, "%3d(chnid %2d)", pstDev->st3DNRUpdate.eStatus, pstDev->st3DNRUpdate.VpeCh);
    handle.OnPrintOut(handle, "%12d", pstDev->eRoiStatus);
    handle.OnPrintOut(handle, "%12d", pstDev->bSupportIrq);
    handle.OnPrintOut(handle, "%9d",  pstDev->uVpeIrqNum);
    handle.OnPrintOut(handle, "%12d", pstDev->bEnbaleIrq);
    handle.OnPrintOut(handle, "%13d", pstDev->stDropFrameFence[0].bStatus);
    handle.OnPrintOut(handle, "%12d", pstDev->stDropFrameFence[0].u16DropFence);
    handle.OnPrintOut(handle, "%13d", pstDev->stDropFrameFence[1].bStatus);
    handle.OnPrintOut(handle, "%12d\n", pstDev->stDropFrameFence[1].u16DropFence);

    handle.OnPrintOut(handle,"%10s%12s\n","eRunMode", "EnIrqMode");
    handle.OnPrintOut(handle,"%10d",pstDev->eRunningMode);
    handle.OnPrintOut(handle,"%12x\n",pstDev->eIrqEnableMode);
    handle.OnPrintOut(handle, "-------------------------- End dump dev %d info --------------------------------\n", u32DevId);

    s32Ret = MI_VPE_OK;
    return s32Ret;
}

static MI_S32 _MI_VPE_ProcOnDumpChannelAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId,void *pUsrData)
{
    MI_U32 u32ChnId = 0;//TODO: while all channel

    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    mi_vpe_ChannelInfo_t *pstChnInfo = NULL;
    MI_U64 u64MeanTime = 0;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    handle.OnPrintOut(handle, "\r\n-------------------------- start dump CHN info --------------------------------\n");
    handle.OnPrintOut(handle,"%7s%8s%8s%8s%7s%7s%7s%7s%9s%15s%14s\n", "ChnId", "status", "InputW","InputH", "CropX","CropY","CropW","CropH", "Roation", "TaskTime_mean", "TaskTime_max");
    for(u32ChnId = 0; u32ChnId < MI_VPE_MAX_CHANNEL_NUM; u32ChnId++)
    {
        pstChnInfo = GET_VPE_CHNN_PTR(u32ChnId);
        if (pstChnInfo->bCreated == TRUE)
        {
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%8d", pstChnInfo->eStatus);
            handle.OnPrintOut(handle,"%8d", pstChnInfo->stSrcWin.u16Width);
            handle.OnPrintOut(handle,"%8d", pstChnInfo->stSrcWin.u16Height);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stCropWin.u16X);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stCropWin.u16Y);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stCropWin.u16Width);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stCropWin.u16Height);
            handle.OnPrintOut(handle,"%9d", pstChnInfo->eRotationType);

            if(pstChnInfo->u64ReleaseBufcnt != 0)
                u64MeanTime = div64_u64(pstChnInfo->u64GetToReleaseSumTime, pstChnInfo->u64ReleaseBufcnt);

            handle.OnPrintOut(handle, "%13lluus", u64MeanTime);
            handle.OnPrintOut(handle, "%12lluus\n", pstChnInfo->u64GetToReleaseMaxTime);
            //pstChnInfo->u64GetToReleaseMaxTime = 0;
            //pstChnInfo->u64GetToReleaseSumTime = 0;
            //pstChnInfo->u64ReleaseBufcnt = 0;

        }
    }

    handle.OnPrintOut(handle,"%7s%10s%15s%13s%10s%10s%6s%11s%12s\n","ChnId","InBufCnt","InBufTodoCnt","InBufStride","GetToK.O", "K.OToRel", "Atom", "sclInMode","sclOutMode");
    for(u32ChnId = 0; u32ChnId < MI_VPE_MAX_CHANNEL_NUM; u32ChnId++)
    {
        pstChnInfo = GET_VPE_CHNN_PTR(u32ChnId);
        if (pstChnInfo->bCreated == TRUE)
        {
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%10llu", pstChnInfo->u64GetInputBufferCnt);
            handle.OnPrintOut(handle,"%15llu", pstChnInfo->u64GetInputBufferTodoCnt);
            handle.OnPrintOut(handle,"%13llu", pstChnInfo->u64InBuffStride);
            handle.OnPrintOut(handle,"%10llu", pstChnInfo->u64GetToKickOffSumTime);
            handle.OnPrintOut(handle,"%10u", pstChnInfo->u32KOToReleaseMeanTime);
            handle.OnPrintOut(handle,"%6d", atomic_read(&pstChnInfo->stAtomTask));

            handle.OnPrintOut(handle, "%11x", pstChnInfo->eSclInPutMode);
            handle.OnPrintOut(handle, "%12x\n", pstChnInfo->eSclOutputMode);
            //pstChnInfo->u64GetInputBufferCnt = 0;
        }
    }

    handle.OnPrintOut(handle,"%7s%5s%7s%5s%11s%5s%8s%8s%8s%8s%10s%9s%6s%6s%6s%11s\n", "ChnId", "bNr","bEdge","bEs","bContrast","bUV","cSfStr", "cTfStr","ySfStr", "yTfStr","MotionTh","StillTh",
        "MWer", "OWei", "SWei", "Constrast");
    for(u32ChnId = 0; u32ChnId < MI_VPE_MAX_CHANNEL_NUM; u32ChnId++)
    {
        pstChnInfo = GET_VPE_CHNN_PTR(u32ChnId);
        if (pstChnInfo->bCreated == TRUE)
        {
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%5d", pstChnInfo->stChnnAttr.bNrEn);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stChnnAttr.bEdgeEn);
            handle.OnPrintOut(handle,"%5d", pstChnInfo->stChnnAttr.bEsEn);
            handle.OnPrintOut(handle,"%11d", pstChnInfo->stChnnAttr.bContrastEn);
            handle.OnPrintOut(handle,"%5d", pstChnInfo->stChnnAttr.bUvInvert);

            handle.OnPrintOut(handle,"%8d", pstChnInfo->stChnnPara.u8NrcSfStr);
            handle.OnPrintOut(handle,"%8d", pstChnInfo->stChnnPara.u8NrcTfStr);
            handle.OnPrintOut(handle,"%8d", pstChnInfo->stChnnPara.u8NrySfStr);
            handle.OnPrintOut(handle,"%8d", pstChnInfo->stChnnPara.u8NryTfStr);
            handle.OnPrintOut(handle,"%10d", pstChnInfo->stChnnPara.u8NryBlendMotionTh);
            handle.OnPrintOut(handle,"%9d", pstChnInfo->stChnnPara.u8NryBlendStillTh);
            handle.OnPrintOut(handle,"%6d", pstChnInfo->stChnnPara.u8NryBlendMotionWei);
            handle.OnPrintOut(handle,"%6d", pstChnInfo->stChnnPara.u8NryBlendOtherWei);
            handle.OnPrintOut(handle,"%6d", pstChnInfo->stChnnPara.u8NryBlendStillWei);
            handle.OnPrintOut(handle,"%11d\n", pstChnInfo->stChnnPara.u8Contrast);
        }
    }

    handle.OnPrintOut(handle,"%7s%7s%7s%7s%7s%7s%7s\n", "ChnId", "Edge0","Edge1","Edge2","Edge3","Edge4","Edge5");
     for(u32ChnId = 0; u32ChnId < MI_VPE_MAX_CHANNEL_NUM; u32ChnId++)
    {
        pstChnInfo = GET_VPE_CHNN_PTR(u32ChnId);
        if (pstChnInfo->bCreated == TRUE)
        {
            handle.OnPrintOut(handle,"%7d", u32ChnId);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stChnnPara.u8EdgeGain[0]);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stChnnPara.u8EdgeGain[1]);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stChnnPara.u8EdgeGain[2]);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stChnnPara.u8EdgeGain[3]);
            handle.OnPrintOut(handle,"%7d", pstChnInfo->stChnnPara.u8EdgeGain[4]);
            handle.OnPrintOut(handle,"%7d\n", pstChnInfo->stChnnPara.u8EdgeGain[5]);
        }
    }

    handle.OnPrintOut(handle, "-------------------------- End dump CHN info -------------------------------\n");

    s32Ret = MI_VPE_OK;
    return s32Ret;
}


static MI_S32 _MI_VPE_ProcOnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_S32 s32Ret = MI_VPE_OK;

    // Add input port implement here
    return s32Ret;
}

static MI_S32 _MI_VPE_ProcOnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    MI_U32 u32ChnId = 0, u32OutPortId = 0;//TODO: while all channel
    mi_vpe_OutPortInfo_t *pstPortInfo = NULL;
    mi_vpe_ChannelInfo_t *pstChnInfo = NULL;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    handle.OnPrintOut(handle, "\r\n-------------------------- start dump OUTPUT PORT info -----------------------\n");
    handle.OnPrintOut(handle, "%7s%8s%8s%7s%9s%9s%8s%10s%8s%9s%11s%5s\n","ChnId", "PortId", "Enable", "Pixel", "OutputW", "OutputH","Stride", "Compress","GetCnt","FailCnt","FinishCnt","fps");
    for(u32ChnId = 0; u32ChnId < MI_VPE_MAX_CHANNEL_NUM; u32ChnId++)
    {
        for(u32OutPortId = 0; u32OutPortId < MI_VPE_MAX_PORT_NUM; u32OutPortId++)
        {
            pstChnInfo = GET_VPE_CHNN_PTR(u32ChnId);
            if (pstChnInfo->bCreated == TRUE)
            {
                pstPortInfo = GET_VPE_PORT_PTR(u32ChnId, u32OutPortId);
                handle.OnPrintOut(handle, "%7d", u32ChnId);
                handle.OnPrintOut(handle, "%8d", u32OutPortId);
                handle.OnPrintOut(handle, "%8d", pstPortInfo->bEnable);
                handle.OnPrintOut(handle, "%7d", pstPortInfo->stPortMode.ePixelFormat);
                handle.OnPrintOut(handle, "%9d", pstPortInfo->stPortMode.u16Width);
                handle.OnPrintOut(handle, "%9d", pstPortInfo->stPortMode.u16Height);
                handle.OnPrintOut(handle, "%8d", pstPortInfo->u32OutbufStride);
                handle.OnPrintOut(handle, "%10d", pstPortInfo->stPortMode.eCompressMode);
                handle.OnPrintOut(handle, "%8llu", pstPortInfo->u64GetOutputBufferCnt);
                handle.OnPrintOut(handle, "%9llu", pstPortInfo->u64GetOutputBuffFailCnt);
                handle.OnPrintOut(handle, "%11llu", pstPortInfo->u64FinishOutputBufferCnt);
                handle.OnPrintOut(handle, "%5d\n", pstPortInfo->u8PortFps);
                //pstPortInfo->u64GetOutputBufferCnt = 0;
                //pstPortInfo->u64FinishOutputBufferCnt = 0;
            }
        }
    }
    handle.OnPrintOut(handle, "-------------------------- End dump OUTPUT PORT info -------------------------\n");

    s32Ret = MI_VPE_OK;

    return s32Ret;
}


static MI_S32 _MI_VPE_ProcOnHelp(MI_SYS_DEBUG_HANDLE_t  handle,MI_U32  u32DevId,void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    handle.OnPrintOut(handle, "disable_cmdq [ON, OFF];              Enable/Disable CMDQ.\n");
    handle.OnPrintOut(handle, "disable_irq  [ON, OFF];              Enable/Disable IRQ.\n");

    s32Ret = MI_VPE_OK;
    return s32Ret;
}
static MI_S32 _MI_VPE_ProcEchoDisableCmdq(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_VPE_OK;
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 1)
    {
        if (strcmp(argv[1], "ON") == 0)
        {
            pstDevInfo->bProcDisableCmdq = TRUE;
        }
        else if (strcmp(argv[1], "OFF") == 0)
        {
            pstDevInfo->bProcDisableCmdq = FALSE;
        }
        else
        {
            handle.OnPrintOut(handle, "Unsupport command: %s.\n", argv[0]);
            s32Ret = MI_ERR_VPE_NOT_SUPPORT;
        }
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "disable_cmdq [ON, OFF];              Enable/Disable CMDQ.\n");
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    //handle.OnPrintOut(handle, "bProcDisableCmdq: %d.\n", pstDevInfo->bProcDisableCmdq);
    return s32Ret;
}

static MI_S32 _MI_VPE_ProcEchoDisableIrq(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_VPE_OK;
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 1)
    {
        if (strcmp(argv[1], "ON") == 0)
        {
            pstDevInfo->bProcDisableIrq = TRUE;
        }
        else if (strcmp(argv[1], "OFF") == 0)
        {
            pstDevInfo->bProcDisableIrq = FALSE;
        }
        else
        {
            handle.OnPrintOut(handle, "Unsupport command: %s.\n", argv[0]);
            s32Ret = MI_ERR_VPE_NOT_SUPPORT;
        }
    }
    else
    {

        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "disable_irq  [ON, OFF];              Enable/Disable IRQ.\n");
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    //handle.OnPrintOut(handle, "bProcDisableIrq: %d.\n", pstDevInfo->bProcDisableIrq);
    return s32Ret;
}

MI_S32 _MI_VPE_DebugSetChnRoation(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
        MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
       MI_VPE_CHANNEL VpeChn;
       MI_U8 u8Roation =0;

       mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
       mi_vpe_ChannelInfo_t *pstChnInfo = NULL;
       MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
       MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

       if (argc > 2)
       {
           VpeChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
           u8Roation = (MI_U8)simple_strtoul(argv[2], NULL, 10);
           if(MI_VPE_CHECK_CHNN_SUPPORTED(VpeChn) && MI_VPE_CHECK_CHNN_CREATED(VpeChn))
           {
               pstChnInfo = GET_VPE_CHNN_PTR(VpeChn);
           }
           else
           {
               handle.OnPrintOut(handle, "channelID[%d] is not Supported or not Create.\n", VpeChn);
               return MI_ERR_VPE_NOT_SUPPORT;
           }

           if (u8Roation == E_MI_SYS_ROTATE_180 || u8Roation > E_MI_SYS_ROTATE_270)
           {
               handle.OnPrintOut(handle, "argv2 paramer err %d. 0:0, 1:90, 2:180(no support), 3:270\n", u8Roation);
               return MI_ERR_VPE_NOT_SUPPORT;
           }
           else
           {
               pstChnInfo->eRotationType = (MI_SYS_Rotate_e)u8Roation;
           }

           handle.OnPrintOut(handle,"ChanID %d roation %d \r\n",VpeChn, pstChnInfo->eRotationType);
           s32Ret = MI_SUCCESS;
       }
       else
       {
           handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
           handle.OnPrintOut(handle, "VPE set roation, chnl; 0:0, 1:90, 2:180(no support), 3:270\n");
           s32Ret = MI_ERR_VPE_NOT_SUPPORT;
       }

       return s32Ret;
}

MI_S32 _MI_VPE_DebugSetDbgLevel(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    MI_BOOL bMhal_Debug = FALSE;
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 1)
    {
        if (strcmp(argv[1], "ON") == 0)
        {
            bMhal_Debug =0xff;
            ExecFunc(MHalVpeIspDbgLevel(&bMhal_Debug));
            MHalVpeSclDbgLevel(&bMhal_Debug);
        }
        else if (strcmp(argv[1], "OFF") == 0)
        {
            bMhal_Debug =FALSE;
            ExecFunc(MHalVpeIspDbgLevel(&bMhal_Debug));
            ExecFunc(MHalVpeSclDbgLevel(&bMhal_Debug));
        }
        handle.OnPrintOut(handle,"MHAL dbg level %d \r\n",bMhal_Debug);
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "VPE set mhal dbg level ON/OFF\n");
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    return s32Ret;
}

MI_S32 _MI_VPE_DebugCheckCmdqStatus(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 1)
    {
        if (strcmp(argv[1], "ON") == 0)
        {

            pstDevInfo->pstCmdqbusy = kthread_create(cmdqstatus, pstDevInfo, "cmdqThread");
            if (IS_ERR(pstDevInfo->pstCmdqbusy ))
            {
                DBG_ERR("Fail to create thread cmdqThread.\n");
                kthread_stop(pstDevInfo->pstCmdqbusy);
            }
            else
            {
                DBG_ERR("CREATE CMDQ TEREAD SUCESS\n");
            }
            wake_up_process(pstDevInfo->pstCmdqbusy);
        }
        else if (strcmp(argv[1], "OFF") == 0)
        {
            kthread_stop(pstDevInfo->pstCmdqbusy);
        }
        handle.OnPrintOut(handle,"cmdq create thread %s \r\n",argv[1]);
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "VPE check cmdq status \n");
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    return s32Ret;
}


MI_S32 _MI_VPE_CheckFramePts(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    MI_VPE_CHANNEL VpeChn;

    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    mi_vpe_ChannelInfo_t *pstChnInfo = NULL;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 2)
    {
        VpeChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(MI_VPE_CHECK_CHNN_SUPPORTED(VpeChn) && MI_VPE_CHECK_CHNN_CREATED(VpeChn))
        {
            pstChnInfo = GET_VPE_CHNN_PTR(VpeChn);
        }
        else
        {
            handle.OnPrintOut(handle, "channelID[%d] is not Supported or not Create.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }

        if (strcmp(argv[2], "ON") == 0 && pstChnInfo->bCheckFramePts == FALSE)
        {
            pstChnInfo->bCheckFramePts = TRUE;
        }
        else if (strcmp(argv[2], "OFF") == 0 && pstChnInfo->bCheckFramePts == TRUE)
        {
            pstChnInfo->bCheckFramePts = FALSE;
        }
        else
        {
            handle.OnPrintOut(handle, "argv[2] is error[ON, OFF].\n");
            return MI_ERR_VPE_NOT_SUPPORT;
        }

        handle.OnPrintOut(handle,"ChanID %d bCheckFramePts %d \r\n",VpeChn, pstChnInfo->bCheckFramePts);
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "VPE CHECK FRAMEID CHANNEL ID; [ON, OFF];.\n");
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    return s32Ret;
}

MI_S32 _MI_VPE_DumpTaskFile(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    MI_VPE_CHANNEL VpeChn;
    MI_U16 u16DumpTaskFileNum = 0;
    mi_vpe_ChannelInfo_t *pstChnInfo = NULL;

    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if (argc > 2)
    {
        VpeChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(MI_VPE_CHECK_CHNN_SUPPORTED(VpeChn) && MI_VPE_CHECK_CHNN_CREATED(VpeChn))
        {
            pstChnInfo = GET_VPE_CHNN_PTR(VpeChn);
        }
        else
        {
            handle.OnPrintOut(handle, "channelID[%d] is not Supported or not Create.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }

        u16DumpTaskFileNum = (MI_U16)simple_strtoul(argv[2], NULL, 10);
        if ( u16DumpTaskFileNum > 0 && u16DumpTaskFileNum < 0xffff)
        {
            pstChnInfo->u16DumpTaskFileNum = u16DumpTaskFileNum;
        }
        else
        {
            handle.OnPrintOut(handle, "argv[2] is %d error[0~65535].\n", u16DumpTaskFileNum);
            return MI_ERR_VPE_NOT_SUPPORT;
        }

        handle.OnPrintOut(handle,"ChanID %d u16DumpTaskFileNum %d \r\n", VpeChn, u16DumpTaskFileNum);
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        handle.OnPrintOut(handle, "VPE DumpTaskFile chnid  num;.\n");
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    return s32Ret;
}

MI_S32 _MI_VPE_SetNrMainParam(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    MI_VPE_CHANNEL VpeChn;
    MI_VPE_ChannelPara_t stChannelPara_t;

    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    if (argc > 5)
    {
        VpeChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(MI_VPE_CHECK_CHNN_SUPPORTED(VpeChn) && MI_VPE_CHECK_CHNN_CREATED(VpeChn))
        {
            if (MI_VPE_IMPL_GetChannelParam(VpeChn, &stChannelPara_t))
            {
                handle.OnPrintOut(handle, "channelID[%d] GetChannelParam Error.\n", VpeChn);
                return MI_ERR_VPE_NOT_SUPPORT;
            }
        }
        else
        {
            handle.OnPrintOut(handle, "channelID[%d] is not Supported or not Create.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }

        stChannelPara_t.u8NrcSfStr = (MI_U8)simple_strtoul(argv[2], NULL, 10);
        stChannelPara_t.u8NrcTfStr = (MI_U8)simple_strtoul(argv[3], NULL, 10);
        stChannelPara_t.u8NrySfStr = (MI_U8)simple_strtoul(argv[4], NULL, 10);
        stChannelPara_t.u8NryTfStr = (MI_U8)simple_strtoul(argv[5], NULL, 10);

        handle.OnPrintOut(handle,"ChanID %d u8NrcSfStr(%d) u8NrcTfStr(%d) u8NrySfStr(%d) u8NryTfStr(%d)\r\n", VpeChn,
            stChannelPara_t.u8NrcSfStr, stChannelPara_t.u8NrcTfStr, stChannelPara_t.u8NrySfStr,
            stChannelPara_t.u8NryTfStr);
        if (MI_VPE_IMPL_SetChannelParam(VpeChn, &stChannelPara_t))
        {
            handle.OnPrintOut(handle, "channelID[%d] SetChannelParam Error.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    return s32Ret;
}

MI_S32 _MI_VPE_SetNrSubParam(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    MI_VPE_CHANNEL VpeChn;
    MI_VPE_ChannelPara_t stChannelPara_t;

    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    if (argc > 6)
    {
        VpeChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(MI_VPE_CHECK_CHNN_SUPPORTED(VpeChn) && MI_VPE_CHECK_CHNN_CREATED(VpeChn))
        {
            if (MI_VPE_IMPL_GetChannelParam(VpeChn, &stChannelPara_t))
            {
                handle.OnPrintOut(handle, "channelID[%d] GetChannelParam Error.\n", VpeChn);
                return MI_ERR_VPE_NOT_SUPPORT;
            }
        }
        else
        {
            handle.OnPrintOut(handle, "channelID[%d] is not Supported or not Create.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }

        stChannelPara_t.u8NryBlendMotionTh = (MI_U8)simple_strtoul(argv[2], NULL, 10);
        stChannelPara_t.u8NryBlendStillTh = (MI_U8)simple_strtoul(argv[3], NULL, 10);
        stChannelPara_t.u8NryBlendMotionWei = (MI_U8)simple_strtoul(argv[4], NULL, 10);
        stChannelPara_t.u8NryBlendOtherWei = (MI_U8)simple_strtoul(argv[5], NULL, 10);
        stChannelPara_t.u8NryBlendStillWei = (MI_U8)simple_strtoul(argv[6], NULL, 10);

        handle.OnPrintOut(handle,"ChanID %d u8NryBlendMotionTh(%d) u8NryBlendStillTh(%d) u8NryBlendMotionWei(%d) u8NryBlendOtherWei(%d) u8NryBlendStillWei(%d)\r\n", VpeChn,
            stChannelPara_t.u8NryBlendMotionTh, stChannelPara_t.u8NryBlendStillTh, stChannelPara_t.u8NryBlendMotionWei,
            stChannelPara_t.u8NryBlendOtherWei, stChannelPara_t.u8NryBlendStillWei);
        if (MI_VPE_IMPL_SetChannelParam(VpeChn, &stChannelPara_t))
        {
            handle.OnPrintOut(handle, "channelID[%d] SetChannelParam Error.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    return s32Ret;
}

MI_S32 _MI_VPE_SetNrEdgeParam(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    MI_VPE_CHANNEL VpeChn;
    MI_VPE_ChannelPara_t stChannelPara_t;

    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    if (argc > 7)
    {
        VpeChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(MI_VPE_CHECK_CHNN_SUPPORTED(VpeChn) && MI_VPE_CHECK_CHNN_CREATED(VpeChn))
        {
            if (MI_VPE_IMPL_GetChannelParam(VpeChn, &stChannelPara_t))
            {
                handle.OnPrintOut(handle, "channelID[%d] GetChannelParam Error.\n", VpeChn);
                return MI_ERR_VPE_NOT_SUPPORT;
            }
        }
        else
        {
            handle.OnPrintOut(handle, "channelID[%d] is not Supported or not Create.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }

        stChannelPara_t.u8EdgeGain[0] = (MI_U8)simple_strtoul(argv[2], NULL, 10);
        stChannelPara_t.u8EdgeGain[1] = (MI_U8)simple_strtoul(argv[3], NULL, 10);
        stChannelPara_t.u8EdgeGain[2] = (MI_U8)simple_strtoul(argv[4], NULL, 10);
        stChannelPara_t.u8EdgeGain[3] = (MI_U8)simple_strtoul(argv[5], NULL, 10);
        stChannelPara_t.u8EdgeGain[4] = (MI_U8)simple_strtoul(argv[6], NULL, 10);
        stChannelPara_t.u8EdgeGain[5] = (MI_U8)simple_strtoul(argv[7], NULL, 10);

        if (MI_VPE_IMPL_SetChannelParam(VpeChn, &stChannelPara_t))
        {
            handle.OnPrintOut(handle, "channelID[%d] SetChannelParam Error.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    return s32Ret;
}

MI_S32 _MI_VPE_SetNrConstrastParam(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    MI_VPE_CHANNEL VpeChn;
    MI_VPE_ChannelPara_t stChannelPara_t;

    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    MI_SYS_BUG_ON(pstDevInfo->u32MagicNumber != __MI_VPE_DEV_MAGIC__);
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    if (argc > 2)
    {
        VpeChn = (MI_U8)simple_strtoul(argv[1], NULL, 10);
        if(MI_VPE_CHECK_CHNN_SUPPORTED(VpeChn) && MI_VPE_CHECK_CHNN_CREATED(VpeChn))
        {
            if (MI_VPE_IMPL_GetChannelParam(VpeChn, &stChannelPara_t))
            {
                handle.OnPrintOut(handle, "channelID[%d] GetChannelParam Error.\n", VpeChn);
                return MI_ERR_VPE_NOT_SUPPORT;
            }
        }
        else
        {
            handle.OnPrintOut(handle, "channelID[%d] is not Supported or not Create.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }

        stChannelPara_t.u8Contrast = (MI_U8)simple_strtoul(argv[2], NULL, 10);

        if (MI_VPE_IMPL_SetChannelParam(VpeChn, &stChannelPara_t))
        {
            handle.OnPrintOut(handle, "channelID[%d] SetChannelParam Error.\n", VpeChn);
            return MI_ERR_VPE_NOT_SUPPORT;
        }
        s32Ret = MI_SUCCESS;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        s32Ret = MI_ERR_VPE_NOT_SUPPORT;
    }

    return s32Ret;
}

#endif

MI_S32 MI_VPE_IMPL_Init(void)
{
    MI_S32 s32Ret = MI_ERR_VPE_NULL_PTR;
    mi_sys_ModuleDevBindOps_t stVPEPOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    MHAL_CMDQ_BufDescript_t stCmdqBufDesp;
    MHalAllocPhyMem_t stAlloc;

#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
#endif

    mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();
    struct sched_param param;
    memset(&param, 0, sizeof(param));

    if (pstDevInfo->bInited == TRUE)
    {
        DBG_ERR("already inited.\n");
        return MI_ERR_VPE_EXIST;
    }
    DBG_INFO("Start Init.\n");
    //Get VPE Cap
    MHalVpeGetCaps(&pstDevInfo->stCaps);

    //Get VPE SCL Port Mode
    if(PortNum == USBCAM_PORTNUM)
    {
        pstDevInfo->eSclPortMode = E_MI_VPE_SCL_3_PORTMODE;
    }
    else
    {
        pstDevInfo->eSclPortMode = E_MI_VPE_SCL_4_PORTMODE;
    }

    // VPE register to mi_sys
    memset(&stVPEPOps, 0, sizeof(stVPEPOps));
    stVPEPOps.OnBindInputPort    = _MI_VPE_OnBindChnnInputputCallback;
    stVPEPOps.OnUnBindInputPort  = _MI_VPE_OnUnBindChnnInputCallback;
    stVPEPOps.OnBindOutputPort   = _MI_VPE_OnBindChnnOutputCallback;
    stVPEPOps.OnUnBindOutputPort = _MI_VPE_OnUnBindChnnOutputCallback;
    stVPEPOps.OnOutputPortBufRelease = NULL;
    memset(&stModInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId      = E_MI_MODULE_ID_VPE;
    stModInfo.u32DevId         = 0;
    stModInfo.u32DevChnNum     = MI_VPE_MAX_CHANNEL_NUM;
    stModInfo.u32InputPortNum  = MI_VPE_MAX_INPUTPORT_NUM;
    stModInfo.u32OutputPortNum = MI_VPE_MAX_PORT_NUM;

#ifdef MI_SYS_PROC_FS_DEBUG

    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
#if(MI_VPE_PROCFS_DEBUG == 1)
    pstModuleProcfsOps.OnDumpDevAttr = _MI_VPE_ProcOnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = _MI_VPE_ProcOnDumpChannelAttr;
    pstModuleProcfsOps.OnDumpInputPortAttr = _MI_VPE_ProcOnDumpInputPortAttr;
    pstModuleProcfsOps.OnDumpOutPortAttr = _MI_VPE_ProcOnDumpOutPortAttr;
    pstModuleProcfsOps.OnHelp = _MI_VPE_ProcOnHelp;
#else
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif

#endif

    pstDevInfo->hDevSysHandle   = mi_sys_RegisterDev(&stModInfo, &stVPEPOps, pstDevInfo
                                                   #ifdef MI_SYS_PROC_FS_DEBUG
                                                   , &pstModuleProcfsOps
                                                   ,MI_COMMON_GetSelfDir
                                                   #endif
                                                   );

    if (pstDevInfo->hDevSysHandle == NULL)
    {
        DBG_ERR("Fail to register dev.\n");
    }
    DBG_INFO("success to register dev.\n");

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
    mi_sys_RegistCommand("disable_cmdq", 1, _MI_VPE_ProcEchoDisableCmdq, pstDevInfo->hDevSysHandle);
    mi_sys_RegistCommand("disable_irq",  1, _MI_VPE_ProcEchoDisableIrq, pstDevInfo->hDevSysHandle);
    mi_sys_RegistCommand("checkframepts",2, _MI_VPE_CheckFramePts, pstDevInfo->hDevSysHandle);
    mi_sys_RegistCommand("DumpTaskFile",2, _MI_VPE_DumpTaskFile, pstDevInfo->hDevSysHandle);
	mi_sys_RegistCommand("setnrmain",5, _MI_VPE_SetNrMainParam, pstDevInfo->hDevSysHandle);
    mi_sys_RegistCommand("setnrsub",6, _MI_VPE_SetNrSubParam, pstDevInfo->hDevSysHandle);
    mi_sys_RegistCommand("setnredge",7, _MI_VPE_SetNrEdgeParam, pstDevInfo->hDevSysHandle);
    mi_sys_RegistCommand("setnrconstrast",2, _MI_VPE_SetNrConstrastParam, pstDevInfo->hDevSysHandle);

    mi_sys_RegistCommand("setroation",2, _MI_VPE_DebugSetChnRoation, pstDevInfo->hDevSysHandle);
    mi_sys_RegistCommand("sethaldbg",1, _MI_VPE_DebugSetDbgLevel, pstDevInfo->hDevSysHandle);
    mi_sys_RegistCommand("checkcmdq",1, _MI_VPE_DebugCheckCmdqStatus, pstDevInfo->hDevSysHandle);
#endif

    // Get cmdQ service
    memset(&stCmdqBufDesp, 0, sizeof(stCmdqBufDesp));
    stCmdqBufDesp.u32CmdqBufSize       = VPE_CMDQ_BUFF_SIZE_MAX;
    stCmdqBufDesp.u32CmdqBufSizeAlign  = VPE_CMDQ_BUFF_ALIGN;
    stCmdqBufDesp.u32MloadBufSize      = VPE_MLOAD_BUFF_SIZE_MAX;
    stCmdqBufDesp.u16MloadBufSizeAlign = VPE_MLOAD_BUFF_ALIGN;
    //if debug
    //pstDevInfo->pstCmdMloadInfo = get_sys_cmdq_service(CMDQ_ID_VPE, cmdq_buffer_descript_t * pCmdqBufDesp, TRUE);
    //else
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
    if (pstDevInfo->bProcDisableCmdq == TRUE)
    {
        pstDevInfo->pstCmdMloadInfo = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_VPE, &stCmdqBufDesp, TRUE);
    }
    else
#endif
    pstDevInfo->pstCmdMloadInfo = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_VPE, &stCmdqBufDesp, FALSE);

    if (pstDevInfo->pstCmdMloadInfo == NULL)
    {
        DBG_ERR("Fail to get cmd service.\n");
        goto __get_cmd_service_fail;
    }
    DBG_INFO("success to get cmd service.\n");

    if(MI_DEVICE_GetDebugLevel() == MI_DBG_ERR)
    {
        MI_BOOL bMhal_Debug = 0xff;
        ExecFunc(MHalVpeIspDbgLevel(&bMhal_Debug));
        ExecFunc(MHalVpeSclDbgLevel(&bMhal_Debug));
    }
    // VPE MHAL init
    memset(&stAlloc, 0, sizeof(stAlloc));
    stAlloc.alloc = _MI_VPE_IMPL_MiSysAlloc;
    stAlloc.free  = _MI_VPE_IMPL_MiSysFree;
    stAlloc.map   = _MI_VPE_IMPL_MiSysMap;
    stAlloc.unmap = _MI_VPE_IMPL_MiSysUnMap;
    stAlloc.flush_cache = _MI_VPE_IMPL_MiSysFlashCache;
    if (MHalVpeInit(&stAlloc, pstDevInfo->pstCmdMloadInfo) == FALSE)
    {
        goto __mhal_vpe_init_fail;
    }

    // Get VPE IRQ
    pstDevInfo->bEnbaleIrq = FALSE;
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_VPE_PROCFS_DEBUG == 1)
    if (pstDevInfo->bProcDisableIrq == TRUE)
    {
        pstDevInfo->bSupportIrq = FALSE;
    }
    else
#endif

    if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_3_PORTMODE)
        pstDevInfo->eIrqEnableMode = E_MHAL_SCL_IRQ_CAM_REALTIME_3PORT;
    else if(pstDevInfo->eSclPortMode == E_MI_VPE_SCL_4_PORTMODE)
        pstDevInfo->eIrqEnableMode = E_MHAL_SCL_IRQ_DVR_FRAMEBUF_4PORT;

    if (MHalVpeSclGetIrqNum(&pstDevInfo->uVpeIrqNum, pstDevInfo->eIrqEnableMode) == FALSE)
    {
        DBG_WRN("Fail to get irq.\n");
        //return MI_ERR_VPE_NOT_SUPPORT;
        pstDevInfo->bSupportIrq = FALSE;
    }
    else
    {
        pstDevInfo->bSupportIrq = TRUE;
        DBG_INFO("get irq: %d.\n", pstDevInfo->uVpeIrqNum);
    }

    INIT_LIST_HEAD(&VPE_todo_task_list);
    INIT_LIST_HEAD(&VPE_working_task_list);

    // Create work thread
    pstDevInfo->pstWorkThread = kthread_create(VPEWorkThread, pstDevInfo, "VPE/WorkThread");
    if (IS_ERR(pstDevInfo->pstWorkThread))
    {
        DBG_ERR("Fail to create thread VPE/WorkThread.\n");
        goto __create_work_thread_fail;
    }
    DBG_INFO("success to  create thread VPE/WorkThread.\n");
    #if defined(VPE_ADJUST_THREAD_PRIORITY) && (VPE_ADJUST_THREAD_PRIORITY == 1)
    param.sched_priority = 99;
    sched_setscheduler(pstDevInfo->pstWorkThread, SCHED_RR, &param);
    #endif

    // Create IRQ Bottom handler
    pstDevInfo->pstProcThread = kthread_create(_MI_VPE_IsrProcThread, pstDevInfo, "VPE/IsrProcThread");
    if (IS_ERR(pstDevInfo->pstProcThread))
    {
        DBG_ERR("Fail to create thread VPE/IsrProcThread.\n");
        goto __create_proc_thread_fail;
    }
    DBG_INFO("success to create thread VPE/IsrProcThread.\n");
#if defined(VPE_ADJUST_THREAD_PRIORITY) && (VPE_ADJUST_THREAD_PRIORITY == 1)
    param.sched_priority = 99;
    sched_setscheduler(pstDevInfo->pstProcThread, SCHED_RR, &param);
#endif
    //_gpstDbgThread = kthread_create(dump_tread, pstDevInfo, "VPE/DbgThread");

    wake_up_process(pstDevInfo->pstWorkThread);
    wake_up_process(pstDevInfo->pstProcThread);
    //wake_up_process(_gpstDbgThread);

    if (TRUE == pstDevInfo->bSupportIrq)
    {
        // Register kernel IRQ
        if (0 > request_irq(pstDevInfo->uVpeIrqNum, _MI_VPE_Isr, IRQF_SHARED | IRQF_ONESHOT, "VPE-IRQ", pstDevInfo))
        {
            DBG_ERR("Fail to request_irq: %d.\n", pstDevInfo->uVpeIrqNum);
            goto __register_vpe_irq_fail;
        }
        DBG_INFO("success to request_irq: %d.\n", pstDevInfo->uVpeIrqNum);
        //Enable Irq In Creat first Channel
    }

    pstDevInfo->bInited = TRUE;
    s32Ret = MI_VPE_OK;

    return s32Ret;

__register_vpe_irq_fail:
    kthread_stop(pstDevInfo->pstProcThread);
__create_proc_thread_fail:
    kthread_stop(pstDevInfo->pstWorkThread);
__create_work_thread_fail:
    ExecFunc(MHalVpeDeInit());
__mhal_vpe_init_fail:
    MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_VPE);
    pstDevInfo->pstCmdMloadInfo = NULL;
__get_cmd_service_fail:
    mi_sys_UnRegisterDev(pstDevInfo->hDevSysHandle);
    pstDevInfo->hDevSysHandle = NULL;

    return s32Ret;
}

void MI_VPE_IMPL_DeInit(void)
{
    MI_U8 u8VpeChnlId =0, u8PortId =0;
    mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();
    pstDevInfo->bInited = FALSE;

    while(pstDevInfo->u32ChannelCreatedNum>0 && u8VpeChnlId < MI_VPE_MAX_CHANNEL_NUM)
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(u8VpeChnlId);

        if(pstChnnInfo->bCreated)
        {
            if(pstChnnInfo->eStatus == E_MI_VPE_CHANNEL_STATUS_START)
                MI_VPE_IMPL_StopChannel(u8VpeChnlId);

            for(u8PortId=0; u8PortId <MI_VPE_MAX_PORT_NUM; u8PortId++)
            {
                mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(u8VpeChnlId, u8PortId);
                if(pstOutPortInfo->bEnable)
                    MI_VPE_IMPL_DisablePort(u8VpeChnlId, u8PortId);
            }

            MI_VPE_IMPL_DestroyChannel(u8VpeChnlId);
        }

        u8VpeChnlId ++;
    }

    if (pstDevInfo->bSupportIrq == TRUE)
    {
        free_irq(pstDevInfo->uVpeIrqNum, pstDevInfo);
        pstDevInfo->bEnbaleIrq = FALSE;
    }
    kthread_stop(pstDevInfo->pstProcThread);
    kthread_stop(pstDevInfo->pstWorkThread);
    //kthread_stop(_gpstDbgThread);
    MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_VPE);
    pstDevInfo->pstCmdMloadInfo = NULL;
    MHalVpeDeInit();
    mi_sys_UnRegisterDev(pstDevInfo->hDevSysHandle);
    pstDevInfo->hDevSysHandle = NULL;
    pstDevInfo->eRunningMode = E_MI_VPE_RUNNING_MODE_INVALID;
}
