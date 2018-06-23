#ifndef _MI_SYS_INTERNAL_H_
#define _MI_SYS_INTERNAL_H_
#include <linux/types.h>

#include "mi_common.h"
#include "mi_sys_datatype.h"

typedef void *  MI_SYS_DRV_HANDLE;

typedef struct mi_sys_ModuleDevInfo_s
{
    MI_SYS_MODULE_ID_e eModuleId;
    MI_U32 u32DevId;
    MI_U32 u32InputPortNum;
    MI_U32 u32OutputPortNum;
    MI_U32 u32DevChnNum;
}mi_sys_ModuleDevInfo_t;


typedef enum
{
    // driver accept current task and continue iterator
    MI_SYS_ITERATOR_ACCEPT_CONTINUTE,
    // driver reject current task and continue iterator,
    MI_SYS_ITERATOR_SKIP_CONTINUTE,
    // driver accept current task and ask to stop iterator
    MI_SYS_ITERATOR_ACCEPT_STOP,
    // driver accept reject task and ask to stop iterator
    MI_SYS_ITERATOR_SKIP_STOP
}mi_sys_TaskIteratorCBAction_e;

typedef struct mi_sys_ModuleDevBindOps_s
{
    MI_S32 (*OnBindInputPort)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData);
    MI_S32 (*OnUnBindInputPort)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData);
    MI_S32 (*OnBindOutputPort)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData);
    MI_S32 (*OnUnBindOutputPort)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData);
} mi_sys_ModuleDevBindOps_t;

typedef struct mi_sys_ChnTaskInfo_s
{
    struct list_head list;//for driver facility
    MI_U32 u32Reserved0;
    MI_U64 u64Reserved0;//for driver facility
    
    MI_SYS_DRV_HANDLE miSysDrvHandle;
    ///which channel
    MI_U32 u32ChnId;
    ///current channel , all need to handle buffer channel numbers
    MI_U32 u32InputPortNum;
    ///current channel , need to handle pending buffer inputport index
    MI_U32 au32InputPortId[MI_SYS_MAX_INPUT_PORT_CNT];
    ///buffer will lock and remove pending list
    MI_SYS_BufInfo_t *astInputPortBufInfo[MI_SYS_MAX_INPUT_PORT_CNT];

    /////////////////////////////////
    ///assign value after call MI_SYS_PrepareTaskOuputPortBuf
    MI_U32 u32OutputPortNum;
    MI_U32 au32OutputPortId[MI_SYS_MAX_OUTPUT_PORT_CNT];
    MI_SYS_BufConf_t   astOutputPortPerfBufConfig[MI_SYS_MAX_OUTPUT_PORT_CNT];
    ///buffer will lock and remove free buffer list
    MI_SYS_BufInfo_t *astOutputPortBufInfo[MI_SYS_MAX_OUTPUT_PORT_CNT];
    ///frame rate control
    MI_BOOL bOutputPortMaskedByFrmrateCtrl[MI_SYS_MAX_OUTPUT_PORT_CNT];
}mi_sys_ChnTaskInfo_t;

///
typedef struct
{
    //channel number
    MI_U32 u32ChannelId;

    MI_U32 u32InputPortNum;
    ///input port pending buffer from peer binded channel
    MI_U32 au32InputPortBindConnectBufPendingCnt[MI_SYS_MAX_INPUT_PORT_CNT];
    ///input port pending buffer from user insert
    MI_U32 au32InputPortUserBufPendingCnt[MI_SYS_MAX_INPUT_PORT_CNT];
 
    ///input port haved locked count
    MI_U32 au32InputPortBufLockedNum[MI_SYS_MAX_INPUT_PORT_CNT];
    
    
    MI_U32 u32OutputPortNum;
    /// output buffer  count locked by user
    MI_U32 au32OutputPortBufUsrLockedNum[MI_SYS_MAX_OUTPUT_PORT_CNT];
    /// output buffer  count in user fifo list
    MI_U32 au32OutputPortBufInUsrFIFONum[MI_SYS_MAX_OUTPUT_PORT_CNT];
    /// output total in used buffer count
    MI_U32 au32OutputPortBufTotalInUsedNum[MI_SYS_MAX_OUTPUT_PORT_CNT];
}mi_sys_ChnBufInfo_t;

