#ifndef _APISWI2C_PRIV_H_
#define _APISWI2C_PRIV_H_


#include "MsTypes.h"
#include "apiSWI2C.h"


typedef enum {
    MApi_CMD_SWI2C_WriteBytes,
    MApi_CMD_SWI2C_ReadBytes,
} eSWI2CIoctlOpt;

typedef struct _SWI2C_WRITEBYTES_PARAM
{
    MS_U16 u16BusNumSlaveID;
    MS_U8 AddrCnt;
    MS_U8* pu8addr;
    MS_U16 u16size;
    MS_U8* pBuf;
}SWI2C_WRITEBYTES_PARAM, *PSWI2C_WRITEBYTES_PARAM;

typedef struct _SWI2C_READBYTES_PARAM
{
    MS_U16 u16BusNumSlaveID;
    MS_U8 ucSubAdr;
    MS_U8* paddr;
    MS_U16 ucBufLen;
    MS_U8* pBuf;
}SWI2C_READBYTES_PARAM, *PSWI2C_READBYTES_PARAM;

//ioctl function pointer type
typedef MS_BOOL (*IOCTL_SWI2C_WRITEBYTES)(MS_U16, MS_U8, MS_U8*, MS_U16, MS_U8*);
typedef MS_BOOL (*IOCTL_SWI2C_READBYTES)(MS_U16, MS_U8, MS_U8*, MS_U16, MS_U8*);

// INSTANCE Private(ioctl function pointer)
typedef struct _SWI2C_INSTANT_PRIVATE
{
    IOCTL_SWI2C_WRITEBYTES       fpSWI2CWriteBytes;
    IOCTL_SWI2C_READBYTES      fpSWI2CReadBytes;
}SWI2C_INSTANT_PRIVATE;

//////////////////////////////////////
// function for utopia2.0 ////////////
//////////////////////////////////////
void SWI2CRegisterToUtopia(void);
MS_U32 SWI2COpen(void** pInstance, MS_U32 u32ModuleVersion, void* pAttribute);
MS_U32 SWI2CClose(void* pInstance);
MS_U32 SWI2CIoctl(void* pInstance, MS_U32 u32Cmd, void *pArgs);

// Resource_Private
typedef struct _SWI2C_RESOURCE_PRIVATE
{
    MS_U32 Dummy;
}SWI2C_RESOURCE_PRIVATE;

#endif
