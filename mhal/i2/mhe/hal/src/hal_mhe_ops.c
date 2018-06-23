#include "hal_mhe_ops.h"

#define _ALIGN(b,x)     (((x)+(1<<(b))-1)&(~((1<<(b))-1)))
#define _BITS_(s)       ((2<<(1?s))-(1<<(0?s)))
#define _MAX_(a,b)      ((a)>(b)?(a):(b))
#define _MIN_(a,b)      ((a)<(b)?(a):(b))

#define MVBLK_DEFAULT   \
    (MHVE_INTER_SKIP|MHVE_INTER_16x16|MHVE_INTER_16x8|MHVE_INTER_8x16|\
     MHVE_INTER_8x8|MHVE_INTER_8x4|MHVE_INTER_4x8|MHVE_INTER_4x4)

#define MHVE_FLAGS_CTRL (MHVE_FLAGS_FORCEI|MHVE_FLAGS_DISREF)

#define CTB_SIZE    32
#define CROP_UNIT_X 2
#define CROP_UNIT_Y 2

#define INT_PREC        0
#define HALF_PREC       1
#define QUARTER_PREC    2

// Mode penalties when RDO cost comparison
//#define PENALTY_BITS   (16)
#define PENALTY_SHIFT  (2)

#if 0
#define HW_ME_WINDOW_X  (32)
#define HW_ME_WINDOW_Y  (16)
#else
#define HW_ME_WINDOW_X  (96)
#define HW_ME_WINDOW_Y  (48)
#endif

#define HEV_ME_4X4_DIS    (1<<0)
#define HEV_ME_8X4_DIS    (1<<1)
#define HEV_ME_4X8_DIS    (1<<2)
#define HEV_ME_16X8_DIS   (1<<3)
#define HEV_ME_8X16_DIS   (1<<4)
#define HEV_ME_8X8_DIS    (1<<5)
#define HEV_ME_16X16_DIS  (1<<6)

// These are only used when g_bNewMSME=1
// Define the center "static" area
#define HW_ME_STATIC_X   (16)
#define HW_ME_STATIC_Y   (16)

#ifdef MODULE_PARAM_SUPPORT
extern unsigned int tmvp_enb;
#endif

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
    mhe_ops* mhe = (mhe_ops*)mops;
    return mhe->p_rqct;
}

//------------------------------------------------------------------------------
//  Function    : _PmbrOps
//  Description : Get pmbr_ops handle.
//------------------------------------------------------------------------------
static void* _PmbrOps(mhve_ops* mops)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    return mhe->p_pmbr;
}

//------------------------------------------------------------------------------
//  Function    : _MheJob
//  Description : Get mhve_job object.
//------------------------------------------------------------------------------
static void* _MheJob(mhve_ops* mops)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    return mhe->p_regs;
}

//------------------------------------------------------------------------------
//  Function    : __OpsFree
//  Description : Release this object.
//------------------------------------------------------------------------------
static void __OpsFree(mhve_ops* mops)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    rqct_ops* rqct = mhe->p_rqct;
    pmbr_ops* pmbr = mhe->p_pmbr;
    mhe_reg* regs = mhe->p_regs;
    h265_enc* h265 = mhe->h265;
    h265_enc* h265_bak = mhe->h265_bak;
    rqct_ops* rqct_bak = mhe->p_rqct_bak;
    pmbr_ops* pmbr_bak = mhe->p_pmbr_bak;

    if(regs)
        MEM_FREE(regs);
    if(rqct)
        rqct->release(rqct);
    if(h265)
        h265->release(h265);
    if(pmbr)
        pmbr->release(pmbr);
    if(rqct_bak)
        rqct_bak->release(rqct_bak);
    if(h265_bak)
        h265_bak->release(h265_bak);
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
    mhe_ops* mhe = (mhe_ops*)mops;
    rqct_ops* rqct = mhe->p_rqct;
    pmbr_ops* pmbr = mhe->p_pmbr;

    /* RQCT : Do nothing now */
    rqct->seq_done(rqct);
    /* PMBR : Do nothing now */
    pmbr->seq_done(pmbr);

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SeqSync
//  Description : Start sequence encoding.
//------------------------------------------------------------------------------
static int _SeqSync(mhve_ops* mops)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    rqct_ops* rqct = mhe->p_rqct;
    pmbr_ops* pmbr = mhe->p_pmbr;
    mhve_job* mjob = (mhve_job*)mhe->p_regs;
    h265_enc* h265 = mhe->h265;
    rqct_cfg  rqcf;
    int err = 0;

    /* Initialize H265 Header parameters */
    if(0 != (err = h265_seq_init(h265, mhe->i_rpbn)))
    {
        mops->seq_done(mops);

        return err;
    }
    mhe->i_seqn = 0;
    mhe->b_seqh = OPS_SEQH_START;

    /* RQCT : Reset encoded picture count */
    rqct->seq_sync(rqct);

    /* PMBR : Reset encoded picture count and Initialize global attribute */
    pmbr->seq_sync(pmbr, mjob);

    /* Reset statistic data */
    mhe->i_obits = 0;
    mhe->i_total = 0;

    rqcf.type = RQCT_CFG_SEQ;
    rqct->get_rqcf(rqct, &rqcf);


    return err;

}

//------------------------------------------------------------------------------
//  Function    : _EncBuff
//  Description : Enqueue video buffer.
//                Prepare the VPS/SPS/PPS/Slice Header before encode.
//                Reset the Reconstructed/Reference buffer attribute.
//------------------------------------------------------------------------------
static int _EncBuff(mhve_ops* mops, mhve_vpb* mvpb)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    rqct_ops* rqct = mhe->p_rqct;
    h265_enc* h265 = mhe->h265;
    mhve_job* mjob = (mhve_job*)mhe->p_regs;
    pmbr_ops* pmbr = mhe->p_pmbr;
    pmbr_cfg  pmbrcfg;
    rqct_cfg  rqcf;
    rqct_buf  rqcb;
    int type, rpsi = 0;
    int err = 0, idx;
    int i, roiidx, maxqp, minqp;
    sps_t *sps = h265_find_set(h265, HEVC_SPS, 0);
    pps_t *pps = h265_find_set(h265, HEVC_PPS, 0);
    slice_t *sh = h265_find_set(h265, HEVC_SLICE, 0);

    /* Link input video buffer */
    mhe->m_encp = *mvpb;
    mvpb = &mhe->m_encp;

    if(mhe->b_seqh & OPS_SEQH_START)
    {
        rqct->seq_sync(rqct);
        rqct->seq_conf(rqct);
        //h265->m_pps.i_init_qp = rqct->i_enc_qp;   //derek check
        mhe->b_seqh = OPS_SEQH_RESET;
        h265_seq_sync(h265);
    }
    else
    {
        if(mhe->b_seqh & OPS_SEQH_RESET)
        {
            rqct->seq_sync(rqct);
            h265_seq_sync(h265);
        }
    }

    if(mvpb->u_flags & MHVE_FLAGS_FORCEI)
    {
        rqct->seq_sync(rqct);
        h265_seq_sync(h265);
    }

    if(mvpb->u_flags & MHVE_FLAGS_DISREF)
        rqct->b_unrefp = 1;

    rqcb.u_config = 0;
    /* RQCT : Decide current picture type (I,P,B) */
    if(0 != (err = rqct->enc_buff(rqct, &rqcb)))
        return err;

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
        pps->b_cu_qp_delta_enabled = 1;

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
            pps->b_cu_qp_delta_enabled = 1;
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
                pps->b_cu_qp_delta_enabled = 0;
            }
            else
            {
                pmbr->i_LutTyp = 0;
                pps->b_cu_qp_delta_enabled = 1;
            }
        }

    }

    //CamOsPrintf("EncQP:%d\n",rqct->i_enc_qp);

    pmbr->i_FrmQP = rqct->i_enc_qp;
    pmbr->i_FrmLamdaScaled = rqct->i_enc_lamda;
    pmbr->i_PicTyp = rqct->i_pictyp;
    pmbr->i_MaxQP = maxqp;
    pmbr->i_MinQP = minqp;
    pmbr->enc_conf(pmbr, mjob);

#ifdef MODULE_PARAM_SUPPORT
    if(tmvp_enb)
        pps->p_sps->b_temporal_mvp_enable = 1;
    else
        pps->p_sps->b_temporal_mvp_enable = 0;
