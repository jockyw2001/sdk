// $Change: 628136 $
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#define  _MDRV_PQ_C_

#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"
#include "hal_scl_reg.h"

#include "hal_scl_pq_color_reg.h"
#include "drv_scl_pq_define.h"
#include "drv_scl_pq_declare.h"
#include "drv_scl_pq.h"
#include "drv_scl_pq_data_types.h"
#include "hal_scl_pq.h"

#include "drv_scl_pq_quality_mode.c"

#ifndef UNUSED //to avoid compile warnings...
#define UNUSED(var) (void)((var) = (var))
#endif

#define PQTAB_DBG(x) do { if( _u16PQDbgSwitch & PQ_DBG_PQTAB) x; } while(0);
#define SRULE_DBG(x) do { if( _u16PQDbgSwitch & PQ_DBG_SRULE) x; } while(0);
#define CSCRULE_DBG(x) do { if( _u16PQDbgSwitch & PQ_DBG_CSCRULE) x; } while(0);
#define PQGRULE_DBG(x) do { if( _u16PQDbgSwitch & PQ_DBG_GRULE) x; } while(0);
#define PQBW_DBG(x) do { if( _u16PQDbgSwitch & PQ_DBG_BW) x; } while(0);
#define PQMADi_DBG(x)   do { if( _u16PQDbgSwitch & PQ_DBG_MADI) x; } while(0);
#define PQINFO_DBG(x) do { if( _u16PQDbgSwitch & PQ_DBG_INFO) x; } while(0);
#define PQIOCTL_DBG(x)  do { if( _u16PQDbgSwitch & PQ_DBG_IOCTL) x; } while(0);
#define PQP2P_DBG(x)  do { if( _u16PQDbgSwitch & PQ_DBG_P2P) x; } while(0);
#define PQCOLOR_DBG(x) do { if( _u16PQDbgSwitch & PQ_DBG_COLOR) x; } while(0);


u32 Scl_PQ_RIU_BASE;

static u16 _u16SclPQSrcType[PQ_MAX_WINDOW];
u16 _u16SclPQSrcType_DBK_Detect[PQ_MAX_WINDOW];     //For Auto_DBK SW driver used

static PQ_INPUT_SOURCE_TYPE _enInputSourceType[PQ_MAX_WINDOW];


//whether current status is Point-to-point mode

static u16 _u16PQDbgSwitch = 0;



typedef struct
{
    // input timing
    u16 u16input_hsize;
    u16 u16input_vtotal;
    u16 u16input_vfreq;

    // output timing
    u16 u16output_hsize;
    u16 u16output_vtotal;
    u16 u16output_vfreq;

    // memory format
    bool bFBL;
    bool bMemFmt422;
    bool bInterlace;
    bool b4RMode;
    u8 u8BitsPerPixel;

    // osd
    bool bOSD_On;
    u16 u16OSD_hsize;
    u8 u8OSD_BitsPerPixel;

    // dram
    u32 u32MemBW;
    bool bSC_MIUSel;
    bool bGOP_MIUSel;
    bool bDDR2;
    u32  u32DDRFreq;
    u8   u8BusWidth;
    u32 u32Miu0MemSize;
    u32 u32Miu1MemSize;

    // PVR
    u32 u32PVR_BW;

    //AEON
    u32 u32MISC_BW;
} BW_INFO_t;



static MS_PQ_Status _status         = {FALSE, FALSE, };


//static BW_INFO_t bw_info;



s32 _Scl_PQ_Mutex = -1;

#ifdef SCLOS_TYPE_LINUX
pthread_mutex_t _PQ_MLoad_Mutex;
#endif

//////////////////////////////////////////////////////////////////
// PQ Patch
bool _bOSD_On = FALSE;

#if PQ_ENABLE_UNUSED_FUNC
static bool bSetFrameCount = TRUE;
#endif



