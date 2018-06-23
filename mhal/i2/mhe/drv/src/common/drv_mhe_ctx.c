#include "mdrv_mhe_io.h"
#include "mdrv_rqct_io.h"
#include "mdrv_pmbr_io.h"
#include "mhve_pmbr_ops.h"
#include "drv_mhe_kernel.h"
#include "drv_mhe_ctx.h"
#include "drv_mhe_dev.h"
#include "hal_mhe_def.h"
#include "hal_mhe_api.h"


#define MMHE_FLAGS_CONTROL  (MMHE_FLAGS_IDR|MMHE_FLAGS_DISPOSABLE|MMHE_FLAGS_NIGHT_MODE)

#ifdef USE_PHYSICAL_ADDR
#define ADDR_TYPE_SELECT(buf)       (CamOsDirectMemPhysToMiu((void*)(u32)buf))
#else
#define ADDR_TYPE_SELECT(buf)       (buf)
#endif

static int _MheCtxStreamOn(void* pctx, int on);
static int _MheCtxEncPict(void* pctx, mmhe_buff* buff);
static int _MheCtxGetBits(void* pctx, mmhe_buff* buff);
static int _MheCtxSetParm(void* pctx, mmhe_parm* parm);
static int _MheCtxGetParm(void* pctx, mmhe_parm* parm);
static int _MheCtxSetCtrl(void* pctx, mmhe_ctrl* ctrl);
static int _MheCtxGetCtrl(void* pctx, mmhe_ctrl* ctrl);
static int _MheCtxCompress(void* pctx, mmhe_buff* buff);
static int _MheCtxPutData(void* pctx, mmhe_buff* buff);
static int _MheCtxSetRqcf(void* pctx, rqct_conf* rqcf);
static int _MheCtxGetRqcf(void* pctx, rqct_conf* rqcf);
static int _MheCtxSetPmbr(void* pctx, pmbr_conf* pmbrcfg);
static int _MheCtxGetPmbr(void* pctx, pmbr_conf* pmbrcfg);


static void _MheCtxRelease(void* pctx)
{
    mmhe_ctx* mctx = pctx;

    if(CamOsAtomicDecAndTest(&mctx->i_refcnt))
    {
        mhve_ops* mops = mctx->p_handle;

        /* Release direct memory buffer */
        while(mctx->i_dmems > 0)
        {
            mctx->i_dmems--;
            CamOsDirectMemRelease(mctx->m_dmems[mctx->i_dmems], mctx->s_dmems[mctx->i_dmems]);
        }

        /* Release OPs and Context object */
        CamOsMutexLock(&mctx->m_stream);
        mctx->i_state = MMHE_CTX_STATE_NULL;
        mctx->p_handle = NULL;
        mops->release(mops);
        CamOsMutexUnlock(&mctx->m_stream);

        CamOsMutexDestroy(&mctx->m_stream);
#if MMHE_TIMER_SIZE>0
        CamOsMemRelease(mctx->p_timer);
#endif
        CamOsMemRelease(mctx->p_usrdt);
        CamOsMemRelease(mctx);
    }
}

static void _MheCtxAdduser(void* pctx)
{
    mmhe_ctx* mctx = pctx;

    CamOsAtomicIncReturn(&mctx->i_refcnt);
}

mmhe_ctx*
MheCtxAcquire(
    mmhe_dev*   mdev)
{
    mmhe_ctx* mctx = NULL;
    mhve_ops* mops = NULL;

    /* Allocate mmhe_ctx object */
    mctx = CamOsMemCalloc(1, sizeof(mmhe_ctx));
    if((mdev == NULL) || (mctx == NULL))
    {
        MHE_MSG(MHE_MSG_ERR, "> Mctx Memory Alloc Fail \n");
        return NULL;
    }

    do
    {
        /* Link OPs function */
        mops = MheOpsAcquire(mdev->i_rctidx);
        if(mops == NULL)
        {
            MHE_MSG(MHE_MSG_ERR, "> Create Mops Fail \n");
            return NULL;
        }

        /* Allocate user data buffer */
        mctx->p_usrdt = CamOsMemCalloc(1, MMHE_USER_DATA_SIZE);
        if(mctx->p_usrdt == NULL)
        {
            MHE_MSG(MHE_MSG_ERR, "> User Data Memory Alloc Fail \n");
            return NULL;
        }
#if MMHE_TIMER_SIZE>0
        mctx->p_timer = CamOsMemCalloc(1, MMHE_TIMER_SIZE);
        mctx->i_numbr = 0;
#endif

        mctx->i_enccnt = 0;

        /* Initialize OS signal */
        CamOsMutexInit(&mctx->m_stream);

        mctx->i_state = MMHE_CTX_STATE_NULL;
        mctx->release = _MheCtxRelease;
        mctx->adduser = _MheCtxAdduser;
        CamOsAtomicSet(&mctx->i_refcnt, 1);
        mctx->p_handle = mops;
        return mctx;
    }
    while(0);

    if(mops)
        mops->release(mops);

    CamOsMemRelease(mctx);
    return NULL;
}

long
MheCtxActions(
    mmhe_ctx*       mctx,
    unsigned int    cmd,
    void*           arg)
{
    int err = 0;

    switch(cmd)
    {
        case IOCTL_MMHE_S_PARM:
            err = _MheCtxSetParm(mctx, (mmhe_parm*)arg);
            break;
        case IOCTL_MMHE_G_PARM:
            err = _MheCtxGetParm(mctx, (mmhe_parm*)arg);
            break;
        case IOCTL_MMHE_STREAMON:
            err = _MheCtxStreamOn(mctx, 1);
            break;
        case IOCTL_MMHE_STREAMOFF:
            err = _MheCtxStreamOn(mctx, 0);
            break;
        case IOCTL_MMHE_S_PICT:
            err = _MheCtxEncPict(mctx, (mmhe_buff*)arg);
            break;
        case IOCTL_MMHE_G_BITS:
            err = _MheCtxGetBits(mctx, (mmhe_buff*)arg);
            break;
        case IOCTL_MMHE_S_CTRL:
            err = _MheCtxSetCtrl(mctx, (mmhe_ctrl*)arg);
            break;
        case IOCTL_MMHE_G_CTRL:
            err = _MheCtxGetCtrl(mctx, (mmhe_ctrl*)arg);
            break;
        case IOCTL_MMHE_ENCODE:
            err = _MheCtxCompress(mctx, (mmhe_buff*)arg);
            break;
        case IOCTL_MMHE_S_DATA:
            err = _MheCtxPutData(mctx, (mmhe_buff*)arg);
            break;
        case IOCTL_RQCT_S_CONF:
            err = _MheCtxSetRqcf(mctx, (rqct_conf*)arg);
            break;
        case IOCTL_RQCT_G_CONF:
            err = _MheCtxGetRqcf(mctx, (rqct_conf*)arg);
            break;
        case IOCTL_PMBR_S_CONF:
            err = _MheCtxSetPmbr(mctx, (pmbr_conf*)arg);
            break;
        case IOCTL_PMBR_G_CONF:
            err = _MheCtxGetPmbr(mctx, (pmbr_conf*)arg);
            break;
        default:
            err = -EINVAL;
            break;
    }
    return (long)err;
}



