#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#else
#include <stdio.h>
#include <string.h>
#endif

#include "MsTypes.h"
//#include "string.h"
//#include <stdio.h>
#include "utopia_dapi.h"
#include "utopia.h"
#include "drvPWM.h"
#include "drvPWM_private.h"
#include "MsOS.h"

#include "ULog.h"

#define TAG_PWM "PWM"

enum
{
    PWM_POOL_ID_PWM0=0
} ePWMPoolID;

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
static int atoi_mstar(char *s)
{
    int sum = 0;
    int i = 0;
    for(i = 0;s[i] != '\0';i++)
    {
        if((s[i]=='A') || s[i]=='a')
        {
          sum = sum*16+10;
        }
        else if((s[i]=='B') || (s[i]=='b'))
        {
          sum = sum*16+11;
        }
        else if((s[i]=='C') || (s[i]=='c'))
        {
          sum = sum*16+12;
        }
        else if((s[i]=='D') || s[i]=='d')
        {
          sum = sum*16+13;
        }
        else if((s[i]=='E') || s[i]=='e')
        {
          sum = sum*16+14;
        }
        else if((s[i]=='F') || s[i]=='f')
        {
          sum = sum*16+15;
        }
        else
        {
          sum = sum*16+s[i]-'0';
        }
    }
    return sum;
}

MS_U32 PWMMdbIoctl(MS_U32 cmd, const void* const pArgs)
{
    MDBCMD_CMDLINE_PARAMETER *paraCmdLine;
    MDBCMD_GETINFO_PARAMETER *paraGetInfo;
    static PWM_ChNum indexPWM = E_PWM_CH0;
    MS_U32 periodPWM = 0;
    MS_U32 dutyPWM = 0;
    char tmpInput[40] = "";
    const char *del = " ";
    const int indxMax = 5;
    char *pch;
    char *cur;
    char indxStr[6] = "";
    char prodStr[6] = "";
    char dutyStr[6] = "";
    int prodInt;
    int dutyInt;
    int i = 0;

    MDrv_PWM_Init(0);

    switch(cmd)
    {
        case MDBCMD_CMDLINE:
            paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
            //MdbPrint(paraCmdLine->u64ReqHdl,"LINE:%d, MDBCMD_CMDLINE\n", __LINE__);
            //MdbPrint(paraCmdLine->u64ReqHdl,"u32CmdSize: %d\n", paraCmdLine->u32CmdSize);
            //MdbPrint(paraCmdLine->u64ReqHdl,"pcCmdLine: %s\n", paraCmdLine->pcCmdLine);

            strncpy(tmpInput, paraCmdLine->pcCmdLine, sizeof(tmpInput));
            if(strncmp(tmpInput, "help", 4) == 0)
            {
                MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar PWM Help---------\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"1.  Information\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      cat /proc/utopia_mdb/pwm\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"2.  Set PWM\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      echo index period duty > /proc/utopia_mdb/pwm\n");
                MdbPrint(paraCmdLine->u64ReqHdl,"      [e.g.]echo 1 aa bb > /proc/utopia_mdb/pwm\n");
            }
            else
            {
                cur = tmpInput;

                i = 0;
                while (pch = strsep(&cur, del))
                {
                    //printf("%s\n", pch);
                    if (i == 0)
                        strncpy(indxStr, pch, sizeof(indxStr));
                    else if (i == 1)
                        strncpy(prodStr, pch, sizeof(prodStr));
                    else if (i == 2)
                        strncpy(dutyStr, pch, sizeof(dutyStr));
                    i++;
                }

                //MdbPrint(paraCmdLine->u64ReqHdl,"indxStr=%s, prodStr=%s, dutyStr=%s\n", indxStr, prodStr, dutyStr);

                if (paraCmdLine->u32CmdSize > 1)
                {
                    /* determine index of PWM */
                    if (strcmp(indxStr,"0") == 0)
                        indexPWM = E_PWM_CH0;
                    else if (strcmp(indxStr,"1") == 0)
                        indexPWM = E_PWM_CH1;
                    else if (strcmp(indxStr,"2") == 0)
                        indexPWM = E_PWM_CH2;
                    else if (strcmp(indxStr,"3") == 0)
                        indexPWM = E_PWM_CH3;
                    else if (strcmp(indxStr,"4") == 0)
                        indexPWM = E_PWM_CH4;
                    else
                    {
                        MdbPrint(paraCmdLine->u64ReqHdl,"Invalid index number (%s)\n", indxStr);
                        break;
                    }

                    if (prodStr[0] != '\0')
                    {
                        prodInt = atoi_mstar(prodStr);
                        //MdbPrint(paraCmdLine->u64ReqHdl,"prodStr=%s, prodInt=%d\n", prodStr, prodInt);

                        if ((prodInt < 65536) && (prodInt >= 0))
                        {
                            MdbPrint(paraCmdLine->u64ReqHdl,"Set index: %d with period: %d\n", indexPWM, prodInt);
                            MDrv_PWM_Period(indexPWM, prodInt);
                        }
                        else
                        {
                            MdbPrint(paraCmdLine->u64ReqHdl,"Invalid period (%d)\n", prodInt);
                        }
                    }

                    if (dutyStr[0] != '\0')
                    {
                        dutyInt = atoi_mstar(dutyStr);
                        //MdbPrint(paraCmdLine->u64ReqHdl,"dutyStr=%s, dutyInt=%d\n", dutyStr, dutyInt);

                        if ((dutyInt < 65536) && (dutyInt >= 0))
                        {
                            MdbPrint(paraCmdLine->u64ReqHdl,"Set index: %d with duty: %d\n", indexPWM, dutyInt);
                            MDrv_PWM_DutyCycle(indexPWM, dutyInt);
                        }
                        else
                        {
                            MdbPrint(paraCmdLine->u64ReqHdl,"Invalid duty (%d)\n", dutyInt);
                        }
                    }
                }
                else
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"Invalid input (%s)\n", paraCmdLine->pcCmdLine);
                }
            }
            paraCmdLine->result = MDBRESULT_SUCCESS_FIN;
            break;
        case MDBCMD_GETINFO:
            paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
            //MdbPrint(paraGetInfo->u64ReqHdl,"LINE:%d, MDBCMD_GETINFO\n", __LINE__);
            MdbPrint(paraGetInfo->u64ReqHdl,"--------- MStar PWM Info ---------\n");
            for (i=0; i<indxMax; i++) {
                MDrv_PWM_GetProperty(E_PWM_GetPeriod, i, &periodPWM);
                MDrv_PWM_GetProperty(E_PWM_GetDutyCycle, i, &dutyPWM);
                MdbPrint(paraGetInfo->u64ReqHdl,"Index of PWM    : %d\n", i);
                MdbPrint(paraGetInfo->u64ReqHdl,"Period of PWM   : %d\n", periodPWM);
                MdbPrint(paraGetInfo->u64ReqHdl,"Duty of PWM     : %d\n", dutyPWM);
                MdbPrint(paraGetInfo->u64ReqHdl,"----------------------------------\n");
            }
            paraGetInfo->result = MDBRESULT_SUCCESS_FIN;
            break;
        default:
            MdbPrint(paraGetInfo->u64ReqHdl,"unknown cmd\n", __LINE__);
            break;
    }
    return 0;
}
#endif

