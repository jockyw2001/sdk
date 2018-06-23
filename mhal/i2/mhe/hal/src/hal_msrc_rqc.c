#include <linux/limits.h>   //derek check
#include <linux/module.h>
#include "hal_mhe_ops.h"
#include "hal_mhe_utl.h"
#include "hal_msrc_rqc.h"
#include "hal_mhe_msmath.h"
#include "hal_mhe_global.h"


/************************************************************************/
/* Version Declaration                                                  */
/************************************************************************/

#define MSRC_NAME               "MSRC"
#define MSRC_VER_MJR            1
#define MSRC_VER_MNR            1
#define MSRC_VER_EXT            01
#define _EXP(expr)              #expr
#define _STR(expr)              _EXP(expr)

/************************************************************************/
/* Macros                                                               */
/************************************************************************/

#define UP_QP_DIFF_LIMIT        3
#define LF_QP_DIFF_LIMIT        3

#define LOGOFF_DEFAULT          1
//#define LOWERQ_DEFAULT          12
//#define UPPERQ_DEFAULT          48
#define BOUNDQ_DEFAULT          -1

#define MIN_QP                      0
#define MAX_QP                      51

#define PMBR_LUT_CENTER_ENTRY 7

// Mode penalties when RDO cost comparison
//#define PENALTY_BITS   (16)
#define PENALTY_SHIFT  (2)

static inline int imin(int a, int b)
{
    return ((a) < (b)) ? (a) : (b);
}

static inline int imax(int a, int b)
{
    return ((a) > (b)) ? (a) : (b);
}

static inline int iabs(int a)
{
    return ((a) < 0) ? -(a) : (a);
}

static inline int iClip3(int low, int high, int x)
{
    x = imax(x, low);
    x = imin(x, high);
    return x;
}

static inline uint32 uimin(uint32 a, uint32 b)
{
    return ((a) < (b)) ? (a) : (b);
}

static inline uint32 uimax(uint32 a, uint32 b)
{
    return ((a) > (b)) ? (a) : (b);
}

static inline uint32 uiClip3(uint32 low, uint32 high, uint32 x)
{
    x = uimax(x, low);
    x = uimin(x, high);
    return x;
}

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/

/************************************************************************/
/* Constant                                                             */
/************************************************************************/

#define COMPLEX_I   (1<<24)
#define COMPLEX_P   (1<<20)

#define MAX_SUMQS   ((2UL<<31)-1)
#define ALIGN2CTU(l) (((l)+31)>>5)

// P-frame min, max QP
//static uint32 g_uiRcPicMinQp = MIN_QP;
//static uint32 g_uiRcPicMaxQp = MAX_QP;
// I-frame min, max QP
//static uint32 g_uiRcIPicMinQp = MIN_QP;
//static uint32 g_uiRcIPicMaxQp = MAX_QP;

// Picture delta Qp range
static uint32 g_uiRcPicClipRange = 4; // default

// min/max frame bit ratio relative to avg bitrate
static int g_iRcMaxIPicBitRatio = 60; // default
static int g_iRcMinIPicBitRatio = 1; // default
static int g_iRcMaxPicBitRatio = 10; // default
static int g_iRcMinPicBitRatio = 0; // default

//static int g_iHWTextCplxAccum;

static int g_iRcStatsTime = 1;

/************************************************************************/
/* Local structures                                                     */
/************************************************************************/

#define PICTYPES 2

typedef struct MsrcRqc_t
{
    mhe_rqc    rqcx;
    int     i_method;           /* Rate Control Method */
    int     i_btrate;           /* Bitrate */
    int     i_btratio;          /* Bitrate Ratio (50~100) */
    int     i_levelq;           /* Frame Level QP */
    int     i_deltaq;           /* QP Delta Value */
    int     n_fmrate;           /* Frame Rate Numerator */
    int     d_fmrate;           /* Frame Rate Denominator */
    int     i_fmrate;           /* Frame Rate */
    int     i_pixels;           /* Picture Pixel Count */
    int     i_blocks;           /* Picture CTU Count */
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

    ms_rc_top msrc_top;         /* MSRC top Attribute */

} msrc_rqc;


// Generate from C-Model (search "g_DefaultLambdaByQp")
// Q16.16
//I frame qp to lambda table
static const uint32 g_DefaultLambdaByQp[52] =
{
    2334,   2941,   3706,   4669,   5883,
    7412,   9338,   11766,  14824,  18677,
    23532,  29649,  37355,  47065,  59298,
    74711,  94130,  118596, 149422, 188260,
    237192, 298844, 376520, 474385, 597688,
    753040, 948771, 1195376,    1506080,    1897542,
    2390753,    3012160,    3795084,    4781506,    6024320,
    7590168,    9563013,    12048641,   15180337,   19126026,
    24097283,   30360674,   38252052,   48194566,   60721348,
    76504104,   96389132,   121442696,  153008209,  192778264,
    242885393,  306016419,
};

//P frame qp to lambda table
static const uint32 g_PFrameLambdaByQp[52] =
{
    1893,       2386,         3006,       3787,       4772,
    6013,       7575,         9545,       12026,     15151,
    19090,     24052,       30303,     38180,     48104,
    60607,     76360,      96208,     121215,   152721,
    192417,   242430,     305443,   384834,   484861,
    610887,   769669,     969723,   1221774,       1539339,
    1939446, 2443549,   3078678, 3878892,       4887098,
    6157357, 7757784,   9774196, 12314715, 15515569,
    19548392, 24629431, 31031138, 39096784, 49258862,
    62062277, 78193569, 98517724, 124124554, 156387139,
    197035448, 248249109,
};


/************************************************************************/
/* Local prototypes                                                     */
/************************************************************************/

char* MheMsrcDescribe(void);

static int  _MsrcSeqSync(rqct_ops*);
static int  _MsrcSeqDone(rqct_ops*);
static int  _MsrcSeqConf(rqct_ops*);
static int  _MsrcEncConf(rqct_ops*, mhve_job*);
static int  _MsrcEncDone(rqct_ops*, mhve_job*);
static int  _MsrcCpyConf(rqct_ops*, rqct_ops*);
static void _MsrcOpsFree(rqct_ops* rqct)
{
    MEM_FREE(rqct);
}

/* Fixed Point Rate Control */
static uint32 _LambdaToBpp_SCALED(ms_rc_top* ptMsRc, uint32 uiLambda_SCALED, int picType, int isRTcModel);
static void _calcAvgLambdaFromHist(ms_rc_top* ptMsRc);
static uint32 _EstPicLambdaByBits(ms_rc_top* ptMsRc, int picType, int picBit);
static uint32 _EstimatePicQpByLambda(ms_rc_top* ptMsRc, int picType, uint32 lambda_SCALED);
static int _EstBitsForIntraFrame(ms_rc_top* ptMsRc);
static void _Est1stFrameParamByQp(ms_rc_top* ptMsRc, int picType, uint32 uiQp, int isRTcModel, int* p_retBit, uint32 *p_retLambda_SCALED);
static void _msrc_gop_start(ms_rc_top* ptMsRcm, msrc_rqc* pMsRc);
static void _msrc_pic_start(ms_rc_top* ptMsRc, int picType);
static void _msrc_update_stats(ms_rc_top* ptMsRc, int picEncBit,int iHWTextCplxAccum);
static void _msrc_update_model(ms_rc_top* ptMsRc, int picActualBit, int picType);

///////////////////////////////////////////////////////////////////////////////
// Internal functions
///////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
//  Function    : ROICtbForceMode
//  Description :
//------------------------------------------------------------------------------
static unsigned char _ROICtbForceMode(char ctb_zmv_en,char ctb_force_intra_en)
{
    /*
    bool bCtbForceMode = !bCtbForceIntra; // 0:force intra, 1:Zmv; Force Intra has higher priority
    bool bCtbForceEn = bCtbForceIntra | bCtbZmv;
    unsigned char first_byte = iEntryOffset&0xFF;
    unsigned char second_byte = (((char)bCtbZmv)<<2) | (((char)bCtbForceMode)<<1) | ((char)bCtbForceEn);
    */

    char ctb_force_en = ctb_zmv_en | ctb_force_intra_en;
    unsigned char ctb_force_mode = (((char)ctb_zmv_en) << 2) | (((char)!ctb_force_intra_en) << 1) | ((char)ctb_force_en);

    return ctb_force_mode;
}

//------------------------------------------------------------------------------
//  Function    : ROISetBgMapValue
//  Description :
//------------------------------------------------------------------------------
static void _ROISetBgMapValue(char value,unsigned char mode, char *map,int ihorzctbnum, int ivertctbnum)
{
    char *pRoiMap;
    int x, y;

    pRoiMap = &map[0];

    for(y = 0; y < ivertctbnum; y++)
    {
        for(x = 0; x < ihorzctbnum; x++)
        {
            *pRoiMap = value & 0xFF;
            pRoiMap++;
            *pRoiMap = mode & 0xFF;
            pRoiMap++;
        }
    }
}

//------------------------------------------------------------------------------
//  Function    : ROISetMapValue
//  Description :
//------------------------------------------------------------------------------
static void _ROISetMapValue(char value,unsigned char mode, char *map, struct roirec* rec, int ihorzctbnum, int ivertctbnum, int ctbnum)
{
    char *pRoiMap, *pRoiMapHor;
    int startX, startY, width, height;
    int stride = ihorzctbnum * 2; //1 CTB need 2 bytes
    int x, y;

    startX = rec->i_posx;
    startY = rec->i_posy;
    width = rec->i_recw;
    height = rec->i_rech;

    //CamOsPrintf("[%s %d] (%d, %d, %d, %d) (%d %d %d) \n", __FUNCTION__, __LINE__, startX, startY, width, height, ihorzctbnum, ivertctbnum, ctbnum);
    //mode = 7;
    //value = 0;

    pRoiMap = &map[startY * stride + startX * 2];

    for(y = 0; y < height; y++)
    {
        pRoiMapHor = pRoiMap;
        for(x = 0; x < width; x++)
        {
            *pRoiMapHor = value & 0xFF;
            pRoiMapHor++;
            *pRoiMapHor = mode & 0xFF;
            pRoiMapHor++;
        }
        pRoiMap += stride;
    }
}