#ifdef SCLOS_TYPE_LINUX
#define PQ_MLOAD_ENTRY() PTH_PQ_RET_CHK(pthread_mutex_lock(&_PQ_MLoad_Mutex))
#define PQ_MLOAD_RETURN() PTH_PQ_RET_CHK(pthread_mutex_unlock(&_PQ_MLoad_Mutex))
#else
#define PQ_MLOAD_ENTRY()
#define PQ_MLOAD_RETURN()
#endif

//////////////////////////////////////////////////////////////////
// OS related
//
//
#define XC_PQ_WAIT_MUTEX              (TICK_PER_ONE_MS * 50)              // 50 ms

/*
// Mutex & Lock
static s32 _s32XC_PQMutex = -1;


#define XC_PQ_ENTRY()                 {   if (_s32XC_PQMutex < 0) { return E_XC_ACE_FAIL; }        \
                                        if(OS_OBTAIN_MUTEX(_s32XC_PQMutex, XC_PQ_WAIT_MUTEX) == FALSE) return E_XC_ACE_OBTAIN_MUTEX_FAIL;  }

#define XC_PQ_RETURN(_ret)            {   OS_RELEASE_MUTEX(_s32XC_PQMutex); return _ret; }
*/

//////////////////////////////////////////////////////////////////
bool MDrv_Scl_PQ_Exit(void)
{
    if(_Scl_PQ_Mutex != -1)
    {
        DrvSclOsDeleteMutex(_Scl_PQ_Mutex);
        _Scl_PQ_Mutex = -1;
    }
    return true;
}

bool MDrv_Scl_PQ_Init(MS_PQ_Init_Info *pstPQInitInfo)
{
    PQTABLE_INFO PQTableInfo;
    bool bSkipCommTable = TRUE;
    if(_Scl_PQ_Mutex != -1)
    {
        sclprintf("%s, already init\n", __FUNCTION__);
        return TRUE;
    }
    _Scl_PQ_Mutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, "_Scl_PQ_Mutex", SCLOS_PROCESS_SHARED);
    if(_Scl_PQ_Mutex == -1)
    {
        (sclprintf("[MAPI PQ][%06d] create mutex fail\r\n", __LINE__));
        return FALSE;
    }
    DrvSclOsMemset(&PQTableInfo, 0, sizeof(PQTableInfo));
    _status.bIsInitialized = TRUE;
    _status.bIsRunning     = TRUE;
    MDrv_Scl_PQ_Set_DisplayType_Main(pstPQInitInfo->u16PnlWidth, PQ_DISPLAY_ONE, bSkipCommTable);
    return TRUE;
}

void MDrv_Scl_PQ_DesideSrcType(PQ_WIN eWindow, PQ_INPUT_SOURCE_TYPE enInputSourceType)
{
    _enInputSourceType[eWindow] = enInputSourceType;
    _u16SclPQSrcType[eWindow] = QM_InputSourceToIndex(eWindow, enInputSourceType);
    _u16SclPQSrcType_DBK_Detect[eWindow] = _u16SclPQSrcType[eWindow];                     //For Auto_DBK SW driver used
    PQTAB_DBG(sclprintf("[PQ_DesideSrcType] window=%u, enInputSrcType=%u, SrcType=%u\r\n",
                     eWindow, enInputSourceType, _u16SclPQSrcType[eWindow]));
}

