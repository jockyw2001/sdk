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
/// file    MsOS.c
/// @brief  MStar OS Wrapper
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined (MSOS_TYPE_ATOMTHREADS)

// Tsaihua has to move
//     OSCPUSaveSR --> MSCPU_OSCPUSaveSR
//     OSCPURestoreSR --> MSCPU_OSCPURestoreSR
//     OSCPUEnableSR --> MSCPU_OSCPUEnableSR
//      from utopia/os/ucos2/mips_ucos/common/src/os_cpu_a.S
extern unsigned int MSCPU_OSCPUSaveSR(void);
extern void MSCPU_OSCPURestoreSR(unsigned int cpu_sr);
extern void MSCPU_OSCPUEnableSR(void);

//-------------------------------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------------------------------
// mstar header
#include "sysdefs.h"
#include "dlmalloc.h"
#include "string.h"
#include "MsCommon.h"
#include "MsOS.h"
#include "asmCPU.h"
#include "regCHIP.h"
#include "halCHIP.h"
#include "halMMIO.h"
#include "MsVersion.h"
#include "mem.h"
// atomthreads kernel header
#include "atom.h"
#include "atommutex.h"
#include "atomqueue.h"
#include "atomsem.h"
#include "atomtimer.h"
// atomthreads port header
#include "atomport.h"

//-------------------------------------------------------------------------------------------------
// Local Defines
//------------------------------------------------------------------------------------------------
#define MAX_TASK_NAME_LENGTH       (50)
//#define MAX_MUTEX_NAME_LENGTH    (50) In MsOS.h
#define MAX_SEMAPHORE_NAME_LENGTH  (50)
#define MAX_EVENTGROUP_NAME_LENGTH (50)
#define MAX_TIMER_NAME_LENGTH      (50)

#define MSOS_DEFAULT_STACK_SIZE    (4096)

// Switch tasks every 1 ms.
#define TASK_TIME_SLICE             (TICK_PER_ONE_MS)

// Combine 3-B prefix with s32ID = MSOS_ID_PREFIX | s32Id
//  to avoid the kernel object being used before initialzed.
#define MSOS_ID_PREFIX              0x76540000
#define MSOS_ID_PREFIX_MASK         0xFFFF0000
#define MSOS_ID_MASK                0x0000FFFF //~MSOS_ID_PREFIX_MASK

#if (defined(CHIP_T3) || defined(CHIP_T4) || defined(CHIP_T7) || defined(CHIP_T9) || defined(CHIP_U4) || defined(CHIP_T13) || defined(CHIP_A1) || defined(CHIP_A6) || defined(CHIP_A7) || defined(CHIP_AMETHYST)) || defined(CHIP_E8)
#define MIU1_CACHEABLE_START        0xC0000000
#define MIU1_CACHEABLE_END          0xD0000000
#define MIU1_UNCACHEABLE_START      0xD0000000
#define MIU1_UNCACHEABLE_END        0xE0000000
#endif

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------------------
MS_BOOL bInitTaskTable = FALSE;
MS_BOOL* pUserDefineTask = NULL;

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------
//
// Variable-size Memory pool
//
typedef struct
{
    MS_BOOL                         bUsed;
    cyg_handle_t                    stMemoryPool;
    cyg_mempool_var                 stMemoryPoolInfo;
} MsOS_MemoryPool_Info;
static MsOS_MemoryPool_Info         _MsOS_MemoryPool_Info[MSOS_MEMPOOL_MAX];
static ATOM_MUTEX                   _MsOS_MemoryPool_Mutex;

//
// Fixed-size Memory pool
//
typedef struct
{
    MS_BOOL                         bUsed;
} MsOS_FixSizeMemoryPool_Info;
static MsOS_FixSizeMemoryPool_Info  _MsOS_FixSizeMemoryPool_Info[MSOS_FIXSIZE_MEMPOOL_MAX];
static ATOM_MUTEX                   _MsOS_FixSizeMemoryPool_Mutex;

//
// Task Management
//
typedef struct
{
    MS_BOOL                         bUsed;
    ATOM_TCB                        stTcb;
    char                            pName[MAX_TASK_NAME_LENGTH];
} MsOS_Task_Info;
static MsOS_Task_Info               _MsOS_Task_Info[MSOS_TASK_MAX];
static ATOM_MUTEX                   _MsOS_Task_Mutex;

//
// Mutex
//
typedef struct
{
    MS_BOOL                         bUsed;
    ATOM_MUTEX                      stMutex;
    char                            pName[MAX_MUTEX_NAME_LENGTH];
} MsOS_Mutex_Info;
static MsOS_Mutex_Info              _MsOS_Mutex_Info[MSOS_MUTEX_MAX];
static ATOM_MUTEX                   _MsOS_Mutex_Mutex;

//
// Semaphore
//
typedef struct
{
    MS_BOOL                         bUsed;
    ATOM_SEM                        stSemaphore;
    MS_U32                          u32InitCnt;
    char                            pName[MAX_SEMAPHORE_NAME_LENGTH];
} MsOS_Semaphore_Info;
static MsOS_Semaphore_Info          _MsOS_Semaphore_Info[MSOS_SEMAPHORE_MAX];
static ATOM_MUTEX                   _MsOS_Semaphore_Mutex;

//
// Event Group
//
typedef struct
{
    MS_BOOL                         bUsed;
    MS_U32                          u32EventGroup;
    ATOM_MUTEX                      stMutexEvent; // performance
    char                            pName[MAX_EVENTGROUP_NAME_LENGTH]; // only set?
} MsOS_EventGroup_Info;
static MsOS_EventGroup_Info         _MsOS_EventGroup_Info[MSOS_EVENTGROUP_MAX];
static ATOM_MUTEX                   _MsOS_EventGroup_Mutex;

//
// Queue
//
typedef struct
{
    MS_BOOL                         bUsed;
    ATOM_QUEUE                      stQueue;
} MsOS_Queue_Info;

static MsOS_Queue_Info              _MsOS_Queue_Info[MSOS_QUEUE_MAX];
static ATOM_MUTEX                   _MsOS_Queue_Mutex;

//
// Timer
//
typedef struct
{
    MS_BOOL                         bUsed;
    ATOM_TIMER                      stTimer;
    MS_U32                          u32Stop; // =0:start, >0:stop
    MS_U32                          u32Period; // =0:once, >0:period
    char                            pName[MAX_TIMER_NAME_LENGTH]; // only set?
} MsOS_Timer_Info;

static MsOS_Timer_Info              _MsOS_Timer_Info[MSOS_TIMER_MAX];
static ATOM_MUTEX                   _MsOS_Timer_Mutex;
// In order to implement period timer
static MS_BOOL bTimerTask = FALSE;
static ATOM_TCB stTimerTcb = {0};
static MS_U8 u8TimeTaskStack[MSOS_DEFAULT_STACK_SIZE];

//
// Interrupt
//
static ATOM_MUTEX                   _MsOS_HISR_Mutex;

//move to mscommon.h
//cached/unchched segment
#define KSEG0_BASE              ((void *)0x80000000)
#define KSEG1_BASE              ((void *)0xa0000000)
#define KSEG0_SIZE              0x20000000
#define KSEG1_SIZE              0x20000000

//cached addr <-> unchched addr
#define KSEG02KSEG1(addr)       ((void *)((MS_U32)(addr)|0x20000000))  //cached -> unchched
#define KSEG12KSEG0(addr)       ((void *)((MS_U32)(addr)&~0x20000000)) //unchched -> cached

//virtual addr <-> physical addr
#define VA2PA(addr)             ((void *)(((MS_U32)addr) & 0x1fffffff)) //virtual -> physical
#define PA2KSEG0(addr)          ((void *)(((MS_U32)addr) | 0x80000000)) //physical -> cached
#define PA2KSEG1(addr)          ((void *)(((MS_U32)addr) | 0xa0000000)) //physical -> unchched

//MS_U32<->MS_U16
#define LOU16(u32Val)           ( (MS_U16)(u32Val) )
#define HIU16(u32Val)           ( (MS_U16)((u32Val) >> 16) )
#define CATU32(u16HiVal, u16LoVal)  ( (MS_U32)(u16HiVal)<<16 | (MS_U32)(u16LoVal) )

//MS_U16<->MS_U8
#define LOU8(u16Val)            ( (MS_U8)(u16Val) )
#define HIU8(u16Val)            ( (MS_U8)((u16Val) >> 8) )

//-------------------------------------------------------------------------------------------------
// Local Function Prototypes
//-------------------------------------------------------------------------------------------------

extern void add_wired_entry(unsigned long entrylo0, unsigned long entrylo1, unsigned long entryhi, unsigned long pagemask);

static MSIF_Version _drv_msos_version = {
    .DDI = { MSOS_DRV_VERSION },
};

