
#ifndef _MHE_DEF_H_
#define _MHE_DEF_H_

#include <linux/string.h>
#include "cam_os_wrapper.h"

typedef unsigned char       uchar;
typedef unsigned short      ushort;
typedef unsigned int        uint;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;
typedef   signed char       int8;
typedef   signed short      int16;
typedef   signed int        int32;
typedef   signed long long  int64;

//#define MMHE_REGS_TRACE

/* macros */
#define MIN(a,b)    ((a)>(b)?(b):(a))
#define MAX(a,b)    ((a)<(b)?(b):(a))

/* debug msg */
#if !defined(DEBUG_LEVEL)
#define DEBUG_LEVEL 0
#endif

#define MHE_MSG_ERR     3
#define MHE_MSG_WARNING 4
#define MHE_MSG_DEBUG   5
#define MHE_MSG_LEVEL    MHE_MSG_WARNING

#define MHE_MSG_ENABLE

#ifdef  MHE_MSG_ENABLE
#define MHE_MSG_FUNC_ENABLE
#define MHE_STRINGIFY(x) #x
#define MHE_TOSTRING(x) MHE_STRINGIFY(x)
#ifdef MHE_MSG_FUNC_ENABLE
#define MHE_MSG_TITLE   "[MHE, %s] "
#define MHE_MSG_FUNC    __func__
#else
#define MHE_MSG_TITLE   "[MHE] %s"
#define MHE_MSG_FUNC    ""
#endif
#endif


#define REGADDR(base,idx)       (((volatile unsigned short*)(base))+2*(idx))
#define REGW(base,idx)          (*(((volatile unsigned short*)(base))+2*(idx)))
#if defined(MMHE_REGS_TRACE)
//#define PRINT(s,idx,v)          CamOsPrintf(#s":bank%d[%02x]=%04x\n",(idx&0x80)?1:0,idx&0x7F,v)
#define PRINT(s,addr,idx,v,cmt)          CamOsPrintf("wriu -w 0x%06x 0x%04x [%02x] %s\n",addr,v,idx,cmt)
#define REGWR(base,idx,v,cmt)   do{REGW(base,idx)=(v);}while(0) /*PRINT(base,REGADDR(base,idx),idx,v,cmt);*/
#define FPGAREGADDR(base,idx)       (((volatile unsigned short*)(base))+(idx))
#define FPGAPRINT(s,addr,idx,v,cmt)          CamOsPrintf("wriu -w 0x%06x 0x%04x%s\n",addr,v,cmt)
#define FPGAREG(base,idx,v,cmt)   do{/*REGW(base,idx)=(v);*/FPGAPRINT(base,FPGAREGADDR(base,idx),idx,v,cmt);}while(0)
#define REGRD(base,idx,cmt)     ({ushort v=REGW(base,idx);/*PRINTR(r,idx,v); */v;})
#else
#define REGWR(base,idx,val,cmt) REGW(base,idx)=(val)
#define REGRD(base,idx,cmt)     REGW(base,idx)
#endif

#define MHE_MSG(dbglv, _fmt, _args...)                          \
    do if(dbglv <= MHE_MSG_LEVEL) {                              \
            CamOsPrintf(MHE_TOSTRING(dbglv) MHE_MSG_TITLE  _fmt, MHE_MSG_FUNC, ## _args);   \
        } while(0)

//check division by zero
#define CDBZ(v,cmt)   do if(v==0){MHE_MSG(MHE_MSG_ERR,"[%s %d] Division by zero !!:%s\n",__FUNCTION__,__LINE__,cmt);}while(0)

//check assert
#define assert(v,cmt)   do if((v)==0){MHE_MSG(MHE_MSG_ERR,"[%s %d] assert !!:%s\n",__FUNCTION__,__LINE__,cmt);}while(0)

#define MEM_ALLC(l)         CamOsMemCalloc(1, l)
#define MEM_FREE(p)         CamOsMemRelease(p)
#define MEM_SETV(p,v,l)     memset(p,v,l)
#define MEM_COPY(p,q,l)     memcpy(p,q,l)

//IMI Buffer
#define MMHE_IMI_MAX_WIDTH 2596
#define MMHE_IMI_BUF_ADDR 0x030400  /* From 0x30400 to 0x95DFF, Size 0x65A00, MaxWidth 2596 */
//#define MMHE_IMI_BUF_ADDR 0x000000

#endif /*_MHE_DEF_H_*/
