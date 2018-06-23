#ifndef __CMD_SERVICE_HH__
#define __CMD_SERVICE_HH__
#include "mi_common.h"
typedef enum
{
   CMDQ_ID_VPE = 0,
   CMDQ_ID_DIPV,
   CMDQ_ID_H265_VENC0,
   CMDQ_ID_H265_VENC1,
   CMDQ_ID_H264_VENC0,
}MI_CMDQ_ID;

typedef struct _cmd_mload_interface_s
{

   //menuload ring buffer dynamic allocation service
  MI_PHY (*get_next_mload_ringbuf_write_ptr)(struct _cmd_mload_interface_s *cmdinf); 
  MI_PHY (*update_mload_ringbuf_read_ptr)(struct _cmd_mload_interface_s *cmdinf, MI_PHY next_read_ptr);  
  MI_S32 (*mload_fetch_buf)(struct _cmd_mload_interface_s *cmdinf, void *mload_buf, MI_U32 size, 
                                                                                MI_U16 u16Alignment, MI_PHY *phyRetAddr);

  MI_U16  (*check_buf_available)(struct _cmd_mload_interface_s *cmdinf, MI_U32 cmdq_available_size, MI_U32 mload_available_size);

  MI_S32 (*write_reg_cmdq_mask)(struct _cmd_mload_interface_s *cmdinf, MI_U32 reg_addr, MI_U16 value,  MI_U16 write_mask);
  MI_S32 (*write_reg_cmdq)(struct _cmd_mload_interface_s *cmdinf, MI_U32 reg_addr, MI_U16 value);
  MI_S32 (*cmdq_poll_reg_bits)(struct _cmd_mload_interface_s *cmdinf, MI_U32 reg_addr, MI_U16 value,  MI_U16 write_mask);
  MI_S32 (*cmdq_add_wait_event_cmd)(struct _cmd_mload_interface_s *cmdinf, MI_U16 event);
  
  MI_S32 (*kick_off_cmdq)(struct _cmd_mload_interface_s *cmdinf);
  
  MI_S32  (*is_cmdq_empty_idle)(struct _cmd_mload_interface_s *cmdinf);
 
  void *ctx;
}cmd_mload_interface;

cmd_mload_interface *get_sys_cmdq_service(MI_CMDQ_ID cmdqId, MI_BOOL bForceRIU);

#endif
