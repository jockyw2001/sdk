#ifndef _AUDIO_R2_SHM_H_
#define  _AUDIO_R2_SHM_H_

/**************************************************************************************************************************
*
*       SHARE MEMORY of DEC - R2
*
**************************************************************************************************************************/
#define MBOX_DEC_R2_CTRL        0x0388
    #define MBOX_DEC_R2_CTRL_LOCK_SHM_BIT                   0x0001          //[0]: lock "share memory" update
    
#define MBOX_DEC_R2_STATUS      0x03A8
    #define MBOX_DEC_R2_STATUS_UPDATING_SHM_BIT             0x0001          //[0]: R2 is updating "share memory" now.    

#define DEC_R2_SHM_DDR_OFFSET                               0x0A000
#define DEC_R2_SHM_DDR_SIZE                                 0x2000
#define MAX_ADEC                                                        2
#define MAX_ES                                                          4
#define SHM_PARAM_BOOT_MAGIC_ID                             0x12345677
typedef struct
{
    
    AUR2_ADEC_ES_INFO_SHM   adec_esInfo_shm[MAX_ES];
    AUR2_ADEC_ES_PARAM_SHM  adec_esParam_shm[MAX_ES];
    
    AUR2_ADEC_INFO_SHM  adec_info_shm[MAX_ADEC];
    AUR2_ADEC_PARAM_SHM adec_param_shm[MAX_ADEC];
#if 0
    AUR2_ADEC_Ch_INFO_SHM adec_chInfo_shm;
#endif
    MS_U32 RfSignalType;                                                                      //0: DVB, 1: ATSC, 2: ISDB

    unsigned int esPassThrough_delaySmp;
    unsigned int esPassThrough_pcmSmpRate;
    unsigned int esPassThrough_hdmiSmpRate;

    unsigned int uart_buf_base;
    unsigned int uart_buf_size;
    unsigned int uart_buf_wptr;

    MS_U32 SHM_magicID_end;
    MS_U32 Security_Check;
    MS_U32 DEC_R2_SHARE_MEM_Reserved[10];

} DEC_R2_SHARE_MEM;

#endif
