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

#include "mi_sys.h"
#include "mi_sys_internal.h"
#include "mi_print.h"
#include "mi_sys_datatype.h"
#define E_MI_ERR_BUSY (2)
#define MI_SUCCESS (0)
#define SYS_TEST_MAX  (1)
#define CHNN_TEST_MAX (64)
#define PORT_TEST_MAX (4)

#define MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle) ((miSysDrvHandle) != &_gstDevFake)
#define MI_SYS_CHECK_CHNN_FAILED(u32ChnId) ((u32ChnId) < 0 || (u32ChnId) > CHNN_TEST_MAX)
#define MI_SYS_CHECK_PORT_FAILED(u32PortId) ((u32PortId) < 0 || (u32PortId) > PORT_TEST_MAX)

typedef struct {
    //struct list_head node;
    MI_BOOL bEnable;
    MI_U32  u32PortId;
}mi_sys_fakePortInfo_t;

typedef struct {
    MI_BOOL bEnable;
    //struct list_head node;
    MI_U32 u32InputPortNum;
    MI_U32 u32OutputPortNum;
    mi_sys_fakePortInfo_t *pstInputPort;
    mi_sys_fakePortInfo_t *pstOutputPort;
} mi_sys_fakeChannelInfo_t;

typedef struct {
    MI_BOOL bInited;
    MI_U32 u32Count;
    MI_U32 u32ChnnNum;
    mi_sys_fakeChannelInfo_t *pstChnn;
} mi_sys_fakeDevInfo_t;

static mi_sys_fakeDevInfo_t _gstDevFake = {FALSE, 0, 0, NULL};
static MI_BOOL _gBufferUesd[3] = {FALSE, FALSE, FALSE};
static MI_SYS_BufInfo_t _gBuffInfo[3] = {
    {
        .eBufType = E_MI_SYS_BUFDATA_FRAME,
        .u64Pts   = 0x12345678,
        .bEndOfStream = TRUE,
        .bUsrBuf      = FALSE,
        .stFrameData  = {
            .eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE,
            .ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420,
            .eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE,
            .eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE,
            .eFieldType = E_MI_SYS_FIELDTYPE_NONE,
            .u16Width = 1920,
            .u16Height = 1080,
            .u32Stride[0] = 1920,
            .u32Stride[1] = 1920,
            .u32Stride[2] = 1920,
        },
    },
    {
        .eBufType = E_MI_SYS_BUFDATA_FRAME,
        .u64Pts   = 0x12345678,
        .bEndOfStream = TRUE,
        .bUsrBuf      = FALSE,
        .stFrameData  = {
            .eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE,
            .ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420,
            .eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE,
            .eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE,
            .eFieldType = E_MI_SYS_FIELDTYPE_NONE,
            .u16Width = 1920,
            .u16Height = 1080,
            .u32Stride[0] = 1920,
            .u32Stride[1] = 1920,
            .u32Stride[2] = 1920,
        },
    },
    {
        .eBufType = E_MI_SYS_BUFDATA_FRAME,
        .u64Pts   = 0x12345678,
        .bEndOfStream = TRUE,
        .bUsrBuf      = FALSE,
        .stFrameData  = {
            .eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE,
            .ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420,
            .eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE,
            .eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE,
            .eFieldType = E_MI_SYS_FIELDTYPE_NONE,
            .u16Width = 1920,
            .u16Height = 1080,
            .u32Stride[0] = 1920,
            .u32Stride[1] = 1920,
            .u32Stride[2] = 1920,
        },
    },

};



