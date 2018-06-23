#ifndef MDRV_MLOAD_H_
#define MDRV_MLOAD_H_

#include "mhal_common.h"
#include "mhal_cmdq.h"


#define LINUX_UNIT_TEST 1

typedef void*  MLOAD_HANDLE;

// MIU
#define GAMMA_TBL_SIZE                 (256 * GMMA_TBL_BITWIDTH/2)  //256  x 64 bits
#define ALSC_TBL_SIZE_W                (61)
#define ALSC_TBL_SIZE_H                (69)
#define ALSC_TBL_SIZE                  (4209 * ALSC_TBL_BITWIDTH/2) //4208 x 64 bits
#define DEFECTPIX_TBL_SIZE             (1024 * DEFECTPIX_TBL_BITWIDTH/2) //1024 x 32 bits
#define FPN_TBL_SIZE                   (2816 * FPN_TBL_BITWIDTH/2)   //2816 x 16 bits

#define GMMA_TBL_BITWIDTH        (8)
#define ALSC_TBL_BITWIDTH        (8)
#define DEFECTPIX_TBL_BITWIDTH   (4)
#define FPN_TBL_BITWIDTH         (2)

#define HSP_C_SC1_SIZE                 (64 * HSP_C_SC1_WIDTH/2)   //64   x64bits
#define HSP_Y_SC1_SIZE                 (64 * HSP_Y_SC1_WIDTH/2)   //64   x64bits
#define VSP_C_SC1_SIZE                 (64 * VSP_C_SC1_WIDTH/2)   //64   x64bits
#define VSP_Y_SC1_SIZE                 (64 * VSP_Y_SC1_WIDTH/2)   //64   x64bits
#define IHC_LUT_SIZE                   (289 * IHC_LUT_WIDTH/2)  //289x64   x64bits
#define ICC_LUT_SIZE                   (289 * ICC_LUT_WIDTH/2)  //289x64   x64bits
#define YUV_GAMMA_Y_SIZE               (256 * YUV_GAMMA_Y_WIDTH/2)    //256   x 16bits
#define YUV_GAMMA_UV_SIZE              (128 * YUV_GAMMA_UV_WIDTH/2)  //289   x 32bits
#define RGB12TO12_PRE_SIZE             (256 * RGB12TO12_PRE_WIDTH/2)  //289   x 64bits
#define RGB12TO12_POST_SIZE            (256 * RGB12TO12_POST_WIDTH/2)  //289   x 64bits
#define WDR_SIZE                       (256 * WDR_WIDTH/2)   //88   x 64bits
#define HSP_C_SC2_SIZE                 (64 * HSP_C_SC2_WIDTH/2)   //64   x64bits
#define HSP_Y_SC2_SIZE                 (64 * HSP_Y_SC2_WIDTH/2)   //64   x64bits
#define VSP_C_SC2_SIZE                 (64 * VSP_C_SC2_WIDTH/2)   //64   x64bits
#define VSP_Y_SC2_SIZE                 (64 * VSP_Y_SC2_WIDTH/2)   //64   x64bits
#define HSP_C_SC3_SIZE                 (64 * HSP_C_SC3_WIDTH/2)   //64   x64bits
#define HSP_Y_SC3_SIZE                 (64 * HSP_Y_SC3_WIDTH/2)   //64   x64bits
#define VSP_C_SC3_SIZE                 (64 * VSP_C_SC3_WIDTH/2)   //64   x64bits
#define VSP_Y_SC3_SIZE                 (64 * VSP_Y_SC3_WIDTH/2)   //64   x64bits
#define HSP_C_SC4_SIZE                 (64 * HSP_C_SC4_WIDTH/2)   //64   x64bits
#define HSP_Y_SC4_SIZE                 (64 * HSP_Y_SC4_WIDTH/2)   //64   x64bits
#define VSP_C_SC4_SIZE                 (64 * VSP_C_SC4_WIDTH/2)   //64   x64bits
#define VSP_Y_SC4_SIZE                 (64 * VSP_Y_SC4_WIDTH/2)   //64   x64bits


