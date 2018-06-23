#ifndef BUFPTR_H
#define BUFPTR_H

typedef struct BufPtr_t
{
//private
    u64 pBuf;
    int  nRefCount;
    void (*_Free)(struct BufPtr_t *pThis);
//public
    void (*Release)(struct BufPtr_t *pThis);
    void (*AddRef)(struct BufPtr_t *pThis);
}BufPtr_t;

//static void BufPtrRelease(struct BufPtr_t* pThis);
//static void BufPtrAddRef(struct BufPtr_t* pThis);
BufPtr_t* BufPtrInit(u64 pBuf,void (*fpFree)(struct BufPtr_t* pBuf));
#define BufPtr(a) ((a)->pBuf)

#endif
