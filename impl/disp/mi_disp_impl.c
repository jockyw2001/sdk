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

#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/math64.h>
#include <asm/uaccess.h>


#include "mi_syscfg_datatype.h"
#include "mi_syscfg.h"
#include "mi_common_datatype.h"
#include "mi_sys_sideband_msg.h"
#include "mi_print.h"
#include "mhal_common.h"
#include "mhal_disp.h"
#include "mi_gfx.h"
#include "mi_sys_internal.h"
#include "../sys/mi_sys_impl.h"
#include "../../interface/include/vdec/mi_vdec_internal.h"
#include "../divp/mi_divp_impl.h"
#include "mi_disp.h"
#include "mi_disp_datatype.h"
#include "mi_disp_datatype_internal.h"
#include "mi_disp_impl.h"
#include "mi_disp_cus_allocator.h"
#include "mi_sys_proc_fs_internal.h"

#define UNUSED(x) (x) = (x)

#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#endif
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, alignment) (( (val)/(alignment))*(alignment))
#endif


//#define DISP_DBG_ERR(fmt, args...)  printk(fmt, ##args)
//#define DISP_DBG_INFO(fmt, args...) printk(fmt, ##args)
#define DISP_DBG_ERR(fmt, args...)     DBG_ERR(fmt, ##args)
#define DISP_DBG_INFO(fmt, args...)    DBG_INFO(fmt, ##args)
#define DISP_DBG_WARN(fmt, args...)    DBG_WRN(fmt, ##args)
#define DISP_DBG_ENTER(fmt, args...)   DBG_ENTER(fmt, ##args)
#define DISP_DBG_EXITOK(fmt, args...)  DBG_EXIT_OK(fmt, ##args)
#define DISP_DBG_EXITERR(fmt, args...) DBG_EXIT_ERR(fmt, ##args)

//#define DISP_DBG_ERR(fmt, args...)
//#define DISP_DBG_INFO(fmt, args...)

#define MI_DISP_FORCE_ENABLE_PTS_CHK (1)

#define MI_DISP_CHECK_NULLPOINTER(pointer)\
    {\
        if(pointer == NULL)\
        {\
            DISP_DBG_ERR("NULL pointer!!!\n");\
            goto EXIT;\
        }\
    }
#define MI_DISP_CHECK_INVAILDDEV(Dev)\
    {\
        if((Dev) > MI_DISP_DEV_MAX)\
        {\
            DISP_DBG_ERR("Invild Display Dev!!!\n");\
            goto EXIT;\
        }\
    }
#define MI_DISP_CHECK_INVAILDLAYER(Layer)\
        {\
            if((Layer) > MI_DISP_LAYER_MAX)\
            {\
                DISP_DBG_ERR("Invild Display Layer!!!\n");\
                goto EXIT;\
            }\
        }
#define MI_DISP_CHECK_INVAILDINPUTPORT(Port)\
            {\
                if((Port) > MI_DISP_INPUTPORT_MAX)\
                {\
                    DISP_DBG_ERR("Invild Input Port!!!\n");\
                    goto EXIT;\
                }\
            }


#define STATIC_ASSERT(_x_)                              \
    do {                                                \
        char c[(_x_)?(1):(-1)];          \
        c[0]='\0';                            \
        c[0]= c[0];                            \
    } while (0)

#define ToMHAL_DISP_SyncInfo(hal, mi)\
        do \
    {\
        (hal)->bSynm = (mi)->bSynm;\
        (hal)->bIop = (mi)->bIop;\
        (hal)->u8Intfb = (mi)->u8Intfb;\
        (hal)->u16Vact  = (mi)->u16Vact ;\
        (hal)->u16Vbb = (mi)->u16Vbb;\
        (hal)->u16Vfb = (mi)->u16Vfb;\
        (hal)->u16Hact = (mi)->u16Hact;\
        (hal)->u16Hbb = (mi)->u16Hbb;\
        (hal)->u16Hfb = (mi)->u16Hfb;\
        (hal)->u16Hmid = (mi)->u16Hmid;\
        (hal)->u16Bvact = (mi)->u16Bvact;\
        (hal)->u16Bvbb = (mi)->u16Bvbb;\
        (hal)->u16Bvfb = (mi)->u16Bvfb;\
        (hal)->u16Hpw = (mi)->u16Hpw;\
        (hal)->u16Vpw = (mi)->u16Vpw;\
        (hal)->bIdv = (mi)->bIdv;\
        (hal)->bIhs = (mi)->bIhs;\
        (hal)->bIvs = (mi)->bIvs;\
        (hal)->u32FrameRate = (mi)->u32FrameRate;\
    }while(0)

typedef enum
{
    E_MI_DISP_FRAME_NORMAL,
    E_MI_DISP_FRAME_WAIT,
    E_MI_DISP_FRAME_DROP,
    E_MI_DISP_FRAME_SHOW_LAST,
}MI_DISP_HandleFrame_e;

#if defined(VPE_TASK_PERF_DBG) && (VPE_TASK_PERF_DBG == 1)
#define VPE_PERF_PRINT(fmt, args...) do {printk("[DISP_PERF]: %s [%d] ", __func__, __LINE__); printk(fmt, ##args);} while(0)

#define DISP_PERF_TIME(pu64Time) do {\
    struct timespec stTime1;\
    memset(&stTime1, 0, sizeof(stTime1));\
    do_posix_clock_monotonic_gettime(&stTime1);\
    *(pu64Time) = (stTime1.tv_sec * 1000 * 1000 + (stTime1.tv_nsec / 1000));\
} while(0)
#else
#define VPE_PERF_PRINT(fmt, args...)
#define DISP_PERF_TIME(pu64Time)
#endif
#define DISP_GET_PERF_TIME(pu64Time) do {\
    struct timespec sttime;\
    memset(&sttime, 0, sizeof(sttime));\
    do_posix_clock_monotonic_gettime(&sttime);\
    *(pu64Time) = ((MI_U64)sttime.tv_sec) * 1000000ULL + (sttime.tv_nsec / 1000);\
} while(0)

#define UNSIGNED_DIFF(a, b)  (((a)>=(b))?((a)-(b)):((b)-(a)))

//Internal function
static MI_BOOL _bDispInit = FALSE;

DEFINE_SEMAPHORE(MI_DISP_globalVarSem);
//#define MI_DISP_GetVarMutex()      DBG_ERR("MI_DISP_GetVarMutex start");down(&(MI_DISP_globalVarSem));DBG_ERR("MI_DISP_GetVarMutex end \n")
//#define MI_DISP_ReleaseVarMutex()  DBG_ERR("MI_DISP_ReleaseVarMutex start");up(&(MI_DISP_globalVarSem));DBG_ERR("MI_DISP_ReleaseVarMutex end\n")
#define MI_DISP_GetVarMutex()      down(&(MI_DISP_globalVarSem))
#define MI_DISP_ReleaseVarMutex()  up(&(MI_DISP_globalVarSem))

static mi_disp_DevStatus_t* pstDevice0Param = NULL;
static mi_disp_DevStatus_t* pstDevice1Param = NULL;
static mi_disp_DevStatus_t astDevStatus[MI_DISP_DEV_MAX];
static mi_disp_LayerStatus_t astLayerParams[MI_DISP_LAYER_MAX];

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
#define PRINTF_PROC(fmt, args...)  {do{MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_RED"[MI DISP PROCFS]:" fmt ASCII_COLOR_END, ##args);}while(0);}
#define PARSING_DISP_INTERFACE(x) ( x == E_MI_DISP_INTF_CVBS    ?  "CVBS"    : \
                                    x == E_MI_DISP_INTF_YPBPR   ?  "YPBPR"   : \
                                    x == E_MI_DISP_INTF_VGA     ?  "VGA"     : \
                                    x == E_MI_DISP_INTF_BT656   ?  "BT656"   : \
                                    x == E_MI_DISP_INTF_BT1120  ?  "BT1120"  : \
                                    x == E_MI_DISP_INTF_HDMI    ?  "HDMI"    : \
                                    x == E_MI_DISP_INTF_LCD     ?  "LCD"     : \
                                    x == E_MI_DISP_INTF_BT656_H ?  "BT656_H" : \
                                    x == E_MI_DISP_INTF_BT656_L ?  "BT656_L" : \
                                                                   "UNKNOWN" )

#define PARSING_DISP_OUTPUT_TIMING(x) ( x == E_MI_DISP_OUTPUT_PAL          ?    "PAL"          : \
                                        x == E_MI_DISP_OUTPUT_NTSC         ?    "NTSC"         : \
                                        x == E_MI_DISP_OUTPUT_960H_PAL     ?    "960H_PAL"     : \
                                        x == E_MI_DISP_OUTPUT_960H_NTSC    ?    "960H_NTSC"    : \
                                        x == E_MI_DISP_OUTPUT_480i60       ?    "480i60"       : \
                                        x == E_MI_DISP_OUTPUT_576i50       ?    "576i50"       : \
                                        x == E_MI_DISP_OUTPUT_480P60       ?    "480P60"       : \
                                        x == E_MI_DISP_OUTPUT_576P50       ?    "576P50"       : \
                                        x == E_MI_DISP_OUTPUT_720P50       ?    "720P50"       : \
                                        x == E_MI_DISP_OUTPUT_720P60       ?    "720P60"       : \
                                        x == E_MI_DISP_OUTPUT_1080P24      ?    "1080P24"      : \
                                        x == E_MI_DISP_OUTPUT_1080P25      ?    "1080P25"      : \
                                        x == E_MI_DISP_OUTPUT_1080P30      ?    "1080P30"      : \
                                        x == E_MI_DISP_OUTPUT_1080I50      ?    "1080I50"      : \
                                        x == E_MI_DISP_OUTPUT_1080I60      ?    "1080I60"      : \
                                        x == E_MI_DISP_OUTPUT_1080P50      ?    "1080P50"      : \
                                        x == E_MI_DISP_OUTPUT_1080P60      ?    "1080P60"      : \
                                        x == E_MI_DISP_OUTPUT_640x480_60   ?    "640x480P60"   : \
                                        x == E_MI_DISP_OUTPUT_800x600_60   ?    "800x600P60"   : \
                                        x == E_MI_DISP_OUTPUT_1024x768_60  ?    "1024x768P60"  : \
                                        x == E_MI_DISP_OUTPUT_1280x1024_60 ?    "1280x1024P60" : \
                                        x == E_MI_DISP_OUTPUT_1366x768_60  ?    "1366x768P60"  : \
                                        x == E_MI_DISP_OUTPUT_1440x900_60  ?    "1440x900P60"  : \
                                        x == E_MI_DISP_OUTPUT_1280x800_60  ?    "1280x800P60"  : \
                                        x == E_MI_DISP_OUTPUT_1680x1050_60 ?    "1680x1050P60" : \
                                        x == E_MI_DISP_OUTPUT_1920x2160_30 ?    "1920x2160P30" : \
                                        x == E_MI_DISP_OUTPUT_1600x1200_60 ?    "1600x1200P60" : \
                                        x == E_MI_DISP_OUTPUT_1920x1200_60 ?    "1920x1200P60" : \
                                        x == E_MI_DISP_OUTPUT_2560x1440_30 ?    "2560x1440P30" : \
                                        x == E_MI_DISP_OUTPUT_2560x1600_60 ?    "2560x1600P60" : \
                                        x == E_MI_DISP_OUTPUT_3840x2160_30 ?    "3840x2160P30" : \
                                        x == E_MI_DISP_OUTPUT_3840x2160_60 ?    "3840x2160P60" : \
                                                                                "UNKNOWN" )

static stDispCheckFrameRateTimer_t gstDispCheckFrameRateTimer;
static MI_U8 gu8StatFrameRateChnCnt = 0;
void _mi_disp_CheckFramePts(MI_SYS_BufInfo_t *pstBufInfo, MI_U8 LayerID, MI_U8 InPortID);
void _mi_disp_CheckFrameRate(MI_SYS_BufInfo_t *pstBufInfo, MI_U8 DispChn ,MI_U8 InPortID);
void _mi_disp_DumpFrame(MI_SYS_BufInfo_t *pstBufInfo, MI_U8 DispChn ,MI_U8 InPortID);
#endif


