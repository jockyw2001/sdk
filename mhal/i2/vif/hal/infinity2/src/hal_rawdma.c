#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <asm/io.h>
#include <linux/clk-provider.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <ms_platform.h>
#include <asm/string.h>
#include <asm/io.h>
#include <hal_dma_private.h>
#include "vif_datatype.h"

#include <hal_rawdma.h>
#include <vif_common.h>
#include "hal_dma.h"

#ifndef REG_R
#define REG_R(base,offset) (*(unsigned short*)((u32)base+(offset*4)))
#define REG_W(base,offset,val) ((*(unsigned short*)(((u32)base)+ (offset*4)))=(val))
//#define REG_W(base,offset,val) ((*(unsigned short*)((base)+ (offset*4)))=(val))
#endif

int HalRawDma_GlobalEnable(void)
{
    VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;
    //sw reset
    pstHnd->isp0->reg_en_isp = 0;
#if 0
    pstHnd->isp0->reg_isp_sw_rstz = 0;
    pstHnd->isp0->reg_isp_sw_p1_rstz = 0;
    pstHnd->isp0->reg_isp_sw_p2_rstz = 0;
    pstHnd->isp0->reg_isp_sw_p3_rstz = 0;

    pstHnd->isp0->reg_isp_fifo_mode = 2; //1024 * 4 , for 4 ch mode
    pstHnd->isp0->reg_42to40_store_en = 0;
    pstHnd->isp0->reg_data_store_src_sel = 0;

    pstHnd->isp0->reg_isp_sw_p1_rstz = 1;
    pstHnd->isp0->reg_isp_sw_p2_rstz = 1;
    pstHnd->isp0->reg_isp_sw_p3_rstz = 1;
    pstHnd->isp0->reg_isp_sw_rstz = 1;
#endif
    pstHnd->isp0->reg_en_isp = 1;
    return 0;
}

int HalRawDma_GroupReset(u32 uGroup)
{
    //volatile infinity2_reg_block_ispsc* vifclk = (infinity2_reg_block_ispsc*) g_ISP_ClkGen;
    VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;
    switch(uGroup)
    {
    case 0:
        //sr0
        pstHnd->isp0->reg_isp_sw_rstz = 0;
        udelay(1);
        pstHnd->isp0->reg_isp_sw_rstz = 1;
    break;
    case 1:
        //sr1
        pstHnd->isp0->reg_isp_sw_p1_rstz = 0;
        udelay(1);
        pstHnd->isp0->reg_isp_sw_p1_rstz = 1;
    break;
    case 2:
        //sr2
        pstHnd->isp0->reg_isp_sw_p2_rstz = 0;
        udelay(1);
        pstHnd->isp0->reg_isp_sw_p2_rstz = 1;
    break;
    case 3:
        //sr3
        pstHnd->isp0->reg_isp_sw_p3_rstz = 0;
        udelay(1);
        pstHnd->isp0->reg_isp_sw_p3_rstz = 1;
    break;
    }
    return 0;
}

int HalRawDma_GroupEnable(u32 uGroup)
{
    //volatile infinity2_reg_block_ispsc* vifclk = (infinity2_reg_block_ispsc*) g_ISP_ClkGen;
    VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;
    u16 reg;
    switch(uGroup)
    {
    case 0:
        //sr0
        /*    VIF CLK    */
        //vifclk->reg_ckg_snr0 = 0x18;
        //vifclk->reg_ckg_snr1 = 0x18;
        //vifclk->reg_ckg_snr2 = 0x18;
        //vifclk->reg_ckg_snr3 = 0x18;
        pstHnd->isp0->reg_isp_fifo_mode = 2; //1024 * 4 , for 4 ch mode
        pstHnd->isp0->reg_p0h_rawstore_mode = 1;
        pstHnd->isp0->reg_42to40_store_en = 0;
    break;
    case 1:
        //sr1
        /*    VIF CLK    */
        //vifclk->reg_ckg_snr4 = 0x18;
        //vifclk->reg_ckg_snr5 = 0x18;
        //vifclk->reg_ckg_snr6 = 0x18;
        //vifclk->reg_ckg_snr7 = 0x18;
        pstHnd->isp11->reg_isp_fifo_mode_sel = 2; //1024 * 4 , for 4 ch mode
        pstHnd->isp0->reg_p1h_rawstore_mode = 1;
        pstHnd->isp0->reg_42to40_store_en = 0;
    break;
    case 2:
        //sr2
        /*    VIF CLK    */
        //vifclk->reg_ckg_snr8 = 0x18;
        //vifclk->reg_ckg_snr9 = 0x18;
        //vifclk->reg_ckg_snr10 = 0x18;
        //vifclk->reg_ckg_snr11 = 0x18;

        //pstHnd->isp12->reg_isp_fifo_mode_sel = 2; //1024 * 4 , for 4 ch mode
        reg = REG_R(pstHnd->isp12,0x70);
        reg &= ~(0x6000);
        reg |= 2<<13;
        REG_W(pstHnd->isp12,0x70,reg); //4096*1, for 1 ch mode

        pstHnd->isp0->reg_p2h_rawstore_mode = 1;
        pstHnd->isp0->reg_42to40_store_en = 0;
    break;
    case 3:
        //sr3
        /*    VIF CLK    */
        //vifclk->reg_ckg_snr12 = 0x18;
        //vifclk->reg_ckg_snr13 = 0x18;
        //vifclk->reg_ckg_snr14 = 0x18;
        //vifclk->reg_ckg_snr15 = 0x18;
        //pstHnd->isp12->reg_isp_fifo_mode_sel = 2; //1024 * 4 , for 4 ch mode
        reg = REG_R(pstHnd->isp12,0x60);
        reg &= ~(0x6000);
        reg |= 2<<13;
        REG_W(pstHnd->isp12,0x60,reg); //4096*1, for 1 ch mode

        pstHnd->isp0->reg_p3h_rawstore_mode = 1;
        pstHnd->isp0->reg_42to40_store_en = 0;
    break;
    }
    return 0;
}

