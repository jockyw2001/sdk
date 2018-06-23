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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    apiVDEC_EX.c
/// @brief  VDEC EXTENSION API FOR DUAL STREAMS
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#else
#include <string.h>
#endif

#include "MsCommon.h"
#include "MsVersion.h"
#include "drvMMIO.h"
#include "MsOS.h"
#include "utopia.h"

#include "apiVDEC_EX.h"
#include "apiVDEC_EX_v2.h"
#include "drvMVOP.h"
#include "apiDMS.h"
#include "apiDMS_Type.h"
#include "apiDMS_v2.h"
#include "apiVsync.h"

#ifdef VSYNC_USE_DMS

#define VSYNC_HDR_TEST 1
#define VSYNC_HVD_HDR_TEST 1
#define VSYNC_DROP_DBG 0

extern VDEC_EX_Result _MApi_VDEC_EX_V2_GetDispInfo(VDEC_StreamId *pStreamId, VDEC_EX_DispInfo *pDispinfo);
extern VDEC_EX_Result _MApi_VDEC_EX_V2_CheckDispInfoRdy(VDEC_StreamId *pStreamId);

//=======================LOCAL PTACH===========================
typedef enum
{
    MS_DISP_FRM_INFO_EXT_TYPE_10BIT,
    MS_DISP_FRM_INFO_EXT_TYPE_INTERLACE = 1, // interlace bottom 8bit will share the same enum value
    MS_DISP_FRM_INFO_EXT_TYPE_DOLBY_EL = 1,  // with dolby enhance layer 8bit
    MS_DISP_FRM_INFO_EXT_TYPE_10BIT_INTERLACE = 2, // interlace bottom 2bit will share the same enum
    MS_DISP_FRM_INFO_EXT_TYPE_10BIT_DOLBY_EL = 2,  // value with dolby enhance layer 2bit
    MS_DISP_FRM_INFO_EXT_TYPE_10BIT_MVC,
    MS_DISP_FRM_INFO_EXT_TYPE_DOLBY_MODE = 4,
    MS_DISP_FRM_INFO_EXT_TYPE_INTERLACE_MVC = 4,
    MS_DISP_FRM_INFO_EXT_TYPE_10BIT_INTERLACE_MVC = 5, // MVC interlace R-View 2bit will share the
    MS_DISP_FRM_INFO_EXT_TYPE_DOLBY_META = 5,          // same enum with dolby meta data
    MS_DISP_FRM_INFO_EXT_TYPE_MFCBITLEN,
    MS_DISP_FRM_INFO_EXT_TYPE_MFCBITLEN_MVC,
    MS_DISP_FRM_INFO_EXT_TYPE_MAX,
} DISP_FRM_INFO_EXT_TYPE;


typedef enum
{
    MS_FRC_NORMAL = 0,
    MS_FRC_32PULLDOWN,               //3:2 pulldown mode (ex. 24p a 60i or 60p)
    MS_FRC_PAL2NTSC ,                //PALaNTSC conversion (50i a 60i)
    MS_FRC_NTSC2PAL,                 //NTSCaPAL conversion (60i a 50i)
    MS_FRC_DISP_2X,                  //output rate is twice of input rate (ex. 30p a 60p)
    MS_FRC_24_50,                    //output rate 24P->50P 48I->50I
    MS_FRC_50P_60P,                  //output rate 50P ->60P
    MS_FRC_60P_50P,                  //output rate 60P ->50P
    MS_FRC_HALF_I,                   //output rate 120i -> 60i, 100i -> 50i
    MS_FRC_120I_50I,                 //output rate 120i -> 60i
    MS_FRC_100I_60I,                 //output rate 100i -> 60i
} MS_FRCMode;

// This should be deinfed by DMS header file
typedef enum
{
    E_MSVDEC_DOLBY_VISION_DISABLE,
    E_MSVDEC_DOLBY_VISION_SINGLE_LAYER,
    E_MSVDEC_DOLBY_VISION_DUAL_LAYER,
} MSVDEC_DolbyVisionMode;
//-------------------------------------------------------------------------------------------------
//  Local Compiler Options
//-------------------------------------------------------------------------------------------------


#if (!defined(MSOS_TYPE_NUTTX) && !defined(MSOS_TYPE_OPTEE)) || defined(SUPPORT_X_MODEL_FEATURE)

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define VSYNC_STACK_SIZE                    2048
#define VSYNC_TASK_LEN                        20
#define VSYNC_DEFAULT_LOOP_TIME     30
#define VSYNC_SAMPLE_RATE                  1
#define VSYNC_DISP_FB_Q_SZ                   8
#define MS_90KHZ                                   90
#define VSYNC_DEFAULT_FRAMERATE     30000
#define VSYNC_ONE_SEC                          1000000
#define MIN_SLEEP_TIME                          4
#define TIME_10_MS                              10
#define VSYNC_DROP_THRESHOLD         1

// =============  config defines  ===============
#define USE_US_MODE_FOR_VDEC             1
#define CORRECT_VC1_INTERLACE_MODE  1
#define VSYNCBRIDGE_EXT_HEADER          1

#define SUPPORT_HEVC_HDR_V2 defined(VDEC_CAP_HEVC_HDR_V2)
#define SUPPORT_HEVC_HDR defined(VDEC_CAP_HEVC_HDR)
#define SUPPORT_HEVC_HDR_V3 defined(VDEC_CAP_HEVC_HDR_V3)

#define SUPPORT_FRAME_INFO_EXT_V4 defined(VDEC_CAP_FRAME_INFO_EXT_V4)
#define SUPPORT_FRAME_INFO_EXT_V5 defined(VDEC_CAP_FRAME_INFO_EXT_V5)
#define SUPPORT_FRAME_INFO_EXT_V6 defined(VDEC_CAP_FRAME_INFO_EXT_V6)
#define SUPPORT_FRAME_INFO_EXT_V7 defined(VDEC_CAP_FRAME_INFO_EXT_V7)


#define SUPPORT_HEVC_INTERLACE          VSYNCBRIDGE_EXT_HEADER
#define SUPPORT_REPORT_SCAN_TYPE        defined(VDEC_CAP_REPORT_SCAN_TYPE)
#define USE_VP9_TILE_FORMAT                     1
#define SUPPORT_10BIT                                  1
#define ENABLE_TOGGLE_INFO_FOR_HWC      1
//=======================================

#define DIFF(a, b)                      (a > b ? (a-b) : (b-a))

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------

typedef enum
{
    E_VSYNC_FREERUN,
    E_VSYNC_AVSYNC,
    E_VSYNC_DROP,
    E_VSYNC_REPEAT,
    E_VSYNC_NONE,
} VSYNC_AVSYNC_STATE;

typedef enum
{
    E_VDEC_EX_DECODER_MVD = 0,
    E_VDEC_EX_DECODER_HVD,
    E_VDEC_EX_DECODER_MJPEG,
    E_VDEC_EX_DECODER_NONE,
} VDEC_EX_Decoder;
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

typedef struct
{
    union
    {
        struct
        {
            unsigned int vsync_log_dbg : 1;
            unsigned int vsync_log_status : 1;
            unsigned int vsync_log_time : 1;
            unsigned int vsync_log_cmd : 1;
            unsigned int vsync_log_drop : 1;
            unsigned int vsync_log_verbose : 1;
            unsigned int vsync_log_seq_chg : 1;
            unsigned int vsync_log_idle : 1;
            unsigned int mvdcmd_handshake_reserved : 24;    // reserved for extend
        };
        unsigned int value;
    };
} VSYNC_LOG_IDX;

#define VSYNC_LOG_DBG(x)          if (pAttr->stLogIdx.vsync_log_dbg)  { (x); }
#define VSYNC_LOG_STATUS(x)       if (pAttr->stLogIdx.vsync_log_status)  { (x); }
#define VSYNC_LOG_TIME(x)         if (pAttr->stLogIdx.vsync_log_time)  { (x); }
#define VSYNC_LOG_CMD(x)          if (pAttr->stLogIdx.vsync_log_cmd)  { (x); }
#define VSYNC_LOG_DROP(x)         if (pAttr->stLogIdx.vsync_log_drop)  { (x); }
#define VSYNC_LOG_VERBOSE(x)      if (pAttr->stLogIdx.vsync_log_verbose)  { (x); }
#define VSYNC_LOG_SEQCHG(x)       if (pAttr->stLogIdx.vsync_log_seq_chg)  { (x); }
#define VSYNC_LOG_IDLE(x)         if (pAttr->stLogIdx.vsync_log_idle)  { (x); }

typedef VSYNC_Result (*Vsync_Proc)(int);


typedef struct
{
    MS_BOOL             bUsed;
    MS_BOOL             bInit;
    int                 VsynDispId;
    MS_U32              eWinID;
    MS_U32              u32OverlayId;
    VDEC_StreamId       VdecStreamId;
    VDEC_EX_CodecType   CodecType;
    VDEC_EX_SrcMode     SrcMode;
    VSYNC_Vdec_Status   eStatus;
    VDEC_EX_DispInfo    Dispinfo;
    VDEC_EX_DispInfo    DispinfoOutput;

    MS_S32              s32VSyncTaskId;
    char                pu8VsyncTask[VSYNC_TASK_LEN];
    Vsync_Proc          pfVsync_Proc;
    MS_U8               u8VsyncStack[VSYNC_STACK_SIZE];
    MS_BOOL             bAvsync;
    MS_BOOL             bAvsyncDone;
    MS_BOOL             bFirstFrame;
    MS_BOOL             bStepDispDone;
    MS_BOOL             bLastFrameShowDone;
    MS_BOOL             bCheckFileEnd;
    MS_BOOL             bDispInfoRdy;
    MS_U8               u8EnableSetDigitalSignal;
    MS_U8               u8EnableClearDigitalSignal;
    // =====================avsync param
    VSYNC_SyncMode eSyncMode;
    MS_BOOL             bShowFirstFrameDirect;
    MS_U32              u32SyncDelay;                  //hw delay that should tuned by audio                                                    unit: 90khz
    MS_U32              u32Sync_tolerance;         //user defined syn_threshold                                                                 unit: 90khz
    MS_U32              u32Syn_threshold;           // 1 vsync duration for stb, 2 vsync duration for tv                                     unit: 90khz
    MS_U32              u32Sync_far_threshold;  //freerun threshold, 5 sec for TS_FILE_MODE & SLQ_TBL_MODE, 1 sec else     unit: 90khz
    MS_U32              u32VsycnDuration;          // decided by frame rate, used as sleep time in _VSYNC_Proc loop                  unit: ms
    MS_U32              u32RepeatThreshold;
    MS_U32              u32DropThresholdCnt;
    MS_U32              u8InterlaceModeDropCnt;
    //====================== slow sync mode
    MS_U8               u8SlowRepeat;                 // play 1 repeat u8SlowRepeatCnt frame (slow sync)
    MS_U8               u8SlowDrop;                    // play 1 drop u8SlowDropCnt frame (slow sync)
    MS_U8               u8SlowRepeatCnt;
    MS_U8               u8SlowDropCnt;
    MS_U32              u32LastFlipStc;               //unit: 90khz
    //====================== speed & trick mode
    VDEC_EX_STCMode     eStcMode;
    VDEC_EX_SpeedType   eSpeedType;
    VDEC_EX_DispSpeed   eSpeed;
    MS_U32              u32latch_time;
    MS_U32              u32latch_stc;
    MS_U32              u32latch_pts;
    MS_BOOL             bSmoothTrickMode;
    MS_U8               u8TraditionalDispCnt;
    //======================= info
    MS_U32              u32DropCnt;
    MS_U32              u32DispCnt;
    MS_U32              u32RepeatCnt;
    MS_U64              u64Pts;
    MS_U64              u64NextPts;
    MS_S64              s64PtsStcDelta;
    MS_U8               u8FirstFrameRdy;
    //======================= callback
    VsyncDispConnectCb      VsyncDispConnect;
    VsyncDispDisconnectCb   VsyncDispDisconnect;
    VsyncDispFlipCb         VsyncDispFlip;
    VDEC_EX_FireUserCallbackFunc VsyncFireVdecCallback;
    //======================= set control
    MS_U8                       u83DMode;
    MS_U8                       u8MsVdec3DLayoutUnlocked;
    MS_U8                       u8LowLatencyMode;
    MS_U8                       u8FieldCtrl;          // control one field mode, always top or bot when FF or FR , 0-> Normal, 1->always top, 2->always bot
    MS_U8                       u8ApplicationType;
    MS_U8                       u8MvopMCUMode;
    //=======================
    MS_PHY                     phyVsyncBridgeShmAddr;
    MS_PHY                     phyVsyncBridgeExtAddr;
    //=======================
    VDEC_EX_DispFrame   DispFrameQ[VSYNC_DISP_FB_Q_SZ];
    MS_U8                        u8DispFbQWptr;
    MS_U8                        u8DispFbQRptr;
    MS_U8                        u8ByPassMode;
    VSYNC_LOG_IDX                stLogIdx;
    MS_U8                        u8IsSeqChange;
    MS_BOOL                      bSelf_SeqChange;
    MS_BOOL                      bEnableDynScale;
    MS_BOOL                      bSuspendDS;
    MS_BOOL                      bSuspendDS_Ftime;
    //========================= debug
    MS_U32                        u32GetNextFrameTime;
    MS_U32                        u32DbgGetFrameCnt;
    MS_U32                        u32DbgDispCnt;
} VSYNC_Attr;

typedef struct
{
    MS_BOOL bDmsInit;
    VSYNC_Attr _Attr[VSYNC_MAX_DISP_PATH];
} VSYNC_CTX;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
VSYNC_CTX* pVsyncContext = NULL;
VSYNC_CTX gVsyncContext;
MS_S32 s32VSYNCMutexID = -1;

//------------------------------------------------------------------------------
// Local Functions Prototype
//------------------------------------------------------------------------------

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_ECOS)

#define  _VSYNC_API_MutexLock()\
    {\
        MsOS_ObtainMutex(s32VSYNCMutexID, MSOS_WAIT_FOREVER);\
    }

#define _VSYNC_API_MutexUnlock()\
    {\
        MsOS_ReleaseMutex(s32VSYNCMutexID);\
    }

#define STREAM_IDX (MS_U8)(pStreamId->u32Id >> 24)

static MS_BOOL _VSYNC_API_MutexCreate(void)
{

#if defined(MSOS_TYPE_ECOS) // avoid creating the mutex name again for ecos project

    if (s32VSYNCMutexID > 0) // created already
    {
        return TRUE;
    }

#endif

    s32VSYNCMutexID = MsOS_CreateMutex(E_MSOS_FIFO, "VSYNC_API_Mutex", MSOS_PROCESS_SHARED);

    if (s32VSYNCMutexID < 0)
    {
        return FALSE;
    }
    return TRUE;
}

#else
#define  _VSYNC_API_MutexLock()
#define _VSYNC_API_MutexUnlock()
#endif

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------
int loopcnt = 0;// for dbg

