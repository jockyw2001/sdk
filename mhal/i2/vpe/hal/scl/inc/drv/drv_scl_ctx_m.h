////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef __DRV_SC_CTX_M_H__
#define __DRV_SC_CTX_M_H__

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define MDRV_SCL_CTX_INSTANT_MAX        (64)  // MAX instant for 32 input
#define MDRV_SCL_CTX_ID_DEFAULT         (MDrvSclCtxGetDefaultCtxId())
//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MDRV_SCL_CTX_ID_SC_ALL,
    E_MDRV_SCL_CTX_ID_SC_VIP,        //for vip/iq
    //E_MDRV_SCL_CTX_ID_M2M,           //ToDo:multiinst for efficiency
    //E_MDRV_SCL_CTX_ID_SC_GEN,       //ToDo:multiinst for efficiency
    E_MDRV_SCL_CTX_ID_NUM,
}MDrvSclCtxIdType_e;

typedef enum
{
    E_MDRV_SCL_CTX_MLOAD_ID_BASE = 33,
    E_MDRV_SCL_CTX_MLOAD_ID_HSP_C_SC1 = 33,
    E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC1 = 34,
    E_MDRV_SCL_CTX_MLOAD_ID_VSP_C_SC1 = 35,
    E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC1 = 36,
    E_MDRV_SCL_CTX_MLOAD_ID_IHC_LUT = 37,
    E_MDRV_SCL_CTX_MLOAD_ID_ICC_LUT = 38,
    E_MDRV_SCL_CTX_MLOAD_ID_YUV_GAMMA_Y = 39,
    E_MDRV_SCL_CTX_MLOAD_ID_YUV_GAMMA_UV = 40,
    E_MDRV_SCL_CTX_MLOAD_ID_RGB12TO12_PRE = 41,
    E_MDRV_SCL_CTX_MLOAD_ID_RGB12TO12_POST = 42,
    E_MDRV_SCL_CTX_MLOAD_ID_WDR = 43,
    E_MDRV_SCL_CTX_MLOAD_ID_HSP_C_SC2 = 49,
    E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC2 = 50,
    E_MDRV_SCL_CTX_MLOAD_ID_VSP_C_SC2 = 51,
    E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC2 = 52,
    E_MDRV_SCL_CTX_MLOAD_ID_HSP_C_SC3 = 53,
    E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC3 = 54,
    E_MDRV_SCL_CTX_MLOAD_ID_VSP_C_SC3 = 55,
    E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC3 = 56,
    E_MDRV_SCL_CTX_MLOAD_ID_HSP_C_SC4 = 57,
    E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC4 = 58,
    E_MDRV_SCL_CTX_MLOAD_ID_VSP_C_SC4 = 59,
    E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC4 = 60,
    E_MDRV_SCL_CTX_MLOAD_ID_NUM = 61

}MDrvSclCtxMloadID_e;


typedef enum
{
    //E_MDRV_SCL_CTX_CMDBUF_OPR = 0,
    E_MDRV_SCL_CTX_CMDBUF_SRAMWDRTBL,
    E_MDRV_SCL_CTX_CMDBUF_NUM,
    E_MDRV_SCL_CTX_CMDBUF_SRAMWDR,
    E_MDRV_SCL_CTX_CMDBUF_SRAMGAMMAY,
    E_MDRV_SCL_CTX_CMDBUF_SRAMGAMMAUV,
    E_MDRV_SCL_CTX_CMDBUF_SRAMA2CR,
    E_MDRV_SCL_CTX_CMDBUF_SRAMA2CG,
    E_MDRV_SCL_CTX_CMDBUF_SRAMA2CB,
    E_MDRV_SCL_CTX_CMDBUF_SRAMC2AR,
    E_MDRV_SCL_CTX_CMDBUF_SRAMC2AG,
    E_MDRV_SCL_CTX_CMDBUF_SRAMC2AB,
    E_MDRV_SCL_CTX_CMDBUF_SRAMHSP1,
    E_MDRV_SCL_CTX_CMDBUF_SRAMHSP2,
    E_MDRV_SCL_CTX_CMDBUF_SRAMHSP3,
    E_MDRV_SCL_CTX_CMDBUF_SRAMVSP1,
    E_MDRV_SCL_CTX_CMDBUF_SRAMVSP2,
    E_MDRV_SCL_CTX_CMDBUF_SRAMVSP3,
}MDrvSclCtxCMDBufType_e;
typedef struct
{
    void *pCmdqVirAddr;     // cmdq vir addr
    u8  u8IdNum;            //channel num
    void *pVipSt;           // VIP/AIP struct
}MDrvSclCtxIspInterface_t;

