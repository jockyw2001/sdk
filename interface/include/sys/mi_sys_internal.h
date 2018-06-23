#ifndef _MI_SYS_INTERNAL_H_
#define _MI_SYS_INTERNAL_H_
#include <linux/types.h>

#include "mi_common.h"
#include "mi_sys_datatype.h"
#include <linux/proc_fs.h>

#define MI_SYS_PROC_FS_DEBUG
#define MI_DISP_PROCFS_DEBUG (1)
#define MI_VIF_PROCFS_DEBUG (1)
#define MI_VDISP_PROCFS_DEBUG (1)
#define MI_UAC_PROCFS_DEBUG (1)
#define MI_DIVP_PROCFS_DEBUG (1)
#define MI_VPE_PROCFS_DEBUG (1)
#define MI_AI_PROCFS_DEBUG (1)
#define MI_AO_PROCFS_DEBUG (1)
#define MI_HDMI_PROCFS_DEBUG (1)
#define MI_GFX_PROCFS_DEBUG (1)
#define MI_RGN_PROCFS_DEBUG (1)
#define MI_VDEC_PROCFS_DEBUG (1)
#define MI_WARP_PROCFS_DEBUG (1)
#define MI_LDC_PROCFS_DEBUG (1)
#define MI_PANEL_PROCFS_DEBUG (1)
//#define MI_SYS_SERIOUS_ERR_MAY_MULTI_TIMES_SHOW

typedef void *  MI_SYS_DRV_HANDLE;

#define ALIGN_DOWN(val, alignment) (((val)/(alignment))*(alignment))
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#define WAKE_UP_QUEUE_IF_NECESSARY(waitqueue)  \
    do{   \
        if(waitqueue_active(&(waitqueue))) \
            wake_up_all(&(waitqueue)); \
    }while(0)