//-------------------------------------------------------------------------------------------------
//  Static Local Functions
//-------------------------------------------------------------------------------------------------

#define _VSYNC_SHM_POINTER_CHECK(x)   \
    do\
    {\
        if (pVsyncContext == NULL)\
        {\
            return x;\
        }\
    }while(0)

#define _VSYNC_SHM_INIT_CHECK(pAttr)   \
    do\
    {\
        if (pAttr->bInit != TRUE)\
        {\
            return E_VSYNC_RET_NOT_INIT;\
        }\
    }while(0)

static MS_BOOL _VSYNC_IsInterlaceFieldMode(int VsynDispId);
static MS_U32 _VSYNC_CovertTo90Khz(int VsynDispId)
{
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    MS_U32 u32TimeUnit = 1;

    if((pAttr->CodecType == E_VDEC_EX_CODEC_TYPE_RV8) || (pAttr->CodecType == E_VDEC_EX_CODEC_TYPE_RV9) || (pAttr->CodecType == E_VDEC_EX_CODEC_TYPE_VP8))
    {
        u32TimeUnit = MS_90KHZ;
    }

    return u32TimeUnit;
}

static MS_BOOL _VSYNC_IsSeqChange(int VsynDispId, VDEC_EX_DispInfo oldDispInfo, VDEC_EX_DispInfo newDispInfo)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    if(pAttr->bSelf_SeqChange == FALSE)
    {
        if(pAttr->bEnableDynScale == FALSE)
        {
            if(oldDispInfo.u16HorSize != newDispInfo.u16HorSize ||
                    oldDispInfo.u16VerSize != newDispInfo.u16VerSize ||
                    oldDispInfo.u32FrameRate != newDispInfo.u32FrameRate ||
                    oldDispInfo.u8Interlace != newDispInfo.u8Interlace ||
                    oldDispInfo.u8AspectRate != newDispInfo.u8AspectRate
              )
            {
                VSYNC_LOG_SEQCHG(VSYNC_PRINT("[%s] %d  (w:%d h:%d fps:%d i:%d) -> (w:%d h:%d fps:%d i:%d)  \n", __FUNCTION__, __LINE__, (int)oldDispInfo.u16HorSize, (int)oldDispInfo.u16VerSize, (int)oldDispInfo.u32FrameRate, (int)oldDispInfo.u8Interlace, (int)newDispInfo.u16HorSize, (int)newDispInfo.u16VerSize, (int)newDispInfo.u32FrameRate, (int)newDispInfo.u8Interlace));
                return TRUE;
            }
        }
        else
        {
            if(oldDispInfo.u32FrameRate != newDispInfo.u32FrameRate ||
                    oldDispInfo.u8Interlace != newDispInfo.u8Interlace
              )
            {
                VSYNC_LOG_SEQCHG(VSYNC_PRINT("[%s] %d  (w:%d h:%d fps:%d i:%d) -> (w:%d h:%d fps:%d i:%d)  \n", __FUNCTION__, __LINE__, (int)oldDispInfo.u16HorSize, (int)oldDispInfo.u16VerSize, (int)oldDispInfo.u32FrameRate, (int)oldDispInfo.u8Interlace, (int)newDispInfo.u16HorSize, (int)newDispInfo.u16VerSize, (int)newDispInfo.u32FrameRate, (int)newDispInfo.u8Interlace));
                return TRUE;
            }
        }
    }
    else
    {
        if((oldDispInfo.u16HorSize   != newDispInfo.u16HorSize) ||
                (oldDispInfo.u16VerSize   != newDispInfo.u16VerSize) ||
                (oldDispInfo.u32FrameRate != newDispInfo.u32FrameRate) ||
                (oldDispInfo.u8Interlace  != newDispInfo.u8Interlace)  ||
                (oldDispInfo.u8AspectRate != newDispInfo.u8AspectRate)
          )
        {
            VSYNC_LOG_SEQCHG(VSYNC_PRINT("[%s] %d  (w:%d h:%d fps:%d i:%d) -> (w:%d h:%d fps:%d i:%d)  \n", __FUNCTION__, __LINE__, (int)oldDispInfo.u16HorSize, (int)oldDispInfo.u16VerSize, (int)oldDispInfo.u32FrameRate, (int)oldDispInfo.u8Interlace, (int)newDispInfo.u16HorSize, (int)newDispInfo.u16VerSize, (int)newDispInfo.u32FrameRate, (int)newDispInfo.u8Interlace));
            return TRUE;
        }
    }
    return FALSE;
}

static VSYNC_Result _VSYNC_HandleSequenceChange(int VsynDispId)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);
    VDEC_StreamId* pStreamId = &( pVsyncContext->_Attr[VsynDispId].VdecStreamId);

    if(E_VDEC_EX_OK != _MApi_VDEC_EX_V2_GetDispInfo(pStreamId, &(pAttr->Dispinfo)))
    {
        return E_VSYNC_FAIL;
    }

    MS_BOOL bResChange = _VSYNC_IsSeqChange(VsynDispId, pAttr->DispinfoOutput, pAttr->Dispinfo);

    if(bResChange == TRUE)
    {
        VSYNC_LOG_SEQCHG(VSYNC_PRINT("[%s] %d  bSuspendDS : 0x%x  bSuspendDS_Ftime: 0x%x  bSelf_SeqChange = 0x%x \n", __FUNCTION__, __LINE__, pAttr->bSuspendDS, pAttr->bSuspendDS_Ftime, pAttr->bSelf_SeqChange));
    }

    if(pAttr->bSuspendDS == TRUE)
    {
        if(pAttr->bSuspendDS_Ftime == TRUE)
        {
            pAttr->bSuspendDS_Ftime = FALSE;
            if(pAttr->bSelf_SeqChange == TRUE)
                pAttr->u8IsSeqChange = bResChange;
            else
                pAttr->u8IsSeqChange = TRUE;
        }
        else
        {
            pAttr->u8IsSeqChange = bResChange;
        }
    }
    else// normal case
    {
        if(pAttr->bSuspendDS_Ftime == FALSE)
        {
            pAttr->bSuspendDS_Ftime = TRUE;
            pAttr->u8IsSeqChange = TRUE;
        }
        else
        {
            pAttr->u8IsSeqChange = bResChange;
        }
    }

    if(pAttr->DispinfoOutput.u32FrameRate != pAttr->Dispinfo.u32FrameRate)
    {
        //check frame rate
        if (pAttr->Dispinfo.u32FrameRate == 0)
        {
            pAttr->Dispinfo.u32FrameRate = VSYNC_DEFAULT_FRAMERATE;
        }

        if(pAttr->Dispinfo.u32FrameRate > 0 && pAttr->Dispinfo.u32FrameRate <= VSYNC_ONE_SEC)
        {
            pAttr->u32VsycnDuration = VSYNC_ONE_SEC / (pAttr->Dispinfo.u32FrameRate);
        }

        if(pAttr->u32Sync_tolerance == 0 && pAttr->u32VsycnDuration > 0)
        {
            pAttr->u32Syn_threshold = pAttr->u32VsycnDuration * MS_90KHZ;
        }

        if (MApi_VDEC_EX_Is32PullDown(pStreamId) && !pAttr->Dispinfo.u8Interlace)
        {
            pAttr->Dispinfo.u8Interlace = 1;
        }
    }

    //VSYNC_PRINT("\033[1;44m[%s] %d  pAttr->u32VsycnDuration = %d   pAttr->Dispinfo.u32FrameRate = %d\033[m\n",__FUNCTION__,__LINE__,pAttr->u32VsycnDuration,pAttr->Dispinfo.u32FrameRate);

    return E_VSYNC_OK;
}

static VDEC_EX_Decoder _VSYNC_GetDecoderByCodecType(VDEC_EX_CodecType eCodecType)
{
    VDEC_EX_Decoder eDecoder;

    switch (eCodecType)
    {
        case E_VDEC_EX_CODEC_TYPE_MPEG2:
        case E_VDEC_EX_CODEC_TYPE_H263:
        case E_VDEC_EX_CODEC_TYPE_MPEG4:
        case E_VDEC_EX_CODEC_TYPE_DIVX311:
        case E_VDEC_EX_CODEC_TYPE_DIVX412:
        case E_VDEC_EX_CODEC_TYPE_FLV:
        case E_VDEC_EX_CODEC_TYPE_VC1_ADV:
        case E_VDEC_EX_CODEC_TYPE_VC1_MAIN:
            eDecoder = E_VDEC_EX_DECODER_MVD;
            break;
        case E_VDEC_EX_CODEC_TYPE_RV8:
        case E_VDEC_EX_CODEC_TYPE_RV9:
            eDecoder = E_VDEC_EX_DECODER_HVD;
            break;
        case E_VDEC_EX_CODEC_TYPE_H264:
        case E_VDEC_EX_CODEC_TYPE_AVS:
        case E_VDEC_EX_CODEC_TYPE_MVC:  /// SUPPORT_MVC
        case E_VDEC_EX_CODEC_TYPE_VP8:
        case E_VDEC_EX_CODEC_TYPE_HEVC:
        case E_VDEC_EX_CODEC_TYPE_VP9:
        case E_VDEC_EX_CODEC_TYPE_HEVC_DV:
            eDecoder = E_VDEC_EX_DECODER_HVD;
            break;
        case E_VDEC_EX_CODEC_TYPE_MJPEG:
            eDecoder = E_VDEC_EX_DECODER_MJPEG;

            break;

        default:
            eDecoder = E_VDEC_EX_DECODER_NONE;

            break;
    }

    return eDecoder;
}


