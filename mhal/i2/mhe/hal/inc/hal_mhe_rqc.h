
#ifndef _MHE_RCTL_H_
#define _MHE_RCTL_H_

#include "mhve_rqct_ops.h"

typedef struct MheRctl_t
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
} mhe_rqc;

#define QP_UPPER    (51)    //(48)
#define QP_LOWER    (0)
#define QP_RANGE    (QP_UPPER-QP_LOWER)
#define QP_MAX      (51)
#define QP_MIN      (0)
#define QP_IFRAME_DELTA -2     //I-frame QP delta from P-frame

void* RqctMheAcquire(int);
char* RqctMheComment(int);

/* basic functions of mhe rate-controller */
int MheMrqcSetRqcf(rqct_ops* rqct, rqct_cfg* rqcf);
int MheMrqcGetRqcf(rqct_ops* rqct, rqct_cfg* rqcf);
int MheMrqcEncBuff(rqct_ops* rqct, rqct_buf* buff);
int MheMrqcRoiDraw(rqct_ops* rqct, mhve_job* mjob);

/* functions to insert/remove additional rate-controller */
int RqctMheInsert(void * (*allc)(void), char * (*desc)(void));
int RqctMheRemove(void);

#endif /*_MHE_RCTL_H_*/
