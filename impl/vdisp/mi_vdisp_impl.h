#ifndef _VDISPIMPL_H_
#define _VDISPIMPL_H_

#define VDISP_IMPL_DBG 0
#define VDISP_IMPL_DBG_PROCESS 0
#define VDISP_SUPPORT_OVERLAYINPUTPORT 1

#define INT_MAX		((int)(~0U>>1))
#define VDISP_INVALID_PTR ((void*)((unsigned long)(-1)))
#define VDISP_BUFQUEUE_DEPTH 4

#define VDISP_VALID_MODID(a) (E_MI_MODULE_ID_VDISP==(a))
#define VDISP_VALID_DEVID(a) ((unsigned int)(a)<VDISP_MAX_DEVICE_NUM)
#define VDISP_VALID_CHNID(a) ((unsigned int)(a)<VDISP_MAX_CHN_NUM)
#if VDISP_SUPPORT_OVERLAYINPUTPORT
#define VDISP_TOTAL_INPUTPORT_NUM (VDISP_MAX_OVERLAYINPUTPORT_NUM+VDISP_MAX_INPUTPORT_NUM)
#define VDISP_IS_OVERLAYINPUTPORTID(a) ((a)>=VDISP_MAX_INPUTPORT_NUM && (a)<VDISP_TOTAL_INPUTPORT_NUM)
#else
#define VDISP_TOTAL_INPUTPORT_NUM VDISP_MAX_INPUTPORT_NUM
#define VDISP_IS_OVERLAYINPUTPORTID(a) 0
#endif
#define VDISP_VALID_INPUTPORTID(a) ((unsigned int)(a)<VDISP_TOTAL_INPUTPORT_NUM)
#define VDISP_VALID_OUTPUTPORTID(a) ((unsigned int)(a)<VDISP_MAX_OUTPUTPORT_NUM)

#define VDISP_PORTID_TO_OVERLAYIDX(a) ((a)-VDISP_OVERLAYINPUTPORTID)


typedef enum
{
    VDISP_INPUTPORT_UNINIT,
    VDISP_INPUTPORT_INIT,
    VDISP_INPUTPORT_ENABLED,
    VDISP_INPUTPORT_DISABLED,
}vdisp_inputport_status_e;

typedef enum
{
    VDISP_DEVICE_UNINIT,
    VDISP_DEVICE_INIT,
    VDISP_DEVICE_START,
    VDISP_DEVICE_STOP,
}vdisp_device_status_e;

typedef enum
{
    VDISP_WORKER_WAKE_VSYNC=0,
    VDISP_WORKER_PAUSE,
    VDISP_WORKER_PAUSED,
}vdisp_worker_wake_event_e;

typedef struct
{
    MI_VDISP_InputPortAttr_t attr;
    vdisp_inputport_status_e status;
    MI_SYS_ChnPort_t inputport;
    int bbind;
    int sub_buf_id;
    MI_SYS_ChnPort_t bindport;
    unsigned long sync_buf_id;
    MI_U64 u64Try;
    MI_U64 u64RecvOk;
}vdisp_inputport_t;

#if VDISP_SUPPORT_OVERLAYINPUTPORT
typedef struct
{
    vdisp_inputport_t *overlayinputport;
    MI_SYS_BufInfo_t *overlaybufinfo;
}vdisp_overlayinfo_t;
#endif

typedef struct
{
    int binited;
    MI_VDISP_OutputPortAttr_t attr;
    MI_SYS_ChnPort_t outputport;
    int bbind; //is bound
    MI_SYS_ChnPort_t bindport;
    MI_U64 interval;
    MI_U64 u64SendOk;
}vdisp_outputport_t;

typedef struct
{
    vdisp_inputport_t inputport[VDISP_TOTAL_INPUTPORT_NUM];
    vdisp_outputport_t outputport; //only one output port
    struct task_struct *work_thread;
    unsigned long wakeevent;
    sba_buf_queue_t bufqueue;
    sba_bufhead_t *prev_buf;
#if VDISP_SUPPORT_OVERLAYINPUTPORT
    vdisp_overlayinfo_t overlayinfo[VDISP_MAX_OVERLAYINPUTPORT_NUM];
#endif
    vdisp_device_status_e status;
    MI_SYS_DRV_HANDLE devhandle;
    atomic_t allocator_cnt;
    MI_U64 u64next_stc; //us
    struct mutex mtx;
}vdisp_device_t;

