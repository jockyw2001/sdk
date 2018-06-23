
#ifndef _MHE_OPS_H_
#define _MHE_OPS_H_

#include "mhve_ops.h"

#include "hal_mhe_def.h"
#include "hal_h265_enc.h"
#include "hal_mhe_reg.h"
#include "hal_mhe_ios.h"
#include "hal_mhe_rqc.h"
#include "hal_mhe_pmbr.h"


typedef struct Rpb_t
{
    char    b_valid;            /* 1: allocated direct memory. 0: use pic+'s memory */
#define RPB_STATE_FREE     0
#define RPB_STATE_BUSY     1
#define RPB_STATE_INVL    -1
#define IS_FREE(s)        ((s)==RPB_STATE_FREE)
#define IS_INVL(s)        ((s)==RPB_STATE_INVL)
    char    i_state;            /* Buffer Status */
    short   i_index;            /* Buffer Index */
#define RPB_YPIX           0
#define RPB_CPIX           1
#define RPB_TMVP           2    //colocated info buffer for writing/reading
#define RPB_SIZE           3
    uint    u_phys[RPB_SIZE];   /* Buffer Physical Address */
} rpb_t;

typedef struct MheOps_t
{
    mhve_ops    ops;            /* MHE Operations */
    h265_enc*   h265;           /* H265 Header*/
    /* Backup h265 header */
    h265_enc*   h265_bak;       /* H265 Header*/
    /* MHE-registers */
    mhe_reg*   p_regs;         /* Register Mirror Handle */
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
#define OPS_SEQH_START  (1<<2)
#define OPS_SEQH_RESET  (1<<1)
#define OPS_SEQH_WRITE  (1<<0)
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
    //slice_t     m_sh;           /* Slice Header */
    /* Multi-slices */
    int         i_rows;         /* Slice produced by CTB Rows */
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
    rpb_t*      m_dpbs[RPB_MAX_NR]; /* Reference Picture Buffer */
    rpb_t       m_pics[RPB_MAX_NR]; /* Reconstructed Picture Buffer */
    /* Backup frame buffers */
    rpb_t*      p_recn_bak;         /* Current Reconstructed Picture */
    rpb_t*      m_dpbs_bak[RPB_MAX_NR]; /* Reference Picture Buffer */
    rpb_t       m_pics_bak[RPB_MAX_NR]; /* Reconstructed Picture Buffer */
    /* Sequence header */
    int         i_seqh;         /* SPS and PPS NALU bit count */
    uchar       m_seqh[64];     /* SPS and PPS NALU temporary buffer */
    /* Picture header */
    int         i_pich;         /* Slice Header NALU bit count */
    uchar       m_pich[32];     /* Slice Header NALU temporary buffer */
    /* Dump Register Buffer */
    uint        p_drpptr;       /* Physical Address of Dump Register Buffer */
    uint        p_drvptr;       /* Virtual Address of Dump Register Buffer */
    uint        u_drsize;       /* Size of Dump Register Buffer */
    /* Clock set */
    char        i_ClkEn;        /* Enable MHE Clock */
    char        i_ClkSor;       /* MHE Clock Source */
    int         (*flush_cache)(void *va, unsigned int len); /*fush cache callback function*/
    /* MHE Buffer */
    // From I2 C-model
    // (When low-bandwidth mode) In SRAM (Otherwise in DRAM)
    uint gn_mem;                              // TXIP intermediate data
    uint ppu_int_b;                           // PPU intermediate data
    // (Only needed when low-bandwidth mode) In SRAM
    uint imi_ref_y_buf;
    uint imi_ref_c_buf;
    // In DRAM
    uint ppu_int_a;                           // PPU intermediate data
    /* frame configuration */
    char i_qpoffset;
    uint i_iframe_bitsthr;
    uint i_pframe_bitsthr;
} mhe_ops;

#endif /*_MHE_OPS_H_*/