static MS_BOOL _VSYNC_IsSmoothTrickMode(int VsynDispId)
{
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    if((pAttr->eSpeedType == E_VDEC_EX_SPEED_FAST  && pAttr->eSpeed == E_VDEC_EX_DISP_SPEED_2X) || pAttr->eSpeedType == E_VDEC_EX_SPEED_SLOW) //smooth fast/slow
    {
        if( _VSYNC_GetDecoderByCodecType(pAttr->CodecType) != E_VDEC_EX_DECODER_MVD || pAttr->bSmoothTrickMode || pAttr->eSpeedType == E_VDEC_EX_SPEED_SLOW)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static MS_BOOL _VSYNC_IsTrasitionTrickMode(int VsynDispId)
{
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    if(pAttr->eSpeedType == E_VDEC_EX_SPEED_FAST  && pAttr->eSpeed == E_VDEC_EX_DISP_SPEED_2X && _VSYNC_GetDecoderByCodecType(pAttr->CodecType) == E_VDEC_EX_DECODER_MVD && pAttr->bSmoothTrickMode == FALSE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static MS_U32 _VSYNC_GetDmxStc32(int VsynDispId)
{
    // TODO: may be we need to select dmx first ?
    //DMX_FILTER_STATUS SYMBOL_WEAK MApi_DMX_Stc_Select(DMX_FILTER_TYPE eFltSrc, MS_U32 u32StcEng);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);
    MS_U32 u32Stc32, u32Stc = 0;

    if(DMX_FILTER_STATUS_OK != MApi_DMX_Stc_Eng_Get( pAttr->eStcMode , &u32Stc32, &u32Stc))
    {
        VSYNC_PRINT("\033[1;31m[%s] %d Get stc fail !!!! VsynDispId = %d \033[m\n", __FUNCTION__, __LINE__, VsynDispId);
    }
    return u32Stc;
}

static MS_BOOL _VSYNC_IsInterlaceFieldMode(int VsynDispId)
{
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    if ((pAttr->CodecType == E_VDEC_EX_CODEC_TYPE_MVC)
#if SUPPORT_HEVC_INTERLACE
            || (pAttr->CodecType == E_VDEC_EX_CODEC_TYPE_HEVC && pAttr->Dispinfo.u8Interlace)
#endif
            || (pAttr->CodecType == E_VDEC_EX_CODEC_TYPE_HEVC_DV))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static MS_U32 _VSYNC_GetStc32(int VsynDispId)
{
    MS_U32 u32Stc = 0;
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    if(_VSYNC_IsSmoothTrickMode(VsynDispId) == TRUE)
    {
        if(pAttr->eSpeedType == E_VDEC_EX_SPEED_FAST)
            u32Stc = (pAttr->u32latch_stc) + ((MsOS_GetSystemTime() - (pAttr->u32latch_time)) * pAttr->eSpeed) * MS_90KHZ;
        else
            u32Stc = (pAttr->u32latch_stc) + ((MsOS_GetSystemTime() - (pAttr->u32latch_time)) / pAttr->eSpeed) * MS_90KHZ;
    }
    else
    {
#if 0
        if(pAttr->eSyncMode == E_VSYNC_VIDEO_MASTER_MODE)
        {
            u32Stc = (pAttr->u32latch_stc) + (MsOS_GetSystemTime() - (pAttr->u32latch_time)) * MS_90KHZ;
        }
        else
#endif
        {
            u32Stc = _VSYNC_GetDmxStc32(VsynDispId);
        }
    }
    return u32Stc;
}

static void _VSYNC_DropFrame( int VsynDispId, VDEC_EX_DispFrame * pDispFrame, int u32TimeUnit, int reason)
{
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);
    VDEC_StreamId* pStreamId = &( pVsyncContext->_Attr[VsynDispId].VdecStreamId);
    int dropped_pts = (int)(pDispFrame->stFrmInfo.u32TimeStamp) * u32TimeUnit / MS_90KHZ;

    MApi_VDEC_EX_ReleaseFrame(pStreamId, pDispFrame);
    pAttr->u32DropCnt++;
    pAttr->u32DropThresholdCnt++;
    pAttr->u8InterlaceModeDropCnt++;

    VSYNC_LOG_DROP(VSYNC_PRINT("\033[1;35m[%s] %d pAttr->u32DropCnt = %d dropped_pts = %d  reason = %d\033[m\n", __FUNCTION__, __LINE__, (int)pAttr->u32DropCnt, dropped_pts, reason));
}

static void _VSYNC_SetupDispFrameFormat(int VsynDispId, ST_DMS_DISPFRAMEFORMAT* pDispFrm, VDEC_EX_DispFrame* pNextDispFrm)
{
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);
    VDEC_StreamId* pStreamId = &( pVsyncContext->_Attr[VsynDispId].VdecStreamId);
    int i, j = 0;

    if(pDispFrm != NULL && pNextDispFrm != NULL)
    {
        memset(pDispFrm, 0, sizeof(ST_DMS_DISPFRAMEFORMAT));
        pDispFrm->u32Version = VERSION_ST_DMS_DISPFRAMEFORMAT;
        pDispFrm->u32Length = sizeof(ST_DMS_DISPFRAMEFORMAT);
        pDispFrm->u32VdecStreamVersion = pStreamId->u32Version;
        pDispFrm->u32OverlayID = pAttr->u32OverlayId;
        pDispFrm->u32CodecType = pAttr->CodecType;
        pDispFrm->u32VdecStreamVersion = pStreamId->u32Version;
        pDispFrm->u32VdecStreamId = pStreamId->u32Id;


        if ((pAttr->CodecType == E_VDEC_EX_CODEC_TYPE_MVC)
#if SUPPORT_HEVC_INTERLACE
                || (pAttr->CodecType == E_VDEC_EX_CODEC_TYPE_HEVC && pAttr->Dispinfo.u8Interlace)
#endif
                || (pAttr->CodecType == E_VDEC_EX_CODEC_TYPE_HEVC_DV))
        {
            pDispFrm->u32FrameNum                              = 2;
        }
        else
        {
            pDispFrm->u32FrameNum                              = 1;
        }
        if (MApi_VDEC_EX_Is32PullDown(pStreamId))
        {
            pAttr->Dispinfo.u8FrcMode = MS_FRC_32PULLDOWN;
        }
        else
        {
            pAttr->Dispinfo.u8FrcMode = MS_FRC_NORMAL;
        }
        pDispFrm->u32FrameRate = pAttr->Dispinfo.u32FrameRate;

        MS_U32 u32TimeUnit = _VSYNC_CovertTo90Khz(VsynDispId);

        pDispFrm->u64Pts = (((MS_U64)pNextDispFrm->stFrmInfo.u32ID_H) << 32 | pNextDispFrm->stFrmInfo.u32TimeStamp) * u32TimeUnit;

#if !USE_US_MODE_FOR_VDEC
        if (pDispFrm->u64Pts != 0xFFFFFFFFFFFFFFFFULL)
        {
            pDispFrm->u64Pts *= 1000; // ms to us
        }
#endif

        pDispFrm->u8AspectRate    = pAttr->Dispinfo.u8AspectRate;
        pDispFrm->u8Interlace     = pAttr->Dispinfo.u8Interlace;
        pDispFrm->u8FrcMode       = pAttr->Dispinfo.u8FrcMode;
        pDispFrm->u83DMode        = pAttr->u83DMode;
        pDispFrm->u83DLayout      = pAttr->u8MsVdec3DLayoutUnlocked;
        pDispFrm->u32AspectWidth  = pAttr->Dispinfo.u32AspectWidth;
        pDispFrm->u32AspectHeight = pAttr->Dispinfo.u32AspectHeight;

        pDispFrm->u8ColorInXVYCC = pAttr->Dispinfo.bColorInXVYCC;
        pDispFrm->u8LowLatencyMode = pAttr->u8LowLatencyMode;
        pDispFrm->u8AFD = pAttr->Dispinfo.u8AFD;

#if VSYNCBRIDGE_EXT_HEADER
        pDispFrm->phyVsyncBridgeExtAddr = pAttr->phyVsyncBridgeExtAddr;
#endif

#if ENABLE_TOGGLE_INFO_FOR_HWC
        // Top field first [16]: 0-> bottom first, 1-> top first
        // Disp times [18-17]: 1~3
        pDispFrm->u8BottomFieldFirst = !(pNextDispFrm->stFrmInfo.u32ID_L & 0x10000);
        pDispFrm->u8ToggleTime = (pNextDispFrm->stFrmInfo.u32ID_L & 0x60000) >> 17;
#else
        pDispFrm->u8BottomFieldFirst = 0;
        pDispFrm->u8ToggleTime = 0;
#endif

#if SUPPORT_REPORT_SCAN_TYPE
        pDispFrm->u8Interlace = ((pNextDispFrm->stFrmInfo.u32ID_L >> 19) & 0x03);
#endif

#if CORRECT_VC1_INTERLACE_MODE
        if (pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_VC1_ADV || pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_VC1_MAIN)
        {
            // Picture format, [ 19-20],
            //    vc1_ProgressiveFrame = 0,   /** Picture is a progressive frame */
            //   vc1_InterlacedFrame  = 2,   /** Picture is an interlaced frame */
            //   vc1_InterlacedField  = 3,   /** Picture is two interlaced fields */

            if ((((pNextDispFrm->stFrmInfo.u32ID_L >> 19) & 0x03) != 0) && (pDispFrm->u8FrcMode == MS_FRC_32PULLDOWN))
            {
                pDispFrm->u8FrcMode = MS_FRC_NORMAL;
            }

            if ((pDispFrm->u8FrcMode == MS_FRC_32PULLDOWN) && (pDispFrm->u8ToggleTime > 1) && !pDispFrm->u8Interlace)
            {
                pDispFrm->u8Interlace = TRUE;
            }
        }
#endif

        if (pDispFrm->u8Interlace == FALSE) //Progressive Mode
        {
            pDispFrm->u8ToggleTime = 2; //For MstPlayer: 1 toggle time means 0.5 frame time
        }

        MS_U32 u32FrmIdx = 0;
        MS_U32 u32FrmCnt = pDispFrm->u32FrameNum;
        for (u32FrmIdx = 0; u32FrmIdx < u32FrmCnt; u32FrmIdx++)
        {
            if (u32FrmIdx == 1)
            {
                while(E_VDEC_EX_OK != MApi_VDEC_EX_GetNextDispFrame(pStreamId, &pNextDispFrm))
                {
                    MsOS_DelayTask(MIN_SLEEP_TIME);
                    if(pAttr->eStatus == E_VSYNC_STOP || pAttr->eStatus == E_VSYNC_FLUSH || pAttr->bUsed == FALSE)
                        break;
                }
                if((pAttr->u8DispFbQWptr + 1) % VSYNC_DISP_FB_Q_SZ == pAttr->u8DispFbQRptr)
                {
                    VSYNC_LOG_DBG(VSYNC_PRINT("QUEUE FULL !!!"));
                    MApi_VDEC_EX_ReleaseFrame(pStreamId, pNextDispFrm);
                    break;
                }
                memcpy(&(pAttr->DispFrameQ[pAttr->u8DispFbQWptr]), pNextDispFrm, sizeof(VDEC_EX_DispFrame));
                pAttr->u8DispFbQWptr = (pAttr->u8DispFbQWptr + 1) % VSYNC_DISP_FB_Q_SZ;
            }

            if (pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_MJPEG)
            {
                pDispFrm->enColorFormat = E_DMS_COLOR_FORMAT_YUYV;
            }
#if !USE_VP9_TILE_FORMAT
            else if (pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_VP9)
            {
                pDispFrm->enColorFormat = E_DMS_COLOR_FORMAT_SW_YUV420_SEMIPLANAR;
            }
#endif
            else if (pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_HEVC || pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_VP9 ||
                     pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_HEVC_DV)
            {
                pDispFrm->enColorFormat = E_DMS_COLOR_FORMAT_HW_EVD;
            }
            else
            {
                pDispFrm->enColorFormat = E_DMS_COLOR_FORMAT_HW_HVD;
            }
            pDispFrm->stFrames[u32FrmIdx].u32Version = VERSION_ST_DMS_FRAMEFORMAT;
            pDispFrm->stFrames[u32FrmIdx].u32Length = sizeof(ST_DMS_FRAMEFORMAT);
            pDispFrm->stFrames[u32FrmIdx].stHWFormat.u32Version = VERSION_ST_DMS_COLORHWFORMAT;
            pDispFrm->stFrames[u32FrmIdx].stHWFormat.u32Length = sizeof(ST_DMS_COLORHWFORMAT);

            pDispFrm->stFrames[u32FrmIdx].u32CropLeft = (MS_U32)pAttr->Dispinfo.u16CropLeft;
            pDispFrm->stFrames[u32FrmIdx].u32CropRight = (MS_U32)pAttr->Dispinfo.u16CropRight;
            pDispFrm->stFrames[u32FrmIdx].u32CropTop = (MS_U32)pAttr->Dispinfo.u16CropTop;
            pDispFrm->stFrames[u32FrmIdx].u32CropBottom = (MS_U32)pAttr->Dispinfo.u16CropBottom;

            pDispFrm->stFrames[u32FrmIdx].enFrameType = (EN_DMS_FRAMETYPE)pNextDispFrm->stFrmInfo.eFrameType;
            pDispFrm->stFrames[u32FrmIdx].enFieldType = (EN_DMS_FIELDTYPE)pNextDispFrm->stFrmInfo.eFieldType;
            pDispFrm->u8FieldCtrl = pAttr->u8FieldCtrl;

            if (u32FrmCnt == 2 && u32FrmIdx == 0)
            {
                pDispFrm->stFrames[u32FrmIdx].enViewType = E_DMS_VIEW_TYPE_LEFT;
            }
            else if (u32FrmCnt == 2 && u32FrmIdx == 1)
            {
                pDispFrm->stFrames[u32FrmIdx].enViewType =  E_DMS_VIEW_TYPE_RIGHT;
            }
            else
            {
                pDispFrm->stFrames[u32FrmIdx].enViewType =  E_DMS_VIEW_TYPE_CENTER;
            }

            pDispFrm->stFrames[u32FrmIdx].u32Width  = pNextDispFrm->stFrmInfo.u16Width;
            pDispFrm->stFrames[u32FrmIdx].u32Height = pNextDispFrm->stFrmInfo.u16Height;

            if (pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_HEVC_DV && u32FrmIdx == 1)
            {
                // E_MSVDEC_CODEC_TYPE_HEVC_DV sFrames[1] is it's 2 bit info
            }
            else
            {
                pDispFrm->stFrames[u32FrmIdx].stHWFormat.phyLumaAddr    = pNextDispFrm->stFrmInfo.u32LumaAddr;
                pDispFrm->stFrames[u32FrmIdx].stHWFormat.u32LumaPitch   = pNextDispFrm->stFrmInfo.u16Pitch;
                pDispFrm->stFrames[u32FrmIdx].stHWFormat.phyChromaAddr  = pNextDispFrm->stFrmInfo.u32ChromaAddr;
                pDispFrm->stFrames[u32FrmIdx].stHWFormat.u32ChromaPitch = pNextDispFrm->stFrmInfo.u16Pitch;
            }

#if VSYNC_HVD_HDR_TEST
#if VSYNCBRIDGE_EXT_HEADER && SUPPORT_HEVC_INTERLACE
            if (pAttr->phyVsyncBridgeExtAddr && pDispFrm->u8Interlace && u32FrmIdx == 1 &&
                    pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_HEVC)
            {
                pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_INTERLACE] = pNextDispFrm->stFrmInfo.u32LumaAddr;
                pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_INTERLACE] = pNextDispFrm->stFrmInfo.u32ChromaAddr;
            }
#endif

#if SUPPORT_HEVC_HDR_V3 || SUPPORT_FRAME_INFO_EXT_V4
            if (pAttr->phyVsyncBridgeExtAddr && u32FrmIdx == 1 && pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_HEVC_DV)
            {
                pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_DOLBY_EL] = pNextDispFrm->stFrmInfo.u32LumaAddr;
                pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_DOLBY_EL] = pNextDispFrm->stFrmInfo.u32ChromaAddr;
            }
#endif
#endif
            if (pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_VC1_ADV || pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_VC1_MAIN)
            {
                pDispFrm->stFrames[u32FrmIdx].u8LumaBitdepth  = (pNextDispFrm->stFrmInfo.u32ID_L & 0xFF00) >> 8;
                pDispFrm->stFrames[u32FrmIdx].u8ChromaBitdepth = (pNextDispFrm->stFrmInfo.u32ID_L & 0x00FF);
            }

            pDispFrm->stFrames[u32FrmIdx].u32Idx     = pNextDispFrm->u32Idx;
            pDispFrm->stFrames[u32FrmIdx].u32PriData = pNextDispFrm->u32PriData;

#if SUPPORT_10BIT
            if (pNextDispFrm->stFrmInfo.u32ID_L & 0x200000)
            {
                pDispFrm->enColorFormat = E_DMS_COLOR_FORMAT_10BIT_TILE;
                if (!pAttr->phyVsyncBridgeExtAddr)
                {
                    VDEC_EX_FrameInfoEX FrmInfo;
                    memset(&FrmInfo, 0 , sizeof(VDEC_EX_FrameInfoEX));
                    if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(pStreamId, E_VDEC_EX_USER_CMD_GET_FRAME_INFO_EX, (MS_U32 *)&FrmInfo))
                    {
                        pDispFrm->stFrames[1].stHWFormat.phyLumaAddr    = FrmInfo.u32LumaAddr_2bit;
                        pDispFrm->stFrames[1].stHWFormat.u32LumaPitch   = FrmInfo.u16Pitch_2bit;
                        pDispFrm->stFrames[1].stHWFormat.phyChromaAddr  = FrmInfo.u32ChromaAddr_2bit;
                        pDispFrm->stFrames[1].stHWFormat.u32ChromaPitch = FrmInfo.u16Pitch_2bit;
                        pDispFrm->stFrames[1].u8LumaBitdepth  = FrmInfo.u8LumaBitdepth;
                        pDispFrm->stFrames[1].u8ChromaBitdepth = FrmInfo.u8ChromaBitdepth;
                    }
                }
            }
#endif

#if VSYNCBRIDGE_EXT_HEADER
            if (pAttr->phyVsyncBridgeExtAddr)
            {
#if SUPPORT_FRAME_INFO_EXT_V7
                VDEC_EX_FrameInfoExt_v7 FrmInfoEXT_v7;
                memset(&FrmInfoEXT_v7, 0, sizeof(VDEC_EX_FrameInfoExt_v7));
#define FrmInfoEXT FrmInfoEXT_v7.sFrameInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt
#define FrmInfoEXT_Vx FrmInfoEXT_v7.sFrameInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3
#define FrmInfoEXT_v4 FrmInfoEXT_v7.sFrameInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4
#define FrmInfoEXT_v5 FrmInfoEXT_v7.sFrameInfoExt_v6.sFrameInfoExt_v5
#define FrmInfoEXT_v6 FrmInfoEXT_v7.sFrameInfoExt_v6

                FrmInfoEXT.stVerCtl.u32version = 7;
                FrmInfoEXT.stVerCtl.u32size = sizeof(VDEC_EX_FrameInfoExt_v7);

                if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(pStreamId,
                        E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT, (MS_U32 *)&FrmInfoEXT_v7))

#elif SUPPORT_FRAME_INFO_EXT_V6
                VDEC_EX_FrameInfoExt_v6 FrmInfoEXT_v6;
                memset(&FrmInfoEXT_v6, 0, sizeof(VDEC_EX_FrameInfoExt_v6));
#define FrmInfoEXT FrmInfoEXT_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt
#define FrmInfoEXT_Vx FrmInfoEXT_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3
#define FrmInfoEXT_v4 FrmInfoEXT_v6.sFrameInfoExt_v5.sFrameInfoExt_v4
#define FrmInfoEXT_v5 FrmInfoEXT_v6.sFrameInfoExt_v5

                FrmInfoEXT.stVerCtl.u32version = 6; // with SUPPORT_FRAME_INFO_EXT_V6
                FrmInfoEXT.stVerCtl.u32size = sizeof(VDEC_EX_FrameInfoExt_v6);

                if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(pStreamId,
                        E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT, (MS_U32 *)&FrmInfoEXT_v6))