//------------------------------------------------------------------------------
//  Function    : ROISetMapValue
//  Description :
//------------------------------------------------------------------------------
static void _ROISetQpMap(char* map, char* qpmap ,char mode, int ctbnum)
{
    int i;
    char *pRoiMap, *pQpMap;
    char qpentry;
    signed char iMapValue;

    pRoiMap = map;
    pQpMap = qpmap;

    for(i = 0; i < ctbnum; i++)
    {
        qpentry = *pQpMap;
        if(qpentry >=0 && qpentry < 14)
        {
            if(qpentry < PMBR_LUT_CENTER_ENTRY)
                iMapValue = qpentry - PMBR_LUT_CENTER_ENTRY;
            else
                iMapValue = qpentry - PMBR_LUT_CENTER_ENTRY + 1;
        }
        else    //0xFF
        {
            iMapValue = 0;
        }

        *pRoiMap = iMapValue & 0xFF;;
        pRoiMap++;
        *pRoiMap = mode & 0xFF;
        pRoiMap++;
        pQpMap++;
    }
}

//------------------------------------------------------------------------------
//  Function    : _draw
//  Description :
//------------------------------------------------------------------------------
static int _draw(rqct_att* attr)
{
    int roi_idx = 0;
    int roi_enable = 0;
    char* dqmmap = (char*)attr->p_dqmkptr;
    int ctbw = attr->i_dqmw;
    int ctbh = attr->i_dqmh;
    int ctbn = ctbw * ctbh;
    struct roirec* roi = attr->m_roirec;
    signed char* roidqp = attr->i_roiqp;
    struct roirec* rec;
    signed char qpoffset, iMapValue;
    int bCtbForceIntra;
    int bCtbZmv;
    unsigned char ctbforcemode;

    if(attr->u_qpmenb)
    {
        _ROISetQpMap(dqmmap,(char *) attr->p_qpmkptr, 0, ctbn);

        return 1;
    }

    //Set QOI MAP background default mapping
    bCtbZmv = 1;
    bCtbForceIntra = 0;
    ctbforcemode = _ROICtbForceMode(bCtbZmv, bCtbForceIntra);

    iMapValue = 0;
    _ROISetBgMapValue(iMapValue, ctbforcemode, dqmmap, ctbw, ctbh);

    for(roi_idx = 0; roi_idx < RQCT_ROI_NR; roi_idx++)
    {
        rec = &roi[roi_idx];
        qpoffset = roidqp[roi_idx];

        if(qpoffset != 0)
        {
            iMapValue = (roi_idx == PMBR_LUT_CENTER_ENTRY)? 1: roi_idx - PMBR_LUT_CENTER_ENTRY;

            bCtbZmv = bCtbForceIntra = 0;
            ctbforcemode = _ROICtbForceMode(bCtbZmv, bCtbForceIntra);

            _ROISetMapValue(iMapValue, ctbforcemode, dqmmap, rec, ctbw, ctbh, ctbn);

            roi_enable = 1;
        }
    }

#if 0
    if(roi_enable)
    {
        CamOsPrintf("ROI MAP\n");
        CamOsHexdump(attr->p_dqmkptr, attr->i_dqmsize);
    }
#endif

    return roi_enable;
}

//------------------------------------------------------------------------------
//  Function    : MheMrqcRoiDraw
//  Description :
//------------------------------------------------------------------------------
static int FrameRateNeedToSkip(rqct_ops* rqct)
{

    mhe_rqc* rqcx = (mhe_rqc*)rqct;
    int NeedToDropFrm = 0;
    int frame_count = rqcx->rqct.i_enc_nr;
    int srcframerate = rqcx->attr.i_roibgsrcfmrate << 1;
    int dstframerate = rqcx->attr.i_roibgdstfmrate << 1;
    int data1, data0;

    if(frame_count <= 0)
    {
        return 0;
    }

    if(rqcx->attr.i_roibgsrcfmrate == rqcx->attr.i_roibgdstfmrate)
    {
        return 0;
    }

    data0 = (((frame_count-1)%srcframerate)*dstframerate)/srcframerate;
    data1 = ((frame_count%srcframerate)*dstframerate)/srcframerate;

    NeedToDropFrm = (data1 != data0)? 0:1;

    //CamOsPrintf("frame_count(%d), (%d), (%d), (%d)\n", frame_count, frame_count%srcframerate, data1, NeedToDropFrm);

    return NeedToDropFrm;
}

int MheMrqcRoiDraw(rqct_ops* rqct, mhve_job* mjob)
{
    mhe_rqc* rqcx = (mhe_rqc*)rqct;
    mhe_reg* regs = (mhe_reg*)mjob;

    if(0 > rqcx->attr.b_dqmstat)
    {
        /* Edit qmap and enable it */

        //Initial ROI MAP
        MEM_SETV((char*)rqcx->attr.p_dqmkptr, 0, rqcx->attr.i_dqmsize);

        if(_draw(&rqcx->attr))
        {
            regs->hev_bank1.reg_mbr_lut_roi_on = 1;     //g_bROIQpEn;
            regs->hev_bank1.reg_txip_ctb_force_on = 0;  //g_bForcedIntraEn | g_bROIZmvEn;

            regs->hev_bank1.reg_mbr_gn_read_map_st_addr_low = rqcx->attr.u_dqmphys >> 4;
            regs->hev_bank1.reg_mbr_gn_read_map_st_addr_high = rqcx->attr.u_dqmphys >> 16;

            rqcx->attr.b_dqmstat = 1;
        }
        else
        {
            regs->hev_bank1.reg_mbr_lut_roi_on = 0;
            regs->hev_bank1.reg_txip_ctb_force_on = 0;

            rqcx->attr.b_dqmstat = 0;
        }
    }

    //ROI background FPS control
    if(rqcx->attr.b_dqmstat == 1)
    {
        regs->hev_bank1.reg_txip_ctb_force_on = FrameRateNeedToSkip(rqct);
    }

    return (rqcx->attr.b_dqmstat);
}

/************************************************************************/
/* Functions                                                            */
/************************************************************************/

//------------------------------------------------------------------------------
//  Function    : MheMsrcAllocate
//  Description :
//------------------------------------------------------------------------------
void* MheMsrcAllocate(void)
{

    rqct_ops* rqct = NULL;
    mhe_rqc* rqcx;
    msrc_rqc* msrc;

    if(!(rqct = MEM_ALLC(sizeof(msrc_rqc))))
        return NULL;

    MEM_COPY(rqct->name, MSRC_NAME, 5);

    /* Link member function */
    rqct->release  = _MsrcOpsFree;
    rqct->seq_sync = _MsrcSeqSync;
    rqct->seq_done = _MsrcSeqDone;
    rqct->set_rqcf = MheMrqcSetRqcf;
    rqct->get_rqcf = MheMrqcGetRqcf;
    rqct->seq_conf = _MsrcSeqConf;
    rqct->enc_buff = MheMrqcEncBuff;
    rqct->enc_conf = _MsrcEncConf;
    rqct->enc_done = _MsrcEncDone;
    rqct->cpy_conf = _MsrcCpyConf;
    rqct->i_enc_nr = 0;
    rqct->i_enc_bs = 0;

    /* Initialize Basic RC Attribute */
    rqcx = (mhe_rqc*)rqct;
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

    //ROI background frame rate
    rqcx->attr.i_roibgsrcfmrate = 0;
    rqcx->attr.i_roibgdstfmrate = 0;

    rqcx->i_config = 0;
    rqcx->i_pcount = 0;
    rqcx->i_period = 0;
    rqcx->b_passiveI = 0;
    rqcx->b_seqhead = 1;

    msrc = (msrc_rqc*)rqcx;
    msrc->i_levelq = 36;

    CamOsPrintf("%s\n", MheMsrcDescribe());

    return rqct;

}

//------------------------------------------------------------------------------
//  Function    : MheMsrcDescribe
//  Description :
//------------------------------------------------------------------------------
char* MheMsrcDescribe(void)
{
    static char line[64];
    sprintf(line, "%s@v%d.%d-%02d:r&d analysis.", MSRC_NAME, MSRC_VER_MJR, MSRC_VER_MNR, MSRC_VER_EXT);
    return line;
}

//------------------------------------------------------------------------------
//  Function    : _MsrcSeqDone
//  Description :
//------------------------------------------------------------------------------
static int _MsrcSeqDone(rqct_ops* rqct)
{
    return 0;
}
//------------------------------------------------------------------------------
//  Function    : _MsrcSeqSync
//  Description :
//------------------------------------------------------------------------------
static int _MsrcSeqSync(rqct_ops* rqct)
{

    mhe_rqc* rqcx = (mhe_rqc*)rqct;

    rqcx->i_pcount = 0;
    rqcx->b_seqhead = 1;

    return 0;
}


