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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drv_scl_os.h
/// @brief  MStar OS Wrapper
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "kernel.h"


#ifndef __DRV_SCL_OS_H__
#define __DRV_SCL_OS_H__

#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
#include "drv_clkgen_cmu.h"
#include "cam_os_wrapper.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"

#define SCLOS_POLLIN             0x0001
#define SCLOS_POLLPRI            0x0002
#define SCLOS_POLLOUT            0x0004
#define SCLOS_POLLERR            0x0008
#define SCLOS_POLLWRNORM         0x0100

#define SCLOS_TASK_MAX           32//(32+120)
#define SCLOS_WORKQUEUE_MAX      8//(8)
#define SCLOS_WORK_MAX           32//(8)
#define SCLOS_TASKLET_MAX        32//(8)
#define SCLOS_MUTEX_MAX          64//(64+240)
#define SCLOS_TSEM_MAX           64//(64+240)

#define SCLOS_SPINLOCK_MAX       8
#define SCLOS_EVENTGROUP_MAX     64*4//(64)
#define SCLOS_TIMER_MAX          (32)
#define SCLOS_CLK_MAX            (E_DRV_SCLOS_CLK_ID_NUM)
#define SCLOS_FLAG_MAX           32//(64)


#define EFAULT                   14

#define SCL_DELAY2FRAMEINDOUBLEBUFFERMode 0
#define SCL_DELAYFRAME (DrvSclOsGetSclFrameDelay())
#define ISZOOMDROPFRAME (DrvSclOsGetHvspDigitalZoomMode())
#define MIU0_BASE 0x20000000
#define MIU0Vir_BASE 0x20000000
#define _Phys2Miu(phys) ((phys&MIU0_BASE) ? (u32)(phys - MIU0_BASE) :(u32)(phys))
#define _Vir2Miu(vir) ((vir&MIU0Vir_BASE) ? (u32)(vir - MIU0Vir_BASE) :(u32)(vir))

#define SCOS_DRIVER_MEMORY_BASE   0x20000000 //RTK_ToDo Vir Base Addr

#define SCLDMA_HANDLER_PRE_FIX          0x12410000
#define SCLHVSP_HANDLER_PRE_FIX         0x12310000
#define HANDLER_PRE_FIX_DEV1            0x00000000
#define HANDLER_PRE_FIX_DEV2            0x00001000
#define HANDLER_PRE_FIX_DEV3            0x00002000
#define HANDLER_PRE_FIX_DEV4            0x00003000
#define SCLVIP_HANDLER_PRE_FIX          0x12330000
#define SCLPNL_HANDLER_PRE_FIX          0x12340000
#define SCLVPE_HANDLER_PRE_FIX          0x12510000
#define SCLM2M_HANDLER_PRE_FIX          0x12610000
#define HANDLER_PRE_FIX_SHIFT            12
#define HANDLER_PRE_MASK         0xFFFF0000
#define HANDLER_DEV_PRE_MASK         0x0000F000

#define SIZE_OF_CMDREG (sizeof(HalUtilityCmdReg_t))
#define MDRV_SCL_CTX_CMDQ_BUFFER_CNT   (256) // 12k for CMDQ buffer ,256 cmd
#define MDRV_SCL_CTX_CMDQ_BUFFER_SIZE   (SIZE_OF_CMDREG * MDRV_SCL_CTX_CMDQ_BUFFER_CNT) // 12k for CMDQ buffer ,256 cmd
#define MDRV_SCL_CTX_WDROPR_BUFFER_SIZE   (SIZE_OF_CMDREG * MDRV_SCL_CTX_CMDQ_BUFFER_CNT) // 2k for CMDQ buffer ,256 cmd
#define MDRV_SCL_CTX_CLIENT_ID_MAX      (E_DRV_SCLOS_DEV_MAX) // SCLDMA * 4, SCLHVSP *4 VIP PNL
extern u8 gu8LevelInst;

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
//time and clock macros
#define TICK_PER_ONE_MS     (1) //Note: confirm Kernel fisrt
#define SCLOS_WAIT_FOREVER   (0xffffff00/TICK_PER_ONE_MS)

