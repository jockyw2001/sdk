
#ifndef _MFE_DC_KERNEL_H_
#define _MFE_DC_KERNEL_H_

#include "mhal_venc.h"

typedef struct MfeDcCtx_t mmfedc_ctx;
typedef struct MfeDcDev_t mmfedc_dev;

void (*MfeEventHandler)(MHAL_VENC_EncResult_t *);

struct MfeDcCtx_t
{
    void*           pCtx[2];
    mmfedc_dev*     pDcDev;
};

struct MfeDcDev_t
{
    void*           pDev[2];
    int             nIrq[2];
    void (*Dev0Cb)(MHAL_VENC_EncResult_t *);
    void (*Dev1Cb)(MHAL_VENC_EncResult_t *);
};

#endif //_MFE_DC_KERNEL_H_
