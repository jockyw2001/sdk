#include "cam_os_wrapper.h"
#include "drv_cmdq_os.h"
#include "drv_cmdq_irq.h"
#include "drv_cmdq.h"
#include "hal_cmdq_dbg.h"
#include "hal_cmdq.h"

typedef struct _CmdqIrqDataPriv_s
{
    u32         pCmdqIrqStatusUsed[EN_CMDQ_TYPE_MAX];
    u32         pCmdqIrqHwDummyUsed[EN_CMDQ_TYPE_MAX];
    InterruptCb pCmdqIrqStatusFunc[EN_CMDQ_TYPE_MAX];
    InterruptCb pCmdqIrqHwDummyFunc[EN_CMDQ_TYPE_MAX];
    u32         pCmdqIrqStatusIrqNum[EN_CMDQ_TYPE_MAX];
    u32         pCmdqIrqHwDummyIrqNum[EN_CMDQ_TYPE_MAX];
    void*       pCmdqIrqStatusPriData[EN_CMDQ_TYPE_MAX];
    void*       pCmdqIrqHwDummyPriData[EN_CMDQ_TYPE_MAX];
} CmdqIrqDataPriv_t;

static   int   gCmdqIrqInitDone = 0;
CmdqIrqDataPriv_t   gCmdqIrqPrivData;

void _DrvCmdqStatusIsr1(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP0;
    u32 u32IrqFlag;
    //u32 u32Time;
    CmdqInterfacePriv_t  *pPrivData;

    if(!gCmdqIrqPrivData.pCmdqIrqStatusUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqStatusPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }


    //u32IrqFlag = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    u32IrqFlag = HAL_CMDQ_Read_FinalIrq_Status(pPrivData->nCmdqDrvIpId);
    HAL_CMDQ_Clear_IRQByFlag(pPrivData->nCmdqDrvIpId, 0xFFFF);
#if 0
    //CMDQ_LOG("pPrivData->irq dev(%d) irq(%d)\n",pPrivData->CmdqIpId,pPrivData->nCmdqIrq);
    if(u32IrqFlag & DRVCMDQ_IRQ_STATE_DONE)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is done irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       //CamOsTsemSignal(&pPrivData->tIrqWaitSem);
    }
#endif
    if(u32IrqFlag & DRVCMDQ_ERROR_STATUS)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is error irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       MDrvCmdqPrintfCrashCommand((void*)pPrivData);
    }
}

void _DrvCmdqStatusIsr2(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP1;
    u32 u32IrqFlag;
    //u32 u32Time;
    CmdqInterfacePriv_t  *pPrivData;

    if(!gCmdqIrqPrivData.pCmdqIrqStatusUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqStatusPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }


    //u32IrqFlag = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    u32IrqFlag = HAL_CMDQ_Read_FinalIrq_Status(pPrivData->nCmdqDrvIpId);
    HAL_CMDQ_Clear_IRQByFlag(pPrivData->nCmdqDrvIpId, 0xFFFF);
#if 0
    //CMDQ_LOG("pPrivData->irq dev(%d) irq(%d)\n",pPrivData->CmdqIpId,pPrivData->nCmdqIrq);
    if(u32IrqFlag & DRVCMDQ_IRQ_STATE_DONE)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is done irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       //CamOsTsemSignal(&pPrivData->tIrqWaitSem);
    }
#endif
    if(u32IrqFlag & DRVCMDQ_ERROR_STATUS)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is error irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       MDrvCmdqPrintfCrashCommand((void*)pPrivData);
    }
}

void _DrvCmdqStatusIsr3(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP2;
    u32 u32IrqFlag;
    //u32 u32Time;
    CmdqInterfacePriv_t  *pPrivData;

    if(!gCmdqIrqPrivData.pCmdqIrqStatusUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqStatusPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }


    //u32IrqFlag = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    u32IrqFlag = HAL_CMDQ_Read_FinalIrq_Status(pPrivData->nCmdqDrvIpId);
    HAL_CMDQ_Clear_IRQByFlag(pPrivData->nCmdqDrvIpId, 0xFFFF);
#if 0
    //CMDQ_LOG("pPrivData->irq dev(%d) irq(%d)\n",pPrivData->CmdqIpId,pPrivData->nCmdqIrq);
    if(u32IrqFlag & DRVCMDQ_IRQ_STATE_DONE)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is done irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       //CamOsTsemSignal(&pPrivData->tIrqWaitSem);
    }
