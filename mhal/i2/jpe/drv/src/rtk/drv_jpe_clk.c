
#include "drv_jpe_module.h"
#include "cpu_mem_map_infinity.h"
#include "_drv_jpe_dev.h"
#include "hal_jpe_ios.h"

int JpeDevClkRate(JpeDev_t* mdev, int rate)
{
    return 0;
}

int JpeDevClkOn(JpeDev_t* mdev, int on)
{
    return 0;
}

int JpeDevClkInit(JpeDev_t* mdev, u32 uDevId)
{
    // fixme, direct enable jpe clock
    u16 nRegVal;
    nRegVal = *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x103300 + 0x12 * 2) * 2)));
    nRegVal = (nRegVal & 0x0FFF);      // 480MHz
    *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x103300 + 0x12 * 2) * 2))) = nRegVal;

    return 0;
}

int JpeDevClkDeinit(JpeDev_t* mdev)
{
    return 0;
}

int JpeDevGetResourceMem(u32 uDevId, void** ppBase, int* pSize)
{
    if (ppBase && pSize)
    {
        *ppBase = (void *)(JPE_OFFSET + IO_START_ADDRESS);
        *pSize = 0x100;
        return 0;
    }

    return -1;
}

int JpeDevGetResourceIrq(u32 uDevId, int* pIrq)
{
    if (pIrq)
    {
        *pIrq = MS_INT_NUM_IRQ_JPE;
        return 0;
    }

    return -1;
}

//=============================================================================
// _DevPowserOn
//     Turn on/off clock
//
// Parameters:
//     pdev: platform device
//     bOn:  use 1 to turn on clock, and use 0 to turn off clock
//
// Return:
//     standard return value
//=============================================================================
JPE_IOC_RET_STATUS_e _DevPowserOn(JpeDev_t* pDev, int bOn)
{
    if(!pDev)
        return JPE_IOC_RET_FAIL;

    return JPE_IOC_RET_SUCCESS;
}
