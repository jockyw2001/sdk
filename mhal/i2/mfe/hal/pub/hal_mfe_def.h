
#ifndef _MFE_DEF_H_
#define _MFE_DEF_H_

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

//#define MMFE_REGS_TRACE

/* macros */
#define MIN(a,b)    ((a)>(b)?(b):(a))
#define MAX(a,b)    ((a)<(b)?(b):(a))

/* debug msg */
#if !defined(DEBUG_LEVEL)
#define DEBUG_LEVEL 0
#endif

#define MFE_MSG_ERR     3
#define MFE_MSG_WARNING 4
#define MFE_MSG_DEBUG   5
#define MFE_MSG_LEVEL    MFE_MSG_ERR //JPE_MSG_WARNING

#define MFE_MSG_ENABLE

#ifdef  MFE_MSG_ENABLE
#define MFE_MSG_FUNC_ENABLE
#define MFE_STRINGIFY(x) #x
#define MFE_TOSTRING(x) MFE_STRINGIFY(x)
#ifdef MFE_MSG_FUNC_ENABLE
#define MFE_MSG_TITLE   "[MFE, %s] "
#define MFE_MSG_FUNC    __func__
#else
#define MFE_MSG_TITLE   "[MFE] %s"
#define MFE_MSG_FUNC    ""
#endif
#endif

#define REGW(base,idx)          (*(((volatile unsigned short*)(base))+2*(idx)))
#if defined(MMFE_REGS_TRACE)
#define PRINT(s,idx,v)          CamOsPrintf(#s":bank%d[%02x]=%04x\n",(idx&0x80)?1:0,idx&0x7F,v)
#define REGWR(base,idx,v,cmt)   do{REGW(base,idx)=(v);PRINT(w,idx,v);}while(0)
#define REGRD(base,idx,cmt)     ({ushort v=REGW(base,idx);PRINT(r,idx,v); v;})
#else
#define REGWR(base,idx,val,cmt) REGW(base,idx)=(val)
#define REGRD(base,idx,cmt)     REGW(base,idx)
#endif

#define MFE_MSG(dbglv, _fmt, _args...)                          \
    do if(dbglv <= MFE_MSG_LEVEL) {                              \
        CamOsPrintf(MFE_TOSTRING(dbglv) MFE_MSG_TITLE  _fmt, MFE_MSG_FUNC, ## _args);   \
    } while(0)

//check division by zero
#define CDBZ(v,cmt)   do if(v==0){CamOsPrintf("[%s %d] Division by zero !!:%s\n",__FUNCTION__,__LINE__,cmt);}while(0)

//check assert
#define assert(v,cmt)   do if((v)==0){CamOsPrintf("[%s %d] assert !!:%s\n",__FUNCTION__,__LINE__,cmt);}while(0)

#define MEM_ALLC(l)         CamOsMemCalloc(1, l)
#define MEM_FREE(p)         CamOsMemRelease(p)
#define MEM_SETV(p,v,l)     memset(p,v,l)
#define MEM_COPY(p,q,l)     memcpy(p,q,l)

#define MMFE_RCTL_PATCH
#define MMFE_IMI_DBF_ADDR  0x00000 /* From 0x00000 to 0x07FFF */
#define MMFE_IMI_LBW_ADDR  0x08000 /* From 0x08000 to 0x283FF */
#define MMFE_IMI_GN_ADDR   0x28400 /* From 0x28400 to 0x303FF */

#endif /*_MFE_DEF_H_*/
