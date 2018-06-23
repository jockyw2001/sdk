
#ifndef _MFE_RCTL_H_
#define _MFE_RCTL_H_

#include "mhve_rqct_ops.h"

typedef struct MfeRctl_t
{
    rqct_ops    rqct;               /* RQCT Operations */
    rqct_att    attr;               /* RQCT Attribute */
    int         i_period;           /* GOP */
    int         i_ltrperiod;        /* LTR P-frame / P-frame period */
    int         i_pcount;           /* Encoded Picture Count (0, 1 ~ i_period) */
    int         i_ltrpcount;        /* LTR-frame/P-frame count up to period */
    int         b_passiveI;         /* Passive encode I frame */
    int         b_seqhead;          /* Flag of sequence head */
#define RCTL_CFG_MB_DQP      (1<<0)
#define RCTL_CFG_NIGHT       (1<<2)
    int         i_refcnt;           /* Reference Frame Count */
    int         i_config;           /* Not Used Now */
} mfe6_rqc;

#define QP_UPPER    (51)
#define QP_LOWER    (0)
#define QP_RANGE    (QP_UPPER-QP_LOWER)
#define QP_MAX      (51)
#define QP_MIN      (0)
#define QP_IFRAME_DELTA -2     //I-frame QP delta from P-frame

void* RqctMfeAcquire(int);
char* RqctMfeComment(int);

/* basic functions of mfe rate-controller */
int MrqcSetRqcf(rqct_ops* rqct, rqct_cfg* rqcf);
int MrqcGetRqcf(rqct_ops* rqct, rqct_cfg* rqcf);
int MrqcEncBuff(rqct_ops* rqct, rqct_buf* buff);
int MrqcRoiDraw(rqct_ops* rqct, mhve_job* mjob);

/* functions to insert/remove additional rate-controller */
int RqctMfeInsert(void*(*allc)(void), char*(*desc)(void));
int RqctMfeRemove(void);

#endif /*_MFE_RCTL_H_*/
