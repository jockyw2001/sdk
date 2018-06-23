/**
 * \file hal_scl.c
 */

/*=============================================================*/
// Include files
/*=============================================================*/
#include "drv_scl_os.h"
#include "hal_scl_util.h"
/*=============================================================*/
// Macro definition
/*=============================================================*/

/*=============================================================*/
// Data type definition
/*=============================================================*/

/*=============================================================*/
// Variable definition
/*=============================================================*/



/*=============================================================*/
// Local function definition
/*=============================================================*/

void HalSclInit(void)
{

}

// Test Function
u16 HalSclRegisterRead(u32 u32Addr)
{
    return R2BYTE((u32Addr));
}

// Test Function
void HalSclRegisterWrite(u32 u32Addr, u16 u16value)
{
    W2BYTE(u32Addr, u16value);
}