#elif SUPPORT_FRAME_INFO_EXT_V5
                VDEC_EX_FrameInfoExt_v5 FrmInfoEXT_v5;
#define FrmInfoEXT FrmInfoEXT_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt
#define FrmInfoEXT_Vx FrmInfoEXT_v5.sFrameInfoExt_v4.sFrameInfoExt_v3
#define FrmInfoEXT_v4 FrmInfoEXT_v5.sFrameInfoExt_v4

                FrmInfoEXT.stVerCtl.u32version = 5; // with SUPPORT_FRAME_INFO_EXT_V5
                FrmInfoEXT.stVerCtl.u32size = sizeof(VDEC_EX_FrameInfoExt_v5);

                if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(pStreamId,
                        E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT, (MS_U32 *)&FrmInfoEXT_v5))

#elif SUPPORT_FRAME_INFO_EXT_V4
                VDEC_EX_FrameInfoExt_v4 FrmInfoEXT_v4;
#define FrmInfoEXT FrmInfoEXT_v4.sFrameInfoExt_v3.sFrameInfoExt
#define FrmInfoEXT_Vx FrmInfoEXT_v4.sFrameInfoExt_v3

                FrmInfoEXT.stVerCtl.u32version = 4; // with SUPPORT_FRAME_INFO_EXT_V4
                FrmInfoEXT.stVerCtl.u32size = sizeof(VDEC_EX_FrameInfoExt_v4);

                if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(pStreamId,
                        E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT, (MS_U32 *)&FrmInfoEXT_v4))
#elif SUPPORT_HEVC_HDR_V3
                VDEC_EX_FrameInfoExt_v3 FrmInfoEXT_v3;
#define FrmInfoEXT FrmInfoEXT_v3.sFrameInfoExt
#define FrmInfoEXT_Vx FrmInfoEXT_v3

                FrmInfoEXT.stVerCtl.u32version = 3; // with HDR ver3
                FrmInfoEXT.stVerCtl.u32size = sizeof(VDEC_EX_FrameInfoExt_v3);

                if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(pStreamId,
                        E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT, (MS_U32 *)&FrmInfoEXT_v3))
#elif SUPPORT_HEVC_HDR_V2
                VDEC_EX_FrameInfoExt_v2 FrmInfoEXT_v2;
#define FrmInfoEXT FrmInfoEXT_v2.sFrameInfoExt
#define FrmInfoEXT_Vx FrmInfoEXT_v2

                FrmInfoEXT.stVerCtl.u32version = 2; // with HDR ver2
                FrmInfoEXT.stVerCtl.u32size = sizeof(VDEC_EX_FrameInfoExt_v2);

                if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(pStreamId,
                        E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT, (MS_U32 *)&FrmInfoEXT_v2))
#else

                VDEC_EX_FrameInfoExt FrmInfoEXT;
#if SUPPORT_HEVC_HDR
                FrmInfoEXT.stVerCtl.u32version = 1; // with HDR
#else
                FrmInfoEXT.stVerCtl.u32version = 0;
#endif
                FrmInfoEXT.stVerCtl.u32size = sizeof(VDEC_EX_FrameInfoExt);

                if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(pStreamId,
                        E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT, (MS_U32 *)&FrmInfoEXT))
#endif
                {
                    pDispFrm->stFrames[u32FrmIdx].u8LumaBitdepth  = FrmInfoEXT.u8LumaBitdepth;
                    pDispFrm->stFrames[u32FrmIdx].u8ChromaBitdepth = FrmInfoEXT.u8ChromaBitdepth;

                    if (u32FrmIdx == 0)
                    {
#if SUPPORT_FRAME_INFO_EXT_V4
                        pDispFrm->u8VdecComplexity = FrmInfoEXT_v4.u8ComplexityLevel;
#endif
                        pDispFrm->stFrames[0].stHWFormat.u32MFCodecInfo = FrmInfoEXT.u32MFCodecInfo;

#if SUPPORT_FRAME_INFO_EXT_V5
                        pDispFrm->u32AspectWidth = FrmInfoEXT_v5.u32ParWidth;
                        pDispFrm->u32AspectHeight = FrmInfoEXT_v5.u32ParHeight;
#endif

#if SUPPORT_FRAME_INFO_EXT_V6
                        pDispFrm->u32TileMode = FrmInfoEXT_v6.eTileMode;
                        pDispFrm->u16MIUBandwidth = FrmInfoEXT_v6.u16MIUBandwidth;
                        pDispFrm->u16Bitrate = FrmInfoEXT_v6.u16Bitrate;
                        pDispFrm->u8HTLBTableId = FrmInfoEXT_v6.u8HTLBTableId;
                        pDispFrm->u8HTLBEntriesSize = FrmInfoEXT_v6.u8HTLBEntriesSize;
                        pDispFrm->phyHTLBEntriesAddr = FrmInfoEXT_v6.u32HTLBEntriesAddr;
#endif

#if SUPPORT_FRAME_INFO_EXT_V7
                        pDispFrm->stFrames[0].stHWFormat.u8V7DataValid = FrmInfoEXT_v7.u8V7DataValid;
                        pDispFrm->stFrames[0].stHWFormat.u16Width_subsample = FrmInfoEXT_v7.u16Width_subsample;
                        pDispFrm->stFrames[0].stHWFormat.u16Height_subsample = FrmInfoEXT_v7.u16Height_subsample;
                        pDispFrm->stFrames[0].stHWFormat.phyLumaAddr_subsample = FrmInfoEXT_v7.phyLumaAddr_subsample;
                        pDispFrm->stFrames[0].stHWFormat.phyChromaAddr_subsample = FrmInfoEXT_v7.phyChromaAddr_subsample;
                        pDispFrm->stFrames[0].stHWFormat.u16Pitch_subsample = FrmInfoEXT_v7.u16Pitch_subsample;
                        pDispFrm->stFrames[0].stHWFormat.u8TileMode_subsample = FrmInfoEXT_v7.u8TileMode_subsample;
                        pDispFrm->stFrames[0].stHWFormat.u32HTLBTableAddr = FrmInfoEXT_v7.phyHTLBTableAddr;
                        pDispFrm->stFrames[0].stHWFormat.u8HTLBPageSizes = FrmInfoEXT_v7.u8HTLBPageSizes;
                        pDispFrm->stFrames[0].stHWFormat.u8HTLBChromaEntriesSize = FrmInfoEXT_v7.u8HTLBChromaEntriesSize;
                        pDispFrm->stFrames[0].stHWFormat.u32HTLBChromaEntriesAddr = FrmInfoEXT_v7.phyHTLBChromaEntriesAddr;
                        pDispFrm->stFrames[0].stHWFormat.u16MaxContentLightLevel = FrmInfoEXT_v7.u16MaxContentLightLevel;
                        pDispFrm->stFrames[0].stHWFormat.u16MaxPicAverageLightLevel = FrmInfoEXT_v7.u16MaxPicAverageLightLevel;
                        pDispFrm->stFrames[0].stHWFormat.u64NumUnitsInTick = FrmInfoEXT_v7.u32NumUnitsInTick;
                        pDispFrm->stFrames[0].stHWFormat.u64TimeScale = FrmInfoEXT_v7.u32TimeScale;
                        for (i = 0; i < VDEC_EX_V7_SWDRHISTOGRAM_INDEX; i++)
                        {
                            pDispFrm->stFrames[0].stHWFormat.u16Histogram[i] = FrmInfoEXT_v7.u16Histogram[i];
                        }
#endif

                        // 2 bit info, first frame's 2 bit information is put in stFrames[1]
                        pDispFrm->stFrames[1].stHWFormat.phyLumaAddr = FrmInfoEXT.u32LumaAddr_2bit;
                        pDispFrm->stFrames[1].stHWFormat.phyChromaAddr = FrmInfoEXT.u32ChromaAddr_2bit;
                        pDispFrm->stFrames[1].stHWFormat.u32LumaPitch = FrmInfoEXT.u16Pitch_2bit;
                        pDispFrm->stFrames[1].stHWFormat.u32ChromaPitch = FrmInfoEXT.u16Pitch_2bit;
                        pDispFrm->stFrames[1].u8LumaBitdepth  = FrmInfoEXT.u8LumaBitdepth;
                        pDispFrm->stFrames[1].u8ChromaBitdepth = FrmInfoEXT.u8ChromaBitdepth;

                        pDispFrm->stFrames[0].stHWFormat.phyLumaAddr2Bit = FrmInfoEXT.u32LumaAddr_2bit;
                        pDispFrm->stFrames[0].stHWFormat.phyChromaAddr2Bit = FrmInfoEXT.u32ChromaAddr_2bit;

                        pDispFrm->stFrames[0].stHWFormat.phyMFCBITLEN = FrmInfoEXT.u32LumaMFCbitlen;
#if VSYNC_HVD_HDR_TEST
                        pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_INTERLACE] = FrmInfoEXT.u32LumaAddrI;
                        pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_INTERLACE] = FrmInfoEXT.u32ChromaAddrI;

                        pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_10BIT_INTERLACE] = FrmInfoEXT.u32LumaAddrI_2bit;
                        pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_10BIT_INTERLACE] = FrmInfoEXT.u32ChromaAddrI_2bit;

                        pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_MFCBITLEN] = FrmInfoEXT.u32LumaMFCbitlen;
                        pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_MFCBITLEN] = FrmInfoEXT.u32ChromaMFCbitlen;
#endif
#if SUPPORT_HEVC_HDR_V2 || SUPPORT_HEVC_HDR_V3 || SUPPORT_FRAME_INFO_EXT_V4
#if VSYNC_HDR_TEST
                        pDispFrm->stHDRInfo.stMasterColorDisplay.u32MaxLuminance = FrmInfoEXT_Vx.sDisplay_colour_volume.u32MaxLuminance;
                        pDispFrm->stHDRInfo.stMasterColorDisplay.u32MinLuminance = FrmInfoEXT_Vx.sDisplay_colour_volume.u32MinLuminance;
                        for (i = 0; i < 3; i++)
                        {
                            for (j = 0; j < 2; j++)
                            {
                                pDispFrm->stHDRInfo.stMasterColorDisplay.u16DisplayPrimaries[i][j] = FrmInfoEXT_Vx.sDisplay_colour_volume.u16Primaries[i][j];
                            }
                        }
                        pDispFrm->stHDRInfo.stMasterColorDisplay.u16WhitePoint[0] = FrmInfoEXT_Vx.sDisplay_colour_volume.u16WhitePoint[0];
                        pDispFrm->stHDRInfo.stMasterColorDisplay.u16WhitePoint[1] = FrmInfoEXT_Vx.sDisplay_colour_volume.u16WhitePoint[1];
#endif
                        pDispFrm->stHDRInfo.u32FrmInfoExtAvail = FrmInfoEXT_Vx.u8Frm_Info_Ext_avail;
                        pDispFrm->stHDRInfo.stColorDescription.u8ColorPrimaries = FrmInfoEXT_Vx.u8Colour_primaries;
                        pDispFrm->stHDRInfo.stColorDescription.u8TransferCharacteristics = FrmInfoEXT_Vx.u8Transfer_characteristics;
                        pDispFrm->stHDRInfo.stColorDescription.u8MatrixCoefficients = FrmInfoEXT_Vx.u8Matrix_coefficients;

#if SUPPORT_FRAME_INFO_EXT_V4

                        if ((FrmInfoEXT_Vx.u8DVMode & 0x03) == E_MSVDEC_DOLBY_VISION_SINGLE_LAYER)
                        {
                            // if single layer, no needs to get EL
                            pDispFrm->u32FrameNum = u32FrmCnt = 1;
                        }

                        if (pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_HEVC_DV &&
                                ((FrmInfoEXT_Vx.u8DVMode & 0x03) == E_MSVDEC_DOLBY_VISION_DISABLE) &&
                                !(FrmInfoEXT_Vx.u8Frm_Info_Ext_avail & E_DMS_HDR_METADATA_DOLBY_HDR))
                        {
                            // demux think it is dolbyHDR but it is not, adjust the frame header codec type to correct value. Prevent display issue.
                            pDispFrm->u32CodecType = E_VDEC_EX_CODEC_TYPE_HEVC;
                        }

                        pDispFrm->stHDRInfo.stDolbyHDRInfo.u8DVMode = FrmInfoEXT_Vx.u8DVMode;
                        pDispFrm->stHDRInfo.stDolbyHDRInfo.phyHDRMetadataAddr = FrmInfoEXT_Vx.u32DVMetaDataAddr;
                        pDispFrm->stHDRInfo.stDolbyHDRInfo.u32HDRMetadataSize = FrmInfoEXT_Vx.u32DVMetaDataSize;
                        pDispFrm->stHDRInfo.stDolbyHDRInfo.phyHDRRegAddr = FrmInfoEXT_v4.u32HDRRegAddr;
                        pDispFrm->stHDRInfo.stDolbyHDRInfo.u32HDRRegSize = FrmInfoEXT_v4.u32HDRRegSize;
                        pDispFrm->stHDRInfo.stDolbyHDRInfo.phyHDRLutAddr = FrmInfoEXT_v4.u32HDRLutAddr;
                        pDispFrm->stHDRInfo.stDolbyHDRInfo.u32HDRLutSize = FrmInfoEXT_v4.u32HDRLutSize;
                        pDispFrm->stHDRInfo.stDolbyHDRInfo.u8DMEnable = FrmInfoEXT_v4.bDMEnable;
                        pDispFrm->stHDRInfo.stDolbyHDRInfo.u8CompEnable = FrmInfoEXT_v4.bCompEnable;
                        pDispFrm->stHDRInfo.stDolbyHDRInfo.u8CurrentIndex = FrmInfoEXT_v4.u8CurrentIndex;
#endif

#elif SUPPORT_HEVC_HDR
#if VSYNC_HDR_TEST
                        pDispFrm->stHDRInfo.stMasterColorDisplay.u32MaxLuminance = FrmInfoEXT.maxLuminance;
                        pDispFrm->stHDRInfo.stMasterColorDisplay.u32MinLuminance = FrmInfoEXT.minLuminance;
                        int i, j;
                        for (i = 0; i < 3; i++)
                        {
                            for (j = 0; j < 2; j++)
                            {
                                pDispFrm->stHDRInfo.stMasterColorDisplay.u16DisplayPrimaries[i][j] = FrmInfoEXT.primaries[i][j];
                            }
                        }
                        pDispFrm->stHDRInfo.stMasterColorDisplay.u16WhitePoint[0] = FrmInfoEXT.whitePoint[0];
                        pDispFrm->stHDRInfo.stMasterColorDisplay.u16WhitePoint[1] = FrmInfoEXT.whitePoint[1];
