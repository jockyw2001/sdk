#if !defined(MSOS_TYPE_LINUX_KERNEL)
#include "string.h"
#include <stdio.h>
#else
#include <linux/string.h>
#include <linux/slab.h>
#endif
#include "MsTypes.h"
#include "utopia_dapi.h"
#include "drvPM.h"
#include "drvPM_private.h"
#include "halPM.h"
#include "MsOS.h"
#include "utopia.h"

#include "ULog.h"
#define TAG_PM "PM"

enum
{
    PM_POOL_ID_PM0=0
} ePmPoolID;

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT

static void _MDrv_PM_PARSE_Cmd(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MdbPrint(paraCmdLine->u64ReqHdl,"MDBCMD_CMDLINE\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"u32CmdSize: %d\n", paraCmdLine->u32CmdSize);
    MdbPrint(paraCmdLine->u64ReqHdl,"pcCmdLine: %s\n", paraCmdLine->pcCmdLine);
}

static void _MDrv_PM_HELP_Cmd(MDBCMD_GETINFO_PARAMETER *paraCmdLine)
{
    MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar help---------\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"echo read [bytes] [base] [8_addr] > /proc/utopia_mdb/pm\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"echo write [bytes] [base] [8_addr] [data] > /proc/utopia_mdb/pm\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"echo [#] > /proc/utopia_mdb/pm\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"1. PM Run mode\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"2. PM Program counter\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"3. PM SRAM address\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"4. PM DRAM address\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"5. PM SPI address\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"6. PM Program bank & data bank\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"\nExample:\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"echo read 1 10 0c > /proc/utopia_mdb/pm\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"echo write 1 10 0c 9 > /proc/utopia_mdb/pm\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"echo 1 > /proc/utopia_mdb/pm\n");
    MdbPrint(paraCmdLine->u64ReqHdl,"\nUse 'echo' to get pm information\n");
}

static void _MDrv_PM_READ_Reg(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MS_U32 u32Reg = 0;
    MS_U32 u32Addr = 0;
    unsigned long args[8] = {0};

    // read, #bytes, #base, #8bit-address
    sscanf(paraCmdLine->pcCmdLine,"%*s %lx %lx %lx\n", &args[0], &args[1], &args[2]);
    u32Addr = (args[1] << 8) + (args[2] * 2);
    switch (args[0])
    {
        case 1:
            u32Reg = HAL_PM_ReadByte(u32Addr);
            MdbPrint(paraCmdLine->u64ReqHdl, "read : %lx, addr : 0x%lx, data : 0x%lx\n",
                                             args[0], u32Addr, u32Reg);
            break;
        case 2:
            u32Reg = HAL_PM_Read2Byte(u32Addr);
            MdbPrint(paraCmdLine->u64ReqHdl, "read : %lx, addr : 0x%lx, data : 0x%lx\n",
                                             args[0], u32Addr, u32Reg);
            break;
        case 4:
            u32Reg = HAL_PM_Read4Byte(u32Addr);
            MdbPrint(paraCmdLine->u64ReqHdl, "read : %lx, addr : 0x%lx, data : 0x%lx\n",
                                             args[0], u32Addr, u32Reg);
            break;
        default:
            MdbPrint(paraCmdLine->u64ReqHdl,"Input Error : %d bytes\n", args[0]);
            break;
    }
}