#endif

    /* Prepare SPS/PPS in sequence header */
    if(mhe->b_seqh & OPS_SEQH_RESET)
    {
        h265_seq_conf(h265);
        mhe->b_seqh = OPS_SEQH_WRITE;
    }

    /* Prepare slice header */
    if(IS_IPIC(rqct->i_pictyp))
    {
        sh->i_slice_type = SLICE_TYPE_I;

        type = HEVC_ISLICE;
        mvpb->u_flags = MHVE_FLAGS_FORCEI;

        //write sequence header:VPS,SPS,PPS for each I frame
        mhe->b_seqh = OPS_SEQH_WRITE;
    }
    else
    {
        sh->i_slice_type = SLICE_TYPE_P;

        if (sps->b_long_term_ref_pics_pres && IS_LTRPPIC(rqct->i_pictyp))
        {
            type = HEVC_LTRPSLICE;
        }
        else
        {    type = HEVC_PSLICE;
            if (mvpb->u_flags & MHVE_FLAGS_DISREF)
                rpsi = !rqct->b_unrefp;
            if (rqct->b_unrefp == 0)
                mvpb->u_flags &= ~MHVE_FLAGS_DISREF;
        }
    }

    sh->i_qp = rqct->i_enc_qp;
    h265->i_picq = rqct->i_enc_qp;

    //MultiSlice
    if(mhe->i_rows > 0)
    {
        sh = h265_find_set(h265, HEVC_SLICE, 0);
        sh->slice_loop_filter_across_slices_enabled_flag = 0;
    }

    if(rqct->b_unrefp == 0)
        mvpb->u_flags &= ~MHVE_FLAGS_DISREF;

    /* pick picture buffer for reconstructing */
    if(0 <= (idx = h265_enc_buff(h265, type, !rqct->b_unrefp)))
    {
        rpb_t* rpb = mhe->m_pics + idx;
        rpb->i_index = mvpb->i_index;
        /* shrink mode: recn buffer address set equal to input buffer */
        if(!rpb->b_valid)
        {
            rpb->u_phys[RPB_YPIX] = mvpb->planes[0].u_phys;
            rpb->u_phys[RPB_CPIX] = mvpb->planes[1].u_phys;
            rpb->u_phys[RPB_TMVP] = mvpb->planes[2].u_phys;
            mvpb->i_index = -1;
        }

        rqcf.type = RQCT_CFG_SEQ;
        rqct->get_rqcf(rqct, &rqcf);
        //    sh->i_poc = rqct->i_enc_nr%rqcf.seq.i_period;
        if(rqcf.seq.i_period != 1)  //fixed for I-only
            sh->i_poc = h265->i_poc;

        //CamOsPrintf("[%s %d]rqcf.seq.i_period = %d, h265->i_poc = %d, sh->i_poc = %d\n", __FUNCTION__, __LINE__ , rqcf.seq.i_period, h265->i_poc, sh->i_poc);
        /* ltr mode */
        do
        {
            if (sps->b_long_term_ref_pics_pres)
            {
                /* write sw slice header */
                if (h265->i_poc > 1)
                    sh->num_long_term_pics = 1;
                else
                    sh->num_long_term_pics = 0;

                if (!IS_IPIC(rqct->i_pictyp))
                {
                    if (IS_LTRPPIC(rqct->i_pictyp))
                    {
                        sh->short_term_ref_pic_set_idx = 1;
                        sh->used_by_curr_pic_lt_flag[0] = 1;
                    }
                    else
                    {
                        sh->short_term_ref_pic_set_idx = 0;
                        sh->used_by_curr_pic_lt_flag[0] = 0;
                    }
                    /* set LTR POC */
                    sh->poc_lsb_lt[0] = 0;
                    if (h265->p_rpls[HEVC_REF_LTR][0] && h265->i_poc-h265->p_rpls[HEVC_REF_LTR][0]->i_poc > 1)
                        sh->poc_lsb_lt[0] = h265->p_rpls[HEVC_REF_LTR][0]->i_poc;
                    sh->delta_poc_msb_present_flag[0] = 0;
                }
            }
        }
        while(0);

        h265->i_swsh = h265_sh_writer(h265);

        if(rqcf.seq.i_period == 1)  //fixed for I-only
            sh->i_poc++;
    }
    else
    {
        err = -1;
    }

    return err;
}