#endif
                        pDispFrm->stHDRInfo.u32FrmInfoExtAvail = FrmInfoEXT.Frm_Info_Ext_avail;
                        pDispFrm->stHDRInfo.stColorDescription.u8ColorPrimaries = FrmInfoEXT.colour_primaries;
                        pDispFrm->stHDRInfo.stColorDescription.u8TransferCharacteristics = FrmInfoEXT.transfer_characteristics;
                        pDispFrm->stHDRInfo.stColorDescription.u8MatrixCoefficients = FrmInfoEXT.matrix_coefficients;
#endif
                    }
                    else if (u32FrmIdx == 1)
                    {
                        pDispFrm->stFrames[1].stHWFormat.u32MFCodecInfo = FrmInfoEXT.u32MFCodecInfo;
                        pDispFrm->stFrames[1].stHWFormat.phyLumaAddr2Bit = FrmInfoEXT.u32LumaAddr_2bit;
                        pDispFrm->stFrames[1].stHWFormat.phyChromaAddr2Bit = FrmInfoEXT.u32ChromaAddr_2bit;

                        pDispFrm->stFrames[1].stHWFormat.phyMFCBITLEN = FrmInfoEXT.u32LumaMFCbitlen;
#if VSYNC_HVD_HDR_TEST
                        pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_10BIT_INTERLACE] = FrmInfoEXT.u32LumaAddr_2bit;
                        pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_10BIT_INTERLACE] = FrmInfoEXT.u32ChromaAddr_2bit;

                        pDispFrm->stDispFrmInfoExt.u16Width = pNextDispFrm->stFrmInfo.u16Width;
                        pDispFrm->stDispFrmInfoExt.u16Height = pNextDispFrm->stFrmInfo.u16Height;
                        pDispFrm->stDispFrmInfoExt.u16Pitch[0] = pNextDispFrm->stFrmInfo.u16Pitch;
                        pDispFrm->stDispFrmInfoExt.u16Pitch[1] = FrmInfoEXT.u16Pitch_2bit;

                        pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_INTERLACE_MVC] = FrmInfoEXT.u32LumaAddrI;
                        pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_INTERLACE_MVC] = FrmInfoEXT.u32ChromaAddrI;


                        pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_10BIT_INTERLACE_MVC] = FrmInfoEXT.u32LumaAddrI_2bit;
                        pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_10BIT_INTERLACE_MVC] = FrmInfoEXT.u32ChromaAddrI_2bit;

                        pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_MFCBITLEN_MVC] = FrmInfoEXT.u32LumaMFCbitlen;
                        pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_MFCBITLEN_MVC] = FrmInfoEXT.u32ChromaMFCbitlen;

#if SUPPORT_HEVC_HDR_V3 || SUPPORT_FRAME_INFO_EXT_V4
                        if (pDispFrm->u32CodecType == E_VDEC_EX_CODEC_TYPE_HEVC_DV)
                        {
                            pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_10BIT_DOLBY_EL] = FrmInfoEXT.u32LumaAddr_2bit;
                            pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_10BIT_DOLBY_EL] = FrmInfoEXT.u32ChromaAddr_2bit;
                            pDispFrm->stDispFrmInfoExt.u32LumaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_DOLBY_META] = FrmInfoEXT_Vx.u32DVMetaDataAddr;
                            pDispFrm->stDispFrmInfoExt.u32ChromaAddrExt[MS_DISP_FRM_INFO_EXT_TYPE_DOLBY_META] = FrmInfoEXT_Vx.u32DVMetaDataSize;
                        }
#endif
#endif
                    }
                }
            }
#endif

        }
        pDispFrm->stFrames[0].stHWFormat.u32LumaPitch2Bit = pDispFrm->stFrames[1].stHWFormat.u32LumaPitch;
        pDispFrm->stFrames[0].stHWFormat.u32ChromaPitch2Bit = pDispFrm->stFrames[1].stHWFormat.u32ChromaPitch;
        pDispFrm->stFrames[1].stHWFormat.u32LumaPitch2Bit = pDispFrm->stFrames[1].stHWFormat.u32LumaPitch;
        pDispFrm->stFrames[1].stHWFormat.u32ChromaPitch2Bit = pDispFrm->stFrames[1].stHWFormat.u32ChromaPitch;
        pDispFrm->phyVsyncBridgeAddr = pAttr->phyVsyncBridgeShmAddr;
        pDispFrm->phyVsyncBridgeExtAddr = pAttr->phyVsyncBridgeExtAddr;
    }
}

static VSYNC_AVSYNC_STATE _VSYNC_AVSync(int VsynDispId, MS_U64 u64Pts)
{
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);
    VDEC_StreamId* pStreamId = &( pVsyncContext->_Attr[VsynDispId].VdecStreamId);
    VSYNC_AVSYNC_STATE res = E_VSYNC_NONE;
    MS_U32 u32diff = 0;
    MS_U32 u32stc = 0;

    if(pAttr->bAvsync == FALSE || pAttr->eStatus == E_VSYNC_STEP_DISP ||  TRUE == _VSYNC_IsTrasitionTrickMode(VsynDispId) || pAttr->u8ByPassMode == TRUE)
        return E_VSYNC_FREERUN;

    u32stc = _VSYNC_GetStc32( VsynDispId);
    u32diff = (MS_U32)DIFF(u32stc, (MS_U32)((u64Pts + pAttr->u32SyncDelay) & 0xFFFFFFFF) );

    if(u32diff <= pAttr->u32Sync_far_threshold )// else, free run mode
    {
        if(u32diff <= pAttr->u32Syn_threshold)
        {
            res =  E_VSYNC_AVSYNC;
            VSYNC_LOG_VERBOSE(VSYNC_PRINT("\033[1;36m[%s] %d  stc = %d pts = %d sys_time = %d  E_VSYNC_AVSYNC  u32diff = %d\033[m\n", __FUNCTION__, loopcnt, ( int)(u32stc / MS_90KHZ) , ( int)((u64Pts + pAttr->u32SyncDelay) & 0xFFFFFFFF) / MS_90KHZ , (int)MsOS_GetSystemTime(), (int)u32diff));
        }
        else
        {
            if ((MS_U32)((u64Pts + pAttr->u32SyncDelay) & 0xFFFFFFFF) > (u32stc))
            {
                res = E_VSYNC_REPEAT;
                VSYNC_LOG_VERBOSE(VSYNC_PRINT("\033[1;33m[%s] %d  stc = %ud pts = %ud sys_time = %d  E_VSYNC_REPEAT\033[m\n", __FUNCTION__, loopcnt, (unsigned int)(u32stc / MS_90KHZ) , (unsigned int) ((u64Pts + pAttr->u32SyncDelay) & 0xFFFFFFFF) / MS_90KHZ , (int)MsOS_GetSystemTime()));
            }
            else
            {
                res = E_VSYNC_DROP;
                VSYNC_LOG_VERBOSE(VSYNC_PRINT("\033[1;35m[%s] %d  stc = %d pts = %d sys_time = %d  E_VSYNC_DROP\033[m\n", __FUNCTION__, loopcnt, (int)(u32stc / MS_90KHZ) , (int) ((u64Pts + pAttr->u32SyncDelay) & 0xFFFFFFFF) / MS_90KHZ , (int)MsOS_GetSystemTime()));
            }
        }
    }
    else
    {
        // exceed far threshold, free run
        res = E_VSYNC_FREERUN;
        VSYNC_LOG_VERBOSE(VSYNC_PRINT("\033[1;45m[%s] %d  stc = %ud pts = %ud sys_time = %d  E_VSYNC_FREERUN  u32diff = %d   u32SyncDelay = %d\033[m\n", __FUNCTION__, loopcnt, (unsigned int)(u32stc) , (unsigned int)((u64Pts + pAttr->u32SyncDelay) & 0xFFFFFFFF), (int)MsOS_GetSystemTime(), (int)u32diff, (int)pAttr->u32SyncDelay));
    }

    VSYNC_LOG_VERBOSE(VSYNC_PRINT("\033[1;36m[%s] %d  res = 0x%x   stc = %d pts = %ud sys_time = %d  u32VsycnDuration = %d frame_cnt = %d\033[m\n", __FUNCTION__, __LINE__, res, (unsigned int)(u32stc / MS_90KHZ), (unsigned int)(((u64Pts + pAttr->u32SyncDelay) & 0xFFFFFFFF) / MS_90KHZ) , (int)MsOS_GetSystemTime(), (int)pAttr->u32VsycnDuration, (int)MApi_VDEC_EX_GetFrameCnt(pStreamId)));

    if(E_VSYNC_DROP == res)
    {
        if( pAttr->u8SlowDrop == 0 )
        {
            //always drop
        }
        else if(pAttr->u8SlowDropCnt < pAttr->u8SlowDrop)
        {
            // slow sync drop
            pAttr->u8SlowDropCnt++;
            res =  E_VSYNC_AVSYNC;
        }
        else
        {
            //slow sync, not drop
            pAttr->u8SlowDropCnt = 0;
        }
    }
    else
    {
        pAttr->u8SlowDropCnt = 0;
    }

    if(E_VSYNC_REPEAT == res)
    {
        pAttr->u32RepeatCnt = ((u32stc - pAttr->u32LastFlipStc) / MS_90KHZ) / pAttr->u32VsycnDuration;
        if( pAttr->u8SlowRepeat == 0 )
        {
            if(pAttr->u32RepeatThreshold == AVSYNC_REPEAT_ALWAYS)
            {
                //always repeat

            }
            else
            {
                if(pAttr->u32RepeatCnt >= pAttr->u32RepeatThreshold)
                {
                    pAttr->u32RepeatCnt = 0;
                    res =  E_VSYNC_AVSYNC;
                }
            }
        }
        else
        {
            if(pAttr->u8SlowRepeatCnt < pAttr->u8SlowRepeat)
            {
                // slow sync repeat, play
                pAttr->u8SlowRepeatCnt++;
                res =  E_VSYNC_AVSYNC;
            }
            else
            {
                //slow sync repeat
                pAttr->u8SlowRepeatCnt = 0;
            }
        }
    }
    else
    {
        pAttr->u8SlowRepeatCnt = 0;
        pAttr->u32RepeatCnt = 0;
    }
    return res;
}

