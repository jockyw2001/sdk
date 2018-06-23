#ifndef _DRVCMDQ_H_
#define _DRVCMDQ_H_

#if defined(CAM_OS_RTK)
#include "hal_int_ctrl_pub.h"
#else
#include <linux/delay.h>
#include <linux/sched.h>
#include "infinity2/irqs.h"
#endif
#define DRVCMDQ_VERSION             "V1.3"
#define DRVCMDQ_NAME                "cmdq"

#define DRVCMDQ_ENABLE_DUMMY_IRQ    0
#define DRVCMDQ_ENABLE_IRQ          1
#define DIRECT_MODE                 0x01
#define INCREMENT_MODE              0x00
#define RING_BUFFER_MODE            0x04

#define DVRCMDQ_COMMAND_SIZE         8
#define DVRCMDQ_COMMAND_SIZE_ALIGN   16
#define DVRCMDQ_COMMAND_ALIGN        2
#define DVRCMDQ_TRIG_EVENT_NUM       16

#define DRVCMDQ_CMDTYPE_NULL         0x00
#define DRVCMDQ_CMDTYPE_WRITE        0x10
#define DRVCMDQ_CMDTYPE_WAIT         0x20
#define DRVCMDQ_CMDTYPE_POLLEQ       0x30
#define DRVCMDQ_CMDTYPE_POLLNEQ      0xb0

#define DRVCMDQ_IRQ_STATE_TRIG         (1 << 0)
#define DRVCMDQ_IRQ_STATE_DONE         (1 << 2)

#define DRVCMDQ_IRQ_SW1                (1 << 4)
#define DRVCMDQ_IRQ_SW2                (1 << 5)
#define DRVCMDQ_IRQ_SW3                (1 << 6)
#define DRVCMDQ_IRQ_SW4                (1 << 7)
#define DRVCMDQ_IRQ_SW_ALL             (DRVCMDQ_IRQ_SW1|DRVCMDQ_IRQ_SW2|DRVCMDQ_IRQ_SW3|DRVCMDQ_IRQ_SW4)


#define DRVCMDQ_POLLNEQ_TIMEOUT        (0x1<<8)
#define DRVCMDQ_POLLEQ_TIMEOUT         (0x1<<9)
#define DRVCMDQ_WAIT_TIMEOUT           (0x1<<10)
#define DRVCMDQ_WRITE_TIMEOUT          (0x1<<12)

#define DRVCMDQ_ERROR_STATUS           (DRVCMDQ_POLLNEQ_TIMEOUT|DRVCMDQ_POLLEQ_TIMEOUT \
                                        |DRVCMDQ_WAIT_TIMEOUT|DRVCMDQ_WRITE_TIMEOUT)

#define DRVCMD_BUFFER_SHIFT          4
#define DRVCMD_TRIGGE_BUS_CLR_CNT    4

#define DRVCMD_FORBIDDENTAG_NUM      128

#if defined(HAL_I2_SIMULATE)
#define DRVCMDQ_CMDQBUFFER_SIZE       (1*1024*1024)
#define DRVCMDQ_MLOADBUFFER_SIZE      (1*1024*1024)
#define DRVCMD_NO_KO_MODULE           1

#elif defined(CAM_OS_RTK)
#define DRVCMDQ_CMDQBUFFER_SIZE       (512*1024)
#define DRVCMDQ_MLOADBUFFER_SIZE      (512*1024)
#define DRVCMD_NO_KO_MODULE           1

#else
#define DRVCMD_NO_KO_MODULE           0
#endif

#define DRVCMDQ_MIU_CLK               (333) //333Mhz
typedef enum
{
    DRVCMDQ_ID_VPE = 0,
    DRVCMDQ_ID_DIVP,
    DRVCMDQ_ID_H265_VENC0,
    DRVCMDQ_ID_H265_VENC1,
    DRVCMDQ_ID_H264_VENC0,
    DRVCMDQ_ID_MAX
} DrvCcmdqId_e;

typedef enum
{
    EN_CMDQ_MIU_0,
    EN_CMDQ_MIU_1
} DrvCmdqMiu_e;

#ifdef HAL_K6_SIMULATE
#define CMDQ_RIU_BASE                0xfd000000UL
#define CMDQ_IP0_IRQ_NUM             (0xD0+13)
#define DRVCMDQEVE_MAX               23