MI_S32 MI_DISP_IMPL_Enable(MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("Enable Display Dev %d !\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_GetVarMutex();
    if(MHAL_DISP_DeviceEnable(astDevStatus[DispDev].pstDevObj, TRUE) != TRUE)
    {
        DISP_DBG_ERR("Hal Enable Display Dev Fail!!!\n");
        goto UP_DEV_MUTEX;
    }

    astDevStatus[DispDev].bDISPEnabled = TRUE;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_Disable(MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8PortCount = 0;
    DISP_DBG_ENTER("Disable Display Dev %d !\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);

    MI_DISP_GetVarMutex();

    if(!list_empty(&(astDevStatus[DispDev].stBindedLayer)))
    {
        struct list_head *pstPos = NULL;
        struct list_head *n = NULL;
        mi_disp_LayerStatus_t *pstTmpLayer = NULL;
        list_for_each_safe(pstPos, n, &(astDevStatus[DispDev].stBindedLayer))
        {
            pstTmpLayer = list_entry(pstPos, mi_disp_LayerStatus_t, stLayerNode);
            for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
            {
                if(!list_empty(&pstTmpLayer->stPortPendingBufQueue[u8PortCount]))
                {
                    struct list_head *pstPendingBufferPos = NULL;
                    struct list_head *n = NULL;
                    mi_disp_PortPendingBuf_t *pstTmpPendingBuf;
                    MI_SYS_BufInfo_t* pstTmpInputBuffer;
                    list_for_each_safe(pstPendingBufferPos, n, &(pstTmpLayer->stPortPendingBufQueue[u8PortCount]))
                    {
                        pstTmpPendingBuf = list_entry(pstPendingBufferPos, mi_disp_PortPendingBuf_t, stPortPendingBufNode);
                        list_del(pstPendingBufferPos);
                        pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                        memset(pstTmpPendingBuf, 0xF2, sizeof(mi_disp_PortPendingBuf_t));
                        kfree(pstTmpPendingBuf);
                        if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_SUCCESS)
                        {
                            DISP_DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                        }
                    }
                }
            }
        }
    }
    /*if(!list_empty(&(astDevStatus[DispDev].stBindedLayer)))
    {
        struct list_head *pstPos = NULL;
        struct list_head *n = NULL;
        mi_disp_LayerStatus_t *pstTmpLayer = NULL;
        list_for_each_safe(pstPos, n, &(astDevStatus[DispDev].stBindedLayer))
        {
            pstTmpLayer = list_entry(pstPos, mi_disp_LayerStatus_t, stLayerNode);
            if(pstTmpLayer->bLayerEnabled)
            {
                DISP_DBG_ERR("Layer %d not disable\n", pstTmpLayer->u8LayerID);
                goto UP_DEV_MUTEX;
            }
        }
    }*/
    if(MHAL_DISP_DeviceEnable((astDevStatus[DispDev].pstDevObj), FALSE) != TRUE)
    {
        DISP_DBG_ERR("Hal Disable Display Dev Fail!!!\n");
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].bDISPEnabled = FALSE;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;

}

MHAL_DISP_DeviceTiming_e mi_disp_ConvertDisp2MhalTiming(MI_DISP_OutputTiming_e eIntfSync, MI_U32 *pu32VsyncInterval)
{
    MHAL_DISP_DeviceTiming_e eHalDeviceTiming = E_MI_DISP_OUTPUT_MAX;

    MI_U32 u32HZ = 60;

    switch(eIntfSync)
    {
        case E_MI_DISP_OUTPUT_PAL:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_PAL;
            u32HZ = 25;
            break;
        case E_MI_DISP_OUTPUT_NTSC:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_NTSC;
            u32HZ = 30; //29.97
            break;
        case E_MI_DISP_OUTPUT_960H_PAL:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_960H_PAL;
             u32HZ = 25;
            break;
        case E_MI_DISP_OUTPUT_960H_NTSC:
            //eHalDeviceTiming = E_MHAL_DISP_OUTPUT_960H_NTSC;
            u32HZ = 30; //29.97
            break;
        case E_MI_DISP_OUTPUT_480i60:
            //eHalDeviceTiming = E_MHAL_DISP_OUTPUT_PAL;
             u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_576i50:
            //eHalDeviceTiming = E_MHAL_DISP_OUTPUT_NTSC;
            u32HZ = 50;
            break;
        case E_MI_DISP_OUTPUT_576P50:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_576P50;
            u32HZ = 50;
            break;
        case E_MI_DISP_OUTPUT_720P50:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_720P50;
            u32HZ = 50;
            break;
        case E_MI_DISP_OUTPUT_720P60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_720P60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_1080P24:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1080P24;
            u32HZ = 24;
            break;
        case E_MI_DISP_OUTPUT_1080P25:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1080P25;
            u32HZ = 25;
            break;
        case E_MI_DISP_OUTPUT_1080P30:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1080P30;
            u32HZ = 30;
            break;
        case E_MI_DISP_OUTPUT_1080I50:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1080I50;
            u32HZ = 50;
            break;
        case E_MI_DISP_OUTPUT_1080P50:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1080P50;
            u32HZ = 50;
            break;
        case E_MI_DISP_OUTPUT_1080P60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1080P60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_640x480_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_640x480_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_800x600_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_800x600_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_1024x768_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1024x768_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_1280x1024_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1280x1024_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_1366x768_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1366x768_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_1440x900_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1440x900_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_1280x800_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1280x800_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_1680x1050_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1680x1050_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_1920x2160_30:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1920x2160_30;
            u32HZ = 30;
            break;
        case E_MI_DISP_OUTPUT_1600x1200_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1600x1200_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_1920x1200_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_1920x1200_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_2560x1440_30:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_2560x1440_30;
            u32HZ = 30;
            break;
        case E_MI_DISP_OUTPUT_2560x1600_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_2560x1600_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_3840x2160_30:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_3840x2160_30;
            u32HZ = 30;
            break;
        case E_MI_DISP_OUTPUT_3840x2160_60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_3840x2160_60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_480P60:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_480P60;
            u32HZ = 60;
            break;
        case E_MI_DISP_OUTPUT_USER:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_USER;
            break;
        default:
            eHalDeviceTiming = E_MHAL_DISP_OUTPUT_MAX;
            break;
    }
    if(pu32VsyncInterval)
        *pu32VsyncInterval = 1000000UL/u32HZ;
    DISP_DBG_INFO("%s %d eDev Timing = %d, IP timing %uHZ \n", __FUNCTION__,__LINE__,eHalDeviceTiming, u32HZ);
    return eHalDeviceTiming;
}

// TODO: Tommy Add more dev, Need verify -- Done
MI_S32 MI_DISP_IMPL_SetPubAttr(MI_DISP_DEV DispDev, const MI_DISP_PubAttr_t *pstPubAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_DeviceTimingInfo_t stDevTimingInfo;
    MHAL_DISP_SyncInfo_t stDispSyncInfo;
    MI_U32 u32HalDevice;
    MI_U32 u32OutInterface =0;
    MI_U32 u32VsyncInterval;
    DISP_DBG_ENTER("dev %d, interface %d, timing %d.\n", DispDev,pstPubAttr->eIntfType, pstPubAttr->eIntfSync);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstPubAttr);
    MI_DISP_GetVarMutex();
    if(MHAL_DISP_DeviceSetBackGroundColor((astDevStatus[DispDev].pstDevObj), pstPubAttr->u32BgColor) != TRUE)
    {
        DISP_DBG_ERR("Hal Set backcolor fail!!!\n");
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].u32BgColor = pstPubAttr->u32BgColor;

    switch(pstPubAttr->eIntfType)
    {
        case E_MI_DISP_INTF_HDMI:
            u32HalDevice = MHAL_DISP_INTF_HDMI;
            break;
        case E_MI_DISP_INTF_CVBS:
            u32HalDevice = MHAL_DISP_INTF_CVBS;
            break;
        case E_MI_DISP_INTF_VGA:
            u32HalDevice = MHAL_DISP_INTF_VGA;
            break;
        case E_MI_DISP_INTF_LCD:
            u32HalDevice = MHAL_DISP_INTF_LCD;
            break;
        case E_MI_DISP_INTF_YPBPR:
            u32HalDevice = MHAL_DISP_INTF_YPBPR;
            break;
        default:
            u32HalDevice = MHAL_DISP_INTF_HDMI;
            break;
    }

    if(((astDevStatus[DispDev].u32Interface &(1 << E_MI_DISP_INTF_HDMI)) && (pstPubAttr->eIntfType == E_MI_DISP_INTF_CVBS))
        ||((astDevStatus[DispDev].u32Interface &(1 << E_MI_DISP_INTF_CVBS)) && (pstPubAttr->eIntfType == E_MI_DISP_INTF_HDMI))
        )
    {
        u32OutInterface = MHAL_DISP_INTF_HDMI | MHAL_DISP_INTF_CVBS;
    }
    else
    {
        u32OutInterface = u32HalDevice;
    }

    if(MHAL_DISP_DeviceAddOutInterface((astDevStatus[DispDev].pstDevObj), u32OutInterface) != TRUE)
    {
        DISP_DBG_ERR("Hal Add Output device fail!!!\n");
        goto UP_DEV_MUTEX;
    }
    memset(&stDevTimingInfo, 0, sizeof(MHAL_DISP_DeviceTimingInfo_t));
    //stDevTimingInfo.eDeviceType = pstPubAttr->eIntfType;
    stDevTimingInfo.eTimeType = mi_disp_ConvertDisp2MhalTiming(pstPubAttr->eIntfSync, &u32VsyncInterval);
    //memcpy(stDevTimingInfo.pstSyncInfo, &pstPubAttr->stSyncInfo, sizeof(pstPubAttr->stSyncInfo));
    memset(&stDispSyncInfo, 0, sizeof(stDispSyncInfo));
    stDevTimingInfo.pstSyncInfo = &stDispSyncInfo;
    ToMHAL_DISP_SyncInfo(stDevTimingInfo.pstSyncInfo, &pstPubAttr->stSyncInfo);

    if(TRUE != MHAL_DISP_DeviceSetOutputTiming((astDevStatus[DispDev].pstDevObj), u32HalDevice, &stDevTimingInfo))
    {
        DISP_DBG_ERR("Hal Set Output Timing fail!!!\n");
        goto UP_DEV_MUTEX;
    }
    DISP_DBG_INFO("stDevTimingInfo.eTimeType: %d.\n", stDevTimingInfo.eTimeType);

    DISP_DBG_INFO("u32Interface: %d.\n", astDevStatus[DispDev].u32Interface);

    astDevStatus[DispDev].u32Interface |= (1 << pstPubAttr->eIntfType);
    astDevStatus[DispDev].eDeviceTiming[pstPubAttr->eIntfType] = pstPubAttr->eIntfSync;
    DISP_DBG_INFO("eDeviceTiming: %d.\n", astDevStatus[DispDev].eDeviceTiming[pstPubAttr->eIntfType]);

    if(!(astDevStatus[DispDev].u32Interface&(astDevStatus[DispDev].u32Interface-1))  //only one device enabled
        ||pstPubAttr->eIntfType==E_MI_DISP_INTF_HDMI||pstPubAttr->eIntfType==E_MI_DISP_INTF_VGA)
    {
        astDevStatus[DispDev].u32VsyncInterval = u32VsyncInterval;
    }

    memcpy(&astDevStatus[DispDev].stSyncInfo, &pstPubAttr->stSyncInfo, sizeof(pstPubAttr->stSyncInfo));

    astDevStatus[DispDev].u32AccumInterruptTimeStamp = 0;
    astDevStatus[DispDev].u32AccumInterruptCnt = 0;

    DISP_DBG_INFO("eDeviceTiming: %d.\n", astDevStatus[DispDev].eDeviceTiming[pstPubAttr->eIntfType]);

    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 mi_disp_CheckInvaildIntfType(MI_DISP_Interface_e eIntfType)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    if((eIntfType >= E_MI_DISP_INTF_CVBS) && (eIntfType < E_MI_DISP_INTF_MAX))
    {
        s32Ret = MI_SUCCESS;
    }
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_GetPubAttr(MI_DISP_DEV DispDev, MI_DISP_PubAttr_t *pstPubAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_DeviceTimingInfo_t stDevTimingInfo;
    memset(&stDevTimingInfo, 0, sizeof(MHAL_DISP_DeviceTimingInfo_t));
    DISP_DBG_ENTER("Get Display dev %d, Public Attrs !\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstPubAttr);
    //MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    MI_DISP_GetVarMutex();
    if(astDevStatus[DispDev].bDISPEnabled == TRUE)
    {
        int index = pstPubAttr->eIntfType;
        pstPubAttr->u32BgColor = astDevStatus[DispDev].u32BgColor;
        if(mi_disp_CheckInvaildIntfType(pstPubAttr->eIntfType))
        {
            pstPubAttr->eIntfSync = astDevStatus[DispDev].eDeviceTiming[index];
        }
        memcpy(&pstPubAttr->stSyncInfo, &astDevStatus[DispDev].stSyncInfo, sizeof(astDevStatus[DispDev].stSyncInfo));
    }
    s32Ret = MI_SUCCESS;
    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;

}

MI_S32 MI_DISP_IMPL_DeviceAttach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("Attach Dev %d to Dest Dev %d !\n", DispSrcDev, DispDstDev);
    MI_DISP_CHECK_INVAILDDEV(DispSrcDev);
    MI_DISP_CHECK_INVAILDDEV(DispDstDev);
    // TODO: Warning Need to check: This may cause dead lock.
#if 0
    down(&(astDevStatus[DispSrcDev].stDevMutex));
    down(&(astDevStatus[DispDstDev].stDevMutex));
#endif
    MI_DISP_GetVarMutex();
    // Just attach. Do not need enable MI_SYS_Channel
    if(!(astDevStatus[DispSrcDev].bDISPEnabled && astDevStatus[DispDstDev].bDISPEnabled))
    {
        DISP_DBG_ERR("Dest or Src Dev not Enabled!!!\n!!!");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }
    if(!list_empty(&(astDevStatus[DispDstDev].stBindedLayer)))
    {
        DISP_DBG_ERR("Layer binded DstDev already!!!\n!!!");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }

    if(!MHAL_DISP_DeviceAttach((astDevStatus[DispSrcDev].pstDevObj), (astDevStatus[DispDstDev].pstDevObj)))
    {
        DISP_DBG_ERR("Hal Attach fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_DeviceDetach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("Detach Dev %d to Dest Dev %d !\n", DispSrcDev, DispDstDev);
    MI_DISP_CHECK_INVAILDDEV(DispSrcDev);
    MI_DISP_CHECK_INVAILDDEV(DispDstDev);
#if 0
    down(&(astDevStatus[DispSrcDev].stDevMutex));
    down(&(astDevStatus[DispDstDev].stDevMutex));
#endif
    MI_DISP_GetVarMutex();

    if(!(astDevStatus[DispSrcDev].bDISPEnabled && astDevStatus[DispDstDev].bDISPEnabled))
    {
        DISP_DBG_ERR("Dest or Src Dev not Enabled!!!\n!!!");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }

    if(FALSE == MHAL_DISP_DeviceDetach((astDevStatus[DispSrcDev].pstDevObj), (astDevStatus[DispDstDev].pstDevObj)))
    {
        DISP_DBG_ERR("Hal Attach fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }
    s32Ret = MI_SUCCESS;
    DISP_DBG_ENTER();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();

EXIT:
    return s32Ret;
}

static MI_U32 mi_disp_TransLayerToDevChnn(MI_DISP_DEV DispDev, MI_DISP_LAYER DispLayer)
{
    if ((DispLayer == 1) && (DispDev == 1))
    {
        return 0;
    }
    else
    {
        return DispLayer;
    }

}

MI_S32 MI_DISP_IMPL_EnableVideoLayer(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_AllocPhyMem_t stHalAlloc;
    MI_U8 u8PortCount = 0;
    MI_U8 u8DevID = 0;
    DISP_DBG_ENTER("VideoLayer = %d !\n", DispLayer);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    u8DevID = astLayerParams[DispLayer].u8BindedDevID;
    MI_DISP_CHECK_INVAILDDEV(u8DevID);
    //down(&(astLayerParams[DispLayer].stLayerMutex));
    MI_DISP_GetVarMutex();
    // TODO: Tommy Channel Channel user trans function
    s32Ret = mi_sys_EnableChannel(astDevStatus[u8DevID].hDevSysHandle, mi_disp_TransLayerToDevChnn(u8DevID, DispLayer));
    if(s32Ret != MI_SUCCESS)
    {
        DISP_DBG_ERR("mi_sys_EnableChannel Fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_LAYER_MUTEX;
    }

    if(!MHAL_DISP_VideoLayerEnable((astLayerParams[DispLayer].apLayerObjs), TRUE))
    {
        DISP_DBG_ERR("Enable Video Layer Fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_LAYER_MUTEX;
    }
    astLayerParams[DispLayer].bLayerEnabled = TRUE;

    stHalAlloc.free = mi_sys_MMA_Free;
    stHalAlloc.alloc = mi_sys_MMA_Alloc;
    for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
    {
        //In k6l other port astLayerParams[DispLayer].astPortStatus[u8PortCount].apInputObjs is NULL
        if(!MHAL_DISP_InputPortCreateInstance(&stHalAlloc, (astLayerParams[DispLayer].apLayerObjs), u8PortCount, &(astLayerParams[DispLayer].astPortStatus[u8PortCount].apInputObjs)))
        {
	    #if (defined CONFIG_MSTAR_CHIP_I2) && (CONFIG_MSTAR_CHIP_I2 == 1)
            DISP_DBG_ERR("InputPort Create Instance fail!!!\n");
            #endif
            continue;
        }
    }
    s32Ret = MI_SUCCESS;

#if DISP_ENABLE_CUS_ALLOCATOR
   mi_disp_config_layer_canvas(astDevStatus[u8DevID].hDevSysHandle, mi_disp_TransLayerToDevChnn(u8DevID, DispLayer), DispLayer, astLayerParams[DispLayer].stVideoLayerAttr.stVidLayerSize.u16Width, astLayerParams[DispLayer].stVideoLayerAttr.stVidLayerSize.u16Height, MI_DISP_CUS_ALLOCATOR_BUF_CNT);
#endif
        DISP_DBG_EXITOK();
UP_LAYER_MUTEX:
    //up(&(astLayerParams[DispLayer].stLayerMutex));
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;

}

// TODO: Tommy doc add ,if video layer disbaled, the inpurt automatic disabled
MI_S32 MI_DISP_IMPL_DisableVideoLayer(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8PortCount = 0;
    MI_U8 u8DevID = 0;
#if DISP_ENABLE_CUS_ALLOCATOR
    MI_DISP_VidWinRect_t stDispWin = {0, 0, 0, 0};
#endif
    DISP_DBG_ENTER("VideoLayer = %d !\n", DispLayer);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    u8DevID = astLayerParams[DispLayer].u8BindedDevID;
    MI_DISP_CHECK_INVAILDDEV(u8DevID);

    //down(&(astLayerParams[DispLayer].stLayerMutex));
    MI_DISP_GetVarMutex();

    for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
    {
        //if apInputObjs == NULL input port instance not Create
        if((astLayerParams[DispLayer].astPortStatus[u8PortCount].apInputObjs != NULL)
            &&(astLayerParams[DispLayer].astPortStatus[u8PortCount].bEnable == FALSE))
        {
            if(MHAL_DISP_InputPortDestroyInstance((astLayerParams[DispLayer].astPortStatus[u8PortCount].apInputObjs)) == FALSE)
            {
                DISP_DBG_ERR("InputPort Destory Instance fail!!!\n");
                continue;
            }
            astLayerParams[DispLayer].astPortStatus[u8PortCount].apInputObjs = NULL;
        }
        else if(astLayerParams[DispLayer].astPortStatus[u8PortCount].bEnable == TRUE)
        {
            DISP_DBG_WARN("Disable layer but inputport %d not disable", u8PortCount);
            //MI_SYS_BUG();
        }
    }

    if(!MHAL_DISP_VideoLayerEnable((astLayerParams[DispLayer].apLayerObjs), FALSE))
    {
        DISP_DBG_ERR("Disable Video Layer Fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_LAYER_MUTEX;
    }

    astLayerParams[DispLayer].bLayerEnabled = FALSE;

    s32Ret = mi_sys_DisableChannel(astDevStatus[u8DevID].hDevSysHandle, mi_disp_TransLayerToDevChnn(u8DevID, DispLayer));
    if(s32Ret != MI_SUCCESS)
    {
        DISP_DBG_ERR("mi_sys_DisableChannel!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_LAYER_MUTEX;
    }
    for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
    {
#if 0
        memset(&(astLayerParams[DispLayer].astPortStatus[u8PortCount]), 0, sizeof(mi_disp_InputPortStatus_t));
        astLayerParams[DispLayer].pstOnScreenBufInfo[u8PortCount] = NULL;
        astLayerParams[DispLayer].pstCurrentFiredBufInfo[u8PortCount] = NULL;
#endif
        //reset global variables
        memset(&(astLayerParams[DispLayer].astPortStatus[u8PortCount]), 0, sizeof(mi_disp_InputPortStatus_t));

        down(&(astLayerParams[DispLayer].stDispLayerPendingQueueMutex));
        if(!list_empty(&astLayerParams[DispLayer].stPortPendingBufQueue[u8PortCount]))
        {
            struct list_head *pstPendingBufferPos = NULL;
            struct list_head *n = NULL;
            mi_disp_PortPendingBuf_t *pstTmpPendingBuf;
            MI_SYS_BufInfo_t* pstTmpInputBuffer;
            list_for_each_safe(pstPendingBufferPos, n, &(astLayerParams[DispLayer].stPortPendingBufQueue[u8PortCount]))
            {
                pstTmpPendingBuf = list_entry(pstPendingBufferPos, mi_disp_PortPendingBuf_t, stPortPendingBufNode);
                list_del(pstPendingBufferPos);
                pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                memset(pstTmpPendingBuf, 0xF2, sizeof(mi_disp_PortPendingBuf_t));
                kfree(pstTmpPendingBuf);
                if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_SUCCESS)
                {
                    DISP_DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                }
            }
        }
        up(&(astLayerParams[DispLayer].stDispLayerPendingQueueMutex));
#if DISP_ENABLE_CUS_ALLOCATOR
        mi_disp_layer_port_set_winrect(DispLayer, u8PortCount, &stDispWin);
#endif
    }//Go through input port in Video Layer

#if DISP_ENABLE_CUS_ALLOCATOR
   mi_disp_config_layer_canvas(astDevStatus[u8DevID].hDevSysHandle, mi_disp_TransLayerToDevChnn(u8DevID, DispLayer), DispLayer, 0, 0, 0);
#endif

    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_LAYER_MUTEX:
//    up(&(astLayerParams[DispLayer].stLayerMutex));
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 mi_disp_impl_GetOutputTiming(MI_DISP_LAYER DispLayer, MI_DISP_OutputTiming_e* pOutputTiming, MI_DISP_SyncInfo_t* pstSyncInfo)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_DEV DispDev = 0;
    DISP_DBG_INFO("MI_DISP_IMPL_GetOutputTiming !\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pOutputTiming);
    //MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    MI_DISP_GetVarMutex();
    DispDev = astLayerParams[DispLayer].u8BindedDevID;
    if(astDevStatus[DispDev].bDISPEnabled == TRUE)
    {
        //ssleep(2);
        printk("[%s %d]  start\n", __FUNCTION__, __LINE__);
        *pOutputTiming = astDevStatus[DispDev].eDeviceTiming[E_MI_DISP_INTF_HDMI];
        //ssleep(2);
        printk("[%s %d]  start\n", __FUNCTION__, __LINE__);
        *pstSyncInfo = astDevStatus[DispDev].stSyncInfo;
        printk("[%s %d]  end\n", __FUNCTION__, __LINE__);
        //ssleep(2);
    }
    s32Ret = MI_SUCCESS;
//UP_DEV_MUTEX:
    //MI_DISP_UP(&(astDevStatus[DispDev].stDevMutex));
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

// TODO: Tommy delete Compress Attr & doc -- done

// TODO: Disable video layer when change layer attr
MI_S32 MI_DISP_IMPL_SetVideoLayerAttr(MI_DISP_LAYER DispLayer, const MI_DISP_VideoLayerAttr_t *pstLayerAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_VideoLayerAttr_t stHalLayerAttr;
    int i = 0;
    DISP_DBG_ENTER("layer%d, layersize (%dx%d), layerdispwin(%dx%d), pixel %d!\n", DispLayer, pstLayerAttr->stVidLayerSize.u16Width, 
        pstLayerAttr->stVidLayerSize.u16Height, pstLayerAttr->stVidLayerDispWin.u16Width, pstLayerAttr->stVidLayerDispWin.u16Height, pstLayerAttr->ePixFormat);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pstLayerAttr);
    //down(&(astDevStatus[astLayerParams[DispLayer].u8BindedDevID].stDevMutex));
    //down(&(astLayerParams[DispLayer].stLayerMutex));
    MI_DISP_GetVarMutex();

    memset(&stHalLayerAttr, 0, sizeof(MHAL_DISP_VideoLayerAttr_t));
    stHalLayerAttr = *((MHAL_DISP_VideoLayerAttr_t*)pstLayerAttr);
    stHalLayerAttr.ePixFormat = pstLayerAttr->ePixFormat;
    stHalLayerAttr.stVidLayerDispWin.u16X = pstLayerAttr->stVidLayerDispWin.u16X;
    stHalLayerAttr.stVidLayerDispWin.u16Y = pstLayerAttr->stVidLayerDispWin.u16Y;
    stHalLayerAttr.stVidLayerDispWin.u16Height = pstLayerAttr->stVidLayerDispWin.u16Height;
    stHalLayerAttr.stVidLayerDispWin.u16Width = pstLayerAttr->stVidLayerDispWin.u16Width;
    stHalLayerAttr.stVidLayerSize.u32Width    = pstLayerAttr->stVidLayerSize.u16Width;
    stHalLayerAttr.stVidLayerSize.u32Height   = pstLayerAttr->stVidLayerSize.u16Height;
    if(!MHAL_DISP_VideoLayerSetAttr((astLayerParams[DispLayer].apLayerObjs), &stHalLayerAttr))
    {
        DISP_DBG_ERR("MHAL_DISP_VideoLayerSetAttr Fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_LAYER_MUTEX;
    }
    // TODO: Tommy move to Disp init
    memcpy(&astLayerParams[DispLayer].stVideoLayerAttr,pstLayerAttr,sizeof(MI_DISP_VideoLayerAttr_t));

    for (i = 0; i < MI_DISP_INPUTPORT_MAX; i++)
    {
        INIT_LIST_HEAD(&astLayerParams[DispLayer].stPortPendingBufQueue[i]);
    }

#if DISP_ENABLE_CUS_ALLOCATOR
   mi_disp_config_layer_canvas(astDevStatus[astLayerParams[DispLayer].u8BindedDevID].hDevSysHandle, mi_disp_TransLayerToDevChnn(astLayerParams[DispLayer].u8BindedDevID, DispLayer), DispLayer, astLayerParams[DispLayer].stVideoLayerAttr.stVidLayerSize.u16Width, astLayerParams[DispLayer].stVideoLayerAttr.stVidLayerSize.u16Height, MI_DISP_CUS_ALLOCATOR_BUF_CNT);
#endif

    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_LAYER_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_GetVideoLayerAttr(MI_DISP_LAYER DispLayer, MI_DISP_VideoLayerAttr_t *pstLayerAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d \n",DispLayer);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pstLayerAttr);
    MI_DISP_GetVarMutex();

    memcpy(pstLayerAttr,&astLayerParams[DispLayer].stVideoLayerAttr,sizeof(MI_DISP_VideoLayerAttr_t));

    s32Ret = MI_SUCCESS;
    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_BindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("DispLayer = %u, DispDev= %u!\n", DispLayer,DispDev );
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
#if 0
    MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    down(&(astLayerParams[DispLayer].stLayerMutex));
#endif
    MI_DISP_GetVarMutex();

    // TODO: Tommy Trans port

    if(!MHAL_DISP_VideoLayerBind((astLayerParams[DispLayer].apLayerObjs), (astDevStatus[DispDev].pstDevObj)))
    {
        DISP_DBG_ERR("Bind Video Layer  Fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_LAYER_MUTEX;
    }
    list_add_tail(&(astLayerParams[DispLayer].stLayerNode), &(astDevStatus[DispDev].stBindedLayer));
    astLayerParams[DispLayer].u8BindedDevID = DispDev;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_LAYER_MUTEX:
    MI_DISP_ReleaseVarMutex();

    if(astDevStatus[DispDev].bSupportIrq == TRUE)
    {
        if(!MHAL_DISP_EnableDevIrq(astDevStatus[DispDev].pstDevObj, astDevStatus[DispDev].u32DevIrq,TRUE))
        {
            s32Ret = E_MI_ERR_FAILED;
            DISP_DBG_ERR("MHAL_DISP_EnableDevIrq Fail!u32DevIrq = %u.\n", astDevStatus[DispDev].u32DevIrq);
        }
        else
        {
            astDevStatus[DispDev].bIrqEnabled = TRUE;
        }
    }
EXIT:
    return s32Ret;
}
MI_S32 MI_DISP_IMPL_UnBindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    struct list_head *pstPos = NULL;
    struct list_head *pTemp = NULL;
    mi_disp_LayerStatus_t *pstTmpLayer = NULL;

    DISP_DBG_ENTER("layer %d, dev %d!\n", DispLayer, DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);

    if(astDevStatus[DispDev].bSupportIrq == TRUE)
    {
        if(!MHAL_DISP_EnableDevIrq(astDevStatus[DispDev].pstDevObj, astDevStatus[DispDev].u32DevIrq,FALSE))
        {
            s32Ret = E_MI_ERR_FAILED;
            DISP_DBG_ERR("MHAL_DISP_DisableDevIrq Fail!u32DevIrq = %u.\n", astDevStatus[DispDev].u32DevIrq);
        }
        else
        {
            astDevStatus[DispDev].bIrqEnabled = FALSE;
        }
    }
#if 0
    MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    down(&(astLayerParams[DispLayer].stLayerMutex));
#endif
    MI_DISP_GetVarMutex();

    if(!MHAL_DISP_VideoLayerUnBind((astLayerParams[DispLayer].apLayerObjs), (astDevStatus[DispDev].pstDevObj)))
    {
        DISP_DBG_ERR("Unbind Video Layer  Fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_LAYER_MUTEX;
    }

    if(!list_empty(&(astDevStatus[DispDev].stBindedLayer)))
    {
        list_for_each_safe(pstPos, pTemp, &(astDevStatus[DispDev].stBindedLayer))
        {
            pstTmpLayer = list_entry(pstPos, mi_disp_LayerStatus_t, stLayerNode);
            if(pstTmpLayer->u8LayerID == DispLayer)
            {
                list_del(&astLayerParams[DispLayer].stLayerNode);
            }
        }
    }

    //Unbinded
    astLayerParams[DispLayer].u8BindedDevID = 0xff;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_LAYER_MUTEX:
    MI_DISP_ReleaseVarMutex();

EXIT:
    return s32Ret;
}

// TODO: Tommy delete Not support

MI_S32 MI_DISP_IMPL_SetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 u32Toleration)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, toleration %d\n",DispLayer, u32Toleration);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    //down(&(astLayerParams[DispLayer].stLayerMutex));
    MI_DISP_GetVarMutex();
    astLayerParams[DispLayer].u32Toleration = u32Toleration;
    s32Ret = MI_SUCCESS;
    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_GetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 *pu32Toleration)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pu32Toleration);
    //down(&(astLayerParams[DispLayer].stLayerMutex));
    MI_DISP_GetVarMutex();
    *pu32Toleration = astLayerParams[DispLayer].u32Toleration;
    s32Ret = MI_SUCCESS;
    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}

typedef void (*DispCaptureCB)(void*);

typedef struct
{
   MI_BOOL bCapDone;;
   wait_queue_head_t stWaitCapDone;
} MI_DISP_CapPrivateData;

// TODO: Tommy, how to specify timeout condition?
void mi_disp_GetCapFrameStatus(void* pPrivateData)
{
    mi_divp_CaptureInfo_t *pstCapInfo = (mi_divp_CaptureInfo_t *)pPrivateData;
    MI_DISP_CapPrivateData* pstCapPrivateData;
    pstCapPrivateData = (MI_DISP_CapPrivateData*)pstCapInfo->pPrivate;
    DISP_DBG_INFO("%s()@line %d: %p.\n", __func__, __LINE__, pstCapPrivateData);
    pstCapPrivateData->bCapDone = TRUE;
    //wake_up_interruptible(&pstCapPrivateData->stWaitCapDone);
    WAKE_UP_QUEUE_IF_NECESSARY(pstCapPrivateData->stWaitCapDone);
}

// TODO: dip 增加一个Dst 的size， buffer 按照Dst sie  申请，和茂光沟通
// TODO: remove timeout


static MI_S32 _mi_disp_ConvertTimingToSize(MI_DISP_OutputTiming_e eOutputTiming, MI_DISP_SyncInfo_t *pstSyncInfo, MI_U16* pu16Width, MI_U16* pu16Height)
{
    MI_S32 s32Ret = MI_DISP_FAIL;
    MI_U16 u16Width = 0;
    MI_U16 u16Height = 0;
    DISP_DBG_INFO("\n %s :%d cys eOutputTiming = %u, * pu16Width = %u,  * pu16Height = %u\n", __FUNCTION__, __LINE__,
        eOutputTiming, *pu16Width,  *pu16Height);
    switch(eOutputTiming)
    {
        case E_MI_DISP_OUTPUT_PAL:
            u16Width = 720;
            u16Height = 576;
            break;
        case E_MI_DISP_OUTPUT_NTSC:
            u16Width = 720;
            u16Height = 480;
            break;
        case E_MI_DISP_OUTPUT_960H_PAL:
            u16Width = 960;
            u16Height = 576;
            break;
        case E_MI_DISP_OUTPUT_960H_NTSC:
            u16Width = 960;
            u16Height = 480;
            break;

        case E_MI_DISP_OUTPUT_480i60:
        case E_MI_DISP_OUTPUT_480P60:
            u16Width = 720;
            u16Height = 480;
            break;
        case E_MI_DISP_OUTPUT_576i50:
        case E_MI_DISP_OUTPUT_576P50:
            u16Width = 720;
            u16Height = 576;
            break;
        case E_MI_DISP_OUTPUT_720P50:
        case E_MI_DISP_OUTPUT_720P60:
            u16Width = 1280;
            u16Height = 720;
            break;
        case E_MI_DISP_OUTPUT_1080P24:
        case E_MI_DISP_OUTPUT_1080P25:
        case E_MI_DISP_OUTPUT_1080P30:
        case E_MI_DISP_OUTPUT_1080I50:
        case E_MI_DISP_OUTPUT_1080I60:
        case E_MI_DISP_OUTPUT_1080P50:
        case E_MI_DISP_OUTPUT_1080P60:
            u16Width = 1920;
            u16Height = 1080;
            break;
        case E_MI_DISP_OUTPUT_640x480_60:
            u16Width = 640;
            u16Height = 480;
            break;
        case E_MI_DISP_OUTPUT_800x600_60:
            u16Width = 800;
            u16Height = 600;
            break;
        case E_MI_DISP_OUTPUT_1024x768_60:
            u16Width = 1024;
            u16Height = 768;
            break;
        case E_MI_DISP_OUTPUT_1280x1024_60:
            u16Width = 1280;
            u16Height = 1024;
            break;
        case E_MI_DISP_OUTPUT_1366x768_60:
            u16Width = 1366;
            u16Height = 768;
            break;
        case E_MI_DISP_OUTPUT_1440x900_60:
            u16Width = 1440;
            u16Height = 900;
            break;
        case E_MI_DISP_OUTPUT_1280x800_60:
            u16Width = 1280;
            u16Height = 800;
            break;
        case E_MI_DISP_OUTPUT_1680x1050_60:
            u16Width = 1680;
            u16Height = 1050;
            break;
        case E_MI_DISP_OUTPUT_1920x2160_30:
            u16Width = 1920;
            u16Height = 2160;
            break;
        case E_MI_DISP_OUTPUT_1600x1200_60:
            u16Width = 1600;
            u16Height = 1200;
            break;
        case E_MI_DISP_OUTPUT_1920x1200_60:
            u16Width = 1920;
            u16Height = 1200;
            break;
        case E_MI_DISP_OUTPUT_2560x1440_30:
            u16Width = 2560;
            u16Height = 1440;
            break;
        case E_MI_DISP_OUTPUT_2560x1600_60:
            u16Width = 2560;
            u16Height = 1600;
            break;
        case E_MI_DISP_OUTPUT_3840x2160_30:
        case E_MI_DISP_OUTPUT_3840x2160_60:
            u16Width = 3840;
            u16Height = 2160;
            break;
        case E_MI_DISP_OUTPUT_USER:
            u16Width = pstSyncInfo->u16Hpw;
            u16Height = pstSyncInfo->u16Vpw;
            break;
        default:
            u16Width = 0;
            u16Height = 0;
    }
    *pu16Width = u16Width;
    *pu16Height = u16Height;

    DISP_DBG_INFO("\n %s :%d cys eOutputTiming = %u, * pu16Width = %u,  * pu16Height = %u\n", __FUNCTION__, __LINE__,
        eOutputTiming, *pu16Width,  *pu16Height);    s32Ret = MI_DISP_SUCCESS;
    return s32Ret;
}


MI_S32 MI_DISP_IMPL_GetScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    mi_divp_CaptureInfo_t stCapInfo;
    MI_DISP_CapPrivateData stCapPrivateData;
    MI_DISP_OutputTiming_e eOutputTiming = E_MI_DISP_OUTPUT_MAX;
    MI_DISP_SyncInfo_t stSyncInfo;

    DISP_DBG_ENTER("layer %d!\n",DispLayer);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pstVFrame);
    memset(&stCapPrivateData, 0, sizeof(MI_DISP_CapPrivateData));
    init_waitqueue_head(&(stCapPrivateData.stWaitCapDone));
    memset(&stCapInfo, 0, sizeof(mi_divp_CaptureInfo_t));
    memset(&stSyncInfo, 0, sizeof(stSyncInfo));
    stCapInfo.eInputPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stCapInfo.eOutputPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    stCapInfo.eCapStage = E_MI_DIVP_CAP_STAGE_OUTPUT_WITH_OSD;
    stCapInfo.eDispId = astLayerParams[DispLayer].u8BindedDevID;
    stCapInfo.u16Height = pstVFrame->u32Height;
    stCapInfo.u16Width = pstVFrame->u32Width;
    stCapInfo.phyBufAddr[0] = pstVFrame->aphyAddr;
    stCapInfo.u32BufSize = pstVFrame->u32Size;
    stCapInfo.pPrivate = (void *)(&stCapPrivateData);
    stCapInfo.stCropWin.u16X = 0;
    stCapInfo.stCropWin.u16Y = 0;

    mi_disp_impl_GetOutputTiming(DispLayer, &eOutputTiming,  &stSyncInfo);
    _mi_disp_ConvertTimingToSize(eOutputTiming, &stSyncInfo, &stCapInfo.stCropWin.u16Width, &stCapInfo.stCropWin.u16Height);

    stCapInfo.u16Stride[0] = pstVFrame->u32Stride;//pstVFrame->u32Width * 2;
    stCapInfo.pfnDispCallBack = mi_disp_GetCapFrameStatus;
    stCapInfo.pPrivate = &stCapPrivateData;

    mi_divp_Init();
    //stCapInfo.u32TimeOut = u32MilliSec;
    DISP_DBG_INFO("%s()@line %d: %p.\n", __func__, __LINE__, &stCapPrivateData);
    if(MI_SUCCESS != mi_divp_CaptureTiming(&stCapInfo))
    {
        DISP_DBG_ERR("Divp Cap Timing fail!!!\n");
        goto EXIT;
    }

    wait_event(stCapPrivateData.stWaitCapDone, stCapPrivateData.bCapDone == TRUE);

    if (stCapInfo.bRetVal == TRUE)
    {
        s32Ret = MI_SUCCESS;
    }
    DISP_DBG_EXITOK();
EXIT:
    mi_divp_DeInit();
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_ReleaseScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer %d!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pstVFrame);
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}

static MI_S32 _MI_DISP_EnableInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_DEV DispDev = 0;
    MI_U32 u32ChnId = 0;

    DISP_DBG_INFO("DispLayer = %d,  LayerInputPort = %d. \n", DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    DispDev = astLayerParams[DispLayer].u8BindedDevID;
    u32ChnId = mi_disp_TransLayerToDevChnn(DispDev, DispLayer);

    s32Ret = mi_sys_EnableInputPort(astDevStatus[DispDev].hDevSysHandle, u32ChnId, LayerInputPort);
    if(s32Ret != MI_SUCCESS)
    {
        DISP_DBG_ERR("mi_sys_EnableInputPort Fail!!!\n");
    }
    else
    {
        astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable = TRUE;
        astLayerParams[DispLayer].astPortStatus[LayerInputPort].bFirstFrame = TRUE;

#if DISP_ENABLE_CUS_ALLOCATOR
        mi_sys_SetInputPortUsrAllocator(astDevStatus[DispDev].hDevSysHandle, u32ChnId, LayerInputPort, mi_disp_layer_get_cus_allocator(DispLayer, LayerInputPort));
#endif
        DISP_DBG_INFO("bFirstFrame = %u\n", astLayerParams[DispLayer].astPortStatus[LayerInputPort].bFirstFrame);
    }

EXIT:
    return s32Ret;
}

static MI_S32 _MI_DISP_DisableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_DEV DispDev = 0;
    MI_U32 u32ChnId = 0;
    void* tmpObjs = NULL;
    DISP_DBG_INFO("DispLayer = %d,  LayerInputPort = %d. \n", DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));

    DispDev = astLayerParams[DispLayer].u8BindedDevID;
    u32ChnId = mi_disp_TransLayerToDevChnn(DispDev, DispLayer);
    mi_sys_DisableInputPort(astDevStatus[DispDev].hDevSysHandle, u32ChnId, LayerInputPort);

    if(!MHAL_DISP_InputPortEnable((astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs), FALSE))
    {
        DISP_DBG_ERR("MHAL_DISP_InputPortEnable Fail!!!\n");
    }
    else
    {
        // TODO: Shoke Need finish it, Finish buffer
        if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable)
        {
            //reset global variables
            tmpObjs = astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs;
            memset(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort]), 0, sizeof(mi_disp_InputPortStatus_t));
            astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs = tmpObjs;

            //finish all buffer of this input port
            if(NULL != astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort])
            {
                mi_sys_FinishBuf(astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort]);
                astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort] = NULL;
            }
            if(NULL != astLayerParams[DispLayer].pstCurrentFiredBufInfo[LayerInputPort])
            {
                mi_sys_FinishBuf(astLayerParams[DispLayer].pstCurrentFiredBufInfo[LayerInputPort]);
                astLayerParams[DispLayer].pstCurrentFiredBufInfo[LayerInputPort] = NULL;
            }

            down(&(astLayerParams[DispLayer].stDispLayerPendingQueueMutex));
            if(!list_empty(&astLayerParams[DispLayer].stPortPendingBufQueue[LayerInputPort]))
            {
                struct list_head *pstPendingBufferPos = NULL;
                struct list_head *n = NULL;
                mi_disp_PortPendingBuf_t *pstTmpPendingBuf;
                MI_SYS_BufInfo_t* pstTmpInputBuffer;
                list_for_each_safe(pstPendingBufferPos, n, &(astLayerParams[DispLayer].stPortPendingBufQueue[LayerInputPort]))
                {
                    pstTmpPendingBuf = list_entry(pstPendingBufferPos, mi_disp_PortPendingBuf_t, stPortPendingBufNode);
                    list_del(pstPendingBufferPos);
                    pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                    memset(pstTmpPendingBuf, 0xF2, sizeof(mi_disp_PortPendingBuf_t));
                    kfree(pstTmpPendingBuf);
                    if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_SUCCESS)
                    {
                        DISP_DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                    }
                }
            }
            else
            {
                DISP_DBG_INFO("input port has been disable before!\n");
            }
            up(&(astLayerParams[DispLayer].stDispLayerPendingQueueMutex));
        }