typedef struct
{
    u32 u32Reg;
    u16 u16Val;
    u16 u32Mask;
}MDrvSclCtxpInquireTbl_t;
typedef struct
{
    void *pValueIdxTblVir;  //**Regtbl
    void *pInquireTbl;      //* RegQueue
    void *pSRAMTbl[E_MDRV_SCL_CTX_CMDBUF_NUM];
    void *pgstGlobalSet;
    void *pvMloadHandler;
    u32 u32TblCnt;
    u32 u32SRAMTblCnt[E_MDRV_SCL_CTX_CMDBUF_NUM];
    MDrvSclCtxIdType_e enCtxId;
}MDrvSclCtxCmdqConfig_t;

typedef struct
{
    u8  u8IdNum;
    s32 s32Id[MDRV_SCL_CTX_CLIENT_ID_MAX]; // for sclhvsp, scldma, sclvip
    bool bUsed;
    MDrvSclCtxCmdqConfig_t stCtx;
}MDrvSclCtxConfig_t;


typedef struct
{
    u8 u8IdNum;
    s32 s32Id[MDRV_SCL_CTX_CLIENT_ID_MAX];
    bool bLock;
    s32 s32Mutex;
    s32 s32Sem;
}MDrvSclCtxLockConfig_t;

typedef struct
{
    s32 *ps32IdBuf;         ///< buffer ID
    u8 u8IdNum;           ///< device count
}MDrvSclCtxInstConfig_t;
typedef struct
{
    u32 u32McnrSize;
    DrvSclOsDmemBusType_t  PhyMcnrYCMAddr;
    void *pvMcnrYCMVirAddr;
    char u8McnrYCMName[16];
    u32 u32McnrReleaseSize;
}MDrvSclCtxNrBufferGlobalSet_t;

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifndef __DRV_SCL_CTX_M_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE bool                      MDrvSclCtxInit(void);
INTERFACE MDrvSclCtxIdType_e MDrvSclCtxGetDefaultCtxId(void);
INTERFACE void MDrvSclCtxResetWdrTblCnt(void);
INTERFACE bool                      MDrvSclCtxSetWaitMload(void);
INTERFACE bool MDrvSclCtxSetMload(MDrvSclCtxMloadID_e enId,MDrvSclCtxCmdqConfig_t *pCtx);
INTERFACE bool MDrvSclCtxFireMload(bool bFire,MDrvSclCtxCmdqConfig_t *pCtx);
INTERFACE bool MDrvSclCtxTriggerM2M(bool bFire,MDrvSclCtxCmdqConfig_t *pCtx);
INTERFACE bool                      MDrvSclCtxFire(MDrvSclCtxCmdqConfig_t *pCtx);
INTERFACE bool MDrvSclCtxFireM2M(MDrvSclCtxCmdqConfig_t *pCtx);
INTERFACE bool                      MDrvSclCtxDeInit(void);
INTERFACE MDrvSclCtxConfig_t        *MDrvSclCtxAllocate(MDrvSclCtxIdType_e enCtxId, MDrvSclCtxInstConfig_t *stCtxInst);
INTERFACE bool                      MDrvSclCtxFree(MDrvSclCtxConfig_t *pCtxCfg);
INTERFACE bool                      MDrvSclCtxSwapConfigCtx(MDrvSclCtxConfig_t *pCtxCfg,MDrvSclCtxIdType_e enCtxId);
INTERFACE MDrvSclCtxCmdqConfig_t    *MDrvSclCtxGetConfigCtx(MDrvSclCtxIdType_e enCtxId);
INTERFACE MDrvSclCtxLockConfig_t    *MDrvSclCtxGetLockConfig(MDrvSclCtxIdType_e enCtxId);
INTERFACE bool                      MDrvSclCtxIspAddNewFrame(MDrvSclCtxIspInterface_t *pstCfg);
INTERFACE bool MDrvSclCtxSetLockConfig(s32 s32Handler,MDrvSclCtxIdType_e enCtxId);
INTERFACE bool MDrvSclCtxSetUnlockConfig(s32 s32Handler,MDrvSclCtxIdType_e enCtxId);
INTERFACE MDrvSclCtxConfig_t *MDrvSclCtxGetDefaultCtx(void);
INTERFACE MDrvSclCtxConfig_t *MDrvSclCtxGetInstByHandler(s32 s32Handler,MDrvSclCtxIdType_e enCtxId);
INTERFACE void MDrvSclCtxKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq);
INTERFACE void MDrvSclCtxKeepM2MCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq);
INTERFACE MDrvSclCtxConfig_t *MDrvSclCtxGetCtx(u16 u16inst);
INTERFACE void MDrvSclCtxDumpSetting(void);
INTERFACE void MDrvSclCtxDumpRegSetting(u32 u32RegAddr,bool bAllbank);

#undef INTERFACE

#endif
