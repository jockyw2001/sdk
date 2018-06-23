
#include "drv_mfe_module.h"
#include "drv_mfe_kernel.h"
#include "drv_mfe_ctx.h"
#include "drv_mfe_dev.h"
#include "mhve_ios.h"
#include "hal_mfe_def.h"
#include "hal_mfe_api.h"

/*=============================================================*/
// Global varable
/*=============================================================*/
static mmfe_dev* _mdev_isr = NULL;

/*=============================================================*/
// Function Prototype
/*=============================================================*/
static void _MfeIsr()
{
    if(_mdev_isr)
    {
        MfeDevIsrFnx(_mdev_isr);
    }
}

mmfe_ctx*
MfeOpen(mmfe_dev* mdev, mmfe_ctx* mctx)
{
    mctx = MfeCtxAcquire(mdev);
    if(mctx == NULL)
    {
        MFE_MSG(MFE_MSG_ERR, "> Create Mctx Fail \n");
        return mctx;
    }
    if (0 <= MfeDevRegister(mdev, mctx))
    {
        return mctx;
    }

    MFE_MSG(MFE_MSG_ERR, "> MfeOpen Fail \n");
    mctx->release(mctx);
    return mctx;
}

int
MfeRelease(mmfe_dev* mdev, mmfe_ctx* mctx)
{
    MfeDevUnregister(mdev, mctx);

    if(mctx)
    {
        mctx->release(mctx);
    }
    return 0;
}

mmfe_dev*
MfeProbe(mmfe_dev* mdev)
{
    mhve_ios* mios = NULL;
    mhve_reg mregs;
    MsIntInitParam_u uInitParam;
    DrvMfeClk_t *clock;
    unsigned long res_base = IO_START_ADDRESS + MFE_OFFSET;
    int res_size = MFE_SIZE;

    if(_mdev_isr)
    {
        return _mdev_isr;
    }

    do
    {
        mdev = CamOsMemCalloc(1, sizeof(mmfe_dev));
        if(mdev == NULL)
        {
            MFE_MSG(MFE_MSG_ERR, "> Create Mdev Fail \n");
            break;
        }
        CamOsMutexInit(&mdev->m_mutex);
        CamOsTsemInit(&mdev->tGetBitsSem, 1);
        CamOsTsemInit(&mdev->m_wqh, MMFE_DEV_STATE_IDLE);

        //set IP base address
        mdev->p_asicip = MfeIosAcquire("mfe5");
        mios = mdev->p_asicip;
        mregs.i_id = 0;
        mregs.base = (unsigned long*)res_base;
        mregs.size = res_size;
        mios->set_bank(mios, &mregs);

        //attach interrupt
        uInitParam.intc.eMap         = INTC_MAP_IRQ;
        uInitParam.intc.ePriority    = INTC_PRIORITY_7;
        uInitParam.intc.pfnIsr       = _MfeIsr;
        MsInitInterrupt(&uInitParam, MS_INT_NUM_IRQ_MFE);
        MsUnmaskInterrupt(MS_INT_NUM_IRQ_MFE);

        //clock settings
        clock = CamOsMemCalloc(1, sizeof(clock));
        if(clock == NULL)
        {
            MFE_MSG(MFE_MSG_ERR, "> Create Clock Fail \n");
            break;
        }
        clock->eModule = CLKGEN_DRV_MFE;
        clock->eClkReq = CLKGEN_CLK_REQ_ON;
        clock->eClk = TOP_CLK_MFE;
        clock->eClkSpeed = CLKGEN_288MHZ;
        clock->nClkSrc = 0x0;

        mdev->p_clocks[0] = clock;
        mdev->i_clkidx = clock->nClkSrc;
        mdev->i_ratehz = clock->eClkSpeed;

        mdev->i_rctidx = 0;

        _mdev_isr = mdev;
        return mdev;
    }
    while (0);

    return _mdev_isr;
}

int
MfeRemove(mmfe_dev* mdev)
{
    mhve_ios* mios = NULL;

    if(mdev)
    {
        CamOsMutexDestroy(&mdev->m_mutex);
        CamOsTsemDeinit(&mdev->m_wqh);
        CamOsTsemDeinit(&mdev->tGetBitsSem);
        mios = mdev->p_asicip;
    }

    MsMaskInterrupt(MS_INT_NUM_IRQ_MFE);
    MsClearInterrupt(MS_INT_NUM_IRQ_MFE);

    if (mios)
    {
        mios->release(mios);
    }

    CamOsMemRelease(mdev);

    _mdev_isr = NULL;
    return 0;
}