static VSYNC_Result _VSYNC_Proc(int VsynDispId)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);

    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);
    VDEC_EX_DispFrame* pNextDispFrm = NULL;
    VDEC_EX_Result res_get_frame = E_VDEC_EX_FAIL;
    VDEC_StreamId* pStreamId = &( pVsyncContext->_Attr[VsynDispId].VdecStreamId);
    VSYNC_AVSYNC_STATE sync_status = E_VSYNC_NONE;
    MS_BOOL bReady = FALSE;
    ST_DMS_DISPFRAMEFORMAT stDispFramFormat;
    MS_U32 eWinID = 0;
    MS_U32 u32IdleCount = 0;
    MS_U32 u32stc = 0;

    VSYNC_LOG_STATUS(VSYNC_PRINT("\033[1;46m[%s] %d   \033[m\n", __FUNCTION__, __LINE__));

    MS_U32 u32TimeUnit = _VSYNC_CovertTo90Khz(VsynDispId);

    while(pAttr->bUsed)
    {
        loopcnt++;
        eWinID = pAttr->eWinID;

        //handle vdec status
        switch(pAttr->eStatus)
        {
            case E_VSYNC_STOP:
                pAttr->bUsed = FALSE;
                break;
            case E_VSYNC_FLUSH:
                pAttr->bCheckFileEnd = FALSE;
                while(pAttr->u8DispFbQRptr != pAttr->u8DispFbQWptr)
                {
                    MApi_VDEC_EX_ReleaseFrame(pStreamId, &(pAttr->DispFrameQ[pAttr->u8DispFbQRptr]));
                    pAttr->u8DispFbQRptr = (pAttr->u8DispFbQRptr + 1) % VSYNC_DISP_FB_Q_SZ;
                    pAttr->u32DropCnt++;
                }
                pAttr->u8InterlaceModeDropCnt = 0;
                pAttr->u32DropThresholdCnt = 0;
                sync_status = E_VSYNC_NONE;

                MsOS_DelayTask(MIN_SLEEP_TIME);
                continue;
            case E_VSYNC_PAUSE:
                if(sync_status == E_VSYNC_DROP)
                {
                    while(pAttr->u8DispFbQRptr != pAttr->u8DispFbQWptr)
                    {
                        MApi_VDEC_EX_ReleaseFrame(pStreamId, &(pAttr->DispFrameQ[pAttr->u8DispFbQRptr]));
                        pAttr->u8DispFbQRptr = (pAttr->u8DispFbQRptr + 1) % VSYNC_DISP_FB_Q_SZ;
                        pAttr->u32DropCnt++;
                    }
                    pAttr->u8InterlaceModeDropCnt = 0;
                }
                sync_status = E_VSYNC_NONE;
                MsOS_DelayTask(MIN_SLEEP_TIME);
                continue;
            case E_VSYNC_STEP_DISP:
                if(pAttr->bStepDispDone)
                {
                    MsOS_DelayTask(MIN_SLEEP_TIME);
                    continue;
                }
                break;
            case E_VSYNC_PLAY:
            default:
                break;
        }

        // wait vdec info ready & handle seq change

        if (bReady == FALSE)
        {
            if(_MApi_VDEC_EX_V2_CheckDispInfoRdy(pStreamId) != E_VDEC_EX_OK)
            {
                MsOS_DelayTask(TIME_10_MS);
                continue;
            }
            else
            {
                bReady = TRUE;

                if (E_VDEC_EX_OK != MApi_VDEC_EX_GetControl(pStreamId, E_VDEC_EX_USER_CMD_GET_VSYNC_BRIDGE_ADDR, (MS_U32*) & (pAttr->phyVsyncBridgeShmAddr)))
                {
                    VSYNC_LOG_DBG(VSYNC_PRINT("E_VDEC_EX_USER_CMD_GET_VSYNC_BRIDGE_ADDR FAIL!!!"));
                }
                if (E_VDEC_EX_OK != MApi_VDEC_EX_GetControl(pStreamId, E_VDEC_EX_USER_CMD_GET_VSYNC_BRIDGE_EXT_ADDR, (MS_U32*) & (pAttr->phyVsyncBridgeExtAddr)))
                {
                    VSYNC_LOG_DBG(VSYNC_PRINT("E_VDEC_EX_USER_CMD_GET_VSYNC_BRIDGE_EXT_ADDR FAIL!!!"));
                }
            }
        }

        if(E_VSYNC_REPEAT != sync_status || res_get_frame != E_VDEC_EX_OK)// need next frame: 1. not repeat, 2. get frame fail
        {
            res_get_frame = MApi_VDEC_EX_GetNextDispFrame(pStreamId, &pNextDispFrm);
            if(res_get_frame == E_VDEC_EX_OK)
            {
                u32IdleCount = 0;
                pAttr->u32GetNextFrameTime = MsOS_GetSystemTime();
                pAttr->u32DbgGetFrameCnt++;

                while(pAttr->u8DispFbQRptr != pAttr->u8DispFbQWptr)
                {
                    // only keep one frame in vsync ctrl
                    _VSYNC_DropFrame( VsynDispId, &(pAttr->DispFrameQ[pAttr->u8DispFbQRptr]), u32TimeUnit, __LINE__);
                    pAttr->u8DispFbQRptr = (pAttr->u8DispFbQRptr + 1) % VSYNC_DISP_FB_Q_SZ;
                }

                if((pAttr->u8DispFbQWptr + 1) % VSYNC_DISP_FB_Q_SZ == pAttr->u8DispFbQRptr)
                {
                    VSYNC_LOG_DBG(VSYNC_PRINT("QUEUE FULL !!!\n"));
                    _VSYNC_DropFrame( VsynDispId, pNextDispFrm, u32TimeUnit, __LINE__);
                    MsOS_DelayTask(TIME_10_MS);
                    res_get_frame = E_VDEC_EX_FAIL;
                    continue;
                }
                if(_VSYNC_IsInterlaceFieldMode(VsynDispId) && pAttr->u8InterlaceModeDropCnt % 2 == 1)
                {
                    VSYNC_LOG_DBG(VSYNC_PRINT("Release second hevc interlace field \n"));
                    _VSYNC_DropFrame( VsynDispId, pNextDispFrm, u32TimeUnit, __LINE__);
                    res_get_frame = E_VDEC_EX_FAIL;
                    continue;
                }
                if(_VSYNC_IsTrasitionTrickMode(VsynDispId) == TRUE || (_VSYNC_IsSmoothTrickMode(VsynDispId) == TRUE && pAttr->eSpeedType == E_VDEC_EX_SPEED_FAST))
                {
                    // display one drop one to make X2 speed effect
                    pAttr->u8TraditionalDispCnt++;
                    if(pAttr->u8TraditionalDispCnt % pAttr->eSpeed == 0)
                    {
                        _VSYNC_DropFrame( VsynDispId, pNextDispFrm, u32TimeUnit, __LINE__);
                        res_get_frame = E_VDEC_EX_FAIL;
                        continue;
                    }
                }

                memcpy(&(pAttr->DispFrameQ[pAttr->u8DispFbQWptr]), pNextDispFrm, sizeof(VDEC_EX_DispFrame));
                pAttr->u8DispFbQWptr = (pAttr->u8DispFbQWptr + 1) % VSYNC_DISP_FB_Q_SZ;

                pAttr->u64NextPts = (MS_U64) (((((MS_U64)pNextDispFrm->stFrmInfo.u32ID_H) << 32 | (MS_U64)pNextDispFrm->stFrmInfo.u32TimeStamp) * u32TimeUnit) + (MS_U64)pAttr->u32SyncDelay);

                _VSYNC_HandleSequenceChange(VsynDispId);
                if( (pAttr->bFirstFrame == FALSE && pAttr->u8EnableSetDigitalSignal != E_VSYNC_DIGITAL_SYNC_CTRL_FORCE_DIS ) || pAttr->u8EnableSetDigitalSignal == E_VSYNC_DIGITAL_SYNC_CTRL_FORCE_EN )
                {
                    pAttr->bFirstFrame = TRUE;
                    pAttr->u8EnableSetDigitalSignal = E_VSYNC_DIGITAL_SYNC_CTRL_DEFAULT;

                    _VSYNC_SetupDispFrameFormat(VsynDispId, &stDispFramFormat, pNextDispFrm);

                    if(VSYNC_INVALID_WIN_ID != eWinID)
                    {
                        if(pAttr->VsyncDispConnect != NULL)
                            pAttr->VsyncDispConnect(eWinID, &stDispFramFormat);
                        else
                        {
                            MApi_DMS_SetDigitalDecodeSignalInfo(eWinID, &stDispFramFormat);
                        }
                    }
                }

                if(pAttr->u8IsSeqChange )
                {
                    if(pAttr->Dispinfo.u16HorSize == pNextDispFrm->stFrmInfo.u16Width && pAttr->Dispinfo.u16VerSize == pNextDispFrm->stFrmInfo.u16Height)
                    {
                        memcpy(&(pAttr->DispinfoOutput), &(pAttr->Dispinfo), sizeof(VDEC_EX_DispInfo));
                        //_MApi_VDEC_EX_V2_GetDispInfo(pStreamId, &(pAttr->DispinfoOutput));
                        if(pAttr->VsyncFireVdecCallback != NULL)
                        {
                            pAttr->VsyncFireVdecCallback(pStreamId, E_VDEC_EX_EVENT_DISP_INFO_CHG);
                        }
                        pAttr->u8IsSeqChange = FALSE;
                        pAttr->bDispInfoRdy = TRUE;
                    }
                }
                else //if(pAttr->DispinfoOutput.u8AspectRate != pAttr->Dispinfo.u8AspectRate)
                {
                    memcpy(&(pAttr->DispinfoOutput), &(pAttr->Dispinfo), sizeof(VDEC_EX_DispInfo));
                }

                if(pAttr->u32DispCnt > 0 && _VSYNC_IsSmoothTrickMode(VsynDispId) == TRUE)
                {
                    // check data discontinuouis to modify clock
                    int ptr = 0;
                    if(pAttr->u8DispFbQWptr > 1)
                        ptr = pAttr->u8DispFbQWptr - 2;
                    else if(pAttr->u8DispFbQWptr > 0)
                        ptr = VSYNC_DISP_FB_Q_SZ - 1;
                    else
                        ptr = VSYNC_DISP_FB_Q_SZ - 2;

                    VDEC_EX_DispFrame* pPrevDispFrm = &(pAttr->DispFrameQ[ptr]);

                    if(((MS_U32)(pNextDispFrm->stFrmInfo.u32TimeStamp * u32TimeUnit) - (MS_U32)(pPrevDispFrm->stFrmInfo.u32TimeStamp * u32TimeUnit)) / MS_90KHZ > (pAttr->u32VsycnDuration * 10))
                    {
                        signed long long new_stc = (signed long long)(pNextDispFrm->stFrmInfo.u32TimeStamp * u32TimeUnit) - (signed long long)(pAttr->u32latch_pts) + (signed long long)(pAttr->u32latch_stc);
                        pAttr->u32latch_stc = (MS_U32)(new_stc & 0xFFFFFFFF);
                        pAttr->u32latch_pts = pNextDispFrm->stFrmInfo.u32TimeStamp * u32TimeUnit;
                        pAttr->u32latch_time = MsOS_GetSystemTime();
                    }
                }
            }
            else// no next frame
            {
                u32IdleCount++;
                if(u32IdleCount % 5000 == 4999)
                {
                    VSYNC_LOG_IDLE(VSYNC_PRINT("\033[1;46m[%s] %d  u32IdleCount = %d   frame_cnt = %d\033[m\n", __FUNCTION__, __LINE__, (int)u32IdleCount, (int)MApi_VDEC_EX_GetFrameCnt(pStreamId)));
                }
            }
        }

        if(pAttr->bCheckFileEnd == TRUE && pAttr->bLastFrameShowDone == FALSE)
        {
            MS_BOOL bGotPattern = FALSE;
            MApi_VDEC_EX_GetControl(pStreamId, E_VDEC_EX_USER_CMD_GET_FILE_END_PATTERN, (MS_U32*)(&bGotPattern));

            if(bGotPattern == TRUE && res_get_frame == E_VDEC_EX_FAIL && pAttr->u8DispFbQRptr == pAttr->u8DispFbQWptr)
            {
                pAttr->bLastFrameShowDone = TRUE;
                pAttr->bCheckFileEnd = FALSE;
            }
        }
        u32stc = _VSYNC_GetStc32( VsynDispId);

        //update avsync status
        switch(sync_status)
        {
            case E_VSYNC_FREERUN:
            case E_VSYNC_AVSYNC:
            case E_VSYNC_REPEAT:
                if(res_get_frame == E_VDEC_EX_OK)
                    sync_status =  _VSYNC_AVSync( VsynDispId, (MS_U64)(((MS_U64)pNextDispFrm->stFrmInfo.u32ID_H) << 32 | pNextDispFrm->stFrmInfo.u32TimeStamp * u32TimeUnit));
                else
                    sync_status = E_VSYNC_REPEAT;
                break;
            case E_VSYNC_DROP:
#if (VSYNC_DROP_DBG == 0)
                if(res_get_frame == E_VDEC_EX_OK)
                {
                    //sync_status =  _VSYNC_AVSync( VsynDispId, (MS_U64)(pNextDispFrm->stFrmInfo.u32TimeStamp));
                    sync_status =  _VSYNC_AVSync( VsynDispId, (MS_U64)((((MS_U64)pNextDispFrm->stFrmInfo.u32ID_H) << 32 | pNextDispFrm->stFrmInfo.u32TimeStamp)) * u32TimeUnit);
                }
                else
                {
                    // get next frame fail
                    if(pAttr->u8DispFbQRptr != pAttr->u8DispFbQWptr)
                    {
                        // only one frame in queue, display it rather than drop it
                        if((pAttr->u32DropThresholdCnt >= VSYNC_DROP_THRESHOLD) && (!_VSYNC_IsInterlaceFieldMode(VsynDispId) ||  pAttr->u8InterlaceModeDropCnt % 2 == 0)   )
                        {
                            VSYNC_LOG_DBG(VSYNC_PRINT("\033[1;31m[%s] %d only one frame , display it (%d %d)  u32LastFlipStc = %d  u32stc = %d  u32DropThresholdCnt = %d\033[m\n", __FUNCTION__, __LINE__, (int)pAttr->u8DispFbQRptr, (int)pAttr->u8DispFbQWptr, (int)(pAttr->u32LastFlipStc / MS_90KHZ), (int)(u32stc / MS_90KHZ), (int)pAttr->u32DropThresholdCnt));
                            pAttr->u8SlowDropCnt = 0;
                            sync_status = E_VSYNC_FREERUN;
                            pAttr->u32DropThresholdCnt = 0;
                        }
                    }
                }
#else
                if(res_get_frame != E_VDEC_EX_OK)
                    sync_status = E_VSYNC_REPEAT;
                else
                    sync_status =  _VSYNC_AVSync( VsynDispId, (MS_U64)((((MS_U64)pNextDispFrm->stFrmInfo.u32ID_H) << 32 | pNextDispFrm->stFrmInfo.u32TimeStamp)) * u32TimeUnit);
#endif
                break;
            case E_VSYNC_NONE:
                if(res_get_frame == E_VDEC_EX_OK)
                {
                    if(pAttr->bShowFirstFrameDirect == TRUE && pAttr->u32DispCnt == 0)
                    {
                        sync_status = E_VSYNC_AVSYNC;
                    }
                    else
                    {
                        //sync_status =  _VSYNC_AVSync( VsynDispId, (MS_U64)(pNextDispFrm->stFrmInfo.u32TimeStamp));
                        sync_status =  _VSYNC_AVSync( VsynDispId, (MS_U64)((((MS_U64)pNextDispFrm->stFrmInfo.u32ID_H) << 32 | pNextDispFrm->stFrmInfo.u32TimeStamp)) * u32TimeUnit);
                    }
                }
                else
                {
                    MsOS_DelayTask(TIME_10_MS);
                    continue;
                }
                break;
            default:
                break;
        }
#if VSYNC_DROP_DBG
        if(sync_status == E_VSYNC_DROP)
        {
            MApi_VDEC_EX_ReleaseFrame(pStreamId, &(pAttr->DispFrameQ[pAttr->u8DispFbQRptr]));
            pAttr->u8DispFbQRptr = (pAttr->u8DispFbQRptr + 1) % VSYNC_DISP_FB_Q_SZ;
            pAttr->u32DropCnt++;
        }
#else
        if(sync_status != E_VSYNC_DROP)
        {
            pAttr->u32DropThresholdCnt = 0;
        }
        else
        {
            if((pAttr->u32DropThresholdCnt < VSYNC_DROP_THRESHOLD))
            {
                if(pAttr->u8DispFbQRptr != pAttr->u8DispFbQWptr)
                {
                    _VSYNC_DropFrame( VsynDispId, &(pAttr->DispFrameQ[pAttr->u8DispFbQRptr]), u32TimeUnit, __LINE__);
                    pAttr->u8DispFbQRptr = (pAttr->u8DispFbQRptr + 1) % VSYNC_DISP_FB_Q_SZ;
                }
            }
        }
#endif
        if(sync_status != E_VSYNC_DROP && pNextDispFrm != NULL && pAttr->u8DispFbQRptr != pAttr->u8DispFbQWptr)
        {
            if((sync_status == E_VSYNC_AVSYNC || sync_status == E_VSYNC_FREERUN))
            {
                //show frame
                if(E_VSYNC_FREEZE != pAttr->eStatus)
                {
                    _VSYNC_SetupDispFrameFormat(VsynDispId, &stDispFramFormat, pNextDispFrm);

                    if(VSYNC_INVALID_WIN_ID != eWinID)
                    {
                        if(pAttr->VsyncDispFlip != NULL)
                            pAttr->VsyncDispFlip(eWinID , &stDispFramFormat);
                        else
                        {
                            MApi_DMS_Video_Flip(eWinID , &stDispFramFormat);
                        }
                    }

                    if(pAttr->VsyncFireVdecCallback != NULL)
                    {
                        pAttr->VsyncFireVdecCallback(pStreamId, E_VDEC_EX_EVENT_DISP_ONE);
                    }

                    pAttr->u32DispCnt++;
                    pAttr->u32DbgDispCnt++;
                    pAttr->u64Pts = (MS_U64) ((((MS_U64)pNextDispFrm->stFrmInfo.u32ID_H) << 32 | (MS_U64)pNextDispFrm->stFrmInfo.u32TimeStamp) * u32TimeUnit) + (MS_U64)pAttr->u32SyncDelay;

                    MS_U32 u32pts = (MS_U32)pAttr->u64Pts;
                    pAttr->s64PtsStcDelta = (MS_S64)(u32pts) -  (MS_S64)(u32stc);
                    pAttr->u32LastFlipStc = u32stc;

                    VSYNC_LOG_TIME( VSYNC_PRINT("\033[1;32m[%s] %d  u32stc = %d  u32pts = %d  systime = %d  sync_status = 0x%x  d_cnt:%d r_cnt:%d  getFrmTime = %d\033[m\n", __FUNCTION__, __LINE__, (int)(u32stc / MS_90KHZ), (int)(u32pts / MS_90KHZ), (int)MsOS_GetSystemTime() , (int)sync_status, (int)pAttr->u32DropCnt, (int)pAttr->u32RepeatCnt, (int)pAttr->u32GetNextFrameTime));

                    if(pAttr->u32DbgGetFrameCnt >= 50)
                    {
                        VSYNC_LOG_DBG(VSYNC_PRINT("\033[1;46m[%s] %d disp ratio = %d/100  pAttr->u32DropCnt = %d  (%d, %d)\033[m\n", __FUNCTION__, __LINE__, (int)((pAttr->u32DbgDispCnt * 100) / pAttr->u32DbgGetFrameCnt), (int)pAttr->u32DropCnt, (int)pAttr->u32DbgDispCnt, (int)pAttr->u32DbgGetFrameCnt));
                        pAttr->u32DbgDispCnt = 0;
                        pAttr->u32DbgGetFrameCnt = 0;
                    }
                }
                if(pAttr->u8FirstFrameRdy == 0)
                {
                    pAttr->u8FirstFrameRdy = 1;
                    VSYNC_PRINT("\033[1;42m[%s] %d \033[m\n", __FUNCTION__, __LINE__);
                }

                if(sync_status == E_VSYNC_AVSYNC)
                {
                    pAttr->bAvsyncDone = TRUE;
                }
                else
                {
                    pAttr->bAvsyncDone = FALSE;
                }

                while(pAttr->u8DispFbQRptr != pAttr->u8DispFbQWptr)
                {
                    MApi_VDEC_EX_ReleaseFrame(pStreamId, &(pAttr->DispFrameQ[pAttr->u8DispFbQRptr]));
                    pAttr->u8DispFbQRptr = (pAttr->u8DispFbQRptr + 1) % VSYNC_DISP_FB_Q_SZ;
                }

                if(pAttr->bStepDispDone == FALSE && pAttr->eStatus == E_VSYNC_STEP_DISP)
                {
                    pAttr->bStepDispDone = TRUE;
                }

            }
            else
            {
                // repeat
            }
            if(pAttr->u8ByPassMode == TRUE)
            {
                MsOS_DelayTask(MIN_SLEEP_TIME);
            }
            else
            {
                MS_U32 u32StcBefore =  _VSYNC_GetStc32( VsynDispId);
                if(sync_status == E_VSYNC_AVSYNC && res_get_frame == E_VDEC_EX_OK)//avsync not because of last frame in display queue
                {
                    if(((MS_U32)(pAttr->u64Pts)*u32TimeUnit ) / MS_90KHZ + pAttr->u32VsycnDuration <= u32StcBefore / MS_90KHZ)
                    {
                        continue;
                    }
                }
                else if(sync_status == E_VSYNC_REPEAT && res_get_frame == E_VDEC_EX_OK)//repeat not because no next frame
                {
                    if(((MS_U32)pAttr->u64NextPts * u32TimeUnit) / MS_90KHZ <  u32StcBefore / MS_90KHZ )
                    {
                        continue;
                    }
                }
                MsOS_DelayTask(MIN_SLEEP_TIME);
            }
        }
        else if(res_get_frame != E_VDEC_EX_OK)//no next frame
        {
            if(pAttr->u8ByPassMode == TRUE)
            {
                MsOS_DelayTask(MIN_SLEEP_TIME);
            }
            else
            {
#if (VSYNC_DROP_DBG == 0)
                if(sync_status != E_VSYNC_DROP)
#endif
                    MsOS_DelayTask(MIN_SLEEP_TIME);
            }
        }
    }

    while(pAttr->u8DispFbQRptr != pAttr->u8DispFbQWptr)
    {
        MApi_VDEC_EX_ReleaseFrame(pStreamId, &(pAttr->DispFrameQ[pAttr->u8DispFbQRptr]));
        pAttr->u8DispFbQRptr = (pAttr->u8DispFbQRptr + 1) % VSYNC_DISP_FB_Q_SZ;
        //VSYNC_DBG_LOG("\033[1;44m[%s] %d rw(%d , %d)  \033[m\n",__FUNCTION__,__LINE__,pAttr->u8DispFbQRptr ,pAttr->u8DispFbQWptr );
    }
    pAttr->eStatus = E_VSYNC_STOP;
    return E_VSYNC_OK;
}


