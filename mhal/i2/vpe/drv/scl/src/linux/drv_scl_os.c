////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (?œMStar Confidential Information?? by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
///

#define __DRV_SCL_OS_C__

/******************************************************************************/
/*                           Header Files                                     */
/* ****************************************************************************/
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/clk-private.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>          /* seems do not need this */
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include "ms_platform.h"
#include "ms_msys.h"

#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"
/********************************************************************************/
/*                           Macro                                              */
/********************************************************************************/
#define SCLOS_MUTEX_USE_SEM

#define SCLOS_ID_PREFIX              0x76540000
#define SCLOS_ID_PREFIX_MASK         0xFFFF0000
#define SCLOS_ID_MASK                0x0000FFFF //~SCLOS_ID_PREFIX_MASK

#define HAS_FLAG(flag, bit)        ((flag) & (bit))
#define SET_FLAG(flag, bit)        ((flag)|= (bit))
#define RESET_FLAG(flag, bit)      ((flag)&= (~(bit)))


#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif // #ifndef MIN

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define SCL_IRQID_Default 0xFB
#define CMDQ_IRQID_Default 0x51
static                          DEFINE_SPINLOCK(_SclOs_Mutex_Mutex);
#define MUTEX_MUTEX_LOCK()      spin_lock(&_SclOs_Mutex_Mutex)
#define MUTEX_MUTEX_UNLOCK()    spin_unlock(&_SclOs_Mutex_Mutex)

#ifdef SCLOS_MUTEX_USE_SEM
struct semaphore _SclOs_Task_Mutex;
#define MUTEX_TASK_LOCK()    down(&_SclOs_Task_Mutex)
#define MUTEX_TASK_UNLOCK()  up(&_SclOs_Task_Mutex)
#else
static                       DEFINE_SPINLOCK(_SclOs_Task_Mutex);
#define MUTEX_TASK_LOCK()    spin_lock(&_SclOs_Task_Mutex)
#define MUTEX_TASK_UNLOCK()  spin_unlock(&_SclOs_Task_Mutex)
#endif
static                          DEFINE_SPINLOCK(_SclOs_EventGroup_Mutex);
#define EVENT_MUTEX_LOCK()      spin_lock(&_SclOs_EventGroup_Mutex)
#define EVENT_MUTEX_UNLOCK()    spin_unlock(&_SclOs_EventGroup_Mutex)



static                          DEFINE_SPINLOCK(_SclOs_Timer_Mutex);
#define TIMER_MUTEX_LOCK()      spin_lock(&_SclOs_Timer_Mutex)
#define TIMER_MUTEX_UNLOCK()    spin_unlock(&_SclOs_Timer_Mutex)

/********************************************************************************/
/*                           Constant                                           */
/********************************************************************************/
typedef struct
{
    bool             bUsed;
#ifdef SCLOS_MUTEX_USE_SEM
    struct semaphore stMutex;
#else
    spinlock_t       stMutex;
#endif
    u8               u8Name[SCLOS_MUTEX_NAME_LENGTH];
} DrvSclOsMutexInfoConfig_t;
typedef struct
{
    bool             bUsed;
    spinlock_t       stMutex;
    u8               u8Name[SCLOS_MUTEX_NAME_LENGTH];
    u32              u32flag;
} DrvSclOsSpinlockInfoConfig_t;

//
// Event Group
//
typedef struct
{
    bool                     bUsed;
    u32                      u32EventGroup;
    spinlock_t                  stMutexEvent;
    wait_queue_head_t           stSemaphore;
} DrvSclOsEventGroupInfoConfig_t;
typedef struct
{
    bool                     bUsed;
    u32                      u32EventGroup;
    u8                       u8Wpoint;
    u8                       u8Rpoint;
} DrvSclOsRingEventGroupInfoConfig_t;
//
// Task Management
//
typedef struct
{
    bool             bUsed;
    struct task_struct* pstThreadInfo;
} DrvSclOsTaskInfoConfig_t;
typedef struct
{
    bool             bUsed;
    DrvSclOsWoorkQueueConfig_t*  pstWorkQueueInfo;
} DrvSclOsWorkQueueInfoConfig_t;
typedef struct
{
    bool             bUsed;
    DrvSclOsWorkConfig_t        stWorkEventInfo;
} DrvSclOsWorkEventInfoConfig_t;
typedef struct
{
    bool             bUsed;
    DrvSclOsTaskletConfig_t     stTaskletInfo;
} DrvSclOsTaskletInfoConfig_t;
//Clk
typedef struct
{
    bool bUsed;
    DrvSclOsClkConfig_t  stClk;
}DrvSclOsClkInfoConfig_t;

// Timer
//
typedef struct
{
    bool             bUsed;
    TimerCb             pTimerCb;
    struct timer_list   timer;
    int                 period;
    int                 first;
} DrvSclOsTimerInfoConfig_t;


/******************************************************************************/
/*                           Enum                                             */
/******************************************************************************/


/******************************************************************************/
/*                           Global Variables                                   */
/******************************************************************************/
//
// Mutex
//

MDrvSclCtxSclOsGlobalSet_t *gstGlobalSclOsSet;

/******************************************************************************/
/*                           Local Variables                                    */
/******************************************************************************/
//keep
static DrvSclOsMutexInfoConfig_t          _SclOs_Mutex_Info[SCLOS_MUTEX_MAX];
static DrvSclOsSpinlockInfoConfig_t       _SclOs_Spinlock_Info[SCLOS_SPINLOCK_MAX];
DrvSclOsAccessRegType_e gbAccessRegMode = E_DRV_SCLOS_AccessReg_CPU;
static DrvSclOsTaskInfoConfig_t        _SclOs_Task_Info[SCLOS_TASK_MAX];
#if 0
static DrvSclOsWorkQueueInfoConfig_t   _SclOs_WorkQueue_Info[SCLOS_WORKQUEUE_MAX];
static DrvSclOsWorkEventInfoConfig_t   _SclOs_WorkEvent_Info[SCLOS_WORK_MAX];
static DrvSclOsTaskletInfoConfig_t     _SclOs_Tasklet_Info[SCLOS_TASKLET_MAX];
#endif
static DrvSclOsEventGroupInfoConfig_t     _SclOs_EventGroup_Info[SCLOS_EVENTGROUP_MAX];
static DrvSclOsTimerInfoConfig_t  _SclOs_Timer_Info[SCLOS_TIMER_MAX];
u32 gSCLIRQID[E_DRV_SCLOS_SCLIRQ_MAX] = {SCL_IRQID_Default,(SCL_IRQID_Default+1),(SCL_IRQID_Default+2)}; //INT_I
void *gvpdev[E_DRV_SCLOS_DEV_MAX];
u32 gu32Init = 0;
bool gbInit = FALSE;
DrvSclOsAllocPhyMem_t gstSclOsAlloc;

/******************************************************************************/
/*               P r i v a t e    F u n c t i o n s                             */
/******************************************************************************/
void _DrvSclOsSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstGlobalSclOsSet = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stSclOsCfg);
}

u32 DrvSclOsGetSystemTime (void)
{
    struct timespec         ts;

    getnstimeofday(&ts);
    return ts.tv_sec* 1000+ ts.tv_nsec/1000000;
}
u64 DrvSclOsGetSystemTimeStamp (void)
{
    struct timespec         tv;
    u64 u64TimeStamp;
    getrawmonotonic(&tv);
    u64TimeStamp =(u64)tv.tv_sec* 1000000ULL+ (u64)(tv.tv_nsec/1000LL);
    return u64TimeStamp;
}

u32 DrvSclOsTimerDiffTimeFromNow(u32 u32TaskTimer) //unit = ms
{
    return (DrvSclOsGetSystemTime() - u32TaskTimer);
}