#define GFP_KERNEL          0 //RTK_ToDo
#define CONFIG_OF           1 //RTK_ToDo
#define PAGE_ALIGN(x)       x //RTK_ToDo
#define SCLOS_BUG()
//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------
typedef void ( *TaskEntry ) (vm_msg_t * ); ///< Task entry function
typedef void ( *InterruptCb ) (void);        ///< Interrupt callback function
typedef void ( *SignalCb ) (u32 u32Signals);        ///< Signal callback function
typedef void ( *TimerCb ) (u32 u32StTimer, u32 u32TimerID);  ///< Timer callback function  u32StTimer: not used; u32TimerID: Timer ID

typedef  u32  DrvSclOsDmemBusType_t;
#define ssize_t u32

#ifndef true
/// definition for true
#define true                        1
/// definition for false
#define false                       0
#endif

/// Task priority
typedef enum
{
    E_TASK_PRI_SYS      = 0,    ///< System priority task   ( interrupt level driver, e.g. TSP, SMART )
    E_TASK_PRI_HIGHEST  = 4,    ///< Highest priority task  ( background monitor driver, e.g. DVBC, HDMI )
    E_TASK_PRI_HIGH     = 8,    ///< High priority task     ( service task )
    E_TASK_PRI_MEDIUM   = 12,   ///< Medium priority task   ( application task )
    E_TASK_PRI_LOW      = 16,   ///< Low priority task      ( nonbusy application task )
    E_TASK_PRI_LOWEST   = 24,   ///< Lowest priority task   ( idle application task )
} TaskPriority;

/// Suspend type
typedef enum
{
    E_DRV_SCLOS_PRIORITY,            ///< Priority-order suspension
    E_DRV_SCLOS_FIFO,                ///< FIFO-order suspension
} DrvSclOsAttributeType_e;
typedef struct
{
    s32 (*SclOsAlloc)(u8 *pu8Name, u32 size, u64 * phyAddr);
    s32 (*SclOsFree)(u64 u64PhyAddr);
    void * (*SclOsmap)(u64 u64PhyAddr, u32 u32Size , bool bCache);
    void   (*SclOsunmap)(void *pVirtAddr);
    s32 (*SclOsflush_cache)(void *pVirtAddr, u32 u32Size);
}DrvSclOsAllocPhyMem_t;
typedef enum
{
    E_DRV_SCLOS_CMDQEVE_S0_MDW_W_DONE,          //Only for cmdq1&cmdq5
    E_DRV_SCLOS_CMDQEVE_S0_MGW_FIRE,            //Only for cmdq1&cmdq5
    E_DRV_SCLOS_CMDQEVE_S1_MDW_W_DONE,          //Only for cmdq2&cmdq4
    E_DRV_SCLOS_CMDQEVE_S1_MGW_FIRE,            //Only for cmdq2&cmdq4
    E_DRV_SCLOS_CMDQEVE_DMAGEN_TRIGGER0,        //Only for cmdq2&cmdq4
    E_DRV_SCLOS_CMDQEVE_DMAGEN_TRIGGER1,        //Only for cmdq2&cmdq4
    E_DRV_SCLOS_CMDQEVE_BDMA_TRIGGER0,          //Only for cmdq3&cmdq5
    E_DRV_SCLOS_CMDQEVE_BDMA_TRIGGER1,          //Only for cmdq3
    E_DRV_SCLOS_CMDQEVE_IVE_CMDQ_TRIG,          //Only for cmdq3&cmdq5
    E_DRV_SCLOS_CMDQEVE_LDC_CMDQ_TRIG,          //Only for cmdq1&cmdq3
    E_DRV_SCLOS_CMDQEVE_GE_CMDQ_TRIG,           //Only for cmdq1&cmdq3
    E_DRV_SCLOS_CMDQEVE_REG_DUMMY_TRIG,         //Only for cmdq1&cmdq2&cmdq4&cmdq5
    E_DRV_SCLOS_CMDQEVE_CORE1_MHE_TRIG,         //Only for ALL
    E_DRV_SCLOS_CMDQEVE_CORE0_MHE_TRIG,         //Only for ALL
    E_DRV_SCLOS_CMDQEVE_CORE1_MFE_TRIG,         //Only for ALL
    E_DRV_SCLOS_CMDQEVE_CORE0_MFE_TRIG,         //Only for ALL
    E_DRV_SCLOS_CMDQEVE_DIP_TRIG,               //Only for ALL
    E_DRV_SCLOS_CMDQEVE_GOP_TRIG4,              //Only for ALL
    E_DRV_SCLOS_CMDQEVE_GOP_TRIG2,              //Only for ALL
    E_DRV_SCLOS_CMDQEVE_GOP_TRIG013,            //Only for ALL
    E_DRV_SCLOS_CMDQEVE_SC_TRIG2,               //Only for ALL
    E_DRV_SCLOS_CMDQEVE_SC_TRIG013,             //Only for ALL
    E_DRV_SCLOS_CMDQEVE_ISP_TRIG,               //Only for ALL
    E_DRV_SCLOS_CMDQEVE_MAX
} E_DRV_SCLOS_CMDQ_EventId_e;