static int _MsrcSeqConf(rqct_ops* rqct)
{
    mhe_rqc* rqcx = (mhe_rqc*)rqct;
    msrc_rqc* msrc = (msrc_rqc*)rqcx;

    uint64 uiremainder;
    uint32 uiSeqTargetBpp_SCALED;
    ms_rc_pic *ptRcPic;

    msrc->i_method = (int)rqcx->attr.i_method;

    memset((void *)&msrc->msrc_top, 0, sizeof(ms_rc_top));

    msrc->i_deltaq = (int)rqcx->attr.i_deltaq;

    msrc->msrc_top.uiRcPPicMaxQp = MAX_QP;
    msrc->msrc_top.uiRcPPicMinQp = MIN_QP;
    msrc->msrc_top.uiRcIPicMaxQp = MAX_QP;
    msrc->msrc_top.uiRcIPicMinQp = MIN_QP;

    if(msrc->i_method == RQCT_MODE_VBR)
    {
        msrc->i_btratio = 80;   //(int) rqcx->attr.i_btratio;
        msrc->i_iupperq = (int) rqcx->attr.i_iupperq;
        msrc->i_ilowerq = (int) rqcx->attr.i_ilowerq;
        msrc->i_pupperq = (int) rqcx->attr.i_pupperq;
        msrc->i_plowerq = (int) rqcx->attr.i_plowerq;

        msrc->msrc_top.uiRcPPicMaxQp = msrc->i_pupperq;
        msrc->msrc_top.uiRcPPicMinQp = msrc->i_plowerq;
        msrc->msrc_top.uiRcIPicMaxQp = msrc->i_iupperq;
        msrc->msrc_top.uiRcIPicMinQp = msrc->i_ilowerq;
    }

    msrc->msrc_top.uiTargetBitrate = (uint32)rqcx->attr.i_btrate;
    msrc->n_fmrate = (int)rqcx->attr.n_fmrate;
    msrc->d_fmrate = (int)rqcx->attr.d_fmrate;
    CDBZ(msrc->n_fmrate,"msrc->n_fmrate");
    CDBZ(msrc->d_fmrate,"msrc->d_fmrate");

    msrc->msrc_top.uiFps = (msrc->n_fmrate + (msrc->d_fmrate >> 1)) / msrc->d_fmrate; // rounding to integer
    rqcx->i_pcount = rqcx->i_period = (int)rqcx->attr.i_period;
    rqcx->b_seqhead = 1;
    msrc->msrc_top.uiGopSize = (uint32)rqcx->i_period;
    msrc->msrc_top.uiPicPixelNum = (int)rqcx->attr.i_pict_w * rqcx->attr.i_pict_h;

    if(msrc->i_method != RQCT_MODE_CQP)
    {
        msrc->i_frmbit = (int)CamOsMathDivU64((uint64)msrc->msrc_top.uiTargetBitrate * msrc->d_fmrate, msrc->n_fmrate, &uiremainder);
        msrc->msrc_top.iPicAvgBit = msrc->i_frmbit;
    }

    msrc->msrc_top.uiPicCtuNum = ALIGN2CTU(rqcx->attr.i_pict_w) * ALIGN2CTU(rqcx->attr.i_pict_h);
    msrc->msrc_top.uiIntraMdlCplx = msrc->msrc_top.uiPicPixelNum * 10;

    if(rqcx->attr.i_leadqp >= QP_MIN && rqcx->attr.i_leadqp <= QP_MAX)
        msrc->i_levelq = (int)rqcx->attr.i_leadqp;

    msrc->msrc_top.uiInitialQp = (uint32)msrc->i_levelq;

    ptRcPic = &msrc->msrc_top.tRcPic;
    ptRcPic->uiPicLambda_SCALED = 0;
    ptRcPic->uiPicQp = 0;

    msrc->msrc_top.iLambdaMinClipMul = (int)MhefPow64(2 << PREC_SCALE_BITS, -(int)(g_uiRcPicClipRange << PREC_SCALE_BITS) / 3);
    msrc->msrc_top.iLambdaMaxClipMul = (int)MhefPow64(2 << PREC_SCALE_BITS, (g_uiRcPicClipRange << PREC_SCALE_BITS) / 3);

    msrc->msrc_top.uiMaxPicBit = msrc->msrc_top.iPicAvgBit * g_iRcMaxPicBitRatio;
    msrc->msrc_top.uiMinPicBit = msrc->msrc_top.iPicAvgBit * g_iRcMinPicBitRatio;
    msrc->msrc_top.uiMaxIPicBit = msrc->msrc_top.iPicAvgBit * g_iRcMaxIPicBitRatio;
    msrc->msrc_top.uiMinIPicBit = msrc->msrc_top.iPicAvgBit * g_iRcMinIPicBitRatio;

    // RC model parameter initial
    msrc->msrc_top.atModelPara[I_SLICE].Alpha_SCALED = ALPHA_SCALED;
    msrc->msrc_top.atModelPara[I_SLICE].Beta_SCALED = BETA2_SCALED;
    msrc->msrc_top.atModelPara[P_SLICE].Alpha_SCALED = ALPHA_P_SCALED;
    msrc->msrc_top.atModelPara[P_SLICE].Beta_SCALED = BETA_P_SCALED;

    CDBZ(msrc->msrc_top.uiPicPixelNum,"msrc->msrc_top.uiPicPixelNum");
    uiSeqTargetBpp_SCALED = (uint32)CamOsMathDivU64((uint64)msrc->msrc_top.iPicAvgBit << PREC_SCALE_BITS, msrc->msrc_top.uiPicPixelNum, &uiremainder);

    // Pre-multiplied magic numbers.
    // NEED_CHANGE_BY_PREC_SCALE_BITS
    if(uiSeqTargetBpp_SCALED < 1966)
    {
        msrc->msrc_top.uiStepAlpha_SCALED = 655;
        msrc->msrc_top.uiStepBeta_SCALED = 328;
    }
    else if(uiSeqTargetBpp_SCALED < 5242)
    {
        msrc->msrc_top.uiStepAlpha_SCALED = 3277;
        msrc->msrc_top.uiStepBeta_SCALED  = 1638;
    }
    else if(uiSeqTargetBpp_SCALED < 13107)
    {
        msrc->msrc_top.uiStepAlpha_SCALED = 6554;
        msrc->msrc_top.uiStepBeta_SCALED  = 3277;
    }
    else if(uiSeqTargetBpp_SCALED < 32768)
    {
        msrc->msrc_top.uiStepAlpha_SCALED = 13107;
        msrc->msrc_top.uiStepBeta_SCALED  = 6554;
    }
    else
    {
        msrc->msrc_top.uiStepAlpha_SCALED = 26214;
        msrc->msrc_top.uiStepBeta_SCALED  = 13107;
    }

    msrc->msrc_top.tMbrLut.auiQps = rqct->auiQps;
    msrc->msrc_top.tMbrLut.auiLambdas_SCALED = rqct->auiLambdas_SCALED;
    msrc->msrc_top.tMbrLut.auiBits = rqct->auiBits;
    msrc->msrc_top.tMbrLut.aiIdcHist = rqct->aiIdcHist;

    MHE_MSG(MHE_MSG_DEBUG, "uiInitialQp = %d\n", msrc->msrc_top.uiInitialQp);
    MHE_MSG(MHE_MSG_DEBUG, "uiTargetBitrate = %d\n", msrc->msrc_top.uiTargetBitrate);
    MHE_MSG(MHE_MSG_DEBUG, "uiFps = %d\n", msrc->msrc_top.uiFps);
    MHE_MSG(MHE_MSG_DEBUG, "uiGopSize = %d\n", msrc->msrc_top.uiGopSize);
    MHE_MSG(MHE_MSG_DEBUG, "uiPicPixelNum = %d\n", msrc->msrc_top.uiPicPixelNum);
    MHE_MSG(MHE_MSG_DEBUG, "iPicAvgBit = %d\n", msrc->msrc_top.iPicAvgBit);
    MHE_MSG(MHE_MSG_DEBUG, "uiPicCtuNum = %d\n", msrc->msrc_top.uiPicCtuNum);

    return 0;
}

static int _MsrcEncConf(rqct_ops* rqct, mhve_job* mjob)
{
    mhe_rqc* rqcx = (mhe_rqc*)rqct;
    mhe_reg* regs = (mhe_reg*)mjob;
    msrc_rqc* msrc = (msrc_rqc*)rqcx;
    ms_rc_top* ptMsRc = &msrc->msrc_top;
    int maxqp, minqp;
    int pictype;

    MheMrqcRoiDraw(rqct, mjob);

    switch(msrc->i_method)
    {
        case RQCT_MODE_CQP:
            rqct->i_enc_qp = msrc->i_levelq;
            if(IS_IPIC(rqct->i_pictyp))
            {
                rqct->i_enc_qp -= msrc->i_deltaq;
                rqct->i_enc_lamda = g_DefaultLambdaByQp[rqct->i_enc_qp];
            }
            else //IS_PPIC and IS_LTRPPIC
            {
                rqct->i_enc_lamda = g_PFrameLambdaByQp[rqct->i_enc_qp];
            }

            //CamOsPrintf("[%s %d] type(%d): rqct->i_enc_qp(%d), msrc->i_deltaq(%d)\n",__FUNCTION__,__LINE__,rqct->i_pictyp,rqct->i_enc_qp,msrc->i_deltaq);

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
            regs->hev_bank0.reg_hev_slice_qp = rqct->i_enc_qp;

            break;
        case RQCT_MODE_CBR:
        case RQCT_MODE_VBR:
            if(IS_IPIC(rqct->i_pictyp) || (ptMsRc->tRcGop.iGopFrameLeft <= 0))
            {
                _msrc_gop_start(ptMsRc, msrc);
            }
            pictype = IS_LTRPPIC(rqct->i_pictyp)? RQCT_PICTYP_P : rqct->i_pictyp;
            _msrc_pic_start(ptMsRc, pictype);

            rqct->i_enc_qp = ptMsRc->tRcPic.uiPicQp;
            rqct->i_enc_lamda = ptMsRc->tRcPic.uiPicLambda_SCALED;

            maxqp = IS_IPIC(rqct->i_pictyp) ? (rqcx->attr.i_iupperq) : (rqcx->attr.i_pupperq);
            minqp = IS_IPIC(rqct->i_pictyp) ? (rqcx->attr.i_ilowerq) : (rqcx->attr.i_plowerq);

            if((rqct->i_enc_qp + rqcx->attr.i_offsetqp) >  maxqp)
                rqct->i_enc_qp = maxqp;
            else if((rqct->i_enc_qp + rqcx->attr.i_offsetqp) < minqp)
                rqct->i_enc_qp = minqp;
            else
                 rqct->i_enc_qp += rqcx->attr.i_offsetqp;
            rqcx->attr.i_offsetqp = 0;

            //CamOsPrintf("[%s %d] pictype(%d), iGopFrameLeft(%d), encQP(%d)\n",__FUNCTION__,__LINE__, rqct->i_pictyp, ptMsRc->tRcGop.iGopFrameLeft, rqct->i_enc_qp);

            // Qp
            regs->hev_bank1.reg_mbr_const_qp_en = 1;
            regs->hev_bank0.reg_hev_slice_qp = rqct->i_enc_qp;

            break;
        default:
            break;
    }

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

    return 0;
}