typedef enum
{
    EN_CMDQ_TYPE_IP0,
    EN_CMDQ_TYPE_MAX
} DrvCmdqIPSupport_e;

typedef enum
{
    DRVCMDQEVE_S0_MDW_W_DONE     =     0x01    ,         //Only for cmdq1&cmdq5
    DRVCMDQEVE_S0_MGW_FIRE       =     0x02    ,         //Only for cmdq1&cmdq5
    DRVCMDQEVE_S1_MDW_W_DONE     =     0x04    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_S1_MGW_FIRE       =     0x08    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_DMAGEN_TRIGGER0   =     0x10    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_DMAGEN_TRIGGER1   =     0x20    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_BDMA_TRIGGER0     =     0x40    ,         //Only for cmdq3&cmdq5
    DRVCMDQEVE_BDMA_TRIGGER1     =     0x80    ,         //Only for cmdq3
    DRVCMDQEVE_IVE_CMDQ_TRIG     =     0x100   ,         //Only for cmdq3&cmdq5
    DRVCMDQEVE_LDC_CMDQ_TRIG     =     0x200   ,         //Only for cmdq1&cmdq3
    DRVCMDQEVE_GE_CMDQ_TRIG      =     0x400   ,         //Only for cmdq1&cmdq3
    DRVCMDQEVE_REG_DUMMY_TRIG    =     0x800   ,         //Only for cmdq1&cmdq2&cmdq4&cmdq5
    DRVCMDQEVE_CORE1_MHE_TRIG    =     0x1000  ,         //Only for ALL
    DRVCMDQEVE_CORE0_MHE_TRIG    =     0x2000  ,         //Only for ALL
    DRVCMDQEVE_CORE1_MFE_TRIG    =     0x4000  ,         //Only for ALL
    DRVCMDQEVE_CORE0_MFE_TRIG    =     0x8000  ,         //Only for ALL
    DRVCMDQEVE_DIP_TRIG          =     0x10000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG4         =     0x20000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG2         =     0x40000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG013       =     0x80000 ,         //Only for ALL
    DRVCMDQEVE_SC_TRIG2          =     0x100000,         //Only for ALL
    DRVCMDQEVE_SC_TRIG013        =     0x200000,         //Only for ALL
    DRVCMDQEVE_ISP_TRIG          =     0x400000          //Only for ALL
} DRV_CMDQ_EVENT_ID;
#elif defined(HAL_I3_SIMULATE)

#define CMDQ_RIU_BASE                0xfd000000UL
#define CMDQ_IP0_IRQ_NUM             0
#define DRVCMDQEVE_MAX               23
typedef enum
{
    EN_CMDQ_TYPE_IP0,
    EN_CMDQ_TYPE_MAX
} DrvCmdqIPSupport_e;

typedef enum
{
    DRVCMDQEVE_S0_MDW_W_DONE     =     0x01    ,         //Only for cmdq1&cmdq5
    DRVCMDQEVE_S0_MGW_FIRE       =     0x02    ,         //Only for cmdq1&cmdq5
    DRVCMDQEVE_S1_MDW_W_DONE     =     0x04    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_S1_MGW_FIRE       =     0x08    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_DMAGEN_TRIGGER0   =     0x10    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_DMAGEN_TRIGGER1   =     0x20    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_BDMA_TRIGGER0     =     0x40    ,         //Only for cmdq3&cmdq5
    DRVCMDQEVE_BDMA_TRIGGER1     =     0x80    ,         //Only for cmdq3
    DRVCMDQEVE_IVE_CMDQ_TRIG     =     0x100   ,         //Only for cmdq3&cmdq5
    DRVCMDQEVE_LDC_CMDQ_TRIG     =     0x200   ,         //Only for cmdq1&cmdq3
    DRVCMDQEVE_GE_CMDQ_TRIG      =     0x400   ,         //Only for cmdq1&cmdq3
    DRVCMDQEVE_REG_DUMMY_TRIG    =     0x800   ,         //Only for cmdq1&cmdq2&cmdq4&cmdq5
    DRVCMDQEVE_CORE1_MHE_TRIG    =     0x1000  ,         //Only for ALL
    DRVCMDQEVE_CORE0_MHE_TRIG    =     0x2000  ,         //Only for ALL
    DRVCMDQEVE_CORE1_MFE_TRIG    =     0x4000  ,         //Only for ALL
    DRVCMDQEVE_CORE0_MFE_TRIG    =     0x8000  ,         //Only for ALL
    DRVCMDQEVE_DIP_TRIG          =     0x10000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG4         =     0x20000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG2         =     0x40000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG013       =     0x80000 ,         //Only for ALL
    DRVCMDQEVE_SC_TRIG2          =     0x100000,         //Only for ALL
    DRVCMDQEVE_SC_TRIG013        =     0x200000,         //Only for ALL
    DRVCMDQEVE_ISP_TRIG          =     0x400000          //Only for ALL
} DRV_CMDQ_EVENT_ID;
#elif defined(CAM_OS_RTK)
#define CMDQ_RIU_BASE                0x1F000000
#define CMDQ_IP0_IRQ_NUM             (MS_INT_NUM_IRQ_73_CMDQ_INT)
#define CMDQ_IP1_IRQ_NUM             (MS_INT_NUM_IRQ_112_CMDQ_INT2)
#define CMDQ_IP2_IRQ_NUM             (MS_INT_NUM_IRQ_74_CMDQ3_INT)
#define CMDQ_IP3_IRQ_NUM             (MS_INT_NUM_IRQ_90_CMDQ4_INT)
#define CMDQ_IP4_IRQ_NUM             (MS_INT_NUM_IRQ_89_CMDQ5_INT)
#define DRVCMDQEVE_MAX               23