//------------------------------------------------------------------------------
//  Function    : _DeqBuff
//  Description : Dequeue video buffer which is encoded.
//------------------------------------------------------------------------------
static int _DeqBuff(mhve_ops* mops, mhve_vpb* mvpb)
{
    mhe_ops* mhe = (mhe_ops*)mops;

    *mvpb = mhe->m_encp;

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
    mhe_ops* mhe = (mhe_ops*)mops;
    h265_enc* h265 = mhe->h265;
    rqct_ops* rqct = mhe->p_rqct;
    pmbr_ops* pmbr = mhe->p_pmbr;
    mhe_reg* regs = mhe->p_regs;
    mhve_vpb* vpb = &mhe->m_encp;
    pmbr_cfg  pmbrcfg;
    int idx, err = 0;

//#define DUMP_REG_DBG

#ifdef DUMP_REG_DBG
    int regaddr;
    void* hev_base0 = (void *)0xfd2e3a00;
    //void* mhe_base0 = (void *)0xfd2e3c00;
    void* hev_base1 = (void *)0xfd2e3e00;
    void* hev_base2 = (void *)0xfd2e1200;
#endif

#ifdef ENABLE_DUMP_REG
    // Parse dump register here
    int i = 0;
    mhe_dump_reg* dump_reg;

    dump_reg = (mhe_dump_reg *)mhe->p_drvptr;

    regs->hev_bank1.reg6d = dump_reg->hev_bank1_reg6d;    //REGRD(hev_base1, 0x6d, "bits size:lo");
    regs->hev_bank1.reg6e = dump_reg->hev_bank1_reg6e;    //REGRD(hev_base1, 0x6e, "bits size:hi");
    regs->enc_bitcnt = ((uint)(regs->hev_bank1.reg_ro_hev_ec_bsp_bit_cnt_high) << 16) + regs->hev_bank1.reg_ro_hev_ec_bsp_bit_cnt_low;
    regs->mjob.i_tick = (int)(regs->enc_cycles);
    regs->mjob.i_bits = (int)(regs->enc_bitcnt - regs->bits_delta); // Get the bit counts of pure bitstream and slice header.

    for(i = 0; i < PMBR_LUT_SIZE; i++)
    {
        regs->pmbr_lut_hist[i] = dump_reg->hev_bank1_reg16[i];
    }

    for(i = 0; i < (1 << PMBR_LOG2_HIST_SIZE); i++)
    {
        regs->pmbr_tc_hist[i] = dump_reg->hev_bank2_reg20[i];
        regs->pmbr_pc_hist[i] = dump_reg->hev_bank2_reg00[i];
    }

    regs->pmbr_tc_accum = ((uint)(dump_reg->hev_bank1_reg2a_low & 0xFF) << 16) + dump_reg->hev_bank1_reg29;

    //initial dump register, avoid no dump register when buffer full
    memset(dump_reg, 0, sizeof(mhe_dump_reg));

#endif

#ifdef DUMP_REG_DBG
    CamOsHexdump( (char *)mhe->p_drvptr, mhe->u_drsize);
    CamOsPrintf("========MHE dump Reg=========\n");
    CamOsPrintf("dump_reg->hev_bank0_reg16: 0x%x\n", dump_reg->hev_bank0_reg16);
    CamOsPrintf("dump_reg->hev_bank0_reg17: 0x%x\n", dump_reg->hev_bank0_reg17);
    CamOsPrintf("dump_reg->hev_bank0_reg18: 0x%x\n", dump_reg->hev_bank0_reg18);
    CamOsPrintf("dump_reg->hev_bank0_reg19: 0x%x\n", dump_reg->hev_bank0_reg19);
    CamOsPrintf("dump_reg->hev_bank1_reg6d: 0x%x\n", dump_reg->hev_bank1_reg6d);
    CamOsPrintf("dump_reg->hev_bank1_reg6e: 0x%x\n", dump_reg->hev_bank1_reg6e);
    CamOsPrintf("dump_reg->hev_bank0_reg1a: 0x%x\n", dump_reg->hev_bank0_reg1a);
    CamOsPrintf("dump_reg->hev_bank0_reg1b: 0x%x\n", dump_reg->hev_bank0_reg1b);

    CamOsPrintf("dump_reg->hev_bank0_reg1e: 0x%x\n", dump_reg->hev_bank0_reg1e);

    for(i = 0; i < PMBR_LUT_SIZE; i++)
    {
        CamOsPrintf("dump_reg->hev_bank1_reg16[%d]: 0x%x\n", i , dump_reg->hev_bank1_reg16[i]);
    }

    for(i = 0; i < (1 << PMBR_LOG2_HIST_SIZE); i++)
    {
        CamOsPrintf("dump_reg->hev_bank2_reg20[%d]: 0x%x\n", i , dump_reg->hev_bank2_reg20[i]);
        CamOsPrintf("dump_reg->hev_bank2_reg00[%d]: 0x%x\n", i , dump_reg->hev_bank2_reg00[i]);
    }

    CamOsPrintf("dump_reg->hev_bank1_reg26: 0x%x\n", dump_reg->hev_bank1_reg26);
    CamOsPrintf("dump_reg->hev_bank1_reg27_low: 0x%x\n", dump_reg->hev_bank1_reg27_low);
    CamOsPrintf("dump_reg->hev_bank1_reg27_high: 0x%x\n", dump_reg->hev_bank1_reg27_high);
    CamOsPrintf("dump_reg->hev_bank1_reg28: 0x%x\n", dump_reg->hev_bank1_reg28);

    CamOsPrintf("dump_reg->hev_bank1_reg2a_high: 0x%x\n", dump_reg->hev_bank1_reg2a_high);
    CamOsPrintf("dump_reg->hev_bank1_reg2b: 0x%x\n", dump_reg->hev_bank1_reg2b);
    CamOsPrintf("dump_reg->hev_bank1_reg29: 0x%x\n", dump_reg->hev_bank1_reg29);
    CamOsPrintf("dump_reg->hev_bank1_reg2a_low: 0x%x\n", dump_reg->hev_bank1_reg2a_low);
    CamOsPrintf("------------------------------\n");

    CamOsPrintf("========MHE Read REG=========\n");
    CamOsPrintf("dump_reg->hev_bank0_reg16: 0x%x\n", REGRD(hev_base0, 0x16, ""));
    CamOsPrintf("dump_reg->hev_bank0_reg17: 0x%x\n", REGRD(hev_base0, 0x17, ""));
    CamOsPrintf("dump_reg->hev_bank0_reg18: 0x%x\n", REGRD(hev_base0, 0x18, ""));
    CamOsPrintf("dump_reg->hev_bank0_reg19: 0x%x\n", REGRD(hev_base0, 0x19, ""));
    CamOsPrintf("dump_reg->hev_bank1_reg6d: 0x%x\n", REGRD(hev_base1, 0x6d, ""));
    CamOsPrintf("dump_reg->hev_bank1_reg6e: 0x%x\n", REGRD(hev_base1, 0x6e, ""));
    CamOsPrintf("dump_reg->hev_bank0_reg1a: 0x%x\n", REGRD(hev_base0, 0x1a, ""));
    CamOsPrintf("dump_reg->hev_bank0_reg1b: 0x%x\n", REGRD(hev_base0, 0x1b, ""));

    CamOsPrintf("dump_reg->hev_bank0_reg1e: 0x%x\n", REGRD(hev_base0, 0x1e, ""));

    for(i = 0; i < PMBR_LUT_SIZE; i++)
    {
        regaddr = 0x16 + i;
        CamOsPrintf("dump_reg->hev_bank1_reg16[%d]: 0x%x\n", i , REGRD(hev_base1, regaddr, ""));
    }

    for(i = 0; i < (1 << PMBR_LOG2_HIST_SIZE); i++)
    {
        regaddr = 0x20 + i;
        CamOsPrintf("dump_reg->hev_bank2_reg20[%d]: 0x%x\n", i , REGRD(hev_base2, regaddr, ""));
        regaddr = 0x00 + i;
        CamOsPrintf("dump_reg->hev_bank2_reg00[%d]: 0x%x\n", i , REGRD(hev_base2, regaddr, ""));
    }

    CamOsPrintf("dump_reg->hev_bank1_reg26: 0x%x\n", REGRD(hev_base1, 0x26, ""));
    CamOsPrintf("dump_reg->hev_bank1_reg27_low: 0x%x\n", REGRD(hev_base1, 0x27, "low"));
    CamOsPrintf("dump_reg->hev_bank1_reg27_high: 0x%x\n", REGRD(hev_base1, 0x27, "high"));
    CamOsPrintf("dump_reg->hev_bank1_reg28: 0x%x\n", REGRD(hev_base1, 0x28, ""));

    CamOsPrintf("dump_reg->hev_bank1_reg2a_high: 0x%x\n", REGRD(hev_base1, 0x2a, ""));
    CamOsPrintf("dump_reg->hev_bank1_reg2b: 0x%x\n", REGRD(hev_base1, 0x2b, ""));
    CamOsPrintf("dump_reg->hev_bank1_reg29: 0x%x\n", REGRD(hev_base1, 0x29, ""));
    CamOsPrintf("dump_reg->hev_bank1_reg2a_low: 0x%x\n", REGRD(hev_base1, 0x2a, ""));
#endif  //DUMP_REG_DBG

    if(CHECK_IRQ_STATUS(regs->irq_status, IRQ_MARB_BSPOBUF_FULL) ||
            (regs->hev_bank1.reg6d == 0 && regs->hev_bank1.reg6e == 0))
    {
        MHE_MSG(MHE_MSG_ERR, "> MHE Encode Fail %s:%d  [0x%04X  0x%04X  0x%04X]\n",
                __FUNCTION__, __LINE__,
                regs->irq_status, regs->hev_bank1.reg6d, regs->hev_bank1.reg6e);
        return -1;
    }

    pmbrcfg.type = PMBR_CFG_LUT;
    pmbr->get_conf(pmbr, &pmbrcfg);

    rqct->auiQps = pmbrcfg.lut.p_kptr[PMBR_LUT_QP];
    rqct->auiBits = pmbrcfg.lut.p_kptr[PMBR_LUT_TARGET];
    rqct->auiLambdas_SCALED = pmbrcfg.lut.p_kptr[PMBR_LUT_LAMBDA_SCALED];
    rqct->aiIdcHist = pmbrcfg.lut.p_kptr[PMBR_LUT_AIIDCHIST];

    /* PMBR : Update statistic data */
    if((err = pmbr->enc_done(pmbr, &regs->mjob)))
    {
        // TBD
    }

    /* rqct control */
    /* feedback to rate-controller */
    /* RQCT : Update statistic data */
    if((err = rqct->enc_done(rqct, &regs->mjob)))
    {
        rpb_t* rpb = &mhe->m_pics[h265->p_recn->i_id];

        mhe->i_obits = mhe->i_total = mhe->u_oused = 0;

        /* shrink mode: set cache buffer */
        if(!rpb->b_valid)
        {
            vpb->i_index = rpb->i_index;
            rpb->i_index = -1;
        }
        return err;
    }

    /* Update statistic data */
    mhe->i_obits = (rqct->i_bitcnt);
    mhe->i_total += (rqct->i_bitcnt) / 8;
    mhe->u_oused += (rqct->i_bitcnt + regs->bits_delta) / 8;
    if(0 <= (idx = h265_enc_done(h265)))
    {
        rpb_t* rpb = mhe->m_pics + idx;
        /* shrink mode: return cache buffer */
        if(!rpb->b_valid)
        {
            vpb->i_index = rpb->i_index;
//            CamOsPrintf("%s(poc<%d>, idx<%d>)-i_index(%d)\n", __func__, h265->i_poc, idx, vpb->i_index);
        }

        rpb->i_index = -1;
//        CamOsPrintf("shrink return(idx<%d>)\n", vpb->i_index);
    }

    //    printk("=======================================\n");
    MHE_MSG(MHE_MSG_DEBUG, "[%s %d] mhe->u_oused = %d\n", __FUNCTION__, __LINE__, mhe->u_oused);

    /* Reset the reconstructed buffer attribute */
    //mhe->p_recn = NULL;
    mhe->i_seqn++;
    mhe->b_seqh = 0;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _OutBuff
//  Description : Output coded buffer.
//------------------------------------------------------------------------------
static int _OutBuff(mhve_ops* mops, mhve_cpb* mcpb)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    mhve_vpb* mvpb = &mhe->m_encp;
    rqct_ops* rqct = mhe->p_rqct;
    h265_enc* h265 = mhe->h265;
    sps_t*   sps = h265_find_set(h265, HEVC_SPS, 0);
    int err = 0;
    if(mcpb->i_index >= 0)
    {
        //mhe->u_used = mhe->u_otrm = 0;
        mhe->u_oused = 0;
        mcpb->planes[0].u_phys = 0;
        mcpb->planes[0].i_size = 0;
        mcpb->i_stamp = 0;
        mcpb->i_flags = (MHVE_FLAGS_SOP | MHVE_FLAGS_EOP);
        return err;
    }

    err = mhe->u_oused;
    mcpb->i_index = 0;
    mcpb->planes[0].u_phys = mhe->u_obase;//mhe->u_otbs;
    mcpb->planes[0].i_size = err;
    mcpb->i_stamp = mvpb->i_stamp;
    mcpb->i_flags = (MHVE_FLAGS_SOP | MHVE_FLAGS_EOP);
    if(err > 0)
        mcpb->i_flags |= (mvpb->u_flags & MHVE_FLAGS_CTRL);
    /* Add flag for LTR P-frame */
    if (sps->b_long_term_ref_pics_pres && IS_LTRPPIC(rqct->i_pictyp))
        mcpb->i_flags |= MHVE_FLAGS_LTR_PFRAME;

    return err;
}

//------------------------------------------------------------------------------
//  Function    : _SeqConf
//  Description : Configure sequence setting.
//------------------------------------------------------------------------------
static int _SeqConf(mhve_ops* mops)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    mhe_reg* regs = mhe->p_regs;
    int g_nHwCoreNum = 1;                  //1:dual core, 0:single core
    int g_iStreamId = 0;
    signed int g_MStarMERange[2][2];       // [x, y][min, max]
    int m_iSearchRange = HW_ME_WINDOW_X;   // ME search range
    int m_iSearchRangeY = HW_ME_WINDOW_Y;  // ME search range Y
    int g_bNewMSME = 1;                    // (Default)0: MHE 1.0 ME; 1: New ME scheme
    int g_DisableMergeSkip = 0;            // (Default)0; 1: disable Merge/skip mode
    int g_MaxFmeMode = 1;                  // (Default)1, 2, or 4(all). How many (partitioning) modes passed from IME to FME

    //int g_iFmePrec = QUARTER_PREC;

    //hev_init_reg
    // -- set constant-nonzero-value register --
    regs->hev_bank0.reg_hev_soft_rstz = 1;
    regs->hev_bank0.reg_hev_enc_mode_hevc = 1;
    regs->hev_bank0.reg_hev_me_4x4_disable = 1;
    regs->hev_bank0.reg_hev_me_ref_en_mode = 1;
    regs->hev_bank0.reg_hev_ime_mesr_max_addr = 0x5D;
    regs->hev_bank0.reg_hev_ime_mvx_min = 0xF0; //0xE0;
    regs->hev_bank0.reg_hev_ime_mvx_max = 0x0F; //0x1F;
    regs->hev_bank0.reg_hev_ime_mvy_min = 0xF0;
    regs->hev_bank0.reg_hev_ime_mvy_max = 0x0F;
    //regs->hev_bank0.reg_hev_fme_quarter_disable = (g_iFmePrec < QUARTER_PREC ? 1 : 0);
    //regs->hev_bank0.reg_hev_fme_half_disable    = (g_iFmePrec < HALF_PREC ? 1 : 0);
    regs->hev_bank0.reg_hev_fme_pipeline_on = 1;
    regs->hev_bank0.reg_hev_mcc_merge32_en = 1;
    regs->hev_bank0.reg_hev_mcc_merge16_en = 1;
    regs->hev_bank0.reg_hev_col_w_en = 1;

    //regs->hev_bank1.reg_mbr_const_qp_en = 1;
    //regs->hev_bank1.reg_mbr_ss_turn_off_perceptual = 1;
    //->hev_bank1.reg_txip_ctb_force_on = 0;

    // Dual-core, dual-stream ctrl
    regs->hev_bank1.reg_mhe_dual_core = (g_nHwCoreNum == 1) ? 0 : 1;
    regs->hev_bank1.reg_mhe_dual_row_dbfprf = 3;    // default value
    regs->hev_bank1.reg_mhe_dual_ctb_dbfprf = 5;    // default value
    regs->hev_bank1.reg_mhe_dual_strm_id = g_iStreamId;
    regs->hev_bank1.reg_mhe_dual_bs0_rstz = 1;
    regs->hev_bank1.reg_mhe_dual_bs1_rstz = 1;

    regs->hev_bank2.reg_hev_newme_h_search_max = 6;
    regs->hev_bank2.reg_hev_newme_v_search_max = 3;

    m_iSearchRange = (mhe->i_dmvx <= 32) ? 32 : 96;
    m_iSearchRangeY = (mhe->i_dmvy <= 16) ? 16 : 48;


    /* Global regs values (codec related) */
    g_MStarMERange[0][0] = 0 - m_iSearchRange;
    g_MStarMERange[0][1] = 0 + m_iSearchRange - 1;
    g_MStarMERange[1][0] = 0 - m_iSearchRangeY;
    g_MStarMERange[1][1] = 0 + m_iSearchRangeY - 1;