static VSYNC_Result _VSYNC_Context_Init(void)
{
    MS_U8 i;

    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    pVsyncContext->bDmsInit = FALSE;

    for (i = 0; i < VSYNC_MAX_DISP_PATH; i++)
    {
        VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[i]);
        memset(pAttr, 0, sizeof(VSYNC_Attr));
    }

    return E_VSYNC_OK;
}

static void _VSYNC_Variable_Init(int VsynDispId)
{
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);
    pAttr->bUsed = TRUE;
    pAttr->u32latch_stc = 0;
    pAttr->u32latch_time = 0;
    pAttr->bSmoothTrickMode = FALSE;
    pAttr->phyVsyncBridgeExtAddr = 0;
    pAttr->u32SyncDelay = 0;
    pAttr->u32Sync_tolerance = 0;
    pAttr->u32Syn_threshold = VSYNC_DEFAULT_LOOP_TIME * MS_90KHZ;
    pAttr->u32Sync_far_threshold = 1000 * MS_90KHZ;
    pAttr->u32VsycnDuration = VSYNC_DEFAULT_LOOP_TIME;
    pAttr->eStatus = E_VSYNC_PLAY;
    pAttr->u8SlowRepeat = 0 * VSYNC_SAMPLE_RATE;
    pAttr->u8SlowDrop = 0;
    pAttr->u64Pts = 0xFFFFFFFFFFFFFFFF;
    pAttr->bAvsync = TRUE;
    pAttr->bAvsyncDone = FALSE;
    pAttr->bFirstFrame = FALSE;
    pAttr->bStepDispDone = FALSE;
    pAttr->bLastFrameShowDone = FALSE;
    pAttr->bCheckFileEnd = FALSE;
    pAttr->eSyncMode = E_VSYNC_STC_MODE;
    pAttr->u8DispFbQWptr = 0;
    pAttr->u8DispFbQRptr = 0;
    pAttr->u32DispCnt = 0;
    pAttr->u32DropCnt = 0;
    pAttr->u32RepeatCnt = 0;
    pAttr->u8FirstFrameRdy = 0;
    pAttr->s32VSyncTaskId = -1;
    pAttr->eStcMode = E_VDEC_EX_STC0;
    pAttr->u32RepeatThreshold = AVSYNC_REPEAT_ALWAYS;
    pAttr->u8ByPassMode = FALSE;
    pAttr->eSpeed = E_VDEC_EX_DISP_SPEED_1X;
    pAttr->eSpeedType = E_VDEC_EX_SPEED_DEFAULT;
    pAttr->bShowFirstFrameDirect = FALSE;
    pAttr->pfVsync_Proc = _VSYNC_Proc;
    pAttr->u8IsSeqChange = FALSE;
    pAttr->bSelf_SeqChange = FALSE;
    pAttr->bSuspendDS = FALSE;
    pAttr->bSuspendDS_Ftime = TRUE;
    pAttr->bDispInfoRdy = FALSE;
    pAttr->u32DropThresholdCnt = 0;
    pAttr->u8InterlaceModeDropCnt = 0;
    pAttr->u32LastFlipStc = 0;
    pAttr->u32GetNextFrameTime = 0;
    pAttr->u32DbgGetFrameCnt = 0;
    pAttr->u32DbgDispCnt = 0;
    pAttr->u8EnableSetDigitalSignal = E_VSYNC_DIGITAL_SYNC_CTRL_DEFAULT;
    pAttr->u8EnableClearDigitalSignal = E_VSYNC_DIGITAL_SYNC_CTRL_DEFAULT;
    memset(&(pAttr->DispinfoOutput), 0, sizeof(VDEC_EX_DispInfo));
}

static VSYNC_Result _VSYNC_Init_Share_Mem(void)
{
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_ECOS)
#if !defined(SUPPORT_X_MODEL_FEATURE)
    MS_U32 u32ShmId;
    MS_VIRT u32Addr;
    MS_U32 u32BufSize;

    if (FALSE == MsOS_SHM_GetId( (MS_U8*)"Linux VSYNC driver",
                                 sizeof(VSYNC_CTX),
                                 &u32ShmId,
                                 &u32Addr,
                                 &u32BufSize,
                                 MSOS_SHM_QUERY))
    {
        if (FALSE == MsOS_SHM_GetId((MS_U8*)"Linux VSYNC driver",
                                    sizeof(VSYNC_CTX),
                                    &u32ShmId,
                                    &u32Addr,
                                    &u32BufSize,
                                    MSOS_SHM_CREATE))
        {
            VSYNC_PRINT("[%s]SHM allocation failed!!!use global structure instead !!!\n", __FUNCTION__);
            if(pVsyncContext == NULL)
            {
                pVsyncContext = &gVsyncContext;
                memset(pVsyncContext, 0, sizeof(VSYNC_CTX));
                _VSYNC_Context_Init();
                VSYNC_PRINT("[%s]Global structure init Success!!!\n", __FUNCTION__);
            }
            else
            {
                VSYNC_PRINT("[%s]Global structure exists!!!\n", __FUNCTION__);
            }
        }
        else
        {
            memset((MS_U8*)u32Addr, 0, sizeof(VSYNC_CTX));
            pVsyncContext = (VSYNC_CTX*)u32Addr; // for one process
            _VSYNC_Context_Init();
        }
    }
    else
    {
        pVsyncContext = (VSYNC_CTX*)u32Addr; // for another process
    }
#else
    if(pVsyncContext == NULL)
    {
        pVsyncContext = &gVsyncContext;
        memset(pVsyncContext, 0, sizeof(VSYNC_CTX));
        _VSYNC_Context_Init();
    }
#endif

    if(_VSYNC_API_MutexCreate() != TRUE)
    {
        VSYNC_PRINT("[%s] API Mutex create failed!\n", __FUNCTION__);
        return E_VSYNC_FAIL;
    }
#else
    if(pVsyncContext == NULL)
    {
        pVsyncContext = &gVsyncContext;
        memset(pVsyncContext, 0, sizeof(VSYNC_CTX));
        _VSYNC_Context_Init();
    }
#endif

    return E_VSYNC_OK;
}

static MS_BOOL _VSYNC_Task_Init(int VsynDispId)
{
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    if(pAttr->s32VSyncTaskId == -1)
    {
        strncpy(pAttr->pu8VsyncTask, "VSYNC_EX_SyncTask", sizeof(pAttr->pu8VsyncTask));

        pAttr->s32VSyncTaskId = MsOS_CreateTask((TaskEntry) pAttr->pfVsync_Proc,
                                                VsynDispId,
                                                //E_TASK_PRI_MEDIUM,
                                                E_TASK_PRI_HIGH,
                                                TRUE,
                                                (void *)pAttr->u8VsyncStack,
                                                VSYNC_STACK_SIZE,
                                                pAttr->pu8VsyncTask);
        if (pAttr->s32VSyncTaskId < 0)
        {
            VSYNC_PRINT("MsOS_CreateTask failed!!\n");
            return FALSE;
        }
    }

    return TRUE;
}

//--------------------------------------------------------------------------------------------------
//  Local functions
//--------------------------------------------------------------------------------------------------

VSYNC_Result MApi_VSync_Init(int VsynDispId, VDEC_StreamId *pStreamId, VDEC_EX_InitParam *pInitParam, MS_U32 eWinID, MS_U32 u32OverlayId)
{
    if(E_VSYNC_OK != _VSYNC_Init_Share_Mem())
        return E_VSYNC_FAIL;

    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);

    _VSYNC_API_MutexLock();

    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    if(pAttr->bUsed == TRUE)
    {
        VSYNC_PRINT("\033[1;33m[%s] %d pVsyncContext->_Attr[%d].bUsed = TRUE  \033[m\n", __FUNCTION__, __LINE__, VsynDispId);
        _VSYNC_API_MutexUnlock();
        return E_VSYNC_FAIL;
    }

    pAttr->VsynDispId = VsynDispId;
    pAttr->eWinID = eWinID;
    pAttr->u32OverlayId = u32OverlayId;
    pAttr->VdecStreamId.u32Id = pStreamId->u32Id;
    pAttr->VdecStreamId.u32Version = pStreamId->u32Version;
    pAttr->CodecType = pInitParam->eCodecType;
    pAttr->SrcMode = pInitParam->VideoInfo.eSrcMode;
    pAttr->bEnableDynScale = pInitParam->EnableDynaScale;

    _VSYNC_Variable_Init(VsynDispId);

    _VSYNC_API_MutexUnlock();

    if(!_VSYNC_Task_Init(VsynDispId))
    {
        VSYNC_PRINT("\033[1;31m[%s] %d  FAIL !!!  \033[m\n", __FUNCTION__, __LINE__);
        return E_VSYNC_FAIL;
    }

    if(pVsyncContext->bDmsInit == FALSE)
    {
        ST_DMS_INITDATA stInitData;
        MApi_DMS_Init(&stInitData);
        pVsyncContext->bDmsInit = TRUE;
    }

    _VSYNC_API_MutexLock();
    pAttr->bInit = TRUE;
    _VSYNC_API_MutexUnlock();

    return E_VSYNC_OK;
}

VSYNC_Result MApi_VSync_Deinit(int VsynDispId)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    _VSYNC_API_MutexLock();
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);
    VSYNC_LOG_STATUS(VSYNC_PRINT("[%s] %d %d \n", __FUNCTION__, __LINE__, VsynDispId));

    if(pAttr->bInit == FALSE)
    {
        _VSYNC_API_MutexUnlock();
        return E_VSYNC_FAIL;
    }

    pAttr->bUsed = FALSE;
    int timeout = 0;
    while(pAttr->eStatus != E_VSYNC_STOP && timeout < 1000)
    {
        timeout++;
        MsOS_DelayTask(MIN_SLEEP_TIME);
    }
    pAttr->bInit = FALSE;
    pAttr->s32VSyncTaskId = -1;
    pAttr->VsynDispId = 0;
    pAttr->VdecStreamId.u32Id = 0;

    if(pAttr->u8EnableClearDigitalSignal != E_VSYNC_DIGITAL_SYNC_CTRL_FORCE_DIS)
    {
        pAttr->u8EnableClearDigitalSignal = E_VSYNC_DIGITAL_SYNC_CTRL_DEFAULT;

        if(pAttr->VsyncDispDisconnect != NULL)
            pAttr->VsyncDispDisconnect(pAttr->eWinID);
        else
        {
            if(VSYNC_INVALID_WIN_ID != pAttr->eWinID)
                MApi_DMS_ClearDigitalDecodeSignalInfo(pAttr->eWinID);
        }
    }

    _VSYNC_API_MutexUnlock();


    // TODO: check this flow with jeff
    EN_DMS_CAPABILITY eCapability;
    MApi_DMS_GetCapability(&eCapability);
    if((eCapability & E_DMS_CAPABILITY_MULTI_WINDOW) != E_DMS_CAPABILITY_MULTI_WINDOW && VSYNC_INVALID_WIN_ID != pAttr->eWinID)
        MApi_DMS_DestroyWindow(pAttr->eWinID);

    return E_VSYNC_OK;
}