static int _MsrcEncDone(rqct_ops* rqct, mhve_job* mjob)
{
    mhe_rqc* rqcx = (mhe_rqc*)rqct;
    mhe_reg* regs = (mhe_reg*)mjob;
    msrc_rqc* msrc = (msrc_rqc*)rqcx;
    ms_rc_top* ptMsRc = &msrc->msrc_top;
    int pictype;

    regs->hev_bank1.reg6d = regs->hev_bank1.reg6e = 0;
    regs->mhe_bank0.reg74 = regs->mhe_bank0.reg75 = 0;

    //g_iHWTextCplxAccum = regs->pmbr_tc_accum;

    rqct->i_bitcnt  = mjob->i_bits;
    rqct->i_enc_bs += mjob->i_bits / 8;
    rqct->i_enc_nr++;
    rqcx->i_refcnt++;

    MHE_MSG(MHE_MSG_DEBUG, "mjob->i_bits = %d\n", mjob->i_bits);

    ptMsRc->tMbrLut.auiQps = rqct->auiQps;
    ptMsRc->tMbrLut.auiBits = rqct->auiBits;
    ptMsRc->tMbrLut.auiLambdas_SCALED = rqct->auiLambdas_SCALED;
    ptMsRc->tMbrLut.aiIdcHist = rqct->aiIdcHist;

    switch(msrc->i_method)
    {
        case RQCT_MODE_CQP:
            break;
        case RQCT_MODE_CBR:
        case RQCT_MODE_VBR:
            _msrc_update_stats(ptMsRc, rqct->i_bitcnt, regs->pmbr_tc_accum);
            pictype = IS_LTRPPIC(rqct->i_pictyp)? RQCT_PICTYP_P : rqct->i_pictyp;
            _msrc_update_model(ptMsRc, rqct->i_bitcnt, pictype);
            break;
    }

    return 0;
}

static int _MsrcCpyConf(rqct_ops* rqct_dst,rqct_ops* rqct_src)
{
    mhe_rqc* rqcx_dst = (mhe_rqc*)rqct_dst;
    msrc_rqc* msrc_dst = (msrc_rqc*)rqcx_dst;
    mhe_rqc* rqcx_src = (mhe_rqc*)rqct_src;
    msrc_rqc* msrc_src = (msrc_rqc*)rqcx_src;

    MEM_COPY(msrc_dst, msrc_src, sizeof(msrc_rqc));

    return 0;
}
///////////////////////////////////////////////////////////////////////////////
// Fixed point rate control functions
///////////////////////////////////////////////////////////////////////////////

// CModel: LambdaToBpp
// Lambda = Alpha * Bpp^Beta
static uint32 _LambdaToBpp_SCALED(ms_rc_top* ptMsRc, uint32 uiLambda_SCALED, int picType, int isRTcModel)
{
    rc_model_para *ptRcMdlPara = &ptMsRc->atModelPara[picType];
    int alpha_SCALED = ptRcMdlPara->Alpha_SCALED;
    int beta_SCALED = ptRcMdlPara->Beta_SCALED;
    uint32 uiEstBpp_SCALED;
    uint64 uiremainder;
    int64 remainder;
    uint32 picPelNum;
    uint32 IframeCplx;
    uint32 CplxPerPel_SCALE;
    uint64 uiNormTC_SCALED;
    uint32 lambda_div_alpha;
    uint32 scal_8bit;
    int neg_inv_beta_SCALED;
    uint64 pow_lambda_x256_div_alpha;
    uint32 lambda_div_alphaSCALED;
    int inv_betaSCALED;


    isRTcModel &= (picType == I_SLICE);

    if(isRTcModel)
    {
        picPelNum = ptMsRc->uiPicPixelNum;
        IframeCplx = ptMsRc->uiIntraMdlCplx;

        CDBZ(picPelNum,"picPelNum");
        CplxPerPel_SCALE = CamOsMathDivU64(((uint64) IframeCplx << PREC_SCALE_BITS), picPelNum, &uiremainder);
        assert(CplxPerPel_SCALE,"CplxPerPel_SCALE");
        uiNormTC_SCALED = MhefPow64(CplxPerPel_SCALE, BETA1_SCALED);

        CDBZ(alpha_SCALED,"alpha_SCALED");
        lambda_div_alpha = (uint32) CamOsMathDivU64(((uint64) uiLambda_SCALED << PREC_SCALE_BITS), alpha_SCALED, &uiremainder);
        scal_8bit = ((uint32) 256 << PREC_SCALE_BITS);

        if(lambda_div_alpha == 0)
        {
            MHE_MSG(MHE_MSG_WARNING, "lambda_div_alpha(%u), uiLambda_SCALED(%u, %llu), alpha_SCALED(%d)\n", lambda_div_alpha, uiLambda_SCALED, (uint64)uiLambda_SCALED<<PREC_SCALE_BITS,alpha_SCALED);
        }

        CDBZ(beta_SCALED,"beta_SCALED");
        neg_inv_beta_SCALED = (int) - (CamOsMathDivS64(((int64) 1 << (PREC_SCALE_BITS << 1)), beta_SCALED, &remainder));

        // org
        // uint64 lambda_div_alpha_x256 = ((uint64)uiLambda_SCALED<<(PREC_SCALE_BITS+8))/alpha_SCALED;

        assert(lambda_div_alpha,"lambda_div_alpha");
        pow_lambda_x256_div_alpha = (MhefPow64(lambda_div_alpha, neg_inv_beta_SCALED) * MhefPow64(scal_8bit, neg_inv_beta_SCALED)) >> PREC_SCALE_BITS;

        uiEstBpp_SCALED = (uint32)((uiNormTC_SCALED * pow_lambda_x256_div_alpha) >> PREC_SCALE_BITS);
    }
    else
    {
        CDBZ(alpha_SCALED,"alpha_SCALED");
        lambda_div_alphaSCALED = CamOsMathDivU64(((uint64) uiLambda_SCALED << PREC_SCALE_BITS), alpha_SCALED, &uiremainder);
        CDBZ(beta_SCALED,"beta_SCALED");
        inv_betaSCALED = (int) CamOsMathDivS64(((int64) 1 << (PREC_SCALE_BITS << 1)), beta_SCALED, &remainder);

        uiEstBpp_SCALED = (uint32)MhefPow64(lambda_div_alphaSCALED, inv_betaSCALED);
    }

    return uiEstBpp_SCALED;
}

// CModel: calcAvgLambdaFromHist
// Update average QP and lambda by LUT histogram
static void _calcAvgLambdaFromHist(ms_rc_top* ptMsRc)
{
    ms_rc_pic *ptRcPic = &ptMsRc->tRcPic;
    mbr_lut *ptMbrLut = &ptMsRc->tMbrLut;
    int64 iTotalLambda_SCALED = 0;  // sum of log-of-lambda_SCALED (for geometric mean)
    uint32 uiTotalQP = 0;
    uint32 uiTotalCnt = 0, idc;
    uint32 uiActAvgLambda_SCALED, uiActAvgQP;
    int32 iAvgTotalLamdba;
    int64 remainder;
    uint32 uiEntryCnt;

    for(idc = 0; idc < MBR_LUT_SIZE; idc++)
    {
        uiEntryCnt = ptMbrLut->aiIdcHist[idc];              //m_nLUTEntryHist
        uiTotalCnt += uiEntryCnt;

        uiTotalQP += uiEntryCnt * ptMbrLut->auiQps[idc];    //m_nLUTQp

        //CamOsPrintf("IdcHist[%d] auiQps[%d] auiLambdas_SCALED[%d] = %d, %d, %d\n", idc, idc, idc, ptMbrLut->aiIdcHist[idc],  ptMbrLut->auiQps[idc], ptMbrLut->auiLambdas_SCALED[idc]);
        iTotalLambda_SCALED += ((int64) uiEntryCnt * MhefLog(ptMbrLut->auiLambdas_SCALED[idc]));   //m_auiLUTLambdas_SCALED
    }

    MHE_MSG(MHE_MSG_DEBUG, "uiTotalQP = %d \n", uiTotalQP);
    MHE_MSG(MHE_MSG_DEBUG, "uiTotalCnt = %d \n", uiTotalCnt);
    if(!uiTotalCnt)
        MHE_MSG(MHE_MSG_WARNING, "uiTotalCnt is 0 !!!!!\n");

    // Update
    CDBZ(uiTotalCnt,"uiTotalCnt");
    iAvgTotalLamdba = (int32) CamOsMathDivS64(iTotalLambda_SCALED, uiTotalCnt, &remainder);
    uiActAvgLambda_SCALED = (uint32) MhefExp(iAvgTotalLamdba);
    uiActAvgQP = (uiTotalQP + (uiTotalCnt >> 1)) / uiTotalCnt;

    MHE_MSG(MHE_MSG_DEBUG,"uiActAvgQP(%d),uiPicQp(%d),uiTotalQP(%d),uiTotalCnt(%d)\n", uiActAvgQP, ptRcPic->uiPicQp, uiTotalQP, uiTotalCnt);
    //CamOsPrintf("uiActAvgLambda_SCALED = %d\n", uiActAvgLambda_SCALED);

    ptRcPic->auiLevelLambda_SCALED[ptRcPic->picType] = uiActAvgLambda_SCALED;
    ptRcPic->auiLevelQp[ptRcPic->picType] = uiActAvgQP;
    ptRcPic->uiPicLambda_SCALED = uiActAvgLambda_SCALED;
    ptRcPic->uiPicQp = uiActAvgQP;
}