void DrvSclOsDelayTask (u32 u32Ms)
{
    //sleep in spinlock will cause deadlock
#ifdef SCLOS_MUTEX_USE_SEM
    msleep_interruptible((unsigned int)u32Ms);
#else
    mdelay(u32Ms);
#endif
}


void DrvSclOsDelayTaskUs (u32 u32Us)
{
        //sleep in spinlock will cause deadlock
    udelay(u32Us);
}
void DrvSclOsSetProbeInformation(DrvSclosProbeType_e enProbe)
{
    gu32Init |= (u32)enProbe;
    if(gu32Init == E_DRV_SCLOS_INIT_ALL)
    {
        SCL_ERR("[SCL] SCL init success\n");
        DrvSclOsSetAccessRegMode(E_DRV_SCLOS_AccessReg_CMDQ);
    }
}

void DrvSclOsClearProbeInformation(DrvSclosProbeType_e enProbe)
{
    gu32Init &= ~((u32)enProbe);
    if(gu32Init == E_DRV_SCLOS_INIT_NONE)
    {
        SCL_ERR("[SCL] SCL Remove success\n");
    }
    else if(gu32Init != E_DRV_SCLOS_INIT_ALL)
    {
        DrvSclOsSetAccessRegMode(E_DRV_SCLOS_AccessReg_CPU);
    }
}

u32 DrvSclOsGetProbeInformation(DrvSclosProbeType_e enProbe)
{
    return (gu32Init & (u32)enProbe);
}
void DrvSclOsSetPlatformDevice(void *pdev,DrvSclosDevType_e enDev)
{
    gvpdev[enDev] = pdev;
}
void *DrvSclOsGetPlatformDevice(DrvSclosDevType_e enDev)
{
    if(enDev!=E_DRV_SCLOS_DEV_MAX)
    {
        return gvpdev[enDev];
    }
    else
    {
        return NULL;
    }
}

