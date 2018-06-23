#include "cam_os_wrapper.h"
#include "drv_cmdq_os.h"
#include "drv_cmdq_irq.h"
#include "drv_cmdq.h"
#include "hal_cmdq_dbg.h"
#include "hal_cmdq.h"


irqreturn_t _DrvCmdqStatusIsr(u32 eIntNum, void* dev_id)
{
    u32 u32IrqFlag;
    //u32 u32Time;
    CamOsTimespec_t ptRes;
    CmdqInterfacePriv_t  *pPrivData;

    pPrivData = (CmdqInterfacePriv_t*) dev_id;
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__,eIntNum);
       return IRQ_HANDLED;
    }


    //u32IrqFlag = HAL_CMDQ_Read_Irq_Status(pPrivData->nCmdqDrvIpId);
    u32IrqFlag = HAL_CMDQ_Read_FinalIrq_Status(pPrivData->nCmdqDrvIpId);
    HAL_CMDQ_Clear_IRQByFlag(pPrivData->nCmdqDrvIpId, u32IrqFlag);
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
       CamOsGetTimeOfDay(&ptRes);
       CMDQ_LOG("[CMDQ]CMDQ(%d-%d) is error irq status(0x%x)time(%u-%u)\n",pPrivData->CmdqIpId
               ,pPrivData->nCmdqIrq,u32IrqFlag,ptRes.nSec,ptRes.nNanoSec);
       MDrvCmdqPrintfCrashCommand((void*)pPrivData);
    }
    return IRQ_HANDLED;
}

irqreturn_t _DrvCmdqHwDummyIsr(u32 eIntNum, void* dev_id)
{
    CmdqInterfacePriv_t  *pPrivData;

    pPrivData = (CmdqInterfacePriv_t*) dev_id;
    if(pPrivData == NULL)
    {
       CMDQ_ERR("%s cmdq IRQ(%d) Unknow PrivData\n",__FUNCTION__,eIntNum);
       return IRQ_HANDLED;
    }

    HAL_CMDQ_Clear_HwDummy_Register(pPrivData->nCmdqDrvIpId);
    CMDQ_LOG("HwDummyIsr cmdq(%d)\n",pPrivData->nCmdqDrvIpId);
    return IRQ_HANDLED;
}

static char aName[EN_CMDQ_TYPE_MAX][12] = {{0,},};

int DrvCmdqIrqStatusInit(u32 nDrvIpId,u32 nIrqNum,void *pPrivData)
{

    if(nDrvIpId >=EN_CMDQ_TYPE_MAX)
    {
        CMDQ_ERR("%s Ipid err(%d)\n",__FUNCTION__,nDrvIpId);
        return -1;
    }
    sprintf(&aName[nDrvIpId][0], "cmdq%d", nDrvIpId);

    CMDQ_LOG("[CMDQ]cmdq(%d) isr = %d, name=%s\n",nDrvIpId,nIrqNum, &aName[nDrvIpId][0]);

    if(DrvCmdqOsAttachInterrupt(nIrqNum, _DrvCmdqStatusIsr, 0, &aName[nDrvIpId][0],pPrivData))
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

    if(nDrvIpId >=EN_CMDQ_TYPE_MAX)
    {
        CMDQ_ERR("%s Ipid err(%d)\n",__FUNCTION__,nDrvIpId);
        return -1;
    }

    sprintf(aName, "dum%d", nDrvIpId);
    CMDQ_LOG("[CMDQ]dum(%d) isr = %d\n",nDrvIpId,nIrqNum);

    if(DrvCmdqOsAttachInterrupt(nIrqNum, _DrvCmdqHwDummyIsr, 0, aName, pPrivData))
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

    DrvCmdqOsDisableInterrupt(nIrqNum);
    DrvCmdqOsDetachInterrupt(nIrqNum,pPrivData);
}
