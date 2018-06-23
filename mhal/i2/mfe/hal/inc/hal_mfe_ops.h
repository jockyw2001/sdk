
#ifndef _MFE_OPS_H_
#define _MFE_OPS_H_

#include "mhve_ops.h"

#include "hal_mfe_def.h"
#include "hal_h264_enc.h"
#include "hal_mfe_reg.h"
#include "hal_mfe_ios.h"
#include "hal_mfe_rqc.h"
#include "hal_mfe_pmbr.h"

typedef struct Rpb_t
{
    char    b_valid;            /* 1: allocated direct memory. 0: use pic+'s memory */
#define RPB_STATE_FREE     0
#define RPB_STATE_BUSY     1
#define RPB_STATE_KEEP     2       // Identify this recn buffer is used to LTR reference picture
#define RPB_STATE_INVL    -1
#define IS_FREE(s)        ((s)==RPB_STATE_FREE)
#define IS_BUSY(s)        ((s)==RPB_STATE_BUSY)
#define IS_KEEP(s)        ((s)==RPB_STATE_KEEP)
#define IS_INVL(s)        ((s)==RPB_STATE_INVL)
    char    i_state;            /* Buffer Status */
    short   i_index;            /* Buffer Index */
#define RPB_YPIX           0
#define RPB_CPIX           1
#define RPB_SIZE           2
    uint    u_phys[RPB_SIZE];   /* Buffer Physical Address */
} rpb_t;

typedef enum AVC_REF_LIST
{
    AVC_REF_L0  = 0,
    AVC_REF_L1  = 1,
    AVC_REF_LTR = 2,
    AVC_REF_MAX = 3,
} AVC_REF_LIST;

typedef struct MfeOps_t
{
    mhve_ops    ops;            /* MFE Operations */
    /* MFE-registers */
    mfe6_reg*   p_regs;         /* Register Mirror Handle */
    /* Rate control */
    rqct_ops*   p_rqct;         /* RQCT Handle */
    pmbr_ops*   p_pmbr;         /* PMBR Handle */
    /* Backup rate control */
    rqct_ops*   p_rqct_bak;         /* RQCT Handle */
    pmbr_ops*   p_pmbr_bak;         /* PMBR Handle */
    /* Seq counter */
    mhve_vpb    m_encp;         /* Video Picture Buffer (Input Frame) */
    int         i_seqn;         /* Coded Picture Count */
    int         b_seqh;         /* Encode SPS, PPS or not */
    /* Current resource */
    mhve_pix_e  e_pixf;         /* Input Picture Format */
    short       i_pixw, i_pixh; /* Frame Width and Frame Height (Unit:Pixel) */
    int         i_rpbn;         /* Check:Unused */
    uint        u_conf;         /* Check:Unused */
    short       i_pctw, i_pcth; /* Aligned Frame Width and Frame Height */
    short       i_mbsw, i_mbsh; /* Frame Width and Frame Height (Unit:MB) */
    int         i_mbsn;         /* Frame MB Count */
    /* Motion search */
    int         i_dmvx;         /* Motion Estimation Search Range +-i_dmvx */
    int         i_dmvy;         /* Motion Estimation Search Range +-i_dmvy */
    int         i_subp;         /* Sub-Pixel Estimation Flag */
    uint        i_blkp[2];      /* Block Prediction Flag */
    /* NALU encode */
    sps_t       m_sps;          /* SPS NALU */
    pps_t       m_pps;          /* PPS NALU */
    slice_t     m_sh;           /* Slice Header */
    /* Backup NALU encode */
    slice_t     m_sh_bak;           /* Slice Header */
    /* Multi-slices */
    int         i_rows;         /* Slice produced by MB Rows */
    int         i_bits;         /* Slice produced by Bits */
    /* Feedback and statistic */
    int         i_total;        /* Total Coded Bitstream Byte Count */
    int         i_obits;        /* Coded Bitstream Bit Count (Current Frame) */
    /* Output bits */
    char*       p_okptr;        /* Temporary buffer pointer */
    uint        u_obase;        /* Output Bitstream Buffer Base Address */
    uint        u_osize;        /* Output Bitstream Buffer Total Free Size */
    uint        u_obias;
    uint        u_oused;        /* Output Bitstream Buffer Used Size (Unit:Byte) */
    uint        u_vacan;        /* Check:Unused */
    /* Get neighbor */
    uint        u_mbp_base;     /* For GN Use */
    /* Frame buffers */
#define RPB_MAX_NR    4
    int         i_refn;         /* Reference Picture Number */
    rpb_t*      p_recn;         /* Current Reconstructed Picture */
    rpb_t*      m_dpbs[AVC_REF_MAX][RPB_MAX_NR]; /* Reference Picture Buffer */
    rpb_t       m_rpbs[RPB_MAX_NR];              /* Reconstructed Picture Buffer */
    /* Backup frame buffers */
    rpb_t*      p_recn_bak;         /* Current Reconstructed Picture */
    rpb_t*      m_dpbs_bak[AVC_REF_MAX][RPB_MAX_NR]; /* Reference Picture Buffer */
    rpb_t       m_rpbs_bak[RPB_MAX_NR]; /* Reconstructed Picture Buffer */
    /* Sequence header */
    int         i_seqh;         /* SPS and PPS NALU bit count */
    uchar       m_seqh[64];     /* SPS and PPS NALU temporary buffer */
    /* Picture header */
    int         i_pich;         /* Slice Header NALU bit count */
    uchar       m_pich[32];     /* Slice Header NALU temporary buffer */
    /* LTR */
    int         b_enable_pred;  /* 0: P ref. I-frame, 1: P ref P-frame */
    /* Dump Register Buffer */
    uint        p_drpptr;       /* Physical Address of Dump Register Buffer */
    uint        p_drvptr;       /* Virtual Address of Dump Register Buffer */
    uint        u_drsize;       /* Size of Dump Register Buffer */
    /* Clock set */
    char        i_ClkEn;        /* Enable MFE Clock */
    char        i_ClkSor;       /* MFE Clock Source */
    unsigned long i_NotifySize; /* Partial Output Size */
    void        (*notifyFunc)(unsigned long nBufAddr, unsigned long nNotifyOffset, unsigned long nNotifySize, unsigned short bFrameDone);
    int         (*flush_cache)(void *va, unsigned int len); /*fush cache callback function*/
    /* frame configuration */
    char i_qpoffset;
    uint i_iframe_bitsthr;
    uint i_pframe_bitsthr;
} mfe6_ops;

#endif /*_MFE_OPS_H_*/
