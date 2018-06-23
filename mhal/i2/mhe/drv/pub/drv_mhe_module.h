#ifndef _DRV_MHE_MODULE_H_
#define _DRV_MHE_MODULE_H_

#include "drv_mhe_kernel.h"

#define MHE_OFFSET  0x00264800
#define MHE_SIZE    0x200

mmhe_ctx*   MheOpen(mmhe_dev* mdev, mmhe_ctx* mctx);
int         MheRelease(mmhe_dev* mdev, mmhe_ctx* mctx);
mmhe_dev*   MheProbe(mmhe_dev* mdev);
int         MheRemove(mmhe_dev* mdev);

#endif //_DRV_MHE_MODULE_H_
