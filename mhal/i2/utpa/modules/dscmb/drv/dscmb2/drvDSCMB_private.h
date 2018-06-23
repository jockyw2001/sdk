#ifndef __DSCMB_PRIVATE__
#define __DSCMB_PRIVATE__

#include "halDSCMB.h"

#define MAGIC_NUM 0x12345678

typedef enum {
    E_DSCMB_RESOURCE,
} eDscmbResourceId;

typedef struct
{
    MS_U32 u32SlotIdOdd;
    MS_U32 u32SlotIdEven;
    MS_U32 u32SlotIdClear;
    MS_U32 u32PidFltIdNum;
    MS_BOOL bUsed;
    MS_BOOL bDscmb;
    MS_U32 u32Tsid;
    DSCMB_Flt_Type eFltType;
    MS_U32 u32CAVid;
    MS_U32 u32RIVIdx;
    MS_BOOL bSecure;
} stDscmbTspMap;

typedef struct
{
    MS_U32 u8DscmbId;
    MS_U8 u8TsId;
    MS_U8 u8CaVid;
    MS_U8 u8CaDestUpp;
    MS_U8 u8CaDestLow;
} stTspFltInfo;

typedef struct _DSCMB_RESOURCE_PRIVATE
{
    stTspFltInfo  u32PidFlt2Dscmb[HAL_DSCMB_PIDFLT_NUM];
    stDscmbTspMap DscmbTspMap[HAL_DSCMB_ENG_NUM][HAL_DSCMB_KTE_MAX];
    MS_BOOL       bFreeSlot[HAL_DSCMB_KTE_MAX];
    MS_BOOL       bFreeRIVSlot[HAL_DSCMB_RIV_MAX];

    MS_U32        u32Cavid;
    MS_BOOL       bInited_Drv;
    MS_U32        u32Magic;
    SYS_TEEINFO   stTeeInfo;
} DSCMB_RESOURCE_PRIVATE;


typedef struct _DSCMB_INSTANT_PRIVATE
{

} DSCMB_INSTANT_PRIVATE;

typedef struct DLL_PACKED _DSCMB_TEE_KL_CFG
{
    MS_U32    u32Algo;
    MS_U32    u32Src;
    MS_U32    u32Dst;
    MS_U32    u32Outsize;
    MS_U32    u32KeyType;
    MS_U32    u32Level;
    MS_U32    u32EngID;
    MS_U32    u32DscID;
    MS_U32    u32KeyACPU;
    MS_U32    u32KeyKLIn;
    MS_BOOL   bDecrypt;
    MS_BOOL   bInverse;
    MS_U16    u16Reserved_1;
    MS_U32    u32KLSel;
    MS_U32    u32CAVid;
    MS_U16    u16AppId;
    MS_U16    u16Reserved_2;
    MS_U32    u32KDFType;
    MS_U32    u32HWKeyId;
    MS_U32    u32FSCB;
} DSCMB_TEE_KL_CFG;

