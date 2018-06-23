#if !defined(MSOS_TYPE_LINUX_KERNEL)
#include "string.h"
#include <stdio.h>
#else
#include <linux/string.h>
#include <linux/slab.h>
#endif

#include "MsTypes.h"
#include "utopia_dapi.h"
#include "apiSWI2C.h"
#include "apiSWI2C_priv.h"
#include "MsOS.h"
#include "utopia.h"

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
#include "drvGPIO.h"
#endif
#define TAG_SWI2C "SWI2C"

enum
{
    SWI2C_POOL_ID_SWI2C0 = 0
} eSWI2C_PoolID;

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
#define CmdCnt 6
static int array[CmdCnt];
extern int testBusNum;

typedef enum{
    MDBCMD_SWI2C_BUS_INFO = 0x0,
    MDBCMD_SWI2C_SET_BUS_CFG,
    MDBCMD_SWI2C_WRITEBYTE,
    MDBCMD_SWI2C_READBYTE
}UTOPIA_MDBCMD_SWI2C;

static int atoi_mstar(char *s)
{
    int sum = 0;
    int i = 0;
    int b = 0;

    for(i = 0; i < CmdCnt; i++)
        array[i] = 0;
    //printf("S's size: %d\n", sizeof(s));
    for(i = 0;s[i] != '\0';i++)
    {
        if((s[i]=='A') || s[i]=='a')
        {
          sum = sum*16+10;
          array[b] = array[b]*16+10;
        }
        else if((s[i]=='B') || (s[i]=='b'))
        {
          sum = sum*16+11;
          array[b] = array[b]*16+11;
        }
        else if((s[i]=='C') || (s[i]=='c'))
        {
          sum = sum*16+12;
          array[b] = array[b]*16+12;
        }
        else if((s[i]=='D') || s[i]=='d')
        {
          sum = sum*16+13;
          array[b] = array[b]*16+13;
        }
        else if((s[i]=='E') || s[i]=='e')
        {
          sum = sum*16+14;
          array[b] = array[b]*16+14;
        }
        else if((s[i]=='F') || s[i]=='f')
        {
          sum = sum*16+15;
          array[b] = array[b]*16+15;
        }
        else if (s[i]==' ')
        {
          b++;
          if (b > CmdCnt-1)
            break;
        }
        else
        {
          sum = sum*16+s[i]-'0';
          array[b] = array[b]*16+s[i]-'0';
        }
    }
    //printf("b=%d\n", b);
    //printf("array [0] : %x\n",array[0]);
    //printf("array [1] : %x\n",array[1]);
    //printf("array [2] : %x\n",array[2]);
    //printf("array [3] : %x\n",array[3]);
    //printf("array [4] : %x\n",array[4]);
    //printf("array [5] : %x\n",array[5]);

    return sum;
};