#if DRVCMDQ_ENABLE_DUMMY_IRQ
#define CMDQ_IP0_DUMMY_IRQ_NUM             (MS_INT_NUM_IRQ_08_REG_CMDQ_DUMMY_15)
#define CMDQ_IP1_DUMMY_IRQ_NUM             (MS_INT_NUM_IRQ_27_REG_CMDQ2_DUMMY_15)
#define CMDQ_IP2_DUMMY_IRQ_NUM             (MS_INT_NUM_IRQ_28_REG_CMDQ3_DUMMY_15)
#define CMDQ_IP3_DUMMY_IRQ_NUM             (MS_INT_NUM_IRQ_32_REG_CMDQ4_DUMMY_15)
#define CMDQ_IP4_DUMMY_IRQ_NUM             (MS_INT_NUM_IRQ_57_REG_CMDQ5_DUMMY)
#endif

typedef enum
{
    EN_CMDQ_TYPE_IP0,
    EN_CMDQ_TYPE_IP1,
    EN_CMDQ_TYPE_IP2,
    EN_CMDQ_TYPE_IP3,
    EN_CMDQ_TYPE_IP4,
    EN_CMDQ_TYPE_MAX
} DrvCmdqIPSupport_e;

typedef enum
{
    DRVCMDQEVE_S0_MDW_W_DONE     =     0x01    ,         //Only for cmdq1&cmdq5
    DRVCMDQEVE_S0_MGW_FIRE       =     0x02    ,         //Only for cmdq1&cmdq5
    DRVCMDQEVE_S1_MDW_W_DONE     =     0x04    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_S1_MGW_FIRE       =     0x08    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_DMAGEN_TRIGGER0   =     0x10    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_DMAGEN_TRIGGER1   =     0x20    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_BDMA_TRIGGER0     =     0x40    ,         //Only for cmdq3&cmdq5
    DRVCMDQEVE_BDMA_TRIGGER1     =     0x80    ,         //Only for cmdq3
    DRVCMDQEVE_IVE_CMDQ_TRIG     =     0x100   ,         //Only for cmdq3&cmdq5
    DRVCMDQEVE_LDC_CMDQ_TRIG     =     0x200   ,         //Only for cmdq1&cmdq3
    DRVCMDQEVE_GE_CMDQ_TRIG      =     0x400   ,         //Only for cmdq1&cmdq3
    DRVCMDQEVE_REG_DUMMY_TRIG    =     0x800   ,         //Only for cmdq1&cmdq2&cmdq4&cmdq5
    DRVCMDQEVE_CORE1_MHE_TRIG    =     0x1000  ,         //Only for ALL
    DRVCMDQEVE_CORE0_MHE_TRIG    =     0x2000  ,         //Only for ALL
    DRVCMDQEVE_CORE1_MFE_TRIG    =     0x4000  ,         //Only for ALL
    DRVCMDQEVE_CORE0_MFE_TRIG    =     0x8000  ,         //Only for ALL
    DRVCMDQEVE_DIP_TRIG          =     0x10000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG4         =     0x20000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG2         =     0x40000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG013       =     0x80000 ,         //Only for ALL
    DRVCMDQEVE_SC_TRIG2          =     0x100000,         //Only for ALL
    DRVCMDQEVE_SC_TRIG013        =     0x200000,         //Only for ALL
    DRVCMDQEVE_ISP_TRIG          =     0x400000          //Only for ALL
} DRV_CMDQ_EVENT_ID;

