#ifndef __DRV_DIVP_H__
#define __DRV_DIVP_H__

#include "mhal_divp_datatype.h"
#include "mhal_dip.h"

#define MHAL_SUCCESS    (0)
#define MHAL_FAILURE    (-1)

typedef struct
{
    MS_U8     u8BufNum;
    MS_U32    u32BaseAddr;
    MS_U32    u32BufSize;
    MS_U32    u32DISize;// = u32DIPDISize;
    MS_U8     u8WrInx;
    MS_U8     u8RdInx;
} MHAL_DIVP_3ddiInfo_t;

typedef struct
{
    MHAL_DIVP_TnrLevel_e eTnrLevel;//TNR level
    MHAL_DIVP_DiType_e eDiType;//DI type
    MHAL_DIVP_Rotate_e eRotateType;//rotate angle
    MHAL_DIVP_Window_t stCropWin;//crop information
    MHAL_DIVP_Mirror_t stMirror;
} MHAL_DIVP_AttrParams_t;

typedef struct
{
    MS_BOOL bInit;
    MHAL_DIVP_DeviceId_e eId;
    MS_U8 u8ChannelId;
    MS_U16 nMaxWidth;
    MS_U16 nMaxHeight;
    MHAL_DIVP_AttrParams_t stAttr;
    MHAL_DIVP_3ddiInfo_t st3dDiInfo;
    MS_U64 u64AllocAddr;
    MS_U32 u32AllocSize;
    PfnAlloc pfAlloc;
    PfnFree pfFree;
    MS_BOOL bIsDiEnable;
    MS_U32 u32DiFrameCnt;
    char  pCtxString[32];
    MS_U32 u32PreFrameCnt;
    MS_U32 u32CurFrameCnt;
    MS_U32 u64PreTime;
    MS_U32 fps;
    MS_U32 time_elapsed;
    MHAL_DIVP_InputInfo_t pstInputInfo;
    MHAL_DIVP_OutPutInfo_t pstOutputInfo;
} __attribute__ ((__packed__)) MHAL_DIVP_InstPrivate_t;

typedef struct
{
    MS_U8 u8NumOpening;
    SCALER_DIP_WIN enDipWin;
    MS_BOOL bIsInit;
    //di buffer info
    MS_U8 u8NwayMaxBufCnt;
    ST_DIP_DI_SETTING st3DDISetting;
    ST_XC_DIPR_PROPERTY_EX stDiprInfo;
    PfnAlloc pfAlloc;
    PfnFree pfFree;
    MS_U8 u8FinalChannelId;
    MHAL_DIVP_InstPrivate_t *ctx[MAX_CHANNEL_NUM];
} __attribute__ ((__packed__)) DivpPrivateDevData;

#endif