SWI2C_BusCfg BusCfg[1];
void MDrv_Swi2c_Mdb_Parse(MS_U64* u64ReqHdl)
{
    MS_U16 u16BusNumSlaveID = array[1];

    //For W/R Bytes
    MS_U8 DataCnt_w = array[2];
    MS_U8 DataCnt_r = array[3];

    int i = 0;

    MS_U8   byTargetRegAddress[5] =
    {
        array[4], array[5], array[6], array[7], array[8]
    };
    MS_U8   byTargetData[5] =
    {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };

    switch(array[0])
    {
        case MDBCMD_SWI2C_BUS_INFO:
            MdbPrint(u64ReqHdl, "---------MStar SWI2C INFO---------\n");
            MdbPrint(u64ReqHdl, "SCL: %d\n", BusCfg[0].padSCL);
            MdbPrint(u64ReqHdl, "SDA: %d\n", BusCfg[0].padSDA);
            MdbPrint(u64ReqHdl, "Clock: %d\n", BusCfg[0].defDelay);
            if(BusCfg[0].padSCL == 0 && BusCfg[0].padSDA == 0 && BusCfg[0].defDelay == 0)
                MdbPrint(u64ReqHdl, "Error! CFG is Null, Please Set Bus Config!\n");
            break;

        case MDBCMD_SWI2C_SET_BUS_CFG:
            MdbPrint(u64ReqHdl, "---------MStar SWI2C SET BUS CONFIG---------\n");
            //MdbPrint(u64ReqHdl, "SCL: %d\n", array[1]);
            //MdbPrint(u64ReqHdl, "SDA: %d\n", array[2]);
            //MdbPrint(u64ReqHdl, "DELAY: %d\n", array[3]);
            BusCfg[0].padSCL = array[1];
            BusCfg[0].padSDA = array[2];
            BusCfg[0].defDelay = array[3];
            MdbPrint(u64ReqHdl,"SCL: %d\n", BusCfg[0].padSCL);
            MdbPrint(u64ReqHdl,"SDA: %d\n", BusCfg[0].padSDA);
            MdbPrint(u64ReqHdl,"DELAY: %d\n", BusCfg[0].defDelay);
            MApi_SWI2C_Init(&BusCfg[0], 1);
            break;

        case MDBCMD_SWI2C_WRITEBYTE:
            MdbPrint(u64ReqHdl, "---------MStar SWI2C WRITEBYTES---------\n");

            if(DataCnt_w == 0 || DataCnt_w > 5 )
            {
                MdbPrint(u64ReqHdl, "Check Write DataCnt.\n Check if the count is set 0 or over 5 Bytes.\n");
                return;
            }

            mdrv_gpio_init();
            if((MApi_SWI2C_WriteBytes(u16BusNumSlaveID, DataCnt_r, byTargetRegAddress, 0, byTargetData)) == true)
            {
                MdbPrint(u64ReqHdl, "Write %d Bytes (", DataCnt_w);
                for(i=0; i<DataCnt_w; i++)
                    MdbPrint(u64ReqHdl, " 0x%x ", byTargetRegAddress[i]);
                MdbPrint(u64ReqHdl, ") Success!\n");
            }
            else
            {
                MdbPrint(u64ReqHdl, "Failed!\n");
            }

            break;

        case MDBCMD_SWI2C_READBYTE:

            MdbPrint(u64ReqHdl, "---------MStar SWI2C READBYTES---------\n");

            if(DataCnt_w == 0 || DataCnt_w > 5 )
            {
                MdbPrint(u64ReqHdl, "Check Write DataCnt.\n Check if the count is set 0 or over 5 Bytes.\n");
                return;
            }

            if(DataCnt_r == 0 || DataCnt_r > 5 )
            {
                MdbPrint(u64ReqHdl, "Check Read DataCnt.\n Check if the count is set 0 or over 5 Bytes.\n");
                return;
            }

            mdrv_gpio_init();
            if((MApi_SWI2C_ReadBytes(u16BusNumSlaveID, DataCnt_w, byTargetRegAddress, DataCnt_r, byTargetData)) == true)
            {
                MdbPrint(u64ReqHdl, "Read %d Bytes (", DataCnt_r);
                for(i=0; i<DataCnt_r; i++)
                    MdbPrint(u64ReqHdl, " 0x%x ", byTargetData[i]);
                MdbPrint(u64ReqHdl, ") Success!\n");
            }
            else
            {
                MdbPrint(u64ReqHdl, "Failed!\n");
            }

            break;
        default:
            MdbPrint(u64ReqHdl, "CMD Error\n");
          break;
    }
}

MS_U32 SWI2CMdbIoctl(MS_U32 cmd, const void* const pArgs)
{
    MDBCMD_CMDLINE_PARAMETER *paraCmdLine;
    MDBCMD_GETINFO_PARAMETER *paraGetInfo;
    char tmpInput[40] = "";
    int i = 0;

    switch(cmd)
    {
        case MDBCMD_CMDLINE:
            paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
            //MdbPrint(paraCmdLine->u64ReqHdl,"LINE:%d, MDBCMD_CMDLINE\n", __LINE__);
            //MdbPrint(paraCmdLine->u64ReqHdl,"u32CmdSize: %d\n", paraCmdLine->u32CmdSize);
            //MdbPrint(paraCmdLine->u64ReqHdl,"pcCmdLine: %s\n", paraCmdLine->pcCmdLine);

            //TC MDB function----------------------------
            strncpy(tmpInput, paraCmdLine->pcCmdLine, sizeof(tmpInput));
            //printf("tmpInput: %s\n", tmpInput);

            if(strncmp(tmpInput, "help", 4) == 0)
            {
                MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar SWIIC Help---------\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"1.  Set SWIIC Bus CONFIG\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      echo 1 (SCL_HEX) (SDA_HEX) (Delay_HEX) > /proc/utopia_mdb/swi2c\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      [e.g.]echo 1 38 37 64 > /proc/utopia_mdb/swi2c\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"2.  Display SWIIC Config\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      [e.g.]echo 0 > /proc/utopia_mdb/swi2c\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"3.  Write SWIIC device\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      echo 2 ADDRESS DataCnt_W DataCnt_R V1 V2 V3 V4 V5 > /proc/utopia_mdb/swi2c\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      [e.g.]echo 2 80 1 0 7f > /proc/utopia_mdb/swi2c\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"4.  Read SWIIC device\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      echo 3 ADDRESS DataCnt_W DataCnt_R V1 V2 V3 V4 V5 > /proc/utopia_mdb/swi2c\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      [e.g]echo 3 80 2 2 10 c0 > /proc/utopia_mdb/swi2c\n");
            }
            else
            {
            atoi_mstar(tmpInput);
                MDrv_Swi2c_Mdb_Parse(paraCmdLine->u64ReqHdl);
            }
            paraCmdLine->result = MDBRESULT_SUCCESS_FIN;
            break;
        case MDBCMD_GETINFO:
            paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
            //MdbPrint(paraGetInfo->u64ReqHdl,"LINE:%d, MDBCMD_GETINFO\n", __LINE__);
            paraGetInfo->result = MDBRESULT_SUCCESS_FIN;

            break;
        default:
            paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
            MdbPrint(paraGetInfo->u64ReqHdl,"unknown cmd\n", __LINE__);
            break;
    }
    return 0;
}
#endif