int HalRawDma_GroupDisable(u32 uGroup)
{
    //volatile infinity2_reg_block_ispsc* vifclk = (infinity2_reg_block_ispsc*) g_ISP_ClkGen;
    VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;
    u16 reg;
    switch(uGroup)
    {
    case 0:
        pstHnd->isp0->reg_isp_fifo_mode = 2; //1024 * 4 , for 4 ch mode
        pstHnd->isp0->reg_p0h_rawstore_mode = 0;
        pstHnd->isp0->reg_42to40_store_en = 0;
    break;
    case 1:
        pstHnd->isp11->reg_isp_fifo_mode_sel = 2; //1024 * 4 , for 4 ch mode
        pstHnd->isp0->reg_p1h_rawstore_mode = 0;
        pstHnd->isp0->reg_42to40_store_en = 0;
    break;
    case 2:
        //pstHnd->isp12->reg_isp_fifo_mode_sel = 2; //1024 * 4 , for 4 ch mode
        reg = REG_R(pstHnd->isp12,0x70);
        reg &= ~(0x6000);
        reg |= 2<<13;
        REG_W(pstHnd->isp12,0x70,reg); //4096*1, for 1 ch mode
        pstHnd->isp0->reg_p2h_rawstore_mode = 0;
        pstHnd->isp0->reg_42to40_store_en = 0;
    break;
    case 3:
        //pstHnd->isp12->reg_isp_fifo_mode_sel = 2; //1024 * 4 , for 4 ch mode
        reg = REG_R(pstHnd->isp12,0x60);
        reg &= ~(0x6000);
        reg |= 2<<13;
        REG_W(pstHnd->isp12,0x60,reg); //4096*1, for 1 ch mode
        pstHnd->isp0->reg_p3h_rawstore_mode = 0;
        pstHnd->isp0->reg_42to40_store_en = 0;
    break;
    }
    return 0;
}

int HalRawDma_Config(u32 uCh,WdmaCropParam_t *ptCrop,u32 uMiuBase)
{
    VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;
    //volatile infinity2_isp_wdma_yuv_ctrl *yuv_ctrl = pstHnd->Dma[uCh].pst422to420Ctl;

    //Y dma setting
    volatile infinity2_isp_wdma *dma = pstHnd->Dma[uCh].pstYDmaMain;
    dma->reg_isp_wdma_mode = 0x6; //4'd6: 64-bit (1T4P, 16-bit/P)
    dma->reg_isp_wdma_pitch = (ptCrop->uW+15)>>4;
    dma->reg_isp_wdma_width_m1 = ((ptCrop->uW+3)/4)-1;
    dma->reg_isp_wdma_height_en = 1;
    dma->reg_isp_wdma_height_m1 = ptCrop->uH - 1;
    dma->reg_isp_wdma_base = (uMiuBase>>5)&0xFFFF;
    dma->reg_isp_wdma_base_1 = (uMiuBase>>21)&0xFFFF;
    dma->reg_isp_wdma_align_en = 1; //auto align mode
    /*
    switch(uCh)
    {
    case 0:
        pstHnd->isp0->reg_p0h_rawstore_mode = 1;
    break;
    case 1:
        pstHnd->isp0->reg_p1h_rawstore_mode = 1;
    break;
    case 2:
        pstHnd->isp0->reg_p2h_rawstore_mode = 1;
    break;
    case 3:
        pstHnd->isp0->reg_p3h_rawstore_mode = 1;
    break;
    }
    */
    return 0;
}

int HalRawDma_Trigger(u32 uCh,WdmaTrigMode_e eMode)
{
    VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;
    //infinity2_isp_wdma *dma = pstHnd->Dma[uCh].pstYDmaSub;
    if(uCh>=16)
    {
        BUG();
    }
    switch(eMode)
    {
    case WDMA_TRIG_SINGLE:
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_en = 1;
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_auto = 0;
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_trigger_mode = 1;
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_trigger = 1;

    break;
    case WDMA_TRIG_CONTINUE:
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_en = 1;
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_auto = 1;
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_trigger_mode = 1;
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_trigger = 0;
    break;
    case WDMA_TRIG_STOP:
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_en = 0;
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_auto = 0;
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_trigger_mode = 0;
        pstHnd->Dma[uCh].pstYDmaMain->reg_isp_wdma_trigger = 0;
    break;
    default:
    break;
    }
    return 0;
}
