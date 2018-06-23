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
#include <string.h>
#include "kernel.h"

#include "sys_sys.h"
#include "sys_sys_isw_uart.h"
#include "sys_MsWrapper_cus_os_msg.h"
#include "sys_MsWrapper_cus_os_timer.h"
#include "sys_MsWrapper_cus_os_mem.h"
#include "sys_MsWrapper_cus_os_util.h"
#include "sys_MsWrapper_cus_os_int_pub.h"
#include "sys_MsWrapper_cus_os_int_ctrl.h"
#include "sys_MsWrapper_cus_os_flag.h"
#include "sys_MsWrapper_cus_os_sem.h"

#include "drv_timer_api.h"
#include "sys_rtk_hp.h"
#include "hal_drv_util.h"
#include "drv_clkgen_cmu.h"
#include "hal_int_ctrl_pub.h"
#include "drv_scl_os.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"
#include "drv_scl_dbg.h"
#include "cam_os_wrapper.h"
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
#define SCLOS_IRQ_MAX      (E_DRV_SCLOS_SCLIRQ_MAX+E_DRV_SCLOS_CMDQIRQ_MAX)

/********************************************************************************/
/*                           Constant                                           */
/********************************************************************************/


/******************************************************************************/
/*                           Enum                                             */
/******************************************************************************/


/******************************************************************************/
/*                           Global Variables                                   */
/******************************************************************************/
//
// Mutex
//
typedef struct
{
    bool             bUsed;
    Ms_Mutex_t       stMutex;
    u8               u8Name[SCLOS_MUTEX_NAME_LENGTH];
} DrvSclOsMutexInfoConfig_t;

typedef struct
{
    bool             bUsed;
    CamOsTsem_t      _SclOs_Lock_Mutex_t;
}DrvSclOsTsemInfoConfig_t ;

typedef struct
{
    volatile u32     u32Key;
    volatile u32     u32Lock;
    volatile u32     u32Access;
} DrvSclOsSpinlockConfig_t;

typedef struct
{
    bool             bUsed;
    DrvSclOsSpinlockConfig_t  stSpin;
    u8               u8Name[SCLOS_MUTEX_NAME_LENGTH];
} DrvSclOsSpinlockInfoConfig_t;

static DrvSclOsTsemInfoConfig_t           _SclOs_Tsem_Info[SCLOS_TSEM_MAX];
static DrvSclOsMutexInfoConfig_t          _SclOs_Mutex_Info[SCLOS_MUTEX_MAX];
//static DrvSclOsSpinlockInfoConfig_t       _SclOs_Spinlock_Info[SCLOS_SPINLOCK_MAX];


u8 gu8SclFrameDelay;
DrvSclOsAccessRegType_e gbAccessRegMode = E_DRV_SCLOS_AccessReg_CPU;

static                          Ms_Mutex_t _SclOs_Mutex_Mutex;
#define MUTEX_MUTEX_LOCK()      MsMutexLock(&_SclOs_Mutex_Mutex)
#define MUTEX_MUTEX_UNLOCK()    MsMutexUnlock(&_SclOs_Mutex_Mutex)


//
// Task Management
//
typedef struct
{
    bool               bUsed;
    MsTaskCreateArgs_t TaskArgs;
    MsTaskId_e         eTaskId;
} DrvSclOsTaskInfoConfig_t;
typedef struct
{
    bool             bUsed;
} DrvSclOsWorkQueueInfoConfig_t;

typedef struct
{
    bool             bUsed;
    Ms_Flag_t                sFlag;         //like wait queue
} DrvSclOsWorkEventInfoConfig_t;

typedef struct
{
    bool             bUsed;
    //DrvSclOsTaskletConfig_t     stTaskletInfo; RTK_ToDo
} DrvSclOsTaskletInfoConfig_t;

//static DrvSclOsTaskInfoConfig_t   _SclOs_Task_Info[SCLOS_TASK_MAX];
static DrvSclOsWorkQueueInfoConfig_t   _SclOS_WorkQueue_Info[SCLOS_WORKQUEUE_MAX];
static DrvSclOsWorkEventInfoConfig_t   _SclOS_WorkEvent_Info[SCLOS_WORK_MAX];
Ms_Flag_t                              _SclOs_Task_Flag;         //like wait queue
Ms_Flag_t                              _SclOs_Poll_Flag;         //like wait queue


//
// Event Group
//
typedef struct
{
    bool                     bUsed;
    u32                      u32EventGroup; // event
    Ms_Flag_t                sFlag;         //like wait queue
    Ms_Mutex_t       sMutexEvent; //spinlock
    u32                      u32Waitflag;         //like wait queue
} DrvSclOsEventGroupInfoConfig_t;
typedef struct
{
    bool                     bUsed;
    u32                      u32EventGroup;
    u8                       u8Wpoint;
    u8                       u8Rpoint;
} DrvSclOsRingEventGroupInfoConfig_t;

static DrvSclOsEventGroupInfoConfig_t         _SclOs_EventGroup_Info[SCLOS_EVENTGROUP_MAX];
static DrvSclOsRingEventGroupInfoConfig_t     _SclOs_RingEventGroup_Info[SCLOS_EVENTGROUP_MAX];

static                          Ms_Mutex_t _SclOs_EventGroup_Mutex;
#define EVENT_MUTEX_LOCK()      MsMutexLock(&_SclOs_EventGroup_Mutex)
#define EVENT_MUTEX_UNLOCK()    MsMutexUnlock(&_SclOs_EventGroup_Mutex)

//Clk
typedef struct
{
    bool bUsed;
    DrvSclOsClkConfig_t  stClk;
}DrvSclOsClkInfoConfig_t;
#define INITCLKGENDRV CLKGEN_DRV_HVSP1
static s32 s32Taskid;
static DrvSclOsClkInfoConfig_t  _SclOs_Clk_Info[SCLOS_CLK_MAX];
static DrvSclOsClkStruct_t _SclOs_Clk_IDCLK = {TOP_CLK_IDCLK,INITCLKGENDRV,CLKGEN_CLK_REQ_INVALID,TOP_CLK_IDCLK,0};
static DrvSclOsClkStruct_t _SclOs_Clk_FCLK1 = {TOP_CLK_FCLK1,INITCLKGENDRV,CLKGEN_CLK_REQ_INVALID,TOP_CLK_FCLK1,0};
static DrvSclOsClkStruct_t _SclOs_Clk_FCLK2 = {TOP_CLK_FCLK2,INITCLKGENDRV,CLKGEN_CLK_REQ_INVALID,TOP_CLK_FCLK2,0};
static DrvSclOsClkStruct_t _SclOs_Clk_ODCLK = {TOP_CLK_ODCLK,INITCLKGENDRV,CLKGEN_CLK_REQ_INVALID,TOP_CLK_ODCLK,0};
static DrvSclOsClkStruct_t _SclOs_Clk_ParentCLK = {TOP_CLK_IDCLK,INITCLKGENDRV,CLKGEN_CLK_REQ_INVALID,TOP_CLK_IDCLK,0};
static                          Ms_Mutex_t _SclOs_Clk_Mutex;
#define CLK_MUTEX_LOCK()        MsMutexLock(&_SclOs_Clk_Mutex)
#define CLK_MUTEX_UNLOCK()      MsMutexUnlock(&_SclOs_Clk_Mutex)

