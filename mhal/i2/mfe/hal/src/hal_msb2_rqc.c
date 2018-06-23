#include "hal_mfe_ops.h"
#include "hal_mfe_utl.h"

/************************************************************************/
/* Version Declaration                                                  */
/************************************************************************/

#define MSB2_NAME               "MSB2"
#define MSB2_VER_MJR            1
#define MSB2_VER_MNR            1
#define MSB2_VER_EXT            01
#define _EXP(expr)              #expr
#define _STR(expr)              _EXP(expr)

/************************************************************************/
/* Macros                                                               */
/************************************************************************/

#define UP_QP_DIFF_LIMIT        3
#define LF_QP_DIFF_LIMIT        3

#define LOGOFF_DEFAULT          1
#define LOWERQ_DEFAULT          12
#define UPPERQ_DEFAULT          48
#define BOUNDQ_DEFAULT          -1

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/

/************************************************************************/
/* Constant                                                             */
/************************************************************************/

#define COMPLEX_I   (1<<24)
#define COMPLEX_P   (1<<20)

/************************************************************************/
/* Local structures                                                     */
/************************************************************************/

#define PICTYPES 2

typedef struct Msb2Rqc_t
{
    mfe6_rqc    rqcx;
    int     i_method;           /* Rate Control Method */
    int     i_btrate;           /* Bitrate */
    int     i_levelq;           /* Frame Level QP */
    int     i_deltaq;           /* QP Delta Value */
    int     n_fmrate;           /* Frame Rate Numerator */
    int     d_fmrate;           /* Frame Rate Denominator */
    int     i_fmrate;           /* Frame Rate */
    int     i_pixels;           /* Picture Pixel Count */
    int     i_blocks;           /* Picture MB Count */
    short   b_mbkadp, i_limitq;
    short   i_frmdqp, i_blkdqp;
    short   i_iupperq, i_ilowerq;
    short   i_pupperq, i_plowerq;
    int     i_gopbit;           /* Target GOP Bit Count */
    int     i_frmbit;           /* Target Frame Bit Count */
    int     i_pixbit;           /* Target Pixel Bit Count */
    int     i_budget;           /* Frame Budget Bit Count */
    int     i_ipbias;
    int     i_smooth;
    int     i_bucket;
    int     i_upperb, i_lowerb;
    int     i_margin;
    int     i_radius;
    int     i_degree;
    int     i_errpro;
    int     i_imbase;
    int     i_imbits;           // bit-pos:imaginary
    int     i_rebits;           // bit-pos:real

    iir_t   iir_rqprod[PICTYPES];
    acc_t   acc_rqprod;
    acc_t   acc_bitcnt;
    int     i_intrabit;

} msb2_rqc;

/************************************************************************/
/* Local prototypes                                                     */
/************************************************************************/

char* Msb2Describe(void);

static int  _Msb2SeqSync(rqct_ops*);
static int  _Msb2SeqDone(rqct_ops*);
static int  _Msb2SeqConf(rqct_ops*);
static int  _Msb2EncConf(rqct_ops*, mhve_job*);
static int  _Msb2EncDone(rqct_ops*, mhve_job*);
static int  _Msb2CpyConf(rqct_ops*, rqct_ops*);

static void _Msb2OpsFree(rqct_ops* rqct)
{
    MEM_FREE(rqct);
}

/************************************************************************/
/* Functions                                                            */
/************************************************************************/

