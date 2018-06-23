#include <vif_common.h>
#include "drv_vif.h"
#include "hal_vif.h"
#include "mdrv_vif_io.h"
#include "vif_datatype.h"
#include <linux/delay.h>
#include "hal_dma.h"
#include "hal_rawdma.h"
#include <mhal_common.h>
#include "drv_csi.h"
#include "arch/infinity2_reg_padtop1.h"
#include "arch/infinity2_reg_block_ispsc.h"
#include <drv_isrcb.h>
#include "bufptr.h"
//#include "drv_vif_export.h"

#define REG_R(base,offset) (*(unsigned short*)(base+(offset*4)))
#define REG_W(base,offset,val) ((*(unsigned short*)((base)+ (offset*4)))=(val))
#define MAILBOX_HEART_REG (0x6D)
#define MAILBOX_CONCTRL_REG (0x6E)
#define MAILBOX_STATE_REG (0x6F)

#define VIF_SHIFTBITS(a)      (0x01<<(a))
#define VIF_MASK(a)      	  (~(0x01<<(a)))
#define VIF_CHECKBITS(a, b)   ((a) & ((u32)0x01 << (b)))
#define VIF_SETBIT(a, b)      (a) |= (((u32)0x01 << (b)))
#define VIF_CLEARBIT(a, b)    (a) &= (~((u32)0x01 << (b)))
//#define VIF_TASK_STACK_SIZE 	  (8192)

extern unsigned int *g_VIFReg[VIF_CHANNEL_NUM];
#if 0
static MsTaskId_e      u8VifFSTaskId = 0xFF;
static MsTaskId_e      u8VifFETaskId = 0xFF;
static u32      *pVifFSStackTop = NULL;
static u32      *pVifFEStackTop = NULL;
Ms_Flag_t       _ints_event_flag;
#endif
#define ISP_MAX_BANK (13)

#define IPC_RAM_SIZE (64*1024)
#define TIMER_RATIO 12


#if IPC_DMA_ALLOC_COHERENT
extern u32 IPCRamPhys;
extern char *IPCRamVirtAddr;
#else
extern unsigned long IPCRamPhys;
extern void *IPCRamVirtAddr;
#endif


/*ToDo: Remove these extern functions */
extern s32 DrvSensorGetSifBus(u32 uCamID, u32 *sif_bus);
extern s32 DrvSensorInit(u32 uCamID);
extern s32 DrvSensorInit1(u32 uCamID, u32 hdr, void *reset_cb, void *set_mclk_cb);
extern s32 DrvSensorGetDataPrec(u32 uCamID, VIF_SENSOR_FORMAT_e *data_prec);
extern s32 DrvSensorGetPclkPol(u32 uCamID, VIF_CLK_POL *pclk_pol);
extern s32 DrvSensorGetVsyncPol(u32 uCamID, VIF_CLK_POL *vsync_pol);
extern s32 DrvSensorGetHsyncPol(u32 uCamID, VIF_CLK_POL *hsync_pol);
extern s32 DrvSensorGetResetPol(u32 uCamID, VIF_PIN_POL *reset_pol);
extern s32 DrvSensorGetCropInfo(u32 uCamID, u32 *x, u32 *y, u32 *width, u32 *height);
extern s32 DrvSensorGetMclk(u32 uCamID, VifMclk_e *mclk);
extern s32 DrvSensorGetBayerId(u32 uCamID, u32 *bayer_id);

/*! @brief Sensor bayer raw pixel order, copy from drv_ms_cus_sensor.h */
typedef enum {
    _CUS_BAYER_RG = 0,       /**< bayer data start with R channel */
    _CUS_BAYER_GR,             /**<  bayer data start with Gr channel */
    _CUS_BAYER_BG,             /**<  bayer data start with B channel */
    _CUS_BAYER_GB              /**<  bayer data start with Gb channel */
}_CUS_SEN_BAYER;


static volatile u32 ulgvif_def_mask[VIF_CHANNEL_NUM] =
{
    //VIF_CHANNEL_0
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_1,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),


    //VIF_CHANNEL_2,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),


    //VIF_CHANNEL_3,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),


    //VIF_CHANNEL_4,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_5,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_6,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_7,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_8,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_9,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_10,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_11,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_12,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_13,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_14,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),

    //VIF_CHANNEL_15,
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE)|
    //VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0),
    VIF_SHIFTBITS(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1)|
    VIF_SHIFTBITS(VIF_INTERRUPT_BT656_CHANNEL_DETECT_DONE),
};


/***********************************8051 IPC ring buffer***********************************/
volatile VifRingBufShm_t *SHM_Ring = NULL;
const char * MCU_state_tlb[] = {"STOP","START","POLLING"};
VifnPTs vif_npts[VIF_PORT_NUM];
const u32 FpsBitMaskArray[E_MHAL_VIF_FRAMERATE_MAX] = {
0xFFFFFFFF,  //FULL:          [11111111 11111111 11111111 11111111]
0x55555555,  //HALF:          [01010101 01010101 01010101 01010101]
0x11111111,  //QUARTER:       [00010001 00010001 00010001 00010001]
0x01010101,  //OCTANT:        [00000001 00000001 00000001 00000001]
0x77777777,  //THREE_QUARTERS:[01110111 01110111 01110111 01110111]
};

struct vif_drv_data{
    void *vif_callback[VIF_CHANNEL_NUM][VIF_FRAME_INT_CB_NUM];
    void *private_data[VIF_CHANNEL_NUM][VIF_FRAME_INT_CB_NUM];

    /* ToDo: remove test code */
    void *vif_callback_test[VIF_CHANNEL_NUM][VIF_FRAME_INT_CB_NUM];

    /* Save interface mode for groups */
    MHal_VIF_IntfMode_e intf_mode[SENSOR_PAD_GROUP_NUM];

    /* Save work mode for groups */
    MHal_VIF_WorkMode_e work_mode[SENSOR_PAD_GROUP_NUM];

    /* Save hdr mode for groups */
    MHalHDRType_e hdr_mode[SENSOR_PAD_GROUP_NUM];

    /* Save line count ring buf for debug */
    MHal_VIF_RingBufElm_t ring_buf[VIF_CHANNEL_NUM];

    /* Save memory allocatior for Realtime HDR mode*/
    MHalAllocPhyMem_t stMemAlloc;

    BufPtr_t* pRtHdrHandle;

    /* Keep metadata counter */
    union MetaData{
        u64 val;
        struct{
            u64 counter:48;
            u64 reserved:8;
            u64 channel:8;
        };
    }meta_data[VIF_CHANNEL_NUM];

    /* Frame end data */
    u8 frame_end_data[VIF_CHANNEL_NUM];
};

// driver context
struct vif_drv_data vif_drv_ctx;

//Mutex for MCU status change
CamOsMutex_t mculock;
//static struct mutex mculock;

void DrvVif_CtxInit(void)
{
    u32 i;
    for (i = 0; i < SENSOR_PAD_GROUP_NUM; i++) {
        vif_drv_ctx.intf_mode[i] = E_MHAL_VIF_MODE_MAX;
        vif_drv_ctx.work_mode[i] = E_MHAL_VIF_WORK_MODE_MAX;
    }
}

void DrvVif_RegisterFrameStartCb(VIF_CHANNEL_e ch, void *CallBack, u32 param, void *pPrivateData)
{
    vif_drv_ctx.vif_callback[ch][VIF_FRAME_START_INT_CB] = CallBack;
    vif_drv_ctx.private_data[ch][VIF_FRAME_START_INT_CB] = pPrivateData;
}

void DrvVif_RegisterLineCntCb(VIF_CHANNEL_e ch, void *CallBack, u32 param, void *pPrivateData)
{
    vif_drv_ctx.vif_callback[ch][VIF_FRAME_LINE_CNT_INT_CB] = CallBack;
    vif_drv_ctx.private_data[ch][VIF_FRAME_LINE_CNT_INT_CB] = pPrivateData;
}

void DrvVif_RegisterLineCntCbTest(VIF_CHANNEL_e ch, void *CallBack)
{
    vif_drv_ctx.vif_callback_test[ch][VIF_FRAME_LINE_CNT_INT_CB] = CallBack;
}
EXPORT_SYMBOL(DrvVif_RegisterLineCntCbTest);

s32 DrvVif_GetFillingRingBufElm(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, MHal_VIF_RingBufElm_t *ptFbInfo)
{
    u32 mcu_chn = u32VifChn + ((u32ChnPort>0)? VIF_PORT_NUM/2:0);
    u8 p_ridx = SHM_Ring[mcu_chn].nReadIdx;

    if (SHM_Ring[mcu_chn].data[p_ridx].nStatus == E_VIF_BUF_FILLING) {
        convertMCUBuffElmtoMhal(&(SHM_Ring[mcu_chn].data[p_ridx]), ptFbInfo, mcu_chn);
        rmb();
    } else {
        return E_HAL_VIF_ERROR;
    }
    return E_HAL_VIF_SUCCESS;
}


void DrvVif_DoVifFrameStartCb(VIF_CHANNEL_e ch)
{
    if (vif_drv_ctx.vif_callback[ch][VIF_FRAME_START_INT_CB]) {
        MHal_VIF_EventCallback_fp cb = (MHal_VIF_EventCallback_fp)vif_drv_ctx.vif_callback[ch][VIF_FRAME_START_INT_CB];
        void *private_data = vif_drv_ctx.private_data[ch][VIF_FRAME_START_INT_CB];
        MHal_VIF_RingBufElm_t *buf = NULL;
        cb(ch, E_VIF_EVENT_FRAME_START, buf, private_data);
    }
    return;
}

MHalPixelFormat_e _DrvVif_BppBayerIDtoFormat(VIF_SENSOR_FORMAT_e eFmt,_CUS_SEN_BAYER eBayerID)
{
    MHalPixelFormat_e eRet;
    switch(eFmt)
    {
    case VIF_SENSOR_FORMAT_8BIT:
        eRet = E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_RG + _CUS_BAYER_RG;
        break;
    case VIF_SENSOR_FORMAT_10BIT:
        eRet = E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_RG + _CUS_BAYER_GR;
        break;
    case VIF_SENSOR_FORMAT_16BIT:
        eRet = E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_RG + _CUS_BAYER_BG;
        break;
    case VIF_SENSOR_FORMAT_12BIT:
        eRet = E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_RG + _CUS_BAYER_GB;
        break;
    }
    return eRet;
}

void DrvVif_DoDmaLineCntCb(VIF_CHANNEL_e ch)
{
    if (vif_drv_ctx.vif_callback[ch][VIF_FRAME_LINE_CNT_INT_CB]) {
        MHal_VIF_EventCallback_fp cb = (MHal_VIF_EventCallback_fp)vif_drv_ctx.vif_callback[ch][VIF_FRAME_LINE_CNT_INT_CB];
        void *private_data = vif_drv_ctx.private_data[ch][VIF_FRAME_LINE_CNT_INT_CB];
        MHal_VIF_RingBufElm_t *buf = &vif_drv_ctx.ring_buf[ch];
        VIF_SENSOR_FORMAT_e data_prec;
        u32 nBayerID;

        if (DrvVif_GetFillingRingBufElm(ch, 0, buf)) {
            VIF_DEBUG("Get filling RingBufElm fail \n");
            buf = NULL;
        } else {

            if (DrvSensorGetBayerId(ch/4, &nBayerID)) {
                VIF_DEBUG("Get sensor bayer id fail \n");
            }

            if (DrvSensorGetDataPrec(ch/4, &data_prec)) {
                VIF_DEBUG("Get sensor pixel format fail \n");
            } else {
                switch (data_prec) {
                    case VIF_SENSOR_FORMAT_8BIT:
                            buf->ePixelFormat = _DrvVif_BppBayerIDtoFormat(VIF_SENSOR_FORMAT_8BIT,nBayerID);
                        break;
                    case VIF_SENSOR_FORMAT_10BIT:
                            buf->ePixelFormat = _DrvVif_BppBayerIDtoFormat(VIF_SENSOR_FORMAT_10BIT,nBayerID);
                        break;
                    case VIF_SENSOR_FORMAT_16BIT:
                            buf->ePixelFormat = _DrvVif_BppBayerIDtoFormat(VIF_SENSOR_FORMAT_16BIT,nBayerID);
                        break;
                    case VIF_SENSOR_FORMAT_12BIT:
                            buf->ePixelFormat = _DrvVif_BppBayerIDtoFormat(VIF_SENSOR_FORMAT_12BIT,nBayerID);
                        break;
                    default:
                        VIF_DEBUG("Sensor pixel format type error\n");
                        break;
                }
            }
            vif_drv_ctx.meta_data[ch].channel = ch;
            vif_drv_ctx.meta_data[ch].counter++;
            buf->u64MetaInfo = vif_drv_ctx.meta_data[ch].val;
        }

        cb(ch, E_VIF_EVENT_LINE_COUNT_HIT0, buf, private_data);
    }

    /* ToDo: Remove this to normal procedure */
    if (vif_drv_ctx.vif_callback_test[ch][VIF_FRAME_LINE_CNT_INT_CB]) {
        typedef void (*test_cb)(void);
        test_cb cb = (test_cb)vif_drv_ctx.vif_callback_test[ch][VIF_FRAME_LINE_CNT_INT_CB];
        cb();
    }

    return;
}

void DrvVif_DoVifFrameEndCb(void *data)
{
    u8 ch = *(u8 *)data;
    if (vif_drv_ctx.vif_callback[ch][VIF_FRAME_END_INT_CB]) {
        MHal_VIF_EventCallback_fp cb = (MHal_VIF_EventCallback_fp)vif_drv_ctx.vif_callback[ch][VIF_FRAME_END_INT_CB];
        void *private_data = vif_drv_ctx.private_data[ch][VIF_FRAME_END_INT_CB];
        MHal_VIF_RingBufElm_t *buf = &vif_drv_ctx.ring_buf[ch];
        cb(ch, E_VIF_EVENT_FRAME_END, buf, private_data);
    }
    return;
}

s32 DrvVif_RegisterFrameEndCb(VIF_CHANNEL_e ch, void *CallBack, u32 param, void *pPrivateData)
{
    s32 ret = E_HAL_VIF_SUCCESS;

    vif_drv_ctx.vif_callback[ch][VIF_FRAME_END_INT_CB] = CallBack;
    vif_drv_ctx.private_data[ch][VIF_FRAME_END_INT_CB] = pPrivateData;

    switch (ch) {
        case VIF_CHANNEL_0:
            vif_drv_ctx.frame_end_data[ch] = ch;
            ISRCB_RegisterCallback(eISRCB_ID_ISP_WDMA0_DONE, DrvVif_DoVifFrameEndCb, &vif_drv_ctx.frame_end_data[ch]);
            break;
        case VIF_CHANNEL_4:
            vif_drv_ctx.frame_end_data[ch] = ch;
            ISRCB_RegisterCallback(eISRCB_ID_ISP_WDMA1_DONE, DrvVif_DoVifFrameEndCb, &vif_drv_ctx.frame_end_data[ch]);
            break;
        case VIF_CHANNEL_8:
            vif_drv_ctx.frame_end_data[ch] = ch;
            ISRCB_RegisterCallback(eISRCB_ID_ISP_WDMA2_DONE, DrvVif_DoVifFrameEndCb, &vif_drv_ctx.frame_end_data[ch]);
            break;
        case VIF_CHANNEL_12:
            vif_drv_ctx.frame_end_data[ch] = ch;
            ISRCB_RegisterCallback(eISRCB_ID_ISP_WDMA3_DONE, DrvVif_DoVifFrameEndCb, &vif_drv_ctx.frame_end_data[ch]);
            break;
        default:
            VIF_DEBUG("Register frame end callback: channel not supported\n");
            ret  = E_HAL_VIF_ERROR;
            break;
    }
    return ret;
}

EXPORT_SYMBOL(DrvVif_DoVifFrameEndCb);