void PWMRegisterToUtopia(FUtopiaOpen ModuleType)
{
    // 1. deal with module
    void* pUtopiaModule = NULL;
    UtopiaModuleCreate(MODULE_PWM, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)PWMOpen, (FUtopiaClose)PWMClose, (FUtopiaIOctl)PWMIoctl);
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    UtopiaModuleRegisterMdbNode("pwm", (FUtopiaMdbIoctl)PWMMdbIoctl);
#endif

    // 2. deal with resource
    void* psResource = NULL;
    UtopiaModuleAddResourceStart(pUtopiaModule, PWM_POOL_ID_PWM0);
    UtopiaResourceCreate("PWM", sizeof(PWM_RESOURCE_PRIVATE), &psResource);
    UtopiaResourceRegister(pUtopiaModule, psResource, PWM_POOL_ID_PWM0);
    UtopiaModuleAddResourceEnd(pUtopiaModule, PWM_POOL_ID_PWM0);

   // FLASHRegisterToUtopia only call once, so driver init code can put here,v
}

MS_U32 PWMOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{
    PWM_INSTANT_PRIVATE *pPwmPri = NULL;
    ULOGD(TAG_PWM, "\n[PWM INFO] pwm open");    
    
    UtopiaInstanceCreate(sizeof(PWM_INSTANT_PRIVATE), ppInstance);
    UtopiaInstanceGetPrivate(*ppInstance, (void**)&pPwmPri);

    pPwmPri->fpPWMOen = MDrv_PWM_Oen;
    pPwmPri->fpPWMPeriod = MDrv_PWM_Period;
    pPwmPri->fpPWMDutyCycle = MDrv_PWM_DutyCycle;
    pPwmPri->fpPWMUnitDiv = MDrv_PWM_UnitDiv;
    pPwmPri->fpPWMDiv = MDrv_PWM_Div;
    pPwmPri->fpPWMPolarity = MDrv_PWM_Polarity;
    pPwmPri->fpPWMVdben = MDrv_PWM_Vdben;
    pPwmPri->fpPWMResetEn = MDrv_PWM_ResetEn;
    pPwmPri->fpPWMDben = MDrv_PWM_Dben;
    pPwmPri->fpPWMShift = MDrv_PWM_Shift;
    pPwmPri->fpPWMNvsync = MDrv_PWM_Nvsync;
    
    MDrv_PWM_Init(E_PWM_DBGLV_ALL);
    return TRUE;
}

