
#include "drv_mhe_dev.h"

#define MHE_CLKSPED_NUM 7
static const DrvMheClk_t _MheClkOptionTbl[] =
{
    {CLKGEN_DRV_MHE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MHE,     CLKGEN_288MHZ,    0x0},
    {CLKGEN_DRV_MHE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MHE,     CLKGEN_240MHZ,    0x1},
    {CLKGEN_DRV_MHE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MHE,     CLKGEN_192MHZ,    0x2},
    {CLKGEN_DRV_MHE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MHE,     CLKGEN_123P4MHZ,  0x3},
    {CLKGEN_DRV_MHE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MHE,     CLKGEN_384MHZ,    0x4},
    {CLKGEN_DRV_MHE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MHE,     CLKGEN_320MHZ,    0x5},
    {CLKGEN_DRV_MHE,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_MHE,     CLKGEN_172P8MHZ,  0x6},
    {CLKGEN_MOD_INVALID,    CLKGEN_CLK_REQ_INVALID,    TOP_CLK_INVALID,    CLKGEN_INVALID,   0xFFFF}
};

int MheDevClkRate(mmhe_dev* mdev, int rate)
{
    int i = 0, best = 0, r = 0;
    for(i = 0; i < MHE_CLKSPED_NUM; i++)
    {
        r = _MheClkOptionTbl[i].eClkSpeed;

        if(rate > best && best < r)
            best = r;
        if(rate < best && rate < r && r < best)
            best = r;
    }
    return best;
}

int MheDevClkOn(mmhe_dev* mdev, int on)
{
    int i = on != 0;
    DrvMheClk_t* clk;
    mhve_ios* mios = mdev->p_asicip;
    mios->irq_mask(mios, 0xFF);
    clk = mdev->p_clocks[0];
    if(i == 0)
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
    return 0;
}
