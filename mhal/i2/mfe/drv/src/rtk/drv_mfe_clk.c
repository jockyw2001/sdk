#include "cpu_mem_map_infinity.h"
#include "drv_mfe_dev.h"
#include "drv_mfe_module.h"

#define MFE_CLKSPED_NUM 7
static const DrvMfeClk_t _MfeClkOptionTbl[] =
{
    {CLKGEN_DRV_MFE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MFE,     CLKGEN_288MHZ,    0x0},
    {CLKGEN_DRV_MFE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MFE,     CLKGEN_240MHZ,    0x1},
    {CLKGEN_DRV_MFE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MFE,     CLKGEN_192MHZ,    0x2},
    {CLKGEN_DRV_MFE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MFE,     CLKGEN_123P4MHZ,  0x3},
    {CLKGEN_DRV_MFE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MFE,     CLKGEN_384MHZ,    0x4},
    {CLKGEN_DRV_MFE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MFE,     CLKGEN_320MHZ,    0x5},
    {CLKGEN_DRV_MFE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MFE,     CLKGEN_172P8MHZ,  0x6},
    {CLKGEN_MOD_INVALID,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_INVALID,    CLKGEN_INVALID,   0xFFFF}
};

int MfeDevClkRate(mmfe_dev* mdev, int rate)
{
    int i = 0, best = 0, r = 0;
    for(i = 0; i < MFE_CLKSPED_NUM; i++)
    {
        r = _MfeClkOptionTbl[i].eClkSpeed;

        if (rate > best && best < r)
            best = r;
        if (rate < best && rate < r && r < best)
            best = r;
    }
    return best;
}

int MfeDevClkOn(mmfe_dev* mdev, int on)
{
#if 1
    int i = on!=0;
    DrvMfeClk_t* clk;
    mhve_ios* mios = mdev->p_asicip;
    mios->irq_mask(mios, 0xFF);
    clk = mdev->p_clocks[0];
    if (i == 0)
    {
        clk->eClkReq = CLKGEN_CLK_REQ_OFF;
        DrvClkgenDrvClockCtl(clk->eModule, clk->eClkReq);
        return 0;
    }
    clk->eClkReq = CLKGEN_CLK_REQ_ON;
    DrvClkgenDrvClockCtl(clk->eModule, clk->eClkReq);
    if(mdev->i_clkidx >= 0)
        DrvClkgenDrvClockSelectSrc(clk->eModule, clk->eClk, mdev->i_clkidx);
    else
        DrvClkgenDrvClockUpdate(clk->eModule, clk->eClk, mdev->i_ratehz);
#else   // Need to check clock setting in MfeProbe()
    int i = on!=0;
    DrvMfeClk_t clock;
    mhve_ios* mios = mdev->p_asicip;
    mios->irq_mask(mios, 0xFF);

    //clock settings
    clock.eModule = CLKGEN_DRV_MFE;
    clock.eClkReq = CLKGEN_CLK_REQ_ON;
    clock.eClk = TOP_CLK_MFE;
    clock.eClkSpeed = CLKGEN_288MHZ;
    clock.nClkSrc = 0x0;
    mdev->i_clkidx = clock.nClkSrc ;
    mdev->i_ratehz = clock.eClkSpeed;

    if (i == 0)
    {
        clock.eClkReq = CLKGEN_CLK_REQ_OFF;
        DrvClkgenDrvClockCtl(clock.eModule, clock.eClkReq);
        return 0;
    }
    clock.eClkReq = CLKGEN_CLK_REQ_ON;
    DrvClkgenDrvClockCtl(clock.eModule, clock.eClkReq);
    if(mdev->i_clkidx >= 0)
        DrvClkgenDrvClockSelectSrc(clock.eModule, clock.eClk, mdev->i_clkidx);
    else
        DrvClkgenDrvClockUpdate(clock.eModule, clock.eClk, mdev->i_ratehz);
#endif

    return 0;
}

int MfeDevClkInit(mmfe_dev* mdev, u32 nDevId)
{
    // fixme, direct enable mfe clock
    u16 nRegVal;
    nRegVal = *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2)));
    nRegVal = (nRegVal & 0xFF00) | 0x0004;      // 480MHz
    *(volatile u16 *)(IO_ADDRESS((0x1F000000 + (0x100a00 + 0x10 * 2) * 2))) = nRegVal;

    return 0;
}

int MfeDevClkDeinit(mmfe_dev* mdev)
{
    return 0;
}

int MfeDevGetResourceMem(u32 nDevId, void** ppBase, int* pSize)
{
    if (ppBase && pSize)
    {
        *ppBase = (void *)(MFE_OFFSET + IO_START_ADDRESS);
        *pSize = 0x100;
        return 0;
    }

    return -1;
}

int MfeDevGetResourceIrq(u32 nDevId, int* pIrq)
{
    if (pIrq)
    {
        *pIrq = MS_INT_NUM_IRQ_MFE;
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
MFE_IOC_RET_STATUS_e _DevPowserOn(MfeDev_t* pDev, int bOn)
{
    if(!pDev)
        return MFE_IOC_RET_FAIL;

    return MFE_IOC_RET_SUCCESS;
}