//    regs->hev_bank0.reg_hev_me_16x16_disable = !(mhe->i_blkp[0] & MHVE_INTER_16x16);
    regs->hev_bank0.reg_hev_me_16x8_disable = !(mhe->i_blkp[0] & MHVE_INTER_16x8);
    regs->hev_bank0.reg_hev_me_8x16_disable = !(mhe->i_blkp[0] & MHVE_INTER_8x16);
    regs->hev_bank0.reg_hev_me_8x8_disable = !(mhe->i_blkp[0] & MHVE_INTER_8x8);
    regs->hev_bank0.reg_hev_me_8x4_disable = !(mhe->i_blkp[0] & MHVE_INTER_8x4);
    regs->hev_bank0.reg_hev_me_4x8_disable = !(mhe->i_blkp[0] & MHVE_INTER_4x8);
//    regs->hev_bank0.reg_hev_me_4x4_disable = !(mhe->i_blkp[0] & MHVE_INTER_4x4);
    regs->hev_bank0.reg_hev_mesr_adapt = !g_bNewMSME; //IMEAdapt can only be 0 when NewMSME is set; forced to 0.

    regs->hev_bank0.reg_hev_ime_umv_disable = 0;
    regs->hev_bank0.reg_hev_ime_mesr_max_addr = (mhe->i_dmvy == 16 ? 95 : 85);
    regs->hev_bank0.reg_hev_ime_mesr_min_addr = (mhe->i_dmvy == 16 ?  0 : 10);
    regs->hev_bank0.reg_hev_ime_mvx_min = g_MStarMERange[0][0]; //_MAX_(-mhe->i_dmvx+32,-32+32);
    regs->hev_bank0.reg_hev_ime_mvx_max = g_MStarMERange[0][1]; //_MIN_( mhe->i_dmvx+32, 31+31);
    regs->hev_bank0.reg_hev_ime_mvy_min = g_MStarMERange[1][0]; //_MAX_(-mhe->i_dmvy+16,-16+16);
    regs->hev_bank0.reg_hev_ime_mvy_max = g_MStarMERange[1][1]; //_MIN_( mhe->i_dmvy+16, 16+15);
    regs->hev_bank0.reg_hev_ime_sr16 = (mhe->i_dmvx <= 32); //(pcEncTop->getSearchRange()==HW_ME_WINDOW_X_0) ? 1 : 0;
    regs->hev_bank2.reg_hev_newme_en = g_bNewMSME;

    if(g_bNewMSME)
    {
        regs->hev_bank0.reg_hev_ime_mvx_min = -HW_ME_STATIC_X;
        regs->hev_bank0.reg_hev_ime_mvx_max = HW_ME_STATIC_X - 1;
        regs->hev_bank0.reg_hev_ime_mvy_min = -HW_ME_STATIC_Y;
        regs->hev_bank0.reg_hev_ime_mvy_max = HW_ME_STATIC_Y - 1;
        regs->hev_bank2.reg_hev_newme_h_search_max = (m_iSearchRange == 96) ? 6 : 2;
        regs->hev_bank2.reg_hev_newme_v_search_max = (m_iSearchRangeY == 48) ? 3 : 1;
    }

    regs->hev_bank0.reg_hev_fme_quarter_disable = (mhe->i_subp != QUARTER_PREC);
    regs->hev_bank0.reg_hev_fme_half_disable = (mhe->i_subp == INT_PREC);
    regs->hev_bank0.reg_hev_fme_skip = 0 != (mhe->i_blkp[0] & MHVE_INTER_SKIP);

    // FME
    regs->hev_bank0.reg_hev_fme_merge32_en = g_DisableMergeSkip ? 0 : 1;
    regs->hev_bank0.reg_hev_fme_merge16_en = g_DisableMergeSkip ? 0 : 1;
    regs->hev_bank0.reg_hev_fme_mode_no = (g_MaxFmeMode == 2) ? 1 : 0;


    // Reset IMI
    regs->hev_bank2.reg60 = 0;
    regs->hev_bank2.reg61 = 0;
    regs->hev_bank2.reg62 = 0;
    regs->hev_bank2.reg63 = 0;
    regs->hev_bank2.reg64 = 0;
    regs->mhe_bank0.reg00 = 0;
    regs->mhe_bank0.reg01 = 0;
    regs->mhe_bank0.reg40 = 0;
    regs->mhe_bank0.reg45 = 0;

#if defined(MMHE_IMI_BUF_ADDR)
    if(mhe->i_pctw <= MMHE_IMI_MAX_WIDTH)
    {
        // Reference Y lbw
        regs->hev_bank2.reg_hev_lbw_mode = 1;
        regs->mhe_bank0.reg_mhe_marb_wp_imi_en = (1 << 7);
        // Reference C lbw
        regs->hev_bank2.reg_hev_mcc_lbw_mode = 1;
        regs->mhe_bank0.reg_mhe_marb_wp_imi_en |= (1 << 4);
        // gn lbw
        regs->mhe_bank0.reg_mhe_marb_wp_imi_en |= (1 << 2);
        regs->mhe_bank0.reg_mhe_marb_rp_imi_en |= (1 << 2);
        // ppu lbw
        regs->mhe_bank0.reg_mhe_marb_wp_imi_en |= (1 << 6);
        regs->mhe_bank0.reg_mhe_marb_rp_imi_en |= (1 << 6);
        // Read/Write histogram enable
        regs->mhe_bank0.reg_mhe_whist_en = 1;
        regs->mhe_bank0.reg_mhe_rhist_en = 1;
        // Read/Write histogram mode ( mode 2 : Latch count)
        regs->mhe_bank0.reg_mhe_whist_mode = 2;
        regs->mhe_bank0.reg_mhe_rhist_mode = 2;
    }
    else
    {
        regs->hev_bank2.reg60 = 0;
        regs->hev_bank2.reg61 = 0;
        regs->hev_bank2.reg62 = 0;
        regs->hev_bank2.reg63 = 0;
        regs->mhe_bank0.reg00 = 0;
        regs->mhe_bank0.reg01 = 0;
        regs->mhe_bank0.reg40 = 0;
        regs->mhe_bank0.reg45 = 0;
    }
#endif

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _PutData
//  Description : Put SPS/PPS/User data/Slice header to output buffer before encode start.
//------------------------------------------------------------------------------
static int _PutData(mhve_ops* mops, void* user, int size)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    h265_enc* h265 = mhe->h265;
    mhe_reg* regs = mhe->p_regs;
    void* src;
    void* dst;
    int len = 0;
    int  bcnt = 0;
    int  bits = 0;

    if(size > 0)
    {
        if(!mhe->p_okptr)
        {
            MHE_MSG(MHE_MSG_ERR, "> _PutData p_okptr NULL\n");
            return -1;
        }

        dst = mhe->p_okptr;

        /* copy sequence-header bits */
        if(mhe->b_seqh & OPS_SEQH_WRITE)
        {
            MEM_COPY(dst, h265->m_seqh, h265->i_seqh);
            bcnt += h265->i_seqh * 8;
            dst += h265->i_seqh;
        }

        /* Insert user-data */
        MEM_COPY(dst, user, size);
        bcnt += size * 8;
        dst += size;

        /* Insert slice-header */
        MEM_COPY(dst, h265->m_swsh, h265->i_swsh);
        bits = h265->i_swsh * 8;
        bcnt += bits;;

        mhe->u_oused = (bcnt >> 11) * 256; // Max 2048 bits(256 Bytes) one time due to bits_coded[256]

        // EROY CHECK : Just copy the rest data bits to bits_coded[], previous 256 Bytes is put in output buffer.
        src = mhe->p_okptr + mhe->u_oused;
        dst = (char*)regs->bits_coded;
        MEM_COPY(dst, src, ((bcnt + 7) / 8) & 255);
        regs->bits_count = bcnt & 2047;
        regs->bits_delta = regs->bits_count - bits;

        if (mhe->flush_cache)
        {
            mhe->flush_cache((void *)mhe->p_okptr, (bcnt + 7) / 8);
        }
        else
        {
            CamOsMemFlush((void *)mhe->p_okptr, (bcnt + 7) / 8);
        }
    }
    else
    {
        dst = (char*)regs->bits_coded;

        /* copy sequence-header bits */
        if(mhe->b_seqh & OPS_SEQH_WRITE)
        {
            MEM_COPY(dst, h265->m_seqh, h265->i_seqh);
            bcnt += h265->i_seqh * 8;
            dst += h265->i_seqh;
        }

        /* Insert slice-header */
        MEM_COPY(dst, h265->m_swsh, h265->i_swsh);
        bits = h265->i_swsh * 8;
        bcnt += bits;

        regs->bits_count = bcnt;
        regs->bits_delta = bcnt - bits;
        mhe->u_oused = 0; // No data is put in output buffer previously.
    }

    return len;
}