static int
_MheCtxStreamOn(
    void*   pctx,
    int     on)
{
    mmhe_ctx* mctx = pctx;
    mmhe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);
    rqct_cfg  rqcf;
    pmbr_ops* pmbr = mops->pmbr_ops(mops);
    pmbr_cfg  pmbrcfg;
    int err = 0;
    char szDmemName[20];

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if(on)
        {
            mhve_cfg mcfg;
            int size_mbr_lut, size_mbr_read_map, enc_frame_size;
            int size_mbr_gn, size_gn_mem;
            int size_ppu_int_b, size_ppu_int_a;
            int size_col, size_ppu_y, size_ppu_c;
            int size_dump_reg;
            int cbw, cbh, cbn;
            int size_lum, size_chr, size_out;
            int size_ppu_total;
            int size_total_al, size_total_rc;
            int score;
            int enc_w, enc_h;
#if defined(MMHE_IMI_BUF_ADDR)
            int ctb_width;
            int size_imi_ref_y = 0;
            int size_imi_ref_c = 0;
            uint32 current_sram_addr = MMHE_IMI_BUF_ADDR;
            uint32 total_imi_size;
#endif
            int i, rpbn;

            void *pUserPtr = NULL;
            void *nMiuAddr = NULL;
            void *nPhysAddr = NULL;
            void *pUserPtr2 = NULL;
            void *nMiuAddr2 = NULL;
            void *nPhysAddr2 = NULL;

            if(MMHE_CTX_STATE_NULL != mctx->i_state)
                break;
            mctx->i_dmems = 0;

            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            cbw = _ALIGN_(5, mcfg.res.i_pixw) / 32;
            cbh = _ALIGN_(5, mcfg.res.i_pixh) / 32;
            cbn = cbw * cbh;

            enc_w = _ALIGN_(5, mcfg.res.i_pixw);
            enc_h = _ALIGN_(5, mcfg.res.i_pixh);

            /* Calculate required buffer size */

            size_dump_reg = 256;    // Fixme, 25x64bit on I2 MHE

            // PMBR (delta-)QP LUT
            size_mbr_lut = 256;
            enc_frame_size = enc_w * enc_h;

            // CTB weight read (or write) buffer
            // Each CTB occupies 1 byte
            // Register: reg_mbr_gn_read_st_addr, reg_mbr_gn_write_st_addr
            size_mbr_gn = (((enc_frame_size >> 10) >> 5) + 1) << 5;

            // ROI map: including (delta-)QP-entry map, forced-Intra map, forced-ZeroMV map
            // Each CTB occupies 2 bytes.
            // Register: reg_mbr_gn_read_map_st_addr
            size_mbr_read_map = (((enc_frame_size >> 9) >> 5) + 1) << 5;

            // GN buffer
            // Each CTB occupies 32 * 8 bytes
            // Register: reg_hev_gn_mem_sadr
            size_gn_mem = enc_w * 8;
#if defined(MMHE_IMI_BUF_ADDR)
            if(enc_w <= MMHE_IMI_MAX_WIDTH)
            {
                size_gn_mem = enc_w * 4;
            }
#endif

            // PPU intermedia A, B buffer
            // Register: reg_hev_ppu_fb_a_y_base, reg_hev_ppu_fb_b_y_base
            size_ppu_int_b = enc_w * 8;
            size_ppu_int_a = enc_h * 8;
#if defined(MMHE_IMI_BUF_ADDR)
            if(enc_w <= MMHE_IMI_MAX_WIDTH)
            {
                size_ppu_int_b = enc_w * 10;
            }
#endif

            // Colocated buffer (read and write)
            // Each CTB occupies 64 bytes
            // Register: reg_hev_col_r_sadr0, reg_hev_col_w_sadr
            size_col = enc_frame_size / 16;

            // Reference frame and reconstruction frame
            // Register: reg_hev_ref_y_adr, reg_hev_ppu_fblut_luma_base
            //           reg_hev_ref_c_adr, reg_hev_ppu_fblut_chroma_base
            size_ppu_y = enc_frame_size;
            size_ppu_c = enc_frame_size / 2;

            // Low bandwidth - imi
#if defined(MMHE_IMI_BUF_ADDR)
            if(enc_w <= MMHE_IMI_MAX_WIDTH)
            {
                ctb_width = ((mcfg.res.i_pixw + 31) >> 5);
                size_imi_ref_y = (ctb_width << 5) * 96 + 4096; // (32*LumaCtbWidth)*96 + 4096  Bytes
                size_imi_ref_c = (ctb_width << 5) * 48 + 2048; // (32*ChromaCtbWidth)*48 + 2048 Bytes
            }
#endif

            // 512 byte align
            size_gn_mem = _ALIGN_(9, size_gn_mem);
            size_ppu_int_b = _ALIGN_(9, size_ppu_int_b);
            size_ppu_int_a = _ALIGN_(9, size_ppu_int_a);
            size_ppu_y = _ALIGN_(9, size_ppu_y);
            size_ppu_c = _ALIGN_(9, size_ppu_c);
            size_col = _ALIGN_(9, size_col);

            size_mbr_gn = _ALIGN_(9, size_mbr_gn);
            size_mbr_read_map = _ALIGN_(9, size_mbr_read_map);
            size_mbr_lut = _ALIGN_(9, size_mbr_lut);

            size_ppu_total = size_ppu_int_b + size_ppu_int_a ;

            // output bitstream buffer
            size_lum = _ALIGN_(16, cbn * MBPIXELS_Y * 4);
            size_chr = _ALIGN_(16, cbn * MBPIXELS_C * 4);
            size_out = _ALIGN_(20, size_lum);
            if(mctx->i_omode == MMHE_OMODE_MMAP)
                size_out = 0;

            rpbn = mcfg.res.i_rpbn;

            /* Calculate total size */
            size_total_rc = (size_ppu_y + size_ppu_c + size_col) * rpbn;
            size_total_al = size_dump_reg + size_gn_mem + size_mbr_lut + size_mbr_read_map + size_mbr_gn*2 + size_ppu_total;
#if 0
            CamOsPrintf("MHE memory info: %d (%d %d)\n", mctx->i_index, mctx->i_ialsz, mctx->i_ircms);
            CamOsPrintf("-------------------\n");
            CamOsPrintf("size_total_rc: %d \n", size_total_rc);
            CamOsPrintf("size_total_al: %d \n", size_total_al);
            CamOsPrintf("   size_dump_reg    : %d \n", size_dump_reg);
            CamOsPrintf("   size_gn_mem      : %d \n", size_gn_mem);
            CamOsPrintf("   size_mbr_lut     : %d \n", size_mbr_lut);
            CamOsPrintf("   size_mbr_read_map: %d \n", size_mbr_read_map);
            CamOsPrintf("   size_mbr_gn      : %d \n", size_mbr_gn);
            CamOsPrintf("   size_ppu_total   : %d \n", size_ppu_total);
            CamOsPrintf("-------------------\n");
#endif
            if(mctx->i_ialsz == 0)
            {
                memset(szDmemName, 0, sizeof(szDmemName));
                snprintf(szDmemName, sizeof(szDmemName), "%d:MHEALBUF", mctx->i_index);

                CamOsDirectMemAlloc(szDmemName, size_total_al, &pUserPtr, &nPhysAddr, &nMiuAddr);

                mctx->m_dmems[mctx->i_dmems] = (char*)pUserPtr;
                mctx->s_dmems[mctx->i_dmems] = size_total_al;

                if(mctx->m_dmems[mctx->i_dmems++] == NULL)
                {
                    break;
                }
                //CamOsPrintf("[%d:MHEALBUF] CamOsDirectMemAlloc: pUserPtr = 0x%08X, nMiuAddr = 0x%08X, 0x%08X\n",
                //              mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr, (u32) (nMiuAddr+size_total_al));
            }
            else
            {
                pUserPtr = mctx->p_ialva;
                nMiuAddr = mctx->p_ialma;
                //CamOsPrintf("[%d:MHEALBUF] MI: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n",
                //        mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr);
            }

            memset((void *)pUserPtr, 0, mctx->i_ialsz);

            if(mctx->i_ircms == 0)
            {
                memset(szDmemName, 0, sizeof(szDmemName));
                snprintf(szDmemName, sizeof(szDmemName), "%d:MHERCBUF", mctx->i_index);

                CamOsDirectMemAlloc(szDmemName, size_total_rc, &pUserPtr2, &nPhysAddr2, &nMiuAddr2);

                mctx->m_dmems[mctx->i_dmems] = (char*)pUserPtr2;
                mctx->s_dmems[mctx->i_dmems] = size_total_rc;

                if(mctx->m_dmems[mctx->i_dmems++] == NULL)
                {
                    break;
                }
                //CamOsPrintf("[%d:MHERCBUF] CamOsDirectMemAlloc: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n",
                //        mctx->i_index, (u32) pUserPtr2,(u32) nMiuAddr2);
            }
            else
            {
                nMiuAddr2 = mctx->p_ircma;
                //CamOsPrintf("[%d:MHERCBUF] MI: pUserPtr = 0x%08X, nMiuAddr = 0x%08X\n",
                //        mctx->i_index, (u32) pUserPtr2,(u32) nMiuAddr2);
            }

            do
            {
                /* Allocate Buffers */
                mctx->p_ovptr = NULL;
                mctx->u_ophys = 0;
                mctx->i_osize = mctx->i_ormdr = 0;

                /* Allocate output bitstream buffer if necessary */
                if(size_out > 0)
                {
                    void *pUserPtrOut = NULL;
                    void *nMiuAddrOut = NULL;
                    void *nPhysAddrOut = NULL;

                    memset(szDmemName, 0, sizeof(szDmemName));
                    snprintf(szDmemName, sizeof(szDmemName), "%d:OUTBUF", mctx->i_index);

                    CamOsDirectMemAlloc(szDmemName, size_out, &pUserPtrOut, &nMiuAddrOut, &nPhysAddrOut);

                    mctx->m_dmems[mctx->i_dmems] = (char*)pUserPtr;
                    mctx->s_dmems[mctx->i_dmems] = size_out;

                    if(mctx->m_dmems[mctx->i_dmems++] == NULL)
                    {
                        break;
                    }

                    mcfg.type = MHVE_CFG_DMA;
                    mcfg.dma.i_dmem = -1;
                    mcfg.dma.p_vptr = pUserPtrOut;
                    mcfg.dma.u_phys = (u32)nMiuAddrOut;
                    mcfg.dma.i_size[0] = size_out;
                    mops->set_conf(mops, &mcfg);

                    mctx->p_ovptr = (unsigned char*)pUserPtrOut;
                    mctx->u_ophys = (u32)nMiuAddrOut;
                    mctx->i_osize = size_out;
                }

                //CamOsPrintf("[%d:gn_mem] user 0x%08X, miu 0x%08X, size %d\n",
                //        mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr, size_gn_mem);
                /* gn mem buffer */
#if defined(MMHE_IMI_BUF_ADDR)
                if(enc_w <= MMHE_IMI_MAX_WIDTH)
                {
                    mcfg.type = MHVE_CFG_BUF;
                    mops->get_conf(mops, &mcfg);
                    mcfg.hev_buff_addr.gn_mem = (u32)current_sram_addr;
                    mops->set_conf(mops, &mcfg);
                    current_sram_addr += size_gn_mem;
                }
                else
                {
                    mcfg.type = MHVE_CFG_BUF;
                    mops->get_conf(mops, &mcfg);
                    mcfg.hev_buff_addr.gn_mem = (u32)nMiuAddr;
                    mops->set_conf(mops, &mcfg);
                    nMiuAddr += size_gn_mem;
                    pUserPtr += size_gn_mem;
                }

#else
                mcfg.type = MHVE_CFG_BUF;
                mops->get_conf(mops, &mcfg);
                mcfg.hev_buff_addr.gn_mem = (u32)nMiuAddr;
                mops->set_conf(mops, &mcfg);
                nMiuAddr += size_gn_mem;
                pUserPtr += size_gn_mem;
#endif

                //CamOsPrintf("[%d:mbr_gn_w] user 0x%08X, miu 0x%08X, size %d\n",
                //        mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr, size_mbr_gn);
                /* Allocate PMBR Buffer */
                pmbrcfg.type = PMBR_CFG_MEM;
                mops->get_conf(mops, &mcfg);
                pmbrcfg.mem.u_phys[PMBR_MEM_MB_MAP_IN] = (u32)nMiuAddr;
                pmbrcfg.mem.p_kptr[PMBR_MEM_MB_MAP_IN] = pUserPtr;
                pmbrcfg.mem.i_size[PMBR_MEM_MB_MAP_IN] = size_mbr_gn;
                nMiuAddr += size_mbr_gn;//size_pmbr_map;
                pUserPtr += size_mbr_gn;

                //CamOsPrintf("[%d:mbr_gn_r] user 0x%08X, miu 0x%08X, size %d\n",
                //         mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr, size_mbr_gn);
                pmbrcfg.mem.u_phys[PMBR_MEM_MB_MAP_OUT] = (u32)nMiuAddr;
                pmbrcfg.mem.p_kptr[PMBR_MEM_MB_MAP_OUT] = pUserPtr;
                pmbrcfg.mem.i_size[PMBR_MEM_MB_MAP_OUT] = size_mbr_gn;
                nMiuAddr += size_mbr_gn;
                pUserPtr += size_mbr_gn;

                //CamOsPrintf("[%d:mbr_lut] user 0x%08X, miu 0x%08X, size %d\n",
                //        mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr, size_mbr_lut);
                pmbrcfg.type = PMBR_CFG_MEM;
                pmbrcfg.mem.u_phys[PMBR_MEM_LUT] = (u32)nMiuAddr;
                pmbrcfg.mem.p_kptr[PMBR_MEM_LUT] = pUserPtr;
                pmbrcfg.mem.i_size[PMBR_MEM_LUT] = size_mbr_lut;
                pmbr->set_conf(pmbr, &pmbrcfg);
                nMiuAddr += size_mbr_lut;
                pUserPtr += size_mbr_lut;

                //CamOsPrintf("[%d:mbr_read_map] user 0x%08X, miu 0x%08X, size %d\n",
                //        mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr, size_mbr_read_map);
                /* Allocate ROI DQ Map/GN Buffer */
                rqcf.type = RQCT_CFG_DQM;
                rqct->get_rqcf(rqct, &rqcf);
                rqcf.dqm.u_phys = (u32)nMiuAddr;
                rqcf.dqm.p_kptr = pUserPtr;
                rqcf.dqm.i_dqmw = cbw;
                rqcf.dqm.i_dqmh = cbh;
                rqcf.dqm.i_size = size_mbr_read_map;
                rqct->set_rqcf(rqct, &rqcf);
                nMiuAddr += size_mbr_read_map;
                pUserPtr += size_mbr_read_map;

                //CamOsPrintf("[%d:ppu_int_b] user 0x%08X, miu 0x%08X, size %d\n",
                //        mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr, size_ppu_int_b);
                /* PPU intermediate data */
#if defined(MMHE_IMI_BUF_ADDR)
                if(enc_w <= MMHE_IMI_MAX_WIDTH)
                {
                    mcfg.type = MHVE_CFG_BUF;
                    mops->get_conf(mops, &mcfg);
                    mcfg.hev_buff_addr.ppu_int_b = (u32)current_sram_addr;
                    mops->set_conf(mops, &mcfg);
                    current_sram_addr += size_ppu_int_b;
                }
                else
                {
                    mcfg.type = MHVE_CFG_BUF;
                    mops->get_conf(mops, &mcfg);
                    mcfg.hev_buff_addr.ppu_int_b = (u32)nMiuAddr;
                    mops->set_conf(mops, &mcfg);
                    nMiuAddr += size_ppu_int_b;
                    pUserPtr += size_ppu_int_b;
                }

#else
                mcfg.type = MHVE_CFG_BUF;
                mops->get_conf(mops, &mcfg);
                mcfg.hev_buff_addr.ppu_int_b = (u32)nMiuAddr;
                mops->set_conf(mops, &mcfg);
                nMiuAddr += size_ppu_int_b;
                pUserPtr += size_ppu_int_b;

                //CamOsPrintf("[%d:ppu_int_a] user 0x%08X, miu 0x%08X, size %d\n",
                //        mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr, size_ppu_int_a);
                mcfg.type = MHVE_CFG_BUF;
                mops->get_conf(mops, &mcfg);
                mcfg.hev_buff_addr.ppu_int_a = (u32)nMiuAddr;
                mops->set_conf(mops, &mcfg);
                nMiuAddr += size_ppu_int_a;
                pUserPtr += size_ppu_int_a;
#endif
                //CamOsPrintf("[%d:dump_reg] user 0x%08X, miu 0x%08X, size %d\n",
                //        mctx->i_index, (u32) pUserPtr,(u32) nMiuAddr, size_dump_reg);
                /* dump reg buffer */
                mcfg.type = MHVE_CFG_DUMP_REG;
                mcfg.dump_reg.u_phys = nMiuAddr;
                mcfg.dump_reg.p_vptr = pUserPtr;
                mcfg.dump_reg.i_size = size_dump_reg;
                mops->set_conf(mops, &mcfg);
                nMiuAddr += size_dump_reg;
                pUserPtr += size_dump_reg;

#if defined(MMHE_IMI_BUF_ADDR)
                if(enc_w <= MMHE_IMI_MAX_WIDTH)
                {
                    mcfg.type = MHVE_CFG_BUF;
                    mops->get_conf(mops, &mcfg);
                    mcfg.hev_buff_addr.imi_ref_y_buf = (u32)current_sram_addr;
                    mops->set_conf(mops, &mcfg);
                    current_sram_addr += size_imi_ref_y;

                    mcfg.type = MHVE_CFG_BUF;
                    mops->get_conf(mops, &mcfg);
                    mcfg.hev_buff_addr.imi_ref_c_buf = (u32)current_sram_addr;
                    mops->set_conf(mops, &mcfg);
                    current_sram_addr += size_imi_ref_c;

                    total_imi_size = size_imi_ref_y + size_imi_ref_c + size_gn_mem + size_ppu_int_b;

                    MHE_MSG(MHE_MSG_DEBUG,"[IMI] Total IMI Size: %d\n", total_imi_size);
                    MHE_MSG(MHE_MSG_DEBUG,"[IMI] IMI Start/End Addr: 0x%X / 0x%X\n", mcfg.hev_buff_addr.gn_mem, current_sram_addr);
                    MHE_MSG(MHE_MSG_DEBUG,"[IMI] gn_mem       : 0x%08X, size: %d\n", mcfg.hev_buff_addr.gn_mem, size_gn_mem);
                    MHE_MSG(MHE_MSG_DEBUG,"[IMI] ppu_int_b    : 0x%08X, size: %d\n", mcfg.hev_buff_addr.ppu_int_b, size_ppu_int_b);
                    MHE_MSG(MHE_MSG_DEBUG,"[IMI] imi_ref_y_buf: 0x%08X, size: %d\n", mcfg.hev_buff_addr.imi_ref_y_buf,size_imi_ref_y);
                    MHE_MSG(MHE_MSG_DEBUG,"[IMI] imi_ref_c_buf: 0x%08X, size: %d\n", mcfg.hev_buff_addr.imi_ref_c_buf,size_imi_ref_c);
                }

#endif
                /* Allocate Reference/Reconstructed Picture Buffers */
                if(mctx->i_imode == MMHE_IMODE_PURE)
                {
                    for(i = 0; i < rpbn; i++)
                    {
                        //CamOsPrintf("%s: RefBuf%d: 0x%08X\n",__FUNCTION__,i, (u32)nMiuAddr2);
                        mcfg.type = MHVE_CFG_DMA;
                        mcfg.dma.i_dmem = i;
                        mcfg.dma.u_phys = (u32)nMiuAddr2;
                        mcfg.dma.p_vptr = NULL;
                        mcfg.dma.i_size[0] = size_ppu_y;
                        mcfg.dma.i_size[1] = size_ppu_c;
                        mcfg.dma.i_size[2] = size_col;
                        mops->set_conf(mops, &mcfg);
                        nMiuAddr2 += _ALIGN_(9, size_ppu_y + size_ppu_c + size_col);
                    }
                }
                else // (mctx->i_imode == MMHE_IMODE_PLUS) // EROY CHECK
                {
                    for(i = 0; i < rpbn; i++)
                    {
                        mcfg.type = MHVE_CFG_DMA;
                        mcfg.dma.i_dmem = i;
                        mcfg.dma.u_phys = 0;
                        mcfg.dma.p_vptr = NULL;
                        mcfg.dma.i_size[0] = 0;
                        mcfg.dma.i_size[1] = 0;
                        mcfg.dma.i_size[2] = 0;
                        mops->set_conf(mops, &mcfg);
                    }
                }
            }
            while(0);

            rqcf.type = RQCT_CFG_FPS;
            rqct->get_rqcf(rqct, &rqcf);

            score = cbn;
            score *= (int)rqcf.fps.n_fps;
            score /= (int)rqcf.fps.d_fps;
            mctx->i_score = score;

            /* Start Streaming */
            if(!(err = mops->seq_sync(mops)))
            {
                mops->seq_conf(mops);
                /* Reset Statistic Data and State */
                MheDevPowerOn(mdev, mctx->i_score);
                mdev->i_counts[mctx->i_index][0] = mdev->i_counts[mctx->i_index][1] = 0;
                mdev->i_counts[mctx->i_index][2] = mdev->i_counts[mctx->i_index][3] = 0;
                mdev->i_counts[mctx->i_index][4] = 0;
                mctx->i_state = MMHE_CTX_STATE_IDLE;
                break;
            }
        }
        else
        {
            if(MMHE_CTX_STATE_NULL == mctx->i_state)
                break;

            MheDevPowerOn(mdev, -mctx->i_score);
            mctx->i_state = MMHE_CTX_STATE_NULL;
            mctx->i_score = 0;

            /* Stop Streaming */
            mops->seq_done(mops);
#if 0
            CamOsPrintf("<%d>mhe performance:\n", mctx->i_index);
            for(i = 0; i < MMHE_TIMER_SIZE / 8; i++)
                CamOsPrintf("<%d>%4d/%4d/%8d\n", mctx->i_index, mctx->p_timer[i].tm_dur[0], mctx->p_timer[i].tm_dur[1], mctx->p_timer[i].tm_cycles);
#endif
        }
    }
    while(0);

    if(!on || err)
    {
        while(mctx->i_dmems > 0)
        {
            mctx->i_dmems--;
            CamOsDirectMemRelease(mctx->m_dmems[mctx->i_dmems], mctx->s_dmems[mctx->i_dmems]);
        }
    }

    mops->backup(mops);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MheCtxEncPict(
    void*           pctx,
    mmhe_buff*      buff)
{
    int err = -EINVAL;
    int pitch = 0;
    mmhe_ctx* mctx = pctx;
    mmhe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    mhve_cpb* pcpb = mctx->m_mcpbs;

    if(buff->i_memory != MMHE_MEMORY_MMAP)
        return -1;

    pitch = buff->i_stride;
    if(pitch < buff->i_width)
        pitch = buff->i_width;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if(MMHE_CTX_STATE_IDLE == mctx->i_state && MMHE_OMODE_USER == mctx->i_omode)
        {
            mhve_vpb mvpb;
            mhve_cfg mcfg;

            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);

            if(buff->i_planes != 2 && (MHVE_PIX_NV21 >= mcfg.res.e_pixf))
                break;
            if(buff->i_planes != 1 && (MHVE_PIX_YUYV <= mcfg.res.e_pixf))
                break;
            if(mcfg.res.i_pixw != buff->i_width || mcfg.res.i_pixh != buff->i_height)
                break;

            mvpb.i_index = buff->i_index;
            mvpb.i_stamp = buff->i_timecode;
            mvpb.u_flags = buff->i_flags & MMHE_FLAGS_CONTROL;
            mvpb.i_pitch = pitch;
            mvpb.planes[1].u_phys = 0;
            mvpb.planes[1].i_bias = 0;
            mvpb.planes[0].u_phys = (uint)ADDR_TYPE_SELECT(buff->planes[0].mem.phys);
            mvpb.planes[0].i_bias = buff->planes[0].i_bias;
            if(mcfg.res.e_pixf <= MHVE_PIX_NV21)
            {
                mvpb.planes[1].u_phys = (uint)ADDR_TYPE_SELECT(buff->planes[1].mem.phys);
                mvpb.planes[1].i_bias = buff->planes[1].i_bias;
            }

            mctx->i_state = MMHE_CTX_STATE_BUSY;

            if(!(err = mops->enc_buff(mops, &mvpb)))
            {
                do
                {
                    mops->put_data(mops, mctx->p_usrdt, mctx->i_usrsz);
                    mops->enc_conf(mops);
                    MheDevPushjob(mdev, mctx);
                }
                while(0 < (err = mops->enc_done(mops)));
            }

            if(!err)
            {
                mctx->i_usrcn = mctx->i_usrsz = 0;
                pcpb->i_index = -1;
            }

            mctx->i_ormdr =
                err =
                    mops->out_buff(mops, mctx->m_mcpbs);
        }
    }
    while(0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MheCtxGetBits(
    void*           pctx,
    mmhe_buff*      buff)
{
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    int err = -ENODATA;

    if(buff->i_memory != MMHE_MEMORY_USER || buff->i_planes != 1)
        return -1;

    CamOsMutexLock(&mctx->m_stream);

    if(MMHE_CTX_STATE_BUSY == mctx->i_state && MMHE_OMODE_USER == mctx->i_omode)
    {
        int flags = 0;
        int rmdr = mctx->i_ormdr;
        int size = mctx->m_mcpbs->planes[0].i_size;
        unsigned char* ptr = mctx->p_ovptr + size - rmdr; // EROY CHECK

        if(rmdr == size)
            flags = MMHE_FLAGS_SOP;
        if(buff->planes[0].i_size < rmdr)
            rmdr = buff->planes[0].i_size;

        buff->i_flags = 0;

        if(CamOsCopyToUpperLayer(buff->planes[0].mem.uptr, ptr, rmdr))
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

        if(0 == mctx->i_ormdr)
        {
            mctx->i_state = MMHE_CTX_STATE_IDLE;
            flags |= MMHE_FLAGS_EOP;
            mops->out_buff(mops, mctx->m_mcpbs); // EROY CHECK
        }

        if(!err)
            buff->i_flags = flags | (mctx->m_mcpbs->i_flags & MMHE_FLAGS_CONTROL);
    }

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MheCtxSetParm(
    void*           pctx,
    mmhe_parm*      parm)
{
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    int err = -EINVAL;

    CamOsMutexLock(&mctx->m_stream);

    /* Set below Parameters Before Streaming On (Static) */
    if(mctx->i_state == MMHE_CTX_STATE_NULL)
    {
        rqct_ops* rqct = mops->rqct_ops(mops);
        rqct_cfg  rqcf;
        pmbr_ops* pmbr = mops->pmbr_ops(mops);
        pmbr_cfg  pmbrcfg;
        mhve_cfg  mcfg;
        unsigned char b_long_term_reference;

        switch(parm->type)
        {
            case MMHE_PARM_RES:
                /* check LTR mode */
                mcfg.type = MHVE_CFG_LTR;
                err = mops->get_conf(mops, &mcfg);
                b_long_term_reference = mcfg.ltr.b_long_term_reference;

                if((unsigned)parm->res.i_pixfmt > MMHE_PIXFMT_YUYV)
                    break;
                if((parm->res.i_pict_w % 16) || (parm->res.i_pict_h % 2))
                    break;

                mctx->i_pict_w = parm->res.i_pict_w;
                mctx->i_pict_h = parm->res.i_pict_h;
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

                // EROY CHECK
                mctx->i_omode = MMHE_OMODE_USER;
                mctx->i_imode = MMHE_IMODE_PURE; // RTK-I3E Setting

                if(parm->res.i_outlen < 0)
                    mctx->i_omode = MMHE_OMODE_MMAP; // RTK-I3E Setting
                if(parm->res.i_outlen < -1)
                    mctx->i_imode = MMHE_IMODE_PLUS;

                mops->set_conf(mops, &mcfg);

                rqcf.type = RQCT_CFG_RES;
                rqcf.res.i_picw = _ALIGN_(5, parm->res.i_pict_w);
                rqcf.res.i_pich = _ALIGN_(5, parm->res.i_pict_h);
                rqct->set_rqcf(rqct, &rqcf);

                pmbrcfg.type = PMBR_CFG_SEQ;
                pmbrcfg.seq.i_enable = 1;
                pmbrcfg.seq.i_ctuw = _ALIGN_(5, mcfg.res.i_pixw) >> 5;
                pmbrcfg.seq.i_ctuh = _ALIGN_(5, mcfg.res.i_pixh) >> 5;
                pmbr->set_conf(pmbr, &pmbrcfg);

                err = 0;
                break;
            case MMHE_PARM_FPS:
                if(parm->fps.i_num > 0 && parm->fps.i_den > 0)
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
            case MMHE_PARM_MOT:
                mcfg.type = MHVE_CFG_MOT;
                mcfg.mot.i_subp = parm->mot.i_subpel;
                mcfg.mot.i_dmvx = parm->mot.i_dmv_x<=32?32:96;
                mcfg.mot.i_dmvy = parm->mot.i_dmv_y<=16?16:48;
                mcfg.mot.i_blkp[0] = parm->mot.i_mvblks[0];
                mcfg.mot.i_blkp[1] = 0;
                err = mops->set_conf(mops, &mcfg);
                break;
            case MMHE_PARM_BPS:
                if((unsigned)parm->bps.i_method > RQCT_METHOD_VBR)
                    break;
                rqcf.type = RQCT_CONF_SEQ;
                if(!rqct->get_rqcf(rqct, &rqcf))
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
            case MMHE_PARM_GOP:
                if(parm->gop.i_pframes < 0)
                    break;
                rqcf.type = RQCT_CONF_SEQ;
                if(!rqct->get_rqcf(rqct, &rqcf))
                {
                    rqcf.seq.i_period = parm->gop.i_pframes + 1;
                    rqcf.seq.b_passiveI = parm->gop.b_passiveI;
                    if(!rqct->set_rqcf(rqct, &rqcf))
                    {
                        err = 0;
                    }
                }
                break;
            case MMHE_PARM_HEVC:
                if((parm->hevc.i_log2_max_cb_size - 3) > 3 ||
                        (parm->hevc.i_log2_min_cb_size - 3) > 3 ||
                        parm->hevc.i_log2_max_cb_size < parm->hevc.i_log2_min_cb_size)
                    break;
                if((parm->hevc.i_log2_max_tr_size - 2) > 3 ||
                        (parm->hevc.i_log2_min_tr_size - 2) > 3 ||
                        parm->hevc.i_log2_max_tr_size < parm->hevc.i_log2_min_tr_size ||
                        parm->hevc.i_log2_max_tr_size < parm->hevc.i_log2_min_cb_size)
                    break;
                if(parm->hevc.i_tr_depth_intra > (parm->hevc.i_log2_max_cb_size - parm->hevc.i_log2_min_tr_size) ||
                        parm->hevc.i_tr_depth_inter > (parm->hevc.i_log2_max_cb_size - parm->hevc.i_log2_min_tr_size))
                    break;
                if((unsigned)(parm->hevc.i_tc_offset_div2 + 6) > 12 ||
                        (unsigned)(parm->hevc.i_beta_offset_div2 + 6) > 12 ||
                        (unsigned)(parm->hevc.i_cb_qp_offset + 12) > 24 ||
                        (unsigned)(parm->hevc.i_cr_qp_offset + 12) > 24)
                    break;
                mcfg.type = MHVE_CFG_HEV;
                mcfg.hev.i_profile = parm->hevc.i_profile;
                mcfg.hev.i_level = parm->hevc.i_level;
                mcfg.hev.i_log2_max_cb_size = parm->hevc.i_log2_max_cb_size;
                mcfg.hev.i_log2_min_cb_size = parm->hevc.i_log2_min_cb_size;
                mcfg.hev.i_log2_max_tr_size = parm->hevc.i_log2_max_tr_size;
                mcfg.hev.i_log2_min_tr_size = parm->hevc.i_log2_min_tr_size;
                mcfg.hev.i_tr_depth_intra = parm->hevc.i_tr_depth_intra;
                mcfg.hev.i_tr_depth_inter = parm->hevc.i_tr_depth_inter;
                mcfg.hev.b_scaling_list_enable = parm->hevc.b_scaling_list_enable;
                mcfg.hev.b_ctu_qp_delta_enable = parm->hevc.b_ctu_qp_delta_enable;
                mcfg.hev.b_sao_enable = parm->hevc.b_sao_enable;
                mcfg.hev.i_cb_qp_offset = parm->hevc.i_cb_qp_offset;
                mcfg.hev.i_cr_qp_offset = parm->hevc.i_cr_qp_offset;
                mcfg.hev.b_strong_intra_smoothing = parm->hevc.b_strong_intra_smoothing;
                mcfg.hev.b_constrained_intra_pred = parm->hevc.b_constrained_intra_pred;
                mcfg.hev.b_deblocking_override_enable = parm->hevc.b_deblocking_override_enable;
                mcfg.hev.b_deblocking_disable = parm->hevc.b_deblocking_disable;
                mcfg.hev.i_tc_offset_div2 = parm->hevc.i_tc_offset_div2;
                mcfg.hev.i_beta_offset_div2 = parm->hevc.i_beta_offset_div2;
                if((err = mops->set_conf(mops, &mcfg)))
                    break;

                pmbrcfg.type = PMBR_CFG_FRM;
                pmbrcfg.frm.i_cb_qp_offset = mcfg.hev.i_cb_qp_offset;
                pmbrcfg.frm.i_cr_qp_offset = mcfg.hev.i_cr_qp_offset;
                if((err = pmbr->set_conf(pmbr, &pmbrcfg)))
                    break;
                break;
            case MMHE_PARM_VUI:
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
                mcfg.vui.i_num_ticks_poc_diff_one_minus1 = parm->vui.i_num_ticks_poc_diff_one_minus1;
                err = mops->set_conf(mops, &mcfg);
                break;
            case MMHE_PARM_LTR:
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
        CamOsPrintf("MHE ERR: Set MMHE_PARM(%d) in wrong state\n", parm->type);
    }

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MheCtxGetParm(
    void*           pctx,
    mmhe_parm*      parm)
{
    int err = 0;
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);
    mhve_cfg  mcfg;
    rqct_cfg  rqcf;

    CamOsMutexLock(&mctx->m_stream);

    switch(parm->type)
    {
        case MMHE_PARM_IDX:
            parm->idx.i_stream = mctx->i_index;
            break;
        case MMHE_PARM_RES:
            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            parm->res.i_pict_w = mctx->i_pict_w;
            parm->res.i_pict_h = mctx->i_pict_h;
            parm->res.i_crop_w = mctx->i_cropw;
            parm->res.i_crop_h = mctx->i_croph;
            parm->res.i_crop_offset_x = mctx->i_cropoffsetx;
            parm->res.i_crop_offset_y = mctx->i_cropoffsety;
            parm->res.i_pixfmt = mcfg.res.e_pixf;
            parm->res.i_outlen = 0;
            parm->res.i_flags = 0;
            break;
        case MMHE_PARM_FPS:
            rqcf.type = RQCT_CFG_FPS;
            if(!(err = rqct->get_rqcf(rqct, &rqcf)))
            {
                parm->fps.i_num = (int)rqcf.fps.n_fps;
                parm->fps.i_den = (int)rqcf.fps.d_fps;
            }
            break;
        case MMHE_PARM_MOT:
            mcfg.type = MHVE_CFG_MOT;
            mops->get_conf(mops, &mcfg);
            parm->mot.i_dmv_x = mcfg.mot.i_dmvx;
            parm->mot.i_dmv_y = mcfg.mot.i_dmvy;
            parm->mot.i_subpel = mcfg.mot.i_subp;
            parm->mot.i_mvblks[0] = mcfg.mot.i_blkp[0];
            parm->mot.i_mvblks[1] = 0;
            break;
        case MMHE_PARM_BPS:
        case MMHE_PARM_GOP:
            rqcf.type = RQCT_CONF_SEQ;
            if(!rqct->get_rqcf(rqct, &rqcf))
            {
                if(parm->type == MMHE_PARM_BPS)
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
                    parm->gop.i_pframes = rqcf.seq.i_period - 1;
                    parm->gop.i_bframes = 0;
                    parm->gop.b_passiveI = rqcf.seq.b_passiveI;
                }
            }
            break;
        case MMHE_PARM_HEVC:
            mcfg.type = MHVE_CFG_HEV;
            if(0 != (err = mops->get_conf(mops, &mcfg)))
                break;
            parm->hevc.i_profile = mcfg.hev.i_profile;
            parm->hevc.i_level = mcfg.hev.i_level;
            parm->hevc.i_log2_max_cb_size = mcfg.hev.i_log2_max_cb_size;
            parm->hevc.i_log2_min_cb_size = mcfg.hev.i_log2_min_cb_size;
            parm->hevc.i_log2_max_tr_size = mcfg.hev.i_log2_max_tr_size;
            parm->hevc.i_log2_min_tr_size = mcfg.hev.i_log2_min_tr_size;
            parm->hevc.i_tr_depth_intra = mcfg.hev.i_tr_depth_intra;
            parm->hevc.i_tr_depth_inter = mcfg.hev.i_tr_depth_inter;
            parm->hevc.b_scaling_list_enable = mcfg.hev.b_scaling_list_enable;
            parm->hevc.b_ctu_qp_delta_enable = mcfg.hev.b_ctu_qp_delta_enable;
            parm->hevc.b_sao_enable = mcfg.hev.b_sao_enable;
            parm->hevc.b_strong_intra_smoothing = mcfg.hev.b_strong_intra_smoothing;
            parm->hevc.b_constrained_intra_pred = mcfg.hev.b_constrained_intra_pred;
            parm->hevc.b_deblocking_disable = mcfg.hev.b_deblocking_disable;
            parm->hevc.b_deblocking_override_enable = mcfg.hev.b_deblocking_override_enable;
            parm->hevc.i_tc_offset_div2 = mcfg.hev.i_tc_offset_div2;
            parm->hevc.i_beta_offset_div2 = mcfg.hev.i_beta_offset_div2;
            parm->hevc.i_cb_qp_offset = mcfg.hev.i_cb_qp_offset;
            parm->hevc.i_cr_qp_offset = mcfg.hev.i_cr_qp_offset;
            break;
        case MMHE_PARM_VUI:
            mcfg.type = MHVE_CFG_VUI;
            if(!(err = mops->get_conf(mops, &mcfg)))
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
                parm->vui.i_num_ticks_poc_diff_one_minus1 = mcfg.vui.i_num_ticks_poc_diff_one_minus1;
            }
            break;
        case MMHE_PARM_LTR:
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
_MheCtxSetCtrl(
    void*           pctx,
    mmhe_ctrl*      ctrl)
{
    int err = -EINVAL;
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);
    rqct_cfg rqcf;
    mhve_cfg mcfg;
    int i;

    CamOsMutexLock(&mctx->m_stream);

    /* Set Below Parameters After Streaming On (Dynamically) */
    switch(ctrl->type)
    {
        case MMHE_CTRL_ROI:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
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
                rqcf.roi.i_posx = ctrl->roi.i_cbx;
                rqcf.roi.i_posy = ctrl->roi.i_cby;
                rqcf.roi.i_recw = ctrl->roi.i_cbw;
                rqcf.roi.i_rech = ctrl->roi.i_cbh;
                rqct->set_rqcf(rqct, &rqcf);

                err = 0;
            }
            else
            {
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_ROI in wrong state\n");
            }
            break;
        case MMHE_CTRL_BGFPS:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
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
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_BGFPS in wrong state\n");
            }
            break;
        case MMHE_CTRL_QPMAP:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
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
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_QPMAP in wrong state\n");
            }
            break;
        case MMHE_CTRL_SPL:
            if(mctx->i_state >= MMHE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_SPL;
                mcfg.spl.i_rows = ctrl->spl.i_rows;
                mcfg.spl.i_bits = ctrl->spl.i_bits;
                mops->set_conf(mops, &mcfg);
                err = 0;
            }
            else
            {
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_SPL in wrong state\n");
            }
            break;
        case MMHE_CTRL_FME:
            if (mctx->i_state >= MMHE_CTX_STATE_NULL)
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
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_FME in wrong state\n");
            }
            err = 0;
            break;
        case MMHE_CTRL_RST:
            if (mctx->i_state >= MMHE_CTX_STATE_NULL)
                mops->restore(mops);
            else
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_RST in wrong state\n");
            err = 0;
            break;
        case MMHE_CTRL_SEQ:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
            {
                if((unsigned)ctrl->seq.i_pixfmt <= MMHE_PIXFMT_YVYU &&
                        ctrl->seq.i_pixelw >= 128 &&
                        ctrl->seq.i_pixelh >= 128 && // EROY CHECK
                        ctrl->seq.i_pixelw <= _ALIGN_(5, mctx->i_cropw) &&
                        ctrl->seq.d_fps > 0 &&
                        ctrl->seq.n_fps > 0)
                {
                    int mbw, mbh, mbn;

                    mcfg.type = MHVE_CFG_RES;
                    mops->get_conf(mops, &mcfg);
                    mcfg.res.e_pixf = ctrl->seq.i_pixfmt;
                    mcfg.res.i_pixw = ctrl->seq.i_pixelw;
                    mcfg.res.i_pixh = ctrl->seq.i_pixelh;
                    if((err = mops->set_conf(mops, &mcfg)))
                        break;

                    rqcf.type = RQCT_CFG_FPS;
                    rqcf.fps.n_fps = (short)ctrl->seq.n_fps;
                    rqcf.fps.d_fps = (short)ctrl->seq.d_fps;
                    if((err = rqct->set_rqcf(rqct, &rqcf)))
                        break;

                    mbw = _ALIGN_(4, mcfg.res.i_pixw) >> 4; //from H264
                    mbh = _ALIGN_(4, mcfg.res.i_pixh) >> 4; //from H264
                    mbn = mbw * mbh;
                    rqcf.type = RQCT_CFG_RES;
                    rqcf.res.i_picw = (short)(mbw * 16);
                    rqcf.res.i_pich = (short)(mbh * 16);
                    if((err = rqct->set_rqcf(rqct, &rqcf)))
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
                    rqcf.dqm.i_size = _ALIGN_(8, (mbn + 1) / 2 + 16);
                    rqct->set_rqcf(rqct, &rqcf);
                    err = 0;
                }
            }
            else
            {
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_SEQ in wrong state\n");
            }
            break;
        case MMHE_CTRL_BAC:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_BAC;
                if(!(err = mops->get_conf(mops, &mcfg)))
                    ctrl->bac.b_init = mcfg.bac.b_init;
            }
            else
            {
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_BAC in wrong state\n");
            }
            break;
        case MMHE_CTRL_DBK:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_LFT;
                if(!(err = mops->get_conf(mops, &mcfg)))
                {
                    ctrl->dbk.b_override = (short)mcfg.lft.b_override;
                    ctrl->dbk.b_disable  = (short)mcfg.lft.b_disable;
                    ctrl->dbk.i_tc_offset_div2   = (short)mcfg.lft.i_offsetA;
                    ctrl->dbk.i_beta_offset_div2 = (short)mcfg.lft.i_offsetB;
                }
            }
            else
            {
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_DBK in wrong state\n");
            }
            break;
        case MMHE_CTRL_LTR:
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
_MheCtxGetCtrl(
    void*           pctx,
    mmhe_ctrl*      ctrl)
{
    int err = -EINVAL;
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);
    rqct_cfg rqcf;
    mhve_cfg mcfg;
    int i;

    CamOsMutexLock(&mctx->m_stream);

    switch(ctrl->type)
    {
        case MMHE_CTRL_ROI:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
            {
                rqcf.type = RQCT_CFG_ROI;
                rqct->get_rqcf(rqct, &rqcf);
                ctrl->roi.i_absqp = rqcf.roi.i_absqp;
                ctrl->roi.i_roiqp = rqcf.roi.i_roiqp;
                ctrl->roi.i_cbx = rqcf.roi.i_posx;
                ctrl->roi.i_cby = rqcf.roi.i_posy;
                ctrl->roi.i_cbw = rqcf.roi.i_recw;
                ctrl->roi.i_cbh = rqcf.roi.i_rech;
                err = 0;
            }
            else
            {
                CamOsPrintf("MHE ERR: Get MMHE_CTRL_ROI in wrong state\n");
            }
            break;
        case MMHE_CTRL_QPMAP:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
            {
                rqcf.type = RQCT_CFG_QPM;
                rqct->get_rqcf(rqct, &rqcf);
                ctrl->qpmap.i_enb = rqcf.qpm.i_qpmenb;
                for(i = 0; i < 14; i++)
                    ctrl->qpmap.i_entry[i] = rqcf.qpm.i_entry[i];
                ctrl->qpmap.p_mapkptr = (unsigned char*)rqcf.qpm.p_kptr;

                err = 0;
            }
            else
            {
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_QPMAP in wrong state\n");
            }
            break;
        case MMHE_CTRL_BGFPS:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
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
                CamOsPrintf("MHE ERR: Set MMHE_CTRL_BGFPS in wrong state\n");
            }
            break;
        case MMHE_CTRL_SPL:
            if(mctx->i_state >= MMHE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_SPL;
                mops->get_conf(mops, &mcfg);
                ctrl->spl.i_rows = mcfg.spl.i_rows;
                ctrl->spl.i_bits = mcfg.spl.i_bits;
                err = 0;
            }
            else
            {
                CamOsPrintf("MHE ERR: Get MMHE_CTRL_SPL in wrong state\n");
            }
            break;
        case MMHE_CTRL_FME:
            if (mctx->i_state >= MMHE_CTX_STATE_NULL)
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
                CamOsPrintf("MHE ERR: Get MMHE_CTRL_FME in wrong state\n");
            }
            break;
        case MMHE_CTRL_SEQ:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
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
                CamOsPrintf("MHE ERR: Get MMHE_CTRL_SEQ in wrong state\n");
            }
            break;
        case MMHE_CTRL_BAC:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_BAC;
                if(!(err = mops->get_conf(mops, &mcfg)))
                    ctrl->bac.b_init = mcfg.bac.b_init;
            }
            else
            {
                CamOsPrintf("MHE ERR: Get MMHE_CTRL_BAC in wrong state\n");
            }
            break;
        case MMHE_CTRL_DBK:
            if(mctx->i_state > MMHE_CTX_STATE_NULL)
            {
                mcfg.type = MHVE_CFG_LFT;
                if(!(err = mops->get_conf(mops, &mcfg)))
                {
                    ctrl->dbk.b_override = (short)mcfg.lft.b_override;
                    ctrl->dbk.b_disable  = (short)mcfg.lft.b_disable;
                    ctrl->dbk.i_tc_offset_div2   = (short)mcfg.lft.i_offsetA;
                    ctrl->dbk.i_beta_offset_div2 = (short)mcfg.lft.i_offsetB;
                }
            }
            else
            {
                CamOsPrintf("MHE ERR: Get MMHE_CTRL_DBK in wrong state\n");
            }
            break;
        case MMHE_CTRL_LTR:
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
_MheCtxCompress(
    void*       pctx,
    mmhe_buff*  buff)
{
    mmhe_buff* buf = buff;
    mmhe_buff* out = buff + 1;
    mmhe_ctx* mctx = pctx;
    mmhe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    int pitch, err = -1;

    // EROY CHECK
    if(buff->i_memory != MMHE_MEMORY_MMAP)
        return -1;
    if(out->i_memory != MMHE_MEMORY_MMAP || out->i_planes != 1)
        return -1;

    pitch = buff->i_stride;
    if(pitch < buff->i_width)
        pitch = buff->i_width;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if(MMHE_CTX_STATE_IDLE == mctx->i_state && MMHE_OMODE_MMAP == mctx->i_omode)
        {
            mhve_cfg mcfg;
            mhve_cpb mcpb;
            mhve_vpb mvpb;

            /* Parameter Check */
            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            if(buff->i_planes != 2 && (MHVE_PIX_NV21 >= mcfg.res.e_pixf))
                break;
            if(buff->i_planes != 1 && (MHVE_PIX_YUYV == mcfg.res.e_pixf))
                break;
            if(mcfg.res.i_pixw != buf->i_width || mcfg.res.i_pixh != buf->i_height)
            {
                //MHE_MSG(MHE_MSG_ERR, "[%s %d] w(%d %d) h(%d %d): pixw and buf size are missmatch !!!\n", __FUNCTION__, __LINE__, mcfg.res.i_pixw, buf->i_width, mcfg.res.i_pixh, buf->i_height);
                //break;
            }


            if(mcfg.res.e_pixf > MHVE_PIX_NV21)
                pitch *= 2;

            mcpb.i_index = 0;

            /* Fill Video Picture Buffer Info From mmhe_buff Structure */
            mvpb.i_index = buf->i_index;
            mvpb.i_stamp = buf->i_timecode;
            mvpb.u_flags = buf->i_flags & MMHE_FLAGS_CONTROL;
            mvpb.i_pitch = pitch;
            mvpb.planes[1].u_phys = 0;
            mvpb.planes[1].i_bias = 0;
            mvpb.planes[0].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[0].mem.phys);
            mvpb.planes[0].i_bias = buf->planes[0].i_bias;
            if(mcfg.res.e_pixf <= MHVE_PIX_NV21)
            {
                mvpb.planes[1].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[1].mem.phys);
                mvpb.planes[1].i_bias = buf->planes[1].i_bias;
            }

            /* Fill Output Bitstream Buffer Info From mmhe_buff Structure */
            mcfg.type = MHVE_CFG_DMA;
            mcfg.dma.i_dmem = -1;
            mcfg.dma.p_vptr = out->planes[0].mem.uptr;
            mcfg.dma.u_phys = (uint)ADDR_TYPE_SELECT(out->planes[0].mem.phys);
            mcfg.dma.i_size[0] = out->planes[0].i_size;
            mcfg.dma.pFlushCacheCb = out->planes[0].mem.pFlushCacheCb;

            /* Change Context State */
            mctx->i_state = MMHE_CTX_STATE_BUSY;
            mops->set_conf(mops, &mcfg);

            do
            {
                if(0 != (err = mops->enc_buff(mops, &mvpb)))
                    break;

                /* Put User Data Before Trigger Encode */
                mops->put_data(mops, mctx->p_usrdt, mctx->i_usrsz);
                mops->enc_conf(mops);

                /* Trigger Encode and Wait Done */
                MheDevPushjob(mdev, mctx);

                if(0 < (err = mops->enc_done(mops)))
                    continue;
            }
            while(0);

            if(!err)
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

            //CamOsPrintf("[%s %d] output bit-stream size = %d\n", __FUNCTION__, __LINE__, out->planes[0].i_used);

            mctx->i_state = MMHE_CTX_STATE_IDLE;
        }
    }
    while(0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MheCtxPutData(
    void*       pctx,
    mmhe_buff*  buff)
{
    int err = -EINVAL;
    mmhe_ctx* mctx = pctx;
    void* dst;

    if(buff->i_memory != MMHE_MEMORY_USER || buff->i_planes != 1)  // EROY CHECK
        return err;

    if(buff->planes[0].mem.uptr == NULL ||
       buff->planes[0].i_size < buff->planes[0].i_used ||
       buff->planes[0].i_used > MMHE_SEI_MAX_LEN)
        return err;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if(MMHE_CTX_STATE_NULL != mctx->i_state && 4 > mctx->i_usrcn)
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
    while(0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

int
MheCtxEncFireAndReturn(
    void*       pctx,
    mmhe_buff*  buff)
{
    mmhe_buff* buf = buff;
    mmhe_buff* out = buff + 1;
    mmhe_ctx* mctx = pctx;
    mmhe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    int pitch, err = -1;

    // EROY CHECK
    if(buff->i_memory != MMHE_MEMORY_MMAP)
        return -1;
    if(out->i_memory != MMHE_MEMORY_MMAP || out->i_planes != 1)
        return -1;

    pitch = buff->i_stride;
    if(pitch < buff->i_width)
        pitch = buff->i_width;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if(MMHE_CTX_STATE_IDLE == mctx->i_state && MMHE_OMODE_MMAP == mctx->i_omode)
        {
            mhve_cfg mcfg;
            mhve_cpb mcpb;
            mhve_vpb mvpb;

            /* Parameter Check */
            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            if(buff->i_planes != 2 && (MHVE_PIX_NV21 >= mcfg.res.e_pixf))
                break;
            if(buff->i_planes != 1 && (MHVE_PIX_YUYV == mcfg.res.e_pixf))
                break;
            if(mcfg.res.i_pixw != buf->i_width || mcfg.res.i_pixh != buf->i_height)
            {
                //MHE_MSG(MHE_MSG_ERR, "[%s %d] w(%d %d) h(%d %d): pixw and buf size are missmatch !!!\n", __FUNCTION__, __LINE__, mcfg.res.i_pixw, buf->i_width, mcfg.res.i_pixh, buf->i_height);
                //break;
            }

            if(mcfg.res.e_pixf > MHVE_PIX_NV21)
                pitch *= 2;

            mcpb.i_index = 0;

            /* Fill Video Picture Buffer Info From mmhe_buff Structure */
            mvpb.i_index = buf->i_index;
            mvpb.i_stamp = buf->i_timecode;
            mvpb.u_flags = buf->i_flags & MMHE_FLAGS_CONTROL;
            mvpb.i_pitch = pitch;
            mvpb.planes[1].u_phys = 0;
            mvpb.planes[1].i_bias = 0;
            mvpb.planes[0].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[0].mem.phys);
            mvpb.planes[0].i_bias = buf->planes[0].i_bias;
            if(mcfg.res.e_pixf <= MHVE_PIX_NV21)
            {
                mvpb.planes[1].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[1].mem.phys);
                mvpb.planes[1].i_bias = buf->planes[1].i_bias;
            }

            /* Fill Output Bitstream Buffer Info From mmhe_buff Structure */
            mcfg.type = MHVE_CFG_DMA;
            mcfg.dma.i_dmem = -1;
            mcfg.dma.p_vptr = out->planes[0].mem.uptr;
            mcfg.dma.u_phys = (uint)ADDR_TYPE_SELECT(out->planes[0].mem.phys);
            mcfg.dma.i_size[0] = out->planes[0].i_size;
            mcfg.dma.pFlushCacheCb = out->planes[0].mem.pFlushCacheCb;

            mops->backup(mops);

            /* Change Context State */
            mctx->i_state = MMHE_CTX_STATE_BUSY;

            mops->set_conf(mops, &mcfg);

            do
            {
                if(0 != (err = mops->enc_buff(mops, &mvpb)))
                    break;
                /* Put User Data Before Trigger Encode */
                mops->put_data(mops, mctx->p_usrdt, mctx->i_usrsz);
                mops->enc_conf(mops);

                /* Trigger Encode and Return Immediately */
                err = MheDevLockAndFire(mdev, mctx);
            }
            while(0);
        }
    }
    while(0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

int
MheCtxEncPostProc(
    void*       pctx,
    mmhe_buff*  buff)
{
    mmhe_buff* out = buff + 1;
    mmhe_ctx* mctx = pctx;
    mmhe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    int err = -1;

    // EROY CHECK
    if(buff->i_memory != MMHE_MEMORY_MMAP)
        return -1;
    if(out->i_memory != MMHE_MEMORY_MMAP || out->i_planes != 1)
        return -1;

    CamOsMutexLock(&mctx->m_stream);

    do
    {
        if(MMHE_CTX_STATE_BUSY == mctx->i_state && MMHE_OMODE_MMAP == mctx->i_omode)
        {
            mhve_cpb mcpb;
            mhve_vpb mvpb;

            mcpb.i_index = 0;

            do
            {
                MheDevUnlockAfterEncDone(mdev, mctx);

                if(0 < (err = mops->enc_done(mops)))
                {
                    continue;
                }
            }
            while(0);

            if(!err)
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

            mctx->i_state = MMHE_CTX_STATE_IDLE;
        }
    }
    while(0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

int
MheCtxGenCompressRegCmd(
    void*       pctx,
    mmhe_buff*  buff,
    void* cmd_buf,
    int* cmd_len)
{
    mmhe_buff* buf = buff;
    mmhe_buff* out = buff + 1;
    mmhe_ctx* mctx = pctx;
    mmhe_dev* mdev = mctx->p_device;
    mhve_ops* mops = mctx->p_handle;
    int pitch, err = -1;

    // EROY CHECK
    if(buff->i_memory != MMHE_MEMORY_MMAP)
        return -1;
    if(out->i_memory != MMHE_MEMORY_MMAP || out->i_planes != 1)
        return -1;

    pitch = buff->i_stride;
    if(pitch < buff->i_width)
        pitch = buff->i_width;

    CamOsMutexLock(&mctx->m_stream);

    do
    {

        if(MMHE_CTX_STATE_IDLE == mctx->i_state && MMHE_OMODE_MMAP == mctx->i_omode)
        {

            mhve_cfg mcfg;
            mhve_cpb mcpb;
            mhve_vpb mvpb;

            /* Parameter Check */
            mcfg.type = MHVE_CFG_RES;
            mops->get_conf(mops, &mcfg);
            if(buff->i_planes != 2 && (MHVE_PIX_NV21 >= mcfg.res.e_pixf))
                break;
            if(buff->i_planes != 1 && (MHVE_PIX_YUYV == mcfg.res.e_pixf))
                break;
            if(mcfg.res.i_pixw != buf->i_width || mcfg.res.i_pixh != buf->i_height)
            {
                //MHE_MSG(MHE_MSG_ERR, "[%s %d] w(%d %d) h(%d %d): pixw and buf size are missmatch !!!\n", __FUNCTION__, __LINE__, mcfg.res.i_pixw, buf->i_width, mcfg.res.i_pixh, buf->i_height);
                //break;
            }


            if(mcfg.res.e_pixf > MHVE_PIX_NV21)
                pitch *= 2;

            mcpb.i_index = 0;

            /* Fill Video Picture Buffer Info From mmhe_buff Structure */
            mvpb.i_index = buf->i_index;
            mvpb.i_stamp = buf->i_timecode;
            mvpb.u_flags = buf->i_flags & MMHE_FLAGS_CONTROL;
            mvpb.i_pitch = pitch;
            mvpb.planes[1].u_phys = 0;
            mvpb.planes[1].i_bias = 0;
            mvpb.planes[0].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[0].mem.phys);
            mvpb.planes[0].i_bias = buf->planes[0].i_bias;
            if(mcfg.res.e_pixf <= MHVE_PIX_NV21)
            {
                mvpb.planes[1].u_phys = (uint)ADDR_TYPE_SELECT(buf->planes[1].mem.phys);
                mvpb.planes[1].i_bias = buf->planes[1].i_bias;
            }

            /* Fill Output Bitstream Buffer Info From mmhe_buff Structure */
            mcfg.type = MHVE_CFG_DMA;
            mcfg.dma.i_dmem = -1;
            mcfg.dma.p_vptr = out->planes[0].mem.uptr;
            mcfg.dma.u_phys = (uint)ADDR_TYPE_SELECT(out->planes[0].mem.phys);
            mcfg.dma.i_size[0] = out->planes[0].i_size;
            mcfg.dma.pFlushCacheCb = out->planes[0].mem.pFlushCacheCb;

            mops->backup(mops);

            /* Change Context State */
            mctx->i_state = MMHE_CTX_STATE_BUSY;

            mops->set_conf(mops, &mcfg);

            do
            {
                if(0 != (err = mops->enc_buff(mops, &mvpb)))
                    break;

                /* Put User Data Before Trigger Encode */
                mops->put_data(mops, mctx->p_usrdt, mctx->i_usrsz);
                mops->enc_conf(mops);

                /* Trigger Encode and Return Immediately */
                MheDevGenEncCmd(mdev, mctx, cmd_buf, cmd_len);
            }
            while(0);
        }
    }
    while(0);

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MheCtxSetRqcf(
    void*       pctx,
    rqct_conf*  rqcf)
{
    int err = -EINVAL;
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);

    CamOsMutexLock(&mctx->m_stream);

    if((unsigned)rqcf->type < RQCT_CONF_END && !rqct->set_rqcf(rqct, (rqct_cfg*)rqcf))
        err = 0;

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MheCtxGetRqcf(
    void*       pctx,
    rqct_conf*  rqcf)
{
    int err = -EINVAL;
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    rqct_ops* rqct = mops->rqct_ops(mops);

    CamOsMutexLock(&mctx->m_stream);

    if((unsigned)rqcf->type < RQCT_CONF_END && !rqct->get_rqcf(rqct, (rqct_cfg*)rqcf))
        err = 0;

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MheCtxSetPmbr(
    void*           pctx,
    pmbr_conf*   pmbrcfg)
{
    int err = -1;
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    pmbr_ops* pmbr = mops->pmbr_ops(mops);

    CamOsMutexLock(&mctx->m_stream);

    if((unsigned)pmbrcfg->type < DRV_PMBR_CFG_END && !pmbr->set_conf(pmbr, (pmbr_cfg*)pmbrcfg))
        err = 0;

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}

static int
_MheCtxGetPmbr(
    void*           pctx,
    pmbr_conf*   pmbrcfg)
{
    int err = -1;
    mmhe_ctx* mctx = pctx;
    mhve_ops* mops = mctx->p_handle;
    pmbr_ops* pmbr = mops->pmbr_ops(mops);

    CamOsMutexLock(&mctx->m_stream);

    if((unsigned)pmbrcfg->type < DRV_PMBR_CFG_END && !pmbr->get_conf(pmbr, (pmbr_cfg*)pmbrcfg))
        err = 0;

    CamOsMutexUnlock(&mctx->m_stream);

    return err;
}