#if DISP_ENABLE_CUS_ALLOCATOR
        mi_sys_SetInputPortUsrAllocator(astDevStatus[astLayerParams[DispLayer].u8BindedDevID].hDevSysHandle, u32ChnId, LayerInputPort, NULL);
#endif
        s32Ret = MI_SUCCESS;
    }

EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_SetVideoLayerAttrBegin(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U32 u32PortId = 0;
    DISP_DBG_ENTER(" DispLayer = %d.\n", DispLayer);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    // down(&(astLayerParams[DispLayer].stLayerMutex));

    MI_DISP_GetVarMutex();
    if(!MHAL_DISP_InputPortAttrBegin((astLayerParams[DispLayer].apLayerObjs)))
    {
        DISP_DBG_ERR("MHAL_DISP_InputPortAttrBegin Fail!!!\n");
    }
    else
    {
        s32Ret = MI_SUCCESS;
        for(u32PortId = 0; u32PortId < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u32PortId ++)
        {
            if(astLayerParams[DispLayer].astPortStatus[u32PortId].bEnable)
            {
                if(MI_SUCCESS == _MI_DISP_DisableInputPort(DispLayer, u32PortId))
                {
                    astLayerParams[DispLayer].astPortStatus[u32PortId].bDisabledInside = TRUE;
                }
                else
                {
                    DISP_DBG_ERR("_MI_DISP_DisableInputPort Fail!!!\n");
                    s32Ret = E_MI_ERR_FAILED;
                }
            }
        }
    }

    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}
