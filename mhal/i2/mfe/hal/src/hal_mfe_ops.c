#include "hal_mfe_ops.h"

#define _ALIGN(b,x)     (((x)+(1<<(b))-1)&(~((1<<(b))-1)))
#define _BITS_(s)       ((2<<(1?s))-(1<<(0?s)))
#define _MAX_(a,b)      ((a)>(b)?(a):(b))
#define _MIN_(a,b)      ((a)<(b)?(a):(b))

#define LOG2_MAX_FRAME_NUM_DEFAULT  8

#define MVBLK_DEFAULT   \
    (MHVE_INTER_SKIP|MHVE_INTER_16x16|MHVE_INTER_16x8|MHVE_INTER_8x16|\
     MHVE_INTER_8x8|MHVE_INTER_8x4|MHVE_INTER_4x8|MHVE_INTER_4x4)

#define MHVE_FLAGS_CTRL (MHVE_FLAGS_FORCEI|MHVE_FLAGS_DISREF)

#define MB_SIZE     16
#define CROP_UNIT_X 2
#define CROP_UNIT_Y 2

static inline int imin(int a, int b)
{
    return ((a) < (b)) ? (a) : (b);
}

static inline int imax(int a, int b)
{
    return ((a) > (b)) ? (a) : (b);
}

static inline int iClip3(int low, int high, int x)
{
    x = imax(x, low);
    x = imin(x, high);
    return x;
}

//------------------------------------------------------------------------------
//  Function    : _RqctOps
//  Description : Get rqct_ops handle.
//------------------------------------------------------------------------------
static void* _RqctOps(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    return mfe6->p_rqct;
}

//------------------------------------------------------------------------------
//  Function    : _PmbrOps
//  Description : Get pmbr_ops handle.
//------------------------------------------------------------------------------
static void* _PmbrOps(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    return mfe6->p_pmbr;
}

//------------------------------------------------------------------------------
//  Function    : _MfeJob
//  Description : Get mhve_job object.
//------------------------------------------------------------------------------
static void* _MfeJob(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    return mfe6->p_regs;
}

//------------------------------------------------------------------------------
//  Function    : _OpsFree
//  Description : Release this object.
//------------------------------------------------------------------------------
static void _OpsFree(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    rqct_ops* rqct = mfe6->p_rqct;
    pmbr_ops* pmbr = mfe6->p_pmbr;
    mfe6_reg* regs = mfe6->p_regs;
    rqct_ops* rqct_bak = mfe6->p_rqct_bak;
    pmbr_ops* pmbr_bak = mfe6->p_pmbr_bak;

    if(regs)
        MEM_FREE(regs);
    if(rqct)
        rqct->release(rqct);
    if(pmbr)
        pmbr->release(pmbr);
    if(rqct_bak)
        rqct_bak->release(rqct_bak);
    if(pmbr_bak)
        pmbr_bak->release(pmbr_bak);
    MEM_FREE(mops);
}