typedef struct MHAL_CMDQ_CmdqInterface_s     DrvSclOsCmdqInterface_t;
/*
struct DrvSclOsCmdqInterface_s
{
    s32 (*SclOsGetNextMlodRignBufWritePtr)(DrvSclOsCmdqInterface_t *pCmdinf, u64* phyWritePtr);
    s32 (*SclOsUpdateMloadRingBufReadPtr)(DrvSclOsCmdqInterface_t* pCmdinf, u64 phyReadPtr);
    s32 (*SclOsMloadCopyBuf)(DrvSclOsCmdqInterface_t *pCmdinf, void * MloadBuf, u32 u32Size, u16 u16Alignment, u64 *phyRetAddr);
    s32 (*SclOsCheckBufAvailable)(DrvSclOsCmdqInterface_t *pCmdinf, u32 u32CmdqNum);
    s32 (*SclOsWriteDummyRegCmdq)(DrvSclOsCmdqInterface_t *pCmdinf, u16 u16Value);
    s32 (*SclOsReadDummyRegCmdq)(DrvSclOsCmdqInterface_t *pCmdinf, u16* u16RegVal);
    s32 (*SclOsWriteRegCmdqMask)(DrvSclOsCmdqInterface_t *pCmdinf, u32 u32RegAddr, u16 u16Value, u16 u16WriteMask);
    s32 (*SclOsWriteRegCmdq)(DrvSclOsCmdqInterface_t *pCmdinf, u32 u32RegAddr, u16 u16Value);
    s32 (*SclOsCmdqPollRegBits)(DrvSclOsCmdqInterface_t *pCmdinf, u32 u32RegAddr, u16 u16Value,  u16 u16WriteMask, bool bPollEq);
    s32 (*SclOsCmdqAddWaitEventCmd)(DrvSclOsCmdqInterface_t *pCmdinf, E_DRV_SCLOS_CMDQ_EventId_e eEvent);
    s32 (*SclOsCmdqAbortBuffer)(DrvSclOsCmdqInterface_t *pCmdinf);
    s32 (*SclOsCmdqResetEngine)(DrvSclOsCmdqInterface_t *pCmdinf);
    s32 (*SclOsReadStatusCmdq)(DrvSclOsCmdqInterface_t *pCmdinf, u32* u32StatVal);
    s32 (*SclOsKickOffCmdq)(DrvSclOsCmdqInterface_t *pCmdinf);
    s32 (*SclOsIsCmdqEmptyIdle)(DrvSclOsCmdqInterface_t *pCmdinf, bool* bIdleVal);
    void *pCtx;
};
*/
typedef enum
{
    E_DRV_SCLOS_INIT_NONE   = 0x00000000,
    E_DRV_SCLOS_INIT_HVSP_1 = 0x00000001,
    E_DRV_SCLOS_INIT_HVSP_2 = 0x00000002,
    E_DRV_SCLOS_INIT_HVSP_3 = 0x00000004,
    E_DRV_SCLOS_INIT_HVSP_4 = 0x00000008,
    E_DRV_SCLOS_INIT_DMA_1  = 0x00000010,
    E_DRV_SCLOS_INIT_DMA_2  = 0x00000020,
    E_DRV_SCLOS_INIT_DMA_3  = 0x00000040,
    E_DRV_SCLOS_INIT_DMA_4  = 0x00000080,
    E_DRV_SCLOS_INIT_PNL    = 0x00000100,
    E_DRV_SCLOS_INIT_VIP    = 0x00000200,
    E_DRV_SCLOS_INIT_M2M    = 0x00000400,
    E_DRV_SCLOS_INIT_HVSP   = (E_DRV_SCLOS_INIT_HVSP_1 | E_DRV_SCLOS_INIT_HVSP_2 | E_DRV_SCLOS_INIT_HVSP_3 |E_DRV_SCLOS_INIT_HVSP_4),
    E_DRV_SCLOS_INIT_DMA    = (E_DRV_SCLOS_INIT_DMA_1 | E_DRV_SCLOS_INIT_DMA_2 | E_DRV_SCLOS_INIT_DMA_3 | E_DRV_SCLOS_INIT_DMA_4),
    E_DRV_SCLOS_INIT_ALL    = (E_DRV_SCLOS_INIT_HVSP | E_DRV_SCLOS_INIT_DMA  | E_DRV_SCLOS_INIT_VIP | E_DRV_SCLOS_INIT_M2M),
} DrvSclosProbeType_e;
typedef enum
{
    E_DRV_SCLOS_DEV_HVSP_1 = 0,
    E_DRV_SCLOS_DEV_HVSP_2 ,
    E_DRV_SCLOS_DEV_HVSP_3 ,
    E_DRV_SCLOS_DEV_HVSP_4 ,
    E_DRV_SCLOS_DEV_DMA_1  ,
    E_DRV_SCLOS_DEV_DMA_2  ,
    E_DRV_SCLOS_DEV_DMA_3  ,
    E_DRV_SCLOS_DEV_DMA_4  ,
    E_DRV_SCLOS_DEV_PNL    ,
    E_DRV_SCLOS_DEV_VIP    ,
    E_DRV_SCLOS_DEV_M2M    ,
    E_DRV_SCLOS_DEV_MAX    ,
} DrvSclosDevType_e;