// TODO: 在doc 中描述使用Case
MI_S32 MI_DISP_IMPL_SetVideoLayerAttrEnd(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U32 u32PortId = 0;
    DISP_DBG_ENTER(" DispLayer = %d.\n", DispLayer);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);

    MI_DISP_GetVarMutex();
    if(!MHAL_DISP_InputPortAttrEnd((astLayerParams[DispLayer].apLayerObjs)))
    {
        DISP_DBG_ERR("MHAL_DISP_InputPortAttrEnd Fail!!!\n");
    }
    else
    {
        s32Ret = MI_SUCCESS;
        for(u32PortId = 0; u32PortId < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u32PortId ++)
        {
            if(astLayerParams[DispLayer].astPortStatus[u32PortId].bDisabledInside)
            {
                if(MI_SUCCESS == _MI_DISP_EnableInputPort(DispLayer, u32PortId))
                {
                    astLayerParams[DispLayer].astPortStatus[u32PortId].bDisabledInside = FALSE;
                }
                else
                {
                    DISP_DBG_ERR("_MI_DISP_EnableInputPort Fail!!!\n");
                    s32Ret = E_MI_ERR_FAILED;
                }
            }
        }
    }
    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_SetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_InputPortAttr_t *pstInputPortAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_InputPortAttr_t stHalInputPortAttr;
    //MI_U64 u64SidebandMsg = 0;
    DISP_DBG_ENTER("layer%d, port%d, port(%d,%d,%d,%d)!\n", DispLayer, LayerInputPort, pstInputPortAttr->stDispWin.u16X,pstInputPortAttr->stDispWin.u16Y,
            pstInputPortAttr->stDispWin.u16Width, pstInputPortAttr->stDispWin.u16Height);
    
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstInputPortAttr);

    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();
    if(pstInputPortAttr->stDispWin.u16X + pstInputPortAttr->stDispWin.u16Width > astLayerParams[DispLayer].stVideoLayerAttr.stVidLayerSize.u16Width)
    {
        DISP_DBG_ERR("port%d, X(%d)+width(%d)>layersize.width(%d)!!!\n",LayerInputPort, pstInputPortAttr->stDispWin.u16X, pstInputPortAttr->stDispWin.u16Width, astLayerParams[DispLayer].stVideoLayerAttr.stVidLayerSize.u16Width);
        s32Ret = E_MI_ERR_FAILED;
        goto UP_PORT_MUTEX;
    }

    if(pstInputPortAttr->stDispWin.u16Y + pstInputPortAttr->stDispWin.u16Height > astLayerParams[DispLayer].stVideoLayerAttr.stVidLayerSize.u16Height)
    {
        DISP_DBG_ERR("port%d, Y(%d)+height(%d)>layersize.height(%d)!!!\n",LayerInputPort, pstInputPortAttr->stDispWin.u16Y, pstInputPortAttr->stDispWin.u16Height, astLayerParams[DispLayer].stVideoLayerAttr.stVidLayerSize.u16Height);
        s32Ret = E_MI_ERR_FAILED;
        goto UP_PORT_MUTEX;
    }
    memset(&stHalInputPortAttr, 0, sizeof(MHAL_DISP_InputPortAttr_t));
    stHalInputPortAttr.stDispWin.u16X = pstInputPortAttr->stDispWin.u16X;
    stHalInputPortAttr.stDispWin.u16Y = pstInputPortAttr->stDispWin.u16Y;
    stHalInputPortAttr.stDispWin.u16Height = pstInputPortAttr->stDispWin.u16Height;
    stHalInputPortAttr.stDispWin.u16Width = pstInputPortAttr->stDispWin.u16Width;
    DISP_DBG_INFO("[%s %d]Inport Attr [%d, %d, %d, %d]  \n", __FUNCTION__, __LINE__, stHalInputPortAttr.stDispWin.u16X,
        stHalInputPortAttr.stDispWin.u16Y, stHalInputPortAttr.stDispWin.u16Width, stHalInputPortAttr.stDispWin.u16Height);

    if(!MHAL_DISP_InputPortSetAttr((astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs), &stHalInputPortAttr))
    {
        DISP_DBG_ERR("MHAL_DISP_InputPortSetAttr!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_PORT_MUTEX;
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin = pstInputPortAttr->stDispWin;
#if DISP_ENABLE_CUS_ALLOCATOR
    mi_disp_layer_port_set_winrect(DispLayer, LayerInputPort, &astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin);
#endif

#if 0 //This will force set peer output MST 420
    if ((astLayerParams[DispLayer].astPortStatus[LayerInputPort].stCropWin.u16Width == 0) && (astLayerParams[DispLayer].astPortStatus[LayerInputPort].stCropWin.u16Height == 0))
    {
        u64SidebandMsg = MI_SYS_MAKE_SIDEBAND_PREFER_CROP_MSG(0, 0, pstInputPortAttr->stDispWin.u16Width, pstInputPortAttr->stDispWin.u16Height, E_MI_SYS_PIXEL_FRAME_YUV_MST_420);
    }
    mi_sys_SetInputPortSidebandMsg(astDevStatus[astLayerParams[DispLayer].u8BindedDevID].hDevSysHandle, DispLayer, LayerInputPort, u64SidebandMsg);
#endif
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_PORT_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_GetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_InputPortAttr_t *pstInputPortAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, port %d!\n",DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstInputPortAttr);

    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();
    memcpy(&pstInputPortAttr->stDispWin, &astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin, sizeof(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin));
    s32Ret = MI_SUCCESS;
    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}


MI_S32 MI_DISP_IMPL_EnableInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    DISP_DBG_ENTER("layer%d, port%d !\n",DispLayer,LayerInputPort);
    MI_DISP_GetVarMutex();
    s32Ret = _MI_DISP_EnableInputPort (DispLayer, LayerInputPort);



    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bDisabledInside)
    {
        astLayerParams[DispLayer].astPortStatus[LayerInputPort].bDisabledInside = FALSE;
    }
    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_DisableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, port%d\n", DispLayer, LayerInputPort);

    MI_DISP_GetVarMutex();
    s32Ret = _MI_DISP_DisableInputPort(DispLayer, LayerInputPort);
    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bDisabledInside)
    {
        astLayerParams[DispLayer].astPortStatus[LayerInputPort].bDisabledInside = FALSE;
    }
    MI_DISP_ReleaseVarMutex();

    DISP_DBG_EXITOK();
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_SetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_Position_t *pstDispPos)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_InputPortAttr_t stHalInputPortAttr;
    DISP_DBG_ENTER("layer%d, port%d, port(%d,%d,%d,%d)!\n", DispLayer, LayerInputPort, pstDispPos->u16X,pstDispPos->u16Y);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstDispPos);
    memset(&stHalInputPortAttr, 0, sizeof(MHAL_DISP_InputPortAttr_t));
    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();
    memcpy(&stHalInputPortAttr.stDispWin, &astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin, sizeof(stHalInputPortAttr.stDispWin));
    stHalInputPortAttr.stDispWin.u16X = pstDispPos->u16X;
    stHalInputPortAttr.stDispWin.u16Y = pstDispPos->u16Y;
    if(!MHAL_DISP_InputPortSetAttr((astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs), &stHalInputPortAttr))
    {
        DISP_DBG_ERR("MHAL_DISP_InputPortSetAttr Fail!!!\n");
        goto UP_PORT_MUTEX;
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin.u16X = pstDispPos->u16X;
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin.u16Y = pstDispPos->u16Y;
#if DISP_ENABLE_CUS_ALLOCATOR
    mi_disp_layer_port_set_winrect(DispLayer, LayerInputPort, &astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin);
#endif
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_PORT_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_GetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_Position_t *pstDispPos)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, port%d!\n", DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstDispPos);
    MI_DISP_GetVarMutex();

    pstDispPos->u16X = astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin.u16X;
    pstDispPos->u16Y = astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin.u16Y;
    s32Ret = MI_SUCCESS;
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_PauseInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, port %d!\n", DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();
    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable != TRUE)
    {
        DISP_DBG_ERR("Input port Not enable!!!\n");
        goto UP_PORT_MUTEX;
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].bPause = TRUE;
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eStatus = E_MI_LAYER_INPUTPORT_STATUS_PAUSE;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_PORT_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_ResumeInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("Layer%d, port%d!\n", LayerInputPort, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();
    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable != TRUE)
    {
        DISP_DBG_ERR("Input port Not enable!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_PORT_MUTEX;
    }

    astLayerParams[DispLayer].astPortStatus[LayerInputPort].bPause = FALSE;
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eMode = astLayerParams[DispLayer].astPortStatus[LayerInputPort].eUserSetSyncMode;
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eStatus = E_MI_LAYER_INPUTPORT_STATUS_RESUME;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_PORT_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_StepInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, port%d!\n",DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();
    // TODO: Shoke: PTS force freerun.
    DISP_DBG_INFO("StepInputPort PTS will force freerun.\n");
    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable != TRUE)
    {
        DISP_DBG_ERR("Input port Not enable!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_PORT_MUTEX;
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eStatus = E_MI_LAYER_INPUTPORT_STATUS_STEP;
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eMode = E_MI_DISP_SYNC_MODE_FREE_RUN;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_PORT_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_ShowInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d port %d!\n", DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);

    // TODO: 增加flow，有合适buffer 去打开Gwin
    // TODO: Tommy 记录软件状态， 在isr 中处理，直接返回
    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();
    while(astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort] != NULL)
    {
        if(FALSE == MHAL_DISP_InputPortShow((astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs)))
        {
            DISP_DBG_ERR("MHAL_DISP_InputPortShow Fail!!!\n");
            s32Ret = E_MI_ERR_FAILED;
            goto UP_PORT_MUTEX;
        }
        else
        {
            break;
        }
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eStatus = E_MI_LAYER_INPUTPORT_STATUS_SHOW;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_PORT_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_HideInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, port%d!\n", DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_GetVarMutex();

    if(!MHAL_DISP_InputPortHide((astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs)))
    {
        DISP_DBG_ERR("MHAL_DISP_InputPortHide Fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_PORT_MUTEX;
    }

    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eStatus = E_MI_LAYER_INPUTPORT_STATUS_HIDE;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_PORT_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_U64  MI_DISP_GetInputPortPts(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_U64 *pu64InputPortPts)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, port%d!\n",DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pu64InputPortPts);

    // TODO: last frame pts
    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();
    if(astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort] != NULL)
    {
        *pu64InputPortPts = astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort]->u64Pts;
    }
    s32Ret = MI_SUCCESS;
    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_SetInputPortSyncMode (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_SyncMode_e eMode)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, port%d mode %d!\n", DispLayer, LayerInputPort, eMode);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);

    if((eMode <= E_MI_DISP_SYNC_MODE_INVALID) || (eMode >= E_MI_DISP_SYNC_MODE_NUM))
    {
        DISP_DBG_ERR("Invaild sync mode!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }
    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();
    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable != TRUE)
    {
        DISP_DBG_ERR("Input port Not enable!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_PORT_MUTEX;
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eMode = eMode;
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eUserSetSyncMode = eMode;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_PORT_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_QueryInputPortStat(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_QueryChannelStatus_t *pstStatus)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer%d, port%d!\n", DispLayer, LayerInputPort);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstStatus);
    MI_DISP_GetVarMutex();

    // TODO: Tommy   Add MI_DISP_InputPortStatus_e eStatus; -- Done
    pstStatus->bEnable = astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable;
    pstStatus->eStatus = astLayerParams[DispLayer].astPortStatus[LayerInputPort].eStatus;
    s32Ret = MI_SUCCESS;
    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_SetZoomInWindow(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_VidWinRect_t* pstCropWin)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U64 u64SidebandMsg;
    MI_DISP_DEV DispDev;

    DISP_DBG_ENTER("layer%d, port%d, crop(%d,%d,%d,%d)!\n", DispLayer, LayerInputPort, pstCropWin->u16X,pstCropWin->u16Y,pstCropWin->u16Width,pstCropWin->u16Height);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstCropWin);

    MI_DISP_GetVarMutex();
    // TODO: Tommy Add check conditions -- Done
    if ((pstCropWin->u16X == 0) &&(pstCropWin->u16Y == 0) && (pstCropWin->u16Width == 0) &&(pstCropWin->u16Height == 0))
    {
        s32Ret = MI_ERR_DISP_ILLEGAL_PARAM;
        DISP_DBG_EXITERR();
    }
    else
    {
        DBG_INFO("Layer = %d, port = %d, CropWin[%d, %d, %d, %d]\n", DispLayer, LayerInputPort, pstCropWin->u16X, pstCropWin->u16Y, pstCropWin->u16Width, pstCropWin->u16Height);
        astLayerParams[DispLayer].astPortStatus[LayerInputPort].stCropWin = *pstCropWin;
        s32Ret = MI_SUCCESS;
    }
    MI_SYS_BUG_ON(MI_DISP_LAYER_MAX <= DispLayer || 0> DispLayer);
    DispDev = astLayerParams[DispLayer].u8BindedDevID;
    MI_SYS_BUG_ON(MI_DISP_DEV_MAX <= DispDev || 0> DispDev);

    if(pstCropWin->u16X || pstCropWin->u16Y||
        pstCropWin->u16Width != astLayerParams[DispLayer].astPortStatus[LayerInputPort]. stDispWin.u16Width||
        pstCropWin->u16Height != astLayerParams[DispLayer].astPortStatus[LayerInputPort]. stDispWin.u16Height)
         u64SidebandMsg = MI_SYS_MAKE_SIDEBAND_PREFER_CROP_MSG(pstCropWin->u16X, pstCropWin->u16Y, pstCropWin->u16Width, pstCropWin->u16Height, E_MI_SYS_PIXEL_FRAME_YUV_MST_420);
    else
         u64SidebandMsg = MI_SYS_SIDEBAND_MSG_NULL;

    mi_sys_SetInputPortSidebandMsg(astDevStatus[DispDev].hDevSysHandle, DispLayer, LayerInputPort, u64SidebandMsg);

    MI_DISP_ReleaseVarMutex();
    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}

// TODO: Tommy. Not verify yet on K6L. It depend on I2.
MI_S32 MI_DISP_IMPL_GetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("DEV%d !!!\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstVgaParam);
    //MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    MI_DISP_GetVarMutex();

    if((astDevStatus[DispDev].u32Interface & (1 <<E_MI_DISP_INTF_VGA)) == 0)
    {
        DISP_DBG_ERR("Not Support Vga output device In device %d\n!!!", DispDev);
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }
    pstVgaParam->stCsc.eCscMatrix = astDevStatus[DispDev].eCscMatrix;
    pstVgaParam->stCsc.u32Luma = astDevStatus[DispDev].u32Luma;
    pstVgaParam->stCsc.u32Contrast = astDevStatus[DispDev].u32Contrast;
    pstVgaParam->stCsc.u32Hue = astDevStatus[DispDev].u32Hue;
    pstVgaParam->stCsc.u32Saturation = astDevStatus[DispDev].u32Saturation;
    pstVgaParam->u32Gain = astDevStatus[DispDev].u32Gain;
    pstVgaParam->u32Sharpness = astDevStatus[DispDev].u32Sharpness;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_SetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_VgaParam_t stHalVgaParams;
    DISP_DBG_ENTER("Dev%d CscMatrix %d, Contrast %d, Hue %d, Luma %d, Saturation %d\n", DispDev, pstVgaParam->stCsc.eCscMatrix, pstVgaParam->stCsc.u32Contrast, pstVgaParam->stCsc.u32Hue, pstVgaParam->stCsc.u32Luma, pstVgaParam->stCsc.u32Saturation);
    DISP_DBG_ENTER("Dev%d Gain %d, Sharpness %d\n", DispDev, pstVgaParam->u32Gain , pstVgaParam->u32Sharpness);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstVgaParam);
    //MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    MI_DISP_GetVarMutex();

    if ((astDevStatus[DispDev].u32Interface & (1 <<E_MI_DISP_INTF_VGA)) == 0)
    {
        DISP_DBG_ERR("Not Support Vga output device In device %d\n!!!", DispDev);
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }

    memset(&stHalVgaParams, 0, sizeof(MHAL_DISP_VgaParam_t));
    stHalVgaParams.stCsc.eCscMatrix = pstVgaParam->stCsc.eCscMatrix;
    stHalVgaParams.stCsc.u32Contrast = pstVgaParam->stCsc.u32Contrast;
    stHalVgaParams.stCsc.u32Hue = pstVgaParam->stCsc.u32Hue;
    stHalVgaParams.stCsc.u32Luma = pstVgaParam->stCsc.u32Luma;
    stHalVgaParams.stCsc.u32Saturation = pstVgaParam->stCsc.u32Saturation;
    stHalVgaParams.u32Gain = pstVgaParam->u32Gain;
    stHalVgaParams.u32Sharpness = pstVgaParam->u32Sharpness;
    if(!(MHAL_DISP_DeviceSetVgaParam((astDevStatus[DispDev].pstDevObj), &stHalVgaParams)))
    {
        DISP_DBG_ERR("MHAL_DISP_DeviceSetVgaParam Fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].eCscMatrix = pstVgaParam->stCsc.eCscMatrix;
    astDevStatus[DispDev].u32Luma = pstVgaParam->stCsc.u32Luma;
    astDevStatus[DispDev].u32Contrast = pstVgaParam->stCsc.u32Contrast;
    astDevStatus[DispDev].u32Hue = pstVgaParam->stCsc.u32Hue;
    astDevStatus[DispDev].u32Saturation = pstVgaParam->stCsc.u32Saturation;
    astDevStatus[DispDev].u32Gain = pstVgaParam->u32Gain;
    astDevStatus[DispDev].u32Sharpness = pstVgaParam->u32Sharpness;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

// TODO:Tommy need verify
// 补充k6l hal  ACE
MI_S32 MI_DISP_IMPL_GetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("Dev %d !!!\n",DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstHdmiParam);
    //MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    MI_DISP_GetVarMutex();

    if ((astDevStatus[DispDev].u32Interface & (1 <<E_MI_DISP_INTF_HDMI)) == 0)
    {
        DISP_DBG_ERR("Not Support HDMI output device In device %d\n!!!", DispDev);
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }

    pstHdmiParam->stCsc.eCscMatrix = astDevStatus[DispDev].eCscMatrix;
    pstHdmiParam->stCsc.u32Luma = astDevStatus[DispDev].u32Luma;
    pstHdmiParam->stCsc.u32Contrast = astDevStatus[DispDev].u32Contrast;
    pstHdmiParam->stCsc.u32Hue = astDevStatus[DispDev].u32Hue;
    pstHdmiParam->stCsc.u32Saturation = astDevStatus[DispDev].u32Saturation;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_SetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_HdmiParam_t stHalHdmiParams;
    DISP_DBG_ENTER("Dev%d CscMatrix %d, Contrast %d, Hue %d, Luma %d, Saturation %d\n", DispDev, pstHdmiParam->stCsc.eCscMatrix, pstHdmiParam->stCsc.u32Contrast,
        pstHdmiParam->stCsc.u32Hue, pstHdmiParam->stCsc.u32Luma, pstHdmiParam->stCsc.u32Saturation);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstHdmiParam);
    //MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    MI_DISP_GetVarMutex();

    if ((astDevStatus[DispDev].u32Interface & (1 <<E_MI_DISP_INTF_HDMI)) == 0)
    {
        DISP_DBG_ERR("Not Support HDMI output device In device %d\n!!!", DispDev);
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }

    memset(&stHalHdmiParams, 0, sizeof(MHAL_DISP_HdmiParam_t));
    if ((pstHdmiParam->stCsc.u32Contrast > 100) || (pstHdmiParam->stCsc.u32Hue > 100) ||
        (pstHdmiParam->stCsc.u32Luma > 100) || pstHdmiParam->stCsc.u32Saturation > 100)
    {
        DISP_DBG_ERR("Param over range u32Contrast(%d) u32Hue(%d) u32Luma(%d) u32Saturation(%d)!!!\n",
            pstHdmiParam->stCsc.u32Contrast, pstHdmiParam->stCsc.u32Hue,
            pstHdmiParam->stCsc.u32Luma, pstHdmiParam->stCsc.u32Saturation);
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }
    stHalHdmiParams.stCsc.eCscMatrix = pstHdmiParam->stCsc.eCscMatrix;
    stHalHdmiParams.stCsc.u32Contrast = pstHdmiParam->stCsc.u32Contrast;
    stHalHdmiParams.stCsc.u32Hue = pstHdmiParam->stCsc.u32Hue;
    stHalHdmiParams.stCsc.u32Luma = pstHdmiParam->stCsc.u32Luma;
    stHalHdmiParams.stCsc.u32Saturation = pstHdmiParam->stCsc.u32Saturation;
    if(!(MHAL_DISP_DeviceSetHdmiParam((astDevStatus[DispDev].pstDevObj), &stHalHdmiParams)))
    {
        DISP_DBG_ERR("MHAL_DISP_DeviceSetHdmiParam!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].eCscMatrix = pstHdmiParam->stCsc.eCscMatrix;
    astDevStatus[DispDev].u32Luma = pstHdmiParam->stCsc.u32Luma;
    astDevStatus[DispDev].u32Contrast = pstHdmiParam->stCsc.u32Contrast;
    astDevStatus[DispDev].u32Hue = pstHdmiParam->stCsc.u32Hue;
    astDevStatus[DispDev].u32Saturation = pstHdmiParam->stCsc.u32Saturation;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_GetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("Dev %d !!!\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstCvbsParam);
    //MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    MI_DISP_GetVarMutex();

    pstCvbsParam->bEnable = astDevStatus[DispDev].bCvbsEnable;
    s32Ret = MI_SUCCESS;
    MI_DISP_ReleaseVarMutex();

    DISP_DBG_EXITOK();
EXIT:
    return s32Ret;
}

MI_S32 MI_DISP_IMPL_SetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_CvbsParam_t stHalCvbsParams;
    DISP_DBG_ENTER("Dev %d !!!\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstCvbsParam);
    //MI_DISP_DOWN(&(astDevStatus[DispDev].stDevMutex));
    MI_DISP_GetVarMutex();

    memset(&stHalCvbsParams, 0, sizeof(MI_DISP_CvbsParam_t));
    stHalCvbsParams.bEnable = pstCvbsParam->bEnable;
    if(!(MHAL_DISP_DeviceSetCvbsParam((astDevStatus[DispDev].pstDevObj), &stHalCvbsParams)))
    {
        DISP_DBG_ERR("MHAL_DISP_DeviceSetCvbsParam Fail!!!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].bCvbsEnable = pstCvbsParam->bEnable;
    s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_DEV_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

// TODO: Tommy  REmove : MI_DISP_IMPL_FrameFlip-- Done
MI_BOOL mi_display_SuitableDispWin(MI_SYS_BufInfo_t* pstFrameBufInfo, MI_DISP_VidWinRect_t *pstDispWin)
{
    MI_BOOL bSuitable = FALSE;
    mi_vdec_DispFrame_t *pstVdecFrameInfo;

    if((pstFrameBufInfo == NULL) || (pstDispWin == NULL))
    {
        DISP_DBG_ERR("NULL Pointer, mi_display_SuitableDispWin!!!\n");
        return FALSE;
    }
    if(pstFrameBufInfo->eBufType == E_MI_SYS_BUFDATA_FRAME)
    {
        bSuitable = (pstFrameBufInfo->stFrameData.u16Width == pstDispWin->u16Width)
            && (pstFrameBufInfo->stFrameData.u16Height == pstDispWin->u16Height);
    }
    else if(pstFrameBufInfo->eBufType == E_MI_SYS_BUFDATA_META)
    {

        pstVdecFrameInfo = (mi_vdec_DispFrame_t*)pstFrameBufInfo->stMetaData.pVirAddr;
        if(pstVdecFrameInfo->ePixelFrm == E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE1_H264)
        {
            pstDispWin->u16Height = ALIGN_UP(pstDispWin->u16Height,16);
        }
        bSuitable = (pstVdecFrameInfo->stFrmInfo.u16Width == pstDispWin->u16Width)
            && (pstVdecFrameInfo->stFrmInfo.u16Height == pstDispWin->u16Height);
    }
    return bSuitable;
}

#define DISP_PTS_PRINT(fmt, args ...)

MI_U64 mi_display_ResetPts(MI_U64 u64FiredTimeStamp, MI_U64 u64FramePts, MI_BOOL* bBefore, MI_U32 u32VsyncInterval)
{
    MI_U64 u64Diff = 0;
    if(bBefore == NULL)
    {
        DISP_DBG_ERR("NULL Pointer, mi_display_CalDiffPts!!!\n");
        return FALSE;
    }

    if(u64FiredTimeStamp > u64FramePts)
    {
        u64Diff = u64FiredTimeStamp - u64FramePts;
        *bBefore = TRUE;
    }
    else
    {
        u64Diff = u64FramePts - u64FiredTimeStamp;
        *bBefore = FALSE;
    }

    //DISP_PTS_PRINT("%s()@line %d, last u64FiredTimeStamp = %llu, last frame = %lld !!!\n", __func__, __LINE__, u64FiredTimeStamp, u64FramePts);
    DISP_DBG_WARN("Expect Fired pts: %llu, Cur frame pts: %lld u64Diff: %lld, pstDevStatus->u3264VsyncInterval: %u expect before: %d!!!\n", u64FiredTimeStamp, u64FramePts, u64Diff, u32VsyncInterval, *bBefore);

    return u64Diff;
}

MI_DISP_HandleFrame_e mi_display_CheckInputPortPts(MI_SYS_BufInfo_t* pstFrameBuf, mi_disp_InputPortStatus_t * pstPortStatus, mi_disp_DevStatus_t* pstDevStatus, MI_U32 TolerationMs)
{
    MI_U64 u64NextPts = 0;
    MI_DISP_HandleFrame_e eHandlePts = E_MI_DISP_FRAME_NORMAL;
    DISP_PTS_PRINT("%s()@line %d!!!\n", __func__, __LINE__);
    MI_SYS_BUG_ON(NULL == pstFrameBuf);

    DISP_PTS_PRINT("%s()@line %dpstFrameBuf->u64Pts: %llu!!!\n", __func__, __LINE__, pstFrameBuf->u64Pts);
    // Change toleration unit to micro second.
    TolerationMs = TolerationMs*1000;

    MI_SYS_BUG_ON(NULL == pstPortStatus);
    MI_SYS_BUG_ON(NULL == pstDevStatus);

    pstPortStatus->u64RecvCurPts = pstFrameBuf->u64Pts;

    if((pstPortStatus->u64LastFiredTimeStamp == 0) && (pstPortStatus->u64LastFramePts == 0))
    {
        //First Frame input port
        eHandlePts = E_MI_DISP_FRAME_NORMAL;
    }
    else
    {
#if 0
        if(pstPortStatus->u64FiredDiff == 0)
        {

            DISP_PTS_PRINT("%s()@line %d pstPortStatus->u64FiredDiff: %llu!!!\n", __func__, __LINE__, pstPortStatus->u64FiredDiff);
            DISP_PTS_PRINT("%s()@line %dpstFrameBuf->u64Pts: %llu!!!\n", __func__, __LINE__, pstFrameBuf->u64Pts, pstDevStatus->u64VsyncInterval);

            pstPortStatus->u64FiredDiff = mi_display_ResetPts(pstPortStatus->u64LastFiredTimeStamp, pstPortStatus->u64LastFramePts, &(pstPortStatus->bFramePtsBefore), pstDevStatus->u64VsyncInterval);
            DISP_PTS_PRINT("%s()@line %d!!!\n", __func__, __LINE__);

        }
#else
        if(pstPortStatus->u64FiredDiff == 0)
        {
            if (pstDevStatus->u64LastIntTimeStamp > pstPortStatus->u64LastFramePts)
            {
                pstPortStatus->bFramePtsBefore = TRUE;
                pstPortStatus->u64FiredDiff = pstDevStatus->u64LastIntTimeStamp - pstPortStatus->u64LastFramePts;
            }
            else
            {
                pstPortStatus->bFramePtsBefore = FALSE;
                pstPortStatus->u64FiredDiff = pstPortStatus->u64LastFramePts - pstDevStatus->u64LastIntTimeStamp;
            }

        }
#endif
        DISP_PTS_PRINT("%s()@line %d!!!\n", __func__, __LINE__);
        // TODO: Tommy check pts vailid, if pts invailid, normal
        pstDevStatus->u32AccumInterruptTimeStamp +=  (pstDevStatus->u64CurrentIntTimeStamp - pstDevStatus->u64LastIntTimeStamp);
        pstDevStatus->u32AccumInterruptCnt++;
        if(pstDevStatus->u32AccumInterruptCnt >= 200)
        {
             if(UNSIGNED_DIFF(pstDevStatus->u32VsyncInterval, pstDevStatus->u32AccumInterruptTimeStamp/pstDevStatus->u32AccumInterruptCnt)>=pstDevStatus->u32VsyncInterval/10)
             {
                  DISP_DBG_ERR("warning, real interrupt inval is %dus, but setted timing is %dus!!\n",
                             (pstDevStatus->u32AccumInterruptTimeStamp/pstDevStatus->u32AccumInterruptCnt), pstDevStatus->u32VsyncInterval);
             }
             pstDevStatus->u32AccumInterruptTimeStamp = 0;
             pstDevStatus->u32AccumInterruptCnt = 0;
        }
        //pstDevStatus->u64VsyncInterval = 16 * 1000;

        DISP_PTS_PRINT("%s()@line %d, interval = %llu!!!\n", __func__, __LINE__);
        // TODO: fired diff 改为s

        if(pstPortStatus->bFramePtsBefore == TRUE)
        {
            u64NextPts = pstDevStatus->u64LastIntTimeStamp - pstPortStatus->u64FiredDiff + pstDevStatus->u32VsyncInterval;
            pstPortStatus->u64NextPts = u64NextPts;
        }
        else
        {
            u64NextPts = pstDevStatus->u64LastIntTimeStamp + pstPortStatus->u64FiredDiff + pstDevStatus->u32VsyncInterval;
            pstPortStatus->u64NextPts = u64NextPts;
        }
        DISP_PTS_PRINT("%s()@line %d, NextPts = %u !!!\n", __func__, __LINE__, u64NextPts, pstDevStatus->u32VsyncInterval);
        if(pstFrameBuf->u64Pts > u64NextPts + TolerationMs)
        {
            // TODO:  d. a head out of time, out of toleration
            pstPortStatus->u64FiredDiff = mi_display_ResetPts(pstDevStatus->u64LastIntTimeStamp + pstDevStatus->u32VsyncInterval, pstFrameBuf->u64Pts, &(pstPortStatus->bFramePtsBefore), pstDevStatus->u32VsyncInterval);
            pstPortStatus->u64PtsResetCnt++;
            eHandlePts = E_MI_DISP_FRAME_NORMAL;
        }
        else if(pstFrameBuf->u64Pts < u64NextPts - TolerationMs)
        {
            // TODO: c. expired, out of toleration
            pstPortStatus->u64FiredDiff = mi_display_ResetPts(pstDevStatus->u64LastIntTimeStamp + pstDevStatus->u32VsyncInterval, pstFrameBuf->u64Pts, &(pstPortStatus->bFramePtsBefore), pstDevStatus->u32VsyncInterval);
            pstPortStatus->u64PtsResetCnt++;
            eHandlePts = E_MI_DISP_FRAME_SHOW_LAST;
        }
        else if((pstFrameBuf->u64Pts < u64NextPts))
        {    // TODO: 1normal case, expired but in toleration
            if((u64NextPts - pstFrameBuf->u64Pts)*2 > pstDevStatus->u32VsyncInterval)
            {
                eHandlePts = E_MI_DISP_FRAME_SHOW_LAST;
            }
            else
            {
                eHandlePts = E_MI_DISP_FRAME_NORMAL;
            }

            //eHandlePts = E_MI_DISP_FRAME_NORMAL;
        }
        else if((pstFrameBuf->u64Pts > u64NextPts))
        {
            // TODO: 2normal case, a head of time, but in tolerance.
            // TODO: Check next older frame， add new eunm
            if((pstFrameBuf->u64Pts - u64NextPts)*2 > pstDevStatus->u32VsyncInterval)
            {
                eHandlePts = E_MI_DISP_FRAME_WAIT;
            }
            else
            {
                eHandlePts = E_MI_DISP_FRAME_NORMAL;
            }
        }

        // TODO: Tommy Remove condition 1,2

    }
    DISP_PTS_PRINT("%s()@line %d!!!\n", __func__, __LINE__);

    return eHandlePts;

}

MI_BOOL mi_disp_checkCropInfo(MI_U8 u8LayerId, MI_U8 u8PortId)
{
    MI_BOOL bValid = TRUE;
    MI_DISP_VidWinRect_t stCropWin;
    MI_DISP_VidWinRect_t stDispWin;
    memset(&stCropWin, 0, sizeof(MI_DISP_VidWinRect_t));
    memset(&stDispWin, 0, sizeof(MI_DISP_VidWinRect_t));
    stDispWin = astLayerParams[u8LayerId].astPortStatus[u8PortId].stDispWin;
    stCropWin = astLayerParams[u8LayerId].astPortStatus[u8PortId].stCropWin;
    if(stCropWin.u16Width > stDispWin.u16Width)
    {
        stCropWin.u16Width = stDispWin.u16Width;
    }
    if(stCropWin.u16Height > stDispWin.u16Height)
    {
        stCropWin.u16Height = stDispWin.u16Height;
    }
    if((stCropWin.u16X > stDispWin.u16Width)
        || (stCropWin.u16Y > stDispWin.u16Height)
        || ((stCropWin.u16X + stCropWin.u16Width) > stDispWin.u16Width)
        || ((stCropWin.u16Y + stCropWin.u16Height) > stDispWin.u16Height)
        || (stCropWin.u16Width == 0)
        || (stCropWin.u16Height == 0)
        || ((stCropWin.u16Width == stDispWin.u16Width) && (stCropWin.u16Height == stDispWin.u16Height))
        )
    {
        bValid = FALSE;
    }
    return bValid;
}

MI_BOOL mi_display_FlipFrame(MI_U8 u8LayerId, MI_U8 u8PortId, MI_SYS_BufInfo_t* pstFrameBufInfo)
{
    MI_BOOL bRet = TRUE;
    MHAL_DISP_VideoFrameData_t stHalFrameData;
    mi_vdec_DispFrame_t *pstVdecFrameInfo;
    memset(&stHalFrameData, 0, sizeof(MHAL_DISP_VideoFrameData_t));

    if(astLayerParams[u8LayerId].astPortStatus[u8PortId].bClrAllBuff == TRUE && 
         astLayerParams[u8LayerId].astPortStatus[u8PortId].eStatus == E_MI_LAYER_INPUTPORT_STATUS_HIDE)
    {
        if(FALSE == MHAL_DISP_InputPortShow((astLayerParams[u8LayerId].astPortStatus[u8PortId].apInputObjs)))
        {
            DISP_DBG_ERR("MHAL_DISP_InputPortShow Fail!!!\n");
            bRet = FALSE;
        }
        else
        {
            astLayerParams[u8LayerId].astPortStatus[u8PortId].bClrAllBuff = FALSE;
            astLayerParams[u8LayerId].astPortStatus[u8PortId].eStatus = E_MI_LAYER_INPUTPORT_STATUS_SHOW;
        }
    }
    if(pstFrameBufInfo->eBufType == E_MI_SYS_BUFDATA_FRAME)
    {
        stHalFrameData.eCompressMode = pstFrameBufInfo->stFrameData.eCompressMode;
        stHalFrameData.ePixelFormat = pstFrameBufInfo->stFrameData.ePixelFormat;
        stHalFrameData.aPhyAddr[0] = pstFrameBufInfo->stFrameData.phyAddr[0];
        stHalFrameData.aPhyAddr[1] = pstFrameBufInfo->stFrameData.phyAddr[1];
        stHalFrameData.aPhyAddr[2] = pstFrameBufInfo->stFrameData.phyAddr[2];
        stHalFrameData.u32Height = pstFrameBufInfo->stFrameData.u16Height;
        stHalFrameData.u32Width = pstFrameBufInfo->stFrameData.u16Width;
        stHalFrameData.au32Stride[0] = pstFrameBufInfo->stFrameData.u32Stride[0];
        stHalFrameData.au32Stride[1] = pstFrameBufInfo->stFrameData.u32Stride[1];
        stHalFrameData.au32Stride[2] = pstFrameBufInfo->stFrameData.u32Stride[2];
        if(!MHAL_DISP_InputPortFlip((astLayerParams[u8LayerId].astPortStatus[u8PortId].apInputObjs), &stHalFrameData))
        {
            DISP_DBG_ERR("mi_sys_FinishBuf Failed, the buffer Droped,  portID = %d!!!\n", u8PortId);
            bRet = FALSE;
        }
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
        else
        {
            stDispCheckFrameRate_t *pstCheckFrameRate = &astLayerParams[u8LayerId].astPortStatus[u8PortId].stDispCheckFrameRate;
            if(pstCheckFrameRate->bstat == TRUE)
            {
                pstCheckFrameRate->u32FlipBufCnt++;
            }
        }
#endif
    }
    else if(pstFrameBufInfo->eBufType == E_MI_SYS_BUFDATA_META)
    {
        pstVdecFrameInfo = (mi_vdec_DispFrame_t*)pstFrameBufInfo->stMetaData.pVirAddr;
        stHalFrameData.aPhyAddr[0] = pstVdecFrameInfo->stFrmInfo.phyLumaAddr;
        stHalFrameData.aPhyAddr[1] = pstVdecFrameInfo->stFrmInfo.phyChromaAddr;
        stHalFrameData.ePixelFormat = (MHAL_DISP_PixelFormat_e)pstVdecFrameInfo->ePixelFrm;
        stHalFrameData.u32Width = pstVdecFrameInfo->stFrmInfo.u16Width;
        stHalFrameData.u32Height = pstVdecFrameInfo->stFrmInfo.u16Height;
        stHalFrameData.au32Stride[0] = pstVdecFrameInfo->stFrmInfo.u16Pitch;
        stHalFrameData.au32Stride[1] = pstVdecFrameInfo->stFrmInfo.u16Pitch;
        stHalFrameData.au32Stride[2] = pstVdecFrameInfo->stFrmInfo.u16Pitch;
        stHalFrameData.eTileMode = (MHAL_DISP_TileMode_e)pstVdecFrameInfo->stFrmInfoExt.eFrameTileMode;
        if(!MHAL_DISP_InputPortFlip((astLayerParams[u8LayerId].astPortStatus[u8PortId].apInputObjs), &stHalFrameData))
        {
            DISP_DBG_ERR("mi_sys_FinishBuf Failed, the buffer Droped,  portID = %d!!!\n", u8PortId);
            bRet = FALSE;
        }
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
        else
        {
            stDispCheckFrameRate_t *pstCheckFrameRate = &astLayerParams[u8LayerId].astPortStatus[u8PortId].stDispCheckFrameRate;
            if(pstCheckFrameRate->bstat == TRUE)
            {
                pstCheckFrameRate->u32FlipBufCnt++;
            }
        }
#endif
    }
    DISP_DBG_INFO("bFirstFrame = %u\n", astLayerParams[u8LayerId].astPortStatus[u8PortId].bFirstFrame);
    if(astLayerParams[u8LayerId].astPortStatus[u8PortId].bFirstFrame)
    {
        if(!MHAL_DISP_InputPortEnable((astLayerParams[u8LayerId].astPortStatus[u8PortId].apInputObjs), TRUE))
        {
            DISP_DBG_ERR("MHAL_DISP_InputPortEnable Fail!!!\n");
            bRet = FALSE;
        }
        else
        {
            astLayerParams[u8LayerId].astPortStatus[u8PortId].bFirstFrame = FALSE;
            DISP_DBG_INFO("bFirstFrame = %u\n", astLayerParams[u8LayerId].astPortStatus[u8PortId].bFirstFrame);
        }
    }

    astLayerParams[u8LayerId].astPortStatus[u8PortId].bGeFlip = FALSE;
    return bRet;
}

MI_BOOL mi_display_ScalingByGe(MI_U8 u8LayerId, MI_U8 u8PortId, MI_SYS_BufInfo_t* pstFrameBufInfo)
{
    MI_BOOL bRet = FALSE;
    MI_DISP_VidWinRect_t stCropWin;
    MI_DISP_VidWinRect_t stDispWin;
    MI_GFX_Surface_t stSrc;
    MI_GFX_Surface_t stDst;
    MI_GFX_Rect_t stSrcRect;
    MI_GFX_Rect_t stDstRect;
    MI_U16 u16Fence = 0;
    MI_SYS_BufInfo_t stOnScreenBufInfo;

    memset(&stCropWin, 0, sizeof(MI_DISP_VidWinRect_t));
    memset(&stDispWin, 0, sizeof(MI_DISP_VidWinRect_t));
    memset(&stSrc, 0, sizeof(MI_GFX_Surface_t));
    memset(&stDst, 0, sizeof(MI_GFX_Surface_t));
    memset(&stSrcRect, 0, sizeof(MI_GFX_Rect_t));
    memset(&stDstRect, 0, sizeof(MI_GFX_Rect_t));
    memset(&stOnScreenBufInfo, 0, sizeof(MI_SYS_BufInfo_t));

    stDispWin = astLayerParams[u8LayerId].astPortStatus[u8PortId].stDispWin;
    stCropWin = astLayerParams[u8LayerId].astPortStatus[u8PortId].stCropWin;

    stOnScreenBufInfo = *(astLayerParams[u8LayerId].pstOnScreenBufInfo[u8PortId]);
    stSrc.eColorFmt = E_MI_GFX_FMT_YUV422;
    stSrc.phyAddr = pstFrameBufInfo->stFrameData.phyAddr[0];
    stSrc.u32Height = pstFrameBufInfo->stFrameData.u16Height;
    stSrc.u32Width = pstFrameBufInfo->stFrameData.u16Width;
    stSrc.u32Stride = pstFrameBufInfo->stFrameData.u32Stride[0];
    stDst.eColorFmt = E_MI_GFX_FMT_YUV422;
    stDst.phyAddr = stOnScreenBufInfo.stFrameData.phyAddr[0];
    stDst.u32Height = stOnScreenBufInfo.stFrameData.u16Height;
    stDst.u32Width = stOnScreenBufInfo.stFrameData.u16Width;
    stDst.u32Stride = stOnScreenBufInfo.stFrameData.u32Stride[0];
    stSrcRect.s32Xpos = stCropWin.u16X;
    stSrcRect.s32Ypos = stCropWin.u16Y;
    stSrcRect.u32Width = stCropWin.u16Width;
    stSrcRect.u32Height = stCropWin.u16Height;
    stDstRect.s32Xpos = stDispWin.u16X;
    stDstRect.s32Ypos = stDispWin.u16Y;
    stDstRect.u32Width = stDispWin.u16Width;
    stDstRect.u32Height = stDispWin.u16Height;

    if(MI_GFX_BitBlit(&stSrc, &stSrcRect, &stDst, &stDstRect, NULL, &u16Fence))
    {
        DISP_DBG_ERR("[%s %d] MI_GFX_BitBlit fail!!!\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    MI_GFX_WaitAllDone(FALSE, u16Fence);

    astLayerParams[u8LayerId].astPortStatus[u8PortId].u16Fence = u16Fence;
    astLayerParams[u8LayerId].astPortStatus[u8PortId].bGeFlip = TRUE;

    return bRet;
}
MI_BOOL mi_display_UpdateOnScreenFrame(MI_U8 u8LayerId, MI_U8 u8PortId)
{
    MI_BOOL bGeFlip = astLayerParams[u8LayerId].astPortStatus[u8PortId].bGeFlip;
    mi_disp_LayerStatus_t *pstTmpLayer = NULL;
    pstTmpLayer = &(astLayerParams[u8LayerId]);
    if(bGeFlip)
    {
        if(pstTmpLayer->pstCurrentFiredBufInfo[u8PortId])
        {
            if(pstTmpLayer->pstOnScreenBufInfo[u8PortId])
            {
                mi_sys_FinishBuf(pstTmpLayer->pstCurrentFiredBufInfo[u8PortId]);
            }
            else
            {
                DBG_INFO("OnScreen buffer is NULL\n");
                pstTmpLayer->pstOnScreenBufInfo[u8PortId] = pstTmpLayer->pstCurrentFiredBufInfo[u8PortId];
            }
            pstTmpLayer->pstCurrentFiredBufInfo[u8PortId] = NULL;
        }
    }
    else
    {
        //DBG_ERR("LayerID: %d u8PortId: %d pstCurrentFiredBufInfo: %p.\n", u8LayerId, u8PortId, pstTmpLayer->pstCurrentFiredBufInfo[u8PortId]);
        if(pstTmpLayer->pstCurrentFiredBufInfo[u8PortId] != NULL)
        {
            if(pstTmpLayer->pstOnScreenBufInfo[u8PortId])
            {
                mi_sys_FinishBuf(pstTmpLayer->pstOnScreenBufInfo[u8PortId]);
                //DBG_INFO("LayerID: %d u8PortId: %d pstOnScreenBufInfo: %p.\n", u8LayerId, u8PortId, pstTmpLayer->pstOnScreenBufInfo[u8PortId]);
            }
            pstTmpLayer->pstOnScreenBufInfo[u8PortId] = pstTmpLayer->pstCurrentFiredBufInfo[u8PortId];
            pstTmpLayer->pstCurrentFiredBufInfo[u8PortId] = NULL;
        }
    }
    //printk("[%s %d] mi_display_UpdateOnScreenFrame bGeFlip = %d\n", __FUNCTION__, __LINE__, bGeFlip == TRUE);
    return TRUE;
}

MI_BOOL mi_display_CheckGeFlip(MI_U8 u8LayerId)
{
    MI_BOOL bRet = FALSE;
    MI_U8 u8PortCount = 0;
    for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
    {
        if(astLayerParams[u8LayerId].astPortStatus[u8PortCount].bGeFlip)
        {
            bRet = TRUE;
        }
    }
    return bRet;
}

MI_S32 MI_DISP_IMPL_ClearInputPortBuffer(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_BOOL bClrAll)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    DISP_DBG_ENTER("layer %d, port %d, bclear %d!\n", DispLayer, LayerInputPort, bClrAll);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
 
    //down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    MI_DISP_GetVarMutex();

    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable)
    {
        if(bClrAll == TRUE)
        {
            if(MHAL_DISP_InputPortHide((astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs)) != TRUE)
            {
                DISP_DBG_ERR("MHAL_DISP_InputPortHide Fail!!!\n");
                goto UP_PORT_MUTEX;
            }
            else 
            {
                astLayerParams[DispLayer].astPortStatus[LayerInputPort].eStatus = E_MI_LAYER_INPUTPORT_STATUS_HIDE;
                astLayerParams[DispLayer].astPortStatus[LayerInputPort].bClrAllBuff = TRUE;
            }
            if(NULL != astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort])
            {
                if(mi_sys_FinishBuf(astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort]) != MI_SUCCESS)
                {
                    DISP_DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                    goto UP_PORT_MUTEX;
                }
                else
                {
                    astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort] = NULL;
                }
            }
            if(NULL != astLayerParams[DispLayer].pstCurrentFiredBufInfo[LayerInputPort])
            {
                if(mi_sys_FinishBuf(astLayerParams[DispLayer].pstCurrentFiredBufInfo[LayerInputPort])!= MI_SUCCESS)
                {
                    DISP_DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                    goto UP_PORT_MUTEX;
                }
                else
                {
                    astLayerParams[DispLayer].pstCurrentFiredBufInfo[LayerInputPort] = NULL;
                }
            }
        }
        else
        {
            mi_display_UpdateOnScreenFrame(DispLayer,LayerInputPort);
        }
        down(&(astLayerParams[DispLayer].stDispLayerPendingQueueMutex));
        if(!list_empty(&astLayerParams[DispLayer].stPortPendingBufQueue[LayerInputPort]))
        {
            struct list_head *pstPendingBufferPos = NULL;
            struct list_head *n = NULL;
            mi_disp_PortPendingBuf_t *pstTmpPendingBuf;
            MI_SYS_BufInfo_t* pstTmpInputBuffer;
            list_for_each_safe(pstPendingBufferPos, n, &(astLayerParams[DispLayer].stPortPendingBufQueue[LayerInputPort]))
            {
                pstTmpPendingBuf = list_entry(pstPendingBufferPos, mi_disp_PortPendingBuf_t, stPortPendingBufNode);
                list_del(pstPendingBufferPos);
                pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                memset(pstTmpPendingBuf, 0xF2, sizeof(mi_disp_PortPendingBuf_t));
                kfree(pstTmpPendingBuf);
                if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_SUCCESS)
                {
                    DISP_DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                    up(&(astLayerParams[DispLayer].stDispLayerPendingQueueMutex));       
                    goto UP_PORT_MUTEX;
                }
            }
        }
        up(&(astLayerParams[DispLayer].stDispLayerPendingQueueMutex));       
    }   
    else
    {
        DISP_DBG_INFO("input port has been disable before!\n");
        goto UP_PORT_MUTEX;
    }

   s32Ret = MI_SUCCESS;
    DISP_DBG_EXITOK();
UP_PORT_MUTEX:
    MI_DISP_ReleaseVarMutex();
EXIT:
    return s32Ret;
}

int Disp_Dev_ISR_Thread(void *pdata)
{
    int ret = -1;
    MI_U32 u32DevId = 0;
    struct timespec sttime;

    mi_disp_DevStatus_t* pstDispDevParam = (mi_disp_DevStatus_t*)pdata;
    if(pdata == NULL)
    {
        DISP_DBG_ERR("NULL Pointer, Disp_Dev_Work_Thread Return!!!\n");
        return ret;
    }

    MI_GFX_Open();
    while(!kthread_should_stop())
    {
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
        interruptible_sleep_on_timeout(&(pstDispDevParam->stWaitQueueHead), 10);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
        wait_event_interruptible_timeout((pstDispDevParam->stWaitQueueHead), atomic_read(&pstDispDevParam->disp_isr_waitqueue_red_flag) > 0, msecs_to_jiffies(10));
#endif
        atomic_dec(&pstDispDevParam->disp_isr_waitqueue_red_flag);

        //MI_DISP_DOWN(&(pstDispDevParam->stDevMutex));
        MI_DISP_GetVarMutex();
        u32DevId = pstDispDevParam->u32DevId;
        if(!pstDispDevParam->bDISPEnabled)
        {
            MI_DISP_ReleaseVarMutex();
            continue;
        }

        if(!list_empty(&pstDispDevParam->stBindedLayer))
        {
            struct list_head *pstPos = NULL;
            mi_disp_LayerStatus_t *pstTmpLayer = NULL;
            struct list_head *n = NULL;
            MI_U8 u8LayerID = 0;
            list_for_each_safe(pstPos, n, &pstDispDevParam->stBindedLayer)// &(astDevStatus[u32DevId].stBindedLayer))
            {
                MI_U8 u8PortCount = 0;
                pstTmpLayer = list_entry(pstPos, mi_disp_LayerStatus_t, stLayerNode);
                u8LayerID = pstTmpLayer->u8LayerID;

                if(!pstTmpLayer->bLayerEnabled)
                {
                    continue;
                }

                DISP_DBG_INFO("Start Scan Layer: %d!!!\n", u8LayerID);
                // TODO:  Tommy Add check Ge fence done flow
                // Neec Check mvop int count
                if(!MHAL_DISP_VideoLayerCheckBufferFired(pstTmpLayer->apLayerObjs)
                    && !mi_display_CheckGeFlip(u8LayerID)
                    )
                {
                    continue;
                }
                DISP_DBG_INFO("Start Scan Layer: %d!!!\n", u8LayerID);

                for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
                {
                   //Update Screen  buffer
                   // TODO: Tommy  : Warning:check ge fence done , if not done ,don't release ge source buf
                    mi_display_UpdateOnScreenFrame(u8LayerID, u8PortCount);
                    if((!pstTmpLayer->astPortStatus[u8PortCount].bEnable) && (pstTmpLayer->pstOnScreenBufInfo[u8PortCount]))
                    {
                        mi_sys_FinishBuf(pstTmpLayer->pstOnScreenBufInfo[u8PortCount]);
                        pstTmpLayer->pstOnScreenBufInfo[u8PortCount] = NULL;
                    }
                    down(&(pstTmpLayer->stDispLayerPendingQueueMutex));
                    if(!list_empty(&pstTmpLayer->stPortPendingBufQueue[u8PortCount]))
                    {
                        struct list_head *pstPendingBufferPos = NULL;
                        struct list_head *m = NULL;
                        mi_disp_PortPendingBuf_t *pstTmpPendingBuf;
                        MI_SYS_BufInfo_t* pstTmpInputBuffer;
                        MI_DISP_HandleFrame_e eHandleFrame;

                        DISP_DBG_INFO("Start Scan Layer: %d  port: %d bEnable: %d!!!\n", u8LayerID, u8PortCount, pstTmpLayer->astPortStatus[u8PortCount].bEnable);
                        list_for_each_safe(pstPendingBufferPos, m, &(pstTmpLayer->stPortPendingBufQueue[u8PortCount]))
                        {
                            if(m == &(pstTmpLayer->stPortPendingBufQueue[u8PortCount]))
                            {
                                DISP_DBG_INFO("[%s %d]Just for Debug\n", __FUNCTION__, __LINE__);
                            }

                            // TODO: 更新fired 时间戳
                            DISP_DBG_INFO("pstPendingBufferPos: %p!!!\n", pstPendingBufferPos);
                            eHandleFrame = E_MI_DISP_FRAME_NORMAL;
                            pstTmpPendingBuf = list_entry(pstPendingBufferPos, mi_disp_PortPendingBuf_t, stPortPendingBufNode);
                            pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                            list_del(pstPendingBufferPos);
                            memset(pstTmpPendingBuf, 0xF1, sizeof(*pstTmpPendingBuf));
                            kfree(pstTmpPendingBuf);
                            DISP_DBG_INFO("[%s %d] pending buffer[%d, %d] !!!\n", __FUNCTION__, __LINE__, pstTmpInputBuffer->stFrameData.u16Width, pstTmpInputBuffer->stFrameData.u16Height);

                            if((astLayerParams[u8LayerID].astPortStatus[u8PortCount].eMode == E_MI_DISP_SYNC_MODE_FREE_RUN)
                                ||(astLayerParams[u8LayerID].astPortStatus[u8PortCount].eStatus == E_MI_LAYER_INPUTPORT_STATUS_STEP))
                            {
                                eHandleFrame = E_MI_DISP_FRAME_NORMAL;
                            }
                            else
                            {
                                eHandleFrame = mi_display_CheckInputPortPts(pstTmpInputBuffer, &(astLayerParams[u8LayerID].astPortStatus[u8PortCount]), pstDispDevParam, astLayerParams[u8LayerID].u32Toleration);
                            }
                            DISP_DBG_INFO("[%s %d] pts handle = %d !!!!!!\n", __FUNCTION__, __LINE__, eHandleFrame);

                            // TODO: Debug condition
                            if(MI_DISP_FORCE_ENABLE_PTS_CHK)
                            {
                                if(eHandleFrame == E_MI_DISP_FRAME_DROP)
                                {
                                    if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_SUCCESS)
                                    {
                                        DISP_DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                                    }
                                    memset(pstTmpPendingBuf, 0xF1, sizeof(*pstTmpPendingBuf));
                                    kfree(pstTmpPendingBuf);
                                    continue;
                                }
                                else if(eHandleFrame == E_MI_DISP_FRAME_SHOW_LAST)
                                {
                                    // TODO: Tommy Relace macro
                                    if(list_empty(&(pstTmpLayer->stPortPendingBufQueue[u8PortCount])))
                                    {
                                        DISP_DBG_INFO("[%s %d]No others buffer ,show Current buffer\n", __FUNCTION__, __LINE__);
                                    }
                                    else
                                    {
                                        DISP_DBG_INFO("[%s %d]Has more buffer,Finish current buffer\n", __FUNCTION__, __LINE__);
                                        if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_SUCCESS)
                                        {
                                            DISP_DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                                        }
                                        continue;
#if 0
                                        pstTmpPendingBuf = list_entry(m, mi_disp_PortPendingBuf_t, stPortPendingBufNode);
                                        pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                                        DISP_DBG_ERR("[%s %d] pending buffer[%d, %d] !!!\n", __FUNCTION__, __LINE__, pstTmpInputBuffer->stFrameData.u16Width, pstTmpInputBuffer->stFrameData.u16Height);
                                        for(m1 = m->next; m->next != &(pstTmpLayer->stPortPendingBufQueue[u8PortCount]); m = m1, m1 = m1->next)
                                        {
                                            pstTmpPendingBuf = list_entry(m, mi_disp_PortPendingBuf_t, stPortPendingBufNode);
                                            list_del(m);
                                            pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                                            kfree(pstTmpPendingBuf);
                                            if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_SUCCESS)
                                            {
                                                DISP_DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                                            }
                                        }
                                        pstTmpPendingBuf = list_entry(m, mi_disp_PortPendingBuf_t, stPortPendingBufNode);
                                        list_del(m);
                                        pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                                        kfree(pstTmpPendingBuf);
                                        DISP_DBG_ERR("[%s %d] pending buffer[%d, %d] !!!\n", __FUNCTION__, __LINE__, pstTmpInputBuffer->stFrameData.u16Width, pstTmpInputBuffer->stFrameData.u16Height);
                                        if(m1 == &(pstTmpLayer->stPortPendingBufQueue[u8PortCount]))
                                        {
                                            DISP_DBG_ERR("[%s %d]Just for Debug\n", __FUNCTION__, __LINE__);
                                        }
                                        m = m1;
#endif
                                    }
                                }
                                else if(eHandleFrame == E_MI_DISP_FRAME_NORMAL)
                                {
                                    //Normal case
                                }
                            }
                            if(mi_display_SuitableDispWin(pstTmpInputBuffer, &astLayerParams[u8LayerID].astPortStatus[u8PortCount].stDispWin))
                            {
                                MI_BOOL bPreCropSideBandAcked;
                                memset(&sttime, 0, sizeof(&sttime));
#if 0
                                if (astLayerParams[u8LayerID].pstCurrentFiredBufInfo[u8PortCount] != NULL)
                                {
                                    mi_sys_FinishBuf(astLayerParams[u8LayerID].pstCurrentFiredBufInfo[u8PortCount]);

                                }
#endif
                                bPreCropSideBandAcked = MI_SYS_SIDEBAND_MSG_ACKED(pstTmpInputBuffer->u64SidebandMsg);
                                MI_SYS_BUG_ON(bPreCropSideBandAcked && MI_SYS_GET_SIDEBAND_MSG_TYPE(pstTmpInputBuffer->u64SidebandMsg)!=MI_SYS_SIDEBAND_MSG_TYPE_PREFER_CROP_RECT);
                                if(!bPreCropSideBandAcked && mi_disp_checkCropInfo(u8LayerID, u8PortCount) && (astLayerParams[u8LayerID].pstOnScreenBufInfo[u8PortCount] != NULL))
                                {
                                    mi_display_ScalingByGe(u8LayerID, u8PortCount, pstTmpInputBuffer);
                                }
                                else
                                {
                                    mi_display_FlipFrame(u8LayerID, u8PortCount, pstTmpInputBuffer);
                                }
                                astLayerParams[u8LayerID].pstCurrentFiredBufInfo[u8PortCount] = pstTmpInputBuffer;
                                do_posix_clock_monotonic_gettime(&sttime);
                                astLayerParams[u8LayerID].astPortStatus[u8PortCount].u64LastFiredTimeStamp = sttime.tv_sec * 1000 * 1000 + (sttime.tv_nsec / 1000);
                                astLayerParams[u8LayerID].astPortStatus[u8PortCount].u64LastFramePts = pstTmpInputBuffer->u64Pts;
                                break;
                            }
                            else
                            {
                                DISP_DBG_INFO("pending buffer[%d, %d] !!!\n", pstTmpInputBuffer->stFrameData.u16Width, pstTmpInputBuffer->stFrameData.u16Height);
                                DISP_DBG_INFO("pending buffer Addr 0x%x !!!\n", pstTmpInputBuffer->stFrameData.phyAddr[0]);
                                DISP_DBG_INFO("Video Layer %d [%d, %d] !!!\n", u8LayerID, astLayerParams[u8LayerID].stVideoLayerAttr.stVidLayerSize.u16Width, astLayerParams[u8LayerID].stVideoLayerAttr.stVidLayerSize.u16Height);
                                DISP_DBG_INFO("Frame Not Suitable video layer= %d !!!!!!\n", u8LayerID);
                                mi_sys_FinishBuf(pstTmpInputBuffer);
                            }
                            DISP_DBG_INFO("[%s %d]pending buffer[%d, %d] !!!\n", __FUNCTION__, __LINE__, pstTmpInputBuffer->stFrameData.u16Width, pstTmpInputBuffer->stFrameData.u16Height);
                            DISP_DBG_INFO("[%s %d]pending buffer Addr 0x%x !!!\n", __FUNCTION__, __LINE__, pstTmpInputBuffer->stFrameData.pVirAddr[0]);
                        }
                     }
                     up(&(pstTmpLayer->stDispLayerPendingQueueMutex));
                }
                //up(&(pstTmpLayer->stLayerMutex));
            }
        }
        //MI_DISP_UP(&(pstDispDevParam->stDevMutex));
        MI_DISP_ReleaseVarMutex();

         //记录中断的确切时间
        // TODO: 记录frame fired 的时间

        #if 0
        memset(&sttime, 0, sizeof(sttime));
        do_posix_clock_monotonic_gettime(&sttime);
        if(pstDispDevParam->u64VsyncInterval == 0)
        {
            pstDispDevParam->u64CurrentIntTimeStamp = sttime.tv_sec * 1000000ULL + (sttime.tv_nsec / 1000);
            pstDispDevParam->u64LastIntTimeStamp = pstDispDevParam->u64CurrentIntTimeStamp;
            pstDispDevParam->u64VsyncInterval = 20 * 1000;
        }
        else
        {
            pstDispDevParam->u64LastIntTimeStamp = pstDispDevParam->u64CurrentIntTimeStamp;
            pstDispDevParam->u64CurrentIntTimeStamp = sttime.tv_sec * 1000000ULL + (sttime.tv_nsec / 1000);
        }
        #endif
        //mdelay(16);
    }
    ret = 0;
    return ret;
}

int Disp_Dev_Work_Thread(void *pdata)
{
    int ret = -1;
    MI_U32 u32DevId = 0;
    MI_BOOL bAvaliable = FALSE;
    mi_disp_DevStatus_t* pstDispDevParam = NULL;
    struct list_head *pstPos = NULL, *pTemp = NULL;
    mi_disp_LayerStatus_t *pstTmpLayer = NULL;
    MI_U8 u8LayerID = 0;
    MI_SYS_ChnPort_t stChnInfo;
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    MI_U8 u8PortCount = 0;

    if(pdata == NULL)
    {
        DISP_DBG_ERR("NULL Pointer, Disp_Dev_Work_Thread Return!!!\n");
        return ret;
    }

    pstDispDevParam = (mi_disp_DevStatus_t*)pdata;
    u32DevId = pstDispDevParam->u32DevId;

    while (!kthread_should_stop())
    {
        trace();
        // TODO: Tommy Remove -- done
       // interruptible_sleep_on_timeout(&pstDispDevParam->stWaitQueueHead, 16);
        trace();

        if(mi_sys_WaitOnInputTaskAvailable(pstDispDevParam->hDevSysHandle, 16) != MI_SUCCESS)
        {
            //DISP_DBG_ERR("mi_sys_WaitOnInputTaskAvailable Fail!!!\n");
            continue;
        }
        trace();
        //MI_DISP_DOWN(&(pstDispDevParam->stDevMutex));
        MI_DISP_GetVarMutex();
        if(!pstDispDevParam->bDISPEnabled)
        {
            //MI_DISP_UP(&(pstDispDevParam->stDevMutex));
            MI_DISP_ReleaseVarMutex();
            continue;
        }
        // TODO: return
        //MI_DISP_UP(&(pstDispDevParam->stDevMutex));
        //MI_DISP_ReleaseVarMutex();
        //continue;

        // TODO: Tommy DevID = Dev, ChnId = bindedVideo layer ; portId = input port
        // Dev only one video layer
        if(!list_empty(&pstDispDevParam->stBindedLayer))
        {
            list_for_each_safe(pstPos, pTemp, &(pstDispDevParam->stBindedLayer))
            {
                pstTmpLayer = list_entry(pstPos, mi_disp_LayerStatus_t, stLayerNode);
                u8LayerID = pstTmpLayer->u8LayerID;
                //down(&(pstTmpLayer->stLayerMutex));

                DISP_DBG_INFO("Start Scan Layer: %d!!!\n", u8LayerID);
                if(!pstTmpLayer->bLayerEnabled)
                {
                    continue;
                }

                for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
                {
                    //down(&(astLayerParams[u8LayerID].astPortStatus[u8PortCount].stPortMutex));

                    if(pstTmpLayer->astPortStatus[u8PortCount].bEnable)
                    {
                        DISP_DBG_INFO("Start Scan Port: %d!!!\n", u8PortCount);
                        memset(&stChnInfo, 0, sizeof(MI_SYS_ChnPort_t));
                        stChnInfo.eModId = E_MI_MODULE_ID_DISP;
                        stChnInfo.u32ChnId = mi_disp_TransLayerToDevChnn(u32DevId, u8LayerID);//Disp module Chn Equals Dev;
                        stChnInfo.u32DevId = u32DevId;
                        stChnInfo.u32PortId = u8PortCount;
                        down(&(astLayerParams[u8LayerID].stDispLayerPendingQueueMutex));
                        while((pstBufInfo = mi_sys_GetInputPortBuf(astDevStatus[u32DevId].hDevSysHandle, stChnInfo.u32ChnId, stChnInfo.u32PortId,0))
                            && (pstBufInfo != NULL))
                        {
                            //Allocate pending buffer node
                            mi_disp_PortPendingBuf_t* pstPortPendingBuf = NULL;

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
                            pstTmpLayer->astPortStatus[u8PortCount].stDispCheckBuffInfo.u32InportRecvBufCnt++;
                            pstTmpLayer->astPortStatus[u8PortCount].stDispCheckBuffInfo.u32RecvBufWidth = pstBufInfo->stFrameData.u16Width;
                            pstTmpLayer->astPortStatus[u8PortCount].stDispCheckBuffInfo.u32RecvBufHeight = pstBufInfo->stFrameData.u16Height;
                            pstTmpLayer->astPortStatus[u8PortCount].stDispCheckBuffInfo.u32RecvBufStride = pstBufInfo->stFrameData.u32Stride[0];
                            _mi_disp_CheckFramePts(pstBufInfo, u8LayerID, u8PortCount);  
                            _mi_disp_CheckFrameRate(pstBufInfo, u8LayerID, u8PortCount);
                            _mi_disp_DumpFrame(pstBufInfo, u8LayerID, u8PortCount);
                            if(pstTmpLayer->astPortStatus[u8PortCount].bStopGetInBuf == TRUE)
                            {
                                mi_sys_FinishBuf(pstBufInfo);
                                continue;
                            }
#endif

#if 0
                            static MI_U64 u64PTS[1024], u64GotTime[1024];
                            static MI_U64 u64Count = 0;

                            if (u64Count >= 1024)
                            {
                                int i = 0;
                                for (i = 1; i < 1024; i++)
                                {
                                    printk("sys - pts: %llu got_interval: %llu.\n", u64GotTime[i] - u64PTS[i], u64GotTime[i] - u64GotTime[i - 1]);
                                }
                                u64Count = 0;
                            }
                            DISP_GET_PERF_TIME(&u64GotTime[u64Count]);
                            u64PTS[u64Count]     = pstBufInfo->u64Pts;
                            u64Count++;
#endif
                            //增加出错处理， 使用kmalloc_cache
                            pstPortPendingBuf = kzalloc(sizeof(mi_disp_PortPendingBuf_t), GFP_KERNEL);//kmem_cache_alloc(sizeof(mi_disp_PortPendingBuf_t), GFP_KERNEL);
                            if(pstPortPendingBuf != NULL)
                            {

                                DISP_DBG_INFO("pstPortPendingBuf->pstInputBuffer: %p -> %p!!!\n", &pstPortPendingBuf->pstInputBuffer, pstBufInfo);
                                INIT_LIST_HEAD(&(pstPortPendingBuf->stPortPendingBufNode));
                                pstPortPendingBuf->pstInputBuffer = pstBufInfo;
                                list_add_tail(&(pstPortPendingBuf->stPortPendingBufNode), &astLayerParams[u8LayerID].stPortPendingBufQueue[u8PortCount]);

                                DISP_DBG_INFO("[%s %d]pending buffer[%d, %d] !!!\n", __FUNCTION__, __LINE__, pstBufInfo->stFrameData.u16Width, pstBufInfo->stFrameData.u16Height);

                                DISP_DBG_INFO("pstPortPendingBuf->pstInputBuffer: %p -> %p!!!\n", &pstPortPendingBuf->pstInputBuffer, pstPortPendingBuf->pstInputBuffer);
                                bAvaliable = TRUE;
                                DISP_DBG_INFO("[%s %d]pending buffer[%d, %d] !!!\n", __FUNCTION__, __LINE__, pstBufInfo->stFrameData.u16Width, pstBufInfo->stFrameData.u16Height);
                                DISP_DBG_INFO("[%s %d]pending buffer Addr 0x%x !!!\n", __FUNCTION__, __LINE__, pstBufInfo->stFrameData.phyAddr[0]);

                                DISP_PERF_TIME(&pstPortPendingBuf->u64GotTime);
                            }
                            else
                            {
                                DISP_DBG_ERR("kmem_cache_alloc Fail!!!\n");
                                mi_sys_RewindBuf(pstBufInfo);
                            }
                        }
                        up(&(astLayerParams[u8LayerID].stDispLayerPendingQueueMutex));
                    }
                    //up(&(astLayerParams[u8LayerID].astPortStatus[u8PortCount].stPortMutex));
                }//Go through input port in Video Layer
                //up(&(pstTmpLayer->stLayerMutex));
            }//Go through All binded Video layer
        }// Check Binded list empty
        if(bAvaliable)
        {
            atomic_inc(&pstDispDevParam->disp_isr_waitqueue_red_flag);
            WAKE_UP_QUEUE_IF_NECESSARY(pstDispDevParam->stWaitQueueHead);
        }
        //MI_DISP_UP(&(pstDispDevParam->stDevMutex));
        MI_DISP_ReleaseVarMutex();
    }
    ret = 0;
    return ret;
}

static MI_S32 mi_disp_InitLayerStatus(mi_disp_LayerStatus_t* pstLayerStatus, MI_U32 u32LayerId)
{
    MI_U16 u16Index = 0;
    if(pstLayerStatus == NULL)
    {
        DISP_DBG_ERR("mi_disp_InitLayerStatus Fail !!!\n");
        return E_MI_ERR_FAILED;
    }

    pstLayerStatus->bCompress = false;
    pstLayerStatus->bInited = true;
    pstLayerStatus->bLayerEnabled = false;
    pstLayerStatus->stVideoLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
    INIT_LIST_HEAD(&pstLayerStatus->stLayerNode);
    for(u16Index = 0; u16Index < MI_DISP_INPUTPORT_MAX; u16Index ++)
    {
        INIT_LIST_HEAD(&pstLayerStatus->stPortPendingBufQueue[u16Index]);
        pstLayerStatus->pstCurrentFiredBufInfo[u16Index] = NULL;
        pstLayerStatus->pstOnScreenBufInfo[u16Index] = NULL;
    }
    pstLayerStatus->u32Priority = 0;
    pstLayerStatus->u32Toleration = 100;
    pstLayerStatus->u8BindedDevID = 0xff;
    pstLayerStatus->u8LayerID = u32LayerId;
    INIT_LIST_HEAD(&(pstLayerStatus->stLayerNode));

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
    init_MUTEX(&(astLayerParams[u32LayerId].stDispLayerPendingQueueMutex));
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    sema_init(&(astLayerParams[u32LayerId].stDispLayerPendingQueueMutex), 1);
#endif

    return MI_SUCCESS;

}

static MI_S32 mi_disp_InitDevStatus(mi_disp_DevStatus_t* pstDevStatus, MI_U32 u32DevId)
{
    if(pstDevStatus == NULL)
    {
        DISP_DBG_ERR("mi_disp_InitDevStatus Fail !!!\n");
        return E_MI_ERR_FAILED;
    }
    INIT_LIST_HEAD(&pstDevStatus->stBindedLayer);
    pstDevStatus->bDISPEnabled = FALSE;
    // TODO: Shoke Check workqueue init
    init_waitqueue_head(&pstDevStatus->stWaitQueueHead);
    atomic_set(&pstDevStatus->disp_isr_waitqueue_red_flag, 0);
    pstDevStatus->u32DevId = u32DevId;
    pstDevStatus->u32SrcW = 0; //layer width
    pstDevStatus->u32SrcH = 0; //layer height
    pstDevStatus->u32BgColor = 0;
    pstDevStatus->u32Luma = 50;
    pstDevStatus->u32Contrast = 50;
    pstDevStatus->u32Hue = 50;
    pstDevStatus->u32Saturation = 50;
    pstDevStatus->u32Gain = 0;
    pstDevStatus->u32Sharpness = 0;
    pstDevStatus->bCvbsEnable = FALSE;
    pstDevStatus->pstDevObj = NULL;
    INIT_LIST_HEAD(&pstDevStatus->stDevNode);
    pstDevStatus->pstWorkTask = NULL;
    pstDevStatus->pstIsrTask = NULL;
    //init_MUTEX(&(pstDevStatus->stDevMutex));
    return MI_SUCCESS;

}

void mi_disp_ConvertToDriverPanelTypeInfoTrans(const MAPI_PanelType* const pApiPanelType, MHAL_DISP_PanelType_t* pDriverPanelType)
{
    // MAPI_APIPNL_LINK_TYPE V.S. APIPNL_LINK_TYPE
    STATIC_ASSERT((int)E_MAPI_LINK_TTL == (int)E_MHAL_LINK_TTL);
    STATIC_ASSERT((int)E_MAPI_LINK_LVDS == (int)E_MHAL_LINK_LVDS);
    STATIC_ASSERT((int)E_MAPI_LINK_RSDS == (int)E_MHAL_LINK_RSDS);
    STATIC_ASSERT((int)E_MAPI_LINK_MINILVDS == (int)E_MHAL_LINK_MINILVDS);
    STATIC_ASSERT((int)E_MAPI_LINK_ANALOG_MINILVDS == (int)E_MHAL_LINK_ANALOG_MINILVDS);
    STATIC_ASSERT((int)E_MAPI_LINK_DIGITAL_MINILVDS == (int)E_MHAL_LINK_DIGITAL_MINILVDS);
    STATIC_ASSERT((int)E_MAPI_LINK_MFC == (int)E_MHAL_LINK_MFC);
    STATIC_ASSERT((int)E_MAPI_LINK_DAC_I == (int)E_MHAL_LINK_DAC_I);
    STATIC_ASSERT((int)E_MAPI_LINK_DAC_P == (int)E_MHAL_LINK_DAC_P);
    STATIC_ASSERT((int)E_MAPI_LINK_PDPLVDS == (int)E_MHAL_LINK_PDPLVDS);
    STATIC_ASSERT((int)E_MAPI_LINK_EXT == (int)E_MHAL_LINK_EXT);

    // MAPI_PNL_ASPECT_RATIO V.S. E_PNL_ASPECT_RATIO
    STATIC_ASSERT((int)E_MAPI_PNL_ASPECT_RATIO_4_3 == (int)E_MHAL_PNL_ASPECT_RATIO_4_3);
    STATIC_ASSERT((int)E_MAPI_PNL_ASPECT_RATIO_WIDE == (int)E_MHAL_PNL_ASPECT_RATIO_WIDE);
    STATIC_ASSERT((int)E_MAPI_PNL_ASPECT_RATIO_OTHER == (int)E_MHAL_PNL_ASPECT_RATIO_OTHER);

    // MAPI_APIPNL_TIBITMODE V.S. APIPNL_TIBITMODE
    STATIC_ASSERT((int)E_MAPI_TI_10BIT_MODE == (int)E_MHAL_TI_10BIT_MODE);
    STATIC_ASSERT((int)E_MAPI_TI_8BIT_MODE == (int)E_MHAL_TI_8BIT_MODE);
    STATIC_ASSERT((int)E_MAPI_TI_6BIT_MODE == (int)E_MHAL_TI_6BIT_MODE);

    if ((NULL != pApiPanelType) && (NULL != pDriverPanelType))
    {
        pDriverPanelType->m_pPanelName = pApiPanelType->pPanelName;
        pDriverPanelType->m_bPanelDither      = pApiPanelType->bPanelDither;

        // need change later
        pDriverPanelType->m_ePanelLinkType    = (MHAL_DISP_ApiPnlLinkType_e)pApiPanelType->ePanelLinkType;

        pDriverPanelType->m_bPanelDualPort    = pApiPanelType->bPanelDualPort;
        pDriverPanelType->m_bPanelSwapPort    = pApiPanelType->bPanelSwapPort;
        pDriverPanelType->m_bPanelSwapOdd_ML  = pApiPanelType->bPanelSwapOdd_ML;
        pDriverPanelType->m_bPanelSwapEven_ML = pApiPanelType->bPanelSwapEven_ML;
        pDriverPanelType->m_bPanelSwapOdd_RB  = pApiPanelType->bPanelSwapOdd_RB;
        pDriverPanelType->m_bPanelSwapEven_RB = pApiPanelType->bPanelSwapEven_RB;

        pDriverPanelType->m_bPanelSwapLVDS_POL = pApiPanelType->bPanelSwapLVDS_POL;
        pDriverPanelType->m_bPanelSwapLVDS_CH = pApiPanelType->bPanelSwapLVDS_CH;
        pDriverPanelType->m_bPanelPDP10BIT    = pApiPanelType->bPanelPDP10BIT;
        pDriverPanelType->m_bPanelLVDS_TI_MODE = pApiPanelType->bPanelLVDS_TI_MODE;

        pDriverPanelType->m_ucPanelDCLKDelay  = pApiPanelType->ucPanelDCLKDelay;
        pDriverPanelType->m_bPanelInvDCLK     = pApiPanelType->bPanelInvDCLK;
        pDriverPanelType->m_bPanelInvDE       = pApiPanelType->bPanelInvDE;
        pDriverPanelType->m_bPanelInvHSync    = pApiPanelType->bPanelInvHSync;
        pDriverPanelType->m_bPanelInvVSync    = pApiPanelType->bPanelInvVSync;

        pDriverPanelType->m_ucPanelDCKLCurrent = pApiPanelType->ucPanelDCKLCurrent;
        pDriverPanelType->m_ucPanelDECurrent  = pApiPanelType->ucPanelDECurrent;
        pDriverPanelType->m_ucPanelODDDataCurrent = pApiPanelType->ucPanelODDDataCurrent;
        pDriverPanelType->m_ucPanelEvenDataCurrent = pApiPanelType->ucPanelEvenDataCurrent;

        pDriverPanelType->m_wPanelOnTiming1   = pApiPanelType->wPanelOnTiming1;
        pDriverPanelType->m_wPanelOnTiming2   = pApiPanelType->wPanelOnTiming2;
        pDriverPanelType->m_wPanelOffTiming1  = pApiPanelType->wPanelOffTiming1;
        pDriverPanelType->m_wPanelOffTiming2  = pApiPanelType->wPanelOffTiming2;

        pDriverPanelType->m_ucPanelHSyncWidth = pApiPanelType->ucPanelHSyncWidth;
        pDriverPanelType->m_ucPanelHSyncBackPorch = pApiPanelType->ucPanelHSyncBackPorch;

        pDriverPanelType->m_ucPanelVSyncWidth = pApiPanelType->ucPanelVSyncWidth;
        pDriverPanelType->m_ucPanelVBackPorch = pApiPanelType->ucPanelVBackPorch;

        pDriverPanelType->m_wPanelHStart      = pApiPanelType->wPanelHStart;
        pDriverPanelType->m_wPanelVStart      = pApiPanelType->wPanelVStart;
        pDriverPanelType->m_wPanelWidth       = pApiPanelType->wPanelWidth;
        pDriverPanelType->m_wPanelHeight      = pApiPanelType->wPanelHeight;


        pDriverPanelType->m_wPanelMaxHTotal   = pApiPanelType->wPanelMaxHTotal;
        pDriverPanelType->m_wPanelHTotal      = pApiPanelType->wPanelHTotal;
        pDriverPanelType->m_wPanelMinHTotal   = pApiPanelType->wPanelMinHTotal;

        pDriverPanelType->m_wPanelMaxVTotal   = pApiPanelType->wPanelMaxVTotal;
        pDriverPanelType->m_wPanelVTotal      = pApiPanelType->wPanelVTotal;
        pDriverPanelType->m_wPanelMinVTotal   = pApiPanelType->wPanelMinVTotal;

        pDriverPanelType->m_dwPanelMaxDCLK    = pApiPanelType->dwPanelMaxDCLK;
        pDriverPanelType->m_dwPanelDCLK       = pApiPanelType->dwPanelDCLK;
        pDriverPanelType->m_dwPanelMinDCLK    = pApiPanelType->dwPanelMinDCLK;

        pDriverPanelType->m_wSpreadSpectrumStep   = pApiPanelType->wSpreadSpectrumStep;
        pDriverPanelType->m_wSpreadSpectrumSpan   = pApiPanelType->wSpreadSpectrumSpan;

        pDriverPanelType->m_ucDimmingCtl      = pApiPanelType->ucDimmingCtl;
        pDriverPanelType->m_ucMaxPWMVal       = pApiPanelType->ucMaxPWMVal;
        pDriverPanelType->m_ucMinPWMVal       = pApiPanelType->ucMinPWMVal;

        pDriverPanelType->m_bPanelDeinterMode = pApiPanelType->bPanelDeinterMode;

        // need change later
        pDriverPanelType->m_ucPanelAspectRatio = (MHAL_DISP_PnlAspectRatio_e)pApiPanelType->ucPanelAspectRatio;

        pDriverPanelType->m_u16LVDSTxSwapValue = pApiPanelType->u16LVDSTxSwapValue;

        // need change later
        pDriverPanelType->m_ucTiBitMode       = (MHAL_DISP_ApiPnlTiBitMode_e)pApiPanelType->ucTiBitMode;

        // need change later
        pDriverPanelType->m_ucOutputFormatBitMode = (MHAL_DISP_ApiPnlOutPutFormatBitMode_e)pApiPanelType->ucOutputFormatBitMode;

        pDriverPanelType->m_bPanelSwapOdd_RG  = pApiPanelType->bPanelSwapOdd_RG;
        pDriverPanelType->m_bPanelSwapEven_RG = pApiPanelType->bPanelSwapEven_RG;
        pDriverPanelType->m_bPanelSwapOdd_GB  = pApiPanelType->bPanelSwapOdd_GB;
        pDriverPanelType->m_bPanelSwapEven_GB = pApiPanelType->bPanelSwapEven_GB;

        pDriverPanelType->m_bPanelDoubleClk   = pApiPanelType->bPanelDoubleClk;
        pDriverPanelType->m_dwPanelMaxSET     = pApiPanelType->dwPanelMaxSET;
        pDriverPanelType->m_dwPanelMinSET     = pApiPanelType->dwPanelMinSET;

        // need change later
        pDriverPanelType->m_ucOutTimingMode   = (MHAL_DISP_ApiPnlOutTimingMode_e)pApiPanelType->ucOutTimingMode;
        pDriverPanelType->m_bPanelNoiseDith = pApiPanelType->bPanelNoiseDith;
    }
}

PanelInfo_t* mi_disp_GetPanelInfo(MI_BOOL bHdmiTx, EN_DISPLAYTIMING_RES_TYPE eTiming, MI_BOOL bActive)
{
    PanelInfo_t* pstPanelInfo = NULL;
    pstPanelInfo = MI_SYSCFG_GetPanelInfo(eTiming, bHdmiTx);

    return pstPanelInfo;
}
#if 1
MHAL_DISP_PanelConfig_t mi_disp_GetPanelAttr(MHAL_DISP_DeviceTiming_e eOutputTiming, MHAL_DISP_PanelConfig_t *pstPanelCfg)
{
    EN_DISPLAYTIMING_RES_TYPE eRes = DISPLAYTIMING_MAX_NUM;
    MHAL_DISP_PanelConfig_t stPanelConfig;
    PanelInfo_t* pstPanelInfo = NULL;
    memset(&stPanelConfig, 0, sizeof(MHAL_DISP_PanelConfig_t));
    stPanelConfig.eTiming = eOutputTiming;
    switch(eOutputTiming)
    {
        case E_MHAL_DISP_OUTPUT_PAL:
            eRes = DISPLAYTIMING_DACOUT_576I;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_CVBS;
            break;
        case E_MHAL_DISP_OUTPUT_NTSC:
            eRes = DISPLAYTIMING_DACOUT_480I;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_CVBS;
            break;
        case E_MHAL_DISP_OUTPUT_1080P24:
            eRes = DISPLAYTIMING_DACOUT_1080P_24;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080P25:
            eRes = DISPLAYTIMING_DACOUT_1080P_25;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080P30:
            eRes = DISPLAYTIMING_DACOUT_1080P_30;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_720P50:
            eRes = DISPLAYTIMING_DACOUT_720P_50;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_720P60:
            eRes = DISPLAYTIMING_DACOUT_720P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080I50:
            eRes = DISPLAYTIMING_DACOUT_1080I_50;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080I60:
            eRes = DISPLAYTIMING_DACOUT_1080I_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080P50:
            eRes = DISPLAYTIMING_DACOUT_1080P_50;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080P60:
            eRes = DISPLAYTIMING_DACOUT_1080P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_576P50:
            eRes = DISPLAYTIMING_DACOUT_576P;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_480P60:
            eRes = DISPLAYTIMING_DACOUT_480P;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_640x480_60:
            eRes = DISPLAYTIMING_VGAOUT_640x480P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_800x600_60:
            //eRes = DISPLAYTIMING_DACOUT_720P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1024x768_60:
            eRes = DISPLAYTIMING_VGAOUT_1024x768P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1280x1024_60:
            eRes = DISPLAYTIMING_VGAOUT_1280x1024P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1366x768_60:
            //eRes = DISPLAYTIMING_DACOUT_576P;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1440x900_60:
            eRes = DISPLAYTIMING_VGAOUT_1440x900P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1280x800_60:
            //eRes = DISPLAYTIMING_VGAOUT_640x480P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1680x1050_60:
            //eRes = DISPLAYTIMING_DACOUT_720P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1920x2160_30:
            //eRes = DISPLAYTIMING_VGAOUT_1024x768P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1600x1200_60:
            eRes = DISPLAYTIMING_VGAOUT_1600x1200P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1920x1200_60:
            //eRes = DISPLAYTIMING_DACOUT_576P;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_2560x1440_30:
            eRes = DISPLAYTIMING_DACOUT_2K2KP_30;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_2560x1600_60:
            eRes = DISPLAYTIMING_DACOUT_2K2KP_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_3840x2160_30:
            eRes = DISPLAYTIMING_DACOUT_4K2KP_30;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_3840x2160_60:
            eRes = DISPLAYTIMING_DACOUT_4K2KP_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        default:
            break;
    }
    if(stPanelConfig.u32OutputDev == MHAL_DISP_INTF_HDMI)
    {
        pstPanelInfo = mi_disp_GetPanelInfo(TRUE, eRes, FALSE);
    }
    else if(stPanelConfig.u32OutputDev == MHAL_DISP_INTF_CVBS)
    {
        pstPanelInfo = mi_disp_GetPanelInfo(TRUE, eRes, FALSE);
    }
    else if(stPanelConfig.u32OutputDev == MHAL_DISP_INTF_VGA)
    {
        pstPanelInfo = mi_disp_GetPanelInfo(TRUE, eRes, FALSE);
    }
    if(pstPanelInfo != NULL)
    {

        mi_disp_ConvertToDriverPanelTypeInfoTrans(&(pstPanelInfo->PanelAttr),&(stPanelConfig.stPanelAttr));
        stPanelConfig.bValid = TRUE;
    }
    memcpy(pstPanelCfg, &stPanelConfig, sizeof(*pstPanelCfg));
    return stPanelConfig;
}
MI_S32 mi_disp_ConfigInit(void)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_PanelConfig_t *pastPanelAttr;//[E_MHAL_DISP_OUTPUT_MAX];
    MHAL_DISP_PanelConfig_t *pastPanelAttr1;
    PanelInfo_t stPanelInfo;
    MHAL_DISP_DeviceTiming_e eOutputTiming = E_MHAL_DISP_OUTPUT_MAX;
    const MI_SYSCFG_MmapInfo_t* pstMmapInfo = NULL;
    MHAL_DISP_MmapType_e eMmType = E_MHAL_DISP_MMAP_MAX;
    MHAL_DISP_MmapInfo_t stMhalMmapInfo;
    memset(&stMhalMmapInfo, 0, sizeof(MHAL_DISP_MmapInfo_t));
    memset(&stPanelInfo, 0, sizeof(PanelInfo_t));
    pastPanelAttr = kmalloc((sizeof(MHAL_DISP_PanelConfig_t) * E_MHAL_DISP_OUTPUT_MAX), GFP_KERNEL);
    memset(pastPanelAttr, 0, (sizeof(MHAL_DISP_PanelConfig_t) * E_MHAL_DISP_OUTPUT_MAX));
    MI_SYS_BUG_ON(pastPanelAttr == NULL);
    for(eOutputTiming = E_MHAL_DISP_OUTPUT_PAL; eOutputTiming < E_MHAL_DISP_OUTPUT_MAX; eOutputTiming++)
    {
        //astPanelAttr[eOutputTiming] = mi_disp_GetPanelAttr(eOutputTiming);
        pastPanelAttr1 = pastPanelAttr + eOutputTiming;
        mi_disp_GetPanelAttr(eOutputTiming, pastPanelAttr1);

        printk("[%s %d] eTiming = %d, bValid = %d \n", __FUNCTION__, __LINE__, eOutputTiming, pastPanelAttr1->bValid == TRUE);
    }

    if(!MHAL_DISP_InitPanelConfig(pastPanelAttr, E_MHAL_DISP_OUTPUT_MAX))
    {
        printk("MHAL_DISP_InitPanelConfig Fail \n");
        s32Ret = E_MI_ERR_FAILED;
    }

    MI_SYSCFG_GetMmapInfo("E_MMAP_ID_XC_MAIN_FB", &pstMmapInfo);
    if(pstMmapInfo != NULL)
    {
        printk("Main buffer Addr =  0x%x \n", pstMmapInfo->u32Addr);
        printk("Main buffer size =  0x%x \n", pstMmapInfo->u32Size);
        printk("Main buffer u32Align =  0x%x \n", pstMmapInfo->u32Align);
        printk("Main buffer u8MiuNo =  0x%x \n", pstMmapInfo->u8MiuNo);
        eMmType = E_MHAL_DISP_MMAP_XC_MAIN;
        stMhalMmapInfo.u32Addr = pstMmapInfo->u32Addr;
        stMhalMmapInfo.u32Align = pstMmapInfo->u32Align;
        stMhalMmapInfo.u32MemoryType = pstMmapInfo->u32MemoryType;
        stMhalMmapInfo.u32Size = pstMmapInfo->u32Size;
        stMhalMmapInfo.u8CMAHid = pstMmapInfo->u8CMAHid;
        stMhalMmapInfo.u8Gid = pstMmapInfo->u8Gid;
        stMhalMmapInfo.u8Layer = pstMmapInfo->u8Layer;
        stMhalMmapInfo.u8MiuNo = pstMmapInfo->u8MiuNo;
        MHAL_DISP_InitMmapConfig(eMmType, &stMhalMmapInfo);
    }
    else
    {
        printk("Get XC Main buffer Fail \n");
        s32Ret = E_MI_ERR_FAILED;
    }

    pstMmapInfo = NULL;
    memset(&stMhalMmapInfo, 0, sizeof(MHAL_DISP_MmapInfo_t));
    MI_SYSCFG_GetMmapInfo("E_MMAP_ID_XC_MLOAD", &pstMmapInfo);
    if(pstMmapInfo != NULL)
    {
        printk("Main buffer Addr =  0x%x \n", pstMmapInfo->u32Addr);
        printk("Main buffer size =  0x%x \n", pstMmapInfo->u32Size);
        printk("Main buffer u32Align =  0x%x \n", pstMmapInfo->u32Align);
        printk("Main buffer u8MiuNo =  0x%x \n", pstMmapInfo->u8MiuNo);
        eMmType = E_MHAL_DISP_MMAP_XC_MENULOAD;
        stMhalMmapInfo.u32Addr = pstMmapInfo->u32Addr;
        stMhalMmapInfo.u32Align = pstMmapInfo->u32Align;
        stMhalMmapInfo.u32MemoryType = pstMmapInfo->u32MemoryType;
        stMhalMmapInfo.u32Size = pstMmapInfo->u32Size;
        stMhalMmapInfo.u8CMAHid = pstMmapInfo->u8CMAHid;
        stMhalMmapInfo.u8Gid = pstMmapInfo->u8Gid;
        stMhalMmapInfo.u8Layer = pstMmapInfo->u8Layer;
        stMhalMmapInfo.u8MiuNo = pstMmapInfo->u8MiuNo;
        MHAL_DISP_InitMmapConfig(eMmType, &stMhalMmapInfo);
    }
    else
    {
        printk("Get XC Menuload buffer Fail \n");
        s32Ret = E_MI_ERR_FAILED;
    }
    kfree(pastPanelAttr);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}