void DrvVif_ShowRegistedCb(void)
{
    u32 i;
    VIF_DEBUG("Show frame start registed callback:\n");
    for (i = 0; i < VIF_CHANNEL_NUM; i++) {
        if (vif_drv_ctx.vif_callback[i][VIF_FRAME_START_INT_CB]) {
            VIF_DEBUG("CH%d CB %p ", i, vif_drv_ctx.vif_callback[i][VIF_FRAME_START_INT_CB]);
        }
        if (vif_drv_ctx.private_data[i][VIF_FRAME_START_INT_CB]) {
            VIF_DEBUG("PD %p\n", vif_drv_ctx.private_data[i][VIF_FRAME_START_INT_CB]);
        }
    }
    VIF_DEBUG("Show line cnt registed callback:\n");
    for (i = 0; i < VIF_CHANNEL_NUM; i++) {
        if (vif_drv_ctx.vif_callback[i][VIF_FRAME_LINE_CNT_INT_CB]) {
            VIF_DEBUG("CH%d CB %p ", i, vif_drv_ctx.vif_callback[i][VIF_FRAME_LINE_CNT_INT_CB]);
        }
        if (vif_drv_ctx.private_data[i][VIF_FRAME_LINE_CNT_INT_CB]) {
            VIF_DEBUG("PD %p\n", vif_drv_ctx.private_data[i][VIF_FRAME_LINE_CNT_INT_CB]);
        }
    }
    VIF_DEBUG("Show frame end registed callback:\n");
    for (i = 0; i < VIF_CHANNEL_NUM; i++) {
        if (vif_drv_ctx.vif_callback[i][VIF_FRAME_END_INT_CB]) {
            VIF_DEBUG("CH%d CB %p ", i, vif_drv_ctx.vif_callback[i][VIF_FRAME_END_INT_CB]);
        }
        if (vif_drv_ctx.private_data[i][VIF_FRAME_END_INT_CB]) {
            VIF_DEBUG("PD %p\n", vif_drv_ctx.private_data[i][VIF_FRAME_END_INT_CB]);
        }
    }
    return;
}

void DrvVif_SetIntfMode(MHal_VIF_DEV u32VifDev, MHal_VIF_IntfMode_e mode)
{
    vif_drv_ctx.intf_mode[u32VifDev] = mode;
}

MHal_VIF_IntfMode_e DrvVif_GetIntfMode(MHal_VIF_DEV u32VifDev)
{
    return vif_drv_ctx.intf_mode[u32VifDev];
}

void DrvVif_SetWorkMode(MHal_VIF_DEV u32VifDev, MHal_VIF_WorkMode_e mode)
{
    vif_drv_ctx.work_mode[u32VifDev] = mode;
}

MHal_VIF_WorkMode_e DrvVif_GetWorkMode(MHal_VIF_DEV u32VifDev)
{
    return vif_drv_ctx.work_mode[u32VifDev];
}

void DrvVif_SetHdrMode(MHal_VIF_DEV u32VifDev, MHalHDRType_e mode)
{
    vif_drv_ctx.hdr_mode[u32VifDev] = mode;
}

MHalHDRType_e DrvVif_GetHdrMode(MHal_VIF_DEV u32VifDev)
{
    return vif_drv_ctx.hdr_mode[u32VifDev];
}

void DrvVif_SetMemAllocator(MHalAllocPhyMem_t *pstAlloc)
{
    memcpy(&vif_drv_ctx.stMemAlloc, pstAlloc, sizeof(MHalAllocPhyMem_t));
}

MHalAllocPhyMem_t *DrvVif_GetMemAllocator(void)
{
    return &vif_drv_ctx.stMemAlloc;
}

int initialSHMRing(void) {
    int idx=0, sub_idx=0;
    SHM_Ring = (VifRingBufShm_t *) (IPCRamVirtAddr + MCU_WINDOWS0_OFFSET);
    VIF_DEBUG("Ptr = %p, sizeof framebuf = %d  size of ring = %d\n",SHM_Ring, sizeof(VifRingBufElm_mcu), sizeof(VifRingBufShm_t));

    if( (sizeof(VifRingBufShm_t)*VIF_PORT_NUM + 0x6000)> 0xFFFF){ //shared memory + code size(24KB) need to less than 64KB
        VIF_DEBUG("mcu shared memory size over/n");
        return E_HAL_VIF_ERROR;
    }
    memset((VifRingBufShm_t *) SHM_Ring, 0x0, sizeof(VifRingBufShm_t)*VIF_PORT_NUM);
    memset(vif_npts, 0x0, sizeof(VifnPTs)*VIF_PORT_NUM);
    for (idx=0; idx<VIF_PORT_NUM; ++idx) {
        SHM_Ring[idx].nReadIdx  = 0/*idx*/;
        SHM_Ring[idx].nWriteIdx  = 0/*VIF_PORT_NUM -idx -1*/;
        SHM_Ring[idx].pre_nReadIdx  = 0;
        SHM_Ring[idx].nFrameStartCnt = 0;
        SHM_Ring[idx].nFrameDoneCnt = 0;
        SHM_Ring[idx].nFrameDoneSubCnt = 0;
        SHM_Ring[idx].nDropFrameCnt = 0;
        SHM_Ring[idx].eFieldIdErr = 0;
        SHM_Ring[idx].nFPS_bitMap = FpsBitMaskArray[E_MHAL_VIF_FRAMERATE_FULL];
        for (sub_idx=0; sub_idx<VIF_RING_QUEUE_SIZE; ++sub_idx) {
            SHM_Ring[idx].data[sub_idx].nStatus = E_VIF_BUF_INVALID;
        }
    }
    //Mutex initial
    CamOsMutexInit(&mculock);
    //mutex_init(&mculock);

    return E_HAL_VIF_SUCCESS;
}

int resetSHMRing(MHal_VIF_CHN u32VifChn){

    int sub_idx=0;

    SHM_Ring[u32VifChn].nReadIdx  = 0/*idx*/;
    SHM_Ring[u32VifChn].nWriteIdx  = 0/*VIF_PORT_NUM -idx -1*/;
    SHM_Ring[u32VifChn].pre_nReadIdx  = 0;
    SHM_Ring[u32VifChn].nFrameStartCnt = 0;
    SHM_Ring[u32VifChn].nFrameDoneCnt = 0;
    SHM_Ring[u32VifChn].nFrameDoneSubCnt = 0;
    SHM_Ring[u32VifChn].nDropFrameCnt = 0;
    SHM_Ring[u32VifChn].eFieldIdErr = 0;
    SHM_Ring[u32VifChn].nFPS_bitMap = FpsBitMaskArray[E_MHAL_VIF_FRAMERATE_FULL];
    for (sub_idx=0; sub_idx<VIF_RING_QUEUE_SIZE; ++sub_idx) {
        SHM_Ring[u32VifChn].data[sub_idx].nStatus = E_VIF_BUF_INVALID;
    }

    return E_HAL_VIF_SUCCESS;
}

s32 unInitialSHMRing(void) {
    memset((VifRingBufShm_t *) SHM_Ring, 0x0, sizeof(VifRingBufShm_t)*VIF_PORT_NUM);
    memset(vif_npts, 0x0, sizeof(VifnPTs)*VIF_PORT_NUM);
    SHM_Ring = NULL;
    return E_HAL_VIF_SUCCESS;
}

u64 GetTimerCnt64(u32 npts, u8 portidx)
{
    //static u64 count64 = 0;
    //static u32 prev_count = 0;
    u32 count = npts;
    if(vif_npts[portidx].prev_count>count) {
        vif_npts[portidx].count64 += (0xFFFFFFFF - vif_npts[portidx].prev_count + count);
    } else
        vif_npts[portidx].count64 += (count - vif_npts[portidx].prev_count);

    vif_npts[portidx].prev_count = count;

    return vif_npts[portidx].count64;
}

u32 GetTimerCnt32(u32 npts, u8 portidx)
{
    u64 val = GetTimerCnt64(npts, portidx);
    do_div(val,TIMER_RATIO);
    return val&0xFFFFFFFF;
}

u32 convertMhalBuffElmtoMCU(volatile VifRingBufElm_mcu *mcu, const MHal_VIF_RingBufElm_t *ptFbInfo) {
    mcu->nPhyBufAddrY_H = (ptFbInfo->u64PhyAddr[0] >> 32) & 0xFFFFFFFF;
    mcu->nPhyBufAddrY_L = (ptFbInfo->u64PhyAddr[0]& 0xFFFFFFFF)>>5;
    mcu->nPhyBufAddrC_H = (ptFbInfo->u64PhyAddr[1] >> 32) & 0xFFFFFFFF;
    mcu->nPhyBufAddrC_L = (ptFbInfo->u64PhyAddr[1]& 0xFFFFFFFF)>>5;

    mcu->PitchY = (ptFbInfo->u32Stride[0]+31)>>5;
    mcu->PitchC = (ptFbInfo->u32Stride[1]+31)>>5;

    mcu->nCropX = ptFbInfo->nCropX;
    mcu->nCropY = ptFbInfo->nCropY;
    mcu->nCropW = ptFbInfo->nCropW-1;
    mcu->nCropH = (ptFbInfo->nCropH-1) | 0x8000;

    mcu->nHeightY = mcu->nCropH;
    mcu->nHeightC = ((ptFbInfo->nCropH >> 1) -1) | 0x8000;

    mcu->nPTS = ptFbInfo->nPTS;
    mcu->nMiPriv = ptFbInfo->nMiPriv;
    mcu->nStatus = ptFbInfo->nStatus;
    mcu->tag = 0;
    return E_HAL_VIF_SUCCESS;
}

/* Convert RGB bayer buffer to WDMA setting*/
u32 convertMhalRgbBuffElmtoMCU(volatile VifRingBufElm_mcu *mcu, const MHal_VIF_RingBufElm_t *ptFbInfo) {

    //dma->reg_isp_wdma_pitch = (ptCrop->uW+15)>>4;
    //dma->reg_isp_wdma_width_m1 = ((ptCrop->uW+3)/4)-1;
    //dma->reg_isp_wdma_height_en = 1;
    //dma->reg_isp_wdma_height_m1 = ptCrop->uH - 1;

    mcu->nPhyBufAddrY_H = (ptFbInfo->u64PhyAddr[0] >> 32) & 0xFFFFFFFF;
    mcu->nPhyBufAddrY_L = (ptFbInfo->u64PhyAddr[0]& 0xFFFFFFFF)>>5;
    mcu->nPhyBufAddrC_H = 0;
    mcu->nPhyBufAddrC_L = 0;

    mcu->PitchY = (ptFbInfo->u32Stride[0]/2+15)>>4;
    mcu->PitchC = 0;

    mcu->nCropX = ptFbInfo->nCropX;
    mcu->nCropY = ptFbInfo->nCropY;
    mcu->nCropW = ((ptFbInfo->nCropW+3)/4)-1;
    mcu->nCropH = (ptFbInfo->nCropH-1) | 0x8000;

    mcu->nHeightY = mcu->nCropH;
    mcu->nHeightC = 0;

    mcu->nPTS = ptFbInfo->nPTS;
    mcu->nMiPriv = ptFbInfo->nMiPriv;
    mcu->nStatus = ptFbInfo->nStatus;
    mcu->tag = 0;
    return E_HAL_VIF_SUCCESS;
}

MHalFieldType_e pre[32]={0};

u32 convertMCUBuffElmtoMhal(volatile VifRingBufElm_mcu *mcu, MHal_VIF_RingBufElm_t *element, u8 portidx) {
    u64 tmp;

    tmp = mcu->nPhyBufAddrY_H;
    tmp = (tmp<<32) | (mcu->nPhyBufAddrY_L << 5);
    element->u64PhyAddr[0] = tmp;

    tmp = mcu->nPhyBufAddrC_H;
    tmp = (tmp<<32) | (mcu->nPhyBufAddrC_L << 5);
    element->u64PhyAddr[1] = tmp;

    element->u32Stride[0] = (mcu->PitchY << 5);
    element->u32Stride[1] = (mcu->PitchC << 5);

    element->nCropX = mcu->nCropX;
    element->nCropY = mcu->nCropY;
    element->nCropW = mcu->nCropW+1;
    element->nCropH = (mcu->nCropH+1)&0x7FFF ;

    element->nMiPriv = mcu->nMiPriv;
    element->nStatus = mcu->nStatus;

    element->nPTS = GetTimerCnt32(mcu->nPTS, portidx);

    if((mcu->eFieldType & 0x01)== 1)  //high byte bit 2 of bit 10
        element->eFieldType = E_VIF_FIELDTYPE_BOTTOM;
    else
        element->eFieldType = E_VIF_FIELDTYPE_TOP;

    //printk("Dequeue buffer info: AddrY=0x%x AddrC=0x%x width=%d height=%d pitch=%d nPTS=0x%x FieldID:0x%x\n",
    //        (u32)element->u64PhyAddr[0], (u32)element->u64PhyAddr[1], element->nCropW, element->nCropH, element->u32Stride[0], element->nPTS,element->eFieldType);

    return E_HAL_VIF_SUCCESS;
}

u32 convertMCURgbBuffElmtoMhal(volatile VifRingBufElm_mcu *mcu, MHal_VIF_RingBufElm_t *element, u8 portidx) {
    u64 tmp;

    tmp = mcu->nPhyBufAddrY_H;
    tmp = (tmp<<32) | (mcu->nPhyBufAddrY_L << 5);
    element->u64PhyAddr[0] = tmp;

    element->u64PhyAddr[1] = 0;

    element->u32Stride[0] = (mcu->PitchY << 4);
    element->u32Stride[1] = 0;

    element->nCropX = mcu->nCropX;
    element->nCropY = mcu->nCropY;
    element->nCropW = (mcu->nCropW+1)*4;
    element->nCropH = (mcu->nCropH+1)&0x7FFF ;

    element->nMiPriv = mcu->nMiPriv;
    element->nStatus = mcu->nStatus;

    element->nPTS = GetTimerCnt32(mcu->nPTS, portidx);

    element->eFieldType = 0;

    //printk("Dequeue buffer info: AddrY=0x%x AddrC=0x%x width=%d height=%d pitch=%d nPTS=0x%x FieldID:0x%x\n",
    //        (u32)element->u64PhyAddr[0], (u32)element->u64PhyAddr[1], element->nCropW, element->nCropH, element->u32Stride[0], element->nPTS,element->eFieldType);
    return E_HAL_VIF_SUCCESS;
}

u32 convertFPSMaskToMCU(u32 mask) {
    u32 res = 0;
    u8 *off = (u8 *)&mask;
    u8 *r_off = (u8 *)&res;
    *r_off = *(off+3);
    *(r_off+1) = *(off+2);
    *(r_off+2) = *(off+1);
    *(r_off+3) = *off;
    return res;
}

void DrvVifFreeRtHdrBuf(struct BufPtr_t* pBuf) {
    MHalAllocPhyMem_t *pMemAlloc = DrvVif_GetMemAllocator();
    pMemAlloc->free(pBuf->pBuf);
}

BufPtr_t *DrvVifGetRtHdrBufHandle(void) {
    return vif_drv_ctx.pRtHdrHandle;
}
EXPORT_SYMBOL(DrvVifGetRtHdrBufHandle);

s32 DrvVif_DevSetConfig_BT1120(MHal_VIF_DEV u32VifDev, MHal_VIF_DevCfg_t *pstDevAttr)
{
    volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*) g_TOPPAD1;
    volatile infinity2_reg_block_ispsc* vifclk = (infinity2_reg_block_ispsc*) g_ISP_ClkGen;
    printk("~~~~~~~~~~~~~~~~~~~BT1120 %s , dev=%d  chnum:%d   edge:%d***\n",__FUNCTION__,u32VifDev,pstDevAttr->eWorkMode,pstDevAttr->eClkEdge);
    // TOPPAD1 setting
    //REG_W(g_TOPPAD1, 0x00, 0x0000);
    padtop->reg_all_pad_in = 0;

    switch(u32VifDev)
    {
    case 0:
    case 1:
        //REG_W(g_TOPPAD1, 0x02, 0xB550);
        //REG_W(g_TOPPAD1, 0x03, 0x0169);
        //sr0
        padtop->reg_ccir0_16b_mode = 1;
        padtop->reg_ccir0_8b_mode  = 0;
        padtop->reg_ccir1_8b_mode  = 1;

        padtop->reg_ccir0_clk_mode = 1;
        padtop->reg_ccir1_clk_mode = 1;

        padtop->reg_ccir0_ctrl_mode =1;

        //REG_W(g_TOPPAD1, 0x28, 0x03FF);
        padtop->reg_snr0_d_ie = 0x3FF;
        padtop->reg_snr1_d_ie = 0x3FF;

        vifclk->reg_ckg_snr0 = 0x18;
        vifclk->reg_ckg_snr1 = 0x18;
        vifclk->reg_ckg_snr2 = 0x18;
        vifclk->reg_ckg_snr3 = 0x18;
        vifclk->reg_ckg_snr4 = 0x18;
        vifclk->reg_ckg_snr5 = 0x18;
        vifclk->reg_ckg_snr6 = 0x18;
        vifclk->reg_ckg_snr7 = 0x18;

    break;

    }

	return 0;
}