static void _MDrv_PM_WRITE_Reg(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MS_U32 u32Reg = 0;
    MS_U32 u32Addr = 0;
    unsigned long args[8] = {0};

    // read, #bytes, #base, #8bit-address, #data
    sscanf(paraCmdLine->pcCmdLine,"%*s %lx %lx %lx %lx\n", &args[0], &args[1], &args[2], &args[3]);
    u32Addr = (args[1] << 8) + (args[2] * 2);
    switch (args[0])
    {
        case 1:
            u32Reg = HAL_PM_WriteByte(u32Addr, (MS_U8)args[3]);
            MdbPrint(paraCmdLine->u64ReqHdl, "write : %lx, addr : 0x%lx, data : 0x%lx\n",
                                             args[0], u32Addr, (MS_U8)args[3]);
            break;
        case 2:
            u32Reg = HAL_PM_Write2Byte(u32Addr, (MS_U16)args[3]);
            MdbPrint(paraCmdLine->u64ReqHdl, "write : %lx, addr : 0x%lx, data : 0x%lx\n",
                                             args[0], u32Addr, (MS_U16)args[3]);
            break;
        case 4:
            u32Reg = HAL_PM_Write4Byte(u32Addr, (MS_U32)args[3]);
            MdbPrint(paraCmdLine->u64ReqHdl, "write : %lx, addr : 0x%lx, data : 0x%lx\n",
                                             args[0], u32Addr, (MS_U32)args[3]);
            break;
        default:
            MdbPrint(paraCmdLine->u64ReqHdl,"Input Error : %d bytes\n", args[0]);
            break;
    }

}

static void _MDrv_PM_GET_RunMode(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MS_U32 u32Reg = 0;

    // h000c [3:0] <-> Run mode
    u32Reg = (HAL_PM_Read2Byte(0x1018UL) & 0xf);

    if(u32Reg & 0x1)
        MdbPrint(paraCmdLine->u64ReqHdl,"PM Run mode : SRAM Mode\n");

    if(u32Reg & 0x2)
        MdbPrint(paraCmdLine->u64ReqHdl,"PM Run mode : SPI Mode\n");

    if(u32Reg & 0x4)
        MdbPrint(paraCmdLine->u64ReqHdl,"PM Run mode : DRAM Mode\n");

    if(u32Reg & 0x8)
        MdbPrint(paraCmdLine->u64ReqHdl,"PM Run mode : I-cache Enable\n");
}

static void _MDrv_PM_GET_PC(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MS_U32 u32ProgCnt = 0;

    // h007e [15:0] <-> program counter [15: 0]
    u32ProgCnt = HAL_PM_Read2Byte(0x10fcUL);
    // h007f [ 7:0] <-> program counter [23:16]
    u32ProgCnt |= ((HAL_PM_Read2Byte(0x10feUL) & 0xff) << 16);
    MdbPrint(paraCmdLine->u64ReqHdl,"PM Program counter : 0x%x\n", u32ProgCnt);
}

static void _MDrv_PM_GET_SramAddr(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MS_U32 u32Addr = 0;

    // h0000 [15:0] <-> SRAM start address [23:16]
    u32Addr = ((HAL_PM_Read2Byte(0x1000UL) & 0xff) << 16);
    // h0002 [15:0] <-> SRAM start address [15: 0], don't care [7:0]
    u32Addr |= (HAL_PM_Read2Byte(0x1004UL));
    MdbPrint(paraCmdLine->u64ReqHdl,"PM SRAM start address : 0x%x\n", u32Addr);

    // h0001 [15:0] <-> SRAM end address [23:16]
    u32Addr = ((HAL_PM_Read2Byte(0x1002UL) & 0xff) << 16);
    // h0003 [15:0] <-> SRAM end address [15: 0], don't care [7:0]
    u32Addr |= (HAL_PM_Read2Byte(0x1006UL));
    MdbPrint(paraCmdLine->u64ReqHdl,"PM SRAM end address : 0x%x\n", u32Addr);
}

