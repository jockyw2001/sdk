#include "cmdq_service.h"

cmd_mload_interface cmdq_ops = 
{
    cmd_xxxx,
    cmd_xxxx,

    
}

cmd_mload_interface riu_ops = 
{
    riu_xxxx,
    riu_xxxx,

    
}

cmd_mload_interface *get_sys_cmdq_service(MI_CMDQ_ID cmdqId, MI_BOOL bForceRIU)
{

    if(bForceRIU)
        return &riu_ops;
   else
              return &cmdq_ops;
}