typedef enum {
    /*Descrambler basic commad*/
    E_TEE_CMD_DSCMB_FltAlloc = 0x10,
    E_TEE_CMD_DSCMB_FltFree,
    E_TEE_CMD_DSCMB_FltConnectFltId,
    E_TEE_CMD_DSCMB_FltDisconnectFltId,
    E_TEE_CMD_DSCMB_FltTypeSet,
    E_TEE_CMD_DSCMB_FltKeySet,
    E_TEE_CMD_DSCMB_FltKeyReset,
    E_TEE_CMD_DSCMB_FltIVSet,

    /*Descrambler advanced commad*/
    E_TEE_CMD_DSCMB_EngSetAlgo = 0x20,
    E_TEE_CMD_DSCMB_EngSetKey,
    E_TEE_CMD_DSCMB_EngResetKey,
    E_TEE_CMD_DSCMB_EngSetSwitch,
    E_TEE_CMD_DSCMB_EngSetFSCB,
    E_TEE_CMD_DSCMB_EngSetKeyFSCB,
    E_TEE_CMD_DSCMB_EngSetIV_Ex,
    E_TEE_CMD_DSCMB_EngSetRIV,
    E_TEE_CMD_DSCMB_EngEnableKey,

    /*Descrambler global HW setting commad*/
    E_TEE_CMD_DSCMB_SetDefaultCAVid = 0x30,
    E_TEE_CMD_DSCMB_Multi2_SetRound,
    E_TEE_CMD_DSCMB_Multi2_SetSysKey,
    E_TEE_CMD_DSCMB_HDCP2_SetRiv,

    /*Switch descrambler dual path for payload encryption*/
    E_TEE_CMD_DSCMB_CAPVR_FlowSet = 0x40,
    E_TEE_CMD_DSCMB_DualPath_Enable,
    E_TEE_CMD_DSCMB_DualPath_Disable,

    /*Keyladder command*/
    E_TEE_CMD_DSCMB_KLadder_AtomicExec = 0x50,
    E_TEE_CMD_DSCMB_KLadder_ETSI,
} DSCMB_TEE_CMD;

// descrambler initial function
MS_BOOL _MDrv_DSCMB2_Init(void);
MS_BOOL _MDrv_DSCMB2_InitBySWFlag(MS_BOOL bSWInit);
MS_BOOL _MDrv_DSCMB2_Exit(void);

// descrambler filter management
MS_U32  _MDrv_DSCMB2_FltAlloc_Ex(MS_U32 u32EngId, DSCMB_Flt_Type eFltType);
MS_BOOL _MDrv_DSCMB2_FltFree(MS_U32 u32EngId, MS_U32 u32DscmbId);

// descrambler filter maps to TSP pid
#if !(defined(MSOS_TYPE_OPTEE) || defined(MSOS_TYPE_NUTTX))
MS_BOOL _MDrv_DSCMB2_FltConnectPid(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_U32 u32Pid);
MS_BOOL _MDrv_DSCMB2_FltDisconnectPid_Ex(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_U32 u32Pid);
#endif

// descrambler filter maps to TSP filter id
MS_BOOL _MDrv_DSCMB2_FltConnectFltId(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_U32 u32FltId);
MS_BOOL _MDrv_DSCMB2_FltDisconnectFltId(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_U32 u32FltId);

MS_BOOL _MDrv_DSCMB2_SetDefaultCAVid(MS_U32 u32EngId, MS_U32 u32CAVid);
MS_BOOL _MDrv_DSCMB2_EngSetAlgo(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Algo_Cfg stConfig);
MS_BOOL _MDrv_DSCMB2_EngSetKeyFSCB(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Key_Type eKeyType, DSCMB_FSCB eForceSCB);
MS_BOOL _MDrv_DSCMB2_EngSetKey(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Key_Type eKeyType, MS_U8* pu8Key);
MS_BOOL _MDrv_DSCMB2_EngResetKey(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Key_Type eKeyType);
MS_BOOL _MDrv_DSCMB2_EngSetIV(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Key_Type eKeyType, MS_U8* pu8IV);
MS_BOOL _MDrv_DSCMB2_EngSetIV_Ex(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Key_Type eKeyType, MS_U8* pu8IV);
MS_BOOL _MDrv_DSCMB2_EngSetFSCB(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_FSCB eForceSCB );
MS_BOOL _MDrv_DSCMB2_EngSetSwitch(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eUppSwitch ,DSCMB_Eng_Type eLowSwitch);

// descrambler for enable dual path
MS_BOOL _MDrv_DSCMB2_CAPVR_FlowSet(MS_U32 u32EngId, DSCMB_CAPVR_MODE eCaMode, DSCMB_TSIF ePvrSrcTSIf);
MS_BOOL _MDrv_DSCMB2_DualPath_Enable(MS_U32 u32EngId, MS_U32 u32FltId);
MS_BOOL _MDrv_DSCMB2_DualPath_Disable(MS_U32 u32EngId, MS_U32 u32FltId);

