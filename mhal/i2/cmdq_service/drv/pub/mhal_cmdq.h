#ifndef __MHAL_CMD_SERVICE_HH__
#define __MHAL_CMD_SERVICE_HH__

#define MHAL_CMDQ_POLLNEQ_TIMEOUT        (0x1<<8)
#define MHAL_CMDQ_POLLEQ_TIMEOUT         (0x1<<9)
#define MHAL_CMDQ_WAIT_TIMEOUT           (0x1<<10)
#define MHAL_CMDQ_WRITE_TIMEOUT          (0x1<<12)

#define MHAL_CMDQ_ERROR_STATUS           (MHAL_CMDQ_POLLNEQ_TIMEOUT|MHAL_CMDQ_POLLEQ_TIMEOUT |MHAL_CMDQ_WAIT_TIMEOUT|MHAL_CMDQ_WRITE_TIMEOUT)

typedef enum
{
    E_MHAL_CMDQEVE_S0_MDW_W_DONE     =     0x01    ,         //Only for cmdq1&cmdq5
    E_MHAL_CMDQEVE_S0_MGW_FIRE       =     0x02    ,         //Only for cmdq1&cmdq5
    E_MHAL_CMDQEVE_S1_MDW_W_DONE     =     0x04    ,         //Only for cmdq2&cmdq4
    E_MHAL_CMDQEVE_S1_MGW_FIRE       =     0x08    ,         //Only for cmdq2&cmdq4
    E_MHAL_CMDQEVE_DMAGEN_TRIGGER0   =     0x10    ,         //Only for cmdq2&cmdq4
    E_MHAL_CMDQEVE_DMAGEN_TRIGGER1   =     0x20    ,         //Only for cmdq2&cmdq4
    E_MHAL_CMDQEVE_BDMA_TRIGGER0     =     0x40    ,         //Only for cmdq3&cmdq5
    E_MHAL_CMDQEVE_BDMA_TRIGGER1     =     0x80    ,         //Only for cmdq3
    E_MHAL_CMDQEVE_IVE_CMDQ_TRIG     =     0x100   ,         //Only for cmdq3&cmdq5
    E_MHAL_CMDQEVE_LDC_CMDQ_TRIG     =     0x200   ,         //Only for cmdq1&cmdq3
    E_MHAL_CMDQEVE_GE_CMDQ_TRIG      =     0x400   ,         //Only for cmdq1&cmdq3
    E_MHAL_CMDQEVE_REG_DUMMY_TRIG    =     0x800   ,         //Only for cmdq1&cmdq2&cmdq4&cmdq5
    E_MHAL_CMDQEVE_CORE1_MHE_TRIG    =     0x1000  ,         //Only for ALL
    E_MHAL_CMDQEVE_CORE0_MHE_TRIG    =     0x2000  ,         //Only for ALL
    E_MHAL_CMDQEVE_CORE1_MFE_TRIG    =     0x4000  ,         //Only for ALL
    E_MHAL_CMDQEVE_CORE0_MFE_TRIG    =     0x8000  ,         //Only for ALL
    E_MHAL_CMDQEVE_DIP_TRIG          =     0x10000 ,         //Only for ALL
    E_MHAL_CMDQEVE_GOP_TRIG4         =     0x20000 ,         //Only for ALL
    E_MHAL_CMDQEVE_GOP_TRIG2         =     0x40000 ,         //Only for ALL
    E_MHAL_CMDQEVE_GOP_TRIG013       =     0x80000 ,         //Only for ALL
    E_MHAL_CMDQEVE_SC_TRIG2          =     0x100000 ,         //Only for ALL
    E_MHAL_CMDQEVE_SC_TRIG013        =     0x200000 ,         //Only for ALL
    E_MHAL_CMDQEVE_ISP_TRIG          =     0x400000 ,         //Only for ALL
} MHAL_CMDQ_EventId_e;

typedef enum
{
    E_MHAL_CMDQ_ID_VPE = 0,
    E_MHAL_CMDQ_ID_DIVP,
    E_MHAL_CMDQ_ID_H265_VENC0,
    E_MHAL_CMDQ_ID_H265_VENC1,
    E_MHAL_CMDQ_ID_H264_VENC0,
    E_MHAL_CMDQ_ID_MAX
} MHAL_CMDQ_Id_e;

typedef struct MHAL_CMDQ_BufDescript_s
{
    MS_U32 u32CmdqBufSize;
    MS_U16 u32CmdqBufSizeAlign;
    MS_U32 u32MloadBufSize;
    MS_U16 u16MloadBufSizeAlign;
} MHAL_CMDQ_BufDescript_t;

typedef struct MHAL_CMDQ_MultiCmdBuf_s
{
    MS_U32 u32RegAddr;
    MS_U32 u32RegValue;
} MHAL_CMDQ_MultiCmdBuf_t;

typedef struct MHAL_CMDQ_MultiCmdBufMask_s
{
    MS_U32 u32RegAddr;
    MS_U16 u16RegValue;
    MS_U16 u16Mask;
} MHAL_CMDQ_MultiCmdBufMask_t;

