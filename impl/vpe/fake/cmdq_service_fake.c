#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
///#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>

//#include "mi_print.h"
#include "mhal_common.h"
#include "mi_common_datatype.h"

#include "mhal_cmdq.h"
typedef struct {
    MHAL_CMDQ_CmdqInterface_t    stInf;
    MHAL_CMDQ_BufDescript_t stCmdqBufDesc;
    MS_BOOL                bForceRiu;
    MHAL_CMDQ_Id_e              cmdqId;
} CmdQService_t;

#define INIT_CMDQ_SERVICE(CMDQ_ID) \
{\
    .stInf = {\
        .MHAL_CMDQ_ReadDummyRegCmdq = fake_cmdq_read_dummy_reg_cmdq, \
        .MHAL_CMDQ_IsCmdqEmptyIdle = fake_cmdq_is_cmdq_empty_idle, \
        .MHAL_CMDQ_UpdateMloadRingBufReadPtr = fake_cmdq_update_mload_ringbuf_read_ptr, \
        .MHAL_CMDQ_CheckBufAvailable = fake_cmdq_check_buf_available, \
        .MHAL_CMDQ_WriteDummyRegCmdq = fake_cmdq_write_dummy_reg_cmdq, \
        .MHAL_CMDQ_KickOffCmdq = fake_cmdq_kick_off_cmdq, \
        .MHAL_CMDQ_GetNextMloadRingBufWritePtr = fake_cmdq_get_next_mload_ringbuf_write_ptr,\
        .pCtx = NULL,\
    },\
    .cmdqId = (CMDQ_ID),\
}

//#define DBG_INFO(fmt, args...) do {printk("[CMDQ INF] "); printk(fmt, ##args);} while(0)
#define DBG_INFO(fmt, args...)
#define DBG_ERR(fmt, args...)  do {printk("[CMDQ ERR] "); printk(fmt, ##args);} while(0)

static inline void showCmdInf(const char* func, MI_U32 u32Line, MHAL_CMDQ_CmdqInterface_t *cmdinf)
{
//    CmdQService_t *pstCmdQService = container_of(cmdinf, CmdQService_t, stInf);
//    DBG_INFO("%s()@line %d CMD_ID: %d bForceRiu: %d.\n", func, u32Line, pstCmdQService->cmdqId, pstCmdQService->bForceRiu);
}
#define test(cmdinf) showCmdInf(__func__, __LINE__, (cmdinf))
MS_U16 _gu16Dummy = 0;
static MS_S32 fake_cmdq_read_dummy_reg_cmdq(MHAL_CMDQ_CmdqInterface_t *cmdinf, MS_U16 *pu16Var)
{
    MS_S32 s32Ret  = -1;
    //MHAL_CMDQ_CmdqInterface_t *cmdinf
    test(cmdinf);

    *pu16Var = _gu16Dummy;
    DBG_INFO("%s@line %d sucess: cmdinf: %p _gu16Dummy: 0x%x!!!\n", __func__, __LINE__, cmdinf, _gu16Dummy);
    s32Ret = MHAL_SUCCESS;
    return s32Ret;
}

static MS_S32 fake_cmdq_is_cmdq_empty_idle(MHAL_CMDQ_CmdqInterface_t *cmdinf, MS_BOOL *pbIdle)
{
    MS_S32 s32Ret  = -1;
    //MHAL_CMDQ_CmdqInterface_t *cmdinf
    test(cmdinf);

    DBG_INFO("%s@line %d sucess: cmdinf: %p !!!\n", __func__, __LINE__, cmdinf);
    *pbIdle = FALSE;
    s32Ret = MHAL_SUCCESS;
    return s32Ret;
}

//static MS_PHYADDR fake_cmdq_update_mload_ringbuf_read_ptr(MHAL_CMDQ_CmdqInterface_t *cmdinf, MS_PHYADDR next_read_ptr)
static MS_S32 fake_cmdq_update_mload_ringbuf_read_ptr(MHAL_CMDQ_CmdqInterface_t* cmdinf, MS_PHYADDR phyReadPtr)
{
    MS_S32 s32Ret  = -1;
    //MHAL_CMDQ_CmdqInterface_t *cmdinf, MS_PHYADDR next_read_ptr
    test(cmdinf);

    DBG_INFO("%s@line %d sucess: cmdinf: %p !!!\n", __func__, __LINE__, cmdinf);
    s32Ret = MHAL_SUCCESS;
    return s32Ret;
}

