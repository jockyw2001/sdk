
#include "drv_mfe_dev.h"

static u32 _GetTime(void)
{
    CamOsTimespec_t tTime;

    CamOsGetMonotonicTime(&tTime);

    return ((tTime.nSec*1000000+ tTime.nNanoSec/1000)/1000);
}

int
MfeDevRegister(
    mmfe_dev*   mdev,
    mmfe_ctx*   mctx)
{
    int i = 0;
    CamOsMutexLock(&mdev->m_mutex);
    mctx->i_index = -1;

    while (i < MMFE_STREAM_NR)
    {
        if (mdev->m_regrqc[i++].p_mctx)
            continue;

        /* Link Device and Context */
        mdev->m_regrqc[--i].p_mctx = mctx;
        mctx->p_device = mdev;
        mctx->i_index = i;
        break;
    }
    CamOsMutexUnlock(&mdev->m_mutex);
    return mctx->i_index;
}

int
MfeDevUnregister(
    mmfe_dev*   mdev,
    mmfe_ctx*   mctx)
{
    int id = mctx->i_index;

    CamOsMutexLock(&mdev->m_mutex);

    /* UnLink Device and Context */
    mdev->m_regrqc[id].p_mctx = NULL;
    mctx->p_device = NULL;
    mctx->i_index = -1;

    CamOsMutexUnlock(&mdev->m_mutex);
    return 0;
}

int
MfeDevPowerOn(
    mmfe_dev*   mdev,
    int         rate)
{
    if (0 == rate)
        return 0;

    CamOsTsemDown(&mdev->tGetBitsSem);

    do
    {
        mdev->i_score += rate;
        if (mdev->i_users > 0)
            MfeDevClkOn(mdev, 0);

        if (rate > 0)
            mdev->i_users++;
        else if (--mdev->i_users == 0)
            break;

        if (mdev->i_clkidx < 0)
            mdev->i_ratehz = MfeDevClkRate(mdev, mdev->i_score*900);
        MfeDevClkOn(mdev, 1);
    }
    while (0);

    CamOsTsemUp(&mdev->tGetBitsSem);
    return 0;
}

int
MfeDevSuspend(
    mmfe_dev*   mdev)
{
    CamOsTsemDown(&mdev->tGetBitsSem);

    if (mdev->i_users > 0)
        MfeDevClkOn(mdev, 0);
    return 0;
}

int
MfeDevResume(
    mmfe_dev*   mdev)
{
    if (mdev->i_users > 0)
        MfeDevClkOn(mdev, 1);

    CamOsTsemUp(&mdev->tGetBitsSem);
    return 0;
}

int
MfeDevPushjob(
    mmfe_dev*   mdev,
    mmfe_ctx*   mctx)
{
    mhve_ops* mops = mctx->p_handle;
    mhve_ios* mios = mdev->p_asicip;
    mhve_job* mjob = mops->mhve_job(mops);
    int otmr, itmr, id = mctx->i_index;

    otmr = _GetTime();
    CamOsTsemDown(&mdev->tGetBitsSem);
    itmr = _GetTime();

    CamOsTsemInit(&mdev->m_wqh, 0);

    /* Trigger encode */
    mdev->i_state = MMFE_DEV_STATE_BUSY;
    mios->enc_fire(mios, mjob);

    /* Wait encode done */
    CamOsTsemDown(&mdev->m_wqh);
    mdev->i_state = MMFE_DEV_STATE_IDLE;

    itmr = _GetTime() - itmr;
    CamOsTsemUp(&mdev->tGetBitsSem);
    otmr = _GetTime() - otmr;

    /* Update time statistic data */
    if (otmr > mdev->i_thresh && mdev->i_thresh > 0)
        mdev->i_counts[id][0]++;
    if (otmr > mdev->i_counts[id][1])
        mdev->i_counts[id][1] = otmr;
    if (itmr > mdev->i_counts[id][2])
        mdev->i_counts[id][2] = itmr;
    if (mjob->i_tick > mdev->i_counts[id][3])
        mdev->i_counts[id][3] = mjob->i_tick;

#if MMFE_TIMER_SIZE>0
    id = mctx->i_numbr&((MMFE_TIMER_SIZE/8)-1);
    mdev->i_counts[mctx->i_index][4] -= mctx->p_timer[id].tm_cycles/(MMFE_TIMER_SIZE/8);
    mctx->p_timer[id].tm_dur[0] = (unsigned char)otmr;
    mctx->p_timer[id].tm_dur[1] = (unsigned char)itmr;
    mctx->p_timer[id].tm_cycles = (int)mjob->i_tick;
    mctx->i_numbr++;
    mdev->i_counts[mctx->i_index][4] += mctx->p_timer[id].tm_cycles/(MMFE_TIMER_SIZE/8);
#endif
    return 0;
}

int
MfeDevLockAndFire(
    mmfe_dev*   mdev,
    mmfe_ctx*   mctx)
{
    mhve_ops* mops = mctx->p_handle;
    mhve_ios* mios = mdev->p_asicip;
    mhve_job* mjob = mops->mhve_job(mops);

    CamOsTsemDown(&mdev->tGetBitsSem);

    /* Trigger encode */
    mdev->i_state = MMFE_DEV_STATE_BUSY;
    mios->enc_fire(mios, mjob);

    return 0;
}

int
MfeDevUnlockAfterEncDone(
    mmfe_dev*   mdev,
    mmfe_ctx*   mctx)
{
    if (mdev->i_state == MMFE_DEV_STATE_BUSY)
    {
        mdev->i_state = MMFE_DEV_STATE_IDLE;
        CamOsTsemUp(&mdev->tGetBitsSem);
    }

    return 0;
}

int
MfeDevGenEncCmd(
    mmfe_dev*   mdev,
    mmfe_ctx*   mctx,
    void* cmd_buf,
    int* cmd_len)
{
    mhve_ops* mops = mctx->p_handle;
    mhve_ios* mios = mdev->p_asicip;
    mhve_job* mjob = mops->mhve_job(mops);

    CamOsTsemDown(&mdev->tGetBitsSem);
    mdev->i_state = MMFE_DEV_STATE_BUSY;

    mios->gen_enc_cmd(mios, mjob, cmd_buf, cmd_len);

    mdev->i_state = MMFE_DEV_STATE_IDLE;
    CamOsTsemUp(&mdev->tGetBitsSem);
    return 0;
}

int
MfeDevIsrFnx(
    mmfe_dev*   mdev)
{
    mhve_ios* mios = mdev->p_asicip;

    // isr_func return 1 means encode not stop
    if (mios->isr_func(mios, 0) != 1)
        CamOsTsemUp(&mdev->m_wqh);
    else
        return 1;

    return 0;
}