#endif
    if(u32IrqFlag & DRVCMDQ_ERROR_STATUS)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is error irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       MDrvCmdqPrintfCrashCommand((void*)pPrivData);
    }
}

void _DrvCmdqStatusIsr4(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP3;
    u32 u32IrqFlag;
    //u32 u32Time;
    CmdqInterfacePriv_t  *pPrivData;

    if(!gCmdqIrqPrivData.pCmdqIrqStatusUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqStatusPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }


    //u32IrqFlag = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    u32IrqFlag = HAL_CMDQ_Read_FinalIrq_Status(pPrivData->nCmdqDrvIpId);
    HAL_CMDQ_Clear_IRQByFlag(pPrivData->nCmdqDrvIpId, 0xFFFF);
#if 0
    //CMDQ_LOG("pPrivData->irq dev(%d) irq(%d)\n",pPrivData->CmdqIpId,pPrivData->nCmdqIrq);
    if(u32IrqFlag & DRVCMDQ_IRQ_STATE_DONE)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is done irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       //CamOsTsemSignal(&pPrivData->tIrqWaitSem);
    }
#endif
    if(u32IrqFlag & DRVCMDQ_ERROR_STATUS)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is error irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       MDrvCmdqPrintfCrashCommand((void*)pPrivData);
    }
}

void _DrvCmdqStatusIsr5(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP4;
    u32 u32IrqFlag;
    //u32 u32Time;
    CmdqInterfacePriv_t  *pPrivData;

    if(!gCmdqIrqPrivData.pCmdqIrqStatusUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqStatusPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }


    //u32IrqFlag = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    u32IrqFlag = HAL_CMDQ_Read_FinalIrq_Status(pPrivData->nCmdqDrvIpId);
    HAL_CMDQ_Clear_IRQByFlag(pPrivData->nCmdqDrvIpId, 0xFFFF);
#if 0
    //CMDQ_LOG("pPrivData->irq dev(%d) irq(%d)\n",pPrivData->CmdqIpId,pPrivData->nCmdqIrq);
    if(u32IrqFlag & DRVCMDQ_IRQ_STATE_DONE)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is done irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       //CamOsTsemSignal(&pPrivData->tIrqWaitSem);
    }
#endif
    if(u32IrqFlag & DRVCMDQ_ERROR_STATUS)
    {
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is error irq status(0x%x)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag);
       MDrvCmdqPrintfCrashCommand((void*)pPrivData);
    }
}

void _DrvCmdqHwDummyIsr1(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP0;

    CmdqInterfacePriv_t  *pPrivData;


    if(!gCmdqIrqPrivData.pCmdqIrqHwDummyUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqHwDummyPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }

    HAL_CMDQ_Clear_HwDummy_Register(pPrivData->nCmdqDrvIpId);
    CMDQ_LOG("HwDummyIsr cmdq(%d)\n",pPrivData->nCmdqDrvIpId);
}

void _DrvCmdqHwDummyIsr2(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP1;

    CmdqInterfacePriv_t  *pPrivData;


    if(!gCmdqIrqPrivData.pCmdqIrqHwDummyUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqHwDummyPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }

    HAL_CMDQ_Clear_HwDummy_Register(pPrivData->nCmdqDrvIpId);
    CMDQ_LOG("HwDummyIsr cmdq(%d)\n",pPrivData->nCmdqDrvIpId);
}

void _DrvCmdqHwDummyIsr3(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP2;

    CmdqInterfacePriv_t  *pPrivData;


    if(!gCmdqIrqPrivData.pCmdqIrqHwDummyUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqHwDummyPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }

    HAL_CMDQ_Clear_HwDummy_Register(pPrivData->nCmdqDrvIpId);
    CMDQ_LOG("HwDummyIsr cmdq(%d)\n",pPrivData->nCmdqDrvIpId);
}

