#ifndef MSOS_TYPE_LINUX_KERNEL
#include "stdio.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "MMAPInfo.h"
#else
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#endif

#include "mhal_common_cmdq.h"

//#include "../../include/common/mhal_common.h"
//#include "cmdq_service.h"
#include "mhal_cmdq.h"

//#include "mhal_common.h"
//#include "cmdq_service.h"
#include "MsOS.h"
#include "drvCMDQ.h"
#include "drvMMIO.h"

#define LOG_TAG "[cmdq_srv]"
//#define CMDQ_WRITE_RIU_COMMAND  (0x57)
typedef struct _MI_CMDQ_DATA
{
    MHAL_CMDQ_Id_e eCmdqId;
    //pthread_mutex_t cmdqMutex;// = PTHREAD_MUTEX_INITIALIZER;
    MS_BOOL bForceRIU;
    MS_U32 u32XcRiuBase;//_XC_RIU_BASE
    MS_U32 u32DummyRegAddr;
    MS_U32 u32DummyRegValue;
    MS_U16 u32CmdqMiuNo;
    MS_U32 u32CmdqBufAddr; //phy buffer for cmdq HW
    MS_U32 u32CmdqBufSize; //phy buffer for cmdq HW
    MS_U32 u32CmdqBufAlign; //phy buffer for cmdq HW

    MS_U32 u32CmdqCmdMaxNum;
    MS_U32 u32CmdqCmdIndex;
    CAF_Struct *pstCmdqCmdBuf;
} MI_CMDQ_DATA;
static MHAL_CMDQ_CmdqInterface_t g_stCmdqOps[E_MHAL_CMDQ_ID_MAX] = {{0}};
static MHAL_CMDQ_CmdqInterface_t * pCmdqOps[E_MHAL_CMDQ_ID_MAX] = {NULL, NULL,NULL,NULL,NULL,};
static MS_VIRT g_u32XcRiuBase = 0;//only for xc space

#define RIU_READ_BYTE(_reg)             (*(volatile MS_U8*)(_reg))
#define RIU_READ_WORD(_reg)             (*(volatile MS_U16*)(_reg))
#define RIU_WRITE_BYTE(_reg, _val)      { (*((volatile MS_U8*)(_reg))) = (MS_U8)(_val); }
#define RIU_WRITE_WORD(_reg, _val)      { (*((volatile MS_U16*)(_reg))) = (MS_U16)(_val); }

#define RIU_ReadByte( u32Reg )      RIU_READ_BYTE(((u32Reg) << 1) - ((u32Reg) & 1))

#define RIU_Read2Byte(u32Reg)     (RIU_READ_WORD((u32Reg) << 1))

#define RIU_ReadRegBit( u32Reg, u8Mask )    (RIU_READ_BYTE(((u32Reg) << 1) - ((u32Reg) & 1)) & (u8Mask))

#define RIU_WriteRegBit( u32Reg, bEnable, u8Mask ) \
    do {\
    RIU_WRITE_BYTE((((u32Reg) << 1) - ((u32Reg) & 1)), (bEnable) ? (RIU_READ_BYTE((((u32Reg) << 1) - ((u32Reg) & 1))  ) | (u8Mask)) :\
                                (RIU_READ_BYTE((((u32Reg) <<1) - ((u32Reg) & 1))) & ~(u8Mask))); \
    } while(0)

#define RIU_WriteByte( u32Reg, u8Val )\
    do {\
    RIU_WRITE_BYTE(((u32Reg) << 1) - ((u32Reg) & 1), u8Val); \
    } while(0)

#define RIU_Write2Byte( u32Reg, u16Val ) \
    do {\
    if (((u32Reg) & 0x01)) \
    { \
        RIU_WRITE_BYTE(((u32Reg) << 1) - 1, (MS_U8)((u16Val))); \
        RIU_WRITE_BYTE(((u32Reg) + 1) << 1, (MS_U8)((u16Val) >> 8)); \
    } \
    else \
    { \
        RIU_WRITE_WORD( ((u32Reg)<<1) ,  u16Val);\
    }\
    } while(0)

#define RIU_WriteByteMask( u32Reg, u8Val, u8Msk )\
    do {\
    RIU_WRITE_BYTE( (((u32Reg) <<1) - ((u32Reg) & 1)), (RIU_READ_BYTE((((u32Reg) <<1) - ((u32Reg) & 1))) & ~(u8Msk)) | ((u8Val) & (u8Msk)));\
    } while(0)
#define RIU_WriteWordMask( u32Reg, u16Val , u16Msk)                                               \
    do {\                                                                   \
    if ( ((u32Reg) & 0x01) )                                                        \
    {                                                                                           \
        if ((u16Msk)&0xff00) RIU_WriteByteMask( ((u32Reg)+1) , (((u16Val) & 0xff00)>>8) , (((u16Msk)&0xff00)>>8) );    \
        RIU_WriteByteMask( (u32Reg) , ((u16Val) & 0x00ff) , ((u16Msk)&0x00ff) );                                                                          \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        RIU_Write2Byte( ((u32Reg)<<1) ,  (((u16Val) & (u16Msk))  | (RIU_Read2Byte( u32Reg  ) & (~( u16Msk ))))  );                                                       \
    }                                                                               \
    } while(0)

#define MUTEX_LOCK(pstCmdqData) //pthread_mutex_lock(&pstCmdqData->cmdqMutex)
#define MUTEX_UNLOCK(pstCmdqData) //pthread_mutex_unlock(&pstCmdqData->cmdqMutex)
#define CHECK_CMDQ_INDEX(pstCmdqData) do {\
    if(pstCmdqData->u32CmdqCmdIndex >= pstCmdqData->u32CmdqCmdMaxNum) {\
        printk("%s:%d cmdq_srv error,cmd buffer is not enough, index=%d, max_num=%d.\n", __FUNCTION__, __LINE__,\
        pstCmdqData->u32CmdqCmdIndex, pstCmdqData->u32CmdqCmdMaxNum);\
    }