// Set descrambler filter as scrambling/decrambling mode
// Default filter mode is descrambling if this function is not specified
// This function should be invoked before MDrv_DSCMB_FltTypeSet
MS_BOOL _MDrv_DSCMB2_FltDscmb(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_BOOL bDscmb);
MS_BOOL _MDrv_DSCMB2_FltTypeSet(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Type DscmbType);
MS_BOOL _MDrv_DSCMB2_FltKeySet(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Key_Type eKeyType, MS_U8* pu8Key);
MS_BOOL _MDrv_DSCMB2_FltKeyReset(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Key_Type eKeyType);
MS_BOOL _MDrv_DSCMB2_FltIVSet(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Key_Type eKeyType, MS_U8* pu8IV);


// descrambler query function
MS_BOOL _MDrv_DSCMB2_GetLibVer(const MSIF_Version **ppVersion);
MS_BOOL _MDrv_DSCMB2_SetDBGLevel(MS_U32 u32Level);
MS_BOOL _MDrv_DSCMB2_GetCap(MS_U32 u32EngId, DSCMB_Query_Type eQueryType, void* pInput, void* pOutput);
MS_BOOL _MDrv_DSCMB2_GetConnectStatus(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_U32 u32TspFltId);

// descrambler altorithm related
MS_BOOL _MDrv_DSCMB2_Multi2_SetRound(MS_U32 u32EngId, MS_U32 u32Round);
MS_BOOL _MDrv_DSCMB2_Multi2_SetSystemKey(MS_U32 u32EngId, MS_U8* pu8Syskey);
MS_BOOL _MDrv_DSCMB2_ES_SetStaticKey(MS_U32 u32EngId, MS_U8* pu8Statickey);
MS_BOOL _MDrv_DSCMB2_HDCP2_SetRIV(MS_U32 u32EngId, MS_U8* pu8RIV);
MS_BOOL _MDrv_DSCMB2_HDCP2_SetRIV_Ex(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_U8* pu8RIV);

MS_BOOL _MDrv_DSCMB2_PVR_RecCtrl(MS_U32 u32EngId, MS_BOOL Enable);
MS_U32  _MDrv_DSCMB2_SetPowerState(EN_POWER_MODE u16PowerState);
MS_BOOL _MDrv_DSCMB2_PidFlt_ScmbStatus(MS_U32 u32EngId, MS_U32 u32PidFltId, SCMB_Level* pScmbLevel);


// key ladder
MS_BOOL _MDrv_DSCMB2_KLadder_AtomicExec(DSCMB_KLCfg_All* KLCfg , MS_U8 *ACPU_Out, DSCMB_KL_Status* u32Status );
MS_BOOL _MDrv_DSCMB2_KLadder_ETSI(DSCMB_KLCfg_All* KLCfg , MS_U8 *ACPU_Out, MS_U8 *pu8Nonce, MS_U8 *pu8Response, DSCMB_KL_Status* u32Status);

