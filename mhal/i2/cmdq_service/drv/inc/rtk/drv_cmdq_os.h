#ifndef __DRV_CMDQ_OS_H__
#define __DRV_CMDQ_OS_H__

#include <stdio.h>
#include <string.h>

#ifndef true
/// definition for true
#define true                        1
/// definition for false
#define false                       0
#endif

#define CMDQOS_ID_PREFIX              0xfedc0000
#define CMDQOS_ID_PREFIX_MASK         0xFFFF0000
#define CMDQOS_ID_MASK                0x0000FFFF //~SCLOS_ID_PREFIX_MASK

#define CMDQOS_IRQ_MAX            (5*2)
#define CMDQOS_WORKQUEUE_MAX      8
#define CMDQOS_WORK_MAX           8

#ifndef  bool
//#define  bool   unsigned char
typedef unsigned char   bool;
#endif
typedef void ( *InterruptCb ) (void);        ///< Interrupt callback function

void* DrvCmdqOsGetIntPrivData(u32 eIntNum);
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
int DrvCmdqOsAttachInterrupt (u32 eIntNum, InterruptCb pIntCb,unsigned long flags,const char *name,void* priv);


//-------------------------------------------------------------------------------------------------
/// Detach the interrupt callback function from interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
int DrvCmdqOsDetachInterrupt (u32 eIntNum,void* priv);


//-------------------------------------------------------------------------------------------------
/// Enable (unmask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
/// @return FALSE :  fail
//-------------------------------------------------------------------------------------------------
int DrvCmdqOsEnableInterrupt (u32 eIntNum);

//-------------------------------------------------------------------------------------------------
/// Disable (mask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
int DrvCmdqOsDisableInterrupt (u32 eIntNum);
void DrvCmdqOsDelayTask (u32 u32Ms);

#if 0

s32 DrvCmdqOsCreateWorkQueueTask(char *pTaskName);

s32 DrvCmdqOsCreateWorkQueueEvent(void * pTaskEntry);

int DrvCmdqOsQueueWork(bool bTask, s32 s32TaskId, s32 s32QueueId, u32 u32WaitMs);

int DrvCmdqOsFlushWorkQueue(bool bTask, s32 s32TaskId);

int DrvCmdqOsDestroyWorkQueueTask(s32 s32Id);
#endif
#endif