s32 DrvVif_DevSetConfig_BT656(MHal_VIF_DEV u32VifDev, MHal_VIF_DevCfg_t *pstDevAttr)
{
	volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*) g_TOPPAD1;
	volatile infinity2_reg_block_ispsc* vifclk = (infinity2_reg_block_ispsc*) g_ISP_ClkGen;
	VIF_DEBUG("%s , dev=%d  chnum:%d   edge:%d***\n",__FUNCTION__,u32VifDev,pstDevAttr->eWorkMode,pstDevAttr->eClkEdge);
	// TOPPAD1 setting
	//REG_W(g_TOPPAD1, 0x00, 0x0000);
	padtop->reg_all_pad_in = 0;
	switch(u32VifDev)
	{
	case 0:
		//REG_W(g_TOPPAD1, 0x02, 0xB550);
		//REG_W(g_TOPPAD1, 0x03, 0x0169);
		//sr0
		padtop->reg_ccir0_16b_mode = 0;
		padtop->reg_ccir0_8b_mode  = 1;
		padtop->reg_ccir0_clk_mode = 1;
		padtop->reg_ccir0_ctrl_mode =1;
		//REG_W(g_TOPPAD1, 0x28, 0x03FF);
		padtop->reg_snr0_d_ie = 0x3FF;

        vifclk->reg_ckg_snr0 = 0x18;
        vifclk->reg_ckg_snr1 = 0x18;
        vifclk->reg_ckg_snr2 = 0x1A;
        vifclk->reg_ckg_snr3 = 0x1A;

        switch(pstDevAttr->eWorkMode){

        case E_MHAL_VIF_WORK_MODE_1MULTIPLEX :

            vifclk->reg_ckg_snr0 = 0x18;
            vifclk->reg_ckg_snr1 = 0x18;
            vifclk->reg_ckg_snr2 = 0x18;
            vifclk->reg_ckg_snr3 = 0x18;

        break;
        case E_MHAL_VIF_WORK_MODE_2MULTIPLEX :

            if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_SINGLE_UP){

                vifclk->reg_ckg_snr0 = 0x10;
                vifclk->reg_ckg_snr1 = 0x10;
                vifclk->reg_ckg_snr2 = 0x12;
                vifclk->reg_ckg_snr3 = 0x12;

            }else if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_DOUBLE){

                vifclk->reg_ckg_snr0 = 0x18;
                vifclk->reg_ckg_snr1 = 0x18;
                vifclk->reg_ckg_snr2 = 0x1A;
                vifclk->reg_ckg_snr3 = 0x1A;

            }else{

                VIF_DEBUG("Not support\n");
            }

        break;
        case E_MHAL_VIF_WORK_MODE_4MULTIPLEX :

            if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_SINGLE_UP){

                vifclk->reg_ckg_snr0 = 0x10;
                vifclk->reg_ckg_snr1 = 0x14;
                vifclk->reg_ckg_snr2 = 0x12;
                vifclk->reg_ckg_snr3 = 0x16;
                vifclk->reg_sc_spare_lo |= 0x01<<4;

            }else if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_DOUBLE){
                VIF_DEBUG("Not support\n");
            }else{
                VIF_DEBUG("Not support\n");
            }

        break;
        default :
        break;
        }


	break;
	case 1:
		//sr1
		padtop->reg_ccir1_8b_mode  = 1;
		padtop->reg_ccir1_clk_mode = 1;
		padtop->reg_ccir1_ctrl_mode =0;
		//REG_W(g_TOPPAD1, 0x2E, 0x03FF);
		padtop->reg_snr1_d_ie = 0x3FF;

		/*    VIF CLK    */
		//REG_W(g_ISP_ClkGen, 0x44, 0x1818);
		vifclk->reg_ckg_snr4 = 0x18;
		vifclk->reg_ckg_snr5 = 0x18;
		//REG_W(g_ISP_ClkGen, 0x45, 0x1A1A);
		vifclk->reg_ckg_snr6 = 0x1A;
		vifclk->reg_ckg_snr7 = 0x1A;


        switch(pstDevAttr->eWorkMode){

        case E_MHAL_VIF_WORK_MODE_1MULTIPLEX :

            vifclk->reg_ckg_snr4 = 0x18;
            vifclk->reg_ckg_snr5 = 0x18;
            vifclk->reg_ckg_snr6 = 0x18;
            vifclk->reg_ckg_snr7 = 0x18;

        break;
        case E_MHAL_VIF_WORK_MODE_2MULTIPLEX :

            if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_SINGLE_UP){

                vifclk->reg_ckg_snr4 = 0x10;
                vifclk->reg_ckg_snr5 = 0x10;
                vifclk->reg_ckg_snr6 = 0x12;
                vifclk->reg_ckg_snr7 = 0x12;

            }else if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_DOUBLE){

                vifclk->reg_ckg_snr4 = 0x18;
                vifclk->reg_ckg_snr5 = 0x18;
                vifclk->reg_ckg_snr6 = 0x1A;
                vifclk->reg_ckg_snr7 = 0x1A;

            }else{

                VIF_DEBUG("Not support\n");
            }

        break;
        case E_MHAL_VIF_WORK_MODE_4MULTIPLEX :

            if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_SINGLE_UP){

                vifclk->reg_ckg_snr4 = 0x10;
                vifclk->reg_ckg_snr5 = 0x14;
                vifclk->reg_ckg_snr6 = 0x12;
                vifclk->reg_ckg_snr7 = 0x16;
                vifclk->reg_sc_spare_lo |= 1<<5;

            }else if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_DOUBLE){
                VIF_DEBUG("Not support\n");
            }else{
                VIF_DEBUG("Not support\n");
            }

        break;
        default :
        break;
        }


	break;
	case 2:
		//sr2
		padtop->reg_ccir2_16b_mode = 0;
		padtop->reg_ccir2_8b_mode  = 1;
		padtop->reg_ccir2_clk_mode = 1;
		padtop->reg_ccir2_ctrl_mode =0;
		//REG_W(g_TOPPAD1, 0x34, 0x03FF);
		padtop->reg_snr2_d_ie = 0x3FF;

		/*    VIF CLK    */
		//REG_W(g_ISP_ClkGen, 0x46, 0x1818);
		vifclk->reg_ckg_snr8 = 0x18;
		vifclk->reg_ckg_snr9 = 0x18;
		//REG_W(g_ISP_ClkGen, 0x47, 0x1A1A);
		vifclk->reg_ckg_snr10 = 0x1A;
		vifclk->reg_ckg_snr11 = 0x1A;


        switch(pstDevAttr->eWorkMode){

        case E_MHAL_VIF_WORK_MODE_1MULTIPLEX :

            vifclk->reg_ckg_snr8 = 0x18;
            vifclk->reg_ckg_snr9 = 0x18;
            vifclk->reg_ckg_snr10 = 0x18;
            vifclk->reg_ckg_snr11 = 0x18;

        break;
        case E_MHAL_VIF_WORK_MODE_2MULTIPLEX :

            if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_SINGLE_UP){

                vifclk->reg_ckg_snr8 = 0x10;
                vifclk->reg_ckg_snr9 = 0x10;
                vifclk->reg_ckg_snr10 = 0x12;
                vifclk->reg_ckg_snr11 = 0x12;

            }else if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_DOUBLE){

                vifclk->reg_ckg_snr8 = 0x18;
                vifclk->reg_ckg_snr9 = 0x18;
                vifclk->reg_ckg_snr10 = 0x1A;
                vifclk->reg_ckg_snr11 = 0x1A;

            }else{

                VIF_DEBUG("Not support\n");
            }

        break;
        case E_MHAL_VIF_WORK_MODE_4MULTIPLEX :

            if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_SINGLE_UP){

                vifclk->reg_ckg_snr8 = 0x10;
                vifclk->reg_ckg_snr9 = 0x14;
                vifclk->reg_ckg_snr10 = 0x12;
                vifclk->reg_ckg_snr11 = 0x16;
                vifclk->reg_sc_spare_lo |= 1<<6;

            }else if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_DOUBLE){
                VIF_DEBUG("Not support\n");
            }else{
                VIF_DEBUG("Not support\n");
            }

        break;
        default :
        break;
        }


	break;
	case 3:
		//sr3
		padtop->reg_ccir3_8b_mode  = 1;
		padtop->reg_ccir3_clk_mode = 1;
		padtop->reg_ccir3_ctrl_mode =0;
		//REG_W(g_TOPPAD1, 0x3A, 0x03FF);
		padtop->reg_snr3_d_ie = 0x3FF;

		/*    VIF CLK    */
		//REG_W(g_ISP_ClkGen, 0x48, 0x1818);
		vifclk->reg_ckg_snr12 = 0x18;
		vifclk->reg_ckg_snr13 = 0x18;
		//REG_W(g_ISP_ClkGen, 0x49, 0x1A1A);
		vifclk->reg_ckg_snr14 = 0x1A;
		vifclk->reg_ckg_snr15 = 0x1A;


        switch(pstDevAttr->eWorkMode){

        case E_MHAL_VIF_WORK_MODE_1MULTIPLEX :

            vifclk->reg_ckg_snr12 = 0x18;
            vifclk->reg_ckg_snr13 = 0x18;
            vifclk->reg_ckg_snr14 = 0x18;
            vifclk->reg_ckg_snr15 = 0x18;
        break;
        case E_MHAL_VIF_WORK_MODE_2MULTIPLEX :

            if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_SINGLE_UP){

                vifclk->reg_ckg_snr12 = 0x10;
                vifclk->reg_ckg_snr13 = 0x10;
                vifclk->reg_ckg_snr14 = 0x12;
                vifclk->reg_ckg_snr15 = 0x12;

            }else if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_DOUBLE){

                vifclk->reg_ckg_snr12 = 0x18;
                vifclk->reg_ckg_snr13 = 0x18;
                vifclk->reg_ckg_snr14 = 0x1A;
                vifclk->reg_ckg_snr15 = 0x1A;

            }else{

                VIF_DEBUG("Not support\n");
            }
        break;
        case E_MHAL_VIF_WORK_MODE_4MULTIPLEX :

            if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_SINGLE_UP){

                vifclk->reg_ckg_snr12 = 0x10;
                vifclk->reg_ckg_snr13 = 0x14;
                vifclk->reg_ckg_snr14 = 0x12;
                vifclk->reg_ckg_snr15 = 0x16;
                vifclk->reg_sc_spare_lo |= 1<<7;

            }else if(pstDevAttr->eClkEdge == E_MHAL_VIF_CLK_EDGE_DOUBLE){
                VIF_DEBUG("Not support\n");
            }else{
                VIF_DEBUG("Not support\n");
            }
        break;
        default :
        break;
        }
	break;
	}

	switch(pstDevAttr->eWorkMode)
	{
    case E_MHAL_VIF_WORK_MODE_1MULTIPLEX :
        HalDma_ConfigGroup(u32VifDev,1);
    break;
    case E_MHAL_VIF_WORK_MODE_2MULTIPLEX :
        HalDma_ConfigGroup(u32VifDev,2);
    break;
    case E_MHAL_VIF_WORK_MODE_4MULTIPLEX :
        HalDma_ConfigGroup(u32VifDev,4);
	break;
	default:
        HalDma_ConfigGroup(u32VifDev,4);
	break;
	}


    return E_HAL_VIF_SUCCESS;
}

s32 DrvVif_DevSetConfig_Parallel(MHal_VIF_DEV u32VifDev, MHal_VIF_DevCfg_t *pstDevAttr)
{
    return E_HAL_VIF_SUCCESS;
}

void DrvVif_SensorReset1(u32 ch, u32 RstPol, u32 ms_delay)
{
    HalVif_SensorReset(ch, RstPol);
    VIF_MS_SLEEP(ms_delay);
    HalVif_SensorReset(ch,~RstPol);
}

void DrvVif_SensorSetMclk1(u32 ch, u32 mclk)
{
    HalVif_SetMCLK(ch,mclk);
}
s32 DrvVif_DevSetConfig_MIPI(MHal_VIF_DEV u32VifDev, MHal_VIF_DevCfg_t *pstDevAttr, MHalAllocPhyMem_t *pstAlloc)
{
    /* Config CSI */
    Drv_CSISetClk(u32VifDev, 0);
    Drv_CSISetLane(u32VifDev, 4, 1);
    Drv_CSISetLongPacket(u32VifDev, 0, 0x1C00, 0);
    Drv_CSISetClkDataSkip(u32VifDev, 0x13,0x13);

    /* Config VIF IE Pad */
    HalVifDataIeConfig(u32VifDev, 0);

    /* Clear HDR setting */
    Drv_CSISetHDRMode(u32VifDev, CSI_HDR_MODE_SONY_290_326, 0);
    Drv_CSISetHDRMode(u32VifDev, CSI_HDR_MODE_EMBEDDED_RAW8, 0);
    Drv_CSISetHDRMode(u32VifDev, CSI_HDR_MODE_EMBEDDED_RAW10, 0);
    Drv_CSISetHDRMode(u32VifDev, CSI_HDR_MODE_EMBEDDED_RAW12, 0);

    /* Config HDR setting */
    switch(pstDevAttr->eHDRMode)
    {
        case E_MHAL_HDR_TYPE_OFF:
            /* Init Sensor */
            DrvSensorInit1(u32VifDev, 0, DrvVif_SensorReset1, DrvVif_SensorSetMclk1);
            break;
        case E_MHAL_HDR_TYPE_DOL:
            Drv_CSISetHDRMode(u32VifDev, CSI_HDR_MODE_SONY_290_326, 1);
            /* Init Sensor */
            DrvSensorInit1(u32VifDev, 1, DrvVif_SensorReset1, DrvVif_SensorSetMclk1);
            if (u32VifDev == 0) {
                DrvVif_SetMemAllocator(pstAlloc);
            }
            break;
        case E_MHAL_HDR_TYPE_EMBEDDED:
        case E_MHAL_HDR_TYPE_VC:
        case E_MHAL_HDR_TYPE_LI:
            VIF_DEBUG("[VIF] Not Implemented interface mode.");
            return E_HAL_VIF_ERROR;
        default:
            VIF_DEBUG("[VIF] Invalid interface mode for mipi sensor.");
            return E_HAL_VIF_ERROR;
    }

        return E_HAL_VIF_SUCCESS;
}

s32 DrvVif_DevSetConfig(MHal_VIF_DEV u32VifDev, MHal_VIF_DevCfg_t *pstDevAttr, MHalAllocPhyMem_t *pstAlloc)
{
    s32 ret = E_HAL_VIF_ERROR;

    //VIF_DEBUG("[DrvVif_DevSetConfig] intf %d, mode %d\n", pstDevAttr->eIntfMode, pstDevAttr->eIntfMode);

    switch (pstDevAttr->eIntfMode) {
        case E_MHAL_VIF_MODE_BT656:
            ret = DrvVif_DevSetConfig_BT656(u32VifDev, pstDevAttr);
            DrvVif_SetIntfMode(u32VifDev, pstDevAttr->eIntfMode);
            DrvVif_SetWorkMode(u32VifDev, pstDevAttr->eWorkMode);
            break;
        case E_MHAL_VIF_MODE_BT1120_STANDARD:
        case E_MHAL_VIF_MODE_BT1120_INTERLEAVED:
            ret = DrvVif_DevSetConfig_BT1120(u32VifDev, pstDevAttr);
            DrvVif_SetIntfMode(u32VifDev, pstDevAttr->eIntfMode);
            DrvVif_SetWorkMode(u32VifDev, pstDevAttr->eWorkMode);
            break;
        case E_MHAL_VIF_MODE_DIGITAL_CAMERA:
            ret = DrvVif_DevSetConfig_Parallel(u32VifDev, pstDevAttr);
            DrvVif_SetIntfMode(u32VifDev, pstDevAttr->eIntfMode);
            DrvVif_SetWorkMode(u32VifDev, pstDevAttr->eWorkMode);
            break;
        case E_MHAL_VIF_MODE_MIPI:
            ret = DrvVif_DevSetConfig_MIPI(u32VifDev, pstDevAttr, pstAlloc);
            DrvVif_SetIntfMode(u32VifDev, pstDevAttr->eIntfMode);
            DrvVif_SetWorkMode(u32VifDev, pstDevAttr->eWorkMode);
            DrvVif_SetHdrMode(u32VifDev, pstDevAttr->eHDRMode);
            break;
        default:
            VIF_DEBUG("Invaild device interface mode\n");
            break;
    }

    return ret;
}
EXPORT_SYMBOL(DrvVif_DevSetConfig);

s32 DrvVif_DevEnable(MHal_VIF_DEV u32VifDev)
{
    //VIF_DEBUG("===============   %s , dev=%d  ==============",__FUNCTION__,u32VifDev);
    s32 ret = E_HAL_VIF_SUCCESS;
    MHal_VIF_WorkMode_e eWorkMode = DrvVif_GetWorkMode(u32VifDev);
    MHalHDRType_e hdr_mode = DrvVif_GetHdrMode(u32VifDev);

    switch (DrvVif_GetIntfMode(u32VifDev)) {
        case E_MHAL_VIF_MODE_BT656:
        case E_MHAL_VIF_MODE_BT1120_STANDARD:
        case E_MHAL_VIF_MODE_BT1120_INTERLEAVED:
            HalDma_GlobalEnable();
            HalDma_EnableGroup(u32VifDev);
            break;

        case E_MHAL_VIF_MODE_DIGITAL_CAMERA:
        case E_MHAL_VIF_MODE_MIPI:
            if (E_MHAL_VIF_WORK_MODE_RAW_FRAMEMODE == eWorkMode) {
                HalRawDma_GlobalEnable();
                HalRawDma_GroupReset(u32VifDev);
                HalRawDma_GroupEnable(u32VifDev);
            }

            if ((E_MHAL_HDR_TYPE_DOL == hdr_mode) && (u32VifDev == 0)) {
                u32 ch = HalVifGetChnByGroup(u32VifDev);
                u32 group = HalVifGetGroupByChn(HalVifGetHDRChnPair(ch));
                HalRawDma_GlobalEnable();
                HalRawDma_GroupReset(group);
                HalRawDma_GroupDisable(group);
            }
            break;
        default:
            VIF_DEBUG("[DrvVif_DevEnable]Invaild device interface mode\n");
            ret = E_HAL_VIF_ERROR;
            break;
    }

    return ret;
}