typedef struct MHAL_CMDQ_Mmap_Info_s
{
    MS_U8     u8CmdqMmapGid;                         // Mmap ID
    MS_U8     u8CmdqMmapLayer;                       // Memory Layer
    MS_U8     u8CmdqMmapMiuNo;                       // 0: MIU0 / 1: MIU1 / 2: MIU2
    MS_U8     u8CmdqMmapCMAHid;                      // Memory CMAHID
    MS_U32    u32CmdqMmapPhyAddr;                       // phy Memory Address
    MS_U32    u32CmdqMmapVirAddr;                       // vir Memory Address
    MS_U32    u32CmdqMmapSize;                       // Memory Size
    MS_U32    u32CmdqMmapAlign;                      // Memory Align
    MS_U32    u32CmdqMmapMemoryType;

    MS_U8     u8MloadMmapGid;                         // Mmap ID
    MS_U8     u8MloadMmapLayer;                       // Memory Layer
    MS_U8     u8MloadMmapMiuNo;                       // 0: MIU0 / 1: MIU1 / 2: MIU2
    MS_U8     u8MloadMmapCMAHid;                      // Memory CMAHID
    MS_U32    u32MloadMmapPhyAddr;                       //phy Memory Address
    MS_U32    u32MloadMmapVirAddr;                       //vir Memory Address
    MS_U32    u32MloadMmapSize;                       // Memory Size
    MS_U32    u32MloadMmapAlign;                      // Memory Align
    MS_U32    u32MloadMmapMemoryType;
} MHAL_CMDQ_Mmap_Info_t;

typedef struct MHAL_CMDQ_CmdqInterface_s     MHAL_CMDQ_CmdqInterface_t;

struct MHAL_CMDQ_CmdqInterface_s
{
    //menuload ring buffer dynamic allocation service
//------------------------------------------------------------------------------
/// @brief Get Menuload buffer current write point
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
/// @param[out]MS_PHYADDR* phyWritePtr : assign write point here
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_GetNextMloadRingBufWritePtr)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_PHYADDR* phyWritePtr);
//------------------------------------------------------------------------------
/// @brief Update Menuload buffer read point
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
/// @param[in] MS_PHYADDR phyReadPtr : read point to updtae
/// @param[out]
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_UpdateMloadRingBufReadPtr)(MHAL_CMDQ_CmdqInterface_t* pCmdinf, MS_PHYADDR phyReadPtr);
//------------------------------------------------------------------------------
/// @brief copy buffer to cmdq 's Menuload buffer
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
/// @param[in] void * MloadBuf
/// @param[in] MS_U32 u32Size
/// @param[in] MS_U16 u16Alignment
/// @param[out]MS_PHYADDR *phyRetAddr : menuload buffer head
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_MloadCopyBuf)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, void * MloadBuf, MS_U32 u32Size, MS_U16 u16Alignment, MS_PHYADDR *phyRetAddr);
//------------------------------------------------------------------------------
/// @brief check cmdq buffer is available
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
/// @param[in] MS_U32 u32CmdqNum : check cmd number
/// @param[out]
/// @return 0 : is unavailable
/// @return current cmdq available number : is success
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_CheckBufAvailable)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32CmdqNum);
//------------------------------------------------------------------------------
/// @brief write CMDQ dummy register
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
/// @param[in] MS_U16 u16Value
/// @param[out]
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_WriteDummyRegCmdq)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16 u16Value);
//------------------------------------------------------------------------------
/// @brief Read CMDQ dummy register
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
/// @param[out]MS_U16* u16RegVal : assign cmdq dummy register value.
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_ReadDummyRegCmdq)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16* u16RegVal);
//------------------------------------------------------------------------------
/// @brief add write command with Mask
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @param[in] MS_U32 u32RegAddr
/// @param[in] MS_U16 u16Value
/// @param[in] MS_U16 u16WriteMask : set bit as 1 , this bit is available.
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_WriteRegCmdqMask)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value, MS_U16 u16WriteMask);
//------------------------------------------------------------------------------
/// @brief add multiple write command with Mask
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @param[in] MHAL_CMDQ_MultiCmdBufMask_t *u32MultiCmdBufMask
/// @param[in] MS_U16 u16Size
/// @return Actual size already write to CMDQ buffer
//------------------------------------------------------------------------------
    MS_U32(*MHAL_CMDQ_WriteRegCmdqMaskMulti)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_MultiCmdBufMask_t *u32MultiCmdBufMask, MS_U16 u16Size);
//------------------------------------------------------------------------------
/// @brief add write command without Mask
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @param[in] MS_U32 u32RegAddr
/// @param[in] MS_U16 u16Value
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_WriteRegCmdq)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value);
//------------------------------------------------------------------------------
/// @brief add multiple write command without Mask
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @param[in] MHAL_CMDQ_MultiCmdBuf_t *u32MultiCmdBuf
/// @param[in] MS_U16 u16Size
/// @return Actual size already write to CMDQ buffer
//------------------------------------------------------------------------------
    MS_U32(*MHAL_CMDQ_WriteRegCmdqMulti)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_MultiCmdBuf_t *u32MultiCmdBuf, MS_U16 u16Size);