//------------------------------------------------------------------------------
//  Function    : _EncConf
//  Description : Configure current frame setting.
//------------------------------------------------------------------------------
static int _EncConf(mhve_ops* mops)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    mhve_vpb* encp = &mhe->m_encp;
    h265_enc* h265 = mhe->h265;
    mhe_reg* pReg = mhe->p_regs;
    sps_t* sps = h265_find_set(h265, HEVC_SPS, 0);
    pps_t* pps = h265_find_set(h265, HEVC_PPS, 0);
    slice_t *sh = h265_find_set(h265, HEVC_SLICE, 0);
    pic_t *recn = h265->p_recn;
    uint phys;
    unsigned int value;

    pic_t *ref;
    rpb_t *rpb;

    unsigned int m_maxNumMergeCand = 5;
    unsigned int m_log2ParallelMergeLevelMinus2 = 0;

    unsigned int g_uiMaxCUWidth  = CTB_SIZE;
    unsigned int g_uiMaxCUHeight = CTB_SIZE;

    unsigned int  picWidthIn32Pel;
    unsigned char colAvailble;

    int m_enableTMVPFlag = 0;
    int maxSliceSegmentAddress;
    int bitsSliceSegmentAddress;


#ifdef MODULE_PARAM_SUPPORT
    m_enableTMVPFlag = tmvp_enb;
#endif

    pReg->coded_framecnt = mhe->i_seqn;

    // Codec type (only support h.265)
    pReg->hev_bank0.reg_hev_enc_mode_hevc = MHE_REG_MOD_HEVC;
    pReg->hev_bank0.reg_hev_enc_mode = MHE_REG_ENC_H265;

    switch(mhe->e_pixf)
    {
        case MHVE_PIX_NV21:
            pReg->hev_bank0.reg_hev_src_chroma_swap = MHE_REG_PLNRLDR_VU;
        case MHVE_PIX_NV12:
            pReg->hev_bank0.reg_hev_src_yuv_format = MHE_REG_PLNRLDR_420;
            pReg->hev_bank0.reg_hev_src_luma_pel_width = mhe->m_encp.i_pitch ;
            pReg->hev_bank0.reg_hev_src_chroma_pel_width = mhe->m_encp.i_pitch;
            break;
        case MHVE_PIX_YVYU:
            pReg->hev_bank0.reg_hev_src_chroma_swap = MHE_REG_PLNRLDR_VU;
        case MHVE_PIX_YUYV:
            pReg->hev_bank0.reg_hev_src_yuv_format = MHE_REG_PLNRLDR_422;
            pReg->hev_bank0.reg_hev_src_luma_pel_width = mhe->m_encp.i_pitch >> 1;
            pReg->hev_bank0.reg_hev_src_chroma_pel_width = 0;
            break;
        default:
            break;
    }

    // Picture Info
    pReg->hev_bank0.reg_hev_frame_type = (recn->i_type == HEVC_ISLICE) ? 0 : 1;
    pReg->hev_bank0.reg_hev_slice_id = 0;
    pReg->hev_bank0.reg_hev_enc_pel_width_m1 = mhe->i_pctw - 1;
    pReg->hev_bank0.reg_hev_enc_pel_height_m1 = mhe->i_pcth - 1;
    pReg->hev_bank0.reg_hev_enc_ctb_cnt_m1 = (mhe->i_pctw / g_uiMaxCUWidth) * (mhe->i_pcth / g_uiMaxCUHeight) - 1;
    pReg->hev_bank0.reg_hev_cur_poc_low = sh->i_poc;        //fixed for I-only
    pReg->hev_bank0.reg_hev_cur_poc_high = sh->i_poc >> 16; //fixed for I-only

    //calculate number of bits required for slice address
    maxSliceSegmentAddress = (mhe->i_pctw / g_uiMaxCUWidth) * (mhe->i_pcth / g_uiMaxCUHeight);//pcSlice->getPic()->getNumCUsInFrame();
    bitsSliceSegmentAddress = 0;
    while(maxSliceSegmentAddress > (1 << bitsSliceSegmentAddress))
    {
        bitsSliceSegmentAddress++;
    }

    pReg->hev_bank0.reg_hev_ec_mdc_bits_ctb_num_m1 = bitsSliceSegmentAddress - 1;
    pReg->hev_bank0.reg_hev_ec_mdc_bits_poc_m1 = sps->i_log2_max_poc_lsb - 1;   //pcSlice->getSPS()->getBitsForPOC()-1;
    pReg->hev_bank0.reg_hev_ec_mdc_nuh_id_p1 = 1; //pcSlice->getTLayer()+1;

    //QP
    pReg->hev_bank0.reg_hev_ec_mdc_chroma_qp_flag = pps->b_slice_chroma_qp_offsets_pres;
    pReg->hev_bank0.reg_hev_ec_qp_delta_enable_flag = pps->b_cu_qp_delta_enabled;

    // Mvp info
    colAvailble = !(h265->i_poc == 0 || h265->i_poc == 1); // unavailable in I frame and first P frame
    pReg->hev_bank0.reg_hev_col_l0_flag = colAvailble;//m_enableTMVPFlag && colAvailble;   // pcSlice->getColFromL0Flag()  && colAvailble;
    pReg->hev_bank0.reg_hev_col_r_en = m_enableTMVPFlag && colAvailble; // col buffer exist or not
    pReg->hev_bank0.reg_hev_temp_mvp_flag = m_enableTMVPFlag; // pcSlice->getEnableTMVPFlag() && colAvailble;

    pReg->hev_bank0.reg_hev_max_merge_cand_m1 = m_maxNumMergeCand - 1; // pcSlice->getMaxNumMergeCand() - 1;
    pReg->hev_bank0.reg_hev_parallel_merge_level = m_log2ParallelMergeLevelMinus2 + 2;

    // Ref List
    value = (recn->i_type == HEVC_ISLICE) ? 0 : h265->i_poc - 1;
    pReg->hev_bank0.reg_hev_reflst0_poc_low = value;
    pReg->hev_bank0.reg_hev_reflst0_poc_high = value >> 16;

    if(!(recn->i_type == HEVC_ISLICE))  //reference frame (not I frame)
    {
        pReg->hev_bank0.reg_hev_reflst0_lt_fg = 0;
        pReg->hev_bank0.reg_hev_reflst0_st_fg = 1;
        pReg->hev_bank0.reg_hev_reflst0_fbidx = 0;//(h265->i_poc - 1) % 5;
    }
    else
    {
        pReg->hev_bank0.reg_hev_reflst0_lt_fg = 0;
        pReg->hev_bank0.reg_hev_reflst0_st_fg = 0;
        pReg->hev_bank0.reg_hev_reflst0_fbidx = 0;
    }

    // NAL unit
    //if(!(recn->i_type == HEVC_ISLICE) && pReg->coded_framecnt == 0)
    if(sh->i_nal_type == IDR_W_RADL)
    {
        pReg->hev_bank0.reg_hev_ec_mdc_nal_unit_type = IDR_W_RADL;  //derek check
        pReg->hev_bank0.reg_hev_ec_mdc_nuh_id_p1 = 1;               //derek check
        pReg->hev_bank0.reg_hev_ec_mdc_is_idr_picture = 1;          //derek check
    }
    else
    {
        pReg->hev_bank0.reg_hev_ec_mdc_nal_unit_type = TRAIL_R;     //derek check //pcSlice->getNalUnitType();
        pReg->hev_bank0.reg_hev_ec_mdc_nuh_id_p1 = 1;               //derek check //pcSlice->getTLayer()+1;
        pReg->hev_bank0.reg_hev_ec_mdc_is_idr_picture = 0;          //derek check
    }

    // Multi-slice mode
    pReg->hev_bank0.reg_hev_ec_multislice_en = (mhe->i_rows == 0) ? 0 : 1;
    pReg->hev_bank0.reg_hev_ec_multislice_1st_ctby = mhe->i_rows;
    pReg->hev_bank0.reg_hev_ec_multislice_ctby = mhe->i_rows;

    // RDO [Rate control coefficients]
    pReg->hev_bank0.reg_hev_rdo_tu4_intra_c1 = 113;     //g_rate_model_C1[0][0] = 113
    pReg->hev_bank0.reg_hev_rdo_tu4_inter_c1 = 117;     //g_rate_model_C1[1][0] = 117
    pReg->hev_bank0.reg_hev_rdo_tu8_intra_c1 = 81;      //g_rate_model_C1[0][1] = 81
    pReg->hev_bank0.reg_hev_rdo_tu8_inter_c1 = 104;     // g_rate_model_C1[1][1] = 104
    pReg->hev_bank0.reg_hev_rdo_tu16_intra_c1 = 67;     //g_rate_model_C1[0][2] = 67
    pReg->hev_bank0.reg_hev_rdo_tu16_inter_c1 = 97;     //g_rate_model_C1[1][2] = 97
    pReg->hev_bank0.reg_hev_rdo_tu32_intra_c1 = 72;     //g_rate_model_C1[0][3] = 72
    pReg->hev_bank0.reg_hev_rdo_tu32_inter_c1 = 85;     //g_rate_model_C1[1][3] = 85
    pReg->hev_bank0.reg_hev_rdo_tu4_intra_c0 = 54;      //g_rate_model_C0[0][0] = 54
    pReg->hev_bank0.reg_hev_rdo_tu4_intra_beta = 22;    //g_rate_model_beta[0][0] = 22
    pReg->hev_bank0.reg_hev_rdo_tu4_inter_c0 = 82;      //g_rate_model_C0[1][0] = 82
    pReg->hev_bank0.reg_hev_rdo_tu4_inter_beta = 18;    //g_rate_model_beta[1][0] = 18
    pReg->hev_bank0.reg_hev_rdo_tu8_intra_c0 = 252;     //g_rate_model_C0[0][1] = 252
    pReg->hev_bank0.reg_hev_rdo_tu8_intra_beta = 20;    //g_rate_model_beta[0][1] = 20
    pReg->hev_bank0.reg_hev_rdo_tu8_inter_c0 = 226;     //g_rate_model_C0[1][1] = 226
    pReg->hev_bank0.reg_hev_rdo_tu8_inter_beta = 18;    //g_rate_model_beta[1][1] = 18
    pReg->hev_bank0.reg_hev_rdo_tu16_intra_c0 = 467;    //g_rate_model_C0[0][2] = 467
    pReg->hev_bank0.reg_hev_rdo_tu16_intra_beta = 20;   //g_rate_model_beta[0][2] = 20
    pReg->hev_bank0.reg_hev_rdo_tu16_inter_c0 = 229;    //g_rate_model_C0[1][2] = 229
    pReg->hev_bank0.reg_hev_rdo_tu16_inter_beta = 18;   //g_rate_model_beta[1][2] = 18
    pReg->hev_bank0.reg_hev_rdo_tu32_intra_c0 = 764;    //g_rate_model_C0[0][3] = 764
    pReg->hev_bank0.reg_hev_rdo_tu32_intra_beta = 10;   //g_rate_model_beta[0][3] = 10
    pReg->hev_bank0.reg_hev_rdo_tu32_inter_c0 = 371;    //g_rate_model_C0[1][3] = 371
    pReg->hev_bank0.reg_hev_rdo_tu32_inter_beta = 26;   //g_rate_model_beta[1][3] = 26

    // PPU
    pReg->hev_bank0.reg_hev_disilf_idc = pps->b_deblocking_filter_disabled ? 1 : 0;
    pReg->hev_bank0.reg_hev_disilf_idc |= (pps->b_loop_filter_across_slices_enabled ? 0 : 1) << 3;
    pReg->hev_bank0.reg_hev_beta_offset = pps->i_beta_offset;
    pReg->hev_bank0.reg_hev_tc_offset = pps->i_tc_offset;
    pReg->hev_bank0.reg_hev_cb_qp_offset = pps->i_cb_qp_offset;
    pReg->hev_bank0.reg_hev_cr_qp_offset = pps->i_cr_qp_offset;

    picWidthIn32Pel = mhe->i_pctw >> 5;
    pReg->hev_bank0.reg_hev_ppu_fb_pitch = picWidthIn32Pel;
    pReg->hev_bank0.reg_hev_ppu_fb_pitch_lsb = ((picWidthIn32Pel % 4) == 0)
            ? (picWidthIn32Pel * 32) / 128
            : ((picWidthIn32Pel + 3) * 32) / 128;

    // GN
    pReg->hev_bank0.reg_hev_gn_sz_ctb_m1 = ((mhe->i_pctw + 31) >> 5) - 1;

    // Set Output buffer attribute
    pReg->coded_data = pReg->bits_coded;
    pReg->coded_bits = pReg->bits_count;
    pReg->outbs_addr = mhe->u_obase + mhe->u_oused;
    if(mhe->i_iframe_bitsthr && (sh->i_slice_type == SLICE_TYPE_I) && (mhe->u_osize > mhe->i_iframe_bitsthr))
        mhe->u_osize = mhe->i_iframe_bitsthr;
    if(mhe->i_pframe_bitsthr && (sh->i_slice_type == SLICE_TYPE_P) && (mhe->u_osize > mhe->i_pframe_bitsthr))
        mhe->u_osize = mhe->i_pframe_bitsthr;
    pReg->outbs_size = mhe->u_osize - mhe->u_oused;

    //MHE buffer
    pReg->gn_mem = mhe->gn_mem;
    pReg->ppu_int_b = mhe->ppu_int_b;
    pReg->ppu_int_a = mhe->ppu_int_a;

