
// mstar hardware video encoder I/F (mhveif)

#ifndef _PMBR_OPS_H_
#define _PMBR_OPS_H_

#include "mhve_ios.h"
#include "mhve_pmbr_cfg.h"

typedef struct pmbr_ops pmbr_ops;

struct pmbr_ops
{
    void (*release)(pmbr_ops* pmbr);
    int (*seq_sync)(pmbr_ops* pmbr, mhve_job* mjob);
    int (*seq_conf)(pmbr_ops* pmbr);
    int (*seq_done)(pmbr_ops* pmbr);
    int (*set_conf)(pmbr_ops* pmbr, pmbr_cfg* pcfg);
    int (*get_conf)(pmbr_ops* pmbr, pmbr_cfg* pcfg);
    int (*enc_conf)(pmbr_ops* pmbr, mhve_job* mjob);
    int (*enc_done)(pmbr_ops* pmbr, mhve_job* mjob);

    int i_LutTyp;  //0: normal, 1: single entry, 2: manual type (for ROI)
    int i_LutRoiQp[PMBR_LUT_SIZE];
    int i_FrmQP;
    int i_FrmLamdaScaled;
    int i_PicTyp;
    int i_MinQP;
    int i_MaxQP;
    int i_FrmCnt;

};

#endif //_PMBR_OPS_H_
