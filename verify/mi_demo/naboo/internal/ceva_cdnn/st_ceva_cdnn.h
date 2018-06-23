#ifndef __ST_CEVA_H__
#define __ST_CEVA_H__

#include "mi_sys_datatype.h"
#include "st_vpe.h"

typedef struct
{
    ST_VPE_PortInfo_t stPortInfo;
    MI_SYS_ChnPort_t stChnPort;
}ST_CEVA_CDNN_PortInfo_t;

typedef struct
{
    MI_U16 u16SrcWidth;
    MI_U16 u16SrcHeight;
    MI_U16 u16DstWidth;
    MI_U16 u16DstHeight;
}ST_CEVA_CDNN_ResolutionMap_t;


MI_S32 ST_CEVA_CDNN_Init(void);
MI_S32 ST_CEVA_CDNN_Deinit();
MI_S32 ST_CEVA_CDNN_Start();
MI_S32 ST_CEVA_CDNN_Stop();
MI_S32 ST_CEVA_CDNN_RegisterChn(ST_CEVA_CDNN_PortInfo_t *pstCdnnPort, ST_CEVA_CDNN_ResolutionMap_t *pstResolutionMap);
MI_S32 ST_CEVA_CDNN_UnRegisterChn(MI_SYS_ChnPort_t stCdnnPort);


#endif