// CModel: estimatePicLambda
//    Return lambda_SCALED
static uint32 _EstPicLambdaByBits(ms_rc_top* ptMsRc, int picType, int picBit)
{

    ms_rc_pic *ptRcPic = &ptMsRc->tRcPic;
    uint32 picPelNum = ptMsRc->uiPicPixelNum;
    int alpha_SCALED = ptMsRc->atModelPara[picType].Alpha_SCALED;
    int beta_SCALED = ptMsRc->atModelPara[picType].Beta_SCALED;
    uint32 estLambda_SCALED, lastPicLambda_SCALED;
    uint64 uiremainder;

    uint64 picBit_SCALED = (uint64) picBit << PREC_SCALE_BITS;
    uint32 bpp_SCALED;
    uint32 IframeCplx;
    int32 tcPerPel;
    uint32 pow_tcPerPel_beta1_SCALED;
    uint32 uiInvBpp_SCALED;
    uint64 pow_InvBpp_beta;
    uint64 pow_tcPerPel_beta;
    uint64 pow_TcDivBit_beta;
    uint64 bpp_pow_beta;
    uint32 lastLevelLambda_SCALED;
    uint64 ui64EstLambda_SCALED;
    uint32 uiMinValue;
    uint32 uiMaxValue;
    uint32 uiMaxPicQp, uiMinPicQp;
    uint32 GlobalMinLambda_SCALED, GlobalMaxLambda_SCALED;

    CDBZ(picPelNum,"picPelNum");
    bpp_SCALED = (uint32) CamOsMathDivU64(picBit_SCALED, picPelNum, &uiremainder);

    if(picType == I_SLICE)
    {
        IframeCplx = ptMsRc->uiIntraMdlCplx;

        CDBZ(picPelNum,"picPelNum");
        tcPerPel = CamOsMathDivU64(((uint64) IframeCplx << PREC_SCALE_BITS), picPelNum, &uiremainder);
        pow_tcPerPel_beta1_SCALED = (uint32)MhefPow64(tcPerPel, BETA1_SCALED);

        CDBZ(bpp_SCALED,"bpp_SCALED");
        uiInvBpp_SCALED = (uint32)(CamOsMathDivU64(((uint64) PREC_SCALE_FACTOR << PREC_SCALE_BITS), bpp_SCALED, &uiremainder));

        pow_InvBpp_beta = MhefPow64(uiInvBpp_SCALED, beta_SCALED);
        pow_tcPerPel_beta = MhefPow64(pow_tcPerPel_beta1_SCALED, beta_SCALED);
        pow_TcDivBit_beta = (pow_tcPerPel_beta * pow_InvBpp_beta) >> PREC_SCALE_BITS;

        estLambda_SCALED = (uint32)((alpha_SCALED * pow_TcDivBit_beta) >> (PREC_SCALE_BITS + 8));
    }
    else // P_Slice
    {
        //double estLambda = alpha * pow(bpp, beta);
        bpp_pow_beta = MhefPow64(bpp_SCALED, beta_SCALED);
        ui64EstLambda_SCALED = (alpha_SCALED * bpp_pow_beta) >> PREC_SCALE_BITS;
        estLambda_SCALED = (ui64EstLambda_SCALED > (uint64)UINT_MAX) ? (uint32)UINT_MAX : (uint32)ui64EstLambda_SCALED;
    }

    lastPicLambda_SCALED = ptRcPic->uiPicLambda_SCALED;

    // If last frame is I and current frame is P
    ptRcPic->isFirstPpic = (ptRcPic->picType == I_SLICE && picType == P_SLICE) ? 1 : 0;

    if(ptMsRc->uiFrameCnt > ptMsRc->uiGopSize && ptRcPic->isFirstPpic == 0)
    {
        //lastLevelLambda = dClip3( 0.1, 10000.0, lastLevelLambda );
        //estLambda = dClip3( lastLevelLambda * pow( 2.0, -3.0/3.0 ), lastLevelLambda * pow( 2.0, 3.0/3.0 ), estLambda);

        lastLevelLambda_SCALED = ptRcPic->auiLevelLambda_SCALED[picType];
        lastLevelLambda_SCALED = uiClip3(PREC_SCALE_FACTOR / 10, PREC_SCALE_FACTOR * 10000, lastLevelLambda_SCALED);
        estLambda_SCALED = uiClip3(lastLevelLambda_SCALED >> 1, lastLevelLambda_SCALED << 1, estLambda_SCALED);
    }



    if(lastPicLambda_SCALED > 0)
    {
        // const uint32 uiMinValue = (lastPicLambda_SCALED*LAMBDA_MIN_CLIP_NUM)>>LAMBDA_MIN_CLIP_SHIFT;
        //const uint32 uiMaxValue = (lastPicLambda_SCALED*LAMBDA_MAX_CLIP_NUM)>>LAMBDA_MAX_CLIP_SHIFT;
        uiMinValue = ((int64) lastPicLambda_SCALED * ptMsRc->iLambdaMinClipMul) >> PREC_SCALE_BITS;
        uiMaxValue = ((int64) lastPicLambda_SCALED * ptMsRc->iLambdaMaxClipMul) >> PREC_SCALE_BITS;

        estLambda_SCALED = uiClip3(uiMinValue, uiMaxValue, estLambda_SCALED);
    }
    else
    {
        const uint32 uiMinValue = PREC_SCALE_FACTOR / 10;
        const uint32 uiMaxValue = PREC_SCALE_FACTOR * 10000;
        estLambda_SCALED = uiClip3(uiMinValue, uiMaxValue, estLambda_SCALED);
    }

    if(estLambda_SCALED < MIN_EST_LAMBA_SCALED)  // 0.1*65536
    {
        estLambda_SCALED = MIN_EST_LAMBA_SCALED;
    }

    // picture lambda clipping according to global picture Qp clipping
    //int iMaxPicQp = (eSliceType == I_SLICE) ? g_iRcIPicMaxQp : g_iRcPicMaxQp;
    //int iMinPicQp = (eSliceType == I_SLICE) ? g_iRcIPicMinQp : g_iRcPicMinQp;
    //double dGlobalMinLambda = QpToLambda(iMinPicQp - 0.5);
    //double dGlobalMaxLambda = QpToLambda(iMaxPicQp + 0.5);
    //estLambda = Clip3(dGlobalMinLambda, dGlobalMaxLambda, estLambda);
    {
        if(picType == I_SLICE)
        {
            uiMaxPicQp = ptMsRc->uiRcIPicMaxQp;
            uiMinPicQp = ptMsRc->uiRcIPicMinQp;
        }
        else
        {
            uiMaxPicQp = ptMsRc->uiRcPPicMaxQp;
            uiMinPicQp = ptMsRc->uiRcPPicMinQp;
        }

        GlobalMinLambda_SCALED = QpToLambdaScaled(uiMinPicQp - 1);
        GlobalMaxLambda_SCALED = QpToLambdaScaled(uiMaxPicQp + 1);
        estLambda_SCALED = uiClip3(GlobalMinLambda_SCALED, GlobalMaxLambda_SCALED, estLambda_SCALED);
    }

    return estLambda_SCALED;
}

// CModel: estimatePicQP
static uint32 _EstimatePicQpByLambda(ms_rc_top* ptMsRc, int picType, uint32 lambda_SCALED)
{
    ms_rc_pic *ptRcPic = &ptMsRc->tRcPic;
    uint32 lastLevelQP;
    uint32 uiMinQp;
    uint32 uiMaxQp;
    uint32 QpMin;
    uint32 QpMax;
    uint32 lastPicQP;
    uint32 Qp;

    uint32 uiRcPicMaxQp;
    uint32 uiRcPicMinQp;

    if(picType == RQCT_PICTYP_I)
    {
        uiRcPicMinQp = ptMsRc->uiRcIPicMinQp;
        uiRcPicMaxQp = ptMsRc->uiRcIPicMaxQp;
    }
    else
    {
        uiRcPicMinQp = ptMsRc->uiRcPPicMinQp;
        uiRcPicMaxQp = ptMsRc->uiRcPPicMaxQp;
    }

    Qp = LambdaScaledToQp(lambda_SCALED);
    //int isFirstPpic = (ptRcPic->picType==I_SLICE && picType==P_SLICE) ? 1 : 0;
    lastPicQP = ptRcPic->uiPicQp;

    if((ptMsRc->uiFrameCnt > ptMsRc->uiGopSize) && (ptRcPic->isFirstPpic == 0))
    {
        lastLevelQP = ptRcPic->auiLevelQp[picType];
        uiMinQp = (lastLevelQP > uiRcPicMinQp + 3 ? lastLevelQP - 3 : uiRcPicMinQp);
        uiMaxQp = (lastLevelQP < uiRcPicMaxQp - 3 ? lastLevelQP + 3 : uiRcPicMaxQp);
        Qp = uiClip3(uiMinQp, uiMaxQp, Qp);
    }


    {
        QpMin = lastPicQP >= uiRcPicMinQp + g_uiRcPicClipRange ? lastPicQP - g_uiRcPicClipRange : uiRcPicMinQp;
        QpMax = lastPicQP <= uiRcPicMaxQp - g_uiRcPicClipRange ? lastPicQP + g_uiRcPicClipRange : uiRcPicMaxQp;
        Qp = uiClip3(QpMin, QpMax, Qp);
    }

    return Qp;
}