MI_S32 mi_sys_EnableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    mi_sys_fakeDevInfo_t *pstDevInfo = (mi_sys_fakeDevInfo_t *)miSysDrvHandle;
    mi_sys_fakeChannelInfo_t *pChnnInfo = NULL;

    if (MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle) || MI_SYS_CHECK_CHNN_FAILED(u32ChnId))
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;

    }
    pChnnInfo = pstDevInfo->pstChnn + u32ChnId;
    pChnnInfo->bEnable = TRUE;
    DBG_INFO("sucess Chnn: %p !!!\n",   pChnnInfo);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_DisableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    mi_sys_fakeDevInfo_t *pstDevInfo = (mi_sys_fakeDevInfo_t *)miSysDrvHandle;
    mi_sys_fakeChannelInfo_t *pChnnInfo = NULL;
    if (MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle) || MI_SYS_CHECK_CHNN_FAILED(u32ChnId))
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;

    }

    pChnnInfo = pstDevInfo->pstChnn + u32ChnId;
    pChnnInfo->bEnable = FALSE;

    DBG_INFO("sucess Chnn: %p !!!\n",   pChnnInfo);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U32 u32Size ,MI_PHY *phyAddr)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    // MI_U8 *u8MMAHeapName, MI_U32 u32Size ,MI_PHY *phyAddr;
#if 0
    //if (NULL == MI_U8 *u8MMAHeapName, MI_U32 u32Size ,MI_PHY *phyAddr )
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;

    }
#endif
    DBG_INFO("sucess u8MMAHeapName: %s u32Size: %d!!!\n",   u8MMAHeapName,u32Size);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_MMA_Free(MI_PHY phyAddr)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
#if 0
    // MI_PHY phyAddr;
    //if (NULL == MI_PHY phyAddr )
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;

    }
#endif
    DBG_INFO("sucess!!!\n", __func__, __LINE__);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_FinishBuf(MI_SYS_BufInfo_t*buf)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    int i = 0;
    if ((NULL == buf) && (buf != &_gBuffInfo[0]) && (buf != &_gBuffInfo[1]) && (buf != &_gBuffInfo[2]))
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;

    }
    for (i = 0; i < sizeof(_gBufferUesd)/sizeof(_gBufferUesd[0]); i++)
    {
        if (&_gBuffInfo[i] == buf)
        {
            _gBufferUesd[i] = FALSE;
            s32Ret = MI_SUCCESS;
            break;
        }
    }

    DBG_INFO("sucess!!!\n", __func__, __LINE__);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_WaitOnInputTaskAvailable(MI_SYS_DRV_HANDLE miSysDrvHandle , MI_S32 u32TimeOutMs)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    static MI_U32  u32Cnt = 0;
    if (MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle))
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;

    }
    schedule_timeout_interruptible(msecs_to_jiffies(u32TimeOutMs));//sleep 10ms
    if (u32Cnt++ > 1000*20)
    {
        DBG_INFO("sucess!!!\n", __func__, __LINE__);
        u32Cnt = 0;
    }
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_SYS_BufInfo_t *mi_sys_GetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_U32 u32Flags)
{
    MI_SYS_BufInfo_t *pBuffer = NULL;
    mi_sys_fakeDevInfo_t *pstDevInfo = (mi_sys_fakeDevInfo_t *)miSysDrvHandle;
    mi_sys_fakeChannelInfo_t *pChnnInfo = NULL;
    int i = 0;
    if (MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle)
        || MI_SYS_CHECK_CHNN_FAILED(u32ChnId)
        || MI_SYS_CHECK_PORT_FAILED(u32PortId)
        )
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return NULL;

    }
    pChnnInfo = pstDevInfo->pstChnn;

    if (pChnnInfo->bEnable == FALSE)
    {
        return NULL;
    }

    for (i = 0; i < sizeof(_gBufferUesd)/sizeof(_gBufferUesd[0]); i++)
    {
        if (_gBufferUesd[i] == FALSE)
        {
            _gBufferUesd[i] = TRUE;
            pBuffer = &_gBuffInfo[i];
            break;
        }
    }
    DBG_INFO("sucess pBuffer: %p!!!\n",   pBuffer);

    return pBuffer;
}

MI_S32 mi_sys_RewindBuf(MI_SYS_BufInfo_t*buf)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    int i = 0;
    if ((NULL == buf) && (buf != &_gBuffInfo[0]) && (buf != &_gBuffInfo[1]) && (buf != &_gBuffInfo[2]))
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;

    }
    for (i = 0; i < sizeof(_gBufferUesd)/sizeof(_gBufferUesd[0]); i++)
    {
        if (&_gBuffInfo[i] == buf)
        {
            _gBufferUesd[i] = FALSE;
            s32Ret = MI_SUCCESS;
            break;
        }
    }

    DBG_INFO("sucess!!!\n", __func__, __LINE__);

    return s32Ret;
}