#define CHECK_CMDQ_POINTER(x) do{ if(!(x) printf("%s:%d %s is null, return error.\n",__FUNCTION__, __LINE__,  #x))}while(0)
#define ASSERT(_x_)                                                                         \
    do  {                                                                                   \
        if ( ! ( _x_ ) )                                                                    \
        {                                                                                   \
            printk("ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);     \
            /*abort();         */                                                               \
        }                                                                                   \
    } while (0)

#define CHECK_CMDQ_BUFFER_NUM(pstCmdqData) \
    do {\
        if (pstCmdqData->u32CmdqCmdIndex >= pstCmdqData->u32CmdqCmdMaxNum) {\
            printk("%s:%d cmdq_srv error,cmd buffer is not enough, index=%d, max_num=%d.\n", __FUNCTION__, __LINE__,\
                pstCmdqData->u32CmdqCmdIndex, pstCmdqData->u32CmdqCmdMaxNum);\
            return -1;\
        }\
    } while(0)

#define CHECK_CMDQ_ID(eCmdqId) \
    do{\
        if (eCmdqId != E_MHAL_CMDQ_ID_DIVP)\
        {\
            return MHAL_SUCCESS;\
        }\
    } while(0)

#ifdef MSOS_TYPE_LINUX_KERNEL
#ifndef _LOG_
#define _LOG_ printk
#endif
#else
#ifndef _LOG_
#define _LOG_ printf
#endif
#endif
#define FUN_ENTRY() //do{_LOG_("%s:%d, cmdq_srv entry.\n", __FUNCTION__, __LINE__);}while(0)
#define FUN_EXIT() //do{_LOG_("%s:%d, cmdq_srv exit.\n", __FUNCTION__, __LINE__);}while(0)
#define LOG_DBG(x,...) //_LOG_("%s:%d"LOG_TAG" "x"",__FUNCTION__, __LINE__,  ##__VA_ARGS__)
#define LOG_INFO(x,...) _LOG_("%s:%d"LOG_TAG" "x"",__FUNCTION__, __LINE__,  ##__VA_ARGS__)
#define LOG_ERR(x,...) _LOG_("%s:%d"LOG_TAG" error:"x"",__FUNCTION__, __LINE__,  ##__VA_ARGS__)

//export _XC_RIU_BASE from utpa2k.ko
extern MS_U32 _XC_RIU_BASE;

//internal api
static bool _allocate_mem_for_cmdq(MHAL_CMDQ_CmdqInterface_t * pstCmdqInfo, MHAL_CMDQ_BufDescript_t *pstCmdqBuf)
{
    MI_CMDQ_DATA *pstCmdqData = (MI_CMDQ_DATA *)pstCmdqInfo->pCtx;
    FUN_ENTRY();
#if 0
    //allocate mem for dip cmdq
    MMapInfo_t* pMMapInfo = MMAPInfo::GetInstance()->get_mmap(
        MMAPInfo::GetInstance()->StrToMMAPID("E_MMAP_ID_CMDQ"));//E_MMAP_ID_CMDQ "E_MMAP_ID_INJPLAY"E_MMAP_ID_CMDQ
    if (pMMapInfo == NULL) {
        LOG_ERR("cmdq_srv get mmap info for cmdq error.\n");
        goto END;
    } else {
        LOG_ERR("cmdq_srv E_MMAP_ID_CMDQ CMDQ buffer: u32Addr=%#x, size=%d, u32MiuSel=%d.\n",
            pMMapInfo->u32Addr, pMMapInfo->u32Size, pMMapInfo->u32MiuNo);

        //check 128 bits alignment.
        if (pstCmdqBuf->u32CmdqBufSize > pMMapInfo->u32Size
            || pMMapInfo->u32Align % pstCmdqBuf->u32CmdqBufSizeAlign) {
            LOG_ERR("cmdq_srv allocate mem error, u32CmdqBufSize=%d, mmapSize=%d, u32CmdqBufSizeAlign=%d, u32Align=%d.\n",
                pstCmdqBuf->u32CmdqBufSize, pMMapInfo->u32Size,pstCmdqBuf->u32CmdqBufSizeAlign, pMMapInfo->u32Align);
            goto END;
        }
        pstCmdqData->u32CmdqMiuNo = pMMapInfo->u32MiuNo;
        pstCmdqData->u32CmdqBufAddr = pMMapInfo->u32Addr;
        pstCmdqData->u32CmdqBufSize = pMMapInfo->u32Size;
        pstCmdqData->u32CmdqBufAlign = pMMapInfo->u32Align;

        MDrv_CMDQ_Init(pstCmdqData->u32CmdqMiuNo);
        MDrv_CMDQ_Get_Memory_Size(pstCmdqData->u32CmdqBufAddr,
            (pstCmdqData->u32CmdqBufAddr + pstCmdqData->u32CmdqBufSize), pstCmdqData->u32CmdqMiuNo);

    }
    return true;
END:
    pstCmdqData->u32CmdqBufAddr = 0;
    pstCmdqData->u32CmdqBufSize = 0;
    pstCmdqData->u32CmdqBufAlign = 0;
    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return false;
#else

/* E_MMAP_ID_CMDQ   */
/*
#define E_MMAP_ID_CMDQ_LAYER                                   1
#define E_MMAP_ID_CMDQ_AVAILABLE                               0x0000000000
#define E_MMAP_ID_CMDQ_ADR                                     0x0000000000  //Alignment 0x01000
#define E_MMAP_ID_CMDQ_GAP_CHK                                 0x0000000000
#define E_MMAP_ID_CMDQ_LEN                                     0x0000030000
#define E_MMAP_ID_CMDQ_MEMORY_TYPE                             (MIU0 | TYPE_NONE | UNCACHE | TYPE_NONE)
#define E_MMAP_ID_CMDQ_CMA_HID                                 0

*/
#if 1
     pstCmdqData->u32CmdqMiuNo = 0;//pMMapInfo->u32MiuNo;
     pstCmdqData->u32CmdqBufAddr = 0x0000000000;//pMMapInfo->u32Addr;
     pstCmdqData->u32CmdqBufSize = 0x30000;//pMMapInfo->u32Size;
     pstCmdqData->u32CmdqBufAlign = 0x01000;//pMMapInfo->u32Align;

    MsOS_MPool_Mapping(0,0x0000000000,0x30000/*BigAddr-SmallAddr*/, TRUE );
     //MDrv_CMDQ_Init(pstCmdqData->u32CmdqMiuNo);
     //MDrv_CMDQ_Get_Memory_Size(0x9100000/*pstCmdqData->u32CmdqBufAddr*/,
     //    0x9200000/*(pstCmdqData->u32CmdqBufAddr + pstCmdqData->u32CmdqBufSize)*/, pstCmdqData->u32CmdqMiuNo);
    LOG_INFO("MDrv_CMDQ_Init.\n");
    MDrv_CMDQ_Init(0);
    MDrv_CMDQ_Get_Memory_Size(0x0000000000, 0x30000, 0);
#else
     pstCmdqData->u32CmdqMiuNo = 0;//pMMapInfo->u32MiuNo;
     pstCmdqData->u32CmdqBufAddr = 0x0009100000;//pMMapInfo->u32Addr;
     pstCmdqData->u32CmdqBufSize = 0x0000100000;//pMMapInfo->u32Size;
     pstCmdqData->u32CmdqBufAlign = 0x01000;//pMMapInfo->u32Align;

    MsOS_MPool_Mapping(0,0x9100000,0x0000100000/*BigAddr-SmallAddr*/, TRUE );
     //MDrv_CMDQ_Init(pstCmdqData->u32CmdqMiuNo);
     //MDrv_CMDQ_Get_Memory_Size(0x9100000/*pstCmdqData->u32CmdqBufAddr*/,
     //    0x9200000/*(pstCmdqData->u32CmdqBufAddr + pstCmdqData->u32CmdqBufSize)*/, pstCmdqData->u32CmdqMiuNo);
    LOG_INFO("MDrv_CMDQ_Init.\n");
    MDrv_CMDQ_Init(0);
    MDrv_CMDQ_Get_Memory_Size(0x9100000, 0x9200000, 0);
    #endif
#endif
    return true;
}

//internal api
static bool _release_mem_for_cmdq(MHAL_CMDQ_CmdqInterface_t * pstCmdqInfo)
{
    FUN_ENTRY();
    // release cmdq hw buffer
    FUN_EXIT();
    return true;
}

static EN_CMDQTriggerBus_ID _transfer_wait_event_command(MHAL_CMDQ_EventId_e event_id)
{
    EN_CMDQTriggerBus_ID bus_id = NONE_USED_SIGNAL;
    switch (event_id)
    {
        case E_MHAL_CMDQEVE_DIP_TRIG:
            bus_id= XC_DIP_CMDQ_INT;
            break;
        default://not support
            bus_id= NONE_USED_SIGNAL;
            break;
    };
    return bus_id;
}

MS_S32 MHAL_CMDQ_GetNextMloadRingBufWritePtr_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_PHYADDR* phyWritePtr)
{
    //not support
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_UpdateMloadRingBufReadPtr_ByRiu(MHAL_CMDQ_CmdqInterface_t* pCmdinf, MS_PHYADDR phyReadPtr)
{
    //not support
    return MHAL_SUCCESS;
}
MS_S32 MHAL_CMDQ_MloadCopyBuf_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, void * MloadBuf, MS_U32 u32Size, MS_U16 u16Alignment, MS_PHYADDR *phyRetAddr)
{
    //not support
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_CheckBufAvailable_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32CmdqNum)
{
    MS_S16 s16Ret = 0;
    FUN_ENTRY();
    //do nothing
    FUN_EXIT();
    return s16Ret;
}

MS_S32 MHAL_CMDQ_WriteDummyRegCmdq_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16 u16Value)
{
    MS_S32 s32Ret = 0;
    MS_U16 u16WriteMask = 0xFFFF;
    MI_CMDQ_DATA *pstCmdqData = NULL;
    CAF_Struct  *pstCmdqCmd = NULL;
    FUN_ENTRY();

    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    pstCmdqCmd = &(pstCmdqData->pstCmdqCmdBuf[pstCmdqData->u32CmdqCmdIndex]);
    LOG_DBG(" MHAL_CMDQ_WriteRegCmdq index=%ld, u32RegAddr=%#lx, u16Value=%#x.\n", pstCmdqData->u32CmdqCmdIndex, pstCmdqData->u32DummyRegAddr, u16Value);
    CHECK_CMDQ_BUFFER_NUM(pstCmdqData);

    //cmdq write
    MUTEX_LOCK(pstCmdqData);
        //write reg

    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_ReadDummyRegCmdq_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16* u16RegVal)
{
    MS_S32 s32Ret = 0;
    MI_CMDQ_DATA *pstCmdqData = NULL;
    CAF_Struct  *pstCmdqCmd = NULL;
    FUN_ENTRY();

    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);

    LOG_INFO("u32DummyRegAddr=%#lx.\n", pstCmdqData->u32DummyRegAddr);
    //s32Ret = RIU_Read2Byte(pstCmdqData->u32DummyRegAddr);
    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_WriteRegCmdqMask_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value, MS_U16 u16WriteMask)
{
    MS_S32 s32Ret = 0;
    FUN_ENTRY();

    //RIU_WriteWordMask(u32RegAddr, u16Value, u16WriteMask);

    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_WriteRegCmdq_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value)
{
     MS_S32 s32Ret = 0;
     FUN_ENTRY();

    //fixme riu addr differeint with cmdq addr
    //RIU_Write2Byte(u32RegAddr, u16Value);

    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_CmdqPollRegBits_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value,  MS_U16 u16WriteMask, MS_BOOL bPollEq)
{
    MS_S32 s32Ret = 0;
    FUN_ENTRY();

    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_CmdqAddWaitEventCmd_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_EventId_e eEvent)
{
    MS_S32 s32Ret = 0;
    FUN_ENTRY();

    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_CmdqAbortBuffer_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    MS_S32 s32Ret = 0;
    MI_CMDQ_DATA *pstCmdqData;
    FUN_ENTRY();

    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    MUTEX_LOCK(pstCmdqData);

    //clear cmdq buffer and index
    memset(pstCmdqData->pstCmdqCmdBuf, 0, sizeof(CAF_Struct) * pstCmdqData->u32CmdqCmdIndex);
    s32Ret = pstCmdqData->u32CmdqCmdIndex;
    pstCmdqData->u32CmdqCmdIndex = 0;

    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_CmdqResetEngine_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_ReadStatusCmdq_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32* u32StatVal)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_KickOffCmdq_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    MS_S32 s32Ret = 0;
    FUN_ENTRY();

    //do nothing
    FUN_EXIT();
    return s32Ret;
}
MS_S32 MHAL_CMDQ_ClearTriggerEvent_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_IsCmdqEmptyIdle_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_BOOL* bIdleVal)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_CmdDelay_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf,MS_U32 uTimeNs)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_GetCurrentCmdqNumber_ByRiu(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_GetNextMloadRingBufWritePtr(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_PHYADDR* phyWritePtr)
{
    //not support
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_UpdateMloadRingBufReadPtr(MHAL_CMDQ_CmdqInterface_t* pCmdinf, MS_PHYADDR phyReadPtr)
{
    //not support
    return MHAL_SUCCESS;
}
MS_S32 MHAL_CMDQ_MloadCopyBuf(MHAL_CMDQ_CmdqInterface_t *pCmdinf, void * MloadBuf, MS_U32 u32Size, MS_U16 u16Alignment, MS_PHYADDR *phyRetAddr)
{
    //not support
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_CheckBufAvailable(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32CmdqNum)
{
    MS_S32 s16Ret = 0;
    MI_CMDQ_DATA *pstCmdqData = NULL;
    FUN_ENTRY();
    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    //CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    if (pstCmdqData->eCmdqId != E_MHAL_CMDQ_ID_DIVP)
    {
        return 1;
    }

    MUTEX_LOCK(pstCmdqData);
    //MDrv_CMDQv2_Get_FreeCmdLine_Size(0);
    s16Ret = (pstCmdqData->u32CmdqCmdMaxNum - pstCmdqData->u32CmdqCmdIndex - u32CmdqNum);
    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return s16Ret;
}

MS_S32 MHAL_CMDQ_ReadDummyRegCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16* u16RegVal)
{
    MS_S32 s32Ret = 0;
    MI_CMDQ_DATA *pstCmdqData = NULL;
    CAF_Struct  *pstCmdqCmd = NULL;
    FUN_ENTRY();

    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    if (pstCmdqData->eCmdqId != E_MHAL_CMDQ_ID_DIVP)
    {
        *u16RegVal = pstCmdqData->u32DummyRegValue;
    }
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);

    LOG_DBG("u32DummyRegAddr=%#lx.\n", pstCmdqData->u32DummyRegAddr);
    //s32Ret = RIU_Read2Byte(pstCmdqData->u32DummyRegAddr);
    *u16RegVal =  *((volatile MS_U32 *)(g_u32XcRiuBase + pstCmdqData->u32DummyRegAddr *2));
    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_WriteRegCmdqMask(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value, MS_U16 u16WriteMask)
{
    MS_S32 s32Ret = 0;
    MI_CMDQ_DATA *pstCmdqData;
    CAF_Struct  *pstCmdqCmd;
    FUN_ENTRY();

    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    pstCmdqCmd = &(pstCmdqData->pstCmdqCmdBuf[pstCmdqData->u32CmdqCmdIndex]);
    LOG_DBG(" write_reg_cmdq_maskindex=%ld u32RegAddr=%#lx, u16Value=%#x mask=%#x.\n", pstCmdqData->u32CmdqCmdIndex, u32RegAddr, u16Value, u16WriteMask);
    CHECK_CMDQ_BUFFER_NUM(pstCmdqData);

    //cmdq write
    MUTEX_LOCK(pstCmdqData);
    pstCmdqCmd->operation =  CMDQ_Write_RIU_Command;
    pstCmdqCmd->destionation_address =  u32RegAddr;
    pstCmdqCmd->destionation_value =  u16Value;
    pstCmdqCmd->mask =  ((~u16WriteMask)&0xFFFF);
    pstCmdqData->u32CmdqCmdIndex++;
    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_WriteRegCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value)
{
    MS_S32 s32Ret = 0;
    MS_U16 u16WriteMask =0xFFFF;
    MI_CMDQ_DATA *pstCmdqData;
    CAF_Struct  *pstCmdqCmd;
    FUN_ENTRY();

    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    pstCmdqCmd = &(pstCmdqData->pstCmdqCmdBuf[pstCmdqData->u32CmdqCmdIndex]);
    LOG_DBG(" MHAL_CMDQ_WriteRegCmdq index=%ld, u32RegAddr=%#lx, u16Value=%#x.\n", pstCmdqData->u32CmdqCmdIndex, u32RegAddr, u16Value);
    CHECK_CMDQ_BUFFER_NUM(pstCmdqData);

    //cmdq write
    MUTEX_LOCK(pstCmdqData);
    pstCmdqCmd->operation =  CMDQ_Write_RIU_Command;
    pstCmdqCmd->destionation_address =  u32RegAddr;
    pstCmdqCmd->destionation_value =  u16Value;
    pstCmdqCmd->mask =  ((~u16WriteMask)&0xFFFF);
    pstCmdqData->u32CmdqCmdIndex++;

    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return s32Ret;

}

MS_S32 MHAL_CMDQ_CmdqPollRegBits(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32 u32RegAddr, MS_U16 u16Value,  MS_U16 u16WriteMask, MS_BOOL bPollEq)
{
    MS_S32 s32Ret = 0;
    MI_CMDQ_DATA *pstCmdqData;
    CAF_Struct  *pstCmdqCmd;
    FUN_ENTRY();

    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    pstCmdqCmd = &(pstCmdqData->pstCmdqCmdBuf[pstCmdqData->u32CmdqCmdIndex]);
    CHECK_CMDQ_BUFFER_NUM(pstCmdqData);

    //cmdq write
    MUTEX_LOCK(pstCmdqData);
    pstCmdqCmd->operation = bPollEq ? CMDQ_Polling_RIU_EQ_Command : CMDQ_Polling_RIU_NEQ_Command;
    pstCmdqCmd->destionation_address =  u32RegAddr;
    pstCmdqCmd->destionation_value =  u16Value;
    pstCmdqCmd->mask =  ((~u16WriteMask)&0xFFFF);
    pstCmdqData->u32CmdqCmdIndex++;
    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return s32Ret;
}


//------------------------------------------------------------------------------
/// @brief command delay
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
///            MS_U32 uTimeNs : ns
/// @return < 0 : is failed
/// @
//------------------------------------------------------------------------------
MS_S32 MHAL_CMDQ_CmdDelay(MHAL_CMDQ_CmdqInterface_t *pCmdinf,MS_U32 uTimeNs)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    if(uTimeNs == 100*1000)///0.1ms
    {
        //patch for K6 start :
        //add delay(0.1ms) for each frame. solution is from tommy.wang.
        MHAL_CMDQ_WriteRegCmdq(pCmdinf, 0x123550, 0xAA);
        MHAL_CMDQ_WriteRegCmdq(pCmdinf, 0x123552, 0x8080);
        MHAL_CMDQ_CmdqPollRegBits(pCmdinf, 0x120332, 0x0000,  0xFFF, TRUE);
        MHAL_CMDQ_WriteRegCmdq(pCmdinf, 0x123550, 0x0);
        MHAL_CMDQ_WriteRegCmdq(pCmdinf, 0x123552, 0x8000);
        //patch for K6 end :
    }
    else
    {
        LOG_ERR("invalid time : %u ns\n", uTimeNs);
        s32Ret = MHAL_FAILURE;
    }
    return s32Ret;
}


MS_S32 MHAL_CMDQ_WriteDummyRegCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U16 u16Value)
{
    MS_S32 s32Ret = 0;
    MS_U16 u16WriteMask = 0xFFF;//notice : only 0~24bit is dumy register bits
    MI_CMDQ_DATA *pstCmdqData = NULL;
    CAF_Struct  *pstCmdqCmd = NULL;
    FUN_ENTRY();

    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    if (pstCmdqData->eCmdqId != E_MHAL_CMDQ_ID_DIVP)
    {
        pstCmdqData->u32DummyRegValue = u16Value;
    }
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    pstCmdqCmd = &(pstCmdqData->pstCmdqCmdBuf[pstCmdqData->u32CmdqCmdIndex]);
    LOG_DBG(" write_dummy_reg_cmdq index=%ld, u32RegAddr=%#lx, u16Value=%#x.\n", pstCmdqData->u32CmdqCmdIndex, pstCmdqData->u32DummyRegAddr, u16Value);
    CHECK_CMDQ_BUFFER_NUM(pstCmdqData);

    //cmdq write
    MUTEX_LOCK(pstCmdqData);
    pstCmdqCmd->operation =  CMDQ_Write_RIU_Command;
    pstCmdqCmd->destionation_address =  pstCmdqData->u32DummyRegAddr;//u32RegAddr;
    pstCmdqCmd->destionation_value =  u16Value;
    pstCmdqCmd->mask =  ((~u16WriteMask)&0xFFFF);
    pstCmdqData->u32CmdqCmdIndex++;
    pstCmdqData->u32DummyRegValue = u16Value;

    MUTEX_UNLOCK(pstCmdqData);
    MHAL_CMDQ_CmdDelay(pCmdinf, 100*1000);///100*1000ns = 100us = 0.1ms
    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_CmdqAddWaitEventCmd(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MHAL_CMDQ_EventId_e eEvent)
{
    MS_S32 s32Ret = 0;
    MI_CMDQ_DATA *pstCmdqData;
    CAF_Struct  *pstCmdqCmd;
    EN_CMDQTriggerBus_ID bus_id;
    FUN_ENTRY();


    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    MUTEX_LOCK(pstCmdqData);
    pstCmdqCmd = &pstCmdqData->pstCmdqCmdBuf[pstCmdqData->u32CmdqCmdIndex];
    CHECK_CMDQ_BUFFER_NUM(pstCmdqData);

    bus_id = _transfer_wait_event_command((MHAL_CMDQ_EventId_e)(eEvent));
    MDrv_CMDQ_Gen_WaitTrigger_Bus_Command(pstCmdqCmd, bus_id, FALSE);//eEvent = XC_DIP_CMDQ_INT
    pstCmdqData->u32CmdqCmdIndex++;

    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_CmdqAbortBuffer(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    MS_S32 s32Ret = 0;
    MI_CMDQ_DATA *pstCmdqData;
    FUN_ENTRY();

    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    MUTEX_LOCK(pstCmdqData);

    //clear cmdq buffer and index
    memset(pstCmdqData->pstCmdqCmdBuf, 0, sizeof(CAF_Struct) * pstCmdqData->u32CmdqCmdIndex);
    s32Ret = pstCmdqData->u32CmdqCmdIndex;
    pstCmdqData->u32CmdqCmdIndex = 0;

    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_CmdqResetEngine(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_ReadStatusCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_U32* u32StatVal)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_KickOffCmdq(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    MS_S32 s32Ret = 0;
    CH_Struct stCh;
    MI_CMDQ_DATA *pstCmdqData;
    FUN_ENTRY();


    ASSERT(pCmdinf != NULL);
    pstCmdqData = (MI_CMDQ_DATA *)pCmdinf->pCtx;
    ASSERT(pstCmdqData != NULL);
    CHECK_CMDQ_ID(pstCmdqData->eCmdqId);
    MUTEX_LOCK(pstCmdqData);
    stCh.Pointer_To_CAFArray = pstCmdqData->pstCmdqCmdBuf;
    stCh.Command_Number = pstCmdqData->u32CmdqCmdIndex;

    //check buffer avaliable
    LOG_DBG("kick_off_cmdq index=%ld .\n", pstCmdqData->u32CmdqCmdIndex);
    //write command
    MS_S32 uRetryNum = 20;
    while (DRVCMDQ_OK != MDrv_CMDQ_Receive(&stCh) && (uRetryNum--)) {
        // print debug log
        LOG_ERR("cmdq_srv warning: kick_off_cmdq buffer is not enough.\n");
        udelay(2*1000); //wait 2ms
    }

    //clear temp cmdq buffer
    memset(pstCmdqData->pstCmdqCmdBuf, 0, sizeof(CAF_Struct) * pstCmdqData->u32CmdqCmdIndex);
    s32Ret = pstCmdqData->u32CmdqCmdIndex;
    pstCmdqData->u32CmdqCmdIndex = 0;
    MUTEX_UNLOCK(pstCmdqData);
    FUN_EXIT();
    return s32Ret;
}

MS_S32 MHAL_CMDQ_ClearTriggerEvent(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_CMDQ_IsCmdqEmptyIdle(MHAL_CMDQ_CmdqInterface_t *pCmdinf, MS_BOOL* bIdleVal)
{
    return MHAL_SUCCESS;
}

//------------------------------------------------------------------------------
/// @brief Get current cmdq number
/// @param[in] MHAL_CMDQ_CmdqInterface_t *pCmdinf : CMDQ interface
/// @return : cmdq number
/// @
//------------------------------------------------------------------------------
MS_U32 MHAL_CMDQ_GetCurrentCmdqNumber(MHAL_CMDQ_CmdqInterface_t *pCmdinf)
{
    MS_S32 s32Ret = MHAL_FAILURE;
    LOG_ERR("this interface is empty!!!\n");
    return s32Ret;
}

MS_S32 MHAL_CMDQ_InitCmdqMmapInfo(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo)
{
    int i = 0;
    for(i = 0; i < E_MHAL_CMDQ_ID_MAX; i++)
    {
        memset(&g_stCmdqOps[i], 0, sizeof(MHAL_CMDQ_CmdqInterface_t));
        pCmdqOps[i] = NULL;
    }

    LOG_INFO("u32Addr=%#x, u32Size=%#x KernelVa=%#x.\n", pCmdqMmapInfo->u32CmdqMmapPhyAddr, pCmdqMmapInfo->u32CmdqMmapSize, pCmdqMmapInfo->u32CmdqMmapVirAddr);
    return MHAL_SUCCESS;
}
MHAL_CMDQ_CmdqInterface_t *MHAL_CMDQ_GetSysCmdqService(MHAL_CMDQ_Id_e eCmdqId, MHAL_CMDQ_BufDescript_t *pCmdqBufDesp, MS_BOOL bForceRIU)
{
    MI_CMDQ_DATA *pstCmdqData;
    FUN_ENTRY();
    LOG_INFO("cmdq_srv get_sys_cmdq_service entry, pCmdqOps[%u]=%p.\n", eCmdqId, pCmdqOps[eCmdqId]);
    ASSERT(pCmdqBufDesp);
    if (eCmdqId > E_MHAL_CMDQ_ID_MAX) {
        LOG_ERR("cmdq_srv error, cmdqId=%d.\n", eCmdqId);
        return NULL;
    }

    if (pCmdqOps[eCmdqId] == NULL) {
        MHAL_CMDQ_CmdqInterface_t *pCmdq = NULL;//pCmdqOps[cmdqId];
        if (eCmdqId == E_MHAL_CMDQ_ID_DIVP) {
            //init variable
            pCmdq = &g_stCmdqOps[E_MHAL_CMDQ_ID_DIVP];///(MHAL_CMDQ_CmdqInterface_t *)kmalloc(sizeof(MHAL_CMDQ_CmdqInterface_t), GFP_KERNEL);//MI_CMDQ_DATA stCmdqData;
            pstCmdqData = (MI_CMDQ_DATA *)kmalloc(sizeof(MI_CMDQ_DATA), GFP_KERNEL);//MI_CMDQ_DATA stCmdqData;
            ASSERT(pCmdq != NULL);
            ASSERT(pstCmdqData != NULL);

            pCmdq->pCtx = pstCmdqData;//MI_CMDQ_DATA stCmdqData;
            memset(pstCmdqData, 0, sizeof(MI_CMDQ_DATA));
            pstCmdqData->eCmdqId = eCmdqId;
            pstCmdqData->u32CmdqCmdMaxNum = pCmdqBufDesp->u32CmdqBufSize/8; //one command is 8 bytes
            pstCmdqData->bForceRIU = bForceRIU;
            pstCmdqData->u32DummyRegAddr = 0x120332;//MS_U32  g_u32Wait_Reg[MAX_DIP_WINDOW] = {0x120332,0x120334,0x120338};
            pstCmdqData->u32DummyRegValue = 0;

            // allocate memory to store temp command.
            pstCmdqData->u32CmdqCmdIndex = 0;
            pstCmdqData->pstCmdqCmdBuf = (CAF_Struct *)kmalloc(pCmdqBufDesp->u32CmdqBufSize, GFP_KERNEL);
            ASSERT(pstCmdqData->pstCmdqCmdBuf != NULL);
            memset(pstCmdqData->pstCmdqCmdBuf, 0, pCmdqBufDesp->u32CmdqBufSize);
            //pthread_mutex_init(&pstCmdqData->cmdqMutex,NULL);

            MS_PHYADDR u32PMBankSize = 0;
            MDrv_MMIO_Init();
            if(MDrv_MMIO_GetBASE( &pstCmdqData->u32XcRiuBase, &u32PMBankSize, MS_MODULE_PM) != TRUE)
            {
                LOG_ERR( "MDrv_MMIO_GetBASE Get PM BASE failure\n");
                return NULL;
            }
            g_u32XcRiuBase = pstCmdqData->u32XcRiuBase;
            LOG_INFO("MMIO u32XcRiuBase=%#lx.\n", g_u32XcRiuBase);
            LOG_INFO("_XC_RIU_BASE = %#lx.\n", _XC_RIU_BASE);

            if (bForceRIU) {
                pCmdq->MHAL_CMDQ_GetNextMloadRingBufWritePtr     = MHAL_CMDQ_GetNextMloadRingBufWritePtr_ByRiu;
                pCmdq->MHAL_CMDQ_UpdateMloadRingBufReadPtr      = MHAL_CMDQ_UpdateMloadRingBufReadPtr_ByRiu;
                pCmdq->MHAL_CMDQ_MloadCopyBuf                   = MHAL_CMDQ_MloadCopyBuf_ByRiu;
                pCmdq->MHAL_CMDQ_CheckBufAvailable              = MHAL_CMDQ_CheckBufAvailable_ByRiu;
                pCmdq->MHAL_CMDQ_WriteRegCmdqMask               = MHAL_CMDQ_WriteRegCmdqMask_ByRiu;
                pCmdq->MHAL_CMDQ_WriteRegCmdq                   = MHAL_CMDQ_WriteRegCmdq_ByRiu;
                pCmdq->MHAL_CMDQ_CmdqPollRegBits                = MHAL_CMDQ_CmdqPollRegBits_ByRiu;
                pCmdq->MHAL_CMDQ_CmdqAddWaitEventCmd            = MHAL_CMDQ_CmdqAddWaitEventCmd_ByRiu;
                pCmdq->MHAL_CMDQ_WriteDummyRegCmdq              = MHAL_CMDQ_WriteDummyRegCmdq_ByRiu;
                pCmdq->MHAL_CMDQ_ReadDummyRegCmdq               = MHAL_CMDQ_ReadDummyRegCmdq_ByRiu;
                pCmdq->MHAL_CMDQ_KickOffCmdq                    = MHAL_CMDQ_KickOffCmdq_ByRiu;
                pCmdq->MHAL_CMDQ_IsCmdqEmptyIdle                = MHAL_CMDQ_IsCmdqEmptyIdle_ByRiu;
                pCmdq->MHAL_CMDQ_CmdqAbortBuffer                = MHAL_CMDQ_CmdqAbortBuffer_ByRiu;
                pCmdq->MHAL_CMDQ_ReadStatusCmdq                 = MHAL_CMDQ_ReadStatusCmdq_ByRiu;
                pCmdq->MHAL_CMDQ_ClearTriggerEvent              = MHAL_CMDQ_ClearTriggerEvent_ByRiu;
                pCmdq->MHAL_CMDQ_CmdqResetEngine                = MHAL_CMDQ_CmdqResetEngine_ByRiu;
                pCmdq->MHAL_CMDQ_CmdDelay                          = MHAL_CMDQ_CmdDelay_ByRiu;
                pCmdq->MHAL_CMDQ_GetCurrentCmdqNumber        = MHAL_CMDQ_GetCurrentCmdqNumber_ByRiu;
            } else {
                pCmdq->MHAL_CMDQ_GetNextMloadRingBufWritePtr     = MHAL_CMDQ_GetNextMloadRingBufWritePtr;
                pCmdq->MHAL_CMDQ_UpdateMloadRingBufReadPtr      = MHAL_CMDQ_UpdateMloadRingBufReadPtr;
                pCmdq->MHAL_CMDQ_MloadCopyBuf                   = MHAL_CMDQ_MloadCopyBuf;
                pCmdq->MHAL_CMDQ_CheckBufAvailable              = MHAL_CMDQ_CheckBufAvailable;
                pCmdq->MHAL_CMDQ_WriteRegCmdqMask               = MHAL_CMDQ_WriteRegCmdqMask;
                pCmdq->MHAL_CMDQ_WriteRegCmdq                   = MHAL_CMDQ_WriteRegCmdq;
                pCmdq->MHAL_CMDQ_CmdqPollRegBits                = MHAL_CMDQ_CmdqPollRegBits;
                pCmdq->MHAL_CMDQ_CmdqAddWaitEventCmd            = MHAL_CMDQ_CmdqAddWaitEventCmd;
                pCmdq->MHAL_CMDQ_WriteDummyRegCmdq              = MHAL_CMDQ_WriteDummyRegCmdq;
                pCmdq->MHAL_CMDQ_ReadDummyRegCmdq               = MHAL_CMDQ_ReadDummyRegCmdq;
                pCmdq->MHAL_CMDQ_KickOffCmdq                    = MHAL_CMDQ_KickOffCmdq;
                pCmdq->MHAL_CMDQ_IsCmdqEmptyIdle                = MHAL_CMDQ_IsCmdqEmptyIdle;
                pCmdq->MHAL_CMDQ_CmdqAbortBuffer                = MHAL_CMDQ_CmdqAbortBuffer;
                pCmdq->MHAL_CMDQ_ReadStatusCmdq                 = MHAL_CMDQ_ReadStatusCmdq;
                pCmdq->MHAL_CMDQ_ClearTriggerEvent              = MHAL_CMDQ_ClearTriggerEvent;
                pCmdq->MHAL_CMDQ_CmdqResetEngine                = MHAL_CMDQ_CmdqResetEngine;
                pCmdq->MHAL_CMDQ_CmdDelay                          = MHAL_CMDQ_CmdDelay;
                pCmdq->MHAL_CMDQ_GetCurrentCmdqNumber        = MHAL_CMDQ_GetCurrentCmdqNumber;
            }
            _allocate_mem_for_cmdq(pCmdq, pCmdqBufDesp);
            pCmdqOps[eCmdqId] = pCmdq;
            LOG_INFO("pCmdqOps[%u]=%#p, write=%#p.\n", eCmdqId, pCmdqOps[eCmdqId],pCmdq->MHAL_CMDQ_WriteRegCmdq);
        }
        else if(eCmdqId == E_MHAL_CMDQ_ID_VPE)
        {
            //fix
            pCmdq = &g_stCmdqOps[E_MHAL_CMDQ_ID_VPE];
            pCmdq->MHAL_CMDQ_GetNextMloadRingBufWritePtr     = MHAL_CMDQ_GetNextMloadRingBufWritePtr;
            pCmdq->MHAL_CMDQ_UpdateMloadRingBufReadPtr      = MHAL_CMDQ_UpdateMloadRingBufReadPtr;
            pCmdq->MHAL_CMDQ_MloadCopyBuf                   = MHAL_CMDQ_MloadCopyBuf;
            pCmdq->MHAL_CMDQ_CheckBufAvailable              = MHAL_CMDQ_CheckBufAvailable;
            pCmdq->MHAL_CMDQ_WriteRegCmdqMask               = MHAL_CMDQ_WriteRegCmdqMask;
            pCmdq->MHAL_CMDQ_WriteRegCmdq                   = MHAL_CMDQ_WriteRegCmdq;
            pCmdq->MHAL_CMDQ_CmdqPollRegBits                = MHAL_CMDQ_CmdqPollRegBits;
            pCmdq->MHAL_CMDQ_CmdqAddWaitEventCmd            = MHAL_CMDQ_CmdqAddWaitEventCmd;
            pCmdq->MHAL_CMDQ_WriteDummyRegCmdq              = MHAL_CMDQ_WriteDummyRegCmdq;
            pCmdq->MHAL_CMDQ_ReadDummyRegCmdq               = MHAL_CMDQ_ReadDummyRegCmdq;
            pCmdq->MHAL_CMDQ_KickOffCmdq                    = MHAL_CMDQ_KickOffCmdq;
            pCmdq->MHAL_CMDQ_IsCmdqEmptyIdle                = MHAL_CMDQ_IsCmdqEmptyIdle;
            pCmdq->MHAL_CMDQ_CmdqAbortBuffer                = MHAL_CMDQ_CmdqAbortBuffer;
            pCmdq->MHAL_CMDQ_ReadStatusCmdq                 = MHAL_CMDQ_ReadStatusCmdq;
            pCmdq->MHAL_CMDQ_ClearTriggerEvent              = MHAL_CMDQ_ClearTriggerEvent;
            pCmdq->MHAL_CMDQ_CmdqResetEngine                = MHAL_CMDQ_CmdqResetEngine;
            pCmdq->MHAL_CMDQ_CmdDelay                          = MHAL_CMDQ_CmdDelay;
            pCmdq->MHAL_CMDQ_GetCurrentCmdqNumber        = MHAL_CMDQ_GetCurrentCmdqNumber;
            //pCmdq->pCtx =

            pstCmdqData = (MI_CMDQ_DATA *)kmalloc(sizeof(MI_CMDQ_DATA), GFP_KERNEL);//MI_CMDQ_DATA stCmdqData;
            ASSERT(pCmdq != NULL);
            ASSERT(pstCmdqData != NULL);

            pCmdq->pCtx = pstCmdqData;//MI_CMDQ_DATA stCmdqData;
            memset(pstCmdqData, 0, sizeof(MI_CMDQ_DATA));
            pstCmdqData->eCmdqId = eCmdqId;
            pCmdqOps[eCmdqId] = pCmdq;
        }
        else{
            LOG_ERR("not support cmdqId=%d.\n", eCmdqId);
        }
    }
    FUN_EXIT();
    return pCmdqOps[eCmdqId];
}

void MHAL_CMDQ_ReleaseSysCmdqService(MHAL_CMDQ_Id_e eCmdqId)
{
    FUN_ENTRY();
    //release resource
    LOG_INFO("release cmdq, pCmdqOps[%u]=%p.\n", eCmdqId, pCmdqOps[eCmdqId]);
    if (pCmdqOps[eCmdqId] != NULL) {
        MI_CMDQ_DATA *pstCmdqData = (MI_CMDQ_DATA *)pCmdqOps[eCmdqId]->pCtx;
        if (pstCmdqData != NULL) {
            //release cmdq hw memory
            _release_mem_for_cmdq(pCmdqOps[eCmdqId]);

            //close MMIO
            //MDrv_MMIO_Close();

            //release private resource
            if (pstCmdqData->pstCmdqCmdBuf != NULL) {
                kfree(pstCmdqData->pstCmdqCmdBuf);
                pstCmdqData->pstCmdqCmdBuf = NULL;
            }
            //pthread_mutex_destroy(&pstCmdqData->cmdqMutex);

            if (pstCmdqData->eCmdqId == E_MHAL_CMDQ_ID_DIVP)
            {
                MDrv_CMDQ_Exit();
            }

            kfree(pstCmdqData);
            pstCmdqData = NULL;
        }
        memset(&g_stCmdqOps[eCmdqId], 0, sizeof(MHAL_CMDQ_CmdqInterface_t));
        pCmdqOps[eCmdqId] = NULL;
    } else {

        LOG_ERR("cmdq_srv has already release, pCmdqOps[%u]=%p.\n", eCmdqId, pCmdqOps[eCmdqId]);
    }

    FUN_EXIT();
    return ;
}


EXPORT_SYMBOL(MHAL_CMDQ_InitCmdqMmapInfo);
EXPORT_SYMBOL(MHAL_CMDQ_GetSysCmdqService);
EXPORT_SYMBOL(MHAL_CMDQ_ReleaseSysCmdqService);