s32 DrvVif_DevDisable(MHal_VIF_DEV u32VifDev)
{
	volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*) g_TOPPAD1;
	volatile infinity2_reg_block_ispsc* vifclk = (infinity2_reg_block_ispsc*) g_ISP_ClkGen;
	//VIF_DEBUG("%s , dev=%d",__FUNCTION__,u32VifDev);
	// TOPPAD1 setting
	//REG_W(g_TOPPAD1, 0x00, 0x0000);
	padtop->reg_all_pad_in = 0;
	switch(u32VifDev)
	{
	case 0:
		//REG_W(g_TOPPAD1, 0x02, 0xB550);
		//REG_W(g_TOPPAD1, 0x03, 0x0169);
		//sr0
		padtop->reg_ccir0_16b_mode = 0;
		padtop->reg_ccir0_8b_mode  = 0;
		padtop->reg_ccir0_clk_mode = 0;
		padtop->reg_ccir0_ctrl_mode =0;
		//REG_W(g_TOPPAD1, 0x28, 0x03FF);
		padtop->reg_snr0_d_ie = 0x3FF;

		/*    VIF CLK    */
		//REG_W(g_ISP_ClkGen, 0x42, 0x1818);
		vifclk->reg_ckg_snr0 = 0x00;
		vifclk->reg_ckg_snr1 = 0x00;
		//REG_W(g_ISP_ClkGen, 0x43, 0x1A1A);
		vifclk->reg_ckg_snr2 = 0x00;
		vifclk->reg_ckg_snr3 = 0x00;
	break;
	case 1:
		//sr1
		padtop->reg_ccir1_8b_mode  = 0;
		padtop->reg_ccir1_clk_mode = 0;
		padtop->reg_ccir1_ctrl_mode =0;
		//REG_W(g_TOPPAD1, 0x2E, 0x03FF);
		padtop->reg_snr1_d_ie = 0x3FF;

		/*    VIF CLK    */
		//REG_W(g_ISP_ClkGen, 0x44, 0x1818);
		vifclk->reg_ckg_snr4 = 0x00;
		vifclk->reg_ckg_snr5 = 0x00;
		//REG_W(g_ISP_ClkGen, 0x45, 0x1A1A);
		vifclk->reg_ckg_snr6 = 0x00;
		vifclk->reg_ckg_snr7 = 0x00;
	break;
	case 2:
		//sr2
		padtop->reg_ccir2_16b_mode = 0;
		padtop->reg_ccir2_8b_mode  = 0;
		padtop->reg_ccir2_clk_mode = 0;
		padtop->reg_ccir2_ctrl_mode =0;
		//REG_W(g_TOPPAD1, 0x34, 0x03FF);
		padtop->reg_snr2_d_ie = 0x3FF;

		/*    VIF CLK    */
		//REG_W(g_ISP_ClkGen, 0x46, 0x1818);
		vifclk->reg_ckg_snr8 = 0x00;
		vifclk->reg_ckg_snr9 = 0x00;
		//REG_W(g_ISP_ClkGen, 0x47, 0x1A1A);
		vifclk->reg_ckg_snr10 = 0x00;
		vifclk->reg_ckg_snr11 = 0x00;
	break;
	case 3:
		//sr3
		padtop->reg_ccir3_8b_mode  = 0;
		padtop->reg_ccir3_clk_mode = 0;
		padtop->reg_ccir3_ctrl_mode =0;
		//REG_W(g_TOPPAD1, 0x3A, 0x03FF);
		padtop->reg_snr3_d_ie = 0x3FF;

		/*    VIF CLK    */
		//REG_W(g_ISP_ClkGen, 0x48, 0x1818);
		vifclk->reg_ckg_snr12 = 0x00;
		vifclk->reg_ckg_snr13 = 0x00;
		//REG_W(g_ISP_ClkGen, 0x49, 0x1A1A);
		vifclk->reg_ckg_snr14 = 0x00;
		vifclk->reg_ckg_snr15 = 0x00;
	break;
	}
    return E_HAL_VIF_SUCCESS;
}


s32 DrvVif_ChnSetConfig_BT656(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnCfg_t *pstAttr)
{
	WdmaCropParam_t stCrop;
	VIF_DEBUG("%s , ch=%d, W=%d, H=%d\n",__FUNCTION__,u32VifChn,pstAttr->stCapRect.u16Width,pstAttr->stCapRect.u16Height);
	// channel setting
	HalVif_SensorSWReset(u32VifChn, VIF_ENABLE);
	HalVif_IFStatusReset(u32VifChn, VIF_DISABLE);
	HalVif_SensorReset(u32VifChn, VIF_DISABLE);
	HalVif_SensorPowerDown(u32VifChn, VIF_DISABLE);
	HalVif_HDRen(u32VifChn, VIF_DISABLE);
	HalVif_HDRSelect(u32VifChn, VIF_HDR_VC0);
	HalVif_SelectSource(u32VifChn, VIF_CH_SRC_BT656);
	//HalVif_SensorChannelEnable(u32VifChn, VIF_ENABLE;
	HalVif_SensorFormatLeftSht(u32VifChn, VIF_DISABLE);
	HalVif_SensorBitSwap(u32VifChn, VIF_DISABLE);
	//HalVif_SensorHsyncPolarity(u32VifChn, VIF_SENSOR_POLARITY_HIGH_ACTIVE);
	HalVif_SensorVsyncPolarity(u32VifChn, VIF_SENSOR_POLARITY_HIGH_ACTIVE);
	HalVif_SensorFormat(u32VifChn, VIF_SENSOR_FORMAT_16BIT);
	HalVif_SensorRgbIn(u32VifChn, VIF_SENSOR_INPUT_FORMAT_RGB);
	HalVif_SensorFormatExtMode(u32VifChn, VIF_SENSOR_BIT_MODE_1);
	HalVif_BT656ChannelDetectEnable(u32VifChn, VIF_ENABLE);
	HalVif_BT656VSDelay(u32VifChn, VIF_BT656_VSYNC_DELAY_BT656);
	HalVif_BT656HorizontalCropSize(u32VifChn, 0xFFFF);
	//HalVif_PixCropEnd(u32VifChn,0xFFFF);

	stCrop.uW = pstAttr->stCapRect.u16Width;
	stCrop.uH = pstAttr->stCapRect.u16Height;
	stCrop.uX = pstAttr->stCapRect.u16X;
	stCrop.uY = pstAttr->stCapRect.u16Y;

    HalVif_Crop(u32VifChn,0, 0, 0x1FFF, 0x1FFF );
    HalVif_CropEnable(u32VifChn,VIF_DISABLE);
#if 1  //work around for shadow not active
    HalDma_Config(u32VifChn,&stCrop,0x08000000,0x08100000);
#else
    HalDma_Config(u32VifChn,&stCrop,0x2266f000,0x2266f000);
#endif
    //HalDma_MaskOutput(u32VifChn,0x1);
	// ISP CLKGen setting

	stCrop.uW /= 2;
	stCrop.uH /= 2;
	HalDma_ConfigSub(u32VifChn,&stCrop,0x08200000,0x08300000);
	//HalDma_MaskOutputSub(u32VifChn,0x1);

    //FPS mask setting
    DrvVif_setChnFPSBitMask(u32VifChn, VIF_CHN_MAIN, pstAttr->eFrameRate, NULL);

    return E_HAL_VIF_SUCCESS;
}
s32 DrvVif_ChnSetConfig_BT1120(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnCfg_t *pstAttr)
{
    WdmaCropParam_t stCrop;
    printk("~~~~~~~~a~~~~~1120~~~~~%s , ch=%d, W=%d, H=%d\n",__FUNCTION__,u32VifChn,pstAttr->stCapRect.u16Width,pstAttr->stCapRect.u16Height);
    // channel setting
    HalVif_SensorSWReset(u32VifChn, VIF_ENABLE);
    HalVif_IFStatusReset(u32VifChn, VIF_DISABLE);
    HalVif_SensorReset(u32VifChn, VIF_DISABLE);
    HalVif_SensorPowerDown(u32VifChn, VIF_DISABLE);
    HalVif_HDRen(u32VifChn, VIF_DISABLE);
    HalVif_HDRSelect(u32VifChn, VIF_HDR_VC0);

    if(u32VifChn == 0)
      HalVif_SelectSource(u32VifChn, VIF_CH_SRC_MAX);  //0x1502 0x00 = 0x8B01 temp setting
    else
      HalVif_SelectSource(u32VifChn, VIF_CH_SRC_BT1120);

    //HalVif_SensorChannelEnable(u32VifChn, VIF_ENABLE;
    HalVif_SensorFormatLeftSht(u32VifChn, VIF_DISABLE);
    HalVif_SensorBitSwap(u32VifChn, VIF_DISABLE);
    //HalVif_SensorHsyncPolarity(u32VifChn, VIF_SENSOR_POLARITY_HIGH_ACTIVE);
    HalVif_SensorVsyncPolarity(u32VifChn, VIF_SENSOR_POLARITY_HIGH_ACTIVE);
    HalVif_SensorFormat(u32VifChn, VIF_SENSOR_FORMAT_16BIT);
    HalVif_SensorRgbIn(u32VifChn, VIF_SENSOR_INPUT_FORMAT_YUV422);
    HalVif_SensorFormatExtMode(u32VifChn, VIF_SENSOR_BIT_MODE_1);
    HalVif_SensorYc16Bit(u32VifChn,VIF_SENSOR_YC_SEPARATE);
    HalVif_SensorHsDly(u32VifChn,VIF_SENSOR_HS_FALLING_EDGE);
    HalVif_BT656InputSelect(u32VifChn, 1);
    HalVif_BT656ChannelDetectEnable(u32VifChn, VIF_ENABLE);
    HalVif_BT656ChannelDetectSelect(u32VifChn, VIF_BT656_EAV_DETECT);
    HalVif_BT656BitSwap(u32VifChn, 0);
    HalVif_Bt656Yc16bitMode(u32VifChn,VIF_SENSOR_BT656_YC);
    HalVif_Bt1120Yc16bitMode(u32VifChn,VIF_SENSOR_BT656_CY);
    HalVif_LiSt(u32VifChn,5);
    HalVif_LiEnd(u32VifChn,10);

    HalVif_BT656VSDelay(u32VifChn, VIF_BT656_VSYNC_DELAY_BT656);
    HalVif_BT656HorizontalCropSize(u32VifChn, 0xFFFF);
    //HalVif_PixCropEnd(u32VifChn,0xFFFF);

    stCrop.uW = pstAttr->stCapRect.u16Width;
    stCrop.uH = pstAttr->stCapRect.u16Height;
    stCrop.uX = pstAttr->stCapRect.u16X;
    stCrop.uY = pstAttr->stCapRect.u16Y;

    HalVif_Crop(u32VifChn,0, 0, 0x1FFF, 0x1FFF );
    HalVif_CropEnable(u32VifChn,VIF_DISABLE);
#if 1  //work around for shadow not active
    HalDma_Config(u32VifChn,&stCrop,0x08000000,0x08100000);
#else
    HalDma_Config(u32VifChn,&stCrop,0x2266f000,0x2266f000);
#endif
    //HalDma_MaskOutput(u32VifChn,0x1);
    // ISP CLKGen setting

    stCrop.uW /= 2;
    stCrop.uH /= 2;
    HalDma_ConfigSub(u32VifChn,&stCrop,0x08200000,0x08300000);
    //HalDma_MaskOutputSub(u32VifChn,0x1);

    //FPS mask setting
    DrvVif_setChnFPSBitMask(u32VifChn, VIF_CHN_MAIN, pstAttr->eFrameRate, NULL);

    return E_HAL_VIF_SUCCESS;
}

s32 DrvVif_ChnSetConfig_Parallel(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnCfg_t *pstAttr)
{
    WdmaCropParam_t stCrop;

    stCrop.uW = pstAttr->stCapRect.u16Width;
    stCrop.uH = pstAttr->stCapRect.u16Height;
    stCrop.uX = pstAttr->stCapRect.u16X;
    stCrop.uY = pstAttr->stCapRect.u16Y;

    DrvVif_ConfigParallelIF(u32VifChn,
                                        VIF_SENSOR_INPUT_FORMAT_RGB,
                                        VIF_SENSOR_FORMAT_10BIT,
                                        VIF_CLK_POL_NEG,
                                        VIF_CLK_POL_NEG,
                                        VIF_CLK_POL_NEG,
                                        VIF_PIN_POL_POS,
                                        0,
                                        1,
                                        1920,
                                        1080+1,
                                        1,
                                        VIF_MCLK_27M);
/*
    HalDma_Config(u32VifChn,&stCrop,0x08000000,0x08100000);
    HalDma_MaskOutput(u32VifChn,0x1);
*/
    // ISP CLKGen setting
/*
    stCrop.uW /= 2;
    stCrop.uH /= 2;
    HalDma_ConfigSub(u32VifChn,&stCrop,0x08200000,0x08300000);
    HalDma_MaskOutputSub(u32VifChn,0x1);
*/
    //FPS mask setting
    //DrvVif_setChnFPSBitMask(u32VifChn, VIF_CHN_MAIN, pstAttr->eFrameRate, NULL);

    return E_HAL_VIF_SUCCESS;
}