void _DrvCmdqHwDummyIsr4(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP3;

    CmdqInterfacePriv_t  *pPrivData;


    if(!gCmdqIrqPrivData.pCmdqIrqHwDummyUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqHwDummyPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }

    HAL_CMDQ_Clear_HwDummy_Register(pPrivData->nCmdqDrvIpId);
    CMDQ_LOG("HwDummyIsr cmdq(%d)\n",pPrivData->nCmdqDrvIpId);
}

void _DrvCmdqHwDummyIsr5(void)
{
    u32 nPrivNum = EN_CMDQ_TYPE_IP4;

    CmdqInterfacePriv_t  *pPrivData;


    if(!gCmdqIrqPrivData.pCmdqIrqHwDummyUsed[nPrivNum])
    {
       CMDQ_ERR("%s cmdq NO USED PrivData\n",__FUNCTION__);
       return;
    }

    pPrivData = (CmdqInterfacePriv_t*) gCmdqIrqPrivData.pCmdqIrqHwDummyPriData[nPrivNum];
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__);
       return;
    }

    HAL_CMDQ_Clear_HwDummy_Register(pPrivData->nCmdqDrvIpId);
    CMDQ_LOG("HwDummyIsr cmdq(%d)\n",pPrivData->nCmdqDrvIpId);
}

void _DrvCmdqIrqInit(void)
{
    memset(&gCmdqIrqPrivData,0x0,sizeof(CmdqIrqDataPriv_t));
    gCmdqIrqPrivData.pCmdqIrqStatusFunc[0] = _DrvCmdqStatusIsr1;
    gCmdqIrqPrivData.pCmdqIrqStatusFunc[1] = _DrvCmdqStatusIsr2;
    gCmdqIrqPrivData.pCmdqIrqStatusFunc[2] = _DrvCmdqStatusIsr3;
    gCmdqIrqPrivData.pCmdqIrqStatusFunc[3] = _DrvCmdqStatusIsr4;
    gCmdqIrqPrivData.pCmdqIrqStatusFunc[4] = _DrvCmdqStatusIsr5;

    gCmdqIrqPrivData.pCmdqIrqHwDummyFunc[0] = _DrvCmdqHwDummyIsr1;
    gCmdqIrqPrivData.pCmdqIrqHwDummyFunc[1] = _DrvCmdqHwDummyIsr2;
    gCmdqIrqPrivData.pCmdqIrqHwDummyFunc[2] = _DrvCmdqHwDummyIsr3;
    gCmdqIrqPrivData.pCmdqIrqHwDummyFunc[3] = _DrvCmdqHwDummyIsr4;
    gCmdqIrqPrivData.pCmdqIrqHwDummyFunc[4] = _DrvCmdqHwDummyIsr5;
}

int DrvCmdqIrqStatusInit(u32 nDrvIpId,u32 nIrqNum,void *pPrivData)
{
    char aName[12];
    if(!gCmdqIrqInitDone)
    {
        gCmdqIrqInitDone = 1;
        _DrvCmdqIrqInit();
    }
    if(nDrvIpId >=EN_CMDQ_TYPE_MAX)
    {
        CMDQ_ERR("%s Ipid err(%d)\n",__FUNCTION__,nDrvIpId);
        return -1;
    }
    sprintf(aName, "cmdq%d", nDrvIpId);

    CMDQ_LOG("[CMDQ]cmdq(%d) isr = %d\n",nDrvIpId,nIrqNum);
    if(gCmdqIrqPrivData.pCmdqIrqStatusUsed[nDrvIpId])
    {
       CMDQ_ERR("%s Ipid Irq init twice(%d)\n",__FUNCTION__,nDrvIpId);
       return -1;
    }
    gCmdqIrqPrivData.pCmdqIrqStatusUsed[nDrvIpId] = 1;
    gCmdqIrqPrivData.pCmdqIrqStatusIrqNum[nDrvIpId] = nIrqNum;
    gCmdqIrqPrivData.pCmdqIrqStatusPriData[nDrvIpId] = pPrivData;

    if(DrvCmdqOsAttachInterrupt(nIrqNum, gCmdqIrqPrivData.pCmdqIrqStatusFunc[nDrvIpId], 0, aName,pPrivData))
    {
        CMDQ_ERR("CMDQ request irq(%d) fail\n",nIrqNum);
        return -1;
    }
    DrvCmdqOsDisableInterrupt(nIrqNum);
    DrvCmdqOsEnableInterrupt(nIrqNum);
    return 0;
}