//------------------------------------------------------------------------------
//  Function    : Msb2Allocate
//  Description :
//------------------------------------------------------------------------------
void* Msb2Allocate(void)
{
    rqct_ops* rqct = NULL;
    mfe6_rqc* rqcx;
    msb2_rqc* msb2;

    if (!(rqct = MEM_ALLC(sizeof(msb2_rqc))))
        return NULL;

    MEM_COPY(rqct->name, MSB2_NAME, 5);

    /* Link member function */
    rqct->release  = _Msb2OpsFree;
    rqct->seq_sync = _Msb2SeqSync;
    rqct->seq_done = _Msb2SeqDone;
    rqct->set_rqcf = MrqcSetRqcf;
    rqct->get_rqcf = MrqcGetRqcf;
    rqct->seq_conf = _Msb2SeqConf;
    rqct->enc_buff = MrqcEncBuff;
    rqct->enc_conf = _Msb2EncConf;
    rqct->enc_done = _Msb2EncDone;
    rqct->cpy_conf = _Msb2CpyConf;
    rqct->i_enc_nr = 0;
    rqct->i_enc_bs = 0;

    /* Initialize Basic RC Attribute */
    rqcx = (mfe6_rqc*)rqct;
    rqcx->attr.i_pict_w = 0;
    rqcx->attr.i_pict_h = 0;
    rqcx->attr.i_method = RQCT_MODE_CQP;
    rqcx->attr.i_btrate = 0;
    rqcx->attr.i_leadqp = -1;
    rqcx->attr.i_deltaq = QP_IFRAME_DELTA;
    rqcx->attr.i_iupperq = QP_UPPER;
    rqcx->attr.i_ilowerq = QP_LOWER;
    rqcx->attr.i_pupperq = QP_UPPER;
    rqcx->attr.i_plowerq = QP_LOWER;
    rqcx->attr.n_fmrate = 30;
    rqcx->attr.d_fmrate = 1;
    rqcx->attr.i_period = 0;
    rqcx->attr.b_logoff = LOGOFF_DEFAULT;

    /* Initialize Penalties */
    rqcx->attr.b_i16pln = 1;
    rqcx->attr.i_peni4x = 0;
    rqcx->attr.i_peni16 = 0;
    rqcx->attr.i_penint = 0;
    rqcx->attr.i_penYpl = 0;
    rqcx->attr.i_penCpl = 0;
    rqcx->i_config = 0;
    rqcx->i_pcount = 0;
    rqcx->i_period = 0;
    rqcx->b_passiveI = 0;
    rqcx->b_seqhead = 1;

    msb2 = (msb2_rqc*)rqcx;
    msb2->i_levelq = 36;

    CamOsPrintf("%s\n",Msb2Describe());

    return rqct;
}

//------------------------------------------------------------------------------
//  Function    : Msb2Describe
//  Description :
//------------------------------------------------------------------------------
char* Msb2Describe(void)
{
    static char line[64];
    CamOsSnprintf(line, sizeof(line), "%s@v%d.%d-%02d:r&d analysis.", MSB2_NAME,MSB2_VER_MJR,MSB2_VER_MNR,MSB2_VER_EXT);
    return line;
}

#define IPR_BIT     (16)
#define IPR_FAC     (1<<IPR_BIT)
#define BPP_BIT     (16)
#define BPP_FAC     (1<<BPP_BIT)
#define QSC_BIT     (5)
#define QSCALE(v)   (((v)+(512>>(QSC_BIT)))>>(10-(QSC_BIT)))

static int qscale[]=
{
    QSCALE(   645),QSCALE(   724),QSCALE(   813),QSCALE(   912),QSCALE(  1024),
    QSCALE(  1149),QSCALE(  1290),QSCALE(  1448),QSCALE(  1625),QSCALE(  1825),QSCALE(  2048),
    QSCALE(  2299),QSCALE(  2580),QSCALE(  2896),QSCALE(  3251),QSCALE(  3649),QSCALE(  4096),
    QSCALE(  4598),QSCALE(  5161),QSCALE(  5793),QSCALE(  6502),QSCALE(  7298),QSCALE(  8192),
    QSCALE(  9195),QSCALE( 10321),QSCALE( 11585),QSCALE( 13004),QSCALE( 14596),QSCALE( 16384),
    QSCALE( 18390),QSCALE( 20643),QSCALE( 23170),QSCALE( 26008),QSCALE( 29193),QSCALE( 32768),
    QSCALE( 36781),QSCALE( 41285),QSCALE( 46341),QSCALE( 52016),QSCALE( 58386),QSCALE( 65536),
    QSCALE( 73562),QSCALE( 82570),QSCALE( 92682),QSCALE(104032),QSCALE(116772),QSCALE(131072),
    QSCALE(147123),QSCALE(165140),QSCALE(185364),QSCALE(208064),QSCALE(233544),QSCALE(262144),
};