//------------------------------------------------------------------------------
//  Function    : _SeqDone
//  Description : Finish sequence encoding.
//------------------------------------------------------------------------------
static int _SeqDone(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    rqct_ops* rqct = mfe6->p_rqct;
    pmbr_ops* pmbr = mfe6->p_pmbr;
    int i;

    /* RQCT : Do nothing now */
    rqct->seq_done(rqct);
    /* PMBR : Do nothing now */
    pmbr->seq_done(pmbr);

    for (i = 0; i < mfe6->i_refn; i++)
    {
        mfe6->m_dpbs[AVC_REF_L0][i] = mfe6->m_dpbs[AVC_REF_L1][i] = mfe6->m_dpbs[AVC_REF_LTR][i] = NULL;
    }

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SeqSync
//  Description : Start sequence encoding.
//------------------------------------------------------------------------------
static int _SeqSync(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    rqct_ops* rqct = mfe6->p_rqct;
    pmbr_ops* pmbr = mfe6->p_pmbr;
    mhve_job* mjob = (mhve_job*)mfe6->p_regs;
    sps_t* sps = &mfe6->m_sps;
    pps_t* pps = &mfe6->m_pps;
    slice_t* sh = &mfe6->m_sh;
    int i;

    pps->sps = sps;
    sh->sps = sps;
    sh->pps = pps;

    /* Initialize SPS/VUI parameters */
    sps->i_num_ref_frames = sh->b_long_term_reference ? 2 : 1;//mfe6->i_refn;

    /* Initialize PPS parameters */
    pps->i_pps_id = 0;
    pps->i_num_ref_idx_l0_default_active = sps->i_num_ref_frames;
    pps->i_num_ref_idx_l1_default_active = sps->i_num_ref_frames;
    pps->i_pic_init_qp = 26;
    pps->i_pic_init_qs = 26;
    pps->b_redundant_pic_cnt = 0;

    /* Initialize SLICE parameters */
    sh->i_idr_pid = 0;

    /* RQCT : Reset encoded picture count */
    rqct->seq_sync(rqct);

    /* PMBR : Reset encoded picture count and Initialize global attribute */
    pmbr->seq_sync(pmbr, mjob);

    /* Reset statistic data */
    mfe6->b_seqh = 1;
    mfe6->i_seqn = 0;
    mfe6->i_obits = 0;
    mfe6->i_total = 0;

    /* initialize picture buffer */
    for (i = 0; i < mfe6->i_rpbn; i++)
    {
        mfe6->m_rpbs[i].i_state = RPB_STATE_FREE;
    }
    for (i = mfe6->i_rpbn; i < RPB_MAX_NR; i++)
    {
        mfe6->m_rpbs[i].i_state = RPB_STATE_INVL;
    }

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncBuff
//  Description : Enqueue video buffer.
//                Prepare the SPS/PPS/Slice Header before encode.
//                Reset the Reconstructed/Reference buffer attribute.
//------------------------------------------------------------------------------
static int _EncBuff(mhve_ops* mops, mhve_vpb* mvpb)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    sps_t *sps = &mfe6->m_sps;
    pps_t* pps = &mfe6->m_pps;
    slice_t* sh = &mfe6->m_sh;
    rqct_ops* rqct = mfe6->p_rqct;
    rqct_cfg  rqcf;
    pmbr_ops* pmbr = mfe6->p_pmbr;
    mhve_job* mjob = (mhve_job*)mfe6->p_regs;
    pmbr_cfg  pmbrcfg;
    rpb_t* rpb;
    rqct_buf rqcb;
    bs_t outbs, *bs = &outbs;
    uchar nal;
    int err = 0;
    int i, roiidx, maxqp, minqp;

    /* Link input video buffer */
    mfe6->m_encp = *mvpb;
    mvpb = &mfe6->m_encp;

    if(mfe6->b_seqh)
        rqct->seq_sync(rqct);
    if(mvpb->u_flags & MHVE_FLAGS_FORCEI)
        rqct->seq_sync(rqct);
    if(mvpb->u_flags & MHVE_FLAGS_DISREF)
        rqct->b_unrefp = 1;

    rqcb.u_config = 0;
    /* RQCT : Decide current picture type (I,P,B) */
    if(0 != (err = rqct->enc_buff(rqct, &rqcb)))
        return err;

    if(mfe6->b_seqh)
    {
        static int mbs_level[6][2] = {{30, 40500}, {31, 108000}, {32, 216000}, {40, 245760}, {50, 589824}, {51, 983040}};
        int mbs;

        /* Update SPS/PPS parameters and write to bitstream buffer */
        sps->i_mb_w = mfe6->i_mbsw;
        sps->i_mb_h = mfe6->i_mbsh;
        sps->b_crop = 0;

        if(mfe6->i_pixw & (MB_SIZE - 1) || mfe6->i_pixh & (MB_SIZE - 1))
        {
            sps->b_crop = 1;
            sps->crop.i_left = sps->crop.i_top = 0;
            sps->crop.i_right = ((-mfe6->i_pixw) & (MB_SIZE - 1)) / CROP_UNIT_X;
            sps->crop.i_bottom = ((-mfe6->i_pixh) & (MB_SIZE - 1)) / CROP_UNIT_Y;
        }

        mbs = sps->i_mb_w * sps->i_mb_h;
        mbs = mbs * 30;
        for(i = 0; i < 5 && mbs_level[i][1] < mbs; i++) ;
        sps->i_level_idc = mbs_level[i][0];


        AvcReset(bs, mfe6->m_seqh, 32);
        AvcWriteNAL(bs, (NAL_PRIO_HIGHEST << 5) | NAL_SPS);
        AvcWriteSPS(bs, sps);
        AvcFlush(bs);
        AvcWriteNAL(bs, (NAL_PRIO_HIGHEST << 5) | NAL_PPS);
        AvcWritePPS(bs, pps);   // Add PPS_0
        AvcFlush(bs);
        if(pps->b_cabac_i != pps->b_cabac_p)    // If I/P frame use different entropy type, add PPS_1
        {
            AvcWriteNAL(bs, (NAL_PRIO_HIGHEST << 5) | NAL_PPS);
            AvcWritePPS2nd(bs, pps);
            AvcFlush(bs);
        }
        mfe6->i_seqh = AvcCount(bs);
    }

    /* RQCT : Update QP, MBR, penalty setting */
    rqct->enc_conf(rqct, mjob);

    rqcf.type = RQCT_CFG_QPR;
    rqct->get_rqcf(rqct, &rqcf);

    maxqp = IS_IPIC(rqct->i_pictyp) ? (rqcf.qpr.i_iupperq) : (rqcf.qpr.i_pupperq);
    minqp = IS_IPIC(rqct->i_pictyp) ? (rqcf.qpr.i_ilowerq) : (rqcf.qpr.i_plowerq);

    /* PMBR : If QPMAP enable, disable PMBR */
    rqcf.type = RQCT_CFG_QPM;
    rqct->get_rqcf(rqct, &rqcf);

    //If ROI/QPMAP enabled, disable PMBR
    if(rqcf.qpm.i_qpmenb)
    {
        //Disable PMBR
        pmbrcfg.type = PMBR_CFG_SEQ;
        pmbr->get_conf(pmbr, &pmbrcfg);
        pmbrcfg.seq.i_enable = 0;
        pmbr->set_conf(pmbr, &pmbrcfg);

        rqcf.type = RQCT_CFG_QPM;
        rqct->get_rqcf(rqct, &rqcf);

        pmbr->i_LutRoiQp[i] = 0;

        //Fill ROI QP to PMBR LUT
        for(i = 0; i < 14; i++)
        {
            if(i < 7)
                roiidx = i;
            else
                roiidx = i + 1;
            pmbr->i_LutRoiQp[roiidx] = iClip3(minqp, maxqp, rqct->i_enc_qp + rqcf.qpm.i_entry[i]);
        }

        pmbr->i_LutTyp = 2;
    }
    else
    {
        /* PMBR : If ROI enable, disable PMBR */
        rqcf.type = RQCT_CFG_ROI;
        rqcf.roi.i_roiidx = 0;
        rqct->get_rqcf(rqct, &rqcf);

        if(rqcf.roi.u_roienb)
        {
            //Disable PMBR
            pmbrcfg.type = PMBR_CFG_SEQ;
            pmbr->get_conf(pmbr, &pmbrcfg);
            pmbrcfg.seq.i_enable = 0;
            pmbr->set_conf(pmbr, &pmbrcfg);

            //Fill ROI QP to PMBR LUT
            for(i = 0; i < RQCT_ROI_NR; i++)
            {
                rqcf.type = RQCT_CFG_ROI;
                rqcf.roi.i_roiidx = i;
                rqct->get_rqcf(rqct, &rqcf);

                pmbr->i_LutRoiQp[i] = 0;

                if(rqcf.roi.i_roiqp != 0)
                {
                    roiidx = i == 7 ? 8 : i;

                    if(rqcf.roi.i_absqp)
                        pmbr->i_LutRoiQp[roiidx] = iClip3(minqp, maxqp, rqcf.roi.i_roiqp);
                    else    //Offset QP
                        pmbr->i_LutRoiQp[roiidx] = iClip3(minqp, maxqp, rqct->i_enc_qp + rqcf.roi.i_roiqp);

//                    CamOsPrintf("%d: (%d %d) - (%d %d) = %d\n", roiidx, minqp, maxqp,
//                            rqct->i_enc_qp, rqcf.roi.i_roiqp,
//                            pmbr->i_LutRoiQp[roiidx]);
                }
            }

            pmbr->i_LutTyp = 2;
        }
        else
        {
            pmbrcfg.type = PMBR_CFG_SEQ;
            pmbr->get_conf(pmbr, &pmbrcfg);
            pmbrcfg.seq.i_enable = 0;   //enable PMBR
            pmbr->set_conf(pmbr, &pmbrcfg);
            /* PMBR : Update QP, LUT setting */
            rqcf.type = RQCT_CFG_SEQ;
            rqct->get_rqcf(rqct, &rqcf);
            if(rqcf.seq.i_method == RQCT_MODE_CQP)
            {
                pmbr->i_LutTyp = 1;
            }
            else
            {
                pmbr->i_LutTyp = 0;
            }
        }
    }

    //CamOsPrintf("EncQP:%d\n",rqct->i_enc_qp);

    pmbr->i_FrmQP = rqct->i_enc_qp;
    pmbr->i_PicTyp = rqct->i_pictyp;
    pmbr->i_MaxQP = maxqp;
    pmbr->i_MinQP = minqp;
    pmbr->enc_conf(pmbr, mjob);

    /* Update Slice header parameters and write to bitstream buffer */
    if(IS_IPIC(rqct->i_pictyp))
    {
        sh->b_idr_pic = 1;
        sh->i_ref_idc = NAL_PRIO_HIGHEST;
        sh->b_ref_pic_list_modification_flag_l0 = 0;
        sh->i_type = SLICE_I;
        sh->i_frm_num = 0;
        /* LTR setting */
        if (sh->b_long_term_reference)
        {
            sh->i_idr_pid = (sh->i_idr_pid+1)%2;
            sh->i_num_ref_idx_l0_active = 0;
            sh->i_poc = 0;
        }
        mvpb->u_flags |= MHVE_FLAGS_FORCEI;
        mvpb->u_flags &= ~MHVE_FLAGS_DISREF;
        mfe6->b_seqh = 1;
    }
    else
    {
        sh->b_idr_pic = 0;
        sh->i_frm_num = (sh->i_frm_num+1)%(1<<sps->i_log2_max_frame_num);
        if (sh->b_long_term_reference && IS_LTRPPIC(rqct->i_pictyp))
        {
            sh->i_ref_idc = NAL_PRIO_HIGH;
            sh->i_type = SLICE_P;
            sh->b_ref_pic_list_modification_flag_l0 = 1;
            sh->rpl0_t[0].modification_of_pic_nums_idc = 2;
            sh->rpl0_t[0].long_term_pic_num = 0;
            sh->rpl0_t[1].modification_of_pic_nums_idc = 3;
            /* LTR P ref P */
            if (mfe6->b_enable_pred)
                sh->b_adaptive_ref_pic_marking_mode = 1;
            else
                sh->b_adaptive_ref_pic_marking_mode = 0;
            sh->mmc_t[0].memory_management_control = 5;
            sh->mmc_t[1].memory_management_control = 6;
            sh->mmc_t[1].long_term_frame_idx = 0;
            sh->mmc_t[2].memory_management_control = 0;
        }
        else
        {
            sh->i_ref_idc = (rqct->b_unrefp!=0)?NAL_PRIO_DISPOSED:NAL_PRIO_HIGH;
            sh->i_type = IS_PPIC(rqct->i_pictyp)? SLICE_P : SLICE_B;
            sh->b_ref_pic_list_modification_flag_l0 = 0;
            sh->b_adaptive_ref_pic_marking_mode = 0;
            if (!rqct->b_unrefp)
                mvpb->u_flags &= ~MHVE_FLAGS_DISREF;
        }
    }
    sh->i_qp = rqct->i_enc_qp;

    /* Pick picture buffer for reconstructing */
    rpb = mfe6->p_recn;

    if(!rpb)
    {
        for(i = 0; i <= mfe6->i_refn; i++)
        {
            if(IS_FREE(mfe6->m_rpbs[i].i_state))
                break;
        }
        if (i == mfe6->i_rpbn)
        {
            CamOsPrintf("%s-buf idx overflow(%d)\n", __func__, i);
            return -1;
        }
        rpb = &mfe6->m_rpbs[i];
        /* setup buffer state */
        rpb->i_state = RPB_STATE_BUSY;
        if (sh->b_long_term_reference)
        {
            /* Long term keep buffer */
            if (IS_IPIC(rqct->i_pictyp) || (mfe6->b_enable_pred && IS_LTRPPIC(rqct->i_pictyp)))
                rpb->i_state = RPB_STATE_KEEP;
        }
        mfe6->p_recn = rpb;
    }
    /* not allocate direct memory */
    if(!rpb->b_valid)  // EROY CHECK
    {
        rpb->i_index = mvpb->i_index;
        rpb->u_phys[RPB_YPIX] = mvpb->planes[0].u_phys;
        rpb->u_phys[RPB_CPIX] = mvpb->planes[1].u_phys;
        mvpb->i_index = -1;
    }

    /* prepare slice header */
    nal = (sh->i_ref_idc << 5) | NAL_SLICE;
    if(sh->b_idr_pic)
        nal = (NAL_PRIO_HIGHEST << 5) | NAL_IDR;

    AvcReset(bs, mfe6->m_pich, 32);
    AvcWriteNAL(bs, nal);
    AvcWriteSliceHeader(bs, sh);
    AvcFlush(bs);
    mfe6->i_pich = AvcCount(bs);

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _DeqBuff
//  Description : Dequeue video buffer which is encoded.
//------------------------------------------------------------------------------
static int _DeqBuff(mhve_ops* mops, mhve_vpb* mvpb)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;

    *mvpb = mfe6->m_encp;
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncDone
//  Description : Finish encode one frame.
//                Update statistic data.
//                Update reference buffer list.
//------------------------------------------------------------------------------
static int _EncDone(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    slice_t* sh = &mfe6->m_sh;
    rqct_ops* rqct = mfe6->p_rqct;
    pmbr_ops* pmbr = mfe6->p_pmbr;
    mfe6_reg* regs = mfe6->p_regs;
    mhve_vpb* vpb = &mfe6->m_encp;
    rpb_t* dpb = mfe6->p_recn;
    pmbr_cfg  pmbrcfg;
    int err = 0;
    int i = 0;

#ifdef ENABLE_DUMP_REG
    // Parse dump register here
    mfe6_dump_reg* dump_reg;

    dump_reg = (mfe6_dump_reg *)mfe6->p_drvptr;

    //CamOsPrintf("ENABLE_DUMP_REG(_EncDone)  0x%08X\n", (u32)dump_reg);
    //CamOsPrintf("_EncDone dump_reg->reg1e: 0x%04X\n", dump_reg->reg1e);

    regs->regf5 = dump_reg->regf5;
    regs->regf6 = dump_reg->regf6;
    regs->reg42 = dump_reg->reg42;
    regs->reg43 = dump_reg->reg43;
    regs->reg129 = dump_reg->reg129;
    regs->reg12a_mbr_tc_accum_high = dump_reg->reg12a_lo;

    regs->enc_cycles = ((uint)(regs->reg77 & 0xFF) << 16) + regs->reg76;
    regs->enc_bitcnt = ((uint)(regs->reg43_s_bsp_bit_cnt_hi) << 16) + regs->reg42_s_bsp_bit_cnt_lo;
    regs->enc_sumpqs = ((uint)(regs->regf6_s_mbr_last_frm_avg_qp_hi) << 16) + regs->regf5_s_mbr_last_frm_avg_qp_lo;
    regs->mjob.i_tick = (int)(regs->enc_cycles);
    regs->mjob.i_bits = (int)(regs->enc_bitcnt - regs->bits_delta);
    regs->irq_status  = dump_reg->reg1e;

    //CamOsPrintf("==================================\n");

    for(i = 0; i < PMBR_LUT_SIZE; i++)
    {
        regs->pmbr_lut_hist[i] = dump_reg->reg116[i];
    }

    for(i = 0; i < (1 << PMBR_LOG2_HIST_SIZE); i++)
    {
        regs->pmbr_tc_hist[i] = dump_reg->reg150[i];
        regs->pmbr_pc_hist[i] = dump_reg->reg130[i];

        //CamOsPrintf("regs->pmbr_tc_hist[%d] = %x \n", i, regs->pmbr_tc_hist[i]);
        //CamOsPrintf("regs->pmbr_pc_hist[%d] = %x \n", i, regs->pmbr_pc_hist[i]);
    }

    regs->pmbr_tc_accum = (uint)((dump_reg->reg12a_lo << 16) + dump_reg->reg129);

    memset(dump_reg, 0x00, sizeof(mfe6_dump_reg));
#endif


    if(regs->irq_status & BIT_BUF_FULL ||
            (regs->reg42 == 0 && regs->reg43 == 0))
    {
        MFE_MSG(MFE_MSG_ERR, "> MFE Encode Fail %s:%d  [0x%04X  0x%04X  0x%04X]\n",
                __FUNCTION__, __LINE__,
                regs->irq_status, regs->reg42, regs->reg43);
        return -1;
    }

    //update MBR LUT info to RC
    pmbrcfg.type = PMBR_CFG_LUT;
    pmbr->get_conf(pmbr, &pmbrcfg);

    rqct->auiQps = pmbrcfg.lut.p_kptr[PMBR_LUT_QP];
    rqct->auiBits = pmbrcfg.lut.p_kptr[PMBR_LUT_TARGET];
    rqct->aiIdcHist = pmbrcfg.lut.p_kptr[PMBR_LUT_AIIDCHIST];

    /* PMBR : Update statistic data */
    if((err = pmbr->enc_done(pmbr, &regs->mjob)))
    {
        // TBD
    }

    /* RQCT : Update statistic data */
    if((err = rqct->enc_done(rqct, &regs->mjob)))
    {
        if(!dpb->b_valid)  // EROY CHECK
        {
            vpb->i_index = dpb->i_index;
            dpb->i_index = -1;
        }
        mfe6->i_obits = mfe6->u_oused = 0;
        return err;
    }

    /* Update statistic data */
    mfe6->i_obits = (rqct->i_bitcnt);
    mfe6->i_total += (rqct->i_bitcnt) / 8;
    mfe6->u_oused += (rqct->i_bitcnt + regs->bits_delta) / 8;

    /* Update Ref-list */
    if(sh->i_ref_idc)
    {
        /* Add reconstructed into reference list */
        int b_ltr_keep = 0;
        rpb_t* out = NULL;

        /* get LTR status */
        if (dpb)
            b_ltr_keep = IS_KEEP(dpb->i_state);

        /* update LTR list */
        if (b_ltr_keep)
        {
            /* return previos LTR buffer if encoded picture is LTR frame */
            out = mfe6->m_dpbs[AVC_REF_LTR][0];
            mfe6->m_dpbs[AVC_REF_LTR][0] = dpb;
            sh->i_frm_num = 0;
        }
        else
        {
            /* find the return buffer */
            for (i = 0; i < mfe6->i_rpbn; i++)
            {
                rpb_t* rpb = mfe6->m_rpbs+i;
                if (IS_BUSY(rpb->i_state) && rpb != dpb)
                {
                    out = rpb;
                    break;
                }
            }
        }
        /* update STR list */
        for (i = 0; i < mfe6->i_refn; i++)
        {
            mfe6->m_dpbs[AVC_REF_L0][i] = dpb;
        }
        dpb = out;
        if (++sh->i_num_ref_idx_l0_active > mfe6->i_refn)
            sh->i_num_ref_idx_l0_active = mfe6->i_refn;
    }
    /* return buffer information */
    if(dpb)  // EROY CHECK
    {
        /* shrink mode: find the return buffer index */
        if(!dpb->b_valid)
            vpb->i_index = dpb->i_index;
        dpb->i_index = -1;
        dpb->i_state = RPB_STATE_FREE;
    }

    /* Reset the reconstructed buffer attribute */
    mfe6->p_recn = NULL;
    ++sh->i_poc;
    ++mfe6->i_seqn;
    mfe6->b_seqh = 0;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _OutBuff
//  Description : Output coded buffer.
//------------------------------------------------------------------------------
static int _OutBuff(mhve_ops* mops, mhve_cpb* mcpb)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    mhve_vpb* mvpb = &mfe6->m_encp;
    rqct_ops* rqct = mfe6->p_rqct;
    slice_t* sh = &mfe6->m_sh;
    int err = 0;

    if(mcpb->i_index >= 0)
    {
        mfe6->u_oused = 0;
        mcpb->planes[0].u_phys = 0;
        mcpb->planes[0].i_size = 0;
        mcpb->i_stamp = 0;
        mcpb->i_flags = (MHVE_FLAGS_SOP | MHVE_FLAGS_EOP);
        return err;
    }

    err = mfe6->u_oused;

    /* Update output buffer attribute */
    mcpb->i_index = 0;
    mcpb->planes[0].u_phys = mfe6->u_obase;
    mcpb->planes[0].i_size = err;
    mcpb->i_stamp = mvpb->i_stamp;
    mcpb->i_flags = (MHVE_FLAGS_SOP | MHVE_FLAGS_EOP);
    if(err > 0)
        mcpb->i_flags |= (mvpb->u_flags & MHVE_FLAGS_CTRL); // EROY CHECK
    /* Add flag for LTR P-frame */
    if (sh->b_long_term_reference && IS_LTRPPIC(rqct->i_pictyp))
        mcpb->i_flags |= MVHE_FLAGS_LTR_PFRAME;
    return err;
}

//------------------------------------------------------------------------------
//  Function    : _SeqConf
//  Description : Configure sequence setting.
//------------------------------------------------------------------------------
static int _SeqConf(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    mfe6_reg* regs = mfe6->p_regs;
    pps_t* pps = &mfe6->m_pps;
    slice_t* sh = &mfe6->m_sh;

    /* HW-IP related regs setting */
    // set clk
    regs->regClk_mfe_clock_setting = mfe6->i_ClkEn;
    regs->regClk_clock_source = mfe6->i_ClkSor;
    // pre-fetch
    regs->reg68_s_prfh_cryc_en = 1; // increase the tolerance of DRAM latency
    regs->reg68_s_prfh_refy_en = 1; // increase the tolerance of DRAM latency
    regs->reg6d_s_prfh_cryc_idle_cnt = 0;
    regs->reg6d_s_prfh_refy_idle_cnt = 0;
    // pixel format
    regs->reg03_g_mstar_tile = 1;
    regs->regd6_g_yuvldr_en = 0;
    regs->regf4_s_plnrldr_en = 1;
    // q-table
    regs->reg03_g_qmode = 0;
    // frame coding only
    regs->reg03_g_fldpic_en = 0;
    // NOT SW Buffer Mode
    regs->reg73_g_sw_buffer_mode = 0;
    regs->reg18_g_jpe_buffer_mode = 1;    // frame-mode
    regs->reg18_g_jpe_fsvs_mode = 0;
    regs->reg18_g_viu_soft_rstz = 1;
    // IMI buffer: low bandwidth
    regs->reg68_s_marb_imi_burst_thd = 8;
    regs->reg68_s_marb_imi_timeout = 7;
    regs->reg68_s_marb_imilast_thd = 4;
    // IMI setting
    regs->reg68_s_marb_eimi_block = 0;
    regs->reg68_s_marb_lbwd_mode = 0;
    regs->reg6b_s_marb_imi_saddr_lo = 0;
    regs->reg6c_s_marb_imi_saddr_hi = 0;
#if defined(MMFE_IMI_LBW_ADDR)
    if(mfe6->i_pctw <= 4096)
    {
        regs->reg68_s_marb_eimi_block = 1;
        regs->reg68_s_marb_lbwd_mode = 1;
        regs->reg6b_s_marb_imi_saddr_lo = (MMFE_IMI_LBW_ADDR >> 4) & 0xFFFF;
        regs->reg6c_s_marb_imi_saddr_hi = (MMFE_IMI_LBW_ADDR >> 20) | 0x1000;
        regs->reg6c_s_marb_imi_cache_size = 0; // imi cache size (0: 64kB, 1:32kB, 2:16kB, 3:8kB)
    }
#endif
    // IMI buffer: dbf last-line
#if defined(MMFE_IMI_DBF_ADDR)
    //It must be disable IMI DBF buffer when disable deblocking mode,
    //otherwise P frame has color block issue.
    if(sh->i_disable_deblocking_filter_idc == 1)
    {
        regs->reg4f_s_gn_bwr_mode = 0;
        regs->reg4e_s_gn_mvibuf_saddr_lo = 0;
        regs->reg4f_s_gn_mvibuf_saddr_hi = 0;
    }
    else
    {
        regs->reg4f_s_gn_bwr_mode = 3;
        regs->reg4e_s_gn_mvibuf_saddr_lo = (MMFE_IMI_DBF_ADDR >> 3) & 0xFFFF;
        regs->reg4f_s_gn_mvibuf_saddr_hi = (MMFE_IMI_DBF_ADDR >> 19) | 0x0800;
    }
#else
    regs->reg4f_s_gn_bwr_mode = 0;
    regs->reg4e_s_gn_mvibuf_saddr_lo = 0;
    regs->reg4f_s_gn_mvibuf_saddr_hi = 0;
#endif
    // Frame Coding Only
    regs->reg2a_s_mvdctl_ref0_offset = 0;
    regs->reg2a_s_mvdctl_ref1_offset = 0;
    regs->reg7d_s_txip_eco0 = 0;
    regs->reg7d_s_txip_eco1 = 0;
    regs->reg7d_eco_mreq_stallgo = 1;
    regs->reg7d_eco_marb_stallgo = 0;
    regs->reg7d_reserved1 = 0;
    regs->reg7d_eco_bsp_stuffing = 1;
    regs->reg7d_eco_bsp_rdy_fix = 1;
    regs->reg7d_eco_bsp_multi_slice_fix = 0;

    /* Global regs values (codec related) */
    regs->reg20_s_me_16x16_disable = !(mfe6->i_blkp[0] & MHVE_INTER_16x16);
    regs->reg20_s_me_16x8_disable = !(mfe6->i_blkp[0] & MHVE_INTER_16x8);
    regs->reg20_s_me_8x16_disable = !(mfe6->i_blkp[0] & MHVE_INTER_8x16);
    regs->reg20_s_me_8x8_disable = !(mfe6->i_blkp[0] & MHVE_INTER_8x8);
    regs->reg20_s_me_8x4_disable = !(mfe6->i_blkp[0] & MHVE_INTER_8x4);
    regs->reg20_s_me_4x8_disable = !(mfe6->i_blkp[0] & MHVE_INTER_4x8);
    regs->reg20_s_me_4x4_disable = !(mfe6->i_blkp[0] & MHVE_INTER_4x4);
    regs->reg20_s_mesr_adapt = 1;

    regs->reg21_s_ime_sr16 = (mfe6->i_dmvx <= 16);
    regs->reg21_s_ime_umv_disable = 0;
    regs->reg22_s_ime_mesr_max_addr = (mfe6->i_dmvy == 16 ? 95 : 85);
    regs->reg22_s_ime_mesr_min_addr = (mfe6->i_dmvy == 16 ?  0 : 10);
    regs->reg23_s_ime_mvx_min = _MAX_(-mfe6->i_dmvx + 32, -32 + 32);
    regs->reg23_s_ime_mvx_max = _MIN_(mfe6->i_dmvx + 32, 31 + 31);
    regs->reg24_s_ime_mvy_min = _MAX_(-mfe6->i_dmvy + 16, -16 + 16);
    regs->reg24_s_ime_mvy_max = _MIN_(mfe6->i_dmvy + 16, 16 + 15);

    regs->reg25_s_fme_quarter_disable = (mfe6->i_subp != 2);
    regs->reg25_s_fme_half_disable = (mfe6->i_subp == 0);
    regs->reg25_s_fme_pmv_enable = 0 != (mfe6->i_blkp[0] & MHVE_INTER_SKIP);

    regs->reg2b_s_ieap_constraint_intra = pps->b_constrained_intra_pred;
    regs->reg2b_s_ieap_last_mode = 8;
    regs->reg2b_s_ieap_ccest_en = 1;
    regs->reg2b_s_ieap_ccest_thr = 3;
    regs->reg2b_s_ieap_drop_i16 = 0;
    regs->reg2b_s_ieap_early_termination = 1;
    regs->reg2c_g_ieap_sram_4x2_swap = 1;

    if(mfe6->i_mbsw < 16)
        regs->regfd_eco = 1;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _PutData
//  Description : Put SPS/PPS/User data/Slice header to output buffer before encode start.
//------------------------------------------------------------------------------
static int _PutData(mhve_ops* mops, void* user, int size)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    mfe6_reg* regs = mfe6->p_regs;
    char* src;
    char* dst;
    int  bcnt = 0;
    int  bits = 0;

    if(size > 0)
    {
        if(!mfe6->p_okptr)
        {
            MFE_MSG(MFE_MSG_ERR, "> _PutData p_okptr NULL\n");
            return -1;
        }

        dst = mfe6->p_okptr;

        /* Insert param-sets */
        if(mfe6->b_seqh)
        {
            MEM_COPY(dst, mfe6->m_seqh, mfe6->i_seqh / 8);
            bcnt += mfe6->i_seqh;
            dst += mfe6->i_seqh / 8;
        }

        /* Insert user-data */
        MEM_COPY(dst, user, size);
        bcnt += size * 8;
        dst += size;

        /* Insert slice-header */
        MEM_COPY(dst, mfe6->m_pich, mfe6->i_pich / 8 + 1);
        bits = mfe6->i_pich;
        bcnt += bits;

        mfe6->u_oused = (bcnt >> 11) * 256; // Max 2048 bits(256 Bytes) one time due to bits_coded[256]

        // EROY CHECK : Just copy the rest data bits to bits_coded[], previous 256 Bytes is put in output buffer.
        src = mfe6->p_okptr + mfe6->u_oused;
        dst = (char*)regs->bits_coded;
        MEM_COPY(dst, src, ((bcnt + 7) / 8) & 255);
        regs->bits_count = bcnt & 2047;
        regs->bits_delta = regs->bits_count - bits;

        if (mfe6->flush_cache)
        {
            mfe6->flush_cache((void *)mfe6->p_okptr, (bcnt + 7) / 8);
        }
        else
        {
            CamOsMemFlush((void *)mfe6->p_okptr, (bcnt + 7) / 8);
        }
    }
    else
    {
        dst = (char*)regs->bits_coded;

        /* Insert param-sets */
        if(mfe6->b_seqh)
        {
            MEM_COPY(dst, mfe6->m_seqh, mfe6->i_seqh / 8);
            bcnt += mfe6->i_seqh;
            dst += mfe6->i_seqh / 8;
        }

        /* Insert slice-header */
        MEM_COPY(dst, mfe6->m_pich, mfe6->i_pich / 8 + 1);
        bits = mfe6->i_pich;
        bcnt += bits;

        regs->bits_count = bcnt;
        regs->bits_delta = bcnt - bits;
        mfe6->u_oused = 0; // No data is put in output buffer previously.
    }
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncConf
//  Description : Configure current frame setting.
//------------------------------------------------------------------------------
static int _EncConf(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    mhve_vpb* encp = &mfe6->m_encp;
    mfe6_reg* regs = mfe6->p_regs;
    sps_t* sps = &mfe6->m_sps;
    pps_t* pps = &mfe6->m_pps;
    slice_t* sh = &mfe6->m_sh;
    rpb_t* rpb = NULL;
    uint phys;

    regs->notify_size = mfe6->i_NotifySize;
    regs->notify_func = mfe6->notifyFunc;

    // Codec type (only support h.264)
    regs->reg00_g_enc_mode = MFE_REG_ENC_H264;
    if(sh->i_type == SLICE_I)
    {
        regs->reg03_g_cabac_en = pps->b_cabac_i;
    }
    else
    {
        regs->reg03_g_cabac_en = pps->b_cabac_p;
    }

    switch(mfe6->e_pixf)
    {
        case MHVE_PIX_NV21:
            regs->regf3_s_plnrldr_c_swap = MFE_REG_PLNRLDR_VU;
        case MHVE_PIX_NV12:
            regs->regf3_s_plnrldr_format = MFE_REG_PLNRLDR_420;
            regs->reg90_g_capture_width_y = mfe6->m_encp.i_pitch;
            regs->reg91_g_capture_width_c = mfe6->m_encp.i_pitch >> 1;
            break;
        case MHVE_PIX_YVYU:
            regs->regf3_s_plnrldr_c_swap = MFE_REG_PLNRLDR_VU;
        case MHVE_PIX_YUYV:
            regs->regf3_s_plnrldr_format = MFE_REG_PLNRLDR_422;
            regs->reg90_g_capture_width_y = mfe6->m_encp.i_pitch;
            regs->reg91_g_capture_width_c = 0;
            break;
        default:
            break;
    }

    // Resolution
    regs->reg01_g_pic_width = mfe6->i_pctw;
    regs->reg02_g_pic_height = mfe6->i_pcth;

    // MDC
    regs->reg29_s_mbr_qp_cidx_offset = pps->i_cqp_idx_offset;

    // Clock gating
    regs->reg16 = 0xFFFF;

    // Ref_frames/frame_type
    regs->reg00_g_frame_type = (sh->i_type == SLICE_I) ? MFE_REG_ITYPE : (sh->i_type == SLICE_P) ? MFE_REG_PTYPE : MFE_REG_BTYPE;
    regs->reg00_g_ref_no = (sh->i_num_ref_idx_l0_active == 2) ? MFE_REG_REF_NUM_TWO : MFE_REG_REF_NUM_ONE;

    // Slice parameters
    regs->reg39_s_mdc_h264_nal_ref_idc = sh->i_ref_idc;
    regs->reg39_s_mdc_h264_nal_unit_type = sh->b_idr_pic;
    regs->reg39_s_mdc_h264_fnum_bits = sps->i_log2_max_frame_num - 5;
    regs->reg39_s_mdc_h264_dbf_control = pps->b_deblocking_filter_control;
    regs->reg39_s_mdc_h264_fnum_value = sh->i_frm_num;
    regs->reg3a_s_mdc_h264_idr_pic_id = sh->i_idr_pid;
    regs->reg3a_s_mdc_h264_disable_dbf_idc = sh->i_disable_deblocking_filter_idc==1?1:2;
    regs->reg3a_s_mdc_h264_alpha = sh->i_alpha_c0_offset_div2;
    regs->reg3a_s_mdc_h264_beta = sh->i_beta_offset_div2;
    regs->reg3a_s_mdc_h264_ridx_aor_flag = (sh->i_num_ref_idx_l0_active != sh->pps->i_num_ref_idx_l0_default_active);

    // Buffer setting: current/reference/reconstructed
    phys = encp->planes[0].u_phys + encp->planes[0].i_bias;
    regs->reg06_g_cur_y_addr_lo = (ushort)(phys >> 8);
    regs->reg07_g_cur_y_addr_hi = (ushort)(phys >> 24);
    phys = encp->planes[1].u_phys + encp->planes[1].i_bias;
    regs->reg08_g_cur_c_addr_lo = (ushort)(phys >> 8);
    regs->reg09_g_cur_c_addr_hi = (ushort)(phys >> 24);

    /* setup ref. picture */
    if (sh->b_long_term_reference && sh->b_ref_pic_list_modification_flag_l0)
    {
        rpb = mfe6->m_dpbs[AVC_REF_LTR][0];

        if(!rpb)
            CamOsPrintf("[%s %d]() null ref buf err.\n", __FUNCTION__,__LINE__);
    }
    else if (sh->i_num_ref_idx_l0_active)
    {
        rpb = mfe6->m_dpbs[AVC_REF_L0][0];
    }

    if (rpb)
    {
        regs->reg0a_g_ref_y_addr0_lo = (ushort)(rpb->u_phys[RPB_YPIX]>> 8);
        regs->reg0b_g_ref_y_addr0_hi = (ushort)(rpb->u_phys[RPB_YPIX]>>24);
        regs->reg0e_g_ref_c_addr0_lo = (ushort)(rpb->u_phys[RPB_CPIX]>> 8);
        regs->reg0f_g_ref_c_addr0_hi = (ushort)(rpb->u_phys[RPB_CPIX]>>24);

        //CamOsPrintf("[%s %d] Ref Buf Y: 0x%08X, C: 0x%08X\n",__FUNCTION__,__LINE__,rpb->u_phys[RPB_YPIX], rpb->u_phys[RPB_CPIX]);
    }
    else if (!rpb && sh->i_type != SLICE_I)
    {
        CamOsPrintf("%s() null ref buf err.\n", __func__);
        return -1;
    }

    regs->reg12_g_rec_y_addr_lo = (ushort)(mfe6->p_recn->u_phys[RPB_YPIX] >> 8);
    regs->reg13_g_rec_y_addr_hi = (ushort)(mfe6->p_recn->u_phys[RPB_YPIX] >> 24);
    regs->reg14_g_rec_c_addr_lo = (ushort)(mfe6->p_recn->u_phys[RPB_CPIX] >> 8);
    regs->reg15_g_rec_c_addr_hi = (ushort)(mfe6->p_recn->u_phys[RPB_CPIX] >> 24);

    //CamOsPrintf("[%s %d] Rec Buf Y: 0x%08X, C: 0x%08X\n",__FUNCTION__,__LINE__, mfe6->p_recn->u_phys[RPB_YPIX], mfe6->p_recn->u_phys[RPB_CPIX]);

    // Output buffers: Must be 8-byte aligned.
    regs->reg3f_s_bspobuf_hw_en = 0;

    // GN
#if defined(MMFE_IMI_GN_ADDR)
    regs->reg4c_s_gn_saddr_lo = (ushort)(MMFE_IMI_GN_ADDR >> 3);
    regs->reg4d_s_gn_saddr_hi = (ushort)((MMFE_IMI_GN_ADDR >> 19) | 0x0800);
    regs->reg4d_s_gn_saddr_mode = 0; // 0: gn data row, 1: gn data frame.
#else
    regs->reg4c_s_gn_saddr_lo = (ushort)(mfe6->u_mbp_base >> 3);
    regs->reg4d_s_gn_saddr_hi = (ushort)(mfe6->u_mbp_base >> 19);
    regs->reg4d_s_gn_saddr_mode = 0; // 0: gn data row, 1: gn data frame.
#endif

    // Motion search
    regs->reg20_s_me_ref_en_mode = (1 << sh->i_num_ref_idx_l0_active) - 1;

    // FME
    regs->reg21_s_ime_ime_wait_fme = 1;
    regs->reg25_s_fme_pipeline_on = regs->reg21_s_ime_ime_wait_fme ? 1 : 0;

    // P8x8 BOUND RECT
    regs->reg21_s_ime_boundrect_en = (sps->i_profile_idc == MHVE_AVC_PROFILE_BP && sps->i_level_idc <= 30);
    regs->reg21_s_ime_h264_p8x8_ctrl_en = 0;//mfeinfo->i_p8x8_max_count < mfe6->i_mb_wxh ? 1 : 0;

    regs->reg25_s_fme_mode0_refno = 1;
    regs->reg25_s_fme_mode1_refno = 1;
    regs->reg25_s_fme_mode2_refno = 1;
    regs->reg25_s_fme_mode_no = (mfe6->i_mbsn <= 396);

    // Intra update (force disabled)
    regs->reg2f_s_txip_irfsh_en = 0;
    regs->reg2c_s_quan_idx_last = 63;
    if(regs->reg2c_s_quan_idx_last < 63)
        regs->reg2c_s_quan_idx_swlast = 1;
    else
        regs->reg2c_s_quan_idx_swlast = 0;

    // Multi-slice mode
    if(mfe6->i_rows)
    {
        regs->regca_s_multislice_en = mfe6->i_rows > 0;
        regs->regcb_s_multislice_1st_mby = mfe6->i_rows;
        regs->regca_s_multislice_mby = mfe6->i_rows;

        if(sh->i_disable_deblocking_filter_idc == 0)
        {
            sh->i_disable_deblocking_filter_idc = 2;
        }

        regs->reg03_g_er_mode = 0;
        regs->reg04_g_er_bs_th = 0;
        regs->reg03_g_er_mby = 0;
    }
    else
    {
        regs->regca_s_multislice_en = 0;
        regs->regcb_s_multislice_1st_mby = 0;
        regs->regca_s_multislice_mby = 0;

        regs->reg03_g_er_mode = 3;
        regs->reg04_g_er_bs_th = 0;
        regs->reg03_g_er_mby = 0;
    }

    // Set POC
    regs->regf7_s_mdc_h264_poc = sh->i_poc;
    regs->regf8_s_mdc_h264_poc_enable = (sps->i_poc_type == 0 ? 1 : 0);
    regs->regf8_s_mdc_h264_poc_width = sps->i_log2_max_poc_lsb;

    if(regs->regf2_g_roi_en != 0)
        regs->reg00_g_mbr_en = 0;
    regs->reg00_g_rec_en = 1;

    // Set Output buffer attribute
    regs->coded_data = regs->bits_coded;
    regs->coded_bits = regs->bits_count;
    regs->outbs_addr = mfe6->u_obase + mfe6->u_oused;
    if(mfe6->i_iframe_bitsthr && (sh->i_type == SLICE_I) && (mfe6->u_osize > mfe6->i_iframe_bitsthr))
        mfe6->u_osize = mfe6->i_iframe_bitsthr;
    if(mfe6->i_pframe_bitsthr && (sh->i_type == SLICE_P) && (mfe6->u_osize > mfe6->i_pframe_bitsthr))
        mfe6->u_osize = mfe6->i_pframe_bitsthr;
    regs->outbs_size = mfe6->u_osize - mfe6->u_oused;

    // Set Dump Register
    regs->rega7_s_dump_reg_sadr_low = (ushort)(mfe6->p_drpptr >> 3);
    regs->rega8_s_dump_reg_sadr_high = (ushort)(mfe6->p_drpptr >> 19);
    regs->rega9_s_dump_reg_en = 1;

    // Set CmdQ IRQ
    regs->reg17b = 0x00FF;
    regs->reg17c = 0xFFFF;

#ifdef ENABLE_DUMP_REG
    regs->dump_reg_vaddr = mfe6->p_drvptr;
#endif

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SetConf
//  Description : Apply configure.
//------------------------------------------------------------------------------
static int _SetConf(mhve_ops* mops, mhve_cfg* mcfg)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    sps_t* sps;
    pps_t* pps;
    int err = -1;

    switch(mcfg->type)
    {
        case MHVE_CFG_RES:
            if((unsigned)mcfg->res.e_pixf <= MHVE_PIX_YVYU)
            {
                mfe6->e_pixf = mcfg->res.e_pixf;
                mfe6->i_pixw = mcfg->res.i_pixw;
                mfe6->i_pixh = mcfg->res.i_pixh;
                mfe6->i_pctw = _ALIGN(4, mcfg->res.i_pixw);
                mfe6->i_pcth = _ALIGN(4, mcfg->res.i_pixh);
                mfe6->i_rpbn = mcfg->res.i_rpbn;
                mfe6->u_conf = mcfg->res.u_conf;
                mfe6->i_mbsw = mfe6->i_pctw >> 4;
                mfe6->i_mbsh = mfe6->i_pcth >> 4;
                mfe6->i_mbsn = mfe6->i_mbsw * mfe6->i_mbsh;
                mfe6->b_seqh = 1;
                mfe6->i_ClkEn = mcfg->res.i_ClkEn;
                mfe6->i_ClkSor = mcfg->res.i_ClkSor;
                mfe6->i_NotifySize = mcfg->res.i_NotifySize;
                mfe6->notifyFunc = mcfg->res.notifyFunc;
                err = 0;
            }
            break;
        case MHVE_CFG_DMA:
            if(mcfg->dma.i_dmem >= 0)
            {
                uint addr = mcfg->dma.u_phys;
                int i = mcfg->dma.i_dmem;
                rpb_t* ref = mfe6->m_rpbs + i;

                ref->i_index = -1;
                ref->i_state = RPB_STATE_FREE;
                ref->u_phys[RPB_YPIX] = !mcfg->dma.i_size[0] ? 0 : addr;
                addr += mcfg->dma.i_size[0];
                ref->u_phys[RPB_CPIX] = !mcfg->dma.i_size[1] ? 0 : addr;
                addr += mcfg->dma.i_size[1];
                ref->b_valid = ref->u_phys[RPB_YPIX] != 0;
                err = 0;
            }
            else if(mcfg->dma.i_dmem == MHVE_CFG_DMA_OUTPUT_BUFFER)
            {
                mfe6->p_okptr = mcfg->dma.p_vptr;
                mfe6->u_obase = mcfg->dma.u_phys;
                mfe6->u_osize = mcfg->dma.i_size[0];
                mfe6->u_oused = 0;
                mfe6->flush_cache = mcfg->dma.pFlushCacheCb;
                err = 0;
            }
            else if(mcfg->dma.i_dmem == MHVE_CFG_DMA_NALU_BUFFER)
            {
                mfe6->u_mbp_base = mcfg->dma.u_phys;
                err = 0;
            }
            break;
        case MHVE_CFG_MOT:
            mfe6->i_subp = mcfg->mot.i_subp;
            mfe6->i_dmvx = mcfg->mot.i_dmvx;
            mfe6->i_dmvy = mcfg->mot.i_dmvy;
            mfe6->i_blkp[0] = mcfg->mot.i_blkp[0];
            mfe6->i_blkp[1] = 0;
            err = 0;
            break;
        case MHVE_CFG_AVC:
        {
            slice_t* sh = &mfe6->m_sh;

            sps = &mfe6->m_sps;
            pps = &mfe6->m_pps;

            mfe6->i_refn = 1;

            sps->i_profile_idc = MHVE_AVC_PROFILE_MP;
            if(mcfg->avc.i_profile < MHVE_AVC_PROFILE_MP)
            {
                sps->i_profile_idc = MHVE_AVC_PROFILE_BP;
                pps->b_cabac_i = 0;
                pps->b_cabac_p = 0;
            }

            if(mcfg->avc.i_level < MHVE_AVC_LEVEL_3)
                sps->i_level_idc = MHVE_AVC_LEVEL_3;
            if(mcfg->avc.i_level > MHVE_AVC_LEVEL_4)
                sps->i_level_idc = MHVE_AVC_LEVEL_4;

            sps->i_poc_type = mcfg->avc.i_poc_type == 0 ? 0 : 2;
            pps->b_cabac_i = mcfg->avc.b_entropy_coding_type_i != 0;
            pps->b_cabac_p = mcfg->avc.b_entropy_coding_type_p != 0;
            pps->b_deblocking_filter_control = mcfg->avc.b_deblock_filter_control;
            pps->b_constrained_intra_pred = mcfg->avc.b_constrained_intra_pred;
            pps->i_cqp_idx_offset = mcfg->avc.i_chroma_qp_index_offset;
            sh->i_disable_deblocking_filter_idc = mcfg->avc.i_disable_deblocking_idc;
            sh->i_alpha_c0_offset_div2 = mcfg->avc.i_alpha_c0_offset;
            sh->i_beta_offset_div2 = mcfg->avc.i_beta_offset;
            mfe6->b_seqh = 1;
            err = 0;
        }
        break;
        case MHVE_CFG_VUI:
        {
            sps_t* sps = &mfe6->m_sps;

            sps->b_vui_param_pres = 1;

            sps->vui.b_aspect_ratio_info_present_flag = mcfg->vui.b_aspect_ratio_info_present_flag;
            sps->vui.i_aspect_ratio_idc = mcfg->vui.i_aspect_ratio_idc;
            sps->vui.i_sar_w = mcfg->vui.i_sar_w;
            sps->vui.i_sar_h = mcfg->vui.i_sar_h;
            sps->vui.b_overscan_info_present_flag = mcfg->vui.b_overscan_info_present_flag;
            sps->vui.b_overscan_appropriate_flag = mcfg->vui.b_overscan_appropriate_flag;
            sps->vui.b_video_signal_pres = mcfg->vui.b_video_signal_pres;
            sps->vui.i_video_format = mcfg->vui.i_video_format;
            sps->vui.b_video_full_range = mcfg->vui.b_video_full_range;
            sps->vui.b_colour_desc_pres = mcfg->vui.b_colour_desc_pres;
            sps->vui.i_colour_primaries = mcfg->vui.i_colour_primaries;
            sps->vui.i_transf_character = mcfg->vui.i_transf_character;
            sps->vui.i_matrix_coeffs = mcfg->vui.i_matrix_coeffs;
            sps->vui.b_timing_info_pres = mcfg->vui.b_timing_info_pres;
            sps->vui.i_num_units_in_tick = mcfg->vui.i_num_units_in_tick;
            sps->vui.i_time_scale = mcfg->vui.i_time_scale;
            sps->vui.b_fixed_frame_rate = mcfg->vui.b_fixed_frame_rate_flag;

            mfe6->b_seqh = 1;
            err = 0;
        }
        break;
        case MHVE_CFG_SPL:
            //check profile
            sps = &mfe6->m_sps;
            pps = &mfe6->m_pps;
            if(sps->i_profile_idc != MHVE_AVC_PROFILE_MP && pps->b_cabac_i != 1 && pps->b_cabac_p != 1)
            {
                err = -1;
                break;
            }

            mfe6->i_bits = mcfg->spl.i_bits;
            mfe6->i_rows = mcfg->spl.i_rows;

            sps = &mfe6->m_sps;
            pps = &mfe6->m_pps;

            err = 0;
            break;
        case MHVE_CFG_LTR:
            {
                slice_t* sh = &mfe6->m_sh;
                sps_t* sps = &mfe6->m_sps;
                if (mcfg->ltr.b_long_term_reference)
                {
                    /* NALU setting */
                    sh->b_long_term_reference = 1;
                    sh->b_num_ref_idx_override = 1;
                    sps->i_log2_max_frame_num = LOG2_MAX_FRAME_NUM_DEFAULT+2;
                    sps->b_gaps_in_frame_num_value_allow = 1;
                    mfe6->b_enable_pred = mcfg->ltr.b_enable_pred;
                }
                else
                {
                    /* NALU setting */
                    sh->b_long_term_reference = 0;
                    sh->b_num_ref_idx_override = 0;
                    sps->i_log2_max_frame_num = LOG2_MAX_FRAME_NUM_DEFAULT;
                    sps->b_gaps_in_frame_num_value_allow = 0;
                    mfe6->b_enable_pred = 0;
                }
                err = 0;
            }
            break;
        case MHVE_CFG_DUMP_REG:
            mfe6->p_drpptr = (uint)mcfg->dump_reg.u_phys;
            mfe6->p_drvptr = (uint)mcfg->dump_reg.p_vptr;
            mfe6->u_drsize = mcfg->dump_reg.i_size;
            err = 0;
            break;
        case MHVE_CFG_FME:
            mfe6->i_iframe_bitsthr = mcfg->fme.i_iframe_bitsthr;
            mfe6->i_pframe_bitsthr = mcfg->fme.i_pframe_bitsthr;
            err = 0;
            break;
        default:
            break;
    }
    return err;
}

//------------------------------------------------------------------------------
//  Function    : _GetConf
//  Description : Query configuration.
//------------------------------------------------------------------------------
static int _GetConf(mhve_ops* mops, mhve_cfg* mcfg)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    int err = -1;

    switch(mcfg->type)
    {
        case MHVE_CFG_RES:
            mcfg->res.e_pixf = mfe6->e_pixf;
            mcfg->res.i_pixw = mfe6->i_pixw;
            mcfg->res.i_pixh = mfe6->i_pixh;
            mcfg->res.i_rpbn = mfe6->i_rpbn;
            mcfg->res.u_conf = mfe6->u_conf;
            mcfg->res.i_NotifySize = mfe6->i_NotifySize;
            mcfg->res.notifyFunc = mfe6->notifyFunc;
            err = 0;
            break;
        case MHVE_CFG_MOT:
            mcfg->mot.i_subp = mfe6->i_subp;
            mcfg->mot.i_dmvx = mfe6->i_dmvx;
            mcfg->mot.i_dmvy = mfe6->i_dmvy;
            mcfg->mot.i_blkp[0] = mfe6->i_blkp[0];
            mcfg->mot.i_blkp[1] = 0;
            err = 0;
            break;
        case MHVE_CFG_AVC:
        {
            sps_t* sps = &mfe6->m_sps;
            pps_t* pps = &mfe6->m_pps;
            slice_t* sh = &mfe6->m_sh;

            mcfg->avc.i_profile = sps->i_profile_idc;
            mcfg->avc.i_level = sps->i_level_idc;
            mcfg->avc.i_num_ref_frames = mfe6->i_refn;
            mcfg->avc.i_poc_type = sps->i_poc_type;
            mcfg->avc.b_entropy_coding_type_i = pps->b_cabac_i;
            mcfg->avc.b_entropy_coding_type_p = pps->b_cabac_p;
            mcfg->avc.b_deblock_filter_control = pps->b_deblocking_filter_control;
            mcfg->avc.b_constrained_intra_pred = pps->b_constrained_intra_pred;
            mcfg->avc.i_chroma_qp_index_offset = pps->i_cqp_idx_offset;
            mcfg->avc.i_disable_deblocking_idc = sh->i_disable_deblocking_filter_idc;
            mcfg->avc.i_alpha_c0_offset = sh->i_alpha_c0_offset_div2;
            mcfg->avc.i_beta_offset = sh->i_beta_offset_div2;
            err = 0;
        }
        break;
        case MHVE_CFG_VUI:
        {
            sps_t* sps = &mfe6->m_sps;
            mcfg->vui.b_aspect_ratio_info_present_flag = sps->vui.b_aspect_ratio_info_present_flag;
            mcfg->vui.i_aspect_ratio_idc = sps->vui.i_aspect_ratio_idc;
            mcfg->vui.i_sar_w = sps->vui.i_sar_w;
            mcfg->vui.i_sar_h = sps->vui.i_sar_h;
            mcfg->vui.b_overscan_info_present_flag = sps->vui.b_overscan_info_present_flag;
            mcfg->vui.b_overscan_appropriate_flag = sps->vui.b_overscan_appropriate_flag;
            mcfg->vui.b_video_full_range = sps->vui.b_video_full_range;
            mcfg->vui.b_video_signal_pres = sps->vui.b_video_signal_pres;
            mcfg->vui.i_video_format = sps->vui.i_video_format;
            mcfg->vui.b_colour_desc_pres = sps->vui.b_colour_desc_pres;
            mcfg->vui.i_colour_primaries = sps->vui.i_colour_primaries;
            mcfg->vui.i_transf_character = sps->vui.i_transf_character;
            mcfg->vui.i_matrix_coeffs = sps->vui.i_matrix_coeffs;
            mcfg->vui.b_timing_info_pres = sps->vui.b_timing_info_pres;
            mcfg->vui.i_num_units_in_tick = sps->vui.i_num_units_in_tick;
            mcfg->vui.i_time_scale = sps->vui.i_time_scale;
            mcfg->vui.b_fixed_frame_rate_flag = sps->vui.b_fixed_frame_rate;
            err = 0;
        }
        break;
        case MHVE_CFG_SPL:
            mcfg->spl.i_rows = mfe6->i_rows;
            mcfg->spl.i_bits = mfe6->i_bits;
            err = 0;
            break;
        case MHVE_CFG_LTR:
            {
                slice_t* sh = &mfe6->m_sh;
                if (sh->b_long_term_reference)
                {
                    mcfg->ltr.b_long_term_reference = 1;
                    mcfg->ltr.b_enable_pred = mfe6->b_enable_pred;
                }
                else
                {
                    mcfg->ltr.b_long_term_reference = 0;
                    mcfg->ltr.b_enable_pred = 0;
                }
                err = 0;
            }
            break;
        case MHVE_CFG_FME:
            mcfg->fme.i_iframe_bitsthr = mfe6->i_iframe_bitsthr;
            mcfg->fme.i_pframe_bitsthr = mfe6->i_pframe_bitsthr;
            err = 0;
            break;
        default:
            break;
    }
    return err;
}

//------------------------------------------------------------------------------
//  Function    : _OpsBackup
//  Description : Backup this object.
//------------------------------------------------------------------------------
static int _OpsBackup(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    slice_t* sh = &mfe6->m_sh;
    slice_t* sh_bak = &mfe6->m_sh_bak;

    rqct_ops* rqct = mfe6->p_rqct;
    rqct_ops* rqct_bak = mfe6->p_rqct_bak;
    //pmbr_ops* pmbr = mfe6->p_pmbr;
    //pmbr_ops* pmbr_bak = mfe6->p_pmbr_bak;

    /* backup rec & ref buffer */
    mfe6->p_recn_bak =  mfe6->p_recn;
    MEM_COPY(&mfe6->m_dpbs_bak[0][0], &mfe6->m_dpbs[0][0], sizeof(rpb_t*)*AVC_REF_MAX*RPB_MAX_NR);
    MEM_COPY(&mfe6->m_rpbs_bak[0], &mfe6->m_rpbs[0], sizeof(rpb_t)*RPB_MAX_NR);

    /* backup sh */
    MEM_COPY(sh_bak, sh, sizeof(slice_t));

    /* backup rqcf */
     rqct->cpy_conf(rqct_bak,rqct);     // src:rqct , dst : rqct_bak

    /* backup pmbr */
    //MEM_COPY(pmbr_bak, pmbr, sizeof(pmbr_ops));

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _OpsRestore
//  Description : Restore this object.
//------------------------------------------------------------------------------
static int _OpsRestore(mhve_ops* mops)
{
    mfe6_ops* mfe6 = (mfe6_ops*)mops;
    slice_t* sh = &mfe6->m_sh;
    slice_t* sh_bak = &mfe6->m_sh_bak;

    rqct_ops* rqct = mfe6->p_rqct;
    rqct_ops* rqct_bak = mfe6->p_rqct_bak;
    //pmbr_ops* pmbr = mfe6->p_pmbr;
    //pmbr_ops* pmbr_bak = mfe6->p_pmbr_bak;

    /* restore rec & ref buffer */
    mfe6->p_recn = mfe6->p_recn_bak;
    MEM_COPY(&mfe6->m_dpbs[0][0], &mfe6->m_dpbs_bak[0][0],  sizeof(rpb_t*)*AVC_REF_MAX*RPB_MAX_NR);
    MEM_COPY(&mfe6->m_rpbs[0], &mfe6->m_rpbs_bak[0], sizeof(rpb_t)*RPB_MAX_NR);

    /* restore sh */
    MEM_COPY(sh, sh_bak, sizeof(slice_t));

    /* restore rqcf */
    rqct->cpy_conf(rqct,rqct_bak);     // // src:rqct_bak , dst : rqct

    /* restore pmbr */
    //MEM_COPY(pmbr, pmbr_bak, sizeof(pmbr_ops));

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : MfeOpsAcquire
//  Description : Allocate mfe6_ops object and link its member function.
//------------------------------------------------------------------------------
void* MfeOpsAcquire(int rqc_id)
{
    mhve_ops* mops = NULL;

    while(NULL != (mops = MEM_ALLC(sizeof(mfe6_ops))))
    {
        mfe6_ops* mfe6 = (mfe6_ops*)mops;
        sps_t* sps = &mfe6->m_sps;
        pps_t* pps = &mfe6->m_pps;
        slice_t* sh = &mfe6->m_sh;

        /* Link member function */
        mops->release  = _OpsFree;
        mops->rqct_ops = _RqctOps;
        mops->pmbr_ops = _PmbrOps;
        mops->mhve_job = _MfeJob;
        mops->seq_sync = _SeqSync;
        mops->seq_conf = _SeqConf;
        mops->seq_done = _SeqDone;
        mops->enc_buff = _EncBuff;
        mops->deq_buff = _DeqBuff;
        mops->put_data = _PutData;
        mops->enc_conf = _EncConf;
        mops->enc_done = _EncDone;
        mops->out_buff = _OutBuff;
        mops->set_conf = _SetConf;
        mops->get_conf = _GetConf;
        mops->backup = _OpsBackup;
        mops->restore = _OpsRestore;

        /* Link RQCT and allocate register mirror structure */
        mfe6->p_rqct = RqctMfeAcquire(rqc_id);
        mfe6->p_pmbr = PmbrAllocate();
        mfe6->p_regs = MEM_ALLC(sizeof(mfe6_reg));
        if(!mfe6->p_regs || !mfe6->p_rqct || !mfe6->p_pmbr)
            break;

        mfe6->p_rqct_bak = RqctMfeAcquire(rqc_id);
        mfe6->p_pmbr_bak = PmbrAllocate();
        if(!mfe6->p_rqct_bak || !mfe6->p_pmbr_bak)
            break;

        /* Initialize common parameters */
        mfe6->i_refn = 1;
        mfe6->i_dmvx = 32;
        mfe6->i_dmvy = 16;
        mfe6->i_subp = 2;
        mfe6->i_blkp[0] = MVBLK_DEFAULT;
        mfe6->i_blkp[1] = 0;
        mfe6->notifyFunc = NULL;
        mfe6->flush_cache = NULL;

        /* Initialize SPS parameters */
        sps->i_profile_idc = MHVE_AVC_PROFILE_MP;
        sps->i_level_idc = 30;
        sps->b_constraint_set0 = 0;
        sps->b_constraint_set1 = 1;
        sps->b_constraint_set2 = 0;
        sps->b_constraint_set3 = 0;
        sps->i_log2_max_frame_num = 5;
        sps->i_poc_type = 2;
        sps->i_log2_max_poc_lsb = 4; // not used for poc_type = 2
        sps->i_num_ref_frames = mfe6->i_refn;
        sps->b_gaps_in_frame_num_value_allow = 0;
        sps->b_direct8x8_inference = 1;
        sps->b_crop = 0;
        sps->b_vui_param_pres = 0;
        sps->vui.b_aspect_ratio_info_present_flag = 0;
        sps->vui.i_aspect_ratio_idc = 1;
        sps->vui.i_sar_w = 1;
        sps->vui.i_sar_h = 1;
        sps->vui.b_overscan_info_present_flag = 0;
        sps->vui.b_overscan_appropriate_flag = 0;
        sps->vui.b_video_signal_pres = 1;
        sps->vui.i_video_format = 5;
        sps->vui.b_video_full_range = 1;
        sps->vui.b_colour_desc_pres = 1;
        sps->vui.i_colour_primaries = 1;
        sps->vui.i_transf_character = 1;
        sps->vui.i_matrix_coeffs = 1;
        sps->vui.b_timing_info_pres = 0;
        sps->vui.i_num_units_in_tick = 1;
        sps->vui.i_time_scale = 60;
        sps->vui.b_fixed_frame_rate = 1;

        /* Initialize PPS parameters */
        pps->i_pps_id = 0;
        pps->b_cabac_i = 1;
        pps->b_cabac_p = 1;
        pps->i_num_ref_idx_l0_default_active = sps->i_num_ref_frames;
        pps->i_num_ref_idx_l1_default_active = sps->i_num_ref_frames;
        pps->i_pic_init_qp = 26;
        pps->i_pic_init_qs = 26;
        pps->i_cqp_idx_offset = 0;
        pps->b_redundant_pic_cnt = 0;
        pps->b_deblocking_filter_control = 1;
        pps->b_constrained_intra_pred = 0;

        /* Initialize Slice-header */
        sh->i_disable_deblocking_filter_idc = 0;
        sh->i_alpha_c0_offset_div2 = 0;
        sh->i_beta_offset_div2 = 0;

        return mops;
    }

    if(mops)
        mops->release(mops);

    return NULL;
}