static MS_S32 fake_cmdq_check_buf_available(MHAL_CMDQ_CmdqInterface_t *cmdinf , MS_U32 cmdq_num)
{
    MS_S32 s32Ret  = -1;
    //MHAL_CMDQ_CmdqInterface_t *cmdinf , MS_U32 cmdq_num
    test(cmdinf);

    DBG_INFO("%s@line %d sucess: cmdinf: %p !!!\n", __func__, __LINE__, cmdinf);
    s32Ret = (MS_S32)cmdq_num;
    return s32Ret;
}

static MS_S32 fake_cmdq_write_dummy_reg_cmdq(MHAL_CMDQ_CmdqInterface_t *cmdinf, MS_U16 value)
{
    MS_S32 s32Ret  = -1;
    //MHAL_CMDQ_CmdqInterface_t *cmdinf, MS_U16 value
    test(cmdinf);
    _gu16Dummy = value;

    DBG_INFO("%s@line %d sucess: cmdinf: %p 0x%x!!!\n", __func__, __LINE__, cmdinf, _gu16Dummy);

    s32Ret = MHAL_SUCCESS;
    return s32Ret;
}

static MS_S32 fake_cmdq_kick_off_cmdq(MHAL_CMDQ_CmdqInterface_t *cmdinf)
{
    MS_S32 s32Ret  = -1;
    //MHAL_CMDQ_CmdqInterface_t *cmdinf
    test(cmdinf);

    DBG_INFO("%s@line %d sucess: cmdinf: %p !!!\n", __func__, __LINE__, cmdinf);
    s32Ret = MHAL_SUCCESS;
    return s32Ret;
}

static MS_S32 fake_cmdq_get_next_mload_ringbuf_write_ptr(MHAL_CMDQ_CmdqInterface_t *cmdinf, MS_PHYADDR* phyWritePtr)
{
    MS_S32 s32Ret  = -1;
    //MHAL_CMDQ_CmdqInterface_t *cmdinf
    test(cmdinf);

    DBG_INFO("%s@line %d sucess: cmdinf: %p !!!\n", __func__, __LINE__, cmdinf);
    s32Ret = MHAL_SUCCESS;
    return s32Ret;
}

static CmdQService_t _gCmdQService[E_MHAL_CMDQ_ID_MAX] = {
    INIT_CMDQ_SERVICE(E_MHAL_CMDQ_ID_VPE),
    INIT_CMDQ_SERVICE(E_MHAL_CMDQ_ID_DIVP),
    INIT_CMDQ_SERVICE(E_MHAL_CMDQ_ID_H265_VENC0),
    INIT_CMDQ_SERVICE(E_MHAL_CMDQ_ID_H265_VENC1),
    INIT_CMDQ_SERVICE(E_MHAL_CMDQ_ID_H264_VENC0),
};

MHAL_CMDQ_CmdqInterface_t *MHAL_CMDQ_GetSysCmdqService(MHAL_CMDQ_Id_e eCmdqId, MHAL_CMDQ_BufDescript_t *pCmdqBufDesp, MS_BOOL bForceRIU)
{
    MHAL_CMDQ_CmdqInterface_t *pstCmdqInf = NULL;
    CmdQService_t *pstCmdqService   = NULL;
    if ((eCmdqId >= 0) && (eCmdqId < E_MHAL_CMDQ_ID_MAX))
    {
        pstCmdqService = &_gCmdQService[eCmdqId];
        memcpy(&pstCmdqService->stCmdqBufDesc, pCmdqBufDesp, sizeof(*pCmdqBufDesp));
        pstCmdqService->stInf.pCtx = &pstCmdqService->stCmdqBufDesc;
        pstCmdqService->bForceRiu = bForceRIU;
        pstCmdqInf = &pstCmdqService->stInf;
    }
    return pstCmdqInf;
}

void MHAL_CMDQ_ReleaseSysCmdqService(MHAL_CMDQ_Id_e cmdqId)
{
    return;
}

