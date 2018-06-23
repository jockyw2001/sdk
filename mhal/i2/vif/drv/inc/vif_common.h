#include <cam_os_wrapper.h>

#define VIF_DEBUG(args...) CamOsPrintf(args)
#define IPC_DMA_ALLOC_COHERENT 0

extern void *g_ptVIF;
extern void *g_ptVIF2;
extern void *g_BDMA;
extern void *g_MCU8051;
extern void *g_PMSLEEP;
extern void *g_MAILBOX;
extern void *g_TOPPAD1;
extern void *g_ISP_ClkGen;
extern void *g_CLKGEN2;

#define VIF_MS_SLEEP(nMsec) CamOsMsSleep(nMsec)

extern void DrvVif_WdmaFiFoFullHandlerInit(void);
extern void DrvVif_WDMA_ISR(void *arg);