#else
MHAL_DISP_PanelConfig_t mi_disp_GetPanelAttr(MHAL_DISP_DeviceTiming_e eOutputTiming)
{
    EN_DISPLAYTIMING_RES_TYPE eRes = DISPLAYTIMING_MAX_NUM;
    MHAL_DISP_PanelConfig_t stPanelConfig;
    PanelInfo_t* pstPanelInfo = NULL;
    memset(&stPanelConfig, 0, sizeof(MHAL_DISP_PanelConfig_t));
    stPanelConfig.eTiming = eOutputTiming;
    switch(eOutputTiming)
    {
        case E_MHAL_DISP_OUTPUT_PAL:
            eRes = DISPLAYTIMING_DACOUT_576I;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_CVBS;
            break;
        case E_MHAL_DISP_OUTPUT_NTSC:
            eRes = DISPLAYTIMING_DACOUT_480I;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_CVBS;
            break;
        case E_MHAL_DISP_OUTPUT_1080P24:
            eRes = DISPLAYTIMING_DACOUT_1080P_24;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080P25:
            eRes = DISPLAYTIMING_DACOUT_1080P_25;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080P30:
            eRes = DISPLAYTIMING_DACOUT_1080P_30;
            break;
        case E_MHAL_DISP_OUTPUT_720P50:
            eRes = DISPLAYTIMING_DACOUT_720P_50;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_720P60:
            eRes = DISPLAYTIMING_DACOUT_720P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080I50:
            eRes = DISPLAYTIMING_DACOUT_1080I_50;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080I60:
            eRes = DISPLAYTIMING_DACOUT_1080I_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080P50:
            eRes = DISPLAYTIMING_DACOUT_1080P_50;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_1080P60:
            eRes = DISPLAYTIMING_DACOUT_1080P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_576P50:
            eRes = DISPLAYTIMING_DACOUT_576P;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_480P60:
            eRes = DISPLAYTIMING_DACOUT_480P;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_640x480_60:
            eRes = DISPLAYTIMING_VGAOUT_640x480P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_800x600_60:
            //eRes = DISPLAYTIMING_DACOUT_720P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1024x768_60:
            eRes = DISPLAYTIMING_VGAOUT_1024x768P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1280x1024_60:
            eRes = DISPLAYTIMING_VGAOUT_1280x1024P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1366x768_60:
            //eRes = DISPLAYTIMING_DACOUT_576P;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1440x900_60:
            eRes = DISPLAYTIMING_VGAOUT_1440x900P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1280x800_60:
            //eRes = DISPLAYTIMING_VGAOUT_640x480P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1680x1050_60:
            //eRes = DISPLAYTIMING_DACOUT_720P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1920x2160_30:
            //eRes = DISPLAYTIMING_VGAOUT_1024x768P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1600x1200_60:
            eRes = DISPLAYTIMING_VGAOUT_1600x1200P_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_1920x1200_60:
            //eRes = DISPLAYTIMING_DACOUT_576P;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_2560x1440_30:
            eRes = DISPLAYTIMING_DACOUT_2K2KP_30;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_2560x1600_60:
            eRes = DISPLAYTIMING_DACOUT_2K2KP_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_VGA;
            break;
        case E_MHAL_DISP_OUTPUT_3840x2160_30:
            eRes = DISPLAYTIMING_DACOUT_4K2KP_30;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        case E_MHAL_DISP_OUTPUT_3840x2160_60:
            eRes = DISPLAYTIMING_DACOUT_4K2KP_60;
            stPanelConfig.u32OutputDev = MHAL_DISP_INTF_HDMI;
            break;
        default:
            break;
    }
    if(stPanelConfig.u32OutputDev == MHAL_DISP_INTF_HDMI)
    {
        pstPanelInfo = mi_disp_GetPanelInfo(TRUE, eRes, FALSE);
    }
    else if(stPanelConfig.u32OutputDev == MHAL_DISP_INTF_CVBS)
    {
        pstPanelInfo = mi_disp_GetPanelInfo(TRUE, eRes, FALSE);
    }
    else if(stPanelConfig.u32OutputDev == MHAL_DISP_INTF_VGA)
    {
        pstPanelInfo = mi_disp_GetPanelInfo(TRUE, eRes, FALSE);
    }
    if(pstPanelInfo != NULL)
    {

        mi_disp_ConvertToDriverPanelTypeInfoTrans(&(pstPanelInfo->PanelAttr),&(stPanelConfig.stPanelAttr));
        stPanelConfig.bValid = TRUE;
    }
    return stPanelConfig;
}