s32 DrvVif_ChnSetConfig_MIPI(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnCfg_t *pstAttr)
{
    WdmaCropParam_t stCrop;
    VIF_SENSOR_FORMAT_e PixDepth;
    u32 group = HalVifGetGroupByChn(u32VifChn);
    //MHal_VIF_IntfMode_e eIntfMode = DrvVif_GetIntfMode(group);
    MHal_VIF_WorkMode_e eWorkMode = DrvVif_GetWorkMode(group);
    MHalHDRType_e hdr_mode = DrvVif_GetHdrMode(group);

    //VIF_DEBUG("[DrvVif_ChnSetConfig_MIPI] group %d, intf %d, work %d, chn %d, hdr chn %d\n", group, eIntfMode, eWorkMode, u32VifChn, u32VifChn_hdr);

    if ((E_MHAL_HDR_TYPE_DOL == hdr_mode) && (u32VifChn != VIF_CHANNEL_0)) {
        VIF_DEBUG("[VIF] HDR mode must use channel 0.");
        return E_HAL_VIF_ERROR;
    }

    VIF_DEBUG("%s , ch=%d, W=%d, H=%d\n",__FUNCTION__,u32VifChn,pstAttr->stCapRect.u16Width,pstAttr->stCapRect.u16Height);

    stCrop.uW = pstAttr->stCapRect.u16Width;
    stCrop.uH = pstAttr->stCapRect.u16Height;
    stCrop.uX = pstAttr->stCapRect.u16X;
    stCrop.uY = pstAttr->stCapRect.u16Y;

    switch (pstAttr->ePixFormat) {
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_RG:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GR:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GB:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_BG:
                PixDepth = VIF_SENSOR_FORMAT_8BIT;
            break;
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_RG:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GR:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GB:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_BG:
                PixDepth = VIF_SENSOR_FORMAT_10BIT;
            break;
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_RG:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GR:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GB:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_BG:
                PixDepth = VIF_SENSOR_FORMAT_16BIT;
            break;
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_RG:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GR:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GB:
        case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_BG:
                PixDepth = VIF_SENSOR_FORMAT_12BIT;
            break;
        default:
            VIF_DEBUG("Sensor pixel format type error %d\n",pstAttr->ePixFormat);
            break;
    }

    HalVif_HDRen(u32VifChn, VIF_DISABLE);
    HalVif_HDRSelect(u32VifChn, VIF_HDR_VC0);

    HalVif_SensorFormat(u32VifChn, PixDepth); //select pixel depth
    HalVif_SensorRgbIn(u32VifChn, VIF_SENSOR_INPUT_FORMAT_RGB);

    HalVif_PixCropStart(u32VifChn, stCrop.uX);
    HalVif_PixCropEnd(u32VifChn, stCrop.uX+stCrop.uW-1);
    HalVif_LineCropStart(u32VifChn, stCrop.uY);
    HalVif_LineCropEnd(u32VifChn, stCrop.uY+stCrop.uH-1);

    HalVif_CropEnable(u32VifChn, VIF_ENABLE);

    HalVif_SelectSource(u32VifChn, group);

    /* ToDo */
    HalVif_SetPclkSource((group<<2), group, 1, 0);
    HalVif_SetPclkSource((group<<2)+1, group, 1, 0);
    HalVif_SetPclkSource((group<<2)+2, group, 1, 0);
    HalVif_SetPclkSource((group<<2)+3, group, 1, 0);

    if (E_MHAL_HDR_TYPE_DOL == hdr_mode) {
        u32 u32VifChn_hdr = HalVifGetHDRChnPair(u32VifChn);
        HalVif_HDRen(u32VifChn, VIF_ENABLE);
        HalVif_HDRSelect(u32VifChn, VIF_HDR_VC1);

        HalVif_SensorFormat(u32VifChn_hdr, PixDepth); //select pixel depth
        HalVif_SensorRgbIn(u32VifChn_hdr, PixDepth);

        HalVif_PixCropStart(u32VifChn_hdr, stCrop.uX);
        HalVif_PixCropEnd(u32VifChn_hdr, stCrop.uX+stCrop.uW-1);
        HalVif_LineCropStart(u32VifChn_hdr, stCrop.uY);
        HalVif_LineCropEnd(u32VifChn_hdr, stCrop.uY+stCrop.uH-1);
        HalVif_CropEnable(u32VifChn_hdr, VIF_ENABLE);

        HalVif_SelectSource(u32VifChn_hdr, group);

        HalVif_SetPclkSource(u32VifChn_hdr, group, 1, 0);
        HalVif_SetPclkSource(u32VifChn_hdr+1, group, 1, 0);
        HalVif_SetPclkSource(u32VifChn_hdr+2, group, 1, 0);
        HalVif_SetPclkSource(u32VifChn_hdr+3, group, 1, 0);

        HalVif_HDRen(u32VifChn_hdr, VIF_ENABLE);
        HalVif_HDRSelect(u32VifChn_hdr, VIF_HDR_VC0);
    }

    if (E_MHAL_VIF_WORK_MODE_RAW_FRAMEMODE == eWorkMode) {
        HalRawDma_Config(u32VifChn, &stCrop, 0x08000000);

        HalDma_MaskOutput(u32VifChn,0x1);
        // ISP CLKGen setting

        /*
        stCrop.uW /= 2;
        stCrop.uH /= 2;
        HalDma_ConfigSub(u32VifChn,&stCrop,0x08200000,0x08300000);
        HalDma_MaskOutputSub(u32VifChn,0x1);
        */
        //FPS mask setting
        DrvVif_setChnFPSBitMask(u32VifChn, VIF_CHN_MAIN, pstAttr->eFrameRate, NULL);
    }

    if ((E_MHAL_VIF_WORK_MODE_RAW_FRAMEMODE == eWorkMode) && (E_MHAL_HDR_TYPE_DOL == hdr_mode)) {
        u32 u32VifChn_hdr = HalVifGetHDRChnPair(u32VifChn);
        HalRawDma_Config(u32VifChn_hdr, &stCrop, 0x08000000);

        HalDma_MaskOutput(u32VifChn_hdr, 0x1);
        // ISP CLKGen setting

        /*
        stCrop.uW /= 2;
        stCrop.uH /= 2;
        HalDma_ConfigSub(u32VifChn,&stCrop,0x08200000,0x08300000);
        HalDma_MaskOutputSub(u32VifChn,0x1);
        */
        //FPS mask setting
        DrvVif_setChnFPSBitMask(u32VifChn_hdr, VIF_CHN_MAIN, pstAttr->eFrameRate, NULL);
    }

    if ((E_MHAL_VIF_WORK_MODE_RAW_REALTIME == eWorkMode) && (E_MHAL_HDR_TYPE_DOL == hdr_mode) && (u32VifChn == 0)) {
        MHalAllocPhyMem_t *pMemAlloc = DrvVif_GetMemAllocator();
        u64 nRtHdrBuf;
        s32 ret;
        u32 u32VifChn_hdr = HalVifGetHDRChnPair(u32VifChn);
        ret = pMemAlloc->alloc(NULL, (stCrop.uW-stCrop.uX)*(stCrop.uH-stCrop.uY)*2, &nRtHdrBuf); //ToDo: parameter for 2?
        if(ret)
        {
            pr_err("Failed to allocate physical memory, req size = 0x%X, err code = 0x%X\n", (stCrop.uW-stCrop.uX)*(stCrop.uH-stCrop.uY)*2, ret);
            return E_HAL_VIF_ERROR;
        }

        vif_drv_ctx.pRtHdrHandle = BufPtrInit(nRtHdrBuf, DrvVifFreeRtHdrBuf);

        HalRawDma_Config(u32VifChn_hdr, &stCrop, (u32)nRtHdrBuf);

        HalDma_MaskOutput(u32VifChn_hdr, 0x1);

    }

    return E_HAL_VIF_SUCCESS;
}

//MAIN Channel
s32 DrvVif_ChnSetConfig(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnCfg_t *pstAttr)
{
    s32 ret = E_HAL_VIF_ERROR;
    u32 group = HalVifGetGroupByChn(u32VifChn);
    MHal_VIF_IntfMode_e eIntfMode = DrvVif_GetIntfMode(group);

    //VIF_DEBUG("[DrvVif_ChnSetConfig] group %d, intf %d, chn %d\n", group, eIntfMode, u32VifChn);

    DrvVif_RegisterFrameStartCb(u32VifChn,(void *)pstAttr->OnFrameStart, 0, NULL);

    if (pstAttr->OnFrameLineCount) {
        DrvVif_SetDmaLineCnt(u32VifChn, pstAttr->u32FrameModeLineCount);
    DrvVif_RegisterLineCntCb(u32VifChn,(void *)pstAttr->OnFrameLineCount, 0, NULL);
    }

    if (pstAttr->OnFrameEnd) {
        if (DrvVif_RegisterFrameEndCb(u32VifChn,(void *)pstAttr->OnFrameEnd, 0, NULL)) {
            return ret;
        }
    }
    VIF_DEBUG("DrvVif_ChnSetConfig interface %d\n", DrvVif_GetIntfMode(group));

    switch (eIntfMode) {
        case E_MHAL_VIF_MODE_BT656:
            ret = DrvVif_ChnSetConfig_BT656(u32VifChn, pstAttr);
            DrvVif_setMCUStoreMode(u32VifChn, 0);
            break;
        case E_MHAL_VIF_MODE_BT1120_STANDARD:
        case E_MHAL_VIF_MODE_BT1120_INTERLEAVED:
            ret = DrvVif_ChnSetConfig_BT1120(u32VifChn, pstAttr);
            ret = DrvVif_ChnSetConfig_BT1120((u32VifChn+4)%12, pstAttr);
            DrvVif_setMCUStoreMode(u32VifChn, 0);
            break;

        case E_MHAL_VIF_MODE_DIGITAL_CAMERA:
            ret = DrvVif_ChnSetConfig_Parallel(u32VifChn, pstAttr);
            DrvVif_setMCUStoreMode(u32VifChn, 2);
            break;
        case E_MHAL_VIF_MODE_MIPI:
            ret = DrvVif_ChnSetConfig_MIPI(u32VifChn, pstAttr);
            DrvVif_setMCUStoreMode(u32VifChn, 2);
            break;
        default:
            VIF_DEBUG("Invaild device interface mode\n");
            break;
    }

    return ret;
}

int DrvVif_LineCountCfg(VIF_CHANNEL_e ch, unsigned int linecnt0, unsigned int linecnt1)
{

    // set interrupt line count for FS & FE
    HalVif_Vif2IspLineCnt0(ch, linecnt0);
    HalVif_Vif2IspLineCnt1(ch, linecnt1);
    return VIF_SUCCESS;
}

int DrvVif_EnableInterrupt(VIF_CHANNEL_e ch, unsigned char en)
{
    if(en){

        HalVif_IrqMask(ch, ulgvif_def_mask[ch]);

    }else{
        HalVif_IrqMask(ch, 0xFF);
    }
    return true;
}




s32 DrvVif_ChnEnable(MHal_VIF_CHN u32VifChn)
{
    //VIF_DEBUG("%s , ch=%d",__FUNCTION__,u32VifChn);
    s32 ret = E_HAL_VIF_SUCCESS;
    u32 group = HalVifGetGroupByChn(u32VifChn);
    MHal_VIF_IntfMode_e eIntfMode = DrvVif_GetIntfMode(group);
    MHal_VIF_WorkMode_e eWorkMode = DrvVif_GetWorkMode(group);
    MHalHDRType_e hdr_mode = DrvVif_GetHdrMode(group);

    //VIF_DEBUG("[DrvVif_ChnEnable] group %d, intf %d, work %d, chn %d, hdr chn %d\n", group, eIntfMode, eWorkMode, u32VifChn, u32VifChn_hdr);

    if ((E_MHAL_HDR_TYPE_DOL == hdr_mode) && (u32VifChn != VIF_CHANNEL_0)) {
        VIF_DEBUG("[VIF] HDR mode must use channel 0.");
        return E_HAL_VIF_ERROR;
    }

    switch (eIntfMode) {
        case E_MHAL_VIF_MODE_BT656:
        case E_MHAL_VIF_MODE_BT1120_STANDARD:
        case E_MHAL_VIF_MODE_BT1120_INTERLEAVED:
            HalDma_Trigger(u32VifChn,WDMA_TRIG_CONTINUE);
            HalDma_EnableIrq(u32VifChn);
            HalVif_EnableIrq(u32VifChn);
            DrvVif_LineCountCfg(u32VifChn,0x0FFF,0x0FFF);
            DrvVif_EnableInterrupt(u32VifChn,VIF_ENABLE);
            HalDma_SetDmaY_WreqThreshold(u32VifChn,3);
            HalDma_SetDmaC_WreqThreshold(u32VifChn,3);
            HalDma_SetSubDmaY_WreqThreshold(u32VifChn,3);
            HalDma_SetSubDmaC_WreqThreshold(u32VifChn,3);
            HalDma_SetDmaY_BurstNumber(u32VifChn,0xF);
            HalDma_SetDmaC_BurstNumber(u32VifChn,0xF);
            HalDma_SetSubDmaY_BurstNumber(u32VifChn,0xF);
            HalDma_SetSubDmaC_BurstNumber(u32VifChn,0xF);
            HalVif_SensorChannelEnable(u32VifChn, VIF_ENABLE);
            HalDma_DmaMaskEnable(u32VifChn,1);
            HalVif_SensorMask(u32VifChn,0);
            HalVif_ChannelAlignment(u32VifChn);
            break;

        case E_MHAL_VIF_MODE_DIGITAL_CAMERA:
        case E_MHAL_VIF_MODE_MIPI:

            if (E_MHAL_VIF_WORK_MODE_RAW_FRAMEMODE == eWorkMode) {
                DrvVif_SetDmaLineCntInt(u32VifChn, 1);
                HalDma_DmaDoneCmdQEnableIrq(u32VifChn);
                DrvVif_FrameStartIrqMask(1<<u32VifChn, 0);
                HalRawDma_Trigger(u32VifChn, WDMA_TRIG_CONTINUE);
            }

            if ((E_MHAL_VIF_WORK_MODE_RAW_FRAMEMODE == eWorkMode) && (E_MHAL_HDR_TYPE_DOL == hdr_mode)) {
                u32 u32VifChn_hdr = HalVifGetHDRChnPair(u32VifChn);
                DrvVif_SetDmaLineCntInt(u32VifChn_hdr, 1);
                HalDma_DmaDoneCmdQEnableIrq(u32VifChn_hdr);
                DrvVif_FrameStartIrqMask(1<<u32VifChn_hdr, 0);
                HalRawDma_Trigger(u32VifChn_hdr, WDMA_TRIG_CONTINUE);
            }
            if ((E_MHAL_VIF_WORK_MODE_RAW_REALTIME == eWorkMode) && (u32VifChn == 0) && (E_MHAL_HDR_TYPE_DOL == hdr_mode)) {
                u32 u32VifChn_hdr = HalVifGetHDRChnPair(u32VifChn);
                vif_drv_ctx.pRtHdrHandle->AddRef(vif_drv_ctx.pRtHdrHandle);
                HalRawDma_Trigger(u32VifChn_hdr, WDMA_TRIG_CONTINUE);
                HalDma_MaskOutput(u32VifChn_hdr, 0x0);
            }

            HalVif_SensorChannelEnable(u32VifChn, VIF_ENABLE);
            HalVif_SensorMask(u32VifChn,VIF_DISABLE);
            if (E_MHAL_HDR_TYPE_DOL == hdr_mode) {
                u32 u32VifChn_hdr = HalVifGetHDRChnPair(u32VifChn);
                HalVif_SensorChannelEnable(u32VifChn_hdr, VIF_ENABLE);
                HalVif_SensorMask(u32VifChn_hdr, VIF_DISABLE);
            }
            break;
        default:
            VIF_DEBUG("[DrvVif_DevEnable]Invaild device interface mode\n");
            ret = E_HAL_VIF_ERROR;
            break;
    }

    return ret;

}


s32 DrvVif_ChnDisable(MHal_VIF_CHN u32VifChn)
{
    //VIF_DEBUG("%s , ch=%d",__FUNCTION__,u32VifChn);
    s32 ret = E_HAL_VIF_SUCCESS;
    u32 group = HalVifGetGroupByChn(u32VifChn);
    MHal_VIF_IntfMode_e eIntfMode = DrvVif_GetIntfMode(group);
    MHal_VIF_WorkMode_e eWorkMode = DrvVif_GetWorkMode(group);
    MHalHDRType_e hdr_mode = DrvVif_GetHdrMode(group);

    if ((E_MHAL_HDR_TYPE_DOL == hdr_mode) && (u32VifChn != VIF_CHANNEL_0)) {
        VIF_DEBUG("[VIF] HDR mode must use channel 0.");
        return E_HAL_VIF_ERROR;
    }

    switch (eIntfMode) {
        case E_MHAL_VIF_MODE_BT656:
        case E_MHAL_VIF_MODE_BT1120_STANDARD:
        case E_MHAL_VIF_MODE_BT1120_INTERLEAVED:
            HalVif_SensorMask(u32VifChn,1);
            DrvVif_LineCountCfg(u32VifChn,0x0FFF,0x0FFF);
            HalVif_SensorChannelEnable(u32VifChn, VIF_DISABLE);
            //HalDma_Trigger(u32VifChn,WDMA_TRIG_STOP);
            HalDma_SetDmaY_WreqThreshold(u32VifChn,0);
            HalDma_SetDmaC_WreqThreshold(u32VifChn,0);
            HalDma_SetSubDmaY_WreqThreshold(u32VifChn,0);
            HalDma_SetSubDmaC_WreqThreshold(u32VifChn,0);
            HalDma_SetDmaY_BurstNumber(u32VifChn,0x0);
            HalDma_SetDmaC_BurstNumber(u32VifChn,0x0);
            HalDma_SetSubDmaY_BurstNumber(u32VifChn,0x0);
            HalDma_SetSubDmaC_BurstNumber(u32VifChn,0x0);
            HalDma_DisableIrq(u32VifChn);
            DrvVif_EnableInterrupt(u32VifChn,VIF_DISABLE);
            HalDma_DmaMaskEnable(u32VifChn,1);
            resetSHMRing(u32VifChn);
            break;

        case E_MHAL_VIF_MODE_DIGITAL_CAMERA:
        case E_MHAL_VIF_MODE_MIPI:
            HalVif_SensorMask(u32VifChn, VIF_ENABLE);
            HalVif_SensorChannelEnable(u32VifChn, VIF_DISABLE);
            if (E_MHAL_HDR_TYPE_DOL == hdr_mode) {
                u32 u32VifChn_hdr = HalVifGetHDRChnPair(u32VifChn);
                HalVif_SensorMask(u32VifChn_hdr, VIF_ENABLE);
                HalVif_SensorChannelEnable(u32VifChn_hdr, VIF_DISABLE);
            }

            if (E_MHAL_VIF_WORK_MODE_RAW_FRAMEMODE == eWorkMode) {
                DrvVif_SetDmaLineCntInt(u32VifChn, 0);
                HalDma_DmaDoneCmdQDisableIrq(u32VifChn);
                HalRawDma_Trigger(u32VifChn, WDMA_TRIG_STOP);
            }

            if ((E_MHAL_VIF_WORK_MODE_RAW_FRAMEMODE == eWorkMode) && (E_MHAL_HDR_TYPE_DOL == hdr_mode)) {
                u32 u32VifChn_hdr = HalVifGetHDRChnPair(u32VifChn);
                DrvVif_SetDmaLineCntInt(u32VifChn_hdr, 0);
                HalDma_DmaDoneCmdQDisableIrq(u32VifChn_hdr);
                HalRawDma_Trigger(u32VifChn_hdr, WDMA_TRIG_STOP);
            }

            if ((E_MHAL_VIF_WORK_MODE_RAW_REALTIME == eWorkMode) && (E_MHAL_HDR_TYPE_DOL == hdr_mode) && (u32VifChn == 0)) {
                u32 u32VifChn_hdr = HalVifGetHDRChnPair(u32VifChn);
                HalRawDma_Trigger(u32VifChn_hdr, WDMA_TRIG_STOP);
                HalDma_MaskOutput(u32VifChn_hdr, 0x1);
                VIF_MS_SLEEP(50);
                vif_drv_ctx.pRtHdrHandle->Release(vif_drv_ctx.pRtHdrHandle);
            }

            break;
        default:
            VIF_DEBUG("[DrvVif_DevEnable]Invaild device interface mode\n");
            ret = E_HAL_VIF_ERROR;
            break;
    }

    return ret;
}

