
#include "drv_mfe_dev.h"
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

extern unsigned int mfe_clock;

int MfeDevClkRate(mmfe_dev* mdev, int rate)
{
    struct clk* clock = mdev->p_clocks[0];
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

int MfeDevClkOn(mmfe_dev* mdev, int on)
{
    int i = on!=0;
    struct clk* clk;
    mhve_ios* mios = mdev->p_asicip;
    mios->irq_mask(mios, 0xFF);
    if (i == 0)
    {
        clk = mdev->p_clocks[0];
        clk_set_parent(clk, clk_get_parent_by_index(clk, 0));
        while (i < MMFE_CLOCKS_NR && (clk = mdev->p_clocks[i++]))
            clk_disable_unprepare(clk);
        return 0;
    }
    while (i < MMFE_CLOCKS_NR && (clk = mdev->p_clocks[i++]))
        clk_prepare_enable(clk);
    clk = mdev->p_clocks[0];
    clk_set_rate(clk, mdev->i_ratehz);
    clk_prepare_enable(clk);
    return 0;
}

int MfeClkInit(void)
{
    u16 nRegVal;
    u16 nMFEClkReg;

    nRegVal = *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2)));

    //check clock setting
    nMFEClkReg = nRegVal & 0x00FF;

    if(0x0018 == (nMFEClkReg & 0x00FC))         // 576MHz
    {
        nRegVal = (nRegVal & 0xFF00) | 0x0018;
        mfe_clock = 5;
    }
    else if(0x0004 == (nMFEClkReg & 0x00FC))    // 480MHz
    {
        nRegVal = (nRegVal & 0xFF00) | 0x0004;
        mfe_clock = 4;
    }
    else if(0x000C == (nMFEClkReg & 0x00FC))    // 384MHz
    {
        nRegVal = (nRegVal & 0xFF00) | 0x000C;
        mfe_clock = 3;
    }
    else if(0x0010 == (nMFEClkReg & 0x00FC))     // 288MHz
    {
        nRegVal = (nRegVal & 0xFF00) | 0x0010;
        mfe_clock = 2;
    }
    else if(0x0014 == (nMFEClkReg & 0x00FC))     // 192MHz
    {
        nRegVal = (nRegVal & 0xFF00) | 0x0014;
        mfe_clock = 1;
    }
    else
    {
        if(5 == mfe_clock)         // 576MHz
        {
            nRegVal = (nRegVal & 0xFF00) | 0x0018;
        }
        else if(4 == mfe_clock)    // 480MHz
        {
            nRegVal = (nRegVal & 0xFF00) | 0x0004;
        }
        else if(3 == mfe_clock)    // 384MHz
        {
            nRegVal = (nRegVal & 0xFF00) | 0x000C;
        }
        else if(2 == mfe_clock)    // 288MHz
        {
            nRegVal = (nRegVal & 0xFF00) | 0x0010;
        }
        else if(1 == mfe_clock)    // 192MHz
        {
            nRegVal = (nRegVal & 0xFF00) | 0x0014;
        }
        else                  //default setting 576Mhz
        {
            nRegVal = (nRegVal & 0xFF00) | 0x0018;
            mfe_clock = 5;
        }
    }

    *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2))) = nRegVal;
    //CamOsPrintf("MFE mfe_clock setting: %d\n", mfe_clock);

    return 0;
}

int MfeDevClkInit(mmfe_dev* mdev, u32 nDevId)
{
    struct device_node *np;
    char compatible[16];
    struct clk* clock;
    int i;

    // fixme, direct enable mfe clock
    u16 nRegVal;
    nRegVal = *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2)));
    //nRegVal = (nRegVal & 0xFF00) | 0x0004;      // 480MHz
    nRegVal = (nRegVal & 0xFF00) | 0x0018;      // 576MHz
    //nRegVal = (nRegVal & 0xFF00) | 0x0008;      // 432MHz
    *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2))) = nRegVal;


    do
    {
        if (nDevId)
            CamOsSnprintf(compatible, sizeof(compatible), "mstar,mfe%d", nDevId);
        else
            CamOsSnprintf(compatible, sizeof(compatible), "mstar,mfe");
        np = of_find_compatible_node(NULL, NULL, compatible);
        clock = of_clk_get_by_name(np, "CKG_mfe");
        if (IS_ERR(clock))
            break;
        mdev->p_clocks[0] = clock;
        mdev->i_clkidx = 0;
        mdev->i_ratehz = clk_get_rate(clk_get_parent_by_index(clock, mdev->i_clkidx));

        for (i = 1; i < MMFE_CLOCKS_NR; i++)
        {
            clock = of_clk_get(np, i);
            if (IS_ERR(clock))
                break;
            mdev->p_clocks[i] = clock;
        }

        return 0;
    }
    while (0);

    return -1;
}

int MfeDevGetResourceMem(u32 nDevId, void** ppBase, int* pSize)
{
    struct device_node *np;
    struct resource res;
    char compatible[16];

    if (ppBase && pSize)
    {
        if (nDevId)
            CamOsSnprintf(compatible, sizeof(compatible), "mstar,mfe%d", nDevId);
        else
            CamOsSnprintf(compatible, sizeof(compatible), "mstar,mfe");
        np = of_find_compatible_node(NULL, NULL, compatible);
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

int MfeDevGetResourceIrq(u32 nDevId, int* pIrq)
{
    struct device_node *np;
    char compatible[16];

    if (pIrq)
    {
        if (nDevId)
            CamOsSnprintf(compatible, sizeof(compatible), "mstar,mfe%d", nDevId);
        else
            CamOsSnprintf(compatible, sizeof(compatible), "mstar,mfe");
        np = of_find_compatible_node(NULL, NULL, compatible);
        if (!np)
        {
            return -1;
        }
        *pIrq = irq_of_parse_and_map(np, 0);
        return 0;
    }

    return -1;
}