MI_S32 mi_disp_ConfigInit(void)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MHAL_DISP_PanelConfig_t astPanelAttr[E_MHAL_DISP_OUTPUT_MAX];
    MI_U8 u8PanelCount = 0;
    PanelInfo_t stPanelInfo;
    MHAL_DISP_DeviceTiming_e eOutputTiming = E_MHAL_DISP_OUTPUT_MAX;
    EN_DISPLAYTIMING_RES_TYPE eRes = DISPLAYTIMING_MAX_NUM;
    MI_SYSCFG_MmapInfo_t* pstMmapInfo = NULL;
    MHAL_DISP_MmapType_e eMmType = E_MHAL_DISP_MMAP_MAX;
    MHAL_DISP_MmapInfo_t stMhalMmapInfo;
    memset(&stMhalMmapInfo, 0, sizeof(MHAL_DISP_MmapInfo_t));
    memset(&stPanelInfo, 0, sizeof(PanelInfo_t));
    memset(astPanelAttr, 0, (sizeof(MHAL_DISP_PanelConfig_t) * E_MHAL_DISP_OUTPUT_MAX));

    for(eOutputTiming = E_MHAL_DISP_OUTPUT_PAL; eOutputTiming < E_MHAL_DISP_OUTPUT_MAX; eOutputTiming++)
    {
        astPanelAttr[eOutputTiming] = mi_disp_GetPanelAttr(eOutputTiming);
        printk("[%s %d] eTiming = %d, bVaild = %d \n", __FUNCTION__, __LINE__, eOutputTiming, astPanelAttr[eOutputTiming].bValid == TRUE);
    }

    if(!MHAL_DISP_InitPanelConfig(astPanelAttr, E_MHAL_DISP_OUTPUT_MAX))
    {
        printk("MHAL_DISP_InitPanelConfig Fail \n");
        s32Ret = E_MI_ERR_FAILED;
    }

    MI_SYSCFG_GetMmapInfo("E_MMAP_ID_XC_MAIN_FB", &pstMmapInfo);
    if(pstMmapInfo != NULL)
    {
        printk("Main buffer Addr =  0x%x \n", pstMmapInfo->u32Addr);
        printk("Main buffer size =  0x%x \n", pstMmapInfo->u32Size);
        printk("Main buffer u32Align =  0x%x \n", pstMmapInfo->u32Align);
        printk("Main buffer u8MiuNo =  0x%x \n", pstMmapInfo->u8MiuNo);
        eMmType = E_MHAL_DISP_MMAP_XC_MAIN;
        stMhalMmapInfo.u32Addr = pstMmapInfo->u32Addr;
        stMhalMmapInfo.u32Align = pstMmapInfo->u32Align;
        stMhalMmapInfo.u32MemoryType = pstMmapInfo->u32MemoryType;
        stMhalMmapInfo.u32Size = pstMmapInfo->u32Size;
        stMhalMmapInfo.u8CMAHid = pstMmapInfo->u8CMAHid;
        stMhalMmapInfo.u8Gid = pstMmapInfo->u8Gid;
        stMhalMmapInfo.u8Layer = pstMmapInfo->u8Layer;
        stMhalMmapInfo.u8MiuNo = pstMmapInfo->u8MiuNo;
        MHAL_DISP_InitMmapConfig(eMmType, &stMhalMmapInfo);
    }
    else
    {
        printk("Get XC Main buffer Fail \n");
        s32Ret = E_MI_ERR_FAILED;
    }

    pstMmapInfo = NULL;
    memset(&stMhalMmapInfo, 0, sizeof(MHAL_DISP_MmapInfo_t));
    MI_SYSCFG_GetMmapInfo("E_MMAP_ID_XC_MLOAD", &pstMmapInfo);
    if(pstMmapInfo != NULL)
    {
        printk("Main buffer Addr =  0x%x \n", pstMmapInfo->u32Addr);
        printk("Main buffer size =  0x%x \n", pstMmapInfo->u32Size);
        printk("Main buffer u32Align =  0x%x \n", pstMmapInfo->u32Align);
        printk("Main buffer u8MiuNo =  0x%x \n", pstMmapInfo->u8MiuNo);
        eMmType = E_MHAL_DISP_MMAP_XC_MENULOAD;
        stMhalMmapInfo.u32Addr = pstMmapInfo->u32Addr;
        stMhalMmapInfo.u32Align = pstMmapInfo->u32Align;
        stMhalMmapInfo.u32MemoryType = pstMmapInfo->u32MemoryType;
        stMhalMmapInfo.u32Size = pstMmapInfo->u32Size;
        stMhalMmapInfo.u8CMAHid = pstMmapInfo->u8CMAHid;
        stMhalMmapInfo.u8Gid = pstMmapInfo->u8Gid;
        stMhalMmapInfo.u8Layer = pstMmapInfo->u8Layer;
        stMhalMmapInfo.u8MiuNo = pstMmapInfo->u8MiuNo;
        MHAL_DISP_InitMmapConfig(eMmType, &stMhalMmapInfo);
    }
    else
    {
        printk("Get XC Menuload buffer Fail \n");
        s32Ret = E_MI_ERR_FAILED;
    }
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

#endif
#if 0
static unsigned long long  i = 0;
long this_jiffies = 0;
#endif