typedef struct buf_allocation_ops_s
{
    void (*OnRef)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*OnUnref)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*OnRelease)(struct MI_SYS_BufferAllocation_s *thiz);
    void *(*map_usr)((truct MI_SYS_BufferAllocation_s *thiz);
    void  (*unmap_user)(struct MI_SYS_BufferAllocation_s *thiz);
}buf_allocation_ops_t;

typedef struct MI_SYS_BufferAllocation_s
{
   atomic_t  ref_cnt;
   buf_allocation_ops_t *ops;
   MI_PHY u64PhyAddr;
   unsigned long u32Length;
}MI_SYS_BufferAllocation_t;

typedef struct buf_allocator_ops_s
{
    MI_S32 (*OnRef)(struct mi_sys_Allocator_s *thiz);
    MI_S32 (*OnUnref)(struct mi_sys_Allocator_s *thiz);
    MI_S32(*onRelease)(struct mi_sys_Allocator_s *thiz);
    void *(*map_allocation_usr)(struct mi_sys_Allocator_s *thiz, MI_SYS_BufferAllocation_t*pst_allocation);
    void  (*unmap_allocation_usr)(struct mi_sys_Allocator_s*thiz,  MI_SYS_BufferAllocation_t*pst_allocation);
    MI_SYS_BufferAllocation_t *(*alloc)(mi_sys_Allocator_t *pstAllocator,
                                       MI_Buf_Config *stBufConfig , MI_SYS_ChnPort_t *stChnPort);
    void (*free)(mi_sys_Allocator_t *pstAllocator, MI_SYS_BufferAllocation_t*pst_allocatio);
}buf_allocator_ops_t;

typedef struct mi_sys_Allocator_s
{
    struct list_head list;
    buf_allocator_ops_t *ops;
}mi_sys_Allocator_t;



MI_SYS_DRV_HANDLE  mi_sys_RegisterDev(mi_sys_ModuleDevInfo_t *pstMouleInfo, mi_sys_ModuleDevBindOps_t *pstModuleBindOps , void *pUsrData);
MI_S32 mi_sys_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle);

MI_S32 mi_sys_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, mi_sys_ChnBufInfo_t *pstChnBufInfo);

typedef MI_S32 (* mi_sys_TaskIteratorCallBack)(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData);
MI_S32 mi_sys_DevTaskIterator(MI_SYS_DRV_HANDLE miSysDrvHandle, mi_sys_TaskIteratorCallBack pfCallBack,void *pUsrData);
MI_S32 mi_sys_PrepareTaskOutputBuf(mi_sys_ChnTaskInfo_t *pstTask);

MI_S32 mi_sys_FinishTaskBuf(mi_sys_ChnTaskInfo_t *pstTask);
MI_S32 mi_sys_RewindTaskBuf(mi_sys_ChnTaskInfo_t *pstTask);

MI_S32 mi_sys_WaitOnInputTaskAvailable(MI_SYS_DRV_HANDLE miSysDrvHandle , MI_S32 u32TimeOutMs);

MI_SYS_BufInfo_t *mi_sys_GetOutputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_SYS_BufConf_t *stBufConfig);
MI_SYS_BufInfo_t *mi_sys_GetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId);

MI_S32 mi_sys_FinishBuf(MI_SYS_BufInfo_t*buf);
MI_S32 mi_sys_RewindBuf(MI_SYS_BufInfo_t*buf);


MI_S32 mi_sys_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U32 u32Size ,MI_PHY *phyAddr);
MI_S32 mi_sys_MMA_Free(MI_PHY phyAddr);


void * mi_sys_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size , MI_BOOL bCache);
void mi_sys_UnVmap(void *pVirtAddr);


MI_S32 mi_sys_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, mi_sys_Allocator_t *pstUserAllocator);
MI_S32 mi_sys_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, mi_sys_Allocator_t *pstUserAllocator);
#endif ///_MI_SYS_INTERNAL_H_