// CModel: getRefineBitsForIntra
static int _EstBitsForIntraFrame(ms_rc_top* ptMsRc)
{
    int iBufferRate = ptMsRc->iPicAvgBit;
    int iBuffUpperBound = ptMsRc->uiTargetBitrate * 9 / 10;
    int iInitBuffStatus = ptMsRc->uiTargetBitrate * 6 / 10;
    int iBuffDeviateRange = ptMsRc->uiTargetBitrate * 5 / 10;
    int iPreviousBitError = ptMsRc->iStreamBitErr;
    int iCurrBuffFullness = iInitBuffStatus + iPreviousBitError;
    uint64 uiremainder;

    // -- Adaptive PI ratio --
    int iTransPoint = iInitBuffStatus + ((iBuffUpperBound - iInitBuffStatus) / 2);

    int iCurrBuffDeviate = iClip3(0, iBuffDeviateRange, iCurrBuffFullness - iTransPoint);
    // Represent ratio by num. / denorm.
    int iCurrPILambdaRatioNum = (g_uiPILambdaRatioNum * iBuffDeviateRange) + (g_uiPILambdaRatioDenorm - g_uiPILambdaRatioNum) * iCurrBuffDeviate;
    int iCurrPILambdaRatioDenorm = g_uiPILambdaRatioDenorm * iBuffDeviateRange;

    // -- I frame bit allocation according to previous P frames --
    rc_model_para *ptRcMdlPara = &ptMsRc->atModelPara[I_SLICE];
    int alpha_SCALED = ptRcMdlPara->Alpha_SCALED;
    int beta_SCALED = ptRcMdlPara->Beta_SCALED;
    uint32 uiLastLambda_SCALED = ptMsRc->HistoryPLambda_SCALED;
    uint32 uiEstBpp_SCALED;

    // lambda scaling
    uint32 uiIntraLambda_SCALED;
    uint32 picPelNum;
    int iNum, iDenorm;
    int iCompensateBit;
    int iIntraBits, iEstBuffFullness;

    CDBZ(iCurrPILambdaRatioDenorm,"iCurrPILambdaRatioDenorm");
    uiIntraLambda_SCALED = CamOsMathDivU64((uint64) uiLastLambda_SCALED * iCurrPILambdaRatioNum, iCurrPILambdaRatioDenorm, &uiremainder);
    picPelNum = ptMsRc->uiPicPixelNum;

    MHE_MSG(MHE_MSG_DEBUG, "alpha_SCALED(%d) beta_SCALED(%d)\n", alpha_SCALED, beta_SCALED);
    MHE_MSG(MHE_MSG_DEBUG, "iCurrPILambdaRatioDenorm(%ld), iBuffDeviateRange(%ld), uiTargetBitrate(%ld),\n",iCurrPILambdaRatioDenorm, iBuffDeviateRange, ptMsRc->uiTargetBitrate);
    MHE_MSG(MHE_MSG_DEBUG, "uiIntraLambda_SCALED(%d), uiLastLambda_SCALED(%d), iCurrPILambdaRatioNum(%d), iCurrPILambdaRatioDenorm(%d)\n", uiIntraLambda_SCALED, uiLastLambda_SCALED, iCurrPILambdaRatioNum, iCurrPILambdaRatioDenorm);

    uiEstBpp_SCALED = _LambdaToBpp_SCALED(ptMsRc, uiIntraLambda_SCALED, I_SLICE, 1);

    // Check default lambda ratio to fall with [0.1, 1.0]
    assert(g_uiPILambdaRatioNum <= g_uiPILambdaRatioDenorm,"<= 1.0"); // <= 1.0
    assert(g_uiPILambdaRatioNum * 10 >= g_uiPILambdaRatioDenorm,">= 0.1"); // >= 0.1

    // Must not be the first frame
    assert(ptMsRc->uiFrameCnt > 0,"ptMsRc->uiFrameCnt > 0");

    // -- Allocated bit surpression --
    iIntraBits = ((uint64) uiEstBpp_SCALED * picPelNum) >> PREC_SCALE_BITS;
    iEstBuffFullness = iCurrBuffFullness - iBufferRate + iIntraBits;
    if(iEstBuffFullness > iBuffUpperBound)  // prevent buffer overflow
    {
        iCompensateBit = iBuffUpperBound + iBufferRate - iCurrBuffFullness;

        iCompensateBit = (iCompensateBit < MIN_PIC_BIT) ? MIN_PIC_BIT : iCompensateBit;

        //orgBitWgt = dClip3(0.5, 0.8, 1.25 - basePILambdaRatio);
        //iIntraBits = (int)(orgBitWgt*iIntraBits + (1.0-orgBitWgt)*iCompensateBit);
        iDenorm = g_uiPILambdaRatioDenorm;
        iNum = g_uiPILambdaRatioNum;
        iNum = iClip3(iDenorm >> 1, (iDenorm << 2) / 5, ((iDenorm * 5) >> 2) - iNum);
        CDBZ(iDenorm,"iDenorm");
        iIntraBits = CamOsMathDivU64(((int64) iNum * iIntraBits + (iDenorm - iNum) * iCompensateBit), iDenorm, &uiremainder);

    }

    iIntraBits = iClip3(ptMsRc->uiMinIPicBit, ptMsRc->uiMaxIPicBit, iIntraBits);

    return iIntraBits;
}



// Partial from CModel: compressGOP
// Input: Qp
// Output: lambda (scaled)
static void _Est1stFrameParamByQp(ms_rc_top* ptMsRc, int picType, uint32 uiQp, int isRTcModel, int* p_retBit, uint32 *p_retLambda_SCALED)
{
    //ms_rc_pic *ptRcPic = &ptMsRc->tRcPic;

// Replaced by C-Model generated g_DefaultLambdaAndBppByQp
    /*
     int NumberBFrames = 0;
     double dLambda_scale = 1.0 - dClip3( 0.0, 0.5, 0.05*(double)NumberBFrames);
     double dQPFactor = (picType==I_SLICE) ? 0.57*dLambda_scale : dLambda_scale;
     const int SHIFT_QP = 12;
     double qp_temp = (double)Qp - SHIFT_QP;//(Double) sliceQP + bitdepth_luma_qp_scale - SHIFT_QP;
     double lambda = dQPFactor*pow( 2.0, qp_temp/3.0 );

     double bpp = LambdaToBpp(ptMsRc, lambda, picType, isRTcModel);
     int bits = (int)(ptMsRc->uiPicPixelNum*bpp);

     *p_retBit = bits;
     *p_retLambda = lambda;
     */
    uint32 uiLambda_SCALED;
    uint32 uiBpp_SCALED;
    uint32 uiBits;

    uiLambda_SCALED = g_DefaultLambdaByQp[uiQp];
    //CamOsPrintf("> [%s %d] uiLambda_SCALED = %d\n", __FUNCTION__, __LINE__, uiLambda_SCALED);
    uiBpp_SCALED = _LambdaToBpp_SCALED(ptMsRc, uiLambda_SCALED, picType, isRTcModel);
    uiBits = ((uint64) ptMsRc->uiPicPixelNum * uiBpp_SCALED) >> PREC_SCALE_BITS;

    *p_retBit = uiBits;
    *p_retLambda_SCALED = uiLambda_SCALED;

}

static void _msrc_gop_start(ms_rc_top* ptMsRc, msrc_rqc* pMsRc)
{
    ms_rc_gop *ptRcGop = &ptMsRc->tRcGop;
    int iSmoothWindowSize = ptMsRc->uiFps * g_iRcStatsTime;
    int iBitErrCompensate = ptMsRc->iStreamBitErr / iSmoothWindowSize;
    uint32 iPicAvgBit = ptMsRc->iPicAvgBit;
    int GOPSize = ptMsRc->uiGopSize;
    int iAllocPicBit;
    int64 iAllocGopBit;
    int iBitCompensLimit;
    int vbrMaxPicBit = iPicAvgBit;

    if(pMsRc->i_method == RQCT_MODE_VBR)
    {
        iPicAvgBit = (iPicAvgBit * pMsRc->i_btratio) / 100;   //g_dRcVBRBitRatio;
    }

    MHE_MSG(MHE_MSG_DEBUG, "iBitErrCompensate = %d \n", iBitErrCompensate);

    // CBR Mode
    iBitCompensLimit = (int)(iPicAvgBit * g_iGOPCompenRatioLimit_MUL100) / 100;
    MHE_MSG(MHE_MSG_DEBUG, "iBitCompensLimit = %d \n", iBitCompensLimit);

    if(pMsRc->i_method == RQCT_MODE_CBR)
        iBitErrCompensate = iClip3(-iBitCompensLimit, iBitCompensLimit, iBitErrCompensate);

    MHE_MSG(MHE_MSG_DEBUG, "iBitErrCompensate = %d \n", iBitErrCompensate);
    iAllocPicBit = iPicAvgBit - iBitErrCompensate;

    if(pMsRc->i_method == RQCT_MODE_VBR)
    {
        iAllocPicBit = min(vbrMaxPicBit, iAllocPicBit);
    }

    if(iAllocPicBit < MIN_PIC_BIT)
    {
        iAllocPicBit = MIN_PIC_BIT;
    }

    //iAllocGopBit = iAllocPicBit * GOPSize;
    iAllocGopBit = (int64)iAllocPicBit * (int64)GOPSize;

    ptRcGop->iGopBitLeft = iAllocGopBit;
    ptRcGop->iPicAllocBit = iAllocPicBit;
    ptRcGop->iGopFrameLeft = GOPSize;

    //MHE_MSG(MHE_MSG_WARNING, "iGopBitLeft(%d), iPicAllocBit(%d), iGopFrameLeft(%d)\n", ptRcGop->iGopBitLeft, ptRcGop->iPicAllocBit, ptRcGop->iGopFrameLeft);
}