//--------------------------------------------------------------------------------------------------
// Utopia2.0 will call this function to register module
//--------------------------------------------------------------------------------------------------
void SWI2CRegisterToUtopia(void)
{/*
    void* pUtopiaModule = NULL;
	  UtopiaModuleCreate(MODULE_SWI2C, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);

    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)SWI2COpen, (FUtopiaClose)SWI2CClose, (FUtopiaIOctl)SWI2CIoctl);

    // 2. deal with resource
    void* psResource = NULL;
    // start func to add res, call once will create 2 access in resource. Also can declare BDMA_POOL_ID_BDMA1 for another channel depend on driver owner.
	  UtopiaModuleAddResourceStart(pUtopiaModule, SWI2C_POOL_ID_SWI2C0);
    // resource can alloc private for internal use, ex, BDMA_RESOURCE_PRIVATE
    UtopiaResourceCreate("swi2c", sizeof(SWI2C_RESOURCE_PRIVATE), &psResource);
    // func to reg res
    UtopiaResourceRegister(pUtopiaModule, psResource, SWI2C_POOL_ID_SWI2C0);
    // end function to add res
	  UtopiaModuleAddResourceEnd(pUtopiaModule, SWI2C_POOL_ID_SWI2C0);*/

	  #ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    UtopiaModuleRegisterMdbNode("swi2c", (FUtopiaMdbIoctl)SWI2CMdbIoctl);
    #endif
}


//--------------------------------------------------------------------------------------------------
// Utopia2.0 will call this function to get a instance to use SEM
// @ \b in: 32ModuleVersion => this is for checking if API version is the same
//--------------------------------------------------------------------------------------------------
MS_U32 SWI2COpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{/*
    SWI2C_INSTANT_PRIVATE *pSwi2cPri = NULL;

    UtopiaInstanceCreate(sizeof(SWI2C_INSTANT_PRIVATE), ppInstance);
    UtopiaInstanceGetPrivate(*ppInstance, (void**)&pSwi2cPri);

    pSwi2cPri->fpSWI2CWriteBytes         = (IOCTL_SWI2C_WRITEBYTES)_MApi_SWI2C_WriteBytes;
    pSwi2cPri->fpSWI2CReadBytes      = (IOCTL_SWI2C_READBYTES)_MApi_SWI2C_ReadBytes;
*/
    return 0;
}

MS_U32 SWI2CIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
  /*
	  printf("Ioctl\n");
	  void* pModule = NULL;
	  UtopiaInstanceGetModule(pInstance, &pModule);
	  void* pResource = NULL;

    PSWI2C_WRITEBYTES_PARAM pWriteBytesParam = NULL;
    PSWI2C_READBYTES_PARAM pReadBytesParam = NULL;

    SWI2C_INSTANT_PRIVATE* psSWI2CInstPri = NULL;
	  UtopiaInstanceGetPrivate(pInstance, (void**)&psSWI2CInstPri);

    MS_U32 u32Ret;

    switch(u32Cmd)
    {

		    case MApi_CMD_SWI2C_WriteBytes:
			    printf("MDrv_CMD_SWI2C_WriteBytes_Init \n ");
          pWriteBytesParam = (PSWI2C_WRITEBYTES_PARAM)pArgs;
          u32Ret = psSWI2CInstPri->fpSWI2CWriteBytes(pWriteBytesParam->u16BusNumSlaveID, pWriteBytesParam->AddrCnt, pWriteBytesParam->pu8addr, pWriteBytesParam->u16size, pWriteBytesParam->pBuf);
          return u32Ret;
		    case MApi_CMD_SWI2C_ReadBytes:
			    pReadBytesParam = (PSWI2C_READBYTES_PARAM)pArgs;
			    u32Ret = psSWI2CInstPri->fpSWI2CReadBytes(pReadBytesParam->u16BusNumSlaveID, pReadBytesParam->ucSubAdr, pReadBytesParam->paddr, pReadBytesParam->ucBufLen, pReadBytesParam->pBuf);
          return u32Ret;

        default:
          break;
    }*/
    return 1; // FIXME: error code
}

MS_U32 SWI2CClose(void* pInstance)
{/*
	  UtopiaInstanceDelete(pInstance);
*/
	  return UTOPIA_STATUS_SUCCESS;
}