// In order to implement period timer
void TimerTask(void)
{
    MS_U8 i = 0;
    MS_U8 u8Err = 0;

    while (1)
    {
        atomMutexGet(&_MsOS_Timer_Mutex, 0);
        for (i = 0; i < MSOS_TIMER_MAX; i++)
        {
            // Check used status
            if (_MsOS_Timer_Info[i].bUsed != TRUE)
                continue;

            // Check stop
            if (_MsOS_Timer_Info[i].u32Stop > 0)
                continue;

            // Period call back
            if ((_MsOS_Timer_Info[i].stTimer.cb_ticks == 0) &&
                (_MsOS_Timer_Info[i].u32Period > 0))
            {
                _MsOS_Timer_Info[i].stTimer.cb_ticks = _MsOS_Timer_Info[i].u32Period;
                u8Err = atomTimerRegister(&_MsOS_Timer_Info[i].stTimer);
                if (u8Err != ATOM_OK)
                    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "Period");
            }
        }
        atomMutexPut(&_MsOS_Timer_Mutex);
        // Delay
        atomTimerDelay(10);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MSOS_GetLibVer
/// @brief \b Function  \b Description: Show the MSOS driver version
/// @param ppVersion    \b Out: Library version string
/// @return             \b Result
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MSOS_GetLibVer(const MSIF_Version **ppVersion)
{
    if (!ppVersion)
        return FALSE;

    *ppVersion = &_drv_msos_version;
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Initialize MsOS
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_Init (void)
{
    MS_U32 u32I = 0;
    MS_U8 u8Err = 0;

    // Empty all the MsOS structures
    for (u32I = 0; u32I < MSOS_MEMPOOL_MAX; u32I++)
    {
        _MsOS_MemoryPool_Info[u32I].bUsed = FALSE;
    }
    for (u32I = 0; u32I < MSOS_FIXSIZE_MEMPOOL_MAX; u32I++)
    {
        _MsOS_FixSizeMemoryPool_Info[u32I].bUsed = FALSE;
    }
    if ((bInitTaskTable == FALSE) || (pUserDefineTask == NULL))
    {
        for (u32I = 0; u32I < MSOS_TASK_MAX; u32I++)
            _MsOS_Task_Info[u32I].bUsed = FALSE;
    }
    else
    {
        for (u32I = 0; u32I < MSOS_TASK_MAX; u32I++)
        {
            _MsOS_Task_Info[u32I].bUsed = pUserDefineTask[u32I];
        }
    }
    for (u32I = 0; u32I < MSOS_MUTEX_MAX; u32I++)
    {
        _MsOS_Mutex_Info[u32I].bUsed = FALSE;
    }
    for (u32I = 0; u32I < MSOS_SEMAPHORE_MAX; u32I++)
    {
        _MsOS_Semaphore_Info[u32I].bUsed = FALSE;
        _MsOS_Semaphore_Info[u32I].u32InitCnt = 0;
    }
    for (u32I = 0; u32I < MSOS_EVENTGROUP_MAX; u32I++)
    {
        _MsOS_EventGroup_Info[u32I].bUsed = FALSE;
        _MsOS_EventGroup_Info[u32I].u32EventGroup = 0;
    }
    for (u32I = 0; u32I < MSOS_QUEUE_MAX; u32I++)
    {
        _MsOS_Queue_Info[u32I].bUsed = FALSE;
    }
    for (u32I = 0; u32I < MSOS_TIMER_MAX; u32I++)
    {
        _MsOS_Timer_Info[u32I].bUsed = FALSE;
        _MsOS_Timer_Info[u32I].u32Stop = 0;
        _MsOS_Timer_Info[u32I].u32Period = 0;
    }

    // Please update the HISR priority here if it is necessary.

    // Create Mutex for all resources.
    atomMutexCreate(&_MsOS_MemoryPool_Mutex);
    atomMutexCreate(&_MsOS_FixSizeMemoryPool_Mutex);
    atomMutexCreate(&_MsOS_Task_Mutex);
    atomMutexCreate(&_MsOS_Mutex_Mutex);
    atomMutexCreate(&_MsOS_Semaphore_Mutex);
    atomMutexCreate(&_MsOS_EventGroup_Mutex);
    atomMutexCreate(&_MsOS_Timer_Mutex);
    atomMutexCreate(&_MsOS_Queue_Mutex);
    atomMutexCreate(&_MsOS_HISR_Mutex);

    // In order to implement period timer
    if (bTimerTask == FALSE)
    {
        // Create task to monitor timer
        u8Err = atomThreadCreate(&stTimerTcb,
                                E_TASK_PRI_MEDIUM,
                                (void *)TimerTask,
                                NULL,
                                u8TimeTaskStack,
                                MSOS_DEFAULT_STACK_SIZE,
                                TRUE);
        if (u8Err == ATOM_OK)
            bTimerTask = TRUE;
        else
            printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "PeriodTimeTask");
    }

#if(defined(MCU_MIPS_34K) || defined(MCU_MIPS_74K) || defined(MCU_MIPS_1004K))
    #if (defined(CHIP_T12) || defined(CHIP_A2) || defined(CHIP_T8) || defined(CHIP_J2) || defined(CHIP_A5) || defined(CHIP_A5P) || defined(CHIP_A3) || defined(CHIP_K2))
        // MIU0
        unsigned long entrylo0 = ((0x10000000>> 12) << 6) | (0<<3) | (1<< 2) | (1<< 1) | (1<< 0); // cacheable
        unsigned long entrylo1 = ((0x10000000>> 12) << 6) | (2<<3) | (1<< 2) | (1<< 1) | (1<< 0); // un-cacheable or 0x7
        // MIU1
        unsigned long entrylo0_miu1_lo256mb = ((0x20000000>> 12) << 6) | (0<<3) | (1<< 2) | (1<< 1) | (1<< 0); // cacheable
        unsigned long entrylo1_miu1_lo256mb = ((0x20000000>> 12) << 6) | (2<<3) | (1<< 2) | (1<< 1) | (1<< 0); // un-cacheable or 0x7
        unsigned long entrylo0_miu1_hi256mb = ((0x30000000>> 12) << 6) | (0<<3) | (1<< 2) | (1<< 1) | (1<< 0); // cacheable
        unsigned long entrylo1_miu1_hi256mb = ((0x30000000>> 12) << 6) | (2<<3) | (1<< 2) | (1<< 1) | (1<< 0); // un-cacheable or 0x7
        // MIU0
        add_wired_entry(entrylo0, entrylo1, 0xC0000000 , 0x1fffe000);  //  PageMask[28:13]=0x1fffe000(256MB max size)
        // MIU1
        add_wired_entry(entrylo0_miu1_lo256mb, entrylo1_miu1_lo256mb, 0x00000000 , 0x1fffe000);
        add_wired_entry(entrylo0_miu1_hi256mb, entrylo1_miu1_hi256mb, 0x20000000 , 0x1fffe000);
    #elif (defined(CHIP_U4))
        unsigned long entrylo0 = ((0x40000000>> 12) << 6) | (0<<3) | (1<< 2) | (1<< 1) | (1<< 0); // cacheable
        unsigned long entrylo1 = ((0x40000000>> 12) << 6) | (2<<3) | (1<< 2) | (1<< 1) | (1<< 0); // un-cacheable or 0x7
        add_wired_entry(entrylo0, entrylo1, MIU1_CACHEABLE_START , 0x1fffe000);
    #endif
#endif

    CHIP_InitISR();

    //can't enable INT till LISR/HISR has been registered/attached
    MsOS_DisableInterrupt(E_INT_IRQ_FIQ_ALL);

    return TRUE;
}

//
// Memory management
//
//-------------------------------------------------------------------------------------------------
/// Create a variable-size memory pool dynamically
/// @param  u32PoolSize         \b IN: pool size in bytes
/// @param  u32MinAllocation    \b IN: not used
/// @param  pPoolAddr           \b IN: starting address for the memory pool
/// @param  eAttribute          \b IN: only E_MSOS_FIFO - suspended in FIFO order
/// @param  pPoolName           \b IN: not used
/// @return >=0 : assigned memory pool ID
/// @return < 0 : fail
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_CreateMemoryPool (MS_U32        u32PoolSize,
                              MS_U32        u32MinAllocation,
                              void          *pPoolAddr,
                              MsOSAttribute eAttribute,
                              char          *pPoolName)
{
    MS_S32 s32Id;

    atomMutexGet(&_MsOS_MemoryPool_Mutex, 0);

    for(s32Id=0;s32Id<MSOS_MEMPOOL_MAX;s32Id++)
    {
        if(_MsOS_MemoryPool_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if(s32Id < MSOS_MEMPOOL_MAX)
    {
        _MsOS_MemoryPool_Info[s32Id].bUsed = TRUE;
    }
    atomMutexPut(&_MsOS_MemoryPool_Mutex);

    if(s32Id >= MSOS_MEMPOOL_MAX)
    {
        return -1;
    }

    //no Priority order for eCos
    //pPoolName is not used for eCos

    // Call OS kernel to create a memory pool.
    cyg_mempool_var_create( pPoolAddr,
                            u32PoolSize,
                            &_MsOS_MemoryPool_Info[s32Id].stMemoryPool,
                            &_MsOS_MemoryPool_Info[s32Id].stMemoryPoolInfo );

    s32Id |= MSOS_ID_PREFIX;
    return s32Id;
}

//-------------------------------------------------------------------------------------------------
/// Delete a variable-size memory pool
/// @param  s32PoolId   \b IN: pool ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DeleteMemoryPool (MS_S32 s32PoolId)
{
    if ( (s32PoolId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32PoolId &= MSOS_ID_MASK;
    }

    cyg_mempool_var_delete(_MsOS_MemoryPool_Info[s32PoolId].stMemoryPool);
    atomMutexGet(&_MsOS_MemoryPool_Mutex, 0);
    _MsOS_MemoryPool_Info[s32PoolId].bUsed = FALSE;
    atomMutexPut(&_MsOS_MemoryPool_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Get the information of a variable-size memory pool
/// @param  s32PoolId   \b IN: memory pool ID
/// @param  pPoolAddr   \b OUT: holding the starting address for the memory pool
/// @param  pu32PoolSize \b OUT: holding the total size of the memory pool
/// @param  pu32FreeSize \b OUT: holding the available free size of the memory pool
/// @param  pu32LargestFreeBlockSize  \b OUT: holding the size of the largest free block
/// @return TRUE : succeed
/// @return FALSE : the pool has not been created
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_InfoMemoryPool (MS_S32    s32PoolId,
                          void      **pPoolAddr,
                          MS_U32    *pu32PoolSize,
                          MS_U32    *pu32FreeSize,
                          MS_U32    *pu32LargestFreeBlockSize)
{
    if ( (s32PoolId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32PoolId &= MSOS_ID_MASK;
    }

    cyg_mempool_info mempool_info;
    cyg_mempool_var_get_info(_MsOS_MemoryPool_Info[s32PoolId].stMemoryPool, &mempool_info);

    *pPoolAddr = (void *)mempool_info.base;
    *pu32PoolSize = mempool_info.size;
    *pu32FreeSize = mempool_info.freemem;
    *pu32LargestFreeBlockSize = mempool_info.maxfree;

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Allocate a memory block with 16-Byte aligned starting address from the variable-size memory pool
/// @param  u32Size     \b IN: request size
/// @param  s32PoolId   \b IN: memory pool ID
/// @return NULL : not enough available memory
/// @return Otherwise : pointer to the allocated memory block
//-------------------------------------------------------------------------------------------------
void * MsOS_AllocateMemory (MS_U32 u32Size, MS_S32 s32PoolId)
{
    void  *pAddr;

    if ( (s32PoolId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX )
    {
        return NULL;
    }
    else
    {
        s32PoolId &= MSOS_ID_MASK;
    }

    // Jerry.Tsao: cyg_mempool_var_try_alloc allocates the maximum pool it has when size is zero.
    if (u32Size == 0)
    {
        return NULL;
    }

    pAddr = cyg_mempool_var_try_alloc( _MsOS_MemoryPool_Info[s32PoolId].stMemoryPool, u32Size);

    //Current eCosPro kernel always allocates 16-B aligned block
    if ( (MS_U32)pAddr % 16 )
    {
        return NULL;
    }

    return ( pAddr );
}

//-------------------------------------------------------------------------------------------------
// Allocate a block of memory with aligned starting address from the variable-size memory pool
// @param  u32Size         \b IN: requested size
// @param  s32PoolId       \b IN: memory pool ID
// @param  u32AlignedByte  \b IN: #-byte aligned start address (must be a multiple of 4)
// @return NULL : fail
// @return Otherwise : pointer to the allocated memory block
//-------------------------------------------------------------------------------------------------
void * MsOS_AllocateAlignedMemory (MS_U32 u32Size, MS_U32 u32AlignedByte, MS_S32 s32PoolId)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return NULL;
}


//-------------------------------------------------------------------------------------------------
/// Free a memory block from the variable-size memory pool
/// @param  pAddress    \b IN: pointer to previously allocated memory block
/// @param  s32PoolId   \b IN: memory pool ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_FreeMemory (void *pAddress, MS_S32 s32PoolId)
{
    if ( (s32PoolId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32PoolId &= MSOS_ID_MASK;
    }

    if (pAddress == NULL)
    {
        return FALSE;
    }

    cyg_mempool_var_free( _MsOS_MemoryPool_Info[s32PoolId].stMemoryPool, pAddress);

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Reallocate a block of memory with 4-byte aligned start address from the variable-size memory pool
/// @param  pOrgAddress \b IN: points to the beginning of the original memory block
/// @param  u32NewSize  \b IN: size of new memory block
/// @param  s32PoolId   \b IN: memory pool ID
/// @return NULL : not enough available memory to expand the block or u32NewSize == 0 && pOrgAddress != NULL
/// @return Otherwise : pointer to the reallocated (and possibly moved) memory block
//  @note   reference realloc in malloc.cxx
//-------------------------------------------------------------------------------------------------
void * MsOS_ReallocateMemory (void *pOrgAddress, MS_U32 u32NewSize, MS_S32 s32PoolId)
{
    void *pNewAddress = NULL;
    MS_U32 u32OrgSize;

    if ( (s32PoolId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX )
    {
        return NULL;
    }
    else
    {
        s32PoolId &= MSOS_ID_MASK;
    }

    if ( pOrgAddress == NULL)
    {
        //malloc
        pNewAddress = cyg_mempool_var_try_alloc( _MsOS_MemoryPool_Info[s32PoolId].stMemoryPool, u32NewSize);
        return pNewAddress;
    }

    if ( u32NewSize == 0)
    {
        //free
        cyg_mempool_var_free( _MsOS_MemoryPool_Info[s32PoolId].stMemoryPool, pOrgAddress);
        return NULL;
    }

    //mvarimpl.inl
    struct memdq {
        struct memdq *prev, *next;
        MS_U32 size;
    };

    struct memdq *dq = (struct memdq *) ((MS_U32 )pOrgAddress  - sizeof(struct memdq));
    u32OrgSize = dq->size - sizeof(struct memdq);   //dq->size was rounded up to 16B-aligned when malloc, so u32OrgSize may be larger than requested

    //No resize function is implemented, so malloc a new block directly
    pNewAddress = cyg_mempool_var_try_alloc( _MsOS_MemoryPool_Info[s32PoolId].stMemoryPool, u32NewSize);

    if ( pNewAddress != NULL) //move to a new block
    {
        if ( u32NewSize < u32OrgSize)
        {
            memcpy( pNewAddress, pOrgAddress, u32NewSize );
        }
        else
        {
            memcpy( pNewAddress, pOrgAddress, u32OrgSize );
        }
        cyg_mempool_var_free( _MsOS_MemoryPool_Info[s32PoolId].stMemoryPool, pOrgAddress);

    }
    else //not enough available memory to expand the block to the given size
    {
        //the original block is unchanged
    }

    return pNewAddress;
}

//-------------------------------------------------------------------------------------------------
/// Create a fixed-size memory pool dynamically
/// @param  u32PoolSize         \b IN: pool size in bytes
/// @param  u32BlockSize        \b IN: fixed block size for each allocated block in the pool
/// @param  pPoolAddr           \b IN: starting address for the memory pool
/// @param  eAttribute          \b IN: only E_MSOS_FIFO - suspended in FIFO order
/// @param  pPoolName           \b IN: not used
/// @return >=0 : assigned memory pool ID
/// @return < 0 : fail
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_CreateFixSizeMemoryPool (MS_U32    u32PoolSize,
                                  MS_U32    u32BlockSize,
                                  void      *pPoolAddr,
                                  MsOSAttribute eAttribute,
                                  char      *pPoolName)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return -1;
}

//-------------------------------------------------------------------------------------------------
/// Delete a fixed-size memory pool
/// @param  s32PoolId   \b IN: pool ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DeleteFixSizeMemoryPool (MS_S32 s32PoolId)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Get the information of a fixed-size memory pool
/// @param  s32PoolId   \b IN: memory pool ID
/// @param  pPoolAddr   \b OUT: holding the starting address for the memory pool
/// @param  pu32PoolSize \b OUT: holding the total size of the memory pool
/// @param  pu32FreeSize \b OUT: holding the available free size of the memory pool
/// @param  pu32LargestFreeBlockSize  \b OUT: holding the size of the largest free block
/// @return TRUE : succeed
/// @return FALSE : the pool has not been created
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_InfoFixSizeMemoryPool (MS_S32 s32PoolId,
                                 void   **pPoolAddr,
                                 MS_U32 *pu32PoolSize,
                                 MS_U32 *pu32FreeSize,
                                 MS_U32 *pu32LargestFreeBlockSize)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Allocate a memory block from the fixed-size memory pool
/// @param  s32PoolId   \b IN: memory pool ID
/// @return NULL : not enough available memory
/// @return Otherwise : pointer to the allocated memory block
/// @note   watch out for alignment if u32BlockSize is not a multiple of 4
//-------------------------------------------------------------------------------------------------
void * MsOS_AllocateFixSizeMemory (MS_S32 s32PoolId)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return NULL;
}

//-------------------------------------------------------------------------------------------------
/// Free a memory block from the fixed-size memory pool
/// @param  pAddress    \b IN: pointer to previously allocated memory block
/// @param  s32PoolId   \b IN: memory pool ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_FreeFixSizeMemory (void *pAddress, MS_S32 s32PoolId)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//
// Task
//
//-------------------------------------------------------------------------------------------------
/// Create a task
/// @param  pTaskEntry       \b IN: task entry point
/// @param  u32TaskEntryData \b IN: task entry data: a pointer to some static data, or a
///          small integer, or NULL if the task does not require any additional data.
/// @param  eTaskPriority    \b IN: task priority
/// @param  bAutoStart       \b IN: start immediately or later
/// @param  pStackBase       \b IN: task stack
/// @param  u32StackSize     \b IN: stack size
/// @param  pTaskName        \b IN: task name
/// @return >=0 : assigned Task ID
/// @return < 0 : fail
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_CreateTask (TaskEntry  pTaskEntry,
                     MS_VIRT     u32TaskEntryData,
                     TaskPriority eTaskPriority,
                     MS_BOOL    bAutoStart,
                     void       *pStackBase,
                     MS_U32     u32StackSize,
                     char       *pTaskName)
{
    MS_S32 s32Id = 0;
    MS_U8 u8Err = 0;

    // Check parameter
    if ((MS_U32)pStackBase % 4)
        return -1;

    // Get id
    atomMutexGet(&_MsOS_Task_Mutex, 0);
    for (s32Id = 0; s32Id < MSOS_TASK_MAX; s32Id++)
    {
        if (_MsOS_Task_Info[s32Id].bUsed == FALSE)
        {
            _MsOS_Task_Info[s32Id].bUsed = TRUE;
            break;
        }
    }
    atomMutexPut(&_MsOS_Task_Mutex);

    // Check id
    if (s32Id >= MSOS_TASK_MAX)
        return -1;

    // Check stack
    if (pStackBase == NULL)
    {
        MS_ASSERT(0);
    }
    else
    {
        // Create task via OS API
        u8Err = atomThreadCreate(&_MsOS_Task_Info[s32Id].stTcb,
                                eTaskPriority,
                                (void *)pTaskEntry,
                                u32TaskEntryData,
                                pStackBase,
                                u32StackSize,
                                TRUE);
        if (u8Err != ATOM_OK)
        {
            atomMutexGet(&_MsOS_Task_Mutex, 0);
            _MsOS_Task_Info[s32Id].bUsed = FALSE;
            atomMutexPut(&_MsOS_Task_Mutex);
            return -1;
        }

        // Do MsOS wrapper
        strcpy(_MsOS_Task_Info[s32Id].pName, pTaskName);
        if (bAutoStart == FALSE)
            _MsOS_Task_Info[s32Id].stTcb.terminated = TRUE;
    }

    return (s32Id | MSOS_ID_PREFIX);
}

//-------------------------------------------------------------------------------------------------
/// Delete a previously created task
/// @param  s32TaskId   \b IN: task ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DeleteTask (MS_S32 s32TaskId)
{
    // Check id
    if ((s32TaskId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32TaskId &= MSOS_ID_MASK;

    // Check running status
    if ((_MsOS_Task_Info[s32TaskId].bUsed != TRUE) ||
        (_MsOS_Task_Info[s32TaskId].stTcb.terminated != FALSE))
        return FALSE;

    // Delete task via OS struct member and MsOS member
    atomMutexGet(&_MsOS_Task_Mutex, 0);
    _MsOS_Task_Info[s32TaskId].bUsed = FALSE;
    _MsOS_Task_Info[s32TaskId].stTcb.terminated = TRUE;
    atomMutexPut(&_MsOS_Task_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Yield the execution right to ready tasks with "the same" priority
/// @return None
//-------------------------------------------------------------------------------------------------
void MsOS_YieldTask(void)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
}

//-------------------------------------------------------------------------------------------------
/// Suspend the calling task for u32Ms milliseconds
/// @param  u32Ms  \b IN: delay 1 ~ MSOS_WAIT_FOREVER ms
/// @return None
/// @note   Not allowed in interrupt context; otherwise, exception will occur.
//-------------------------------------------------------------------------------------------------
void MsOS_DelayTask (MS_U32 u32Ms)
{
    MS_U32 u32Ticks = 0;

    //u32Ms=0 => The task will enter sleep state and wake up immediately with relative trigger time = 0
    // => Other ready tasks with the same or lower priorities will have no chance to run
    MS_ASSERT(u32Ms != 0);
    MS_ASSERT(MsOS_In_Interrupt() == FALSE);

    // Transform unit
    u32Ticks = u32Ms;

    // Delay task via OS API
    if (u32Ticks < 1)
        atomTimerDelay(1);
    else
        atomTimerDelay(u32Ticks);
}

//-------------------------------------------------------------------------------------------------
/// Delay for u32Us microseconds
/// @param  u32Us  \b IN: delay 0 ~ 999 us
/// @return None
/// @note   implemented by "busy waiting". Plz call MsOS_DelayTask directly for ms-order delay
//-------------------------------------------------------------------------------------------------
void MsOS_DelayTaskUs (MS_U32 u32Us)
{
    MAsm_CPU_DelayUs(u32Us);
}

//-------------------------------------------------------------------------------------------------
/// Delay Poll for u32Us microseconds
/// @param  u32Us  \b IN: delay 0 ~ 999 us
/// @return None
/// @note   implemented by "busy waiting". Plz call MsOS_DelayTask directly for ms-order delay
//-------------------------------------------------------------------------------------------------
void MsOS_DelayTaskUs_Poll(MS_U32 u32Us)
{
    MAsm_CPU_DelayUs(u32Us);
}

//-------------------------------------------------------------------------------------------------
/// Resume the specified suspended task
/// @param  s32TaskId   \b IN: Task ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   This API is not supported in Linux
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_ResumeTask (MS_S32 s32TaskId)
{
    // Check id
    if ((s32TaskId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32TaskId &= MSOS_ID_MASK;

    // Check suspend status
    if ((_MsOS_Task_Info[s32TaskId].bUsed != TRUE) ||
        (_MsOS_Task_Info[s32TaskId].stTcb.terminated != TRUE))
        return FALSE;

    // Resume task via OS struct member
    atomMutexGet(&_MsOS_Task_Mutex, 0);
    _MsOS_Task_Info[s32TaskId].stTcb.terminated = FALSE;
    atomMutexPut(&_MsOS_Task_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Suspend the specified task
/// @param  s32TaskId   \b IN: Task ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   This API is not supported in Linux
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_SuspendTask (MS_S32 s32TaskId)
{
    // Check id
    if ((s32TaskId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32TaskId &= MSOS_ID_MASK;

    // Check runnig status
    if ((_MsOS_Task_Info[s32TaskId].bUsed != TRUE) ||
        (_MsOS_Task_Info[s32TaskId].stTcb.terminated != FALSE))
        return FALSE;

    // Suspend tsak via OS struct member
    atomMutexGet(&_MsOS_Task_Mutex, 0);
    _MsOS_Task_Info[s32TaskId].stTcb.terminated = TRUE;
    atomMutexPut(&_MsOS_Task_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Get a task information
/// @param  s32TaskId       \b IN: task ID
/// @param  peTaskPriority  \b OUT: ptr to task priority
/// @param  pStackBase      \b OUT: ptr to stack base
/// @param  pu32StackSize   \b OUT: ptr to stack size
/// @param  pTaskName       \b OUT: ptr to task name
/// @return TRUE : succeed
/// @return FALSE : the task has not been created
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_InfoTask (MS_S32 s32TaskId,
                       TaskPriority *peTaskPriority,
                       void* pStackBase,
                       MS_U32* pu32StackSize,
                       MS_U32* pu32StackUsed,
                       char *pTaskName)
{
    // Check parameter
    if ((peTaskPriority == NULL) || (pStackBase == NULL) ||
         (pu32StackSize == NULL) || (pTaskName == NULL))
        return FALSE;

    // Check id
    if ((s32TaskId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32TaskId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Task_Info[s32TaskId].bUsed != TRUE)
        return FALSE;

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_Task_Mutex, 0);
    *peTaskPriority = (TaskPriority)_MsOS_Task_Info[s32TaskId].stTcb.priority;
    *((MS_U32 *)pStackBase) = (MS_U32)(_MsOS_Task_Info[s32TaskId].stTcb.stack_bottom);
    *pu32StackSize = (MS_U32)(_MsOS_Task_Info[s32TaskId].stTcb.stack_size);
    if (pu32StackUsed != NULL)
        printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "StackUsed");
    strcpy(pTaskName, _MsOS_Task_Info[s32TaskId].pName);
    atomMutexPut(&_MsOS_Task_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Get all task ID
/// @param  ps32TaskIdList   \b IN/OUT: the memory for the all task ID
/// @return number of task created
//-------------------------------------------------------------------------------------------------
MS_U32 MsOS_InfoTaskList (MS_S32* ps32TaskIdList)
{
    MS_S32 s32Id = 0;
    MS_U32 u32TaskNum = 0;

    // Find used task
    for (s32Id = 0; s32Id < MSOS_TASK_MAX; s32Id++)
    {
        if (_MsOS_Task_Info[s32Id].bUsed == TRUE)
        {
            ps32TaskIdList[u32TaskNum] = (s32Id | MSOS_ID_PREFIX);
            u32TaskNum++;
        }
    }
    return u32TaskNum;
}

//-------------------------------------------------------------------------------------------------
/// Get thread ID of current thread/process in Linux(Kernel)
/// @return : current thread ID
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_GetOSThreadID (void)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Get current task ID
/// @return >=0 : current task ID
/// @return <0 : fail
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_InfoTaskID (void)
{
    ATOM_TCB* stTcb = NULL;
    MS_S32 s32Id = 0;

    // Get current task via OS API
    stTcb = atomCurrentContext();

    // Compare date for each task
    for (s32Id = 0; s32Id < MSOS_TASK_MAX; s32Id++)
    {
        if ((_MsOS_Task_Info[s32Id].bUsed == TRUE) &&
            (_MsOS_Task_Info[s32Id].stTcb.entry_point == stTcb->entry_point))
            return s32Id;
    }
    return -1;
}

//
// Mutex
//
//-------------------------------------------------------------------------------------------------
/// Create a mutex in the unlocked state
/// @param  eAttribute  \b IN: E_MSOS_FIFO: suspended in FIFO order
/// @param  pMutexName  \b IN: mutex name
/// @return >=0 : assigned mutex Id
/// @return <0 : fail
/// @note   A mutex has the concept of an owner, whereas a semaphore does not.
///         A mutex provides priority inheritance protocol against proiorty inversion, whereas a binary semaphore does not.
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_CreateMutex ( MsOSAttribute eAttribute, char *pMutexName, MS_U32 u32Flag)
{
    MS_S32 s32Id = 0;
    MS_U8 u8Err = 0;

    // Get id
    atomMutexGet(&_MsOS_Mutex_Mutex, 0);
    for (s32Id = 0; s32Id < MSOS_MUTEX_MAX; s32Id++)
    {
        if (_MsOS_Mutex_Info[s32Id].bUsed == FALSE)
        {
            _MsOS_Mutex_Info[s32Id].bUsed = TRUE;
            break;
        }
    }
    atomMutexPut(&_MsOS_Mutex_Mutex);

    // Check id
    if (s32Id >= MSOS_MUTEX_MAX)
        return -1;

    // Create mutex via OS API
    u8Err = atomMutexCreate(&_MsOS_Mutex_Info[s32Id].stMutex);
    if (u8Err != ATOM_OK)
    {
        atomMutexGet(&_MsOS_Mutex_Mutex, 0);
        _MsOS_Mutex_Info[s32Id].bUsed = FALSE;
        atomMutexPut(&_MsOS_Mutex_Mutex);
        return -1;
    }

    // Do MsOS wrapper
    /* \par Priority-based queueing
     * Where multiple threads are blocking on a mutex, they are woken in
     * order of the threads' priorities. Where multiple threads of the same
     * priority are blocking, they are woken in FIFO order. */
    if (eAttribute == E_MSOS_FIFO)
        printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "E_MSOS_FIFO");
    strcpy(_MsOS_Mutex_Info[s32Id].pName, pMutexName);
    if (u32Flag == MSOS_PROCESS_PRIVATE)
        printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "MSOS_PROCESS_PRIVATE");
    else if (u32Flag == MSOS_PROCESS_SHARED)
        printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "MSOS_PROCESS_SHARED");

    return (s32Id | MSOS_ID_PREFIX);
}

//-------------------------------------------------------------------------------------------------
/// Delete the specified mutex
/// @param  s32MutexId  \b IN: mutex ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   It is important that the mutex be in the unlocked state when it is
///            destroyed, or else the behavior is undefined.
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DeleteMutex (MS_S32 s32MutexId)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32MutexId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32MutexId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Mutex_Info[s32MutexId].bUsed != TRUE)
        return FALSE;

    // Delete mutex by OS API
    u8Err = atomMutexDelete(&_MsOS_Mutex_Info[s32MutexId].stMutex);
    if (u8Err != ATOM_OK)
        return FALSE;

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_Mutex_Mutex, 0);
    _MsOS_Mutex_Info[s32MutexId].bUsed = FALSE;
    atomMutexPut(&_MsOS_Mutex_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Attempt to lock a mutex
/// @param  s32MutexId  \b IN: mutex ID
/// @param  u32WaitMs   \b IN: 0 ~ MSOS_WAIT_FOREVER: suspend time (ms) if the mutex is locked
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_ObtainMutex (MS_S32 s32MutexId, MS_U32 u32WaitMs)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32MutexId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32MutexId &= MSOS_ID_MASK;

    MS_ASSERT(MsOS_In_Interrupt() == FALSE);

    // Check used status
    if (_MsOS_Mutex_Info[s32MutexId].bUsed != TRUE)
        return FALSE;

    // Get mutex via OS API
    if (u32WaitMs == MSOS_WAIT_FOREVER) //blocking wait
        u8Err = atomMutexGet(&_MsOS_Mutex_Info[s32MutexId].stMutex, 0);
    else if (u32WaitMs == 0) //non-blocking
        u8Err = atomMutexGet(&_MsOS_Mutex_Info[s32MutexId].stMutex, -1);
    else //blocking wait with timeout
        u8Err = atomMutexGet(&_MsOS_Mutex_Info[s32MutexId].stMutex, u32WaitMs);

    return ((u8Err == ATOM_OK) ? TRUE : FALSE);
}

//-------------------------------------------------------------------------------------------------
/// Attempt to unlock a mutex
/// @param  s32MutexId  \b IN: mutex ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   Only the owner thread of the mutex can unlock it.
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_ReleaseMutex (MS_S32 s32MutexId)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32MutexId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32MutexId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Mutex_Info[s32MutexId].bUsed != TRUE)
        return FALSE;

    // Put mutex via OS API
    u8Err = atomMutexPut(&_MsOS_Mutex_Info[s32MutexId].stMutex);

    return ((u8Err == ATOM_OK) ? TRUE : FALSE);
}

//-------------------------------------------------------------------------------------------------
// Get a mutex informaton
// @param  s32MutexId  \b IN: mutex ID
// @param  peAttribute \b OUT: ptr to suspended mode: E_MSOS_FIFO / E_MSOS_PRIORITY
// @param  pMutexName  \b OUT: ptr to mutex name
// @return TRUE : succeed
// @return FALSE : the mutex has not been created.
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_InfoMutex (MS_S32 s32MutexId, MsOSAttribute *peAttribute, char *pMutexName)
{
    // Check parameter
    if ((peAttribute == NULL) || (pMutexName == NULL))
        return FALSE;

    // Check id
    if ((s32MutexId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32MutexId &= MSOS_ID_MASK;

    // Check use status
    if (_MsOS_Mutex_Info[s32MutexId].bUsed != TRUE)
        return FALSE;

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_Mutex_Mutex, 0);
    /* \par Priority-based queueing
     * Where multiple threads are blocking on a mutex, they are woken in
     * order of the threads' priorities. Where multiple threads of the same
     * priority are blocking, they are woken in FIFO order. */
    *peAttribute = E_MSOS_PRIORITY;
    strcpy(pMutexName, _MsOS_Mutex_Info[s32MutexId].pName);
    atomMutexPut(&_MsOS_Mutex_Mutex);
    return TRUE;
}

//
// Semaphore
//
//-------------------------------------------------------------------------------------------------
/// Create a semaphore
/// @param  u32InitCnt \b IN: initial semaphore value
/// @param  eAttribute \b IN: E_MSOS_FIFO suspended in FIFO order
/// @param  pSemaphoreName \b IN: semaphore name
/// @return >=0 : assigned Semaphore Id
/// @return <0 : fail
/// @note   A semaphore does not have the concept of an owner; it is possible for one thread to lock a
///           binary semaphore and another thread to unlock it.
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_CreateSemaphore (MS_U32 u32InitCnt,
                          MsOSAttribute eAttribute,
                          char *pSemaphoreName)
{
    MS_S32 s32Id = 0;
    MS_U8 u8Err = 0;

    // Get id
    atomMutexGet(&_MsOS_Semaphore_Mutex, 0);
    for (s32Id = 0; s32Id < MSOS_SEMAPHORE_MAX; s32Id++)
    {
        if (_MsOS_Semaphore_Info[s32Id].bUsed == FALSE)
        {
            _MsOS_Semaphore_Info[s32Id].bUsed = TRUE;
            break;
        }
    }
    atomMutexPut(&_MsOS_Semaphore_Mutex);

    // Check id
    if (s32Id >= MSOS_SEMAPHORE_MAX)
        return -1;

    // Create semaphore by OS API
    u8Err = atomSemCreate(&_MsOS_Semaphore_Info[s32Id].stSemaphore, u32InitCnt);
    if (u8Err != ATOM_OK)
    {
        atomMutexGet(&_MsOS_Semaphore_Mutex, 0);
        _MsOS_Semaphore_Info[s32Id].bUsed = FALSE;
        atomMutexPut(&_MsOS_Semaphore_Mutex);
        return -1;
    }

    // Do MsOS wrapper
    _MsOS_Semaphore_Info[s32Id].u32InitCnt = u32InitCnt;
    /* \par Priority-based queueing
     * Where multiple threads are blocking on a semaphore, they are woken in
     * order of the threads' priorities. Where multiple threads of the same
     * priority are blocking, they are woken in FIFO order. */
    if (eAttribute == E_MSOS_FIFO)
        printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "E_MSOS_FIFO");
    strcpy(_MsOS_Semaphore_Info[s32Id].pName, pSemaphoreName);

    return (s32Id | MSOS_ID_PREFIX);
}

//-------------------------------------------------------------------------------------------------
/// Delete the specified semaphore
/// @param  s32SemaphoreId  \b IN: semaphore ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   It is important that there are not any threads waiting on the semaphore
///             when this function is called or the behavior is undefined.
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DeleteSemaphore (MS_S32 s32SemaphoreId)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32SemaphoreId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32SemaphoreId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Semaphore_Info[s32SemaphoreId].bUsed != TRUE)
        return FALSE;

    // Delete semaphore via OS API
    u8Err = atomSemDelete(&_MsOS_Semaphore_Info[s32SemaphoreId].stSemaphore);
    if (u8Err != ATOM_OK)
        return FALSE;

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_Semaphore_Mutex, 0);
    _MsOS_Semaphore_Info[s32SemaphoreId].bUsed = FALSE;
    atomMutexPut(&_MsOS_Semaphore_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Attempt to decrement a semaphore count
/// @param  s32SemaphoreId  \b IN: semaphore ID
/// @param  u32WaitMs       \b IN: 0 ~ MSOS_WAIT_FOREVER: suspend time (ms) if the semaphore count = 0
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_ObtainSemaphore (MS_S32 s32SemaphoreId, MS_U32 u32WaitMs)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32SemaphoreId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32SemaphoreId &= MSOS_ID_MASK;

    MS_ASSERT(MsOS_In_Interrupt() == FALSE);

    // check used status
    if (_MsOS_Semaphore_Info[s32SemaphoreId].bUsed != TRUE)
        return FALSE;

    // Get semaphore via OS API
    if (u32WaitMs == MSOS_WAIT_FOREVER) //blocking wait
        u8Err = atomSemGet(&_MsOS_Semaphore_Info[s32SemaphoreId].stSemaphore, 0);
    else if (u32WaitMs == 0) //non-blocking
        u8Err = atomSemGet(&_MsOS_Semaphore_Info[s32SemaphoreId].stSemaphore, -1);
    else //blocking wait with timeout
        u8Err = atomSemGet(&_MsOS_Semaphore_Info[s32SemaphoreId].stSemaphore, u32WaitMs);

    return ((u8Err == ATOM_OK) ? TRUE : FALSE);
}

//-------------------------------------------------------------------------------------------------
/// Increase a semaphore count
/// @param  s32SemaphoreId  \b IN: semaphore ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   It's possible for any thread to increase the semaphore count
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_ReleaseSemaphore (MS_S32 s32SemaphoreId)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32SemaphoreId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32SemaphoreId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Semaphore_Info[s32SemaphoreId].bUsed != TRUE)
        return FALSE;

    // Put semaphore via OS API
    u8Err = atomSemPut(&_MsOS_Semaphore_Info[s32SemaphoreId].stSemaphore);

    return ((u8Err == ATOM_OK) ? TRUE : FALSE);
}

//-------------------------------------------------------------------------------------------------
// Get a semaphore informaton
// @param  s32SemaphoreId  \b IN: semaphore ID
// @param  pu32InitCnt     \b OUT: ptr to initial semaphore value
// @param  peAttribute     \b OUT: ptr to suspended mode: E_MSOS_FIFO / E_MSOS_PRIORITY
// @param  pSemaphoreName  \b OUT: ptr to semaphore name
// @return TRUE : succeed
// @return FALSE : the semaphore has not been created.
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_InfoSemaphore (MS_S32 s32SemaphoreId, MS_U32 *pu32InitCnt,
                            MsOSAttribute *peAttribute, char *pSemaphoreName)
{
    // Check parameter
    if ((pu32InitCnt == NULL) || (peAttribute == NULL) || (pSemaphoreName == NULL))
        return FALSE;

    // Check id
    if ((s32SemaphoreId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32SemaphoreId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Semaphore_Info[s32SemaphoreId].bUsed != TRUE)
        return FALSE;

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_Semaphore_Mutex, 0);
    *pu32InitCnt = _MsOS_Semaphore_Info[s32SemaphoreId].u32InitCnt;
    /* \par Priority-based queueing
     * Where multiple threads are blocking on a semaphore, they are woken in
     * order of the threads' priorities. Where multiple threads of the same
     * priority are blocking, they are woken in FIFO order. */
    *peAttribute = E_MSOS_PRIORITY;
    strcpy(pSemaphoreName, _MsOS_Semaphore_Info[s32SemaphoreId].pName);
    atomMutexPut(&_MsOS_Semaphore_Mutex);
    return TRUE;
}

//
// Event management
//
//-------------------------------------------------------------------------------------------------
/// Create an event group
/// @param  pEventName  \b IN: event group name
/// @return >=0 : assigned Event Id
/// @return <0 : fail
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_CreateEventGroup (char *pEventName)
{
    MS_S32 s32Id = 0;
    MS_U8 u8Err = 0;

    // Get id
    atomMutexGet(&_MsOS_EventGroup_Mutex, 0);
    for (s32Id = 0; s32Id < MSOS_EVENTGROUP_MAX; s32Id++)
    {
        if(_MsOS_EventGroup_Info[s32Id].bUsed == FALSE)
        {
            _MsOS_EventGroup_Info[s32Id].bUsed = TRUE;
            break;
        }
    }
    atomMutexPut(&_MsOS_EventGroup_Mutex);

    // Check id
    if(s32Id >= MSOS_EVENTGROUP_MAX)
        return -1;

    // Create event mutex via OS API
    u8Err = atomMutexCreate(&_MsOS_EventGroup_Info[s32Id].stMutexEvent);
    if (u8Err != ATOM_OK)
    {
        atomMutexGet(&_MsOS_EventGroup_Mutex, 0);
        _MsOS_EventGroup_Info[s32Id].bUsed = FALSE;
        atomMutexPut(&_MsOS_EventGroup_Mutex);
        return -1;
    }

    // Do MsOS wrapper
    _MsOS_EventGroup_Info[s32Id].u32EventGroup = 0;
    strcpy(_MsOS_EventGroup_Info[s32Id].pName, pEventName);

    return (s32Id | MSOS_ID_PREFIX);
}

//-------------------------------------------------------------------------------------------------
/// Delete the event group
/// @param  s32EventGroupId \b IN: event group ID
/// @return TRUE : succeed
/// @return FALSE : fail, sb is waiting for the event flag
/// @note event group that are being waited on must not be deleted
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DeleteEventGroup (MS_S32 s32EventGroupId)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32EventGroupId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32EventGroupId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_EventGroup_Info[s32EventGroupId].bUsed != TRUE)
        return FALSE;

    // Delete event mutex via OS API
    u8Err = atomMutexDelete(&_MsOS_EventGroup_Info[s32EventGroupId].stMutexEvent);
    if (u8Err != ATOM_OK)
        return FALSE;

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_EventGroup_Mutex, 0);
    _MsOS_EventGroup_Info[s32EventGroupId].bUsed = FALSE;
    atomMutexPut(&_MsOS_EventGroup_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Set the event flag (bitwise OR w/ current value) in the specified event group
/// @param  s32EventGroupId \b IN: event group ID
/// @param  u32EventFlag    \b IN: event flag value
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_SetEvent (MS_S32 s32EventGroupId, MS_U32 u32EventFlag)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32EventGroupId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32EventGroupId &= MSOS_ID_MASK;

    // check used status
    if (_MsOS_EventGroup_Info[s32EventGroupId].bUsed != TRUE)
        return FALSE;

    // Set event via MsOS member
    u8Err = atomMutexGet(&_MsOS_EventGroup_Info[s32EventGroupId].stMutexEvent, 0);
    _MsOS_EventGroup_Info[s32EventGroupId].u32EventGroup |= u32EventFlag;
    if (u8Err == ATOM_ERR_CONTEXT)
        printf("[%s][%d] atomMutexGet in ISR\n", __FUNCTION__, __LINE__);
    else
        atomMutexPut(&_MsOS_EventGroup_Info[s32EventGroupId].stMutexEvent);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Clear the specified event flag (bitwise XOR operation) in the specified event group
/// @param  s32EventGroupId \b IN: event group ID
/// @param  u32EventFlag    \b IN: event flag value
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_ClearEvent (MS_S32 s32EventGroupId, MS_U32 u32EventFlag)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32EventGroupId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32EventGroupId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_EventGroup_Info[s32EventGroupId].bUsed != TRUE)
        return FALSE;

    // Clear event via MsOS member
    u8Err = atomMutexGet(&_MsOS_EventGroup_Info[s32EventGroupId].stMutexEvent, 0);
    _MsOS_EventGroup_Info[s32EventGroupId].u32EventGroup &= (~u32EventFlag);
    if (u8Err == ATOM_ERR_CONTEXT)
        printf("[%s][%d] atomMutexGet in ISR\n", __FUNCTION__, __LINE__);
    else
        atomMutexPut(&_MsOS_EventGroup_Info[s32EventGroupId].stMutexEvent);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Wait for the specified event flag combination from the event group
/// @param  s32EventGroupId     \b IN: event group ID
/// @param  u32WaitEventFlag    \b IN: wait event flag value
/// @param  pu32RetrievedEventFlag \b OUT: retrieved event flag value
/// @param  eWaitMode           \b IN: E_AND/E_OR/E_AND_CLEAR/E_OR_CLEAR
/// @param  u32WaitMs           \b IN: 0 ~ MSOS_WAIT_FOREVER: suspend time (ms) if the event is not ready
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_WaitEvent (MS_S32     s32EventGroupId,
                     MS_U32     u32WaitEventFlag,
                     MS_U32     *pu32RetrievedEventFlag,
                     EventWaitMode eWaitMode,
                     MS_U32     u32WaitMs)
{
    MS_U32 u32Timeout = 0;
    MS_U32 u32Delay = 0;

    // Check id
    if ((s32EventGroupId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32EventGroupId &= MSOS_ID_MASK;

    // Check used status and parameter
    if ((_MsOS_EventGroup_Info[s32EventGroupId].bUsed != TRUE) ||
        (u32WaitEventFlag == 0) || (pu32RetrievedEventFlag == NULL))
        return FALSE;

    // Prepare while loop
    *pu32RetrievedEventFlag = 0;
    u32Timeout = (u32WaitMs == MSOS_WAIT_FOREVER) ? 0xFFFFFFFF : u32WaitMs;

    // Wait event via MsOS member
    do
    {
        // Receive event
        if ((((eWaitMode == E_AND) || (eWaitMode == E_AND_CLEAR)) &&
            ((_MsOS_EventGroup_Info[s32EventGroupId].u32EventGroup & u32WaitEventFlag) == u32WaitEventFlag))
            ||
            (((eWaitMode == E_OR) || (eWaitMode == E_OR_CLEAR)) &&
            ((_MsOS_EventGroup_Info[s32EventGroupId].u32EventGroup & u32WaitEventFlag) != 0)))
        {
            *pu32RetrievedEventFlag = _MsOS_EventGroup_Info[s32EventGroupId].u32EventGroup;
            break;
        }

        // Delay via OS API
        u32Delay = (u32Timeout >= 10) ? 10 : u32Timeout;
        if (u32Delay)
               atomTimerDelay(u32Delay); // Delay 10ms
        if (u32WaitMs != MSOS_WAIT_FOREVER)
               u32Timeout -= u32Delay;

    } while (u32Timeout > 0);

    // Check receive event
    if (*pu32RetrievedEventFlag == 0)
        return FALSE;

    // Clear Mode
    if ((eWaitMode == E_AND_CLEAR) || (eWaitMode == E_OR_CLEAR))
    {
        atomMutexGet(&_MsOS_EventGroup_Info[s32EventGroupId].stMutexEvent, 0);
        _MsOS_EventGroup_Info[s32EventGroupId].u32EventGroup &= ~(*pu32RetrievedEventFlag);
        atomMutexPut(&_MsOS_EventGroup_Info[s32EventGroupId].stMutexEvent);
    }
    return TRUE;
}

//
// Timer management
//
//-------------------------------------------------------------------------------------------------
/// Create a Timer
/// @param  pTimerCb        \b IN: timer callback function
/// @param  u32FirstTimeMs  \b IN: first ms for timer expiration
/// @param  u32PeriodTimeMs \b IN: periodic ms for timer expiration after first expiration
///                                0: one shot timer
/// @param  bStartTimer     \b IN: TRUE: activates the timer after it is created
///                                FALSE: leaves the timer disabled after it is created
/// @param  pTimerName      \b IN: Timer name (not used by eCos)
/// @return >=0 : assigned Timer ID
///         <0 : fail
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_CreateTimer (TimerCb   pTimerCb,
                         MS_U32    u32FirstTimeMs,
                         MS_U32    u32PeriodTimeMs,
                         MS_BOOL   bStartTimer,
                         char      *pTimerName)
{
    MS_S32 s32Id = 0;
    MS_U8 u8Err = 0;

    // Check timer task
    if (bTimerTask == FALSE)
        printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "PeriodTimeTask");

    // Get id
    atomMutexGet(&_MsOS_Timer_Mutex, 0);
    for (s32Id = 0; s32Id < MSOS_TIMER_MAX; s32Id++)
    {
        if (_MsOS_Timer_Info[s32Id].bUsed == FALSE)
        {
            _MsOS_Timer_Info[s32Id].bUsed = TRUE;
            break;
        }
    }
    atomMutexPut(&_MsOS_Timer_Mutex);

    // Check id
    if (s32Id >= MSOS_TIMER_MAX)
        return -1;

    // Fill out data by OS API parameter
    _MsOS_Timer_Info[s32Id].stTimer.cb_func = (TIMER_CB_FUNC)pTimerCb;
    _MsOS_Timer_Info[s32Id].stTimer.cb_data = NULL;
    _MsOS_Timer_Info[s32Id].stTimer.cb_ticks = u32FirstTimeMs;

    // Do MsOS wrapper
    _MsOS_Timer_Info[s32Id].u32Stop = (bStartTimer ? 0 : u32FirstTimeMs);
    _MsOS_Timer_Info[s32Id].u32Period = u32PeriodTimeMs;
    strcpy(_MsOS_Timer_Info[s32Id].pName, pTimerName);

    // Start now
    if (_MsOS_Timer_Info[s32Id].u32Stop == 0)
    {
        // Create timer by OS API
        u8Err = atomTimerRegister(&_MsOS_Timer_Info[s32Id].stTimer);
        if (u8Err != ATOM_OK)
        {
            atomMutexGet(&_MsOS_Timer_Mutex, 0);
            _MsOS_Timer_Info[s32Id].bUsed = FALSE;
            atomMutexPut(&_MsOS_Timer_Mutex);
            return -1;
        }
    }

    return (s32Id | MSOS_ID_PREFIX);
}

//-------------------------------------------------------------------------------------------------
/// Delete the Timer
/// @param  s32TimerId  \b IN: Timer ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DeleteTimer (MS_S32 s32TimerId)
{
    MS_U8 u8Err = 0;

    // Check id
    if ((s32TimerId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32TimerId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Timer_Info[s32TimerId].bUsed != TRUE)
        return FALSE;

    // Delete timer via OS API
    u8Err = atomTimerCancel(&_MsOS_Timer_Info[s32TimerId].stTimer);
    if (u8Err != ATOM_OK)
        return FALSE;

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_Timer_Mutex, 0);
    _MsOS_Timer_Info[s32TimerId].bUsed = FALSE;
    atomMutexPut(&_MsOS_Timer_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Start the Timer
/// @param  s32TimerId  \b IN: Timer ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_StartTimer (MS_S32 s32TimerId)
{
    MS_U8 u8Err = 0;

    // Check timer task
    if (bTimerTask == FALSE)
        printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "StartTimer");

    // Check id
    if ((s32TimerId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32TimerId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Timer_Info[s32TimerId].bUsed != TRUE)
        return FALSE;

    // Check timer stop
    if (_MsOS_Timer_Info[s32TimerId].u32Stop == 0)
        return FALSE;

    // Start timer via OS API after timer stop
    u8Err = atomTimerRegister(&_MsOS_Timer_Info[s32TimerId].stTimer);
    if (u8Err != ATOM_OK)
        return FALSE;

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_Timer_Mutex, 0);
    _MsOS_Timer_Info[s32TimerId].u32Stop = 0;
    atomMutexPut(&_MsOS_Timer_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Stop the Timer
/// @param  s32TimerId  \b IN: Timer ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   MsOS_StopTimer then MsOS_StartTimer => The timer will trigger at the same relative
///             intervals that it would have if it had not been disabled.
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_StopTimer (MS_S32 s32TimerId)
{
    MS_U8 u8Err = 0;

    // Check timer task
    if (bTimerTask == FALSE)
        printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "StopTimer");

    // Check id
    if ((s32TimerId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32TimerId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Timer_Info[s32TimerId].bUsed != TRUE)
        return FALSE;

    // Check timer running
    if (_MsOS_Timer_Info[s32TimerId].u32Stop > 0)
        return FALSE;

    // Too late
    if (_MsOS_Timer_Info[s32TimerId].stTimer.cb_ticks < 500)
        return FALSE;

    // Stop timer via OS API after timer running
    u8Err = atomTimerCancel(&_MsOS_Timer_Info[s32TimerId].stTimer);
    if (u8Err != ATOM_OK)
        return FALSE;

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_Timer_Mutex, 0);
    _MsOS_Timer_Info[s32TimerId].u32Stop = _MsOS_Timer_Info[s32TimerId].stTimer.cb_ticks;
    atomMutexPut(&_MsOS_Timer_Mutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Reset a Timer & reset the expiration periods
/// @param  s32TimerId      \b IN: Timer ID
/// @param  u32FirstTimeMs  \b IN: first ms for timer expiration
/// @param  u32PeriodTimeMs \b IN: periodic ms for timer expiration after first expiration
///                                0: one shot timer
/// @param  bStartTimer     \b IN: TRUE: activates the timer after it is created
///                                FALSE: leaves the timer disabled after it is created
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_ResetTimer (MS_S32     s32TimerId,
                         MS_U32     u32FirstTimeMs,
                         MS_U32     u32PeriodTimeMs,
                         MS_BOOL    bStartTimer)
{
    MS_U8 u8Err = 0;

    // Check timer task
    if (bTimerTask == FALSE)
        printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, "PeriodTimeTask");

    // Check id
    if ((s32TimerId & MSOS_ID_PREFIX_MASK) != MSOS_ID_PREFIX)
        return FALSE;

    // Get id
    s32TimerId &= MSOS_ID_MASK;

    // Check used status
    if (_MsOS_Timer_Info[s32TimerId].bUsed != TRUE)
        return FALSE;

    // Timer running
    if (_MsOS_Timer_Info[s32TimerId].stTimer.cb_ticks > 0)
    {
        // Too late
        if (_MsOS_Timer_Info[s32TimerId].stTimer.cb_ticks < 500)
            return FALSE;

        // Stop timer via OS API
        u8Err = atomTimerCancel(&_MsOS_Timer_Info[s32TimerId].stTimer);
        if (u8Err != ATOM_OK)
            return FALSE;
    }

    // Do MsOS wrapper
    atomMutexGet(&_MsOS_Timer_Mutex, 0);
    _MsOS_Timer_Info[s32TimerId].stTimer.cb_ticks = u32FirstTimeMs;
    _MsOS_Timer_Info[s32TimerId].u32Stop = (bStartTimer ? 0 : u32FirstTimeMs);
    _MsOS_Timer_Info[s32TimerId].u32Period = u32PeriodTimeMs;
    atomMutexPut(&_MsOS_Timer_Mutex);

    // Restart timer via OS API after stop timer
    u8Err = atomTimerRegister(&_MsOS_Timer_Info[s32TimerId].stTimer);

    return ((u8Err == ATOM_OK) ? TRUE : FALSE);
}

//
// System time
//
//-------------------------------------------------------------------------------------------------
/// Get current system time in ms
/// @return system time in ms
//-------------------------------------------------------------------------------------------------
MS_U32 MsOS_GetSystemTime (void)
{
    return atomTimeGet();
}

//-------------------------------------------------------------------------------------------------
///[OBSOLETE]
/// Time difference between current time and task time
/// @return system time diff in ms
//-------------------------------------------------------------------------------------------------
MS_U32 MsOS_Timer_DiffTimeFromNow(MS_U32 u32TaskTimer) //unit = ms
{
    return (atomTimeGet() - u32TaskTimer);
}

//-------------------------------------------------------------------------------------------------
///[OBSOLETE]
/// Time difference between setting time and task time
/// @return system time diff in ms
//-------------------------------------------------------------------------------------------------
MS_U32 MsOS_Timer_DiffTime(MS_U32 u32Timer, MS_U32 u32TaskTimer) //unit = ms
{
    return abs(u32Timer - u32TaskTimer);
}

//
// Queue
//
//-------------------------------------------------------------------------------------------------
/// Create a Queue
/// @param  pStartAddr      \b IN: It is useless now, can pass NULL.
/// @param  u32QueueSize    \b IN: queue size (byte unit) : now fixed as
///                                CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE * u32MessageSize
/// @param  eMessageType    \b IN: E_MSG_FIXED_SIZE / E_MSG_VAR_SIZE
/// @param  u32MessageSize  \b IN: message size (byte unit) for E_MSG_FIXED_SIZE
///                                max message size (byte unit) for E_MSG_VAR_SIZE
/// @param  eAttribute      \b IN: E_MSOS_FIFO suspended in FIFO order
/// @param  pQueueName      \b IN: queue name
/// @return assigned message queue ID
/// @return < 0 - fail
//-------------------------------------------------------------------------------------------------
MS_S32 MsOS_CreateQueue(void            *pStartAddr,
                        MS_U32          u32QueueSize,
                        MessageType     eMessageType,
                        MS_U32          u32MessageSize,
                        MsOSAttribute   eAttribute,
                        char            *pQueueName)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return -1;
}

//-------------------------------------------------------------------------------------------------
/// Delete the Queue
/// @param  s32QueueId  \b IN: Queue ID
/// @return TRUE : succeed
/// @return FALSE :  fail
/// @note   It is important that there are not any threads blocked on the queue
///             when this function is called or the behavior is undefined.
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DeleteQueue (MS_S32 s32QueueId)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Send a message to the end of the specified queue
/// @param  s32QueueId  \b IN: Queue ID
/// @param  pu8Message  \b IN: ptr to msg to send. NULL ptr is not allowed
/// @param  u32Size     \b IN: msg size (byte)
/// @param  u32WaitMs   \b IN: 0 ~ MSOS_WAIT_FOREVER: suspend time (ms) if the queue is full
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_SendToQueue (MS_S32 s32QueueId, MS_U8 *pu8Message, MS_U32 u32Size, MS_U32 u32WaitMs)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Receive a message from the specified queue
/// @param  s32QueueId      \b IN: Queue ID
/// @param  pu8Message      \b OUT: msg destination
/// @param  u32IntendedSize \b IN: intended msg size (byte unit) to receive:
/// @param  pu32ActualSize  \b OUT: actual msg size (byte unit) received
/// @param  u32WaitMs       \b IN: 0 ~ MSOS_WAIT_FOREVER: suspend time (ms) if the queue is empty
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_RecvFromQueue (MS_S32 s32QueueId, MS_U8 *pu8Message, MS_U32 u32IntendedSize, MS_U32 *pu32ActualSize, MS_U32 u32WaitMs)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Receive a message from the specified queue
/// @param  s32QueueId      \b IN: Queue ID
/// @param  pu8Message      \b OUT: msg destination
/// @param  u32IntendedSize \b IN: intended msg size (byte unit) to receive:
/// @param  pu32ActualSize  \b OUT: actual msg size (byte unit) received
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_PeekFromQueue (MS_S32 s32QueueId, MS_U8 *pu8Message, MS_U32 u32IntendedSize, MS_U32 *pu32ActualSize)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
// Get a Queue information
// @param  s32QueueId      \b IN: Queue ID
// @param  pu32QueueSize   \b OUT: ptr to queue size (DW)
// @param  pu32MessageSize \b OUT: ptr to message size (DW)
// @param  peAttribute     \b OUT: ptr to suspended mode: E_MSOS_FIFO / E_MSOS_PRIORITY
// @param  pQueueName      \b OUT: ptr to Queue name
// @return TRUE : succeed
// @return FALSE : the Queue has not been created
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_InfoQueue ( MS_S32 s32QueueId, MS_U32 *pu32QueueSize, MS_U32 *pu32MessageSize,
                      MsOSAttribute *peAttribute, char *pQueueName)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//
// Interrupt management
//
//-------------------------------------------------------------------------------------------------
/// Attach the interrupt callback function to interrupt #
/// @param  eIntNum \b IN: Interrupt number in enumerator InterruptNum
/// @param  pIntCb  \b IN: Interrupt callback function
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_AttachInterrupt (InterruptNum eIntNum, InterruptCb pIntCb)
{
    MS_BOOL u8Err = FALSE;

    atomMutexGet(&_MsOS_HISR_Mutex, 0);
    u8Err = CHIP_AttachISR(eIntNum, pIntCb);
    atomMutexPut(&_MsOS_HISR_Mutex);
    return u8Err;
}

//-------------------------------------------------------------------------------------------------
/// Detach the interrupt callback function from interrupt #
/// @param  eIntNum \b IN: Interrupt number in enumerator InterruptNum
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DetachInterrupt (InterruptNum eIntNum)
{
    MS_BOOL u8Err = FALSE;

    atomMutexGet(&_MsOS_HISR_Mutex, 0);
    u8Err = CHIP_DetachISR(eIntNum);
    atomMutexPut(&_MsOS_HISR_Mutex);
    return u8Err;
}

//-------------------------------------------------------------------------------------------------
/// Enable (unmask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number in enumerator InterruptNum
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_EnableInterrupt (InterruptNum eIntNum)
{
    return CHIP_EnableIRQ(eIntNum);
}

//-------------------------------------------------------------------------------------------------
/// Disable (mask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number in enumerator InterruptNum
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_DisableInterrupt (InterruptNum eIntNum)
{
    return CHIP_DisableIRQ(eIntNum);
}

//-------------------------------------------------------------------------------------------------
/// Disable all interrupts (including timer interrupt), the scheduler is disabled.
/// @return Interrupt register value before all interrupts disable
//-------------------------------------------------------------------------------------------------
MS_U32 MsOS_DisableAllInterrupts(void)
{
    return (MS_U32) MSCPU_OSCPUSaveSR();
}

//-------------------------------------------------------------------------------------------------
/// Restore the interrupts from last MsOS_DisableAllInterrupts.
/// @param  u32OldInterrupts \b IN: Interrupt register value from @ref MsOS_DisableAllInterrupts
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_RestoreAllInterrupts(MS_U32 u32OldInterrupts)
{
    MSCPU_OSCPURestoreSR(u32OldInterrupts);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Enable all CPU interrupts.
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_EnableAllInterrupts(void)
{
    MSCPU_OSCPUEnableSR();
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// In Interuupt Context or not
/// @return TRUE : Yes
/// @return FALSE : No
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_In_Interrupt (void)
{
    return CHIP_InISRContext();
}

//-------------------------------------------------------------------------------------------------
/// Write back if dirty & Invalidate the cache lines in the given range
/// @param  u32Start \b IN: start address (must be 16-B aligned and in cacheable area)
/// @param  u32Size  \b IN: size (must be 16-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail due to invalide parameter
//-------------------------------------------------------------------------------------------------
#define DCACHE_LINE_SIZE 16
extern void DCACHE_vFlushAddr(U32 wStartAddr, U32 wEndAddr);
extern void DCACHE_vInvalidateAddr(U32 wStartAddr, U32 wEndAddr);
MS_BOOL MsOS_Dcache_Flush( MS_VIRT ptrStart, MS_SIZE tSize  )
{
    MS_U32 u32OldIntr;
    MS_U32 u32StartAddr;
    MS_U32 u32EndAddr;

    u32StartAddr = ptrStart & 0xFFFFFFF0;
    tSize+= (ptrStart - u32StartAddr);
    tSize = ALIGN_16(tSize);

    if ((u32StartAddr % DCACHE_LINE_SIZE) || ((u32StartAddr & (MS_U32)KSEG1_BASE) != (MS_U32)KSEG0_BASE))
    {
        return FALSE;
    }
    u32EndAddr = u32StartAddr+tSize;
    u32OldIntr = MsOS_DisableAllInterrupts();
    DCACHE_vFlushAddr(u32StartAddr, u32EndAddr);
    //flush EC's write FIFO
    MAsm_CPU_Sync();
    MsOS_RestoreAllInterrupts(u32OldIntr);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Invalidate the cache lines in the given range
/// @param  u32Start \b IN: start address (must be 16-B aligned and in cacheable area)
/// @param  u32Size  \b IN: size (must be 16-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail due to invalide parameter
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_Dcache_Invalidate( MS_VIRT ptrStart , MS_SIZE tSize )
{
    MS_U32 u32OldIntr;
    MS_U32 u32StartAddr;
    MS_U32 u32EndAddr;

    u32StartAddr = ptrStart & 0xFFFFFFF0;
    tSize+= (ptrStart - u32StartAddr);
    tSize = ALIGN_16(tSize);
    if ( (ptrStart % DCACHE_LINE_SIZE) || ( (ptrStart & (MS_U32)KSEG1_BASE) != (MS_U32)KSEG0_BASE) )
    {
        return FALSE;
    }
    u32EndAddr = u32StartAddr + tSize;
    u32OldIntr = MsOS_DisableAllInterrupts();
    DCACHE_vInvalidateAddr(u32StartAddr, u32EndAddr);
    //flush EC's write FIFO
    MAsm_CPU_Sync();
    MsOS_RestoreAllInterrupts(u32OldIntr);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Write back if dirty the cache lines in the given range
/// @param  u32Start \b IN: start address (must be 16-B aligned and in cacheable area)
/// @param  u32Size  \b IN: size (must be 16-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail due to invalide parameter
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_Dcache_Writeback( MS_VIRT u32Start, MS_SIZE u32Size )
{
    MS_U32 u32OldIntr;
    if ( (u32Start % DCACHE_LINE_SIZE) || ( (u32Start & (MS_U32)KSEG1_BASE) != (MS_U32)KSEG0_BASE) )
    {
        return FALSE;
    }
    u32OldIntr = MsOS_DisableAllInterrupts();
    printf("WARNING: this ecos doesn't enable the dcache_store\b");
//TODO:    HAL_DCACHE_STORE( u32Start, u32Size);
    //flush EC's write FIFO
    MAsm_CPU_Sync();
    MsOS_RestoreAllInterrupts(u32OldIntr);
    return TRUE;
}

MS_PHY MsOS_VA2PA(MS_VIRT addr)
{
#if (defined(MCU_AEON))
    #if (defined(CHIP_T3) || defined(CHIP_U3) || defined(CHIP_JANUS))
        return ((MS_U32)(addr) & ~(0xC0000000));
    #else
        return ((MS_U32)(addr) & ~(0x80000000));
    #endif
#else
    #if (!defined(CHIP_T2) && !defined(CHIP_U3))
        #if (defined(CHIP_T12) || defined(CHIP_A2) || defined(CHIP_T8) || defined(CHIP_J2) || defined(CHIP_A5) || defined(CHIP_A5P) || defined(CHIP_A3) || defined(CHIP_K2) || defined(CHIP_EIFFEL) || defined(CHIP_NIKE) || defined(CHIP_MADISON) || defined(CHIP_CLIPPERS) || defined(CHIP_MIAMI) || defined(CHIP_NADAL) || defined(CHIP_MUNICH))
            // miu0 (low 256MB) (VA)0x8000:0000~0x9000:0000 --> (PA)0x0000:0000~0x1000:0000
            // miu0 (low 256MB) (VA)0xA000:0000~0xB000:0000 --> (PA)0x0000:0000~0x1000:0000
            if ((0x80000000 <= addr) && (0x90000000 > addr))
            {
                return (addr & 0x1fffffff);
            }
            if ((0xA0000000 <= addr) && (0xB0000000 > addr))
            {
                return (addr & 0x1fffffff);
            }
            // miu0 (high 256MB) (VA)0xC000:0000~0xD000:0000 -> (PA)0x1000:0000~0x2000:0000
            // miu0 (high 256MB) (VA)0xD000:0000~0xE000:0000 -> (PA)0x1000:0000~0x2000:0000
            if ((0xC0000000 <= addr) && (0xD0000000 > addr))
            {
                return ((addr - 0xC0000000) + 0x10000000);
            }
            if ((0xD0000000 <= addr) && (0xE0000000 > addr))
            {
                return ((addr - 0xD0000000) + 0x10000000);
            }
            // miu1 (low 256MB) (VA)0x0000:0000~0x1000:0000 -> (PA)0x2000:0000~0x3000:0000
            // miu1 (low 256MB) (VA)0x1000:0000~0x2000:0000 -> (PA)0x2000:0000~0x3000:0000
            if ((0x00000000 <= addr) && (0x10000000 > addr))
            {
                return (addr + 0x20000000);
            }
            if ((0x10000000 <= addr) && (0x20000000 > addr))
            {
                return (addr + 0x10000000);
            }
            // miu1 (high 256MB) (VA)0x2000:0000~0x3000:0000 -> (PA)0x3000:0000~0x4000:0000
            // miu1 (high 256MB) (VA)0x3000:0000~0x4000:0000 -> (PA)0x3000:0000~0x4000:0000
            if ((0x20000000 <= addr) && (0x30000000 > addr))
            {
                return (addr + 0x10000000);
            }
            if ((0x30000000 <= addr) && (0x40000000 > addr))
            {
                return addr;
            }
            return 0;
        #elif (defined(CHIP_K1)||defined(CHIP_KELTIC))
            // K1 has only 1 MIU
            // miu0
            if ((0x80000000 <= addr) && (0x90000000 > addr))
            {
                return ((MS_U32)(((MS_U32)addr) & 0x1fffffff));
            }
            if ((0xA0000000 <= addr) && (0xB0000000 > addr))
            {
                return ((MS_U32)(((MS_U32)addr) & 0x1fffffff));
            }

            return 0;
         #elif(defined(CHIP_K6) || defined(CHIP_K6LITE)|| (defined(CHIP_K7U)))
            // miu0 (low 256MB) (VA)0x0000:0000~0x1000:0000 --> (PA)0x0000:0000~0x1000:0000
            // miu0 (low 256MB) (VA)0x2000:0000~0x3000:0000 --> (PA)0x0000:0000~0x1000:0000
            if ((0x00000000 <= addr) && (0x10000000 > addr))
            {
                return (addr);
            }
            if ((0x20000000 <= addr) && (0x30000000 > addr))
            {
                return (addr & 0x0FFFFFFF);
            }
            // miu0 (high 256MB) (VA)0x1000:0000~0x2000:0000 -> (PA)0x1000:0000~0x2000:0000
            // miu0 (high 256MB) (VA)0x3000:0000~0x4000:0000 -> (PA)0x1000:0000~0x2000:0000
            if ((0x10000000 <= addr) && (0x20000000 > addr))
            {
                return (addr);
            }
            if ((0x30000000 <= addr) && (0x40000000 > addr))
            {
                return (addr - 0x20000000);
            }
            // miu1 (low 256MB) (VA)0x4000:0000~0x5000:0000 -> (PA)0x8000:0000~09000:0000
            // miu1 (low 256MB) (VA)0x6000:0000~0x7000:0000 -> (PA)0x8000:0000~0x9000:0000
            if ((0x40000000 <= addr) && (0x50000000 > addr))
            {
                return (addr + 0x40000000);
            }
            if ((0x60000000 <= addr) && (0x70000000 > addr))
            {
                return (addr + 0x20000000);
            }
            // miu1 (high 256MB) (VA)0x5000:0000~0x6000:0000 -> (PA)0x9000:0000~0xA000:0000
            // miu1 (high 256MB) (VA)0x7000:0000~0x8000:0000 -> (PA)0x9000:0000~0xA000:0000
            if ((0x50000000 <= addr) && (0x60000000 > addr))
            {
                return (addr + 0x40000000);
            }
            if ((0x70000000 <= addr) && (0x80000000 > addr))
            {
                return  (addr + 0x20000000);
            }
            return 0;
        #elif(defined(CHIP_KANO)) // Kano : 1G *512 solution
            // miu0 (0~256MB) (VA)0x0000:0000~0x1000:0000 --> (PA)0x0000:0000~0x1000:0000
            // miu0 (0~256MB) (VA)0x4000:0000~0x5000:0000 --> (PA)0x0000:0000~0x1000:0000
            if ((0x00000000 <= addr) && (0x10000000 > addr))
            {
                return (addr);
            }
            if ((0x40000000 <= addr) && (0x50000000 > addr))
            {
                return (addr - 0x40000000);
            }
            // miu0 (256MB~512MB) (VA)0x1000:0000~0x2000:0000 -> (PA)0x1000:0000~0x2000:0000
            // miu0 (256MB~512MB) (VA)0x5000:0000~0x6000:0000 -> (PA)0x1000:0000~0x2000:0000
            if ((0x10000000 <= addr) && (0x20000000 > addr))
            {
                return (addr);
            }
            if ((0x50000000 <= addr) && (0x60000000 > addr))
            {
                return (addr - 0x40000000);
            }
            // miu0 (512~768MB) (VA)0x2000:0000~0x3000:0000 -> (PA)0x2000:0000~0x3000:0000
            // miu0 (512~768MB) (VA)0x6000:0000~0x7000:0000 -> (PA)0x2000:0000~0x3000:0000
            if ((0x20000000 <= addr) && (0x30000000 > addr))
            {
                return (addr);
            }
            if ((0x60000000 <= addr) && (0x70000000 > addr))
            {
                return (addr - 0x40000000);
            }
            // miu0 (768MB~1024MB) (VA)0x3000:0000~0x4000:0000 -> (PA)0x3000:0000~0x4000:0000
            // miu0 (768MB~1024MB) (VA)0x7000:0000~0x8000:0000 -> (PA)0x3000:0000~0x4000:0000
            if ((0x30000000 <= addr) && (0x40000000 > addr))
            {
                return (addr);
            }
            if ((0x70000000 <= addr) && (0x80000000 > addr))
            {
                return (addr - 0x40000000);
            }
            // miu1 (low 256MB) (VA)0x8000:0000~0x9000:0000 -> (PA)0x8000:0000~09000:0000
            // miu1 (low 256MB) (VA)0xA000:0000~0xB000:0000 -> (PA)0x8000:0000~0x9000:0000
            if ((0x80000000 <= addr) && (0x90000000 > addr))
            {
                return (addr);
            }
            if ((0xA0000000 <= addr) && (0xB0000000 > addr))
            {
                return (addr - 0x20000000);
            }
            // miu1 (high 256MB) (VA)0x9000:0000~0xA000:0000 -> (PA)0x9000:0000~0xA000:0000
            // miu1 (high 256MB) (VA)0xB000:0000~0xC000:0000 -> (PA)0x9000:0000~0xA000:0000
            if ((0x90000000 <= addr) && (0xA0000000 > addr))
            {
                return (addr);
            }
            if ((0xB0000000 <= addr) && (0xC0000000 > addr))
            {
                return  (addr - 0x20000000);
            }
        #else
            // miu0
            if ((0x80000000 <= addr) && (0x90000000 > addr))
            {
                return ((MS_U32)(((MS_U32)addr) & 0x1fffffff));
            }
            if ((0xA0000000 <= addr) && (0xB0000000 > addr))
            {
                return ((MS_U32)(((MS_U32)addr) & 0x1fffffff));
            }
            // miu1
            if ((MIU1_CACHEABLE_START <= addr) && (MIU1_CACHEABLE_END > addr))
            {
                return ((addr & ~MIU1_CACHEABLE_START) | (HAL_MIU1_BASE));
            }
            if ((MIU1_UNCACHEABLE_START <= addr) && (MIU1_UNCACHEABLE_END > addr))
            {
                return ((MS_U32)(((MS_U32)(addr & ~MIU1_UNCACHEABLE_START)) | (HAL_MIU1_BASE)));
            }
            return 0;
        #endif
    #else
        return ((MS_U32)(((MS_U32)addr) & 0x1fffffff));
    #endif
#endif
    return FALSE;
}

MS_BOOL MsOS_MPool_IsPA2KSEG1Mapped(MS_PHY pAddrPhys) // un-cache
{
    MS_BOOL bRet = ( MsOS_PA2KSEG1(pAddrPhys) == 0 ? FALSE : TRUE );
    return bRet;
}

MS_BOOL MsOS_MPool_IsPA2KSEG0Mapped(MS_PHY pAddrPhys) // cache
{
    MS_BOOL bRet = ( MsOS_PA2KSEG0(pAddrPhys) == 0 ? FALSE : TRUE );
    return bRet;
}

MS_VIRT MsOS_PA2KSEG0(MS_PHY addr)
{
#if (defined(MCU_AEON))
    return (addr);
#else // for mips and refine it later
    #if (!defined(CHIP_T2) && !defined(CHIP_U3))
        #if (defined(CHIP_T12) || defined(CHIP_A2) || defined(CHIP_T8) || defined(CHIP_J2) || defined(CHIP_A5) || defined(CHIP_A5P) || defined(CHIP_A3) || defined(CHIP_K2) || defined(CHIP_EIFFEL) || defined(CHIP_NIKE) || defined(CHIP_MADISON) || defined(CHIP_CLIPPERS) || defined(CHIP_MIAMI) || defined(CHIP_NADAL) || defined(CHIP_MUNICH))
            // miu0 (low 256MB) - (PA)0x0000:0000~0x1000:0000 -> (VA)0x8000:0000~0x9000:0000 cached
            if ((0x00000000 <= addr) && (0x10000000 > addr))
            {
                return (addr + 0x80000000);
            }
            // miu0 (high 256MB) - (PA)0x1000:0000~0x2000:0000 -> (VA)0xC000:0000~0xD000:0000 cached
            if ((0x10000000 <= addr) && (0x20000000 > addr))
            {
                return ((addr - 0x10000000) + 0xC0000000);
            }
            // miu1 (low 256MB) - (PA)0x2000:0000~0x3000:0000 -> (VA)0x0000:0000~0x1000:0000 cached
            if ((0x20000000 <= addr) && (0x30000000 > addr))
            {
                return (addr - 0x20000000);
            }
            // miu1 (high 256MB) - (PA)0x3000:0000~0x4000:0000 -> (VA)0x2000:0000~0x3000:0000 cached
            if ((0x30000000 <= addr) && (0x40000000 > addr))
            {
                return (addr - 0x10000000);
            }
            return 0;
        #elif (defined(CHIP_K1) || defined(CHIP_KELTIC))
            // miu0
            if ((0x00000000 <= addr) && (HAL_MIU1_BASE > addr))
            {
                return ((MS_U32)(((MS_U32)addr) | 0x80000000));
            }
            return 0;
        #elif(defined(CHIP_K6) ||defined(CHIP_K6LITE)||defined(CHIP_K7U))
            // miu0 (low 256MB) - (PA)0x0000:0000~0x1000:0000 -> (VA)0x0000:0000~0x1000:0000 cached
            if ((0x00000000 <= addr) && (0x10000000 > addr))
            {
                return (addr);
            }
            // miu0 (high 256MB) - (PA)0x1000:0000~0x2000:0000 -> (VA)0x1000:0000~0x2000:0000 cached
            if ((0x10000000 <= addr) && (0x20000000 > addr))
            {
                return (addr);
            }
            // miu1 (low 256MB) - (PA)0x8000:0000~0x9000:0000 -> (VA)0x4000:0000~0x5000:0000 cached
            if ((0x80000000 <= addr) && (0x90000000 > addr))
            {
                return (addr - 0x40000000);
            }
            // miu1 (high 256MB) - (PA)0x9000:0000~0xA000:0000 -> (VA)0x5000:0000~0x6000:0000 cached
            if ((0x90000000 <= addr) && (0xA0000000 > addr))
            {
                return (addr - 0x40000000);
            }
            return 0;
        #elif defined(CHIP_KANO) // Kano : 1G * 512 solution
            // miu0 (0~1024MB) - (PA)0x0000:0000~0x4000:0000 -> (VA)0x0000:0000~0x4000:0000 cached
            if ((0x00000000 <= addr) && (0x40000000 > addr))
            {
                return (addr);
            }
            // miu1 (low 256MB) - (PA)0x8000:0000~0x9000:0000 -> (VA)0x8000:0000~0x9000:0000 cached
            if ((0x80000000 <= addr) && (0x90000000 > addr))
            {
                return (addr);
            }
            // miu1 (high 256MB) - (PA)0x9000:0000~0xA000:0000 -> (VA)0x9000:0000~0xA000:0000 cached
            if ((0x90000000 <= addr) && (0xA0000000 > addr))
            {
                return (addr);
            }
            return 0;
        #else
            // miu0
            if ((0x00000000 <= addr) && (HAL_MIU1_BASE > addr))
            {
                return ((MS_U32)(((MS_U32)addr) | 0x80000000));
            }
            // miu1
            if ((HAL_MIU1_BASE <= addr) && ((HAL_MIU1_BASE * 2) > addr))
            {
                return ((addr & ~(HAL_MIU1_BASE)) | 0xC0000000);
            }
            return 0;
        #endif
    #else
        return ((MS_U32)(((MS_U32)addr) | 0x80000000));
    #endif
#endif
}

MS_VIRT MsOS_PA2KSEG1(MS_PHY addr)
{
#if (defined(MCU_AEON))
    #if (defined(CHIP_T3) || defined(CHIP_U3))
        return ((MS_U32)(addr) | (0xC0000000));
    #else
        return ((MS_U32)(addr) | (0x80000000));
    #endif
#else // for mips and refine it later
    #if (!defined(CHIP_T2) && !defined(CHIP_U3))
        #if (defined(CHIP_T12) || defined(CHIP_A2) || defined(CHIP_T8) || defined(CHIP_J2) || defined(CHIP_A5) || defined(CHIP_A5P) || defined(CHIP_A3) || defined(CHIP_K2) || defined(CHIP_EIFFEL) || defined(CHIP_NIKE) || defined(CHIP_MADISON) || defined(CHIP_CLIPPERS) || defined(CHIP_MIAMI) || defined(CHIP_NADAL) || defined(CHIP_MUNICH))
            // miu0 (low 256MB) - (PA)0x0000:0000~0x1000:0000 -> 0xA000:0000~0xB000:0000 uncached
            if ((0x00000000 <= addr) && (0x10000000 > addr))
            {
                return (addr + 0xA0000000);
            }
            // miu0 (high 256MB) - (PA)0x1000:0000~0x2000:0000 -> 0xD000:0000~0xE000:0000 uncached
            if ((0x10000000 <= addr) && (0x20000000 > addr))
            {
                return ((addr - 0x10000000) + 0xD0000000);
            }
            // miu1 (low 256MB) - (PA)0x2000:0000~0x3000:0000 -> 0x1000:0000~0x2000:0000 uncached
            if ((0x20000000 <= addr) && (0x30000000 > addr))
            {
                return (addr - 0x10000000);
            }
            // miu1 (high 256MB) - (PA)0x3000:0000~0x4000:0000 -> 0x3000:0000~0x4000:0000 uncached
            if ((0x30000000 <= addr) && (0x40000000 > addr))
            {
                return addr;
            }
            return 0;
        #elif (defined(CHIP_K1) || defined(CHIP_KELTIC))
            // miu0
            if ((0x00000000 <= addr) && (HAL_MIU1_BASE > addr))
            {
                return ((MS_U32)(((MS_U32)addr) | 0xa0000000));
            }
            return 0;
        #elif (defined(CHIP_K6) ||defined(CHIP_K6LITE)||defined(CHIP_K7U))
            // miu0 (low 256MB) - (PA)0x0000:0000~0x1000:0000 -> 0x2000:0000~0x3000:0000 uncached
            if ((0x00000000 <= addr) && (0x10000000 > addr))
            {
                return (addr + 0x20000000);
            }
            // miu0 (high 256MB) - (PA)0x1000:0000~0x2000:0000 -> 0x3000:0000~0x4000:0000 uncached
            if ((0x10000000 <= addr) && (0x20000000 > addr))
            {
                return (addr + 0x20000000);
            }
            // miu1 (low 256MB) - (PA)0x8000:0000~0x9000:0000 -> 0x6000:0000~0x7000:0000 uncached
            if ((0x80000000 <= addr) && (0x90000000 > addr))
            {
                return (addr - 0x20000000);
            }
            // miu1 (high 256MB) - (PA)0x9000:0000~0xA000:0000 -> 0x7000:0000~0x8000:0000 uncached
            if ((0x90000000 <= addr) && (0xA0000000 > addr))
            {
                return (addr - 0x20000000);
            }
            return 0;
        #elif defined(CHIP_KANO) // Kano : 1G * 512 solution
            // miu0 (0~1024MB) - (PA)0x0000:0000~0x4000:0000 -> 0x4000:0000~0x8000:0000 uncached
            if ((0x00000000 <= addr) && (0x40000000 > addr))
            {
                return (addr + 0x40000000);
            }
            // miu1 (low 256MB) - (PA)0x8000:0000~0x9000:0000 -> 0xA000:0000~0xB000:0000 uncached
            if ((0x80000000 <= addr) && (0x90000000 > addr))
            {
                return (addr + 0x20000000);
            }
            // miu1 (high 256MB) - (PA)0x9000:0000~0xA000:0000 -> 0xB000:0000~0xC000:0000 uncached
            if ((0x90000000 <= addr) && (0xA0000000 > addr))
            {
                return (addr + 0x20000000);
            }
            return 0;
        #else
            // miu0
            if ((0x00000000 <= addr) && (HAL_MIU1_BASE > addr))
            {
                return ((MS_U32)(((MS_U32)addr) | 0xa0000000));
            }
            // miu1
            if ((HAL_MIU1_BASE <= addr) && ((HAL_MIU1_BASE * 2) > addr))
            {
                return ((addr & ~(HAL_MIU1_BASE)) | 0xD0000000);
            }
            return 0;
        #endif
    #else
        return ((MS_U32)(((MS_U32)addr) | 0xa0000000));
    #endif
#endif
}

#define SHM_SIZE                (200*1024)
#define MAX_CLIENT_NUM          200
typedef struct
{
    MS_U8       u8ClientName[MAX_CLIENT_NAME_LENGTH+ 1];
    MS_U32      u32Offset;
    MS_U32      u32Size;
    MS_U8       u8ClientId;    // 0 means "never used"
    MS_U8       u8RefCnt;
    MS_U8       u8Dummy[2];
} MsOS_SHM_Context;

typedef struct
{
    MS_U32              u32MaxClientNum;
    MS_U32              u32ClientNum;
    MS_U32              u32ShmSize;
    MS_U32              u32Offset;
    MsOS_SHM_Context    context[MAX_CLIENT_NUM];
} MsOS_SHM_Hdr;

static int _shm_id = -1;
static MS_U8 *_pu8ShareMem = NULL;
static MsOS_SHM_Hdr _ShmHdr; // dummy storage

// Share memory operation
// MS_BOOL MsOS_SHM_Init(MS_U32 u32ShmSize)
MS_BOOL MsOS_SHM_Init(void)
{
    MS_BOOL bInit = FALSE;
    MS_U32 u32ShmSize;

    if (-1 != _shm_id)
    {
        return FALSE;
    }

    u32ShmSize = SHM_SIZE;
    u32ShmSize += sizeof(MsOS_SHM_Hdr);
    u32ShmSize += ((1<< 12)- 1);
    u32ShmSize = (u32ShmSize>> 12)<< 12; // make it 4KBytes alignment

    if (NULL == (_pu8ShareMem = (MS_U8*)malloc(u32ShmSize)))
    {
        printf("[%s][%d] fail, ask for 0x%X mem_size\n", __FUNCTION__, __LINE__, (unsigned int)u32ShmSize);
        return FALSE;
    }
    bInit = TRUE;
    if (bInit)
    {
        memset(&_ShmHdr, 0, sizeof(_ShmHdr)); // dummy storage
        _ShmHdr.u32MaxClientNum = MAX_CLIENT_NUM;
        _ShmHdr.u32ClientNum = 0;
        _ShmHdr.u32ShmSize = u32ShmSize;
        _ShmHdr.u32Offset = (sizeof(MsOS_SHM_Hdr)+7)&~7;
        memcpy(_pu8ShareMem, &_ShmHdr, sizeof(_ShmHdr));
    }
    return TRUE;
}

// Share memory operation
MS_BOOL MsOS_SHM_GetId(MS_U8* pu8ClientName, MS_U32 u32BufSize, MS_U32* pu32ShmId, MS_VIRT* pu32Addr, MS_U32* pu32BufSize, MS_U32 u32Flag)
{
    MsOS_SHM_Context* pContext = NULL;
    MsOS_SHM_Context* pClient = NULL;
    MS_U32 i;
    MS_U32 u32CopyLen;

    if (!_pu8ShareMem)
    {
        printf("[%s][%d] MsOS_SHM_Init should be invoked first\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    memcpy(&_ShmHdr, _pu8ShareMem, sizeof(_ShmHdr));
    pContext = (MsOS_SHM_Context*)_ShmHdr.context;
    for (i= 0; i< MAX_CLIENT_NUM; i++, pContext++)
    {
        if (0 == pContext->u8ClientId)
        {
            continue;
        }
        if (0== strcmp((const char*)pContext->u8ClientName, (const char*)pu8ClientName))
        {
            pClient = pContext;
            if (u32BufSize != pClient->u32Size)
            {
                printf("[%s][%d] MsOS_SHM_GetId: inconsistent buffer size with other process\n", __FUNCTION__, __LINE__);
            }
            break;
        }
    }
    if ((NULL == pClient) && (MSOS_SHM_CREATE == u32Flag))
    {
        pContext = (MsOS_SHM_Context*)_ShmHdr.context;
        for (i= 0; i< MAX_CLIENT_NUM; i++, pContext++)
        {
            if (pContext->u8ClientId)
            {
                continue;
            }
            if ((_ShmHdr.u32Offset + u32BufSize)> _ShmHdr.u32ShmSize)
            {
                printf("[%s][%d] MsOS_SHM_GetId: shared memory buffer overflow\n", __FUNCTION__, __LINE__);
                return FALSE;
            }
            _ShmHdr.u32ClientNum++;
            pClient = pContext;
            u32CopyLen = MIN((int)strlen((const char*)pu8ClientName), MAX_CLIENT_NAME_LENGTH);
            strncpy((char*)pClient->u8ClientName, (const char*)pu8ClientName, u32CopyLen);
            pClient->u32Size = u32BufSize;
            pClient->u8ClientId = i + 1;
            pClient->u8RefCnt = 0;
            pClient->u32Offset = _ShmHdr.u32Offset;
            _ShmHdr.u32Offset += (u32BufSize+7)&~7;
            memcpy(_pu8ShareMem, &_ShmHdr, sizeof(_ShmHdr));
            break;
        }
    }
    if (NULL == pClient)
    {
        if(MSOS_SHM_QUERY != u32Flag)
        {
            printf("[%s][%d] MsOS_SHM_Init: Unable to get available share memeory\n", __FUNCTION__, __LINE__);
        }
        else
        {
            //printf("[%s][%d] MsOS_SHM_Init: Unable to query share memeory[%s]\n", __FUNCTION__, __LINE__, (char *)pu8ClientName);
        }
        return FALSE;
    }

    *pu32ShmId = pClient->u8ClientId;
    *pu32BufSize = pClient->u32Size;
    *pu32Addr = (MS_VIRT)_pu8ShareMem + pClient->u32Offset;
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Disable the CPU interrupt
/// @return Interrupt register value before all interrupts disable
//-------------------------------------------------------------------------------------------------
MS_U32 MsOS_CPU_DisableInterrupt (void)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Enable the CPU interrupt
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_EnableInterrupt (void)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Restore the CPU interrupt from last MsOS_CPU_DisableInterrupts.
/// @param  u32OldInterrupts \b IN: Interrupt register value from @ref MsOS_CPU_DisableInterrupts
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_RestoreInterrupt (MS_U32 u32OldInterrupts)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Mask all the CPU interrupt
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_MaskAllInterrupt (void)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Mask the CPU interrupt
/// @param  intr_num \b IN: Interrupt number in enumerator MHAL_INTERRUPT_TYPE
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_MaskInterrupt (MHAL_INTERRUPT_TYPE intr_num)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// UnMask the CPU interrupt
/// @param  intr_num \b IN: Interrupt number in enumerator MHAL_INTERRUPT_TYPE
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_UnMaskInterrupt (MHAL_INTERRUPT_TYPE intr_num)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Lock the CPU interrupt
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_LockInterrupt (void)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// UnLock the CPU interrupt
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_UnLockInterrupt (void)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Attach the CPU interrupt callback function to interrupt #
/// @param  eIntNum \b IN: Interrupt number in enumerator InterruptNum
/// @param  pIntCb  \b IN: Interrupt callback function
/// @param  dat  \b IN: Data
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_AttachInterrupt (MHAL_INTERRUPT_TYPE intr_num, mhal_isr_t isr, MS_U32 dat)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Detach the CPU interrupt callback function to interrupt #
/// @param  eIntNum \b IN: Interrupt number in enumerator InterruptNum
/// @param  pIntCb  \b IN: Interrupt callback function
/// @param  dat  \b IN: Data
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_DetachInterrupt (MHAL_INTERRUPT_TYPE intr_num)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Attach the CPU exception callback function to interrupt #
/// @param  eIntNum \b IN: Exception number in enumerator InterruptNum
/// @param  pIntCb  \b IN: Exception callback function
/// @param  dat  \b IN: Data
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_AttachException (MHAL_EXCEPTION_TYPE expt_num, mhal_isr_t isr, MS_U32 dat)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Detach the CPU exception callback function to interrupt #
/// @param  eIntNum \b IN: Exception number in enumerator InterruptNum
/// @param  pIntCb  \b IN: Exception callback function
/// @param  dat  \b IN: Data
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_DetachException (MHAL_EXCEPTION_TYPE expt_num)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}
//-------------------------------------------------------------------------------------------------
/// Set EBASE
/// @param  u32Addr \b IN: MIPS Code Start Address
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
MS_BOOL MsOS_CPU_SetEBASE (MS_U32 u32Addr)
{
    printf("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}
//-------------------------------------------------------------------------------------------------
/// Sync data in EC-Brigde
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
void MsOS_Sync(void)
{
    MAsm_CPU_Sync();
}

void MsOS_FlushMemory(void)
{
    HAL_MMIO_FlushMemory();
}

void MsOS_ReadMemory(void)
{
    HAL_MMIO_ReadMemory();
}

MS_U32 MsOS_GetKattribute(char *pAttr)
{
    // This is for linux only, do nothing here
    return 0;
}
#endif