void DrvSclOsGetCap(DrvSclOsGetCap_t *pstCaps)
{
#if defined(SCLOS_TYPE_LINUX_KERNEL_I2)
	pstCaps->nbFrmBufCam = 0x1;
	pstCaps->nbFrmBufDvr = 0x1;
	pstCaps->nbRealTime  = 0x1;
#else
	pstCaps->nbFrmBufCam = 0x0;
	pstCaps->nbFrmBufDvr = 0x0;
	pstCaps->nbRealTime  = 0x0;
#endif
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//
// Interrupt management
//
//-------------------------------------------------------------------------------------------------
/// Attach the interrupt callback function to interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @param  pIntCb  \b IN: Interrupt callback function
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsAttachInterrupt (u32 eIntNum, InterruptCb pIntCb,u32 flags,const char *name)
{
    int i;
    i = request_irq(eIntNum, (irq_handler_t)pIntCb, flags, name, NULL);

    return i;
}

//-------------------------------------------------------------------------------------------------
/// Detach the interrupt callback function from interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsDetachInterrupt (u32 eIntNum)
{
    // PTH_RET_CHK(pthread_mutex_lock(&_ISR_Mutex));
    //HAL_IRQ_Detech((int)eIntNum); TODo!!
    // PTH_RET_CHK(pthread_mutex_unlock(&_ISR_Mutex));
    free_irq(eIntNum, NULL);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Enable (unmask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsEnableInterrupt (u32 eIntNum)
{
    enable_irq((int)eIntNum);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Disable (mask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
bool DrvSclOsDisableInterrupt (u32 eIntNum)
{
    disable_irq((int)eIntNum);
    return TRUE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//
// Mutex
//
//-------------------------------------------------------------------------------------------------
/// Create a mutex in the unlocked state
/// @param  eAttribute  \b IN: E_DRV_SCLOS_FIFO: suspended in FIFO order
/// @param  pMutexName  \b IN: mutex name
/// @param  u32Flag  \b IN: process data shared flag
/// @return >=0 : assigned mutex Id
/// @return <0 : fail
/// @note   A mutex has the concept of an owner, whereas a semaphore does not.
///         A mutex provides priority inheritance protocol against proiorty inversion, whereas a binary semaphore does not.
//-------------------------------------------------------------------------------------------------
s32 DrvSclOsCreateMutex ( DrvSclOsAttributeType_e eAttribute, char *pMutexName1, u32 u32Flag)
{
    s32 s32Id, s32LstUnused = SCLOS_MUTEX_MAX;
    u8 pMutexName[SCLOS_MUTEX_NAME_LENGTH];
    u32 u32MaxLen;

    if (NULL == pMutexName1)
    {
        return -1;
    }
    if (strlen(pMutexName1) >= (SCLOS_MUTEX_NAME_LENGTH-1))
    {
        sclprintf("%s: Warning strlen(%s) is longer than SCLOS_MUTEX_NAME_LENGTH(%d). Oversize char will be discard.\n",
        __FUNCTION__,pMutexName1,SCLOS_MUTEX_NAME_LENGTH);
    }
    if (0 == (u32MaxLen = MIN(strlen(pMutexName1), (SCLOS_MUTEX_NAME_LENGTH-1))))
    {
        return -1;
    }

    strncpy((char*)pMutexName, (const char*)pMutexName1, u32MaxLen);
    pMutexName[u32MaxLen] = '\0';

    MUTEX_MUTEX_LOCK();
    for(s32Id=0;s32Id<SCLOS_MUTEX_MAX;s32Id++)
    {
        // if (PTHREAD_PROCESS_SHARED == s32Prop) // @FIXME: Richard: is the mutex name always used as an id, regardless of process shared/private property?
        {
            if(TRUE == _SclOs_Mutex_Info[s32Id].bUsed)
            {
                if (0== strcmp((const char*)_SclOs_Mutex_Info[s32Id].u8Name, (const char*)pMutexName))
                {
                    break;
                }
            }
        }
        if (FALSE==_SclOs_Mutex_Info[s32Id].bUsed  && SCLOS_MUTEX_MAX==s32LstUnused)
        {
            s32LstUnused = s32Id;
        }
    }
    if ((SCLOS_MUTEX_MAX==s32Id) && (SCLOS_MUTEX_MAX>s32LstUnused))
    {
        _SclOs_Mutex_Info[s32LstUnused].bUsed = TRUE;
        strcpy((char*)_SclOs_Mutex_Info[s32LstUnused].u8Name, (const char*)pMutexName);
#ifdef SCLOS_MUTEX_USE_SEM
        sema_init(&_SclOs_Mutex_Info[s32LstUnused].stMutex, 1);
#else
        spin_lock_init(&_SclOs_Mutex_Info[s32LstUnused].stMutex);
#endif
        s32Id = s32LstUnused;
    }
    MUTEX_MUTEX_UNLOCK();

    if(SCLOS_MUTEX_MAX <= s32Id)
    {
        return -1;
    }

    s32Id |= SCLOS_ID_PREFIX;

    return s32Id;
}
s32 DrvSclOsTsemInit(u32 nVal)
{
    s32 s32Id;
    static u8 u8cnt = 48;
    u8 u8TsemName[20]="Tsem_Mutex";

    strcat(u8TsemName, &u8cnt);
    s32Id = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, u8TsemName, SCLOS_PROCESS_SHARED);

    u8cnt++;
    return s32Id;
}
s32 DrvSclOsTsemDeinit(s32 s32Id)
{
    DrvSclOsDeleteMutex(s32Id);
    return -1;
}
bool DrvSclOsReleaseTsem(s32 s32Id)
{
    return DrvSclOsReleaseMutex(s32Id);
}
bool DrvSclOsObtainTsem(s32 s32Id)
{
    return DrvSclOsObtainMutex(s32Id, SCLOS_WAIT_FOREVER);
}
s32 DrvSclOsCreateSpinlock ( DrvSclOsAttributeType_e eAttribute, char *pMutexName1, u32 u32Flag)
{
    s32 s32Id, s32LstUnused = SCLOS_SPINLOCK_MAX;
    u8 pMutexName[SCLOS_MUTEX_NAME_LENGTH];
    u32 u32MaxLen;

    if (NULL == pMutexName1)
    {
        return -1;
    }
    if (strlen(pMutexName1) >= (SCLOS_MUTEX_NAME_LENGTH-1))
    {
        sclprintf("%s: Warning strlen(%s) is longer than SCLOS_MUTEX_NAME_LENGTH(%d). Oversize char will be discard.\n",
        __FUNCTION__, pMutexName1, SCLOS_MUTEX_NAME_LENGTH);
    }
    if (0 == (u32MaxLen = MIN(strlen(pMutexName1), (SCLOS_MUTEX_NAME_LENGTH-1))))
    {
        return -1;
    }
    strncpy((char*)pMutexName, (const char*)pMutexName1, u32MaxLen);
    pMutexName[u32MaxLen] = '\0';

    MUTEX_MUTEX_LOCK();
    for(s32Id=0;s32Id<SCLOS_SPINLOCK_MAX;s32Id++)
    {
        // if (PTHREAD_PROCESS_SHARED == s32Prop) // @FIXME: Richard: is the mutex name always used as an id, regardless of process shared/private property?
        {
            if(TRUE == _SclOs_Spinlock_Info[s32Id].bUsed)
            {
                if (0== strcmp((const char*)_SclOs_Spinlock_Info[s32Id].u8Name, (const char*)pMutexName))
                {
                    break;
                }
            }
        }
        if (FALSE==_SclOs_Spinlock_Info[s32Id].bUsed  && SCLOS_SPINLOCK_MAX==s32LstUnused)
        {
            s32LstUnused = s32Id;
        }
    }
    if ((SCLOS_SPINLOCK_MAX==s32Id) && (SCLOS_SPINLOCK_MAX>s32LstUnused))
    {
        _SclOs_Spinlock_Info[s32LstUnused].bUsed = TRUE;
        strcpy((char*)_SclOs_Spinlock_Info[s32LstUnused].u8Name, (const char*)pMutexName);
        spin_lock_init(&_SclOs_Spinlock_Info[s32LstUnused].stMutex);
        s32Id = s32LstUnused;
    }
    MUTEX_MUTEX_UNLOCK();

    if(SCLOS_SPINLOCK_MAX <= s32Id)
    {
        return -1;
    }

    s32Id |= SCLOS_ID_PREFIX;

    return s32Id;
}

//-------------------------------------------------------------------------------------------------
/// Delete the specified mutex
/// @param  s32MutexId  \b IN: mutex ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   It is important that the mutex be in the unlocked state when it is
///            destroyed, or else the behavior is undefined.
//-------------------------------------------------------------------------------------------------
bool DrvSclOsDeleteMutex (s32 s32MutexId)
{
    if ( (s32MutexId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32MutexId &= SCLOS_ID_MASK;
    }

    MUTEX_MUTEX_LOCK();

    SCL_ASSERT(_SclOs_Mutex_Info[s32MutexId].bUsed);
    _SclOs_Mutex_Info[s32MutexId].bUsed = FALSE;
    _SclOs_Mutex_Info[s32MutexId].u8Name[0] = '\0';

    MUTEX_MUTEX_UNLOCK();
    return TRUE;
}

bool DrvSclOsDeleteSpinlock (s32 s32MutexId)
{
    if ( (s32MutexId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32MutexId &= SCLOS_ID_MASK;
    }
    MUTEX_MUTEX_LOCK();

    SCL_ASSERT(_SclOs_Spinlock_Info[s32MutexId].bUsed);
    _SclOs_Spinlock_Info[s32MutexId].bUsed = FALSE;
    _SclOs_Spinlock_Info[s32MutexId].u8Name[0] = '\0';
    _SclOs_Spinlock_Info[s32MutexId].u32flag = 0;
    MUTEX_MUTEX_UNLOCK();
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Attempt to lock a mutex
/// @param  s32MutexId  \b IN: mutex ID
/// @param  u32WaitMs   \b IN: 0 ~ SCLOS_WAIT_FOREVER: suspend time (ms) if the mutex is locked
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
// @FIXME: don't support time-out at this stage
bool DrvSclOsObtainMutexIrq(s32 s32MutexId)
{
    if ( (s32MutexId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32MutexId &= SCLOS_ID_MASK;
    }
    spin_lock_irqsave(&(_SclOs_Spinlock_Info[s32MutexId].stMutex),_SclOs_Spinlock_Info[s32MutexId].u32flag);
    return TRUE;
}
bool DrvSclOsObtainMutex (s32 s32MutexId, u32 u32WaitMs)
{
    bool bRet = FALSE;
    if ( (s32MutexId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32MutexId &= SCLOS_ID_MASK;
    }

    if (u32WaitMs==SCLOS_WAIT_FOREVER) //blocking wait
    {
#ifdef SCLOS_MUTEX_USE_SEM
        down(&(_SclOs_Mutex_Info[s32MutexId].stMutex));
#else
        spin_lock(&(_SclOs_Mutex_Info[s32MutexId].stMutex));
#endif
        bRet = TRUE;
    }
    else if (u32WaitMs==0) //non-blocking
    {
#ifdef SCLOS_MUTEX_USE_SEM
        if (!down_trylock(&_SclOs_Mutex_Info[s32MutexId].stMutex))
#else
        if (spin_trylock(&_SclOs_Mutex_Info[s32MutexId].stMutex))
#endif
        {
            bRet = TRUE;
        }
    }
    else //blocking wait with timeout
    {
#ifndef SCLOS_MUTEX_USE_SEM
        u32 u32CurTime = DrvSclOsGetSystemTime();
#endif

        while(1)
        {
#ifdef SCLOS_MUTEX_USE_SEM
            if (0 == down_timeout(&_SclOs_Mutex_Info[s32MutexId].stMutex, msecs_to_jiffies(u32WaitMs)))
#else
            if (spin_trylock(&_SclOs_Mutex_Info[s32MutexId].stMutex))
#endif
            {
                bRet = TRUE;
                break;
            }
#ifdef SCLOS_MUTEX_USE_SEM
            else
#else
            else if((DrvSclOsGetSystemTime() - u32CurTime) > u32WaitMs)
#endif
            {
                sclprintf("Mutext TimeOut: ID:%x \n", (int)s32MutexId);
                bRet = FALSE;
                break;
            }

           // msleep(5);
        }

    }
    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Attempt to unlock a mutex
/// @param  s32MutexId  \b IN: mutex ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   Only the owner thread of the mutex can unlock it.
//-------------------------------------------------------------------------------------------------
char * DrvSclOsCheckMutex(char *str,char *end)
{
    s32 s32MutexId;
    for(s32MutexId=0;s32MutexId<SCLOS_MUTEX_MAX;s32MutexId++)
    {
        if((_SclOs_Mutex_Info[s32MutexId].bUsed))
        {
            #ifdef SCLOS_MUTEX_USE_SEM
            if(_SclOs_Mutex_Info[s32MutexId].stMutex.count==0)
            {
                str += DrvSclOsScnprintf(str, end - str, "%s Lock\n",_SclOs_Mutex_Info[s32MutexId].u8Name);
            }
            else
            {
                str += DrvSclOsScnprintf(str, end - str, "%s UnLock\n",_SclOs_Mutex_Info[s32MutexId].u8Name);
            }
            #else
            if(spin_is_locked(&(_SclOs_Mutex_Info[s32MutexId].stMutex)))
            {
                str += DrvSclOsScnprintf(str, end - str, "%s Lock\n",_SclOs_Mutex_Info[s32MutexId].u8Name);
            }
            else
            {
                str += DrvSclOsScnprintf(str, end - str, "%s UnLock\n",_SclOs_Mutex_Info[s32MutexId].u8Name);
            }

            #endif
        }
    }
    for(s32MutexId=0;s32MutexId<SCLOS_SPINLOCK_MAX;s32MutexId++)
    {
        if((_SclOs_Spinlock_Info[s32MutexId].bUsed))
        {
            if(spin_is_locked(&(_SclOs_Spinlock_Info[s32MutexId].stMutex)))
            {
                str += DrvSclOsScnprintf(str, end - str, "%s SpinLock\n",_SclOs_Spinlock_Info[s32MutexId].u8Name);
            }
            else
            {
                str += DrvSclOsScnprintf(str, end - str, "%s SpinUnLock\n",_SclOs_Spinlock_Info[s32MutexId].u8Name);
            }

        }
    }
    return str;
}
bool DrvSclOsReleaseMutexIrq (s32 s32MutexId)
{
    if ( (s32MutexId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32MutexId &= SCLOS_ID_MASK;
    }
    if(spin_is_locked(&(_SclOs_Spinlock_Info[s32MutexId].stMutex)))
    {
        spin_unlock_irqrestore(&(_SclOs_Spinlock_Info[s32MutexId].stMutex),_SclOs_Spinlock_Info[s32MutexId].u32flag);
    }
    return TRUE;

}
bool DrvSclOsReleaseMutex (s32 s32MutexId)
{
    if ( (s32MutexId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32MutexId &= SCLOS_ID_MASK;
    }
#ifdef SCLOS_MUTEX_USE_SEM
    up(&(_SclOs_Mutex_Info[s32MutexId].stMutex));
#else
    if(spin_is_locked(&(_SclOs_Mutex_Info[s32MutexId].stMutex)))
    {
        spin_unlock(&(_SclOs_Mutex_Info[s32MutexId].stMutex));
    }
#endif

    return TRUE;
}
bool DrvSclOsReleaseMutexAll (void)
{
    s32 s32Id;
    bool bRet = 1;
    for(s32Id=0;s32Id<SCLOS_MUTEX_MAX;s32Id++)
    {
        if(TRUE == _SclOs_Mutex_Info[s32Id].bUsed)
        {
#ifdef SCLOS_MUTEX_USE_SEM
            if((_SclOs_Mutex_Info[s32Id].stMutex.count==0))
#else
            if(spin_is_locked(&(_SclOs_Mutex_Info[s32Id].stMutex)))
#endif
            {
                bRet =0;
                SCL_ERR("[SCLOS]Mutex Not free %lx %s \n",s32Id,_SclOs_Mutex_Info[s32Id].u8Name);
                #ifdef SCLOS_MUTEX_USE_SEM
                    up(&(_SclOs_Mutex_Info[s32Id].stMutex));
                #else
                    spin_unlock(&(_SclOs_Mutex_Info[s32Id].stMutex));
                #endif
            }
        }
    }
    for(s32Id=0;s32Id<SCLOS_SPINLOCK_MAX;s32Id++)
    {
        if(TRUE == _SclOs_Spinlock_Info[s32Id].bUsed)
        {
            bRet =0;
            if(spin_is_locked(&(_SclOs_Spinlock_Info[s32Id].stMutex)))
            {
                SCL_ERR("[SCLOS]spinlock Not free %lx %s \n",s32Id,_SclOs_Spinlock_Info[s32Id].u8Name);
                spin_unlock_irqrestore(&(_SclOs_Spinlock_Info[s32Id].stMutex),_SclOs_Spinlock_Info[s32Id].u32flag);
            }
        }
    }
    return bRet;
}


//-------------------------------------------------------------------------------------------------
// Get a mutex informaton
// @param  s32MutexId  \b IN: mutex ID
// @param  peAttribute \b OUT: ptr to suspended mode: E_DRV_SCLOS_FIFO / E_DRV_SCLOS_PRIORITY
// @param  pMutexName  \b OUT: ptr to mutex name
// @return TRUE : succeed
// @return FALSE : the mutex has not been created.
//-------------------------------------------------------------------------------------------------
bool DrvSclOsInfoMutex (s32 s32MutexId, DrvSclOsAttributeType_e *peAttribute, char *pMutexName)
{
    if ( (s32MutexId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32MutexId &= SCLOS_ID_MASK;
    }

    if(_SclOs_Mutex_Info[s32MutexId].bUsed == TRUE)
    {
        //ToDo: extend _SclOs_Mutex_Info structure ?
        *peAttribute = E_DRV_SCLOS_FIFO; //only FIFO for eCos
        // @FIXME: linux porting
        // UTL_strcpy(pMutexName, "ABC");
        strcpy(pMutexName, (const char*)_SclOs_Mutex_Info[s32MutexId].u8Name);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
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
#if defined(SCLOS_TYPE_LINUX_TEST)
DrvSclOsTaskStruct_t DrvSclOsGetTaskinfo(s32 s32Id)
{
    DrvSclOsTaskStruct_t stTask;
    stTask.pThread = _SclOs_Task_Info[(s32Id&0xFFFF)].pstThreadInfo;
    return stTask;
}


bool DrvSclOsSleepTaskWork(s32 s32TaskId)
{
    if ( (s32TaskId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32TaskId &= SCLOS_ID_MASK;
    }
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    //schedule_timeout_interruptible(msecs_to_jiffies(10000));
    return 1;
}
bool DrvSclOsSetTaskWork(s32 s32TaskId)
{
    if ( (s32TaskId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32TaskId &= SCLOS_ID_MASK;
    }
    wake_up_process(_SclOs_Task_Info[s32TaskId].pstThreadInfo);
    return 1;
}

s32 DrvSclOsCreateTask (TaskEntry pTaskEntry,
                     u32 u32TaskEntryData,
                     bool bAutoStart,
                     char *pTaskName)
{
    // @FIXME:
    //     (1) eTaskPriority: Task priority is ignored here
    //     (2) pTaskName: is ignored here
    //     (3) Need mutex to protect critical section

    s32 s32Id;

    MUTEX_TASK_LOCK();

    for( s32Id=0; s32Id<SCLOS_TASK_MAX; s32Id++)
    {
        if(_SclOs_Task_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if( s32Id >= SCLOS_TASK_MAX)
    {
        return -1;
    }

    _SclOs_Task_Info[s32Id].bUsed = TRUE;
    //_SclOs_Task_Info[s32Id].pstThreadInfo = kthread_create(((int)(void *)pTaskEntry), (void*)u32TaskEntryData, pTaskName);
    _SclOs_Task_Info[s32Id].pstThreadInfo = kthread_create(pTaskEntry, (void*)u32TaskEntryData, pTaskName);

    MUTEX_TASK_UNLOCK();

    if (bAutoStart)
    {
        wake_up_process(_SclOs_Task_Info[s32Id].pstThreadInfo);
    }
    s32Id |= SCLOS_ID_PREFIX;
    return s32Id;
}


//-------------------------------------------------------------------------------------------------
/// Delete a previously created task
/// @param  s32TaskId   \b IN: task ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsDeleteTask (s32 s32TaskId)
{
#if 1
    if ( (s32TaskId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32TaskId &= SCLOS_ID_MASK;
    }

    kthread_stop(_SclOs_Task_Info[s32TaskId].pstThreadInfo);
    _SclOs_Task_Info[s32TaskId].bUsed = FALSE;
#else
    printk("[%s][%d] %s is not supported\n", __FUNCTION__, __LINE__, __FUNCTION__);
#endif
    return TRUE;
}
#endif
int DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_TYPE enType)
{
    return gSCLIRQID[enType];
}
int DrvSclOsSetSclIrqIDFormSys(MSOS_ST_PLATFORMDEVICE *pdev,u8 u8idx,E_DRV_SCLOS_SCLIRQ_TYPE enType)
{
    unsigned int SCL_IRQ_ID = 0; //INT_IRQ_AU_SYSTEM;
    SCL_IRQ_ID  = of_irq_to_resource(pdev->dev.of_node, u8idx, NULL);
    if (!SCL_IRQ_ID)
    {
        SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[MSOS] Can't Get SCL_IRQ\n");
        SCL_ERR("[MSOS] Can't Get SCL_IRQ\n");
        return -EINVAL;
    }
    else
    {
        SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[MSOS] Get resource SCL_IRQ = 0x%x\n",SCL_IRQ_ID);
        gSCLIRQID[enType] = SCL_IRQ_ID;
    }
    return 0;
}

bool DrvSclOsInit (void)
{
    u32 u32I;
    if(gbInit)
    {
        return TRUE;
    }
    //
    // Task Management
    //
#ifdef SCLOS_MUTEX_USE_SEM
    sema_init(&_SclOs_Task_Mutex, 1);
#else
    spin_lock_init(&_SclOs_Task_Mutex);
#endif
    for( u32I=0; u32I<SCLOS_TASK_MAX; u32I++)
    {
        _SclOs_Task_Info[u32I].bUsed = FALSE;
    }

    // Even Group
    spin_lock_init(&_SclOs_EventGroup_Mutex);
    for( u32I=0; u32I<SCLOS_EVENTGROUP_MAX; u32I++)
    {
        _SclOs_EventGroup_Info[u32I].bUsed = FALSE;
    }

    //
    // Timer
    //
    spin_lock_init(&_SclOs_Timer_Mutex);
    for( u32I=0; u32I<SCLOS_TIMER_MAX; u32I++)
    {
        _SclOs_Timer_Info[u32I].bUsed = FALSE;
        _SclOs_Timer_Info[u32I].pTimerCb = NULL;
        _SclOs_Timer_Info[u32I].period = 0;
        _SclOs_Timer_Info[u32I].first = 0;
        init_timer(&(_SclOs_Timer_Info[u32I].timer));
    }
    gbInit = TRUE;
    return TRUE;
}
void DrvSclOsExit (void)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclOsSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    gbInit = 0;
    gstGlobalSclOsSet->gu8SclFrameDelay = 0;
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
s32 DrvSclOsCreateEventGroup (char *pEventName)
{
    s32 s32Id;

    EVENT_MUTEX_LOCK();
    for(s32Id=0; s32Id<SCLOS_EVENTGROUP_MAX; s32Id++)
    {
        if(_SclOs_EventGroup_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if(s32Id < SCLOS_EVENTGROUP_MAX)
    {
        _SclOs_EventGroup_Info[s32Id].bUsed = TRUE;
        _SclOs_EventGroup_Info[s32Id].u32EventGroup= 0;
    }
    EVENT_MUTEX_UNLOCK();

    if(s32Id >= SCLOS_EVENTGROUP_MAX)
    {
        return -1;
    }
    spin_lock_init(&_SclOs_EventGroup_Info[s32Id].stMutexEvent);
    init_waitqueue_head(&_SclOs_EventGroup_Info[s32Id].stSemaphore);
    s32Id |= SCLOS_ID_PREFIX;
    return s32Id;
}
//-------------------------------------------------------------------------------------------------
/// Delete the event group
/// @param  s32EventGroupId \b IN: event group ID
/// @return TRUE : succeed
/// @return FALSE : fail, sb is waiting for the event flag
/// @note event group that are being waited on must not be deleted
//-------------------------------------------------------------------------------------------------
bool DrvSclOsDeleteEventGroup (s32 s32EventGroupId)
{
    s32 s32Id;
    s32Id = s32EventGroupId &SCLOS_ID_MASK;
    init_waitqueue_head(&_SclOs_EventGroup_Info[s32Id].stSemaphore);
    // PTH_RET_CHK(pthread_mutex_destroy(&_SclOs_EventGroup_Info[s32EventGroupId].stMutex));
    EVENT_MUTEX_LOCK();
    _SclOs_EventGroup_Info[s32Id].u32EventGroup= 0;
    _SclOs_EventGroup_Info[s32Id].bUsed = FALSE;
    EVENT_MUTEX_UNLOCK();
    return TRUE;
}


//-------------------------------------------------------------------------------------------------
/// Set the event flag (bitwise OR w/ current value) in the specified event group
/// @param  s32EventGroupId \b IN: event group ID
/// @param  u32EventFlag    \b IN: event flag value
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------

bool DrvSclOsSetEventIrq (s32 s32EventGroupId, u32 u32EventFlag)
{
    if ( (s32EventGroupId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32EventGroupId &= SCLOS_ID_MASK;
    }

    spin_lock_irq(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
    SET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32EventFlag);
    spin_unlock_irq(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
    wake_up(&_SclOs_EventGroup_Info[s32EventGroupId].stSemaphore);
    return TRUE;
}
bool DrvSclOsSetEvent (s32 s32EventGroupId, u32 u32EventFlag)
{
    if ( (s32EventGroupId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32EventGroupId &= SCLOS_ID_MASK;
    }

    spin_lock(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
    SET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32EventFlag);
    spin_unlock(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
    wake_up(&_SclOs_EventGroup_Info[s32EventGroupId].stSemaphore);
    return TRUE;
}
u32 DrvSclOsGetEvent(s32 s32EventGroupId)
{
    u32 u32Event;
    if ( (s32EventGroupId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32EventGroupId &= SCLOS_ID_MASK;
    }

    u32Event = HAS_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, 0xFFFFFFFF);
    return u32Event;
}
//-------------------------------------------------------------------------------------------------
/// Clear the specified event flag (bitwise XOR operation) in the specified event group
/// @param  s32EventGroupId \b IN: event group ID
/// @param  u32EventFlag    \b IN: event flag value
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsClearEvent (s32 s32EventGroupId, u32 u32EventFlag)
{
    if ( (s32EventGroupId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32EventGroupId &= SCLOS_ID_MASK;
    }

    spin_lock(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
    RESET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32EventFlag);
    spin_unlock(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
    return TRUE;
}
bool DrvSclOsClearEventIRQ (s32 s32EventGroupId, u32 u32EventFlag)
{
    if ( (s32EventGroupId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32EventGroupId &= SCLOS_ID_MASK;
    }
    spin_lock_irq(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
    RESET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32EventFlag);
    spin_unlock_irq(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Wait for the specified event flag combination from the event group
/// @param  s32EventGroupId     \b IN: event group ID
/// @param  u32WaitEventFlag    \b IN: wait event flag value
/// @param  pu32RetrievedEventFlag \b OUT: retrieved event flag value
/// @param  eWaitMode           \b IN: E_DRV_SCLOS_EVENT_MD_AND/E_DRV_SCLOS_EVENT_MD_OR/E_DRV_SCLOS_EVENT_MD_AND_CLEAR/E_DRV_SCLOS_EVENT_MD_OR_CLEAR
/// @param  u32WaitMs           \b IN: 0 ~ SCLOS_WAIT_FOREVER: suspend time (ms) if the event is not ready
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsWaitEvent (s32 s32EventGroupId,
                     u32 u32WaitEventFlag,
                     u32 *pu32RetrievedEventFlag,
                     DrvSclOsEventWaitMoodeType_e eWaitMode,
                     u32 u32WaitMs)
{
    bool bRet= FALSE;
    bool bAnd;
    bool bClear;
    bool bTimeout=0;

    *pu32RetrievedEventFlag = 0;
    if (!u32WaitEventFlag)
    {
        return FALSE;
    }

    if ( (s32EventGroupId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32EventGroupId &= SCLOS_ID_MASK;
    }

    bClear= ((E_DRV_SCLOS_EVENT_MD_AND_CLEAR== eWaitMode) || (E_DRV_SCLOS_EVENT_MD_OR_CLEAR== eWaitMode))? TRUE: FALSE;
    bAnd= ((E_DRV_SCLOS_EVENT_MD_AND== eWaitMode)|| (E_DRV_SCLOS_EVENT_MD_AND_CLEAR== eWaitMode))? TRUE: FALSE;

    do{
/*
        *pu32RetrievedEventFlag= HAS_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32WaitEventFlag);
        if ((bAnd)? (*pu32RetrievedEventFlag== u32WaitEventFlag): (0!= *pu32RetrievedEventFlag))
        {
            break;
        }
*/
        if (u32WaitMs== SCLOS_WAIT_FOREVER) //blocking wait
        {
            if (bAnd)
            {
                wait_event(_SclOs_EventGroup_Info[s32EventGroupId].stSemaphore,
                           ((_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup & u32WaitEventFlag) == u32WaitEventFlag));
            }
            else
            {
                //(sclprintf("[DRVSCLDMA]%ld before Wait event:%lx @:%ld \n",u32WaitEventFlag,_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup & u32WaitEventFlag,DrvSclOsGetSystemTime()));
                wait_event(_SclOs_EventGroup_Info[s32EventGroupId].stSemaphore,
                           ((_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup & u32WaitEventFlag) != 0));
                //(sclprintf("[DRVSCLDMA]%ld After Wait event:%lx @:%ld \n",u32WaitEventFlag,DrvSclOsGetEvent(s32EventGroupId),DrvSclOsGetSystemTime()));
            }
        }
        else
        {
            u32WaitMs = msecs_to_jiffies(u32WaitMs);
            if (bAnd)
            {
                bTimeout = wait_event_timeout(_SclOs_EventGroup_Info[s32EventGroupId].stSemaphore,
                                   ((_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup & u32WaitEventFlag) == u32WaitEventFlag),
                                   u32WaitMs);
            }
            else
            {
                bTimeout = wait_event_timeout(_SclOs_EventGroup_Info[s32EventGroupId].stSemaphore,
                                   ((_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup & u32WaitEventFlag) != 0),
                                   u32WaitMs);
            }
            if(!bTimeout)
                SCL_DBGERR("[SCLOS]wait timeout\n");
        }
        *pu32RetrievedEventFlag= HAS_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32WaitEventFlag);
    } while (0);
    bRet= (bAnd)? (*pu32RetrievedEventFlag== u32WaitEventFlag): (0!= *pu32RetrievedEventFlag);
    if (bRet && bClear)
    {
        spin_lock(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
        RESET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, *pu32RetrievedEventFlag);
        spin_unlock(&_SclOs_EventGroup_Info[s32EventGroupId].stMutexEvent);
    }
    return bRet;
}
#if 0
wait_queue_head_t* DrvSclOsGetEventQueue (s32 s32EventGroupId)
{
    if ( (s32EventGroupId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32EventGroupId &= SCLOS_ID_MASK;
    }
    return (&_SclOs_EventGroup_Info[s32EventGroupId].stSemaphore);
}


//
// Timer management
//
static void _DrvSclOsTimerNotify(u32 data)
{
    s32 s32Id = (s32) data;

    if (_SclOs_Timer_Info[s32Id].pTimerCb)
    {
        _SclOs_Timer_Info[s32Id].pTimerCb(0, s32Id| SCLOS_ID_PREFIX);
    }
    _SclOs_Timer_Info[s32Id].timer.expires = jiffies + HZ*_SclOs_Timer_Info[s32Id].period/1000;
    add_timer(&(_SclOs_Timer_Info[s32Id].timer));
}


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
s32 DrvSclOsCreateTimer (TimerCb pTimerCb,
                      u32 u32FirstTimeMs,
                      u32 u32PeriodTimeMs,
                      bool bStartTimer,
                      char *pTimerName)
{
    s32 s32Id;

    TIMER_MUTEX_LOCK();
    for(s32Id=0;s32Id<SCLOS_TIMER_MAX;s32Id++)
    {
        if(_SclOs_Timer_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if(s32Id < SCLOS_TIMER_MAX)
    {
        _SclOs_Timer_Info[s32Id].bUsed = TRUE;
    }
    TIMER_MUTEX_UNLOCK();

    if(s32Id >= SCLOS_TIMER_MAX)
    {
        return -1;
    }

    _SclOs_Timer_Info[s32Id].pTimerCb=   pTimerCb;
    _SclOs_Timer_Info[s32Id].first = u32FirstTimeMs;
    _SclOs_Timer_Info[s32Id].period = u32PeriodTimeMs;
    _SclOs_Timer_Info[s32Id].timer.data = (u32)s32Id;
    _SclOs_Timer_Info[s32Id].timer.expires = jiffies + HZ*u32FirstTimeMs/1000;
    _SclOs_Timer_Info[s32Id].timer.function = _DrvSclOsTimerNotify;
    if (bStartTimer)
    {
        add_timer(&(_SclOs_Timer_Info[s32Id].timer));
    }
    s32Id |= SCLOS_ID_PREFIX;
    return s32Id;
}

//-------------------------------------------------------------------------------------------------
/// Delete the Timer
/// @param  s32TimerId  \b IN: Timer ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsDeleteTimer (s32 s32TimerId)
{
    //return FALSE;

    if ( (s32TimerId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32TimerId &= SCLOS_ID_MASK;
    }

    if( _SclOs_Timer_Info[s32TimerId].bUsed )
    {
        TIMER_MUTEX_LOCK();
        del_timer(&(_SclOs_Timer_Info[s32TimerId].timer));
        _SclOs_Timer_Info[s32TimerId].bUsed = FALSE;

        _SclOs_Timer_Info[s32TimerId].pTimerCb = NULL;
        _SclOs_Timer_Info[s32TimerId].period = 0;
        _SclOs_Timer_Info[s32TimerId].first = 0;
        TIMER_MUTEX_UNLOCK();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/// Start the Timer
/// @param  s32TimerId  \b IN: Timer ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsStartTimer (s32 s32TimerId)
{
    //return FALSE;

    if ( (s32TimerId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32TimerId &= SCLOS_ID_MASK;
    }

    if( _SclOs_Timer_Info[s32TimerId].bUsed )
    {
        _SclOs_Timer_Info[s32TimerId].timer.expires = jiffies + _SclOs_Timer_Info[s32TimerId].period/1000;
        add_timer(&(_SclOs_Timer_Info[s32TimerId].timer));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/// Stop the Timer
/// @param  s32TimerId  \b IN: Timer ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   DrvSclOsStopTimer then DrvSclOsStartTimer => The timer will trigger at the same relative
///             intervals that it would have if it had not been disabled.
//-------------------------------------------------------------------------------------------------
bool DrvSclOsStopTimer (s32 s32TimerId)
{
    //return FALSE;

    if ( (s32TimerId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32TimerId &= SCLOS_ID_MASK;
    }

    if( _SclOs_Timer_Info[s32TimerId].bUsed )
    {
        del_timer(&(_SclOs_Timer_Info[s32TimerId].timer));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
s32 DrvSclOsCreateWorkQueueEvent(void * pTaskEntry)
{
    s32 s32Id;
    MUTEX_TASK_LOCK();

    for( s32Id=0; s32Id<SCLOS_WORK_MAX; s32Id++)
    {
        if(_SclOs_WorkEvent_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if( s32Id >= SCLOS_WORK_MAX)
    {
        return -1;
    }

    _SclOs_WorkEvent_Info[s32Id].bUsed = TRUE;
    INIT_WORK(&_SclOs_WorkEvent_Info[s32Id].stWorkEventInfo, pTaskEntry);

    MUTEX_TASK_UNLOCK();
    s32Id |= SCLOS_ID_PREFIX;
    return s32Id;
}

bool DrvSclOsestroyWorkQueueTask(s32 s32Id)
{

    if ( (s32Id & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32Id &= SCLOS_ID_MASK;
    }
    MUTEX_TASK_LOCK();
    _SclOs_WorkQueue_Info[s32Id].bUsed = FALSE;
    destroy_workqueue(_SclOs_WorkQueue_Info[s32Id].pstWorkQueueInfo);
    _SclOs_WorkQueue_Info[s32Id].pstWorkQueueInfo = NULL;

    MUTEX_TASK_UNLOCK();
    return TRUE;
}

s32 DrvSclOsCreateWorkQueueTask(char *pTaskName)
{
    s32 s32Id;
    MUTEX_TASK_LOCK();

    for( s32Id=0; s32Id<SCLOS_WORKQUEUE_MAX; s32Id++)
    {
        if(_SclOs_WorkQueue_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if( s32Id >= SCLOS_WORKQUEUE_MAX)
    {
        return -1;
    }

    _SclOs_WorkQueue_Info[s32Id].bUsed = TRUE;
    _SclOs_WorkQueue_Info[s32Id].pstWorkQueueInfo = create_workqueue(pTaskName);

    MUTEX_TASK_UNLOCK();
    s32Id |= SCLOS_ID_PREFIX;
    return s32Id;
}

bool DrvSclOsQueueWork(bool bTask, s32 s32TaskId, s32 s32QueueId, u32 u32WaitMs)
{
    bool bRet;
    if(bTask)
    {
        if ( (s32TaskId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
        {
            return FALSE;
        }
        else
        {
            s32TaskId &= SCLOS_ID_MASK;
        }
    }
    if ( (s32QueueId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32QueueId &= SCLOS_ID_MASK;
    }
    MUTEX_TASK_LOCK();
    if(u32WaitMs)
    {
        u32WaitMs = msecs_to_jiffies(u32WaitMs);
        if(bTask)
        {
            bRet = queue_delayed_work(_SclOs_WorkQueue_Info[s32TaskId].pstWorkQueueInfo ,
                            to_delayed_work(&_SclOs_WorkEvent_Info[s32QueueId].stWorkEventInfo),
                            u32WaitMs);
        }
        else
        {
            bRet = schedule_delayed_work(to_delayed_work(&_SclOs_WorkEvent_Info[s32QueueId].stWorkEventInfo), u32WaitMs);
        }
    }
    else
    {
        if(bTask)
        {
            bRet = queue_work(_SclOs_WorkQueue_Info[s32TaskId].pstWorkQueueInfo ,
                &_SclOs_WorkEvent_Info[s32QueueId].stWorkEventInfo);
        }
        else
        {
            bRet = schedule_work(&_SclOs_WorkEvent_Info[s32QueueId].stWorkEventInfo);
        }
    }
    MUTEX_TASK_UNLOCK();
    return bRet;
}

bool DrvSclOsQueueWait(s32 s32QueueId)
{
    return TRUE;
}


bool DrvSclOsGetQueueExist(s32 s32QueueId)
{
    return TRUE;
}


bool DrvSclOsFlushWorkQueue(bool bTask, s32 s32TaskId)
{
    if(bTask)
    {
        if ( (s32TaskId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
        {
            return FALSE;
        }
        else
        {
            s32TaskId &= SCLOS_ID_MASK;
        }
    }
    MUTEX_TASK_LOCK();
    if(bTask)
    {
        flush_workqueue(_SclOs_WorkQueue_Info[s32TaskId].pstWorkQueueInfo);
    }
    else
    {
        flush_scheduled_work();
    }
    MUTEX_TASK_UNLOCK();
    return TRUE;
}
s32 DrvSclOsCreateTasklet(void * pTaskEntry,u32 u32data)
{
    s32 s32Id;
    MUTEX_TASK_LOCK();

    for( s32Id=0; s32Id<SCLOS_TASKLET_MAX; s32Id++)
    {
        if(_SclOs_Tasklet_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if( s32Id >= SCLOS_TASKLET_MAX)
    {
        return -1;
    }

    _SclOs_Tasklet_Info[s32Id].bUsed = TRUE;
    tasklet_init(&_SclOs_Tasklet_Info[s32Id].stTaskletInfo, pTaskEntry,u32data);
    MUTEX_TASK_UNLOCK();
    s32Id |= SCLOS_ID_PREFIX;
    return s32Id;
}
bool DrvSclOsDestroyTasklet(s32 s32Id)
{

    if ( (s32Id & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32Id &= SCLOS_ID_MASK;
    }
    MUTEX_TASK_LOCK();
    _SclOs_Tasklet_Info[s32Id].bUsed = FALSE;
    tasklet_kill(&_SclOs_Tasklet_Info[s32Id].stTaskletInfo);

    MUTEX_TASK_UNLOCK();
    return TRUE;
}
bool DrvSclOsEnableTasklet (s32 s32Id)
{
    if ( (s32Id & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32Id &= SCLOS_ID_MASK;
    }
    MUTEX_TASK_LOCK();
    tasklet_enable(&_SclOs_Tasklet_Info[s32Id].stTaskletInfo);
    MUTEX_TASK_UNLOCK();
    return TRUE;
}
bool DrvSclOsDisableTasklet (s32 s32Id)
{
    if ( (s32Id & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32Id &= SCLOS_ID_MASK;
    }
    MUTEX_TASK_LOCK();
    tasklet_disable(&_SclOs_Tasklet_Info[s32Id].stTaskletInfo);
    MUTEX_TASK_UNLOCK();
    return TRUE;
}
bool DrvSclOsTaskletWork(s32 s32TaskId)
{
    bool bRet = TRUE;
    if ( (s32TaskId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32TaskId &= SCLOS_ID_MASK;
    }
    MUTEX_TASK_LOCK();
    tasklet_schedule(&_SclOs_Tasklet_Info[s32TaskId].stTaskletInfo);
    MUTEX_TASK_UNLOCK();
    return bRet;
}
#endif
// linux
void* DrvSclOsMemalloc(size_t size, gfp_t flags)
{
    return kmalloc(size, flags);
}
void DrvSclOsMemFree(void *pVirAddr)
{
    kfree(pVirAddr);
}
void* DrvSclOsVirMemalloc(size_t size)
{
    void *vp = vmalloc(size);
    //SCL_ERR("[SCLOS]Alloc @:%lx ,size :%u\n",(u32)vp,size);
    return vp;
}
void DrvSclOsVirMemFree(void *pVirAddr)
{
    //SCL_ERR("[SCLOS]Free @:%lx \n",(u32)pVirAddr);
    vfree(pVirAddr);
}
void* DrvSclOsMemcpy(void *pstCfg,const void *pstInformCfg,__kernel_size_t size)
{
    return memcpy(pstCfg, pstInformCfg, size);
}
void* DrvSclOsMemset(void *pstCfg,int val,__kernel_size_t size)
{
    return memset(pstCfg, val, size);
}
void DrvSclOsStrcat(char *pstCfg,char *name)
{
    strcat(pstCfg, (const char*)name);
}
u32 DrvSclOsCopyFromUser(void *to, const void *from, u32 n)
{
    return copy_from_user(to, (void __user *)from, n);
}
u32 DrvSclOsCopyToUser(void *to, const void *from, u32 n)
{
    return copy_to_user(to, (void __user *)from, n);
}
void DrvSclOsWaitForCpuWriteToDMem(void)
{
    Chip_Flush_MIU_Pipe();                          //wait for CPU write to mem
}
void DrvSclOsChipFlushCacheRange(unsigned long u32Addr, unsigned long u32Size)
{
    Chip_Flush_Cache_Range(u32Addr,u32Size);                          //wait for CPU write to mem
}
u8 DrvSclOsGetSclFrameBufferNum(void)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclOsSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    return gstGlobalSclOsSet->gu8FrameBufferNum;
}
void DrvSclOsSetSclFrameBufferNum(u8 u8FrameBufferReadyNum)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclOsSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    gstGlobalSclOsSet->gu8FrameBufferNum = u8FrameBufferReadyNum;
}
DrvSclOsFBAlloced_e DrvSclOsGetSclFrameBufferAlloced(void)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclOsSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    return gstGlobalSclOsSet->enFbAlloced;
}
void DrvSclOsSetSclFrameBufferAlloced(DrvSclOsFBAlloced_e enType)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclOsSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    gstGlobalSclOsSet->enFbAlloced = enType;
}
DrvSclOsAccessRegType_e DrvSclOsGetAccessRegMode(void)
{
    return gbAccessRegMode;
}
void DrvSclOsSetAccessRegMode(DrvSclOsAccessRegType_e enAccMode)
{
    gbAccessRegMode = enAccMode;
}


u32 DrvSclOsPa2Miu(DrvSclOsDmemBusType_t addr)
{
    return Chip_Phys_to_MIU(addr);
}
DrvSclOsDmemBusType_t DrvSclOsMiu2Pa(DrvSclOsDmemBusType_t addr)
{
    return Chip_MIU_to_Phys(addr);
}
void DrvSclOsKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq)
{
    if(pstCmdq)
    {
        MDrvSclCtxKeepCmdqFunction(pstCmdq);
    }
}

void DrvSclOsKeepAllocFunction(DrvSclOsAllocPhyMem_t *pstAlloc)
{
    if(pstAlloc)
    {
        gstSclOsAlloc.SclOsAlloc = pstAlloc->SclOsAlloc;
        gstSclOsAlloc.SclOsFree = pstAlloc->SclOsFree;
        gstSclOsAlloc.SclOsmap = pstAlloc->SclOsmap;
        gstSclOsAlloc.SclOsunmap = pstAlloc->SclOsunmap;
        gstSclOsAlloc.SclOsflush_cache = pstAlloc->SclOsflush_cache;
    }
}
void DrvSclOsDirectMemFlush(unsigned long u32Addr, unsigned long u32Size)
{
    if(gstSclOsAlloc.SclOsflush_cache)
    {
        gstSclOsAlloc.SclOsflush_cache((void *)u32Addr,u32Size);
    }
    else
    {
        Chip_Flush_Cache_Range(u32Addr,u32Size);                          //wait for CPU write to mem
    }
}
void * _DrvSclOsDirectMemMap(u64 u64PhyAddr, u32 u32Size)
{
    struct vm_struct *area;
    unsigned long count;
    unsigned long addr;
    unsigned long long cpu_bus_addr;
    int err;
    pgprot_t pgprot;
    pgprot = pgprot_writecombine(PAGE_KERNEL);

    cpu_bus_addr = DrvSclOsMiu2Pa(u64PhyAddr);

    count = PAGE_ALIGN(u32Size) / PAGE_SIZE;
    // to get empty vm area.
    area = get_vm_area_caller((count << PAGE_SHIFT), VM_MAP, __builtin_return_address(0));
    if(!area)
    {
        SCL_ERR("get_vm_area_caller failed\n");
        return NULL;
    }

    area->phys_addr = cpu_bus_addr;
    addr = (unsigned long)area->addr;
    //modify page to realize mapping
    err = ioremap_page_range(addr, addr + count*PAGE_SIZE,  cpu_bus_addr, pgprot);
    if(err)
    {
        vunmap((void *)addr);
        return NULL;
    }
    return (void*)addr;
}

void* DrvSclOsDirectMemMap(unsigned long u32Addr, unsigned long u32Size)
{
    u64 u64phyAddr = 0;
    void *pv = NULL;
    u64phyAddr = u32Addr;
    if(gstSclOsAlloc.SclOsmap)
    {
        pv = gstSclOsAlloc.SclOsmap(u64phyAddr,u32Size,0);
    }
    else
    {
        pv = _DrvSclOsDirectMemMap(u64phyAddr,u32Size);                          //wait for CPU write to mem
    }
    return (void *)pv;
}
void DrvSclOsDirectMemUnmap(unsigned long u32Addr)
{
    if(gstSclOsAlloc.SclOsunmap)
    {
        gstSclOsAlloc.SclOsunmap((void *)u32Addr);
    }
    else
    {
        vunmap((void *)u32Addr);
    }
}
void* DrvSclOsDirectMemAlloc(char* name, u32 size, DrvSclOsDmemBusType_t *addr)
{
    if(gstSclOsAlloc.SclOsAlloc)
    {
        void *pv;
        u64 u64phyAddr = 0;
        if(!gstSclOsAlloc.SclOsAlloc(NULL,size,(u64 *)&u64phyAddr))
        {
            *addr = (DrvSclOsDmemBusType_t)u64phyAddr;
            pv = gstSclOsAlloc.SclOsmap(u64phyAddr,size,0);
        }
        return (void *)pv;
    }
    else
    {
        MSYS_DMEM_INFO dmem;
        memcpy(dmem.name,name,strlen(name)+1);
        dmem.length = size;
        if(0 != msys_request_dmem(&dmem))
        {
            return NULL;
        }

        *addr = DrvSclOsPa2Miu(dmem.phys);
        return (void *)((uintptr_t)dmem.kvirt);
    }
}

void DrvSclOsDirectMemFree(const char* name, u32 size, void *virt, DrvSclOsDmemBusType_t addr)
{
    if(gstSclOsAlloc.SclOsFree)
    {
        gstSclOsAlloc.SclOsunmap(virt);
        gstSclOsAlloc.SclOsFree((u64)addr);
    }
    else
    {
        MSYS_DMEM_INFO dmem;
        memcpy(dmem.name,name,strlen(name)+1);
        dmem.length = size;
        dmem.kvirt  = (unsigned long long)((uintptr_t)virt);
        dmem.phys   = (unsigned long long)DrvSclOsMiu2Pa(((uintptr_t)addr));
        msys_release_dmem(&dmem);
    }
}
DrvSclosDevType_e MDrvSclOsGetDevByHandler(s32 s32Handler)
{
    DrvSclosDevType_e enDevId;
    s32 s32Id = s32Handler&HANDLER_PRE_MASK;
    switch(s32Id)
    {
        case SCLDMA_HANDLER_PRE_FIX:
            enDevId = E_DRV_SCLOS_DEV_DMA_1;
            break;
        case SCLHVSP_HANDLER_PRE_FIX:
            enDevId = E_DRV_SCLOS_DEV_HVSP_1;
            break;
        case SCLVIP_HANDLER_PRE_FIX:
            enDevId = E_DRV_SCLOS_DEV_VIP;
            break;
        case SCLPNL_HANDLER_PRE_FIX:
            enDevId = E_DRV_SCLOS_DEV_PNL;
            break;
        case SCLM2M_HANDLER_PRE_FIX:
            enDevId = E_DRV_SCLOS_DEV_M2M;
            break;
        default:
            enDevId = E_DRV_SCLOS_DEV_MAX;
            break;
    }
    if(enDevId==E_DRV_SCLOS_DEV_DMA_1 || enDevId==E_DRV_SCLOS_DEV_HVSP_1)
    {
        enDevId += ((s32Handler&HANDLER_DEV_PRE_MASK)>>(HANDLER_PRE_FIX_SHIFT));
    }
    SCL_DBG(SCL_DBG_LV_IOCTL()&EN_DBGMG_IOCTLEVEL_SC1HLEVEL, "[CTX]Get s32Handler:%lx enDevId:%d\n"
        ,s32Handler,enDevId);
    return enDevId;
}
