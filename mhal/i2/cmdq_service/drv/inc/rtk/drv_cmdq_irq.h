#ifndef _DRVCMDQ_IRQ_H_
#define _DRVCMDQ_IRQ_H_
#if 0
typedef enum
{
    EN_CMDQIRQ_TYPE_IP0,
    EN_CMDQIRQ_TYPE_IP1,
    EN_CMDQIRQ_TYPE_IP2,
    EN_CMDQIRQ_TYPE_IP3,
    EN_CMDQIRQ_TYPE_IP4,
    EN_CMDQIRQ_TYPE_MAX
} DrvCmdqIrqIPSupport_e;

#define DRVCMDQIRQ_POLLNEQ_TIMEOUT        (0x1<<8)
#define DRVCMDQIRQ_POLLEQ_TIMEOUT         (0x1<<9)
#define DRVCMDQIRQ_WAIT_TIMEOUT           (0x1<<10)
#define DRVCMDQIRQ_WRITE_TIMEOUT          (0x1<<12)

#define DRVCMDQIRQ_ERROR_STATUS           (DRVCMDQIRQ_POLLNEQ_TIMEOUT|DRVCMDQIRQ_POLLEQ_TIMEOUT \
                                        |DRVCMDQIRQ_WAIT_TIMEOUT|DRVCMDQIRQ_WRITE_TIMEOUT)
#endif
int DrvCmdqIrqStatusInit(u32 nDrvIpId,u32 nIrqNum,void *pPrivData);


int DrvCmdqIrqHwDummyInit(u32 nDrvIpId,u32 nIrqNum,void *pPrivData);

void DrvCmdqIrqStatusDeInit(u32 nDrvIpId,u32 nIrqNum,void *pPrivData);

void DrvCmdqIrqHwDummyDeInit(u32 nDrvIpId,u32 nIrqNum,void *pPrivData);

#endif
