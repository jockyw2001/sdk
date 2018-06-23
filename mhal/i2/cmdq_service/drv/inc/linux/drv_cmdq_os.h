#ifndef __DRV_CMDQ_OS_H__
#define __DRV_CMDQ_OS_H__

#include <linux/irqreturn.h>
#include <linux/interrupt.h>

typedef irqreturn_t ( *InterruptCb ) (u32 eIntNum, void* dev_id);
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
#endif