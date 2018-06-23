#if defined(CONFIG_TRACEPOINTS) && (CONFIG_TRACEPOINTS == 1)
#undef TRACE_SYSTEM
#define TRACE_SYSTEM mi_sys

#if !defined(_TRACE_MI_SYS_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MI_SYS_TRACE_H

#include <linux/tracepoint.h>

DECLARE_EVENT_CLASS(bufqueue_action_class,

    TP_PROTO(MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t *pstQueue),

    TP_ARGS(pstBufRef, pstQueue),

    TP_STRUCT__entry(
            __field(unsigned long long,    u64Pts                )
            __field(void *, pstBufRef                            )
            __field(int,    queue_buf_count                      )
            __field(unsigned char,    module_id                  )
            __field(unsigned char,    queue_id                   )
            __field(unsigned char,    device_port                )
            __field(unsigned char,    channel_io                 )
    ),

    TP_fast_assign(
            __entry->u64Pts                 = pstBufRef?pstBufRef->bufinfo.u64Pts:MI_SYS_INVALID_PTS;
            __entry->pstBufRef              = pstBufRef;
            __entry->queue_buf_count        = pstQueue->queue_buf_count;
            __entry->module_id              = pstQueue->module_id;
            __entry->queue_id               = pstQueue->queue_id;
            __entry->device_port            = (pstQueue->device_id << 5) | (pstQueue->port & 0x1f);
            __entry->channel_io             = pstQueue->channel_id | (pstQueue->io << 7);
    ),

    TP_printk("[Module:%s][Queue:%s][Dev:Port:%d:%d(%s)][CHN:%d][PTS:0x%llu][Ref:%p][Count:%d]",
            __print_symbolic(__entry->module_id,
                {E_MI_MODULE_ID_VDEC , "VDE" },
                {E_MI_MODULE_ID_VENC , "VEN" },
                { E_MI_MODULE_ID_DISP , "DSP" },
                { E_MI_MODULE_ID_VIF , "VIF" },
                { E_MI_MODULE_ID_AI , "AI" },
                { E_MI_MODULE_ID_AO , "AO" },
                { E_MI_MODULE_ID_RGN , "RGN" },
                { E_MI_MODULE_ID_VPE , "VPE" },
                { E_MI_MODULE_ID_DIVP , "DIP" },
                { E_MI_MODULE_ID_GFX , "GFX" },
                { E_MI_MODULE_ID_IVE , "IVE" },
                { E_MI_MODULE_ID_IAE , "IAE" },
                { E_MI_MODULE_ID_MD , "MD" },
                { E_MI_MODULE_ID_OD , "OD" },
                { E_MI_MODULE_ID_VDF , "VDF" },
                { E_MI_MODULE_ID_VDISP , "VDP" },
                { E_MI_MODULE_ID_FB , "FB" },
                { E_MI_MODULE_ID_SYS , "SYS" }),
            __print_symbolic(__entry->queue_id,
                { MI_SYS_QUEUE_ID_USR_INJECT , "UsrInject" },
                { MI_SYS_QUEUE_ID_USR_GET_FIFO , "UsrGetFifo" },
                { MI_SYS_QUEUE_ID_BIND_INPUT , "BindInput" },
                { MI_SYS_QUEUE_ID_DRIVER_BACK_REF , "DrvBkRef" },
                { MI_SYS_QUEUE_ID_WORKING , "Working" }),
            __entry->device_port >> 5,
            __entry->device_port & 0x1f,
            __print_symbolic(__entry->channel_io & (0x1 << 7),
                { 0x00 , "IN"  },
                { 0x80 , "OUT" }),
            __entry->channel_io & 0x7f,
            __entry->u64Pts, __entry->pstBufRef, __entry->queue_buf_count)
);

DEFINE_EVENT(bufqueue_action_class, mi_sys_bufqueue_add_head,

    TP_PROTO(MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t *pstQueue),

    TP_ARGS(pstBufRef, pstQueue)
);

DEFINE_EVENT(bufqueue_action_class, mi_sys_bufqueue_add_tail,

    TP_PROTO(MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t *pstQueue),

    TP_ARGS(pstBufRef, pstQueue)
);

DEFINE_EVENT(bufqueue_action_class, mi_sys_bufqueue_rm_head,

    TP_PROTO(MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t *pstQueue),

    TP_ARGS(pstBufRef, pstQueue)
);

DEFINE_EVENT(bufqueue_action_class, mi_sys_bufqueue_rm_tail,

    TP_PROTO(MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t *pstQueue),

    TP_ARGS(pstBufRef, pstQueue)
);

#endif /* _TRACE_MI_SYS_TRACE_H */
/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE mi_sys_trace
#include <trace/define_trace.h>
#else
#define trace_mi_sys_bufqueue_add_head(...) /* nop */
#define trace_mi_sys_bufqueue_add_tail(...) /* nop */
#define trace_mi_sys_bufqueue_rm_head(...) /* nop */
#define trace_mi_sys_bufqueue_rm_tail(...) /* nop */
#endif