#else
#define CMDQ_RIU_BASE                0xFD000000
#define CMDQ_IP0_IRQ_NUM             (32+INT_IRQ_73_CMDQ_INT)
#define CMDQ_IP1_IRQ_NUM             (32+INT_IRQ_112_CMDQ_INT2)
#define CMDQ_IP2_IRQ_NUM             (32+INT_IRQ_74_CMDQ3_INT)
#define CMDQ_IP3_IRQ_NUM             (32+INT_IRQ_90_CMDQ4_INT)
#define CMDQ_IP4_IRQ_NUM             (32+INT_IRQ_89_CMDQ5_INT)
#define DRVCMDQEVE_MAX               23

#if DRVCMDQ_ENABLE_DUMMY_IRQ
#define CMDQ_IP0_DUMMY_IRQ_NUM             (32+INT_IRQ_08_REG_CMDQ_DUMMY_15)
#define CMDQ_IP1_DUMMY_IRQ_NUM             (32+INT_IRQ_27_REG_CMDQ2_DUMMY_15)
#define CMDQ_IP2_DUMMY_IRQ_NUM             (32+INT_IRQ_28_REG_CMDQ3_DUMMY_15)
#define CMDQ_IP3_DUMMY_IRQ_NUM             (32+INT_IRQ_32_REG_CMDQ4_DUMMY_15)
#define CMDQ_IP4_DUMMY_IRQ_NUM             (32+INT_IRQ_57_REG_CMDQ5_DUMMY)
#endif

#define CDMQ_MASK_DEFAULT                   0xFFFF
typedef enum
{
    EN_CMDQ_TYPE_IP0,
    EN_CMDQ_TYPE_IP1,
    EN_CMDQ_TYPE_IP2,
    EN_CMDQ_TYPE_IP3,
    EN_CMDQ_TYPE_IP4,
    EN_CMDQ_TYPE_MAX
} DrvCmdqIPSupport_e;

typedef enum
{
    DRVCMDQEVE_S0_MDW_W_DONE     =     0x01    ,         //Only for cmdq1&cmdq5
    DRVCMDQEVE_S0_MGW_FIRE       =     0x02    ,         //Only for cmdq1&cmdq5
    DRVCMDQEVE_S1_MDW_W_DONE     =     0x04    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_S1_MGW_FIRE       =     0x08    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_DMAGEN_TRIGGER0   =     0x10    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_DMAGEN_TRIGGER1   =     0x20    ,         //Only for cmdq2&cmdq4
    DRVCMDQEVE_BDMA_TRIGGER0     =     0x40    ,         //Only for cmdq3&cmdq5
    DRVCMDQEVE_BDMA_TRIGGER1     =     0x80    ,         //Only for cmdq3
    DRVCMDQEVE_IVE_CMDQ_TRIG     =     0x100   ,         //Only for cmdq3&cmdq5
    DRVCMDQEVE_LDC_CMDQ_TRIG     =     0x200   ,         //Only for cmdq1&cmdq3
    DRVCMDQEVE_GE_CMDQ_TRIG      =     0x400   ,         //Only for cmdq1&cmdq3
    DRVCMDQEVE_REG_DUMMY_TRIG    =     0x800   ,         //Only for cmdq1&cmdq2&cmdq4&cmdq5
    DRVCMDQEVE_CORE1_MHE_TRIG    =     0x1000  ,         //Only for ALL
    DRVCMDQEVE_CORE0_MHE_TRIG    =     0x2000  ,         //Only for ALL
    DRVCMDQEVE_CORE1_MFE_TRIG    =     0x4000  ,         //Only for ALL
    DRVCMDQEVE_CORE0_MFE_TRIG    =     0x8000  ,         //Only for ALL
    DRVCMDQEVE_DIP_TRIG          =     0x10000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG4         =     0x20000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG2         =     0x40000 ,         //Only for ALL
    DRVCMDQEVE_GOP_TRIG013       =     0x80000 ,         //Only for ALL
    DRVCMDQEVE_SC_TRIG2          =     0x100000,         //Only for ALL
    DRVCMDQEVE_SC_TRIG013        =     0x200000,         //Only for ALL
    DRVCMDQEVE_ISP_TRIG          =     0x400000          //Only for ALL
} DRV_CMDQ_EVENT_ID;

