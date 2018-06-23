#if !defined(MSOS_TYPE_LINUX_KERNEL)
#include "string.h"
#include <stdio.h>
#define ascii2hex(str) strtol(((str != NULL) ? str : ""), NULL, 16)

#else
#include <linux/string.h>
#include <linux/slab.h>
#define ascii2hex(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 16)

#endif
#include "MsTypes.h"
#include "utopia_dapi.h"
#include "drvGPIO.h"
#include "drvGPIO_private.h"
#include "MsOS.h"
#include "utopia.h"


enum
{
    GPIO_POOL_ID_GPIO0=0
} eGPIOPoolID;


#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
#define CmdCnt 3
static int array[CmdCnt];
extern GPIO_DbgLv _geDbfLevel;
typedef enum{
    MDBCMD_GET_GPIO = 0x0,
    MDBCMD_SET_GPIO,
    MDBCMD_GET_GPIO_OEN,
    MDBCMD_SET_GPIO_DEBUG_LEVEL,
}UTOPIA_MDBCMD_GPIO;

void MDrv_GPIO_Mdb_Parse(MS_U64* u64ReqHdl)
{
    int temp = 0;

    mdrv_gpio_init();
    array[1]++;
    switch(array[0])
    {
        case MDBCMD_GET_GPIO:
            MdbPrint(u64ReqHdl, "---------MStar GPIO INPUT STATUS----------\n");
            temp = MDrv_GPIO_get_level(array[1]);
            MdbPrint(u64ReqHdl, "GPIO PAD: %d\n", array[1]);
            MdbPrint(u64ReqHdl, "GET STATUS: %d\n", temp);
            break;
        case MDBCMD_SET_GPIO:
            MdbPrint(u64ReqHdl, "--------- MStar GPIO SETTING ---------\n");
            if(array[2] == 0)
            {
                MDrv_GPIO_set_low(array[1]);
                MdbPrint(u64ReqHdl, "GPIO PAD: %d\n", array[1]);
                MdbPrint(u64ReqHdl, "SET GPIO: 0\n");
            }
            else if (array[2] == 1)
            {
                MDrv_GPIO_set_high(array[1]);
                MdbPrint(u64ReqHdl, "GPIO PAD: %d\n", array[1]);
                MdbPrint(u64ReqHdl, "SET GPIO: 1\n");
            }
            else
            {
                MdbPrint(u64ReqHdl, "Error command: %d, HIGH:1 LOW:0\n", array[2]);
            }
            break;
        case MDBCMD_GET_GPIO_OEN:
            MdbPrint(u64ReqHdl, "---------MStar GPIO GET OEN STATUS----------\n");
            temp = MDrv_GPIO_get_inout(array[1]);
            MdbPrint(u64ReqHdl, "GPIO PAD: %d\n", array[1]);
            MdbPrint(u64ReqHdl, "GET OEN STATUS: ", temp);
            if(temp == 0)
                MdbPrint(u64ReqHdl, "OUTPUT\n");
            else
                MdbPrint(u64ReqHdl, "INPUT\n");
            break;
        case MDBCMD_SET_GPIO_DEBUG_LEVEL:
            MdbPrint(u64ReqHdl, "---------MStar GPIO SET DEBUG LEVEL----------\n");
            array[1]--;
            switch(array[1])
            {
                case E_GPIO_DBGLV_NONE:
                    _geDbfLevel = E_GPIO_DBGLV_NONE;
                    MdbPrint(u64ReqHdl, "GPIO SET DEBUG LEVEL E_GPIO_DBGLV_NONE\n");
                    break;
                case E_GPIO_DBGLV_ERR_ONLY:
                    _geDbfLevel = E_GPIO_DBGLV_ERR_ONLY;
                    MdbPrint(u64ReqHdl, "GPIO SET DEBUG LEVEL E_GPIO_DBGLV_ERR_ONLY\n");
                    break;
                case E_GPIO_DBGLV_REG_DUMP:
                    _geDbfLevel = E_GPIO_DBGLV_REG_DUMP;
                    MdbPrint(u64ReqHdl, "GPIO SET DEBUG LEVEL E_GPIO_DBGLV_REG_DUMP\n");
                    break;
                case E_GPIO_DBGLV_INFO:
                    _geDbfLevel = E_GPIO_DBGLV_INFO;
                    MdbPrint(u64ReqHdl, "GPIO SET DEBUG LEVEL E_GPIO_DBGLV_INFO\n");
                    break;
                case E_GPIO_DBGLV_ALL:
                    _geDbfLevel = E_GPIO_DBGLV_ALL;
                    MdbPrint(u64ReqHdl, "GPIO SET DEBUG LEVEL E_GPIO_DBGLV_ALL\n");
                    break;
                default:
                    MdbPrint(u64ReqHdl, "UNKNOWN DEBUG LEVEL!!\n");
                    break;
            }
            break;
        default:
            MdbPrint(u64ReqHdl, "Check CMD.\n");
            MdbPrint(u64ReqHdl, "GET_GPIO: 0\n");
            MdbPrint(u64ReqHdl, "SET_GPIO: 1\n");
            MdbPrint(u64ReqHdl, "GET_GPIO_OEN: 2\n");
          break;
    }
}