typedef struct
{
    int binited;
    vdisp_device_t dev[VDISP_MAX_DEVICE_NUM];
}vdisp_module_t;

typedef struct
{
    void *src_vaddr;
    phys_addr_t src_paddr;
    MI_U32 src_stride;
    void *dst_vaddr;
    phys_addr_t dst_paddr;
    MI_U32 dst_stride;
    MI_U32 width;
    MI_U32 height;
}vdisp_copyinfo_plane_t;

typedef struct
{
    int plane_num;
    vdisp_copyinfo_plane_t plane[3];
    MI_SYS_BufInfo_t* finish_buf;
}vdisp_copyinfo_t;

vdisp_device_t* MI_VDISP_IMPL_GetDevice(MI_VDISP_DEV DevId);
static inline int MI_VDISP_IMPL_GetBufPlaneNum(MI_SYS_PixelFormat_e ePixelFmt)
{
    switch(ePixelFmt){
    case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
        return 1;
    case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        return 2;
    default:
        MI_SYS_BUG();
    }
    return 1;
}
static inline int MI_VDISP_IMPL_IsSupPixelFmt(MI_SYS_PixelFormat_e ePixelFormat)
{
    if(ePixelFormat!=E_MI_SYS_PIXEL_FRAME_YUV422_YUYV
        && ePixelFormat!=E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
        return 0;
    return 1;
}
static inline int MI_VDISP_IMPL_IsSupBufInfo(MI_SYS_BufInfo_t *bufinfo)
{
    if(bufinfo->eBufType!=E_MI_SYS_BUFDATA_FRAME)
        return 0;
    if(!MI_VDISP_IMPL_IsSupPixelFmt(bufinfo->stFrameData.ePixelFormat))
        return 0;
    return 1;
}
static inline int MI_VDISP_IMPL_IsSupBufConf(MI_SYS_BufConf_t *stBufConfig)
{
    if(stBufConfig->eBufType!=E_MI_SYS_BUFDATA_FRAME)
        return 0;
    if(!MI_VDISP_IMPL_IsSupPixelFmt(stBufConfig->stFrameCfg.eFormat))
        return 0;
    return 1;
}

static inline unsigned long MI_VDISP_IMPL_GetBufsizeFromBufinfo(MI_SYS_BufInfo_t *bufinfo)
{
    if(bufinfo->stFrameData.ePixelFormat==E_MI_SYS_PIXEL_FRAME_YUV422_YUYV){
        return (bufinfo->stFrameData.u32Stride[0]*bufinfo->stFrameData.u16Height);
    }else if(bufinfo->stFrameData.ePixelFormat==E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420){
        return (bufinfo->stFrameData.u32Stride[0]*bufinfo->stFrameData.u16Height*3/2);
    }
    return 0;
}
void MI_VDISP_IMPL_InitBufInfoFromSubBuf(MI_SYS_BufInfo_t *bufinfo,
    sba_subbufhead_t* subbufhd, MI_U64 u64TargetPts);
void MI_VDISP_IMPL_InitBufHeadFromBufInfo(
    sba_bufhead_t * bufhd,
    MI_SYS_BufInfo_t *bufinfo,
    uint64_t u64pts,
    uint32_t pts_tolerance);
MI_SYS_BufInfo_t * MI_VDISP_IMPL_GetBufInfoFromBufhead(sba_bufhead_t *bufhd);

MI_S32 MI_VDISP_IMPL_Init(void);
MI_S32 MI_VDISP_IMPL_Exit(void);

MI_S32 MI_VDISP_IMPL_OpenDevice(MI_VDISP_DEV DevId);
MI_S32 MI_VDISP_IMPL_CloseDevice(MI_VDISP_DEV DevId);

MI_S32 MI_VDISP_IMPL_SetOutputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_OutputPortAttr_t *pstOutputPortAttr);
MI_S32 MI_VDISP_IMPL_GetOutputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_OutputPortAttr_t *pstOutputPortAttr);

MI_S32 MI_VDISP_IMPL_SetInputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_InputPortAttr_t *pstInputPortAttr);
MI_S32 MI_VDISP_IMPL_GetInputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_InputPortAttr_t *pstInputPortAttr);

MI_S32 MI_VDISP_IMPL_EnableInputPort(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId);
MI_S32 MI_VDISP_IMPL_DisableInputPort(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId);

MI_S32 MI_VDISP_IMPL_StartDev(MI_VDISP_DEV DevId);
MI_S32 MI_VDISP_IMPL_StopDev(MI_VDISP_DEV DevId);

#endif
