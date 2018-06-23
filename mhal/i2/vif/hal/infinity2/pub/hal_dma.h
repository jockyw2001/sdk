#ifndef HAL_DMA_H
#define HAL_DMA_H
#include <vif_datatype.h>

typedef struct
{
    u32 uX;
    u32 uY;
    u32 uW;
    u32 uH;
}WdmaCropParam_t;

typedef enum
{
    WDMA_TRIG_SINGLE = 0,
    WDMA_TRIG_CONTINUE = 1,
    WDMA_TRIG_STOP = 2,
}WdmaTrigMode_e;

typedef struct
{
    u32 uWidth;
    u32 uHeight;

}WdmaInfo_t;

typedef enum
{
    WDMA_LINE_CNT_MAIN_Y = 0x1,
    WDMA_LINE_CNT_MAIN_C = 0x2,
    WDMA_LINE_CNT_SUB_Y = 0x4,
    WDMA_LINE_CNT_SUB_C = 0x8,
}WdmaLineCntType_e;

void HalDma_Init(void);
void HalDma_Uninit(void);
int HalDma_GlobalEnable(void);
int HalDma_Config(u32 uCh,WdmaCropParam_t *ptCrop,u32 uMiuBaseY,u32 uMiuBaseC);
int HalDma_ConfigSub(u32 uCh,WdmaCropParam_t *ptCrop,u32 uMiuBaseY,u32 uMiuBaseC);
int HalDma_Trigger(u32 uCh,WdmaTrigMode_e eMode);
int HalDma_TriggerSub(u32 uCh,WdmaTrigMode_e eMode);
int HalDma_EnableIrq(u32 nCh);
int HalDma_DisableIrq(u32 nCh);
int HalDma_DmaDoneCmdQEnableIrq(u32 nCh);
int HalDma_DmaDoneCmdQDisableIrq(u32 nCh);
int HalDma_DmaMaskEnable(u32 nCh,u8 enable);
int HalDma_EnableGroup(u32 uGroup);
int HalDma_DisableGroup(u32 uGroup);
int HalDma_ConfigGroup(u32 uGroup,u32 uMaxChns);
int HalDma_MaskOutput(u32 nCh,u8 uMask);
int HalDma_MaskOutputSub(u32 uCh,u8 uMask);
int HalDma_SetOutputAddr(u32 uCh,u32 uOutAddrY,u32 uOutAddrC,u32 uPitch);
int HalDma_SetOutputAddrSub(u32 uCh,u32 uOutAddrY,u32 uOutAddrC,u32 uPitch);

unsigned int HalDma_FifoFullIrqFinalStatus(VIF_CHANNEL_e ch);
void HalDma_FifoFullIrqMask(VIF_CHANNEL_e ch, unsigned int mask);
void HalDma_FifoFullIrqUnMask(VIF_CHANNEL_e ch, unsigned int mask);
void HalDma_FifoFullIrqClr(VIF_CHANNEL_e ch, unsigned int mask);

void HalDma_GetDmaInfo(VIF_CHANNEL_e ch,WdmaInfo_t *info);
void HalDma_GetSubDmaInfo(VIF_CHANNEL_e ch,WdmaInfo_t *info);
void HalDma_ResetDMA(VIF_CHANNEL_e ch);
void HalDma_CMDQ_SyncResetDMA(VIF_CHANNEL_e ch);
void HalDma_SetDmaY_WreqThreshold(VIF_CHANNEL_e ch, u8 val);
void HalDma_SetDmaC_WreqThreshold(VIF_CHANNEL_e ch, u8 val);
void HalDma_SetSubDmaY_WreqThreshold(VIF_CHANNEL_e ch, u8 val);
void HalDma_SetSubDmaC_WreqThreshold(VIF_CHANNEL_e ch, u8 val);
void HalDma_SetDmaY_BurstNumber(VIF_CHANNEL_e ch, u8 val);
void HalDma_SetDmaC_BurstNumber(VIF_CHANNEL_e ch, u8 val);
void HalDma_SetSubDmaY_BurstNumber(VIF_CHANNEL_e ch, u8 val);
void HalDma_SetSubDmaC_BurstNumber(VIF_CHANNEL_e ch, u8 val);
u8 HalDma_LineCntIrqFinalStatus(VIF_CHANNEL_e ch);
#define DrvVif_LineCntIrqFinalStatus(a) HalDma_LineCntIrqFinalStatus(a)
void HalDma_LineCntIrqMask(VIF_CHANNEL_e ch, u8 mask, u8 on_off);
#define DrvVif_LineCntIrqMask(a, b, c) HalDma_LineCntIrqMask(a, b, c)
void HalDma_LineCntIrqClr(VIF_CHANNEL_e ch, u8 mask);
#define DrvVif_LineCntIrqClr(a, b) HalDma_LineCntIrqClr(a, b)
void HalDma_SetDmaLineCnt(VIF_CHANNEL_e ch, u16 cntY, u16 cntC, u16 cnty, u16 cntc);
#endif