static void _msrc_pic_start(ms_rc_top* ptMsRc, int picType)
{
    ms_rc_gop *ptRcGop = &ptMsRc->tRcGop;
    ms_rc_pic *ptRcPic = &ptMsRc->tRcPic;

    int picTargetBit;
    uint32 uiPicQp, picLambda_SCALED;
    int gopAvgPicBit = ptRcGop->iPicAllocBit;
    int64 remainder;

    const uint32 g_uiWeightOrgPicBit_SCALED = (1 << (PREC_SCALE_BITS - 1));

    if(ptMsRc->uiFrameCnt == 0)
    {
        uiPicQp = ptMsRc->uiInitialQp;

        _Est1stFrameParamByQp(ptMsRc, picType, uiPicQp, 1, &picTargetBit, &picLambda_SCALED);

        goto SET_RC_PARA;
    }

    if(picType == I_SLICE)
    {
        picTargetBit = _EstBitsForIntraFrame(ptMsRc);
    }
    else // P_SLICE
    {
        //picTargetBit = ptRcGop->iGopBitLeft / ptRcGop->iGopFrameLeft;
        CDBZ(ptRcGop->iGopFrameLeft,"iGopFrameLeft");
        picTargetBit = (int)CamOsMathDivS64(ptRcGop->iGopBitLeft,ptRcGop->iGopFrameLeft,&remainder);

        if(picTargetBit < MIN_PIC_BIT)
        {
            picTargetBit = MIN_PIC_BIT;
        }

        picTargetBit = (g_uiWeightOrgPicBit_SCALED * (uint64) gopAvgPicBit + (PREC_SCALE_FACTOR - g_uiWeightOrgPicBit_SCALED) * (uint64) picTargetBit) >> PREC_SCALE_BITS;

        picTargetBit = iClip3(ptMsRc->uiMinPicBit, ptMsRc->uiMaxPicBit, picTargetBit); // min/max picture bit clipping
    }

    picLambda_SCALED = _EstPicLambdaByBits(ptMsRc, picType, picTargetBit);
    uiPicQp = _EstimatePicQpByLambda(ptMsRc, picType, picLambda_SCALED);

    // ----------------------------
    // update status
SET_RC_PARA:
    ptRcPic->iTargetBit = picTargetBit;
    ptRcPic->picType = picType;

    assert(picLambda_SCALED >= 0,"lambda < 0");  // No reason to have lambda < 0

    ptRcPic->auiLevelLambda_SCALED[picType] = picLambda_SCALED;
    ptRcPic->uiPicLambda_SCALED = picLambda_SCALED;
    ptRcPic->auiLevelQp[picType] = uiPicQp;
    ptRcPic->uiPicQp = uiPicQp;

    MHE_MSG(MHE_MSG_DEBUG, "picLambda_SCALED = %d\n", picLambda_SCALED);
    MHE_MSG(MHE_MSG_DEBUG, "uiPicQp = %d\n", uiPicQp);

    if(uiPicQp < QP_MIN || uiPicQp > QP_MAX)
        CamOsPrintf("========== Warring!!! uiPicQp is over range !!! ===========\n");
}

static void _msrc_update_stats(ms_rc_top* ptMsRc, int picEncBit, int iHWTextCplxAccum)
{
    ms_rc_gop *ptRcGop = &ptMsRc->tRcGop;
    int64 temp;
    //ms_rc_pic  *ptRcPic = &ptMsRc->tRcPic;
    //int picType = ptRcPic->picType;

    _calcAvgLambdaFromHist(ptMsRc);

    // Convert from sum-of-16x16average to sum-of-pixel
    ptMsRc->uiIntraMdlCplx = iHWTextCplxAccum << 8;

    //ptMsRc->iStreamBitErr += (picEncBit - ptMsRc->iPicAvgBit);
    temp = ptMsRc->iStreamBitErr + (picEncBit - ptMsRc->iPicAvgBit);
    ptMsRc->iStreamBitErr = iClip3(INT_MIN, INT_MAX, temp);


    ptRcGop->iGopBitLeft -= picEncBit;

    ptMsRc->uiFrameCnt++;
    ptRcGop->iGopFrameLeft--;

#if 0
    //ptMsRc->iStreamBitErr = 0;
    if((ptMsRc->uiFrameCnt % 300) == 0)
        MHE_MSG(MHE_MSG_WARNING, "ptMsRc->iStreamBitErr (%d) \n", ptMsRc->iStreamBitErr);

    //if(ptRcGop->iGopBitLeft < 0)
    //    MHE_MSG(MHE_MSG_WARNING, "ptRcGop->iGopBitLeft (%d) \n", ptRcGop->iGopBitLeft);
#endif

    MHE_MSG(MHE_MSG_DEBUG, "ptRcGop->iGopBitLeft = %d\n", ptRcGop->iGopBitLeft);
}