int qp2qs(int qp)
{
    if (qp < 0)  return qscale[ 0];
    if (qp > 51)  return 2147483647;
    return qscale[qp];
}

static int qindex[]=
{
    4,10,16,16,22,22,22,22,28,28,28,28,28,28,28,28,
    34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,
    40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,
    40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,
    46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,
    46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,
    46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,
    46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,
    52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
    52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
    52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
    52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
    52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
    52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
    52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
    52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
};

static int qs2qp(unsigned long qs)
{
    int idx, cost, best;
    if (qs<qscale[ 0])  return  0;
    if (qs>qscale[51])  return 51;
    idx = qindex[qs>>QSC_BIT];
    best = (qscale[idx]-qs);
    while (0<(cost=(qscale[--idx]-qs)))
        best=cost;
    if (best<-cost)
        return ++idx;
    return idx;
}

#define DQBIAS_BIT  15
#define DQBIAS_FAC  (1<<DQBIAS_BIT)
#define DQBIAS(r)   (((r)+(1<<(15-(DQBIAS_BIT))))>>(16-DQBIAS_BIT))

static const int dqplot[] =
{
    DQBIAS(    0),//0.0000000000
    DQBIAS(13889),//0.2119255505
    DQBIAS(22749),//0.3471209568
    DQBIAS(28655),//0.4372439481
    DQBIAS(33009),//0.5036719007
    DQBIAS(36440),//0.5560238879
    DQBIAS(39266),//0.5991464061
    DQBIAS(41666),//0.6357766516
    DQBIAS(43752),//0.6676004651
    DQBIAS(45595),//0.6957262149
    DQBIAS(47246),//0.7209202303
    DQBIAS(48741),//0.7437338663
    DQBIAS(50107),//0.7645767803
    DQBIAS(51365),//0.7837615271
    DQBIAS(52529),//0.8015319306
    DQBIAS(53614),//0.8180818208
    DQBIAS(54629),//0.8335678038
    DQBIAS(55582),//0.8481182052
    DQBIAS(56482),//0.8618394843
    DQBIAS(57332),//0.8748209289
    DQBIAS(58139),//0.8871381559
    DQBIAS(58907),//0.8988557638
    DQBIAS(59640),//0.9100293726
    DQBIAS(60339),//0.9207072127
    DQBIAS(61010),//0.9309313772
    DQBIAS(61652),//0.9407388203
    DQBIAS(62270),//0.9501621606
    DQBIAS(62864),//0.9592303332
    DQBIAS(63437),//0.9679691242
    DQBIAS(63989),//0.9764016112
    DQBIAS(64523),//0.9845485296
    DQBIAS(65040),//0.9924285792
};

static int qdelta(int r)
{
    int d = 0, s = r<0;
    r = s?-r:r;
    while (r > dqplot[++d]) ;
    --d;
    return s?-d:d;
}

#define MAX_SUMQS   ((2UL<<31)-1)
#define ALIGN2MB(l) (((l)+15)>>4)
static int _CalcLimitQ(int mbs)
{
    unsigned long qs = MAX_SUMQS/(mbs-1);
    int qp = qs2qp(qs);
    return qs<qp2qs(qp) ? qp-1 : qp;
}

