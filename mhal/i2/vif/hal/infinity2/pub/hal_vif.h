#ifndef __HAL_VIF__
#define __HAL_VIF__

#include "vif.h"
#include "drv_vif.h"

extern void __iomem *g_ptVIF;
extern void __iomem *g_ptVIF2;

u32 HalVifGetGroupByChn(VIF_CHANNEL_e ch);
u32 HalVifGetChnByGroup(u32 group);
VIF_STATUS_e HalVif_SetVifChanelBaseAddr(VIF_CHANNEL_e ch);
unsigned int HalVif_Bt656DetStatus0(VIF_CHANNEL_e ch);
unsigned int HalVif_Bt656DetStatus1(VIF_CHANNEL_e ch);
void HalVif_Bt656Field(VIF_CHANNEL_e ch, unsigned int bt656_fd);
void HalVif_Bt656Chanel(VIF_CHANNEL_e ch, unsigned int bt656_ch);
void HalVif_Bt1120Yc16bitMode(VIF_CHANNEL_e ch, VIF_SENSOR_BT656_FORMAT_e format);
void HalVif_TotalLineCount(VIF_CHANNEL_e ch, unsigned int line_cnt);
void HalVif_TotalPixelCount(VIF_CHANNEL_e ch, unsigned int line_cnt);
void HalVif_LineCropStart(VIF_CHANNEL_e ch, unsigned int crop_start);
void HalVif_LineCropEnd(VIF_CHANNEL_e ch, unsigned int crop_end);
void HalVif_PixCropEnd(VIF_CHANNEL_e ch, unsigned int crop_end);
void HalVif_CropEnable(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_PixCropStart(VIF_CHANNEL_e ch, unsigned int crop_start);
void HalVif_BT656HorizontalCropSize(VIF_CHANNEL_e ch, unsigned int bt656_horizontal_crop_size);
void HalVif_BT656VerticalCropSize(VIF_CHANNEL_e ch, unsigned int bt656_vertical_crop_size);
void HalVif_BT656VsyncInvert(VIF_CHANNEL_e ch, unsigned int bt656_vsync_inv);
void HalVif_BT656HsyncInvert(VIF_CHANNEL_e ch, unsigned int bt656_hsync_inv);
void HalVif_BT656VSDelay(VIF_CHANNEL_e ch, VIF_BT656_VSYNC_DELAY_e bt656_vs_delay);
//void HalVif_BT6568BitExt(VIF_CHANNEL_e ch, unsigned int bt656_8bit_ext);
//void HalVif_BT6568BitMode(VIF_CHANNEL_e ch, unsigned int bt656_8bit_mode);
void HalVif_BT656BitSwap(VIF_CHANNEL_e ch, unsigned int bt656_bit_swap);
void HalVif_BT656ChannelDetectSelect(VIF_CHANNEL_e ch, VIF_BT656_CHANNEL_SELECT_e bt656ch_det_sel);
void HalVif_BT656ChannelDetectEnable(VIF_CHANNEL_e ch, unsigned int bt656ch_det_en);
void HalVif_BT656InputSelect(VIF_CHANNEL_e ch, unsigned int bt656_input_sel);
void HalVif_BT656ClampEnable(VIF_CHANNEL_e ch, unsigned int bt656_clamp_en);
void HalVif_LiSt(VIF_CHANNEL_e ch, unsigned int line);
void HalVif_LiEnd(VIF_CHANNEL_e ch, unsigned int line);
void HalVif_SensorSWReset(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_IFStatusReset(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_SensorReset(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_SensorPowerDown(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_ChannelAlignment(VIF_CHANNEL_e ch);
void HalVif_SensorChannelEnable(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_HDRen(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_HDRSelect(VIF_CHANNEL_e ch, VIF_HDR_SOURCE_e src);
VIF_ONOFF_e HalVif_SensorMask(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_IfDeMode(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_EnSWStrobe(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_SWStrobe(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_StrobePolarity(VIF_CHANNEL_e ch, VIF_STROBE_POLARITY_e polarity);
void HalVif_StrobeStart(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_StrobeRef(VIF_CHANNEL_e ch, VIF_STROBE_VERTICAL_START_e startType);
void HalVif_StrobeEnd(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_EnHWStrobe(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_HWStrobeCNT(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_HWLongStrobeEndRef(VIF_CHANNEL_e ch, VIF_STROBE_VERTICAL_START_e startType);
void HalVif_HWStrobeMode(VIF_CHANNEL_e ch, VIF_STROBE_MODE_e strobMode);
void HalVif_SensorFormatLeftSht(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_SensorBitSwap(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_SensorHsyncPolarity(VIF_CHANNEL_e ch, VIF_SENSOR_POLARITY_e polarity);
void HalVif_SensorVsyncPolarity(VIF_CHANNEL_e ch, VIF_SENSOR_POLARITY_e polarity);
void HalVif_SensorFormat(VIF_CHANNEL_e ch, VIF_SENSOR_FORMAT_e format);
void HalVif_SensorRgbIn(VIF_CHANNEL_e ch, VIF_SENSOR_INPUT_FORMAT_e format);
void HalVif_SensorFormatExtMode(VIF_CHANNEL_e ch, VIF_SENSOR_BIT_MODE_e mode);
void HalVif_SensorYc16Bit(VIF_CHANNEL_e ch, VIF_SENSOR_YC_INPUT_FORMAT_e format);
void HalVif_SensorVsDly(VIF_CHANNEL_e ch, VIF_SENSOR_VS_DELAY_e delay);
void HalVif_SensorHsDly(VIF_CHANNEL_e ch, VIF_SENSOR_HS_DELAY_e delay);
//void HalVif_ParWidth(VIF_CHANNEL_e ch, unsigned int width);
void HalVif_Bt601ByteintEn(VIF_ONOFF_e OnOff);
void HalVif_Bt601ChSel(VIF_CHANNEL_e ch, unsigned int channel);
void HalVif_Bt656Yc16bitMode(VIF_CHANNEL_e ch, VIF_SENSOR_BT656_FORMAT_e format);
void HalVif_DebugSel(unsigned int sel);
void HalVif_CaptureEn(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff);
void HalVif_IrqMask(VIF_CHANNEL_e ch, unsigned int mask);
void HalVif_IrqForce(VIF_CHANNEL_e ch, unsigned int mask);
void HalVif_IrqClr(VIF_CHANNEL_e ch, unsigned int mask);
void HalVif_IrqClr1(VIF_CHANNEL_e ch, unsigned int mask);
void HalVif_Vif2IspLineCnt0(VIF_CHANNEL_e ch, unsigned int LineCnt);
void HalVif_Vif2IspLineCnt1(VIF_CHANNEL_e ch, unsigned int LineCnt);
unsigned int HalVif_IrqFinalStatus(VIF_CHANNEL_e ch);
unsigned int HalVif_IrqRawStatus(VIF_CHANNEL_e ch);
void HalVif_SelectSource(VIF_CHANNEL_e ch,VIF_CHANNEL_SOURCE_e src);
void HalVif_SensorPclkPolarity(VIF_CHANNEL_e ch, VIF_SENSOR_POLARITY_e polarity);
void HalVif_Crop(VIF_CHANNEL_e ch, unsigned int x,unsigned int y,unsigned int width,unsigned int height);
void HalVif_SetPclkSource(VIF_CHANNEL_e ch,VifPclk_e eClk, u8 inverse, u8 gating);
void HalVif_SetMCLK(VIF_CHANNEL_e ch,VifMclk_e eClk);
void HalVif_EnableParallelIF(VIF_CHANNEL_e ch,u8 en);

void HalVif_IrqUnMask(VIF_CHANNEL_e ch, u32 mask);
s32 HalVifDataIeConfig(u32 group, u32 nValue);
void HalVif_Ccir8BitDataPadConfig(VIF_CHANNEL_e ch,u32 nValue);
void HalVif_Ccir16BitDataPadConfig(VIF_CHANNEL_e ch,u32 nValue);
void HalVif_CcirCtrlPadConfig(VIF_CHANNEL_e ch,u32 nValue);
void HalVif_CcirPclkPadConfig(VIF_CHANNEL_e ch,u32 nValue);
void HalVif_EnableIrq(VIF_CHANNEL_e ch);
void HalVif_GetVifInfo(VIF_CHANNEL_e ch,VifInfo_t *info);

void HalVif_GlobalInit(void);

u16 HalVif_FrameStartIrqFinalStatus(void);
#define DrvVif_FrameStartIrqFinalStatus() HalVif_FrameStartIrqFinalStatus()
void HalVif_FrameStartIrqMask(u16 ch_bitmask, u8 on_off);
#define DrvVif_FrameStartIrqMask(a, b) HalVif_FrameStartIrqMask(a, b)
void HalVif_FrameStartIrqClr(u16 ch_bitmask);
u32 HalVifGetHDRChnPair(VIF_CHANNEL_e ch);

#define DrvVif_FrameStartIrqClr(a) HalVif_FrameStartIrqClr(a)
#endif