#define MI_SYS_BUG_ON(exp)    \
    do{    \
        if(exp)    \
        {    \
            printk("[%s][%s][%d]Case %s BUG ON!!!\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, #exp);    \
            BUG();\
        }    \
    }while(0)

#define MI_SYS_BUG() MI_SYS_BUG_ON(1)


typedef struct mi_sys_ModuleDevInfo_s
{
    MI_ModuleId_e eModuleId;
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

#ifdef MI_SYS_PROC_FS_DEBUG

typedef struct MI_SYS_DEBUG_HANDLE_s
{
    MI_S32 (*OnPrintOut)( struct MI_SYS_DEBUG_HANDLE_s  handle, char *data,...);
    void *file;
} MI_SYS_DEBUG_HANDLE_t;


typedef struct mi_sys_ModuleDevProcfsOps_s
{
    //these callbacks include all dump info that needed!!!
    //if these callbacks  success,please return MI_SUCCESS,do not return MI_DEF_ERR(E_MI_SYS_MODULE_ID_xx, E_MI_ERR_LEVEL_yy, MI_SUCCESS)
    MI_S32 (*OnDumpDevAttr)(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId,void *pUsrData);
    MI_S32 (*OnDumpChannelAttr)(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData);
    MI_S32 (*OnDumpInputPortAttr)(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData);
    MI_S32 (*OnDumpOutPortAttr)(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData);
    MI_S32 (*OnHelp)(MI_SYS_DEBUG_HANDLE_t  handle,MI_U32  u32DevId,void *pUsrData);
} mi_sys_ModuleDevProcfsOps_t;

typedef struct allocator_echo_cmd_info_s
{
    MI_U8 dir_name[256];
    MI_U32 dir_size;
    MI_U32 offset;
    MI_U32 length;
}allocator_echo_cmd_info_t;


typedef struct mi_sys_AllocatorProcfsOps_s
{
    MI_S32 (*OnDumpAllocatorAttr)(MI_SYS_DEBUG_HANDLE_t  handle,void *private);
    MI_S32 (*OnAllocatorExecCmd)(MI_SYS_DEBUG_HANDLE_t  handle,allocator_echo_cmd_info_t *cmd_info,void *private);
} mi_sys_AllocatorProcfsOps_t;
#endif

typedef struct mi_sys_ChnTaskInfo_s
{
    struct list_head listChnTask;//for driver facility
    MI_U32 u32Reserved0;
    MI_U32 u32Reserved1;
    MI_U64 u64Reserved0;//for driver facility
    MI_U64 u64Reserved1;

    MI_U64 u64Reserved2;
    MI_U64 u64Reserved3;
    MI_U64 u64Reserved4;
    MI_U64 u64Reserved5;
    MI_U64 u64Reserved6;

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

    unsigned char padding[8];
}mi_sys_ChnTaskInfo_t;

///
typedef struct mi_sys_ChnBufInfo_s
{
    //channel number
    MI_U32 u32ChannelId;

    MI_U32 u32InputPortNum;
    ///input port pending buffer from peer binded channel
    MI_U16 au16InputPortBindConnectBufPendingCnt[MI_SYS_MAX_INPUT_PORT_CNT];
    ///input port pending buffer from user insert
    MI_U16 au16InputPortUserBufPendingCnt[MI_SYS_MAX_INPUT_PORT_CNT];

    ///input port haved locked count
    MI_U16 au16InputPortBufHoldByDrv[MI_SYS_MAX_INPUT_PORT_CNT];


    MI_U32 u32OutputPortNum;
    /// output buffer  count hold by user through MI_SYS_Get_Outputport_Buf();
    MI_U16 au16OutputPortBufUsrLockedNum[MI_SYS_MAX_OUTPUT_PORT_CNT];
    /// output buffer  count in user fifo list
    MI_U16 au16OutputPortBufInUsrFIFONum[MI_SYS_MAX_OUTPUT_PORT_CNT];
    //output buffer current hold by MI Driver
    MI_U16 au16OutputPortBufHoldByDrv[MI_SYS_MAX_OUTPUT_PORT_CNT];

    /// output total in used buffer count
    MI_U16 au16OutputPortBufTotalInUsedNum[MI_SYS_MAX_OUTPUT_PORT_CNT];
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
    unsigned int u32MagicNumber;
   atomic_t  ref_cnt;
   struct buf_allocation_ops_s ops;
   MI_SYS_BufInfo_t stBufInfo;
   void *private1;
   void *private2;
}MI_SYS_BufferAllocation_t;

struct mi_sys_Allocator_s;
typedef struct buf_allocator_ops_s
{
    MI_S32 (*OnRef)(struct mi_sys_Allocator_s *thiz);
    MI_S32 (*OnUnref)(struct mi_sys_Allocator_s *thiz);
    MI_S32 (*OnRelease)(struct mi_sys_Allocator_s *thiz);
    MI_SYS_BufferAllocation_t *(*alloc)(struct mi_sys_Allocator_s *pstAllocator, MI_SYS_BufConf_t *pstBufConfig);
    int (*suit_bufconfig)(struct mi_sys_Allocator_s *pstAllocator, MI_SYS_BufConf_t *pstBufConfig);
}buf_allocator_ops_t;

typedef struct mi_sys_Allocator_s
{
    atomic_t  ref_cnt;
    unsigned int u32MagicNumber;
    struct list_head list;
    buf_allocator_ops_t *ops;
}mi_sys_Allocator_t;

#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 mi_sys_allocator_proc_create(MI_U8 *allocator_proc_name,mi_sys_AllocatorProcfsOps_t *Ops,void *allocator_private_data);
void mi_sys_allocator_proc_remove_allocator(MI_U8 *entry_name);
#endif
void generic_allocation_on_ref(struct MI_SYS_BufferAllocation_s *pstBufAllocation);

void generic_allocation_on_unref(struct MI_SYS_BufferAllocation_s *pstBufAllocation);

//use this function to replace mi_sys_buf_mgr_init_std_allocation function
void generic_allocation_init(MI_SYS_BufferAllocation_t *pstBufAllocation,buf_allocation_ops_t *ops);

MI_S32 generic_allocator_on_ref(struct mi_sys_Allocator_s *pstAllocator);

MI_S32 generic_allocator_on_unref(struct mi_sys_Allocator_s *pstAllocator);
void generic_allocator_init(mi_sys_Allocator_t *pstAllocator, buf_allocator_ops_t *ops);

MI_SYS_DRV_HANDLE  mi_sys_RegisterDev(mi_sys_ModuleDevInfo_t *pstMouleInfo, mi_sys_ModuleDevBindOps_t *pstModuleBindOps , void *pUsrData
                                           #ifdef MI_SYS_PROC_FS_DEBUG
                                              , mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps
                                              , struct proc_dir_entry *dir_entry
                                           #endif
                                     );
MI_S32 mi_sys_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle);
#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 mi_sys_RegistCommand(MI_U8 *u8Cmd, MI_U8 u8MaxPara,
                    MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData),
                    MI_SYS_DRV_HANDLE hHandle);
#endif
MI_S32 mi_sys_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, mi_sys_ChnBufInfo_t *pstChnBufInfo);

typedef mi_sys_TaskIteratorCBAction_e (* mi_sys_TaskIteratorCallBack)(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData);

MI_S32 mi_sys_DevTaskIterator(MI_SYS_DRV_HANDLE miSysDrvHandle, mi_sys_TaskIteratorCallBack pfCallBack,void *pUsrData);
MI_S32 mi_sys_PrepareTaskOutputBuf(mi_sys_ChnTaskInfo_t *pstTask);
MI_S32 mi_sys_FinishAndReleaseTask(mi_sys_ChnTaskInfo_t *pstTask);
MI_S32 mi_sys_FinishAndReleaseTask_LL(mi_sys_ChnTaskInfo_t *pstTask);
MI_S32 mi_sys_RewindTask(mi_sys_ChnTaskInfo_t *pstTask);

MI_S32 mi_sys_WaitOnInputTaskAvailable(MI_SYS_DRV_HANDLE miSysDrvHandle , MI_S32 u32TimeOutMs);