/// Message size type
typedef enum
{
    E_DRV_SCLOS_MSG_FIXED_SIZE,           ///< Fixed size message
    E_DRV_SCLOS_MSG_VAR_SIZE,             ///< Variable size message
} DrvSclOsMessageType_e;

/// Event mode
typedef enum
{
    E_DRV_SCLOS_EVENT_MD_AND,                      ///< Specify all of the requested events are require.
    E_DRV_SCLOS_EVENT_MD_OR,                       ///< Specify any of the requested events are require.
    E_DRV_SCLOS_EVENT_MD_AND_CLEAR,                ///< Specify all of the requested events are require. If the request are successful, clear the event.
    E_DRV_SCLOS_EVENT_MD_OR_CLEAR,                 ///< Specify any of the requested events are require. If the request are successful, clear the event.
} DrvSclOsEventWaitMoodeType_e;

typedef struct
{
    u16 		nbFrmBufDvr : 1,
			 	nbFrmBufCam : 1,
			 	nbRealTime  : 1,
				nReserved : 13;
} DrvSclOsGetCap_t;

typedef struct
{
    s32                          iId;
    TaskPriority                 ePriority;
    void                         *pStack;
    u32                          u32StackSize;
    char szName[16];
} DrvSclOsTaskInfo_t, *PDrvSclOsTaskInfo_t;

///////////paul include for RTOS and LINUX
typedef struct
{
    void *pThread;
}DrvSclOsTaskStruct_t;

// CLK
typedef struct
{
    ClkgenTopClk_e eTopClk;
    ClkgenModule_e eModule;
    ClkgenClkReq_e eClkReq;
    ClkgenClkUpdate_e eClk;
    u16 u16Src;
}DrvSclOsClkStruct_t;


typedef enum
{
    E_DRV_SCLOS_CLK_ID_HVSP1,
    E_DRV_SCLOS_CLK_ID_HVSP2,
    E_DRV_SCLOS_CLK_ID_HVSP3,
    E_DRV_SCLOS_CLK_ID_HVSP4,
    E_DRV_SCLOS_CLK_ID_DMA1,
    E_DRV_SCLOS_CLK_ID_DMA2,
    E_DRV_SCLOS_CLK_ID_DMA3,
    E_DRV_SCLOS_CLK_ID_DMA4,
    E_DRV_SCLOS_CLK_ID_NUM,
}DrvSclOsClkIdType_e;

