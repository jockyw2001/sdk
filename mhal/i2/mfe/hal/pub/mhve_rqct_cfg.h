
#ifndef _RQCT_CFG_H_
#define _RQCT_CFG_H_

typedef union  rqct_cfg rqct_cfg;
typedef struct rqct_buf rqct_buf;

union rqct_cfg {
    enum rqct_cfg_e {
    RQCT_CFG_SEQ = 0,
    RQCT_CFG_DQP,
    RQCT_CFG_QPR,
    RQCT_CFG_LOG,
    RQCT_CFG_PEN,
    RQCT_CFG_SPF,
    RQCT_CFG_LTR,       // Long term reference
    /* below RQCT_CFG type MUST one by one match rqct_conf_e in mdrv_rqct_st.h */
    RQCT_CFG_RES=32,
    RQCT_CFG_FPS,
    RQCT_CFG_ROI,
    RQCT_CFG_BGF,
    RQCT_CFG_QPM,
    RQCT_CFG_DQM,
    RQCT_CFG_ZMV,
    RQCT_CFG_RCM,
    RQCT_CFG_OFT,
    } type;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_SEQ"
    enum rqct_mode {
        RQCT_MODE_CQP = 0,
        RQCT_MODE_CBR,
        RQCT_MODE_VBR,
    }               i_method;           /* Rate Control Method */
    int             i_period;           /* GOP */
    int             i_leadqp;           /* Frame Level QP */
    int             i_btrate;           /* Bitrate */
    int             b_passiveI;         /* Passive encode I frame */
    } seq;

    struct
    {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_LTR"
    int i_period;
    } ltr;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_DQP"
    int             i_dqp;              /* QP Delta Range */
    } dqp;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_QPR"
    int             i_iupperq;          /* QP Upper Bound */
    int             i_ilowerq;          /* QP Lower Bound */
    int             i_pupperq;          /* QP Upper Bound */
    int             i_plowerq;          /* QP Lower Bound */
    } qpr;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_LOG"
    int             b_logm;             /* Log Enable */
    } log;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_PEN"
    short           b_i16pln;           /* Intra 16x16 Planar Mode Enable */
    short           i_peni4x;           /* Intra 4x4 Cost Penalty */
    short           i_peni16;           /* Intra 16x16 Cost Penalty */
    short           i_penint;           /* Inter Cost Penalty */
    short           i_penYpl;           /* Intra 16x16 Lume Cost Penalty */
    short           i_penCpl;           /* Intra 8x8 Chroma Cost Penalty */
    } pen;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_RCM"
    enum rqct_spfrm_mode {
        RQCT_SPFRM_NONE = 0,            //!< super frame mode none.
        RQCT_SPFRM_DISCARD,             //!< super frame mode discard.
        RQCT_SPFRM_REENCODE,            //!< super frame mode re-encode.
    } e_spfrm;
    int             i_IfrmThr;
    int             i_PfrmThr;
    int             i_BfrmThr;
    } spf;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_RES"
    short           i_picw;             /* Picture Width */
    short           i_pich;             /* Picture Height */
    } res;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_FPS"
    short           n_fps;              /* FPS Numerator */
    short           d_fps;              /* FPS Denominator */
    } fps;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_ROI"
    unsigned int    u_roienb;           /* ROI Rectangle Enable */
    short           i_roiidx;           /* ROI Rectangle Index */
    short           i_absqp;            /* ROI Rectangle Absolute QP Flag */
    short           i_roiqp;            /* ROI Rectangle QP (abs or offset) */
    short           i_posx;             /* ROI Rectangle StartX */
    short           i_posy;             /* ROI Rectangle StartY */
    short           i_recw;             /* ROI Rectangle Width */
    short           i_rech;             /* ROI Rectangle Height */
    } roi;

    struct
    {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_FPS"
    int             i_bgsrcfps;         /* Background Source FPS  */
    int             i_bgdstfps;         /* Background Destination FPS  */
    } bgf;

    struct
    {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_QPM"
    int             i_qpmenb;           /* QpMap Enable */
    int             i_entry[14];        /* QpMap Entry (Offset QP) */
    void*           p_kptr;             /* QpMap Virtual Address */
    } qpm;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_DQM"
    unsigned int    u_phys;             /* DQM Buffer Physical Address */
    void*           p_kptr;             /* DQM Buffer Virtual Address */
    short           i_dqmw;             /* DQM MB Width */
    short           i_dqmh;             /* DQM MB Height */
    int             i_size;             /* DQM Buffer Size */
    int             i_unit;
    } dqm;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_ZMV"
    unsigned int    u_phys;             /* ZMV Buffer Physical Address */
    void*           p_kptr;             /* ZMV Buffer Virtual Address */
    short           i_zmvw;             /* ZMV MB Width */
    short           i_zmvh;             /* ZMV MB Height */
    int             i_size;             /* ZMV Buffer Size */
    int             i_unit;
    } zmv;

    struct {
    enum rqct_cfg_e i_type;             //!< MUST BE "RQCT_CFG_RCM"
    unsigned int    u_phys;
    void*           p_kptr;
    int             i_size;
    } rcm;

    struct {
    enum rqct_cfg_e i_type;     //!< i_type MUST be RQCT_CFG_OFT
    short i_offsetqp;
    } oft;
};

struct  rqct_buf {
    unsigned int    u_config;   /* Not Used Now */
};

#endif //_RQCT_CFG_H_