static irqreturn_t MI_DISP_DevISR(int eIntNum, void* pstDevParam)
{
    MI_U8 i=0;
    mi_disp_DevStatus_t *pstDispDevParam = NULL;

    MHAL_DISP_IRQFlag_t stDevIrqFlag;
    MI_U32 u32IrqStatus = 0;
    struct timespec sttime;

    if(pstDevParam == NULL)
    {
        DBG_ERR("NULL Pointer: dev_id!!!\n");
    }
    else
    {
        pstDispDevParam = (mi_disp_DevStatus_t*)pstDevParam;
       // get IRQ status
        MHAL_DISP_GetDevIrqFlag(pstDispDevParam->pstDevObj, &stDevIrqFlag);
        if(FALSE == MHAL_DISP_ClearDevInterrupt(pstDispDevParam->pstDevObj, &stDevIrqFlag))
        {
           DBG_ERR("[%s %d] Clear Dev Int Fail!!!!\n", __FUNCTION__, __LINE__);
        }
        else
        {
            u32IrqStatus = stDevIrqFlag.u32IrqFlag & stDevIrqFlag.u32IrqMask;
            // deliver IRQ by status
            for(i =0; i < MI_DISP_LAYER_MAX; i++)
            {
                if ((u32IrqStatus & (0x01<<i)) != 0)
                {
                    // Status bits mmap to Layer Id
                    // Convert layer ID to Device Id
                    pstDispDevParam = &astDevStatus[astLayerParams[i].u8BindedDevID];
                    //-------------------------------- Body start ------------------------
                    if(pstDispDevParam->bDISPEnabled)
                    {
                        //记录中断的确切时间
                        // TODO: 记录frame fired 的时间戳
                        memset(&sttime, 0, sizeof(sttime));
                        do_posix_clock_monotonic_gettime(&sttime);
                        // TODO: interval 的位置
                        if(pstDispDevParam->u32VsyncInterval == 0)
                        {
                            pstDispDevParam->u64CurrentIntTimeStamp = ((MI_U64)sttime.tv_sec) * 1000000ULL + (sttime.tv_nsec / 1000);
                            pstDispDevParam->u64LastIntTimeStamp = pstDispDevParam->u64CurrentIntTimeStamp;
                            // TODO: Tommy hard code 20ms
                            pstDispDevParam->u32VsyncInterval = 20 * 1000;
                        }
                        else
                        {
                            pstDispDevParam->u64LastIntTimeStamp = pstDispDevParam->u64CurrentIntTimeStamp;
                            pstDispDevParam->u64CurrentIntTimeStamp = ((MI_U64)sttime.tv_sec)  * 1000000ULL + (sttime.tv_nsec / 1000);
                        }
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
                        if(pstDispDevParam->stDispCheckIsr.bCheckIsr == TRUE)
                        {
                            pstDispDevParam->stDispCheckIsr.u32IsrCnt++;
                        }
#endif
                        atomic_inc(&pstDispDevParam->disp_isr_waitqueue_red_flag);
                        WAKE_UP_QUEUE_IF_NECESSARY(pstDispDevParam->stWaitQueueHead);
                    }
                }
            }
        }
    }

    // -------------------------------------- end ------------------------------------------
    return IRQ_HANDLED;
}

static MI_S32 mi_disp_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
#if DISP_ENABLE_CUS_ALLOCATOR
    mi_disp_DevStatus_t *pstDispDev = (mi_disp_DevStatus_t*)pUsrData;
    mi_sys_Allocator_t *pstCusAllocator = mi_disp_layer_get_cus_allocator((MI_DISP_LAYER)pstChnCurPort->u32ChnId, pstChnCurPort->u32PortId);

    mi_sys_SetPeerOutputPortCusAllocator(pstDispDev->hDevSysHandle, pstChnCurPort->u32ChnId,
                 pstChnCurPort->u32PortId, pstCusAllocator);
    UNUSED(pstChnPeerPort);
#else
    UNUSED(pstChnCurPort);
    UNUSED(pstChnPeerPort);
    UNUSED(pUsrData);
#endif
    return MI_SUCCESS;
}
static MI_S32 mi_disp_OnUnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_disp_DevStatus_t *pstDispDev = (mi_disp_DevStatus_t*)pUsrData;

    mi_sys_SetPeerOutputPortCusAllocator(pstDispDev->hDevSysHandle, pstChnCurPort->u32ChnId,
                 pstChnCurPort->u32PortId, NULL);
    UNUSED(pstChnPeerPort);
    return MI_SUCCESS;
}

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
MI_S32 mi_disp_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_U8 i = 0;
    MI_U8 u8TmpStr[20];
    MI_U8 u8IntfIdx = 0;
    MI_DISP_Interface_e eInterface[3];

    handle.OnPrintOut(handle,"============================== Private DISP%d Info ============================\n",u32DevId);
    handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s\n","DevStatus","IrqNum","IrqStatus","BgColor","CvbsStatus");
    handle.OnPrintOut(handle,"%10d",astDevStatus[u32DevId].bDISPEnabled);
    handle.OnPrintOut(handle,"%15d",astDevStatus[u32DevId].u32DevIrq);
    handle.OnPrintOut(handle,"%15d",astDevStatus[u32DevId].bIrqEnabled);
    handle.OnPrintOut(handle,"%15d",astDevStatus[u32DevId].u32BgColor);
    handle.OnPrintOut(handle,"%15d\n",astDevStatus[u32DevId].bCvbsEnable);

    while((astDevStatus[u32DevId].u32Interface >> i) != 0)
    {
        if(((astDevStatus[u32DevId].u32Interface >> i) & 1) != 0)
        {
            sprintf(u8TmpStr, "%s%d","Interface", u8IntfIdx);
            handle.OnPrintOut(handle, "%10s", u8TmpStr);
            memset(u8TmpStr, 0, sizeof(u8TmpStr));
            sprintf(u8TmpStr, "%s%d", "DevTiming", u8IntfIdx);
            handle.OnPrintOut(handle, "%15s", u8TmpStr);
            eInterface[u8IntfIdx] = (MI_DISP_Interface_e)i;
            u8IntfIdx++;
        }
        i++;
    }
    handle.OnPrintOut(handle,"%15s%15s%15s\n","CscMatrix","Luma","Contrast");
    for(i = 0; i < u8IntfIdx; i++)
    {
        handle.OnPrintOut(handle,"%10s",PARSING_DISP_INTERFACE(eInterface[i]));
        handle.OnPrintOut(handle,"%15s",PARSING_DISP_OUTPUT_TIMING(astDevStatus[u32DevId].eDeviceTiming[eInterface[i]]));
    }
    handle.OnPrintOut(handle,"%15d",astDevStatus[u32DevId].eCscMatrix);
    handle.OnPrintOut(handle,"%15d",astDevStatus[u32DevId].u32Luma);
    handle.OnPrintOut(handle,"%15d\n",astDevStatus[u32DevId].u32Contrast);

    handle.OnPrintOut(handle,"%10s%15s%15s%15s\n","Hue","Saturation","Gain","Sharpness");
    handle.OnPrintOut(handle,"%10d",astDevStatus[u32DevId].u32Hue);
    handle.OnPrintOut(handle,"%15d",astDevStatus[u32DevId].u32Saturation);
    handle.OnPrintOut(handle,"%15d",astDevStatus[u32DevId].u32Gain);
    handle.OnPrintOut(handle,"%15d\n",astDevStatus[u32DevId].u32Sharpness);
    return MI_SUCCESS;
}

MI_S32 mi_disp_OnDumpChannelAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_U8 u8LayerId = 0;
    mi_disp_LayerStatus_t *pstTmpLayer = NULL;
    mi_disp_DevStatus_t* pstDispDevParam = NULL;

    pstDispDevParam = &astDevStatus[u32DevId];
    if(!list_empty(&pstDispDevParam->stBindedLayer))
    {
        struct list_head *pstPos = NULL;
        struct list_head *n = NULL;
        handle.OnPrintOut(handle,"--------------------------------- Layer Info ---------------------------------\n");
        handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s%15s\n","LayerId","LayerStatus","BindedDevID","ePixFormat","Compress","LayerWidth","LayerHeight");
        list_for_each_safe(pstPos, n, &pstDispDevParam->stBindedLayer)
        {
            pstTmpLayer = list_entry(pstPos, mi_disp_LayerStatus_t, stLayerNode);
            u8LayerId = pstTmpLayer->u8LayerID;
            if(pstTmpLayer->bLayerEnabled == TRUE)
            {
                handle.OnPrintOut(handle,"%10d",u8LayerId);
                handle.OnPrintOut(handle,"%15d",pstTmpLayer->bLayerEnabled);
                handle.OnPrintOut(handle,"%15d",pstTmpLayer->u8BindedDevID);
                handle.OnPrintOut(handle,"%15d",pstTmpLayer->stVideoLayerAttr.ePixFormat);
                handle.OnPrintOut(handle,"%15d",pstTmpLayer->bCompress);
                handle.OnPrintOut(handle,"%15d",pstTmpLayer->stVideoLayerAttr.stVidLayerSize.u16Width);
                handle.OnPrintOut(handle,"%15d\n",pstTmpLayer->stVideoLayerAttr.stVidLayerSize.u16Height);
            }
        }
        handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s\n","LayerId","LayDispWidth","LayDispHeight","Toleration","customer_alloc");
        list_for_each_safe(pstPos, n, &pstDispDevParam->stBindedLayer)
        {
            pstTmpLayer = list_entry(pstPos, mi_disp_LayerStatus_t, stLayerNode);
            u8LayerId = pstTmpLayer->u8LayerID;
            handle.OnPrintOut(handle,"%10d",u8LayerId);
            handle.OnPrintOut(handle,"%15d",pstTmpLayer->stVideoLayerAttr.stVidLayerDispWin.u16Width);
            handle.OnPrintOut(handle,"%15d",pstTmpLayer->stVideoLayerAttr.stVidLayerDispWin.u16Height);
            handle.OnPrintOut(handle,"%15d",pstTmpLayer->u32Toleration);
#if DISP_ENABLE_CUS_ALLOCATOR
            handle.OnPrintOut(handle,"%15s\n","enable");
#else
            handle.OnPrintOut(handle,"%15s\n","disable");
#endif
        }
    }
    return MI_SUCCESS;
}

MI_S32 mi_disp_OnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_U8 u8LayerId=0,u32InPortId = 0;

    for(u8LayerId = 0; u8LayerId < MI_DISP_VIDEO_LAYER_MAX; u8LayerId++)
    {
        if(astLayerParams[u8LayerId].bLayerEnabled == TRUE)
        {
            handle.OnPrintOut(handle,"--------------------------------- Layer%d InputPort Info ---------------------------------\n",u8LayerId);
        }
        else
            continue;
        handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s\n","PortId","PortStatus","DispWin_x","DispWin_y","DispWinWidth","DispWinHeight");
        for(u32InPortId = 0; u32InPortId < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u32InPortId++)
        {
            if(astLayerParams[u8LayerId].astPortStatus[u32InPortId].bEnable == TRUE)
            {
                handle.OnPrintOut(handle,"%10d",u32InPortId);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].eStatus);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stDispWin.u16X);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stDispWin.u16Y);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stDispWin.u16Width);
                handle.OnPrintOut(handle,"%15d\n",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stDispWin.u16Height);

            }
        }
        handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s\n","PortId","ZoomWin_x","ZoomWin_y","ZoomWinWidth","ZoomWinHeight","SyncMode");
        for(u32InPortId = 0; u32InPortId < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u32InPortId++)
        {
            if(astLayerParams[u8LayerId].astPortStatus[u32InPortId].bEnable == TRUE)
            {
                handle.OnPrintOut(handle,"%10d",u32InPortId);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stCropWin.u16X);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stCropWin.u16Y);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stCropWin.u16Width);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stCropWin.u16Height);
                handle.OnPrintOut(handle,"%15d\n",astLayerParams[u8LayerId].astPortStatus[u32InPortId].eMode);
            }
        }
        handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s\n","PortId","FlipByGe","RecvBufCnt","RecvBufWidth","RecvBufHeight","RecvBufStride");
        for(u32InPortId = 0; u32InPortId < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u32InPortId++)
        {
            if(astLayerParams[u8LayerId].astPortStatus[u32InPortId].bEnable == TRUE)
            {
                handle.OnPrintOut(handle,"%10d",u32InPortId);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].bGeFlip);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stDispCheckBuffInfo.u32InportRecvBufCnt);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stDispCheckBuffInfo.u32RecvBufWidth);
                handle.OnPrintOut(handle,"%15d",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stDispCheckBuffInfo.u32RecvBufHeight);
                handle.OnPrintOut(handle,"%15d\n",astLayerParams[u8LayerId].astPortStatus[u32InPortId].stDispCheckBuffInfo.u32RecvBufStride);
            }
        }
    }
    return MI_SUCCESS;
}

MI_S32 mi_disp_OnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    return MI_SUCCESS;
}