s32 DrvVif_ChnQuery(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnStat_t *pstStat)
{
    //VIF_DEBUG("%s , ch=%d",__FUNCTION__,u32VifChn);

	WdmaInfo_t WdmaInfo;
    VifInfo_t VifInfo;
    u32 tmp = 0;
    int i=0;

	pstStat->nReadIdx = SHM_Ring[u32VifChn].nReadIdx;
	pstStat->nWriteIdx = SHM_Ring[u32VifChn].nWriteIdx;
	pstStat->nDequeueIdx = SHM_Ring[u32VifChn].pre_nReadIdx;
    // 8051 Cail C byte order convert to ARM format
	tmp =  ((SHM_Ring[u32VifChn].nDropFrameCnt >> 8) & 0x00FF00FF) + ((SHM_Ring[u32VifChn].nDropFrameCnt << 8) & 0xFF00FF00);
	pstStat->nDropFrameCnt = ((tmp >> 16) & 0x0000FFFF) + ((tmp << 16) & 0xFFFF0000);

	tmp =  ((SHM_Ring[u32VifChn].nFrameDoneCnt >> 8) & 0x00FF00FF) + ((SHM_Ring[u32VifChn].nFrameDoneCnt << 8) & 0xFF00FF00);
    pstStat->nFrameDoneCnt = ((tmp >> 16) & 0x0000FFFF) + ((tmp << 16) & 0xFFFF0000);

    tmp =  ((SHM_Ring[u32VifChn].nFrameStartCnt >> 8) & 0x00FF00FF) + ((SHM_Ring[u32VifChn].nFrameStartCnt << 8) & 0xFF00FF00);
    pstStat->nFrameStartCnt = ((tmp >> 16) & 0x0000FFFF) + ((tmp << 16) & 0xFFFF0000);

    tmp =  ((SHM_Ring[u32VifChn].eFieldIdErr >> 8) & 0x00FF00FF) + ((SHM_Ring[u32VifChn].eFieldIdErr << 8) & 0xFF00FF00);
    pstStat->u32VbFail = ((tmp >> 16) & 0x0000FFFF) + ((tmp << 16) & 0xFFFF0000);

	HalDma_GetDmaInfo(u32VifChn,&WdmaInfo);
	pstStat->nOutputWidth = WdmaInfo.uWidth;
	pstStat->nOutputHeight = WdmaInfo.uHeight;
	HalDma_GetSubDmaInfo(u32VifChn,&WdmaInfo);
	pstStat->nSubOutputWidth = WdmaInfo.uWidth;
	pstStat->nSubOutputHeight = WdmaInfo.uHeight;
	HalVif_GetVifInfo(u32VifChn,&VifInfo);
    pstStat->nReceiveWidth = VifInfo.uReceiveWidth;
    pstStat->nReceiveHeight = VifInfo.uReceiveHeight;

    for(i=0 ; i< VIF_RING_QUEUE_SIZE; i++){
        pstStat->eStatus[i] = SHM_Ring[u32VifChn].data[i].nStatus;
        //printk("<%d,%d>",pstStat->eStatus[i],SHM_Ring[u32VifChn].data[i].nStatus);
    }


    return E_HAL_VIF_SUCCESS;
}


//SUB Channel
s32 DrvVif_SubChnSetConfig(MHal_VIF_CHN u32VifChn, MHal_VIF_SubChnCfg_t *pstAttr)
{
    //VIF_DEBUG("%s , ch=%d",__FUNCTION__,u32VifChn);
    //FPS mask setting
    DrvVif_setChnFPSBitMask(u32VifChn, VIF_CHN_SUB, pstAttr->eFrameRate, NULL);

    return E_HAL_VIF_SUCCESS;
}


s32 DrvVif_SubChnEnable(MHal_VIF_CHN u32VifChn)
{
    //VIF_DEBUG("%s , ch=%d",__FUNCTION__,u32VifChn);
    HalDma_TriggerSub(u32VifChn,WDMA_TRIG_CONTINUE);
    return E_HAL_VIF_SUCCESS;
}


s32 DrvVif_SubChnDisable(MHal_VIF_CHN u32VifChn)
{
    //VIF_DEBUG("%s , ch=%d",__FUNCTION__,u32VifChn);
    HalDma_TriggerSub(u32VifChn,WDMA_TRIG_STOP);
    return E_HAL_VIF_SUCCESS;
}


s32 DrvVif_SubChnQuery(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnStat_t *pstStat)
{

    //VIF_DEBUG("%s , ch=%d",__FUNCTION__,u32VifChn);

    WdmaInfo_t WdmaInfo;
    VifInfo_t VifInfo;
    u32 tmp = 0;
    int i=0;
    int uWdmaChn;

    uWdmaChn = u32VifChn + 16;

    pstStat->nReadIdx = SHM_Ring[uWdmaChn].nReadIdx;
    pstStat->nWriteIdx = SHM_Ring[uWdmaChn].nWriteIdx;
    pstStat->nDequeueIdx = SHM_Ring[uWdmaChn].pre_nReadIdx;
    // 8051 Cail C byte order convert to ARM format
    tmp =  ((SHM_Ring[uWdmaChn].nDropFrameCnt >> 8) & 0x00FF00FF) + ((SHM_Ring[uWdmaChn].nDropFrameCnt << 8) & 0xFF00FF00);
    pstStat->nDropFrameCnt = ((tmp >> 16) & 0x0000FFFF) + ((tmp << 16) & 0xFFFF0000);

    tmp =  ((SHM_Ring[uWdmaChn].nFrameDoneSubCnt >> 8) & 0x00FF00FF) + ((SHM_Ring[uWdmaChn].nFrameDoneSubCnt << 8) & 0xFF00FF00);
    pstStat->nFrameDoneCnt = ((tmp >> 16) & 0x0000FFFF) + ((tmp << 16) & 0xFFFF0000);

    tmp =  ((SHM_Ring[uWdmaChn].nFrameStartCnt >> 8) & 0x00FF00FF) + ((SHM_Ring[uWdmaChn].nFrameStartCnt << 8) & 0xFF00FF00);
    pstStat->nFrameStartCnt = ((tmp >> 16) & 0x0000FFFF) + ((tmp << 16) & 0xFFFF0000);

    tmp =  ((SHM_Ring[uWdmaChn].eFieldIdErr >> 8) & 0x00FF00FF) + ((SHM_Ring[uWdmaChn].eFieldIdErr << 8) & 0xFF00FF00);
    pstStat->u32VbFail = ((tmp >> 16) & 0x0000FFFF) + ((tmp << 16) & 0xFFFF0000);

    HalDma_GetDmaInfo(u32VifChn,&WdmaInfo);
    pstStat->nOutputWidth = WdmaInfo.uWidth;
    pstStat->nOutputHeight = WdmaInfo.uHeight;
    HalDma_GetSubDmaInfo(u32VifChn,&WdmaInfo);
    pstStat->nSubOutputWidth = WdmaInfo.uWidth;
    pstStat->nSubOutputHeight = WdmaInfo.uHeight;
    HalVif_GetVifInfo(u32VifChn,&VifInfo);
    pstStat->nReceiveWidth = VifInfo.uReceiveWidth;
    pstStat->nReceiveHeight = VifInfo.uReceiveHeight;

    for(i=0 ; i< VIF_RING_QUEUE_SIZE; i++){
        pstStat->eStatus[i] = SHM_Ring[uWdmaChn].data[i].nStatus;
        //printk("<%d,%d>",pstStat->eStatus[i],SHM_Ring[u32VifChn].data[i].nStatus);
    }


    return E_HAL_VIF_SUCCESS;

    return E_HAL_VIF_SUCCESS;
}


//Ring buffer queue/dequeue
s32 DrvVif_QueueFrameBuffer(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, const MHal_VIF_RingBufElm_t *ptFbInfo)
{
    int mcu_chn = u32VifChn + ((u32ChnPort>0)? VIF_PORT_NUM/2:0);
    u16 w_idx = SHM_Ring[mcu_chn].nWriteIdx;

    /* DEBUG */
    //#define DRAM_BASE 0x20000000
    pr_debug("%s Port=%d, BufY=0x%X, BufC=0x%X",__FUNCTION__,u32ChnPort,(u32)ptFbInfo->u64PhyAddr[0],(u32)ptFbInfo->u64PhyAddr[1]);
    pr_debug("Stride0=0x%X, Stride1=0x%X",(u32)ptFbInfo->u32Stride[0],(u32)ptFbInfo->u32Stride[1]);
    /*
    if(u32ChnPort==0)
        HalDma_SetOutputAddr(u32VifChn,
            Chip_Phys_to_MIU(ptFbInfo->u64PhyAddr[0]),
            Chip_Phys_to_MIU(ptFbInfo->u64PhyAddr[1]),
            ptFbInfo->u32Stride[0]);
    else
        HalDma_SetOutputAddrSub(u32VifChn,
            Chip_Phys_to_MIU(ptFbInfo->u64PhyAddr[0]),
            Chip_Phys_to_MIU(ptFbInfo->u64PhyAddr[1]),
            ptFbInfo->u32Stride[0]);

    return E_HAL_VIF_SUCCESS;
    */
    pr_debug("[VIF] Channel[%d] Port[%d] Ququeue frame!\n",u32VifChn,u32ChnPort);
    if (E_MHAL_VIF_MODE_BT656 != DrvVif_GetIntfMode(u32VifChn >> 2)) {
        SHM_Ring[u32VifChn].nEnable =1;
    }

    if (!SHM_Ring[mcu_chn].nEnable) {
        //SHM_Ring[u32VifChn].nEnable =1;
        pr_debug("Port[%d] do not ready to enqueue!!\n",u32VifChn);
        return MHAL_FAILURE;
    }

    if (SHM_Ring[mcu_chn].data[w_idx].nStatus != E_VIF_BUF_INVALID) {
        //SHM_Ring[u32VifChn].data[w_idx] = *ptFbInfo;
        pr_debug("Ring buffer full!! Waiting for dequeue!!\n");
        return MHAL_FAILURE;
    }

    //convert Mhal elm to MCU elm
    if(DrvVif_GetWorkMode(u32VifChn/4)==E_MHAL_VIF_WORK_MODE_RAW_FRAMEMODE)
        convertMhalRgbBuffElmtoMCU(&(SHM_Ring[mcu_chn].data[w_idx]), ptFbInfo);
    else
        convertMhalBuffElmtoMCU(&(SHM_Ring[mcu_chn].data[w_idx]), ptFbInfo);

    SHM_Ring[mcu_chn].data[w_idx].nStatus = E_VIF_BUF_EMPTY;
    wmb();

    if ((w_idx +1) == VIF_RING_QUEUE_SIZE)
        SHM_Ring[mcu_chn].nWriteIdx = 0;
    else
        SHM_Ring[mcu_chn].nWriteIdx++;

    wmb();

    return E_HAL_VIF_SUCCESS;
}


s32 DrvVif_QueryFrames(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, u32 *pNumBuf)
{
    int mcu_chn = u32VifChn + ((u32ChnPort>0)? VIF_PORT_NUM/2:0);
    u8 p_ridx = SHM_Ring[mcu_chn].pre_nReadIdx;

    u32 readyNum = 0;
    while(readyNum < VIF_RING_QUEUE_SIZE && SHM_Ring[mcu_chn].data[p_ridx].nStatus == E_VIF_BUF_READY) {
        readyNum++;
        p_ridx = (p_ridx+1)%VIF_RING_QUEUE_SIZE;
    }
    *pNumBuf = readyNum;
    rmb();
    //VIF_DEBUG("Port[%d] Number of ready frame = %d \n",u32VifChn, *pNumBuf);
    return E_HAL_VIF_SUCCESS;
}

#if 0  //for debug repeat fieldID
MHalFieldType_e tmp[32]={0};
u32 tmp_fd=0;
u32 tmp_id=0;
u32 aa=0;
#endif
s32 DrvVif_DequeueFrameBuffer(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, MHal_VIF_RingBufElm_t *ptFbInfo)
{
    int mcu_chn = u32VifChn + ((u32ChnPort>0)? VIF_PORT_NUM/2:0);
    u8 p_ridx = SHM_Ring[mcu_chn].pre_nReadIdx;

    //VIF_DEBUG("[VIF] Channel[%d] Port[%d] Dequeue frame!\n",u32VifChn,u32ChnPort);

    //Chip_Flush_Cache_Range((unsigned long)phys_to_virt(ptFbInfo->u64PhyAddr[0]), ptFbInfo->nCropW*ptFbInfo->nCropH);
    //Chip_Flush_Cache_Range((unsigned long)phys_to_virt(ptFbInfo->u64PhyAddr[1]), ((ptFbInfo->nCropW + 31)/32)*32 * ptFbInfo->nCropH);



    if (1/*SHM_Ring[mcu_chn].data[p_ridx].nStatus == E_VIF_BUF_READY*/) {
        if(DrvVif_GetWorkMode(u32VifChn/4)==E_MHAL_VIF_WORK_MODE_RAW_FRAMEMODE)
            convertMCURgbBuffElmtoMhal(&(SHM_Ring[mcu_chn].data[p_ridx]), ptFbInfo, mcu_chn);
        else
            convertMCUBuffElmtoMhal(&(SHM_Ring[mcu_chn].data[p_ridx]), ptFbInfo, mcu_chn);

        rmb();

        SHM_Ring[mcu_chn].data[p_ridx].nStatus = E_VIF_BUF_INVALID;
        wmb();

        if (SHM_Ring[mcu_chn].pre_nReadIdx +1 == VIF_RING_QUEUE_SIZE)
            SHM_Ring[mcu_chn].pre_nReadIdx = 0;
        else
            SHM_Ring[mcu_chn].pre_nReadIdx++;
        wmb();

#if 0  //for debug repeat fieldID
        aa =  ((SHM_Ring[mcu_chn].nDropFrameCnt >> 8) & 0x00FF00FF) + ((SHM_Ring[mcu_chn].nDropFrameCnt << 8) & 0xFF00FF00);
        tmp_fd = ((aa >> 16) & 0x0000FFFF) + ((aa << 16) & 0xFFFF0000);

        aa =  ((SHM_Ring[mcu_chn].eFieldIdErr >> 8) & 0x00FF00FF) + ((SHM_Ring[mcu_chn].eFieldIdErr << 8) & 0xFF00FF00);
        tmp_id = ((aa >> 16) & 0x0000FFFF) + ((aa << 16) & 0xFFFF0000);


        if(tmp[mcu_chn] == ptFbInfo->eFieldType){
           printk("[ch:%d]!!!%d  drop:%d, id:%d   pts:%d\n",mcu_chn,ptFbInfo->eFieldType,tmp_fd,tmp_id,ptFbInfo->nPTS);
        }

        //if(mcu_chn == 8)
        //     printk("[ch:%d]!!!%d  drop:%d, id:%d   pts:%d\n",mcu_chn,ptFbInfo->eFieldType,tmp_fd,tmp_id,ptFbInfo->nPTS);

        tmp[mcu_chn] = ptFbInfo->eFieldType;
#endif

    } else {
        return E_HAL_VIF_SUCCESS;
    }
    return E_HAL_VIF_SUCCESS;
}


