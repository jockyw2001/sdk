#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "mhal_warp.h"
#include "mhal_cmdq.h"
//#include "mhal_warp_datatype.h"





static u32 g_u32LogLevel = 1;
#define LOG_TAG "[warp]"
#define LOG printk
#define FUN_ENTRY() {if(g_u32LogLevel & (1<<4)) LOG("%s:%d"LOG_TAG" entry >>>>>.\n", __FUNCTION__, __LINE__);}
#define FUN_EXIT() {if(g_u32LogLevel & (1<<4)) LOG("%s:%d"LOG_TAG" exit  <<<<<.\n", __FUNCTION__, __LINE__);}
#define LOG_DBG(x,...) {if(g_u32LogLevel & (1<<3)) LOG("%s:%d"LOG_TAG""x"\n", __FUNCTION__, __LINE__,  ##__VA_ARGS__);}
#define LOG_WARN(x,...) {if(g_u32LogLevel & (1<<2)) LOG("%s:%d"LOG_TAG" "x"",__FUNCTION__, __LINE__,  ##__VA_ARGS__);}
#define LOG_INFO(x,...) {if(g_u32LogLevel & (1<<1)) LOG("%s:%d"LOG_TAG" "x"",__FUNCTION__, __LINE__,  ##__VA_ARGS__);}
#define LOG_ERR(x,...) do{if(g_u32LogLevel & (1<<0)) LOG("%s:%d"LOG_TAG" error:"x"",__FUNCTION__, __LINE__,  ##__VA_ARGS__);}while(0)

typedef struct MHAL_WARP_Instance_s
{
    struct list_head instanceLst;
    MS_U32 u32DevId;
    MS_U32 u32ChnId;
}MHAL_WARP_Instance_t;

typedef struct MHAL_WARP_Dev_s
{
    struct list_head devLst;
    struct list_head instanceLst;
    MS_U32 u32DevId;
    MS_U32 u32InstCnt;
}MHAL_WARP_Dev_t;

typedef struct MHAL_WARP_DevHead_s
{
    struct list_head devLst;
    MS_U32 u32DevCnt;
}MHAL_WARP_DevHead_t;

static MHAL_WARP_DevHead_t _stWarpDevHead = {
    .devLst = {&_stWarpDevHead.devLst, &_stWarpDevHead.devLst},
    .u32DevCnt = 0,
};

//------------------------------------------------------------------------------
/// @brief Get Device Handle
/// @param[in]  id: Device Core ID.
/// @param[out] device: Pointer to device handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_CreateDevice(MS_U32 id, MHAL_WARP_DEV_HANDLE *device)
{
//    MS_S32 rt = MHAL_SUCCESS;
    MHAL_WARP_Dev_t *pstDev;
    struct list_head *pos;
    if(id >= MHAL_WARP_MAX_DEVICE_NUM)
    {
        LOG_ERR("devId(%d) is not support.\n", id);
        return MHAL_FAILURE;
    }

    list_for_each(pos, &_stWarpDevHead.devLst)
    {
        pstDev = container_of(pos, MHAL_WARP_Dev_t, devLst);
        if (pstDev->u32DevId == id)
        {
            LOG_ERR("devId(%d) has been created.\n", id);
            //return MHAL_FAILURE;
            return MHAL_SUCCESS;
        }
    }

    pstDev = (MHAL_WARP_Dev_t*)kmalloc(sizeof(MHAL_WARP_Dev_t), GFP_ATOMIC);
    memset(pstDev, 0, sizeof(MHAL_WARP_Dev_t));
    pstDev->u32DevId = id;
    INIT_LIST_HEAD(&pstDev->instanceLst);
    list_add_tail(&pstDev->devLst, &_stWarpDevHead.devLst);
    _stWarpDevHead.u32DevCnt++;

    *device = pstDev;
    return MHAL_SUCCESS;
}

