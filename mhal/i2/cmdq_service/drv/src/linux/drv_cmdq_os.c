#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include "hal_cmdq_dbg.h"
#include "drv_cmdq_os.h"
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
    if(request_irq(eIntNum, (irq_handler_t)pIntCb, flags, name, priv))
    {
        CMDQ_ERR("%s request irq err(%d)\n",__FUNCTION__,eIntNum);
        return -1;
    }

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
    free_irq(eIntNum, priv);
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
    enable_irq((int)eIntNum);
    return 0;
}

//-------------------------------------------------------------------------------------------------
/// Disable (mask) the interrupt #
/// @param  eIntNum \b IN: Interrupt number
/// @return TRUE : succeed
//-------------------------------------------------------------------------------------------------
int DrvCmdqOsDisableInterrupt (u32 eIntNum)
{
    disable_irq((int)eIntNum);
    return 0;
}

void DrvCmdqOsDelayTask (u32 u32Ms)
{
    msleep_interruptible(u32Ms);
}