typedef struct
{
    void *ptIdclk;
    void *ptFclk1;
    void *ptFclk2;
    void *ptOdclk;
}DrvSclOsClkConfig_t;
typedef struct
{
    s32 s32EventId;
    u32 u32WaitEvent;
    u32 u32Timeout;
}DrvSclOsPollWaitConfig_t;

typedef enum
{
    E_DRV_SCLOS_VIPSETRUle_DEFAULT  = 0,
    E_DRV_SCLOS_VIPSETRUle_CMDQACT  = 0x1,
    E_DRV_SCLOS_VIPSETRUle_CMDQCHECK  = 0x2,
    E_DRV_SCLOS_VIPSETRUle_CMDQALL  = 0x4,
    E_DRV_SCLOS_VIPSETRUle_CMDQALLONLYSRAMCheck  = 0x8,
    E_DRV_SCLOS_VIPSETRUle_CMDQALLCheck  = 0x10,
} DrvSclOsVipSetRuleType_e;
typedef enum
{
    E_DRV_SCLOS_FBALLOCED_NON  = 0,
    E_DRV_SCLOS_FBALLOCED_YCM  = 0x1,
}DrvSclOsFBAlloced_e;

typedef enum
{
    E_DRV_SCLOS_AccessReg_CPU  = 0, //I3 mode  ,only critical CMDQ
    E_DRV_SCLOS_AccessReg_CMDQ  = 0x1,//All CMDQ
    E_DRV_SCLOS_AccessReg_MLOAD  = 0x2,
    E_DRV_SCLOS_AccessReg_CMDQM2M  = 0x4,//All CMDQ
} DrvSclOsAccessRegType_e;
typedef enum
{
    E_DRV_SCLOS_SCLIRQ_SC0  = 0,
    E_DRV_SCLOS_SCLIRQ_SC1  ,
    E_DRV_SCLOS_SCLIRQ_SC2  ,
    E_DRV_SCLOS_SCLIRQ_MAX  ,
} E_DRV_SCLOS_SCLIRQ_TYPE;
typedef enum
{
    E_DRV_SCLOS_CMDQIRQ_CMDQ0  = 0,
    E_DRV_SCLOS_CMDQIRQ_CMDQ1  ,
    E_DRV_SCLOS_CMDQIRQ_CMDQ2  ,
    E_DRV_SCLOS_CMDQIRQ_MAX  ,
} E_DRV_SCLOS_CMDQIRQ_TYPE;
typedef struct
{
    u8 gu8SclFrameDelay;
    u8 gu8FrameBufferNum;
    DrvSclOsFBAlloced_e enFbAlloced;
}MDrvSclCtxSclOsGlobalSet_t;
#define VIPDEFAULTSETRULE E_DRV_SCLOS_VIPSETRUle_CMDQALL
//for OSD bug, need to set same clk freq with fclk1


#define OSDinverseBug 1
#define CLKDynamic    0
#define USE_Utility   1
#define VIR_RIUBASE 0x1F000000
#define I2_DVR 1
#define I2_DVR_TEST 0 // for probe test
#define I2_DVR_TEST_ISR 0 //for test whether open isr
#define I2_DVR_SIMPLE 0  //for allocate memory simple/diffcult
//-------------------------------------------------------------------------------------------------
// Extern Functions
//-------------------------------------------------------------------------------------------------
//
// Init
//
bool DrvSclOsInit (void);
void DrvSclOsExit (void);

//
// Memory management
//



//
// Task
//
s32  DrvSclOsCreateTask (TaskEntry pTaskEntry, u32 u32TaskEntryData, bool bAutoStart, char *pTaskName);
DrvSclOsTaskStruct_t DrvSclOsGetTaskinfo(s32 s32Id);
bool DrvSclOsDeleteTask (s32 s32TaskId);
bool DrvSclOsSetTaskWork(s32 s32TaskId);
void DrvSclOsTaskWait(u32 u32Waitflag);
void DrvSclOsDelayTask (u32 u32Ms);
void DrvSclOsDelayTaskUs (u32 u32Us);
void DrvSclOsSetProbeInformation(DrvSclosProbeType_e enProbe);
u32  DrvSclOsGetProbeInformation(DrvSclosProbeType_e enProbe);
void DrvSclOsClearProbeInformation(DrvSclosProbeType_e enProbe);