//------------------------------------------------------------------------------
/// @brief add poll command with Mask
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @param[in] MS_U32 u32RegAddr
/// @param[in] MS_U16 u16Value
/// @param[in] MS_U16 u16WriteMask
/// @param[in] MS_BOOL bPollEq : true is poll eq command , false is poll neq command
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_CmdqPollRegBits)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value,  MS_U16 u16WriteMask, MS_BOOL bPollEq);
//------------------------------------------------------------------------------
/// @brief add wait command
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @param[in] MHAL_CMDQ_EventId_e eEvent
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_CmdqAddWaitEventCmd)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_EventId_e eEvent);
//------------------------------------------------------------------------------
/// @brief Abort cmdq buffer , will go back previrous write point
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_CmdqAbortBuffer)(MHAL_CMDQ_CmdqInterface_t *pCmdinf);
//------------------------------------------------------------------------------
/// @brief reset cmdq engine
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_CmdqResetEngine)(MHAL_CMDQ_CmdqInterface_t *pCmdinf);
//------------------------------------------------------------------------------
/// @brief Read cmdq current status
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_ReadStatusCmdq)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32* u32StatVal);
//------------------------------------------------------------------------------
/// @brief kick off cmdq
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @return < 0 : is failed
/// @return kick off cmd number : is success
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_KickOffCmdq)(MHAL_CMDQ_CmdqInterface_t *pCmdinf);
//------------------------------------------------------------------------------
/// @brief clear all trigger bus event
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_ClearTriggerEvent)(MHAL_CMDQ_CmdqInterface_t *pCmdinf);

//------------------------------------------------------------------------------
/// @brief check cmdq is empty
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @param[out],MS_BOOL* bIdleVal : idle value
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_IsCmdqEmptyIdle)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_BOOL* bIdleVal);
    VOID *  pCtx;
//------------------------------------------------------------------------------
/// @brief kick off cmdq
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @return < 0 : is failed
/// @return kick off cmd number : is success
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_KickOffCmdqByTag)(MHAL_CMDQ_CmdqInterface_t *pCmdinf,MS_U16 u16Tag);
//------------------------------------------------------------------------------
/// @brief command delay
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
///            MS_U32 uTimeNs : ns
/// @return < 0 : is failed
/// @
//------------------------------------------------------------------------------
    MS_S32(*MHAL_CMDQ_CmdDelay)(MHAL_CMDQ_CmdqInterface_t *pCmdinf,MS_U32 uTimeNs);
//------------------------------------------------------------------------------
/// @brief Get current cmdq number
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
/// @return : cmdq number
/// @
//------------------------------------------------------------------------------
    MS_U32 (*MHAL_CMDQ_GetCurrentCmdqNumber)(MHAL_CMDQ_CmdqInterface_t *pCmdinf);
//------------------------------------------------------------------------------
/// @brief add poll command with Mask
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface.
/// @param[in] MS_U32 u32RegAddr
/// @param[in] MS_U16 u16Value
/// @param[in] MS_U16 u16WriteMask
/// @param[in] MS_BOOL bPollEq : true is poll eq command , false is poll neq command
/// @param[in] MS_U32 uTimeNs : poll time (ns)
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
	MS_S32(*MHAL_CMDQ_CmdqPollRegBits_ByTime)(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value,  MS_U16 u16WriteMask, MS_BOOL bPollEq,MS_U32 uTimeNs);

};
//------------------------------------------------------------------------------
/// @brief Init CMDQ mmap info
/// @param[in] MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo
/// @param[out]
/// @return MHAL_SUCCESS : is ok
/// @return MHAL_FAILURE : is failed
//------------------------------------------------------------------------------
MS_S32 MHAL_CMDQ_InitCmdqMmapInfo(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo);

//------------------------------------------------------------------------------
/// @brief Get Cmdq service
/// @param[in] MHAL_CMDQ_Id_e eCmdqId : CMDQ ID
/// @param[in] MHAL_CMDQ_BufDescript_t *pCmdqBufDesp : CMDQ buffer description
/// @param[in] MS_BOOL bForceRIU : CMDQ RIU mode
/// @param[out]
/// @return NULL : is failed
/// @return MHAL_CMDQ_CmdqInterface_t point is success
/// @return MI_ERR_INVALID_PARAMETER: Null parameter
//------------------------------------------------------------------------------
MHAL_CMDQ_CmdqInterface_t *MHAL_CMDQ_GetSysCmdqService(MHAL_CMDQ_Id_e eCmdqId, MHAL_CMDQ_BufDescript_t *pCmdqBufDesp, MS_BOOL bForceRIU);
//------------------------------------------------------------------------------
/// @brief release cmdq service
/// @param[in] MHAL_CMDQ_Id_e eCmdqId : CMDQ ID
/// @param[out]
/// @return
//------------------------------------------------------------------------------
void MHAL_CMDQ_ReleaseSysCmdqService(MHAL_CMDQ_Id_e eCmdqId);

#endif
