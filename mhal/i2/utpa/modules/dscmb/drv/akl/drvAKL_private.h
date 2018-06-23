#ifndef __AKL_PRIVATE__
#define __AKL_PRIVATE__

#include "drvAKL.h"

#define MAGIC_NUM 0x12345678

typedef enum {
    E_AKL_RESOURCE,
} eAKLResourceId;

typedef struct _AKL_RESOURCE_PRIVATE
{
    MS_BOOL       bInited_Drv;
    MS_U32        u32Magic;
} AKL_RESOURCE_PRIVATE;

typedef struct _AKL_INSTANT_PRIVATE
{

} AKL_INSTANT_PRIVATE;

MS_BOOL _MDrv_AKL_Init(void);
MS_BOOL _MDrv_AKL_Reset(void);
MS_BOOL _MDrv_AKL_SetDbgLevel(AKL_DBGMSG_LEVEL eDbgLevel);
MS_BOOL _MDrv_AKL_SetManualACK(void);
DRV_AKL_RET _MDrv_AKL_SetDMAKey(MS_U32 u32KeyIndex);
DRV_AKL_RET _MDrv_AKL_SetDMAParserKey(MS_U32 u32OddKeyIndex, MS_U32 u32EvenKeyIndex);
DRV_AKL_RET _MDrv_AKL_SetDSCMBKey(MS_U32 u32EngId, MS_U32 u32Index, AKL_Eng_Type eEngType, AKL_Key_Type eKeyType);
DRV_AKL_RET _MDrv_AKL_SetTSIOKey(MS_U32 u32ServiceId, AKL_Key_Type eKeyType);
MS_BOOL _MDrv_AKL_SetManualACKMode(MS_BOOL bEnable);  //K series
DRV_AKL_RET _MDrv_AKL_CERTCmd(AKL_CERT_Cmd eCmd, MS_U8 *pu8Data, MS_U32 u32DataSize);

void AKLRegisterToUtopia(void);
MS_U32 AKLOpen(void** pInstantTmp, MS_U32 u32ModuleVersion, void* pAttribute);
MS_U32 AKLClose(void* pInstantTmp);
MS_U32 AKLIoctl(void* pInstantTmp, MS_U32 u32Cmd, void* pu32Args);


#endif // __AKL_PRIVATE__