//-------------------------------------------------------------------------------------------------
/// Get thread ID of current thread/process in OS
/// @return : current thread ID
//-------------------------------------------------------------------------------------------------
//
// Mutex
//
#define SCLOS_PROCESS_PRIVATE    0x00000000
#define SCLOS_PROCESS_SHARED     0x00000001
#define SCLOS_MUTEX_NAME_LENGTH   16
s32 DrvSclOsCreateMutex ( DrvSclOsAttributeType_e eAttribute, char *pMutexName, u32 u32Flag);
s32 DrvSclOsCreateSpinlock ( DrvSclOsAttributeType_e eAttribute, char *pMutexName1, u32 u32Flag);

bool DrvSclOsDeleteMutex (s32 s32MutexId);
bool DrvSclOsDeleteSpinlock (s32 s32MutexId);
bool DrvSclOsObtainMutexIrq(s32 s32MutexId);
bool DrvSclOsObtainMutex (s32 s32MutexId, u32 u32WaitMs);
bool DrvSclOsReleaseMutexIrq (s32 s32MutexId);
char * DrvSclOsCheckMutex(char *str,char *end);
bool DrvSclOsReleaseMutex (s32 s32MutexId);
bool DrvSclOsReleaseMutexAll (void);
bool DrvSclOsInfoMutex (s32 s32MutexId, DrvSclOsAttributeType_e *peAttribute, char *pMutexName);

s32  DrvSclOsTsemInit(u32 nVal);
bool DrvSclOsReleaseTsem(s32 s32Id);
bool DrvSclOsObtainTsem(s32 s32Id);
s32  DrvSclOsTsemDeinit(s32 s32Id);

//
// Semaphore
//
s32 DrvSclOsCreateSemaphore (u32 u32InitCnt, DrvSclOsAttributeType_e eAttribute, char *pName);
bool DrvSclOsDeleteSemaphore (s32 s32SemaphoreId);
bool DrvSclOsObtainSemaphore (s32 s32SemaphoreId, u32 u32WaitMs);
bool DrvSclOsReleaseSemaphore (s32 s32SemaphoreId);
bool DrvSclOsInfoSemaphore (s32 s32SemaphoreId, u32 *pu32Cnt, DrvSclOsAttributeType_e *peAttribute, char *pSemaphoreName);


//
// Event management
//
s32 DrvSclOsCreateEventGroup (char *pName);
bool DrvSclOsCreateEventGroupRing (u8 u8Id);
bool DrvSclOsDeleteEventGroup (s32 s32EventGroupId);
bool DrvSclOsDeleteEventGroupRing (s32 s32EventGroupId);
bool DrvSclOsSetEventIrq (s32 s32EventGroupId, u32 u32EventFlag);
bool DrvSclOsSetEvent (s32 s32EventGroupId, u32 u32EventFlag);
bool DrvSclOsSetEventRing (s32 s32EventGroupId);
u32 DrvSclOsGetEvent(s32 s32EventGroupId);
u32 DrvSclOsGetandClearEventRing(u32 u32EventGroupId);
bool DrvSclOsClearEventIRQ (s32 s32EventGroupId, u32 u32EventFlag);
bool DrvSclOsClearEvent (s32 s32EventGroupId, u32 u32EventFlag);
bool DrvSclOsWaitEvent (s32  s32EventGroupId,  u32  u32WaitEventFlag, u32  *pu32RetrievedEventFlag, DrvSclOsEventWaitMoodeType_e eWaitMode, u32  u32WaitMs);
//wait_queue_head_t* DrvSclOsGetEventQueue (s32 s32EventGroupId); RTK_ToDo