MI_SYS_BufInfo_t *mi_sys_GetOutputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_SYS_BufConf_t *pstBufConfig , MI_BOOL *pbBlockedByRateCtrl);
MI_SYS_BufInfo_t *mi_sys_GetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_U32 u32Flags);
MI_S32 mi_sys_FinishBuf(MI_SYS_BufInfo_t*buf);
MI_S32 mi_sys_FinishBuf_LL(MI_SYS_BufInfo_t*buf);
MI_S32 mi_sys_RewindBuf(MI_SYS_BufInfo_t*buf);

MI_S32 mi_sys_EnableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);
MI_S32 mi_sys_DisableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);

MI_S32 mi_sys_EnableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);
MI_S32 mi_sys_DisableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);
MI_S32 mi_sys_SetOutputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf);
MI_S32 mi_sys_SetInputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf);

MI_S32 mi_sys_EnableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);
MI_S32 mi_sys_DisableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId);
MI_S32 mi_sys_SetInputPortSidebandMsg(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_U64 u64SidebandMsg);

MI_S32 mi_sys_SetReg (MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask);
MI_S32 mi_sys_GetReg (MI_U32 u32RegAddr, MI_U16 *pu16Value);

/*
N.B.
below MMAHeapName can only be NULL or real mma heap name, do not set it with random character string.
you can get mma heap name xxx from "mma_heap=xxx," of cat /proc/cmdline.
*/
MI_S32 mi_sys_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U32 u32Size ,MI_PHY *phyAddr);
MI_S32 mi_sys_MMA_Free(MI_PHY phyAddr);

void * mi_sys_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size , MI_BOOL bCache);
void mi_sys_UnVmap(void *pVirtAddr);
MI_S32 mi_sys_VFlushInvCache(void *pVirtAddr, MI_U32 u32Size);

MI_S32 mi_sys_EnsureOutportBkRefFifoDepth (MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_U32 u32BkRefFifoDepth);
MI_S32 mi_sys_MmapBufToUser(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_SYS_BufInfo_t *pstBufInfo, void **ppBufHandle);
MI_S32 mi_sys_UnmapBufToUser(MI_SYS_DRV_HANDLE miSysDrvHandle, void *pBufHandle, MI_SYS_BufInfo_t **ppstBufInfo);
MI_PHY mi_sys_Cpu2Miu_BusAddr(unsigned long long cpu_addr);
unsigned long long mi_sys_Miu2Cpu_BusAddr(MI_PHY miu_phy_addr);


//the following are APIs to support MI modules' customer allocator implement
MI_S32 mi_sys_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);
MI_S32 mi_sys_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);
MI_SYS_BufferAllocation_t* mi_sys_AllocMMALowLevelBufAllocation(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_SYS_BufConf_t *pstBufConfig);
MI_S32 mi_sys_NotifyPreProcessBuf(MI_SYS_BufInfo_t *buf);
/*if enabled lowlatency preprocess Mode, then:
   then this input port will receive a  buffer just after the pre-stage pipe element start to process this buffer
            if you need to delay X milliseconds before you could start to process this buffer, just set delayMs to be X
   at the same time, mi_sys will filter out the notify when the pre-stage finish process this buffer
*/
MI_S32 mi_sys_EnableLowlatencyPreProcessMode(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_BOOL bEnable, MI_U32 delayMs);

MI_SYS_BufInfo_t* mi_sys_InnerAllocBufFromVbPool(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32blkSize, MI_PHY *phyAddr);
MI_S32 mi_sys_FreeInnerVbPool(MI_SYS_BufInfo_t* pBufAllocation);
MI_S32 mi_sys_ConfDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId , MI_VB_PoolListConf_t  stPoolListConf);
MI_S32 mi_sys_RelDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId);
MI_U32 mi_sys_Get_Vdec_VBPool_UseStatus(void);
MI_S32 mi_sys_DropTask(mi_sys_ChnTaskInfo_t *pstTask);



#define DEBUG_YUV_KERN_API
#ifdef DEBUG_YUV_KERN_API
typedef  struct file* FILE_HANDLE;

//return value of the following API is not standard file operation return value.


/*
return :
FILE_HANDLE type in fact is a pointer,
NULL:fail
not NULL:success
*/
FILE_HANDLE open_yuv_file(const char *pathname, int open_for_write);

/*
return value:
0:success
-1:fail
*/
int read_yuv_file(FILE_HANDLE filehandle, MI_SYS_FrameData_t framedata);

/*
return value:
0:success
-1:fail
*/
int write_yuv_file(FILE_HANDLE filehandle, MI_SYS_FrameData_t framedata);

void close_yuv_file(FILE_HANDLE filehandle);

/*
return value:
0:success
-1:fail
*/
int reset_yuv_file(FILE_HANDLE filehandle);

/*
return value:
0:success
-1:fail
*/
int is_in_yuv_file_end(FILE_HANDLE filehandle);
#endif

#endif ///_MI_SYS_INTERNAL_H_
