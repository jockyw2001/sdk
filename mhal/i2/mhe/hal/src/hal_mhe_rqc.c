#include "hal_mhe_ops.h"

#define MRQC_NAME       "MRQC"
#define MRQC_VER_MJR    0
#define MRQC_VER_MNR    0
#define MRQC_VER_EXT    1
#define DQP_MAX    51
#define DQP_MIN    -51

// Mode penalties when RDO cost comparison
//#define PENALTY_BITS   (16)
#define PENALTY_SHIFT  (2)

void* MheMsrcAllocate(void);
char* MheMsrcDescribe(void);

static void* MrqcAllocate(void);
static char* MrqcDescribe(void);

#define MRQC_FACTORIES_NR   8

static struct MrqcFactory_t
{
    void* (*rqc_alloc)(void);
    char* (*rqc_descr)(void);
} factories[MRQC_FACTORIES_NR] =
{
    {MrqcAllocate, MrqcDescribe},
    {MheMsrcAllocate, MheMsrcDescribe},
    {NULL, NULL},
};


static int  _SeqSync(rqct_ops* rqct);
static int  _SeqDone(rqct_ops* rqct);
static int  _SeqConf(rqct_ops* rqct);
static int  _EncConf(rqct_ops* rqct, mhve_job* mjob);
static int  _EncDone(rqct_ops* rqct, mhve_job* mjob);
static int  _CpyConf(rqct_ops*rqct_dst, rqct_ops* rqct_src);
static void _MrqcFree(rqct_ops* rqct)
{
    MEM_FREE(rqct);
}