#endif

typedef struct _CmdqTriggerEvent_s
{
    DRV_CMDQ_EVENT_ID   eTrigEvnt;
    u32                 nBit;
} CmdqTriggerEvent_t;

typedef struct _CmdqPollTimeRegInfo_t
{
    u32      nCmdqCtlReg;
    u32      nDummyReg;
    u32      nInitDone;
} CmdqPollTimeRegInfo_t;

typedef struct _CmdqBufferInfor_s
{
    u32    BufPhy;
    u32    BufVir;
    u32    nBufSize;

} CmdqBufferInfor_t;

typedef struct _CmdqQueInfor_s
{
    u8         *pBufStart;
    u8         *pBufEnd;
    u8         *pBufWrite;
    u8         *pBufReadHw;
    u8         *pBufFireWriteHw;
    u8         *pBufLastFireWriteHw;
    u32        nCurCmdCount;
    u32        nTotalCmdCount;
} CmdqQueInfor_t;

typedef struct _CmdqInterfacePriv_s
{
    u8                        bUsed;
    u8                        bFirstFireDone;
    DrvCcmdqId_e              CmdqIpId;
    DrvCmdqIPSupport_e        nCmdqDrvIpId;
    bool                      bForceRIU;
    u32                       nCmdMode;
    u32                       nCmdMiuSel;
    u32                       nCmdqRiuBase;
    u32                       nCmdqIrq;
    CamOsMutex_t              tCmdqMutex;
    CmdqBufferInfor_t         tCmdqBuf;
    CmdqQueInfor_t            tCmdqInfo;
    CamOsMutex_t              tMloadMutex;
    CmdqBufferInfor_t         tMloadBuf;
    CmdqQueInfor_t            tMloadInfo;
    CmdqTriggerEvent_t        tDrvCmdTrigEvt[DVRCMDQ_TRIG_EVENT_NUM];
    u16                       nForbiddenTag[DRVCMD_FORBIDDENTAG_NUM];
    //CamOsTsem_t               tWaitQue;
#if DRVCMDQ_ENABLE_IRQ
    CamOsTsem_t               tIrqWaitSem;
#endif
} CmdqInterfacePriv_t;

typedef unsigned int                 DRVCMDQ_RESULT;

/// @name DRVCMDQ_RESULT
/// @ref DRVCMDQ_RESULT
/// return value
/// @{
#define DRVCMDQ_OK                   0x00000000
#define DRVCMDQ_FAIL                 0x00000001
#define DRVCMDQ_INVALID_PARAM        0x00000002
#define DRVCMDQ_FUNC_ERROR           0x00000003
#define DRVCMDQ_MIU_ADDR_ERROR       0x00000004
#define DRVCMDQ_CMDQ_FULL            0x00000005

CmdqInterfacePriv_t* MDrvCmdqInit(DrvCcmdqId_e eCmdId);
void MDrvCmdqSetBufInfo(CmdqInterfacePriv_t* pPrivData);
CmdqInterfacePriv_t* MDrvGetCmdqPrivateDataByCmdqId(DrvCcmdqId_e eCmdId);
DRVCMDQ_RESULT MDrvCMDQSetTimerRatio(DrvCmdqIPSupport_e eIpNum, u32 time, u32 ratio);
void MDrvRelesaeCmdqService(CmdqInterfacePriv_t *pPrivData);
u32 MDrvCmdqGetAvailCmdqNumber(void* pPriv);
u32 MDrvCmdqCheckBufferAvail(void* pPriv, u32 nCmdqNum);
s32 MDrvCmdqWriteCommandMask(void* pPriv, u32 reg_addr, u16 value, u16 write_mask);
u32 MDrvCmdqWriteCommandMaskMulti(void* pPriv, void *u32MultiCmdBufMask, u16 u16Size);
s32 MDrvCmdqWriteCommand(void* pPriv, u32 reg_addr, u16 value);
u32 MDrvCmdqWriteCommandMulti(void* pPriv, void *u32MultiCmdBuf, u16 u16Size);
s32 MDrvCmdqPollEqCommandMask(void* pPriv, u32 reg_addr, u16 value, u16 write_mask, bool bPollEq);
s32 MDrvCmdqWaitCommand(void* pPriv,  DRV_CMDQ_EVENT_ID nTrigEvnt);
s32 MDrvCmdqDummyCommand(void* pPriv);
s32 MDrvCmdqKickOfftCommand(void* pPriv);
s32 MDrvCmdqKickOfftCommandByTag(void* pPriv,u16 nTagValue);