// TEE Function
MS_BOOL _MDrv_DSCMB2_TEE_Open(void);
MS_BOOL _MDrv_DSCMB2_TEE_Close(void);
MS_BOOL _MDrv_DSCMB2_TEE_Send_CMD(DSCMB_TEE_CMD eCMD,void* para,MS_U32 size);
MS_U32  _MDrv_DSCMB2_TEE_FltAlloc_Ex(MS_U32 u32EngId, DSCMB_Flt_Type eFltType);
MS_BOOL _MDrv_DSCMB2_TEE_FltFree(MS_U32 u32EngId, MS_U32 u32DscmbId);
MS_BOOL _MDrv_DSCMB2_TEE_FltConnectFltId(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_U32 u32FltId);
MS_BOOL _MDrv_DSCMB2_TEE_FltDisconnectFltId(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_U32 u32FltId);
MS_BOOL _MDrv_DSCMB2_TEE_SetDefaultCAVid(MS_U32 u32EngId, MS_U32 u32CAVid);
MS_U32  _MDrv_DSCMB2_TEE_GetPidSlotMapIndex(MS_U32 u32EngId, MS_U32 u32DscmbId);
MS_BOOL _MDrv_DSCMB2_TEE_EngSetAlgo(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Algo_Cfg stConfig);
MS_BOOL _MDrv_DSCMB2_TEE_EngSetKeyFSCB(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Key_Type eKeyType, DSCMB_FSCB eForceSCB);
MS_BOOL _MDrv_DSCMB2_TEE_EngSetKey(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Key_Type eKeyType, MS_U8* pu8Key);
MS_BOOL _MDrv_DSCMB2_TEE_EngResetKey(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Key_Type eKeyType);
MS_BOOL _MDrv_DSCMB2_TEE_EngEnableKey(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Key_Type eKeyType, MS_BOOL bEnable);
MS_BOOL _MDrv_DSCMB2_TEE_EngSetIV(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Key_Type eKeyType, MS_U8* pu8IV);
MS_BOOL _MDrv_DSCMB2_TEE_EngSetIV_Ex(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eEngType, DSCMB_Key_Type eKeyType, MS_U8* pu8IV);
MS_BOOL _MDrv_DSCMB2_TEE_EngSetRIV(MS_U32 u32EngId, MS_U32 u32DscmbId, MS_U8* pu8RIV);
MS_BOOL _MDrv_DSCMB2_TEE_EngSetFSCB(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_FSCB eForceSCB );
MS_BOOL _MDrv_DSCMB2_TEE_EngSetSwitch(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Eng_Type eUppSwitch ,DSCMB_Eng_Type eLowSwitch);
MS_BOOL _MDrv_DSCMB2_TEE_CAPVR_FlowSet(MS_U32 u32EngId, DSCMB_CAPVR_MODE eCaMode, DSCMB_TSIF ePvrSrcTSIf);
MS_BOOL _MDrv_DSCMB2_TEE_DualPath_Enable(MS_U32 u32EngId, MS_U32 u32FltId);
MS_BOOL _MDrv_DSCMB2_TEE_DualPath_Disable(MS_U32 u32EngId, MS_U32 u32FltId);
MS_BOOL _MDrv_DSCMB2_TEE_FltTypeSet(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Type DscmbType);
MS_BOOL _MDrv_DSCMB2_TEE_FltKeySet(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Key_Type eKeyType, MS_U8* pu8Key);
MS_BOOL _MDrv_DSCMB2_TEE_FltKeyReset(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Key_Type eKeyType);
MS_BOOL _MDrv_DSCMB2_TEE_FltIVSet(MS_U32 u32EngId, MS_U32 u32DscmbId, DSCMB_Key_Type eKeyType, MS_U8* pu8IV);
MS_BOOL _MDrv_DSCMB2_TEE_Multi2_SetRound(MS_U32 u32EngId, MS_U32 u32Round);
MS_BOOL _MDrv_DSCMB2_TEE_Multi2_SetSystemKey(MS_U32 u32EngId, MS_U8* pu8Syskey);
MS_BOOL _MDrv_DSCMB2_TEE_KLadder_AtomicExec(DSCMB_KLCfg_All* KLCfg , MS_U8 *ACPU_Out, DSCMB_KL_Status* u32Status );
MS_BOOL _MDrv_DSCMB2_TEE_KLadder_ETSI(DSCMB_KLCfg_All* KLCfg , MS_U8 *ACPU_Out, MS_U8 *pu8Nonce, MS_U8 *pu8Response, DSCMB_KL_Status* u32Status);

#endif // __DSCMB_PRIVATE__