//------------------------------------------------------------------------------
//  Function    : RqctMheInsert
//  Description : Insert new RQC factory as default factory.
//------------------------------------------------------------------------------
int RqctMheInsert(void * (*allc)(void), char * (*desc)(void))
{
    if(factories[0].rqc_alloc != MrqcAllocate)
        return -1;
    factories[0].rqc_alloc = allc;
    factories[0].rqc_descr = desc;
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : RqctMheRemove
//  Description : Remove RQC factory from default factory.
//------------------------------------------------------------------------------
int RqctMheRemove(void)
{
    if(factories[0].rqc_alloc == MrqcAllocate)
        return -1;
    factories[0].rqc_alloc = MrqcAllocate;
    factories[0].rqc_descr = MrqcDescribe;
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : RqctMheAcquire
//  Description :
//------------------------------------------------------------------------------
void* RqctMheAcquire(int idx)
{
    if((unsigned)idx < MRQC_FACTORIES_NR && factories[idx].rqc_alloc)
        return (factories[idx].rqc_alloc)();
    return (factories[0].rqc_alloc)();
}

//------------------------------------------------------------------------------
//  Function    : RqctMheComment
//  Description :
//------------------------------------------------------------------------------
char* RqctMheComment(int idx)
{
    if((unsigned)idx < MRQC_FACTORIES_NR && factories[idx].rqc_alloc)
        return (factories[idx].rqc_descr)();
    return NULL;
}
//EXPORT_SYMBOL(RqctMheComment);

/* Default rate controller */
//------------------------------------------------------------------------------
//  Function    : MrqcAllocate
//  Description :
//------------------------------------------------------------------------------
static void* MrqcAllocate(void)
{

    rqct_ops* rqct = NULL;
    mhe_rqc* rqcx;

    if(!(rqct = MEM_ALLC(sizeof(mhe_rqc))))
        return NULL;

    MEM_COPY(rqct->name, "mrct", 5);

    /* Link member function */
    rqct->release  = _MrqcFree;
    rqct->seq_sync = _SeqSync;
    rqct->seq_done = _SeqDone;
    rqct->seq_conf = _SeqConf;
    rqct->set_rqcf = MheMrqcSetRqcf;
    rqct->get_rqcf = MheMrqcGetRqcf;
    rqct->enc_buff = MheMrqcEncBuff;
    rqct->enc_conf = _EncConf;
    rqct->enc_done = _EncDone;
    rqct->cpy_conf = _CpyConf;

    /* Initialize Basic RC Attribute */
    rqcx = (mhe_rqc*)rqct;
    rqcx->attr.i_method = RQCT_MODE_CQP;
    rqcx->attr.i_pict_w = 0;
    rqcx->attr.i_pict_h = 0;
    rqcx->attr.n_fmrate = 30;
    rqcx->attr.d_fmrate = 1;
    rqcx->attr.i_leadqp = -1;
    rqcx->attr.i_deltaq = 0;//QP_IFRAME_DELTA;
    rqcx->attr.i_btrate = 0;
    rqcx->attr.i_iupperq = QP_MAX;
    rqcx->attr.i_ilowerq = QP_MIN;
    rqcx->attr.i_pupperq = QP_MAX;
    rqcx->attr.i_plowerq = QP_MIN;

    /* Initialize Penalties */
    rqcx->attr.b_ia8xlose = 0;
    rqcx->attr.b_ir8xlose = 0;
    rqcx->attr.b_ia16lose = 0;
    rqcx->attr.b_ir16lose = 0;
    rqcx->attr.b_ir16mlos = 0;
    rqcx->attr.b_ir16slos = 0;
    rqcx->attr.b_ir16mslos = 0;
    rqcx->attr.b_ia32lose = 0;
    rqcx->attr.b_ir32mlos = 0;
    rqcx->attr.b_ir32mslos = 0;

    rqcx->attr.u_ia8xpen = 0;
    rqcx->attr.u_ir8xpen = 0;
    rqcx->attr.u_ia16pen = 0;
    rqcx->attr.u_ir16pen = 0;
    rqcx->attr.u_ir16mpen = 0;
    rqcx->attr.u_ir16spen = 0;
    rqcx->attr.u_ir16mspen = 0;
    rqcx->attr.u_ia32pen = 0;
    rqcx->attr.u_ir32mpen = 0;
    rqcx->attr.u_ir32mspen = 0;

    rqcx->i_config = 0;
    rqcx->i_pcount = 0;
    rqcx->i_period = 0;
    rqcx->b_passiveI = 0;
    rqcx->b_seqhead = 1;

    return rqcx;

}

//------------------------------------------------------------------------------
//  Function    : MrqcDescribe
//  Description :
//------------------------------------------------------------------------------
static char* MrqcDescribe(void)
{
    static char line[64];
    CamOsSnprintf(line, sizeof(line), "%s@v%d.%d.%02d:fixed qp only.", MRQC_NAME, MRQC_VER_MJR, MRQC_VER_MNR, MRQC_VER_EXT);
    return line;
}

#if 1
//------------------------------------------------------------------------------
//  Function    : _SeqConf
//  Description :
//------------------------------------------------------------------------------
static int  _SeqConf(rqct_ops* rqct)
{

    mhe_rqc* rqcx = (mhe_rqc*)rqct;

    rqcx->i_period = rqcx->attr.i_period;
    //if (rqcx->attr.i_leadqp < 8 || rqcx->attr.i_leadqp > 48)
    if(rqcx->attr.i_leadqp < QP_MIN || rqcx->attr.i_leadqp > QP_MAX)
        rqcx->attr.i_leadqp = 36;

    rqcx->i_pcount = rqcx->i_period;
    rqcx->b_seqhead = 1;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SeqSync
//  Description :
//------------------------------------------------------------------------------
static int  _SeqSync(rqct_ops* rqct)
{

    mhe_rqc* rqcx = (mhe_rqc*)rqct;

    rqcx->i_pcount = 0;
    rqcx->b_seqhead = 1;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SeqDone
//  Description :
//------------------------------------------------------------------------------
static int  _SeqDone(rqct_ops* rqct)
{
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncConf
//  Description :
//------------------------------------------------------------------------------
static int  _EncConf(rqct_ops* rqct, mhve_job* mjob)
{
    mhe_rqc* rqcx = (mhe_rqc*)rqct;
    mhe_reg* regs = (mhe_reg*)mjob;
    int maxqp, minqp;

    rqct->i_enc_qp = rqcx->attr.i_leadqp;
    if(rqct->i_pictyp == RQCT_PICTYP_I)
        rqct->i_enc_qp = rqcx->attr.i_leadqp - rqcx->attr.i_deltaq;

    MheMrqcRoiDraw(rqct, mjob);

    maxqp = IS_IPIC(rqct->i_pictyp) ? (rqcx->attr.i_iupperq) : (rqcx->attr.i_pupperq);
    minqp = IS_IPIC(rqct->i_pictyp) ? (rqcx->attr.i_ilowerq) : (rqcx->attr.i_plowerq);

    if((rqct->i_enc_qp + rqcx->attr.i_offsetqp) >  maxqp)
        rqct->i_enc_qp = maxqp;
    else if((rqct->i_enc_qp + rqcx->attr.i_offsetqp) < minqp)
        rqct->i_enc_qp = minqp;
    else
         rqct->i_enc_qp += rqcx->attr.i_offsetqp;
    rqcx->attr.i_offsetqp = 0;

    // Qp
    regs->hev_bank1.reg_mbr_const_qp_en = 1;
    regs->hev_bank0.reg_hev_ec_qp_delta_enable_flag = 0;
    regs->hev_bank0.reg_hev_slice_qp = rqct->i_enc_qp;

    // TXIP
    /* Initialize Penalties of CTB */
    regs->hev_bank0.reg_hev_txip_cu8_intra_lose              = rqcx->attr.b_ia8xlose;
    regs->hev_bank0.reg_hev_txip_cu8_inter_lose              = rqcx->attr.b_ir8xlose;
    regs->hev_bank0.reg_hev_txip_cu16_intra_lose             = rqcx->attr.b_ia16lose;
    regs->hev_bank0.reg_hev_txip_cu16_inter_mvp_lose         = rqcx->attr.b_ir16lose;
    regs->hev_bank0.reg_hev_txip_cu16_inter_merge_lose       = rqcx->attr.b_ir16mlos;
    regs->hev_bank0.reg_hev_txip_cu16_inter_mvp_nores_lose   = rqcx->attr.b_ir16slos;
    regs->hev_bank0.reg_hev_txip_cu16_inter_merge_nores_lose = rqcx->attr.b_ir16mslos;
    regs->hev_bank0.reg_hev_txip_cu32_intra_lose             = rqcx->attr.b_ia32lose;
    regs->hev_bank0.reg_hev_txip_cu32_inter_merge_lose       = rqcx->attr.b_ir32mlos;
    regs->hev_bank0.reg_hev_txip_cu32_inter_merge_nores_lose = rqcx->attr.b_ir32mslos;

    // Mode penalties when RDO cost comparison
    regs->hev_bank0.reg_hev_txip_cu8_intra_penalty              = rqcx->attr.u_ia8xpen >> PENALTY_SHIFT;
    regs->hev_bank0.reg_hev_txip_cu8_inter_penalty              = rqcx->attr.u_ir8xpen >> PENALTY_SHIFT;
    regs->hev_bank0.reg_hev_txip_cu16_intra_penalty             = rqcx->attr.u_ia16pen >> PENALTY_SHIFT;
    regs->hev_bank0.reg_hev_txip_cu16_inter_mvp_penalty         = rqcx->attr.u_ir16pen >> PENALTY_SHIFT;
    regs->hev_bank0.reg_hev_txip_cu16_inter_merge_penalty       = rqcx->attr.u_ir16mpen >> PENALTY_SHIFT;
    regs->hev_bank0.reg_hev_txip_cu16_inter_mvp_nores_penalty   = rqcx->attr.u_ir16spen >> PENALTY_SHIFT;
    regs->hev_bank0.reg_hev_txip_cu16_inter_merge_nores_penalty = rqcx->attr.u_ir16mspen >> PENALTY_SHIFT;
    regs->hev_bank0.reg_hev_txip_cu32_intra_penalty             = rqcx->attr.u_ia32pen >> PENALTY_SHIFT;
    regs->hev_bank0.reg_hev_txip_cu32_inter_merge_penalty       = rqcx->attr.u_ir32mpen >> PENALTY_SHIFT;
    regs->hev_bank0.reg_hev_txip_cu32_inter_merge_nores_penalty = rqcx->attr.u_ir32mspen >> PENALTY_SHIFT;

    if(!rqcx->attr.b_logoff)
        CamOsSnprintf(rqct->print_line, RQCT_LINE_SIZE - 1, "<%s#%04d@%c:%2d> bitcnt:%8d",
                      rqct->name, rqct->i_enc_nr, IS_IPIC(rqct->i_pictyp) ? 'I' : 'P', rqct->i_enc_qp, rqct->i_bitcnt);


    return 0;
}
#endif
//------------------------------------------------------------------------------
//  Function    : MheMrqcEncBuff
//  Description :
//------------------------------------------------------------------------------
int MheMrqcEncBuff(rqct_ops* rqct, rqct_buf* buff)
{
    mhe_rqc* rqcx = (mhe_rqc*)rqct;

    rqcx->i_config = buff->u_config;

    if(rqcx->i_pcount == 0)
    {
        rqct->seq_conf(rqct);   //rate control initial
    }

    /* Picture-Type Decision */
    if(rqcx->b_seqhead ||
       ((rqcx->i_pcount == rqcx->i_period) && !rqcx->b_passiveI))
    {
        /* I-Frame */
        rqcx->i_pcount = 1;
        rqcx->i_ltrpcount = 1;
        rqcx->i_refcnt = 1;
        rqct->i_pictyp = RQCT_PICTYP_I;
        rqct->b_unrefp = 0;
        rqcx->b_seqhead = 0;
    }
    else
    {
        rqcx->i_pcount++;
        if (rqcx->i_ltrperiod != 0 && rqcx->i_ltrpcount >= rqcx->i_ltrperiod)
        {
            //CamOsPrintf("%s-LTR period(%d), count(%d)\n", __func__, rqcx->i_ltrperiod, rqcx->i_ltrpcount);
            /* LTR P-frame */
            rqct->i_pictyp = RQCT_PICTYP_LTRP;
            rqcx->i_ltrpcount = 1;
        }
        else
        {
            /* P-frame */
            rqct->i_pictyp = RQCT_PICTYP_P;
            if (rqcx->i_ltrperiod)
                rqcx->i_ltrpcount++;
            if(rqct->b_unrefp)
                rqct->b_unrefp = !(1 & rqcx->i_refcnt); // EROY CHECK
            rqcx->b_seqhead = 0;
        }

    }

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncDone
//  Description :
//------------------------------------------------------------------------------
static int  _EncDone(rqct_ops* rqct, mhve_job* mjob)
{

    mhe_rqc* rqcx = (mhe_rqc*)rqct;
    mhe_reg* regs = (mhe_reg*)mjob;

    regs->hev_bank1.reg6d = regs->hev_bank1.reg6e = 0;
    regs->mhe_bank0.reg74 = regs->mhe_bank0.reg75 = 0;

    rqct->i_bitcnt  = mjob->i_bits;
    rqct->i_enc_bs += mjob->i_bits / 8;
    rqct->i_enc_nr++;
    rqcx->i_refcnt++;

    return 0;
}

static int _CpyConf(rqct_ops* rqct_dst,rqct_ops* rqct_src)
{
    mhe_rqc* rqcx_dst = (mhe_rqc*)rqct_dst;
    mhe_rqc* rqcx_src = (mhe_rqc*)rqct_src;

    MEM_COPY(rqcx_dst, rqcx_src, sizeof(mhe_rqc));

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : MheMrqcSetRqcf
//  Description :
//------------------------------------------------------------------------------
int MheMrqcSetRqcf(rqct_ops* rqct, rqct_cfg* rqcf)
{
    int err = -1;
    int i;

    mhe_rqc* rqcx = (mhe_rqc*)rqct;

    switch(rqcf->type)
    {
        case RQCT_CFG_SEQ:
            if((unsigned)rqcf->seq.i_method > RQCT_MODE_VBR)
                break;
            rqcx->attr.i_method = rqcf->seq.i_method;
            rqcx->attr.i_period = rqcf->seq.i_period;
            rqcx->attr.i_btrate = rqcf->seq.i_btrate;
            rqcx->attr.i_leadqp = rqcf->seq.i_leadqp;
            rqcx->i_pcount = 0;
            rqcx->b_passiveI = rqcf->seq.b_passiveI;
            rqcx->b_seqhead = 1;
            err = 0;
            break;
        case RQCT_CFG_LTR:
            rqcx->attr.i_ltrperiod = rqcf->ltr.i_period;
            rqcx->i_ltrperiod = rqcx->attr.i_ltrperiod;
    //        rqcx->i_ltrpcount = 0;
            err = 0;
            break;
        case RQCT_CFG_DQP:
            rqcx->attr.i_deltaq = MAX(MIN(rqcf->dqp.i_dqp, DQP_MAX), DQP_MIN);
            err = 0;
            break;
        case RQCT_CFG_QPR:
            if(rqcf->qpr.i_iupperq < rqcf->qpr.i_ilowerq)
            {
                err = -1;
                break;
            }
            if(rqcf->qpr.i_pupperq < rqcf->qpr.i_plowerq)
            {
                err = -1;
                break;
            }
            rqcx->attr.i_iupperq = MAX(MIN(rqcf->qpr.i_iupperq, QP_MAX), QP_MIN);
            rqcx->attr.i_ilowerq = MAX(MIN(rqcf->qpr.i_ilowerq, QP_MAX), QP_MIN);
            rqcx->attr.i_pupperq = MAX(MIN(rqcf->qpr.i_pupperq, QP_MAX), QP_MIN);
            rqcx->attr.i_plowerq = MAX(MIN(rqcf->qpr.i_plowerq, QP_MAX), QP_MIN);
            err = 0;
            break;
        case RQCT_CFG_OFT:
            rqcx->attr.i_offsetqp = rqcf->oft.i_offsetqp;
            err = 0;
            break;
        case RQCT_CFG_LOG:
            rqcx->attr.b_logoff = !rqcf->log.b_logm;
            err = 0;
            break;
            /* Private Config */
        case RQCT_CFG_RES:
            rqcx->attr.i_pict_w = rqcf->res.i_picw;
            rqcx->attr.i_pict_h = rqcf->res.i_pich;
            err = 0;
            break;
        case RQCT_CFG_FPS:
            rqcx->attr.n_fmrate = rqcf->fps.n_fps;
            rqcx->attr.d_fmrate = rqcf->fps.d_fps;
            err = 0;
            break;
        case RQCT_CFG_ROI:
            err = 0;

            if(-1 == rqcf->roi.i_roiidx)
            {
                memset(rqcx->attr.i_roiqp, 0, sizeof(rqcx->attr.i_roiqp));
                break;
            }
            if(rqcf->roi.i_roiqp == 0 ||
                    (unsigned)rqcf->roi.i_posx > (unsigned)rqcx->attr.i_dqmw || // EROY CHECK
                    (unsigned)rqcf->roi.i_posy > (unsigned)rqcx->attr.i_dqmh ||
                    rqcf->roi.i_recw <= 0 ||
                    rqcf->roi.i_rech <= 0)
            {
                rqcx->attr.i_roiqp[rqcf->roi.i_roiidx] = 0;
                rqcx->attr.u_roienb &= ~(1 << rqcf->roi.i_roiidx);
                break;
            }
            if(rqcf->roi.i_recw > (rqcx->attr.i_dqmw - rqcf->roi.i_posx))
                rqcf->roi.i_recw = (rqcx->attr.i_dqmw - rqcf->roi.i_posx);
            if(rqcf->roi.i_rech > (rqcx->attr.i_dqmh - rqcf->roi.i_posy))
                rqcf->roi.i_rech = (rqcx->attr.i_dqmh - rqcf->roi.i_posy);
            rqcx->attr.i_absqp[rqcf->roi.i_roiidx] = rqcf->roi.i_absqp;
            rqcx->attr.i_roiqp[rqcf->roi.i_roiidx] = rqcf->roi.i_roiqp;
            rqcx->attr.m_roirec[rqcf->roi.i_roiidx].i_posx = rqcf->roi.i_posx;
            rqcx->attr.m_roirec[rqcf->roi.i_roiidx].i_posy = rqcf->roi.i_posy;
            rqcx->attr.m_roirec[rqcf->roi.i_roiidx].i_recw = rqcf->roi.i_recw;
            rqcx->attr.m_roirec[rqcf->roi.i_roiidx].i_rech = rqcf->roi.i_rech;

            rqcx->attr.u_roienb |= (1 << rqcf->roi.i_roiidx);

            if(rqcx->attr.u_roienb)
            {
                rqcx->attr.b_dqmstat = -1;
            }
            break;
        case RQCT_CFG_BGF:
            rqcx->attr.i_roibgsrcfmrate = rqcf->bgf.i_bgsrcfps;
            rqcx->attr.i_roibgdstfmrate = rqcf->bgf.i_bgdstfps;

            err = 0;
            break;
        case RQCT_CFG_QPM:
            rqcx->attr.u_qpmenb = rqcf->qpm.i_qpmenb;
            if(rqcx->attr.u_qpmenb)
            {
                rqcx->attr.b_dqmstat = -1;
            }
            for(i = 0; i < 14; i++)
                rqcx->attr.i_qpmentry[i] = rqcf->qpm.i_entry[i];
            rqcx->attr.p_qpmkptr = rqcf->qpm.p_kptr;
            break;
        case RQCT_CFG_DQM:
            rqcx->attr.u_dqmphys = rqcf->dqm.u_phys;
            rqcx->attr.p_dqmkptr = rqcf->dqm.p_kptr;
            rqcx->attr.i_dqmsize = rqcf->dqm.i_size;
            rqcx->attr.i_dqmw = rqcf->dqm.i_dqmw;
            rqcx->attr.i_dqmh = rqcf->dqm.i_dqmh;
            err = 0;
            break;
        case RQCT_CFG_PEN:
            rqcx->attr.b_ia8xlose = rqcf->pen.b_ia8xlose;
            rqcx->attr.b_ir8xlose = rqcf->pen.b_ir8xlose;
            rqcx->attr.b_ia16lose = rqcf->pen.b_ia16lose;
            rqcx->attr.b_ir16lose = rqcf->pen.b_ir16lose;
            rqcx->attr.b_ir16mlos = rqcf->pen.b_ir16mlos;
            rqcx->attr.b_ir16slos = rqcf->pen.b_ir16slos;
            rqcx->attr.b_ir16mslos = rqcf->pen.b_ir16mslos;
            rqcx->attr.b_ia32lose = rqcf->pen.b_ia32lose;
            rqcx->attr.b_ir32mlos = rqcf->pen.b_ir32mlos;
            rqcx->attr.b_ir32mslos = rqcf->pen.b_ir32mslos;

            rqcx->attr.u_ia8xpen = rqcf->pen.u_ia8xpen;
            rqcx->attr.u_ir8xpen = rqcf->pen.u_ir8xpen;
            rqcx->attr.u_ia16pen = rqcf->pen.u_ia16pen;
            rqcx->attr.u_ir16pen = rqcf->pen.u_ir16pen;
            rqcx->attr.u_ir16mpen = rqcf->pen.u_ir16mpen;
            rqcx->attr.u_ir16spen = rqcf->pen.u_ir16spen;
            rqcx->attr.u_ir16mspen = rqcf->pen.u_ir16mspen;
            rqcx->attr.u_ia32pen = rqcf->pen.u_ia32pen;
            rqcx->attr.u_ir32mpen = rqcf->pen.u_ir32mpen;
            rqcx->attr.u_ir32mspen = rqcf->pen.u_ir32mspen;

            err = 0;
            break;
        default:
            break;
    }

    return err;
}

//------------------------------------------------------------------------------
//  Function    : MheMrqcGetRqcf
//  Description :
//------------------------------------------------------------------------------
int MheMrqcGetRqcf(rqct_ops* rqct, rqct_cfg* rqcf)
{

    mhe_rqc* rqcx = (mhe_rqc*)rqct;
    int err = -1;
    int i;

    switch(rqcf->type)
    {
        case RQCT_CFG_SEQ:
            rqcf->seq.i_method = rqcx->attr.i_method;
            rqcf->seq.i_period = rqcx->attr.i_period;
            rqcf->seq.i_btrate = rqcx->attr.i_btrate;
            rqcf->seq.i_leadqp = rqcx->attr.i_leadqp;
            rqcf->seq.b_passiveI = rqcx->b_passiveI;
            err = 0;
            break;
        case RQCT_CFG_LTR:
            rqcf->ltr.i_period = rqcx->attr.i_ltrperiod;
            err = 0;
            break;
        case RQCT_CFG_DQP:
            rqcf->dqp.i_dqp = rqcx->attr.i_deltaq;
            err = 0;
            break;
        case RQCT_CFG_QPR:
            rqcf->qpr.i_iupperq = rqcx->attr.i_iupperq;
            rqcf->qpr.i_ilowerq = rqcx->attr.i_ilowerq;
            rqcf->qpr.i_pupperq = rqcx->attr.i_pupperq;
            rqcf->qpr.i_plowerq = rqcx->attr.i_plowerq;
            err = 0;
            break;
        case RQCT_CFG_OFT:
            rqcf->oft.i_offsetqp = rqcx->attr.i_offsetqp;
            err = 0;
            break;
        case RQCT_CFG_LOG:
            rqcf->log.b_logm = !rqcx->attr.b_logoff;
            err = 0;
            break;
            /* Private Config */
        case RQCT_CFG_RES:
            rqcf->res.i_picw = rqcx->attr.i_pict_w;
            rqcf->res.i_pich = rqcx->attr.i_pict_h;
            err = 0;
            break;
        case RQCT_CFG_FPS:
            rqcf->fps.n_fps = rqcx->attr.n_fmrate;
            rqcf->fps.d_fps = rqcx->attr.d_fmrate;
            err = 0;
            break;
        case RQCT_CFG_ROI:
            if(rqcf->roi.i_roiidx < 0 || rqcf->roi.i_roiidx >= RQCT_ROI_NR)
            {
                err = -1;
                break;
            }
            rqcf->roi.i_absqp = rqcx->attr.i_absqp[rqcf->roi.i_roiidx];
            rqcf->roi.i_roiqp = rqcx->attr.i_roiqp[rqcf->roi.i_roiidx];
            rqcf->roi.i_posx = rqcx->attr.m_roirec[rqcf->roi.i_roiidx].i_posx;
            rqcf->roi.i_posy = rqcx->attr.m_roirec[rqcf->roi.i_roiidx].i_posy;
            rqcf->roi.i_recw = rqcx->attr.m_roirec[rqcf->roi.i_roiidx].i_recw;
            rqcf->roi.i_rech = rqcx->attr.m_roirec[rqcf->roi.i_roiidx].i_rech;
            rqcf->roi.u_roienb = rqcx->attr.u_roienb;
            err = 0;
            break;
        case RQCT_CFG_BGF:
            rqcf->bgf.i_bgsrcfps = rqcx->attr.i_roibgsrcfmrate;
            rqcf->bgf.i_bgdstfps = rqcx->attr.i_roibgdstfmrate;

            err = 0;
            break;
        case RQCT_CFG_QPM:
            rqcf->qpm.i_qpmenb = rqcx->attr.u_qpmenb;
            for(i = 0; i < 14; i++)
                rqcf->qpm.i_entry[i] = rqcx->attr.i_qpmentry[i];
            rqcf->qpm.p_kptr = rqcx->attr.p_qpmkptr;
            break;
        case RQCT_CFG_DQM:
            rqcf->dqm.u_phys = rqcx->attr.u_dqmphys;
            rqcf->dqm.p_kptr = rqcx->attr.p_dqmkptr;
            rqcf->dqm.i_dqmw = rqcx->attr.i_dqmw;
            rqcf->dqm.i_dqmh = rqcx->attr.i_dqmh;
            rqcf->dqm.i_size = rqcx->attr.i_dqmsize;
            err = 0;
            break;
        case RQCT_CFG_PEN:
            rqcf->pen.b_ia8xlose = rqcx->attr.b_ia8xlose;
            rqcf->pen.b_ir8xlose = rqcx->attr.b_ir8xlose;
            rqcf->pen.b_ia16lose = rqcx->attr.b_ia16lose;
            rqcf->pen.b_ir16lose = rqcx->attr.b_ir16lose;
            rqcf->pen.b_ir16mlos = rqcx->attr.b_ir16mlos;
            rqcf->pen.b_ir16slos = rqcx->attr.b_ir16slos;
            rqcf->pen.b_ir16mslos = rqcx->attr.b_ir16mslos;
            rqcf->pen.b_ia32lose = rqcx->attr.b_ia32lose;
            rqcf->pen.b_ir32mlos = rqcx->attr.b_ir32mlos;
            rqcf->pen.b_ir32mslos = rqcx->attr.b_ir32mslos;

            rqcf->pen.u_ia8xpen = rqcx->attr.u_ia8xpen;
            rqcf->pen.u_ir8xpen = rqcx->attr.u_ir8xpen;
            rqcf->pen.u_ia16pen = rqcx->attr.u_ia16pen;
            rqcf->pen.u_ir16pen = rqcx->attr.u_ir16pen;
            rqcf->pen.u_ir16mpen = rqcx->attr.u_ir16mpen;
            rqcf->pen.u_ir16spen = rqcx->attr.u_ir16spen;
            rqcf->pen.u_ir16mspen = rqcx->attr.u_ir16mspen;
            rqcf->pen.u_ia32pen = rqcx->attr.u_ia32pen;
            rqcf->pen.u_ir32mpen = rqcx->attr.u_ir32mpen;
            rqcf->pen.u_ir32mspen = rqcx->attr.u_ir32mspen;
            err = 0;
            break;
        default:
            break;
    }
    return err;
}
