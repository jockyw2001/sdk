#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>


#include "MsTypes.h"
#include "utopia.h"
#include "apiSWI2C.h"
#include "apiSWI2C_priv.h"
#include "SWI2C_adp.h"
#include "utopia_adp.h"
/*
UADP_SPT_0NXT_DEF(SWI2C_WRITEBYTES_PARAM);
UADP_SPT_0NXT_DEF(SWI2C_READBYTES_PARAM);
*/
MS_U32 SWI2C_adp_Init(FUtopiaIOctl* pIoctl)
{
//member of struct
//set table
/*
    UADP_SPT_0NXT(SWI2C_WRITEBYTES_PARAM);
    UADP_SPT_0NXT(SWI2C_READBYTES_PARAM);

	  *pIoctl= (FUtopiaIOctl)SWI2C_adp_Ioctl;*/
	  return 0;
}

MS_U32 SWI2C_adp_Ioctl(void* pInstanceTmp, MS_U32 u32Cmd, void* const pArgs)
{
    MS_U32 u32Ret=0;/*
    char buffer_arg[2048];
    switch(u32Cmd)
    {
        case MApi_CMD_SWI2C_WriteBytes:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SWI2C_WRITEBYTES_PARAM, spt_SWI2C_WRITEBYTES_PARAM ,buffer_arg,sizeof(buffer_arg));
            break;
        case MApi_CMD_SWI2C_ReadBytes:
            u32Ret=UADPBypassIoctl(pInstanceTmp,u32Cmd,pArgs,spt_SWI2C_READBYTES_PARAM, spt_SWI2C_READBYTES_PARAM ,buffer_arg,sizeof(buffer_arg));
		        break;
		    default:
            break;
    } */
    return u32Ret;
}
