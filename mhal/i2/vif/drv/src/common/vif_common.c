#include <vif_common.h>
#include <cam_os_wrapper.h>
#include "mdrv_vif_io.h"
#include "hal_dma.h"

void *g_ptVIF = NULL;  // BANK:0x1502 ~ 0x1507
void *g_ptVIF2 = NULL; // BANK:0x1705 ~ 0x1706
void *g_BDMA = NULL;   // BANK:0x1009 bdma ch0
void *g_MCU8051 = NULL; // BANK:0x0010 MCU
void *g_PMSLEEP = NULL; // BANK:0x000e PM_SLEEP
void *g_MAILBOX = NULL; // BANK:0x1033H MAILBOX
void *g_TOPPAD1 = NULL; // BANK:0x1026H TOPPAD1
void *g_ISP_ClkGen = NULL; // BANK:0x1432H reg_block_ispsc
void *g_CLKGEN2 = NULL; // BANK:0x100AH CLKGEN2

#if IPC_DMA_ALLOC_COHERENT
u32 IPCRamPhys = 0;
char *IPCRamVirtAddr  = NULL;
#else
unsigned long IPCRamPhys = 0;
void *IPCRamVirtAddr = NULL;
#endif

CamOsTcond_t tTcond;
CamOsThread tThread;

struct WdmaIsrArg_t{

    u8 uChannel;

}tWdmaIsrArg;

#if 1

u8 GetChannel(unsigned int status){

    int i=0;
    int j=0;

    for(i=0; i<16;i++){

        if( (status & 0x01) == 1)
            return j;
        else
        {
            status = status >> 1;
            j++;

        }

    }

    return 0;
}


void DrvVif_WDMA_ISR(void *arg)
{

    unsigned int dma_fifo_status;

    dma_fifo_status = HalDma_FifoFullIrqFinalStatus(0);

    if(dma_fifo_status  != 0){

        VIF_DEBUG("##fifo full### %x\n",dma_fifo_status);

        HalDma_FifoFullIrqMask(0,dma_fifo_status);

        tWdmaIsrArg.uChannel = GetChannel(dma_fifo_status);
        CamOsTcondSignal(&tTcond);

        HalDma_FifoFullIrqClr(0,dma_fifo_status);
        HalDma_FifoFullIrqUnMask(0,dma_fifo_status);
    }

    return;
}

void* DrvVif_WdmaFiFoFullHandler(void *arg)
{

    struct WdmaIsrArg_t *wdmaArg = (struct WdmaIsrArg_t *)arg;

    while(1){

        CamOsTcondWaitInterruptible(&tTcond);

        VIF_DEBUG("reset ch:%d\n",wdmaArg->uChannel);

        //HalDma_CMDQ_SyncResetDMA(wdmaArg->uChannel);

    }

    return NULL;
}

void DrvVif_WdmaFiFoFullHandlerInit(void)
{

    CamOsThreadAttrb_t tAttr;

    /* Initialize condition wait object*/
    CamOsTcondInit(&tTcond);

    /* Create thread */
    tAttr.nPriority = 99;
    tAttr.nStackSize = 0;

    CamOsThreadCreate(&tThread, &tAttr,DrvVif_WdmaFiFoFullHandler,&tWdmaIsrArg);

    return;
}
#endif
