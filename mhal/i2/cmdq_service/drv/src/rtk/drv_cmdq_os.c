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
#include "drv_cmdq_os.h"

typedef struct
{
    bool             bUsed;
} DrvCmdqOsWorkQueueInfoConfig_t;

typedef struct
{
    bool             bUsed;
    Ms_Flag_t                sFlag;         //like wait queue
} DrvCmdqOsWorkEventInfoConfig_t;

//static DrvSclOsTaskInfoConfig_t   _SclOs_Task_Info[SCLOS_TASK_MAX];
static DrvCmdqOsWorkQueueInfoConfig_t   _gCmdqOS_WorkQueue_Info[CMDQOS_WORKQUEUE_MAX];
static DrvCmdqOsWorkEventInfoConfig_t   _gCmdqOS_WorkEvent_Info[CMDQOS_WORK_MAX];
Ms_Flag_t                               _gCmdqOS_Task_Flag;         //like wait queue

u32 gCmdqIRQIdx = 0;
u32 gIrqNum[CMDQOS_IRQ_MAX];
void * gIrqNumPrivData[CMDQOS_IRQ_MAX];

static                          Ms_Mutex_t _CmdqOs_EventGroup_Mutex;
#define EVENT_MUTEX_LOCK()      MsMutexLock(&_CmdqOs_EventGroup_Mutex)
#define EVENT_MUTEX_UNLOCK()    MsMutexUnlock(&_CmdqOs_EventGroup_Mutex)


int DrvCmdqOsInit (void)
{
    static bool bInit = FALSE;

    if(bInit)
    {
        return 0;
    }

      // Even Group
    MsInitMutex(&_CmdqOs_EventGroup_Mutex);
    bInit = TRUE;
    return 0;
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
void* DrvCmdqOsGetIntPrivData(u32 eIntNum)
{
    u32 i;
    u32 bfound = 0;

    for(i=0 ; i<CMDQOS_IRQ_MAX;i++)
    {
        if(gIrqNum[i] == eIntNum)
        {
            bfound = 1;
            break;
        }
    }

    if(bfound)
        return gIrqNumPrivData[i];

    return NULL;
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
int DrvCmdqOsAttachInterrupt (u32 eIntNum, InterruptCb pIntCb,unsigned long flags,const char *name,void* priv)
{
    MsIntInitParam_u uInitParam;
    uInitParam.intc.eMap = INTC_MAP_IRQ;
    uInitParam.intc.ePriority = INTC_PRIORITY_3;
    uInitParam.intc.pfnIsr = pIntCb;
    gIrqNum[gCmdqIRQIdx] = eIntNum;
    gIrqNumPrivData[gCmdqIRQIdx]=priv;
    gCmdqIRQIdx++;
    MsInitInterrupt(&uInitParam, eIntNum);

    return 0;
}

//-------------------------------------------------------------------------------------------------
/// Detach the interrupt callback function from interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
int DrvCmdqOsDetachInterrupt (u32 eIntNum,void* priv)
{
    return 0;
}

//-------------------------------------------------------------------------------------------------
/// Enable (unmask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
int DrvCmdqOsEnableInterrupt (u32 eIntNum)
{
    MsUnmaskInterrupt((MsIntNumber_e)eIntNum);
    return 0;
}

//-------------------------------------------------------------------------------------------------
/// Disable (mask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
int DrvCmdqOsDisableInterrupt (u32 eIntNum)
{
    MsMaskInterrupt((MsIntNumber_e)eIntNum);
    return 0;
}

void DrvCmdqOsDelayTask (u32 u32Ms)
{
    MsSleep(u32Ms);
}

s32 DrvCmdqOsCreateWorkQueueTask(char *pTaskName)
{
    s32 s32Id;

    DrvCmdqOsInit();
    EVENT_MUTEX_LOCK();

    for( s32Id=0; s32Id<CMDQOS_WORKQUEUE_MAX; s32Id++)
    {
        if(_gCmdqOS_WorkQueue_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if( s32Id >= CMDQOS_WORKQUEUE_MAX)
    {
        return -1;
    }

    _gCmdqOS_WorkQueue_Info[s32Id].bUsed = TRUE;

    EVENT_MUTEX_UNLOCK();
    s32Id |= CMDQOS_ID_PREFIX;
    return s32Id;
}

s32 DrvCmdqOsCreateWorkQueueEvent(void * pTaskEntry)
{
    s32 s32Id;

    DrvCmdqOsInit();
    EVENT_MUTEX_LOCK();

    for( s32Id=0; s32Id<CMDQOS_WORK_MAX; s32Id++)
    {
        if(_gCmdqOS_WorkEvent_Info[s32Id].bUsed == FALSE)
        {
            break;
        }
    }
    if( s32Id >= CMDQOS_WORK_MAX)
    {
        return -1;
    }

    _gCmdqOS_WorkEvent_Info[s32Id].bUsed = TRUE;
    MsFlagInit(&_gCmdqOS_WorkEvent_Info[s32Id].sFlag);
    MsFlagMaskbits(&_gCmdqOS_WorkEvent_Info[s32Id].sFlag, 0x00000000);
    EVENT_MUTEX_UNLOCK();
    s32Id |= CMDQOS_ID_PREFIX;
    return s32Id;
}

int DrvCmdqOsQueueWork(bool bTask, s32 s32TaskId, s32 s32QueueId, u32 u32WaitMs)
{
    int bRet = 0;

    DrvCmdqOsInit();
    if(bTask)
    {
        if ( (s32TaskId & CMDQOS_ID_PREFIX_MASK) != CMDQOS_ID_PREFIX )
        {
            return -1;
        }
        else
        {
            s32TaskId &= CMDQOS_ID_MASK;
        }
    }
    if ( (s32QueueId & CMDQOS_ID_PREFIX_MASK) != CMDQOS_ID_PREFIX )
    {
        return -1;
    }
    else
    {
        s32QueueId &= CMDQOS_ID_MASK;
    }
    EVENT_MUTEX_LOCK();
    if(u32WaitMs)
    {
        DrvCmdqOsDelayTask(u32WaitMs);
        MsFlagSetbits(&_gCmdqOS_WorkEvent_Info[s32QueueId].sFlag, 0x1);
        MsFlagSetbits(&_gCmdqOS_Task_Flag, (0x1<<s32TaskId));
    }
    else
    {
        MsFlagSetbits(&_gCmdqOS_WorkEvent_Info[s32QueueId].sFlag, 0x1);
        MsFlagSetbits(&_gCmdqOS_Task_Flag, (0x1<<s32TaskId));
    }
    EVENT_MUTEX_UNLOCK();
    return bRet;
}
int DrvCmdqOsFlushWorkQueue(bool bTask, s32 s32TaskId)
{
    DrvCmdqOsInit();
    return 0;
}
int DrvCmdqOsDestroyWorkQueueTask(s32 s32Id)
{
    DrvCmdqOsInit();
    if ( (s32Id & CMDQOS_ID_PREFIX_MASK) != CMDQOS_ID_PREFIX )
    {
        return -1;
    }
    else
    {
        s32Id &= CMDQOS_ID_MASK;
    }
    EVENT_MUTEX_LOCK();
    _gCmdqOS_WorkQueue_Info[s32Id].bUsed = FALSE;

    EVENT_MUTEX_UNLOCK();
    return 0;
}
