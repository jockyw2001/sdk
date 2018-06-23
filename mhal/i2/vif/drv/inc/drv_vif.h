#ifndef __DRV_VIF__
#define __DRV_VIF__
#include <mhal_vif_datatype.h>
#include <vif_datatype.h>
#include <linux/interrupt.h>

#if 0
void DrvVif_WaitFSInts(VIF_CHANNEL_e ch);
void DrvVif_WaitFEInts(VIF_CHANNEL_e ch);
int DrvVif_CreateFSTask(void);
int DrvVif_DeleteFSTask(void);
int DrvVif_CreateFETask(void);
int DrvVif_DeleteFETask(void);
#endif
int DrvVif_PatGenCfg(VIF_CHANNEL_e ch, VifPatGenCfg_t *PatGenCfg);
int DrvVif_SensorFrontEndCfg(VIF_CHANNEL_e ch, VifSensorCfg_t *sensorcfg);
int DrvVif_SensorReset(VIF_CHANNEL_e ch,int reset);
int DrvVif_SensorPdwn(VIF_CHANNEL_e ch, int Pdwn);
int DrvVif_ChannelEnable(VIF_CHANNEL_e ch, bool ben);
int DrvVif_ConfigParallelIF(VIF_CHANNEL_e ch,
                            VIF_SENSOR_INPUT_FORMAT_e svif_sensor_in_format,
                            VIF_SENSOR_FORMAT_e PixDepth,
                            VIF_CLK_POL PclkPol,
                            VIF_CLK_POL VsyncPol,
                            VIF_CLK_POL HsyncPol,
                            VIF_PIN_POL RstPol,
                            u32 CropX,
                            u32 CropY,
                            u32 CropW,
                            u32 CropH,
                            u32 CropEnable,
                            VifPclk_e Mclk
                            );
int DrvVif_ConfigMipiIF(VIF_CHANNEL_e ch,
                            VIF_SENSOR_INPUT_FORMAT_e svif_sensor_in_format,
                            VIF_SENSOR_FORMAT_e PixDepth,
                            //VIF_CLK_POL PclkPol,
                            //VIF_CLK_POL VsyncPol,
                            //VIF_CLK_POL HsyncPol,
                            VIF_PIN_POL RstPol,
                            u32 CropX,
                            u32 CropY,
                            u32 CropW,
                            u32 CropH,
                            u32 CropEnable,
                            VifPclk_e Mclk,
                            VifHdrMode_e hdr
                            );
int DrvVif_ConfigBT656IF(SENSOR_PAD_GROUP_e pad_group, VIF_CHANNEL_e ch, VifBT656Cfg_t *pvif_bt656_cfg);
int DrvVif_SetChannelSource(VIF_CHANNEL_e ch, VIF_CHANNEL_SOURCE_e svif_ch_src);

/*@brief MHAL driver interface and
   ARM to 8051 ring buffer control,
   queue, dequeue interface
*/

#define VIF_RING_QUEUE_SIZE 8   /**< Ring buffer size of each channel*/
#define VIF_PORT_NUM  32        /**< Number of channel*/

#define VIF_CHN_MAIN 0
#define VIF_CHN_SUB 1

typedef struct
{
    u64 count64;
    u32 prev_count;
}VifnPTs;

/*! @brief ring buffer element*/
typedef struct
{
    u32 nPhyBufAddrY_H;  /**<Highbyte physical buffer Y address 32k alignement*/
    u32 nPhyBufAddrY_L;  /**<Lowbyte physical buffer Y address 32k alignement*/
    u32 nPhyBufAddrC_H;  /**<Highbyte physical buffer C address 32k alignement*/
    u32 nPhyBufAddrC_L;  /**<Lowbyte physical buffer C address 32k alignement*/

    u16 PitchY;        /**< Width +31/32*/
    u16 PitchC;        /**< Width +31/32*/

    u16 nCropX;       /**< crop start x*/
    u16 nCropY;       /**< crop start y*/
    u16 nCropW;       /**< crop start width*/
    u16 nCropH;       /**< crop start height*/

    u16 nHeightY;      /**< Y plane height*/
    u16 nHeightC;      /**< C plane height*/

    u32 size;         /**< Frame size*/
    u32 nPTS;         /**< timestamp in 90KHz*/
    u32 nMiPriv;      /**< MI private*/

    u8 nStatus;       /**< Vif buffer status*/
    u8 eFieldType;   /**<interlaced sensor's fieldID>*/
    /**Extension*/
    u8 tag;
    /*u8 reserve[1];*/
	u8 Padding[1];    /*4 byte alignment for arm performance*/

}__attribute__((packed, aligned(4))) VifRingBufElm_mcu;

/*! @brief 32 channels ring buffer*/
typedef struct
{
    u8 nEnable;
    u8 nReadIdx;
    u8 nWriteIdx;
    u8 pre_nReadIdx;
    u32 nFPS_bitMap;
    u32 nFrameStartCnt;
    u32 nFrameDoneCnt;
    u32 nFrameDoneSubCnt;
    u32 nDropFrameCnt;
    u32 eFieldIdErr;
    VifRingBufElm_mcu data[VIF_RING_QUEUE_SIZE];
}__attribute__((packed, aligned(4))) VifRingBufShm_t;

/** @brief ARM to 8051 ring buffer binding.
@param[in] pstRingBuf physical address of ring buffer
@retval 0 if successed or E_HAL_VIF_ERROR if error occurs.
@remark MI call this function to bind 8051 ring buffer
*/

