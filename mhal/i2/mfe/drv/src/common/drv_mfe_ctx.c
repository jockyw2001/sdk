#include "mdrv_mfe_io.h"
#include "mdrv_rqct_io.h"
#include "mdrv_pmbr_io.h"
#include "mhve_pmbr_ops.h"
#include "drv_mfe_kernel.h"
#include "drv_mfe_ctx.h"
#include "drv_mfe_dev.h"
#include "hal_mfe_def.h"
#include "hal_mfe_api.h"


#define MMFE_FLAGS_CONTROL  (MMFE_FLAGS_IDR|MMFE_FLAGS_DISPOSABLE|MMFE_FLAGS_NIGHT_MODE)

#ifdef USE_PHYSICAL_ADDR
#define ADDR_TYPE_SELECT(buf)       (CamOsDirectMemPhysToMiu((void*)(u32)buf))
#else
#define ADDR_TYPE_SELECT(buf)       (buf)
#endif

static int _MfeCtxStreamOn(void* pctx, int on);
static int _MfeCtxEncPict(void* pctx, mmfe_buff* buff);
static int _MfeCtxGetBits(void* pctx, mmfe_buff* buff);
static int _MfeCtxSetParm(void* pctx, mmfe_parm* parm);
static int _MfeCtxGetParm(void* pctx, mmfe_parm* parm);
static int _MfeCtxSetCtrl(void* pctx, mmfe_ctrl* ctrl);
static int _MfeCtxGetCtrl(void* pctx, mmfe_ctrl* ctrl);
static int _MfeCtxCompress(void* pctx, mmfe_buff* buff);
static int _MfeCtxPutData(void* pctx, mmfe_buff* buff);
static int _MfeCtxSetRqcf(void* pctx, rqct_conf* rqcf);
static int _MfeCtxGetRqcf(void* pctx, rqct_conf* rqcf);
static int _MfeCtxSetPmbr(void* pctx, pmbr_conf* pmbrcfg);
static int _MfeCtxGetPmbr(void* pctx, pmbr_conf* pmbrcfg);

static void _MfeCtxRelease(void* pctx)
{
    mmfe_ctx* mctx = pctx;

    if (CamOsAtomicDecAndTest(&mctx->i_refcnt))
    {
        mhve_ops* mops = mctx->p_handle;

        /* Release direct memory buffer */
        while (mctx->i_dmems > 0)
        {
            mctx->i_dmems--;
            CamOsDirectMemRelease(mctx->m_dmems[mctx->i_dmems], mctx->s_dmems[mctx->i_dmems]);
        }

        /* Release OPs and Context object */
        CamOsMutexLock(&mctx->m_stream);
        mctx->i_state = MMFE_CTX_STATE_NULL;
        mctx->p_handle = NULL;
        mops->release(mops);
        CamOsMutexUnlock(&mctx->m_stream);

        CamOsMutexDestroy(&mctx->m_stream);
#if MMFE_TIMER_SIZE>0
        CamOsMemRelease(mctx->p_timer);
#endif
        CamOsMemRelease(mctx->p_usrdt);
        CamOsMemRelease(mctx);
    }
}

static void _MfeCtxAdduser(void* pctx)
{
    mmfe_ctx* mctx = pctx;

    CamOsAtomicIncReturn(&mctx->i_refcnt);
}

mmfe_ctx*
MfeCtxAcquire(
    mmfe_dev*   mdev)
{
    mmfe_ctx* mctx = NULL;
    mhve_ops* mops = NULL;

    /* Allocate mmfe_ctx object */
    mctx = CamOsMemCalloc(1, sizeof(mmfe_ctx));
    if ((mdev == NULL) || (mctx == NULL))
    {
        MFE_MSG(MFE_MSG_ERR, "> Mctx Memory Alloc Fail \n");
        return NULL;
    }

    do
    {
        /* Link OPs function */
        mops = MfeOpsAcquire(mdev->i_rctidx);
        if (mops == NULL)
        {
            MFE_MSG(MFE_MSG_ERR, "> Create Mops Fail \n");
            return NULL;
        }

        /* Allocate user data buffer */
        mctx->p_usrdt = CamOsMemCalloc(1, MMFE_USER_DATA_SIZE);
        if (mctx->p_usrdt == NULL)
        {
            MFE_MSG(MFE_MSG_ERR, "> User Data Memory Alloc Fail \n");
            return NULL;
        }
#if MMFE_TIMER_SIZE>0
        mctx->p_timer = CamOsMemCalloc(1, MMFE_TIMER_SIZE);
        mctx->i_numbr = 0;
#endif

        mctx->i_enccnt = 0;

        /* Initialize OS signal */
        CamOsMutexInit(&mctx->m_stream);

        mctx->i_state = MMFE_CTX_STATE_NULL;
        mctx->release = _MfeCtxRelease;
        mctx->adduser = _MfeCtxAdduser;
        CamOsAtomicSet(&mctx->i_refcnt, 1);
        mctx->p_handle = mops;
        return mctx;
    }
    while (0);

    if (mops)
        mops->release(mops);

    CamOsMemRelease(mctx);
    return NULL;
}

long
MfeCtxActions(
    mmfe_ctx*       mctx,
    unsigned int    cmd,
    void*           arg)
{
    int err = 0;

    switch (cmd)
    {
        case IOCTL_MMFE_S_PARM:
            err = _MfeCtxSetParm(mctx, (mmfe_parm*)arg);
            break;
        case IOCTL_MMFE_G_PARM:
            err = _MfeCtxGetParm(mctx, (mmfe_parm*)arg);
            break;
        case IOCTL_MMFE_STREAMON:
            err = _MfeCtxStreamOn(mctx, 1);
            break;
        case IOCTL_MMFE_STREAMOFF:
            err = _MfeCtxStreamOn(mctx, 0);
            break;
        case IOCTL_MMFE_S_PICT:
            err = _MfeCtxEncPict(mctx, (mmfe_buff*)arg);
            break;
        case IOCTL_MMFE_G_BITS:
            err = _MfeCtxGetBits(mctx, (mmfe_buff*)arg);
            break;
        case IOCTL_MMFE_S_CTRL:
            err = _MfeCtxSetCtrl(mctx, (mmfe_ctrl*)arg);
            break;
        case IOCTL_MMFE_G_CTRL:
            err = _MfeCtxGetCtrl(mctx, (mmfe_ctrl*)arg);
            break;
        case IOCTL_MMFE_ENCODE:
            err = _MfeCtxCompress(mctx, (mmfe_buff*)arg);
            break;
        case IOCTL_MMFE_S_DATA:
            err = _MfeCtxPutData(mctx, (mmfe_buff*)arg);
            break;
        case IOCTL_RQCT_S_CONF:
            err = _MfeCtxSetRqcf(mctx, (rqct_conf*)arg);
            break;
        case IOCTL_RQCT_G_CONF:
            err = _MfeCtxGetRqcf(mctx, (rqct_conf*)arg);
            break;
        case IOCTL_PMBR_S_CONF:
            err = _MfeCtxSetPmbr(mctx, (pmbr_conf*)arg);
            break;
        case IOCTL_PMBR_G_CONF:
            err = _MfeCtxGetPmbr(mctx, (pmbr_conf*)arg);
            break;
        default:
            err = -EINVAL;
            break;
    }
    return (long)err;
}