s32 DrvVif_setChnFPSBitMask(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, MHal_VIF_FrameRate_e u32Chnfps, u32 *manualMask) {
    int mcu_chn = u32VifChn + ((u32ChnPort>0)? VIF_PORT_NUM/2:0);


    static u8 uShiftCnt = 0;
    static u8 uShiftCntSub = 0;

    switch(u32Chnfps) {
        case E_MHAL_VIF_FRAMERATE_FULL:
        case E_MHAL_VIF_FRAMERATE_HALF:
        case E_MHAL_VIF_FRAMERATE_QUARTER:
        case E_MHAL_VIF_FRAMERATE_OCTANT:
        case E_MHAL_VIF_FRAMERATE_THREE_QUARTERS:
            SHM_Ring[mcu_chn].nFPS_bitMap = (FpsBitMaskArray[u32Chnfps] << uShiftCnt) | (FpsBitMaskArray[u32Chnfps] >> (32 - uShiftCnt));
        break;
        case E_MHAL_VIF_FRAMERATE_MANUAL:
            if (manualMask) {
                pr_err("FPS manual mask is 0x%x!!\n",*manualMask);
                SHM_Ring[mcu_chn].nFPS_bitMap = convertFPSMaskToMCU(*manualMask);
            } else {
                pr_err("FPS manual mask is NULL!!\n");
                SHM_Ring[mcu_chn].nFPS_bitMap = FpsBitMaskArray[E_MHAL_VIF_FRAMERATE_FULL];
            }
        break;
        default:
            SHM_Ring[mcu_chn].nFPS_bitMap = FpsBitMaskArray[E_MHAL_VIF_FRAMERATE_FULL];
            pr_err("MHal_VIF_FrameRate out of range!!\n");
    }

/*    if (u32Chnfps >= E_MHAL_VIF_FRAMERATE_MAX) {
        SHM_Ring[mcu_chn].nFPS_bitMap = FpsBitMaskArray[E_MHAL_VIF_FRAMERATE_FULL];
        pr_err("MHal_VIF_FrameRate out of range!!\n");
    } else {
        SHM_Ring[mcu_chn].nFPS_bitMap = FpsBitMaskArray[u32Chnfps];
    }
*/

    if(u32ChnPort < 1)
        uShiftCnt++;
    else
        uShiftCntSub++;

    return E_HAL_VIF_SUCCESS;
}

// MCU control function

int DrvVif_setARMControlStatus(u16 status) {
    status = status & 0xFF;
    REG_W(g_MAILBOX, MAILBOX_CONCTRL_REG, status);
    VIF_MS_SLEEP(20);
    return E_HAL_VIF_SUCCESS;
}

int DrvVif_setMCUStoreMode(u8 ch, u8 mode) {
    u16 tmp;

    if ((ch ==0) || (ch ==4) || (ch ==8) || (ch ==12)) {
        tmp  =  REG_R(g_MAILBOX, MAILBOX_CONCTRL_REG);
        tmp &= ~(0x3 << ((ch>>1)+8));
        tmp |= (mode << ((ch>>1)+8));
        REG_W(g_MAILBOX, MAILBOX_CONCTRL_REG, tmp);
        VIF_MS_SLEEP(20);
    }


    return E_HAL_VIF_SUCCESS;
}

int DrvVif_getMCUStatus(u16 *status) {
    *status = REG_R(g_MAILBOX,MAILBOX_STATE_REG);
    VIF_MS_SLEEP(1000);
    return E_HAL_VIF_SUCCESS;
}

u32 DrvVif_changeMCUStatus(u16 status) {
    u16 m_st = 0xFFFF;
    u16 wait_count = 0;

    if (status >= MCU_STATE_NUM)
        VIF_DEBUG("Setting status out of bound!!\n");

    DrvVif_setARMControlStatus(status);
    DrvVif_getMCUStatus(&m_st);
    while(m_st != status) {
        VIF_DEBUG("Waititg for MCU %s done~~~....(%x)\n",MCU_state_tlb[status],m_st);
        DrvVif_getMCUStatus(&m_st);
        ++wait_count;

        if (wait_count >MCU_REG_TIMEOUT)
            return E_HAL_VIF_ERROR;
    }

    VIF_DEBUG("MCU %s id done(%x)\n",MCU_state_tlb[status],m_st);

    return E_HAL_VIF_SUCCESS;
}

u32 DrvVif_stopMCU(void) {
    CamOsMutexLock(&mculock);
    //Get reg initial value, must be STOP
    if (DrvVif_changeMCUStatus(MCU_STATE_STOP)) {
        CamOsMutexUnlock(&mculock);
        return E_HAL_VIF_ERROR;
    }
    CamOsMutexUnlock(&mculock);
    return E_HAL_VIF_SUCCESS;
}


u32 DrvVif_startMCU(void) {
    CamOsMutexLock(&mculock);
    //Initial success, set to READY mode
    if (DrvVif_changeMCUStatus(MCU_STATE_READY)) {
        CamOsMutexUnlock(&mculock);
        return E_HAL_VIF_ERROR;
    }
    CamOsMutexUnlock(&mculock);
    return E_HAL_VIF_SUCCESS;
}


u32 DrvVif_pollingMCU(void) {
    u16 m_st = 0x00FF;
    DrvVif_getMCUStatus(&m_st);
    if (m_st == MCU_STATE_STOP) {
        pr_err("Invalid operation from stop state to polling!!\n");
        return E_HAL_VIF_ERROR;
    }
    //Enter POLLING mode
    CamOsMutexLock(&mculock);
    if (DrvVif_changeMCUStatus(MCU_STATE_POLLING)) {
        CamOsMutexUnlock(&mculock);
        return E_HAL_VIF_ERROR;
    }
    CamOsMutexUnlock(&mculock);
    return E_HAL_VIF_SUCCESS;
}


u32 DrvVif_pauseMCU(void) {
    CamOsMutexLock(&mculock);
    if (DrvVif_changeMCUStatus(MCU_STATE_READY)) {
        CamOsMutexUnlock(&mculock);
        return E_HAL_VIF_ERROR;
    }
    CamOsMutexUnlock(&mculock);
    return E_HAL_VIF_SUCCESS;
}


/*******************************************************************************************8*/


// For MV5 use VIF 3 channel
void DrvVif_ISR(void)
{
#if 0
    //volatile u32 u4Clear0 = 0;
    //volatile u32 u4Clear1 = 0;
    //volatile u32 u4Clear2 = 0;

    volatile u32 u4Status0 = 0;
    volatile u32 u4Status1 = 0;
    volatile u32 u4Status2 = 0;

    u4Status0 = HalVif_IrqFinalStatus(VIF_CHANNEL_0);
    //HalVif_IrqMask(VIF_CHANNEL_0, u4Status0);

    u4Status1 = HalVif_IrqFinalStatus(VIF_CHANNEL_1);
    //HalVif_IrqMask(VIF_CHANNEL_1, u4Status1);

    u4Status2 = HalVif_IrqFinalStatus(VIF_CHANNEL_2);
    //HalVif_IrqMask(VIF_CHANNEL_2, u4Status2);


    //UartSendTrace("## [%s] mask1 0x%04x, mask2 0x%04x, mask3 0x%04x\n", __func__, u4Status, u4Status2, u4Status3);

	/***********************************
	* LINECNT 0
	***********************************/
	// Channel_0
    if(VIF_CHECKBITS(u4Status0, VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0))
    {
        //wake_up_interruptible_all(&isp_wq_VSTART);
        MsFlagSetbits(&_ints_event_flag, VIF_CH0_FRAME_START_INTS);
        HalVif_IrqClr(VIF_CHANNEL_0, VIF_MASK(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0));
    }

	//Channel_01
    if(VIF_CHECKBITS(u4Status1, VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0))
    {
        //wake_up_interruptible_all(&isp_wq_VSTART);
        MsFlagSetbits(&_ints_event_flag, VIF_CH1_FRAME_START_INTS);
        HalVif_IrqClr(VIF_CHANNEL_1, VIF_MASK(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0));
    }

	// Channel_2
    if(VIF_CHECKBITS(u4Status2, VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0))
    {
        //wake_up_interruptible_all(&isp_wq_VSTART);
        MsFlagSetbits(&_ints_event_flag, VIF_CH2_FRAME_START_INTS);
        HalVif_IrqClr(VIF_CHANNEL_2, VIF_MASK(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT0));
    }

	/***********************************
	* LINECNT 1
	***********************************/
	// Channel_0
    if(VIF_CHECKBITS(u4Status0, VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1))
    {
        //wake_up_interruptible_all(&isp_wq_VSTART);
        MsFlagSetbits(&_ints_event_flag, VIF_CH0_FRAME_END_INTS);
        HalVif_IrqClr(VIF_CHANNEL_0, VIF_MASK(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1));
    }

	// Channel_0
    if(VIF_CHECKBITS(u4Status1, VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1))
    {
        //wake_up_interruptible_all(&isp_wq_VSTART);
        MsFlagSetbits(&_ints_event_flag, VIF_CH1_FRAME_END_INTS);
        HalVif_IrqClr(VIF_CHANNEL_1, VIF_MASK(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1));
    }

	// Channel_0
    if(VIF_CHECKBITS(u4Status2, VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1))
    {
        //wake_up_interruptible_all(&isp_wq_VSTART);
        MsFlagSetbits(&_ints_event_flag, VIF_CH2_FRAME_END_INTS);
        HalVif_IrqClr(VIF_CHANNEL_2, VIF_MASK(VIF_INTERRUPT_VIF_TO_ISP_LINE_COUNT_HIT1));
    }
#endif
}

#if 0
int DrvVif_IntsSimulator(VIF_INTS_EVENT_TYPE_e ints)
{
    MsFlagSetbits(&_ints_event_flag, ints);
	return VIF_SUCCESS;
}

int DrvVif_ISRCreate()
{
    MsIntInitParam_u param;

    memset(&param, 0, sizeof(param));
    param.intc.eMap = INTC_MAP_IRQ;
    param.intc.ePriority = INTC_PRIORITY_10;
    param.intc.pfnIsr = DrvVif_ISR;
    MsInitInterrupt(&param, MS_INT_NUM_IRQ_VIF);
    MsUnmaskInterrupt(MS_INT_NUM_IRQ_VIF);

	return VIF_SUCCESS;
}
#endif
int DrvVif_CLK(void)
{

	return VIF_SUCCESS;
}

int DrvVif_PatGenCfg(VIF_CHANNEL_e ch, VifPatGenCfg_t *PatGenCfg)
{

	return VIF_SUCCESS;
}

int DrvVif_SensorFrontEndCfg(VIF_CHANNEL_e ch, VifSensorCfg_t *sensorcfg)
{

	// set interrupt line count for FS & FE
	HalVif_Vif2IspLineCnt0(ch, sensorcfg->FrameStartLineCount);
	HalVif_Vif2IspLineCnt1(ch, sensorcfg->FrameEndLineCount);
	return VIF_SUCCESS;
}

int DrvVif_SensorReset(VIF_CHANNEL_e ch, int reset)
{
    HalVif_SensorReset(ch, reset);//h0000, bit: 2
	return VIF_SUCCESS;
}

int DrvVif_SensorPdwn(VIF_CHANNEL_e ch, int Pdwn)
{
    if(Pdwn){
        HalVif_SensorSWReset(ch, VIF_DISABLE);//h0000, bit: 0
        HalVif_SensorPowerDown(ch, VIF_ENABLE);//h0000, bit: 3
    }
    else{ //Enable sensor
        HalVif_SensorSWReset(ch, VIF_ENABLE);//h0000, bit: 0
        HalVif_SensorPowerDown(ch, VIF_DISABLE);//h0000, bit: 3
    }
	return VIF_SUCCESS;
}

int DrvVif_ChannelEnable(VIF_CHANNEL_e ch, bool ben)
{
    if(ben){
        HalVif_SensorChannelEnable(ch, VIF_ENABLE);//h0000, bit: 15
    }
    else{ //Enable sensor
        HalVif_SensorChannelEnable(ch, VIF_DISABLE);//h0000, bit: 15
    }
    return VIF_SUCCESS;
}

int DrvVif_SetDefaultIntsMask(void)
{
    u32 ulloop=0;

    for(ulloop = 0; ulloop < VIF_CHANNEL_NUM; ++ulloop){
        HalVif_IrqMask(ulloop, ulgvif_def_mask[ulloop]);
    }

#if 0
    unsigned int irqDefaultMask = VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE);

    HalVif_IrqMask(VIF_CHANNEL_0, irqDefaultMask);
    HalVif_IrqMask(VIF_CHANNEL_1, irqDefaultMask);
    HalVif_IrqMask(VIF_CHANNEL_2, irqDefaultMask);
#endif

    return VIF_SUCCESS;
}

int DrvVif_SetVifChanelBaseAddr(void)
{
    u32 ulloop=0;

    for(ulloop = 0; ulloop < VIF_CHANNEL_NUM; ++ulloop)
    {
		HalVif_SetVifChanelBaseAddr(ulloop);
    }

#if 0
    unsigned int irqDefaultMask = VIF_SHIFTBITS(VIF_INTERRUPT_VREG_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_VREG_FALLING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_HW_FLASH_STROBE_DONE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_RISING_EDGE)|
    VIF_SHIFTBITS(VIF_INTERRUPT_PAD2VIF_VSYNC_FALLING_EDGE);

    HalVif_IrqMask(VIF_CHANNEL_0, irqDefaultMask);
    HalVif_IrqMask(VIF_CHANNEL_1, irqDefaultMask);
    HalVif_IrqMask(VIF_CHANNEL_2, irqDefaultMask);
#endif

    return VIF_SUCCESS;
}

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
                            )
{
    volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*) g_TOPPAD1;
    VIF_CHANNEL_SOURCE_e source = VIF_CH_SRC_PARALLEL_SENSOR_0;
    VIF_ONOFF_e mask;

    switch(ch)
    {
    case VIF_CHANNEL_0:
    case VIF_CHANNEL_4:
    case VIF_CHANNEL_8:
    case VIF_CHANNEL_12:
    break;
    default:
        pr_err("[VIF] Invaild channel for parallel interface.\n");
    break;
    }

    mask = HalVif_SensorMask(ch,VIF_DISABLE);
    //Disable HDR as default.
    HalVif_HDRen(ch, VIF_DISABLE);// h0000, bit: 4
    HalVif_HDRSelect(ch, VIF_HDR_SRC_MIPI0);// h0000, bit: 5~7

    HalVif_SensorFormatLeftSht(ch, VIF_DISABLE);// h0006, bit: 3
    HalVif_SensorBitSwap(ch, VIF_DISABLE);// h0006, bit: 7

    //HalVif_SelectSource(ch,src);  //select vif source
    HalVif_SensorFormat(ch,PixDepth); //select pixel depth

    //VIF_SENSOR_POLARITY_HIGH_ACTIVE,
    //VIF_SENSOR_POLARITY_LOW_ACTIVE,
    HalVif_SensorPclkPolarity(ch,PclkPol==VIF_CLK_POL_POS ? VIF_SENSOR_POLARITY_HIGH_ACTIVE : VIF_SENSOR_POLARITY_LOW_ACTIVE);
    HalVif_SensorHsyncPolarity(ch,HsyncPol==VIF_CLK_POL_POS ? VIF_SENSOR_POLARITY_HIGH_ACTIVE : VIF_SENSOR_POLARITY_LOW_ACTIVE);
    //HalVif_SensorVsyncPolarity(ch,VsyncPol==VIF_CLK_POL_POS?VIF_SENSOR_POLARITY_HIGH_ACTIVE:VIF_SENSOR_POLARITY_LOW_ACTIVE);
    HalVif_SensorVsyncPolarity(ch,VsyncPol==VIF_CLK_POL_POS ? VIF_SENSOR_POLARITY_HIGH_ACTIVE: VIF_SENSOR_POLARITY_LOW_ACTIVE); //need to check with designer

    if(VIF_SENSOR_INPUT_FORMAT_RGB == svif_sensor_in_format){ //TBD. Ask designer.
        HalVif_SensorFormatExtMode(ch,VIF_SENSOR_BIT_MODE_1);
    }
    else{
        HalVif_SensorFormatExtMode(ch,VIF_SENSOR_BIT_MODE_0);
    }
    HalVif_SensorRgbIn(ch, svif_sensor_in_format);

    HalVif_PixCropStart(ch,CropX);
    HalVif_PixCropEnd(ch,CropX+CropW-1);
    if(CropY==0)
    {
        HalVif_LineCropStart(ch,1);
        HalVif_LineCropEnd(ch,1+CropH-1);
    }
    else
    {
        HalVif_LineCropStart(ch,CropY);
        HalVif_LineCropEnd(ch,CropY+CropH-1);
    }

    HalVif_CropEnable(ch,CropEnable);

    HalVif_SetPclkSource(ch,VIF_PLCK_SENSOR, 0, 0);
    HalVif_SetPclkSource(ch+1,VIF_PLCK_SENSOR, 0, 0);
    HalVif_SetPclkSource(ch+2,VIF_PLCK_SENSOR, 0, 0);
    HalVif_SetPclkSource(ch+3,VIF_PLCK_SENSOR, 0, 0);
    HalVif_SetMCLK(ch,Mclk);

    HalVif_SensorReset(ch,RstPol);
    msleep(1);
    HalVif_SensorReset(ch,~RstPol);

    switch(ch)
    {
    case 0:
        //padtop->reg_ccir0_clk_mode = 1;//PCLK SNR GPIO2
        padtop->reg_snr0_in_mode = 1;
        //padtop->reg_ccir0_ctrl_mode = 1; //PDWN/RST/MCLK
        padtop->reg_snr0_d_ie = 0x03FF;
        padtop->reg_snr0_gpio_drv = 0xFF;
        padtop->reg_snr0_gpio_ie = 0x07;
        source = VIF_CH_SRC_PARALLEL_SENSOR_0;
        break;
    case 4:
        //padtop->reg_ccir1_clk_mode = 1;//PCLK SNR GPIO2
        padtop->reg_snr1_in_mode = 1;
        //padtop->reg_ccir1_ctrl_mode = 1; //PDWN/RST/MCLK
        padtop->reg_snr1_d_ie = 0x03FF;
        padtop->reg_snr1_gpio_drv = 0xFF;
        padtop->reg_snr1_gpio_ie = 0x01;
        source = VIF_CH_SRC_PARALLEL_SENSOR_1;;
        break;
    case 8:
        //padtop->reg_ccir2_clk_mode = 1;//PCLK SNR GPIO2
        padtop->reg_snr2_in_mode = 1;
        //padtop->reg_ccir2_ctrl_mode = 1; //PDWN/RST/MCLK
        padtop->reg_snr2_d_ie = 0x03FF;
        padtop->reg_snr2_gpio_drv = 0x00FF;
        padtop->reg_snr2_gpio_ie = 0x07;
        source = VIF_CH_SRC_PARALLEL_SENSOR_2;
        break;
    case 12:
        //padtop->reg_ccir3_clk_mode = 1;//PCLK SNR GPIO2
        padtop->reg_snr3_in_mode = 1;
        //padtop->reg_ccir3_ctrl_mode = 1; //PDWN/RST/MCLK
        padtop->reg_snr3_d_ie = 0x03FF;
        padtop->reg_snr3_gpio_drv = 0xFF;
        padtop->reg_snr3_gpio_ie = 0x07;
        source = VIF_CH_SRC_PARALLEL_SENSOR_3;
        break;
    default:
        pr_info("[VIF] Invalid channel for parallel sensor.");
        break;
    }
    HalVif_SelectSource(ch,source);
    HalVif_EnableParallelIF(ch,1); //channel enable
    mask = HalVif_SensorMask(ch,mask);
    HalVif_SensorChannelEnable(ch,VIF_ENABLE);
    return 0;
}
EXPORT_SYMBOL(DrvVif_ConfigParallelIF);