s32 DrvVif_Init(void);
s32 DrvVif_Deinit(void);
s32 DrvVif_Reset(void);

s32 DrvVif_DevSetConfig(MHal_VIF_DEV u32VifDev, MHal_VIF_DevCfg_t *pstDevAttr, MHalAllocPhyMem_t *pstAlloc);
s32 DrvVif_DevEnable(MHal_VIF_DEV u32VifDev);
s32 DrvVif_DevDisable(MHal_VIF_DEV u32VifDev);

s32 DrvVif_ChnSetConfig(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnCfg_t *pstAttr);
s32 DrvVif_ChnEnable(MHal_VIF_CHN u32VifChn);
s32 DrvVif_ChnDisable(MHal_VIF_CHN u32VifChn);
s32 DrvVif_ChnQuery(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnStat_t *pstStat);

s32 DrvVif_SubChnSetConfig(MHal_VIF_CHN u32VifChn, MHal_VIF_SubChnCfg_t *pstAttr);
s32 DrvVif_SubChnEnable(MHal_VIF_CHN u32VifChn);
s32 DrvVif_SubChnDisable(MHal_VIF_CHN u32VifChn);
s32 DrvVif_SubChnQuery(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnStat_t *pstStat);

/** @brief Push new frame buffer into VIF wait queue
@param[in] u32VifChn VIF channel
@retval 0 if successed or E_HAL_VIF_ERROR if error occurs.
*/
s32 DrvVif_QueueFrameBuffer(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, const MHal_VIF_RingBufElm_t *ptFbInfo);

/** @brief Query VIF for ready frame
@param[in] u32VifChn VIF channel
@param[out] pNumBuf number of frame ready
@retval 0 if successed or E_HAL_VIF_ERROR if error occurs.
*/
s32 DrvVif_QueryFrames(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, u32 *pNumBuf);

/** @brief Receive ready frame from VIF quque
@param[in] u32VifChn VIF channel
@param[out] ptFbInfo output frame buffer
@retval 0 if successed or E_HAL_VIF_ERROR if error occurs.
*/
s32 DrvVif_DequeueFrameBuffer(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, MHal_VIF_RingBufElm_t *ptFbInfo);

/** @brief Set framerate
@param[in] u32VifChn VIF channel
@param[in] port number of u32VifChn
@param[in] frame rate value
@param[in] manual fps mask
@retval 0 if successed or E_HAL_VIF_ERROR if error occurs.
*/
s32 DrvVif_setChnFPSBitMask(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, MHal_VIF_FrameRate_e u32Chnfps, u32 *manualMask);

u8 DrvVif_FakeIrq(irqreturn_t func);

//#define MCU_WINDOWS0_OFFSET 0x4400UL
#define MCU_WINDOWS0_OFFSET 0x6000UL
//8051 state machine
#define MCU_STATE_STOP    0
#define MCU_STATE_READY   1
#define MCU_STATE_POLLING 2
#define MCU_STATE_NUM MCU_STATE_POLLING+1

#define MCU_REG_TIMEOUT 10


int initialSHMRing(void);
int unInitialSHMRing(void);

/* MCU control function */
int DrvVif_setARMControlStatus(u16 status);
int DrvVif_setMCUStoreMode(u8 ch, u8 mode);
int DrvVif_getMCUStatus(u16 *status);
u32 DrvVif_changeMCUStatus(u16 status);
u32 convertMhalBuffElmtoMCU(volatile VifRingBufElm_mcu *mcu, const MHal_VIF_RingBufElm_t *ptFbInfo);
u32 convertMCUBuffElmtoMhal(volatile VifRingBufElm_mcu *mcu, MHal_VIF_RingBufElm_t *element, u8 portidx);
u32 convertFPSMaskToMCU(u32 mask);

u32 DrvVif_stopMCU(void);
u32 DrvVif_startMCU(void);
u32 DrvVif_pollingMCU(void);
u32 DrvVif_pauseMCU(void);
void DrvVif_dumpInfo(void);
int DrvVif_MCULoadBin(void);
int DrvVif_SetVifChanelBaseAddr(void);
int DrvVif_CLK(void);

int DrvVif_DmaEnable(VIF_CHANNEL_e ch,unsigned char en);
int DrvVif_RawStore(VIF_CHANNEL_e ch,u32 uMiuBase,u32 ImgW,u32 ImgH);

int DrvVif_EnableInterrupt(VIF_CHANNEL_e ch, u8 en);

void DrvVif_SetDmaLineCntInt(VIF_CHANNEL_e ch, u8 en);
void DrvVif_SetDmaLineCnt(VIF_CHANNEL_e ch, u16 cnt);
void DrvVif_DoDmaLineCntCb(VIF_CHANNEL_e ch);
void DrvVif_DoVifFrameStartCb(VIF_CHANNEL_e ch);

void DrvVif_RegisterFrameStartCb(VIF_CHANNEL_e ch, void *CallBack, u32 param, void *pPrivateData);
void DrvVif_RegisterLineCntCb(VIF_CHANNEL_e ch, void *CallBack, u32 param, void *pPrivateData);
void DrvVif_RegisterLineCntCbTest(VIF_CHANNEL_e ch, void *CallBack);
s32 DrvVif_RegisterFrameEndCb(VIF_CHANNEL_e ch, void *CallBack, u32 param, void *pPrivateData);
void DrvVif_ShowRegistedCb(void);

void DrvVif_SetVifFrameStartInt(u16 ch_bitmask, u8 en);

void DrvVif_CtxInit(void);
#endif
