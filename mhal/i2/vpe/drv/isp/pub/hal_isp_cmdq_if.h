#ifndef _DRV_ISP_CMDQ_
#define _DRV_ISP_CMDQ_

//typedef struct IspCmdqIf_t;
typedef struct IspCmdqIf_t
{
    void (*RegW)(struct IspCmdqIf_t* this,unsigned int uRiuAddr,unsigned short uVal);
    void (*RegWM)(struct IspCmdqIf_t* this,unsigned int uRiuAddr,unsigned short uVal,unsigned short nMask);
    int (*RegPollWait)(struct IspCmdqIf_t* this,unsigned int uRiuAddr,unsigned short uMask,unsigned short uVal,u32 u32Time);
}IspCmdqIf_t;

#endif