s32 DrvVif_ConfigMipiIF(VIF_CHANNEL_e ch,
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
                            )
{
    volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*) g_TOPPAD1;
    u8 group = 0;//(ch >> 2);

    if (hdr && (ch != VIF_CHANNEL_0)) {
        VIF_DEBUG("[VIF] HDR mode must use channel 0.");
        return VIF_FAIL;
    }

    HalVif_HDRen(ch, 0);
    HalVif_HDRSelect(ch, VIF_HDR_VC0);

    HalVif_SensorFormat(ch,PixDepth); //select pixel depth

    HalVif_SensorRgbIn(ch, svif_sensor_in_format);

    HalVif_PixCropStart(ch,CropX);
    HalVif_PixCropEnd(ch,CropX+CropW-1);
    HalVif_LineCropStart(ch,CropY);
    HalVif_LineCropEnd(ch,CropY+CropH-1);

    HalVif_CropEnable(ch,CropEnable);

    HalVif_SetMCLK(ch,Mclk);

    HalVif_SelectSource(ch, group);

    HalVif_SetPclkSource(ch, group, 1, 0);
    HalVif_SetPclkSource(ch+1, group, 1, 0);
    HalVif_SetPclkSource(ch+2, group, 1, 0);
    HalVif_SetPclkSource(ch+3, group, 1, 0);

    //padtop->reg_ccir0_ctrl_mode = 1; //PDWN/RST/MCLK

    switch(group)
    {
    case 0:
        padtop->reg_snr0_d_ie = 0x0;
        break;
    case 1:
        padtop->reg_snr1_d_ie = 0x0;
        break;
    case 2:
        padtop->reg_snr2_d_ie = 0x0;
        break;
    case 3:
        padtop->reg_snr3_d_ie = 0x0;
        break;
    default:
            VIF_DEBUG("[VIF] Invalid channel for mipi sensor.");
        break;
    }

    //HalVif_SensorReset(ch,RstPol);
    //VIF_MS_SLEEP(1);
    //HalVif_SensorReset(ch,~RstPol);
    HalVif_SensorChannelEnable(ch,VIF_ENABLE);

    if (hdr) {
        HalVif_HDRen(ch, 1);
        HalVif_HDRSelect(ch, VIF_HDR_VC1);

        ch += 4;
        HalVif_SensorFormat(ch,PixDepth); //select pixel depth

        HalVif_SensorRgbIn(ch, svif_sensor_in_format);

        HalVif_PixCropStart(ch,CropX);
        HalVif_PixCropEnd(ch,CropX+CropW-1);
        HalVif_LineCropStart(ch,CropY);
        HalVif_LineCropEnd(ch,CropY+CropH-1);
        HalVif_CropEnable(ch,CropEnable);
        HalVif_SelectSource(ch, group);

        HalVif_SetPclkSource(ch, group, 1, 0);
        HalVif_SetPclkSource(ch+1, group, 1, 0);
        HalVif_SetPclkSource(ch+2, group, 1, 0);
        HalVif_SetPclkSource(ch+3, group, 1, 0);

        HalVif_HDRen(ch, 1);
        HalVif_HDRSelect(ch, VIF_HDR_VC0);

        HalVif_SensorChannelEnable(ch,VIF_ENABLE);
    }

    return VIF_SUCCESS;
}
EXPORT_SYMBOL(DrvVif_ConfigMipiIF);

#if 0
int DrvVif_ConfigMipiRX(u32 MipiPort,
                        u32 Lans,
                        u32 Speed
                       )
{

}
#endif
int DrvVif_ConfigBT656IF(SENSOR_PAD_GROUP_e pad_group, VIF_CHANNEL_e ch, VifBT656Cfg_t *pvif_bt656_cfg)
{
    int ret = VIF_SUCCESS;

    HalVif_BT656InputSelect(ch, pad_group);
    HalVif_BT656ChannelDetectEnable(ch, pvif_bt656_cfg->bt656_ch_det_en);
    HalVif_BT656ChannelDetectSelect(ch, pvif_bt656_cfg->bt656_ch_det_sel);
    HalVif_BT656BitSwap(ch, pvif_bt656_cfg->bt656_bit_swap);
#if 0
    HalVif_BT6568BitMode(ch, pvif_bt656_cfg->bt656_8bit_mode);
    switch(pad_group){ /*Note: It is sensor pad not VIF channel.*/
        case SENSOR_PAD_GROUP_A:
            HalVif_BT6568BitExt(ch, 1);
            break;
        case SENSOR_PAD_GROUP_B:
            HalVif_BT6568BitExt(ch, 0);
            break;
        default:
            //Error
            break;
    }
#endif
    HalVif_BT656VSDelay(ch, pvif_bt656_cfg->bt656_vsync_delay);
    HalVif_BT656HsyncInvert(ch,  pvif_bt656_cfg->bt656_hsync_inv);
    HalVif_BT656VsyncInvert(ch, pvif_bt656_cfg->bt656_vsync_inv);
    HalVif_BT656ClampEnable(ch, pvif_bt656_cfg->bt656_clamp_en);
    HalVif_BT656VerticalCropSize(ch,  0); //Do not modify it.
    HalVif_BT656HorizontalCropSize(ch, 0x0fff);  //Do not modify it.

    return ret;
}

int DrvVif_SetChannelSource(VIF_CHANNEL_e ch, VIF_CHANNEL_SOURCE_e svif_ch_src)
{
    int ret = VIF_SUCCESS;

    HalVif_SelectSource(ch, svif_ch_src);  //select vif source
    return ret;
}

int DrvVif_MCULoadBin(void)
{
    int ret=VIF_FAIL;
    int i=0;

    /* Move fw to MCU SRAM */
    REG_W(g_BDMA,0x02,0x0630); //02 SRC:SPI  DST:51 psram
    REG_W(g_BDMA,0x04,(IPCRamPhys & 0xFFFF)); //04 SRC start l addr
    REG_W(g_BDMA,0x05,((IPCRamPhys & 0x1FFFFFFF)>>16)&0xFFFF); //05 SRC start h addr
    REG_W(g_BDMA,0x06,0x0000); //06 DST start l addr
    REG_W(g_BDMA,0x07,0x0000); //07 DST start h addr
    REG_W(g_BDMA,0x08,0x6000); //08 DMA size 16kB   , can change size here
    REG_W(g_BDMA,0x09,0x0000); //09 DMA size
    REG_W(g_BDMA,0x03,0x0000); //09 DMA size


    VIF_MS_SLEEP(20);

    REG_W(g_BDMA,0x00,0x0001); //00 trigger DMA

#if 0
    DrvVif_msleep(500); //wait DMA done
    REG_W(g_BDMA,0x01,0x0008); //01 write [3] to clear DMA done
    ret = VIF_SUCCESS;
    //DrvVif_msleep(1000); //wait DMA done
#else


    for(i=0; i< 10000; i++){

        if( ((REG_R(g_BDMA,0x01) >> 3) & 0x0001) ==  1){
            VIF_DEBUG("-----------Load MCU done-----------\n");
            REG_W(g_BDMA,0x01,0x0008); //01 write [3] to clear DMA done
            VIF_MS_SLEEP(500);
            ret = VIF_SUCCESS;
            break;
        }
        else{
            VIF_MS_SLEEP(1); //wait DMA done
        }
    }


#endif
    if(ret == VIF_SUCCESS){

        /* MCU51 boot from SRAM */
        REG_W(g_MCU8051,0x0C,0x0001); //0C
        REG_W(g_MCU8051,0x00,0x0000); //00
        REG_W(g_MCU8051,0x01,0x0000); //01
        REG_W(g_MCU8051,0x02,0x0000); //02
        REG_W(g_MCU8051,0x03,0xFBFF); //03

        VIF_MS_SLEEP(500);
        REG_W(g_PMSLEEP,0x29,0x9003); //29
    }else{

        printk("------------Load MCU Fail-----------\n");
    }

    return ret;

}

int DrvVif_DmaEnable(VIF_CHANNEL_e ch,unsigned char en)
{
    HalRawDma_GlobalEnable();
    HalRawDma_GroupReset(ch/4);
    HalRawDma_GroupEnable(ch/4);
    //DrvVif_EnableInterrupt(ch,1);
    return VIF_SUCCESS;
}
EXPORT_SYMBOL(DrvVif_DmaEnable);

int DrvVif_RawStore(VIF_CHANNEL_e ch,u32 uMiuBase,u32 ImgW,u32 ImgH)
{
    WdmaCropParam_t tCrop = {0,0,ImgW,ImgH};
    pr_info("CH=%d, DMA Base = 0x%X\n",ch,uMiuBase);
    //DrvVif_SetDmagMLineCntInt(ch,1);
    //DrvVif_SetDmagMLineCntInt(ch,ImgH-1);
    HalRawDma_Config(ch,&tCrop,uMiuBase);
    HalRawDma_Trigger(ch,WDMA_TRIG_SINGLE);
    return VIF_SUCCESS;
}
EXPORT_SYMBOL(DrvVif_RawStore);

VIF_STATUS_e DrvVif_ContinueRawStore(VIF_CHANNEL_e ch,u32 uMiuBase,u32 ImgW,u32 ImgH,u8 bEn)
{
    WdmaCropParam_t tCrop = {0,0,ImgW,ImgH};
    if(bEn)
    {
        VIF_DEBUG("CH=%d, DMA Base = 0x%X\n",ch,uMiuBase);
        HalRawDma_Config(ch,&tCrop,uMiuBase);
        HalRawDma_Trigger(ch,WDMA_TRIG_CONTINUE);
    }
    else
    {
        HalRawDma_Trigger(ch,WDMA_TRIG_STOP);
    }
    return VIF_SUCCESS;
}
EXPORT_SYMBOL(DrvVif_ContinueRawStore);

int DrvVif_InputMask(int ch,int OnOff)
{
    return HalVif_SensorMask(ch,OnOff);
}
EXPORT_SYMBOL(DrvVif_InputMask);

s32 DrvVif_Reset(void)
{
    return true;
}


void DrvVif_SetDmaLineCntInt(VIF_CHANNEL_e ch, u8 en)
{
    /* [0]Main Y, [1]Main C, [2]Sub Y, [3]Sub C */
    u8 mask = 1;

    if(en){
        HalDma_LineCntIrqMask(ch, mask, 1);
        HalDma_LineCntIrqClr(ch, mask);
        HalDma_LineCntIrqMask(ch, mask, 0);
    }else{
        HalDma_LineCntIrqMask(ch, mask, 1);
    }
    return;
}
EXPORT_SYMBOL(DrvVif_SetDmaLineCntInt);

void DrvVif_SetDmaLineCnt(VIF_CHANNEL_e ch, u16 cnt)
{

    HalDma_SetDmaLineCnt(ch, cnt, 0, 0, 0);

    return;
}
EXPORT_SYMBOL(DrvVif_SetDmaLineCnt);

void DrvVif_SetVifFrameStartInt(u16 ch_bitmask, u8 en)
{

    if(en){
        HalVif_FrameStartIrqMask(ch_bitmask, 1);
        HalVif_FrameStartIrqClr(ch_bitmask);
        HalVif_FrameStartIrqMask(ch_bitmask, 0);
    }else{
        HalVif_FrameStartIrqMask(ch_bitmask, 1);
    }
    return;
}

void DriVif_SensorInit(u32 uCamID)
{
#if 0
    u32 sif_bus;
    VIF_SENSOR_FORMAT_e data_prec;
    VIF_CLK_POL pclk_pol;
    VIF_CLK_POL vsync_pol;
    VIF_CLK_POL hsync_pol;
    VIF_PIN_POL reset_pol;
    u32 x, y, width, height;
    VifMclk_e mclk;
    SensorInfo_t tInfo;

    if(DrvSensorGetInfo(uCamID,&tInfo)<0){
        VIF_DEBUG("Get group %d sensor information failed \n", uCamID);
        return;
    }

    if (DrvSensorGetSifBus(uCamID, &sif_bus)) {
        VIF_DEBUG("Get group %d sensor interface bus fail \n", uCamID);
            return;
    }

    DrvSensorGetDataPrec(uCamID, &data_prec);
    DrvSensorGetPclkPol(uCamID, &pclk_pol);
    DrvSensorGetVsyncPol(uCamID, &vsync_pol);
    DrvSensorGetHsyncPol(uCamID, &hsync_pol);
    DrvSensorGetResetPol(uCamID, &reset_pol);
    DrvSensorGetCropInfo(uCamID, &x, &y, &width, &height);
    DrvSensorGetMclk(uCamID, &mclk);

    VIF_DEBUG("sif_bus %d data_prec %d pclk_pol %d vsync_pol %d hsync_pol %d reset_pol %d\n",
                sif_bus, data_prec, pclk_pol, vsync_pol, hsync_pol, reset_pol);
    VIF_DEBUG("x %d y %d width %d height %d mclk %d\n", x, y, width, height, mclk);
    VIF_DEBUG("HDR %d \n", tInfo.eHDRMode);
    switch (sif_bus) {
        case 0:
        {
            DrvVif_ConfigParallelIF(uCamID*4,
                                            VIF_SENSOR_INPUT_FORMAT_RGB,
                                            data_prec,
                                            pclk_pol,
                                            vsync_pol,
                                            hsync_pol,
                                            reset_pol,
                                            x,
                                            y,
                                            width,
                                            height,
                                            1,
                                            mclk);
        }
        break;
        case 1:
        {
        Drv_CSISetClk(uCamID, 0);
        Drv_CSISetLane(uCamID, 4, 1);
        Drv_CSISetLongPacket(uCamID, 0, 0x1C00, 0);
        Drv_CSISetClkDataSkip(uCamID, 0x13,0x13);
            if(tInfo.eHDRMode == SENSOR_HDR_MODE_SONY_DOL)
                Drv_CSISetHDRMode(uCamID,CSI_HDR_MODE_SONY_290_326,1);

        DrvVif_ConfigMipiIF(uCamID*4,
                                    VIF_SENSOR_INPUT_FORMAT_RGB,
                                    data_prec,
                                    reset_pol,
                                    x,
                                    y,
                                    width,
                                    height,
                                    1,
                                mclk,
                                tInfo.eHDRMode?1:0);
        }
        break;
        default:
        break;
    }

    if(tInfo.eHDRMode == SENSOR_HDR_MODE_SONY_DOL) {
    //ToDo change order
        if (DrvSensorInit(uCamID, 1)) {
                VIF_DEBUG("Init group %d sensor fail \n", uCamID);
            return;
    }
    } else {
        //ToDo change order
        if (DrvSensorInit(uCamID, 0)) {
                VIF_DEBUG("Init group %d sensor fail \n", uCamID);
                return;
        }
    }
#endif


}
EXPORT_SYMBOL(DriVif_SensorInit);
