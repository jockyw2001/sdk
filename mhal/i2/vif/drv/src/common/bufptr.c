#include "cam_os_wrapper.h"
#include <linux/kernel.h>
#include <bufptr.h>

static void BufPtrRelease(struct BufPtr_t* pThis)
{
    //debug
    if(!pThis->pBuf || pThis->nRefCount<0)
    {
        pr_err("%s double free\n",__FUNCTION__);
        return;
    }

    if( --(pThis->nRefCount) == 0)
    {
        pThis->_Free(pThis);
        CamOsMemRelease(pThis);
        //pThis->pBuf = 0;//set buffer pointer to NULL
    }
}

static void BufPtrAddRef(struct BufPtr_t* pThis)
{
    pThis->nRefCount++;
}

BufPtr_t* BufPtrInit(u64 pBuf,void (*fpFree)(struct BufPtr_t* pBuf))
{
    BufPtr_t *p = CamOsMemAlloc(sizeof(BufPtr_t));
    memset(p,0,sizeof(BufPtr_t));
    p->nRefCount = 1;
    p->pBuf = pBuf;
    p->_Free = fpFree;
    p->Release = BufPtrRelease;
    p->AddRef = BufPtrAddRef;
    return p;
}