static int
_MfeCtxStreamOn(
    void*   pctx,
    int     on)
{
    mmfe_ctx* mctx = pctx;
    mmfe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);
    rqct_cfg  rqcf;
    pmbr_ops* pmbr = mops->pmbr_ops(mops);
    pmbr_cfg  pmbrcfg;
    int i, err = 0;
    char szDmemName[20];

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if (on)
        {
            mhve_cfg mcfg;
            int size_out, size_mbs, size_mbp, size_dqm, size_refrec, size_zmv;
            int size_lum, size_chr, mbw, mbh, mbn;
            int size_pmbr, size_pmbr_map, size_pmbr_lut, size_dump_reg;
            int size_total_c, size_total_nc;
            int score, rpbn;
            void *pUserPtr = NULL;
            void *nMiuAddr = NULL;
            void *nPhysAddr = NULL;
            void *pUserPtr2 = NULL;
            void *nMiuAddr2 = NULL;
            void *nPhysAddr2 = NULL;

            if (MMFE_CTX_STATE_NULL != mctx->i_state)
                break;
            mctx->i_dmems = 0;

            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            mbw = _ALIGN_(4, mcfg.res.i_pixw)/16;
            mbh = _ALIGN_(4, mcfg.res.i_pixh)/16;
            mbn = mbw*mbh;

            /* Calculate required buffer size */
            size_mbp = _ALIGN_(8, mbw*MB_GN_DATA);
            //size_dqm = _ALIGN_(8, (mbn+1)/2+16);
            size_dqm = _ALIGN_(8, (mbn*2));
            //CamOsPrintf("size_dqm = %d, mbn = %d\n",size_dqm,mbn);

            size_mbs = size_mbp + size_dqm;

            size_dump_reg = 256;    // Fixme, 25x64bit on I2 MFE

            size_pmbr_map   = _ALIGN_(8, sizeof(int)*mbn);
            size_pmbr_lut   = _ALIGN_(8, 16 * PMBR_LUT_SIZE);
            size_pmbr       = size_pmbr_lut + (size_pmbr_map * 2);

            size_lum = _ALIGN_(8, mbn*MBPIXELS_Y);
            size_chr = _ALIGN_(8, mbn*MBPIXELS_C);
            size_out = _ALIGN_(12, size_lum);
            if (mctx->i_omode == MMFE_OMODE_MMAP)
                size_out = 0;

            rpbn = mcfg.res.i_rpbn;

            /* Calculate total size */
            size_refrec = 0;
            if (mctx->i_imode == MMFE_IMODE_PURE)
            {
                size_refrec =  _ALIGN_(12,size_lum+size_chr)*rpbn;
            }

            size_total_c = size_out + size_mbs + size_pmbr + size_dump_reg;
            size_total_nc = size_refrec;

            //CamOsPrintf("MFE: Total Internal Required Size (%d %d)\n", size_total_c, size_total_nc);

            if(mctx->i_ialsz == 0)
            {
                memset(szDmemName, 0, sizeof(szDmemName));
                CamOsSnprintf(szDmemName, sizeof(szDmemName), "%d:INTRCBUF", mctx->i_index);

                CamOsDirectMemAlloc(szDmemName, size_total_c, &pUserPtr, &nPhysAddr, &nMiuAddr);

                mctx->m_dmems[mctx->i_dmems] = (char*)pUserPtr;
                mctx->s_dmems[mctx->i_dmems] = size_mbs;

                if (mctx->m_dmems[mctx->i_dmems++] == NULL)
                {
                    //CamOsPrintf("[INTRCBUF:%d] CamOsDirectMemAlloc get addr:  pUserPtr 0x%08X  nMiuAddr 0x%08X  nPhysAddr 0x%08X\n",
                    //        __LINE__, (u32)pUserPtr, (u32)nMiuAddr, (u32)nPhysAddr);
                    break;
                }

                //CamOsPrintf("[INTRCBUF:%d] Size = %d, pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n", mctx->i_index, size_total_c, (u32) pUserPtr,(u32) nMiuAddr);
            }
            else
            {
                pUserPtr = mctx->p_ialva;
                nMiuAddr = mctx->p_ialma;
                //CamOsPrintf("[%s %d] pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n", __func__, __LINE__,(u32) pUserPtr,(u32) nMiuAddr);
            }

            if(mctx->i_ircms == 0)
            {
                memset(szDmemName, 0, sizeof(szDmemName));
                CamOsSnprintf(szDmemName, sizeof(szDmemName), "%d:INTRNCBUF", mctx->i_index);

                CamOsDirectMemAlloc(szDmemName, size_total_nc, &pUserPtr2, &nPhysAddr2, &nMiuAddr2);

                mctx->m_dmems[mctx->i_dmems] = (char*)pUserPtr2;
                mctx->s_dmems[mctx->i_dmems] = size_mbs;

                if (mctx->m_dmems[mctx->i_dmems++] == NULL)
                {
                    //CamOsPrintf("[INTRNCBUF:%d] CamOsDirectMemAlloc get addr:  pUserPtr2 0x%08X  nMiuAddr2 0x%08X  nPhysAddr2 0x%08X\n",
                    //      __LINE__, (u32)pUserPtr2, (u32)nMiuAddr2, (u32)nPhysAddr2);
                    break;
                }
                //CamOsPrintf("[INTRNCBUF:%d] Size = %d, pUserPtr2 = 0x%08X, nMiuAddr2 = 0x%08X\n", mctx->i_index, size_total_nc,(u32) pUserPtr2,(u32) nMiuAddr2);
            }
            else
            {
                nMiuAddr2 = mctx->p_ircma;
                //CamOsPrintf("[%s %d] nMiuAddr2 = 0x%08X\n", __func__, __LINE__,(u32) nMiuAddr2);
            }

            do
            {
                mctx->p_ovptr = NULL;
                mctx->u_ophys = 0;
                mctx->i_osize = mctx->i_ormdr = 0;

                /* Allocate output bitstream buffer if necessary */
                if (size_out > 0)
                {
                    mcfg.type = MHVE_CFG_DMA;
                    mcfg.dma.i_dmem = -1;
                    mcfg.dma.p_vptr = pUserPtr;
                    mcfg.dma.u_phys = (u32)nMiuAddr;
                    mcfg.dma.i_size[0] = size_out;
                    mops->set_conf(mops, &mcfg);

                    mctx->p_ovptr = (unsigned char*)pUserPtr;
                    mctx->u_ophys = (u32)nMiuAddr;
                    mctx->i_osize = size_out;
                }

                /* Allocate ROI DQ Map/GN Buffer */
                rqcf.type = RQCT_CFG_DQM;
                rqcf.dqm.u_phys = (u32)nMiuAddr;
                rqcf.dqm.p_kptr = pUserPtr;
                rqcf.dqm.i_dqmw = mbw;
                rqcf.dqm.i_dqmh = mbh;
                rqcf.dqm.i_size = size_dqm;
                rqct->set_rqcf(rqct, &rqcf);
                //CamOsPrintf("[%s %d] ROI DQ Map: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n", __func__, __LINE__,(u32) pUserPtr,(u32) nMiuAddr);
                nMiuAddr += size_dqm;
                pUserPtr += size_dqm;

                /* Allocate ROI Zero Motion Buffer */
                rqcf.type = RQCT_CFG_ZMV;
                rqcf.zmv.u_phys = (u32)nMiuAddr;
                rqcf.zmv.p_kptr = pUserPtr;
                rqcf.zmv.i_zmvw = mbw;
                rqcf.zmv.i_zmvh = mbh;
                rqcf.zmv.i_size = size_dqm;
                rqct->set_rqcf(rqct, &rqcf);
                CamOsPrintf("[%s %d] ROI Zero Motion: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n", __func__, __LINE__,(u32) pUserPtr,(u32) nMiuAddr);
                nMiuAddr += size_zmv;
                pUserPtr += size_zmv;

                mcfg.type = MHVE_CFG_DMA;
                mcfg.dma.i_dmem = -2;
                mcfg.dma.u_phys = (u32)nMiuAddr;
                mcfg.dma.p_vptr = NULL;
                mcfg.dma.i_size[0] = size_mbp;
                mops->set_conf(mops, &mcfg);
                //CamOsPrintf("[%s %d] GN Buffer: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n", __func__, __LINE__,(u32) pUserPtr,(u32) nMiuAddr);
                nMiuAddr += size_mbp;
                pUserPtr += size_mbp;

                /* Allocate Dump Register Buffer */
                mcfg.type = MHVE_CFG_DUMP_REG;
                mcfg.dump_reg.u_phys = nMiuAddr;
                mcfg.dump_reg.p_vptr = pUserPtr;
                mcfg.dump_reg.i_size = size_dump_reg;
                mops->set_conf(mops, &mcfg);
                //CamOsPrintf("[%s %d] DUMP REG: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n", __func__, __LINE__,(u32) pUserPtr,(u32) nMiuAddr);
                nMiuAddr += size_dump_reg;
                pUserPtr += size_dump_reg;

                /* Allocate PMBR Buffer */
                pmbrcfg.type = PMBR_CFG_MEM;
                pmbrcfg.mem.u_phys[PMBR_MEM_MB_MAP_IN] = (u32)nMiuAddr;
                pmbrcfg.mem.p_kptr[PMBR_MEM_MB_MAP_IN] = pUserPtr;
                pmbrcfg.mem.i_size[PMBR_MEM_MB_MAP_IN] = size_pmbr_map;
                //CamOsPrintf("[%s %d] PMBR IN: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n", __func__, __LINE__,(u32) pUserPtr,(u32) nMiuAddr);
                nMiuAddr += size_pmbr_map;
                pUserPtr += size_pmbr_map;

                pmbrcfg.mem.u_phys[PMBR_MEM_MB_MAP_OUT] = (u32)nMiuAddr;
                pmbrcfg.mem.p_kptr[PMBR_MEM_MB_MAP_OUT] = pUserPtr;
                pmbrcfg.mem.i_size[PMBR_MEM_MB_MAP_OUT] = size_pmbr_map;
                //CamOsPrintf("[%s %d] PMBR OUT: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n", __func__, __LINE__,(u32) pUserPtr,(u32) nMiuAddr);
                nMiuAddr += size_pmbr_map;
                pUserPtr += size_pmbr_map;

                pmbrcfg.mem.u_phys[PMBR_MEM_LUT] = (u32)nMiuAddr;
                pmbrcfg.mem.p_kptr[PMBR_MEM_LUT] = pUserPtr;
                pmbrcfg.mem.i_size[PMBR_MEM_LUT] = size_pmbr_lut;
                pmbr->set_conf(pmbr, &pmbrcfg);
                //CamOsPrintf("[%s %d] PMBR LUT: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n", __func__, __LINE__,(u32) pUserPtr,(u32) nMiuAddr);
                nMiuAddr+=size_pmbr_lut;
                pUserPtr+=size_pmbr_lut;

                /* Allocate Reference/Reconstructed Picture Buffers */
                if (mctx->i_imode == MMFE_IMODE_PURE)
                {
                    for (i = 0; i < rpbn; i++)
                    {
                        mcfg.type = MHVE_CFG_DMA;
                        mcfg.dma.i_dmem = i;
                        mcfg.dma.u_phys = (u32)nMiuAddr2;
                        mcfg.dma.p_vptr = NULL;
                        mcfg.dma.i_size[0] = size_lum;
                        mcfg.dma.i_size[1] = size_chr;
                        mops->set_conf(mops, &mcfg);
                        //CamOsPrintf("[%s %d] (%d:REF&REC %d): nMiuAddr = 0x%08X\n", __func__, __LINE__,mctx->i_index, i,(u32) nMiuAddr2);
                        nMiuAddr2+=_ALIGN_(12,size_lum+size_chr);
                    }
                }
                else // (mctx->i_imode == MMFE_IMODE_PLUS) // EROY CHECK
                {
                    for (i = 0; i < rpbn; i++)
                    {
                        mcfg.type = MHVE_CFG_DMA;
                        mcfg.dma.i_dmem = i;
                        mcfg.dma.u_phys = 0;
                        mcfg.dma.p_vptr = NULL;
                        mcfg.dma.i_size[0] = 0;
                        mcfg.dma.i_size[1] = 0;
                        mops->set_conf(mops, &mcfg);
                    }
                }
            }
            while (0);

            rqcf.type = RQCT_CFG_FPS;
            rqct->get_rqcf(rqct, &rqcf);

            score = mbn;
            score *= (int)rqcf.fps.n_fps;
            score /= (int)rqcf.fps.d_fps;
            mctx->i_score = score;

            /* Start Streaming */
            if (!(err = mops->seq_sync(mops)))
            {
                mops->seq_conf(mops);

                /* Reset Statistic Data and State */
                MfeDevPowerOn(mdev, mctx->i_score);
                mdev->i_counts[mctx->i_index][0] = mdev->i_counts[mctx->i_index][1] = 0;
                mdev->i_counts[mctx->i_index][2] = mdev->i_counts[mctx->i_index][3] = 0;
                mdev->i_counts[mctx->i_index][4] = 0;
                mctx->i_state = MMFE_CTX_STATE_IDLE;
                break;
            }
        }
        else
        {
            if (MMFE_CTX_STATE_NULL == mctx->i_state)
                break;

            MfeDevPowerOn(mdev, -mctx->i_score);
            mctx->i_state = MMFE_CTX_STATE_NULL;
            mctx->i_score = 0;

            /* Stop Streaming */
            mops->seq_done(mops);
#if 0
            CamOsPrintf("<%d>mfe performance:\n",mctx->i_index);
            for (i = 0; i < MMFE_TIMER_SIZE/8; i++)
                CamOsPrintf("<%d>%4d/%4d/%8d\n",mctx->i_index,mctx->p_timer[i].tm_dur[0],mctx->p_timer[i].tm_dur[1],mctx->p_timer[i].tm_cycles);
#endif
        }
    }
    while (0);

    if (!on || err)
    {
        while (mctx->i_dmems > 0) {
            mctx->i_dmems--;
            CamOsDirectMemRelease(mctx->m_dmems[mctx->i_dmems], mctx->s_dmems[mctx->i_dmems]);
        }
    }

    mops->backup(mops);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxEncPict(
    void*           pctx,
    mmfe_buff*      buff)
{
    int err = -EINVAL;
    int pitch = 0;
    mmfe_ctx* mctx = pctx;
    mmfe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    mhve_cpb* pcpb = mctx->m_mcpbs;

    if (buff->i_memory != MMFE_MEMORY_MMAP)
        return -1;

    pitch = buff->i_stride;
    if (pitch < buff->i_width)
        pitch = buff->i_width;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if (MMFE_CTX_STATE_IDLE == mctx->i_state && MMFE_OMODE_USER == mctx->i_omode)
        {
            mhve_vpb mvpb;
            mhve_cfg mcfg;

            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);

            if (buff->i_planes != 2 && (MHVE_PIX_NV21 >= mcfg.res.e_pixf))
                break;
            if (buff->i_planes != 1 && (MHVE_PIX_YUYV <= mcfg.res.e_pixf))
                break;
            if (mcfg.res.i_pixw != buff->i_width || mcfg.res.i_pixh != buff->i_height)
                break;

            mvpb.i_index = buff->i_index;
            mvpb.i_stamp = buff->i_timecode;
            mvpb.u_flags = buff->i_flags & MMFE_FLAGS_CONTROL;
            mvpb.i_pitch = pitch;
            mvpb.planes[1].u_phys = 0;
            mvpb.planes[1].i_bias = 0;
            mvpb.planes[0].u_phys = (uint)ADDR_TYPE_SELECT(buff->planes[0].mem.phys);
            mvpb.planes[0].i_bias = buff->planes[0].i_bias;
            if (mcfg.res.e_pixf <= MHVE_PIX_NV21)
            {
                mvpb.planes[1].u_phys = (uint)ADDR_TYPE_SELECT(buff->planes[1].mem.phys);
                mvpb.planes[1].i_bias = buff->planes[1].i_bias;
            }

            mctx->i_state = MMFE_CTX_STATE_BUSY;

            if (!(err = mops->enc_buff(mops, &mvpb)))
            {
                do
                {
                    mops->put_data(mops, mctx->p_usrdt, mctx->i_usrsz);
                    mops->enc_conf(mops);
                    MfeDevPushjob(mdev, mctx);
                }
                while (0 < (err = mops->enc_done(mops)));
            }

            if (!err)
            {
                mctx->i_usrcn = mctx->i_usrsz = 0;
                pcpb->i_index = -1;
            }

            mctx->i_ormdr =
            err =
            mops->out_buff(mops, mctx->m_mcpbs);
        }
    }
    while (0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxGetBits(
    void*           pctx,
    mmfe_buff*      buff)
{
    mmfe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    int err = -ENODATA;

    if (buff->i_memory != MMFE_MEMORY_USER || buff->i_planes != 1)
        return -1;

    CamOsMutexLock(&mctx->m_stream);

    if (MMFE_CTX_STATE_BUSY == mctx->i_state && MMFE_OMODE_USER == mctx->i_omode)
    {
        int flags = 0;
        int rmdr = mctx->i_ormdr;
        int size = mctx->m_mcpbs->planes[0].i_size;
        unsigned char* ptr = mctx->p_ovptr + size - rmdr; // EROY CHECK

        if (rmdr == size)
            flags = MMFE_FLAGS_SOP;
        if (buff->planes[0].i_size < rmdr)
            rmdr = buff->planes[0].i_size;

        buff->i_flags = 0;

        if (CamOsCopyToUpperLayer(buff->planes[0].mem.uptr, ptr, rmdr))
        {
            err = -EFAULT;
        }
        else
        {
            buff->planes[0].i_used = rmdr;
            buff->i_timecode = mctx->m_mcpbs->i_stamp;
            mctx->i_ormdr -= rmdr;
            err = 0;
        }

        if (0 == mctx->i_ormdr)
        {
            mctx->i_state = MMFE_CTX_STATE_IDLE;
            flags |= MMFE_FLAGS_EOP;
            mops->out_buff(mops, mctx->m_mcpbs); // EROY CHECK
        }

        if (!err)
            buff->i_flags = flags | (mctx->m_mcpbs->i_flags&MMFE_FLAGS_CONTROL);
    }

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxSetParm(
    void*           pctx,
    mmfe_parm*      parm)
{
    mmfe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    int err = -EINVAL;

    CamOsMutexLock(&mctx->m_stream);

    /* Set below Parameters Before Streaming On (Static) */
    if (mctx->i_state == MMFE_CTX_STATE_NULL)
    {
        rqct_ops* rqct = mops->rqct_ops(mops);
        rqct_cfg  rqcf;
        pmbr_ops* pmbr = mops->pmbr_ops(mops);
        pmbr_cfg  pmbrcfg;
        mhve_cfg  mcfg;
        unsigned char b_long_term_reference;

        switch (parm->type)
        {
            case MMFE_PARM_RES:
                /* check LTR mode */
                mcfg.type = MHVE_CFG_LTR;
                err = mops->get_conf(mops, &mcfg);
                b_long_term_reference = mcfg.ltr.b_long_term_reference;

                if ((unsigned)parm->res.i_pixfmt > MMFE_PIXFMT_YUYV)
                    break;
                if ((parm->res.i_pict_w % 16) || (parm->res.i_pict_h % 2))
                    break;

                mctx->i_picture_w = parm->res.i_pict_w;
                mctx->i_picture_h = parm->res.i_pict_h;
                mctx->i_cropw = parm->res.i_crop_w;     // encoded image
                mctx->i_croph = parm->res.i_crop_h;
                mctx->i_cropoffsetx = parm->res.i_crop_offset_x;  // offset
                mctx->i_cropoffsety = parm->res.i_crop_offset_y;

                mcfg.type = MHVE_CFG_RES;
                mcfg.res.e_pixf = parm->res.i_pixfmt;
                mcfg.res.i_pixw = parm->res.i_crop_w;
                mcfg.res.i_pixh = parm->res.i_crop_h;
                /* recn buffer number depend on LTR mode */
                if (b_long_term_reference)
                    mcfg.res.i_rpbn = 3;
                else
                    mcfg.res.i_rpbn = 2;
                mcfg.res.u_conf = 0;
                mcfg.res.i_ClkEn = parm->res.i_ClkEn;
                mcfg.res.i_ClkSor = parm->res.i_ClkSor;
                mcfg.res.i_NotifySize = parm->res.i_NotifySize;
                mcfg.res.notifyFunc = parm->res.notifyFunc;

                // EROY CHECK
                mctx->i_omode = MMFE_OMODE_USER;
                mctx->i_imode = MMFE_IMODE_PURE; // RTK-I3E Setting

                if (parm->res.i_outlen < 0)
                    mctx->i_omode = MMFE_OMODE_MMAP; // RTK-I3E Setting
                if (parm->res.i_outlen < -1)
                    mctx->i_imode = MMFE_IMODE_PLUS;

                mops->set_conf(mops, &mcfg);

                rqcf.type = RQCT_CFG_RES;
                rqcf.res.i_picw = _ALIGN_(4, parm->res.i_pict_w);
                rqcf.res.i_pich = _ALIGN_(4, parm->res.i_pict_h);
                rqct->set_rqcf(rqct, &rqcf);

                pmbrcfg.type = PMBR_CFG_SEQ;
                pmbrcfg.seq.i_enable = 1;
                pmbrcfg.seq.i_mbw = _ALIGN_(4, mcfg.res.i_pixw)>>4;
                pmbrcfg.seq.i_mbh = _ALIGN_(4, mcfg.res.i_pixh)>>4;
                pmbr->set_conf(pmbr, &pmbrcfg);

                err = 0;
                break;
            case MMFE_PARM_FPS:
                if (parm->fps.i_num > 0 && parm->fps.i_den > 0)
                {
                    rqcf.type = RQCT_CFG_FPS;
                    rqcf.fps.n_fps = parm->fps.i_num;
                    rqcf.fps.d_fps = parm->fps.i_den;
                    rqct->set_rqcf(rqct, &rqcf);

                    //initial ROI background frame rate
                    rqcf.type = RQCT_CFG_BGF;
                    rqcf.bgf.i_bgsrcfps = (parm->fps.i_num + (parm->fps.i_den >> 1)) / parm->fps.i_den;
                    rqcf.bgf.i_bgdstfps = rqcf.bgf.i_bgsrcfps;
                    rqct->set_rqcf(rqct, &rqcf);

                    err = 0;
                }
                break;
            case MMFE_PARM_MOT:
                mcfg.type = MHVE_CFG_MOT;
                mcfg.mot.i_subp = parm->mot.i_subpel;
                mcfg.mot.i_dmvx = _MIN(_MAX(8,parm->mot.i_dmv_x),32);
                mcfg.mot.i_dmvy = parm->mot.i_dmv_y<=8?8:16;
                mcfg.mot.i_blkp[0] = parm->mot.i_mvblks[0];
                mcfg.mot.i_blkp[1] = 0;
                err = mops->set_conf(mops, &mcfg);
                break;
            case MMFE_PARM_BPS:
                if ((unsigned)parm->bps.i_method > RQCT_METHOD_VBR)
                    break;
                rqcf.type = RQCT_CONF_SEQ;
                if (!rqct->get_rqcf(rqct, &rqcf))
                {
                    rqcf.seq.i_method = parm->bps.i_method;
                    rqcf.seq.i_btrate = parm->bps.i_bps;
                    rqcf.seq.i_leadqp = parm->bps.i_ref_qp;
                    err = rqct->set_rqcf(rqct, &rqcf);
                }
                rqcf.type = RQCT_CFG_DQP;
                if (!rqct->get_rqcf(rqct, &rqcf))
                {
                    rqcf.dqp.i_dqp = parm->bps.i_delta_qp;
                    err = rqct->set_rqcf(rqct, &rqcf);
                }
                break;
            case MMFE_PARM_GOP:
                if (parm->gop.i_pframes < 0)
                    break;

                rqcf.type = RQCT_CONF_SEQ;
                if (!rqct->get_rqcf(rqct, &rqcf))
                {
                    rqcf.seq.i_period = parm->gop.i_pframes+1;
                    rqcf.seq.b_passiveI = parm->gop.b_passiveI;
                    if (!rqct->set_rqcf(rqct, &rqcf))
                        err = 0;
                }
                break;
            case MMFE_PARM_AVC:
                mcfg.type = MHVE_CFG_AVC;
                mcfg.avc.i_profile = parm->avc.i_profile;
                mcfg.avc.i_level = parm->avc.i_level;
                mcfg.avc.i_num_ref_frames = parm->avc.i_num_ref_frames;
                mcfg.avc.i_poc_type = parm->avc.i_poc_type;
                mcfg.avc.b_entropy_coding_type_i = parm->avc.b_cabac_i;
                mcfg.avc.b_entropy_coding_type_p = parm->avc.b_cabac_p;
                mcfg.avc.b_constrained_intra_pred = parm->avc.b_constrained_intra_pred;
                mcfg.avc.b_deblock_filter_control = parm->avc.b_deblock_filter_control;
                mcfg.avc.i_disable_deblocking_idc = parm->avc.i_disable_deblocking_idc;
                mcfg.avc.i_alpha_c0_offset = parm->avc.i_alpha_c0_offset;
                mcfg.avc.i_beta_offset = parm->avc.i_beta_offset;
                mcfg.avc.i_chroma_qp_index_offset = parm->avc.i_chroma_qp_index_offset;
                err = mops->set_conf(mops, &mcfg);
                break;
            case MMFE_PARM_VUI:
                mcfg.type = MHVE_CFG_VUI;
                mcfg.vui.b_aspect_ratio_info_present_flag = parm->vui.b_aspect_ratio_info_present_flag;
                mcfg.vui.i_aspect_ratio_idc = parm->vui.i_aspect_ratio_idc;
                mcfg.vui.i_sar_w = parm->vui.i_sar_w;
                mcfg.vui.i_sar_h = parm->vui.i_sar_h;
                mcfg.vui.b_overscan_info_present_flag = parm->vui.b_overscan_info_present_flag;
                mcfg.vui.b_overscan_appropriate_flag = parm->vui.b_overscan_appropriate_flag;
                mcfg.vui.b_video_full_range = parm->vui.b_video_full_range;
                mcfg.vui.b_video_signal_pres = parm->vui.b_video_signal_pres;
                mcfg.vui.i_video_format = parm->vui.i_video_format;
                mcfg.vui.b_colour_desc_pres = parm->vui.b_colour_desc_pres;
                mcfg.vui.i_colour_primaries = parm->vui.i_colour_primaries;
                mcfg.vui.i_transf_character = parm->vui.i_transf_character;
                mcfg.vui.i_matrix_coeffs = parm->vui.i_matrix_coeffs;
                mcfg.vui.b_timing_info_pres = parm->vui.b_timing_info_pres;
                mcfg.vui.i_num_units_in_tick = parm->vui.i_num_units_in_tick;
                mcfg.vui.i_time_scale = parm->vui.i_time_scale;
                mcfg.vui.b_fixed_frame_rate_flag = parm->vui.b_fixed_frame_rate_flag;
                err = mops->set_conf(mops, &mcfg);
                break;
            case MMFE_PARM_LTR:
                mcfg.type = MHVE_CFG_LTR;
                mcfg.ltr.b_long_term_reference = parm->ltr.b_long_term_reference;
                mcfg.ltr.b_enable_pred = parm->ltr.b_enable_pred;
                err = mops->set_conf(mops, &mcfg);
                b_long_term_reference = mcfg.ltr.b_long_term_reference;
                /* Set recn buffer depend on LTR mode */
                mcfg.type = MHVE_CFG_RES;
                if (!mops->get_conf(mops, &mcfg))
                {
                    mcfg.res.i_rpbn = b_long_term_reference ? 3 : 2;
                    err = mops->set_conf(mops, &mcfg);
                }
                /* RQCT control LTR P-frame period */
                rqcf.type = RQCT_CFG_LTR;
                if (!rqct->get_rqcf(rqct, &rqcf))
                {
                    rqcf.ltr.i_period = b_long_term_reference ? parm->ltr.i_ltr_period : 0;
                    err = rqct->set_rqcf(rqct, &rqcf);
                }
                break;
            default:
                CamOsPrintf("unsupported config\n");
                break;
        }
    }
    else
    {
        CamOsPrintf("MFE ERR: Set MMFE_PARM(%d) in wrong state\n", parm->type);
    }

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxGetParm(
    void*           pctx,
    mmfe_parm*      parm)
{
    int err = 0;
    mmfe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);
    mhve_cfg  mcfg;
    rqct_cfg  rqcf;

    CamOsMutexLock(&mctx->m_stream);

    switch (parm->type)
    {
        case MMFE_PARM_IDX:
            parm->idx.i_stream = mctx->i_index;
            break;
        case MMFE_PARM_RES:
            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            parm->res.i_pict_w = mctx->i_picture_w;
            parm->res.i_pict_h = mctx->i_picture_h;
            parm->res.i_crop_w = mctx->i_cropw;
            parm->res.i_crop_h = mctx->i_croph;
            parm->res.i_crop_offset_x = mctx->i_cropoffsetx;
            parm->res.i_crop_offset_y = mctx->i_cropoffsety;
            parm->res.i_pixfmt = mcfg.res.e_pixf;
            //parm->res.i_outlen = 0;
            if (mctx->i_imode == MMFE_IMODE_PLUS)
                parm->res.i_outlen = -2;
            else if (mctx->i_imode == MMFE_OMODE_MMAP)
                parm->res.i_outlen = -1;
            else
                parm->res.i_outlen = 0;

            parm->res.i_flags = 0;
            parm->res.i_NotifySize = mcfg.res.i_NotifySize;
            parm->res.notifyFunc = mcfg.res.notifyFunc;
            break;
        case MMFE_PARM_FPS:
            rqcf.type = RQCT_CFG_FPS;
            if (!(err = rqct->get_rqcf(rqct, &rqcf)))
            {
                parm->fps.i_num = (int)rqcf.fps.n_fps;
                parm->fps.i_den = (int)rqcf.fps.d_fps;
            }
            break;
        case MMFE_PARM_MOT:
            mcfg.type = MHVE_CFG_MOT;
            mops->get_conf(mops, &mcfg);
            parm->mot.i_dmv_x = mcfg.mot.i_dmvx;
            parm->mot.i_dmv_y = mcfg.mot.i_dmvy;
            parm->mot.i_subpel= mcfg.mot.i_subp;
            parm->mot.i_mvblks[0] = mcfg.mot.i_blkp[0];
            parm->mot.i_mvblks[1] = 0;
            break;
        case MMFE_PARM_BPS:
        case MMFE_PARM_GOP:
            rqcf.type = RQCT_CONF_SEQ;
            if (!rqct->get_rqcf(rqct, &rqcf))
            {
                if (parm->type == MMFE_PARM_BPS)
                {
                    parm->bps.i_method = rqcf.seq.i_method;
                    parm->bps.i_ref_qp = rqcf.seq.i_leadqp;
                    parm->bps.i_bps = rqcf.seq.i_btrate;

                    rqcf.type = RQCT_CFG_DQP;
                    if (!rqct->get_rqcf(rqct, &rqcf))
                    {
                        parm->bps.i_delta_qp = rqcf.dqp.i_dqp;
                    }
                }
                else
                {
                    parm->gop.i_pframes = rqcf.seq.i_period-1;
                    parm->gop.i_bframes = 0;
                    parm->gop.b_passiveI = rqcf.seq.b_passiveI;
                }
            }
            break;
        case MMFE_PARM_AVC:
            mcfg.type = MHVE_CFG_AVC;
            if (!(err = mops->get_conf(mops, &mcfg)))
            {
                parm->avc.i_profile = mcfg.avc.i_profile;
                parm->avc.i_level = mcfg.avc.i_level;
                parm->avc.i_num_ref_frames = mcfg.avc.i_num_ref_frames;
                parm->avc.i_poc_type = mcfg.avc.i_poc_type;
                parm->avc.b_cabac_i = mcfg.avc.b_entropy_coding_type_i;
                parm->avc.b_cabac_p = mcfg.avc.b_entropy_coding_type_p;
                parm->avc.b_deblock_filter_control = mcfg.avc.b_deblock_filter_control;
                parm->avc.b_constrained_intra_pred = mcfg.avc.b_constrained_intra_pred;
                parm->avc.i_disable_deblocking_idc = mcfg.avc.i_disable_deblocking_idc;
                parm->avc.i_alpha_c0_offset = mcfg.avc.i_alpha_c0_offset;
                parm->avc.i_beta_offset = mcfg.avc.i_beta_offset;
                parm->avc.i_chroma_qp_index_offset = mcfg.avc.i_chroma_qp_index_offset;
            }
            break;
        case MMFE_PARM_VUI:
            mcfg.type = MHVE_CFG_VUI;
            if (!(err = mops->get_conf(mops, &mcfg)))
            {
                parm->vui.b_aspect_ratio_info_present_flag = mcfg.vui.b_aspect_ratio_info_present_flag;
                parm->vui.i_aspect_ratio_idc = mcfg.vui.i_aspect_ratio_idc;
                parm->vui.i_sar_w = mcfg.vui.i_sar_w;
                parm->vui.i_sar_h = mcfg.vui.i_sar_h;
                parm->vui.b_overscan_info_present_flag = mcfg.vui.b_overscan_info_present_flag;
                parm->vui.b_overscan_appropriate_flag = mcfg.vui.b_overscan_appropriate_flag;
                parm->vui.b_video_full_range = mcfg.vui.b_video_full_range;
                parm->vui.b_video_signal_pres = mcfg.vui.b_video_signal_pres;
                parm->vui.i_video_format = mcfg.vui.i_video_format;
                parm->vui.b_colour_desc_pres = mcfg.vui.b_colour_desc_pres;
                parm->vui.i_colour_primaries = mcfg.vui.i_colour_primaries;
                parm->vui.i_transf_character = mcfg.vui.i_transf_character;
                parm->vui.i_matrix_coeffs = mcfg.vui.i_matrix_coeffs;
                parm->vui.b_timing_info_pres = mcfg.vui.b_timing_info_pres;
                parm->vui.i_num_units_in_tick = mcfg.vui.i_num_units_in_tick;
                parm->vui.i_time_scale = mcfg.vui.i_time_scale;
                parm->vui.b_fixed_frame_rate_flag = mcfg.vui.b_fixed_frame_rate_flag;
            }
            break;
        case MMFE_PARM_LTR:
            mcfg.type = MHVE_CFG_LTR;
            if (!(err = mops->get_conf(mops, &mcfg)))
            {
                parm->ltr.b_long_term_reference = mcfg.ltr.b_long_term_reference;
                if (mcfg.ltr.b_long_term_reference)
                {
                    rqcf.type = RQCT_CFG_LTR;
                    if (!rqct->get_rqcf(rqct, &rqcf))
                    {
                        parm->ltr.i_ltr_period = rqcf.ltr.i_period;
                    }
                    parm->ltr.b_enable_pred = mcfg.ltr.b_enable_pred;
                }
                else
                {
                    parm->ltr.i_ltr_period = 0;
                    parm->ltr.b_enable_pred = 0;
                }
            }
            break;
        default:
            CamOsPrintf("unsupported config\n");
            break;
    }

    CamOsMutexUnlock(&mctx->m_stream);
    return err;
}

static int
_MfeCtxSetCtrl(
    void*           pctx,
    mmfe_ctrl*      ctrl)
{
    int err = -EINVAL;
    mmfe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);
    rqct_cfg rqcf;
    mhve_cfg mcfg;
    int i;

    CamOsMutexLock(&mctx->m_stream);

    /* Set Below Parameters After Streaming On (Dynamically) */
    switch (ctrl->type)
    {
        case MMFE_CTRL_ROI:
            if (mctx->i_state > MMFE_CTX_STATE_NULL)
            {
                if(ctrl->roi.i_index >= RQCT_ROI_NR || ctrl->roi.i_index < -1)
                    break;

                rqcf.type = RQCT_CFG_ROI;
                rqcf.roi.i_roiidx = ctrl->roi.i_index;

                if(ctrl->roi.i_roiqp < -51 || ctrl->roi.i_roiqp > 51 )
                {
                    rqcf.roi.i_roiqp = 0;
                }
                rqcf.roi.i_absqp = ctrl->roi.i_absqp;
                rqcf.roi.i_roiqp = ctrl->roi.i_roiqp;
                rqcf.roi.i_posx = ctrl->roi.i_mbx;
                rqcf.roi.i_posy = ctrl->roi.i_mby;
                rqcf.roi.i_recw = ctrl->roi.i_mbw;
                rqcf.roi.i_rech = ctrl->roi.i_mbh;
                rqct->set_rqcf(rqct, &rqcf);

                err = 0;
            }
            else
            {
                CamOsPrintf("MFE ERR: Set MMFE_CTRL_ROI in wrong state\n");
            }
            break;
        case MMFE_CTRL_BGFPS:
            if(mctx->i_state > MMFE_CTX_STATE_NULL)
            {
                rqcf.type = RQCT_CFG_BGF;
                if(!(err = rqct->get_rqcf(rqct, &rqcf)))
                {
                    rqcf.bgf.i_bgsrcfps = ctrl->bgfps.i_bgsrcfps;
                    rqcf.bgf.i_bgdstfps = ctrl->bgfps.i_bgdstfps;

                    rqct->set_rqcf(rqct, &rqcf);

                    err = 0;
                }
            }
            else
            {
                CamOsPrintf("MFE ERR: Set MMFE_CTRL_BGFPS in wrong state\n");
            }
            break;
        case MMFE_CTRL_QPMAP:
            if(mctx->i_state > MMFE_CTX_STATE_NULL)
            {
                if(ctrl->qpmap.p_mapkptr == NULL && ctrl->qpmap.i_enb)
                    break;
                for(i = 0; i < 14; i++)
                {
                    if(ctrl->qpmap.i_entry[i] < -51 || ctrl->qpmap.i_entry[i] > 51)
                        break;
                }

                rqcf.type = RQCT_CFG_QPM;
                rqcf.qpm.i_qpmenb = ctrl->qpmap.i_enb;
                for(i = 0; i < 14; i++)
                     rqcf.qpm.i_entry[i] = ctrl->qpmap.i_entry[i];
                rqcf.qpm.p_kptr = (void *)ctrl->qpmap.p_mapkptr;

                rqct->set_rqcf(rqct, &rqcf);

                err = 0;
            }
            else
            {
                CamOsPrintf("MFE ERR: Set MMFE_CTRL_QPMAP in wrong state\n");
            }
            break;
        case MMFE_CTRL_SPL:
            if (mctx->i_state >= MMFE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_SPL;
                mcfg.spl.i_rows = ctrl->spl.i_rows;
                mcfg.spl.i_bits = ctrl->spl.i_bits;
                if((err =  mops->set_conf(mops, &mcfg)))
                    break;
            }
            else
            {
                CamOsPrintf("MFE ERR: Set MMFE_CTRL_SPL in wrong state\n");
            }
            err = 0;
            break;
        case MMFE_CTRL_FME:
            if (mctx->i_state >= MMFE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_FME;
                mcfg.fme.i_iframe_bitsthr = ctrl->fme.i_iframe_bits_threshold;
                mcfg.fme.i_pframe_bitsthr = ctrl->fme.i_pframe_bits_threshold;
                if((err = mops->set_conf(mops, &mcfg)))
                    break;

                rqcf.type = RQCT_CFG_OFT;
                if((err = rqct->get_rqcf(rqct, &rqcf)))
                    break;

                rqcf.oft.i_offsetqp = ctrl->fme.i_qp_offset;
                if((err =  rqct->set_rqcf(rqct, &rqcf)))
                    break;
            }
            else
            {
                CamOsPrintf("MFE ERR: Set MMFE_CTRL_FME in wrong state\n");
            }
            err = 0;
            break;
        case MMFE_CTRL_RST:
            if (mctx->i_state >= MMFE_CTX_STATE_NULL)
                mops->restore(mops);
            else
                CamOsPrintf("MFE ERR: Set MMFE_CTRL_RST in wrong state\n");
            err = 0;
            break;
        case MMFE_CTRL_SEQ:
            if (mctx->i_state > MMFE_CTX_STATE_NULL)
            {
                if ((unsigned)ctrl->seq.i_pixfmt <= MMFE_PIXFMT_YVYU &&
                        ctrl->seq.i_pixelw >= 128 && ctrl->seq.i_pixelw <= _ALIGN_(4, mctx->i_cropw) &&
                        ctrl->seq.i_pixelh >= 128 && // EROY CHECK
                        ctrl->seq.d_fps > 0 &&
                        ctrl->seq.n_fps > 0)
                {
                    int mbw, mbh, mbn;

                    mcfg.type = MHVE_CFG_RES;
                    mops->get_conf(mops, &mcfg);
                    mcfg.res.e_pixf = ctrl->seq.i_pixfmt;
                    mcfg.res.i_pixw = ctrl->seq.i_pixelw;
                    mcfg.res.i_pixh = ctrl->seq.i_pixelh;
                    if ((err = mops->set_conf(mops, &mcfg)))
                        break;

                    rqcf.type = RQCT_CFG_FPS;
                    rqcf.fps.n_fps = (short)ctrl->seq.n_fps;
                    rqcf.fps.d_fps = (short)ctrl->seq.d_fps;
                    if ((err = rqct->set_rqcf(rqct, &rqcf)))
                        break;

                    mbw = _ALIGN_(4, mcfg.res.i_pixw)>>4;
                    mbh = _ALIGN_(4, mcfg.res.i_pixh)>>4;
                    mbn = mbw*mbh;
                    rqcf.type = RQCT_CFG_RES;
                    rqcf.res.i_picw = (short)(mbw*16);
                    rqcf.res.i_pich = (short)(mbh*16);
                    if ((err = rqct->set_rqcf(rqct, &rqcf)))
                        break;

                    /* Disable ROI */
                    rqcf.type = RQCT_CFG_ROI;
                    rqcf.roi.i_roiidx = -1;
                    rqct->set_rqcf(rqct, &rqcf);

                    /* Reset DQM */
                    rqcf.type = RQCT_CFG_DQM;
                    rqct->get_rqcf(rqct, &rqcf);
                    rqcf.dqm.i_dqmw = mbw;
                    rqcf.dqm.i_dqmh = mbh;
                    rqcf.dqm.i_size = _ALIGN_(8,(mbn+1)/2+16);
                    rqct->set_rqcf(rqct, &rqcf);
                    err = 0;
                }
            }
            else
            {
                CamOsPrintf("MFE ERR: Set MMFE_CTRL_SEQ in wrong state\n");
            }
            break;
        case MMFE_CTRL_LTR:
            /* MUST set param first(can't open LTR mode after streamon) */
            mcfg.type = MHVE_CFG_LTR;
            err = mops->get_conf(mops, &mcfg);
            /* RQCT control LTR P-frame period */
            if (mcfg.ltr.b_long_term_reference)
            {
                mcfg.ltr.b_enable_pred = ctrl->ltr.b_enable_pred;
                err = mops->set_conf(mops, &mcfg);
                rqcf.type = RQCT_CFG_LTR;
                if (!rqct->get_rqcf(rqct, &rqcf))
                {
                    rqcf.ltr.i_period = ctrl->ltr.i_ltr_period;
                    if (!rqct->set_rqcf(rqct, &rqcf))
                        err = 0;
                }
            }
            err = 0;
            break;
        default:
            break;
    }

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxGetCtrl(
    void*           pctx,
    mmfe_ctrl*      ctrl)
{
    int err = -EINVAL;
    mmfe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);
    rqct_cfg rqcf;
    mhve_cfg mcfg;

    CamOsMutexLock(&mctx->m_stream);

    switch (ctrl->type)
    {
        case MMFE_CTRL_ROI:
            if (mctx->i_state > MMFE_CTX_STATE_NULL)
            {
                rqcf.type = RQCT_CFG_ROI;
                rqcf.roi.i_roiidx = ctrl->roi.i_index;
                rqct->get_rqcf(rqct, &rqcf);
                ctrl->roi.i_absqp = rqcf.roi.i_absqp;
                ctrl->roi.i_roiqp = rqcf.roi.i_roiqp;
                ctrl->roi.i_mbx = rqcf.roi.i_posx;
                ctrl->roi.i_mby = rqcf.roi.i_posy;
                ctrl->roi.i_mbw = rqcf.roi.i_recw;
                ctrl->roi.i_mbh = rqcf.roi.i_rech;
                err = 0;
            }
            else
            {
                CamOsPrintf("MFE ERR: Get MMFE_CTRL_ROI in wrong state\n");
            }
            break;
        case MMFE_CTRL_BGFPS:
            if(mctx->i_state > MMFE_CTX_STATE_NULL)
            {
                rqcf.type = RQCT_CFG_BGF;
                if(!(err = rqct->get_rqcf(rqct, &rqcf)))
                {
                    ctrl->bgfps.i_bgsrcfps = rqcf.bgf.i_bgsrcfps;
                    ctrl->bgfps.i_bgdstfps = rqcf.bgf.i_bgdstfps;
                }

                err = 0;
            }
            else
            {
                CamOsPrintf("MFE ERR: Set MMFE_CTRL_BGFPS in wrong state\n");
            }
            break;
        case MMFE_CTRL_SPL:
            if (mctx->i_state >= MMFE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_SPL;
                mops->get_conf(mops, &mcfg);
                ctrl->spl.i_rows = mcfg.spl.i_rows;
                ctrl->spl.i_bits = mcfg.spl.i_bits;
                err = 0;
            }
            else
            {
                CamOsPrintf("MFE ERR: Get MMFE_CTRL_SPL in wrong state\n");
            }
            break;
        case MMFE_CTRL_FME:
            if (mctx->i_state >= MMFE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_FME;
                if((err = mops->get_conf(mops, &mcfg)))
                    break;
                ctrl->fme.i_iframe_bits_threshold = mcfg.fme.i_iframe_bitsthr;
                ctrl->fme.i_pframe_bits_threshold = mcfg.fme.i_pframe_bitsthr;

                rqcf.type = RQCT_CFG_OFT;
                if((err = rqct->get_rqcf(rqct, &rqcf)))
                    break;
                ctrl->fme.i_qp_offset = rqcf.oft.i_offsetqp;
                err = 0;
            }
            else
            {
                CamOsPrintf("MFE ERR: Get MMFE_CTRL_FME in wrong state\n");
            }
            break;
        case MMFE_CTRL_SEQ:
            if (mctx->i_state > MMFE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_RES;
                mops->get_conf(mops, &mcfg);
                ctrl->seq.i_pixelw = mcfg.res.i_pixw;
                ctrl->seq.i_pixelh = mcfg.res.i_pixh;
                ctrl->seq.i_pixfmt = mcfg.res.e_pixf;
                rqcf.type = RQCT_CFG_FPS;
                rqct->get_rqcf(rqct, &rqcf);
                ctrl->seq.n_fps = (int)rqcf.fps.n_fps;
                ctrl->seq.d_fps = (int)rqcf.fps.d_fps;
                err = 0;
            }
            else
            {
                CamOsPrintf("MFE ERR: Get MMFE_CTRL_SEQ in wrong state\n");
            }
            break;
        case MMFE_CTRL_LTR:
            mcfg.type = MHVE_CFG_LTR;
            err = mops->get_conf(mops, &mcfg);
            /* RQCT control LTR P-frame period */
            if (mcfg.ltr.b_long_term_reference)
            {
                ctrl->ltr.b_enable_pred = mcfg.ltr.b_enable_pred;
                rqcf.type = RQCT_CFG_LTR;
                if (!rqct->get_rqcf(rqct, &rqcf))
                {
                    ctrl->ltr.i_ltr_period = rqcf.ltr.i_period;
                }
            }
            else
            {
                ctrl->ltr.b_enable_pred = 0;
                ctrl->ltr.i_ltr_period = 0;
            }
            err = 0;
            break;
        default:
            break;
    }

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxCompress(
    void*       pctx,
    mmfe_buff*  buff)
{
    mmfe_buff* buf = buff;
    mmfe_buff* out = buff + 1;
    mmfe_ctx* mctx = pctx;
    mmfe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    int pitch, err = -1;

    // EROY CHECK
    if (buff->i_memory != MMFE_MEMORY_MMAP)
        return -1;
    if (out->i_memory != MMFE_MEMORY_MMAP || out->i_planes != 1)
        return -1;

    pitch = buff->i_stride;
    if (pitch < buff->i_width)
        pitch = buff->i_width;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if (MMFE_CTX_STATE_IDLE == mctx->i_state && MMFE_OMODE_MMAP == mctx->i_omode)
        {
            mhve_cfg mcfg;
            mhve_cpb mcpb;
            mhve_vpb mvpb;

            /* Parameter Check */
            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            if (buff->i_planes != 2 && (MHVE_PIX_NV21 >= mcfg.res.e_pixf))
                break;
            if (buff->i_planes != 1 && (MHVE_PIX_YUYV == mcfg.res.e_pixf))
                break;
            if (mcfg.res.i_pixw != buf->i_width ||
                mcfg.res.i_pixh != buf->i_height)
                break;

            if (mcfg.res.e_pixf > MHVE_PIX_NV21)
                pitch *= 2;

            mcpb.i_index = 0;

            /* Fill Video Picture Buffer Info From mmfe_buff Structure */
            mvpb.i_index = buf->i_index;
            mvpb.i_stamp = buf->i_timecode;
            mvpb.u_flags = buf->i_flags & MMFE_FLAGS_CONTROL;
            mvpb.i_pitch = pitch;
            mvpb.planes[1].u_phys = 0;
            mvpb.planes[1].i_bias = 0;
            mvpb.planes[0].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[0].mem.phys);
            mvpb.planes[0].i_bias = buf->planes[0].i_bias;
            if (mcfg.res.e_pixf <= MHVE_PIX_NV21)
            {
                mvpb.planes[1].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[1].mem.phys);
                mvpb.planes[1].i_bias = buf->planes[1].i_bias;
            }

            /* Fill Output Bitstream Buffer Info From mmfe_buff Structure */
            mcfg.type = MHVE_CFG_DMA;
            mcfg.dma.i_dmem = MHVE_CFG_DMA_OUTPUT_BUFFER;
            mcfg.dma.p_vptr = out->planes[0].mem.uptr;
            mcfg.dma.u_phys = (uint)ADDR_TYPE_SELECT(out->planes[0].mem.phys);
            mcfg.dma.i_size[0] = out->planes[0].i_size;
            mcfg.dma.pFlushCacheCb = out->planes[0].mem.pFlushCacheCb;

            /* Change Context State */
            mctx->i_state = MMFE_CTX_STATE_BUSY;

            mops->set_conf(mops, &mcfg);

            do
            {
                if (0 != (err = mops->enc_buff(mops, &mvpb)))
                    break;

                /* Put User Data Before Trigger Encode */
                mops->put_data(mops, mctx->p_usrdt, mctx->i_usrsz);
                mops->enc_conf(mops);

                /* Trigger Encode and Wait Done */
                MfeDevPushjob(mdev, mctx);

                if (0 < (err = mops->enc_done(mops)))
                    continue;
            }
            while (0);

            if (!err)
            {
                mctx->i_usrcn = mctx->i_usrsz = 0;
                mcpb.i_index = -1;
            }

            /* Dequeue the Input Frame Buffer */
            mops->deq_buff(mops, &mvpb);
            buff->i_index = mvpb.i_index;

            /* Get Output Bitstream Information */
            err = mops->out_buff(mops, &mcpb);
            out->planes[0].i_used = mcpb.planes[0].i_size;
            out->i_timecode = mcpb.i_stamp;
            out->i_flags = mcpb.i_flags;

            mctx->i_state = MMFE_CTX_STATE_IDLE;
        }
    }
    while (0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxPutData(
    void*       pctx,
    mmfe_buff*  buff)
{
    int err = -EINVAL;
    mmfe_ctx* mctx = pctx;
    void* dst;

    if (buff->i_memory != MMFE_MEMORY_USER || buff->i_planes != 1) // EROY CHECK
        return err;

    if (buff->planes[0].mem.uptr == NULL ||
        buff->planes[0].i_size < buff->planes[0].i_used ||
        buff->planes[0].i_used > MMFE_SEI_MAX_LEN)
        return err;

    CamOsMutexLock(&mctx->m_stream);

    do {
        if (MMFE_CTX_STATE_NULL != mctx->i_state && 4 > mctx->i_usrcn)
        {
            dst = (char*)mctx->p_usrdt + mctx->i_usrsz;
            err = -EFAULT;

            /* Copy data to user data buffer */
            MEM_COPY(dst, buff->planes[0].mem.uptr, buff->planes[0].i_used);

            mctx->i_usrsz += buff->planes[0].i_used;
            mctx->i_usrcn++;
            err = 0;
        }
    }
    while (0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

int
MfeCtxEncFireAndReturn(
    void*       pctx,
    mmfe_buff*  buff)
{
    mmfe_buff* buf = buff;
    mmfe_buff* out = buff + 1;
    mmfe_ctx* mctx = pctx;
    mmfe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    int pitch, err = -1;

    //CamOsPrintf("%s\n", __FUNCTION__);
    // EROY CHECK
    if (buff->i_memory != MMFE_MEMORY_MMAP)
        return -1;
    if (out->i_memory != MMFE_MEMORY_MMAP || out->i_planes != 1)
        return -1;

    pitch = buff->i_stride;
    if (pitch < buff->i_width)
        pitch = buff->i_width;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if (MMFE_CTX_STATE_IDLE == mctx->i_state && MMFE_OMODE_MMAP == mctx->i_omode)
        {
            mhve_cfg mcfg;
            mhve_cpb mcpb;
            mhve_vpb mvpb;

            /* Parameter Check */
            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            if (buff->i_planes != 2 && (MHVE_PIX_NV21 >= mcfg.res.e_pixf))
                break;
            if (buff->i_planes != 1 && (MHVE_PIX_YUYV == mcfg.res.e_pixf))
                break;
            if (mcfg.res.i_pixw != buf->i_width ||  mcfg.res.i_pixh != buf->i_height)
            {
                // break;
            }

            if (mcfg.res.e_pixf > MHVE_PIX_NV21)
                pitch *= 2;

            mcpb.i_index = 0;

            /* Fill Video Picture Buffer Info From mmfe_buff Structure */
            mvpb.i_index = buf->i_index;
            mvpb.i_stamp = buf->i_timecode;
            mvpb.u_flags = buf->i_flags & MMFE_FLAGS_CONTROL;
            mvpb.i_pitch = pitch;
            mvpb.planes[1].u_phys = 0;
            mvpb.planes[1].i_bias = 0;
            mvpb.planes[0].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[0].mem.phys);
            mvpb.planes[0].i_bias = buf->planes[0].i_bias;
            if (mcfg.res.e_pixf <= MHVE_PIX_NV21)
            {
                mvpb.planes[1].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[1].mem.phys);
                mvpb.planes[1].i_bias = buf->planes[1].i_bias;
            }

            /* Fill Output Bitstream Buffer Info From mmfe_buff Structure */
            mcfg.type = MHVE_CFG_DMA;
            mcfg.dma.i_dmem = -1;
            mcfg.dma.p_vptr = out->planes[0].mem.uptr;
            mcfg.dma.u_phys = (uint)ADDR_TYPE_SELECT(out->planes[0].mem.phys);
            mcfg.dma.i_size[0] = out->planes[0].i_size;
            mcfg.dma.pFlushCacheCb = out->planes[0].mem.pFlushCacheCb;

            mops->backup(mops);

            /* Change Context State */
            mctx->i_state = MMFE_CTX_STATE_BUSY;
            mops->set_conf(mops, &mcfg);

            do
            {
                if (0 != (err = mops->enc_buff(mops, &mvpb)))
                    break;

                /* Put User Data Before Trigger Encode */
                mops->put_data(mops, mctx->p_usrdt, mctx->i_usrsz);
                mops->enc_conf(mops);

                /* Trigger Encode and Return Immediately */
                err = MfeDevLockAndFire(mdev, mctx);
            }
            while (0);
        }
    }
    while (0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

int
MfeCtxEncPostProc(
    void*       pctx,
    mmfe_buff*  buff)
{
    mmfe_buff* out = buff + 1;
    mmfe_ctx* mctx = pctx;
    mmfe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    int err = -1;

    //CamOsPrintf("%s\n", __FUNCTION__);

    // EROY CHECK
    if (buff->i_memory != MMFE_MEMORY_MMAP)
        return -1;
    if (out->i_memory != MMFE_MEMORY_MMAP || out->i_planes != 1)
        return -1;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if (MMFE_CTX_STATE_BUSY == mctx->i_state && MMFE_OMODE_MMAP == mctx->i_omode)
        {
            mhve_cpb mcpb;
            mhve_vpb mvpb;

            mcpb.i_index = 0;

            do
            {
                MfeDevUnlockAfterEncDone(mdev, mctx);

                if (0 < (err = mops->enc_done(mops)))
                    continue;
            }
            while (0);

            if (!err)
            {
                mctx->i_usrcn = mctx->i_usrsz = 0;
                mcpb.i_index = -1;
            }

            /* Dequeue the Input Frame Buffer */
            mops->deq_buff(mops, &mvpb);
            buff->i_index = mvpb.i_index;

            /* Get Output Bitstream Information */
            err = mops->out_buff(mops, &mcpb);
            out->planes[0].i_used = mcpb.planes[0].i_size;
            out->i_timecode = mcpb.i_stamp;
            out->i_flags = mcpb.i_flags;

            mctx->i_state = MMFE_CTX_STATE_IDLE;
        }
    }
    while (0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

int
MfeCtxGenCompressRegCmd(
    void*       pctx,
    mmfe_buff*  buff,
    void* cmd_buf,
    int* cmd_len)
{
    mmfe_buff* buf = buff;
    mmfe_buff* out = buff + 1;
    mmfe_ctx* mctx = pctx;
    mmfe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    int pitch, err = -1;

    //CamOsPrintf("%s\n", __FUNCTION__);

    // EROY CHECK
    if (buff->i_memory != MMFE_MEMORY_MMAP)
        return -1;
    if (out->i_memory != MMFE_MEMORY_MMAP || out->i_planes != 1)
        return -1;

    pitch = buff->i_stride;
    if (pitch < buff->i_width)
        pitch = buff->i_width;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if (MMFE_CTX_STATE_IDLE == mctx->i_state && MMFE_OMODE_MMAP == mctx->i_omode)
        {
            mhve_cfg mcfg;
            mhve_cpb mcpb;
            mhve_vpb mvpb;

            /* Parameter Check */
            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            if (buff->i_planes != 2 && (MHVE_PIX_NV21 >= mcfg.res.e_pixf))
                break;
            if (buff->i_planes != 1 && (MHVE_PIX_YUYV == mcfg.res.e_pixf))
                break;
            if (mcfg.res.i_pixw != buf->i_width ||  mcfg.res.i_pixh != buf->i_height)
            {
                //break;
            }

            if (mcfg.res.e_pixf > MHVE_PIX_NV21)
                pitch *= 2;

            mcpb.i_index = 0;

            /* Fill Video Picture Buffer Info From mmfe_buff Structure */
            mvpb.i_index = buf->i_index;
            mvpb.i_stamp = buf->i_timecode;
            mvpb.u_flags = buf->i_flags & MMFE_FLAGS_CONTROL;
            mvpb.i_pitch = pitch;
            mvpb.planes[1].u_phys = 0;
            mvpb.planes[1].i_bias = 0;
            mvpb.planes[0].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[0].mem.phys);
            mvpb.planes[0].i_bias = buf->planes[0].i_bias;
            if (mcfg.res.e_pixf <= MHVE_PIX_NV21)
            {
                mvpb.planes[1].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[1].mem.phys);
                mvpb.planes[1].i_bias = buf->planes[1].i_bias;
            }

            /* Fill Output Bitstream Buffer Info From mmfe_buff Structure */
            mcfg.type = MHVE_CFG_DMA;
            mcfg.dma.i_dmem = MHVE_CFG_DMA_OUTPUT_BUFFER;
            mcfg.dma.p_vptr = out->planes[0].mem.uptr;
            mcfg.dma.u_phys = (uint)ADDR_TYPE_SELECT(out->planes[0].mem.phys);
            mcfg.dma.i_size[0] = out->planes[0].i_size;
            mcfg.dma.pFlushCacheCb = out->planes[0].mem.pFlushCacheCb;

            mops->backup(mops);

            /* Change Context State */
            mctx->i_state = MMFE_CTX_STATE_BUSY;

            mops->set_conf(mops, &mcfg);

            do
            {
                if (0 != (err = mops->enc_buff(mops, &mvpb)))
                    break;

                /* Put User Data Before Trigger Encode */
                mops->put_data(mops, mctx->p_usrdt, mctx->i_usrsz);
                mops->enc_conf(mops);

                /* Trigger Encode and Return Immediately */
                MfeDevGenEncCmd(mdev, mctx, cmd_buf, cmd_len);
            }
            while (0);
        }
    }
    while (0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxSetRqcf(
    void*       pctx,
    rqct_conf*  rqcf)
{
    int err = -EINVAL;
    mmfe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);

    CamOsMutexLock(&mctx->m_stream);

    if ((unsigned)rqcf->type < RQCT_CONF_END && !rqct->set_rqcf(rqct, (rqct_cfg*)rqcf))
        err = 0;

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxGetRqcf(
    void*       pctx,
    rqct_conf*  rqcf)
{
    int err = -EINVAL;
    mmfe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);

    CamOsMutexLock(&mctx->m_stream);

    if ((unsigned)rqcf->type < RQCT_CONF_END && !rqct->get_rqcf(rqct, (rqct_cfg*)rqcf))
        err = 0;

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxSetPmbr(
    void*           pctx,
    pmbr_conf*   pmbrcfg)
{
    int err = -1;
    mmfe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    pmbr_ops* pmbr = mops->pmbr_ops(mops);

    CamOsMutexLock(&mctx->m_stream);

    if ((unsigned)pmbrcfg->type < DRV_PMBR_CFG_END && !pmbr->set_conf(pmbr, (pmbr_cfg*)pmbrcfg))
        err = 0;

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MfeCtxGetPmbr(
    void*           pctx,
    pmbr_conf*   pmbrcfg)
{
    int err = -1;
    mmfe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    pmbr_ops* pmbr = mops->pmbr_ops(mops);

    CamOsMutexLock(&mctx->m_stream);

    if ((unsigned)pmbrcfg->type < DRV_PMBR_CFG_END && !pmbr->get_conf(pmbr, (pmbr_cfg*)pmbrcfg))
        err = 0;

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}
