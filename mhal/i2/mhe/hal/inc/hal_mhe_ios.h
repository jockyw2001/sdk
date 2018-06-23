
#ifndef _MHE_IOS_H_
#define _MHE_IOS_H_

#include "mhve_ios.h"

#define MHEIOS_MAXBASE_NUM 4

typedef struct MheIos_t
{
    mhve_ios    mios;
    void*       p_base[MHEIOS_MAXBASE_NUM];
    mhe_reg*    p_regs;
} mhe_ios;

#define BIT_ENC_DONE    (1<<0)
#define BIT_BUF_FULL    (1<<1)

#endif /*_MHE_IOS_H_*/