//----------------------------------------

MI_SYS_DRV_HANDLE  mi_sys_RegisterDev(mi_sys_ModuleDevInfo_t *pstMouleInfo, mi_sys_ModuleDevBindOps_t *pstModuleBindOps , void *pUsrData
                                                #ifdef MI_SYS_PROC_FS_DEBUG
                                                ,mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps
                                                ,struct proc_dir_entry *proc_dir_entry
                                                #endif
                                                        )

{
    MI_SYS_DRV_HANDLE handle = NULL;
    mi_sys_fakeDevInfo_t *pstDevInfo = &_gstDevFake;
    mi_sys_fakeChannelInfo_t *pstChnn    = NULL;
    mi_sys_fakePortInfo_t *pstInputPort  = NULL, *pstPortTmp = NULL;
    mi_sys_fakePortInfo_t *pstOutputPort = NULL;
    int i = 0, j = 0;

    if (pstDevInfo->u32Count++ > SYS_TEST_MAX)
    {
        return handle;
    }

    if (NULL == pstMouleInfo)
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return handle;
    }
    pstChnn = kzalloc((sizeof(*pstChnn)*pstMouleInfo->u32DevChnNum), GFP_KERNEL);
    if (pstChnn == NULL)
    {
        DBG_ERR("%s()@line %d alloc channels failed.\n", __func__, __LINE__);
        goto create_chnns_err;
    }
    pstDevInfo->pstChnn    = pstChnn;

    pstInputPort  = kzalloc(sizeof(*pstInputPort)*pstMouleInfo->u32InputPortNum*pstMouleInfo->u32DevChnNum, GFP_KERNEL);
    pstOutputPort = kzalloc(sizeof(*pstOutputPort)*pstMouleInfo->u32OutputPortNum*pstMouleInfo->u32DevChnNum, GFP_KERNEL);
    if ((pstInputPort == NULL) || (pstOutputPort == NULL))
    {
        if (pstInputPort != NULL)
        {
            kfree(pstInputPort);
        }
        if (pstOutputPort != NULL)
        {
            kfree(pstOutputPort);
        }
        goto port_alloc_erro;
    }

    pstDevInfo->u32ChnnNum = pstMouleInfo->u32DevChnNum;

    for (i = 0; i < pstMouleInfo->u32DevChnNum; i++)
    {
        pstChnn += i;
        pstChnn->bEnable = FALSE;
        pstChnn->u32InputPortNum = pstMouleInfo->u32InputPortNum;
        pstChnn->pstInputPort    = pstInputPort + i* pstMouleInfo->u32InputPortNum;
        for (j = 0; j < pstChnn->u32InputPortNum; j++)
        {
            pstPortTmp = pstChnn->pstInputPort + j;
            pstPortTmp->bEnable   = FALSE;
            pstPortTmp->u32PortId = j;
        }

        pstChnn->u32OutputPortNum = pstMouleInfo->u32OutputPortNum;
        pstChnn->pstOutputPort    = pstOutputPort + i* pstMouleInfo->u32OutputPortNum;
        for (j = 0; j < pstChnn->u32OutputPortNum; j++)
        {
            pstPortTmp = pstChnn->pstInputPort + j;
            pstPortTmp->bEnable   = FALSE;
            pstPortTmp->u32PortId = j;
        }
    }


    DBG_INFO("sucess!!!\n", __func__, __LINE__);
    return pstDevInfo;

port_alloc_erro:
    kfree(pstDevInfo->pstChnn);
    pstDevInfo->pstChnn = NULL;
create_chnns_err:
    return handle;
}