#define HSP_C_SC1_WIDTH                 (8)   //x64bits
#define HSP_Y_SC1_WIDTH                 (8)   //64bits
#define VSP_C_SC1_WIDTH                 (8)   //64bits
#define VSP_Y_SC1_WIDTH                 (8)   //64bits
#define IHC_LUT_WIDTH                   (8)   //64bits
#define ICC_LUT_WIDTH                   (8)   //64bits
#define YUV_GAMMA_Y_WIDTH               (2)   //16bits
#define YUV_GAMMA_UV_WIDTH              (4)   //32bits
#define RGB12TO12_PRE_WIDTH             (8)   //64bits
#define RGB12TO12_POST_WIDTH            (8)   //64bits
#define WDR_WIDTH                       (8)   //64bits
#define HSP_C_SC2_WIDTH                 (8)   //64bits
#define HSP_Y_SC2_WIDTH                 (8)   //64bits
#define VSP_C_SC2_WIDTH                 (8)   //64bits
#define VSP_Y_SC2_WIDTH                 (8)   //64bits
#define HSP_C_SC3_WIDTH                 (8)   //64bits
#define HSP_Y_SC3_WIDTH                 (8)   //64bits
#define VSP_C_SC3_WIDTH                 (8)   //64bits
#define VSP_Y_SC3_WIDTH                 (8)   //64bits
#define HSP_C_SC4_WIDTH                 (8)   //64bits
#define HSP_Y_SC4_WIDTH                 (8)   //64bits
#define VSP_C_SC4_WIDTH                 (8)   //64bits
#define VSP_Y_SC4_WIDTH                 (8)   //64bits