//
// Timer
//
typedef struct
{
    bool                bUsed;
    TimerCb             pTimerCb;
    int                 period;
    int                 first;
} DrvSclOsTimerInfoConfig_t;

static DrvSclOsTimerInfoConfig_t  _SclOs_Timer_Info[SCLOS_TIMER_MAX];

static                          Ms_Mutex_t _SclOs_Timer_Mutex;
#define TIMER_MUTEX_LOCK()      MsMutexLock(&_SclOs_Timer_Mutex)
#define TIMER_MUTEX_UNLOCK()    MsMutexUnlock(&_SclOs_Timer_Mutex)

/******************************************************************************/
/*                           Local Variables                                  */
/******************************************************************************/
u32 gIRQNUM[SCLOS_IRQ_MAX];
u32 gIRQIdx = 0;
u8 gbDigitalZoomDropMode = 0;
bool gbclkforcemode;
u32 gSCLIRQID[E_DRV_SCLOS_SCLIRQ_MAX] = {SCL_IRQID_Default,(SCL_IRQID_Default+38),(SCL_IRQID_Default+39)}; //INT_I
u32 gCMDQIRQID[E_DRV_SCLOS_CMDQIRQ_MAX] = {CMDQ_IRQID_Default,(CMDQ_IRQID_Default+35),(CMDQ_IRQID_Default+36)}; //
void *gvpdev[E_DRV_SCLOS_DEV_MAX];
u32 gu32Init = 0;
bool gbInit = FALSE;
DrvSclOsAllocPhyMem_t gstSclOsAlloc;
MDrvSclCtxSclOsGlobalSet_t *gstGlobalSclOsSet;

/******************************************************************************/
/*               P r i v a t e    F u n c t i o n s                             */
/******************************************************************************/
void _DrvSclOsSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstGlobalSclOsSet = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stSclOsCfg);
}
void DrvSclOsExit (void)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclOsSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    gbInit = 0;
    gstGlobalSclOsSet->gu8SclFrameDelay = 0;
}

bool DrvSclOsSetPollWait(DrvSclOsPollWaitConfig_t *stPollWait)
{
    u32 u32Events = 0;
    bool bTimeOut;
    u32 u32WaitEvent;
    u32WaitEvent = stPollWait->u32WaitEvent;
    bTimeOut = DrvSclOsWaitEvent(stPollWait->s32EventId,u32WaitEvent, &u32Events, E_DRV_SCLOS_EVENT_MD_OR, stPollWait->u32Timeout); // get status: FRM END
    return bTimeOut;
}

u32 DrvSclOsGetSystemTime (void)
{
    return VM_RTK_TICK_TO_MS(MsGetOsTick());
}

u32 DrvSclOsGetSystemTimeStamp (void)
{
    return DrvTimerStdaTimerTick2Us(RtkGetOsTickExt());
}

u32 DrvSclOsTimerDiffTimeFromNow(u32 u32TaskTimer) //unit = ms
{
    return  VM_RTK_TICK_TO_MS(MsGetOsTick()) - u32TaskTimer;
}

void DrvSclOsDelayTask (u32 u32Ms)
{
    MsSleep(u32Ms);
}


void DrvSclOsDelayTaskUs (u32 u32Us)
{
    SYS_UDELAY(u32Us);
}