MI_S32 mi_sys_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    mi_sys_fakeDevInfo_t *pstDevInfo = (mi_sys_fakeDevInfo_t *)miSysDrvHandle;

    if (((miSysDrvHandle) != &_gstDevFake))
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;
    }
    kfree(pstDevInfo->pstChnn->pstInputPort);
    kfree(pstDevInfo->pstChnn->pstOutputPort);
    kfree(pstDevInfo->pstChnn);
    pstDevInfo->u32Count--;
    pstDevInfo->u32ChnnNum = 0;
    pstDevInfo->bInited    = FALSE;

    DBG_INFO("sucess!!!\n", __func__, __LINE__);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}
#define CHNN_BUFF_MAX   (2)
#define INPUT_PORT_MAX  (1)
#define OUTPUT_PORT_MAX (4)

typedef struct {
    MI_SYS_BufInfo_t stInputBuffer[INPUT_PORT_MAX][CHNN_BUFF_MAX];
    MI_SYS_BufInfo_t stOutputBuffer[OUTPUT_PORT_MAX][CHNN_BUFF_MAX];
    MI_U32 u32InputUsed[INPUT_PORT_MAX];
    MI_U32 u32OutputUsed[OUTPUT_PORT_MAX];
}mi_sys_fakeBuffer;
typedef struct {
    mi_sys_ChnTaskInfo_t stTaskInfo;
    MI_U32 u32InputIndex[INPUT_PORT_MAX];
    MI_U32 u32OutputIndex[OUTPUT_PORT_MAX];
    MI_U32 u32ChnnId;
    MI_U32 u32InputPortId;
}mi_sys_fakeTask;

static mi_sys_fakeTask _gstTaskInfo[CHNN_TEST_MAX][CHNN_BUFF_MAX];
static mi_sys_fakeBuffer    _gBufferMgr[CHNN_TEST_MAX];


