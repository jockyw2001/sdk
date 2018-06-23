#ifndef _MI_SYS_INTERNAL_H_
#define _MI_SYS_INTERNAL_H_
#include <linux/types.h>

#include "mi_common.h"
#include "mi_sys_datatype.h"

typedef void *  MI_SYS_DRV_HANDLE;

#define ALIGN_DOWN(val, alignment) (((val)/(alignment))*(alignment))
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))


#define MI_SYS_MAP_VA   0x80000000

typedef struct mi_sys_ModuleDevInfo_s
{
    MI_SYS_ModuleId_e eModuleId;
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
}mi_sys_TaskIteratorCBAction_e;

typedef struct mi_sys_ModuleDevBindOps_s
{
    MI_S32 (*OnBindInputPort)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData);
    MI_S32 (*OnUnBindInputPort)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData);
    MI_S32 (*OnBindOutputPort)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData);
    MI_S32 (*OnUnBindOutputPort)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData);
    MI_S32 (*OnOutputPortBufRelease)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_BufInfo_t *pstBufInfo);
} mi_sys_ModuleDevBindOps_t;

typedef struct mi_sys_ChnTaskInfo_s
{
    struct list_head listChnTask;//for driver facility
    MI_U32 u32Reserved0;
    MI_U32 u32Reserved1;
    MI_U64 u64Reserved0;//for driver facility
    MI_U32 u64Reserved1;

    MI_SYS_DRV_HANDLE miSysDrvHandle;
    ///which channel
    MI_U32 u32ChnId;
    ///buffer will lock and remove pending list
    MI_SYS_BufInfo_t *astInputPortBufInfo[MI_SYS_MAX_INPUT_PORT_CNT];

    //Fill in the buf configuration information by the moudle owner
    MI_SYS_BufConf_t   astOutputPortPerfBufConfig[MI_SYS_MAX_OUTPUT_PORT_CNT];

    ///buffer will lock and remove free buffer list
    MI_SYS_BufInfo_t *astOutputPortBufInfo[MI_SYS_MAX_OUTPUT_PORT_CNT];
    ///frame rate control
    MI_BOOL bOutputPortMaskedByFrmrateCtrl[MI_SYS_MAX_OUTPUT_PORT_CNT];
}mi_sys_ChnTaskInfo_t;

///
typedef struct mi_sys_ChnBufInfo_s
{
    //channel number
    MI_U32 u32ChannelId;

    MI_U32 u32InputPortNum;
    ///input port pending buffer from peer binded channel
    MI_U32 au32InputPortBindConnectBufPendingCnt[MI_SYS_MAX_INPUT_PORT_CNT];
    ///input port pending buffer from user insert
    MI_U32 au32InputPortUserBufPendingCnt[MI_SYS_MAX_INPUT_PORT_CNT];

    ///input port haved locked count
    MI_U32 au32InputPortBufHoldByDrv[MI_SYS_MAX_INPUT_PORT_CNT];


    MI_U32 u32OutputPortNum;
    /// output buffer  count hold by user through MI_SYS_Get_Outputport_Buf();
    MI_U32 au32OutputPortBufUsrLockedNum[MI_SYS_MAX_OUTPUT_PORT_CNT];
    /// output buffer  count in user fifo list
    MI_U32 au32OutputPortBufInUsrFIFONum[MI_SYS_MAX_OUTPUT_PORT_CNT];
    //output buffer current hold by MI Driver
    MI_U32 au32OutputPortBufHoldByDrv[MI_SYS_MAX_OUTPUT_PORT_CNT];

    /// output total in used buffer count
    MI_U32 au32OutputPortBufTotalInUsedNum[MI_SYS_MAX_OUTPUT_PORT_CNT];
}mi_sys_ChnBufInfo_t;

struct MI_SYS_BufferAllocation_s;
typedef struct buf_allocation_ops_s
{
    void (*OnRef)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*OnUnref)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*OnRelease)(struct MI_SYS_BufferAllocation_s *thiz);
    void *(*map_user)(struct MI_SYS_BufferAllocation_s *thiz);
    void  (*unmap_user)(struct MI_SYS_BufferAllocation_s *thiz);
    void *(*vmap_kern)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*vunmap_kern)(struct MI_SYS_BufferAllocation_s *thiz);
}buf_allocation_ops_t;

typedef void (*MI_SYS_Allocation_Free_Notify)(struct MI_SYS_BufferAllocation_s *allocation, void *pUsrdata);

typedef struct MI_SYS_BufferAllocation_s
{
   atomic_t  ref_cnt;
   struct buf_allocation_ops_s ops;
   MI_SYS_BufInfo_t stBufInfo;
   void *private1;
}MI_SYS_BufferAllocation_t;

