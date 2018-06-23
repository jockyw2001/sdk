
#include "drv_mhe_module.h"
#include "drv_mhe_kernel.h"
#include "drv_mhe_ctx.h"
#include "drv_mhe_dev.h"
#include "mhve_ios.h"
#include "hal_mhe_def.h"
#include "hal_mhe_api.h"

/*=============================================================*/
// Global varable
/*=============================================================*/
static mmhe_dev* _mdev_isr = NULL;

/*=============================================================*/
// Function Prototype
/*=============================================================*/
static void _MheIsr()
{
    if(_mdev_isr)
    {
        MheDevIsrFnx(_mdev_isr);
    }
}

mmhe_ctx*
MheOpen(mmhe_dev* mdev, mmhe_ctx* mctx)
{
    mctx = MheCtxAcquire(mdev);
    if(mctx == NULL)
    {
        MHE_MSG(MHE_MSG_ERR, "> Create Mctx Fail \n");
        return mctx;
    }
    if(0 <= MheDevRegister(mdev, mctx))
    {
        return mctx;
    }

    MHE_MSG(MHE_MSG_ERR, "> MheOpen Fail \n");
    mctx->release(mctx);
    return mctx;
}

int
MheRelease(mmhe_dev* mdev, mmhe_ctx* mctx)
{
    MheDevUnregister(mdev, mctx);

    if(mctx)
    {
        mctx->release(mctx);
    }
    return 0;
}

mhve_ios* MheIosAcquire(char* tags);

mmhe_dev*
MheProbe(mmhe_dev* mdev)
{
    mhve_ios* mios = NULL;
    mhve_reg mregs;
    MsIntInitParam_u uInitParam;
    DrvMheClk_t *clock;
    unsigned long res_base = IO_START_ADDRESS + MHE_OFFSET;
    int res_size = MHE_SIZE;

    if(_mdev_isr)
    {
        return _mdev_isr;
    }

    do
    {
        mdev = CamOsMemCalloc(1, sizeof(mmhe_dev));
        if(mdev == NULL)
        {
            MHE_MSG(MHE_MSG_ERR, "> Create Mdev Fail \n");
            break;
        }
        CamOsMutexInit(&mdev->m_mutex);
        CamOsTsemInit(&mdev->tGetBitsSem, 1);
        CamOsTsemInit(&mdev->m_wqh, MMHE_DEV_STATE_IDLE);

        //set IP base address
        mdev->p_asicip = MheIosAcquire("mhe");
        mios = mdev->p_asicip;
        mregs.i_id = 0;
        mregs.base = (unsigned long*)res_base;
        mregs.size = res_size;
        mios->set_bank(mios, &mregs);

        //attach interrupt
        uInitParam.intc.eMap         = INTC_MAP_IRQ;
        uInitParam.intc.ePriority    = INTC_PRIORITY_7;
        uInitParam.intc.pfnIsr       = _MheIsr;
        MsInitInterrupt(&uInitParam, MS_INT_NUM_IRQ_MHE);
        MsUnmaskInterrupt(MS_INT_NUM_IRQ_MHE);

        //clock settings
        clock = CamOsMemCalloc(1, sizeof(clock));
        if(clock == NULL)
        {
            MHE_MSG(MHE_MSG_ERR, "> Create Clock Fail \n");
            break;
        }
        clock->eModule = CLKGEN_DRV_MHE;
        clock->eClkReq = CLKGEN_CLK_REQ_ON;
        clock->eClk = TOP_CLK_MHE;
        clock->eClkSpeed = CLKGEN_288MHZ;
        clock->nClkSrc = 0x0;

        mdev->p_clocks[0] = clock;
        mdev->i_clkidx = clock->nClkSrc;
        mdev->i_ratehz = clock->eClkSpeed;

        mdev->i_rctidx = 0;

        _mdev_isr = mdev;
        return mdev;
    }
    while(0);

    return _mdev_isr;
}

int
MheRemove(mmhe_dev* mdev)
{
    mhve_ios* mios = NULL;

    if(mdev)
    {
        CamOsMutexDestroy(&mdev->m_mutex);
        CamOsTsemDeinit(&mdev->m_wqh);
        CamOsTsemDeinit(&mdev->tGetBitsSem);
        mios = mdev->p_asicip;
    }

    MsMaskInterrupt(MS_INT_NUM_IRQ_MHE);
    MsClearInterrupt(MS_INT_NUM_IRQ_MHE);

    if(mios)
    {
        mios->release(mios);
    }

    CamOsMemRelease(mdev);

    _mdev_isr = NULL;
    return 0;
}