VSYNC_Result MApi_VSync_AVSyncOn(int VsynDispId, MS_BOOL bOn, MS_U32 u32SyncDelay, MS_U16 u16SyncTolerance)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    pAttr->bAvsync = bOn;
    pAttr->u32SyncDelay = u32SyncDelay * MS_90KHZ;
    pAttr->u32Sync_tolerance = u16SyncTolerance * MS_90KHZ;
    if(pAttr->u32Sync_tolerance != 0)
    {
        pAttr->u32Syn_threshold = pAttr->u32Sync_tolerance;
    }
    if(!pAttr->bAvsync)
        pAttr->bAvsyncDone = FALSE;

    VSYNC_LOG_CMD(VSYNC_PRINT("\033[1;41m[%s] %d  pAttr->u32Sync_tolerance = %d   pAttr->u32SyncDelay = %d\033[m\n", __FUNCTION__, __LINE__, (int)pAttr->u32Sync_tolerance, (int)pAttr->u32SyncDelay));

    return E_VSYNC_OK;
}

VDEC_EX_Result MApi_VSync_SetControl(int VsynDispId, VSYNC_User_Cmd cmd_id, void* param)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    switch(cmd_id)
    {
        case E_VSYNC_USER_CMD_SET_DISP_WIN_ID:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_DISP_WIN_ID  \n", __FUNCTION__, __LINE__));
            _VSYNC_API_MutexLock();
            if (param != NULL) pAttr->eWinID = (*(MS_U32*)param);
            _VSYNC_API_MutexUnlock();
            break;
        case E_VSYNC_USER_CMD_SET_STATUS:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_STATUS  0x%x \n", __FUNCTION__, __LINE__, (*(VSYNC_Vdec_Status*)param)));
            if (param != NULL) pAttr->eStatus = (*(VSYNC_Vdec_Status*)param);
            break;
        case E_VSYNC_USER_CMD_SET_3D_MODE:                  //may used by dms
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_3D_MODE  \n", __FUNCTION__, __LINE__));
            if (param != NULL) pAttr->u83DMode = (*(MS_U8*)param);
            break;
        case E_VSYNC_USER_CMD_SET_3DLAYOUTUNLOCKED://may used by dms
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_3DLAYOUTUNLOCKED  \n", __FUNCTION__, __LINE__));
            if (param != NULL) pAttr->u8MsVdec3DLayoutUnlocked = (*(MS_U8*)param);
            break;
        case E_VSYNC_USER_CMD_SET_LOW_LATENCY:          //may used by dms
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_LOW_LATENCY  \n", __FUNCTION__, __LINE__));
            if (param != NULL) pAttr->u8LowLatencyMode = (*(MS_U8*)param);
            break;
        case E_VSYNC_USER_CMD_SET_SLOW_SYNC:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_SLOW_SYNC  \n", __FUNCTION__, __LINE__));
            if (param != NULL)
            {
                pAttr->u8SlowDrop = (*(MS_U8*)param);
                pAttr->u8SlowRepeat = (*(MS_U8*)param) * VSYNC_SAMPLE_RATE;
            }
            break;
        case E_VSYNC_USER_CMD_SET_AVSYNC_FREERUN_THRESHOLD:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_AVSYNC_FREERUN_THRESHOLD  \n", __FUNCTION__, __LINE__));
            if (param != NULL) pAttr->u32Sync_far_threshold = (*(MS_U32*)param);
            break;
        case E_VSYNC_USER_CMD_SET_DISP_ONE_FIELD:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_DISP_ONE_FIELD  \n", __FUNCTION__, __LINE__));
            if (param != NULL) pAttr->u8FieldCtrl = (*(MS_U32*)param); //0-> Normal, 1->always top, 2->always bot
            break;
        case E_VSYNC_USER_CMD_SET_MVOP_MCU_MODE:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_MVOP_MCU_MODE  \n", __FUNCTION__, __LINE__));
            if (param != NULL) pAttr->u8MvopMCUMode = (*(MS_U8*)param);
            break;
        case E_VSYNC_USER_CMD_SET_SHOW_FIRST_FRAME_DIRECT:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_SHOW_FIRST_FRAME_DIRECT 0x%x  \n", __FUNCTION__, __LINE__, (*(MS_BOOL*)param)));
            if (param != NULL) pAttr->bShowFirstFrameDirect = (*(MS_BOOL*)param);
            break;
        case E_VSYNC_USER_CMD_SET_TRICKPLAY_2X_MODE:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_TRICKPLAY_2X_MODE  0x%x \n", __FUNCTION__, __LINE__, (*(MS_BOOL*)param)));
            if (param != NULL) pAttr->bSmoothTrickMode = (*(MS_BOOL*)param);
            break;
        case E_VSYNC_USER_CMD_FD_MASK_DELAY_COUNT:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_FD_MASK_DELAY_COUNT  \n", __FUNCTION__, __LINE__));
            break;
        case E_VSYNC_USER_CMD_FRC_ONLY_SHOW_TOP_FIELD:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_FRC_ONLY_SHOW_TOP_FIELD  \n", __FUNCTION__, __LINE__));
            break;
        case E_VSYNC_USER_CMD_DISP_ONE_FIELD:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_DISP_ONE_FIELD  \n", __FUNCTION__, __LINE__));
            break;
        case E_VSYNC_USER_CMD_AVC_DISP_IGNORE_CROP:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_AVC_DISP_IGNORE_CROP  \n", __FUNCTION__, __LINE__));
            break;
        case E_VSYNC_USER_CMD_AVSYNC_REPEAT_TH:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_AVSYNC_REPEAT_TH  \n", __FUNCTION__, __LINE__));
            if (param != NULL) pAttr->u32RepeatThreshold = (*(MS_U32*)param);
            break;
        case E_VSYNC_USER_CMD_SET_BLUE_SCREEN:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_BLUE_SCREEN  \n", __FUNCTION__, __LINE__));
            break;
        case E_VSYNC_USER_CMD_SET_FREEZE_DISP:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_FREEZE_DISP 0x%x \n", __FUNCTION__, __LINE__, (*(MS_BOOL*)param)));
            if((param != NULL) && (*(MS_BOOL*)param) == TRUE)
                pAttr->eStatus = E_VSYNC_FREEZE;
            else
                pAttr->eStatus = E_VSYNC_PLAY;
            break;
        case E_VSYNC_USER_CMD_STEP_DISP:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_STEP_DISP  \n", __FUNCTION__, __LINE__));
            pAttr->eStatus = E_VSYNC_STEP_DISP;
            pAttr->bStepDispDone = FALSE;
            break;
        case E_VSYNC_USER_CMD_ENABLE_LAST_FRAME_SHOW:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_ENABLE_LAST_FRAME_SHOW  \n", __FUNCTION__, __LINE__));
            pAttr->bLastFrameShowDone = FALSE;
            pAttr->bCheckFileEnd = TRUE;
            break;
        case E_VSYNC_USER_CMD_PRESET_STC:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_PRESET_STC  0x%x\n", __FUNCTION__, __LINE__, (*(VDEC_EX_STCMode*)param)));
            if (param != NULL) pAttr->eStcMode = (*(VDEC_EX_STCMode*)param);
            break;
        case E_VSYNC_USER_CMD_SET_MULTI_SPEED_ENCODE:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_MULTI_SPEED_ENCODE  0x%x\n", __FUNCTION__, __LINE__, (*(MS_BOOL*)param)));
            if (param != NULL) pAttr->u8ByPassMode = (*(MS_BOOL*)param);
            break;
        case E_VSYNC_USER_CMD_DBG_IDX:
            if (param != NULL) pAttr->stLogIdx = (*(VSYNC_LOG_IDX*)param);
            break;
        case E_VSYNC_USER_CMD_SET_SELF_SEQCHANGE:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_SELF_SEQCHANGE  0x%x\n", __FUNCTION__, __LINE__, (*(MS_BOOL*)param)));
            if (param != NULL) pAttr->bSelf_SeqChange = (*(MS_BOOL*)param);
            break;
        case E_VSYNC_USER_CMD_SUSPEND_DYNAMIC_SCALE:
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SUSPEND_DYNAMIC_SCALE  0x%x\n", __FUNCTION__, __LINE__, (*(MS_BOOL*)param)));
            if (param != NULL) pAttr->bSuspendDS = (*(MS_BOOL*)param);
            break;
        case E_VSYNC_USER_CMD_SET_WIN_INIT:
        {
            _VSYNC_API_MutexLock();
            pAttr->eWinID = ((VDEC_WIN_INFO*)param)->win_id;
            pAttr->u8EnableSetDigitalSignal = ((VDEC_WIN_INFO*)param)->u8EnableSetDigitalSignal;
            pAttr->u8EnableClearDigitalSignal = ((VDEC_WIN_INFO*)param)->u8EnableClearDigitalSignal;
            VSYNC_LOG_CMD(VSYNC_PRINT("[%s] %d  E_VSYNC_USER_CMD_SET_WIN_INIT  0x%x  0x%x  0x%x\n", __FUNCTION__, __LINE__, pAttr->eWinID , pAttr->u8EnableSetDigitalSignal , pAttr->u8EnableClearDigitalSignal));
            _VSYNC_API_MutexUnlock();
        }
        break;
        default:
            break;
    }
    return E_VSYNC_OK;
}
VDEC_EX_Result MApi_VSync_GetControl(int VsynDispId, VSYNC_User_Cmd cmd_id, void* param)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    switch(cmd_id)
    {
        case E_VSYNC_USER_CMD_GET_IsAVSyncOn:
            (*(MS_U32*)param) = pAttr->bAvsync;
            break;
        case E_VSYNC_USER_CMD_GET_IS_REACH_SYNC:
            (*(MS_U32*)param) = pAttr->bAvsyncDone;
            break;
        case E_VSYNC_USER_CMD_GET_DROP_CNT:
            (*(MS_U32*)param) = pAttr->u32DropCnt;
            break;
        case E_VSYNC_USER_CMD_GET_DISP_CNT:
            (*(MS_U32*)param) = pAttr->u32DispCnt;
            break;
        case E_VSYNC_USER_CMD_GET_U64PTS:
            (*(MS_U64*)param) = (MS_U64)(pAttr->u64Pts / MS_90KHZ);
            break;
        case E_VSYNC_USER_CMD_GET_PTS:
            (*(MS_U32*)param) = (MS_U32)((pAttr->u64Pts & 0xFFFFFFFF) / MS_90KHZ);
            break;
        case E_VSYNC_USER_CMD_GET_IS_FRAME_RDY:
            if(pAttr->u32DispCnt > 0)
                (*(MS_U32*)param) = TRUE;
            else
                (*(MS_U32*)param) = FALSE;
            break;
        case E_VSYNC_USER_CMD_GET_NEXT_PTS:
            (*(MS_U32*)param) = (MS_U32)((pAttr->u64NextPts & 0xFFFFFFFF) / MS_90KHZ);
            break;
        case E_VSYNC_USER_CMD_GET_IS_STEP_DISP_DONE:
            (*(MS_U32*)param) = pAttr->bStepDispDone;
            break;
        case E_VSYNC_USER_CMD_IS_DISP_FINISH:
            (*(MS_U32*)param) = pAttr->bLastFrameShowDone;
            break;
        case E_VSYNC_USER_CMD_GET_VIDEO_PTS_STC_DELTA:
            (*(MS_S64*)param) = (MS_S64)pAttr->s64PtsStcDelta;
            break;
        case E_VSYNC_USER_CMD_IS_FRAME_RDY:
            (*(MS_U8*)param) = pAttr->u8FirstFrameRdy;
            break;
        case E_VSYNC_USER_CMD_IS_DISPQ_EMPTY:
            (*(MS_U8*)param) = (pAttr->u8DispFbQRptr == pAttr->u8DispFbQWptr) ? TRUE : FALSE;
            break;
        case E_VSYNC_USER_CMD_GET_DISP_INFO:
            if(pAttr->bDispInfoRdy)
                memcpy(param, &(pAttr->DispinfoOutput), sizeof(VDEC_EX_DispInfo));
            else
                return E_VSYNC_FAIL;
            break;
        case E_VSYNC_USER_CMD_CHECK_DISPINFO_READY:
            (*(MS_U8*)param) = pAttr->bDispInfoRdy;
            break;
        default:
            break;
    }
    return E_VSYNC_OK;
}

VSYNC_Result MApi_VSync_IsInited(int VsynDispId)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    if(pAttr->bInit)
        return E_VSYNC_OK;
    else
        return E_VSYNC_FAIL;
}

VSYNC_Result MApi_VSync_SetCallbackFunc(int VsynDispId,
                                        VsyncDispConnectCb VsyncDispConnect,
                                        VsyncDispDisconnectCb VsyncDispDisconnect,
                                        VsyncDispFlipCb VsyncDispFlip
                                       )
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    pAttr->VsyncDispConnect = VsyncDispConnect;
    pAttr->VsyncDispDisconnect = VsyncDispDisconnect;
    pAttr->VsyncDispFlip = VsyncDispFlip;

    return E_VSYNC_OK;
}

VSYNC_Result MApi_VSync_SetVdecCBFunc(int VsynDispId, VDEC_EX_FireUserCallbackFunc VDEC_EX_FireUserCBFunc)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    pAttr->VsyncFireVdecCallback = VDEC_EX_FireUserCBFunc;
    return E_VSYNC_OK;
}

VSYNC_Result MApi_VSync_SetSpeed(int VsynDispId, VDEC_EX_SpeedType eSpeedType, VDEC_EX_DispSpeed eSpeed)
{
    _VSYNC_SHM_POINTER_CHECK(E_VSYNC_FAIL);
    VSYNC_Attr* pAttr = &(pVsyncContext->_Attr[VsynDispId]);

    VSYNC_LOG_CMD(VSYNC_PRINT("\033[1;45m[%s] %d  eSpeedType = 0x%x  eSpeed = 0x%x  pAttr->bSmoothTrickMode = 0x%x  \033[m\n", __FUNCTION__, __LINE__, eSpeedType, eSpeed, pAttr->bSmoothTrickMode));

    if((eSpeedType == E_VDEC_EX_SPEED_FAST && eSpeed == E_VDEC_EX_DISP_SPEED_2X)  || eSpeedType == E_VDEC_EX_SPEED_SLOW)//smooth fast/slow
    {
        pAttr->u32latch_stc = _VSYNC_GetStc32(VsynDispId);
        pAttr->u32latch_time = MsOS_GetSystemTime();
        pAttr->u32latch_pts =  (MS_U32)pAttr->u64Pts;
    }
    else
    {
        pAttr->u32latch_stc = 0;
        pAttr->u32latch_pts = 0;
        pAttr->u32latch_time = 0;
    }
    pAttr->eSpeedType = eSpeedType;
    pAttr->eSpeed = eSpeed;

    pAttr->u32DbgGetFrameCnt = 0;
    pAttr->u32DbgDispCnt = 0;

    return E_VSYNC_OK;
}

#endif
#endif