bool MDrv_Scl_PQ_Set_DisplayType_Main(u16 u16DisplayWidth, PQ_DISPLAY_TYPE enDisplaType, bool bSkipCommTable)
{
    PQTABLE_INFO PQTableInfo;
    PQTABLE_INFO PQTableInfoEx;
    DrvSclOsMemset(&PQTableInfo, 0, sizeof(PQTABLE_INFO));
    DrvSclOsMemset(&PQTableInfoEx, 0, sizeof(PQTABLE_INFO));
    switch(enDisplaType)
    {
        default:
        case PQ_DISPLAY_ONE:
            PQTableInfo.pQuality_Map_Aray = (void*)((u16DisplayWidth == 1366)?QMAP_1920_Main: QMAP_1920_Main);
             break;
    }
    // table config parameter
    PQTableInfo.eWin = PQ_MAIN_WINDOW;
    PQTableInfo.u8PQ_InputType_Num = QM_INPUTTYPE_NUM_Main;
    PQTableInfo.u8PQ_IP_Num = PQ_IP_NUM_Main;
    PQTableInfo.pIPTAB_Info = (void*)Scl_PQ_IPTAB_INFO_Main;
    PQTableInfo.pSkipRuleIP = (void*)MST_SkipRule_IP_Main;
    PQTableInfo.u8PQ_XRule_IP_Num[E_XRULE_HSP] = PQ_HSPRule_IP_NUM_Main;
    PQTableInfo.u8PQ_XRule_IP_Num[E_XRULE_VSP] = PQ_VSPRule_IP_NUM_Main;
    PQTableInfo.pXRule_IP_Index[E_XRULE_HSP] = (void*)NULL;
    PQTableInfo.pXRule_IP_Index[E_XRULE_VSP] = (void*)NULL;
    PQTableInfo.pXRule_Array[E_XRULE_HSP] = (void*)NULL;
    PQTableInfo.pXRule_Array[E_XRULE_VSP] = (void*)NULL;
    MDrv_Scl_PQ_AddTable_(MAIN, &PQTableInfo);
    return TRUE;
}

void MDRv_Scl_PQ_Set_CmdqCfg(PQ_WIN eWindow, bool bEnCMDQ,bool bFire)
{
    PQ_CMDQ_CONFIG CmdqCfg;
    PQ_DBG(sclprintf("%s %d, Win:%d, CMDQEn:%d\n", __FUNCTION__, __LINE__, eWindow, bEnCMDQ));
    if(eWindow == PQ_MAIN_WINDOW)
    {
        CmdqCfg.bEnFmCnt = bEnCMDQ;
        CmdqCfg.bfire = bFire;
        MDrv_Scl_PQ_Set_CmdqCfg_(MAIN, CmdqCfg);
    }
    else
    {
        SCL_ASSERT(0);
    }

}
void MDrv_Scl_PQ_init_RIU(u32 riu_addr)
{
    Scl_PQ_RIU_BASE = riu_addr;
}
u16 MDrv_Scl_PQ_GetIPRegCount(u16 u16PQIPIdx)
{
    return MDrv_Scl_PQ_GetIPRegCount_(MAIN,_u16SclPQSrcType[0], (u8)u16PQIPIdx);
}
void MDrv_Scl_PQ_LoadSettingByData(PQ_WIN eWindow, u8 u8PQIPIdx, u8 *pData, u16 u16DataSize)
{
    PQ_DATA_INFO DataInfo;
    PQ_DBG(sclprintf("%s %d, Win:%d, IPIdx:%d, Size%d  \n", __FUNCTION__, __LINE__, eWindow, u8PQIPIdx, u16DataSize));
    if(eWindow == PQ_MAIN_WINDOW)
    {
        DataInfo.pBuf = pData;
        DataInfo.u16BufSize = u16DataSize;
        MDrv_Scl_PQ_LoadTableByData_(MAIN, _u16SclPQSrcType[eWindow], u8PQIPIdx, &DataInfo);
    }
    else
    {
        SCL_ASSERT(0);
    }

}


void MDrv_Scl_PQ_LoadSettings(PQ_WIN eWindow)
{
    if(eWindow == PQ_MAIN_WINDOW)
    {
        PQTAB_DBG(sclprintf("MAIN table\r\n"));
        MDrv_Scl_PQ_LoadTableBySrcType_(MAIN, _u16SclPQSrcType[eWindow], PQ_IP_ALL);
        PQTAB_DBG(sclprintf("...done\r\n"));
    }
    else
    {
        SCL_ASSERT(0);
    }
}