#if defined(MMHE_IMI_BUF_ADDR)
    pReg->imi_ref_y_buf = mhe->imi_ref_y_buf;
    pReg->imi_ref_c_buf = mhe->imi_ref_c_buf;
#endif

    // Buffer setting: current
    phys = encp->planes[0].u_phys + encp->planes[0].i_bias;
    pReg->hev_bank0.reg_hev_cur_y_adr_low = (ushort)(phys >> 4);
    pReg->hev_bank0.reg_hev_cur_y_adr_high = (ushort)(phys >> 16);
    phys = encp->planes[1].u_phys + encp->planes[1].i_bias;
    pReg->hev_bank0.reg_hev_cur_c_adr_low = (ushort)(phys >> 4);
    pReg->hev_bank0.reg_hev_cur_c_adr_high = (ushort)(phys >> 16);

    // Set Dump Register
    pReg->hev_bank2.reg_hev_dump_reg_sadr_low = (ushort)(mhe->p_drpptr >> 4);
    pReg->hev_bank2.reg_hev_dump_reg_sadr_high = (ushort)(mhe->p_drpptr >> 16);
    pReg->hev_bank2.reg_hev_dump_reg_en = 1;

    /* recn picture */
    rpb =&mhe->m_pics[recn->i_id];
    //CamOsPrintf("recn:[idx:%d] Y:0x%08X C:0x%08X Col:0x%08X\n", recn->i_id, rpb->u_phys[RPB_YPIX], rpb->u_phys[RPB_CPIX], rpb->u_phys[RPB_TMVP]);
    pReg->ppu_y_base_buf[0] = rpb->u_phys[RPB_YPIX];
    pReg->ppu_c_base_buf[0] = rpb->u_phys[RPB_CPIX];
    pReg->col_w_sadr_buf[0] = rpb->u_phys[RPB_TMVP];

    /* reference-list0 */
    if (h265->i_active_l0_count > 0)
    {
        if (recn->i_type == HEVC_LTRPSLICE)
            ref = h265_find_rpl(h265, HEVC_REF_LTR, 0);
        else
            ref = h265_find_rpl(h265, HEVC_REF_ACTIVE_L0, 0);
        if (!ref)
        {
            MHE_MSG(MHE_MSG_ERR,"h265_find_rpl return NULL\n");
            return -1;
        }
        rpb =&mhe->m_pics[ref->i_id];
        //CamOsPrintf("ref%d:[idx:%d] Y:0x%08X C:0x%08X Col:0x%08X\n", h265->i_active_l0_count, ref->i_id, rpb->u_phys[RPB_YPIX], rpb->u_phys[RPB_CPIX], rpb->u_phys[RPB_TMVP]);
        pReg->ppu_y_base_buf[1] = rpb->u_phys[RPB_YPIX];
        pReg->ppu_c_base_buf[1] = rpb->u_phys[RPB_CPIX];
        pReg->col_w_sadr_buf[1] = rpb->u_phys[RPB_TMVP];
    }
//    printk("====================================================================================\n");
    /* reference-list0 */
    if (h265->i_active_l0_count > 1)
    {
        ref = h265_find_rpl(h265, HEVC_REF_ACTIVE_L0, 1);
        if (!ref)
        {
            MHE_MSG(MHE_MSG_ERR,"h265_find_rpl return NULL\n");
            return -1;
        }
        rpb =&mhe->m_pics[ref->i_id];
    }

    // Set CmdQ IRQ
    pReg->hev_bank1.reg2c = 0x00FF;
    pReg->hev_bank1.reg2d = 0xFFFF;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SetConf