int DrvCmdqIrqHwDummyInit(u32 nDrvIpId,u32 nIrqNum,void *pPrivData)
{
    char aName[12];
    if(!gCmdqIrqInitDone)
    {
        gCmdqIrqInitDone = 1;
        _DrvCmdqIrqInit();
    }

    if(nDrvIpId >=EN_CMDQ_TYPE_MAX)
    {
        CMDQ_ERR("%s Ipid err(%d)\n",__FUNCTION__,nDrvIpId);
        return -1;
    }
    sprintf(aName, "dum%d", nDrvIpId);
    CMDQ_LOG("[CMDQ]dum(%d) isr = %d\n",nDrvIpId,nIrqNum);

    if(gCmdqIrqPrivData.pCmdqIrqHwDummyUsed[nDrvIpId])
    {
       CMDQ_ERR("%s Ipid Irq init twice(%d)\n",__FUNCTION__,nDrvIpId);
       return -1;
    }

    gCmdqIrqPrivData.pCmdqIrqHwDummyUsed[nDrvIpId] = 1;
    gCmdqIrqPrivData.pCmdqIrqHwDummyIrqNum[nDrvIpId] = nIrqNum;
    gCmdqIrqPrivData.pCmdqIrqHwDummyPriData[nDrvIpId] = pPrivData;
    if(DrvCmdqOsAttachInterrupt(nIrqNum, gCmdqIrqPrivData.pCmdqIrqHwDummyFunc[nDrvIpId], 0, aName, pPrivData))
    {
        CMDQ_ERR("CMDQ request irq(%d) fail\n", nIrqNum);
    }
    DrvCmdqOsDisableInterrupt(nIrqNum);
    DrvCmdqOsEnableInterrupt(nIrqNum);
    return 0;
}

void DrvCmdqIrqStatusDeInit(u32 nDrvIpId,u32 nIrqNum,void *pPrivData)
{
    if(nDrvIpId >=EN_CMDQ_TYPE_MAX)
    {
        CMDQ_ERR("%s Ipid err(%d)\n",__FUNCTION__,nDrvIpId);
        return;
    }

    if(gCmdqIrqPrivData.pCmdqIrqStatusUsed[nDrvIpId])
    {
       CMDQ_ERR("%s Ipid Irq Not init (%d)\n",__FUNCTION__,nDrvIpId);
       return;
    }
    gCmdqIrqPrivData.pCmdqIrqStatusUsed[nDrvIpId] = 0;
    DrvCmdqOsDisableInterrupt(nIrqNum);
    DrvCmdqOsDetachInterrupt(nIrqNum,pPrivData);
}
void DrvCmdqIrqHwDummyDeInit(u32 nDrvIpId,u32 nIrqNum,void *pPrivData)
{
    if(nDrvIpId >=EN_CMDQ_TYPE_MAX)
    {
        CMDQ_ERR("%s Ipid err(%d)\n",__FUNCTION__,nDrvIpId);
        return;
    }

    if(gCmdqIrqPrivData.pCmdqIrqStatusUsed[nDrvIpId])
    {
       CMDQ_ERR("%s Ipid Irq Not init (%d)\n",__FUNCTION__,nDrvIpId);
       return;
    }
    gCmdqIrqPrivData.pCmdqIrqStatusUsed[nDrvIpId] = 0;
    DrvCmdqOsDisableInterrupt(nIrqNum);
    DrvCmdqOsDetachInterrupt(nIrqNum,pPrivData);
}