MS_U32 GPIOMdbIoctl(MS_U32 cmd, const void* const pArgs)
{
    MDBCMD_CMDLINE_PARAMETER *paraCmdLine;
    MDBCMD_GETINFO_PARAMETER *paraGetInfo;
    char tmpInput[40] = "";
    int i = 0;
    const char *del = " ";
    char *pch;
    char *cur;

    switch(cmd)
    {
        case MDBCMD_CMDLINE:
            paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
            //MdbPrint(paraCmdLine->u64ReqHdl,"LINE:%d, MDBCMD_CMDLINE\n", __LINE__);
            //MdbPrint(paraCmdLine->u64ReqHdl,"u32CmdSize: %d\n", paraCmdLine->u32CmdSize);
            //MdbPrint(paraCmdLine->u64ReqHdl,"pcCmdLine: %s\n", paraCmdLine->pcCmdLine);

            //TC MDB function----------------------------
            strncpy(tmpInput, paraCmdLine->pcCmdLine, sizeof(tmpInput)-1);
            //printf("tmpInput: %s\n", tmpInput);

            if(strncmp(tmpInput, "help", 4) == 0)
            {
                MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar GPIO Help---------\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"1.  Get GPIO status(high/low)\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      #echo 0 (GPIO NUM HEX) > /proc/utopia_mdb/gpio\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      [e.g.]#echo 0 1 > /proc/utopia_mdb/gpio\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"2.  Set GPIO (HIGH/LOW)\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      echo 1 (GPIO NUM HEX) (HIGH/LOW)> /proc/utopia_mdb/gpio\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      [e.g.]echo 1 1 1> /proc/utopia_mdb/gpio\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"3.  Get GPIO OEN\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      echo 2 (GPIO NUM) > proc/utopia_mdb/gpio\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      [e.g.]echo 2 1 > /proc/utopia_mdb/gpio\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"4.  Set GPIO Debug Level\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      echo 3 (GPIO DEBUG LEVEL) > proc/utopia_mdb/gpio\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      [e.g.]echo 3 4 > /proc/utopia_mdb/gpio\n");
            }
            else
            {
                cur = tmpInput;
                for(i = 0; i < CmdCnt; i++)
                  array[i] = 0;

                i = 0;
                while (pch = strsep(&cur, del))
                {
                    //printf("%s", pch);
                    array[i]=ascii2hex(pch);
                    i++;
                }
                //printf("array [0] : %x\n",array[0]);
                //printf("array [1] : %x\n",array[1]);
                //printf("array [2] : %x\n",array[2]);
                //printf("array [3] : %x\n",array[3]);
                //printf("array [4] : %x\n",array[4]);
                //printf("array [5] : %x\n",array[5]);
                MDrv_GPIO_Mdb_Parse(paraCmdLine->u64ReqHdl);
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

// this func will be call to init by utopia20 framework
void GPIORegisterToUtopia(FUtopiaOpen ModuleType)
{
     void* pUtopiaModule = NULL;
    UtopiaModuleCreate(MODULE_GPIO, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);
    // register func for module, after register here, then ap call UtopiaOpen/UtopiaIoctl/UtopiaClose can call to these registered standard func
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)GPIOOpen, (FUtopiaClose)GPIOClose, (FUtopiaIOctl)GPIOIoctl);
    // 2. Resource register
    void* psResource = NULL;
    // start func to add resources of a certain Resource_Pool
    UtopiaModuleAddResourceStart(pUtopiaModule, GPIO_POOL_ID_GPIO0);
    // create a resouce and regiter it to a certain Resource_Pool, resource can alloc private for internal use
    UtopiaResourceCreate("gpio0", sizeof(GPIO_RESOURCE_PRIVATE), &psResource);
    UtopiaResourceRegister(pUtopiaModule, psResource, GPIO_POOL_ID_GPIO0);

    //UtopiaResourceCreate("cmdq1", sizeof(CMDQ_RESOURCE_PRIVATE), &psResource);
    //UtopiaResourceRegister(pUtopiaModule, psResource, CMDQ_POOL_ID_CMDQ0);
    // end func to add resources of a certain Resource_Pool(this will set the ResourceSemaphore of this ResourcePool)
    UtopiaModuleAddResourceEnd(pUtopiaModule, GPIO_POOL_ID_GPIO0);

    #ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    UtopiaModuleRegisterMdbNode("gpio", (FUtopiaMdbIoctl)GPIOMdbIoctl);
    #endif
}

MS_U32 GPIOOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{
    GPIO_INSTANT_PRIVATE *pGpioPri = NULL;
    void *pGpioPriVoid = NULL;

    //UTOPIA_TRACE(MS_UTOPIA_DB_LEVEL_TRACE,printf("enter %s %d\n",__FUNCTION__,__LINE__));
    // instance is allocated here, also can allocate private for internal use, ex, BDMA_INSTANT_PRIVATE
    UtopiaInstanceCreate(sizeof(GPIO_INSTANT_PRIVATE), ppInstance);
    // setup func in private and assign the calling func in func ptr in instance private
    UtopiaInstanceGetPrivate(*ppInstance, &pGpioPriVoid);
    pGpioPri= (GPIO_INSTANT_PRIVATE*)pGpioPriVoid;
    pGpioPri->fpGpioInit= (IOCTL_GPIO_INIT)mdrv_gpio_init_U2K;
    pGpioPri->fpGpioGetSts= (IOCTL_GPIO_GET_STS)MDrv_GPIO_GetStatus_U2K;
    pGpioPri->fpGpioSetH= (IOCTL_GPIO_SET_H)MDrv_GPIO_set_high_U2K;
    pGpioPri->fpGpioSetL= (IOCTL_GPIO_SET_L)MDrv_GPIO_set_low_U2K;
    pGpioPri->fpGpioSetInput= (IOCTL_GPIO_SET_INPUT)MDrv_GPIO_set_input_U2K;
    pGpioPri->fpGpioGetInout= (IOCTL_GPIO_GET_INOUT)MDrv_GPIO_get_inout_U2K;
    pGpioPri->fpGpioGetLevel= (IOCTL_GPIO_GET_LEVEL)MDrv_GPIO_get_level_U2K;
    pGpioPri->fpGpioAttachInterrupt= (IOCTL_GPIO_ATTACH_INTERRUPT)mdrv_gpio_attach_interrupt_U2K;
    pGpioPri->fpGpioDetachInterrupt= (IOCTL_GPIO_DETACH_INTERRUPT)mdrv_gpio_detach_interrupt_U2K;
    pGpioPri->fpGpioDisableInterrupt = (IOCTL_GPIO_DISABLE_INTERRUPT)mdrv_gpio_disable_interrupt_U2K;
    pGpioPri->fpGpioEnableInterrupt = (IOCTL_GPIO_ENALBE_INTERRUPT)mdrv_gpio_enable_interrupt_U2K;
    pGpioPri->fpGpioDisableInterruptAll= (IOCTL_GPIO_DISABLE_INTERRUPT_ALL)mdrv_gpio_disable_interrupt_all_U2K;
    pGpioPri->fpGpioEnableInterruptAll = (IOCTL_GPIO_ENABLE_INTERRUPT_ALL)mdrv_gpio_enable_interrupt_all_U2K;
    pGpioPri->fpGpioInterruptAction = (IOCTL_GPIO_INTERRUPT_ACTION) mdrv_gpio_interrupt_action_U2K;
    return UTOPIA_STATUS_SUCCESS;
}

// FIXME: why static?
MS_U32 GPIOIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
    void* pModule = NULL;
    GPIO_INSTANT_PRIVATE* psGpioInstPri = NULL;
    MS_U32 u32Ret = TRUE;
    MS_U32 u32InOutVal;
    MS_U32 u32LevelVal;

    UtopiaInstanceGetModule(pInstance, &pModule);
    UtopiaInstanceGetPrivate(pInstance, (void**)&psGpioInstPri);
    GPIO_PRIVATE_PARAM *param = NULL;

    param= (GPIO_PRIVATE_PARAM *) pArgs;


    switch(u32Cmd)
    {
        case MDrv_CMD_GPIO_Init:
            psGpioInstPri->fpGpioInit();
            break;
        case MDrv_CMD_GPIO_GetStauts:
            psGpioInstPri->fpGpioGetSts(param->privatGpioSts.pSts);
            break;
        case MDrv_CMD_GPIO_Set_High:
            psGpioInstPri->fpGpioSetH(param->privateGpioSetHigh.gpio_num);
            break;
        case MDrv_CMD_GPIO_Set_Low:
            psGpioInstPri->fpGpioSetL(param->privateGpioSetLow.gpio_num);
            break;
        case  MDrv_CMD_GPIO_Set_Input:
            psGpioInstPri->fpGpioSetInput(param->privateGpioInput.gpio_num);
            break;
        case MDrv_CMD_GPIO_Get_Inout:
            u32InOutVal = psGpioInstPri->fpGpioGetInout(param->privateGpioGetInout.gpio_num);
            param->privateGpioGetInout.u32InOut=u32InOutVal;
            break;
        case MDrv_CMD_GPIO_Get_Level:
             u32LevelVal = psGpioInstPri->fpGpioGetLevel(param->privateGpioGetLevel.gpio_num);
            param->privateGpioGetLevel.u32Level=u32LevelVal;
            break;
        case MDrv_CMD_GPIO_Attach_Interrupt:
            u32Ret = psGpioInstPri->fpGpioAttachInterrupt(param->privateGpioAttachInterrupt.gpio_num,param->privateGpioAttachInterrupt.gpio_edge_type,param->privateGpioAttachInterrupt.GPIO_Callback);
            break;
        case MDrv_CMD_GPIO_Detach_Interrupt:
            u32Ret = psGpioInstPri->fpGpioDetachInterrupt(param->privateGpioDetachInterrupt.gpio_num);
            break;
        case  MDrv_CMD_GPIO_En_Interrupt:
            u32Ret = psGpioInstPri->fpGpioEnableInterrupt(param->privateGpioEnableInterrupt.gpio_num);
        case MDrv_CMD_GPIO_Dis_Interrupt:
            u32Ret = psGpioInstPri->fpGpioDisableInterrupt(param->privateGpioDisableInterrupt.gpio_num);
            break;
        case MDrv_CMD_GPIO_Dis_Interrupt_All:
            psGpioInstPri->fpGpioDisableInterruptAll();
            break;
        case MDrv_CMD_GPIO_En_Interrupt_All:
            psGpioInstPri->fpGpioEnableInterruptAll();
            break;
        case MDrv_CMD_GPIO_Interrupt_Action:
            psGpioInstPri->fpGpioInterruptAction();
            break;
        default:
            break;
    }
    return  (u32Ret==TRUE?UTOPIA_STATUS_SUCCESS:UTOPIA_STATUS_FAIL); // FIXME: error code
}

MS_U32 GPIOClose(void* pInstance)
{
    UtopiaInstanceDelete(pInstance);

    return UTOPIA_STATUS_SUCCESS;
}