//
// Timer management
//
s32   DrvSclOsCreateTimer (TimerCb pTimerCb, u32 u32FirstTimeMs, u32 u32PeriodTimeMs, bool bStartTimer, char *pName);
bool  DrvSclOsDeleteTimer (s32 s32TimerId);
bool  DrvSclOsStartTimer (s32 s32TimerId) ;
bool  DrvSclOsStopTimer (s32 s32TimerId);
bool  DrvSclOsResetTimer (s32 s32TimerId, u32 u32FirstTimeMs, u32 u32PeriodTimeMs, bool bStartTimer);
void* DrvSclOsMemalloc(u32 size, u32 flags);
bool  DrvSclOsFlushWorkQueue(bool bTask, s32 s32TaskId);
bool  DrvSclOsGetQueueExist(s32 s32QueueId);
bool  DrvSclOsQueueWork(bool bTask, s32 s32TaskId, s32 s32QueueId, u32 u32WaitMs);
bool  DrvSclOsQueueWait(s32 s32QueueId);
s32   DrvSclOsCreateWorkQueueTask(char *pTaskName);
bool  DrvSclOsestroyWorkQueueTask(s32 s32Id);
s32   DrvSclOsCreateWorkQueueEvent(void * pTaskEntry);
bool  DrvSclOsTaskletWork(s32 s32TaskId);
bool  DrvSclOsDisableTasklet (s32 s32Id);
bool  DrvSclOsEnableTasklet (s32 s32Id);
bool  DrvSclOsDestroyTasklet(s32 s32Id);
s32   DrvSclOsCreateTasklet(void * pTaskEntry,u32 u32data);
void  DrvSclOsMemFree(void *pVirAddr);
void* DrvSclOsVirMemalloc(u32 size);
void  DrvSclOsVirMemFree(void *pVirAddr);
void* DrvSclOsMemcpy(void *pstCfg,const void *pstInformCfg,u32 size);
void* DrvSclOsMemset(void *pstCfg,int val,u32 size);
void  DrvSclOsStrcat(char *pstCfg,char *name);

bool  DrvSclOsClkSetConfig(DrvSclOsClkIdType_e enClkId, DrvSclOsClkConfig_t stClk);
DrvSclOsClkConfig_t *DrvSclOsClkGetConfig(DrvSclOsClkIdType_e enClkId);
u32   DrvSclOsClkGetEnableCount(DrvSclOsClkStruct_t * clk);
DrvSclOsClkStruct_t * DrvSclOsClkGetParentByIndex(DrvSclOsClkStruct_t * clk,u8 index);
u32   DrvSclOsClkSetParent(DrvSclOsClkStruct_t *clk, DrvSclOsClkStruct_t *parent);
u32   DrvSclOsClkPrepareEnable(DrvSclOsClkStruct_t *clk);
void  DrvSclOsClkDisableUnprepare(DrvSclOsClkStruct_t *clk);
u32   DrvSclOsClkGetRate(DrvSclOsClkStruct_t *clk);

u32   DrvSclOsCopyFromUser(void *to, const void *from, u32 n);
u32   DrvSclOsCopyToUser(void *to, const void *from, u32 n);
void  DrvSclOsWaitForCpuWriteToDMem(void);
u8    DrvSclOsGetSclFrameDelay(void);
void DrvSclOsSetHvspDigitalZoomMode(bool bDrop);
bool DrvSclOsGetHvspDigitalZoomMode(void);
bool DrvSclOsGetClkForceMode(void);
void DrvSclOsSetClkForceMode(bool bEn);
void  DrvSclOsSetSclFrameDelay(u8 u8delay);
void DrvSclOsSetSclFrameBufferNum(u8 u8FrameBufferReadyNum);
u8 DrvSclOsGetSclFrameBufferNum(void);
DrvSclOsFBAlloced_e DrvSclOsGetSclFrameBufferAlloced(void);
void DrvSclOsSetSclFrameBufferAlloced(DrvSclOsFBAlloced_e enType);
DrvSclOsAccessRegType_e DrvSclOsGetAccessRegMode(void);
void DrvSclOsSetAccessRegMode(DrvSclOsAccessRegType_e enAccMode);
int DrvSclOsSetSclIrqIDFormSys(E_DRV_SCLOS_SCLIRQ_TYPE enType);
int DrvSclOsSetCmdqIrqIDFormSys(E_DRV_SCLOS_CMDQIRQ_TYPE enType);
void * DrvSclOsClkGetClk(u32 idx);