MI_S32 mi_sys_DevTaskIterator (MI_SYS_DRV_HANDLE miSysDrvHandle, mi_sys_TaskIteratorCallBack pfCallBack,void *pUsrData)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    mi_sys_fakeDevInfo_t *pstDevInfo         = (mi_sys_fakeDevInfo_t *)miSysDrvHandle;
    mi_sys_fakeChannelInfo_t *pstChnInfo     = NULL;
    mi_sys_ChnTaskInfo_t *pstChnTask         = NULL;
    int i, j, inputPort;

    if ((MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle)) || (pfCallBack == NULL))
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;
    }

    for (i = 0; i < pstDevInfo->u32ChnnNum; i++)
    {
        pstChnInfo = pstDevInfo->pstChnn + i;
        if (pstChnInfo->bEnable == FALSE)
        {
            continue;
        }

        // Fixme: Fixed input port number = 0
        if ( FALSE == pstChnInfo->pstInputPort->bEnable)
        {
            continue;
        }
        inputPort = 0;
        for (j = 0; j < CHNN_BUFF_MAX; j++)
        {
            // Just scan input port 0
            if ((_gBufferMgr[i].u32InputUsed[inputPort] & (1 << j)) == 0)
            {
                mi_sys_fakeTask *pstFakeTask = &_gstTaskInfo[i][j];
                pstFakeTask->u32InputPortId = inputPort;
                pstChnTask = &pstFakeTask->stTaskInfo;
                //pstChnTask->eBufType = E_MI_SYS_BUFDATA_FRAME;
                pstChnTask->miSysDrvHandle = miSysDrvHandle;
                pstChnTask->u32ChnId       = i;
                pstChnTask->astInputPortBufInfo[inputPort] = &_gBufferMgr[i].stInputBuffer[inputPort][j];
                pstChnTask->astInputPortBufInfo[inputPort]->eBufType = E_MI_SYS_BUFDATA_FRAME;
                pstFakeTask->u32InputIndex[inputPort]  = j;
                pstFakeTask->u32ChnnId = i;
                _gBufferMgr[i].u32InputUsed[inputPort] |= (1 << j);
                pfCallBack(pstChnTask, pUsrData);
                break;
            }
        }
    }

    DBG_INFO("sucess !!!\n");
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_DisableInputPort (MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    mi_sys_fakeDevInfo_t *pstDevInfo      = (mi_sys_fakeDevInfo_t *)miSysDrvHandle;
    mi_sys_fakeChannelInfo_t *pstChnInfo  = NULL;
    mi_sys_fakePortInfo_t    *pstPortInfo = NULL;
    if ((MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle)) || (MI_SYS_CHECK_CHNN_FAILED(u32ChnId)) || (MI_SYS_CHECK_PORT_FAILED(u32PortId)))
    {
        DBG_ERR("pointer Failed miSysDrvHandle: %p u32ChnId: %d u32PortId: %d !!!\n",   miSysDrvHandle, u32ChnId, u32PortId);
        return E_MI_ERR_BUSY;
    }
    pstChnInfo = pstDevInfo->pstChnn + u32ChnId;
    pstPortInfo = pstChnInfo->pstInputPort + u32PortId;
    pstPortInfo->bEnable = FALSE;

    DBG_INFO("sucess ChId: %d PortId: %d !!!\n",   u32ChnId, u32PortId);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_DisableOutputPort (MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    mi_sys_fakeDevInfo_t *pstDevInfo      = (mi_sys_fakeDevInfo_t *)miSysDrvHandle;
    mi_sys_fakeChannelInfo_t *pstChnInfo  = NULL;
    mi_sys_fakePortInfo_t    *pstPortInfo = NULL;
    if ((MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle)) || (MI_SYS_CHECK_CHNN_FAILED(u32ChnId)) || (MI_SYS_CHECK_PORT_FAILED(u32PortId)))
    {
        DBG_ERR("pointer Failed miSysDrvHandle: %p u32ChnId: %d u32PortId: %d !!!\n",   miSysDrvHandle, u32ChnId, u32PortId);
        return E_MI_ERR_BUSY;
    }

    pstChnInfo = pstDevInfo->pstChnn + u32ChnId;
    pstPortInfo = pstChnInfo->pstOutputPort + u32PortId;
    pstPortInfo->bEnable = FALSE;

    DBG_INFO("sucess ChId: %d PortId: %d !!!\n",   u32ChnId, u32PortId);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_EnableInputPort (MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    mi_sys_fakeDevInfo_t *pstDevInfo      = (mi_sys_fakeDevInfo_t *)miSysDrvHandle;
    mi_sys_fakeChannelInfo_t *pstChnInfo  = NULL;
    mi_sys_fakePortInfo_t    *pstPortInfo = NULL;
    if ((MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle)) || (MI_SYS_CHECK_CHNN_FAILED(u32ChnId)) || (MI_SYS_CHECK_PORT_FAILED(u32PortId)))
    {
        DBG_ERR("pointer Failed miSysDrvHandle: %p u32ChnId: %d u32PortId: %d !!!\n",   miSysDrvHandle, u32ChnId, u32PortId);
        return E_MI_ERR_BUSY;
    }
    pstChnInfo = pstDevInfo->pstChnn + u32ChnId;
    pstPortInfo = pstChnInfo->pstInputPort + u32PortId;
    pstPortInfo->bEnable = TRUE;

    DBG_INFO("sucess ChId: %d PortId: %d !!!\n",   u32ChnId, u32PortId);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_EnableOutputPort (MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    mi_sys_fakeDevInfo_t *pstDevInfo      = (mi_sys_fakeDevInfo_t *)miSysDrvHandle;
    mi_sys_fakeChannelInfo_t *pstChnInfo  = NULL;
    mi_sys_fakePortInfo_t    *pstPortInfo = NULL;
    if ((MI_SYS_CHECK_HANDLE_FAILED(miSysDrvHandle)) || (MI_SYS_CHECK_CHNN_FAILED(u32ChnId)) || (MI_SYS_CHECK_PORT_FAILED(u32PortId)))
    {
        DBG_ERR("pointer Failed miSysDrvHandle: %p u32ChnId: %d u32PortId: %d !!!\n",   miSysDrvHandle, u32ChnId, u32PortId);
        return E_MI_ERR_BUSY;
    }

    pstChnInfo = pstDevInfo->pstChnn + u32ChnId;
    pstPortInfo = pstChnInfo->pstOutputPort + u32PortId;
    pstPortInfo->bEnable = TRUE;

    DBG_INFO("sucess ChId: %d PortId: %d !!!\n",   u32ChnId, u32PortId);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mi_sys_FinishAndReleaseTask (mi_sys_ChnTaskInfo_t *pstTask)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    int i;
    MI_U32 u32ChnnId = 0, u32InputPortId;
    mi_sys_fakeDevInfo_t *pstDevInfo = &_gstDevFake;
    mi_sys_fakeTask *pstFakeTask = NULL;

    if (pstTask == NULL)
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;
    }
    pstFakeTask = container_of(pstTask, mi_sys_fakeTask, stTaskInfo);
    u32ChnnId = pstFakeTask->u32ChnnId;
    u32InputPortId = pstFakeTask->u32InputPortId;

    // Release input buffer
    _gBufferMgr[u32ChnnId].u32InputUsed[u32InputPortId] &= ~(1 <<pstFakeTask->u32InputIndex[u32InputPortId]);

    // Release output buffer
    for (i = 0; i < pstDevInfo->pstChnn->u32OutputPortNum; i++)
    {
        if (pstTask->astOutputPortBufInfo[i] != NULL)
        {
            _gBufferMgr[u32ChnnId].u32OutputUsed[i] &= ~(1 << pstFakeTask->u32OutputIndex[i]);
            DBG_INFO("success to  finish and release: %p !!!\n",   pstTask);
            goto out_ok;
        }
    }

    DBG_ERR("Fail to  finish and release: %p !!!\n",   pstTask);
    return s32Ret;
