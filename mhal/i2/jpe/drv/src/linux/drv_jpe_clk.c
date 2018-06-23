
#include "_drv_jpe_dev.h"
#include "hal_jpe_ios.h"
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

extern unsigned int jpe_clock;

int JpeDevClkRate(JpeDev_t* mdev, int rate)
{
    struct clk* clock = mdev->pClock[0];
    struct clk* clk;
    int i = 0, best = 0;
    while (!(clk = clk_get_parent_by_index(clock, i++)))
    {
        int r = clk_get_rate(clk);

        if (rate > best && best < r)
            best = r;
        if (rate < best && rate < r && r < best)
            best = r;
    }
    return best;
}

int JpeDevClkOn(JpeDev_t* mdev, int on)
{
    int i = on!=0;
    struct clk* clk;
    JpeIosCB_t* mios = mdev->p_asicip;
    mios->irqMask(mios, 0xFF);
    if (i == 0)
    {
        clk = mdev->pClock[0];
        clk_set_parent(clk, clk_get_parent_by_index(clk, 0));
        while (i < JPE_CLOCKS_NR && (clk = mdev->pClock[i++]))
            clk_disable_unprepare(clk);
        return 0;
    }
    while (i < JPE_CLOCKS_NR && (clk = mdev->pClock[i++]))
        clk_prepare_enable(clk);
    clk = mdev->pClock[0];
    clk_set_rate(clk, mdev->nClockRate);
    clk_prepare_enable(clk);
    return 0;
}

int JpeClkInit(void)
{
    u16 nRegVal;
    u16 nJPGClkReg;

    nRegVal = *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x103300 + 0x12 * 2) * 2)));

    //check clock setting
    nJPGClkReg = nRegVal & 0xF000;

    if(4 == jpe_clock)         // 480MHz
    {
        nRegVal = (nRegVal & 0x0FFF) | 0x0000;
    }
    else if(3 == jpe_clock)    // 432MHz
    {
        nRegVal = (nRegVal & 0x0FFF) | 0x4000;
    }
    else if(2 == jpe_clock)    // 288MHz
    {
        nRegVal = (nRegVal & 0x0FFF) | 0x8000;
    }
    else                  //default setting 480MHz
    {
        if(0x0000 == (nJPGClkReg & 0xC000))         // 480MHz
        {
            nRegVal = (nRegVal & 0x0FFF) | 0x0000;
            jpe_clock = 4;
        }
        else if(0x4000 == (nJPGClkReg & 0xC000))    // 432MHz
        {
            nRegVal = (nRegVal & 0x0FFF) | 0x4000;
            jpe_clock = 3;
        }
        else if(0x8000 == (nJPGClkReg & 0xC000))    // 288MHz
        {
            nRegVal = (nRegVal & 0x0FFF) | 0x8000;
            jpe_clock = 2;
        }
        else
        {
            nRegVal = (nRegVal & 0x0FFF) | 0x0000;
            jpe_clock = 4;
        }
    }

    *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x103300 + 0x12 * 2) * 2))) = nRegVal;
    //CamOsPrintf("JPE clock setting: %d\n", clock);

    return 0;
}

int JpeDevClkInit(JpeDev_t* mdev, u32 uDevId)
{
    struct device_node *np = NULL;
    struct clk* clock;
    int i;

    // fixme, direct enable jpe clock
    u16 nRegVal;
    nRegVal = *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x103300 + 0x12 * 2) * 2)));
    nRegVal = (nRegVal & 0x0FFF) | 0x0000;      // 480MHz
    //nRegVal = (nRegVal & 0x0FFF) | 0x4000;      // 432MHz
    //nRegVal = (nRegVal & 0x0FFF) | 0x8000;      // 288MHz
    *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x103300 + 0x12 * 2) * 2))) = nRegVal;

    do
    {
        np = of_find_compatible_node(NULL, NULL, "mstar,cedric-jpe");
        clock = of_clk_get_by_name(np, "CKG_jpe");
        if (IS_ERR(clock))
        {
            break;
        }
        mdev->pClock[0] = clock;
        mdev->nClockIdx = 0;
        mdev->nClockRate = clk_get_rate(clk_get_parent_by_index(clock, mdev->nClockIdx));

        for (i = 1; i < JPE_CLOCKS_NR; i++)
        {
            clock = of_clk_get(np, i);
            if (IS_ERR(clock))
            {
                break;
            }
            mdev->pClock[i] = clock;
        }

        return 0;
    }
    while (0);

    return -1;
}

int JpeDevClkDeinit(JpeDev_t* mdev)
{
    clk_put(mdev->pClock[0]);

    return 0;
}

int JpeDevGetResourceMem(u32 uDevId, void** ppBase, int* pSize)
{
    struct device_node *np;
    struct resource res;

    if (ppBase && pSize)
    {
        np = of_find_compatible_node(NULL, NULL, "mstar,cedric-jpe");
        if (!np || of_address_to_resource(np, 0, &res))
        {
            return -1;
        }

        //CamOsPrintf("res.start: %p    res.end: %p\n", res.start, res.end);

        *ppBase = (void *)IO_ADDRESS(res.start);
        *pSize = res.end - res.start;
        return 0;
    }

    return -1;
}

int JpeDevGetResourceIrq(u32 uDevId, int* pIrq)
{
    struct device_node *np;

    if (pIrq)
    {
        np = of_find_compatible_node(NULL, NULL, "mstar,cedric-jpe");
        if (!np)
        {
            return -1;
        }
        *pIrq = irq_of_parse_and_map(np, 0);
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
    int nRet;
    struct clk* pClock;

    if(!pDev)
        return JPE_IOC_RET_FAIL;

    pClock = pDev->pClock[0];
    if(pClock)
    {
        if(JPE_CLOCK_ON == bOn)
        {
            nRet = clk_set_rate(pClock, pDev->nClockRate);
            if(nRet)
            {
                JPE_MSG(JPE_MSG_ERR, "clk_set_rate() Fail, ret=%d\n", nRet);
                return JPE_IOC_RET_FAIL;
            }
            nRet = clk_prepare_enable(pClock);
            if(nRet)
            {
                JPE_MSG(JPE_MSG_ERR, "clk_prepare_enable() Fail, ret=%d\n", nRet);
                return JPE_IOC_RET_FAIL;
            }
            pDev->i_users++;
        }
        else
        {
            clk_disable_unprepare(pClock);
            pDev->i_users--;
        }
    }

    return JPE_IOC_RET_SUCCESS;
}