static void _MDrv_PM_GET_DramAddr(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MS_U32 u32Addr = 0;

    // h0004 [15:0] <-> DRAM start address [23:16]
    u32Addr = ((HAL_PM_Read2Byte(0x1008UL) & 0xff) << 16);
    // h0006 [15:0] <-> DRAM start address [15: 0], don't care [7:0]
    u32Addr |= (HAL_PM_Read2Byte(0x100cUL));
    MdbPrint(paraCmdLine->u64ReqHdl,"PM DRAM start address : 0x%x\n", u32Addr);

    // h0005 [15:0] <-> DRAM end address [23:16]
    u32Addr = ((HAL_PM_Read2Byte(0x100aUL) & 0xff) << 16);
    // h0007 [15:0] <-> DRAM end address [15: 0], don't care [7:0]
    u32Addr |= (HAL_PM_Read2Byte(0x100eUL));
    MdbPrint(paraCmdLine->u64ReqHdl,"PM DRAM end address : 0x%x\n", u32Addr);
}

static void _MDrv_PM_GET_SpiAddr(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MS_U32 u32Addr = 0;

    // h0008 [15:0] <-> SPI start address [23:16]
    u32Addr = ((HAL_PM_Read2Byte(0x1010UL) & 0xff) << 16);
    // h000a [15:0] <-> SPI start address [15: 0], don't care [7:0]
    u32Addr |= (HAL_PM_Read2Byte(0x1014UL));
    MdbPrint(paraCmdLine->u64ReqHdl,"PM SPI start address : 0x%x\n", u32Addr);

    // h0009 [15:0] <-> SPI end address [23:16]
    u32Addr = ((HAL_PM_Read2Byte(0x1012UL) & 0xff) << 16);
    // h000b [15:0] <-> SPI end address [15: 0], don't care [7:0]
    u32Addr |= (HAL_PM_Read2Byte(0x1016UL));
    MdbPrint(paraCmdLine->u64ReqHdl,"PM SPI end address : 0x%x\n", u32Addr);
}

static void _MDrv_PM_GET_ProgramBank(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MS_U16 u16ProgCnt = 0;

    // h007a [15:0]
    u16ProgCnt = HAL_PM_Read2Byte(0x10f4UL);
    MdbPrint(paraCmdLine->u64ReqHdl,"PM Program bank : 0x%x\n", u16ProgCnt);
}

static void _MDrv_PM_GET_DataBank(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    MS_U16 u16ProgCnt = 0;

    // h007b [15:0]
    u16ProgCnt = HAL_PM_Read2Byte(0x10f6UL);
    MdbPrint(paraCmdLine->u64ReqHdl,"PM Date bank : 0x%x\n", u16ProgCnt);
}

static void _MDrv_PM_GET_Info(MDBCMD_CMDLINE_PARAMETER *paraCmdLine)
{
    unsigned long args[8] = {0};

    sscanf(paraCmdLine->pcCmdLine,"%lx\n", &args[0]);

    switch (args[0])
    {
        case 1:
            _MDrv_PM_GET_RunMode(paraCmdLine);
            break;
        case 2:
            _MDrv_PM_GET_PC(paraCmdLine);
            break;
        case 3:
            _MDrv_PM_GET_SramAddr(paraCmdLine);
            break;
        case 4:
            _MDrv_PM_GET_DramAddr(paraCmdLine);
            break;
        case 5:
            _MDrv_PM_GET_SpiAddr(paraCmdLine);
            break;
        case 6:
            _MDrv_PM_GET_ProgramBank(paraCmdLine);
            _MDrv_PM_GET_DataBank(paraCmdLine);
            break;
        default:
            MdbPrint(paraCmdLine->u64ReqHdl,"Unknown cmd\n");
    }
}