//
// System time
//
u32 DrvSclOsGetSystemTime (void);
bool DrvSclOsSetPollWait(DrvSclOsPollWaitConfig_t *stPollWait);
u32 DrvSclOsGetSystemTimeStamp (void);
u32 DrvSclOsTimerDiffTimeFromNow(u32 u32TaskTimer); ///[OBSOLETE]
u32 DrvSclOsTimerDiffTime(u32 u32Timer, u32 u32TaskTimer); ///[OBSOLETE]
//
// Interrupt management
//
bool DrvSclOsAttachInterrupt (u32 eIntNum, InterruptCb pIntCb,unsigned long flags,const char *name);
bool DrvSclOsDetachInterrupt (u32 eIntNum);
bool DrvSclOsEnableInterrupt (u32 eIntNum);
bool DrvSclOsDisableInterrupt (u32 eIntNum);
bool DrvSclOsInInterrupt (void);
u32  DrvSclOsDisableAllInterrupts(void);
bool DrvSclOsRestoreAllInterrupts(void);
bool DrvSclOsEnableAllInterrupts(void);
void* DrvSclOsGetEventQueue (s32 *s32EventGroupId);

// direct memory
u32 DrvSclOsPa2Miu(DrvSclOsDmemBusType_t addr);
DrvSclOsDmemBusType_t DrvSclOsMiu2Pa(DrvSclOsDmemBusType_t addr);
void DrvSclOsDirectMemFlush(unsigned long u32Addr, unsigned long u32Size);
void* DrvSclOsDirectMemAlloc(const char* name, u32 size, DrvSclOsDmemBusType_t *addr);
void DrvSclOsDirectMemFree(const char* name, u32 size, void *virt, DrvSclOsDmemBusType_t addr);
void DrvSclOsSetDbgTaskWork(void);
DrvSclosDevType_e MDrvSclOsGetDevByHandler(s32 s32Handler);
void DrvSclOsKeepAllocFunction(DrvSclOsAllocPhyMem_t *pstAlloc);
void DrvSclOsKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq);

int DrvSclOsGetIrqIDCMDQ(E_DRV_SCLOS_CMDQIRQ_TYPE enType);
int DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_TYPE enType);

DrvSclOsAccessRegType_e DrvSclOsGetAccessRegMode(void);
void DrvSclOsSetAccessRegMode(DrvSclOsAccessRegType_e enAccMode);
void DrvSclOsMsSleep(u32 nMsec);
void DrvSclOsGetCap(DrvSclOsGetCap_t *pstCaps);
//
// OS Dependent Macro
//

// Worldwide thread safe macro
// Usage:
//     s32 os_X_MutexID;
//     os_X_MutexID = OS_CREATE_MUTEX(_M_);
//     if (os_X_MutexID < 0) {  return FALSE; }
//     if (OS_OBTAIN_MUTEX(os_X_MutexID, 1000) == FALSE) { return FALSE; }
//     ...
//     OS_RELEASE_MUTEX(os_X_MutexID);
//     return X;
//

#define OS_CREATE_MUTEX(_M_)        DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, "OS_"#_M_"_Mutex", SCLOS_PROCESS_SHARED)
#define OS_OBTAIN_MUTEX(_mx, _tm)   DrvSclOsObtainMutex(_mx, _tm)
#define OS_RELEASE_MUTEX(_mx)       DrvSclOsReleaseMutex(_mx)
#define OS_DELETE_MUTEX(_mx)        DrvSclOsDeleteMutex(_mx)
#define OS_DELAY_TASK(_msec)        DrvSclOsDelayTask(_msec)
#define OS_SYSTEM_TIME()            DrvSclOsGetSystemTime()
#define OS_ENTER_CRITICAL()         DrvSclOsDisableAllInterrupts();
#define OS_EXIT_CRITICAL()          DrvSclOsEnableAllInterrupts();

//-------------------------------------------------------------------------------------------------
// Virutal/Physial address operation
//-------------------------------------------------------------------------------------------------
u32 DrvSclOsVa2Pa(u32 addr);

#define OS_VA2PA(_addr_)                (u32)DrvSclOsVa2Pa((_addr_))

//-------------------------------------------------------------------------------------------------
// Debug message
//-------------------------------------------------------------------------------------------------
#define sclprintf                       UartSendTrace
#define DrvSclOsScnprintf(buf, size, _fmt, _args...)        UartSendTrace(_fmt,##_args)
#define PAGE_SIZE 512
//#define printf printk



#ifdef __cplusplus
}
#endif

#endif // __DRV_SCL_OS_H__
