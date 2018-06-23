#ifndef HAL_DMA_PRIVATE_H
#define HAL_DMA_PRIVATE_H

#include "arch/infinity2_reg_block_ispsc.h"
#include "arch/infinity2_reg_isp0.h"
//#include "arch/infinity2_reg_isp10.h"
#include "arch/infinity2_reg_isp11.h"
#include "arch/infinity2_reg_isp12.h"
//#include "arch/infinity2_reg_isp1.h"
//#include "arch/infinity2_reg_isp2.h"
//#include "arch/infinity2_reg_isp3.h"
//#include "arch/infinity2_reg_isp4.h"
//#include "arch/infinity2_reg_isp5.h"
//#include "arch/infinity2_reg_isp6.h"
//#include "arch/infinity2_reg_isp7.h"
//#include "arch/infinity2_reg_isp8.h"
//#include "arch/infinity2_reg_isp9.h"
//#include "arch/infinity2_reg_isp_dmag.h"
//#include "arch/infinity2_reg_isp_dmag_m.h"
//#include "arch/infinity2_reg_isp_dmag_sub.h"
//#include "arch/infinity2_reg_isp_dmag_sub_m1.h"
//#include "arch/infinity2_reg_isp_dmag_sub_m2.h"
//#include "arch/infinity2_reg_isp_dmag_sub_m3.h"
#include "arch/infinity2_reg_isp_dma_irq.h"
//#include "arch/infinity2_reg_isp_miu2sram.h"
//#include "arch/infinity2_reg_ispsc_vif.h"
//#include "arch/infinity2_reg_padtop1.h"
#include "arch/infinity2_reg_isp_wdma.h"

typedef struct
{
    volatile infinity2_isp_wdma* pstYDmaMain;   //Main Y dma controller
    volatile infinity2_isp_wdma* pstCDmaMain;   //Main C dma controller
    volatile infinity2_isp_wdma* pstYDmaSub;    //Sub Y dma controller
    volatile infinity2_isp_wdma* pstCDmaSub;    //Sub C dma controller
    volatile infinity2_isp_wdma_yuv_ctrl* pst422to420Ctl;   //YUV422 to YUV420 re-sample parameters
}VifYuvDma_t;

typedef struct
{
    volatile infinity2_reg_isp0* isp0;  //BANK 1509H
    //volatile infinity2_reg_isp1* isp1;    //BANK 150AH
    //volatile infinity2_reg_isp2* isp2;    //BANH 150BH
    //volatile infinity2_reg_isp3* isp3;    //BANH 150CH
    //volatile infinity2_reg_isp4* isp4;    //BANH 150DH
    //volatile infinity2_reg_isp5* isp5;    //BANH 150EH
    //volatile infinity2_reg_isp6* isp6;    //BANH 150FH
    //volatile infinity2_reg_isp7* isp7;    //BANH 1510H
    //volatile infinity2_reg_isp8* isp8;    //BANH 1511H
    //volatile infinity2_reg_isp9* isp9;    //BANH 1512H
    //volatile infinity2_reg_isp10* isp10;//BANH 1513H
    volatile infinity2_reg_isp11* isp11;//BANH 1514H
    volatile infinity2_reg_isp12* isp12;//BANH 1516H

    //DMA GROUP0
    //volatile infinity2_reg_isp_dmag_sub_m2* dmag00;   //BANK 1420H
    //volatile infinity2_reg_isp_dmag_sub*  dmag01; //BANK 1421H
    //volatile infinity2_reg_isp_dmag_sub_m1*   dmag02; //BANK 1422H
    //volatile infinity2_reg_isp_dmag_sub_m1*   dmag03; //BANK 1423H
    //DMA GROUP1
    //volatile infinity2_reg_isp_dmag_sub_m3* dmag10;   //BANK 1424H
    //volatile infinity2_reg_isp_dmag_sub_m1* dmag11;   //BANK 1425H
    //volatile infinity2_reg_isp_dmag_sub_m1*   dmag12; //BANK 1426H
    //volatile infinity2_reg_isp_dmag_sub_m1*   dmag13; //BANK 1427H
    //DMA GROUP2
    //volatile infinity2_reg_isp_dmag_sub_m3* dmag10;   //BANK 1428H
    //volatile infinity2_reg_isp_dmag_sub_m1* dmag11;   //BANK 1429H
    //volatile infinity2_reg_isp_dmag_sub_m1*   dmag12; //BANK 142AH
    //volatile infinity2_reg_isp_dmag_sub_m1*   dmag13; //BANK 142BH
    //DMA GROUP3
    //volatile infinity2_reg_isp_dmag_sub_m3* dmag10;   //BANK 142CH
    //volatile infinity2_reg_isp_dmag_sub_m1* dmag11;   //BANK 142DH
    //volatile infinity2_reg_isp_dmag_sub_m1*   dmag12; //BANK 142EH
    //volatile infinity2_reg_isp_dmag_sub_m1*   dmag13; //BANK 142FH

    volatile infinity2_reg_isp_dma_irq *irq;
    //YUV DMAs
    VifYuvDma_t Dma[16];
}VdmaHandle_t;

#define BANK_TO_ADDR(bank) (bank<<9)
#define CMQ_RIU_REG(bank,offset) (((bank)<<8)+((offset)<<1)) //register address from CMQ
#define CPU_RIU_REG(bank,offset) (((bank)<<9)+((offset)<<2)) //register address from CPU

typedef void* HalDmaHandle;
extern VdmaHandle_t gstHandle;
#endif