//  Description : Apply configure.
//------------------------------------------------------------------------------
static int _SetConf(mhve_ops* mops, mhve_cfg* mcfg)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    h265_enc* h265 = mhe->h265;
    int err = -1;
    //int i;

    switch(mcfg->type)
    {
            pps_t* pps;
            sps_t* sps;
            vps_t* vps;
        case MHVE_CFG_RES:
            if((unsigned)mcfg->res.e_pixf <= MHVE_PIX_YVYU)
            {
                pps = h265_find_set(h265, HEVC_PPS, 0);
                sps = pps->p_sps;
                mhe->e_pixf = mcfg->res.e_pixf;
                mhe->i_pixw = mcfg->res.i_pixw;
                mhe->i_pixh = mcfg->res.i_pixh;
                mhe->i_pctw = _ALIGN(5, mcfg->res.i_pixw);
                mhe->i_pcth = _ALIGN(5, mcfg->res.i_pixh);
                mhe->i_rpbn = mcfg->res.i_rpbn;
                mhe->u_conf = mcfg->res.u_conf;
                h265->i_picw = mhe->i_pixw;
                h265->i_pich = mhe->i_pixh;
                h265->i_cb_w = (h265->i_picw + ((1 << sps->i_log2_max_cb_size) - 1)) >> sps->i_log2_max_cb_size;
                h265->i_cb_h = (h265->i_pich + ((1 << sps->i_log2_max_cb_size) - 1)) >> sps->i_log2_max_cb_size;
                mhe->b_seqh |= OPS_SEQH_RESET;
                err = 0;
            }
            break;
        case MHVE_CFG_DMA:
            if(mcfg->dma.i_dmem >= 0)
            {
                uint addr = mcfg->dma.u_phys;
                int index = mcfg->dma.i_dmem;
                rpb_t* ref = mhe->m_pics + index;
                ref->i_index = -1;

                ref->i_state = RPB_STATE_FREE;
                ref->u_phys[RPB_YPIX] = !mcfg->dma.i_size[0] ? 0 : addr;
                addr += mcfg->dma.i_size[0];
                ref->u_phys[RPB_CPIX] = !mcfg->dma.i_size[1] ? 0 : addr;
                addr += mcfg->dma.i_size[1];
                ref->u_phys[RPB_TMVP] = !mcfg->dma.i_size[2] ? 0 : addr;
                addr += mcfg->dma.i_size[2];
                ref->b_valid = ref->u_phys[RPB_YPIX] != 0;
                err = 0;
            }
            else if(mcfg->dma.i_dmem == MHVE_CFG_DMA_OUTPUT_BUFFER)
            {
                mhe->p_okptr = mcfg->dma.p_vptr;
                mhe->u_obase = mcfg->dma.u_phys;
                mhe->u_osize = mcfg->dma.i_size[0];
                mhe->u_oused = 0;
                mhe->flush_cache = mcfg->dma.pFlushCacheCb;
                err = 0;
            }
            else if(mcfg->dma.i_dmem == MHVE_CFG_DMA_NALU_BUFFER)
            {
                mhe->u_mbp_base = mcfg->dma.u_phys;
                err = 0;
            }
            break;
        case MHVE_CFG_HEV:
            pps = h265_find_set(h265, HEVC_PPS, 0);
            sps = pps->p_sps;
            h265->i_profile = mcfg->hev.i_profile;
            h265->i_level = mcfg->hev.i_level;
            sps->i_log2_max_cb_size = mcfg->hev.i_log2_max_cb_size;
            sps->i_log2_min_cb_size = mcfg->hev.i_log2_min_cb_size;
            sps->i_log2_max_tr_size = mcfg->hev.i_log2_max_tr_size;
            sps->i_log2_min_tr_size = mcfg->hev.i_log2_min_tr_size;
            sps->i_max_tr_hierarchy_depth_intra = mcfg->hev.i_tr_depth_intra;
            sps->i_max_tr_hierarchy_depth_inter = mcfg->hev.i_tr_depth_inter;
            sps->b_scaling_list_enable = mcfg->hev.b_scaling_list_enable;
            sps->b_sao_enabled = mcfg->hev.b_sao_enable;
            sps->b_strong_intra_smoothing_enabled = mcfg->hev.b_strong_intra_smoothing;
            pps->i_diff_cu_qp_delta_depth = 0;
            pps->b_cu_qp_delta_enabled = mcfg->hev.b_ctu_qp_delta_enable > 0;
            if(pps->b_cu_qp_delta_enabled)
                pps->i_diff_cu_qp_delta_depth = 3 & (mcfg->hev.b_ctu_qp_delta_enable - 1);
            pps->b_constrained_intra_pred = mcfg->hev.b_constrained_intra_pred;
            pps->i_cb_qp_offset = mcfg->hev.i_cb_qp_offset;
            pps->i_cr_qp_offset = mcfg->hev.i_cr_qp_offset;
            //de-blocking
            pps->b_deblocking_filter_override_enabled = mcfg->hev.b_deblocking_override_enable;
            pps->b_deblocking_filter_disabled = h265->b_deblocking_disable = mcfg->hev.b_deblocking_disable;
            if(h265->b_deblocking_disable)
                h265->i_tc_offset = h265->i_beta_offset = pps->i_tc_offset = pps->i_beta_offset = 0;
            else
            {
                h265->i_tc_offset = pps->i_tc_offset = mcfg->hev.i_tc_offset_div2 * 2;
                h265->i_beta_offset = pps->i_beta_offset = mcfg->hev.i_beta_offset_div2 * 2;
            }
            pps->b_deblocking_filter_control_pres = pps->b_deblocking_filter_override_enabled ||
                                                    !pps->b_deblocking_filter_disabled ||
                                                    pps->i_tc_offset ||
                                                    pps->i_beta_offset;
            mhe->b_seqh |= OPS_SEQH_RESET;
            err = 0;
            break;
        case MHVE_CFG_MOT:
            mhe->i_subp = mcfg->mot.i_subp;
            mhe->i_dmvx = mcfg->mot.i_dmvx;
            mhe->i_dmvy = mcfg->mot.i_dmvy;
            mhe->i_blkp[0] = mcfg->mot.i_blkp[0];
            mhe->i_blkp[1] = 0;
            err = 0;
            break;
        case MHVE_CFG_VUI:
        {
            sps = h265_find_set(h265, HEVC_SPS, 0);

            sps->b_vui_param_pres = 1;  //0

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
            sps->vui.i_num_ticks_poc_diff_one_minus1 = mcfg->vui.i_num_ticks_poc_diff_one_minus1;

            mhe->b_seqh |= OPS_SEQH_RESET;
            err = 0;
        }
        break;
        case MHVE_CFG_LFT:
            pps = h265_find_set(h265, HEVC_PPS, 0);
            if(!pps->b_deblocking_filter_override_enabled)
                break;
            if(!mcfg->lft.b_override)
            {
                h265->b_deblocking_override = 0;
                h265->b_deblocking_disable = pps->b_deblocking_filter_disabled;
                h265->i_tc_offset = pps->i_tc_offset;
                h265->i_beta_offset = pps->i_beta_offset;
                err = 0;
                break;
            }
            if(!mcfg->lft.b_disable && ((unsigned)(mcfg->lft.i_offsetA + 6) > 12 || (unsigned)(mcfg->lft.i_offsetB + 6) > 12))
                break;
            err = 0;
            h265->b_deblocking_override = 1;
            if(!(h265->b_deblocking_disable = mcfg->lft.b_disable))
            {
                h265->i_tc_offset  = mcfg->lft.i_offsetA * 2;
                h265->i_beta_offset = mcfg->lft.i_offsetB * 2;
            }
            break;
        case MHVE_CFG_SPL:
            mhe->i_bits = mcfg->spl.i_bits;
            mhe->i_rows = 0;

            if(0 < mcfg->spl.i_rows)
            {
                mhe->i_rows = mcfg->spl.i_rows;
            }
            err = 0;
            break;
        case MHVE_CFG_BAC:
            h265->b_cabac_init = mcfg->bac.b_init;
            err = 0;
            break;
        case MHVE_CFG_LTR:
        {
            sps = h265_find_set(h265, HEVC_SPS, 0);
            vps = h265_find_set(h265, HEVC_VPS, 0);
            if (mcfg->ltr.b_long_term_reference)
            {
                /* NALU setting */
                vps->i_max_dec_pic_buffering = MAX_DEC_PIC_BUFFERING_DEFAULT+1;
                sps->i_max_dec_pic_buffering = MAX_DEC_PIC_BUFFERING_DEFAULT+1;
                sps->b_long_term_ref_pics_pres = 1;
                sps->i_log2_max_poc_lsb = 16;
                h265->b_enable_pred = mcfg->ltr.b_enable_pred;
            }
            else
            {
                /* NALU setting */
                vps->i_max_dec_pic_buffering = MAX_DEC_PIC_BUFFERING_DEFAULT;
                sps->i_max_dec_pic_buffering = MAX_DEC_PIC_BUFFERING_DEFAULT;
                sps->b_long_term_ref_pics_pres = 0;
                sps->i_log2_max_poc_lsb = 8;
                h265->b_enable_pred = 0;
            }
            err = 0;
        }
            break;
        case MHVE_CFG_DUMP_REG:
            mhe->p_drpptr = (uint)mcfg->dump_reg.u_phys;
            mhe->p_drvptr = (uint)mcfg->dump_reg.p_vptr;
            mhe->u_drsize = mcfg->dump_reg.i_size;
            err = 0;
            break;
        case MHVE_CFG_BUF:
            mhe->gn_mem = mcfg->hev_buff_addr.gn_mem;               // TXIP intermediate data
            mhe->ppu_int_b = mcfg->hev_buff_addr.ppu_int_b;         // PPU intermediate data
            // (Only needed when low-bandwidth mode) In SRAM
            mhe->imi_ref_y_buf = mcfg->hev_buff_addr.imi_ref_y_buf;
            mhe->imi_ref_c_buf = mcfg->hev_buff_addr.imi_ref_c_buf;
            // In DRAM
            mhe->ppu_int_a = mcfg->hev_buff_addr.ppu_int_a;                           // PPU intermediate data
            break;
        case MHVE_CFG_FME:
            mhe->i_iframe_bitsthr = mcfg->fme.i_iframe_bitsthr;
            mhe->i_pframe_bitsthr = mcfg->fme.i_pframe_bitsthr;
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

    mhe_ops* mhe = (mhe_ops*)mops;
    h265_enc* h265 = mhe->h265;
    int err = -1;

    switch(mcfg->type)
    {
            pps_t* pps;
            sps_t* sps;
        case MHVE_CFG_RES:
            mcfg->res.e_pixf = mhe->e_pixf;
            mcfg->res.i_pixw = mhe->i_pixw;
            mcfg->res.i_pixh = mhe->i_pixh;
            mcfg->res.i_rpbn = mhe->i_rpbn;
            mcfg->res.u_conf = mhe->u_conf;
            err = 0;
            break;
        case MHVE_CFG_MOT:  //from H264
            mcfg->mot.i_subp = mhe->i_subp;
            mcfg->mot.i_dmvx = mhe->i_dmvx;
            mcfg->mot.i_dmvy = mhe->i_dmvy;
            mcfg->mot.i_blkp[0] = mhe->i_blkp[0];
            mcfg->mot.i_blkp[1] = 0;
            err = 0;
            break;
        case MHVE_CFG_HEV:
            pps = h265_find_set(h265, HEVC_PPS, 0);
            sps = pps->p_sps;
            mcfg->hev.i_profile = h265->i_profile;
            mcfg->hev.i_level = h265->i_level;
            mcfg->hev.i_log2_max_cb_size = sps->i_log2_max_cb_size;
            mcfg->hev.i_log2_min_cb_size = sps->i_log2_min_cb_size;
            mcfg->hev.i_log2_max_tr_size = sps->i_log2_max_tr_size;
            mcfg->hev.i_log2_min_tr_size = sps->i_log2_min_tr_size;
            mcfg->hev.i_tr_depth_intra = sps->i_max_tr_hierarchy_depth_intra;
            mcfg->hev.i_tr_depth_inter = sps->i_max_tr_hierarchy_depth_inter;
            mcfg->hev.b_scaling_list_enable = sps->b_scaling_list_enable;
            mcfg->hev.b_sao_enable = sps->b_sao_enabled;
            mcfg->hev.b_strong_intra_smoothing = sps->b_strong_intra_smoothing_enabled;
            mcfg->hev.b_ctu_qp_delta_enable = pps->b_cu_qp_delta_enabled + pps->i_diff_cu_qp_delta_depth;
            mcfg->hev.b_constrained_intra_pred = pps->b_constrained_intra_pred;
            mcfg->hev.i_cb_qp_offset = pps->i_cb_qp_offset;
            mcfg->hev.i_cr_qp_offset = pps->i_cr_qp_offset;
            mcfg->hev.b_deblocking_override_enable = pps->b_deblocking_filter_override_enabled;
            mcfg->hev.b_deblocking_disable = pps->b_deblocking_filter_disabled;
            mcfg->hev.i_tc_offset_div2 = pps->i_tc_offset >> 1;
            mcfg->hev.i_beta_offset_div2 = pps->i_beta_offset >> 1;
            err = 0;
            break;
        case MHVE_CFG_VUI:
        {
            sps = h265_find_set(h265, HEVC_SPS, 0);
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
            mcfg->vui.i_num_ticks_poc_diff_one_minus1 = sps->vui.i_num_ticks_poc_diff_one_minus1;
            err = 0;
        }
        break;
        case MHVE_CFG_LFT:
            mcfg->lft.b_override = (signed char)h265->b_deblocking_override;
            mcfg->lft.b_disable = (signed char)h265->b_deblocking_disable;
            mcfg->lft.i_offsetA = (signed char)h265->i_tc_offset / 2;
            mcfg->lft.i_offsetB = (signed char)h265->i_beta_offset / 2;
            err = 0;
            break;
        case MHVE_CFG_SPL:
            mcfg->spl.i_rows = mhe->i_rows;//h265->i_rows;
            mcfg->spl.i_bits = err = 0;
            break;
        case MHVE_CFG_BAC:
            mcfg->bac.b_init = h265->b_cabac_init != 0;
            err = 0;
            break;
        case MHVE_CFG_LTR:
            sps = h265_find_set(h265, HEVC_SPS, 0);
            if (sps->b_long_term_ref_pics_pres)
            {
                mcfg->ltr.b_long_term_reference = 1;
                mcfg->ltr.b_enable_pred = h265->b_enable_pred;
            }
            else
            {
                mcfg->ltr.b_long_term_reference = 0;
                mcfg->ltr.b_enable_pred = 0;
            }
            err = 0;
            break;
        case MHVE_CFG_BUF:
            mcfg->hev_buff_addr.gn_mem = mhe->gn_mem;
            mcfg->hev_buff_addr.ppu_int_b = mhe->ppu_int_b;
            // (Only needed when low-bandwidth mode) In SRAM
            mcfg->hev_buff_addr.imi_ref_y_buf = mhe->imi_ref_y_buf;
            mcfg->hev_buff_addr.imi_ref_c_buf = mhe->imi_ref_c_buf;
            mcfg->hev_buff_addr.ppu_int_a = mhe->ppu_int_a;
            err = 0;
            break;
        case MHVE_CFG_FME:
            mcfg->fme.i_iframe_bitsthr = mhe->i_iframe_bitsthr;
            mcfg->fme.i_pframe_bitsthr = mhe->i_pframe_bitsthr;
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
    mhe_ops * mhe = (mhe_ops*)mops;
    h265_enc* h265 = mhe->h265;
    h265_enc* h265_bak = mhe->h265_bak;

    rqct_ops* rqct = mhe->p_rqct;
    rqct_ops* rqct_bak = mhe->p_rqct_bak;
    //pmbr_ops* pmbr = mhe->p_pmbr;
    //pmbr_ops* pmbr_bak = mhe->p_pmbr_bak;

    /* backup rec & ref buffer */
    mhe->p_recn_bak =  mhe->p_recn;
    MEM_COPY(&mhe->m_dpbs_bak[0], &mhe->m_dpbs[0], sizeof(rpb_t*)*RPB_MAX_NR);
    MEM_COPY(&mhe->m_pics_bak[0], &mhe->m_pics[0], sizeof(rpb_t)*RPB_MAX_NR);

    /* backup h265 */
    MEM_COPY(h265_bak, h265, sizeof(h265_enc));

    /* backup rqcf */
     rqct->cpy_conf(rqct_bak,rqct);     // src:rqct , dst : rqct_bak

    /* backup pmbr */
    // MEM_COPY(pmbr_bak, pmbr, sizeof(pmbr_ops));

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _OpsRestore
//  Description : Restore this object.
//------------------------------------------------------------------------------
static int _OpsRestore(mhve_ops* mops)
{
    mhe_ops* mhe = (mhe_ops*)mops;
    h265_enc* h265 = mhe->h265;
    h265_enc* h265_bak = mhe->h265_bak;

    rqct_ops* rqct = mhe->p_rqct;
    rqct_ops* rqct_bak = mhe->p_rqct_bak;
    //pmbr_ops* pmbr = mhe->p_pmbr;
    //pmbr_ops* pmbr_bak = mhe->p_pmbr_bak;

    /* restore rec & ref buffer */
    mhe->p_recn = mhe->p_recn_bak;
    MEM_COPY(&mhe->m_dpbs[0], &mhe->m_dpbs_bak[0],  sizeof(rpb_t*)*RPB_MAX_NR);
    MEM_COPY(&mhe->m_pics[0], &mhe->m_pics_bak[0], sizeof(rpb_t)*RPB_MAX_NR);

    /* restore h265 */
    MEM_COPY(h265, h265_bak, sizeof(h265_enc));

    /* restore rqcf */
    rqct->cpy_conf(rqct,rqct_bak);     // // src:rqct_bak , dst : rqct

    /* restore pmbr */
    //MEM_COPY(pmbr, pmbr_bak, sizeof(pmbr_ops));

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : MheOpsAcquire
//  Description : Allocate mhe_ops object and link its member function.
//------------------------------------------------------------------------------
void* MheOpsAcquire(int rqc_id)
{

    mhve_ops* mops = NULL;

    while(NULL != (mops = MEM_ALLC(sizeof(mhe_ops))))
    {
        mhe_ops* mhe = (mhe_ops*)mops;

        /* Link member function */
        mops->release = __OpsFree;
        mops->rqct_ops = _RqctOps;
        mops->pmbr_ops = _PmbrOps;
        mops->mhve_job = _MheJob;
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
        mops->backup   = _OpsBackup;
        mops->restore  = _OpsRestore;

        /* Link RQCT and allocate register mirror structure */
        mhe->p_rqct = RqctMheAcquire(rqc_id);
        mhe->p_pmbr = MhePmbrAllocate();
        mhe->p_regs = MEM_ALLC(sizeof(mhe_reg));
        mhe->h265 = h265enc_acquire();
        if(!mhe->p_regs || !mhe->p_rqct || !mhe->p_pmbr || !mhe->h265)
        {
            break;
        }

        mhe->p_rqct_bak = RqctMheAcquire(rqc_id);
        mhe->p_pmbr_bak = MhePmbrAllocate();
        mhe->h265_bak= h265enc_acquire();
        if(!mhe->p_rqct_bak || !mhe->p_pmbr_bak || !mhe->h265_bak)
            break;

        /* Initialize common parameters */
        mhe->i_refn = 1;
        mhe->i_dmvx = HW_ME_WINDOW_X;
        mhe->i_dmvy = HW_ME_WINDOW_Y;
        mhe->i_subp = QUARTER_PREC;
        mhe->i_blkp[0] = MVBLK_DEFAULT;
        mhe->i_blkp[1] = 0;
        mhe->flush_cache = NULL;

        return mops;
    }

    if(mops)
        mops->release(mops);

    return NULL;
}
//EXPORT_SYMBOL(MheOpsAcquire);