MS_U32 PMMdbIoctl(MS_U32 cmd, const void* const pArgs)
{
    MDBCMD_CMDLINE_PARAMETER *paraCmdLine = NULL;
    MDBCMD_GETINFO_PARAMETER *paraGetInfo = NULL;

    switch(cmd)
    {
        case MDBCMD_CMDLINE:
            paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
            // command log
            _MDrv_PM_PARSE_Cmd(paraCmdLine);
            // command
            if (strncmp(paraCmdLine->pcCmdLine, "help", 4) == 0)
            {
                paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
                _MDrv_PM_HELP_Cmd(paraGetInfo);
            }
            else if (strncmp(paraCmdLine->pcCmdLine, "read", 4) == 0)
            {
                _MDrv_PM_READ_Reg(paraCmdLine);
            }
            else if (strncmp(paraCmdLine->pcCmdLine, "write", 5) == 0)
            {
                _MDrv_PM_WRITE_Reg(paraCmdLine);
            }
            else
            {
                _MDrv_PM_GET_Info(paraCmdLine);
            }
            // finish
            paraCmdLine->result = MDBRESULT_SUCCESS_FIN;
            break;
        case MDBCMD_GETINFO:
            paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
            // command
            _MDrv_PM_HELP_Cmd(paraGetInfo);
            // finish
            paraGetInfo->result = MDBRESULT_SUCCESS_FIN;
            break;
        default:
             break;
    }
    return 0;
}
#endif

// this func will be call to init by utopia20 framework
void PMRegisterToUtopia(FUtopiaOpen ModuleType)
{
    // 1. deal with module
    void* pUtopiaModule = NULL;
    UtopiaModuleCreate(MODULE_PM, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);
    // register func for module, after register here, then ap call UtopiaOpen/UtopiaIoctl/UtopiaClose can call to these registered standard func
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)PMOpen, (FUtopiaClose)PMClose, (FUtopiaIOctl)PMIoctl);

#if defined(MSOS_TYPE_LINUX_KERNEL)
    UtopiaModuleSetupSTRFunctionPtr(pUtopiaModule,(FUtopiaSTR)PMStr);
    //UtopiaModuleSetSTRPrivate(pUtopiaModule, STRPrivateDataSize);
#endif

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    UtopiaModuleRegisterMdbNode("pm", (FUtopiaMdbIoctl)PMMdbIoctl);
#endif

    // 2. deal with resource
    void* psResource = NULL;
    // start func to add res, call once will create 2 access in resource.
    UtopiaModuleAddResourceStart(pUtopiaModule, PM_POOL_ID_PM0);
    // resource can alloc private for internal use, ex, PM_RESOURCE_PRIVATE
    UtopiaResourceCreate("pm0", sizeof(PM_RESOURCE_PRIVATE), &psResource);
    // func to reg res
    UtopiaResourceRegister(pUtopiaModule, psResource, PM_POOL_ID_PM0);

    // end function to add res
    UtopiaModuleAddResourceEnd(pUtopiaModule, PM_POOL_ID_PM0);
}

MS_U32 PMOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{
    void *pPmPriVoid = NULL;

    // instance is allocated here, also can allocate private for internal use, ex, PM_INSTANT_PRIVATE
    UtopiaInstanceCreate(sizeof(PM_INSTANT_PRIVATE), ppInstance);
    // setup func in private and assign the calling func in func ptr in instance private
    UtopiaInstanceGetPrivate(*ppInstance, &pPmPriVoid);

    return UTOPIA_STATUS_SUCCESS;
}

// FIXME: why static?
MS_U32 PMIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
    return UTOPIA_STATUS_SUCCESS; // FIXME: error code
}

MS_U32 PMClose(void* pInstance)
{
    UtopiaInstanceDelete(pInstance);

    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 PMStr(MS_U32 u32PowerState, void* pModule)
{
    MS_U32 u32Return = UTOPIA_STATUS_FAIL;

    //UtopiaModuleGetSTRPrivate(pModule, (void**));

    if (u32PowerState == E_POWER_SUSPEND)
    {
        /* Please Implement Module Suspend Flow Here. */
        u32Return = UTOPIA_STATUS_SUCCESS;//SUSPEND_OK;
    }
    else if (u32PowerState == E_POWER_RESUME)
    {
        /* Please Implement Module Resume Flow Here. */
        u32Return = UTOPIA_STATUS_SUCCESS;//RESUME_OK;
    }
    else
    {
        u32Return = UTOPIA_STATUS_FAIL;
    }

    return u32Return;// for success
}