//------------------------------------------------------------------------------
//  Function    : _Msb2SeqDone
//  Description :
//------------------------------------------------------------------------------
static int _Msb2SeqDone(rqct_ops* rqct)
{
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _Msb2SeqSync
//  Description :
//------------------------------------------------------------------------------
static int _Msb2SeqSync(rqct_ops* rqct)
{
    mfe6_rqc* rqcx = (mfe6_rqc*)rqct;

    rqcx->i_pcount = 0;
    rqcx->i_ltrpcount = 0;
    rqcx->b_seqhead = 1;

    return 0;
}

#define SMOOTH_X    (16)

static int _Msb2SeqConf(rqct_ops* rqct)
{
    mfe6_rqc* rqcx = (mfe6_rqc*)rqct;
    msb2_rqc* msb2 = (msb2_rqc*)rqcx;
    short upperq = rqcx->attr.i_iupperq>rqcx->attr.i_pupperq?rqcx->attr.i_iupperq:rqcx->attr.i_pupperq;
    short lowerq = rqcx->attr.i_ilowerq<rqcx->attr.i_plowerq?rqcx->attr.i_ilowerq:rqcx->attr.i_plowerq;
    int64 remainder;

    msb2->i_method = (int)rqcx->attr.i_method;
    msb2->i_btrate = (int)rqcx->attr.i_btrate;
    if (rqcx->attr.i_leadqp > 8 && rqcx->attr.i_leadqp < 48)
        msb2->i_levelq = (int)rqcx->attr.i_leadqp;
    msb2->i_deltaq = (int)rqcx->attr.i_deltaq;
    msb2->n_fmrate = (int)rqcx->attr.n_fmrate;
    msb2->d_fmrate = (int)rqcx->attr.d_fmrate;
    msb2->i_pixels = (int)rqcx->attr.i_pict_w * rqcx->attr.i_pict_h;
    msb2->i_fmrate = (msb2->n_fmrate + msb2->n_fmrate-1) / msb2->d_fmrate;
    msb2->i_blocks = ALIGN2MB(rqcx->attr.i_pict_w) * ALIGN2MB(rqcx->attr.i_pict_h);
    msb2->i_limitq = _CalcLimitQ(msb2->i_blocks);
    msb2->i_iupperq = MAX(MIN(rqcx->attr.i_iupperq, QP_MAX), QP_MIN);
    msb2->i_ilowerq = MAX(MIN(rqcx->attr.i_ilowerq, QP_MAX), QP_MIN);
    msb2->i_pupperq = MAX(MIN(rqcx->attr.i_pupperq, QP_MAX), QP_MIN);
    msb2->i_plowerq = MAX(MIN(rqcx->attr.i_plowerq, QP_MAX), QP_MIN);
    if (msb2->i_btrate == 0)
        msb2->i_method = RQCT_MODE_CQP;
    rqcx->i_pcount =
        rqcx->i_period = (int)rqcx->attr.i_period;
    rqcx->i_ltrpcount =
    rqcx->i_ltrperiod = (int)rqcx->attr.i_ltrperiod;
    rqcx->b_seqhead = 1;

    msb2->i_smooth = rqcx->i_period*2;
    msb2->i_frmbit = (int)CamOsMathDivS64((int64)msb2->i_btrate*msb2->d_fmrate,msb2->n_fmrate, &remainder);
    msb2->i_pixbit = (int)CamOsMathDivS64((int64)msb2->i_frmbit*BPP_FAC,msb2->i_pixels, &remainder);
    msb2->i_gopbit = rqcx->i_period*msb2->i_frmbit;
    msb2->b_mbkadp = 0;
    msb2->i_blkdqp = 4;
    msb2->i_frmdqp = 1;

    msb2->i_degree = msb2->i_btrate/(QP_RANGE*2);
    msb2->i_margin = msb2->i_degree*(QP_RANGE);
    msb2->i_bucket = msb2->i_margin<<1;
    msb2->i_radius = msb2->i_margin>>1;
    msb2->i_rebits =
        msb2->i_imbits =
            msb2->i_imbase = msb2->i_margin+(msb2->i_levelq-QP_LOWER)*msb2->i_degree;
    msb2->i_lowerb = msb2->i_margin+(lowerq-QP_LOWER)*msb2->i_degree;
    msb2->i_upperb = msb2->i_margin+(upperq-QP_LOWER)*msb2->i_degree;

    iirInit(&msb2->iir_rqprod[RQCT_PICTYP_I], COMPLEX_I, KAPA_ONE*3/4);
    iirInit(&msb2->iir_rqprod[RQCT_PICTYP_P], COMPLEX_P, KAPA_ONE*2/4);
    accInit(&msb2->acc_rqprod, COMPLEX_P);
    accInit(&msb2->acc_bitcnt, msb2->i_frmbit*3/4);

    return 0;
}

static int _Msb2EncConf(rqct_ops* rqct, mhve_job* mjob)
{
    mfe6_rqc* rqcx = (mfe6_rqc*)rqct;
    mfe6_reg* regs = (mfe6_reg*)mjob;
    msb2_rqc* msb2 = (msb2_rqc*)rqcx;
    int qsc, bpp, pqp, dqp, lim, tgt=0;
    int uqp = QP_MAX;
    int lqp = QP_MIN;
    int64 remainder;
    int maxqp, minqp;

    switch (msb2->i_method)
    {
        case RQCT_MODE_CQP:
            rqct->i_enc_qp = msb2->i_levelq;
            if (IS_IPIC(rqct->i_pictyp))
                rqct->i_enc_qp -= msb2->i_deltaq;
            break;
        case RQCT_MODE_CBR:
        case RQCT_MODE_VBR:
        default:
            lim = msb2->i_limitq;

            if (IS_IPIC(rqct->i_pictyp))
            {
                int xi = iirData(&msb2->iir_rqprod[RQCT_PICTYP_I]);
                int xp = accCalc(&msb2->acc_rqprod,SMOOTH_X)/SMOOTH_X;
                int ri = (int)CamOsMathDivS64((int64)qp2qs(40+msb2->i_deltaq)*IPR_FAC,qp2qs(40), &remainder);
                int rp = (int)CamOsMathDivS64((int64)xp*IPR_FAC,xi, &remainder);

                uqp = msb2->i_iupperq;
                lqp = msb2->i_ilowerq;
                msb2->i_budget = (int)CamOsMathDivS64((int64)msb2->i_gopbit*ri,(rqcx->i_period*rp+ri-rp), &remainder);
                bpp = (int)CamOsMathDivS64((int64)msb2->i_budget*BPP_FAC,msb2->i_pixels, &remainder);
                qsc = xi/bpp;
                pqp = _MAX(qs2qp(qsc),msb2->i_levelq-msb2->i_deltaq);
                rqct->i_enc_qp = pqp = _MAX(lqp,_MIN(pqp,uqp));
                tgt = msb2->i_budget/msb2->i_blocks;
                uqp = _MIN(pqp+msb2->i_blkdqp,_MIN(lim,uqp));
                lqp = _MAX(pqp-msb2->i_blkdqp,lqp);
                msb2->b_mbkadp = msb2->i_blkdqp>0 && pqp<lim;
                msb2->i_ipbias = 0;
                break;
            }

            uqp = msb2->i_pupperq;
            lqp = msb2->i_plowerq;
            pqp = QP_LOWER+(msb2->i_imbase-msb2->i_margin)/msb2->i_degree;
            dqp = qdelta((int)CamOsMathDivS64((int64)(msb2->i_imbits-msb2->i_imbase)*DQBIAS_FAC,msb2->i_radius, &remainder));
            msb2->i_levelq = pqp;
            msb2->i_budget = msb2->i_frmbit-msb2->i_ipbias;
            msb2->i_budget = _MAX(msb2->i_budget,msb2->i_frmbit/16);
            rqct->i_enc_qp = pqp = _MAX(_MIN(uqp,pqp+dqp),lqp);
            tgt = msb2->i_budget/msb2->i_blocks;
            uqp = _MIN(pqp+msb2->i_blkdqp,_MIN(lim,uqp));
            lqp = _MAX(pqp-msb2->i_blkdqp,lqp);
            msb2->b_mbkadp = msb2->i_blkdqp>0 && pqp<lim;
            break;
    }

    maxqp = IS_IPIC(rqct->i_pictyp) ? (rqcx->attr.i_iupperq) : (rqcx->attr.i_pupperq);
    minqp = IS_IPIC(rqct->i_pictyp) ? (rqcx->attr.i_ilowerq) : (rqcx->attr.i_plowerq);

    if((rqct->i_enc_qp + rqcx->attr.i_offsetqp) >  maxqp)
        rqct->i_enc_qp = maxqp;
    else if((rqct->i_enc_qp + rqcx->attr.i_offsetqp) < minqp)
        rqct->i_enc_qp = minqp;
    else
         rqct->i_enc_qp += rqcx->attr.i_offsetqp;
    rqcx->attr.i_offsetqp = 0;

    //MrqcRoiDraw(rqct, mjob);

    msb2->b_mbkadp = msb2->b_mbkadp && !rqcx->attr.b_dqmstat;
    regs->reg00_g_mbr_en = msb2->b_mbkadp;
    regs->reg26_s_mbr_pqp_dlimit = LF_QP_DIFF_LIMIT;
    regs->reg26_s_mbr_uqp_dlimit = UP_QP_DIFF_LIMIT;
    regs->reg00_g_qscale = rqct->i_enc_qp;
    regs->reg27_s_mbr_frame_qstep = qp2qs(rqct->i_enc_qp);
    regs->reg26_s_mbr_tmb_bits = tgt>1 ? tgt : 1;
    regs->reg2a_s_mbr_qp_min = lqp;
    regs->reg2a_s_mbr_qp_max = uqp;
    regs->reg6e_s_mbr_qstep_min = qp2qs(lqp);
    regs->reg6f_s_mbr_qstep_max = qp2qs(uqp);

    /* penalties of mbtypies */
    regs->regf2_g_i16pln_en = (0!=rqcx->attr.b_i16pln);
    regs->reg87_g_intra4_penalty = rqcx->attr.i_peni4x;
    regs->reg88_g_intra16_penalty = rqcx->attr.i_peni16;
    regs->reg88_g_inter_penalty = rqcx->attr.i_penint;
    regs->reg89_g_planar_penalty_luma = rqcx->attr.i_penYpl;
    regs->reg89_g_planar_penalty_cbcr = rqcx->attr.i_penCpl;

    return 0;
}

#define BSP_DUR (32)

static int _Msb2EncDone(rqct_ops* rqct, mhve_job* mjob)
{
    mfe6_rqc* rqcx = (mfe6_rqc*)rqct;
    msb2_rqc* msb2 = (msb2_rqc*)rqcx;
    mfe6_reg* regs = (mfe6_reg*)mjob;
    int bpp, cpx, bps, inc, dec, dur, avq = qp2qs(rqct->i_enc_qp), bcn = mjob->i_bits;
    int64 remainder;

    rqct->i_bitcnt = mjob->i_bits;
    rqct->i_enc_bs+= rqct->i_bitcnt/8;
    rqct->i_enc_nr++;

    if (msb2->b_mbkadp)
        avq = (avq + regs->enc_sumpqs)/msb2->i_blocks;

    regs->reg28 = regs->reg29 = 0;
    regs->regf5 = regs->regf6 = 0;
    regs->reg42 = regs->reg43 = 0;

    bpp = (int)CamOsMathDivS64((int64)rqct->i_bitcnt*BPP_FAC,msb2->i_pixels, &remainder);
    dur = rqcx->i_period;
    cpx = bpp * avq;

    if (IS_PPIC(rqct->i_pictyp))
    {
        accPush(&msb2->acc_bitcnt, bcn);
        accPush(&msb2->acc_rqprod, cpx);
        iirPush(&msb2->iir_rqprod[RQCT_PICTYP_P], accCalc(&msb2->acc_rqprod,8)/8);
        msb2->i_errpro = (msb2->i_ipbias-msb2->i_frmbit+bcn);
        msb2->i_imbits += msb2->i_errpro;
        msb2->i_imbits = _MIN(msb2->i_imbits,msb2->i_upperb+msb2->i_radius);
        msb2->i_imbits = _MAX(msb2->i_imbits,msb2->i_lowerb);
        msb2->i_rebits = _MAX(msb2->i_rebits,msb2->i_imbits);
    }
    else
    {
        msb2->i_intrabit = bcn;
        iirPush(&msb2->iir_rqprod[RQCT_PICTYP_I], cpx);
        if (msb2->i_imbits > msb2->i_upperb)
            msb2->i_rebits = msb2->i_imbits;
        msb2->i_errpro = (msb2->i_rebits-msb2->i_imbits);
        msb2->i_imbits += msb2->i_errpro;
        if (dur >= 2)
            msb2->i_ipbias = (bcn - msb2->i_frmbit) / (dur-1);
        if (dur == 0)
            accPush(&msb2->acc_bitcnt, bcn);
    }
    msb2->i_rebits += (bcn - msb2->i_frmbit);

    if (dur > 0)
        bps = accCalc(&msb2->acc_bitcnt, dur-1) + msb2->i_intrabit;
    else
        bps = accCalc(&msb2->acc_bitcnt, dur=BSP_DUR);
    bps = (int)CamOsMathDivS64((int64)bps*msb2->n_fmrate,msb2->d_fmrate*dur, &remainder);

    if (!rqcx->attr.b_logoff)
        CamOsSnprintf(rqct->print_line,RQCT_LINE_SIZE-1,"<%s#%04d@%c:%2d> bitcnt:%8d bps:%8d",
                      rqct->name,rqct->i_enc_nr%10000,rqct->i_pictyp==RQCT_PICTYP_I?'I':'P',rqct->i_enc_qp,rqct->i_bitcnt,bps);

#if 0
    //  CamOsPrintf("%s\n",rqct->print_line);
    CamOsPrintf("<%s#%04d@%c:%2d> bitcnt:%8d bps:%8d im:%8d df:%8d re:%8d\n",
               rqct->name,rqct->i_enc_nr%10000,rqct->i_pictyp==RQCT_PICTYP_I?'I':'P',rqct->i_enc_qp,rqct->i_bitcnt,bps,msb2->i_imbits,msb2->i_imbase,msb2->i_rebits);
#endif

    bcn = msb2->i_imbits-msb2->i_imbase;
    inc = msb2->i_frmbit/64;
    dec = -3*inc;
    bcn = _MIN(inc,_MAX(bcn,dec));
    msb2->i_imbase += bcn;
    msb2->i_imbase = _MIN(_MAX(msb2->i_imbase,msb2->i_lowerb),msb2->i_upperb);
    rqcx->i_refcnt++;
    return 0;
}

static int _Msb2CpyConf(rqct_ops* rqct_dst,rqct_ops* rqct_src)
{
    mfe6_rqc* rqcx_dst = (mfe6_rqc*)rqct_dst;
    msb2_rqc* msb2_dst = (msb2_rqc*)rqcx_dst;
    mfe6_rqc* rqcx_src = (mfe6_rqc*)rqct_src;
    msb2_rqc* msb2_src = (msb2_rqc*)rqcx_src;

    MEM_COPY(msb2_dst, msb2_src, sizeof(msb2_rqc));

    return 0;
}