// lambda-bpp model:
//   I-frame: Lambda = (Alhpa/256) * (TC/Bpp)^Beta
//   P-frame: Lambda = Alpha * Bpp^Beta
// Update:
//   dAlpha, dBeta
//   dLastAlpha, dLastBeta, dLastAlphaIncr, dLastBetaIncr
//   dHitoryPLambda
static void _msrc_update_model(ms_rc_top* ptMsRc, int picActualBit, int picType)
{
    rc_model_para *ptRcMdlPara = &ptMsRc->atModelPara[picType];
    ms_rc_pic *ptRcPic = &ptMsRc->tRcPic;
    int alpha_SCALED = ptRcMdlPara->Alpha_SCALED;
    int beta_SCALED = ptRcMdlPara->Beta_SCALED;
    uint32 picPelNum = ptMsRc->uiPicPixelNum;
    uint64 uiremainder;
    int64 remainder;
    int picTargetBit;
    uint32 IframeCplx;
    uint32 CplxPerPel_SCALE;
    uint32 pow_cplxperpel_SCALED;
    int lnbpp_SCALED;
    uint32 actbit_div_targetbit;
    int64 log_actbit_div_targetbit;
    int iDiffLambda_SCALED;
    int AlphaIncr_SCALED, BetaIncr_SCALED, delAlpha_SCALED, delBeta_SCALED;
    int64 IncrTerm_SCALED;
    uint32 picActualBpp;
    uint32 uiCalLambda_SCALED;
    uint32 uiInputLambda_SCALED;
    uint32 uiStepSizeA_SCALED;
    uint32 uiStepSizeB_SCALED;
    int iAlphaScaleR;
    int iBetaScaleR;
    int64 alphaScale;
    int64 betaScale;
    int alphaUpdate_SCALED;
    int betaUpdate_SCALED;
    int lastDeltaAlpha_SCALED;
    int lastDeltaBeta_SCALED;
    int lastDelAlphaIncr_SCALED;
    int lastDelBetaIncr_SCALED;
    int STEP_LIMIT_A_SCALED;
    int STEP_LIMIT_B_SCALED;
    uint32 uiMaxCalLambdaClip;
    uint64 ui64CalLambda_SCALED;

    if(picType == I_SLICE)
    {
        picTargetBit = ptRcPic->iTargetBit;
        IframeCplx = ptMsRc->uiIntraMdlCplx;
        CDBZ(picPelNum,"picPelNum");
        CplxPerPel_SCALE = CamOsMathDivU64((uint64) IframeCplx << PREC_SCALE_BITS, picPelNum, &uiremainder);

        //double lnbpp = log(pow(IframeCplx/picPelNum, g_bRcICostExpt));
        //double diffLambda = beta*(log(picActualBits)-log(picTargetBit));
        CDBZ(CplxPerPel_SCALE,"CplxPerPel_SCALE");
        pow_cplxperpel_SCALED = (uint32)MhefPow64(CplxPerPel_SCALE, BETA1_SCALED);
        CDBZ(pow_cplxperpel_SCALED,"pow_cplxperpel_SCALED");
        lnbpp_SCALED = MhefLog(pow_cplxperpel_SCALED);
        if(lnbpp_SCALED == 0) //pow_cplxperpel_SCALED = 65535~ 65537
          lnbpp_SCALED = 1;

        CDBZ(picTargetBit,"picTargetBit");
        actbit_div_targetbit = (uint32) CamOsMathDivU64((uint64) picActualBit << PREC_SCALE_BITS, picTargetBit, &uiremainder);
        log_actbit_div_targetbit = MhefLog(actbit_div_targetbit);
        iDiffLambda_SCALED = (int)((beta_SCALED * (log_actbit_div_targetbit)) >> PREC_SCALE_BITS);

        //diffLambda = dClip3(-0.125, 0.125, 0.25*diffLambda);
        //alpha = alpha*exp(diffLambda);
        //beta = beta + diffLambda / lnbpp;
        iDiffLambda_SCALED = iClip3(-8192, 8192, iDiffLambda_SCALED / 4); // NEED_CHANGE_BY_PREC_SCALE_BITS
        alpha_SCALED = (alpha_SCALED * (uint64) MhefExp(iDiffLambda_SCALED)) >> PREC_SCALE_BITS;
        CDBZ(lnbpp_SCALED,"lnbpp_SCALED");
        beta_SCALED = beta_SCALED + CamOsMathDivS64((int64) iDiffLambda_SCALED << PREC_SCALE_BITS, lnbpp_SCALED, &remainder);

        if((alpha_SCALED == 0) || (beta_SCALED == 0))
        {
            MHE_MSG(MHE_MSG_WARNING, "%d: alpha/beta_SCALED(%d / %d), iDiffLambda_SCALED(%d)\n", __LINE__,alpha_SCALED, beta_SCALED, iDiffLambda_SCALED);
        }

        alpha_SCALED = iClip3(g_iRCIntraAlphaMinValue_SCALED, g_iRCIntraAlphaMaxValue_SCALED, alpha_SCALED);
        beta_SCALED  = iClip3(g_iRCIntraBetaMinValue_SCALED,  g_iRCIntraBetaMaxValue_SCALED,  beta_SCALED);

    }
    else
    {

        CDBZ(picPelNum,"picPelNum");
        picActualBpp = (uint32) CamOsMathDivU64((uint64) picActualBit << PREC_SCALE_BITS, picPelNum, &uiremainder);

        lnbpp_SCALED = MhefLog(picActualBpp);

        ui64CalLambda_SCALED = ((int64)alpha_SCALED * MhefPow64(picActualBpp, beta_SCALED)) >> PREC_SCALE_BITS;
        uiCalLambda_SCALED = (ui64CalLambda_SCALED > (uint64)UINT_MAX) ? (uint32)UINT_MAX : (uint32)ui64CalLambda_SCALED;
        //CDBZ(picActualBpp);
        //uiCalLambda_SCALED = (uint32) (((int64) alpha_SCALED * fPow(picActualBpp, beta_SCALED)) >> PREC_SCALE_BITS);
        uiInputLambda_SCALED = ptRcPic->uiPicLambda_SCALED;
        uiStepSizeA_SCALED = ptMsRc->uiStepAlpha_SCALED;
        uiStepSizeB_SCALED = ptMsRc->uiStepBeta_SCALED;

        //  escape Method for in-accurate pow()
        if(picActualBpp <= 131)  // 0.002*65536 // NEED_CHANGE_BY_PREC_SCALE_BITS
        {
            /*
             alpha *= ( 1.0 - m_encRCSeq->getAlphaUpdate() / 2.0 );
             beta  *= ( 1.0 - m_encRCSeq->getBetaUpdate() / 2.0 );
             */
            iAlphaScaleR = uiStepSizeA_SCALED >> 1;
            iBetaScaleR = uiStepSizeB_SCALED >> 1;
            alphaScale = PREC_SCALE_FACTOR + ((uiInputLambda_SCALED > uiCalLambda_SCALED) ? iAlphaScaleR : -iAlphaScaleR);
            betaScale = PREC_SCALE_FACTOR + ((uiInputLambda_SCALED > uiCalLambda_SCALED) ? iBetaScaleR : -iBetaScaleR);

            alphaUpdate_SCALED = (int)(((int64) alpha_SCALED * alphaScale) >> PREC_SCALE_BITS);
            betaUpdate_SCALED = (int)(((int64) beta_SCALED * betaScale) >> PREC_SCALE_BITS);

            alpha_SCALED = iClip3(g_iRCAlphaMinValue_SCALED, g_iRCAlphaMaxValue_SCALED, alphaUpdate_SCALED);
            beta_SCALED = iClip3(g_iRCBetaMinValue_SCALED, g_iRCBetaMaxValue_SCALED, betaUpdate_SCALED);

            goto UPDATE_PARAMETERS;
        }
        //calLambda = dClip3(inputLambda/10.0, inputLambda*10.0, calLambda);
        //AlphaIncr = (log(inputLambda) - log(calLambda)) * alpha;
        uiMaxCalLambdaClip = ((uiInputLambda_SCALED >> PREC_SCALE_BITS) < ((uint32)PREC_SCALE_FACTOR / 10)) ? uiInputLambda_SCALED * 10 : ((uint32)PREC_SCALE_FACTOR / 10) << PREC_SCALE_BITS;

        uiCalLambda_SCALED = uiClip3(uiInputLambda_SCALED / 10, uiMaxCalLambdaClip, uiCalLambda_SCALED);

        IncrTerm_SCALED = (int64)(MhefLog(uiInputLambda_SCALED) - MhefLog(uiCalLambda_SCALED));

        CDBZ(PREC_SCALE_FACTOR,"PREC_SCALE_FACTOR");
        AlphaIncr_SCALED = (int) CamOsMathDivS64((IncrTerm_SCALED * alpha_SCALED), PREC_SCALE_FACTOR, &remainder);

        //lnbpp = dClip3(-5.0, -0.1, lnbpp );
        //BetaIncr = (log(inputLambda) - log(calLambda)) * lnbpp;
        lnbpp_SCALED = iClip3(-(PREC_SCALE_FACTOR * 5), -(PREC_SCALE_FACTOR / 10), lnbpp_SCALED);
        CDBZ(PREC_SCALE_FACTOR,"PREC_SCALE_FACTOR");
        BetaIncr_SCALED = (int) CamOsMathDivS64((IncrTerm_SCALED * lnbpp_SCALED), PREC_SCALE_FACTOR, &remainder);

        // -- adaptive step size decision --
        if(ptRcPic->isFirstPpic == 0)
        {
            lastDeltaAlpha_SCALED = alpha_SCALED - ptMsRc->LastAlpha_SCALED;
            lastDeltaBeta_SCALED = beta_SCALED - ptMsRc->LastBeta_SCALED;
            lastDelAlphaIncr_SCALED = AlphaIncr_SCALED - ptMsRc->LastAlphaIncr_SCALED;
            lastDelBetaIncr_SCALED = BetaIncr_SCALED - ptMsRc->LastBetaIncr_SCALED;

            STEP_LIMIT_A_SCALED = uiStepSizeA_SCALED >> 2;
            STEP_LIMIT_B_SCALED = uiStepSizeB_SCALED >> 2;

            // numerical regulator: add "1" to prevent divide 0
            lastDelAlphaIncr_SCALED = iabs(lastDelAlphaIncr_SCALED) + 1;
            lastDelBetaIncr_SCALED = iabs(lastDelBetaIncr_SCALED) + 1;
            CDBZ(lastDelAlphaIncr_SCALED,"lastDelAlphaIncr_SCALED");
            uiStepSizeA_SCALED = iabs((int)(CamOsMathDivS64((int64) lastDeltaAlpha_SCALED << PREC_SCALE_BITS, lastDelAlphaIncr_SCALED, &remainder)));
            CDBZ(lastDelBetaIncr_SCALED,"lastDelBetaIncr_SCALED");
            uiStepSizeB_SCALED = iabs((int)(CamOsMathDivS64((int64) lastDeltaBeta_SCALED << PREC_SCALE_BITS, lastDelBetaIncr_SCALED, &remainder)));

            uiStepSizeA_SCALED = iClip3(STEP_LIMIT_A_SCALED >> 2, STEP_LIMIT_A_SCALED << 2, uiStepSizeA_SCALED);
            uiStepSizeB_SCALED = iClip3(STEP_LIMIT_B_SCALED >> 2, STEP_LIMIT_B_SCALED << 2, uiStepSizeB_SCALED);
        }
        // update
        ptMsRc->LastAlpha_SCALED = alpha_SCALED;
        ptMsRc->LastBeta_SCALED = beta_SCALED;
        ptMsRc->LastAlphaIncr_SCALED = AlphaIncr_SCALED;
        ptMsRc->LastBetaIncr_SCALED = BetaIncr_SCALED;

        CDBZ(PREC_SCALE_FACTOR,"PREC_SCALE_FACTOR");
        delAlpha_SCALED = CamOsMathDivS64(((int64) uiStepSizeA_SCALED * AlphaIncr_SCALED), PREC_SCALE_FACTOR, &remainder);
        CDBZ(PREC_SCALE_FACTOR,"PREC_SCALE_FACTOR");
        delBeta_SCALED = CamOsMathDivS64(((int64) uiStepSizeB_SCALED * BetaIncr_SCALED), PREC_SCALE_FACTOR, &remainder);

        // ---------------------------------------------------
        alpha_SCALED += delAlpha_SCALED;
        beta_SCALED += delBeta_SCALED;

        alpha_SCALED = iClip3(g_iRCAlphaMinValue_SCALED, g_iRCAlphaMaxValue_SCALED, alpha_SCALED);
        beta_SCALED = iClip3(g_iRCBetaMinValue_SCALED, g_iRCBetaMaxValue_SCALED, beta_SCALED);
    }

UPDATE_PARAMETERS:

    CDBZ(alpha_SCALED,"alpha_SCALED");
    CDBZ(beta_SCALED,"beta_SCALED");

    ptRcMdlPara->Alpha_SCALED = alpha_SCALED;
    ptRcMdlPara->Beta_SCALED = beta_SCALED;

    if(picType == P_SLICE)
    {
        uint32 currLambda_SCALED = iClip3(PREC_SCALE_FACTOR / 10, 10000 << PREC_SCALE_BITS, ptRcPic->auiLevelLambda_SCALED[P_SLICE]);
        uint32 updateLambda_SCALED;

        if(ptRcPic->isFirstPpic)
        {
            updateLambda_SCALED = currLambda_SCALED;
        }
        else
        {
            updateLambda_SCALED = (g_iRCWeightHistoryLambda_MUL10 * ptMsRc->HistoryPLambda_SCALED + (10 - g_iRCWeightHistoryLambda_MUL10) * currLambda_SCALED) / 10;
        }
        ptMsRc->HistoryPLambda_SCALED = updateLambda_SCALED;
    }
}
