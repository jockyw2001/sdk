#ifndef _MI_SYS__H_
#define _MI_SYS__H_

#define __MI_SYS_MAGIC_NUM__ 0x4D535953
#define __MI_SYS_CHN_MAGIC_NUM__ 0x4D43484E
#define __MI_SYS_INPUTPORT_MAGIC_NUM__ 0x4D5F494E
#define __MI_SYS_OUTPORT_MAGIC_NUM__ 0x4D4F5554

struct MI_SYS_BufferAllocation_s;

struct MI_SYS_BufRef_s;

MI_SYS_BufRef_t *mi_sys_create_bufref(MI_SYS_BufferAllocation_t *pstBufAllocation,MI_SYS_BufConf_t *pstBufConfig, OnBufRefRelFunc onRelCB, void *pCBData);
void mi_sys_release_bufref(MI_SYS_BufRef_t *pstBufRef);
MI_SYS_BufRef_t *mi_sys_dup_bufref(MI_SYS_BufRef_t *pstBufRef);


void mi_sys_init_buf_queue(MI_SYS_BufferQueue_t*pstQueue);
void mi_sys_add_to_queue_head( MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue);
void mi_sys_add_to_queue_tail( MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue);
MI_SYS_BufRef_t *mi_sys_remove_from_queue_head( MI_SYS_BufferQueue_t*pstQueue);
MI_SYS_BufRef_t * mi_sys_remove_from_queue_tail(MI_SYS_BufferQueue_t*pstQueue);
void mi_sys_remove_from_queue( MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue);

struct MI_SYS_OutputPort_s;
struct MI_SYS_Channel_s;
typedef struct MI_SYS_InputPort_s
{
   unsigned int u32MagicNumber;
   struct list_head bind_relationship_list;
   struct MI_SYS_Channel_s *pstChannel;
   int port_index;
   MI_SYS_BufferQueue_t usr_inject_buf_queue; 
   MI_SYS_BufferQueue_t bind_input_buf_queue;
   MI_SYS_BufferQueue_t workingQueue; 
   struct MI_SYS_OutputPort_s *pst_bind_peer_output_port;
   mi_sys_Allocator_t *pstCUSBufAllocator;
   //for frame rate control when binded
   int src_frmrate;
   int dst_frmrate;
   int cur_rc_param;
}MI_SYS_InputPort_t;


typedef struct MI_SYS_OutputPort_s
{
   unsigned int u32MagicNumber;
   struct MI_SYS_Channel_s *pstChannel;
   MI_SYS_BufferQueue_t usr_get_fifo_buf_queue; 
   atomic_t usr_locked_buf_cnt;
   MI_U32 usr_fifo_count;

   struct semaphore binded_inputlist_semlock;
   struct list_head bind_peer_inputport_list;//output

   MI_SYS_BufferQueue_t workingQueue; 
   mi_sys_Allocator_t *pstCUSBufAllocator;
}MI_SYS_OutputPort_t;

struct MI_SYS_MOD_DEV_s;
typedef struct MI_SYS_Channel_s
{
   unsigned int u32MagicNumber;
   struct MI_SYS_MOD_DEV_s *pst_mod_dev;
   MI_SYS_Allocator_Collection_t stAllocatorCollection;
   MI_SYS_InputPort_t *input_ports[MI_SYS_MAX_INPUT_PORT_CNT];
   MI_SYS_OutputPort_t output_ports[MI_SYS_MAX_OUTPUT_PORT_CNT];
}MI_SYS_Channel_t;

typedef struct MI_SYS_MOD_DEV_s
{
    unsigned int u32MagicNumber;
    struct list_head list;
    MI_U32 u32ModuleId;
    MI_U32 u32DevId;
    MI_U32 u32InputPortNum;
    MI_U32 u32OutputPortNum;
    MI_U32 u32DevChnNum;
    mi_sys_ModuleBindOps_t ops;
    MI_SYS_Allocator_Collection_t stAllocatorCollection;
    void *pUsrData;
    MI_SYS_Channel_t  channels[0];
}MI_SYS_MOD_DEV_t;




MI_SYS_InputPort_t *mi_sys_create_input_port( MI_SYS_Channel_s *pstChannel);
void mi_sys_destroy_input_port( MI_SYS_InputPort_t *pstInputPort);

MI_SYS_OutputPort_t *mi_sys_create_output_port( MI_SYS_Channel_s *pstChannel);
void mi_sys_destroy_output_port( MI_SYS_OutputPort_t *pstOutputPort);


int mi_sys_init_channel( MI_SYS_Channel_s *pstChannel, MI_SYS_MOD_DEV_t* pstModDev);
void mi_sys_deinit_channel( MI_SYS_Channel_s *pstChannel);



#endif
