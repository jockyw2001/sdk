#ifndef HAL_RAWDMA_H
#define HAL_RAWDMA_H
#include <hal_dma.h>
int HalRawDma_GroupReset(u32 uGroup);
int HalRawDma_GlobalEnable(void);
int HalRawDma_Config(u32 uCh,WdmaCropParam_t *ptCrop,u32 uMiuBase);
int HalRawDma_Trigger(u32 uCh,WdmaTrigMode_e eMode);
int HalRawDma_GroupEnable(u32 uGroup);
int HalRawDma_GroupDisable(u32 uGroup);
#endif