// FIXME: why static?
MS_U32 PWMIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
    void* pModule = NULL;
    UtopiaInstanceGetModule(pInstance, &pModule);
    void* pResource = NULL;
    
    PPWM_OEN_PARAM pOenParam = NULL;
    PPWM_PERIOD_PARAM pPeriodParam = NULL;
    PPWM_DUTYCYCLE_PARAM pDutyCycleParam = NULL;
    PPWM_UNITDIV_PARAM pUnitDivParam = NULL;
    PPWM_DIV_PARAM pDivParam = NULL;
    PPWM_POLARITY_PARAM pPolarityParam = NULL;
    PPWM_VDBEN_PARAM pVdbenParam = NULL;
    PPWM_RESETEN_PARAM pResetEnParam = NULL;
    PPWM_DBEN_PARAM pDbenParam = NULL;
    PPWM_SHIFT_PARAM pShiftParam = NULL;
    PPWM_NVSYNC_PARAM pNvsyncParam = NULL;

    MS_U32 u32Ret;
    PWM_INSTANT_PRIVATE* psPWMInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psPWMInstPri);

    switch(u32Cmd)
    {
        case MDrv_CMD_PWM_Oen:
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_Oen\n");
            pOenParam = (PPWM_OEN_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psPWMInstPri->fpPWMOen(pOenParam->u8IndexPWM,pOenParam->bOenPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_Period:	
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_Period\n");
            pPeriodParam = (PPWM_PERIOD_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psPWMInstPri->fpPWMPeriod(pPeriodParam->u8IndexPWM,pPeriodParam->u32PeriodPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_DutyCycle:	
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_DutyCycle\n");
            pDutyCycleParam = (PPWM_DUTYCYCLE_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psPWMInstPri->fpPWMDutyCycle(pDutyCycleParam->u8IndexPWM,pDutyCycleParam->u32DutyPWM); 
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_UnitDiv:	
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_UnitDiv\n");
            pUnitDivParam = (PPWM_UNITDIV_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psPWMInstPri->fpPWMUnitDiv(pUnitDivParam->u16UnitDivPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_Div:	
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_Div\n");
            pDivParam = (PPWM_DIV_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            psPWMInstPri->fpPWMDiv(pDivParam->u8IndexPWM,pDivParam->u16DivPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_Polarity:	
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_Polarity\n");
            pPolarityParam = (PPWM_POLARITY_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            psPWMInstPri->fpPWMPolarity(pPolarityParam->u8IndexPWM,pPolarityParam->bPolPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_Vdben:	
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_Vdben\n");
            pVdbenParam = (PPWM_VDBEN_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            psPWMInstPri->fpPWMVdben(pVdbenParam->u8IndexPWM,pVdbenParam->bVdbenPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_ResetEn:	
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_ResetEn\n");
            pResetEnParam = (PPWM_RESETEN_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            psPWMInstPri->fpPWMResetEn(pResetEnParam->u8IndexPWM,pResetEnParam->bRstPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_Dben:	
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_Dben\n");
            pDbenParam = (PPWM_DBEN_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            psPWMInstPri->fpPWMDben(pDbenParam->u8IndexPWM,pDbenParam->bDbenPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_Shift:	
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_Shift\n");
            pShiftParam = (PPWM_SHIFT_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psPWMInstPri->fpPWMShift(pShiftParam->u8IndexPWM,pShiftParam->u32DutyPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_PWM_Nvsync:
            ULOGD(TAG_PWM, "PWMIoctl - MDrv_CMD_PWM_Nvsync\n");
            pNvsyncParam = (PPWM_NVSYNC_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, PWM_POOL_ID_PWM0, &pResource) != 0)
            {
                ULOGE(TAG_PWM, "UtopiaResourceObtainToInstant fail\n");
	        return 0xFFFFFFFF;
            }
            psPWMInstPri->fpPWMNvsync(pNvsyncParam->u8IndexPWM,pNvsyncParam->bNvsPWM);
            UtopiaResourceRelease(pResource);
            return 0;
        default:
            break;
    }

    return 0; // FIXME: error code

}

MS_U32 PWMClose(void* pInstance)
{
    UtopiaInstanceDelete(pInstance);

    return TRUE;
}
