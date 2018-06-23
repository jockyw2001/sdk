
#include "mdrv_mhe_io.h"
#include "mdrv_rqct_io.h"

#include "drv_mhe_kernel.h"

long
MheRqcIoctls(
    void*           pctx,
    unsigned int    cmd,
    void*           arg)
{
    int err = -EINVAL;
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = NULL;
    CamOsMutexLock(&mctx->m_stream);
    if(NULL != (mops = mctx->p_handle))
    {
        rqct_ops* rqct = mops->rqct_ops(mops);
        switch(cmd)
        {
            case IOCTL_RQCT_S_CONF:
                if(!rqct->set_rqcf(rqct, (rqct_cfg*)arg))
                    err = 0;
                break;
            case IOCTL_RQCT_G_CONF:
                if(!rqct->get_rqcf(rqct, (rqct_cfg*)arg))
                    err = 0;
                break;
            default:
                break;
        }
    }
    CamOsMutexUnlock(&mctx->m_stream);
    return err;
}

size_t
MheRqcPrint(
    void*   pctx,
    char*   line,
    size_t  size)
{
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = NULL;
    rqct_ops* rqct = NULL;
    if(size > RQCT_LINE_SIZE)
        size = RQCT_LINE_SIZE;
    CamOsMutexLock(&mctx->m_stream);
    if(!(mops = mctx->p_handle))
        size = 0;
    else
    {
        rqct = mops->rqct_ops(mops);
        memset(line, 0, size);
        strncpy(line, rqct->print_line, size - 1);
        memset(rqct->print_line, 0, RQCT_LINE_SIZE);
    }
    CamOsMutexUnlock(&mctx->m_stream);
    return size;
}