struct mi_sys_Allocator_s;
typedef struct buf_allocator_ops_s
{
    MI_S32 (*OnRef)(struct mi_sys_Allocator_s *thiz);
    MI_S32 (*OnUnref)(struct mi_sys_Allocator_s *thiz);
    MI_S32 (*OnRelease)(struct mi_sys_Allocator_s *thiz);
    MI_SYS_BufferAllocation_t *(*alloc)(struct mi_sys_Allocator_s *pstAllocator, MI_SYS_BufConf_t *stBufConfig);
    int (*suit_bufconfig)(struct mi_sys_Allocator_s *pstAllocator, MI_SYS_BufConf_t *pstBufConfig);
}buf_allocator_ops_t;

typedef struct mi_sys_Allocator_s
{
    atomic_t  ref_cnt;
    unsigned int u32MagicNumber;
    struct list_head list;
    buf_allocator_ops_t *ops;

}mi_sys_Allocator_t;

void generic_allocation_on_ref(struct MI_SYS_BufferAllocation_s *pstBufAllocation);

void generic_allocation_on_unref(struct MI_SYS_BufferAllocation_s *pstBufAllocation);

//use this function to replace mi_sys_buf_mgr_init_std_allocation function
void generic_allocation_init(MI_SYS_BufferAllocation_t *pstBufAllocation,buf_allocation_ops_t *ops);

MI_S32 generic_allocator_on_ref(struct mi_sys_Allocator_s *pstAllocator);

MI_S32 generic_allocator_on_unref(struct mi_sys_Allocator_s *pstAllocator);
void generic_allocator_init(mi_sys_Allocator_t *pstAllocator, buf_allocator_ops_t *ops);


MI_SYS_DRV_HANDLE  mi_sys_RegisterDev(mi_sys_ModuleDevInfo_t *pstMouleInfo, mi_sys_ModuleDevBindOps_t *pstModuleBindOps , void *pUsrData);
MI_S32 mi_sys_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle);

MI_S32 mi_sys_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, mi_sys_ChnBufInfo_t *pstChnBufInfo);

typedef MI_S32 (* mi_sys_TaskIteratorCallBack)(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData);

MI_S32 mi_sys_DevTaskIterator(MI_SYS_DRV_HANDLE miSysDrvHandle, mi_sys_TaskIteratorCallBack pfCallBack,void *pUsrData);
MI_S32 mi_sys_PrepareTaskOutputBuf(mi_sys_ChnTaskInfo_t *pstTask);
MI_S32 mi_sys_FinishAndReleaseTask(mi_sys_ChnTaskInfo_t *pstTask);
MI_S32 mi_sys_RewindTask(mi_sys_ChnTaskInfo_t *pstTask);

MI_S32 mi_sys_WaitOnInputTaskAvailable(MI_SYS_DRV_HANDLE miSysDrvHandle , MI_S32 u32TimeOutMs);

MI_SYS_BufInfo_t *mi_sys_GetOutputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_SYS_BufConf_t *stBufConfig , MI_BOOL *pbBlockedByRateCtrl);
MI_SYS_BufInfo_t *mi_sys_GetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_U32 u32Flags);
MI_S32 mi_sys_FinishBuf(MI_SYS_BufInfo_t*buf);
MI_S32 mi_sys_RewindBuf(MI_SYS_BufInfo_t*buf);

MI_S32 mi_sys_EnableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);
MI_S32 mi_sys_DisableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);

MI_S32 mi_sys_EnableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);
MI_S32 mi_sys_DisableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);


MI_S32 mi_sys_EnableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);
MI_S32 mi_sys_DisableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);

MI_S32 mi_sys_SetReg (MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask);
MI_S32 mi_sys_GetReg (MI_U32 u32RegAddr, MI_U16 *pu16Value);

MI_S32 mi_sys_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U32 u32Size ,MI_PHY *phyAddr);
MI_S32 mi_sys_MMA_Free(MI_PHY phyAddr);


void * mi_sys_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size , MI_BOOL bCache);
void mi_sys_UnVmap(void *pVirtAddr);

MI_S32 mi_sys_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);
MI_S32 mi_sys_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);
MI_S32 mi_sys_EnsureOutportBkRefFifoDepth (MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_U32 u32BkRefFifoDepth);
MI_S32 mi_sys_MmapBufToUser(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_SYS_BufInfo_t *pstBufInfo, void **ppBufHandle);
MI_S32 mi_sys_UnmapBufToUser(MI_SYS_DRV_HANDLE miSysDrvHandle, void *pBufHandle, MI_SYS_BufInfo_t **ppstBufInfo);
#endif ///_MI_SYS_INTERNAL_H_