MI_S32 mi_disp_OnHelp(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    handle.OnPrintOut(handle,"CatModuleInfo: cat /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"SetSyncMode: echo setsyncmode [layerid] [portid] [mode] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"SetIrqStatus: echo setIrqStatus [status] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"SetToleration: echo settoleration [layerid] [portid] [value] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"GetCapFrame: echo getcapframe [devid] [layerid] [path] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"SetLayerAttr: echo setlayer [layerid] [lay_width] [lay_height] [laydisp_x] [laydisp_y] [laydisp_width] [laydisp_height] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"SetLayerStatus: echo setlayerstatus [layerid] [status] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"SetCsc: echo setcsc [devid] [CscMatrix] [Contrast] [Hue] [Luma] [Saturation] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"SetBgcolor: echo setbgcolor [devid] [value] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"CheckFramePts: echo checkframepts [layeid] [portid] [ON/OFF] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"CheckFrameRate: echo checkframerate [layerid] [portid] [ON/OFF] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"Dumpframe: echo dumpframe [layerid] [portid] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    handle.OnPrintOut(handle,"StopGetBuff: echo stopgetbuff [layerid] [portid] [ON/OFF] > /proc/mi_modules/mi_disp/mi_disp%d\n",u32DevId);
    return MI_SUCCESS;
}

/*
customer_alloc <on/off> <buff number>
irq <on/off>(device id)
syncmode <layer id> <port ID> <checkpts/freerun>
toleration <layer id> <port id> <Ms>
screen <device id> <layer id> <output path>
*/

MI_S32 mi_disp_DebugSetIrqStatus(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_BOOL bIrqEnable;
    mi_disp_DevStatus_t* pstDispDev = NULL;

    MI_DISP_CHECK_NULLPOINTER(pUsrData);
    pstDispDev = (mi_disp_DevStatus_t*)pUsrData;

    if(argc < 2)
    {
        DISP_DBG_ERR("[0] off IRQ / [1] on IRQ\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    bIrqEnable = (MI_BOOL)simple_strtoul(argv[1], NULL, 10);
    handle.OnPrintOut(handle, " bIrqEnable  (%d)\r\n",  bIrqEnable);

    if(0 == bIrqEnable)
    {
        pstDispDev->bProcDisableIrq = TRUE;
    }
    else if(1 == bIrqEnable)
    {
        pstDispDev->bProcDisableIrq = FALSE;
    }
    else
    {
        DISP_DBG_ERR("[0] off IRQ / [1] on IRQ");
        goto EXIT;
    }
    s32Ret = MI_SUCCESS;

EXIT:
    return s32Ret;
}

MI_S32 mi_disp_DebugSetSyncMode(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8LayerId = 0;
    MI_U8 u8PortId = 0;
    MI_DISP_SyncMode_e eMod;

    if(argc < 4)
    {
        DISP_DBG_ERR("1.LAYERID 2.PORTID 3.[1]CHECK PTS/[2] FREERUN\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    u8LayerId = (MI_U8)simple_strtoul(argv[1], NULL, 10);
    u8PortId = (MI_U8)simple_strtoul(argv[2], NULL, 10);
    eMod = (MI_DISP_SyncMode_e)simple_strtoul(argv[3], NULL, 10);
    handle.OnPrintOut(handle, "u8LayerId(%d), u8PortId(%d), eMod(%d)\r\n", u8LayerId, u8PortId, eMod);

    astLayerParams[u8LayerId].astPortStatus[u8PortId].eMode = eMod;
    MI_DISP_IMPL_SetInputPortSyncMode(u8LayerId, u8PortId, eMod);

    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 mi_disp_DebugSetToleration(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8LayerId = 0;
    MI_U8 u8PortId = 0;
    MI_U32 u32Toleration;

    if(argc < 4)
    {
        DISP_DBG_ERR("1.LAYERID 2.PORTID 3.u32Toleration(ms)\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    u8LayerId = (MI_U8)simple_strtoul(argv[1], NULL, 10);
    u8PortId = (MI_U8)simple_strtoul(argv[2], NULL, 10);// tolleration port not
    u32Toleration = (MI_U32)simple_strtoul(argv[3], NULL, 10);

    handle.OnPrintOut(handle, "u8LayerId(%d), u8PortId(%d), u32Toleration(%d)\r\n", u8LayerId, u8PortId, u32Toleration);
    MI_DISP_IMPL_SetPlayToleration(u8LayerId, u32Toleration);

    s32Ret = MI_SUCCESS;

EXIT:
    return s32Ret;
}

MI_S32 mi_disp_DebugGetScreenFrame(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8DeviceId = 0;
    MI_U8 u8LayerId = 0;
    char *FilePath;
    MI_DISP_VideoFrame_t stScreenVideoFrame;
    MI_U32 u32Size = 0;

    if(argc < 4)
    {
        DISP_DBG_ERR("1.DeviceID 2.LAYERID 3.SCREEN FILE PATH\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    u8DeviceId = (MI_U8)simple_strtoul(argv[1], NULL, 10);
    u8LayerId = (MI_U8)simple_strtoul(argv[2], NULL, 10);
    FilePath = argv[3];
    printk("DeviceID(%d), LayerID(%d), FilePath(%s)\r\n", u8DeviceId, u8LayerId, FilePath);

    u32Size = astLayerParams[u8LayerId].stVideoLayerAttr.stVidLayerSize.u16Height * astLayerParams[u8LayerId].stVideoLayerAttr.stVidLayerSize.u16Width * 2;
    stScreenVideoFrame.u32Height = astLayerParams[u8LayerId].stVideoLayerAttr.stVidLayerSize.u16Height;
    stScreenVideoFrame.u32Width = astLayerParams[u8LayerId].stVideoLayerAttr.stVidLayerSize.u16Width;
    stScreenVideoFrame.u32Size = u32Size;//buffer size
    stScreenVideoFrame.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stScreenVideoFrame.u32Stride = astLayerParams[u8LayerId].stVideoLayerAttr.stVidLayerSize.u16Width * 2;

    if (MI_SUCCESS != mi_sys_MMA_Alloc(NULL, u32Size, (MI_PHY *)&stScreenVideoFrame.aphyAddr))
    {
        DISP_DBG_ERR("[BUF_ERROR]Error!! DISP SCREEN LAYER BUFFER NULL!\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }
    stScreenVideoFrame.pavirAddr = (MI_U8 *)mi_sys_Vmap((MI_PHY)stScreenVideoFrame.aphyAddr, u32Size, FALSE);
    if (NULL == stScreenVideoFrame.pavirAddr)
    {
        DISP_DBG_ERR("[BUF_ERROR]Error!! map DISP SCREEN LAYER BUFFER!\n");
        mi_sys_MMA_Free((MI_PHY)stScreenVideoFrame.aphyAddr);
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    MI_DISP_IMPL_GetScreenFrame(u8LayerId, &stScreenVideoFrame);

    {
        struct file *pFile = NULL;
        mm_segment_t tFs;
        loff_t pos = 0;
        ssize_t writeBytes = 0;
        pFile = filp_open(FilePath, O_RDWR | O_CREAT,0644);

        if (IS_ERR(pFile))
        {
            DISP_DBG_ERR("Create file:%s error.\n", FilePath);
            s32Ret = E_MI_ERR_FAILED;
            goto EXIT;
        }

        tFs = get_fs();
        set_fs(KERNEL_DS);
        writeBytes = vfs_write(pFile, (char*)stScreenVideoFrame.pavirAddr, stScreenVideoFrame.u32Size, &pos);
        filp_close(pFile, NULL);
        set_fs(tFs);
    }

     s32Ret = MI_SUCCESS;
EXIT:
     return s32Ret;
}

MI_S32 mi_disp_DebugSetLayer(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_LAYER DispLayer;
    MI_DISP_VideoLayerAttr_t stVideoLayerAttr;

    if(argc < 8)
    {
        DISP_DBG_ERR("1.LayerID 2.LayerWidth 3.LayerHeight 4.LayerDisp_x 5.LayerDisp_y 6.LayerDispWidth 7.LayerDispHeight\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    DispLayer = (MI_DISP_LAYER)simple_strtoul(argv[1], NULL, 10);

    if(MI_DISP_GetVideoLayerAttr(DispLayer, &stVideoLayerAttr) != MI_SUCCESS)
    {
        s32Ret = E_MI_ERR_FAILED;
        DISP_DBG_ERR("MI_DISP_GetVideoLayerAttr Fail !!!\n");
        goto EXIT;
    }

    stVideoLayerAttr.stVidLayerSize.u16Width = (MI_U16)simple_strtoul(argv[2], NULL, 10);
    stVideoLayerAttr.stVidLayerSize.u16Height = (MI_U16)simple_strtoul(argv[3], NULL, 10);
    stVideoLayerAttr.stVidLayerDispWin.u16X = (MI_U16)simple_strtoul(argv[4], NULL, 10);
    stVideoLayerAttr.stVidLayerDispWin.u16Y = (MI_U16)simple_strtoul(argv[5], NULL, 10);
    stVideoLayerAttr.stVidLayerDispWin.u16Width = (MI_U16)simple_strtoul(argv[6], NULL, 10);
    stVideoLayerAttr.stVidLayerDispWin.u16Height = (MI_U16)simple_strtoul(argv[7], NULL, 10);

    if(MI_DISP_IMPL_SetVideoLayerAttr(DispLayer,&stVideoLayerAttr) != MI_SUCCESS)
    {
        s32Ret = E_MI_ERR_FAILED;
        DISP_DBG_ERR("MI_DISP_IMPL_SetVideoLayerAttr Fail !!!\n");
        goto EXIT;
    }

    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 mi_disp_DebugSetLayerStatus(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_BOOL bLayerStatus;
    MI_DISP_LAYER DispLayer;

    if(argc < 3)
    {
        DISP_DBG_ERR("1.LayerID 2.LayerStatus\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }

    DispLayer = (MI_DISP_LAYER)simple_strtoul(argv[1], NULL, 10);
    bLayerStatus = (MI_BOOL)simple_strtoul(argv[2], NULL, 10);
    astLayerParams[DispLayer].bLayerEnabled = bLayerStatus;

    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 mi_disp_DebugSetCsc(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_DEV DispDev;
    MI_DISP_HdmiParam_t stHdmiParam;
    if(argc < 7)
    {
        DISP_DBG_ERR("1.DeviceID 2.CscMatrix 3.Contrast 4.Hue 5.Luma 6.Saturation\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }
    DispDev = (MI_DISP_DEV)simple_strtoul(argv[1], NULL, 10);

    if(MI_DISP_IMPL_GetHdmiParam(DispDev,&stHdmiParam) != MI_SUCCESS)
    {
        s32Ret = E_MI_ERR_FAILED;
        DISP_DBG_ERR("MI_DISP_IMPL_GetHdmiParam Fail !!!\n");
        goto EXIT;
    }

    stHdmiParam.stCsc.eCscMatrix = (MI_DISP_CscMattrix_e)simple_strtoul(argv[2], NULL, 10);
    stHdmiParam.stCsc.u32Contrast = (MI_U32)simple_strtoul(argv[3], NULL, 10);
    stHdmiParam.stCsc.u32Hue = (MI_U32)simple_strtoul(argv[4], NULL, 10);
    stHdmiParam.stCsc.u32Luma = (MI_U32)simple_strtoul(argv[5], NULL, 10);
    stHdmiParam.stCsc.u32Saturation = (MI_U32)simple_strtoul(argv[6], NULL, 10);

    if(MI_DISP_IMPL_SetHdmiParam(DispDev,&stHdmiParam) != MI_SUCCESS)
    {
        s32Ret = E_MI_ERR_FAILED;
        DISP_DBG_ERR("MI_DISP_IMPL_SetHdmiParam Fail !!!\n");
        goto EXIT;
    }

    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 mi_disp_DebugSetBgColor(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_DISP_DEV DispDev;
    MI_U32 u32BgColor;
    if(argc < 3)
    {
        DISP_DBG_ERR("1.DeviceID 2.Bgcolor\n");
        s32Ret = E_MI_ERR_FAILED;
        goto EXIT;
    }
    DispDev = (MI_DISP_DEV)simple_strtoul(argv[1], NULL, 10);
    u32BgColor = (MI_U32)simple_strtoul(argv[2], NULL, 10);
    if(MHAL_DISP_DeviceSetBackGroundColor((astDevStatus[DispDev].pstDevObj), u32BgColor) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DISP_DBG_ERR("Hal Set backcolor fail!!!\n");
        goto EXIT;
    }
    astDevStatus[DispDev].u32BgColor = u32BgColor;

    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

void _mi_disp_DumpFrame(MI_SYS_BufInfo_t *pstBufInfo, MI_U8 LayerID, MI_U8 InPortID)
{
    MI_S8 s8FilePath[128];
    FILE_HANDLE hFileHandle;
    MI_SYS_BufRef_t *pstBufRef;
    MI_SYS_BufferAllocation_t* pstAllocation;

    if(astLayerParams[LayerID].astPortStatus[InPortID].bDumpFrame == FALSE)
        return;
    astLayerParams[LayerID].astPortStatus[InPortID].bDumpFrame = FALSE;
    
    sprintf(s8FilePath,"/mnt/dump_disp_layer%d_inport%d_frame.yuv",LayerID,InPortID);
    hFileHandle = open_yuv_file(s8FilePath, 1);
    if(hFileHandle == NULL)
    {
        printk("disp dumpframe open file failed\n");
        return;
    }

    pstBufRef = container_of(pstBufInfo, MI_SYS_BufRef_t, bufinfo);
    pstAllocation = pstBufRef->pstBufAllocation;
    if(pstBufInfo->stFrameData.pVirAddr[0] == NULL)
    {
        pstBufInfo->stFrameData.pVirAddr[0] = pstAllocation->ops.vmap_kern(pstAllocation);
        pstBufInfo->stFrameData.pVirAddr[1] = (char*)pstBufInfo->stFrameData.pVirAddr[0] + (pstBufInfo->stFrameData.phyAddr[1] - pstBufInfo->stFrameData.phyAddr[0]);
        pstBufInfo->stFrameData.pVirAddr[2] = (char*)pstBufInfo->stFrameData.pVirAddr[0] + (pstBufInfo->stFrameData.phyAddr[2] - pstBufInfo->stFrameData.phyAddr[0]);
    }
    
    if(write_yuv_file(hFileHandle , pstBufInfo->stFrameData) < 0)
    {
        printk("disp dump frame write file failed");
        return;
    }
    
    close_yuv_file(hFileHandle);
    pstAllocation->ops.vunmap_kern(pstAllocation);
}
void _mi_disp_CheckFramePts(MI_SYS_BufInfo_t *pstBufInfo, MI_U8 LayerID, MI_U8 InPortID)
{
    stDispCheckFramePts_t *pstCheckFramePts = &astLayerParams[LayerID].astPortStatus[InPortID].stDispCheckFramePts;

    if(pstCheckFramePts->bCheckFramePts == TRUE)
    {
        pstCheckFramePts->u32CheckFramePtsBufCnt ++;
        pstCheckFramePts->u64Pts = pstBufInfo->u64Pts;

        PRINTF_PROC("LayerID %d, PortID %d, receive buffer ID = %u, PTS = %lld\n",
            LayerID, InPortID, pstCheckFramePts->u32CheckFramePtsBufCnt,
            pstCheckFramePts->u64Pts);
    }
    
}
void _mi_disp_CheckFrameRate(MI_SYS_BufInfo_t *pstBufInfo, MI_U8 LayerID, MI_U8 InPortID)
{
    stDispCheckFrameRate_t *pstCheckFrameRate = &astLayerParams[LayerID].astPortStatus[InPortID].stDispCheckFrameRate;

    if(pstCheckFrameRate->bstat == TRUE)
    {
        MI_U64 u64IntervalTime = 0;
        struct timespec sttime;
        memset(&sttime, 0, sizeof(sttime));
        do_posix_clock_monotonic_gettime(&sttime);
        pstCheckFrameRate->u64CurrentInputFrameTime = ((MI_U64)sttime.tv_sec) * 1000000ULL + (sttime.tv_nsec / 1000);

        if(pstCheckFrameRate->u64LastInputFrameTime != 0)
        {
            u64IntervalTime = pstCheckFrameRate->u64CurrentInputFrameTime - pstCheckFrameRate->u64LastInputFrameTime;
            pstCheckFrameRate->u32GetBufCnt++;

            //PRINTF_PROC("interval time %llu \n", u64IntervalTime);

            if(u64IntervalTime > pstCheckFrameRate->u64InterValTimeMAX[2])
            {
                pstCheckFrameRate->u64InterValTimeMAX[0] = pstCheckFrameRate->u64InterValTimeMAX[1];
                pstCheckFrameRate->u64InterValTimeMAX[1] = pstCheckFrameRate->u64InterValTimeMAX[2];
                pstCheckFrameRate->u64InterValTimeMAX[2] = u64IntervalTime;
            }
            else if(u64IntervalTime > pstCheckFrameRate->u64InterValTimeMAX[1])
            {
                pstCheckFrameRate->u64InterValTimeMAX[0] = pstCheckFrameRate->u64InterValTimeMAX[1];
                pstCheckFrameRate->u64InterValTimeMAX[1] = u64IntervalTime;
            }
            else if(u64IntervalTime > pstCheckFrameRate->u64InterValTimeMAX[0])
            {
                pstCheckFrameRate->u64InterValTimeMAX[0] = u64IntervalTime;
            }

            if(u64IntervalTime < pstCheckFrameRate->u64InterValTimeMIN[2])
            {
                pstCheckFrameRate->u64InterValTimeMIN[0] = pstCheckFrameRate->u64InterValTimeMIN[1];
                pstCheckFrameRate->u64InterValTimeMIN[1] = pstCheckFrameRate->u64InterValTimeMIN[2];
                pstCheckFrameRate->u64InterValTimeMIN[2] = u64IntervalTime;
            }
            else if(u64IntervalTime < pstCheckFrameRate->u64InterValTimeMIN[1])
            {
                pstCheckFrameRate->u64InterValTimeMIN[0] = pstCheckFrameRate->u64InterValTimeMIN[1];
                pstCheckFrameRate->u64InterValTimeMIN[1] = u64IntervalTime;
            }
            else if(u64IntervalTime < pstCheckFrameRate->u64InterValTimeMIN[0])
            {
                pstCheckFrameRate->u64InterValTimeMIN[0] = u64IntervalTime;
            }
        }
        pstCheckFrameRate->u64InterValTimeSum += u64IntervalTime;
        pstCheckFrameRate->u64LastInputFrameTime = pstCheckFrameRate->u64CurrentInputFrameTime;
    }
}

void _mi_disp_CheckFrameRateTimer(unsigned long arg)
{
    MI_U64 u64AverageIntervalFrametime = 0;
    MI_U8 u8LayerID = 0;
    MI_U8 u8PortID = 0;
    stDispCheckFrameRate_t *pstCheckFrameRate;

    PRINTF_PROC("%10s%10s%15s%15s%15s%15s%10s%10s%10s%10s%10s%10s\n","LayerID","PortID","GetBufCnt_1s","IsrCnt_1s","FlipBufCnt_1s","Average time",
                "MAX_0","MAX_1","MAX_2","MIN_0","MIN_1","MIN_2");
    for(u8LayerID = 0; u8LayerID < MI_DISP_LAYER_MAX; u8LayerID ++)
    {
        for(u8PortID = 0; u8PortID < MI_DISP_INPUTPORT_MAX; u8PortID++)
        {
            mi_disp_DevStatus_t *pstDispDevParam = &astDevStatus[astLayerParams[u8LayerID].u8BindedDevID];
            pstCheckFrameRate = &astLayerParams[u8LayerID].astPortStatus[u8PortID].stDispCheckFrameRate;
            if(pstCheckFrameRate->bstat != TRUE)
                continue;
            u64AverageIntervalFrametime = div_u64(pstCheckFrameRate->u64InterValTimeSum, pstCheckFrameRate->u32GetBufCnt);
            if(pstCheckFrameRate->bFirstSet == FALSE)
            {
                PRINTF_PROC("%10d%10d%15d%15d%15d%15llu%10llu%10llu%10llu%10llu%10llu%10llu\n",u8LayerID,u8PortID,pstCheckFrameRate->u32GetBufCnt,pstDispDevParam->stDispCheckIsr.u32IsrCnt,pstCheckFrameRate->u32FlipBufCnt,
                             u64AverageIntervalFrametime,pstCheckFrameRate->u64InterValTimeMAX[0],pstCheckFrameRate->u64InterValTimeMAX[1],pstCheckFrameRate->u64InterValTimeMAX[2],
                             pstCheckFrameRate->u64InterValTimeMIN[0],pstCheckFrameRate->u64InterValTimeMIN[1],pstCheckFrameRate->u64InterValTimeMIN[2]);
            }
            else
                pstCheckFrameRate->bFirstSet = FALSE;

            pstCheckFrameRate->u32GetBufCnt = 0;
            pstCheckFrameRate->u32FlipBufCnt = 0;
            pstDispDevParam->stDispCheckIsr.u32IsrCnt = 0;
            pstCheckFrameRate->u64InterValTimeSum = 0;
            pstCheckFrameRate->u64InterValTimeMAX[2] = 0;
            pstCheckFrameRate->u64InterValTimeMAX[1] = 0;
            pstCheckFrameRate->u64InterValTimeMAX[0] = 0;
            pstCheckFrameRate->u64InterValTimeMIN[2] = -1;
            pstCheckFrameRate->u64InterValTimeMIN[1] = -1;
            pstCheckFrameRate->u64InterValTimeMIN[0] = -1;
            pstCheckFrameRate->u64CurrentInputFrameTime = 0;
            pstCheckFrameRate->u64LastInputFrameTime = 0;
        }
    }
    mod_timer(&gstDispCheckFrameRateTimer.Timer,jiffies+HZ);//set 1s timeout
}

MI_S32 mi_disp_DebugCheckFrameRate(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8LayerID = 0;
    MI_U8 u8PortID = 0;
    stDispCheckFrameRate_t *pstCheckFrameRate;
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if(argc > 3)
    {
        u8LayerID = simple_strtoul(argv[1],NULL,10);
        u8PortID = simple_strtoul(argv[2],NULL,10);
        MI_DISP_CHECK_INVAILDLAYER(u8LayerID);
        MI_DISP_CHECK_INVAILDINPUTPORT(u8PortID);

        pstCheckFrameRate = &astLayerParams[u8LayerID].astPortStatus[u8PortID].stDispCheckFrameRate;
        if(astLayerParams[u8LayerID].astPortStatus[u8PortID].bEnable != TRUE)
        {
            goto EXIT;
        }
        if (strcmp(argv[3], "ON") == 0 && pstCheckFrameRate->bstat == FALSE)
        {
            pstCheckFrameRate->bstat = TRUE;
            astDevStatus[u32DevId].stDispCheckIsr.bCheckIsr = TRUE;
            pstCheckFrameRate->u32GetBufCnt = 0;
            pstCheckFrameRate->u64InterValTimeSum = 0;
            pstCheckFrameRate->u64InterValTimeMAX[2] = 0;
            pstCheckFrameRate->u64InterValTimeMAX[1] = 0;
            pstCheckFrameRate->u64InterValTimeMAX[0] = 0;
            pstCheckFrameRate->u64InterValTimeMIN[2] = -1;
            pstCheckFrameRate->u64InterValTimeMIN[1] = -1;
            pstCheckFrameRate->u64InterValTimeMIN[0] = -1;
            pstCheckFrameRate->u64CurrentInputFrameTime = 0;
            pstCheckFrameRate->u64LastInputFrameTime = 0;
            pstCheckFrameRate->bFirstSet = TRUE;
            handle.OnPrintOut(handle,"LayerID %d PortID %d bstat %d \r\n", u8LayerID, u8PortID, pstCheckFrameRate->bstat);
            if(gstDispCheckFrameRateTimer.bCreate == FALSE)
            {
                init_timer(&gstDispCheckFrameRateTimer.Timer);
                gstDispCheckFrameRateTimer.Timer.function =  _mi_disp_CheckFrameRateTimer;
                add_timer(&gstDispCheckFrameRateTimer.Timer);
                mod_timer(&gstDispCheckFrameRateTimer.Timer,jiffies+HZ);//set 1s timeout
                gstDispCheckFrameRateTimer.bCreate = TRUE;
            }
            gu8StatFrameRateChnCnt++;
        }
        else if (strcmp(argv[3], "OFF") == 0 && pstCheckFrameRate->bstat == TRUE)
        {
            pstCheckFrameRate->bstat = FALSE;
            astDevStatus[u32DevId].stDispCheckIsr.bCheckIsr = FALSE;
            gu8StatFrameRateChnCnt--;
            handle.OnPrintOut(handle,"LayerID %d PortID %d bstat %d \r\n", u8LayerID, u8PortID, pstCheckFrameRate->bstat);
            if(gu8StatFrameRateChnCnt == 0)
            {
                del_timer(&gstDispCheckFrameRateTimer.Timer);
                gstDispCheckFrameRateTimer.bCreate = FALSE;
            }
        }
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        goto EXIT;
    }
    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 mi_disp_DebugCheckFramePts(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8LayerID = 0;
    MI_U8 u8PortID = 0;
    stDispCheckFramePts_t *pstCheckFramePts;
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if(argc > 3)
    {
        u8LayerID = simple_strtoul(argv[1],NULL,10);
        u8PortID = simple_strtoul(argv[2],NULL,10);
        MI_DISP_CHECK_INVAILDLAYER(u8LayerID);
        MI_DISP_CHECK_INVAILDINPUTPORT(u8PortID);

        pstCheckFramePts = &astLayerParams[u8LayerID].astPortStatus[u8PortID].stDispCheckFramePts;
        if(astLayerParams[u8LayerID].astPortStatus[u8PortID].bEnable != TRUE)
        {
            goto EXIT;
        }
        if(strcmp(argv[3], "ON") == 0 && pstCheckFramePts->bCheckFramePts == FALSE)
        {
            pstCheckFramePts->u32CheckFramePtsBufCnt = 0;
            pstCheckFramePts->bCheckFramePts = TRUE;
            handle.OnPrintOut(handle,"LayerID %d PortID %d bCheckFramePts %d \r\n",u8LayerID,u8PortID,pstCheckFramePts->bCheckFramePts);
        }
        else if (strcmp(argv[3], "OFF") == 0 && pstCheckFramePts->bCheckFramePts == TRUE)
        {
            pstCheckFramePts->bCheckFramePts = FALSE;
            handle.OnPrintOut(handle,"LayerID %d PortID %d bCheckFramePts %d \r\n",u8LayerID,u8PortID,pstCheckFramePts->bCheckFramePts);
        }
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        goto EXIT;
    }
    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 mi_disp_DebugDumpFrame(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8LayerId;
    MI_U8 u8PortId;
    
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    if(argc > 2)
    {
        u8LayerId = simple_strtoul(argv[1],NULL,10);
        u8PortId = simple_strtoul(argv[2],NULL,10);
        MI_DISP_CHECK_INVAILDLAYER(u8LayerId);
        MI_DISP_CHECK_INVAILDINPUTPORT(u8PortId);

        astLayerParams[u8LayerId].astPortStatus[u8PortId].bDumpFrame = TRUE;
        handle.OnPrintOut(handle,"dumpframe layerid:%d portid:%d\n",u8LayerId,u8PortId);
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        goto EXIT;
    }
    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 mi_disp_DebugStopGetInBuff(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U8 u8LayerId;
    MI_U8 u8PortId;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);
    if(argc > 3)
    {
        u8LayerId = simple_strtoul(argv[1],NULL,10);
        u8PortId = simple_strtoul(argv[2],NULL,10);
        MI_DISP_CHECK_INVAILDLAYER(u8LayerId);
        MI_DISP_CHECK_INVAILDINPUTPORT(u8PortId);
        if(strcmp(argv[3], "ON") == 0 && astLayerParams[u8LayerId].astPortStatus[u8PortId].bStopGetInBuf == FALSE)
        {
            handle.OnPrintOut(handle,"layerid:%d portid:%d bStopGetInBuf:%d\n",u8LayerId, u8PortId, astLayerParams[u8LayerId].astPortStatus[u8PortId].bStopGetInBuf);
            astLayerParams[u8LayerId].astPortStatus[u8PortId].bStopGetInBuf = TRUE;
        }
        if(strcmp(argv[3], "OFF") == 0 && astLayerParams[u8LayerId].astPortStatus[u8PortId].bStopGetInBuf == TRUE)
        {
            handle.OnPrintOut(handle,"layerid:%d portid:%d bStopGetInBuf:%d\n",u8LayerId, u8PortId, astLayerParams[u8LayerId].astPortStatus[u8PortId].bStopGetInBuf);
            astLayerParams[u8LayerId].astPortStatus[u8PortId].bStopGetInBuf = FALSE;
        }
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command count: %d.\n", argc);
        goto EXIT;
    }
    s32Ret = MI_SUCCESS;
EXIT:
    return s32Ret;
}

#endif

MI_S32 MI_DISP_IMPL_Init(void)
{
    MI_S32 s32Err = 0;
    MI_U32 u32DevID = 0;
    MI_U32 u32LayerID = 0;
    mi_sys_ModuleDevBindOps_t stDISPOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    MHAL_DISP_AllocPhyMem_t stHalAlloc;
    MI_SYS_DRV_HANDLE hDevSysHandle;
    MI_S32 s32IrqRet = 0;

#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t stModuleProcfsOps;
#endif
    if(_bDispInit)
    {
        DISP_DBG_INFO("MI_DISPLAY already Inited, return ok !\n");
        return MI_SUCCESS;
    }
    //Init Config
    mi_disp_ConfigInit();

    pstDevice0Param = &astDevStatus[0];
    pstDevice1Param = &astDevStatus[1];

    stHalAlloc.free = mi_sys_MMA_Free;
    stHalAlloc.alloc = mi_sys_MMA_Alloc;

    for(u32DevID = 0; u32DevID < MI_DISP_DEV_MAX; u32DevID++)
    {
        memset(&astDevStatus[u32DevID], 0, sizeof(mi_disp_DevStatus_t));
        stDISPOps.OnBindInputPort = mi_disp_OnBindInputPort;
        stDISPOps.OnUnBindInputPort = mi_disp_OnUnBindInputPort;
        stDISPOps.OnBindOutputPort = NULL; //DISPOnBindChnOutputCallback;
        stDISPOps.OnUnBindOutputPort = NULL; //DISPOnUnBindChnOutputCallback;
        stDISPOps.OnOutputPortBufRelease = NULL; //DISPOnUnBindChnOutputCallback;

        memset(&stModInfo, 0x0, sizeof(stModInfo));
        stModInfo.eModuleId = E_MI_MODULE_ID_DISP;
        stModInfo.u32DevId = u32DevID;
        stModInfo.u32DevChnNum = ((u32DevID == 0) ? 2 : 1);
        stModInfo.u32InputPortNum = MI_DISP_INPUTPORT_MAX;
        stModInfo.u32OutputPortNum = 0;
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
        memset(&stModuleProcfsOps, 0 , sizeof(stModuleProcfsOps));
        stModuleProcfsOps.OnDumpDevAttr = mi_disp_OnDumpDevAttr;
        stModuleProcfsOps.OnDumpChannelAttr = mi_disp_OnDumpChannelAttr;
        stModuleProcfsOps.OnDumpInputPortAttr = mi_disp_OnDumpInputPortAttr;
        stModuleProcfsOps.OnDumpOutPortAttr = mi_disp_OnDumpOutPortAttr;
        stModuleProcfsOps.OnHelp = mi_disp_OnHelp;
#else
        stModuleProcfsOps.OnDumpDevAttr = NULL;
        stModuleProcfsOps.OnDumpChannelAttr = NULL;
        stModuleProcfsOps.OnDumpInputPortAttr = NULL;
        stModuleProcfsOps.OnDumpOutPortAttr = NULL;
        stModuleProcfsOps.OnHelp = NULL;
#endif
        hDevSysHandle = mi_sys_RegisterDev(&stModInfo, &stDISPOps, &astDevStatus[u32DevID]
                                          #ifdef MI_SYS_PROC_FS_DEBUG
                                              , &stModuleProcfsOps
                                              ,MI_COMMON_GetSelfDir
                                          #endif
                                          );
        astDevStatus[u32DevID].hDevSysHandle = hDevSysHandle;
        astDevStatus[u32DevID].u32DevId = u32DevID;
        DISP_DBG_INFO("astDevStatus[%d](%p) -> hDevSysHandle: %p !!!\n", u32DevID, &astDevStatus[u32DevID], astDevStatus[u32DevID].hDevSysHandle);

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
        mi_sys_RegistCommand("setIrqStatus",   1, mi_disp_DebugSetIrqStatus, hDevSysHandle);
        mi_sys_RegistCommand("setsyncmode",    3, mi_disp_DebugSetSyncMode, hDevSysHandle);
        mi_sys_RegistCommand("settoleration",  3, mi_disp_DebugSetToleration, hDevSysHandle);
        mi_sys_RegistCommand("getcapframe",    3, mi_disp_DebugGetScreenFrame, hDevSysHandle);
        mi_sys_RegistCommand("setlayer",       7, mi_disp_DebugSetLayer, hDevSysHandle);
        mi_sys_RegistCommand("setlayerstatus", 2, mi_disp_DebugSetLayerStatus, hDevSysHandle);
        mi_sys_RegistCommand("setcsc",         6, mi_disp_DebugSetCsc, hDevSysHandle);
        mi_sys_RegistCommand("setbgcolor",     2, mi_disp_DebugSetBgColor, hDevSysHandle);
        mi_sys_RegistCommand("checkframepts",  3, mi_disp_DebugCheckFramePts, hDevSysHandle);
        mi_sys_RegistCommand("checkframerate",  3, mi_disp_DebugCheckFrameRate, hDevSysHandle);
        mi_sys_RegistCommand("dumpframe",      2, mi_disp_DebugDumpFrame, hDevSysHandle);
        mi_sys_RegistCommand("stopgetbuff",    3, mi_disp_DebugStopGetInBuff, hDevSysHandle);
#endif

        if(mi_disp_InitDevStatus(&astDevStatus[u32DevID], u32DevID) != MI_SUCCESS)
        {
            DISP_DBG_ERR("mi_disp_InitDevStatus Fail!!!\n");
            return E_MI_ERR_FAILED;
        }
        if(MHAL_DISP_DeviceCreateInstance(&stHalAlloc, u32DevID, &(astDevStatus[u32DevID].pstDevObj)) != TRUE)
        {
            DISP_DBG_ERR("Dev%d Create Instance fail!!!\n", u32DevID);
            continue;
        }
        astDevStatus[u32DevID].pstWorkTask = kthread_create(Disp_Dev_Work_Thread, &astDevStatus[u32DevID], "DispDevWorkThread");
        if(IS_ERR(astDevStatus[u32DevID].pstWorkTask))
        {
            DISP_DBG_ERR("Unable to start kernel thread.\n");
            s32Err = PTR_ERR(astDevStatus[u32DevID].pstWorkTask);
            astDevStatus[u32DevID].pstWorkTask  = NULL;
            return s32Err;
        }
        astDevStatus[u32DevID].pstIsrTask = kthread_create(Disp_Dev_ISR_Thread, &astDevStatus[u32DevID], "DispDevIsrThread");
        if(IS_ERR(astDevStatus[u32DevID].pstIsrTask))
        {
            DISP_DBG_ERR("Unable to start kernel thread.\n");
            s32Err = PTR_ERR(astDevStatus[u32DevID].pstIsrTask);
            astDevStatus[u32DevID].pstIsrTask = NULL;
            return s32Err;
        }
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
        if(astDevStatus[u32DevID].bProcDisableIrq == TRUE)
        {
            astDevStatus[u32DevID].bSupportIrq = FALSE;
        }
        else
#endif
        if(MHAL_DISP_GetDevIrq(astDevStatus[u32DevID].pstDevObj, &astDevStatus[u32DevID].u32DevIrq) == TRUE)
        {
            astDevStatus[u32DevID].bSupportIrq = TRUE;
            printk("[%s %d] Irq = %d \n", __FUNCTION__, __LINE__, astDevStatus[u32DevID].u32DevIrq);
        }
        else
        {
            DISP_DBG_ERR("DISP GET IQR FAIL\n");
            astDevStatus[u32DevID].bSupportIrq = FALSE;
        }
        if(astDevStatus[u32DevID].bSupportIrq == TRUE)
        {
            s32IrqRet = request_irq(astDevStatus[u32DevID].u32DevIrq, MI_DISP_DevISR, IRQF_SHARED | IRQF_ONESHOT, "mi_disp_isr", &astDevStatus[u32DevID]);
            if(0 != s32IrqRet)
            {
                DISP_DBG_ERR("request_irq failed. u32DevIrq = %u, s32IrqRet = %d.\n\n ", astDevStatus[u32DevID].u32DevIrq, s32IrqRet);
            }
        }

        wake_up_process(astDevStatus[u32DevID].pstWorkTask);
        wake_up_process(astDevStatus[u32DevID].pstIsrTask);
    }

    for(u32LayerID =0 ; u32LayerID < MI_DISP_LAYER_MAX; u32LayerID++)
    {

        if(mi_disp_InitLayerStatus(&(astLayerParams[u32LayerID]), u32LayerID) != MI_SUCCESS)
        {
            DISP_DBG_ERR("mi_disp_InitLayerStatus Fail!!!\n");
            return E_MI_ERR_FAILED;
        }

        if(FALSE == astLayerParams[u32LayerID].bLayerCreated)
        {
            if(MHAL_DISP_VideoLayerCreateInstance(&stHalAlloc, u32LayerID, &(astLayerParams[u32LayerID].apLayerObjs)) != TRUE)
            {
                DISP_DBG_ERR("Layer%d Create Instance fail!!!\n",u32LayerID);
                continue;
            }
            astLayerParams[u32LayerID].bLayerCreated =TRUE;
        }
        else
        {
            DISP_DBG_INFO("Layer%d alreday Created\n", u32LayerID);
        }

    }

    _bDispInit = TRUE;

    return MI_SUCCESS;
}


MI_S32 MI_DISP_IMPL_DeInit(void)
{
    MI_U32 u32DevID = 0;
    MI_U32 u32PortId = 0;
    MI_U32 u32LayerId = 0;

    if(_bDispInit == FALSE)
    {
        DISP_DBG_INFO("MI_DISPLAY already deinit, return ok !\n");
        return MI_SUCCESS;
    }

    for(u32LayerId = 0; u32LayerId < MI_DISP_LAYER_MAX; u32LayerId ++)
    {
        for(u32PortId=0; u32PortId <MI_DISP_INPUTPORT_MAX; u32PortId++)
        {
            if(astLayerParams[u32LayerId].astPortStatus[u32PortId].bEnable)
            {
                MI_DISP_IMPL_DisableInputPort(u32LayerId, u32PortId);
            }
        }

        if(astLayerParams[u32LayerId].bLayerEnabled)
        {
            MI_DISP_IMPL_DisableVideoLayer(u32LayerId);
        }

        if(astLayerParams[u32LayerId].u8BindedDevID < MI_DISP_LAYER_MAX)
        {
            MI_DISP_IMPL_UnBindVideoLayer(u32LayerId, astLayerParams[u32LayerId].u8BindedDevID);
        }
        if(astLayerParams[u32LayerId].apLayerObjs != NULL)
        {
            MHAL_DISP_VideoLayerDestoryInstance(astLayerParams[u32LayerId].apLayerObjs);
        }
        memset(&astLayerParams[u32LayerId], 0, sizeof(mi_disp_LayerStatus_t));
    }

    for(u32DevID = 0; u32DevID < MI_DISP_DEV_MAX; u32DevID++)
    {
        if( astDevStatus[u32DevID].bDISPEnabled)
        {
            MI_DISP_IMPL_Disable(u32DevID);
        }
        if(astDevStatus[u32DevID].bSupportIrq == TRUE)
        {
            free_irq(astDevStatus[u32DevID].u32DevIrq, &astDevStatus[u32DevID]);
        }
        if(astDevStatus[u32DevID].pstIsrTask != NULL)
        {
            kthread_stop(astDevStatus[u32DevID].pstIsrTask);
        }
        if(astDevStatus[u32DevID].pstWorkTask != NULL)
        {
            kthread_stop(astDevStatus[u32DevID].pstWorkTask);
        }

        if(astDevStatus[u32DevID].pstDevObj != NULL)
        {
            MHAL_DISP_DeviceDestroyInstance(astDevStatus[u32DevID].pstDevObj);
        }
        mi_disp_InitDevStatus(&astDevStatus[u32DevID], u32DevID);
        mi_sys_UnRegisterDev(astDevStatus[u32DevID].hDevSysHandle);
        astDevStatus[u32DevID].hDevSysHandle = 0;
        memset(&astDevStatus[u32DevID], 0, sizeof(mi_disp_DevStatus_t));
    }

    pstDevice0Param = NULL;
    pstDevice1Param = NULL;

    _bDispInit = FALSE;

    return MI_SUCCESS;
}