out_ok:
    return MI_SUCCESS;
}

MI_S32 mi_sys_PrepareTaskOutputBuf (mi_sys_ChnTaskInfo_t *pstTask)
{
    MI_S32 s32Ret = E_MI_ERR_BUSY;
    int i, j;
    MI_U32 u32ChnnId = 0;
    mi_sys_fakeDevInfo_t *pstDevInfo = &_gstDevFake;
    mi_sys_fakeChannelInfo_t *pstChnn = NULL;
    mi_sys_fakeTask *pstFakeTask = NULL;
    mi_sys_fakePortInfo_t *pstPort = NULL;

    if (pstTask == NULL)
    {
        DBG_ERR("pointer Failed!!!\n", __func__, __LINE__);
        return E_MI_ERR_BUSY;
    }

    pstFakeTask = container_of(pstTask, mi_sys_fakeTask, stTaskInfo);
    u32ChnnId = pstFakeTask->u32ChnnId;
    pstChnn   = pstDevInfo->pstChnn + u32ChnnId;
    if (pstChnn->bEnable == TRUE)
    {
        pstPort = pstChnn->pstOutputPort;
        for (i = 0; i < pstDevInfo->pstChnn->u32OutputPortNum; i++)
        {
            pstPort += i;
            if (((pstTask->astOutputPortPerfBufConfig[i].eBufType == E_MI_SYS_BUFDATA_RAW) && (pstTask->astOutputPortPerfBufConfig[i].stRawCfg.u32Size == 0))
                || (pstPort->bEnable == FALSE))
            {
                pstTask->astOutputPortBufInfo[i] = NULL;
            }
            else
            {
                // Get output buffer
                for (j = 0; j < pstChnn->u32OutputPortNum; j++)
                {
                    if ((_gBufferMgr[u32ChnnId].u32OutputUsed[i] & (1 << j)) == 0)
                    {
                        pstTask->astOutputPortBufInfo[i] = &_gBufferMgr[u32ChnnId].stOutputBuffer[i][j];
                        _gBufferMgr[u32ChnnId].u32OutputUsed[i] |= (1 << j);
                        break;
                    }
                }
                pstTask->astOutputPortBufInfo[i] = NULL;
            }
        }
    }
    DBG_INFO("sucess finish and release: %p !!!\n",   pstTask);
    s32Ret = MI_SUCCESS;
    return s32Ret;
}