//------------------------------------------------------------------------------
/// @brief Destroy Device Handle
/// @param[in]  device: Device handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_DestroyDevice(MHAL_WARP_DEV_HANDLE device)
{
    MS_S32 rt = MHAL_SUCCESS;
    MHAL_WARP_Dev_t *pstDev = (MHAL_WARP_Dev_t*)device;

    if (pstDev)
    {
        list_del(&pstDev->devLst);
        kfree(pstDev);
        _stWarpDevHead.u32DevCnt--;
    }
//    rt = CevaXM6DeInit(devId)  == TRUE? MHAL_SUCCESS: MHAL_FAILURE;
    return rt ;
}

//------------------------------------------------------------------------------
/// @brief Get Instance Handle
/// @param[in]  device: Device handle.
/// @param[out] instance: Pointer to instance handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_CreateInstance(MHAL_WARP_DEV_HANDLE device, MHAL_WARP_INST_HANDLE *instance)
{
    MS_S32 rt = MHAL_SUCCESS;

    //*instance = CevaXM6CreateInstance(device);
    return rt;
}

//------------------------------------------------------------------------------
/// @brief Destroy Instance Handle
/// @param[in]  instance: Instance handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_DestroyInstance(MHAL_WARP_INST_HANDLE instance)
{
    MS_S32 rt = MHAL_SUCCESS;

    //rt = CevaXM6DestroyInstance(instance);
    return rt;
}

//------------------------------------------------------------------------------
/// @brief Trigger Wrapper
/// @param[in]  instance: Instance handle.
/// @param[in]  config: configuration.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
//MS_S32 MHAL_WARP_Trigger(MHAL_WARP_INST_HANDLE instance, MHAL_WARP_CONFIG* config)
MS_S32 MHAL_WARP_Trigger(MHAL_WARP_INST_HANDLE instance, MHAL_WARP_CONFIG* config, MHAL_WARP_CALLBACK pfCallback, void *pUsrData)
{
    MS_S32 rt = MHAL_SUCCESS;

    //rt = CevaXM6SetConfig(instance, config);
    return rt;
}

//------------------------------------------------------------------------------
/// @brief Post Process after Warp ISR Process Done
/// @param[in]  instance: Instance handle.
/// @param[out] pstEncRet: Pointer to encode result.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_PostProc(MHAL_WARP_INST_HANDLE instance, void *pResult)
{
    MS_S32 rt = MHAL_SUCCESS;

    //rt = CevaXM6PostProc(instance, pResult);
    return rt;
}

//------------------------------------------------------------------------------
/// @brief Device Interrupt Processing
/// @param[in]  device: Device handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MHAL_WARP_ISR_STATE_E MHAL_WARP_IsrProc(MHAL_WARP_DEV_HANDLE device)
{
    MHAL_WARP_ISR_STATE_E rt = MHAL_WARP_ISR_STATE_DONE;

    //rt = CevaXM6IsrProc(device);
    return rt;
}

MS_S32 MHAL_WARP_GetIrqNum(MS_U32 *pu32IrqNum)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
//    *pu32IrqNum = INT_IRQ_78_WARP2RIU_INT;      // irqNo. undefined
    return s32Ret;
}

MS_S32 MHAL_WARP_EnableIsr(MS_BOOL bEnable)
{
    if (bEnable)
    {
//        enable_irq(INT_IRQ_78_WARP2RIU_INT);
    }
    else
    {
//        disable_irq(INT_IRQ_78_WARP2RIU_INT);
    }

    return MHAL_SUCCESS;
}


EXPORT_SYMBOL(MHAL_WARP_CreateDevice);
EXPORT_SYMBOL(MHAL_WARP_DestroyDevice);
EXPORT_SYMBOL(MHAL_WARP_CreateInstance);
EXPORT_SYMBOL(MHAL_WARP_DestroyInstance);
EXPORT_SYMBOL(MHAL_WARP_Trigger);
EXPORT_SYMBOL(MHAL_WARP_PostProc);
EXPORT_SYMBOL(MHAL_WARP_IsrProc);
EXPORT_SYMBOL(MHAL_WARP_GetIrqNum);
EXPORT_SYMBOL(MHAL_WARP_EnableIsr);