/////////////// MIU MenuLoad ///////////
#define _MLOAD_PADDING u64:64   //padding for MLoad HW bug
typedef struct
{
    u16 pipe0_fpn[FPN_TBL_SIZE] __attribute__((aligned(16)));           _MLOAD_PADDING;
    u16 pipe0_gamma16to16[GAMMA_TBL_SIZE] __attribute__((aligned(16))); _MLOAD_PADDING;
    u16 pipe0_alsc[ALSC_TBL_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 pipe1_fpn[FPN_TBL_SIZE] __attribute__((aligned(16)));           _MLOAD_PADDING;
    u16 pipe1_gamma12to12[GAMMA_TBL_SIZE] __attribute__((aligned(16))); _MLOAD_PADDING;
    u16 pipe1_alsc[ALSC_TBL_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 dpc[DEFECTPIX_TBL_SIZE] __attribute__((aligned(16)));           _MLOAD_PADDING;
    u16 pipe0_gamma10to10_r[GAMMA_TBL_SIZE] __attribute__((aligned(16))); _MLOAD_PADDING;
    u16 wdr[WDR_SIZE] __attribute__((aligned(16)));                     _MLOAD_PADDING;
    u16 yuv_gamma_y[YUV_GAMMA_Y_SIZE] __attribute__((aligned(16)));     _MLOAD_PADDING;
    u16 yuv_gamma_uv[YUV_GAMMA_UV_SIZE] __attribute__((aligned(16)));   _MLOAD_PADDING;
    u16 rgb12to12_pre[RGB12TO12_PRE_SIZE] __attribute__((aligned(16))); _MLOAD_PADDING;
    u16 rgb12to12_post[RGB12TO12_POST_SIZE] __attribute__((aligned(16)));_MLOAD_PADDING;
    u16 hsp_c_sc1[HSP_C_SC1_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 hsp_y_sc1[HSP_Y_SC1_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;;
    u16 vsp_c_sc1[VSP_C_SC1_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 vsp_y_sc1[VSP_Y_SC1_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 hsp_c_sc2[HSP_C_SC2_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 hsp_y_sc2[HSP_Y_SC2_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 vsp_c_sc2[VSP_C_SC2_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 vsp_y_sc2[VSP_Y_SC2_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 hsp_c_sc3[HSP_C_SC3_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 hsp_y_sc3[HSP_Y_SC3_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 vsp_c_sc3[VSP_C_SC3_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 vsp_y_sc3[VSP_Y_SC3_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 hsp_c_sc4[HSP_C_SC4_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 hsp_y_sc4[HSP_Y_SC4_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 vsp_c_sc4[VSP_C_SC4_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
    u16 vsp_y_sc4[VSP_Y_SC4_SIZE] __attribute__((aligned(16)));         _MLOAD_PADDING;
}__attribute__((aligned(16))) MLoadLayout,MLoadLayout_t;

typedef enum
{
    MLOAD_ID_PIPE0_FPN = 0,         //pipe0 FPN
    MLOAD_ID_PIPE0_GMA16TO16 = 1,   //pipe0 RGB gamma 16 to 16
    MLOAD_ID_PIPE0_ALSC = 2,        //pipe0 RGB lens shading
    MLOAD_ID_PIPE1_FPN = 4,         //pipe1 FPN
    MLOAD_ID_PIPE1_GMA12TO12 = 5,   //pipe1 RGB gamma 12 to 12
    MLOAD_ID_PIPE1_ALSC = 6,        //pipe1 RGB lens shading
    MLOAD_ID_DPC = 16,              //defect pixel
    MLOAD_ID_GMA10TO10 = 17,         //pipe0 RGB gamma 10 to 10 table
    MLOAD_ID_NUM = 18,
    SCL_MLOAD_ID_BASE = 33,
    SCL_MLOAD_ID_HSP_C_SC1 = 33,
    SCL_MLOAD_ID_HSP_Y_SC1 = 34,
    SCL_MLOAD_ID_VSP_C_SC1 = 35,
    SCL_MLOAD_ID_VSP_Y_SC1 = 36,
    SCL_MLOAD_ID_IHC_LUT = 37,
    SCL_MLOAD_ID_ICC_LUT = 38,
    SCL_MLOAD_ID_YUV_GAMMA_Y = 39,
    SCL_MLOAD_ID_YUV_GAMMA_UV = 40,
    SCL_MLOAD_ID_RGB12TO12_PRE = 41,
    SCL_MLOAD_ID_RGB12TO12_POST = 42,
    SCL_MLOAD_ID_WDR = 43,
    SCL_MLOAD_ID_HSP_C_SC2 = 49,
    SCL_MLOAD_ID_HSP_Y_SC2 = 50,
    SCL_MLOAD_ID_VSP_C_SC2 = 51,
    SCL_MLOAD_ID_VSP_Y_SC2 = 52,
    SCL_MLOAD_ID_HSP_C_SC3 = 53,
    SCL_MLOAD_ID_HSP_Y_SC3 = 54,
    SCL_MLOAD_ID_VSP_C_SC3 = 55,
    SCL_MLOAD_ID_VSP_Y_SC3 = 56,
    SCL_MLOAD_ID_HSP_C_SC4 = 57,
    SCL_MLOAD_ID_HSP_Y_SC4 = 58,
    SCL_MLOAD_ID_VSP_C_SC4 = 59,
    SCL_MLOAD_ID_VSP_Y_SC4 = 60,
    SCL_MLOAD_ID_NUM = 61

}IspMloadID_e;


typedef enum{
    CMDQ_MODE,
    RIU_MODE
}FrameSyncMode;

#if 0
typedef struct MloadCmdqIf_t
{
    void (*RegW)(struct MloadCmdqIf_t* this,unsigned int uRiuAddr,unsigned short uVal);
    void (*RegWM)(struct MloadCmdqIf_t* this,unsigned int uRiuAddr,unsigned short uVal,unsigned short nMask);
    int (*RegPollWait)(struct MloadCmdqIf_t* this,unsigned int uRiuAddr,unsigned short uMask,unsigned short uVal,u32 u32Time);
}MloadCmdqIf_t;
#else
typedef MHAL_CMDQ_CmdqInterface_t MloadCmdqIf_t;
#endif

typedef struct{
    FrameSyncMode mode;
    MloadCmdqIf_t *pCmqInterface_t;
}MLOAD_ATTR;

typedef enum{
    ISP_MLOAD_DOMAIN_FRONT, //before bayer scaler
    ISP_MLOAD_DOMAIN_IMAGE, //after bayer scaler
    SCL_MLOAD,
}IspMloadDomain_e;

MLOAD_HANDLE IspMLoadInit(MLOAD_ATTR attr);
int IspMLoadDeInit(MLOAD_HANDLE handle);
int IspMLoadTableSet(MLOAD_HANDLE handle, IspMloadID_e id,void *table);
const u16* IspMLoadTableGet(MLOAD_HANDLE handle, IspMloadID_e id);
int IspMLoadApply(MLOAD_HANDLE handle,IspMloadDomain_e domain);
int SclMLoadApply(MLOAD_HANDLE handle);
MloadCmdqIf_t* IspMLoadChangeCmdqIF(MLOAD_HANDLE handle,MloadCmdqIf_t* pNew);
void IspMloadUT(MHAL_CMDQ_CmdqInterface_t *cmdq,FrameSyncMode mode);



#endif //MDRV_MLOAD_H_