void DrvSclOsSetProbeInformation(DrvSclosProbeType_e enProbe)
{
    gu32Init |= (u32)enProbe;
    SCL_ERR("[SCL] DrvSclOsSetProbeInformation 0x%x-0x%x-0x%x\n",gu32Init,enProbe,E_DRV_SCLOS_INIT_ALL);
    if(gu32Init == E_DRV_SCLOS_INIT_ALL)
    {
        SCL_ERR("[SCL] SCL init success\n");
        DrvSclOsSetAccessRegMode(E_DRV_SCLOS_AccessReg_CMDQ);
#if I2_DVR_TEST
        DrvSclOsTestVpe();
        DrvSclOsTestVpeIq();
#endif
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
bool DrvSclOsAttachInterrupt (u32 eIntNum, InterruptCb pIntCb,unsigned long flags,const char *name)
{
    MsIntInitParam_u uInitParam;
    uInitParam.intc.eMap = INTC_MAP_IRQ;
    uInitParam.intc.ePriority = INTC_PRIORITY_3;
    uInitParam.intc.pfnIsr = pIntCb;
    gIRQNUM[gIRQIdx] = eIntNum;
    gIRQIdx++;
    MsInitInterrupt(&uInitParam, eIntNum);

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Detach the interrupt callback function from interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsDetachInterrupt (u32 eIntNum)
{
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// Enable (unmask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsEnableInterrupt (u32 eIntNum)
{
    MsUnmaskInterrupt((MsIntNumber_e)eIntNum);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Disable (mask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
bool DrvSclOsDisableInterrupt (u32 eIntNum)
{
    MsMaskInterrupt((MsIntNumber_e)eIntNum);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Disable all interrupts (including timer interrupt), the scheduler is disabled.
/// @return Interrupt register value before all interrupts disable
//-------------------------------------------------------------------------------------------------
u32 _DrvSclOsAllInterruptsMask(bool bSet,u32 u32OldInterrupts)
{
    static MsIntMask_e Mask = 0;
    static u8 u8RefCnt = 0;
    u32 Ret = 0;
    MUTEX_MUTEX_LOCK();
    if(bSet)
    {
        Mask |= u32OldInterrupts;
        Ret = 0;
        u8RefCnt++;
    }
    else
    {
        if(u8RefCnt == 0)
        {
            Ret = Mask;
        }
        else
        {
            u8RefCnt--;
            if(u8RefCnt == 0)
            {
                Ret = Mask;
            }
            else
                Ret = 0;
        }
    }
    MUTEX_MUTEX_UNLOCK();
    return (u32)Ret;
}
u32 DrvSclOsDisableAllInterrupts(void)
{
    MsIntMask_e Mask;
    Mask = MsDisableInterrupt();
    _DrvSclOsAllInterruptsMask(1,Mask);
    return 1;
}

//-------------------------------------------------------------------------------------------------
/// Restore the interrupts from last DrvSclOsDisableAllInterrupts.
/// @param  u32OldInterrupts \b IN: Interrupt register value from @ref DrvSclOsDisableAllInterrupts
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
bool DrvSclOsRestoreAllInterrupts(void)
{
    MsIntMask_e Mask = 0;
    Mask = _DrvSclOsAllInterruptsMask(0,Mask);
    if(Mask!=0)
        MsEnableInterrupt(Mask);
    return 1;
}
//-------------------------------------------------------------------------------------------------
/// Enable all CPU interrupts.
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
bool DrvSclOsEnableAllInterrupts(void)
{
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/// In Interuupt Context or not
/// @return TRUE : Yes
/// @return FALSE : No
//-------------------------------------------------------------------------------------------------
bool DrvSclOsInInterrupt (void)
{
    return FALSE;
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
        MsInitMutex(&_SclOs_Mutex_Info[s32LstUnused].stMutex);
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
    s32 s32Id, s32LstUnused = SCLOS_TSEM_MAX;
    for(s32Id=0;s32Id<SCLOS_TSEM_MAX;s32Id++)
    {
        if(TRUE == _SclOs_Tsem_Info[s32Id].bUsed)
        {
        }
        if (FALSE==_SclOs_Tsem_Info[s32Id].bUsed  && SCLOS_TSEM_MAX==s32LstUnused)
        {
            s32LstUnused = s32Id;
        }
    }
    if ((SCLOS_TSEM_MAX==s32Id) && (SCLOS_TSEM_MAX>s32LstUnused))
    {
        _SclOs_Tsem_Info[s32LstUnused].bUsed = TRUE;
        CamOsTsemInit(&_SclOs_Tsem_Info[s32LstUnused]._SclOs_Lock_Mutex_t, nVal);
        s32Id = s32LstUnused;
    }
    s32Id |= SCLOS_ID_PREFIX;
    return s32Id;
}
s32 DrvSclOsTsemDeinit(s32 s32Id)
{
    if ( (s32Id & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32Id &= SCLOS_ID_MASK;
    }
    CamOsTsemDeinit(&_SclOs_Tsem_Info[s32Id]._SclOs_Lock_Mutex_t);
    return -1;
}
bool DrvSclOsReleaseTsem(s32 s32Id)
{
    if ( (s32Id & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return 0;
    }
    else
    {
        s32Id &= SCLOS_ID_MASK;
    }
    CamOsTsemUp(&_SclOs_Tsem_Info[s32Id]._SclOs_Lock_Mutex_t);
    return 1;
}
bool DrvSclOsObtainTsem(s32 s32Id)
{
    if ( (s32Id & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return 0;
    }
    else
    {
        s32Id &= SCLOS_ID_MASK;
    }
    CamOsTsemDown(&_SclOs_Tsem_Info[s32Id]._SclOs_Lock_Mutex_t);
    return 1;
}
void DrvSclOsGetSclSpinlockIrq(DrvSclOsSpinlockConfig_t *stSpin)
{
    volatile u32 u32Idx = 0;
    volatile u32 u32Key = 0;
    u16 u16idx = 0;
    for(u16idx = 0;u16idx< gIRQIdx;u16idx++)
    {
        DrvSclOsDisableInterrupt(gIRQNUM[u16idx]);
    }
    (stSpin->u32Access)++;
    u32Idx = (stSpin->u32Access);
    do
    {
        do
        {
        }
        while((stSpin->u32Lock));
        if(stSpin->u32Key && u32Idx==1)
        {
            stSpin->u32Key = 0;
            u32Key = 1;
        }
        else
        {
            if(u32Idx>1)
            {
                u32Idx--;
            }
        }
        if(u32Key)
        {
            break;
        }
    }
    while(1);
}
void DrvSclOsFreeSclSpinlockIrq(DrvSclOsSpinlockConfig_t *stspin)
{
    u16 u16idx = 0;
    for(u16idx = 0;u16idx< gIRQIdx;u16idx++)
    {
        DrvSclOsEnableInterrupt(gIRQNUM[u16idx]);
    }
    stspin->u32Access--;
    stspin->u32Key = 1;
    stspin->u32Lock = 0;
}

void DrvSclOsGetSclSpinlock(DrvSclOsSpinlockConfig_t *stSpin)
{
    volatile u32 u32Idx = 0;
    volatile u32 u32Key = 0;
    (stSpin->u32Access)++;
    u32Idx = (stSpin->u32Access);
    do
    {
        do
        {
        }
        while((stSpin->u32Lock));
        if(stSpin->u32Key && u32Idx==1)
        {
            stSpin->u32Key = 0;
            u32Key = 1;
        }
        else
        {
            if(u32Idx>1)
            {
                u32Idx--;
            }
        }
        if(u32Key)
        {
            break;
        }
    }
    while(1);
}
void DrvSclOsFreeSpinlock(DrvSclOsSpinlockConfig_t *stspin)
{
    stspin->u32Access--;
    stspin->u32Key = 1;
    stspin->u32Lock = 0;
}

void DrvSclOsSpinlockInit(DrvSclOsSpinlockConfig_t *stspin)
{
    stspin->u32Key = 1;
    stspin->u32Lock = 0;
    stspin->u32Access= 0;
}

s32 DrvSclOsCreateSpinlock ( DrvSclOsAttributeType_e eAttribute, char *pMutexName1, u32 u32Flag)
{
    s32 s32Id;
    #if 0
    s32  s32LstUnused = SCLOS_SPINLOCK_MAX;
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
        //spin_lock_init(&_SclOs_Spinlock_Info[s32LstUnused].stMutex);
        DrvSclOsSpinlockInit(&_SclOs_Spinlock_Info[s32LstUnused].stSpin);
        s32Id = s32LstUnused;
    }
    MUTEX_MUTEX_UNLOCK();

    if(SCLOS_SPINLOCK_MAX <= s32Id)
    {
        return -1;
    }

    s32Id |= SCLOS_ID_PREFIX;
    #endif
    s32Id = DrvSclOsCreateMutex(eAttribute,pMutexName1,u32Flag);

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
    return DrvSclOsDeleteMutex(s32MutexId);
}

//-------------------------------------------------------------------------------------------------
/// Attempt to lock a mutex
/// @param  s32MutexId  \b IN: mutex ID
/// @param  u32WaitMs   \b IN: 0 ~ SCLOS_WAIT_FOREVER: suspend time (ms) if the mutex is locked
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
// @FIXME: don't support time-out at this stage
bool DrvSclOsObtainMutex (s32 s32MutexId, u32 u32WaitMs)
{
    if ( (s32MutexId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32MutexId &= SCLOS_ID_MASK;
    }

    MsMutexLock(&(_SclOs_Mutex_Info[s32MutexId].stMutex));

    return TRUE;
}
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
    DrvSclOsDisableAllInterrupts();
    MsMutexLock(&(_SclOs_Mutex_Info[s32MutexId].stMutex));

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
/// Attempt to unlock a mutex
/// @param  s32MutexId  \b IN: mutex ID
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   Only the owner thread of the mutex can unlock it.
//-------------------------------------------------------------------------------------------------
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
    MsMutexUnlock(&(_SclOs_Mutex_Info[s32MutexId].stMutex));
    DrvSclOsRestoreAllInterrupts();

    return TRUE;
}

char * DrvSclOsCheckMutex(char *str,char *end)
{
    return NULL;
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

    MsMutexUnlock(&(_SclOs_Mutex_Info[s32MutexId].stMutex));

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
            if((_SclOs_Mutex_Info[s32Id].stMutex.bLocked))
            {
                bRet =0;
                SCL_ERR("[SCLOS]Mutex Not free %lx %s \n",s32Id,_SclOs_Mutex_Info[s32Id].u8Name);
                MsMutexUnlock(&(_SclOs_Mutex_Info[s32Id].stMutex));
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
DrvSclOsTaskStruct_t DrvSclOsGetTaskinfo(s32 s32Id)
{
    DrvSclOsTaskStruct_t stRet = {NULL};

    return stRet;
}



bool DrvSclOsSetTaskWork(s32 s32TaskId)
{
    MsFlagSetbits(&_SclOs_Task_Flag, (0x1<<(s32TaskId&SCLOS_ID_MASK)));
    return 1;
}

s32 DrvSclOsCreateTask (TaskEntry pTaskEntry,
                     u32 u32TaskEntryData,
                     bool bAutoStart,
                     char *pTaskName)
{
    s32 s32Id;
    s32Id = DrvSclOsCreateWorkQueueTask(pTaskName);
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
    DrvSclOsestroyWorkQueueTask(s32TaskId);
    return TRUE;
}
bool DrvSclOsInit (void)
{
    u32 u32I;
    static bool bInit = FALSE;
    char pName2[] ={"SCLTASK"};
    if(bInit)
    {
        return TRUE;
    }

    MsInitMutex(&_SclOs_Mutex_Mutex);

    //
    // Mutex
    //
    for(u32I=0; u32I<SCLOS_MUTEX_MAX; u32I++)
    {
        _SclOs_Mutex_Info[u32I].bUsed = FALSE;

    }
    for(u32I=0; u32I<SCLOS_TSEM_MAX; u32I++)
    {
        _SclOs_Tsem_Info[u32I].bUsed = FALSE;

    }
    /*
    //
    // Task Management
    //
    MsInitMutex(&_SclOs_Task_Mutex);
    for( u32I=0; u32I<SCLOS_TASK_MAX; u32I++)
    {
        _SclOs_Task_Info[u32I].bUsed = FALSE;
    }
    */

    // Even Group
    MsInitMutex(&_SclOs_EventGroup_Mutex);
    for( u32I=0; u32I<SCLOS_EVENTGROUP_MAX; u32I++)
    {
        _SclOs_EventGroup_Info[u32I].bUsed = FALSE;
    }

    // Clk
    MsInitMutex(&_SclOs_Clk_Mutex);
    for (u32I=0; u32I<SCLOS_CLK_MAX; u32I++)
    {
        _SclOs_Clk_Info[u32I].bUsed = TRUE;
        _SclOs_Clk_Info[u32I].stClk.ptIdclk = &_SclOs_Clk_IDCLK;
        _SclOs_Clk_Info[u32I].stClk.ptFclk1 = &_SclOs_Clk_FCLK1;
        _SclOs_Clk_Info[u32I].stClk.ptFclk2 = &_SclOs_Clk_FCLK2;
        _SclOs_Clk_Info[u32I].stClk.ptOdclk = &_SclOs_Clk_ODCLK;
    }



    //
    // Timer
    //
    MsInitMutex(&_SclOs_Timer_Mutex);
    for( u32I=0; u32I<SCLOS_TIMER_MAX; u32I++)
    {
        _SclOs_Timer_Info[u32I].bUsed = FALSE;
        _SclOs_Timer_Info[u32I].pTimerCb = NULL;
        _SclOs_Timer_Info[u32I].period = 0;
        _SclOs_Timer_Info[u32I].first = 0;
        //init_timer(&(_SclOs_Timer_Info[u32I].timer)); RTK_ToDo
    }
    gu8SclFrameDelay = 0;
    for(u32I=0;u32I<SCLOS_IRQ_MAX;u32I++)
    {
        gIRQNUM[u32I] = 0;
    }
    gIRQIdx = 0;
    // task

    s32Taskid = DrvSclOsCreateWorkQueueTask(pName2);

    bInit = TRUE;

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
        _SclOs_EventGroup_Info[s32Id].u32Waitflag = 0;
        memset(&_SclOs_EventGroup_Info[s32Id].sFlag, 0, sizeof(Ms_Flag_t));
        MsFlagInit(&_SclOs_EventGroup_Info[s32Id].sFlag);
        MsFlagMaskbits(&_SclOs_EventGroup_Info[s32Id].sFlag, 0x00000000);
        //DrvSclOsSpinlockInit(&_SclOs_Spinlock_Info[s32LstUnused].stSpin);
        MsInitMutex(&_SclOs_EventGroup_Info[s32Id].sMutexEvent);

    }
    EVENT_MUTEX_UNLOCK();

    if(s32Id >= SCLOS_EVENTGROUP_MAX)
    {
        return -1;
    }

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
    if ( (s32EventGroupId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32EventGroupId &= SCLOS_ID_MASK;
    }

    EVENT_MUTEX_LOCK();
    _SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup= 0;
    _SclOs_EventGroup_Info[s32EventGroupId].bUsed = FALSE;
    _SclOs_EventGroup_Info[s32EventGroupId].u32Waitflag = 0;
    MsFlagDestroy(&_SclOs_EventGroup_Info[s32EventGroupId].sFlag);

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
bool DrvSclOsDeleteEventGroupRing (s32 s32EventGroupId)
{
    if ( (s32EventGroupId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32EventGroupId &= SCLOS_ID_MASK;
    }

    EVENT_MUTEX_LOCK();
    DrvSclOsMemset(&_SclOs_RingEventGroup_Info[s32EventGroupId],0,sizeof(DrvSclOsRingEventGroupInfoConfig_t));
    EVENT_MUTEX_UNLOCK();
    return TRUE;
}
bool DrvSclOsCreateEventGroupRing (u8 u8Id)
{
    EVENT_MUTEX_LOCK();
    if(_SclOs_RingEventGroup_Info[u8Id].bUsed == TRUE)
    {
        return 0;
    }
    if(u8Id < SCLOS_EVENTGROUP_MAX)
    {
        _SclOs_RingEventGroup_Info[u8Id].bUsed = TRUE;
        _SclOs_RingEventGroup_Info[u8Id].u32EventGroup= 0;
        _SclOs_RingEventGroup_Info[u8Id].u8Rpoint= 0;
        _SclOs_RingEventGroup_Info[u8Id].u8Wpoint= 0;
    }
    EVENT_MUTEX_UNLOCK();

    if(u8Id >= SCLOS_EVENTGROUP_MAX)
    {
        return 0;
    }
    u8Id |= SCLOS_ID_PREFIX;
    return 1;
}
bool DrvSclOsSetEventRing (s32 s32EventGroupId)
{
    SET_FLAG(_SclOs_RingEventGroup_Info[s32EventGroupId].u32EventGroup,
        (u32)(0x1 << _SclOs_RingEventGroup_Info[s32EventGroupId].u8Wpoint));
    _SclOs_RingEventGroup_Info[s32EventGroupId].u8Wpoint++;
    if(_SclOs_RingEventGroup_Info[s32EventGroupId].u8Wpoint>=32)
    {
        _SclOs_RingEventGroup_Info[s32EventGroupId].u8Wpoint = 0;
    }
    if(_SclOs_RingEventGroup_Info[s32EventGroupId].u8Wpoint == _SclOs_RingEventGroup_Info[s32EventGroupId].u8Rpoint)
    {
        SCL_ERR("SetEventRing BIT FULL W:%hhd ,R:%hhd\n",_SclOs_RingEventGroup_Info[s32EventGroupId].u8Wpoint,
            _SclOs_RingEventGroup_Info[s32EventGroupId].u8Rpoint);
    }
    return TRUE;
}
u32 DrvSclOsGetandClearEventRing(u32 u32EventGroupId)
{
    u32 u32Event = 0;
    u8 u8Count = 0;
    u32Event = HAS_FLAG(_SclOs_RingEventGroup_Info[u32EventGroupId].u32EventGroup,
        (u32)(0x1 << _SclOs_RingEventGroup_Info[u32EventGroupId].u8Rpoint));
    if(u32Event)
    {
        while(_SclOs_RingEventGroup_Info[u32EventGroupId].u8Rpoint != _SclOs_RingEventGroup_Info[u32EventGroupId].u8Wpoint)
        {
            RESET_FLAG(_SclOs_RingEventGroup_Info[u32EventGroupId].u32EventGroup,
                (u32)(0x1 << _SclOs_RingEventGroup_Info[u32EventGroupId].u8Rpoint));
            _SclOs_RingEventGroup_Info[u32EventGroupId].u8Rpoint++;
            if(_SclOs_RingEventGroup_Info[u32EventGroupId].u8Rpoint>=32)
            {
                _SclOs_RingEventGroup_Info[u32EventGroupId].u8Rpoint = 0;
            }
            u8Count++;
        }
        if(u8Count >1)
        {
            SCL_ERR("GetEvent Count:%hhd IST too late\n",u8Count);
        }
        return u8Count;
    }
    return 0;
}
void _DrvSclOsReleaseEventMutexIrq(s32 s32EventGroupId)
{
    MsMutexUnlock(&_SclOs_EventGroup_Info[s32EventGroupId].sMutexEvent);
    DrvSclOsRestoreAllInterrupts();
}
void _DrvSclosObtainEventMutexIrq(s32 s32EventGroupId)
{
    DrvSclOsDisableAllInterrupts();
    MsMutexLock(&_SclOs_EventGroup_Info[s32EventGroupId].sMutexEvent);
}
void _DrvSclOsFreeWaitFlagIrq(s32 s32EventGroupId ,u32 u32Flag)
{
    _DrvSclosObtainEventMutexIrq(s32EventGroupId);
    _SclOs_EventGroup_Info[s32EventGroupId].u32Waitflag &= ~(u32Flag);
    _DrvSclOsReleaseEventMutexIrq(s32EventGroupId);
}
u32 _DrvSclOsGetWaitFlagIrq(s32 s32EventGroupId )
{
    u8 idx = 0;
    for(idx =0 ;idx<SCLOS_FLAG_MAX ;idx++)
    {
        if(!(_SclOs_EventGroup_Info[s32EventGroupId].u32Waitflag & ((0x1)<<idx)))
        {
            _DrvSclosObtainEventMutexIrq(s32EventGroupId);
            _SclOs_EventGroup_Info[s32EventGroupId].u32Waitflag |= (0x1)<<idx;
            _DrvSclOsReleaseEventMutexIrq(s32EventGroupId);
            break;
        }
    }
    return (u32)((0x1)<<idx);
}
Ms_flag_value_t _DrvSclOsGetAllWaitFlag(s32 s32EventGroupId)
{
    u8 idx = 0;
    Ms_flag_value_t flagval = 0;
    if(_SclOs_EventGroup_Info[s32EventGroupId].u32Waitflag)
    {
        for(idx =0 ;idx<SCLOS_FLAG_MAX ;idx++)
        {
            if(_SclOs_EventGroup_Info[s32EventGroupId].u32Waitflag & ((0x1)<<idx))
            {
                flagval |= (0x1)<<idx;
            }
        }
    }
    else
    {
        flagval = 0;
    }
    return flagval;
}
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

    _DrvSclosObtainEventMutexIrq(s32EventGroupId);
    SET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32EventFlag);
    _DrvSclOsReleaseEventMutexIrq(s32EventGroupId);
    MsFlagSetbits(&_SclOs_EventGroup_Info[s32EventGroupId].sFlag, (Ms_flag_value_t)_DrvSclOsGetAllWaitFlag(s32EventGroupId));
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

    SET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32EventFlag);
    MsFlagSetbits(&_SclOs_EventGroup_Info[s32EventGroupId].sFlag, (Ms_flag_value_t)_DrvSclOsGetAllWaitFlag(s32EventGroupId));
    return TRUE;
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

    RESET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32EventFlag);
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

    _DrvSclosObtainEventMutexIrq(s32EventGroupId);
    RESET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32EventFlag);
    _DrvSclOsReleaseEventMutexIrq(s32EventGroupId);
    return TRUE;
}
s32 DrvSclOsCreateWorkQueueEvent(void * pTaskEntry)
{
    s32 s32Id;
    EVENT_MUTEX_LOCK();

    for( s32Id=0; s32Id<SCLOS_WORK_MAX; s32Id++)
    {
        if(_SclOS_WorkEvent_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if( s32Id >= SCLOS_WORK_MAX)
    {
        return -1;
    }

    _SclOS_WorkEvent_Info[s32Id].bUsed = TRUE;
    MsFlagInit(&_SclOS_WorkEvent_Info[s32Id].sFlag);
    MsFlagMaskbits(&_SclOS_WorkEvent_Info[s32Id].sFlag, 0x00000000);
    EVENT_MUTEX_UNLOCK();
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
    EVENT_MUTEX_LOCK();
    _SclOS_WorkQueue_Info[s32Id].bUsed = FALSE;

    EVENT_MUTEX_UNLOCK();
    return TRUE;
}

s32 DrvSclOsCreateWorkQueueTask(char *pTaskName)
{
    s32 s32Id;
    EVENT_MUTEX_LOCK();

    for( s32Id=0; s32Id<SCLOS_WORKQUEUE_MAX; s32Id++)
    {
        if(_SclOS_WorkQueue_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if( s32Id >= SCLOS_WORKQUEUE_MAX)
    {
        return -1;
    }

    _SclOS_WorkQueue_Info[s32Id].bUsed = TRUE;

    EVENT_MUTEX_UNLOCK();
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
    EVENT_MUTEX_LOCK();
    if(u32WaitMs)
    {
        DrvSclOsDelayTask(u32WaitMs);
        MsFlagSetbits(&_SclOS_WorkEvent_Info[s32QueueId].sFlag, 0x1);
        MsFlagSetbits(&_SclOs_Task_Flag, (0x1<<s32TaskId));
    }
    else
    {
        MsFlagSetbits(&_SclOS_WorkEvent_Info[s32QueueId].sFlag, 0x1);
        MsFlagSetbits(&_SclOs_Task_Flag, (0x1<<s32TaskId));
    }
    EVENT_MUTEX_UNLOCK();
    return bRet;
}
bool DrvSclOsQueueWait(s32 s32QueueId)
{
    bool bRet;
    if ( (s32QueueId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32QueueId &= SCLOS_ID_MASK;
    }
    if(MsFlagPeek(&_SclOS_WorkEvent_Info[s32QueueId].sFlag)&0x1)
    {
        MsFlagMaskbits(&_SclOS_WorkEvent_Info[s32QueueId].sFlag, 0x0);
        bRet = 1;
    }
    else
    {
        bRet = 0;
    }
    return bRet;
}

void DrvSclOsTaskWait(u32 u32Waitflag)
{
    static bool buse = 0;
    if(!buse)
    {
        MsFlagInit(&_SclOs_Task_Flag);
        MsFlagMaskbits(&_SclOs_Task_Flag, 0x00000000);
        buse = 1;
    }
    MsFlagWait(&_SclOs_Task_Flag,
               (Ms_flag_value_t)u32Waitflag,
               (MS_FLAG_WAITMODE_OR | MS_FLAG_WAITMODE_CLR));
}
bool DrvSclOsGetQueueExist(s32 s32QueueId)
{
    if ( (s32QueueId & SCLOS_ID_PREFIX_MASK) != SCLOS_ID_PREFIX )
    {
        return FALSE;
    }
    else
    {
        s32QueueId &= SCLOS_ID_MASK;
    }
    return _SclOS_WorkEvent_Info[s32QueueId].bUsed;
}

bool DrvSclOsFlushWorkQueue(bool bTask, s32 s32TaskId)
{
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
    Ms_flag_mode_t eFlagMode;
    u32 u32Waitflag;
    bool bTimeout = 1;
    bool bAnd;
    bool bClear;
    bool bRet;
    u32 u32Time = 0;
    u32 u32DiffTime = 0;
    u32 u32WaitMsForTimeOut = u32WaitMs;
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
    eFlagMode = (eWaitMode ==  E_DRV_SCLOS_EVENT_MD_AND)       ? (MS_FLAG_WAITMODE_AND) :
                (eWaitMode ==  E_DRV_SCLOS_EVENT_MD_OR)        ? (MS_FLAG_WAITMODE_OR)  :
                (eWaitMode ==  E_DRV_SCLOS_EVENT_MD_AND_CLEAR) ? (MS_FLAG_WAITMODE_AND | MS_FLAG_WAITMODE_CLR) :
                (eWaitMode ==  E_DRV_SCLOS_EVENT_MD_OR_CLEAR)  ? (MS_FLAG_WAITMODE_OR  | MS_FLAG_WAITMODE_CLR)  :
                                               0;

    if(eFlagMode == 0)
    {
        return FALSE;
    }
    bClear= ((E_DRV_SCLOS_EVENT_MD_AND_CLEAR== eWaitMode) || (E_DRV_SCLOS_EVENT_MD_OR_CLEAR== eWaitMode))? TRUE: FALSE;
    bAnd= ((E_DRV_SCLOS_EVENT_MD_AND== eWaitMode)|| (E_DRV_SCLOS_EVENT_MD_AND_CLEAR== eWaitMode))? TRUE: FALSE;

    u32Waitflag = _DrvSclOsGetWaitFlagIrq(s32EventGroupId);
    if(u32WaitMs == SCLOS_WAIT_FOREVER)
    {
        if (bAnd)
        {
            while(!(HAS_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32WaitEventFlag)== u32WaitEventFlag))
            {
                MsFlagWait(&_SclOs_EventGroup_Info[s32EventGroupId].sFlag,
                           (Ms_flag_value_t)u32Waitflag,
                           eFlagMode);
            }
        }
        else
        {
            while(!HAS_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32WaitEventFlag))
            {
                MsFlagWait(&_SclOs_EventGroup_Info[s32EventGroupId].sFlag,
                           (Ms_flag_value_t)u32Waitflag,
                           eFlagMode);
            }
        }
    }
    else
    {
        if (bAnd)
        {
            while(!(HAS_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32WaitEventFlag)== u32WaitEventFlag))
            {
                u32Time = DrvSclOsGetSystemTime();
                bTimeout = MsFlagTimedWait(&_SclOs_EventGroup_Info[s32EventGroupId].sFlag,
                                (Ms_flag_value_t)u32Waitflag,
                                eFlagMode,
                                VM_RTK_MS_TO_TICK(u32WaitMsForTimeOut));
                u32DiffTime = (DrvSclOsGetSystemTime()>=u32Time)? DrvSclOsGetSystemTime()-u32Time : DrvSclOsGetSystemTime();
                if(!bTimeout)
                {
                    //SCL_ERR("[SCLOS]wait timeout\n");
                    break;
                }
                else
                {
                    if(u32WaitMsForTimeOut > u32DiffTime)
                    {
                        u32WaitMsForTimeOut -= u32DiffTime;
                    }
                    else
                    {
                        //SCL_ERR("[SCLOS]Wait Diff timeout\n");
                        bTimeout = 0;
                        break;
                    }
                }
            }
        }
        else
        {
            while(!HAS_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32WaitEventFlag))
            {
                u32Time = DrvSclOsGetSystemTime();
                bTimeout = MsFlagTimedWait(&_SclOs_EventGroup_Info[s32EventGroupId].sFlag,
                                (Ms_flag_value_t)u32Waitflag,
                                eFlagMode,
                                VM_RTK_MS_TO_TICK(u32WaitMsForTimeOut));
                u32DiffTime = (DrvSclOsGetSystemTime()>=u32Time)? DrvSclOsGetSystemTime()-u32Time : DrvSclOsGetSystemTime();
                if(!bTimeout)
                {

                    //SCL_ERR("[SCLOS]wait timeout\n");
                    break;
                }
                else
                {
                    if(u32WaitMsForTimeOut > u32DiffTime)
                    {
                        u32WaitMsForTimeOut -= u32DiffTime;
                    }
                    else
                    {
                        //SCL_ERR("[SCLOS]Wait Diff timeout\n");
                        bTimeout = 0;
                        break;
                    }
                }
            }
        }
    }
    _DrvSclOsFreeWaitFlagIrq(s32EventGroupId,u32Waitflag);

    if(pu32RetrievedEventFlag)
    {
        *pu32RetrievedEventFlag= HAS_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, u32WaitEventFlag);
    }
    bRet= (bAnd)? (*pu32RetrievedEventFlag== u32WaitEventFlag): (0!= *pu32RetrievedEventFlag);
    if (bRet && bClear)
    {
        _DrvSclosObtainEventMutexIrq(s32EventGroupId);
        RESET_FLAG(_SclOs_EventGroup_Info[s32EventGroupId].u32EventGroup, *pu32RetrievedEventFlag);
        _DrvSclOsReleaseEventMutexIrq(s32EventGroupId);
    }
    return (bTimeout) ? 0 :1;
}


void* DrvSclOsGetEventQueue (s32 *s32EventGroupId)
{
    return (void *)s32EventGroupId;
}

//
// Timer management
//
/* RTK_ToDo
static void _DrvSclosTimerNotify(unsigned long data)
{
}
*/

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
    return SCLOS_ID_PREFIX;
}

//-------------------------------------------------------------------------------------------------
/// Delete the Timer
/// @param  s32TimerId  \b IN: Timer ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsDeleteTimer (s32 s32TimerId)
{
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Start the Timer
/// @param  s32TimerId  \b IN: Timer ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
bool DrvSclOsStartTimer (s32 s32TimerId)
{
    return TRUE;
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
    return TRUE;
}

s32 DrvSclOsCreateTasklet(void * pTaskEntry,u32 u32data)
{
    return SCLOS_ID_PREFIX;
}

bool DrvSclOsDestroyTasklet(s32 s32Id)
{
    return TRUE;
}


bool DrvSclOsEnableTasklet (s32 s32Id)
{
    return TRUE;
}


bool DrvSclOsDisableTasklet (s32 s32Id)
{
    return TRUE;
}

bool DrvSclOsTaskletWork(s32 s32TaskId)
{
    return TRUE;
}

void DrvSclOsMsSleep(u32 nMsec)
{
    MsSleep(RTK_MS_TO_TICK(nMsec));
}

void* DrvSclOsMemalloc(u32 size, u32 flags)
{
    return MsAllocateNonCacheMem(size);
}

void DrvSclOsMemFree(void *pVirAddr)
{
    MsReleaseMemory(pVirAddr);
}

void* DrvSclOsVirMemalloc(u32 size)
{
    return MsAllocateMem(size);
}

void DrvSclOsVirMemFree(void *pVirAddr)
{
    MsReleaseMemory(pVirAddr);
}

void* DrvSclOsMemcpy(void *pstCfg,const void *pstInformCfg,u32 size)
{
    memcpy(pstCfg, pstInformCfg, size);
    return 0;
}

void* DrvSclOsMemset(void *pstCfg,int val,u32 size)
{
    memset(pstCfg, val, size);
    return 0;
}

void DrvSclOsStrcat(char *pstCfg,char *name)
{
    strcat(pstCfg, (const char*)name);
}

bool DrvSclOsClkSetConfig(DrvSclOsClkIdType_e enClkId, DrvSclOsClkConfig_t stClk)
{
    bool bRet = FALSE;
    if(_SclOs_Clk_Info[enClkId].bUsed == TRUE)
    {
        SCL_ERR("%s %d, Id=%d is not Empty\n", __FUNCTION__, __LINE__, enClkId);
        bRet = FALSE;
    }
    else
    {
        _SclOs_Clk_Info[enClkId].bUsed = TRUE;
        _SclOs_Clk_Info[enClkId].stClk.ptIdclk = stClk.ptIdclk;
        _SclOs_Clk_Info[enClkId].stClk.ptFclk1 = stClk.ptFclk1;
        _SclOs_Clk_Info[enClkId].stClk.ptFclk2 = stClk.ptFclk2;
        _SclOs_Clk_Info[enClkId].stClk.ptOdclk = stClk.ptOdclk;
        bRet = TRUE;
    }
    return bRet;
}


DrvSclOsClkConfig_t *DrvSclOsClkGetConfig(DrvSclOsClkIdType_e enClkId)
{
    DrvSclOsClkConfig_t *pClkCfg = NULL;

    if(_SclOs_Clk_Info[enClkId].bUsed == FALSE)
    {
        SCL_ERR("%s %d, Id=%d is NULL\n", __FUNCTION__, __LINE__, enClkId);
        pClkCfg = NULL;
    }
    else
    {
        pClkCfg = &_SclOs_Clk_Info[enClkId].stClk;
    }
    return pClkCfg;
}


u32 DrvSclOsClkGetEnableCount(DrvSclOsClkStruct_t * clk)
{
    return DrvClkgenTopGetClkEnableCount(clk->eTopClk);
}

DrvSclOsClkStruct_t * DrvSclOsClkGetParentByIndex(DrvSclOsClkStruct_t * clk,u8 index)
{
    DrvSclOsMemcpy(&_SclOs_Clk_ParentCLK,clk,sizeof(DrvSclOsClkStruct_t));
    _SclOs_Clk_ParentCLK.u16Src = index;
    clk->u16Src = index;
    return &_SclOs_Clk_ParentCLK;
}

u32 DrvSclOsClkSetParent(DrvSclOsClkStruct_t *clk, DrvSclOsClkStruct_t *parent)
{
    DrvClkgenDrvClockSelectSrc(clk->eModule,clk->eClk,clk->u16Src);
    return 1;
}

u32 DrvSclOsClkPrepareEnable(DrvSclOsClkStruct_t *clk)
{
    DrvClkgenDrvClockCtl(clk->eModule,CLKGEN_CLK_REQ_ON);
    return 1;
}

void DrvSclOsClkDisableUnprepare(DrvSclOsClkStruct_t *clk)
{
    DrvClkgenDrvClockCtl(clk->eModule,CLKGEN_CLK_REQ_OFF);
}

u32 DrvSclOsClkGetRate(DrvSclOsClkStruct_t *clk)
{
    if(clk->u16Src == 3)
    {
        return 432000000;
    }
    else
    {
        return DrvClkgenTopGetClkRate(clk->eTopClk);
    }
}


u32 DrvSclOsCopyFromUser(void *to, const void  *from, u32 n)
{
    memcpy(to, from, n);
    return 0;
}
u32 DrvSclOsCopyToUser(void *to, const void  *from, u32 n)
{
    memcpy(to, from, n);
    return 0;
}

void DrvSclOsWaitForCpuWriteToDMem(void)
{
}

u8 DrvSclOsGetSclFrameDelay(void)
{
    return gu8SclFrameDelay;
}
void DrvSclOsSetSclFrameDelay(u8 u8delay)
{
    gu8SclFrameDelay = u8delay;
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

u32 DrvSclOsVa2Pa(u32 addr)
{
    return (u32)MsVA2PA((void *)(addr));
}

u32 DrvSclOsPa2Miu(DrvSclOsDmemBusType_t addr)
{
    return HalUtilPHY2MIUAddr(addr);
}
DrvSclOsDmemBusType_t DrvSclOsMiu2Pa(DrvSclOsDmemBusType_t addr)
{
    return HalUtilMIU2PHYAddr(addr);
}

void DrvSclOsDirectMemFlush(unsigned long u32Addr, unsigned long u32Size)
{

}

void* DrvSclOsDirectMemAlloc(const char* name, u32 size, DrvSclOsDmemBusType_t *addr)
{
    void *pBuf = NULL;;

    pBuf = DrvSclOsMemalloc(size, 0);
    if(pBuf == NULL)
    {
        return NULL;
    }

    *addr = DrvSclOsVa2Pa((u32)pBuf);
    *addr = DrvSclOsPa2Miu(*addr);
    sclprintf("%s va=0x%x pa=0x%x\n", __FUNCTION__,(u32)pBuf,(u32)*addr );
    return pBuf;

}

void DrvSclOsDirectMemFree(const char* name, u32 size, void *virt, DrvSclOsDmemBusType_t addr)
{
    DrvSclOsMemFree(virt);
}
void DrvSclOsSetDbgTaskWork(void)
{
    DrvSclOsSetTaskWork((s32)(s32Taskid&0xFFFF) );
    // add wait event block app
    DrvSclOsTaskWait((u32)(0x1 <<(SCLOS_WORKQUEUE_MAX) ));
}
void SCL_Task(void)
{
    //char pst[64];
    static bool bopen = 0;
    while(1)
    {
        if(!bopen)
        {
            //DrvSclProcProbeAllDevice();
            bopen = 1;
        }
        DrvSclOsTaskWait((u32)(0x1 <<(s32Taskid&0xFFFF) ));
        sclprintf("%s %d going\n", __FUNCTION__, __LINE__);
        //DrvSclProcScanf(pst);
        // handle event
        //DrvSclProcParser(pst);

        // add set event to wake up APP
        DrvSclOsSetTaskWork((s32)(SCLOS_WORKQUEUE_MAX) );
    }
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
    }
}
bool DrvSclOsGetClkForceMode(void)
{
    return gbclkforcemode;
}
void DrvSclOsSetClkForceMode(bool bEn)
{
    gbclkforcemode = bEn;
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
int DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_TYPE enType)
{
    return gSCLIRQID[enType];
}
int DrvSclOsGetIrqIDCMDQ(E_DRV_SCLOS_CMDQIRQ_TYPE enType)
{
    return gCMDQIRQID[enType];
}
int DrvSclOsSetSclIrqIDFormSys(E_DRV_SCLOS_SCLIRQ_TYPE enType)
{
    unsigned int SCL_IRQ_ID = 0; //INT_IRQ_AU_SYSTEM;
    SCL_IRQ_ID  = MS_INT_NUM_IRQ_121_sc_top_int_0;
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[MSOS] Get resource SCL_IRQ = 0x%x\n",SCL_IRQ_ID);
    gSCLIRQID[enType] = SCL_IRQ_ID;

    return 0;
}
int DrvSclOsSetCmdqIrqIDFormSys(E_DRV_SCLOS_CMDQIRQ_TYPE enType)
{
    unsigned int CMDQ_IRQ_ID = 0; //INT_IRQ_AU_SYSTEM;
    CMDQ_IRQ_ID = MS_INT_NUM_IRQ_73_CMDQ_INT;

    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[MSOS] Get resource CMDQ_IRQ = 0x%x\n",CMDQ_IRQ_ID);
    gCMDQIRQID[enType] = CMDQ_IRQ_ID;

    return 0;
}
void * DrvSclOsClkGetClk(u32 idx)
{
    switch(idx)
    {
        case 0:
            return (void*)&_SclOs_Clk_IDCLK ;
            break;
        case 1:
            return (void*)&_SclOs_Clk_FCLK1 ;
            break;
        case 2:
            return (void*)&_SclOs_Clk_FCLK2 ;
            break;
        case 3:
            return (void*)&_SclOs_Clk_ODCLK ;
            break;
        default:

            break;
    }
    return (void*)&_SclOs_Clk_IDCLK ;
}