bool MDrvCmdqIsIdle(void* pPriv);
s32  MDrvUpdateMloadRingBufReadPtr(void* pPriv, u64 nReadPtr);
u64  MDrvGetMloadRingBufWritePtr(void* pPriv);
u64  MDrvCmdqMloadCopyBuf(void* pPriv, void * mload_buf, u32 size, u16 u16Alignment);
s32 MDrvAbortCmdqCommand(void* pPriv);
s32 MDrvCmdqWriteDummyRegister(void* pPriv, u16 nValue);
u16 MDrvCmdqReadDummyRegister(void* pPriv);

void MDrvCmdDbgWriteDummy(void* pPriv, u32 value);
u32 MDrvCmdDbgReadDummy(void* pPriv);
u16 MDrvCmdqReadIrqStatus(void* pPriv);
void MDrvCmdqDumpContext(void);
u32 MDrvGetDummyRegisterRiuAddr(void* pPriv);
void MDrvCmdqPrintfCrashCommand(void* pPriv);
s32 MDrvCmdqResetEngine(void* pPriv);
s32 MDrvCmdqClearTriggerEvent(void* pPriv, u16 u16Event);
s32 MDrvCmdqCmdDelay(void* pPriv,u32 nTimens);
u32 MDrvCmdqGetCurrentCmdqNumber(void* pPriv);
s32 MDrvCmdqPollEqCommandMaskByTime(void* pPriv, u32 reg_addr, u16 value, u16 write_mask, bool bPollEq,u32 nTimeNs);

#define CMDQ_PROFILE 0

#if CMDQ_PROFILE

#define _CMDQ_PROFILE_WriteCMD 8
#define _CMDQ_PROFILE_DRV_CHECK  9
#define _CMDQ_PROFILE_DRV_CHECK2 10
#define _CMDQ_PROFILE_CHECK_BUF 11
#define _CMDQ_PROFILE_SLEEP 17
#define _CMDQ_PROFILE_KICK 18

#define U16 unsigned short
extern void MDrv_GPIO_Set_Low(U16 u8IndexGPIO);
extern void MDrv_GPIO_Set_High(U16 u8IndexGPIO);
extern void MDrv_GPIO_Pad_Set(U16 u8IndexGPIO);
#define _CDMQ_PROFILE_LOW(x) MDrv_GPIO_Set_Low(x)
#define _CDMQ_PROFILE_HIGH(x) MDrv_GPIO_Set_High(x)
#define _CDMQ_PROFILE_SET(x) MDrv_GPIO_Set_Set()
#define _CDMQ_PROFILE_INIT \
    MDrv_GPIO_Pad_Set(_CMDQ_PROFILE_DRV_CHECK); \
    MDrv_GPIO_Pad_Set(_CMDQ_PROFILE_DRV_CHECK2); \
    MDrv_GPIO_Pad_Set(_CMDQ_PROFILE_SLEEP); \
    MDrv_GPIO_Pad_Set(_CMDQ_PROFILE_WriteCMD); \
    MDrv_GPIO_Pad_Set(_CMDQ_PROFILE_CHECK_BUF); \
    MDrv_GPIO_Pad_Set(_CMDQ_PROFILE_KICK); \
    MDrv_GPIO_Set_Low(_CMDQ_PROFILE_DRV_CHECK); \
    MDrv_GPIO_Set_Low(_CMDQ_PROFILE_DRV_CHECK2); \
    MDrv_GPIO_Set_Low(_CMDQ_PROFILE_SLEEP); \
    MDrv_GPIO_Set_Low(_CMDQ_PROFILE_WriteCMD); \
    MDrv_GPIO_Set_Low(_CMDQ_PROFILE_CHECK_BUF); \
    MDrv_GPIO_Set_Low(_CMDQ_PROFILE_KICK);

#else

#define _CDMQ_PROFILE_LOW(x)
#define _CDMQ_PROFILE_HIGH(x)
#define _CDMQ_PROFILE_SET(x)
#define _CDMQ_PROFILE_INIT

#endif

#endif
